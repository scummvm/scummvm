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

#include "lure/palette.h"
#include "common/util.h"

namespace Lure {

// Constructor
// Defaults the palette to a full 256 entry palette

Palette::Palette() {
	_numEntries = GAME_COLOURS;
	_palette = Memory::allocate(_numEntries * 4);
	_palette->empty();
}

// Consructor
// Sets up a palette with the given number of entries and a copy of the passed data

Palette::Palette(uint8 numEntries1, const byte *data1, PaletteSource paletteSource) {
	_numEntries = numEntries1;
	_palette = Memory::allocate(_numEntries * 4);

	if (data1) {
		if (paletteSource == RGB64) 
			convertPalette(data1, _numEntries);
		else
			_palette->copyFrom(data1, 0, 0, _numEntries * 4);
	} else {
		// No data provided, set a null palette
		_palette->empty();
	}
}

// Constructor
// Makes a copy of a passed palette object

Palette::Palette(Palette &src) {
	_numEntries = src.numEntries();
	_palette = Memory::duplicate(src._palette);
}

// Constructor
// Loads a palette from a resource

Palette::Palette(uint16 resourceId) {
	Disk &d = Disk::getReference();

	MemoryBlock *srcData = d.getEntry(resourceId); 
	if (((srcData->size() % 3) != 0) || ((srcData->size() / 3) > GAME_COLOURS))
		error("Specified resource %d is not a palette", resourceId);

	_numEntries = srcData->size() / 3;
	_palette = Memory::allocate(_numEntries * 4);
	convertPalette(srcData->data(), _numEntries);
	delete srcData;
}

void Palette::convertPalette(const byte *palette1, uint16 numEntries1) {
	byte *pDest = _palette->data();
	const byte *pSrc = palette1;

	while (numEntries1-- > 0) {
		*pDest++ = (pSrc[0] << 2) + (pSrc[0] >> 4);
		*pDest++ = (pSrc[1] << 2) + (pSrc[1] >> 4);
		*pDest++ = (pSrc[2] << 2) + (pSrc[2] >> 4);
		*pDest++ = 0;
		pSrc += 3;
	}
}

void Palette::setEntry(uint8 index, uint32 value) {
	if (index >= numEntries()) error("Invalid palette index: %d", index);
	uint32 *entry = (uint32 *) (data() + index * 4);
	*entry = value;
}

uint32 Palette::getEntry(uint8 index) {
	if (index >= numEntries()) error("Invalid palette index: %d", index);
	uint32 *entry = (uint32 *) (data() + index * 4);
	return *entry;
}

void Palette::copyFrom(Palette *src) { 
	_palette->copyFrom(src->palette());
}

/*--------------------------------------------------------------------------*/

PaletteCollection::PaletteCollection(uint16 resourceId) {
	Disk &d = Disk::getReference();
	MemoryBlock *resource = d.getEntry(resourceId);
	uint32 palSize;
	uint8 *data = resource->data();

	if (resource->size() % (SUB_PALETTE_SIZE * 3) != 0)
		error("Resource #%d is not a valid palette set", resourceId);

	palSize = SUB_PALETTE_SIZE * 3;
	_numPalettes = resource->size() / palSize;

	_palettes = (Palette **) Memory::alloc(_numPalettes * sizeof(Palette *));
	for (uint8 paletteCtr = 0; paletteCtr < _numPalettes; ++paletteCtr, data += palSize)
		_palettes[paletteCtr] = new Palette(SUB_PALETTE_SIZE, data, RGB64);

	delete resource;
}

PaletteCollection::~PaletteCollection() {
	for (int paletteCtr = 0; paletteCtr < _numPalettes; ++paletteCtr)
		delete _palettes[paletteCtr];
	free(_palettes);
}


Palette &PaletteCollection::getPalette(uint8 paletteNum) {
	if (paletteNum >= _numPalettes)
		error("Invalid palette index specified");
	return *_palettes[paletteNum];
}

} // end of namespace Lure
