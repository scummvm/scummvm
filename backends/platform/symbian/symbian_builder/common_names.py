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

from datetime import datetime

mmps = "mmp"
pipe = "pipe"

timestamp = datetime.now().strftime('_%H_%M_%d_%m_%Y')

build_log = "ScummVM_build%s.log" %timestamp
build_err = "ScummVM_build%s.err" %timestamp

whitelist = 'whitelist.txt'

build = 'full'
# build = 'release'

uids = ("0xA0000657", "0xA0000658", "0x2006FE7C", "0x2006FE7D", "0x2006FE7E", "0x2006FE7F", "0x2006FE80")
# Test uids 3
uids_tests = ("0xAA000657", "0xAA000658", "0xA006FE7C", "0xA006FE7D", "0xA006FE7E", "0xA006FE7F", "0xA006FE80")

def get_UIDs(build):
   if build == 'full':
      return uids_tests
   return uids

active_config = ("DISABLE_NUKED_OPL", "USE_A52", "USE_MPEG2",  "USE_BINK", "USE_THEORADEC", "USE_TINYGL",
"ENABLE_VKEYBD")
#activate USE_SCALERS USE_ARM_SCALER_ASM USE_TTS USE_SPEECH_DISPATCHER USE_CLOUD USE_LIBCURL
# USE_SDL_NET USE_DISCORD USE_UPDATES USE_LUA
disabled_config = ("USE_ALSA", "ENABLE_OPL2LPT", "USE_SCALERS", "USE_ARM_SCALER_ASM",
"USE_HQ_SCALERS", "USE_NASM", "USE_ELF_LOADER", "USE_SDL2", "USE_FLUIDSYNTH", "USE_TTS", "USE_SPEECH_DISPATCHER",
"USE_CLOUD", "USE_LIBCURL", "USE_SDL_NET", "USE_OPENGL", "USE_DISCORD", "USE_LINUXCD", "ENABLE_EVENTRECORDER",
"USE_UPDATES", "USE_LUA")

if __name__ == "__main__":
   print "This script holds together data used multiple modules."
