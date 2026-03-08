/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/ym2149.h"
#include "audio/softsynth/ym2149.h"

#include "common/textconsole.h"

namespace YM2149 {

YM2149 *Config::create() {
	return new Audio::YM2149Emu();
}

bool YM2149::_hasInstance = false;

YM2149::YM2149() {
	if (_hasInstance)
		error("There are multiple YM2149 output instances running.");
	_hasInstance = true;
}

YM2149::~YM2149() {
	_hasInstance = false;
}

} // End of namespace YM2149
