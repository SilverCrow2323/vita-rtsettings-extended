#ifndef RT_SETTINGS_XML_H
#define RT_SETTINGS_XML_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RT_SETTINGS_XML_BUFFER_SIZE 16384
typedef enum {
    RT_SETTINGS_XML_ERROR = -1,
    RT_SETTINGS_XML_UNCHANGED = 0,
    RT_SETTINGS_XML_INSERTED = 1
} rt_settings_xml_result;
/* Bounded, idempotent insertion into Sony's existing system_settings_plugin root XML.
 * Returns ERROR for malformed/oversized input; callers must pass the original XML
 * through unchanged in that case. out_length excludes the terminating NUL. */
rt_settings_xml_result rt_settings_xml_splice_root(
    const char *source, size_t source_length,
    char *destination, size_t destination_capacity,
    size_t *out_length);
#ifdef __cplusplus
}
#endif
#endif
