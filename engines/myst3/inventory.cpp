/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#include "engines/myst3/inventory.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/state.h"

namespace Myst3 {

const Inventory::ItemData Inventory::_availableItems[8] = {
		{   0, 41, 47, 481 },
		{  41, 38, 50, 480 },
		{  79, 38, 49, 279 },
		{ 117, 34, 48, 277 },
		{ 151, 35, 44, 345 },
		{ 186, 35, 44, 398 },
		{ 221, 35, 44, 447 },
		{   0,  0,  0,   0 }
};

Inventory::Inventory(Myst3Engine *vm) :
	_vm(vm),
	_texture(0) {

	initializeTexture();
}

Inventory::~Inventory() {
	_vm->_gfx->freeTexture(_texture);
}

void Inventory::initializeTexture() {
	Graphics::Surface *s = _vm->loadTexture(1204);

	_texture = _vm->_gfx->createTexture(s);

	s->free();
	delete s;
}

void Inventory::draw() {
	Common::Point mouse = _vm->_cursor->getPosition();

	for (ItemList::const_iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		const ItemData &item = getData(it->var);

		Common::Rect textureRect = Common::Rect(item.textureWidth,
				item.textureHeight);
		textureRect.translate(item.textureX, 0);

		bool itemHighlighted = it->rect.contains(mouse)
				|| (_vm->_state->getVar(it->var) == 2);

		if (itemHighlighted)
			textureRect.translate(0, _texture->height / 2);

		_vm->_gfx->drawTexturedRect2D(it->rect, textureRect, _texture);
	}
}

void Inventory::reset() {
	_inventory.clear();
	reflow();
}

void Inventory::addItem(uint16 var, bool atEnd) {
	// Only add objects once to the inventory
	if (!hasItem(var)) {
		_vm->_state->setVar(var, 1);

		InventoryItem i;
		i.var = var;

		if (atEnd) {
			_inventory.push_back(i);
		} else {
			_inventory.push_front(i);
		}

		reflow();
	}
}

void Inventory::removeItem(uint16 var) {
	_vm->_state->setVar(var, 0);

	for (ItemList::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		if (it->var == var) {
			_inventory.erase(it);
			break;
		}
	}

	reflow();
}

void Inventory::addAll() {
	for (uint i = 0; _availableItems[i].var; i++)
		addItem(_availableItems[i].var, true);
}

bool Inventory::hasItem(uint16 var) {
	for (ItemList::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		if (it->var == var)
			return true;
	}

	return false;
}

const Inventory::ItemData &Inventory::getData(uint16 var) {
	for (uint i = 0; _availableItems[i].var; i++) {
		if (_availableItems[i].var == var)
			return _availableItems[i];
	}

	return _availableItems[7];
}

void Inventory::reflow() {
	uint16 itemCount = 0;
	uint16 totalWidth = 0;

	for (uint i = 0; _availableItems[i].var; i++) {
		if (hasItem(_availableItems[i].var)) {
			totalWidth += _availableItems[i].textureWidth;
			itemCount++;
		}
	}

	if (itemCount >= 2)
		totalWidth += 9 * (itemCount - 1);

	uint16 left = (Renderer::kOriginalWidth - totalWidth) / 2;

	for (ItemList::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		const ItemData &item = getData(it->var);

		uint16 top = Scene::kTopBorderHeight + Scene::kFrameHeight
				+ (Scene::kBottomBorderHeight - item.textureHeight) / 2;

		it->rect = Common::Rect(item.textureWidth,
				item.textureHeight);
		it->rect.translate(left, top);

		left += item.textureWidth;

		if (itemCount >= 2)
			left += 9;
	}
}

uint16 Inventory::hoveredItem() {
	Common::Point mouse = _vm->_cursor->getPosition();

	for (ItemList::const_iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		if(it->rect.contains(mouse))
			return it->var;
	}

	return 0;
}

void Inventory::useItem(uint16 var) {
	switch (var) {
	case 277: // Atrus
		closeAllBooks();
		_vm->_state->setJournalAtrusState(2);
		openBook(9, 902, 100);
		break;
	case 279: // Saavedro
		closeAllBooks();
		_vm->_state->setJournalSaavedroState(2);
		openBook(9, 902, 200);
		break;
	case 480: // Tomahna
		closeAllBooks();
		_vm->_state->setBookStateTomahna(2);
		openBook(8, 801, 220);
		break;
	case 481: // Releeshahn
		closeAllBooks();
		_vm->_state->setBookStateReleeshahn(2);
		openBook(9, 902, 300);
		break;
	default:
		debug("Used inventory item %d which is not implemented", var);
	}
}

void Inventory::closeAllBooks() {
	if (_vm->_state->getJournalAtrusState())
		_vm->_state->setJournalAtrusState(1);
	if (_vm->_state->getJournalSaavedroState())
		_vm->_state->setJournalSaavedroState(1);
	if (_vm->_state->getBookStateTomahna())
		_vm->_state->setBookStateTomahna(1);
	if (_vm->_state->getBookStateReleeshahn())
		_vm->_state->setBookStateReleeshahn(1);
}

void Inventory::openBook(uint16 age, uint16 room, uint16 node) {
	if (!_vm->_state->getBookSavedNode()) {
		_vm->_state->setBookSavedAge(_vm->_state->getLocationAge());
		_vm->_state->setBookSavedRoom(_vm->_state->getLocationRoom());
		_vm->_state->setBookSavedNode(_vm->_state->getLocationNode());
	}

	_vm->_state->setLocationNextAge(age);
	_vm->_state->setLocationNextRoom(room);
	_vm->goToNode(node, 1);
}

void Inventory::addSaavedroChapter(uint16 var) {
	_vm->_state->setVar(var, 1);
	_vm->_state->setJournalSaavedroState(2);
	_vm->_state->setJournalSaavedroChapter(var - 285);
	_vm->_state->setJournalSaavedroPageInChapter(0);
	openBook(9, 902, 200);
}

} /* namespace Myst3 */
