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

#include "audio/sid.h"
#include "audio/softsynth/sid.h"

#include "common/textconsole.h"

namespace SID {

SID *Config::create(SidType type) {
#ifdef USE_SID_AUDIO
	// For now this is fixed to the ReSID emulator.
	Resid::SID *sid = new Resid::SID(type);
	return sid;
#else
	return nullptr;
#endif
}

bool SID::_hasInstance = false;

SID::SID() {
	if (_hasInstance)
		error("There are multiple SID output instances running.");
	_hasInstance = true;
}

SID::~SID() {
	_hasInstance = false;
}

} // End of namespace SID
