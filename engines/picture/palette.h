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

#ifndef PICTURE_PALETTE_H
#define PICTURE_PALETTE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

//#define ROT(index) (((index << 4) & 0xF0) | ((index >> 4) & 0x0F))
//#define ROT(index) (index)

class Palette {
public:
	Palette(PictureEngine *vm);
	~Palette();

	void setFullPalette(byte *palette);
	void setDeltaPalette(byte *palette, byte mask, char deltaValue, int16 count, int16 startIndex);

	void loadAddPalette(uint resIndex, byte startIndex);
	void loadAddPaletteFrom(byte *source, byte startIndex, byte count);

	void addFragment(uint resIndex, int16 id);
	uint16 findFragment(int16 id);
	void clearFragments();

	byte *getMainPalette() { return _mainPalette; }

protected:

	struct PaletteFragment {
		int16 id;
		byte index, count;
	};
	
	typedef Common::Array<PaletteFragment> PaletteFragmentArray;

	PictureEngine *_vm;

	byte _mainPalette[768];

	PaletteFragmentArray _fragments;
	byte _fragmentIndex;

};

} // End of namespace Picture

#endif /* PICTURE_PALETTE_H */
