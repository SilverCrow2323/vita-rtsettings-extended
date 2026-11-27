#ifndef TEST_VITA_IOFILEMGR_H
#define TEST_VITA_IOFILEMGR_H
#include <psp2/types.h>
#define SCE_O_RDONLY 0x0001
#define SCE_O_WRONLY 0x0002
#define SCE_O_RDWR (SCE_O_RDONLY | SCE_O_WRONLY)
#define SCE_O_CREAT 0x0200
#define SCE_O_TRUNC 0x0400
#define SCE_O_EXCL 0x0800
#define SCE_O_WRLOCK 0x0020
#define SCE_O_NOWAIT 0x8000
#define SCE_SEEK_SET 0
#define SCE_SEEK_CUR 1
#define SCE_SEEK_END 2
#define SCE_S_IFMT 0170000
#define SCE_S_IFDIR 0010000
#define SCE_S_IFREG 0020000
#define SCE_S_IFLNK 0040000
#define SCE_S_ISDIR(m) (((m) & SCE_S_IFMT) == SCE_S_IFDIR)
#define SCE_S_ISREG(m) (((m) & SCE_S_IFMT) == SCE_S_IFREG)
#define SCE_S_ISLNK(m) (((m) & SCE_S_IFMT) == SCE_S_IFLNK)
typedef struct { SceOff max_size; SceOff free_size; SceSize cluster_size; void *unk; } SceIoDevInfo;
typedef struct { SceMode st_mode; unsigned int st_attr; SceOff st_size; unsigned int st_private[6]; } SceIoStat;
typedef struct { SceIoStat d_stat; char d_name[256]; void *d_private; int dummy; } SceIoDirent;
#endif
