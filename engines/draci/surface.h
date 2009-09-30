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
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_SURFACE_H
#define DRACI_SURFACE_H

#include "graphics/surface.h"

namespace Draci {

class Surface : public Graphics::Surface {

public:
	Surface(int width, int height);
	~Surface();

	void markDirtyRect(Common::Rect r);
	const Common::List<Common::Rect> *getDirtyRects() const;
	void clearDirtyRects();
	void markDirty();
	void markClean();
	bool needsFullUpdate() const;
	uint getTransparentColour() const;
	void setTransparentColour(uint colour);
	void fill(uint colour);
	uint centerOnY(uint y, uint height) const;
	uint centerOnX(uint x, uint width) const;
	Common::Rect getRect() const;

private:
	/** The current transparent colour of the surface. See getTransparentColour() and
	 *  setTransparentColour().
	 */
	uint _transparentColour;

	/** Set when the surface is scheduled for a full update.
	 *  See markDirty(), markClean(). Accessed via needsFullUpdate().
	 */
	bool _fullUpdate;

	Common::List<Common::Rect> _dirtyRects; //!< List of currently dirty rectangles

};

} // End of namespace Draci

#endif // DRACI_SURFACE_H
