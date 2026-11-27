#include <stdio.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

#define SCREEN_W 960
#define SCREEN_H 544
#define WHITE 0xffffffffu
#define MUTED 0xff9aa8b8u
#define CYAN 0xffffb84au
#define DARK 0xff101722u
#define PANEL 0xff1a2635u
#define GREEN 0xff65d69au
#define AMBER 0xffe6bd68u

typedef struct {
    const char *id;
    const char *name;
    const char *description;
    int installed;
} component_t;

static component_t components[] = {
    {"settings-core", "Rt Settings Core", "Native Sony Settings integration and shared framework.", 0},
    {"tai-saver", "Tai Saver", "taiHEN configuration snapshots, verification and recovery.", 0},
    {"plugin-manager", "Plugin Control Centre", "Plugin inventory, dependency and conflict inspection.", 0},
    {"storage", "Storage & Devices", "SD2Vita, memory card and mount diagnostics.", 0},
    {"network", "Network & Streaming", "FTP, Wi-Fi and USB streaming integrations.", 0},
    {"display", "Display & Audio", "Validated display, OLED/LCD and audio adapters.", 0},
    {"boot-visuals", "Boot Visuals", "Boot animation, splash and startup appearance adapters.", 0}
};
#define COMPONENT_COUNT (sizeof(components) / sizeof(components[0]))

static int page = 0;
static int selected = 0;
static int show_help = 0;
static vita2d_pgf *ui_font = NULL;

static int path_exists(const char *path) {
    SceIoStat st;
    memset(&st, 0, sizeof(st));
    return sceIoGetstat(path, &st) >= 0;
}

static void scan_components(void) {
    unsigned int i;
    char path[256];
    for (i = 0; i < COMPONENT_COUNT; ++i) {
        snprintf(path, sizeof(path),
                 "ur0:data/RtSettingsExtended/registry/modules/%s.json",
                 components[i].id);
        components[i].installed = path_exists(path);
    }
}

static void draw_text(float x, float y, unsigned int color, const char *s) {
    vita2d_pgf_draw_text(ui_font, (int)x, (int)y, color, 1.0f, s);
}

static void draw_shell(void) {
    unsigned int i;
    char line[160];
    int installed = 0;
    for (i = 0; i < COMPONENT_COUNT; ++i) {
        if (components[i].installed) ++installed;
    }

    vita2d_clear_screen();
    vita2d_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, DARK);
    vita2d_draw_rectangle(0, 0, SCREEN_W, 72, PANEL);
    vita2d_draw_rectangle(0, 70, SCREEN_W, 2, CYAN);
    draw_text(34, 45, CYAN, "RT / VITA COCKPIT");
    draw_text(720, 43, MUTED, "SYSTEM CONSOLE  |  3.65+");
    draw_text(34, 112, WHITE, "COMPONENT AUDIT");
    draw_text(34, 140, MUTED, "Read-only scan of Rt-owned component registry entries");

    snprintf(line, sizeof(line), "Detected components: %d / %d", installed, (int)COMPONENT_COUNT);
    draw_text(620, 112, GREEN, line);

    for (i = 0; i < COMPONENT_COUNT; ++i) {
        float y = 166.0f + (float)i * 48.0f;
        unsigned int bg = ((int)i == selected && page == 1) ? 0xff263c50u : PANEL;
        vita2d_draw_rectangle(30, y - 25, 900, 40, bg);
        draw_text(48, y, ((int)i == selected && page == 1) ? CYAN : WHITE, components[i].name);
        draw_text(610, y, components[i].installed ? GREEN : AMBER,
                  components[i].installed ? "INSTALLED" : "NOT DETECTED");
    }

    draw_text(34, 520, MUTED, "UP/DOWN: SELECT   CROSS: DETAILS   TRIANGLE: HELP   CIRCLE: RESCAN   START: EXIT");

    if (show_help) {
        vita2d_draw_rectangle(110, 125, 740, 300, 0xf0182635u);
        vita2d_draw_rectangle(110, 125, 740, 3, CYAN);
        draw_text(140, 170, CYAN, "ABOUT THIS SCREEN");
        draw_text(140, 215, WHITE, "This first Cockpit build performs a read-only component audit.");
        draw_text(140, 250, WHITE, "It does not edit taiHEN config, install plugins or change boot files.");
        draw_text(140, 285, WHITE, "A component is detected only when its Rt registry record exists.");
        draw_text(140, 320, WHITE, "Press TRIANGLE to close this information overlay.");
        draw_text(140, 380, MUTED, "Installer, updater and module activation are separate milestones.");
    } else if (page == 1) {
        vita2d_draw_rectangle(110, 125, 740, 300, 0xf0182635u);
        vita2d_draw_rectangle(110, 125, 740, 3, CYAN);
        draw_text(140, 170, CYAN, components[selected].name);
        draw_text(140, 220, WHITE, components[selected].description);
        draw_text(140, 275, components[selected].installed ? GREEN : AMBER,
                  components[selected].installed ? "Registry entry detected." : "No registry entry detected.");
        draw_text(140, 330, MUTED, "This view is informational; no files are modified.");
        draw_text(140, 380, MUTED, "Press CROSS or CIRCLE to return.");
    }
}

int main(void) {
    SceCtrlData pad, old;
    memset(&old, 0, sizeof(old));
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    vita2d_init();
    ui_font = vita2d_load_default_pgf();
    if (!ui_font) { vita2d_fini(); sceKernelExitProcess(1); return 1; }
    vita2d_set_clear_color(DARK);
    scan_components();

    for (;;) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        unsigned int pressed = pad.buttons & ~old.buttons;
        old = pad;

        if (pressed & SCE_CTRL_START) break;
        if (pressed & SCE_CTRL_TRIANGLE) show_help = !show_help;
        if (!show_help && page == 0) {
            if ((pressed & SCE_CTRL_DOWN) && selected < (int)COMPONENT_COUNT - 1) ++selected;
            if ((pressed & SCE_CTRL_UP) && selected > 0) --selected;
            if (pressed & SCE_CTRL_CROSS) page = 1;
            if (pressed & SCE_CTRL_CIRCLE) scan_components();
        } else if (!show_help && page == 1) {
            if (pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) page = 0;
        }

        vita2d_start_drawing();
        vita2d_clear_screen();
        draw_shell();
        vita2d_end_drawing();
        vita2d_swap_buffers();
        sceKernelDelayThread(16000);
    }

    vita2d_free_pgf(ui_font);
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
