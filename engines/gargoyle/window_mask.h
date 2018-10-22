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

#ifndef GARGOYLE_WINDOW_MASK_H
#define GARGOYLE_WINDOW_MASK_H

#include "common/rect.h"
#include "gargoyle/glk_types.h"

namespace Gargoyle {

class Window;

struct WindowMask {
	size_t _hor, _ver;
	glui32 **_links;
	Common::Rect _select;

	static int _lastX, _lastY;

	/**
	 * Constructor
	 */
	WindowMask();

	/**
	 * Resize the links array
	 */
	void resize(size_t x, size_t y);

	void putHyperlink(glui32 linkval, uint x0, uint y0, uint x1, uint y1);

	glui32 getHyperlink(const Common::Point &pos);

	void startSelection(const Common::Point &pos);

	void moveSelection(const Common::Point &pos);

	void clearSelection();

	int checkSelection(uint x0, uint y0, uint x1, uint y1);

	int getSelection(uint x0, uint y0, uint x1, uint y1, uint *rx0, uint *rx1);
};

} // End of namespace Gargoyle

#endif
