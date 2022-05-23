import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.IOException;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;


public class ServerFrame extends JDialog {
	
	private TCPServerComms serverComms;
	
	public ServerFrame(JFrame owner, TCPServerComms serverComms) {
		super(owner,"Server Configuration",true);
		this.serverComms = serverComms;
		
		JLabel dataPortLabel = new JLabel("Data TCP/IP Port:");
		JLabel cmdPortLabel = new JLabel("Command TCP/IP Port:");
		
		JButton connectButton = new JButton("Connect");
		JButton disconnectButton = new JButton("Disconnect");
		
		JTextField dataPortTextField = new JTextField();
		dataPortTextField.setColumns(10);
		dataPortTextField.setText( Params.prefs.get(Params.PREF_DATA_TCP_PORT, "") );
		
		JTextField cmdPortTextField = new JTextField();
		cmdPortTextField.setColumns(10);
		cmdPortTextField.setText( Params.prefs.get(Params.PREF_CMD_TCP_PORT, "") );
		
		JPanel cpane = (JPanel)this.getContentPane();
		cpane.setLayout(new GridBagLayout());
		cpane.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
		
		GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(2, 2, 2, 2);
        
        // row 0
        c.gridx = 0;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 1;
        cpane.add(dataPortLabel,c);
        c.gridx = 1;
        c.gridy = 0;
        c.gridwidth = 1;
        c.weightx = 10;
		cpane.add(dataPortTextField,c);
		// row 1
        c.gridx = 0;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 1;
        cpane.add(cmdPortLabel,c);
        c.gridx = 1;
        c.gridy = 1;
        c.gridwidth = 1;
        c.weightx = 10;
		cpane.add(cmdPortTextField,c);
		// row 0 and 1
        c.gridx = 2;
        c.gridy = 0;
        c.gridwidth = 1;
        c.gridheight = 2;
        c.weightx = 1;
        c.fill = GridBagConstraints.BOTH;
		cpane.add(connectButton,c);
        c.gridx = 3;
        c.gridy = 0;
        c.gridwidth = 1;
        c.gridheight = 2;
        c.weightx = 1;
        c.fill = GridBagConstraints.BOTH;
		cpane.add(disconnectButton,c);
		
		this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
		this.pack();
		
		connectButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				// Parse the Port
				String dataPortString = dataPortTextField.getText();
				String cmdPortString = cmdPortTextField.getText();
				int dataPort = 0;
				int cmdPort = 0;
				try {
					dataPort = Integer.parseInt(dataPortString);
					cmdPort = Integer.parseInt(cmdPortString);
					if(dataPort==cmdPort) {
						TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Data and Command port cannot be the same.");
						return;
					}
				}catch(NumberFormatException ex) {
					TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to parse TCP/IP port.");
					return;
				}
				// Start the server
				try {
					// save the preference
					 Params.prefs.put(Params.PREF_DATA_TCP_PORT, dataPortString); 
					 Params.prefs.put(Params.PREF_CMD_TCP_PORT, cmdPortString);
					// start the server
					serverComms.start(dataPort, cmdPort);
					dataPortTextField.setEnabled(false);
					cmdPortTextField.setEnabled(false);
					connectButton.setEnabled(false);
				} catch (IOException ex) {
					TACSChallenge.log(TACSChallenge.LogLevel.ERROR, "Failed to connect to TCP/IP port.");
					ex.printStackTrace();
				}
			}
		});
		
		disconnectButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				serverComms.stop();
				dataPortTextField.setEnabled(true);
				cmdPortTextField.setEnabled(true);
				connectButton.setEnabled(true);
			}
		});
		
		this.addWindowListener(new WindowAdapter() {
			@Override
			public void windowActivated(WindowEvent e) {
				// ToDo: Check if clients are still connected
			}
			@Override
			public void windowDeactivated(WindowEvent e) {
				
			}
		});
		
	}
	
	
}

