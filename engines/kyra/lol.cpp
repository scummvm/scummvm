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

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/util.h"

#include "common/endian.h"
#include "base/version.h"

namespace Kyra {

LoLEngine::LoLEngine(OSystem *system, const GameFlags &flags) : KyraEngine_v1(system, flags) {
	_screen = 0;

	switch (_flags.lang) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		_lang = 0;
		break;

	case Common::FR_FRA:
		_lang = 1;
		break;

	case Common::DE_DEU:
		_lang = 2;
		break;

	default:
		warning("unsupported language, switching back to English");
		_lang = 0;
		break;
	}

	_chargenWSA = 0;
	_lastUsedStringBuffer = 0;
	_landsFile = 0;
	_levelLangFile = 0;

	_lastMusicTrack = -1;
	_lastSfxTrack = -1;
	_curTlkFile = -1;

	memset(_moneyColumnHeight, 0, 5);
	_credits = 0;

	_itemsInPlay = 0;
	_itemProperties = 0;
	_itemInHand = 0;
	memset(_inventoryItemIndex, 0, 48);
	_inventoryCurItem = 0;
	_unkInventFlag = 0;

	_itemIconShapes = _itemShapes = _gameShapes = _thrownShapes = _iceShapes = _fireballShapes = 0;
	_levelShpList = _levelDatList = 0;
	_monsterShapes = _monsterPalettes = 0;
	_buf4 = 0;
	_gameShapeMap = 0;
	memset(_monsterUnk, 0, 3);

	_charSelection = -1;
	_characters = 0;
	_spellProperties = 0;
	_charFlagUnk = 0;
	_selectedSpell = 0;
	_updateCharNum = _updateCharV1 = _updateCharV2 = _updateCharV3 = _updateCharV4 = _updateCharV5 = _updateCharV6 = 0;
	_updateCharTime = _updatePortraitNext = 0;
	_lampStatusTimer = 0xffffffff;

	_weaponsDisabled = false;
	_unkDrawPortraitIndex = 0;
	_unkFlag = 0;
	_scriptBoolSkipExec = _boolScriptFuncDone = false;
	_unkScriptByte = 0;
	_currentDirection = 0;
	_currentBlock = 0;
	memset(_scriptExecutedFuncs, 0, 18 * sizeof(uint16));

	_wllVmpMap = _wllBuffer3 = _wllBuffer4 = _wllWallFlags = 0;
	_wllShapeMap = 0;
	_lvlShapeTop = _lvlShapeBottom = _lvlShapeLeftRight = 0;
	_levelBlockProperties = 0;
	_lvlBuffer = 0;
	_lvl415 = 0;
	_lvlBlockIndex = _lvlShapeIndex = 0;
	_unkDrawLevelBool = true;
	_vcnBlocks = 0;
	_vcnShift = 0;
	_vcnExpTable = 0;
	_vmpPtr = 0;
	_tlcTable2 = 0;
	_tlcTable1 = 0;
	_levelShapeProperties = 0;
	_levelShapes = 0;
	_blockDrawingBuffer = 0;
	_sceneWindowBuffer = 0;
	memset (_doorShapes, 0, 2 * sizeof(uint8*));

	_lampOilStatus = _brightness = _lampStatusUnk = 0;
	_tempBuffer5120 = 0;
	_tmpData136 = 0;
	_lvlBuffer = 0;
	_unkGameFlag = 0;

	_dscUnk1 = 0;
	_dscShapeIndex = 0;
	_dscOvlMap = 0;	
	_dscShapeScaleW = 0;
	_dscShapeScaleH = 0;
	_dscShapeX = 0;
	_dscShapeY = 0;
	_dscTileIndex = 0;	
	_dscUnk2 = 0;
	_dscDoorShpIndex = 0;
	_dscDim1 = 0;
	_dscDim2 = 0;
	_dscBlockMap = _dscDoor1 = _dscShapeOvlIndex = 0;
	_dscBlockIndex = 0;
	_dscDimMap = 0;
	_dscDoorMonsterX = _dscDoorMonsterY = 0;
	_dscDoor4 = 0;

	_ingameSoundList = 0;
	_ingameSoundIndex = 0;
	_ingameSoundListSize = 0;
	_musicTrackMap = 0;
	_curMusicTheme = -1;
	_curMusicFileExt = 0;
	_curMusicFileIndex = -1;

	_sceneDrawVar1 = _sceneDrawVar2 = _sceneDrawVar3 = _wllProcessFlag = 0;
	_unkCmzU1 = _unkCmzU2 = 0;
	_shpDmX = _shpDmY = _dmScaleW = _dmScaleH = 0;
}

LoLEngine::~LoLEngine() {
	setupPrologueData(false);

	delete[] _landsFile;
	delete[] _levelLangFile;

	delete _screen;
	delete _tim;

	delete[]  _itemsInPlay;
	delete[]  _itemProperties;

	delete[]  _characters;

	if (_itemIconShapes) {
		for (int i = 0; i < _numItemIconShapes; i++)
			delete[]  _itemIconShapes[i];
		delete[] _itemIconShapes;
	}
	if (_itemShapes) {
		for (int i = 0; i < _numItemShapes; i++)
			delete[]  _itemShapes[i];
		delete[] _itemShapes;
	}
	if (_gameShapes) {
		for (int i = 0; i < _numGameShapes; i++)
			delete[]  _gameShapes[i];
		delete[] _gameShapes;
	}
	if (_thrownShapes) {
		for (int i = 0; i < _numThrownShapes; i++)
			delete[]  _thrownShapes[i];
		delete[] _thrownShapes;
	}
	if (_iceShapes) {
		for (int i = 0; i < _numIceShapes; i++)
			delete[]  _iceShapes[i];
		delete[] _iceShapes;
	}
	if (_fireballShapes) {
		for (int i = 0; i < _numFireballShapes; i++)
			delete[]  _fireballShapes[i];
		delete[] _fireballShapes;
	}

	if (_monsterShapes) {
		for (int i = 0; i < 48; i++)
			delete[]  _monsterShapes[i];
		delete[] _monsterShapes;
	}
	if (_monsterPalettes) {
		for (int i = 0; i < 48; i++)
			delete[]  _monsterPalettes[i];
		delete[] _monsterPalettes;
	}
	if (_buf4) {
		for (int i = 0; i < 384; i++)
			delete[]  _buf4[i];
		delete[] _buf4;
	}

	for (Common::Array<const TIMOpcode*>::iterator i = _timIntroOpcodes.begin(); i != _timIntroOpcodes.end(); ++i)
		delete *i;
	_timIntroOpcodes.clear();

	delete[] _wllVmpMap;
	delete[] _wllShapeMap;
	delete[] _wllBuffer3;
	delete[] _wllBuffer4;
	delete[] _wllWallFlags;
	delete[] _lvlShapeTop;
	delete[] _lvlShapeBottom;
	delete[] _lvlShapeLeftRight;
	delete[] _tempBuffer5120;
	delete[] _tmpData136;
	delete[] _lvlBuffer;
	delete[] _levelBlockProperties;
	delete[] _lvl415;

	delete[] _levelFileData;
	delete[] _vcnExpTable;
	delete[] _vcnBlocks;
	delete[] _vcnShift;
	delete[] _vmpPtr;
	delete[] _tlcTable2;
	delete[] _tlcTable1;
	delete[] _levelShapeProperties;
	delete[] _blockDrawingBuffer;
	delete[] _sceneWindowBuffer;

	if (_levelShapes) {
		for (int i = 0; i < 400; i++)
			delete[]  _levelShapes[i];
		delete[] _levelShapes;
	}

	for (int i = 0; i < 2; i++)
		delete[] _doorShapes[i];
	
	delete _lvlShpFileHandle;

	if (_ingameSoundList) {
		for (int i = 0; i < _ingameSoundListSize; i++)
			delete[] _ingameSoundList[i];
		delete[] _ingameSoundList;	
	}
}

Screen *LoLEngine::screen() {
	return _screen;
}

Common::Error LoLEngine::init() {
	_screen = new Screen_LoL(this, _system);
	assert(_screen);
	_screen->setResolution();

	KyraEngine_v1::init();
	initStaticResource();

	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);

	_screen->setAnimBlockPtr(10000);
	_screen->setScreenDim(0);

	_itemsInPlay = new ItemInPlay[401];
	memset(_itemsInPlay, 0, sizeof(ItemInPlay) * 400);

	_characters = new LoLCharacter[4];
	memset(_characters, 0, sizeof(LoLCharacter) * 3);

	if (!_sound->init())
		error("Couldn't init sound");

	_unkAudioSpecOffs = 0x48;
	_unkLangAudio = _lang ? true : false;

	_wllVmpMap = new uint8[80];
	memset(_wllVmpMap, 0, 80);
	_wllShapeMap = new int8[80];
	memset(_wllShapeMap, 0, 80);
	_wllBuffer3 = new uint8[80];
	memset(_wllBuffer3, 0, 80);
	_wllBuffer4 = new uint8[80];
	memset(_wllBuffer4, 0, 80);
	_wllWallFlags = new uint8[80];
	memset(_wllWallFlags, 0, 80);
	_lvlShapeTop = new int16[18];
	memset(_lvlShapeTop, 0, 18 * sizeof(int16));
	_lvlShapeBottom = new int16[18];
	memset(_lvlShapeBottom, 0, 18 * sizeof(int16));
	_lvlShapeLeftRight = new int16[36];
	memset(_lvlShapeLeftRight, 0, 36 * sizeof(int16));
	_levelShapeProperties = new LevelShapeProperty[100];
	memset(_levelShapeProperties, 0, 100 * sizeof(LevelShapeProperty));
	_levelShapes = new uint8*[400];
	memset(_levelShapes, 0, 400 * sizeof(uint8*));
	_blockDrawingBuffer = new uint16[1320];
	memset(_blockDrawingBuffer, 0, 1320 * sizeof(uint16));
	_sceneWindowBuffer = new uint8[21120];
	memset(_sceneWindowBuffer, 0, 21120);

	_levelBlockProperties = new LevelBlockProperty[1025];
	memset(_levelBlockProperties, 0, 1025 * sizeof(LevelBlockProperty));
	_lvlBuffer = new LVL[30];
	memset(_lvlBuffer, 0, 30 * sizeof(LVL));
	_lvl415 = new uint8[415];
	memset(_lvl415, 0, 415);

	_vcnExpTable = new uint8[128];
	for (int i = 0; i < 128; i++)
		_vcnExpTable[i] = i & 0x0f;

	_tempBuffer5120 = new uint8[5120];
	memset(_tempBuffer5120, 0, 5120);

	_tmpData136 = new uint8[136];
	memset(_tmpData136, 0, 136);

	memset(_gameFlags, 0, 15 * sizeof(uint16));
	memset(_unkEMC46, 0, 16 * sizeof(uint16));	

	_levelFileData = 0;
	_lvlShpFileHandle = 0;

	_sceneDrawPage1 = 2;
	_sceneDrawPage2 = 6;

	_monsterShapes = new uint8*[48];
	memset(_monsterShapes, 0, 48 * sizeof(uint8*));
	_monsterPalettes = new uint8*[48];
	memset(_monsterPalettes, 0, 48 * sizeof(uint8*));

	_buf4 = new uint8*[384];
	memset(_buf4, 0, 384 * sizeof(uint8*));
	memset(&_scriptData, 0, sizeof(EMCData));
	
	_levelFlagUnk = 0;

	return Common::kNoError;
}

Common::Error LoLEngine::go() {
	if (!saveFileLoadable(0)) {
		setupPrologueData(true);
		showIntro();
		setupPrologueData(false);
	}

	preInit();

	int processSelection = -1;
	while (!shouldQuit() && processSelection == -1) {
		_screen->loadBitmap("TITLE.CPS", 2, 2, _screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

		_screen->setFont(Screen::FID_6_FNT);
		// Original version: (260|193) "V CD1.02 D"
		_screen->fprintString("SVM %s", 255, 193, 0x67, 0x00, 0x04, gScummVMVersion);
		_screen->setFont(Screen::FID_9_FNT);

		_screen->fadePalette(_screen->getPalette(0), 0x1E);
		_screen->updateScreen();

		_eventList.clear();
		int selection = mainMenu();
		_screen->hideMouse();

		// Unlike the original, we add a nice fade to black
		memset(_screen->getPalette(0), 0, 768);
		_screen->fadePalette(_screen->getPalette(0), 0x54);

		switch (selection) {
		case 0:		// New game
			processSelection = 0;
			break;

		case 1:		// Show intro
			setupPrologueData(true);
			showIntro();
			setupPrologueData(true);
			break;

		case 2:		// "Lore of the Lands" (only CD version)
			break;

		case 3:		// Load game
			// For now fall through
			//processSelection = 3;
			//break;

		case 4:		// Quit game
		default:
			quitGame();
			updateInput();
			break;
		}
	}

	if (processSelection == -1)
		return Common::kNoError;

	if (processSelection == 0) {
		setupPrologueData(true);
		_sound->loadSoundFile("LOREINTR");
		_sound->playTrack(6);
		/*int character = */chooseCharacter();
		_sound->playTrack(1);
		_screen->fadeToBlack();
		setupPrologueData(true);
	}

	if (!shouldQuit() && (processSelection == 0 || processSelection == 3))
		startup();

	if (!shouldQuit() && processSelection == 0)
		startupNew();

	if (!shouldQuit() && (processSelection == 0 || processSelection == 3))
		runLoop();

	return Common::kNoError;
}

#pragma mark - Initialization

void LoLEngine::preInit() {
	debugC(9, kDebugLevelMain, "LoLEngine::preInit()");

	if (!_res->loadFileList("FILEDATA.FDT"))
		error("Couldn't load file list: 'FILEDATA.FDT'");

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6P.FNT");

	uint8 *pal = _screen->getPalette(0);
	memset(pal, 0, 768);
	_screen->setScreenPalette(pal);

	// TODO: We need to check if the SYSEX events of intro and ingame differ.
	// If they differ, we really need to setup the proper ingame SYSEX when starting
	// the game. But the proper place to do it would not be in this function.
	/*if (_sound->getMusicType() == Sound::kMidiMT32 || _sound->getSfxType() == Sound::kMidiMT32) {
		_sound->loadSoundFile("LOLSYSEX");
		_sound->playTrack(0);

		while (_sound->isPlaying() && !shouldQuit())
			delay(10);
	}*/

	if (shouldQuit())
		return;

	_eventList.clear();

	loadTalkFile(0);

	char filename[32];
	snprintf(filename, sizeof(filename), "LANDS.%s", _languageExt[_lang]);
	_res->exists(filename, true);
	_landsFile = _res->fileData(filename, 0);

	initializeCursors();
}

void LoLEngine::initializeCursors() {
	debugC(9, kDebugLevelMain, "LoLEngine::initializeCursors()");

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	const uint8 *shp = _screen->getCPagePtr(3);
	_numItemIconShapes = READ_LE_UINT16(shp);
	_itemIconShapes = new uint8*[_numItemIconShapes];
	for (int i = 0; i < _numItemIconShapes; i++)
		_itemIconShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->setMouseCursor(0, 0, _itemIconShapes[0]);
}

void LoLEngine::setMouseCursorToIcon(int icon) {
	_screen->_drawGuiFlag |= 0x200;
	int i = _itemProperties[_itemsInPlay[_itemInHand].itemPropertyIndex].shpIndex;
	if (i == icon)
		return;
	_screen->setMouseCursor(0, 0, _itemIconShapes[icon]);
}

void LoLEngine::setMouseCursorToItemInHand() {
	_screen->_drawGuiFlag &= 0xFDFF;
	int o = (_itemInHand == 0) ? 0 : 10;
	_screen->setMouseCursor(o, o, getItemIconShapePtr(_itemInHand));
}

uint8 *LoLEngine::getItemIconShapePtr(int index) {
	int ix = _itemProperties[_itemsInPlay[index].itemPropertyIndex].shpIndex;
	if (_itemProperties[_itemsInPlay[index].itemPropertyIndex].flags & 0x200)
		ix += (_itemsInPlay[index].shpCurFrame_flg & 0x1fff) - 1;
	
	return _itemIconShapes[ix];
}

int LoLEngine::mainMenu() {
	debugC(9, kDebugLevelMain, "LoLEngine::mainMenu()");

	bool hasSave = saveFileLoadable(0);

	MainMenu::StaticData data = {
		{ 0, 0, 0, 0, 0 },
		{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0x3D, 0x9F },
		{ 0x2C, 0x19, 0x48, 0x2C },
		Screen::FID_9_FNT, 1
	};

	if (hasSave)
		++data.menuTable[3];

	static const uint16 mainMenuStrings[4][5] = {
		{ 0x4248, 0x4249, 0x42DD, 0x424A, 0x0000 },
		{ 0x4248, 0x4249, 0x42DD, 0x4001, 0x424A },
		{ 0x4248, 0x4249, 0x424A, 0x0000, 0x0000 },
		{ 0x4248, 0x4249, 0x4001, 0x424A, 0x0000 }
	};

	int tableOffs = _flags.isTalkie ? 0 : 2;

	for (int i = 0; i < 5; ++i) {
		if (hasSave)
			data.strings[i] = getLangString(mainMenuStrings[1 + tableOffs][i]);
		else
			data.strings[i] = getLangString(mainMenuStrings[tableOffs][i]);
	}

	MainMenu *menu = new MainMenu(this);
	assert(menu);
	menu->init(data, MainMenu::Animation());

	int selection = menu->handle(_flags.isTalkie ? (hasSave ? 12 : 6) : (hasSave ? 6 : 13));
	delete menu;
	_screen->setScreenDim(0);

	if (!_flags.isTalkie && selection >= 2)
		selection++;

	if (!hasSave && selection == 3)
		selection = 4;

	return selection;
}

void LoLEngine::startup() {
	_screen->clearPage(0);
	_screen->loadBitmap("PLAYFLD.CPS", 3, 3, _screen->_currentPalette);

	uint8 *tmpPal = new uint8[0x300];
	memcpy(tmpPal, _screen->_currentPalette, 0x300);
	memset(_screen->_currentPalette, 0x3f, 0x180);
	memcpy(_screen->_currentPalette + 3, tmpPal + 3, 3);
	memset(_screen->_currentPalette + 0x240, 0x3f, 12);
	_screen->generateOverlay(_screen->_currentPalette, _screen->_paletteOverlay1, 1, 6);
	_screen->generateOverlay(_screen->_currentPalette, _screen->_paletteOverlay2, 0x90, 0x41);
	memcpy(_screen->_currentPalette, tmpPal, 0x300);
	delete[] tmpPal;

	memset(_screen->getPalette(1), 0, 0x300);
	memset(_screen->getPalette(2), 0, 0x300);

	_screen->setMouseCursor(0, 0, _itemIconShapes[0x85]);

	_screen->loadBitmap("ITEMSHP.SHP", 3, 3, 0);
	const uint8 *shp = _screen->getCPagePtr(3);
	_numItemShapes = READ_LE_UINT16(shp);
	_itemShapes = new uint8*[_numItemShapes];
	for (int i = 0; i < _numItemShapes; i++)
		_itemShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("GAMESHP.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numGameShapes = READ_LE_UINT16(shp);
	_gameShapes = new uint8*[_numGameShapes];
	for (int i = 0; i < _numGameShapes; i++)
		_gameShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("THROWN.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numThrownShapes = READ_LE_UINT16(shp);
	_thrownShapes = new uint8*[_numThrownShapes];
	for (int i = 0; i < _numThrownShapes; i++)
		_thrownShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("ICE.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numIceShapes = READ_LE_UINT16(shp);
	_iceShapes = new uint8*[_numIceShapes];
	for (int i = 0; i < _numIceShapes; i++)
		_iceShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->loadBitmap("FIREBALL.SHP", 3, 3, 0);
	shp = _screen->getCPagePtr(3);
	_numFireballShapes = READ_LE_UINT16(shp);
	_fireballShapes = new uint8*[_numFireballShapes];
	for (int i = 0; i < _numFireballShapes; i++)
		_fireballShapes[i] = _screen->makeShapeCopy(shp, i);

	memset(_itemsInPlay, 0, 400 * sizeof(ItemInPlay));
	for (int i = 0; i < 400; i++)
		_itemsInPlay[i].shpCurFrame_flg |= 0x8000;

	runInitScript("ONETIME.INF", 0);
	_emc->load("ITEM.INF", &_itemScript, &_opcodes);

	_tlcTable1 = new uint8[256];
	_tlcTable2 = new uint8[5120];
	
	_loadSuppFilesFlag = 1;

	setMouseCursorToItemInHand();
}

void LoLEngine::startupNew() {
	_selectedSpell = 0;
	_updateUnk2 = _compassDirectionIndex = -1;
	/*
	_unk3 = -1;*/
	_unkGameFlag |= 0x1B;
	/*
	_unk5 = 1;
	_unk6 = 1;
	_unk7 = 1
	_unk8 = 1*/
	_currentLevel = 1;

	giveCredits(41, 0);
	_inventoryItemIndex[0] = makeItem(0xd8, 0, 0);
	_inventoryItemIndex[1] = makeItem(0xd9, 0, 0);
	_inventoryItemIndex[2] = makeItem(0xda, 0, 0);

	memset(_availableSpells, -1, 7);
	setupScreenDims();

	//memset(_unkWordArraySize8, 0x100, 8);

	static int selectIds[] = { -9, -1, -8, -5 };
	addCharacter(selectIds[_charSelection]);

	// TODO 

	loadLevel(1);

	_screen->showMouse();
}

void LoLEngine::runLoop() {
	_screen->updateScreen();

	bool _runFlag = true;
	while (!shouldQuit() && _runFlag) {
		checkInput(0, false);
		removeInputTop();
		_screen->updateScreen();
		_system->delayMillis(10);
	}
}

#pragma mark - Localization

const char *LoLEngine::getLangString(uint16 id) {
	debugC(9, kDebugLevelMain, "LoLEngine::getLangString(0x%.04X)", id);

	if (id == 0xFFFF)
		return 0;

	uint16 realId = id & 0x3FFF;
	uint8 *buffer = 0;

	if (id & 0x4000)
		buffer = _landsFile;
	else
		buffer = _levelLangFile;

	if (!buffer)
		return 0;

	const char *string = (const char *)getTableEntry(buffer, realId);

	char *srcBuffer = _stringBuffer[_lastUsedStringBuffer];
	Util::decodeString1(string, srcBuffer);
	Util::decodeString2(srcBuffer, srcBuffer);

	++_lastUsedStringBuffer;
	_lastUsedStringBuffer %= ARRAYSIZE(_stringBuffer);

	return srcBuffer;
}

uint8 *LoLEngine::getTableEntry(uint8 *buffer, uint16 id) {
	debugC(9, kDebugLevelMain, "LoLEngine::getTableEntry(%p, %d)", (const void *)buffer, id);
	if (!buffer)
		return 0;

	return buffer + READ_LE_UINT16(buffer + (id<<1));
}

#pragma mark - Intro

void LoLEngine::setupPrologueData(bool load) {
	debugC(9, kDebugLevelMain, "LoLEngine::setupPrologueData(%d)", load);

	static const char * const fileListCD[] = {
		"GENERAL.PAK", "INTROVOC.PAK", "STARTUP.PAK", "INTRO1.PAK",
		"INTRO2.PAK", "INTRO3.PAK", "INTRO4.PAK", "INTRO5.PAK",
		"INTRO6.PAK", "INTRO7.PAK", "INTRO8.PAK", "INTRO9.PAK", 0
	};

	static const char * const fileListFloppyExtracted[] = {
		"INTRO.PAK", "INTROVOC.PAK", 0
	};

	static const char * const fileListFloppy[] = {
		"INTRO.PAK", "INTROVOC.CMP", 0
	};

	const char * const *fileList = _flags.isTalkie ? fileListCD :
		(_flags.useInstallerPackage ? fileListFloppy : fileListFloppyExtracted);


	char filename[32];
	for (uint i = 0; fileList[i]; ++i) {
		filename[0] = '\0';

		if (_flags.isTalkie) {
			strcpy(filename, _languageExt[_lang]);
			strcat(filename, "/");
		}

		strcat(filename, fileList[i]);

		if (load) {
			if (!_res->loadPakFile(filename))
				error("Couldn't load file: '%s'", filename);
		} else {
			_res->unloadPakFile(filename);
		}
	}

	_screen->clearPage(0);
	_screen->clearPage(3);

	if (load) {
		_chargenWSA = new WSAMovie_v2(this, _screen);
		assert(_chargenWSA);

		//_charSelection = -1;
		_charSelectionInfoResult = -1;

		_selectionAnimFrames[0] = _selectionAnimFrames[2] = 0;
		_selectionAnimFrames[1] = _selectionAnimFrames[3] = 1;

		memset(_selectionAnimTimers, 0, sizeof(_selectionAnimTimers));
		memset(_screen->getPalette(1), 0, 768);
	} else {
		delete _chargenWSA; _chargenWSA = 0;
	}
}

void LoLEngine::showIntro() {
	debugC(9, kDebugLevelMain, "LoLEngine::showIntro()");

	TIM *intro = _tim->load("LOLINTRO.TIM", &_timIntroOpcodes);

	_screen->loadFont(Screen::FID_8_FNT, "NEW8P.FNT");
	_screen->loadFont(Screen::FID_INTRO_FNT, "INTRO.FNT");
	_screen->setFont(Screen::FID_8_FNT);

	_tim->resetFinishedFlag();
	_tim->setLangData("LOLINTRO.DIP");

	_screen->hideMouse();

	uint32 palNextFadeStep = 0;
	while (!_tim->finished() && !shouldQuit() && !skipFlag()) {
		updateInput();
		_tim->exec(intro, false);
		_screen->checkedPageUpdate(8, 4);

		if (_tim->_palDiff) {
			if (palNextFadeStep < _system->getMillis()) {
				_tim->_palDelayAcc += _tim->_palDelayInc;
				palNextFadeStep = _system->getMillis() + ((_tim->_palDelayAcc >> 8) * _tickLength);
				_tim->_palDelayAcc &= 0xFF;

				if (!_screen->fadePalStep(_screen->getPalette(0), _tim->_palDiff)) {
					_screen->setScreenPalette(_screen->getPalette(0));
					_tim->_palDiff = 0;
				}
			}
		}

		_system->delayMillis(10);
		_screen->updateScreen();
	}
	_screen->showMouse();
	_sound->voiceStop();
	_sound->beginFadeOut();

	_eventList.clear();

	_tim->unload(intro);
	_tim->clearLangData();

	_screen->fadePalette(_screen->getPalette(1), 30, 0);
}

int LoLEngine::chooseCharacter() {
	debugC(9, kDebugLevelMain, "LoLEngine::chooseCharacter()");

	_tim->setLangData("LOLINTRO.DIP");

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	_screen->setMouseCursor(0, 0, _screen->getPtrToShape(_screen->getCPagePtr(3), 0));

	while (!_screen->isMouseVisible())
		_screen->showMouse();

	_screen->loadBitmap("CHAR.CPS", 2, 2, _screen->getPalette(0));
	_screen->loadBitmap("BACKGRND.CPS", 4, 4, _screen->getPalette(0));

	if (!_chargenWSA->open("CHARGEN.WSA", 1, 0))
		error("Couldn't load CHARGEN.WSA");
	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(2);
	_chargenWSA->displayFrame(0, 0, 0, 0);

	_screen->setFont(Screen::FID_9_FNT);
	_screen->_curPage = 2;

	for (int i = 0; i < 4; ++i)
		_screen->fprintStringIntro(_charPreviews[i].name, _charPreviews[i].x + 16, _charPreviews[i].y + 36, 0xC0, 0x00, 0x9C, 0x120);

	for (int i = 0; i < 4; ++i) {
		_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 48, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[0]);
		_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 56, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[1]);
		_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 64, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[2]);
	}

	_screen->fprintStringIntro(_tim->getCTableEntry(51), 36, 173, 0x98, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(53), 36, 181, 0x98, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(55), 36, 189, 0x98, 0x00, 0x9C, 0x20);

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->_curPage = 0;

	_screen->fadePalette(_screen->getPalette(0), 30, 0);

	bool kingIntro = true;
	while (!shouldQuit()) {
		if (kingIntro)
			kingSelectionIntro();

		if (_charSelection < 0)
			processCharacterSelection();

		if (shouldQuit())
			break;

		if (_charSelection == 100) {
			kingIntro = true;
			_charSelection = -1;
			continue;
		}

		_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_screen->showMouse();

		if (selectionCharInfo(_charSelection) == -1) {
			_charSelection = -1;
			kingIntro = false;
		} else {
			break;
		}
	}

	if (shouldQuit())
		return -1;

	uint32 waitTime = _system->getMillis() + 420 * _tickLength;
	while (waitTime > _system->getMillis() && !skipFlag() && !shouldQuit()) {
		updateInput();
		_system->delayMillis(10);
	}

	// HACK: Remove all input events
	_eventList.clear();

	_tim->clearLangData();

	return _charSelection;
}

void LoLEngine::kingSelectionIntro() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionIntro()");

	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
	int y = 38;

	_screen->fprintStringIntro(_tim->getCTableEntry(57), 8, y, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(58), 8, y + 10, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(59), 8, y + 20, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(60), 8, y + 30, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(61), 8, y + 40, 0x32, 0x00, 0x9C, 0x20);

	_sound->voicePlay("KING01");

	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(0);

	int index = 4;
	while (_sound->voiceIsPlaying("KING01") && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 0, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar1IdxTable[index]*2+0], _selectionPosTable[_selectionChar1IdxTable[index]*2+1], _charPreviews[0].x, _charPreviews[0].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar2IdxTable[index]*2+0], _selectionPosTable[_selectionChar2IdxTable[index]*2+1], _charPreviews[1].x, _charPreviews[1].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar3IdxTable[index]*2+0], _selectionPosTable[_selectionChar3IdxTable[index]*2+1], _charPreviews[2].x, _charPreviews[2].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar4IdxTable[index]*2+0], _selectionPosTable[_selectionChar4IdxTable[index]*2+1], _charPreviews[3].x, _charPreviews[3].y, 32, 32, 4, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 7 * _tickLength;
		while (waitEnd > _system->getMillis() && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		index = (index + 1) % 22;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0, 0, 0);
	_screen->updateScreen();
	_sound->voiceStop("KING01");
}

void LoLEngine::kingSelectionReminder() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionReminder()");

	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
	int y = 48;

	_screen->fprintStringIntro(_tim->getCTableEntry(62), 8, y, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(63), 8, y + 10, 0x32, 0x00, 0x9C, 0x20);

	_sound->voicePlay("KING02");

	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(0);

	int index = 0;
	while (_sound->voiceIsPlaying("KING02") && _charSelection == -1 && !shouldQuit() && index < 15) {
		_chargenWSA->displayFrame(_chargenFrameTable[index+9], 0, 0, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar1IdxTable[index]*2+0], _selectionPosTable[_reminderChar1IdxTable[index]*2+1], _charPreviews[0].x, _charPreviews[0].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar2IdxTable[index]*2+0], _selectionPosTable[_reminderChar2IdxTable[index]*2+1], _charPreviews[1].x, _charPreviews[1].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar3IdxTable[index]*2+0], _selectionPosTable[_reminderChar3IdxTable[index]*2+1], _charPreviews[2].x, _charPreviews[2].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar4IdxTable[index]*2+0], _selectionPosTable[_reminderChar4IdxTable[index]*2+1], _charPreviews[3].x, _charPreviews[3].y, 32, 32, 4, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit()) {
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		index = (index + 1) % 22;
	}

	_sound->voiceStop("KING02");
}

void LoLEngine::kingSelectionOutro() {
	debugC(9, kDebugLevelMain, "LoLEngine::kingSelectionOutro()");

	_sound->voicePlay("KING03");

	_chargenWSA->setX(113);
	_chargenWSA->setY(0);
	_chargenWSA->setDrawPage(0);

	int index = 0;
	while (_sound->voiceIsPlaying("KING03") && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 0, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit() && !skipFlag()) {
			updateInput();
			_system->delayMillis(10);
		}

		index = (index + 1) % 22;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0, 0, 0);
	_screen->updateScreen();
	_sound->voiceStop("KING03");
}

void LoLEngine::processCharacterSelection() {
	debugC(9, kDebugLevelMain, "LoLEngine::processCharacterSelection()");

	_charSelection = -1;
	while (!shouldQuit() && _charSelection == -1) {
		uint32 nextKingMessage = _system->getMillis() + 900 * _tickLength;

		while (nextKingMessage > _system->getMillis() && _charSelection == -1 && !shouldQuit()) {
			updateSelectionAnims();
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		if (_charSelection == -1)
			kingSelectionReminder();
	}
}

void LoLEngine::updateSelectionAnims() {
	debugC(9, kDebugLevelMain, "LoLEngine::updateSelectionAnims()");

	for (int i = 0; i < 4; ++i) {
		if (_system->getMillis() < _selectionAnimTimers[i])
			continue;

		const int index = _selectionAnimIndexTable[_selectionAnimFrames[i] + i * 2];
		_screen->copyRegion(_selectionPosTable[index*2+0], _selectionPosTable[index*2+1], _charPreviews[i].x, _charPreviews[i].y, 32, 32, 4, 0);

		int delayTime = 0;
		if (_selectionAnimFrames[i] == 1)
			delayTime = _rnd.getRandomNumberRng(0, 31) + 80;
		else
			delayTime = _rnd.getRandomNumberRng(0, 3) + 10;

		_selectionAnimTimers[i] = _system->getMillis() + delayTime * _tickLength;
		_selectionAnimFrames[i] = (_selectionAnimFrames[i] + 1) % 2;
	}

	_screen->updateScreen();
}

int LoLEngine::selectionCharInfo(int character) {
	debugC(9, kDebugLevelMain, "LoLEngine::selectionCharInfo(%d)", character);
	if (character < 0)
		return -1;

	char filename[16];
	char vocFilename[6];
	strcpy(vocFilename, "000X0");

	switch (character) {
	case 0:
		strcpy(filename, "FACE09.SHP");
		vocFilename[3] = 'A';
		break;

	case 1:
		strcpy(filename, "FACE01.SHP");
		vocFilename[3] = 'M';
		break;

	case 2:
		strcpy(filename, "FACE08.SHP");
		vocFilename[3] = 'K';
		break;

	case 3:
		strcpy(filename, "FACE05.SHP");
		vocFilename[3] = 'C';
		break;

	default:
		break;
	};

	_screen->loadBitmap(filename, 9, 9, 0);
	_screen->copyRegion(0, 122, 0, 122, 320, 78, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(_charPreviews[character].x - 3, _charPreviews[character].y - 3, 8, 127, 38, 38, 2, 0);

	static const uint8 charSelectInfoIdx[] = { 0x1D, 0x22, 0x27, 0x2C };
	const int idx = charSelectInfoIdx[character];

	_screen->fprintStringIntro(_tim->getCTableEntry(idx+0), 50, 127, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+1), 50, 137, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+2), 50, 147, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+3), 50, 157, 0x53, 0x00, 0xCF, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(idx+4), 50, 167, 0x53, 0x00, 0xCF, 0x20);

	_screen->fprintStringIntro(_tim->getCTableEntry(69), 100, 168, 0x32, 0x00, 0xCF, 0x20);

	selectionCharInfoIntro(vocFilename);
	if (_charSelectionInfoResult == -1) {
		while (_charSelectionInfoResult == -1) {
			_charSelectionInfoResult = selectionCharAccept();
			_system->delayMillis(10);
		}
	}

	if (_charSelectionInfoResult != 1) {
		_charSelectionInfoResult = -1;
		_screen->copyRegion(0, 122, 0, 122, 320, 78, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		return -1;
	}

	_screen->copyRegion(48, 127, 48, 127, 272, 60, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->hideMouse();
	_screen->copyRegion(48, 127, 48, 160, 272, 35, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);

	_screen->fprintStringIntro(_tim->getCTableEntry(64), 3, 28, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(65), 3, 38, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(66), 3, 48, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(67), 3, 58, 0x32, 0x00, 0x9C, 0x20);
	_screen->fprintStringIntro(_tim->getCTableEntry(68), 3, 68, 0x32, 0x00, 0x9C, 0x20);

	resetSkipFlag();
	kingSelectionOutro();
	return character;
}

void LoLEngine::selectionCharInfoIntro(char *file) {
	debugC(9, kDebugLevelMain, "LoLEngine::selectionCharInfoIntro(%p)", (const void *)file);
	int index = 0;
	file[4] = '0';

	while (_charSelectionInfoResult == -1 && !shouldQuit()) {
		if (!_sound->voicePlay(file))
			break;

		int i = 0;
		while (_sound->voiceIsPlaying(file) && _charSelectionInfoResult == -1 && !shouldQuit()) {
			_screen->drawShape(0, _screen->getPtrToShape(_screen->getCPagePtr(9), _charInfoFrameTable[i]), 11, 130, 0, 0);
			_screen->updateScreen();

			uint32 nextFrame = _system->getMillis() + 8 * _tickLength;
			while (nextFrame > _system->getMillis() && _charSelectionInfoResult == -1) {
				_charSelectionInfoResult = selectionCharAccept();
				_system->delayMillis(10);
			}

			i = (i + 1) % 32;
		}

		_sound->voiceStop(file);
		file[4] = ++index + '0';
	}

	_screen->drawShape(0, _screen->getPtrToShape(_screen->getCPagePtr(9), 0), 11, 130, 0, 0);
	_screen->updateScreen();
}

int LoLEngine::getCharSelection() {
	int inputFlag = checkInput(0, false) & 0xCF;
	removeInputTop();

	if (inputFlag == 200) {
		for (int i = 0; i < 4; ++i) {
			if (_charPreviews[i].x <= _mouseX && _mouseX <= _charPreviews[i].x + 31 &&
				_charPreviews[i].y <= _mouseY && _mouseY <= _charPreviews[i].y + 31)
				return i;
		}
	}

	return -1;
}

int LoLEngine::selectionCharAccept() {
	int inputFlag = checkInput(0, false) & 0xCF;
	removeInputTop();

	if (inputFlag == 200) {
		if (88 <= _mouseX && _mouseX <= 128 && 180 <= _mouseY && _mouseY <= 194)
			return 1;
		if (196 <= _mouseX && _mouseX <= 236 && 180 <= _mouseY && _mouseY <= 194)
			return 0;
	}

	return -1;
}

bool LoLEngine::addCharacter(int id) {
	int numChars = countActiveCharacters();
	if (numChars == 4)
		return false;

	int i = 0;
	for (; i < _charDefaultsSize; i++) {
		if (_charDefaults[i].id == id) {
			memcpy(&_characters[numChars], &_charDefaults[i], sizeof(LoLCharacter));
			break;
		}
	}
	if (i == _charDefaultsSize)
		return false;

	loadCharFaceShapes(numChars, id);

	_characters[numChars].rand = _rnd.getRandomNumberRng(1, 12);

	i = 0;
	for (; i < 11; i++) {
		uint16 *tmp = &_characters[numChars].items[i];
		if (*tmp) {
			*tmp = makeItem(*tmp, 0, 0);
			runItemScript(numChars, *tmp, 0x80, 0, 0);
		}
	}

	calcCharPortraitXpos();
	if (numChars > 0)
		initCharacter(numChars, 2, 6, 0);

	return true;
}

void LoLEngine::initCharacter(int charNum, int firstFaceFrame, int unk2, int redraw) {
	_characters[charNum].nextFaceFrame = firstFaceFrame;
	if (firstFaceFrame || unk2)
		initCharacterUnkSub(charNum, 6, unk2, 1);
	if (redraw)
		gui_drawCharPortraitWithStats(charNum);
}

void LoLEngine::initCharacterUnkSub(int charNum, int unk1, int unk2, int unk3) {
	for (int i = 0; i < 5; i++) {
		if (_characters[charNum].arrayUnk1[i] == 0 || (unk3 && _characters[charNum].arrayUnk1[i] == unk1)) {
			_characters[charNum].arrayUnk1[i] = unk1;
			_characters[charNum].arrayUnk2[i] = unk2;

			// TODO

			break;
		}
	}
}

int LoLEngine::countActiveCharacters() {
	int i = 0;
	while (_characters[i].flags & 1)
		i++;
	return i;
}

void LoLEngine::loadCharFaceShapes(int charNum, int id) {
	if (id < 0)
		id = -id;

	char file[13];
	snprintf(file, sizeof(file), "FACE%02d.SHP", id);
	_screen->loadBitmap(file, 3, 3, 0);

	const uint8 *p = _screen->getCPagePtr(3);
	for (int i = 0; i < 40; i++)
		_characterFaceShapes[i][charNum] = _screen->makeShapeCopy(p, i);
}

void LoLEngine::updatePortraitWithStats() {
	int x = 0;
	int y = 0;
	bool redraw = false;

	if (_updateCharV2 == 0) {
		x = _activeCharsXpos[_updateCharNum];
		y = 144;
		redraw = true;
	} else if (_updateCharV2 == 1) {
		if (_unkLangAudio) {
			x = 90;
			y = 130;
		} else {
			x = _activeCharsXpos[_updateCharNum];
			y = 144;
		}
	} else if (_updateCharV2 == 2) {
		if (_unkLangAudio) {
			x = 16;
			y = 134;
		} else {
			x = _activeCharsXpos[_updateCharNum] + 10;
			y = 145;
		}
	}

	int f = _rnd.getRandomNumberRng(1, 6) - 1;
	if (f == _characters[_updateCharNum].curFaceFrame)
		f++;
	if (f > 5)
		f -= 5;
	f += 7;

	if (_unkAudioSpecOffs) {
		//TODO
		//if (unk() == 2)
		//	_updateCharV1 = 2;
		//else
			_updateCharV1 = 1;
	}

	if (--_updateCharV1) {
		setCharFaceFrame(_updateCharNum, f);
		if (redraw)
			gui_drawCharPortraitWithStats(_updateCharNum);
		else
			gui_drawCharFaceShape(_updateCharNum, x, y, 0);
		_updatePortraitNext = _system->getMillis() + 10 * _tickLength;
	} else if (_updateCharV1 == 0 && _updateCharV3 != 0) {
		faceFrameRefresh(_updateCharNum);
		if (redraw) {
			gui_drawCharPortraitWithStats(_updateCharNum);
			updatePortraitUnkTimeSub(0, 0);
		} else {
			gui_drawCharFaceShape(_updateCharNum, x, y, 0);
		}
		_updateCharNum = -1;
	}
}

void LoLEngine::updatePortraits() {
	if (_updateCharNum == -1)
		return;

	_updateCharV1 = _updateCharV3 = 1;
	updatePortraitWithStats();
	_updateCharV1 = 1;
	_updateCharNum = -1;

	if (!_updateCharV2)
		updatePortraitUnkTimeSub(0, 0);
}

void LoLEngine::updatePortraitUnkTimeSub(int unk1, int unk2) {
	if (_updateCharV4 == unk1 || !unk1) {
		_updateCharV5 = 1;
		_updateCharTime = _system->getMillis();
	}

	if (!unk2)
		return;

	updatePortraits();
	if (_updateCharV6) {
		_screen->hideMouse();
		_screen->clearDim(3);
		_screen->showMouse();
	}
	
	_updateCharV5 = 0;
	//initGuiUnk(11);
}

void LoLEngine::setCharFaceFrame(int charNum, int frameNum) {
	_characters[charNum].curFaceFrame = frameNum;
}

void LoLEngine::faceFrameRefresh(int charNum) {
	if (_characters[charNum].curFaceFrame == 1)
		initCharacter(charNum, 0, 0, 0);
	else if (_characters[charNum].curFaceFrame == 6)
		if (_characters[charNum].nextFaceFrame != 5)
			initCharacter(charNum, 0, 0, 0);
		else
			_characters[charNum].curFaceFrame = 5;
	else
		_characters[charNum].curFaceFrame = 0;
}

void LoLEngine::setupScreenDims() {
	if (_unkLangAudio)
		_screen->modifyScreenDim(4, 11, 124, 28, 45);
	else
		_screen->modifyScreenDim(4, 11, 124, 28, 9);
	_screen->modifyScreenDim(5, 85, 123, 233, 18);
}

void LoLEngine::loadTalkFile(int index) {
	char file[8];
	
	if (index == _curTlkFile)
		return;

	if (_curTlkFile >= 0) {
		snprintf(file, sizeof(file), "%02d.TLK", _curTlkFile);
		_res->unloadPakFile(file);
	}

	snprintf(file, sizeof(file), "%02d.TLK", index);
	_res->loadPakFile(file);
	
	_curTlkFile = index;
}

void LoLEngine::snd_playVoiceFile(int) {

}

void LoLEngine::snd_playSoundEffect(int track, int volume) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "LoLEngine::snd_playSoundEffect(%d, %d)", track, volume);

	if (track == 1 && (_lastSfxTrack == -1 || _lastSfxTrack == 1))
		return;

	_lastSfxTrack = track;

	int16 volIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2 + 1]);

	if (volIndex > 0)
		volIndex = (volIndex * volume) >> 8;
	else
		volIndex *= -1;

	// volume TODO

	int16 vocIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2]);
	if (vocIndex != -1) {
		_sound->voicePlay(_ingameSoundList[vocIndex], true);
	} else if (_flags.platform == Common::kPlatformPC) {
		if (_sound->getSfxType() == Sound::kMidiMT32)
			track = track < _ingameMT32SoundIndexSize ? _ingameMT32SoundIndex[track] - 1 : -1;
		else if (_sound->getSfxType() == Sound::kMidiGM)
			track = track < _ingameGMSoundIndexSize ? _ingameGMSoundIndex[track] - 1: -1;

		if (track != -1)
			KyraEngine_v1::snd_playSoundEffect(track);
	}
}

void LoLEngine::snd_loadSoundFile(int track) {
	if (_unkGameFlag & 2) {
		char filename[13];
		int t = (track - 250) * 3;

		if (_curMusicFileIndex != _musicTrackMap[t] || _curMusicFileExt != (char)_musicTrackMap[t + 1]) {
			snd_stopMusic();
			snprintf(filename, sizeof(filename), "LORE%02d%c", _musicTrackMap[t], (char)_musicTrackMap[t + 1]);
			_sound->loadSoundFile(filename);
			_curMusicFileIndex = _musicTrackMap[t];
			_curMusicFileExt = (char)_musicTrackMap[t + 1];
		} else {
			snd_stopMusic();
		}
	} else {
		//XXX
	}
}

int LoLEngine::snd_playTrack(int track) {
	if (track == -1)
		return _lastMusicTrack;
	
	int res = _lastMusicTrack;
	_lastMusicTrack = track;

	if (_unkGameFlag & 2) {
		snd_loadSoundFile(track);
		int t = (track - 250) * 3;
		_sound->playTrack(_musicTrackMap[t + 2]);
	}

	return res;
}

int LoLEngine::snd_stopMusic() {
	if (_unkGameFlag & 2) {
		if (_sound->isPlaying()) {
			_sound->beginFadeOut();
			_system->delayMillis(3 * _tickLength);
		}

		_sound->haltTrack();
	}
	return snd_playTrack(-1);
}

} // end of namespace Kyra

