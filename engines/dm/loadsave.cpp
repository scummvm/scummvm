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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "gui/saveload.h"

#include "dm/dm.h"
#include "dm/dungeonman.h"
#include "dm/timeline.h"
#include "dm/group.h"
#include "dm/champion.h"
#include "dm/menus.h"
#include "dm/eventman.h"
#include "dm/projexpl.h"
#include "dm/dialog.h"

namespace DM {

LoadgameResult DMEngine::loadgame(int16 slot) {
	if (slot == -1 && _gameMode == kDMModeLoadSavedGame)
		return kDMLoadgameFailure;

	bool fadePalette = true;
	Common::String fileName;
	Common::SaveFileManager *saveFileManager = nullptr;
	Common::InSaveFile *file = nullptr;

	struct {
		SaveTarget _saveTarget;
		int32 _saveVersion;
		OriginalSaveFormat _saveFormat;
		OriginalSavePlatform _savePlatform;
		uint16 _dungeonId;
	} dmSaveHeader;

	if (_gameMode != kDMModeLoadSavedGame) {
		//L1366_B_FadePalette = !F0428_DIALOG_RequireGameDiskInDrive_NoDialogDrawn(C0_DO_NOT_FORCE_DIALOG_DM_CSB, true);
		_restartGameAllowed = false;
		_championMan->_partyChampionCount = 0;
		_championMan->_leaderHandObject = _thingNone;
	} else {
		fileName = getSavefileName(slot);
		saveFileManager = _system->getSavefileManager();
		file = saveFileManager->openForLoading(fileName);

		SaveGameHeader header;
		if (!readSaveGameHeader(file, &header)) {
			delete file;
			return kDMLoadgameFailure;
		}

		warning("MISSING CODE: missing check for matching format and platform in save in f435_loadgame");

		dmSaveHeader._saveTarget = (SaveTarget)file->readSint32BE();
		dmSaveHeader._saveVersion = file->readSint32BE();
		dmSaveHeader._saveFormat = (OriginalSaveFormat)file->readSint32BE();
		dmSaveHeader._savePlatform = (OriginalSavePlatform)file->readSint32BE();

		// Skip _gameId, which was useless
		file->readSint32BE();
		dmSaveHeader._dungeonId = file->readUint16BE();

		_gameTime = file->readSint32BE();
		// G0349_ul_LastRandomNumber = L1371_s_GlobalData.LastRandomNumber;
		_championMan->_partyChampionCount = file->readUint16BE();
		_dungeonMan->_partyMapX = file->readSint16BE();
		_dungeonMan->_partyMapY = file->readSint16BE();
		_dungeonMan->_partyDir = (Direction)file->readUint16BE();
		_dungeonMan->_partyMapIndex = file->readByte();
		_championMan->_leaderIndex = (ChampionIndex)file->readSint16BE();
		_championMan->_magicCasterChampionIndex = (ChampionIndex)file->readSint16BE();
		_timeline->_eventCount = file->readUint16BE();
		_timeline->_firstUnusedEventIndex = file->readUint16BE();
		_timeline->_eventMaxCount = file->readUint16BE();
		_groupMan->_currActiveGroupCount = file->readUint16BE();
		_projexpl->_lastCreatureAttackTime = file->readSint32BE();
		_projexpl->_lastPartyMovementTime = file->readSint32BE();
		_disabledMovementTicks = file->readSint16BE();
		_projectileDisableMovementTicks = file->readSint16BE();
		_lastProjectileDisabledMovementDirection = file->readSint16BE();
		_championMan->_leaderHandObject = Thing(file->readUint16BE());
		_groupMan->_maxActiveGroupCount = file->readUint16BE();
		if (!_restartGameRequest) {
			_timeline->initTimeline();
			_groupMan->initActiveGroups();
		}

		_groupMan->loadActiveGroupPart(file);
		_championMan->loadPartyPart2(file);
		_timeline->loadEventsPart(file);
		_timeline->loadTimelinePart(file);

		// read sentinel
		uint32 sentinel = file->readUint32BE();
		assert(sentinel == 0x6f85e3d3);

		_dungeonId = dmSaveHeader._dungeonId;
	}

	_dungeonMan->loadDungeonFile(file);
	delete file;

	if (_gameMode != kDMModeLoadSavedGame) {
		_timeline->initTimeline();
		_groupMan->initActiveGroups();

		if (fadePalette) {
			_displayMan->startEndFadeToPalette(_displayMan->_blankBuffer);
			delay(1);
			_displayMan->fillScreen(kDMColorBlack);
			_displayMan->startEndFadeToPalette(_displayMan->_paletteTopAndBottomScreen);
		}
	} else {
		_restartGameAllowed = true;

		switch (getGameLanguage()) { // localized
		case Common::DE_DEU:
			_dialog->dialogDraw(nullptr, "SPIEL WIRD GELADEN . . .", nullptr, nullptr, nullptr, nullptr, true, true, true);
			break;
		case Common::FR_FRA:
			_dialog->dialogDraw(nullptr, "CHARGEMENT DU JEU . . .", nullptr, nullptr, nullptr, nullptr, true, true, true);
			break;
		default:
			_dialog->dialogDraw(nullptr, "LOADING GAME . . .", nullptr, nullptr, nullptr, nullptr, true, true, true);
			break;
		}
	}
	_championMan->_partyDead = false;

	return kDMLoadgameSuccess;
}


void DMEngine::saveGame() {
	_menuMan->drawDisabledMenu();
	_eventMan->showMouse();

	switch (getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		_dialog->dialogDraw(nullptr, nullptr, "SAVE AND PLAY", "SAVE AND QUIT", "CANCEL", "LOAD", false, false, false);
		break;
	case Common::DE_DEU:
		_dialog->dialogDraw(nullptr, nullptr, "SICHERN/SPIEL", "SICHERN/ENDEN", "WIDERRUFEN", "LOAD", false, false, false);
		break;
	case Common::FR_FRA:
		_dialog->dialogDraw(nullptr, nullptr, "GARDER/JOUER", "GARDER/SORTIR", "ANNULLER", "LOAD", false, false, false);
		break;
	}

	enum SaveAndPlayChoice {
		kSaveAndPlay = 1,
		kSaveAndQuit = 2,
		kCancel = 3,
		kLoad = 4
	};

	SaveAndPlayChoice saveAndPlayChoice = (SaveAndPlayChoice)_dialog->getChoice(4, kDMDialogCommandSetViewport, 0, kDMDialogChoiceNone);

	if (saveAndPlayChoice == kLoad) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		int loadSlot = dialog->runModalWithCurrentTarget();
		delete dialog;
		if (loadSlot >= 0) {
			_loadSaveSlotAtRuntime = loadSlot;
			return;
		}

		saveAndPlayChoice = kCancel;
	}

	if (saveAndPlayChoice == kSaveAndQuit || saveAndPlayChoice == kSaveAndPlay) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		int16 saveSlot = dialog->runModalWithCurrentTarget();
		Common::String saveDescription = dialog->getResultString();
		if (saveDescription.empty())
			saveDescription = "Nice save ^^";
		delete dialog;

		if (saveSlot >= 0) {
			switch (getGameLanguage()) { // localized
			default:
			case Common::EN_ANY:
				_dialog->dialogDraw(nullptr, "SAVING GAME . . .", nullptr, nullptr, nullptr, nullptr, false, false, false);
				break;
			case Common::DE_DEU:
				_dialog->dialogDraw(nullptr, "SPIEL WIRD GESICHERT . . .", nullptr, nullptr, nullptr, nullptr, false, false, false);
				break;
			case Common::FR_FRA:
				_dialog->dialogDraw(nullptr, "UN MOMENT A SAUVEGARDER DU JEU...", nullptr, nullptr, nullptr, nullptr, false, false, false);
				break;
			}

			uint16 champHandObjWeight = 0;
			if (!_championMan->_leaderEmptyHanded) {
				champHandObjWeight = _dungeonMan->getObjectWeight(_championMan->_leaderHandObject);
				_championMan->_champions[_championMan->_leaderIndex]._load -= champHandObjWeight;
			}

			if (!writeCompleteSaveFile(saveSlot, saveDescription, saveAndPlayChoice)) {
				_dialog->dialogDraw(nullptr, "Unable to open file for saving", "OK", nullptr, nullptr, nullptr, false, false, false);
				_dialog->getChoice(1, kDMDialogCommandSetViewport, 0, kDMDialogChoiceNone);
			}

			if (!_championMan->_leaderEmptyHanded) {
				_championMan->_champions[_championMan->_leaderIndex]._load += champHandObjWeight;
			}
		} else
			saveAndPlayChoice = kCancel;
	}


	if (saveAndPlayChoice == kSaveAndQuit) {
		_eventMan->hideMouse();
		endGame(false);
	}

	_restartGameAllowed = true;
	_menuMan->drawEnabledMenus();
	_eventMan->hideMouse();
}

Common::String DMEngine::getSavefileName(uint16 slot) {
	return Common::String::format("%s.%03u", _targetName.c_str(), slot);
}

#define SAVEGAME_ID       MKTAG('D', 'M', '2', '1')
#define SAVEGAME_VERSION  1

void DMEngine::writeSaveGameHeader(Common::OutSaveFile *out, const Common::String& saveName) {
	out->writeUint32BE(SAVEGAME_ID);

	// Write version
	out->writeByte(SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(saveName);
	out->writeByte(0);

	// Save the game thumbnail
	if (_saveThumbnail)
		out->write(_saveThumbnail->getData(), _saveThumbnail->size());
	else
		Graphics::saveThumbnail(*out);

	// Creation date/time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = getTotalPlayTime() / 1000;

	out->writeUint32BE(saveDate);
	out->writeUint16BE(saveTime);
	out->writeUint32BE(playTime);
}

bool DMEngine::writeCompleteSaveFile(int16 saveSlot, Common::String& saveDescription, int16 saveAndPlayChoice) {
	Common::String savefileName = getSavefileName(saveSlot);
	Common::SaveFileManager *saveFileManager = _system->getSavefileManager();
	Common::OutSaveFile *file = saveFileManager->openForSaving(savefileName);

	if (!file)
		return false;

	writeSaveGameHeader(file, saveDescription);

	file->writeSint32BE(_gameVersion->_saveTargetToWrite);
	file->writeSint32BE(1); // save version
	file->writeSint32BE(_gameVersion->_origSaveFormatToWrite);
	file->writeSint32BE(_gameVersion->_origPlatformToWrite);

	// Was _gameID, useless.
	file->writeSint32BE(0);
	file->writeUint16BE(_dungeonId);

	// write C0_SAVE_PART_GLOBAL_DATA part
	file->writeSint32BE(_gameTime);
	//L1348_s_GlobalData.LastRandomNumber = G0349_ul_LastRandomNumber;
	file->writeUint16BE(_championMan->_partyChampionCount);
	file->writeSint16BE(_dungeonMan->_partyMapX);
	file->writeSint16BE(_dungeonMan->_partyMapY);
	file->writeUint16BE(_dungeonMan->_partyDir);
	file->writeByte(_dungeonMan->_partyMapIndex);
	file->writeSint16BE(_championMan->_leaderIndex);
	file->writeSint16BE(_championMan->_magicCasterChampionIndex);
	file->writeUint16BE(_timeline->_eventCount);
	file->writeUint16BE(_timeline->_firstUnusedEventIndex);
	file->writeUint16BE(_timeline->_eventMaxCount);
	file->writeUint16BE(_groupMan->_currActiveGroupCount);
	file->writeSint32BE(_projexpl->_lastCreatureAttackTime);
	file->writeSint32BE(_projexpl->_lastPartyMovementTime);
	file->writeSint16BE(_disabledMovementTicks);
	file->writeSint16BE(_projectileDisableMovementTicks);
	file->writeSint16BE(_lastProjectileDisabledMovementDirection);
	file->writeUint16BE(_championMan->_leaderHandObject.toUint16());
	file->writeUint16BE(_groupMan->_maxActiveGroupCount);

	// write C1_SAVE_PART_ACTIVE_GROUP part
	_groupMan->saveActiveGroupPart(file);
	// write C2_SAVE_PART_PARTY part
	_championMan->savePartyPart2(file);
	// write C3_SAVE_PART_EVENTS part
	_timeline->saveEventsPart(file);
	// write C4_SAVE_PART_TIMELINE part
	_timeline->saveTimelinePart(file);

	// write sentinel
	file->writeUint32BE(0x6f85e3d3);

	// save _g278_dungeonFileHeader
	DungeonFileHeader &header = _dungeonMan->_dungeonFileHeader;
	file->writeUint16BE(header._ornamentRandomSeed);
	file->writeUint16BE(header._rawMapDataSize);
	file->writeByte(header._mapCount);
	file->writeByte(0); // to match the structure of dungeon.dat, will be discarded
	file->writeUint16BE(header._textDataWordCount);
	file->writeUint16BE(header._partyStartLocation);
	file->writeUint16BE(header._squareFirstThingCount);
	for (uint16 i = 0; i < 16; ++i)
		file->writeUint16BE(header._thingCounts[i]);

	// save _g277_dungeonMaps
	for (uint16 i = 0; i < _dungeonMan->_dungeonFileHeader._mapCount; ++i) {
		Map &map = _dungeonMan->_dungeonMaps[i];

		file->writeUint16BE(map._rawDunDataOffset);
		file->writeUint32BE(0); // to match the structure of dungeon.dat, will be discarded
		file->writeByte(map._offsetMapX);
		file->writeByte(map._offsetMapY);

		uint16 tmp;
		tmp = ((map._height & 0x1F) << 11) | ((map._width & 0x1F) << 6) | (map._level & 0x3F);
		file->writeUint16BE(tmp);

		tmp = ((map._randFloorOrnCount & 0xF) << 12) | ((map._floorOrnCount & 0xF) << 8)
			| ((map._randWallOrnCount & 0xF) << 4) | (map._wallOrnCount & 0xF);
		file->writeUint16BE(tmp);

		tmp = ((map._difficulty & 0xF) << 12) | ((map._creatureTypeCount & 0xF) << 4) | (map._doorOrnCount & 0xF);
		file->writeUint16BE(tmp);

		tmp = ((map._doorSet1 & 0xF) << 12) | ((map._doorSet0 & 0xF) << 8)
			| ((map._wallSet & 0xF) << 4) | (map._floorSet & 0xF);
		file->writeUint16BE(tmp);
	}

	// save _g280_dungeonColumnsCumulativeSquareThingCount
	for (uint16 i = 0; i < _dungeonMan->_dungeonColumCount; ++i)
		file->writeUint16BE(_dungeonMan->_dungeonColumnsCumulativeSquareThingCount[i]);

	// save _g283_squareFirstThings
	for (uint16 i = 0; i < _dungeonMan->_dungeonFileHeader._squareFirstThingCount; ++i)
		file->writeUint16BE(_dungeonMan->_squareFirstThings[i].toUint16());

	// save _g260_dungeonTextData
	for (uint16 i = 0; i < _dungeonMan->_dungeonFileHeader._textDataWordCount; ++i)
		file->writeUint16BE(_dungeonMan->_dungeonTextData[i]);

	// save _g284_thingData
	for (uint16 thingIndex = 0; thingIndex < 16; ++thingIndex)
		for (uint16 i = 0; i < _dungeonMan->_thingDataWordCount[thingIndex] * _dungeonMan->_dungeonFileHeader._thingCounts[thingIndex]; ++i)
			file->writeUint16BE(_dungeonMan->_thingData[thingIndex][i]);

	// save _g276_dungeonRawMapData
	for (uint32 i = 0; i < _dungeonMan->_dungeonFileHeader._rawMapDataSize; ++i)
		file->writeByte(_dungeonMan->_dungeonRawMapData[i]);

	file->flush();
	file->finalize();
	delete file;

	return true;
}

WARN_UNUSED_RESULT bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader *header, bool skipThumbnail) {
	uint32 id = in->readUint32BE();

	// Check if it's a valid ScummVM savegame
	if (id != SAVEGAME_ID)
		return false;

	// Read in the version
	header->_version = in->readByte();

	// Check that the save version isn't newer than this binary
	if (header->_version > SAVEGAME_VERSION)
		return false;

	// Read in the save name
	Common::String saveName;
	char ch;
	while ((ch = (char)in->readByte()) != '\0')
		saveName += ch;
	header->_descr.setDescription(saveName);

	// Get the thumbnail
	Graphics::Surface *thumbnail;
	if (!Graphics::loadThumbnail(*in, thumbnail, skipThumbnail)) {
		return false;
	}
	header->_descr.setThumbnail(thumbnail);

	uint32 saveDate = in->readUint32BE();
	uint16 saveTime = in->readUint16BE();
	uint32 playTime = in->readUint32BE();

	int day = (saveDate >> 24) & 0xFF;
	int month = (saveDate >> 16) & 0xFF;
	int year = saveDate & 0xFFFF;
	header->_descr.setSaveDate(year, month, day);

	int hour = (saveTime >> 8) & 0xFF;
	int minutes = saveTime & 0xFF;
	header->_descr.setSaveTime(hour, minutes);

	header->_descr.setPlayTime(playTime * 1000);
	if (g_engine)
		g_engine->setTotalPlayTime(playTime * 1000);

	return true;
}

}

