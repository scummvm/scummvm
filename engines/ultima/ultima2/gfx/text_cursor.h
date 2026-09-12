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

#ifndef ULTIMA2_GFX_TEXT_CURSOR_H
#define ULTIMA2_GFX_TEXT_CURSOR_H

#include "ultima/shared/engine/events.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

/**
 * Blinking text-input cursor shown after an input prompt (the main menu's
 * "CHOICE:", and later character creation's text entry). Doesn't handle
 * keypresses itself - that's left to whichever view owns it.
 */
class TextCursor : public Shared::UIElement {
private:
	bool _visible = true;

public:
	TextCursor(const Common::String &name, UIElement *parent = nullptr);
	~TextCursor() override {}

	void draw() override;
	void timeout() override;

	/**
	 * Shows the cursor and (re)starts its blink cycle
	 */
	void show();

	/**
	 * Hides the cursor
	 */
	void hide();

	/**
	 * Sets the screen position of the cursor
	 */
	void setPosition(const Common::Point &pt);
};

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima

#endif
