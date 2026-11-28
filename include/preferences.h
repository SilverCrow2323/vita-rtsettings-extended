#ifndef RTSETTINGS_PREFERENCES_H
#define RTSETTINGS_PREFERENCES_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_MAX_ENTRIES 128
#define RTP_ID_MAX 63
#define RTP_PATH_MAX 511
typedef enum { RTP_ENTRY_CATEGORY=1, RTP_ENTRY_SETTING=2, RTP_ENTRY_TOOL=3, RTP_ENTRY_ACTION=4 } rtp_entry_kind;
typedef struct {
 char id[RTP_ID_MAX+1], parent_id[RTP_ID_MAX+1];
 rtp_entry_kind kind;
 unsigned int default_order, order;
 int default_visible, visible;
} rtp_menu_entry;
typedef struct { rtp_menu_entry entries[RTP_MAX_ENTRIES]; size_t count; } rtp_preferences;
typedef enum {
 RTP_PREFS_OK=0, RTP_PREFS_INVALID_ARGUMENT=-1, RTP_PREFS_NOT_FOUND=-2,
 RTP_PREFS_DUPLICATE_ID=-3, RTP_PREFS_CAPACITY=-4, RTP_PREFS_INVALID_PARENT=-5,
 RTP_PREFS_CYCLE=-6, RTP_PREFS_IO_ERROR=-7, RTP_PREFS_INVALID_DATA=-8
} rtp_preferences_result;
void rtp_preferences_init(rtp_preferences *prefs);
int rtp_preferences_register(rtp_preferences *prefs,const char *id,const char *parent_id,rtp_entry_kind kind,int default_visible,unsigned int default_order);
int rtp_preferences_set_visible(rtp_preferences *prefs,const char *id,int visible);
int rtp_preferences_set_order(rtp_preferences *prefs,const char *id,unsigned int order);
int rtp_preferences_set_parent(rtp_preferences *prefs,const char *id,const char *new_parent_id);
int rtp_preferences_reset_entry(rtp_preferences *prefs,const char *id);
void rtp_preferences_reset_all(rtp_preferences *prefs);
const rtp_menu_entry *rtp_preferences_find(const rtp_preferences *prefs,const char *id);
int rtp_preferences_save(const rtp_preferences *prefs,const char *path);
int rtp_preferences_load(rtp_preferences *prefs,const char *path);
#ifdef __cplusplus
}
#endif
#endif
