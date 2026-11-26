#include "wifi_turbo_adapter.h"
#include <stdio.h>
#include <string.h>
static int copy_host(char *dst,size_t cap,const char *src){size_t n;if(!src)return 0;n=strlen(src);if(!n||n>=cap)return 0;memcpy(dst,src,n+1);return 1;}
static int valid_client(const rt_wt_client *c){return c&&c->exchange&&c->host[0]&&c->port&&c->timeout_ms;}
int rt_wt_client_init(rt_wt_client *c,const char *host,unsigned short port,unsigned int timeout,int enabled,rt_wt_exchange_fn fn,void *ctx){
 if(!c||!fn||!host||!port||!timeout)return RT_WT_INVALID;
 memset(c,0,sizeof(*c));if(!copy_host(c->host,sizeof(c->host),host))return RT_WT_INVALID;
 c->port=port;c->timeout_ms=timeout;c->developer_remote_control_enabled=enabled?1:0;c->exchange=fn;c->context=ctx;return RT_WT_OK;
}
static int send(rt_wt_client *c,const char *command,char *response,size_t cap){
 int rc;if(!valid_client(c)||!command||!response||cap<2)return RT_WT_INVALID;
 response[0]='\0';if(!c->developer_remote_control_enabled)return RT_WT_UNAVAILABLE;
 rc=c->exchange(c->context,c->host,c->port,command,c->timeout_ms,response,cap);
 if(rc!=0)return RT_WT_TRANSPORT_ERROR;
 if(!memchr(response,'\0',cap))return RT_WT_RESPONSE_TOO_LARGE;
 return RT_WT_OK;
}
int rt_wt_get_status(rt_wt_client *c,char *r,size_t n){return send(c,"status",r,n);}
int rt_wt_set_toggle(rt_wt_client *c,rt_wt_toggle t,int on,char *r,size_t n){
 const char *name;char cmd[40];
 switch(t){case RT_WT_TOGGLE_KEEP_WIFI:name="keepwifi";break;case RT_WT_TOGGLE_HUD:name="hud";break;case RT_WT_TOGGLE_AUTO_RECONNECT:name="autoreconnect";break;case RT_WT_TOGGLE_ROAM:name="roam";break;default:return RT_WT_INVALID;}
 if(on!=0&&on!=1)return RT_WT_INVALID;
 if(snprintf(cmd,sizeof(cmd),"%s %s",name,on?"on":"off")<0)return RT_WT_INVALID;
 return send(c,cmd,r,n);
}
int rt_wt_set_profile(rt_wt_client *c,rt_wt_profile p,char *r,size_t n){
 const char *v;char cmd[32];int rc;
 switch(p){case RT_WT_PROFILE_MAX_PERFORMANCE:v="maxperf";break;case RT_WT_PROFILE_BALANCED:v="balanced";break;case RT_WT_PROFILE_BATTERY_SAVER:v="maxbatt";break;default:return RT_WT_INVALID;}
 if(snprintf(cmd,sizeof(cmd),"profile %s",v)<0)return RT_WT_INVALID;
 rc=send(c,cmd,r,n);return rc==RT_WT_OK?RT_WT_PENDING_CONFIRM:rc;
}
int rt_wt_set_ht40(rt_wt_client *c,int on,char *r,size_t n){
 char cmd[16];int rc;if(on!=0&&on!=1)return RT_WT_INVALID;
 if(snprintf(cmd,sizeof(cmd),"ht40 %s",on?"on":"off")<0)return RT_WT_INVALID;
 rc=send(c,cmd,r,n);return rc==RT_WT_OK?RT_WT_PENDING_CONFIRM:rc;
}
int rt_wt_confirm(rt_wt_client *c,char *r,size_t n){return send(c,"confirm",r,n);}
int rt_wt_revert(rt_wt_client *c,char *r,size_t n){return send(c,"revert",r,n);}
int rt_wt_roam_now(rt_wt_client *c,char *r,size_t n){return send(c,"roam now",r,n);}
int rt_wt_connect_saved_networks(rt_wt_client *c,char *r,size_t n){return send(c,"icm",r,n);}
