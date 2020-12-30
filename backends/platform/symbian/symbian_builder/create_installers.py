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


from __future__ import with_statement
from prj_generator import SafeWriteFile
import os, subprocess
from common_names import *


def makesis(pkg, path):
   t = "makesis -d%EPOCROOT% %s" %pkg
   cmd = subprocess.Popen(t, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=path, shell=True)
   out, err = cmd.communicate()
   SafeWriteFile(build_log, out)
   SafeWriteFile(build_err, err)

def create_installers(path = "S60v3"):
   t = os.listdir(path)
   [makesis(pkg, path) for pkg in t if ".pkg" in pkg]
