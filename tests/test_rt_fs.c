#define _POSIX_C_SOURCE 200809L
#include "rt_fs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void) {
    char root[] = "/tmp/rtfs-test-XXXXXX";
    char file[512], renamed[512], sub[512];
    const char payload[] = "RtFS partial-I/O contract";
    char readback[sizeof(payload)] = {0};
    rtfs_stat st;
    rtfs_dir dir;
    rtfs_dirent ent;
    uint64_t free_bytes = 0;
    int fd, seen = 0, result;
    assert(mkdtemp(root) != NULL);
    snprintf(file, sizeof(file), "%s/data.bin", root);
    snprintf(renamed, sizeof(renamed), "%s/renamed.bin", root);
    snprintf(sub, sizeof(sub), "%s/sub", root);

    fd = rtfs_open(file, RTFS_OPEN_WRITE | RTFS_OPEN_CREATE | RTFS_OPEN_EXCLUSIVE, 0600);
    assert(fd >= 0);
    assert(rtfs_write(fd, payload, sizeof(payload)) == (int64_t)sizeof(payload));
    assert(rtfs_sync(fd) == 0);
    assert(rtfs_close(fd) == 0);

    assert(rtfs_stat_path(file, &st) == 0);
    assert(st.type == RTFS_TYPE_FILE && st.size == sizeof(payload));
    fd = rtfs_open(file, RTFS_OPEN_READ, 0);
    assert(fd >= 0);
    assert(rtfs_read(fd, readback, sizeof(readback)) == (int64_t)sizeof(readback));
    assert(memcmp(payload, readback, sizeof(payload)) == 0);
    assert(rtfs_close(fd) == 0);

    assert(rtfs_rename(file, renamed) == 0);
    assert(rtfs_stat_path(renamed, &st) == 0 && st.type == RTFS_TYPE_FILE);
    assert(rtfs_mkdir(sub, 0700) == 0);
    assert(rtfs_dir_open(&dir, root) == 0);
    while ((result = rtfs_dir_read(&dir, &ent)) > 0) {
        if (!strcmp(ent.name, "renamed.bin")) ++seen;
    }
    assert(result == 0 && seen == 1);
    assert(rtfs_dir_close(&dir) == 0);
    assert(rtfs_free_bytes(root, &free_bytes) == 0 && free_bytes > 0);
    assert(rtfs_remove(renamed) == 0);
    assert(rtfs_rmdir(sub) == 0);
    assert(rtfs_rmdir(root) == 0);
    puts("RT filesystem abstraction tests passed");
    return 0;
}
