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

#include "common/config-manager.h"
#include "awe/resource.h"
#include "awe/file.h"
#include "awe/pak.h"
#include "awe/resource_nth.h"
#include "awe/resource_win31.h"
#include "awe/resource_3do.h"
#include "awe/unpack.h"
#include "awe/util.h"
#include "awe/video.h"

namespace Awe {

static const char *atariDemo = "aw.tos";

static const char *kGameTitleEU = "Another World";
static const char *kGameTitleUS = "Out Of This World";
static const char *kGameTitle15thEdition = "Another World 15th anniversary edition";
static const char *kGameTitle20thEdition = "Another World 20th anniversary edition";

Resource::Resource(Video *vid, DataType dataType) :
		_vid(vid), _dataType(dataType) {
	if (_dataType == DT_ATARI) {
		_amigaMemList = detectAmigaAtari();
	}
}

Resource::~Resource() {
	free(_demo3Joy.bufPtr);
	delete _nth;
	delete _win31;
	delete _3do;
}

bool Resource::readBank(const MemEntry *me, uint8_t *dstBuf) {
	bool ret = false;
	char name[10];
	snprintf(name, sizeof(name), "%s%02x", _bankPrefix, me->bankNum);
	File f;
	if (f.open(name) || (_dataType == DT_ATARI_DEMO && f.open(atariDemo))) {
		f.seek(me->bankPos);
		const size_t count = f.read(dstBuf, me->packedSize);
		ret = (count == me->packedSize);
		if (ret && me->packedSize != me->unpackedSize) {
			ret = bytekiller_unpack(dstBuf, me->unpackedSize, dstBuf, me->packedSize);
		}
	}
	return ret;
}

const AmigaMemEntry *Resource::detectAmigaAtari() {
	Common::File f;

	static const struct {
		uint32_t bank01Size;
		const AmigaMemEntry *entries;
	} _files[] = {
		{ 244674, Resource::_memListAmigaFR },
		{ 244868, Resource::_memListAmigaEN },
		{ 227142, Resource::_memListAtariEN },
		{ 0, 0 }
	};
	if (f.open("bank01")) {
		const uint32_t size = f.size();
		for (int i = 0; _files[i].entries; ++i) {
			if (_files[i].bank01Size == size) {
				return _files[i].entries;
			}
		}
	}

	return nullptr;
}

const char *Resource::getGameTitle(Language lang) const {
	switch (_dataType) {
	case DT_15TH_EDITION:
		return kGameTitle15thEdition;
	case DT_20TH_EDITION:
		return kGameTitle20thEdition;
	case DT_3DO:
		return kGameTitleUS;
	case DT_DOS:
		if (lang == Common::EN_ANY) {
			return kGameTitleUS;
		}
		/* fall-through */
	default:
		break;
	}
	return kGameTitleEU;
}

void Resource::readEntries() {
	switch (_dataType) {
	case DT_15TH_EDITION:
		_numMemList = ENTRIES_COUNT;
		_nth = ResourceNth::create(15);
		if (_nth && _nth->init()) {
			return;
		}
		break;
	case DT_20TH_EDITION:
		_numMemList = ENTRIES_COUNT_20TH;
		_nth = ResourceNth::create(20);
		if (_nth && _nth->init()) {
			return;
		}
		break;
	case DT_AMIGA:
	case DT_ATARI:
		assert(_amigaMemList);
		readEntriesAmiga(_amigaMemList, ENTRIES_COUNT);
		return;
	case DT_DOS:
	{
		_hasPasswordScreen = false; // DOS demo versions do not have the resources
		Common::File f;
		if (Common::File::exists("demo01")) {
			_bankPrefix = "demo";
		}
		if (f.open("memlist.bin")) {
			MemEntry *me = _memList;

			for (;;) {
				assert(_numMemList < ARRAYSIZE(_memList));
				me->load(&f);

				if (me->status == 0xFF) {
					const int num = _memListParts[8][1]; // 16008 bytecode
					assert(num < _numMemList);
					Common::String bank = Common::String::format(
						"%s%02x", _bankPrefix, _memList[num].bankNum);
					_hasPasswordScreen = Common::File::exists(bank.c_str());
					return;
				}

				++_numMemList;
				++me;
			}
		}
	}
	break;
	case DT_WIN31:
		_numMemList = ENTRIES_COUNT;
		_win31 = new ResourceWin31();
		if (_win31->readEntries()) {
			return;
		}
		break;
	case DT_3DO:
		_numMemList = ENTRIES_COUNT;
		_3do = new Resource3do();
		if (_3do->readEntries()) {
			return;
		}
		break;
	case DT_ATARI_DEMO:
	{
		File f;
		if (f.open(atariDemo)) {
			static const struct {
				uint8_t type;
				uint8_t num;
				uint32_t offset;
				uint16_t size;
			} data[] = {
				{ RT_SHAPE, 0x19, 0x50f0, 65146 },
				{ RT_PALETTE, 0x17, 0x14f6a, 2048 },
				{ RT_BYTECODE, 0x18, 0x1576a, 8368 }
			};
			_numMemList = ENTRIES_COUNT;
			for (int i = 0; i < 3; ++i) {
				MemEntry *entry = &_memList[data[i].num];
				entry->type = data[i].type;
				entry->bankNum = 15;
				entry->bankPos = data[i].offset;
				entry->packedSize = entry->unpackedSize = data[i].size;
			}
			return;
		}
	}
	break;
	}

	error("No data files found");
}

void Resource::readEntriesAmiga(const AmigaMemEntry *entries, int count) {
	_numMemList = count;
	for (int i = 0; i < count; ++i) {
		_memList[i].type = entries[i].type;
		_memList[i].bankNum = entries[i].bank;
		_memList[i].bankPos = entries[i].offset;
		_memList[i].packedSize = entries[i].packedSize;
		_memList[i].unpackedSize = entries[i].unpackedSize;
	}
	_memList[count].status = 0xFF;
}

void Resource::dumpEntries() {
	static const bool kDump = false;
	if (kDump && (_dataType == DT_DOS || _dataType == DT_AMIGA || _dataType == DT_ATARI)) {
		for (int i = 0; i < _numMemList; ++i) {
			if (_memList[i].unpackedSize == 0) {
				continue;
			}
			if (_memList[i].bankNum == 5 && (_dataType == DT_AMIGA || _dataType == DT_ATARI)) {
				continue;
			}
			uint8_t *p = (uint8_t *)malloc(_memList[i].unpackedSize);
			if (p) {
				if (readBank(&_memList[i], p)) {
					char name[16];
					snprintf(name, sizeof(name), "data_%02x_%d", i, _memList[i].type);
					dumpFile(name, p, _memList[i].unpackedSize);
				}
				free(p);
			}
		}
	}
}

void Resource::load() {
	while (1) {
		MemEntry *me = 0;

		// get resource with max rankNum
		uint8_t maxNum = 0;
		for (int i = 0; i < _numMemList; ++i) {
			MemEntry *it = &_memList[i];
			if (it->status == STATUS_TOLOAD && maxNum <= it->rankNum) {
				maxNum = it->rankNum;
				me = it;
			}
		}
		if (me == 0) {
			break; // no entry found
		}

		const int resourceNum = me - _memList;

		uint8_t *memPtr = 0;
		if (me->type == RT_BITMAP) {
			memPtr = _vidCurPtr;
		} else {
			memPtr = _scriptCurPtr;
			const uint32_t avail = uint32_t(_vidCurPtr - _scriptCurPtr);
			if (me->unpackedSize > avail) {
				warning("Resource::load() not enough memory, available=%d", avail);
				me->status = STATUS_NULL;
				continue;
			}
		}
		if (me->bankNum == 0) {
			warning("Resource::load() ec=0x%X (me->bankNum == 0)", 0xF00);
			me->status = STATUS_NULL;
		} else {
			debugC(kDebugBank, "Resource::load() bufPos=0x%x size=%d type=%d pos=0x%x bankNum=%d",
				(uint)(memPtr - _memPtrStart), me->packedSize, me->type,
				me->bankPos, me->bankNum);

			if (readBank(me, memPtr)) {
				if (me->type == RT_BITMAP) {
					_vid->copyBitmapPtr(_vidCurPtr, me->unpackedSize);
					me->status = STATUS_NULL;
				} else {
					me->bufPtr = memPtr;
					me->status = STATUS_LOADED;
					_scriptCurPtr += me->unpackedSize;
				}
			} else {
				if (_dataType == DT_DOS && me->bankNum == 12 && me->type == RT_BANK) {
					// DOS demo version does not have the bank for this resource
					// this should be safe to ignore as the resource does not appear to be used by the game code
					me->status = STATUS_NULL;
					continue;
				}
				error("Unable to read resource %d from bank %d", resourceNum, me->bankNum);
			}
		}
	}
}

void Resource::invalidateRes() {
	for (int i = 0; i < _numMemList; ++i) {
		MemEntry *me = &_memList[i];
		if (me->type <= 2 || me->type > 6) {
			me->status = STATUS_NULL;
		}
	}
	_scriptCurPtr = _scriptBakPtr;
	_vid->_currentPal = 0xFF;
}

void Resource::invalidateAll() {
	for (int i = 0; i < _numMemList; ++i) {
		_memList[i].status = STATUS_NULL;
	}
	_scriptCurPtr = _memPtrStart;
	_vid->_currentPal = 0xFF;
}

static const uint8_t *getSoundsList3DO(int num) {
	static const uint8_t intro7[] = {
		0x33, 0xFF
	};
	static const uint8_t water7[] = {
		0x08, 0x10, 0x2D, 0x30, 0x31, 0x32, 0x35, 0x39, 0x3A, 0x3C,
		0x3D, 0x3E, 0x4A, 0x4B, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52,
		0x54, 0xFF
	};
	static const uint8_t pri1[] = {
		0x52, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D,
		0x5E, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x70, 0x71, 0x72, 0x73, 0xFF
	};
	static const uint8_t cite1[] = {
		0x02, 0x03, 0x52, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B,
		0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x63, 0x66, 0x6A, 0x6B, 0x6C,
		0x6D, 0x6E, 0x6F, 0x70, 0x72, 0x74, 0x75, 0x77, 0x78, 0x79,
		0x7A, 0x7B, 0x7C, 0x88, 0xFF
	};
	static const uint8_t arene2[] = {
		0x52, 0x57, 0x58, 0x59, 0x5B, 0x84, 0x8B, 0x8C, 0x8E, 0xFF
	};
	static const uint8_t luxe2[] = {
		0x30, 0x52, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C,
		0x5D, 0x5E, 0x5F, 0x60, 0x66, 0x67, 0x6B, 0x6C, 0x70, 0x74,
		0x75, 0x79, 0x7A, 0x8D, 0xFF
	};
	static const uint8_t final3[] = {
		0x08, 0x0E, 0x0F, 0x57, 0xFF
	};
	switch (num) {
	case 2001: return intro7;
	case 2002: return water7;
	case 2003: return pri1;
	case 2004: return cite1;
	case 2005: return arene2;
	case 2006: return luxe2;
	case 2007: return final3;
	}
	return 0;
}

static const int _memListBmp[] = {
	145, 144, 73, 72, 70, 69, 68, 67, -1
};

void Resource::update(uint16_t num, PreloadSoundProc preloadSound, void *data) {
	if (num > 16000) {
		_nextPart = num;
		return;
	}
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
		if (num >= 3000) {
			loadBmp(num);
			break;
		} else if (num >= 2000) {
			break;
		}
		/* fall-through */
	case DT_WIN31:
		if (num == 71 || num == 83) {
			loadBmp(num);
			break;
		}
		for (int i = 0; _memListBmp[i] != -1; ++i) {
			if (num == _memListBmp[i]) {
				loadBmp(num);
				return;
			}
		}
		break;
	case DT_3DO:
		if (num >= 2000) { // preload sounds
			const uint8_t *soundsList = getSoundsList3DO(num);
			for (int i = 0; soundsList[i] != 255; ++i) {
				const int soundNum = soundsList[i];
				loadDat(soundNum);
				if (_memList[soundNum].status == STATUS_LOADED) {
					preloadSound(data, soundNum, _memList[soundNum].bufPtr);
				}
			}
		} else if (num >= 200) {
			loadBmp(num);
		}
		break;
	case DT_AMIGA:
	case DT_ATARI:
	case DT_ATARI_DEMO:
	case DT_DOS:
		MemEntry *me = &_memList[num];
		if (me->status == STATUS_NULL) {
			me->status = STATUS_TOLOAD;
			load();
		}
		break;
	}
}

void Resource::loadBmp(int num) {
	uint32_t size = 0;
	uint8_t *p = 0;
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
		p = _nth->loadBmp(num);
		break;
	case DT_WIN31:
		p = _win31->loadFile(num, 0, &size);
		break;
	case DT_3DO:
		p = _3do->loadFile(num, 0, &size);
		break;
	default:
		break;
	}
	if (p) {
		_vid->copyBitmapPtr(p, size);
		free(p);
	}
}

uint8_t *Resource::loadDat(int num) {
	assert(num < _numMemList);
	if (_memList[num].status == STATUS_LOADED) {
		return _memList[num].bufPtr;
	}
	uint32_t size = 0;
	uint8_t *p = 0;
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
		p = _nth->loadDat(num, _scriptCurPtr, &size);
		break;
	case DT_WIN31:
		p = _win31->loadFile(num, _scriptCurPtr, &size);
		break;
	case DT_3DO:
		p = _3do->loadFile(num, _scriptCurPtr, &size);
		break;
	default:
		break;
	}
	if (p) {
		_scriptCurPtr += size;
		_memList[num].bufPtr = p;
		_memList[num].status = STATUS_LOADED;
	}
	return p;
}

void Resource::loadFont() {
	if (_nth) {
		uint8_t *p = _nth->load("font.bmp");
		if (p) {
			_vid->setFont(p);
			free(p);
		}
	}
}

void Resource::loadHeads() {
	if (_nth) {
		uint8_t *p = _nth->load("heads.bmp");
		if (p) {
			_vid->setHeads(p);
			free(p);
		}
	}
}

uint8_t *Resource::loadWav(int num, uint32 *size) {
	if (_memList[num].status == STATUS_LOADED) {
		return _memList[num].bufPtr;
	}

	uint32_t dummy = 0;
	if (!size)
		size = &dummy;
	uint8_t *p = 0;
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
		p = _nth->loadWav(num, _scriptCurPtr, size);
		break;
	case DT_WIN31:
		p = _win31->loadFile(num, _scriptCurPtr, size);
		break;
	default:
		break;
	}
	if (p && size != 0) {
		_scriptCurPtr += *size;
		_memList[num].bufPtr = p;
		_memList[num].status = STATUS_LOADED;
	}

	return p;
}

const char *Resource::getString(int num) {
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
		return _nth->getString(_lang, num);
	case DT_WIN31:
		return _win31->getString(num);
	default:
		break;
	}
	return 0;
}

const char *Resource::getMusicPath(int num, char *buf, int bufSize, uint32_t *offset) {
	const char *name = 0;
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
		name = _nth->getMusicName(num);
		break;
	case DT_WIN31:
		name = _win31->getMusicName(num);
		break;
	case DT_3DO:
		assert(offset);
		name = _3do->getMusicName(num, offset);
#ifdef OPERA_ISO
		if (*offset != 0) {
			// playing music from .ISO
			return dataDir;
		}
#endif
		break;
	default:
		break;
	}

	if (name) {
		snprintf(buf, bufSize, "%s", name);
		return buf;
	}

	return 0;
}

const uint8_t Resource::_memListParts[][4] = {
	{ 0x14, 0x15, 0x16, 0x00 }, // 16000 - protection screens
	{ 0x17, 0x18, 0x19, 0x00 }, // 16001 - introduction
	{ 0x1A, 0x1B, 0x1C, 0x11 }, // 16002 - water
	{ 0x1D, 0x1E, 0x1F, 0x11 }, // 16003 - jail
	{ 0x20, 0x21, 0x22, 0x11 }, // 16004 - 'cite'
	{ 0x23, 0x24, 0x25, 0x00 }, // 16005 - 'arene'
	{ 0x26, 0x27, 0x28, 0x11 }, // 16006 - 'luxe'
	{ 0x29, 0x2A, 0x2B, 0x11 }, // 16007 - 'final'
	{ 0x7D, 0x7E, 0x7F, 0x00 }, // 16008 - password screen
	{ 0x7D, 0x7E, 0x7F, 0x00 }  // 16009 - password screen
};

void Resource::setupPart(int ptrId) {
	int firstPart = kPartCopyProtection;
	switch (_dataType) {
	case DT_15TH_EDITION:
	case DT_20TH_EDITION:
	case DT_3DO:
		firstPart = kPartIntro;
		/* fall-through */
	case DT_WIN31:
		if (ptrId >= firstPart && ptrId <= 16009) {
			invalidateAll();
			uint8_t **segments[4] = { &_segVideoPal, &_segCode, &_segVideo1, &_segVideo2 };
			for (int i = 0; i < 4; ++i) {
				const int num = _memListParts[ptrId - 16000][i];
				if (num != 0) {
					if (_dataType == DT_20TH_EDITION && 0) {
						// HD assets
						_nth->preloadDat(ptrId - 16000, i, num);
					}
					*segments[i] = loadDat(num);
				}
			}
			_currentPart = ptrId;
		} else {
			error("Resource::setupPart() ec=0x%X invalid part", 0xF07);
		}
		_scriptBakPtr = _scriptCurPtr;
		break;
	case DT_AMIGA:
	case DT_ATARI:
	case DT_ATARI_DEMO:
	case DT_DOS:
		if (ptrId != _currentPart) {
			uint8_t ipal = 0;
			uint8_t icod = 0;
			uint8_t ivd1 = 0;
			uint8_t ivd2 = 0;
			if (ptrId >= 16000 && ptrId <= 16009) {
				uint16_t part = ptrId - 16000;
				ipal = _memListParts[part][0];
				icod = _memListParts[part][1];
				ivd1 = _memListParts[part][2];
				ivd2 = _memListParts[part][3];
			} else {
				error("Resource::setupPart() ec=0x%X invalid part", 0xF07);
			}
			invalidateAll();
			_memList[ipal].status = STATUS_TOLOAD;
			_memList[icod].status = STATUS_TOLOAD;
			_memList[ivd1].status = STATUS_TOLOAD;
			if (ivd2 != 0) {
				_memList[ivd2].status = STATUS_TOLOAD;
			}
			load();
			_segVideoPal = _memList[ipal].bufPtr;
			_segCode = _memList[icod].bufPtr;
			_segVideo1 = _memList[ivd1].bufPtr;
			if (ivd2 != 0) {
				_segVideo2 = _memList[ivd2].bufPtr;
			}
			_currentPart = ptrId;
		}
		_scriptBakPtr = _scriptCurPtr;
		break;
	}
}

void Resource::allocMemBlock() {
	_memPtrStart = (uint8_t *)malloc(MEM_BLOCK_SIZE);
	_scriptBakPtr = _scriptCurPtr = _memPtrStart;
	_vidCurPtr = _memPtrStart + MEM_BLOCK_SIZE - (320 * 200 / 2); // 4bpp bitmap
	_useSegVideo2 = false;
}

void Resource::freeMemBlock() {
	free(_memPtrStart);
	_memPtrStart = 0;
}

void Resource::readDemo3Joy() {
	static const char *filename = "demo3.joy";
	File f;
	if (f.open(filename)) {
		const uint32_t fileSize = f.size();
		_demo3Joy.bufPtr = (uint8_t *)malloc(fileSize);
		if (_demo3Joy.bufPtr) {
			_demo3Joy.bufSize = f.read(_demo3Joy.bufPtr, fileSize);
			_demo3Joy.bufPos = -1;
		}
	} else {
		warning("Unable to open '%s'", filename);
	}
}

void MemEntry::load(Common::SeekableReadStream *src) {
	status = src->readByte();
	type = src->readByte();
	bufPtr = nullptr; (void)src->readUint32BE();
	rankNum = src->readByte();
	bankNum = src->readByte();
	bankPos = src->readUint32BE();
	packedSize = src->readUint32BE();
	unpackedSize = src->readUint32BE();
}

} // namespace Awe
