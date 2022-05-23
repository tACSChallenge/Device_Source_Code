package Interface.PacketSerial_Mod;


import Interface.PacketSerial_Mod.Encoding.*;
import com.fazecast.jSerialComm.*;


public class PacketSerialBase
{

    public PacketSerialBase(Encoding encoding, byte PacketMarker, int BufferSize)
    {
    	_recieveBuffer = new byte[BufferSize];
    	_recieveBufferIndex = 0;
    	_encoding = encoding;
        _PacketMarker = PacketMarker;
        _BufferSize = BufferSize;
    }

    public boolean begin(SerialPort serial, int baudRate)
    {
        _serial = serial;
        _serial.setComPortParameters(baudRate,8,SerialPort.ONE_STOP_BIT,SerialPort.NO_PARITY);
        boolean success = _serial.openPort();
        if(success) {
            _serial.addDataListener(new SerialPortDataListener() {
         	   @Override
         	   public int getListeningEvents() { return SerialPort.LISTENING_EVENT_DATA_RECEIVED; }
         	   @Override
         	   public void serialEvent(SerialPortEvent event)
         	   {
         	      byte[] newData = event.getReceivedData();         	      
         	      for(int i=0; i<newData.length; i++) {
         	    	  update(newData[i]);
         	      }
         	   }
            });
            return true;
        }else {
            return false;
        }
    }

    private void update(byte data)
    {
        if (_serial == null) return;
        
        if (data == _PacketMarker)
        {
            if (_onPacketClass!=null)
            {
                byte[] _decodeBuffer = new byte[_recieveBufferIndex];

                int numDecoded = _encoding.decode(_recieveBuffer,
                                                        _recieveBufferIndex,
                                                        _decodeBuffer);
                
                if(_onPacketClass!=null){
                  _onPacketClass.onPacketFunction(_decodeBuffer, numDecoded);
                }
                
            }

            _recieveBufferIndex = 0;
        }
        else
        {
            if ((_recieveBufferIndex + 1) < _BufferSize)
            {
                _recieveBuffer[_recieveBufferIndex++] = data;
            }
            else
            {
                // Error, buffer overflow if we write.
            }
        }
    }

    public void send(byte[] buffer, int size)
    {
        if(_serial == null || buffer == null || size == 0) return;

        byte[] _encodeBuffer = new byte[_encoding.getEncodedBufferSize(size)];

        int numEncoded = _encoding.encode(buffer,
                                                size,
                                                _encodeBuffer);
                                                
        _serial.writeBytes(_encodeBuffer, numEncoded);
        _serial.writeBytes(new byte[] {_PacketMarker},1);
    }


    public void setPacketHandler(PacketHandlerInterface onPacketClass)
    {
        _onPacketClass = onPacketClass;
    }


    private byte[] _recieveBuffer;
    private int _recieveBufferIndex;
    private byte _PacketMarker;
    private int _BufferSize;
    private Encoding _encoding;
    
    private SerialPort _serial = null;
    
    private PacketHandlerInterface _onPacketClass;
}








