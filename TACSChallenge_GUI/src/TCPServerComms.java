import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicReference;

public class TCPServerComms {

	public interface ComSender{
		public void send(String msg);
	}
	
	private final AtomicReference<ComSender> comSender = new AtomicReference<ComSender>(null);
	
	private ServerSocket dataServer = null;
	private Thread dataServerThread = null;
	private final ConcurrentLinkedQueue<TCPClientComms> dataClientQueue = new ConcurrentLinkedQueue<TCPClientComms>();
	
	private ServerSocket cmdServer = null;
	private Thread cmdServerThread = null;
	private final ConcurrentLinkedQueue<TCPClientComms> cmdClientQueue = new ConcurrentLinkedQueue<TCPClientComms>();

	public TCPServerComms() {
		
	}
	
	public synchronized void start(int dataPort, int cmdPort) throws IOException {
		stop();
		if(dataServer == null || dataServer.isClosed()) {
			// Setup Data Server
			dataServer = new ServerSocket(dataPort);
			dataServerThread = new Thread() {
				@Override
				public void run() {
					while(true) {
						Socket client = null;
						try {
							client = dataServer.accept();
						} catch (Exception e) {
							e.printStackTrace();
							return;
						}
						try {
							dataClientQueue.add(new TCPClientComms(client));
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
				}
			};
			dataServerThread.start();
		}
		if(cmdServer == null || cmdServer.isClosed()) {
			// Setup Command Server
			cmdServer =  new ServerSocket(cmdPort);
			cmdServerThread = new Thread() {
				@Override
				public void run() {
					while(true) {
						Socket client = null;
						try {
							client = cmdServer.accept();
						} catch (Exception e) {
							e.printStackTrace();
							return;
						}
						try {
							cmdClientQueue.add(new TCPClientComms(client));
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
				}
			};
			cmdServerThread.start();			
		}
	}
	
	public synchronized void stop() {
		// Close the Data Server
		if(dataServer == null) {
			return;
		}
		try {
			dataServer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		try {
			dataServerThread.join();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		dataServer = null;
		dataServerThread = null;
		dataClientQueue.clear();
		// Close the Command Server
		if(cmdServer == null) {
			return;
		}
		try {
			cmdServer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		try {
			cmdServerThread.join();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		cmdServer = null;
		cmdServerThread = null;
		cmdClientQueue.clear();
	}
	
	
	public class TCPClientComms implements Runnable{
		public final PrintWriter out;
		public final BufferedReader in;
		private final Thread thread;
		
		public TCPClientComms(Socket client) throws IOException {
		    out = new PrintWriter(client.getOutputStream(), true);
		    in = new BufferedReader(new InputStreamReader(client.getInputStream()));
		    thread = new Thread(this);
		    thread.start();
		}

		@Override
		public void run() {
			while(true) {
				try {
					String s = in.readLine();
					client2com(s);
				}catch(Exception e) {
					e.printStackTrace();
					return;
				}
			}
		}
	}
	
	public void data2server(String msg) {
		Iterator<TCPClientComms> it = dataClientQueue.iterator();
		while(it.hasNext()) {
			try {
				TCPClientComms client = it.next();
				client.out.write(msg);
			}catch(Exception e) {
				it.remove();
				e.printStackTrace();
			}
		}
	}
	
	public void command2server(String msg) {
		Iterator<TCPClientComms> it = cmdClientQueue.iterator();
		while(it.hasNext()) {
			try {
				TCPClientComms client = it.next();
				client.out.write(msg);
				client.out.flush();
			}catch(Exception e) {
				it.remove();
				e.printStackTrace();
			}
		}
	}
	
	public void setComSender(ComSender comSender) {
		this.comSender.set(comSender);
	}
	
	private void client2com(String msg) {
		ComSender comSender = this.comSender.get();
		if(comSender==null) {
			return;
		}
		comSender.send(msg);
	}
	
	
}
