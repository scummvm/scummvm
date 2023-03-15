#!/bin/sh -e
#
# release-checks.sh - sanity checking for release readiness
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

TMP=release-check.tmp

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo_n() {
	printf "$@"
}

echoOk() {
  echo "${GREEN}ok${NC}"
  totalChecks=$((totalChecks+1))
}

failPlus() {
  failedChecks=$((failedChecks+1))
  totalChecks=$((totalChecks+1))
}

if [ ! -d ../scummvm-icons ]; then
  echo scummvm-icons repo is missing
  echo Run
  echo cd ..\; git clone https://github.com/scummvm/scummvm-icons.git

  exit 1
fi

if [ ! -d ../scummvm-shaders ]; then
  echo scummvm-shaders repo is missing
  echo Run
  echo cd ..\; git clone https://github.com/scummvm/scummvm-shaders.git

  exit 1
fi

oldpwd=`pwd`

failedChecks=0
totalChecks=0

echo_n "Updating scummvm-icons..."
cd ../scummvm-icons
git pull --rebase >/dev/null
echoOk

echo_n "Updating scummvm-shaders..."
cd ../scummvm-shaders
git pull --rebase >/dev/null
echoOk

cd "$oldpwd"


###########
# Icons
###########

echo_n "Checking default icons pack..."

fileDate=`git log -1 gui/themes/gui-icons.dat | grep Date | sed 's/Date: //'`

cd ../scummvm-icons

num_lines=`git -P log --oneline "--since=$fileDate" default/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Repack gui/themes/gui-icons.dat${NC}"

  failPlus
else
  echoOk
fi


echo_n "Checking icon packs..."

cd ../scummvm-icons

last_md5=`tail -1 LIST|awk -F, '{ print $3 }'`
num_lines=`git log --oneline ${last_md5}.. | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Run 'gen-set.py' in ../scummvm-icons${NC}"

  failPlus
else
  echoOk
fi

cd "$oldpwd"

###########
# Shaders
###########

echo_n "Checking default shaders pack..."

fileDate=`git log -1 gui/themes/shaders.dat | grep Date | sed 's/Date: //'`

cd ../scummvm-shaders

num_lines=`git -P log --oneline "--since=$fileDate" base/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Repack gui/themes/shaders.dat${NC}"

  failPlus
else
  echoOk
fi

echo_n "Checking shader packs..."

cd ../scummvm-shaders

last_md5=`tail -1 LIST-SHADERS|awk -F, '{ print $3 }'`
num_lines=`git log --oneline ${last_md5}.. | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Run 'gen-shaders.py' in ../scummvm-shaders${NC}"

  failPlus
else
  echoOk
fi

cd "$oldpwd"

###########
# Sanity
###########

echo_n "Checking ADGF_TESTING..."

# Filter out known valid declarations
#
# as of 2022.03 they are:
#   engines/advancedDetector.cpp:	else if (desc->flags & ADGF_TESTING)
#   engines/advancedDetector.h:	ADGF_TESTING         = (1u << 19), ///< Flag to designate not yet officially supported games that are fit for public testing.
#   engines/ags/detection_tables.h:	DETECTION_ENTRY(ID, FILENAME, MD5, SIZE, LANG, PLATFORM, nullptr, ADGF_TESTING)

git -P grep ADGF_TESTING | grep -v engines/advancedDetector. | grep -v "engines/ags/detection_tables.h:\tDETECTION_ENTRY.ID," >$TMP

num_lines=`cat $TMP | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines entries. ${RED}They must be removed:${NC}"
  cat $TMP
  echo

  failPlus
else
  echoOk
fi

rm -f $TMP

echo_n "Checking ideprojects..."

if [ -f dists/codeblocks/scummvm.cbp ]; then
  echoOk
else
  echo "missing. ${RED}Run 'make ideprojects'${NC}"

  failPlus
fi

echo_n "Checking translations..."

fileDate=`git log -1 gui/themes/translations.dat | grep Date | sed 's/Date: //'`

num_lines=`git -P log --oneline "--since=$fileDate" po/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Run 'make update-translations'${NC}"

  failPlus
else
  echoOk
fi


###########
# Release Notes
###########

echo_n "Checking Release Notes..."

VERSION=`grep SCUMMVM_VERSION base/internal_version.h | awk -F\" '{ print $2 }'`

if curl -s -I https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes | head -n 1 | grep 404 >/dev/null; then
  echo "missing. ${RED}Upload to https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes${NC}"

  failPlus
else
  echoOk
fi

echo_n "Checking Release Notes HTML..."

if curl -s -I https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes.html | head -n 1 | grep 404 >/dev/null; then
  echo "missing. ${RED}Upload to https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes.html${NC}"

  failPlus
else
  echoOk
fi

###########
# MM engine
###########

echo_n "Checking mm.dat..."

fileDate=`git log -1 dists/engine-data/mm.dat | grep Date | sed 's/Date: //'`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_mm/files | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Run 'cd devtools/create_mm/files; zip -r9 ../../../dists/engine-data/mm.dat .'${NC}"

  failPlus
else
  echoOk
fi



###########
# Ultima engine
###########

echo_n "Checking ultima.dat..."

fileDate=`git log -1 dists/engine-data/ultima.dat | grep Date | sed 's/Date: //'`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_ultima/files | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo "$num_lines unprocessed commits. ${RED}Run 'cd devtools/create_ultima/files; zip -r9 ../../../dists/engine-data/ultima.dat .'${NC}"

  failPlus
else
  echoOk
fi



###########
# Totals
###########

if [ "$failedChecks" -eq "0" ]; then
  echo "${GREEN}All ok${NC}"
else
  echo "${RED}Failed $failedChecks checks of $totalChecks"${NC}
fi


exit 0
