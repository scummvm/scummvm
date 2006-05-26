/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/savefile.h"

#include "sound/mixer.h"
#include "sound/mididrv.h"

#include "gui/message.h"

#include "kyra/kyra.h"
#include "kyra/resource.h"
#include "kyra/screen.h"
#include "kyra/script.h"
#include "kyra/seqplayer.h"
#include "kyra/sound.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
#include "kyra/animator.h"
#include "kyra/text.h"
#include "kyra/debugger.h"

namespace Kyra {

KyraEngine::KyraEngine(OSystem *system)
	: Engine(system) {
	_seq_Forest = _seq_KallakWriting = _seq_KyrandiaLogo = _seq_KallakMalcolm =
	_seq_MalcolmTree = _seq_WestwoodLogo = _seq_Demo1 = _seq_Demo2 = _seq_Demo3 =
	_seq_Demo4 = 0;
	
	_seq_WSATable = _seq_CPSTable = _seq_COLTable = _seq_textsTable = 0;
	_seq_WSATable_Size = _seq_CPSTable_Size = _seq_COLTable_Size = _seq_textsTable_Size = 0;
	
	_roomFilenameTable = _characterImageTable = 0;
	_roomFilenameTableSize = _characterImageTableSize = 0;
	_itemList = _takenList = _placedList = _droppedList = _noDropList = 0;
	_itemList_Size = _takenList_Size = _placedList_Size = _droppedList_Size = _noDropList_Size = 0;
	_putDownFirst = _waitForAmulet = _blackJewel = _poisonGone = _healingTip = 0;
	_putDownFirst_Size = _waitForAmulet_Size = _blackJewel_Size = _poisonGone_Size = _healingTip_Size = 0;
	_thePoison = _fluteString = _wispJewelStrings = _magicJewelString = _flaskFull = _fullFlask = 0;
	_thePoison_Size = _fluteString_Size = _wispJewelStrings_Size = 0;
	_magicJewelString_Size = _flaskFull_Size = _fullFlask_Size = 0;
	
	_defaultShapeTable = 0;
	_healingShapeTable = _healingShape2Table = 0;
	_defaultShapeTableSize = _healingShapeTableSize = _healingShape2TableSize = 0;
	_posionDeathShapeTable = _fluteAnimShapeTable = 0;
	_posionDeathShapeTableSize = _fluteAnimShapeTableSize = 0;
	_winterScrollTable = _winterScroll1Table = _winterScroll2Table = 0;
	_winterScrollTableSize = _winterScroll1TableSize = _winterScroll2TableSize = 0;
	_drinkAnimationTable = _brandonToWispTable = _magicAnimationTable = _brandonStoneTable = 0;
	_drinkAnimationTableSize = _brandonToWispTableSize = _magicAnimationTableSize = _brandonStoneTableSize = 0;
	memset(&_specialPalettes, 0, sizeof(_specialPalettes));
	_debugger = 0;
	_sprites = 0;
	_animator = 0;
	_screen = 0;
	_res = 0;
	_sound = 0;
	_saveFileMan = 0;
	_seq = 0;
	_scriptInterpreter = 0;
	_text = 0;
	_npcScriptData = 0;
	_scriptMain = 0;
	_scriptClickData = 0;
	_scriptClick = 0;
	_characterList = 0;
	_movFacingTable = 0;
	memset(_shapes, 0, sizeof(_shapes));
	memset(_movieObjects, 0, sizeof(_movieObjects));
	_finalA = _finalB = _finalC = 0;
	_endSequenceBackUpRect = 0;
	memset(_panPagesTable, 0, sizeof(_panPagesTable));
	_npcScriptData = _scriptClickData = 0;
	_scrollUpButton.process0PtrShape = _scrollUpButton.process1PtrShape = _scrollUpButton.process2PtrShape = 0;
	_scrollDownButton.process0PtrShape = _scrollDownButton.process1PtrShape = _scrollDownButton.process2PtrShape = 0;
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	_features = 0;
}

KyraEngine_v1::KyraEngine_v1(OSystem *system)
	: KyraEngine(system) {
}

KyraEngine_v2::KyraEngine_v2(OSystem *system)
	: KyraEngine(system) {
}

int KyraEngine::init() {
	if (setupGameFlags()) {
		return -1;
	}

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	// sets up all engine specific debug levels
	Common::addSpecialDebugLevel(kDebugLevelScriptFuncs, "ScriptFuncs", "Script function debug level");
	Common::addSpecialDebugLevel(kDebugLevelScript, "Script", "Script interpreter debug level");
	Common::addSpecialDebugLevel(kDebugLevelSprites, "Sprites", "Sprite debug level");
	Common::addSpecialDebugLevel(kDebugLevelScreen, "Screen", "Screen debug level");
	Common::addSpecialDebugLevel(kDebugLevelSound, "Sound", "Sound debug level");
	Common::addSpecialDebugLevel(kDebugLevelAnimator, "Animator", "Animator debug level");
	Common::addSpecialDebugLevel(kDebugLevelMain, "Main", "Generic debug level");
	Common::addSpecialDebugLevel(kDebugLevelGUI, "GUI", "GUI debug level");
	Common::addSpecialDebugLevel(kDebugLevelSequence, "Sequence", "Sequence debug level");
	Common::addSpecialDebugLevel(kDebugLevelMovie, "Movie", "Movie debug level");

	// for now we prefer Adlib over native MIDI
	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB/* | MDT_PREFER_MIDI*/);

	if (midiDriver == MD_ADLIB) {
		_sound = new SoundAdlibPC(_mixer, this);
		assert(_sound);
	} else {
		bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

		MidiDriver *driver = MidiDriver::createMidi(midiDriver);
		assert(driver);
		if (native_mt32) {
			driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		}

		SoundMidiPC *soundMidiPc = new SoundMidiPC(driver, _mixer, this);
		_sound = soundMidiPc;
		assert(_sound);
		soundMidiPc->hasNativeMT32(native_mt32);
		
		// Unlike some SCUMM games, it's not that the MIDI sounds are
		// missing. It's just that at least at the time of writing they
		// decidedly inferior to the Adlib ones.

		if (ConfMan.getBool("multi_midi")) {
			SoundAdlibPC *adlib = new SoundAdlibPC(_mixer, this);
			assert(adlib);
			
			_sound = new MixedSoundDriver(this, _mixer, soundMidiPc, adlib);
			assert(_sound);
		}
	}
	if (!_sound->init()) {
		error("Couldn't init sound");
	}
	_sound->setVolume(255);
	
	_saveFileMan = _system->getSavefileManager();
	assert(_saveFileMan);	
	_res = new Resource(this);
	assert(_res);
	_screen = new Screen(this, _system);
	assert(_screen);
	if (!_screen->init())
		error("_screen->init() failed");
	_sprites = new Sprites(this, _system);
	assert(_sprites);
	_seq = new SeqPlayer(this, _system);
	assert(_seq);
	_animator = new ScreenAnimator(this, _system);
	assert(_animator);
	_animator->init(5, 11, 12);
	assert(*_animator);
	_text = new TextDisplayer(_screen);
	assert(_text);

	_staticres = new StaticResource(this);
	assert(_staticres);
	if (!_staticres->init())
		error("_staticres->init() failed");
	
	initStaticResource();
	
	_paletteChanged = 1;
	_currentCharacter = 0;
	_characterList = new Character[11];
	assert(_characterList);
	for (int i = 0; i < 11; ++i) {
		memset(&_characterList[i], 0, sizeof(Character));
		memset(_characterList[i].inventoryItems, 0xFF, sizeof(_characterList[i].inventoryItems));
	}
	_characterList[0].sceneId = 5;
	_characterList[0].height = 48;
	_characterList[0].facing = 3;
	_characterList[0].currentAnimFrame = 7;
	
	_scriptInterpreter = new ScriptHelper(this);
	assert(_scriptInterpreter);
	
	_npcScriptData = new ScriptData;
	memset(_npcScriptData, 0, sizeof(ScriptData));
	assert(_npcScriptData);
	_npcScript = new ScriptState;
	assert(_npcScript);
	memset(_npcScript, 0, sizeof(ScriptState));
	
	_scriptMain = new ScriptState;
	assert(_scriptMain);
	memset(_scriptMain, 0, sizeof(ScriptState));
	
	_scriptClickData = new ScriptData;
	assert(_scriptClickData);
	memset(_scriptClickData, 0, sizeof(ScriptData));
	_scriptClick = new ScriptState;
	assert(_scriptClick);
	memset(_scriptClick, 0, sizeof(ScriptState));
	
	_debugger = new Debugger(this);
	assert(_debugger);	
	memset(_shapes, 0, sizeof(_shapes));

	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		_movieObjects[i] = createWSAMovie();
	}

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_abortWalkFlag = false;
	_abortWalkFlag2 = false;
	_talkingCharNum = -1;
	_charSayUnk3 = -1;
	_mouseX = _mouseY = -1;
	memset(_currSentenceColor, 0, 3);
	_startSentencePalIndex = -1;
	_fadeText = false;

	_cauldronState = 0;
	_crystalState[0] = _crystalState[1] = -1;

	_brandonStatusBit = 0;
	_brandonStatusBit0x02Flag = _brandonStatusBit0x20Flag = 10;
	_brandonPosX = _brandonPosY = -1;
	_deathHandler = 0xFF;
	_poisonDeathCounter = 0;
	
	memset(_itemTable, 0, sizeof(_itemTable));
	memset(_exitList, 0xFFFF, sizeof(_exitList));
	_exitListPtr = 0;
	_pathfinderFlag = _pathfinderFlag2 = 0;
	_lastFindWayRet = 0;
	_sceneChangeState = _loopFlag2 = 0;
	_timerNextRun = 0;

	_movFacingTable = new int[150];
	assert(_movFacingTable);
	_movFacingTable[0] = 8;

	_configTextspeed = 1;
	_configWalkspeed = 2;
	_configMusic = true;
	_configSounds = true;
	_configVoice = 1;

	_skipFlag = false;

	_marbleVaseItem = -1;
	memset(_foyerItemTable, -1, sizeof(_foyerItemTable));
	_mouseState = _itemInHand = -1;
	_handleInput = false;
	
	_currentRoom = 0xFFFF;
	_scenePhasingFlag = 0;
	_lastProcessedItem = 0;
	_lastProcessedItemHeight = 16;
	
	_unkScreenVar1 = 1;
	_unkScreenVar2 = 0;
	_unkScreenVar3 = 0;
	_unkAmuletVar = 0;
	
	_endSequenceNeedLoading = 1;
	_malcolmFlag = 0;
	_beadStateVar = 0;
	_endSequenceSkipFlag = 0;
	_unkEndSeqVar2 = 0;
	_endSequenceBackUpRect = 0;
	_unkEndSeqVar4 = 0;
	_unkEndSeqVar5 = 0;
	_lastDisplayedPanPage = 0;
	memset(_panPagesTable, 0, sizeof(_panPagesTable));
	_finalA = _finalB = _finalC = 0;
	memset(&_kyragemFadingState, 0, sizeof(_kyragemFadingState));	
	_kyragemFadingState.gOffset = 0x13;
	_kyragemFadingState.bOffset = 0x13;

	_mousePressFlag = false;
	
	_menuDirectlyToLoad = false;

	_lastMusicCommand = 0;

	_gameSpeed = 60;
	_tickLength = (uint8)(1000.0 / _gameSpeed);

	return 0;
}

KyraEngine::~KyraEngine() {
	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		if (_movieObjects[i])
			_movieObjects[i]->close();
		delete _movieObjects[i];
		_movieObjects[i] = 0;
	}

	closeFinalWsa();
	if (_scriptInterpreter) {
		_scriptInterpreter->unloadScript(_npcScriptData);
		_scriptInterpreter->unloadScript(_scriptClickData);
	}

	Common::clearAllSpecialDebugLevels();

	delete _debugger;
	delete _sprites;
	delete _animator;
	delete _screen;
	delete _res;
	delete _sound;
	delete _saveFileMan;
	delete _seq;
	delete _scriptInterpreter;
	delete _text;
	
	delete _npcScriptData;
	delete _scriptMain;
	
	delete _scriptClickData;
	delete _scriptClick;
	
	delete [] _characterList;
	
	delete [] _movFacingTable;

	free(_scrollUpButton.process0PtrShape);
	free(_scrollUpButton.process1PtrShape);
	free(_scrollUpButton.process2PtrShape);
	free(_scrollDownButton.process0PtrShape);
	free(_scrollDownButton.process1PtrShape);
	free(_scrollDownButton.process2PtrShape);
		
	for (int i = 0; i < ARRAYSIZE(_shapes); ++i) {
		if (_shapes[i] != 0) {
			free(_shapes[i]);
			for (int i2 = 0; i2 < ARRAYSIZE(_shapes); i2++) {
				if (_shapes[i2] == _shapes[i] && i2 != i) {
					_shapes[i2] = 0;
				}
			}
			_shapes[i] = 0;
		}
	}
	for (int i = 0; i < ARRAYSIZE(_sceneAnimTable); ++i) {
		free(_sceneAnimTable[i]);
	}
}

KyraEngine_v1::~KyraEngine_v1() {
	// FIXME: I am leaking badly!

}

KyraEngine_v2::~KyraEngine_v2() {
	// FIXME: I am leaking badly!
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int KyraEngine::go() {
	_quitFlag = false;
	uint32 sz;

	if (_features & GF_FLOPPY && !(_features & GF_AMIGA)) {
		_screen->loadFont(Screen::FID_6_FNT, _res->fileData("6.FNT", &sz));
	}
	_screen->loadFont(Screen::FID_8_FNT, _res->fileData("8FAT.FNT", &sz));
	_screen->setScreenDim(0);

	_abortIntroFlag = false;

	if (_features & GF_DEMO) {
		seq_demo();
	} else {
		setGameFlag(0xF3);
		setGameFlag(0xFD);
		setGameFlag(0xEF);
		seq_intro();
		if (_quitFlag)
			return 0;
		if (_skipIntroFlag && _abortIntroFlag)
			resetGameFlag(0xEF);
		startup();
		resetGameFlag(0xEF);
		mainLoop();
	}
	return 0;
}

int KyraEngine_v2::go() {
	// Kyra2 goes here :)
	loadPalette("palette.col", _screen->_currentPalette);
	_screen->setScreenPalette(_screen->_currentPalette);
	loadBitmap("_playfld.cps", 0, 0, 0);
	_screen->updateScreen();
	waitForEvent();
	return 0;
}

void KyraEngine::startup() {
	debugC(9, kDebugLevelMain, "KyraEngine::startup()");
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	_screen->setTextColorMap(colorMap);
//	_screen->setFont(Screen::FID_6_FNT);
	_screen->setAnimBlockPtr(3750);
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	loadMouseShapes();
	_currentCharacter = &_characterList[0];
	for (int i = 1; i < 5; ++i)
		_animator->setCharacterDefaultFrame(i);
	for (int i = 5; i <= 10; ++i)
		setCharactersPositions(i);
	_animator->setCharactersHeight();
	resetBrandonPoisonFlags();
	_maskBuffer = _screen->getPagePtr(5);
	_screen->_curPage = 0;
	// XXX
	for (int i = 0; i < 0x0C; ++i) {
		int size = _screen->getRectSize(3, 24);
		_shapes[365+i] = (byte*)malloc(size);
	}
	_shapes[0] = (uint8*)malloc(_screen->getRectSize(3, 24));
	memset(_shapes[0], 0, _screen->getRectSize(3, 24));
	_shapes[1] = (uint8*)malloc(_screen->getRectSize(4, 32));
	memset(_shapes[1], 0, _screen->getRectSize(4, 32));
	_shapes[2] = (uint8*)malloc(_screen->getRectSize(8, 69));
	memset(_shapes[2], 0, _screen->getRectSize(8, 69));
	_shapes[3] = (uint8*)malloc(_screen->getRectSize(8, 69));
	memset(_shapes[3], 0, _screen->getRectSize(8, 69));
	for (int i = 0; i < _roomTableSize; ++i) {
		for (int item = 0; item < 12; ++item) {
			_roomTable[i].itemsTable[item] = 0xFF;
			_roomTable[i].itemsXPos[item] = 0xFFFF;
			_roomTable[i].itemsYPos[item] = 0xFF;
			_roomTable[i].needInit[item] = 0;
		}
	}
	loadCharacterShapes();
	loadSpecialEffectShapes();
	loadItems();
	loadButtonShapes();
	initMainButtonList();
	loadMainScreen();
	setupTimers();
	loadPalette("PALETTE.COL", _screen->_currentPalette);

	// XXX
	_animator->initAnimStateList();
	setCharactersInDefaultScene();

	if (!_scriptInterpreter->loadScript("_STARTUP.EMC", _npcScriptData, _opcodeTable, _opcodeTableSize, 0)) {
		error("Could not load \"_STARTUP.EMC\" script");
	}
	_scriptInterpreter->initScript(_scriptMain, _npcScriptData);
	if (!_scriptInterpreter->startScript(_scriptMain, 0)) {
		error("Could not start script function 0 of script \"_STARTUP.EMC\"");
	}
	while (_scriptInterpreter->validScript(_scriptMain)) {
		_scriptInterpreter->runScript(_scriptMain);
	}
	
	_scriptInterpreter->unloadScript(_npcScriptData);
	if (!_scriptInterpreter->loadScript("_NPC.EMC", _npcScriptData, _opcodeTable, _opcodeTableSize, 0)) {
		error("Could not load \"_NPC.EMC\" script");
	}
	
	snd_playTheme(1);
	enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);
	
	if (_abortIntroFlag && _skipIntroFlag) {
		_menuDirectlyToLoad = true;
		_screen->setMouseCursor(1, 1, _shapes[4]);
		buttonMenuCallback(0);
		_menuDirectlyToLoad = false;
	} else
		saveGame(getSavegameFilename(0), "New game");
}

void KyraEngine::mainLoop() {
	debugC(9, kDebugLevelMain, "KyraEngine::mainLoop()");

	while (!_quitFlag) {
		int32 frameTime = (int32)_system->getMillis();
		_skipFlag = false;

		if (_currentCharacter->sceneId == 210) {
			updateKyragemFading();
			if (seq_playEnd()) {
				if (_deathHandler != 8)
					break;
			}
		}
		
		if (_deathHandler != 0xFF) {
			snd_playWanderScoreViaMap(0, 1);
			snd_playSoundEffect(49);
			_screen->hideMouse();
			_screen->setMouseCursor(1, 1, _shapes[4]);
			destroyMouseItem();
			_screen->showMouse();
			buttonMenuCallback(0);
			_deathHandler = 0xFF;
		}
		
		if (_brandonStatusBit & 2) {
			if (_brandonStatusBit0x02Flag)
				_animator->animRefreshNPC(0);
		}
		if (_brandonStatusBit & 0x20) {
			if (_brandonStatusBit0x20Flag) {
				_animator->animRefreshNPC(0);
				_brandonStatusBit0x20Flag = 0;
			}
		}
		
		_screen->showMouse();

		processButtonList(_buttonList);
		updateMousePointer();
		updateGameTimers();
		updateTextFade();

		_handleInput = true;
		delay((frameTime + _gameSpeed) - _system->getMillis(), true, true);
		_handleInput = false;

		_sound->process();
	}
}

void KyraEngine::quitGame() {
	debugC(9, kDebugLevelMain, "KyraEngine::quitGame()");
	_quitFlag = true;
	// Nothing to do here
}

void KyraEngine::delayUntil(uint32 timestamp, bool updateTimers, bool update, bool isMainLoop) {
	while (_system->getMillis() < timestamp && !_quitFlag) {
		if (updateTimers)
			updateGameTimers();
		if (timestamp - _system->getMillis() >= 10)
			delay(10, update, isMainLoop);
	}
}

void KyraEngine::delay(uint32 amount, bool update, bool isMainLoop) {
	OSystem::Event event;
	char saveLoadSlot[20];
	char savegameName[14];

	uint32 start = _system->getMillis();
	do {
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode >= '1' && event.kbd.keycode <= '9' && 
						(event.kbd.flags == OSystem::KBD_CTRL || event.kbd.flags == OSystem::KBD_ALT) && isMainLoop) {
					sprintf(saveLoadSlot, "%s.00%d", _targetName.c_str(), event.kbd.keycode - '0');
					if (event.kbd.flags == OSystem::KBD_CTRL)
						loadGame(saveLoadSlot);
					else {
						sprintf(savegameName, "Quicksave %d",  event.kbd.keycode - '0');
						saveGame(saveLoadSlot, savegameName);
					}
				} else if (event.kbd.flags == OSystem::KBD_CTRL) {
					if (event.kbd.keycode == 'd')
						_debugger->attach();
					else if (event.kbd.keycode == 'q')
						_quitFlag = true;
				} else if (event.kbd.keycode == '.')
						_skipFlag = true;
				else if (event.kbd.keycode == 13 || event.kbd.keycode == 32 || event.kbd.keycode == 27) {
					_abortIntroFlag = true;
					_skipFlag = true;
				}

				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				_system->updateScreen();
				break;
			case OSystem::EVENT_QUIT:
				quitGame();
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_mousePressFlag = true;
				break;
			case OSystem::EVENT_LBUTTONUP:
				_mousePressFlag = false;
				if (_abortWalkFlag2) {
					_abortWalkFlag = true;
					_mouseX = event.mouse.x;
					_mouseY = event.mouse.y;
				}
				if (_handleInput) {
					_mouseX = event.mouse.x;
					_mouseY = event.mouse.y;
					_handleInput = false;
					processInput(_mouseX, _mouseY);
					_handleInput = true;
				} else
					_skipFlag = true;
				break;
			default:
				break;
			}
		}
		if (_debugger->isAttached())
			_debugger->onFrame();

		if (update) {
			_sprites->updateSceneAnims();
			_animator->updateAllObjectShapes();
			updateTextFade();
			updateMousePointer();
		}

		if (_currentCharacter && _currentCharacter->sceneId == 210 && update) {
			updateKyragemFading();
		}

		if (_skipFlag && !_abortIntroFlag && !queryGameFlag(0xFE))
			_skipFlag = false;
			
		if (amount > 0 && !_skipFlag && !_quitFlag) {
			_system->delayMillis((amount > 10) ? 10 : amount);
		}
	} while (!_skipFlag && _system->getMillis() < start + amount && !_quitFlag);
	
}

void KyraEngine::waitForEvent() {
	bool finished = false;
	OSystem::Event event;
	while (!finished) {
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				finished = true;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case OSystem::EVENT_QUIT:
				quitGame();
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				finished = true;
				_skipFlag = true;
				break;
			default:
				break;
			}
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		_system->delayMillis(10);
	}
}

void KyraEngine::delayWithTicks(int ticks) {
	uint32 nextTime = _system->getMillis() + ticks * _tickLength;
	while (_system->getMillis() < nextTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (_currentCharacter->sceneId == 210) {
			updateKyragemFading();
			seq_playEnd();
		}
		if (_skipFlag)
			break;
		if (nextTime - _system->getMillis() >= 10)
			delay(10);
	}
}

#pragma mark -
#pragma mark - Animation/shape specific code
#pragma mark -

void KyraEngine::setupShapes123(const Shape *shapeTable, int endShape, int flags) {
	debugC(9, kDebugLevelMain, "KyraEngine::setupShapes123(%p, %d, %d)", (const void *)shapeTable, endShape, flags);
	for (int i = 123; i <= 172; ++i) {
		_shapes[4+i] = NULL;
	}
	uint8 curImage = 0xFF;
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 8;	// we are using page 8 here in the original page 2 was backuped and then used for this stuff
	int shapeFlags = 2;
	if (flags)
		shapeFlags = 3;
	for (int i = 123; i < 123+endShape; ++i) {
		uint8 newImage = shapeTable[i-123].imageIndex;
		if (newImage != curImage && newImage != 0xFF) {
			assert(_characterImageTable);
			loadBitmap(_characterImageTable[newImage], 8, 8, 0);
			curImage = newImage;
		}
		_shapes[4+i] = _screen->encodeShape(shapeTable[i-123].x<<3, shapeTable[i-123].y, shapeTable[i-123].w<<3, shapeTable[i-123].h, shapeFlags);
		assert(i-7 < _defaultShapeTableSize);
		_defaultShapeTable[i-7].xOffset = shapeTable[i-123].xOffset;
		_defaultShapeTable[i-7].yOffset = shapeTable[i-123].yOffset;
		_defaultShapeTable[i-7].w = shapeTable[i-123].w;
		_defaultShapeTable[i-7].h = shapeTable[i-123].h;
	}
	_screen->_curPage = curPageBackUp;
}

void KyraEngine::freeShapes123() {
	debugC(9, kDebugLevelMain, "KyraEngine::freeShapes123()");
	for (int i = 123; i <= 172; ++i) {
		free(_shapes[4+i]);
		_shapes[4+i] = NULL;
	}
}

#pragma mark -
#pragma mark - Misc stuff
#pragma mark -

Movie *KyraEngine::createWSAMovie() {
	// for kyra2 here could be added then WSAMovieV2
	return new WSAMovieV1(this);
}

int KyraEngine::setGameFlag(int flag) {
	_flagsTable[flag >> 3] |= (1 << (flag & 7));
	return 1;
}

int KyraEngine::queryGameFlag(int flag) {
	return ((_flagsTable[flag >> 3] >> (flag & 7)) & 1);
}

int KyraEngine::resetGameFlag(int flag) {
	_flagsTable[flag >> 3] &= ~(1 << (flag & 7));
	return 0;
}

void KyraEngine::setBrandonPoisonFlags(int reset) {
	debugC(9, kDebugLevelMain, "KyraEngine::setBrandonPoisonFlags(%d)", reset);
	_brandonStatusBit |= 1;
	if (reset)
		_poisonDeathCounter = 0;
	for (int i = 0; i < 0x100; ++i) {
		_brandonPoisonFlagsGFX[i] = i;
	}
	_brandonPoisonFlagsGFX[0x99] = 0x34;
	_brandonPoisonFlagsGFX[0x9A] = 0x35;
	_brandonPoisonFlagsGFX[0x9B] = 0x37;
	_brandonPoisonFlagsGFX[0x9C] = 0x38;
	_brandonPoisonFlagsGFX[0x9D] = 0x2B;
}

void KyraEngine::resetBrandonPoisonFlags() {
	debugC(9, kDebugLevelMain, "KyraEngine::resetBrandonPoisonFlags()");
	_brandonStatusBit = 0;
	for (int i = 0; i < 0x100; ++i) {
		_brandonPoisonFlagsGFX[i] = i;
	}
}

#pragma mark -
#pragma mark - Input
#pragma mark -

void KyraEngine::processInput(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine::processInput(%d, %d)", xpos, ypos);
	_abortWalkFlag2 = false;

	if (processInputHelper(xpos, ypos)) {
		return;
	}
	uint8 item = findItemAtPos(xpos, ypos);
	if (item == 0xFF) {
		_changedScene = false;
		int handled = clickEventHandler(xpos, ypos);
		if (_changedScene || handled)
			return;
	} 
	
	// XXX _deathHandler specific
	if (ypos <= 158) {
		uint16 exit = 0xFFFF;
		if (xpos < 12) {
			exit = _walkBlockWest;
		} else if (xpos >= 308) {
			exit = _walkBlockEast;
		} else if (ypos >= 136) {
			exit = _walkBlockSouth;
		} else if (ypos < 12) {
			exit = _walkBlockNorth;
		}
		
		if (exit != 0xFFFF) {
			_abortWalkFlag2 = true;
			handleSceneChange(xpos, ypos, 1, 1);
			_abortWalkFlag2 = false;
			return;
		} else {
			int script = checkForNPCScriptRun(xpos, ypos);
			if (script >= 0) {
				runNpcScript(script);
				return;
			}
			if (_itemInHand != -1) {
				if (ypos < 155) {
					if (hasClickedOnExit(xpos, ypos)) {
						_abortWalkFlag2 = true;
						handleSceneChange(xpos, ypos, 1, 1);
						_abortWalkFlag2 = false;
						return;
					}
					dropItem(0, _itemInHand, xpos, ypos, 1);
				}
			} else {
				if (ypos <= 155) {
					_abortWalkFlag2 = true;
					handleSceneChange(xpos, ypos, 1, 1);
					_abortWalkFlag2 = false;
				}
			}
		}
	}	
}

int KyraEngine::processInputHelper(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine::processInputHelper(%d, %d)", xpos, ypos);
	uint8 item = findItemAtPos(xpos, ypos);
	if (item != 0xFF) {
		if (_itemInHand == -1) {
			_screen->hideMouse();
			_animator->animRemoveGameItem(item);
			snd_playSoundEffect(53);
			assert(_currentCharacter->sceneId < _roomTableSize);
			Room *currentRoom = &_roomTable[_currentCharacter->sceneId];
			int item2 = currentRoom->itemsTable[item];
			currentRoom->itemsTable[item] = 0xFF;
			setMouseItem(item2);
			assert(_itemList && _takenList);
			updateSentenceCommand(_itemList[item2], _takenList[0], 179);
			_itemInHand = item2;
			_screen->showMouse();
			clickEventHandler2();
			return 1;
		} else {
			exchangeItemWithMouseItem(_currentCharacter->sceneId, item);
			return 1;
		}
	}
	return 0;
}

int KyraEngine::clickEventHandler(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine::clickEventHandler(%d, %d)", xpos, ypos);
	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	_scriptClick->variables[1] = xpos;
	_scriptClick->variables[2] = ypos;
	_scriptClick->variables[3] = 0;
	_scriptClick->variables[4] = _itemInHand;
	_scriptInterpreter->startScript(_scriptClick, 1);

	while (_scriptInterpreter->validScript(_scriptClick)) {
		_scriptInterpreter->runScript(_scriptClick);
	}

	return _scriptClick->variables[3];
}

void KyraEngine::updateMousePointer(bool forceUpdate) {
	int shape = 0;
	
	int newMouseState = 0;
	int newX = 0;
	int newY = 0;
	if (_mouseY <= 158) {
		if (_mouseX >= 12) {
			if (_mouseX >= 308) {
				if (_walkBlockEast == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -5;
					shape = 3;
					newX = 7;
					newY = 5;
				}
			} else if (_mouseY >= 136) {
				if (_walkBlockSouth == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -4;
					shape = 4;
					newX = 5;
					newY = 7;
				}
			} else if (_mouseY < 12) {
				if (_walkBlockNorth == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -6;
					shape = 2;
					newX = 5;
					newY = 1;
				}
			}
		} else {
			if (_walkBlockWest == 0xFFFF) {
				newMouseState = -2;
			} else {
				newMouseState = -3;
				newX = 1;
				newY = shape = 5;
			}
		}
	}
	
	if (_mouseX >= _entranceMouseCursorTracks[0] && _mouseY >= _entranceMouseCursorTracks[1]
		&& _mouseX <= _entranceMouseCursorTracks[2] && _mouseY <= _entranceMouseCursorTracks[3]) {
		switch (_entranceMouseCursorTracks[4]) {
		case 0:
			newMouseState = -6;
			shape = 2;
			newX = 5;
			newY = 1;
			break;

		case 2:
			newMouseState = -5;
			shape = 3;
			newX = 7;
			newY = 5;
			break;

		case 4:
			newMouseState = -4;
			shape = 4;
			newX = 5;
			newY = 7;
			break;

		case 6:
			newMouseState = -3;
			shape = 5;
			newX = 1;
			newY = 5;
			break;

		default:
			break;
		}
	}
	
	if (newMouseState == -2) {
		shape = 6;
		newX = 4;
		newY = 4;
	}
	
	if ((newMouseState && _mouseState != newMouseState) || (newMouseState && forceUpdate)) {
		_mouseState = newMouseState;
		_screen->hideMouse();
		_screen->setMouseCursor(newX, newY, _shapes[4+shape]);
		_screen->showMouse();
	}
	
	if (!newMouseState) {
		if (_mouseState != _itemInHand || forceUpdate) {
			if (_mouseY > 158 || (_mouseX >= 12 && _mouseX < 308 && _mouseY < 136 && _mouseY >= 12) || forceUpdate) {
				_mouseState = _itemInHand;
				_screen->hideMouse();
				if (_itemInHand == -1) {
					_screen->setMouseCursor(1, 1, _shapes[4]);
				} else {
					_screen->setMouseCursor(8, 15, _shapes[220+_itemInHand]);
				}
				_screen->showMouse();
			}
		}
	}
}

bool KyraEngine::hasClickedOnExit(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine::hasClickedOnExit(%d, %d)", xpos, ypos);
	if (xpos < 16 || xpos >= 304) {
		return true;
	}
	if (ypos < 8)
		return true;
	if (ypos < 136 || ypos > 155) {
		return false;
	}
	return true;
}

void KyraEngine::clickEventHandler2() {
	debugC(9, kDebugLevelMain, "KyraEngine::clickEventHandler2()");
	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	_scriptClick->variables[0] = _currentCharacter->sceneId;
	_scriptClick->variables[1] = _mouseX;
	_scriptClick->variables[2] = _mouseY;
	_scriptClick->variables[4] = _itemInHand;
	_scriptInterpreter->startScript(_scriptClick, 6);
	
	while (_scriptInterpreter->validScript(_scriptClick)) {
		_scriptInterpreter->runScript(_scriptClick);
	}
}

int KyraEngine::checkForNPCScriptRun(int xpos, int ypos) {
	debugC(9, kDebugLevelMain, "KyraEngine::checkForNPCScriptRun(%d, %d)", xpos, ypos);
	int returnValue = -1;
	const Character *currentChar = _currentCharacter;
	int charLeft = 0, charRight = 0, charTop = 0, charBottom = 0;
	
	int scaleFactor = _scaleTable[currentChar->y1];
	int addX = (((scaleFactor*8)*3)>>8)>>1;
	int addY = ((scaleFactor*3)<<4)>>8;
	
	charLeft = currentChar->x1 - addX;
	charRight = currentChar->x1 + addX;
	charTop = currentChar->y1 - addY;
	charBottom = currentChar->y1;
	
	if (xpos >= charLeft && charRight >= xpos && charTop <= ypos && charBottom >= ypos) {
		return 0;
	}
	
	if (xpos > 304 || xpos < 16) {
		return -1;
	}
	
	for (int i = 1; i < 5; ++i) {
		currentChar = &_characterList[i];
		
		if (currentChar->sceneId != _currentCharacter->sceneId)
			continue;
			
		charLeft = currentChar->x1 - 12;
		charRight = currentChar->x1 + 11;
		charTop = currentChar->y1 - 48;
		// if (!i) {
		// 	charBottom = currentChar->y2 - 16;
		// } else {
		charBottom = currentChar->y1;
		// }
		
		if (xpos < charLeft || xpos > charRight || ypos < charTop || charBottom < ypos) {
			continue;
		}
		
		if (returnValue != -1) {
			if (currentChar->y1 >= _characterList[returnValue].y1) {
				returnValue = i;
			}
		} else {
			returnValue = i;
		}
	}
	
	return returnValue;
}

void KyraEngine::runNpcScript(int func) {
	debugC(9, kDebugLevelMain, "KyraEngine::runNpcScript(%d)", func);
	_scriptInterpreter->initScript(_npcScript, _npcScriptData);
	_scriptInterpreter->startScript(_npcScript, func);
	_npcScript->variables[0] = _currentCharacter->sceneId;
	_npcScript->variables[4] = _itemInHand;
	_npcScript->variables[5] = func;
	
	while (_scriptInterpreter->validScript(_npcScript)) {
		_scriptInterpreter->runScript(_npcScript);
	}
}

} // End of namespace Kyra
