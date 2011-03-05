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

#ifndef GRAPHICS_WINCURSOR_H
#define GRAPHICS_WINCURSOR_H

#include "common/array.h"
#include "common/winexe.h"

namespace Common {
	class NEResources;
	class PEResources;
	class SeekableReadStream;
}

namespace Graphics {

/** A Windows cursor. */
class WinCursor {
public:
	WinCursor();
	~WinCursor();

	/** Return the cursor's width. */
	uint16 getWidth() const;
	/** Return the cursor's height. */
	uint16 getHeight() const;
	/** Return the cursor's hotspot's x coordinate. */
	uint16 getHotspotX() const;
	/** Return the cursor's hotspot's y coordinate. */
	uint16 getHotspotY() const;
	/** Return the cursor's transparent key. */
	byte getKeyColor() const;

	const byte *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }

	/** Read the cursor's data out of a stream. */
	bool readFromStream(Common::SeekableReadStream &stream);

private:
	byte *_surface;
	byte _palette[256 * 3];

	uint16 _width;    ///< The cursor's width.
	uint16 _height;   ///< The cursor's height.
	uint16 _hotspotX; ///< The cursor's hotspot's x coordinate.
	uint16 _hotspotY; ///< The cursor's hotspot's y coordinate.
	byte   _keyColor; ///< The cursor's transparent key

	/** Clear the cursor. */
	void clear();
};

struct WinCursorGroup {
	WinCursorGroup();
	~WinCursorGroup();

	struct CursorItem {
		Common::WinResourceID id;
		WinCursor *cursor;
	};

	Common::Array<CursorItem> cursors;

	static WinCursorGroup *createCursorGroup(Common::NEResources &exe, const Common::WinResourceID &id);
	//static WinCursorGroup *createCursorGroup(Common::PEResources &exe, const Common::WinResourceID &id);
};

} // End of namespace Graphics

#endif
