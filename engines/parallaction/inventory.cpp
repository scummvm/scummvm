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

#include "common/stdafx.h"

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

static byte		*_buffer;
uint16			 _numInvLines = 0;
static Common::Point	 _invPosition;

InventoryItem _inventory[INVENTORY_MAX_ITEMS] = {
	{ kZoneDoor,		1 },		// open/close icon
	{ kZoneExamine, 	3 },		// examine icon
	{ kZoneGet, 		2 },		// pick up/use icon
	{ kZoneSpeak,		4 },		// speak icon
	{ 0,	0 },					// items...
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 },
	{ 0,	0 }
};


int16 getNumUsedSlots() {
	int16 num = 0;
	while (num < INVENTORY_MAX_ITEMS && _inventory[num]._id != 0)
		num++;
	return num;
}


//	get inventory item index at position (x,y)
//	in screen coordinates
//
int16 Parallaction::getHoverInventoryItem(int16 x, int16 y) {

	int16 slot = getNumUsedSlots();
	slot = (slot + 4) / INVENTORY_ITEMS_PER_LINE;

	Common::Rect r(INVENTORY_WIDTH, _numInvLines * INVENTORYITEM_HEIGHT);
	r.moveTo(_invPosition);

	if (!r.contains(Common::Point(x,y)))
		return -1;

	return ((x - _invPosition.x) / INVENTORYITEM_WIDTH) + (INVENTORY_ITEMS_PER_LINE * ((y - _invPosition.y) / INVENTORYITEM_HEIGHT));

}

void drawInventoryItem(uint16 pos, InventoryItem *item) {

	uint16 line = pos / INVENTORY_ITEMS_PER_LINE;
	uint16 col = pos % INVENTORY_ITEMS_PER_LINE;

	// FIXME: this will end up in a general blit function
	byte* s = _vm->_char._objs->getFramePtr(item->_index);
	byte* d = _buffer + col * INVENTORYITEM_WIDTH + line * _vm->_char._objs->_height * INVENTORY_WIDTH;
	for (uint32 i = 0; i < INVENTORYITEM_HEIGHT; i++) {
		memcpy(d, s, INVENTORYITEM_WIDTH);

		d += INVENTORY_WIDTH;
		s += INVENTORYITEM_PITCH;
	}

	return;
}



void refreshInventory() {
	for (uint16 i = 0; i < INVENTORY_MAX_ITEMS; i++)
		drawInventoryItem(i, &_inventory[i]);

	return;
}

int Parallaction::addInventoryItem(uint16 item) {

	int16 slot = getNumUsedSlots();
	if (slot == INVENTORY_MAX_ITEMS)
		return -1;

	_inventory[slot]._id = MAKE_INVENTORY_ID(item);
	_inventory[slot]._index = item;

	return 0;
}


void Parallaction::dropItem(uint16 v) {

	bool found = false;
	for (uint16 slot = 0; slot < INVENTORY_MAX_ITEMS - 1; slot++) {

		if (v == _inventory[slot]._index) {
			found = true;
		}

		if (!found) continue;

		memcpy(&_inventory[slot], &_inventory[slot+1], sizeof(InventoryItem));
	}

	return;
}


int16 Parallaction::isItemInInventory(int32 v) {

	for (uint16 slot = 0; slot < INVENTORY_MAX_ITEMS; slot++) {
		if (_inventory[slot]._id == (uint)v)
			return 1;
	}

	return 0;
}


void drawBorder(const Common::Rect& r, byte *buffer, byte color) {

	byte *d = buffer + r.left + INVENTORY_WIDTH * r.top;

	memset(d, color, r.width());

	for (uint16 i = 0; i < r.height(); i++) {
		d[i * INVENTORY_WIDTH] = color;
		d[i * INVENTORY_WIDTH + r.width() - 1] = color;
	}

	d = buffer + r.left + INVENTORY_WIDTH * (r.bottom - 1);
	memset(d, color, r.width());

	return;
}

//
//	draws a color border around the specified position in the inventory
//
void highlightInventoryItem(int16 pos, byte color) {

	if (color != 12) color = 19;

	if (pos == -1) return;

	uint16 line = pos / INVENTORY_ITEMS_PER_LINE;
	uint16 col = pos % INVENTORY_ITEMS_PER_LINE;

	Common::Rect r(INVENTORYITEM_WIDTH, _vm->_char._objs->_height);
	r.moveTo(col * INVENTORYITEM_WIDTH, line * _vm->_char._objs->_height);

	drawBorder(r, _buffer, color);

	return;
}


int16 getInventoryItemIndex(int16 pos) {
	// TODO: should assert against the number of items actually contained,
	// not the theoretical limit.
	assert(pos >= 0 && pos < INVENTORY_MAX_ITEMS);
	return _inventory[pos]._index;
}



void jobShowInventory(void *parm, Job *j) {
//	printf("job_showInventory()...");

	int16 slot = getNumUsedSlots();
	_numInvLines = (slot + 4) / INVENTORY_ITEMS_PER_LINE;

	Common::Rect r(INVENTORY_WIDTH, _numInvLines * INVENTORYITEM_HEIGHT);

	r.moveTo(_invPosition);

	_vm->_gfx->copyRect(
		Gfx::kBitBack,
		r,
		_buffer,
		INVENTORY_WIDTH
	);

	return;
}



void jobHideInventory(void *parm, Job *j) {
//	printf("job_hideInventory()\n");

	static uint16 count = 0;

	_engineFlags |= kEngineBlockInput;

	count++;
	if (count == 2) {
		count = 0;
		j->_finished = 1;
		_engineFlags &= ~kEngineBlockInput;
	}

	Common::Rect r(INVENTORY_WIDTH, _numInvLines * INVENTORYITEM_HEIGHT);
	r.moveTo(_invPosition);

	_vm->_gfx->restoreBackground(r);

	return;
}



void openInventory() {
	_engineFlags |= kEngineInventory;

	int16 slot = getNumUsedSlots();
	uint16 lines = (slot + 4) / INVENTORY_ITEMS_PER_LINE;

	_invPosition.x = CLIP(_vm->_mousePos.x - (INVENTORY_WIDTH / 2), 0, (int)(_vm->_screenWidth - INVENTORY_WIDTH));
	_invPosition.y = CLIP(_vm->_mousePos.y - 2 - (lines * INVENTORYITEM_HEIGHT), 0, (int)(_vm->_screenHeight - lines * INVENTORYITEM_HEIGHT));

	refreshInventory();

	return;

}



void closeInventory() {
	_engineFlags &= ~kEngineInventory;
}

void initInventory() {
	_buffer = (byte*)malloc(INVENTORY_WIDTH * INVENTORY_HEIGHT);
}

void destroyInventory() {
	if (_buffer)
		free(_buffer);
	_buffer = 0;
}

void cleanInventory() {

	for (uint16 slot = INVENTORY_FIRST_ITEM; slot < INVENTORY_MAX_ITEMS; slot++) {
		_inventory[slot]._id = 0;
		_inventory[slot]._index = 0;
	}

	return;
}


} // namespace Parallaction
