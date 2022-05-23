import java.awt.Color;
import java.awt.Dimension;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

import javax.swing.BorderFactory;
import javax.swing.SwingWorker;

import org.knowm.xchart.XChartPanel;
import org.knowm.xchart.XYChart;


/**
 * Creates a real-time chart using SwingWorker
 */
public class ChartsManager {

	public UpdateChartsWorker worker;
	public XChartPanel<XYChart> TACSChartPanel;
	public XChartPanel<XYChart> BNCChartPanel;
	public XChartPanel<XYChart> ButtonChartPanel;
	public XChartPanel<XYChart> LEDChartPanel;
	public XYChart TACSChart;
	public XYChart BNCChart;
	public XYChart ButtonChart;
	public XYChart LEDChart;

	public void start() {

		// Create TACS Chart
		TACSChart = new XYChart(400, 150);
		TACSChart.setTitle("TACS Volts");
//		TACSChart.setXAxisTitle("Time [s]");
		TACSChart.setYAxisTitle("Volts [V]");
		TACSChart.addSeries("TACS", new double[] { 0 }, new double[] { 0 });
		TACSChart.getStyler().setLegendVisible(true);
		TACSChart.getStyler().setXAxisTicksVisible(true);
		TACSChart.getStyler().setMarkerSize(0);

		// Create BNC Chart
		BNCChart = new XYChart(400, 150);
		BNCChart.setTitle("BNC Volts");
		BNCChart.setXAxisTitle("Time [s]");
		BNCChart.setYAxisTitle("Volts [V]");
		BNCChart.addSeries("in", new double[] { 0 }, new double[] { 0 });
		BNCChart.addSeries("out", new double[] { 0 }, new double[] { 0 });
		BNCChart.getStyler().setLegendVisible(true);
		BNCChart.getStyler().setXAxisTicksVisible(true);
		BNCChart.getStyler().setYAxisMin(0.0);
		BNCChart.getStyler().setYAxisMax(4.0);
		BNCChart.getStyler().setMarkerSize(0);

		// Create Button Chart
		ButtonChart = new XYChart(400, 150);
		ButtonChart.setTitle("Buttons");
//		ButtonChart.setXAxisTitle("Time [s]");
		ButtonChart.setYAxisTitle("Pressed");
		ButtonChart.addSeries("left", new double[] { 0 }, new double[] { 0 });
		ButtonChart.addSeries("right", new double[] { 0 }, new double[] { 0 });
		ButtonChart.getStyler().setLegendVisible(true);
		ButtonChart.getStyler().setXAxisTicksVisible(true);
		ButtonChart.getStyler().setYAxisMin(0.0);
		ButtonChart.getStyler().setYAxisMax(1.0);
		ButtonChart.getStyler().setMarkerSize(0);

		// Create LED Chart
		LEDChart = new XYChart(400, 150);
		LEDChart.setTitle("LEDs");
//		LEDChart.setXAxisTitle("Time [s]");
		LEDChart.setYAxisTitle("Brightness [%]");
		LEDChart.addSeries("led 0", new double[] { 0 }, new double[] { 0 });
		LEDChart.addSeries("led 1", new double[] { 0 }, new double[] { 0 });
		LEDChart.addSeries("led 2", new double[] { 0 }, new double[] { 0 });
		LEDChart.addSeries("led 3", new double[] { 0 }, new double[] { 0 });
		LEDChart.addSeries("led 4", new double[] { 0 }, new double[] { 0 });
		LEDChart.addSeries("led 5", new double[] { 0 }, new double[] { 0 });
		LEDChart.getStyler().setLegendVisible(true);
		LEDChart.getStyler().setXAxisTicksVisible(true);
		LEDChart.getStyler().setYAxisMin(0.0);
		LEDChart.getStyler().setYAxisMax(1.0);
		LEDChart.getStyler().setMarkerSize(0);

		// Show it
		TACSChartPanel = new XChartPanel<XYChart>(TACSChart);
		BNCChartPanel = new XChartPanel<XYChart>(BNCChart);
		ButtonChartPanel = new XChartPanel<XYChart>(ButtonChart);
		LEDChartPanel = new XChartPanel<XYChart>(LEDChart);
		
		// REMOVE action listeners from the charts
		TACSChartPanel.getActionMap().clear();
		BNCChartPanel.getActionMap().clear();
		ButtonChartPanel.getActionMap().clear();
		LEDChartPanel.getActionMap().clear();
		
		worker = new UpdateChartsWorker();
		worker.execute();
	}

	public void stop() {
		worker.cancel(true);
	}
	
	public void setXAxisParameters(int numSamples, double sampleRateHz, boolean enableDownSampling) {
		worker.setXAxisParameters(numSamples, sampleRateHz, enableDownSampling);
	}
	
	public void setDynamicXAxis(boolean enable) {
		worker.setDynamicXAxis(enable);
	}

	public class UpdateChartsWorker extends SwingWorker<Boolean, Double> {

		/* Down Sampling Parameters */
		final int downSamplingMaxNumSamples = 1000;
		private boolean enableDownSampling = true;
		private int dropCounter = 0;
		private int dropCount = 0;
		
		private long numTotalSamples = 0;
		private double sampleRateHz = 500;
		private int valuesCapacity = 0;
		private int valuesIndex = 0;
		private double values[][] = null;
		
		// The lock that secures all of the above variables
		private final Object valuesLock = new Object();
		
		private AtomicBoolean enableDynamicXAxis = new AtomicBoolean(false);
		private AtomicReference<double[]> defaultXAxisVals = new AtomicReference<double[]>(null);

		public UpdateChartsWorker() {
			setXAxisParameters(2000, 500, true);
		}
		
		public void setXAxisParameters(int numSamples, double sampleRateHz, boolean enableDownSampling) {			
			synchronized (valuesLock) {
				this.numTotalSamples = 0;
				this.sampleRateHz = sampleRateHz;
				// Check if we are enabling down sampling
				this.enableDownSampling = enableDownSampling;
				if(enableDownSampling) {
					// Setup drop count - keep only 1000 samples
					if(numSamples < downSamplingMaxNumSamples) {
						dropCount = 1;
					}else {
						dropCount = (int)Math.ceil( (double)numSamples/downSamplingMaxNumSamples );
					}
					// reset dropCounter
					dropCounter = 0;
					// set values capacity - to the floor
					this.valuesCapacity = (int) (numSamples/dropCount);
					
					// Create default X axis values
					double[] defaultXAxisVals_Arr = new double[this.valuesCapacity];
					for(int i=0; i<this.valuesCapacity; i++) {
						defaultXAxisVals_Arr[i] = (double)i*dropCount/sampleRateHz;
					}
					defaultXAxisVals.set(defaultXAxisVals_Arr);
				}else {
					// set values capacity
					this.valuesCapacity = numSamples;
					
					// Create default X axis values
					double[] defaultXAxisVals_Arr = new double[this.valuesCapacity];
					for(int i=0; i<this.valuesCapacity; i++) {
						defaultXAxisVals_Arr[i] = (double)i/sampleRateHz;
					}
					defaultXAxisVals.set(defaultXAxisVals_Arr);
				}

				// ensure that valuesIndex is always less than valuesCapacity 
				// (so we can index into the values array)
				if(this.valuesIndex < this.valuesCapacity) {
					// do nothing
				}else {
					this.valuesIndex = 0;
				}

				// Initialize values
				values = new double[11][this.valuesCapacity];
				for(int i=0; i<values.length; i++) {
					Arrays.fill(values[i], 0.0);
				}

			}
		}
		
		public void setDynamicXAxis(boolean enable) {
			enableDynamicXAxis.set(enable);
		}

		public void addData(float tacs_volts, float bnc_in_volts, float bnc_out_volts, byte left_button,
				byte right_button, float led0_bright, float led1_bright, float led2_bright, float led3_bright,
				float led4_bright, float led5_bright) {

			synchronized (valuesLock) {
				// only keep values if dropCounter == 0
				if(this.enableDownSampling) {
					// create a copy of the drop counter
					int dropCounterTemp = dropCounter;
					// increment dropCounter
					dropCounter = (dropCounter+1) % dropCount;
					// decide whether to keep the sample
					if(dropCounterTemp == 0) {
						// keep the sample
					} else {
						// skip this sample
						return;
					}
				}
				// save off values
				if(values != null) {
					numTotalSamples += 1;
					values[0][valuesIndex] = tacs_volts;
					values[1][valuesIndex] = bnc_in_volts;
					values[2][valuesIndex] = bnc_out_volts;
					values[3][valuesIndex] = (double) left_button;
					values[4][valuesIndex] = (double) right_button;
					values[5][valuesIndex] = led0_bright;
					values[6][valuesIndex] = led1_bright;
					values[7][valuesIndex] = led2_bright;
					values[8][valuesIndex] = led3_bright;
					values[9][valuesIndex] = led4_bright;
					values[10][valuesIndex] = led5_bright;
					valuesIndex = (valuesIndex + 1) % valuesCapacity;
					// publish
					publish((double)tacs_volts);
				}
			}
		}

		@Override
		protected Boolean doInBackground() throws Exception {
			// TODO Auto-generated method stub
			return null;
		}
		
		private void copyCircularToLinear(double circ[], double lin[]) {
			synchronized (valuesLock) {
				// copy the old data to the head
				System.arraycopy(circ, valuesIndex, lin, 0, valuesCapacity - valuesIndex);
				// copy the new data to the tail
				System.arraycopy(circ, 0, lin, valuesCapacity - valuesIndex, valuesIndex);
//	        	System.arraycopy(sourceArray, sourceStartIndex,
//                               targetArray, targetStartIndex,
//                               length);
			}
		}

		@Override
		protected void process(List<Double> chunks) {
			synchronized (valuesLock) {
				if(values != null) {
					// Create local array for newest data
					double[][] mostRecentDataSet = new double[11][valuesCapacity];
					
					// Copy off values
					for(int i=0; i<11; i++) {
						copyCircularToLinear(values[i], mostRecentDataSet[i]);
					}
					
					// Create X axis
					double[] xAxisVals = new double[valuesCapacity];
					if(enableDynamicXAxis.get()) {
						for(int i=0; i<valuesCapacity; i++) {
							xAxisVals[i] = (double)(numTotalSamples-valuesCapacity+i)/sampleRateHz;
						}
					}else if(defaultXAxisVals.get() != null) {
						xAxisVals = defaultXAxisVals.get();
					}else{
						xAxisVals = null;
					}
					
					// Update Charts
					TACSChart.updateXYSeries("TACS", xAxisVals, mostRecentDataSet[0], null);
					TACSChartPanel.repaint();
					
					BNCChart.updateXYSeries("in", xAxisVals, mostRecentDataSet[1], null);
					BNCChart.updateXYSeries("out", xAxisVals, mostRecentDataSet[2], null);
					BNCChartPanel.repaint();
					
					ButtonChart.updateXYSeries("left", xAxisVals, mostRecentDataSet[3], null);
					ButtonChart.updateXYSeries("right", xAxisVals, mostRecentDataSet[4], null);
					ButtonChartPanel.repaint();
					
					LEDChart.updateXYSeries("led 0", xAxisVals, mostRecentDataSet[5], null);
					LEDChart.updateXYSeries("led 1", xAxisVals, mostRecentDataSet[6], null);
					LEDChart.updateXYSeries("led 2", xAxisVals, mostRecentDataSet[7], null);
					LEDChart.updateXYSeries("led 3", xAxisVals, mostRecentDataSet[8], null);
					LEDChart.updateXYSeries("led 4", xAxisVals, mostRecentDataSet[9], null);
					LEDChart.updateXYSeries("led 5", xAxisVals, mostRecentDataSet[10], null);
					LEDChartPanel.repaint();
				}
			}
		}


	}
}