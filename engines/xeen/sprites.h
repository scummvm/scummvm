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
	SPRFLAG_MODE_MASK = 0xF00, SPRFLAG_DRAWER1 = 0x100, SPRFLAG_DRAWER2 = 0x200,
	SPRFLAG_DRAWER3 = 0x300, SPRFLAG_DRAWER4 = 0x400, SPRFLAG_DRAWER5 = 0x500, SPRFLAG_DRAWER6 = 0x600,
	SPRFLAG_DRAWER7 = 0x700, SPRFLAG_800 = 0x800, SPRFLAG_SCENE_CLIPPED = 0x2000,
	SPRFLAG_BOTTOM_CLIPPED = 0x4000, SPRFLAG_HORIZ_FLIPPED = 0x8000, SPRFLAG_RESIZE = 0x10000
};

class SpriteResource {
private:
	struct IndexEntry {
		uint16 _offset1, _offset2;
	};
	Common::Array<IndexEntry> _index;
	size_t _filesize;
	byte *_data;
	int _scaledWidth, _scaledHeight;
	Common::String _filename;
	static int _clippedBottom;

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

	/**
	 * Set the bottom Y position where sprites are clipped if SPRFLAG_BOTTOM_CLIPPED
	 * is applied
	 */
	static void setClippedBottom(int y) { _clippedBottom = y; }
};

/**
 * Basic sprite drawer
 */
class SpriteDrawer {
private:
	byte *_data;
	size_t _filesize;
private:
	/**
	 * Scale a co-ordinate value based on the passed scaling mask
	 */
	static uint getScaledVal(int xy, uint16 &scaleMask);
protected:
	/**
	 * Output a pixel
	 */
	virtual void drawPixel(byte *dest, byte pixel);
public:
	/**
	 * Constructor
	 */
	SpriteDrawer(byte *data, size_t filesize) : _data(data), _filesize(filesize) {}

	/**
	 * Destructor
	 */
	virtual ~SpriteDrawer() {}

	/**
	 * Draw a sprite frame based on a passed offset into the data stream
	 */
	void draw(XSurface &dest, uint16 offset, const Common::Point &pt,
		const Common::Rect &clipRect, uint flags, int scale);
};

class SpriteDrawer1 : public SpriteDrawer {
private:
	byte _offset, _mask;
protected:
	/**
	 * Output a pixel
	 */
	virtual void drawPixel(byte *dest, byte pixel) override;
public:
	/**
	 * Constructor
	 */
	SpriteDrawer1(byte *data, size_t filesize, int index);
};

/**
 * Draws the sprite as faint ghostly, see-through.
 */
class SpriteDrawer3 : public SpriteDrawer {
private:
	uint16 _offset, _mask;
	byte _palette[256 * 3];
	bool _hasPalette;
private:
	/**
	 * Output a pixel
	 */
	virtual void drawPixel(byte *dest, byte pixel) override;
public:
	/**
	 * Constructor
	 */
	SpriteDrawer3(byte *data, size_t filesize, int index);
};

class SpriteDrawer4 : public SpriteDrawer {
private:
	byte _threshold;
protected:
	/**
	 * Output a pixel
	 */
	virtual void drawPixel(byte *dest, byte pixel) override;
public:
	/**
	 * Constructor
	 */
	SpriteDrawer4(byte *data, size_t filesize, int index);
};

/**
 * Draws a sprite with a fuzziness effect where only some pixels of the sprite are randomly drawn
 */
class SpriteDrawer5 : public SpriteDrawer {
private:
	uint16 _threshold, _random1, _random2;
private:
	/**
	 * Roll carry right opcode emulation
	 */
	void rcr(uint16 &val, bool &cf);
protected:
	/**
	 * Output a pixel
	 */
	virtual void drawPixel(byte *dest, byte pixel) override;
public:
	/**
	 * Constructor
	 */
	SpriteDrawer5(byte *data, size_t filesize, int index);
};

class SpriteDrawer6 : public SpriteDrawer {
private:
	byte _mask;
protected:
	/**
	 * Output a pixel
	 */
	virtual void drawPixel(byte *dest, byte pixel) override;
public:
	/**
	 * Constructor
	 */
	SpriteDrawer6(byte *data, size_t filesize, int index);
};

} // End of namespace Xeen

#endif /* MADS_SPRITES_H */
