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

#include "engines/myst3/inventory.h"

#include "engines/myst3/cursor.h"
#include "engines/myst3/database.h"
#include "engines/myst3/scene.h"
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
		Window(),
		_vm(vm),
		_texture(0) {
	_scaled = !_vm->isWideScreenModEnabled();
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

bool Inventory::isMouseInside() {
	Common::Point mouse = _vm->_cursor->getPosition(false);
	return getPosition().contains(mouse);
}

void Inventory::draw() {
	if (_vm->isWideScreenModEnabled()) {
		// Draw a black background to cover the main game frame
		Common::Rect screen = _vm->_gfx->viewport();
		_vm->_gfx->drawRect2D(Common::Rect(screen.width(), Renderer::kBottomBorderHeight), 0xFF000000);
	}

	uint16 hoveredItemVar = hoveredItem();

	for (ItemList::const_iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		int32 state = _vm->_state->getVar(it->var);

		// Don't draw if the item is being dragged or is hidden
		if (state == -1 || state == 0)
			continue;

		const ItemData &item = getData(it->var);

		Common::Rect textureRect = Common::Rect(item.textureWidth,
				item.textureHeight);
		textureRect.translate(item.textureX, 0);

		bool itemHighlighted = it->var == hoveredItemVar || state == 2;

		if (itemHighlighted)
			textureRect.translate(0, _texture->height / 2);

		_vm->_gfx->drawTexturedRect2D(it->rect, textureRect, _texture);
	}
}

void Inventory::reset() {
	_inventory.clear();
	reflow();
	updateState();
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
		updateState();
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
	updateState();
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

	uint16 left;
	if (_vm->isWideScreenModEnabled()) {
		Common::Rect screen = _vm->_gfx->viewport();
		left = (screen.width() - totalWidth) / 2;
	} else {
		left = (Renderer::kOriginalWidth - totalWidth) / 2;
	}

	for (ItemList::iterator it = _inventory.begin(); it != _inventory.end(); it++) {
		const ItemData &item = getData(it->var);

		uint16 top = (Renderer::kBottomBorderHeight - item.textureHeight) / 2;

		it->rect = Common::Rect(item.textureWidth, item.textureHeight);
		it->rect.translate(left, top);

		left += item.textureWidth;

		if (itemCount >= 2)
			left += 9;
	}
}

uint16 Inventory::hoveredItem() {
	Common::Point mouse = _vm->_cursor->getPosition(false);
	mouse = scalePoint(mouse);

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
		openBook(9, kRoomJournals, 100);
		break;
	case 279: // Saavedro
		closeAllBooks();
		_vm->_state->setJournalSaavedroState(2);
		openBook(9, kRoomJournals, 200);
		break;
	case 480: // Tomahna
		closeAllBooks();
		_vm->_state->setBookStateTomahna(2);
		openBook(8, kRoomNarayan, 220);
		break;
	case 481: // Releeshahn
		closeAllBooks();
		_vm->_state->setBookStateReleeshahn(2);
		openBook(9, kRoomJournals, 300);
		break;
	case 345:
		_vm->dragSymbol(345, 1002);
		break;
	case 398:
		_vm->dragSymbol(398, 1001);
		break;
	case 447:
		_vm->dragSymbol(447, 1000);
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
	_vm->goToNode(node, kTransitionFade);
}

void Inventory::addSaavedroChapter(uint16 var) {
	_vm->_state->setVar(var, 1);
	_vm->_state->setJournalSaavedroState(2);
	_vm->_state->setJournalSaavedroChapter(var - 285);
	_vm->_state->setJournalSaavedroPageInChapter(0);
	openBook(9, kRoomJournals, 200);
}

void Inventory::loadFromState() {
	Common::Array<uint16> items = _vm->_state->getInventory();

	_inventory.clear();
	for (uint i = 0; i < items.size(); i++)
		addItem(items[i], true);
}

void Inventory::updateState() {
	Common::Array<uint16> items;
	for (ItemList::iterator it = _inventory.begin(); it != _inventory.end(); it++)
		items.push_back(it->var);

	_vm->_state->updateInventory(items);
}

Common::Rect Inventory::getPosition() const {
	Common::Rect screen = _vm->_gfx->viewport();

	Common::Rect frame;
	if (_vm->isWideScreenModEnabled()) {
		frame = Common::Rect(screen.width(), Renderer::kBottomBorderHeight);

		Common::Rect scenePosition = _vm->_scene->getPosition();
		int16 top = CLIP<int16>(screen.height() - frame.height(), 0, scenePosition.bottom);

		frame.translate(0, top);
	} else {
		frame = Common::Rect(screen.width(), screen.height() * Renderer::kBottomBorderHeight / Renderer::kOriginalHeight);
		frame.translate(screen.left, screen.top + screen.height() * (Renderer::kTopBorderHeight + Renderer::kFrameHeight) / Renderer::kOriginalHeight);
	}

	return frame;
}

Common::Rect Inventory::getOriginalPosition() const {
	Common::Rect originalPosition = Common::Rect(Renderer::kOriginalWidth, Renderer::kBottomBorderHeight);
	originalPosition.translate(0, Renderer::kTopBorderHeight + Renderer::kFrameHeight);
	return originalPosition;
}

void Inventory::updateCursor() {
	uint16 item = hoveredItem();
	if (item > 0) {
		_vm->_cursor->changeCursor(1);
	} else {
		_vm->_cursor->changeCursor(8);
	}
}

DragItem::DragItem(Myst3Engine *vm, uint id):
		_vm(vm),
		_texture(0),
		_frame(1) {
	// Draw on the whole screen
	_isConstrainedToWindow = false;
	_scaled = !_vm->isWideScreenModEnabled();

	ResourceDescription movieDesc = _vm->getFileDescription("DRAG", id, 0, Archive::kStillMovie);

	if (!movieDesc.isValid())
		error("Movie %d does not exist", id);

	// Load the movie
	_movieStream = movieDesc.getData();
	_bink.setDefaultHighColorFormat(Texture::getRGBAPixelFormat());
	_bink.loadStream(_movieStream);
	_bink.start();

	const Graphics::Surface *frame = _bink.decodeNextFrame();
	_texture = _vm->_gfx->createTexture(frame);
}

DragItem::~DragItem() {
	_vm->_gfx->freeTexture(_texture);
}

void DragItem::drawOverlay() {
	Common::Rect textureRect = Common::Rect(_texture->width, _texture->height);
	_vm->_gfx->drawTexturedRect2D(getPosition(), textureRect, _texture, 0.99f);
}

void DragItem::setFrame(uint16 frame) {
	if (frame != _frame) {
		_frame = frame;
		_bink.seekToFrame(frame - 1);
		const Graphics::Surface *s = _bink.decodeNextFrame();
		_texture->update(s);
	}
}

Common::Rect DragItem::getPosition() {
	Common::Rect viewport;
	Common::Point mouse;

	if (_scaled) {
		viewport = Common::Rect(Renderer::kOriginalWidth, Renderer::kOriginalHeight);
		mouse = _vm->_cursor->getPosition(true);
	} else {
		viewport = _vm->_gfx->viewport();
		mouse = _vm->_cursor->getPosition(false);
	}

	uint posX = CLIP<uint>(mouse.x, _texture->width / 2, viewport.width() - _texture->width / 2);
	uint posY = CLIP<uint>(mouse.y, _texture->height / 2, viewport.height() - _texture->height / 2);

	Common::Rect screenRect = Common::Rect::center(posX, posY, _texture->width, _texture->height);
	return screenRect;
}

} // End of namespace Myst3
