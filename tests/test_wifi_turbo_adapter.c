#include "wifi_turbo_adapter.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct { char command[64],host[64];unsigned short port;unsigned int timeout;int calls,fail; } mock;
static int exchange(void *ctx,const char *host,unsigned short port,const char *cmd,unsigned int timeout,char *response,size_t cap){
 mock *m=(mock*)ctx;m->calls++;strncpy(m->command,cmd,sizeof(m->command)-1);strncpy(m->host,host,sizeof(m->host)-1);m->port=port;m->timeout=timeout;
 if(m->fail)return -1;if(cap<3)return -1;strncpy(response,"ok",cap-1);response[cap-1]='\0';return 0;
}
int main(void){
 rt_wt_client c;mock m;char response[128];memset(&m,0,sizeof(m));
 assert(rt_wt_client_init(&c,"192.168.1.20",RT_WT_DEFAULT_PORT,RT_WT_DEFAULT_TIMEOUT_MS,1,exchange,&m)==RT_WT_OK);
 assert(rt_wt_get_status(&c,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"status")==0);
 assert(m.port==1339&&m.timeout==2500&&strcmp(m.host,"192.168.1.20")==0);
 assert(rt_wt_set_toggle(&c,RT_WT_TOGGLE_KEEP_WIFI,0,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"keepwifi off")==0);
 assert(rt_wt_set_toggle(&c,RT_WT_TOGGLE_HUD,1,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"hud on")==0);
 assert(rt_wt_set_toggle(&c,RT_WT_TOGGLE_AUTO_RECONNECT,1,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"autoreconnect on")==0);
 assert(rt_wt_set_toggle(&c,RT_WT_TOGGLE_ROAM,1,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"roam on")==0);
 assert(rt_wt_set_profile(&c,RT_WT_PROFILE_MAX_PERFORMANCE,response,sizeof(response))==RT_WT_PENDING_CONFIRM);assert(strcmp(m.command,"profile maxperf")==0);
 assert(rt_wt_confirm(&c,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"confirm")==0);
 assert(rt_wt_set_profile(&c,RT_WT_PROFILE_BATTERY_SAVER,response,sizeof(response))==RT_WT_PENDING_CONFIRM);assert(strcmp(m.command,"profile maxbatt")==0);
 assert(rt_wt_revert(&c,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"revert")==0);
 assert(rt_wt_set_ht40(&c,1,response,sizeof(response))==RT_WT_PENDING_CONFIRM);assert(strcmp(m.command,"ht40 on")==0);
 assert(rt_wt_roam_now(&c,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"roam now")==0);
 assert(rt_wt_connect_saved_networks(&c,response,sizeof(response))==RT_WT_OK);assert(strcmp(m.command,"icm")==0);
 c.developer_remote_control_enabled=0;assert(rt_wt_get_status(&c,response,sizeof(response))==RT_WT_UNAVAILABLE);
 c.developer_remote_control_enabled=1;m.fail=1;assert(rt_wt_get_status(&c,response,sizeof(response))==RT_WT_TRANSPORT_ERROR);
 assert(rt_wt_set_toggle(&c,(rt_wt_toggle)99,1,response,sizeof(response))==RT_WT_INVALID);
 assert(rt_wt_client_init(&c,"",1339,1000,1,exchange,&m)==RT_WT_INVALID);
 puts("Wi-Fi Turbo adapter tests passed.");return 0;
}
