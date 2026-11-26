#include "preferences.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RTP_FILE_HEADER "RTSPREFS\t1\n"
#define RTP_LINE_MAX 160
static int valid_id(const char *id,int allow_empty){
 size_t i;if(!id||(!allow_empty&&id[0]=='\0')||strlen(id)>RTP_ID_MAX)return 0;
 for(i=0;id[i];++i){char c=id[i];if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return 0;}return 1;
}
static int valid_kind(rtp_entry_kind k){return k>=RTP_ENTRY_CATEGORY&&k<=RTP_ENTRY_ACTION;}
static size_t find_index(const rtp_preferences *p,const char *id){
 size_t i;if(!p||!id)return RTP_MAX_ENTRIES;for(i=0;i<p->count;++i)if(strcmp(p->entries[i].id,id)==0)return i;return RTP_MAX_ENTRIES;
}
static int validate_tree(const rtp_preferences *p){
 size_t i,depth;if(!p||p->count>RTP_MAX_ENTRIES)return RTP_PREFS_INVALID_DATA;
 for(i=0;i<p->count;++i){const rtp_menu_entry *e=&p->entries[i];size_t parent;
  if(!valid_id(e->id,0)||!valid_id(e->parent_id,1)||!valid_kind(e->kind)||(e->visible!=0&&e->visible!=1)||(e->default_visible!=0&&e->default_visible!=1))return RTP_PREFS_INVALID_DATA;
  if(find_index(p,e->id)!=i)return RTP_PREFS_DUPLICATE_ID;
  if(e->parent_id[0]){parent=find_index(p,e->parent_id);if(parent==RTP_MAX_ENTRIES||p->entries[parent].kind!=RTP_ENTRY_CATEGORY)return RTP_PREFS_INVALID_PARENT;}
  if(e->kind==RTP_ENTRY_CATEGORY){size_t current=i;for(depth=0;depth<=p->count;++depth){const char *pid=p->entries[current].parent_id;if(!pid[0])break;current=find_index(p,pid);if(current==RTP_MAX_ENTRIES)return RTP_PREFS_INVALID_PARENT;if(current==i)return RTP_PREFS_CYCLE;}if(depth>p->count)return RTP_PREFS_CYCLE;}
 }return RTP_PREFS_OK;
}
void rtp_preferences_init(rtp_preferences *p){if(p)memset(p,0,sizeof(*p));}
int rtp_preferences_register(rtp_preferences *p,const char *id,const char *parent,rtp_entry_kind kind,int visible,unsigned int order){
 rtp_menu_entry e;size_t pi;if(!p||!valid_id(id,0)||!valid_id(parent?parent:"",1)||!valid_kind(kind)||(visible!=0&&visible!=1))return RTP_PREFS_INVALID_ARGUMENT;
 if(p->count>=RTP_MAX_ENTRIES)return RTP_PREFS_CAPACITY;if(find_index(p,id)!=RTP_MAX_ENTRIES)return RTP_PREFS_DUPLICATE_ID;
 if(parent&&parent[0]){pi=find_index(p,parent);if(pi==RTP_MAX_ENTRIES||p->entries[pi].kind!=RTP_ENTRY_CATEGORY)return RTP_PREFS_INVALID_PARENT;}
 memset(&e,0,sizeof(e));strcpy(e.id,id);if(parent)strcpy(e.parent_id,parent);e.kind=kind;e.default_visible=e.visible=visible;e.default_order=e.order=order;p->entries[p->count++]=e;return RTP_PREFS_OK;
}
int rtp_preferences_set_visible(rtp_preferences *p,const char *id,int visible){size_t i;if(!p||!id||(visible!=0&&visible!=1))return RTP_PREFS_INVALID_ARGUMENT;i=find_index(p,id);if(i==RTP_MAX_ENTRIES)return RTP_PREFS_NOT_FOUND;p->entries[i].visible=visible;return RTP_PREFS_OK;}
int rtp_preferences_set_order(rtp_preferences *p,const char *id,unsigned int order){size_t i;if(!p||!id)return RTP_PREFS_INVALID_ARGUMENT;i=find_index(p,id);if(i==RTP_MAX_ENTRIES)return RTP_PREFS_NOT_FOUND;p->entries[i].order=order;return RTP_PREFS_OK;}
int rtp_preferences_set_parent(rtp_preferences *p,const char *id,const char *new_parent){
 size_t i,pi;char old[RTP_ID_MAX+1];int rc;if(!p||!valid_id(id,0)||!valid_id(new_parent?new_parent:"",1))return RTP_PREFS_INVALID_ARGUMENT;
 i=find_index(p,id);if(i==RTP_MAX_ENTRIES)return RTP_PREFS_NOT_FOUND;
 if(new_parent&&new_parent[0]){pi=find_index(p,new_parent);if(pi==RTP_MAX_ENTRIES||p->entries[pi].kind!=RTP_ENTRY_CATEGORY)return RTP_PREFS_INVALID_PARENT;}
 strcpy(old,p->entries[i].parent_id);strcpy(p->entries[i].parent_id,new_parent?new_parent:"");rc=validate_tree(p);if(rc!=RTP_PREFS_OK){strcpy(p->entries[i].parent_id,old);return rc;}return RTP_PREFS_OK;
}
int rtp_preferences_reset_entry(rtp_preferences *p,const char *id){size_t i;if(!p||!id)return RTP_PREFS_INVALID_ARGUMENT;i=find_index(p,id);if(i==RTP_MAX_ENTRIES)return RTP_PREFS_NOT_FOUND;p->entries[i].visible=p->entries[i].default_visible;p->entries[i].order=p->entries[i].default_order;return RTP_PREFS_OK;}
void rtp_preferences_reset_all(rtp_preferences *p){size_t i;if(!p)return;for(i=0;i<p->count;++i){p->entries[i].visible=p->entries[i].default_visible;p->entries[i].order=p->entries[i].default_order;}}
const rtp_menu_entry *rtp_preferences_find(const rtp_preferences *p,const char *id){size_t i=find_index(p,id);return i==RTP_MAX_ENTRIES?NULL:&p->entries[i];}
int rtp_preferences_save(const rtp_preferences *p,const char *path){
 char tmp[RTP_PATH_MAX+1];FILE *f;size_t i;int rc;if(!p||!path||!path[0]||strlen(path)>RTP_PATH_MAX-4)return RTP_PREFS_INVALID_ARGUMENT;
 rc=validate_tree(p);if(rc!=RTP_PREFS_OK)return rc;strcpy(tmp,path);strcat(tmp,".tmp");f=fopen(tmp,"wb");if(!f)return RTP_PREFS_IO_ERROR;
 if(fputs(RTP_FILE_HEADER,f)==EOF)goto fail;
 for(i=0;i<p->count;++i){const rtp_menu_entry *e=&p->entries[i];if(fprintf(f,"%s\t%s\t%d\t%d\t%u\n",e->id,e->parent_id,(int)e->kind,e->visible,e->order)<0)goto fail;}
 if(fflush(f)!=0)goto fail;if(fclose(f)!=0){remove(tmp);return RTP_PREFS_IO_ERROR;}if(rename(tmp,path)!=0){remove(tmp);return RTP_PREFS_IO_ERROR;}return RTP_PREFS_OK;
fail:fclose(f);remove(tmp);return RTP_PREFS_IO_ERROR;
}
static int parse_uint(const char *s,unsigned int *v){char *end;unsigned long n;if(!s||!s[0]||!v)return 0;n=strtoul(s,&end,10);if(*end||n>0xffffffffUL)return 0;*v=(unsigned int)n;return 1;}
int rtp_preferences_load(rtp_preferences *p,const char *path){
 FILE *f;rtp_preferences tmp;char line[RTP_LINE_MAX];if(!p||!path||!path[0])return RTP_PREFS_INVALID_ARGUMENT;f=fopen(path,"rb");if(!f)return RTP_PREFS_IO_ERROR;rtp_preferences_init(&tmp);
 if(!fgets(line,sizeof(line),f)||strcmp(line,RTP_FILE_HEADER)!=0){fclose(f);return RTP_PREFS_INVALID_DATA;}
 while(fgets(line,sizeof(line),f)){char *v[5],*cur;unsigned int kind,visible,order;size_t i;
  if(!strchr(line,'\n')&&!feof(f)){fclose(f);return RTP_PREFS_INVALID_DATA;}line[strcspn(line,"\r\n")]='\0';if(!line[0])continue;v[0]=line;cur=line;
  for(i=1;i<5;++i){cur=strchr(cur,'\t');if(!cur){fclose(f);return RTP_PREFS_INVALID_DATA;}*cur++='\0';v[i]=cur;}
  if(strchr(v[4],'\t')||!valid_id(v[0],0)||!valid_id(v[1],1)||!parse_uint(v[2],&kind)||!parse_uint(v[3],&visible)||!parse_uint(v[4],&order)||kind<1||kind>4||visible>1||tmp.count>=RTP_MAX_ENTRIES||find_index(&tmp,v[0])!=RTP_MAX_ENTRIES){fclose(f);return RTP_PREFS_INVALID_DATA;}
  if(rtp_preferences_register(&tmp,v[0],v[1],(rtp_entry_kind)kind,(int)visible,order)!=RTP_PREFS_OK){fclose(f);return RTP_PREFS_INVALID_DATA;}
 }
 if(ferror(f)){fclose(f);return RTP_PREFS_IO_ERROR;}fclose(f);if(validate_tree(&tmp)!=RTP_PREFS_OK)return RTP_PREFS_INVALID_DATA;*p=tmp;return RTP_PREFS_OK;
}
