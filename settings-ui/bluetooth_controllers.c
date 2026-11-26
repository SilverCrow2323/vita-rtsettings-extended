#include "bluetooth_controllers.h"
#include <string.h>
static int copy_text(char *dst,size_t cap,const char *src,int allow_empty){
 size_t n;if(!src)return 0;n=strlen(src);if((!allow_empty&&!n)||n>=cap)return 0;
 memcpy(dst,src,n+1);return 1;
}
rtp_bt_device_category rtp_bt_classify_device(unsigned int cod){
 unsigned int major=(cod>>8)&0x1fu,minor=(cod>>2)&0x3fu;
 switch(major){
 case 0x01:return RTP_BT_CATEGORY_COMPUTER;
 case 0x02:return RTP_BT_CATEGORY_PHONE;
 case 0x04:return RTP_BT_CATEGORY_AUDIO;
 case 0x05:
  if(minor==0x01||minor==0x02)return RTP_BT_CATEGORY_CONTROLLER;
  if(minor==0x03)return RTP_BT_CATEGORY_REMOTE;
  if(minor==0x10)return RTP_BT_CATEGORY_KEYBOARD;
  if(minor==0x20)return RTP_BT_CATEGORY_MOUSE;
  if(minor==0x30)return RTP_BT_CATEGORY_KEYBOARD;
  return RTP_BT_CATEGORY_OTHER;
 case 0x06:return (minor&0x20u)?RTP_BT_CATEGORY_PRINTER:RTP_BT_CATEGORY_OTHER;
 default:return RTP_BT_CATEGORY_UNKNOWN;
 }
}
int rtp_bt_snapshot_build(const rtp_bt_controller_observation *obs,size_t count,
                          rtp_bt_snapshot *out){
 size_t i,j;
 if(!out||(!obs&&count))return RTP_BT_INVALID_ARGUMENT;
 if(count>RTP_BT_MAX_CONTROLLERS)return RTP_BT_CAPACITY;
 memset(out,0,sizeof(*out));
 for(i=0;i<count;++i){
  const rtp_bt_controller_observation *s=&obs[i];rtp_bt_controller *d=&out->controllers[i];
  if(!copy_text(d->id,sizeof(d->id),s->id,0)||!copy_text(d->label,sizeof(d->label),s->label,1)||
     s->kind<RTP_BT_DEVICE_UNKNOWN||s->kind>RTP_BT_8BITDO||
     s->category<RTP_BT_CATEGORY_UNKNOWN||s->category>RTP_BT_CATEGORY_OTHER||
     s->state<RTP_BT_DISCONNECTED||s->state>RTP_BT_ERROR||
     s->provider<RTP_BT_PROVIDER_UNKNOWN||s->provider>RTP_BT_PROVIDER_8BITVITA||
     s->battery_percent < -1 || s->battery_percent > 100)
   return RTP_BT_INVALID_CONTROLLER;
  for(j=0;j<i;++j)if(strcmp(out->controllers[j].id,d->id)==0)return RTP_BT_DUPLICATE_ID;
  d->kind=s->kind;d->category=s->category;d->category_source=s->category_source;d->state=s->state;d->provider=s->provider;
  d->capabilities=s->capabilities;d->bluetooth_class=s->bluetooth_class;
  d->battery_percent=s->battery_percent;d->driver_active=s->driver_active?1:0;
  if(d->category==RTP_BT_CATEGORY_UNKNOWN && d->bluetooth_class){d->category=rtp_bt_classify_device(d->bluetooth_class);if(d->category!=RTP_BT_CATEGORY_UNKNOWN)d->category_source=RTP_BT_CATEGORY_SOURCE_CLASS_OF_DEVICE;}
  if(d->category==RTP_BT_CATEGORY_UNKNOWN && d->kind!=RTP_BT_DEVICE_UNKNOWN){d->category=RTP_BT_CATEGORY_CONTROLLER;d->category_source=RTP_BT_CATEGORY_SOURCE_CLASS_OF_DEVICE;}
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
  RTP_BT_DRIVER_DS34VITA|RTP_BT_DRIVER_DS3VITA|RTP_BT_DRIVER_DS4VITA|
  RTP_BT_DRIVER_X1VITA|RTP_BT_DRIVER_8BITVITA;
 unsigned int bits=mask&known;
 unsigned int minivita_8bit=RTP_BT_DRIVER_MINIVITATV|RTP_BT_DRIVER_8BITVITA;
 if(conflicts)*conflicts=0;
 if(mask&~known)return RTP_BT_INVALID_ARGUMENT;
 if(bits && bits!=minivita_8bit && (bits&(bits-1u))){
  if(conflicts)*conflicts=bits;return RTP_BT_DRIVER_CONFLICT;
 }
 return RTP_BT_OK;
}
int rtp_bt_set_device_category(rtp_bt_snapshot *s,const char *id,rtp_bt_device_category category){
 size_t i;if(!s||!id||category<RTP_BT_CATEGORY_UNKNOWN||category>RTP_BT_CATEGORY_OTHER)return RTP_BT_INVALID_ARGUMENT;
 for(i=0;i<s->count;++i)if(strcmp(s->controllers[i].id,id)==0){s->controllers[i].category=category;s->controllers[i].category_source=RTP_BT_CATEGORY_SOURCE_USER;return RTP_BT_OK;}
 return RTP_BT_INVALID_CONTROLLER;
}
const char *rtp_bt_device_kind_name(rtp_bt_device_kind k){
 switch(k){case RTP_BT_DUALSHOCK3:return "dualshock-3";case RTP_BT_DUALSHOCK4:return "dualshock-4";
 case RTP_BT_DUALSENSE:return "dualsense";case RTP_BT_DUALSENSE_EDGE:return "dualsense-edge";
 case RTP_BT_XBOX_ONE:return "xbox-one";case RTP_BT_SWITCH_PRO:return "switch-pro";
 case RTP_BT_8BITDO:return "8bitdo";case RTP_BT_OTHER:return "other";default:return "unknown";}
}
const char *rtp_bt_device_category_name(rtp_bt_device_category c){
 switch(c){case RTP_BT_CATEGORY_CONTROLLER:return "controller";case RTP_BT_CATEGORY_AUDIO:return "audio";
 case RTP_BT_CATEGORY_COMPUTER:return "computer";case RTP_BT_CATEGORY_PHONE:return "phone";
 case RTP_BT_CATEGORY_TABLET:return "tablet";case RTP_BT_CATEGORY_KEYBOARD:return "keyboard";
 case RTP_BT_CATEGORY_MOUSE:return "mouse";case RTP_BT_CATEGORY_PRINTER:return "printer";
 case RTP_BT_CATEGORY_REMOTE:return "remote";case RTP_BT_CATEGORY_OTHER:return "other";
 default:return "unknown";}
}
const char *rtp_bt_connection_state_name(rtp_bt_connection_state s){
 switch(s){case RTP_BT_CONNECTED:return "connected";case RTP_BT_PAIRED:return "paired";
 case RTP_BT_DISCONNECTED:return "disconnected";case RTP_BT_UNAVAILABLE:return "unavailable";
 case RTP_BT_ERROR:return "error";default:return "invalid";}
}
const char *rtp_bt_provider_name(rtp_bt_provider p){
 switch(p){case RTP_BT_PROVIDER_VITACONTROL:return "vitacontrol";case RTP_BT_PROVIDER_MINIVITATV:return "minivitatv";
 case RTP_BT_PROVIDER_DS34VITA:return "ds34vita";case RTP_BT_PROVIDER_DS3VITA:return "ds3vita";
 case RTP_BT_PROVIDER_DS4VITA:return "ds4vita";case RTP_BT_PROVIDER_X1VITA:return "x1vita";
 case RTP_BT_PROVIDER_8BITVITA:return "8bitvita";default:return "unknown";}
}
void rtp_bt_input_session_init(rtp_bt_input_session *s){if(s)memset(s,0,sizeof(*s));}
int rtp_bt_input_session_start(rtp_bt_input_session *s){
 if(!s)return RTP_BT_INVALID_ARGUMENT;memset(s,0,sizeof(*s));s->active=1;return RTP_BT_OK;
}
int rtp_bt_input_session_stop(rtp_bt_input_session *s){
 if(!s)return RTP_BT_INVALID_ARGUMENT;if(!s->active)return RTP_BT_INPUT_NOT_ACTIVE;s->active=0;return RTP_BT_OK;
}
int rtp_bt_input_record(rtp_bt_input_session *s,const rtp_bt_input_observation *o){
 size_t i;rtp_bt_input_control *c;
 if(!s||!o||o->kind<RTP_BT_INPUT_BUTTON||o->kind>RTP_BT_INPUT_UNKNOWN||
    (o->label&&strlen(o->label)>RTP_BT_CONTROL_LABEL_MAX))return RTP_BT_INVALID_ARGUMENT;
 if(!s->active)return RTP_BT_INPUT_NOT_ACTIVE;
 for(i=0;i<s->count;++i)if(s->controls[i].code==o->code&&s->controls[i].kind==o->kind)break;
 if(i==s->count){if(s->count>=RTP_BT_MAX_INPUT_EVENTS)return RTP_BT_CAPACITY;
  c=&s->controls[s->count++];memset(c,0,sizeof(*c));c->code=o->code;c->kind=o->kind;
  c->first_value=o->value;if(!copy_text(c->label,sizeof(c->label),o->label?o->label:"",1))return RTP_BT_INVALID_ARGUMENT;
 }else c=&s->controls[i];
 c->last_value=o->value;c->observations++;
 return RTP_BT_OK;
}
