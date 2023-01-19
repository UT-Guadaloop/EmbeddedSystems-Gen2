/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
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
/**
 * Management class for communicating with a HID device. Currently, only one
 *  Hid device can be connected at once. This class relies on the lower level
 *  custom JNI libraries. Each time a user connects to a device, a reference
 *  to the open HID handle in the lower level is kept track in the connectedDevice
 *  variable. This variable is passed through the send/receive functions and
 *  used by the lower JNI level to communicate with the HID device. 
 *  
 * @author Timothy Logan
 */
package com.ti.msp432.usb.hiddemo.management;


import java.nio.ByteBuffer;
import java.util.StringTokenizer;

public class HidCommunicationManager {

	/* TI Specific VID/PID */
	public static final int USB_VENDOR = 0x1CBE;  
	public static final int USB_PRODUCT = 0x0012; 
	public static final int TI_EXAMPLE_REPORT_ID = 0x00; 
	private boolean is64bit=false;

	/* JNI Native Management Functions */
	public native int getNumberOfInterfaces(int vid, int pid);

	public native String[] getSerialsForVidPid(int vid, int pid);

	public native int[] getInterfacesForVidPid(int vid, int pid);

	/* Communication functions */
	public native int connectDeviceNative(int vid, int pid, String serial,
			int infNumber);

	private native void disconnectDeviceNative(int handle);

	private native int sendDataNative(int handle, byte[] data, int size);

	private native byte[] receiveDataNative(int handle);

	/* Management Data */
	private int connectedDevice;
	private boolean connected;

public HidCommunicationManager() {

    String property = System.getProperty("java.library.path");
    StringTokenizer parser = new StringTokenizer(property, ";");
    while (parser.hasMoreTokens()) {
        System.err.println(parser.nextToken());
    }

    is64bit = (System.getProperty("os.arch").indexOf("64") != -1);
		System.out.println("is64Bit="+is64bit);
		if (is64bit) {
		   System.out.println("Loading 64 bit version HidCommunicationManager64");
		   System.loadLibrary("HidCommunicationManager64");
		} else {
		  System.out.println("Loading 32 bit version HidCommunicationManager");
		  System.loadLibrary("HidCommunicationManager");
		}
		System.out.println("loaded JNI Library");
	}


	/**
	 * Open the handle to our HID device. Note there is no polling function
	 * currently that determines if our device is still connected.
	 * 
	 * @param vid
	 * @param pid
	 * @param serial
	 * @param infNumber
	 * @throws HidCommunicationException
	 */
	public void connectDevice(int vid, int pid, String serial, int infNumber)
			throws HidCommunicationException {

		int res = connectDeviceNative(vid, pid, serial, infNumber);

		if (res <= 0)
			throw new HidCommunicationException(
					"Unable to connect to HidDevice");

		connectedDevice = res;
		setConnected(true);
	}

	/**
	 * 
	 */
	public void disconnectDevice() {
		disconnectDeviceNative(connectedDevice);
		setConnected(false);
	}

	/**
	 * Wrapper function to accept String and send to native function as a byte
	 * array. The HidAPI packet specifies that we need to send the ReportID
	 * followed of the number of bytes to write, followed by the actual data to
	 * write. Since we use one byte to represent the size in the HID packet, we
	 * have to carve the string data into 252 byte packets and send them to the
	 * device.
	 * 
	 * @param vid
	 *            Vendor ID
	 * @param pid
	 *            Product ID
	 * @param s
	 *            String to send
	 * @return Negative if the send failed, number of bytes sent if the
	 *         operation went through
	 * @throws HidCommunicationException
	 */


	public int sendData(String s) throws HidCommunicationException {

		int res = 0;
		String temp = "";
       
        
		if (!isConnected())
			throw new HidCommunicationException(
					"Not connected! Connect to device first!");

		/* We need to send the data to the JNI level at 253 byte chunks */
		while (s.length() > 252) 
		{
			temp = s.substring(0, 252);
			s = s.substring(253, s.length() - 1);
			byte arr[] = new byte[255];
			arr[0] = TI_EXAMPLE_REPORT_ID;
			arr[1] = (byte) (arr.length - 2);

			for (int i = 0; i < temp.length(); i++) {
				arr[i + 2] = (byte) temp.charAt(i);
			}

			res = sendDataNative(connectedDevice, arr, arr.length);

			if (res < 0)
				throw new HidCommunicationException(
						"Unable to transfer buffer!");
		}
		/* Prepare the packet */
		byte arr[] = new byte[s.length() + 2];
		arr[0] = TI_EXAMPLE_REPORT_ID;
		arr[1] = (byte) (arr.length - 2);

		for (int i = 0; i <s.length(); i++) {
			arr[i + 2] = (byte) s.charAt(i);
		}

		res = sendDataNative(connectedDevice, arr, arr.length);

		if (res < 0)
			throw new HidCommunicationException("Unable to transfer buffer!");

		return res;
	}

	/**
	 * Wrapper function for the native receive data from HID device function. We
	 * receive the information in a byte array, parse out how long the data is,
	 * and then convert it into a standard Java string.
	 * 
	 * @param vid
	 *            Vendor ID
	 * @param pid
	 *            Product ID
	 * @return String value of whatever was read
	 * @throws HidCommunicationException
	 */
	public String receiveData() throws HidCommunicationException {

		if (!isConnected())
			throw new HidCommunicationException(
					"Not connected! Connect to device first!");

		byte[] buf;
        String sBuf;

		/* Native function call */
		buf = receiveDataNative(connectedDevice);
    
		if (buf == null)
			throw new HidCommunicationException(
					"Unable to read buffer from HID device!");

		/* If the buffer is two bytes, there is no data to read */
		if (buf.length == 1 && buf[0] == 0)
			return "";

        int length = buf.length;  //get total bytes 

		byte parsedArr[] = new byte[length-1];  //new array size is length-1 since
                                               //buf[0] contains report ID
       
		/* Extracting the relevant data.  Length is subtracted
           by 1 since the first byte is the report ID. */
		for (int i = 0; i < length-1; i++) {
			parsedArr[i] = buf[i + 1];  
      
		}
  
		sBuf = new String(parsedArr);  

        return sBuf;

	}

	public boolean isConnected() {
		return connected;
	}

	private void setConnected(boolean connected) {
		this.connected = connected;
	}

	@SuppressWarnings("serial")
	public class HidCommunicationException extends Exception {
		public HidCommunicationException(String s) {
			super(s);
		}
	}
}
//Released_Version_5_20_06_02
