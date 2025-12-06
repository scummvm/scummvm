#!/bin/bash
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
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'

echo_n() {
	printf "$@"
}

echoOk() {
  echo -e "${GREEN}ok${NC}"
  totalChecks=$((totalChecks+1))
}

failPlus() {
  failedChecks=$((failedChecks+1))
  totalChecks=$((totalChecks+1))
}

if [ ! -d ../scummvm-icons ]; then
  echo scummvm-icons repo is missing
  echo -e "${RED}Run cd ..; git clone https://github.com/scummvm/scummvm-icons.git${NC}"

  exit 1
fi

if [ ! -d ../scummvm-shaders ]; then
  echo scummvm-shaders repo is missing
  echo -e "${RED}Run cd ..; git clone https://github.com/scummvm/scummvm-shaders.git${NC}"

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

fileDate=`git log -1 --pretty=format:"%aI" gui/themes/gui-icons.dat`

cd ../scummvm-icons

num_lines=`git -P log --oneline "--since=$fileDate" default/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'cd ../scummvm-icons/default; zip -r9 ../../scummvm/gui/themes/gui-icons.dat .'${NC}"

  failPlus
else
  echoOk
fi


echo_n "Checking icon packs..."

cd ../scummvm-icons

last_md5=`tail -1 LIST|awk -F, '{ print $3 }'`
num_lines=`git log --oneline ${last_md5}.. | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'gen-set.py' in ../scummvm-icons${NC}"

  failPlus
else
  echoOk
fi

cd "$oldpwd"

###########
# Shaders
###########

echo_n "Checking default shaders pack..."

fileDate=`git log -1 --pretty=format:"%aI" gui/themes/shaders.dat`

cd ../scummvm-shaders

num_lines=`git -P log --oneline "--since=$fileDate" base/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Repack gui/themes/shaders.dat${NC}"

  failPlus
else
  echoOk
fi

echo_n "Checking shader packs..."

cd ../scummvm-shaders

last_md5=`tail -1 LIST-SHADERS|awk -F, '{ print $3 }'`
num_lines=`git log --oneline ${last_md5}.. | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'gen-shaders.py' in ../scummvm-shaders${NC}"

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

git -P grep ADGF_TESTING | grep -v engines/advancedDetector. | grep -v "engines/ags/detection_tables.h:.DETECTION_ENTRY.ID," | grep -v devtools/release-checks.sh >$TMP

num_lines=`cat $TMP | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines entries. ${RED}They must be removed:${NC}"
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
  echo -e "missing. ${RED}Run 'make ideprojects'${NC}"

  failPlus
fi

###########
# Translations
###########

echo_n "Checking translations..."

fileDate=`git log -1 --pretty=format:"%aI" gui/themes/translations.dat`

num_lines=`git -P log --oneline "--since=$fileDate" po/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'make translations-dat'${NC}"

  failPlus
else
  echoOk
fi

echo_n "Checking Android translations..."

fileDate=`git log -1 --pretty=format:"%aI" dists/android/res/values-*/strings.xml`

num_lines=`git -P log --oneline "--since=$fileDate" po/ | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'make android-translations'${NC}"

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
  echo -e "missing. ${RED}Upload to https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes${NC}"

  failPlus
else
  echoOk
fi

echo_n "Checking Release Notes HTML..."

if curl -s -I https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes.html | head -n 1 | grep 404 >/dev/null; then
  echo -e "missing. ${RED}Upload to https://downloads.scummvm.org/frs/scummvm/${VERSION}/ReleaseNotes.html${NC}"

  failPlus
else
  echoOk
fi

##########
# POTFILES
##########

echo_n "Checking POTFILES duplicates..."

OLDIFS="$IFS"
IFS=$'\n'  # allow arguments with tabs and spaces

# First, assemble all the files

list=`cat po/POTFILES engines/*/POTFILES | sed '/^$/d' | sort`

dupes=`uniq -c <<<"$list" | grep -v "1 "`

if [ ! -z "$dupes" ]; then
    num_lines=`wc -l <<< "$dupes"`
    echo -e "$num_lines dupes detected"

    echo $dupes

  failPlus
else
  echoOk
fi

echo_n "Checking POTFILES header includes..."

headerlist=`grep \.h$ <<< "$list" | grep -v detection_ | grep -v keymapper_ | grep -v engines/gob/detection/tables`

if [ ! -z "$headerlist" ]; then
  num_lines=`wc -l <<< "$headerlist"`
  echo -e "$num_lines headers detected"
  echo -e "${YELLOW}It is not advised to include common/translation.h in header files."
  echo -e "The exception is detection_*.h and keymapper_*.h files."
  echo -e "Putting this include in header could easily lead to missing POTFILES addition.${NC}"

  echo "$headerlist"

  failPlus
else
  echoOk
fi


echo_n "Checking missing/extra POTFILES..."

# Now get list of includes
git grep -l "common/translation\.h" | grep -v devtools/create_engine | grep -v devtools/release-checks.sh | grep -v devtools/generate-android-i18n-strings.py | grep -v engines/gob/detection/tables.h | sort > $TMP

res=`diff -  $TMP <<< "$list"`

if [ ! -z "$res" ]; then
    echo -e "${RED}Failed.${NC}"
    echo -e "${YELLOW}The diff is below. < marks extra files in POTFILES, > marks missing file in POTFILES${NC}"

    echo "$res"

    failPlus
else
  echoOk
fi

rm -f $TMP

IFS="$OLDIFS"

##########
# Dat files
##########

echo_n "Checking engine-data files..."

cat dists/engine-data/engine_data.mk dists/engine-data/engine_data_big.mk dists/engine-data/engine_data_core.mk >dists/engine-data/engine_data_combined.mk

# Use # as delimiter, %FILE% for the filename
declare -a distfiles=(
  "dists/engine-data/README#%FILE%:"
  "dists/engine-data/engine_data_combined.mk#DIST_FILES_LIST \+= dists/engine-data/%FILE%"
  "dists/irix/scummvm.idb#f 0644 root sys usr/ScummVM/share/scummvm/%FILE% %FILE% scummvm.sw.eoe"
)

OLDIFS="$IFS"
IFS=$'\n'  # allow arguments with tabs and spaces

absentFiles=0

for f in dists/engine-data/*
do
    # Skip directories
    if [ -d $f ]; then
        continue
    fi

    file=`basename $f`

    # Skip README file
    if [ $file == "README" -o $file == "create-playground3d-data.sh"  -o $file == "create-testbed-data.sh" \
            -o $file == "engine_data.mk" -o $file == "engine_data_big.mk" -o $file == "engine_data_core.mk" \
            -o $file == "engine_data_combined.mk" ]; then
        continue
    fi

    for d in "${distfiles[@]}"
    do
        target=`cut -d '#' -f 1 <<< "$d"`
        pattern=`cut -d '#' -f 2 <<< "$d"`

        res=`sed "s|%FILE%|$file|g" <<< "$pattern"`

        if [ -z $(grep -E "$res" "$target" | head -1) ]; then
            echo "$file is absent in $target"
            absentFiles=$((absentFiles+1))
        fi
    done
done

rm dists/engine-data/engine_data_combined.mk

if [ "$absentFiles" -ne "0" ]; then
  echo -e "$absentFiles missing files ${RED}Fix them${NC}"

  failPlus
else
  echoOk
fi

##########
# GUI theme files
##########

echo_n "Checking GUI theme files..."

absentFiles=0

declare -a themefiles=(
  "Makefile.common#DIST_FILES_THEMES.*%FILE%"
  "devtools/create_project/xcode.cpp#[	 ]+files.push_back\(\"gui/themes/%FILE%\"\);"
  "dists/irix/scummvm.idb#f 0644 root sys usr/ScummVM/share/scummvm/%FILE% %FILE% scummvm.sw.eoe"
  "dists/scummvm.rc#%FILE%[	 ]+FILE[	 ]+\"gui/themes/%FILE%\""
)

for f in gui/themes/*
do
    # Skip directories
    if [ -d $f ]; then
        continue
    fi

    file=`basename $f`

    # Skip README file
    if [ $file == "scummtheme.py" -o $file == "default.inc" ]; then
        continue
    fi

    for d in "${themefiles[@]}"
    do
        target=`cut -d '#' -f 1 <<< "$d"`
        pattern=`cut -d '#' -f 2 <<< "$d"`

        res=`sed "s|%FILE%|$file|g" <<< "$pattern"`

        if [ -z $(grep -E "$res" "$target" | head -1) ]; then
            echo "$file is absent in $target"
            absentFiles=$((absentFiles+1))
        fi
    done
done

if [ "$absentFiles" -ne "0" ]; then
  echo -e "$absentFiles missing files ${RED}Fix them${NC}"

  failPlus
else
  echoOk
fi

##########
# LICENSE files
##########

OLDIFS="$IFS"
IFS=$'\n'  # allow arguments with tabs and spaces

echo_n "Checking LICENSE files..."

absentFiles=0

declare -a licensefiles=(
  "Makefile.common#DIST_FILES_DOCS.*%FILE%"
  "backends/platform/maemo/debian/rules#install -m0644.*%FILE%.*debian/scummvm/usr/share/doc/scummvm"
  "backends/platform/sdl/macosx/appmenu_osx.mm#openFromBundle\(@\"%FILEUNDOTTED%\", @\"licenses\"\);"
  "backends/platform/sdl/win32/win32.mk#cp \\$\(srcdir\)/%FILE% \\$\(WIN32PATH\)/%FILEBASE%\.txt"
  "devtools/create_project/create_project.cpp#in\.push_back\(setup.srcDir \+ \"/%FILE%\"\)"
  "devtools/create_project/xcode.cpp#[	 ]+files.push_back\(\"%FILE%\"\);"
  "dists/irix/scummvm.idb#f 0644 root sys usr/ScummVM/%FILEBASE% %FILE% scummvm.man.readme"
  "dists/redhat/scummvm.spec#%doc AUTHORS README.md.*%FILE%"
  "dists/redhat/scummvm.spec.in#%doc AUTHORS README.md.*%FILE%"
  "ports.mk#mv \\$\(bundle_name\)/Contents/Resources/%FILEBASE% \\$\(bundle_name\)/Contents/Resources/licenses/%FILEUNDOTTED%"
  "ports.mk#mv \./ScummVM-snapshot/%FILEBASE% \./ScummVM-snapshot"
)

for f in LICENSES/*
do
    file=$f
    filebase=`basename $f`
    fileundotted=`sed "s|\.|-|g" <<< "$filebase"`

    for d in "${licensefiles[@]}"
    do
        if [ $filebase == "CatharonLicense.txt" -o $d == "backends/platform/sdl/win32/win32.mk" ]; then
            # the file is not renamed to .txt.txt
            continue
        fi

        target=`cut -d '#' -f 1 <<< "$d"`
        pattern=`cut -d '#' -f 2 <<< "$d"`

        res=`sed "s|%FILE%|$file|g;s|%FILEBASE%|$filebase|g;s|%FILEUNDOTTED%|$fileundotted|g" <<< "$pattern"`

        if [ -z $(grep -E "$res" "$target" | head -1) ]; then
            echo "$file is absent in $target"
            absentFiles=$((absentFiles+1))
        fi
    done
done

if [ "$absentFiles" -ne "0" ]; then
  echo -e "$absentFiles missing files ${RED}Fix them${NC}"

  failPlus
else
  echoOk
fi

echo_n "Checking LICENSE files contents..."

checkFailed=0

for f in LICENSES/*
do

  if [ -z $(head -2 $f | grep NOTE: ) ]; then
    echo "$f is missing NOTE"
    checkFailed=$((checkFailed+1))
  fi
done

if [ "$checkFailed" -ne "0" ]; then
  echo -e "$checkFailed files malformed ${RED}Fix them${NC}"

  failPlus
else
  echoOk
fi

IFS="$OLDIFS"


#############################################################################
#
# Engines go here
#
#############################################################################

###########
# Bagel engine
###########

echo_n "Checking bagel.dat..."

fileDate=`git log -1 --pretty=format:"%aI" dists/engine-data/bagel.dat`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_bagel/files | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'cd devtools/create_bagel/files; zip -r9 ../../../dists/engine-data/bagel.dat .'${NC}"

  failPlus
else
  echoOk
fi


###########
# MM engine
###########

echo_n "Checking mm.dat..."

fileDate=`git log -1 --pretty=format:"%aI" dists/engine-data/mm.dat`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_mm/files | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'cd devtools/create_mm/files; zip -r9 ../../../dists/engine-data/mm.dat .'${NC}"

  failPlus
else
  echoOk
fi


###########
# Nancy engine
###########

echo_n "Checking nancy.dat..."

fileDate=`git log -1 --pretty=format:"%aI" dists/engine-data/nancy.dat`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_nancy/files | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'make devtools/create_nancy;cd devtools/create_nancy;./create_nancy;cp nancy.dat ../../dists/engine-data'${NC}"

  failPlus
else
  echoOk
fi


###########
# Ultima engine
###########

echo_n "Checking ultima.dat..."

fileDate=`git log -1 --pretty=format:"%aI" dists/engine-data/ultima.dat`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_ultima/files | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'cd devtools/create_ultima/files; zip -r9 ../../../dists/engine-data/ultima.dat .'${NC}"

  failPlus
else
  echoOk
fi



###########
# Ultima 8 engine
###########

echo_n "Checking ultima8.dat..."

fileDate=`git log -1 --pretty=format:"%aI" dists/engine-data/ultima8.dat`

num_lines=`git -P log --oneline "--since=$fileDate" devtools/create_ultima8 | wc -l`

if [ "$num_lines" -ne "0" ]; then
  echo -e "$num_lines unprocessed commits. ${RED}Run 'cd devtools/create_ultima8; zip -r9 ../../dists/engine-data/ultima8.dat .'${NC}"

  failPlus
else
  echoOk
fi



###########
# Totals
###########

if [ "$failedChecks" -eq "0" ]; then
  echo -e "${GREEN}All ok${NC}"
else
  echo -e "${RED}Failed $failedChecks checks of $totalChecks"${NC}
fi


exit 0
