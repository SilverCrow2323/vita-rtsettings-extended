#include "storage_devices.h"
#include <string.h>
#include <ctype.h>
static int copy_text(char *dst,size_t cap,const char *src,int required){size_t n;if(!src)return !required&&cap?(dst[0]='\0',1):0;n=strlen(src);if((required&&!n)||n>=cap)return 0;memcpy(dst,src,n+1);return 1;}
static int valid_mount(const char *p){size_t n,i;if(!p||!p[0])return 1;n=strlen(p);if(n<3||n>RTP_STORAGE_MOUNT_MAX||p[n-1]!=':')return 0;for(i=0;i+1<n;++i)if(!isalnum((unsigned char)p[i]))return 0;return 1;}
int rtp_storage_snapshot_build(const rtp_storage_observation *obs,size_t count,rtp_storage_snapshot *out){
 size_t i,j;if(!out||(!obs&&count))return RTP_STORAGE_INVALID_ARGUMENT;if(count>RTP_STORAGE_MAX_DEVICES)return RTP_STORAGE_CAPACITY;memset(out,0,sizeof(*out));
 for(i=0;i<count;++i){const rtp_storage_observation *src=&obs[i];rtp_storage_device *dst=&out->devices[i];
  if(!copy_text(dst->id,sizeof(dst->id),src->id,1)||!copy_text(dst->label,sizeof(dst->label),src->label,1)||!copy_text(dst->mount_point,sizeof(dst->mount_point),src->mount_point,0)||!valid_mount(src->mount_point)||src->kind<RTP_STORAGE_UNKNOWN||src->kind>RTP_STORAGE_USB||src->state<RTP_STORAGE_MOUNTED||src->state>RTP_STORAGE_ERROR||src->free_bytes>src->total_bytes||(src->state==RTP_STORAGE_MOUNTED&&(!src->mount_point||!src->mount_point[0])))return RTP_STORAGE_INVALID_DEVICE;
  for(j=0;j<i;++j)if(strcmp(out->devices[j].id,dst->id)==0)return RTP_STORAGE_DUPLICATE_ID;
  dst->kind=src->kind;dst->state=src->state;dst->total_bytes=src->total_bytes;dst->free_bytes=src->free_bytes;dst->read_only=src->read_only?1:0;
  if(src->total_bytes)dst->free_percent=(unsigned int)(((long double)src->free_bytes*100.0L)/(long double)src->total_bytes);
  if(dst->state==RTP_STORAGE_MOUNTED)out->mounted_count++;else if(dst->state==RTP_STORAGE_UNAVAILABLE)out->unavailable_count++;else if(dst->state==RTP_STORAGE_ERROR)out->error_count++;
  out->count++;
 }
 for(i=0;i<out->count;++i){rtp_storage_device *a=&out->devices[i];if(a->state!=RTP_STORAGE_MOUNTED||!a->mount_point[0])continue;for(j=i+1;j<out->count;++j){rtp_storage_device *b=&out->devices[j];if(b->state==RTP_STORAGE_MOUNTED&&strcmp(a->mount_point,b->mount_point)==0){if(!a->duplicate_mount_point)out->duplicate_mount_count++;a->duplicate_mount_point=1;b->duplicate_mount_point=1;}}}
 return RTP_STORAGE_OK;
}
const char *rtp_storage_kind_name(rtp_storage_kind k){switch(k){case RTP_STORAGE_SD2VITA:return "sd2vita";case RTP_STORAGE_SONY_MEMORY_CARD:return "sony-memory-card";case RTP_STORAGE_INTERNAL:return "internal";case RTP_STORAGE_USB:return "usb";default:return "unknown";}}
const char *rtp_storage_state_name(rtp_storage_state s){switch(s){case RTP_STORAGE_MOUNTED:return "mounted";case RTP_STORAGE_UNMOUNTED:return "unmounted";case RTP_STORAGE_UNAVAILABLE:return "unavailable";case RTP_STORAGE_ERROR:return "error";default:return "invalid";}}
