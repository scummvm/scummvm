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
 * $URL$
 * $Id$
 *
 */

#include "common/str.h"

#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
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

	RS_MODIFIED = 0x10
};



extern const char *resTypeFromId(int id);

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
	const uint32 diskNumber = room ? _res->roomno[rtRoom][room] : 0;
	const uint32 room_offs = room ? _res->roomoffs[rtRoom][room] : 0;

	// FIXME: Since room_offs is const, clearly the following loop either
	// is never entered, or loops forever (if it wasn't for the return/error
	// statements in it, that is). -> This should be cleaned up!
	// Maybe we should re-enabled the looping properly, to deal with disc
	// changes in COMI ?
	while (room_offs != RES_INVALID_OFFSET) {

		if (room_offs != 0 && room != 0 && _game.heversion < 98) {
			_fileOffset = _res->roomoffs[rtRoom][room];
			return;
		}

		Common::String filename(generateFilename(room));

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
			_fileOffset = _res->roomoffs[rtRoom][room];

			if (_fileOffset != 8)
				return;

			error("Room %d not in %s", room, filename.c_str());
			return;
		}
		askForDisk(filename.c_str(), diskNumber);
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
		if (_res->roomoffs[rtRoom][i] != RES_INVALID_OFFSET)
			_res->roomoffs[rtRoom][i] = 0;
	}
}

/** Read room offsets */
void ScummEngine::readRoomsOffsets() {
	int num, room;

	debug(9, "readRoomOffsets()");

	if (_game.features & GF_SMALL_HEADER) {
		_fileHandle->seek(12, SEEK_SET);	// Directly searching for the room offset block would be more generic...
	} else {
		_fileHandle->seek(16, SEEK_SET);
	}

	num = _fileHandle->readByte();
	while (num--) {
		room = _fileHandle->readByte();
		if (_res->roomoffs[rtRoom][room] != RES_INVALID_OFFSET) {
			_res->roomoffs[rtRoom][room] = _fileHandle->readUint32LE();
		} else {
			_fileHandle->readUint32LE();
		}
	}
}

bool ScummEngine::openFile(BaseScummFile &file, const Common::String &filename, bool resourceFile) {
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

bool ScummEngine::openResourceFile(const Common::String &filename, byte encByte) {
	debugC(DEBUG_GENERAL, "openResourceFile(%s)", filename.c_str());

	if (openFile(*_fileHandle, filename, true)) {
		_fileHandle->setEnc(encByte);
		return true;
	}
	return false;
}

void ScummEngine::askForDisk(const char *filename, int disknum) {
	char buf[128];

	if (_game.version == 8) {
#ifdef ENABLE_SCUMM_7_8
		char result;

		_imuseDigital->stopAllSounds();

#ifdef MACOSX
		sprintf(buf, "Cannot find file: '%s'\nPlease insert disc %d.\nPress OK to retry, Quit to exit", filename, disknum);
#else
		sprintf(buf, "Cannot find file: '%s'\nInsert disc %d into drive %s\nPress OK to retry, Quit to exit", filename, disknum, _gameDataDir.getPath().c_str());
#endif

		result = displayMessage("Quit", buf);
		if (!result) {
			error("Cannot find file: '%s'", filename);
		}
#endif
	} else {
		sprintf(buf, "Cannot find file: '%s'", filename);
		InfoDialog dialog(this, (char*)buf);
		runDialog(dialog);
		error("Cannot find file: '%s'", filename);
	}
}

void ScummEngine::readIndexFile() {
	uint32 blocktype, itemsize;
	int numblock = 0;

	debugC(DEBUG_GENERAL, "readIndexFile()");

	closeRoom();
	openRoom(0);

	if (_game.version <= 5) {
		// Figure out the sizes of various resources
		while (!_fileHandle->eos()) {
			blocktype = _fileHandle->readUint32BE();
			itemsize = _fileHandle->readUint32BE();
			if (_fileHandle->ioFailed())
				break;
			switch (blocktype) {
			case MKID_BE('DOBJ'):
				_numGlobalObjects = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;
			case MKID_BE('DROO'):
				_numRooms = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKID_BE('DSCR'):
				_numScripts = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKID_BE('DCOS'):
				_numCostumes = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;

			case MKID_BE('DSOU'):
				_numSounds = _fileHandle->readUint16LE();
				itemsize -= 2;
				break;
			}
			_fileHandle->seek(itemsize - 8, SEEK_CUR);
		}
		_fileHandle->clearIOFailed();
		_fileHandle->seek(0, SEEK_SET);
	}

	if (checkTryMedia(_fileHandle)) {
		displayMessage(NULL, "You're trying to run game encrypted by ActiveMark. This is not supported.");
		quitGame();

		return;
	}

	while (true) {
		blocktype = _fileHandle->readUint32BE();
		itemsize = _fileHandle->readUint32BE();

		if (_fileHandle->ioFailed())
			break;

		numblock++;
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
	case MKID_BE('ANAM'):		// Used by: The Dig, FT
		debug(9, "found ANAM block, reading audio names");
		num = _fileHandle->readUint16LE();
		ptr = (char*)malloc(num * 9);
		_fileHandle->read(ptr, num * 9);
		_imuseDigital->setAudioNames(num, ptr);
		break;

	case MKID_BE('DRSC'):		// Used by: COMI
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
	case MKID_BE('DIRI'):
		readResTypeList(rtRoomImage);
		break;

	case MKID_BE('DIRM'):
		readResTypeList(rtImage);
		break;

	case MKID_BE('DIRT'):
		readResTypeList(rtTalkie);
		break;

	case MKID_BE('DLFL'):
		i = _fileHandle->readUint16LE();
		_fileHandle->seek(-2, SEEK_CUR);
		_heV7RoomOffsets = (byte *)calloc(2 + (i * 4), 1);
		_fileHandle->read(_heV7RoomOffsets, (2 + (i * 4)) );
		break;

	case MKID_BE('DISK'):
		i = _fileHandle->readUint16LE();
		_heV7DiskOffsets = (byte *)calloc(i, 1);
		_fileHandle->read(_heV7DiskOffsets, i);
		break;

	case MKID_BE('SVER'):
		// Index version number
		_fileHandle->seek(itemsize - 8, SEEK_CUR);
		break;

	case MKID_BE('INIB'):
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
	case MKID_BE('DCHR'):
	case MKID_BE('DIRF'):
		readResTypeList(rtCharset);
		break;

	case MKID_BE('DOBJ'):
		debug(9, "found DOBJ block, reading object table");
		readGlobalObjects();
		break;

	case MKID_BE('RNAM'):
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

	case MKID_BE('DROO'):
	case MKID_BE('DIRR'):
		readResTypeList(rtRoom);
		break;

	case MKID_BE('DSCR'):
	case MKID_BE('DIRS'):
		readResTypeList(rtScript);
		break;

	case MKID_BE('DCOS'):
	case MKID_BE('DIRC'):
		readResTypeList(rtCostume);
		break;

	case MKID_BE('MAXS'):
		readMAXS(itemsize);
		allocateArrays();
		break;

	case MKID_BE('DIRN'):
	case MKID_BE('DSOU'):
		readResTypeList(rtSound);
		break;

	case MKID_BE('AARY'):
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

void ScummEngine::readResTypeList(int id) {
	int num;
	int i;

	debug(9, "readResTypeList(%s)", resTypeFromId(id));

	if (_game.version == 8)
		num = _fileHandle->readUint32LE();
	else
		num = _fileHandle->readUint16LE();

	if (num != _res->num[id]) {
		error("Invalid number of %ss (%d) in directory", resTypeFromId(id), num);
	}

	for (i = 0; i < num; i++) {
		_res->roomno[id][i] = _fileHandle->readByte();
	}
	for (i = 0; i < num; i++) {
		_res->roomoffs[id][i] = _fileHandle->readUint32LE();

		if (id == rtRoom && _game.heversion >= 70)
			_heV7RoomIntOffsets[i] = _res->roomoffs[id][i];
	}

	if (_game.heversion >= 70) {
		for (i = 0; i < num; i++) {
			_res->globsize[id][i] = _fileHandle->readUint32LE();
		}
	}
}

void ResourceManager::allocResTypeData(int id, uint32 tag, int num_, const char *name_, int mode_) {
	debug(9, "allocResTypeData(%s/%s,%s,%d,%d)", resTypeFromId(id), name_, tag2str(TO_BE_32(tag)), num_, mode_);
	assert(id >= 0 && id < (int)(ARRAYSIZE(this->mode)));

	if (num_ >= 8000)
		error("Too many %ss (%d) in directory", name_, num_);

	mode[id] = mode_;
	num[id] = num_;
	tags[id] = tag;
	name[id] = name_;
	address[id] = (byte **)calloc(num_, sizeof(void *));
	flags[id] = (byte *)calloc(num_, sizeof(byte));
	status[id] = (byte *)calloc(num_, sizeof(byte));

	if (mode_) {
		roomno[id] = (byte *)calloc(num_, sizeof(byte));
		roomoffs[id] = (uint32 *)calloc(num_, sizeof(uint32));
	}

	if (_vm->_game.heversion >= 70) {
		globsize[id] = (uint32 *)calloc(num_, sizeof(uint32));
	}

}

void ScummEngine::loadCharset(int no) {
	int i;
	byte *ptr;

	debugC(DEBUG_GENERAL, "loadCharset(%d)", no);

	/* FIXME - hack around crash in Indy4 (occurs if you try to load after dieing) */
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

void ScummEngine::ensureResourceLoaded(int type, int i) {
	void *addr = NULL;

	debugC(DEBUG_RESOURCE, "ensureResourceLoaded(%s,%d)", resTypeFromId(type), i);

	if ((type == rtRoom) && i > 0x7F && _game.version < 7 && _game.heversion <= 71) {
		i = _resourceMapper[i & 0x7F];
	}

	// FIXME: This check used to be "i==0". However, that causes
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
	if (type != rtCharset && i == 0)
		return;

	if (i <= _res->num[type])
		addr = _res->address[type][i];

	if (addr)
		return;

	loadResource(type, i);

	if (_game.version == 5 && type == rtRoom && i == _roomResource)
		VAR(VAR_ROOM_FLAG) = 1;
}

int ScummEngine::loadResource(int type, int idx) {
	int roomNr;
	uint32 fileOffs;
	uint32 size, tag;

	debugC(DEBUG_RESOURCE, "loadResource(%s,%d)", resTypeFromId(type), idx);

	if (type == rtCharset && (_game.features & GF_SMALL_HEADER)) {
		loadCharset(idx);
		return 1;
	}

	roomNr = getResourceRoomNr(type, idx);

	if (idx >= _res->num[type])
		error("%s %d undefined %d %d", _res->name[type], idx, _res->num[type], roomNr);

	if (roomNr == 0)
		roomNr = _roomResource;

	if (type == rtRoom) {
		if (_game.version == 8)
			fileOffs = 8;
		else if (_game.heversion >= 70)
			fileOffs = _heV7RoomIntOffsets[idx];
		else
			fileOffs = 0;
	} else {
		fileOffs = _res->roomoffs[type][idx];
		if (fileOffs == RES_INVALID_OFFSET)
			return 0;
	}

	openRoom(roomNr);

	_fileHandle->seek(fileOffs + _fileOffset, SEEK_SET);

	if (_game.features & GF_OLD_BUNDLE) {
		if ((_game.version == 3) && !(_game.platform == Common::kPlatformAmiga) && (type == rtSound)) {
			return readSoundResourceSmallHeader(idx);
		} else {
			size = _fileHandle->readUint16LE();
			_fileHandle->seek(-2, SEEK_CUR);
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

		tag = _fileHandle->readUint32BE();

		if (tag != _res->tags[type] && _game.heversion < 70) {
			error("%s %d not in room %d at %d+%d in file %s",
					_res->name[type], idx, roomNr,
					_fileOffset, fileOffs, _fileHandle->getName());
		}

		size = _fileHandle->readUint32BE();
		_fileHandle->seek(-8, SEEK_CUR);
	}
	_fileHandle->read(_res->createResource(type, idx, size), size);

	// dump the resource if requested
	if (_dumpScripts && type == rtScript) {
		dumpResource("script-", idx, getResourceAddress(rtScript, idx));
	}

	if (!_fileHandle->ioFailed()) {
		return 1;
	}

	_res->nukeResource(type, idx);

	error("Cannot read resource");
}

int ScummEngine::getResourceRoomNr(int type, int idx) {
	if (type == rtRoom && _game.heversion < 70)
		return idx;
	return _res->roomno[type][idx];
}

int ScummEngine::getResourceSize(int type, int idx) {
	byte *ptr = getResourceAddress(type, idx);
	assert(ptr);
	MemBlkHeader *hdr = (MemBlkHeader *)(ptr - sizeof(MemBlkHeader));

	return hdr->size;
}

byte *ScummEngine::getResourceAddress(int type, int idx) {
	byte *ptr;

	if (_game.heversion >= 80 && type == rtString)
		idx &= ~0x33539000;

	if (!_res->validateResource("getResourceAddress", type, idx))
		return NULL;

	if (!_res->address[type]) {
		debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d), _res->address[type] == NULL", resTypeFromId(type), idx);
		return NULL;
	}

	if (_res->mode[type] && !_res->address[type][idx]) {
		ensureResourceLoaded(type, idx);
	}

	if (!(ptr = (byte *)_res->address[type][idx])) {
		debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d) == NULL", resTypeFromId(type), idx);
		return NULL;
	}

	_res->setResourceCounter(type, idx, 1);

	debugC(DEBUG_RESOURCE, "getResourceAddress(%s,%d) == %p", resTypeFromId(type), idx, ptr + sizeof(MemBlkHeader));
	return ptr + sizeof(MemBlkHeader);
}

byte *ScummEngine::getStringAddress(int i) {
	byte *addr = getResourceAddress(rtString, i);
	return addr;
}

byte *ScummEngine_v6::getStringAddress(int i) {
	byte *addr = getResourceAddress(rtString, i);
	if (addr == NULL)
		return NULL;
	// Skip over the ArrayHeader
	return addr + 6;
}

byte *ScummEngine::getStringAddressVar(int i) {
	return getStringAddress(_scummVars[i]);
}

void ResourceManager::increaseExpireCounter() {
	if (!(++_expireCounter)) {
		increaseResourceCounter();
	}
}

void ResourceManager::increaseResourceCounter() {
	int i, j;
	byte counter;

	for (i = rtFirst; i <= rtLast; i++) {
		for (j = num[i]; --j >= 0;) {
			counter = flags[i][j] & RF_USAGE;
			if (counter && counter < RF_USAGE_MAX) {
				setResourceCounter(i, j, counter + 1);
			}
		}
	}
}

void ResourceManager::setResourceCounter(int type, int idx, byte flag) {
	flags[type][idx] &= ~RF_USAGE;
	flags[type][idx] |= flag;
}

/* 2 bytes safety area to make "precaching" of bytes in the gdi drawer easier */
#define SAFETY_AREA 2

byte *ResourceManager::createResource(int type, int idx, uint32 size) {
	debugC(DEBUG_RESOURCE, "_res->createResource(%s,%d,%d)", resTypeFromId(type), idx, size);

	if (!validateResource("allocating", type, idx))
		return NULL;

	if (_vm->_game.version <= 2) {
		// Nuking and reloading a resource can be harmful in some
		// cases. For instance, Zak tries to reload the intro music
		// while it's playing. See bug #1253171.

		if (address[type][idx] && (type == rtSound || type == rtScript || type == rtCostume))
			return address[type][idx] + sizeof(MemBlkHeader);
	}

	nukeResource(type, idx);

	expireResources(size);

	void *ptr = calloc(size + sizeof(MemBlkHeader) + SAFETY_AREA, 1);
	if (ptr == NULL) {
		error("createResource(%s,%d): Out of memory while allocating %d", resTypeFromId(type), idx, size);
	}

	_allocatedSize += size;

	address[type][idx] = (byte *)ptr;
	((MemBlkHeader *)ptr)->size = size;
	setResourceCounter(type, idx, 1);
	return (byte *)ptr + sizeof(MemBlkHeader);	/* skip header */
}

ResourceManager::ResourceManager(ScummEngine *vm) {
	memset(this, 0, sizeof(ResourceManager));
	_vm = vm;
//	_allocatedSize = 0;
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

bool ResourceManager::validateResource(const char *str, int type, int idx) const {
	if (type < rtFirst || type > rtLast || (uint) idx >= (uint)num[type]) {
		error("%s Illegal Glob type %s (%d) num %d", str, resTypeFromId(type), type, idx);
		return false;
	}
	return true;
}

void ResourceManager::nukeResource(int type, int idx) {
	byte *ptr;

	if (!address[type])
		return;

	assert(idx >= 0 && idx < num[type]);

	ptr = address[type][idx];
	if (ptr != NULL) {
		debugC(DEBUG_RESOURCE, "nukeResource(%s,%d)", resTypeFromId(type), idx);
		address[type][idx] = 0;
		flags[type][idx] = 0;
		status[type][idx] &= ~RS_MODIFIED;
		_allocatedSize -= ((MemBlkHeader *)ptr)->size;
		free(ptr);
	}
}

const byte *ScummEngine::findResourceData(uint32 tag, const byte *ptr) {
	if (_game.features & GF_OLD_BUNDLE)
		error("findResourceData must not be used in GF_OLD_BUNDLE games");
	else if (_game.features & GF_SMALL_HEADER)
		ptr = findResourceSmall(tag, ptr);
	else
		ptr = findResource(tag, ptr);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

int ScummEngine::getResourceDataSize(const byte *ptr) const {
	if (ptr == NULL)
		return 0;

	if (_game.features & GF_OLD_BUNDLE)
		return READ_LE_UINT16(ptr) - _resourceHeaderSize;
	else if (_game.features & GF_SMALL_HEADER)
		return READ_LE_UINT32(ptr) - _resourceHeaderSize;
	else
		return READ_BE_UINT32(ptr - 4) - _resourceHeaderSize;
}

void ResourceManager::lock(int type, int i) {
	if (!validateResource("Locking", type, i))
		return;
	flags[type][i] |= RF_LOCK;
}

void ResourceManager::unlock(int type, int i) {
	if (!validateResource("Unlocking", type, i))
		return;
	flags[type][i] &= ~RF_LOCK;
}

bool ResourceManager::isLocked(int type, int i) const {
	if (!validateResource("isLocked", type, i))
		return false;
	return (flags[type][i] & RF_LOCK) != 0;
}

bool ScummEngine::isResourceInUse(int type, int i) const {
	if (!_res->validateResource("isResourceInUse", type, i))
		return false;
	switch (type) {
	case rtRoom:
		return _roomResource == (byte)i;
	case rtRoomImage:
		return _roomResource == (byte)i;
	case rtRoomScripts:
		return _roomResource == (byte)i;
	case rtScript:
		return isScriptInUse(i);
	case rtCostume:
		return isCostumeInUse(i);
	case rtSound:
		// Sound resource 1 is used for queued speech
		if (_game.heversion >= 60 && i == 1)
			return true;
		else
			return _sound->isSoundInUse(i);
	case rtCharset:
		return _charset->getCurID() == i;
	case rtImage:
		return _res->isModified(type, i) != 0;
	case rtSpoolBuffer:
		return _sound->isSoundRunning(10000 + i) != 0;
	default:
		return false;
	}
}

void ResourceManager::setModified(int type, int i) {
	if (!validateResource("Modified", type, i))
		return;
	status[type][i] |= RS_MODIFIED;
}

bool ResourceManager::isModified(int type, int i) const {
	if (!validateResource("isModified", type, i))
		return false;
	return (status[type][i] & RS_MODIFIED) != 0;
}

void ResourceManager::expireResources(uint32 size) {
	int i, j;
	byte flag;
	byte best_counter;
	int best_type, best_res = 0;
	uint32 oldAllocatedSize;

	if (_expireCounter != 0xFF) {
		_expireCounter = 0xFF;
		increaseResourceCounter();
	}

	if (size + _allocatedSize < _maxHeapThreshold)
		return;

	oldAllocatedSize = _allocatedSize;

	do {
		best_type = 0;
		best_counter = 2;

		for (i = rtFirst; i <= rtLast; i++)
			if (mode[i]) {
				for (j = num[i]; --j >= 0;) {
					flag = flags[i][j];
					if (!(flag & RF_LOCK) && flag >= best_counter && address[i][j] && !_vm->isResourceInUse(i, j)) {
						best_counter = flag;
						best_type = i;
						best_res = j;
					}
				}
			}

		if (!best_type)
			break;
		nukeResource(best_type, best_res);
	} while (size + _allocatedSize > _minHeapThreshold);

	increaseResourceCounter();

	debugC(DEBUG_RESOURCE, "Expired resources, mem %d -> %d", oldAllocatedSize, _allocatedSize);
}

void ResourceManager::freeResources() {
	int i, j;
	for (i = rtFirst; i <= rtLast; i++) {
		for (j = num[i]; --j >= 0;) {
			if (isResourceLoaded(i, j))
				nukeResource(i, j);
		}
		free(address[i]);
		free(flags[i]);
		free(status[i]);
		free(roomno[i]);
		free(roomoffs[i]);

		free(globsize[i]);
	}
}

void ScummEngine::loadPtrToResource(int type, int resindex, const byte *source) {
	byte *alloced;
	int i, len;

	_res->nukeResource(type, resindex);

	len = resStrLen(source) + 1;

	if (len <= 0)
		return;

	alloced = _res->createResource(type, resindex, len);

	if (!source) {
		alloced[0] = fetchScriptByte();
		for (i = 1; i < len; i++)
			alloced[i] = *_scriptPointer++;
	} else {
		for (i = 0; i < len; i++)
			alloced[i] = source[i];
	}
}

bool ResourceManager::isResourceLoaded(int type, int idx) const {
	if (!validateResource("isResourceLoaded", type, idx))
		return false;
	return address[type][idx] != NULL;
}

void ResourceManager::resourceStats() {
	int i, j;
	uint32 lockedSize = 0, lockedNum = 0;
	byte flag;

	for (i = rtFirst; i <= rtLast; i++)
		for (j = num[i]; --j >= 0;) {
			flag = flags[i][j];
			if (flag & RF_LOCK && address[i][j]) {
				lockedSize += ((MemBlkHeader *)address[i][j])->size;
				lockedNum++;
			}
		}

	debug(1, "Total allocated size=%d, locked=%d(%d)", _allocatedSize, lockedSize, lockedNum);
}

void ScummEngine_v5::readMAXS(int blockSize) {
	debug(9, "ScummEngine_v5 readMAXS: MAXS has blocksize %d", blockSize);

	_numVariables = _fileHandle->readUint16LE();      // 800
	_fileHandle->readUint16LE();                      // 16
	_numBitVariables = _fileHandle->readUint16LE();   // 2048
	_numLocalObjects = _fileHandle->readUint16LE();   // 200
	_numArray = 50;
	_numVerbs = 100;
	// Used to be 50, which wasn't enough for MI2 and FOA. See bugs
	// #933610, #936323 and #941275.
	_numNewNames = 150;
	_objectRoomTable = NULL;

	_fileHandle->readUint16LE();                      // 50
	_numCharsets = _fileHandle->readUint16LE();       // 9
	_fileHandle->readUint16LE();                      // 100
	_fileHandle->readUint16LE();                      // 50
	_numInventory = _fileHandle->readUint16LE();      // 80
	_numGlobalScripts = 200;

	_shadowPaletteSize = 256;

	_numFlObject = 50;

	if (_shadowPaletteSize)
		_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v8::readMAXS(int blockSize) {
	debug(9, "ScummEngine_v8 readMAXS: MAXS has blocksize %d", blockSize);

	_fileHandle->seek(50, SEEK_CUR);                 // Skip over SCUMM engine version
	_fileHandle->seek(50, SEEK_CUR);                 // Skip over data file version
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

	_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
	_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
}

void ScummEngine_v7::readMAXS(int blockSize) {
	debug(9, "ScummEngine_v7 readMAXS: MAXS has blocksize %d", blockSize);

	_fileHandle->seek(50, SEEK_CUR);                 // Skip over SCUMM engine version
	_fileHandle->seek(50, SEEK_CUR);                 // Skip over data file version
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

	_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);

	if ((_game.id == GID_FT) && (_game.features & GF_DEMO) &&
		(_game.platform == Common::kPlatformPC))
		_numGlobalScripts = 300;
	else
		_numGlobalScripts = 2000;

	_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
}
#endif

void ScummEngine_v6::readMAXS(int blockSize) {
	if (blockSize == 38) {
		debug(0, "ScummEngine_v6 readMAXS: MAXS has blocksize %d", blockSize);

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

		_objectRoomTable = NULL;
		_numGlobalScripts = 200;

		if (_game.heversion >= 70) {
			_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		}

		if (_game.heversion <= 70) {
			_shadowPaletteSize = 256;
			_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);
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

	_objectOwnerTable = (byte *)calloc(_numGlobalObjects, 1);
	_objectStateTable = (byte *)calloc(_numGlobalObjects, 1);
	_classData = (uint32 *)calloc(_numGlobalObjects, sizeof(uint32));
	_newNames = (uint16 *)calloc(_numNewNames, sizeof(uint16));

	_inventory = (uint16 *)calloc(_numInventory, sizeof(uint16));
	_verbs = (VerbSlot *)calloc(_numVerbs, sizeof(VerbSlot));
	_objs = (ObjectData *)calloc(_numLocalObjects, sizeof(ObjectData));
	_roomVars = (int32 *)calloc(_numRoomVariables, sizeof(int32));
	_scummVars = (int32 *)calloc(_numVariables, sizeof(int32));
	_bitVars = (byte *)calloc(_numBitVariables >> 3, 1);
	if (_game.heversion >= 60) {
		_arraySlot = (byte *)calloc(_numArray, 1);
	}

	_res->allocResTypeData(rtCostume, (_game.features & GF_NEW_COSTUMES) ? MKID_BE('AKOS') : MKID_BE('COST'),
								_numCostumes, "costume", 1);
	_res->allocResTypeData(rtRoom, MKID_BE('ROOM'), _numRooms, "room", 1);
	_res->allocResTypeData(rtRoomImage, MKID_BE('RMIM'), _numRooms, "room image", 1);
	_res->allocResTypeData(rtRoomScripts, MKID_BE('RMSC'), _numRooms, "room script", 1);
	_res->allocResTypeData(rtSound, MKID_BE('SOUN'), _numSounds, "sound", 2);
	_res->allocResTypeData(rtScript, MKID_BE('SCRP'), _numScripts, "script", 1);
	_res->allocResTypeData(rtCharset, MKID_BE('CHAR'), _numCharsets, "charset", 1);
	_res->allocResTypeData(rtObjectName, 0, _numNewNames, "new name", 0);
	_res->allocResTypeData(rtInventory, 0, _numInventory, "inventory", 0);
	_res->allocResTypeData(rtTemp, 0, 10, "temp", 0);
	_res->allocResTypeData(rtScaleTable, 0, 5, "scale table", 0);
	_res->allocResTypeData(rtActorName, 0, _numActors, "actor name", 0);
	_res->allocResTypeData(rtVerb, 0, _numVerbs, "verb", 0);
	_res->allocResTypeData(rtString, 0, _numArray, "array", 0);
	_res->allocResTypeData(rtFlObject, 0, _numFlObject, "flobject", 0);
	_res->allocResTypeData(rtMatrix, 0, 10, "boxes", 0);
	_res->allocResTypeData(rtImage, MKID_BE('AWIZ'), _numImages, "images", 1);
	_res->allocResTypeData(rtTalkie, MKID_BE('TLKE'), _numTalkies, "talkie", 1);

	if (_game.heversion >= 70) {
		_res->allocResTypeData(rtSpoolBuffer, 0, 9, "spool buffer", 1);
		_heV7RoomIntOffsets = (uint32 *)calloc(_numRooms, sizeof(uint32));
	}
}

void ScummEngine::dumpResource(const char *tag, int idx, const byte *ptr, int length) {
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

	sprintf(buf, "dumps/%s%d.dmp", tag, idx);

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
	const byte *result = 0;

	if (_smallHeader) {
		uint16 smallTag = newTag2Old(tag);
		do {
			if (_pos >= _size)
				return 0;

			result = _ptr;
			size = READ_LE_UINT32(result);
			if ((int32)size <= 0)
				return 0;	// Avoid endless loop

			_pos += size;
			_ptr += size;
		} while (READ_LE_UINT16(result + 4) != smallTag);
	} else {
		do {
			if (_pos >= _size)
				return 0;

			result = _ptr;
			size = READ_BE_UINT32(result + 4);
			if ((int32)size <= 0)
				return 0;	// Avoid endless loop

			_pos += size;
			_ptr += size;
		} while (READ_BE_UINT32(result) != tag);
	}

	return result;
}

const byte *ScummEngine::findResource(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;

	debugC(DEBUG_RESOURCE, "findResource(%s, %p)", tag2str(tag), (const void *)searchin);

	if (!searchin) {
		if (_game.heversion >= 70) {
			searchin = _resourceLastSearchBuf;
			totalsize = _resourceLastSearchSize;
			curpos = 0;
		} else {
			assert(searchin);
			return NULL;
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
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

const byte *findResourceSmall(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;
	uint16 smallTag;

	smallTag = newTag2Old(tag);
	if (smallTag == 0)
		return NULL;

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
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

uint16 newTag2Old(uint32 newTag) {
	switch (newTag) {
	case (MKID_BE('RMHD')):
		return (0x4448);	// HD
	case (MKID_BE('IM00')):
		return (0x4D42);	// BM
	case (MKID_BE('EXCD')):
		return (0x5845);	// EX
	case (MKID_BE('ENCD')):
		return (0x4E45);	// EN
	case (MKID_BE('SCAL')):
		return (0x4153);	// SA
	case (MKID_BE('LSCR')):
		return (0x534C);	// LS
	case (MKID_BE('OBCD')):
		return (0x434F);	// OC
	case (MKID_BE('OBIM')):
		return (0x494F);	// OI
	case (MKID_BE('SMAP')):
		return (0x4D42);	// BM
	case (MKID_BE('CLUT')):
		return (0x4150);	// PA
	case (MKID_BE('BOXD')):
		return (0x5842);	// BX
	case (MKID_BE('CYCL')):
		return (0x4343);	// CC
	case (MKID_BE('EPAL')):
		return (0x5053);	// SP
	default:
		return (0);
	}
}

const char *resTypeFromId(int id) {
	static char buf[100];

	switch (id) {
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
	case rtNumTypes:
		return "NumTypes";
	default:
		sprintf(buf, "%d", id);
		return buf;
	}
}

} // End of namespace Scumm
