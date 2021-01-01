# ScummVM - Graphic Adventure Engine
# Copyright (C) 2020-2021 Stryzhniou Fiodar

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


import subprocess
from prj_generator import SafeWriteFile
from common_names import *


def build_apps(plats = "S60v3"):
   cmd = subprocess.Popen('bldmake bldfiles', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=plats, shell=True)
   out, err = cmd.communicate()
   cmd1 = subprocess.Popen('abld build gcce urel', stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=plats, shell=True)
   out1, err1 = cmd1.communicate()
   SafeWriteFile(build_log, out + out1, 'a')
   SafeWriteFile(build_err, err + err1, 'a')

if __name__ == "__main__":
   build_apps(plats = "S60v3")
