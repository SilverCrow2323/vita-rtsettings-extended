#ifndef RTSETTINGS_ADRENALINE_CONFIG_H
#define RTSETTINGS_ADRENALINE_CONFIG_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_ADRENALINE_CONFIG_SIZE 60u
#define RTP_ADRENALINE_MAGIC_1 0x31483943u
#define RTP_ADRENALINE_MAGIC_2 0x334F4E33u
#define RTP_ADRENALINE_PATH "ux0:app/PSPEMUCFW/adrenaline.bin"
typedef struct {
 uint32_t graphics_filtering,no_smooth_graphics,flux_mode,screen_size;
 uint32_t ms_location,use_ds3_ds4,screen_mode,skip_logo;
 float psp_screen_scale_x,psp_screen_scale_y,ps1_screen_scale_x,ps1_screen_scale_y;
 uint32_t usbdevice;
} rtp_adrenaline_config;
typedef struct {
 void *context;
 int (*read)(void *context,const char *path,void *buffer,size_t capacity,size_t *actual);
 int (*backup_atomic)(void *context,const char *path,const void *bytes,size_t length);
 int (*replace_atomic)(void *context,const char *path,const void *bytes,size_t length);
} rtp_adrenaline_config_io;
typedef enum {
 RTP_ADRENALINE_OK=0,RTP_ADRENALINE_INVALID_ARGUMENT=-1,
 RTP_ADRENALINE_INVALID_SIZE=-2,RTP_ADRENALINE_INVALID_CONFIG=-3,
 RTP_ADRENALINE_BACKEND_ERROR=-4,RTP_ADRENALINE_INTEGRITY_ERROR=-5
} rtp_adrenaline_result;
int rtp_adrenaline_config_decode(const void *bytes,size_t length,rtp_adrenaline_config *config);
int rtp_adrenaline_config_encode(const rtp_adrenaline_config *config,void *bytes,size_t capacity);
int rtp_adrenaline_config_validate(const rtp_adrenaline_config *config);
int rtp_adrenaline_config_load(const rtp_adrenaline_config_io *io,rtp_adrenaline_config *config);
int rtp_adrenaline_config_save(const rtp_adrenaline_config_io *io,const rtp_adrenaline_config *config);
const char *rtp_adrenaline_graphics_filter_name(unsigned int value);
const char *rtp_adrenaline_memory_location_name(unsigned int value);
const char *rtp_adrenaline_usb_device_name(unsigned int value);
#ifdef __cplusplus
}
#endif
#endif
