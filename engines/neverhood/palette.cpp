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

#include "neverhood/palette.h"
#include "neverhood/resource.h"
#include "neverhood/screen.h"

namespace Neverhood {

// Palette

Palette::Palette(NeverhoodEngine *vm) : Entity(vm, 0) {
	init();
	memset(_palette, 0, 1024);
	SetUpdateHandler(&Palette::update);
}

Palette::Palette(NeverhoodEngine *vm, byte *palette) : Entity(vm, 0) {
	init();
	memcpy(_palette, palette, 1024);
	SetUpdateHandler(&Palette::update);
}

Palette::Palette(NeverhoodEngine *vm, const char *filename) : Entity(vm, 0) {
	PaletteResource paletteResource(_vm);
	init();
	paletteResource.load(calcHash(filename));
	paletteResource.copyPalette(_palette);
	SetUpdateHandler(&Palette::update);
}

Palette::Palette(NeverhoodEngine *vm, uint32 fileHash) : Entity(vm, 0) {
	PaletteResource paletteResource(_vm);
	init();
	paletteResource.load(fileHash);
	paletteResource.copyPalette(_palette);
	SetUpdateHandler(&Palette::update);
}

Palette::~Palette() {
	_vm->_screen->unsetPaletteData(_palette);
	delete[] _palette;
	delete[] _basePalette;
}

void Palette::init() {
	_status = 0;
	_palette = new byte[1024];
	_basePalette = new byte[1024];
	_palCounter = 0;
	_fadeToR = 0;
	_fadeToG = 0;
	_fadeToB = 0;
	_fadeStep = 0;
}

void Palette::usePalette() {
	_vm->_screen->setPaletteData(_palette);
}

void Palette::addPalette(const char *filename, int toIndex, int count, int fromIndex) {
	addPalette(calcHash(filename), toIndex, count, fromIndex);
}

void Palette::addPalette(uint32 fileHash, int toIndex, int count, int fromIndex) {
	PaletteResource paletteResource(_vm);
	if (toIndex + count > 256)
		count = 256 - toIndex;
	paletteResource.load(fileHash);
	memcpy(_palette + toIndex * 4, paletteResource.palette() + fromIndex * 4, count * 4);
	_vm->_screen->testPalette(_palette);
}

void Palette::addBasePalette(uint32 fileHash, int toIndex, int count, int fromIndex) {
	PaletteResource paletteResource(_vm);
	if (toIndex + count > 256)
		count = 256 - toIndex;
	paletteResource.load(fileHash);
	memcpy(_basePalette + toIndex * 4, paletteResource.palette() + fromIndex * 4, count * 4);
}

void Palette::copyPalette(const byte *palette, int toIndex, int count, int fromIndex) {
	if (toIndex + count > 256)
		count = 256 - toIndex;
	memcpy(_palette + toIndex * 4, palette + fromIndex * 4, count * 4);
	_vm->_screen->testPalette(_palette);
}

void Palette::copyBasePalette(int toIndex, int count, int fromIndex) {
	if (toIndex + count > 256)
		count = 256 - toIndex;
	memcpy(_basePalette + toIndex * 4, _palette + fromIndex * 4, count * 4);
}

void Palette::startFadeToBlack(int counter) {
	debug(2, "Palette::startFadeToBlack(%d)", counter);
	if (counter == 0)
		counter = 1;
	_fadeToR = 0;
	_fadeToG = 0;
	_fadeToB = 0;
	_palCounter = counter;
	_fadeStep = calculateFadeStep(counter);
	_status = 1;
}

void Palette::startFadeToWhite(int counter) {
	debug(2, "Palette::startFadeToWhite(%d)", counter);
	if (counter == 0)
		counter = 1;
	_fadeToR = 255;
	_fadeToG = 255;
	_fadeToB = 255;
	_palCounter = counter;
	_fadeStep = calculateFadeStep(counter);
	_status = 1;
}

void Palette::startFadeToPalette(int counter) {
	debug(2, "Palette::startFadeToPalette(%d)", counter);
	if (counter == 0)
		counter = 1;
	_palCounter = counter;
	_fadeStep = calculateFadeStep(counter);
	_status = 2;
}

void Palette::fillBaseWhite(int index, int count) {
	if (index + count > 256)
		count = 256 - index;
	for (int i = 0; i < count; i++) {
		_basePalette[(i + index) * 4 + 0] = 0xFF;
		_basePalette[(i + index) * 4 + 1] = 0xFF;
		_basePalette[(i + index) * 4 + 2] = 0xFF;
		_basePalette[(i + index) * 4 + 3] = 0;
	}
}

void Palette::fillBaseBlack(int index, int count) {
	if (index + count > 256)
		count = 256 - index;
	for (int i = 0; i < count; i++) {
		_basePalette[(i + index) * 4 + 0] = 0;
		_basePalette[(i + index) * 4 + 1] = 0;
		_basePalette[(i + index) * 4 + 2] = 0;
		_basePalette[(i + index) * 4 + 3] = 0;
	}
}

void Palette::copyToBasePalette(byte *palette) {
	memcpy(_basePalette, palette, 256 * 4);
}

void Palette::update() {
	debug(2, "Palette::update() _status = %d", _status);
	if (_status == 1) {
		if (_palCounter > 1) {
			for (int i = 0; i < 256; i++)
				fadeColor(_palette + i * 4, _fadeToR, _fadeToG, _fadeToB);
			_vm->_screen->testPalette(_palette);
			_palCounter--;
		} else {
			memset(_palette, 0, 1024);
			_status = 0;
		}
	} else if (_status == 2) {
		if (_palCounter > 1) {
			for (int i = 0; i < 256; i++)
				fadeColor(_palette + i * 4, _basePalette[i * 4 + 0], _basePalette[i * 4 + 1], _basePalette[i * 4 + 2]);
			_vm->_screen->testPalette(_palette);
			_palCounter--;
		} else {
			memcpy(_palette, _basePalette, 256 * 4);
			_status = 0;
		}
	}
}

void Palette::fadeColor(byte *rgb, byte toR, byte toG, byte toB) {
	#define FADE(color, toColor) color += _fadeStep < toColor - color ? _fadeStep : (-_fadeStep <= toColor - color ? toColor - color : -_fadeStep)
	FADE(rgb[0], toR);
	FADE(rgb[1], toG);
	FADE(rgb[2], toB);
	#undef FADE
}

int Palette::calculateFadeStep(int counter) {
	int fadeStep = 255 / counter;
	if (255 % counter)
		fadeStep++;
	return fadeStep;
}

} // End of namespace Neverhood
