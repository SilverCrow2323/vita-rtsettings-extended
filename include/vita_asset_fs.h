#ifndef RTSE_VITA_ASSET_FS_H
#define RTSE_VITA_ASSET_FS_H

#include "custom_assets.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * VitaSDK filesystem bridge for Custom Assets Manager.
 *
 * The read callback is compatible with rta_asset_hash_reader and opens the
 * requested file for each bounded read. It never allocates the whole asset.
 * Directory bootstrap is restricted to Rt-owned ur0:data/RtSettingsExtended
 * paths; it does not create or modify arbitrary caller-supplied directories.
 */
int rta_vita_asset_read(void *context, const char *path, uint64_t offset,
                        unsigned char *buffer, size_t capacity, size_t *bytes_read);

/* Creates ur0:data/rt/assets, accepting an existing directory at each level. */
int rta_vita_asset_ensure_directory(void);

/* Returns 1 when present; -1 when absence or an I/O failure cannot be distinguished. */
int rta_vita_asset_file_exists(const char *path);

#ifdef __cplusplus
}
#endif
#endif
