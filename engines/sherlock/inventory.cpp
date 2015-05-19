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

#include "sherlock/inventory.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

InventoryItem::InventoryItem(int requiredFlag, const Common::String &name,
		const Common::String &description, const Common::String &examine) :
		_requiredFlag(requiredFlag), _name(name), _description(description),
		_examine(examine), _lookFlag(0) {
}

void InventoryItem::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_requiredFlag);
	s.syncAsSint16LE(_lookFlag);
	s.syncString(_name);
	s.syncString(_description);
	s.syncString(_examine);
}

/*----------------------------------------------------------------*/

Inventory::Inventory(SherlockEngine *vm) : Common::Array<InventoryItem>(), _vm(vm) {
	Common::fill(&_invShapes[0], &_invShapes[MAX_VISIBLE_INVENTORY], (ImageFile *)nullptr);
	_invGraphicsLoaded = false;
	_invIndex = 0;
	_holdings = 0;
	_invMode = INVMODE_EXIT;
}

Inventory::~Inventory() {
	freeGraphics();
}

void Inventory::freeInv() {
	freeGraphics();

	_names.clear();
	_invGraphicsLoaded = false;
}

void Inventory::freeGraphics() {
	for (uint idx = 0; idx < MAX_VISIBLE_INVENTORY; ++idx)
		delete _invShapes[idx];

	Common::fill(&_invShapes[0], &_invShapes[MAX_VISIBLE_INVENTORY], (ImageFile *)nullptr);
	_invGraphicsLoaded = false;
}

void Inventory::loadInv() {
	// Exit if the inventory names are already loaded
	if (_names.size() > 0)
		return;

	// Load the inventory names
	Common::SeekableReadStream *stream = _vm->_res->load("invent.txt");

	int streamSize = stream->size();
	while (stream->pos() < streamSize) {
		Common::String name;
		char c;
		while ((c = stream->readByte()) != 0)
			name += c;

		_names.push_back(name);
	}

	delete stream;

	loadGraphics();
}

void Inventory::loadGraphics() {
	if (_invGraphicsLoaded)
		return;

	// Default all inventory slots to empty
	Common::fill(&_invShapes[0], &_invShapes[MAX_VISIBLE_INVENTORY], (ImageFile *)nullptr);

	for (int idx = _invIndex; (idx < _holdings) && (idx - _invIndex) < MAX_VISIBLE_INVENTORY; ++idx) {
		// Get the name of the item to be displayed, figure out its accompanying
		// .VGS file with its picture, and then load it
		int invNum = findInv((*this)[idx]._name);
		Common::String fName = Common::String::format("item%02d.vgs", invNum + 1);

		_invShapes[idx - _invIndex] = new ImageFile(fName);
	}

	_invGraphicsLoaded = true;
}

int Inventory::findInv(const Common::String &name) {
	for (int idx = 0; idx < (int)_names.size(); ++idx) {
		if (name.equalsIgnoreCase(_names[idx]))
			return idx;
	}

	// Couldn't find the desired item
	error("Couldn't find inventory item - %s", name.c_str());
}

void Inventory::putInv(InvSlamMode slamIt) {
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	// If an inventory item has disappeared (due to using it or giving it),
	// a blank space slot may have appeared. If so, adjust the inventory
	if (_invIndex > 0 && _invIndex > (_holdings - 6)) {
		--_invIndex;
		freeGraphics();
		loadGraphics();
	}

	if (slamIt != SLAM_SECONDARY_BUFFER) {
		screen.makePanel(Common::Rect(6, 163, 54, 197));
		screen.makePanel(Common::Rect(58, 163, 106, 197));
		screen.makePanel(Common::Rect(110, 163, 158, 197));
		screen.makePanel(Common::Rect(162, 163, 210, 197));
		screen.makePanel(Common::Rect(214, 163, 262, 197));
		screen.makePanel(Common::Rect(266, 163, 314, 197));
	}

	// Iterate through displaying up to 6 objects at a time
	for (int idx = _invIndex; idx < _holdings && (idx - _invIndex) < MAX_VISIBLE_INVENTORY; ++idx) {
		int itemNum = idx - _invIndex;
		Surface &bb = slamIt == SLAM_SECONDARY_BUFFER ? screen._backBuffer2 : screen._backBuffer1;
		Common::Rect r(8 + itemNum * 52, 165, 51 + itemNum * 52, 194);

		// Draw the background
		if (idx == ui._selector) {
			bb.fillRect(r, 235);
		} else if (slamIt == SLAM_SECONDARY_BUFFER) {
			bb.fillRect(r, BUTTON_MIDDLE);
		}

		// Draw the item image
		ImageFrame &frame = (*_invShapes[itemNum])[0];
		bb.transBlitFrom(frame, Common::Point(6 + itemNum * 52 + ((47 - frame._width) / 2),
			163 + ((33 - frame._height) / 2)));
	}

	if (slamIt == SLAM_DISPLAY)
		screen.slamArea(6, 163, 308, 34);

	if (slamIt != SLAM_SECONDARY_BUFFER)
		ui.clearInfo();

	if (slamIt == 0) {
		invCommands(0);
	} else if (slamIt == SLAM_SECONDARY_BUFFER) {
		screen._backBuffer = &screen._backBuffer2;
		invCommands(0);
		screen._backBuffer = &screen._backBuffer1;
	}
}

void Inventory::drawInventory(InvNewMode mode) {
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;
	InvNewMode tempMode = mode;

	loadInv();

	if (mode == INVENTORY_DONT_DISPLAY) {
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
		CONTROLS_Y1 + 10), INVENTORY_POINTS[0][2] - screen.stringWidth("Exit") / 2, "Exit");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[1][0], CONTROLS_Y1, INVENTORY_POINTS[1][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[1][2] - screen.stringWidth("Look") / 2, "Look");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[2][0], CONTROLS_Y1, INVENTORY_POINTS[2][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[2][2] - screen.stringWidth("Use") / 2, "Use");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[3][0], CONTROLS_Y1, INVENTORY_POINTS[3][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[3][2] - screen.stringWidth("Give") / 2, "Give");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[4][0], CONTROLS_Y1, INVENTORY_POINTS[4][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[4][2], "^^");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[5][0], CONTROLS_Y1, INVENTORY_POINTS[5][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[5][2], "^");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[6][0], CONTROLS_Y1, INVENTORY_POINTS[6][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[6][2], "_");
	screen.makeButton(Common::Rect(INVENTORY_POINTS[7][0], CONTROLS_Y1, INVENTORY_POINTS[7][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[7][2], "__");

	if (tempMode == INVENTORY_DONT_DISPLAY)
		mode = LOOK_INVENTORY_MODE;
	_invMode = (InvMode)mode;

	if (mode != PLAIN_INVENTORY) {
		ui._oldKey = INVENTORY_COMMANDS[(int)mode];
	} else {
		ui._oldKey = -1;
	}

	invCommands(0);
	putInv(SLAM_DONT_DISPLAY);

	if (tempMode != INVENTORY_DONT_DISPLAY) {
		if (!ui._slideWindows) {
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
	Screen &screen = *_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	if (slamIt) {
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[0][2], CONTROLS_Y1),
			_invMode == 0 ? COMMAND_HIGHLIGHTED :COMMAND_FOREGROUND,
			true, "Exit");
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[1][2], CONTROLS_Y1),
			_invMode == 1 ? COMMAND_HIGHLIGHTED :COMMAND_FOREGROUND,
			true, "Look");
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[2][2], CONTROLS_Y1),
			_invMode == 2 ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			true, "Use");
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[3][2], CONTROLS_Y1),
			_invMode == 3 ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			true, "Give");
		screen.print(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1 + 1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^^");
		screen.print(Common::Point(INVENTORY_POINTS[5][2], CONTROLS_Y1 + 1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^");
		screen.print(Common::Point(INVENTORY_POINTS[6][2], CONTROLS_Y1 + 1),
			(_holdings - _invIndex <= 6) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"_");
		screen.print(Common::Point(INVENTORY_POINTS[7][2], CONTROLS_Y1 + 1),
			(_holdings - _invIndex <= 6) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"__");
		if (_invMode != 1)
			ui.clearInfo();
	} else {
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[0][2], CONTROLS_Y1),
			_invMode == 0 ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, "Exit");
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[1][2], CONTROLS_Y1),
			_invMode == 1 ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, "Look");
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[2][2], CONTROLS_Y1),
			_invMode == 2 ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, "Use");
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[3][2], CONTROLS_Y1),
			_invMode == 3 ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, "Give");
		screen.gPrint(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^^");
		screen.gPrint(Common::Point(INVENTORY_POINTS[5][2], CONTROLS_Y1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^");
		screen.gPrint(Common::Point(INVENTORY_POINTS[6][2], CONTROLS_Y1),
			(_holdings - _invIndex < 7) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"_");
		screen.gPrint(Common::Point(INVENTORY_POINTS[7][2], CONTROLS_Y1),
			(_holdings - _invIndex < 7) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"__");
	}
}

void Inventory::highlight(int index, byte color) {
	Screen &screen = *_vm->_screen;
	Surface &bb = *screen._backBuffer;
	int slot = index - _invIndex;
	ImageFrame &frame = (*_invShapes[slot])[0];

	bb.fillRect(Common::Rect(8 + slot * 52, 165, (slot + 1) * 52, 194), color);
	bb.transBlitFrom(frame, Common::Point(6 + slot * 52 + ((47 - frame._width) / 2),
		163 + ((33 - frame._height) / 2)));
	screen.slamArea(8 + slot * 52, 165, 44, 30);
}

void Inventory::refreshInv() {
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;

	ui._invLookFlag = true;
	freeInv();

	ui._infoFlag = true;
	ui.clearInfo();

	screen._backBuffer2.blitFrom(screen._backBuffer1, Common::Point(0, CONTROLS_Y),
		Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	ui.examine();

	if (!talk._talkToAbort) {
		screen._backBuffer2.blitFrom((*ui._controlPanel)[0], Common::Point(0, CONTROLS_Y));
		loadInv();
	}
}

int Inventory::putNameInInventory(const Common::String &name) {
	Scene &scene = *_vm->_scene;
	int matches = 0;

	for (uint idx = 0; idx < scene._bgShapes.size(); ++idx) {
		Object &o = scene._bgShapes[idx];
		if (name.equalsIgnoreCase(o._name) && o._type != INVALID) {
			putItemInInventory(o);
			++matches;
		}
	}

	return matches;
}

int Inventory::putItemInInventory(Object &obj) {
	Scene &scene = *_vm->_scene;
	int matches = 0;
	bool pickupFound = false;

	if (obj._pickupFlag)
		_vm->setFlags(obj._pickupFlag);

	for (int useNum = 0; useNum < USE_COUNT; ++useNum) {
		if (obj._use[useNum]._target.equalsIgnoreCase("*PICKUP*")) {
			pickupFound = true;

			for (int namesNum = 0; namesNum < NAMES_COUNT; ++namesNum) {
				for (uint bgNum = 0; bgNum < scene._bgShapes.size(); ++bgNum) {
					Object &bgObj = scene._bgShapes[bgNum];
					if (obj._use[useNum]._names[namesNum].equalsIgnoreCase(bgObj._name)) {
						copyToInventory(bgObj);
						if (bgObj._pickupFlag)
							_vm->setFlags(bgObj._pickupFlag);

						if (bgObj._type == ACTIVE_BG_SHAPE || bgObj._type == NO_SHAPE || bgObj._type == HIDE_SHAPE) {
							if (bgObj._imageFrame == nullptr || bgObj._frameNumber < 0)
								// No shape to erase, so flag as hidden
								bgObj._type = INVALID;
							else
								bgObj._type = REMOVE;
						} else if (bgObj._type == HIDDEN) {
							bgObj._type = INVALID;
						}

						++matches;
					}
				}
			}
		}
	}

	if (!pickupFound) {
		// No pickup item found, so add the passed item
		copyToInventory(obj);
		matches = 0;
	}

	if (matches == 0) {
		if (!pickupFound)
			matches = 1;

		if (obj._type == ACTIVE_BG_SHAPE || obj._type == NO_SHAPE || obj._type == HIDE_SHAPE) {
			if (obj._imageFrame == nullptr || obj._frameNumber < 0)
				// No shape to erase, so flag as hidden
				obj._type = INVALID;
			else
				obj._type = REMOVE;
		} else if (obj._type == HIDDEN) {
			obj._type = INVALID;
		}
	}

	return matches;
}

void Inventory::copyToInventory(Object &obj) {
	InventoryItem invItem;
	invItem._name = obj._name;
	invItem._description = obj._description;
	invItem._examine = obj._examine;
	invItem._lookFlag = obj._lookFlag;
	invItem._requiredFlag = obj._requiredFlag;

	insert_at(_holdings, invItem);
	++_holdings;
}

int Inventory::deleteItemFromInventory(const Common::String &name) {
	int invNum = -1;

	for (int idx = 0; idx < (int)size() && invNum == -1; ++idx) {
		if (name.equalsIgnoreCase((*this)[idx]._name))
			invNum = idx;
	}

	if (invNum == -1)
		// Item not present
		return 0;

	// Item found, so delete it
	remove_at(invNum);
	--_holdings;

	return 1;
}

void Inventory::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_holdings);

	uint count = size();
	s.syncAsUint16LE(count);
	if (s.isLoading()) {
		resize(count);

		// Reset inventory back to start
		_invIndex = 0;
	}

	for (uint idx = 0; idx < size(); ++idx) {
		(*this)[idx].synchronize(s);

	}
}

} // End of namespace Sherlock
