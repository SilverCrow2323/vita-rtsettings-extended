#include "repatch_manager.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {char paths[8][256];int count,fail_rename;} fake_fs;
static int exists(void *ctx,const char *path){fake_fs *f=ctx;int i;for(i=0;i<f->count;++i)if(!strcmp(f->paths[i],path))return 1;return 0;}
static int rename_nr(void *ctx,const char *src,const char *dst){fake_fs *f=ctx;int i;if(f->fail_rename)return -1;
 for(i=0;i<f->count;++i)if(!strcmp(f->paths[i],src)){if(exists(ctx,dst))return -1;strcpy(f->paths[i],dst);return 0;}return -1;}
int main(void){
 rtp_repatch_observation items[]={{"PCSE00001","Example Game",RTP_REPATCH_GAME},
 {"PCSE00002DIS","Another Game",RTP_REPATCH_GAME},{"PCSE00003","DLC pack",RTP_REPATCH_DLC}};
 rtp_repatch_snapshot s;fake_fs f={0};rtp_repatch_fs io={&f,exists,rename_nr};
 assert(rtp_repatch_snapshot_build(items,3,&s)==RTP_REPATCH_OK);
 assert(s.count==3&&s.entries[0].state==RTP_REPATCH_ACTIVE);
 assert(s.entries[1].state==RTP_REPATCH_DISABLED&&!strcmp(s.entries[1].id,"PCSE00002"));
 strcpy(f.paths[f.count++],"ux0:/rePatch/PCSE00001");
 assert(rtp_repatch_set_state(&s,"PCSE00001",RTP_REPATCH_DISABLED,RTP_REPATCH_GAME,"ux0:/rePatch",&io)==RTP_REPATCH_OK);
 assert(exists(&f,"ux0:/rePatch/PCSE00001DIS"));
 assert(rtp_repatch_set_state(&s,"PCSE00001",RTP_REPATCH_ACTIVE,RTP_REPATCH_GAME,"ux0:/rePatch",&io)==RTP_REPATCH_OK);
 assert(exists(&f,"ux0:/rePatch/PCSE00001"));
 strcpy(f.paths[f.count++],"ux0:/rePatch/PCSE00001DIS");
 assert(rtp_repatch_set_state(&s,"PCSE00001",RTP_REPATCH_DISABLED,RTP_REPATCH_GAME,"ux0:/rePatch",&io)==RTP_REPATCH_DESTINATION_EXISTS);
 /* Remove the intentional collision so the next assertion reaches rename(). */
 --f.count;
 f.fail_rename=1;
 assert(rtp_repatch_set_state(&s,"PCSE00001",RTP_REPATCH_DISABLED,RTP_REPATCH_GAME,"ux0:/rePatch",&io)==RTP_REPATCH_BACKEND_ERROR);
 {
  rtp_repatch_observation bad={"../evil","bad",RTP_REPATCH_GAME};
  assert(rtp_repatch_snapshot_build(&bad,1,&s)==RTP_REPATCH_INVALID_ENTRY);
 }
 puts("RePatch manager tests passed.");return 0;
}
