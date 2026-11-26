#ifndef RTSE_PLUGIN_HELP_H
#define RTSE_PLUGIN_HELP_H
#include <stddef.h>
#include "preferences.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RTP_HELP_MAX_ENTRIES RTP_MAX_ENTRIES
#define RTP_HELP_TITLE_MAX 95
#define RTP_HELP_AUTHOR_MAX 95
#define RTP_HELP_VERSION_MAX 31
#define RTP_HELP_TEXT_MAX 1023
#define RTP_HELP_URL_MAX 255
#define RTP_HELP_LICENSE_MAX 63
#define RTP_HELP_FIRMWARE_MAX 127

typedef enum {
    RTP_HELP_OK=0, RTP_HELP_INVALID_ARGUMENT=-1, RTP_HELP_DUPLICATE=-2,
    RTP_HELP_CAPACITY=-3, RTP_HELP_NOT_FOUND=-4, RTP_HELP_INVALID_TEXT=-5
} rtp_plugin_help_result;

typedef struct {
    const char *plugin_id;
    const char *display_name;
    const char *author;
    const char *version;
    const char *summary;
    const char *technical_details;
    const char *configuration_details;
    const char *dependencies;
    const char *compatibility;
    const char *known_conflicts;
    const char *credits;
    const char *license;
    const char *source_url;
    const char *release_url;
    const char *verification_note;
} rtp_plugin_help_descriptor;

typedef struct {
    char plugin_id[RTP_ID_MAX+1];
    char display_name[RTP_HELP_TITLE_MAX+1];
    char author[RTP_HELP_AUTHOR_MAX+1];
    char version[RTP_HELP_VERSION_MAX+1];
    char summary[RTP_HELP_TEXT_MAX+1];
    char technical_details[RTP_HELP_TEXT_MAX+1];
    char configuration_details[RTP_HELP_TEXT_MAX+1];
    char dependencies[RTP_HELP_TEXT_MAX+1];
    char compatibility[RTP_HELP_TEXT_MAX+1];
    char known_conflicts[RTP_HELP_TEXT_MAX+1];
    char credits[RTP_HELP_TEXT_MAX+1];
    char license[RTP_HELP_LICENSE_MAX+1];
    char source_url[RTP_HELP_URL_MAX+1];
    char release_url[RTP_HELP_URL_MAX+1];
    char verification_note[RTP_HELP_TEXT_MAX+1];
} rtp_plugin_help_entry;

typedef struct rtp_plugin_help_catalogue {
    size_t count;
    rtp_plugin_help_entry entries[RTP_HELP_MAX_ENTRIES];
} rtp_plugin_help_catalogue;

typedef struct {
    const rtp_plugin_help_entry *entry;
    int show_help_button;
    int has_source_link;
    int has_release_link;
} rtp_plugin_help_panel;

void rtp_plugin_help_init(rtp_plugin_help_catalogue *catalogue);
rtp_plugin_help_result rtp_plugin_help_register(
    rtp_plugin_help_catalogue *catalogue,
    const rtp_plugin_help_descriptor *descriptor);
const rtp_plugin_help_entry *rtp_plugin_help_find(
    const rtp_plugin_help_catalogue *catalogue, const char *plugin_id);
/* Builds the ? affordance and its read-only detail-panel data. */
rtp_plugin_help_result rtp_plugin_help_open(
    const rtp_plugin_help_catalogue *catalogue, const char *plugin_id,
    rtp_plugin_help_panel *panel);
/* Registers source-attributed entries for adapters shipped by this project. */
rtp_plugin_help_result rtp_plugin_help_register_builtin(
    rtp_plugin_help_catalogue *catalogue);
#ifdef __cplusplus
}
#endif
#endif
