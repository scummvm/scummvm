/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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
#include "engines/myst3/variables.h"

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
	_vm(vm) {

	initializeTexture();
}

Inventory::~Inventory() {
	glDeleteTextures(1, &_textureId);
}

void Inventory::initializeTexture() {
	Graphics::Surface *s = _vm->loadTexture(1204);

	glGenTextures(1, &_textureId);
	glBindTexture(GL_TEXTURE_2D, _textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s->w, s->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
				|| (_vm->_vars->get(it->var) == 2);

		drawItem(it->rect, textureRect, itemHighlighted);
	}
}

void Inventory::drawItem(const Common::Rect &screenRect, const Common::Rect &textureRect, bool hovered) {
	// Used fragment of texture
	const float tleft = textureRect.left / (float)(256);
	const float twidth = textureRect.width() / (float)(256);
	float ttop = textureRect.top / (float)(128);
	const float theight = textureRect.height() / (float)(128);

	const float left = screenRect.left;
	const float top = screenRect.top;
	const float w = screenRect.width();
	const float h = screenRect.height();

	if (hovered)
		ttop += 64.0 / 128.0;

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, _textureId);
	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(tleft, ttop + theight);
		glVertex3f( left + 0, top + h, 1.0f);

		glTexCoord2f(tleft + twidth, ttop + theight);
		glVertex3f( left + w, top + h, 1.0f);

		glTexCoord2f(tleft, ttop);
		glVertex3f( left + 0, top + 0, 1.0f);

		glTexCoord2f(tleft + twidth, ttop);
		glVertex3f( left + w, top + 0, 1.0f);
	glEnd();

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

void Inventory::reset() {
	_inventory.clear();
	reflow();
}

void Inventory::addItem(uint16 var, bool atEnd) {
	// Only add objects once to the inventory
	if (!hasItem(var)) {
		_vm->_vars->set(var, 1);

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
	_vm->_vars->set(var, 0);

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

	uint16 left = (Scene::_originalWidth - totalWidth) / 2;

	for (ItemList::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		const ItemData &item = getData(it->var);

		uint16 top = Scene::_topBorderHeight + Scene::_frameHeight
				+ (Scene::_bottomBorderHeight - item.textureHeight) / 2;

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
		_vm->_vars->setJournalAtrusState(2);
		openBook(9, 902, 100);
		break;
	case 279: // Saavedro
		closeAllBooks();
		_vm->_vars->setJournalSaavedroState(2);
		openBook(9, 902, 200);
		break;
	case 480: // Tomahna
		closeAllBooks();
		_vm->_vars->setBookStateTomahna(2);
		openBook(8, 801, 220);
		break;
	case 481: // Releeshahn
		closeAllBooks();
		_vm->_vars->setBookStateReleeshahn(2);
		openBook(9, 902, 300);
		break;
	default:
		debug("Used inventory item %d which is not implemented", var);
	}
}

void Inventory::closeAllBooks() {
	if (_vm->_vars->getJournalAtrusState())
		_vm->_vars->setJournalAtrusState(1);
	if (_vm->_vars->getJournalSaavedroState())
		_vm->_vars->setJournalSaavedroState(1);
	if (_vm->_vars->getBookStateTomahna())
		_vm->_vars->setBookStateTomahna(1);
	if (_vm->_vars->getBookStateReleeshahn())
		_vm->_vars->setBookStateReleeshahn(1);
}

void Inventory::openBook(uint16 age, uint16 room, uint16 node) {
	if (!_vm->_vars->getBookSavedNode()) {
		_vm->_vars->setBookSavedAge(_vm->_vars->getLocationAge());
		_vm->_vars->setBookSavedRoom(_vm->_vars->getLocationRoom());
		_vm->_vars->setBookSavedNode(_vm->_vars->getLocationNode());
	}

	_vm->_vars->setLocationNextAge(age);
	_vm->_vars->setLocationNextRoom(room);
	_vm->goToNode(node, 1);
}

void Inventory::addSaavedroChapter(uint16 var) {
	_vm->_vars->set(var, 1);
	_vm->_vars->setJournalSaavedroState(2);
	_vm->_vars->setJournalSaavedroChapter(var - 285);
	_vm->_vars->setJournalSaavedroPageInChapter(0);
	openBook(9, 902, 200);
}

} /* namespace Myst3 */
