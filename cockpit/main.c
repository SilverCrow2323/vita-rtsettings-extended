#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#include "tai_saver.h"
#include "native_settings_installer.h"
#include "plugin_control_centre.h"

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

static int page = 0; /* 0 dashboard, 1 component, 2 Tai Saver, 3 backup confirm, 4 backup result, 5-7 native Settings install flow */
static int selected = 0;
static int show_help = 0;
static int tai_result = TS_ERR_ARGUMENT;
static int tai_verified = 0;
static int native_install_status = RTNS_INSTALL_ERROR;
static char native_install_message[192];
static ts_snapshot_result snapshot_result;
static ts_snapshot_info snapshot_info;
static vita2d_pgf *ui_font = NULL;
static char plugin_config[65536];
static rt_pi_inventory plugin_inventory;
static rt_pi_adapter_registry plugin_adapters;
static rtp_plugin_centre_snapshot plugin_snapshot;
static int plugin_scan_status = -1;
static unsigned int plugin_scroll = 0;

static int path_exists(const char *path) {
    SceIoStat st;
    memset(&st, 0, sizeof(st));
    return sceIoGetstat(path, &st) >= 0;
}

static void scan_components(void) {
    unsigned int i;
    char path[256];
    for (i = 0; i < COMPONENT_COUNT; ++i) {
        if (strcmp(components[i].id, "plugin-manager") == 0) {
            components[i].installed = path_exists("ur0:tai/config.txt");
            continue;
        }
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

static int scan_plugin_inventory(void) {
    SceIoStat st;
    SceUID fd;
    size_t total = 0;
    int n;
    if (sceIoGetstat("ur0:tai/config.txt", &st) < 0 || st.st_size < 0 ||
        (uint64_t)st.st_size > sizeof(plugin_config)) {
        plugin_scan_status = -1;
        return plugin_scan_status;
    }
    fd = sceIoOpen("ur0:tai/config.txt", SCE_O_RDONLY, 0);
    if (fd < 0) { plugin_scan_status = -1; return plugin_scan_status; }
    while (total < (size_t)st.st_size) {
        n = sceIoRead(fd, plugin_config + total, (SceSize)((size_t)st.st_size - total));
        if (n <= 0) { sceIoClose(fd); plugin_scan_status = -1; return plugin_scan_status; }
        total += (size_t)n;
    }
    if (sceIoClose(fd) < 0) { plugin_scan_status = -1; return plugin_scan_status; }
    rt_pi_inventory_init(&plugin_inventory);
    rt_pi_adapter_registry_init(&plugin_adapters);
    if (rt_pi_parse(plugin_config, total, &plugin_inventory) != RT_PI_OK ||
        rtp_plugin_centre_build(&plugin_inventory, &plugin_adapters, &plugin_snapshot) != RTP_PC_OK) {
        plugin_scan_status = -1;
        return plugin_scan_status;
    }
    plugin_scroll = 0;
    plugin_scan_status = 0;
    return 0;
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
                if (i == 2) draw_text(690, y, components[i].installed ? GREEN : AMBER,
                                      components[i].installed ? "TAIHEN CONFIG FOUND" : "CONFIG MISSING");
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
        draw_text(48, 170, WHITE, "Settings > Rt:Settings Extended");
        draw_text(48, 200, WHITE, "> Preferences & Settings Management > Rt Manager");
        draw_text(48, 245, WHITE, "Module: app0:rtsettings.suprx");
        draw_text(48, 275, WHITE, "Destination: ur0:tai/rtsettings.suprx");
        draw_text(48, 305, WHITE, "taiHEN title section: *NPXS10015");
        draw_text(48, 360, MUTED, "Installs one user-mode SUPRX and adds only its own config line.");
        draw_text(48, 390, MUTED, "Creates a verified permanent Pre-Restore snapshot first.");
        draw_text(48, 430, GREEN, "Sony XML and unrelated taiHEN entries are preserved.");
        draw_text(48, 520, MUTED, "CROSS INSTALL / ENABLE   CIRCLE BACK");
    } else if (page == 6) {
        draw_header("NATIVE SETTINGS  /  CONFIRM");
        draw_text(48, 145, CYAN, "INSTALL RT:SETTINGS EXTENDED?");
        draw_text(48, 195, WHITE, "Verifies the SUPRX payload and current taiHEN config.");
        draw_text(48, 235, WHITE, "Creates a permanent Pre-Restore snapshot before changes.");
        draw_text(48, 275, WHITE, "Adds ur0:tai/rtsettings.suprx under *NPXS10015 only.");
        draw_text(48, 325, MUTED, "A different existing SUPRX is never overwritten.");
        draw_text(48, 365, AMBER, "A commented Rt entry is enabled only after this confirmation.");
        draw_text(48, 520, MUTED, "CROSS CONFIRM INSTALL   CIRCLE CANCEL");
    } else if (page == 8) {
        unsigned int i, visible = 0;
        draw_header("PLUGIN CONTROL CENTRE  /  READ ONLY");
        draw_text(48, 112, CYAN, "LIVE TAIHEN CONFIG INVENTORY");
        if (plugin_scan_status < 0) {
            draw_text(48, 175, RED, "Cannot read ur0:tai/config.txt or the file exceeds the safe scan limit.");
            draw_text(48, 225, MUTED, "No changes were made. Check storage access and rescan from Cockpit.");
        } else {
            snprintf(line, sizeof(line), "Entries: %u   Unknown: %u   Disabled: %u   Blocked: %u   Review: %u",
                     (unsigned)plugin_snapshot.count, (unsigned)plugin_snapshot.unknown_count,
                     (unsigned)plugin_snapshot.disabled_count, (unsigned)plugin_snapshot.blocked_count,
                     (unsigned)plugin_snapshot.review_count);
            draw_text(48, 145, WHITE, line);
            for (i = plugin_scroll; i < plugin_snapshot.count && visible < 7; ++i, ++visible) {
                const rtp_plugin_centre_row *r = &plugin_snapshot.rows[i];
                char display_path[80];
                size_t path_len = strlen(r->path);
                float y = 198.0f + (float)visible * 43.0f;
                if (path_len >= sizeof(display_path)) {
                    memcpy(display_path, "...", 3);
                    memcpy(display_path + 3, r->path + path_len - (sizeof(display_path) - 4), sizeof(display_path) - 4);
                    display_path[sizeof(display_path) - 1] = '\0';
                } else {
                    memcpy(display_path, r->path, path_len + 1);
                }
                unsigned int color = r->state == RTP_PC_ROW_BLOCKED ? RED :
                    (r->state == RTP_PC_ROW_REVIEW ? AMBER :
                    (r->state == RTP_PC_ROW_DISABLED ? MUTED : WHITE));
                draw_text(48, y, color, display_path);
                draw_text(760, y, color, rtp_plugin_centre_state_name(r->state));
                snprintf(line, sizeof(line), "%s  |  line %u  |  occurrence %u",
                         r->section, r->line_number, r->occurrence);
                draw_text(72, y + 17, MUTED, line);
            }
            if (!plugin_snapshot.count) draw_text(48, 205, MUTED, "No plugin entries were found in taiHEN config.");
            snprintf(line, sizeof(line), "Compatibility findings: %u blocking / %u review",
                     (unsigned)plugin_snapshot.blocking_findings, (unsigned)plugin_snapshot.review_findings);
            draw_text(48, 505, AMBER, line);
        }
        draw_text(48, 530, MUTED, "UP/DOWN SCROLL   CIRCLE BACK   READ ONLY — NO PLUGIN IS ENABLED OR DISABLED HERE");
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
                    if (selected == 2) { scan_plugin_inventory(); page = 8; }
                    else if (selected == (int)COMPONENT_COUNT) page = 2;
                    else if (selected == (int)COMPONENT_COUNT + 1) page = 5;
                    else page = 1;
                }
                if (pressed & SCE_CTRL_CIRCLE) scan_components();
            } else if (page == 8) {
                if (pressed & SCE_CTRL_UP) { if (plugin_scroll > 0) --plugin_scroll; }
                if (pressed & SCE_CTRL_DOWN) {
                    unsigned int max_scroll = plugin_snapshot.count > 7 ? (unsigned int)plugin_snapshot.count - 7 : 0;
                    if (plugin_scroll < max_scroll) ++plugin_scroll;
                }
                if (pressed & SCE_CTRL_CIRCLE) page = 0;
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
