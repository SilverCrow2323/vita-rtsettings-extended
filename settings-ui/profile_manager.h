#ifndef RTSETTINGS_PROFILE_MANAGER_H
#define RTSETTINGS_PROFILE_MANAGER_H
#include "preferences_ui.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_PM_MAX_ROWS (RTP_PROFILE_MAX+1)
#define RTP_PM_LABEL_MAX RTP_PROFILE_NAME_MAX
typedef enum { RTP_PM_PAGE_SETTINGS=0,RTP_PM_PAGE_PROFILES=1,RTP_PM_PAGE_PROFILE_ACTIONS=2,RTP_PM_PAGE_CONFIRM_DELETE=3 } rtp_pm_page_kind;
typedef enum { RTP_PM_INPUT_UP=0,RTP_PM_INPUT_DOWN=1,RTP_PM_INPUT_CONFIRM=2,RTP_PM_INPUT_BACK=3 } rtp_pm_input;\ntypedef enum { RTP_PM_ROW_CREATE=0,RTP_PM_ROW_PROFILE=1,RTP_PM_ROW_APPLY=2,RTP_PM_ROW_UPDATE=3,RTP_PM_ROW_RENAME=4,RTP_PM_ROW_DELETE=5,RTP_PM_ROW_CONFIRM=6,RTP_PM_ROW_CANCEL=7 } rtp_pm_row_kind;
typedef struct { char id[RTP_ID_MAX+1]; char title[RTP_PM_LABEL_MAX+1]; rtp_pm_row_kind kind; int selected; } rtp_pm_row;
typedef struct { rtp_pm_page_kind page; char title[64]; char selected_profile_id[RTP_ID_MAX+1]; size_t count; size_t selected_index; rtp_pm_row rows[RTP_PM_MAX_ROWS]; } rtp_pm_view;
typedef enum { RTP_PM_OK=0,RTP_PM_INVALID_ARGUMENT=-1,RTP_PM_NOT_FOUND=-2,RTP_PM_CAPACITY=-3,RTP_PM_CONFIRMATION_REQUIRED=-4,RTP_PM_PROFILE_ERROR=-5 } rtp_pm_result;
typedef struct { rtp_preferences_ui *navigation; rtp_preferences *preferences; rtp_profile_store *profiles; rtp_pm_page_kind page; size_t selected_index; char selected_profile_id[RTP_ID_MAX+1]; char pending_delete_id[RTP_ID_MAX+1]; } rtp_profile_manager;
void rtp_profile_manager_init(rtp_profile_manager *manager,rtp_preferences_ui *navigation,rtp_preferences *preferences,rtp_profile_store *profiles);
int rtp_profile_manager_open(rtp_profile_manager *manager);
int rtp_profile_manager_back(rtp_profile_manager *manager);\nint rtp_profile_manager_handle_input(rtp_profile_manager *manager,rtp_pm_input input,const char *create_id,const char *text_value);
int rtp_profile_manager_build_view(const rtp_profile_manager *manager,rtp_pm_view *view);
int rtp_profile_manager_select(rtp_profile_manager *manager,const char *profile_id);
int rtp_profile_manager_create(rtp_profile_manager *manager,const char *id,const char *name);
int rtp_profile_manager_apply(rtp_profile_manager *manager);
int rtp_profile_manager_update(rtp_profile_manager *manager);
int rtp_profile_manager_rename(rtp_profile_manager *manager,const char *name);
int rtp_profile_manager_request_delete(rtp_profile_manager *manager);
int rtp_profile_manager_confirm_delete(rtp_profile_manager *manager,int confirmed);
#ifdef __cplusplus
}
#endif
#endif
