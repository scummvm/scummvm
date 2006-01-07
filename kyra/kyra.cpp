/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/md5.h"
#include "common/savefile.h"

#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/voc.h"
#include "sound/audiostream.h"

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

using namespace Kyra;

enum {
	// We only compute MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

// Kyra MD5 detection brutally ripped from the Gobliins engine.
struct KyraGameSettings {
	const char *name;
	const char *description;
	byte id;
	uint32 features;
	const char *md5sum;
	const char *checkFile;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const KyraGameSettings kyra_games[] = {
	{ "kyra1", "Legend of Kyrandia (Floppy, English)",	GI_KYRA1, GF_ENGLISH | GF_FLOPPY, // english floppy 1.0 from Malice
										"3c244298395520bb62b5edfe41688879", "GEMCUT.EMC" },
	{ "kyra1", "Legend of Kyrandia (Floppy, English)",	GI_KYRA1, GF_ENGLISH | GF_FLOPPY, 
										"796e44863dd22fa635b042df1bf16673", "GEMCUT.EMC" },
	{ "kyra1", "Legend of Kyrandia (Floppy, French)",	GI_KYRA1, GF_FRENCH | GF_FLOPPY,
										"abf8eb360e79a6c2a837751fbd4d3d24", "GEMCUT.EMC" },
	{ "kyra1", "Legend of Kyrandia (Floppy, German)",	GI_KYRA1, GF_GERMAN | GF_FLOPPY, 
										"6018e1dfeaca7fe83f8d0b00eb0dd049", "GEMCUT.EMC"},
	{ "kyra1", "Legend of Kyrandia (Floppy, Spanish)",	GI_KYRA1, GF_SPANISH | GF_FLOPPY, // from VooD
										"8909b41596913b3f5deaf3c9f1017b01", "GEMCUT.EMC"},
	{ "kyra1", "Legend of Kyrandia (Floppy, Spanish)",	GI_KYRA1, GF_SPANISH | GF_FLOPPY, // floppy 1.8 from clemmy
										"747861d2a9c643c59fdab570df5b9093", "GEMCUT.EMC"},
	{ "kyra1", "Legend of Kyrandia (CD, English)",		GI_KYRA1, GF_ENGLISH | GF_TALKIE, 
										"fac399fe62f98671e56a005c5e94e39f", "GEMCUT.PAK" },
	{ "kyra1", "Legend of Kyrandia (CD, German)",		GI_KYRA1, GF_GERMAN | GF_TALKIE, 
										"230f54e6afc007ab4117159181a1c722", "GEMCUT.PAK" },
	{ "kyra1", "Legend of Kyrandia (CD, French)",		GI_KYRA1, GF_FRENCH | GF_TALKIE, 
										"b037c41768b652a040360ffa3556fd2a", "GEMCUT.PAK" },
	{ "kyra1", "Legend of Kyrandia (Demo)",			GI_KYRA1, GF_DEMO | GF_ENGLISH,
										"fb722947d94897512b13b50cc84fd648", "DEMO1.WSA" },
	{ 0, 0, 0, 0, 0, 0 }
};

// Keep list of different supported games
struct KyraGameList {
	const char *name;
	const char *description;
	uint32 features;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const KyraGameList kyra_list[] = {
	{ "kyra1", "Legend of Kyrandia", 0 },
	{ 0, 0, 0 }
};

GameList Engine_KYRA_gameList() {
	GameList games;
	const KyraGameList *g = kyra_list;

	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_KYRA_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const KyraGameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	bool isFound = false;
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		for (g = kyra_games; g->name; g++) {
			if (scumm_stricmp(file->displayName().c_str(), g->checkFile) == 0)
				isFound = true;
		}
		if (isFound)
			break;
	}

	if (file == fslist.end())
		return detectedGames;

	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file(file->path().c_str(), md5sum, NULL, kMD5FileSizeLimit)) {
		for (int i = 0; i < 16; i++) {
			sprintf(md5str + i * 2, "%02x", (int)md5sum[i]);
		}
		for (g = kyra_games; g->name; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				detectedGames.push_back(g->toGameSettings());
			}
		}
		if (detectedGames.isEmpty()) {
			debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const KyraGameList *g1 = kyra_list;
			while (g1->name) {
				detectedGames.push_back(g1->toGameSettings());
				g1++;
			}
		}
	}
	return detectedGames;
}

Engine *Engine_KYRA_create(GameDetector *detector, OSystem *system) {
	return new KyraEngine(detector, system);
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine")

namespace Kyra {

KyraEngine::KyraEngine(GameDetector *detector, OSystem *system)
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
	
	_defaultShapeTable = _healingShapeTable = _healingShape2Table = 0;
	_defaultShapeTableSize = _healingShapeTableSize = _healingShape2TableSize = 0;
	_posionDeathShapeTable = _fluteAnimShapeTable = 0;
	_posionDeathShapeTableSize = _fluteAnimShapeTableSize = 0;
	_winterScrollTable = _winterScroll1Table = _winterScroll2Table = 0;
	_winterScrollTableSize = _winterScroll1TableSize = _winterScroll2TableSize = 0;
	_drinkAnimationTable = _brandonToWispTable = _magicAnimationTable = _brandonStoneTable = 0;
	_drinkAnimationTableSize = _brandonToWispTableSize = _magicAnimationTableSize = _brandonStoneTableSize = 0;

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	// Detect game features based on MD5. Again brutally ripped from Gobliins.
	uint8 md5sum[16];
	char md5str[32 + 1];

	const KyraGameSettings *g;
	bool found = false;

	// TODO
	// Fallback. Maybe we will be able to determine game type from game
	// data contents
	_features = 0;

	for (g = kyra_games; g->name; g++) {
		if (!Common::File::exists(g->checkFile))
			continue;

		if (Common::md5_file(g->checkFile, md5sum, ConfMan.get("path").c_str(), kMD5FileSizeLimit)) {
			for (int j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
		} else
			continue;

		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_features = g->features;
			_game = g->id;

			if (g->description)
				g_system->setWindowCaption(g->description);

			found = true;
			break;
		}
	}

	if (!found) {
		debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team", md5str);
		_features = GF_LNGUNK;
		_game = GI_KYRA1;
		Common::File test;
		if (test.open("INTRO.VRM")) {
			_features |= GF_TALKIE;
		} else {
			_features |= GF_FLOPPY;
		}
	}
}

int KyraEngine::init(GameDetector &detector) {
	_currentVocFile = 0;
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		_system->initSize(320, 200);
	_system->endGFXTransaction();

	// for now we prefer MIDI-to-Adlib conversion over native midi
	int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB/* | MDT_PREFER_MIDI*/);
	bool native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

	MidiDriver *driver = MidiDriver::createMidi(midiDriver);
	if (midiDriver == MD_ADLIB) {
		// In this case we should play the Adlib tracks, but for now
		// the automagic MIDI-to-Adlib conversion will do.
	} else if (native_mt32) {
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
	}

	_midi = new MusicPlayer(driver, this);
	assert(_midi);
	_midi->hasNativeMT32(native_mt32);
	_midi->setVolume(255);
	
	_saveFileMan = _system->getSavefileManager();
	assert(_saveFileMan);	
	_res = new Resource(this);
	assert(_res);
	_screen = new Screen(this, _system);
	assert(_screen);
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

	_fastMode = false;
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
	_brandonDrawFrame = 113;
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

	_configTalkspeed = 1;
	
	_marbleVaseItem = -1;
	memset(_foyerItemTable, -1, sizeof(_foyerItemTable));
	_mouseState = _itemInHand = -1;
	_handleInput = false;
	
	_currentRoom = 0xFFFF;
	_scenePhasingFlag = 0;
	_lastProcessedItem = 0;
	_lastProcessedItemHeight = 16;
	
	_hidPage = _screenPage = 0;
	
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
	
	memset(_specialPalettes, 0, sizeof(_specialPalettes));
	_mousePressFlag = false;
	
	 _targetName = detector._targetName;

	return 0;
}

KyraEngine::~KyraEngine() {
	closeFinalWsa();
	_scriptInterpreter->unloadScript(_npcScriptData);
	_scriptInterpreter->unloadScript(_scriptClickData);

	delete _debugger;
	delete _sprites;
	delete _animator;
	delete _screen;
	delete _res;
	delete _midi;
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
	
	free(_buttonShape0);
	free(_buttonShape1);
	free(_buttonShape2);
	free(_buttonShape3);
	free(_buttonShape4);
	free(_buttonShape5);
	
	for (int i = 0; i < ARRAYSIZE(_shapes); ++i) {
		if (_shapes[i] != 0) {
			free(_shapes[i]);
			_shapes[i] = 0;
			for (int i2 = 0; i2 < ARRAYSIZE(_shapes); i2++) {
				if (_shapes[i2] == _shapes[i] && i2 != i) {
					_shapes[i2] = 0;
				}
			}
		}
	}
	for (int i = 0; i < ARRAYSIZE(_sceneAnimTable); ++i) {
		free(_sceneAnimTable[i]);
	}
	free(_unkPtr1);
	free(_unkPtr2);
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int KyraEngine::go() {
	_quitFlag = false;
	uint32 sz;

	res_loadResources();
	if (_features & GF_FLOPPY) {
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
		startup();
		resetGameFlag(0xEF);
		mainLoop();
	}
	quitGame();
	return 0;
}

void KyraEngine::startup() {
	debug(9, "KyraEngine::startup()");
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	_screen->setTextColorMap(colorMap);
//	_screen->setFont(Screen::FID_6_FNT);
	_screen->setAnimBlockPtr(3750);
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	loadMouseShapes();
	_currentCharacter = &_characterList[0];
	for (int i = 1; i < 5; ++i)
		setCharacterDefaultFrame(i);
	for (int i = 5; i <= 10; ++i)
		setCharactersPositions(i);
	setCharactersHeight();
	resetBrandonPosionFlags();
	_maskBuffer = _screen->getPagePtr(5);
	_screen->_curPage = 0;
	// XXX
	for (int i = 0; i < 0x0C; ++i) {
		int size = _screen->getRectSize(3, 24);
		_shapes[365+i] = (byte*)malloc(size);
	}
	_unkPtr1 = (uint8*)malloc(_screen->getRectSize(1, 144));
	memset(_unkPtr1, 0, _screen->getRectSize(1, 144));
	_unkPtr2 = (uint8*)malloc(_screen->getRectSize(1, 144));
	memset(_unkPtr2, 0, _screen->getRectSize(1, 144));
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
	initAnimStateList();
	setCharactersInDefaultScene();

	_gameSpeed = 50;
	_tickLength = (uint8)(1000.0 / _gameSpeed);

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
	snd_setSoundEffectFile(1);
	enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);
}

void KyraEngine::delay(uint32 amount, bool update) {
	OSystem::Event event;
	char saveLoadSlot[20];

	_mousePressFlag = false;
	uint32 start = _system->getMillis();
	do {
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == 'q' || event.kbd.keycode == 27) {
					_quitFlag = true;
				} else if (event.kbd.keycode == 'd' && !_debugger->isAttached()) {
					_debugger->attach();
				} else if (event.kbd.keycode >= '0' && event.kbd.keycode <= '9' && 
						(event.kbd.flags == OSystem::KBD_CTRL || event.kbd.flags == OSystem::KBD_ALT)) {
					sprintf(saveLoadSlot, "%s.00%d", _targetName.c_str(), event.kbd.keycode - '0');
					if (event.kbd.flags == OSystem::KBD_CTRL)
						loadGame(saveLoadSlot);
					else
						saveGame(saveLoadSlot, saveLoadSlot);
				}	else if (event.kbd.flags == OSystem::KBD_CTRL && event.kbd.keycode == 'f') {
						_fastMode = !_fastMode;
				}
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				break;
			case OSystem::EVENT_QUIT:
				quitGame();
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				_mousePressFlag = true;
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
				}
				break;
			default:
				break;
			}
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		_sprites->updateSceneAnims();
		if (update)
			_animator->updateAllObjectShapes();

		if (_currentCharacter->sceneId == 210) {
			_animator->updateKyragemFading();
		}

		if (amount > 0) {
			_system->delayMillis((amount > 10) ? 10 : amount);
		}
	} while (!_fastMode && _system->getMillis() < start + amount);
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

void KyraEngine::mainLoop() {
	debug(9, "KyraEngine::mainLoop()");

	while (!_quitFlag) {
		int32 frameTime = (int32)_system->getMillis();
		if (_currentCharacter->sceneId == 210) {
			_animator->updateKyragemFading();
			if (seq_playEnd()) {
				if (_deathHandler != 8)
					break;
			}
		}
		
		if (_deathHandler != 0xFF) {
			// this is only used until the original gui is implemented
			GUI::MessageDialog dialog("Brandon is dead! Game over!", "Quit");
			dialog.runModal();
			break;
		}
		
		if (_brandonStatusBit & 2) {
			if (_brandonStatusBit0x02Flag)
				animRefreshNPC(0);
		}
		if (_brandonStatusBit & 0x20) {
			if (_brandonStatusBit0x20Flag) {
				animRefreshNPC(0);
				_brandonStatusBit0x20Flag = 0;
			}
		}
		
		processButtonList(_buttonList);
		updateMousePointer();
		updateGameTimers();
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		updateTextFade();

		_handleInput = true;
		delay((frameTime + _gameSpeed) - _system->getMillis(), true);
		_handleInput = false;
	}
}

void KyraEngine::quitGame() {
	res_unloadResources();

	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		_movieObjects[i]->close();
		delete _movieObjects[i];
		_movieObjects[i] = 0;
	}

	_system->quit();
}

void KyraEngine::loadPalette(const char *filename, uint8 *palData) {
	debug(9, "KyraEngine::loadPalette('%s' 0x%X)", filename, palData);
	uint32 fileSize = 0;
	uint8 *srcData = _res->fileData(filename, &fileSize);

	if (palData && fileSize) {
		debug(9, "Loading a palette of size %i from '%s'", fileSize, filename);
		memcpy(palData, srcData, fileSize);
	}
	delete [] srcData;
}

void KyraEngine::loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData) {
	debug(9, "KyraEngine::copyBitmap('%s', %d, %d, 0x%X)", filename, tempPage, dstPage, palData);
	uint32 fileSize;
	uint8 *srcData = _res->fileData(filename, &fileSize);
	uint8 compType = srcData[2];
	uint32 imgSize = READ_LE_UINT32(srcData + 4);
	uint16 palSize = READ_LE_UINT16(srcData + 8);
	if (palData && palSize) {
		debug(9, "Loading a palette of size %i from %s", palSize, filename);
		memcpy(palData, srcData + 10, palSize);		
	}
	uint8 *srcPtr = srcData + 10 + palSize;
	uint8 *dstData = _screen->getPagePtr(dstPage);
	switch (compType) {
	case 0:
		memcpy(dstData, srcPtr, imgSize);
		break;
	case 3:
		Screen::decodeFrame3(srcPtr, dstData, imgSize);
		break;
	case 4:
		Screen::decodeFrame4(srcPtr, dstData, imgSize);
		break;
	default:
		error("Unhandled bitmap compression %d", compType);
		break;
	}
	delete[] srcData;
}

void KyraEngine::waitTicks(int ticks) {
	debug(9, "KyraEngine::waitTicks(%d)", ticks);
	const uint32 end = _system->getMillis() + ticks * 1000 / 60;
	do {
		OSystem::Event event;
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_QUIT:
				_quitFlag = true;
				quitGame();
				break;
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.flags == OSystem::KBD_CTRL) {
					if (event.kbd.keycode == 'f') {
						_fastMode = !_fastMode;
					}
				} else if (event.kbd.keycode == 13 || event.kbd.keycode == 32 || event.kbd.keycode == 27) {
					_abortIntroFlag = true;
				}
				break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
	} while (!_fastMode && _system->getMillis() < end);
}

void KyraEngine::delayWithTicks(int ticks) {
	uint32 nextTime = _system->getMillis() + ticks * _tickLength;
	while (_system->getMillis() < nextTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (_currentCharacter->sceneId == 210) {
			_animator->updateKyragemFading();
			seq_playEnd();
		}
	}
}

void KyraEngine::seq_demo() {
	debug(9, "KyraEngine::seq_demo()");

	snd_playTheme(MUSIC_INTRO, 2);

	loadBitmap("START.CPS", 7, 7, _screen->_currentPalette);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	waitTicks(60);
	_screen->fadeToBlack();

	_screen->clearPage(0);
	loadBitmap("TOP.CPS", 7, 7, NULL);
	loadBitmap("BOTTOM.CPS", 5, 5, _screen->_currentPalette);
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 0);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	
	_seq->playSequence(_seq_WestwoodLogo, true);
	waitTicks(60);

	_seq->playSequence(_seq_KyrandiaLogo, true);

	_screen->fadeToBlack();
	_screen->clearPage(2);
	_screen->clearPage(0);

	_seq->playSequence(_seq_Demo1, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo2, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo3, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo4, true);

	_screen->clearPage(0);
	loadBitmap("FINAL.CPS", 7, 7, _screen->_currentPalette);
	_screen->_curPage = 0;
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	waitTicks(60);
	_screen->fadeToBlack();
	_midi->stopMusic();
}

void KyraEngine::seq_intro() {
	debug(9, "KyraEngine::seq_intro()");
	if (_features & GF_TALKIE) {
		_res->loadPakFile("INTRO.VRM");
	}
	
	static const IntroProc introProcTable[] = {
		&KyraEngine::seq_introLogos,
		&KyraEngine::seq_introStory,
		&KyraEngine::seq_introMalcolmTree,
		&KyraEngine::seq_introKallakWriting,
		&KyraEngine::seq_introKallakMalcolm
	};
	_skipIntroFlag = true; // only true if user already saved the game once
	_seq->setCopyViewOffs(true);
	_screen->setFont(Screen::FID_8_FNT);
	snd_playTheme(MUSIC_INTRO, 2);
 	snd_setSoundEffectFile(MUSIC_INTRO);
	_text->setTalkCoords(144);
	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		(this->*introProcTable[i])();
	}
	_text->setTalkCoords(136);
	waitTicks(30);
	_seq->setCopyViewOffs(false);
	_midi->stopMusic();
	if (_features & GF_TALKIE) {
		_res->unloadPakFile("INTRO.VRM");
	}
	res_unloadResources(RES_INTRO | RES_OUTRO);
}

void KyraEngine::seq_introLogos() {
	debug(9, "KyraEngine::seq_introLogos()");
	_screen->clearPage(0);
	loadBitmap("TOP.CPS", 7, 7, NULL);
	loadBitmap("BOTTOM.CPS", 5, 5, _screen->_currentPalette);
	_screen->_curPage = 0;
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 0);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 0);
	_system->copyRectToScreen(_screen->getPagePtr(0), 320, 0, 0, 320, 200);
	_screen->fadeFromBlack();
	
	if (_seq->playSequence(_seq_WestwoodLogo, _skipIntroFlag)) {
		_screen->fadeToBlack();
		_screen->clearPage(0);
		return;
	}
	waitTicks(60);
	if (_seq->playSequence(_seq_KyrandiaLogo, _skipIntroFlag)) {
		_screen->fadeToBlack();
		_screen->clearPage(0);
		return;
	}
	_screen->fillRect(0, 179, 319, 199, 0);

	int y1 = 8;
	int h1 = 175;
	int y2 = 176;
	int h2 = 0;
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 2);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 2);
	do {
		if (h1 > 0) {
			_screen->copyRegion(0, y1, 0, 8, 320, h1, 2, 0);
		}
		++y1;
		--h1;
		if (h2 > 0) {
			_screen->copyRegion(0, 64, 0, y2, 320, h2, 4, 0);
		}
		--y2;
		++h2;
		_screen->updateScreen();
		waitTicks(1);
	} while (y2 >= 64);

	_seq->playSequence(_seq_Forest, true);
}

void KyraEngine::seq_introStory() {
	debug(9, "KyraEngine::seq_introStory()");
	_screen->clearPage(3);
	_screen->clearPage(0);
	if ((_features & GF_ENGLISH) && (_features & GF_TALKIE)) {
		loadBitmap("TEXT_ENG.CPS", 3, 3, 0);
	} else if (_features & GF_GERMAN) {
		loadBitmap("TEXT_GER.CPS", 3, 3, 0);
	} else if (_features & GF_FRENCH) {
		loadBitmap("TEXT_FRE.CPS", 3, 3, 0);
	} else if (_features & GF_SPANISH) {
		loadBitmap("TEXT_SPA.CPS", 3, 3, 0);
	} else if ((_features & GF_ENGLISH) && (_features & GF_FLOPPY)) {
		loadBitmap("TEXT.CPS", 3, 3, 0);
	} else {
		warning("no story graphics file found");
	}
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 3, 0);
	_screen->updateScreen();
	waitTicks(360);
}

void KyraEngine::seq_introMalcolmTree() {
	debug(9, "KyraEngine::seq_introMalcolmTree()");
	_screen->_curPage = 0;
	_screen->clearPage(3);
	_seq->playSequence(_seq_MalcolmTree, true);
}

void KyraEngine::seq_introKallakWriting() {
	debug(9, "KyraEngine::seq_introKallakWriting()");
	_seq->makeHandShapes();
	_screen->setAnimBlockPtr(5060);
	_screen->_charWidth = -2;
	_screen->clearPage(3);
	_seq->playSequence(_seq_KallakWriting, true);
}

void KyraEngine::seq_introKallakMalcolm() {
	debug(9, "KyraEngine::seq_introKallakMalcolm()");
	_screen->clearPage(3);
	_seq->playSequence(_seq_KallakMalcolm, true);
}

void KyraEngine::seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly) {
	debug(9, "seq_createAmuletJewel(%d, %d, %d, %d)", jewel, page, noSound, drawOnly);
	const static uint16 specialJewelTable[] = {
		0x167, 0x162, 0x15D, 0x158, 0x153, 0xFFFF
	};
	const static uint16 specialJewelTable1[] = {
		0x14F, 0x154, 0x159, 0x15E, 0x163, 0xFFFF
	};
	const static uint16 specialJewelTable2[] = {
		0x150, 0x155, 0x15A, 0x15F, 0x164, 0xFFFF
	};
	const static uint16 specialJewelTable3[] = {
		0x151, 0x156, 0x15B, 0x160, 0x165, 0xFFFF
	};
	const static uint16 specialJewelTable4[] = {
		0x152, 0x157, 0x15C, 0x161, 0x166, 0xFFFF
	};
	if (!noSound)
		// snd_playSoundEffect(0x5F)
	_screen->hideMouse();
	if (!drawOnly) {
		for (int i = 0; specialJewelTable[i] != 0xFFFF; ++i) {
			_screen->drawShape(page, _shapes[4+specialJewelTable[i]], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
			_screen->updateScreen();
			delayWithTicks(3);
		}
		
		const uint16 *opcodes = 0;
		switch (jewel - 1) {
			case 0:
				opcodes = specialJewelTable1;
				break;
				
			case 1:
				opcodes = specialJewelTable2;
				break;
				
			case 2:
				opcodes = specialJewelTable3;
				break;
				
			case 3:
				opcodes = specialJewelTable4;
				break;
		}
		
		if (opcodes) {
			for (int i = 0; opcodes[i] != 0xFFFF; ++i) {
				_screen->drawShape(page, _shapes[4+opcodes[i]], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
				_screen->updateScreen();
				delayWithTicks(3);
			}
		}
	}
	_screen->drawShape(page, _shapes[327+jewel], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	_screen->showMouse();
	setGameFlag(0x55+jewel);
}

void KyraEngine::seq_brandonHealing() {
	debug(9, "seq_brandonHealing()");
	if (!(_deathHandler & 8))
		return;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShapeTable);
	setupShapes123(_healingShapeTable, 22, 0);
	setBrandonAnimSeqSize(3, 48);
	// snd_playSoundEffect(0x53);
	for (int i = 123; i <= 144; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	for (int i = 125; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_brandonHealing2() {
	debug(9, "seq_brandonHealing2()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShape2Table);
	setupShapes123(_healingShape2Table, 30, 0);
	resetBrandonPoisonFlags();
	setBrandonAnimSeqSize(3, 48);
	// snd_playSoundEffect(0x50);
	for (int i = 123; i <= 152; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
	assert(_poisonGone);
	characterSays(_poisonGone[0], 0, -2);
	characterSays(_poisonGone[1], 0, -2);
}

void KyraEngine::seq_poisonDeathNow(int now) {
	debug(9, "seq_poisonDeathNow(%d)", now);
	if (!(_brandonStatusBit & 1))
		return;
	++_poisonDeathCounter;
	if (now)
		_poisonDeathCounter = 2;
	if (_poisonDeathCounter >= 2) {
		// XXX
		assert(_thePoison);
		characterSays(_thePoison[0], 0, -2);
		characterSays(_thePoison[1], 0, -2);
		seq_poisonDeathNowAnim();
		_deathHandler = 3;
	} else {
		assert(_thePoison);
		characterSays(_thePoison[2], 0, -2);
		characterSays(_thePoison[3], 0, -2);
	}
}

void KyraEngine::seq_poisonDeathNowAnim() {
	debug(9, "seq_poisonDeathNowAnim()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_posionDeathShapeTable);
	setupShapes123(_posionDeathShapeTable, 20, 0);
	setBrandonAnimSeqSize(8, 48);
	
	_currentCharacter->currentAnimFrame = 124;
	animRefreshNPC(0);
	delayWithTicks(30);
	
	_currentCharacter->currentAnimFrame = 123;
	animRefreshNPC(0);
	delayWithTicks(30);
	
	for (int i = 125; i <= 139; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(60);
	
	for (int i = 140; i <= 142; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(60);
	
	resetBrandonAnimSeqSize();
	freeShapes123();
	_animator->restoreAllObjectBackgrounds();
	_currentCharacter->x1 = _currentCharacter->x2 = -1;
	_currentCharacter->y1 = _currentCharacter->y2 = -1;
	_animator->preserveAllBackgrounds();
	_screen->showMouse();
}

void KyraEngine::seq_playFluteAnimation() {
	debug(9, "seq_playFluteAnimation()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	setupShapes123(_fluteAnimShapeTable, 36, 0);
	setBrandonAnimSeqSize(3, 75);
	for (int i = 123; i <= 130; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(2);
	}
	
	int delayTime = 0, soundType = 0;
	if (queryGameFlag(0x85)) {
		// snd_playSoundEffect(0x63);
		delayTime = 9;
		soundType = 3;
	} else if (queryGameFlag(0x86)) {
		// snd_playSoundEffect(0x61);
		delayTime = 2;
		soundType = 1;
	} else {
		// snd_playSoundEffect(0x62);
		delayTime = 2;
		soundType = 2;
	}
	
	for (int i = 131; i <= 158; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	
	for (int i = 126; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
	
	if (soundType == 1) {
		assert(_fluteString);
		characterSays(_fluteString[0], 0, -2);
	} else if (soundType == 2) {
		assert(_fluteString);
		characterSays(_fluteString[1], 0, -2);
	}
}

void KyraEngine::seq_winterScroll1() {
	debug(9, "seq_winterScroll1()");
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	assert(_winterScroll1Table);
	assert(_winterScroll2Table);
	setupShapes123(_winterScrollTable, 7, 0);
	setBrandonAnimSeqSize(5, 66);
	
	for (int i = 123; i <= 129; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	freeShapes123();
	// snd_playSoundEffect(0x20);
	setupShapes123(_winterScroll1Table, 35, 0);
	
	for (int i = 123; i <= 146; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	if (_currentCharacter->sceneId == 41 && !queryGameFlag(0xA2)) {
		// snd_playSoundEffect(0x20);
		_sprites->_anims[0].play = false;
		_animator->sprites()[0].active = 0;
		_sprites->_anims[1].play = true;
		_animator->sprites()[1].active = 1;
	}
	
	for (int i = 147; i <= 157; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	if (_currentCharacter->sceneId == 117 && !queryGameFlag(0xB3)) {
		for (int i = 0; i <= 7; ++i) {
			_sprites->_anims[i].play = false;
			_animator->sprites()[i].active = 0;
		}
		uint8 tmpPal[768];
		memcpy(tmpPal, _screen->_currentPalette, 768);
		memcpy(&tmpPal[684], palTable2()[0], 60);
		_screen->fadePalette(tmpPal, 72);
	} else {
		delayWithTicks(120);
	}
	
	freeShapes123();
	setupShapes123(_winterScroll2Table, 4, 0);
	
	for (int i = 123; i <= 126; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_winterScroll2() {
	debug(9, "seq_winterScroll2()");	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	setupShapes123(_winterScrollTable, 7, 0);
	setBrandonAnimSeqSize(5, 66);
	
	for (int i = 123; i <= 128; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(120);
	
	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonInv() {
	debug(9, "seq_makeBrandonInv()");
	if (_deathHandler == 8)
		return;

	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	_brandonStatusBit |= 0x20;
	setTimerCountdown(18, 2700);
	_brandonStatusBit |= 0x40;
	// snd_playSoundEffect(0x77);
	_brandonInvFlag = 0;
	while (_brandonInvFlag <= 0x100) {
		animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag += 0x10;
	}
	_brandonStatusBit &= 0xFFBF;
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonNormal() {
	debug(9, "seq_makeBrandonNormal()");
	_screen->hideMouse();
	_brandonStatusBit |= 0x40;
	// snd_playSoundEffect(0x77);
	_brandonInvFlag = 0x100;
	while (_brandonInvFlag >= 0) {
		animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag -= 0x10;
	}
	_brandonInvFlag = 0;
	_brandonStatusBit &= 0xFF9F;
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonNormal2() {
	debug(9, "seq_makeBrandonNormal2()");
	_screen->hideMouse();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	setBrandonAnimSeqSize(5, 48);
	_brandonStatusBit &= 0xFFFD;
	// snd_playSoundEffect(0x6C);
	for (int i = 138; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	setBrandonAnimSeqSize(4, 48);
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245) {
		_screen->fadeSpecialPalette(31, 234, 13, 4);
	} else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186) {
		_screen->fadeSpecialPalette(14, 228, 15, 4);
	}
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_makeBrandonWisp() {
	debug(9, "seq_makeBrandonWisp()");
	if (_deathHandler == 8)
		return;
	
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}	
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	setBrandonAnimSeqSize(5, 48);
	// snd_playSoundEffect(0x6C);
	for (int i = 123; i <= 138; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	_brandonStatusBit |= 2;
	if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
		setTimerCountdown(14, 18000);
	} else {
		setTimerCountdown(14, 7200);
	}
	_brandonDrawFrame = 113;
	_brandonStatusBit0x02Flag = 1;
	_currentCharacter->currentAnimFrame = 113;
	animRefreshNPC(0);
	_animator->updateAllObjectShapes();
	if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245) {
		_screen->fadeSpecialPalette(30, 234, 13, 4);
	} else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186) {
		_screen->fadeSpecialPalette(14, 228, 15, 4);
	}
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_dispelMagicAnimation() {
	debug(9, "seq_dispelMagicAnimation()");
	if (_deathHandler == 8)
		return;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	if (_currentCharacter->sceneId == 210 && _currentCharacter->sceneId < 160)
		_currentCharacter->facing = 3;
	if (_malcolmFlag == 7 && _beadStateVar == 3) {
		_beadStateVar = 6;
		_unkEndSeqVar5 = 2;
		_malcolmFlag = 10;
	}
	checkAmuletAnimFlags();
	setGameFlag(0xEE);
	assert(_magicAnimationTable);
	setupShapes123(_magicAnimationTable, 5, 0);
	setBrandonAnimSeqSize(8, 49);
	// snd_playSoundEffect(0x15);
	for (int i = 123; i <= 127; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	
	delayWithTicks(120);
	
	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(10);
	}
	resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_fillFlaskWithWater(int item, int type) {
	debug(9, "seq_fillFlaskWithWater(%d, %d)", item, type);
	int newItem = -1;
	static const uint8 flaskTable1[] = { 0x46, 0x48, 0x4A, 0x4C };
	static const uint8 flaskTable2[] = { 0x47, 0x49, 0x4B, 0x4D };
	
	if (item >= 60 && item <= 77) {
		assert(_flaskFull);
		characterSays(_flaskFull[0], 0, -2);
	} else if (item == 78) {
		assert(type >= 0 && type < ARRAYSIZE(flaskTable1));
		newItem = flaskTable1[type];
	} else if (item == 79) {
		assert(type >= 0 && type < ARRAYSIZE(flaskTable2));
		newItem = flaskTable2[type];
	}
	
	if (newItem == -1)
		return;
	
	_screen->hideMouse();
	setMouseItem(newItem);
	_screen->showMouse();
	_itemInHand = newItem;
	assert(_fullFlask);
	assert(type < _fullFlask_Size && type >= 0);
	characterSays(_fullFlask[type], 0, -2);
}

void KyraEngine::seq_playDrinkPotionAnim(int unk1, int unk2, int flags) {
	debug(9, "KyraEngine::seq_playDrinkPotionAnim(%d, %d, %d)", unk1, unk2, flags);
	// XXX
	_screen->hideMouse();
	checkAmuletAnimFlags();
	_currentCharacter->facing = 5;
	animRefreshNPC(0);
	assert(_drinkAnimationTable);
	setupShapes123(_drinkAnimationTable, 9, flags);
	setBrandonAnimSeqSize(5, 54);
	
	for (int i = 123; i <= 131; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(5);
	}	
	// snd_playSoundEffect(0x34);
	for (int i = 0; i < 2; ++i) {
		_currentCharacter->currentAnimFrame = 130;
		animRefreshNPC(0);
		delayWithTicks(7);
		_currentCharacter->currentAnimFrame = 131;
		animRefreshNPC(0);
		delayWithTicks(7);
	}
	
	if (unk2) {
		// XXX
	}
	
	for (int i = 131; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(5);
	}
	
	resetBrandonAnimSeqSize();	
	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

int KyraEngine::seq_playEnd() {
	debug(9, "KyraEngine::seq_playEnd()");
	if (_endSequenceSkipFlag) {
		return 0;
	}
	if (_deathHandler == 8) {
		return 0;
	}
	_screen->_curPage = 2;
	if (_endSequenceNeedLoading) {
		// XXX
		setupPanPages();
		_finalA = new WSAMovieV1(this);
		assert(_finalA);
		_finalA->open("finala.wsa", 1, 0);
		_finalB = new WSAMovieV1(this);
		assert(_finalB);
		_finalB->open("finalb.wsa", 1, 0);
		_finalC = new WSAMovieV1(this);
		assert(_finalC);
		_endSequenceNeedLoading = 0;
		_finalC->open("finalc.wsa", 1, 0);
		_screen->_curPage = 0;
		_beadStateVar = 0;
		_malcolmFlag = 0;
		// wired stuff with _unkEndSeqVar2 which needs timer handling
		_screen->copyRegion(312, 0, 312, 0, 8, 136, 0, 2);
	}
	if (handleMalcolmFlag()) {
		_beadStateVar = 0;
		_malcolmFlag = 12;
		handleMalcolmFlag();
		handleBeadState();
		closeFinalWsa();
		if (_deathHandler == 8) {
			_screen->_curPage = 0;
			checkAmuletAnimFlags();
			seq_brandonToStone();
			waitTicks(60);
			return 1;
		} else {
			_endSequenceSkipFlag = 1;
			if (_text->printed()) {
				_text->restoreTalkTextMessageBkgd(2, 0);
			}
			_screen->_curPage = 0;
			_screen->hideMouse();
			_screen->fadeSpecialPalette(32, 228, 20, 60);
			waitTicks(60);
			loadBitmap("GEMHEAL.CPS", 3, 3, _screen->_currentPalette);
			_screen->setScreenPalette(_screen->_currentPalette);
			_screen->shuffleScreen(8, 8, 304, 128, 2, 0, 1, 0);
			uint32 nextTime = _system->getMillis() + 120 * _tickLength;
			_finalA = new WSAMovieV1(this);
			assert(_finalA);
			_finalA->open("finald.wsa", 1, 0);
			_finalA->_x = _finalA->_y = 8;
			_finalA->_drawPage = 0;
			while (_system->getMillis() < nextTime) {}
			// snd_playSoundEffect(0x40);
			for (int i = 0; i < 22; ++i) {
				while (_system->getMillis() < nextTime) {}
				if (i == 4) {
					// snd_playSoundEffect(0x3E);
				} else if (i == 20) {
					// snd_playSoundEffect(0x0E);
				}
				nextTime = _system->getMillis() + 8 * _tickLength;
				_finalA->displayFrame(i);
				_screen->updateScreen();
			}
			delete _finalA;
			_finalA = 0;
			seq_playEnding();
			return 1;
		}
	} else {
		handleBeadState();
		_screen->bitBlitRects();
		_screen->updateScreen();
		_screen->_curPage = 0;
	}
	return 0;
}

void KyraEngine::seq_brandonToStone() {
	debug(9, "KyraEngine::seq_brandonToStone()");
	_screen->hideMouse();
	assert(_brandonStoneTable);
	setupShapes123(_brandonStoneTable, 14, 0);
	setBrandonAnimSeqSize(5, 51);
	for (int i = 123; i <= 136; ++i) {
		_currentCharacter->currentAnimFrame = i;
		animRefreshNPC(0);
		delayWithTicks(8);
	}
	resetBrandonAnimSeqSize();
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine::seq_playEnding() {
	debug(9, "KyraEngine::seq_playEnding()");
	_screen->hideMouse();
	res_unloadResources(RES_INGAME);
	res_loadResources(RES_OUTRO);
	loadBitmap("REUNION.CPS", 3, 3, _screen->_currentPalette);
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	_screen->_curPage = 0;
	// XXX
	assert(_homeString);
	drawSentenceCommand(_homeString[0], 179);
	_screen->_curPage = 0;
	_screen->fadeToBlack();
	_seq->playSequence(_seq_Reunion, false);
	_screen->fadeToBlack();
	_screen->showMouse();
	seq_playCredits();
}

void KyraEngine::seq_playCredits() {
	debug(9, "KyraEngine::seq_playCredits()");
	static const uint8 colorMap[] = { 0, 0, 0xC, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	_screen->hideMouse();
	uint32 sz = 0;
	if (_features & GF_FLOPPY) {
		_screen->loadFont(Screen::FID_CRED6_FNT, _res->fileData("CREDIT6.FNT", &sz));
		_screen->loadFont(Screen::FID_CRED8_FNT, _res->fileData("CREDIT8.FNT", &sz));
	}
	loadBitmap("CHALET.CPS", 2, 2, _screen->_currentPalette);
	_screen->setScreenPalette(_screen->_currentPalette);
	_screen->setCurPage(0);
	_screen->clearCurPage();
	_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	_screen->setTextColorMap(colorMap);
	_screen->_charWidth = -1;
	// delete
	_screen->updateScreen();
	// XXX
	waitTicks(120); // wait until user presses escape normally
	_screen->fadeToBlack();
	_screen->clearCurPage();
	_screen->showMouse();
}

bool KyraEngine::seq_skipSequence() const {
	debug(9, "KyraEngine::seq_skipSequence()");
	return _quitFlag || _abortIntroFlag;
}

void KyraEngine::snd_playTheme(int file, int track) {
	debug(9, "KyraEngine::snd_playTheme(%d)", file);
	assert(file < _xmidiFilesCount);
	_curMusicTheme = _newMusicTheme = file;
	_midi->playMusic(_xmidiFiles[file]);
	_midi->playTrack(track, false);
}

void KyraEngine::snd_playTrack(int track) {
	debug(9, "KyraEngine::snd_playTrack(%d)", track);
	_midi->playTrack(track, false);
}

void KyraEngine::snd_setSoundEffectFile(int file) {
	debug(9, "KyraEngine::snd_setSoundEffectFile(%d)", file);
	assert(file < _xmidiFilesCount);
	_midi->loadSoundEffectFile(_xmidiFiles[file]);
}

void KyraEngine::snd_playSoundEffect(int track) {
	debug(9, "KyraEngine::snd_playSoundEffect(%d)", track);
	_midi->playSoundEffect(track);
}

void KyraEngine::snd_playVoiceFile(int id) {
	debug(9, "KyraEngine::snd_playVoiceFile(%d)", id);
	char vocFile[9];
	assert(id >= 0 && id < 9999);
	sprintf(vocFile, "%03d.VOC", id);
	uint32 fileSize = 0;
	byte *fileData = 0;
	fileData = _res->fileData(vocFile, &fileSize);
	assert(fileData);
	Common::MemoryReadStream vocStream(fileData, fileSize);
	_mixer->stopHandle(_vocHandle);
	_currentVocFile = makeVOCStream(vocStream);
	if (_currentVocFile)
		_mixer->playInputStream(Audio::Mixer::kSpeechSoundType, &_vocHandle, _currentVocFile);
	delete fileData;
	fileSize = 0;
}

bool KyraEngine::snd_voicePlaying() {
	return _mixer->isSoundHandleActive(_vocHandle);
}

void KyraEngine::snd_startTrack() {
	debug(9, "KyraEngine::snd_startTrack()");
	_midi->startTrack();
}

void KyraEngine::snd_haltTrack() {
	debug(9, "KyraEngine::snd_haltTrack()");
	_midi->haltTrack();
}

void KyraEngine::loadMouseShapes() {
	loadBitmap("MOUSE.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_shapes[4] = _screen->encodeShape(0, 0, 8, 10, 0);
	_shapes[5] = _screen->encodeShape(0, 0x17, 0x20, 7, 0);
	_shapes[6] = _screen->encodeShape(0x50, 0x12, 0x10, 9, 0);
	_shapes[7] = _screen->encodeShape(0x60, 0x12, 0x10, 11, 0);
	_shapes[8] = _screen->encodeShape(0x70, 0x12, 0x10, 9, 0);
	_shapes[9] = _screen->encodeShape(0x80, 0x12, 0x10, 11, 0);
	_shapes[10] = _screen->encodeShape(0x90, 0x12, 0x10, 10, 0);
	_shapes[364] = _screen->encodeShape(0x28, 0, 0x10, 13, 0);
	_screen->setMouseCursor(1, 1, 0);
	_screen->setMouseCursor(1, 1, _shapes[4]);
	_screen->setShapePages(5, 3);
}

void KyraEngine::loadCharacterShapes() {
	int curImage = 0xFF;
	int videoPage = _screen->_curPage;
	_screen->_curPage = 2;
	for (int i = 0; i < 115; ++i) {	
		assert(i < _defaultShapeTableSize);
		Shape *shape = &_defaultShapeTable[i];
		if (shape->imageIndex == 0xFF) {
			_shapes[i+7+4] = 0;
			continue;
		}
		if (shape->imageIndex != curImage) {
			assert(shape->imageIndex < _characterImageTableSize);
			loadBitmap(_characterImageTable[shape->imageIndex], 3, 3, 0);
			curImage = shape->imageIndex;
		}
		_shapes[i+7+4] = _screen->encodeShape(shape->x<<3, shape->y, shape->w<<3, shape->h, 1);
	}
	_screen->_curPage = videoPage;
}

void KyraEngine::loadSpecialEffectShapes() {
	loadBitmap("EFFECTS.CPS", 3, 3, 0);
	_screen->_curPage = 2;
 
	int currShape; 
	for (currShape = 173; currShape < 183; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-173) * 24, 0, 24, 24, 1);
 
	for (currShape = 183; currShape < 190; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-183) * 24, 24, 24, 24, 1);
 
	for (currShape = 190; currShape < 201; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-190) * 24, 48, 24, 24, 1);
 
	for (currShape = 201; currShape < 206; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-201) * 16, 106, 16, 16, 1);
}

int KyraEngine::findDuplicateItemShape(int shape) {
	static uint8 dupTable[] = {
		0x48, 0x46, 0x49, 0x47, 0x4a, 0x46, 0x4b, 0x47,
		0x4c, 0x46, 0x4d, 0x47, 0x5b, 0x5a, 0x5c, 0x5a,
		0x5d, 0x5a, 0x5e, 0x5a, 0xFF, 0xFF
	};

	int i = 0;

	while (dupTable[i] != 0xFF) {
		if (dupTable[i] == shape)
			return dupTable[i+1];
		i += 2;
	}
	return -1;
}

void KyraEngine::loadItems() {
	int shape;

	loadBitmap("JEWELS3.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	_shapes[327] = 0;

	for (shape = 1; shape < 6; shape++ )
		_shapes[327 + shape] = _screen->encodeShape((shape - 1) * 32, 0, 32, 17, 0);

	for (shape = 330; shape <= 334; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-330) * 32, 102, 32, 17, 0);

	for (shape = 335; shape <= 339; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-335) * 32, 17,  32, 17, 0);

	for (shape = 340; shape <= 344; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-340) * 32, 34,  32, 17, 0);

	for (shape = 345; shape <= 349; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-345) * 32, 51,  32, 17, 0);

	for (shape = 350; shape <= 354; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-350) * 32, 68,  32, 17, 0);

	for (shape = 355; shape <= 359; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-355) * 32, 85,  32, 17, 0);


	loadBitmap("ITEMS.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	for (int i = 0; i < 107; i++) {
		shape = findDuplicateItemShape(i);

		if (shape != -1)
			_shapes[220 + i] = _shapes[220 + shape];
		else
			_shapes[220 + i] = _screen->encodeShape( (i % 20) * 16, i/20 * 16, 16, 16, 0);
	}

	uint32 size;
	uint8 *fileData = _res->fileData("_ITEM_HT.DAT", &size);
	assert(fileData);

	for (int i = 0; i < 107; i++) {
		_itemTable[i].height = fileData[i];
		_itemTable[i].unk1 = _itemTable[i].unk2 = 0;
	}

	delete[] fileData;
}

void KyraEngine::loadButtonShapes() {
	loadBitmap("BUTTONS2.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_buttonShape0 = _screen->encodeShape(0, 0, 24, 14, 1);
	_buttonShape1 = _screen->encodeShape(24, 0, 24, 14, 1);
	_buttonShape2 = _screen->encodeShape(48, 0, 24, 14, 1);
	_buttonShape3 = _screen->encodeShape(0, 15, 24, 14, 1);
	_buttonShape4 = _screen->encodeShape(24, 15, 24, 14, 1);
	_buttonShape5 = _screen->encodeShape(48, 15, 24, 14, 1);
	_screen->_curPage = 0;
}

void KyraEngine::initMainButtonList() {
	_buttonList = &_buttonData[0];
	for (int i = 0; _buttonDataListPtr[i]; ++i) {
		_buttonList = initButton(_buttonList, _buttonDataListPtr[i]);
	}
}

void KyraEngine::loadMainScreen(int page) {
	if ((_features & GF_ENGLISH) && (_features & GF_TALKIE)) 
		loadBitmap("MAIN_ENG.CPS", page, page, 0);
	else if(_features & GF_FRENCH)
		loadBitmap("MAIN_FRE.CPS", page, page, 0);
	else if(_features & GF_GERMAN)
		loadBitmap("MAIN_GER.CPS", page, page, 0);
	else if ((_features & GF_ENGLISH) && (_features & GF_FLOPPY))
		loadBitmap("MAIN15.CPS", page, page, 0);
	else if (_features & GF_SPANISH)
		loadBitmap("MAIN_SPA.CPS", page, page, 0);
	else
		warning("no main graphics file found");
	
	uint8 *_pageSrc = _screen->getPagePtr(page);
	uint8 *_pageDst = _screen->getPagePtr(0);
	memcpy(_pageDst, _pageSrc, 320*200);
}

void KyraEngine::setCharactersInDefaultScene() {
	static const uint32 defaultSceneTable[][4] = {
		{ 0xFFFF, 0x0004, 0x0003, 0xFFFF },
		{ 0xFFFF, 0x0022, 0xFFFF, 0x0000 },
		{ 0xFFFF, 0x001D, 0x0021, 0xFFFF },
		{ 0xFFFF, 0x0000, 0x0000, 0xFFFF }
	};
	
	for (int i = 1; i < 5; ++i) {
		Character *cur = &_characterList[i];
		//cur->field_20 = 0;
		const uint32 *curTable = defaultSceneTable[i-1];
		cur->sceneId = curTable[0];
		if (cur->sceneId == _currentCharacter->sceneId) {
			//++cur->field_20;
			cur->sceneId = curTable[1/*cur->field_20*/];
		}
		//cur->field_23 = curTable[cur->field_20+1];
	}
}

void KyraEngine::setCharacterDefaultFrame(int character) {
	static uint16 initFrameTable[] = {
		7, 41, 77, 0, 0
	};
	assert(character < ARRAYSIZE(initFrameTable));
	Character *edit = &_characterList[character];
	edit->sceneId = 0xFFFF;
	edit->facing = 0;
	edit->currentAnimFrame = initFrameTable[character];
	// edit->unk6 = 1;
}

void KyraEngine::setCharactersPositions(int character) {
	static uint16 initXPosTable[] = {
		0x3200, 0x0024, 0x2230, 0x2F00, 0x0020, 0x002B,
		0x00CA, 0x00F0, 0x0082, 0x00A2, 0x0042
	};
	static uint8 initYPosTable[] = {
		0x00, 0xA2, 0x00, 0x42, 0x00,
		0x67, 0x67, 0x60, 0x5A, 0x71,
		0x76
	};
	assert(character < ARRAYSIZE(initXPosTable));
	Character *edit = &_characterList[character];
	edit->x1 = edit->x2 = initXPosTable[character];
	edit->y1 = edit->y2 = initYPosTable[character];
}

void KyraEngine::setCharactersHeight() {
	static int8 initHeightTable[] = {
		48, 40, 48, 47, 56,
		44, 42, 47, 38, 35,
		40
	};
	for (int i = 0; i < 11; ++i) {
		_characterList[i].height = initHeightTable[i];
	}
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

void KyraEngine::enterNewScene(int sceneId, int facing, int unk1, int unk2, int brandonAlive) {
	debug(9, "KyraEngine::enterNewScene(%d, %d, %d, %d, %d)", sceneId, facing, unk1, unk2, brandonAlive);
	int unkVar1 = 1;
	_screen->hideMouse();
	_handleInput = false;
	_abortWalkFlag = false;
	_abortWalkFlag2 = false;
	if (_currentCharacter->sceneId == 7 && sceneId == 24) {
		_newMusicTheme = 2;
	} else if (_currentCharacter->sceneId == 25 && sceneId == 109) {
		_newMusicTheme = 3;
	} else if (_currentCharacter->sceneId == 120 && sceneId == 37) {
		_newMusicTheme = 4;
	} else if (_currentCharacter->sceneId == 52 && sceneId == 199) {
		_newMusicTheme = 5;
	} else if (_currentCharacter->sceneId == 37 && sceneId == 120) {
		_newMusicTheme = 3;
	} else if (_currentCharacter->sceneId == 109 && sceneId == 25) {
		_newMusicTheme = 2;
	} else if (_currentCharacter->sceneId == 24 && sceneId == 7) {
		_newMusicTheme = 1;
	}
	if (_newMusicTheme != _curMusicTheme) {
		snd_playTheme(_newMusicTheme);
	}
	
	switch (_currentCharacter->sceneId) {
		case 1:
			if (sceneId == 0) {
				moveCharacterToPos(0, 0, _currentCharacter->x1, 84);
				unkVar1 = 0;
			}
		break;
		
		case 3:
			if (sceneId == 2) {
				moveCharacterToPos(0, 6, 155, _currentCharacter->y1);
				unkVar1 = 0;
			}
		break;
		
		case 26:
			if (sceneId == 27) {
				moveCharacterToPos(0, 6, 155, _currentCharacter->y1);
				unkVar1 = 0;
			}
		break;
		
		case 44:
			if (sceneId == 45) {
				moveCharacterToPos(0, 2, 192, _currentCharacter->y1);
				unkVar1 = 0;
			}
		break;
		
		default:
		break;
	}
	
	if (unkVar1 && unk1) {
		int xpos = _currentCharacter->x1;
		int ypos = _currentCharacter->y1;
		switch (facing) {
			case 0:
				ypos = _currentCharacter->y1 - 6;
			break;
			
			case 2:
				xpos = 336;
			break;
			
			case 4:
				ypos = 143;
			break;
			
			case 6:
				xpos = -16;
			break;
			
			default:
			break;
		}
		
		moveCharacterToPos(0, facing, xpos, ypos);
	}
	
	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		_movieObjects[i]->close();
	}
	
	if (!brandonAlive) {
		_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
		_scriptInterpreter->startScript(_scriptClick, 5);
		while (_scriptInterpreter->validScript(_scriptClick)) {
			_scriptInterpreter->runScript(_scriptClick);
		}
	}
	
	memset(_entranceMouseCursorTracks, 0xFFFF, sizeof(uint16)*4);
	_currentCharacter->sceneId = sceneId;
	
	assert(sceneId < _roomTableSize);
	assert(_roomTable[sceneId].nameIndex < _roomFilenameTableSize);

	Room *currentRoom = &_roomTable[sceneId];
	
	if (_currentRoom != 0xFFFF && (_features & GF_TALKIE)) {
		char file[32];
		assert(_currentRoom < _roomTableSize);
		int tableId = _roomTable[_currentRoom].nameIndex;
		assert(tableId < _roomFilenameTableSize);
		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".VRM");
		_res->unloadPakFile(file);
	}
	
	_currentRoom = sceneId;
	
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".DAT");
	_sprites->loadDAT(fileNameBuffer, _sceneExits);
	_sprites->setupSceneAnims();
	_scriptInterpreter->unloadScript(_scriptClickData);
	loadSceneMSC();
	
	if ((_features & GF_TALKIE)) {
		strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
		strcat(fileNameBuffer, ".VRM");
		_res->loadPakFile(fileNameBuffer);
	}
	
	_walkBlockNorth = currentRoom->northExit;
	_walkBlockEast = currentRoom->eastExit;
	_walkBlockSouth = currentRoom->southExit;
	_walkBlockWest = currentRoom->westExit;
	
	if (_walkBlockNorth == 0xFFFF) {
		blockOutRegion(0, 0, 320, (_northExitHeight & 0xFF)+3);
	}
	if (_walkBlockEast == 0xFFFF) {
		blockOutRegion(312, 0, 8, 139);
	}
	if (_walkBlockSouth == 0xFFFF) {
		blockOutRegion(0, 135, 320, 8);
	}
	if (_walkBlockWest == 0xFFFF) {
		blockOutRegion(0, 0, 8, 139);
	}
	
	if (!brandonAlive) {
		updatePlayerItemsForScene();
	}

	startSceneScript(brandonAlive);
	setupSceneItems();
	
	initSceneData(facing, unk2, brandonAlive);
	
	_loopFlag2 = 0;
	_screen->showMouse();
	if (!brandonAlive) {
		seq_poisonDeathNow(0);
	}
	updateMousePointer(true);
	_changedScene = true;
}

void KyraEngine::transcendScenes(int roomIndex, int roomName) {
	debug(9, "KyraEngine::transcendScenes(%d, %d)", roomIndex, roomName);
	assert(roomIndex < _roomTableSize);
	if (_features & GF_TALKIE) {
		char file[32];
		assert(roomIndex < _roomTableSize);
		int tableId = _roomTable[roomIndex].nameIndex;
		assert(tableId < _roomFilenameTableSize);
		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".VRM");
		_res->unloadPakFile(file);
	}
	_roomTable[roomIndex].nameIndex = roomName;
	_unkScreenVar2 = 1;
	_unkScreenVar3 = 1;
	_unkScreenVar1 = 0;
	_brandonPosX = _currentCharacter->x1;
	_brandonPosY = _currentCharacter->y1;
	enterNewScene(roomIndex, _currentCharacter->facing, 0, 0, 0);
	_unkScreenVar1 = 1;
	_unkScreenVar2 = 0;
	_unkScreenVar3 = 0;
}

void KyraEngine::setSceneFile(int roomIndex, int roomName) {
	debug(9, "KyraEngine::setSceneFile(%d, %d)", roomIndex, roomName);
	assert(roomIndex < _roomTableSize);
	_roomTable[roomIndex].nameIndex = roomName;
}

void KyraEngine::moveCharacterToPos(int character, int facing, int xpos, int ypos) {
	debug(9, "KyraEngine::moveCharacterToPos(%d, %d, %d, %d)", character, facing, xpos, ypos);
	Character *ch = &_characterList[character];
	ch->facing = facing;
	_screen->hideMouse();
	xpos = (int16)(xpos & 0xFFFC);
	ypos = (int16)(ypos & 0xFFFE);
	disableTimer(19);
	disableTimer(14);
	disableTimer(18);
	uint32 nextFrame = 0;
	switch (facing) {
		case 0:
			while (ypos < ch->y1) {
				nextFrame = getTimerDelay(5 + character) * _tickLength + _system->getMillis();
				setCharacterPositionWithUpdate(character);
				while (_system->getMillis() < nextFrame) { updateGameTimers(); }
			}
			break;
		
		case 2:	
			while (ch->x1 < xpos) {
				nextFrame = getTimerDelay(5 + character) * _tickLength + _system->getMillis();
				setCharacterPositionWithUpdate(character);
				while (_system->getMillis() < nextFrame) { updateGameTimers(); }
			}
			break;
		
		case 4:
			while (ypos > ch->y1) {
				nextFrame = getTimerDelay(5 + character) * _tickLength + _system->getMillis();
				setCharacterPositionWithUpdate(character);
				while (_system->getMillis() < nextFrame) { updateGameTimers(); }
			}
			break;
		
		case 6:
			while (ch->x1 > xpos) {
				nextFrame = getTimerDelay(5 + character) * _tickLength + _system->getMillis();
				setCharacterPositionWithUpdate(character);
				while (_system->getMillis() < nextFrame) { updateGameTimers(); }
			}
			break;
		
		default:
			break;
	}
	enableTimer(19);
	enableTimer(14);
	enableTimer(18);
	_screen->showMouse();
}

void KyraEngine::setCharacterPositionWithUpdate(int character) {
	debug(9, "KyraEngine::setCharacterPositionWithUpdate(%d)", character);
	setCharacterPosition(character, 0);
	_sprites->updateSceneAnims();
	updateGameTimers();
	_animator->updateAllObjectShapes();
	updateTextFade();

	if (_currentCharacter->sceneId == 210) {
		_animator->updateKyragemFading();
	}
}

int KyraEngine::setCharacterPosition(int character, int *facingTable) {
	debug(9, "KyraEngine::setCharacterPosition(%d, 0x%X)", character, facingTable);
	if (character == 0) {
		_currentCharacter->x1 += _charXPosTable[_currentCharacter->facing];
		_currentCharacter->y1 += _charYPosTable[_currentCharacter->facing];
		setCharacterPositionHelper(0, facingTable);
		return 1;
	} else {
		_characterList[character].x1 += _charXPosTable[_characterList[character].facing];
		_characterList[character].y1 += _charYPosTable[_characterList[character].facing];
		if (_characterList[character].sceneId == _currentCharacter->sceneId) {
			setCharacterPositionHelper(character, 0);
		}
	}
	return 0;
}

void KyraEngine::setCharacterPositionHelper(int character, int *facingTable) {
	debug(9, "KyraEngine::setCharacterPositionHelper(%d, 0x%X)", character, facingTable);
	Character *ch = &_characterList[character];
	++ch->currentAnimFrame;
	int facing = ch->facing;
	if (facingTable) {
		if (*facingTable != *(facingTable - 1)) {
			if (*(facingTable - 1) == *(facingTable + 1)) {
				facing = getOppositeFacingDirection(*(facingTable - 1));
				*facingTable = *(facingTable - 1);
			}
		}
	}
	
	static uint8 facingIsZero[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static uint8 facingIsFour[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	
	if (facing == 0) {
		++facingIsZero[character];
	} else {
		bool resetTables = false;
		if (facing != 7) {
			if (facing - 1 != 0) {
				if (facing != 4) {
					if (facing == 3 || facing == 5) {
						if (facingIsFour[character] > 2) {
							facing = 4;
						}
						resetTables = true;
					}
				} else {
					++facingIsFour[character];
				}
			} else {
				if (facingIsZero[character] > 2) {
					facing = 0;
				}
				resetTables = true;
			}
		} else {
			if (facingIsZero[character] > 2) {
				facing = 0;
			}
			resetTables = true;
		}
		
		if (resetTables) {
			facingIsZero[character] = 0;
			facingIsFour[character] = 0;
		}
	}
	
	static const uint16 maxAnimationFrame[] = {
		0x000F, 0x0031, 0x0055, 0x0000, 0x0000, 0x0000,
		0x0008, 0x002A, 0x004E, 0x0000, 0x0000, 0x0000,
		0x0022, 0x0046, 0x006A, 0x0000, 0x0000, 0x0000,
		0x001D, 0x0041, 0x0065, 0x0000, 0x0000, 0x0000,
		0x001F, 0x0043, 0x0067, 0x0000, 0x0000, 0x0000,
		0x0028, 0x004C, 0x0070, 0x0000, 0x0000, 0x0000,
		0x0023, 0x0047, 0x006B, 0x0000, 0x0000, 0x0000
	};
	
	if (facing == 0) {
		if (maxAnimationFrame[36+character] > ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[36+character];
		}
		if (maxAnimationFrame[30+character] < ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[36+character];
		}
	} else if (facing == 4) {
		if (maxAnimationFrame[18+character] > ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[18+character];
		}
		if (maxAnimationFrame[12+character] < ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[18+character];
		}
	} else {
		if (maxAnimationFrame[18+character] < ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[30+character];
		}
		if (maxAnimationFrame[character] == ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[6+character];
		}
		if (maxAnimationFrame[character] < ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[6+character]+2;
		}
	}
	
	if (character == 0) {
		if (_brandonStatusBit & 0x10)
			ch->currentAnimFrame = 88;
	}
	
	animRefreshNPC(character);
}

int KyraEngine::getOppositeFacingDirection(int dir) {
	debug(9, "KyraEngine::getOppositeFacingDirection(%d)", dir);
	switch (dir) {
		case 0:
			return 2;
			break;
			
		case 1:
			return 1;
			break;
			
		case 3:
			return 7;
			break;
			
		case 4:
			return 6;
			break;
			
		case 5:
			return 5;
			break;
			
		case 6:
			return 4;
			break;
			
		case 7:
			return 3;
			break;
			
		default:
			break;
	}
	return 0;
}

void KyraEngine::loadSceneMSC() {
	assert(_currentCharacter->sceneId < _roomTableSize);
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".MSC");
	_screen->fillRect(0, 0, 319, 199, 0, 5);
	loadBitmap(fileNameBuffer, 3, 5, 0);
}

// maybe move these two functions to Screen
void KyraEngine::blockInRegion(int x, int y, int width, int height) {
	debug(9, "KyraEngine::blockInRegion(%d, %d, %d, %d)", x, y, width, height);
	assert(_screen->_shapePages[0]);
	byte *toPtr = _screen->_shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2) {
			*toPtr++ &= 0x7F;
		}
		toPtr = (backUpTo + 320);
	}
}

void KyraEngine::blockOutRegion(int x, int y, int width, int height) {
	debug(9, "KyraEngine::blockOutRegion(%d, %d, %d, %d)", x, y, width, height);
	assert(_screen->_shapePages[0]);
	byte *toPtr = _screen->_shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2) {
			*toPtr++ |= 0x80;
		}
		toPtr = (backUpTo + 320);
	}
}

void KyraEngine::startSceneScript(int brandonAlive) {
	debug(9, "KyraEngine::startSceneScript(%d)", brandonAlive);
	assert(_currentCharacter->sceneId < _roomTableSize);
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".CPS");
	loadBitmap(fileNameBuffer, 3, 3, 0);
	_sprites->loadSceneShapes();
	_exitListPtr = 0;

	_screen->setScreenPalette(_screen->_currentPalette);
	
	_scaleMode = 1;	
	for (int i = 0; i < 145; ++i) {
		_scaleTable[i] = 256;
	}
	
	clearNoDropRects();
	_scriptInterpreter->initScript(_scriptClick, _scriptClickData);
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".EMC");
	_scriptInterpreter->unloadScript(_scriptClickData);
	_scriptInterpreter->loadScript(fileNameBuffer, _scriptClickData, _opcodeTable, _opcodeTableSize, 0);
	_scriptInterpreter->startScript(_scriptClick, 0);
	_scriptClick->variables[0] = _currentCharacter->sceneId;
	_scriptClick->variables[7] = brandonAlive;
	
	while (_scriptInterpreter->validScript(_scriptClick)) {
		_scriptInterpreter->runScript(_scriptClick);
	}
}

void KyraEngine::initSceneData(int facing, int unk1, int brandonAlive) {
	debug(9, "KyraEngine::initSceneData(%d, %d, %d)", facing, unk1, brandonAlive);
	
	int16 xpos2 = 0;
	int setFacing = 1;
	
	int16 xpos = 0, ypos = 0;
	
	if (_brandonPosX == -1 && _brandonPosY == -1) {
		switch (facing+1) {
			case 0:
				xpos = ypos = -1;
			break;
			
			case 1: case 2: case 8:
				xpos = _sceneExits.southXPos;
				ypos = _sceneExits.southYPos;
				break;
			
			case 3:
				xpos = _sceneExits.westXPos;
				ypos = _sceneExits.westYPos;
				break;
			
			case 4: case 5: case 6:
				xpos = _sceneExits.northXPos;
				ypos = _sceneExits.northYPos;
				break;
			
			case 7:
				xpos = _sceneExits.eastXPos;
				ypos = _sceneExits.eastYPos;
				break;
			
			default:
			break;
		}
		
		if ((uint8)(_northExitHeight & 0xFF) + 2 >= ypos) {
			ypos = (_northExitHeight & 0xFF) + 4;
		}
		if (xpos >= 308) {
			xpos = 304;
		}
		if ((uint8)(_northExitHeight >> 8) - 2 <= ypos) {
			ypos = (_northExitHeight >> 8) - 4;
		}
		if (xpos <= 12) {
			xpos = 16;
		}
	}
	
	if (_brandonPosX > -1) {
		xpos = _brandonPosX;
	}
	if (_brandonPosY > -1) {
		ypos = _brandonPosY;
	}
	
	int16 ypos2 = 0;
	if (_brandonPosX > -1 && _brandonPosY > -1) {
		switch (_currentCharacter->sceneId) {
			case 1:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 4;
				xpos2 = 192;
				ypos2 = 104;
				setFacing = 0;
				unk1 = 1;
				break;
				
			case 3:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 2;
				xpos2 = 204;
				ypos2 = 94;
				setFacing = 0;
				unk1 = 1;
				break;
				
			case 26:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 2;
				xpos2 = 192;
				ypos2 = 128;
				setFacing = 0;
				unk1 = 1;
				break;
				
			case 44:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 6;
				xpos2 = 156;
				ypos2 = 96;
				setFacing = 0;
				unk1 = 1;
				break;
				
			case 37:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 2;
				xpos2 = 148;
				ypos2 = 114;
				setFacing = 0;
				unk1 = 1;
				break;
				
			default:
				break;
		}
	}
	
	_brandonPosX = _brandonPosY = -1;
	
	if (unk1 && setFacing) {
		ypos2 = ypos;
		xpos2 = xpos;
		switch (facing) {
			case 0:
				ypos = 142;
				break;
				
			case 2:
				xpos = -16;
				break;
				
			case 4:
				ypos = (uint8)(_northExitHeight & 0xFF) - 4;
				break;
				
			case 6:
				xpos = 336;
				break;
				
			default:
				break;
		}
	}
	
	xpos2 = (int16)(xpos2 & 0xFFFC);
	ypos2 = (int16)(ypos2 & 0xFFFE);
	xpos = (int16)(xpos & 0xFFFC);
	ypos = (int16)(ypos & 0xFFFE);
	_currentCharacter->facing = facing;
	_currentCharacter->x1 = xpos;
	_currentCharacter->x2 = xpos;
	_currentCharacter->y1 = ypos;
	_currentCharacter->y2 = ypos;
	
	initSceneObjectList(brandonAlive);
	
	if (unk1 && brandonAlive == 0) {
		moveCharacterToPos(0, facing, xpos2, ypos2);
	}
	
	_scriptClick->variables[4] = _itemInHand;
	_scriptClick->variables[7] = brandonAlive;
	_scriptInterpreter->startScript(_scriptClick, 3);
	while (_scriptInterpreter->validScript(_scriptClick)) {
		_scriptInterpreter->runScript(_scriptClick);
	}
}

void KyraEngine::resetBrandonPosionFlags() {
	_brandonStatusBit = 0;
	for (int i = 0; i < 256; ++i) {
		_brandonPoisonFlagsGFX[i] = i;
	}
}

void KyraEngine::initAnimStateList() {
	AnimObject *animStates = _animator->objects();
	animStates[0].index = 0;
	animStates[0].active = 1;
	animStates[0].flags = 0x800;
	animStates[0].background = _shapes[2];
	animStates[0].rectSize = _screen->getRectSize(4, 48);
	animStates[0].width = 4;
	animStates[0].height = 48;
	animStates[0].width2 = 4;
	animStates[0].height2 = 3;
	
	for (int i = 1; i <= 4; ++i) {
		animStates[i].index = i;
		animStates[i].active = 0;
		animStates[i].flags = 0x800;
		animStates[i].background = _shapes[3];
		animStates[i].rectSize = _screen->getRectSize(4, 64);
		animStates[i].width = 4;
		animStates[i].height = 48;
		animStates[i].width2 = 4;
		animStates[i].height2 = 3;
	}
	
	for (int i = 5; i < 16; ++i) {
		animStates[i].index = i;
		animStates[i].active = 0;
		animStates[i].flags = 0;
	}
	
	for (int i = 16; i < 28; ++i) {
		animStates[i].index = i;
		animStates[i].flags = 0;
		animStates[i].background = _shapes[349+i];
		animStates[i].rectSize = _screen->getRectSize(3, 24);
		animStates[i].width = 3;
		animStates[i].height = 16;
		animStates[i].width2 = 0;
		animStates[i].height2 = 0;
	}
}

void KyraEngine::initSceneObjectList(int brandonAlive) {
	debug(9, "KyraEngine::initSceneObjectList(%d)", brandonAlive);
	for (int i = 0; i < 31; ++i) {
		_animator->actors()[i].active = 0;
	}
	
	int startAnimFrame = 0;
	
	AnimObject *curAnimState = _animator->actors();
	curAnimState->active = 1;
	curAnimState->drawY = _currentCharacter->y1;
	curAnimState->sceneAnimPtr = _shapes[4+_currentCharacter->currentAnimFrame];
	curAnimState->animFrameNumber = _currentCharacter->currentAnimFrame;
	startAnimFrame = _currentCharacter->currentAnimFrame-7;
	int xOffset = _defaultShapeTable[startAnimFrame].xOffset;
	int yOffset = _defaultShapeTable[startAnimFrame].yOffset;
	if (_scaleMode) {
		curAnimState->x1 = _currentCharacter->x1;
		curAnimState->y1 = _currentCharacter->y1;
		
		_brandonScaleX = _scaleTable[_currentCharacter->y1];
		_brandonScaleY = _scaleTable[_currentCharacter->y1];
		
		curAnimState->x1 += (_brandonScaleX * xOffset) >> 8;
		curAnimState->y1 += (_brandonScaleY * yOffset) >> 8;
	} else {
		curAnimState->x1 = _currentCharacter->x1 + xOffset;
		curAnimState->y1 = _currentCharacter->y1 + yOffset;
	}
	curAnimState->x2 = curAnimState->x1;
	curAnimState->y2 = curAnimState->y1;
	curAnimState->refreshFlag = 1;
	curAnimState->bkgdChangeFlag = 1;
	_animator->clearQueue();
	_animator->addObjectToQueue(curAnimState);
	
	int listAdded = 0;
	int addedObjects = 1;
	
	for (int i = 1; i < 5; ++i) {
		Character *ch = &_characterList[i];
		curAnimState = &_animator->actors()[addedObjects];
		if (ch->sceneId != _currentCharacter->sceneId) {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
			++addedObjects;
			continue;
		}
		
		curAnimState->drawY = ch->y1;
		curAnimState->sceneAnimPtr = _shapes[4+ch->currentAnimFrame];
		curAnimState->animFrameNumber = ch->currentAnimFrame;
		startAnimFrame = ch->currentAnimFrame-7;
		xOffset = _defaultShapeTable[startAnimFrame].xOffset;
		yOffset = _defaultShapeTable[startAnimFrame].yOffset;
		if (_scaleMode) {
			curAnimState->x1 = ch->x1;
			curAnimState->y1 = ch->y1;
		
			_brandonScaleX = _scaleTable[ch->y1];
			_brandonScaleY = _scaleTable[ch->y1];
		
			curAnimState->x1 += (_brandonScaleX * xOffset) >> 8;
			curAnimState->y1 += (_brandonScaleY * yOffset) >> 8;
		} else {
			curAnimState->x1 = ch->x1 + xOffset;
			curAnimState->y1 = ch->y1 + yOffset;
		}
		curAnimState->x2 = curAnimState->x1;
		curAnimState->y2 = curAnimState->y1;
		curAnimState->active = 1;
		curAnimState->refreshFlag = 1;
		curAnimState->bkgdChangeFlag = 1;
		
		if (ch->facing >= 1 && ch->facing <= 3) {
			curAnimState->flags |= 1;
		} else if (ch->facing >= 5 && ch->facing <= 7) {
			curAnimState->flags &= 0xFFFFFFFE;
		}
		
		_animator->addObjectToQueue(curAnimState);
		
		++addedObjects;
		++listAdded;
		if (listAdded < 2)
			i = 5;
	}
	
	for (int i = 0; i < 11; ++i) {
		curAnimState = &_animator->sprites()[i];

		if (_sprites->_anims[i].play) {
			curAnimState->active = 1;
			curAnimState->refreshFlag = 1;
			curAnimState->bkgdChangeFlag = 1;
		}
		else {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
		}
		curAnimState->height = _sprites->_anims[i].height;
		curAnimState->height2 = _sprites->_anims[i].height2;
		curAnimState->width = _sprites->_anims[i].width + 1;
		curAnimState->width2 = _sprites->_anims[i].width2;
		curAnimState->drawY = _sprites->_anims[i].drawY;
		curAnimState->x1 = curAnimState->x2 = _sprites->_anims[i].x;
		curAnimState->y1 = curAnimState->y2 = _sprites->_anims[i].y;
		curAnimState->background = _sprites->_anims[i].background;
		curAnimState->sceneAnimPtr = _sprites->_sceneShapes[_sprites->_anims[i].sprite];
		
		if(_sprites->_anims[i].unk2)
			curAnimState->flags = 0x800;
		else
			curAnimState->flags = 0;

		if (_sprites->_anims[i].flipX)
			curAnimState->flags |= 0x1;
		
		_animator->addObjectToQueue(curAnimState);
	}
	
	for (int i = 0; i < 12; ++i) {
		curAnimState = &_animator->items()[i];
		Room *curRoom = &_roomTable[_currentCharacter->sceneId];
		byte curItem = curRoom->itemsTable[i];
		if (curItem != 0xFF) {
			curAnimState->drawY = curRoom->itemsYPos[i];
			curAnimState->sceneAnimPtr = _shapes[220+curItem];
			curAnimState->animFrameNumber = (int16)0xFFFF;
			curAnimState->y1 = curRoom->itemsYPos[i];
			curAnimState->x1 = curRoom->itemsXPos[i];
			
			curAnimState->x1 -= (fetchAnimWidth(curAnimState->sceneAnimPtr, _scaleTable[curAnimState->drawY])) >> 1;
			curAnimState->y1 -= fetchAnimHeight(curAnimState->sceneAnimPtr, _scaleTable[curAnimState->drawY]);
			
			curAnimState->x2 = curAnimState->x1;
			curAnimState->y2 = curAnimState->y1;
			
			curAnimState->active = 1;
			curAnimState->refreshFlag = 1;
			curAnimState->bkgdChangeFlag = 1;
			
			_animator->addObjectToQueue(curAnimState);
		} else {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
		}
	}
	
	_animator->preserveAnyChangedBackgrounds();
	curAnimState = _animator->actors();
	curAnimState->bkgdChangeFlag = 1;
	curAnimState->refreshFlag = 1;
	for (int i = 1; i < 28; ++i) {
		curAnimState = &_animator->objects()[i];
		if (curAnimState->active) {
			curAnimState->bkgdChangeFlag = 1;
			curAnimState->refreshFlag = 1;
		}
	}
	_animator->restoreAllObjectBackgrounds();
	_animator->preserveAnyChangedBackgrounds();
	_animator->prepDrawAllObjects();
	initSceneScreen(brandonAlive);
	_animator->copyChangedObjectsForward(0);
}

void KyraEngine::initSceneScreen(int brandonAlive) {
	// XXX (Pointless?) Palette stuff
	if (_unkScreenVar2 == 1) {
		_screen->shuffleScreen(8, 8, 304, 128, 2, 0, _unkScreenVar3, false);
	} else {
		_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	}
	_screen->updateScreen();
	// XXX More (pointless?) palette stuff

	if (!_scriptInterpreter->startScript(_scriptClick, 2))
		error("Could not start script function 2 of scene script");

	_scriptClick->variables[7] = brandonAlive;

	while (_scriptInterpreter->validScript(_scriptClick))
		_scriptInterpreter->runScript(_scriptClick);

	setTextFadeTimerCountdown(-1);
	if (_currentCharacter->sceneId == 210) {
		if (_itemInHand != -1)
			magicOutMouseItem(2, -1);
		
		_screen->hideMouse();
		for (int i = 0; i < 10; ++i) {
			if (_currentCharacter->inventoryItems[i] != 0xFF)
				magicOutMouseItem(2, i);
		}
		_screen->showMouse();
	}
}

#pragma mark -
#pragma mark - Text handling
#pragma mark -

void KyraEngine::waitForChatToFinish(int16 chatDuration, char *chatStr, uint8 charNum) {
	debug(9, "KyraEngine::waitForChatToFinish(%i, %s, %i)", chatDuration, chatStr, charNum); 
	bool hasUpdatedNPCs = false;
	bool runLoop = true;
	uint8 currPage;
	OSystem::Event event;
	int16 delayTime;

	//while( towns_isEscKeyPressed() )
		//towns_getKey();

	uint32 timeToEnd = strlen(chatStr) * 8 * _tickLength + _system->getMillis();

	if (chatDuration != -1 ) {
		switch (_configTalkspeed) {
			case 0: chatDuration *= 2;
					break;
			case 2: chatDuration /= 4;
					break;
			case 3: chatDuration = -1;
		}
	}

	if (chatDuration != -1)
		chatDuration *= _tickLength;

	disableTimer(14);
	disableTimer(18);
	disableTimer(19);

	uint32 timeAtStart = _system->getMillis();
	uint32 loopStart;
	while (runLoop) {
		loopStart = _system->getMillis();
		if (_currentCharacter->sceneId == 210)
			if (seq_playEnd())
				break;

		if (_system->getMillis() > timeToEnd && !hasUpdatedNPCs) {
			hasUpdatedNPCs = true;
			disableTimer(15);
			_currHeadShape = 4;
			animRefreshNPC(0);
			animRefreshNPC(_talkingCharNum);

			if (_charSayUnk2 != -1) {
				_animator->sprites()[_charSayUnk2].active = 0;
				_sprites->_anims[_charSayUnk2].play = false;
				_charSayUnk2 = -1;
			}
		}

		updateGameTimers();
		_sprites->updateSceneAnims();
		_animator->restoreAllObjectBackgrounds();
		_animator->preserveAnyChangedBackgrounds();
		_animator->prepDrawAllObjects();

		currPage = _screen->_curPage;
		_screen->_curPage = 2;
		_text->printCharacterText(chatStr, charNum, _characterList[charNum].x1);
		_screen->_curPage = currPage;

		_animator->copyChangedObjectsForward(0);
		updateTextFade();

		if ((chatDuration < (int16)(_system->getMillis() - timeAtStart)) && chatDuration != -1)
			break;

		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == '.')
					runLoop = false;
				break;
			case OSystem::EVENT_QUIT:
				quitGame();
			case OSystem::EVENT_LBUTTONDOWN:
				runLoop = false;
				break;
			default:
				break;
			}
		}
		
		if (_fastMode)
			runLoop = false;

		delayTime = (loopStart + _gameSpeed) - _system->getMillis();
		if (delayTime > 0)
			_system->delayMillis(delayTime);
	}

	enableTimer(14);
	enableTimer(15);
	enableTimer(18);
	enableTimer(19);
	//clearKyrandiaButtonIO();
}

void KyraEngine::endCharacterChat(int8 charNum, int16 convoInitialized) {
	_charSayUnk3 = -1;

	if (charNum > 4 && charNum < 11) {
		//TODO: weird _game_inventory stuff here
		warning("STUB: endCharacterChat() for high charnums");
	}

	if (convoInitialized != 0) {
		_talkingCharNum = -1;
		_currentCharacter->currentAnimFrame = 7;
		animRefreshNPC(0);
		_animator->updateAllObjectShapes();
	}
}

void KyraEngine::restoreChatPartnerAnimFrame(int8 charNum) {
	_talkingCharNum = -1;

	if (charNum > 0 && charNum < 5) {
		_characterList[charNum].currentAnimFrame = _currentChatPartnerBackupFrame;
		animRefreshNPC(charNum);
	}

	_currentCharacter->currentAnimFrame = 7;
	animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

void KyraEngine::backupChatPartnerAnimFrame(int8 charNum) {
	_talkingCharNum = 0;

	if (charNum < 5 && charNum > 0) 
		_currentChatPartnerBackupFrame = _characterList[charNum].currentAnimFrame;

	if (_scaleMode != 0)
		_currentCharacter->currentAnimFrame = 7;
	else
		_currentCharacter->currentAnimFrame = _currentCharAnimFrame;

	animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

int8 KyraEngine::getChatPartnerNum() {
	uint8 sceneTable[] = {0x2, 0x5, 0x2D, 0x7, 0x1B, 0x8, 0x22, 0x9, 0x30, 0x0A};
	int pos = 0;
	int partner = -1;

	for (int i = 1; i < 6; i++) {
		if (_currentCharacter->sceneId == sceneTable[pos]) {
			partner = sceneTable[pos+1];
			break;
		}
		pos += 2;
	}

	for (int i = 1; i < 5; i++) {
		if (_characterList[i].sceneId == _currentCharacter->sceneId) {
			partner = i;
			break;
		}
	}
	return partner;
}

int KyraEngine::initCharacterChat(int8 charNum) {
	if (_talkingCharNum == -1) {
		_talkingCharNum = 0;

		if (_scaleMode != 0)
			_currentCharacter->currentAnimFrame = 7;
		else
			_currentCharacter->currentAnimFrame = 16;

		animRefreshNPC(0);
		_animator->updateAllObjectShapes();
	}

	_charSayUnk2 = -1;
	_animator->flagAllObjectsForBkgdChange();
	_animator->restoreAllObjectBackgrounds();

	if (charNum > 4 && charNum < 11) {
		// TODO: Fill in weird _game_inventory stuff here
		warning("STUB: initCharacterChat() for high charnums");
	}

	_animator->flagAllObjectsForRefresh();
	_animator->flagAllObjectsForBkgdChange();
	_animator->preserveAnyChangedBackgrounds();
	_charSayUnk3 = charNum;

	return 1;
}

void KyraEngine::characterSays(char *chatStr, int8 charNum, int8 chatDuration) {
	debug(9, "KyraEngine::characterSays('%s', %i, %d)", chatStr, charNum, chatDuration);
	uint8 startAnimFrames[] =  { 0x10, 0x32, 0x56, 0x0, 0x0, 0x0 };

	uint16 chatTicks;
	int16 convoInitialized;
	int8 chatPartnerNum;

	if (_currentCharacter->sceneId == 210)
		return;

	convoInitialized = initCharacterChat(charNum);	
	chatPartnerNum = getChatPartnerNum();

	if (chatPartnerNum != -1 && chatPartnerNum < 5)
		backupChatPartnerAnimFrame(chatPartnerNum);

	if (charNum < 5) {
		_characterList[charNum].currentAnimFrame = startAnimFrames[charNum];
		_charSayUnk3 = charNum;
		_talkingCharNum = charNum;
		animRefreshNPC(charNum);
	}

	char *processedString = _text->preprocessString(chatStr);
	int lineNum = _text->buildMessageSubstrings(processedString);

	int16 yPos = _characterList[charNum].y1;
	yPos -= _scaleTable[charNum] * _characterList[charNum].height;
	yPos -= 8;
	yPos -= lineNum * 10;

	if (yPos < 11)
		yPos = 11;

	if (yPos > 100)
		yPos = 100;

	_text->_talkMessageY = yPos;
	_text->_talkMessageH = lineNum * 10;
	_animator->restoreAllObjectBackgrounds();

	_screen->copyRegion(1, _text->_talkMessageY, 1, 136, 319, _text->_talkMessageH, 2, 2);
	_screen->hideMouse();

	_text->printCharacterText(processedString, charNum, _characterList[charNum].x1);
	_screen->showMouse();

	if (chatDuration == -2)
		chatTicks = strlen(processedString) * 9;
	else
		chatTicks = chatDuration;

	waitForChatToFinish(chatTicks, chatStr, charNum);

	_animator->restoreAllObjectBackgrounds();

	_screen->copyRegion(1, 136, 1, _text->_talkMessageY, 319, _text->_talkMessageH, 2, 2);
	_animator->preserveAllBackgrounds();
	_animator->prepDrawAllObjects();
	_screen->hideMouse();

	_screen->copyRegion(1, _text->_talkMessageY, 1, _text->_talkMessageY, 319, _text->_talkMessageH, 2, 0);
	_screen->showMouse();
	_animator->flagAllObjectsForRefresh();
	_animator->copyChangedObjectsForward(0);

	if (chatPartnerNum != -1 && chatPartnerNum < 5)
		restoreChatPartnerAnimFrame(chatPartnerNum);

	endCharacterChat(charNum, convoInitialized);
}

void KyraEngine::drawSentenceCommand(char *sentence, int color) {
	debug(9, "KyraEngine::drawSentenceCommand('%s', %i)", sentence, color);
	_screen->hideMouse();
	_screen->fillRect(8, 143, 311, 152, 12);

	if (_startSentencePalIndex != color || _fadeText != false) {
		_currSentenceColor[0] = _screen->_currentPalette[765] = _screen->_currentPalette[color*3];
		_currSentenceColor[1] = _screen->_currentPalette[766] = _screen->_currentPalette[color*3+1];
		_currSentenceColor[2] = _screen->_currentPalette[767] = _screen->_currentPalette[color*3+2];
	
		_screen->setScreenPalette(_screen->_currentPalette);
		_startSentencePalIndex = 0;
	}

	_text->printText(sentence, 8, 143, 0xFF, 12, 0);
	_screen->showMouse();
	setTextFadeTimerCountdown(15);
	_fadeText = false;
}

void KyraEngine::updateSentenceCommand(char *str1, char *str2, int color) {
	debug(9, "KyraEngine::updateSentenceCommand('%s', '%s', %i)", str1, str2, color);
	char sentenceCommand[500];
	strncpy(sentenceCommand, str1, 500);
	if (str2)
		strncat(sentenceCommand, str2, 500 - strlen(sentenceCommand));

	drawSentenceCommand(sentenceCommand, color);
	_screen->updateScreen();
}

void KyraEngine::updateTextFade() {
	debug(9, "KyraEngine::updateTextFade()");
	if (!_fadeText)
		return;
	
	bool finished = false;
	for (int i = 0; i < 3; i++)
		if (_currSentenceColor[i] > 4)
			_currSentenceColor[i] -= 4;
		else
			if (_currSentenceColor[i]) {
				_currSentenceColor[i] = 0;
				finished = true;
			}
		
	_screen->_currentPalette[765] = _currSentenceColor[0];
	_screen->_currentPalette[766] = _currSentenceColor[1];
	_screen->_currentPalette[767] = _currSentenceColor[2];
	_screen->setScreenPalette(_screen->_currentPalette);

	if (finished) {
		_fadeText = false;
		_startSentencePalIndex = -1;
	}
}

#pragma mark -
#pragma mark - Item handling
#pragma mark -

void KyraEngine::addToNoDropRects(int x, int y, int w, int h) {
	debug(9, "KyraEngine::addToNoDropRects(%d, %d, %d, %d)", x, y, w, h);
	for (int rect = 0; rect < 11; ++rect) {
		if (_noDropRects[rect].x == -1) {
			_noDropRects[rect].x = x;
			_noDropRects[rect].y = y;
			_noDropRects[rect].x2 = x + w - 1;
			_noDropRects[rect].y2 = y + h - 1;
			break;
		}
	}
}

void KyraEngine::clearNoDropRects() {
	debug(9, "KyraEngine::clearNoDropRects()");
	memset(_noDropRects, -1, sizeof(_noDropRects));
}

byte KyraEngine::findFreeItemInScene(int scene) {
	debug(9, "KyraEngine::findFreeItemInScene(%d)", scene);
	assert(scene < _roomTableSize);
	Room *room = &_roomTable[scene];
	for (int i = 0; i < 12; ++i) {
		if (room->itemsTable[i] == 0xFF)
			return i;
	}
	return 0xFF;
}

byte KyraEngine::findItemAtPos(int x, int y) {
	debug(9, "KyraEngine::findItemAtPos(%d, %d)", x, y);
	assert(_currentCharacter->sceneId < _roomTableSize);
	const uint8 *itemsTable = _roomTable[_currentCharacter->sceneId].itemsTable;
	const uint16 *xposOffset = _roomTable[_currentCharacter->sceneId].itemsXPos;
	const uint8 *yposOffset = _roomTable[_currentCharacter->sceneId].itemsYPos;
	
	int highestYPos = -1;
	byte returnValue = 0xFF;
	
	for (int i = 0; i < 12; ++i) {
		if (*itemsTable != 0xFF) {
			int xpos = *xposOffset - 11;
			int xpos2 = *xposOffset + 10;
			if (x > xpos && x < xpos2) {
				assert(*itemsTable < ARRAYSIZE(_itemTable));
				int itemHeight = _itemTable[*itemsTable].height;
				int ypos = *yposOffset + 3;
				int ypos2 = ypos - itemHeight - 3;
				
				if (y > ypos2 && ypos > y) {
					if (highestYPos <= ypos) {
						returnValue = i;
						highestYPos = ypos;
					}
				}
			}
		}
		++xposOffset;
		++yposOffset;
		++itemsTable;
	}
	
	return returnValue;
}

void KyraEngine::placeItemInGenericMapScene(int item, int index) {
	debug(9, "KyraEngine::placeItemInGenericMapScene(%d, %d)", item, index);
	static const uint16 itemMapSceneMinTable[] = {
		0x0000, 0x0011, 0x006D, 0x0025, 0x00C7, 0x0000
	};
	static const uint16 itemMapSceneMaxTable[] = {
		0x0010, 0x0024, 0x00C6, 0x006C, 0x00F5, 0x0000
	};
	
	int minValue = itemMapSceneMinTable[index];
	int maxValue = itemMapSceneMaxTable[index];
	
	while (true) {
		int room = _rnd.getRandomNumberRng(minValue, maxValue);
		assert(room < _roomTableSize);
		int nameIndex = _roomTable[room].nameIndex;
		bool placeItem = false;
		
		switch (nameIndex) {
			case 0:  case 1:   case 2:   case 3:
			case 4:  case 5:   case 6:   case 11:
			case 12: case 16:  case 17:  case 20:
			case 22: case 23:  case 25:  case 26:
			case 27: case 31:  case 33:  case 34:
			case 36: case 37:  case 58:  case 59:
			case 60: case 61:  case 83:  case 84:
			case 85: case 104: case 105: case 106:
				placeItem = true;
			break;
			
			case 51:
				if (room != 46) {
					placeItem = true;
					break;
				}		
			default:
				placeItem = false;
			break;
		}
		
		if (placeItem) {
			Room *roomPtr = &_roomTable[room];
			if (roomPtr->northExit == 0xFFFF && roomPtr->eastExit == 0xFFFF && roomPtr->southExit == 0xFFFF && roomPtr->westExit == 0xFFFF) {
				placeItem = false;
			} else if (_currentCharacter->sceneId == room) {
				placeItem = false;
			}
		}
		
		if (placeItem) {
			if (!processItemDrop(room, item, -1, -1, 2, 0))
				continue;
			break;
		}
	}
}

void KyraEngine::createMouseItem(int item) {
	debug(9, "KyraEngine::createMouseItem(%d)", item);
	_screen->hideMouse();
	setMouseItem(item);
	_itemInHand = item;
	_screen->showMouse();
}

void KyraEngine::destroyMouseItem() {
	debug(9, "KyraEngine::destroyMouseItem()");
	_screen->hideMouse();
	_screen->setMouseCursor(1, 1, _shapes[4]);
	_itemInHand = -1;
	_screen->showMouse();
}

void KyraEngine::setMouseItem(int item) {
	debug(9, "KyraEngine::setMouseItem(%d)", item);
	if (item == -1) {
		_screen->setMouseCursor(1, 1, _shapes[10]);
	} else {
		_screen->setMouseCursor(8, 15, _shapes[220+item]);
	}
}

void KyraEngine::wipeDownMouseItem(int xpos, int ypos) {
	debug(9, "KyraEngine::wipeDownMouseItem(%d, %d)", xpos, ypos);
	if (_itemInHand == -1)
		return;
	xpos -= 8;
	ypos -= 15;
	_screen->hideMouse();
	backUpRect1(xpos, ypos);
	int y = ypos;
	int height = 16;
	
	while (height >= 0) {
		restoreRect1(xpos, ypos);
		_screen->setNewShapeHeight(_shapes[220+_itemInHand], height);
		uint32 nextTime = _system->getMillis() + 1 * _tickLength;
		_screen->drawShape(0, _shapes[220+_itemInHand], xpos, y, 0, 0);
		_screen->updateScreen();
		y += 2;
		height -= 2;
		while (_system->getMillis() < nextTime) {}
	}	
	restoreRect1(xpos, ypos);
	_screen->resetShapeHeight(_shapes[220+_itemInHand]);
	destroyMouseItem();
	_screen->showMouse();
}

void KyraEngine::setupSceneItems() {
	debug(9, "KyraEngine::setupSceneItems()");
	uint16 sceneId = _currentCharacter->sceneId;
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	for (int i = 0; i < 12; ++i) {
		uint8 item = currentRoom->itemsTable[i];
		if (item == 0xFF || !currentRoom->needInit[i]) {
			continue;
		}
		
		int xpos = 0;
		int ypos = 0;
		
		if (currentRoom->itemsXPos[i] == 0xFFFF) {
			xpos = currentRoom->itemsXPos[i] = _rnd.getRandomNumberRng(24, 296);
			ypos = currentRoom->itemsYPos[i] = _rnd.getRandomNumberRng(_northExitHeight & 0xFF, 130);
		} else {
			xpos = currentRoom->itemsXPos[i];
			ypos = currentRoom->itemsYPos[i];
		}
		
		_lastProcessedItem = i;
		
		int stop = 0;
		while (!stop) {
			stop = processItemDrop(sceneId, item, xpos, ypos, 3, 0);
			if (!stop) {
				xpos = currentRoom->itemsXPos[i] = _rnd.getRandomNumberRng(24, 296);
				ypos = currentRoom->itemsYPos[i] = _rnd.getRandomNumberRng(_northExitHeight & 0xFF, 130);
				if (countItemsInScene(sceneId) >= 12) {
					break;
				}
			} else {
				currentRoom->needInit[i] = 0;
			}
		}
	}
}

int KyraEngine::countItemsInScene(uint16 sceneId) {
	debug(9, "KyraEngine::countItemsInScene(%d)", sceneId);
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	
	int items = 0;
	
	for (int i = 0; i < 12; ++i) {
		if (currentRoom->itemsTable[i] != 0xFF) {
			++items;
		}
	}
	
	return items;
}

int KyraEngine::processItemDrop(uint16 sceneId, uint8 item, int x, int y, int unk1, int unk2) {
	debug(9, "KyraEngine::processItemDrop(%d, %d, %d, %d, %d, %d)", sceneId, item, x, y, unk1, unk2);
	int freeItem = -1;
	uint8 itemIndex = findItemAtPos(x, y);
	if (unk1) {
		itemIndex = 0xFF;
	}
	
	if (itemIndex != 0xFF) {
		exchangeItemWithMouseItem(sceneId, itemIndex);
		return 0;
	}
	
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	
	if (unk1 != 3) {
		for (int i = 0; i < 12; ++i) {
			if (currentRoom->itemsTable[i] == 0xFF) {
				freeItem = i;
				break;
			}
		}
	} else {
		freeItem = _lastProcessedItem;
	}

	if (freeItem == -1) {
		return 0;
	}
	
	if (sceneId != _currentCharacter->sceneId) {
		addItemToRoom(sceneId, item, freeItem, x, y);
		return 1;
	}
	
	int itemHeight = _itemTable[item].height;
	_lastProcessedItemHeight = itemHeight;
	
	if (x == -1 && x == -1) {
		x = _rnd.getRandomNumberRng(16, 304);
		y = _rnd.getRandomNumberRng(_northExitHeight & 0xFF, 135);
	}
	
	int xpos = x;
	int ypos = y;
	int destY = -1;
	int destX = -1;
	int running = 1;

	while (running) {
		if ((_northExitHeight & 0xFF) <= ypos) {
			bool running2 = true;
			
			if (getDrawLayer(xpos, ypos) > 1) {
				if (((_northExitHeight >> 8) & 0xFF) != ypos) {
					running2 = false;
				}
			}
			
			if (getDrawLayer2(xpos, ypos, itemHeight) > 1) {
				if (((_northExitHeight >> 8) & 0xFF) != ypos) {
					running2 = false;
				}
			}
			
			if (!isDropable(xpos, ypos)) {
				if (((_northExitHeight >> 8) & 0xFF) != ypos) {
					running2 = false;
				}
			}
			
			int xpos2 = xpos;
			int xpos3 = xpos;
			
			while (running2) {
				if (isDropable(xpos2, ypos)) {
					if (getDrawLayer2(xpos2, ypos, itemHeight) < 7) {
						if (findItemAtPos(xpos2, ypos) == 0xFF) {
							destX = xpos2;
							destY = ypos;
							running = 0;
							running2 = false;
						}
					}
				}
				
				if (isDropable(xpos3, ypos)) {
					if (getDrawLayer2(xpos3, ypos, itemHeight) < 7) {
						if (findItemAtPos(xpos3, ypos) == 0xFF) {
							destX = xpos3;
							destY = ypos;
							running = 0;
							running2 = false;
						}
					}
				}
				
				if (!running2)
					continue;
				
				xpos2 -= 2;
				if (xpos2 < 16) {
					xpos2 = 16;
				}
				
				xpos3 += 2;
				if (xpos3 > 304) {
					xpos3 = 304;
				}
				
				if (xpos2 > 16)
					continue;
				if (xpos3 < 304)
					continue;
				running2 = false;
			}
		}
		
		if (((_northExitHeight >> 8) & 0xFF) == ypos) {
			running = 0;
			destY -= _rnd.getRandomNumberRng(0, 3);
			
			if ((_northExitHeight & 0xFF) < destY) {
				continue;
			}
			
			destY = (_northExitHeight & 0xFF) + 1;
			continue;
		}		
		ypos += 2;
		if (((_northExitHeight >> 8) & 0xFF) >= ypos) {
			continue;
		}
		ypos = (_northExitHeight >> 8) & 0xFF;
	}
	
	if (destX == -1 || destY == -1) {
		return 0;
	}
	
	if (unk1 == 3) {
		currentRoom->itemsXPos[freeItem] = destX;
		currentRoom->itemsYPos[freeItem] = destY;
		return 1;
	}
	
	if (unk1 == 2) {
		itemSpecialFX(x, y, item);
	}
	
	if (unk1 == 0) {
		destroyMouseItem();
	}
	
	itemDropDown(x, y, destX, destY, freeItem, item);
	
	if (unk1 == 0 && unk2 != 0) {
		assert(_itemList && _droppedList);
		updateSentenceCommand(_itemList[item], _droppedList[0], 179);
	}
	
	return 1;
}

void KyraEngine::exchangeItemWithMouseItem(uint16 sceneId, int itemIndex) {
	debug(9, "KyraEngine::exchangeItemWithMouseItem(%d, %d)", sceneId, itemIndex);
	_screen->hideMouse();
	_animator->animRemoveGameItem(itemIndex);
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	
	int item = currentRoom->itemsTable[itemIndex];
	currentRoom->itemsTable[itemIndex] = _itemInHand;
	_itemInHand = item;
	_animator->animAddGameItem(itemIndex, sceneId);
	// XXX snd_kyraPlaySound 53
	
	setMouseItem(_itemInHand);
	assert(_itemList && _takenList);
	updateSentenceCommand(_itemList[_itemInHand], _takenList[1], 179);
	_screen->showMouse();
	clickEventHandler2();
}

void KyraEngine::addItemToRoom(uint16 sceneId, uint8 item, int itemIndex, int x, int y) {
	debug(9, "KyraEngine::addItemToRoom(%d, %d, %d, %d, %d)", sceneId, item, itemIndex, x, y);
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	currentRoom->itemsTable[itemIndex] = item;
	currentRoom->itemsXPos[itemIndex] = x;
	currentRoom->itemsYPos[itemIndex] = y;
	currentRoom->needInit[itemIndex] = 1;
}

int KyraEngine::checkNoDropRects(int x, int y) {
	debug(9, "KyraEngine::checkNoDropRects(%d, %d)", x, y);
	if (_lastProcessedItemHeight < 1 || _lastProcessedItemHeight > 16) {
		_lastProcessedItemHeight = 16;
	}
	if (_noDropRects[0].x == -1) {
		return 0;
	}
	
	for (int i = 0; i < 11; ++i) {
		if (_noDropRects[i].x == -1) {
			break;
		}
		
		int xpos = _noDropRects[i].x;
		int ypos = _noDropRects[i].y;
		int xpos2 = _noDropRects[i].x2;
		int ypos2 = _noDropRects[i].y2;
		
		if (xpos > x + 16)
			continue;
		if (xpos2 < x)
			continue;
		if (y < ypos)
			continue;
		if (ypos2 < y - _lastProcessedItemHeight)
			continue;
		return 1;
	}
	
	return 0;
}

int KyraEngine::isDropable(int x, int y) {
	debug(9, "KyraEngine::isDropable(%d, %d)", x, y);
	x -= 8;
	y -= 1;
	
	if (checkNoDropRects(x, y)) {
		return 0;
	}
	
	for (int xpos = x; xpos < x + 16; ++xpos) {
		if (_screen->getShapeFlag1(xpos, y) == 0) {
			return 0;
		}
	}	
	return 1;
}

void KyraEngine::itemDropDown(int x, int y, int destX, int destY, byte freeItem, int item) {
	debug(9, "KyraEngine::itemDropDown(%d, %d, %d, %d, %d, %d)", x, y, destX, destY, freeItem, item);
	assert(_currentCharacter->sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[_currentCharacter->sceneId];
	if (x == destX && y == destY) {
		currentRoom->itemsXPos[freeItem] = destX;
		currentRoom->itemsYPos[freeItem] = destY;
		currentRoom->itemsTable[freeItem] = item;
		// call kyraPlaySound(0x32)
		_animator->animAddGameItem(freeItem, _currentCharacter->sceneId);
		return;
	}
	_screen->hideMouse();
	if (y <= destY) {
		int tempY = y;
		int addY = 2;
		int drawX = x - 8;
		int drawY = 0;
		
		backUpRect0(drawX, y - 16);
		
		while (tempY < destY) {
			restoreRect0(drawX, tempY - 16);
			tempY += addY;
			if (tempY > destY) {
				tempY = destY;
			}
			++addY;
			drawY = tempY - 16;
			backUpRect0(drawX, drawY);
			uint32 nextTime = _system->getMillis() + 1 * _tickLength;
			_screen->drawShape(0, _shapes[220+item], drawX, drawY, 0, 0);
			_screen->updateScreen();
			while (_system->getMillis() < nextTime) {
				if ((nextTime - _system->getMillis()) >= 10)
					delay(10);
			}
		}
		
		bool skip = false;
		if (x == destX) {
			if (destY - y <= 16) {
				skip = true;
			}
		}
		
		if (!skip) {
			// call kyraPlaySound(0x47)
			if (addY < 6)
				addY = 6;
			
			int xDiff = (destX - x) << 4;
			xDiff /= addY;
			int startAddY = addY;
			addY >>= 1;
			if (destY - y <= 8) {
				addY >>= 1;
			}
			addY = -addY;
			int unkX = x << 4;
			while (--startAddY) {
				drawX = (unkX >> 4) - 8;
				drawY = tempY - 16;
				restoreRect0(drawX, drawY);
				tempY += addY;
				unkX += xDiff;
				if (tempY > destY) {
					tempY = destY;
				}
				++addY;
				drawX = (unkX >> 4) - 8;
				drawY = tempY - 16;
				backUpRect0(drawX, drawY);
				uint32 nextTime = _system->getMillis() + 1 * _tickLength;
				_screen->drawShape(0, _shapes[220+item], drawX, drawY, 0, 0);
				_screen->updateScreen();
				while (_system->getMillis() < nextTime) {
					if ((nextTime - _system->getMillis()) >= 10)
						delay(10);
				}
			}
			restoreRect0(drawX, drawY);
		} else {
			restoreRect0(drawX, tempY - 16);
		}
	}
	currentRoom->itemsXPos[freeItem] = destX;
	currentRoom->itemsYPos[freeItem] = destY;
	currentRoom->itemsTable[freeItem] = item;
	// call kyraPlaySound(0x32)
	_animator->animAddGameItem(freeItem, _currentCharacter->sceneId);
	_screen->showMouse();
}

void KyraEngine::dropItem(int unk1, int item, int x, int y, int unk2) {
	debug(9, "KyraEngine::dropItem(%d, %d, %d, %d, %d)", unk1, item, x, y, unk2);
	if (processItemDrop(_currentCharacter->sceneId, item, x, y, unk1, unk2))
		return;
	// call kyraPlaySound(54)
	if (12 == countItemsInScene(_currentCharacter->sceneId)) {
		assert(_noDropList);
		drawSentenceCommand(_noDropList[0], 6);
	} else {
		assert(_noDropList);
		drawSentenceCommand(_noDropList[1], 6);
	}
}

void KyraEngine::itemSpecialFX(int x, int y, int item) {
	debug(9, "KyraEngine::itemSpecialFX(%d, %d, %d)", x, y, item);
	if (item == 41) {
		itemSpecialFX1(x, y, item);
	} else {
		itemSpecialFX2(x, y, item);
	}
}

void KyraEngine::itemSpecialFX1(int x, int y, int item) {
	debug(9, "KyraEngine::itemSpecialFX1(%d, %d, %d)", x, y, item);
	uint8 *shape = _shapes[220+item];
	x -= 8;
	int startY = y;
	y -= 15;
	_screen->hideMouse();
	backUpRect0(x, y);
	for (int i = 1; i <= 16; ++i) {
		_screen->setNewShapeHeight(shape, i);
		--startY;
		restoreRect0(x, y);
		uint32 nextTime = _system->getMillis() + 1 * _tickLength;
		_screen->drawShape(0, shape, x, startY, 0, 0);
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if ((nextTime - _system->getMillis()) >= 10)
				delay(10);
		}
	}
	restoreRect0(x, y);
	_screen->showMouse();
}

void KyraEngine::itemSpecialFX2(int x, int y, int item) {
	debug(9, "KyraEngine::itemSpecialFX2(%d, %d, %d)", x, y, item);
	x -= 8;
	y -= 15;
	int yAdd = (int8)(((16 - _itemTable[item].height) >> 1) & 0xFF);
	backUpRect0(x, y);
	if (item >= 80 && item <= 89) {
		// snd_kyraPlaySound(55);
	}
	
	for (int i = 201; i <= 205; ++i) {
		restoreRect0(x, y);
		uint32 nextTime = _system->getMillis() + 3 * _tickLength;
		_screen->drawShape(0, _shapes[4+i], x, y + yAdd, 0, 0);
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if ((nextTime - _system->getMillis()) >= 10)
				delay(10);
		}
	}
	
	for (int i = 204; i >= 201; --i) {
		restoreRect0(x, y);
		uint32 nextTime = _system->getMillis() + 3 * _tickLength;
		_screen->drawShape(0, _shapes[220+item], x, y, 0, 0);
		_screen->drawShape(0, _shapes[4+i], x, y + yAdd, 0, 0);
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if ((nextTime - _system->getMillis()) >= 10)
				delay(10);
		}
	}
	restoreRect0(x, y);
}

void KyraEngine::magicOutMouseItem(int animIndex, int itemPos) {
	debug(9, "KyraEngine::magicOutMouseItem(%d, %d)", animIndex, itemPos);
	int videoPageBackUp = _screen->_curPage;
	_screen->_curPage = 0;
	int x = 0, y = 0;
	if (itemPos == -1) {
		x = _mouseX - 12;
		y = _mouseY - 18;
	} else {
		x = _itemPosX[itemPos] - 4;
		y = _itemPosY[itemPos] - 3;
	}
	
	if (_itemInHand == -1 && itemPos == -1) {
		return;
	}
	
	int tableIndex = 0, loopStart = 0, maxLoops = 0;
	if (animIndex == 0) {
		tableIndex = _rnd.getRandomNumberRng(0, 5);
		loopStart = 35;
		maxLoops = 9;
	} else if (animIndex == 1) {
		tableIndex = _rnd.getRandomNumberRng(0, 11);
		loopStart = 115;
		maxLoops = 8;
	} else if (animIndex == 2) {
		tableIndex = 0;
		loopStart = 124;
		maxLoops = 4;
	} else {
		tableIndex = -1;
	}
	
	if (animIndex == 2) {
		// snd_kyraPlaySound(0x5E);
	} else {
		// snd_kyraPlaySound(0x37);
	}
	_screen->hideMouse();
	backUpRect1(x, y);

	for (int shape = _magicMouseItemStartFrame[animIndex]; shape <= _magicMouseItemEndFrame[animIndex]; ++shape) {
		restoreRect1(x, y);
		uint32 nextTime = _system->getMillis() + 4 * _tickLength;
		_screen->drawShape(0, _shapes[220+_itemInHand], x + 4, y + 3, 0, 0);
		if (tableIndex == -1) {
			_screen->drawShape(0, _shapes[4+shape], x, y, 0, 0);
		} else {
			specialMouseItemFX(shape, x, y, animIndex, tableIndex, loopStart, maxLoops);
		}
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if (nextTime - _system->getMillis() >= 10)
				delay(10);
		}
	}
	
	if (itemPos != -1) {
		restoreRect1(x, y);
		_screen->fillRect(_itemPosX[itemPos], _itemPosY[itemPos], _itemPosX[itemPos] + 15, _itemPosY[itemPos] + 15, 12, 0);
		backUpRect1(x, y);
	}
	
	for (int shape = _magicMouseItemStartFrame2[animIndex]; shape <= _magicMouseItemEndFrame2[animIndex]; ++shape) {
		restoreRect1(x, y);
		uint32 nextTime = _system->getMillis() + 4 * _tickLength;
		_screen->drawShape(0, _shapes[220+_itemInHand], x + 4, y + 3, 0, 0);
		if (tableIndex == -1) {
			_screen->drawShape(0, _shapes[4+shape], x, y, 0, 0);
		} else {
			specialMouseItemFX(shape, x, y, animIndex, tableIndex, loopStart, maxLoops);
		}
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if (nextTime - _system->getMillis() >= 10)
				delay(10);
		}
	}
	restoreRect1(x, y);
	if (itemPos == -1) {
		_screen->setMouseCursor(1, 1, _shapes[4]);
		_itemInHand = -1;
	} else {
		_characterList[0].inventoryItems[itemPos] = 0xFF;
		_screen->hideMouse();
		_screen->fillRect(_itemPosX[itemPos], _itemPosY[itemPos], _itemPosX[itemPos] + 15, _itemPosY[itemPos] + 15, 12, 0);
		_screen->showMouse();
	}
	_screen->showMouse();
	_screen->_curPage = videoPageBackUp;
}

void KyraEngine::magicInMouseItem(int animIndex, int item, int itemPos) {
	debug(9, "KyraEngine::magicInMouseItem(%d, %d, %d)", animIndex, item, itemPos);
	int videoPageBackUp = _screen->_curPage;
	_screen->_curPage = 0;
	int x = 0, y = 0;
	if (itemPos == -1) {
		x = _mouseX - 12;
		y = _mouseY - 18;
	} else {
		x = _itemPosX[itemPos] - 4;
		y = _itemPosX[itemPos] - 3;
	}
	if (item < 0)
		return;

	int tableIndex = -1, loopStart = 0, maxLoops = 0;
	if (animIndex == 0) {
		tableIndex = _rnd.getRandomNumberRng(0, 5);
		loopStart = 35;
		maxLoops = 9;
	} else if (animIndex == 1) {
		tableIndex = _rnd.getRandomNumberRng(0, 11);
		loopStart = 115;
		maxLoops = 8;
	} else if (animIndex == 2) {
		tableIndex = 0;
		loopStart = 124;
		maxLoops = 4;
	}
	
	_screen->hideMouse();
	backUpRect1(x, y);
	if (animIndex == 2) {
		// snd_playSoundEffect(0x5E);
	} else {
		// snd_playSoundEffect(0x37);
	}
	
	for (int shape = _magicMouseItemStartFrame[animIndex]; shape <= _magicMouseItemEndFrame[animIndex]; ++shape) {
		restoreRect1(x, y);
		uint32 nextTime = _system->getMillis() + 4 * _tickLength;
		if (tableIndex == -1) {
			_screen->drawShape(0, _shapes[4+shape], x, y, 0, 0);
		} else {
			specialMouseItemFX(shape, x, y, animIndex, tableIndex, loopStart, maxLoops);
		}
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if (nextTime - _system->getMillis() >= 10)
				delay(10);
		}
	}
	
	for (int shape = _magicMouseItemStartFrame2[animIndex]; shape <= _magicMouseItemEndFrame2[animIndex]; ++shape) {
		restoreRect1(x, y);
		uint32 nextTime = _system->getMillis() + 4 * _tickLength;
		if (tableIndex == -1) {
			_screen->drawShape(0, _shapes[4+shape], x, y, 0, 0);
		} else {
			specialMouseItemFX(shape, x, y, animIndex, tableIndex, loopStart, maxLoops);
		}
		_screen->updateScreen();
		while (_system->getMillis() < nextTime) {
			if (nextTime - _system->getMillis() >= 10)
				delay(10);
		}
	}
	restoreRect1(x, y);
	if (itemPos == -1) {
		_screen->setMouseCursor(8, 15, _shapes[220+item]);
		_itemInHand = item;
	} else {
		_characterList[0].inventoryItems[itemPos] = item;
		_screen->hideMouse();
		_screen->drawShape(0, _shapes[220+item], _itemPosX[itemPos], _itemPosY[itemPos], 0, 0);
		_screen->showMouse();
	}
	_screen->showMouse();
	_screen->_curPage = videoPageBackUp;
}

void KyraEngine::specialMouseItemFX(int shape, int x, int y, int animIndex, int tableIndex, int loopStart, int maxLoops) {
	debug(9, "KyraEngine::specialMouseItemFX(%d, %d, %d, %d, %d, %d, %d)", shape, x, y, animIndex, tableIndex, loopStart, maxLoops);
	static const uint8 table1[] = {
		0x23, 0x45, 0x55, 0x72, 0x84, 0xCF, 0x00, 0x00
	};
	static const uint8 table2[] = {
		0x73, 0xB5, 0x80, 0x21, 0x13, 0x39, 0x45, 0x55, 0x62, 0xB4, 0xCF, 0xD8
	};
	static const uint8 table3[] = {
		0x7C, 0xD0, 0x74, 0x84, 0x87, 0x00, 0x00, 0x00
	};
	int tableValue = 0;
	if (animIndex == 0) {
		tableValue = table1[tableIndex];
	} else if (animIndex == 1) {
		tableValue = table2[tableIndex];
	} else if (animIndex == 2) {
		tableValue = table3[tableIndex];
	} else {
		return;
	}
	processSpecialMouseItemFX(shape, x, y, tableValue, loopStart, maxLoops);
}

void KyraEngine::processSpecialMouseItemFX(int shape, int x, int y, int tableValue, int loopStart, int maxLoops) {
	debug(9, "KyraEngine::processSpecialMouseItemFX(%d, %d, %d, %d, %d, %d)", shape, x, y, tableValue, loopStart, maxLoops);
	uint8 shapeColorTable[16];
	uint8 *shapePtr = _shapes[4+shape] + 10;
	if (_features & GF_TALKIE)
		shapePtr += 2;
	for (int i = 0; i < 16; ++i) {
		shapeColorTable[i] = shapePtr[i];
	}
	for (int i = loopStart; i < loopStart + maxLoops; ++i) {
		for (int i2 = 0; i2 < 16; ++i2) {
			if (shapePtr[i2] == i) {
				shapeColorTable[i2] = (i + tableValue) - loopStart;
			}
		}
	}
	_screen->drawShape(0, _shapes[4+shape], x, y, 0, 0x8000, shapeColorTable);
}

void KyraEngine::updatePlayerItemsForScene() {
	debug(9, "KyraEngine::updatePlayerItemsForScene()");
	if (_itemInHand >= 29 && _itemInHand < 33) {
		++_itemInHand;
		if (_itemInHand > 33)
			_itemInHand = 33;
		_screen->hideMouse();
		_screen->setMouseCursor(8, 15, _shapes[220+_itemInHand]);
		_screen->showMouse();
	}
	
	bool redraw = false;
	for (int i = 0; i < 10; ++i) {
		uint8 item = _currentCharacter->inventoryItems[i];
		if (item >= 29 && item < 33) {
			++item;
			if (item > 33)
				item = 33;
			_currentCharacter->inventoryItems[i] = item;
			redraw = true;
		}
	}
	
	if (redraw) {
		_screen->hideMouse();
		redrawInventory(0);
		_screen->showMouse();
	}
	
	if (_itemInHand == 33) {
		magicOutMouseItem(2, -1);
	}
	
	_screen->hideMouse();
	for (int i = 0; i < 10; ++i) {
		uint8 item = _currentCharacter->inventoryItems[i];
		if (item == 33) {
			magicOutMouseItem(2, i);
		}
	}
	_screen->showMouse();
}

void KyraEngine::redrawInventory(int page) {
	int videoPageBackUp = _screen->_curPage;
	_screen->_curPage = page;
	_screen->hideMouse();
	for (int i = 0; i < 10; ++i) {
		_screen->fillRect(_itemPosX[i], _itemPosY[i], _itemPosX[i] + 15, _itemPosY[i] + 15, 12, page);
		if (_currentCharacter->inventoryItems[i] != 0xFF) {
			uint8 item = _currentCharacter->inventoryItems[i];
			_screen->drawShape(page, _shapes[220+item], _itemPosX[i], _itemPosY[i], 0, 0);
		}
	}
	_screen->showMouse();
	_screen->_curPage = videoPageBackUp;
	_screen->updateScreen();
}

#pragma mark -
#pragma mark - Animation/shape specific code
#pragma mark -

void KyraEngine::animRefreshNPC(int character) {
	debug(9, "KyraEngine::animRefreshNPC(%d)", character);
	AnimObject *animObj = &_animator->actors()[character];
	Character *ch = &_characterList[character];

	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	int facing = ch->facing;
	if (facing >= 1 && facing <= 3) {
		animObj->flags |= 1;
	} else if (facing >= 5 && facing <= 7) {
		animObj->flags &= 0xFFFFFFFE;
	}
	
	animObj->drawY = ch->y1;
	animObj->sceneAnimPtr = _shapes[4+ch->currentAnimFrame];
	animObj->animFrameNumber = ch->currentAnimFrame;
	if (character == 0) {
		if (_brandonStatusBit & 10) {
			animObj->animFrameNumber = 88;
			ch->currentAnimFrame = 88;
		}
		if (_brandonStatusBit & 2) {
			animObj->animFrameNumber = _brandonDrawFrame;
			ch->currentAnimFrame = _brandonDrawFrame;
			animObj->sceneAnimPtr = _shapes[4+_brandonDrawFrame];
			if (_brandonStatusBit0x02Flag) {
				++_brandonDrawFrame;
				if (_brandonDrawFrame >= 122)
					_brandonDrawFrame = 113;
					_brandonStatusBit0x02Flag = 0;
			}
		}
	}
	
	int xOffset = _defaultShapeTable[ch->currentAnimFrame-7].xOffset;
	int yOffset = _defaultShapeTable[ch->currentAnimFrame-7].yOffset;
	
	if (_scaleMode) {
		animObj->x1 = ch->x1;
		animObj->y1 = ch->y1;
		
		_brandonScaleX = _scaleTable[ch->y1];
		_brandonScaleY = _scaleTable[ch->y1];

		animObj->x1 += (_brandonScaleX * xOffset) >> 8;
		animObj->y1 += (_brandonScaleY * yOffset) >> 8;
	} else {
		animObj->x1 = ch->x1 + xOffset;
		animObj->y1 = ch->y1 + yOffset;
	}
	animObj->width2 = 4;
	animObj->height2 = 3;

	_animator->refreshObject(animObj);
}

void KyraEngine::drawJewelPress(int jewel, int drawSpecial) {
	debug(9, "KyraEngine::drawJewelPress(%d, %d)", jewel, drawSpecial);
	_screen->hideMouse();
	int shape = 0;
	if (drawSpecial) {
		shape = 0x14E;
	} else {
		shape = jewel + 0x149;
	}
	// snd_playSoundEffect(0x45);
	_screen->drawShape(0, _shapes[4+shape], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	delayWithTicks(2);
	if (drawSpecial) {
		shape = 0x148;
	} else {
		shape = jewel + 0x143;
	}
	_screen->drawShape(0, _shapes[4+shape], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	_screen->showMouse();
}

void KyraEngine::drawJewelsFadeOutStart() {
	debug(9, "KyraEngine::drawJewelsFadeOutStart()");
	static const uint16 jewelTable1[] = { 0x164, 0x15F, 0x15A, 0x155, 0x150, 0xFFFF };
	static const uint16 jewelTable2[] = { 0x163, 0x15E, 0x159, 0x154, 0x14F, 0xFFFF };
	static const uint16 jewelTable3[] = { 0x166, 0x160, 0x15C, 0x157, 0x152, 0xFFFF };
	static const uint16 jewelTable4[] = { 0x165, 0x161, 0x15B, 0x156, 0x151, 0xFFFF };
	for (int i = 0; jewelTable1[i] != 0xFFFF; ++i) {
		if (queryGameFlag(0x57)) {
			_screen->drawShape(0, _shapes[4+jewelTable1[i]], _amuletX2[2], _amuletY2[2], 0, 0);
		}
		if (queryGameFlag(0x59)) {
			_screen->drawShape(0, _shapes[4+jewelTable3[i]], _amuletX2[4], _amuletY2[4], 0, 0);
		}
		if (queryGameFlag(0x56)) {
			_screen->drawShape(0, _shapes[4+jewelTable2[i]], _amuletX2[1], _amuletY2[1], 0, 0);
		}
		if (queryGameFlag(0x58)) {
			_screen->drawShape(0, _shapes[4+jewelTable4[i]], _amuletX2[3], _amuletY2[3], 0, 0);
		}
		_screen->updateScreen();
		delayWithTicks(3);
	}
}

void KyraEngine::drawJewelsFadeOutEnd(int jewel) {
	debug(9, "KyraEngine::drawJewelsFadeOutEnd(%d)", jewel);
	static const uint16 jewelTable[] = { 0x153, 0x158, 0x15D, 0x162, 0x148, 0xFFFF };
	int newDelay = 0;
	switch (jewel-1) {
		case 2:
			if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
				newDelay = 18900;
			} else {
				newDelay = 8100;
			}
			break;
			
		default:
			newDelay = 3600;
			break;
	}
	setGameFlag(0xF1);
	setTimerCountdown(19, newDelay);
	_screen->hideMouse();
	for (int i = 0; jewelTable[i] != 0xFFFF; ++i) {
		uint16 shape = jewelTable[i];
		if (queryGameFlag(0x57)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[2], _amuletY2[2], 0, 0);
		}
		if (queryGameFlag(0x59)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[4], _amuletY2[4], 0, 0);
		}
		if (queryGameFlag(0x56)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[1], _amuletY2[1], 0, 0);
		}
		if (queryGameFlag(0x58)) {
			_screen->drawShape(0, _shapes[4+shape], _amuletX2[3], _amuletY2[3], 0, 0);
		}
		_screen->updateScreen();
		delayWithTicks(3);
	}
	_screen->showMouse();
}

void KyraEngine::setupShapes123(const Shape *shapeTable, int endShape, int flags) {
	debug(9, "KyraEngine::setupShapes123(0x%X, startShape, flags)", shapeTable, endShape, flags);
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
	debug(9, "KyraEngine::freeShapes123()");
	for (int i = 123; i <= 172; ++i) {
		free(_shapes[4+i]);
		_shapes[4+i] = NULL;
	}
}

void KyraEngine::setBrandonAnimSeqSize(int width, int height) {
	debug(9, "KyraEngine::setBrandonAnimSeqSize(%d, %d)", width, height);
	_animator->restoreAllObjectBackgrounds();
	_brandonAnimSeqSizeWidth = _animator->actors()[0].width;
	_brandonAnimSeqSizeHeight = _animator->actors()[0].height;
	_animator->actors()[0].width = width + 1;
	_animator->actors()[0].height = height;
	_animator->preserveAllBackgrounds();
}

void KyraEngine::resetBrandonAnimSeqSize() {
	_animator->restoreAllObjectBackgrounds();
	_animator->actors()[0].width = _brandonAnimSeqSizeWidth;
	_animator->actors()[0].height = _brandonAnimSeqSizeHeight;
	_animator->preserveAllBackgrounds();
}

#pragma mark -
#pragma mark - Misc stuff
#pragma mark -

Movie *KyraEngine::createWSAMovie() {
	// for kyra2 here could be added then WSAMovieV2
	return new WSAMovieV1(this);
}

int16 KyraEngine::fetchAnimWidth(const uint8 *shape, int16 mult) {
	debug(9, "KyraEngine::fetchAnimWidth(0x%X, %d)", shape, mult);
	if (_features & GF_TALKIE)
		shape += 2;
	return (((int16)READ_LE_UINT16((shape+3))) * mult) >> 8;
}

int16 KyraEngine::fetchAnimHeight(const uint8 *shape, int16 mult) {
	debug(9, "KyraEngine::fetchAnimHeight(0x%X, %d)", shape, mult);
	if (_features & GF_TALKIE)
		shape += 2;
	return (int16)(((int8)*(shape+2)) * mult) >> 8;
}

void KyraEngine::rectClip(int &x, int &y, int w, int h) {
	if (x < 0) {
		x = 0;
	} else if (x + w >= 320) {
		x = 320 - w;
	}
	if (y < 0) {
		y = 0;
	} else if (y + h >= 200) {
		y = 200 - h;
	}
}

void KyraEngine::backUpRect0(int xpos, int ypos) {
	debug(9, "KyraEngine::backUpRect0(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 3<<3, 24);
	_screen->copyRegionToBuffer(_screen->_curPage, xpos, ypos, 3<<3, 24, _shapes[0]);
}

void KyraEngine::restoreRect0(int xpos, int ypos) {
	debug(9, "KyraEngine::restoreRect0(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 3<<3, 24);
	_screen->copyBlockToPage(_screen->_curPage, xpos, ypos, 3<<3, 24, _shapes[0]);
}

void KyraEngine::backUpRect1(int xpos, int ypos) {
	debug(9, "KyraEngine::backUpRect1(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 4<<3, 32);
	_screen->copyRegionToBuffer(_screen->_curPage, xpos, ypos, 4<<3, 32, _shapes[1]);
}

void KyraEngine::restoreRect1(int xpos, int ypos) {
	debug(9, "KyraEngine::restoreRect1(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 4<<3, 32);
	_screen->copyBlockToPage(_screen->_curPage, xpos, ypos, 4<<3, 32, _shapes[1]);
}

int KyraEngine::getDrawLayer(int x, int y) {
	debug(9, "KyraEngine::getDrawLayer(%d, %d)", x, y);
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;
	for (int curX = xpos; curX < xpos + 16; ++curX) {
		int tempLayer = _screen->getShapeFlag2(curX, ypos);
		if (layer < tempLayer) {
			layer = tempLayer;
		}
		if (layer >= 7) {
			return 7;
		}
	}
	return layer;
}

int KyraEngine::getDrawLayer2(int x, int y, int height) {
	debug(9, "KyraEngine::getDrawLayer2(%d, %d, %d)", x, y, height);
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;
	
	for (int useX = xpos; useX < xpos + 16; ++useX) {
		for (int useY = ypos - height; useY < ypos; ++useY) {
			int tempLayer = _screen->getShapeFlag2(useX, useY);
			if (tempLayer > layer) {
				layer = tempLayer;
			}
			
			if (tempLayer >= 7) {
				return 7;
			}
		}
	}	
	return layer;
}

void KyraEngine::copyBackgroundBlock(int x, int page, int flag) {
	debug(9, "KyraEngine::copyBackgroundBlock(%d, %d, %d)", x, page, flag);
	
	if (x < 1)
		return;
	
	int height = 128;
	if (flag)
		height += 8;	
	if (!(x & 1))
		++x;
	if (x == 19)
		x = 17;
	uint8 *ptr1 = _unkPtr1;
	uint8 *ptr2 = _unkPtr2;
	int oldVideoPage = _screen->_curPage;
	_screen->_curPage = page;
	
	int curX = x;
	_screen->hideMouse();
	_screen->copyRegionToBuffer(_screen->_curPage, 8, 8, 8, height, ptr2);
	for (int i = 0; i < 19; ++i) {
		int tempX = curX + 1;
		_screen->copyRegionToBuffer(_screen->_curPage, tempX<<3, 8, 8, height, ptr1);
		_screen->copyBlockToPage(_screen->_curPage, tempX<<3, 8, 8, height, ptr2);
		int newXPos = curX + x;
		if (newXPos > 37) {
			newXPos = newXPos % 38;
		}
		tempX = newXPos + 1;
		_screen->copyRegionToBuffer(_screen->_curPage, tempX<<3, 8, 8, height, ptr2);
		_screen->copyBlockToPage(_screen->_curPage, tempX<<3, 8, 8, height, ptr1);
		curX += x*2;
		if (curX > 37) {
			curX = curX % 38;
		}
	}
	_screen->showMouse();
	_screen->_curPage = oldVideoPage;
}

void KyraEngine::copyBackgroundBlock2(int x) {
	copyBackgroundBlock(x, 4, 1);
}

void KyraEngine::makeBrandonFaceMouse() {
	debug(9, "KyraEngine::makeBrandonFaceMouse()");
	if (_mouseX >= _currentCharacter->x1) {
		_currentCharacter->facing = 3;
	} else {
		_currentCharacter->facing = 5;
	}
	animRefreshNPC(0);
	_animator->updateAllObjectShapes();
}

void KyraEngine::setBrandonPoisonFlags(int reset) {
	debug(9, "KyraEngine::setBrandonPoisonFlags(%d)", reset);
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
	debug(9, "KyraEngine::resetBrandonPoisonFlags()");
	_brandonStatusBit = 0;
	for (int i = 0; i < 0x100; ++i) {
		_brandonPoisonFlagsGFX[i] = i;
	}
}

void KyraEngine::setupPanPages() {
	debug(9, "KyraEngine::setupPanPages()");
	loadBitmap("bead.cps", 3, 3, 0);
	for (int i = 0; i <= 19; ++i) {
		_panPagesTable[i] = _seq->setPanPages(3, i);
	}
}

void KyraEngine::freePanPages() {
	debug(9, "KyraEngine::freePanPages()");
	delete _endSequenceBackUpRect;
	_endSequenceBackUpRect = 0;
	for (int i = 0; i <= 19; ++i) {
		free(_panPagesTable[i]);
		_panPagesTable[i] = NULL;
	}
}

void KyraEngine::closeFinalWsa() {
	debug(9, "KyraEngine::closeFinalWsa()");
	delete _finalA;
	_finalA = 0;
	delete _finalB;
	_finalB = 0;
	delete _finalC;
	_finalC = 0;
	freePanPages();
	_endSequenceNeedLoading = 1;
}

int KyraEngine::handleMalcolmFlag() {
	debug(9, "KyraEngine::handleMalcolmFlag()");
	static uint16 frame = 0;
	static uint32 timer1 = 0;
	static uint32 timer2 = 0;
	
	switch (_malcolmFlag) {
		case 1:
			frame = 0;
			_malcolmFlag = 2;
			timer2 = 0;
		case 2:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 13) {
					_malcolmFlag = 3;
					timer1 = _system->getMillis() + 180 * _tickLength;
				}
			}
			break;
		
		case 3:
			if (_system->getMillis() < timer1) {
				if (_system->getMillis() >= timer2) {
					frame = _rnd.getRandomNumberRng(14, 17);
					_finalA->_x = 8;
					_finalA->_y = 46;
					_finalA->_drawPage = 0;
					_finalA->displayFrame(frame);
					_screen->updateScreen();
					timer2 = _system->getMillis() + 8 * _tickLength;
				}
			} else {
				_malcolmFlag = 4;
				frame = 18;
			}
			break;
		
		case 4:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 25) {
					frame = 26;
					_malcolmFlag = 5;
					_beadStateVar = 1;
				}
			}
			break;
		
		case 5:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 31) {
					frame = 32;
					_malcolmFlag = 6;
				}
			}
			break;
			
		case 6:
			if (_unkEndSeqVar4) {
				if (frame <= 33 && _system->getMillis() >= timer2) {
					_finalA->_x = 8;
					_finalA->_y = 46;
					_finalA->_drawPage = 0;
					_finalA->displayFrame(frame);
					_screen->updateScreen();
					timer2 = _system->getMillis() + 8 * _tickLength;
					++frame;
					if (frame > 33) {
						_malcolmFlag = 7;
						frame = 32;
						_unkEndSeqVar5 = 0;
					}
				}
			}
			break;
		
		case 7:
			if (_unkEndSeqVar5 == 1) {
				_malcolmFlag = 8;
				frame = 34;
			} else if (_unkEndSeqVar5 == 2) {
				_malcolmFlag = 3;
				timer1 = _system->getMillis() + 180 * _tickLength;
			}
			break;
		
		case 8:
			if (_system->getMillis() >= timer2) {
				_finalA->_x = 8;
				_finalA->_y = 46;
				_finalA->_drawPage = 0;
				_finalA->displayFrame(frame);
				_screen->updateScreen();
				timer2 = _system->getMillis() + 8 * _tickLength;
				++frame;
				if (frame > 37) {
					_malcolmFlag = 0;
					_deathHandler = 8;
					return 1;
				}
			}
			break;
		
		case 9:
			// snd_playSoundEffect(12);
			// snd_playSoundEffect(12);
			_finalC->_x = 16;
			_finalC->_y = 50;
			_finalC->_drawPage = 0;
			for (int i = 0; i < 18; ++i) {
				timer2 = _system->getMillis() + 4 * _tickLength;
				_finalC->displayFrame(i);
				_screen->updateScreen();
				while (_system->getMillis() < timer2) {}
			}
			// XXX
			waitTicks(60);
			_malcolmFlag = 0;
			return 1;
			break;
		
		case 10:
			if (!_beadStateVar) {
				handleBeadState();
				_screen->bitBlitRects();
				assert(_veryClever);
				_text->printTalkTextMessage(_veryClever[0], 60, 31, 5, 0, 2);
				timer2 = _system->getMillis() + 180 * _tickLength;
				_malcolmFlag = 11;
			}
			break;
		
		case 11:
			if (_system->getMillis() >= timer2) {
				_text->restoreTalkTextMessageBkgd(2, 0);
				_malcolmFlag = 3;
				timer1 = _system->getMillis() + 180 * _tickLength;
			}
			break;
		
		default:
			break;
	}
	
	return 0;
}

int KyraEngine::handleBeadState() {
	debug(9, "KyraEngine::handleBeadState()");
	static uint32 timer1 = 0;
	static uint32 timer2 = 0;
	static BeadState beadState1 = { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static BeadState beadState2 = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	
	static const int table1[] = {
		-1, -2, -4, -5, -6, -7, -6, -5,
		-4, -2, -1,  0,  1,  2,  4,  5,
		 6,  7,  6,  5,  4,  2,  1,  0
	};
	static const int table2[] = {
		0, 0, 1, 1, 2, 2, 3, 3,
		4, 4, 5, 5, 5, 5, 4, 4,
		3, 3, 2, 2, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};
	
	switch (_beadStateVar) {
		case 0:
			if (beadState1.x != -1 && _endSequenceBackUpRect) {
				_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
			} else {
				beadState1.x = -1;
				beadState1.tableIndex = 0;
				timer1 = 0;
				timer2 = 0;
				_lastDisplayedPanPage = 0;
				return 1;
			}
		
		case 1:
			if (beadState1.x != -1) {
				if (_endSequenceBackUpRect) {
					_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				}
				beadState1.x = -1;
				beadState1.tableIndex = 0;
			}
			_beadStateVar = 2;
			break;
		
		case 2:
			if (_system->getMillis() >= timer1) {
				int x = 0, y = 0;
				timer1 = _system->getMillis() + 4 * _tickLength;
				if (beadState1.x == -1) {
					assert(_panPagesTable);
					beadState1.width2 = fetchAnimWidth(_panPagesTable[19], 256);
					beadState1.width = ((beadState1.width2 + 7) >> 3) + 1;
					beadState1.height = fetchAnimHeight(_panPagesTable[19], 256);
					if (!_endSequenceBackUpRect) {
						_endSequenceBackUpRect = new uint8[(beadState1.width * beadState1.height) << 3];
						assert(_endSequenceBackUpRect);
						memset(_endSequenceBackUpRect, 0, ((beadState1.width * beadState1.height) << 3) * sizeof(uint8));
					}
					x = beadState1.x = 60;
					y = beadState1.y = 40;
					initBeadState(x, y, x, 25, 8, &beadState2);
				} else {
					if (processBead(beadState1.x, beadState1.y, x, y, &beadState2)) {
						_beadStateVar = 3;
						timer2 = _system->getMillis() + 240 * _tickLength;
						_unkEndSeqVar4 = 0;
						beadState1.dstX = beadState1.x;
						beadState1.dstY = beadState1.y;
						return 0;
					} else {
						_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
						_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
						beadState1.x = x;
						beadState1.y = y;
					}
				}
				_screen->copyCurPageBlock(x >> 3, y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
				if (_lastDisplayedPanPage > 17)
					_lastDisplayedPanPage = 0;
				_screen->addBitBlitRect(x, y, beadState1.width2, beadState1.height);
			}
			break;
		
		case 3:
			if (_system->getMillis() >= timer1) {
				timer1 = _system->getMillis() + 4 * _tickLength;
				_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				beadState1.x = beadState1.dstX + table1[beadState1.tableIndex];
				beadState1.y = beadState1.dstY + table2[beadState1.tableIndex];
				_screen->copyCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
				_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], beadState1.x, beadState1.y, 0, 0);
				if (_lastDisplayedPanPage >= 17) {
					_lastDisplayedPanPage = 0;
				}
				_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				++beadState1.tableIndex;
				if (beadState1.tableIndex > 24) {
					beadState1.tableIndex = 0;
					_unkEndSeqVar4 = 1;
				}
				if (_system->getMillis() > timer2 && _malcolmFlag == 7 && !_unkAmuletVar && !_text->printed()) {
					// snd_playSoundEffect(0x0B);
					if (_currentCharacter->x1 > 233 && _currentCharacter->x1 < 305 && _currentCharacter->y1 > 85 && _currentCharacter->y1 < 105 &&
						(_brandonStatusBit & 0x20)) {
						beadState1.unk8 = 290;
						beadState1.unk9 = 40;
						_beadStateVar = 5;
					} else {
						_beadStateVar = 4;
						beadState1.unk8 = _currentCharacter->x1 - 4;
						beadState1.unk9 = _currentCharacter->y1 - 30;
					}
					
					if (_text->printed()) {
						_text->restoreTalkTextMessageBkgd(2, 0);
					}
					initBeadState(beadState1.x, beadState1.y, beadState1.unk8, beadState1.unk9, 6, &beadState2);
					_lastDisplayedPanPage = 18;
				}
			}
			break;
			
		case 4:
			if (_system->getMillis() >= timer1) {
				int x = 0, y = 0;
				timer1 = _system->getMillis();
				if (processBead(beadState1.x, beadState1.y, x, y, &beadState2)) {
					if (_brandonStatusBit & 20) {
						_unkEndSeqVar5 = 2;
						_beadStateVar = 6;
					} else {
						// XXX
						// snd_playSoundEffect(0x0C);
						_unkEndSeqVar5 = 1;
						_beadStateVar = 0;
					}
				} else {
					_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
					beadState1.x = x;
					beadState1.y = y;
					_screen->copyCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
					if (_lastDisplayedPanPage > 17) {
						_lastDisplayedPanPage = 0;
					}
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				}
			}
			break;
		
		case 5:
			if (_system->getMillis() >= timer1) {
				timer1 = _system->getMillis();
				int x = 0, y = 0;
				if (processBead(beadState1.x, beadState1.y, x, y, &beadState2)) {
					if (beadState1.dstX == 290) {
						_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
						uint32 nextRun = 0;
						_finalB->_x = 224;
						_finalB->_y = 8;
						_finalB->_drawPage = 0;
						for (int i = 0; i < 8; ++i) {
							nextRun = _system->getMillis() + _tickLength;
							_finalB->displayFrame(i);
							_screen->updateScreen();
							while (_system->getMillis() < nextRun) {}
						}
						// snd_playSoundEffect(0x0D);
						for (int i = 7; i >= 0; --i) {
							nextRun = _system->getMillis() + _tickLength;
							_finalB->displayFrame(i);
							_screen->updateScreen();
							while (_system->getMillis() < nextRun) {}
						}
						initBeadState(beadState1.x, beadState1.y, 63, 60, 6, &beadState2);
					} else {
						_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
						_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
						beadState1.x = -1;
						beadState1.tableIndex = 0;
						_beadStateVar = 0;
						_malcolmFlag = 9;
					}
				} else {
					_screen->copyFromCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
					beadState1.x = x;
					beadState1.y = y;
					_screen->copyCurPageBlock(beadState1.x >> 3, beadState1.y, beadState1.width, beadState1.height, _endSequenceBackUpRect);
					_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
					if (_lastDisplayedPanPage > 17) {
						_lastDisplayedPanPage = 0;
					}
					_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
				}
			}
			break;
		
		case 6:
			_screen->drawShape(2, _panPagesTable[19], beadState1.x, beadState1.y, 0, 0);
			_screen->addBitBlitRect(beadState1.x, beadState1.y, beadState1.width2, beadState1.height);
			_beadStateVar = 0;
			break;
		
		default:
			break;
	}
	return 0;
}

void KyraEngine::initBeadState(int x, int y, int x2, int y2, int unk, BeadState *ptr) {
	debug(9, "KyraEngine::initBeadState(%d, %d, %d, %d, %d, 0x%X)", x, y, x2, y2, unk, ptr);
	ptr->unk9 = unk;
	int xDiff = x2 - x;
	int yDiff = y2 - y;
	int unk1 = 0, unk2 = 0;
	if (xDiff > 0) {
		unk1 = 1;
	} else if (xDiff == 0) {
		unk1 = 0;
	} else {
		unk1 = -1;
	}
	
	if (yDiff > 0) {
		unk2 = 1;
	} else if (yDiff == 0) {
		unk2 = 0;
	} else {
		unk2 = -1;
	}
	
	xDiff = abs(xDiff);
	yDiff = abs(yDiff);
	
	ptr->y = 0;
	ptr->x = 0;
	ptr->width = xDiff;
	ptr->height = yDiff;
	ptr->dstX = x2;
	ptr->dstY = y2;
	ptr->width2 = unk1;
	ptr->unk8 = unk2;
}

int KyraEngine::processBead(int x, int y, int &x2, int &y2, BeadState *ptr) {
	debug(9, "KyraEngine::processBead(%d, %d, 0x%X, 0x%X, 0x%X)", x, y, &x2, &y2, ptr);
	if (x == ptr->dstX && y == ptr->dstY) {
		return 1;
	}
	
	int xPos = x, yPos = y;
	if (ptr->width >= ptr->height) {
		for (int i = 0; i < ptr->unk9; ++i) {
			ptr->y += ptr->height;
			if (ptr->y >= ptr->width) {
				ptr->y -= ptr->width;
				yPos += ptr->unk8;
			}
			xPos += ptr->width2;
		}
	} else {
		for (int i = 0; i < ptr->unk9; ++i) {
			ptr->x += ptr->width;
			if (ptr->x >= ptr->height) {
				ptr->x -= ptr->height;
				xPos += ptr->width2;
			}
			yPos += ptr->unk8;
		}
	}
	
	int temp = abs(x - ptr->dstX);
	if (ptr->unk9 > temp) {
		xPos = ptr->dstX;
	}
	temp = abs(y - ptr->dstY);
	if (ptr->unk9 > temp) {
		yPos = ptr->dstY;
	}
	x2 = xPos;
	y2 = yPos;
	return 0;
}

#pragma mark -
#pragma mark - Pathfinder
#pragma mark -

int KyraEngine::findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize) {
	debug(9, "KyraEngine::findWay(%d, %d, %d, %d, 0x%X, %d)", x, y, toX, toY, moveTable, moveTableSize);
	x &= 0xFFFC; toX &= 0xFFFC;
	y &= 0xFFFE; toY &= 0xFFFE;
	x = (int16)x; y = (int16)y; toX = (int16)toX; toY = (int16)toY;
	
	if (x == toY && y == toY) {
		moveTable[0] = 8;
		return 0;
	}
	
	int curX = x;
	int curY = y;
	int lastUsedEntry = 0;
	int tempValue = 0;
	int *pathTable1 = new int[0x7D0];
	int *pathTable2 = new int[0x7D0];
	assert(pathTable1 && pathTable2);
	
	while (true) {
		int newFacing = getFacingFromPointToPoint(x, y, toX, toY);
		changePosTowardsFacing(curX, curY, newFacing);
		
		if (curX == toX && curY == toY) {
			if (!lineIsPassable(curX, curY))
				break;
			moveTable[lastUsedEntry++] = newFacing;
			break;
		}
		
		if (lineIsPassable(curX, curY)) {
			if (lastUsedEntry == moveTableSize) {
				delete [] pathTable1;
				delete [] pathTable2;
				return 0x7D00;
			}
			// debug drawing
			//if (curX >= 0 && curY >= 0 && curX < 320 && curY < 200) {
			//	_screen->setPagePixel(0, curX, curY, 11);
			//	_screen->updateScreen();
			//	waitTicks(5);
			//}
			moveTable[lastUsedEntry++] = newFacing;
			x = curX;
			y = curY;
			continue;
		}
		
		int temp = 0;
		while (true) {
			newFacing = getFacingFromPointToPoint(curX, curY, toX, toY);
			changePosTowardsFacing(curX, curY, newFacing);
			// debug drawing
			//if (curX >= 0 && curY >= 0 && curX < 320 && curY < 200) {
			//	_screen->setPagePixel(0, curX, curY, 8);
			//	_screen->updateScreen();
			//	waitTicks(5);
			//}
			
			if (!lineIsPassable(curX, curY)) {
				if (curX != toX || curY != toY)
					continue;
			}
			
			if (curX == toX && curY == toY) {
				if (!lineIsPassable(curX, curY)) {
					tempValue = 0;
					temp = 0;
					break;
				}
			}
			
			temp = findSubPath(x, y, curX, curY, pathTable1, 1, 0x7D0);
			tempValue = findSubPath(x, y, curX, curY, pathTable2, 0, 0x7D0);
			if (curX == toX && curY == toY) {
				if (temp == 0x7D00 && tempValue == 0x7D00) {
					delete [] pathTable1;
					delete [] pathTable2;
					return 0x7D00;
				}
			}
			
			if (temp != 0x7D00 || tempValue != 0x7D00) {
				break;
			}
		}
		
		if (temp < tempValue) {
			if (lastUsedEntry + temp > moveTableSize) {
				delete [] pathTable1;
				delete [] pathTable2;
				return 0x7D00;
			}
			memcpy(&moveTable[lastUsedEntry], pathTable1, temp*sizeof(int));
			lastUsedEntry += temp;
		} else {
			if (lastUsedEntry + tempValue > moveTableSize) {
				delete [] pathTable1;
				delete [] pathTable2;
				return 0x7D00;
			}
			memcpy(&moveTable[lastUsedEntry], pathTable2, tempValue*sizeof(int));
			lastUsedEntry += tempValue;
		}
		x = curX;
		y = curY;
		if (curX == toX && curY == toY) {
			break;
		}
	}
	delete [] pathTable1;
	delete [] pathTable2;
	moveTable[lastUsedEntry] = 8;
	return getMoveTableSize(moveTable);
}

int KyraEngine::findSubPath(int x, int y, int toX, int toY, int *moveTable, int start, int end) {
	debug(9, "KyraEngine::findSubPath(%d, %d, %d, %d, 0x%X, %d, %d)", x, y, toX, toY, moveTable, start, end);
	// only used for debug specific code
	//static uint16 unkTable[] = { 8, 5 };
	static const int8 facingTable1[] = {  7,  0,  1,  2,  3,  4,  5,  6,  1,  2,  3,  4,  5,  6,  7,  0 };
	static const int8 facingTable2[] = { -1,  0, -1,  2, -1,  4, -1,  6, -1,  2, -1,  4, -1,  6, -1,  0 };
	static const int8 facingTable3[] = {  2,  4,  4,  6,  6,  0,  0,  2,  6,  6,  0,  0,  2,  2,  4,  4 };
	static const int8 addPosTableX[] = { -1,  0, -1,  4, -1,  0, -1, -4, -1, -4, -1,  0, -1,  4, -1,  0 };
	static const int8 addPosTableY[] = { -1,  2, -1,  0, -1, -2, -1,  0, -1,  0, -1,  2, -1,  0, -1, -2 };
	
	// debug specific
	//++unkTable[start];
	//while (_screen->getPalette(0)[unkTable[start]] != 0x0F) {
	//	++unkTable[start];
	//}
	
	int xpos1 = x, xpos2 = x;
	int ypos1 = y, ypos2 = y;
	int newFacing = getFacingFromPointToPoint(x, y, toX, toY);
	int position = 0;
	
	while (position != end) {
		int newFacing2 = newFacing;
		while (true) {
			changePosTowardsFacing(xpos1, ypos1, facingTable1[start*8 + newFacing2]);
			if (!lineIsPassable(xpos1, ypos1)) {
				if (facingTable1[start*8 + newFacing2] == newFacing) {
					return 0x7D00;
				}
				newFacing2 = facingTable1[start*8 + newFacing2];
				xpos1 = x;
				ypos1 = y;
				continue;
			}
			newFacing = facingTable1[start*8 + newFacing2];
			break;
		}
		// debug drawing
		//if (xpos1 >= 0 && ypos1 >= 0 && xpos1 < 320 && ypos1 < 200) {
		//	_screen->setPagePixel(0, xpos1, ypos1, unkTable[start]);
		//	_screen->updateScreen();
		//	waitTicks(5);
		//}
		if (newFacing & 1) {
			int temp = xpos1 + addPosTableX[newFacing + start * 8];
			if (toX == temp) {
				temp = ypos1 + addPosTableY[newFacing + start * 8];
				if (toY == temp) {
					moveTable[position++] = facingTable2[newFacing + start * 8];
					return position;
				}
			}
		}
		moveTable[position++] = newFacing;
		x = xpos1;
		y = ypos1;
		if (x == toX && y == toY) {
			return position;
		}
		
		if (xpos1 == xpos2 && ypos1 == ypos2) {
			break;
		}
		
		newFacing = facingTable3[start*8 + newFacing];
	}
	return 0x7D00;
}

int KyraEngine::getFacingFromPointToPoint(int x, int y, int toX, int toY) {
	debug(9, "KyraEngine::getFacingFromPointToPoint(%d, %d, %d, %d)", x, y, toX, toY);
	static const int facingTable[] = {
		1, 0, 1, 2, 3, 4, 3, 2, 7, 0, 7, 6, 5, 4, 5, 6
	};
	
	int facingEntry = 0;
	int ydiff = y - toY;
	if (ydiff < 0) {
		++facingEntry;
		ydiff = -ydiff;
	}	
	facingEntry <<= 1;
	
	int xdiff = toX - x;
	if (xdiff < 0) {
		++facingEntry;
		xdiff = -xdiff;
	}
	
	if (xdiff >= ydiff) {
		int temp = ydiff;
		ydiff = xdiff;
		xdiff = temp;
		
		facingEntry <<= 1;
	} else {
		facingEntry <<= 1;
		facingEntry += 1;
	}
	int temp = (ydiff + 1) >> 1;
	
	if (xdiff < temp) {
		facingEntry <<= 1;
		facingEntry += 1;
	} else {
		facingEntry <<= 1;
	}
	assert(facingEntry < ARRAYSIZE(facingTable));
	return facingTable[facingEntry];
}

void KyraEngine::changePosTowardsFacing(int &x, int &y, int facing) {
	debug(9, "KyraEngine::changePosTowardsFacing(%d, %d, %d)", x, y, facing);
	x += _addXPosTable[facing];
	y += _addYPosTable[facing];
}

bool KyraEngine::lineIsPassable(int x, int y) {
	debug(9, "KyraEngine::lineIsPassable(%d, %d)", x, y);
	if (queryGameFlag(0xEF)) {
		if (_currentCharacter->sceneId == 5)
			return true;
	}
	
	if (_pathfinderFlag & 2) {
		if (x >= 312)
			return false;
	}
	
	if (_pathfinderFlag & 4) {
		if (y >= 136)
			return false;
	}
	
	if (_pathfinderFlag & 8) {
		if (x < 8)
			return false;
	}
	
	if (_pathfinderFlag2) {
		if (x <= 8 || x >= 312)
			return true;
		if (y < (_northExitHeight & 0xFF) || y > 135)
			return true;
	}
	
	if (y > 137) {
		return false;
	}
	
	int ypos = 8;
	if (_scaleMode) {
		ypos = (_scaleTable[y] >> 5) + 1;
		if (8 < ypos)
			ypos = 8;
	}
	
	x -= (ypos >> 1);
	if (y < 0)
		y = 0;
	
	int xpos = x;
	int xtemp = xpos + ypos - 1;
	if (x < 0)
		xpos = 0;
		
	if (xtemp > 319)
		xtemp = 319;

	for (; xpos < xtemp; ++xpos) {
		if (!_screen->getShapeFlag1(xpos, y))
			return false;
	}
	return true;
}

int KyraEngine::getMoveTableSize(int *moveTable) {
	debug(9, "KyraEngine::getMoveTableSize(0x%X)", moveTable);
	int retValue = 0;
	if (moveTable[0] == 8)
		return 0;
	
	static const int facingTable[] = {
		4, 5, 6, 7, 0, 1, 2, 3
	};
	static const int unkTable[] = {
		-1, -1,  1,  2, -1,  6,  7, -1,
		-1, -1, -1, -1,  2, -1,  0, -1,
		 1, -1, -1, -1,  3,  4, -1,  0,
		 2, -1, -1, -1, -1, -1,  4, -1,
		-1,  2,  3, -1, -1, -1,  5,  6,
		 6, -1,  4, -1, -1, -1, -1, -1,
		 7,  0, -1,  4,  5, -1, -1, -1,
		-1, -1,  0, -1,  6, -1, -1, -1
	};
	
	int *oldPosition = moveTable;
	int *tempPosition = moveTable;
	int *curPosition = moveTable + 1;
	retValue = 1;

	while (*curPosition != 8) {
		if (*oldPosition == facingTable[*curPosition]) {
			retValue -= 2;
			*oldPosition = 9;
			*curPosition = 9;
			
			while (tempPosition != moveTable) {
				--tempPosition;
				if (*tempPosition != 9)
					break;
			}
			
			if (tempPosition == moveTable && *tempPosition == 9) {
				while (*tempPosition != 8 && *tempPosition == 9) {
					++tempPosition;
				}
				if (*tempPosition == 8) {
					return 0;
				}
			}
			
			oldPosition = tempPosition;
			curPosition = oldPosition+1;
			while (*curPosition != 8 && *curPosition == 9) {
				++curPosition;
			}
			continue;
		}
		
		if (unkTable[*curPosition+((*oldPosition)*8)] != -1) {
			--retValue;
			*oldPosition = unkTable[*curPosition+((*oldPosition)*8)];
			*curPosition = 9;
			
			if (tempPosition != oldPosition) {
				curPosition = oldPosition;
				oldPosition = tempPosition;
				while (true) {
					if (tempPosition == moveTable) {
						break;
					}
					--tempPosition;
					if (*tempPosition != 9) {
						break;
					}
				}
			} else {
				while (true) {
					++curPosition;
					if (*curPosition != 9) {
						break;
					}
				}
			}
			continue;
		}
		
		tempPosition = oldPosition;
		oldPosition = curPosition;
		++retValue;
		while (true) {
			++curPosition;
			if (*curPosition != 9) {
				break;
			}
		}
	}

	return retValue;
}

#pragma mark -
#pragma mark - Scene handling
#pragma mark -

int KyraEngine::handleSceneChange(int xpos, int ypos, int unk1, int frameReset) {
	debug(9, "KyraEngine::handleSceneChange(%d, %d, %d, %d)", xpos, ypos, unk1, frameReset);
	if (queryGameFlag(0xEF)) {
		unk1 = 0;
	}
	int sceneId = _currentCharacter->sceneId;
	_pathfinderFlag = 0;
	if (xpos < 12) {
		if (_roomTable[sceneId].westExit != 0xFFFF) {
			xpos = 12;
			ypos = _sceneExits.westYPos;
			_pathfinderFlag = 7;
		}
	} else if(xpos >= 308) {
		if (_roomTable[sceneId].eastExit != 0xFFFF) {
			xpos = 307;
			ypos = _sceneExits.eastYPos;
			_pathfinderFlag = 13;
		}
	}
	
	if (ypos <= (_northExitHeight&0xFF)+2) {
		if (_roomTable[sceneId].northExit != 0xFFFF) {
			xpos = _sceneExits.northXPos;
			ypos = _northExitHeight & 0xFF;
			_pathfinderFlag = 14;
		}
	} else if (ypos >= 136) {
		if (_roomTable[sceneId].southExit != 0xFFFF) {
			xpos = _sceneExits.southXPos;
			ypos = 136;
			_pathfinderFlag = 11;
		}
	}
	
	int temp = xpos - _currentCharacter->x1;
	if (ABS(temp) < 4) {
		temp = ypos - _currentCharacter->y1;
		if (ABS(temp) < 2) {
			return 0;
		}
	}
	
	int x = (int16)(_currentCharacter->x1 & 0xFFFC);
	int y = (int16)(_currentCharacter->y1 & 0xFFFE);
	xpos = (int16)(xpos & 0xFFFC);
	ypos = (int16)(ypos & 0xFFFE);
	int ret = findWay(x, y, xpos, ypos, _movFacingTable, 150);
	_pathfinderFlag = 0;
	if (ret >= _lastFindWayRet) {
		_lastFindWayRet = ret;
	}
	if (ret == 0x7D00 || ret == 0) {
		return 0;
	}
	return processSceneChange(_movFacingTable, unk1, frameReset);
}

int KyraEngine::processSceneChange(int *table, int unk1, int frameReset) {
	debug(9, "KyraEngine::processSceneChange(0x%X, %d, %d)", table, unk1, frameReset);
	if (queryGameFlag(0xEF)) {
		unk1 = 0;
	}
	int *tableStart = table;
	_sceneChangeState = 0;
	_loopFlag2 = 0;
	bool running = true;
	int returnValue = 0;
	uint32 nextFrame = 0;
	_abortWalkFlag = false;
	_mousePressFlag = false;
	while (running) {
		if (_abortWalkFlag) {
			*table = 8;
			_currentCharacter->currentAnimFrame = 7;
			animRefreshNPC(0);
			_animator->updateAllObjectShapes();
			processInput(_mouseX, _mouseY);
			return 0;
		}
		bool forceContinue = false;
		switch (*table) {
			case 0: case 1: case 2:
			case 3: case 4: case 5:
			case 6: case 7:
				_currentCharacter->facing = getOppositeFacingDirection(*table);
				break;
			
			case 8:
				forceContinue = true;
				running = false;
				break;
			
			default:
				++table;
				forceContinue = true;
				break;
		}
		
		returnValue = changeScene(_currentCharacter->facing);
		if (returnValue) {
			running = false;
			_abortWalkFlag = false;
		}
		
		if (unk1) {
			if (_mousePressFlag) {
				running = false;
				_sceneChangeState = 1;
			}
		}
		
		if (forceContinue || !running) {
			continue;
		}
		
		int temp = 0;
		if (table == tableStart || table[1] == 8) {
			temp = setCharacterPosition(0, 0);
		} else {
			temp = setCharacterPosition(0, table);
		}
		if (temp) {
			++table;
		}
		
		nextFrame = getTimerDelay(5) * _tickLength + _system->getMillis();
		while (_system->getMillis() < nextFrame) {
			_sprites->updateSceneAnims();
			updateMousePointer();
			updateGameTimers();
			_animator->updateAllObjectShapes();
			updateTextFade();
			if (_currentCharacter->sceneId == 210) {
				_animator->updateKyragemFading();
				if (seq_playEnd() || _beadStateVar == 4 || _beadStateVar == 5) {
					*table = 8;
					running = false;
					break;
				}
			}
			if ((nextFrame - _system->getMillis()) >= 10)
				delay(10);
		}
	}
	
	if (frameReset && !(_brandonStatusBit & 2)) {
		_currentCharacter->currentAnimFrame = 7;
	}
	animRefreshNPC(0);
	_animator->updateAllObjectShapes();
	return returnValue;
}

int KyraEngine::changeScene(int facing) {
	debug(9, "KyraEngine::changeScene(%d)", facing);
	if (queryGameFlag(0xEF)) {
		if (_currentCharacter->sceneId == 5) {
			return 0;
		}
	}
	
	int xpos = _charXPosTable[facing] + _currentCharacter->x1;
	int ypos = _charYPosTable[facing] + _currentCharacter->y1;
	
	if (xpos >= 12 && xpos <= 308) {
		if (!lineIsPassable(xpos, ypos))
			return false;
	}
	
	if (_exitListPtr) {
		int16 *ptr = _exitListPtr;
		// this loop should be only entered on time, seems to be some hack in the original
		while (true) {
			if (*ptr == -1)
				break;
			
			if (*ptr > _currentCharacter->x1 || _currentCharacter->y1 < ptr[1] || _currentCharacter->x1 > ptr[2] || _currentCharacter->y1 > ptr[3]) {
				ptr += 10;
				break;
			}
			_brandonPosX = ptr[6];
			_brandonPosY = ptr[7];
			uint16 sceneId = ptr[5];
			facing = ptr[4];
			int unk1 = ptr[8];
			int unk2 = ptr[9];
			if (sceneId == 0xFFFF) {
				switch (facing) {
					case 0:
						sceneId = _roomTable[_currentCharacter->sceneId].northExit;
						break;
						
					case 2:
						sceneId = _roomTable[_currentCharacter->sceneId].eastExit;
						break;
					
					case 4:
						sceneId = _roomTable[_currentCharacter->sceneId].southExit;
						break;
						
					case 6:
						sceneId = _roomTable[_currentCharacter->sceneId].westExit;
						break;
						
					default:
						break;
				}
			}
			
			_currentCharacter->facing = facing;
			animRefreshNPC(0);
			_animator->updateAllObjectShapes();
			enterNewScene(sceneId, facing, unk1, unk2, 0);
			resetGameFlag(0xEE);
			return 1;
		}
	}
	
	int returnValue = 0;
	facing = 0;
	
	if ((_northExitHeight & 0xFF) + 2 >= ypos || (_northExitHeight & 0xFF) + 2 >= _currentCharacter->y1) {
		facing = 0;
		returnValue = 1;
	}
	
	if (xpos >= 308 || (_currentCharacter->x1 + 4) >= 308) {
		facing = 2;
		returnValue = 1;
	}
	
	if (((_northExitHeight >> 8) & 0xFF) - 2 < ypos || ((_northExitHeight >> 8) & 0xFF) - 2 < _currentCharacter->y1) {
		facing = 4;
		returnValue = 1;
	}
	
	if (xpos <= 12 || _currentCharacter->y1 <= 12) {
		facing = 6;
		returnValue = 1;
	}
	
	if (!returnValue)
		return 0;
	
	uint16 sceneId = 0xFFFF;
	switch (facing) {
		case 0:
			sceneId = _roomTable[_currentCharacter->sceneId].northExit;
			break;
		
		case 2:
			sceneId = _roomTable[_currentCharacter->sceneId].eastExit;
			break;
		
		case 4:
			sceneId = _roomTable[_currentCharacter->sceneId].southExit;
			break;
		
		default:
			sceneId = _roomTable[_currentCharacter->sceneId].westExit;
			break;
	}
	
	if (sceneId == 0xFFFF)
		return 0;

	enterNewScene(sceneId, facing, 1, 1, 0);
	return returnValue;
}

#pragma mark -
#pragma mark - Input
#pragma mark -

void KyraEngine::processInput(int xpos, int ypos) {
	debug(9, "KyraEngine::processInput(%d, %d)", xpos, ypos);
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
	debug(9, "KyraEngine::processInputHelper(%d, %d)", xpos, ypos);
	uint8 item = findItemAtPos(xpos, ypos);
	if (item != 0xFF) {
		if (_itemInHand == -1) {
			_screen->hideMouse();
			_animator->animRemoveGameItem(item);
			// XXX call kyraPlaySound(53)
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
	debug(9, "KyraEngine::clickEventHandler(%d, %d)", xpos, ypos);
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
	debug(9, "KyraEngine::hasClickedOnExit(%d, %d)", xpos, ypos);
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
	debug(9, "KyraEngine::clickEventHandler2()");
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
	debug(9, "KyraEngine::checkForNPCScriptRun(%d, %d)", xpos, ypos);
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
	debug(9, "KyraEngine::runNpcScript(%d)", func);
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
