#include "adrenaline_config.h"
#include <string.h>
static uint32_t get32(const unsigned char *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put32(unsigned char *p,uint32_t v){p[0]=(unsigned char)v;p[1]=(unsigned char)(v>>8);p[2]=(unsigned char)(v>>16);p[3]=(unsigned char)(v>>24);}
static int valid_scale(float f){return f>=0.005f&&f<=100.0f;}
int rtp_adrenaline_config_validate(const rtp_adrenaline_config *c){
 if(!c)return RTP_ADRENALINE_INVALID_ARGUMENT;
 if(c->graphics_filtering>5||c->no_smooth_graphics>1||c->flux_mode>3||
    c->screen_size>4||c->ms_location>4||c->use_ds3_ds4>1||c->screen_mode>3||
    c->skip_logo>1||c->usbdevice>3||
    !valid_scale(c->psp_screen_scale_x)||!valid_scale(c->psp_screen_scale_y)||
    !valid_scale(c->ps1_screen_scale_x)||!valid_scale(c->ps1_screen_scale_y))
  return RTP_ADRENALINE_INVALID_CONFIG;
 return RTP_ADRENALINE_OK;
}
int rtp_adrenaline_config_decode(const void *bytes,size_t length,rtp_adrenaline_config *c){
 const unsigned char *p=bytes;uint32_t v;unsigned int i;
 if(!bytes||!c)return RTP_ADRENALINE_INVALID_ARGUMENT;
 if(length!=RTP_ADRENALINE_CONFIG_SIZE)return RTP_ADRENALINE_INVALID_SIZE;
 if(get32(p)!=RTP_ADRENALINE_MAGIC_1||get32(p+4)!=RTP_ADRENALINE_MAGIC_2)return RTP_ADRENALINE_INVALID_CONFIG;
 c->graphics_filtering=get32(p+8);c->no_smooth_graphics=get32(p+12);
 c->flux_mode=get32(p+16);c->screen_size=get32(p+20);c->ms_location=get32(p+24);
 c->use_ds3_ds4=get32(p+28);c->screen_mode=get32(p+32);c->skip_logo=get32(p+36);
 {float *scales[4]={&c->psp_screen_scale_x,&c->psp_screen_scale_y,&c->ps1_screen_scale_x,&c->ps1_screen_scale_y};for(i=0;i<4;++i){v=get32(p+40+i*4);memcpy(scales[i],&v,sizeof(v));}}
 c->usbdevice=get32(p+56);return rtp_adrenaline_config_validate(c);
}
int rtp_adrenaline_config_encode(const rtp_adrenaline_config *c,void *bytes,size_t cap){
 unsigned char *p=bytes;uint32_t v;unsigned int i;
 if(!c||!bytes)return RTP_ADRENALINE_INVALID_ARGUMENT;
 if(cap<RTP_ADRENALINE_CONFIG_SIZE)return RTP_ADRENALINE_INVALID_SIZE;
 if(rtp_adrenaline_config_validate(c)!=RTP_ADRENALINE_OK)return RTP_ADRENALINE_INVALID_CONFIG;
 put32(p,RTP_ADRENALINE_MAGIC_1);put32(p+4,RTP_ADRENALINE_MAGIC_2);
 put32(p+8,c->graphics_filtering);put32(p+12,c->no_smooth_graphics);
 put32(p+16,c->flux_mode);put32(p+20,c->screen_size);put32(p+24,c->ms_location);
 put32(p+28,c->use_ds3_ds4);put32(p+32,c->screen_mode);put32(p+36,c->skip_logo);
 {const float *scales[4]={&c->psp_screen_scale_x,&c->psp_screen_scale_y,&c->ps1_screen_scale_x,&c->ps1_screen_scale_y};for(i=0;i<4;++i){memcpy(&v,scales[i],sizeof(v));put32(p+40+i*4,v);}}
 put32(p+56,c->usbdevice);return RTP_ADRENALINE_OK;
}
int rtp_adrenaline_config_load(const rtp_adrenaline_config_io *io,rtp_adrenaline_config *c){
 unsigned char bytes[RTP_ADRENALINE_CONFIG_SIZE];size_t actual=0;
 if(!io||!io->read||!c)return RTP_ADRENALINE_INVALID_ARGUMENT;
 if(io->read(io->context,RTP_ADRENALINE_PATH,bytes,sizeof(bytes),&actual)!=0)return RTP_ADRENALINE_BACKEND_ERROR;
 return rtp_adrenaline_config_decode(bytes,actual,c);
}
int rtp_adrenaline_config_save(const rtp_adrenaline_config_io *io,const rtp_adrenaline_config *c){
 unsigned char bytes[RTP_ADRENALINE_CONFIG_SIZE],check[RTP_ADRENALINE_CONFIG_SIZE],old[RTP_ADRENALINE_CONFIG_SIZE];
 size_t actual=0,old_length=0;
 if(!io||!io->read||!io->backup_atomic||!io->replace_atomic||!c)return RTP_ADRENALINE_INVALID_ARGUMENT;
 if(rtp_adrenaline_config_encode(c,bytes,sizeof(bytes))!=RTP_ADRENALINE_OK)return RTP_ADRENALINE_INVALID_CONFIG;
 if(io->read(io->context,RTP_ADRENALINE_PATH,old,sizeof(old),&old_length)!=0)return RTP_ADRENALINE_BACKEND_ERROR;
 {rtp_adrenaline_config previous;if(rtp_adrenaline_config_decode(old,old_length,&previous)!=RTP_ADRENALINE_OK)return RTP_ADRENALINE_INVALID_CONFIG;}
 if(io->backup_atomic(io->context,RTP_ADRENALINE_PATH,old,old_length)!=0)return RTP_ADRENALINE_BACKEND_ERROR;
 if(io->replace_atomic(io->context,RTP_ADRENALINE_PATH,bytes,sizeof(bytes))!=0)return RTP_ADRENALINE_BACKEND_ERROR;
 if(io->read(io->context,RTP_ADRENALINE_PATH,check,sizeof(check),&actual)!=0)return RTP_ADRENALINE_BACKEND_ERROR;
 if(actual!=sizeof(bytes)||memcmp(bytes,check,sizeof(bytes)))return RTP_ADRENALINE_INTEGRITY_ERROR;
 return RTP_ADRENALINE_OK;
}
const char *rtp_adrenaline_graphics_filter_name(unsigned int v){
 static const char *names[]={"Original","Bilinear","Sharp bilinear","Advanced AA","LCD3x","Sharp bilinear (no scanlines)"};
 return v<6?names[v]:"Unknown";
}
const char *rtp_adrenaline_memory_location_name(unsigned int v){
 static const char *names[]={"ux0:pspemu","ur0:pspemu","imc0:pspemu","xmc0:pspemu","uma0:pspemu"};
 return v<5?names[v]:"Unknown";
}
const char *rtp_adrenaline_usb_device_name(unsigned int v){
 static const char *names[]={"Memory Card","Internal Storage","SD2Vita","PSVSD"};
 return v<4?names[v]:"Unknown";
}
