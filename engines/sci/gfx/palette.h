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

#ifndef SCI_GFX_PALETTE_H
#define SCI_GFX_PALETTE_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Sci {

const int PALENTRY_LOCKED = -42;
const int PALENTRY_FREE = -41;

struct PaletteEntry {
	PaletteEntry()
		: r(0), g(0), b(0), parent_index(-1), refcount(PALENTRY_FREE)
	{ }
	PaletteEntry(byte R, byte G, byte B)
		: r(R), g(G), b(B), parent_index(-1), refcount(PALENTRY_FREE)
	{ }

	// Color data
	byte r, g, b;

	// Index in parent palette, or -1
	int parent_index;

	// Number of references from child palettes. (This includes palettes
	// of pixmaps.)
	// Special values: PALENTRY_LOCKED, PALENTRY_FREE
	int refcount;
};

struct gfx_pixmap_color_t;

class Palette {
public:
	explicit Palette(unsigned int size);
	Palette(gfx_pixmap_color_t* colors, unsigned int size);
	~Palette();

	Palette* getref();
	void free();
	Palette* copy();

	void resize(unsigned int size);
	void setColor(unsigned int index, byte r, byte g, byte b);
	void makeSystemColor(unsigned int index, const PaletteEntry& color);
	const PaletteEntry& getColor(unsigned int index) const {
		assert(index < _size);
		return _colors[index];
	}
	const PaletteEntry& operator[](unsigned int index) const {
		return getColor(index);
	}
	unsigned int size() const { return _size; }
	bool isDirty() const { return _dirty; }
	bool isShared() const { return _refcount > 1; }
	Palette* getParent() { return _parent; }

	void markClean() { _dirty = false; }

	unsigned int findNearbyColor(byte r, byte g, byte b, bool lock=false);

	void mergeInto(Palette *parent);
	void unmerge();

	Common::String name; // strictly for debugging purposes
private:
	PaletteEntry *_colors;
	unsigned int _size;

	Palette *_parent;

	bool _dirty; // Palette has changed
	int _refcount; // Number of pixmaps (or other objects) using this palette
};




} // End of namespace Sci

#endif // SCI_GFX_PALETTE_H
