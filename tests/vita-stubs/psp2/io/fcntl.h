#ifndef TEST_VITA_STUB_FCNTL_H
#define TEST_VITA_STUB_FCNTL_H
#include <stddef.h>
#include <stdint.h>
typedef int SceUID;
typedef uint32_t SceSize;
typedef int64_t SceOff;
typedef uint32_t SceMode;
#define SCE_O_RDONLY 0x0001
SceUID sceIoOpen(const char *file, int flags, SceMode mode);
int sceIoPread(SceUID fd, void *data, SceSize size, SceOff offset);
int sceIoClose(SceUID fd);
#endif
