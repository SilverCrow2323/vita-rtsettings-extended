#include "vita_asset_fs.h"

#ifndef RTSE_PLATFORM_VITA
#error "vita_asset_fs.c must only be compiled for the Vita target"
#endif

#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <limits.h>
#include <string.h>

static int safe_vita_path(const char *path) {
    size_t i, n;
    int rooted = 0;
    if (!path || !(n = strlen(path)) || n > RTA_PATH_MAX) return 0;
    if (strncmp(path, "ur0:", 4) == 0 || strncmp(path, "ux0:", 4) == 0 ||
        strncmp(path, "uma0:", 5) == 0 || strncmp(path, "imc0:", 5) == 0 ||
        strncmp(path, "app0:", 5) == 0) rooted = 1;
    if (!rooted) return 0;
    for (i = 0; i < n; ++i) {
        unsigned char ch = (unsigned char)path[i];
        if (ch < 32 || ch == 127) return 0;
        if (path[i] == '.' && i + 1 < n && path[i + 1] == '.') return 0;
    }
    return 1;
}

int rta_vita_asset_read(void *context, const char *path, uint64_t offset,
                        unsigned char *buffer, size_t capacity, size_t *bytes_read) {
    SceUID fd;
    int result;
    int close_result;
    (void)context;
    if (!bytes_read || (!buffer && capacity) || !safe_vita_path(path) ||
        capacity > (size_t)INT_MAX || offset > (uint64_t)INT_MAX)
        return -1;
    *bytes_read = 0;
    if (capacity == 0) return 1;
    fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0) return -1;
    result = sceIoPread(fd, buffer, (SceSize)capacity, (SceOff)offset);
    close_result = sceIoClose(fd);
    if (result < 0 || close_result < 0) return -1;
    if ((size_t)result > capacity) return -1;
    *bytes_read = (size_t)result;
    return result == 0 ? 0 : 1;
}

static int ensure_one_directory(const char *path) {
    SceIoStat st;
    if (sceIoMkdir(path, 0777) == 0) return 0;
    memset(&st, 0, sizeof(st));
    if (sceIoGetstat(path, &st) < 0) return -1;
    return SCE_S_ISDIR(st.st_mode) ? 0 : -1;
}

int rta_vita_asset_ensure_directory(void) {
    if (ensure_one_directory("ur0:data") != 0) return -1;
    if (ensure_one_directory("ur0:data/rt") != 0) return -1;
    if (ensure_one_directory("ur0:data/rt/assets") != 0) return -1;
    return 0;
}

int rta_vita_asset_file_exists(const char *path) {
    SceIoStat st;
    if (!safe_vita_path(path)) return -1;
    memset(&st, 0, sizeof(st));
    if (sceIoGetstat(path, &st) == 0) return 1;
    return 0;
}
