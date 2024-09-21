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

#include "common/md5.h"
#include "common/str.h"
#include "common/memstream.h"
#include "common/macresman.h"
#ifndef MACOSX
#include "common/config-manager.h"
#endif

#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v5.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

enum {
	RF_LOCK = 0x80,
	RF_USAGE = 0x7F,
	RF_USAGE_MAX = RF_USAGE,

	RS_MODIFIED = 0x10,
	RF_OFFHEAP = 0x40
};



extern const char *nameOfResType(ResType type);

static uint16 newTag2Old(uint32 newTag);
static const byte *findResourceSmall(uint32 tag, const byte *searchin);

static bool checkTryMedia(BaseScummFile *handle);


/* Open a room */
void ScummEngine::openRoom(const int room) {
	bool result;
	byte encByte = 0;

	debugC(DEBUG_GENERAL, "openRoom(%d)", room);
	assert(room >= 0);

	/* Don't load the same room again */
	if (_lastLoadedRoom == room)
		return;
	_lastLoadedRoom = room;

	/* Room -1 means close file */
	if (room == -1) {
		deleteRoomOffsets();
		_fileHandle->close();
		return;
	}

	// Load the disk numer / room offs (special case for room 0 exists because
	// room 0 contains the data which is used to create the roomno / roomoffs
	// tables -- hence obviously we mustn't use those when loading room 0.
	const uint32 diskNumber = room ? _res->_types[rtRoom][room]._roomno : 0;
	const uint32 room_offs = room ? _res->_types[rtRoom][room]._roomoffs : 0;

	// FIXME: Since room_offs is const, clearly the following loop either
	// is never entered, or loops forever (if it wasn't for the return/error
	// statements in it, that is). -> This should be cleaned up!
	// Maybe we should re-enabled the looping properly, to deal with disc
	// changes in COMI ?
	while (room_offs != RES_INVALID_OFFSET) {

		if (room_offs != 0 && room != 0 && _game.heversion < 98) {
			_fileOffset = _res->_types[rtRoom][room]._roomoffs;
			return;
		}

		Common::Path filename(generateFilename(room));

		// Determine the encryption, if any.
		if (_game.features & GF_USE_KEY) {
			if (_game.version <= 3)
				encByte = 0xFF;
			else if ((_game.version == 4) && (room == 0 || room >= 900))
				encByte = 0;
			else
				encByte = 0x69;
		} else
			encByte = 0;

		if (room > 0 && (_game.version == 8))
			VAR(VAR_CURRENTDISK) = diskNumber;

		// Try to open the file
		result = openResourceFile(filename, encByte);

		if (result) {
			if (room == 0)
				return;
			deleteRoomOffsets();
			readRoomsOffsets();
			_fileOffset = _res->_types[rtRoom][room]._roomoffs;

			if (_fileOffset != 8)
				return;

			error("Room %d not in %s", room, filename.toString().c_str());
			return;
		}
		askForDisk(filename, diskNumber);
	}

	do {
		char buf[16];
		snprintf(buf, sizeof(buf), "%.3d.lfl", room);
		encByte = 0;
		if (openResourceFile(buf, encByte))
			break;
		askForDisk(buf, diskNumber);
	} while (1);

	deleteRoomOffsets();
	_fileOffset = 0;		// start of file
}

void ScummEngine::closeRoom() {
	if (_lastLoadedRoom != -1) {
		_lastLoadedRoom = -1;
		deleteRoomOffsets();
		_fileHandle->close();
	}
}

/** Delete the currently loaded room offsets. */
void ScummEngine::deleteRoomOffsets() {
	for (int i = 0; i < _numRooms; i++) {
		if (_res->_types[rtRoom][i]._roomoffs != RES_INVALID_OFFSET)
			_res->_types[rtRoom][i]._roomoffs = 0;
	}
}

/** Read room offsets */
void ScummEngine::readRoomsOffsets() {
	if (_game.features & GF_SMALL_HEADER) {
		_fileHandle->seek(12, SEEK_SET);	// Directly searching for the room offset block would be more generic...
	} else {
		_fileHandle->seek(16, SEEK_SET);
	}

	int num = _fileHandle->readByte();
	while (num--) {
		int room = _fileHandle->readByte();
		int offset =  _fileHandle->readUint32LE();
		if (_res->_types[rtRoom][room]._roomoffs != RES_INVALID_OFFSET) {
			_res->_types[rtRoom][room]._roomoffs = offset;
		}
	}
}

bool ScummEngine::openFile(BaseScummFile &file, const Common::Path &filename, bool resourceFile) {
	bool result = false;

	if (!_containerFile.empty()) {
		file.close();
		file.open(_containerFile);
		assert(file.isOpen());

		result = file.openSubFile(filename);
	}

	if (!result) {
		file.close();
		result = file.open(filename);
	}

	return result;
}

bool ScummEngine::openResourceFile(const Common::Path &filename, byte encByte) {
	debugC(DEBUG_GENERAL, "openResourceFile(%s)", filename.toString().c_str());

	if (openFile(*_fileHandle, filename, true)) {
		_fileHandle->setEnc(encByte);
		return true;
	}
	return false;
}

void ScummEngine::askForDisk(const Common::Path &filename, int disknum) {
	char buf[128];

	if (_game.version == 8) {
#ifdef ENABLE_SCUMM_7_8
		char result;

		_imuseDigital->stopAllSounds();

#ifdef MACOSX
		Common::sprintf_s(buf, "Cannot find file: '%s'\nPlease insert disc %d.\nPress OK to retry, Quit to exit", filename.toString(Common::Path::kNativeSeparator).c_str(), disknum);
#else
		Common::sprintf_s(buf, "Cannot find file: '%s'\nInsert disc %d into drive %s\nPress OK to retry, Quit to exit", filename.toString(Common::Path::kNativeSeparator).c_str(), disknum,
				ConfMan.getPath("path").toString(Common::Path::kNativeSeparator).c_str());
#endif

		result = displayMessage("Quit", "%s", buf);
		if (!result) {
			error("Cannot find file: '%s'", filename.toString(Common::Path::kNativeSeparator).c_str());
		}
#endif
	} else {
		Common::sprintf_s(buf, "Cannot find file: '%s'", filename.toString(Common::Path::kNativeSeparator).c_str());
		InfoDialog dialog(this, Common::U32String(buf));
		runDialog(dialog);
		error("Cannot find file: '%s'", filename.toString(Common::Path::kNativeSeparator).c_str());
	}
}

void ScummEngine::readIndexFile() {
	uint32 blocktype, itemsize;

	debugC(DEBUG_GENERAL, "readIndexFile()");

	closeRoom();
	openRoom(0);

	if (_game.version <= 5) {
		// Figure out the sizes of various resources
		while (true) {
			blocktype = _fileHandle->readUint32BE();
			itemsize = _fileHandle->readUint32BE();
			if (_fileHandle->eos() || _fileHandle->err())
				break;
			switch (blocktype) {
			case MKTAG('D','O','B','J'):
				_numGlobalObjects = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;
			case MKTAG('D','R','O','O'):
				_numRooms = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKTAG('D','S','C','R'):
				_numScripts = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKTAG('D','C','O','S'):
				_numCostumes = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKTAG('D','S','O','U'):
				_numSounds = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			default:
				break;
			}
			_fileHandle->seek(itemsize - 8, SEEK_CUR);
		}
		_fileHandle->seek(0, SEEK_SET);
	}

	if (checkTryMedia(_fileHandle)) {
		displayMessage(nullptr, "You're trying to run game encrypted by ActiveMark. This is not supported.");
		quitGame();

		return;
	}

	while (true) {
		blocktype = _fileHandle->readUint32BE();
		itemsize = _fileHandle->readUint32BE();

		if (_fileHandle->eos() || _fileHandle->err())
			break;

		debug(2, "Reading index block of type '%s', size %d", tag2str(blocktype), itemsize);
		readIndexBlock(blocktype, itemsize);
	}

//  if (numblock!=9)
//    error("Not enough blocks read from directory");

	closeRoom();
}



#define TRYMEDIA_MARK_LEN 6

bool checkTryMedia(BaseScummFile *handle) {
	byte buf[TRYMEDIA_MARK_LEN];
	bool matched = true;
	const byte magic[2][TRYMEDIA_MARK_LEN] =
		{{ 0x00,  'T', 'M', 'S', 'A', 'M' },
		 { 'i',   '=', '$', ':', '(', '$' }};  // Same but 0x69 xored

	handle->read(buf, TRYMEDIA_MARK_LEN);

	for (int i = 0; i < 2; i++) {
		matched = true;
		for (int j = 0; j < TRYMEDIA_MARK_LEN; j++)
			if (buf[j] != magic[i][j]) {
				matched = false;
				break;
			}

		if (matched)
			break;
	}

	if (matched)
		return true;

	handle->seek(0, SEEK_SET);

	return false;
}


#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::readIndexBlock(uint32 blocktype, uint32 itemsize) {
	int num;
	char *ptr;
	switch (blocktype) {
	case MKTAG('A','N','A','M'):		// Used by: The Dig, FT
		num = _fileHandle->readUint16LE();
		ptr = (char *)malloc(num * 9);
		_fileHandle->read(ptr, num * 9);
		_imuseDigital->setAudioNames(num, ptr);
		break;

	case MKTAG('D','R','S','C'):		// Used by: COMI
		readResTypeList(rtRoomScripts);
		break;

	default:
		ScummEngine::readIndexBlock(blocktype, itemsize);
	}
}
#endif

void ScummEngine_v70he::readIndexBlock(uint32 blocktype, uint32 itemsize) {
	int i;
	switch (blocktype) {
	case MKTAG('D','I','R','I'):
		readResTypeList(rtRoomImage);
		break;

	case MKTAG('D','I','R','M'):
		readResTypeList(rtImage);
		break;

	case MKTAG('D','I','R','T'):
		readResTypeList(rtTalkie);
		break;

	case MKTAG('D','L','F','L'):
		i = _fileHandle->readUint16LE();
		_fileHandle->seek(-2, SEEK_CUR);
		_heV7RoomOffsets = (byte *)reallocateArray(_heV7RoomOffsets, 2 + (i * 4), 1);
		_fileHandle->read(_heV7RoomOffsets, (2 + (i * 4)) );
		break;

	case MKTAG('D','I','S','K'):
		i = _fileHandle->readUint16LE();
		_heV7DiskOffsets = (byte *)reallocateArray(_heV7DiskOffsets, i, 1);
		_fileHandle->read(_heV7DiskOffsets, i);
		break;

	case MKTAG('S','V','E','R'):
		// Index version number
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
		break;

	case MKTAG('I','N','I','B'):
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
		debug(2, "INIB index block not yet handled, skipping");
		break;

	default:
		ScummEngine::readIndexBlock(blocktype, itemsize);
	}
}

void ScummEngine::readIndexBlock(uint32 blocktype, uint32 itemsize) {
	int i;
	switch (blocktype) {
	case MKTAG('D','C','H','R'):
	case MKTAG('D','I','R','F'):
		readResTypeList(rtCharset);
		break;

	case MKTAG('D','O','B','J'):
		readGlobalObjects();
		break;

	case MKTAG('R','N','A','M'):
		// Names of rooms. Maybe we should put them into a table, for use by the debugger?
		if (_game.heversion >= 80) {
			for (int room; (room = _fileHandle->readUint16LE()); ) {
				char buf[100];
				i = 0;
				for (byte s; (s = _fileHandle->readByte()) && i < ARRAYSIZE(buf) - 1; ) {
					buf[i++] = s;
				}
				buf[i] = 0;
				debug(5, "Room %d: '%s'", room, buf);
			}
		} else {
			for (int room; (room = _fileHandle->readByte()); ) {
				char buf[10];
				_fileHandle->read(buf, 9);
				buf[9] = 0;
				for (i = 0; i < 9; i++)
					buf[i] ^= 0xFF;
				debug(5, "Room %d: '%s'", room, buf);
			}
		}
		break;

	case MKTAG('D','R','O','O'):
	case MKTAG('D','I','R','R'):
		readResTypeList(rtRoom);
		break;

	case MKTAG('D','S','C','R'):
	case MKTAG('D','I','R','S'):
		readResTypeList(rtScript);
		break;

	case MKTAG('D','C','O','S'):
	case MKTAG('D','I','R','C'):
		readResTypeList(rtCostume);
		break;

	case MKTAG('M','A','X','S'):
		readMAXS(itemsize);
		allocateArrays();
		break;

	case MKTAG('D','I','R','N'):
	case MKTAG('D','S','O','U'):
		readResTypeList(rtSound);
		break;

	case MKTAG('A','A','R','Y'):
		readArrayFromIndexFile();
		break;

	default:
		error("Bad ID %04X('%s') found in index file directory", blocktype,
				tag2str(blocktype));
	}
}

void ScummEngine::readArrayFromIndexFile() {
	error("readArrayFromIndexFile() not supported in pre-V6 games");
}

int ScummEngine::readResTypeList(ResType type) {
	uint num;
	ResId idx;

	if (_game.version == 8)
		num = _fileHandle->readUint32LE();
	else
		num = _fileHandle->readUint16LE();

	if (num != _res->_types[type].size()) {
		error("Invalid number of %ss (%d) in directory", nameOfResType(type), num);
	}

	debug(2, "  readResTypeList(%s): %d entries", nameOfResType(type), num);


	for (idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomno = _fileHandle->readByte();
	}
	for (idx = 0; idx < num; idx++) {
		_res->_types[type][idx]._roomoffs = _fileHandle->readUint32LE();
	}

	return num;
}

int ScummEngine_v70he::readResTypeList(ResType type) {
	uint num;
	ResId idx;

	num = ScummEngine::readResTypeList(type);

	if (type == rtRoom)
		for (idx = 0; idx < num; idx++) {
			_heV7RoomIntOffsets[idx] = _res->_types[rtRoom][idx]._roomoffs;
		}

	for (idx = 0; idx < num; idx++) {
		// The globsize is currently not being used
		/*_res->_types[type][idx]._globsize =*/ _fileHandle->readUint32LE();
	}

	return num;
}

void ResourceManager::allocResTypeData(ResType type, uint32 tag, int num, ResTypeMode mode) {
	debug(2, "allocResTypeData(%s,%s,%d,%d)", nameOfResType(type), tag2str(TO_BE_32(tag)), num, mode);
	assert(type >= 0 && type < (int)(ARRAYSIZE(_types)));

	if (num >= 8000)
		error("Too many %s resources (%d) in directory", nameOfResType(type), num);

	_types[type]._mode = mode;
	_types[type]._tag = tag;

	// If there was data in there, let's clear it out completely. This is important
	// in case we are restarting the game.
	_types[type].clear();
	_types[type].resize(num);

/*
	TODO: Use multiple Resource subclasses, one for each res mode; then,
	given them serializability.
	if (mode) {
		_types[type].roomno = (byte *)calloc(num, sizeof(byte));
		_types[type].roomoffs = (uint32 *)calloc(num, sizeof(uint32));
	}

	if (_vm->_game.heversion >= 70) {
		_types[type].globsize = (uint32 *)calloc(num, sizeof(uint32));
	}
*/
}

void ScummEngine::loadCharset(int no) {
	int i;
	byte *ptr;

	debugC(DEBUG_GENERAL, "loadCharset(%d)", no);

	/* FIXME - hack around crash in Indy4 (occurs if you try to load after dying) */
	if (_game.id == GID_INDY4 && no == 0)
		no = 1;

	/* for Humongous catalogs */
	if (_game.heversion >= 70 && _numCharsets == 1) {
		debug(0, "Not loading charset as it doesn't seem to exist?");
		return;
	}

	assert(no < (int)sizeof(_charsetData) / 16);
	assertRange(1, no, _numCharsets - 1, "charset");

	ptr = getResourceAddress(rtCharset, no);

	for (i = 0; i < 15; i++) {
		_charsetData[no][i + 1] = ptr[i + 14];
	}
}

void ScummEngine::nukeCharset(int i) {
	assertRange(1, i, _numCharsets - 1, "charset");
	_res->nukeResource(rtCharset, i);
}

void ScummEngine::ensureResourceLoaded(ResType type, ResId idx) {
	Common::StackLock lock(_resourceAccessMutex);

	debugC(DEBUG_RESOURCE, "ensureResourceLoaded(%s,%d)", nameOfResType(type), idx);

	if ((type == rtRoom) && idx > 0x7F && _game.version < 7 && _game.heversion <= 71) {
		idx = _resourceMapper[idx & 0x7F];
	}

	// FIXME: This check used to be "idx==0". However, that causes
	// problems when using this function to ensure charset 0 is loaded.
	// This is done for many games, e.g. Zak256 or Indy3 (EGA and VGA).
	// For now we restrict the check to anything which is not a charset.
	// Question: Why was this check like that in the first place?
	// Answer: costumes with an index of zero in the newer games at least.
	// TODO: determine why the heck anything would try to load a costume
	// with id 0. Is that "normal", or is it caused by yet another bug in
	// our code base? After all we also have to add special cases for many
	// of our script opcodes that check for the (invalid) actor 0... so
	// maybe both issues are related...
	if (type != rtCharset && idx == 0)
		return;

	if (idx <= _res->_types[type].size() && _res->_types[type][idx]._address)
		return;

	#ifdef ENABLE_SCUMM_7_8
	_resourceAccessMutex.unlock();

	if (_imuseDigital) {
		int32 bufSize, criticalSize, freeSpace;
		int paused;
		if (_imuseDigital->isFTSoundEngine() && _imuseDigital->queryNextSoundFile(bufSize, criticalSize, freeSpace, paused)) {
			_imuseDigital->fillStreamsWhileMusicCritical(5);
		} else {
			_imuseDigital->fillStreamsWhileMusicCritical(_game.id == GID_DIG ? 30 : 20);
		}
	}

	_resourceAccessMutex.lock();
#endif

	loadResource(type, idx);

	if (_game.version == 5 && type == rtRoom && (int)idx == _roomResource)
		VAR(VAR_ROOM_FLAG) = 1;
}

int ScummEngine::loadResource(ResType type, ResId idx) {
	int roomNr;
	uint32 fileOffs;
	uint32 size, tag;

	debugC(DEBUG_RESOURCE, "loadResource(%s,%d)", nameOfResType(type), idx);

	if (type == rtCharset && (_game.features & GF_SMALL_HEADER)) {
		loadCharset(idx);
		return 1;
	}

	roomNr = getResourceRoomNr(type, idx);

	if (idx >= _res->_types[type].size())
		error("%s %d undefined %d %d", nameOfResType(type), idx, _res->_types[type].size(), roomNr);

	if (roomNr == 0)
		roomNr = _roomResource;

	fileOffs = getResourceRoomOffset(type, idx);
	if (fileOffs == RES_INVALID_OFFSET)
		return 0;

	openRoom(roomNr);

	_fileHandle->seek(fileOffs + _fileOffset, SEEK_SET);

	if (_game.features & GF_OLD_BUNDLE) {
		if ((_game.version == 3) && !(_game.platform == Common::kPlatformAmiga) && (type == rtSound)) {
			return readSoundResourceSmallHeader(idx);
		} else {
			// WORKAROUND: Apple //gs MM has malformed sound resource #68
			if (_fileHandle->pos() + 2 > _fileHandle->size()) {
				warning("loadResource(%s,%d): resource is too short", nameOfResType(type), idx);
				size = 0;
			} else {
				size = _fileHandle->readUint16LE();
				_fileHandle->seek(-2, SEEK_CUR);
			}
		}
	} else if (_game.features & GF_SMALL_HEADER) {
		if (_game.version == 4)
			_fileHandle->seek(8, SEEK_CUR);
		size = _fileHandle->readUint32LE();
		tag = _fileHandle->readUint16LE();
		_fileHandle->seek(-6, SEEK_CUR);
		if ((type == rtSound) && !(_game.platform == Common::kPlatformAmiga) && !(_game.platform == Common::kPlatformFMTowns)) {
			return readSoundResourceSmallHeader(idx);
		}
	} else {
		if (type == rtSound) {
			return readSoundResource(idx);
		}

		// Sanity check: Is this the right tag for this resource type?
		//
		// Currently disabled for newer HE games because they use different
		// tags. For example, for rtRoom, 'ROOM' changed to 'RMDA'; and for
		// rtImage, 'AWIZ' and 'MULT' can both occur simultaneously.
		// On the long run, it would be preferable to not turn this check off,
		// but instead to explicitly support the variations in the HE games.
		tag = _fileHandle->readUint32BE();
		if (tag != _res->_types[type]._tag && _game.heversion < 70) {
			error("Unknown res tag '%s' encountered (expected '%s') "
			        "while trying to load res (%s,%d) in room %d at %d+%d in file %s",
			        tag2str(tag), tag2str(_res->_types[type]._tag),
					nameOfResType(type), idx, roomNr,
			                _fileOffset, fileOffs, _fileHandle->getDebugName().c_str());
		}

		size = _fileHandle->readUint32BE();
		_fileHandle->seek(-8, SEEK_CUR);
	}
	_fileHandle->read(_res->createResource(type, idx, size), size);

	applyWorkaroundIfNeeded(type, idx);

	// NB: The workaround may have changed the resource size, so don't rely on 'size' after this.

	// dump the resource if requested
	if (_dumpScripts && type == rtScript) {
		dumpResource("script-", idx, getResourceAddress(rtScript, idx));
	}

	if (_fileHandle->err() || _fileHandle->eos()) {
		error("Cannot read resource");
	}

	return 1;
}

int ScummEngine::getResourceRoomNr(ResType type, ResId idx) {
	if (type == rtRoom && _game.heversion < 70)
		return idx;
	return _res->_types[type][idx]._roomno;
}

uint32 ScummEngine::getResourceRoomOffset(ResType type, ResId idx) {
	if (type == rtRoom) {
		return (_game.version == 8) ? 8 : 0;
	}
	return _res->_types[type][idx]._roomoffs;
}

uint32 ScummEngine_v70he::getResourceRoomOffset(ResType type, ResId idx) {
	if (type == rtRoom) {
		return _heV7RoomIntOffsets[idx];
	}
	return _res->_types[type][idx]._roomoffs;
}

int ScummEngine::getResourceSize(ResType type, ResId idx) {
	Common::StackLock lock(_resourceAccessMutex);
	byte *ptr = getResourceAddress(type, idx);
	assert(ptr);
	return _res->_types[type][idx]._size;
}

byte *ScummEngine::getResourceAddress(ResType type, ResId idx) {
	Common::StackLock lock(_resourceAccessMutex);
	byte *ptr;

	if (_game.heversion >= 80 && type == rtString)
		idx &= ~MAGIC_ARRAY_NUMBER;

	if (!_res->validateResource("getResourceAddress", type, idx))
		return nullptr;

	// If the resource is missing, but loadable from the game data files, try to do so.
	if (!_res->_types[type][idx]._address && _res->_types[type]._mode != kDynamicResTypeMode) {
		ensureResourceLoaded(type, idx);
	}

	ptr = (byte *)_res->_types[type][idx]._address;
	if (!ptr) {
		debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d) == NULL", nameOfResType(type), idx);
		return nullptr;
	}

	_res->setResourceCounter(type, idx, 1);

	debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d) == %p", nameOfResType(type), idx, (void *)ptr);
	return ptr;
}

byte *ScummEngine::getStringAddress(ResId idx) {
	byte *addr = getResourceAddress(rtString, idx);
	return addr;
}

byte *ScummEngine_v6::getStringAddress(ResId idx) {
	byte *addr = getResourceAddress(rtString, idx);
	if (addr == nullptr)
		return nullptr;
	// Skip over the ArrayHeader
	return addr + 6;
}

byte *ScummEngine::getStringAddressVar(int i) {
	return getStringAddress(_scummVars[i]);
}

void ResourceManager::increaseExpireCounter() {
	++_expireCounter;
	if (_expireCounter == 0) {	// overflow?
		increaseResourceCounters();
	}
}

void ResourceManager::increaseResourceCounters() {
	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
		ResId idx = _types[type].size();
		while (idx-- > 0) {
			byte counter = _types[type][idx].getResourceCounter();
			if (counter && counter < RF_USAGE_MAX) {
				setResourceCounter(type, idx, counter + 1);
			}
		}
	}
}

void ResourceManager::setResourceCounter(ResType type, ResId idx, byte counter) {
	_types[type][idx].setResourceCounter(counter);
}

void ResourceManager::Resource::setResourceCounter(byte counter) {
	_flags &= RF_LOCK;	// Clear lower 7 bits, preserve the lock bit.
	_flags |= counter;	// Update the usage counter
}

byte ResourceManager::Resource::getResourceCounter() const {
	return _flags & RF_USAGE;
}

/* 2 bytes safety area to make "precaching" of bytes in the gdi drawer easier */
#define SAFETY_AREA 2

byte *ResourceManager::createResource(ResType type, ResId idx, uint32 size) {
	debugC(DEBUG_RESOURCE, "_res->createResource(%s,%d,%d)", nameOfResType(type), idx, size);

	_vm->_insideCreateResource++; // For the HE sound engine

	if (!validateResource("allocating", type, idx)) {
		_vm->_insideCreateResource--;
		return nullptr;
	}

	if (_vm->_game.version <= 2) {
		// Nuking and reloading a resource can be harmful in some
		// cases. For instance, Zak tries to reload the intro music
		// while it's playing. See bug #2115.

		if (_types[type][idx]._address && (type == rtSound || type == rtScript || type == rtCostume))
			return _types[type][idx]._address;
	}

	nukeResource(type, idx);

	expireResources(size);

	byte *ptr = new byte[size + SAFETY_AREA]();
	if (ptr == nullptr) {
		error("createResource(%s,%d): Out of memory while allocating %d", nameOfResType(type), idx, size);
	}

	_allocatedSize += size;

	_types[type][idx]._address = ptr;
	_types[type][idx]._size = size;
	setResourceCounter(type, idx, 1);

	_vm->_insideCreateResource--;

	return ptr;
}

ResourceManager::Resource::Resource() {
	_address = nullptr;
	_size = 0;
	_flags = 0;
	_status = 0;
	_roomno = 0;
	_roomoffs = 0;
}

ResourceManager::Resource::~Resource() {
	delete[] _address;
	_address = nullptr;
}

void ResourceManager::Resource::nuke() {
	delete[] _address;
	_address = nullptr;
	_size = 0;
	_flags = 0;
	_status &= ~RS_MODIFIED;
}

ResourceManager::ResTypeData::ResTypeData() {
	_mode = kDynamicResTypeMode;
	_tag = 0;
}

ResourceManager::ResTypeData::~ResTypeData() {
}

ResourceManager::ResourceManager(ScummEngine *vm) : _vm(vm) {
	_allocatedSize = 0;
	_maxHeapThreshold = 0;
	_minHeapThreshold = 0;
	_expireCounter = 0;
}

ResourceManager::~ResourceManager() {
	freeResources();
}

void ResourceManager::setHeapThreshold(int min, int max) {
	assert(0 < max);
	assert(min <= max);
	_maxHeapThreshold = max;
	_minHeapThreshold = min;
}

bool ResourceManager::validateResource(const char *str, ResType type, ResId idx) const {
	if (type < rtFirst || type > rtLast || (uint)idx >= (uint)_types[type].size()) {
		warning("%s Illegal Glob type %s (%d) num %d", str, nameOfResType(type), type, idx);
		return false;
	}
	return true;
}

void ResourceManager::nukeResource(ResType type, ResId idx) {
	byte *ptr = _types[type][idx]._address;
	if (ptr != nullptr) {
		debugC(DEBUG_RESOURCE, "nukeResource(%s,%d)", nameOfResType(type), idx);
		_allocatedSize -= _types[type][idx]._size;
		_types[type][idx].nuke();
	}
}

const byte *ScummEngine::findResourceData(uint32 tag, const byte *ptr) {
	if (_game.features & GF_OLD_BUNDLE)
		error("findResourceData must not be used in GF_OLD_BUNDLE games");
	else if (_game.features & GF_SMALL_HEADER)
		ptr = findResourceSmall(tag, ptr);
	else
		ptr = findResource(tag, ptr);

	if (ptr == nullptr)
		return nullptr;
	return ptr + _resourceHeaderSize;
}

int ScummEngine::getResourceDataSize(const byte *ptr) const {
	if (ptr == nullptr)
		return 0;

	if (_game.features & GF_OLD_BUNDLE)
		return READ_LE_UINT16(ptr) - _resourceHeaderSize;
	else if (_game.features & GF_SMALL_HEADER)
		return READ_LE_UINT32(ptr) - _resourceHeaderSize;
	else
		return READ_BE_UINT32(ptr - 4) - _resourceHeaderSize;
}

void ResourceManager::lock(ResType type, ResId idx) {
	if (!validateResource("Locking", type, idx))
		return;
	_types[type][idx].lock();
}

void ResourceManager::unlock(ResType type, ResId idx) {
	if (!validateResource("Unlocking", type, idx))
		return;
	_types[type][idx].unlock();
}

bool ResourceManager::isLocked(ResType type, ResId idx) const {
	if (!validateResource("isLocked", type, idx))
		return false;
	return _types[type][idx].isLocked();
}

void ResourceManager::Resource::lock() {
	_flags |= RF_LOCK;
}

void ResourceManager::Resource::unlock() {
	_flags &= ~RF_LOCK;
}

bool ResourceManager::Resource::isLocked() const {
	return (_flags & RF_LOCK) != 0;
}

bool ScummEngine::isResourceInUse(ResType type, ResId idx) const {
	if (!_res->validateResource("isResourceInUse", type, idx))
		return false;
	switch (type) {
	case rtRoom:
		return _roomResource == (byte)idx;
	case rtRoomImage:
		return _roomResource == (byte)idx;
	case rtRoomScripts:
		return _roomResource == (byte)idx;
	case rtScript:
		return isScriptInUse(idx);
	case rtCostume:
		return isCostumeInUse(idx);
	case rtSound:
		// Sound resource 1 is used for queued speech
		if (_game.heversion >= 60 && idx == 1)
			return true;
		else
			return _sound->isSoundInUse(idx);
	case rtCharset:
		return _charset->getCurID() == (int)idx;
	case rtImage:
		return _res->isModified(type, idx) != 0;
	case rtSpoolBuffer:
		return _sound->isSoundRunning(10000 + idx) != 0;
	default:
		return false;
	}
}

void ResourceManager::setModified(ResType type, ResId idx) {
	if (!validateResource("Modified", type, idx))
		return;
	_types[type][idx].setModified();
}

void ResourceManager::setOffHeap(ResType type, ResId idx) {
	if (!validateResource("setOffHeap", type, idx))
		return;
	_types[type][idx].setOffHeap();
}

void ResourceManager::setOnHeap(ResType type, ResId idx) {
	if (!validateResource("setOnHeap", type, idx))
		return;
	_types[type][idx].setOnHeap();
}

bool ResourceManager::isModified(ResType type, ResId idx) const {
	if (!validateResource("isModified", type, idx))
		return false;
	return _types[type][idx].isModified();
}

bool ResourceManager::isOffHeap(ResType type, ResId idx) const {
	if (!validateResource("isOffHeap", type, idx))
		return false;
	return _types[type][idx].isOffHeap();
}

bool ResourceManager::Resource::isModified() const {
	return (_status & RS_MODIFIED) != 0;
}

bool ResourceManager::Resource::isOffHeap() const {
	return (_status & RF_OFFHEAP) != 0;
}

void ResourceManager::Resource::setModified() {
	_status |= RS_MODIFIED;
}

void ResourceManager::Resource::setOffHeap() {
	_status |= RF_OFFHEAP;
}

void ResourceManager::Resource::setOnHeap() {
	_status &= ~RF_OFFHEAP;
}

void ResourceManager::expireResources(uint32 size) {
	byte best_counter;
	ResType best_type;
	int best_res = 0;
	uint32 oldAllocatedSize;

	if (_expireCounter != 0xFF) {
		_expireCounter = 0xFF;
		increaseResourceCounters();
	}

	if (size + _allocatedSize < _maxHeapThreshold)
		return;

	oldAllocatedSize = _allocatedSize;

	do {
		best_type = rtInvalid;
		best_counter = 2;

		for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
			if (_types[type]._mode != kDynamicResTypeMode) {
				// Resources of this type can be reloaded from the data files,
				// so we can potentially unload them to free memory.
				ResId idx = _types[type].size();
				while (idx-- > 0) {
					Resource &tmp = _types[type][idx];
					byte counter = tmp.getResourceCounter();
					if (!tmp.isLocked() && counter >= best_counter && tmp._address && !_vm->isResourceInUse(type, idx) && !tmp.isOffHeap()) {
						best_counter = counter;
						best_type = type;
						best_res = idx;
					}
				}
			}
		}

		if (!best_type)
			break;
		nukeResource(best_type, best_res);
	} while (size + _allocatedSize > _minHeapThreshold);

	increaseResourceCounters();

	debugC(DEBUG_RESOURCE, "Expired resources, mem %d -> %d", oldAllocatedSize, _allocatedSize);
}

void ResourceManager::freeResources() {
	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
		ResId idx = _types[type].size();
		while (idx-- > 0) {
			if (isResourceLoaded(type, idx))
				nukeResource(type, idx);
		}
		_types[type].clear();
	}
}

void ScummEngine::loadPtrToResource(ResType type, ResId idx, const byte *source) {
	byte *alloced;
	int len;

	bool sourceWasNull = !source;
	int originalLen;

	_res->nukeResource(type, idx);

	len = resStrLen(source) + 1;
	if (len <= 0)
		return;

	originalLen = len;

	// Translate resource text
	byte translateBuffer[512];
	if (isScummvmKorTarget()) {
		if (!source) {
			refreshScriptPointer();
			source = _scriptPointer;
		}
		translateText(source, translateBuffer, sizeof(translateBuffer));

		source = translateBuffer;
		len = resStrLen(source) + 1;
	}

	alloced = _res->createResource(type, idx, len);

	if (!source) {
		// Need to refresh the script pointer, since createResource may
		// have caused the script resource to expire.
		refreshScriptPointer();
		memcpy(alloced, _scriptPointer, originalLen);
		_scriptPointer += originalLen;
	} else if (sourceWasNull) {
		refreshScriptPointer();
		memcpy(alloced, source, len);
		_scriptPointer += originalLen;
	} else {
		memcpy(alloced, source, len);
	}
}

bool ResourceManager::isResourceLoaded(ResType type, ResId idx) const {
	if (!validateResource("isResourceLoaded", type, idx))
		return false;
	return _types[type][idx]._address != nullptr;
}

void ResourceManager::resourceStats() {
	uint32 lockedSize = 0, lockedNum = 0;

	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
		ResId idx = _types[type].size();
		while (idx-- > 0) {
			Resource &tmp = _types[type][idx];
			if (tmp.isLocked() && tmp._address) {
				lockedSize += tmp._size;
				lockedNum++;
			}
		}
	}

	debug(1, "Total allocated size=%d, locked=%d(%d)", _allocatedSize, lockedSize, lockedNum);
}

void ScummEngine_v5::readMAXS(int blockSize) {
	_numVariables = _fileHandle->readUint16LE();      // 800
	_fileHandle->readUint16LE();                      // 16
	_numBitVariables = _fileHandle->readUint16LE();   // 2048
	_numLocalObjects = _fileHandle->readUint16LE();   // 200
	_numArray = 50;
	_numVerbs = 100;
	// Used to be 50, which wasn't enough for MI2 and FOA. See bugs
	// #1591, #1600 and #1607.
	_numNewNames = 150;
	_objectRoomTable = nullptr;

	_fileHandle->readUint16LE();                      // 50
	_numCharsets = _fileHandle->readUint16LE();       // 9
	_fileHandle->readUint16LE();                      // 100
	_fileHandle->readUint16LE();                      // 50
	_numInventory = _fileHandle->readUint16LE();      // 80
	_numGlobalScripts = 200;

	_shadowPaletteSize = 256;

	_numFlObject = 50;

	if (_shadowPaletteSize)
		_shadowPalette = (byte *)reallocateArray(_shadowPalette, _shadowPaletteSize, 1);
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::readMAXS(int blockSize) {
	_fileHandle->read(_engineVersionString, 50);
	_fileHandle->read(_dataFileVersionString, 50);
	_numVariables = _fileHandle->readUint32LE();     // 1500
	_numBitVariables = _fileHandle->readUint32LE();  // 2048
	_fileHandle->readUint32LE();                     // 40
	_numScripts = _fileHandle->readUint32LE();       // 458
	_numSounds = _fileHandle->readUint32LE();        // 789
	_numCharsets = _fileHandle->readUint32LE();      // 1
	_numCostumes = _fileHandle->readUint32LE();      // 446
	_numRooms = _fileHandle->readUint32LE();         // 95
	_fileHandle->readUint32LE();                     // 80
	_numGlobalObjects = _fileHandle->readUint32LE(); // 1401
	_fileHandle->readUint32LE();                     // 60
	_numLocalObjects = _fileHandle->readUint32LE();  // 200
	_numNewNames = _fileHandle->readUint32LE();      // 100
	_numFlObject = _fileHandle->readUint32LE();      // 128
	_numInventory = _fileHandle->readUint32LE();     // 80
	_numArray = _fileHandle->readUint32LE();         // 200
	_numVerbs = _fileHandle->readUint32LE();         // 50

	_objectRoomTable = (byte *)reallocateArray(_objectRoomTable, _numGlobalObjects, 1);
	_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)reallocateArray(_shadowPalette, _shadowPaletteSize, 1);
}

void ScummEngine_v7::readMAXS(int blockSize) {
	_fileHandle->read(_engineVersionString, 50);
	_fileHandle->read(_dataFileVersionString, 50);
	_numVariables = _fileHandle->readUint16LE();
	_numBitVariables = _fileHandle->readUint16LE();
	_fileHandle->readUint16LE();
	_numGlobalObjects = _fileHandle->readUint16LE();
	_numLocalObjects = _fileHandle->readUint16LE();
	_numNewNames = _fileHandle->readUint16LE();
	_numVerbs = _fileHandle->readUint16LE();
	_numFlObject = _fileHandle->readUint16LE();
	_numInventory = _fileHandle->readUint16LE();
	_numArray = _fileHandle->readUint16LE();
	_numRooms = _fileHandle->readUint16LE();
	_numScripts = _fileHandle->readUint16LE();
	_numSounds = _fileHandle->readUint16LE();
	_numCharsets = _fileHandle->readUint16LE();
	_numCostumes = _fileHandle->readUint16LE();

	_objectRoomTable = (byte *)reallocateArray(_objectRoomTable, _numGlobalObjects, 1);

	if ((_game.id == GID_FT) && (_game.features & GF_DEMO) &&
		(_game.platform == Common::kPlatformDOS))
		_numGlobalScripts = 300;
	else
		_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)reallocateArray(_shadowPalette, _shadowPaletteSize, 1);
}
#endif

void ScummEngine_v6::readMAXS(int blockSize) {
	if (blockSize == 38) {
		_numVariables = _fileHandle->readUint16LE();
		_fileHandle->readUint16LE();
		_numBitVariables = _fileHandle->readUint16LE();
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
		_numNewNames = 50;

		_objectRoomTable = nullptr;
		_numGlobalScripts = 200;

		if (_game.heversion >= 70) {
			_objectRoomTable = (byte *)reallocateArray(_objectRoomTable, _numGlobalObjects, 1);
		}

		if (_game.heversion <= 70) {
			_shadowPaletteSize = 256;
			_shadowPalette = (byte *)reallocateArray(_shadowPalette, _shadowPaletteSize, 1);
		}
	} else
		error("readMAXS(%d) failed to read MAXS data", blockSize);
}

void ScummEngine::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectOwnerTable, num);
	for (i = 0; i < num; i++) {
		_objectStateTable[i] = _objectOwnerTable[i] >> OF_STATE_SHL;
		_objectOwnerTable[i] &= OF_OWNER_MASK;
	}

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::readGlobalObjects() {
	int i;
	int num = _fileHandle->readUint32LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_objectIDMap = new ObjectNameId[num];
	_objectIDMapSize = num;
	for (i = 0; i < num; i++) {
		// Add to object name-to-id map
		_fileHandle->read(_objectIDMap[i].name, 40);
		_objectIDMap[i].id = i;

		_objectStateTable[i] = _fileHandle->readByte();
		_objectRoomTable[i] = _fileHandle->readByte();
		_classData[i] = _fileHandle->readUint32LE();
	}
	memset(_objectOwnerTable, 0xFF, num);

	// Finally, sort the object name->ID map, so we can later use
	// bsearch on it. For this we (ab)use strcmp, which works fine
	// since the table entries start with a string.
	qsort(_objectIDMap, _objectIDMapSize, sizeof(ObjectNameId),
			(int (*)(const void*, const void*))strcmp);
}

void ScummEngine_v7::readGlobalObjects() {
	int num = _fileHandle->readUint16LE();
	assert(num == _numGlobalObjects);
	assert(_objectStateTable);
	assert(_objectOwnerTable);

	_fileHandle->read(_objectStateTable, num);
	_fileHandle->read(_objectRoomTable, num);
	memset(_objectOwnerTable, 0xFF, num);

	_fileHandle->read(_classData, num * sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	// Correct the endianess if necessary
	for (int i = 0; i != num; i++)
		_classData[i] = FROM_LE_32(_classData[i]);
#endif
}
#endif

void ScummEngine::allocateArrays() {
	// Note: Buffers are now allocated in scummMain to allow for
	//     early GUI init.

	_objectOwnerTable = (byte *)reallocateArray(_objectOwnerTable, _numGlobalObjects, 1);
	_objectStateTable = (byte *)reallocateArray(_objectStateTable, _numGlobalObjects, 1);
	_classData = (uint32 *)reallocateArray(_classData, _numGlobalObjects, sizeof(uint32));
	_newNames = (uint16 *)reallocateArray(_newNames, _numNewNames, sizeof(uint16));

	_inventory = (uint16 *)reallocateArray(_inventory, _numInventory, sizeof(uint16));
	_verbs = (VerbSlot *)reallocateArray(_verbs, _numVerbs, sizeof(VerbSlot));
	_objs = (ObjectData *)reallocateArray(_objs, _numLocalObjects, sizeof(ObjectData));
	_roomVars = (int32 *)reallocateArray(_roomVars, _numRoomVariables, sizeof(int32));
	_scummVars = (int32 *)reallocateArray(_scummVars, _numVariables, sizeof(int32));
	_bitVars = (byte *)reallocateArray(_bitVars, _numBitVariables >> 3, 1);
	if (_game.heversion >= 60) {
		_arraySlot = (byte *)reallocateArray(_arraySlot, _numArray, 1);
	}

	_res->allocResTypeData(rtCostume, (_game.features & GF_NEW_COSTUMES) ? MKTAG('A','K','O','S') : MKTAG('C','O','S','T'),
								_numCostumes, kStaticResTypeMode);
	_res->allocResTypeData(rtRoom, MKTAG('R','O','O','M'), _numRooms, kStaticResTypeMode);
	_res->allocResTypeData(rtRoomImage, MKTAG('R','M','I','M'), _numRooms, kStaticResTypeMode);
	_res->allocResTypeData(rtRoomScripts, MKTAG('R','M','S','C'), _numRooms, kStaticResTypeMode);
	_res->allocResTypeData(rtSound, MKTAG('S','O','U','N'), _numSounds, kSoundResTypeMode);
	_res->allocResTypeData(rtScript, MKTAG('S','C','R','P'), _numScripts, kStaticResTypeMode);
	_res->allocResTypeData(rtCharset, MKTAG('C','H','A','R'), _numCharsets, kStaticResTypeMode);
	_res->allocResTypeData(rtObjectName, 0, _numNewNames, kDynamicResTypeMode);
	_res->allocResTypeData(rtInventory, 0, _numInventory, kDynamicResTypeMode);
	_res->allocResTypeData(rtTemp, 0, 10, kDynamicResTypeMode);
	_res->allocResTypeData(rtScaleTable, 0, 5, kDynamicResTypeMode);
	_res->allocResTypeData(rtActorName, 0, _numActors, kDynamicResTypeMode);
	_res->allocResTypeData(rtVerb, 0, _numVerbs, kDynamicResTypeMode);
	_res->allocResTypeData(rtString, 0, _numArray, kDynamicResTypeMode);
	_res->allocResTypeData(rtFlObject, 0, _numFlObject, kDynamicResTypeMode);
	_res->allocResTypeData(rtMatrix, 0, 10, kDynamicResTypeMode);
	_res->allocResTypeData(rtImage, MKTAG('A','W','I','Z'), _numImages, kStaticResTypeMode);
	_res->allocResTypeData(rtTalkie, MKTAG('T','L','K','E'), _numTalkies, kStaticResTypeMode);
}

void ScummEngine_v70he::allocateArrays() {
	ScummEngine::allocateArrays();

	_res->allocResTypeData(rtSpoolBuffer, 0, 9, kStaticResTypeMode);
	_heV7RoomIntOffsets = (uint32 *)reallocateArray(_heV7RoomIntOffsets, _numRooms, sizeof(uint32));
}


void ScummEngine::dumpResource(const char *tag, int id, const byte *ptr, int length) {
	char buf[256];
	Common::DumpFile out;

	uint32 size;
	if (length >= 0)
		size = length;
	else if (_game.features & GF_OLD_BUNDLE)
		size = READ_LE_UINT16(ptr);
	else if (_game.features & GF_SMALL_HEADER)
		size = READ_LE_UINT32(ptr);
	else
		size = READ_BE_UINT32(ptr + 4);

	Common::sprintf_s(buf, "dumps/%s%d.dmp", tag, id);

	out.open(buf);
	if (out.isOpen() == false)
		return;
	out.write(ptr, size);
	out.close();
}

ResourceIterator::ResourceIterator(const byte *searchin, bool smallHeader)
	: _ptr(searchin), _smallHeader(smallHeader) {
	assert(searchin);
	if (_smallHeader) {
		_size = READ_LE_UINT32(searchin);
		_pos = 6;
		_ptr = searchin + 6;
	} else {
		_size = READ_BE_UINT32(searchin + 4);
		_pos = 8;
		_ptr = searchin + 8;
	}

}

const byte *ResourceIterator::findNext(uint32 tag) {
	uint32 size = 0;
	const byte *result = nullptr;

	if (_smallHeader) {
		uint16 smallTag = newTag2Old(tag);
		do {
			if (_pos >= _size)
				return nullptr;

			result = _ptr;
			size = READ_LE_UINT32(result);
			if ((int32)size <= 0)
				return nullptr;	// Avoid endless loop

			_pos += size;
			_ptr += size;
		} while (READ_LE_UINT16(result + 4) != smallTag);
	} else {
		do {
			if (_pos >= _size)
				return nullptr;

			result = _ptr;
			size = READ_BE_UINT32(result + 4);
			if ((int32)size <= 0)
				return nullptr;	// Avoid endless loop

			_pos += size;
			_ptr += size;
		} while (READ_BE_UINT32(result) != tag);
	}

	return result;
}

const byte *ScummEngine::findResource(uint32 tag, const byte *searchin) {
	Common::StackLock lock(_resourceAccessMutex);
	uint32 curpos, totalsize, size;

	debugC(DEBUG_RESOURCE, "findResource(%s, %p)", tag2str(tag), (const void *)searchin);

	if (!searchin) {
		if (_game.heversion >= 70) {
			searchin = _resourceLastSearchBuf;
			totalsize = _resourceLastSearchSize;
			curpos = 0;
		} else {
			assert(searchin);
			return nullptr;
		}
	} else {
		searchin += 4;
		_resourceLastSearchSize = totalsize = READ_BE_UINT32(searchin);
		curpos = 8;
		searchin += 4;
	}

	while (curpos < totalsize) {
		if (READ_BE_UINT32(searchin) == tag) {
			_resourceLastSearchBuf = searchin;
			return searchin;
		}

		size = READ_BE_UINT32(searchin + 4);
		if ((int32)size <= 0) {
			error("(%s) Not found in %d... illegal block len %d", tag2str(tag), 0, size);
			return nullptr;
		}

		curpos += size;
		searchin += size;
	}

	return nullptr;
}

const byte *findResourceSmall(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;
	uint16 smallTag;

	smallTag = newTag2Old(tag);
	if (smallTag == 0)
		return nullptr;

	assert(searchin);

	totalsize = READ_LE_UINT32(searchin);
	searchin += 6;
	curpos = 6;

	while (curpos < totalsize) {
		size = READ_LE_UINT32(searchin);

		if (READ_LE_UINT16(searchin + 4) == smallTag)
			return searchin;

		if ((int32)size <= 0) {
			error("(%s) Not found in %d... illegal block len %d", tag2str(tag), 0, size);
			return nullptr;
		}

		curpos += size;
		searchin += size;
	}

	return nullptr;
}

uint16 newTag2Old(uint32 newTag) {
	switch (newTag) {
	case (MKTAG('R','M','H','D')):
		return (0x4448);	// HD
	case (MKTAG('I','M','0','0')):
		return (0x4D42);	// BM
	case (MKTAG('E','X','C','D')):
		return (0x5845);	// EX
	case (MKTAG('E','N','C','D')):
		return (0x4E45);	// EN
	case (MKTAG('S','C','A','L')):
		return (0x4153);	// SA
	case (MKTAG('L','S','C','R')):
		return (0x534C);	// LS
	case (MKTAG('O','B','C','D')):
		return (0x434F);	// OC
	case (MKTAG('O','B','I','M')):
		return (0x494F);	// OI
	case (MKTAG('S','M','A','P')):
		return (0x4D42);	// BM
	case (MKTAG('C','L','U','T')):
		return (0x4150);	// PA
	case (MKTAG('B','O','X','D')):
		return (0x5842);	// BX
	case (MKTAG('C','Y','C','L')):
		return (0x4343);	// CC
	case (MKTAG('E','P','A','L')):
		return (0x5053);	// SP
	case (MKTAG('T','I','L','E')):
		return (0x4C54);	// TL
	case (MKTAG('Z','P','0','0')):
		return (0x505A);	// ZP
	default:
		return (0);
	}
}

const char *nameOfResType(ResType type) {
	static char buf[100];

	switch (type) {
	case rtRoom:
		return "Room";
	case rtScript:
		return "Script";
	case rtCostume:
		return "Costume";
	case rtSound:
		return "Sound";
	case rtInventory:
		return "Inventory";
	case rtCharset:
		return "Charset";
	case rtString:
		return "String";
	case rtVerb:
		return "Verb";
	case rtActorName:
		return "ActorName";
	case rtBuffer:
		return "Buffer";
	case rtScaleTable:
		return "ScaleTable";
	case rtTemp:
		return "Temp";
	case rtFlObject:
		return "FlObject";
	case rtMatrix:
		return "Matrix";
	case rtBox:
		return "Box";
	case rtObjectName:
		return "ObjectName";
	case rtRoomScripts:
		return "RoomScripts";
	case rtRoomImage:
		return "RoomImage";
	case rtImage:
		return "Image";
	case rtTalkie:
		return "Talkie";
	case rtSpoolBuffer:
		return "SpoolBuffer";
	default:
		Common::sprintf_s(buf, "rt%d", type);
		return buf;
	}
}

void ScummEngine::applyWorkaroundIfNeeded(ResType type, int idx) {
	// The resource isn't always loaded into memory, in which case no
	// workaround is needed. This happens when loading some HE savegames
	// where sound resource 1 isn't loaded. Possibly other cases as well.
	if (!_res->isResourceLoaded(type, idx))
		return;

	int size = getResourceSize(type, idx);

	// WORKAROUND: FM-TOWNS Zak used the extra 40 pixels at the bottom to increase the inventory to 10 items
	// if we trim to 200 pixels, we can show only 6 items
	// therefore we patch the inventory script (20)
	// replacing the 5 occurrences of 10 as limit to 6
	if (_game.platform == Common::kPlatformFMTowns && _game.id == GID_ZAK && ConfMan.getBool("trim_fmtowns_to_200_pixels")) {
		if (type == rtScript && idx == 20) {
			byte *ptr = getResourceAddress(rtScript, idx);
			for (int cnt = 5; cnt; ptr++) {
				if (*ptr == 10) {
					*ptr = 6;
					cnt--;
				}
			}
		}
	}

	// WORKAROUND: The Mac version of Monkey Island 2 that was distributed
	// on CD as the LucasArts Adventure Game Pack II is missing the part of
	// the boot script that shows the copy protection and difficulty
	// selection screen. Presumably it didn't include the code wheel. In
	// fact, none of the games on this CD have any copy protection.
	//
	// The games on the first Game Pack CD does have copy protection, but
	// since I only own the discs I can neither confirm nor deny if the
	// necessary documentation was included.
	//
	// However, this means that there is no way to pick the difficulty
	// level. Since ScummVM bypasses the copy protection check, there is
	// no harm in showing the screen by simply re-inserting the missing
	// part of the script.

	else if (_game.id == GID_MONKEY2 && _game.platform == Common::kPlatformMacintosh && type == rtScript && idx == 1 && size == 6718) {
		byte *unpatchedScript = getResourceAddress(type, idx);

		const byte patch[] = {
0x48, 0x00, 0x40, 0x00, 0x00, 0x13, 0x00, // [0926] if (Local[0] == 0) {
0x33, 0x03, 0x00, 0x00, 0xc8, 0x00,       // [092D]   SetScreen(0,200);
0x0a, 0x82, 0xff,                         // [0933]   startScript(130,[]);
0x80,                                     // [0936]   breakHere();
0x68, 0x00, 0x00, 0x82,                   // [0937]   VAR_RESULT = isScriptRunning(130);
0x28, 0x00, 0x00, 0xf6, 0xff,             // [093B]   unless (!VAR_RESULT) goto 0936;
                                          // [0940] }
0x48, 0x00, 0x40, 0x3f, 0xe1, 0x1d, 0x00, // [0940] if (Local[0] == -7873) [
0x1a, 0x32, 0x00, 0x3f, 0x01,             // [0947]   VAR_MAINMENU_KEY = 319;
0x33, 0x03, 0x00, 0x00, 0xc8, 0x00,       // [094C]   SetScreen(0,200);
0x0a, 0x82, 0xff,                         // [0952]   startScript(130,[]);
0x80,                                     // [0955]   breakHere();
0x68, 0x00, 0x00, 0x82,                   // [0956]   VAR_RESULT = isScriptRunning(130);
0x28, 0x00, 0x00, 0xf6, 0xff,             // [095A]   unless (!VAR_RESULT) goto 0955;
0x1a, 0x00, 0x40, 0x00, 0x00              // [095F]   Local[0] = 0;
                                          // [0964] }
		};

		byte *patchedScript = new byte[6780];

		memcpy(patchedScript, unpatchedScript, 2350);
		memcpy(patchedScript + 2350, patch, sizeof(patch));
		memcpy(patchedScript + 2350 + sizeof(patch), unpatchedScript + 2350, 6718 - 2350);

		WRITE_BE_UINT32(patchedScript + 4, 6780);

		// Just to be completely safe, check that the patched script now
		// matches the boot script from the other known Mac version.
		// Only if it does can we replace the unpatched script.

		if (verifyMI2MacBootScript(patchedScript, 6780)) {
			byte *newResource = _res->createResource(type, idx, 6780);
			memcpy(newResource, patchedScript, 6780);
		} else
			warning("Could not patch MI2 Mac boot script");

		delete[] patchedScript;
	} else

	// WORKAROUND: For some reason, the CD version of Monkey Island 1
	// removes some of the text when giving the wimpy idol to the cannibals.
	// It looks like a mistake, because one of the text that is printed is
	// immediately overwritten. This probably affects all CD versions, so we
	// just have to add further patches as they are reported.

	if (_game.id == GID_MONKEY && type == rtRoom && idx == 25 && enhancementEnabled(kEnhRestoredContent)) {
		tryPatchMI1CannibalScript(getResourceAddress(type, idx), size);
	} else if (_game.id == GID_MANIAC && _game.version == 2 && _game.platform == Common::kPlatformDOS &&
			   type == rtScript && idx == 44 && size == 199 && enhancementEnabled(kEnhMinorBugFixes)) {
		// WORKAROUND: There is a cracked version of Maniac Mansion v2 that
		// attempts to remove the security door copy protection. With it, any
		// code is accepted as long as you get the last digit wrong.
		// Unfortunately, it changes a script that is used by all keypads in the
		// game, which means some puzzles are completely nerfed.
		//
		// Even worse, this is the version that GOG and Steam are selling. No,
		// seriously! I've reported this as a bug, but it remains unclear
		// whether or not they will fix it.
		//
		// Please note, this fix was posthumously made optional using the kEnhMinorBugFixes
		// enhancement class, since this is still (somehow...) an official release, and this
		// represents an external bug fix on the data files. As of why this was made optional:
		// there was an interest within the speedrunning community (see #14815).
		//
		// Quoting from the ticket:
		// "The speedruns for the PC version of Maniac Mansion are separated into two categories:
		// one that makes use of the broken lab door and one that does not. All of the runs, that
		// make use of it are from within the first two years after the digital release of the game.
		// I'm pretty sure, no one understood what was going on, they just recognized the door being
		// broken from runs of the NES game (this is me guessing). You could say the category is
		// "grandfathered in". Still, it is the most popular and shortest category for Maniac Mansion."

		byte *data = getResourceAddress(type, idx);

		if (data[184] == 0) {
			Common::MemoryReadStream stream(data, size);
			Common::String md5 = Common::computeStreamMD5AsString(stream);

			if (md5 == "11adc9b47497b26ac2b9627e0982b3fe") {
				warning("Removing bad copy protection crack from keypad script");
				data[184] = 1;
			}
		}
	}
}

bool ScummEngine::verifyMI2MacBootScript() {
	return verifyMI2MacBootScript(getResourceAddress(rtScript, 1), getResourceSize(rtScript, 1));
}

bool ScummEngine::verifyMI2MacBootScript(byte *buf, int size) {
	if (size == 6780) {
		Common::MemoryReadStream stream(buf, size);
		Common::String md5 = Common::computeStreamMD5AsString(stream);

		if (md5 != "92b1cb7902b57d02b8e7434903d8508b") {
			warning("Unexpected MI2 Mac boot script checksum: %s", md5.c_str());
			return false;
		}
	} else {
		warning("Unexpected MI2 Mac boot script length: %d", size);
		return false;
	}
	return true;
}

bool ScummEngine::tryPatchMI1CannibalScript(byte *buf, int size) {
	assert(_game.id == GID_MONKEY);

	// The room resource is a collection of resources. We need to know the
	// offset to the initial LSCR tag of the room-25-205 script, and its
	// length up to (but not including) the LSCR tag of the next script.
	// Furthermore we need to know the offset and length of the part of
	// the script that we are going to replace. As an illustration, this
	// is what that part of script looks like in the English CD version:
	//
	// [009C] (AE) WaitForMessage();
	// [009E] (14) print(3,[Text("Oooh, that's nice.")]);
	// [00B4] (14) print(3,[Text("And it says, `Made by Lemonhead`^" +
	//             wait() + "^just like one of mine!" + wait() +
	//             "We should take this to the Great Monkey.")]);
	// [011C] (AE) WaitForMessage();
	//
	// What we want to do is make it behave like the script from the VGA
	// floppy version:
	//
	// [009E] (AE) WaitForMessage();
	// [00A0] (14) print(3,[Text("Oooh, that's nice." + wait() +
	//             "Simple.  Just like one of mine." + wait() +
	//             "And little.  Like mine.")]);
	// [00F0] (AE) WaitForMessage();
	// [00F2] (14) print(3,[Text("And it says, `Made by Lemonhead`^" +
	//             wait() + "^just like one of mine!" + wait() +
	//             "We should take this to the Great Monkey.")]);
	// [015A] (AE) WaitForMessage();
	//
	// So we want to adjust the message, and insert a WaitForMessage().
	// Unfortunately there isn't enough space to do that, and rather than
	// modifying the length of the whole resource (which is easy to get
	// wrong), we insert a placeholder message that gets replaced by
	// decodeParseString().
	//
	// There should be enough space to do this even if we only change the
	// first message. Any leftover space in the message is padded with
	// spaces, since I can't find any NOP opcode.

	int expectedSize = -1;
	int scriptOffset = -1;
	int scriptLength = -1;
	Common::String expectedMd5;
	int patchOffset = -1;
	int patchLength = -1;
	byte lang[3];

	switch (_language) {
	case Common::EN_ANY:
		expectedSize = 82906;
		scriptOffset = 73883;
		scriptLength = 607;
		expectedMd5 = "98b1126a836ef5bfefff10b605b20555";
		patchOffset = 167;
		patchLength = 22;
		lang[0] = 'E';
		lang[1] = 'N';
		lang[2] = 'G';

		// The Macintosh resource is 4 bytes shorter, which affects
		// the script offset as well. Otherwise, both Mac versions
		// that I have are identical to the DOS CD version in this
		// particular case.

		if (_game.platform == Common::kPlatformMacintosh) {
			expectedSize -= 4;
			scriptOffset -= 4;
		} else if (_game.platform == Common::kPlatformFMTowns) {
			expectedSize = 82817;
			scriptOffset = 73794;
		} else if (_game.platform == Common::kPlatformSegaCD) {
			expectedSize = 61844;
			scriptOffset = 51703;
		}
		break;
	case Common::DE_DEU:
		expectedSize = 83554;
		scriptOffset = 74198;
		scriptLength = 632;
		expectedMd5 = "27d6d8eab4e0f66792e10769090ae047";
		patchOffset = 170;
		patchLength = 23;
		lang[0] = 'D';
		lang[1] = 'E';
		lang[2] = 'U';
		break;
	case Common::IT_ITA:
		expectedSize = 83211;
		scriptOffset = 73998;
		scriptLength = 602;
		expectedMd5 = "39eb6116d67f2318f31d6fa98df2e931";
		patchOffset = 161;
		patchLength = 20;
		lang[0] = 'I';
		lang[1] = 'T';
		lang[2] = 'A';
		break;
	case Common::ES_ESP:
		expectedSize = 82829;
		scriptOffset = 73905;
		scriptLength = 579;
		expectedMd5 = "0e282d86f80d4e062a9a145601e6fed3";
		patchOffset = 161;
		patchLength = 21;
		lang[0] = 'E';
		lang[1] = 'S';
		lang[2] = 'P';
		break;
	// For some reason, those lines were already missing from the official
	// French floppy EGA/VGA releases, and so there's no official content
	// to restore for this language.
	case Common::FR_FRA:
	default:
		return false;
	}

	// Note that the patch will not apply to the "Ultimate Talkie" edition
	// since that script has been patched to a different length.

	if (size == expectedSize) {
		// There isn't enough space in the script for the revised
		// texts, so these abbreviations will be expanded in
		// decodeParseString().
		const byte patchData[] = {
			0x14, 0x03, 0x0F,       // print(3,[Text("/LH.$$$/");
			0x2F, 0x4C, 0x48, 0x2E,
			0x24, 0x24, 0x24, 0x2F  // No terminating 0x00!
		};

		byte *scriptPtr = buf + scriptOffset;

		// Check that the data is a local script.
		if (READ_BE_UINT32(scriptPtr) != MKTAG('L','S','C','R'))
			return false;

		// Check that the first instruction to be patched is o5_print
		if (scriptPtr[patchOffset] != 0x14)
			return false;

		// Check that the MD5 sum matches a known patchable script.
		Common::MemoryReadStream stream(buf + scriptOffset, scriptLength);
		Common::String md5 = Common::computeStreamMD5AsString(stream);

		if (md5 != expectedMd5)
			return false;

		// Insert the script patch and tag it with the appropriate
		// language.

		memcpy(scriptPtr + patchOffset, patchData, sizeof(patchData));
		memcpy(scriptPtr + patchOffset + 7, lang, sizeof(lang));

		// Pad the rest of the replaced script part with spaces before
		// terminating the string. Finally, add WaitForMessage().

		memset(scriptPtr + patchOffset + sizeof(patchData), 32, patchLength - sizeof(patchData) - 3);
		scriptPtr[patchOffset + patchLength - 3] = 0;
		scriptPtr[patchOffset + patchLength - 2] = 0xAE;
		scriptPtr[patchOffset + patchLength - 1] = 0x02;
	}

	return true;
}


} // End of namespace Scumm
