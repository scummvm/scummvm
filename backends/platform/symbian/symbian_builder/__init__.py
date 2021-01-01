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


from prj_generator import create_mmps, SafeWriteFile
from mmp_gen import create_engine_mmps
from build_apps import build_apps
from create_installers import create_installers
from datetime import datetime
from piper import build_mmp
from common_names import *


def run(build, pkgs):
   log = "Build started at: %s\n" %datetime.now()
   SafeWriteFile(build_log, log)
   SafeWriteFile(build_err, '')
   create_mmps(build, pkgs)
   create_engine_mmps(build)
   
   log = "Build engines started at: %s\n" %datetime.now()
   SafeWriteFile(build_log, log, 'a')
   build_mmp()
   log = "Build engines ended at: %s\n" %datetime.now()
   SafeWriteFile(build_log, log, 'a')
   
   build_apps(pkgs)
   create_installers(pkgs)

   