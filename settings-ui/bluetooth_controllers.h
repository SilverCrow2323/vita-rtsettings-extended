#ifndef RTSETTINGS_BLUETOOTH_CONTROLLERS_H
#define RTSETTINGS_BLUETOOTH_CONTROLLERS_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RTP_BT_MAX_CONTROLLERS 32
#define RTP_BT_ID_MAX 31
#define RTP_BT_LABEL_MAX 63
#define RTP_BT_MAX_INPUT_EVENTS 128
#define RTP_BT_CONTROL_LABEL_MAX 31

typedef enum {
 RTP_BT_DEVICE_UNKNOWN=0, RTP_BT_DUALSHOCK3=1, RTP_BT_DUALSHOCK4=2,
 RTP_BT_DUALSENSE=3, RTP_BT_DUALSENSE_EDGE=4, RTP_BT_XBOX_ONE=5,
 RTP_BT_SWITCH_PRO=6, RTP_BT_OTHER=7, RTP_BT_8BITDO=8
} rtp_bt_device_kind;
typedef enum {
 RTP_BT_CATEGORY_UNKNOWN=0, RTP_BT_CATEGORY_CONTROLLER=1,
 RTP_BT_CATEGORY_AUDIO=2, RTP_BT_CATEGORY_COMPUTER=3,
 RTP_BT_CATEGORY_PHONE=4, RTP_BT_CATEGORY_TABLET=5,
 RTP_BT_CATEGORY_KEYBOARD=6, RTP_BT_CATEGORY_MOUSE=7,
 RTP_BT_CATEGORY_PRINTER=8, RTP_BT_CATEGORY_REMOTE=9,
 RTP_BT_CATEGORY_OTHER=10
} rtp_bt_device_category;
typedef enum {RTP_BT_CATEGORY_SOURCE_UNKNOWN=0,RTP_BT_CATEGORY_SOURCE_CLASS_OF_DEVICE=1,RTP_BT_CATEGORY_SOURCE_USER=2} rtp_bt_category_source;
typedef enum {
 RTP_BT_DISCONNECTED=0, RTP_BT_CONNECTED=1, RTP_BT_PAIRED=2,
 RTP_BT_UNAVAILABLE=3, RTP_BT_ERROR=4
} rtp_bt_connection_state;
typedef enum {
 RTP_BT_PROVIDER_UNKNOWN=0, RTP_BT_PROVIDER_VITACONTROL=1,
 RTP_BT_PROVIDER_MINIVITATV=2, RTP_BT_PROVIDER_DS34VITA=3,
 RTP_BT_PROVIDER_DS3VITA=4, RTP_BT_PROVIDER_DS4VITA=5,
 RTP_BT_PROVIDER_X1VITA=6, RTP_BT_PROVIDER_8BITVITA=7
} rtp_bt_provider;
typedef enum {
 RTP_BT_CAP_BUTTONS=1u<<0, RTP_BT_CAP_ANALOG_STICKS=1u<<1,
 RTP_BT_CAP_TRIGGERS=1u<<2, RTP_BT_CAP_TOUCHPAD=1u<<3,
 RTP_BT_CAP_GYROSCOPE=1u<<4, RTP_BT_CAP_RUMBLE=1u<<5,
 RTP_BT_CAP_MOTION=1u<<6, RTP_BT_CAP_MULTI_CONTROLLER=1u<<7,
 RTP_BT_CAP_AUDIO_OUTPUT=1u<<8, RTP_BT_CAP_AUDIO_INPUT=1u<<9,
 RTP_BT_CAP_KEYBOARD=1u<<10, RTP_BT_CAP_POINTER=1u<<11
} rtp_bt_capability;
typedef enum {
 RTP_BT_DRIVER_VITACONTROL=1u<<0, RTP_BT_DRIVER_MINIVITATV=1u<<1,
 RTP_BT_DRIVER_DS34VITA=1u<<2, RTP_BT_DRIVER_DS3VITA=1u<<3,
 RTP_BT_DRIVER_DS4VITA=1u<<4, RTP_BT_DRIVER_X1VITA=1u<<5,
 RTP_BT_DRIVER_8BITVITA=1u<<6
} rtp_bt_driver_mask;
typedef struct {
 const char *id,*label;
 rtp_bt_device_kind kind;
 rtp_bt_connection_state state;
 rtp_bt_provider provider;
 unsigned int capabilities;
 int battery_percent;
 int driver_active;
 rtp_bt_device_category category;
 rtp_bt_category_source category_source;
 unsigned int bluetooth_class;
} rtp_bt_controller_observation;
typedef struct {
 char id[RTP_BT_ID_MAX+1],label[RTP_BT_LABEL_MAX+1];
 rtp_bt_device_kind kind;
 rtp_bt_device_category category;
 rtp_bt_category_source category_source;
 rtp_bt_connection_state state;
 rtp_bt_provider provider;
 unsigned int capabilities,bluetooth_class;
 int battery_percent,driver_active;
} rtp_bt_controller;
typedef struct {
 size_t count,connected_count,paired_count,unavailable_count,error_count;
 rtp_bt_controller controllers[RTP_BT_MAX_CONTROLLERS];
} rtp_bt_snapshot;
typedef enum {
 RTP_BT_INPUT_BUTTON=1,RTP_BT_INPUT_AXIS=2,RTP_BT_INPUT_TRIGGER=3,
 RTP_BT_INPUT_TOUCH=4,RTP_BT_INPUT_GYRO=5,RTP_BT_INPUT_ACCELEROMETER=6,
 RTP_BT_INPUT_HAT=7,RTP_BT_INPUT_UNKNOWN=8
} rtp_bt_input_kind;
typedef struct {
 unsigned int code;
 rtp_bt_input_kind kind;
 int value;
 const char *label;
} rtp_bt_input_observation;
typedef struct {
 unsigned int code;
 rtp_bt_input_kind kind;
 int first_value,last_value,observations;
 char label[RTP_BT_CONTROL_LABEL_MAX+1];
} rtp_bt_input_control;
typedef struct {
 int active;
 size_t count;
 rtp_bt_input_control controls[RTP_BT_MAX_INPUT_EVENTS];
} rtp_bt_input_session;
typedef enum {
 RTP_BT_OK=0,RTP_BT_INVALID_ARGUMENT=-1,RTP_BT_CAPACITY=-2,
 RTP_BT_INVALID_CONTROLLER=-3,RTP_BT_DUPLICATE_ID=-4,
 RTP_BT_DRIVER_CONFLICT=-5,RTP_BT_DRIVER_LIMIT=-6,
 RTP_BT_INPUT_NOT_ACTIVE=-7,RTP_BT_INPUT_DUPLICATE=-8
} rtp_bt_result;
rtp_bt_device_category rtp_bt_classify_device(unsigned int bluetooth_class);
int rtp_bt_set_device_category(rtp_bt_snapshot *snapshot,const char *device_id,rtp_bt_device_category category);
int rtp_bt_snapshot_build(const rtp_bt_controller_observation *observations,
                          size_t count,rtp_bt_snapshot *snapshot);
int rtp_bt_validate_driver_stack(unsigned int enabled_driver_mask,
                                 unsigned int *conflicting_mask);
const char *rtp_bt_device_kind_name(rtp_bt_device_kind kind);
const char *rtp_bt_device_category_name(rtp_bt_device_category category);
const char *rtp_bt_connection_state_name(rtp_bt_connection_state state);
const char *rtp_bt_provider_name(rtp_bt_provider provider);
/* Input learning stores observed controls only; it does not fabricate unsupported events. */
void rtp_bt_input_session_init(rtp_bt_input_session *session);
int rtp_bt_input_session_start(rtp_bt_input_session *session);
int rtp_bt_input_session_stop(rtp_bt_input_session *session);
int rtp_bt_input_record(rtp_bt_input_session *session,
                        const rtp_bt_input_observation *observation);
#ifdef __cplusplus
}
#endif
#endif
