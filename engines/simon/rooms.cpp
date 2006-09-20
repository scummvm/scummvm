/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "simon/simon.h"
#include "simon/intern.h"

using Common::File;

namespace Simon {

uint16 SimonEngine::getDoorState(Item *item, uint16 d) {
	uint16 mask = 3;
	uint16 n;

	SubRoom *subRoom = (SubRoom *)findChildOfType(item, 1);
	if (subRoom == NULL)
	    return 0;

	d <<= 1;
	mask <<= d;
	n = subRoom->roomExitStates & mask;
	n >>= d;

	return n;
}

uint16 SimonEngine::getExitOf(Item *item, uint16 d) {
	uint16 x;
	uint16 y = 0;

	SubRoom *subRoom = (SubRoom *)findChildOfType(item, 1);
	if (subRoom == NULL)
		return 0;
	x = d;
	while (x > y) {
		if (getDoorState(item, y) == 0)
			d--;
		y++;
	}
	return subRoom->roomExit[d];
}

bool SimonEngine::loadRoomItems(uint item) {
	byte *p;
	uint i, min_num, max_num;
	char filename[30];
	File in;

	p = _roomsList;
	if (p == NULL)
		return 0;

	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for (;;) {
			min_num = (p[0] * 256) | p[1];
			p += 2;

			if (min_num == 0)
				break;

			max_num = (p[0] * 256) | p[1];
			p += 2;

			if (item >= min_num && item <= max_num) {

				in.open(filename);
				if (in.isOpen() == false) {
					error("loadRoomItems: Can't load rooms file '%s'", filename);
				}

				for (i = min_num; i <= max_num; i++) {
					_itemArrayPtr[i] = (Item *)allocateItem(sizeof(Item));
					in.readUint16BE();
					readItemFromGamePc(&in, _itemArrayPtr[i]);
				}
				in.close();

				return 1;
			}
		}
	}

	debug(1,"loadRoomItems: didn't find %d", item);
	return 0;
}

} // End of namespace Simon
