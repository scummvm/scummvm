/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AUDIO_SOFTSYNTH_OPL_MAME_FAKE_DEVICE_H
#define AUDIO_SOFTSYNTH_OPL_MAME_FAKE_DEVICE_H

#include "common/textconsole.h"
#include "common/str.h"

struct device_t {
	void *state;
	device_t() : state(nullptr) {}
	void logerror(const char *s, ...) {
		Common::String output;
		va_list va;

		va_start(va, s);
		output = Common::String::vformat(s, va);
		va_end(va);

		warning("%s", output.c_str());
	}
};

#endif // AUDIO_SOFTSYNTH_OPL_MAME_FAKE_DEVICE_H
