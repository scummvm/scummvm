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

#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"

#include "tucker/tucker.h"

namespace Tucker {

#define kSavegameSignature MKTAG('T', 'C', 'K', 'R')

enum {
	kSavegameVersionCurrent = 2,
	kSavegameVersionMinimum = 1
};

enum SavegameFlag {
	kSavegameFlagAutosave = 1 << 0
};

Common::String generateGameStateFileName(const char *target, int slot, bool prefixOnly) {
	Common::String name(target);
	if (prefixOnly) {
		name += ".#*";
	} else {
		name += Common::String::format(".%d", slot);
	}
	return name;
}

static void saveOrLoadVar(Common::WriteStream &stream, int &i) {
	stream.writeSint32LE(i);
}

static void saveOrLoadVar(Common::ReadStream &stream, int &i) {
	i = stream.readSint32LE();
}

static void saveOrLoadVar(Common::WriteStream &stream, Location &location) {
	stream.writeSint32LE((int)location);
}

static void saveOrLoadVar(Common::ReadStream &stream, Location &location) {
	location = (Location)stream.readSint32LE();
}

template<class S>
TuckerEngine::SavegameError TuckerEngine::saveOrLoadGameStateData(S &s) {
	for (int i = 0; i < kFlagsTableSize; ++i) {
		saveOrLoadVar(s, _flagsTable[i]);
	}
	for (int i = 0; i < 40; ++i) {
		saveOrLoadVar(s, _inventoryObjectsList[i]);
	}
	for (int i = 0; i < 50; ++i) {
		saveOrLoadVar(s, _inventoryItemsState[i]);
	}
	for (int i = 0; i < 50; ++i) {
		saveOrLoadVar(s, _panelObjectsOffsetTable[i]);
	}
	saveOrLoadVar(s, _mainSpritesBaseOffset);
	saveOrLoadVar(s, _selectedObject._xPos);
	saveOrLoadVar(s, _selectedObject._yPos);
	saveOrLoadVar(s, _location);
	saveOrLoadVar(s, _xPosCurrent);
	saveOrLoadVar(s, _yPosCurrent);
	saveOrLoadVar(s, _inventoryObjectsCount);
	saveOrLoadVar(s, _inventoryObjectsOffset);

	return s.err() ? kSavegameIoError : kSavegameNoError;
}

Common::Error TuckerEngine::loadGameState(int slot) {
	Common::String fileName = getSaveStateName(slot);
	Common::InSaveFile *file = _saveFileMan->openForLoading(fileName);

	if (!file) {
		return Common::kReadingFailed;
	}

	SavegameHeader header;
	SavegameError savegameError = readSavegameHeader(file, header);

	if (!savegameError) {
		savegameError = saveOrLoadGameStateData(*file);
	}

	if (savegameError) {
		switch (savegameError) {
		case kSavegameInvalidTypeError:
			warning("Invalid savegame '%s' (does not look like a ScummVM Tucker-engine savegame)", fileName.c_str());
			break;

		case kSavegameInvalidVersionError:
			warning("Invalid savegame '%s' (expected savegame version v%i-v%i, got v%i)",
				fileName.c_str(), kSavegameVersionMinimum, kSavegameVersionCurrent, header.version);
			break;

		default:
			warning("Failed to load savegame '%s'", fileName.c_str());
			break;
		}

		delete file;
		return Common::kReadingFailed;
	}

	g_engine->setTotalPlayTime(header.playTime * 1000);

	_nextLocation = _location;
	setBlackPalette();
	loadBudSpr();
	_forceRedrawPanelItems = true;
	_panelType = kPanelTypeNormal;
	setCursorState(kCursorStateNormal);

	delete file;
	return Common::kNoError;
}


WARN_UNUSED_RESULT TuckerEngine::SavegameError TuckerEngine::readSavegameHeader(const char *target, int slot, SavegameHeader &header) {
	Common::String fileName = generateGameStateFileName(target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (!file) {
		return kSavegameNotFoundError;
	}

	SavegameError savegameError = readSavegameHeader(file, header);

	delete file;
	return savegameError;
}

WARN_UNUSED_RESULT TuckerEngine::SavegameError TuckerEngine::readSavegameHeader(Common::InSaveFile *file, SavegameHeader &header, bool skipThumbnail) {
	header.version   = 0;
	header.flags     = 0;
	header.description.clear();
	header.saveDate  = 0;
	header.saveTime  = 0;
	header.playTime  = 0;
	header.thumbnail = nullptr;

	if (file->readUint32BE() == kSavegameSignature) {
		header.version = file->readUint16LE();
	} else {
		// possibly an old, headerless savegame

		file->seek(0, SEEK_SET);

		header.version = file->readUint16LE();
		// old savegames are always version 1
		if (header.version != 1) {
			return kSavegameInvalidTypeError;
		}

		file->skip(2);
	}

	if (header.version < kSavegameVersionMinimum || header.version > kSavegameVersionCurrent) {
		return kSavegameInvalidVersionError;
	}

	if (header.version >= 2) {
		// savegame flags
		header.flags = file->readUint32LE();

		char ch;
		while ((ch = (char)file->readByte()) != '\0')
			header.description += ch;

		header.saveDate = file->readUint32LE();
		header.saveTime = file->readUint32LE();
		header.playTime = file->readUint32LE();

		if (!Graphics::loadThumbnail(*file, header.thumbnail, skipThumbnail)) {
			return kSavegameIoError;
		}
	}

	return ((file->err() || file->eos()) ? kSavegameIoError : kSavegameNoError);
}

TuckerEngine::SavegameError TuckerEngine::writeSavegameHeader(Common::OutSaveFile *file, SavegameHeader &header) {
	// Tucker savegame signature
	file->writeUint32BE(kSavegameSignature);

	// version information
	file->writeUint16LE(kSavegameVersionCurrent);

	// savegame flags
	file->writeUint32LE(header.flags);

	// savegame name
	file->writeString(header.description);
	file->writeByte(0);

	// creation/play time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);
	header.saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	header.saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min)     & 0xFF) <<  8) | ((curTime.tm_sec)         & 0xFF);
	header.playTime = g_engine->getTotalPlayTime() / 1000;
	file->writeUint32LE(header.saveDate);
	file->writeUint32LE(header.saveTime);
	file->writeUint32LE(header.playTime);

	// thumbnail
	Graphics::saveThumbnail(*file);

	return (file->err() ? kSavegameIoError : kSavegameNoError);
}

Common::Error TuckerEngine::saveGameState(int slot, const Common::String &description, bool isAutosave) {
	Common::String fileName = getSaveStateName(slot);
	Common::OutSaveFile *file = _saveFileMan->openForSaving(fileName);
	SavegameHeader header;
	SavegameError savegameError = kSavegameNoError;

	if (!file)
		savegameError = kSavegameIoError;

	if (!savegameError) {
		// savegame flags
		if (isAutosave)
			header.flags |= kSavegameFlagAutosave;

		// description
		header.description = description;

		savegameError = writeSavegameHeader(file, header);
	}

	if (!savegameError)
		savegameError = saveOrLoadGameStateData(*file);

	if (!savegameError)
		file->finalize();

	delete file;

	if (savegameError) {
		warning("Error writing savegame '%s'", fileName.c_str());
		return Common::kWritingFailed;
	}

	return Common::kNoError;
}

bool TuckerEngine::canSaveAutosaveCurrently() {
	return isAutosaveAllowed(_targetName.c_str());
}

bool TuckerEngine::isAutosaveAllowed(const char *target) {
	SavegameHeader savegameHeader;
	SavegameError savegameError = readSavegameHeader(target, kAutoSaveSlot, savegameHeader);
	return (savegameError == kSavegameNotFoundError || (savegameHeader.flags & kSavegameFlagAutosave));
}

bool TuckerEngine::canLoadOrSave() const {
	return !_player && _cursorState != kCursorStateDisabledHidden;
}

bool TuckerEngine::canLoadGameStateCurrently() {
	return canLoadOrSave();
}

bool TuckerEngine::canSaveGameStateCurrently() {
	return canLoadOrSave();
}

bool TuckerEngine::existsSavegame() {
	Common::String pattern = generateGameStateFileName(_targetName.c_str(), 0, true);
	return !_saveFileMan->listSavefiles(pattern).empty();
}

} // namespace Tucker
