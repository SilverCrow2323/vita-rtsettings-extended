#ifndef RTSETTINGS_STORAGE_DEVICES_H
#define RTSETTINGS_STORAGE_DEVICES_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_STORAGE_MAX_DEVICES 8
#define RTP_STORAGE_ID_MAX 31
#define RTP_STORAGE_LABEL_MAX 63
#define RTP_STORAGE_MOUNT_MAX 15
typedef enum { RTP_STORAGE_UNKNOWN=0,RTP_STORAGE_SD2VITA=1,RTP_STORAGE_SONY_MEMORY_CARD=2,RTP_STORAGE_INTERNAL=3,RTP_STORAGE_USB=4 } rtp_storage_kind;
typedef enum { RTP_STORAGE_MOUNTED=0,RTP_STORAGE_UNMOUNTED=1,RTP_STORAGE_UNAVAILABLE=2,RTP_STORAGE_ERROR=3 } rtp_storage_state;
typedef struct {
 const char *id,*label,*mount_point;
 rtp_storage_kind kind;
 rtp_storage_state state;
 unsigned long long total_bytes,free_bytes;
 int read_only;
} rtp_storage_observation;
typedef struct {
 char id[RTP_STORAGE_ID_MAX+1],label[RTP_STORAGE_LABEL_MAX+1],mount_point[RTP_STORAGE_MOUNT_MAX+1];
 rtp_storage_kind kind;
 rtp_storage_state state;
 unsigned long long total_bytes,free_bytes;
 unsigned int free_percent;
 int read_only,duplicate_mount_point;
} rtp_storage_device;
typedef struct {
 size_t count, mounted_count, unavailable_count, error_count, duplicate_mount_count;
 rtp_storage_device devices[RTP_STORAGE_MAX_DEVICES];
} rtp_storage_snapshot;
typedef enum { RTP_STORAGE_OK=0,RTP_STORAGE_INVALID_ARGUMENT=-1,RTP_STORAGE_CAPACITY=-2,RTP_STORAGE_INVALID_DEVICE=-3,RTP_STORAGE_DUPLICATE_ID=-4 } rtp_storage_result;
/* Builds a read-only snapshot from platform-reported observations; it performs no mount operations. */
int rtp_storage_snapshot_build(const rtp_storage_observation *observations,size_t count,rtp_storage_snapshot *snapshot);
const char *rtp_storage_kind_name(rtp_storage_kind kind);
const char *rtp_storage_state_name(rtp_storage_state state);
#ifdef __cplusplus
}
#endif
#endif
