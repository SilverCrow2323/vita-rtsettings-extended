#ifndef RTSETTINGS_REPATCH_MANAGER_H
#define RTSETTINGS_REPATCH_MANAGER_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_REPATCH_MAX_ENTRIES 128
#define RTP_REPATCH_ID_MAX 15
#define RTP_REPATCH_NAME_MAX 63
#define RTP_REPATCH_PATH_MAX 127
typedef enum {RTP_REPATCH_GAME=1,RTP_REPATCH_DLC=2} rtp_repatch_kind;
typedef enum {RTP_REPATCH_ACTIVE=1,RTP_REPATCH_DISABLED=2} rtp_repatch_state;
typedef struct {
 const char *folder_name,*title;
 rtp_repatch_kind kind;
} rtp_repatch_observation;
typedef struct {
 char id[RTP_REPATCH_ID_MAX+1],title[RTP_REPATCH_NAME_MAX+1];
 char folder_name[RTP_REPATCH_PATH_MAX+1];
 rtp_repatch_kind kind;
 rtp_repatch_state state;
} rtp_repatch_entry;
typedef struct {size_t count;rtp_repatch_entry entries[RTP_REPATCH_MAX_ENTRIES];} rtp_repatch_snapshot;
typedef struct {
 void *context;
 int (*exists)(void *context,const char *path);
 /* Must be an atomic, no-clobber rename on the target filesystem. */
 int (*rename_no_replace)(void *context,const char *source,const char *destination);
} rtp_repatch_fs;
typedef enum {
 RTP_REPATCH_OK=0,RTP_REPATCH_INVALID_ARGUMENT=-1,RTP_REPATCH_CAPACITY=-2,
 RTP_REPATCH_INVALID_ENTRY=-3,RTP_REPATCH_DUPLICATE_ID=-4,
 RTP_REPATCH_NOT_FOUND=-5,RTP_REPATCH_ALREADY_STATE=-6,
 RTP_REPATCH_DESTINATION_EXISTS=-7,RTP_REPATCH_BACKEND_ERROR=-8,
 RTP_REPATCH_VERIFY_FAILED=-9
} rtp_repatch_result;
int rtp_repatch_snapshot_build(const rtp_repatch_observation *items,size_t count,
                               rtp_repatch_snapshot *snapshot);
int rtp_repatch_set_state(rtp_repatch_snapshot *snapshot,const char *id,
                          rtp_repatch_state desired,rtp_repatch_kind kind,const char *mount,
                          const rtp_repatch_fs *fs);
#ifdef __cplusplus
}
#endif
#endif
