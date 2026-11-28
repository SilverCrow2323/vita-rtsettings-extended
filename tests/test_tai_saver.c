#define _POSIX_C_SOURCE 200809L
#include "tai_saver.h"
#include "sha256.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void join_path(char *out, size_t size, const char *left, const char *right) {
    size_t a = strlen(left), b = strlen(right);
    assert(a + b + 2 <= size);
    memmove(out, left, a); out[a] = '/'; memmove(out + a + 1, right, b + 1);
}

static void write_text(const char *path, const char *text) {
    FILE *f;
    char parent[TS_PATH_MAX];
    char *slash;
    (void)snprintf(parent, sizeof(parent), "%s", path);
    slash = strrchr(parent, '/');
    if (slash) {
        *slash = '\0';
        if (mkdir(parent, 0700) != 0 && errno != EEXIST) {
            perror("mkdir test parent");
            abort();
        }
    }
    f = fopen(path, "wb");
    assert(f != NULL);
    assert(fwrite(text, 1, strlen(text), f) == strlen(text));
    assert(fclose(f) == 0);
}

static int exists(const char *path) {
    struct stat st;
    return lstat(path, &st) == 0;
}

static void read_text(const char *path, char *out, size_t size) {
    FILE *f = fopen(path, "rb");
    size_t n;
    assert(f != NULL);
    n = fread(out, 1, size - 1, f);
    out[n] = '\0';
    assert(!ferror(f));
    fclose(f);
}

static unsigned count_directories(const char *path) {
    DIR *d = opendir(path);
    struct dirent *e;
    unsigned n = 0;
    assert(d != NULL);
    while ((e = readdir(d)) != NULL) {
        char child[TS_PATH_MAX];
        struct stat st;
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        join_path(child, sizeof(child), path, e->d_name);
        if (lstat(child, &st) == 0 && S_ISDIR(st.st_mode)) ++n;
    }
    closedir(d);
    return n;
}

static void test_corruption_detection(void) {
    char temp[] = "/tmp/rtse-tai-corrupt-XXXXXX";
    char source[TS_PATH_MAX], root[TS_PATH_MAX], file[TS_PATH_MAX];
    char payload[TS_PATH_MAX], snapshot_file[TS_PATH_MAX], manifest_file[TS_PATH_MAX];
    char manifest[8192], *id_value;
    ts_snapshot_result first, second, third;
    uint32_t removed = 99;
    char *base = mkdtemp(temp);
    assert(base != NULL);
    join_path(source, sizeof(source), base, "tai");
    join_path(root, sizeof(root), base, "archive");
    assert(mkdir(source, 0700) == 0);
    join_path(file, sizeof(file), source, "config.txt");
    write_text(file, "original configuration\n");
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 5, "integrity fixture", &first) == TS_OK);
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 5, "second integrity fixture", &second) == TS_OK);
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 5, "third integrity fixture", &third) == TS_OK);

    /* A changed payload is rejected, and automatic rotation leaves it for diagnosis. */
    join_path(payload, sizeof(payload), first.snapshot_path, "payload");
    join_path(snapshot_file, sizeof(snapshot_file), payload, "config.txt");
    write_text(snapshot_file, "tampered configuration\n");
    assert(ts_verify_snapshot(first.snapshot_path, NULL) == TS_ERR_INTEGRITY);
    assert(ts_apply_retention(root, 2, &removed) == TS_OK);
    assert(removed == 0);
    assert(exists(first.snapshot_path));

    /* The manifest identity must agree with the snapshot directory name. */
    join_path(manifest_file, sizeof(manifest_file), second.snapshot_path, "manifest.json");
    read_text(manifest_file, manifest, sizeof(manifest));
    id_value = strstr(manifest, "\"snapshot_id\": \"");
    assert(id_value != NULL);
    id_value += strlen("\"snapshot_id\": \"");
    id_value[0] = id_value[0] == 'X' ? 'Y' : 'X';
    write_text(manifest_file, manifest);
    assert(ts_verify_snapshot(second.snapshot_path, NULL) == TS_ERR_MANIFEST);
}

static void test_path_overlap_rejected(void) {
    char temp[] = "/tmp/rtse-path-overlap-XXXXXX";
    char source[TS_PATH_MAX], nested_storage[TS_PATH_MAX];
    char storage[TS_PATH_MAX], nested_source[TS_PATH_MAX], unsafe_source[TS_PATH_MAX];
    ts_snapshot_result snapshot;
    char *base = mkdtemp(temp);
    assert(base != NULL);
    join_path(source, sizeof(source), base, "tai");
    join_path(nested_storage, sizeof(nested_storage), source, "archives");
    assert(mkdir(source, 0700) == 0);
    assert(ts_create_snapshot(source, nested_storage, TS_ARCHIVE_AUTOMATIC, 2,
                              "must reject nested storage", &snapshot) == TS_ERR_ARGUMENT);
    assert(!exists(nested_storage));

    join_path(storage, sizeof(storage), base, "archive-root");
    join_path(nested_source, sizeof(nested_source), storage, "tai");
    assert(ts_create_snapshot(nested_source, storage, TS_ARCHIVE_AUTOMATIC, 2,
                              "must reject nested source", &snapshot) == TS_ERR_ARGUMENT);
    assert(!exists(storage));

    assert(snprintf(unsafe_source, sizeof(unsafe_source), "%s/../outside", source) < (int)sizeof(unsafe_source));
    assert(ts_create_snapshot(unsafe_source, storage, TS_ARCHIVE_AUTOMATIC, 2,
                              "must reject traversal path", &snapshot) == TS_ERR_ARGUMENT);
}

static void test_sha256(void) {
    ts_sha256_ctx ctx;
    uint8_t digest[32];
    char hex[65];
    ts_sha256_init(&ctx);
    ts_sha256_update(&ctx, "abc", 3);
    ts_sha256_final(&ctx, digest);
    ts_sha256_hex(digest, hex);
    assert(strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);
}

int main(void) {
    char temp[] = "/tmp/rtse-tai-saver-XXXXXX";
    char root[TS_PATH_MAX], source[TS_PATH_MAX], source_with_slash[TS_PATH_MAX];
    char path[TS_PATH_MAX], snapshot[TS_PATH_MAX];
    char content[256];
    ts_snapshot_result first, second, third, fourth, fifth, safety;
    ts_snapshot_info info;
    ts_compare_result comparison;
    uint32_t removed = 0;
    char *base = mkdtemp(temp);
    assert(base != NULL);
    snprintf(root, sizeof(root), "%s/archive", base);
    snprintf(source, sizeof(source), "%s/tai", base);
    assert(mkdir(source, 0700) == 0);

    test_sha256();
    test_path_overlap_rejected();
    join_path(path, sizeof(path), source, "config.txt");
    write_text(path, "# taiHEN configuration\n*main\nux0:tai/net_patch.skprx\n");
    join_path(path, sizeof(path), source, "profiles/user.txt");
    write_text(path, "profile=default\n");
    join_path(path, sizeof(path), source, "net_patch.skprx");
    write_text(path, "plugin-binary-v1\n");

    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 2, "Initial working setup", &first) == TS_OK);
    assert(first.file_count == 3 && first.total_bytes > 0);
    assert(ts_verify_snapshot(first.snapshot_path, &info) == TS_OK);
    assert(info.archive_class == TS_ARCHIVE_AUTOMATIC && info.file_count == 3);

    join_path(path, sizeof(path), source, "config.txt");
    write_text(path, "# changed setup\n*main\nux0:tai/broken.skprx\n");
    join_path(path, sizeof(path), source, "net_patch.skprx");
    assert(unlink(path) == 0);
    join_path(path, sizeof(path), source, "broken.skprx");
    write_text(path, "plugin-binary-v2\n");
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 2, "Modified setup", &second) == TS_OK);
    assert(ts_compare_snapshots(first.snapshot_path, second.snapshot_path, &comparison) == TS_OK);
    assert(comparison.changed == 1 && comparison.only_in_left == 1 && comparison.only_in_right == 1 && comparison.unchanged == 1);

    assert(ts_classify_snapshot(root, first.snapshot_id, TS_ARCHIVE_KNOWN_GOOD) == TS_OK);
    join_path(snapshot, sizeof(snapshot), root, "known-good");
    join_path(snapshot, sizeof(snapshot), snapshot, first.snapshot_id);
    assert(exists(snapshot));
    assert(ts_verify_snapshot(snapshot, &info) == TS_OK && info.archive_class == TS_ARCHIVE_KNOWN_GOOD);

    /* The failed classification must remain outside the Automatic retention pool. */
    assert(ts_classify_snapshot(root, second.snapshot_id, TS_ARCHIVE_FAILED) == TS_OK);
    join_path(snapshot, sizeof(snapshot), root, "failed");
    join_path(snapshot, sizeof(snapshot), snapshot, second.snapshot_id);
    assert(exists(snapshot));

    join_path(path, sizeof(path), source, "config.txt");
    write_text(path, "automatic-v3\n");
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 5, "third automatic", &third) == TS_OK);
    join_path(path, sizeof(path), source, "config.txt");
    write_text(path, "automatic-v4\n");
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 5, "fourth automatic", &fourth) == TS_OK);
    join_path(path, sizeof(path), source, "config.txt");
    write_text(path, "automatic-v5\n");
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 5, "fifth automatic", &fifth) == TS_OK);
    join_path(path, sizeof(path), root, "automatic");
    assert(count_directories(path) == 3);
    assert(ts_apply_retention(root, 2, &removed) == TS_OK);
    assert(removed == 1);
    join_path(snapshot, sizeof(snapshot), path, third.snapshot_id);
    assert(!exists(snapshot));
    join_path(snapshot, sizeof(snapshot), path, fourth.snapshot_id);
    assert(exists(snapshot));
    join_path(snapshot, sizeof(snapshot), path, fifth.snapshot_id);
    assert(exists(snapshot));
    join_path(snapshot, sizeof(snapshot), root, "known-good");
    join_path(snapshot, sizeof(snapshot), snapshot, first.snapshot_id);
    assert(exists(snapshot));
    join_path(snapshot, sizeof(snapshot), root, "failed");
    join_path(snapshot, sizeof(snapshot), snapshot, second.snapshot_id);
    assert(exists(snapshot));

    /* Restore normalizes a trailing slash and creates a mandatory Pre-Restore snapshot. */
    assert(snprintf(source_with_slash, sizeof(source_with_slash), "%s/", source) < (int)sizeof(source_with_slash));
    assert(ts_restore_snapshot(source_with_slash, root, TS_ARCHIVE_KNOWN_GOOD, first.snapshot_id, &safety) == TS_OK);
    assert(safety.snapshot_id[0] != '\0');
    assert(strstr(safety.snapshot_path, "/pre-restore/") != NULL);
    join_path(path, sizeof(path), source, "config.txt");
    read_text(path, content, sizeof(content));
    assert(strstr(content, "# taiHEN configuration") != NULL);
    join_path(path, sizeof(path), source, "profiles/user.txt");
    read_text(path, content, sizeof(content));
    assert(strcmp(content, "profile=default\n") == 0);
    join_path(path, sizeof(path), source, "net_patch.skprx");
    read_text(path, content, sizeof(content));
    assert(strcmp(content, "plugin-binary-v1\n") == 0);

    assert(ts_verify_snapshot(safety.snapshot_path, &info) == TS_OK);
    assert(info.archive_class == TS_ARCHIVE_PRE_RESTORE);

    /* A symlink or special file is rejected rather than followed into an unsafe path. */
    join_path(path, sizeof(path), source, "escape-link");
    assert(symlink("/etc/passwd", path) == 0);
    assert(ts_create_snapshot(source, root, TS_ARCHIVE_AUTOMATIC, 2, "must reject symlink", &safety) == TS_ERR_UNSUPPORTED_FILE);
    unlink(path);

    test_corruption_detection();
    printf("Tai Saver Core tests passed.\n");
    printf("Snapshots exercised: Automatic, Known-Good, Failed, Pre-Restore.\n");
    printf("Retention verified: only Automatic archives are eligible for pruning.\n");
    return 0;
}
