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
#include "titanic/support/screen_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/core/room_item.h"
#include "titanic/core/view_item.h"
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CViewItem, CNamedItem)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MouseDoubleClickMsg)
	ON_MESSAGE(MouseMoveMsg)
END_MESSAGE_MAP()

CViewItem::CViewItem() : CNamedItem() {
	Common::fill(&_buttonUpTargets[0], &_buttonUpTargets[4], (CTreeItem *)nullptr);
	_field24 = 0;
	_angle = 0.0;
	_viewNumber = 0;
	setAngle(0.0);
}

void CViewItem::setAngle(double angle) {
	_angle = angle;
	_viewPos.x = (int16)(cos(_angle) * 30.0);
	_viewPos.y = (int16)(sin(_angle) * -30.0);
}

void CViewItem::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	_resourceKey.save(file, indent);
	file->writeQuotedLine("V", indent);
	file->writeFloatLine(_angle, indent + 1);
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
		setAngle(file->readFloat());
		_viewNumber = file->readNumber();
		break;
	}

	CNamedItem::load(file);
}

bool CViewItem::getResourceKey(CResourceKey *key) {
	*key = _resourceKey;
	CString filename = key->getFilename();
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
					gm->roomChange();

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
		viewMsg.execute(newView, nullptr, MSGFLAG_SCAN);

		CNodeItem *oldNode = findNode();
		CNodeItem *newNode = newView->findNode();
		if (newNode != oldNode) {
			CPreEnterNodeMsg nodeMsg(oldNode, newNode);
			nodeMsg.execute(newNode, nullptr, MSGFLAG_SCAN);

			CRoomItem *oldRoom = oldNode->findRoom();
			CRoomItem *newRoom = newNode->findRoom();
			if (newRoom != oldRoom) {
				CPreEnterRoomMsg roomMsg(oldRoom, newRoom);
				roomMsg.execute(newRoom, nullptr, MSGFLAG_SCAN);
			}
		}
	}
}

void CViewItem::enterView(CViewItem *newView) {
	// Only do the processing if we've been passed a view, and it's not the same
	if (newView && newView != this) {
		CEnterViewMsg viewMsg(this, newView);
		viewMsg.execute(newView, nullptr, MSGFLAG_SCAN);

		CNodeItem *oldNode = findNode();
		CNodeItem *newNode = newView->findNode();
		if (newNode != oldNode) {
			CEnterNodeMsg nodeMsg(oldNode, newNode);
			nodeMsg.execute(newNode, nullptr, MSGFLAG_SCAN);

			CRoomItem *oldRoom = oldNode->findRoom();
			CRoomItem *newRoom = newNode->findRoom();

			CPetControl *petControl = nullptr;
			if (newRoom != nullptr) {
				petControl = newRoom->getRoot()->getPetControl();
				if (petControl)
					petControl->enterNode(newNode);
			}

			if (newRoom != oldRoom) {
				CEnterRoomMsg roomMsg(oldRoom, newRoom);
				roomMsg.execute(newRoom, nullptr, MSGFLAG_SCAN);

				if (petControl)
					petControl->enterRoom(newRoom);
			}
		}
	}
}

CLinkItem *CViewItem::findLink(CViewItem *newView) {
	for (CTreeItem *treeItem = getFirstChild(); treeItem;
			treeItem = treeItem->scan(this)) {
		CLinkItem *link = dynamic_cast<CLinkItem *>(treeItem);
		if (link && link->connectsTo(newView))
			return link;
	}

	return nullptr;
}

bool CViewItem::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (msg->_buttons & MB_LEFT) {
		if (!handleMouseMsg(msg, true)) {
			CGameManager *gm = getGameManager();
			if (gm->isntTransitioning()) {
				findNode()->findRoom();

				CLinkItem *linkItem = dynamic_cast<CLinkItem *>(
					findChildInstanceOf(CLinkItem::_type));
				while (linkItem) {
					if (linkItem->_bounds.contains(msg->_mousePos)) {
						gm->_gameState.triggerLink(linkItem);
						return true;
					}

					linkItem = dynamic_cast<CLinkItem *>(
						findNextInstanceOf(CLinkItem::_type, linkItem));
				}

				handleMouseMsg(msg, false);
			}
		}
	}

	return true;
}

bool CViewItem::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (msg->_buttons & MB_LEFT)
		handleMouseMsg(msg, false);

	return true;
}

bool CViewItem::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	if (msg->_buttons & MB_LEFT)
		handleMouseMsg(msg, false);

	return true;
}

bool CViewItem::MouseMoveMsg(CMouseMoveMsg *msg) {
	CScreenManager *screenManager = CScreenManager::_screenManagerPtr;
	uint changeCount = screenManager->_mouseCursor->getChangeCount();

	if (handleMouseMsg(msg, true)) {
		// If the cursor hasn't been set in the call to handleMouseMsg,
		// then reset it back to the default arrow cursor
		if (screenManager->_mouseCursor->getChangeCount() == changeCount)
			screenManager->_mouseCursor->setCursor(CURSOR_ARROW);
	} else {
		// Iterate through each link item, and if any is highlighted,
		// change the mouse cursor to the designated cursor for the item
		CTreeItem *treeItem = getFirstChild();
		while (treeItem) {
			CLinkItem *linkItem = dynamic_cast<CLinkItem *>(treeItem);
			if (linkItem && linkItem->_bounds.contains(msg->_mousePos)) {
				screenManager->_mouseCursor->setCursor(linkItem->_cursorId);
				return true;
			}

			treeItem = treeItem->getNextSibling();
		}

		if (!handleMouseMsg(msg, false) || (screenManager->_mouseCursor->getChangeCount() == changeCount))
			screenManager->_mouseCursor->setCursor(CURSOR_ARROW);
	}

	return true;
}

bool CViewItem::handleMouseMsg(CMouseMsg *msg, bool flag) {
	CMouseButtonUpMsg *upMsg = dynamic_cast<CMouseButtonUpMsg *>(msg);
	if (upMsg) {
		handleButtonUpMsg(upMsg);
		return true;
	}

	Common::Array<CGameObject *> gameObjects;
	for (CTreeItem *treeItem = scan(this); treeItem; treeItem = treeItem->scan(this)) {
		CGameObject *gameObject = dynamic_cast<CGameObject *>(treeItem);
		if (gameObject) {
			if (gameObject->checkPoint(msg->_mousePos, false, true) &&
					(!flag || !gameObject->_handleMouseFlag)) {
				if (gameObjects.size() < 256)
					gameObjects.push_back(gameObject);
			}
		}
	}

	const CMouseMoveMsg *moveMsg = dynamic_cast<const CMouseMoveMsg *>(msg);
	if (moveMsg) {
		if (gameObjects.size() == 0)
			return false;

		for (int idx = (int)gameObjects.size() - 1; idx >= 0; --idx) {
			if (gameObjects[idx]->_cursorId != CURSOR_IGNORE) {
				CScreenManager::_screenManagerPtr->_mouseCursor->setCursor(gameObjects[idx]->_cursorId);
				break;
			}
		}
	}
	if (gameObjects.size() == 0)
		return false;

	bool result = false;
	for (int idx = (int)gameObjects.size() - 1; idx >= 0; --idx) {
		if (msg->execute(gameObjects[idx])) {
			if (msg->isButtonDownMsg())
				_buttonUpTargets[msg->_buttons >> 1] = gameObjects[idx];
			return true;
		}

		if (CMouseMsg::isSupportedBy(gameObjects[idx]))
			result = true;
	}

	return result;
}

void CViewItem::handleButtonUpMsg(CMouseButtonUpMsg *msg) {
	CTreeItem *&target = _buttonUpTargets[msg->_buttons >> 1];

	if (target) {
		msg->execute(target);
		target = nullptr;
	}
}

void CViewItem::getPosition(double &xp, double &yp, double &zp) {
	// Get the position of the owning node within the room
	CNodeItem *node = findNode();
	node->getPosition(xp, yp, zp);

	// Adjust the position slightly to compensate for view's angle,
	// ensuring different direction views don't all have the same position
	xp += cos(_angle) * 0.5;
	yp -= sin(_angle) * 0.5;
}

CString CViewItem::getFullViewName() const {
	CNodeItem *node = findNode();
	CRoomItem *room = node->findRoom();

	return CString::format("%s.%s.%s", room->getName().c_str(),
		node->getName().c_str(), getName().c_str());
}

CString CViewItem::getNodeViewName() const {
	CNodeItem *node = findNode();

	return CString::format("%s.%s", node->getName().c_str(), getName().c_str());
}

} // End of namespace Titanic
