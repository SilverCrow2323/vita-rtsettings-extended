#include "bluetooth_controllers.h"
#include <string.h>
static int copy_text(char *dst,size_t cap,const char *src){
 size_t n;if(!src)return 0;n=strlen(src);if(!n||n>=cap)return 0;
 memcpy(dst,src,n+1);return 1;
}
int rtp_bt_snapshot_build(const rtp_bt_controller_observation *obs,size_t count,
                          rtp_bt_snapshot *out){
 size_t i,j;
 if(!out||(!obs&&count))return RTP_BT_INVALID_ARGUMENT;
 if(count>RTP_BT_MAX_CONTROLLERS)return RTP_BT_CAPACITY;
 memset(out,0,sizeof(*out));
 for(i=0;i<count;++i){
  const rtp_bt_controller_observation *s=&obs[i];
  rtp_bt_controller *d=&out->controllers[i];
  if(!copy_text(d->id,sizeof(d->id),s->id)||
     !copy_text(d->label,sizeof(d->label),s->label)||
     s->kind<RTP_BT_DEVICE_UNKNOWN||s->kind>RTP_BT_OTHER||
     s->state<RTP_BT_DISCONNECTED||s->state>RTP_BT_ERROR||
     s->provider<RTP_BT_PROVIDER_UNKNOWN||s->provider>RTP_BT_PROVIDER_DS4VITA||
     s->battery_percent < -1 || s->battery_percent > 100)
   return RTP_BT_INVALID_CONTROLLER;
  for(j=0;j<i;++j)if(strcmp(out->controllers[j].id,d->id)==0)
   return RTP_BT_DUPLICATE_ID;
  d->kind=s->kind;d->state=s->state;d->provider=s->provider;
  d->capabilities=s->capabilities;d->battery_percent=s->battery_percent;
  d->driver_active=s->driver_active?1:0;
  if(d->state==RTP_BT_CONNECTED)out->connected_count++;
  else if(d->state==RTP_BT_PAIRED)out->paired_count++;
  else if(d->state==RTP_BT_UNAVAILABLE)out->unavailable_count++;
  else if(d->state==RTP_BT_ERROR)out->error_count++;
  out->count++;
 }
 return RTP_BT_OK;
}
int rtp_bt_validate_driver_stack(unsigned int mask,unsigned int *conflicts){
 unsigned int known=RTP_BT_DRIVER_VITACONTROL|RTP_BT_DRIVER_MINIVITATV|
  RTP_BT_DRIVER_DS34VITA|RTP_BT_DRIVER_DS3VITA|RTP_BT_DRIVER_DS4VITA;
 unsigned int bits=mask&known;
 if(conflicts)*conflicts=0;
 if(mask&~known)return RTP_BT_INVALID_ARGUMENT;
 if(bits && (bits&(bits-1u))){
  if(conflicts)*conflicts=bits;
  return RTP_BT_DRIVER_CONFLICT;
 }
 return RTP_BT_OK;
}
const char *rtp_bt_device_kind_name(rtp_bt_device_kind k){
 switch(k){case RTP_BT_DUALSHOCK3:return "dualshock-3";case RTP_BT_DUALSHOCK4:return "dualshock-4";
 case RTP_BT_DUALSENSE:return "dualsense";case RTP_BT_DUALSENSE_EDGE:return "dualsense-edge";
 case RTP_BT_XBOX_ONE:return "xbox-one";case RTP_BT_SWITCH_PRO:return "switch-pro";
 case RTP_BT_OTHER:return "other";default:return "unknown";}
}
const char *rtp_bt_connection_state_name(rtp_bt_connection_state s){
 switch(s){case RTP_BT_CONNECTED:return "connected";case RTP_BT_PAIRED:return "paired";
 case RTP_BT_DISCONNECTED:return "disconnected";case RTP_BT_UNAVAILABLE:return "unavailable";
 case RTP_BT_ERROR:return "error";default:return "invalid";}
}
const char *rtp_bt_provider_name(rtp_bt_provider p){
 switch(p){case RTP_BT_PROVIDER_VITACONTROL:return "vitacontrol";
 case RTP_BT_PROVIDER_MINIVITATV:return "minivitatv";case RTP_BT_PROVIDER_DS34VITA:return "ds34vita";
 case RTP_BT_PROVIDER_DS3VITA:return "ds3vita";case RTP_BT_PROVIDER_DS4VITA:return "ds4vita";
 default:return "unknown";}
}
