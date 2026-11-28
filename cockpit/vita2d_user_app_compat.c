/*
 * libvita2d's static archive includes system-app-mode references even for a
 * normal LiveArea application. The public VitaSDK container does not ship
 * generated import stubs for these optional system-only APIs.
 *
 * Cockpit is a normal user application, never a Settings/system applet. This
 * compatibility unit deliberately forces libvita2d's normal-app path. The
 * shared-framebuffer functions are defensive link shims and must never be
 * reached; appmgr budget detection always reports "not a system app".
 */
#include <psp2/appmgr.h>
#include <psp2/sharedfb.h>
#include <psp2/types.h>

int sceAppMgrGetBudgetInfo(SceAppMgrBudgetInfo *info) {
    (void)info;
    return -1;
}

SceUID _sceSharedFbOpen(int index, int sysver) {
    (void)index;
    (void)sysver;
    return -1;
}

int sceSharedFbClose(SceUID fb_id) {
    (void)fb_id;
    return -1;
}

int sceSharedFbBegin(SceUID fb_id, SceSharedFbInfo *info) {
    (void)fb_id;
    (void)info;
    return -1;
}

int sceSharedFbEnd(SceUID fb_id) {
    (void)fb_id;
    return -1;
}

int sceSharedFbGetInfo(SceUID fb_id, SceSharedFbInfo *info) {
    (void)fb_id;
    (void)info;
    return -1;
}
