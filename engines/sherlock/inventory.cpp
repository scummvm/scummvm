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
 */

#include "sherlock/inventory.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

Inventory::Inventory(SherlockEngine *vm) : Common::Array<InventoryItem>(), _vm(vm) {
	Common::fill(&_invShapes[0], &_invShapes[MAX_VISIBLE_INVENTORY], (ImageFile *)nullptr);
	_invGraphicsLoaded = false;
	_invIndex = 0;
	_holdings = 0;
	_oldFlag = 0;
	_invFlag = 0;
}

Inventory::~Inventory() {
	freeGraphics();
}

void Inventory::freeInv() {
	freeGraphics();

	_names.clear();
	_invGraphicsLoaded = false;
}

/**
 * Free any loaded inventory graphics
 */
void Inventory::freeGraphics() {
	for (uint idx = 0; idx < MAX_VISIBLE_INVENTORY; ++idx)
		delete _invShapes[idx];
	
	Common::fill(&_invShapes[0], &_invShapes[MAX_VISIBLE_INVENTORY], (ImageFile *)nullptr);
	_invGraphicsLoaded = false;
}

/** Load the list of names the inventory items correspond to.
 */
void Inventory::loadInv() {
	// Exit if the inventory names are already loaded
	if (_names.size() > 0)
		return;

	// Load the inventory names
	Common::SeekableReadStream *stream = _vm->_res->load("invent.txt");
	_names.clear();

	while (stream->pos() < stream->size()) {
		Common::String name;
		char c;
		while ((c = stream->readByte()) != 0)
			name += c;

		_names.push_back(name);
	}
	
	delete stream;
}

/**
 * Load the list of names of graphics for the inventory
 */
void Inventory::loadGraphics() {
	if (_invGraphicsLoaded)
		return;

	// Default all inventory slots to empty
	Common::fill(&_invShapes[0], &_invShapes[MAX_VISIBLE_INVENTORY], (ImageFile *)nullptr);

	for (int idx = _invIndex; (idx < _holdings) && (idx - _invIndex) < 6; ++idx) {
		// Get the name of the item to be dispalyed, figure out it's accompanying
		// .VGS file with it's picture, and then load it
		int invNum = findInv((*this)[idx]._name);
		Common::String fName = Common::String::format("item%02d.vgs", invNum);

		_invShapes[idx] = new ImageFile(fName);
	}

	_invGraphicsLoaded = true;
}

/**
 * Searches through the list of names that correspond to the inventory items
 * and returns the numer that matches the passed name
 */
int Inventory::findInv(const Common::String &name) {
	int result = -1;

	for (int idx = 0; (idx < _holdings) && result == -1; ++idx) {
		if (scumm_stricmp(name.c_str(), _names[idx].c_str()) == 0)
			result = idx;
	}

	if (result == -1)
		result = 1;
	return result;
}

void Inventory::putInv(int slamit) {
	// TODO
}

/**
 * Put the game into inventory mode and open the interface window.
 * The flag parameter specifies the mode:
 * 0   = plain inventory mode
 * 2   = use inventory mode
 * 3   = give inventory mode
 * 128 = Draw window in the back buffer, but don't display it 
 */
void Inventory::drawInventory(int flag) {
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	int tempFlag = flag;

	_oldFlag = 7;
	loadInv();

	if (flag == 128) {
		screen._backBuffer = &screen._backBuffer2;
	}

	// Draw the window background
	Surface &bb = *screen._backBuffer;
	bb.fillRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y1 + 10), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, CONTROLS_Y1 + 10, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y1 + 10, 
		SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 2, SHERLOCK_SCREEN_WIDTH,
		SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	bb.fillRect(Common::Rect(2, CONTROLS_Y1 + 10, SHERLOCK_SCREEN_WIDTH - 2, SHERLOCK_SCREEN_HEIGHT - 2),
		INV_BACKGROUND);

	// Draw the buttons
	screen.makeButton(Common::Rect(INVENTORY_POINTS[0][0], CONTROLS_Y1, INVENTORY_POINTS[0][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[0][2] - screen.stringWidth("Exit") / 2, "Exit");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[1][0], CONTROLS_Y1, INVENTORY_POINTS[1][1],
		CONTROLS_Y1 + 9), INVENTORY_POINTS[1][2] - screen.stringWidth("Look") / 2, "Look");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[2][0], CONTROLS_Y1, INVENTORY_POINTS[2][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[2][2] - screen.stringWidth("Use") / 2, "Use");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[3][0], CONTROLS_Y1, INVENTORY_POINTS[3][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[3][2] - screen.stringWidth("Give") / 2, "Give");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[4][0], CONTROLS_Y1, INVENTORY_POINTS[4][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[4][2], "^^");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[5][0], CONTROLS_Y1, INVENTORY_POINTS[5][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[5][2], "^");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[6][0], CONTROLS_Y1, INVENTORY_POINTS[6][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[6][2], "_");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[7][0], CONTROLS_Y1, INVENTORY_POINTS[7][1], 
		CONTROLS_Y1 + 9), INVENTORY_POINTS[7][2], "__");

	if (tempFlag == 128)
		flag = 1;
	ui._invMode = flag;

	if (flag) {
		ui._oldKey = INVENTORY_COMMANDS[flag];
		_oldFlag = flag;
	} else {
		ui._oldKey = -1;
		_invFlag = 6;
	}

	invCommands(0);
	putInv(0);

	if (tempFlag != 128) {
		if (!ui._windowStyle) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		} else {
			ui.summonWindow(false, CONTROLS_Y1);
		}

		ui._windowOpen = true;
	} else {
		// Reset the screen back buffer to the first buffer now that drawing is done
		screen._backBuffer = &screen._backBuffer1;
	}

	ui._oldUse = -1;
}

void Inventory::invCommands(bool slamIt) {
	// TODO
}

void Inventory::doInvLite(int index, byte color) {
	// TODO
}

void Inventory::doInvJF() {
	// TODO
}

} // End of namespace Sherlock
