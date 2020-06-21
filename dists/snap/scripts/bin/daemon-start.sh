#!/bin/sh

if [ "$(id -u)" = "0" ]  && [ "$(snapctl get daemon)" = "false" ]
then
  # If not configured to run as a daemon we have to stop here
  # (There's no "snapctl disable ...")
  snapctl stop $SNAP_NAME.daemon
  exit 0
fi

mkdir -p "$XDG_RUNTIME_DIR" -m 700

if [ -z "${WAYLAND_DISPLAY}" ]
then WAYLAND_DISPLAY=wayland-0
fi

real_wayland=$(dirname "$XDG_RUNTIME_DIR")/${WAYLAND_DISPLAY}
while [ ! -O "${real_wayland}" ]; do echo waiting for Wayland socket; sleep 4; done

ln -sf "${real_wayland}" "$XDG_RUNTIME_DIR"

exec "$@"