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
#include "sci/graphics/helpers.h"

namespace Sci {

class GfxScreen;

enum ColorRemappingType {
	kRemappingNone = 0,
	kRemappingByRange = 1,
	kRemappingByPercent = 2,
	kRemappingToGray = 3,
	kRemappingToPercentGray = 4
};

#define REMAP_COLOR_COUNT 9
#define NON_REMAPPED_COLOR_COUNT 236

/**
 * Remap class, handles color remapping
 */
class GfxRemap {
public:
	GfxRemap(GfxPalette *_palette);
	~GfxRemap();

	void resetRemapping();
	void setRemappingPercent(byte color, byte percent);
	void setRemappingRange(byte color, byte from, byte to, byte base);
	bool isRemapped(byte color) const {
		return _remapOn && (_remappingType[color] != kRemappingNone);
	}
	byte remapColor(byte remappedColor, byte screenColor);
	void updateRemapping();

private:
	GfxScreen *_screen;
	GfxPalette *_palette;

	bool _remapOn;
	ColorRemappingType _remappingType[256];
	byte _remappingByPercent[256];
	byte _remappingByRange[256];
	uint16 _remappingPercentToSet;
};

#ifdef ENABLE_SCI32

struct RemapParams {
	byte from;
	byte to;
	byte base;
	byte gray;
	byte oldGray;
	byte percent;
	byte oldPercent;
	ColorRemappingType type;
	Color curColor[256];
	Color targetColor[256];
	byte distance[256];
	byte remap[256];
	bool colorChanged[256];

	RemapParams() {
		from = to = base = gray = oldGray = percent = oldPercent = 0;
		type = kRemappingNone;

		// curColor and targetColor are initialized in GfxRemap32::initColorArrays
		memset(curColor, 0, 256 * sizeof(Color));
		memset(targetColor, 0, 256 * sizeof(Color));
		memset(distance, 0, 256);
		for (int i = 0; i < NON_REMAPPED_COLOR_COUNT; i++)
			remap[i] = i;
		Common::fill(colorChanged, colorChanged + ARRAYSIZE(colorChanged), true);
	}

	RemapParams(byte from_, byte to_, byte base_, byte gray_, byte percent_, ColorRemappingType type_) {
		from = from_;
		to = to_;
		base = base_;
		gray = oldGray = gray_;
		percent = oldPercent = percent_;
		type = type_;

		// curColor and targetColor are initialized in GfxRemap32::initColorArrays
		memset(curColor, 0, 256 * sizeof(Color));
		memset(targetColor, 0, 256 * sizeof(Color));
		memset(distance, 0, 256);
		for (int i = 0; i < NON_REMAPPED_COLOR_COUNT; i++)
			remap[i] = i;
		Common::fill(colorChanged, colorChanged + ARRAYSIZE(colorChanged), true);
	}
};

class GfxRemap32 {
public:
	GfxRemap32(GfxPalette32 *palette);
	~GfxRemap32() {}

	void remapOff(byte color);
	void setRemappingRange(byte color, byte from, byte to, byte base);
	void setRemappingPercent(byte color, byte percent);
	void setRemappingToGray(byte color, byte gray);
	void setRemappingToPercentGray(byte color, byte gray, byte percent);
	void setNoMatchRange(byte from, byte count);
	bool remapEnabled(byte color) const;
	byte remapColor(byte color, byte target);
	bool remapAllTables(bool palChanged);
	int getRemapCount() const { return _remapCount; }
	int getStartColor() const { return _remapEndColor - REMAP_COLOR_COUNT + 1; }
	int getEndColor() const { return _remapEndColor; }
private:
	GfxPalette32 *_palette;
	RemapParams _remaps[REMAP_COLOR_COUNT];
	bool _update;
	byte _noMapStart, _noMapCount;
	bool _targetChanged[NON_REMAPPED_COLOR_COUNT];
	byte _remapEndColor;
	int _remapCount;

	void initColorArrays(byte index);
	bool applyRemap(byte index);
	bool updateRemap(byte index, bool palChanged);
};
#endif

} // End of namespace Sci

#endif
