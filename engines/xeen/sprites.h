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

enum {
	SCALE_MASK = 0x7FFF, SCALE_ENLARGE = 0x8000
};

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
	int _scaledWidth, _scaledHeight;

	/**
	 * Load a sprite resource from a stream
	 */
	void load(Common::SeekableReadStream &f);

	/**
	 * Draw the sprite onto the given surface
	 */
	void draw(XSurface &dest, int frame, const Common::Point &destPos,
		const Common::Rect &bounds, uint flags = 0, int scale = 0);

	/**
	 * Draw the sprite onto a given window
	 */
	void draw(int windowNum, int frame, const Common::Point &destPos,
		const Common::Rect &bounds, uint flags = 0, int scale = 0);

	/**
	 * Draw a sprite frame based on a passed offset into the data stream
	 */
	void drawOffset(XSurface &dest, uint16 offset, const Common::Point &pt,
		const Common::Rect &clipRect, uint flags, int scale);

	/**
	 * Scale a co-ordinate value based on the passed scaling mask
	 */
	static uint getScaledVal(int xy, uint16 &scaleMask);
public:
	SpriteResource();
	SpriteResource(const Common::String &filename);
	SpriteResource(const Common::String &filename, int ccMode);

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
	void load(const Common::String &filename, int ccMode);

	/**
	 * Clears the sprite resource
	 */
	void clear();

	/**
	 * Draw a sprite onto a surface
	 * @param dest		Destination surface
	 * @param frame		Frame number
	 * @param destPos	Destination position
	 * @param flags		Flags
	 * @param scale		Scale: 0=No scale, SCALE_ENLARGE=Enlarge it
	 *					1..15   -> reduces the sprite: the higher, the smaller it'll be
	 */
	void draw(XSurface &dest, int frame, const Common::Point &destPos,
		uint flags = 0, int scale = 0);

	/**
	 * Draw a sprite onto a specific window
	 * @param dest		Destination window
	 * @param frame		Frame number
	 * @param destPos	Destination position
	 * @param flags		Flags
	 * @param scale		Scale: 0=No scale, SCALE_ENLARGE=Enlarge it
	 *					1..15   -> reduces the sprite: the higher, the smaller it'll be
	 */
	void draw(Window &dest, int frame, const Common::Point &destPos,
		uint flags = 0, int scale = 0);

	/**
	 * Draw a sprite onto a given window
	 * @param windowIndex	Destination window number
	 * @param frame		Frame number
	 * @param destPos	Destination position
	 * @param flags		Flags
	 * @param scale		Scale: 0=No scale, SCALE_ENLARGE=Enlarge it
	 *					1..15   -> reduces the sprite: the higher, the smaller it'll be
	 */
	void draw(int windowIndex, int frame, const Common::Point &destPos,
		uint flags = 0, int scale = 0);

	/**
	 * Draw the sprite onto the given surface
	 * @param dest		Destination surface
	 * @param frame		Frame number
	 */
	void draw(XSurface &dest, int frame);

	/**
	 * Draw the sprite onto the given window
	 * @param windowIndex	Destination window number
	 * @param frame			Frame number
	 */
	void draw(int windowIndex, int frame);

	/**
	 * Gets the size of a sprite
	 */
	Common::Point getFrameSize(int frame) const;

	/**
	 * Returns the number of frames the sprite resource has
	 */
	size_t size() const { return _index.size(); }

	/**
	 * Returns true if the sprite resource is empty (ie. nothing is loaded)
	 */
	bool empty() const { return _index.size() == 0; }
};

} // End of namespace Xeen

#endif /* MADS_SPRITES_H */
