#include "layout_profiles.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define PROFILE_HEADER "RTSPROFILES\t1\n"
#define LINE_MAX_PROFILE 640
static int valid_id(const char *s){size_t i;if(!s||!s[0]||strlen(s)>RTP_ID_MAX)return 0;for(i=0;s[i];i++){char c=s[i];if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return 0;}return 1;}
static int valid_name(const char *s){size_t i,n;if(!s||(n=strlen(s))==0||n>RTP_PROFILE_NAME_MAX)return 0;for(i=0;i<n;i++)if((unsigned char)s[i]<32||s[i]=='\t')return 0;return 1;}
static size_t index_of(const rtp_profile_store *s,const char *id){size_t i;if(!s||!id)return RTP_PROFILE_MAX;for(i=0;i<s->count;i++)if(!strcmp(s->profiles[i].id,id))return i;return RTP_PROFILE_MAX;}
const rtp_layout_profile *rtp_profiles_find(const rtp_profile_store *s,const char *id){size_t i=index_of(s,id);return i==RTP_PROFILE_MAX?NULL:&s->profiles[i];}
void rtp_profiles_init(rtp_profile_store *s){if(s)memset(s,0,sizeof(*s));}
int rtp_profiles_create(rtp_profile_store *s,const char *id,const char *name,const rtp_preferences *p){
 rtp_layout_profile *v;if(!s||!p||!valid_id(id)||!valid_name(name))return RTP_PROFILE_INVALID_ARGUMENT;
 if(s->count>=RTP_PROFILE_MAX)return RTP_PROFILE_CAPACITY;if(index_of(s,id)!=RTP_PROFILE_MAX)return RTP_PROFILE_DUPLICATE;
 if(p->count>RTP_MAX_ENTRIES)return RTP_PROFILE_INVALID_DATA;v=&s->profiles[s->count++];memset(v,0,sizeof(*v));strcpy(v->id,id);strcpy(v->name,name);v->count=p->count;memcpy(v->entries,p->entries,p->count*sizeof(p->entries[0]));
 if(!s->active_id[0])strcpy(s->active_id,id);return RTP_PROFILE_OK;
}
int rtp_profiles_update(rtp_profile_store *s,const char *id,const rtp_preferences *p){size_t i;if(!s||!p||p->count>RTP_MAX_ENTRIES)return RTP_PROFILE_INVALID_ARGUMENT;i=index_of(s,id);if(i==RTP_PROFILE_MAX)return RTP_PROFILE_NOT_FOUND;s->profiles[i].count=p->count;memcpy(s->profiles[i].entries,p->entries,p->count*sizeof(p->entries[0]));return RTP_PROFILE_OK;}
int rtp_profiles_rename(rtp_profile_store *s,const char *id,const char *name){size_t i;if(!s||!valid_name(name))return RTP_PROFILE_INVALID_ARGUMENT;i=index_of(s,id);if(i==RTP_PROFILE_MAX)return RTP_PROFILE_NOT_FOUND;if(!strcmp(id,"default"))return RTP_PROFILE_PROTECTED;strcpy(s->profiles[i].name,name);return RTP_PROFILE_OK;}
int rtp_profiles_delete(rtp_profile_store *s,const char *id){size_t i;if(!s||!valid_id(id))return RTP_PROFILE_INVALID_ARGUMENT;if(!strcmp(id,"default"))return RTP_PROFILE_PROTECTED;i=index_of(s,id);if(i==RTP_PROFILE_MAX)return RTP_PROFILE_NOT_FOUND;if(!strcmp(s->active_id,id))s->active_id[0]='\0';if(i+1<s->count)memmove(&s->profiles[i],&s->profiles[i+1],(s->count-i-1)*sizeof(s->profiles[0]));s->count--;memset(&s->profiles[s->count],0,sizeof(s->profiles[0]));return RTP_PROFILE_OK;}
int rtp_profiles_apply(rtp_profile_store *s,const char *id,rtp_preferences *p){
 size_t i,j;rtp_preferences next;const rtp_layout_profile *v=rtp_profiles_find(s,id);if(!s||!p||!v)return RTP_PROFILE_NOT_FOUND;
 next=*p;
 for(i=0;i<v->count;i++){const rtp_menu_entry *saved=&v->entries[i];size_t target=RTP_MAX_ENTRIES;for(j=0;j<next.count;j++)if(!strcmp(next.entries[j].id,saved->id)){target=j;break;}if(target==RTP_MAX_ENTRIES)continue;if(next.entries[target].kind!=saved->kind)return RTP_PROFILE_INVALID_DATA;next.entries[target].visible=saved->visible;next.entries[target].order=saved->order;}
 for(i=0;i<v->count;i++){const rtp_menu_entry *saved=&v->entries[i];size_t target=RTP_MAX_ENTRIES,parent=RTP_MAX_ENTRIES;for(j=0;j<next.count;j++){if(!strcmp(next.entries[j].id,saved->id))target=j;if(!strcmp(next.entries[j].id,saved->parent_id))parent=j;}if(target==RTP_MAX_ENTRIES)continue;if(saved->parent_id[0]&&(parent==RTP_MAX_ENTRIES||next.entries[parent].kind!=RTP_ENTRY_CATEGORY))continue;if(rtp_preferences_set_parent(&next,saved->id,saved->parent_id)!=RTP_PREFS_OK)return RTP_PROFILE_LAYOUT_ERROR;}
 *p=next;strcpy(s->active_id,id);return RTP_PROFILE_OK;
}
int rtp_profiles_save(const rtp_profile_store *s,const char *path){
 char tmp[RTP_PATH_MAX+1];FILE *f;size_t i,j;if(!s||!path||!path[0]||strlen(path)>RTP_PATH_MAX-4||s->count>RTP_PROFILE_MAX)return RTP_PROFILE_INVALID_ARGUMENT;
 if(s->active_id[0]&&!valid_id(s->active_id))return RTP_PROFILE_INVALID_DATA;
 for(i=0;i<s->count;i++){const rtp_layout_profile *p=&s->profiles[i];if(!valid_id(p->id)||!valid_name(p->name)||p->count>RTP_MAX_ENTRIES||index_of(s,p->id)!=i)return RTP_PROFILE_INVALID_DATA;}
 strcpy(tmp,path);strcat(tmp,".tmp");f=fopen(tmp,"wb");if(!f)return RTP_PROFILE_IO_ERROR;
 if(fputs(PROFILE_HEADER,f)==EOF)goto fail;if(fprintf(f,"A\t%s\n",s->active_id)<0)goto fail;
 for(i=0;i<s->count;i++){const rtp_layout_profile *p=&s->profiles[i];if(fprintf(f,"P\t%s\t%s\n",p->id,p->name)<0)goto fail;for(j=0;j<p->count;j++){const rtp_menu_entry *e=&p->entries[j];if(fprintf(f,"E\t%s\t%s\t%d\t%d\t%u\n",e->id,e->parent_id,(int)e->kind,e->visible,e->order)<0)goto fail;}}
 if(fflush(f)!=0)goto fail;if(fclose(f)!=0){remove(tmp);return RTP_PROFILE_IO_ERROR;}if(rename(tmp,path)!=0){remove(tmp);return RTP_PROFILE_IO_ERROR;}return RTP_PROFILE_OK;
fail:fclose(f);remove(tmp);return RTP_PROFILE_IO_ERROR;
}
static int number(const char *s,unsigned int *v){char *end;unsigned long n;if(!s||!s[0])return 0;errno=0;n=strtoul(s,&end,10);if(errno==ERANGE||*end||n>0xffffffffUL)return 0;*v=(unsigned int)n;return 1;}
int rtp_profiles_load(rtp_profile_store *s,const char *path){
 FILE *f;char line[LINE_MAX_PROFILE];rtp_profile_store next;rtp_layout_profile *current=NULL;int got_active=0;if(!s||!path||!path[0])return RTP_PROFILE_INVALID_ARGUMENT;f=fopen(path,"rb");if(!f)return RTP_PROFILE_IO_ERROR;rtp_profiles_init(&next);
 if(!fgets(line,sizeof(line),f)||strcmp(line,PROFILE_HEADER)){fclose(f);return RTP_PROFILE_INVALID_DATA;}
 while(fgets(line,sizeof(line),f)){char *v[6],*p;size_t n=0;unsigned int a,b,c;if(!strchr(line,'\n')&&!feof(f)){fclose(f);return RTP_PROFILE_INVALID_DATA;}line[strcspn(line,"\r\n")]=0;if(!line[0])continue;p=line;v[n++]=p;while(n<6&&(p=strchr(p,'\t'))!=NULL){*p++=0;v[n++]=p;}
  if(!strcmp(v[0],"A")){if(n!=2||got_active++|| (v[1][0]&&!valid_id(v[1]))){fclose(f);return RTP_PROFILE_INVALID_DATA;}strcpy(next.active_id,v[1]);}
  else if(!strcmp(v[0],"P")){if(n!=3||next.count>=RTP_PROFILE_MAX||!valid_id(v[1])||!valid_name(v[2])||index_of(&next,v[1])!=RTP_PROFILE_MAX){fclose(f);return RTP_PROFILE_INVALID_DATA;}current=&next.profiles[next.count++];strcpy(current->id,v[1]);strcpy(current->name,v[2]);}
  else if(!strcmp(v[0],"E")){rtp_menu_entry *e;if(n!=6||!current||current->count>=RTP_MAX_ENTRIES||!valid_id(v[1])||!valid_id(v[2])&&v[2][0]||!number(v[3],&a)||!number(v[4],&b)||!number(v[5],&c)||a<1||a>4||b>1){fclose(f);return RTP_PROFILE_INVALID_DATA;}e=&current->entries[current->count++];memset(e,0,sizeof(*e));strcpy(e->id,v[1]);strcpy(e->parent_id,v[2]);e->kind=(rtp_entry_kind)a;e->visible=(int)b;e->order=c;}
  else {fclose(f);return RTP_PROFILE_INVALID_DATA;}
 }
 if(ferror(f)){fclose(f);return RTP_PROFILE_IO_ERROR;}fclose(f);if(!got_active||(next.active_id[0]&&index_of(&next,next.active_id)==RTP_PROFILE_MAX))return RTP_PROFILE_INVALID_DATA;*s=next;return RTP_PROFILE_OK;
}
