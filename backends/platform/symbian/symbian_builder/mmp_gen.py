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


from __future__ import with_statement
import os, shutil
from collections import defaultdict as defdict
from common_names import *


#ignore unfinished engines in release build
def CheckForRelease(line_):
   if 'no' in line_:
      return build != 'release'
   else:
      return True


# This function is frozen! Don't edit!
def processengine(path):
   with open(os.path.join(path, "configure.engine")) as f:
      ff = f.readlines()
   ff = [n for n in ff if not n.startswith('# ')] #exclude comments
   ff = [n for n in ff if len(n) > 2]
   ff = [n.split('"', 2) for n in ff]
   try:
      tt = [n[0] + n[2] for n in ff]
      ff = tt
   except IndexError:
      print "Something wrong here:"
      print ff
      return None
   f = [n.split() for n in ff]

   if f[0][2] == 'no' and build == "release":
      return None
   # print "MACRO   ENABLE_%s  //  LIB:scummvm_%s.lib" %(f[0][1].upper(), f[0][1])
   buildparam = ["MACRO   ENABLE_%s  //  LIB:scummvm_%s.lib" %(f[0][1].upper(), f[0][1])]
   libname = ["STATICLIBRARY    scummvm_%s.lib" %f[0][1]]

   if len(f) == 1:
      return [buildparam, libname]

   if len(f[0]) > 3:
      f = f[1:]

   for i in f:
      if CheckForRelease(i[2]):
         # print "   MACRO   ENABLE_%s  //  Subengine" %i[1].upper()
         buildparam += ["   MACRO   ENABLE_%s  //  Subengine" %i[1].upper()]

   return [buildparam, libname]


# This function is frozen! Don't edit!
def processModule_mk(path, buildparams):
   with open(os.path.join(path, "module.mk")) as ff:
      f = ff.readlines()

   src = []
   addsrc = None

   for i in f:
#add engine base source code first
      if "MODULE_OBJS" in i:
         if addsrc is None:
            addsrc = True
      elif "ifdef ENABLE_" in i:
         for x in buildparams:
            if "MACRO   %s"%i.strip()[6:] in x:
               addsrc = True
               src += ["// Subengine %s"%x[18:]]
      elif addsrc is True:
         if ".o \\" in i[-5:]:
            src += ["SOURCE   " + i[1:-5] + ".cpp"]
         elif ".o" in i[-3:]:
            src += ["SOURCE   " + i[1:-3] + ".cpp"]
         elif len(i) == 1:
            addsrc = False
   return src


# Add per engine fixes
libc_engines = ("bladerunner", "glk", "illusions", "titanic", "ultima")
def CheckEngine(lst, game):
   if game == "sword25":
      return None
   if game in libc_engines:
      lst = lst + "\nOPTION    GCCE -I'/Symbian/S60_5th_Edition_SDK_v1.0/epoc32/include/libc'\n"
   if game == "zvision":
      lst = lst + "\nMACRO    USE_A52\n"
   return lst


def ProcessDup(src):
   """Function saves file duplicates with new name, origin remains as is."""
   ourDict = defdict(list)
   destDict = {}
   for item in src:
      if "SOURCE" in item:
         t = item.split()[1]
         destDict[t] = t
         m = os.path.split(t)
         ourDict.setdefault(m[1], []).append(m[0])

   ourDict = dict((k, v) for k, v in ourDict.iteritems() if len(v) > 1)

   result = src
   if( len(ourDict.values()) > 0):
      result = []
      fixed = FindDup(ourDict)
      for item in src:
         if "SOURCE" in item:
            dst = item.split()[1]
            if dst in fixed:
               d = item.replace(dst, fixed[dst])
               result.append(d)
            else:
               result.append(item)
   ourDict.clear()
   return result


def FindDup(files):
   val = {}
   # print "File duplicates found:\n %s" %files
   keys = files.keys()
   # print "for keys: %s" %keys
   for k in keys:
      for v in files[k]:
         if len(v) > 0:
            t = MakeRenamePair(v, k)
            src = os.path.join(currentEngine, t[0])
            dst = os.path.join(currentEngine, t[1])
            val[t[0]] = t[1]
            # if(os.path.exists(dst)): # todo: fix possible file duplicates
               # dst = str(hash(dst)) + dst
            # print "Rename file %s to %s" %(src, dst)
            shutil.copyfile(src, dst)
            t = open(dst) # Fail if file not exist
            t.close()
   return val


def MakeRenamePair(val, key):
   # nuvie/core + events.cpp -> nuvie_core + events.cpp -> nuvie/core + nuvie_core_events.cpp -> nuvie/core/nuvie_core_events.cpp
   v = val.replace("/", "_")
   k = v + "_" + key # nuvie_core_events.cpp
   newFile = val + "/" + k #nuvie/core/nuvie_core_events.cpp
   oldFile = val + "/" + key
   #if newFile exists: nuvie_core_events.cpp -> (hash)_nuvie_core_events.cpp
   return [oldFile, newFile]


# This function is frozen! Don't edit!
def SafeWriteFile(path, mode, data):
   """Save list elments as strings. Save strings as is"""
   with open(path, mode) as f:
      if type(data) is list:
         for s in data:
            f.write(s + '\n')
      else:
         f.write(data)


def FilterGlk(src):
   src = [x for x in src if "glulx" not in x]
   print "Exclude glulx engine from detection_tables.h and detection.cpp!"
   return src

def FilterUltima(src):
   src = [x for x in src if "nuvie" not in x] #Ultima VI
   src = [x for x in src if "ultima4" not in x]
   print "Exclude nuvie and ultima4 engines from detection_tables.h and detection.cpp!"
   return src


def DeleteUsuportedSrcs(src, engine):
   if "glk" in engine:
      return FilterGlk(src)
   if "ultima" in engine:
      return FilterUltima(src)
   # if "" in engine:
      # Filter(src)
   return src


guard_macro = "#ifdef SCUMMVM_PT_%s\n#endif // SCUMMVM_PT_%s\n"
def MakeMMP(engine):
   global firstRun, currentEngine
   print "Start processing engine: %s" %engine

   pth = os.path.join(pt, engine)
   currentEngine = pth
   tt = processengine(pth)
   if tt is None:
      return

   macrolist = tt[0]
   staticlib = tt[1]

   src = processModule_mk(pth, macrolist)
   src = DeleteUsuportedSrcs(src, engine)

   mmp = """TARGET scummvm_%s.lib
TARGETTYPE lib\n
#include "../S60v3/build_config.mmh"

#define SCUMMVM_PT_1
#define SCUMMVM_PT_2
#define SCUMMVM_PT_3
#define SCUMMVM_PT_4
#include "macros.mmh"\n
USERINCLUDE    ..\..\..\..\engines\%s\n
// *** SOURCE files
SOURCEPATH   ..\..\..\..\engines\%s\n
""" %(engine, engine, engine)

   print "TARGET scummvm_%s.lib" %engine
   mmp = CheckEngine(mmp, engine)
   if mmp is None:
      return

   src = ProcessDup(src)

   plugins_table = """
#if PLUGIN_ENABLED_STATIC(%s)
LINK_PLUGIN(%s)
#endif
""" %(engine.upper(), engine.upper())

   bldinf = os.path.join(local, "bld.inf")
   mmpfile = os.path.join(local, "%s.mmp" %engine)
   macros = os.path.join(local, "macros.mmh")
   ptable = os.path.join(pt, "plugins_table.h")
   engines = os.path.join(local, "engines.mmh")

#create files and add bld.inf header
   if firstRun is True:
      SafeWriteFile(bldinf, 'w', "PRJ_MMPFILES\n")
      if(build == 'release'):
         SafeWriteFile(macros, 'w', "MACRO RELEASE_BUILD\n")
      guard_macros = [guard_macro %(i + 1, i + 1) for i in range(len(uids))]
      SafeWriteFile(macros, 'w', "// This is autogenerated file.\n")
      SafeWriteFile(macros, 'a', guard_macros)
      SafeWriteFile(ptable, 'w', "// This is autogenerated file.\n")
      SafeWriteFile(engines, 'w', "// This is autogenerated file.\n")
      SafeWriteFile(engines, 'a', guard_macros)
      firstRun = False

   SafeWriteFile(mmpfile, 'w', mmp)
   SafeWriteFile(mmpfile, 'a', src)

   SafeWriteFile(bldinf, 'a', "    %s.mmp\n" %engine)
   SafeWriteFile(macros, 'a', macrolist)
   SafeWriteFile(engines, 'a', staticlib)
   SafeWriteFile(ptable, 'a', plugins_table)
   currentEngine = None
   print "End processing\n"

firstRun = True
currentEngine = None
pt = '..\..\..\engines'
local = mmps

# pt = 'e:\Scu\engines'
# local = pt

def create_engine_mmps(arg = 'full'):
   global build
   build = arg
   if(os.path.isfile(local)):
      raise TypeError("Expected dir but file found %s" %pt)
   if(local != pt):
      if(os.path.isdir(local)):
         shutil.rmtree(local)
   os.mkdir(local)
   t = os.listdir(pt)
   [MakeMMP(m) for m in t if os.path.isdir(os.path.join(pt, m))]

if __name__ == "__main__":
   create_engine_mmps()


