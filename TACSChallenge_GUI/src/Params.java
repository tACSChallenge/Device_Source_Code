import java.util.prefs.Preferences;

public class Params {
	public static final Preferences prefs = Preferences.userRoot().node(TACSChallengeFrame.class.getName());
	public static final String PREF_DATA_TCP_PORT = "data_tcp_port";
	public static final String PREF_CMD_TCP_PORT = "cmd_tcp_port";
	public static final String PREF_COM_PORT = "com_port";
	public static final String PREF_LOG = "log";
	public static final String PREF_CMDS = "cmds";
}
