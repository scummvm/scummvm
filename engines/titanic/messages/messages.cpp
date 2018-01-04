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
#include "titanic/core/message_target.h"
#include "titanic/core/project_item.h"
#include "titanic/core/tree_item.h"
#include "titanic/main_game_window.h"
#include "titanic/messages/mouse_messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/titanic.h"

namespace Titanic {

CMessage::CMessage() : CSaveableObject() {
}

void CMessage::save(SimpleFile *file, int indent) {
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

		if (!classDef || item->isInstanceOf(classDef)) {
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

bool CMessage::execute(const CString &target, const ClassDef *classDef, int flags) {
	// Scan for the target by name
	CProjectItem *project = g_vm->_window->_project;
	for (CTreeItem *treeItem = project; treeItem; treeItem = treeItem->scan(project)) {
		if (!treeItem->getName().compareToIgnoreCase(target))
			return execute(treeItem, classDef, flags);
	}

	return false;
}

const MSGMAP_ENTRY *CMessage::findMapEntry(const CTreeItem *treeItem, const ClassDef *classDef) {
	// Iterate through the class and any parent classes
	for (const MSGMAP *msgMap = treeItem->getMessageMap(); msgMap->pFnGetBaseMap;
			msgMap = msgMap->pFnGetBaseMap()) {
		// Iterate through the map entries for this class
		for (const MSGMAP_ENTRY *entry = msgMap->lpEntries;
				entry->_class != nullptr; ++entry) {
			// Check if the class or any of it's ancesotrs is handled by this entry
			for (const ClassDef *entryDef = *entry->_class; entryDef;
					entryDef = entryDef->_parent) {
				if (entryDef == classDef)
					return entry;
			}
		}
	}

	return nullptr;
}

bool CMessage::perform(CTreeItem *treeItem) {
	const MSGMAP_ENTRY *entry = findMapEntry(treeItem, getType());
	return entry && (*treeItem.*(entry->_fn))(this);
}

bool CMessage::supports(const CTreeItem *treeItem, ClassDef *classDef) {
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

bool CMessage::isEnterRoomMsg() const {
	return dynamic_cast<const CEnterRoomMsg *>(this) != nullptr;
}

bool CMessage::isPreEnterRoomMsg() const {
	return dynamic_cast<const CPreEnterRoomMsg *>(this) != nullptr;
}

bool CMessage::isleaveRoomMsg() const {
	return dynamic_cast<const CLeaveRoomMsg *>(this) != nullptr;
}

bool CMessage::isEnterNodeMsg() const {
	return dynamic_cast<const CEnterNodeMsg *>(this) != nullptr;
}

bool CMessage::isPreEnterNodeMsg() const {
	return dynamic_cast<const CPreEnterNodeMsg *>(this) != nullptr;
}

bool CMessage::isLeaveNodeMsg() const {
	return dynamic_cast<const CLeaveNodeMsg *>(this) != nullptr;
}

bool CMessage::isEnterViewMsg() const {
	return dynamic_cast<const CEnterViewMsg *>(this) != nullptr;
}

bool CMessage::isPreEnterViewMsg() const {
	return dynamic_cast<const CPreEnterViewMsg *>(this) != nullptr;
}

bool CMessage::isLeaveViewMsg() const {
	return dynamic_cast<const CLeaveViewMsg *>(this) != nullptr;
}

/*------------------------------------------------------------------------*/

CShowTextMsg::CShowTextMsg() : CMessage(), _message("NO TEXT INCLUDED!!!") {
}

CShowTextMsg::CShowTextMsg(const CString &msg) : CMessage(), _message(msg) {
}

CShowTextMsg::CShowTextMsg(StringId stringId) : CMessage() {
	_message = g_vm->_strings[stringId];
}

/*------------------------------------------------------------------------*/

Movement CMovementMsg::getMovement(Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		return TURN_LEFT;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		return TURN_RIGHT;
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		return MOVE_FORWARDS;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		return MOVE_BACKWARDS;
	default:
		return MOVE_NONE;
	}
}

} // End of namespace Titanic
