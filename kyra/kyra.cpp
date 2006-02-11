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
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *md5sum;
	const char *checkFile;
	GameSettings toGameSettings() const {
		GameSettings dummy = { gameid, description, features };
		return dummy;
	}
};

// We could get rid of md5 detection at least for kyra 1 since we can locate all
// needed files for detecting the right language and version (Floppy, Talkie)
static const KyraGameSettings kyra_games[] = {
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ENGLISH | GF_FLOPPY, // english floppy 1.0 from Malice
										"3c244298395520bb62b5edfe41688879", "GEMCUT.EMC" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ENGLISH | GF_FLOPPY, 
										"796e44863dd22fa635b042df1bf16673", "GEMCUT.EMC" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_FRENCH | GF_FLOPPY,
										"abf8eb360e79a6c2a837751fbd4d3d24", "GEMCUT.EMC" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_GERMAN | GF_FLOPPY, 
										"6018e1dfeaca7fe83f8d0b00eb0dd049", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_GERMAN | GF_FLOPPY, // from Arne.F 
										"f0b276781f47c130f423ec9679fe9ed9", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_SPANISH | GF_FLOPPY, // from VooD
										"8909b41596913b3f5deaf3c9f1017b01", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_SPANISH | GF_FLOPPY, // floppy 1.8 from clemmy
										"747861d2a9c643c59fdab570df5b9093", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ENGLISH | GF_TALKIE, 
										"fac399fe62f98671e56a005c5e94e39f", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_GERMAN | GF_TALKIE, 
										"230f54e6afc007ab4117159181a1c722", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_FRENCH | GF_TALKIE, 
										"b037c41768b652a040360ffa3556fd2a", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia Demo",	GI_KYRA1, GF_DEMO | GF_ENGLISH,
										"fb722947d94897512b13b50cc84fd648", "DEMO1.WSA" },
	{ 0, 0, 0, 0, 0, 0 }
};

// Keep list of different supported games
struct KyraGameList {
	const char *gameid;
	const char *description;
	uint32 features;
	GameSettings toGameSettings() const {
		GameSettings dummy = { gameid, description, features };
		return dummy;
	}
};

static const KyraGameList kyra_list[] = {
	{ "kyra1", "The Legend of Kyrandia", 0 },
	{ 0, 0, 0 }
};

struct KyraLanguageTable {
	const char *file;
	uint32 language;
	Common::Language detLanguage;
};

static const KyraLanguageTable kyra_languages[] = {
	{ "MAIN15.CPS", GF_ENGLISH, Common::EN_USA },
	{ "MAIN_ENG.CPS", GF_ENGLISH, Common::EN_USA },
	{ "MAIN_FRE.CPS", GF_FRENCH, Common::FR_FRA },
	{ "MAIN_GER.CPS", GF_GERMAN, Common::DE_DEU },
	{ "MAIN_SPA.CPS", GF_SPANISH, Common::ES_ESP },
	{ 0, 0, Common::UNK_LANG }
};

static Common::Language convertKyraLang(uint32 features) {
	if (features & GF_ENGLISH) {
		return Common::EN_USA;
	} else if (features & GF_FRENCH) {
		return Common::FR_FRA;
	} else if (features & GF_GERMAN) {
		return Common::DE_DEU;
	} else if (features & GF_SPANISH) {
		return Common::ES_ESP;
	}
	return Common::UNK_LANG;
}

GameList Engine_KYRA_gameList() {
	GameList games;
	const KyraGameList *g = kyra_list;

	while (g->gameid) {
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

		for (g = kyra_games; g->gameid; g++) {
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
		for (g = kyra_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				detectedGames.push_back(DetectedGame(g->toGameSettings(), convertKyraLang(g->features), Common::kPlatformUnknown));
			}
		}
		if (detectedGames.isEmpty()) {
			debug("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const KyraGameList *g1 = kyra_list;
			while (g1->gameid) {
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

	for (g = kyra_games; g->gameid; g++) {
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
		_features = 0;
		_game = GI_KYRA1;
		Common::File test;
		if (test.open("INTRO.VRM")) {
			_features |= GF_TALKIE;
		} else {
			_features |= GF_FLOPPY;
		}
		
		// tries to detect the language
		const KyraLanguageTable *lang = kyra_languages;
		for (; lang->file; ++lang) {
			if (test.open(lang->file)) {
				_features |= lang->language;
				found = true;
				break;
			}
		}
		
		if (!found) {
			_features |= GF_LNGUNK;
		}
	}
}

int KyraEngine::init(GameDetector &detector) {
	_system->beginGFXTransaction();
		initCommonGFX(detector);
		//for debug reasons (see Screen::updateScreen)
		//_system->initSize(640, 200);
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

	_sound = new SoundPC(driver, _mixer, this);
	assert(_sound);
	static_cast<SoundPC*>(_sound)->hasNativeMT32(native_mt32);
	_sound->setVolume(255);
	
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
	_menuDirectlyToLoad = false;
	 
	 _lastMusicCommand = 0;

	_gameSpeed = 60;
	_tickLength = (uint8)(1000.0 / _gameSpeed);

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
		if (_skipIntroFlag &&_abortIntroFlag)
			resetGameFlag(0xEF);
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
	snd_setSoundEffectFile(1);
	enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);
	
	if (_abortIntroFlag && _skipFlag) {
		_menuDirectlyToLoad = true;
		_screen->setMouseCursor(1, 1, _shapes[4]);
		buttonMenuCallback(0);
		_menuDirectlyToLoad = false;
	} else
		saveGame(getSavegameFilename(0), "New game");
}

void KyraEngine::mainLoop() {
	debug(9, "KyraEngine::mainLoop()");

	while (!_quitFlag) {
		int32 frameTime = (int32)_system->getMillis();
		_skipFlag = false;

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
		
		_screen->showMouse();

		processButtonList(_buttonList);
		updateMousePointer();
		updateGameTimers();
		updateTextFade();

		_handleInput = true;
		delay((frameTime + _gameSpeed) - _system->getMillis(), true, true);
		_handleInput = false;
	}
}

void KyraEngine::quitGame() {
	res_unloadResources(RES_ALL);

	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		_movieObjects[i]->close();
		delete _movieObjects[i];
		_movieObjects[i] = 0;
	}

	_system->quit();
}

void KyraEngine::delay(uint32 amount, bool update, bool isMainLoop) {
	OSystem::Event event;
	char saveLoadSlot[20];
	char savegameName[14];

	_mousePressFlag = false;
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

		if (update)
			_sprites->updateSceneAnims();
			_animator->updateAllObjectShapes();

		if (_currentCharacter && _currentCharacter->sceneId == 210) {
			_animator->updateKyragemFading();
		}

		if (amount > 0 && !_skipFlag) {
			_system->delayMillis((amount > 10) ? 10 : amount);
		}
	} while (!_skipFlag && _system->getMillis() < start + amount);
	
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
	while (_system->getMillis() < nextTime && !_skipFlag) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();
		if (_currentCharacter->sceneId == 210) {
			_animator->updateKyragemFading();
			seq_playEnd();
		}
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
	snd_playSoundEffect(0x45);
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
			snd_playSoundEffect(12);
			snd_playSoundEffect(12);
			_finalC->_x = 16;
			_finalC->_y = 50;
			_finalC->_drawPage = 0;
			for (int i = 0; i < 18; ++i) {
				timer2 = _system->getMillis() + 4 * _tickLength;
				_finalC->displayFrame(i);
				_screen->updateScreen();
				while (_system->getMillis() < timer2) {}
			}
			snd_playWanderScoreViaMap(51, 1);
			delay(60*_tickLength);
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
					snd_playSoundEffect(0x0B);
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
						snd_playWanderScoreViaMap(52, 1);
						snd_playSoundEffect(0x0C);
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
						snd_playSoundEffect(0x0D);
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
