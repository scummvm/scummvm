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

	Graphics::PixelFormat screenFormat = screenPixelFormat();

	for (uint32 i = 0; i != _paletteCount; ++i) {
		for (uint32 j = 0; j != 256; ++j) {
			uint8 color_r = file.readByte();
			uint8 color_g = file.readByte();
			uint8 color_b = file.readByte();

			_palettes[i].color[j].r = color_r;
			_palettes[i].color[j].g = color_g;
			_palettes[i].color[j].b = color_b;

			const int bladeToScummVmConstant = 256 / 32; // 5 bits to 8 bits
			_palettes[i].value[j] = screenFormat.RGBToColor(color_r * bladeToScummVmConstant, color_g * bladeToScummVmConstant, color_b * bladeToScummVmConstant);
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

	// close open files
	_coreAnimPageFile.close(0);
	if (!_vm->_cutContent) {
		_framesPageFile.close(_framesPageFile._fileNumber);
	} else {
		for (uint i = 0; i < 5; ++i) {
			_framesPageFile.close(i);
		}
	}
}

bool SliceAnimations::openCoreAnim() {
	return _coreAnimPageFile.open("COREANIM.DAT", 0);
}

bool SliceAnimations::openFrames(int fileNumber) {

	if (_framesPageFile._fileNumber == -1) { // Running for the first time, need to probe
		// First, try HDFRAMES.DAT
		if (_framesPageFile.open("HDFRAMES.DAT", 0)) {
			_framesPageFile._fileNumber = 0;

			return true;
		} else {
			debug("SliceAnimations::openFrames: HDFRAMES.DAT resource not found. Falling back to using CDFRAMESx.DAT files instead...");
		}
	}

	if (_framesPageFile._fileNumber == 0) // HDFRAMES.DAT
		return true;

	if (!_vm->_cutContent && _framesPageFile._fileNumber == fileNumber)
		return true;

	if (_vm->_cutContent && _framesPageFile._fileNumber == 5) // all frame files loaded
		return true;

	if (!_vm->_cutContent) {
		// _fileNumber can normally be in [1,4]
		// but it will be "5" if we switched from restored content to original
		if (_framesPageFile._fileNumber == 5) {
			for (uint i = 1; i < 5; ++i) {
				_framesPageFile.close(i);
			}
		} else if (_framesPageFile._fileNumber > 0) {
			_framesPageFile.close(_framesPageFile._fileNumber);
		}
		_framesPageFile._fileNumber = fileNumber;
		// For Chapter1 we try both CDFRAMES.DAT and CDFRAMES1.DAT
		if (fileNumber == 1 && _framesPageFile.open("CDFRAMES.DAT", fileNumber)) {
			return true;
		}

		if (_framesPageFile.open(Common::String::format("CDFRAMES%d.DAT", fileNumber), fileNumber)) {
			return true;
		}
	} else {
		// Restored cut content case
		// open all four CDFRAMESx.DAT files in slots [1,5]
		// So that all animation resources are available at all times as if we just had the single HDFRAMES.DAT file
		for (uint i = 1; i < 5; ++i) {
			_framesPageFile.close(i);
			if (i == 1
			    && (!_framesPageFile.open("CDFRAMES.DAT", i))
			    && (!_framesPageFile.open(Common::String::format("CDFRAMES%d.DAT", i), i))
			) {
				// For Chapter1 we try both CDFRAMES.DAT and CDFRAMES1.DAT
				return false;
			} else if (i != 1 &&
			          !_framesPageFile.open(Common::String::format("CDFRAMES%d.DAT", i), i)
			) {
				return false;
			}
		}
		_framesPageFile._fileNumber = 5;
		return true;
	}
	return false;
}

bool SliceAnimations::PageFile::open(const Common::String &name, int8 fileIdx) {
	if (!_files[fileIdx].open(name))
		return false;

	uint32 timestamp = _files[fileIdx].readUint32LE();
	if (timestamp != _sliceAnimations->_timestamp)
		return false;

	if (!_sliceAnimations->_vm->_cutContent
		|| (_pageOffsets.size() < _sliceAnimations->_pageCount) ) {
		_pageOffsets.resize(_sliceAnimations->_pageCount);
		_pageOffsetsFileIdx.resize(_sliceAnimations->_pageCount);
		for (uint32 i = 0; i != _sliceAnimations->_pageCount; ++i) {
			_pageOffsets[i] = -1;
			_pageOffsetsFileIdx[i] = -1;
		}
	}

	uint32 pageCount  = _files[fileIdx].readUint32LE();
	uint32 dataOffset = 8 + 4 * pageCount;

	for (uint32 i = 0; i != pageCount; ++i) {
		uint32 pageNumber = _files[fileIdx].readUint32LE();
		if (pageNumber == 0xffffffff)
			continue;
		_pageOffsets[pageNumber] = dataOffset + i * _sliceAnimations->_pageSize;
		_pageOffsetsFileIdx[pageNumber] = fileIdx;
	}

	// debug(5, "PageFile::Open: page file \"%s\" opened with %d pages", name.c_str(), pageCount);

	return true;
}

void SliceAnimations::PageFile::close(int8 fileIdx) {
	if (fileIdx >= 0 && fileIdx < 5) {
		if (_files[fileIdx].isOpen()) {
			_files[fileIdx].close();
		}
	}
}

void *SliceAnimations::PageFile::loadPage(uint32 pageNumber) {
	if (_pageOffsets.size() < _sliceAnimations->_pageCount
	    || _pageOffsetsFileIdx.size() < _sliceAnimations->_pageCount
	    || _pageOffsets[pageNumber] == -1
	    || _pageOffsetsFileIdx[pageNumber] == -1) {
		return nullptr;
	}

	uint32 pageSize = _sliceAnimations->_pageSize;

	// TODO: Retire oldest pages if we exceed some memory limit

	void *data = malloc(pageSize);
	_files[_pageOffsetsFileIdx[pageNumber]].seek(_pageOffsets[pageNumber], SEEK_SET);
	uint32 r = _files[_pageOffsetsFileIdx[pageNumber]].read(data, pageSize);
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

	if (_pages[page]._data == nullptr) {                          // if not cached already
		_pages[page]._data = _coreAnimPageFile.loadPage(page);    // look in COREANIM first

		if (_pages[page]._data == nullptr) {                      // if not in COREAMIM
			_pages[page]._data = _framesPageFile.loadPage(page);  // Look in CDFRAMES or HDFRAMES loaded data

			if (_pages[page]._data == nullptr) {
				error("Unable to locate page %d for animation %d frame %d", page, animation, frame);
			}
		}
	}

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
