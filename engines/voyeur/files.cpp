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

#include "voyeur/files.h"
#include "voyeur/graphics.h"
#include "voyeur/voyeur.h"

namespace Voyeur {

#define BOLT_GROUP_SIZE 16

BoltFilesState::BoltFilesState() {
	_curLibPtr = NULL;
	_curGroupPtr = NULL;
	_curMemberPtr = NULL;
	_curMemInfoPtr = NULL;
	_fromGroupFlag = 0;
	_xorMask = 0;
	_encrypt = false;
	_curFilePosition = 0;
	_bufferEnd = 0;
	_bufferBegin = 0;
	_bytesLeft = 0;
	_bufSize = 0;
	_bufStart = NULL;
	_bufPos = NULL;
	_historyIndex = 0;
	_runLength = 0;
	_decompState = 0;
	_runType = 0;
	_runValue = 0;
	_runOffset = 0;
	Common::fill(&_historyBuffer[0], &_historyBuffer[0x200], 0);
	_curFd = NULL;
	_boltPageFrame = NULL;
}

#define NEXT_BYTE if (--_bytesLeft <= 0) nextBlock()

byte *BoltFilesState::decompress(byte *buf, int size, int mode) {
	if (!buf)
		buf = new byte[size];
	byte *bufP = buf;

	if (mode & 8) {
		_decompState = 1;
		_runType = 0;
		_runLength = size;
	}

	while (size > 0) {
		if (!_decompState) {
			NEXT_BYTE;
			byte nextByte = *_bufPos++;

			switch (nextByte & 0xC0) {
			case 0:
				_runType = 0;
				_runLength = 30 - (nextByte & 0x1f) + 1;
				break;
			case 0x40:
				_runType = 1;
				_runLength = 35 - (nextByte & 0x1f);
				NEXT_BYTE;
				_runOffset = *_bufPos++ + ((nextByte & 0x20) << 3);
				break;
			case 0x80:
				_runType = 1;
				_runLength = (nextByte & 0x20) ? ((32 - (nextByte & 0x1f)) << 2) + 2 :
					(32 - (nextByte & 0x1f)) << 2;
				NEXT_BYTE;
				_runOffset = *_bufPos++ << 1;
				break;
			default:
				_runType = 2;

				if (nextByte & 0x20) {
					_runLength = 0;
				} else {
					NEXT_BYTE;
					_runLength = ((32 - (nextByte & 0x1f)) + (*_bufPos++ << 5)) << 2;
					NEXT_BYTE;
					_bufPos++;
					NEXT_BYTE;
					_runValue = *_bufPos++;
				}
				break;
			}

			_runOffset = _historyIndex - _runOffset;
		}

		int runOffset = _runOffset & 0x1ff;
		int len;
		if (_runLength <= size) {
			len = _runLength;
			_decompState = 0;
		} else {
			_decompState = 1;
			_runLength = len = size;
			if (_runType == 1)
				_runOffset += len;
		}

		// Reduce the remaining size
		size -= len;

		// Handle the run lengths
		switch (_runType) {
		case 0:
			while (len-- > 0) {
				NEXT_BYTE;
				byte v = *_bufPos++;
				_historyBuffer[_historyIndex] = v;
				*bufP++ = v;
				_historyIndex = (_historyIndex + 1) & 0x1ff;
			}
			break;
		case 1:
			while (len-- > 0) {
				_historyBuffer[_historyIndex] = _historyBuffer[runOffset];
				*bufP++ = _historyBuffer[runOffset];
				_historyIndex = (_historyIndex + 1) & 0x1ff;
				runOffset = (runOffset + 1) & 0x1ff;
			}
			break;
		default:
			while (len-- > 0) {
				_historyBuffer[_historyIndex] = _runValue;
				*bufP++ = _runValue;
				_historyIndex = (_historyIndex + 1) & 0x1ff;
			}
			break;
		}
	}

	return buf;
}

#undef NEXT_BYTE

void BoltFilesState::nextBlock() {
	if (_curFilePosition != _bufferEnd)
		_curFd->seek(_bufferEnd);

	_bufferBegin = _bufferEnd;
	int bytesRead = _curFd->read(_bufStart, _bufSize);

	_bufferEnd = _curFilePosition = _bufferBegin + bytesRead;
	_bytesLeft = bytesRead - 1;
	_bufPos = _bufStart;
}

/*------------------------------------------------------------------------*/

FilesManager::FilesManager() {
	_decompressSize = 0x7000;
}

bool FilesManager::openBoltLib(const Common::String &filename, BoltFile *&boltFile) {
	if (boltFile != NULL) {
		_boltFilesState._curLibPtr = boltFile;
		return true;
	}

	// TODO: Specific library classes for buoy.blt versus stampblt.blt 
	// Create the bolt file interface object and load the index
	boltFile = _boltFilesState._curLibPtr = new BoltFile(_boltFilesState);
	return true;
}

/*------------------------------------------------------------------------*/

const BoltMethodPtr BoltFile::_fnInitType[25] = {
	&BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initDefault,
	&BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initDefault,
	&BoltFile::sInitPic, &BoltFile::initDefault, &BoltFile::vInitCMap, &BoltFile::vInitCycl,
	&BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initViewPort,
	&BoltFile::initViewPortList, &BoltFile::initDefault, &BoltFile::initFontInfo,
	&BoltFile::initSoundMap, &BoltFile::initDefault, &BoltFile::initDefault, &BoltFile::initDefault,
	&BoltFile::initDefault, &BoltFile::initDefault
};

BoltFile::BoltFile(BoltFilesState &state): _state(state) {
	_state._curFd = &_file;
	if (!_file.open("bvoy.blt"))
		error("Could not open buoy.blt");
	_state._curFilePosition = 0;

	// Read in the file header
	byte header[16];
	_file.read(&header[0], 16);

	if (strncmp((const char *)&header[0], "BOLT", 4) != 0)
		error("Tried to load non-bolt file");

	int totalGroups = header[11] ? header[11] : 0x100;
	for (int i = 0; i < totalGroups; ++i)
		_groups.push_back(BoltGroup(_state._curFd));
}

BoltFile::~BoltFile() {
	_state._curFd->close();
}

bool BoltFile::getBoltGroup(uint32 id) {
	++_state._fromGroupFlag;
	_state._curLibPtr = this;
	_state._curGroupPtr = &_groups[(id >> 8) & 0xff];

	if (!_state._curGroupPtr->_loaded) {
		// Load the group index
		_state._curGroupPtr->load();
	}

	if (_state._curGroupPtr->_callInitGro)
		initGro();

	if ((id >> 16) != 0) {
		id &= 0xff00;
		for (int idx = 0; idx < _state._curGroupPtr->_count; ++idx, ++id) {
			byte *member = getBoltMember(id);
			assert(member);
		}
	} else if (!_state._curGroupPtr->_processed) {
		_state._curGroupPtr->_processed = true;
		_state._curGroupPtr->load();
	}

	resolveAll();
	--_state._fromGroupFlag;
	return true;
}

void BoltFile::freeBoltGroup(uint32 id) {
	warning("TODO: freeBoltGroup");
}

BoltEntry &BoltFile::getBoltEntry(uint32 id) {
	BoltGroup &group = _groups[id >> 24];
	assert(group._loaded);

	BoltEntry &entry = group._entries[(id >> 16) & 0xff];
	assert(!entry.hasResource() || (id & 0xffff) == 0);

	return entry;
}

PictureResource *BoltFile::getPictureResouce(uint32 id) {
	if ((int32)id == -1)
		return NULL;

	return getBoltEntry(id)._picResource;
}

CMapResource *BoltFile::getCMapResource(uint32 id) {
		if ((int32)id == -1)
		return NULL;

	return getBoltEntry(id)._cMapResource;
}

byte *BoltFile::memberAddr(uint32 id) {
	BoltGroup &group = _groups[id >> 8];
	if (!group._loaded)
		return NULL;

	// If an entry already has a processed representation, we shouldn't
	// still be accessing the raw data
	BoltEntry &entry = group._entries[id & 0xff];
	assert(!entry.hasResource());

	return entry._data;
}

byte *BoltFile::memberAddrOffset(uint32 id) {
	BoltGroup &group = _groups[id >> 24];
	if (!group._loaded)
		return NULL;

	// If an entry already has a processed representation, we shouldn't
	// still be accessing the raw data
	BoltEntry &entry = group._entries[(id >> 16) & 0xff];
	assert(!entry.hasResource());

	return entry._data + (id & 0xffff);
}

/**
 * Resolves an Id to an offset within a loaded resource
 */
void BoltFile::resolveIt(uint32 id, byte **p) {
	if ((int32)id == -1) {
		*p = NULL;
	} else {
		byte *ptr = memberAddrOffset(id);
		if (ptr) {
			*p = ptr;
		} else {
			*p = NULL;
			assert(_state._resolves.size() < 1000);
			_state._resolves.push_back(ResolveEntry(id, p));
		}
	}
}

void BoltFile::resolveFunction(uint32 id, GraphicMethodPtr *fn) {
	if ((int32)id == -1) {
		*fn = NULL;
	} else {
		error("Function fnTermGro array not supported");
	}
}

/**
 * Resolve any data references to within resources that weren't
 * previously loaded, but are now
 */
void BoltFile::resolveAll() {
	for (uint idx = 0; idx < _state._resolves.size(); ++idx)
		*_state._resolves[idx]._p = memberAddrOffset(_state._resolves[idx]._id);
}

byte *BoltFile::getBoltMember(uint32 id) {
	_state._curLibPtr = this;

	// Get the group, and load it's entry list if not already loaded
	_state._curGroupPtr = &_groups[(id >> 8) & 0xff];
	if (!_state._curGroupPtr->_loaded)
		_state._curGroupPtr->load();

	// Get the entry
	_state._curMemberPtr = &_state._curGroupPtr->_entries[id & 0xff];
	if (_state._curMemberPtr->_field1)
		initMem(_state._curMemberPtr->_field1);

	// Return the data for the entry if it's already been loaded
	if (_state._curMemberPtr->_data)
		return _state._curMemberPtr->_data;

	_state._xorMask = _state._curMemberPtr->_xorMask;
	_state._encrypt = (_state._curMemberPtr->_mode & 0x10) != 0;

	if (_state._curGroupPtr->_processed) {
		// TODO: Figure out weird access type. Uncompressed read perhaps?
		//int fileDiff = _state._curGroupPtr->_fileOffset - _state._curMemberPtr->_fileOffset;

	} else {
		_state._bufStart = _state._decompressBuf;
		_state._bufSize = DECOMPRESS_SIZE;

		if ((_state._curFd != &_file) || (_state._curMemberPtr->_fileOffset < _state._bufferBegin)
				|| (_state._curMemberPtr->_fileOffset >= _state._bufferEnd)) {
			_state._bytesLeft = 0;
			_state._bufPos = _state._bufStart;
			_state._bufferBegin = -1;
			_state._bufferEnd = _state._curMemberPtr->_fileOffset;
		} else {
			_state._bufPos = _state._curMemberPtr->_fileOffset - _state._bufferBegin + _state._bufStart;
			_state._bytesLeft = _state._bufSize - (_state._bufPos - _state._bufStart);
		}
	}

	_state._decompState = 0;
	_state._historyIndex = 0;

	// Initialise the resource
	assert(_state._curMemberPtr->_initMethod < 25);
	(this->*_fnInitType[_state._curMemberPtr->_initMethod])();

	return _state._curMemberPtr->_data;
}

void BoltFile::initDefault() {
	_state._curMemberPtr->_data = _state.decompress(0, _state._curMemberPtr->_size, 
		_state._curMemberPtr->_mode);	
}

void BoltFile::sInitPic() {
	// Read in the header data
	_state._curMemberPtr->_data = _state.decompress(NULL, 24, _state._curMemberPtr->_mode);
	_state._curMemberPtr->_picResource = new PictureResource(_state, 
		_state._curMemberPtr->_data);
}

void BoltFile::vInitCMap() {
	initDefault();
	_state._curMemberPtr->_cMapResource = new CMapResource(
		_state, _state._curMemberPtr->_data);
}

void BoltFile::vInitCycl() {
	initDefault();
	_state._vm->_eventsManager.vStopCycle();
	_state._curMemberPtr->_vInitCyclResource = new VInitCyclResource(
		_state, _state._curMemberPtr->_data);
}

void BoltFile::initViewPort() {
	initDefault();
	_state._curMemberPtr->_viewPortResource = new ViewPortResource(
		_state, _state._curMemberPtr->_data);
}

void BoltFile::initViewPortList() {
	initDefault();

	ViewPortListResource *res;
	_state._curMemberPtr->_viewPortListResource = res = new ViewPortListResource(
		_state, _state._curMemberPtr->_data);

	_state._vm->_graphicsManager._viewPortListPtr = res;
	_state._vm->_graphicsManager._vPort = &res->_entries[0];
}

void BoltFile::initFontInfo() {
	initDefault();
	_state._curMemberPtr->_fontResource = new FontResource(
		_state, _state._curMemberPtr->_data);
}

void BoltFile::initSoundMap() {
	initDefault();
}

/*------------------------------------------------------------------------*/

BoltGroup::BoltGroup(Common::SeekableReadStream *f): _file(f) {
	byte buffer[BOLT_GROUP_SIZE];

	_loaded = false;

	_file->read(&buffer[0], BOLT_GROUP_SIZE);
	_processed = buffer[0] != 0;
	_callInitGro = buffer[1] != 0;
	_termGroIndex = buffer[2];
	_count = buffer[3] ? buffer[3] : 256;	// TODO: Added this in. Check it's okay
	_fileOffset = READ_LE_UINT32(&buffer[8]);
}

void BoltGroup::load() {
	_file->seek(_fileOffset);

	// Read the entries
	for (int i = 0; i < _count; ++i)
		_entries.push_back(BoltEntry(_file));

	_loaded = true;
}

/*------------------------------------------------------------------------*/

BoltEntry::BoltEntry(Common::SeekableReadStream *f): _file(f) {
	_data = NULL;
	_picResource = NULL;
	_viewPortResource = NULL;
	_viewPortListResource = NULL;
	_fontResource = NULL;
	_cMapResource = NULL;
	_vInitCyclResource = NULL;

	byte buffer[16];
	_file->read(&buffer[0], 16);
	_mode = buffer[0];
	_field1 = buffer[1];
	_initMethod = buffer[3];
	_xorMask = buffer[4] & 0xff;	// TODO: Is this right??
	_size = READ_LE_UINT32(&buffer[4]);
	_fileOffset = READ_LE_UINT32(&buffer[8]); 
}

BoltEntry::~BoltEntry() {
	delete[] _data;
	delete _picResource;
	delete _viewPortResource;
	delete _viewPortListResource;
	delete _fontResource;
	delete _cMapResource;
	delete _vInitCyclResource;
}

void BoltEntry::load() {
	// TODO: Currently, all entry loading and decompression is done in BoltFile::memberAddr.
	// Ideally, a lot of the code should be moved here
}

/**
 * Returns true if the given bolt entry has an attached resource
 */
bool BoltEntry::hasResource() const {
	return _picResource || _viewPortResource || _viewPortListResource
		|| _fontResource || _cMapResource || _vInitCyclResource;
}

/*------------------------------------------------------------------------*/

PictureResource::PictureResource(BoltFilesState &state, const byte *src) {
	_flags = READ_LE_UINT16(src);
	_select = src[2];
	_pick = src[3];
	_onOff = src[4];
	_depth = src[5];

	int xs = READ_LE_UINT16(&src[6]);
	int ys = READ_LE_UINT16(&src[8]);
	_bounds = Common::Rect(xs, ys, xs + READ_LE_UINT16(&src[10]),
		ys + READ_LE_UINT16(&src[12]));
	_maskData = READ_LE_UINT32(&src[14]);

	_imgData = NULL;

	int nbytes = _bounds.width() * _bounds.height();
	if (_flags & 0x20) {
		error("TODO: sInitPic flags&0x20");
	} else if (_flags & 8) {
		int mode = 0;
		if (_bounds.width() == 320) {
			mode = 147;
			state._sImageShift = 2;
			state._SVGAReset = false;
		} else {
			state._SVGAReset = true;
			if (_bounds.width() == 640) {
				if (_bounds.height() == 400) {
					mode = 220;
					state._sImageShift = 3;
				} else {
					mode = 221;
					state._sImageShift = 3;
				}
			} else if (_bounds.width() == 800) {
				mode = 222;
				state._sImageShift = 3;
			} else if (_bounds.width() == 1024) {
				mode = 226;
				state._sImageShift = 3;
			}
		}

		if (mode != state._vm->_graphicsManager._SVGAMode) {
			state._vm->_graphicsManager._SVGAMode = mode;
			state._vm->_graphicsManager.clearPalette();			
		}

//		byte *imgData = _imgData;
		if (_flags & 0x10) {
			// TODO: Figure out what it's doing. Looks like a direct clearing
			// of the screen directly
		} else {
			// TODO: Figure out direct screen loading
		}
	} else {
		if (_flags & 0x1000) {
			if (!(_flags & 0x10))
				nbytes = state._curMemberPtr->_size - 24;

			int mask = (nbytes + 0x3FFF) >> 14;
			_imgData = NULL;

			if (state._boltPageFrame == 0)
				state.EMSGetFrameAddr(&state._boltPageFrame);
			if (state._boltPageFrame != 0) {
				if (!state.EMSAllocatePages(&_planeSize)) {
					_maskData = mask;

					for (int idx = 0; idx < mask; ++idx)
						state.EMSMapPageHandle(_planeSize, idx, idx);
					
					state.decompress(state._boltPageFrame, nbytes, state._curMemberPtr->_mode);
					return;
				}
			}
		}

		if (_flags & 0x10) {
			_imgData = new byte[nbytes];
		} else {
			_imgData = state.decompress(NULL, nbytes, state._curMemberPtr->_mode);			
		}
	}
}

PictureResource::~PictureResource() {
	delete _imgData;
}

/*------------------------------------------------------------------------*/

ViewPortResource::ViewPortResource(BoltFilesState &state, const byte *src):
		_state(state) {
	_flags = READ_LE_UINT16(src);
	_next = state._curLibPtr->getBoltEntry(READ_LE_UINT32(src + 2))._viewPortResource;
	_pageCount = READ_LE_UINT16(src + 6);
	_pageIndex = READ_LE_UINT16(src + 8);
	_lastPage = READ_LE_UINT16(src + 10);

	int xs = READ_LE_UINT16(src + 12);
	int ys = READ_LE_UINT16(src + 14);
	_bounds = Common::Rect(xs, ys, xs + READ_LE_UINT16(src + 16), 
		ys + READ_LE_UINT16(src + 18));
	_field18 = READ_LE_UINT16(src + 0x18);

	_currentPic = state._curLibPtr->getPictureResouce(READ_LE_UINT32(src + 0x20));
	_activePage = state._curLibPtr->getPictureResouce(READ_LE_UINT32(src + 0x24));
	_pages[0] = state._curLibPtr->getPictureResouce(READ_LE_UINT32(src + 0x28));
	_pages[1] = state._curLibPtr->getPictureResouce(READ_LE_UINT32(src + 0x2C));

	state._curLibPtr->resolveIt(READ_LE_UINT32(src + 0x30), &_field30);

	// Get the rect list
	for (int listIndex = 0; listIndex < 3; ++listIndex) {
		_rectListCount[listIndex] = (int16)READ_LE_UINT16(src + 0x40 + 2 * listIndex);
		uint32 id = READ_LE_UINT32(src + 0x34 + listIndex * 4);

		if (id == -1) {
			_rectListPtr[listIndex] = NULL;
		} else {
			_rectListPtr[listIndex] = new Common::Array<Common::Rect>();

			if (_rectListCount[listIndex] > 0) {
				int16 *rectList = (int16 *)state._curLibPtr->memberAddrOffset(id);
				for (int i = 0; i < _rectListCount[listIndex]; ++i) {
					int xs = FROM_LE_16(rectList[0]);
					int ys = FROM_LE_16(rectList[1]);
					_rectListPtr[i]->push_back(Common::Rect(xs, ys, xs + FROM_LE_16(rectList[2]),
						ys + FROM_LE_16(rectList[3])));
				}
			}
		}
	}

	xs = READ_LE_UINT16(src + 0x46);
	ys = READ_LE_UINT16(src + 0x48);
	_clipRect = Common::Rect(xs, ys, xs + READ_LE_UINT16(src + 0x4A),
		ys + READ_LE_UINT16(src + 0x4C));

	state._curLibPtr->resolveIt(READ_LE_UINT32(src + 0x7A), &_field7A);

	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x7E), (GraphicMethodPtr *)&_fn1);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x82), (GraphicMethodPtr *)&_setupFn);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x86), (GraphicMethodPtr *)&_addFn);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x8A), (GraphicMethodPtr *)&_restoreFn);

	if (!_restoreFn && _addFn)
		_addFn = &GraphicsManager::addRectNoSaveBack;
}

ViewPortResource::~ViewPortResource() {
	for (int i = 0; i < 3; ++i)
		delete _rectListPtr[i];
}

void ViewPortResource::setupViewPort(PictureResource *page, Common::Rect *clipRect,
		ViewPortSetupPtr setupFn, ViewPortAddPtr addFn, ViewPortRestorePtr restoreFn) {
	PictureResource *pic = _currentPic;
	Common::Rect r = _bounds;
	r.translate(pic->_bounds.left, pic->_bounds.top);
	int xDiff, yDiff;

	if (page) {
		// Clip based on the passed picture resource
		xDiff = page->_bounds.left - r.left;
		yDiff = page->_bounds.top - r.top;

		if (xDiff > 0) {
			int width = r.width();
			r.left = page->_bounds.left;
			r.setWidth(xDiff <= width ? width - xDiff : 0);
		}
		if (yDiff > 0) {
			int height = r.height();
			r.top = page->_bounds.top;
			r.setHeight(yDiff <= height ? height - yDiff : 0);
		}

		xDiff = r.right - page->_bounds.right;
		yDiff = r.bottom - page->_bounds.bottom;

		if (xDiff > 0)
			r.setWidth(xDiff <= r.width() ? r.width() - xDiff : 0); 
		if (yDiff > 0)
			r.setHeight(yDiff <= r.height() ? r.height() - yDiff : 0);
	}

	if (clipRect) {
		// Clip based on the passed clip rectangles
		xDiff = clipRect->left - r.left;
		yDiff = clipRect->top - r.top;

		if (xDiff > 0) {
			int width = r.width();
			r.left = clipRect->left;
			r.setWidth(xDiff <= width ? width - xDiff : 0);
		}
		if (yDiff > 0) {
			int height = r.height();
			r.top = clipRect->top;
			r.setHeight(yDiff <= height ? height - yDiff : 0);
		}
		
		xDiff = r.right - clipRect->right;
		yDiff = r.right - clipRect->right;

		if (xDiff > 0)
			r.setWidth(xDiff <= r.width() ? r.width() - xDiff : 0);
		if (yDiff > 0)
			r.setHeight(yDiff <= r.height() ? r.height() - yDiff : 0);
	}
// clip = (0x20, 0x14, width: 0x140, height: 0C8h
	_activePage = page;
	_field18 = 0;
	_clipRect = r;
	_setupFn = setupFn;
	_addFn = addFn;
	_restoreFn = restoreFn;

	if (setupFn)
		(_state._vm->_graphicsManager.*setupFn)(this);
}

void ViewPortResource::setupViewPort() {
	setupViewPort(_state._vm->_graphicsManager._backgroundPage, NULL,
		&GraphicsManager::setupMCGASaveRect, &GraphicsManager::addRectOptSaveRect,
		&GraphicsManager::restoreMCGASaveRect);
}

/*------------------------------------------------------------------------*/

ViewPortListResource::ViewPortListResource(BoltFilesState &state, const byte *src) {
	uint count = READ_LE_UINT16(src);

	uint32 *idP = (uint32 *)&src[8];

	for (uint i = 0; i < count; ++i, ++idP) {
		uint32 id = READ_LE_UINT32(idP);
		BoltEntry &entry = state._curLibPtr->getBoltEntry(id);

		assert(entry._viewPortResource);
		_entries.push_back(entry._viewPortResource);
	}

	state._curLibPtr->resolveIt(READ_LE_UINT32(src + 4), &_palette);
}

/*------------------------------------------------------------------------*/

FontResource::FontResource(BoltFilesState &state, const byte *src) {
	state._curLibPtr->resolveIt(READ_LE_UINT32(src + 0xC), &_fieldC);
}

/*------------------------------------------------------------------------*/

CMapResource::CMapResource(BoltFilesState &state, const byte *src): _vm(state._vm) {
	_steps = READ_LE_UINT16(src);
	_start = READ_LE_UINT16(src + 2);
	_end = READ_LE_UINT16(src + 4);
	
	int count = _end - _start;
	_entries = new byte[count * 3];
	Common::copy(src + 6, src + 6 + 3 * count, _entries);
}

CMapResource::~CMapResource() {
	delete[] _entries;
}

void CMapResource::startFade() {
	_vm->_eventsManager.startFade(this);
}

/*------------------------------------------------------------------------*/

VInitCyclResource::VInitCyclResource(BoltFilesState &state, const byte *src) {
	for (int i = 0; i < 4; ++i) {
		state._curLibPtr->resolveIt(READ_LE_UINT32(src + 8 + i * 4), &_ptr[i]);
	}
}

} // End of namespace Voyeur
