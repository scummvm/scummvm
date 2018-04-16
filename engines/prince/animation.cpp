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

#include "prince/animation.h"
#include "prince/decompress.h"

#include "common/debug.h"
#include "common/endian.h"

namespace Prince {

Animation::Animation() : _loopCount(0), _phaseCount(0), _frameCount(0), _baseX(0), _baseY(0)
{
}

Animation::~Animation() {
	clear();
}

void Animation::clear() {
	_phaseList.clear();
	for (int i = 0; i < _frameCount; i++) {
		_frameList[i]._surface->free();
		delete _frameList[i]._surface;
		_frameList[i]._surface = nullptr;
		if (_frameList[i]._compressedData != nullptr) {
			free(_frameList[i]._compressedData);
			_frameList[i]._compressedData = nullptr;
		}
	}
}

bool Animation::loadStream(Common::SeekableReadStream &stream) {
	stream.skip(2); // skip not used x and y coord diff
	_loopCount = stream.readUint16LE();
	_phaseCount = stream.readUint16LE();
	stream.skip(2); // skip _frameCount here
	_baseX = stream.readUint16LE();
	_baseY = stream.readUint16LE();
	uint32 phaseTableOffset = stream.readUint32LE();
	uint32 tableOfFrameOffsets = stream.pos();

	stream.seek(phaseTableOffset);
	Phase tempPhase;
	_frameCount = 0;
	for (int phase = 0; phase < _phaseCount; phase++) {
		tempPhase._phaseOffsetX = stream.readSint16LE();
		tempPhase._phaseOffsetY = stream.readSint16LE();
		tempPhase._phaseToFrameIndex = stream.readUint16LE();
		if (tempPhase._phaseToFrameIndex > _frameCount) {
			_frameCount = tempPhase._phaseToFrameIndex;
		}
		_phaseList.push_back(tempPhase);
		stream.skip(2);
	}
	if (_phaseCount) {
		_frameCount++;
	}

	Frame tempFrame;
	for (int frame = 0; frame < _frameCount; frame++) {
		stream.seek(tableOfFrameOffsets + frame * 4);
		uint32 frameInfoOffset = stream.readUint32LE();
		stream.seek(frameInfoOffset);
		uint16 frameWidth = stream.readUint16LE();
		uint16 frameHeight = stream.readUint16LE();
		uint32 frameDataPos = stream.pos();
		uint32 frameDataOffset = stream.readUint32BE();

		tempFrame._surface = new Graphics::Surface();
		tempFrame._surface->create(frameWidth, frameHeight, Graphics::PixelFormat::createFormatCLUT8());
		if (frameDataOffset == MKTAG('m', 'a', 's', 'm')) {
			tempFrame._isCompressed = true;
			tempFrame._dataSize = stream.readUint32LE();
			tempFrame._compressedData = (byte *)malloc(tempFrame._dataSize);
			stream.read(tempFrame._compressedData, tempFrame._dataSize);
		} else {
			tempFrame._isCompressed = false;
			tempFrame._dataSize = 0;
			tempFrame._compressedData = nullptr;
			stream.seek(frameDataPos);
			for (uint16 i = 0; i < frameHeight; i++) {
				stream.read(tempFrame._surface->getBasePtr(0, i), frameWidth);
			}
		}
		_frameList.push_back(tempFrame);
	}

	return true;
}

int16 Animation::getLoopCount() const {
	return _loopCount;
}

int32 Animation::getPhaseCount() const {
	return _phaseCount;
}

int32 Animation::getFrameCount() const {
	return _frameCount;
}

int16 Animation::getBaseX() const {
	return _baseX;
}

int16 Animation::getBaseY() const {
	return _baseY;
}

int16 Animation::getPhaseOffsetX(int phaseIndex) const {
	if (phaseIndex < _phaseCount) {
		return _phaseList[phaseIndex]._phaseOffsetX;
	} else {
		error("getPhaseOffsetX() phaseIndex: %d, phaseCount: %d", phaseIndex, _phaseCount);
	}
}

int16 Animation::getPhaseOffsetY(int phaseIndex) const {
	if (phaseIndex < _phaseCount) {
		return _phaseList[phaseIndex]._phaseOffsetY;
	} else {
		error("getPhaseOffsetY() phaseIndex: %d, phaseCount: %d", phaseIndex, _phaseCount);
	}
}

int16 Animation::getPhaseFrameIndex(int phaseIndex) const {
	if (phaseIndex < _phaseCount) {
		return _phaseList[phaseIndex]._phaseToFrameIndex;
	} else {
		error("getPhaseFrameIndex() phaseIndex: %d, phaseCount: %d", phaseIndex, _phaseCount);
	}
}

Graphics::Surface *Animation::getFrame(int frameIndex) {
	if (frameIndex < _frameCount) {
		if (_frameList[frameIndex]._isCompressed) {
			Decompressor dec;
			byte *ddata = (byte *)malloc(_frameList[frameIndex]._dataSize);
			dec.decompress(_frameList[frameIndex]._compressedData, ddata, _frameList[frameIndex]._dataSize);
			int frameHeight = _frameList[frameIndex]._surface->h;
			int frameWidth = _frameList[frameIndex]._surface->w;
			for (uint16 i = 0; i < frameHeight; i++) {
				memcpy(_frameList[frameIndex]._surface->getBasePtr(0, i), ddata + frameWidth * i, frameWidth);
			}
			free(ddata);
			free(_frameList[frameIndex]._compressedData);
			_frameList[frameIndex]._compressedData = nullptr;
			_frameList[frameIndex]._dataSize = 0;
			_frameList[frameIndex]._isCompressed = false;
		}
		return _frameList[frameIndex]._surface;
	} else {
		error("getFrame() frameIndex: %d, frameCount: %d", frameIndex, _frameCount);
	}
}

} // End of namespace Prince
