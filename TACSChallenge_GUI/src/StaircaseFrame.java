import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.knowm.xchart.XChartPanel;
import org.knowm.xchart.XYChart;
import org.knowm.xchart.XYSeries;
import org.knowm.xchart.style.markers.None;
import org.knowm.xchart.style.markers.TriangleDown;
import org.knowm.xchart.style.markers.TriangleUp;

public class StaircaseFrame extends JDialog{
	
	public static final float defaultBaseBright = 0.5f;  //150.0f/255.0f;
	public static final float defaultTargetBright = 0.2f;// 45.0f/255.0f;
	public static final float defaultStepSize = 0.02f;   //10.0f/255.0f;
	
	public static final int defaultNumTrials = 40;
	public static final int defaultTargetLatency_ms = 25;
	public static final int defaultISIMin_ms = 2*1000;
	public static final int defaultISIMax_ms = 4*1000;
	public static final int defaultNumMiss = 1;
	public static final int defaultNumHit = 2;
	
	public static final String hitsSeriesName = "hits";
	public static final String missesSeriesName = "misses";
	public static final String trajectorySeriesName = "trajectory";
	public static final String thresholdSeriesName = "threshold";

	private TACSChallengeFrame owner = null;
	
	private final ArrayList<Double> responses = new ArrayList<Double>();
	private final ArrayList<Double> intensities = new ArrayList<Double>();
	
	private LabeledPlaceholderTextField commandField;
	
	private LabeledPlaceholderTextField baseBrightTF;
	private LabeledPlaceholderTextField targetBrightTF;
	private LabeledPlaceholderTextField stepSizeTF;
	private LabeledPlaceholderTextField numTrialsTF;
	private LabeledPlaceholderTextField targetLatencyTF;
	private LabeledPlaceholderTextField isiMinTF;
	private LabeledPlaceholderTextField isiMaxTF;
	private LabeledPlaceholderTextField numMissTF;
	private LabeledPlaceholderTextField numHitTF;
	
	private JButton startButton = new JButton("Start Staircase");
	private JButton stopButton = new JButton("Abort Staircase");
	
	public XChartPanel<XYChart> stairChartPanel;
	public XYChart stairChart;	

	public StaircaseFrame(TACSChallengeFrame owner) {
		super(owner,"Staircase",true);
		this.owner = owner;
		
		String defaultStairCommand = "start_stair,"+baseBrightTF +","+ targetBrightTF +","+ stepSizeTF +","+ 
		                                          defaultNumTrials +","+ defaultTargetLatency_ms +","+ defaultISIMin_ms +","+ 
				                                  defaultISIMax_ms +","+ defaultNumMiss +","+ defaultNumHit;
		commandField = new LabeledPlaceholderTextField("Stair Command: ",defaultStairCommand,20);
		
		int tfWidth = 5;
		baseBrightTF  = new LabeledPlaceholderTextField("Base Brightness [0-1]: ",""+defaultBaseBright,tfWidth);
		targetBrightTF  = new LabeledPlaceholderTextField("Target Brightness [0-1]: ",""+defaultTargetBright,tfWidth);
		stepSizeTF  = new LabeledPlaceholderTextField("Step Size [0-1]: ",""+defaultStepSize,tfWidth);
		numTrialsTF = new LabeledPlaceholderTextField("Num Trials: ",""+defaultNumTrials,tfWidth);
		targetLatencyTF = new LabeledPlaceholderTextField("Target Latency [ms]: ",""+defaultTargetLatency_ms,tfWidth);
		isiMinTF = new LabeledPlaceholderTextField("Inter Stimulus Interval Min [ms]: ",""+defaultISIMin_ms,tfWidth);
		isiMaxTF = new LabeledPlaceholderTextField("Inter Stimulus Internal Max [ms]: ",""+defaultISIMax_ms,tfWidth);
		numMissTF = new LabeledPlaceholderTextField("Num Miss: ",""+defaultNumMiss,tfWidth);
		numHitTF = new LabeledPlaceholderTextField("Num Hit: ",""+defaultNumHit,tfWidth);
		
		GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(2, 2, 2, 2);
		
		JPanel option1 = new JPanel(new GridBagLayout());
		option1.setBorder(BorderFactory.createTitledBorder("Option 1 (type single-line command)"));
		
        // row 0
        c.gridx = 0;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 1;
        option1.add(commandField,c);
        
		JPanel option2 = new JPanel(new GridBagLayout());
		option2.setBorder(BorderFactory.createTitledBorder("Option 2 (enter each field of the command separately)"));

        // row 0
        c.gridx = 0;
        c.gridy = 0;
        c.gridwidth = 2;
        c.weightx = 1;
        option2.add(baseBrightTF,c);
        c.gridx = 2;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(targetBrightTF,c);
        c.gridx = 3;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(stepSizeTF,c);
        // row 1
        c.gridx = 0;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(numTrialsTF,c);
        c.gridx = 1;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(targetLatencyTF,c);
        c.gridx = 2;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(isiMinTF,c);
        c.gridx = 3;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(isiMaxTF,c);
        c.gridx = 4;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(numMissTF,c);
        c.gridx = 5;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        option2.add(numHitTF,c);
		
        // Setup the chart
        
		// Create TACS Chart
		stairChart = new XYChart(600, 500);
		stairChart.setTitle("Staircase");
		stairChart.setXAxisTitle("Num Trials");
		stairChart.setYAxisTitle("Target Brightness (Added to Base Brightness)");
		stairChart.addSeries(hitsSeriesName, new double[] { 0 }, new double[] { 0 });
		stairChart.addSeries(missesSeriesName, new double[] { 0 }, new double[] { 0 });
		stairChart.addSeries(trajectorySeriesName, new double[] { 0 }, new double[] { 0 });
		stairChart.addSeries(thresholdSeriesName, new double[] { 0 }, new double[] { 0 });
		stairChart.getStyler().setLegendVisible(true);
		stairChart.getStyler().setXAxisTicksVisible(true);
//		stairChart.getStyler().setXAxisDecimalPattern("0");
//		stairChart.getStyler().setYAxisMin(-1.0);
//		stairChart.getStyler().setYAxisMax(1.0);
		Map<String, XYSeries> stairSeries = stairChart.getSeriesMap();
		XYSeries hitsSeries = stairSeries.get(hitsSeriesName);
		XYSeries missesSeries = stairSeries.get(missesSeriesName);
		XYSeries trajectorySeries = stairSeries.get(trajectorySeriesName);
		XYSeries threshSeries = stairSeries.get(thresholdSeriesName);
		
		hitsSeries.setLineWidth(0);
		hitsSeries.setMarker(new TriangleDown());
		hitsSeries.setLineStyle(new BasicStroke(0));
		hitsSeries.setMarkerColor(Color.green);
		hitsSeries.setLineColor(Color.green);
		missesSeries.setLineWidth(0);
		missesSeries.setMarker(new TriangleUp());
		missesSeries.setLineStyle(new BasicStroke(0));
		missesSeries.setMarkerColor(Color.red);
		missesSeries.setLineColor(Color.red);
		trajectorySeries.setMarker(new None());
		trajectorySeries.setLineColor(Color.black);
		threshSeries.setMarker(new None());
		threshSeries.setLineColor(Color.blue);
		
		// Show it
		stairChartPanel = new XChartPanel<XYChart>(stairChart);
        
		// Setup the content panel
		JPanel cpane = (JPanel)this.getContentPane();
		cpane.setLayout(new GridBagLayout());
		cpane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

        Font currFont = startButton.getFont();
        
        Font bigFont = currFont.deriveFont(currFont.getSize()*1.2f);
        Font bigBoldFont = bigFont.deriveFont(currFont.getStyle() | Font.BOLD);
        startButton.setFont(bigBoldFont);
        stopButton.setFont(bigFont);
		
        // row 0
        c.gridx = 0;
        c.gridy = 0;
        c.gridwidth = 1;
        c.gridheight = 1;
        c.weightx = 1;
        c.fill = GridBagConstraints.HORIZONTAL;
        cpane.add(option1,c);
        // row 1
        c.gridx = 0;
        c.gridy = 1;
        c.gridwidth = 1;
        c.gridheight = 1;
        c.weightx = 1;
        c.fill = GridBagConstraints.HORIZONTAL;
        cpane.add(option2,c);
        // row 0 and row 1
        c.gridx = 1;
        c.gridy = 0;
        c.gridwidth = 1;
        c.gridheight = 1;
        c.weightx = 1;
        c.fill = GridBagConstraints.BOTH;
        cpane.add(startButton,c);
        c.gridx = 1;
        c.gridy = 1;
        c.gridwidth = 1;
        c.gridheight = 1;
        c.weightx = 1;
        c.fill = GridBagConstraints.BOTH;
        cpane.add(stopButton,c);
        // row 2
        c.gridx = 0;
        c.gridy = 2;
        c.gridwidth = 2;
        c.weighty = Integer.MAX_VALUE;
        c.weightx = 1;
        cpane.add(stairChartPanel,c);
        
        
		this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
		this.pack();
		
		// CREATE LISTENERS
		ChangeListener stairCmdListener = new ChangeListener() {
			@Override
			public void stateChanged(ChangeEvent e) {
				if(!commandField.textField.hasFocus()) {
					return;
				}
				String commandText = commandField.textField.getText();
				if(commandText == null) {
					return;
				}
				updateCommandFiledFromComponentFields(commandText);
				String[] splitCommandText = commandText.split(",");
				if(splitCommandText.length>10) {
					TACSChallenge.log(TACSChallenge.LogLevel.WARN, "Too many arguments to \"start_stair\" command.");
					return;
				}
				if(!isCommandFieldNumerical(true)) {
					TACSChallenge.log(TACSChallenge.LogLevel.WARN, "Arguments to \"start_stair\" command must be floats or integers.");
					return;
				}
				if (!isCommandFieldNumerical(false)) {
					return;
				}
				TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Good stair command!");
			}
		};
		commandField.addChangeListener(stairCmdListener);
		
		ChangeListener stairCmdCompListener = new ChangeListener() {
			@Override
			public void stateChanged(ChangeEvent e) {
				if(baseBrightTF.textField.hasFocus() ||
						targetBrightTF.textField.hasFocus() ||
						stepSizeTF.textField.hasFocus() ||
						numTrialsTF.textField.hasFocus() ||
						targetLatencyTF.textField.hasFocus() ||
						isiMinTF.textField.hasFocus() ||
						isiMaxTF.textField.hasFocus() ||
						numMissTF.textField.hasFocus() ||
						numHitTF.textField.hasFocus()) {
					String stairCmd = "start_stair,"+
						baseBrightTF.textField.getText()+","+
						targetBrightTF.textField.getText()+","+
						stepSizeTF.textField.getText()+","+
						numTrialsTF.textField.getText()+","+
						targetLatencyTF.textField.getText()+","+
						isiMinTF.textField.getText()+","+
						isiMaxTF.textField.getText()+","+
						numMissTF.textField.getText()+","+
						numHitTF.textField.getText();				
					commandField.textField.setText(stairCmd);
					if (isCommandFieldNumerical(false)) {
						TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Good stair command!");
					}
				}
			}
		};
		baseBrightTF.addChangeListener(stairCmdCompListener);
		targetBrightTF.addChangeListener(stairCmdCompListener);
		stepSizeTF.addChangeListener(stairCmdCompListener);
		numTrialsTF.addChangeListener(stairCmdCompListener);
		targetLatencyTF.addChangeListener(stairCmdCompListener);
		isiMinTF.addChangeListener(stairCmdCompListener);
		isiMaxTF.addChangeListener(stairCmdCompListener);
		numMissTF.addChangeListener(stairCmdCompListener);
		numHitTF.addChangeListener(stairCmdCompListener);
		
		startButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				if (!isValidStartStairCommand()) {
					TACSChallenge.log(TACSChallenge.LogLevel.WARN, "Stair command is malformed, NOT sent.");
					return;
				}
				// send abort on anything running now
				owner.abortEverything(false);
				// send the stair command
				owner.sendCmds(commandField.textField.getText(),false);
				// clear the chart
				clear();
			}
		});
		
		stopButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				owner.abortEverything(false);
			}
		});

	}
	
	public void setCommandField(String stair_cmd) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
        		commandField.textField.setText(stair_cmd);
        		updateCommandFiledFromComponentFields(stair_cmd);
            }
        });
	}
	
	private void updateCommandFiledFromComponentFields(String stair_cmd) {
		String[] splitCommandText = stair_cmd.split(",");
		baseBrightTF.textField.setText( splitCommandText.length>=2 ? splitCommandText[1] : "" );
		targetBrightTF.textField.setText( splitCommandText.length>=3 ? splitCommandText[2] : "" );
		stepSizeTF.textField.setText( splitCommandText.length>=4 ? splitCommandText[3] : "" );
		numTrialsTF.textField.setText( splitCommandText.length>=5 ? splitCommandText[4] : "" );
		targetLatencyTF.textField.setText( splitCommandText.length>=6 ? splitCommandText[5] : "" );
		isiMinTF.textField.setText( splitCommandText.length>=7 ? splitCommandText[6] : "" );
		isiMaxTF.textField.setText( splitCommandText.length>=8 ? splitCommandText[7] : "" );
		numMissTF.textField.setText( splitCommandText.length>=9 ? splitCommandText[8] : "" );
		numHitTF.textField.setText( splitCommandText.length>=10 ? splitCommandText[9] : "" );
	}
	
	private boolean isValidStartStairCommand() {
		String cmdText = commandField.textField.getText();
		return cmdText !=null && cmdText.startsWith("start_stair") && isCommandFieldNumerical(false);
	}
	
	private boolean isCommandFieldNumerical(boolean ignoreBlanks) {
		boolean isGood = true;
		isGood &= isTextFieldFloat(baseBrightTF.textField, ignoreBlanks);
		isGood &= isTextFieldFloat(targetBrightTF.textField, ignoreBlanks);
		isGood &= isTextFieldFloat(stepSizeTF.textField, ignoreBlanks);
		isGood &= isTextFieldInteger(numTrialsTF.textField, ignoreBlanks);
		isGood &= isTextFieldInteger(targetLatencyTF.textField, ignoreBlanks);
		isGood &= isTextFieldInteger(isiMinTF.textField, ignoreBlanks);
		isGood &= isTextFieldInteger(isiMaxTF.textField, ignoreBlanks);
		isGood &= isTextFieldInteger(numMissTF.textField, ignoreBlanks);
		isGood &= isTextFieldInteger(numHitTF.textField, ignoreBlanks);
		return isGood;
	}
	
	private static boolean isTextFieldFloat(JTextField tf, boolean ignoreBlanks) {
		String text = tf.getText();
		// Run basic checks first
		if(text==null) {
			return false;
		}
		if(text.isEmpty()) {
			return ignoreBlanks;
		}
		// Run parsing check next
		try {
			Float.parseFloat(text);
			return true;
		} catch (NumberFormatException e) {
			return false;
		}
	}
	
	private static boolean isTextFieldInteger(JTextField tf, boolean ignoreBlanks) {
		String text = tf.getText();
		// Run basic checks first
		if(text==null) {
			return false;
		}
		if(text.isEmpty()) {
			return ignoreBlanks;
		}
		// Run parsing check next
		try {
			Integer.parseInt(text);
			return true;
		} catch (NumberFormatException e) {
			return false;
		}
	}
	
	public synchronized void clear() {
		intensities.clear();
		responses.clear();
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				updateChart(intensities, responses);
			}
		});
	}
	
	public synchronized void addData(double intensity, double response) {
		intensities.add(intensity);
		responses.add(response);
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				updateChart(intensities, responses);
			}
		});
	}
	
	public synchronized void updateChart(ArrayList<Double> intensities, ArrayList<Double> responses) {
		// Assume responses and intensities array have the same length
	
		// Create hits and misses data series
		ArrayList<Double> hitsX = new ArrayList<Double>();
		ArrayList<Double> hitsY = new ArrayList<Double>();
		ArrayList<Double> missesX = new ArrayList<Double>();
		ArrayList<Double> missesY = new ArrayList<Double>();
		for(int i=0; i<intensities.size(); i++) {
			if(responses.get(i) > 0) {
				// hits
				hitsX.add((double) (i+1));
				hitsY.add(intensities.get(i));
			}else {
				// misses
				missesX.add((double) (i+1));
				missesY.add(intensities.get(i));
			}
		}
		stairChart.updateXYSeries(hitsSeriesName, 
				hitsX.stream().mapToDouble(d -> d).toArray(), 
				hitsY.stream().mapToDouble(d -> d).toArray(), 
				null);
		stairChart.updateXYSeries(missesSeriesName, 
				missesX.stream().mapToDouble(d -> d).toArray(), 
				missesY.stream().mapToDouble(d -> d).toArray(), 
				null);

		// Create x axis for intensities
		double[] trialsXAxis = new double[intensities.size()];
		for(int i=0; i<intensities.size(); i++) {
			trialsXAxis[i] = i+1;
		}
		stairChart.updateXYSeries(trajectorySeriesName, trialsXAxis, intensities.stream().mapToDouble(d -> d).toArray(), null);
		
		// Create mean intensities chart (average of last 10)
		final int num2Average = 10;
		int count = 0;
		double total = 0;
		for(int i=intensities.size()-1; i>=0; i--) {
			if(count<num2Average) {
				total += intensities.get(i);
				count++;
			}else {
				break;
			}
		}
		double meanIntensities = total/count;
		stairChart.updateXYSeries(thresholdSeriesName, 
				                  new double[] {Math.max(1,intensities.size()-num2Average), intensities.size()}, 
				                  new double[] {meanIntensities, meanIntensities}, 
				                  null);
		
		stairChartPanel.repaint();
	}

	
}
