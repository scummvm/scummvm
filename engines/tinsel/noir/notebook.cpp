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

#include "tinsel/noir/notebook.h"

#include "tinsel/dialogs.h"

namespace Tinsel {

void Notebook::AddHyperlink(int32 id1, int32 id2) {
	auto *invObject = _vm->_dialogs->GetInvObject(id1);

	if (invObject->getTitle() != 0) {
		error("A clue can only be hyperlinked if it only has one title!");
		return;
	}

	invObject = _vm->_dialogs->GetInvObject(id2);

	if (invObject->getTitle() != 0) {
		error("A clue can only be hyperlinked if it only has one title!");
		return;
	}

	uint32 i;
	for (i = 0; i < MAX_HYPERS; ++i) {
		int32 curr_id1 = _hyperlinks[i].id1;
		if (curr_id1 == 0) {
			_hyperlinks[i].id1 = id1;
			_hyperlinks[i].id2 = id2;
			return;
		}

		if ((curr_id1 == id1) || (id1 == _hyperlinks[i].id2)) {
			if ((curr_id1 != id2) && (id2 != _hyperlinks[i].id2)) {
				error("A clue/title can only be hyperlinked to one other clue/title!");
			}
			return;
		}
	}

	// No free hyperlink entry was found
	error("Too many hyperlinks");
}


void Notebook::Show(bool isOpen) {
	error("TODO: Implement Notebook::Show()");
}

bool Notebook::IsOpen() const {
	return _state != BOOKSTATE::CLOSED;
}

} // End of namespace Tinsel
