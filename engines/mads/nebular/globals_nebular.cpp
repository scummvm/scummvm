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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/nebular/globals_nebular.h"

namespace MADS {

namespace Nebular {

Globals::Globals() {
	// Initialize lists
	_flags.resize(210);
	_spriteIndexes.resize(30);
	_sequenceIndexes.resize(30);

	// Initialize game flags
	_timebombClock = 0;
	_timebombTimer = 0;
	_v0 = 0;
	_frameTime = 0;
	_v2 = 0;
	_v3 = 0;
	_v4 = 0;
	_v5 = 0;
	_v6 = 0;
	_v7 = 0;
	_v8 = 0;
	_abortVal = 0;
	_v84262 = 0;
	_v84264 = 0;
	_v84266 = 0;
	_v84268 = 0;
}

void Globals::reset() {
	for (uint i = 0; i < _flags.size(); ++i)
		_flags[i] = 0;
}

} // End of namespace Nebular

} // End of namespace MADS
