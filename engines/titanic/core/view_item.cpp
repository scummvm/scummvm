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

#include "titanic/game_manager.h"
#include "titanic/screen_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/core/room_item.h"
#include "titanic/core/view_item.h"
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CViewItem::CViewItem() : CNamedItem() {
	Common::fill(&_buttonUpTargets[0], &_buttonUpTargets[3], nullptr);
	_field24 = 0;
	_field28 = 0.0;
	_viewNumber = 0;
	_field50 = 0;
	_field54 = 0;
	setData(0.0);
}

void CViewItem::setData(double v) {
	_field28 = v;
	_field50 = cos(_field28) * 30.0;
	_field54 = sin(_field28) * -30.0;
}

void CViewItem::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	_resourceKey.save(file, indent);
	file->writeQuotedLine("V", indent);
	file->writeFloatLine(_field28, indent + 1);
	file->writeNumberLine(_viewNumber, indent + 1);

	CNamedItem::save(file, indent);
}

void CViewItem::load(SimpleFile *file) {
	int val = file->readNumber();
	
	switch (val) {
	case 1:
		_resourceKey.load(file);
		// Deliberate fall-through

	default:
		file->readBuffer();
		setData(file->readFloat());
		_viewNumber = file->readNumber();
		break;
	}

	CNamedItem::load(file);
}

bool CViewItem::getResourceKey(CResourceKey *key) {
	*key = _resourceKey;
	CString filename = key->exists();
	return !filename.empty();
}

void CViewItem::leaveView(CViewItem *newView) {
	// Only do the processing if we've been passed a view, and it's not the same 
	if (newView && newView != this) {
		CLeaveViewMsg viewMsg(this, newView);
		viewMsg.execute(this, nullptr, MSGFLAG_SCAN);

		CNodeItem *oldNode = findNode();
		CNodeItem *newNode = newView->findNode();
		if (newNode != oldNode) {
			CLeaveNodeMsg nodeMsg(oldNode, newNode);
			nodeMsg.execute(oldNode, nullptr, MSGFLAG_SCAN);

			CRoomItem *oldRoom = oldNode->findRoom();
			CRoomItem *newRoom = newNode->findRoom();
			if (newRoom != oldRoom) {
				CGameManager *gm = getGameManager();
				if (gm)
					gm->viewChange();

				CLeaveRoomMsg roomMsg(oldRoom, newRoom);
				roomMsg.execute(oldRoom, nullptr, MSGFLAG_SCAN);
			}
		}
	}
}

void CViewItem::preEnterView(CViewItem *newView) {
	// Only do the processing if we've been passed a view, and it's not the same 
	if (newView && newView != this) {
		CPreEnterViewMsg viewMsg(this, newView);
		viewMsg.execute(this, nullptr, MSGFLAG_SCAN);

		CNodeItem *oldNode = findNode();
		CNodeItem *newNode = newView->findNode();
		if (newNode != oldNode) {
			CPreEnterNodeMsg nodeMsg(oldNode, newNode);
			nodeMsg.execute(oldNode, nullptr, MSGFLAG_SCAN);

			CRoomItem *oldRoom = oldNode->findRoom();
			CRoomItem *newRoom = newNode->findRoom();
			if (newRoom != oldRoom) {
				CPreEnterRoomMsg roomMsg(oldRoom, newRoom);
				roomMsg.execute(oldRoom, nullptr, MSGFLAG_SCAN);
			}
		}
	}
}

void CViewItem::enterView(CViewItem *newView) {
	// Only do the processing if we've been passed a view, and it's not the same 
	if (newView && newView != this) {
		CEnterViewMsg viewMsg(this, newView);
		viewMsg.execute(this, nullptr, MSGFLAG_SCAN);

		CNodeItem *oldNode = findNode();
		CNodeItem *newNode = newView->findNode();
		if (newNode != oldNode) {
			CEnterNodeMsg nodeMsg(oldNode, newNode);
			nodeMsg.execute(oldNode, nullptr, MSGFLAG_SCAN);

			CRoomItem *oldRoom = oldNode->findRoom();
			CRoomItem *newRoom = newNode->findRoom();

			CPetControl *petControl = nullptr;
			if (newRoom != nullptr) {
				petControl = newRoom->getRoot()->getPetControl();
				petControl->enterNode(newNode);
			}

			if (newRoom != oldRoom) {
				CEnterRoomMsg roomMsg(oldRoom, newRoom);
				roomMsg.execute(oldRoom, nullptr, MSGFLAG_SCAN);

				if (petControl)
					petControl->enterRoom(newRoom);
			}
		}
	}
}

bool CViewItem::handleEvent(CMouseButtonDownMsg &msg) {
	if (msg._buttons & MB_LEFT) {
		mouseChange(&msg, true);
		// TODO
	}

	return true;
}

bool CViewItem::mouseChange(const CMouseMsg *msg, bool flag) {
	const CMouseButtonUpMsg *upMsg = dynamic_cast<const CMouseButtonUpMsg *>(msg);
	if (msg->isButtonUpMsg()) {
		mouseButtonUp(upMsg);
		return true;
	}

	Common::Array<CGameObject *> gameObjects;
	CTreeItem *treeItem = scan(this);
	while (treeItem) {
		CGameObject *gameObject = dynamic_cast<CGameObject *>(treeItem);
		if (gameObject) {
			if (gameObject->checkPoint(msg->_mousePos, 0, 1) &&
					(!flag || !gameObject->_field60)) {
				if (gameObjects.size() < 256)
					gameObjects.push_back(gameObject);
			}
		}
	}

	const CMouseMoveMsg *moveMsg = dynamic_cast<const CMouseMoveMsg *>(msg);
	if (moveMsg) {
		if (gameObjects.size() == 0)
			return false;

		for (int idx = (int)gameObjects.size() - 1; idx >= 0; ++idx) {
			if (gameObjects[idx]->_cursorId != 12) {
				CScreenManager::_screenManagerPtr->_mouseCursor->setCursorId(gameObjects[idx]->_cursorId);
				break;
			}
		}
	}

	bool result = false;
	for (int idx = (int)gameObjects.size() - 1; idx >= 0; --idx) {
		if (msg->execute(gameObjects[idx])) {
			if (msg->isButtonDownMsg())
				_buttonUpTargets[msg->_buttons >> 1] = gameObjects[idx];
			return true;
		}

		// TODO
	}

	return result;
}

void CViewItem::mouseButtonUp(const CMouseButtonUpMsg *msg) {
	CTreeItem *&target = _buttonUpTargets[msg->_buttons >> 1];

	if (target) {
		msg->execute(target);
		target = nullptr;
	}
}

} // End of namespace Titanic
