#ifndef RTSE_TAI_CONFIG_H
#define RTSE_TAI_CONFIG_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    RTTC_ERROR = -1,
    RTTC_CAPACITY = -2,
    RTTC_AMBIGUOUS = -3,
    RTTC_NOT_FOUND = -4,
    RTTC_ADDED = 0,
    RTTC_ALREADY_ACTIVE = 1,
    RTTC_DISABLED = 2,
    RTTC_ENABLED = 3,
    RTTC_REMOVED = 4
} rttc_result;
/* Operates only on the exact plugin line inside *NPXS10015. All unrelated
 * bytes, comments, sections and line endings are preserved. out_length excludes NUL. */
rttc_result rttc_add_settings_plugin(const char *source, size_t source_length,
                                     const char *plugin_line,
                                     char *destination, size_t destination_capacity,
                                     size_t *out_length);
rttc_result rttc_enable_settings_plugin(const char *source, size_t source_length,
                                        const char *plugin_line,
                                        char *destination, size_t destination_capacity,
                                        size_t *out_length);
rttc_result rttc_remove_settings_plugin(const char *source, size_t source_length,
                                        const char *plugin_line,
                                        char *destination, size_t destination_capacity,
                                        size_t *out_length);
#ifdef __cplusplus
}
#endif
#endif
