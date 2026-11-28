#ifndef RT_NATIVE_SETTINGS_INSTALLER_H
#define RT_NATIVE_SETTINGS_INSTALLER_H
#include <stddef.h>
typedef enum {
    RTNS_INSTALL_ERROR = -1,
    RTNS_INSTALL_CONFLICT = -2,
    RTNS_INSTALL_BACKUP_FAILED = -3,
    RTNS_INSTALL_IO_FAILED = -4,
    RTNS_INSTALL_CONFIG_AMBIGUOUS = -5,
    RTNS_INSTALL_DISABLED_ENTRY = -6,
    RTNS_INSTALL_OK = 0,
    RTNS_INSTALL_ALREADY = 1
} rtns_install_result;
/* enable_disabled is set only after the user explicitly confirms Install/Enable. */
rtns_install_result rt_settings_install_native(int enable_disabled,
                                                char *message,
                                                size_t message_capacity);
#endif
