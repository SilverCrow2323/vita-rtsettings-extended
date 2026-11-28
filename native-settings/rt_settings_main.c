#include <psp2/kernel/modulemgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <taihen.h>
#include "rt_settings_xml.h"
#include "rt_settings_runtime.h"

#define TAI_LIB_KERNEL 0xCAE9ACE6
#define TAI_LIB_PAF_MISC 0x3D643CE8
#define TAI_LIB_PAF_TOPLEVEL 0x4D9A9DD0
#define NID_LOAD_START_MODULE 0x2DCC4AFA
#define NID_STOP_UNLOAD_MODULE 0x2415F8A4
#define NID_PAF_LOAD_XML 0x19FE55A8
#define NID_PAF_INIT_PLUGIN_FUNCTIONS 0xF5354FEF
#define NID_PAF_GET_TEXT 0x19CEFDA7
#define RT_CONFIG_PATH "ur0:tai/config.txt"
#define RT_MODULE_PATH "ur0:tai/rtsettings.suprx"
#define RT_CONFIG_CAPACITY 65536

static SceUID g_load_hook[2] = {-1, -1};
static SceUID g_stop_hook = -1;
static SceUID g_xml_hook[2] = {-1, -1};
static SceUID g_functions_hook = -1;
static SceUID g_text_hook = -1;
static tai_hook_ref_t g_load_ref[2];
static tai_hook_ref_t g_stop_ref;
static tai_hook_ref_t g_xml_ref[2];
static tai_hook_ref_t g_functions_ref;
static tai_hook_ref_t g_text_ref;
static SceUID g_settings_core_modid = -1;
static int g_page_hooks_ready = 0;
static char g_root_xml[RT_SETTINGS_XML_BUFFER_SIZE];
static char g_config[RT_CONFIG_CAPACITY];
static unsigned short g_status_text[192];
static int (*g_system_settings_button)(const char *id, int a2, void *a3);

static int text_equal(const char *a, const char *b) {
    size_t i = 0;
    if (!a || !b) return 0;
    while (a[i] && b[i] && a[i] == b[i]) ++i;
    return a[i] == b[i];
}
static int contains_text(const char *text, const char *needle) {
    size_t i, n = 0;
    if (!text || !needle) return 0;
    while (needle[n]) ++n;
    for (i = 0; text[i]; ++i) {
        size_t j = 0;
        while (j < n && text[i + j] && text[i + j] == needle[j]) ++j;
        if (j == n) return 1;
    }
    return 0;
}
static int xml_hook_acts(int which) {
    if (g_xml_hook[which] < 0) return 0;
    if (g_xml_hook[which ^ 1] < 0) return 1;
    {
        struct _tai_hook_user *other = (struct _tai_hook_user *)g_xml_ref[which ^ 1];
        struct _tai_hook_user *hook = (struct _tai_hook_user *)g_xml_ref[which];
        int i;
        for (i = 0; i < 64 && hook->next; ++i) {
            hook = (struct _tai_hook_user *)hook->next;
            if (hook == other) return 0;
        }
    }
    return 1;
}
static void release_page_hooks(void) {
    g_page_hooks_ready = 0;
    g_system_settings_button = NULL;
    if (g_xml_hook[1] >= 0) { taiHookRelease(g_xml_hook[1], g_xml_ref[1]); g_xml_hook[1] = -1; }
    if (g_xml_hook[0] >= 0) { taiHookRelease(g_xml_hook[0], g_xml_ref[0]); g_xml_hook[0] = -1; }
    if (g_functions_hook >= 0) { taiHookRelease(g_functions_hook, g_functions_ref); g_functions_hook = -1; }
    if (g_text_hook >= 0) { taiHookRelease(g_text_hook, g_text_ref); g_text_hook = -1; }
}
static void maybe_splice_root(void **buffer, int *size) {
    if (g_page_hooks_ready && buffer && *buffer && size && *size > 0 &&
        *size < RT_SETTINGS_XML_BUFFER_SIZE) {
        size_t output_length = 0;
        rt_settings_xml_result result = rt_settings_xml_splice_root(
            (const char *)*buffer, (size_t)*size, g_root_xml, sizeof(g_root_xml), &output_length);
        if (result == RT_SETTINGS_XML_INSERTED && output_length <= 0x7fffffffU) {
            *buffer = g_root_xml;
            *size = (int)output_length;
        }
    }
}
static int xml_layout_hook_a(void *out, void *buffer, int size, void *a4) {
    if (xml_hook_acts(0)) maybe_splice_root(&buffer, &size);
    return TAI_CONTINUE(int, g_xml_ref[0], out, buffer, size, a4);
}
static int xml_layout_hook_b(void *out, void *buffer, int size, void *a4) {
    if (xml_hook_acts(1)) maybe_splice_root(&buffer, &size);
    return TAI_CONTINUE(int, g_xml_ref[1], out, buffer, size, a4);
}
static void status_to_wide(const char *ascii) {
    size_t i = 0;
    while (ascii[i] && i + 1 < sizeof(g_status_text)/sizeof(g_status_text[0])) {
        g_status_text[i] = (unsigned short)(unsigned char)ascii[i];
        ++i;
    }
    g_status_text[i] = 0;
}
static void refresh_runtime_status(void) {
    SceIoStat st;
    SceUID fd;
    size_t total = 0;
    int module_present = sceIoGetstat(RT_MODULE_PATH, &st) >= 0;
    char message[192];
    rt_settings_runtime_snapshot snapshot;
    if (sceIoGetstat(RT_CONFIG_PATH, &st) < 0 || st.st_size < 0 ||
        (uint64_t)st.st_size > sizeof(g_config)) {
        status_to_wide("taiHEN config: MISSING OR TOO LARGE");
        return;
    }
    fd = sceIoOpen(RT_CONFIG_PATH, SCE_O_RDONLY, 0);
    if (fd < 0) {
        status_to_wide("taiHEN config: UNREADABLE | SUPRX: checked separately");
        return;
    }
    while (total < (size_t)st.st_size) {
        int n = sceIoRead(fd, g_config + total, (SceSize)((size_t)st.st_size - total));
        if (n < 0) { total = sizeof(g_config) + 1; break; }
        if (n == 0) break;
        total += (size_t)n;
    }
    sceIoClose(fd);
    if (total != (size_t)st.st_size) {
        status_to_wide("taiHEN config: TOO LARGE / READ ERROR");
        return;
    }
    if (rt_settings_runtime_inspect(g_config, total, module_present, &snapshot) < 0) {
        status_to_wide("taiHEN config: INSPECTION ERROR");
        return;
    }
    if (snapshot.config_state == RT_SETTINGS_CONFIG_ACTIVE && module_present)
        snprintf(message, sizeof(message), "READY | taiHEN entry ACTIVE | SUPRX PRESENT");
    else if (snapshot.config_state == RT_SETTINGS_CONFIG_DISABLED)
        snprintf(message, sizeof(message), "ACTION NEEDED | taiHEN entry DISABLED");
    else if (snapshot.config_state == RT_SETTINGS_CONFIG_DUPLICATE_SECTION)
        snprintf(message, sizeof(message), "REVIEW REQUIRED | duplicate *NPXS10015 sections");
    else if (snapshot.config_state == RT_SETTINGS_CONFIG_DUPLICATE_ENTRY)
        snprintf(message, sizeof(message), "REVIEW REQUIRED | duplicate Rt plugin entries");
    else if (snapshot.config_state == RT_SETTINGS_CONFIG_SECTION_MISSING)
        snprintf(message, sizeof(message), "NOT CONFIGURED | *NPXS10015 section missing");
    else if (snapshot.config_state == RT_SETTINGS_CONFIG_ENTRY_MISSING)
        snprintf(message, sizeof(message), "NOT CONFIGURED | Rt plugin entry missing");
    else
        snprintf(message, sizeof(message), "CONFIG %s | SUPRX %s",
                 rt_settings_config_state_name(snapshot.config_state),
                 module_present ? "PRESENT" : "MISSING");
    if (snapshot.config_state == RT_SETTINGS_CONFIG_ACTIVE && !module_present)
        snprintf(message, sizeof(message), "ACTION NEEDED | taiHEN entry ACTIVE | SUPRX MISSING");
    status_to_wide(message);
}
static int rt_settings_button(const char *id, int a2, void *a3) {
    if (text_equal(id, "rt_manager_about")) {
        refresh_runtime_status();
        return 0;
    }
    return g_system_settings_button ? g_system_settings_button(id, a2, a3) : 0;
}
static int plugin_functions_hook(void *a1, int a2, uint32_t *functions) {
    int result = TAI_CONTINUE(int, g_functions_ref, a1, a2, functions);
    const char *plugin = a1 ? (const char *)((uint32_t *)a1)[1] : NULL;
    if (plugin && functions && text_equal(plugin, "system_settings_plugin") &&
        functions[6] != (uint32_t)rt_settings_button) {
        g_system_settings_button = (int (*)(const char *, int, void *))functions[6];
        if (g_system_settings_button) functions[6] = (uint32_t)rt_settings_button;
    }
    return result;
}
static const unsigned short RT_STATUS_HINT[] =
    {'C','h','e','c','k','s',' ','t','h','e',' ','l','i','v','e',' ','t','a','i','H','E','N',' ','e','n','t','r','y',' ',
     'a','n','d',' ','R','t',' ','S','U','P','R','X',' ','f','i','l','e','.',' ', 'R','e','a','d','-','o','n','l','y','.',0};
static void *settings_text_hook(void *arg, char **message) {
    if (message && *message) {
        if (text_equal(*message, "rt_manager_installed")) return (void *)g_status_text;
        if (text_equal(*message, "rt_manager_status_hint")) return (void *)RT_STATUS_HINT;
    }
    return TAI_CONTINUE(void *, g_text_ref, arg, message);
}
static int install_page_hooks(void) {
    if (g_page_hooks_ready) return 0;
    g_text_hook = taiHookFunctionImport(&g_text_ref, "SceSystemSettingsCore",
        TAI_LIB_PAF_TOPLEVEL, NID_PAF_GET_TEXT, settings_text_hook);
    if (g_text_hook < 0) goto fail;
    g_functions_hook = taiHookFunctionImport(&g_functions_ref, "SceSettings",
        TAI_LIB_PAF_TOPLEVEL, NID_PAF_INIT_PLUGIN_FUNCTIONS, plugin_functions_hook);
    if (g_functions_hook < 0) goto fail;
    g_xml_hook[0] = taiHookFunctionImport(&g_xml_ref[0], "SceSettings",
        TAI_LIB_PAF_MISC, NID_PAF_LOAD_XML, xml_layout_hook_a);
    if (g_xml_hook[0] < 0) goto fail;
    g_xml_hook[1] = taiHookFunctionImport(&g_xml_ref[1], "SceSettings",
        TAI_LIB_PAF_MISC, NID_PAF_LOAD_XML, xml_layout_hook_b);
    if (g_xml_hook[1] < 0) goto fail;
    g_page_hooks_ready = 1;
    return 0;
fail:
    release_page_hooks();
    return -1;
}
static void settings_core_loaded(SceUID result, const char *path) {
    if (result < 0 || !path || !contains_text(path, "system_settings_core.suprx") ||
        result == g_settings_core_modid) return;
    release_page_hooks();
    g_settings_core_modid = result;
    (void)install_page_hooks();
}
static SceUID load_start_module_hook_a(const char *path, SceSize args, void *argp,
                                       int flags, SceKernelLMOption *option, int *status) {
    SceUID result = TAI_CONTINUE(SceUID, g_load_ref[0], path, args, argp, flags, option, status);
    settings_core_loaded(result, path);
    return result;
}
static SceUID load_start_module_hook_b(const char *path, SceSize args, void *argp,
                                       int flags, SceKernelLMOption *option, int *status) {
    SceUID result = TAI_CONTINUE(SceUID, g_load_ref[1], path, args, argp, flags, option, status);
    settings_core_loaded(result, path);
    return result;
}
static int stop_unload_module_hook(SceUID modid, SceSize args, void *argp,
                                   int flags, SceKernelULMOption *option, int *status) {
    if (modid == g_settings_core_modid) {
        release_page_hooks();
        g_settings_core_modid = -1;
    }
    return TAI_CONTINUE(int, g_stop_ref, modid, args, argp, flags, option, status);
}
int module_start(SceSize argc, const void *args) {
    (void)argc; (void)args;
    status_to_wide("Rt Manager status has not been refreshed yet.");
    g_load_hook[0] = taiHookFunctionImport(&g_load_ref[0], "SceSettings",
        TAI_LIB_KERNEL, NID_LOAD_START_MODULE, load_start_module_hook_a);
    if (g_load_hook[0] < 0) goto fail;
    g_load_hook[1] = taiHookFunctionImport(&g_load_ref[1], "SceSettings",
        TAI_LIB_KERNEL, NID_LOAD_START_MODULE, load_start_module_hook_b);
    if (g_load_hook[1] < 0) goto fail;
    g_stop_hook = taiHookFunctionImport(&g_stop_ref, "SceSettings",
        TAI_LIB_KERNEL, NID_STOP_UNLOAD_MODULE, stop_unload_module_hook);
    if (g_stop_hook < 0) goto fail;
    return SCE_KERNEL_START_SUCCESS;
fail:
    if (g_load_hook[1] >= 0) { taiHookRelease(g_load_hook[1], g_load_ref[1]); g_load_hook[1] = -1; }
    if (g_load_hook[0] >= 0) { taiHookRelease(g_load_hook[0], g_load_ref[0]); g_load_hook[0] = -1; }
    if (g_stop_hook >= 0) { taiHookRelease(g_stop_hook, g_stop_ref); g_stop_hook = -1; }
    return SCE_KERNEL_START_FAILED;
}
int module_stop(SceSize argc, const void *args) {
    (void)argc; (void)args;
    release_page_hooks();
    if (g_load_hook[1] >= 0) { taiHookRelease(g_load_hook[1], g_load_ref[1]); g_load_hook[1] = -1; }
    if (g_load_hook[0] >= 0) { taiHookRelease(g_load_hook[0], g_load_ref[0]); g_load_hook[0] = -1; }
    if (g_stop_hook >= 0) { taiHookRelease(g_stop_hook, g_stop_ref); g_stop_hook = -1; }
    g_settings_core_modid = -1;
    return SCE_KERNEL_STOP_SUCCESS;
}
