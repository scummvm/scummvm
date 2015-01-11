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
 *
 */

#include "xeen/scripts.h"

namespace Xeen {

MazeEvent::MazeEvent() : _direction(DIR_ALL), _line(-1), _opcode(OP_None) {
}

void MazeEvent::synchronize(Common::SeekableReadStream &s) {
	int len = s.readByte();
	_position.x = s.readByte();
	_position.y = s.readByte();
	_direction = (Direction)s.readByte();
	_line = s.readByte();
	_opcode = (Opcode)s.readByte();

	for (int i = 0; i < (len - 5); ++i)
		_parameters.push_back(s.readByte());
}

/*------------------------------------------------------------------------*/

void MazeEvents::synchronize(Common::SeekableReadStream &s) {
	MazeEvent e;

	clear();
	while (!s.eos()) {
		e.synchronize(s);
		push_back(e);
	}
}

} // End of namespace Xeen
