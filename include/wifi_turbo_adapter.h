#ifndef RT_WIFI_TURBO_ADAPTER_H
#define RT_WIFI_TURBO_ADAPTER_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define RT_WT_HOST_MAX 63
#define RT_WT_DEFAULT_PORT 1339
#define RT_WT_DEFAULT_TIMEOUT_MS 2500
typedef enum { RT_WT_OK=0, RT_WT_PENDING_CONFIRM=1, RT_WT_INVALID=-1, RT_WT_UNAVAILABLE=-2, RT_WT_TRANSPORT_ERROR=-3, RT_WT_RESPONSE_TOO_LARGE=-4 } rt_wt_result;
typedef enum { RT_WT_TOGGLE_KEEP_WIFI=1, RT_WT_TOGGLE_HUD=2, RT_WT_TOGGLE_AUTO_RECONNECT=3, RT_WT_TOGGLE_ROAM=4 } rt_wt_toggle;
typedef enum { RT_WT_PROFILE_MAX_PERFORMANCE=1, RT_WT_PROFILE_BALANCED=2, RT_WT_PROFILE_BATTERY_SAVER=3 } rt_wt_profile;
/* One command per connection; transport must NUL-terminate the response. */
typedef int (*rt_wt_exchange_fn)(void *context,const char *host,unsigned short port,const char *command,unsigned int timeout_ms,char *response,size_t response_capacity);
typedef struct { char host[RT_WT_HOST_MAX+1]; unsigned short port; unsigned int timeout_ms; int developer_remote_control_enabled; rt_wt_exchange_fn exchange; void *context; } rt_wt_client;
int rt_wt_client_init(rt_wt_client *,const char *,unsigned short,unsigned int,int,rt_wt_exchange_fn,void *);
int rt_wt_get_status(rt_wt_client *,char *,size_t);
int rt_wt_set_toggle(rt_wt_client *,rt_wt_toggle,int,char *,size_t);
int rt_wt_set_profile(rt_wt_client *,rt_wt_profile,char *,size_t);
int rt_wt_set_ht40(rt_wt_client *,int,char *,size_t);
int rt_wt_confirm(rt_wt_client *,char *,size_t);
int rt_wt_revert(rt_wt_client *,char *,size_t);
int rt_wt_roam_now(rt_wt_client *,char *,size_t);
int rt_wt_connect_saved_networks(rt_wt_client *,char *,size_t);
#ifdef __cplusplus
}
#endif
#endif
