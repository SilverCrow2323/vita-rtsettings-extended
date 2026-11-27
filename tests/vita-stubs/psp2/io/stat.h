#ifndef TEST_VITA_IO_STAT_H
#define TEST_VITA_IO_STAT_H
#include <psp2common/kernel/iofilemgr.h>
int sceIoGetstat(const char *, SceIoStat *);
int sceIoGetstatByFd(SceUID, SceIoStat *);
int sceIoMkdir(const char *, SceMode);
int sceIoRmdir(const char *);
#endif
