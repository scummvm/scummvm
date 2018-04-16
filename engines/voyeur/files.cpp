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

#include "voyeur/files.h"
#include "voyeur/screen.h"
#include "voyeur/voyeur.h"
#include "voyeur/staticres.h"

namespace Voyeur {

#define BOLT_GROUP_SIZE 16

BoltFilesState::BoltFilesState(VoyeurEngine *vm) : _vm(vm) {
	_curLibPtr = NULL;
	_curGroupPtr = NULL;
	_curMemberPtr = NULL;
	_bufferEnd = 0;
	_bufferBegin = 0;
	_bytesLeft = 0;
	_bufSize = 0;
	_bufStart = NULL;
	_bufPos = NULL;
	_historyIndex = 0;
	_runLength = 0;
	_decompState = false;
	_runType = 0;
	_runValue = 0;
	_runOffset = 0;
	Common::fill(&_historyBuffer[0], &_historyBuffer[0x200], 0);
	_curFd = NULL;
	_boltPageFrame = NULL;
}

#define NEXT_BYTE if (--_bytesLeft < 0) nextBlock()

byte *BoltFilesState::decompress(byte *buf, int size, int mode) {
	if (!buf) {
		buf = new byte[size];
		Common::fill(buf, buf + size, 0);
	}
	byte *bufP = buf;

	if (mode & 8) {
		_decompState = true;
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
			_decompState = false;
		} else {
			_decompState = true;
			len = size;
			_runLength -= size;
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
	if (&_curLibPtr->_file != _curFd || _curFd->pos() != _bufferEnd)
		_curLibPtr->_file.seek(_bufferEnd);

	_curFd = &_curLibPtr->_file;
	_bufferBegin = _bufferEnd;
	int bytesRead = _curFd->read(_bufStart, _bufSize);

	_bufferEnd = _curFd->pos();
	_bytesLeft = bytesRead - 1;
	_bufPos = _bufStart;
}

/*------------------------------------------------------------------------*/

FilesManager::FilesManager(VoyeurEngine *vm) {
	_curLibPtr = nullptr;
	_boltFilesState = new BoltFilesState(vm);
}

FilesManager::~FilesManager() {
	delete _boltFilesState;
}

bool FilesManager::openBoltLib(const Common::String &filename, BoltFile *&boltFile) {
	if (boltFile != NULL) {
		_boltFilesState->_curLibPtr = boltFile;
		return true;
	}

	// Create the bolt file interface object and load the index
	if (filename == "bvoy.blt")
		boltFile = _boltFilesState->_curLibPtr = new BVoyBoltFile(*_boltFilesState);
	else if (filename == "stampblt.blt")
		boltFile = _boltFilesState->_curLibPtr = new StampBoltFile(*_boltFilesState);
	else
		error("Unknown bolt file specified");

	return true;
}

byte *FilesManager::fload(const Common::String &filename, int *size) {
	Common::File f;
	int filesize;
	byte *data = NULL;

	if (f.open(filename)) {
		// Read in the file
		filesize = f.size();
		data = new byte[filesize];
		f.read(data, filesize);
	} else {
		filesize = 0;
	}

	if (size)
		*size = filesize;
	return data;
}

/*------------------------------------------------------------------------*/

BoltFile::BoltFile(const Common::String &filename, BoltFilesState &state): _state(state) {
	if (!_file.open(filename))
		error("Could not open %s", filename.c_str());

	// Read in the file header
	byte header[16];
	_file.read(&header[0], 16);

	if (strncmp((const char *)&header[0], "BOLT", 4) != 0)
		error("Tried to load non-bolt file");

	int totalGroups = header[11] ? header[11] : 0x100;
	for (int i = 0; i < totalGroups; ++i)
		_groups.push_back(BoltGroup(&_file));
}

BoltFile::~BoltFile() {
	_file.close();
	if (_state._curFd == &_file)
		_state._curFd = NULL;
	if (_state._curLibPtr == this)
		_state._curLibPtr = NULL;
}

BoltGroup *BoltFile::getBoltGroup(uint16 id) {
	_state._curLibPtr = this;
	_state._curGroupPtr = &_groups[(id >> 8) & 0xff];

	if (!_state._curGroupPtr->_loaded) {
		// Load the group index
		_state._curGroupPtr->load(id & 0xff00);
	}

	// Pre-process the resources
	id &= 0xff00;
	for (int idx = 0; idx < _state._curGroupPtr->_count; ++idx, ++id) {
		byte *member = getBoltMember(id);
		assert(member);
	}

	resolveAll();

	return _state._curGroupPtr;
}

void BoltFile::freeBoltGroup(uint16 id) {
	_state._curLibPtr = this;
	_state._curGroupPtr = &_groups[(id >> 8) & 0xff];

	// Unload the group
	_state._curGroupPtr->unload();
}

void BoltFile::freeBoltMember(uint32 id) {
	// No implementation in ScummVM
}

BoltEntry &BoltFile::getBoltEntryFromLong(uint32 id) {
	BoltGroup &group = _groups[id >> 24];
	assert(group._loaded);

	BoltEntry &entry = group._entries[(id >> 16) & 0xff];
	assert(!entry.hasResource() || (id & 0xffff) == 0);

	return entry;
}

BoltEntry &BoltFile::boltEntry(uint16 id) {
	BoltGroup &group = _groups[id >> 8];
	assert(group._loaded);

	BoltEntry &entry = group._entries[id & 0xff];
	assert(entry.hasResource());

	return entry;
}

PictureResource *BoltFile::getPictureResource(uint32 id) {
	if ((int32)id == -1)
		return NULL;

	if (id & 0xffff)
		id <<= 16;
	return getBoltEntryFromLong(id)._picResource;
}

CMapResource *BoltFile::getCMapResource(uint32 id) {
	if ((int32)id == -1)
		return NULL;

	if (id & 0xffff)
		id <<= 16;

	return getBoltEntryFromLong(id)._cMapResource;
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

void BoltFile::resolveFunction(uint32 id, ScreenMethodPtr *fn) {
	if ((int32)id == -1)
		*fn = NULL;
	else
		error("Function fnTermGro array not supported");
}

/**
 * Resolve any data references to within resources that weren't
 * previously loaded, but are now
 */
void BoltFile::resolveAll() {
	for (uint idx = 0; idx < _state._resolves.size(); ++idx)
		*_state._resolves[idx]._p = memberAddrOffset(_state._resolves[idx]._id);

	_state._resolves.clear();
}

byte *BoltFile::getBoltMember(uint32 id) {
	_state._curLibPtr = this;

	// Get the group, and load it's entry list if not already loaded
	_state._curGroupPtr = &_groups[(id >> 8) & 0xff];
	if (!_state._curGroupPtr->_loaded)
		_state._curGroupPtr->load(id & 0xff00);

	// Get the entry
	_state._curMemberPtr = &_state._curGroupPtr->_entries[id & 0xff];

	// Return the data for the entry if it's already been loaded
	if (_state._curMemberPtr->_data)
		return _state._curMemberPtr->_data;

	if (_state._curGroupPtr->_processed) {
		error("Processed resources are not supported");
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

	_state._decompState = false;
	_state._historyIndex = 0;

	// Initialize the resource
	assert(_state._curMemberPtr->_initMethod < 25);
	initResource(_state._curMemberPtr->_initMethod);

	return _state._curMemberPtr->_data;
}

void BoltFile::initDefault() {
	_state._curMemberPtr->_data = _state.decompress(NULL, _state._curMemberPtr->_size,
		_state._curMemberPtr->_mode);
}

/*------------------------------------------------------------------------*/

BVoyBoltFile::BVoyBoltFile(BoltFilesState &state): BoltFile("bvoy.blt", state) {
}

void BVoyBoltFile::initResource(int resType) {
	switch (resType) {
	case 2:
		// Also used for point list, and ending credits credit data
		sInitRect();
		break;
	case 8:
		sInitPic();
		break;
	case 10:
		vInitCMap();
		break;
	case 11:
		vInitCycl();
		break;
	case 15:
		initViewPort();
		break;
	case 16:
		initViewPortList();
		break;
	case 17:
		initFont();
		break;
	case 18:
		initFontInfo();
		break;
	case 19:
		initSoundMap();
		break;
	default:
		initDefault();
		break;
	}
}

void BVoyBoltFile::initViewPort() {
	initDefault();

	ViewPortResource *viewPort;
	byte *src = _state._curMemberPtr->_data;
	_state._curMemberPtr->_viewPortResource = viewPort = new ViewPortResource(_state, src);

	// This is done post-constructor, since viewports can be self referential, so
	// we need the _viewPortResource field to have been set before resolving the pointer
	viewPort->_parent = getBoltEntryFromLong(READ_LE_UINT32(src + 2))._viewPortResource;
}

void BVoyBoltFile::initViewPortList() {
	initDefault();

	ViewPortListResource *res;
	_state._curMemberPtr->_viewPortListResource = res = new ViewPortListResource(
		_state, _state._curMemberPtr->_data);

	_state._vm->_screen->_viewPortListPtr = res;
	_state._vm->_screen->_vPort = res->_entries[0];
}

void BVoyBoltFile::initFontInfo() {
	initDefault();
	_state._curMemberPtr->_fontInfoResource = new FontInfoResource(
		_state, _state._curMemberPtr->_data);
}

void BVoyBoltFile::initFont() {
	initDefault();
	_state._curMemberPtr->_fontResource = new FontResource(_state, _state._curMemberPtr->_data);
}

void BVoyBoltFile::initSoundMap() {
	initDefault();
}

void BVoyBoltFile::sInitRect() {
	_state._curMemberPtr->_data = _state.decompress(NULL, _state._curMemberPtr->_size,
		_state._curMemberPtr->_mode);

	// Check whether the resource Id is in the list of extended rects
	bool isExtendedRects = false;
	for (int i = 0; i < 49 && !isExtendedRects; ++i)
		isExtendedRects = RESOLVE_TABLE[i] == (_state._curMemberPtr->_id & 0xff00);

	int rectSize = isExtendedRects ? 12 : 8;
	if ((_state._curMemberPtr->_size % rectSize) == 0 || (_state._curMemberPtr->_size % rectSize) == 2)
		_state._curMemberPtr->_rectResource = new RectResource(_state._curMemberPtr->_data,
			_state._curMemberPtr->_size, isExtendedRects);
}

void BVoyBoltFile::sInitPic() {
	// Read in the header data
	_state._curMemberPtr->_data = _state.decompress(NULL, 24, _state._curMemberPtr->_mode);
	_state._curMemberPtr->_picResource = new PictureResource(_state,
		_state._curMemberPtr->_data);
}

void BVoyBoltFile::vInitCMap() {
	initDefault();
	_state._curMemberPtr->_cMapResource = new CMapResource(
		_state, _state._curMemberPtr->_data);
}

void BVoyBoltFile::vInitCycl() {
	initDefault();
	_state._curMemberPtr->_vInitCycleResource = new VInitCycleResource(
		_state, _state._curMemberPtr->_data);
	_state._curMemberPtr->_vInitCycleResource->vStopCycle();
}

/*------------------------------------------------------------------------*/

StampBoltFile::StampBoltFile(BoltFilesState &state): BoltFile("stampblt.blt", state) {
}

void StampBoltFile::initResource(int resType) {
	switch (resType) {
	case 0:
		initThread();
		break;
	case 4:
		initState();
		break;
	case 6:
		initPtr();
		break;
	case 24:
		initControl();
		break;
	default:
		initDefault();
		break;
	}
}

void StampBoltFile::initThread() {
	initDefault();

	_state._curMemberPtr->_threadResource = new ThreadResource(_state,
		_state._curMemberPtr->_data);
}

void StampBoltFile::initPtr() {
	initDefault();

	_state._curMemberPtr->_ptrResource = new PtrResource(_state,
		_state._curMemberPtr->_data);
}

	void initControlData();


void StampBoltFile::initControl() {
	initDefault();

	ControlResource *res;
	_state._curMemberPtr->_controlResource = res = new ControlResource(_state,
		_state._curMemberPtr->_data);

	_state._vm->_controlGroupPtr = _state._curGroupPtr;
	_state._vm->_controlPtr = res;
}

void StampBoltFile::initState() {
	initDefault();

	assert(_state._curMemberPtr->_size == 16);
	_state._curMemberPtr->_stateResource = new StateResource(_state,
		_state._curMemberPtr->_data);
}

/*------------------------------------------------------------------------*/

BoltGroup::BoltGroup(Common::SeekableReadStream *f): _file(f) {
	byte buffer[BOLT_GROUP_SIZE];

	_loaded = false;

	_file->read(&buffer[0], BOLT_GROUP_SIZE);
	_processed = buffer[0] != 0;
	_count = buffer[3] ? buffer[3] : 256;
	_fileOffset = READ_LE_UINT32(&buffer[8]);
}

BoltGroup::~BoltGroup() {
}

void BoltGroup::load(uint16 groupId) {
	_file->seek(_fileOffset);

	// Read the entries
	for (int i = 0; i < _count; ++i)
		_entries.push_back(BoltEntry(_file, groupId + i));

	_loaded = true;
}

void BoltGroup::unload() {
	if (!_loaded)
		return;

	_entries.clear();
	_loaded = false;
}

/*------------------------------------------------------------------------*/

BoltEntry::BoltEntry(Common::SeekableReadStream *f, uint16 id): _file(f), _id(id) {
	_data = nullptr;
	_rectResource = nullptr;
	_picResource = nullptr;
	_viewPortResource = nullptr;
	_viewPortListResource = nullptr;
	_fontResource = nullptr;
	_fontInfoResource = nullptr;
	_cMapResource = nullptr;
	_vInitCycleResource = nullptr;

	_ptrResource = nullptr;
	_stateResource = nullptr;
	_controlResource = nullptr;
	_vInitCycleResource = nullptr;
	_threadResource = nullptr;

	byte buffer[16];
	_file->read(&buffer[0], 16);
	_mode = buffer[0];
	_initMethod = buffer[3];
	_size = READ_LE_UINT32(&buffer[4]) & 0xffffff;
	_fileOffset = READ_LE_UINT32(&buffer[8]);
}

BoltEntry::~BoltEntry() {
	delete[] _data;
	delete _rectResource;
	delete _picResource;
	delete _viewPortResource;
	delete _viewPortListResource;
	delete _fontResource;
	delete _fontInfoResource;
	delete _cMapResource;

	delete _ptrResource;
	delete _controlResource;
	delete _stateResource;
	delete _vInitCycleResource;
	delete _threadResource;
}

void BoltEntry::load() {
	// Currently, all entry loading and decompression is done in BoltFile::memberAddr.
}

/**
 * Returns true if the given bolt entry has an attached resource
 */
bool BoltEntry::hasResource() const {
	return _rectResource ||  _picResource || _viewPortResource || _viewPortListResource
		|| _fontResource || _fontInfoResource || _cMapResource || _vInitCycleResource
		|| _ptrResource || _controlResource || _stateResource || _threadResource;
}

/*------------------------------------------------------------------------*/

RectEntry::RectEntry(int x1, int y1, int x2, int y2, int arrIndex, int count):
		Common::Rect(x1, y1, x2, y2), _arrIndex(arrIndex), _count(count) {
}

/*------------------------------------------------------------------------*/

RectResource::RectResource(const byte *src, int size, bool isExtendedRects) {
	int count;
	int rectSize = isExtendedRects ? 12 : 8;
	if ((size % rectSize) == 2) {
		count = READ_LE_UINT16(src);
		src += 2;
	} else {
		count = size / rectSize;
	}

	for (int i = 0; i < count; ++i, src += 8) {
		int arrIndex = 0, rectCount = 0;
		if (isExtendedRects) {
			arrIndex = READ_LE_UINT16(src);
			rectCount = READ_LE_UINT16(src + 2);
			src += 4;
		}

		int x1 = READ_LE_UINT16(src);
		int y1 = READ_LE_UINT16(src + 2);
		int x2 = READ_LE_UINT16(src + 4);
		int y2 = READ_LE_UINT16(src + 6);

		_entries.push_back(RectEntry(x1, y1, x2, y2, arrIndex, rectCount));
	}

	left = _entries[0].left;
	top = _entries[0].top;
	right = _entries[0].right;
	bottom = _entries[0].bottom;
}

RectResource::RectResource(int x1, int y1, int x2, int y2) {
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

/*------------------------------------------------------------------------*/

DisplayResource::DisplayResource() {
	_vm = NULL;
	_flags = DISPFLAG_NONE;
}

DisplayResource::DisplayResource(VoyeurEngine *vm) {
	_vm = vm;
	_flags = DISPFLAG_NONE;
}

void DisplayResource::sFillBox(int width, int height) {
	assert(_vm);
	bool saveBack = _vm->_screen->_saveBack;
	_vm->_screen->_saveBack = false;

	PictureResource pr;
	pr._flags = DISPFLAG_1;
	pr._select = 0xff;
	pr._pick = 0;
	pr._onOff = _vm->_screen->_drawPtr->_penColor;
	pr._bounds = Common::Rect(0, 0, width, height);

	_vm->_screen->sDrawPic(&pr, this, _vm->_screen->_drawPtr->_pos);
	_vm->_screen->_saveBack = saveBack;
}

bool DisplayResource::clipRect(Common::Rect &rect) {
	Common::Rect clippingRect;
	if (_vm->_screen->_clipPtr) {
		clippingRect = *_vm->_screen->_clipPtr;
	} else if (_flags & DISPFLAG_VIEWPORT) {
		clippingRect = ((ViewPortResource *)this)->_clipRect;
	} else {
		clippingRect = ((PictureResource *)this)->_bounds;
	}

	Common::Rect r = rect;
	if (r.left < clippingRect.left) {
		if (r.right <= clippingRect.left)
			return false;
		r.setWidth(r.right - clippingRect.left);
	}
	if (r.right >= clippingRect.right) {
		if (r.left >= clippingRect.left)
			return false;
		r.setWidth(clippingRect.right - r.left);
	}

	if (r.top < clippingRect.top) {
		if (r.bottom <= clippingRect.top)
			return false;
		r.setHeight(r.bottom - clippingRect.top);
	}
	if (r.bottom >= clippingRect.bottom) {
		if (r.top >= clippingRect.top)
			return false;
		r.setWidth(clippingRect.bottom - r.top);
	}

	rect = r;
	return true;
}

int DisplayResource::drawText(const Common::String &msg) {
	Screen &screen = *_vm->_screen;
	assert(screen._fontPtr);
	assert(screen._fontPtr->_curFont);
	FontInfoResource &fontInfo = *screen._fontPtr;
	PictureResource &fontChar = *_vm->_screen->_fontChar;
	FontResource &fontData = *fontInfo._curFont;
	int xShadows[9] = { 0, 1, 1, 1, 0, -1, -1, -1, 0 };
	int yShadows[9] = { 0, 1, 0, -1, -1, -1, 0, 1, 1 };

	Common::Rect *clipPtr = screen._clipPtr;
	if (!(fontInfo._picFlags & DISPFLAG_1))
		screen._clipPtr = NULL;

	int minChar = fontData._minChar;
	int padding = fontData._padding;
	int fontHeight = fontData._fontHeight;
	int totalChars = fontData._maxChar - fontData._minChar;
	int msgWidth = 0;
	int xp = 0, yp = 0;

	Common::Point pos = Common::Point(fontInfo._pos.x, fontInfo._pos.y + fontData._topPadding);

	fontChar._flags = fontInfo._picFlags | DISPFLAG_2;
	fontChar._select = fontInfo._picSelect;
	fontChar._bounds.setHeight(fontHeight);

	ViewPortResource *viewPort = !(_flags & DISPFLAG_VIEWPORT) ? NULL :
		(ViewPortResource *)this;

	if ((fontInfo._fontFlags & DISPFLAG_1) || fontInfo._justify ||
			(screen._saveBack && fontInfo._fontSaveBack && (_flags & DISPFLAG_VIEWPORT))) {
		msgWidth = viewPort->textWidth(msg);
		yp = pos.y;
		xp = pos.x;

		switch (fontInfo._justify) {
		case 1:
			xp = pos.x + (fontInfo._justifyWidth / 2) - (msgWidth / 2);
			break;
		case 2:
			xp = pos.x + fontInfo._justifyWidth - msgWidth;
			break;
		default:
			break;
		}

		if (!(fontInfo._fontFlags & (DISPFLAG_1 | DISPFLAG_2))) {
			viewPort->_fontRect.left = xp;
			viewPort->_fontRect.top = yp;
			viewPort->_fontRect.setWidth(msgWidth);
			viewPort->_fontRect.setHeight(fontHeight);
		} else {
			viewPort->_fontRect.left = pos.x;
			viewPort->_fontRect.top = pos.y;
			viewPort->_fontRect.setWidth(fontInfo._justifyWidth);
			viewPort->_fontRect.setHeight(fontInfo._justifyHeight);
		}

		pos.x = xp;
		pos.y = yp;

		if (fontInfo._fontFlags & DISPFLAG_4) {
			if (fontInfo._shadow.x <= 0) {
				viewPort->_fontRect.left += fontInfo._shadow.x;
				viewPort->_fontRect.right -= fontInfo._shadow.x * 2;
			} else {
				viewPort->_fontRect.right += fontInfo._shadow.x;
			}

			if (fontInfo._shadow.y <= 0) {
				viewPort->_fontRect.top += fontInfo._shadow.y;
				viewPort->_fontRect.bottom -= fontInfo._shadow.y * 2;
			} else {
				viewPort->_fontRect.bottom += fontInfo._shadow.y;
			}
		} else if (fontInfo._fontFlags & 8) {
			if (fontInfo._shadow.x <= 0) {
				viewPort->_fontRect.left += fontInfo._shadow.x;
				viewPort->_fontRect.right -= fontInfo._shadow.x * 3;
			} else {
				viewPort->_fontRect.right += fontInfo._shadow.x * 3;
				viewPort->_fontRect.left -= fontInfo._shadow.x;
			}

			if (fontInfo._shadow.y <= 0) {
				viewPort->_fontRect.top += fontInfo._shadow.y;
				viewPort->_fontRect.bottom -= fontInfo._shadow.y * 3;
			} else {
				viewPort->_fontRect.bottom += fontInfo._shadow.y * 3;
				viewPort->_fontRect.top -= fontInfo._shadow.y;
			}
		}
	}

	if (screen._saveBack && fontInfo._fontSaveBack && (_flags & DISPFLAG_VIEWPORT)) {
		viewPort->addSaveRect(viewPort->_pageIndex, viewPort->_fontRect);
	}

	if (fontInfo._fontFlags & DISPFLAG_1) {
		screen._drawPtr->_pos = Common::Point(viewPort->_fontRect.left, viewPort->_fontRect.top);
		screen._drawPtr->_penColor = fontInfo._backColor;
		sFillBox(viewPort->_fontRect.width(), viewPort->_fontRect.height());
	}

	bool saveBack = screen._saveBack;
	screen._saveBack = false;

	int count = 0;
	if (fontInfo._fontFlags & DISPFLAG_4)
		count = 1;
	else if (fontInfo._fontFlags & DISPFLAG_8)
		count = 8;

	for (int i = count; i >= 0; --i) {
		xp = pos.x;
		yp = pos.y;

		switch (xShadows[i]) {
		case -1:
			xp -= fontInfo._shadow.x;
			break;
		case 1:
			xp += fontInfo._shadow.x;
			break;
		default:
			break;
		}

		switch (yShadows[i]) {
		case -1:
			yp -= fontInfo._shadow.y;
			break;
		case 1:
			yp += fontInfo._shadow.y;
			break;
		default:
			break;
		}

		if (i != 0) {
			fontChar._pick = 0;
			fontChar._onOff = fontInfo._shadowColor;
		} else if (fontData._fontDepth == 1 || (fontInfo._fontFlags & DISPFLAG_10)) {
			fontChar._pick = 0;
			fontChar._onOff = fontInfo._foreColor;
		} else {
			fontChar._pick = fontInfo._picPick;
			fontChar._onOff = fontInfo._picOnOff;
		}

		// Loop to draw each character in turn
		msgWidth = -padding;
		const char *msgP = msg.c_str();
		char ch;

		while ((ch = *msgP++) != '\0') {
			int charValue = (int)ch - minChar;
			if (charValue < 0 || charValue >= totalChars || fontData._charWidth[charValue] == 0)
				charValue = fontData._maxChar - minChar;

			int charWidth = fontData._charWidth[charValue];
			fontChar._bounds.setWidth(charWidth);

			uint16 offset = READ_LE_UINT16(fontData._charOffsets + charValue * 2);
			fontChar._imgData = fontData._charImages + offset * 2;

			screen.sDrawPic(&fontChar, this, Common::Point(xp, yp));

			fontChar._imgData = NULL;
			xp += charWidth + padding;
			msgWidth += charWidth + padding;
		}
	}

	msgWidth = MAX(msgWidth, 0);
	if (fontInfo._justify == ALIGN_LEFT)
		fontInfo._pos.x = xp;

	screen._saveBack = saveBack;
	screen._clipPtr = clipPtr;

	return msgWidth;
}

int DisplayResource::textWidth(const Common::String &msg) {
	if (msg.size() == 0)
		return 0;

	const char *msgP = msg.c_str();
	FontResource &fontData = *_vm->_screen->_fontPtr->_curFont;
	int minChar = fontData._minChar;
	int maxChar = fontData._maxChar;
	int padding = fontData._padding;
	int totalWidth = -padding;
	char ch;

	// Loop through the characters
	while ((ch = *msgP++) != '\0') {
		int charValue = (int)ch;
		if (charValue < minChar || charValue > maxChar)
			charValue = maxChar;

		if (!fontData._charWidth[charValue - minChar])
			charValue = maxChar;

		totalWidth += fontData._charWidth[charValue - minChar] + padding;
	}

	if (totalWidth < 0)
		totalWidth = 0;
	return totalWidth;
}

/*------------------------------------------------------------------------*/

PictureResource::PictureResource(BoltFilesState &state, const byte *src):
		DisplayResource(state._vm) {
	_flags = READ_LE_UINT16(src);
	_select = src[2];
	_pick = src[3];
	_onOff = src[4];
	// depth is in src[5], unused.

	int xs = READ_LE_UINT16(&src[6]);
	int ys = READ_LE_UINT16(&src[8]);
	_bounds = Common::Rect(xs, ys, xs + READ_LE_UINT16(&src[10]),
		ys + READ_LE_UINT16(&src[12]));
	_maskData = READ_LE_UINT32(&src[14]);
	_planeSize = READ_LE_UINT16(&src[22]);

	_keyColor = 0;
	_imgData = NULL;
	_freeImgData = DisposeAfterUse::YES;

	int nbytes = _bounds.width() * _bounds.height();
	if (_flags & PICFLAG_20) {
		if (_flags & (PICFLAG_VFLIP | PICFLAG_HFLIP)) {
			// Get the raw data for the picture from another resource
			uint32 id = READ_LE_UINT32(&src[18]);
			const byte *srcData = state._curLibPtr->boltEntry(id)._data;
			_imgData = new byte[nbytes];

			// Flip the image data either horizontally or vertically
			if (_flags & PICFLAG_HFLIP)
				flipHorizontal(srcData);
			else
				flipVertical(srcData);
		} else {
			uint32 id = READ_LE_UINT32(&src[18]) >> 16;
			byte *imgData = state._curLibPtr->boltEntry(id)._picResource->_imgData;
			_freeImgData = DisposeAfterUse::NO;

#if 0
			// TODO: Double check code below. Despite different coding in the
			// original, both looked like they do the same formula.
			// Until it's clarified, this check is disabled and replaced by the
			// common code.
			if (_flags & PICFLAG_PIC_OFFSET) {
				_imgData = imgData + (READ_LE_UINT32(&src[18]) & 0xffff);
			} else {
				_imgData = imgData + (READ_LE_UINT32(&src[18]) & 0xffff);
			}
#endif
			_imgData = imgData + (READ_LE_UINT32(&src[18]) & 0xffff);
		}
	} else if (_flags & PICFLAG_PIC_OFFSET) {
		int mode = 0;
		if (_bounds.width() == 320)
			mode = 147;
		else {
			if (_bounds.width() == 640) {
				if (_bounds.height() == 400)
					mode = 220;
				else
					mode = 221;
			} else if (_bounds.width() == 800)
				mode = 222;
			else if (_bounds.width() == 1024)
				mode = 226;
		}

		if (mode != state._vm->_screen->_SVGAMode) {
			state._vm->_screen->_SVGAMode = mode;
			state._vm->_screen->clearPalette();
		}

		int screenOffset = READ_LE_UINT32(&src[18]) & 0xffff;
		assert(screenOffset == 0);

		if (_flags & PICFLAG_CLEAR_SCREEN) {
			// Clear screen picture. That's right. This game actually has a picture
			// resource flag to clear the screen! Bizarre.
			state._vm->_screen->clear();
		} else {
			// Direct screen loading picture. In this case, the raw data of the resource
			// is directly decompressed into the screen surface. Again, bizarre.
			Screen &screen = *state._vm->_screen;
			byte *pDest = (byte *)screen.getPixels();
			state.decompress(pDest, SCREEN_WIDTH * SCREEN_HEIGHT, state._curMemberPtr->_mode);
			screen.markAllDirty();
		}
	} else {
		if (_flags & PICFLAG_CLEAR_SCREEN00) {
			if (!(_flags & PICFLAG_CLEAR_SCREEN))
				nbytes = state._curMemberPtr->_size - 24;

			int mask = (nbytes + 0x3FFF) >> 14;
			_imgData = NULL;

			if (state._boltPageFrame != NULL) {
				_maskData = mask;
				state.decompress(state._boltPageFrame, nbytes, state._curMemberPtr->_mode);
				return;
			}
		}

		if (_flags & PICFLAG_CLEAR_SCREEN) {
			_imgData = new byte[nbytes];
			Common::fill(_imgData, _imgData + nbytes, 0);
		} else {
			_imgData = state.decompress(NULL, nbytes, state._curMemberPtr->_mode);
		}
	}
}

PictureResource::PictureResource(Graphics::Surface *surface) {
	_flags = DISPFLAG_NONE;
	_select = 0;
	_pick = 0;
	_onOff = 0;
	_maskData = 0;
	_planeSize = 0;
	_keyColor = 0;

	_bounds = Common::Rect(0, 0, surface->w, surface->h);
	_imgData = (byte *)surface->getPixels();
	_freeImgData = DisposeAfterUse::NO;
}

PictureResource::PictureResource() {
	_flags = DISPFLAG_NONE;
	_select = 0;
	_pick = 0;
	_onOff = 0;
	_maskData = 0;
	_planeSize = 0;
	_keyColor = 0;

	_imgData = NULL;
	_freeImgData = DisposeAfterUse::NO;
}

PictureResource::PictureResource(int flags, int select, int pick, int onOff,
		const Common::Rect &bounds, int maskData, byte *imgData, int planeSize) {
	_flags = flags;
	_select = select;
	_pick = pick;
	_onOff = onOff;
	_bounds = bounds;
	_maskData = maskData;
	_imgData = imgData;
	_planeSize = planeSize;
	_freeImgData = DisposeAfterUse::NO;
	_keyColor = 0;
}

PictureResource::~PictureResource() {
	if (_freeImgData == DisposeAfterUse::YES)
		delete[] _imgData;
}

void PictureResource::flipHorizontal(const byte *data) {
	const byte *srcP = data + 18;
	byte *destP = _imgData + _bounds.width() - 1;

	for (int y = 0; y < _bounds.height(); ++y) {
		for (int x = 0; x < _bounds.width(); ++x, ++srcP, --destP)
			*destP = *srcP;

		srcP += _bounds.width();
		destP += _bounds.width();
	}
}

void PictureResource::flipVertical(const byte *data) {
	const byte *srcP = data + 18;
	byte *destP = _imgData + _bounds.width() * (_bounds.height() - 1);

	for (int y = 0; y < _bounds.height(); ++y) {
		Common::copy(srcP, srcP + _bounds.width(), destP);
		srcP += _bounds.width();
		destP -= _bounds.width();
	}
}

/*------------------------------------------------------------------------*/

ViewPortResource::ViewPortResource(BoltFilesState &state, const byte *src):
		_state(state), DisplayResource(state._vm) {
	_flags = READ_LE_UINT16(src);
	_parent = NULL;
	_pageCount = READ_LE_UINT16(src + 6);
	_pageIndex = READ_LE_UINT16(src + 8);
	_lastPage = READ_LE_UINT16(src + 10);

	int xs = READ_LE_UINT16(src + 12);
	int ys = READ_LE_UINT16(src + 14);
	_bounds = Common::Rect(xs, ys, xs + READ_LE_UINT16(src + 16),
		ys + READ_LE_UINT16(src + 18));

	_currentPic = state._curLibPtr->getPictureResource(READ_LE_UINT32(src + 0x20));
	_activePage = state._curLibPtr->getPictureResource(READ_LE_UINT32(src + 0x24));
	_pages[0] = state._curLibPtr->getPictureResource(READ_LE_UINT32(src + 0x28));
	_pages[1] = state._curLibPtr->getPictureResource(READ_LE_UINT32(src + 0x2C));

	byte *dummy;
	state._curLibPtr->resolveIt(READ_LE_UINT32(src + 0x30), &dummy);

	// Get the rect list
	for (int listIndex = 0; listIndex < 3; ++listIndex) {
		_rectListCount[listIndex] = (int16)READ_LE_UINT16(src + 0x40 + 2 * listIndex);
		int id = (int)READ_LE_UINT32(src + 0x34 + listIndex * 4);

		if (id == -1) {
			_rectListPtr[listIndex] = NULL;
		} else {
			_rectListPtr[listIndex] = new Common::Array<Common::Rect>();

			if (_rectListCount[listIndex] > 0) {
				int16 *rectList = (int16 *)state._curLibPtr->memberAddrOffset(id);
				for (int i = 0; i < _rectListCount[listIndex]; ++i) {
					xs = FROM_LE_16(rectList[0]);
					ys = FROM_LE_16(rectList[1]);
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

	state._curLibPtr->resolveIt(READ_LE_UINT32(src + 0x7A), &dummy);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x7E), (ScreenMethodPtr *)&_fn1);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x82), (ScreenMethodPtr *)&_setupFn);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x86), (ScreenMethodPtr *)&_addFn);
	state._curLibPtr->resolveFunction(READ_LE_UINT32(src + 0x8A), (ScreenMethodPtr *)&_restoreFn);

	if (!_restoreFn && _addFn)
		_addFn = &Screen::addRectNoSaveBack;
}

ViewPortResource::~ViewPortResource() {
	for (int i = 0; i < 3; ++i)
		delete _rectListPtr[i];
}

void ViewPortResource::setupViewPort(PictureResource *page, Common::Rect *clippingRect,
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

	if (clippingRect) {
		// Clip based on the passed clip rectangles
		xDiff = clippingRect->left - r.left;
		yDiff = clippingRect->top - r.top;

		if (xDiff > 0) {
			int width = r.width();
			r.left = clippingRect->left;
			r.setWidth(xDiff <= width ? width - xDiff : 0);
		}
		if (yDiff > 0) {
			int height = r.height();
			r.top = clippingRect->top;
			r.setHeight(yDiff <= height ? height - yDiff : 0);
		}

		xDiff = r.right - clippingRect->right;
		yDiff = r.bottom - clippingRect->bottom;

		if (xDiff > 0)
			r.setWidth(xDiff <= r.width() ? r.width() - xDiff : 0);
		if (yDiff > 0)
			r.setHeight(yDiff <= r.height() ? r.height() - yDiff : 0);
	}

	_activePage = page;
	_clipRect = r;
	_setupFn = setupFn;
	_addFn = addFn;
	_restoreFn = restoreFn;

	if (setupFn)
		(_state._vm->_screen->*setupFn)(this);
}

void ViewPortResource::setupViewPort() {
	setupViewPort(_state._vm->_screen->_backgroundPage, NULL,
		&Screen::setupMCGASaveRect, &Screen::addRectOptSaveRect,
		&Screen::restoreMCGASaveRect);
}

void ViewPortResource::setupViewPort(PictureResource *pic, Common::Rect *clippingRect) {
	setupViewPort(pic, clippingRect,
		&Screen::setupMCGASaveRect, &Screen::addRectOptSaveRect,
		&Screen::restoreMCGASaveRect);
}

void ViewPortResource::addSaveRect(int pageIndex, const Common::Rect &r) {
	Common::Rect rect = r;

	if (clipRect(rect)) {
		if (_addFn) {
			(_state._vm->_screen->*_addFn)(this, pageIndex, rect);
		} else if (_rectListCount[pageIndex] != -1) {
			_rectListPtr[pageIndex]->push_back(rect);
		}
	}
}

void ViewPortResource::fillPic(byte onOff) {
	_state._vm->_screen->fillPic(this, onOff);
}

void ViewPortResource::drawIfaceTime() {
	// Hour display
	_state._vm->_screen->drawANumber(_state._vm->_screen->_vPort,
		(_state._vm->_gameHour / 10) == 0 ? 10 : _state._vm->_gameHour / 10,
		Common::Point(161, 25));
	_state._vm->_screen->drawANumber(_state._vm->_screen->_vPort,
		_state._vm->_gameHour % 10, Common::Point(172, 25));

	// Minute display
	_state._vm->_screen->drawANumber(_state._vm->_screen->_vPort,
		_state._vm->_gameMinute / 10, Common::Point(190, 25));
	_state._vm->_screen->drawANumber(_state._vm->_screen->_vPort,
		_state._vm->_gameMinute % 10, Common::Point(201, 25));

	// AM/PM indicator
	PictureResource *pic = _state._vm->_bVoy->boltEntry(_state._vm->_voy->_isAM ? 272 : 273)._picResource;
	_state._vm->_screen->sDrawPic(pic, _state._vm->_screen->_vPort,
		Common::Point(215, 27));
}

void ViewPortResource::drawPicPerm(PictureResource *pic, const Common::Point &pt) {
	Common::Rect bounds = pic->_bounds;
	bounds.translate(pt.x, pt.y);

	bool saveBack = _state._vm->_screen->_saveBack;
	_state._vm->_screen->_saveBack = false;
	_state._vm->_screen->sDrawPic(pic, this, pt);
	clipRect(bounds);

	for (int pageIndex = 0; pageIndex < _pageCount; ++pageIndex) {
		if (_pageIndex != pageIndex) {
			addSaveRect(pageIndex, bounds);
		}
	}

	_state._vm->_screen->_saveBack = saveBack;
}
/*------------------------------------------------------------------------*/

ViewPortListResource::ViewPortListResource(BoltFilesState &state, const byte *src) {
	uint count = READ_LE_UINT16(src);
	_palIndex = READ_LE_UINT16(src + 2);

	// Load palette map
	byte *palData = state._curLibPtr->memberAddr(READ_LE_UINT32(src + 4));
	for (uint i = 0; i < 256; ++i, palData += 16)
		_palette.push_back(ViewPortPalEntry(palData));

	// Load view port pointer list
	const uint32 *idP = (const uint32 *)&src[8];
	for (uint i = 0; i < count; ++i, ++idP) {
		uint32 id = READ_LE_UINT32(idP);
		BoltEntry &entry = state._curLibPtr->getBoltEntryFromLong(id);

		assert(entry._viewPortResource);
		_entries.push_back(entry._viewPortResource);
	}
}

/*------------------------------------------------------------------------*/

ViewPortPalEntry::ViewPortPalEntry(const byte *src) {
	const uint16 *v = (const uint16 *)src;
	_rEntry = READ_LE_UINT16(v++);
	_gEntry = READ_LE_UINT16(v++);
	_bEntry = READ_LE_UINT16(v++);
	_rChange = READ_LE_UINT16(v++);
	_gChange = READ_LE_UINT16(v++);
	_bChange = READ_LE_UINT16(v++);
	_palIndex = READ_LE_UINT16(v++);
}


/*------------------------------------------------------------------------*/

FontResource::FontResource(BoltFilesState &state, byte *src) {
	_minChar = src[0];
	_maxChar = src[1];
	_fontDepth = src[2];
	_padding = src[3];
	_fontHeight = src[5];
	_topPadding = (int8)src[6];

	int totalChars = _maxChar - _minChar + 1;
	_charWidth = new int[totalChars];
	for (int i = 0; i < totalChars; ++i)
		_charWidth[i] = READ_LE_UINT16(src + 8 + 2 * i);

	_charOffsets = src + 8 + totalChars * 2;
	_charImages = _charOffsets + totalChars * 2;
}

FontResource::~FontResource() {
	delete[] _charWidth;
}

/*------------------------------------------------------------------------*/

FontInfoResource::FontInfoResource(BoltFilesState &state, const byte *src) {
	_curFont = NULL;
	_picFlags = src[4];
	_picSelect = src[5];
	_picPick = src[6];
	_picOnOff = src[7];
	_fontFlags = src[8];
	_justify = (FontJustify)src[9];
	_fontSaveBack = READ_LE_UINT16(src + 10);
	_pos.x = (int16)READ_LE_UINT16(src + 12);
	_pos.y = (int16)READ_LE_UINT16(src + 14);
	_justifyWidth = READ_LE_UINT16(src + 16);
	_justifyHeight = READ_LE_UINT16(src + 18);
	_shadow.x = READ_LE_UINT16(src + 20);
	_shadow.y = READ_LE_UINT16(src + 22);
	_foreColor = READ_LE_UINT16(src + 24);
	_backColor = READ_LE_UINT16(src + 26);
	_shadowColor = READ_LE_UINT16(src + 28);
}

FontInfoResource::FontInfoResource() {
	_curFont = NULL;
	_picFlags = DISPFLAG_1 | DISPFLAG_2;
	_picSelect = 0xff;
	_picPick = 0xff;
	_picOnOff = 0;
	_fontFlags = DISPFLAG_NONE;
	_justify = ALIGN_LEFT;
	_fontSaveBack = 0;
	_justifyWidth = 1;
	_justifyHeight = 1;
	_shadow = Common::Point(1, 1);
	_foreColor = 1;
	_backColor = 0;
	_shadowColor = 0;
}

FontInfoResource::FontInfoResource(byte picFlags, byte picSelect, byte picPick, byte picOnOff,
		byte fontFlags, FontJustify justify, int fontSaveBack, const Common::Point &pos,
		int justifyWidth, int justifyHeight, const Common::Point &shadow, int foreColor,
		int backColor, int shadowColor) {
	_curFont = NULL;
	_picFlags = picFlags;
	_picSelect = picSelect;
	_picPick = picPick;
	_picOnOff = picOnOff;
	_fontFlags = fontFlags;
	_justify = justify;
	_fontSaveBack = fontSaveBack;
	_pos = pos;
	_justifyWidth = justifyWidth;
	_justifyHeight = justifyHeight;
	_shadow = shadow;
	_foreColor = foreColor;
	_backColor = backColor;
	_shadowColor = shadowColor;
}

/*------------------------------------------------------------------------*/

CMapResource::CMapResource(BoltFilesState &state, const byte *src): _vm(state._vm) {
	_steps = src[0];
	_fadeStatus = src[1];
	_start = READ_LE_UINT16(src + 2);
	_end = READ_LE_UINT16(src + 4);

	int count = _end - _start + 1;
	_entries = new byte[count * 3];
	Common::copy(src + 6, src + 6 + 3 * count, _entries);

	int palIndex = state._vm->_screen->_viewPortListPtr->_palIndex;
	if (_end > palIndex)
		_end = palIndex;
	if (_start > palIndex)
		_start = palIndex;
}

CMapResource::~CMapResource() {
	delete[] _entries;
}

void CMapResource::startFade() {
	_vm->_eventsManager->startFade(this);
}

/*------------------------------------------------------------------------*/

VInitCycleResource::VInitCycleResource(BoltFilesState &state, const byte *src):
		_state(state) {
	// Set up arrays
	for (int i = 0; i < 4; ++i) {
		_type[i] = READ_LE_UINT16(src + i * 2);
		state._curLibPtr->resolveIt(READ_LE_UINT32(src + 8 + i * 4), &_ptr[i]);
	}
}

void VInitCycleResource::vStartCycle() {
	EventsManager &evt = *_state._vm->_eventsManager;
	evt._cycleIntNode._flags |= 1;
	evt._cyclePtr = this;

	for (int i = 0; i < 4; ++i) {
		evt._cycleNext[i] = _ptr[i];
		evt._cycleTime[i] = 0;
	}

	evt._cycleStatus = 1;
	evt._cycleIntNode._flags &= ~1;
}

void VInitCycleResource::vStopCycle() {
	EventsManager &evt = *_state._vm->_eventsManager;
	evt._cycleIntNode._flags |= 1;
	evt._cycleStatus &= ~1;
}

/*------------------------------------------------------------------------*/

PtrResource::PtrResource(BoltFilesState &state, const byte *src) {
	// Load pointer list
	const uint32 *idP = (const uint32 *)&src[0];
	int size = state._curMemberPtr->_size;

	for (int i = 0; i < size / 4; ++i, ++idP) {
		uint32 id = READ_LE_UINT32(idP);
		BoltEntry &entry = state._curLibPtr->getBoltEntryFromLong(id);

		_entries.push_back(&entry);
	}
}

/*------------------------------------------------------------------------*/

ControlResource::ControlResource(BoltFilesState &state, const byte *src) {
	// Get Id for the state data. Since it refers to a following entry in the same
	// group, for simplicity we set the _state back in the main playStamp method
	_stateId = READ_LE_UINT32(&src[0x32]);
	_state = nullptr;

	for (int i = 0; i < 8; ++i)
		_memberIds[i] = READ_LE_UINT16(src + i * 2);

	// Load pointer list
	const uint32 *idP = (const uint32 *)&src[0x10];
	int count = READ_LE_UINT16(&src[0x36]);

	Common::fill(&_entries[0], &_entries[8], (byte *)nullptr);
	for (int i = 0; i < count; ++i, ++idP) {
		uint32 id = READ_LE_UINT32(idP);
		state._curLibPtr->resolveIt(id, &_entries[i]);
	}
}

/*------------------------------------------------------------------------*/

StateResource::StateResource(BoltFilesState &state, const byte *src):
		_victimIndex(_vals[1]), _victimEvidenceIndex(_vals[2]),
		_victimMurderIndex(_vals[3]) {
	for (int i = 0; i < 4; ++i)
		_vals[i] = READ_LE_UINT32(src + i * 4);
}

void StateResource::synchronize(Common::Serializer &s) {
	for (int i = 0; i < 4; ++i)
		s.syncAsSint32LE(_vals[i]);
}

} // End of namespace Voyeur
