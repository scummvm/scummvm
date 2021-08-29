#!/bin/bash
# print commands
set -o xtrace
# exit when any command fails
set -e
if [ "$#" -ne 1 ]; then
  echo "$0: exactly 1 arguments expected: configure, make, data, all"
  exit 3
fi

EMSDK_VERSION="2.0.26"
ROOT_FOLDER=$(pwd)
DIST_FOLDER="$ROOT_FOLDER/dists/emscripten"
LIBS_FOLDER="$DIST_FOLDER/libs"
if [[ ! -d "$DIST_FOLDER" ]]; then
  echo "/dists/emscripten/ not found. Please make sure to run this script from the root of the project - ./dists/emscripten/build.sh "
  exit 1
fi
if [[ "$1" =~ ^(clean)$ ]]; then
  make clean
  make distclean
  rm -rf ./dists/emscripten/libs/build
  rm -rf ./dists/emscripten/libs/*/
  rm -rf ./dists/emscripten/emsdk*/
  rm scummvm.debug.wasm
  find . -name "*.o"
  find . -name "*.a"
  find . -name "*.wasm"
  exit 0
fi

# Activate Emscripten
if [[ ! -d "$DIST_FOLDER/emsdk-$EMSDK_VERSION" ]]; then
  echo "$DIST_FOLDER/emsdk-$EMSDK_VERSION not found. Installing Emscripten"
  cd "$DIST_FOLDER"
  wget -nc --content-disposition "https://github.com/emscripten-core/emsdk/archive/refs/tags/${EMSDK_VERSION}.tar.gz"
  tar xzvf "emsdk-${EMSDK_VERSION}.tar.gz"
  cd "emsdk-${EMSDK_VERSION}"
  ./emsdk install ${EMSDK_VERSION}
  ./emsdk activate ${EMSDK_VERSION}
fi
source "$DIST_FOLDER/emsdk-$EMSDK_VERSION/emsdk_env.sh"

# Download + Install Libraries
mkdir -p "$LIBS_FOLDER"
if [[ ! -d "$LIBS_FOLDER/build" ]]; then
  echo "$LIBS_FOLDER/build/ not found. Building plugins..."
  echo "build libtheora-1.1.1"
  cd "$LIBS_FOLDER"
  pwd
  wget -nc "https://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.xz"
  tar -xf libtheora-1.1.1.tar.xz
  cd "./libtheora-1.1.1/"
  CFLAGS="-fPIC -s USE_OGG=1 -s USE_VORBIS=1 " emconfigure ./configure --host=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/" --disable-asm
  emmake make -j 3
  emmake make install

  echo "building faad2-2.8.8"
  cd "$LIBS_FOLDER"
  wget -nc "https://sourceforge.net/projects/faac/files/faad2-src/faad2-2.8.0/faad2-2.8.8.tar.gz"
  tar -xf faad2-2.8.8.tar.gz
  cd "./faad2-2.8.8/"
  CFLAGS="-fPIC" emconfigure ./configure --host=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/"
  emmake make
  emmake make install

  echo "building libmad-0.15.1b"
  cd "$LIBS_FOLDER"
  # libmad needs patching: https://stackoverflow.com/questions/14015747/gccs-fforce-mem-option
  wget -nc "http://www.linuxfromscratch.org/patches/blfs/svn/libmad-0.15.1b-fixes-1.patch"
  wget -nc "https://downloads.sourceforge.net/mad/libmad-0.15.1b.tar.gz"
  tar -xf libmad-0.15.1b.tar.gz
  cd "$LIBS_FOLDER/libmad-0.15.1b/"
  patch -Np1 -i ../libmad-0.15.1b-fixes-1.patch
  emconfigure ./configure --host=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/" --with-pic --enable-fpm=no
  emmake make
  emmake make install

  echo "building libmpeg2-0.5.1"
  cd "$LIBS_FOLDER"
  wget -nc "http://libmpeg2.sourceforge.net/files/libmpeg2-0.5.1.tar.gz"
  tar -xf libmpeg2-0.5.1.tar.gz
  cd "$LIBS_FOLDER/libmpeg2-0.5.1/"

  CFLAGS="-fPIC" emconfigure ./configure --host=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/" --disable-sdl
  emmake make
  emmake make install

  echo "building a52dec-0.7.4"
  cd "$LIBS_FOLDER"
  wget -nc "https://liba52.sourceforge.io/files/a52dec-0.7.4.tar.gz"
  tar -xf a52dec-0.7.4.tar.gz
  cd "$LIBS_FOLDER/a52dec-0.7.4/"
  CFLAGS="-fPIC" emconfigure ./configure --host=wasm32-unknown-none --prefix="$LIBS_FOLDER/build/"
  emmake make -j 3
  emmake make install
fi

cd "$ROOT_FOLDER"

## Emscripten configuration (should probably go into the configure file)
## IMPORTANT: ASYNCIFY WITH -O0 doesnt work (presumably because the stack gets too big)
export LDFLAGS="-O2 -s ASSERTIONS=1 -s GL_ASSERTIONS=1 -s LLD_REPORT_UNDEFINED -s INITIAL_MEMORY=33554432"

#debugging
export LDFLAGS="${LDFLAGS} -g -gseparate-dwarf=scummvm.debug.wasm -s SEPARATE_DWARF_URL=\"http://localhost:8080/scummvm.debug.wasm\""

# linker flags (bundle JS and default assets)
export LDFLAGS_LINKER=" --pre-js ./dists/emscripten/pre.js --post-js ./dists/emscripten/post.js --shell-file ./dists/emscripten/custom_shell.html "

if [[ "$1" =~ ^(configure|all)$ ]]; then

  echo "clean, & configure"
  make clean || true
  emconfigure ./configure --enable-debug --enable-verbose-build --host=wasm32-unknown-emscripten \
    --disable-all-engines \
    --enable-engine=testbed,scumm,scumm_7_8,grim,monkey4,mohawk,myst,riven,sci32,agos2,sword2,drascula,sky,lure,queen,testbed \
    --with-theoradec-prefix="$LIBS_FOLDER/build/" \
    --with-faad-prefix="$LIBS_FOLDER/build/" \
    --with-mad-prefix="$LIBS_FOLDER/build/" \
    --with-mpeg2-prefix="$LIBS_FOLDER/build/" \
    --with-a52-prefix="$LIBS_FOLDER/build/"

  # TODO: enable dynamic linking so we can enable more plugins
  # https://forums.scummvm.org/viewtopic.php?t=14918
  # https://github.com/emscripten-core/emscripten/wiki/Linking
  # https://freecontent.manning.com/dynamic-linking-a-crash-course/
  # https://iandouglasscott.com/2019/07/18/experimenting-with-webassembly-dynamic-linking-with-clang/

  # HACK: the preload flags break emcc during configure as emcc enables NODERAWFS when run as part of configure
  # which doesn't support preloading assets, so we have to manually add those after configure to the config.mk file
  echo "LDFLAGS += ${LDFLAGS_LINKER}" >>config.mk

  # configure currently doesn't clean up all files it created
  rm scummvm-conf.*

fi

if [[ "$1" =~ ^(data|all)$ ]]; then
  cd "${ROOT_FOLDER}"
  rm -rf ./build-emscripten/games/
  mkdir -p ./build-emscripten/games/
  cd dists/engine-data
  ./create-testbed-data.sh
  mv testbed "${ROOT_FOLDER}/build-emscripten/games/testbed"

  games=true
  if [ "$games" = true ]; then
    mkdir -p ./dists/emscripten/games/
    cd "${ROOT_FOLDER}/dists/emscripten/games/"
    wget -nc https://downloads.scummvm.org/frs/demos/scumm/ft-dos-demo-en.zip
    unzip -n ft-dos-demo-en -d "${ROOT_FOLDER}/build-emscripten/games/ft-dos-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/mohawk/myst-win-demo-en.zip
    unzip -n myst-win-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/myst-win-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/mohawk/riven-win-demo-en.zip
    unzip -n riven-win-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/riven-win-demo-en/" -x DXSETUP/* -x QTWSETUP/*
    wget -nc https://downloads.scummvm.org/frs/demos/sword2/sword2-win-demo-en.zip
    unzip -n sword2-win-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/sword2-win-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/scumm/comi-win-large-demo-en.zip
    unzip -n comi-win-large-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/comi-win-large-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/grim/emi-win-demo-en.zip
    unzip -n emi-win-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/emi-win-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/grim/grim-win-demo2-en.zip
    unzip -n grim-win-demo2-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/grim-win-demo2-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/agos/feeble-dos-ni-demo-en.zip
    unzip -n feeble-dos-ni-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/feeble-dos-ni-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/sci/lsl7-dos-demo-en.zip
    unzip -n lsl7-dos-demo-en.zip -d "${ROOT_FOLDER}/build-emscripten/games/lsl7-dos-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/demos/sci/phantasmagoria-dos-win-demo-en.zip
    unzip -n phantasmagoria-dos-win-demo-en -d "${ROOT_FOLDER}/build-emscripten/games/phantasmagoria-dos-win-demo-en/"
    wget -nc https://downloads.scummvm.org/frs/extras/Beneath%20a%20Steel%20Sky/BASS-Floppy-1.3.zip
    unzip -n BASS-Floppy-1.3.zip -d "${ROOT_FOLDER}/build-emscripten/games/bass-floppy/"
    wget -nc https://downloads.scummvm.org/frs/extras/Drascula_%20The%20Vampire%20Strikes%20Back/drascula-1.0.zip
    unzip -n drascula-1.0.zip -d "${ROOT_FOLDER}/build-emscripten/games/drascula/"
    wget -nc https://downloads.scummvm.org/frs/extras/Drascula_%20The%20Vampire%20Strikes%20Back/drascula-audio-mp3-2.0.zip
    unzip -n drascula-audio-mp3-2.0.zip -d "${ROOT_FOLDER}/build-emscripten/games/drascula/"
    wget -nc https://downloads.scummvm.org/frs/extras/Flight%20of%20the%20Amazon%20Queen/FOTAQ_Floppy.zip
    unzip -n FOTAQ_Floppy.zip -d "${ROOT_FOLDER}/build-emscripten/games/fotaq-floppy/"
    wget -nc https://downloads.scummvm.org/frs/extras/Lure%20of%20the%20Temptress/lure-1.1.zip
    unzip -n lure-1.1.zip -d "${ROOT_FOLDER}/build-emscripten/games/lure/"
  fi

  cd "${ROOT_FOLDER}/build-emscripten/games/"
  NODE_DIR=$(dirname "$EMSDK_NODE")
  "$NODE_DIR/npx" -p browserfs make_xhrfs_index >index.json
fi

if [[ "$1" =~ ^(make|all)$ ]]; then
  cd "${ROOT_FOLDER}"
  emmake make
  emmake make dist-generic
  # preload data
  "$EMSDK_PYTHON" "$EMSDK/upstream/emscripten/tools/file_packager.py" files.data --preload ./dist-generic/scummvm/data@/scummvm --use-preload-cache --js-output=files.js
  rm -rf dist-generic/
fi

if [[ "$1" =~ ^(dist|all)$ ]]; then
  cd "${ROOT_FOLDER}"
  mkdir -p build-emscripten
  mv scummvm.* build-emscripten/
  mv files.* build-emscripten/
  cp dists/emscripten/scummvm-512.png build-emscripten/
  cp dists/emscripten/scummvm-192.png build-emscripten/
  cp dists/emscripten/manifest.json build-emscripten/

fi
