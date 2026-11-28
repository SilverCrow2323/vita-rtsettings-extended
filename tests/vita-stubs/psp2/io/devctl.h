#ifndef TEST_VITA_IO_DEVCTL_H
#define TEST_VITA_IO_DEVCTL_H
#include <psp2common/kernel/iofilemgr.h>
int sceIoDevctl(const char *, unsigned int, void *, int, void *, int);
#endif
