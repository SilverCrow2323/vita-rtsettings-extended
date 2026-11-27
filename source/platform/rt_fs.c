#define _POSIX_C_SOURCE 200809L
#include "rt_fs.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#if defined(RTSE_PLATFORM_VITA)
#include <psp2/io/dirent.h>
#include <psp2/io/devctl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

static int vita_type(unsigned mode) {
    if (SCE_S_ISDIR(mode)) return RTFS_TYPE_DIRECTORY;
    if (SCE_S_ISREG(mode)) return RTFS_TYPE_FILE;
    if (SCE_S_ISLNK(mode)) return RTFS_TYPE_SYMLINK;
    return RTFS_TYPE_OTHER;
}

int rtfs_open(const char *path, unsigned flags, unsigned mode) {
    int native_flags = 0;
    if (!path || !*path || !(flags & (RTFS_OPEN_READ | RTFS_OPEN_WRITE))) return -1;
    if ((flags & (RTFS_OPEN_READ | RTFS_OPEN_WRITE)) == (RTFS_OPEN_READ | RTFS_OPEN_WRITE))
        native_flags |= SCE_O_RDWR;
    else if (flags & RTFS_OPEN_WRITE) native_flags |= SCE_O_WRONLY;
    else native_flags |= SCE_O_RDONLY;
    if (flags & RTFS_OPEN_CREATE) native_flags |= SCE_O_CREAT;
    if (flags & RTFS_OPEN_EXCLUSIVE) native_flags |= SCE_O_EXCL;
    if (flags & RTFS_OPEN_TRUNCATE) native_flags |= SCE_O_TRUNC;
    return sceIoOpen(path, native_flags, (SceMode)mode);
}

int64_t rtfs_read(int fd, void *buffer, size_t capacity) {
    if ((!buffer && capacity) || capacity > INT_MAX) return -1;
    return (int64_t)sceIoRead(fd, buffer, (SceSize)capacity);
}
int64_t rtfs_write(int fd, const void *buffer, size_t length) {
    if ((!buffer && length) || length > INT_MAX) return -1;
    return (int64_t)sceIoWrite(fd, buffer, (SceSize)length);
}
int64_t rtfs_seek(int fd, int64_t offset, int whence) {
    int native_whence;
    if (whence == 0) native_whence = SCE_SEEK_SET;
    else if (whence == 1) native_whence = SCE_SEEK_CUR;
    else if (whence == 2) native_whence = SCE_SEEK_END;
    else return -1;
    return (int64_t)sceIoLseek(fd, (SceOff)offset, native_whence);
}
int rtfs_sync(int fd) { return sceIoSyncByFd(fd, 0); }
int rtfs_close(int fd) { return sceIoClose(fd); }

int rtfs_stat_path(const char *path, rtfs_stat *out) {
    SceIoStat st;
    if (!path || !out) return -1;
    memset(&st, 0, sizeof(st));
    if (sceIoGetstat(path, &st) < 0) return -1;
    out->type = (rtfs_type)vita_type(st.st_mode);
    out->size = st.st_size < 0 ? 0 : (uint64_t)st.st_size;
    out->mtime = 0; /* SceDateTime is not flattened into a guessed Unix timestamp. */
    return 0;
}
int rtfs_mkdir(const char *path, unsigned mode) { return sceIoMkdir(path, (SceMode)mode); }
int rtfs_remove(const char *path) { return sceIoRemove(path); }
int rtfs_rmdir(const char *path) { return sceIoRmdir(path); }
int rtfs_rename(const char *source, const char *destination) { return sceIoRename(source, destination); }

int rtfs_dir_open(rtfs_dir *dir, const char *path) {
    SceUID fd;
    if (!dir || !path || !*path) return -1;
    fd = sceIoDopen(path);
    if (fd < 0) return fd;
    dir->handle = fd;
    dir->opaque = NULL;
    if (strlen(path) >= sizeof(dir->path)) { (void)sceIoDclose(fd); dir->handle = -1; return -1; }
    memcpy(dir->path, path, strlen(path) + 1);
    return 0;
}
int rtfs_dir_read(rtfs_dir *dir, rtfs_dirent *out) {
    SceIoDirent entry;
    int result;
    if (!dir || !out || dir->handle < 0) return -1;
    memset(&entry, 0, sizeof(entry));
    result = sceIoDread((SceUID)dir->handle, &entry);
    if (result <= 0) return result;
    if (!memchr(entry.d_name, '\0', sizeof(entry.d_name))) return -1;
    memset(out, 0, sizeof(*out));
    memcpy(out->name, entry.d_name, strlen(entry.d_name) + 1);
    out->type = (rtfs_type)vita_type(entry.d_stat.st_mode);
    out->size = entry.d_stat.st_size < 0 ? 0 : (uint64_t)entry.d_stat.st_size;
    return 1;
}
int rtfs_dir_close(rtfs_dir *dir) {
    int result;
    if (!dir || dir->handle < 0) return -1;
    result = sceIoDclose((SceUID)dir->handle);
    dir->handle = -1;
    dir->opaque = NULL;
    return result;
}
int rtfs_free_bytes(const char *device, uint64_t *bytes) {
    SceIoDevInfo info;
    if (!device || !*device || !bytes) return -1;
    memset(&info, 0, sizeof(info));
    if (sceIoDevctl(device, 0x3001, NULL, 0, &info, sizeof(info)) < 0 || info.free_size < 0) return -1;
    *bytes = (uint64_t)info.free_size;
    return 0;
}

#else
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

static int neg_errno(void) { return errno ? -errno : -1; }
static rtfs_type host_type(mode_t mode) {
    if (S_ISDIR(mode)) return RTFS_TYPE_DIRECTORY;
    if (S_ISREG(mode)) return RTFS_TYPE_FILE;
#ifdef S_ISLNK
    if (S_ISLNK(mode)) return RTFS_TYPE_SYMLINK;
#endif
    return RTFS_TYPE_OTHER;
}
int rtfs_open(const char *path, unsigned flags, unsigned mode) {
    int f = 0;
    if (!path || !*path || !(flags & (RTFS_OPEN_READ | RTFS_OPEN_WRITE))) return -1;
    if ((flags & (RTFS_OPEN_READ | RTFS_OPEN_WRITE)) == (RTFS_OPEN_READ | RTFS_OPEN_WRITE)) f |= O_RDWR;
    else if (flags & RTFS_OPEN_WRITE) f |= O_WRONLY;
    else f |= O_RDONLY;
    if (flags & RTFS_OPEN_CREATE) f |= O_CREAT;
    if (flags & RTFS_OPEN_EXCLUSIVE) f |= O_EXCL;
    if (flags & RTFS_OPEN_TRUNCATE) f |= O_TRUNC;
    { int fd = open(path, f, (mode_t)mode); return fd < 0 ? neg_errno() : fd; }
}
int64_t rtfs_read(int fd, void *buffer, size_t capacity) {
    ssize_t n;
    if ((!buffer && capacity) || capacity > (size_t)SSIZE_MAX) return -1;
    n = read(fd, buffer, capacity);
    return n < 0 ? neg_errno() : (int64_t)n;
}
int64_t rtfs_write(int fd, const void *buffer, size_t length) {
    ssize_t n;
    if ((!buffer && length) || length > (size_t)SSIZE_MAX) return -1;
    n = write(fd, buffer, length);
    return n < 0 ? neg_errno() : (int64_t)n;
}
int64_t rtfs_seek(int fd, int64_t offset, int whence) {
    off_t p = lseek(fd, (off_t)offset, whence);
    return p < 0 ? neg_errno() : (int64_t)p;
}
int rtfs_sync(int fd) { return fsync(fd) == 0 ? 0 : neg_errno(); }
int rtfs_close(int fd) { return close(fd) == 0 ? 0 : neg_errno(); }
int rtfs_stat_path(const char *path, rtfs_stat *out) {
    struct stat st;
    if (!path || !out) return -1;
    if (lstat(path, &st) != 0) return neg_errno();
    out->type = host_type(st.st_mode);
    out->size = st.st_size < 0 ? 0 : (uint64_t)st.st_size;
    out->mtime = st.st_mtime < 0 ? 0 : (uint64_t)st.st_mtime;
    return 0;
}
int rtfs_mkdir(const char *path, unsigned mode) { return mkdir(path, (mode_t)mode) == 0 ? 0 : neg_errno(); }
int rtfs_remove(const char *path) { return unlink(path) == 0 ? 0 : neg_errno(); }
int rtfs_rmdir(const char *path) { return rmdir(path) == 0 ? 0 : neg_errno(); }
int rtfs_rename(const char *source, const char *destination) { return rename(source, destination) == 0 ? 0 : neg_errno(); }
int rtfs_dir_open(rtfs_dir *dir, const char *path) {
    DIR *d;
    if (!dir || !path || !*path) return -1;
    d = opendir(path);
    if (!d) return neg_errno();
    dir->handle = -1;
    dir->opaque = d;
    if (strlen(path) >= sizeof(dir->path)) { (void)closedir(d); dir->opaque = NULL; return -1; }
    memcpy(dir->path, path, strlen(path) + 1);
    return 0;
}
int rtfs_dir_read(rtfs_dir *dir, rtfs_dirent *out) {
    struct dirent *entry;
    struct stat st;
    char path[PATH_MAX];
    size_t n;
    if (!dir || !dir->opaque || !out) return -1;
    errno = 0;
    entry = readdir((DIR *)dir->opaque);
    if (!entry) return errno ? neg_errno() : 0;
    n = strlen(entry->d_name);
    if (!n || n >= sizeof(out->name)) return -1;
    memset(out, 0, sizeof(*out));
    memcpy(out->name, entry->d_name, n + 1);
    if (snprintf(path, sizeof(path), "%s/%s", dir->path, entry->d_name) < 0) return -1;
    if (lstat(path, &st) == 0) {
        out->type = host_type(st.st_mode);
        out->size = st.st_size < 0 ? 0 : (uint64_t)st.st_size;
    } else {
        return neg_errno();
    }
    return 1;
}
int rtfs_dir_close(rtfs_dir *dir) {
    int result;
    if (!dir || !dir->opaque) return -1;
    result = closedir((DIR *)dir->opaque);
    dir->opaque = NULL;
    return result == 0 ? 0 : neg_errno();
}
int rtfs_free_bytes(const char *device, uint64_t *bytes) {
    struct statvfs st;
    if (!device || !bytes) return -1;
    if (statvfs(device, &st) != 0) return neg_errno();
    if (st.f_frsize && st.f_bavail > UINT64_MAX / st.f_frsize) *bytes = UINT64_MAX;
    else *bytes = (uint64_t)st.f_bavail * (uint64_t)st.f_frsize;
    return 0;
}
#endif
