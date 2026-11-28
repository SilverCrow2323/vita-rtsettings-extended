#ifndef RTSE_CUSTOM_WARNING_ADAPTER_H
#define RTSE_CUSTOM_WARNING_ADAPTER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTCW_MAX_FILE_BYTES 8192
#define RTCW_MAX_TEXT_BYTES 4095
#define RTCW_MAX_PRESETS 16
#define RTCW_PRESET_NAME_MAX 63
#define RTCW_PATH_MAX 255

typedef enum {
    RTCW_MODE_DEFAULT = 0,
    RTCW_MODE_FIXED_FILE,
    RTCW_MODE_PRESET_ROTATION
} rtcw_mode;

typedef enum {
    RTCW_OK = 0,
    RTCW_ERR_ARGUMENT = -1,
    RTCW_ERR_PLUGIN_DISABLED = -2,
    RTCW_ERR_NOT_FOUND = -3,
    RTCW_ERR_BACKEND = -4,
    RTCW_ERR_INVALID_ENCODING = -5,
    RTCW_ERR_TEXT_TOO_LONG = -6,
    RTCW_ERR_INVALID_TEXT = -7,
    RTCW_ERR_MODE = -8,
    RTCW_ERR_ROLLBACK = -9,
    RTCW_ERR_INTEGRITY = -10,
    RTCW_ERR_CAPACITY = -11
} rtcw_result;

typedef struct {
    void *context;
    /* 1 active, 0 disabled/not installed, -1 detection error. */
    int (*plugin_active)(void *context);
    /* 1 directory found, 0 missing, -1 I/O error. Names are regular files only. */
    int (*list_directory)(void *context, const char *path,
                          char names[RTCW_MAX_PRESETS][RTCW_PRESET_NAME_MAX + 1],
                          size_t capacity, size_t *count);
    /* 1 file read, 0 missing, -1 I/O error. */
    int (*read_file)(void *context, const char *path, unsigned char *buffer,
                     size_t capacity, size_t *size);
    /* Backup exact bytes or an absent-file marker; 0 success, nonzero failure. */
    int (*backup_file)(void *context, const char *path);
    /* Must stage, verify and atomically replace the target file. */
    int (*write_atomic)(void *context, const char *path,
                        const unsigned char *bytes, size_t size);
    /* Restore exact previous bytes or remove the file if it did not exist. */
    int (*restore_backup)(void *context, const char *path);
} rtcw_backend;

typedef struct {
    rtcw_mode mode;
    char mount[5];
    char path[RTCW_PATH_MAX + 1];
    size_t preset_count;
    char preset_names[RTCW_MAX_PRESETS][RTCW_PRESET_NAME_MAX + 1];
    char current_text[RTCW_MAX_TEXT_BYTES + 1];
    size_t current_text_bytes;
} rtcw_state;

/* UTF-8 to/from the exact UTF-16LE + BOM format expected by VitaTweaks Custom Warning. */
rtcw_result rtcw_encode_utf8(const char *utf8, unsigned char *out,
                             size_t capacity, size_t *out_size);
rtcw_result rtcw_decode_utf16le(const unsigned char *bytes, size_t size,
                                char *utf8, size_t capacity, size_t *out_size);
rtcw_result rtcw_validate_utf16le(const unsigned char *bytes, size_t size);

/* Detect the plugin's real precedence: ux0 preset directory, ur0 preset directory,
   ux0 fixed file, ur0 fixed file, then default warning. */
rtcw_result rtcw_read_state(const rtcw_backend *backend, rtcw_state *out);
rtcw_result rtcw_read_preset(const rtcw_backend *backend, const char *mount,
                             const char *name, char *utf8, size_t capacity,
                             size_t *out_size);
/* add_if_missing permits creation of a new random-rotation preset. */
rtcw_result rtcw_write_preset(const rtcw_backend *backend, const char *mount,
                              const char *name, const char *utf8, int add_if_missing);
/* Fixed-file mode only; creates ux0:tai/custom_warning.txt if no override exists. */
rtcw_result rtcw_write_message(const rtcw_backend *backend, const char *utf8);
rtcw_result rtcw_restore_original(const rtcw_backend *backend, const char *path);

const char *rtcw_mode_name(rtcw_mode mode);
const char *rtcw_result_name(rtcw_result result);

#ifdef __cplusplus
}
#endif
#endif
