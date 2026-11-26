#include "plugin_compatibility.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void expect_block(const rt_pc_report *r,const char *plugin,const char *related){
 size_t i;for(i=0;i<r->count;++i)if(r->findings[i].severity==RT_PC_SEVERITY_BLOCK&&
 strcmp(r->findings[i].plugin_id,plugin)==0&&strcmp(r->findings[i].related_plugin_id,related)==0)return;
 assert(!"expected blocking finding");
}
int main(void){
 rt_pc_report r;
 const char *storage[]={"yamt","storagemgr"};
 const char *ftp_missing[]={"ftp-for-vita"};
 const char *ftp_ok[]={"ftp-for-vita","quickmenureborn"};
 const char *usb[]={"vitastream","udcd-uvc"};
 const char *sleep[]={"caffeine-for-vita","nosleep","quickmenureborn"};
 const char *perf[]={"psvshell-plus","lolicon"};
 const char *review[]={"vitabright-ex","brightness-lut-other","bluetooth-turbo","bluetooth-audio-hook-other"};
 const char *safe[]={"yamt","wifi-turbo","vitastream","quickmenureborn"};
 assert(rt_pc_evaluate(storage,2,&r)==RT_PC_OK);assert(r.blocking_count==1);expect_block(&r,"yamt","storagemgr");
 assert(rt_pc_evaluate(ftp_missing,1,&r)==RT_PC_OK);assert(r.blocking_count==1);expect_block(&r,"ftp-for-vita","quickmenureborn");
 assert(rt_pc_evaluate(ftp_ok,2,&r)==RT_PC_OK);assert(r.count==0);
 assert(rt_pc_evaluate(usb,2,&r)==RT_PC_OK);assert(r.blocking_count==1);expect_block(&r,"vitastream","udcd-uvc");
 assert(rt_pc_evaluate(sleep,3,&r)==RT_PC_OK);assert(r.blocking_count==1);expect_block(&r,"caffeine-for-vita","nosleep");
 assert(rt_pc_evaluate(perf,2,&r)==RT_PC_OK);assert(r.blocking_count==1);
 assert(rt_pc_evaluate(review,4,&r)==RT_PC_OK);assert(r.blocking_count==0&&r.review_count==2);
 assert(rt_pc_evaluate(safe,4,&r)==RT_PC_OK);assert(r.count==0);
 assert(rt_pc_evaluate(NULL,1,&r)==RT_PC_INVALID);
 assert(strcmp(rt_pc_severity_name(RT_PC_SEVERITY_BLOCK),"block")==0);
 puts("Plugin compatibility tests passed.");return 0;
}
