#ifndef RTSE_RT_FS_H
#define RTSE_RT_FS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTFS_NAME_MAX 256

typedef enum {
    RTFS_TYPE_OTHER = 0,
    RTFS_TYPE_FILE = 1,
    RTFS_TYPE_DIRECTORY = 2,
    RTFS_TYPE_SYMLINK = 3
} rtfs_type;

typedef struct {
    rtfs_type type;
    uint64_t size;
    uint64_t mtime;
} rtfs_stat;

typedef struct {
    char name[RTFS_NAME_MAX];
    rtfs_type type;
    uint64_t size;
} rtfs_dirent;

typedef struct {
    intptr_t handle;
    void *opaque;
    char path[1024];
} rtfs_dir;

enum {
    RTFS_OPEN_READ = 1u << 0,
    RTFS_OPEN_WRITE = 1u << 1,
    RTFS_OPEN_CREATE = 1u << 2,
    RTFS_OPEN_EXCLUSIVE = 1u << 3,
    RTFS_OPEN_TRUNCATE = 1u << 4
};

/* File descriptors and directory handles are owned by the caller. */
int rtfs_open(const char *path, unsigned flags, unsigned mode);
int64_t rtfs_read(int fd, void *buffer, size_t capacity);
int64_t rtfs_write(int fd, const void *buffer, size_t length);
int64_t rtfs_seek(int fd, int64_t offset, int whence);
int rtfs_sync(int fd);
int rtfs_close(int fd);

int rtfs_stat_path(const char *path, rtfs_stat *out);
int rtfs_mkdir(const char *path, unsigned mode);
int rtfs_remove(const char *path);
int rtfs_rmdir(const char *path);
/* Wraps the platform rename primitive; it does NOT promise atomic replacement or no-clobber. */
int rtfs_rename(const char *source, const char *destination);

int rtfs_dir_open(rtfs_dir *dir, const char *path);
/* Returns 1 for an entry, 0 at end, or a negative platform error. */
int rtfs_dir_read(rtfs_dir *dir, rtfs_dirent *out);
int rtfs_dir_close(rtfs_dir *dir);

/* Returns 0 and fills bytes on success; unsupported/unknown capacity is an error. */
int rtfs_free_bytes(const char *device, uint64_t *bytes);

#ifdef __cplusplus
}
#endif
#endif
