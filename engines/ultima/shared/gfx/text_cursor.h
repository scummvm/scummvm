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

#ifndef ULTIMA_SHARED_GFX_TEXT_CURSOR_H
#define ULTIMA_SHARED_GFX_TEXT_CURSOR_H

#include "ultima/shared/core/rect.h"
#include "ultima/shared/gfx/screen.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

/**
 * Base class for text cursors, and is used by those games that don't have a visible cursor
 */
class TextCursor : public Cursor {
protected:
	bool _visible;
	Common::Rect _bounds;
protected:
	/**
	 * Marks the area of the screen the cursor covers as dirty
	 */
	void markAsDirty();
public:
	TextCursor() : _visible(false) {
	}
	~TextCursor() override {
	}

	/**
	 * Returns true if the cursor is visible
	 */
	bool isVisible() const {
		return _visible;
	}

	/**
	 * Sets the visibility of the cursor
	 */
	virtual void setVisible(bool isVis) {
		_visible = isVis;
		markAsDirty();
	}

	/**
	 * Get the bounds of the cursor
	 */
	Common::Rect getBounds() const override { return _bounds; }

	/**
	 * Returns the position of the cursor
	 */
	Point getPosition() const { return Point(_bounds.left, _bounds.top); }

	/**
	 * Sets the position of the cursor
	 */
	void setPosition(const Point &pt) {
		bool vis = _visible;
		setVisible(false);
		_bounds.moveTo(pt);
		setVisible(vis);
	}

	/**
	 * Update the cursor
	 */
	virtual void update() = 0;
};

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima

#endif
