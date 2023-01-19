#include "com_ti_msp432_usb_hiddemo_management_HidCommunicationManager.h"

#include <hidapi.h>
#include <string.h>
#include <stdlib.h>

/**
 * Converts a Java JNI jstring to a wide character array (what HidAPI uses)
 */
wchar_t * javaToWSZ(JNIEnv* env, jstring string)
{
    if (string == NULL)
        return NULL;
        
    int len = (*env)->GetStringLength(env,string);
    const jchar* raw = (*env)->GetStringChars(env,string, NULL);
    
    if (raw == NULL)
        return NULL;

    wchar_t* wsz = malloc(len+1);
    memcpy(wsz, raw, len*2);
    wsz[len] = 0;

    (*env)->ReleaseStringChars(env,string, raw);
    return wsz;
}


/*
 * Useful funct to dump the hid_device_info structure.
 * Not used in the code...
*/
void dumpHidDeviceInfo(struct hid_device_info *devs) {

                     printf ("path=%s\n",devs->path);
                     printf ("vendor_id=%d\n", devs->vendor_id);
                     printf ("product_id=%d\n", devs->product_id);
                     printf ("serial_number=%ls\n", devs->serial_number);
                     printf ("release_number=%d\n", devs->release_number);
                     printf ("manufacturer_string=%ls\n",devs->manufacturer_string);
                     printf ("product_string=%ls\n", devs->product_string);
                     printf ("usage_page=%d\n", devs->usage_page);
                     printf ("usage=%d\n", devs->usage);
                     printf ("interface_number=%d\n", devs->interface_number);
                     printf ("END\n\n");

}

/**
 * Returns the total number of connected HID Devces on a system with a valid VID
 * and PID number.
 */
JNIEXPORT jint JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_getNumberOfInterfaces
  (JNIEnv * env, jobject obj, jint vid, jint pid ) {

        struct hid_device_info *devs, *cur_dev;
        int count = 0;
        
        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;	
        while (cur_dev) {
            // To see the contents of hid_device_info structure, un-comment the next line
            // dumpHidDeviceInfo(cur_dev);
            
            if(!cur_dev || !cur_dev->vendor_id || !cur_dev->product_id)
                continue;
            
            if(cur_dev->vendor_id == vid && cur_dev->product_id == pid) {
                count++;
            }
            cur_dev = cur_dev->next;
        }
        
        hid_free_enumeration(devs);
        
        return count;
  }
  
/**
 * Returns an JNI object array with the serials of all devices with the 
 * specified VID and PID number.
 */
JNIEXPORT jobjectArray JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_getSerialsForVidPid
  (JNIEnv * env, jobject obj, jint vid, jint pid) {
        
        struct hid_device_info *devs, *cur_dev;
        jobjectArray ret;
        int count = 0;
        int dev_count = Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_getNumberOfInterfaces(env,obj,vid,pid);
       
        //Creating our Java Array
        ret = (jobjectArray)(*env)->NewObjectArray(env,dev_count, 
                (*env)->FindClass(env,"java/lang/String"),(*env)->NewStringUTF(env,""));
        
        // Iterating through our list of HID devices
        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;	
        
        while (cur_dev) {

            if(!cur_dev->serial_number) {
                    cur_dev = cur_dev->next;
                    continue;
            }
                
            jstring serial= (*env)->NewString(env,cur_dev->serial_number,wcslen(cur_dev->serial_number));
            
            if(cur_dev->vendor_id == vid && cur_dev->product_id == pid) {
                (*env)->SetObjectArrayElement(env,ret,count,serial);
                count++;
            }
            
            cur_dev = cur_dev->next;
        }
        
        hid_free_enumeration(devs);
        
        return ret;
  }
  
/**
 * Returns an JNI integer array with the interfaces of all devices with the 
 * specified VID and PID number.
*/
  JNIEXPORT jintArray JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_getInterfacesForVidPid
  (JNIEnv * env, jobject obj, jint vid, jint pid) {
  
        struct hid_device_info *devs, *cur_dev;
        jintArray ret;
        int count;
        int dev_count = Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_getNumberOfInterfaces(env,obj,vid,pid);
        
        //Creating our Java Array
        ret = (jintArray)((*env)->NewIntArray(env, dev_count));
        jint buf[dev_count];
        
        // Iterating through our list of HID devices
        devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;	
        count = 0;
        
        while (cur_dev) {
                            
            if(cur_dev->vendor_id == vid && cur_dev->product_id == pid) {
                buf[count] = cur_dev->interface_number;
                count++;
            }
            
            cur_dev = cur_dev->next;          
        }
        
        (*env)->SetIntArrayRegion(env,ret,0,dev_count,(jint*)buf);
        
        hid_free_enumeration(devs);
        
        return ret;
  }
   
/**
 * Connects to the HID device with the given parameters and returns a pointer
 * to the open handle. The pointer is then kept track of in the Java layer and
 * passed through the other communication functions. If the serial is null or
 * if the interface is a negative value, this function will try to connect to
 * the first available matching VID/PID
 */
  JNIEXPORT jint JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_connectDeviceNative
  (JNIEnv * env, jobject obj, jint vid, jint pid, jstring javaserial, jint device) {
  
        hid_device *handle;
        struct hid_device_info *devs, *cur_dev;
        
      	devs = hid_enumerate(0x0, 0x0);
        cur_dev = devs;
        const char* serial = (*env)->GetStringUTFChars(env,javaserial,0);
        
        /* Iterating over all the connected devices and finding the one we want */
        while (cur_dev) {
      		
            if(cur_dev->vendor_id == vid && cur_dev->product_id == pid) {

                /* If we have an interface number, check it to make sure it matches */
                if(device >= 0 && cur_dev->interface_number != device) {
                    cur_dev = cur_dev->next;
                    continue;
                }
                
                /* Same goes with the serial */
                if(cur_dev->serial_number && javaserial != NULL) {
                    
                    jstring curjavaSerial= (*env)->NewString(env,cur_dev->serial_number,wcslen(cur_dev->serial_number));
                    const char* curSerial=(*env)->GetStringUTFChars(env,javaserial,0);
                    
                    if(strcmp(serial,curSerial) != 0) {
                        cur_dev = cur_dev->next;
                        continue;
                    }
                }
                                
                handle = hid_open_path(cur_dev->path);  
                hid_set_nonblocking(handle, 1);
                
                if(!handle) {
                    printf("HID open of path failed\n");
                    hid_free_enumeration(devs);
                    return -1;
                }
                
                hid_free_enumeration(devs);
                return (int)handle;
            }
    
            cur_dev = cur_dev->next;
        }
    
        hid_free_enumeration(devs);        
        printf("Could not find HID device\n");
        return -1;   
  }
  
/**
 * Close the connection to the HID device with the provided handle and exit the
 * HID API.
 */
  JNIEXPORT void JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_disconnectDeviceNative
  (JNIEnv * env, jobject obj, jint longhandle) {
  
        hid_device *handle = (hid_device*)(longhandle);
        hid_close(handle);
        hid_exit();
  }
  
  /**
   * Send data to the connected HID device. Size is the total size of the buffer
   * (including the Report ID and Data Size bytes). Since Windows and other OSs
   * tend to like 64 Byte chunks of data, if the data size is above 62 bytes
   * the data is carved into 64 byte chunks (2 byte header + 62 bytes of data)
   * each. The last packet's data chunk is padded with zeros to make a complete
   * 62 bytes worth of data.
   */
  JNIEXPORT jint JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_sendDataNative
  (JNIEnv * env, jobject obj, jint longhandle, jbyteArray data, jint size) {
        
        int res = 0;
        int bytes_written = 0;
        int totalsize;
        int reportid;
        hid_device *handle;
        unsigned char trans_data[64];
        unsigned char *buf;
        
        /* Opening the device */
        handle = (hid_device*)longhandle;
        
        /* The buffer should be at least 3 (ReportID,Size,At least 1 byte) */
        if(size < 3) {
            printf("Error: Invalid HID packet size\n");
            return -1;
        }
        
        if(!handle){
            printf("Error: Invalid handle passed to write\n");
            return -1;
        }
        
        buf = (*env)->GetByteArrayElements(env, data, NULL);
        
        if(!buf){
            printf("Error: Unable to translate buffer to write\n");
            return -1;
        }
        
       /* Extracting/Setting the header */
        reportid = buf[0];
        totalsize = buf[1];

        trans_data[0] = reportid;
       
        while(totalsize > 62) {

            trans_data[1] = 62;
            memcpy(trans_data+2,buf+bytes_written+2,62);
            res = hid_write(handle,trans_data,64);

            if(res < 0) {
                printf("Error Writing: %ls\n", hid_error(handle));
                printf("bytes_written: %d totalsize %d handle 0x%x\n",bytes_written,totalsize,handle);
                return res;
            }
            
            bytes_written += 62;
            totalsize -= 62;
        }

        /* Corner case where no more bytes exist */
        if(totalsize == 0)
            return bytes_written;
        
        trans_data[1] = totalsize;
        /* Prepare one last 64 byte buffer padded with zeros */
        memcpy(trans_data+2,buf+bytes_written+2,totalsize);
        memset(trans_data+2+totalsize,0,64-(totalsize+2));   
        
        res = hid_write(handle,trans_data,64);
        
        if(res < 0) {
            printf("Error Writing: %ls\n", hid_error(handle));
            printf("bytes_written: %d totalsize %d handle 0x%x\n",bytes_written,totalsize,handle);
            return res;
        }
        
        bytes_written += res;
        
        return bytes_written;
            
  }
  
/**
 * Queries the HID port for data to read. If there is data to read, the data is
 * returned in the standard HID packet format (1 byte Report ID, 1 byte size,
 * 62 bytes worth of data. It is up to the Java layer to keep calling this
 * function until there is no more data to be written. Since the HidAPI is set
 * to nonblocking during initialization, if there is no data to be read a 0
 * value is returned from the hid_read api call. A "dummy" two byte packet in
 * this case will be returned to the Java layer to signify that no data has been
 * read.
 */
  JNIEXPORT jbyteArray JNICALL Java_com_ti_msp432_usb_hiddemo_management_HidCommunicationManager_receiveDataNative
  (JNIEnv * env, jobject obj, jint longhandle) {  
  
        hid_device *handle;
        jbyteArray ret;
        int res;
        
        /* Opening the device */
        handle = (hid_device*)longhandle;
        
        if(!handle) {
            printf("Error: Opening handle in read\n");
            return NULL;
        }
            
        jbyte buf[5];  //Temperature sensor device sends out only 4 data bytes + reportID
                       //The data bytes depends on the HID Input Report
        res = hid_read(handle,buf,5);  

        if(res < 0) {
            printf("Error: %ls\n", hid_error(handle));
            return NULL;
        }
        
        /* If res is zero, there is nothing to read and we have to signal the host */
        if(res == 0) {

            ret = (*env)->NewByteArray(env,1);
            jbyte fakeBuf[1];
            fakeBuf[0] = 0x0;
            (*env)->SetByteArrayRegion(env,ret,0,1,fakeBuf);
            return ret;
        }

                
        /* Reading a 5 byte chunk and return it */
        ret = (*env)->NewByteArray(env,5);
        (*env)->SetByteArrayRegion(env,ret,0,5,buf);

        return ret;
          
  }
  
  
//Released_Version_5_20_06_02
