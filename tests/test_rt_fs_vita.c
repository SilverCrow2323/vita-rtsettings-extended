#include "rt_fs.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/io/devctl.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int open_flags, open_mode, sync_result, close_result, dir_read_index, dir_read_error;
static int rename_result, mkdir_result, remove_result, rmdir_result, stat_result, devctl_result;
static SceOff seek_result = 123;
static SceIoStat mock_stat;
static SceIoDevInfo mock_devinfo = {0, 987654, 4096, NULL};

SceUID sceIoOpen(const char *p, int flags, SceMode mode) { (void)p; open_flags=flags; open_mode=(int)mode; return 7; }
int sceIoClose(SceUID fd) { (void)fd; return close_result; }
SceSSize sceIoRead(SceUID fd, void *b, SceSize n) { (void)fd; if(n>3)n=3; memset(b,'R',n); return (SceSSize)n; }
SceSSize sceIoWrite(SceUID fd, const void *b, SceSize n) { (void)fd;(void)b; return n>2?2:(SceSSize)n; }
SceOff sceIoLseek(SceUID fd, SceOff off, int whence) { (void)fd;(void)off;(void)whence;return seek_result; }
int sceIoSyncByFd(SceUID fd, int flag) { (void)fd;(void)flag;return sync_result; }
int sceIoGetstat(const char *p, SceIoStat *st) { (void)p;*st=mock_stat;return stat_result; }
int sceIoGetstatByFd(SceUID fd, SceIoStat *st) { (void)fd;*st=mock_stat;return stat_result; }
int sceIoMkdir(const char *p,SceMode m) {(void)p;(void)m;return mkdir_result;}
int sceIoRemove(const char *p) {(void)p;return remove_result;}
int sceIoRmdir(const char *p) {(void)p;return rmdir_result;}
int sceIoRename(const char *a,const char *b) {(void)a;(void)b;return rename_result;}
SceUID sceIoDopen(const char *p) {(void)p;return 9;}
int sceIoDread(SceUID fd,SceIoDirent *d) {
    (void)fd;
    if(dir_read_error) return dir_read_error;
    if(dir_read_index++ == 0) { memset(d,0,sizeof(*d));strcpy(d->d_name,"sample.dat");d->d_stat.st_mode=SCE_S_IFREG;d->d_stat.st_size=42;return 1; }
    return 0;
}
int sceIoDclose(SceUID fd) {(void)fd;return 0;}
int sceIoDevctl(const char *d,unsigned int cmd,void *in,int il,void *out,int ol) {
    (void)d;(void)in;(void)il;(void)ol;assert(cmd==0x3001);*(SceIoDevInfo *)out=mock_devinfo;return devctl_result;
}

int main(void) {
    char buf[8]={0}; rtfs_stat st; rtfs_dir d; rtfs_dirent e; uint64_t free_bytes=0; int fd;
    fd=rtfs_open("ur0:data/test",RTFS_OPEN_READ|RTFS_OPEN_WRITE|RTFS_OPEN_CREATE|RTFS_OPEN_EXCLUSIVE|RTFS_OPEN_TRUNCATE|RTFS_OPEN_WRITE_LOCK|RTFS_OPEN_NOWAIT,0600);
    assert(fd==7 && open_flags==(SCE_O_RDWR|SCE_O_CREAT|SCE_O_EXCL|SCE_O_TRUNC|SCE_O_WRLOCK|SCE_O_NOWAIT) && open_mode==0600);
    assert(rtfs_read(fd,buf,sizeof(buf))==3 && !memcmp(buf,"RRR",3));
    assert(rtfs_write(fd,"abcdef",6)==2);
    assert(rtfs_seek(fd,0,0)==seek_result);
    assert(rtfs_sync(fd)==0 && rtfs_close(fd)==0);
    sync_result=-41; assert(rtfs_sync(fd)==-41); sync_result=0;

    memset(&mock_stat,0,sizeof(mock_stat));mock_stat.st_mode=SCE_S_IFDIR;mock_stat.st_size=100;
    assert(rtfs_stat_path("ur0:data",&st)==0 && st.type==RTFS_TYPE_DIRECTORY && st.size==100);
    assert(rtfs_stat_fd(fd,&st)==0 && st.type==RTFS_TYPE_DIRECTORY && st.size==100);
    stat_result=-42;assert(rtfs_stat_path("ur0:missing",&st)==-42);stat_result=0;
    assert(rtfs_mkdir("ur0:data",0700)==0);
    assert(rtfs_remove("ur0:file")==0 && rtfs_rmdir("ur0:dir")==0 && rtfs_rename("a","b")==0);

    assert(rtfs_dir_open(&d,"ur0:data")==0);
    dir_read_index=0;dir_read_error=0;
    assert(rtfs_dir_read(&d,&e)==1 && !strcmp(e.name,"sample.dat") && e.type==RTFS_TYPE_FILE && e.size==42);
    assert(rtfs_dir_read(&d,&e)==0);
    assert(rtfs_dir_close(&d)==0);
    assert(rtfs_dir_open(&d,"ur0:data")==0);
    dir_read_error=-55;assert(rtfs_dir_read(&d,&e)==-55);dir_read_error=0;assert(rtfs_dir_close(&d)==0);

    assert(rtfs_free_bytes("ur0:",&free_bytes)==0 && free_bytes==987654);
    devctl_result=-77;assert(rtfs_free_bytes("ur0:",&free_bytes)==-77);devctl_result=0;
    puts("Vita-native filesystem adapter mock tests passed");
    return 0;
}
