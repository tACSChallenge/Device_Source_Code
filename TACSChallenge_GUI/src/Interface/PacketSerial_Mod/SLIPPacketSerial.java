package Interface.PacketSerial_Mod;

import Interface.PacketSerial_Mod.Encoding.SLIP;

public class SLIPPacketSerial extends PacketSerialBase{
	public SLIPPacketSerial() {
		super(new SLIP(), SLIP.END, 256);
	}
}
