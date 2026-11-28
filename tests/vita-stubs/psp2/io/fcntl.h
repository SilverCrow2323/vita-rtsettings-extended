#ifndef TEST_VITA_IO_FCNTL_H
#define TEST_VITA_IO_FCNTL_H
#include <psp2common/kernel/iofilemgr.h>
SceUID sceIoOpen(const char *, int, SceMode);
int sceIoClose(SceUID);
SceSSize sceIoRead(SceUID, void *, SceSize);
SceSSize sceIoWrite(SceUID, const void *, SceSize);
SceOff sceIoLseek(SceUID, SceOff, int);
int sceIoSyncByFd(SceUID, int);
int sceIoPread(SceUID, void *, SceSize, SceOff);
int sceIoRemove(const char *);
int sceIoRename(const char *, const char *);
#endif
