#include "plugin_compatibility.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void expect_block(const rt_pc_report *r,const char *p,const char *q){size_t i;for(i=0;i<r->count;++i)if(r->findings[i].severity==RT_PC_SEVERITY_BLOCK&&!strcmp(r->findings[i].plugin_id,p)&&!strcmp(r->findings[i].related_plugin_id,q))return;assert(!"expected blocking finding");}
int main(void){
 rt_pc_report r;rt_pc_rule_registry custom;
 const char *storage[]={"yamt","storagemgr"},*ftp_missing[]={"ftp-for-vita"},*ftp_ok[]={"ftp-for-vita","quickmenureborn"};
 const char *usb[]={"vitastream","udcd-uvc"},*sleep[]={"caffeine-for-vita","nosleep","quickmenureborn"};
 const char *perf[]={"psvshell-plus","lolicon"},*review[]={"vitabright-ex","brightness-lut-other","bluetooth-turbo","bluetooth-audio-hook-other"};
 const char *safe[]={"yamt","wifi-turbo","vitastream","quickmenureborn"},*custom_ids[]={"rt-shellbar","shellbat"};
 assert(rt_pc_evaluate(storage,2,&r)==RT_PC_OK&&r.blocking_count==1);expect_block(&r,"yamt","storagemgr");
 assert(rt_pc_evaluate(ftp_missing,1,&r)==RT_PC_OK&&r.blocking_count==1);expect_block(&r,"ftp-for-vita","quickmenureborn");
 assert(rt_pc_evaluate(ftp_ok,2,&r)==RT_PC_OK&&r.count==0);
 assert(rt_pc_evaluate(usb,2,&r)==RT_PC_OK&&r.blocking_count==1);
 assert(rt_pc_evaluate(sleep,3,&r)==RT_PC_OK&&r.blocking_count==1);
 assert(rt_pc_evaluate(perf,2,&r)==RT_PC_OK&&r.blocking_count==1);
 assert(rt_pc_evaluate(review,4,&r)==RT_PC_OK&&r.blocking_count==0&&r.review_count==2);
 assert(rt_pc_evaluate(safe,4,&r)==RT_PC_OK&&r.count==0);
 assert(rt_pc_evaluate(NULL,1,&r)==RT_PC_INVALID);
 rt_pc_rule_registry_init(&custom);
 assert(rt_pc_rule_register(&custom,RT_PC_FINDING_CONFLICT,RT_PC_SEVERITY_BLOCK,"rt-shellbar","shellbat")==RT_PC_OK);
 assert(rt_pc_rule_register(&custom,RT_PC_FINDING_CONFLICT,RT_PC_SEVERITY_BLOCK,"shellbat","rt-shellbar")==RT_PC_INVALID);
 assert(rt_pc_evaluate_registry(custom_ids,2,&custom,&r)==RT_PC_OK&&r.blocking_count==1);expect_block(&r,"rt-shellbar","shellbat");
 assert(rt_pc_rule_register(&custom,RT_PC_FINDING_MISSING_DEPENDENCY,RT_PC_SEVERITY_REVIEW,"my-plugin","my-lib")==RT_PC_INVALID);
 assert(rt_pc_rule_register(&custom,RT_PC_FINDING_CONFLICT,RT_PC_SEVERITY_BLOCK,"same","same")==RT_PC_INVALID);
 puts("Plugin compatibility tests passed.");return 0;
}