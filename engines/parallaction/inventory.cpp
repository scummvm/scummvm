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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "parallaction/parallaction.h"



namespace Parallaction {

//
//	inventory is a grid made of (at most) 30 cells, 24x24 pixels each,
//	arranged in 6 lines
//
//	inventory items are stored in cnv files in a 32x24 grid
//	but only 24x24 pixels are actually copied to graphic memory
//

#define INVENTORY_MAX_ITEMS 		30
#define INVENTORY_FIRST_ITEM		4		// first four entries are used up by verbs

#define INVENTORY_ITEMS_PER_LINE	5
#define INVENTORY_LINES 			6

#define INVENTORY_WIDTH 			(INVENTORY_ITEMS_PER_LINE*INVENTORYITEM_WIDTH)
#define INVENTORY_HEIGHT			(INVENTORY_LINES*INVENTORYITEM_HEIGHT)

Inventory *_inv = 0;
InventoryRenderer *_re = 0;


int16 Parallaction::getHoverInventoryItem(int16 x, int16 y) {
	return _re->hitTest(Common::Point(x,y));
}

void highlightInventoryItem(ItemPosition pos, byte color) {
	_re->highlightItem(pos, color);
}

int Parallaction::addInventoryItem(ItemName item) {
	return _inv->addItem(item);
}

int Parallaction::addInventoryItem(ItemName item, uint32 value) {
	return _inv->addItem(item, value);
}

void Parallaction::dropItem(uint16 v) {
	_inv->removeItem(v);
}

bool Parallaction::isItemInInventory(int32 v) {
	return (_inv->findItem(v) != -1);
}

const InventoryItem* getInventoryItem(int16 pos) {
	return _inv->getItem(pos);
}

int16 getInventoryItemIndex(int16 pos) {
	return _inv->getItemName(pos);
}

void initInventory() {
	_inv = new Inventory(INVENTORY_MAX_ITEMS);
	_re = new InventoryRenderer(_vm);
	_re->bindInventory(_inv);
}

void destroyInventory() {
	delete _inv;
	delete _re;
}

void cleanInventory(bool keepVerbs) {
	_inv->clear(keepVerbs);
}

void openInventory() {
	_re->showInventory();
}

void closeInventory() {
	_re->hideInventory();
}




void Parallaction_ns::jobShowInventory(void *parm, Job *j) {
	Common::Rect r;
	_re->getRect(r);
	_gfx->copyRect(Gfx::kBitBack, r, _re->getData(), INVENTORY_WIDTH);
}

void Parallaction_ns::jobHideInventory(void *parm, Job *j) {
	static uint16 count = 0;
	_engineFlags |= kEngineBlockInput;

	count++;
	if (count == 2) {
		count = 0;
		j->_finished = 1;
		_engineFlags &= ~kEngineBlockInput;
	}

	Common::Rect r;
	_re->getRect(r);
	_gfx->restoreBackground(r);
}





InventoryRenderer::InventoryRenderer(Parallaction *vm) : _vm(vm) {
	_surf.create(INVENTORY_WIDTH, INVENTORY_HEIGHT, 1);
}

InventoryRenderer::~InventoryRenderer() {
	_surf.free();
}

void InventoryRenderer::showInventory() {
	if (!_inv)
		error("InventoryRenderer not bound to inventory");

	_engineFlags |= kEngineInventory;

	uint16 lines = getNumLines();

	_pos.x = CLIP(_vm->_mousePos.x - (INVENTORY_WIDTH / 2), 0, (int)(_vm->_screenWidth - INVENTORY_WIDTH));
	_pos.y = CLIP(_vm->_mousePos.y - 2 - (lines * INVENTORYITEM_HEIGHT), 0, (int)(_vm->_screenHeight - lines * INVENTORYITEM_HEIGHT));

	refresh();
}

void InventoryRenderer::hideInventory() {
	if (!_inv)
		error("InventoryRenderer not bound to inventory");

	_engineFlags &= ~kEngineInventory;
}

void InventoryRenderer::getRect(Common::Rect& r) const {
	r.setWidth(INVENTORY_WIDTH);
	r.setHeight(INVENTORYITEM_HEIGHT * getNumLines());
	r.moveTo(_pos);
}

ItemPosition InventoryRenderer::hitTest(const Common::Point &p) const {
	Common::Rect r;
	getRect(r);
	if (!r.contains(p))
		return -1;

	return ((p.x - _pos.x) / INVENTORYITEM_WIDTH) + (INVENTORY_ITEMS_PER_LINE * ((p.y - _pos.y) / INVENTORYITEM_HEIGHT));
}


void InventoryRenderer::drawItem(ItemPosition pos, ItemName name) {

	Common::Rect r;
	getItemRect(pos, r);

	// FIXME: this will end up in a general blit function

	byte* s = _vm->_char._objs->getData(name);
	byte* d = (byte*)_surf.getBasePtr(r.left, r.top);
	for (uint32 i = 0; i < INVENTORYITEM_HEIGHT; i++) {
		memcpy(d, s, INVENTORYITEM_WIDTH);

		d += INVENTORY_WIDTH;
		s += INVENTORYITEM_PITCH;
	}
}

int16 InventoryRenderer::getNumLines() const {
	int16 num = _inv->getNumItems();
	return (num / INVENTORY_ITEMS_PER_LINE) + ((num % INVENTORY_ITEMS_PER_LINE) > 0 ? 1 : 0);
}


void InventoryRenderer::refresh() {
	for (uint16 i = 0; i < INVENTORY_MAX_ITEMS; i++) {
		ItemName name = _inv->getItemName(i);
		drawItem(i, name);
	}
}

void InventoryRenderer::highlightItem(ItemPosition pos, byte color) {
	if (pos == -1)
		return;

	Common::Rect r;
	getItemRect(pos, r);

	if (color != 12)
		color = 19;

	_surf.frameRect(r, color);
}

void InventoryRenderer::getItemRect(ItemPosition pos, Common::Rect &r) {

	r.setHeight(INVENTORYITEM_HEIGHT);
	r.setWidth(INVENTORYITEM_WIDTH);

	uint16 line = pos / INVENTORY_ITEMS_PER_LINE;
	uint16 col = pos % INVENTORY_ITEMS_PER_LINE;

	r.moveTo(col * INVENTORYITEM_WIDTH, line * INVENTORYITEM_HEIGHT);

}













Inventory::Inventory(uint16 maxItems) : _maxItems(maxItems), _numItems(0) {
	_items = (InventoryItem*)calloc(_maxItems, sizeof(InventoryItem));

	addItem(1, kZoneDoor);
	addItem(3, kZoneExamine);
	addItem(2, kZoneGet);
	addItem(4, kZoneSpeak);
}


Inventory::~Inventory() {
	free(_items);
}

ItemPosition Inventory::addItem(ItemName name, uint32 value) {
	if (_numItems == INVENTORY_MAX_ITEMS)
		return -1;

	// NOTE: items whose name == 0 aren't really inventory items,
	// but the engine expects the inventory to accept them as valid.
	// This nasty trick has been discovered because of regression
	// after r29060.
	if (name == 0)
		return 0;

	_items[_numItems]._id = value;
	_items[_numItems]._index = name;

	_numItems++;

	return _numItems;
}

ItemPosition Inventory::addItem(ItemName name) {
	return addItem(name, MAKE_INVENTORY_ID(name));
}

ItemPosition Inventory::findItem(ItemName name) const {
	for (ItemPosition slot = 0; slot < _numItems; slot++) {
		if (name == _items[slot]._index)
			return slot;
	}

	return -1;
}

void Inventory::removeItem(ItemName name) {
	ItemPosition pos = findItem(name);
	if (pos == -1) {
		return;
	}

	_numItems--;

	if (_numItems != pos) {
		memmove(&_items[pos], &_items[pos+1], (_numItems - pos) * sizeof(InventoryItem));
	}

	_items[_numItems]._id = 0;
	_items[_numItems]._index = 0;
}

void Inventory::clear(bool keepVerbs) {
	uint first = (keepVerbs ? INVENTORY_FIRST_ITEM : 0);

	for (uint16 slot = first; slot < _maxItems; slot++) {
		_items[slot]._id = 0;
		_items[slot]._index = 0;
	}

	_numItems = first;
}


ItemName Inventory::getItemName(ItemPosition pos) const {
	return (pos >= 0 && pos < INVENTORY_MAX_ITEMS) ? _items[pos]._index : 0;
}

const InventoryItem* Inventory::getItem(ItemPosition pos) const {
	return &_items[pos];
}




} // namespace Parallaction
