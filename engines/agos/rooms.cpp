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

#include "agos/agos.h"
#include "agos/intern.h"

using Common::File;

namespace AGOS {

uint16 AGOSEngine::getDoorOf(Item *i, uint16 d) {
	SubGenExit *g;
	Item *x;

	g = (SubGenExit *)findChildOfType(i, 4);
	if (g == NULL)
		return 0;

	x = derefItem(g->dest[d]);
	if (x == NULL)
		return 0;
	if (findChildOfType(x, 1))
		return 0;
	return itemPtrToID(x);
}

uint16 AGOSEngine::getDoorState(Item *item, uint16 d) {
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

uint16 AGOSEngine::getExitOf_e1(Item *item, uint16 d) {
	SubGenExit *g;
	Item *x;

	g = (SubGenExit *)findChildOfType(item, 4);
	if (g == NULL)
		return 0;

	x = derefItem(g->dest[d]);
	if (x == NULL)
		return 0;
	if (findChildOfType(x, 1))
		return itemPtrToID(x);
	if (x->state != 0)
		return 0;
	return x->parent;
}

uint16 AGOSEngine::getExitOf(Item *item, uint16 d) {
	SubRoom *subRoom;
	uint16 x;
	uint16 y = 0;

	subRoom = (SubRoom *)findChildOfType(item, 1);
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

uint16 AGOSEngine::getExitState(Item *i, uint16 x, uint16 d) {
	SubSuperRoom *sr;
	uint16 mask = 3;
	uint16 n;
	uint16 *c;

	sr = (SubSuperRoom *)findChildOfType(i, 4);
	if (sr == NULL)
	    return 0;

	c = sr->roomExitStates;
	c += x - 1;
	d <<= 1;
	mask <<= d;
	n = *c & mask;
	n >>= d;
	return n;
}

void AGOSEngine::moveDirn_e1(Item *i, uint x) {
	Item *d, *p;
	uint16 n;

	if (i->parent == 0)
		return;

	p = derefItem(i->parent);

	n = getExitOf_e1(p, x);
	d = derefItem(n);
	if (n) {
		if (canPlace(i, d))
			return;

		setItemParent(i, d);
		return;
	}

	d = derefItem(getDoorOf(p, x));
	if (d) {
		const byte *name = getStringPtrByID(d->itemName);
		if (d->state == 1)
			showMessageFormat("%s is closed.\n", name);
		else
			showMessageFormat("%s is locked.\n", name);
		return;
	}

	showMessageFormat("You can't go that way.\n");
}

void AGOSEngine::moveDirn_e2(Item *i, uint x) {
	SubSuperRoom *sr;
	Item *d, *p;
	uint16 a, n;

	if (i->parent == 0)
		return;

	p = derefItem(i->parent);
	if (findChildOfType(p, 4)) {
		n = getExitState(p, _superRoomNumber,x);
		if (n == 1) {
			sr = (SubSuperRoom *)findChildOfType(p, 4);
			switch (x) {
				case 0: a = -(sr->roomX); break;
				case 1: a = 1; break;
				case 2: a = sr->roomX; break;
				case 3: a = (uint)-1; break;
				case 4: a = -(sr->roomX * sr->roomY); break;
				case 5: a = (sr->roomX * sr->roomY); break;
				default: return;
			}
			_superRoomNumber += a;
		}
		return;
	}

	n = getExitOf(derefItem(i->parent), x);
	if (derefItem(n) == NULL) {
		loadRoomItems(n);
		n=getExitOf(derefItem(i->parent), x);
	}

	d = derefItem(n);
	if (d) {
		n = getDoorState(derefItem(i->parent), x);
		if (n == 1) {
			if (!canPlace(i, d))
				setItemParent(i, d);
		}
	}
}

void AGOSEngine::moveDirn_ww(Item *i, uint x) {
	Item *d;
	uint16 n;

	if (i->parent == 0)
		return;

	n = getExitOf(derefItem(i->parent), x);
	if (derefItem(n) == NULL) {
		loadRoomItems(n);
		n = getExitOf(derefItem(i->parent), x);
	}

	d = derefItem(n);
	if (d) {
		n = getDoorState(derefItem(i->parent), x);
		if (n == 1) {
			if(!canPlace(i, d))
				setItemParent(i, d);
		}
	}
}

bool AGOSEngine::loadRoomItems(uint item) {
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
			min_num = READ_BE_UINT16(p); p += 2;
			if (min_num == 0)
				break;

			max_num = READ_BE_UINT16(p); p += 2;

			if (item >= min_num && item <= max_num) {

				in.open(filename);
				if (in.isOpen() == false) {
					error("loadRoomItems: Can't load rooms file '%s'", filename);
				}

				while ((i = in.readUint16BE()) != 0) {
					_itemArrayPtr[i + 1] = (Item *)allocateItem(sizeof(Item));
					readItemFromGamePc(&in, _itemArrayPtr[i + 1]);

					Item *tmp = _itemArrayPtr[i + 1];
					tmp->child = 0;
					tmp->parent = 0;
				}
				in.close();

				return 1;
			}
		}
	}

	debug(1,"loadRoomItems: didn't find %d", item);
	return 0;
}

} // End of namespace AGOS
