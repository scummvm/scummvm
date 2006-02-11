/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_palette_h__
#define __lure_palette_h__

#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/memory.h"

namespace Lure {

enum PaletteSource {RGB, RGB64};

class Palette {
private:
	MemoryBlock *_palette;
	uint16 _numEntries;

	void convertPalette(const byte *palette, uint16 numEntries);
public:
	Palette();
	Palette(uint8 numEntries, const byte *data, PaletteSource paletteSource);
	Palette(Palette &src);
	Palette(uint16 resourceId);

	uint8 *data() { return _palette->data(); }
	MemoryBlock *palette() { return _palette; }
	uint16 numEntries() { return _palette->size() / 4; }
	void setEntry(uint8 index, uint32 value);
	uint32 getEntry(uint8 index);
	void copyFrom(Palette *src); 
};

class PaletteCollection {
private:
	Palette **_palettes;
	uint8 _numPalettes;
public:
	PaletteCollection(uint16 resourceId);
	~PaletteCollection();

	uint8 numPalettes() { return _numPalettes; }
	Palette &getPalette(uint8 paletteNum);
};

} // end of namspace Lure

#endif
