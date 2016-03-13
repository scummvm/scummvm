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

#include "titanic/messages/messages.h"
#include "titanic/core/game_object.h"
#include "titanic/core/tree_item.h"

namespace Titanic {

CMessage::CMessage() : CSaveableObject() {
}

void CMessage::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
}

void CMessage::load(SimpleFile *file) {
	file->readNumber();
	CSaveableObject::load(file);
}

bool CMessage::execute(CTreeItem *target, const ClassDef *classDef, int flags) {
	// If no target was specified, then there's nothing to do
	if (!target)
		return false;

	bool result = false;
	CTreeItem *item = target;
	CTreeItem *nextItem = nullptr;
	do {
		if (flags & MSGFLAG_SCAN)
			nextItem = item->scan(target);

		if (!(flags & MSGFLAG_CLASS_DEF) || item->isInstanceOf(classDef)) {
			bool handled = perform(item);

			if (handled) {
				result = true;
				if (flags & MSGFLAG_BREAK_IF_HANDLED)
					return true;
			}
		}

		item = nextItem;
	} while (nextItem);

	return result;
}

} // End of namespace Titanic
