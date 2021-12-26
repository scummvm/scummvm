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

pkg_file = "ScummVM_gamedata.pkg"
component_name = "ScummVM gamedata package"
component_name_test = "ScummVM teting gamedata package"
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
#{"%s"},(%s),%s,%s,%s,  TYPE=SISPATCH

;Supports Series 60 v 3.0.
[0x101F7961], 0, 0, 0, {"Series60ProductID"}
;Supports Series 60 v 5.0.
[0x1028315F], 0, 0, 0, {"Series60ProductID"}
; Scummvm documentation.
"..\\..\\..\\..\\COPYRIGHT"        - "", FT, TC
"..\\..\\..\\..\\COPYING.FREEFONT" - "!:\\data\\scummvm\\COPYING.FREEFONT"
"..\\..\\..\\..\\AUTHORS"          - "", FT, TC

; Scummvm help.
"..\\help\\ScummVM.hlp"-"!:\\resource\\help\\ScummVM.hlp"

; Common datafiles needed for some games.
"""

pkg_string_template = "\"%s\" - \"!:\\data\\scummvm\\%s\""
pkg_string_template_beta = "\"%s\" - \"!:\\data\\scummvm\\beta\\%s\""


def make_install_path(x, data_pth, build):
   data_pth = os.path.join("..", data_pth)
   if build == 'full':
      return pkg_string_template_beta %(os.path.join(data_pth, x),x)
   return pkg_string_template %(os.path.join(data_pth, x),x)

def get_gamedata(data_pth, build):
   files = os.listdir(data_pth)
   files = [make_install_path(x, data_pth, build) for x in files if os.path.isfile(os.path.join(data_pth, x))]
   return files

def parse_gamedata(build, platform = "S60v3"):
   uids = get_UIDs(build)
   comp_name = component_name
   if build == 'full':
      comp_name = component_name_test
   t = get_gamedata(data_path, build)
   t += get_gamedata(theme_path, build)
   pkg = os.path.join(platform, pkg_file)
   p_template = pkg_template %(comp_name, uids[0], sis_major_version, sis_minor_version, sis_build_number)
   if build == 'full':
      p_template = p_template.replace("resource\\help\\ScummVM.hlp", "resource\\help\\ScummVM_test.hlp")
   SafeWriteFile(pkg,  p_template)
   SafeWriteFile(pkg, t, mode = 'a')


if __name__ == "__main__":
   parse_gamedata(build = 'full', platform = "S60v3")
