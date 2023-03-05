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
# $1 [REQ] repository URL
# $2 [REQ] target commit
# $3 [REQ] submodules folder
# $4 [REQ] allow dirty submodules [0,1]
# $5 [OPT] submodule folder (if not specified will be the same name of the repository)

# Any error response will interrupt the script and will not reach the "0" output, to be used for success test upstream
set -e

TARGET_URL=$1
TARGET_COMMIT=$2
SUBMODULES_PATH=$3
[ -z $4 ] && ALLOW_DIRTY=0 || ALLOW_DIRTY=$4
[ -z $5 ] && SUBMODULE_FOLDER=$(echo $TARGET_URL | sed "s|.*/||") || SUBMODULE_FOLDER=$5

clone=0
reset=0

# Exit if in parameters are not provided
if [ -z $TARGET_URL ] || [ -z $TARGET_COMMIT ] || [ -z $SUBMODULES_PATH ] || [ -z $SUBMODULE_FOLDER ] ; then
	exit 1
fi

# Create submodules folder if does not exist and move into it
[ ! -d $SUBMODULES_PATH ] && mkdir -p $SUBMODULES_PATH

cd $SUBMODULES_PATH

# Folder exists
if [ -d $SUBMODULE_FOLDER ]; then
	# Folder is not our repo, not our commit or not git repo at all, remove and clone (no history) again
	if [ ! $(cd $SUBMODULE_FOLDER && git config --get remote.origin.url 2>/dev/null) = $TARGET_URL ] || [ ! $(cd $SUBMODULE_FOLDER && git rev-parse HEAD 2>/dev/null) = $TARGET_COMMIT ] ; then
		rm -rf ${SUBMODULE_FOLDER} && clone=1
	fi

	# Dirty repo in folder, reset hard
	[ $ALLOW_DIRTY -ne 1 ] && [ $clone -ne 1 ] && [ ! -z "$(cd $SUBMODULE_FOLDER && git diff --shortstat 2>/dev/null)" ] && reset=1

# Folder does not exist
else
	clone=1
fi

# Apply collected actions
if [ $clone -eq 1 ] ; then
	mkdir -p $SUBMODULE_FOLDER
	git -C $SUBMODULE_FOLDER init > /dev/null 2>&1
	git -C $SUBMODULE_FOLDER remote add origin $TARGET_URL > /dev/null 2>&1
	git -C $SUBMODULE_FOLDER fetch --depth 1 origin $TARGET_COMMIT > /dev/null 2>&1
	git -C $SUBMODULE_FOLDER checkout FETCH_HEAD > /dev/null 2>&1
	git -C $SUBMODULE_FOLDER submodule update --init --recursive --depth 1 > /dev/null 2>&1
fi

cd $SUBMODULE_FOLDER
[ $reset -eq 1 ] && git reset --hard > /dev/null 2>&1

# Success
echo 0
