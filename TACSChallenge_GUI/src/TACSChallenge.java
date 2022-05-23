/**
 * 
 */

import java.awt.Dimension;
import java.awt.Toolkit;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import javax.swing.JFrame;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.text.DefaultCaret;

import com.fazecast.jSerialComm.*;

import Interface.PacketSerial_Mod.COBSPacketSerial;
import Interface.PacketSerial_Mod.PacketHandlerInterface;




/**
 * @author David Wang
 *
 */
public class TACSChallenge {

	private static TACSChallengeFrame frame = null;
	
	public static enum LogLevel {
		VERBOSE,
		DEBUG,
		INFO,
		WARN,
		ERROR
	};

	public static void main(String[] args) {
    	//SingleInstanceChecker.checkIfRunning();
        /* Use an appropriate Look and Feel */
        try {
            UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
            //UIManager.setLookAndFeel("javax.swing.plaf.metal.MetalLookAndFeel");
        } catch (UnsupportedLookAndFeelException ex) {
            ex.printStackTrace();
        } catch (IllegalAccessException ex) {
            ex.printStackTrace();
        } catch (InstantiationException ex) {
            ex.printStackTrace();
        } catch (ClassNotFoundException ex) {
            ex.printStackTrace();
        }
        /* Turn off metal's use of bold fonts */
        UIManager.put("swing.boldMetal", Boolean.FALSE);
        //Schedule a job for the event-dispatching thread:
        //adding TrayIcon.
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
            	createAndShow();
            }
        });
	}
	
	private static void createAndShow() {
		frame = new TACSChallengeFrame();
		frame.setTitle("TACS Challenge Controller v1.4");
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		frame.pack();
		
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		frame.setLocation(dim.width/2-frame.getSize().width/2, dim.height/2-frame.getSize().height/2);
		
		frame.setVisible(true);
		frame.setResizable(true);
	}
	
	public static File getBaseFile() {
		URL baseURL = ClassLoader.getSystemClassLoader().getResource(".");
		try {
			return new File(baseURL.toURI());
		} catch (URISyntaxException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
	
	public static void log(LogLevel level, String s) {
		switch(level) {
		case VERBOSE: println("VERBOSE: "+s); break;
		case DEBUG: println("DEBUG: "+s); break;
		case INFO:  println("INFO: "+s); break;
		case WARN:  println("WARN: "+s); break;
		case ERROR: println("ERROR: "+s); break;
		}
	}
	
	public static void print(String s) {
		if(frame==null) {
			return;
		}
		frame.appendStatusText(s);
	}
	
	public static void println(String s) {
		if(frame==null) {
			return;
		}
		frame.appendStatusText(s+"\n");
	}
	
	public static void println() {
		if(frame==null) {
			return;
		}
		frame.appendStatusText("\n");
	}
		

}
