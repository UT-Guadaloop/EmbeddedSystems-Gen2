/*
 * fat_usbmsc.h
 *
 *  Created on: Sep 21, 2017
 *      Author: a0221163
 */

#ifndef FAT_USBMSC_H_
#define FAT_USBMSC_H_

#include "third_party/fatfs/diskio.h"

extern DSTATUS  FATUSBMSC_diskInitialize(BYTE drv);
extern DRESULT  FATUSBMSC_diskIOctl(BYTE drv, BYTE ctrl, void *buf);
extern DRESULT  FATUSBMSC_diskRead(BYTE drv, BYTE *buf,
                                   DWORD sector, UINT count);
extern DSTATUS  FATUSBMSC_diskStatus(BYTE drv);
extern DRESULT  FATUSBMSC_diskWrite(BYTE drv, const BYTE *buf,
                                    DWORD sector, UINT count);



#endif /* FAT_USBMSC_H_ */
