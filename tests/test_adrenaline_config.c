#include "adrenaline_config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {unsigned char data[60],backup[60];size_t length;int fail_backup,fail_write,corrupt;} fake;
static int rd(void *ctx,const char *path,void *out,size_t cap,size_t *actual){fake *f=ctx;(void)path;if(f->length>cap)return -1;memcpy(out,f->data,f->length);*actual=f->length;return 0;}
static int bk(void *ctx,const char *path,const void *p,size_t n){fake *f=ctx;(void)path;if(f->fail_backup)return -1;memcpy(f->backup,p,n);return 0;}
static int wr(void *ctx,const char *path,const void *p,size_t n){fake *f=ctx;(void)path;if(f->fail_write)return -1;memcpy(f->data,p,n);f->length=n;if(f->corrupt)f->data[8]^=1;return 0;}
int main(void){
 rtp_adrenaline_config c={0},out;unsigned char bytes[60],original[60];fake f={0};
 rtp_adrenaline_config_io io={&f,rd,bk,wr};
 c.graphics_filtering=5;c.no_smooth_graphics=1;c.flux_mode=2;c.screen_size=3;
 c.ms_location=4;c.use_ds3_ds4=1;c.screen_mode=2;c.skip_logo=1;
 c.psp_screen_scale_x=2.0f;c.psp_screen_scale_y=2.0f;
 c.ps1_screen_scale_x=1.5f;c.ps1_screen_scale_y=1.5f;c.usbdevice=2;
 assert(rtp_adrenaline_config_encode(&c,bytes,sizeof(bytes))==RTP_ADRENALINE_OK);
 assert(rtp_adrenaline_config_decode(bytes,sizeof(bytes),&out)==RTP_ADRENALINE_OK);
 assert(out.graphics_filtering==5&&out.ms_location==4&&out.usbdevice==2);
 assert(rtp_adrenaline_config_decode(bytes,59,&out)==RTP_ADRENALINE_INVALID_SIZE);
 bytes[0]=0;assert(rtp_adrenaline_config_decode(bytes,sizeof(bytes),&out)==RTP_ADRENALINE_INVALID_CONFIG);
 c.usbdevice=4;assert(rtp_adrenaline_config_encode(&c,bytes,sizeof(bytes))==RTP_ADRENALINE_INVALID_CONFIG);c.usbdevice=2;
 assert(rtp_adrenaline_config_encode(&c,bytes,sizeof(bytes))==RTP_ADRENALINE_OK);
 memcpy(f.data,bytes,sizeof(bytes));memcpy(original,bytes,sizeof(bytes));f.length=sizeof(bytes);
 assert(rtp_adrenaline_config_load(&io,&out)==RTP_ADRENALINE_OK);
 out.graphics_filtering=3;
 assert(rtp_adrenaline_config_save(&io,&out)==RTP_ADRENALINE_OK);
 assert(memcmp(f.backup,original,60)==0);
 assert(rtp_adrenaline_config_load(&io,&c)==RTP_ADRENALINE_OK&&c.graphics_filtering==3);
 f.fail_backup=1;assert(rtp_adrenaline_config_save(&io,&out)==RTP_ADRENALINE_BACKEND_ERROR);f.fail_backup=0;
 f.corrupt=1;out.graphics_filtering=4;
 assert(rtp_adrenaline_config_save(&io,&out)==RTP_ADRENALINE_INTEGRITY_ERROR);
 assert(strcmp(rtp_adrenaline_usb_device_name(2),"SD2Vita")==0);
 puts("Adrenaline configuration adapter tests passed.");return 0;
}
