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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"

#include "kyra/sound.h"
#include "kyra/timer.h"
#include "kyra/util.h"

#include "sound/voc.h"
#include "sound/audiostream.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "base/version.h"

namespace Kyra {

LoLEngine::LoLEngine(OSystem *system, const GameFlags &flags) : KyraEngine_v1(system, flags) {
	_screen = 0;
	_gui = 0;
	_txt = 0;
	_tim = 0;

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
	_lastSpeaker = _lastSpeechId = _nextSpeechId = _nextSpeaker = -1;

	memset(_moneyColumnHeight, 0, 5);
	_credits = 0;

	_itemsInPlay = 0;
	_itemProperties = 0;
	_itemInHand = 0;
	memset(_inventory, 0, sizeof(_inventory));
	_inventoryCurItem = 0;
	_currentControlMode = 0;
	_specialSceneFlag = 0;
	_lastCharInventory = -1;

	_itemIconShapes = _itemShapes = _gameShapes = _thrownShapes = _effectShapes = _fireballShapes = 0;
	_levelShpList = _levelDatList = 0;
	_monsterShapes = _monsterPalettes = 0;
	_monsterShapesEx = 0;
	_gameShapeMap = 0;
	memset(_monsterUnk, 0, 3);
	_pageSavedFlag = false;

	_ingameMT32SoundIndex = _ingameGMSoundIndex = /*_ingameADLSoundIndex =*/ 0;

	_charSelection = -1;
	_characters = 0;
	_spellProperties = 0;
	_updateFlags = 0;
	_selectedSpell = 0;
	_updateCharNum = _updatePortraitSpeechAnimDuration = _portraitSpeechAnimMode = _updateCharV3 = _textColourFlag = _needSceneRestore = 0;
	_fadeText = false;
	_palUpdateTimer = _updatePortraitNext = 0;
	_lampStatusTimer = 0xffffffff;

	_weaponsDisabled = false;
	_charInventoryUnk = 0;
	_lastButtonShape = 0;
	_buttonPressTimer = 0;
	_selectedCharacter = 0;
	_unkFlag = 0;
	_suspendScript = _sceneUpdateRequired = false;
	_scriptDirection = 0;
	_currentDirection = 0;
	_currentBlock = 0;
	memset(_visibleBlockIndex, 0, sizeof(_visibleBlockIndex));

	_smoothScrollModeNormal = 1;
	_wllVmpMap = _wllBuffer3 = _wllBuffer4 = _wllWallFlags = 0;
	_wllShapeMap = 0;
	_lvlShapeTop = _lvlShapeBottom = _lvlShapeLeftRight = 0;
	_levelBlockProperties = 0;
	_monsters = 0;
	_monsterProperties = 0;
	_lvlBlockIndex = _lvlShapeIndex = 0;
	_unkDrawLevelBool = true;
	_vcnBlocks = 0;
	_vcnShift = 0;
	_vcnExpTable = 0;
	_vmpPtr = 0;
	_trueLightTable2 = 0;
	_trueLightTable1 = 0;
	_levelShapeProperties = 0;
	_levelShapes = 0;
	_scriptAssignedLevelShape = 0;
	_blockDrawingBuffer = 0;
	_sceneWindowBuffer = 0;
	memset(_doorShapes, 0, sizeof(_doorShapes));

	_lampEffect = _brightness = _lampOilStatus = 0;
	_lampStatusSuspended = false;
	_tempBuffer5120 = 0;
	_flyingObjects = 0;
	_monsters = 0;
	_lastMouseRegion = 0;
	_monsterLastWalkDirection = _monsterCountUnk = _monsterShiftAlt = 0;
	_monsterCurBlock = 0;
	_seqWindowX1 = _seqWindowY1 = _seqWindowX2 = _seqWindowY2 = _seqTrigger = 0;
	_spsWindowX = _spsWindowY = _spsWindowW = _spsWindowH = 0;

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
	_environmentSfx = _environmentSfxVol = _environmentSfxDistThreshold = 0;

	_sceneDrawVarDown = _sceneDrawVarRight = _sceneDrawVarLeft = _wllProcessFlag = 0;
	_partyPosX = _partyPosY = 0;
	_shpDmX = _shpDmY = _dmScaleW = _dmScaleH = 0;

	_floatingMouseArrowControl = 0;

	memset(_activeTim, 0, sizeof(_activeTim));
	memset(_activeVoiceFile, 0, sizeof(_activeVoiceFile));
	memset(_openDoorState, 0, sizeof(_openDoorState));
	
	_activeVoiceFileTotalTime = 0;
	_pageBuffer1 = _pageBuffer2 = 0;

	memset(_charStatsTemp, 0, sizeof(_charStatsTemp));
	
	_unkBt1 = _unkBt2 = 0;
	_dialogueField = false;

	_rndSpecial = 0x12349876;
	
	_buttonData = 0;
	_activeButtons = 0;
	_preserveEvents = false;
	_buttonList1 = _buttonList2 = _buttonList3 = _buttonList4 = _buttonList5 = _buttonList6 = _buttonList7 = _buttonList8 = 0;

	_monsterDifficulty = 1;
	_smoothScrollingEnabled = true;
	_floatingCursorsEnabled = false;

	memset(_lvlTempData, 0, sizeof(_lvlTempData));
	_unkIceSHpFlag = 0;
	
	_mapOverlay = 0;
	_automapShapes = 0;
	_defaultLegendData = 0;
}

LoLEngine::~LoLEngine() {
	setupPrologueData(false);
	gui_resetButtonList();

	delete[] _landsFile;
	delete[] _levelLangFile;

	delete _screen;
	_screen = 0;
	delete _gui;
	_gui = 0;
	delete _tim;
	_tim = 0;
	delete _txt;
	_txt = 0;

	delete[] _itemsInPlay;
	delete[] _itemProperties;
	delete[] _characters;

	delete[] _pageBuffer1;
	delete[] _pageBuffer2;

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
	if (_effectShapes) {
		for (int i = 0; i < _numEffectShapes; i++)
			delete[]  _effectShapes[i];
		delete[] _effectShapes;
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
	if (_monsterShapesEx) {
		for (int i = 0; i < 576; i++)
			delete[]  _monsterShapesEx[i];
		delete[] _monsterShapesEx;
	}

	if (_automapShapes)
		delete[] _automapShapes;

	for (Common::Array<const TIMOpcode*>::iterator i = _timIntroOpcodes.begin(); i != _timIntroOpcodes.end(); ++i)
		delete *i;
	_timIntroOpcodes.clear();

	for (Common::Array<const TIMOpcode*>::iterator i = _timIngameOpcodes.begin(); i != _timIngameOpcodes.end(); ++i)
		delete *i;
	_timIngameOpcodes.clear();


	delete[] _wllVmpMap;
	delete[] _wllShapeMap;
	delete[] _wllBuffer3;
	delete[] _wllBuffer4;
	delete[] _wllWallFlags;
	delete[] _lvlShapeTop;
	delete[] _lvlShapeBottom;
	delete[] _lvlShapeLeftRight;
	delete[] _tempBuffer5120;
	delete[] _flyingObjects;
	delete[] _monsters;
	delete[] _levelBlockProperties;
	delete[] _monsterProperties;
	delete[] _scriptAssignedLevelShape;

	delete[] _levelFileData;
	delete[] _vcnExpTable;
	delete[] _vcnBlocks;
	delete[] _vcnShift;
	delete[] _vmpPtr;
	delete[] _trueLightTable2;
	delete[] _trueLightTable1;
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

	for (int i = 0; i < 28; i++) {
		if (_lvlTempData[i]) {
			delete[] _lvlTempData[i]->wallsXorData;
			delete[] _lvlTempData[i]->flags;
			delete[] _lvlTempData[i]->monsters;
			delete[] _lvlTempData[i]->flyingObjects;
			delete _lvlTempData[i];
		}		
	}

	delete[] _defaultLegendData;
	delete[] _mapCursorOverlay;
	delete[] _mapOverlay;
}

Screen *LoLEngine::screen() {
	return _screen;
}

GUI *LoLEngine::gui() const {
	return _gui;
}

Common::Error LoLEngine::init() {
	_screen = new Screen_LoL(this, _system);
	assert(_screen);
	_screen->setResolution();

	KyraEngine_v1::init();
	initStaticResource();

	_environmentSfxDistThreshold = (MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB) == MD_ADLIB || ConfMan.getBool("multi_midi")) ? 15 : 3;

	_gui = new GUI_LoL(this);
	assert(_gui);

	_txt = new TextDisplayer_LoL(this, _screen);

	_screen->setAnimBlockPtr(10000);
	_screen->setScreenDim(0);

	_pageBuffer1 = new uint8[0xfa00];
	memset(_pageBuffer1, 0, 0xfa00);
	_pageBuffer2 = new uint8[0xfa00];
	memset(_pageBuffer2, 0, 0xfa00);
 
	_itemsInPlay = new ItemInPlay[400];
	memset(_itemsInPlay, 0, sizeof(ItemInPlay) * 400);

	_characters = new LoLCharacter[4];
	memset(_characters, 0, sizeof(LoLCharacter) * 4);

	if (!_sound->init())
		error("Couldn't init sound");

	_speechFlag = speechEnabled() ? 0x48 : 0;

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
	_monsters = new MonsterInPlay[30];
	memset(_monsters, 0, 30 * sizeof(MonsterInPlay));
	_monsterProperties = new MonsterProperty[5];
	memset(_monsterProperties, 0, 5 * sizeof(MonsterProperty));

	_vcnExpTable = new uint8[128];
	for (int i = 0; i < 128; i++)
		_vcnExpTable[i] = i & 0x0f;

	_tempBuffer5120 = new uint8[5120];
	memset(_tempBuffer5120, 0, 5120);

	_flyingObjects = new FlyingObject[8];
	memset(_flyingObjects, 0, 8 * sizeof(FlyingObject));

	memset(_gameFlags, 0, sizeof(_gameFlags));
	memset(_globalScriptVars, 0, sizeof(_globalScriptVars));

	_levelFileData = 0;
	_lvlShpFileHandle = 0;

	_sceneDrawPage1 = 2;
	_sceneDrawPage2 = 6;

	_monsterShapes = new uint8*[48];
	memset(_monsterShapes, 0, 48 * sizeof(uint8*));
	_monsterPalettes = new uint8*[48];
	memset(_monsterPalettes, 0, 48 * sizeof(uint8*));

	_monsterShapesEx = new uint8*[576];
	memset(_monsterShapesEx, 0, 576 * sizeof(uint8*));
	memset(&_scriptData, 0, sizeof(EMCData));

	_hasTempDataFlags = 0;
	_unkCharNum = -1;

	_automapShapes = new const uint8*[109];
	_mapOverlay = new uint8[256];

	return Common::kNoError;
}

Common::Error LoLEngine::go() {
	setupPrologueData(true);

	if (!saveFileLoadable(0))
		showIntro();

	preInit();

	int processSelection = -1;
	while (!shouldQuit() && processSelection == -1) {
		_screen->loadBitmap("TITLE.CPS", 2, 2, _screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

		_screen->setFont(Screen::FID_6_FNT);
		// Original version: (260|193) "V CD1.02 D"
		const int width = _screen->getTextWidth(gScummVMVersion);
		_screen->fprintString("SVM %s", 300 - width, 193, 0x67, 0x00, 0x04, gScummVMVersion);
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
			showIntro();
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
		_sound->loadSoundFile("LOREINTR");
		_sound->playTrack(6);
		chooseCharacter();
		_sound->playTrack(1);
		_screen->fadeToBlack();
	}

	setupPrologueData(false);

	_tim = new TIMInterpreter_LoL(this, _screen, _system);
	assert(_tim);

	if (!shouldQuit() && (processSelection == 0 || processSelection == 3))
		startup();

	if (!shouldQuit() && processSelection == 0)
		startupNew();

	if (!shouldQuit() && (processSelection == 0 || processSelection == 3)) {
		_screen->_fadeFlag = 3;
		_sceneUpdateRequired = true;
		enableSysTimer(1);
		runLoop();
	}

	delete _tim;
	_tim = 0;

	// TODO: outro
	return Common::kNoError;
}

#pragma mark - Initialization

void LoLEngine::preInit() {
	debugC(9, kDebugLevelMain, "LoLEngine::preInit()");

	_res->loadPakFile("GENERAL.PAK");
	if (_flags.isTalkie)
		_res->loadPakFile("STARTUP.PAK");

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6P.FNT");

	loadTalkFile(0);

	char filename[32];
	snprintf(filename, sizeof(filename), "LANDS.%s", _languageExt[_lang]);
	_res->exists(filename, true);
	_landsFile = _res->fileData(filename, 0);
	loadItemIconShapes();
}

void LoLEngine::loadItemIconShapes() {
	debugC(9, kDebugLevelMain, "LoLEngine::loadItemIconShapes()");

	if (_itemIconShapes) {
		for (int i = 0; i < _numItemIconShapes; i++)
			delete[]  _itemIconShapes[i];
		delete[] _itemIconShapes;
	}

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	const uint8 *shp = _screen->getCPagePtr(3);
	_numItemIconShapes = READ_LE_UINT16(shp);
	_itemIconShapes = new uint8*[_numItemIconShapes];
	for (int i = 0; i < _numItemIconShapes; i++)
		_itemIconShapes[i] = _screen->makeShapeCopy(shp, i);

	_screen->setMouseCursor(0, 0, _itemIconShapes[0]);
}

void LoLEngine::setMouseCursorToIcon(int icon) {
	_gameFlags[15] |= 0x200;
	int i = _itemProperties[_itemsInPlay[_itemInHand].itemPropertyIndex].shpIndex;
	if (i == icon)
		return;
	_screen->setMouseCursor(0, 0, _itemIconShapes[icon]);
}

void LoLEngine::setMouseCursorToItemInHand() {
	_gameFlags[15] &= 0xFDFF;
	int o = (_itemInHand == 0) ? 0 : 10;
	_screen->setMouseCursor(o, o, getItemIconShapePtr(_itemInHand));
}

bool LoLEngine::posWithinRect(int mouseX, int mouseY, int x1, int y1, int x2, int y2) {
	if (mouseX < x1 || mouseX > x2 || mouseY < y1 || mouseY > y2)
		return false;
	return true;
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
	_screen->generateOverlay(_screen->_currentPalette, _screen->_paletteOverlay1, 1, 96);
	_screen->generateOverlay(_screen->_currentPalette, _screen->_paletteOverlay2, 144, 65);
	memcpy(_screen->_currentPalette, tmpPal, 0x300);
	delete[] tmpPal;

	memset(_screen->getPalette(1), 0, 0x300);
	memset(_screen->getPalette(2), 0, 0x300);

	loadItemIconShapes();
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
	_numEffectShapes = READ_LE_UINT16(shp);
	_effectShapes = new uint8*[_numEffectShapes];
	for (int i = 0; i < _numEffectShapes; i++)
		_effectShapes[i] = _screen->makeShapeCopy(shp, i);

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

	_trueLightTable1 = new uint8[256];
	_trueLightTable2 = new uint8[5120];

	_loadSuppFilesFlag = 1;

	_txt->setAnimParameters("<MORE>", 10, 31, 0);
	_txt->setAnimFlag(true);

	_sound->loadSfxFile("LORESFX");

	setMouseCursorToItemInHand();
}

void LoLEngine::startupNew() {
	_selectedSpell = 0;
	_compassUnk = 0;
	_compassDirection = _compassDirectionIndex = -1;

	_lastMouseRegion = -1;
	
	/*
	_unk5 = 1;
	_unk6 = 1;
	_unk7 = 1
	_unk8 = 1*/
	_currentLevel = 1;

	giveCredits(41, 0);
	_inventory[0] = makeItem(216, 0, 0);
	_inventory[1] = makeItem(217, 0, 0);
	_inventory[2] = makeItem(218, 0, 0);

	memset(_availableSpells, -1, 7);
	_availableSpells[0] = 0;
	setupScreenDims();

	memset(_unkWordArraySize8, 0x100, 8);

	static int selectIds[] = { -9, -1, -8, -5 };
	addCharacter(selectIds[_charSelection]);

	gui_enableDefaultPlayfieldButtons();

	loadLevel(_currentLevel);

	_screen->showMouse();
}

void LoLEngine::runLoop() {
	enableSysTimer(2);

	bool _runFlag = true;
	_unkFlag |= 0x800;

	while (!shouldQuit() && _runFlag) {
		if (_nextScriptFunc) {
			runLevelScript(_nextScriptFunc, 2);
			_nextScriptFunc = 0;
		}

		_timer->update();

		//checkFloatingPointerRegions();
		gui_updateInput();

		update();

		if (_sceneUpdateRequired)
			gui_drawScene(0);
		else
			updateEnvironmentalSfx(0);

		/*if (_partyDeathFlag != -1) {
			checkForPartyDeath(_partyDeathFlag);
			_partyDeathFlag = -1;
		}*/

		delay(_tickLength);
	}
}

void LoLEngine::update() {
	updateSequenceBackgroundAnimations();

	if (_updateCharNum != -1 && _system->getMillis() > _updatePortraitNext)
		updatePortraitSpeechAnim();

	if (_gameFlags[15] & 0x800 || !(_updateFlags & 4))
		updateLampStatus();

	if (_gameFlags[15] & 0x4000 && !(_updateFlags & 4) && (_compassDirection == -1 || (_currentDirection << 6) != _compassDirection || _compassUnk))
		updateCompass();

	snd_characterSpeaking();
	fadeText();

	updateInput();
	_screen->updateScreen();
}

#pragma mark - Localization

char *LoLEngine::getLangString(uint16 id) {
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

	char *string = (char *)getTableEntry(buffer, realId);

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

bool LoLEngine::addCharacter(int id) {
	const uint16 *cdf[] = { _charDefsMan, _charDefsMan, _charDefsMan, _charDefsWoman,
		_charDefsMan, _charDefsMan, _charDefsWoman, _charDefsKieran, _charDefsAkshel };

	int numChars = countActiveCharacters();
	if (numChars == 4)
		return false;

	int i = 0;
	for (; i < _charDefaultsSize; i++) {
		if (_charDefaults[i].id == id) {
			memcpy(&_characters[numChars], &_charDefaults[i], sizeof(LoLCharacter));
			_characters[numChars].defaultModifiers = cdf[i];
			break;
		}
	}
	if (i == _charDefaultsSize)
		return false;

	loadCharFaceShapes(numChars, id);

	_characters[numChars].nextAnimUpdateCountdown = (int16) _rnd.getRandomNumberRng(1, 12) + 6;

	for (i = 0; i < 11; i++) {
		if (_characters[numChars].items[i]) {
			_characters[numChars].items[i] = makeItem(_characters[numChars].items[i], 0, 0);
			runItemScript(numChars, _characters[numChars].items[i], 0x80, 0, 0);
		}
	}

	calcCharPortraitXpos();
	if (numChars > 0)
		setFaceFrames(numChars, 2, 6, 0);

	return true;
}

void LoLEngine::setFaceFrames(int charNum, int defaultFrame, int unk2, int redraw) {
	_characters[charNum].defaultFaceFrame = defaultFrame;
	if (defaultFrame || unk2)
		setFaceFramesUnkArrays(charNum, 6, unk2, 1);
	if (redraw)
		gui_drawCharPortraitWithStats(charNum);
}

void LoLEngine::setFaceFramesUnkArrays(int charNum, int unk1, int unk2, int unk3) {
	LoLCharacter *l = &_characters[charNum];
	for (int i = 0; i < 5; i++) {
		if (l->arrayUnk2[i] && (!unk3 || l->arrayUnk2[i] != unk1))
			continue;

		l->arrayUnk2[i] = unk1;
		l->arrayUnk1[i] = unk2;
		_timer->setNextRun(3, _system->getMillis());
		_timer->enable(3);		
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

void LoLEngine::updatePortraitSpeechAnim() {
	int x = 0;
	int y = 0;
	bool redraw = false;

	if (_portraitSpeechAnimMode == 0) {
		x = _activeCharsXpos[_updateCharNum];
		y = 144;
		redraw = true;
	} else if (_portraitSpeechAnimMode == 1) {
		if (textEnabled()) {
			x = 90;
			y = 130;
		} else {
			x = _activeCharsXpos[_updateCharNum];
			y = 144;
		}
	} else if (_portraitSpeechAnimMode == 2) {
		if (textEnabled()) {
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

	if (_speechFlag) {
		if (snd_characterSpeaking() == 2)
			_updatePortraitSpeechAnimDuration = 2;
		else
			_updatePortraitSpeechAnimDuration = 1;
	}

	_updatePortraitSpeechAnimDuration--;

	if (_updatePortraitSpeechAnimDuration) {
		setCharFaceFrame(_updateCharNum, f);
		if (redraw)
			gui_drawCharPortraitWithStats(_updateCharNum);
		else
			gui_drawCharFaceShape(_updateCharNum, x, y, 0);
		_updatePortraitNext = _system->getMillis() + 10 * _tickLength;
	} else if (_updateCharV3 != 0) {
		faceFrameRefresh(_updateCharNum);
		if (redraw) {
			gui_drawCharPortraitWithStats(_updateCharNum);
			initTextFading(0, 0);
		} else {
			gui_drawCharFaceShape(_updateCharNum, x, y, 0);
		}
		_updateCharNum = -1;
	}
}

void LoLEngine::updatePortraits() {
	if (_updateCharNum == -1)
		return;

	_updatePortraitSpeechAnimDuration = _updateCharV3 = 1;
	updatePortraitSpeechAnim();
	_updatePortraitSpeechAnimDuration = 1;
	_updateCharNum = -1;

	if (!_portraitSpeechAnimMode)
		initTextFading(0, 0);
}

void LoLEngine::initTextFading(int textType, int clearField) {
	if (_textColourFlag == textType || !textType) {
		_fadeText = true;
		_palUpdateTimer = _system->getMillis();
	}

	if (!clearField)
		return;

	updatePortraits();
	if (_needSceneRestore)
		_screen->setScreenDim(_txt->clearDim(3));

	_fadeText = false;
	_timer->disable(11);
}

void LoLEngine::setCharFaceFrame(int charNum, int frameNum) {
	_characters[charNum].curFaceFrame = frameNum;
}

void LoLEngine::faceFrameRefresh(int charNum) {
	if (_characters[charNum].curFaceFrame == 1)
		setFaceFrames(charNum, 0, 0, 0);
	else if (_characters[charNum].curFaceFrame == 6)
		if (_characters[charNum].defaultFaceFrame != 5)
			setFaceFrames(charNum, 0, 0, 0);
		else
			_characters[charNum].curFaceFrame = 5;
	else
		_characters[charNum].curFaceFrame = 0;
}

void LoLEngine::recalcCharacterStats(int charNum) {
	for (int i = 0; i < 5; i++)
		_charStatsTemp[i] = calculateCharacterStats(charNum, i);
}

int LoLEngine::calculateCharacterStats(int charNum, int index) {
	if (index == 0) {
		// Might
		int c = 0;
		for (int i = 0; i < 8; i++)
			c += _characters[charNum].itemsMight[i];
		if (c)
			c += _characters[charNum].might;
		else
			c = _characters[charNum].defaultModifiers[8];

		c = (c * _characters[charNum].defaultModifiers[1]) >> 8;
		c = (c * _characters[charNum].totalMightModifier) >> 8;

		return c;

	} else if (index == 1) {
		// Protection
		return calculateProtection(charNum);

	} else if (index > 4) {
		return -1;

	} else {
		// Fighter
		// Rogue
		// Mage
		index -= 2;
		return _characters[charNum].skillLevels[index] + _characters[charNum].skillModifiers[index];
	}

	return 1;
}

int LoLEngine::calculateProtection(int index) {
	int c = 0;
	if (index & 0x8000) {
		// Monster
		index &= 0x7fff;
		c = (_monsters[index].properties->itemProtection * _monsters[index].properties->fightingStats[2]) >> 8;
	} else {
		// Character
		c = _characters[index].itemsProtection + _characters[index].protection;
		c = (c * _characters[index].defaultModifiers[2]) >> 8;
		c = (c * _characters[index].totalProtectionModifier) >> 8;
	}

	return c;
}

void LoLEngine::increaseExperience(int charNum, int skill, uint32 points) {
	if (charNum & 0x8000)
		return;

	if (_characters[charNum].flags & 8)
		return;

	_characters[charNum].experiencePts[skill] += points;

	bool loop = true;
	while (loop) {
		if (_characters[charNum].experiencePts[skill] <= _expRequirements[_characters[charNum].skillLevels[skill]])
			break;

		_characters[charNum].skillLevels[skill]++;
		_characters[charNum].flags |= (0x200 << skill);
		int inc = 0;

		switch (skill) {
			case 0:
				_txt->printMessage(0x8003, getLangString(0x4023), _characters[charNum].name);
				inc = _rnd.getRandomNumberRng(4, 6);
				_characters[charNum].hitPointsCur += inc;
				_characters[charNum].hitPointsMax += inc;
				break;

			case 1:
				_txt->printMessage(0x8003, getLangString(0x4025), _characters[charNum].name);
				inc = _rnd.getRandomNumberRng(2, 6);
				_characters[charNum].hitPointsCur += inc;
				_characters[charNum].hitPointsMax += inc;
				break;

			case 2:
				_txt->printMessage(0x8003, getLangString(0x4024), _characters[charNum].name);
				inc = (_characters[charNum].defaultModifiers[6] * (_rnd.getRandomNumberRng(1, 8) + 17)) >> 8;
				_characters[charNum].magicPointsCur += inc;
				_characters[charNum].magicPointsMax += inc;			
				inc = _rnd.getRandomNumberRng(1, 6);
				_characters[charNum].hitPointsCur += inc;
				_characters[charNum].hitPointsMax += inc;
				break;

			default:
				break;
		}

		snd_playSoundEffect(118, -1);
		gui_drawCharPortraitWithStats(charNum);
	}
}

void LoLEngine::setupScreenDims() {
	if (textEnabled()) {
		_screen->modifyScreenDim(4, 11, 124, 28, 45);
		_screen->modifyScreenDim(5, 85, 123, 233, 54);
	} else {
		_screen->modifyScreenDim(4, 11, 124, 28, 9);
		_screen->modifyScreenDim(5, 85, 123, 233, 18);
	}
}

void LoLEngine::initSceneWindowDialogue(int controlMode) {
	resetPortraitsAndDisableSysTimer();
	gui_prepareForSequence(112, 0, 176, 120, controlMode);

	_updateFlags |= 3;

	_txt->setupField(true);
	_txt->expandField();
	setupScreenDims();
	gui_disableControls(controlMode);
}

void LoLEngine::toggleSelectedCharacterFrame(bool mode) {
	if (countActiveCharacters() == 1)
		return;

	int col = mode ? 212 : 1;

	int cp = _screen->setCurPage(0);
	int x = _activeCharsXpos[_selectedCharacter];

	_screen->drawBox(x, 143, x + 65, 176, col);
	_screen->setCurPage(cp);
}

void LoLEngine::gui_prepareForSequence(int x, int y, int w, int h, int buttonFlags) {
	setSequenceButtons(x, y, w, h, buttonFlags);

	_seqWindowX1 = x;
	_seqWindowY1 = y;
	_seqWindowX2 = x + w;
	_seqWindowY2 = y + h;

	int mouseOffs = _itemInHand ? 10 : 0;
	_screen->setMouseCursor(mouseOffs, mouseOffs, getItemIconShapePtr(_itemInHand));

	_lastMouseRegion = -1;

	if (w == 320) {
		setLampMode(false);
		_lampStatusSuspended = true;
	}
}

void LoLEngine::gui_specialSceneSuspendControls(int controlMode) {
	if (controlMode) {
		_updateFlags |= 4;
		setLampMode(false);
	}
	_updateFlags |= 1;
	_specialSceneFlag = 1;
	_currentControlMode = controlMode;
	calcCharPortraitXpos();
	//checkMouseRegions();
}

void LoLEngine::gui_specialSceneRestoreControls(int restoreLamp) {
	if (restoreLamp) {
		_updateFlags &= 0xfffa;
		resetLampStatus();
	}
	_updateFlags &= 0xfffe;
	_specialSceneFlag = 0;
	//checkMouseRegions();
}

void LoLEngine::restoreAfterSceneWindowDialogue(int redraw) {
	gui_enableControls();
	_txt->setupField(false);
	_updateFlags &= 0xffdf;

	setDefaultButtonState();

	for (int i = 0; i < 6; i++)
		_tim->freeAnimStruct(i);

	_updateFlags = 0;

	if (redraw) {
		if (_screen->_fadeFlag != 2)
			_screen->fadeClearSceneWindow(10);
		gui_drawPlayField();
		setPaletteBrightness(_screen->_currentPalette, _brightness, _lampEffect);
		_screen->_fadeFlag = 0;
	}

	_needSceneRestore = 0;
	enableSysTimer(2);
}

void LoLEngine::initDialogueSequence(int controlMode, int pageNum) {
	if (controlMode) {
		_timer->disable(11);
		_fadeText = false;
		int cp = _screen->setCurPage(pageNum);

		_screen->fillRect(0, 128, 319, 199, 1);
		gui_drawBox(0, 129, 320, 71, 136, 251, -1);
		gui_drawBox(1, 130, 318, 69, 136, 251, 252);

		_screen->modifyScreenDim(5, 8, 131, 304, 66);
		_screen->modifyScreenDim(4, 1, 133, 38, 60);
		_txt->clearDim(4);

		_updateFlags |= 2;
		_currentControlMode = controlMode;
		calcCharPortraitXpos();

		if (!textEnabled() && (!(controlMode & 2))) {
			int nc = countActiveCharacters();
			for (int i = 0; i < nc; i++) {
				_portraitSpeechAnimMode = 2;
				_updateCharNum = i;
				_screen->drawShape(0, _gameShapes[88], _activeCharsXpos[_updateCharNum] + 8, 142, 0, 0);
				updatePortraits();
			}
		}

		_screen->setCurPage(cp);

	} else {
		_txt->setupField(true);
		_txt->expandField();
		setupScreenDims();
		_txt->clearDim(4);
	}
	
	_currentControlMode = controlMode;
	_dialogueField = true;	
}

void LoLEngine::restoreAfterDialogueSequence(int controlMode) {
	if (!_dialogueField)
		return;

	updatePortraits();
	_currentControlMode = controlMode;
	calcCharPortraitXpos();

	if (_currentControlMode) {
		_screen->modifyScreenDim(4, 11, 124, 28, 45);
		_screen->modifyScreenDim(5, 85, 123, 233, 54);
		_updateFlags &= 0xfffd;
	} else {
		const ScreenDim *d = _screen->getScreenDim(5);
		_screen->fillRect(d->sx, d->sy, d->sx + d->w - 2, d->sy + d->h - 2, d->unkA);
		_txt->clearDim(4);
		_txt->setupField(false);
	}

	_dialogueField = false;
}

void LoLEngine::resetPortraitsAndDisableSysTimer() {
	_needSceneRestore = 1;
	if (!textEnabled() || (!(_currentControlMode & 2)))
		timerUpdatePortraitAnimations(1);

	disableSysTimer(2);
}

void LoLEngine::fadeText() {
	if (!_fadeText)
		return;

	if (_screen->fadeColour(192, 252, _system->getMillis() - _palUpdateTimer, 60 * _tickLength))
		return;

	if (_needSceneRestore)
		return;

	_screen->setScreenDim(_txt->clearDim(3));

	_timer->disable(11);

	_fadeText = false;
}

void LoLEngine::setPaletteBrightness(uint8 *palette, int brightness, int modifier) {
	generateBrightnessPalette(palette, _screen->getPalette(1), brightness, modifier);
	_screen->fadePalette(_screen->getPalette(1), 5, 0);
	_screen->_fadeFlag = 0;
}

void LoLEngine::generateBrightnessPalette(uint8 *src, uint8 *dst, int brightness, int modifier) {
	memcpy(dst, src, 0x300);
	_screen->loadSpecialColours(dst);
	brightness = (8 - brightness) << 5;
	if (modifier >= 0 && modifier < 8 && _gameFlags[15] & 0x800) {
		brightness = 256 - ((((modifier & 0xfffe) << 5) * (256 - brightness)) >> 8);
		if (brightness < 0)
			brightness = 0;
	}
	
	for (int i = 0; i < 384; i++) {
		uint16 c = (dst[i] * brightness) >> 8;
		dst[i] = c & 0xff;
	}
}

void LoLEngine::updateSequenceBackgroundAnimations() {
	if (_updateFlags & 8)
		return;

	for (int i = 0; i < 6; i++)
		_tim->updateBackgroundAnimation(i);
}

void LoLEngine::savePage5() {
	if (_pageSavedFlag)
		return;

	_screen->copyRegionToBuffer(5, 0, 0, 320, 200, _pageBuffer2);
	_pageSavedFlag = true;
}

void LoLEngine::restorePage5() {
	if (!_pageSavedFlag)
		return;
	
	for (int i = 0; i < 6; i++)
		_tim->freeAnimStruct(i);

	_screen->copyBlockToPage(5, 0, 0, 320, 200, _pageBuffer2);
	_pageSavedFlag = false;
}

void LoLEngine::loadTalkFile(int index) {
	char file[8];

	if (index == _curTlkFile)
		return;

	if (_curTlkFile > 0 && index > 0) {
		snprintf(file, sizeof(file), "%02d.TLK", _curTlkFile);
		_res->unloadPakFile(file);
	}

	if (index > 0)
		_curTlkFile = index;

	snprintf(file, sizeof(file), "%02d.TLK", index);
	_res->loadPakFile(file);
}

bool LoLEngine::snd_playCharacterSpeech(int id, int8 speaker, int) {
	if (!_speechFlag)
		return false;

	if (speaker < 65) {
		if (_characters[speaker].flags & 1)
			speaker = (int) _characters[speaker].name[0];
		else
			speaker = 0;
	}

	if (_lastSpeechId == id && speaker == _lastSpeaker)
		return true;

	_lastSpeechId = id;
	_lastSpeaker = speaker;
	_nextSpeechId = _nextSpeaker = -1;

	Common::List<const char*> playList;

	char pattern1[8];
	char pattern2[5];
	char file1[13];
	char file2[13];
	char file3[13];
	file3[0] = 0;

	snprintf(pattern2, sizeof(pattern2), "%02d", id & 0x4000 ? 0 : _curTlkFile);

	if (id & 0x4000) {
		snprintf(pattern1, sizeof(pattern1), "%03X", id & 0x3fff);
	} else if (id < 1000) {
		snprintf(pattern1, sizeof(pattern1), "%03d", id);
	} else {
		snprintf(file3, sizeof(file3), "@%04d%c.%s", id - 1000, (char)speaker, pattern2);
		if (_res->exists(file3)) {
			char *f = new char[strlen(file3) + 1];
			strcpy(f, file3);
			playList.push_back(f);
		}
	}

	if (!file3[0]) {
		for (char i = 0; ; i++) {
			char symbol = '0' + i;
			snprintf(file1, sizeof(file1), "%s%c%c.%s", pattern1, (char)speaker, symbol, pattern2);
			snprintf(file2, sizeof(file2), "%s%c%c.%s", pattern1, '_', symbol, pattern2);
			if (_res->exists(file1)) {
				char *f = new char[strlen(file1) + 1];
				strcpy(f, file1);
				playList.push_back(f);
			} else if (_res->exists(file2)) {
				char *f = new char[strlen(file2) + 1];
				strcpy(f, file2);
				playList.push_back(f);
			} else {
				break;
			}
		}
	}

	if (playList.empty())
		return false;

	while (_sound->voiceIsPlaying(_activeVoiceFile)) {
		update();
		delay(_tickLength);
	};

	strcpy(_activeVoiceFile, *playList.begin());
	_activeVoiceFileTotalTime = _sound->voicePlayFromList(playList);

	for (Common::List<const char*>::iterator i = playList.begin(); i != playList.end(); i++)
		delete []*i;
	playList.clear();

	_tim->_abortFlag = 0;

	return true;
}

int LoLEngine::snd_characterSpeaking() {
	if (_sound->voiceIsPlaying(_activeVoiceFile))
		return 2;

	_lastSpeechId = _lastSpeaker = -1;
	_activeVoiceFileTotalTime = 0;

	if (_nextSpeechId != -1) {
		if (snd_playCharacterSpeech(_nextSpeechId, _nextSpeaker, 0))
			return 2;
	}

	return 1;
}

void LoLEngine::snd_stopSpeech(bool setFlag) {
	if (!_sound->voiceIsPlaying(_activeVoiceFile))
		return;

	//_dlgTimer = 0;
	_sound->voiceStop(_activeVoiceFile);
	_activeVoiceFileTotalTime = 0;
	_nextSpeechId = _nextSpeaker = -1;

	if (setFlag)
		_tim->_abortFlag = 1;
}

void LoLEngine::snd_playSoundEffect(int track, int volume) {
	debugC(9, kDebugLevelMain | kDebugLevelSound, "LoLEngine::snd_playSoundEffect(%d, %d)", track, volume);

	if (track == 1 && (_lastSfxTrack == -1 || _lastSfxTrack == 1))
		return;

	_lastSfxTrack = track;

	int16 volIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2 + 1]);

	if (volIndex > 0)
		volume = (volIndex * volume) >> 8;
	else
		volume = -volIndex;

	// volume TODO
	volume = 254 - volume;

	int16 vocIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2]);
	if (vocIndex != -1
) {
		_sound->voicePlay(_ingameSoundList[vocIndex], volume & 0xff, true);
	} else if (_flags.platform == Common::kPlatformPC) {
		if (_sound->getSfxType() == Sound::kMidiMT32)
			track = track < _ingameMT32SoundIndexSize ? _ingameMT32SoundIndex[track] - 1 : -1;
		else if (_sound->getSfxType() == Sound::kMidiGM)
			track = track < _ingameGMSoundIndexSize ? _ingameGMSoundIndex[track] - 1: -1;

		if (track == 168)
			track = 167;

		if (track != -1)
			KyraEngine_v1::snd_playSoundEffect(track, volume);
	}
}

void LoLEngine::snd_processEnvironmentalSoundEffect(int soundId, int block) {
	if (!_sound->sfxEnabled())
		return;

	if (_environmentSfx)
		snd_playSoundEffect(_environmentSfx, _environmentSfxVol);

	int dist = 0;
	if (block) {
		dist = getMonsterDistance(_currentBlock, block);
		if (dist > _environmentSfxDistThreshold) {
			_environmentSfx = 0;
			return;
		}
	}

	_environmentSfx = soundId;
	_environmentSfxVol = (15 - ((block || dist < 2) ? dist : 0)) << 4;

	if (block != _currentBlock) {
		static const int8 blockShiftTable[] = { -32, -31, 1, 33, 32, 31, -1, -33 };
		uint16 cbl = _currentBlock;

		for (int i = 3; i > 0; i--) {
			int dir = calcMonsterDirection(cbl & 0x1f, cbl >> 5, block & 0x1f, block >> 5);
			cbl += blockShiftTable[dir];
			if (cbl != block) {
				if (testWallFlag(cbl, 0, 1))
					_environmentSfxVol >>= 1;
			}
		}
	}

	if (!soundId || _sceneUpdateRequired)
		return;

	snd_processEnvironmentalSoundEffect(0, 0);
}

void LoLEngine::snd_loadSoundFile(int track) {
	if (_sound->musicEnabled()) {
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

	if (_sound->musicEnabled()) {
		snd_loadSoundFile(track);
		int t = (track - 250) * 3;
		_sound->playTrack(_musicTrackMap[t + 2]);
	}

	return res;
}

int LoLEngine::snd_stopMusic() {
	if (_sound->musicEnabled()) {
		if (_sound->isPlaying()) {
			_sound->beginFadeOut();
			_system->delayMillis(3 * _tickLength);
		}

		_sound->haltTrack();
	}
	return snd_playTrack(-1);
}

int LoLEngine::characterSays(int track, int charId, bool redraw) {
	if (charId == 1) {
		charId = _selectedCharacter;
	} else {
		int i = 0;
		for (;i < 4; i++) {
			if (charId != _characters[i].id || !(_characters[i].flags & 1))
				continue;
			charId = i;
			break;
		}

		if (i == 4)
			return 0;
	}

	bool r = snd_playCharacterSpeech(track, charId, 0);

	if (r && redraw) {
		updatePortraits();
		_updateCharNum = charId;
		_portraitSpeechAnimMode = 0;
		_updateCharV3 = 1;
		_fadeText = false;
		updatePortraitSpeechAnim();
	}

	return r ? (textEnabled() ? 1 : 0) : 1;
}

int LoLEngine::playCharacterScriptChat(int charId, int mode, int unk1, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex) {
	int ch = 0;
	bool skipAnim = false;

	if ((charId == -1) || (!(charId & 0x70)))
		charId = ch = (charId == 1) ? (_selectedCharacter ? _characters[_selectedCharacter].id : 0) : charId;
	else
		charId ^= 0x70;

	updatePortraits();

	if (charId < 0) {
		charId = ch = (_rnd.getRandomNumber(0x7fff) * countActiveCharacters()) / 0x8000;
		ch = _rnd.getRandomNumber(countActiveCharacters() - 1);
	} else if (charId > 0) {
		int i = 0;

		for (; i < 4; i++) {
			if (_characters[i].id != charId || !(_characters[i].flags & 1))
				continue;
			if (charId == ch)
				ch = i;
			charId = i;
			break;
		}

		if (i == 4) {
			if (charId == 8)
				skipAnim = true;
			else
				return 0;
		}
	}

	if (!skipAnim) {
		_updateCharNum = charId;
		_portraitSpeechAnimMode = mode;
		_updatePortraitSpeechAnimDuration = strlen(str) >> 1;
		_updateCharV3 = unk1;
	}

	if (script)
		snd_playCharacterSpeech(script->stack[script->sp + 2], ch, 0);
	else if (paramList)
		snd_playCharacterSpeech(paramList[2], ch, 0);

	if (textEnabled()) {
		if (mode == 0) {
			_txt->printDialogueText(3, str, script, paramList, paramIndex);

		} else if (mode == 1) {
			_txt->clearDim(4);
			_screen->modifyScreenDim(4, 16, 123, 23, 47);
			_txt->printDialogueText(4, str, script, paramList, paramIndex);
			_screen->modifyScreenDim(4, 11, 123, 28, 47);

		} else if (mode == 2) {
			_txt->clearDim(4);
			_screen->modifyScreenDim(4, 9, 133, 30, 60);
			_txt->printDialogueText(4, str, script, paramList, 3);
			_screen->modifyScreenDim(4, 1, 133, 37, 60);
		}
	}

	_fadeText = 0;
	if (!skipAnim)
		updatePortraitSpeechAnim();

	return 1;
}

void LoLEngine::giveItemToMonster(MonsterInPlay *monster, uint16 item) {
	uint16 *c = &monster->assignedItems;
	while (*c)
		c = &_itemsInPlay[*c].nextAssignedObject;
	*c = item;
	_itemsInPlay[item].nextAssignedObject = 0;
}

const uint16 *LoLEngine::getCharacterOrMonsterStats(int id) {
	return (id & 0x8000) ? (const uint16*)_monsters[id & 0x7fff].properties->fightingStats : _characters[id].defaultModifiers;
}

void LoLEngine::delay(uint32 millis, bool cUpdate, bool isMainLoop) {
	uint32 endTime = _system->getMillis() + millis;
	while (endTime > _system->getMillis()) {
		if (cUpdate)
			update();
		_system->delayMillis(4);
	}
}

uint8 LoLEngine::getRandomNumberSpecial() {
	uint8 a = _rndSpecial & 0xff;
	uint8 b = (_rndSpecial >> 8) & 0xff;
	uint8 c = (_rndSpecial >> 16) & 0xff;

	a >>= 1;

	uint as = a & 1;
	uint bs = (b >> 7) ? 0 : 1;
	uint cs = c >> 7;

	a >>= 1;
	c = (c << 1) | as;	
	b = (b << 1) | cs;

	a -= ((_rndSpecial & 0xff) - bs);	
	as = a & 1;
	a >>= 1;

	a = ((_rndSpecial & 0xff) >> 1) | (as << 7);

	_rndSpecial = (_rndSpecial & 0xff000000) | (c << 16) | (b << 8) | a;

	return a ^ b;
}

void LoLEngine::updateEnvironmentalSfx(int soundId) {
	snd_processEnvironmentalSoundEffect(soundId, _currentBlock);
}

bool LoLEngine::notEnoughMagic(int charNum, int spellNum, int spellLevel) {
	if (_spellProperties[spellNum].mpRequired[spellLevel] > _characters[charNum].magicPointsCur) {
		if (characterSays(0x4043, _characters[charNum].id, true))
			_txt->printMessage(6, getLangString(0x4043), _characters[charNum].name);
		return true;
	} else if (_spellProperties[spellNum + 1].unkArr[spellLevel] >= _characters[charNum].hitPointsCur) {
		_txt->printMessage(2, getLangString(0x4179), _characters[charNum].name);
		return true;
	}

	return false;
}

int LoLEngine::battleHitSkillTest(int16 attacker, int16 target, int skill) {
	if (target == -1)
		return 0;
	if (attacker == -1)
		return 1;

	if (target & 0x8000) {
		if (_monsters[target & 0x7fff].mode >= 13)
			return 0;
	}
	
	uint16 hitChanceModifier = 0;
	uint16 evadeChanceModifier = 0;
	int sk = 0;

	if (attacker & 0x8000) {
		hitChanceModifier = _monsters[target & 0x7fff].properties->fightingStats[0];
		sk = 100 - _monsters[target & 0x7fff].properties->skillLevel;
	} else {
		hitChanceModifier = _characters[attacker].defaultModifiers[0];		
		uint8 m = _characters[attacker].skillModifiers[skill];
		if (skill == 1)
			m *= 3;
		sk = 100 - (_characters[attacker].skillLevels[skill] + m);
	}

	if (target & 0x8000) {
		evadeChanceModifier = _monsters[target & 0x7fff].properties->fightingStats[3];
		_monsters[target & 0x7fff].flags |= 0x10;
	} else {
		evadeChanceModifier = _characters[target].defaultModifiers[3];
	}

	int r = _rnd.getRandomNumberRng(1, 100);
	if (r >= sk)
		return 2;

	uint16 v = ((_monsterModifiers[9 + _monsterDifficulty] * evadeChanceModifier) & 0xffffff00) / hitChanceModifier;

	if (r < v)
		return 0;

	return 1;
}

int LoLEngine::calcInflictableDamage(int16 attacker, int16 target, int hitType) {
	const uint16 *s = getCharacterOrMonsterStats(attacker);
	
	int res = 0;
	for (int i = 0; i < 8; i++)
		res += calcInflictableDamagePerStat(attacker, target, s[2 + i], i, hitType);

	return res;
}

void LoLEngine::battleHit_sub2(int16 target, int damageInflicted, int16 attacker, uint32 b) {

}

void LoLEngine::battleHit_sub3(MonsterInPlay *monster, int16 target, int16 damageInflicted) {

}

int LoLEngine::calcInflictableDamagePerStat(int16 attacker, int16 target, uint16 stat2m, int index, int hitType) {
	return 1;
}

uint16 LoLEngine::getClosestMonster(int x, int y) {
	uint16 id = 0xffff;
	int minDist = 0x7fff;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].mode > 13)
			continue;

		int d = ABS(x - _monsters[i].x) + ABS(y - _monsters[i].y);		
		if (d < minDist) {
			minDist = d;
			id = 0x8000 | i;
		}
	}

	return id;
}

uint16 LoLEngine::getClosestPartyMember(int x, int y) {
	uint16 id = 0xffff;
	int minDist = 0x7fff;

	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1) || _characters[i].hitPointsCur <= 0)
			continue;

		uint16 charX = 0;
		uint16 charY = 0;
		calcCoordinatesForSingleCharacter(i, charX, charY);

		int d = ABS(x - charX) + ABS(y - charY);
		if (d < minDist) {
			minDist = d;
			id = i;
		}
	}

	return id;
}

void LoLEngine::generateTempData() {
	int l = _currentLevel - 1;
	if (_lvlTempData[l]) {
		delete[] _lvlTempData[l]->wallsXorData;
		delete[] _lvlTempData[l]->flags;
		delete[] _lvlTempData[l]->monsters;
		delete[] _lvlTempData[l]->flyingObjects;
		delete _lvlTempData[l];
	}

	_lvlTempData[l] = new LevelTempData;

	_lvlTempData[l]->wallsXorData = new uint8[4096];
	_lvlTempData[l]->flags = new uint8[1024];
	_lvlTempData[l]->monsters = new MonsterInPlay[30];
	_lvlTempData[l]->flyingObjects = new FlyingObject[8];

	char filename[13];
	snprintf(filename, sizeof(filename), "LEVEL%d.CMZ", _currentLevel);

	_screen->loadBitmap(filename, 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(p + 4);
	p += 6;

	memset(_lvlTempData[l]->wallsXorData, 0, 4096);
	memset(_lvlTempData[l]->flags, 0, 1024);
	uint8 *d = _lvlTempData[l]->wallsXorData;
	uint8 *df = _lvlTempData[l]->flags;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			*d++ = p[i * len + ii] ^ _levelBlockProperties[i].walls[ii];
		*df++ = _levelBlockProperties[i].flags;
	}

	memcpy(_lvlTempData[l]->monsters, _monsters,  sizeof(MonsterInPlay) * 30);
	memcpy(_lvlTempData[l]->flyingObjects, _flyingObjects,  sizeof(FlyingObject) * 8);

	_lvlTempData[l]->monsterDifficulty =_monsterDifficulty;

	_hasTempDataFlags |= (1 << l);
}

// magic atlas

void LoLEngine::displayAutomap() {
	snd_playSoundEffect(105, -1);
	gui_toggleButtonDisplayMode(78, 1);

	_currentMapLevel = _currentLevel;
	uint8 *tmpWll = new uint8[80];
	memcpy(tmpWll, _wllBuffer4, 80);

	_screen->loadBitmap("parch.cps", 2, 2, _screen->getPalette(3));
	_screen->loadBitmap("autobut.shp", 3, 5, 0);
	const uint8 *shp = _screen->getCPagePtr(5);

	for (int i = 0; i < 109; i++)
		_automapShapes[i] = _screen->getPtrToShape(shp, i + 11);
	
	_screen->generateGrayOverlay(_screen->getPalette(3), _mapOverlay, 52, 0, 0, 0, 256, false);

	_screen->loadFont(Screen::FID_9_FNT, "FONT9PN.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6PN.FNT");

	for (int i = 0; i < 11; i++)
		_defaultLegendData[i].enable = false;

	disableSysTimer(2);
	generateTempData();
	resetItems(1);
	disableMonsters();

	bool exitAutomap = false;
	_mapUpdateNeeded = false;

	restoreBlockTempData(_currentMapLevel);
	loadMapLegendData(_currentMapLevel);
	_screen->fadeToBlack(10);
	drawMapPage(2);

	_screen->copyPage(2, 0);
	_screen->updateScreen();
	_screen->fadePalette(_screen->getPalette(3), 10);
	_smoothScrollTimer = _system->getMillis() + 8 * _tickLength;

	while (!exitAutomap) {
		if (_mapUpdateNeeded) {
			drawMapPage(2);
			_screen->copyPage(2, 0);
			_screen->updateScreen();
			_mapUpdateNeeded = false;
		}

		if (_system->getMillis() >= _smoothScrollTimer) {
			redrawMapCursor();
			_smoothScrollTimer = _system->getMillis() + 8 * _tickLength;
		}

		int f = checkInput(0) & 0xff;
		removeInputTop();

		if (f) {
			exitAutomap = automapProcessButtons(f);
			gui_notifyButtonListChanged();
		}

		if (f == 0x30) {
			for (int i = 0; i < 1024; i++)
				 _levelBlockProperties[i].flags |= 7;
			_mapUpdateNeeded = true;
		} else if (f == 0x6e) {
			exitAutomap = true;
		}

		delay (_tickLength);
	}

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");
	_screen->loadFont(Screen::FID_6_FNT, "FONT6P.FNT");

	_screen->fadeToBlack(10);
	loadLevelWallData(_currentLevel, false);
	memcpy(_wllBuffer4, tmpWll, 80);
	delete[] tmpWll;
	restoreBlockTempData(_currentLevel);
	addLevelItems();
	gui_notifyButtonListChanged();
	enableSysTimer(2);
}

void LoLEngine::updateAutoMap(uint16 block) {
	if (!(_gameFlags[15] & 0x1000))
		return;
	_levelBlockProperties[block].flags |= 7;

	uint16 x = block & 0x1f;
	uint16 y = block >> 5;

	updateAutoMapIntern(block, x, y, -1, -1);
	updateAutoMapIntern(block, x, y, 1, -1);
	updateAutoMapIntern(block, x, y, -1, 1);
	updateAutoMapIntern(block, x, y, 1, 1);
	updateAutoMapIntern(block, x, y, 0, -1);
	updateAutoMapIntern(block, x, y, 0, 1);
	updateAutoMapIntern(block, x, y, -1, 0);
	updateAutoMapIntern(block, x, y, 1, 0);
}

bool LoLEngine::updateAutoMapIntern(uint16 block, uint16 x, uint16 y, int16 xOffs, int16 yOffs) {
	static const int16 blockPosTable[] = { 1, -1, 3, 2, -1, 0, -1, 0, 1, -32, 0, 32 };
	x += xOffs;
	y += yOffs;

	if ((x & 0xffe0) || (y & 0xffe0))
		return false;

	xOffs++;
	yOffs++;

	int16 fx = blockPosTable[xOffs];
	uint16 b = block + blockPosTable[6 + xOffs];

	if (fx != -1) {
		if (_wllBuffer4[_levelBlockProperties[b].walls[fx]] & 0xc0)
			return false;
	}

	int16 fy = blockPosTable[3 + yOffs];
	b = block + blockPosTable[9 + yOffs];

	if (fy != -1) {
		if (_wllBuffer4[_levelBlockProperties[b].walls[fy]] & 0xc0)
			return false;
	}

	b = block + blockPosTable[6 + xOffs] + blockPosTable[9 + yOffs];

	if ((fx != -1) && (fy != -1) && (_wllBuffer4[_levelBlockProperties[b].walls[fx]] & 0xc0) && (_wllBuffer4[_levelBlockProperties[b].walls[fy]] & 0xc0))
		return false;

	_levelBlockProperties[b].flags |= 7;

	return true;
}

void LoLEngine::loadMapLegendData(int level) {
	uint16 *legendData= (uint16*) _tempBuffer5120;
	for (int i = 0; i < 32; i++) {
		legendData[i * 6] = 0xffff;
		legendData[i * 6 + 5] = 0xffff;
	}

	char file[13];
	uint32 size = 0;
	snprintf(file, 12, "level%d.xxx", level);
	uint8 *data = _res->fileData(file, &size);
	uint8 *pos = data;
	size = MIN<uint32>(size / 12, 32);

	for (uint32 i = 0; i < size; i++) {
		uint16 *l = &legendData[i * 6];
		l[3] = READ_LE_UINT16(pos);
		pos += 2;
		l[4] = READ_LE_UINT16(pos);
		pos += 2;
		l[5] = READ_LE_UINT16(pos);
		pos += 2;
		l[0] = READ_LE_UINT16(pos);
		pos += 2;
		l[1] = READ_LE_UINT16(pos);
		pos += 2;
		l[2] = READ_LE_UINT16(pos);
		pos += 2;
	}

	delete[] data;
}

void LoLEngine::drawMapPage(int pageNum) {
	for (int i = 0; i < 2; i++) {
		_screen->loadBitmap("parch.cps", pageNum, pageNum, _screen->getPalette(3));
		
		int cp = _screen->setCurPage(pageNum);
		Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
		_screen->printText(getLangString(_autoMapStrings[_currentMapLevel]), 236, 8, 1, 0);
		uint16 blX = mapGetStartPosX();
		uint16 bl = (mapGetStartPosY() << 5) + blX;

		int sx = _automapTopLeftX;
		int sy = _automapTopLeftY;

		for (; bl < 1024; bl++) {
			uint8 *w = _levelBlockProperties[bl].walls;
			if ((_levelBlockProperties[bl].flags & 7) == 7 && (!(_wllBuffer4[w[0]] & 0xc0)) && (!(_wllBuffer4[w[2]] & 0xc0)) && (!(_wllBuffer4[w[1]] & 0xc0))&& (!(_wllBuffer4[w[3]] & 0xc0))) {
				uint16 b0 = calcNewBlockPosition(bl, 0);
				uint16 b2 = calcNewBlockPosition(bl, 2);
				uint16 b1 = calcNewBlockPosition(bl, 1);
				uint16 b3 = calcNewBlockPosition(bl, 3);

				uint8 w02 = _levelBlockProperties[b0].walls[2];
				uint8 w20 = _levelBlockProperties[b2].walls[0];
				uint8 w13 = _levelBlockProperties[b1].walls[3];
				uint8 w31 = _levelBlockProperties[b3].walls[1];

				// draw block
				_screen->copyBlockSpecial(_screen->_curPage, sx, sy, _screen->_curPage, sx, sy, 7, 6, 0, _mapOverlay);

				// draw north wall
				drawMapBlockWall(b3, w31, sx, sy, 3);
				drawMapShape(w31, sx, sy, 3);
				if (_wllBuffer4[w31] & 0xc0)
					_screen->copyBlockSpecial(_screen->_curPage, sx, sy, _screen->_curPage, sx, sy, 1, 6, 0, _mapOverlay);

				// draw west wall
				drawMapBlockWall(b1, w13, sx, sy, 1);
				drawMapShape(w13, sx, sy, 1);
				if (_wllBuffer4[w13] & 0xc0)
					_screen->copyBlockSpecial(_screen->_curPage, sx + 6, sy, _screen->_curPage, sx + 6, sy, 1, 6, 0, _mapOverlay);

				// draw east wall
				drawMapBlockWall(b0, w02, sx, sy, 0);
				drawMapShape(w02, sx, sy, 0);
				if (_wllBuffer4[w02] & 0xc0)
					_screen->copyBlockSpecial(_screen->_curPage, sx, sy, _screen->_curPage, sx, sy, 7, 1, 0, _mapOverlay);

				//draw south wall
				drawMapBlockWall(b2, w20, sx, sy, 2);
				drawMapShape(w20, sx, sy, 2);
				if (_wllBuffer4[w20] & 0xc0)
					_screen->copyBlockSpecial(_screen->_curPage, sx, sy + 5, _screen->_curPage, sx, sy + 5, 7, 1, 0, _mapOverlay);
			}

			sx += 7;
			if (bl % 32 == 31) {
				sx = _automapTopLeftX;
				sy += 6;
				bl += blX;
			}
		}

		_screen->setFont(of);
		_screen->setCurPage(cp);

		of = _screen->setFont(Screen::FID_6_FNT);

		int tY = 0;
		sx = mapGetStartPosX();
		sy = mapGetStartPosY();

		uint16 *legendData = (uint16*)_tempBuffer5120;

		for (int ii = 0; ii < 32; ii++)  {
			uint16 *l = &legendData[ii * 6];
			if (l[0] == 0xffff)
				break;

			uint16 cbl = l[0] + (l[1] << 5);
			if ((_levelBlockProperties[cbl].flags & 7) != 7)
				continue;

			if (l[2] == 0xffff)
				continue;

			printMapText(l[2], 244, (tY << 3) + 22);

			if (l[5] == 0xffff) {
				tY++;
				continue;
			}

			uint16 cbl2 = l[3] + (l[4] << 5);
			_levelBlockProperties[cbl2].flags |= 7;
			_screen->drawShape(2, _automapShapes[l[5] << 2], (l[3] - sx) * 7 + _automapTopLeftX - 3, (l[4] - sy) * 6 + _automapTopLeftY - 3, 0, 0);
			_screen->drawShape(2, _automapShapes[l[5] << 2], 231, (tY << 3) + 19, 0, 0);
			tY++;			
		}
			
		cp = _screen->setCurPage(pageNum);

		for (int ii = 0; ii < 11; ii++) {
			if (!_defaultLegendData[ii].enable)
				continue;
			_screen->copyBlockSpecial(_screen->_curPage, 235, (tY << 3) + 21, _screen->_curPage, 235, (tY << 3) + 21, 7, 6, 0, _mapOverlay);
			_screen->drawShape(_screen->_curPage, _automapShapes[_defaultLegendData[ii].shapeIndex << 2], 232, (tY << 3) + 18 + _defaultLegendData[ii].x, 0, 0);
			printMapText(_defaultLegendData[ii].stringId, 244, (tY << 3) + 22);
			tY++;
		}

		_screen->setFont(of);
		_screen->setCurPage(cp);
	}

	printMapExitButtonText();
}

bool LoLEngine::automapProcessButtons(int inputFlag) {
	if (inputFlag != 199)
		return false;

	int r = -1;
	if (posWithinRect(_mouseX, _mouseY, 252, 175, 273, 200))
		r = 0;
	else if (posWithinRect(_mouseX, _mouseY, 231, 175, 252, 200))
		r = 1;
	else if (posWithinRect(_mouseX, _mouseY, 275, 175, 315, 197))
		r = 2;

	printMapExitButtonText();

	while (inputFlag == 199 || inputFlag == 200) {
		inputFlag = checkInput(0, false);
		removeInputTop();
		delay (_tickLength);
	}

	if (r == 0) {			
		automapForwardButton();
		printMapExitButtonText();
	} else if (r == 1) {
		automapBackButton();
		printMapExitButtonText();
	} if (r == 2) {
		return true;
	}

	return false;
}

void LoLEngine::automapForwardButton() {
	int i = (_currentMapLevel + 1) & 0x1f;
	for (; !(_hasTempDataFlags & (1 << (i - 1))); i++) {
		if (i >= 32 || i == _currentMapLevel)
			return;
	}
	
	for (int l = 0; l < 11; l++) {
		_defaultLegendData[l].enable = false;
		_defaultLegendData[l].shapeIndex = 255;
	}

	_currentMapLevel = i;
	loadLevelWallData(i, false);
	restoreBlockTempData(i);
	loadMapLegendData(i);
	_mapUpdateNeeded = true;
}

void LoLEngine::automapBackButton() {
	int i = (_currentMapLevel - 1) & 0x1f;
	for (; !(_hasTempDataFlags & (1 << (i - 1))); i--) {
		if (i < 0 || i == _currentMapLevel)
			return;
	}
	
	for (int l = 0; l < 11; l++) {
		_defaultLegendData[l].enable = false;
		_defaultLegendData[l].shapeIndex = 255;
	}

	_currentMapLevel = i;
	loadLevelWallData(i, false);
	restoreBlockTempData(i);
	loadMapLegendData(i);
	_mapUpdateNeeded = true;
}

void LoLEngine::redrawMapCursor() {
	int sx = mapGetStartPosX();
	int sy = mapGetStartPosY();

	if (_currentLevel != _currentMapLevel)
		return;

	_screen->fillRect(0, 0, 16, 16, 0, 2);
	_screen->drawShape(2, _automapShapes[48 + _currentDirection], 0, 0, 0, 0);
	int cx = _automapTopLeftX + (((_currentBlock - sx) % 32) * 7);
	int cy = _automapTopLeftY + (((_currentBlock - (sy << 5)) / 32) * 6);
	_screen->copyRegion(cx, cy, cx, cy, 16, 16, 2, 0);
	_screen->copyBlockSpecial(2, 0, 0, 0, cx - 3, cy - 2, 16, 16, 0, _mapCursorOverlay);

	_mapCursorOverlay[24] = _mapCursorOverlay[1];
	for (int i = 1; i < 24; i++)
		_mapCursorOverlay[i] = _mapCursorOverlay[i + 1];

	_screen->updateScreen();
}

void LoLEngine::drawMapBlockWall(uint16 block, uint8 wall, int x, int y, int direction) {
	if (((1 << direction) & _levelBlockProperties[block].flags) || ((_wllBuffer4[wall] & 0x1f) != 13))
		return;

	int cp = _screen->_curPage;
	_screen->copyBlockSpecial(cp, x + _mapCoords[0][direction], y + _mapCoords[1][direction], cp, x + _mapCoords[0][direction], y + _mapCoords[1][direction], _mapCoords[2][direction], _mapCoords[3][direction], 0, _mapOverlay);
	_screen->copyBlockSpecial(cp, x + _mapCoords[4][direction], y + _mapCoords[5][direction], cp, x + _mapCoords[4][direction], y + _mapCoords[5][direction], _mapCoords[8][direction], _mapCoords[9][direction], 0, _mapOverlay);
	_screen->copyBlockSpecial(cp, x + _mapCoords[6][direction], y + _mapCoords[7][direction], cp, x + _mapCoords[6][direction], y + _mapCoords[7][direction], _mapCoords[8][direction], _mapCoords[9][direction], 0, _mapOverlay);
}

void LoLEngine::drawMapShape(uint8 wall, int x, int y, int direction) {
	int l = _wllBuffer4[wall] & 0x1f;
	if (l == 0x1f)
		return;

	_screen->drawShape(_screen->_curPage, _automapShapes[(l << 2) + direction], x + _mapCoords[10][direction] - 2, y + _mapCoords[11][direction] - 2, 0, 0);
	mapIncludeLegendData(l);
}

int LoLEngine::mapGetStartPosX() {
	int c = 0;
	int a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(a << 5) + c].flags)
				break;
		}
		if (a == 32)
			c++;
	} while (c < 32 && a == 32);

	int d = 31;
	a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(a << 5) + d].flags)
				break;				
		}
		if (a == 32)
			d--;
	} while (d > 0 && a == 32);

	_automapTopLeftX = (d > c) ? ((32 - (d - c)) >> 1) * 7 + 5 : 5;
	return (d > c) ? c : 0;
}

int LoLEngine::mapGetStartPosY() {
	int c = 0;
	int a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(c << 5) + a].flags)
				break;
		}
		if (a == 32)
			c++;
	} while (c < 32 && a == 32);

	int d = 31;
	a = 32;

	do {
		for (a = 0; a < 32; a++) {
			if (_levelBlockProperties[(d << 5) + a].flags)
				break;
		}
		if (a == 32)
			d--;
	} while (d > 0 && a == 32);

	_automapTopLeftY = (d > c) ? ((32 - (d - c)) >> 1) * 6 + 4 : 4;
	return (d > c) ? c : 0;
}

void LoLEngine::mapIncludeLegendData(int type) {
	type &= 0x7f;
	for (int i = 0; i < 11; i++) {
		if (_defaultLegendData[i].shapeIndex != type)
			continue;
		_defaultLegendData[i].enable = true;
		return;
	}
}

void LoLEngine::printMapText(uint16 stringId, int x, int y) {
	int cp = _screen->setCurPage(2);
	_screen->printText(getLangString(stringId), x, y, 239, 0);
	_screen->setCurPage(cp);
}

void LoLEngine::printMapExitButtonText() {
	int cp = _screen->setCurPage(2);
	_screen->fprintString(getLangString(0x4033), 295, 182, 172, 0, 5);
	_screen->setCurPage(cp);
}

} // end of namespace Kyra

#endif // ENABLE_LOL

