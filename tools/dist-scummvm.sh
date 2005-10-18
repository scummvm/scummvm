#!/bin/sh -e
#
# dist-module.sh - make release tarballs for one CVS module
#
# Largely based on dist-fink.sh, Copyright (c) 2001 Christoph Pfisterer
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

### configuration

#cvsroot=':ext:USER@cvs.sourceforge.net:/cvsroot/scummvm'
cvsroot=':pserver:anonymous@cvs.sourceforge.net:/cvsroot/scummvm'

### init

if [ $# -lt 2 ]; then
  echo "Usage: $0 <module> <version-number> [<temporary-directory> [<tag>]]"
  exit 1
fi

module=$1
version=$2
tmpdir=${3:-/tmp}
tag=$4
if [ -z "$tag" ]; then
  tag=release-`echo $version | sed 's/\./-/g'`
fi
fullname="$module-$version"

echo "packaging $module release $version, CVS tag $tag"

### setup temp directory

mkdir -p $tmpdir
cd $tmpdir
umask 022

if [ -d $fullname ]; then
  echo "There is a left-over directory in $tmpdir."
  echo "Remove $fullname, then try again."
  exit 1
fi

### check code out from CVS

echo "Exporting module $module, tag $tag from CVS:"
cvs -d "$cvsroot" export -r "$tag" -d $fullname $module
if [ ! -d $fullname ]; then
  echo "CVS export failed, directory $fullname doesn't exist!"
  exit 1
fi

### remove any .cvsignore files

find $fullname -name .cvsignore -exec rm {} \;

### versioning

if [ -f $fullname/VERSION ]; then
  echo $version >$fullname/VERSION
fi
if [ -f $fullname/stamp-cvs-live ]; then
  rm -f $fullname/stamp-cvs-live
  touch $fullname/stamp-rel-$version
fi

### roll the tarball

echo "Creating tarball $fullname.tar:"
rm -f $fullname.tar $fullname.tar.gz
tar -cvf $fullname.tar $fullname

echo "Compressing (using gzip) tarball $fullname.tar.gz..."
gzip -c9 $fullname.tar > $fullname.tar.gz
if [ ! -f $fullname.tar.gz ]; then
  echo "Packaging failed, $fullname.tar.gz doesn't exist!"
  exit 1
fi

echo "Compressing (using bzip2) tarball $fullname.tar.bz2..."
bzip2 -c9 $fullname.tar > $fullname.tar.bz2
if [ ! -f $fullname.tar.bz2 ]; then
  echo "Packaging failed, $fullname.tar.bz2 doesn't exist!"
  exit 1
fi

echo "Zipping $fullname.zip..."
zip -r9 $fullname.zip $fullname
if [ ! -f $fullname.zip ]; then
  echo "Packaging failed, $fullname.zip doesn't exist!"
  exit 1
fi


### finish up

echo "Done:"
ls -l *.tar.gz

exit 0
