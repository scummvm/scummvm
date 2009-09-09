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

	/** @name Color data */
	/** @{ */
	byte r, g, b;
	/** @} */

	/** Index in parent palette, or -1 */
	int parent_index;

	/**
	 * Number of references from child palettes. (This includes palettes
	 * of pixmaps.)
	 * Special values: PALENTRY_LOCKED, PALENTRY_FREE */
	int refcount;
};

class Palette {
public:
	explicit Palette(uint size);
	Palette(const PaletteEntry *colors, uint size);
	~Palette();

	Palette *getref();
	void free();
	Palette *copy();

	void resize(uint size);
	void setColor(uint index, byte r, byte g, byte b, int parentIndex = -1);
	void makeSystemColor(uint index, const PaletteEntry &color);
	const PaletteEntry &getColor(uint index) const {
		assert(index < _size);
		return _colors[index];
	}
	const PaletteEntry &operator[](uint index) const {
		return getColor(index);
	}
	uint size() const { return _size; }
	bool isDirty() const { return _dirty; }
	bool isShared() const { return _refcount > 1; }
	Palette *getParent() { return _parent; }
	int getRevision() const { return _revision; }

	void markClean() { _dirty = false; }

	uint findNearbyColor(byte r, byte g, byte b, bool lock=false);

	bool mergeInto(Palette *parent); // returns false if already merged
	void forceInto(Palette *parent);

	void unmerge();

	Common::String name; // strictly for debugging purposes
private:
	PaletteEntry *_colors;
	uint _size;

	Palette *_parent;

	bool _dirty; /**< Palette has changed */
	int _refcount; /**< Number of pixmaps (or other objects) using this palette */
	int _revision; /**< When this is incremented, all child references are invalidated */
};


} // End of namespace Sci

#endif // SCI_GFX_PALETTE_H
