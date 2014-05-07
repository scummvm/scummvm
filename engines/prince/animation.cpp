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
 */

#include "prince/animation.h"
#include "prince/decompress.h"

#include "common/debug.h"
#include "common/endian.h"

namespace Prince {

bool Animation::loadFromStream(Common::SeekableReadStream &stream) {
	_dataSize = stream.size();
	_data = (byte *)malloc(_dataSize);

	if (stream.read(_data, _dataSize) != _dataSize) {
		free(_data);
		return false;
	}
	return true;
}

Animation::Animation(): _data(NULL) {

}

Animation::Animation(byte *data, uint32 dataSize)
	: _data(data), _dataSize(dataSize) {
}

Animation::~Animation() {
	free(_data);
}

void Animation::clear() {
	if (_data != NULL) {
		free(_data);
	}
}

// TEMP
/*
int8 Animation::getZoom(uint16 offset) const {
	return *(uint8*)(_data+offset);
}
*/
int16 Animation::getZoom(uint16 offset) const {
	return READ_LE_UINT16(_data + offset);
}

// AH_Loop
int16 Animation::getLoopCount() const {
	return READ_LE_UINT16(_data + 2);
}

// AH_Fazy
uint Animation::getPhaseCount() const {
	return READ_LE_UINT16(_data + 4);
}

// AH_Ramki
uint Animation::getFrameCount() const {
	return READ_LE_UINT16(_data + 6);
}

// AH_X
int16 Animation::getBaseX() const {
	return READ_LE_UINT16(_data + 8);
}

// AH_Y
int16 Animation::getBaseY() const {
	return READ_LE_UINT16(_data + 10);
}

byte *Animation::getPhaseEntry(uint phaseIndex) const {
	return _data + READ_LE_UINT32(_data + 12) + phaseIndex * 8;
}

int16 Animation::getPhaseOffsetX(uint phaseIndex) const {
	return READ_LE_UINT16(getPhaseEntry(phaseIndex) + 0);
}

int16 Animation::getPhaseOffsetY(uint phaseIndex) const {
	return READ_LE_UINT16(getPhaseEntry(phaseIndex) + 2);
}

int16 Animation::getPhaseFrameIndex(uint phaseIndex) const {
	return READ_LE_UINT16(getPhaseEntry(phaseIndex) + 4);
}

int16 Animation::getFrameWidth(uint frameIndex) const {
	byte *frameData = _data + READ_LE_UINT32(_data + 16 + frameIndex * 4);
	return READ_LE_UINT16(frameData + 0);
}

int16 Animation::getFrameHeight(uint frameIndex) const {
	byte *frameData = _data + READ_LE_UINT32(_data + 16 + frameIndex * 4);
	return READ_LE_UINT16(frameData + 2);
}

Graphics::Surface *Animation::getFrame(uint frameIndex) {
	byte *frameData = _data + READ_LE_UINT32(_data + 16 + frameIndex * 4);
	int16 width = READ_LE_UINT16(frameData + 0);
	int16 height = READ_LE_UINT16(frameData + 2);
	debug("width = %d; height = %d", width, height);
	Graphics::Surface *surf = new Graphics::Surface();
	surf->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	debug("frameData %p", frameData);
	if (READ_BE_UINT32(frameData + 4) == MKTAG('m', 'a', 's', 'm')) {
		// Compressed
		Decompressor dec;
		uint32 ddataSize = READ_LE_UINT32(frameData + 8);
		byte *ddata = (byte *)malloc(ddataSize);
		dec.decompress(frameData + 12, ddata, ddataSize);
		for (uint16 i = 0; i < height; i++) {
			memcpy(surf->getBasePtr(0, i), ddata + width * i, width);
		}
		free(ddata);
	} else {
		// Uncompressed
        for (uint16 i = 0; i < height; i++) {
            memcpy(surf->getBasePtr(0, i), frameData + 4 + width * i, width);
        }
	}
	return surf;
}
}

/* vim: set tabstop=4 noexpandtab: */
