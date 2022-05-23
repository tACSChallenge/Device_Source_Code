import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import java.util.Optional;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JEditorPane;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.SwingConstants;
import javax.swing.SwingUtilities;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.text.DefaultCaret;

import org.knowm.xchart.XYChart;
import com.fazecast.jSerialComm.SerialPort;

import Interface.PacketSerial_Mod.COBSPacketSerial;
import Interface.PacketSerial_Mod.PacketHandlerInterface;
import sun.misc.IOUtils;

import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;


public class TACSChallengeFrame extends JFrame implements PacketHandlerInterface, TCPServerComms.ComSender {
	
	private static final DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss:SSS");
	private static final DateFormat logDateFormat = new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss");
	
	private final long updateTimerRate_ms = 30000; // update ports every 30 seconds
	/* A thread that checks the ports. */
	private final Timer PortUpdateTimer = new Timer(true);
	
	private ChartsManager cm = new ChartsManager();
	private XYChart TACSChart;
	private XYChart BNCChart;
	private XYChart ButtonChart;
	private XYChart LEDChart;
	
    private final Integer defaultXAxisDuration_sec = 4; // seconds
    private final Integer defaultDeviceSampleRate_hz = 1000; // hz
	
	/**
	 * ComboBoxes for Ports and Boards
	 */
	AtomicReference<SerialPort[]> ports = new AtomicReference<SerialPort[]>(null);
	private final JComboBox<SerialPort> portComboBox;

	private final JTextArea cmdArea;
	private final JTextArea statusArea;
	private AtomicBoolean statusAutoScrollEnable = new AtomicBoolean(true);
	
	private SerialPort selectedPort = null;
	private SerialPort openPort = null;
	private final Object comPortLock = new Object();
	
	private final JButton openButton;
	private final JButton loadButton;
	private final JButton saveButton;
	private final JButton abortButton;
	private final JButton sendButton;
	
	private TCPServerComms serverComms = new TCPServerComms();
	private ServerFrame serverFrame = new ServerFrame(this, serverComms);
	
	private StaircaseFrame stairFrame = new StaircaseFrame(this);
	
	private JMenuBar menuBar;
	
	private JTextField logField = null;
	private File logFile = null;
	private final Object logFileLock = new Object();
	
	private String logHeader = "DateTime\t"+
					   "Time[us]\t"+
			           "TACS[V]\t"+
					   "BNCMode\t"+
					   "BNCIn[V]\t"+
					   "BNCOut[V]\t"+
					   "LeftButton\t"+
					   "RightButton\t"+
					   "LED0Bright\t"+
					   "LED1Bright\t"+
					   "LED2Bright\t"+
					   "LED3Bright\t"+
					   "LED4Bright\t"+
					   "LED5Bright";
		
	public void updatePortComboBox() {
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				if(ports.get() == null) {
					return;
				}
				// remove previous from combo box
				portComboBox.removeAllItems();

				// add all serial ports to the combo box
				for (SerialPort port : ports.get()) {
					portComboBox.addItem(port);
				}
				// set selected port
				String prevSelectedPort = Params.prefs.get(Params.PREF_COM_PORT, null);
				if (selectedPort==null && prevSelectedPort!=null) {
					// If there is NOT a port selected and we HAVE a preference saved off,
					// Try and find the port object that matches the preference.
					for (int i=0; i<portComboBox.getItemCount(); i++) {
						SerialPort boardI = portComboBox.getItemAt(i);
						if(prevSelectedPort.equals(boardI.getSystemPortName())) {
							portComboBox.setSelectedIndex(i);
						}
					}
				} else if (selectedPort != null) {
					// There is a port already selected OR there is NO preference.
					for (int i=0; i<portComboBox.getItemCount(); i++) {
						SerialPort boardI = portComboBox.getItemAt(i);
						if(selectedPort.getSystemPortName().equals(boardI.getSystemPortName())) {
							portComboBox.setSelectedIndex(i);
						}
					}
				}
				
			}
		});
	}
	
	public TACSChallengeFrame() {
		
		// CLOSE SAFETY
		this.addWindowListener(new WindowAdapter(){
            public void windowClosing(WindowEvent e){
            	TACSChallengeFrame.this.closeLog();
            	TACSChallengeFrame.this.closeComPort();
            }
        });
		
		JPanel cpane = (JPanel)this.getContentPane();
		cpane.setLayout(new BorderLayout());
		cpane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
	
        // Port DropDown : Select COM Port
        JLabel portLabel = new JLabel("Port:  ");
        portComboBox = new JComboBox<SerialPort>();
        portComboBox.setRenderer(new PortComboBoxRenderer());
        portComboBox.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent itemEvent) {
				int state = itemEvent.getStateChange();
				if (state == ItemEvent.SELECTED && portComboBox.isPopupVisible()) {
					selectedPort = (SerialPort) portComboBox.getSelectedItem();
					// save off port preference
					Params.prefs.put(Params.PREF_COM_PORT, selectedPort.getSystemPortName());
				}
			}
        });
        portComboBox.addPopupMenuListener(new PopupMenuListener() {
			@Override
			public void popupMenuWillBecomeVisible(PopupMenuEvent e) {
				updatePortComboBox();
			}
			@Override
			public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {}
			@Override
			public void popupMenuCanceled(PopupMenuEvent e) {}
        });
                
        // Log: Select TSV File
        JLabel logLabel = new JLabel("Log To:  ");
        logField = new JTextField();
        logField.setColumns(40);
        JButton browseButton = new JButton("Browse Log");
        
        Font boldFont = browseButton.getFont();
        boldFont = boldFont.deriveFont(boldFont.getStyle() | Font.BOLD);
        
        browseButton.setFont(boldFont);
        
        // Open Button
        openButton = new JButton("Open");
        openButton.setFont(boldFont);
        
         // Command Label
        JLabel cmdLabel = new JLabel("Commands:");
        cmdLabel.setHorizontalAlignment(SwingConstants.LEFT);
        
        // Command Help Label
        JButton refButton = new JButton("Cmd Ref.");
//        helpButton.setFont(uploadFont);
        refButton.setPreferredSize(new Dimension(100, 20));
        
        // Load Command Label
        loadButton = new JButton("Load Cmds");
//        loadButton.setFont(uploadFont);
        
        // Save Command Label
        saveButton = new JButton("Save Cmds");
//        saveButton.setFont(uploadFont);
        
        // Abort Command Label
        abortButton = new JButton("Abort Cmds");
//        sendButton.setFont(boldFont);
        
        // Send Command Label
        sendButton = new JButton("Send Cmds");
        sendButton.setFont(boldFont);
        
        // Command Area:
        cmdArea = new JTextArea(0,0);
        cmdArea.setEditable(true);
        cmdArea.setBorder(BorderFactory.createLoweredSoftBevelBorder());
        cmdArea.setEnabled(true);
        
        // Command Scroll:
        JScrollPane cmdScroll = new JScrollPane(cmdArea);
        cmdScroll.setMinimumSize(new Dimension(800,100));
        cmdScroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        cmdScroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        
        // Status Label
        JLabel statusLabel = new JLabel("Status:");
        statusLabel.setHorizontalAlignment(SwingConstants.LEFT);
        
        // Status Auto Scroll
        JCheckBox enableStatusAutoScroll = new JCheckBox ("Auto Scroll");
        enableStatusAutoScroll.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				setStatusAutoScroll( enableStatusAutoScroll.isSelected() );
			}
        });
        
        // Status Button
        JButton clearStatusButton = new JButton("Clear Status");

        // Status Area:
        statusArea = new JTextArea(10,20);
        statusArea.setEditable(false);
        statusArea.setBorder(BorderFactory.createLoweredSoftBevelBorder());
        
        // Set Status Defaults
        enableStatusAutoScroll.setSelected(true);
        setStatusAutoScroll(true);
        
        // Status Area:
        JScrollPane statusScroll = new JScrollPane(statusArea);
        statusScroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        statusScroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        
        // Chart
        cm.start();       

        // Create Constraints
		GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(2, 2, 2, 2);
        
        
        /* CREATE NORTH PANEL */
        JPanel northPanel = new JPanel();
        northPanel.setLayout(new GridBagLayout());
        
        // row 0
        c.gridx = 0;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 1;
        northPanel.add(portLabel,c);
        c.gridx = 1;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 10;
        northPanel.add(portComboBox,c);
        c.gridx = 2;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 1;
        northPanel.add(openButton,c);	
        // row 1
        c.gridx = 0;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        northPanel.add(logLabel,c);
        c.gridx = 1;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 10;
        northPanel.add(logField,c);
        c.gridx = 2;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        northPanel.add(browseButton,c);
        
        /* CREATE CENTER PANEL */
        JPanel centerPanel = new JPanel();
        centerPanel.setLayout(new GridBagLayout());
        
        // Create duration combo box
        Integer[] xAxisDurations = new Integer[] {1,2,3,4,5,10,30,60}; 
        JComboBox<Integer> xAxisDurationComboBox = new JComboBox<Integer>(xAxisDurations);

        JCheckBox dynamicXAxisCheckBox = new JCheckBox ("Enable Dynamic X Axis");
        dynamicXAxisCheckBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				cm.setDynamicXAxis(dynamicXAxisCheckBox.isSelected());
			}
        });
        
        JCheckBox enableDownSamplingCheckBox = new JCheckBox ("Enable Auto Down Sampling (faster plots at lower res)");
        enableDownSamplingCheckBox.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int selectedDur = (int) xAxisDurationComboBox.getSelectedItem();
				cm.setXAxisParameters(selectedDur*defaultDeviceSampleRate_hz,
						defaultDeviceSampleRate_hz,
						enableDownSamplingCheckBox.isSelected());
			}
        });
        
        // continue setting up duration combo box
        xAxisDurationComboBox.addItemListener(new ItemListener() {
			@Override
			public void itemStateChanged(ItemEvent itemEvent) {
				int state = itemEvent.getStateChange();
				if (state == ItemEvent.SELECTED && xAxisDurationComboBox.isPopupVisible()) {
					int selectedDur = (int) xAxisDurationComboBox.getSelectedItem();
					cm.setXAxisParameters(selectedDur*defaultDeviceSampleRate_hz,
							defaultDeviceSampleRate_hz,
							enableDownSamplingCheckBox.isSelected());
				}
			}
        });
        
        // Setup Defaults
        cm.setDynamicXAxis(dynamicXAxisCheckBox.isSelected());
        enableDownSamplingCheckBox.setSelected(true);
        xAxisDurationComboBox.setSelectedItem(defaultXAxisDuration_sec);
        cm.setXAxisParameters(defaultXAxisDuration_sec*defaultDeviceSampleRate_hz,
        		defaultDeviceSampleRate_hz,
        		enableDownSamplingCheckBox.isSelected());
        
        // row 0 - 
        c.gridx = 0;
        c.gridy = 0;
        c.weightx = 40;
        c.gridwidth = 1;
        c.anchor = GridBagConstraints.WEST;
        centerPanel.add(dynamicXAxisCheckBox,c);
        c.gridx = 1;
        c.gridy = 0;
        c.weightx = 40;
        c.gridwidth = 1;
        c.anchor = GridBagConstraints.WEST;
        centerPanel.add(enableDownSamplingCheckBox,c);
        c.gridx = 2;
        c.gridy = 0;
        c.weightx = 1;
        c.gridwidth = 1;
        c.anchor = GridBagConstraints.EAST;
        JLabel xAxisDurationLabel = new JLabel("X Axis Duration [s]:");
        centerPanel.add(xAxisDurationLabel,c);
        c.gridx = 3;
        c.gridy = 0;
        c.weightx = 1;
        c.gridwidth = 1;
        c.anchor = GridBagConstraints.WEST;
        centerPanel.add(xAxisDurationComboBox,c);
        c.anchor = GridBagConstraints.CENTER;
        // row 1
        c.gridx = 0;
        c.gridy = 1;
        c.gridwidth = 4;
        c.weighty = Integer.MAX_VALUE;
        c.fill = GridBagConstraints.BOTH;
        centerPanel.add(cm.TACSChartPanel,c);	
        // row 2
        c.gridx = 0;
        c.gridy = 2;
        c.gridwidth = 4;
        c.weighty = Integer.MAX_VALUE;
        c.fill = GridBagConstraints.BOTH;
        centerPanel.add(cm.LEDChartPanel,c);
        // row 3
        c.gridx = 0;
        c.gridy = 3;
        c.gridwidth = 4;
        c.weighty = Integer.MAX_VALUE;
        c.fill = GridBagConstraints.BOTH;
        centerPanel.add(cm.ButtonChartPanel,c);
        // row 4
        c.gridx = 0;
        c.gridy = 4;
        c.gridwidth = 4;
        c.weighty = Integer.MAX_VALUE;
        c.fill = GridBagConstraints.BOTH;
        centerPanel.add(cm.BNCChartPanel,c);
              
        
        
        /* CREATE EAST PANEL */
        JPanel eastPanel = new JPanel();
        eastPanel.setLayout(new GridBagLayout());
        eastPanel.setPreferredSize(new Dimension(400,100));

        // row 1
        c.gridx = 0;
        c.gridy = 0;
        c.weighty = 1;
        c.gridwidth = 1;
        c.gridheight = 1;
        c.fill = GridBagConstraints.HORIZONTAL;
        eastPanel.add(cmdLabel,c);	
        c.gridx = 1;
        c.gridy = 0;
        c.gridwidth = 1;
        c.gridheight = 1;
        eastPanel.add(refButton,c);
        c.gridx = 2;
        c.gridy = 0;
        c.gridwidth = 1;
        c.gridheight = 1;
        eastPanel.add(abortButton,c);
        // row 2
        c.gridx = 0;
        c.gridy = 1;
        c.gridwidth = 1;
        c.gridheight = 1;
        eastPanel.add(loadButton,c);
        c.gridx = 1;
        c.gridy = 1;
        c.gridwidth = 1;
        c.gridheight = 1;
        eastPanel.add(saveButton,c);
        c.gridx = 2;
        c.gridy = 1;
        c.gridwidth = 1;
        c.gridheight = 1;
        eastPanel.add(sendButton,c);
        // row 3
        c.gridx = 0;
        c.gridy = 2;
        c.gridwidth = 3;
        c.gridheight = 1;
        c.weighty = Integer.MAX_VALUE;
        c.fill = GridBagConstraints.BOTH;
        eastPanel.add(cmdScroll,c);	
        
        /* CREATE SOUTH PANEL */
        JPanel southPanel = new JPanel();
        southPanel.setLayout(new GridBagLayout());
        southPanel.setPreferredSize(new Dimension(100,200)); // the height affects layout.
        
	    // row 0
	    c.gridx = 0;
	    c.gridy = 0;
	    c.gridwidth = 1;
	    c.weightx = 20;
	    c.weighty = 1;
	    c.fill = GridBagConstraints.HORIZONTAL;
	    southPanel.add(statusLabel,c);
	    c.gridx = 1;
	    c.gridy = 0;
	    c.gridwidth = 1;
	    c.weightx = 1;
	    c.weighty = 1;
	    c.fill = GridBagConstraints.HORIZONTAL;
	    southPanel.add(enableStatusAutoScroll,c);
	    c.gridx = 2;
	    c.gridy = 0;
	    c.gridwidth = 1;
	    c.weightx = 1;
	    c.weighty = 1;
	    c.fill = GridBagConstraints.HORIZONTAL;
	    southPanel.add(clearStatusButton,c);
	    // row 1
	    c.gridx = 0;
	    c.gridy = 1;
	    c.gridwidth = 3;
	    c.weightx = 1;
	    c.weighty = 20;
	    c.fill = GridBagConstraints.BOTH;
	    southPanel.add(statusScroll,c);
        
        // Add everything to the content panel
        
        cpane.add(northPanel,BorderLayout.NORTH);
        cpane.add(centerPanel,BorderLayout.CENTER);
        cpane.add(southPanel,BorderLayout.SOUTH);
        cpane.add(eastPanel,BorderLayout.EAST);


        // Setup Firmware Selection
    	String prevFirmware = Params.prefs.get(Params.PREF_LOG, null);
    	
    	// Setup the Firmware Text Field
    	logField.setText(prevFirmware);

        // Setup the File Chooser
    	final JFileChooser fc = new JFileChooser();
    	fc.setCurrentDirectory(prevFirmware == null ? null : (new File(prevFirmware)).toPath().getParent().toFile());
    	fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
    	fc.setDialogTitle("Choose Base Log File Path");
    	
    	// Setup Command Script Selection
    	String prevScript = Params.prefs.get(Params.PREF_CMDS, null);
    	
        // Setup the File Chooser
    	final JFileChooser fc_cmd = new JFileChooser();
    	fc_cmd.setCurrentDirectory(prevScript == null ? null : (new File(prevScript)).toPath().getParent().toFile());
    	fc_cmd.setFileSelectionMode(JFileChooser.FILES_ONLY);
    	FileFilter filter_cmd = new FileNameExtensionFilter("Text Script (.txt .csv)","txt","csv");
    	fc_cmd.addChoosableFileFilter(filter_cmd);
    	fc_cmd.setFileFilter(filter_cmd);
    	fc_cmd.setDialogTitle("Command Script");
 
        
    	browseButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int returnValue = fc.showOpenDialog(TACSChallengeFrame.this);
				if (returnValue == JFileChooser.APPROVE_OPTION) {
					File selectedPath = fc.getSelectedFile();
					TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Selecting log path: "+selectedPath.getAbsolutePath());
					
					Params.prefs.put(Params.PREF_LOG, selectedPath.getAbsolutePath());
					logField.setText(selectedPath.getAbsolutePath());
				}
			}
        });
    	
		
		openButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				if(openButton.getText() == "Open") {
					new Thread() {
						public void run() {
							openButton.setEnabled(false);
							openComPort();
							openButton.setEnabled(true);
						}
					}.start();
				}else {
					new Thread() {
						public void run() {
							openButton.setEnabled(false);
							closeComPort();
							openButton.setEnabled(true);
						}
					}.start();
				}
			}
		});
		
		loadButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int returnValue = fc_cmd.showOpenDialog(TACSChallengeFrame.this);
				if (returnValue == JFileChooser.APPROVE_OPTION) {
					File selectedFile = fc_cmd.getSelectedFile();
					Path selectedPath = selectedFile.getAbsoluteFile().toPath();
					TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Loading command file: "+selectedPath.toString());
					Params.prefs.put(Params.PREF_CMDS, selectedFile.getAbsolutePath());
					
					String contents = "";
					try {
						contents = new String(Files.readAllBytes(selectedPath));
						cmdArea.setText(contents);
					} catch (IOException e1) {
						TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to open command file.");
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
				}
			}
		});
		
		saveButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				int returnValue = fc_cmd.showSaveDialog(TACSChallengeFrame.this);
				if (returnValue == JFileChooser.APPROVE_OPTION) {
					File selectedFile = fc_cmd.getSelectedFile();
					Path selectedPath = selectedFile.getAbsoluteFile().toPath();
					Params.prefs.put(Params.PREF_CMDS, selectedFile.getAbsolutePath());
					
					String cmdString = cmdArea.getText().replace("\r", "");
					try {
						Files.write(selectedPath, cmdString.getBytes());
					} catch (IOException e1) {
						TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to save command file.");
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
				}
			}
		});
		
		sendButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				String cmds = cmdArea.getText().replace("\r", "");
				sendCmds(cmds,false);
			}
		});
		
		abortButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				abortEverything(false);
			}
		});
		
		
		String cmdref_str_temp = "";
		try {
			InputStream msg_is = TACSChallenge.class.getClassLoader().getResourceAsStream("CmdRef.html");
			
			final int bufferSize = 1024;
			final char[] buffer = new char[bufferSize];
			final StringBuilder out = new StringBuilder();
			Reader in = new InputStreamReader(msg_is, StandardCharsets.US_ASCII);
			int charsRead;
			while((charsRead = in.read(buffer, 0, buffer.length)) > 0) {
			    out.append(buffer, 0, charsRead);
			}
			cmdref_str_temp = out.toString();
			
			msg_is.close();
		} catch(Exception ex) {
			ex.printStackTrace();
		}
		final String cmdref_str = cmdref_str_temp;
		
		refButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {

				
				JEditorPane ep = new JEditorPane();
				ep.setEditable(false);
				ep.setContentType("text/html");
				ep.setText(cmdref_str);

				JScrollPane scrollPane = new JScrollPane();
				scrollPane.getViewport().add(ep);
				scrollPane.setMaximumSize(new Dimension(500,500));
				
//		        JOptionPane.showConfirmDialog(null,
//		        		scrollPane,
//                        "Commands Listing : ",
//                        JOptionPane.CLOSED_OPTION,
//                        JOptionPane.PLAIN_MESSAGE);
				
		        JOptionPane pane = new JOptionPane(
		        		scrollPane,
                        JOptionPane.CLOSED_OPTION,
                        JOptionPane.PLAIN_MESSAGE);
				JDialog dialog = pane.createDialog(null,  "Commands Listing : ");
				
		        
		        dialog.setSize(new Dimension(700,700));

				Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
				dialog.setLocation(dim.width/2-dialog.getSize().width/2, dim.height/2-dialog.getSize().height/2);
				
		        dialog.setVisible(true);
			}
		});
		
		clearStatusButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				SwingUtilities.invokeLater(new Runnable() {
					@Override
					public void run() {
						statusArea.setText("");
					}
				});
			}
		});
    	
		PortUpdateTimer.scheduleAtFixedRate(new TimerTask() {
			@Override
			public void run() {
				SerialPort[] prevPorts = ports.get();
				ports.set( SerialPort.getCommPorts() );
				if(prevPorts==null) {
					updatePortComboBox();
				}
				
			}
		},0,updateTimerRate_ms);

		enableCommandPanel(false);
		
		this.getRootPane().registerKeyboardAction(saveButton.getActionListeners()[0], KeyStroke.getKeyStroke(KeyEvent.VK_S, KeyEvent.CTRL_DOWN_MASK), JComponent.WHEN_IN_FOCUSED_WINDOW);
	
		serverComms.setComSender(this);
		
		/*
		 * SETUP MENU
		 */
	
		menuBar = new JMenuBar();
		this.setJMenuBar(menuBar);
		
		JMenu configMenu = new JMenu("Configuration");
		menuBar.add(configMenu);
		
		JMenuItem serverMenuItem = new JMenuItem("Config. Server",
                KeyEvent.VK_T);
		serverMenuItem.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
				serverFrame.setLocation(dim.width/2-serverFrame.getSize().width/2, dim.height/2-serverFrame.getSize().height/2);
				
				serverFrame.setVisible(true);
				serverFrame.setResizable(false);
			}
		});
		configMenu.add(serverMenuItem);
		
		JMenuItem staircaseMenuItem = new JMenuItem("Staircase",
                KeyEvent.VK_T);
		staircaseMenuItem.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
				stairFrame.setLocation(dim.width/2-stairFrame.getSize().width/2, dim.height/2-stairFrame.getSize().height/2);
				
				stairFrame.setVisible(true);
				stairFrame.setResizable(false);
			}
		});
		configMenu.add(staircaseMenuItem);
		

	}
	
	public void abortEverything(final boolean wait) {
		stopWaitingForCmdDone(wait);
		String cmds = "stop_stim\n stop_stair\n wled,0,0\n wled,1,0\n wled,2,0\n wled,3,0\n wled,4,0\n wled,5,0\n wbnc,0\n log,close\n";
		Thread watchDog = new Thread() {
			public void run() {
				TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Waiting 5 seconds for execution of abort commands...");
				try {
					// wait for 5 seconds
					for(int i=5; i>0; i--) {
						TACSChallenge.log(TACSChallenge.LogLevel.INFO, i+" seconds");
						Thread.sleep(1000);
					}
				} catch (InterruptedException e) {
					// Ignore this interrupt, as this is an expected way to terminate this thread.
				}
				TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Abort failed (Device may not be off/idle)");
				stopWaitingForCmdDone(wait);
			}
		};
		if(wait) {
			// start watch dog
			watchDog.start();
			sendCmds(cmds, wait);
			// terminate watch dog
			watchDog.interrupt();
			try {
				watchDog.join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				//e.printStackTrace();
			}
		}else {
			sendCmds(cmds, wait);
		}
	}
	
	public void enableCommandPanel(boolean enable) {
		sendButton.setEnabled(enable);
		abortButton.setEnabled(enable);
	}
	
	String getString(ByteBuffer buf) {
		byte size = buf.get();
		StringBuffer sb = new StringBuffer();
		for(int s = 0; s<size; s++) {
			sb.append((char)buf.get());
		}
		return sb.toString();
	}

	public void handleDataPacket(ByteBuffer wrapped) {
		long currms         = wrapped.getLong();   // unsigned long, currms
		float tacs_volts    = wrapped.getFloat(); // float, tacs_volts
		byte bnc_mode       = wrapped.get();      // byte/bool, bnc_mode
		float bnc_in_volts  = wrapped.getFloat(); // float, bnc_in_volts
		float bnc_out_volts = wrapped.getFloat(); // float, bnc_out_volts
		byte left_button    = wrapped.get();      // byte/bool, left_button
		byte right_button   = wrapped.get();      // byte/bool, right_button
		
		float led0_bright   = wrapped.getFloat(); // float, led0_bright
		float led1_bright   = wrapped.getFloat(); // float, led1_bright
		float led2_bright   = wrapped.getFloat(); // float, led2_bright
		float led3_bright   = wrapped.getFloat(); // float, led3_bright
		float led4_bright   = wrapped.getFloat(); // float, led4_bright
		float led5_bright   = wrapped.getFloat(); // float, led5_bright
		
		Date date = new Date();
		
		String delim = "\t";
		String msg = dateFormat.format(date) + delim +
				     currms + delim +
				     tacs_volts + delim +
					 bnc_mode + delim +
					 bnc_in_volts + delim +
					 bnc_out_volts + delim +
					 left_button + delim +
					 right_button + delim +
					 led0_bright + delim +
					 led1_bright + delim +
					 led2_bright + delim +
					 led3_bright + delim +
					 led4_bright + delim +
					 led5_bright;

		cm.worker.addData(tacs_volts, bnc_in_volts, bnc_out_volts, left_button, right_button, 
				led0_bright, led1_bright, led2_bright, led3_bright, led4_bright, led5_bright);
		
	    log(msg+"\n");
	    
	    serverComms.data2server(msg+"\n");
	}
	
	public void handleLogPacket(ByteBuffer wrapped) {

		// ToDO: handle log command
	}
	
	public void handleStairPacket(ByteBuffer wrapped) {
		float intensity = wrapped.getFloat();
		byte response  = wrapped.get();
		
		String msg = "DEVICE STAIRCASE TRIAL: "+intensity +","+response;
		TACSChallenge.println(msg);
		serverComms.command2server(msg+"\n");
		
		stairFrame.addData(intensity, response);
	}
	
	public void handleCmdEcho(ByteBuffer wrapped) {
		String echo = getString(wrapped);
		String msg = "DEVICE RECEIVED: \""+echo+"\"";
		TACSChallenge.println(msg);
		serverComms.command2server(msg+"\n");
		
		// skip null echos
		if(echo==null) {
			return;
		}
		echo = echo.trim();
		// skip empty echos and returned comments
		if(echo.isEmpty() || echo.startsWith("%")) {
			return;
		}
		// Split the echo
		String[] split_echo = echo.split(",");
		String echo_cmd = split_echo[0];
		// Start processing echos
		if (echo_cmd.startsWith("log")) {
			if(split_echo[1].trim().equalsIgnoreCase("open")) {
				if( split_echo.length==3 ) {
					// Clean up log path string
	 				Path logPath = null;
					String logPathString = logField.getText();
					if(logPathString==null || logPathString.isEmpty()) {
						// Create a downlads path
						String home = System.getProperty("user.home");
						logPath =  Paths.get(home,"Downloads");
					}else {
						logPath = Paths.get(logPathString);
					}
					// Clean up log file string
					String fileName = split_echo[2];
					if(fileName == null || fileName.isEmpty()){
						fileName = "log";
					}
					// Create log file when port opened
					Path totalLogPath = Paths.get(logPath.toString(), fileName);
					logFile = createLogFile( totalLogPath.toFile() );
					openLog(logFile);
				}else {
					TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Malfored log command received");
					TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Emergency log being opened at: " + logFile.getAbsolutePath());
				}
			} else if(split_echo[1].trim().equalsIgnoreCase("close")) {
				if( split_echo.length==2 ) {
					closeLog();
				} else {
					TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Malfored log command received");
				}
			}
		}else if(echo_cmd.startsWith("start_stair")) {
			stairFrame.clear();
			stairFrame.setCommandField(echo);
		}else {
			// unrecognized echo
		}
	}
	
	public void handleCmdDone(ByteBuffer wrapped) {
		byte err_code = wrapped.get();
		byte cmd_code = wrapped.get();
		
		// create command string
		CommandTypes.Command cmd_obj = CommandTypes.getCommand(cmd_code);
		String cmd_str = (cmd_obj==null) ? "unknown_cmd("+"cmd_code"+")" : cmd_obj.name;

		// create error string
		String err_str = "";
		switch(err_code) {
		case 0: err_str = "success"; break;
		case 1: err_str = "aborted"; break;
		case 2: err_str = "parse_fail"; break;
		case 3: err_str = "max num variables"; break;
		case 4: err_str = "variable does not exist"; break;
		default: err_str = "unknown"; break;
		}
		String msg = "DEVICE COMPLETED: " + cmd_str + ", " + err_str ;
		TACSChallenge.println(msg);
		serverComms.command2server(msg+"\n");
		
		// TODO: unblock script execution
		notifyCmdDone(err_code, cmd_code);
	}
	
	public void handleVarValue(ByteBuffer wrapped) {
		String name = getString(wrapped);
		String value = getString(wrapped);
		
		String msg = "DEVICE ADDED VARIABLE: " + name + ", " + value ;
		TACSChallenge.println(msg);
		serverComms.command2server(msg+"\n");
	}
	
	@Override
	public void onPacketFunction(byte[] buffer, int size) {
		if(size==0) {
			return;
		}
		ByteBuffer wrapped = ByteBuffer.wrap(buffer);
		wrapped.order(ByteOrder.LITTLE_ENDIAN);
		
		byte return_code = 0;
		try {
			String msg = "";
			return_code = wrapped.get(); // msg type
			switch(return_code) {
			case 0: //RC_NONE
				break;
			case 1: //RC_ERROR
				msg = "DEVICE ERROR: "+getString(wrapped);
				TACSChallenge.println(msg);
				serverComms.command2server(msg+"\n");
				break;
			case 2: //RC_WARN
				msg = "DEVICE WARN: "+getString(wrapped);
				TACSChallenge.println(msg);
				serverComms.command2server(msg+"\n");
				break;
			case 3: //RC_INFO
				msg = "DEVICE INFO: "+getString(wrapped);
				TACSChallenge.println(msg);
				serverComms.command2server(msg+"\n");
				break;
			case 4: //RC_DEBUG
				msg = "DEVICE DEBUG: "+getString(wrapped);
				TACSChallenge.println(msg);
				serverComms.command2server(msg+"\n");
				break;
			case 5: //RC_VERBOSE
				msg = "DEVICE VERBOSE: "+getString(wrapped);
				TACSChallenge.println(msg);
				serverComms.command2server(msg+"\n");
				break;
			case 6: //RC_CMD_ECHO
				handleCmdEcho(wrapped);
				break;
			case 7: //RC_CMD_DONE
				handleCmdDone(wrapped);
				break;
			case 8: //RC_DATA
				handleDataPacket(wrapped);
				break;
			case 9: //RC_LOG
				// ToDo: Do nothing?
				break;
			case 10: //RC_STAIR_RESP
				handleStairPacket(wrapped);
				break;
			case 11: //RC_VAR_VALUE
				handleVarValue(wrapped);
				break;
			}
			
		}catch(BufferUnderflowException  e) {
			System.out.println("onPacketFunction return_code:"+ return_code + " size:" + size);
			System.err.println("onPacketFunction: "+ e);
		}
	}
	
	private FileWriter fw = null;
	private BufferedWriter bw = null;
	
	public Optional<String> getFileExtension(String filename) {
		return Optional.ofNullable(filename).filter(f -> f.contains(".")).map(f -> f.substring(filename.lastIndexOf(".") + 1));
	}
	
	public File createLogFile(File file) {
		String finalext = "tsv";
		Date date = new Date();
		String filepath = file.getParent();
		String filename = file.getName().replaceFirst("[.][^.]+$", "");;
		String datestr = logDateFormat.format(date);
		String finalpath = filepath+File.separator+filename+"_"+datestr+"."+finalext;
		File finalFile = new File(finalpath);
		return finalFile;
	}
	
	public boolean openLog(File file) {
		synchronized(logFileLock) {
			// get the parent directory:
			File parent = file.getParentFile();
			if(!parent.exists()) {
				// parent directory doesn't exist, so create it.
				boolean mkdir_success = false;
				try {
					mkdir_success = file.getParentFile().mkdirs();
				} catch (SecurityException e) {
					e.printStackTrace();
				}
				if(!mkdir_success) {
					TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to create path to log: "+file.getAbsolutePath());
					closeLog();
					return false;
				}
			}
			// create the file writer
			try {
				fw = new FileWriter(file);
				bw = new BufferedWriter(fw);
				bw.write(logHeader+"\n");
				TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Opening log: "+file.getAbsolutePath());
				return true;
			} catch (IOException e) {
				TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to open log "+file.getAbsolutePath());
				closeLog();
				return false;
			}
		}
	}
	
	public boolean log(String msg) {
		synchronized(logFileLock) {
			if(bw!=null) {
				try {
					bw.write(msg);
					return true;
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			return false;
		}
	}
	
	public void closeLog() {
		synchronized(logFileLock) {
			TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Closing Log");
			if(bw!=null) {
				try {
					bw.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			if(fw!=null) {
				try {
					fw.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			bw = null;
			fw = null;
			logFile = null;
		}
	}
	
	public void setStatusAutoScroll(boolean scroll) {
		statusAutoScrollEnable.set(scroll);
	}
	
	public void appendStatusText(String msg) {
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				statusArea.append(msg);
				// change caret update style
		        DefaultCaret caret = (DefaultCaret) statusArea.getCaret();
				if(statusAutoScrollEnable.get()) {
					int len = statusArea.getDocument().getLength();
					statusArea.setCaretPosition(len);
			        caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);
				}else {
					caret.setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
				}
			}
		});
	}
	
	private void openComPort() {
		synchronized(comPortLock) {
			selectedPort = (SerialPort) portComboBox.getSelectedItem();
			if(selectedPort==null) {
				TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "No serial port selected" );
				return;
			}
			
			COBSPacketSerial ps = new COBSPacketSerial();
			ps.setPacketHandler(TACSChallengeFrame.this);
			boolean success = ps.begin(selectedPort,921600); // increase baud rate to 921600 from 115200
			if(success) {
				openPort = selectedPort;
				TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Opening serial port: "+ openPort.getSystemPortName() );
				// switch open button text to close
				openButton.setText("Close");
				// enable command panel
				enableCommandPanel(true);
			}else {
				TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to open serial port: "+ selectedPort.getSystemPortName() );
			}
		}
	}
	
	private void closeComPort() {
		// abort everything
		abortEverything(true);
		synchronized(comPortLock) {
			if(openPort==null) {
				return;
			}
			TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Closing serial port: "+ openPort.getSystemPortName() );
			// if there is a running log, close it.
			closeLog();
			openPort.closePort();
			openPort = null;
			enableCommandPanel(false);
			// switch open button text to open
			openButton.setText("Open");
		}
	}
	
	public void sendCmds(String cmds, boolean wait) {
		if(cmds==null) {
			return;
		}
		TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Sending Commands..." );
		// Disable the send button
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				sendButton.setEnabled(false);
			}
		});
		
		final ConcurrentLinkedQueue<String> queuedCmds = new ConcurrentLinkedQueue<String>();
		for(String cmd: cmds.split("\n")) {
			queuedCmds.add(cmd);
		}
		final Thread thread = new Thread() {
			@Override
			public void run() {
				synchronized(comPortLock) {
					if(openPort==null) {
						TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "No Serial Port Selected" );
						return;
					}
					Iterator<String> it = queuedCmds.iterator();
					script_loop:
					while(it.hasNext()) {
						String cmd = it.next().trim();
						// skip comments
						if(cmd.isEmpty() || cmd.charAt(0) == '%') {
							continue;
						}
						String cmd_returned = cmd+"\n";
						TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Sending \""+cmd+"\"" );
						if(openPort.isOpen()) {
							openPort.writeBytes(cmd_returned.getBytes(),cmd_returned.length());
							// wait for that type of command to finish
							CommandAwaitingStatus waitForCmdDone = waitForCmdDone(cmd, this);
							switch(waitForCmdDone) {
								case Malformed:
									TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Malformed command: "+cmd);
									break script_loop;
								case Aborted:
									TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Aborted command: "+cmd);
									break script_loop;
								case Done:
//									TACSChallenge.log(TACSChallenge.LogLevel.INFO, "");
									continue script_loop;
							}
							// delay 1 milliseconds so the device can process the command.
							try {
								Thread.sleep(1);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
						}
					}
				}
				// Enable the send button.
				SwingUtilities.invokeLater(new Runnable() {
					@Override
					public void run() {
						sendButton.setEnabled(true);
					}
				});
				TACSChallenge.log(TACSChallenge.LogLevel.INFO, "Done Sending Commands." );
			}
		};
		thread.start();
		if(wait) {
			try {
				thread.join();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	

	public static enum CommandAwaitingStatus {Malformed, Done, Aborted};
	
	private class CommandAwaitingCompletion {
		
		public int cmd_code = 0;
		public String cmd = "";
		public AtomicBoolean abort = new AtomicBoolean(false);
		public AtomicBoolean done = new AtomicBoolean(false);
		public Thread thread = null;
		
		public CommandAwaitingCompletion(String sentCmd, Thread thread) throws Exception {
			if(sentCmd == null) { 
				throw new Exception("command is null");
			}
			this.cmd = sentCmd;
			this.thread = thread;
			// split the command
			String[] cmd_split = sentCmd.split(",");
			if(cmd_split.length<1) {
				throw new Exception("command is empty");
			}
			String cmd_name = cmd_split[0].trim().toLowerCase();
			if(cmd_name == null || cmd_name.isEmpty() ) {
				throw new Exception("command is null");
			}
			CommandTypes.Command cmd_obj = CommandTypes.getCommand(cmd_name);
			if(cmd_obj == null) {
				throw new Exception("cannot find command");
			}
			cmd_code = cmd_obj.code;
		}
	}
	
	private final ConcurrentLinkedQueue<CommandAwaitingCompletion> waitingCmdQ = new ConcurrentLinkedQueue<CommandAwaitingCompletion>();
	
	public CommandAwaitingStatus waitForCmdDone(String sentCmd, Thread thread) {
		CommandAwaitingCompletion waitingCmd = null;
		try {
			waitingCmd = new CommandAwaitingCompletion(sentCmd, thread);
		} catch (Exception e) {
			e.printStackTrace();
		}
		if(waitingCmd == null) {
			return CommandAwaitingStatus.Malformed;
		}
		waitingCmdQ.add(waitingCmd);
		synchronized( waitingCmd ) {
			while (true) {
				// check for done
				if ( waitingCmd.done.get() ) {
					waitingCmdQ.remove(waitingCmd);
					return CommandAwaitingStatus.Done;
				}
				// check for abort
				if ( waitingCmd.abort.get() ) {
					waitingCmdQ.remove(waitingCmd);
					return CommandAwaitingStatus.Aborted;
				}
				// wait
				try {
					waitingCmd.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	public void stopWaitingForCmdDone(boolean wait) {
		Iterator<CommandAwaitingCompletion> it = waitingCmdQ.iterator();
		while(it.hasNext()) {
			CommandAwaitingCompletion waitingCmd = it.next();
			// notify the command of an abort.
			synchronized( waitingCmd ) {
				waitingCmd.abort.set(true);
				waitingCmd.notifyAll();
			}
			// wait for the sendCmd() thread to terminate.
			if(wait && waitingCmd.thread != null) {
				try {
					waitingCmd.thread.join();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
	}
	
	public void notifyCmdDone(int err_code, int cmd_code) {
		Iterator<CommandAwaitingCompletion> it = waitingCmdQ.iterator();
		while (it.hasNext()) {
			CommandAwaitingCompletion waitingCmd = it.next();
			if (waitingCmd.cmd_code != cmd_code) {
				continue;
			}
			// notify the command that it is done.
			synchronized (waitingCmd) {
				waitingCmd.done.set(true);
				waitingCmd.notifyAll();
			}
			return;
		}
	}

	@Override
	public void send(String msg) {
		sendCmds(msg, false);
	}
	
}
