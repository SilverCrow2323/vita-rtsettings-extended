#ifndef RTSETTINGS_MODULE_CONTROLS_H
#define RTSETTINGS_MODULE_CONTROLS_H
#include <stddef.h>
#include "preferences.h"
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_CONTROL_MAX RTP_MAX_ENTRIES
#define RTP_CONTROL_OPTIONS_MAX 12
#define RTP_CONTROL_LABEL_MAX 63
#define RTP_CONTROL_VALUE_MAX 63
typedef enum { RTP_CONTROL_TOGGLE=0,RTP_CONTROL_SELECT=1,RTP_CONTROL_SLIDER=2,RTP_CONTROL_NUMBER=3 } rtp_control_kind;
typedef struct { char value[RTP_CONTROL_VALUE_MAX+1]; char label[RTP_CONTROL_LABEL_MAX+1]; } rtp_control_option;
typedef struct { const char *id; const char *title; const char *description; const char *module_id; rtp_control_kind kind; long minimum; long maximum; long step; const rtp_control_option *options; size_t option_count; int read_only; } rtp_control_descriptor;
typedef struct { char id[RTP_ID_MAX+1]; char title[RTP_CONTROL_LABEL_MAX+1]; char description[192]; char module_id[RTP_ID_MAX+1]; rtp_control_kind kind; long minimum,maximum,step,current; size_t option_count; rtp_control_option options[RTP_CONTROL_OPTIONS_MAX]; int read_only; } rtp_module_control;
typedef struct { size_t count; rtp_module_control controls[RTP_CONTROL_MAX]; } rtp_module_control_catalogue;
typedef struct { int (*read)(void *context,const char *id,long *numeric_value,char *text_value,size_t text_capacity); int (*write)(void *context,const char *id,long numeric_value,const char *text_value); } rtp_control_backend;
typedef enum { RTP_CONTROL_OK=0,RTP_CONTROL_INVALID_ARGUMENT=-1,RTP_CONTROL_NOT_FOUND=-2,RTP_CONTROL_DUPLICATE=-3,RTP_CONTROL_CAPACITY=-4,RTP_CONTROL_INVALID_DESCRIPTOR=-5,RTP_CONTROL_BACKEND_ERROR=-6,RTP_CONTROL_READ_ONLY=-7,RTP_CONTROL_INVALID_VALUE=-8 } rtp_control_result;
void rtp_module_controls_init(rtp_module_control_catalogue *catalogue);
int rtp_module_control_register(rtp_module_control_catalogue *catalogue,const rtp_control_descriptor *descriptor);
const rtp_module_control *rtp_module_control_find(const rtp_module_control_catalogue *catalogue,const char *id);
int rtp_module_control_read(const rtp_module_control_catalogue *catalogue,const char *id,const rtp_control_backend *backend,void *context,long *numeric_value,char *text_value,size_t text_capacity);
int rtp_module_control_write(const rtp_module_control_catalogue *catalogue,const char *id,const rtp_control_backend *backend,void *context,long numeric_value,const char *text_value);
#ifdef __cplusplus
}
#endif
#endif
