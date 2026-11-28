#include "plugin_compatibility.h"
#include <string.h>
typedef struct { const char *plugin,*dependency; } dependency_rule;
typedef struct { const char *left,*right; rt_pc_severity severity; } conflict_rule;
static const dependency_rule dependencies[]={{"ftp-for-vita","quickmenureborn"},{"caffeine-for-vita","quickmenureborn"}};
static const conflict_rule conflicts[]={
 {"yamt","storagemgr",RT_PC_SEVERITY_BLOCK},
 {"vitastream","udcd-uvc",RT_PC_SEVERITY_BLOCK},
 {"vitastream","vita-usb-stream",RT_PC_SEVERITY_BLOCK},
 {"ftp-for-vita","ftp-everywhere",RT_PC_SEVERITY_BLOCK},
 {"caffeine-for-vita","nosleep",RT_PC_SEVERITY_BLOCK},
 {"psvshell-plus","lolicon",RT_PC_SEVERITY_BLOCK},
 {"psvshell-plus","nopowerlimits",RT_PC_SEVERITY_BLOCK},
 {"revita-extended","revita-original",RT_PC_SEVERITY_BLOCK},
 {"quickmenureborn","quick-menu-plus",RT_PC_SEVERITY_REVIEW},
 {"psvshell","lolicon",RT_PC_SEVERITY_REVIEW},
 {"psvshell","oclock-vita",RT_PC_SEVERITY_REVIEW},
 {"lolicon","oclock-vita",RT_PC_SEVERITY_REVIEW},
 {"vitabright-ex","brightness-lut-other",RT_PC_SEVERITY_REVIEW},
 {"bluetooth-turbo","bluetooth-audio-hook-other",RT_PC_SEVERITY_REVIEW}
};
static int valid_id(const char *s){return s&&s[0]&&strlen(s)<=RT_PC_PLUGIN_ID_MAX;}
static int has(const char *const *ids,size_t n,const char *id){size_t i;for(i=0;i<n;++i)if(!strcmp(ids[i],id))return 1;return 0;}
void rt_pc_rule_registry_init(rt_pc_rule_registry *r){if(r)memset(r,0,sizeof(*r));}
static int same_rule(const rt_pc_rule *r,rt_pc_finding_type t,rt_pc_severity s,const char *a,const char *b){
 if(r->type!=t||r->severity!=s)return 0;
 return (!strcmp(r->plugin_id,a)&&!strcmp(r->related_plugin_id,b))||
 (t==RT_PC_FINDING_CONFLICT&&!strcmp(r->plugin_id,b)&&!strcmp(r->related_plugin_id,a));
}
int rt_pc_rule_register(rt_pc_rule_registry *r,rt_pc_finding_type t,rt_pc_severity s,const char *a,const char *b){
 size_t i;rt_pc_rule *x;
 if(!r||r->count>RT_PC_MAX_RULES||!valid_id(a)||!valid_id(b)||!strcmp(a,b))return RT_PC_INVALID;
 if((t!=RT_PC_FINDING_MISSING_DEPENDENCY&&t!=RT_PC_FINDING_CONFLICT)||(s!=RT_PC_SEVERITY_BLOCK&&s!=RT_PC_SEVERITY_REVIEW))return RT_PC_INVALID;
 if(t==RT_PC_FINDING_MISSING_DEPENDENCY&&s!=RT_PC_SEVERITY_BLOCK)return RT_PC_INVALID;
 for(i=0;i<r->count;++i)if(same_rule(&r->rules[i],t,s,a,b))return RT_PC_INVALID;
 if(r->count>=RT_PC_MAX_RULES)return RT_PC_CAPACITY;
 x=&r->rules[r->count++];memset(x,0,sizeof(*x));x->type=t;x->severity=s;strcpy(x->plugin_id,a);strcpy(x->related_plugin_id,b);return RT_PC_OK;
}
static int add(rt_pc_report *r,const rt_pc_rule *x){
 rt_pc_finding *f;if(r->count>=RT_PC_MAX_FINDINGS)return 0;f=&r->findings[r->count++];
 memset(f,0,sizeof(*f));f->type=x->type;f->severity=x->severity;strcpy(f->plugin_id,x->plugin_id);strcpy(f->related_plugin_id,x->related_plugin_id);
 if(x->severity==RT_PC_SEVERITY_BLOCK)r->blocking_count++;else r->review_count++;return 1;
}
int rt_pc_evaluate_registry(const char *const *ids,size_t n,const rt_pc_rule_registry *registry,rt_pc_report *r){
 size_t i;if(!r||!registry||registry->count>RT_PC_MAX_RULES||(!ids&&n))return RT_PC_INVALID;memset(r,0,sizeof(*r));
 for(i=0;i<n;++i)if(!valid_id(ids[i]))return RT_PC_INVALID;
 for(i=0;i<registry->count;++i){const rt_pc_rule *x=&registry->rules[i];int hit=has(ids,n,x->plugin_id);
  if(x->type==RT_PC_FINDING_MISSING_DEPENDENCY)hit=hit&&!has(ids,n,x->related_plugin_id);
  else if(x->type==RT_PC_FINDING_CONFLICT)hit=hit&&has(ids,n,x->related_plugin_id);else return RT_PC_INVALID;
  if((x->severity!=RT_PC_SEVERITY_BLOCK&&x->severity!=RT_PC_SEVERITY_REVIEW)||(x->type==RT_PC_FINDING_MISSING_DEPENDENCY&&x->severity!=RT_PC_SEVERITY_BLOCK))return RT_PC_INVALID;
  if(hit&&!add(r,x))return RT_PC_CAPACITY;
 }return RT_PC_OK;
}
int rt_pc_evaluate(const char *const *ids,size_t n,rt_pc_report *r){
 rt_pc_rule_registry reg;size_t i;int rc;rt_pc_rule_registry_init(&reg);
 for(i=0;i<sizeof(dependencies)/sizeof(dependencies[0]);++i){rc=rt_pc_rule_register(&reg,RT_PC_FINDING_MISSING_DEPENDENCY,RT_PC_SEVERITY_BLOCK,dependencies[i].plugin,dependencies[i].dependency);if(rc)return rc;}
 for(i=0;i<sizeof(conflicts)/sizeof(conflicts[0]);++i){rc=rt_pc_rule_register(&reg,RT_PC_FINDING_CONFLICT,conflicts[i].severity,conflicts[i].left,conflicts[i].right);if(rc)return rc;}
 return rt_pc_evaluate_registry(ids,n,&reg,r);
}
const char *rt_pc_finding_type_name(rt_pc_finding_type t){switch(t){case RT_PC_FINDING_MISSING_DEPENDENCY:return "missing-dependency";case RT_PC_FINDING_CONFLICT:return "conflict";default:return "unknown";}}
const char *rt_pc_severity_name(rt_pc_severity s){switch(s){case RT_PC_SEVERITY_BLOCK:return "block";case RT_PC_SEVERITY_REVIEW:return "review";default:return "unknown";}}
