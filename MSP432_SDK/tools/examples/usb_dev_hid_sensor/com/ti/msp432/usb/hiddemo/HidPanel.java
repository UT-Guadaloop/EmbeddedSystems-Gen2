/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
package com.ti.msp432.usb.hiddemo;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.DefaultComboBoxModel;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JToggleButton;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;
import javax.swing.text.DefaultCaret;
import javax.swing.text.MaskFormatter;

import com.ti.msp432.usb.hiddemo.management.DataReceivedActionListener;
import com.ti.msp432.usb.hiddemo.management.HidCommunicationManager;
import com.ti.msp432.usb.hiddemo.management.HidDataReceiveThread;
import com.ti.msp432.usb.hiddemo.management.HidCommunicationManager.HidCommunicationException;

/**
 * HidPanel
 * 
 * Class to use as a starting point for HID communication. This is the Java
 * based GUI only. The actual HID communication is done in a JNI Java class
 * (HidCommunicationManager).
 * 
 * HidCommunicationManager contains Java Native Interface C calls to the actual
 * HID implementation.
 * 
 * @see com.ti.msp432.usb.hiddemo.management.HidCommunicationManager
 * @see com.ti.msp432.usb.hiddemo.management.HidDataReceiveThread
 * 
 * 
 * @author a0194920
 * 
 */
public class HidPanel extends JPanel implements DataReceivedActionListener {

	/**
	 * FixedSizeButton
	 * 
	 * Class creates a JButton with a maximum size of 100x40 so buttons align
	 * nicely in Box layout.
	 */
	class FixedSizeButton extends JButton {

		/**
		 * Serial version ID for FixedSizeButton
		 */
		private static final long serialVersionUID = 5685101956115410024L;

		public FixedSizeButton() {
			setMaximumSize(new Dimension(100, 40));
		}

	}

	/**
	 * ImageButton
	 * 
	 * Class creates a JButton with a maximum size of 100x40 so buttons align
	 * nicely in Box layout and displays an image
	 */
	public class ImageButton extends FixedSizeButton {

		/**
		 * Serial version ID for ImageButton
		 */
		private static final long serialVersionUID = 5685101956115410024L;

		public ImageButton(final ImageIcon icon) {
			setIcon(icon);
			setBorder(raisedbevel);
			setText(null);
			setSize(icon.getImage().getWidth(null), icon.getImage().getHeight(null));
		}

		public ImageButton(final String img) {
			this(new ImageIcon(img));
		}

	}

	/**
	 * ImageToggleButton
	 * 
	 * Class creates a simple image based toggle button.
	 */
	class ImageToggleButton extends JToggleButton {

		/**
		 * Serial version ID for ImageToggleButton
		 */
		private static final long serialVersionUID = 4138611018433770890L;

		public ImageToggleButton(final ImageIcon icon) {
			setIcon(icon);

			setBorder(raisedbevel);
			setText(null);
			setMargin(new Insets(0, 0, 0, 0));
			setIconTextGap(0);
			setSize(icon.getImage().getWidth(null), icon.getImage().getHeight(null));
		}

		public void setSelected(boolean state) {
			super.setSelected(state);
			if (state == false) {
				setBorder(raisedbevel);
			} else {
				setBorder(loweredbevel);
			}

		}

		public ImageToggleButton(final String img) {
			this(new ImageIcon(img));
		}

	}

	/**
	 * Serial version ID for HidPanel
	 */
	private static final long serialVersionUID = -3684673247084559831L;

	/**
	 * Main entry point for HidPanel
	 * 
	 * Creates a JFrame and adds a HidPanel to it. Also uses a WindowAdapter to
	 * handle decoration close.
	 * 
	 */
	@SuppressWarnings("deprecation")
	public static void main(final String[] args) {

		final JFrame jf = new JFrame("MSP432 HID USB Application");

		jf.setSize(new java.awt.Dimension(863, 560));

		jf.getContentPane().setLayout(new BorderLayout());
		final HidPanel hp = new HidPanel();
		jf.setIconImage(Toolkit.getDefaultToolkit().getImage(jf.getClass().getResource("/icons/TI_Bug_Icon_Red.gif")));
		jf.getContentPane().add(hp, BorderLayout.CENTER);

		final WindowAdapter wa = new WindowAdapter() {
			public void windowClosing(final WindowEvent e) {
				hp.disconnect();
				System.exit(0);
			}
		};
		jf.addWindowListener(wa);
		jf.show();

	}

	protected final String aboutMessage = "Texas Instruments MSP432 USB HID Application\n\n\nBased on original design by William Goh\nS. Tsongas and Tim Logan\n\nCopyright: Texas Instruments 2012 ";
	protected final HidCommunicationManager hMan;

	protected javax.swing.JButton clearButton;
	protected javax.swing.JButton aboutButton; 
	protected ImageToggleButton connectButton;
	protected javax.swing.JTextArea consoleArea;
	protected javax.swing.JScrollPane consoleScrollPane;
	protected javax.swing.JButton exitButton;
	protected javax.swing.JComboBox interfaceBox;
	protected javax.swing.JLabel interfaceLabel;
	protected javax.swing.JLabel lightLabel;
//	protected ImageButton mspButton;  
	protected Image normalIcon;
	protected javax.swing.JLabel numcharsLabel;
	protected javax.swing.JFormattedTextField pidField;
	protected javax.swing.JLabel pidLabel;
	protected HidDataReceiveThread readThread;
	protected Image selectedIcon;
	protected javax.swing.JButton sendButton;
	protected javax.swing.JTextField sendreceiveBox;
	protected javax.swing.JLabel sendreceiveLabel;
	protected javax.swing.JLabel serialLabel;
	protected javax.swing.JComboBox serialNumberBox;
	protected javax.swing.JButton setVidPidButton;
	protected javax.swing.JLabel statusLabel;
	protected javax.swing.JLabel toolHelpLabel;
	protected javax.swing.JFormattedTextField vidField;
	protected javax.swing.JLabel vidLabel;
	protected javax.swing.JLabel vidpidLabel;
	protected Border raisedbevel = BorderFactory.createRaisedBevelBorder();
	protected Border loweredbevel = BorderFactory.createLoweredBevelBorder();

	/**
	 * HidPanel
	 * 
	 * Class to use as a starting point for HID communication. This is the Java
	 * based GUI only. The actual HID communication is done in a JNI Java class
	 * (HidCommunicationManager).
	 * 
	 * HidCommunicationManager contains Java Native Interface C calls to the actual
	 * HID implementation.
	 * 
	 * @see com.ti.msp432.usb.hiddemo.management.HidCommunicationManager
	 * @see com.ti.msp432.usb.hiddemo.management.HidDataReceiveThread
	 * 
	 * 
	 * @author a0194920
	 * 
	 */
	public HidPanel() {

		normalIcon = Toolkit.getDefaultToolkit().getImage(getClass().getResource("/icons/disc.gif"));
		selectedIcon = Toolkit.getDefaultToolkit().getImage(getClass().getResource("/icons/conn.gif"));

		init();
		hMan = new HidCommunicationManager();

		sendButton.setEnabled(false);

		// Setting the default vid/pid to TI
		vidField.setText("0x" + Integer.toHexString(HidCommunicationManager.USB_VENDOR));
		pidField.setText("0x00" + Integer.toHexString(HidCommunicationManager.USB_PRODUCT));

		final DefaultComboBoxModel mod = new DefaultComboBoxModel();
		serialNumberBox.setModel(mod);

		final DefaultCaret caret = (DefaultCaret) consoleArea.getCaret();
		caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);

		consoleArea.setRows(1);
		consoleArea.setRows(consoleArea.getLineCount());

		/* Adding Action Listeners to all of our buttons */
		// Setting up our action listeners
		setVidPidButton.addActionListener(new ActionListener() {
			public void actionPerformed(final ActionEvent e) {
				setVidPidButtonClicked();
			}
		});

		connectButton.addActionListener(new ActionListener() {
			public void actionPerformed(final ActionEvent e) {
				connectButtonClicked();
			}
		});

		sendButton.addActionListener(new ActionListener() {

			public void actionPerformed(final ActionEvent e) {
				sendButtonClicked();
			}
		});

		clearButton.addActionListener(new ActionListener() {

			public void actionPerformed(final ActionEvent e) {
				clearButtonClicked();
			}
		});

		aboutButton.addActionListener(new ActionListener() {

			public void actionPerformed(final ActionEvent e) {
				aboutButtonClicked();
			}
		});
		setVidPidButtonClicked();
	}

	private void clearButtonClicked() {
		consoleArea.setText("");
		consoleArea.setRows(0);
	}

	private void clearButtonMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("Clears the sent/received messages window");
	}

	private void clearButtonMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private void aboutButtonClicked() {
	
		aboutButton.setBorder(loweredbevel);
		final JOptionPane pane = new JOptionPane(aboutMessage);

		final JDialog dialog = pane.createDialog(null, "About HidDemo");
		dialog.show();
		aboutButton.setBorder(raisedbevel);

	}

	private void aboutButtonMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("More Information");
	}

	private void aboutButtonMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private void connect() {
		String serial;
		int inf;

		if (serialNumberBox.getItemCount() == 0 && interfaceBox.getItemCount() == 0) {
			consoleArea.append("\n ERROR: Please ensure that VID/PID information is valid.\n");
			connectButton.setSelected(false);
			return;
		}

		if (serialNumberBox.getItemCount() == 0) {
			serial = null;
		} else
			serial = serialNumberBox.getSelectedItem().toString();
		
		//single interface device
		if (interfaceBox.getItemCount() <= 1) {
			inf = -1;
			
		}
		//multiple single interface devices connected 
		else if ((interfaceBox.getItemCount() > 1) && (interfaceBox.getSelectedItem().toString()== "HID 0"))
		{
			inf = -1;
			
		}
		//composite devices of various VID/PIDs
		else {
			final String stringInf = interfaceBox.getSelectedItem().toString();
			inf = Integer.parseInt("" + stringInf.charAt(stringInf.length() - 1));
		}
		
		try {
			hMan.connectDevice(getFormattedVid(), getFormattedPid(), serial, inf);
		} catch (final HidCommunicationException e) {
			consoleArea.append("\nCould not connect to device");
			connectButton.setSelected(false);
			consoleArea.setRows(consoleArea.getLineCount());
			e.printStackTrace();
			return;
		}

		readThread = new HidDataReceiveThread(hMan);
		readThread.setListener(this);
		readThread.start();

		consoleArea.append("\nConnected to device VID: " + getFormattedVid() + " PID: " + getFormattedPid());
		consoleArea.setRows(consoleArea.getLineCount());

		vidField.setEnabled(false);
		pidField.setEnabled(false);
		setVidPidButton.setEnabled(false);
		serialNumberBox.setEnabled(false);
		sendButton.setEnabled(true);
		interfaceBox.setEnabled(false);
		connectButton.setSelected(true);
		statusLabel.setText("Connected");
		lightLabel.setIcon(new ImageIcon(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/icons/green.png"))));
	}

	private void connectButtonClicked() {
		if (connectButton.isSelected()) {
			connect();
		} else {
			disconnect();
		}
	}

	private void connectButtonMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("Establishes a connection with the selected HID interface. All fields must contain values.");
	}

	private void connectButtonMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private JPanel createConnectArea() {

		final JPanel jp = new JPanel();
		lightLabel = new javax.swing.JLabel();
		statusLabel = new javax.swing.JLabel();

		connectButton = new ImageToggleButton(new ImageIcon(normalIcon));
		connectButton.setSelectedIcon(new ImageIcon(selectedIcon));
		connectButton.setName("connectButton");
		connectButton.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				connectButtonMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				connectButtonMouseExited(evt);
			}
		});

		lightLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
		ImageIcon im = new ImageIcon(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/icons/red.png")));
		lightLabel.setIcon(im);
		lightLabel.setName("lightLabel");

		statusLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
		statusLabel.setText("Not Initialized");
		statusLabel.setName("statusLabel");
		final JPanel left = new JPanel();
		left.setLayout(new BoxLayout(left, BoxLayout.PAGE_AXIS));
		left.add(connectButton);
		left.add(statusLabel);

		final JPanel right = new JPanel();
		right.add(lightLabel);
		jp.setLayout(new BoxLayout(jp, BoxLayout.LINE_AXIS));
		jp.add(left);
		jp.add(Box.createRigidArea(new Dimension(5, 0)));
		jp.add(right);

		return (jp);
	}

	private MaskFormatter createFormatter(final String s) {
		MaskFormatter formatter = null;
		try {
			formatter = new MaskFormatter(s);
		} catch (final java.text.ParseException exc) {
			System.err.println("formatter is bad: " + exc.getMessage());
			System.exit(-1);
		}
		return formatter;
	}

	private JPanel createMiddle() {
		final JPanel jp = new JPanel();

		sendreceiveBox = new javax.swing.JTextField();
		sendButton = new javax.swing.JButton();
		numcharsLabel = new javax.swing.JLabel();

		final Border blackline = BorderFactory.createLineBorder(Color.black);
		TitledBorder title;
		title = BorderFactory.createTitledBorder(blackline, "Send and Receive");

		title.setTitleJustification(TitledBorder.CENTER);
		jp.setBorder(title);

		sendreceiveBox.setText("Type Here");
		sendreceiveBox.setName("sendreceiveBox");
        sendreceiveBox.setEnabled(false);
		sendreceiveBox.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(final java.awt.event.ActionEvent evt) {
				sendreceiveBoxActionPerformed(evt);
			}
		});
		sendreceiveBox.addKeyListener(new java.awt.event.KeyAdapter() {
			public void keyPressed(final java.awt.event.KeyEvent evt) {
				sendreceiveBoxKeyPressed(evt);
			}

			public void keyReleased(final java.awt.event.KeyEvent evt) {
				sendreceiveBoxKeyReleased(evt);
			}

			public void keyTyped(final java.awt.event.KeyEvent evt) {
				sendreceiveBoxKeyTyped(evt);
			}
		});

		sendButton.setText("Send");
		sendButton.setName("sendButton");
		sendButton.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				sendButtonMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				sendButtonMouseExited(evt);
			}
		});

		numcharsLabel.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
		numcharsLabel.setText("9 Characters");
		numcharsLabel.setName("numcharsLabel");

		final JPanel left = new JPanel();
		left.setLayout(new BoxLayout(left, BoxLayout.PAGE_AXIS));
		left.add(sendreceiveBox);
		left.add(numcharsLabel);

		jp.setLayout(new BoxLayout(jp, BoxLayout.LINE_AXIS));
		jp.add(left);
		jp.add(Box.createRigidArea(new Dimension(5, 0)));
		jp.add(sendButton);

		return (jp);
	}

	private JPanel createSerialPanel() {
		final JPanel jp = new JPanel();
		final JPanel left = new JPanel();
		left.setLayout(new BoxLayout(left, BoxLayout.PAGE_AXIS));
		final JPanel right = new JPanel();
		right.setLayout(new BoxLayout(right, BoxLayout.PAGE_AXIS));

		serialLabel = new javax.swing.JLabel();
		interfaceLabel = new javax.swing.JLabel();
		serialNumberBox = new javax.swing.JComboBox();
		interfaceBox = new javax.swing.JComboBox();

		serialLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
		serialLabel.setText("Serial Number");
		serialLabel.setName("serialLabel");
		serialLabel.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				serialLabelMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				serialLabelMouseExited(evt);
			}
		});

		interfaceLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
		interfaceLabel.setText("Interface");
		interfaceLabel.setName("interfaceLabel");
		interfaceLabel.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				interfaceLabelMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				interfaceLabelMouseExited(evt);
			}
		});

		serialNumberBox.setName("serialNumberBox");
		serialNumberBox.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				serialNumberBoxMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				serialNumberBoxMouseExited(evt);
			}
		});

		interfaceBox.setName("interfaceBox");
		interfaceBox.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				interfaceBoxMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				interfaceBoxMouseExited(evt);
			}
		});

		left.add(serialNumberBox);
		left.add(interfaceBox);
		// left.add(Box.createRigidArea(new Dimension(5,0)));
		right.add(serialLabel);

		right.add(Box.createRigidArea(new Dimension(0, 10)));
		right.add(interfaceLabel);

		jp.setLayout(new BoxLayout(jp, BoxLayout.LINE_AXIS));
		jp.add(left);
		jp.add(Box.createRigidArea(new Dimension(5, 0)));
		jp.add(right);

		return (jp);
	}

	private JPanel createTextPanel() {

		final JPanel jp = new JPanel();

		jp.setLayout(new BorderLayout());
		JPanel east = new JPanel();
		east.setLayout(new BoxLayout(east, BoxLayout.PAGE_AXIS));
		final JPanel south = new JPanel();
		south.setLayout(new BoxLayout(south, BoxLayout.LINE_AXIS));
		south.add(Box.createRigidArea(new Dimension(20, 60)));

		consoleScrollPane = new javax.swing.JScrollPane();
		consoleArea = new javax.swing.JTextArea();
		clearButton = new FixedSizeButton();
        aboutButton = new FixedSizeButton();
		exitButton = new FixedSizeButton();
		toolHelpLabel = new javax.swing.JLabel();

		consoleScrollPane.setAutoscrolls(true);
		consoleScrollPane.setName("consoleScrollPane");
		consoleScrollPane.setPreferredSize(new java.awt.Dimension(183, 200));

		consoleArea.setColumns(20);
		consoleArea.setEditable(false);
		consoleArea.setLineWrap(true);
		consoleArea.setWrapStyleWord(true);
		consoleArea.setName("consoleArea");
		consoleScrollPane.setViewportView(consoleArea);
	
		clearButton.setText("Clear");
		clearButton.setName("clearButton");

		clearButton.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				clearButtonMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				clearButtonMouseExited(evt);
			}
		});

		aboutButton.setText("About");
		aboutButton.setName("aboutButton");

		aboutButton.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				aboutButtonMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				aboutButtonMouseExited(evt);
			}
		});

		aboutButton.setPreferredSize(clearButton.getPreferredSize());

		exitButton.setText("Exit");
		exitButton.setName("exitButton");
		exitButton.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseExited(final java.awt.event.MouseEvent evt) {
               aboutButtonMouseExited(evt); 
			}

			public void mousePressed(final java.awt.event.MouseEvent evt) {
				hMan.disconnectDevice();

				if (isConnected()) {
					disconnect();
				}
				System.exit(0);
			}
		});

		exitButton.setPreferredSize(clearButton.getPreferredSize());

		toolHelpLabel.setHorizontalAlignment(javax.swing.SwingConstants.LEFT);
		toolHelpLabel.setVerticalAlignment(javax.swing.SwingConstants.TOP);
		toolHelpLabel.setName("toolHelpLabel");

		vidField.setName("vidField");

		pidField.setName("pidField");
		pidField.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(final java.awt.event.ActionEvent evt) {
				pidFieldActionPerformed(evt);
			}
		});

		south.add(toolHelpLabel, BorderLayout.CENTER);
		east.add(clearButton);
		east.add(exitButton);
		east.add(aboutButton);

		jp.add(consoleScrollPane, BorderLayout.CENTER);
		jp.add(east, BorderLayout.EAST);
		jp.add(south, BorderLayout.SOUTH);
		return (jp);
	}

	private JPanel createTop() {
		final JPanel top = new JPanel();
		top.setLayout(new BoxLayout(top, BoxLayout.LINE_AXIS));

		final JPanel vp = createVidPidPanel();
		top.add(vp);

		top.add(Box.createRigidArea(new Dimension(20, 0)));
		top.add(Box.createGlue());
		final JPanel serial = createSerialPanel();
		top.add(serial);
		top.add(Box.createGlue());

		final JPanel connectArea = createConnectArea();
		top.add(Box.createRigidArea(new Dimension(20, 0)));
		top.add(Box.createGlue());
		top.add(connectArea);
		return (top);
	}

	private JPanel createVidPidPanel() {
		final JPanel jp = new JPanel();

		final Border blackline = BorderFactory.createLineBorder(Color.black);
		TitledBorder title;
		title = BorderFactory.createTitledBorder(blackline, "Vendor and Product ID (in Hex)");
		title.setTitleJustification(TitledBorder.CENTER);
		jp.setBorder(title);

		vidpidLabel = new javax.swing.JLabel();
		vidLabel = new javax.swing.JLabel();
		pidLabel = new javax.swing.JLabel();
		setVidPidButton = new javax.swing.JButton();
		vidField = new javax.swing.JFormattedTextField(createFormatter("0xHHHH"));
		pidField = new javax.swing.JFormattedTextField(createFormatter("0xHHHH"));

		vidpidLabel.setFont(new java.awt.Font("Tahoma", 1, 11));
		vidpidLabel.setText("Vendor and Product ID (in Hex)");
		vidpidLabel.setName("vidpidLabel");

		vidLabel.setText("VID");
		vidLabel.setName("vidLabel");

		pidLabel.setText("PID");
		pidLabel.setName("pidLabel");

		setVidPidButton.setText("SET VID PID");
		setVidPidButton.setName("setVidPidButton");
		setVidPidButton.addMouseListener(new java.awt.event.MouseAdapter() {
			public void mouseEntered(final java.awt.event.MouseEvent evt) {
				setVidPidButtonMouseEntered(evt);
			}

			public void mouseExited(final java.awt.event.MouseEvent evt) {
				setVidPidButtonMouseExited(evt);
			}
		});
		jp.setLayout(new BoxLayout(jp, BoxLayout.LINE_AXIS));
		jp.add(vidLabel);
		jp.add(Box.createRigidArea(new Dimension(5, 0)));
		jp.add(vidField);

		jp.add(Box.createRigidArea(new Dimension(10, 0)));

		jp.add(pidLabel);
		jp.add(Box.createRigidArea(new Dimension(5, 0)));
		jp.add(pidField);

		jp.add(Box.createRigidArea(new Dimension(10, 0)));
		jp.add(setVidPidButton);

		return (jp);
	}

	private void disconnect() {

		if (!isConnected())
			return;

		hMan.disconnectDevice();

		readThread.setStop(true);
		try {
			readThread.join();
		} catch (final InterruptedException e) {

			e.printStackTrace();
		}

		consoleArea.append("\nDisconnected from device");
		consoleArea.setRows(consoleArea.getLineCount());

		vidField.setEnabled(true);
		pidField.setEnabled(true);
		setVidPidButton.setEnabled(true);
		serialNumberBox.setEnabled(true);
		interfaceBox.setEnabled(true);
		sendButton.setEnabled(false);
		connectButton.setSelected(false);
		statusLabel.setText("Disconnected");
		lightLabel.setIcon(new ImageIcon(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/icons/red.png"))));
		
	}

	public void fireStringReceivedEvent(final String s) {
        int size;
        char tempChar;
        long tempValue;
        long tempValue2;
        long temperature;
        int sensorState;
        int sensorEvent;
		if (!s.equals("")) {
            size = s.length();
            //get value at position 2 in string s
            tempChar = s.charAt(2);
            //convert value to number value
            tempValue = tempChar;
            tempChar = s.charAt(3);
            tempValue2 = tempChar;
            //shift bits appropriately to get a 16bit value from string values at positions
            //2 and 3
            temperature = tempValue + (tempValue2 << 8);
 
            tempChar = s.charAt(0);
            sensorState = tempChar;
            tempChar = s.charAt(1);
            sensorEvent = tempChar;
            
            consoleArea.append("\nRX--> Sensor Temperature:" + temperature + "C, Sensor State:" + sensorState + ", Sensor Event:" + sensorEvent );
			consoleArea.setRows(consoleArea.getLineCount());
		}
	}

	public void fireUnableToReadEvent() {
		consoleArea.append("\nDisconnected from device");
		consoleArea.setRows(consoleArea.getLineCount());

		vidField.setEnabled(true);
		pidField.setEnabled(true);
		setVidPidButton.setEnabled(true);
		serialNumberBox.setEnabled(true);
		interfaceBox.setEnabled(true);
		sendButton.setEnabled(false);

		statusLabel.setText("Disconnected");
		lightLabel.setIcon(new ImageIcon(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/icons/red.png"))));

	}

	private int getFormattedPid() {
		return Integer.parseInt(pidField.getText().replace("0x", ""), 16);
	}

	private int getFormattedVid() {
		return Integer.parseInt(vidField.getText().replace("0x", ""), 16);
	}

	private void init() {

		final JPanel top = new JPanel();
		top.setLayout(new BoxLayout(top, BoxLayout.PAGE_AXIS));

		final JPanel vidPidPanel = createTop();

		final JPanel sendReceivePanel = createMiddle();

		final JPanel main = createTextPanel();

		top.add(vidPidPanel);
		top.add(sendReceivePanel);
		this.setLayout(new BorderLayout());

		this.add(top, BorderLayout.NORTH);
		this.add(main, BorderLayout.CENTER);

	}

	private void interfaceBoxMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("<HTML>Each item in this list is an HID interface available on the selected physical USB "
		        + "<br>device, in the same order they are declared in the device.</HTML>");
	}

	private void interfaceBoxMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private void interfaceLabelMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("<HTML>Each item in this list is an HID interface available on the selected physical USB "
		        + "<br>device, in the same order they are declared in the device.</HTML>");
	}

	private void interfaceLabelMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private boolean isConnected() {
		if (readThread != null)
			return (readThread.isAlive());
		else
			return (false);

	}

	private void pidFieldActionPerformed(final java.awt.event.ActionEvent evt) {

	}

	private void sendButtonClicked() {

		try {
			consoleArea.append("\nTX-->" + sendreceiveBox.getText());
			consoleArea.setRows(consoleArea.getLineCount());
			hMan.sendData(sendreceiveBox.getText());
		} catch (final HidCommunicationException e) {
			consoleArea.append("\nUnable to send buffer!");
			consoleArea.setRows(consoleArea.getLineCount());
		}

	}

	private void sendButtonMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("Send messages shown in window the left box to the device.");
	}

	private void sendButtonMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private void sendreceiveBoxActionPerformed(final java.awt.event.ActionEvent evt) {
		if (sendButton.isEnabled()) sendButtonClicked();	
	}

	private void sendreceiveBoxKeyPressed(final java.awt.event.KeyEvent evt) {
		numcharsLabel.setText(sendreceiveBox.getText().length() + " Characters");
	}

	private void sendreceiveBoxKeyReleased(final java.awt.event.KeyEvent evt) {
		numcharsLabel.setText(sendreceiveBox.getText().length() + " Characters");
	}

	private void sendreceiveBoxKeyTyped(final java.awt.event.KeyEvent evt) {
		numcharsLabel.setText(sendreceiveBox.getText().length() + " Characters");
	}

	private void serialLabelMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("<HTML>Every physical USB device has a unique serial number. Therefore, each item in this list "
		        + "<br>represents a physical USB device on the system that is reporting the VID or PID selected.</HTML>");
	}

	private void serialLabelMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	private void serialNumberBoxMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("<HTML>Every physical USB device has a unique serial number. Therefore, each item in this list "
		        + "<br>represents a physical USB device on the system that is reporting the VID or PID selected.</HTML>");
	}

	private void serialNumberBoxMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

	/**
	 * Action listener that updates the serial/interfaces boxes when the user
	 * clicks the set VID/PID
	 */
	private void setVidPidButtonClicked() {
		int[] interfaces=new int[0];
		String[] serials=new String[0];
		int singleInfCount = 0;
		/* Removing all old entries, checking validity of input */
		serialNumberBox.removeAllItems();
		interfaceBox.removeAllItems();

		if (vidField.getText().trim().equals("0x") || pidField.getText().trim().equals("0x"))
			return;

		final int vid = getFormattedVid();
		final int pid = getFormattedPid();
		
		
		try {
			/* Calling our native functions and processing the results */
			interfaces = hMan.getInterfacesForVidPid(vid, pid);
		} catch (Exception e) {
			JOptionPane.showMessageDialog(null,  "getInterfacesForVidPid error", "Alert",JOptionPane.ERROR_MESSAGE);
			
		}

		try {
			/* Calling our native functions and processing the results */
			serials = hMan.getSerialsForVidPid(vid, pid);
		} catch (Exception e) {
			JOptionPane.showMessageDialog(null,  "getSerialsForVidPid error", "Alert",JOptionPane.ERROR_MESSAGE);
			e.printStackTrace();
		}
		
		
		final DefaultComboBoxModel mod = (DefaultComboBoxModel) serialNumberBox.getModel();
		if ((serials.length==0) ||  (interfaces.length == 0) ){
			JOptionPane.showMessageDialog(null,  "No Serial Interfaces Found for VID/PID combination (vid="+Integer.toHexString(vid)+ 
					" pid="+Integer.toHexString(pid)+")\nPlease connect a board with USB on this workstation.", "Oops",JOptionPane.WARNING_MESSAGE);
		}
		for (int i = 0; i < serials.length; i++) {
			if (!serials[i].equals("") && mod.getIndexOf(serials[i]) < 0)
				serialNumberBox.insertItemAt(serials[i], 0);
		}
	
        //counts the number of single interface devices		
		for (int j = 0; j < interfaces.length; j++) {
			if (interfaces[j] == -1){
				singleInfCount++;
			}
		}
		
		//sets the default HID value for display in Interface box.  Especially necessary for one HID interface
		if ((interfaces.length == 1) && (interfaces[0] == -1)) 
		{
			interfaceBox.addItem("HID 0");     
		} 
		//multiple single interface devices
		else if ((interfaces.length > 1) && (singleInfCount > 1))
		{
			for (int i = 0; i < singleInfCount; i++)
			{
				interfaceBox.insertItemAt("HID 0", interfaceBox.getItemCount());
			}
		}
		//single or multiple composite devices
	    else 
		{
			for (int i = 0; i < interfaces.length; i++) {
		
				interfaceBox.insertItemAt("HID " + (interfaces[i]), interfaceBox.getItemCount());

			}
		}

		if (serialNumberBox.getItemCount() > 0)
			serialNumberBox.setSelectedIndex(0);

		if (interfaceBox.getItemCount() > 0)
			interfaceBox.setSelectedIndex(0);

	}

	private void setVidPidButtonMouseEntered(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("Enter the VID/PID of the USB device the application should look for, then press this button.");
	}

	private void setVidPidButtonMouseExited(final java.awt.event.MouseEvent evt) {
		toolHelpLabel.setText("");
	}

}
//Released_Version_5_20_06_02
