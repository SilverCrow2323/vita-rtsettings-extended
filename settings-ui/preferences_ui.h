#ifndef RTSETTINGS_PREFERENCES_UI_H
#define RTSETTINGS_PREFERENCES_UI_H
#include "preferences.h"
#include "layout_profiles.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_UI_MAX_DEPTH 16
#define RTP_UI_MAX_ROWS RTP_MAX_ENTRIES
typedef struct { char id[RTP_ID_MAX+1]; rtp_entry_kind kind; unsigned int order; size_t registry_index; } rtp_ui_row;
typedef struct { char category_id[RTP_ID_MAX+1]; char selected_id[RTP_ID_MAX+1]; size_t selected_index; } rtp_ui_frame;
typedef struct { rtp_ui_frame frames[RTP_UI_MAX_DEPTH]; size_t depth; } rtp_preferences_ui;
typedef enum { RTP_UI_OK=0, RTP_UI_INVALID_ARGUMENT=-1, RTP_UI_NOT_FOUND=-2, RTP_UI_CAPACITY=-3, RTP_UI_ROOT=-4, RTP_UI_EMPTY=-5 } rtp_ui_result;
void rtp_preferences_ui_init(rtp_preferences_ui *ui);
int rtp_preferences_ui_enter(rtp_preferences_ui *ui,const rtp_preferences *prefs,const char *category_id);
int rtp_preferences_ui_back(rtp_preferences_ui *ui);
const char *rtp_preferences_ui_current_category(const rtp_preferences_ui *ui);
int rtp_preferences_ui_rows(const rtp_preferences_ui *ui,const rtp_preferences *prefs,rtp_ui_row *rows,size_t capacity,size_t *row_count);
int rtp_preferences_ui_select(rtp_preferences_ui *ui,const rtp_preferences *prefs,const char *entry_id);
int rtp_preferences_ui_move(rtp_preferences_ui *ui,rtp_preferences *prefs,const char *entry_id,int direction);
int rtp_preferences_ui_set_visible(rtp_preferences_ui *ui,rtp_preferences *prefs,const char *entry_id,int visible);
int rtp_preferences_ui_reset(rtp_preferences_ui *ui,rtp_preferences *prefs,const char *entry_id);
int rtp_preferences_ui_apply_profile(rtp_preferences_ui *ui,rtp_preferences *prefs,rtp_profile_store *profiles,const char *profile_id);
#ifdef __cplusplus
}
#endif
#endif
