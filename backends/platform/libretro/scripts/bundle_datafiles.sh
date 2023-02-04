#!/bin/bash

 # Copyright (C) 2022 Giovanni Cascione <ing.cascione@gmail.com>
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

set -e

function get_firmware_entry(){
	echo "
firmware$1_desc = \"$2\"
firmware$1_path = \"$3/$2\"
firmware$1_opt = \"true\"
"
}

function process_group(){
	local dirname="$1"
	shift
	local target="$1"
	shift
	local arr=("$@")
	for item in "${arr[@]}"; do
		[ $target = "bundle" ] && cp $item "${TMP_PATH}/${dirname}/"
		fragment=$(get_firmware_entry $count $(echo "$item" | sed "s|^.*/||g") "$dirname")
		CORE_INFO_DATS="${CORE_INFO_DATS}${fragment}"
		count=$(expr $count + 1)
	done
}

# Externally passed variables shall be:
# $1 [REQ] BUILD_PATH
# $2 [REQ] SCUMMVM_PATH
# $3 [REQ] target build ("bundle" to build scummvm.zip, any other string to build core info file)
# $4 [OPT] target name (prefix for core info file)
# $5 [OPT] displayed core name (shown in frontend)
# $6 [OPT] allowed extensions - backup if ScummVM.dat is not available

# Exit if in parameters are not provided
if [ -z $1 ] || [ -z $2 ] || [ -z $3 ] ; then
	exit 1
fi

[ -z $4 ] && INFO_FILE_PRE=scummvm || INFO_FILE_PRE=$4
[ -z $5 ] && NICE_NAME=ScummVM || NICE_NAME=$5
[ -z $6 ] && ALLOWED_EXT=scummvm || ALLOWED_EXT=$6

# Set variables
BUILD_PATH="$1"
SCUMMVM_PATH="$2"
TMP_PATH="${BUILD_PATH}/tmp_data"
TARGET_PATH="${BUILD_PATH}"
BUNDLE_DIR="scummvm"
BUNDLE_DATAFILES_DIR="${BUNDLE_DIR}/extra"
BUNDLE_THEME_DIR="${BUNDLE_DIR}/theme"
BUNDLE_ZIP_FILE="${BUNDLE_DIR}.zip"
BUNDLE_LOCAL_DATAFILES_DIR="${BUILD_PATH}/dist"

# Retrieve data file info from ScummVM source
THEMES_LIST=$(cat "${SCUMMVM_PATH}/dists/scummvm.rc" 2>/dev/null | grep FILE.*gui/themes.*\* | sed "s|.*\"\(.*\)\"|${SCUMMVM_PATH}/\1|g")
DATAFILES_LIST=$(cat "${SCUMMVM_PATH}/dists/scummvm.rc" 2>/dev/null| grep FILE.*dists/engine-data | sed "s|.*\"\(.*\)\"|${SCUMMVM_PATH}/\1|g")

# Put retrieved data into arrays
set +e
read -a THEME_ARRAY -d '' -r <<< "${THEMES_LIST}"
read -a DATAFILES_ARRAY -d '' -r <<< "$DATAFILES_LIST"
set -e

# Add specific data files
DATAFILES_ARRAY[${#DATAFILES_ARRAY[@]}]="${SCUMMVM_PATH}"/backends/vkeybd/packs/vkeybd_default.zip

# Make sure target folders exist
[ $3 = "bundle" ] && mkdir -p "${TMP_PATH}/${BUNDLE_THEME_DIR}/"
[ $3 = "bundle" ] && mkdir -p "${TMP_PATH}/${BUNDLE_DATAFILES_DIR}/"

count=0
# Process themes
	process_group "$BUNDLE_THEME_DIR" $3 ${THEME_ARRAY[@]}

# Process datafiles
	process_group "$BUNDLE_DATAFILES_DIR" $3 ${DATAFILES_ARRAY[@]}

# Process additional local bundle files
if [ -d "$BUNDLE_LOCAL_DATAFILES_DIR" -a ! -z "$(ls -A ${BUNDLE_LOCAL_DATAFILES_DIR} 2>/dev/null)" ] ; then
	for item in $BUNDLE_LOCAL_DATAFILES_DIR/*; do
		[ ! $(echo "$item" | sed "s|^.*/||g") = "README.md" ] && LOCAL_EXTRA_ARRAY+=("$item")
	done
	process_group "$BUNDLE_DATAFILES_DIR" $3 ${LOCAL_EXTRA_ARRAY[@]}
fi

if [ ! $3 = "bundle" ]; then

# Update from libretro ScummVM.dat
wget -NO "$BUILD_PATH"/ScummVM.dat https://raw.githubusercontent.com/libretro/libretro-database/master/dat/ScummVM.dat
[ -f "$BUILD_PATH"/ScummVM.dat ] && SUPPORTED_EXTENSIONS="$(cat $BUILD_PATH/ScummVM.dat | grep 'rom (' | sed -e 's/\" .*//g' -e 's/.*\.//g' | sort -u | tr '\n' '|')" || SUPPORTED_EXTENSIONS="$ALLOWED_EXT"

	# Create core.info file
	set +e
	read -d '' CORE_INFO_CONTENT <<EOF
# Software Information
display_name = "$NICE_NAME"
authors = "SCUMMVMdev"
supported_extensions = "$SUPPORTED_EXTENSIONS"
corename = "$NICE_NAME"
categories = "Game"
license = "GPLv3"
permissions = ""
display_version = $(cat $SCUMMVM_PATH/base/internal_version.h 2>/dev/null | grep SCUMMVM_VERSION | sed "s|^.*SCUMMVM_VERSION *||g")

# Hardware Information
manufacturer = "Various"
systemname = "Game engine"
systemid = "scummvm"

# Libretro Features
database = "ScummVM"
supports_no_game = "true"
savestate = "false"
savestate_features = "null"
cheats = "false"
input_descriptors = "true"
memory_descriptors = "false"
libretro_saves = "false"
core_options = "true"
core_options_version = "1.3"
load_subsystem = "false"
hw_render = "false"
needs_fullpath = "true"
disk_control = "false"
is_experimental = "false"

# Firmware / BIOS
firmware_count = $count
EOF
	set -e

	CORE_INFO_CONTENT="${CORE_INFO_CONTENT}${CORE_INFO_DATS}
description = \"The ScummVM adventure game engine ported to libretro. This core is built directly from the upstream repo and is synced upon stable releases, though it is not supported upstream. So please report any bug to Libretro and/or make sure the same apply to the standalone ScummVM program as well, before making any report to ScummVM Team.\""
	echo "$CORE_INFO_CONTENT" > "${TARGET_PATH}/${INFO_FILE_PRE}_libretro.info"
	echo "${INFO_FILE_PRE}_libretro.info created successfully"
else

	# Create archive
	rm -f "${TARGET_PATH}/$BUNDLE_ZIP_FILE"
	cd "${TMP_PATH}"
	zip -rq "${TARGET_PATH}/$BUNDLE_ZIP_FILE" "${BUNDLE_DIR}" > /dev/null 2>&1
	cd - > /dev/null

	# Remove temporary directories
	rm -rf "$TMP_PATH"
	echo "$BUNDLE_ZIP_FILE created successfully"
fi
