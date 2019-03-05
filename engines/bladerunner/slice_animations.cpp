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
#include "bladerunner/time.h"

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
		_animations[i].frameCount       = file.readUint32LE();
		_animations[i].frameSize        = file.readUint32LE();
		_animations[i].fps              = file.readFloatLE();
		_animations[i].positionChange.x = file.readFloatLE();
		_animations[i].positionChange.y = file.readFloatLE();
		_animations[i].positionChange.z = file.readFloatLE();
		_animations[i].facingChange     = file.readFloatLE();
		_animations[i].offset           = file.readUint32LE();
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

bool SliceAnimations::openFrames(int fileNumber) {
	if (_framesPageFile._fileNumber == -1) { // Running for the first time, need to probe
		// First, try HDFRAMES.DAT
		if (_framesPageFile.open("HDFRAMES.DAT")) {
			_framesPageFile._fileNumber = 0;

			return true;
		}
	}

	if (_framesPageFile._fileNumber == 0) // HDFRAMES.DAT
		return true;

	if (_framesPageFile._fileNumber == fileNumber)
		return true;

	_framesPageFile.close();

	if (fileNumber == 1 && _framesPageFile.open("CDFRAMES.DAT")) // For Chapter1 we try both CDFRAMES.DAT and CDFRAMES1.DAT
		return true;

	return _framesPageFile.open(Common::String::format("CDFRAMES%d.DAT", fileNumber));
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

	// debug(5, "PageFile::Open: page file \"%s\" opened with %d pages", name.c_str(), pageCount);

	return true;
}

void SliceAnimations::PageFile::close() {
	if (_file.isOpen()) {
		_file.close();
	}
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
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// FIXME: Maybe there's a better way?
	// Sanitize bad frame value
	// For some actors (currently only happened with hawkers_barkeep) it is possible
	// to SAVE a frame value (while saving a game)
	// that in conjunction with other actor script vars not being re-initialized
	// upon LOADING that game (for hawkers_barkeep this variable is "_var2")
	// will lead to an invalid frame here and an assertion fault (now commented out).
	// Example of faulty case:
	// hawkers_barkeep was SAVED as:
	// (animationState, animationFrame, animationStateNext, nextAnimation) = (0, 19, 0, 0)
	// while his animationID was 705
	// if _var1, _var2, _var3  == (0, 6, 1) when LOADING that save file,
	// then animationFrame will remain 19, which is invalid for his 705 animation
	// and the assert will produce a fault when trying to call drawInWorld for him.
	if (frame >= _animations[animation].frameCount) {
		debug("Bad frame: %u max: %u animation: %u", frame, _animations[animation].frameCount, animation);
		frame = 0;
	}
//	assert(frame < _animations[animation].frameCount);
#endif // BLADERUNNER_ORIGINAL_BUGS

	uint32 frameOffset = _animations[animation].offset + frame * _animations[animation].frameSize;
	uint32 page        = frameOffset / _pageSize;
	uint32 pageOffset  = frameOffset % _pageSize;

	if (!_pages[page]._data)
		_pages[page]._data = _coreAnimPageFile.loadPage(page);

	if (!_pages[page]._data)
		_pages[page]._data = _framesPageFile.loadPage(page);

	if (!_pages[page]._data)
		error("Unable to locate page %d for animation %d frame %d", page, animation, frame);

	_pages[page]._lastAccess = _vm->_time->currentSystem();

	return (byte *)_pages[page]._data + pageOffset;
}

Vector3 SliceAnimations::getPositionChange(int animation) const {
	return _animations[animation].positionChange;
}

float SliceAnimations::getFacingChange(int animation) const {
	return _animations[animation].facingChange;
}

} // End of namespace BladeRunner
