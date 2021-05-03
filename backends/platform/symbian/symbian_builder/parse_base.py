# ScummVM - Graphic Adventure Engine
# Copyright (C) 2021 Stryzhniou Fiodar

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

from __future__ import with_statement
import os
from common_names import *
from prj_generator import SafeWriteFile

excluded = ("", "")

def exclude_special_cpp(tmp):
   d = "%s.cpp" %tmp[:-2]
   if d in excluded:
      print "excluded: %s" %d
      return ""
   return tmp

src_dirs = ("backends", "common", "engines", "gui", "math")
mmp_name = "scummvm_base.mmp"

mmp_template = """
TARGET          scummvm_base.lib
TARGETTYPE      lib
#include "../%s/build_config.mmh"

// compiler must use png.h from libpng.lib instead ScummVM's
OPTION			GCCE -I'/Symbian/S60_5th_Edition_SDK_v1.0/epoc32/include/png'

// added and managed by porter
SOURCEPATH  ..\..\..\..\\backends
SOURCE     audiocd\sdl\sdl-audiocd.cpp
SOURCE     fs\symbian\symbianstream.cpp
SOURCE     mixer\symbiansdl\symbiansdl-mixer.cpp
// end porter job\n
#include "build_parts.mmh"
\n#include \"../mmp/macros.mmh\"\n
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
         addsrc = True
         if addsrc is None:
            addsrc = True
      elif "endif" in i:
         addsrc = False
      elif "SDL_BACKEND" in i:
         addsrc = True
      elif "ENABLE_" in i:
         t = i.split()[-1]
         if t in active_config:
            addsrc = True
         else:
            addsrc = False
            if t not in disabled_config:
               print "New macro found: %s" %t
      elif "USE_" in i:
         t = i.split()[-1]
         if t in active_config:
            addsrc = True
         else:
            addsrc = False
            if t not in disabled_config:
               print "New macro found: %s" %t
      elif "DISABLE_" in i:
         t = i.split()[-1]
         if t in active_config:
            addsrc = True
            addsrc = False
            if t not in disabled_config:
               print "New macro found: %s" %t
      elif addsrc is True:
         tmp = i.strip()
         tmp = tmp.rstrip("\\")
         tmp = tmp.strip()
         tmp = exclude_special_cpp(tmp)
         if tmp.endswith(".o"):
            src += ["SOURCE   %s.cpp" %tmp[:-2]]
   SafeWriteFile(mmp_file, src, 'a')

def parse_base(platform = "S60v3"):
   uids = get_UIDs(build)
   codecs_mmp = os.path.join(mmps, mmp_name)
   SafeWriteFile(codecs_mmp, mmp_template %platform)
   [processModule_mk(i, codecs_mmp) for i in src_dirs]

if __name__ == "__main__":
   parse_base()
