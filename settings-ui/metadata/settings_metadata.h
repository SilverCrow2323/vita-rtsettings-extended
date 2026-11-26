#ifndef RTSETTINGS_METADATA_H
#define RTSETTINGS_METADATA_H
#include <stddef.h>
#include "preferences.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_META_MAX_ENTRIES RTP_MAX_ENTRIES
#define RTP_META_TITLE_MAX 63
#define RTP_META_DESCRIPTION_MAX 191
#define RTP_META_CATEGORY_MAX RTP_ID_MAX
#define RTP_META_ICON_MAX 31
#define RTP_META_MODULE_MAX RTP_ID_MAX
typedef enum { RTP_META_OK=0, RTP_META_INVALID_ARGUMENT=-1, RTP_META_NOT_FOUND=-2, RTP_META_DUPLICATE_ID=-3, RTP_META_CAPACITY=-4, RTP_META_INVALID_CATEGORY=-5, RTP_META_INVALID_TEXT=-6 } rtp_metadata_result;
typedef enum { RTP_META_FLAG_NONE=0, RTP_META_FLAG_REQUIRES_REBOOT=1u<<0, RTP_META_FLAG_REQUIRES_BACKEND=1u<<1, RTP_META_FLAG_READ_ONLY=1u<<2, RTP_META_FLAG_EXPERIMENTAL=1u<<3 } rtp_metadata_flags;
typedef struct { const char *entry_id; const char *title; const char *description; const char *category_id; const char *icon_key; const char *module_id; unsigned int flags; } rtp_metadata_descriptor;
typedef struct { char entry_id[RTP_ID_MAX+1]; char title[RTP_META_TITLE_MAX+1]; char description[RTP_META_DESCRIPTION_MAX+1]; char category_id[RTP_META_CATEGORY_MAX+1]; char icon_key[RTP_META_ICON_MAX+1]; char module_id[RTP_META_MODULE_MAX+1]; unsigned int flags; } rtp_metadata_entry;
typedef struct { rtp_metadata_entry entries[RTP_META_MAX_ENTRIES]; size_t count; } rtp_metadata_catalogue;
void rtp_metadata_init(rtp_metadata_catalogue *catalogue);
int rtp_metadata_register(rtp_metadata_catalogue *catalogue,const rtp_preferences *preferences,const rtp_metadata_descriptor *descriptor);
const rtp_metadata_entry *rtp_metadata_find(const rtp_metadata_catalogue *catalogue,const char *entry_id);
int rtp_metadata_validate(const rtp_metadata_catalogue *catalogue,const rtp_preferences *preferences);
/* Registers the Appearance & Customization and Quick Menu Settings entries. */
int rtp_metadata_register_appearance(rtp_metadata_catalogue *catalogue,rtp_preferences *preferences);
/* Registers the first complete Settings Hub feature tree for Tai Saver. */
int rtp_metadata_register_tai_saver(rtp_metadata_catalogue *catalogue,rtp_preferences *preferences);
#ifdef __cplusplus
}
#endif
#endif
