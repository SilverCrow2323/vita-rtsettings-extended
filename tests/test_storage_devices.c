#include "storage_devices.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 const rtp_storage_observation devices[]={
  {"sd2vita","SD2Vita","ux0:",RTP_STORAGE_SD2VITA,RTP_STORAGE_MOUNTED,128ULL*1024*1024*1024,64ULL*1024*1024*1024,0},
  {"sony-card","Sony Memory Card","uma0:",RTP_STORAGE_SONY_MEMORY_CARD,RTP_STORAGE_MOUNTED,16ULL*1024*1024*1024,4ULL*1024*1024*1024,0},
  {"internal","Internal storage","imc0:",RTP_STORAGE_INTERNAL,RTP_STORAGE_UNAVAILABLE,0,0,1}
 };
 rtp_storage_snapshot s;
 assert(rtp_storage_snapshot_build(devices,3,&s)==RTP_STORAGE_OK);assert(s.count==3&&s.mounted_count==2&&s.unavailable_count==1);
 assert(s.devices[0].free_percent==50);assert(strcmp(s.devices[1].mount_point,"uma0:")==0);
 assert(rtp_storage_snapshot_build(devices,9,&s)==RTP_STORAGE_CAPACITY);
 assert(rtp_storage_snapshot_build(NULL,1,&s)==RTP_STORAGE_INVALID_ARGUMENT);
 {
  rtp_storage_observation bad={"bad","Bad","ux0:",RTP_STORAGE_SD2VITA,RTP_STORAGE_MOUNTED,10,11,0};
  assert(rtp_storage_snapshot_build(&bad,1,&s)==RTP_STORAGE_INVALID_DEVICE);
 }
 {
  rtp_storage_observation dup[]={
   {"a","A","ux0:",RTP_STORAGE_SD2VITA,RTP_STORAGE_MOUNTED,100,50,0},
   {"b","B","ux0:",RTP_STORAGE_SONY_MEMORY_CARD,RTP_STORAGE_MOUNTED,100,25,0}
  };
  assert(rtp_storage_snapshot_build(dup,2,&s)==RTP_STORAGE_OK);assert(s.duplicate_mount_count==1);
  assert(s.devices[0].duplicate_mount_point&&s.devices[1].duplicate_mount_point);
 }
 {
  rtp_storage_observation dupid[]={
   {"same","A","ux0:",RTP_STORAGE_SD2VITA,RTP_STORAGE_MOUNTED,100,50,0},
   {"same","B","uma0:",RTP_STORAGE_SONY_MEMORY_CARD,RTP_STORAGE_MOUNTED,100,25,0}
  };
  assert(rtp_storage_snapshot_build(dupid,2,&s)==RTP_STORAGE_DUPLICATE_ID);
 }
 assert(strcmp(rtp_storage_state_name(RTP_STORAGE_ERROR),"error")==0);
 puts("Storage & Devices tests passed.");return 0;
}
