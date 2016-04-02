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
	_timers[0] = _timers[1] = nullptr;
	_sections[PET_INVENTORY] = &_inventory;
	_sections[PET_CONVERSATION] = &_conversations;
	_sections[PET_REMOTE] = &_remote;
	_sections[PET_ROOMS] = &_rooms;
	_sections[PET_SAVE] = &_saves;
	_sections[PET_5] = &_sub5;
	_sections[PET_6] = &_sub7;
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
		_sections[_currentArea]->draw(screenManager);
	}
}

Rect CPetControl::getBounds() {
	return _sections[_currentArea]->getBounds();
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
	if (newArea == _currentArea || !isUnlocked())
		return _currentArea;

	// Signal the currently active area that it's being left
	_sections[_currentArea]->leave();

	// Change the current area
	PetArea oldArea = _currentArea;
	_frame.setArea(newArea);
	_currentArea = newArea;

	// Signal to the new view that it's been activated
	_sections[_currentArea]->enter(oldArea);

	makeDirty();
	return newArea;
}

void CPetControl::fn2(int val) {
	_sections[_currentArea]->proc38(val);
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

bool CPetControl::getC0() const {
	return _fieldC0 > 0;
}

bool CPetControl::handleMessage(CMouseButtonDownMsg &msg) {
	if (!containsPt(msg._mousePos) || getC0())
		return false;

	bool result = false;
	if (isUnlocked())
		result = _frame.handleMessage(msg);

	if (!result) {
		result = _sections[_currentArea]->handleMessage(msg);
	}

	makeDirty();
	return result;
}

bool CPetControl::handleMessage(CMouseDragStartMsg &msg) {
	if (!containsPt(msg._mousePos) || getC0())
		return false;

	return _sections[_currentArea]->handleMessage(msg);
}

bool CPetControl::handleMessage(CMouseDragMoveMsg &msg) {
	return _sections[_currentArea]->handleMessage(msg);
}

bool CPetControl::handleMessage(CMouseDragEndMsg &msg) {
	return _sections[_currentArea]->handleMessage(msg);
}

bool CPetControl::handleMessage(CMouseButtonUpMsg &msg) {
	if (!containsPt(msg._mousePos) || getC0())
		return false;

	bool result = false;
	if (isUnlocked())
		result = _frame.handleMessage(msg);

	if (!result)
		result = _sections[_currentArea]->handleMessage(msg);

	makeDirty();
	return result;
}

bool CPetControl::handleMessage(CMouseDoubleClickMsg &msg) {
	if (!containsPt(msg._mousePos) || getC0())
		return false;

	return _sections[_currentArea]->handleMessage(msg);
}

bool CPetControl::handleMessage(CKeyCharMsg &msg) {
	if (getC0())
		return false;

	return _sections[_currentArea]->handleMessage(msg);
}

bool CPetControl::handleMessage(CVirtualKeyCharMsg &msg) {
	if (getC0())
		return false;

	bool result = _sections[_currentArea]->handleMessage(msg);

	if (!result) {
		switch (msg._keyState.keycode) {
		case Common::KEYCODE_F1:
			result = true;
			setArea(PET_INVENTORY);
			break;
		case Common::KEYCODE_F2:
			result = true;
			setArea(PET_CONVERSATION);
			break;
		case Common::KEYCODE_F3:
			result = true;
			setArea(PET_REMOTE);
			break;
		case Common::KEYCODE_F4:
			result = true;
			setArea(PET_ROOMS);
			break;
		case Common::KEYCODE_F5:
			result = true;
			setArea(PET_SAVE);
			break;
		default:
			break;
		}
	}

	return result;
}

bool CPetControl::handleMessage(CTimerMsg &msg) {
	warning("TODO: CPetControl::CTimerMsg");
	return true;
}

void CPetControl::drawIndent(CScreenManager *screenManager, int indent) {
	_frame.drawIndent(screenManager, indent);
}

} // End of namespace Titanic
