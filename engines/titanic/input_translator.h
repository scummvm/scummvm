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

#ifndef TITANIC_INPUT_TRANSLATOR_H
#define TITANIC_INPUT_TRANSLATOR_H

#include "common/keyboard.h"
#include "titanic/messages/mouse_messages.h"

namespace Titanic {

class CInputHandler;

class CInputTranslator {
private:
	/**
	 * Converts the special buttons bitset into a buttons bitset
	 */
	int getButtons(int special) const;

	/**
	 * Returns true if a key down contains a special non-ascii key
	 * that should still be passed onto the game
	 */
	bool isSpecialKey(Common::KeyCode key);
public:
	CInputHandler *_inputHandler;
public:
	CInputTranslator(CInputHandler *inputHandler);

	void mouseMove(int special, const Point &pt);
	void leftButtonDown(int special, const Point &pt);
	void leftButtonUp(int special, const Point &pt);
	void leftButtonDoubleClick(int special, const Point &pt);
	void middleButtonDown(int special, const Point &pt);
	void middleButtonUp(int special, const Point &pt);
	void middleButtonDoubleClick(int special, const Point &pt);
	void mouseWheel(bool wheelUp, const Point &pt);
	void keyDown(const Common::KeyState &keyState);

	/**
	 * Returns true if any mouse button is currently pressed
	 */
	bool isMousePressed() const;
};

} // End of namespace Titanic

#endif /* TITANIC_INPUT_TRANSLATOR_H */
