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

#ifndef ILLUSIONS_INPUT_H
#define ILLUSIONS_INPUT_H

#include "common/array.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/rect.h"

namespace Illusions {

enum {
	MOUSE_NONE         = 0,
	MOUSE_LEFT_BUTTON  = 1,
	MOUSE_RIGHT_BUTTON = 2
};

enum {
	kEventLeftClick    = 0,
	kEventRightClick   = 1,
	kEventInventory    = 2,
	kEventAbort        = 3,
	kEventSkip         = 4,
	kEventF1           = 5,
	kEventUp           = 6,
	kEventDown         = 7,
	kEventMax
};

struct KeyMapping {
	Common::KeyCode _key;
	int _mouseButton;
	bool _down;
};

class KeyMap : public Common::Array<KeyMapping> {
public:
	void addKey(Common::KeyCode key);
	void addMouseButton(int mouseButton);
protected:
	void add(Common::KeyCode key, int mouseButton);
};

class InputEvent {
public:
	InputEvent();
	InputEvent& setBitMask(uint bitMask);
	InputEvent& addKey(Common::KeyCode key);
	InputEvent& addMouseButton(int mouseButton);
	uint handle(Common::KeyCode key, int mouseButton, bool down);
	uint getBitMask() const { return _bitMask; }
protected:
	uint _bitMask;
	KeyMap _keyMap;
};

class Input {
public:
	Input();
	void processEvent(Common::Event event);
	bool pollEvent(uint evt);
	bool hasNewEvents();
	void discardEvent(uint evt);
	void discardAllEvents();
	bool pollButton(uint bitMask);
	void activateButton(uint bitMask);
	void deactivateButton(uint bitMask);
	Common::Point getCursorPosition();
	void setCursorPosition(Common::Point mousePos);
	Common::Point getCursorDelta();
	InputEvent& setInputEvent(uint evt, uint bitMask);
	bool isCursorMovedByKeyboard() const { return _cursorMovedByKeyboard; }
	bool isCheatModeActive();
protected:
	uint _cheatCodeIndex;
	uint _buttonStates, _newButtons, _buttonsDown;
	uint _enabledButtons;
	uint _newKeys;
	Common::Point _cursorPos, _prevCursorPos;
	InputEvent _inputEvents[kEventMax];
	bool _cursorMovedByKeyboard;
	void handleKey(Common::KeyCode key, int mouseButton, bool down);
	void handleMouseButton(int mouseButton, bool down);
	void discardButtons(uint bitMask);
	bool lookButtonStates(uint bitMask);
	bool lookNewButtons(uint bitMask);
	void setButtonState(uint bitMask);
	void moveCursorByKeyboard(int deltaX, int deltaY);
};

} // End of namespace Illusions

#endif // ILLUSIONS_INPUT_H
