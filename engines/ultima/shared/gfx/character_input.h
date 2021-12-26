/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA_GFX_CHARACTER_INPUT_H
#define ULTIMA_GFX_CHARACTER_INPUT_H

#include "ultima/shared/gfx/popup.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

/**
 * Text input control
 */
class CharacterInput : public Popup {
	DECLARE_MESSAGE_MAP;
	bool KeypressMsg(CKeypressMsg *msg);
private:
	byte _color;
public:
	CLASSDEF;
	CharacterInput(GameBase *game) : Popup(game), _color(0) {
	}

	/**
	 * Show the character input
	 * @param pt		Position of the input
	 * @param color		Text color
	 * @param respondTo	Where to send response to
	 * @remarks			Generates a TextInputMsg message with the character when pressed
	 */
	void show(const Point &pt, byte color, TreeItem *respondTo = nullptr);
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
