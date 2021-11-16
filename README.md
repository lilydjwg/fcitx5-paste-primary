This is a standalone module for fcitx to paste the primary selection with keyboard shortcut.

The shortcut is configurable, with default as <kdb>Ctrl-;</kdb>.

(Note that it won't trigger bracketed paste mode since text is directly commited from fcitx.)

Requires Fcitx >= 5.

For Wayland to work, fcitx5 must be started with the correct `WAYLAND_DISPLAY`
environment, and `wl-paste -p` must work.
