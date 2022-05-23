package Interface.PacketSerial_Mod;

public interface PacketHandlerInterface {
	void onPacketFunction(byte[] buffer, int size);
}

