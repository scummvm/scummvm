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

			// We need to convert from 5 bits per channel (r,g,b) to 8 bits
			_palettes[i].value[j] = screenFormat.RGBToColor(Color::get8BitColorFrom5Bit(color_r), Color::get8BitColorFrom5Bit(color_g), Color::get8BitColorFrom5Bit(color_b));
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

		if (_framesPageFile.open(Common::Path(Common::String::format("CD%d/CDFRAMES.DAT", fileNumber)), fileNumber)) {
			return true;
		}

		// For Chapter1 we try both CDFRAMES.DAT and CDFRAMES1.DAT
		if (fileNumber == 1 && _framesPageFile.open("CDFRAMES.DAT", fileNumber)) {
			return true;
		}

		if (_framesPageFile.open(Common::Path(Common::String::format("CDFRAMES%d.DAT", fileNumber)), fileNumber)) {
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
			    && (!_framesPageFile.open(Common::Path(Common::String::format("CD%d/CDFRAMES.DAT", i)), i))
			    && (!_framesPageFile.open(Common::Path(Common::String::format("CDFRAMES%d.DAT", i)), i))
			) {
				// For Chapter1 we try both CDFRAMES.DAT and CDFRAMES1.DAT
				return false;
			} else if (i != 1 &&
				   (!_framesPageFile.open(Common::Path(Common::String::format("CD%d/CDFRAMES.DAT", i)), i)) &&
			          !_framesPageFile.open(Common::Path(Common::String::format("CDFRAMES%d.DAT", i)), i)
			) {
				return false;
			}
		}
		_framesPageFile._fileNumber = 5;
		return true;
	}
	return false;
}

bool SliceAnimations::PageFile::open(const Common::Path &name, int8 fileIdx) {
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

	void *data = malloc(pageSize);
	assert(data);

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
	uint32 pageId      = frameOffset / _pageSize;
	uint32 pageOffset  = frameOffset % _pageSize;

	Page &page = _pages[pageId];

	bool newPage = false;

	if (page._data == nullptr) {                          // if not cached already
		newPage = true;
		page._data = _coreAnimPageFile.loadPage(pageId);    // look in COREANIM first

		if (page._data == nullptr) {                      // if not in COREAMIM
			page._data = _framesPageFile.loadPage(pageId);  // Look in CDFRAMES or HDFRAMES loaded data

			if (page._data == nullptr) {
				error("Unable to locate page %d for animation %d frame %d", pageId, animation, frame);
			}
		}
	}

	page._lastAccess = _vm->_time->currentSystem();
	updatePagesList(page, newPage);

	return (byte *)page._data + pageOffset;
}

void SliceAnimations::updatePagesList(Page &page, bool newPage) {
	// We are already at the end, nothing to update
	// Only cleanup old pages if any
	if (_lastUsedPage == &page) {
		if (newPage) {
			cleanupOutdatedPages();
		}
		return;
	}

	// As our list is circular, it's either full nullptr or non nullptr
	assert((page._prevPage == nullptr) == (page._nextPage == nullptr));

	// If the page was in the doubly chained list, unhook it
	if (page._prevPage) {
		// On a fully generic circular linked list, if we were the only item
		// we would have to clear _lastUsedPage.
		// But if we are alone, we are at the end of the list and we were handled above
		page._prevPage->_nextPage = page._nextPage;
		page._nextPage->_prevPage = page._prevPage;
		page._prevPage = nullptr;
		page._nextPage = nullptr;
	}

	// Now hook it at the end
	if (!_lastUsedPage) {
		// Empty list
		page._prevPage = &page;
		page._nextPage = &page;
	} else {
		page._nextPage = _lastUsedPage->_nextPage;
		page._prevPage = _lastUsedPage;
		_lastUsedPage->_nextPage->_prevPage = &page;
		_lastUsedPage->_nextPage = &page;
	}
	_lastUsedPage = &page;

	// Don't cleanup every time, only when allocating new pages
	if (newPage) {
		cleanupOutdatedPages();
	}
}

void SliceAnimations::cleanupOutdatedPages() {
	if (!_lastUsedPage) {
		return;
	}

	// If we are too young, no cleanup to do
	// The counter will wrap in approx. 49 days
	if (_lastUsedPage->_lastAccess < 60000) {
		return;
	}

	// Keep all pages used in the last 60s
	uint32 deadline = _lastUsedPage->_lastAccess - 60000;

	// _lastUsedPage->_nextNode is the oldest page in the list
	Page *page = _lastUsedPage->_nextPage;
	while(page != _lastUsedPage) {
		if (page->_lastAccess >= deadline) {
			break;
		}
		Page *next = page->_nextPage;

		// As we delete from the start of the list,
		// the previous page is always the end of the list
		_lastUsedPage->_nextPage = next;
		next->_prevPage = _lastUsedPage;

		free(page->_data);
		page->_data = nullptr;
		page->_lastAccess = 0;
		page->_prevPage = nullptr;
		page->_nextPage = nullptr;

		page = next;
	}
}

Vector3 SliceAnimations::getPositionChange(int animation) const {
	return _animations[animation].positionChange;
}

float SliceAnimations::getFacingChange(int animation) const {
	return _animations[animation].facingChange;
}

} // End of namespace BladeRunner
