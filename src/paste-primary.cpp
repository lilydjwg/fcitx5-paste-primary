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
#include <thread>
#include <sys/wait.h>
#include <spawn.h>
#include <unistd.h>

typedef std::function<void(const std::string)> WaylandPrimarySelectionCallback;

static void _run_thread(WaylandPrimarySelectionCallback callback) {
    int r;
    int pfds[2];
    r = pipe(pfds);
    if (r != 0) {
        FCITX_ERROR() << "failed to create pipe for wl-paste: " << errno;
        return;
    }

    pid_t pid;
    const char* const argv[] = {"wl-paste", "-p", "-t", "text", "--no-newline", NULL};
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_adddup2(&file_actions, pfds[1], 1);

    r = posix_spawnp(&pid, "wl-paste", &file_actions, NULL, const_cast<char * const *>(argv), environ);
    close(pfds[1]);
    posix_spawn_file_actions_destroy(&file_actions);
    if (r != 0) {
        close(pfds[0]);
        FCITX_ERROR() << "wl-paste failed with " << r;
        return;
    }

    char buf[4000];
    int n;
    std::string s;
    while ((n = read(pfds[0], buf, 4000)) != 0) {
        s.append(buf, n);
    }
    close(pfds[0]);
    int st;
    r = waitpid(pid, &st, 0);
    if (r < 0) {
        FCITX_ERROR() << "waitpid wl-paste failed with " << errno;
    }
    if (st != 0) {
        FCITX_ERROR() << "wl-paste failed with " << st;
    }
    callback(std::move(s));
}

static void _get_primary_wayland(WaylandPrimarySelectionCallback callback) {
    std::thread th(_run_thread, std::move(callback));
    th.detach();
}

fcitx::PastePrimary::PastePrimary(fcitx::Instance *instance)
    : instance_(instance) {
    event_ = instance->watchEvent(
        EventType::InputContextKeyEvent, EventWatcherPhase::PreInputMethod,
        [this](Event &event) {
            auto &keyEvent = static_cast<KeyEvent &>(event);
            if (!keyEvent.isRelease() &&
                keyEvent.key().checkKeyList(*config_.hotkey)) {
                auto ic = keyEvent.inputContext();

                if (stringutils::startsWith(ic->display(), "wayland:")) {
                    _get_primary_wayland(
                        [this, icRef = ic->watch()](std::string str) {
                            // Check if IC is still valid and has focus.
                            if (auto *ic = icRef.get(); ic && ic->hasFocus()) {
                                size_t start = 0;
                                size_t MAX_CHUNK_SIZE = 4000;
                                while (start < str.size()) {
                                    size_t chunk_size = std::min(MAX_CHUNK_SIZE, str.size() - start);
                                    ic->commitString(str.substr(start, chunk_size));
                                    start += chunk_size;
                                }
                            }
                        });
                    keyEvent.filterAndAccept();

                } else if (stringutils::startsWith(ic->display(), "x11:")) {
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
            }
        });
    reloadConfig();
}

FCITX_ADDON_FACTORY(fcitx::PastePrimaryFactory);
