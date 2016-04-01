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

#include "titanic/pet_control/pet_control.h"
#include "titanic/core/project_item.h"
#include "titanic/game_manager.h"
#include "titanic/game_state.h"

namespace Titanic {

CPetControl::CPetControl() : CGameObject(), 
	_currentArea(PET_CONVERSATION), _fieldC0(0), _locked(0), _fieldC8(0),
	_treeItem1(nullptr), _treeItem2(nullptr), _hiddenRoom(nullptr),
	_drawBounds(20, 350, 620, 480) {
}

void CPetControl::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	file->writeNumberLine(_currentArea, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);

	saveAreas(file, indent);
	CGameObject::save(file, indent);
}

void CPetControl::load(SimpleFile *file) {
	int val = file->readNumber();
	isValid();
	
	if (!val) {
		_currentArea = (PetArea)file->readNumber();
		_string1 = file->readString();
		_string2 = file->readString();
		
		loadAreas(file, 0);
	}

	CGameObject::load(file);
}

bool CPetControl::isValid() {
	return _conversations.isValid(this) &&
		_rooms.isValid(this) && 
		_remote.isValid(this) &&
		_inventory.isValid(this) &&
		_sub5.isValid(this) &&
		_saves.isValid(this) &&
		_sub7.isValid(this) &&
		_frame.isValid(this);
}

void CPetControl::loadAreas(SimpleFile *file, int param) {
	_conversations.load(file, param);
	_rooms.load(file, param);
	_remote.load(file, param);
	_inventory.load(file, param);
	_sub5.load(file, param);
	_saves.load(file, param);
	_sub7.load(file, param);
	_frame.load(file, param);
}

void CPetControl::saveAreas(SimpleFile *file, int indent) const {
	_conversations.save(file, indent);
	_rooms.save(file, indent);
	_remote.save(file, indent);
	_inventory.save(file, indent);
	_sub5.save(file, indent);
	_saves.save(file, indent);
	_sub7.save(file, indent);
	_frame.save(file, indent);
}

void CPetControl::draw(CScreenManager *screenManager) {
	CGameManager *gameManager = getGameManager();
	Rect bounds = _drawBounds;
	bounds.constrain(gameManager->_bounds);

	if (!bounds.isEmpty()) {
		if (_fieldC8 >= 0) {
			_inventory.proc5(_fieldC8);
			_fieldC8 = -1;
		}

		_frame.drawFrame(screenManager);

		// Draw the specific area that's currently active
		switch (_currentArea) {
		case PET_INVENTORY:
			_inventory.draw(screenManager);
			break;
		case PET_CONVERSATION:
			_conversations.draw(screenManager);
			break;
		case PET_REMOTE:
			_remote.draw(screenManager);
			break;
		case PET_ROOMS:
			_rooms.draw(screenManager);
			break;
		case PET_SAVE:
			_saves.draw(screenManager);
			break;
		case PET_5:
			_sub5.draw(screenManager);
			break;
		case PET_6:
			_sub7.draw(screenManager);
			break;
		default:
			break;
		}
	}
}

Rect CPetControl::getBounds() {
	switch (_currentArea) {
	case PET_INVENTORY:
		return _inventory.getBounds();
		break;
	case PET_CONVERSATION:
		return _conversations.getBounds();
		break;
	case PET_REMOTE:
		return _remote.getBounds();
		break;
	case PET_ROOMS:
		return _rooms.getBounds();
		break;
	case PET_SAVE:
		return _saves.getBounds();
		break;
	case PET_5:
		return _sub5.getBounds();
		break;
	case PET_6:
		return _sub7.getBounds();
		break;
	default:
		break;
	}
}

void CPetControl::postLoad() {
	CProjectItem *root = getRoot();

	if (!_string1.empty() && root)
		_treeItem1 = root->findByName(_string1);
	if (!_string2.empty() && root)
		_treeItem2 = root->findByName(_string2);

	setArea(_currentArea);
	loaded();
}

void CPetControl::loaded() {
	_conversations.postLoad();
	_rooms.postLoad();
	_remote.postLoad();
	_inventory.postLoad();
	_sub5.postLoad();
	_saves.postLoad();
	_sub7.postLoad();
	_frame.postLoad();
}

void CPetControl::enterNode(CNodeItem *node) {
	getGameManager()->_gameState.enterNode();
}

void CPetControl::enterRoom(CRoomItem *room) {
	_rooms.enterRoom(room);
	_remote.enterRoom(room);
}

void CPetControl::clear() {
	_treeItem2 = nullptr;
	_string2.clear();
}

bool CPetControl::fn1(int val) {
	warning("TODO: CPetControl::fn1");
	return false;
}

void CPetControl::fn4() {
	warning("TODO: CPetControl::fn4");
}

PetArea CPetControl::setArea(PetArea newArea) {
	if (newArea == _currentArea || !canChangeArea())
		return _currentArea;

	// Signal the currently active area that it's being left
	switch (_currentArea) {
	case PET_INVENTORY:
		_inventory.leave();
		break;
	case PET_CONVERSATION:
		_conversations.leave();
		break;
	case PET_REMOTE:
		_remote.leave();
		break;
	case PET_ROOMS:
		_rooms.leave();
		break;
	case PET_SAVE:
		_saves.leave();
		break;
	case PET_5:
		_sub5.leave();
		break;
	case PET_6:
		_sub7.leave();
		break;
	default:
		break;
	}

	// Change the current area
	PetArea oldArea = _currentArea;
	_frame.setArea(newArea);
	_currentArea = newArea;

	// Signal to the new view that it's been activated
	switch (newArea) {
	case PET_INVENTORY:
		_inventory.enter(oldArea);
		
		break;
	case PET_CONVERSATION:
		_conversations.enter(oldArea);
		break;
	case PET_REMOTE:
		_remote.enter(oldArea);
		break;
	case PET_ROOMS:
		_rooms.enter(oldArea);
		break;
	case PET_SAVE:
		_saves.enter(oldArea);
		break;
	case PET_5:
		_sub5.enter(oldArea);
		break;
	case PET_6:
		_sub7.enter(oldArea);
		break;
	default:
		break;
	}

	makeDirty();
	return newArea;
}

void CPetControl::fn2(int val) {
	switch (_currentArea) {
	case PET_INVENTORY:
		_inventory.proc38(val);
		break;
	case PET_CONVERSATION:
		_conversations.proc38(val);
		break;
	case PET_REMOTE:
		_remote.proc38(val);
		break;
	case PET_ROOMS:
		_rooms.proc38(val);
		break;
	case PET_SAVE:
		_saves.proc38(val);
		break;
	case PET_5:
		_sub5.proc38(val);
		break;
	case PET_6:
		_sub7.proc38(val);
		break;
	default:
		break;
	}
}

void CPetControl::fn3(CTreeItem *item) {
	_treeItem2 = item;
	if (item)
		_string2 = item->getName();
	else
		_string2.clear();
}

CRoomItem *CPetControl::getHiddenRoom() {
	if (!_hiddenRoom)
		_hiddenRoom = CTreeItem::getHiddenRoom();

	return _hiddenRoom;
}

CGameObject *CPetControl::findItemInRoom(CRoomItem *room, 
		const CString &name) const {
	if (!room)
		return nullptr;

	for (CTreeItem *treeItem = room->getFirstChild(); treeItem;
			treeItem = treeItem->scan(room)) {
		if (!treeItem->getName().compareTo(name)) {
			return dynamic_cast<CGameObject *>(treeItem);
		}
	}

	return nullptr;
}

CGameObject *CPetControl::getHiddenObject(const CString &name) {
	CRoomItem *room = getHiddenRoom();
	return room ? findItemInRoom(room, name) : nullptr;
}

bool CPetControl::containsPt(const Common::Point &pt) const {
	return _drawBounds.contains(pt);
}

} // End of namespace Titanic
