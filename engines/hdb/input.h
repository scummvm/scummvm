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

#ifndef HDB_INPUT_H
#define HDB_INPUT_H

#include "common/events.h"

namespace HDB {

enum Button {
	kButtonUp		= 2 << 0,
	kButtonDown		= 2 << 1,
	kButtonLeft		= 2 << 2,
	kButtonRight	= 2 << 3,
	kButtonA		= 2 << 4,
	kButtonB		= 2 << 5,
	kButtonC		= 2 << 6,
	kButtonD		= 2 << 7,
	kButtonExit		= 2 << 8
};

enum HDBAction {
	kHDBActionNone,
	kHDBActionUp,
	kHDBActionDown,
	kHDBActionLeft,
	kHDBActionRight,
	kHDBActionUse,
	kHDBActionClearWaypoints,
	kHDBActionInventory,
	kHDBActionMenu,
	kHDBActionPause,
	kHDBActionDebug,
	kHDBActionQuit
};

class Input {
public:

	void init();

	void setButtons(uint16 b);
	uint16 getButtons();
	void stylusDown(int x, int y);
	void stylusUp(int x, int y);
	void stylusMove(int x, int y);

	void updateMouse(int newX, int newY);
	void updateMouseButtons(bool isDown);
	void updateActions(Common::Event event, bool keyDown, bool fromMouse);

	int getMouseX() {
		return _mouseX;
	}
	int getMouseY() {
		return _mouseY;
	}

private:

	uint16 _buttons;	// Flags for buttons
	bool _stylusDown;
	int _mouseX, _mouseY;
};

} // End of Namespace

#endif // !HDB_INPUT_H
