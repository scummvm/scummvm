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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_GRAPHICS_REMAP_H
#define SCI_GRAPHICS_REMAP_H

#include "common/array.h"
#include "common/serializer.h"

namespace Sci {

class GfxScreen;

/**
 * This class handles color remapping for the QFG4 demo.
 */
class GfxRemap {
private:
	enum ColorRemappingType {
		kRemapNone = 0,
		kRemapByRange = 1,
		kRemapByPercent = 2
	};

public:
	GfxRemap(GfxPalette *_palette);

	void resetRemapping();
	void setRemappingPercent(byte color, byte percent);
	void setRemappingRange(byte color, byte from, byte to, byte base);
	bool isRemapped(byte color) const {
		return _remapOn && (_remappingType[color] != kRemapNone);
	}
	byte remapColor(byte remappedColor, byte screenColor);
	void updateRemapping();

private:
	GfxPalette *_palette;

	bool _remapOn;
	ColorRemappingType _remappingType[256];
	byte _remappingByPercent[256];
	byte _remappingByRange[256];
	uint16 _remappingPercentToSet;
};
} // End of namespace Sci

#endif
