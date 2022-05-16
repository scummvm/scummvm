#!/bin/bash
#
# .dists/emscripten/build.sh -- Sets up an emscripten build environment and builds ScummVM for webassembly
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# print commands
if [[ "$_verbose" = true ]]; then
  set -o xtrace
fi

# exit when any command fails
set -e

EMSDK_VERSION="3.1.8"
ROOT_FOLDER=$(pwd)
DIST_FOLDER="$ROOT_FOLDER/dists/emscripten"
LIBS_FOLDER="$DIST_FOLDER/libs"
TASKS=()
CONFIGURE_ARGS=()
_bundle_games=()
_verbose=false
EMSCRIPTEN_VERSION=$EMSDK_VERSION

usage="\
Usage: ./dists/emscripten/build.sh [TASKS] [OPTIONS]

Output the configuration name of the system \`$me' is run on.

Tasks: 
  (space separated) List of tasks to run. See ./dists/emscripten/README.md for details.    

Options:
  -h, --help         print this help, then exit
  --bundle-games=    comma-separated list of demos and freeware games to bundle. 
  -v, --verbose          print all commands run by the script
  --*                all other options are passed on to the configure script
"

# parse inputs
for i in "$@"; do
  case $i in
  --bundle-games=*)
    str="${i#*=}"
    _bundle_games="${str//,/ }"
    shift # past argument=value
    ;;
  -h | --help)
    echo "$usage"
    exit
    ;;
  -v | --verbose)
    _verbose=true
    ;;
  -* | --*)
    CONFIGURE_ARGS+=" $i"
    ;;
  *)
    TASKS+="|$i" # save positional arg
    shift        # past argument
    ;;
  esac
done

TASKS="${TASKS:1}"
if [[ -z "$TASKS" ]]; then
  echo "$usage"
  exit
fi

#################################
# Setup Toolchain
#################################

if [[ "setup" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then

  # Activate Emscripten
  if [[ ! -d "$DIST_FOLDER/emsdk-$EMSDK_VERSION" ]]; then
    echo "$DIST_FOLDER/emsdk-$EMSDK_VERSION not found. Installing Emscripten"
    cd "$DIST_FOLDER"
    if [[ "$EMSDK_VERSION" = "tot" ]]; then
      git clone "https://github.com/emscripten-core/emsdk/" emsdk-tot
    else
      wget -nc --content-disposition "https://github.com/emscripten-core/emsdk/archive/refs/tags/${EMSDK_VERSION}.tar.gz"
      tar -xf "emsdk-${EMSDK_VERSION}.tar.gz"
    fi
    cd "$DIST_FOLDER/emsdk-${EMSDK_VERSION}"
    ./emsdk install ${EMSCRIPTEN_VERSION}
    # We currently require a few patches for unreleased changes in SDL2 and Emscripten
    if [[ "$EMSCRIPTEN_VERSION" == "3.1.8" ]]; then
      echo "Patching Emscripten"
      cd upstream/emscripten
      # until https://github.com/emscripten-core/emscripten/pull/15893 gets merged and released, we need to manually patch it
      # as the upstream PR has been rebased against an incompatible main,  we'll actually use https://github.com/chkuendig/emscripten/pull/2
      wget -nc https://github.com/chkuendig/emscripten/pull/2.patch  -O "$DIST_FOLDER/emscripten-15893.patch" || true 
      patch -p1 --verbose <"$DIST_FOLDER/emscripten-15893.patch"
      # until https://github.com/emscripten-core/emscripten/pull/16559 gets merged and released, we need to manually patch it
      wget -nc https://github.com/emscripten-core/emscripten/pull/16559.patch  -O "$DIST_FOLDER/emscripten-16559.patch" || true 
      patch -p1 --verbose <"$DIST_FOLDER/emscripten-16559.patch"
      # until https://github.com/emscripten-core/emscripten/pull/16687 gets merged and released, we need to manually patch it
      wget -nc https://github.com/emscripten-core/emscripten/pull/16559.patch  -O "$DIST_FOLDER/emscripten-16559.patch" || true 
      patch -p1 --verbose <"$DIST_FOLDER/emscripten-16687.patch"
    fi

    cd "$DIST_FOLDER/emsdk-${EMSDK_VERSION}"
    ./emsdk activate ${EMSCRIPTEN_VERSION}

    # install some required npm packages
    source "$DIST_FOLDER/emsdk-$EMSDK_VERSION/emsdk_env.sh"
    EMSDK_NPM=$(dirname $EMSDK_NODE)/npm
    export NODE_PATH=$(dirname $EMSDK_NODE)/../lib/node_modules/
    "$EMSDK_NPM" -g install "puppeteer@13.5.1"
    "$EMSDK_NPM" -g install "request@2.88.2"
    "$EMSDK_NPM" -g install "node-static@0.7.11"
  fi
fi
source "$DIST_FOLDER/emsdk-$EMSDK_VERSION/emsdk_env.sh"

# export node_path - so we can use all node_modules bundled with emscripten (e.g. requests)
EMSDK_NPM=$(dirname $EMSDK_NODE)/npm
export NODE_PATH=$(dirname $EMSDK_NODE)/../lib/node_modules/
LIBS_FLAGS=""

cd "$ROOT_FOLDER"
#################################
# Clean
#################################
if [[ "clean" =~ $(echo ^\(${TASKS}\)$) ]]; then
  emmake make clean || true
  emmake make distclean || true
  rm -rf ./dists/emscripten/libs/build || true
  rm -rf ./dists/emscripten/libs/*/ || true
  rm -rf ./build-emscripten/ || true
  rm scummvm.debug.wasm || true
  find . -name "*.o" || true
  find . -name "*.a" || true
  find . -name "*.wasm" || true
  exit 0
fi

#################################
# Download + Install Libraries
#################################
if [[ "libs" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then

  if [[ ! -d "$LIBS_FOLDER/build" ]]; then
    mkdir -p "$LIBS_FOLDER/build"
  fi

  # Emscripten has an official port for vorbis, but it doesn't properly link vorbisfile https://github.com/emscripten-core/emscripten/pull/14005
  if [[ ! -f "$LIBS_FOLDER/build/lib/libvorbis.a" ]]; then
    echo "building libvorbis-1.3.7"
    cd "$LIBS_FOLDER"
    wget -nc "https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz"
    tar -xf libvorbis-1.3.7.tar.gz
    cd "$LIBS_FOLDER/libvorbis-1.3.7"
    CFLAGS="-fPIC -s USE_OGG=1" emconfigure ./configure --host=wasm32-unknown-none --build=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/"
    emmake make -j 3
    emmake make install
  fi
  LIBS_FLAGS="${LIBS_FLAGS} --with-vorbis-prefix=$LIBS_FOLDER/build/"

  if [[ ! -f "$LIBS_FOLDER/build/lib/libtheora.a" ]]; then
    echo "build libtheora-1.1.1"
    cd "$LIBS_FOLDER"
    wget -nc "https://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.xz"
    tar -xf libtheora-1.1.1.tar.xz
    cd "$LIBS_FOLDER/libtheora-1.1.1/"
    CFLAGS="-fPIC -s USE_OGG=1" emconfigure ./configure --host=wasm32-unknown-none --build=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/" --disable-asm
    emmake make -j 3
    emmake make install
  fi
  LIBS_FLAGS="${LIBS_FLAGS} --with-theoradec-prefix=$LIBS_FOLDER/build/"

  if [[ ! -f "$LIBS_FOLDER/build/lib/libfaad.a" ]]; then
    echo "building faad2-2.8.8"
    cd "$LIBS_FOLDER"
    wget -nc "https://sourceforge.net/projects/faac/files/faad2-src/faad2-2.8.0/faad2-2.8.8.tar.gz"
    tar -xf faad2-2.8.8.tar.gz
    cd "$LIBS_FOLDER/faad2-2.8.8/"
    CFLAGS="-fPIC" emconfigure ./configure --host=wasm32-unknown-none --build=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/"
    emmake make
    emmake make install
  fi
  LIBS_FLAGS="${LIBS_FLAGS} --with-faad-prefix=$LIBS_FOLDER/build/"

  if [[ ! -f "$LIBS_FOLDER/build/lib/libmad.a" ]]; then
    echo "building libmad-0.15.1b"
    cd "$LIBS_FOLDER"
    wget -nc "https://downloads.sourceforge.net/mad/libmad-0.15.1b.tar.gz"
    # libmad needs patching: https://stackoverflow.com/questions/14015747/gccs-fforce-mem-option
    wget -nc "http://www.linuxfromscratch.org/patches/blfs/svn/libmad-0.15.1b-fixes-1.patch" -O "$DIST_FOLDER/libmad-0.15.1b-fixes-1.patch" || true
    rm -rf "$LIBS_FOLDER/libmad-0.15.1b/"
    tar -xf libmad-0.15.1b.tar.gz
    cd "$LIBS_FOLDER/libmad-0.15.1b/"
    patch -Np1 -i "$DIST_FOLDER/libmad-0.15.1b-fixes-1.patch"
    emconfigure ./configure --host=wasm32-unknown-none --build=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/" --with-pic --enable-fpm=no
    emmake make
    emmake make install
  fi
  LIBS_FLAGS="${LIBS_FLAGS} --with-mad-prefix=$LIBS_FOLDER/build/"

  if [[ ! -f "$LIBS_FOLDER/build/lib/libmpeg2.a" ]]; then
    echo "building libmpeg2-0.5.1"
    cd "$LIBS_FOLDER"
    wget -nc "http://libmpeg2.sourceforge.net/files/libmpeg2-0.5.1.tar.gz"
    tar -xf libmpeg2-0.5.1.tar.gz
    cd "$LIBS_FOLDER/libmpeg2-0.5.1/"
    CFLAGS="-fPIC" emconfigure ./configure --host=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/" --disable-sdl
    emmake make
    emmake make install
  fi
  LIBS_FLAGS="${LIBS_FLAGS} --with-mpeg2-prefix=$LIBS_FOLDER/build/"

  if [[ ! -f "$LIBS_FOLDER/build/lib/liba52.a" ]]; then
    echo "building a52dec-0.7.4"
    cd "$LIBS_FOLDER"
    wget -nc "https://liba52.sourceforge.io/files/a52dec-0.7.4.tar.gz"
    tar -xf a52dec-0.7.4.tar.gz
    cd "$LIBS_FOLDER/a52dec-0.7.4/"
    CFLAGS="-fPIC" emconfigure ./configure --host=wasm32-unknown-none --build=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/"
    emmake make -j 3
    emmake make install
  fi
  LIBS_FLAGS="${LIBS_FLAGS} --with-a52-prefix=$LIBS_FOLDER/build/"
fi

#################################
# Configure
#################################
if [[ "configure" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then
  cd "${ROOT_FOLDER}"
  echo "Running configure"
  # TODO: Figure out how configure could guess the host
  emconfigure ./configure --host=wasm32-unknown-emscripten --build=wasm32-unknown-emscripten ${CONFIGURE_ARGS} ${LIBS_FLAGS}

  # TODO: configure currently doesn't clean up all files it creates
  rm scummvm-conf.*
fi

#################################
# Make / Compile
#################################
if [[ "make" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then
  cd "${ROOT_FOLDER}"
  echo "Running make"
  emmake make
  emmake make dist-generic
fi

# The following steps copy stuff to build-emscripten:
mkdir -p "${ROOT_FOLDER}/build-emscripten/"

#################################
# Create Games & Testbed Data
#################################
if [[ "games" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then
  cd "${ROOT_FOLDER}"
  echo "Creating Games + Testbed Data"
  mkdir -p "${ROOT_FOLDER}/build-emscripten/games/"

  if [[ "testbed" =~ $(echo ^\(${_bundle_games// /|}\)$) ]]; then
    _bundle_games="${_bundle_games//testbed/}"
    rm -rf "${ROOT_FOLDER}/build-emscripten/games/testbed"
    cd "${ROOT_FOLDER}/dists/engine-data"
    ./create-testbed-data.sh
    mv testbed "${ROOT_FOLDER}/build-emscripten/games/testbed"
  fi

  if [ -n "$_bundle_games" ]; then
    mkdir -p "${DIST_FOLDER}/games/"
    cd "${DIST_FOLDER}/games/"
    files=$("$EMSDK_NODE" --unhandled-rejections=strict --trace-warnings "$DIST_FOLDER/build-download_games.js" ${_bundle_games})
    for dir in "${ROOT_FOLDER}/build-emscripten/games/"*; do # cleanup games folder
      if [ $(basename $dir) != "testbed" ]; then
        rm -rf "$dir"
      fi
    done
    for f in $files; do # unpack into games folder
      echo "Unzipping $f ..."
      unzip -q -n "$f" -d "${ROOT_FOLDER}/build-emscripten/games/${f%.zip}"
      # some zip files have weird permissions, this fixes that:
      find "${ROOT_FOLDER}/build-emscripten/games/${f%.zip}" -type d -exec chmod 0755 {} \;
      find "${ROOT_FOLDER}/build-emscripten/games/${f%.zip}" -type f -exec chmod 0644 {} \;
    done
  fi
  cd "${ROOT_FOLDER}/build-emscripten/games/"
  "$EMSDK_NODE" "$DIST_FOLDER/build-make_http_index.js" >index.json
fi

#################################
# Bundle everything into a neat package
#################################
if [[ "dist" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then
  cd "${ROOT_FOLDER}"
  mv "${ROOT_FOLDER}"/scummvm.* "${ROOT_FOLDER}"/build-emscripten/ || true

  # prepare data
  if [[ -d "${ROOT_FOLDER}/dist-generic/scummvm/data" ]]; then
    echo "Bundle ScummVM + Data"
    rm -rf "${ROOT_FOLDER}/build-emscripten/data"
    mv "${ROOT_FOLDER}/dist-generic/scummvm/data" "${ROOT_FOLDER}/build-emscripten/"
    cd "${ROOT_FOLDER}/build-emscripten/data"
    "$EMSDK_NODE" "$DIST_FOLDER/build-make_http_index.js" >index.json
    rm -rf "${ROOT_FOLDER}/dist-generic/"
  fi

  # bundle plugins
  echo "Bundle Plugins"
  mkdir -p "${ROOT_FOLDER}/build-emscripten/plugins"
  mv "${ROOT_FOLDER}/plugins/"* "${ROOT_FOLDER}/build-emscripten/plugins/" || true
  cd "${ROOT_FOLDER}/build-emscripten/plugins"
  "$EMSDK_NODE" "$DIST_FOLDER/build-make_http_index.js" >index.json

  # add logos and other assets
  cd "${ROOT_FOLDER}"
  cp "$DIST_FOLDER/assets/"* "${ROOT_FOLDER}/build-emscripten/"
  cp "$ROOT_FOLDER/gui/themes/common-svg/logo.svg" "${ROOT_FOLDER}/build-emscripten/"
  cp "$ROOT_FOLDER/icons/scummvm.ico" "${ROOT_FOLDER}/build-emscripten/favicon.ico"
fi

#################################
# Add icons
#################################
if [[ "icons" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then

  if [[ -d "${ROOT_FOLDER}/../scummvm-icons/" ]]; then
    echo "Adding files from icons repository "
    cp "${ROOT_FOLDER}/gui/themes/gui-icons.dat" "${ROOT_FOLDER}/build-emscripten/data"
    cd "${ROOT_FOLDER}/../scummvm-icons/"
    ${EMSDK_PYTHON:-'python3'} gen-set.py
    echo "add icons"
    zip -q -u "${ROOT_FOLDER}/build-emscripten/data/gui-icons.dat" icons/*
    echo "add xml"
    zip -q -u "${ROOT_FOLDER}/build-emscripten/data/gui-icons.dat" *.xml
    echo "update index"
    cd "${ROOT_FOLDER}/build-emscripten/data"
    "$EMSDK_NODE" "$DIST_FOLDER/build-make_http_index.js" >index.json
  else
    echo "Icons repository not found"
  fi

fi

#################################
# Automatically detect games and create scummvm.ini file
#################################
if [[ "add-games" =~ $(echo ^\(${TASKS}\)$) || "build" =~ $(echo ^\(${TASKS}\)$) ]]; then
  cd "${ROOT_FOLDER}"
  cp "$DIST_FOLDER/assets/scummvm.ini" "${ROOT_FOLDER}/build-emscripten/"
  cd "${ROOT_FOLDER}/build-emscripten/"
  "$EMSDK_NODE" "$DIST_FOLDER/build-add_games.js"
fi

#################################
# Run Development Server
#################################
if [[ "run" =~ $(echo ^\(${TASKS}\)$) ]]; then
  echo "Run ScummVM"
  cd "${ROOT_FOLDER}/build-emscripten/"
  # emrun doesn't support range requests. Once it will, we don't need node-static anymore
  # emrun --browser=chrome scummvm.html

  EMSDK_NPX=$(dirname $EMSDK_NODE)/npx
  $EMSDK_NPX -p node-static static .
fi
