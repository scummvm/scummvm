# ScummVM - Graphic Adventure Engine
# Copyright (C) 2021 Stryzhniou Fiodar

# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from __future__ import with_statement
import os
from common_names import *


active_config = ("USE_LUA", )
src_dirs = ("common", )
mmp_name = "scummvm_lua.mmp"


mmp_template = """
TARGET          scummvm_lua.lib
TARGETTYPE      lib

MACRO           __IGNORE__E32STD_H__

#include        "../%s/build_config.mmh"
"""


def processModule_mk(folder, mmp_file, active_conf = active_config):
   pth = os.path.join('..\..\..', folder)
   with open(os.path.join(pth, "module.mk")) as ff:
      f = ff.readlines()
   pth = os.path.join('..\..\..\..', folder)
   SafeWriteFile(mmp_file, "\nSOURCEPATH  %s\n" %pth, 'a')

   src = []
   addsrc = None

   for i in f:
      if "MODULE_OBJS :=" in i:
         continue
      elif "endif" in i:
         addsrc = False
      elif "USE_LUA" in i:
         addsrc = True
      elif addsrc is True:
         tmp = i.strip()
         tmp = tmp.rstrip("\\")
         tmp = tmp.strip()
         if tmp.endswith(".o"):
            src += ["SOURCE   %s.cpp" %tmp[:-2]]
   SafeWriteFile(mmp_file, src, 'a')


def parse_lua(platform = "S60v3"):
   mmp_file = os.path.join(mmps, mmp_name)
   SafeWriteFile(mmp_file, mmp_template %platform)
   [processModule_mk(i, mmp_file, active_config) for i in src_dirs]


if __name__ == "__main__":
   parse_lua()