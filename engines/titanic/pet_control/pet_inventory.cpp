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

#include "titanic/pet_control/pet_inventory.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/carry/carry.h"
#include "titanic/titanic.h"

namespace Titanic {

CPetInventory::CPetInventory() : CPetSection(),
		_movie(nullptr), _isLoading(false), _titaniaBitFlags(0) {
	for (int idx = 0; idx < TOTAL_ITEMS; ++idx) {
		_itemBackgrounds[idx] = nullptr;
	}
}

bool CPetInventory::setup(CPetControl *petControl) {
	return setPetControl(petControl) && reset();
}

bool CPetInventory::reset() {
	_items.reset();
	_text.setup();
	_text.setColor(getColor(0));
	_text.setLineColor(0, getColor(0));

	return true;
}

void CPetInventory::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 7);
	_items.draw(screenManager);
	_text.draw(screenManager);
}

Rect CPetInventory::getBounds() const {
	return _movie ? _movie->getBounds() : Rect();
}

void CPetInventory::changed(int changeType) {
	switch (changeType) {
	case 0:
	case 2:
		itemsChanged();
		break;
	case 1:
		removeInvalid();
		break;
	default:
		break;
	}
}

void CPetInventory::enterRoom(CRoomItem *room) {
	int index = _items.getHighlightIndex();
	if (index != -1) {
		_items.resetHighlight();
		_items.highlight(index);
	}
}

bool CPetInventory::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return _items.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetInventory::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	bool result = _items.MouseDragStartMsg(msg);
	if (result)
		_petControl->makeDirty();
	return result;
}

bool CPetInventory::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return _items.MouseButtonUpMsg(msg->_mousePos);
}

bool CPetInventory::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	return _items.MouseDoubleClickMsg(msg->_mousePos);
}

bool CPetInventory::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	return _items.VirtualKeyCharMsg(msg);
}

bool CPetInventory::MouseWheelMsg(CMouseWheelMsg *msg) {
	if (msg->_wheelUp)
		_items.scrollLeft();
	else
		_items.scrollRight();

	return true;
}

CGameObject *CPetInventory::dragEnd(const Point &pt) const {
	return _items.getObjectAt(pt);
}

bool CPetInventory::isValid(CPetControl *petControl) {
	setPetControl(petControl);
	return true;
}

void CPetInventory::load(SimpleFile *file, int param) {
	_titaniaBitFlags = file->readNumber();
}

void CPetInventory::postLoad() {
	reset();
	_isLoading = true;
	itemsChanged();
	_isLoading = false;
}

void CPetInventory::save(SimpleFile *file, int indent) {
	file->writeNumberLine(_titaniaBitFlags, indent);
}

void CPetInventory::enter(PetArea oldArea) {
	_items.enter();
}

void CPetInventory::leave() {
	_items.leave();
}

CGameObject *CPetInventory::getBackground(int index) const {
	return (index >= 0 && index < 46) ? _itemBackgrounds[index] : nullptr;
}

bool CPetInventory::setPetControl(CPetControl *petControl) {
	if (!petControl)
		return false;

	_petControl = petControl;
	_items.setup(7,  this);
	_items.setFlags(28);

	Rect tempRect(0, 0, 52, 52);
	for (uint idx = 0; idx < TOTAL_ITEMS; ++idx) {
		if (!g_vm->_itemNames[idx].empty()) {
			CString name = "3Pet" + g_vm->_itemNames[idx];
			_itemBackgrounds[idx] = petControl->getHiddenObject(name);
		}
	}

	tempRect = Rect(0, 0, 580, 15);
	tempRect.translate(32, 445);
	_text.setBounds(tempRect);
	_text.setHasBorder(false);

	return true;
}

void CPetInventory::change(CGameObject *item) {
	if (item) {
		CInventoryGlyphAction action(item, ACTION_CHANGE);
		_items.doAction(&action);
	}
}

void CPetInventory::itemRemoved(CGameObject *item) {
	if (item) {
		CInventoryGlyphAction action(item, ACTION_REMOVED);
		_items.doAction(&action);
	}
}

void CPetInventory::itemsChanged() {
	_items.clear();

	CGameObject *item = _petControl->getFirstObject();
	while (item) {
		CPetInventoryGlyph *glyph = new CPetInventoryGlyph();
		glyph->setup(_petControl, &_items);
		glyph->setItem(item, _isLoading);

		_items.push_back(glyph);
		item = _petControl->getNextObject(item);
	}
}

void CPetInventory::highlightItem(CGameObject *item) {
	int itemIndex = getItemIndex(item);
	_items.highlight(itemIndex);
}

int CPetInventory::getItemIndex(CGameObject *item) const {
	int index = 0;
	for (CGameObject *obj = _petControl->getFirstObject(); obj && obj != item;
			++index, obj = _petControl->getNextObject(obj)) {
	}

	return index;
}

CGameObject *CPetInventory::getTransformAnimation(int index) {
	if (index >= 0 && index < 46) {
		// Certain items are pieces of Titania, and they only have the
		// one-time initial transformation into Titania pieces
		CString name;
		int bits = 0;

		switch (index) {
		case 20:
			name = "PetEarMorph";
			bits = 4;
			break;
		case 21:
			name = "PetEarMorph1";
			bits = 8;
			break;
		case 22:
			name = "PetEyeMorph";
			bits = 1;
			break;
		case 23:
			name = "PetEyeMorph";
			bits = 2;
			break;
		case 36:
			name = "PetMouthMorph";
			bits = 32;
			break;
		case 39:
			name = "PetNoseMorph";
			bits = 16;
			break;
		default:
			break;
		}

		if (!(bits & _titaniaBitFlags) && !name.empty()) {
			CGameObject *obj = getPetControl()->getHiddenObject(name);
			assert(obj);

			_titaniaBitFlags = bits | _titaniaBitFlags;
			return obj;
		}
	}

	return nullptr;
}

void CPetInventory::playMovie(CGameObject *movie, bool repeat) {
	if (_movie)
		_movie->stopMovie();
	_movie = movie;

	if (_movie) {
		if (repeat)
			_movie->playMovie(0, 14, MOVIE_REPEAT);
		else
			_movie->playMovie(0);
	}
}

void CPetInventory::removeInvalid() {
	_items.removeInvalid();
}

} // End of namespace Titanic
