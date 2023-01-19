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
 * Read thread that continuously polls the HID device for data to be received. 
 * The JNI/HID layer has the hid device set to "nonblocking". This means that
 * if the user application tries to read data and there is no data to be written,
 * a zero value of returned. The JNI layer  communicates this with the Java layer
 * and the Java layer will sleep for 500ms and then poll the read again.
 */

package com.ti.msp432.usb.hiddemo.management;

import com.ti.msp432.usb.hiddemo.management.HidCommunicationManager.HidCommunicationException;

public class HidDataReceiveThread extends Thread {

        
        private HidCommunicationManager hMan;
        private DataReceivedActionListener listener;
        private boolean stop = false;
        
        public HidDataReceiveThread(HidCommunicationManager hMan) {
                this.hMan = hMan;         
        }
        
        public DataReceivedActionListener getListener() {
                return listener;
        }

        public boolean isStop() {
                return stop;
        }

        public void setStop(boolean stop) {
                this.stop = stop;
        }

        public void setListener(DataReceivedActionListener listener) {
                this.listener = listener;
        }

        public void run() {
                while(true) {
                        
                        if(stop)
                                return;
                        
                        String data = "";
                        try {
                                data = hMan.receiveData();
                                if(data.equals("")) {
                                   Thread.sleep(500);     
                                }
                                
                        } catch (HidCommunicationException e) {
                                listener.fireStringReceivedEvent("Error receiving buffer from device!");
                                listener.fireUnableToReadEvent();
                                return;
                        } catch (InterruptedException e) {
                                listener.fireStringReceivedEvent("Read polling thread existed");
                                listener.fireUnableToReadEvent();
                                return;
                        } 
                        listener.fireStringReceivedEvent(data);                        
                }
        }
}
//Released_Version_5_20_06_02
