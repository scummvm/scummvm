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

#ifndef XEEN_SPRITES_H
#define XEEN_SPRITES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "graphics/surface.h"
#include "xeen/files.h"
#include "xeen/xsurface.h"

namespace Xeen {

class XeenEngine;
class Window;

enum SpriteFlags {
	SPRFLAG_800 = 0x800, SPRFLAG_SCENE_CLIPPED = 0x2000,
	SPRFLAG_4000 = 0x4000, SPRFLAG_HORIZ_FLIPPED = 0x8000,
	SPRFLAG_RESIZE = 0x10000
};

class SpriteResource {
private:
	struct IndexEntry {
		uint16 _offset1, _offset2;
	};
	Common::Array<IndexEntry> _index;
	int32 _filesize;
	byte *_data;
	bool _lineDist[320];
	int _scaledWidth, _scaledHeight;

	/**
	 * Load a sprite resource from a stream
	 */
	void load(Common::SeekableReadStream &f);

	/**
	 * Draw the sprite onto the given surface
	 */
	void draw(XSurface &dest, int frame, const Common::Point &destPos,
		const Common::Rect &bounds, int flags = 0, int scale = 0);

	/**
	 * Draw a sprite frame based on a passed offset into the data stream
	 */
	void drawOffset(XSurface &dest, uint16 offset, const Common::Point &pt,
		const Common::Rect &clipRect, int flags, int scale);

	/**
	 * Scale a co-ordinate value based on the passed scaling mask
	 */
	static uint getScaledVal(int xy, uint16 &scaleMask);
public:
	SpriteResource();
	SpriteResource(const Common::String &filename);

	virtual ~SpriteResource();

	/**
	 * Copy operator for duplicating a sprite resource
	 */
	SpriteResource &operator=(const SpriteResource &src);

	/**
	 * Load a sprite resource from a given file
	 */
	void load(const Common::String &filename);

	/**
	 * Load a sprite resource from a given file and archive
	 */
	void load(const Common::String &filename, ArchiveType archiveType);

	/**
	 * Clears the sprite resource
	 */
	void clear();

	void draw(XSurface &dest, int frame, const Common::Point &destPos,
		int flags = 0, int scale = 0);

	void draw(Window &dest, int frame, const Common::Point &destPos,
		int flags = 0, int scale = 0);

	/**
	 * Draw the sprite onto the given surface
	 */
	void draw(XSurface &dest, int frame);

	int size() const { return _index.size(); }

	bool empty() const { return _index.size() == 0; }
};

} // End of namespace Xeen

#endif /* MADS_SPRITES_H */
