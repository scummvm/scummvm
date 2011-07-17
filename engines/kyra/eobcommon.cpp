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
 */

#ifdef ENABLE_EOB

#include "kyra/loleobbase.h"
#include "kyra/resource.h"
#include "kyra/sound_intern.h"
#include "kyra/script_eob.h"
#include "kyra/timer.h"

#include "common/config-manager.h"

#include "audio/mididrv.h"
#include "audio/mixer.h"

namespace Kyra {

EobCoreEngine::EobCoreEngine(OSystem *system, const GameFlags &flags) : LolEobBaseEngine(system, flags), _numLargeItemShapes(flags.gameID == GI_EOB1 ? 14 : 11),
	_numSmallItemShapes(flags.gameID == GI_EOB1 ? 23 : 26), _numThrownItemShapes(flags.gameID == GI_EOB1 ? 12 : 9), _numItemIconShapes(flags.gameID == GI_EOB1 ? 89 : 112),
	_teleporterWallId(flags.gameID == GI_EOB1 ? 52 : 44) {
	_screen = 0;
	_gui = 0;
	//_processingButtons=false;
	//_runLoopUnk2 = 0;
	//_runLoopTimerUnk = 0;
	_playFinale = false;
	_runFlag = true;
	_configMouse = true;

	_largeItemShapes = _smallItemShapes = _thrownItemShapes = _spellShapes = _firebeamShapes = _itemIconShapes =
		_wallOfForceShapes = _teleporterShapes = _sparkShapes = _compassShapes = 0;
	_redSplatShape = _greenSplatShape = _deadCharShape = _disabledCharGrid = _blackBoxSmallGrid =
		_weaponSlotGrid = _blackBoxWideGrid = _lightningColumnShape = 0;
	_tempIconShape = 0;

	_monsterDustStrings = 0;
	_monsterDistAttType10 = 0;
	_monsterDistAttSfx10 = 0;
	_monsterDistAttType17 = 0;
	_monsterDistAttSfx17 = 0;

	_faceShapes = 0;
	_characters = 0;
	_items = 0;
	_itemTypes = 0;
	_itemNames = 0;
	_itemInHand = -1;
	_numItems = _numItemNames = 0;

	_castScrollSlot = 0;
	_currentSub = 0;

	_itemsOverlay = 0;

	_partyEffectFlags = 0;
	_lastUsedItem = 0;

	_levelDecorationRects = 0;
	_doorSwitches = 0;
	_monsterProps = 0;
	_monsterDecorations = 0;
	_monsterOvl1 = _monsterOvl2 = 0;
	_monsters = 0;
	_dstMonsterIndex = 0;
	_inflictMonsterDamageUnk = 0;

	_teleporterPulse = 0;

	_dscShapeCoords = 0;
	_dscItemPosIndex = 0;
	_dscItemShpX = 0;
	_dscItemScaleIndex = 0;
	_dscItemTileIndex = 0;
	_dscItemShapeMap = 0;
	_dscDoorScaleOffs = 0;
	_dscDoorScaleMult1 = 0;
	_dscDoorScaleMult2 = 0;
	_dscDoorScaleMult3 = 0;
	_dscDoorY1 = 0;

	_color9 = 17;
	_color10 = 23;
	_color11 = 20;

	_menuDefs = 0;

	_exchangeCharacterId = -1;
	_charExchangeSwap = 0;
	_configHpBarGraphs = true;

	memset(_dialogueLastBitmap, 0, 13);
	_dlgUnk1 = 0;
	_moveCounter = 0;
	_partyResting = false;

	_flyingObjects = 0;

	_inf = 0;
	_stepCounter = 0;
	_stepsUntilScriptCall = 0;
	_scriptTimersMode = 3;
	_currentDirection = 0;

	_openBookSpellLevel = 0;
	_openBookSpellSelectedItem = 0;
	_openBookSpellListOffset = 0;
	_openBookChar = _openBookCharBackup = 0;
	_openBookType = _openBookTypeBackup = 0;
	_openBookSpellList = 0;
	_openBookAvailableSpells = 0;
	_activeSpellCaster = 0;
	_activeSpellCasterPos = 0;
	_activeSpell = 0;
	_returnAfterSpellCallback = false;
	_spells = 0;
	_spellAnimBuffer = 0;
	_clericSpellOffset = 0;
	_restPartyElapsedTime = 0;
}

EobCoreEngine::~EobCoreEngine() {
	releaseItemsAndDecorationsShapes();
	releaseTempData();

	if (_faceShapes) {
		for (int i = 0; i < 44; i++) {
			if (_characters) {
				for (int ii = 0; ii < 6; ii++) {
					if (_characters[ii].faceShape == _faceShapes[i])
						_characters[ii].faceShape = 0;
				}
			}
			delete[] _faceShapes[i];
			_faceShapes[i] = 0;
		}
		delete[] _faceShapes;
	}

	if (_characters) {
		for (int i = 0; i < 6; i++)
			delete[] _characters[i].faceShape;
	}

	delete[] _characters;
	delete[] _items;
	delete[] _itemTypes;
	if (_itemNames) {
		for (int i = 0; i < 130; i++)
			delete _itemNames[i];
	}
	delete[] _itemNames;
	delete[] _flyingObjects;

	delete[] _monsterOvl1;
	delete[] _monsterOvl2;
	delete[] _monsters;

	if (_monsterDecorations) {
		releaseMonsterShapes(0, 36);
		delete[] _monsterShapes;
		delete[] _monsterDecorations;

		for (int i = 0; i < 24; i++)
			delete[] _monsterPalettes[i];
		delete[] _monsterPalettes;
	}

	delete[] _monsterProps;

	if (_doorSwitches) {
		releaseDoorShapes();
		delete[] _doorSwitches;
	}

	releaseDecorations();
	delete[] _levelDecorationRects;

	delete[] _spells;
	delete[] _spellAnimBuffer;

	delete _gui;
	_gui = 0;
	delete[] _menuDefs;
	_menuDefs = 0;

	delete _inf;
	delete _timer;
	_timer = 0;
}

Common::Error EobCoreEngine::init() {
	// In EOB the timer proc is directly invoked via interrupt 0x1c, 18.2 times per second.
	// This makes a tick length of 54.94.
	_tickLength = 55;

	_screen = new Screen_Eob(this, _system);

	assert(_screen);
	_screen->setResolution();

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	//MidiDriverType midiDriver = MidiDriver::detectDevice(MDT_PCSPK | MDT_ADLIB);
	_sound = new SoundAdLibPC(this, _mixer);
	assert(_sound);

	if (_sound)
		_sound->updateVolumeSettings();

	_res = new Resource(this);
	assert(_res);
	_res->reset();

	_staticres = new StaticResource(this);
	assert(_staticres);
	if (!_staticres->init())
		error("_staticres->init() failed");

	if (!screen()->init())
		error("screen()->init() failed");

	if (ConfMan.hasKey("save_slot")) {
		_gameToLoad = ConfMan.getInt("save_slot");
		if (!saveFileLoadable(_gameToLoad))
			_gameToLoad = -1;
	}

	setupKeyMap();
	_gui = new GUI_Eob(this);
	_txt = new TextDisplayer_Eob(this, _screen);
	_inf = new EobInfProcessor(this, _screen);

	_screen->loadFont(Screen::FID_6_FNT, "FONT6.FNT");
	_screen->loadFont(Screen::FID_8_FNT, "FONT8.FNT");

	readSettings();

	Common::Error err = LolEobBaseEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initButtonData();
	initMenus();
	initStaticResource();
	initSpells();

	_timer = new TimerManager(this, _system);
	assert(_timer);
	setupTimers();

	_wllVmpMap[1] = 1;
	_wllVmpMap[2] = 2;
	memset(&_wllVmpMap[3], 3, 20);
	_wllVmpMap[23] = 4;
	_wllVmpMap[24] = 5;

	memcpy(_wllWallFlags, _wllFlagPreset, _wllFlagPresetSize);

	memset(&_specialWallTypes[3], 1, 5);
	memset(&_specialWallTypes[13], 1, 5);
	_specialWallTypes[8] = _specialWallTypes[18] = 6;

	memset(&_wllShapeMap[3], -1, 5);
	memset(&_wllShapeMap[13], -1, 5);

	_wllVcnOffset = 16;

	_monsters = new EobMonsterInPlay[30];
	memset(_monsters, 0, 30 * sizeof(EobMonsterInPlay));

	_characters = new EobCharacter[6];
	memset(_characters, 0, sizeof(EobCharacter) * 6);

	_items = new EobItem[600];
	memset(_items, 0, sizeof(EobItem) * 600);

	_itemNames = new char*[130];
	for (int i = 0; i < 130; i++) {
		_itemNames[i] = new char[35];
		memset(_itemNames[i], 0, 35);
	}

	_flyingObjects = new EobFlyingObject[_numFlyingObjects];
	_flyingObjectsPtr = _flyingObjects;
	memset(_flyingObjects, 0, _numFlyingObjects * sizeof(EobFlyingObject));

	_spellAnimBuffer = new uint8[4096];
	memset(_spellAnimBuffer, 0, 4096);

	memset(_doorType, 0, sizeof(_doorType));
	memset(_noDoorSwitch, 0, sizeof(_noDoorSwitch));

	_monsterShapes = new uint8*[36];
	memset(_monsterShapes, 0, 36 * sizeof(uint8*));
	_monsterDecorations = new SpriteDecoration[36];
	memset(_monsterDecorations, 0, 36 * sizeof(SpriteDecoration));
	_monsterPalettes = new uint8*[24];
	for (int i = 0; i < 24; i++)
		_monsterPalettes[i] = new uint8[16];

	_doorSwitches = new SpriteDecoration[6];
	memset(_doorSwitches, 0, 6 * sizeof(SpriteDecoration));

	_monsterOvl1 = new uint8[16];
	_monsterOvl2 = new uint8[16];
	memset(_monsterOvl1, 15, 16 * sizeof(uint8));
	memset(_monsterOvl2, 13, 16 * sizeof(uint8));
	_monsterOvl1[0] = _monsterOvl2[0] = 0;

	return Common::kNoError;
}

Common::Error EobCoreEngine::go() {
	_txt->removePageBreakFlag();
	_screen->loadPalette("palette.col", _screen->getPalette(0));
	_screen->setScreenPalette(_screen->getPalette(0));
	_screen->setFont(Screen::FID_8_FNT);

	loadItemsAndDecorationsShapes();
	_screen->setMouseCursor(0, 0, _itemIconShapes[0]);
	_screen->showMouse();

	loadItemDefs();
	int action = 0;

	for (bool repeatLoop = true; repeatLoop; repeatLoop ^= true) {
		action = 0;

		if (_gameToLoad != -1) {
			if (loadGameState(_gameToLoad).getCode() != Common::kNoError)
				error("Couldn't load game slot %d on startup", _gameToLoad);
			startupLoad();
			_gameToLoad = -1;
		} else {
			action = mainMenu();
		}

		if (action == -1) {
			// load game
			repeatLoop = _gui->runLoadMenu(72, 14);
			if (repeatLoop && !shouldQuit())
				startupLoad();
		} else if (action == -2) {
			// new game
			repeatLoop = startCharacterGeneration();
			if (repeatLoop && !shouldQuit())
				startupNew();
		} else if (action == -3) {
			// transfer party
			repeatLoop = false;
		}
	}

	if (!shouldQuit() && action > -3) {
		runLoop();

		if (_playFinale)
			seq_playFinale();
	}

	return Common::kNoError;
}

void EobCoreEngine::registerDefaultSettings() {
	KyraEngine_v1::registerDefaultSettings();
	ConfMan.registerDefault("hpbargraphs", true);
}

void EobCoreEngine::readSettings() {
	_configHpBarGraphs = ConfMan.getBool("hpbargraphs");
	_configSounds = ConfMan.getBool("sfx_mute") ? 0 : 1;
	_configMusic = _configSounds ? 1 : 0;

	if (_sound)
		_sound->enableSFX(_configSounds);
}

void EobCoreEngine::writeSettings() {
	ConfMan.setBool("hpbargraphs", _configHpBarGraphs);
	ConfMan.setBool("sfx_mute", _configSounds == 0);

	if (_sound) {
		if (!_configSounds)
			_sound->beginFadeOut();
		_sound->enableMusic(_configSounds ? 1 : 0);
		_sound->enableSFX(_configSounds);
	}

	ConfMan.flushToDisk();
}

void EobCoreEngine::startupNew() {
	gui_setPlayFieldButtons();
	_screen->_curPage = 0;
	gui_drawPlayField(0);
	_screen->_curPage = 0;
	gui_drawAllCharPortraitsWithStats();
	drawScene(1);
	_updateFlags = 0;
	_updateCharNum = 0;
}

void EobCoreEngine::runLoop() {
	_envAudioTimer = _system->getMillis() + (rollDice(1, 10, 3) * 18 * _tickLength);	
	_flashShapeTimer = 0;
	_drawSceneTimer = _system->getMillis();
	//__unkB__ = 1;
	_screen->setFont(Screen::FID_6_FNT);
	_screen->setScreenDim(7);

	//_runLoopUnk2 = _currentBlock;
	_runFlag = true;

	while (!shouldQuit() && _runFlag) {
		//_runLoopUnk2 = _currentBlock;
		checkPartyStatus(true);
		checkInput(_activeButtons, true, 0);
		removeInputTop();

		_timer->update();
		updateScriptTimers();

		if (_sceneUpdateRequired)
			drawScene(1);

		if (_envAudioTimer >= _system->getMillis() || (_flags.gameID == GI_EOB1 && (_currentLevel == 0 || _currentLevel > 3)))
			continue;

		_envAudioTimer = _system->getMillis() + (rollDice(1, 10, 3) * 18 * _tickLength);
		snd_processEnvironmentalSoundEffect(_flags.gameID == GI_EOB1 ? 30 : (rollDice(1, 2, -1) ? 27 : 28), _currentBlock + rollDice(1, 12, -1));
		updateEnvironmentalSfx(0);
		//TODO
		//EOB1__level_2_7__turnUndead();
	}
}

bool EobCoreEngine::checkPartyStatus(bool handleDeath) {
	int numChars = 0;
	for (int i = 0; i < 6; i++)
		numChars += testCharacter(i, 13);

	if (numChars)
		return false;

	if (!handleDeath)
		return true;

	gui_drawAllCharPortraitsWithStats();
	checkPartyStatusExtra();

	if (_gui->confirmDialogue2(14, 67, 1)) {
		_screen->setFont(Screen::FID_8_FNT);
		gui_updateControls();
		if (_gui->runLoadMenu(0, 0)) {
			_screen->setFont(Screen::FID_6_FNT);
			return true;
		}
	}

	quitGame();
	return false;
}

void EobCoreEngine::loadItemsAndDecorationsShapes() {
	releaseItemsAndDecorationsShapes();
	_screen->setCurPage(2);

	_screen->loadBitmap("ITEML1.CPS", 5, 3, 0);
	_largeItemShapes = new const uint8*[_numLargeItemShapes];
	int div = (_flags.gameID == GI_EOB1) ? 3 : 8;
	int mul = (_flags.gameID == GI_EOB1) ? 64 : 24;

	for (int i = 0; i < _numLargeItemShapes; i++)
		_largeItemShapes[i] = _screen->encodeShape((i / div) << 3, (i % div) * mul, 8, 24);

	_screen->loadBitmap("ITEMS1.CPS", 5, 3, 0);
	_smallItemShapes = new const uint8*[_numSmallItemShapes];
	for (int i = 0; i < _numSmallItemShapes; i++)
		_smallItemShapes[i] = _screen->encodeShape((i / div) << 2, (i % div) * mul, 4, 24);

	_screen->loadBitmap("THROWN.CPS", 5, 3, 0);
	_thrownItemShapes = new const uint8*[_numThrownItemShapes];
	for (int i = 0; i < _numThrownItemShapes; i++)
		_thrownItemShapes[i] = _screen->encodeShape((i / div) << 2, (i % div) * mul, 4, 24);

	_spellShapes = new const uint8*[4];
	for (int i = 0; i < 4; i++)
		_spellShapes[i] = _screen->encodeShape(8, i << 5, 6, 32);

	_firebeamShapes = new const uint8*[3];
	_firebeamShapes[0] = _screen->encodeShape(16, 0, 4, 24);
	_firebeamShapes[1] = _screen->encodeShape(16, 24, 4, 24);
	_firebeamShapes[2] = _screen->encodeShape(16, 48, 3, 24);
	_redSplatShape = _screen->encodeShape(16, _flags.gameID == GI_EOB1 ? 144 : 72, 5, 24);
	_greenSplatShape = _screen->encodeShape(16, _flags.gameID == GI_EOB1 ? 168 : 96, 5, 16);

	_screen->loadBitmap("ITEMICN.CPS", 5, 3, 0);
	_itemIconShapes = new const uint8*[_numItemIconShapes];
	for (int i = 0; i < _numItemIconShapes; i++)
		_itemIconShapes[i] = _screen->encodeShape((i % 0x14) << 1, (i / 0x14) << 4, 2, 0x10);
	_tempIconShape = new uint8[300];

	_screen->loadBitmap("DECORATE.CPS", 5, 3, 0);

	if (_flags.gameID == GI_EOB2) {
		_lightningColumnShape = _screen->encodeShape(18, 88, 4, 64);
		_wallOfForceShapes = new const uint8*[6];
		for (int i = 0; i < 6; i++)
			_wallOfForceShapes[i] = _screen->encodeShape(_wallOfForceShapeDefs[(i << 2)], _wallOfForceShapeDefs[(i << 2) + 1], _wallOfForceShapeDefs[(i << 2) + 2], _wallOfForceShapeDefs[(i << 2) + 3]);
	}

	_teleporterShapes = new const uint8*[6];
	for (int i = 0; i < 6; i++)
		_teleporterShapes[i] = _screen->encodeShape(_teleporterShapeDefs[(i << 2)], _teleporterShapeDefs[(i << 2) + 1], _teleporterShapeDefs[(i << 2) + 2], _teleporterShapeDefs[(i << 2) + 3]);
	_sparkShapes = new const uint8*[3];
	_sparkShapes[0] = _screen->encodeShape(29, 0, 2, 16);
	_sparkShapes[1] = _screen->encodeShape(31, 0, 2, 16);
	_sparkShapes[2] = _screen->encodeShape(33, 0, 2, 16);
	_deadCharShape = _screen->encodeShape(0, 88, 4, 32);
	_disabledCharGrid = _screen->encodeShape(4, 88, 4, 32);
	_blackBoxSmallGrid = _screen->encodeShape(9, 88, 2, 8);
	_weaponSlotGrid = _screen->encodeShape(8, 88, 4, 16);
	_blackBoxWideGrid = _screen->encodeShape(8, 104, 4, 8);

	static const uint8 dHeight[] = { 17, 10, 10 };
	static const uint8 dY[] = { 120, 137, 147 };

	_compassShapes = new const uint8*[12];
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 4; x++)
			_compassShapes[(y << 2) + x] = _screen->encodeShape(x * 3, dY[y], 3, dHeight[y]);
	}
}

void EobCoreEngine::releaseItemsAndDecorationsShapes() {
	if (_largeItemShapes) {
		for (int i = 0; i < _numLargeItemShapes; i++) {
			if (_largeItemShapes[i])
				delete[] _largeItemShapes[i];
		}
		delete[] _largeItemShapes;
	}

	if (_smallItemShapes) {
		for (int i = 0; i < _numSmallItemShapes; i++) {
			if (_smallItemShapes[i])
				delete[] _smallItemShapes[i];
		}
		delete[] _smallItemShapes;
	}

	if (_thrownItemShapes) {
		for (int i = 0; i < _numThrownItemShapes; i++) {
			if (_thrownItemShapes[i])
				delete[] _thrownItemShapes[i];
		}
		delete[] _thrownItemShapes;
	}

	if (_spellShapes) {
		for (int i = 0; i < 4; i++) {
			if (_spellShapes[i])
				delete [] _spellShapes[i];
		}
		delete[] _spellShapes;
	}

	if (_itemIconShapes) {
		for (int i = 0; i < _numItemIconShapes; i++) {
			if (_itemIconShapes[i])
				delete[] _itemIconShapes[i];
		}
		delete[] _itemIconShapes;
	}
	delete[] _tempIconShape;

	if (_sparkShapes) {
		for (int i = 0; i < 3; i++) {
			if (_sparkShapes[i])
				delete[] _sparkShapes[i];
		}
		delete[] _sparkShapes;
	}

	if (_wallOfForceShapes) {
		for (int i = 0; i < 6; i++) {
			if (_wallOfForceShapes[i])
				delete[] _wallOfForceShapes[i];
		}
		delete[] _wallOfForceShapes;
	}

	if (_teleporterShapes) {
		for (int i = 0; i < 6; i++) {
			if (_teleporterShapes[i])
				delete[] _teleporterShapes[i];
		}
		delete[] _teleporterShapes;
	}

	if (_compassShapes) {
		for (int i = 0; i < 12; i++) {
			if (_compassShapes[i])
				delete[] _compassShapes[i];
		}
		delete[] _compassShapes;
	}

	if (_firebeamShapes) {
		for (int i = 0; i < 3; i++) {
			if (_firebeamShapes[i])
				delete[] _firebeamShapes[i];
		}
		delete []_firebeamShapes;
	}

	delete[] _deadCharShape;
	delete[] _disabledCharGrid;
	delete[] _blackBoxSmallGrid;
	delete[] _weaponSlotGrid;
	delete[] _blackBoxWideGrid;
	delete[] _lightningColumnShape;
}

void EobCoreEngine::setHandItem(Item itemIndex) {
	if (itemIndex == -1)
		return;

	if (_screen->curDimIndex() == 7 && itemIndex) {
		printFullItemName(itemIndex);
		_txt->printMessage(_takenStrings[0]);
	}

	_itemInHand = itemIndex;
	int icon = _items[_itemInHand].icon;
	const uint8 *shp = _itemIconShapes[icon];

	if (icon && (_items[_itemInHand].flags & 0x80) && ((_flags.gameID == GI_EOB2 && (_partyEffectFlags & 2)) || (_flags.gameID == GI_EOB1 && (_partyEffectFlags & 0x10000)))) {
		memcpy(_tempIconShape, shp, 300);
		if (_flags.gameID == GI_EOB1)
			_screen->replaceShapePalette(_tempIconShape, &_itemsOverlay[icon << 4]);
		else
			_screen->applyShapeOverlay(_tempIconShape, 3);
		shp = _tempIconShape;
	}

	int mouseOffs = itemIndex ? 8 : 0;
	_screen->setMouseCursor(mouseOffs, mouseOffs, shp);
}

int EobCoreEngine::getDexterityArmorClassModifier(int dexterity) {
	static const int mod[] = { 5, 5, 5, 4, 3, 2, 1, 0, 0,
		0, 0, 0, 0, 0, 0, -1, -2, -3, -4, -4, -5, -5, -5, -6, -6 };
	return mod[dexterity];
}

int EobCoreEngine::generateCharacterHitpointsByLevel(int charIndex, int levelIndex) {
	EobCharacter *c = &_characters[charIndex];
	int m = getClassAndConstHitpointsModifier(c->cClass, c->constitutionCur);

	int h = 0;

	for (int i = 0; i < 3; i++) {
		if (!(levelIndex & (1 << i)))
			continue;

		int d = getCharacterClassType(c->cClass, i);

		if (c->level[i] <= _hpIncrPerLevel[6 + i])
			h += rollDice(1, (d >= 0) ? _hpIncrPerLevel[d] : 0);
		else
			h += _hpIncrPerLevel[12 + i];

		h += m;
	}

	h /= _numLevelsPerClass[c->cClass];

	if (h < 1)
		h = 1;

	return h;
}

int EobCoreEngine::getClassAndConstHitpointsModifier(int cclass, int constitution) {
	int res = _hpConstModifiers[constitution];

	if (res <= 2 || (_classModifierFlags[cclass] & 0x31))
		return res;

	return 2;
}

int EobCoreEngine::getCharacterClassType(int cclass, int levelIndex) {
	return _characterClassType[cclass * 3 + levelIndex];
}

int EobCoreEngine::getModifiedHpLimits(int hpModifier, int constModifier, int level, bool mode) {
	int s = _hpIncrPerLevel[6 + hpModifier] > level ? level : _hpIncrPerLevel[6 + hpModifier];
	int res = s;

	if (!mode)
		res *= (hpModifier >= 0 ? _hpIncrPerLevel[hpModifier] : 0);

	if (level > s) {
		s = level - s;
		res += (s * _hpIncrPerLevel[12 + hpModifier]);
	}

	if (!mode || (constModifier > 0))
		res += (level * constModifier);

	return res;
}

Common::String EobCoreEngine::getCharStrength(int str, int strExt) {
	if (strExt) {
		if (strExt == 100)
			strExt = 0;
		_strenghtStr = Common::String::format("%d/%02d", str, strExt);
	} else {
		_strenghtStr = Common::String::format("%d", str);
	}

	return _strenghtStr;
}

int EobCoreEngine::testCharacter(int index, int flags) {
	if (index == -1)
		return 0;

	EobCharacter *c = &_characters[index];
	int res = 1;

	if (flags & 1)
		res &= (c->flags & 1);

	if (flags & 2)
		res &= ((c->hitPointsCur <= -10) || (c->flags & 8)) ? 0 : 1;

	if (flags & 4)
		res &= ((c->hitPointsCur <= 0) || (c->flags & 8)) ? 0 : 1;

	if (flags & 8)
		res &= (c->flags & 12) ? 0 : 1;

	if (flags & 0x20)
		res &= (c->flags & 4) ? 0 : 1;

	if (flags & 0x10)
		res &= (c->flags & 2) ? 0 : 1;

	if (flags & 0x40)
		res &= (c->food <= 0) ? 0 : 1;

	return res;
}

int EobCoreEngine::getNextValidCharIndex(int curCharIndex, int searchStep) {
	do {
		curCharIndex += searchStep;
		if (curCharIndex < 0)
			curCharIndex = 5;
		if (curCharIndex > 5)
			curCharIndex = 0;
	} while (!testCharacter(curCharIndex, 1));

	return curCharIndex;
}

void EobCoreEngine::recalcArmorClass(int index) {
	EobCharacter *c = &_characters[index];
	int acm = getDexterityArmorClassModifier(c->dexterityCur);
	c->armorClass = 10 + acm;

	static uint8 slot[] = { 17, 0, 1, 18 };
	for (int i = 0; i < 4; i++) {
		int itm = c->inventory[slot[i]];
		if (!itm)
			continue;

		if (i == 2) {
			if (!validateWeaponSlotItem(index, 1))
				continue;
		}

		int tp = _items[itm].type;

		if (!(_itemTypes[tp].allowedClasses & _classModifierFlags[c->cClass]) || (_itemTypes[tp].extraProperties & 0x7f) || (i >= 1 && i <= 2 && tp != 27 && !(_flags.gameID == GI_EOB2 && tp == 57)))
			continue;

		c->armorClass += _itemTypes[tp].armorClass;
		c->armorClass -= _items[itm].value;
	}

	if (!_items[c->inventory[17]].value) {
		int8 m1 = 0;
		int8 m2 = 0;

		if (c->inventory[25]) {
			if (!(_itemTypes[_items[c->inventory[25]].type].extraProperties & 0x7f))
				m1 = _items[c->inventory[25]].value;
		}

		if (c->inventory[26]) {
			if (!(_itemTypes[_items[c->inventory[26]].type].extraProperties & 0x7f))
				m2 = _items[c->inventory[26]].value;
		}

		c->armorClass -= MAX(m1, m2);
	}

	if (c->effectsRemainder[0] > 0) {
		if (c->armorClass <= (acm + 6))
			c->effectsRemainder[0] = 0;
		else
			c->armorClass = (acm + 6);
	}

	// shield
	if ((c->effectFlags & 8) && (c->armorClass > 4))
		c->armorClass = 4;

	// magical vestment
	if (c->effectFlags & 0x4000) {
		int8 m1 = 5;

		if (getCharacterClericPaladinLevel(index) > 5)
			m1 += ((getCharacterClericPaladinLevel(index) - 5) / 3);

		if (c->armorClass > m1)
			c->armorClass = m1;
	}

	if (c->armorClass < -10)
		c->armorClass = -10;
}

int EobCoreEngine::validateWeaponSlotItem(int index, int slot) {
	EobCharacter *c = &_characters[index];
	int itm1 = c->inventory[0];
	int r = itemUsableByCharacter(index, itm1);
	int tp1 = _items[itm1].type;

	if (!slot)
		return (!itm1 || r) ? 1 : 0;

	int itm2 = c->inventory[1];
	r = itemUsableByCharacter(index, itm2);
	int tp2 = _items[itm2].type;

	if (itm1 && _itemTypes[tp1].requiredHands == 2)
		return 0;

	if (!itm2)
		return 1;

	int f = (_itemTypes[tp2].extraProperties & 0x7f);
	if (f <= 0 || f > 3)
		return r;

	if (_itemTypes[tp2].requiredHands)
		return 0;

	return r;
}

int EobCoreEngine::getCharacterClericPaladinLevel(int index) {
	if (_castScrollSlot)
		return 9;

	if (index == -1)
		return (_currentLevel < 7) ? 5 : 9;

	int l = getCharacterLevelIndex(2, _characters[index].cClass);
	if (l > -1)
		return _characters[index].level[l];

	l = getCharacterLevelIndex(4, _characters[index].cClass);
	if (l > -1) {
		if (_characters[index].level[l] > 8)
			return _characters[index].level[l] - 8;
	}

	return 1;
}

int EobCoreEngine::getCharacterMageLevel(int index) {
	if (_castScrollSlot)
		return 9;

	if (index == -1)
		return (_currentLevel < 7) ? 5 : 9;

	int l = getCharacterLevelIndex(1, _characters[index].cClass);
	return (l > -1) ? _characters[index].level[l] : 1;
}

int EobCoreEngine::getCharacterLevelIndex(int type, int cClass) {
	if (getCharacterClassType(cClass, 0) == type)
		return 0;

	if (getCharacterClassType(cClass, 1) == type)
		return 1;

	if (getCharacterClassType(cClass, 2) == type)
		return 2;

	return -1;
}

int EobCoreEngine::countCharactersWithSpecificItems(int16 itemType, int16 itemValue) {
	int res = 0;
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (checkInventoryForItem(i, itemType, itemValue) != -1)
			res++;
	}
	return res;
}

int EobCoreEngine::checkInventoryForItem(int character, int16 itemType, int16 itemValue) {
	for (int i = 0; i < 27; i++) {
		uint16 inv = _characters[character].inventory[i];
		if (!inv)
			continue;
		if (_items[inv].type != itemType && itemType != -1)
			continue;
		if (_items[inv].value == itemValue || itemValue == -1)
			return i;
	}
	return -1;
}

void EobCoreEngine::modifyCharacterHitpoints(int character, int16 points) {
	if (!testCharacter(character, 3))
		return;

	EobCharacter *c = &_characters[character];
	c->hitPointsCur += points;
	if (c->hitPointsCur > c->hitPointsMax)
		c->hitPointsCur = c->hitPointsMax;

	gui_drawHitpoints(character);
	gui_drawCharPortraitWithStats(character);
}

void EobCoreEngine::neutralizePoison(int character) {
	_characters[character].flags &= ~2;
	_characters[character].effectFlags &= ~0x2000;
	deleteCharEventTimer(character, -34);
	gui_drawCharPortraitWithStats(character);
}

void EobCoreEngine::initNpc(int npcIndex) {
	EobCharacter *c = _characters;
	int i = 0;
	for (; i < 6; i++) {
		if (!(_characters[i].flags & 1)) {
			c = &_characters[i];
			break;
		}
	}

	delete[] c->faceShape;
	memcpy(c, &_npcPreset[npcIndex], sizeof(EobCharacter));
	recalcArmorClass(i);

	for (i = 0; i < 25; i++) {
		if (!c->inventory[i])
			continue;
		c->inventory[i] = duplicateItem(c->inventory[i]);
	}

	_screen->loadEobBitmap(_flags.gameID == GI_EOB2 ? "OUTPORTS" : "OUTTAKE", 3, 3);
	_screen->_curPage = 2;
	c->faceShape = _screen->encodeShape(npcIndex << 2, _flags.gameID == GI_EOB2 ? 0 : 160, 4, 32, true);
	_screen->_curPage = 0;
}

int EobCoreEngine::npcJoinDialogue(int npcIndex, int queryJoinTextId, int confirmJoinTextId, int noJoinTextId) {
	gui_drawDialogueBox();
	_txt->printDialogueText(queryJoinTextId, 0);

	int r = runDialogue(-1, 0, _yesNoStrings[0], _yesNoStrings[1]) - 1;
	if (r == 0) {
		if (confirmJoinTextId == -1) {
			Common::String tmp = Common::String::format(_npcJoinStrings[0], _npcPreset[npcIndex].name);
			_txt->printDialogueText(tmp.c_str(), true);
		} else {
			_txt->printDialogueText(confirmJoinTextId, _okStrings[0]);
		}

		if (prepareForNewPartyMember(33, npcIndex + 1))
			initNpc(npcIndex);

	} else if (r == 1) {
		_txt->printDialogueText(noJoinTextId, _okStrings[0]);
	}

	return r ^ 1;
}

int EobCoreEngine::prepareForNewPartyMember(int16 itemType, int16 itemValue) {
	int numChars = 0;
	for (int i = 0; i < 6; i++)
		numChars += (_characters[i].flags & 1);

	if (numChars < 6) {
		deletePartyItem(itemType, itemValue);
	} else {
		gui_drawDialogueBox();
		_txt->printDialogueText(_npcMaxStrings[0]);
		int r = runDialogue(-1, 1, _characters[0].name, _characters[1].name, _characters[2].name, _characters[3].name,
			_characters[4].name, _characters[5].name, _abortStrings[0], 0, 0) - 1;

		if (r == 6)
			return 0;

		deletePartyItem(itemType, itemValue);
		removeCharacterFromParty(r);
	}

	return 1;
}

void EobCoreEngine::dropCharacter(int charIndex) {
	if (!testCharacter(charIndex, 1))
		return;

	removeCharacterFromParty(charIndex);
	
	if (charIndex < 5)
		exchangeCharacters(charIndex, testCharacter(5, 1) ? 5 : 4);

	gui_processCharPortraitClick(0);
	gui_setPlayFieldButtons();
	setupCharacterTimers();
}

void EobCoreEngine::removeCharacterFromParty(int charIndex) {
	EobCharacter *c = &_characters[charIndex];
	c->flags = 0;

	for (int i = 0; i < 27; i++) {
		if (i == 16 || !c->inventory[i])
			continue;

		setItemPosition((Item*)&_levelBlockProperties[_currentBlock & 0x3ff].drawObjects, _currentBlock, c->inventory[i], _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);
		c->inventory[i] = 0;
	}

	while (c->inventory[16])
		setItemPosition((Item*)&_levelBlockProperties[_currentBlock & 0x3ff].drawObjects, _currentBlock, getQueuedItem(&c->inventory[16], 0, -1), _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);

	c->inventory[16] = 0;

	if (_updateCharNum == charIndex)
		_updateCharNum = 0;

	setupCharacterTimers();
}

void EobCoreEngine::exchangeCharacters(int charIndex1, int charIndex2) {
	EobCharacter temp;
	memcpy(&temp, &_characters[charIndex1], sizeof(EobCharacter));
	memcpy(&_characters[charIndex1], &_characters[charIndex2], sizeof(EobCharacter));
	memcpy(&_characters[charIndex2], &temp, sizeof(EobCharacter));
}

void EobCoreEngine::increasePartyExperience(int16 points) {
	int cnt = 0;
	for (int i = 0; i < 6; i++) {
		if (testCharacter(i, 3))
			cnt++;
	}

	if (cnt <= 0)
		return;

	points /= cnt;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		increaseCharacterExperience(i, points);
	}
}

void EobCoreEngine::increaseCharacterExperience(int charIndex, int32 points) {
	int cl = _characters[charIndex].cClass;
	points /= _numLevelsPerClass[cl];

	for (int i = 0; i < 3; i++) {
		if (getCharacterClassType(cl, i) == -1)
			continue;
		_characters[charIndex].experience[i] += points;

		uint32 er = getRequiredExperience(cl, i, _characters[charIndex].level[i] + 1);
		if (er == 0xffffffff)
			continue;

		if (_characters[charIndex].experience[i] >= er)
			increaseCharacterLevel(charIndex, i);
	}
}

uint32 EobCoreEngine::getRequiredExperience(int cClass, int levelIndex, int level) {
	cClass = getCharacterClassType(cClass, levelIndex);
	if (cClass == -1)
		return 0xffffffff;

	const uint32 *tbl = _expRequirementTables[cClass];
	return tbl[level - 1];
}

void EobCoreEngine::increaseCharacterLevel(int charIndex, int levelIndex) {
	_characters[charIndex].level[levelIndex]++;
	int hpInc = generateCharacterHitpointsByLevel(charIndex, levelIndex);
	_characters[charIndex].hitPointsCur += hpInc;
	_characters[charIndex].hitPointsMax += hpInc;

	gui_drawCharPortraitWithStats(charIndex);
	_txt->printMessage(_levelGainStrings[0], -1, _characters[charIndex].name);
	snd_playSoundEffect(23);
}

void EobCoreEngine::setWeaponSlotStatus(int charIndex, int mode, int slot) {
	if (mode == 0 || mode == 2)
		_characters[charIndex].disabledSlots ^= (1 << slot);
	else if (mode != 1)
		return;

	_characters[charIndex].slotStatus[slot] = 0;
	gui_drawCharPortraitWithStats(charIndex);
}

void EobCoreEngine::setupDialogueButtons(int presetfirst, int numStr, const char *str1, ...) {
	_dialogueNumButtons = numStr;
	_dialogueButtonString[0] = str1;
	_dialogueHighlightedButton = 0;

	va_list args;
	va_start(args, str1);
	const char **s5p = va_arg(args, const char**);
	va_end(args);
	for (int i = 1; i < numStr; i++) {
		if (s5p[i - 1])
			_dialogueButtonString[i] = s5p[i - 1];
		else
			_dialogueNumButtons = numStr = i;
	}

	static const uint16 prsX[] = { 59, 166, 4, 112, 220, 4, 112, 220, 4, 112, 220, 4, 112, 220 };
	static const uint8 prsY[] = { 0, 0, 0, 0, 0, 12, 12, 12, 24, 24, 24, 36, 36, 36 };

	const ScreenDim *dm = screen()->_curDim;
	int yOffs = (_txt->lineCount() + 1) * _screen->getFontHeight() + dm->sy + 4;

	_dialogueButtonPosX = &prsX[presetfirst];
	_dialogueButtonPosY = &prsY[presetfirst];
	_dialogueButtonYoffs = yOffs;

	drawDialogueButtons();

	if (!shouldQuit())
		removeInputTop();
}

void EobCoreEngine::initDialogueSequence() {
	_dlgUnk1 = -1;
	_txt->setWaitButtonMode(0);
	_dialogueField = true;

	_dialogueLastBitmap[0] = 0;

	_txt->resetPageBreakString();
	gui_updateControls();

	_sound->playTrack(0);
	Common::SeekableReadStream *s = _res->createReadStream("TEXT.DAT");
	_screen->loadFileDataToPage(s, 5, 32000);
	_txt->setupField(9, 0);
	delete s;
}

void EobCoreEngine::restoreAfterDialogueSequence() {
	_txt->allowPageBreak(false);
	_dialogueField = false;

	_dialogueLastBitmap[0] = 0;

	gui_restorePlayField();
	_screen->setScreenDim(7);

	if (_flags.gameID == GI_EOB2)
		_sound->playTrack(2);

	_sceneUpdateRequired = true;
}

void EobCoreEngine::drawSequenceBitmap(const char *file, int destRect, int x1, int y1, int flags) {
	static const uint8 frameX[] = { 1, 0 };
	static const uint8 frameY[] = { 8, 0 };
	static const uint8 frameW[] = { 20, 40 };
	static const uint8 frameH[] = { 96, 121 };

	int page = ((flags & 2) || destRect) ? 0 : 6;

	if (scumm_stricmp(_dialogueLastBitmap, file)) {
		if (!destRect) {
			if (!(flags & 1)) {
				_screen->loadEobCpsFileToPage("BORDER", 0, 3, 3, 2);
				_screen->copyRegion(0, 0, 0, 0, 184, 121, 2, page, Screen::CR_NO_P_CHECK);
			} else {
				_screen->copyRegion(0, 0, 0, 0, 184, 121, 0, page, Screen::CR_NO_P_CHECK);
			}

			if (!page)
				_screen->copyRegion(0, 0, 0, 0, 184, 121, 2, 6, Screen::CR_NO_P_CHECK);
		}

		_screen->loadEobCpsFileToPage(file, 0, 3, 3, 2);
		strcpy(_dialogueLastBitmap, file);
	}

	if (flags & 2)
		_screen->crossFadeRegion(x1 << 3, y1, frameX[destRect] << 3, frameY[destRect], frameW[destRect] << 3, frameH[destRect], 2, page);
	else
		_screen->copyRegion(x1 << 3, y1, frameX[destRect] << 3, frameY[destRect], frameW[destRect] << 3, frameH[destRect], 2, page, Screen::CR_NO_P_CHECK);

	if (page == 6)
		_screen->copyRegion(0, 0, 0, 0, 184, 121, 6, 0, Screen::CR_NO_P_CHECK);

	_screen->updateScreen();
}

int EobCoreEngine::runDialogue(int dialogueTextId, int style, const char *button1, ...) {
	if (dialogueTextId != -1)
		txt()->printDialogueText(dialogueTextId, 0);

	va_list args;
	va_start(args, button1);
	if (style)
		setupDialogueButtons(2, 9, button1, args);
	else
		setupDialogueButtons(0, 2, button1, args);
	va_end(args);

	int res = 0;
	while (res == 0 && !shouldQuit())
		res = processDialogue();

	gui_drawDialogueBox();

	return res;
}

void EobCoreEngine::restParty_displayWarning(const char *str) {
	int od = _screen->curDimIndex();
	_screen->setScreenDim(7);
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);
	_screen->setCurPage(0);

	_txt->printMessage(Common::String::format("\r%s\r", str).c_str());

	_screen->setFont(of);
	_screen->setScreenDim(od);
}

bool EobCoreEngine::restParty_updateMonsters() {
	bool sfxEnabled = _sound->sfxEnabled();
	bool musicEnabled = _sound->musicEnabled();
	_sound->enableSFX(false);
	_sound->enableMusic(false);

	for (int i = 0; i < 5; i++) {
		_partyResting = true;
		Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);
		int od = _screen->curDimIndex();
		_screen->setScreenDim(7);
		updateMonsters(0);
		updateMonsters(1);
		timerProcessFlyingObjects(0);
		_screen->setScreenDim(od);
		_screen->setFont(of);
		_partyResting = false;

		for (int ii = 0; ii < 30; ii++) {
			if (_monsters[ii].mode == 8)
				continue;
			if (getBlockDistance(_currentBlock, _monsters[ii].block) >= 2)
				continue;

			restParty_displayWarning(_menuStringsRest4[0]);
			_sound->enableSFX(sfxEnabled);
			_sound->enableMusic(musicEnabled);
			return true;
		}
	}
	
	_sound->enableSFX(sfxEnabled);
	_sound->enableMusic(musicEnabled);
	return false;
}

int EobCoreEngine::restParty_getCharacterWithLowestHp() {
	int lhp = 900;
	int res = -1;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		if (_characters[i].hitPointsCur >= _characters[i].hitPointsMax)
			continue;
		if (_characters[i].hitPointsCur < lhp) {
			lhp = _characters[i].hitPointsCur;
			res = i;
		}
	}

	return res + 1;
}

bool EobCoreEngine::restParty_checkHealSpells(int charIndex) {
	static const uint8 eob1healSpells[] = { 2, 15, 20 };
	static const uint8 eob2healSpells[] = { 3, 16, 20 };
	const uint8 *spells = _flags.gameID == GI_EOB1 ? eob1healSpells : eob2healSpells;
	const int8 *list = _characters[charIndex].clericSpells;
	
	for (int i = 0; i < 80; i++) {
		int s = list[i] < 0 ? -list[i] : list[i];
		if (s == spells[0] || s == spells[1] || s == spells[2])
			return true;
	}

	return false;
}

bool EobCoreEngine::restParty_checkSpellsToLearn() {
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 0x43))
			continue;

		if ((getCharacterLevelIndex(2, _characters[i].cClass) != -1 || getCharacterLevelIndex(4, _characters[i].cClass) != -1) && (checkInventoryForItem(i, 30, -1) != -1)) {
			for (int ii = 0; ii < 80; ii++) {
				if (_characters[i].clericSpells[ii] < 0)
					return true;
			}
		}

		if ((getCharacterLevelIndex(1, _characters[i].cClass) != -1) && (checkInventoryForItem(i, 29, -1) != -1)) {
			for (int ii = 0; ii < 80; ii++) {
				if (_characters[i].mageSpells[ii] < 0)
					return true;
			}
		}
	}

	return false;
}

void EobCoreEngine::restParty_npc() {

}

bool EobCoreEngine::restParty_extraAbortCondition() {
	return false;
}

void EobCoreEngine::delay(uint32 millis, bool, bool) {
	while (millis && !shouldQuit() && !skipFlag()) {
		updateInput();
		uint32 step = MIN<uint32>(millis, (_tickLength / 5));
		_system->delayMillis(step);
		millis -= step;
	}
}

void EobCoreEngine::displayParchment(int id) {
	_txt->setWaitButtonMode(1);
	_txt->resetPageBreakString();
	gui_updateControls();

	if (id >= 0) {
		// display text
		Common::SeekableReadStream *s = _res->createReadStream("TEXT.DAT");
		_screen->loadFileDataToPage(s, 5, 32000);
		gui_drawBox(0, 0, 176, 175, _color1_1, _color2_1, _bkgColor_1);
		_txt->setupField(12, 1);
		if (_flags.gameID == GI_EOB2)
			id++;
		_txt->printDialogueText(id, _okStrings[0]);

	} else {
		// display bitmap
		id = -id - 1;
		static const uint8 x[] = { 0, 20, 0 };
		static const uint8 y[] = { 0, 0, 96 };
		drawSequenceBitmap("MAP", 0, x[id], y[id], 0);

		removeInputTop();
		while (!shouldQuit()) {
			delay(_tickLength);
			if (checkInput(0, false, 0) & 0xff)
				break;
			removeInputTop();
		}
		removeInputTop();
	}

	restoreAfterDialogueSequence();
}

void EobCoreEngine::useSlotWeapon(int charIndex, int slotIndex, int item) {
	EobCharacter *c = &_characters[charIndex];
	int tp = item ? _items[item].type : 0;

	if (c->effectFlags & 0x40)
		removeCharacterEffect(10, charIndex, 1); // remove invisibility effect

	int ep = _itemTypes[tp].extraProperties & 0x7f;
	int8 inflict = 0;

	if (ep == 1) {
		inflict = closeDistanceAttack(charIndex, item);
		if (!inflict)
			inflict = -1;
		snd_playSoundEffect(32);
	} else if (ep == 2) {
		inflict = thrownAttack(charIndex, slotIndex, item);
	} else if (ep == 3) {
		inflict = bowAttack(charIndex, item);
		gui_drawCharPortraitWithStats(charIndex);
	}

	if (inflict > 0) {
		if (_items[item].flags & 8) {
			c->hitPointsCur += inflict;
			gui_drawCharPortraitWithStats(charIndex);
		}

		if (_items[item].flags & 0x10)
			c->inventory[slotIndex] = 0;

		inflictMonsterDamage(&_monsters[_dstMonsterIndex], inflict, true);
	}

	c->disabledSlots ^= (1 << slotIndex);
	c->slotStatus[slotIndex] = inflict;

	gui_drawCharPortraitWithStats(charIndex);
	setCharEventTimer(charIndex, 18, inflict >= -2 ? slotIndex + 2 : slotIndex, 1);
}

int EobCoreEngine::closeDistanceAttack(int charIndex, int item) {
	if (charIndex > 1)
		return -3;

	uint16 d = calcNewBlockPosition(_currentBlock, _currentDirection);
	int r = getClosestMonsterPos(charIndex, d);

	if (r == -1) {
		uint8 w = _specialWallTypes[_levelBlockProperties[d].walls[_sceneDrawVarDown]];
		if (w == 0xff) {
			if (_flags.gameID == GI_EOB1) {
				_levelBlockProperties[d].walls[_sceneDrawVarDown]++;
				_levelBlockProperties[d].walls[_sceneDrawVarDown ^ 2]++;

			} else {
				for (int i = 0; i < 4; i++) {
					if (_specialWallTypes[_levelBlockProperties[d].walls[i]] == 0xff)
						_levelBlockProperties[d].walls[i]++;
				}
			}
			_sceneUpdateRequired = true;

		} else if ((_flags.gameID == GI_EOB1) || (_flags.gameID == GI_EOB2 && w != 8 && w != 9)) {
			return -1;
		}

		return (_flags.gameID == GI_EOB2 && ((_itemTypes[_items[item].type].allowedClasses & 4) || !item)) ? -5 : -2;

	} else {
		if (_monsters[r].flags & 0x20) {
			killMonster(&_monsters[r], 1);
			_txt->printMessage(_monsterDustStrings[0]);
			return -2;
		}

		if (!characterAttackHitTest(charIndex, r, item, 1))
			return -1;

		uint16 flg = 0x100;

		if ((_flags.gameID == GI_EOB1 && _items[item].type > 51 && _items[item].type < 57) || (_flags.gameID == GI_EOB2 && isMagicWeapon(item)))
			flg |= 1;

		_dstMonsterIndex = r;
		return calcCloseDistanceMonsterDamage(&_monsters[r], charIndex, item, 1, flg, 5, 3);
	}

	return 0;
}

int EobCoreEngine::thrownAttack(int charIndex, int slotIndex, int item) {
	int d = charIndex > 3 ? charIndex - 2 : charIndex;
	if (!launchObject(charIndex, item, _currentBlock, _dropItemDirIndex[(_currentDirection << 2) + d], _currentDirection, _items[item].type))
		return 0;

	snd_playSoundEffect(11);
	_characters[charIndex].inventory[slotIndex] = 0;
	reloadWeaponSlot(charIndex, slotIndex, -1, 0);
	_sceneUpdateRequired = true;
	return 0;
}

int EobCoreEngine::bowAttack(int charIndex, int item) {
	return 0;
}

void EobCoreEngine::inflictMonsterDamage(EobMonsterInPlay *m, int damage, bool giveExperience) {
	m->hitPointsCur -= damage;
	m->flags = (m->flags & 0xf7) | 1;

	if (_monsterProps[m->type].flags & 0x2000) {
		explodeMonster(m);
		checkSceneUpdateNeed(m->block);
		m->hitPointsCur = 0;
	} else {
		if (checkSceneUpdateNeed(m->block)) {
			m->flags |= 2;
			if (_inflictMonsterDamageUnk)
				return;
			flashMonsterShape(m);
		}
	}

	if (m->hitPointsCur <= 0)
		killMonster(m, giveExperience);
	else if (getBlockDistance(m->block, _currentBlock) < 4)
		m->dest = _currentBlock;
}

void EobCoreEngine::calcAndInflictMonsterDamage(EobMonsterInPlay *m, int times, int pips, int offs, int flags, int b, int damageType) {
	int dmg = calcCloseDistanceMonsterDamage(m, times, pips, offs, flags, b, damageType);
	if (dmg > 0)
		inflictMonsterDamage(m, dmg, flags & 0x800 ? true : false);
}

void EobCoreEngine::calcAndInflictCharacterDamage(int charIndex, int times, int itemOrPips, int useStrModifierOrBase, int flg, int a, int damageType) {
	int dmg = calcCharacterDamage(charIndex, times, itemOrPips, useStrModifierOrBase, flg, a, damageType);
	if (dmg)
		inflictCharacterDamage(charIndex, dmg);
}

int EobCoreEngine::calcCharacterDamage(int charIndex, int times, int itemOrPips, int useStrModifierOrBase, int flg, int a, int damageType) {
	int s = (flg & 0x100) ? calcDamageModifers(times, 0, itemOrPips, _items[itemOrPips].type, useStrModifierOrBase) : rollDice(times, itemOrPips, useStrModifierOrBase);
	EobCharacter *c = &_characters[charIndex];

	if (a != 5) {
		if (checkUnkConstModifiers(c, _charClassModUnk[c->cClass], c->level[0], a, c->raceSex))
			s = recalcDamageModifier(damageType, s);
	}

	if ((flg & 0x110) == 0x110) {
		if (!calcDamageCheckItemType(_items[itemOrPips].type))
			s = 1;
	}

	if (flg & 4) {
		if (checkInventoryForRings(charIndex, 3))
			s = 0;
	}

	if (flg & 0x400) {
		if (c->effectFlags & 0x2000)
			s = 0;
		else
			_txt->printMessage(_characterStatusStrings8[0], -1, c->name);
	}

	return s;
}

void EobCoreEngine::inflictCharacterDamage(int charIndex, int damage) {
	EobCharacter *c = &_characters[charIndex];
	if (!testCharacter(charIndex, 3))
		return;

	if (c->effectsRemainder[3])
		c->effectsRemainder[3] = (damage < c->effectsRemainder[3]) ? (c->effectsRemainder[3] - damage) : 0;

	c->hitPointsCur -= damage;
	c->damageTaken = damage;

	if (c->hitPointsCur > -10) {
		snd_playSoundEffect(21);
	} else {
		c->hitPointsCur = -10;
		c->flags &= 1;
		c->food = 0;
		removeAllCharacterEffects(charIndex);
		snd_playSoundEffect(22);
	}

	if (c->effectsRemainder[0]) {
		c->effectsRemainder[0] = (damage < c->effectsRemainder[0]) ? (c->effectsRemainder[0] - damage) : 0;
		if (!c->effectsRemainder[0])
			removeCharacterEffect(1, charIndex, 1);
	}

	if (_currentControlMode)
		gui_drawFaceShape(charIndex);
	else
		gui_drawCharPortraitWithStats(charIndex);

	if (c->hitPointsCur <= 0 && _updateFlags == 1 && charIndex == _openBookChar) {
		Button b;
		clickedSpellbookAbort(&b);
	}

	setCharEventTimer(charIndex, 18, 6, 1);
}

bool EobCoreEngine::characterAttackHitTest(int charIndex, int monsterIndex, int item, int attackType) {
	if (charIndex < 0)
		return true;

	int p = item ? (_flags.gameID == GI_EOB1 ? _items[item].type : (_itemTypes[_items[item].type].extraProperties & 0x7f)) : 0;

	if (_monsters[monsterIndex].flags & 0x20)
		return true;// EOB 2 only ?

	int t = _monsters[monsterIndex].type;
	int d = (p < 1 || p > 3) ? 0 : _items[item].value;

	if (_flags.gameID == GI_EOB2) {
		if ((p > 0 && p < 4) || !item ){
			if (((_monsterProps[t].statusFlags & 0x200) && (d <= 0)) || ((_monsterProps[t].statusFlags & 0x1000) && (d <= 1)))
				return false;
		}
	}

	d += (attackType ? getStrHitChanceModifier(charIndex) : getDexHitChanceModifier(charIndex));

	int m = getMonsterAcHitChanceModifier(charIndex, _monsterProps[t].armorClass) - d;
	int s = rollDice(1, 20);

	_monsters[monsterIndex].flags |= 1;

	if (_flags.gameID == GI_EOB1) {
		if (_partyEffectFlags & 0x30)
			s++;
		if (_characters[charIndex].effectFlags & 0x40)
			s++;
	} else if ((_partyEffectFlags & 0x8400) || (_characters[charIndex].effectFlags & 0x1000)) {
		s++;
	}

	s = CLIP(s, 1, 20);

	return s < m ? false : true;
}

bool EobCoreEngine::monsterAttackHitTest(EobMonsterInPlay *m, int charIndex) {
	int tp = m->type;
	EobMonsterProperty *p = &_monsterProps[tp];

	int r = rollDice(1, 20);
	if (r != 20) {
		if (_characters[charIndex].effectFlags & 0x800)
			r -= 2;
		if (_characters[charIndex].effectFlags & 0x10)
			r -= 2;
		if (_partyEffectFlags & 0x8000)
			r--;
	}

	return ((r == 20) || (r >= (p->hitChance - _characters[charIndex].armorClass)));
}

bool EobCoreEngine::flyingObjectMonsterHit(EobFlyingObject *fo, int monsterIndex) {
	if (fo->attackerId != -1) {
		if (!characterAttackHitTest(fo->attackerId, monsterIndex, fo->item, 0))
			return false;
	}
	calcAndInflictMonsterDamage(&_monsters[monsterIndex], fo->attackerId, fo->item, 0, (fo->attackerId == -1) ? 0x110: 0x910, 5, 3);
	return true;
}

bool EobCoreEngine::flyingObjectPartyHit(EobFlyingObject *fo) {
	int ps = _dscItemPosIndex[(_currentDirection << 2) + (_items[fo->item].pos & 3)];
	bool res = false;

	bool b = ((_currentDirection == fo->direction || _currentDirection == (fo->direction ^ 2)) && ps > 2);
	int s = ps << 1;
	if (ps > 2)
		s += rollDice(1, 2, -1);

	static const int8 charId[] = { 0, -1, 1, -1, 2, 4, 3, 5 };

	for (int i = 0; i < 2; i++) {
		int c = charId[s];
		s ^= 1;
		if (!testCharacter(c, 3))
			continue;
		calcAndInflictCharacterDamage(c, -1, fo->item, 0, 0x110, 5, 3);
		res = true;
		if (ps < 2 || b == 0)
			break;
	}

	return res;
}

void EobCoreEngine::monsterCloseAttack(EobMonsterInPlay *m) {
	int first = _monsterCloseAttDstTable1[(_currentDirection << 2) + m->dir] * 12;
	int v = (m->pos == 4) ? rollDice(1, 2, -1) : _monsterCloseAttChkTable2[(m->dir << 2) + m->pos];
	if (!v)
		first += 6;

	int last = first + 6;
	for (int i = first; i < last; i++) {
		int c = _monsterCloseAttDstTable2[i];
		if (!testCharacter(c, 3))
			continue;

		// Character Invisibility
		if ((_characters[c].effectFlags & 0x140) && (rollDice(1, 20) >= 5))
			continue;

		int dmg = 0;
		for (int ii = 0; ii < _monsterProps[m->type].attacksPerRound; ii++) {
			if (!monsterAttackHitTest(m, c))
				continue;
			dmg += rollDice(_monsterProps[m->type].dmgDc[ii].times, _monsterProps[m->type].dmgDc[ii].pips, _monsterProps[m->type].dmgDc[ii].base);
		}

		if (dmg > 0) {
			if ((_monsterProps[m->type].flags & 0x80) && rollDice(1, 4, -1) != 3) {
				int slot = rollDice(1, 27, -1);
				for (int iii = 0; iii < 27; iii++) {
					Item itm = _characters[c].inventory[slot];
					if (!itm || !(_itemTypes[_items[itm].type].extraProperties & 0x80)) {
						if (++slot == 27)
							slot = 0;
						continue;
					}

					_characters[c].inventory[slot] = 0;
					_txt->printMessage(_itemExtraStrings[(_characters[c].raceSex & 1) ^ 1], -1, _characters[c].name);
					printFullItemName(itm);
					_txt->printMessage(_itemExtraStrings[2]);
				}
				gui_drawCharPortraitWithStats(c);
			}

			inflictCharacterDamage(c, dmg);

			if (_monsterProps[m->type].flags & 0x10) {
				statusAttack(c, 2, _monsterSpecAttStrings[_flags.gameID == GI_EOB1 ? 3 : 2], 0, 1, 8, 1);
				_characters[c].effectFlags &= ~0x2000;
			}

			if (_monsterProps[m->type].flags & 0x20)
				statusAttack(c, 4, _monsterSpecAttStrings[_flags.gameID == GI_EOB1 ? 4 : 3], 2, 5, 9, 1);

			if (_monsterProps[m->type].flags & 0x8000)
				statusAttack(c, 8, _monsterSpecAttStrings[4], 2, 0, 0, 1);

		}

		if (!(_monsterProps[m->type].flags & 0x4000))
			return;
	}
}

void EobCoreEngine::monsterSpellCast(EobMonsterInPlay *m, int type) {
	launchMagicObject(-1, type, m->block, m->pos, m->dir);
	snd_processEnvironmentalSoundEffect(_spells[_magicFlightObjectProperties[type << 2]].sound, m->block);
}

void EobCoreEngine::statusAttack(int charIndex, int attackStatusFlags, const char *attackStatusString, int a, uint32 effectDuration, int restoreEvent, int noRefresh) {
	EobCharacter *c = &_characters[charIndex];
	if ((c->flags & attackStatusFlags) && noRefresh)
		return;
	if (!testCharacter(charIndex, 3))
		return;

	if (a != 5 && specialAttackConstTest(charIndex, a))
		return;

	if (attackStatusFlags & 8) {
		removeAllCharacterEffects(charIndex);
		c->flags = (c->flags & 1) | 8;
	} else {
		c->flags |= attackStatusFlags;
	}

	if ((attackStatusFlags & 0x0c) && (_openBookChar == charIndex) && _updateFlags) {
		Button b;
		clickedSpellbookAbort(&b);
	}

	if (effectDuration)
		setCharEventTimer(charIndex, effectDuration * 546, restoreEvent, 1);

	gui_drawCharPortraitWithStats(charIndex);
	_txt->printMessage(_characterStatusStrings13[0], -1, c->name, attackStatusString);
}

int EobCoreEngine::calcCloseDistanceMonsterDamage(EobMonsterInPlay *m, int times, int pips, int offs, int flags, int b, int damageType) {
	int s = flags & 0x100 ? calcDamageModifers(times, m, pips, _items[pips].type, offs) : rollDice(times, pips, offs);
	EobMonsterProperty *p = &_monsterProps[m->type];

	if (b == 5) {
		if (checkUnkConstModifiers(m, 0, p->level, b, 6))
			s = recalcDamageModifier(damageType, s);
	}

	if ((flags & 0x110) == 0x110) {
		if (!calcDamageCheckItemType(_items[pips].type))
			s = 1;
	}

	if ((flags & 0x100) && ((_flags.gameID == GI_EOB2 && (p->statusFlags & 0x100)) || (_flags.gameID == GI_EOB1 && (p->flags & 4))) && (!(_itemTypes[_items[pips].type].allowedClasses & 4 /* bug in original code ??*/)))
		s >>= 1;

	if (p->statusFlags & 0x2000) {
		if (flags & 0x100) {
			if (_items[pips].value < 3)
				s >>= 2;
			if (_items[pips].value == 3)
				s >>= 1;
			if (s == 0)
				s = _items[pips].value;

		} else {
			s >>= 1;
		}
	}

	if (flags & 1) {
		if (checkMonsterDamageEvasion(m))
			s = 0;
	}

	if (_flags.gameID == GI_EOB1)
		return s;

	static const uint16 damageImmunityFlags[] = { 0x01, 0x10, 0x02, 0x20, 0x80, 0x400, 0x20, 0x800, 0x40, 0x80, 0x400, 0x40 };
	for (int i = 0; i < 12; i += 2) {
		if ((flags & damageImmunityFlags[i]) && (p->statusFlags & damageImmunityFlags[i + 1]))
			s = 0;
	}

	return s;
}

int EobCoreEngine::calcDamageModifers(int charIndex, EobMonsterInPlay *m, int item, int itemType, int useStrModifier) {
	int s = (useStrModifier && (charIndex != -1)) ? getStrDamageModifier(charIndex) : 0;
	if (item) {
		EobItemType *p = &_itemTypes[itemType];
		int t = m ? m->type : 0;
		s += ((m && (_monsterProps[t].flags & 1)) ?	rollDice(p->dmgNumDiceL, p->dmgNumPipsL, p->dmgIncS /* bug in original code ? */) :
			rollDice(p->dmgNumDiceS, p->dmgNumPipsS, p->dmgIncS));
		s += _items[item].value;
	} else {
		s += rollDice(1, 2);
	}

	return (s < 0) ? 0 : s;
}

bool EobCoreEngine::checkUnkConstModifiers(void *target, int hpModifier, int level, int b, int race) {
	static const int8 constMod[] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5 };

	if (b == 5)
		return false;

	int s = getConstModifierTableValue(hpModifier, level, b);
	if (((race == 3 || race == 5) && (b == 4 || b == 1 || b == 0)) || (race == 4 && (b == 4 || b == 1))) {
		EobCharacter *c = (EobCharacter*)target;
		s -= constMod[c->constitutionCur];
	}

	return rollDice(1, 20) < s ? false : true;
}

bool EobCoreEngine::specialAttackConstTest(int charIndex, int b) {
	return checkUnkConstModifiers(&_characters[charIndex], _charClassModUnk[_characters[charIndex].cClass], _characters[charIndex].level[0], b, _characters[charIndex].raceSex >> 1);
}

int EobCoreEngine::getConstModifierTableValue(int hpModifier, int level, int b) {
	const uint8 *tbl = _constModTables[hpModifier];
	if (_constModLevelIndex[hpModifier] < level)
		level = _constModLevelIndex[hpModifier];
	level /= _constModDiv[hpModifier];
	level += (_constModExt[hpModifier] * b);

	return tbl[level];
}

bool EobCoreEngine::calcDamageCheckItemType(int itemType) {
	itemType = _itemTypes[itemType].extraProperties & 0x7f;
	return (itemType == 2 || itemType == 3) ? true : false;
}

int EobCoreEngine::recalcDamageModifier(int damageType, int dmgModifier) {
	if (damageType == 3)
		return 0;

	if (damageType == 0 || damageType == 1)
		return dmgModifier >> 1;

	return dmgModifier;
}

bool EobCoreEngine::checkMonsterDamageEvasion(EobMonsterInPlay *m) {
	return rollDice(1, 100) < _monsterProps[m->type].dmgModifierEvade ? true : false;
}

int EobCoreEngine::getStrHitChanceModifier(int charIndex) {
	static const int8 strExtLimit[] = { 1, 51, 76, 91, 100 };
	static const int8 strExtMod[] = { 1, 2, 2, 2, 3 };
	static const int8 strMod[] = { -4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 4, 4, 5, 6, 7 };

	int r = strMod[_characters[charIndex].strengthCur - 1];
	if (_characters[charIndex].strengthExtCur) {
		for (int i = 0; i < 5; i++) {
			if (_characters[charIndex].strengthExtCur >= strExtLimit[i])
				r = strExtMod[i];
		}
	}

	return r;
}

int EobCoreEngine::getStrDamageModifier(int charIndex) {
	static const int8 strExtLimit[] = { 1, 51, 76, 91, 100 };
	static const int8 strExtMod[] = { 3, 3, 4, 5, 6 };
	static const int8 strMod[] = { -3, -2, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 7, 8, 9, 10, 11, 12, 14 };

	int r = strMod[_characters[charIndex].strengthCur - 1];
	if (_characters[charIndex].strengthExtCur) {
		for (int i = 0; i < 5; i++) {
			if (_characters[charIndex].strengthExtCur >= strExtLimit[i])
				r = strExtMod[i];
		}
	}

	return r;
}

int EobCoreEngine::getDexHitChanceModifier(int charIndex) {
	static const int8 dexMod[] = { -5, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 3, 4, 4, 4 };
	return dexMod[_characters[charIndex].dexterityCur - 1];
}

int EobCoreEngine::getMonsterAcHitChanceModifier(int charIndex, int monsterAc) {
	static const uint8 mod1[] = { 1, 3, 3, 2 };
	static const uint8 mod2[] = { 1, 1, 2, 1 };

	int l = _characters[charIndex].level[0] - 1;
	int cm = _charClassModUnk[_characters[charIndex].cClass];

	return (20 - ((l / mod1[cm]) * mod2[cm])) - monsterAc;
}

void EobCoreEngine::explodeMonster(EobMonsterInPlay *m) {
	m->flags |= 2;
	if (getBlockDistance(m->block, _currentBlock) < 2) {
		explodeObject(0, _currentBlock, 2);
		for (int i = 0; i < 6; i++)
			calcAndInflictCharacterDamage(i, 6, 6, 0, 8, 1, 0);
	} else {
		explodeObject(0, m->block, 2);
	}
	m->flags &= ~2;
}

void EobCoreEngine::snd_playSoundEffect(int id, int volume) {
	if (id < 1 || id > 119 || shouldQuit())
		return;

	_sound->playSoundEffect(id, volume);
}

}	// End of namespace Kyra

#endif // ENABLE_EOB
