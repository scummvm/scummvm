/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#include "kyra/kyra.h"
#include "kyra/animator.h"
#include "kyra/screen.h"
#include "kyra/resource.h"

#include "common/savefile.h"
#include "common/system.h"

#define CURRENT_VERSION 4

namespace Kyra {
void KyraEngine::loadGame(const char *fileName) {
	debugC(9, kDebugLevelMain, "loadGame('%s')", fileName);
	Common::InSaveFile *in;

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName);
		return;
	}

	uint32 type = in->readUint32BE();

	// FIXME: The kyra savegame code used to be endian unsafe. Uncomment the
	// following line to graciously handle old savegames from LE machines.
	// if (type != MKID_BE('KYRA') && type != MKID_BE('ARYK')) {
	if (type != MKID_BE('KYRA')) {
		warning("No Kyrandia 1 savefile header");
		delete in;
		return;
	}
	uint32 version = in->readUint32BE();
	if (version > CURRENT_VERSION) {
		warning("Savegame is not the right version (%d)", version);
		delete in;
		return;
	}
	
	char saveName[31];
	in->read(saveName, 31);

	if (version >= 2) {
		uint32 gameFlags = in->readUint32BE();
		if ((gameFlags & GF_FLOPPY) && !(_features & GF_FLOPPY)) {
			warning("can not load floppy savefile for this (non floppy) gameversion!");
			delete in;
			return;
		} else if ((gameFlags & GF_TALKIE) && !(_features & GF_TALKIE)) {
			warning("can not load cdrom savefile for this (non cdrom) gameversion!");
			delete in;
			return;
		}
	} else {
		warning("Make sure your savefile was from this version! (too old savefile version to detect that)");
	}
	
	snd_playSoundEffect(0x0A);
	snd_playWanderScoreViaMap(0, 1);

	// unload the current voice file should fix some problems with voices
	if (_currentRoom != 0xFFFF && (_features & GF_TALKIE)) {
		char file[32];
		assert(_currentRoom < _roomTableSize);
		int tableId = _roomTable[_currentRoom].nameIndex;
		assert(tableId < _roomFilenameTableSize);
		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".VRM");
		_res->unloadPakFile(file);
	}

	int brandonX = 0, brandonY = 0;
	for (int i = 0; i < 11; i++) {
		_characterList[i].sceneId = in->readUint16BE();
		_characterList[i].height = in->readByte();
		_characterList[i].facing = in->readByte();
		_characterList[i].currentAnimFrame = in->readUint16BE();
		//_characterList[i].unk6 = in->readUint32BE();
		in->read(_characterList[i].inventoryItems, 10);
		_characterList[i].x1 = in->readSint16BE();
		_characterList[i].y1 = in->readSint16BE();
		_characterList[i].x2 = in->readSint16BE();
		_characterList[i].y2 = in->readSint16BE();
		if (i == 0) {
			brandonX = _characterList[i].x1;
			brandonY = _characterList[i].y1;
		}
		//_characterList[i].field_20 = in->readUint16BE();
		//_characterList[i].field_23 = in->readUint16BE();
	}

	_marbleVaseItem = in->readSint16BE();
	_itemInHand = in->readByte();
	
	for (int i = 0; i < 4; ++i) {
		_birthstoneGemTable[i] = in->readByte();
	}
	for (int i = 0; i < 3; ++i) {
		_idolGemsTable[i] = in->readByte();
	}
	for (int i = 0; i < 3; ++i) {
		_foyerItemTable[i] = in->readByte();
	}
	_cauldronState = in->readByte();
	for (int i = 0; i < 2; ++i) {
		_crystalState[i] = in->readByte();
	}
	
	_brandonStatusBit = in->readUint16BE();
	_brandonStatusBit0x02Flag = in->readByte();
	_brandonStatusBit0x20Flag = in->readByte();
	in->read(_brandonPoisonFlagsGFX, 256);
	_brandonInvFlag = in->readSint16BE();
	_poisonDeathCounter = in->readByte();
	_animator->_brandonDrawFrame = in->readUint16BE();

	for (int i = 0; i < 32; i++) {
		_timers[i].active = in->readByte();
		_timers[i].countdown = in->readSint32BE();
		_timers[i].nextRun = in->readUint32BE();
		if (_timers[i].nextRun != 0)
			_timers[i].nextRun += _system->getMillis();
	}
	_timerNextRun = 0;

	uint32 flagsSize = in->readUint32BE();
	assert(flagsSize == sizeof(_flagsTable));
	in->read(_flagsTable, flagsSize);

	for (int i = 0; i < _roomTableSize; ++i) {
		for (int item = 0; item < 12; ++item) {
			_roomTable[i].itemsTable[item] = 0xFF;
			_roomTable[i].itemsXPos[item] = 0xFFFF;
			_roomTable[i].itemsYPos[item] = 0xFF;
			_roomTable[i].needInit[item] = 0;
		}
	}

	uint16 sceneId = 0;

	while (true) {
		sceneId = in->readUint16BE();
		if (sceneId == 0xFFFF)
			break;
		assert(sceneId < _roomTableSize);
		_roomTable[sceneId].nameIndex = in->readByte();

		for (int i = 0; i < 12; i++) {
			_roomTable[sceneId].itemsTable[i] = in->readByte();
			_roomTable[sceneId].itemsXPos[i] = in->readUint16BE();
			_roomTable[sceneId].itemsYPos[i] = in->readUint16BE();
			_roomTable[sceneId].needInit[i] = in->readByte();
		}
	}
	if (version >= 3) {
		_lastMusicCommand = in->readSint16BE();
		if (_lastMusicCommand != -1)
			snd_playWanderScoreViaMap(_lastMusicCommand, 1);
	}
	
	if (version >= 4) {
		_configTextspeed = in->readByte();
		_configWalkspeed = in->readByte();
		_configMusic = in->readByte();
		_configSounds = in->readByte();
		_configVoice = in->readByte();
	}

	if (queryGameFlag(0x2D)) {
		loadMainScreen(8);
		loadBitmap("AMULET3.CPS", 10, 10, 0);
		if (!queryGameFlag(0xF1)) {
			for (int i = 0x55; i <= 0x5A; ++i) {
				if (queryGameFlag(i)) {
					seq_createAmuletJewel(i-0x55, 10, 1, 1);
				}
			}
		}
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 10, 8);
		uint8 *_pageSrc = _screen->getPagePtr(8);
		uint8 *_pageDst = _screen->getPagePtr(0);
		memcpy(_pageDst, _pageSrc, 320*200);
	} else {
		loadMainScreen(8);
	}
	
	createMouseItem(_itemInHand);
	_animator->setBrandonAnimSeqSize(3, 48);
	_animator->_noDrawShapesFlag = 1;
	enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);
	_animator->_noDrawShapesFlag = 0;
	
	_currentCharacter->x1 = brandonX;
	_currentCharacter->y1 = brandonY;
	_animator->animRefreshNPC(0);
	_animator->restoreAllObjectBackgrounds();
	_animator->preserveAnyChangedBackgrounds();
	_animator->prepDrawAllObjects();
	_animator->copyChangedObjectsForward(0);
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	redrawInventory(0);
	
	_abortWalkFlag = true;
	_abortWalkFlag2 = false;
	_mousePressFlag = false;
	_mouseX = brandonX;
	_mouseY = brandonY;
	_system->warpMouse(brandonX, brandonY);

	if (in->ioFailed())
		error("Load failed.");
	else
		debugC(1, kDebugLevelMain, "Loaded savegame '%s.'", saveName);

	delete in;
}

void KyraEngine::saveGame(const char *fileName, const char *saveName) {
	debugC(9, kDebugLevelMain, "saveGame('%s', '%s')", fileName, saveName);
	Common::OutSaveFile *out;

	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName);
		return;
	}

	// Savegame version
	out->writeUint32BE(MKID_BE('KYRA'));
	out->writeUint32BE(CURRENT_VERSION);
	out->write(saveName, 31);
	out->writeUint32BE(_features);

	for (int i = 0; i < 11; i++) {
		out->writeUint16BE(_characterList[i].sceneId);
		out->writeByte(_characterList[i].height);
		out->writeByte(_characterList[i].facing);
		out->writeUint16BE(_characterList[i].currentAnimFrame);
		//out->writeUint32BE(_characterList[i].unk6);
		out->write(_characterList[i].inventoryItems, 10);
		out->writeSint16BE(_characterList[i].x1);
		out->writeSint16BE(_characterList[i].y1);
		out->writeSint16BE(_characterList[i].x2);
		out->writeSint16BE(_characterList[i].y2);
		//out->writeUint16BE(_characterList[i].field_20);
		//out->writeUint16BE(_characterList[i].field_23);
	}
	
	out->writeSint16BE(_marbleVaseItem);
	out->writeByte(_itemInHand);
	
	for (int i = 0; i < 4; ++i) {
		out->writeByte(_birthstoneGemTable[i]);
	}
	for (int i = 0; i < 3; ++i) {
		out->writeByte(_idolGemsTable[i]);
	}
	for (int i = 0; i < 3; ++i) {
		out->writeByte(_foyerItemTable[i]);
	}
	out->writeByte(_cauldronState);
	for (int i = 0; i < 2; ++i) {
		out->writeByte(_crystalState[i]);
	}
	
	out->writeUint16BE(_brandonStatusBit);
	out->writeByte(_brandonStatusBit0x02Flag);
	out->writeByte(_brandonStatusBit0x20Flag);
	out->write(_brandonPoisonFlagsGFX, 256);
	out->writeSint16BE(_brandonInvFlag);
	out->writeByte(_poisonDeathCounter);
	out->writeUint16BE(_animator->_brandonDrawFrame);

	for (int i = 0; i < 32; i++) {
		out->writeByte(_timers[i].active);
		out->writeSint32BE(_timers[i].countdown);
		if (_system->getMillis() >= _timers[i].nextRun) {
			out->writeUint32BE(0);
		} else {
			out->writeUint32BE(_timers[i].nextRun - _system->getMillis());
		}
	}

	out->writeUint32BE(sizeof(_flagsTable));
	out->write(_flagsTable, sizeof(_flagsTable));

	for (uint16 i = 0; i < _roomTableSize; i++) {
		out->writeUint16BE(i);
		out->writeByte(_roomTable[i].nameIndex);
		for (int a = 0; a < 12; a++) {
			out->writeByte(_roomTable[i].itemsTable[a]);
			out->writeUint16BE(_roomTable[i].itemsXPos[a]);
			out->writeUint16BE(_roomTable[i].itemsYPos[a]);
			out->writeByte(_roomTable[i].needInit[a]);
		}
	}
	// room table terminator
	out->writeUint16BE(0xFFFF);
	
	out->writeSint16BE(_lastMusicCommand);

	out->writeByte(_configTextspeed);
	out->writeByte(_configWalkspeed);
	out->writeByte(_configMusic);
	out->writeByte(_configSounds);
	out->writeByte(_configVoice);

	out->flush();

	// check for errors
	if (out->ioFailed())
		warning("Can't write file '%s'. (Disk full?)", fileName);
	else
		debugC(1, kDebugLevelMain, "Saved game '%s.'", saveName);

	delete out;
}
} // end of namespace Kyra
