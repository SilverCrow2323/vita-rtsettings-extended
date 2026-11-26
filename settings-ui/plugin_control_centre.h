#ifndef RTSETTINGS_PLUGIN_CONTROL_CENTRE_H
#define RTSETTINGS_PLUGIN_CONTROL_CENTRE_H
#include <stddef.h>
#include "plugin_inventory.h"
#include "plugin_compatibility.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_PLUGIN_CENTRE_MAX RT_PI_MAX_ENTRIES
typedef enum { RTP_PC_ROW_UNKNOWN=0, RTP_PC_ROW_DISABLED=1, RTP_PC_ROW_DETECTED=2, RTP_PC_ROW_CONFIGURABLE=3, RTP_PC_ROW_BLOCKED=4, RTP_PC_ROW_REVIEW=5 } rtp_plugin_row_state;
typedef struct {
 char path[RT_PI_PATH_MAX+1], section[RT_PI_SECTION_MAX+1], plugin_id[RT_PI_PLUGIN_ID_MAX+1], adapter_id[RT_PI_ADAPTER_ID_MAX+1];
 unsigned int line_number, occurrence, duplicate_count;
 int enabled, known;
 rtp_plugin_row_state state;
} rtp_plugin_centre_row;
typedef struct {
 size_t count, unknown_count, disabled_count, detected_count, configurable_count, blocked_count, review_count;
 size_t blocking_findings, review_findings;
 rtp_plugin_centre_row rows[RTP_PLUGIN_CENTRE_MAX];
 rt_pc_report compatibility;
} rtp_plugin_centre_snapshot;
typedef enum { RTP_PC_OK=0, RTP_PC_INVALID_ARGUMENT=-1, RTP_PC_CAPACITY=-2, RTP_PC_INVENTORY_INVALID=-3, RTP_PC_ADAPTER_INVALID=-4, RTP_PC_COMPATIBILITY_ERROR=-5 } rtp_plugin_centre_result;
/* Builds a read-only view. Compatibility is evaluated only from known enabled entries. */
int rtp_plugin_centre_build(const rt_pi_inventory *inventory, const rt_pi_adapter_registry *adapters, rtp_plugin_centre_snapshot *snapshot);
const char *rtp_plugin_centre_state_name(rtp_plugin_row_state state);
#ifdef __cplusplus
}
#endif
#endif
