/***************************************************************************
 *   Copyright (C) 2019 by lilydjwg                                        *
 *   lilydjwg@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "fcitx/fcitx.h"

#include <libintl.h>
#include <errno.h>

#include "fcitx/module.h"
#include "fcitx-utils/utf8.h"
#include "fcitx-utils/uthash.h"
#include "fcitx-config/xdg.h"
#include "fcitx/hook.h"
#include "fcitx/ui.h"
#include "fcitx-utils/log.h"
#include "fcitx/instance.h"
#include "fcitx/context.h"
#include "fcitx-utils/utils.h"
#include "fcitx/module/x11/fcitx-x11.h"
#include "paste-primary.h"

#define _(x) dgettext("fcitx-paste-primary", (x))

typedef struct _FcitxPasteprimary {
    FcitxGenericConfig gconfig;
    FcitxHotkey hotkey[2];
    FcitxInstance* owner;
} FcitxPasteprimary;

void* PasteprimaryCreate(FcitxInstance* instance);
void ReloadPasteprimary(void* arg);
boolean LoadPasteprimaryConfig(FcitxPasteprimary* pasteprimaryState);
static FcitxConfigFileDesc* GetPasteprimaryConfigDesc();
static void SavePasteprimaryConfig(FcitxPasteprimary* pasteprimaryState);
static INPUT_RETURN_VALUE HotkeyPasteprimary(void*);
static void _X11ClipboardConvertCb(void *owner, const char *sel_str, const char *tgt_str, int format, size_t nitems, const void *buff, void *data);

CONFIG_BINDING_BEGIN(FcitxPasteprimary)
CONFIG_BINDING_REGISTER("Pasteprimary", "Hotkey", hotkey)
CONFIG_BINDING_END()

FCITX_EXPORT_API
FcitxModule module = {
    PasteprimaryCreate,
    NULL,
    NULL,
    NULL,
    ReloadPasteprimary
};

FCITX_EXPORT_API
int ABI_VERSION = FCITX_ABI_VERSION;

void* PasteprimaryCreate(FcitxInstance* instance)
{
    FcitxPasteprimary* pasteprimaryState = fcitx_utils_malloc0(sizeof(FcitxPasteprimary));
    pasteprimaryState->owner = instance;
    if (!LoadPasteprimaryConfig(pasteprimaryState)) {
        free(pasteprimaryState);
        return NULL;
    }

    FcitxHotkeyHook hk;
    hk.arg = pasteprimaryState;
    hk.hotkey = pasteprimaryState->hotkey;
    hk.hotkeyhandle = HotkeyPasteprimary;

    FcitxInstanceRegisterHotkeyFilter(instance, hk);

    return pasteprimaryState;
}

static void _X11ClipboardConvertCb(
    void *owner, const char *sel_str, const char *tgt_str, int format,
    size_t nitems, const void *buff, void *data)
{
    FCITX_UNUSED(sel_str);
    FCITX_UNUSED(tgt_str);
    FCITX_UNUSED(data);
    if (format != 8)
        return;

    FcitxPasteprimary* pasteprimary = owner;
    FcitxInstance *instance = pasteprimary->owner;

    char* content = malloc(nitems + 1);
    memcpy(content, buff, nitems);
    content[nitems] = '\0';
    FcitxInstanceCommitString(instance, FcitxInstanceGetCurrentIC(instance),
                              content);
    free(content);
}

INPUT_RETURN_VALUE HotkeyPasteprimary(void* arg)
{
    FcitxPasteprimary* pasteprimaryState = (FcitxPasteprimary*) arg;
    FcitxInstance *instance = pasteprimaryState->owner;
    FcitxX11RequestConvertSelect(instance, "PRIMARY", NULL,
                                 pasteprimaryState, _X11ClipboardConvertCb,
                                 NULL, NULL);
    return IRV_DO_NOTHING;
}

boolean LoadPasteprimaryConfig(FcitxPasteprimary* pasteprimaryState)
{
    FcitxConfigFileDesc* configDesc = GetPasteprimaryConfigDesc();
    if (configDesc == NULL)
        return false;

    FILE *fp;
    char *file;
    fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-paste-primary.config", "r", &file);
    FcitxLog(DEBUG, "Load Config File %s", file);
    free(file);
    if (!fp) {
        if (errno == ENOENT)
            SavePasteprimaryConfig(pasteprimaryState);
    }

    FcitxConfigFile *cfile = FcitxConfigParseConfigFileFp(fp, configDesc);

    FcitxPasteprimaryConfigBind(pasteprimaryState, cfile, configDesc);
    FcitxConfigBindSync((FcitxGenericConfig*)pasteprimaryState);

    if (fp)
        fclose(fp);

    return true;
}

CONFIG_DESC_DEFINE(GetPasteprimaryConfigDesc, "fcitx-paste-primary.desc")

void SavePasteprimaryConfig(FcitxPasteprimary* pasteprimaryState)
{
    FcitxConfigFileDesc* configDesc = GetPasteprimaryConfigDesc();
    char *file;
    FILE *fp = FcitxXDGGetFileUserWithPrefix("conf", "fcitx-paste-primary.config", "w", &file);
    FcitxLog(DEBUG, "Save Config to %s", file);
    FcitxConfigSaveConfigFileFp(fp, &pasteprimaryState->gconfig, configDesc);
    free(file);
    if (fp)
        fclose(fp);
}

void ReloadPasteprimary(void* arg)
{
    FcitxPasteprimary* pasteprimaryState = (FcitxPasteprimary*) arg;
    LoadPasteprimaryConfig(pasteprimaryState);
}
