#include "asset_manifest.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct { unsigned char data[RTA_MANIFEST_MAX_SIZE]; size_t length; int fail_write; int fail_read; int corrupt_write; } mem_store;
static int store_read(void *ctx,const char *path,unsigned char *out,size_t cap,size_t *len) {
    mem_store *m=(mem_store *)ctx; (void)path;
    if (m->fail_read || m->length>cap) return -1;
    memcpy(out,m->data,m->length); *len=m->length; return 0;
}
static int store_write(void *ctx,const char *path,const unsigned char *data,size_t len) {
    mem_store *m=(mem_store *)ctx; (void)path;
    if (m->fail_write || len>sizeof(m->data)) return -1;
    memcpy(m->data,data,len); m->length=len;
    if (m->corrupt_write && m->length > 20) m->data[20] ^= 0x20;
    return 0;
}
static void make_registry(rta_asset_registry *r) {
    rta_asset_observation o; memset(&o,0,sizeof(o));
    o.id="boot-splash-1"; o.title="Night sky"; o.path="ur0:data/rt/assets/night.png";
    o.owner="Rt"; o.source="ux0:data/imports/night.png"; o.format="png";
    o.sha256="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    o.type=RTA_TYPE_BOOT_SPLASH; o.state=RTA_STATE_AVAILABLE; o.size_bytes=3;
    o.width=960; o.height=544; o.managed=1; o.compatible=1; o.hash_verified=1;
    assert(rta_asset_registry_build(&o,1,r)==RTA_OK);
}
int main(void) {
    rta_asset_registry source, decoded, untouched;
    unsigned char buffer[RTA_MANIFEST_MAX_SIZE], corrupted[RTA_MANIFEST_MAX_SIZE];
    size_t length=0; mem_store memory; rta_asset_manifest_io io;
    make_registry(&source); memset(&untouched,0x5A,sizeof(untouched)); decoded=untouched;
    assert(rta_asset_manifest_encode(&source,buffer,sizeof(buffer),&length)==RTA_OK);
    assert(length>44 && memcmp(buffer,"RTAM",4)==0);
    assert(rta_asset_manifest_decode(buffer,length,&decoded)==RTA_OK);
    assert(decoded.count==1 && strcmp(decoded.assets[0].title,"Night sky")==0);
    assert(strcmp(decoded.assets[0].source,"ux0:data/imports/night.png")==0);
    assert(decoded.assets[0].hash_verified && decoded.active_count==0);
    memcpy(corrupted,buffer,length); corrupted[20]^=0x40;
    assert(rta_asset_manifest_decode(corrupted,length,&decoded)==RTA_ERR_INTEGRITY);
    assert(decoded.count==1 && strcmp(decoded.assets[0].title,"Night sky")==0); /* output unchanged */
    assert(rta_asset_manifest_decode(buffer,length-1,&decoded)==RTA_ERR_INTEGRITY);
    memcpy(corrupted,buffer,length); corrupted[4]=99;
    assert(rta_asset_manifest_decode(corrupted,length,&decoded)==RTA_ERR_INTEGRITY);
    assert(rta_asset_manifest_encode(&source,buffer,20,&length)==RTA_ERR_CAPACITY);
    {
        rta_asset_registry invalid=source;
        strcpy(invalid.assets[0].id,"bad id");
        assert(rta_asset_manifest_encode(&invalid,buffer,sizeof(buffer),&length)==RTA_ERR_INVALID_ASSET);
        invalid=source;
        strcpy(invalid.assets[0].sha256,"");
        invalid.assets[0].hash_verified=0;
        assert(rta_asset_manifest_encode(&invalid,buffer,sizeof(buffer),&length)==RTA_OK);
        assert(rta_asset_manifest_decode(buffer,length,&decoded)==RTA_OK);
        assert(!decoded.assets[0].hash_verified && decoded.assets[0].sha256[0]=='\0');
    }
    memset(&memory,0,sizeof(memory)); io.context=&memory; io.read=store_read; io.replace_atomic=store_write;
    assert(rta_asset_manifest_save(&source,&io,"ur0:data/rt/assets/manifest.rtam",buffer,sizeof(buffer))==RTA_OK);
    /* A backend that reports success but stores altered bytes must be caught. */
    memory.corrupt_write=1;
    assert(rta_asset_manifest_save(&source,&io,"ur0:data/rt/assets/manifest.rtam",buffer,sizeof(buffer))==RTA_ERR_INTEGRITY);
    memory.corrupt_write=0;
    /* Read-back failure is reported even though atomic replacement succeeded. */
    memory.fail_read=1;
    assert(rta_asset_manifest_save(&source,&io,"ur0:data/rt/assets/manifest.rtam",buffer,sizeof(buffer))==RTA_ERR_BACKEND);
    memory.fail_read=0;
    memset(&decoded,0,sizeof(decoded));
    assert(rta_asset_manifest_load(&io,"ur0:data/rt/assets/manifest.rtam",buffer,sizeof(buffer),&decoded)==RTA_OK);
    assert(decoded.count==1 && strcmp(decoded.assets[0].id,"boot-splash-1")==0);
    memory.fail_write=1;
    assert(rta_asset_manifest_save(&source,&io,"manifest",buffer,sizeof(buffer))==RTA_ERR_BACKEND);
    memory.fail_write=0;
    memory.fail_read=1;
    assert(rta_asset_manifest_load(&io,"manifest",buffer,sizeof(buffer),&decoded)==RTA_ERR_BACKEND);
    puts("Asset manifest codec and persistence contract tests passed.");
    return 0;
}
