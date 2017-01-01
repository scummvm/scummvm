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

#include "bladerunner/slice_animations.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"

namespace BladeRunner {

bool SliceAnimations::open(const Common::String &name) {
	Common::File file;
	if (!file.open(_vm->getResourceStream(name), name))
		return false;

	_timestamp    = file.readUint32LE();
	_pageSize     = file.readUint32LE();
	_pageCount    = file.readUint32LE();
	_paletteCount = file.readUint32LE();

	if (_timestamp != 0x3457b6f6) // Timestamp: Wed, 29 Oct 1997 22:21:42 GMT
		return false;

	_palettes.resize(_paletteCount);

	for (uint32 i = 0; i != _paletteCount; ++i) {
		for (uint32 j = 0; j != 256; ++j) {
			uint8 color_r = file.readByte();
			uint8 color_g = file.readByte();
			uint8 color_b = file.readByte();

			_palettes[i].color[j].r = color_r;
			_palettes[i].color[j].g = color_g;
			_palettes[i].color[j].b = color_b;

			uint16 rgb555 = ((uint16)color_r << 10) |
			                ((uint16)color_g <<  5) |
			                 (uint16)color_b;

			_palettes[i].color555[j] = rgb555;
		}
	}

	uint32 animationCount = file.readUint32LE();
	_animations.resize(animationCount);

	for (uint32 i = 0; i != animationCount; ++i) {
		_animations[i].frameCount = file.readUint32LE();
		_animations[i].frameSize  = file.readUint32LE();
		_animations[i].fps        = file.readFloatLE();
		_animations[i].positionChange.x = file.readFloatLE();
		_animations[i].positionChange.y = file.readFloatLE();
		_animations[i].positionChange.z = file.readFloatLE();
		_animations[i].facingChange = file.readFloatLE();
		_animations[i].offset     = file.readUint32LE();

#if 0
		debug("%4d  %6d %6x  %7.2g %7.2g %7.2g %7.2g %7.2g %8x",
			i,
			_animations[i].frameCount,
			_animations[i].frameSize,
			_animations[i].fps,
			_animations[i].unk0,
			_animations[i].unk1,
			_animations[i].unk2,
			_animations[i].unk3,
			_animations[i].offset);
#endif
	}

	_pages.resize(_pageCount);
	for (uint32 i = 0; i != _pageCount; ++i)
		_pages[i]._data = nullptr;

	return true;
}

SliceAnimations::~SliceAnimations() {
	for (uint32 i = 0; i != _pages.size(); ++i)
		free(_pages[i]._data);
}

bool SliceAnimations::openCoreAnim() {
	return _coreAnimPageFile.open("COREANIM.DAT");
}

bool SliceAnimations::openHDFrames() {
	return _framesPageFile.open("HDFRAMES.DAT");
}

bool SliceAnimations::PageFile::open(const Common::String &name) {
	if (!_file.open(name))
		return false;

	uint32 timestamp = _file.readUint32LE();
	if (timestamp != _sliceAnimations->_timestamp)
		return false;

	_pageOffsets.resize(_sliceAnimations->_pageCount);
	for (uint32 i = 0; i != _sliceAnimations->_pageCount; ++i)
		_pageOffsets[i] = -1;

	uint32 pageCount  = _file.readUint32LE();
	uint32 dataOffset = 8 + 4 * pageCount;

	for (uint32 i = 0; i != pageCount; ++i) {
		uint32 pageNumber = _file.readUint32LE();
		if (pageNumber == 0xffffffff)
			continue;
		_pageOffsets[pageNumber] = dataOffset + i * _sliceAnimations->_pageSize;
	}

	// debug("PageFile::Open: page file \"%s\" opened with %d pages", name.c_str(), pageCount);

	return true;
}

void *SliceAnimations::PageFile::loadPage(uint32 pageNumber) {
	if (_pageOffsets[pageNumber] == -1)
		return nullptr;

	uint32 pageSize = _sliceAnimations->_pageSize;

	// TODO: Retire oldest pages if we exceed some memory limit

	void *data = malloc(pageSize);
	_file.seek(_pageOffsets[pageNumber], SEEK_SET);
	uint32 r = _file.read(data, pageSize);
	assert(r == pageSize);

	return data;
}

void *SliceAnimations::getFramePtr(uint32 animation, uint32 frame) {
	assert(frame < _animations[animation].frameCount);

	uint32 frameOffset = _animations[animation].offset + frame * _animations[animation].frameSize;
	uint32 page        = frameOffset / _pageSize;
	uint32 pageOffset  = frameOffset % _pageSize;

	if (!_pages[page]._data)
		_pages[page]._data = _coreAnimPageFile.loadPage(page);

	if (!_pages[page]._data)
		_pages[page]._data = _framesPageFile.loadPage(page);

	if (!_pages[page]._data)
		error("Unable to locate page %d for animation %d frame %d", page, animation, frame);

	_pages[page]._lastAccess = _vm->_system->getMillis();

	return (byte*)_pages[page]._data + pageOffset;
}

Vector3 SliceAnimations::getPositionChange(int animation) {
	return _animations[animation].positionChange;
}

float SliceAnimations::getFacingChange(int animation) {
	return _animations[animation].facingChange;
}

} // End of namespace BladeRunner
