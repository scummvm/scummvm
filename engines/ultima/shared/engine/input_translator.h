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

#ifndef ULTIMA_SHARED_ENGINE_INPUT_TRANSLATOR_H
#define ULTIMA_SHARED_ENGINE_INPUT_TRANSLATOR_H

#include "common/keyboard.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Shared {

class InputHandler;

class InputTranslator {
private:
	/**
	 * Converts the special buttons bitset into a buttons bitset
	 */
	int getButtons(int special) const;
public:
	InputHandler *_inputHandler;
public:
	InputTranslator(InputHandler *inputHandler);

	void mouseMove(int special, const Point &pt);
	void mouseDrag(int special, const Point &pt);
	void leftButtonDown(int special, const Point &pt);
	void leftButtonUp(int special, const Point &pt);
	void leftButtonDoubleClick(int special, const Point &pt);
	void middleButtonDown(int special, const Point &pt);
	void middleButtonUp(int special, const Point &pt);
	void middleButtonDoubleClick(int special, const Point &pt);
	void rightButtonDown(int special, const Point &pt);
	void rightButtonUp(int special, const Point &pt);
	void mouseWheel(bool wheelUp, const Point &pt);
	void rightButtonDoubleClick(int special, const Point &pt);
	void keyDown(const Common::KeyState &keyState);

	/**
	 * Returns true if any mouse button is currently pressed
	 */
	bool isMousePressed() const;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
