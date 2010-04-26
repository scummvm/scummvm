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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"

namespace Prisoner {

int16 PrisonerEngine::addClickBox(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag) {
	int16 clickBoxIndex = _clickBoxes.getFreeSlot();
	ClickBox *clickBox = &_clickBoxes[clickBoxIndex];
	if (x1 > x2) SWAP(x1, x2);
	if (y1 > y2) SWAP(y1, y2);
	clickBox->used = 1;
	clickBox->flag1 = 1;
	clickBox->flag2 = 1;
	clickBox->x1 = x1;
	clickBox->y1 = y1;
	clickBox->x2 = x2;
	clickBox->y2 = y2;
	clickBox->tag = tag;
	return clickBoxIndex;
}

int16 PrisonerEngine::findClickBoxAtPos(int16 x, int16 y, int16 tag) {
	for (int16 clickBoxIndex = 0; clickBoxIndex < kMaxClickBoxes; clickBoxIndex++) {
		ClickBox *clickBox = &_clickBoxes[clickBoxIndex];
		if (clickBox->used == 1 && clickBox->flag1 &&
			(tag == -1 || clickBox->tag == tag) &&
			x >= clickBox->x1 && x <= clickBox->x2 &&
			y >= clickBox->y1 && y <= clickBox->y2) {
				return clickBoxIndex;
			}
	}
	return -1;
}

bool PrisonerEngine::isPointInClickBox(int16 clickBoxIndex, int16 x, int16 y) {
	ClickBox *clickBox = &_clickBoxes[clickBoxIndex];
	return x >= clickBox->x1 && x <= clickBox->x2 && y >= clickBox->y1 && y <= clickBox->y2;
}

int16 PrisonerEngine::getClickBoxTag(int16 clickBoxIndex) {
	return _clickBoxes[clickBoxIndex].tag;
}

} // End of namespace Prisoner
