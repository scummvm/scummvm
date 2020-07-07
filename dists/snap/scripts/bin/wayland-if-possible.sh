#!/bin/sh

if [ -O "$XDG_RUNTIME_DIR/${WAYLAND_DISPLAY:-wayland-0}" ] && [ -e $SNAP_COMMON/wayland.connected ]; then export SDL_VIDEODRIVER=wayland; fi

exec "$@"
