#include <stdio.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#include "tai_saver.h"
#include "native_settings_installer.h"

#define SCREEN_W 960
#define SCREEN_H 544
#define WHITE 0xffffffffu
#define MUTED 0xff9aa8b8u
#define CYAN 0xffffb84au
#define DARK 0xff101722u
#define PANEL 0xff1a2635u
#define GREEN 0xff65d69au
#define AMBER 0xffe6bd68u
#define RED 0xff6b6bffu
#define TAI_SOURCE "ur0:tai"
#define TAI_ARCHIVES "ur0:data/RtSettingsExtended/TaiSaver"
#define TAI_RETENTION 5U

typedef struct {
    const char *id;
    const char *name;
    const char *description;
    int installed;
    int bundled;
} component_t;

static component_t components[] = {
    {"settings-core", "Rt Settings Core", "Native Sony Settings integration and shared framework.", 0, 0},
    {"tai-saver", "Tai Saver", "Verified taiHEN configuration snapshots and protected recovery.", 0, 1},
    {"plugin-manager", "Plugin Control Centre", "Plugin inventory, dependency and conflict inspection.", 0, 0},
    {"storage", "Storage & Devices", "SD2Vita, memory card and mount diagnostics.", 0, 0},
    {"network", "Network & Streaming", "FTP, Wi-Fi and USB streaming integrations.", 0, 0},
    {"display", "Display & Audio", "Validated display, OLED/LCD and audio adapters.", 0, 0},
    {"boot-visuals", "Boot Visuals", "Boot animation, splash and startup appearance adapters.", 0, 0}
};
#define COMPONENT_COUNT (sizeof(components) / sizeof(components[0]))
#define MENU_COUNT (COMPONENT_COUNT + 2)

static int page = 0; /* 0 dashboard, 1 component details, 2 Tai Saver, 3 confirm, 4 result */
static int selected = 0;
static int show_help = 0;
static int tai_result = TS_ERR_ARGUMENT;
static int tai_verified = 0;
static int native_install_status = RTNS_INSTALL_ERROR;
static char native_install_message[192];
static ts_snapshot_result snapshot_result;
static ts_snapshot_info snapshot_info;
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
        if (components[i].bundled) {
            components[i].installed = 1;
            continue;
        }
        snprintf(path, sizeof(path),
                 "ur0:data/RtSettingsExtended/registry/modules/%s.json",
                 components[i].id);
        components[i].installed = path_exists(path);
    }
}

static void draw_text(float x, float y, unsigned int color, const char *s) {
    vita2d_pgf_draw_text(ui_font, (int)x, (int)y, color, 1.0f, s);
}

static void startup_animation(void) {
    int frame;
    for (frame = 0; frame < 42; ++frame) {
        float pulse = 18.0f + (float)(frame % 14) * 2.0f;
        int progress = (frame + 1) * 900 / 42;
        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, DARK);
        vita2d_draw_rectangle(0, 0, SCREEN_W, 3, CYAN);
        vita2d_draw_fill_circle(480.0f, 210.0f, pulse + 10.0f, 0xff1b3448u);
        vita2d_draw_fill_circle(480.0f, 210.0f, pulse, CYAN);
        vita2d_draw_fill_circle(480.0f, 210.0f, 10.0f, DARK);
        draw_text(365, 310, WHITE, "RT / VITA COCKPIT");
        draw_text(390, 345, MUTED, "INITIALIZING SYSTEM");
        vita2d_draw_rectangle(30, 470, 900, 5, PANEL);
        vita2d_draw_rectangle(30, 470, (float)progress, 5, CYAN);
        vita2d_end_drawing();
        vita2d_swap_buffers();
        sceKernelDelayThread(16000);
    }
}

static void run_tai_backup(void) {
    memset(&snapshot_result, 0, sizeof(snapshot_result));
    memset(&snapshot_info, 0, sizeof(snapshot_info));
    tai_verified = 0;
    tai_result = ts_create_snapshot(TAI_SOURCE, TAI_ARCHIVES,
                                    TS_ARCHIVE_AUTOMATIC, TAI_RETENTION,
                                    "Created by Rt:Vita Cockpit", &snapshot_result);
    if (tai_result == TS_OK) {
        tai_result = ts_verify_snapshot(snapshot_result.snapshot_path, &snapshot_info);
        if (tai_result == TS_OK) tai_verified = 1;
    }
    page = 4;
}

static void draw_header(const char *section) {
    vita2d_draw_rectangle(0, 0, SCREEN_W, 72, PANEL);
    vita2d_draw_rectangle(0, 70, SCREEN_W, 2, CYAN);
    draw_text(34, 45, CYAN, "RT / VITA COCKPIT");
    draw_text(700, 43, MUTED, section);
}

static void draw_shell(void) {
    unsigned int i;
    char line[160];
    int installed = 0;
    for (i = 0; i < COMPONENT_COUNT; ++i) if (components[i].installed) ++installed;

    vita2d_clear_screen();
    vita2d_draw_rectangle(0, 0, SCREEN_W, SCREEN_H, DARK);

    if (page == 0) {
        draw_header("SYSTEM CONSOLE  |  3.65+");
        draw_text(34, 112, WHITE, "SYSTEM OVERVIEW");
        draw_text(34, 140, MUTED, "Select a module or open the first integrated utility.");
        snprintf(line, sizeof(line), "Rt registry records: %d / %d", installed, (int)COMPONENT_COUNT);
        draw_text(620, 112, GREEN, line);
        for (i = 0; i < MENU_COUNT; ++i) {
            float y = 174.0f + (float)i * 42.0f;
            unsigned int bg = ((int)i == selected) ? 0xff263c50u : PANEL;
            vita2d_draw_rectangle(30, y - 25, 900, 36, bg);
            if (i < COMPONENT_COUNT) {
                draw_text(48, y, WHITE, components[i].name);
                draw_text(690, y, components[i].bundled ? CYAN : (components[i].installed ? GREEN : AMBER),
                          components[i].bundled ? "BUILT INTO COCKPIT" :
                          (components[i].installed ? "REGISTRY DETECTED" : "NOT DETECTED"));
            } else if (i == COMPONENT_COUNT) {
                draw_text(48, y, CYAN, "Tai Saver  /  Backup & Recovery");
                draw_text(690, y, path_exists(TAI_SOURCE) ? GREEN : RED,
                          path_exists(TAI_SOURCE) ? "SOURCE AVAILABLE" : "SOURCE MISSING");
            } else {
                draw_text(48, y, CYAN, "Native Sony Settings  /  Rt Manager Installer");
                draw_text(690, y, path_exists("ur0:tai/rtsettings.suprx") ? GREEN : AMBER,
                          path_exists("ur0:tai/rtsettings.suprx") ? "MODULE FILE PRESENT" : "NOT INSTALLED");
            }
        }
        draw_text(34, 520, MUTED, "UP/DOWN SELECT   CROSS OPEN   TRIANGLE HELP   CIRCLE RESCAN/BACK   START EXIT");
    } else if (page == 1) {
        draw_header("COMPONENT INFORMATION");
        draw_text(48, 150, CYAN, components[selected].name);
        draw_text(48, 205, WHITE, components[selected].description);
        draw_text(48, 270, components[selected].installed ? GREEN : AMBER,
                  components[selected].installed ? "Registry record detected." : "No registry record detected.");
        draw_text(48, 330, MUTED, "Detection is informational; this screen does not install or remove files.");
        draw_text(48, 520, MUTED, "CROSS / CIRCLE BACK");
    } else if (page == 2) {
        draw_header("TAI SAVER  /  PROTECTION");
        draw_text(48, 132, CYAN, "TAIHEN CONFIGURATION SNAPSHOTS");
        draw_text(48, 180, WHITE, "Source: ur0:tai/");
        draw_text(48, 215, WHITE, "Archive: ur0:data/RtSettingsExtended/TaiSaver/");
        draw_text(48, 260, MUTED, "Creates a complete file snapshot, hashes every file and verifies the archive.");
        draw_text(48, 292, MUTED, "Automatic retention: 5 snapshots. Permanent Known-Good / Failed archives are untouched.");
        draw_text(48, 345, GREEN, "READ-ONLY AGAINST LIVE TAI CONFIG: no plugin line is edited.");
        draw_text(48, 410, path_exists(TAI_SOURCE) ? GREEN : RED,
                  path_exists(TAI_SOURCE) ? "ur0:tai is present." : "ur0:tai was not found; backup is unavailable.");
        draw_text(48, 520, MUTED, "CROSS CREATE BACKUP   CIRCLE BACK");
    } else if (page == 3) {
        draw_header("TAI SAVER  /  CONFIRM");
        draw_text(48, 150, CYAN, "CREATE A VERIFIED BACKUP?");
        draw_text(48, 205, WHITE, "The current ur0:tai directory will be copied to the Rt Tai Saver archive.");
        draw_text(48, 245, WHITE, "No live configuration files will be changed or deleted.");
        draw_text(48, 300, MUTED, "The new Automatic snapshot is verified before success is reported.");
        draw_text(48, 355, AMBER, "Ensure ur0: has enough free space before continuing.");
        draw_text(48, 520, MUTED, "CROSS CONFIRM   CIRCLE CANCEL");
    } else if (page == 4) {
        draw_header("TAI SAVER  /  RESULT");
        if (tai_result == TS_OK && tai_verified) {
            draw_text(48, 145, GREEN, "BACKUP CREATED AND VERIFIED");
            snprintf(line, sizeof(line), "Snapshot: %s", snapshot_result.snapshot_id);
            draw_text(48, 195, WHITE, line);
            snprintf(line, sizeof(line), "Files: %u   Size: %llu bytes",
                     (unsigned)snapshot_info.file_count,
                     (unsigned long long)snapshot_info.total_bytes);
            draw_text(48, 235, WHITE, line);
            draw_text(48, 285, MUTED, snapshot_result.snapshot_path);
            draw_text(48, 345, GREEN, "Integrity verification completed successfully.");
        } else {
            draw_text(48, 145, RED, "BACKUP NOT COMPLETED");
            snprintf(line, sizeof(line), "Result: %s", ts_result_name((ts_result)tai_result));
            draw_text(48, 195, WHITE, line);
            draw_text(48, 250, MUTED, "The live tai configuration was not intentionally modified.");
            if (snapshot_result.warning[0]) draw_text(48, 300, AMBER, snapshot_result.warning);
        }
        draw_text(48, 520, MUTED, "CROSS / CIRCLE BACK");
    } else if (page == 5) {
        draw_header("NATIVE SONY SETTINGS  /  INSTALLER");
        draw_text(48, 125, CYAN, "RT:SETTINGS EXTENDED");
        draw_text(48, 170, WHITE, "Settings > Rt:Settings Extended > Preferences & Settings Management > Rt Manager");
        draw_text(48, 220, WHITE, "Module: app0:rtsettings.suprx");
        draw_text(48, 250, WHITE, "Destination: ur0:tai/rtsettings.suprx");
        draw_text(48, 280, WHITE, "taiHEN title section: *NPXS10015");
        draw_text(48, 335, MUTED, "Installs a user-mode SUPRX and adds only its own config line.");
        draw_text(48, 365, MUTED, "Creates a verified permanent Pre-Restore snapshot before changes.");
        draw_text(48, 405, GREEN, "Sony XML and all unrelated taiHEN entries are preserved.");
        draw_text(48, 520, MUTED, "CROSS INSTALL / ENABLE   CIRCLE BACK");
    } else if (page == 6) {
        draw_header("NATIVE SETTINGS  /  CONFIRM");
        draw_text(48, 145, CYAN, "INSTALL RT:SETTINGS EXTENDED?");
        draw_text(48, 195, WHITE, "The installer will verify the SUPRX payload and the existing taiHEN config.");
        draw_text(48, 235, WHITE, "A verified Pre-Restore snapshot is created before any live change.");
        draw_text(48, 275, WHITE, "Only *NPXS10015 receives ur0:tai/rtsettings.suprx.");
        draw_text(48, 325, MUTED, "A different existing module is never overwritten.");
        draw_text(48, 365, AMBER, "An existing disabled Rt line will be enabled because you confirmed.");
        draw_text(48, 520, MUTED, "CROSS CONFIRM INSTALL   CIRCLE CANCEL");
    } else if (page == 7) {
        draw_header("NATIVE SETTINGS  /  RESULT");
        if (native_install_status == RTNS_INSTALL_OK || native_install_status == RTNS_INSTALL_ALREADY) {
            draw_text(48, 145, GREEN, native_install_status == RTNS_INSTALL_ALREADY ? "RT SETTINGS ALREADY INSTALLED" : "RT SETTINGS INSTALLED");
        } else {
            draw_text(48, 145, RED, "INSTALLATION NOT COMPLETED");
        }
        draw_text(48, 205, WHITE, native_install_message);
        draw_text(48, 275, MUTED, "Restart the Vita before checking the new Sony Settings entry.");
        draw_text(48, 520, MUTED, "CROSS / CIRCLE BACK");
    }

    if (show_help) {
        vita2d_draw_rectangle(100, 120, 760, 310, 0xf0182635u);
        vita2d_draw_rectangle(100, 120, 760, 3, CYAN);
        draw_text(130, 165, CYAN, "RT COCKPIT HELP");
        draw_text(130, 215, WHITE, "Tai Saver copies ur0:tai into an Rt-owned archive.");
        draw_text(130, 250, WHITE, "Each snapshot includes a manifest and SHA-256 file hashes.");
        draw_text(130, 285, WHITE, "This first VPK supports create + verify only; restore UI follows later.");
        draw_text(130, 320, WHITE, "No taiHEN config entries or third-party plugin files are changed.");
        draw_text(130, 385, MUTED, "Press TRIANGLE to close this overlay.");
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
    startup_animation();

    for (;;) {
        sceCtrlPeekBufferPositive(0, &pad, 1);
        unsigned int pressed = pad.buttons & ~old.buttons;
        old = pad;

        if (page == 0 && (pressed & SCE_CTRL_START)) break;
        if (pressed & SCE_CTRL_TRIANGLE) show_help = !show_help;
        if (!show_help) {
            if (page == 0) {
                if ((pressed & SCE_CTRL_DOWN) && selected < (int)MENU_COUNT - 1) ++selected;
                if ((pressed & SCE_CTRL_UP) && selected > 0) --selected;
                if (pressed & SCE_CTRL_CROSS) {
                    if (selected == (int)COMPONENT_COUNT) page = 2;
                    else if (selected == (int)COMPONENT_COUNT + 1) page = 5;
                    else page = 1;
                }
                if (pressed & SCE_CTRL_CIRCLE) scan_components();
            } else if (page == 1) {
                if (pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) page = 0;
            } else if (page == 2) {
                if (pressed & SCE_CTRL_CROSS) page = 3;
                if (pressed & SCE_CTRL_CIRCLE) page = 0;
            } else if (page == 3) {
                if (pressed & SCE_CTRL_CROSS) run_tai_backup();
                if (pressed & SCE_CTRL_CIRCLE) page = 2;
            } else if (page == 4) {
                if (pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) page = 2;
            } else if (page == 5) {
                if (pressed & SCE_CTRL_CROSS) page = 6;
                if (pressed & SCE_CTRL_CIRCLE) page = 0;
            } else if (page == 6) {
                if (pressed & SCE_CTRL_CROSS) {
                    native_install_status = rt_settings_install_native(1, native_install_message, sizeof(native_install_message));
                    page = 7;
                }
                if (pressed & SCE_CTRL_CIRCLE) page = 5;
            } else if (page == 7) {
                if (pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) page = 5;
            }
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
