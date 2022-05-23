package Interface.PacketSerial_Mod;

import Interface.PacketSerial_Mod.Encoding.COBS;

public class COBSPacketSerial extends PacketSerialBase{
	public COBSPacketSerial() {
		super(new COBS(),(byte)0,256);
	}
}