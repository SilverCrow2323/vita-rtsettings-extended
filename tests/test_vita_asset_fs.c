#include "vita_asset_fs.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const unsigned char file_data[] = "vita-asset-test";
static int close_error;
static int mkdir_error;
static int wrong_existing_type;
static int stat_error;
static int open_calls, read_calls, close_calls, mkdir_calls;

SceUID sceIoOpen(const char *file, int flags, SceMode mode) {
    (void)flags; (void)mode; open_calls++;
    return strcmp(file, "ux0:data/sample.bin") == 0 ? 9 : -1;
}
int sceIoPread(SceUID fd, void *data, SceSize size, SceOff offset) {
    size_t remain;
    (void)fd; read_calls++;
    if (offset < 0 || (uint64_t)offset >= sizeof(file_data) - 1) return 0;
    remain = sizeof(file_data) - 1 - (size_t)offset;
    if (remain > size) remain = size;
    memcpy(data, file_data + (size_t)offset, remain);
    return (int)remain;
}
int sceIoClose(SceUID fd) { (void)fd; close_calls++; return close_error ? -1 : 0; }
int sceIoMkdir(const char *dir, SceMode mode) {
    (void)mode; mkdir_calls++;
    if (mkdir_error) return -1;
    if (strcmp(dir, "ur0:data") == 0 || strcmp(dir, "ur0:data/rt") == 0 ||
        strcmp(dir, "ur0:data/rt/assets") == 0) return 0;
    return -1;
}
int sceIoGetstat(const char *path, SceIoStat *st) {
    memset(st, 0, sizeof(*st));
    if (stat_error) return -1;
    if (strcmp(path, "ur0:data") == 0 || strcmp(path, "ur0:data/rt") == 0 ||
        strcmp(path, "ur0:data/rt/assets") == 0) {
        st->st_mode = wrong_existing_type ? SCE_S_IFREG : SCE_S_IFDIR;
        return 0;
    }
    if (strcmp(path, "ux0:data/sample.bin") == 0) {
        st->st_mode = SCE_S_IFREG;
        st->st_size = (SceOff)(sizeof(file_data) - 1);
        return 0;
    }
    return -1;
}

int main(void) {
    unsigned char buffer[32];
    size_t bytes = 0;
    memset(buffer, 0, sizeof(buffer));
    assert(rta_vita_asset_read(NULL, "ux0:data/sample.bin", 0, buffer, 4, &bytes) == 1);
    assert(bytes == 4 && memcmp(buffer, "vita", 4) == 0);
    assert(rta_vita_asset_read(NULL, "ux0:data/sample.bin", 5, buffer, sizeof(buffer), &bytes) == 1);
    assert(bytes == sizeof(file_data) - 1 - 5 && memcmp(buffer, "asset-test", bytes) == 0);
    assert(rta_vita_asset_read(NULL, "ux0:data/sample.bin", 999, buffer, sizeof(buffer), &bytes) == 0);
    assert(bytes == 0);
    assert(rta_vita_asset_read(NULL, "ux0:data/../tai/config.txt", 0, buffer, 4, &bytes) == -1);
    assert(rta_vita_asset_read(NULL, "host:/sample.bin", 0, buffer, 4, &bytes) == -1);
    assert(rta_vita_asset_read(NULL, "ux0:data/sample.bin", 0, NULL, 4, &bytes) == -1);
    assert(open_calls == 3 && read_calls == 3 && close_calls == 3);

    close_error = 1;
    assert(rta_vita_asset_read(NULL, "ux0:data/sample.bin", 0, buffer, 4, &bytes) == -1);
    close_error = 0;

    assert(rta_vita_asset_ensure_directory() == 0);
    assert(mkdir_calls == 3);
    mkdir_error = 1;
    assert(rta_vita_asset_ensure_directory() == 0); /* existing directories are accepted */
    wrong_existing_type = 1;
    assert(rta_vita_asset_ensure_directory() == -1); /* file at directory path is rejected */
    wrong_existing_type = 0; stat_error = 1;
    assert(rta_vita_asset_ensure_directory() == -1); /* stat failure is not treated as success */
    stat_error = 0;

    assert(rta_vita_asset_file_exists("ux0:data/sample.bin") == 1);
    assert(rta_vita_asset_file_exists("ux0:data/missing.bin") == -1);
    assert(rta_vita_asset_file_exists("ux0:data/../bad") == -1);
    puts("Vita asset filesystem mock tests passed.");
    return 0;
}
