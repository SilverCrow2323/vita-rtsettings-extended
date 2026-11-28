#include "native_settings_installer.h"
#include "tai_saver.h"
#include "tai_config.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <string.h>

#define RT_PLUGIN_SOURCE "app0:rtsettings.suprx"
#define RT_PLUGIN_TARGET "ur0:tai/rtsettings.suprx"
#define RT_CONFIG_PATH "ur0:tai/config.txt"
#define RT_PLUGIN_TEMP "ur0:tai/rtsettings.suprx.rt-tmp"
#define RT_CONFIG_TEMP "ur0:tai/config.txt.rt-tmp"
#define RT_CONFIG_OLD "ur0:tai/config.txt.rt-old"
#define RT_ARCHIVES "ur0:data/RtSettingsExtended/TaiSaver"
#define RT_INSTALLER_DIR "ur0:data/RtSettingsExtended/Installer"
#define RT_PLUGIN_LINE "ur0:tai/rtsettings.suprx"
#define RT_CONFIG_MAX 65536
#define RT_CONFIG_OUTPUT_MAX (RT_CONFIG_MAX + 128)
#define RT_PLUGIN_MAX (256 * 1024)

static char g_config[RT_CONFIG_MAX];
static char g_config_new[RT_CONFIG_OUTPUT_MAX];
static char g_config_verify[RT_CONFIG_OUTPUT_MAX];
static unsigned char g_payload[RT_PLUGIN_MAX];
static unsigned char g_file_verify[RT_PLUGIN_MAX];

static void set_message(char *out,size_t cap,const char *message) {
    if(!out||!cap)return;
    snprintf(out,cap,"%s",message?message:"");
}
static int path_stat(const char *path,SceIoStat *st) {
    memset(st,0,sizeof(*st));
    return sceIoGetstat(path,st);
}
static int path_exists(const char *path) {
    SceIoStat st;return path_stat(path,&st)>=0;
}
static int ensure_dir(const char *path) {
    SceIoStat st;
    if(path_stat(path,&st)>=0)return SCE_S_ISDIR(st.st_mode)?0:-1;
    if(sceIoMkdir(path,0777)<0 && path_stat(path,&st)<0)return -1;
    return path_stat(path,&st)>=0&&SCE_S_ISDIR(st.st_mode)?0:-1;
}
static int ensure_installer_dir(void) {
    if(ensure_dir("ur0:data/RtSettingsExtended")<0)return -1;
    return ensure_dir(RT_INSTALLER_DIR);
}
static int read_file(const char *path,void *buffer,size_t capacity,size_t *length) {
    SceIoStat st;SceUID fd;size_t total=0;int n;
    if(!buffer||!length||path_stat(path,&st)<0||st.st_size<0||(uint64_t)st.st_size>capacity)return -1;
    fd=sceIoOpen(path,SCE_O_RDONLY,0);if(fd<0)return -1;
    while(total<(size_t)st.st_size) {
        n=sceIoRead(fd,(char *)buffer+total,(SceSize)((size_t)st.st_size-total));
        if(n<=0){sceIoClose(fd);return -1;}total+=(size_t)n;
    }
    { unsigned char extra; n=sceIoRead(fd,&extra,1); if(n!=0){sceIoClose(fd);return -1;} }
    if(sceIoClose(fd)<0)return -1;
    *length=total;return 0;
}
static int write_new_file(const char *path,const void *data,size_t length) {
    SceUID fd;size_t total=0;int n,ok=1;
    fd=sceIoOpen(path,SCE_O_WRONLY|SCE_O_CREAT|SCE_O_EXCL,0777);if(fd<0)return -1;
    while(total<length) {
        n=sceIoWrite(fd,(const char *)data+total,(SceSize)(length-total));
        if(n<=0){ok=0;break;}total+=(size_t)n;
    }
    if(ok&&sceIoSyncByFd(fd,0)<0)ok=0;
    if(sceIoClose(fd)<0)ok=0;
    if(!ok){sceIoRemove(path);return -1;}
    return 0;
}
static int verify_file(const char *path,const void *expected,size_t expected_length,
                      void *scratch,size_t scratch_capacity) {
    size_t actual=0;
    if(read_file(path,scratch,scratch_capacity,&actual)<0||actual!=expected_length)return -1;
    return memcmp(scratch,expected,expected_length)==0?0:-1;
}
static int read_payload(size_t *length) {
    SceIoStat st;SceUID fd;size_t total=0;int n;
    if(path_stat(RT_PLUGIN_SOURCE,&st)<0||st.st_size<=0||(uint64_t)st.st_size>sizeof(g_payload))return -1;
    fd=sceIoOpen(RT_PLUGIN_SOURCE,SCE_O_RDONLY,0);if(fd<0)return -1;
    while(total<(size_t)st.st_size) {
        n=sceIoRead(fd,g_payload+total,(SceSize)((size_t)st.st_size-total));
        if(n<=0){sceIoClose(fd);return -1;}total+=(size_t)n;
    }
    { unsigned char extra; n=sceIoRead(fd,&extra,1); if(n!=0){sceIoClose(fd);return -1;} }
    if(sceIoClose(fd)<0)return -1;
    *length=total;return 0;
}
static int write_config_backup(const char *path,const char *data,size_t length) {
    if(path_exists(path))return -1;
    if(write_new_file(path,data,length)<0)return -1;
    return verify_file(path,data,length,g_config_verify,sizeof(g_config_verify));
}
static int rollback_config(int old_moved,int plugin_added) {
    int ok=1;
    if(old_moved) {
        (void)sceIoRemove(RT_CONFIG_PATH);
        if(sceIoRename(RT_CONFIG_OLD,RT_CONFIG_PATH)<0)ok=0;
    }
    if(plugin_added && sceIoRemove(RT_PLUGIN_TARGET)<0)ok=0;
    (void)sceIoRemove(RT_CONFIG_TEMP);
    (void)sceIoRemove(RT_PLUGIN_TEMP);
    return ok?0:-1;
}

rtns_install_result rt_settings_install_native(int enable_disabled,char *message,size_t message_capacity) {
    size_t config_length=0,new_length=0,payload_length=0,verify_length=0;
    size_t snapshot_length=0;
    char backup_path[256];
    ts_snapshot_result snapshot;
    rttc_result config_plan;
    rtns_install_result result=RTNS_INSTALL_IO_FAILED;
    int config_changed=0,plugin_exists=0,plugin_added=0,old_moved=0;
    int snapshot_created=0;
    if(!message||!message_capacity)return RTNS_INSTALL_ERROR;
    message[0]='\0';
    if(read_payload(&payload_length)<0) {
        set_message(message,message_capacity,"Installer payload missing or invalid in VPK.");
        return RTNS_INSTALL_IO_FAILED;
    }
    if(read_file(RT_CONFIG_PATH,g_config,sizeof(g_config),&config_length)<0) {
        set_message(message,message_capacity,"ur0:tai/config.txt is missing, unreadable or too large. Nothing changed.");
        return RTNS_INSTALL_IO_FAILED;
    }
    config_plan=rttc_add_settings_plugin(g_config,config_length,RT_PLUGIN_LINE,
                                         g_config_new,sizeof(g_config_new),&new_length);
    if(config_plan==RTTC_AMBIGUOUS) {
        set_message(message,message_capacity,"Multiple Rt entries or duplicate *NPXS10015 sections detected. Manual review required.");
        return RTNS_INSTALL_CONFIG_AMBIGUOUS;
    }
    if(config_plan==RTTC_DISABLED) {
        if(!enable_disabled) {
            set_message(message,message_capacity,"A disabled Rt entry already exists. Confirm Install/Enable to activate it.");
            return RTNS_INSTALL_DISABLED_ENTRY;
        }
        config_plan=rttc_enable_settings_plugin(g_config,config_length,RT_PLUGIN_LINE,
                                                g_config_new,sizeof(g_config_new),&new_length);
        if(config_plan!=RTTC_ENABLED&&config_plan!=RTTC_ALREADY_ACTIVE) {
            set_message(message,message_capacity,"Could not safely enable the existing Rt entry.");
            return RTNS_INSTALL_CONFIG_AMBIGUOUS;
        }
    }
    config_changed=(config_plan==RTTC_ADDED||config_plan==RTTC_ENABLED);

    if(path_exists(RT_PLUGIN_TARGET)) {
        plugin_exists=1;
        if(verify_file(RT_PLUGIN_TARGET,g_payload,payload_length,g_file_verify,sizeof(g_file_verify))<0) {
            set_message(message,message_capacity,"A different rtsettings.suprx already exists. It was not overwritten.");
            return RTNS_INSTALL_CONFLICT;
        }
    }
    if(plugin_exists&&!config_changed) {
        set_message(message,message_capacity,"Rt native Settings integration is already installed.");
        return RTNS_INSTALL_ALREADY;
    }
    if(path_exists(RT_PLUGIN_TEMP)||path_exists(RT_CONFIG_TEMP)||path_exists(RT_CONFIG_OLD)) {
        set_message(message,message_capacity,"An unfinished installer transaction file exists. Nothing was overwritten.");
        return RTNS_INSTALL_CONFLICT;
    }
    if(ensure_installer_dir()<0) {
        set_message(message,message_capacity,"Could not create the Rt installer backup directory.");
        return RTNS_INSTALL_BACKUP_FAILED;
    }

    memset(&snapshot,0,sizeof(snapshot));
    if(ts_create_snapshot("ur0:tai",RT_ARCHIVES,TS_ARCHIVE_PRE_RESTORE,0,
                          "Before installing Rt native Settings SUPRX",&snapshot)!=TS_OK) {
        set_message(message,message_capacity,"Pre-install taiHEN snapshot failed. No files were changed.");
        return RTNS_INSTALL_BACKUP_FAILED;
    }
    snapshot_created=1;
    if(config_changed) {
        if(snprintf(backup_path,sizeof(backup_path),"%s/config-%s.bak",
                    RT_INSTALLER_DIR,snapshot.snapshot_id)>=(int)sizeof(backup_path)||
           write_config_backup(backup_path,g_config,config_length)<0) {
            set_message(message,message_capacity,"Verified config backup could not be created. No files were changed.");
            return RTNS_INSTALL_BACKUP_FAILED;
        }
    }

    if(!plugin_exists) {
        if(write_new_file(RT_PLUGIN_TEMP,g_payload,payload_length)<0||
           verify_file(RT_PLUGIN_TEMP,g_payload,payload_length,g_file_verify,sizeof(g_file_verify))<0) {
            (void)sceIoRemove(RT_PLUGIN_TEMP);
            set_message(message,message_capacity,"Could not stage and verify rtsettings.suprx.");
            return RTNS_INSTALL_IO_FAILED;
        }
        if(sceIoRename(RT_PLUGIN_TEMP,RT_PLUGIN_TARGET)<0) {
            (void)sceIoRemove(RT_PLUGIN_TEMP);
            set_message(message,message_capacity,"Could not place rtsettings.suprx in ur0:tai.");
            return RTNS_INSTALL_IO_FAILED;
        }
        plugin_added=1;
    }

    if(config_changed) {
        if(write_new_file(RT_CONFIG_TEMP,g_config_new,new_length)<0||
           verify_file(RT_CONFIG_TEMP,g_config_new,new_length,g_config_verify,sizeof(g_config_verify))<0) {
            (void)rollback_config(0,plugin_added);
            set_message(message,message_capacity,"Could not stage and verify the updated taiHEN config.");
            return RTNS_INSTALL_IO_FAILED;
        }
        if(sceIoRename(RT_CONFIG_PATH,RT_CONFIG_OLD)<0) {
            (void)rollback_config(0,plugin_added);
            set_message(message,message_capacity,"Could not move the original config into the rollback slot.");
            return RTNS_INSTALL_IO_FAILED;
        }
        old_moved=1;
        if(sceIoRename(RT_CONFIG_TEMP,RT_CONFIG_PATH)<0) {
            (void)rollback_config(old_moved,plugin_added);
            set_message(message,message_capacity,"Config replacement failed; original config was restored where possible.");
            return RTNS_INSTALL_IO_FAILED;
        }
        if(verify_file(RT_CONFIG_PATH,g_config_new,new_length,g_config_verify,sizeof(g_config_verify))<0) {
            (void)rollback_config(old_moved,plugin_added);
            set_message(message,message_capacity,"Config read-back verification failed; rollback was attempted.");
            return RTNS_INSTALL_IO_FAILED;
        }
        if(sceIoRemove(RT_CONFIG_OLD)<0) {
            /* The verified external backup and the new config are both safe;
               leave the old copy as an extra recovery file. */
            set_message(message,message_capacity,"Installed and verified. An extra config rollback copy remains in ur0:tai.");
            return RTNS_INSTALL_OK;
        }
    } else {
        /* The config entry was already active; only a missing module was installed. */
        if(verify_file(RT_CONFIG_PATH,g_config,config_length,g_config_verify,sizeof(g_config_verify))<0) {
            (void)rollback_config(0,plugin_added);
            set_message(message,message_capacity,"Existing config changed during installation; new module was rolled back.");
            return RTNS_INSTALL_IO_FAILED;
        }
    }
    if(!snapshot_created) {
        set_message(message,message_capacity,"Installation finished without a verified backup.");
        return RTNS_INSTALL_BACKUP_FAILED;
    }
    snprintf(message,message_capacity,"Installed and verified. Pre-install backup: %s",snapshot.snapshot_id);
    (void)snapshot_length;(void)verify_length;
    return RTNS_INSTALL_OK;
}
