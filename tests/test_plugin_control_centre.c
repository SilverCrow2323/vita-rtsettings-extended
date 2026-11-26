#include "plugin_control_centre.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const rtp_plugin_centre_row *find_row(const rtp_plugin_centre_snapshot *s,const char *path){size_t i;for(i=0;i<s->count;++i)if(strcmp(s->rows[i].path,path)==0)return &s->rows[i];return NULL;}
int main(void){
 const char config[]="*KERNEL\nur0:tai/yamt_helper.skprx\nur0:tai/storagemgr.skprx\nur0:tai/wifi_turbo.skprx\n# ur0:tai/oldthing.skprx\n*main\nur0:tai/ftpforvita.suprx\n";
 rt_pi_inventory inv;rt_pi_adapter_registry adapters;rtp_plugin_centre_snapshot view;const rtp_plugin_centre_row *row;
 assert(rt_pi_parse(config,sizeof(config)-1,&inv)==RT_PI_OK);rt_pi_adapter_registry_init(&adapters);
 assert(rt_pi_adapter_register(&adapters,"wifi-turbo","wifi-turbo")==RT_PI_OK);
 assert(rt_pi_adapter_register(&adapters,"ftp-for-vita","ftp-vita")==RT_PI_OK);
 assert(rtp_plugin_centre_build(&inv,&adapters,&view)==RTP_PC_OK);assert(view.count==5);assert(view.blocking_findings==2);
 row=find_row(&view,"ur0:tai/yamt_helper.skprx");assert(row&&row->state==RTP_PC_ROW_BLOCKED);
 row=find_row(&view,"ur0:tai/storagemgr.skprx");assert(row&&row->state==RTP_PC_ROW_BLOCKED);
 row=find_row(&view,"ur0:tai/wifi_turbo.skprx");assert(row&&row->state==RTP_PC_ROW_CONFIGURABLE);
 row=find_row(&view,"ur0:tai/oldthing.skprx");assert(row&&row->state==RTP_PC_ROW_UNKNOWN);
 row=find_row(&view,"ur0:tai/ftpforvita.suprx");assert(row&&row->state==RTP_PC_ROW_BLOCKED);
 assert(view.unknown_count==1&&view.configurable_count==1&&view.blocked_count==3);
 assert(strcmp(rtp_plugin_centre_state_name(RTP_PC_ROW_REVIEW),"review")==0);
 assert(rtp_plugin_centre_build(NULL,&adapters,&view)==RTP_PC_INVALID_ARGUMENT);
 puts("Plugin Control Centre tests passed.");return 0;
}
