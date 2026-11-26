#include "custom_assets.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct { int activate_calls, verify_calls, restore_calls, remove_calls; int activate_fail, verify_fail, restore_fail, remove_fail; char active_path[RTA_PATH_MAX+1]; } fake_backend;
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
 puts("Custom Assets Manager tests passed.");return 0;
}
