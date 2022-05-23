

import java.awt.Component;

import javax.swing.JList;
import javax.swing.plaf.basic.BasicComboBoxRenderer;

import com.fazecast.jSerialComm.SerialPort;

public class PortComboBoxRenderer extends BasicComboBoxRenderer {
	private static final long serialVersionUID = 1L;
	@Override
	public Component getListCellRendererComponent(JList list,
			Object value, int index, boolean isSelected,
			boolean cellHasFocus) {
		
		super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

		String text = "";
		if (value != null && value instanceof SerialPort) {
			SerialPort porti = (SerialPort) value;
			if (porti.getSystemPortName() != null) {
				text = porti.getSystemPortName();
			}
		}
		setText(text);
		return this;
	}
}
