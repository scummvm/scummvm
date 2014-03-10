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

#include "common/events.h"
#include "common/rect.h"

namespace Illusions {

class Input {
public:
	Input();
	void processEvent(Common::Event event);
	bool pollButton(uint buttons);
	bool lookButtonStates(uint buttons);
	bool lookNewButtons(uint buttons);
	void setButtonState(uint buttons);
	void discardButtons(uint buttons);
	void activateButton(uint buttons);
	void deactivateButton(uint buttons);
	Common::Point getCursorPosition();
	void setCursorPosition(Common::Point mousePos);
	Common::Point getCursorDelta();
protected:
	uint _buttonStates, _newButtons;
	uint _enabledButtons;
	Common::Point _cursorPos, _prevCursorPos;
};

} // End of namespace Illusions

#endif // ILLUSIONS_INPUT_H
