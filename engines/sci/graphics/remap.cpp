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
#include "sci/graphics/palette.h"
#include "sci/graphics/remap.h"
#include "sci/graphics/screen.h"

namespace Sci {

GfxRemap::GfxRemap(GfxPalette *palette)
	: _palette(palette) {
	_remapOn = false;
	resetRemapping();
}

byte GfxRemap::remapColor(byte remappedColor, byte screenColor) {
	assert(_remapOn);
	if (_remappingType[remappedColor] == kRemapByRange)
		return _remappingByRange[screenColor];
	else if (_remappingType[remappedColor] == kRemapByPercent)
		return _remappingByPercent[screenColor];
	else
		error("remapColor(): Color %d isn't remapped", remappedColor);

	return 0;	// should never reach here
}

void GfxRemap::resetRemapping() {
	_remapOn = false;
	_remappingPercentToSet = 0;

	for (int i = 0; i < 256; i++) {
		_remappingType[i] = kRemapNone;
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

	_remappingType[color] = kRemapByPercent;
}

void GfxRemap::setRemappingRange(byte color, byte from, byte to, byte base) {
	_remapOn = true;

	for (int i = from; i <= to; i++) {
		_remappingByRange[i] = i + base;
	}

	_remappingType[color] = kRemapByRange;
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
} // End of namespace Sci
