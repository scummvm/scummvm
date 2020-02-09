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

#ifndef ULTIMA_SHARED_GFX_SCREEN_H
#define ULTIMA_SHARED_GFX_SCREEN_H

#include "common/list.h"
#include "ultima/shared/core/rect.h"
#include "graphics/screen.h"
#include "ultima/shared/gfx/font.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Shared {

namespace Gfx {

#define VGA_COLOR_TRANS(x) ((x) * 255 / 63)


/**
 * Base class for an on-screen cursor. Currently used for text cursor display
 */
class Cursor {
public:
	/**
	 * Destructor
	 */
	virtual ~Cursor() {}

	/**
	 * Get the bounds of the cursor
	 */
	virtual Common::Rect getBounds() const = 0;

	/**
	 * Draw the cursor
	 */
	virtual void draw() = 0;
};

class Screen: public Graphics::Screen {
private:
	Cursor *_cursor;
	bool _drawCursor;
public:
	/**
	 * Constructor
	 */
	Screen();

	/**
	 * Updates the screen by copying any affected areas to the system
	 */
	void update() override;

	/**
	 * Updates the screen at the end of an update call
	 */
	void updateScreen() override;

	/**
	 * Sets the currently active cursor
	 */
	void setCursor(Cursor *cursor) { _cursor = cursor; }
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
