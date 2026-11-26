#include "settings_metadata.h"
#include <string.h>
static int copy_text(char *dst,size_t cap,const char *src,int required){size_t n;if(!src)src="";n=strlen(src);if((required&&!n)||n>=cap)return 0;memcpy(dst,src,n+1);return 1;}
static int valid_key(const char *s,int required){size_t i;if(!s)s="";if(required&&!s[0])return 0;for(i=0;s[i];++i){char c=s[i];if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.'))return 0;}return 1;}
void rtp_metadata_init(rtp_metadata_catalogue *c){if(c)memset(c,0,sizeof(*c));}
const rtp_metadata_entry *rtp_metadata_find(const rtp_metadata_catalogue *c,const char *id){size_t i;if(!c||!id||c->count>RTP_META_MAX_ENTRIES)return NULL;for(i=0;i<c->count;++i)if(strcmp(c->entries[i].entry_id,id)==0)return &c->entries[i];return NULL;}
int rtp_metadata_register(rtp_metadata_catalogue *c,const rtp_preferences *p,const rtp_metadata_descriptor *d){
 rtp_metadata_entry e;const rtp_menu_entry *menu;const char *category;
 if(!c||!p||!d||!d->entry_id||!d->title||!d->description||!d->module_id)return RTP_META_INVALID_ARGUMENT;
 if(c->count>RTP_META_MAX_ENTRIES)return RTP_META_INVALID_ARGUMENT;
 if(c->count>=RTP_META_MAX_ENTRIES)return RTP_META_CAPACITY;
 if(!valid_key(d->entry_id,1)||!valid_key(d->module_id,1)||!valid_key(d->category_id,0)||!valid_key(d->icon_key,0))return RTP_META_INVALID_ARGUMENT;
 if(rtp_metadata_find(c,d->entry_id))return RTP_META_DUPLICATE_ID;
 menu=rtp_preferences_find(p,d->entry_id);if(!menu)return RTP_META_NOT_FOUND;
 category=d->category_id?d->category_id:"";
 if(category[0]){const rtp_menu_entry *cat=rtp_preferences_find(p,category);if(!cat||cat->kind!=RTP_ENTRY_CATEGORY)return RTP_META_INVALID_CATEGORY;}
 memset(&e,0,sizeof(e));
 if(!copy_text(e.entry_id,sizeof(e.entry_id),d->entry_id,1)||!copy_text(e.title,sizeof(e.title),d->title,1)||!copy_text(e.description,sizeof(e.description),d->description,0)||!copy_text(e.category_id,sizeof(e.category_id),category,0)||!copy_text(e.icon_key,sizeof(e.icon_key),d->icon_key,0)||!copy_text(e.module_id,sizeof(e.module_id),d->module_id,1))return RTP_META_INVALID_TEXT;
 e.flags=d->flags;c->entries[c->count++]=e;return RTP_META_OK;
}
int rtp_metadata_validate(const rtp_metadata_catalogue *c,const rtp_preferences *p){
 size_t i,j;if(!c||!p||c->count>RTP_META_MAX_ENTRIES)return RTP_META_INVALID_ARGUMENT;
 for(i=0;i<c->count;++i){const rtp_metadata_entry *e=&c->entries[i];const rtp_menu_entry *m=rtp_preferences_find(p,e->entry_id);
  if(!m||!valid_key(e->entry_id,1)||!valid_key(e->module_id,1)||!e->title[0]||!memchr(e->title,'\\0',sizeof(e->title))||!memchr(e->description,'\\0',sizeof(e->description))||!memchr(e->category_id,'\\0',sizeof(e->category_id))||!memchr(e->icon_key,'\\0',sizeof(e->icon_key)))return RTP_META_INVALID_TEXT;
  if(e->category_id[0]){const rtp_menu_entry *cat=rtp_preferences_find(p,e->category_id);if(!cat||cat->kind!=RTP_ENTRY_CATEGORY)return RTP_META_INVALID_CATEGORY;}
  for(j=i+1;j<c->count;++j)if(strcmp(e->entry_id,c->entries[j].entry_id)==0)return RTP_META_DUPLICATE_ID;
 }return RTP_META_OK;
}
