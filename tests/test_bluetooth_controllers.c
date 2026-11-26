#include "bluetooth_controllers.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 rtp_bt_controller_observation devices[]={
  {"ds4-main","DualShock 4",RTP_BT_DUALSHOCK4,RTP_BT_CONNECTED,RTP_BT_PROVIDER_VITACONTROL,
   RTP_BT_CAP_BUTTONS|RTP_BT_CAP_ANALOG_STICKS|RTP_BT_CAP_TRIGGERS|RTP_BT_CAP_TOUCHPAD,82,1},
  {"ds3-spare","DualShock 3",RTP_BT_DUALSHOCK3,RTP_BT_PAIRED,RTP_BT_PROVIDER_VITACONTROL,
   RTP_BT_CAP_BUTTONS|RTP_BT_CAP_ANALOG_STICKS|RTP_BT_CAP_TRIGGERS, -1,1},
  {"xbox","Xbox One",RTP_BT_XBOX_ONE,RTP_BT_DISCONNECTED,RTP_BT_PROVIDER_VITACONTROL,
   RTP_BT_CAP_BUTTONS|RTP_BT_CAP_ANALOG_STICKS, -1,1}
 };
 rtp_bt_snapshot snap;unsigned int conflicts=0;
 assert(rtp_bt_snapshot_build(devices,3,&snap)==RTP_BT_OK);
 assert(snap.count==3&&snap.connected_count==1&&snap.paired_count==1);
 assert(snap.controllers[0].battery_percent==82);
 assert(snap.controllers[1].battery_percent==-1);
 assert(strcmp(rtp_bt_provider_name(snap.controllers[0].provider),"vitacontrol")==0);
 assert(rtp_bt_snapshot_build(NULL,1,&snap)==RTP_BT_INVALID_ARGUMENT);
 assert(rtp_bt_snapshot_build(devices,9,&snap)==RTP_BT_CAPACITY);
 {
  rtp_bt_controller_observation bad=devices[0];bad.battery_percent=101;
  assert(rtp_bt_snapshot_build(&bad,1,&snap)==RTP_BT_INVALID_CONTROLLER);
  bad=devices[0];bad.id="";
  assert(rtp_bt_snapshot_build(&bad,1,&snap)==RTP_BT_INVALID_CONTROLLER);
 }
 {
  rtp_bt_controller_observation dup[]={devices[0],devices[1]};
  dup[1].id=dup[0].id;
  assert(rtp_bt_snapshot_build(dup,2,&snap)==RTP_BT_DUPLICATE_ID);
 }
 assert(rtp_bt_validate_driver_stack(RTP_BT_DRIVER_VITACONTROL,&conflicts)==RTP_BT_OK);
 assert(conflicts==0);
 assert(rtp_bt_validate_driver_stack(RTP_BT_DRIVER_VITACONTROL|RTP_BT_DRIVER_MINIVITATV,
                                    &conflicts)==RTP_BT_DRIVER_CONFLICT);
 assert(conflicts==(RTP_BT_DRIVER_VITACONTROL|RTP_BT_DRIVER_MINIVITATV));
 assert(rtp_bt_validate_driver_stack(1u<<9,&conflicts)==RTP_BT_INVALID_ARGUMENT);
 puts("Bluetooth Controller Control Centre tests passed.");return 0;
}
