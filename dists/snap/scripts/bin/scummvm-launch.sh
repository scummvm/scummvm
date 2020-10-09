#!/bin/sh

set -x

if [ -z "${XDG_CONFIG_HOME}" ]
then
  if [ -z "${HOME}" ]
  then XDG_CONFIG_HOME=$SNAP_USER_DATA/.config
  else XDG_CONFIG_HOME=${HOME}/.config
  fi
fi

# Hook up speech-dispatcher
mkdir -p $XDG_RUNTIME_DIR/speech-dispatcher
$SNAP/usr/bin/speech-dispatcher -d -C "$SNAP/etc/speech-dispatcher" -S "$XDG_RUNTIME_DIR/speech-dispatcher/speechd.sock" -m "$SNAP/usr/lib/speech-dispatcher-modules" -t 30

# Initial setup
if [ ! -f "${XDG_CONFIG_HOME}/scummvm/scummvm.ini" ]; then
  mkdir -p ${XDG_CONFIG_HOME}/scummvm/
  echo "[scummvm]\naspect_ratio=true\n"                                             >> ${XDG_CONFIG_HOME}/scummvm/scummvm.ini
  echo "[cloud]\nrootpath=/home/${USER}/snap/scummvm/current/.local/share/scummvm"  >> ${XDG_CONFIG_HOME}/scummvm/scummvm.ini
fi

# We need to do this for the user that launches scummvm, so
# it can't be done on installation
if [ ! -f "${XDG_CONFIG_HOME}/scummvm/.added-games-bundle" ]; then
  touch ${XDG_CONFIG_HOME}/scummvm/.added-games-bundle
  if ! grep -E "comi|drascula|dreamweb|lure|myst|queen|sky|sword" ${XDG_CONFIG_HOME}/scummvm/scummvm.ini
  then
  # Register the bundled games.
  $SNAP/bin/scummvm -p /usr/share/scummvm/ --recursive --add
  fi
fi

exec $SNAP/bin/scummvm "$@"