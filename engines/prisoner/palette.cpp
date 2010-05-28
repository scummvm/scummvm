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

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Palette */

void PrisonerEngine::setPalette(Common::String &pakName, int16 pakSlot) {
	int16 paletteResourceCacheSlot = _res->load<PaletteResource>(pakName, pakSlot, 6);
	PaletteResource *paletteResource = _res->get<PaletteResource>(paletteResourceCacheSlot);
	memcpy(_effectPalette, paletteResource->getPalette(), 768);
	memcpy(_scenePalette, paletteResource->getPalette(), 768);
	_scenePaletteOk = true;
	_effectPaletteOk = true;
	_screen->buildPaletteTransTable(paletteResource->getPalette(), 0);
	_res->unload(paletteResourceCacheSlot);
	_needToUpdatePalette = true;
}

void PrisonerEngine::fadeInOutColor(byte *source, int16 fadeDirection, int16 fadeR, int16 fadeG, int16 fadeB, int16 fadePosition) {
	if (fadeDirection == 1)
		fadePosition = 255 - fadePosition;
	byte *dst = _effectPalette;
	for (int i = 0; i < 240; i++) {
		byte r = *source++;
		byte g = *source++;
		byte b = *source++;
		*dst++ = (((fadeR - r) * fadePosition) >> 8) + r;
		*dst++ = (((fadeG - g) * fadePosition) >> 8) + g;
		*dst++ = (((fadeB - b) * fadePosition) >> 8) + b;
	}
}

void PrisonerEngine::alarmPalette(byte *source) {
	byte *dst = _effectPalette;
	*dst++ = *source++;
	*dst++ = *source++;
	*dst++ = *source++;
	for (int i = 1; i < 240; i++) {
		*dst++ = *source++;
		*dst++ = MAX(*source++ - _alarmPaletteSub, 0);
		*dst++ = MAX(*source++ - _alarmPaletteSub, 0);
	}
	_alarmPaletteSub += _alarmPaletteSubDelta;
	if (_alarmPaletteSub <= 10 || _alarmPaletteSub >= 80)
		_alarmPaletteSubDelta = -_alarmPaletteSubDelta;
}

void PrisonerEngine::startPaletteTask(int16 type, int16 value1, int16 value2, int16 value3) {

	switch (type) {
	case 3:
		_paletteTasks[1].active = true;
		_paletteTasks[1].positionIncr = value3;
		_paletteTasks[1].updateTicks = value3;
		debug("alarm: %d, %d, %d", value1, value2, value3);
		break;
	case 4:
		_paletteTasks[1].active = false;
		break;
	case 7:
		_paletteTasks[3].active = true;
		fadeInOutColor(_scenePalette, value2 & 0xFF, (value1 & 0xFF00) >> 8, value1 & 0xFF, (value2 & 0xFF00) >> 8, 0);
		_paletteTasks[3].value2 = value1;
		_paletteTasks[3].value3 = value2;
		if (value3 > 255) {
			_paletteTasks[3].positionIncr = 1;
			_paletteTasks[3].updateTicks = value3 / 255;
		} else {
			_paletteTasks[3].positionIncr = 255 / value3;
			_paletteTasks[3].updateTicks = 1;
		}
		_fadeInOutColorUpdateTicks = _paletteTasks[3].updateTicks;
		_fadeInOutColorPosition = 0;
		break;
	case 8:
	case 9:
		_paletteTasks[3].active = false;
		fadeInOutColor(_scenePalette, _paletteTasks[3].value3 & 0xFF, (_paletteTasks[3].value2 & 0xFF00) >> 8,
			_paletteTasks[3].value2 & 0x00FF, (_paletteTasks[3].value3 & 0xFF00) >> 8, 255);
		_screen->setFullPalette(_effectPalette);
		break;
	case 10:
		unloadBackground();
		break;
	default:
		break;
	}

}

void PrisonerEngine::updatePaletteTasks() {

	if (_paletteTasks[1].active) {
		_paletteTasks[1].updateTicks -= _frameTicks;
		if (_paletteTasks[1].updateTicks <= 0) {
			_paletteTasks[1].updateTicks = _paletteTasks[1].positionIncr;
			alarmPalette(_scenePalette);
			_needToUpdatePalette = true;
		}
	}

	if (_paletteTasks[3].active) {
		_paletteTasks[3].updateTicks -= _frameTicks;
		if (_paletteTasks[3].updateTicks <= 0) {
			_paletteTasks[3].updateTicks = _fadeInOutColorUpdateTicks;
			_fadeInOutColorPosition += _paletteTasks[3].positionIncr;
			if (_fadeInOutColorPosition > 255) {
				_fadeInOutColorPosition = 255;
				_paletteTasks[3].active = false;
			}
			fadeInOutColor(_scenePalette, _paletteTasks[3].value3 & 0xFF, (_paletteTasks[3].value2 & 0xFF00) >> 8,
				_paletteTasks[3].value2 & 0x00FF, (_paletteTasks[3].value3 & 0xFF00) >> 8,
				_fadeInOutColorPosition);
			_needToUpdatePalette = true;
		}
	}

}

void PrisonerEngine::clearPaletteTasks() {
	for (int16 i = 0; i < kMaxPaletteTasks; i++) {
		_paletteTasks[i].active = false;
		_paletteTasks[i].value1 = -1;
	}
}

} // End of namespace Prisoner
