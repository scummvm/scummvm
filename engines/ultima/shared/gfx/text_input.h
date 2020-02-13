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

#ifndef ULTIMA_GFX_TEXT_INPUT_H
#define ULTIMA_GFX_TEXT_INPUT_H

#include "ultima/shared/gfx/popup.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

/**
 * Text input control
 */
class TextInput : public Popup {
	DECLARE_MESSAGE_MAP;
	bool KeypressMsg(CKeypressMsg &msg);
private:
	bool _isNumeric;
	size_t _maxCharacters;
	Common::String _text;
	byte _color;
public:
	CLASSDEF;
	TextInput(GameBase *game) : Popup(game), _isNumeric(false), _maxCharacters(0), _color(0) {}

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;

	/**
	 * Show the text input
	 * @param pt				Position of the input
	 * @param maxCharacters		Maximum length of input
	 * @param color				Text color
	 * @param respondTo			Element to send any response to when the popup closes.
	 *							If not provided, any response goes to the active view
	 * @remarks		Generates a TextInputMsg message with the text when Enter or escape is pressed
	 */
	void show(const Point &pt, bool isNumeric, size_t maxCharacters, byte color,
		TreeItem *respondTo = nullptr);
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
