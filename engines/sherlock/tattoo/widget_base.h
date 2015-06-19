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

#ifndef SHERLOCK_TATTOO_WIDGET_BASE_H
#define SHERLOCK_TATTOO_WIDGET_BASE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "sherlock/surface.h"

namespace Sherlock {

class SherlockEngine;
class ImageFile;

namespace Tattoo {

class WidgetBase {
protected:
	SherlockEngine *_vm;
	Common::Rect _bounds, _oldBounds;
	Surface _surface;
	ImageFile *_images;

	/**
	 * Used by descendent classes to split up long text for display across multiple lines
	 */
	Common::String splitLines(const Common::String &str, Common::StringArray &lines, int maxWidth, uint maxLines);

	/**
	 * Ensure that menu is drawn entirely on-screen
	 */
	void checkMenuPosition();

	void makeInfoArea();
public:
	WidgetBase(SherlockEngine *vm);
	virtual ~WidgetBase() {}

	/**
	 * Summon the window
	 */
	virtual void summonWindow();

	/**
	 * Close a currently active menu
	 */
	virtual void banishWindow();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
