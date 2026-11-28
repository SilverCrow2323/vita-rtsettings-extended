#include "repatch_manager.h"
#include <stdio.h>
#include <string.h>
static int copy_text(char *d,size_t cap,const char *s){
 size_t n;if(!s)return 0;n=strlen(s);if(!n||n>=cap)return 0;memcpy(d,s,n+1);return 1;
}
static int safe_folder(const char *s){
 size_t i,n;if(!s)return 0;n=strlen(s);if(!n||n>RTP_REPATCH_PATH_MAX)return 0;
 if(!strcmp(s,".")||!strcmp(s,".."))return 0;
 for(i=0;i<n;++i)if(s[i]=='/'||s[i]=='\\'||s[i]==':'||s[i]<32)return 0;
 return 1;
}
static int disabled_name(const char *s){size_t n=strlen(s);return n>=3&&strcmp(s+n-3,"DIS")==0;}
static int make_path(char *out,size_t cap,const char *mount,const char *folder){
 int n;if(!mount||!folder)return 0;n=snprintf(out,cap,"%s/%s",mount,folder);
 return n>0&&(size_t)n<cap;
}
int rtp_repatch_snapshot_build(const rtp_repatch_observation *items,size_t count,
                               rtp_repatch_snapshot *out){
 size_t i,j; if(!out||(!items&&count))return RTP_REPATCH_INVALID_ARGUMENT;
 if(count>RTP_REPATCH_MAX_ENTRIES)return RTP_REPATCH_CAPACITY;
 memset(out,0,sizeof(*out));
 for(i=0;i<count;++i){
  rtp_repatch_entry *e=&out->entries[i];const char *name=items[i].folder_name;
  size_t n; if(!safe_folder(name)|| (items[i].kind!=RTP_REPATCH_GAME&&items[i].kind!=RTP_REPATCH_DLC))
   return RTP_REPATCH_INVALID_ENTRY;
  n=strlen(name);e->state=disabled_name(name)?RTP_REPATCH_DISABLED:RTP_REPATCH_ACTIVE;
  if(e->state==RTP_REPATCH_DISABLED)n-=3;
  if(!n||n>RTP_REPATCH_ID_MAX)return RTP_REPATCH_INVALID_ENTRY;
  memcpy(e->id,name,n);e->id[n]='\0';
  if(!copy_text(e->folder_name,sizeof(e->folder_name),name))return RTP_REPATCH_INVALID_ENTRY;
  if(items[i].title&&!copy_text(e->title,sizeof(e->title),items[i].title))return RTP_REPATCH_INVALID_ENTRY;
  e->kind=items[i].kind;
  for(j=0;j<i;++j)if(e->kind==out->entries[j].kind&&!strcmp(e->id,out->entries[j].id))
   return RTP_REPATCH_DUPLICATE_ID;
  out->count++;
 }
 return RTP_REPATCH_OK;
}
int rtp_repatch_set_state(rtp_repatch_snapshot *s,const char *id,rtp_repatch_state desired,
                          rtp_repatch_kind kind,const char *mount,const rtp_repatch_fs *fs){
 size_t i;rtp_repatch_entry *e=NULL;char src[256],dst[256],target[256];int n;
 if(!s||!id||!mount||!fs||!fs->exists||!fs->rename_no_replace||
    (desired!=RTP_REPATCH_ACTIVE&&desired!=RTP_REPATCH_DISABLED))return RTP_REPATCH_INVALID_ARGUMENT;
 for(i=0;i<s->count;++i)if(s->entries[i].kind==kind&&!strcmp(s->entries[i].id,id)){e=&s->entries[i];break;}
 if(!e)return RTP_REPATCH_NOT_FOUND;
 if(e->state==desired)return RTP_REPATCH_ALREADY_STATE;
 if(!safe_folder(e->folder_name))return RTP_REPATCH_INVALID_ENTRY;
 if(!make_path(src,sizeof(src),mount,e->folder_name))return RTP_REPATCH_INVALID_ARGUMENT;
 if(desired==RTP_REPATCH_DISABLED)n=snprintf(target,sizeof(target),"%sDIS",e->folder_name);
 else {size_t len=strlen(e->folder_name);if(len<3||strcmp(e->folder_name+len-3,"DIS"))return RTP_REPATCH_INVALID_ENTRY;
  memcpy(target,e->folder_name,len-3);target[len-3]='\0';n=(int)(len-3);}
 if(n<0||(size_t)n>=sizeof(target)||!safe_folder(target)||!make_path(dst,sizeof(dst),mount,target))
  return RTP_REPATCH_INVALID_ARGUMENT;
 {int present=fs->exists(fs->context,dst);if(present<0)return RTP_REPATCH_BACKEND_ERROR;if(present>0)return RTP_REPATCH_DESTINATION_EXISTS;}
 if(fs->exists(fs->context,src)!=1)return RTP_REPATCH_BACKEND_ERROR;
 if(fs->rename_no_replace(fs->context,src,dst)!=0)return RTP_REPATCH_BACKEND_ERROR;
 if(fs->exists(fs->context,src)!=0||fs->exists(fs->context,dst)!=1)return RTP_REPATCH_VERIFY_FAILED;
 if(!copy_text(e->folder_name,sizeof(e->folder_name),target))return RTP_REPATCH_VERIFY_FAILED;
 e->state=desired;return RTP_REPATCH_OK;
}
