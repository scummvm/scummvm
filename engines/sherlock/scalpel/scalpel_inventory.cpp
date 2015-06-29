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

#include "sherlock/scalpel/scalpel_inventory.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

ScalpelInventory::ScalpelInventory(SherlockEngine *vm) : Inventory(vm) {
	_invShapes.resize(6);
}

ScalpelInventory::~ScalpelInventory() {
}

void ScalpelInventory::drawInventory(InvNewMode mode) {
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
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
	Common::String fixedText_Exit = fixedText.getText(kFixedText_Inventory_Exit);
	Common::String fixedText_Look = fixedText.getText(kFixedText_Inventory_Look);
	Common::String fixedText_Use  = fixedText.getText(kFixedText_Inventory_Use);
	Common::String fixedText_Give = fixedText.getText(kFixedText_Inventory_Give);

	screen.makeButton(Common::Rect(INVENTORY_POINTS[0][0], CONTROLS_Y1, INVENTORY_POINTS[0][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[0][2] - screen.stringWidth(fixedText_Exit) / 2, fixedText_Exit);
	screen.makeButton(Common::Rect(INVENTORY_POINTS[1][0], CONTROLS_Y1, INVENTORY_POINTS[1][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[1][2] - screen.stringWidth(fixedText_Look) / 2, fixedText_Look);
	screen.makeButton(Common::Rect(INVENTORY_POINTS[2][0], CONTROLS_Y1, INVENTORY_POINTS[2][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[2][2] - screen.stringWidth(fixedText_Use) / 2, fixedText_Use);
	screen.makeButton(Common::Rect(INVENTORY_POINTS[3][0], CONTROLS_Y1, INVENTORY_POINTS[3][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[3][2] - screen.stringWidth(fixedText_Give) / 2, fixedText_Give);
	screen.makeButton(Common::Rect(INVENTORY_POINTS[4][0], CONTROLS_Y1, INVENTORY_POINTS[4][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[4][2], "^^"); // 2 arrows pointing to the left
	screen.makeButton(Common::Rect(INVENTORY_POINTS[5][0], CONTROLS_Y1, INVENTORY_POINTS[5][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[5][2], "^"); // 1 arrow pointing to the left
	screen.makeButton(Common::Rect(INVENTORY_POINTS[6][0], CONTROLS_Y1, INVENTORY_POINTS[6][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[6][2], "_"); // 1 arrow pointing to the right
	screen.makeButton(Common::Rect(INVENTORY_POINTS[7][0], CONTROLS_Y1, INVENTORY_POINTS[7][1],
		CONTROLS_Y1 + 10), INVENTORY_POINTS[7][2], "__"); // 2 arrows pointing to the right

	if (tempMode == INVENTORY_DONT_DISPLAY)
		mode = LOOK_INVENTORY_MODE;
	_invMode = (InvMode)((int)mode);

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

	assert(IS_SERRATED_SCALPEL);
	((ScalpelUserInterface *)_vm->_ui)->_oldUse = -1;
}

void ScalpelInventory::invCommands(bool slamIt) {
	FixedText &fixedText = *_vm->_fixedText;
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	Common::String fixedText_Exit = fixedText.getText(kFixedText_Inventory_Exit);
	Common::String fixedText_Look = fixedText.getText(kFixedText_Inventory_Look);
	Common::String fixedText_Use  = fixedText.getText(kFixedText_Inventory_Use);
	Common::String fixedText_Give = fixedText.getText(kFixedText_Inventory_Give);

	if (slamIt) {
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[0][2], CONTROLS_Y1),
			_invMode == INVMODE_EXIT ? COMMAND_HIGHLIGHTED :COMMAND_FOREGROUND,
			true, fixedText_Exit);
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[1][2], CONTROLS_Y1),
			_invMode == INVMODE_LOOK ? COMMAND_HIGHLIGHTED :COMMAND_FOREGROUND,
			true, fixedText_Look);
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[2][2], CONTROLS_Y1),
			_invMode == INVMODE_USE ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			true, fixedText_Use);
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[3][2], CONTROLS_Y1),
			_invMode == INVMODE_GIVE ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			true, fixedText_Give);
		screen.print(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1 + 1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^^"); // 2 arrows pointing to the left
		screen.print(Common::Point(INVENTORY_POINTS[5][2], CONTROLS_Y1 + 1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^"); // 2 arrows pointing to the left
		screen.print(Common::Point(INVENTORY_POINTS[6][2], CONTROLS_Y1 + 1),
			(_holdings - _invIndex <= 6) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"_"); // 1 arrow pointing to the right
		screen.print(Common::Point(INVENTORY_POINTS[7][2], CONTROLS_Y1 + 1),
			(_holdings - _invIndex <= 6) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"__"); // 2 arrows pointing to the right
		if (_invMode != INVMODE_LOOK)
			ui.clearInfo();
	} else {
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[0][2], CONTROLS_Y1),
			_invMode == INVMODE_EXIT ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, fixedText_Exit);
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[1][2], CONTROLS_Y1),
			_invMode == INVMODE_LOOK ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, fixedText_Look);
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[2][2], CONTROLS_Y1),
			_invMode == INVMODE_USE ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, fixedText_Use);
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[3][2], CONTROLS_Y1),
			_invMode == INVMODE_GIVE ? COMMAND_HIGHLIGHTED : COMMAND_FOREGROUND,
			false, fixedText_Give);
		screen.gPrint(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^^"); // 2 arrows pointing to the left
		screen.gPrint(Common::Point(INVENTORY_POINTS[5][2], CONTROLS_Y1),
			_invIndex == 0 ? COMMAND_NULL : COMMAND_FOREGROUND,
			"^"); // 1 arrow pointing to the left
		screen.gPrint(Common::Point(INVENTORY_POINTS[6][2], CONTROLS_Y1),
			(_holdings - _invIndex < 7) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"_"); // 1 arrow pointing to the right
		screen.gPrint(Common::Point(INVENTORY_POINTS[7][2], CONTROLS_Y1),
			(_holdings - _invIndex < 7) ? COMMAND_NULL : COMMAND_FOREGROUND,
			"__"); // 2 arrows pointing to the right
	}
}

void ScalpelInventory::highlight(int index, byte color) {
	Screen &screen = *_vm->_screen;
	Surface &bb = *screen._backBuffer;
	int slot = index - _invIndex;
	ImageFrame &frame = (*_invShapes[slot])[0];

	bb.fillRect(Common::Rect(8 + slot * 52, 165, (slot + 1) * 52, 194), color);
	bb.transBlitFrom(frame, Common::Point(6 + slot * 52 + ((47 - frame._width) / 2),
		163 + ((33 - frame._height) / 2)));
	screen.slamArea(8 + slot * 52, 165, 44, 30);
}

void ScalpelInventory::refreshInv() {
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	ScalpelUserInterface &ui = *(ScalpelUserInterface *)_vm->_ui;

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

void ScalpelInventory::putInv(InvSlamMode slamIt) {
	ScalpelScreen &screen = *(ScalpelScreen *)_vm->_screen;
	UserInterface &ui = *_vm->_ui;

	// If an inventory item has disappeared (due to using it or giving it),
	// a blank space slot may have appeared. If so, adjust the inventory
	if (_invIndex > 0 && _invIndex > (_holdings - (int)_invShapes.size())) {
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
	for (int idx = _invIndex; idx < _holdings && (idx - _invIndex) < (int)_invShapes.size(); ++idx) {
		int itemNum = idx - _invIndex;
		Surface &bb = slamIt == SLAM_SECONDARY_BUFFER ? screen._backBuffer2 : screen._backBuffer1;
		Common::Rect r(8 + itemNum * 52, 165, 51 + itemNum * 52, 194);

		// Draw the background
		if (idx == ui._selector) {
			bb.fillRect(r, BUTTON_BACKGROUND);
		}
		else if (slamIt == SLAM_SECONDARY_BUFFER) {
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
	}
	else if (slamIt == SLAM_SECONDARY_BUFFER) {
		screen._backBuffer = &screen._backBuffer2;
		invCommands(0);
		screen._backBuffer = &screen._backBuffer1;
	}
}

void ScalpelInventory::loadInv() {
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

} // End of namespace Scalpel

} // End of namespace Sherlock
