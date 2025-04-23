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

#include "lastexpress/game/savegame.h"

#include "lastexpress/data/cvcrfile.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/menu.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/debug.h"
#include "lastexpress/lastexpress.h"

#include "common/file.h"
#include "common/savefile.h"

namespace LastExpress {

// Labels of savegames
static const struct {
	const char *label;
} gameLabel[SaveLoad::kMaximumSaveSlots] = {
	{"blue"},
	{"red"},
	{"green"},
	{"purple"},
	{"teal"},
	{"gold"}
};

//////////////////////////////////////////////////////////////////////////
// SavegameStream
//////////////////////////////////////////////////////////////////////////

uint32 SavegameStream::write(const void *dataPtr, uint32 dataSize) {
#if !DISABLE_COMPRESSION
	if (_enableCompression)
		return writeCompressed(dataPtr, dataSize);
#endif

	return Common::MemoryWriteStreamDynamic::write(dataPtr, dataSize);
}

uint32 SavegameStream::read(void *dataPtr, uint32 dataSize) {
#if !DISABLE_COMPRESSION
	if (_enableCompression)
		return readCompressed(dataPtr, dataSize);
#endif

	return readUncompressed(dataPtr, dataSize);
}

uint32 SavegameStream::readUncompressed(void *dataPtr, uint32 dataSize) {
	if ((int32)dataSize > size() - pos()) {
		dataSize = (uint32)(size() - pos());
		_eos = true;
	}
	memcpy(dataPtr, getData() + pos(), dataSize);

	seek(dataSize, SEEK_CUR);

	return dataSize;
}

void SavegameStream::writeBuffer(uint8 value, bool onlyValue) {
	if (_bufferOffset == -1)
		_bufferOffset = 0;

	if (_bufferOffset == 256) {
		_bufferOffset = 0;
		Common::MemoryWriteStreamDynamic::write(_buffer, 256);
	}

	if (onlyValue || value < 0xFB)
		_buffer[_bufferOffset] = value;
	else
		_buffer[_bufferOffset] = 0xFE;

	_offset++;
	_bufferOffset++;

	if (!onlyValue && value >= 0xFB)
	{
		if (_bufferOffset == 256) {
			_bufferOffset = 0;
			Common::MemoryWriteStreamDynamic::write(_buffer, 256);
		}

		_buffer[_bufferOffset] = value;

		_bufferOffset++;
		_offset++;
	}
}

uint8 SavegameStream::readBuffer() {
	if (_bufferOffset == -1 || _bufferOffset >= 256) {
		readUncompressed(_buffer, 256);
		_bufferOffset = 0;
	}

	byte val = _buffer[_bufferOffset];
	_bufferOffset++;

	return val;
}

uint32 SavegameStream::process() {
	_enableCompression = !_enableCompression;

#if DISABLE_COMPRESSION
	return 0;
#else
	switch (_status) {
	default:
		break;

	case kStatusReading:
		_status = kStatusReady;
		if (_bufferOffset != -1 && _bufferOffset != 256) {
			seek(_bufferOffset - 256, SEEK_CUR);
			_bufferOffset = -1;
		}
		break;

	case kStatusWriting:
		switch (_valueCount) {
		default:
			break;

		case 1:
			writeBuffer(_previousValue, false);
			break;

		case 2:
			if (_previousValue) {
				writeBuffer(0xFF);
				writeBuffer(_repeatCount);
				writeBuffer(_previousValue);
				break;
			}

			if (_repeatCount == 3) {
				writeBuffer(0xFB);
				break;
			}

			if (_repeatCount == 255) {
				writeBuffer(0xFC);
				break;
			}

			writeBuffer(0xFD);
			writeBuffer(_repeatCount);
			break;
		}

		if (_bufferOffset != -1 && _bufferOffset != 0) {
			Common::MemoryWriteStreamDynamic::write(_buffer, _bufferOffset);
			_bufferOffset = -1;
		}
		break;
	}

	_status = kStatusReady;
	_valueCount = 0;
	uint32 offset = _offset;
	_offset = 0;

	return offset;
#endif
}

uint32 SavegameStream::writeCompressed(const void *dataPtr, uint32 dataSize) {
	if (_status == kStatusReading)
		error("[SavegameStream::writeCompressed] Error: Compression buffer is in read mode.");

	_status = kStatusWriting;
	const byte *data = (const byte *)dataPtr;

	while (dataSize) {
		switch (_valueCount) {
		default:
			error("[SavegameStream::writeCompressed] Invalid value count (%d)", _valueCount);

		case 0:
			_previousValue = *data++;
			_valueCount = 1;
			break;

		case 1:
			if (*data != _previousValue) {
				writeBuffer(_previousValue, false);
				_previousValue = *data;
			} else {
				_valueCount = 2;
				_repeatCount = 2;
			}

			++data;
			break;

		case 2:
			if (*data != _previousValue || _repeatCount >= 255) {
				if (_previousValue) {
					writeBuffer(0xFF, true);
					writeBuffer((uint8)_repeatCount, true);
					writeBuffer(_previousValue, true);

					_previousValue = *data++;
					_valueCount = 1;
					break;
				}

				if (_repeatCount == 3) {
					writeBuffer(0xFB, true);

					_previousValue = *data++;
					_valueCount = 1;
					break;
				}

				if (_repeatCount == -1) {
					writeBuffer(0xFC, true);

					_previousValue = *data++;
					_valueCount = 1;
					break;
				}

				writeBuffer(0xFD, true);
				writeBuffer((uint8)_repeatCount, true);

				_previousValue = *data++;
				_valueCount = 1;
			}

			++data;
			++_repeatCount;
			break;
		}

		--dataSize;
	}

	return _offset;
}

uint32 SavegameStream::readCompressed(void *dataPtr, uint32 dataSize) {
	if (_status == kStatusWriting)
		error("[SavegameStream::writeCompressed] Error: Compression buffer is in write mode.");

	_status = kStatusReady;
	byte *data = (byte *)dataPtr;

	while (dataSize) {
		switch (_valueCount) {
		default:
			error("[SavegameStream::readCompressed] Invalid value count (%d)", _valueCount);

		case 0:
		case 1: {
			// Read control code
			byte control = readBuffer();

			switch (control) {
			default:
				// Data value
				*data++ = control;
				break;

			case 0xFB:
				_repeatCount = 2;
				_previousValue = 0;
				*data++ = 0;
				_valueCount = 2;
				break;

			case 0xFC:
				_repeatCount = 254;
				_previousValue = 0;
				*data++ = 0;
				_valueCount = 2;
				break;

			case 0xFD:
				_repeatCount = readBuffer() - 1;
				_previousValue = 0;
				*data++ = 0;
				_valueCount = 2;
				break;

			case 0xFE:
				*data++ = readBuffer();
				break;

			case 0xFF:
				_repeatCount = readBuffer() - 1;
				_previousValue = readBuffer();
				*data++ = _previousValue;
				_valueCount = 2;
				break;
			}
			}
			break;

		case 2:
			*data++ = _previousValue;
			_repeatCount--;
			if (!_repeatCount)
				_valueCount = 1;
			break;
		}

		--dataSize;
	}

	return _offset;
}

//////////////////////////////////////////////////////////////////////////
// Constructors
//////////////////////////////////////////////////////////////////////////

SaveLoad::SaveLoad(LastExpressEngine *engine) : _engine(engine), _savegame(nullptr), _gameTicksLastSavegame(0), _entity(kCharacterCath) {
}

SaveLoad::~SaveLoad() {
	clear(true);
	_savegame = nullptr;

	// Zero passed pointers
	_engine = nullptr;
}

void SaveLoad::initStream() {
	delete _savegame;
	_savegame = new SavegameStream();
}

void SaveLoad::flushStream(const Common::String &target, GameId id) {
	Common::OutSaveFile *save = openForSaving(target, id);
	if (!save)
		error("[SaveLoad::flushStream] Cannot open savegame (%s)", getFilename(target, id).c_str());

	if (!_savegame)
		error("[SaveLoad::flushStream] Savegame stream is invalid");

	save->write(_savegame->getData(), (uint32)_savegame->size());
	save->finalize();

	delete save;
}

//////////////////////////////////////////////////////////////////////////
// Init
//////////////////////////////////////////////////////////////////////////
void SaveLoad::create(const Common::String &target, GameId id) {
	initStream();

	Common::Serializer ser(nullptr, _savegame);
	SavegameMainHeader header;
	header.saveLoadWithSerializer(ser);
	flushStream(target, id);
}

uint32 SaveLoad::init(const Common::String &target, GameId id, bool resetHeaders) {
	initStream();

	// Load game data
	loadStream(target, id);

	// Get the main header
	Common::Serializer ser(_savegame, nullptr);
	SavegameMainHeader mainHeader;
	mainHeader.saveLoadWithSerializer(ser);
	if (!mainHeader.isValid())
		error("[SaveLoad::init] Savegame seems to be corrupted (invalid header)");

	// Reset cached entry headers if needed
	if (resetHeaders) {
		clear();

		SavegameEntryHeader *entryHeader = new SavegameEntryHeader();
		// TODO This check and code (for demo case) may be removed in the future 
		if (_engine->isDemo()) {
			entryHeader->time = kTime2241000;
			entryHeader->chapter = kChapter3;
		} else {
			entryHeader->time = kTimeCityParis;
			entryHeader->chapter = kChapter1;
		}
		_gameHeaders.push_back(entryHeader);
	}

	// Read the list of entry headers
	if (_savegame->size() > 32) {
		while (_savegame->pos() < _savegame->size() && !_savegame->eos() && !_savegame->err()) {

			// Update sound queue while we go through the savegame
			getSoundQueue()->updateQueue();

			SavegameEntryHeader *entry = new SavegameEntryHeader();
			entry->saveLoadWithSerializer(ser);

			if (!entry->isValid())
				break;

			_gameHeaders.push_back(entry);

			_savegame->seek(entry->offset, SEEK_CUR);
		}
	}

	// return the index to the current save game entry (we store count + 1 entries, so we're good)
	return mainHeader.count;
}

void SaveLoad::loadStream(const Common::String &target, GameId id) {
	Common::InSaveFile *save = openForLoading(target, id);
	if (save->size() < 32)
		error("[SaveLoad::loadStream] Savegame seems to be corrupted (not enough data: %i bytes)", (int)save->size());

	if (!_savegame)
		error("[SaveLoad::loadStream] Savegame stream is invalid");

	// Load all savegame data
	uint8 *buf = new uint8[8192];
	while (!save->eos() && !save->err()) {
		_engine->pollEventsOld();

		uint32 count = save->read(buf, 8192);
		if (count) {
			uint32 w = _savegame->write(buf, count);
			assert (w == count);
		}
	}

	if (save->err())
		error("SaveLoad::init - Error reading savegame");

	delete[] buf;
	delete save;

	// Move back to the beginning of the stream
	_savegame->seek(0);
}

void SaveLoad::clear(bool clearStream) {
	for (uint i = 0; i < _gameHeaders.size(); i++)
		SAFE_DELETE(_gameHeaders[i]);

	_gameHeaders.clear();

	if (clearStream)
		SAFE_DELETE(_savegame);
}

//////////////////////////////////////////////////////////////////////////
// Save & Load
//////////////////////////////////////////////////////////////////////////

// Load last saved game
void SaveLoad::loadLastGame() {
	if (!_savegame)
		error("[SaveLoad::loadLastGame] No savegame stream present");

	// Rewind current savegame
	_savegame->seek(0);

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "Cannot load main header: %s", getFilename(_engine->getTargetName(), getMenuOld()->getGameId()).c_str());
		return;
	}

	// Load the last entry
	_savegame->seek(header.offsetEntry);

	SavegameType type = kSavegameTypeIndex;
	CharacterIndex entity = kCharacterCath;
	uint32 val = 0;
	readEntry(&type, &entity, &val, header.keepIndex == 1);

	// Setup last loading time
	_gameTicksLastSavegame = getState()->timeTicks;

	if (header.keepIndex) {
		getSoundQueue()->destroyAllSound();

		readEntry(&type, &entity, &val, false);
	}
	getEntities()->reset();
	getEntities()->setup(false, entity);
}

// Load a specific game entry
void SaveLoad::loadGame(uint32 index) {
	if (!_savegame)
		error("[SaveLoad::loadLastGame] No savegame stream present");

	// Rewind current savegame
	_savegame->seek(0);

	// Write main header (with selected index)
	SavegameMainHeader header;
	header.count = index;
	header.brightness = getState()->brightness;
	header.volume = getState()->volume;

	Common::Serializer ser(nullptr, _savegame);
	header.saveLoadWithSerializer(ser);

	// TODO
	// Go to the entry
	// Load the entry
	// Get offset (main and entry)
	// Write main header again with correct entry offset
	// Setup game and start

	error("[SaveLoad::loadGame] Not implemented! (only loading the last entry is working for now)");
}

// Save game
void SaveLoad::saveGame(SavegameType type, CharacterIndex entity, uint32 value) {
	if (getState()->scene <= kSceneIntro)
		return;

	// Validate main header
	SavegameMainHeader header;
	if (!loadMainHeader(_savegame, &header)) {
		debugC(2, kLastExpressDebugSavegame, "Cannot load main header: %s", getFilename(_engine->getTargetName(), getMenuOld()->getGameId()).c_str());
		return;
	}

	if (!_savegame)
		error("[SaveLoad::saveGame] Savegame stream is invalid");

	// Validate the current entry if it exists
	if (header.count > 0) {
		_savegame->seek(header.offsetEntry);

		// Load entry header
		SavegameEntryHeader entry;
		Common::Serializer ser(_savegame, nullptr);
		entry.saveLoadWithSerializer(ser);

		if (!entry.isValid()) {
			warning("[SaveLoad::saveGame] Invalid entry. This savegame might be corrupted");
			_savegame->seek(header.offset);
		} else if (getState()->time < entry.time || (type == kSavegameTypeTickInterval && getState()->time == entry.time)) {
			// Not ready to save a game, skipping!
			return;
		} else if ((type == kSavegameTypeTime || type == kSavegameTypeEvent)
			&& (entry.type == kSavegameTypeTickInterval && (getState()->time - entry.time) < 450)) {
			_savegame->seek(header.offsetEntry);
			--header.count;
		} else {
			_savegame->seek(header.offset);
		}
	} else {
		// Seek to the next savegame entry
		_savegame->seek(header.offset);
	}

	if (type != kSavegameTypeEvent2 && type != kSavegameTypeAuto)
		header.offsetEntry = (uint32)_savegame->pos();

	// Write the savegame entry
	writeEntry(type, entity, value);

	if (!header.keepIndex)
		++header.count;

	if (type == kSavegameTypeEvent2 || type == kSavegameTypeAuto) {
		header.keepIndex = 1;
	} else {
		header.keepIndex = 0;
		header.offset = (uint32)_savegame->pos();

		// Save ticks
		_gameTicksLastSavegame = getState()->timeTicks;
	}

	// Validate the main header
	if (!header.isValid())
		error("[SaveLoad::saveGame] Main game header is invalid");

	// Write the main header
	_savegame->seek(0);
	Common::Serializer ser(nullptr, _savegame);
	header.saveLoadWithSerializer(ser);

	flushStream(_engine->getTargetName(), getMenuOld()->getGameId());
}

void SaveLoad::saveVolumeBrightness() {
	warning("[SaveLoad::saveVolumeBrightness] Not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Headers
//////////////////////////////////////////////////////////////////////////
bool SaveLoad::loadMainHeader(Common::InSaveFile *stream, SavegameMainHeader *header) {
	if (!stream)
		return false;

	// Check there is enough data (32 bytes)
	if (stream->size() < 32) {
		debugC(2, kLastExpressDebugSavegame, "Savegame seems to be corrupted (not enough data: %i bytes)", (int)stream->size());
		return false;
	}

	// Rewind stream
	stream->seek(0);

	Common::Serializer ser(stream, nullptr);
	header->saveLoadWithSerializer(ser);

	// Validate the header
	if (!header->isValid()) {
		debugC(2, kLastExpressDebugSavegame, "Cannot validate main header");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Entries
//////////////////////////////////////////////////////////////////////////
uint32 SaveLoad::writeValue(Common::Serializer &ser, const char *name, Common::Functor1<Common::Serializer &, void> *function, uint size) {
	if (!_savegame)
		error("[SaveLoad::writeValue] Stream not initialized properly");

	debugC(kLastExpressDebugSavegame, "Savegame: Writing %s: %u bytes", name, size);

	uint32 prevPosition = (uint32)_savegame->pos();

	// Serialize data into our buffer
	(*function)(ser);

	uint32 count = (uint32)_savegame->pos() - prevPosition;

#if DISABLE_COMPRESSION
	if (count != size)
		error("[SaveLoad::writeValue] %s - Number of bytes written (%d) differ from expected count (%d)", name, count, size);
#endif

	return count;
}

uint32 SaveLoad::readValue(Common::Serializer &ser, const char *name, Common::Functor1<Common::Serializer &, void> *function, uint size) {
	if (!_savegame)
		error("[SaveLoad::readValue] Stream not initialized properly");

	debugC(kLastExpressDebugSavegame, "Savegame: Reading %s: %u bytes", name, size);

	uint32 prevPosition = (uint32)_savegame->pos();

	(*function)(ser);

	uint32 count = (uint32)_savegame->pos() - prevPosition;

#if DISABLE_COMPRESSION
	if (size != 0 && count != size)
		error("[SaveLoad::readValue] %s - Number of bytes read (%d) differ from expected count (%d)", name, count, size);
#endif

	return count;
}

void SaveLoad::syncEntity(Common::Serializer &ser) {
	ser.syncAsUint32LE(_entity);
}

void SaveLoad::writeEntry(SavegameType type, CharacterIndex entity, uint32 value) {
	if (!_savegame)
		error("[SaveLoad::writeEntry] Savegame stream is invalid");

	SavegameEntryHeader header;

	header.type = type;
	header.time = (uint32)getState()->time;
	header.chapter = getProgress().chapter;
	header.value = value;

	// Save position
	uint32 originalPosition = (uint32)_savegame->pos();

	// Write header
	Common::Serializer ser(nullptr, _savegame);
	header.saveLoadWithSerializer(ser);

	// Write game data
	_entity = entity;

	_savegame->process();
	writeValue(ser, "character index", WRAP_SYNC_FUNCTION(this, SaveLoad, syncEntity), 4);
	writeValue(ser, "state", WRAP_SYNC_FUNCTION(getState(), State::GameState, saveLoadWithSerializer), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	writeValue(ser, "selected item", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveSelectedItem), 4);
	writeValue(ser, "positions", WRAP_SYNC_FUNCTION(getEntities(), Entities, savePositions), 4 * 1000);
	writeValue(ser, "compartments", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveCompartments), 4 * 16 * 2);
	writeValue(ser, "progress", WRAP_SYNC_FUNCTION(&getProgress(), State::GameProgress, saveLoadWithSerializer), 4 * 128);
	writeValue(ser, "events", WRAP_SYNC_FUNCTION(getState(), State::GameState, syncEvents), 512);
	writeValue(ser, "inventory", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveLoadWithSerializer), 7 * 32);
	writeValue(ser, "objects", WRAP_SYNC_FUNCTION(getObjects(), Objects, saveLoadWithSerializer), 5 * 128);
	writeValue(ser, "entities", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveLoadWithSerializer), 1262 * 40);
	writeValue(ser, "sound", WRAP_SYNC_FUNCTION(getSoundQueue(), SoundQueue, saveLoadWithSerializer), 3 * 4 + getSoundQueue()->count() * 68);
	writeValue(ser, "savepoints", WRAP_SYNC_FUNCTION(getSavePoints(), SavePoints, saveLoadWithSerializer), 128 * 16 + 4 + getSavePoints()->count() * 16);
	_savegame->process();

	header.offset = (uint32)_savegame->pos() - (originalPosition + 32);

	// Add padding if necessary
	while (header.offset & 0xF) {
		_savegame->writeByte(0);
		header.offset++;
	}

	// Save end position
	uint32 endPosition = (uint32)_savegame->pos();

	// Validate entry header
	if (!header.isValid())
		error("[SaveLoad::writeEntry] Entry header is invalid");

	// Save the header with the updated info
	_savegame->seek(originalPosition);
	header.saveLoadWithSerializer(ser);

	// Move back to the end of the entry
	_savegame->seek(endPosition);
}

void SaveLoad::readEntry(SavegameType *type, CharacterIndex *entity, uint32 *val, bool keepIndex) {
	if (!type || !entity || !val)
		error("[SaveLoad::readEntry] Invalid parameters passed");

	if (!_savegame)
		error("[SaveLoad::readEntry] No savegame stream present");

	// Load entry header
	SavegameEntryHeader entry;
	Common::Serializer ser(_savegame, nullptr);
	entry.saveLoadWithSerializer(ser);

	if (!entry.isValid())
		error("[SaveLoad::readEntry] Entry header is invalid");

	// Init type, character & value
	*type = entry.type;
	*val = entry.value;

	// Save position
	uint32 originalPosition = (uint32)_savegame->pos();

	// Load game data
	_savegame->process();
	readValue(ser, "character index", WRAP_SYNC_FUNCTION(this, SaveLoad, syncEntity), 4);
	readValue(ser, "state", WRAP_SYNC_FUNCTION(getState(), State::GameState, saveLoadWithSerializer), 4 + 4 + 4 + 4 + 1 + 4 + 4);
	readValue(ser, "selected item", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveSelectedItem), 4);
	readValue(ser, "positions", WRAP_SYNC_FUNCTION(getEntities(), Entities, savePositions), 4 * 1000);
	readValue(ser, "compartments", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveCompartments), 4 * 16 * 2);
	readValue(ser, "progress", WRAP_SYNC_FUNCTION(&getProgress(), State::GameProgress, saveLoadWithSerializer), 4 * 128);
	readValue(ser, "events", WRAP_SYNC_FUNCTION(getState(), State::GameState, syncEvents), 512);
	readValue(ser, "inventory", WRAP_SYNC_FUNCTION(getInventory(), Inventory, saveLoadWithSerializer), 7 * 32);
	readValue(ser, "objects", WRAP_SYNC_FUNCTION(getObjects(), Objects, saveLoadWithSerializer), 5 * 128);
	readValue(ser, "entities", WRAP_SYNC_FUNCTION(getEntities(), Entities, saveLoadWithSerializer), 1262 * 40);
	readValue(ser, "sound", WRAP_SYNC_FUNCTION(getSoundQueue(), SoundQueue, saveLoadWithSerializer));
	readValue(ser, "savepoints", WRAP_SYNC_FUNCTION(getSavePoints(), SavePoints, saveLoadWithSerializer));
	_savegame->process();

	// Update chapter
	*entity = _entity;
	getProgress().chapter = entry.chapter;

	// Skip padding
	uint32 offset = (uint32)_savegame->pos() - originalPosition;
	if (offset & 0xF) {
		// (offset & 0xF) is a value in [0, 15]; the remainder of division of offset with 16.
		// Entering here, that remainder is not zero so, with the following code, we skip the padding
		// by seeking ahead (forward) from SEEK_CUR for the amount of the bytes required to complete
		// a full 16 bytes final segment for the character entry that we are reading.
		// That is: 16 - (offset & 0xF)  or equivalently: (~offset & 0xF) + 1) bytes skipped ahead.
		_savegame->seek(16 - (offset & 0xF), SEEK_CUR);
	}
}

SaveLoad::SavegameEntryHeader *SaveLoad::getEntry(uint32 index) {
	if (index >= _gameHeaders.size())
		error("[SaveLoad::getEntry] Invalid index (was:%d, max:%d)", index, _gameHeaders.size() - 1);

	return _gameHeaders[index];
}

SaveStateList SaveLoad::list(const MetaEngine *metaEngine, const Common::String &target) {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray files = saveFileMan->listSavefiles(target + "*.egg");

	SaveStateList saveList;
	for (Common::StringArray::const_iterator fileName = files.begin(); fileName != files.end(); ++fileName) {
		for (int i = 0; i < kMaximumSaveSlots; ++i) {
			// Do another more accurate filtering (than the more generic pattern used with listSavefiles() above)
			// of save file names here
			if (*fileName == getFilename(target, (GameId)i)) {
				Common::InSaveFile *saveFile = saveFileMan->openForLoading(*fileName);
				if (saveFile != nullptr && !saveFile->err()) {
					saveList.push_back(SaveStateDescriptor(metaEngine, i, gameLabel[i].label));
				}
				delete saveFile;
				break;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());

	return saveList;
}

//////////////////////////////////////////////////////////////////////////
// Checks
//////////////////////////////////////////////////////////////////////////

// Check if a specific savegame exists
bool SaveLoad::isSavegamePresent(const Common::String &target, GameId id) {
	if (g_system->getSavefileManager()->listSavefiles(getFilename(target, id)).size() == 0)
		return false;

	return true;
}

// Check if the game has been started in the specific savegame
bool SaveLoad::isSavegameValid(const Common::String &target, GameId id) {
	if (!isSavegamePresent(target, id)) {
		debugC(2, kLastExpressDebugSavegame, "Savegame does not exist: %s", getFilename(target, id).c_str());
		return false;
	}

	SavegameMainHeader header;

	Common::InSaveFile *save = openForLoading(target, id);
	bool isHeaderValid = loadMainHeader(save, &header);
	delete save;

	return isHeaderValid;
}

bool SaveLoad::isGameFinished(uint32 menuIndex, uint32 savegameIndex) {
	SavegameEntryHeader *data = getEntry(menuIndex);

	if (savegameIndex != menuIndex)
		return false;

	if (data->type != kSavegameTypeEvent)
		return false;

	return (data->value == kEventAnnaKilled
	     || data->value == kEventKronosHostageAnnaNoFirebird
	     || data->value == kEventKahinaPunchBaggageCarEntrance
	     || data->value == kEventKahinaPunchBlue
	     || data->value == kEventKahinaPunchYellow
	     || data->value == kEventKahinaPunchSalon
	     || data->value == kEventKahinaPunchKitchen
	     || data->value == kEventKahinaPunchBaggageCar
	     || data->value == kEventKahinaPunchCar
	     || data->value == kEventKahinaPunchSuite4
	     || data->value == kEventKahinaPunchRestaurant
	     || data->value == kEventKahinaPunch
	     || data->value == kEventKronosGiveFirebird
	     || data->value == kEventAugustFindCorpse
	     || data->value == kEventMertensBloodJacket
	     || data->value == kEventMertensCorpseFloor
	     || data->value == kEventMertensCorpseBed
	     || data->value == kEventCoudertBloodJacket
	     || data->value == kEventGendarmesArrestation
	     || data->value == kEventAbbotDrinkGiveDetonator
	     || data->value == kEventMilosCorpseFloor
	     || data->value == kEventLocomotiveAnnaStopsTrain
	     || data->value == kEventTrainStopped
	     || data->value == kEventCathVesnaRestaurantKilled
	     || data->value == kEventCathVesnaTrainTopKilled
	     || data->value == kEventLocomotiveConductorsDiscovered
	     || data->value == kEventViennaAugustUnloadGuns
	     || data->value == kEventViennaKronosFirebird
	     || data->value == kEventVergesAnnaDead
	     || data->value == kEventTrainExplosionBridge
	     || data->value == kEventKronosBringNothing);
}


//////////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////////

// Get the file eraseData from the savegame ID
Common::String SaveLoad::getFilename(const Common::String &target, GameId id) {
	if (id < 0 || id >= kMaximumSaveSlots)
		error("[SaveLoad::getFilename] Attempting to use an invalid game id. Valid values: 0 - %d, was %d", kMaximumSaveSlots - 1, id);

	return target + "-" + gameLabel[id].label + ".egg";
}

Common::InSaveFile *SaveLoad::openForLoading(const Common::String &target, GameId id) {
	Common::InSaveFile *load = g_system->getSavefileManager()->openForLoading(getFilename(target, id));

	if (!load)
		debugC(2, kLastExpressDebugSavegame, "Cannot open savegame for loading: %s", getFilename(target, id).c_str());

	return load;
}

Common::OutSaveFile *SaveLoad::openForSaving(const Common::String &target, GameId id) {
	Common::OutSaveFile *save = g_system->getSavefileManager()->openForSaving(getFilename(target, id), false); // TODO Enable compression again

	if (!save)
		debugC(2, kLastExpressDebugSavegame, "Cannot open savegame for writing: %s", getFilename(target, id).c_str());

	return save;
}

bool SaveLoad::remove(const Common::String &target, GameId id) {
	Common::String filename = getFilename(target, id);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveManager::SaveManager(LastExpressEngine *engine) {
	_engine = engine;
}

void SaveManager::writeSavePoint(CVCRFile *file, int saveType, int character, int value) {
	int32 originalFilePos;
	int32 paddingSize;
	int32 posAfterWriting;
	SVCRSavePointHeader savePointHeader;
	byte emptyHeader[15];

	savePointHeader.saveType = saveType;
	savePointHeader.signature = 0xE660E660;
	savePointHeader.headerSize = 0;
	savePointHeader.gameTime = _engine->getLogicManager()->_gameTime;
	savePointHeader.chapter = _engine->getLogicManager()->_gameProgress[kProgressChapter];
	savePointHeader.latestGameEvent = value;
	savePointHeader.emptyField1 = 0;
	savePointHeader.emptyField2 = 0;

	originalFilePos = file->tell();
	file->write(&savePointHeader, sizeof(SVCRSavePointHeader), 1, 0);
	file->flush();
	file->writeRLE(&character, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_gameTime, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_gameTimeTicksDelta, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_currentGameSessionTicks, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_trainNodeIndex, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_useLastSavedNodeIndex, 1, 1);
	file->writeRLE(&_engine->getLogicManager()->_lastNodeIndex, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_lastSavedNodeIndex, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_inventorySelectedItemIdx, 4, 1);

	file->writeRLE(_engine->getLogicManager()->_positions, 4, 1000);
	file->writeRLE(_engine->getLogicManager()->_blockedEntitiesBits, 4, 16);
	file->writeRLE(_engine->getLogicManager()->_softBlockedEntitiesBits, 4, 16);
	file->writeRLE(_engine->getLogicManager()->_gameProgress, 4, 128);
	file->writeRLE(_engine->getLogicManager()->_gameEvents, 1, 512);

	// Handle complex types (which were fine in the original, but not within a crossplatform context)...
	byte *inventoryBuffer = (byte *)malloc(32 * 7); // 32 items, 7 bytes each
	byte *objectsBuffer = (byte *)malloc(128 * 5); // 128 objects, 5 bytes each
	byte *charactersBuffer = (byte *)malloc(40 * 1262); // 128 objects, 5 bytes each

	assert(inventoryBuffer && objectsBuffer && charactersBuffer);

	// Copy Item data...
	for (int i = 0; i < 32; i++) {
		int offset = i * 7;
		inventoryBuffer[offset] = _engine->getLogicManager()->_gameInventory[i].cursor;
		WRITE_LE_UINT16(&inventoryBuffer[offset + 1], _engine->getLogicManager()->_gameInventory[i].scene);
		inventoryBuffer[offset + 3] = _engine->getLogicManager()->_gameInventory[i].isSelectable;
		inventoryBuffer[offset + 4] = _engine->getLogicManager()->_gameInventory[i].isPresent;
		inventoryBuffer[offset + 5] = _engine->getLogicManager()->_gameInventory[i].manualSelect;
		inventoryBuffer[offset + 6] = _engine->getLogicManager()->_gameInventory[i].location;
	}

	file->writeRLE(inventoryBuffer, 7, 32);

	// Copy Object data...
	for (int i = 0; i < 128; i++) {
		int offset = i * 5;
		objectsBuffer[offset] = _engine->getLogicManager()->_gameObjects[i].character;
		objectsBuffer[offset + 1] = _engine->getLogicManager()->_gameObjects[i].door;
		objectsBuffer[offset + 2] = _engine->getLogicManager()->_gameObjects[i].cursor;
		objectsBuffer[offset + 3] = _engine->getLogicManager()->_gameObjects[i].cursor2;
		objectsBuffer[offset + 4] = _engine->getLogicManager()->_gameObjects[i].model;
	}

	file->writeRLE(objectsBuffer, 5, 128);

	// Copy Character data...
	for (int i = 0; i < 40; i++) {
		int offset = 0;
		Character characterStruct = getCharacter(i);

		// First copy CallParams (9 sets of 32 integers)...
		for (int j = 0; j < 9; j++) {
			for (int k = 0; k < 32; k++) {
				WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], characterStruct.callParams[j].parameters[k]);
				offset += 4;
			}
		}

		// Copy callbacks array (16 bytes)...
		for (int j = 0; j < 16; j++) {
			charactersBuffer[i * 1262 + offset++] = characterStruct.callbacks[j];
		}

		// Copy currentCall (1 byte)...
		charactersBuffer[i * 1262 + offset++] = characterStruct.currentCall;

		// Copy characterPosition (3 uint16)...
		WRITE_LE_UINT16(&charactersBuffer[i * 1262 + offset], characterStruct.characterPosition.position); offset += 2;
		WRITE_LE_UINT16(&charactersBuffer[i * 1262 + offset], characterStruct.characterPosition.location); offset += 2;
		WRITE_LE_UINT16(&charactersBuffer[i * 1262 + offset], characterStruct.characterPosition.car); offset += 2;

		// Copy the remaining basic fields...
		charactersBuffer[i * 1262 + offset++] = characterStruct.walkCounter;
		charactersBuffer[i * 1262 + offset++] = characterStruct.attachedConductor;
		charactersBuffer[i * 1262 + offset++] = characterStruct.inventoryItem;
		charactersBuffer[i * 1262 + offset++] = characterStruct.direction;

		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.waitedTicksUntilCycleRestart); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.currentFrameSeq1); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.currentFrameSeq2); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.elapsedFrames); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.walkStepSize); offset += 2;

		charactersBuffer[i * 1262 + offset++] = characterStruct.clothes;
		charactersBuffer[i * 1262 + offset++] = characterStruct.position2;
		charactersBuffer[i * 1262 + offset++] = characterStruct.car2;
		charactersBuffer[i * 1262 + offset++] = characterStruct.doProcessEntity;
		charactersBuffer[i * 1262 + offset++] = characterStruct.field_4A9;
		charactersBuffer[i * 1262 + offset++] = characterStruct.field_4AA;
		charactersBuffer[i * 1262 + offset++] = characterStruct.directionSwitch;

		// Copy string fields
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceName, 13); offset += 13;
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceName2, 13); offset += 13;
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceNamePrefix, 7); offset += 7;
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceNameCopy, 13); offset += 13;

		// Set pointers to zero (each 4 bytes)
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // frame1
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // frame2
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // sequence1
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // sequence2
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // sequence3

		// At this point, offset should equal 1262!
		assert(offset == 1262);
	}

	file->writeRLE(charactersBuffer, 1262, 40);

	free(inventoryBuffer);
	free(objectsBuffer);
	free(charactersBuffer);

	_engine->getSoundManager()->saveSoundInfo(file);
	_engine->getMessageManager()->saveMessages(file);

	savePointHeader.headerSize = file->flush();

	if ((savePointHeader.headerSize & 0xF) != 0) {
		memset(emptyHeader, 0, sizeof(emptyHeader));
		paddingSize = ((~(savePointHeader.headerSize & 0xFF) & 0xF) + 1);
		file->write(&emptyHeader, 1, paddingSize, 0);
		savePointHeader.headerSize += paddingSize;
	}

	posAfterWriting = file->tell();
	file->seek(originalFilePos, 0);
	checkSavePointHeader(&savePointHeader);
	file->write(&savePointHeader, 32, 1, false);
	file->seek(posAfterWriting, 0);
}

void SaveManager::readSavePoint(CVCRFile *file, int *saveType, uint8 *character, int *saveEvent, bool skipSoundLoading) {
	int latestGameEvent;
	int32 originalPos;
	int32 posDiff;
	SVCRSavePointHeader savePointHeader;

	if (saveType && character && saveEvent) {
		*saveType = 1;
		*character = kCharacterCath;
		*saveEvent = 0;

		file->read(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false, true);

		if (checkSavePointHeader(&savePointHeader)) {
			latestGameEvent = savePointHeader.latestGameEvent;
			*saveType = savePointHeader.saveType;
			*saveEvent = latestGameEvent;
			file->flush();
			originalPos = file->tell();

			// The original treats the "character" arg as uint8, but then asks
			// for a four bytes integer, causing a stack corruption around it.
			// This is our workaround...
			int32 intCharacter;
			file->readRLE(&intCharacter, 4, 1);
			*character = (uint8)(intCharacter & 0xFF);

			if (*character >= 40) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_gameTime, 4, 1);
			if (_engine->getLogicManager()->_gameTime < 1061100) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			if (_engine->getLogicManager()->_gameTime > 4941000) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_gameTimeTicksDelta, 4, 1);
			if (_engine->getLogicManager()->_gameTime > 4941000) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_currentGameSessionTicks, 4, 1);

			file->readRLE(&_engine->getLogicManager()->_trainNodeIndex, 4, 1);
			if (_engine->getLogicManager()->_trainNodeIndex >= 2500) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_useLastSavedNodeIndex, 1, 1);
			if (_engine->getLogicManager()->_useLastSavedNodeIndex > 1) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_lastNodeIndex, 4, 1);
			if (_engine->getLogicManager()->_lastNodeIndex >= 2500) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_lastSavedNodeIndex, 4, 1);
			if (_engine->getLogicManager()->_lastSavedNodeIndex >= 2500) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_inventorySelectedItemIdx, 4, 1);
			if (_engine->getLogicManager()->_inventorySelectedItemIdx >= 32) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(_engine->getLogicManager()->_positions, 4, 1000);
			file->readRLE(_engine->getLogicManager()->_blockedEntitiesBits, 4, 16);
			file->readRLE(_engine->getLogicManager()->_softBlockedEntitiesBits, 4, 16);
			file->readRLE(_engine->getLogicManager()->_gameProgress, 4, 128);
			file->readRLE(_engine->getLogicManager()->_gameEvents, 1, 512);

			// Handle complex types (which were fine in the original, but not within a crossplatform context)...
			byte *inventoryBuffer = (byte *)malloc(32 * 7);     // 32 items, 7 bytes each
			byte *objectsBuffer = (byte *)malloc(128 * 5);      // 128 objects, 5 bytes each
			byte *charactersBuffer = (byte *)malloc(40 * 1262); // 40 characters, 1262 bytes each

			assert(inventoryBuffer && objectsBuffer && charactersBuffer);

			// Read data from file
			file->readRLE(inventoryBuffer, 7, 32);
			file->readRLE(objectsBuffer, 5, 128);
			file->readRLE(charactersBuffer, 1262, 40);

			// Copy Item data from buffer to structures
			for (int i = 0; i < 32; i++) {
				int offset = i * 7;
				_engine->getLogicManager()->_gameInventory[i].cursor = inventoryBuffer[offset];
				_engine->getLogicManager()->_gameInventory[i].scene = READ_LE_UINT16(&inventoryBuffer[offset + 1]);
				_engine->getLogicManager()->_gameInventory[i].isSelectable = inventoryBuffer[offset + 3];
				_engine->getLogicManager()->_gameInventory[i].isPresent = inventoryBuffer[offset + 4];
				_engine->getLogicManager()->_gameInventory[i].manualSelect = inventoryBuffer[offset + 5];
				_engine->getLogicManager()->_gameInventory[i].location = inventoryBuffer[offset + 6];
			}

			// Copy Object data from buffer to structures
			for (int i = 0; i < 128; i++) {
				int offset = i * 5;
				_engine->getLogicManager()->_gameObjects[i].character = objectsBuffer[offset];
				_engine->getLogicManager()->_gameObjects[i].door = objectsBuffer[offset + 1];
				_engine->getLogicManager()->_gameObjects[i].cursor = objectsBuffer[offset + 2];
				_engine->getLogicManager()->_gameObjects[i].cursor2 = objectsBuffer[offset + 3];
				_engine->getLogicManager()->_gameObjects[i].model = objectsBuffer[offset + 4];
			}

			// Copy Character data from buffer to structures
			for (int i = 0; i < 40; i++) {
				int offset = 0;
				Character *characterStruct = &getCharacter(i);

				// Copy CallParams (9 sets of 32 integers)
				for (int j = 0; j < 9; j++) {
					for (int k = 0; k < 32; k++) {
						characterStruct->callParams[j].parameters[k] = READ_LE_UINT32(&charactersBuffer[i * 1262 + offset]);
						offset += 4;
					}
				}

				// Copy callbacks array (16 bytes)
				for (int j = 0; j < 16; j++) {
					characterStruct->callbacks[j] = charactersBuffer[i * 1262 + offset]; offset++;
				}

				// Copy currentCall (1 byte)
				characterStruct->currentCall = charactersBuffer[i * 1262 + offset]; offset++;

				// Copy characterPosition (3 uint16)
				characterStruct->characterPosition.position = READ_LE_UINT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->characterPosition.location = READ_LE_UINT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->characterPosition.car = READ_LE_UINT16(&charactersBuffer[i * 1262 + offset]); offset += 2;

				// Copy the remaining basic fields
				characterStruct->walkCounter = charactersBuffer[i * 1262 + offset++];
				characterStruct->attachedConductor = charactersBuffer[i * 1262 + offset++];
				characterStruct->inventoryItem = charactersBuffer[i * 1262 + offset++];
				characterStruct->direction = charactersBuffer[i * 1262 + offset++];

				characterStruct->waitedTicksUntilCycleRestart = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->currentFrameSeq1 = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->currentFrameSeq2 = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->elapsedFrames = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->walkStepSize = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;

				characterStruct->clothes = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->position2 = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->car2 = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->doProcessEntity = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->field_4A9 = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->field_4AA = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->directionSwitch = charactersBuffer[i * 1262 + offset]; offset++;

				// Copy string fields
				memcpy(characterStruct->sequenceName, &charactersBuffer[i * 1262 + offset], 13); offset += 13;
				memcpy(characterStruct->sequenceName2, &charactersBuffer[i * 1262 + offset], 13); offset += 13;
				memcpy(characterStruct->sequenceNamePrefix, &charactersBuffer[i * 1262 + offset], 7); offset += 7;
				memcpy(characterStruct->sequenceNameCopy, &charactersBuffer[i * 1262 + offset], 13); offset += 13;

				// Skip pointer data...
				offset += 4; // frame1
				offset += 4; // frame2
				offset += 4; // sequence1
				offset += 4; // sequence2
				offset += 4; // sequence3

				// At this point, offset should equal 1262!
				assert(offset == 1262);
			}

			free(inventoryBuffer);
			free(objectsBuffer);
			free(charactersBuffer);

			_engine->getSoundManager()->loadSoundInfo(file, skipSoundLoading);
			_engine->getMessageManager()->loadMessages(file);

			_engine->getLogicManager()->_gameProgress[kProgressChapter] = savePointHeader.chapter;

			file->flush();

			posDiff = (file->tell() - originalPos) & 0xFF;
			if ((posDiff & 0xF) != 0)
				file->seek(((~posDiff & 0xF) + 1), 1);

			for (int i = 0; i < 40; i++) {
				getCharacter(i).frame1 = nullptr;
				getCharacter(i).frame2 = nullptr;
				getCharacter(i).sequence1 = nullptr;
				getCharacter(i).sequence2 = nullptr;
				getCharacter(i).sequence3 = nullptr;
			}
		}
	}
}

void SaveManager::validateSaveFile(bool flag) {
	SVCRSavePointHeader savePointHeader;
	SVCRFileHeader fileHeader;

	CVCRFile *tempFile = new CVCRFile(_engine);
	CVCRFile *saveFile = new CVCRFile(_engine);

	bool hasValidationError = false;

	if (flag) {
		if (_engine->_savePointHeaders)
			_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);

		_engine->_savePointHeaders = (SVCRSavePointHeader *)_engine->getMemoryManager()->allocMem(
			sizeof(SVCRSavePointHeader), _engine->_savegameFilename, kCharacterMaster
		);

		if (!_engine->_savePointHeaders) {
			error("Out of memory");
		}

		_engine->_savePointHeaders->gameTime = 1037700;
		_engine->_savePointHeaders->chapter = 1;
	}

	saveFile->open(_engine->_savegameFilename, CVCRMODE_RB);

	saveFile->seek(0, SEEK_END);
	int fileSize = saveFile->tell();
	saveFile->seek(0, SEEK_SET);

	if (fileSize >= sizeof(SVCRFileHeader)) {
		saveFile->read(&fileHeader, sizeof(SVCRFileHeader), 1, false, true);
		if (checkFileHeader(&fileHeader)) {
			if (flag) {
				if (_engine->_savePointHeaders)
					_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);

				_engine->_savePointHeaders = (SVCRSavePointHeader *)_engine->getMemoryManager()->allocMem(
					sizeof(SVCRSavePointHeader) * (fileHeader.numSavePoints + 1),
					_engine->_savegameFilename,
					kCharacterMaster
				);

				if (!_engine->_savePointHeaders) {
					error("Out of memory");
				}

				_engine->_savePointHeaders->gameTime = 1037700;
				_engine->_savePointHeaders->chapter = 1;
			}

			for (int i = 0; fileSize >= sizeof(SVCRFileHeader) && i < fileHeader.numSavePoints; ++i) {
				_engine->getSoundManager()->soundThread();

				saveFile->read(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false, true);
				if (flag) {
					memcpy(&_engine->_savePointHeaders[i + 1], &savePointHeader, sizeof(savePointHeader));
				}

				fileSize -= sizeof(SVCRSavePointHeader);
				if (fileSize >= 0) {
					if (checkSavePointHeader(&savePointHeader)) {
						fileSize -= savePointHeader.headerSize;
						if (fileSize >= 0) {
							saveFile->seek(savePointHeader.headerSize, SEEK_CUR);
						} else {
							hasValidationError = true;
						}
					} else {
						fileSize = 0;
						hasValidationError = true;
					}
				} else {
					hasValidationError = true;
				}
			}

			saveFile->close();
		} else {
			hasValidationError = true;
			saveFile->close();
		}
	} else {
		hasValidationError = true;
		saveFile->close();
	}

	if (hasValidationError) {
		saveFile->open(_engine->_savegameFilename, CVCRMODE_RB);
		saveFile->seek(0, SEEK_END);
		fileSize = saveFile->tell();
		saveFile->seek(0, SEEK_SET);

		if (fileSize < sizeof(SVCRFileHeader)) {
			if (fileSize) {
				error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
			}

			saveFile->close();
			_engine->getVCR()->virginSaveFile();

			delete tempFile;
			delete saveFile;

			return;
		}

		saveFile->read(&fileHeader, sizeof(SVCRFileHeader), 1, false, true);
		if (!checkFileHeader(&fileHeader)) {
			error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
			saveFile->close();
			_engine->getVCR()->virginSaveFile();

			delete tempFile;
			delete saveFile;

			return;
		}

		if (flag) {
			if (_engine->_savePointHeaders)
				_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);

			_engine->_savePointHeaders = (SVCRSavePointHeader *)_engine->getMemoryManager()->allocMem(
				sizeof(SVCRSavePointHeader) * (fileHeader.numSavePoints + 1),
				_engine->_savegameFilename,
				kCharacterMaster
			);

			if (!_engine->_savePointHeaders) {
				error("Out of memory");
			}

			_engine->_savePointHeaders->gameTime = 1037700;
			_engine->_savePointHeaders->chapter = 1;
		}

		int offset = sizeof(SVCRFileHeader);
		tempFile->open("temp.egg", CVCRMODE_WB);
		tempFile->seek(sizeof(SVCRFileHeader), 0);
		tempFile->close();
		fileHeader.savePointsOffset = sizeof(SVCRFileHeader);
		fileHeader.offset = sizeof(SVCRFileHeader);
		fileHeader.skipSoundLoading = 0;
		int numSavePoints = 0;

		for (int j = 0; true; j++) {
			if (fileSize < sizeof(SVCRFileHeader) || j >= fileHeader.numSavePoints) {
				saveFile->close();
				fileHeader.numSavePoints = numSavePoints;
				tempFile->open("temp.egg", CVCRMODE_RWB);
				tempFile->seek(0, SEEK_SET);
				tempFile->write(&fileHeader, sizeof(SVCRFileHeader), 1, false);
				tempFile->seek(offset, SEEK_SET);
				tempFile->close();

				if (removeSavegame(_engine->_savegameFilename)) {
					error("Error deleting file \"%s\"", _engine->_savegameFilename);
				} else if (renameSavegame("temp.egg", _engine->_savegameFilename)) {
					error("Error renaming file \"%s\" to \"%s\"", "temp.egg", _engine->_savegameFilename);
				}

				delete tempFile;
				delete saveFile;

				return;
			}

			_engine->getSoundManager()->soundThread();
			saveFile->read(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false, true);
			if (flag) {
				memcpy(&_engine->_savePointHeaders[j + 1], &savePointHeader, sizeof(savePointHeader));
			}

			fileSize -= sizeof(SVCRSavePointHeader);
			if (fileSize < 0)
				break;

			if (checkSavePointHeader(&savePointHeader)) {
				fileSize -= savePointHeader.headerSize;
				if (fileSize < 0)
					break;

				numSavePoints++;

				byte *tempMem = (byte *)malloc(savePointHeader.headerSize);

				saveFile->read(tempMem, savePointHeader.headerSize, 1, false, true);
				tempFile->open("temp.egg", CVCRMODE_RWB);
				tempFile->seek(offset, SEEK_SET);

				if (savePointHeader.saveType != 3 && savePointHeader.saveType != 4)
					fileHeader.savePointsOffset = offset;

				tempFile->write(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false);
				tempFile->write(tempMem, savePointHeader.headerSize, 1, false);
				tempFile->close();

				free(tempMem);

				offset += savePointHeader.headerSize + sizeof(SVCRSavePointHeader);
				if (savePointHeader.saveType == 3 || savePointHeader.saveType == 4) {
					fileHeader.skipSoundLoading = 1;
				} else {
					fileHeader.skipSoundLoading = 0;
					fileHeader.offset = offset;
				}
			} else {
				fileSize = 0;
				error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
			}
		}

		error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
	}

	delete tempFile;
	delete saveFile;
}

bool SaveManager::checkFileHeader(SVCRFileHeader *fileHeader) {
	if (fileHeader->signature == 0x12001200 &&
		fileHeader->numSavePoints >= 0 &&
		fileHeader->offset >= sizeof(SVCRFileHeader) &&
		fileHeader->savePointsOffset >= sizeof(SVCRFileHeader) &&
		fileHeader->skipSoundLoading < 2 &&
		fileHeader->gammaLevel <= 6 &&
		fileHeader->volume < 8) {

		if (fileHeader->saveVersion == 9) {
			return true;
		} else {
			error("Save game file \"%s\" is incompatible with this version of the game", _engine->_savegameFilename);
			return false;
		}
	} else {
		error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
		return false;
	}
}

bool SaveManager::checkSavePointHeader(SVCRSavePointHeader *savePointHeader) {
	if (savePointHeader->signature == 0xE660E660) {
		if (savePointHeader->saveType > 0 && savePointHeader->saveType <= 5) {
			if (savePointHeader->gameTime >= 1061100 && savePointHeader->gameTime <= 4941000) {
				if (savePointHeader->headerSize > 0 && (savePointHeader->headerSize & 0xF) == 0 && savePointHeader->chapter > 0)
					return true;
			}
		}
	}

	error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);

	return false;
}

void SaveManager::continueGame() {
	uint8 character;
	int saveEvent;
	int saveType;
	SVCRFileHeader header;

	_engine->_savegame->open(_engine->_savegameFilename, CVCRMODE_RB);
	_engine->_savegame->read(&header, sizeof(SVCRFileHeader), 1, false, true);

	if (checkFileHeader(&header)) {
		_engine->_savegame->seek(header.savePointsOffset, SEEK_SET);
		readSavePoint(_engine->_savegame, &saveType, &character, &saveEvent, header.skipSoundLoading);
		_engine->getLogicManager()->_lastSavegameSessionTicks = _engine->getLogicManager()->_currentGameSessionTicks;

		if (header.skipSoundLoading) {
			_engine->getSoundManager()->destroyAllSound();
			readSavePoint(_engine->_savegame, &saveType, &character, &saveEvent, 0);
		}

		_engine->_savegame->close();
		_engine->getOtisManager()->wipeAllGSysInfo();
		_engine->getLogicManager()->CONS_All(false, character);
	} else {
		_engine->_savegame->close();
	}
}

void SaveManager::startRewoundGame() {
	SVCRFileHeader header;
	SVCRSavePointHeader savePointHeader;
	int saveEvent;
	int saveType;
	uint8 character = 0;

	CVCRFile *saveFile = new CVCRFile(_engine);
	byte *buf = (byte *)malloc(0x2000);

	_engine->_fightSkipCounter = 0;

	_engine->_savegame->open(_engine->_savegameNames[_engine->_currentGameFileColorId], CVCRMODE_RB);
	_engine->_savegameFilename = _engine->_savegameTempNames[_engine->_currentGameFileColorId];
	saveFile->open(_engine->_savegameFilename, CVCRMODE_WB);

	header.offset = sizeof(SVCRFileHeader);
	header.numSavePoints = _engine->_currentSavePoint;
	header.savePointsOffset = sizeof(SVCRFileHeader);
	header.signature = 0x12001200;
	header.skipSoundLoading = 0;
	header.gammaLevel = _engine->getGraphicsManager()->getGammaLevel();
	header.volume = _engine->getSoundManager()->getMasterVolume();
	header.saveVersion = 9;
	saveFile->write(&header, sizeof(header), 1, false);

	_engine->_savegame->seek(sizeof(header), SEEK_SET);

	if (_engine->_currentSavePoint > 1) {
		int count = _engine->_currentSavePoint - 1;
		do {
			_engine->_savegame->read(&savePointHeader, sizeof(savePointHeader), 1, false, true);
			saveFile->write(&savePointHeader, sizeof(savePointHeader), 1, false);
			for (; savePointHeader.headerSize > 0x2000; savePointHeader.headerSize -= 0x2000) {
				_engine->_savegame->read(buf, 0x2000, 1, false, true);
				saveFile->write(buf, 0x2000, 1, false);
			}
			_engine->_savegame->read(buf, savePointHeader.headerSize, 1, false, true);
			saveFile->write(buf, savePointHeader.headerSize, 1, false);
			--count;
		} while (count);
	}

	free(buf);
	buf = nullptr;

	if (_engine->_currentSavePoint) {
		readSavePoint(_engine->_savegame, &saveType, &character, &saveEvent, false);
		_engine->_savegame->close();
		_engine->getLogicManager()->_lastSavegameSessionTicks = _engine->getLogicManager()->_currentGameSessionTicks;
		header.savePointsOffset = saveFile->tell();

		writeSavePoint(saveFile, saveType, character, saveEvent);

		header.offset = saveFile->tell();
		checkFileHeader(&header);
		saveFile->seek(0, SEEK_SET);
		saveFile->write(&header, sizeof(header), 1, false);
	}

	_engine->_savegame->close();
	saveFile->close();
	delete saveFile;

	_engine->_gracePeriodIndex = _engine->_currentSavePoint;
	_engine->_gracePeriodTimer = _engine->getLogicManager()->_gameProgress[kProgressJacket] < 2 ? 225 : 450;

	if (_engine->_currentSavePoint) {
		_engine->getOtisManager()->wipeAllGSysInfo();
		_engine->getLogicManager()->CONS_All(false, character);
	} else {
		_engine->startNewGame();
	}
}

bool SaveManager::fileExists(const char *filename) {
	return g_system->getSavefileManager()->exists(_engine->getTargetName() + "-" + Common::String(filename));
}

bool SaveManager::removeSavegame(const char *filename) {
	return !g_system->getSavefileManager()->removeSavefile(_engine->getTargetName() + "-" + Common::String(filename));
}

bool SaveManager::renameSavegame(const char *oldName, const char *newName) {
	return !g_system->getSavefileManager()->renameSavefile(
		_engine->getTargetName() + "-" + Common::String(oldName),
		_engine->getTargetName() + "-" + Common::String(newName), false);
}

} // End of namespace LastExpress
