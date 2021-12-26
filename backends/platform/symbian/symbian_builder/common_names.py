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

from datetime import datetime

mmps = "mmp"
pipe = "pipe"

timestamp = datetime.now().strftime('_%H_%M_%d_%m_%Y')

build_log = "ScummVM_build%s.log" %timestamp
build_err = "ScummVM_build%s.err" %timestamp
build_time = "ScummVM_build_time%s.log" %timestamp

whitelist = 'whitelist.txt'

build = 'full'
# build = 'release'

#sis package version
sis_major_version, sis_minor_version, sis_build_number = 2, '03', 0


uids = ("0xA0000657", "0xA0000658", "0x2006FE7C", "0x2006FE7D", "0x2006FE7E", "0x2006FE7F", "0x2006FE80", "0x2006FE81", "0x2006FE82")
# Test uids 3
uids_tests = ("0xAA000657", "0xAA000658", "0xA006FE7C", "0xA006FE7D", "0xA006FE7E", "0xA006FE7F", "0xA006FE80", "0xA006FE81", "0xA006FE82")

def get_UIDs(build):
   if build == 'full':
      return uids_tests
   return uids

def SafeWriteFile(path, data, mode = 'w'):
   """Save list elements as strings. Save strings as is"""
   with open(path, mode) as f:
      if type(data) is list:
         for s in data:
            f.write(s + '\n')
      else:
         f.write(data)

def AppendToFile(path, data):
   SafeWriteFile(path, data, mode = 'a')

active_config = ("DISABLE_NUKED_OPL", "USE_A52", "USE_MPEG2",  "USE_BINK", "USE_THEORADEC", "USE_TINYGL",
"ENABLE_VKEYBD")
#activate USE_SCALERS USE_ARM_SCALER_ASM USE_TTS USE_SPEECH_DISPATCHER USE_CLOUD USE_LIBCURL
# USE_SDL_NET USE_DISCORD USE_UPDATES
#LUA disabled by default due to conflicting declaration of struct TKey in lobject.h and system header e32std.h
disabled_config = ("USE_ALSA", "ENABLE_OPL2LPT", "USE_SCALERS", "USE_ARM_SCALER_ASM",
"USE_HQ_SCALERS", "USE_NASM", "USE_ELF_LOADER", "USE_SDL2", "USE_FLUIDSYNTH", "USE_TTS", "USE_SPEECH_DISPATCHER",
"USE_CLOUD", "USE_LIBCURL", "USE_SDL_NET", "USE_OPENGL", "USE_DISCORD", "USE_LINUXCD", "ENABLE_EVENTRECORDER",
"USE_UPDATES", "USE_LUA")

if __name__ == "__main__":
   print "This script holds together data used multiple modules."
