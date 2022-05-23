import java.util.concurrent.ConcurrentHashMap;

public class CommandTypes {

    static class Command {
    	int    code;
    	String name;
    	
    	public Command(int code, String name) {
    		this.code = code;
    		this.name = name;
    	}
    }
    
    private static final ConcurrentHashMap<Integer, Command> commands_by_code = new ConcurrentHashMap<Integer,Command>();
    private static final ConcurrentHashMap<String, Command> commands_by_name = new ConcurrentHashMap<String,Command>(); 
    static {
    	addCommand(0,"none");
    	addCommand(1,"shkreq");
    	addCommand(2,"shkrsp");
    	addCommand(3,"echo");
    	addCommand(4,"log_level");
    	addCommand(5,"wled");
    	addCommand(6,"wbnc");
    	addCommand(7,"wbncmode");
    	addCommand(8,"clear_stim");
    	addCommand(9,"log");
    	addCommand(10,"*stim");
    	addCommand(11,"*wait");
    	addCommand(12,"*trig");
    	addCommand(13,"start_stim");
    	addCommand(14,"stop_stim");
    	addCommand(15,"pause");
    	addCommand(16,"start_stair");
    	addCommand(17,"stop_stair");
    	addCommand(18,"set_var");
    	addCommand(19,"get_var");
    	addCommand(20,"delete_var");
    	addCommand(21,"delete_vars");
    }
    
    public static boolean addCommand(int code, String name) {
    	synchronized(commands_by_code) {
    		synchronized(commands_by_name) {
    	    	String name_clean = name.trim().toLowerCase();
    	    	// check if the command already exists
    	    	if(commands_by_name.contains(name_clean) || commands_by_code.contains(code)) {
    	    		// command exists
    	    		return false;
    	    	}else {
    	    		// command does NOT exist
    	    		Command cmd = new Command(code, name_clean);
    	    		commands_by_code.put(code, cmd);
    	    		commands_by_name.put(name_clean, cmd);
    	    		return true;
    	    	}    			
    		}
    	}
    }
    
    public static Command getCommand(int code) {
		return commands_by_code.get(code);
    }
	
    public static Command getCommand(String name) {
		return commands_by_name.get(name);
    }
    
    public static void clear() {
    	synchronized(commands_by_code) {
    		synchronized(commands_by_name) {
		    	commands_by_code.clear();
		    	commands_by_name.clear();
    		}
    	}
    }
    
    
}
