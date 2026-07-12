/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "comfy/comfy.h"

#include "common/textconsole.h"
#include "graphics/paletteman.h"

namespace Comfy {

void ComfyEngine::colorDatOpen() {
	if (_colorDatStream)
		return;

	_colorDatStream = pathFOpen(Common::Path("COLORS.DAT"), true);
	if (!_colorDatStream)
		error("Could not open COLORS.DAT");
}

void ComfyEngine::colorDatClose() {
	delete _colorDatStream;
	_colorDatStream = nullptr;
}

void ComfyEngine::colorDatReadEntry(uint16 paletteId) {
	colorDatOpen();
	uint16 offset = uint16(paletteId * COMFY_PALETTE_BYTES);
	if (!_colorDatStream->seek(offset))
		error("Could not seek to palette %u in COLORS.DAT", uint(paletteId));

	if (_colorDatStream->read(_paletteTarget, COMFY_PALETTE_BYTES) != COMFY_PALETTE_BYTES)
		error("COLORS.DAT is truncated while reading palette %u", uint(paletteId));

	if (_engineVersion == kEngineVersion3) {
		// Version 3 forces color 0 to black immediately after reading the palette...
		_paletteTarget[0] = 0;
		_paletteTarget[1] = 0;
		_paletteTarget[2] = 0;
	}
}

void ComfyEngine::vsyncSetPalettePtr(byte *palette) {
	_vsyncPending = 2;
	_paletteDataPtr = palette;
}

void ComfyEngine::paletteInterpolate(uint16 step, uint16 maximum) {
	if (!maximum)
		return;

	int16 remaining = int16(maximum - step);
	for (uint i = 0; i < COMFY_PALETTE_BYTES; i++) {
		int32 from = _paletteFadeSource[i];
		int32 to = _paletteTarget[i];
		_paletteDisplay[i] = byte((from * remaining + to * int16(step)) / int16(maximum));
	}
}

void ComfyEngine::paletteFadeStep(uint16 delta) {
	if (!_paletteFading)
		return;

	_fadeStep = uint16(_fadeStep + delta);
	if (int16(_fadeStep) >= int16(_fadeMax)) {
		_fadeStep = _fadeMax;
		memcpy(_paletteFadeSource, _paletteTarget, COMFY_PALETTE_BYTES);
		memcpy(_paletteDisplay, _paletteTarget, COMFY_PALETTE_BYTES);
		_paletteFading = false;
	} else {
		paletteInterpolate(_fadeStep, _fadeMax);
	}

	vsyncSetPalettePtr(_paletteDisplay);
}

void ComfyEngine::paletteLoadWithFade(uint16 paletteId, uint16 fadeTicks) {
	colorDatReadEntry(paletteId);
	if (_engineVersion != kEngineVersion3)
		_palettePage = 'S';

	if (_paletteFading)
		memcpy(_paletteFadeSource, _paletteDisplay, COMFY_PALETTE_BYTES);

	if (!fadeTicks) {
		_paletteFading = false;
		memcpy(_paletteDisplay, _paletteTarget, COMFY_PALETTE_BYTES);
		memcpy(_paletteFadeSource, _paletteTarget, COMFY_PALETTE_BYTES);
		vsyncSetPalettePtr(_paletteDisplay);
	} else {
		_paletteFading = true;
		_fadeMax = fadeTicks;
		_fadeStep = 0;
	}
}

void ComfyEngine::paletteApplyBrightness(uint16 brightness) {
	if (_paletteFading)
		_paletteFading = false;

	uint16 inverse = uint16(0x100 - brightness);
	memcpy(_paletteFadeSource, _paletteDisplay, COMFY_PALETTE_BYTES);
	memcpy(_paletteDisplay, _paletteFadeSource + inverse * 3, brightness * 3);
	memcpy(_paletteDisplay + brightness * 3, _paletteFadeSource, inverse * 3);
	vsyncSetPalettePtr(_paletteDisplay);
}

void ComfyEngine::paletteConvertRgbToLogical(byte *source, byte *destination) {
	for (uint i = 0; i < 256; i++) {
		destination[i * 4 + 2] = byte(source[i * 3] << 2);
		destination[i * 4 + 1] = byte(source[i * 3 + 1] << 2);
		destination[i * 4] = byte(source[i * 3 + 2] << 2);
	}
}

void ComfyEngine::paletteRealize(byte *rawPalette) {
	if (rawPalette)
		paletteConvertRgbToLogical(rawPalette, _logicalPalette);

	_logicalPalette[255 * 4 + 2] = 0xFF;
	_logicalPalette[255 * 4 + 1] = 0xFF;
	_logicalPalette[255 * 4] = 0xFF;
	_logicalPalette[2] = 0;
	_logicalPalette[1] = 0;
	_logicalPalette[0] = 0;

	byte palette[COMFY_PALETTE_BYTES];
	for (uint i = 0; i < 256; i++) {
		palette[i * 3] = _logicalPalette[i * 4 + 2];
		palette[i * 3 + 1] = _logicalPalette[i * 4 + 1];
		palette[i * 3 + 2] = _logicalPalette[i * 4];
	}

	_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void ComfyEngine::paletteVsyncFlip() {
	if (!_vsyncPending)
		return;

	paletteRealize(_paletteDataPtr);
	_vsyncPending = 0;
}

} // End of namespace Comfy
