#include "custom_warning_adapter.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
 int active, dir_ux0, dir_ur0, corrupt_write, backup_exists;
 char names_ux0[RTCW_MAX_PRESETS][RTCW_PRESET_NAME_MAX+1];
 char names_ur0[RTCW_MAX_PRESETS][RTCW_PRESET_NAME_MAX+1];
 size_t count_ux0, count_ur0;
 struct { char path[RTCW_PATH_MAX+1]; unsigned char bytes[RTCW_MAX_FILE_BYTES]; size_t size; int exists; } files[8];
 char backup_path[RTCW_PATH_MAX+1]; unsigned char backup[RTCW_MAX_FILE_BYTES]; size_t backup_size;
} fake_fs;
static int find_file(fake_fs *f,const char *path){size_t i;for(i=0;i<8;i++)if(f->files[i].exists&&strcmp(f->files[i].path,path)==0)return (int)i;return -1;}
static int alloc_file(fake_fs *f,const char *path){size_t i;int x=find_file(f,path);if(x>=0)return x;for(i=0;i<8;i++)if(!f->files[i].exists){strcpy(f->files[i].path,path);f->files[i].exists=1;return (int)i;}return -1;}
static int active(void *ctx){return ((fake_fs*)ctx)->active;}
static int list_dir(void *ctx,const char *path,char names[RTCW_MAX_PRESETS][RTCW_PRESET_NAME_MAX+1],size_t cap,size_t *count){
 fake_fs *f=(fake_fs*)ctx;size_t i;
 if(strcmp(path,"ux0:tai/custom_warning")==0){if(!f->dir_ux0)return 0;*count=f->count_ux0;if(*count>cap)return -1;for(i=0;i<*count;i++)strcpy(names[i],f->names_ux0[i]);return 1;}
 if(strcmp(path,"ur0:tai/custom_warning")==0){if(!f->dir_ur0)return 0;*count=f->count_ur0;if(*count>cap)return -1;for(i=0;i<*count;i++)strcpy(names[i],f->names_ur0[i]);return 1;}
 return -1;
}
static int read_file(void *ctx,const char *path,unsigned char *buf,size_t cap,size_t *size){
 fake_fs *f=(fake_fs*)ctx;int i=find_file(f,path);if(i<0){*size=0;return 0;}if(f->files[i].size>cap)return -1;memcpy(buf,f->files[i].bytes,f->files[i].size);*size=f->files[i].size;return 1;
}
static int backup_file(void *ctx,const char *path){
 fake_fs *f=(fake_fs*)ctx;int i=find_file(f,path);strcpy(f->backup_path,path);f->backup_exists=i>=0;
 if(i>=0){f->backup_size=f->files[i].size;memcpy(f->backup,f->files[i].bytes,f->backup_size);}else f->backup_size=0;
 return 0;
}
static int write_atomic(void *ctx,const char *path,const unsigned char *bytes,size_t size){
 fake_fs *f=(fake_fs*)ctx;int i=alloc_file(f,path);if(i<0||size>RTCW_MAX_FILE_BYTES)return -1;
 memcpy(f->files[i].bytes,bytes,size);f->files[i].size=size;
 if(f->corrupt_write&&size>2)f->files[i].bytes[size-1]^=1;
 return 0;
}
static int restore_backup(void *ctx,const char *path){
 fake_fs *f=(fake_fs*)ctx;int i=find_file(f,path);if(strcmp(path,f->backup_path)!=0)return -1;
 if(!f->backup_exists){if(i>=0)f->files[i].exists=0;return 0;}
 if(i<0)i=alloc_file(f,path);if(i<0)return -1;
 memcpy(f->files[i].bytes,f->backup,f->backup_size);f->files[i].size=f->backup_size;return 0;
}
static void put(fake_fs *f,const char *path,const char *text){
 unsigned char bytes[RTCW_MAX_FILE_BYTES];size_t n=0;int i;
 assert(rtcw_encode_utf8(text,bytes,sizeof(bytes),&n)==RTCW_OK);
 i=alloc_file(f,path);assert(i>=0);memcpy(f->files[i].bytes,bytes,n);f->files[i].size=n;
}
int main(void){
 fake_fs fs;rtcw_backend b;rtcw_state state;char text[RTCW_MAX_TEXT_BYTES+1];size_t text_size=0;
 unsigned char encoded[RTCW_MAX_FILE_BYTES];size_t encoded_size=0;
 memset(&fs,0,sizeof(fs));memset(&b,0,sizeof(b));fs.active=1;
 b.context=&fs;b.plugin_active=active;b.list_directory=list_dir;b.read_file=read_file;b.backup_file=backup_file;b.write_atomic=write_atomic;b.restore_backup=restore_backup;
 assert(rtcw_encode_utf8("Hello 🌍",encoded,sizeof(encoded),&encoded_size)==RTCW_OK);
 assert(encoded[0]==0xFF&&encoded[1]==0xFE);
 assert(rtcw_decode_utf16le(encoded,encoded_size,text,sizeof(text),&text_size)==RTCW_OK);
 assert(strcmp(text,"Hello 🌍")==0);
 {
  const char invalid_utf8[]={(char)0xC0,(char)0x80,0};
  assert(rtcw_encode_utf8(invalid_utf8,encoded,sizeof(encoded),&encoded_size)==RTCW_ERR_INVALID_TEXT);
  const unsigned char no_bom[]={0x41,0x00};
  const unsigned char odd[]={0xFF,0xFE,0x41};
  const unsigned char unpaired[]={0xFF,0xFE,0x00,0xD8};
  assert(rtcw_validate_utf16le(no_bom,sizeof(no_bom))==RTCW_ERR_INVALID_ENCODING);
  assert(rtcw_validate_utf16le(odd,sizeof(odd))==RTCW_ERR_INVALID_ENCODING);
  assert(rtcw_validate_utf16le(unpaired,sizeof(unpaired))==RTCW_ERR_INVALID_ENCODING);
 }
 put(&fs,"ux0:tai/custom_warning.txt","Original warning");
 put(&fs,"ur0:tai/custom_warning.txt","Lower priority");
 assert(rtcw_read_state(&b,&state)==RTCW_OK);
 assert(state.mode==RTCW_MODE_FIXED_FILE&&strcmp(state.mount,"ux0")==0);
 assert(strcmp(state.current_text,"Original warning")==0);
 assert(rtcw_write_message(&b,"New warning 🚀")==RTCW_OK);
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&strcmp(state.current_text,"New warning 🚀")==0);
 fs.corrupt_write=1;
 assert(rtcw_write_message(&b,"Corrupt me")==RTCW_ERR_INTEGRITY);
 fs.corrupt_write=0;
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&strcmp(state.current_text,"New warning 🚀")==0);
 assert(rtcw_restore_original(&b,"ux0:tai/custom_warning.txt")==RTCW_OK);
 assert(rtcw_restore_original(&b,"ux0:tai/custom_warning/../bad")==RTCW_ERR_ARGUMENT);
 fs.dir_ux0=1;fs.count_ux0=2;strcpy(fs.names_ux0[0],"hello.txt");strcpy(fs.names_ux0[1],"italian.txt");
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&state.mode==RTCW_MODE_PRESET_ROTATION&&state.preset_count==2);
 assert(rtcw_write_message(&b,"not used")==RTCW_ERR_MODE);
 put(&fs,"ux0:tai/custom_warning/hello.txt","Hello from preset");
 assert(rtcw_read_preset(&b,"ux0","hello.txt",text,sizeof(text),&text_size)==RTCW_OK);
 assert(strcmp(text,"Hello from preset")==0);
 assert(rtcw_write_preset(&b,"ux0","hello.txt","Edited preset",0)==RTCW_OK);
 assert(rtcw_read_preset(&b,"ux0","hello.txt",text,sizeof(text),&text_size)==RTCW_OK&&strcmp(text,"Edited preset")==0);
 assert(rtcw_write_preset(&b,"ux0","new.txt","New preset",1)==RTCW_OK);
 assert(rtcw_write_preset(&b,"ux0","../escape","bad",1)==RTCW_ERR_ARGUMENT);
 assert(rtcw_write_preset(&b,"ux0","hello.txt","duplicate",1)==RTCW_ERR_MODE);
 fs.dir_ux0=1;fs.count_ux0=0;fs.dir_ur0=1;fs.count_ur0=1;strcpy(fs.names_ur0[0],"fallback.txt");
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&state.mode==RTCW_MODE_FIXED_FILE&&strcmp(state.mount,"ux0")==0);
 {int fixed=find_file(&fs,"ux0:tai/custom_warning.txt");assert(fixed>=0);fs.files[fixed].exists=0;}
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&state.mode==RTCW_MODE_PRESET_ROTATION&&strcmp(state.mount,"ur0")==0);
 fs.dir_ux0=0;fs.dir_ur0=0;fs.active=0;assert(rtcw_read_state(&b,&state)==RTCW_ERR_PLUGIN_DISABLED);\n assert(rtcw_write_preset(&b,"ux0","hello.txt","blocked",0)==RTCW_ERR_PLUGIN_DISABLED);
 fs.active=1;fs.dir_ux0=0;fs.count_ux0=0;
 memset(fs.files,0,sizeof(fs.files));
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&state.mode==RTCW_MODE_DEFAULT);
 assert(rtcw_write_message(&b,"Created from default")==RTCW_OK);
 assert(rtcw_read_state(&b,&state)==RTCW_OK&&strcmp(state.current_text,"Created from default")==0);
 assert(strcmp(rtcw_mode_name(RTCW_MODE_PRESET_ROTATION),"preset-rotation")==0);
 puts("Custom Warning adapter tests passed.");return 0;
}
