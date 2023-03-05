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

# Externally passed variables shall be:

# $1 [REQ] BUILD_PATH
# $2 [REQ] SCUMMVM_PATH
# $3 [REQ] NO_HIGH_DEF [0,1]
# $4 [REQ] NO_WIP [0,1]
# $5 [REQ] STATIC_LINKING [0,1]
# $6 [REQ] LITE [0,1]

set -e

# Exit if in parameters are not provided
if [ -z $1 ] || [ -z $2 ] || [ -z $3 ] || [ -z $4 ] || [ -z $5 ] || [ -z $6 ] ; then
	exit 1
fi

BUILD_PATH="$1"
SCUMMVM_PATH="$2"

cd "${SCUMMVM_PATH}"

# Retrieve all configure functions
sed -i.bak -e "s/exit 0/return 0/g" configure
. configure -h > /dev/null 2>&1
mv configure.bak configure > /dev/null 2>&1

_parent_engines_list=""

# Collect all default engines dependencies and force to yes
tot_deps=""
for a in $_engines ; do
	engine_deps_var=_engine_${a}_deps
	for dep in ${!engine_deps_var} ; do
		found=0
		for rec_dep in $tot_deps ; do
			[ $dep = $rec_dep ] && found=1
		done
		[ $found -eq 0 ] && tot_deps+=" $dep"
	done

	# Static linking support files
	not_subengine_var=_engine_${a}_sub
	not_wip_engine_var=_engine_${a}_build_default
	if [ $5 -eq 1 ] && [ -z ${!not_subengine_var} ] ; then
		good_to_go=1
		# Test NO_HIGH_DEF
		[ $3 -eq 1 ] && [ $((echo ${!engine_deps_var} | grep -q highres); echo $?) -eq 0 ]  && good_to_go=0
		[ $4 -eq 1 ] && [ $(echo ${!not_wip_engine_var} = no) ] && good_to_go=0
		[ $6 -eq 1 ] && [ $((cat ${BUILD_PATH}/lite_engines.list | grep -wq ${a}); echo $?) -eq 1 ] && good_to_go=0
		[ $good_to_go -eq 1 ] &&  _parent_engines_list+="ADDLIB libtemp/lib${a}.a"$'\n'
	fi
done

[ $5 -eq 1 ] && printf "$_parent_engines_list" >> "$BUILD_PATH"/script.mri

for dep in $tot_deps ; do
	eval _$dep=yes
done

# Test NO_HIGH_DEF
if [ $3 -eq 1 ] ; then
	_highres=no
fi

# Test LITE
if [ $6 -eq 1 ] ; then
	cp "${BUILD_PATH}"/lite_engines.list "${BUILD_PATH}"/config.mk.engines.lite
	sed -i.bak -e "y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/" -e "s/^/ENABLE_/g;s/$/ = STATIC_PLUGIN/g" "${BUILD_PATH}"/config.mk.engines.lite
	rm "${BUILD_PATH}"/config.mk.engines.lite.bak
fi

# Create needed engines build files
awk -f "engines.awk" < /dev/null > /dev/null 2>&1

mkdir -p "engines"

copy_if_changed engines/engines.mk.new "engines/engines.mk"
copy_if_changed engines/detection_table.h.new "engines/detection_table.h"
copy_if_changed engines/plugins_table.h.new "engines/plugins_table.h"

# Test NO_WIP
[ $4 -ne 1 ] && sed -i.bak -e "s/# \(.*\)/\1 = STATIC_PLUGIN/g" "config.mk.engines"

echo 0
