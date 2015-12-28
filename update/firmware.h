#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#include <stdbool.h>

bool isNeedDownloadFirmware(void);
bool isNeedUpdateFirmware(void);

bool downloadFirmwareToRam(void);
bool updateFirmware(void);
bool programFromRam(void);
void notifyOperation( char *info);
void eraseupdatemark(void);
bool setFirmwareUpdate(unsigned int lenth, unsigned int type);

#endif
