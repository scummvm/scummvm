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

#include "ultima/shared/engine/messages.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/gfx/screen.h"
#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {

CMessage::CMessage() : BaseObject() {
}

bool CMessage::execute(TreeItem *target, const ClassDef *classDef, int flags) {
	// If no target was specified, then there's nothing to do
	if (!target)
		return false;

	bool result = false;
	TreeItem *item = target;
	TreeItem *nextItem = nullptr;
	do {
		if (flags & MSGFLAG_SCAN)
			nextItem = item->scan(target);

		if (!classDef || item->isInstanceOf(*classDef)) {
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

bool CMessage::execute(const Common::String &target, const ClassDef *classDef, int flags) {
	// Scan for the target by name
	GameBase *game = g_vm->_game;
	for (TreeItem *treeItem = game; treeItem; treeItem = treeItem->scan(game)) {
		if (!treeItem->getName().compareToIgnoreCase(target))
			return execute(treeItem, classDef, flags);
	}

	return false;
}

const MSGMAP_ENTRY *CMessage::findMapEntry(const TreeItem *treeItem, const ClassDef &classDef) {
	// Iterate through the class and any parent classes
	for (const MSGMAP *msgMap = treeItem->getMessageMap(); msgMap->pFnGetBaseMap;
		msgMap = msgMap->pFnGetBaseMap()) {
		// Iterate through the map entries for this class
		for (const MSGMAP_ENTRY *entry = msgMap->lpEntries;
			entry->_classDef != nullptr; ++entry) {
			// Check if the class or any of it's ancesotrs is handled by this entry
			for (ClassDef def = (*entry->_classDef)(); def.hasParent(); def = def.parent()) {
				if (def == classDef)
					return entry;
			}
		}
	}

	return nullptr;
}

bool CMessage::perform(TreeItem *treeItem) {
	const MSGMAP_ENTRY *entry = findMapEntry(treeItem, getType());
	return entry && (*treeItem.*(entry->_fn))(this);
}

bool CMessage::supports(const TreeItem *treeItem, const ClassDef &classDef) {
	return findMapEntry(treeItem, classDef) != nullptr;
}

bool CMessage::isMouseMsg() const {
	return dynamic_cast<const CMouseMsg *>(this) != nullptr;
}

bool CMessage::isButtonDownMsg() const {
	return dynamic_cast<const CMouseButtonDownMsg *>(this) != nullptr;
}

bool CMessage::isButtonUpMsg() const {
	return dynamic_cast<const CMouseButtonUpMsg *>(this) != nullptr;
}

bool CMessage::isMouseMoveMsg() const {
	return dynamic_cast<const CMouseMoveMsg *>(this) != nullptr;
}

bool CMessage::isDoubleClickMsg() const {
	return dynamic_cast<const CMouseDoubleClickMsg *>(this) != nullptr;
}

} // End of namespace Shared
} // End of namespace Ultima
