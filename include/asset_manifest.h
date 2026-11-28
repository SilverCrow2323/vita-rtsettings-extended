#ifndef RTSE_ASSET_MANIFEST_H
#define RTSE_ASSET_MANIFEST_H

#include "custom_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTA_MANIFEST_VERSION 1U
#define RTA_MANIFEST_MAX_SIZE 65536U
#define RTA_MANIFEST_DIGEST_SIZE 32U

typedef struct {
    void *context;
    /* read: 0 success, nonzero failure; length must be <= capacity */
    int (*read)(void *context, const char *path, unsigned char *buffer,
                size_t capacity, size_t *length);
    /*
     * Must replace path atomically: on failure the previous complete file
     * remains intact; on success the new complete file is visible.
     * Implementations must not follow caller-controlled paths.
     */
    int (*replace_atomic)(void *context, const char *path,
                          const unsigned char *data, size_t length);
    /* exists: 1 present, 0 absent, -1 backend error. */
    int (*exists)(void *context, const char *path);
    /*
     * Single crash-consistent pair transaction. When preserve_previous is
     * true, publish the old primary as backup and data as the new primary.
     * Otherwise leave backup untouched. On failure the old pair is recoverable.
     */
    int (*replace_with_backup_atomic)(void *context, const char *primary_path,
                                      const char *backup_path,
                                      const unsigned char *data, size_t length,
                                      int preserve_previous);
} rta_asset_manifest_io;

/* Binary, versioned, little-endian manifest codec with SHA-256 integrity. */
rta_result rta_asset_manifest_encode(const rta_asset_registry *registry,
                                     unsigned char *buffer, size_t capacity,
                                     size_t *written);
rta_result rta_asset_manifest_decode(const unsigned char *buffer, size_t length,
                                     rta_asset_registry *registry);

/* Persistence wrapper. Caller supplies a bounded scratch buffer (>= max size). */
rta_result rta_asset_manifest_save(const rta_asset_registry *registry,
                                   const rta_asset_manifest_io *io,
                                   const char *path,
                                   unsigned char *scratch, size_t capacity);
rta_result rta_asset_manifest_load(const rta_asset_manifest_io *io,
                                   const char *path,
                                   unsigned char *scratch, size_t capacity,
                                   rta_asset_registry *registry);
/* Validate primary first, then backup; output changes only after valid decode. */
rta_result rta_asset_manifest_load_recover(const rta_asset_manifest_io *io,
                                           const char *primary_path,
                                           const char *backup_path,
                                           unsigned char *scratch, size_t capacity,
                                           rta_asset_registry *registry,
                                           int *used_backup);
/* Two bounded buffers are required. The backend must implement a genuinely
 * crash-consistent pair transaction; the core never emulates atomicity. */
rta_result rta_asset_manifest_save_rotate(const rta_asset_registry *registry,
                                          const rta_asset_manifest_io *io,
                                          const char *primary_path,
                                          const char *backup_path,
                                          unsigned char *scratch,
                                          unsigned char *previous_scratch,
                                          size_t capacity);

#ifdef __cplusplus
}
#endif
#endif
