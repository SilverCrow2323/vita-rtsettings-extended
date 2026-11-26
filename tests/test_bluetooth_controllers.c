#include "bluetooth_controllers.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){
 rtp_bt_controller_observation devices[]={
  {"ds4-main","DualShock 4",RTP_BT_DUALSHOCK4,RTP_BT_CONNECTED,RTP_BT_PROVIDER_VITACONTROL,
   RTP_BT_CAP_BUTTONS|RTP_BT_CAP_ANALOG_STICKS|RTP_BT_CAP_TRIGGERS|RTP_BT_CAP_TOUCHPAD,82,1,
   RTP_BT_CATEGORY_CONTROLLER,RTP_BT_CATEGORY_SOURCE_UNKNOWN,0x2508},
  {"audio","Wireless Headset",RTP_BT_DEVICE_UNKNOWN,RTP_BT_CONNECTED,RTP_BT_PROVIDER_UNKNOWN,
   RTP_BT_CAP_AUDIO_OUTPUT|RTP_BT_CAP_AUDIO_INPUT,-1,0,RTP_BT_CATEGORY_UNKNOWN,RTP_BT_CATEGORY_SOURCE_UNKNOWN,0x000400},
  {"pc","Desktop PC",RTP_BT_DEVICE_UNKNOWN,RTP_BT_PAIRED,RTP_BT_PROVIDER_UNKNOWN,
   0,-1,0,RTP_BT_CATEGORY_UNKNOWN,RTP_BT_CATEGORY_SOURCE_UNKNOWN,0x000100},
  {"phone","Android phone",RTP_BT_DEVICE_UNKNOWN,RTP_BT_DISCONNECTED,RTP_BT_PROVIDER_UNKNOWN,
   0,-1,0,RTP_BT_CATEGORY_UNKNOWN,RTP_BT_CATEGORY_SOURCE_UNKNOWN,0x000200},
  {"ds3-spare","DualShock 3",RTP_BT_DUALSHOCK3,RTP_BT_PAIRED,RTP_BT_PROVIDER_VITACONTROL,
   RTP_BT_CAP_BUTTONS|RTP_BT_CAP_ANALOG_STICKS|RTP_BT_CAP_TRIGGERS,-1,1,
   RTP_BT_CATEGORY_CONTROLLER,RTP_BT_CATEGORY_SOURCE_UNKNOWN,0x2508},
  {"xbox","Xbox One",RTP_BT_XBOX_ONE,RTP_BT_DISCONNECTED,RTP_BT_PROVIDER_VITACONTROL,
   RTP_BT_CAP_BUTTONS|RTP_BT_CAP_ANALOG_STICKS,-1,1,RTP_BT_CATEGORY_CONTROLLER,RTP_BT_CATEGORY_SOURCE_UNKNOWN,0}
 };
 rtp_bt_snapshot snap;unsigned int conflicts=0;rtp_bt_input_session session;
 rtp_bt_input_observation event={1,RTP_BT_INPUT_BUTTON,1,"Button A"};
 assert(rtp_bt_snapshot_build(devices,6,&snap)==RTP_BT_OK);
 assert(snap.count==6&&snap.connected_count==2&&snap.paired_count==2);
 assert(snap.controllers[0].battery_percent==82&&snap.controllers[1].battery_percent==-1);
 assert(snap.controllers[1].category==RTP_BT_CATEGORY_AUDIO);
 assert(strcmp(rtp_bt_device_category_name(snap.controllers[2].category),"computer")==0);
 assert(snap.controllers[2].category_source==RTP_BT_CATEGORY_SOURCE_CLASS_OF_DEVICE);
 assert(rtp_bt_set_device_category(&snap,"phone",RTP_BT_CATEGORY_TABLET)==RTP_BT_OK);
 assert(snap.controllers[3].category==RTP_BT_CATEGORY_TABLET&&snap.controllers[3].category_source==RTP_BT_CATEGORY_SOURCE_USER);
 assert(strcmp(rtp_bt_device_category_name(snap.controllers[3].category),"phone")==0);
 assert(rtp_bt_classify_device(0x000100)==RTP_BT_CATEGORY_COMPUTER);
 assert(rtp_bt_classify_device(0x000200)==RTP_BT_CATEGORY_PHONE);
 assert(rtp_bt_classify_device(0x000400)==RTP_BT_CATEGORY_AUDIO);
 assert(rtp_bt_classify_device(0x000508)==RTP_BT_CATEGORY_CONTROLLER);
 assert(rtp_bt_classify_device(0x000540)==RTP_BT_CATEGORY_KEYBOARD);
 assert(rtp_bt_classify_device(0x000580)==RTP_BT_CATEGORY_MOUSE);
 assert(rtp_bt_snapshot_build(NULL,1,&snap)==RTP_BT_INVALID_ARGUMENT);
 assert(rtp_bt_snapshot_build(devices,33,&snap)==RTP_BT_CAPACITY);
 {
  rtp_bt_controller_observation bad=devices[0];bad.battery_percent=101;
  assert(rtp_bt_snapshot_build(&bad,1,&snap)==RTP_BT_INVALID_CONTROLLER);
  bad=devices[0];bad.id="";
  assert(rtp_bt_snapshot_build(&bad,1,&snap)==RTP_BT_INVALID_CONTROLLER);
 }
 {
  rtp_bt_controller_observation dup[]={devices[0],devices[4]};dup[1].id=dup[0].id;
  assert(rtp_bt_snapshot_build(dup,2,&snap)==RTP_BT_DUPLICATE_ID);
 }
 assert(rtp_bt_validate_driver_stack(RTP_BT_DRIVER_VITACONTROL,&conflicts)==RTP_BT_OK);
 assert(rtp_bt_validate_driver_stack(RTP_BT_DRIVER_MINIVITATV|RTP_BT_DRIVER_8BITVITA,&conflicts)==RTP_BT_OK);
 assert(rtp_bt_validate_driver_stack(RTP_BT_DRIVER_X1VITA|RTP_BT_DRIVER_DS34VITA,&conflicts)==RTP_BT_DRIVER_CONFLICT);
 assert(rtp_bt_validate_driver_stack(1u<<9,&conflicts)==RTP_BT_INVALID_ARGUMENT);
 rtp_bt_input_session_init(&session);
 assert(rtp_bt_input_record(&session,&event)==RTP_BT_INPUT_NOT_ACTIVE);
 assert(rtp_bt_input_session_start(&session)==RTP_BT_OK);
 assert(rtp_bt_input_record(&session,&event)==RTP_BT_OK);
 event.value=0;assert(rtp_bt_input_record(&session,&event)==RTP_BT_OK);
 event.code=4;event.kind=RTP_BT_INPUT_AXIS;event.value=-32768;event.label="Left stick X";
 assert(rtp_bt_input_record(&session,&event)==RTP_BT_OK);
 assert(session.count==2&&session.controls[0].observations==2);
 assert(session.controls[0].first_value==1&&session.controls[0].last_value==0);
 assert(session.controls[1].first_value==-32768);
 assert(rtp_bt_input_session_stop(&session)==RTP_BT_OK);
 assert(rtp_bt_input_record(&session,&event)==RTP_BT_INPUT_NOT_ACTIVE);
 puts("Bluetooth device classification and input learning tests passed.");return 0;
}
