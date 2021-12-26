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

excluded = ("renderer.cpp", "sjis.cpp", "coktel_decoder.cpp")

def exclude_special_cpp(tmp):
   d = "%s.cpp" %tmp[:-2]
   if d in excluded:
      print "excluded: %s" %d
      return ""
   return tmp

src_dirs = ("audio", "graphics", "image", "video")

mmp_name = "scummvm_codecs.mmp"

mmp_template = """
TARGET          scummvm_codecs.lib
TARGETTYPE      lib
#include "../%s/build_config.mmh"

// compiler must use png.h from libpng.lib instead ScummVM's
OPTION			GCCE -I'/Symbian/S60_5th_Edition_SDK_v1.0/epoc32/include/png'

"""

def processModule_mk(dir, mmp_file):
   pth = os.path.join('..\..\..', dir)
   with open(os.path.join(pth, "module.mk")) as ff:
      f = ff.readlines()
   pth = os.path.join('..\..\..\..', dir)
   SafeWriteFile(mmp_file, "\nSOURCEPATH  %s\n" %pth, 'a')

   src = []
   addsrc = None

   for i in f:
      if "MODULE_OBJS :=" in i:
         print i
         addsrc = True
         if addsrc is None:
            addsrc = True
      elif len(i) == 1:
         addsrc = False
      elif "endif" in i:
         addsrc = False
      elif "ENABLE_" in i:
         t = i.split()[-1]
         if t in active_config:
            addsrc = True
         elif t in disabled_config:
            addsrc = False
         elif t not in disabled_config:
            SafeWriteFile(build_log, "New macro found: %s" %t)
      elif "USE_" in i:
         t = i.split()[-1]
         print "%s %s" %(dir, t)
         if t in active_config:
            print "active_config %s %s" %(dir, t)
            addsrc = True
         else:
            print "disabled_config %s %s" %(dir, t)
            addsrc = False
            if t not in disabled_config:
               SafeWriteFile(build_log, "New macro found: %s" %t)
      elif "DISABLE_" in i:
         t = i.split()[-1]
         if t in active_config:
            addsrc = True
         elif t not in disabled_config:
            SafeWriteFile(build_log, "New macro found: %s" %t)
         else:
            addsrc = False
      elif addsrc is True:
         tmp = i.strip()
         tmp = tmp.rstrip("\\")
         tmp = tmp.strip()
         tmp = exclude_special_cpp(tmp)
         if tmp.endswith(".o"):
            src += ["SOURCE   %s.cpp" %tmp[:-2]]
   SafeWriteFile(mmp_file, src, 'a')

def parse_codecs(platform):
   uids = get_UIDs(build)
   codecs_mmp = os.path.join(mmps, mmp_name)
   SafeWriteFile(codecs_mmp, mmp_template %platform)
   for i in range(len(uids)):
      idx = i+1
      SafeWriteFile(codecs_mmp, "#define SCUMMVM_PT_%d\n" %idx, 'a')
   SafeWriteFile(codecs_mmp, "\n#include \"../%s/macros.mmh\"\n" %platform, 'a')
   [processModule_mk(i, codecs_mmp) for i in src_dirs]

if __name__ == "__main__":
   parse_codecs(platform = "S60v3")
