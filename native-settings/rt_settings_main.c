#include <psp2/kernel/modulemgr.h>
#include <stdint.h>
#include <taihen.h>
#include "rt_settings_xml.h"

#define TAI_LIB_KERNEL 0xCAE9ACE6
#define TAI_LIB_PAF_MISC 0x3D643CE8
#define TAI_LIB_PAF_TOPLEVEL 0x4D9A9DD0
#define NID_LOAD_START_MODULE 0x2DCC4AFA
#define NID_STOP_UNLOAD_MODULE 0x2415F8A4
#define NID_PAF_LOAD_XML 0x19FE55A8
#define NID_PAF_INIT_PLUGIN_FUNCTIONS 0xF5354FEF
#define NID_PAF_GET_TEXT 0x19CEFDA7

static SceUID g_load_hook = -1;
static SceUID g_stop_hook = -1;
static SceUID g_xml_hook = -1;
static SceUID g_functions_hook = -1;
static SceUID g_text_hook = -1;
static tai_hook_ref_t g_load_ref;
static tai_hook_ref_t g_stop_ref;
static tai_hook_ref_t g_xml_ref;
static tai_hook_ref_t g_functions_ref;
static tai_hook_ref_t g_text_ref;
static SceUID g_settings_core_modid = -1;
static int g_page_hooks_ready = 0;

static char g_root_xml[RT_SETTINGS_XML_BUFFER_SIZE];
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

static void release_page_hooks(void) {
    g_page_hooks_ready = 0;
    if (g_xml_hook >= 0) { taiHookRelease(g_xml_hook, g_xml_ref); g_xml_hook = -1; }
    if (g_functions_hook >= 0) { taiHookRelease(g_functions_hook, g_functions_ref); g_functions_hook = -1; }
    if (g_text_hook >= 0) { taiHookRelease(g_text_hook, g_text_ref); g_text_hook = -1; }
}

static int xml_layout_hook(void *out, void *buffer, int size, void *a4) {
    if (g_page_hooks_ready && buffer && size > 0 && size < RT_SETTINGS_XML_BUFFER_SIZE) {
        size_t output_length = 0;
        rt_settings_xml_result result = rt_settings_xml_splice_root(
            (const char *)buffer, (size_t)size, g_root_xml, sizeof(g_root_xml), &output_length);
        if (result == RT_SETTINGS_XML_INSERTED && output_length <= 0x7fffffffU) {
            buffer = g_root_xml;
            size = (int)output_length;
        }
    }
    return TAI_CONTINUE(int, g_xml_ref, out, buffer, size, a4);
}

static int (*g_system_settings_button)(const char *id, int a2, void *a3);
static int rt_settings_button(const char *id, int a2, void *a3) {
    if (text_equal(id, "rt_manager_about")) return 0;
    return g_system_settings_button ? g_system_settings_button(id, a2, a3) : 0;
}

static int plugin_functions_hook(void *a1, int a2, uint32_t *functions) {
    int result = TAI_CONTINUE(int, g_functions_ref, a1, a2, functions);
    const char *plugin = a1 ? (const char *)((uint32_t *)a1)[1] : NULL;
    if (plugin && functions && contains_text(plugin, "system_settings_plugin") &&
        functions[6] != (uint32_t)rt_settings_button) {
        g_system_settings_button = (int (*)(const char *, int, void *))functions[6];
        if (g_system_settings_button) functions[6] = (uint32_t)rt_settings_button;
    }
    return result;
}

static const unsigned short RT_INSTALLED_TEXT[] =
    {'R','t',' ','M','a','n','a','g','e','r',' ','i','s',' ','i','n','s','t','a','l','l','e','d','.',
     ' ','O','n','l','y',' ','c','o','m','p','a','t','i','b','l','e',' ','R','t',' ','c','o','m','p','o','n','e','n','t','s',' ',
     'a','n','d',' ','t','h','e','i','r',' ','v','e','r','i','f','i','e','d',' ','b','a','c','k','e','n','d','s',' ','w','i','l','l',' ',
     'a','p','p','e','a','r',' ','a','s',' ','a','c','t','i','v','e',' ','c','o','n','t','r','o','l','s','.',0};
static const unsigned short RT_STATUS_HINT[] =
    {'S','h','o','w','s',' ','t','h','e',' ','i','n','s','t','a','l','l','a','t','i','o','n',' ','s','t','a','t','u','s',' ',
     'o','f',' ','t','h','e',' ','R','t',' ','S','e','t','t','i','n','g','s',' ','e','x','t','e','n','s','i','o','n','.',0};

static void *settings_text_hook(void *arg, char **message) {
    if (message && *message) {
        if (text_equal(*message, "rt_manager_installed")) return (void *)RT_INSTALLED_TEXT;
        if (text_equal(*message, "rt_manager_status_hint")) return (void *)RT_STATUS_HINT;
    }
    return TAI_CONTINUE(void *, g_text_ref, arg, message);
}

static int install_page_hooks(void) {
    /* Install the text and callback hooks first. The XML splice is the final
       gate: no Rt entry is exposed unless all three interfaces are available. */
    g_text_hook = taiHookFunctionImport(&g_text_ref, "SceSystemSettingsCore",
        TAI_LIB_PAF_TOPLEVEL, NID_PAF_GET_TEXT, settings_text_hook);
    if (g_text_hook < 0) goto fail;
    g_functions_hook = taiHookFunctionImport(&g_functions_ref, "SceSettings",
        TAI_LIB_PAF_TOPLEVEL, NID_PAF_INIT_PLUGIN_FUNCTIONS, plugin_functions_hook);
    if (g_functions_hook < 0) goto fail;
    g_xml_hook = taiHookFunctionImport(&g_xml_ref, "SceSettings",
        TAI_LIB_PAF_MISC, NID_PAF_LOAD_XML, xml_layout_hook);
    if (g_xml_hook < 0) goto fail;
    g_page_hooks_ready = 1;
    return 0;
fail:
    release_page_hooks();
    return -1;
}

static SceUID load_start_module_hook(const char *path, SceSize args, void *argp,
                                     int flags, SceKernelLMOption *option, int *status) {
    SceUID result = TAI_CONTINUE(SceUID, g_load_ref, path, args, argp, flags, option, status);
    if (result >= 0 && path && contains_text(path, "system_settings_core.suprx") &&
        result != g_settings_core_modid) {
        g_settings_core_modid = result;
        (void)install_page_hooks();
    }
    return result;
}

static int stop_unload_module_hook(SceUID modid, SceSize args, void *argp,
                                   int flags, SceKernelULMOption *option, int *status) {
    int result = TAI_CONTINUE(int, g_stop_ref, modid, args, argp, flags, option, status);
    if (result >= 0 && modid == g_settings_core_modid) {
        release_page_hooks();
        g_settings_core_modid = -1;
    }
    return result;
}

int module_start(SceSize argc, const void *args) {
    (void)argc; (void)args;
    g_load_hook = taiHookFunctionImport(&g_load_ref, "SceSettings",
        TAI_LIB_KERNEL, NID_LOAD_START_MODULE, load_start_module_hook);
    if (g_load_hook < 0) goto fail;
    g_stop_hook = taiHookFunctionImport(&g_stop_ref, "SceSettings",
        TAI_LIB_KERNEL, NID_STOP_UNLOAD_MODULE, stop_unload_module_hook);
    if (g_stop_hook < 0) goto fail;
    return SCE_KERNEL_START_SUCCESS;
fail:
    if (g_load_hook >= 0) { taiHookRelease(g_load_hook, g_load_ref); g_load_hook = -1; }
    if (g_stop_hook >= 0) { taiHookRelease(g_stop_hook, g_stop_ref); g_stop_hook = -1; }
    return SCE_KERNEL_START_FAILED;
}

int module_stop(SceSize argc, const void *args) {
    (void)argc; (void)args;
    release_page_hooks();
    if (g_load_hook >= 0) { taiHookRelease(g_load_hook, g_load_ref); g_load_hook = -1; }
    if (g_stop_hook >= 0) { taiHookRelease(g_stop_hook, g_stop_ref); g_stop_hook = -1; }
    g_settings_core_modid = -1;
    return SCE_KERNEL_STOP_SUCCESS;
}
