#include "custom_assets.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct { int activate_calls, verify_calls, restore_calls, remove_calls; int activate_fail, verify_fail, restore_fail, remove_fail; char active_path[RTA_PATH_MAX+1]; } fake_backend;
typedef struct { char path[8][RTA_PATH_MAX+1]; unsigned char bytes[8][4096]; size_t size[8]; int used[8]; int fail_copy,fail_commit,corrupt_copy; } asset_fs;
static int fs_find(asset_fs *f,const char *path){int i;for(i=0;i<8;i++)if(f->used[i]&&strcmp(f->path[i],path)==0)return i;return -1;}
static int fs_alloc(asset_fs *f,const char *path){int i=fs_find(f,path),j;if(i>=0)return i;for(j=0;j<8;j++)if(!f->used[j]){f->used[j]=1;strcpy(f->path[j],path);return j;}return -1;}
static int asset_read(void *ctx,const char *path,uint64_t offset,unsigned char *buffer,size_t cap,size_t *readn){
 asset_fs *f=(asset_fs*)ctx;int i=fs_find(f,path);size_t n;*readn=0;if(i<0)return -1;if(offset>=f->size[i])return 0;
 n=f->size[i]-(size_t)offset;if(n>cap)n=cap;memcpy(buffer,f->bytes[i]+(size_t)offset,n);*readn=n;return 1;
}
static int fs_copy(asset_fs *f,const char *src,const char *dst){int a=fs_find(f,src),b;if(a<0)return -1;b=fs_alloc(f,dst);if(b<0)return -1;f->size[b]=f->size[a];memcpy(f->bytes[b],f->bytes[a],f->size[a]);if(f->corrupt_copy&&f->size[b])f->bytes[b][0]^=1;return 0;}
static int copy_stage(void *ctx,const char *src,const char *stage){asset_fs *f=(asset_fs*)ctx;return f->fail_copy?-1:fs_copy(f,src,stage);}
static int commit_stage(void *ctx,const char *stage,const char *dest){asset_fs *f=(asset_fs*)ctx;int i=fs_find(f,stage);if(f->fail_commit||i<0)return -1;if(fs_copy(f,stage,dest)!=0)return -1;f->used[i]=0;return 0;}
static int fs_remove(void *ctx,const char *path){asset_fs *f=(asset_fs*)ctx;int i=fs_find(f,path);if(i<0)return -1;f->used[i]=0;return 0;}
static int fs_copy_atomic(void *ctx,const char *src,const char *dest){return fs_copy((asset_fs*)ctx,src,dest);}
static int activate(void *ctx,rta_asset_type type,const char *path){fake_backend *f=(fake_backend*)ctx;(void)type;f->activate_calls++;if(f->activate_fail)return -1;strcpy(f->active_path,path);return 0;}
static int verify_active(void *ctx,rta_asset_type type,const char *path){fake_backend *f=(fake_backend*)ctx;(void)type;(void)path;f->verify_calls++;return f->verify_fail?-1:0;}
static int restore(void *ctx,rta_asset_type type,const char *path){fake_backend *f=(fake_backend*)ctx;(void)type;f->restore_calls++;if(f->restore_fail)return -1;if(path)strcpy(f->active_path,path);else f->active_path[0]='\0';return 0;}
static int remove_asset(void *ctx,const char *path){fake_backend *f=(fake_backend*)ctx;(void)path;f->remove_calls++;return f->remove_fail?-1:0;}
typedef struct { const unsigned char *data; size_t length; int fail; } memory_file;
static int memory_read(void *ctx,const char *path,uint64_t offset,unsigned char *buffer,size_t capacity,size_t *bytes_read){
 memory_file *m=(memory_file*)ctx;size_t n;(void)path;*bytes_read=0;if(m->fail)return -1;
 if(offset>=m->length)return 0;n=m->length-(size_t)offset;if(n>capacity)n=capacity;
 memcpy(buffer,m->data+(size_t)offset,n);*bytes_read=n;return 1;
}
#define HASH "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
#define OBS(ID,TITLE,PATH,TYPE,STATE,MANAGED,COMPAT,VERIFIED,PROTECTED) {ID,TITLE,PATH,"rt-settings","test-import", ((TYPE)==RTA_TYPE_BOOT_SOUND?"wav":(TYPE)==RTA_TYPE_STARTUP_WARNING?"utf-16le":(TYPE)==RTA_TYPE_BOOT_ANIMATION?"rcf":"png"), HASH,TYPE,STATE,4096,960,544,500,48000,2,MANAGED,COMPAT,VERIFIED,PROTECTED}
int main(void){
 rta_asset_observation obs[]={
  OBS("splash-default","Default Splash","ur0:data/rt/assets/splash-default.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_ACTIVE,1,1,1,1),
  OBS("splash-neon","Neon Splash","ur0:data/rt/assets/splash-neon.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_AVAILABLE,1,1,1,0),
  OBS("splash-user","User Splash","ux0:downloads/user.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_AVAILABLE,0,1,1,0),
  OBS("splash-backup","Previous Splash","ur0:data/rt/assets/splash-backup.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_BACKUP,1,1,1,1)
 };
 rta_asset_registry reg; fake_backend fake={0}; rta_asset_backend backend={&fake,activate,verify_active,restore,remove_asset};
 assert(rta_asset_registry_build(obs,4,&reg)==RTA_OK);
 assert(reg.count==4&&reg.active_count==1&&reg.backup_count==1);
 assert(rta_asset_activate(&reg,"splash-neon",&backend)==RTA_OK);
 assert(strcmp(fake.active_path,obs[1].path)==0&&reg.assets[0].state==RTA_STATE_AVAILABLE&&reg.assets[1].state==RTA_STATE_ACTIVE);
 fake.verify_fail=1;
 assert(rta_asset_activate(&reg,"splash-default",&backend)==RTA_ERR_BACKEND);
 assert(strcmp(fake.active_path,obs[1].path)==0&&reg.assets[1].state==RTA_STATE_ACTIVE);
 fake.restore_fail=1;
 assert(rta_asset_activate(&reg,"splash-default",&backend)==RTA_ERR_ROLLBACK);
 fake.restore_fail=0;fake.verify_fail=0;
 assert(rta_asset_activate(&reg,"splash-user",&backend)==RTA_ERR_UNSAFE);
 assert(rta_asset_remove(&reg,"splash-default",&backend)==RTA_ERR_PROTECTED);
 assert(rta_asset_remove(&reg,"splash-backup",&backend)==RTA_ERR_PROTECTED);
 assert(rta_asset_remove(&reg,"splash-neon",&backend)==RTA_ERR_UNSAFE);
 assert(rta_asset_remove(&reg,"splash-user",&backend)==RTA_ERR_UNSAFE);
 assert(rta_asset_remove(&reg,"missing",&backend)==RTA_ERR_NOT_FOUND);
 {
  rta_asset_observation bad=OBS("../escape","bad","ur0:data/x.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_AVAILABLE,1,1,1,0);
  assert(rta_asset_validate(&bad)==RTA_ERR_INVALID_ASSET);
  bad= (rta_asset_observation)OBS("badpath","bad","ur0:data/../x.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_AVAILABLE,1,1,1,0);
  assert(rta_asset_validate(&bad)==RTA_ERR_INVALID_ASSET);
  bad= (rta_asset_observation)OBS("big-sound","bad","ur0:data/a.wav",RTA_TYPE_BOOT_SOUND,RTA_STATE_AVAILABLE,1,1,1,0);
  bad.size_bytes=1024ULL*1024ULL+1;bad.duration_ms=500;bad.sample_rate_hz=48000;bad.channels=2;
  assert(rta_asset_validate(&bad)==RTA_ERR_INVALID_ASSET);
  bad= (rta_asset_observation)OBS("no-hash","bad","ur0:data/x.png",RTA_TYPE_BOOT_SPLASH,RTA_STATE_AVAILABLE,1,1,0,0);
  bad.sha256="";bad.hash_verified=0;
  assert(rta_asset_validate(&bad)==RTA_OK);
  bad.format="mp3";assert(rta_asset_validate(&bad)==RTA_ERR_INVALID_ASSET);
  bad= (rta_asset_observation)OBS("bad-audio","bad","ur0:data/a.wav",RTA_TYPE_BOOT_SOUND,RTA_STATE_AVAILABLE,1,1,1,0);
  bad.sample_rate_hz=96000;assert(rta_asset_validate(&bad)==RTA_ERR_INVALID_ASSET);
  bad.sample_rate_hz=48000;bad.channels=3;assert(rta_asset_validate(&bad)==RTA_ERR_INVALID_ASSET);
 }
 {
  rta_asset_observation dup[]={obs[0],obs[0]};
  assert(rta_asset_registry_build(dup,2,&reg)==RTA_ERR_DUPLICATE_ID);
  dup[1]=obs[1];dup[1].id="other";dup[1].path=obs[0].path;
  assert(rta_asset_registry_build(dup,2,&reg)==RTA_ERR_DUPLICATE_PATH);
 }
 fake.remove_fail=1;
 assert(rta_asset_registry_build(obs,4,&reg)==RTA_OK);
 reg.assets[2].managed=1;reg.assets[2].state=RTA_STATE_AVAILABLE;reg.assets[2].protected_asset=0;
 assert(rta_asset_remove(&reg,"splash-user",&backend)==RTA_ERR_BACKEND&&reg.count==4);
 fake.remove_fail=0;
 assert(rta_asset_remove(&reg,"splash-user",&backend)==RTA_OK&&reg.count==3);
 assert(strcmp(rta_asset_type_name(RTA_TYPE_BOOT_SOUND),"boot-sound")==0);
 assert(strcmp(reg.assets[0].source,"test-import")==0);
 {
  static const unsigned char abc[]={'a','b','c'};
  memory_file mem={abc,sizeof(abc),0};char digest[65];uint64_t size=0;
  assert(rta_asset_hash_reader(memory_read,&mem,"ur0:data/test.bin",digest,&size)==RTA_OK);
  assert(size==3&&strcmp(digest,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")==0);
  assert(rta_asset_verify_reader(memory_read,&mem,"ur0:data/test.bin",digest,3)==RTA_OK);
  assert(rta_asset_verify_reader(memory_read,&mem,"ur0:data/test.bin",HASH,3)==RTA_ERR_INTEGRITY);
  assert(rta_asset_verify_reader(memory_read,&mem,"ur0:data/test.bin",digest,4)==RTA_ERR_INTEGRITY);
  mem.fail=1;assert(rta_asset_hash_reader(memory_read,&mem,"ur0:data/test.bin",digest,&size)==RTA_ERR_BACKEND);
  assert(rta_asset_hash_reader(memory_read,&mem,"host:/bad",digest,&size)==RTA_ERR_ARGUMENT);
 }
 {
  asset_fs fs; rta_asset_file_backend fb; rta_asset_registry imported; char id[RTA_ID_MAX+1];
  static const unsigned char abc[]={'a','b','c'};
  rta_asset_import_spec spec;
  memset(&fs,0,sizeof(fs));memset(&fb,0,sizeof(fb));memset(&imported,0,sizeof(imported));
  fb.context=&fs;fb.copy_to_staging=copy_stage;fb.commit_staged=commit_stage;fb.remove_path=fs_remove;fb.read_file=asset_read;fb.copy_atomic=fs_copy_atomic;
  {int source=fs_alloc(&fs,"ux0:downloads/splash.png");assert(source>=0);memcpy(fs.bytes[source],abc,3);fs.size[source]=3;}
  memset(&spec,0,sizeof(spec));spec.id="imported-splash";spec.title="Imported Splash";spec.source_path="ux0:downloads/splash.png";spec.owner="rt-settings";spec.format="png";spec.expected_sha256="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";spec.type=RTA_TYPE_BOOT_SPLASH;spec.width=960;spec.height=544;spec.compatible=1;
  assert(rta_asset_import(&imported,&spec,&fb,id)==RTA_OK);
  assert(strcmp(id,"imported-splash")==0&&imported.count==1&&imported.assets[0].managed&&imported.assets[0].hash_verified);
  assert(strcmp(imported.assets[0].source,spec.source_path)==0);
  assert(rta_asset_export(&imported,id,"ux0:exports/splash.png",&fb)==RTA_OK);
  assert(fs_find(&fs,"ux0:exports/splash.png")>=0);
  assert(rta_asset_import(&imported,&spec,&fb,id)==RTA_ERR_DUPLICATE_ID);
  spec.id="bad-integrity";spec.expected_sha256=HASH;
  assert(rta_asset_import(&imported,&spec,&fb,id)==RTA_ERR_INTEGRITY&&imported.count==1);
  assert(fs_find(&fs,"ur0:data/rt/assets/.bad-integrity.stage")<0);
  spec.id="failed-copy";spec.expected_sha256=NULL;fs.fail_copy=1;
  assert(rta_asset_import(&imported,&spec,&fb,id)==RTA_ERR_BACKEND&&imported.count==1);
  fs.fail_copy=0;fs.fail_commit=1;spec.id="failed-commit";
  assert(rta_asset_import(&imported,&spec,&fb,id)==RTA_ERR_BACKEND&&imported.count==1);
  fs.fail_commit=0;spec.id="verified-export";assert(rta_asset_import(&imported,&spec,&fb,id)==RTA_OK);
  fs.corrupt_copy=1;
  assert(rta_asset_export(&imported,id,"ux0:exports/corrupt.png",&fb)==RTA_ERR_INTEGRITY);
  assert(fs_find(&fs,"ux0:exports/corrupt.png")<0);
 }
 puts("Custom Assets Manager tests passed.");return 0;
}
