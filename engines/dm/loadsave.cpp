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
#include "graphics/thumbnail.h"

#include "dm.h"
#include "dungeonman.h"
#include "timeline.h"
#include "group.h"
#include "champion.h"
#include "menus.h"
#include "eventman.h"
#include "projexpl.h"


namespace DM {
#define C2_FORMAT_DM_AMIGA_2X_PC98_X68000_FM_TOWNS_CSB_ATARI_ST 2
#define C3_PLATFORM_AMIGA 3
#define C10_DUNGEON_DM 10
LoadgameResponse DMEngine::f435_loadgame(int16 slot) {
	Common::String fileName;
	Common::SaveFileManager *saveFileManager = nullptr;
	Common::InSaveFile *file = nullptr;

	if (!_g298_newGame) {
		fileName = getSavefileName(slot);
		saveFileManager = _system->getSavefileManager();
		file = saveFileManager->openForLoading(fileName);
	}

	_g528_saveFormat = C2_FORMAT_DM_AMIGA_2X_PC98_X68000_FM_TOWNS_CSB_ATARI_ST;
	_g527_platform = C3_PLATFORM_AMIGA;
	_g526_dungeonId = C10_DUNGEON_DM;
	if (_g298_newGame) {
		//L1366_B_FadePalette = !F0428_DIALOG_RequireGameDiskInDrive_NoDialogDrawn(C0_DO_NOT_FORCE_DIALOG_DM_CSB, true);
T0435002:
		_g524_restartGameAllowed = false;
		_championMan->_g305_partyChampionCount = 0;
		_championMan->_g414_leaderHandObject = Thing::_none;
		_g525_gameId = ((int32)getRandomNumber(65536)) * getRandomNumber(65536);
	} else {
		warning(false, "MISSING CODE: missing check for matching _g525_gameId in f435_loadgame");
		/*if (_vm->_g523_restartGameRequest && (L1372_s_SaveHeader.GameID != _vm->_g525_gameId)) {
			L1367_pc_Message = G0546_pc_THATSNOTTHESAMEGAME;
			goto T0435004;
		}*/

		SaveGameHeader header;
		readSaveGameHeader(file, &header);

		warning(false, "MISSING CODE: missing check for matching format and platform in save in f435_loadgame");


		_g313_gameTime = file->readSint32BE();
		// G0349_ul_LastRandomNumber = L1371_s_GlobalData.LastRandomNumber;
		_championMan->_g305_partyChampionCount = file->readUint16BE();
		_dungeonMan->_g306_partyMapX = file->readSint16BE();
		_dungeonMan->_g307_partyMapY = file->readSint16BE();
		_dungeonMan->_g308_partyDir = (direction)file->readUint16BE();
		_dungeonMan->_g309_partyMapIndex = file->readByte();
		_championMan->_g411_leaderIndex = (ChampionIndex)file->readSint16BE();
		_championMan->_g514_magicCasterChampionIndex = (ChampionIndex)file->readSint16BE();
		_timeline->_g372_eventCount = file->readUint16BE();
		_timeline->_g373_firstUnusedEventIndex = file->readUint16BE();
		_timeline->_g369_eventMaxCount = file->readUint16BE();
		_groupMan->_g377_currActiveGroupCount = file->readUint16BE();
		_projexpl->_g361_lastCreatureAttackTime = file->readSint32BE();
		_projexpl->_g362_lastPartyMovementTime = file->readSint32BE();
		_g310_disabledMovementTicks = file->readSint16BE();
		_g311_projectileDisableMovementTicks = file->readSint16BE();
		_g312_lastProjectileDisabledMovementDirection = file->readSint16BE();
		_championMan->_g414_leaderHandObject = Thing(file->readUint16BE());
		_groupMan->_g376_maxActiveGroupCount = file->readUint16BE();
		if (!_g523_restartGameRequest) {
			_timeline->f233_initTimeline();
			_groupMan->f196_initActiveGroups();
		}

		_groupMan->load1_ActiveGroupPart(file);
		_championMan->load2_PartyPart(file);
		_timeline->load3_eventsPart(file);
		_timeline->load4_timelinePart(file);

		_g525_gameId = file->readSint32BE();
	}

	_dungeonMan->f434_loadDungeonFile();
	if (_g298_newGame) {
		_timeline->f233_initTimeline();
		_groupMan->f196_initActiveGroups();
		warning(false, "MISSING CODE: missing fadePlette stuff in f435_loadgame on newGame");
		/*
		if (L1366_B_FadePalette) {
			F0436_STARTEND_FadeToPalette(G0345_aui_BlankBuffer);
			D26_WaitForVerticalBlank();
			D18_FillScreenBlack();
			F0436_STARTEND_FadeToPalette(_vm->_displayMan->_g347_paletteTopAndBottomScreen);
		}*/
	} else {
		_g528_saveFormat = file->readSint16BE();
		_g527_platform = file->readSint16BE();
		_g526_dungeonId = file->readUint16BE();

		_g524_restartGameAllowed = true;
		warning(false, "MISSING CDOE: F0427_DIALOG_Draw in f435_loadgame");
	}
	_championMan->_g303_partyDead = false;

	delete file;
	return k1_LoadgameSuccess;
}


void DMEngine::f433_processCommand140_saveGame(uint16 slot, const Common::String desc) {
	char *message = nullptr;

	_menuMan->f456_drawDisabledMenu();
	_eventMan->f78_showMouse();

	// do {
	//		ask the player what he wants
	// while 

	// F0427_DIALOG_Draw(0, G0551_pc_SAVINGGAME, 0, 0, 0, 0, false, false, false);

	uint16 champHandObjWeight;
	if (!_championMan->_g415_leaderEmptyHanded) {
		champHandObjWeight = _dungeonMan->f140_getObjectWeight(_championMan->_g414_leaderHandObject);
		_championMan->_gK71_champions[_championMan->_g411_leaderIndex]._load -= champHandObjWeight;
	}


	Common::String savefileName = getSavefileName(slot);
	Common::SaveFileManager *saveFileManager = _system->getSavefileManager();
	Common::OutSaveFile *file = saveFileManager->openForSaving(savefileName);

	if (!file)
		return; // TODO: silent fail

	writeSaveGameHeader(file, desc);

	// write C0_SAVE_PART_GLOBAL_DATA part
	file->writeSint32BE(_g313_gameTime);
	//L1348_s_GlobalData.LastRandomNumber = G0349_ul_LastRandomNumber;
	file->writeUint16BE(_championMan->_g305_partyChampionCount);
	file->writeSint16BE(_dungeonMan->_g306_partyMapX);
	file->writeSint16BE(_dungeonMan->_g307_partyMapY);
	file->writeUint16BE(_dungeonMan->_g308_partyDir);
	file->writeByte(_dungeonMan->_g309_partyMapIndex);
	file->writeSint16BE(_championMan->_g411_leaderIndex);
	file->writeSint16BE(_championMan->_g514_magicCasterChampionIndex);
	file->writeUint16BE(_timeline->_g372_eventCount);
	file->writeUint16BE(_timeline->_g373_firstUnusedEventIndex);
	file->writeUint16BE(_timeline->_g369_eventMaxCount);
	file->writeUint16BE(_groupMan->_g377_currActiveGroupCount);
	file->writeSint32BE(_projexpl->_g361_lastCreatureAttackTime);
	file->writeSint32BE(_projexpl->_g362_lastPartyMovementTime);
	file->writeSint16BE(_g310_disabledMovementTicks);
	file->writeSint16BE(_g311_projectileDisableMovementTicks);
	file->writeSint16BE(_g312_lastProjectileDisabledMovementDirection);
	file->writeUint16BE(_championMan->_g414_leaderHandObject.toUint16());
	file->writeUint16BE(_groupMan->_g376_maxActiveGroupCount);

	// write C1_SAVE_PART_ACTIVE_GROUP part
	_groupMan->save1_ActiveGroupPart(file);
	// write C2_SAVE_PART_PARTY part
	_championMan->save2_PartyPart(file);
	// write C3_SAVE_PART_EVENTS part
	_timeline->save3_eventsPart(file);
	// write C4_SAVE_PART_TIMELINE part
	_timeline->save4_timelinePart(file);

	file->writeSint32BE(_g525_gameId);
	file->writeSint16BE(_g528_saveFormat);
	file->writeSint16BE(_g527_platform);
	file->writeUint16BE(_g526_dungeonId);

	warning(false, "MISSING CODE in save game");

	file->flush();
	file->finalize();
	delete file;
}

Common::String DMEngine::getSavefileName(uint16 slot) {
	return Common::String::format("%s.%03u", _targetName.c_str(), slot);
}

#define SAVEGAME_ID       MKTAG('D', 'M', 'D', 'M')
#define SAVEGAME_VERSION  1

void DMEngine::writeSaveGameHeader(Common::OutSaveFile* out, const Common::String& saveName) {
	out->writeUint32BE(SAVEGAME_ID);

	// Write version
	out->writeByte(SAVEGAME_VERSION);

	// Write savegame name
	out->writeString(saveName);
	out->writeByte(0);

	// Save the game thumbnail
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


bool DMEngine::readSaveGameHeader(Common::InSaveFile* in, SaveGameHeader* header) {
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
	header->_descr.setThumbnail(Graphics::loadThumbnail(*in));

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

