#ifndef RTSETTINGS_BLUETOOTH_CONTROLLERS_H
#define RTSETTINGS_BLUETOOTH_CONTROLLERS_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_BT_MAX_CONTROLLERS 8
#define RTP_BT_ID_MAX 31
#define RTP_BT_LABEL_MAX 63
#define RTP_BT_PROVIDER_MAX 31

typedef enum {
 RTP_BT_DEVICE_UNKNOWN=0, RTP_BT_DUALSHOCK3=1, RTP_BT_DUALSHOCK4=2,
 RTP_BT_DUALSENSE=3, RTP_BT_DUALSENSE_EDGE=4, RTP_BT_XBOX_ONE=5,
 RTP_BT_SWITCH_PRO=6, RTP_BT_OTHER=7
} rtp_bt_device_kind;
typedef enum {
 RTP_BT_DISCONNECTED=0, RTP_BT_CONNECTED=1, RTP_BT_PAIRED=2,
 RTP_BT_UNAVAILABLE=3, RTP_BT_ERROR=4
} rtp_bt_connection_state;
typedef enum {
 RTP_BT_PROVIDER_UNKNOWN=0, RTP_BT_PROVIDER_VITACONTROL=1,
 RTP_BT_PROVIDER_MINIVITATV=2, RTP_BT_PROVIDER_DS34VITA=3,
 RTP_BT_PROVIDER_DS3VITA=4, RTP_BT_PROVIDER_DS4VITA=5
} rtp_bt_provider;
typedef enum {
 RTP_BT_CAP_BUTTONS=1u<<0, RTP_BT_CAP_ANALOG_STICKS=1u<<1,
 RTP_BT_CAP_TRIGGERS=1u<<2, RTP_BT_CAP_TOUCHPAD=1u<<3,
 RTP_BT_CAP_GYROSCOPE=1u<<4, RTP_BT_CAP_RUMBLE=1u<<5,
 RTP_BT_CAP_MOTION=1u<<6, RTP_BT_CAP_MULTI_CONTROLLER=1u<<7
} rtp_bt_capability;
typedef enum {
 RTP_BT_DRIVER_VITACONTROL=1u<<0, RTP_BT_DRIVER_MINIVITATV=1u<<1,
 RTP_BT_DRIVER_DS34VITA=1u<<2, RTP_BT_DRIVER_DS3VITA=1u<<3,
 RTP_BT_DRIVER_DS4VITA=1u<<4
} rtp_bt_driver_mask;
typedef struct {
 const char *id,*label;
 rtp_bt_device_kind kind;
 rtp_bt_connection_state state;
 rtp_bt_provider provider;
 unsigned int capabilities;
 int battery_percent; /* -1 means unavailable; never inferred. */
 int driver_active;
} rtp_bt_controller_observation;
typedef struct {
 char id[RTP_BT_ID_MAX+1],label[RTP_BT_LABEL_MAX+1];
 rtp_bt_device_kind kind;
 rtp_bt_connection_state state;
 rtp_bt_provider provider;
 unsigned int capabilities;
 int battery_percent,driver_active;
} rtp_bt_controller;
typedef struct {
 size_t count,connected_count,paired_count,unavailable_count,error_count;
 rtp_bt_controller controllers[RTP_BT_MAX_CONTROLLERS];
} rtp_bt_snapshot;
typedef enum {
 RTP_BT_OK=0,RTP_BT_INVALID_ARGUMENT=-1,RTP_BT_CAPACITY=-2,
 RTP_BT_INVALID_CONTROLLER=-3,RTP_BT_DUPLICATE_ID=-4,
 RTP_BT_DRIVER_CONFLICT=-5,RTP_BT_DRIVER_LIMIT=-6
} rtp_bt_result;
/* A read-only projection of backend observations. No pairing or radio operation occurs. */
int rtp_bt_snapshot_build(const rtp_bt_controller_observation *observations,
                          size_t count,rtp_bt_snapshot *snapshot);
/* Driver plugins are mutually exclusive providers. reVita/remapping plugins are not drivers. */
int rtp_bt_validate_driver_stack(unsigned int enabled_driver_mask,
                                 unsigned int *conflicting_mask);
const char *rtp_bt_device_kind_name(rtp_bt_device_kind kind);
const char *rtp_bt_connection_state_name(rtp_bt_connection_state state);
const char *rtp_bt_provider_name(rtp_bt_provider provider);
#ifdef __cplusplus
}
#endif
#endif
