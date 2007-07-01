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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/file.h"

#include "gob/gob.h"
#include "gob/saveload.h"
#include "gob/global.h"
#include "gob/draw.h"
#include "gob/video.h"

namespace Gob {

SaveLoad::SaveLoad(GobEngine *vm, const char *targetName) : _vm(vm) {
	_curSlot = -1;

	_stagesCount = 0;
	_buffer = 0;

	_tempSprite = 0;
	memset(_tempPal, 0, 768);
	_tempSpriteSize = -1;

	_saveFiles = new char*[5];

	assert(_saveFiles);

	_saveFiles[0] = new char[strlen(targetName) + 5];
	_saveFiles[1] = 0;
	_saveFiles[2] = new char[strlen(targetName) + 5];
	_saveFiles[3] = _saveFiles[0];
	_saveFiles[4] = 0;

	assert(_saveFiles[0] && _saveFiles[2]);

	sprintf(_saveFiles[0], "%s.s00", targetName);
	sprintf(_saveFiles[2], "%s.blo", targetName);
}

SaveLoad::~SaveLoad() {
	for (int i = 0; i < _stagesCount; i++)
		delete[] _buffer[i];
	delete[] _buffer;

	delete _tempSprite;

	delete[] _saveFiles[0];
	delete[] _saveFiles[2];
	delete[] _saveFiles; 
}

const char *SaveLoad::setCurSlot(int slot) {
	static char *slotBase = _saveFiles[0] + strlen(_saveFiles[0]) - 2;

	if (_curSlot != slot) {
		_curSlot = slot;

		if (_curSlot >= 0)
			snprintf(slotBase, 3, "%02d", slot);
	}

	return _saveFiles[0];
}

uint32 SaveLoad::read(Common::ReadStream &in, byte *buf,
		byte *sizes, uint32 count) {
	uint32 nRead;

	nRead = in.read(buf, count);
	if (nRead != count) {
		warning("Can't read data: requested %d, got %d", count, nRead);
		return 0;
	}

	nRead = in.read(sizes, count);
	if (nRead != count) {
		warning("Can't read data sizes: requested %d, got %d", count, nRead);
		return 0;
	}

	return count;
}

uint32 SaveLoad::write(Common::WriteStream &out, byte *buf,
		byte *sizes, uint32 count) {
	uint32 written;

	written = out.write(buf, count);
	if (written != count) {
		warning("Can't write data: requested %d, wrote %d", count, written);
		return 0;
	}

	written = out.write(sizes, count);
	if (written != count) {
		warning("Can't write data: requested %d, wrote %d", count, written);
		return 0;
	}

	return count;
}

bool SaveLoad::loadDataEndian(Common::ReadStream &in,
		int16 dataVar, uint32 size) {

	bool retVal = false;

	byte *varBuf = new byte[size];
	byte *sizeBuf = new byte[size];

	assert(varBuf && sizeBuf);

	if (read(in, varBuf, sizeBuf, size) == size) {
		if (fromEndian(varBuf, sizeBuf, size)) {
			memcpy(_vm->_global->_inter_variables + dataVar, varBuf, size);
			memcpy(_vm->_global->_inter_variablesSizes + dataVar, sizeBuf, size);
			retVal = true;
		}
	}

	delete[] varBuf;
	delete[] sizeBuf;

	return retVal;
}

bool SaveLoad::saveDataEndian(Common::WriteStream &out,
		int16 dataVar, uint32 size) {

	bool retVal = false;

	byte *varBuf = new byte[size];
	byte *sizeBuf = new byte[size];

	assert(varBuf && sizeBuf);

	memcpy(varBuf, _vm->_global->_inter_variables + dataVar, size);
	memcpy(sizeBuf, _vm->_global->_inter_variablesSizes + dataVar, size);

	if (toEndian(varBuf, sizeBuf, size))
		if (write(out, varBuf, sizeBuf, size) == size)
			retVal = true;

	delete[] varBuf;
	delete[] sizeBuf;

	return retVal;
}

int32 SaveLoad::getSize(SaveType type) {
	switch(type) {
	case kSaveNone:
		return -1;
		break;

	case kSaveGame:
		return getSizeGame();
		break;

	case kSaveTempSprite:
		return getSizeTempSprite();
		break;

	case kSaveNotes:
		return getSizeNotes();
		break;

	case kSaveScreenshot:
		return getSizeScreenshot();
		break;

	case kSaveIgnore:
		return -1;
		break;
	}

	return -1;
}

bool SaveLoad::load(SaveType type, int16 dataVar, int32 size, int32 offset) {
	switch(type) {
	case kSaveNone:
		return false;
		break;

	case kSaveGame:
		return loadGame(dataVar, size, offset);
		break;

	case kSaveTempSprite:
		return loadTempSprite(dataVar, size, offset);
		break;

	case kSaveNotes:
		return loadNotes(dataVar, size, offset);
		break;

	case kSaveScreenshot:
		return loadScreenshot(dataVar, size, offset);
		break;

	case kSaveIgnore:
		return true;
		break;
	}

	return false;
}

bool SaveLoad::save(SaveType type, int16 dataVar, int32 size, int32 offset) {
	switch(type) {
	case kSaveNone:
		return false;
		break;

	case kSaveGame:
		return saveGame(dataVar, size, offset);
		break;

	case kSaveTempSprite:
		return saveTempSprite(dataVar, size, offset);
		break;

	case kSaveNotes:
		return saveNotes(dataVar, size, offset);
		break;

	case kSaveScreenshot:
		return saveScreenshot(dataVar, size, offset);
		break;

	case kSaveIgnore:
		return true;
		break;
	}

	return false;
}

int32 SaveLoad::getSizeTempSprite() {
	return _tempSpriteSize;
}

bool SaveLoad::loadTempSprite(int16 dataVar, int32 size, int32 offset) {
	int index;
	bool readPal;

	if (size >= 0) {
		warning("Invalid attempt at loading from the temporary sprite");
		return false;
	}

	index = getSpriteIndex(size);
	readPal = getSpritePalette(size);

	if ((index < 0) || (index >= SPRITES_COUNT)) {
		warning("Index out of range while loading from the temporary "
				"sprite (%d)", index);
		return false;
	}

	return loadTempSprite(index, readPal);
}

bool SaveLoad::saveTempSprite(int16 dataVar, int32 size, int32 offset) {
	int index;
	bool readPal;

	if (size >= 0) {
		warning("Invalid attempt at saving to the temporary sprite");
		return false;
	}

	index = getSpriteIndex(size);
	readPal = getSpritePalette(size);

	if ((index < 0) || (index >= SPRITES_COUNT)) {
		warning("Index out of range while saving to the temporary sprite (%d)",
				index);
		return false;
	}

	return saveTempSprite(index, readPal);
}

bool SaveLoad::loadTempSprite(uint32 index, bool palette) {
	SurfaceDesc *sprite;

	if (palette) {
		memcpy((char *) _vm->_global->_pPaletteDesc->vgaPal,
				(char *) _tempPal, 768);
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}

	sprite = _vm->_draw->_spritesArray[index];

	if (!sprite) {
		warning("Couldn't load from the temporary sprite: "
				"No such sprite %d", index);
		return false;
	}

	if ((sprite->getWidth() != _tempSprite->getWidth()) ||
			(sprite->getHeight() != _tempSprite->getHeight())) {
		warning("Resolution doesn't match while loading from the "
				"temporary sprite (%d: %dx%d vs. %dx%d)", index,
				sprite->getWidth(), sprite->getHeight(),
				_tempSprite->getWidth(), _tempSprite->getHeight());
		return false;
	}

	_vm->_video->drawSprite(_tempSprite, sprite, 0, 0,
			sprite->getWidth() - 1, sprite->getHeight() - 1, 0, 0, 0);

	if (index == 21) {
		_vm->_draw->forceBlit();
		_vm->_video->retrace();
	}

	return true;
}

bool SaveLoad::saveTempSprite(uint32 index, bool palette) {
	SurfaceDesc *sprite = _vm->_draw->_spritesArray[index];

	if (!sprite) {
		warning("Couldn't save to the temporary sprite: "
				"No such sprite %d", index);
		return false;
	}

	delete _tempSprite;
	_tempSprite = _vm->_video->initSurfDesc(_vm->_global->_videoMode,
			sprite->getWidth(), sprite->getHeight(), 0);

	_vm->_video->drawSprite(sprite, _tempSprite, 0, 0,
			sprite->getWidth() - 1, sprite->getHeight() - 1, 0, 0, 0);

	_tempSpriteSize = _vm->_draw->getSpriteRectSize(index);

	if (palette) {
		memcpy((char *) _tempPal,
				(char *) _vm->_global->_pPaletteDesc->vgaPal, 768);
		_tempSpriteSize += 768;
	}

	return true;
}

bool SaveLoad::loadSprite(Common::ReadStream &in, int32 size) {
	SurfaceDesc *sprite;
	byte *buf;
	int nRead;
	int index;
	bool readPal;

	if (size >= 0) {
		warning("Invalid attempt at loading a sprite");
		return false;
	}

	index = getSpriteIndex(size);
	readPal = getSpritePalette(size);

	if ((index < 0) || (index >= SPRITES_COUNT)) {
		warning("Index out of range while loading a sprite (%d)",
				index);
		return false;
	}

	size = _vm->_draw->getSpriteRectSize(index);
	sprite = _vm->_draw->_spritesArray[index];

	if (!sprite) {
		warning("Couldn't load sprite: No such sprite %d", index);
		return false;
	}

	buf = new byte[MAX<int>(768, size)];
	assert(buf);

	if (readPal) {
		nRead = in.read(buf, 768);
		if (nRead != 768) {
			warning("Couldn't read a palette: requested 768, got %d", nRead);
			delete[] buf;
			return false;
		}

		memcpy((char *) _vm->_global->_pPaletteDesc->vgaPal,
				(char *) buf, 768);
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}

	nRead = in.read(buf, size);
	if (nRead != size) {
		warning("Couldn't read sprite data: requested %d, got %d", size, nRead);
		delete[] buf;
		return false;
	}

	memcpy((char *) sprite->getVidMem(), buf, size);

	delete[] buf;
	return true;
}

bool SaveLoad::saveSprite(Common::WriteStream &out, int32 size) {
	SurfaceDesc *sprite;
	int written;
	int index;
	bool readPal;

	if (size >= 0) {
		warning("Invalid attempt at saving a sprite");
		return false;
	}

	index = getSpriteIndex(size);
	readPal = getSpritePalette(size);

	if ((index < 0) || (index >= SPRITES_COUNT)) {
		warning("Index out of range while saving a sprite (%d)",
				index);
		return false;
	}

	size = _vm->_draw->getSpriteRectSize(index);
	sprite = _vm->_draw->_spritesArray[index];

	if (!sprite) {
		warning("Couldn't save sprite: No such sprite %d", index);
		return false;
	}

	if (readPal) {
		written = out.write((char *) _vm->_global->_pPaletteDesc->vgaPal, 768);
		if (written != 768) {
			warning("Couldn't write a palette: requested 768, wrote %d", written);
			return false;
		}
	}

	written = out.write((char *) sprite->getVidMem(), size);
	if (written != size) {
		warning("Couldn't write a sprite: requested %d, wrote %d",
				size, written);
		return false;
	}

	return true;
}

bool SaveLoad::fromEndian(byte *buf, const byte *sizes, uint32 count) {
	while (count-- > 0) {
		if (*sizes == 3)
			*((uint32 *) buf) = READ_LE_UINT32(buf);
		else if (*sizes == 1)
			*((uint16 *) buf) = READ_LE_UINT16(buf);
		else if (*sizes != 0) {
			warning("SaveLoad::fromEndian(): Corrupted variables sizes");
			return false;
		}

		count -= *sizes;
		buf += *sizes + 1;
		sizes += *sizes + 1;
	}

	return true;
}

bool SaveLoad::toEndian(byte *buf, const byte *sizes, uint32 count) {
	while (count-- > 0) {
		if (*sizes == 3)
			WRITE_LE_UINT32(buf, *((uint32 *) buf));
		else if (*sizes == 1)
			WRITE_LE_UINT16(buf, *((uint16 *) buf));
		else if (*sizes != 0) {
			warning("SaveLoad::toEndian(): Corrupted variables sizes");
			return false;
		}

		count -= *sizes;
		buf += *sizes + 1;
		sizes += *sizes + 1;
	}

	return true;
}

} // End of namespace Gob
