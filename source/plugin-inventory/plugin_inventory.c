#include "plugin_inventory.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
static void trim(char *s){size_t n=strlen(s),i=0;while(i<n&&isspace((unsigned char)s[i]))++i;if(i)memmove(s,s+i,n-i+1);n=strlen(s);while(n&&isspace((unsigned char)s[n-1]))s[--n]='\\0';}
static int copy(char *d,size_t cap,const char *s){size_t n=strlen(s);if(n>=cap)return 0;memcpy(d,s,n+1);return 1;}
static int plugin_suffix(const char *s){size_t n=strlen(s);return (n>=6&&strcmp(s+n-6,".skprx")==0)||(n>=6&&strcmp(s+n-6,".suprx")==0);}
static void identify(rt_pi_entry *e){const char *base=strrchr(e->path,'/');char name[RT_PI_PATH_MAX+1];size_t i;base=base?base+1:e->path;if(!copy(name,sizeof(name),base))return;for(i=0;name[i];++i)name[i]=(char)tolower((unsigned char)name[i]);
#define MATCH(file,id) if(strcmp(name,file)==0){copy(e->plugin_id,sizeof(e->plugin_id),id);e->known=1;return;}
 MATCH("wifi_turbo.skprx","wifi-turbo")
 MATCH("vitastream.skprx","vitastream")
 MATCH("storageturbo_kernel.skprx","turbo-storage")
 MATCH("storageturbo.suprx","turbo-storage")
 MATCH("storageturbo_settings.suprx","turbo-storage")
 MATCH("bluetoothturbo_kernel.skprx","bluetooth-turbo")
 MATCH("bluetoothturbo.suprx","bluetooth-turbo")
 MATCH("vitabright.skprx","vitabright-ex")
 MATCH("psvshellplus_kernel.skprx","psvshell-plus")
 MATCH("psvshellplus_shell.suprx","psvshell-plus")
 MATCH("revita.skprx","revita-extended")
 MATCH("ftpforvita.suprx","ftp-for-vita")
 MATCH("caffeine.suprx","caffeine-for-vita")
 MATCH("livearea_nolimits.suprx","livearea-nolimits")
 MATCH("screenshot_noshard.suprx","screenshot-noshard")
 MATCH("repatch_ex.skprx","repatch-reloaded")
 MATCH("kubridge.skprx","kubridge")
 MATCH("ioplus.skprx","ioplus")
 MATCH("yamt_helper.skprx","yamt")
 MATCH("storagemgr.skprx","storagemgr")
 MATCH("quickmenureborn.suprx","quickmenureborn")
#undef MATCH
}
void rt_pi_inventory_init(rt_pi_inventory *v){if(v)memset(v,0,sizeof(*v));}
static int section_line(const char *s,char *section,size_t cap){size_t n=strlen(s);if(n<3||s[0]!='['||s[n-1]!=']')return (s[0]=='['|| (n&&s[n-1]==']'))?-1:0;{char tmp[RT_PI_SECTION_MAX+1];if(n-2>=sizeof(tmp)||n-2>=cap)return -1;memcpy(tmp,s+1,n-2);tmp[n-2]='\\0';trim(tmp);if(!tmp[0])return -1;return copy(section,cap,tmp)?1:-1;}}
int rt_pi_parse(const char *text,size_t length,rt_pi_inventory *out){
 size_t pos=0;unsigned int line=0;char section[RT_PI_SECTION_MAX+1]="";
 if(!text||!out)return RT_PI_INVALID;rt_pi_inventory_init(out);
 while(pos<length){char buf[512],path[RT_PI_PATH_MAX+1];size_t n=0;int disabled=0,sec;char *p,*end;++line;
  while(pos<length&&text[pos]!='\\n'){if(n+1<sizeof(buf))buf[n++]=text[pos];else {out->malformed_lines++;while(pos<length&&text[pos]!='\\n')++pos;break;}++pos;}
  if(pos<length&&text[pos]=='\\n')++pos;buf[n]='\\0';if(n&&buf[n-1]=='\\r')buf[--n]='\\0';trim(buf);if(!buf[0]){out->ignored_lines++;continue;}
  p=buf;if(*p=='#'){disabled=1;++p;while(*p&&isspace((unsigned char)*p))++p;if(!plugin_suffix(p)){out->ignored_lines++;continue;}}
  sec=section_line(p,section,sizeof(section));if(sec==1)continue;if(sec<0){out->malformed_lines++;continue;}
  if(!disabled&&*p=='['){out->malformed_lines++;continue;}
  end=p;while(*end&&!isspace((unsigned char)*end)&&*end!='#')++end;if(end==p){out->malformed_lines++;continue;}
  if((size_t)(end-p)>=sizeof(path)){out->malformed_lines++;continue;}memcpy(path,p,(size_t)(end-p));path[end-p]='\\0';
  if(!plugin_suffix(path)){out->ignored_lines++;continue;}
  if(out->count>=RT_PI_MAX_ENTRIES)return RT_PI_CAPACITY;
  {rt_pi_entry *e=&out->entries[out->count];size_t i;memset(e,0,sizeof(*e));if(!copy(e->path,sizeof(e->path),path)||!copy(e->section,sizeof(e->section),section))return RT_PI_TEXT_TOO_LONG;e->line_number=line;e->enabled=!disabled;e->occurrence=1;
   for(i=0;i<out->count;++i)if(strcmp(out->entries[i].path,e->path)==0){e->occurrence++;out->entries[i].duplicate_count++;}e->duplicate_count=e->occurrence;identify(e);out->count++;}
 }
 return RT_PI_OK;
}
const rt_pi_entry *rt_pi_find_path(const rt_pi_inventory *v,const char *path,size_t occurrence){size_t i;if(!v||!path||!occurrence||v->count>RT_PI_MAX_ENTRIES)return NULL;for(i=0;i<v->count;++i)if(strcmp(v->entries[i].path,path)==0&&v->entries[i].occurrence==occurrence)return &v->entries[i];return NULL;}
void rt_pi_adapter_registry_init(rt_pi_adapter_registry *r){if(r)memset(r,0,sizeof(*r));}
const rt_pi_adapter *rt_pi_adapter_find(const rt_pi_adapter_registry *r,const char *id){size_t i;if(!r||!id||r->count>RT_PI_MAX_ADAPTERS)return NULL;for(i=0;i<r->count;++i)if(strcmp(r->items[i].plugin_id,id)==0)return &r->items[i];return NULL;}
int rt_pi_adapter_register(rt_pi_adapter_registry *r,const char *plugin,const char *adapter){rt_pi_adapter a;if(!r||!plugin||!adapter||!plugin[0]||!adapter[0])return RT_PI_INVALID;if(r->count>RT_PI_MAX_ADAPTERS)return RT_PI_INVALID;if(rt_pi_adapter_find(r,plugin))return RT_PI_DUPLICATE;if(r->count==RT_PI_MAX_ADAPTERS)return RT_PI_CAPACITY;if(!copy(a.plugin_id,sizeof(a.plugin_id),plugin)||!copy(a.adapter_id,sizeof(a.adapter_id),adapter))return RT_PI_TEXT_TOO_LONG;r->items[r->count++]=a;return RT_PI_OK;}
rt_pi_adapter_state rt_pi_entry_adapter_state(const rt_pi_entry *e,const rt_pi_adapter_registry *r){if(!e||!e->known||!rt_pi_adapter_find(r,e->plugin_id))return RT_PI_ADAPTER_INVENTORY_ONLY;return RT_PI_ADAPTER_CONFIGURABLE;}
