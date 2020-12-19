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

#include "paste-primary.h"
#include "xcb_public.h"

fcitx::PastePrimary::PastePrimary(fcitx::Instance *instance)
    : instance_(instance) {
    event_ = instance->watchEvent(
        EventType::InputContextKeyEvent, EventWatcherPhase::PreInputMethod,
        [this](Event &event) {
            auto &keyEvent = static_cast<KeyEvent &>(event);
            if (!keyEvent.isRelease() &&
                keyEvent.key().checkKeyList(*config_.hotkey)) {
                auto ic = keyEvent.inputContext();
                // Check if we're X11
                if (!stringutils::startsWith(ic->display(), "x11:")) {
                    return;
                }

                // Fetch primary and do thing in callback.
                primaryCallback_ = xcb()->call<IXCBModule::convertSelection>(
                    ic->display().substr(4), "PRIMARY", "",
                    [this, icRef = ic->watch()](xcb_atom_t, const char *data,
                                                size_t length) {
                        if (data) {
                            // Check if IC is still valid and has focus.
                            if (auto *ic = icRef.get(); ic && ic->hasFocus()) {
                                std::string str(data, length);
                                ic->commitString(str);
                            }
                        }
                        // Clear the callback handler.
                        primaryCallback_.reset();
                    });
                keyEvent.filterAndAccept();
            }
        });
    reloadConfig();
}

FCITX_ADDON_FACTORY(fcitx::PastePrimaryFactory);
