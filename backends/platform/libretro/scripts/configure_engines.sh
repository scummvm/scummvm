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

# $1     [REQ] BUILD_PATH
# $2     [REQ] SCUMMVM_PATH
# $3     [REQ] NO_WIP [0,1]
# $4     [REQ] STATIC_LINKING [0,1]
# $5     [REQ] LITE [0,1,2]
# $[...] [OPT] Engines dependencies not available

set -e

# Exit if in parameters are not provided
if [ -z $1 ] || [ -z $2 ] || [ -z $3 ] || [ -z $4 ] || [ -z $5 ] ; then
	exit 1
fi

# Get parameters
BUILD_PATH="$1"
shift
SCUMMVM_PATH="$1"
shift
NO_WIP=$1
shift
STATIC_LINKING=$1
shift
LITE=$1
shift
no_deps=$@

cd "${SCUMMVM_PATH}"

# Retrieve all configure functions
sed -i.bak -e "s/exit 0/return 0/g" configure
. configure -h > /dev/null 2>&1
mv configure.bak configure > /dev/null 2>&1

_parent_engines_list=""
tot_deps=""

# Test NO_WIP
[ $NO_WIP -ne 1 ] && engine_enable_all

# Test LITE
[ $LITE -ne 0 ] && engine_disable_all

if [ $LITE -eq 1 ] ; then
	for eng in $(cat "${BUILD_PATH}"/lite_engines.list) ; do
		engine_enable "$eng"
	done
fi

# Define engines list
for a in $_engines ; do
	# Collect all default engines dependencies and force to yes
	for dep in $(get_var _engine_${a}_deps) ; do
		found=0
		for rec_dep in $tot_deps ; do
			[ $dep = $rec_dep ] && found=1
		done
		[ $found -eq 0 ] && append_var tot_deps "$dep"
	done
done

for dep in $tot_deps ; do
	eval _$dep=yes
done

for dep in $no_deps ; do
	eval _$dep=no
done

# Create needed engines build files
awk -f "engines.awk" < /dev/null > /dev/null 2>&1

mkdir -p "engines"

copy_if_changed engines/engines.mk.new "engines/engines.mk"
copy_if_changed engines/detection_table.h.new "engines/detection_table.h"
copy_if_changed engines/plugins_table.h.new "engines/plugins_table.h"

echo 0
