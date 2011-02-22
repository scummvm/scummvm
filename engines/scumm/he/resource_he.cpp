/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Parts of this code are based on:
 * icoutils - A set of programs dealing with MS Windows icons and cursors.
 * Copyright (C) 1998-2001 Oskar Liljeblad
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

#include "scumm/scumm.h"
#include "scumm/file.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/he/resource_he.h"
#include "scumm/he/sound_he.h"

#include "audio/decoders/wave.h"
#include "graphics/cursorman.h"

#include "common/archive.h"
#include "common/memstream.h"
#include "common/system.h"

namespace Scumm {

ResExtractor::ResExtractor(ScummEngine_v70he *scumm)
	: _vm(scumm) {

	memset(_cursorCache, 0, sizeof(_cursorCache));
}

ResExtractor::~ResExtractor() {
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (cc->valid) {
			free(cc->bitmap);
			free(cc->palette);
		}
	}
	memset(_cursorCache, 0, sizeof(_cursorCache));
}

ResExtractor::CachedCursor *ResExtractor::findCachedCursor(int id) {
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (cc->valid && cc->id == id) {
			return cc;
		}
	}
	return NULL;
}

ResExtractor::CachedCursor *ResExtractor::getCachedCursorSlot() {
	uint32 min_last_used = 0;
	CachedCursor *r = NULL;
	for (int i = 0; i < MAX_CACHED_CURSORS; ++i) {
		CachedCursor *cc = &_cursorCache[i];
		if (!cc->valid) {
			return cc;
		} else {
			if (min_last_used == 0 || cc->last_used < min_last_used) {
				min_last_used = cc->last_used;
				r = cc;
			}
		}
	}
	assert(r);
	free(r->bitmap);
	free(r->palette);
	memset(r, 0, sizeof(CachedCursor));
	return r;
}

void ResExtractor::setCursor(int id) {
	byte *cursorRes = 0;
	int cursorsize;
	int keycolor = 0;
	CachedCursor *cc = findCachedCursor(id);
	if (cc != NULL) {
		debug(7, "Found cursor %d in cache slot %lu", id, (long)(cc - _cursorCache));
	} else {
		cc = getCachedCursorSlot();
		assert(cc && !cc->valid);
		cursorsize = extractResource(id, &cursorRes);
		convertIcons(cursorRes, cursorsize, &cc->bitmap, &cc->w, &cc->h, &cc->hotspot_x, &cc->hotspot_y, &keycolor, &cc->palette, &cc->palSize);
		debug(7, "Adding cursor %d to cache slot %lu", id, (long)(cc - _cursorCache));
		free(cursorRes);
		cc->valid = true;
		cc->id = id;
		cc->last_used = g_system->getMillis();
	}

	if (cc->palette)
		CursorMan.replaceCursorPalette(cc->palette, 0, cc->palSize);

	_vm->setCursorHotspot(cc->hotspot_x, cc->hotspot_y);
	_vm->setCursorFromBuffer(cc->bitmap, cc->w, cc->h, cc->w);
}


Win32ResExtractor::Win32ResExtractor(ScummEngine_v70he *scumm) : ResExtractor(scumm) {
}

int Win32ResExtractor::extractResource(int resId, byte **data) {
	if (_fileName.empty()) { // We are running for the first time
		_fileName = _vm->generateFilename(-3);

		if (!_exe.loadFromEXE(_fileName))
			error("Cannot open file %s", _fileName.c_str());
	}

	Common::SeekableReadStream *cursorGroup = _exe.getResource(Common::kPEGroupCursor, resId);

	if (!cursorGroup)
		error("Could not find cursor group %d", resId);

	cursorGroup->skip(4);
	uint16 count = cursorGroup->readUint16LE();
	assert(count > 0);

	cursorGroup->skip(12);
	resId = cursorGroup->readUint16LE();

	delete cursorGroup;

	Common::SeekableReadStream *cursor = _exe.getResource(Common::kPECursor, resId);

	if (!cursor)
		error("Could not find cursor %d", resId);

	int size = cursor->size();
	*data = (byte *)malloc(size);
	cursor->read(*data, size);
	delete cursor;

	return size;
}

#define ROW_BYTES(bits) ((((bits) + 31) >> 5) << 2)

int Win32ResExtractor::convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			int *hotspot_x, int *hotspot_y, int *keycolor, byte **pal, int *palSize) {

	Common::MemoryReadStream *in = new Common::MemoryReadStream(data, datasize);

	*hotspot_x = in->readUint16LE();
	*hotspot_y = in->readUint16LE();

	Win32BitmapInfoHeader bitmap;

	in->read(&bitmap, sizeof(Win32BitmapInfoHeader));

	fix_win32_bitmap_info_header_endian(&bitmap);
	if (bitmap.size < sizeof(Win32BitmapInfoHeader))
		error("bitmap header is too short");

	if (bitmap.compression != 0)
		error("compressed image data not supported");

	if (bitmap.x_pels_per_meter != 0)
		error("x_pels_per_meter field in bitmap should be zero");

	if (bitmap.y_pels_per_meter != 0)
		error("y_pels_per_meter field in bitmap should be zero");

	if (bitmap.clr_important != 0)
		error("clr_important field in bitmap should be zero");

	if (bitmap.planes != 1)
		error("planes field in bitmap should be one");

	Win32RGBQuad *palette = NULL;
	uint32 palette_count = 0;
	if (bitmap.clr_used != 0 || bitmap.bit_count < 24) {
		palette_count = (bitmap.clr_used != 0 ? bitmap.clr_used : 1 << bitmap.bit_count);
		palette = (Win32RGBQuad *)malloc(sizeof(Win32RGBQuad) * palette_count);
		in->read(palette, sizeof(Win32RGBQuad) * palette_count);
	}

	uint32 width = bitmap.width;
	uint32 height = ABS(bitmap.height) / 2;

	uint32 image_size = height * ROW_BYTES(width * bitmap.bit_count);
	uint32 mask_size = height * ROW_BYTES(width);

	byte *image_data = (byte *)malloc(image_size);
	in->read(image_data, image_size);

	byte *mask_data = (byte *)malloc(mask_size);
	in->read(mask_data, mask_size);

	
	*w = width;
	*h = height;
	*keycolor = 0;
	*cursor = (byte *)malloc(width * height);

	byte *row = (byte *)malloc(width * 4);

	for (uint32 d = 0; d < height; d++) {
		uint32 y = (bitmap.height < 0 ? d : height - d - 1);
		uint32 imod = y * (image_size / height) * 8 / bitmap.bit_count;
		uint32 mmod = y * (mask_size / height) * 8;

		for (uint32 x = 0; x < width; x++) {
			uint32 color = simple_vec(image_data, x + imod, bitmap.bit_count);

			// HACK: Ignore the actual cursor palette and use SCUMM's
			if (!simple_vec(mask_data, x + mmod, 1)) {
				if (color)
					cursor[0][width * d + x] = 254; // white
				else
					cursor[0][width * d + x] = 253; // black
			} else {
				cursor[0][width * d + x] = 255; // transparent
			}
		}

	}

	free(row);
	free(palette);
	if (image_data != NULL) {
		free(image_data);
		free(mask_data);
	}

	return 1;
}

uint32 Win32ResExtractor::simple_vec(byte *data, uint32 ofs, byte size) {
	switch (size) {
	case 1:
		return (data[ofs/8] >> (7 - ofs%8)) & 1;
	case 2:
		return (data[ofs/4] >> ((3 - ofs%4) << 1)) & 3;
	case 4:
		return (data[ofs/2] >> ((1 - ofs%2) << 2)) & 15;
	case 8:
		return data[ofs];
	case 16:
		return data[2*ofs] | data[2*ofs+1] << 8;
	case 24:
		return data[3*ofs] | data[3*ofs+1] << 8 | data[3*ofs+2] << 16;
	case 32:
		return data[4*ofs] | data[4*ofs+1] << 8 | data[4*ofs+2] << 16 | data[4*ofs+3] << 24;
	}

	return 0;
}

#if defined(SCUMM_LITTLE_ENDIAN)
#define LE16(x)
#define LE32(x)
#elif defined(SCUMM_BIG_ENDIAN)
#define LE16(x)      ((x) = TO_LE_16(x))
#define LE32(x)      ((x) = TO_LE_32(x))
#endif

void Win32ResExtractor::fix_win32_bitmap_info_header_endian(Win32BitmapInfoHeader *obj) {
	LE32(obj->size);
	LE32(obj->width);
	LE32(obj->height);
	LE16(obj->planes);
	LE16(obj->bit_count);
	LE32(obj->compression);
	LE32(obj->size_image);
	LE32(obj->x_pels_per_meter);
	LE32(obj->y_pels_per_meter);
	LE32(obj->clr_used);
	LE32(obj->clr_important);
}

#undef LE16
#undef LE32

MacResExtractor::MacResExtractor(ScummEngine_v70he *scumm) : ResExtractor(scumm) {
	_resMgr = NULL;
}

int MacResExtractor::extractResource(int id, byte **buf) {
	// Create the MacResManager if not created already
	if (_resMgr == NULL) {
		_resMgr = new Common::MacResManager();
		if (!_resMgr->open(_vm->generateFilename(-3)))
			error("Cannot open file %s", _fileName.c_str());
	}

	Common::SeekableReadStream *dataStream = _resMgr->getResource('crsr', 1000 + id);
	
	if (!dataStream)
		error("There is no cursor ID #%d", 1000 + id);
	
	uint32 size = dataStream->size();
	*buf = (byte *)malloc(size);
	dataStream->read(*buf, size);
	delete dataStream;

	return size;
}

int MacResExtractor::convertIcons(byte *data, int datasize, byte **cursor, int *w, int *h,
			  int *hotspot_x, int *hotspot_y, int *keycolor, byte **palette, int *palSize) {
			  
	_resMgr->convertCrsrCursor(data, datasize, cursor, w, h, hotspot_x, hotspot_y, keycolor,
						_vm->_system->hasFeature(OSystem::kFeatureCursorHasPalette), palette, palSize);
	return 1;
}

void ScummEngine_v70he::readRoomsOffsets() {
	int num, i;
	byte *ptr;

	debug(9, "readRoomOffsets()");

	num = READ_LE_UINT16(_heV7RoomOffsets);
	ptr = _heV7RoomOffsets + 2;
	for (i = 0; i < num; i++) {
		_res->roomoffs[rtRoom][i] = READ_LE_UINT32(ptr);
		ptr += 4;
	}
}

void ScummEngine_v70he::readGlobalObjects() {
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectStateTable, num);
	_fileHandle->read(_objectOwnerTable, num);
	_fileHandle->read(_objectRoomTable, num);

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (int i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}

#ifdef ENABLE_HE
void ScummEngine_v99he::readMAXS(int blockSize) {
	if (blockSize == 52) {
		debug(0, "ScummEngine_v99he readMAXS: MAXS has blocksize %d", blockSize);

		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_HEHeapSize = _fileHandle->readUint16LE();
		_numPalettes = _fileHandle->readUint16LE();
		_numUnk = _fileHandle->readUint16LE();
		_numTalkies = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2048;
	} else
		ScummEngine_v90he::readMAXS(blockSize);
}

void ScummEngine_v90he::readMAXS(int blockSize) {
	if (blockSize == 46) {
		debug(0, "ScummEngine_v90he readMAXS: MAXS has blocksize %d", blockSize);

		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numSprites = _fileHandle->readUint16LE();
		_numLocalScripts = _fileHandle->readUint16LE();
		_HEHeapSize = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		if (_game.features & GF_HE_985)
			_numGlobalScripts = 2048;
		else
			_numGlobalScripts = 200;
	} else
		ScummEngine_v72he::readMAXS(blockSize);
}

void ScummEngine_v72he::readMAXS(int blockSize) {
	if (blockSize == 40) {
		debug(0, "ScummEngine_v72he readMAXS: MAXS has blocksize %d", blockSize);

		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numBitVariables = _numRoomVariables = _fileHandle->readUint16LE();
		_numLocalObjects = _fileHandle->readUint16LE();
		_numArray = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numVerbs = _fileHandle->readUint16LE();
		_numFlObject = _fileHandle->readUint16LE();
		_numInventory = _fileHandle->readUint16LE();
		_numRooms = _fileHandle->readUint16LE();
		_numScripts = _fileHandle->readUint16LE();
		_numSounds = _fileHandle->readUint16LE();
		_numCharsets = _fileHandle->readUint16LE();
		_numCostumes = _fileHandle->readUint16LE();
		_numGlobalObjects = _fileHandle->readUint16LE();
		_numImages = _fileHandle->readUint16LE();
		_numNewNames = 10;

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 200;
	} else
		ScummEngine_v6::readMAXS(blockSize);
}

byte *ScummEngine_v72he::getStringAddress(int i) {
	byte *addr = getResourceAddress(rtString, i);
	if (addr == NULL)
		return NULL;
	return ((ScummEngine_v72he::ArrayHeader *)addr)->data;
}

int ScummEngine_v72he::getSoundResourceSize(int id) {
	const byte *ptr;
	int offs, size;

	if (id > _numSounds) {
		if (!((SoundHE *)_sound)->getHEMusicDetails(id, offs, size)) {
			debug(0, "getSoundResourceSize: musicID %d not found", id);
			return 0;
		}
	} else {
		ptr = getResourceAddress(rtSound, id);
		if (!ptr)
			return 0;

		if (READ_BE_UINT32(ptr) == MKID_BE('RIFF')) {
			byte flags;
			int rate;

			size = READ_BE_UINT32(ptr + 4);
			Common::MemoryReadStream stream(ptr, size);

			if (!Audio::loadWAVFromStream(stream, size, rate, flags)) {
				error("getSoundResourceSize: Not a valid WAV file");
			}
		} else {
			ptr += 8 + READ_BE_UINT32(ptr + 12);
			if (READ_BE_UINT32(ptr) == MKID_BE('SBNG')) {
				ptr += READ_BE_UINT32(ptr + 4);
			}

			assert(READ_BE_UINT32(ptr) == MKID_BE('SDAT'));
			size = READ_BE_UINT32(ptr + 4) - 8;
		}
	}

	return size;
}

#endif

} // End of namespace Scumm
