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

#ifndef MADS_NEBULAR_ROOMS_TELEPORTER_H
#define MADS_NEBULAR_ROOMS_TELEPORTER_H

#include "common/serializer.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Forcefield {
	bool _flag;
	int _vertical;
	int _horizontal;
	int _seqId[40];
	long _timer;

	void init() {
		_flag = false;
		_vertical = _horizontal = -1;
		_timer = 0;
		for (int i = 0; i < 40; ++i)
			_seqId[i] = -1;
	}

	void synchronize(Common::Serializer &s) {
		s.syncAsByte(_flag);
		s.syncAsSint32LE(_vertical);
		s.syncAsSint32LE(_horizontal);
		for (int i = 0; i < 40; ++i)
			s.syncAsSint32LE(_seqId[i]);
		s.syncAsUint32LE(_timer);
	};
};

extern void init_forcefield(Forcefield *force, bool flag);
extern void handle_forcefield(Forcefield *force, int16 *sprites);

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS

#endif
