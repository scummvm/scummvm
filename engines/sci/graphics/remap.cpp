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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/remap.h"
#include "sci/graphics/screen.h"

namespace Sci {

#pragma mark -
#pragma mark SCI16 remapping (QFG4 demo)

GfxRemap::GfxRemap(GfxPalette *palette)
	: _palette(palette) {
	_remapOn = false;
	resetRemapping();
}

GfxRemap::~GfxRemap() {
}

byte GfxRemap::remapColor(byte remappedColor, byte screenColor) {
	assert(_remapOn);
	if (_remappingType[remappedColor] == kRemappingByRange)
		return _remappingByRange[screenColor];
	else if (_remappingType[remappedColor] == kRemappingByPercent)
		return _remappingByPercent[screenColor];
	else
		error("remapColor(): Color %d isn't remapped", remappedColor);

	return 0;	// should never reach here
}

void GfxRemap::resetRemapping() {
	_remapOn = false;
	_remappingPercentToSet = 0;

	for (int i = 0; i < 256; i++) {
		_remappingType[i] = kRemappingNone;
		_remappingByPercent[i] = i;
		_remappingByRange[i] = i;
	}
}

void GfxRemap::setRemappingPercent(byte color, byte percent) {
	_remapOn = true;

	// We need to defer the setup of the remapping table every time the screen
	// palette is changed, so that kernelFindColor() can find the correct
	// colors. Set it once here, in case the palette stays the same and update
	// it on each palette change by copySysPaletteToScreen().
	_remappingPercentToSet = percent;

	for (int i = 0; i < 256; i++) {
		byte r = _palette->_sysPalette.colors[i].r * _remappingPercentToSet / 100;
		byte g = _palette->_sysPalette.colors[i].g * _remappingPercentToSet / 100;
		byte b = _palette->_sysPalette.colors[i].b * _remappingPercentToSet / 100;
		_remappingByPercent[i] = _palette->kernelFindColor(r, g, b);
	}

	_remappingType[color] = kRemappingByPercent;
}

void GfxRemap::setRemappingRange(byte color, byte from, byte to, byte base) {
	_remapOn = true;

	for (int i = from; i <= to; i++) {
		_remappingByRange[i] = i + base;
	}

	_remappingType[color] = kRemappingByRange;
}

void GfxRemap::updateRemapping() {
	// Check if we need to reset remapping by percent with the new colors.
	if (_remappingPercentToSet) {
		for (int i = 0; i < 256; i++) {
			byte r = _palette->_sysPalette.colors[i].r * _remappingPercentToSet / 100;
			byte g = _palette->_sysPalette.colors[i].g * _remappingPercentToSet / 100;
			byte b = _palette->_sysPalette.colors[i].b * _remappingPercentToSet / 100;
			_remappingByPercent[i] = _palette->kernelFindColor(r, g, b);
		}
	}
}

#pragma mark -
#pragma mark SCI32 remapping

#ifdef ENABLE_SCI32

GfxRemap32::GfxRemap32(GfxPalette32 *palette) : _palette(palette) {
	for (int i = 0; i < REMAP_COLOR_COUNT; i++)
		_remaps[i] = RemapParams(0, 0, 0, 0, 100, kRemappingNone);
	_noMapStart = _noMapCount = 0;
	_update = false;
	_remapCount = 0;

	// The remap range was 245 - 254 in SCI2, but was changed to 235 - 244 in SCI21 middle
	_remapEndColor = (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) ? 244 : 254;
}

void GfxRemap32::remapOff(byte color) {
	if (!color) {
		for (int i = 0; i < REMAP_COLOR_COUNT; i++)
			_remaps[i] = RemapParams(0, 0, 0, 0, 100, kRemappingNone);

		_remapCount = 0;
	} else {
		assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
		const byte index = _remapEndColor - color;
		_remaps[index] = RemapParams(0, 0, 0, 0, 100, kRemappingNone);
		_remapCount--;
	}

	_update = true;
}

void GfxRemap32::setRemappingRange(byte color, byte from, byte to, byte base) {
	assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
	_remaps[_remapEndColor - color] = RemapParams(from, to, base, 0, 100, kRemappingByRange);
	initColorArrays(_remapEndColor - color);
	_remapCount++;
	_update = true;
}

void GfxRemap32::setRemappingPercent(byte color, byte percent) {
	assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
	_remaps[_remapEndColor - color] = RemapParams(0, 0, 0, 0, percent, kRemappingByPercent);
	initColorArrays(_remapEndColor - color);
	_remapCount++;
	_update = true;
}

void GfxRemap32::setRemappingToGray(byte color, byte gray) {
	assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
	_remaps[_remapEndColor - color] = RemapParams(0, 0, 0, gray, 100, kRemappingToGray);
	initColorArrays(_remapEndColor - color);
	_remapCount++;
	_update = true;
}

void GfxRemap32::setRemappingToPercentGray(byte color, byte gray, byte percent) {
	assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
	_remaps[_remapEndColor - color] = RemapParams(0, 0, 0, gray, percent, kRemappingToPercentGray);
	initColorArrays(_remapEndColor - color);
	_remapCount++;
	_update = true;
}

void GfxRemap32::setNoMatchRange(byte from, byte count) {
	_noMapStart = from;
	_noMapCount = count;
}

bool GfxRemap32::remapEnabled(byte color) const {
	assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
	const byte index = _remapEndColor - color;
	return (_remaps[index].type != kRemappingNone);
}

byte GfxRemap32::remapColor(byte color, byte target) {
	assert(_remapEndColor - color >= 0 && _remapEndColor - color < REMAP_COLOR_COUNT);
	const byte index = _remapEndColor - color;
	if (_remaps[index].type != kRemappingNone)
		return _remaps[index].remap[target];
	else
		return target;
}

void GfxRemap32::initColorArrays(byte index) {
	Palette *curPalette = &_palette->_sysPalette;
	RemapParams *curRemap = &_remaps[index];

	memcpy(curRemap->curColor, curPalette->colors, NON_REMAPPED_COLOR_COUNT * sizeof(Color));
	memcpy(curRemap->targetColor, curPalette->colors, NON_REMAPPED_COLOR_COUNT * sizeof(Color));
}

bool GfxRemap32::updateRemap(byte index, bool palChanged) {
	int result;
	RemapParams *curRemap = &_remaps[index];
	const Palette *curPalette = &_palette->_sysPalette;
	const Palette *nextPalette = _palette->getNextPalette();
	bool changed = false;

	if (!_update && !palChanged)
		return false;

	Common::fill(_targetChanged, _targetChanged + NON_REMAPPED_COLOR_COUNT, false);

	switch (curRemap->type) {
	case kRemappingNone:
		return false;
	case kRemappingByRange:
		for (int i = 0; i < NON_REMAPPED_COLOR_COUNT; i++)  {
			if (curRemap->from <= i && i <= curRemap->to)
				result = i + curRemap->base;
			else
				result = i;

			if (curRemap->remap[i] != result) {
				changed = true;
				curRemap->remap[i] = result;
			}

			curRemap->colorChanged[i] = true;
		}
		return changed;
	case kRemappingByPercent:
		for (int i = 1; i < NON_REMAPPED_COLOR_COUNT; i++) {
			// NOTE: This method uses nextPalette instead of curPalette
			Color color = nextPalette->colors[i];

			if (curRemap->curColor[i] != color) {
				curRemap->colorChanged[i] = true;
				curRemap->curColor[i] = color;
			}

			if (curRemap->percent != curRemap->oldPercent || curRemap->colorChanged[i])  {
				byte red = CLIP<byte>(color.r * curRemap->percent / 100, 0, 255);
				byte green = CLIP<byte>(color.g * curRemap->percent / 100, 0, 255);
				byte blue = CLIP<byte>(color.b * curRemap->percent / 100, 0, 255);
				byte used = curRemap->targetColor[i].used;

				Color newColor = { used, red, green, blue };
				if (curRemap->targetColor[i] != newColor)  {
					_targetChanged[i] = true;
					curRemap->targetColor[i] = newColor;
				}
			}
		}
		
		changed = applyRemap(index);
		Common::fill(curRemap->colorChanged, curRemap->colorChanged + NON_REMAPPED_COLOR_COUNT, false);
		curRemap->oldPercent = curRemap->percent;
		return changed;
	case kRemappingToGray:
		for (int i = 1; i < NON_REMAPPED_COLOR_COUNT; i++) {
			Color color = curPalette->colors[i];

			if (curRemap->curColor[i] != color) {
				curRemap->colorChanged[i] = true;
				curRemap->curColor[i] = color;
			}

			if (curRemap->gray != curRemap->oldGray || curRemap->colorChanged[i])  {
				byte lumosity = ((color.r * 77) + (color.g * 151) + (color.b * 28)) >> 8;
				byte red = CLIP<byte>(color.r - ((color.r - lumosity) * curRemap->gray / 100), 0, 255);
				byte green = CLIP<byte>(color.g - ((color.g - lumosity) * curRemap->gray / 100), 0, 255);
				byte blue = CLIP<byte>(color.b - ((color.b - lumosity) * curRemap->gray / 100), 0, 255);
				byte used = curRemap->targetColor[i].used;

				Color newColor = { used, red, green, blue };
				if (curRemap->targetColor[i] != newColor)  {
					_targetChanged[i] = true;
					curRemap->targetColor[i] = newColor;
				}
			}
		}

		changed = applyRemap(index);
		Common::fill(curRemap->colorChanged, curRemap->colorChanged + NON_REMAPPED_COLOR_COUNT, false);
		curRemap->oldGray = curRemap->gray;
		return changed;
	case kRemappingToPercentGray:
		for (int i = 1; i < NON_REMAPPED_COLOR_COUNT; i++) {
			Color color = curPalette->colors[i];

			if (curRemap->curColor[i] != color) {
				curRemap->colorChanged[i] = true;
				curRemap->curColor[i] = color;
			}

			if (curRemap->percent != curRemap->oldPercent || curRemap->gray != curRemap->oldGray || curRemap->colorChanged[i])  {
				byte lumosity = ((color.r * 77) + (color.g * 151) + (color.b * 28)) >> 8;
				lumosity = lumosity * curRemap->percent / 100;
				byte red = CLIP<byte>(color.r - ((color.r - lumosity) * curRemap->gray / 100), 0, 255);
				byte green = CLIP<byte>(color.g - ((color.g - lumosity) * curRemap->gray / 100), 0, 255);
				byte blue = CLIP<byte>(color.b - ((color.b - lumosity) * curRemap->gray / 100), 0, 255);
				byte used = curRemap->targetColor[i].used;

				Color newColor = { used, red, green, blue };
				if (curRemap->targetColor[i] != newColor)  {
					_targetChanged[i] = true;
					curRemap->targetColor[i] = newColor;
				}
			}
		}

		changed = applyRemap(index);
		Common::fill(curRemap->colorChanged, curRemap->colorChanged + NON_REMAPPED_COLOR_COUNT, false);
		curRemap->oldPercent = curRemap->percent;
		curRemap->oldGray = curRemap->gray;
		return changed;
	default:
		return false;
	}
}

static int colorDistance(Color a, Color b) {
	int rDiff = (a.r - b.r) * (a.r - b.r);
	int gDiff = (a.g - b.g) * (a.g - b.g);
	int bDiff = (a.b - b.b) * (a.b - b.b);
	return rDiff + gDiff + bDiff;
}

bool GfxRemap32::applyRemap(byte index) {
	RemapParams *curRemap = &_remaps[index];
	const bool *cycleMap = _palette->getCyclemap();
	bool unmappedColors[NON_REMAPPED_COLOR_COUNT];
	Color newColors[NON_REMAPPED_COLOR_COUNT];
	bool changed = false;

	Common::fill(unmappedColors, unmappedColors + NON_REMAPPED_COLOR_COUNT, false);
	if (_noMapCount)
		Common::fill(unmappedColors + _noMapStart, unmappedColors + _noMapStart + _noMapCount, true);

	for (int i = 0; i < NON_REMAPPED_COLOR_COUNT; i++)  {
		if (cycleMap[i])
			unmappedColors[i] = true;
	}

	int curColor = 0;
	for (int i = 1; i < NON_REMAPPED_COLOR_COUNT; i++)  {
		if (curRemap->colorChanged[i] && !unmappedColors[i])
			newColors[curColor++] = curRemap->curColor[i];
	}

	for (int i = 1; i < NON_REMAPPED_COLOR_COUNT; i++)  {
		Color targetColor = curRemap->targetColor[i];
		bool colorChanged = curRemap->colorChanged[curRemap->remap[i]];

		if (!_targetChanged[i] && !colorChanged)
			continue;

		if (_targetChanged[i] && colorChanged)
			if (curRemap->distance[i] < 100 && colorDistance(targetColor, curRemap->curColor[curRemap->remap[i]]) <= curRemap->distance[i])
				continue;

		int diff = 0;
		int16 result = _palette->matchColor(targetColor.r, targetColor.g, targetColor.b, curRemap->distance[i], diff, unmappedColors);
		if (result != -1 && curRemap->remap[i] != result)  {
			changed = true;
			curRemap->remap[i] = result;
			curRemap->distance[i] = diff;
		}
	}

	return changed;
}

bool GfxRemap32::remapAllTables(bool palChanged) {
	bool changed = false;

	for (int i = 0; i < REMAP_COLOR_COUNT; i++) {
		changed |= updateRemap(i, palChanged);
	}

	_update = false;
	return changed;
}

#endif

} // End of namespace Sci
