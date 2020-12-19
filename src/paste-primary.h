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

#ifndef _FCITX_MODULE_PASTE_PRIMARY_H_
#define _FCITX_MODULE_PASTE_PRIMARY_H_

#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/instance.h>

namespace fcitx {

FCITX_CONFIGURATION(PastePrimaryConfig,
                    KeyListOption hotkey{this,
                                         "Hotkey",
                                         _("Hotkey"),
                                         {Key("Control+semicolon")},
                                         KeyListConstrain()};);

class PastePrimary : public AddonInstance {
public:
    PastePrimary(Instance *instance);

    const fcitx::Configuration *getConfig() const override { return &config_; }

    void setConfig(const fcitx::RawConfig &config) override {
        config_.load(config);
        safeSaveAsIni(config_, "conf/paste-primary.conf");
    }

    void reloadConfig() override {
        readAsIni(config_, "conf/paste-primary.conf");
    }

    FCITX_ADDON_DEPENDENCY_LOADER(xcb, instance_->addonManager());

private:
    Instance *instance_;
    PastePrimaryConfig config_;
    std::unique_ptr<HandlerTableEntry<EventHandler>> event_;
    std::unique_ptr<HandlerTableEntryBase> primaryCallback_;
};

class PastePrimaryFactory : public AddonFactory {
    AddonInstance *create(fcitx::AddonManager *manager) override {
        return new PastePrimary(manager->instance());
    }
};

} // namespace fcitx

#endif
