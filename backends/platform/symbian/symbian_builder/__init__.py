# ScummVM - Graphic Adventure Engine
# Copyright (C) 2020 Stryzhniou Fiodar

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


from prj_generator import create_mmps
from mmp_gen import create_engine_mmps
from build_apps import build_apps
from create_installers import create_installers
from datetime import datetime
import time

from piper import build_mmp
from parse_codecs import parse_codecs
from parse_base import parse_base
from common_names import *
from scummvm_hrh import Generate_ScummVm_hrh
from parse_lua import parse_lua
from parse_gamedata import parse_gamedata
from pkg_generator import create_pkgs
from source_patcher import fix_src

def minimal_whitelist(engine):
   data = "#ifdef SCUMMVM_PT_1\nscummvm_base.mmp\nscummvm_codecs.mmp\n%s\n#endif // SCUMMVM_PT_%s\n" %engine
   SafeWriteFile(os.path.join(mmps, whitelist), data)

def run(build, pkgs, only_begotten_engine = None):
   log = "Build started at: %s\n" %time.ctime()
   SafeWriteFile(build_log, log)
   SafeWriteFile(build_err, '')
   Generate_ScummVm_hrh(build)
   create_mmps(build, pkgs)
   create_engine_mmps(build)
   parse_codecs(pkgs)
   parse_base()
   parse_lua()
   parse_gamedata(build, pkgs)
   create_pkgs(build, pkgs)
   fix_src()

   SafeWriteFile(build_log, log, 'a')
   if only_begotten_engine:
      minimal_whitelist(build_log)
   build_mmp(only_begotten_engine)
   log = "Build engines ended at: %s\n" %time.ctime()
   SafeWriteFile(build_log, log, 'a')

   build_apps(pkgs)
   create_installers(pkgs)

