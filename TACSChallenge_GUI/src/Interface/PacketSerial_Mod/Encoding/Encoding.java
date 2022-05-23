package Interface.PacketSerial_Mod.Encoding;

public interface Encoding {
	public int encode(byte[] buffer, int size, byte[] encoded);
    public int decode(byte[] buffer, int size, byte[] decoded);
    public int getEncodedBufferSize(int sourceSize);
}
