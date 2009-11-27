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

#include "asylum/actionarray.h"

namespace Asylum {

ActionArray::ActionArray(AsylumEngine *engine): _vm(engine) {
	_actionFlag = false;
	reset();
}

ActionArray::~ActionArray() {

}

void ActionArray::reset() {
	memset(&_items, 0, sizeof(ActionStruct));
	for (int i = 0; i < 10; i++)
		_items.entries[i].actionListIndex = -1;
}

void ActionArray::initItem(ActionDefinitions *command, int actionIndex, int actorIndex) {
	// TODO properly define what actionFlag is actually for.
	// It appears to remain false 99% of the time, so I'm guessing
	// it's a "skip processing" flag.
	if (!_actionFlag) {
		int i;
		// iterate through the availble entry slots to determine
		// the next available slot
		for (i = 1; i < 10; i++) {
			if (_items.entries[i].actionListIndex == -1)
				break;
		}

		command->counter = 0;

		_items.entries[i].field_10 = 0;
		_items.entries[i].field_C = 0;

		if (_items.count) {
			_items.entries[_items.field_CC].field_C = i ;
			_items.entries[0].field_10 = _items.field_CC;
		} else {
			_items.count = i;
		}
		_items.field_CC = i;
		_items.entries[0].actionListIndex = actionIndex;
		_items.entries[0].actionListItemIndex = 0;
		_items.entries[0].actorIndex = actorIndex;
	}
}

} // end of namespace Asylum
