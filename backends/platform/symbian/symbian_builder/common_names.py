# ScummVM - Graphic Adventure Engine
# Copyright (C) 2020 Stryzhniou Fiodar

# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

mmps = "mmp"
pipe = "pipe"

build_log = "build.log"
build_err = "build.err"

build = 'full'
# build = 'release'

uids = ("0xA0000657", "0xA0000658", "0x2006FE7C", "0x2006FE7D", "0x2006FE7E", "0x2006FE7F", "0x2006FE80")
# Test uids 3
uids_tests = ("0xAA000657", "0xAA000658", "0xA006FE7C", "0xA006FE7D", "0xA006FE7E", "0xA006FE7F", "0xA006FE80")

def get_UIDs(build):
   if build == 'full':
      return uids_tests
   return uids


if __name__ == "__main__":
   print "This script holds together data used multiple modules."
