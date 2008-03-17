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

#include "common/endian.h"
#include "common/savefile.h"
#include "common/system.h"

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/timer.h"

namespace Kyra {

void KyraEngine_v2::saveGame(const char *fileName, const char *saveName) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::saveGame('%s', '%s')", fileName, saveName);

	Common::OutSaveFile *out = openSaveForWriting(fileName, saveName);
	if (!out)
		return;

	_timer->saveDataToFile(out);

	out->writeUint32BE(sizeof(_flagsTable));
	out->write(_flagsTable, sizeof(_flagsTable));

	// usually we have to save the flag set by opcode 10 here
	//out->writeUint16BE(word_2AB05);
	out->writeSint16BE(_lastMusicCommand);
	out->writeByte(_newChapterFile);
	out->writeByte(_loadedZTable);
	out->writeByte(_cauldronState);
	out->writeByte(_colorCodeFlag1);
	out->writeByte(_colorCodeFlag2);
	out->writeByte(_bookCurPage);
	out->writeByte(_bookMaxPage);
	for (int i = 0; i < 7; ++i)
		out->writeByte(_presetColorCode[i]);
	for (int i = 0; i < 7; ++i)
		out->writeByte(_inputColorCode[i]);
	for (int i = 0; i < 25; ++i)
		out->writeSint16BE(_cauldronTable[i]);
	for (int i = 0; i < 20; ++i)
		out->writeUint16BE(_hiddenItems[i]);
	for (int i = 0; i < 19; ++i)
		out->write(_conversationState[i], 14);
	out->write(_newSceneDlgState, 32);
	out->writeSint16BE(_cauldronUseCount);

	out->writeUint16BE(_mainCharacter.sceneId);
	out->writeUint16BE(_mainCharacter.dlgIndex);
	out->writeByte(_mainCharacter.height);
	out->writeByte(_mainCharacter.facing);
	out->writeUint16BE(_mainCharacter.animFrame);
	out->writeByte(_mainCharacter.unk8);
	out->writeByte(_mainCharacter.unk9);
	out->writeByte(_mainCharacter.unkA);
	for (int i = 0; i < 20; ++i)
		out->writeUint16BE(_mainCharacter.inventory[i]);
	out->writeSint16BE(_mainCharacter.x1);
	out->writeSint16BE(_mainCharacter.y1);
	out->writeSint16BE(_mainCharacter.x2);
	out->writeSint16BE(_mainCharacter.y2);

	for (int i = 0; i < 30; ++i) {
		out->writeUint16BE(_itemList[i].id);
		out->writeUint16BE(_itemList[i].sceneId);
		out->writeSint16BE(_itemList[i].x);
		out->writeByte(_itemList[i].y);
		out->writeUint16BE(_itemList[i].unk7);
	}

	for (int i = 0; i < 72; ++i) {
		out->write(_talkObjectList[i].filename, 13);
		out->writeByte(_talkObjectList[i].scriptId);
		out->writeSint16BE(_talkObjectList[i].x);
		out->writeSint16BE(_talkObjectList[i].y);
		out->writeByte(_talkObjectList[i].color);
	}

	for (int i = 0; i < 86; ++i) {
		out->write(_sceneList[i].filename, 10);
		out->writeUint16BE(_sceneList[i].exit1);
		out->writeUint16BE(_sceneList[i].exit2);
		out->writeUint16BE(_sceneList[i].exit3);
		out->writeUint16BE(_sceneList[i].exit4);
		out->writeByte(_sceneList[i].flags);
		out->writeByte(_sceneList[i].sound);
	}

	out->writeSint16BE(_itemInHand);
	out->writeUint16BE(_sceneExit1);
	out->writeUint16BE(_sceneExit2);
	out->writeUint16BE(_sceneExit3);
	out->writeUint16BE(_sceneExit4);

	out->finalize();

	// check for errors
	if (out->ioFailed())
		warning("Can't write file '%s'. (Disk full?)", fileName);
	else
		debugC(1, kDebugLevelMain, "Saved game '%s.'", saveName);

	delete out;
}

void KyraEngine_v2::loadGame(const char *fileName) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::loadGame('%s')", fileName);

	uint32 version = 0;
	char saveName[31];
	Common::InSaveFile *in = openSaveForReading(fileName, version, saveName);
	if (!in)
		return;

	bool setFlag1EE = (queryGameFlag(0x1EE) != 0);
	
	_deathHandler = -1;
	if (!_unkSceneScreenFlag1) {
		_sound->beginFadeOut();
		_system->delayMillis(5 * _tickLength);
		_lastMusicCommand = -1;
	}

	int loadedZTable = _loadedZTable;

	_screen->hideMouse();

	_timer->loadDataFromFile(in, version);

	uint32 flagsSize = in->readUint32BE();
	assert(flagsSize <= sizeof(_flagsTable));
	in->read(_flagsTable, flagsSize);

	// usually we have to save the flag set by opcode 10 here
	//word_2AB05 = in->readUint16BE();
	_lastMusicCommand = in->readSint16BE();
	_newChapterFile = in->readByte();
	_loadedZTable = in->readByte();
	_cauldronState = in->readByte();
	_colorCodeFlag1 = in->readByte();
	_colorCodeFlag2 = in->readByte();
	_bookCurPage = in->readByte();
	_bookMaxPage = in->readByte();
	for (int i = 0; i < 7; ++i)
		_presetColorCode[i] = in->readByte();
	for (int i = 0; i < 7; ++i)
		_inputColorCode[i] = in->readByte();
	for (int i = 0; i < 25; ++i)
		_cauldronTable[i] = in->readSint16BE();
	for (int i = 0; i < 20; ++i)
		_hiddenItems[i] = in->readUint16BE();
	for (int i = 0; i < 19; ++i)
		in->read(_conversationState[i], 14);
	in->read(_newSceneDlgState, 32);
	_cauldronUseCount = in->readSint16BE();

	_mainCharacter.sceneId = in->readUint16BE();
	_mainCharacter.dlgIndex = in->readUint16BE();
	_mainCharacter.height = in->readByte();
	_mainCharacter.facing = in->readByte();
	_mainCharacter.animFrame = in->readUint16BE();
	_mainCharacter.unk8 = in->readByte();
	_mainCharacter.unk9 = in->readByte();
	_mainCharacter.unkA = in->readByte();
	for (int i = 0; i < 20; ++i)
		_mainCharacter.inventory[i] = in->readUint16BE();
	_mainCharacter.x1 = in->readSint16BE();
	_mainCharacter.y1 = in->readSint16BE();
	_mainCharacter.x2 = in->readSint16BE();
	_mainCharacter.y2 = in->readSint16BE();

	for (int i = 0; i < 30; ++i) {
		_itemList[i].id = in->readUint16BE();
		_itemList[i].sceneId = in->readUint16BE();
		_itemList[i].x = in->readSint16BE();
		_itemList[i].y = in->readByte();
		_itemList[i].unk7 = in->readUint16BE();
	}

	for (int i = 0; i < 72; ++i) {
		in->read(_talkObjectList[i].filename, 13);
		_talkObjectList[i].scriptId = in->readByte();
		_talkObjectList[i].x = in->readSint16BE();
		_talkObjectList[i].y = in->readSint16BE();
		_talkObjectList[i].color = in->readByte();
	}

	for (int i = 0; i < 86; ++i) {
		in->read(_sceneList[i].filename, 10);
		_sceneList[i].exit1 = in->readUint16BE();
		_sceneList[i].exit2 = in->readUint16BE();
		_sceneList[i].exit3 = in->readUint16BE();
		_sceneList[i].exit4 = in->readUint16BE();
		_sceneList[i].flags = in->readByte();
		_sceneList[i].sound = in->readByte();
	}

	_itemInHand = in->readSint16BE();
	_sceneExit1 = in->readUint16BE();
	_sceneExit2 = in->readUint16BE();
	_sceneExit3 = in->readUint16BE();
	_sceneExit4 = in->readUint16BE();

	if (in->ioFailed())
		error("Load failed ('%s', '%s').", fileName, saveName);
	else
		debugC(1, kDebugLevelMain, "Loaded savegame '%s.'", saveName);

	delete in;

	if (loadedZTable != _loadedZTable)
		loadZShapes(_loadedZTable);

	_screen->loadBitmap("_PLAYALL.CPS", 3, 3, 0);
	if (queryGameFlag(1))
		_screen->copyRegion(0xCE, 0x90, 0xCE, 0x90, 0x2C, 0x2C, 2, 0, Screen::CR_NO_P_CHECK);
	if (queryGameFlag(2))
		_screen->copyRegion(0xFA, 0x90, 0xFA, 0x90, 0x46, 0x2C, 2, 0, Screen::CR_NO_P_CHECK);

	redrawInventory(0);
	int cauldronUseCount = _cauldronUseCount;
	setCauldronState(_cauldronState, 0);
	_cauldronUseCount = cauldronUseCount;
	_mainCharX = _mainCharacter.x2 = _mainCharacter.x1;
	_mainCharY = _mainCharacter.y2 = _mainCharacter.y1;
	_mainCharacter.facing = 4;
	
	enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
	setHandItem(_itemInHand);

	if (_lastMusicCommand >= 0 && !_unkSceneScreenFlag1)
		snd_playWanderScoreViaMap(_lastMusicCommand, 1);

	_screen->showMouse();

	setTimer1DelaySecs(7);
	_shownMessage = " ";
	_msgUnk1 = 0;

	if (setFlag1EE)
		setGameFlag(0x1EE);
}

} // end of namespace Kyra
