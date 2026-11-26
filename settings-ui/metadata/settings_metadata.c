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
  if(!m||!valid_key(e->entry_id,1)||!valid_key(e->module_id,1)||!e->title[0]||!memchr(e->title,'\0',sizeof(e->title))||!memchr(e->description,'\0',sizeof(e->description))||!memchr(e->category_id,'\0',sizeof(e->category_id))||!memchr(e->icon_key,'\0',sizeof(e->icon_key)))return RTP_META_INVALID_TEXT;
  if(e->category_id[0]){const rtp_menu_entry *cat=rtp_preferences_find(p,e->category_id);if(!cat||cat->kind!=RTP_ENTRY_CATEGORY)return RTP_META_INVALID_CATEGORY;}
  for(j=i+1;j<c->count;++j)if(strcmp(e->entry_id,c->entries[j].entry_id)==0)return RTP_META_DUPLICATE_ID;
 }return RTP_META_OK;
}
typedef struct { const char *id,*title,*description,*category,*icon,*module; unsigned int flags; rtp_entry_kind kind; unsigned int order; } default_entry;
static int register_defaults(rtp_metadata_catalogue *catalogue,rtp_preferences *preferences,const default_entry *entries,size_t count){
 size_t i;
 for(i=0;i<count;++i){
  rtp_metadata_descriptor d;
  if(rtp_preferences_register(preferences,entries[i].id,entries[i].category,entries[i].kind,1,entries[i].order)!=RTP_PREFS_OK)return RTP_META_INVALID_ARGUMENT;
  memset(&d,0,sizeof(d));d.entry_id=entries[i].id;d.title=entries[i].title;d.description=entries[i].description;d.category_id=entries[i].category;d.icon_key=entries[i].icon;d.module_id=entries[i].module;d.flags=entries[i].flags;
  if(rtp_metadata_register(catalogue,preferences,&d)!=RTP_META_OK)return RTP_META_INVALID_ARGUMENT;
 }
 return RTP_META_OK;
}
int rtp_metadata_register_appearance(rtp_metadata_catalogue *catalogue,rtp_preferences *preferences){
 static const default_entry entries[]={
  {"appearance-customization","Appearance & Customization","Customize startup presentation, supported interface elements and managed assets.","","palette","appearance",RTP_META_FLAG_NONE,RTP_ENTRY_CATEGORY,20},
  {"boot-animation","Boot Animation","Choose and manage a compatible animated startup sequence.","appearance-customization","film","appearance",RTP_META_FLAG_REQUIRES_BACKEND|RTP_META_FLAG_REQUIRES_REBOOT,RTP_ENTRY_SETTING,21},
  {"boot-splash","Boot Splash","Choose a compatible startup image and restore the original splash.","appearance-customization","image","appearance",RTP_META_FLAG_REQUIRES_BACKEND|RTP_META_FLAG_REQUIRES_REBOOT,RTP_ENTRY_SETTING,22},
  {"boot-sound","Boot Sound","Select a supported startup sound and check playback requirements.","appearance-customization","volume-2","appearance",RTP_META_FLAG_REQUIRES_BACKEND|RTP_META_FLAG_REQUIRES_REBOOT,RTP_ENTRY_SETTING,23},
  {"startup-warning","Startup Warning","Manage supported startup warning messages and presets.","appearance-customization","message-square-warning","appearance",RTP_META_FLAG_REQUIRES_BACKEND|RTP_META_FLAG_REQUIRES_REBOOT,RTP_ENTRY_SETTING,24},
  {"interface-customization","Interface Customization","Configure supported shell appearance and visual options.","appearance-customization","sliders-horizontal","appearance",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,25},
  {"custom-assets-manager","Custom Assets Manager","Import, validate, organize and restore supported startup assets.","appearance-customization","images","appearance",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,26},
  {"quick-menu","Quick Menu","Review the installed Quick Menu framework and supported extensions.","appearance-customization","layout-dashboard","quick-menu",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_CATEGORY,27},
  {"quick-menu-widgets","Quick Menu Widgets","Manage compatible widgets and their supported options.","quick-menu","grid-2x2","quick-menu",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,28},
  {"quick-menu-shortcuts","Quick Menu Shortcuts","Configure supported Quick Menu shortcuts and launch actions.","quick-menu","keyboard","quick-menu",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,29}
 };
 if(!catalogue||!preferences)return RTP_META_INVALID_ARGUMENT;
 return register_defaults(catalogue,preferences,entries,sizeof(entries)/sizeof(entries[0]));
}
int rtp_metadata_register_tai_saver(rtp_metadata_catalogue *catalogue,rtp_preferences *preferences){
 static const default_entry entries[]={
  {"tai-saver","Tai Saver","Protect ur0:tai with verified snapshots, archive management and guarded recovery.","","shield-check","tai-saver",RTP_META_FLAG_NONE,RTP_ENTRY_CATEGORY,10},
  {"tai-saver-protection","Protection","Automatic boot protection, manual backup and the latest backup result.","tai-saver","shield","tai-saver",RTP_META_FLAG_NONE,RTP_ENTRY_CATEGORY,10},
  {"tai-saver-auto-backup","Automatic Backup","Enable or disable the boot-time snapshot service. This does not edit taiHEN configuration.","tai-saver-protection","shield-check","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,10},
  {"tai-saver-backup-now","Back Up Now","Create and verify a snapshot of ur0:tai immediately.","tai-saver-protection","save","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_ACTION,20},
  {"tai-saver-last-result","Last Backup Result","Read-only status and timestamp from the most recent backup run.","tai-saver-protection","history","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND|RTP_META_FLAG_READ_ONLY,RTP_ENTRY_SETTING,30},
  {"tai-saver-policy","Automatic Backup Policy","Configure snapshot retention without rotating protected archives.","tai-saver","sliders-horizontal","tai-saver",RTP_META_FLAG_NONE,RTP_ENTRY_CATEGORY,20},
  {"tai-saver-retention","Automatic Retention","Choose how many verified Automatic snapshots to keep.","tai-saver-policy","archive","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,10},
  {"tai-saver-archives","Snapshot Archives","Browse Automatic, Known-Good, Failed and Pre-Restore snapshots.","tai-saver","folder-archive","tai-saver",RTP_META_FLAG_NONE,RTP_ENTRY_CATEGORY,30},
  {"tai-saver-automatic-archives","Automatic Snapshots","Inspect and verify snapshots managed by the retention policy.","tai-saver-archives","history","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,10},
  {"tai-saver-known-good","Known-Good Archives","Preserve configurations explicitly marked as working. Never auto-rotated.","tai-saver-archives","bookmark-check","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,20},
  {"tai-saver-failed-archives","Failed / KO Archives","Keep diagnostic snapshots separate from working configurations.","tai-saver-archives","archive-x","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,30},
  {"tai-saver-pre-restore-archives","Pre-Restore Archives","Review safety snapshots created before a restore operation.","tai-saver-archives","history-restore","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_SETTING,40},
  {"tai-saver-recovery","Compare & Recovery","Compare snapshots and restore only after complete integrity verification.","tai-saver","refresh-ccw","tai-saver",RTP_META_FLAG_NONE,RTP_ENTRY_CATEGORY,40},
  {"tai-saver-compare","Compare Snapshots","Compare file paths and SHA-256 hashes between two verified snapshots.","tai-saver-recovery","git-compare","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_ACTION,10},
  {"tai-saver-verify","Verify Snapshot","Verify the manifest, exact file set, file sizes and SHA-256 hashes.","tai-saver-recovery","file-check","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_ACTION,20},
  {"tai-saver-restore","Restore Snapshot","Restore a verified snapshot after creating a Pre-Restore safety snapshot.","tai-saver-recovery","history","tai-saver",RTP_META_FLAG_REQUIRES_BACKEND,RTP_ENTRY_ACTION,30}
 };
 if(!catalogue||!preferences)return RTP_META_INVALID_ARGUMENT;
 return register_defaults(catalogue,preferences,entries,sizeof(entries)/sizeof(entries[0]));
}
