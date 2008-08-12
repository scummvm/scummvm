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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/palette.h"
#include "picture/resource.h"

namespace Picture {

Palette::Palette(PictureEngine *vm) : _vm(vm) {

	clearFragments();
	
	memset(_colorTransTable, 0, sizeof(_colorTransTable));
	
}

Palette::~Palette() {
}

void Palette::setFullPalette(byte *palette) {
	byte colors[1024];
	for (int i = 0; i < 256; i++) {
		colors[i * 4 + 0] = palette[i * 3 + 0] << 2;
		colors[i * 4 + 1] = palette[i * 3 + 1] << 2;
		colors[i * 4 + 2] = palette[i * 3 + 2] << 2;
		colors[i * 4 + 3] = 255;
	}
	_vm->_system->setPalette((const byte *)colors, 0, 256);
	_vm->_system->updateScreen();
}

void Palette::getFullPalette(byte *palette) {
	byte colors[1024];
	_vm->_system->grabPalette(colors, 0, 256);
	for (int i = 0; i < 256; i++) {
		palette[i * 3 + 0] = colors[i * 4 + 0] >> 2;
		palette[i * 3 + 1] = colors[i * 4 + 1] >> 2;
		palette[i * 3 + 2] = colors[i * 4 + 2] >> 2;
	}
}

void Palette::setDeltaPalette(byte *palette, byte mask, char deltaValue, int16 count, int16 startIndex) {

	byte colors[1024];

	byte *palPtr = palette + startIndex * 3;
	int16 index = startIndex, colorCount = count;
	byte rgb;

	count++;

	mask &= 7;

	_vm->_system->grabPalette(colors, 0, 256);
	
	if (deltaValue < 0) {
		deltaValue = -deltaValue;
		while (count--) {
			rgb = *palPtr++;
			if (mask & 1) colors[index * 4 + 0] = CLIP<int>(rgb + deltaValue, 0, 63) << 2;
			rgb = *palPtr++;
			if (mask & 2) colors[index * 4 + 1] = CLIP<int>(rgb + deltaValue, 0, 63) << 2;
			rgb = *palPtr++;
			if (mask & 4) colors[index * 4 + 2] = CLIP<int>(rgb + deltaValue, 0, 63) << 2;
			index++;
		}
	} else {
		while (count--) {
			rgb = *palPtr++;
			if (mask & 1) colors[index * 4 + 0] = CLIP<int>(rgb - deltaValue, deltaValue, 255) << 2;
			rgb = *palPtr++;
			if (mask & 2) colors[index * 4 + 1] = CLIP<int>(rgb - deltaValue, deltaValue, 255) << 2;
			rgb = *palPtr++;
			if (mask & 4) colors[index * 4 + 2] = CLIP<int>(rgb - deltaValue, deltaValue, 255) << 2;
		 	index++;
		}
	}
	
	debug(0, "startIndex = %d; colorCount = %d", startIndex, colorCount);

	_vm->_system->setPalette((const byte *)colors, 0, 256);

}

void Palette::loadAddPalette(uint resIndex, byte startIndex) {
	byte *data = _vm->_res->load(resIndex);
	memcpy(&_mainPalette[startIndex * 3], data, _vm->_res->getCurItemSize());
}

void Palette::loadAddPaletteFrom(byte *source, byte startIndex, byte count) {
	memcpy(&_mainPalette[startIndex * 3], source, count * 3);
}

void Palette::addFragment(uint resIndex, int16 id) {
	debug(0, "Palette::addFragment(%d, %d)", resIndex, id);

	byte *fragmentData = _vm->_res->load(resIndex);
	byte count = _vm->_res->getCurItemSize() / 3;
	
	memcpy(&_mainPalette[_fragmentIndex * 3], fragmentData, count * 3);
	
	PaletteFragment fragment;
	fragment.id = id;
	fragment.index = _fragmentIndex;
	fragment.count = count;
	_fragments.push_back(fragment);

	debug(0, "Palette::addFragment() index = %02X; count = %02X", fragment.index, fragment.count);

	_fragmentIndex += count;

}

uint16 Palette::findFragment(int16 id) {
	debug(0, "Palette::findFragment(%d)", id);

	uint16 result = 0;
	for (PaletteFragmentArray::iterator iter = _fragments.begin(); iter != _fragments.end(); iter++) {
		PaletteFragment fragment = *iter;
		if (fragment.id == id) {
			result = (fragment.count << 8) | fragment.index;
			break;
		}
	}
	
	debug(0, "Palette::findFragment() result = %04X", result);

	return result;
}

void Palette::clearFragments() {
	debug(0, "Palette::clearFragments()");
	_fragmentIndex = 128;
	_fragments.clear();
}

void Palette::saveState(Common::WriteStream *out) {

	// Save currently active palette
	byte palette[768];
	getFullPalette(palette);
	out->write(palette, 768);

	out->write(_mainPalette, 768);
	out->write(_animPalette, 768);
	out->write(_colorTransTable, 256);

	uint16 fragmentCount = _fragments.size();
	out->writeUint16LE(fragmentCount);
	for (PaletteFragmentArray::iterator iter = _fragments.begin(); iter != _fragments.end(); iter++) {
		PaletteFragment fragment = *iter;
		out->writeUint16LE(fragment.id);
		out->writeByte(fragment.index);
		out->writeByte(fragment.count);
	}
	out->writeByte(_fragmentIndex);

}

void Palette::loadState(Common::ReadStream *in) {

	// Save currently active palette
	byte palette[768];
	in->read(palette, 768);
	setFullPalette(palette);

	in->read(_mainPalette, 768);
	in->read(_animPalette, 768);
	in->read(_colorTransTable, 256);

	uint16 fragmentCount = in->readUint16LE();
	_fragments.clear();
	for (uint16 i = 0; i < fragmentCount; i++) {
		PaletteFragment fragment;
		fragment.id = in->readUint16LE();
		fragment.index = in->readByte();
		fragment.count = in->readByte();
		_fragments.push_back(fragment);
	}
	_fragmentIndex = in->readByte();

}


} // End of namespace Picture
