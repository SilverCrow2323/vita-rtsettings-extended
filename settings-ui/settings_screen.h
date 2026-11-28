#ifndef RTSETTINGS_SCREEN_H
#define RTSETTINGS_SCREEN_H
#include "preferences_ui.h"
#include "settings_metadata.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_SCREEN_TITLE_MAX RTP_META_TITLE_MAX
#define RTP_SCREEN_DESCRIPTION_MAX RTP_META_DESCRIPTION_MAX
typedef struct { char id[RTP_ID_MAX+1]; char title[RTP_SCREEN_TITLE_MAX+1]; char description[RTP_SCREEN_DESCRIPTION_MAX+1]; char icon_key[RTP_META_ICON_MAX+1]; rtp_entry_kind kind; unsigned int flags; int selected; } rtp_screen_row;
typedef struct { char title[RTP_SCREEN_TITLE_MAX+1]; char description[RTP_SCREEN_DESCRIPTION_MAX+1]; char category_id[RTP_ID_MAX+1]; size_t row_count; size_t selected_index; rtp_screen_row rows[RTP_UI_MAX_ROWS]; } rtp_screen_page;
typedef struct {
 int (*begin_page)(void *context,const rtp_screen_page *page);
 int (*draw_row)(void *context,const rtp_screen_row *row,size_t index,size_t total);
 int (*draw_footer)(void *context,const char *left_hint,const char *right_hint);
 int (*end_page)(void *context);
} rtp_screen_renderer;
typedef enum { RTP_SCREEN_OK=0,RTP_SCREEN_INVALID_ARGUMENT=-1,RTP_SCREEN_MODEL_ERROR=-2,RTP_SCREEN_RENDER_ERROR=-3 } rtp_screen_result;
int rtp_settings_screen_build(const rtp_preferences_ui *ui,const rtp_preferences *prefs,const rtp_metadata_catalogue *metadata,rtp_screen_page *page);
int rtp_settings_screen_render(const rtp_screen_page *page,const rtp_screen_renderer *renderer,void *context);
#ifdef __cplusplus
}
#endif
#endif
