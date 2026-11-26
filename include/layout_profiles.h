#ifndef RTSETTINGS_LAYOUT_PROFILES_H
#define RTSETTINGS_LAYOUT_PROFILES_H
#include "preferences.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_PROFILE_MAX 8
#define RTP_PROFILE_NAME_MAX 31
typedef struct {
 char id[RTP_ID_MAX+1], name[RTP_PROFILE_NAME_MAX+1];
 size_t count;
 rtp_menu_entry entries[RTP_MAX_ENTRIES];
} rtp_layout_profile;
typedef struct {
 size_t count;
 char active_id[RTP_ID_MAX+1];
 rtp_layout_profile profiles[RTP_PROFILE_MAX];
} rtp_profile_store;
typedef enum {
 RTP_PROFILE_OK=0, RTP_PROFILE_INVALID_ARGUMENT=-1, RTP_PROFILE_NOT_FOUND=-2,
 RTP_PROFILE_DUPLICATE=-3, RTP_PROFILE_CAPACITY=-4, RTP_PROFILE_PROTECTED=-5,
 RTP_PROFILE_INVALID_DATA=-6, RTP_PROFILE_IO_ERROR=-7, RTP_PROFILE_LAYOUT_ERROR=-8
} rtp_profile_result;
void rtp_profiles_init(rtp_profile_store *store);
int rtp_profiles_create(rtp_profile_store *store,const char *id,const char *name,const rtp_preferences *prefs);
int rtp_profiles_update(rtp_profile_store *store,const char *id,const rtp_preferences *prefs);
int rtp_profiles_rename(rtp_profile_store *store,const char *id,const char *name);
int rtp_profiles_delete(rtp_profile_store *store,const char *id);
int rtp_profiles_apply(rtp_profile_store *store,const char *id,rtp_preferences *prefs);
const rtp_layout_profile *rtp_profiles_find(const rtp_profile_store *store,const char *id);
int rtp_profiles_save(const rtp_profile_store *store,const char *path);
int rtp_profiles_load(rtp_profile_store *store,const char *path);
#ifdef __cplusplus
}
#endif
#endif
