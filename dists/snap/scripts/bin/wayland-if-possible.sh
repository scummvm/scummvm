#!/bin/bash

# By default, we set SDL_VIDEODRIVER to 'wayland'.
if [ -O "$XDG_RUNTIME_DIR/${WAYLAND_DISPLAY:-wayland-0}" ] && [ -e $SNAP_COMMON/wayland.connected ]; then export SDL_VIDEODRIVER=wayland; fi

# Since GNOME doesn't have support for server-side decorations,
# we check if we are running from within a GNOME session, we
# enforce using the x11 backend for XWayland fallback support.
if [[ "$XDG_CURRENT_DESKTOP" =~ "GNOME" ]]; then
    export SDL_VIDEODRIVER=x11
fi

exec "$@"
