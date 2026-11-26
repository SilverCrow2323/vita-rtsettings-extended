#ifndef RTSE_CUSTOM_ASSETS_H
#define RTSE_CUSTOM_ASSETS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTA_MAX_ASSETS 64
#define RTA_ID_MAX 47
#define RTA_TITLE_MAX 79
#define RTA_PATH_MAX 255
#define RTA_OWNER_MAX 47
#define RTA_SOURCE_MAX 191
#define RTA_FORMAT_MAX 15
#define RTA_SHA256_HEX 64

typedef enum {
    RTA_TYPE_BOOT_SPLASH = 1,
    RTA_TYPE_BOOT_ANIMATION,
    RTA_TYPE_BOOT_SOUND,
    RTA_TYPE_STARTUP_WARNING,
    RTA_TYPE_INTERFACE_THEME,
    RTA_TYPE_OTHER
} rta_asset_type;

typedef enum {
    RTA_STATE_AVAILABLE = 1,
    RTA_STATE_ACTIVE,
    RTA_STATE_BACKUP,
    RTA_STATE_ORPHANED,
    RTA_STATE_INCOMPATIBLE,
    RTA_STATE_INVALID
} rta_asset_state;

typedef struct {
    const char *id;
    const char *title;
    const char *path;
    const char *owner;
    const char *source;
    const char *format;
    const char *sha256;
    rta_asset_type type;
    rta_asset_state state;
    uint64_t size_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t duration_ms;
    uint32_t sample_rate_hz;
    uint32_t channels;
    int managed;
    int compatible;
    int hash_verified;
    int protected_asset;
} rta_asset_observation;

typedef struct {
    char id[RTA_ID_MAX + 1];
    char title[RTA_TITLE_MAX + 1];
    char path[RTA_PATH_MAX + 1];
    char owner[RTA_OWNER_MAX + 1];
    char source[RTA_SOURCE_MAX + 1];
    char format[RTA_FORMAT_MAX + 1];
    char sha256[RTA_SHA256_HEX + 1];
    rta_asset_type type;
    rta_asset_state state;
    uint64_t size_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t duration_ms;
    uint32_t sample_rate_hz;
    uint32_t channels;
    int managed;
    int compatible;
    int hash_verified;
    int protected_asset;
} rta_asset;

typedef struct {
    size_t count;
    size_t active_count;
    size_t backup_count;
    size_t orphaned_count;
    size_t incompatible_count;
    size_t unverified_count;
    size_t active_conflict_count;
    rta_asset assets[RTA_MAX_ASSETS];
} rta_asset_registry;

typedef enum {
    RTA_OK = 0,
    RTA_ERR_ARGUMENT = -1,
    RTA_ERR_CAPACITY = -2,
    RTA_ERR_INVALID_ASSET = -3,
    RTA_ERR_DUPLICATE_ID = -4,
    RTA_ERR_DUPLICATE_PATH = -5,
    RTA_ERR_NOT_FOUND = -6,
    RTA_ERR_UNSAFE = -7,
    RTA_ERR_INCOMPATIBLE = -8,
    RTA_ERR_UNVERIFIED = -9,
    RTA_ERR_BACKEND = -10,
    RTA_ERR_ROLLBACK = -11,
    RTA_ERR_PROTECTED = -12,
    RTA_ERR_INTEGRITY = -13
} rta_result;

/* read callback: 1 = bytes returned, 0 = EOF, -1 = I/O error. */
typedef int (*rta_asset_read_fn)(void *context, const char *path, uint64_t offset,
                                 unsigned char *buffer, size_t capacity, size_t *bytes_read);

/* Hashes a file incrementally through the platform reader; no whole-file allocation. */
rta_result rta_asset_hash_reader(rta_asset_read_fn reader, void *context, const char *path,
                                 char sha256_hex[65], uint64_t *size_bytes);
rta_result rta_asset_verify_reader(rta_asset_read_fn reader, void *context, const char *path,
                                   const char *expected_sha256, uint64_t expected_size);

typedef struct {
    void *context;
    int (*activate)(void *context, rta_asset_type type, const char *path);
    int (*verify_active)(void *context, rta_asset_type type, const char *path);
    int (*restore)(void *context, rta_asset_type type, const char *previous_path);
    int (*remove)(void *context, const char *path);
} rta_asset_backend;

/* Build a bounded registry from platform observations. No filesystem writes occur. */
rta_result rta_asset_registry_build(const rta_asset_observation *observations,
                                    size_t count,
                                    rta_asset_registry *out);

/* Validate metadata and type-specific conservative size/format constraints. */
rta_result rta_asset_validate(const rta_asset_observation *observation);

/* Activate a verified compatible asset with rollback to the previous asset/default. */
rta_result rta_asset_activate(rta_asset_registry *registry,
                              const char *asset_id,
                              const rta_asset_backend *backend);

/* Remove only Rt-managed, inactive, unprotected assets through the supplied backend. */
rta_result rta_asset_remove(rta_asset_registry *registry,
                            const char *asset_id,
                            const rta_asset_backend *backend);

const char *rta_asset_type_name(rta_asset_type type);
const char *rta_asset_state_name(rta_asset_state state);
const char *rta_result_name(rta_result result);

#ifdef __cplusplus
}
#endif
#endif
