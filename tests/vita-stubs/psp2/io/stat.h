#ifndef TEST_VITA_STUB_STAT_H
#define TEST_VITA_STUB_STAT_H
#include <stdint.h>
#include "fcntl.h"
#define SCE_S_IFMT 0170000
#define SCE_S_IFDIR 0010000
#define SCE_S_IFREG 0020000
#define SCE_S_ISDIR(m) (((m) & SCE_S_IFMT) == SCE_S_IFDIR)
typedef struct { SceMode st_mode; uint32_t st_attr; SceOff st_size; } SceIoStat;
int sceIoMkdir(const char *dir, SceMode mode);
int sceIoGetstat(const char *file, SceIoStat *st);
#endif
