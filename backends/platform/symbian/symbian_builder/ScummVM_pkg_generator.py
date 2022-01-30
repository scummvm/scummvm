# ScummVM - Graphic Adventure Engine
# Copyright (C) 2020-2022 Stryzhniou Fiodar

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
from addon_pkg_generator import ResolveEpocRoot

data_path = "..\\..\\..\\dists\\engine-data"
theme_path = "..\\..\\..\\gui\\themes"

# Install_path has trailing '\\'.
install_path = "!:\\data\\scummvm\\"

# Template args = (Component name, UID, major version, minor version, build number).
pkg_template = """;;;
;;; ScummVM .PKG file for .SIS generation.
;;;

; Language - standard language definitions.
&EN

; List of localised vendor names - one per language. At least one must be provided (English [EN]).
; List must correspond to list of languages specified elsewhere in the .pkg
%%{"ScummVM"}
; The non-localised, globally unique vendor name (mandatory).
:"ScummVM"

; UID is the app's UID.
#{"%s"},(%s),%s,%s,%s

;Supports Series 60 v 3.0.
[0x101F7961], 0, 0, 0, {"Series60ProductID"}
;Supports Series 60 v 5.0.
[0x1028315F], 0, 0, 0, {"Series60ProductID"}

; Scummvm help.
"..\\help\\ScummVM.hlp"-"!:\\resource\\help\\ScummVM.hlp"

; Common datafiles needed for some games.
"""
# ends pkg_template

pkg_string_template = "\"%s\" - \"!:\\data\\scummvm\\%s\""
pkg_string_template_beta = "\"%s\" - \"!:\\data\\scummvm\\beta\\%s\""

pkg_licences = """

; Scummvm Documentation, licences.
"..\\..\\..\\..\\COPYRIGHT" - "!:\\data\\scummvm\\COPYRIGHT", FT, TA
"..\\..\\..\\..\\COPYING"   - "!:\\data\\scummvm\\COPYING", FT, TC
"..\\..\\..\\..\\AUTHORS"   - "!:\\data\\scummvm\\AUTHORS", FT, TC
"..\\README"  - "!:\\data\\scummvm\\SYMBIAN_README", FT, TC
"..\\..\\..\\..\\README.md" - "!:\\data\\scummvm\\README.md"
"..\\..\\..\\..\\NEWS.md"   - "!:\\data\\scummvm\\NEWS.md"
"""

license_template = """\"..\\..\\..\\..\\%s\"   - \"!:\\data\\scummvm\\%s\""""
clean_uninstall = """

; Config/log files: 'empty' will automagically be removed on uninstall.
""- "c:\data\scummvm\scummvm.ini", FILENULL
""- "c:\data\scummvm\scummvm.stdout.txt", FILENULL
""- "c:\data\scummvm\scummvm.stderr.txt", FILENULL
""- "c:\data\scummvm\sdl.ini", FILENULL
"""

# SDK examples use full path to file with conditional installation.
# I don't know if statement "!:\path" works everywere.
# "Z:\" is ROM drive
mif_install = """
; Icon in menu
IF NOT (exists(\"A:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"B:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"C:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"D:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"E:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"F:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"G:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"H:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"I:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"J:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"K:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"L:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"M:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"O:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"P:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"Q:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"R:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"S:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"T:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"U:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"V:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"X:\\resource\\apps\\ScummVM.mif\") AND
        exists(\"Y:\\resource\\apps\\ScummVM.mif\")
        )
\"$(EPOCROOT)epoc32\\data\\z\\resource\\apps\\ScummVM.mif\" -	\"!:\\resource\\apps\\ScummVM.mif\"
ENDIF
"""


def makeMIFInstall():
   return ResolveEpocRoot(mif_install)

def get_pkg_licences():
   if isRelease():
      return pkg_licences
   return pkg_licences.replace("scummvm", "scummvm\\beta")

def make_pkg_licences():
   t = os.listdir("..\\..\\..\\")
   return [license_template %(x, x) for x in t if "COPYING" in x]

def make_install_path(x, data_pth):
   data_pth = os.path.join("..", data_pth)
   if isRelease():
      return pkg_string_template %(os.path.join(data_pth, x),x)
   return pkg_string_template_beta %(os.path.join(data_pth, x),x)

def get_gamedata(data_pth):
   files = os.listdir(data_pth)
   files = [make_install_path(x, data_pth) for x in files if os.path.isfile(os.path.join(data_pth, x))]
   return files

def get_component_name():
   if isRelease():
      return "ScummVM %s.%s.%s" %(sis_major_version, sis_minor_version, sis_build_number)
   return "ScummVM %s.%s.%s unstable" %(sis_major_version, sis_minor_version, sis_build_number)

def make_pkg_template():
   uids = get_UIDs()
   comp_name = get_component_name()
   p_template = pkg_template %(comp_name, uids[0], sis_major_version, sis_minor_version, sis_build_number)
   if isRelease():
      return p_template
   return p_template.replace("resource\\help\\ScummVM.hlp", "resource\\help\\ScummVM_test.hlp")

def GenerateScummVMPackage(platform = "S60v3"):
   pkg = os.path.join(platform, MainInstallerName())
   SafeWriteFile(pkg,  make_pkg_template())
   AppendToFile(pkg, get_gamedata(data_path))
   AppendToFile(pkg, get_gamedata(theme_path))
   AppendToFile(pkg, makeMIFInstall())
   AppendToFile(pkg, get_pkg_licences())
   AppendToFile(pkg, make_pkg_licences())
   AppendToFile(pkg, clean_uninstall)

if __name__ == "__main__":
   SetFullBuild()
   GenerateScummVMPackage(platform = "S60v3")
