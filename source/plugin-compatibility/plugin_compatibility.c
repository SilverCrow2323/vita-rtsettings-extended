#include "plugin_compatibility.h"
#include <string.h>
typedef struct { const char *plugin; const char *dependency; } dependency_rule;
typedef struct { const char *left; const char *right; rt_pc_severity severity; } conflict_rule;
static const dependency_rule dependencies[] = {
 {"ftp-for-vita","quickmenureborn"},
 {"caffeine-for-vita","quickmenureborn"}
};
static const conflict_rule conflicts[] = {
 {"yamt","storagemgr",RT_PC_SEVERITY_BLOCK},
 {"vitastream","udcd-uvc",RT_PC_SEVERITY_BLOCK},
 {"vitastream","vita-usb-stream",RT_PC_SEVERITY_BLOCK},
 {"ftp-for-vita","ftp-everywhere",RT_PC_SEVERITY_BLOCK},
 {"caffeine-for-vita","nosleep",RT_PC_SEVERITY_BLOCK},
 {"psvshell-plus","lolicon",RT_PC_SEVERITY_BLOCK},
 {"psvshell-plus","nopowerlimits",RT_PC_SEVERITY_BLOCK},
 {"revita-extended","revita-original",RT_PC_SEVERITY_BLOCK},
 {"vitabright-ex","brightness-lut-other",RT_PC_SEVERITY_REVIEW},
 {"bluetooth-turbo","bluetooth-audio-hook-other",RT_PC_SEVERITY_REVIEW}
};
static int has(const char *const *ids,size_t n,const char *id){size_t i;for(i=0;i<n;++i)if(ids[i]&&strcmp(ids[i],id)==0)return 1;return 0;}
static int add(rt_pc_report *r,rt_pc_finding_type type,rt_pc_severity severity,const char *id,const char *related){
 rt_pc_finding *f;size_t a=strlen(id),b=strlen(related);
 if(r->count>=RT_PC_MAX_FINDINGS)return 0;
 if(a>RT_PC_PLUGIN_ID_MAX||b>RT_PC_PLUGIN_ID_MAX)return 0;
 f=&r->findings[r->count++];f->type=type;f->severity=severity;
 memcpy(f->plugin_id,id,a+1);memcpy(f->related_plugin_id,related,b+1);
 if(severity==RT_PC_SEVERITY_BLOCK)r->blocking_count++;else r->review_count++;
 return 1;
}
int rt_pc_evaluate(const char *const *ids,size_t count,rt_pc_report *r){
 size_t i,j;
 if(!r||(!ids&&count))return RT_PC_INVALID;
 memset(r,0,sizeof(*r));
 for(i=0;i<count;++i)if(!ids[i]||!ids[i][0]||strlen(ids[i])>RT_PC_PLUGIN_ID_MAX)return RT_PC_INVALID;
 for(i=0;i<sizeof(dependencies)/sizeof(dependencies[0]);++i){
  if(has(ids,count,dependencies[i].plugin)&&!has(ids,count,dependencies[i].dependency))
   if(!add(r,RT_PC_FINDING_MISSING_DEPENDENCY,RT_PC_SEVERITY_BLOCK,dependencies[i].plugin,dependencies[i].dependency))return RT_PC_CAPACITY;
 }
 for(i=0;i<sizeof(conflicts)/sizeof(conflicts[0]);++i){
  if(has(ids,count,conflicts[i].left)&&has(ids,count,conflicts[i].right))
   if(!add(r,RT_PC_FINDING_CONFLICT,conflicts[i].severity,conflicts[i].left,conflicts[i].right))return RT_PC_CAPACITY;
 }
 /* Duplicate IDs do not create duplicate findings: rules are evaluated once. */
 for(i=0;i<r->count;++i)for(j=i+1;j<r->count;){
  if(r->findings[i].type==r->findings[j].type&&
     strcmp(r->findings[i].plugin_id,r->findings[j].plugin_id)==0&&
     strcmp(r->findings[i].related_plugin_id,r->findings[j].related_plugin_id)==0){
   size_t k;for(k=j+1;k<r->count;++k)r->findings[k-1]=r->findings[k];r->count--;
   if(r->findings[i].severity==RT_PC_SEVERITY_BLOCK)r->blocking_count--;else r->review_count--;
  }else ++j;
 }
 return RT_PC_OK;
}
const char *rt_pc_finding_type_name(rt_pc_finding_type t){switch(t){case RT_PC_FINDING_MISSING_DEPENDENCY:return "missing-dependency";case RT_PC_FINDING_CONFLICT:return "conflict";default:return "unknown";}}
const char *rt_pc_severity_name(rt_pc_severity s){switch(s){case RT_PC_SEVERITY_BLOCK:return "block";case RT_PC_SEVERITY_REVIEW:return "review";default:return "unknown";}}
