#ifndef TEST_VITA_IO_DIRENT_H
#define TEST_VITA_IO_DIRENT_H
#include <psp2common/kernel/iofilemgr.h>
SceUID sceIoDopen(const char *);
int sceIoDread(SceUID, SceIoDirent *);
int sceIoDclose(SceUID);
#endif
