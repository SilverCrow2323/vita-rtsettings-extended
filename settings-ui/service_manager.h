#ifndef RTSETTINGS_SERVICE_MANAGER_H
#define RTSETTINGS_SERVICE_MANAGER_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_SERVICE_MAX 16
#define RTP_SERVICE_TITLE_MAX 63
#define RTP_SERVICE_GROUP_MAX 31
typedef enum { RTP_SERVICE_STOPPED=0,RTP_SERVICE_RUNNING=1,RTP_SERVICE_UNAVAILABLE=2,RTP_SERVICE_ERROR=3 } rtp_service_state;
typedef enum { RTP_SERVICE_CAN_START=1u<<0,RTP_SERVICE_CAN_STOP=1u<<1,RTP_SERVICE_CAN_AUTOSTART=1u<<2,RTP_SERVICE_READ_ONLY=1u<<3 } rtp_service_capability;
typedef struct { const char *id; const char *title; const char *description; const char *conflict_group; unsigned int capabilities; } rtp_service_descriptor;
typedef struct { char id[32]; char title[RTP_SERVICE_TITLE_MAX+1]; char description[191]; char conflict_group[RTP_SERVICE_GROUP_MAX+1]; unsigned int capabilities; rtp_service_state state; int auto_start; } rtp_service_entry;
typedef struct { size_t count; rtp_service_entry entries[RTP_SERVICE_MAX]; } rtp_service_catalogue;
typedef struct { int (*query)(void *context,const char *id,rtp_service_state *state,int *auto_start); int (*start)(void *context,const char *id); int (*stop)(void *context,const char *id); int (*set_auto_start)(void *context,const char *id,int enabled); } rtp_service_backend;
typedef enum { RTP_SERVICE_OK=0,RTP_SERVICE_INVALID_ARGUMENT=-1,RTP_SERVICE_NOT_FOUND=-2,RTP_SERVICE_DUPLICATE=-3,RTP_SERVICE_CAPACITY=-4,RTP_SERVICE_INVALID_DESCRIPTOR=-5,RTP_SERVICE_BACKEND_ERROR=-6,RTP_SERVICE_UNSUPPORTED=-7,RTP_SERVICE_CONFLICT=-8 } rtp_service_result;
void rtp_service_catalogue_init(rtp_service_catalogue *catalogue);
int rtp_service_register(rtp_service_catalogue *catalogue,const rtp_service_descriptor *descriptor);
const rtp_service_entry *rtp_service_find(const rtp_service_catalogue *catalogue,const char *id);
int rtp_service_refresh(rtp_service_catalogue *catalogue,const rtp_service_backend *backend,void *context);
int rtp_service_start(rtp_service_catalogue *catalogue,const char *id,const rtp_service_backend *backend,void *context);
int rtp_service_stop(rtp_service_catalogue *catalogue,const char *id,const rtp_service_backend *backend,void *context);
int rtp_service_set_auto_start(rtp_service_catalogue *catalogue,const char *id,int enabled,const rtp_service_backend *backend,void *context);
#ifdef __cplusplus
}
#endif
#endif
