#include "plugin_control_centre.h"
#include <string.h>
static int copy_text(char *dst,size_t cap,const char *src){size_t n;if(!dst||!src||!cap)return 0;n=strlen(src);if(n>=cap)return 0;memcpy(dst,src,n+1);return 1;}
static int has_finding(const rt_pc_report *r,const char *id,rt_pc_severity severity){size_t i;for(i=0;i<r->count;++i)if(r->findings[i].severity==severity&&(strcmp(r->findings[i].plugin_id,id)==0||strcmp(r->findings[i].related_plugin_id,id)==0))return 1;return 0;}
int rtp_plugin_centre_build(const rt_pi_inventory *inv,const rt_pi_adapter_registry *adapters,rtp_plugin_centre_snapshot *out){
 const char *enabled[RT_PI_MAX_ENTRIES];size_t enabled_count=0,i;
 if(!inv||!adapters||!out)return RTP_PC_INVALID_ARGUMENT;
 if(inv->count>RT_PI_MAX_ENTRIES||adapters->count>RT_PI_MAX_ADAPTERS)return RTP_PC_INVENTORY_INVALID;
 memset(out,0,sizeof(*out));
 for(i=0;i<inv->count;++i){const rt_pi_entry *e=&inv->entries[i];size_t j;int seen=0;if(e->enabled&&e->known&&e->plugin_id[0]){for(j=0;j<enabled_count;++j)if(strcmp(enabled[j],e->plugin_id)==0){seen=1;break;}if(!seen)enabled[enabled_count++]=e->plugin_id;}}
 if(rt_pc_evaluate(enabled,enabled_count,&out->compatibility)!=RT_PC_OK)return RTP_PC_COMPATIBILITY_ERROR;
 out->blocking_findings=out->compatibility.blocking_count;out->review_findings=out->compatibility.review_count;
 for(i=0;i<inv->count;++i){const rt_pi_entry *src=&inv->entries[i];rtp_plugin_centre_row *row=&out->rows[out->count++];const rt_pi_adapter *adapter=NULL;memset(row,0,sizeof(*row));
  if(!copy_text(row->path,sizeof(row->path),src->path)||!copy_text(row->section,sizeof(row->section),src->section)||!copy_text(row->plugin_id,sizeof(row->plugin_id),src->plugin_id))return RTP_PC_INVENTORY_INVALID;
  row->line_number=src->line_number;row->occurrence=src->occurrence;row->duplicate_count=src->duplicate_count;row->enabled=src->enabled;row->known=src->known;
  if(src->known&&src->plugin_id[0])adapter=rt_pi_adapter_find(adapters,src->plugin_id);
  if(adapter&&!copy_text(row->adapter_id,sizeof(row->adapter_id),adapter->adapter_id))return RTP_PC_ADAPTER_INVALID;
  if(!src->known){row->state=RTP_PC_ROW_UNKNOWN;out->unknown_count++;}
  else if(!src->enabled){row->state=RTP_PC_ROW_DISABLED;out->disabled_count++;}
  else if(has_finding(&out->compatibility,src->plugin_id,RT_PC_SEVERITY_BLOCK)){row->state=RTP_PC_ROW_BLOCKED;out->blocked_count++;}
  else if(has_finding(&out->compatibility,src->plugin_id,RT_PC_SEVERITY_REVIEW)){row->state=RTP_PC_ROW_REVIEW;out->review_count++;}
  else if(adapter){row->state=RTP_PC_ROW_CONFIGURABLE;out->configurable_count++;}
  else{row->state=RTP_PC_ROW_DETECTED;out->detected_count++;}
 }
 return RTP_PC_OK;
}
const char *rtp_plugin_centre_state_name(rtp_plugin_row_state s){switch(s){case RTP_PC_ROW_UNKNOWN:return "unknown";case RTP_PC_ROW_DISABLED:return "disabled";case RTP_PC_ROW_DETECTED:return "detected";case RTP_PC_ROW_CONFIGURABLE:return "configurable";case RTP_PC_ROW_BLOCKED:return "blocked";case RTP_PC_ROW_REVIEW:return "review";default:return "invalid";}}
