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

#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/voc.h"
#include "sound/audiostream.h"

#include "kyra/kyra.h"
#include "kyra/resource.h"
#include "kyra/screen.h"
#include "kyra/script.h"
#include "kyra/seqplayer.h"
#include "kyra/sound.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
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
	int midiDrv = MidiDriver::detectMusicDriver(MDT_NATIVE | MDT_ADLIB/* | MDT_PREFER_NATIVE*/);
	bool native_mt32 = (ConfMan.getBool("native_mt32") || (midiDrv == MD_MT32));

	MidiDriver *driver = MidiDriver::createMidi(midiDrv);
	if (!driver) {
		// In this case we should play the Adlib tracks, but for now
		// the automagic MIDI-to-Adlib conversion will do.
		driver = MidiDriver_ADLIB_create(_mixer);
	} else if (native_mt32) {
		driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
	}

	_midi = new MusicPlayer(driver, this);
	assert(_midi);
	_midi->hasNativeMT32(native_mt32);
	_midi->setVolume(255);
	
	_res = new Resource(this);
	assert(_res);
	_screen = new Screen(this, _system);
	assert(_screen);
	_sprites = new Sprites(this, _system);
	assert(_sprites);
	_seq = new SeqPlayer(this, _system);
	assert(_seq);
	
	_currentCharacter = 0;
	_characterList = new Character[11];
	assert(_characterList);
	for (int i = 0; i < 11; ++i) {
		memset(&_characterList[i], 0, sizeof(Character));
	}
	_characterList[0].sceneId = 5;
	_characterList[0].height = 48;
	_characterList[0].facing = 3;
	_characterList[0].currentAnimFrame = 7;
	
	_objectQueue = 0;
	_animStates = new AnimObject[31];
	assert(_animStates);
	memset(_animStates, 0, sizeof(AnimObject)*31);
	_charactersAnimState = &_animStates[0];
	_sprites->_animObjects =  &_animStates[5];
	_animItems = &_animStates[16];
	
	_scriptInterpreter = new ScriptHelper(this);
	assert(_scriptInterpreter);
	
	_npcScriptData = new ScriptData;
	memset(_npcScriptData, 0, sizeof(ScriptData));
	assert(_npcScriptData);
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
	memset(_wsaObjects, 0, sizeof(_wsaObjects));

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_fastMode = false;
	_talkCoords.y = 0x88;
	_talkCoords.x = 0;
	_talkCoords.w = 0;
	_talkMessageY = 0xC;
	_talkMessageH = 0;
	_talkMessagePrinted = false;
	_talkingCharNum = -1;
	_charSayUnk3 = -1;
	_mouseX = _mouseY = -1;
	memset(_currSentenceColor, 0, 3);
	_startSentencePalIndex = -1;
	_fadeText = false;

	_brandonPosX = _brandonPosY = -1;
	_brandonDrawFrame = 113;
	
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
	_mouseState = _itemInHand = -1;
	_handleInput = false;
	
	_currentRoom = 0xFFFF;
	_lastProcessedItem = 0;
	_lastProcessedItemHeight = 16;

	return 0;
}

KyraEngine::~KyraEngine() {
	delete _debugger;
	delete _sprites;
	delete _screen;
	delete _res;
	delete _midi;
	delete _seq;
	delete _scriptInterpreter;
	
	delete _npcScriptData;
	delete _scriptMain;
	
	delete _scriptClickData;
	delete _scriptClick;
	
	delete [] _animStates;
	delete [] _characterList;
	
	delete [] _movFacingTable;
	
	for (int i = 0; i < ARRAYSIZE(_shapes); ++i) {
		if (_shapes[i] != 0) {
			free(_shapes[i]);
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
	_shapes[0] = (byte*)malloc(_screen->getRectSize(3, 24));
	memset(_shapes[0], 0, _screen->getRectSize(3, 24));
	_shapes[1] = (byte*)malloc(_screen->getRectSize(4, 32));
	memset(_shapes[1], 0, _screen->getRectSize(4, 32));
	_shapes[2] = (byte*)malloc(_screen->getRectSize(8, 69));
	memset(_shapes[2], 0, _screen->getRectSize(8, 69));
	_shapes[3] = (byte*)malloc(_screen->getRectSize(8, 69));
	memset(_shapes[3], 0, _screen->getRectSize(8, 69));
	for (int i = 0; i < _roomTableSize; ++i) {
		for (int item = 0; item < 12; ++item) {
			_roomTable[i].itemsTable[item] = 0xFF;
			_roomTable[i].itemsXPos[item] = 0xFFFF;
			_roomTable[i].itemsYPos[item] = 0xFF;
			_roomTable[i].unkField3[item] = 0;
		}
	}
	loadCharacterShapes();
	loadSpecialEffectShapes();
	loadItems();
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

void KyraEngine::delay(uint32 amount) {
	OSystem::Event event;
	uint32 start = _system->getMillis();
	do {
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == 'q' || event.kbd.keycode == 27) {
					_quitFlag = true;
				} else if (event.kbd.keycode == 'd' && !_debugger->isAttached()) {
					_debugger->attach();
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
		updateAllObjectShapes();

		if (_currentCharacter->sceneId == 210) {
			//XXX
		}

		if (amount > 0) {
			_system->delayMillis((amount > 10) ? 10 : amount);
		}
	} while (_system->getMillis() < start + amount);
}

void KyraEngine::mainLoop() {
	debug(9, "KyraEngine::mainLoop()");

	while (!_quitFlag) {
		int32 frameTime = (int32)_system->getMillis();
		updateMousePointer();
		updateGameTimers();
		_sprites->updateSceneAnims();
		updateAllObjectShapes();
		updateTextFade();

		_handleInput = true;
		delay((frameTime + _gameSpeed) - _system->getMillis());
		_handleInput = false;
	}
}

void KyraEngine::quitGame() {
	res_unloadResources();

	for (int i = 0; i < 10; i++)
		wsa_close(_wsaObjects[i]);

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
	delete[] srcData;
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
	setTalkCoords(144);
	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		(this->*introProcTable[i])();
	}
	setTalkCoords(136);
	waitTicks(30);
	_seq->setCopyViewOffs(false);
	_midi->stopMusic();
	if (_features & GF_TALKIE) {
		_res->unloadPakFile("INTRO.VRM");
	}
	res_unloadResources(RES_INTRO);
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
	char vocFile[8];
	assert(id >= 0 && id < 1000);
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

void KyraEngine::loadMainScreen() {
	if ((_features & GF_ENGLISH) && (_features & GF_TALKIE)) 
		loadBitmap("MAIN_ENG.CPS", 3, 3, 0);
	else if(_features & GF_FRENCH)
		loadBitmap("MAIN_FRE.CPS", 3, 3, 0);
	else if(_features & GF_GERMAN)
		loadBitmap("MAIN_GER.CPS", 3, 3, 0);
	else if ((_features & GF_ENGLISH) && (_features & GF_FLOPPY))
		loadBitmap("MAIN15.CPS", 3, 3, 0);
	else if (_features & GF_SPANISH)
		loadBitmap("MAIN_SPA.CPS", 3, 3, 0);
	else
		warning("no main graphics file found");
	
	uint8 *_page3 = _screen->getPagePtr(3);
	uint8 *_page0 = _screen->getPagePtr(0);
	memcpy(_page0, _page3, 320*200);
}

void KyraEngine::setCharactersInDefaultScene() {
	static uint32 defaultSceneTable[][4] = {
		{ 0xFFFF, 0x0004, 0x0003, 0xFFFF },
		{ 0xFFFF, 0x0022, 0xFFFF, 0x0000 },
		{ 0xFFFF, 0x001D, 0x0021, 0xFFFF },
		{ 0xFFFF, 0x0000, 0x0000, 0xFFFF }
	};
	
	for (int i = 1; i < 5; ++i) {
		Character *cur = &_characterList[i];
		cur->field_20 = 0;
		uint32 *curTable = defaultSceneTable[i-1];
		cur->sceneId = curTable[0];
		if (cur->sceneId == _currentCharacter->sceneId) {
			++cur->field_20;
			cur->sceneId = curTable[cur->field_20];
		}
		cur->field_23 = curTable[cur->field_20+1];
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
	edit->unk6 = 1;
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
	debug(9, "enterNewScene(%d, %d, %d, %d, %d)", sceneId, facing, unk1, unk2, brandonAlive);
	int unkVar1 = 1;
	_screen->hideMouse();
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
	
	for (int i = 0; i < 10; ++i) {
		wsa_close(_wsaObjects[i]);
		_wsaObjects[i] = 0;
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
		// XXX
	}

	startSceneScript(brandonAlive);
	setupSceneItems();
	
	initSceneData(facing, unk2, brandonAlive);
	
	_loopFlag2 = 0;
	_screen->showMouse();
	if (!brandonAlive) {
		// XXX seq_poisionDeathNow
	}
}

void KyraEngine::moveCharacterToPos(int character, int facing, int xpos, int ypos) {
	debug(9, "moveCharacterToPos(%d, %d, %d, %d)", character, facing, xpos, ypos);
	Character *ch = &_characterList[character];
	ch->facing = facing;
	_screen->hideMouse();
	xpos &= 0xFFFC;
	ypos &= 0xFFFE;
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
	debug(9, "setCharacterPositionWithUpdate(%d)", character);
	setCharacterPosition(character, 0);
	_sprites->updateSceneAnims();
	updateGameTimers();
	updateAllObjectShapes();
	updateTextFade();

	if (_currentCharacter->sceneId == 210) {
		// XXX game_updateKyragemFading
	}
}

int KyraEngine::setCharacterPosition(int character, int *facingTable) {
	debug(9, "setCharacterPosition(%d, 0x%X)", character, facingTable);
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
	debug(9, "setCharacterPositionHelper(%d, 0x%X)", character, facingTable);
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
	debug(9, "getOppositeFacingDirection(%d)", dir);
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
	debug(9, "blockInRegion(%d, %d, %d, %d)", x, y, width, height);
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
	debug(9, "blockOutRegion(%d, %d, %d, %d)", x, y, width, height);
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
	debug(9, "startSceneScript(%d)", brandonAlive);
	assert(_currentCharacter->sceneId < _roomTableSize);
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".CPS");
	loadBitmap(fileNameBuffer, 3, 3, 0);
	_sprites->loadSceneShapes();
	_exitListPtr = 0;

	_screen->fillRect(7, 7, 312, 136, 0, 0);
	_screen->setScreenPalette(_screen->_currentPalette);
	_screen->copyRegion(7, 7, 7, 7, 305, 129, 3, 0);
	_screen->updateScreen();

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
	debug(9, "initSceneData(%d, %d, %d)", facing, unk1, brandonAlive);
	
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
		
		if ((uint8)(_northExitHeight & 0xFF) >= ypos) {
			ypos = (_northExitHeight & 0xFF);
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
				facing = 4;
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
	
	xpos2 &= 0xFFFC;
	ypos2 &= 0xFFFE;
	xpos &= 0xFFFC;
	ypos &= 0xFFFE;
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
		_unkBrandonPoisonFlags[i] = i;
	}
}

void KyraEngine::initAnimStateList() {
	_animStates[0].index = 0;
	_animStates[0].active = 1;
	_animStates[0].flags = 0x800;
	_animStates[0].background = _shapes[2];
	_animStates[0].rectSize = _screen->getRectSize(4, 48);
	_animStates[0].width = 4;
	_animStates[0].height = 48;
	_animStates[0].width2 = 4;
	_animStates[0].height2 = 3;
	
	for (int i = 1; i <= 4; ++i) {
		_animStates[i].index = i;
		_animStates[i].active = 0;
		_animStates[i].flags = 0x800;
		_animStates[i].background = _shapes[3];
		_animStates[i].rectSize = _screen->getRectSize(4, 64);
		_animStates[i].width = 4;
		_animStates[i].height = 48;
		_animStates[i].width2 = 4;
		_animStates[i].height2 = 3;
	}
	
	for (int i = 5; i < 16; ++i) {
		_animStates[i].index = i;
		_animStates[i].active = 0;
		_animStates[i].flags = 0;
	}
	
	for (int i = 16; i < 28; ++i) {
		_animStates[i].index = i;
		_animStates[i].flags = 0;
		_animStates[i].background = _shapes[349+i];
		_animStates[i].rectSize = _screen->getRectSize(3, 24);
		_animStates[i].width = 3;
		_animStates[i].height = 16;
		_animStates[i].width2 = 0;
		_animStates[i].height2 = 0;
	}
}

void KyraEngine::initSceneObjectList(int brandonAlive) {
	debug(9, "initSceneObjectList(%d)", brandonAlive);
	for (int i = 0; i < 31; ++i) {
		_charactersAnimState[i].active = 0;
	}
	
	int startAnimFrame = 0;
	
	AnimObject *curAnimState = _charactersAnimState;
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
	_objectQueue = 0;
	_objectQueue = objectAddHead(0, curAnimState);
	
	int listAdded = 0;
	int addedObjects = 1;
	
	for (int i = 1; i < 5; ++i) {
		Character *ch = &_characterList[i];
		if (ch->sceneId != _currentCharacter->sceneId) {
			++addedObjects;
			continue;
		}
		
		curAnimState = &_charactersAnimState[addedObjects];
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
		
		_objectQueue = objectQueue(_objectQueue, curAnimState);
		
		++addedObjects;
		++listAdded;
		if (listAdded < 2)
			i = 5;
	}
	
	for (int i = 0; i < 11; ++i) {
		curAnimState = &_sprites->_animObjects[i];

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
		
		_objectQueue = objectQueue(_objectQueue, curAnimState);

	}
	
	for (int i = 0; i < 12; ++i) {
		curAnimState = &_animItems[i];
		Room *curRoom = &_roomTable[_currentCharacter->sceneId];
		byte curItem = curRoom->itemsTable[i];
		if (curItem != 0xFF) {
			curAnimState->drawY = curRoom->itemsYPos[i];
			curAnimState->sceneAnimPtr = _shapes[220+i];
			curAnimState->animFrameNumber = 0xFFFF;
			curAnimState->y1 = curRoom->itemsYPos[i];
			curAnimState->x1 = curRoom->itemsXPos[i];
			
			curAnimState->x1 -= (fetchAnimWidth(curAnimState->sceneAnimPtr, _scaleTable[curAnimState->drawY])) >> 1;
			curAnimState->y1 -= fetchAnimHeight(curAnimState->sceneAnimPtr, _scaleTable[curAnimState->drawY]);
			
			curAnimState->x2 = curAnimState->x1;
			curAnimState->y2 = curAnimState->y1;
			
			curAnimState->active = 1;
			curAnimState->refreshFlag = 1;
			curAnimState->bkgdChangeFlag = 1;
			
			_objectQueue = objectQueue(_objectQueue, curAnimState);
		} else {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
		}
	}
	
	preserveAnyChangedBackgrounds();
	curAnimState = _charactersAnimState;
	curAnimState->bkgdChangeFlag = 1;
	curAnimState->refreshFlag = 1;
	for (int i = 0; i < 28; ++i) {
		curAnimState = &_charactersAnimState[i];
		if (curAnimState->active) {
			curAnimState->bkgdChangeFlag = 1;
			curAnimState->refreshFlag = 1;
		}
	}
	restoreAllObjectBackgrounds();
	preserveAnyChangedBackgrounds();
	prepDrawAllObjects();
	_screen->hideMouse();
	initSceneScreen(brandonAlive);
	_screen->showMouse();
	copyChangedObjectsForward(0);
}

void KyraEngine::initSceneScreen(int brandonAlive) {
	// XXX (Pointless?) Palette stuff
	//_screen->shuffleScreen(8, 8, 0x130, 0x80, 2, 0, byte_2EE1C);
	_screen->copyRegion(1, 8, 1, 8, 304, 0x80, 2, 0);
	// XXX More (pointless?) palette stuff

	if (!_scriptInterpreter->startScript(_scriptClick, 2))
		error("Could not start script function 2 of scene script");

	_scriptClick->variables[7] = brandonAlive;

	while (_scriptInterpreter->validScript(_scriptClick))
		_scriptInterpreter->runScript(_scriptClick);

	setTextFadeTimerCountdown(-1);
	if (_currentCharacter->sceneId == 0xD2) {
		// XXX
	}
}

#pragma mark -
#pragma mark - Text handling
#pragma mark -

void KyraEngine::setTalkCoords(uint16 y) {
	debug(9, "KyraEngine::setTalkCoords(%d)", y);
	_talkCoords.y = y;
}

int KyraEngine::getCenterStringX(const char *str, int x1, int x2) {
	debug(9, "KyraEngine::getCenterStringX('%s', %d, %d)", str, x1, x2);
	_screen->_charWidth = -2;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	int strWidth = _screen->getTextWidth(str);
	_screen->setFont(curFont);
	_screen->_charWidth = 0;
	int w = x2 - x1 + 1;
	return x1 + (w - strWidth) / 2;
}

int KyraEngine::getCharLength(const char *str, int len) {
	debug(9, "KyraEngine::getCharLength('%s', %d)", str, len);
	int charsCount = 0;
	if (*str) {
		_screen->_charWidth = -2;
		Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
		int i = 0;
		while (i <= len && *str) {
			i += _screen->getCharWidth(*str++);
			++charsCount;
		}
		_screen->setFont(curFont);
		_screen->_charWidth = 0;
	}
	return charsCount;
}

int KyraEngine::dropCRIntoString(char *str, int offs) {
	debug(9, "KyraEngine::dropCRIntoString('%s', %d)", str, offs);
	int pos = 0;
	str += offs;
	while (*str) {
		if (*str == ' ') {
			*str = '\r';
			return pos;
		}
		++str;
		++pos;
	}
	return 0;
}

char *KyraEngine::preprocessString(const char *str) {
	debug(9, "KyraEngine::preprocessString('%s')", str);
	assert(strlen(str) < sizeof(_talkBuffer) - 1);
	strcpy(_talkBuffer, str);
	char *p = _talkBuffer;
	while (*p) {
		if (*p == '\r') {
			return _talkBuffer;
		}
		++p;
	}
	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	int textWidth = _screen->getTextWidth(p);
	_screen->_charWidth = 0;
	if (textWidth > 176) {
		if (textWidth > 352) {
			int count = getCharLength(p, textWidth / 3);
			int offs = dropCRIntoString(p, count);
			p += count + offs;
			_screen->_charWidth = -2;
			textWidth = _screen->getTextWidth(p);
			_screen->_charWidth = 0;
			count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		} else {
			int count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		}
	}
	_screen->setFont(curFont);
	return _talkBuffer;
}

int KyraEngine::buildMessageSubstrings(const char *str) {
	debug(9, "KyraEngine::buildMessageSubstrings('%s')", str);
	int currentLine = 0;
	int pos = 0;
	while (*str) {
		if (*str == '\r') {
			assert(currentLine < TALK_SUBSTRING_NUM);
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
			++currentLine;
			pos = 0;
		} else {
			_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = *str;
			++pos;
			if (pos > TALK_SUBSTRING_LEN - 2) {
				pos = TALK_SUBSTRING_LEN - 2;
			}
		}
		++str;
	}
	_talkSubstrings[currentLine * TALK_SUBSTRING_LEN + pos] = '\0';
	return currentLine + 1;
}

int KyraEngine::getWidestLineWidth(int linesCount) {
	debug(9, "KyraEngine::getWidestLineWidth(%d)", linesCount);
	int maxWidth = 0;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	for (int l = 0; l < linesCount; ++l) {
		int w = _screen->getTextWidth(&_talkSubstrings[l * TALK_SUBSTRING_LEN]);
		if (maxWidth < w) {
			maxWidth = w;
		}
	}
	_screen->setFont(curFont);
	_screen->_charWidth = 0;
	return maxWidth;
}

void KyraEngine::calcWidestLineBounds(int &x1, int &x2, int w, int cx) {
	debug(9, "KyraEngine::calcWidestLineBounds(%d, %d)", w, cx);
	x1 = cx - w / 2;
	if (x1 + w >= Screen::SCREEN_W - 12) {
		x1 = Screen::SCREEN_W - 12 - w - 1;
	} else if (x1 < 12) {
		x1 = 12;
	}
	x2 = x1 + w + 1;
}

void KyraEngine::restoreTalkTextMessageBkgd(int srcPage, int dstPage) {
	debug(9, "KyraEngine::restoreTalkTextMessageBkgd(%d, %d)", srcPage, dstPage);
	if (_talkMessagePrinted) {
		_talkMessagePrinted = false;
		_screen->copyRegion(_talkCoords.x, _talkCoords.y, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, srcPage, dstPage);
	}
}

void KyraEngine::printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage) {
	debug(9, "KyraEngine::printTalkTextMessage('%s', %d, %d, %d, %d, %d)", text, x, y, color, srcPage, dstPage);
	char *str = preprocessString(text);
	int lineCount = buildMessageSubstrings(str);
	int top = y - lineCount * 10;
	if (top < 0) {
		top = 0;
	}
	_talkMessageY = top;
	_talkMessageH = lineCount * 10;
	int w = getWidestLineWidth(lineCount);
	int x1, x2;
	calcWidestLineBounds(x1, x2, w, x);
	_talkCoords.x = x1;
	_talkCoords.w = w + 2;
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkCoords.y, _talkCoords.w, _talkMessageH, srcPage, dstPage);
	int curPage = _screen->_curPage;
	_screen->_curPage = srcPage;
	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		char *msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		int left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, 0);
	}
	_screen->_curPage = curPage;
	_talkMessagePrinted = true;
}

void KyraEngine::printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2) {
	uint8 colorMap[] = { 0, 15, 12, 12 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	_screen->printText(str, x, y, c0, c2);
	_screen->_charWidth = 0;
	_screen->setFont(curFont);
}

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
		/*
		if (_currentCharacter.sceneId == 0xD2)
			if (seq_playEnd())
				break;
		*/

		if (_system->getMillis() > timeToEnd && !hasUpdatedNPCs) {
			hasUpdatedNPCs = true;
			disableTimer(15);
			_currHeadShape = 4;
			animRefreshNPC(0);
			animRefreshNPC(_talkingCharNum);

			if (_charSayUnk2 != -1) {
				_sprites->_animObjects[_charSayUnk2].active = 0;
				_sprites->_anims[_charSayUnk2].play = false;
				_charSayUnk2 = -1;
			}
		}

		updateGameTimers();
		_sprites->updateSceneAnims();
		restoreAllObjectBackgrounds();
		preserveAnyChangedBackgrounds();
		prepDrawAllObjects();

		currPage = _screen->_curPage;
		_screen->_curPage = 2;
		printCharacterText(chatStr, charNum);
		_screen->_curPage = currPage;

		copyChangedObjectsForward(0);
		updateTextFade();

		if ((chatDuration < (int16)(_system->getMillis() - timeAtStart)) && chatDuration != -1)
			break;

		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode == 0x20 || event.kbd.keycode == 0xC6)
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
	}

	if (convoInitialized != 0) {
		_talkingCharNum = -1;
		_currentCharacter->currentAnimFrame = 7;
		animRefreshNPC(0);
		updateAllObjectShapes();
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
	updateAllObjectShapes();
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
	updateAllObjectShapes();
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
		updateAllObjectShapes();
	}

	_charSayUnk2 = -1;
	flagAllObjectsForBkgdChange();
	restoreAllObjectBackgrounds();

	if (charNum > 4 && charNum < 11) {
		// TODO: Fill in weird _game_inventory stuff here
	}

	flagAllObjectsForRefresh();
	flagAllObjectsForBkgdChange();
	preserveAnyChangedBackgrounds();
	_charSayUnk3 = charNum;

	return 1;
}

void KyraEngine::printCharacterText(char *text, int8 charNum) {
	uint8 colorTable[] = {0x0F, 0x9, 0x0C9, 0x80, 0x5, 0x81, 0x0E, 0xD8, 0x55, 0x3A, 0x3a};
	int top, left, x1, x2, w, x;
	char *msg;

	uint8 color = colorTable[charNum];
	text = preprocessString(text);
	int lineCount = buildMessageSubstrings(text);
	w = getWidestLineWidth(lineCount);
	x = _characterList[charNum].x1;
	calcWidestLineBounds(x1, x2, w, x);

	for (int i = 0; i < lineCount; ++i) {
		top = i * 10 + _talkMessageY;
		msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
		left = getCenterStringX(msg, x1, x2);
		printText(msg, left, top, color, 0xC, 0);
	}
}

void KyraEngine::characterSays(char *chatStr, int8 charNum, int8 chatDuration) {
	debug(9, "KyraEngine:::characterSays('%s', %i, %d)", chatStr, charNum, chatDuration);
	uint8 startAnimFrames[] =  { 0x10, 0x32, 0x56, 0x0, 0x0, 0x0 };

	uint16 chatTicks;
	int16 convoInitialized;
	int8 chatPartnerNum;

	if (_currentCharacter->sceneId == 0xD2)
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

	char *processedString = preprocessString(chatStr);
	int lineNum = buildMessageSubstrings(processedString);

	int16 yPos = _characterList[charNum].y1;
	yPos -= _scaleTable[charNum] * _characterList[charNum].height;
	yPos -= 8;
	yPos -= lineNum * 10;

	if (yPos < 11)
		yPos = 11;

	if (yPos > 100 )
		yPos = 100;

	_talkMessageY = yPos;
	_talkMessageH = lineNum * 10;
	restoreAllObjectBackgrounds();

	_screen->copyRegion(1, _talkMessageY, 1, 136, 319, _talkMessageH, 2, 2);
	_screen->hideMouse();

	printCharacterText(processedString, charNum);
	_screen->showMouse();

	if (chatDuration == -2)
		chatTicks = strlen(processedString) * 9;
	else
		chatTicks = chatDuration;

	waitForChatToFinish(chatTicks, chatStr, charNum);

	restoreAllObjectBackgrounds();

	_screen->copyRegion(1, 136, 1, _talkMessageY, 319, _talkMessageH, 2, 2);
	preserveAllBackgrounds();
	prepDrawAllObjects();
	_screen->hideMouse();

	_screen->copyRegion(1, _talkMessageY, 1, _talkMessageY, 319, _talkMessageH, 2, 0);
	_screen->showMouse();
	flagAllObjectsForRefresh();
	copyChangedObjectsForward(0);

	if (chatPartnerNum != -1 && chatPartnerNum < 5)
		restoreChatPartnerAnimFrame(chatPartnerNum);

	endCharacterChat(charNum, convoInitialized);
}

void KyraEngine::drawSentenceCommand(char *sentence, int color) {
	debug(9, "drawSentenceCommand('%s', %i)", sentence, color);
	_screen->hideMouse();
	_screen->fillRect(8, 143, 311, 152, 12);

	if (_startSentencePalIndex != color || _fadeText != false) {
		_currSentenceColor[0] = _screen->_currentPalette[765] = _screen->_currentPalette[color*3];
		_currSentenceColor[1] = _screen->_currentPalette[766] = _screen->_currentPalette[color*3+1];
		_currSentenceColor[2] = _screen->_currentPalette[767] = _screen->_currentPalette[color*3+2];
	
		_screen->setScreenPalette(_screen->_currentPalette);
		_startSentencePalIndex = 0;
	}

	printText(sentence, 8, 143, 0xFF, 12, 0);
	_screen->showMouse();
	setTextFadeTimerCountdown(15);
	_fadeText = false;
}

void KyraEngine::updateSentenceCommand(char *str1, char *str2, int color) {
	debug(9, "updateSentenceCommand('%s', '%s', %i)", str1, str2, color);
	char sentenceCommand[500];
	strncpy(sentenceCommand, str1, 500);
	if (str2)
		strncat(sentenceCommand, str2, 500 - strlen(sentenceCommand));

	drawSentenceCommand(sentenceCommand, color);
}

void KyraEngine::updateTextFade() {
	debug(9, "updateTextFade()");
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
	debug(9, "addToNoDropRects(%d, %d, %d, %d)", x, y, w, h);
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
	debug(9, "clearNoDropRects()");
	memset(_noDropRects, -1, sizeof(_noDropRects));
}

byte KyraEngine::findFreeItemInScene(int scene) {
	debug(9, "findFreeItemInScene(%d)", scene);
	assert(scene < _roomTableSize);
	Room *room = &_roomTable[scene];
	for (int i = 0; i < 12; ++i) {
		if (room->itemsTable[i] == 0xFF)
			return i;
	}
	return 0xFF;
}

byte KyraEngine::findItemAtPos(int x, int y) {
	debug(9, "findItemAtPos(%d, %d)", x, y);
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
	debug(9, "placeItemInGenericMapScene(%d, %d)", item, index);
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
	debug(9, "createMouseItem(%d)", item);
	_screen->hideMouse();
	setMouseItem(item);
	_itemInHand = item;
	_screen->showMouse();
}

void KyraEngine::destroyMouseItem() {
	debug(9, "destroyMouseItem()");
	_screen->hideMouse();
	_screen->setMouseCursor(1, 1, _shapes[4]);
	_itemInHand = -1;
	_screen->showMouse();
}

void KyraEngine::setMouseItem(int item) {
	debug(9, "setMouseItem(%d)", item);
	if (item == -1) {
		_screen->setMouseCursor(1, 1, _shapes[10]);
	} else {
		_screen->setMouseCursor(8, 15, _shapes[220+item]);
	}
}

void KyraEngine::wipeDownMouseItem(int xpos, int ypos) {
	debug(9, "wipeDownMouseItem(%d, %d)", xpos, ypos);
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
		_screen->drawShape(0, _shapes[220+_itemInHand], xpos, y, 0, 0);
		_screen->updateScreen();
		y += 2;
		height -= 2;
		// XXX
		waitTicks(1);
	}	
	restoreRect1(xpos, ypos);
	_screen->resetShapeHeight(_shapes[220+_itemInHand]);
	destroyMouseItem();
	_screen->showMouse();
}

void KyraEngine::setupSceneItems() {
	debug(9, "setupSceneItems()");
	uint16 sceneId = _currentCharacter->sceneId;
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	for (int i = 0; i < 12; ++i) {
		uint8 item = currentRoom->itemsTable[i];
		if (item == 0xFF || !currentRoom->unkField3[i]) {
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
				currentRoom->unkField3[i] = 0;
			}
		}
	}
}

int KyraEngine::countItemsInScene(uint16 sceneId) {
	debug(9, "countItemsInScene(%d)", sceneId);
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
	debug(9, "processItemDrop(%d, %d, %d, %d, %d, %d)", sceneId, item, x, y, unk1, unk2);
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
		warning("processItemDrop unk1 == 2 is NOT implemented");
		// XXX
	}
	
	if (unk1 == 0) {
		destroyMouseItem();
	}
	
	itemDropDown(x, y, destX, destY, freeItem, item);
	
	if (unk1 == 0 && unk2 != 0) {
		// XXX updateSentenceCommand
	}
	
	return 1;
}

void KyraEngine::exchangeItemWithMouseItem(uint16 sceneId, int itemIndex) {
	debug(9, "exchangeItemWithMouseItem(%d, %d)", sceneId, itemIndex);
	_screen->hideMouse();
	animRemoveGameItem(itemIndex);
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	
	int item = currentRoom->itemsTable[itemIndex];
	currentRoom->itemsTable[itemIndex] = _itemInHand;
	_itemInHand = item;
	animAddGameItem(itemIndex, sceneId);
	// XXX snd_kyraPlaySound 53
	
	setMouseItem(_itemInHand);
	// XXX	
	_screen->showMouse();
	clickEventHandler2();
}

void KyraEngine::addItemToRoom(uint16 sceneId, uint8 item, int itemIndex, int x, int y) {
	debug(9, "addItemToRoom(%d, %d, %d, %d, %d)", sceneId, item, itemIndex, x, y);
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	currentRoom->itemsTable[itemIndex] = item;
	currentRoom->itemsXPos[itemIndex] = x;
	currentRoom->itemsYPos[itemIndex] = y;
	currentRoom->unkField3[itemIndex] = 1;
}

int KyraEngine::checkNoDropRects(int x, int y) {
	debug(9, "checkNoDropRects(%d, %d)", x, y);
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
	debug(9, "isDropable(%d, %d)", x, y);
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
	debug(9, "itemDropDown(%d, %d, %d, %d, %d, %d)", x, y, destX, destY, freeItem, item);
	assert(_currentCharacter->sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[_currentCharacter->sceneId];
	if (x == destX && y == destY) {
		currentRoom->itemsXPos[freeItem] = destX;
		currentRoom->itemsYPos[freeItem] = destY;
		currentRoom->itemsTable[freeItem] = item;
		// call kyraPlaySound(0x32)
		animAddGameItem(freeItem, _currentCharacter->sceneId);
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
			_screen->drawShape(0, _shapes[220+item], drawX, drawY, 0, 0);
			delay(1);
			_screen->updateScreen();
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
				_screen->drawShape(0, _shapes[220+item], drawX, drawY, 0, 0);
				delay(1);
				_screen->updateScreen();
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
	animAddGameItem(freeItem, _currentCharacter->sceneId);
	_screen->showMouse();
}

void KyraEngine::dropItem(int unk1, int item, int x, int y, int unk2) {
	debug(9, "dropItem(%d, %d, %d, %d, %d)", unk1, item, x, y, unk2);
	if (processItemDrop(_currentCharacter->sceneId, item, x, y, unk1, unk2))
		return;
	// call kyraPlaySound(54)
	if (12 == countItemsInScene(_currentCharacter->sceneId)) {
		// XXX drawSentenceCommand
	} else {
		// XXX drawSentenceCommand
	}
}

#pragma mark -
#pragma mark - Animation specific code
#pragma mark -

void KyraEngine::preserveAllBackgrounds() {
	uint8 currPage = _screen->_curPage;
	_screen->_curPage = 2;

	AnimObject *curObject = _objectQueue;
	while (curObject) {
		if (curObject->active && !curObject->unk1) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	_screen->_curPage = currPage;
}

void KyraEngine::flagAllObjectsForBkgdChange() {
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->bkgdChangeFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void KyraEngine::flagAllObjectsForRefresh() {
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->refreshFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void KyraEngine::restoreAllObjectBackgrounds() {
	debug(9, "restoreAllObjectBackground()");
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;
	
	while (curObject) {
		// XXX
		if (curObject->active) {
			preserveOrRestoreBackground(curObject, true);
			curObject->x2 = curObject->x1;
			curObject->y2 = curObject->y1;
		}
		curObject = curObject->nextAnimObject;
	}
	
	_screen->_curPage = 0;
}

void KyraEngine::preserveAnyChangedBackgrounds() {
	debug(9, "preserveAnyChangedBackgrounds()");
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;
	
	while (curObject) {
		// XXX
		if (curObject->active && curObject->bkgdChangeFlag) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	
	_screen->_curPage = 0;
}

void KyraEngine::preserveOrRestoreBackground(AnimObject *obj, bool restore) {
	debug(9, "preserveOrRestoreBackground(0x%X, restore)", obj, restore);
	int x = 0, y = 0, width = obj->width << 3, height = obj->height;
	
	if (restore) {
		x = obj->x2;
		y = obj->y2;
	} else {
		x = obj->x1;
		y = obj->y1;
	}
	
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	
	int temp;
	
	temp = x + width;
	if (temp >= 319) {
		x = 319 - width;
	}
	temp = y + height;
	if (temp >= 136) {
		y = 136 - height;
	}
	
	if (restore) {
		_screen->copyBlockToPage(_screen->_curPage, x, y, width, height, obj->background);
	} else {
		_screen->copyRegionToBuffer(_screen->_curPage, x, y, width, height, obj->background);
	}
}

void KyraEngine::prepDrawAllObjects() {
	debug(9, "prepDrawAllObjects()");
	AnimObject *curObject = _objectQueue;
	int drawPage = 2;
	int flagUnk1 = 0, flagUnk2 = 0, flagUnk3 = 0;
	// XXX
	if (_brandonStatusBit & 0x20)
		flagUnk1 = 0x200;
	if (_brandonStatusBit & 0x40)
		flagUnk2 = 0x4000;

	while (curObject) {
		if (curObject->active) {
			int xpos = curObject->x1;
			int ypos = curObject->y1;
			
			int drawLayer = 0;
			if (!(curObject->flags & 0x800)) {
				drawLayer = 7;
			} else if (curObject->unk1) {
				drawLayer = 0;
			} else {
				drawLayer = _sprites->getDrawLayer(curObject->drawY);
			}
			
			// talking head functionallity
			if (_talkingCharNum != -1) {
				const int16 baseAnimFrameTable1[] = { 0x11, 0x35, 0x59, 0x00, 0x00, 0x00 };
				const int16 baseAnimFrameTable2[] = { 0x15, 0x39, 0x5D, 0x00, 0x00, 0x00 };
				const int8 xOffsetTable1[] = { 2, 4, 0, 5, 2, 0, 0, 0 };
				const int8 xOffsetTable2[] = { 6, 4, 8, 3, 6, 0, 0, 0 };
				const int8 yOffsetTable1[] = { 0, 8, 1, 1, 0, 0, 0, 0 };
				const int8 yOffsetTable2[] = { 0, 8, 1, 1, 0, 0, 0, 0 };
				if (curObject->index == 0 || curObject->index <= 4) {
					int shapesIndex = 0;
					if (curObject->index == _charSayUnk3) {
						shapesIndex = _currHeadShape + baseAnimFrameTable1[curObject->index];
					} else {
						shapesIndex = baseAnimFrameTable2[curObject->index];
						int temp2 = 0;
						if (curObject->index == 2) {
							if (_characterList[2].sceneId == 0x4D || _characterList[2].sceneId == 0x56) {
								temp2 = 1;
							} else {
								temp2 = 0;
							}
						} else {
							temp2 = 1;
						}
						
						if (!temp2) {
							shapesIndex = -1;
						}
					}
						
					xpos = curObject->x1;
					ypos = curObject->y1;
						
					int tempX = 0, tempY = 0;
					if (curObject->flags & 0x1) {
						tempX = (xOffsetTable1[curObject->index] * _brandonScaleX) >> 8;
						tempY = yOffsetTable1[curObject->index];
					} else {
						tempX = (xOffsetTable2[curObject->index] * _brandonScaleX) >> 8;
						tempY = yOffsetTable2[curObject->index];
					}
					tempY = (tempY * _brandonScaleY) >> 8;
					xpos += tempX;
					ypos += tempY;
					
					if (_scaleMode && _brandonScaleX != 256) {
						++xpos;
					}
					
					if (curObject->index == 0) {
						if (!(_brandonStatusBit & 2)) {
							flagUnk3 = 0x100;
							if ((flagUnk1 & 0x200) || (flagUnk2 & 0x4000)) {
								flagUnk3 = 0;
							}
							
							int tempFlags = 0;
							if (flagUnk3 & 0x100) {
								if (curObject->flags & 1) {
									tempFlags = 1;
								}
								tempFlags |= 0x800 | flagUnk1 | 0x100;
							}
							
							if (!(flagUnk3 & 0x100) && (flagUnk2 & 0x4000)) {
								tempFlags = 0;
								if (curObject->flags & 1) {
									tempFlags = 1;
								}
								tempFlags |= 0x900 | flagUnk1 | 0x4000;
								_screen->drawShape(drawPage, _shapes[4+shapesIndex], xpos, ypos, 2, tempFlags | 4, _unkBrandonPoisonFlags, 1, 0/*XXX*/, drawLayer, _brandonScaleX, _brandonScaleY);
							} else {
								if (!(flagUnk2 & 0x4000)) {
									tempFlags = 0;
									if (curObject->flags & 1) {
										tempFlags = 1;
									}
									tempFlags |= 0x900 | flagUnk1;
								}
								
								_screen->drawShape(drawPage, _shapes[4+shapesIndex], xpos, ypos, 2, tempFlags | 4, _unkBrandonPoisonFlags, 1, drawLayer, _brandonScaleX, _brandonScaleY);
							}
						}
					} else {
						if (shapesIndex != -1) {
							int tempFlags = 0;
							if (curObject->flags & 1) {
								tempFlags = 1;
							}
							_screen->drawShape(drawPage, _shapes[4+shapesIndex], xpos, ypos, 2, tempFlags | 0x800, drawLayer); 							
						}
					}
				}
			}
			
			xpos = curObject->x1;
			ypos = curObject->y1;
			
			curObject->flags |= 0x800;
			if (curObject->index == 0) {
				flagUnk3 = 0x100;
				
				if (flagUnk1 & 0x200 || flagUnk2 & 0x4000) {
					flagUnk3 = 0;
				}
				
				if (_brandonStatusBit & 2) {
					curObject->flags &= 0xFFFFFFFE;
				}
				
				if (!_scaleMode) {
					if (flagUnk3 & 0x100) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x100, (uint8*)_unkBrandonPoisonFlags, 1, drawLayer);
					} else if (flagUnk3 & 0x4000) {
						// XXX
						int hackVar = 0;
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4000, hackVar, drawLayer);
					} else {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1, drawLayer);
					}
				} else {
					if (flagUnk3 & 0x100) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x104, (uint8*)_unkBrandonPoisonFlags, 1, drawLayer, _brandonScaleX, _brandonScaleY);
					} else if (flagUnk3 & 0x4000) {
						// XXX
						int hackVar = 0;
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4004, 0, drawLayer, hackVar, _brandonScaleX, _brandonScaleY);
					} else {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4, drawLayer, _brandonScaleX, _brandonScaleY);
					}
				}
			} else {
				if (curObject->index >= 16 && curObject->index <= 27) {
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | 4, drawLayer, (int)_scaleTable[curObject->drawY], (int)_scaleTable[curObject->drawY]);
				} else {
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags, drawLayer);
				}
			}
		}
		curObject = curObject->nextAnimObject;
	}
}

void KyraEngine::copyChangedObjectsForward(int refreshFlag) {
	debug(9, "copyChangedObjectsForward(%d)", refreshFlag);
	AnimObject *curObject = _objectQueue;
	bool changed = false;

	while (curObject) {
		if (curObject->active) {
			if (curObject->refreshFlag || refreshFlag) {
				int xpos = 0, ypos = 0, width = 0, height = 0;
				xpos = curObject->x1 - (curObject->width2+1);
				ypos = curObject->y1 - curObject->height2;				
				width = (curObject->width + curObject->width2*2)<<3;
				height = curObject->height + curObject->height2*2;
				
				if (xpos < 8) {
					xpos = 8;
				} else if (xpos + width > 312) {
					width = 312 - xpos;
				}
				
				if (ypos < 8) {
					ypos = 8;
				} else if (ypos + height > 136) {
					height = 136 - ypos;
				}
				
				_screen->copyRegion(xpos, ypos, xpos, ypos, width, height, 2, 0, Screen::CR_CLIPPED);
				curObject->refreshFlag = 0;
				changed = true;
			}
		}
		curObject = curObject->nextAnimObject;
	}
	if (changed)
		_screen->updateScreen();
}

void KyraEngine::updateAllObjectShapes() {
	debug(9, "updateAllObjectShapes()");
	restoreAllObjectBackgrounds();
	preserveAnyChangedBackgrounds();
	prepDrawAllObjects();
	copyChangedObjectsForward(0);
}

void KyraEngine::animRefreshNPC(int character) {
	debug(9, "animRefreshNPC(%d)", character);
	AnimObject *animObj = &_charactersAnimState[character];
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
			// XXX
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

	_objectQueue = objectRemoveQueue(_objectQueue, animObj);
	if (_objectQueue) {
		_objectQueue = objectQueue(_objectQueue, animObj);
	} else {
		_objectQueue = objectAddHead(0, animObj);
	}
}

void KyraEngine::animRemoveGameItem(int index) {
	debug(9, "animRemoveGameItem(%d)", index);
	restoreAllObjectBackgrounds();
	
	AnimObject *animObj = &_animItems[index];
	animObj->sceneAnimPtr = 0;
	animObj->animFrameNumber = -1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;	
	updateAllObjectShapes();
	animObj->active = 0;
	
	objectRemoveQueue(_objectQueue, animObj);
}

void KyraEngine::animAddGameItem(int index, uint16 sceneId) {
	debug(9, "animRemoveGameItem(%d, %d)", index, sceneId);
	restoreAllObjectBackgrounds();
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];
	AnimObject *animObj = &_animItems[index];
	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = currentRoom->itemsYPos[index];
	animObj->sceneAnimPtr = _shapes[220+currentRoom->itemsTable[index]];
	animObj->animFrameNumber = -1;
	animObj->x1 = currentRoom->itemsXPos[index];
	animObj->y1 = currentRoom->itemsYPos[index];
	animObj->x1 -= fetchAnimWidth(animObj->sceneAnimPtr, _scaleTable[animObj->drawY]) >> 1;
	animObj->y1 -= fetchAnimHeight(animObj->sceneAnimPtr, _scaleTable[animObj->drawY]);
	animObj->x2 = animObj->x1;
	animObj->y2 = animObj->y1;
	animObj->width2 = 0;
	animObj->height2 = 0;
	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

#pragma mark -
#pragma mark - Queue handling
#pragma mark -

AnimObject *KyraEngine::objectRemoveQueue(AnimObject *queue, AnimObject *rem) {
	AnimObject *cur = queue;
	AnimObject *prev = queue;

	while (cur != rem && cur) {
		AnimObject *temp = cur->nextAnimObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (cur == queue) {
		if (!cur)
			return 0;
		return cur->nextAnimObject;
	}
	
	if (!cur->nextAnimObject) {
		if (cur == rem) {
			if (!prev) {
				return 0;
			} else {
				prev->nextAnimObject = 0;
			}
		}
	} else {
		if (cur == rem) {
			prev->nextAnimObject = rem->nextAnimObject;
		}
	}
	
	return queue;
}

AnimObject *KyraEngine::objectAddHead(AnimObject *queue, AnimObject *head) {
	head->nextAnimObject = queue;
	return head;
}

AnimObject *KyraEngine::objectQueue(AnimObject *queue, AnimObject *add) {
	if (add->drawY <= queue->drawY || !queue) {
		add->nextAnimObject = queue;
		return add;
	}
	AnimObject *cur = queue;
	AnimObject *prev = queue;
	while (add->drawY > cur->drawY) {
		AnimObject *temp = cur->nextAnimObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (add->drawY <= cur->drawY) {
		prev->nextAnimObject = add;
		add->nextAnimObject = cur;
	} else {
		cur->nextAnimObject = add;
		add->nextAnimObject = 0;
	}
	return queue;
}

#pragma mark -
#pragma mark - Misc stuff
#pragma mark -

int16 KyraEngine::fetchAnimWidth(const uint8 *shape, int16 mult) {
	debug(9, "fetchAnimWidth(0x%X, %d)", shape, mult);
	if (_features & GF_TALKIE)
		shape += 2;
	return (((int16)READ_LE_UINT16((shape+3))) * mult) >> 8;
}

int16 KyraEngine::fetchAnimHeight(const uint8 *shape, int16 mult) {
	debug(9, "fetchAnimHeight(0x%X, %d)", shape, mult);
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
	debug(9, "backUpRect0(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 3<<3, 24);
	_screen->copyRegionToBuffer(_screen->_curPage, xpos, ypos, 3<<3, 24, _shapes[0]);
}

void KyraEngine::restoreRect0(int xpos, int ypos) {
	debug(9, "restoreRect0(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 3<<3, 24);
	_screen->copyBlockToPage(_screen->_curPage, xpos, ypos, 3<<3, 24, _shapes[0]);
}

void KyraEngine::backUpRect1(int xpos, int ypos) {
	debug(9, "backUpRect1(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 4<<3, 32);
	_screen->copyRegionToBuffer(_screen->_curPage, xpos, ypos, 4<<3, 32, _shapes[1]);
}

void KyraEngine::restoreRect1(int xpos, int ypos) {
	debug(9, "restoreRect1(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 4<<3, 32);
	_screen->copyBlockToPage(_screen->_curPage, xpos, ypos, 4<<3, 32, _shapes[1]);
}

int KyraEngine::getDrawLayer(int x, int y) {
	debug(9, "getDrawLayer(%d, %d)", x, y);
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
	debug(9, "getDrawLayer2(%d, %d, %d)", x, y, height);
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

#pragma mark -
#pragma mark - Pathfinder
#pragma mark -

int KyraEngine::findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize) {
	debug(9, "findWay(%d, %d, %d, %d, 0x%X, %d)", x, y, toX, toY, moveTable, moveTableSize);
	x &= 0xFFFC; toX &= 0xFFFC;
	y &= 0xFFFE; toY &= 0xFFFE;
	
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
	debug(9, "findSubPath(%d, %d, %d, %d, 0x%X, %d, %d)", x, y, toX, toY, moveTable, start, end);
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
	debug(9, "getFacingFromPointToPoint(%d, %d, %d, %d)", x, y, toX, toY);
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
	debug(9, "changePosTowardsFacing(%d, %d, %d)", x, y, facing);
	x += _addXPosTable[facing];
	y += _addYPosTable[facing];
}

bool KyraEngine::lineIsPassable(int x, int y) {
	debug(9, "lineIsPassable(%d, %d)", x, y);
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
	debug(9, "getMoveTableSize(0x%X)", moveTable);
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
	debug(9, "handleSceneChange(%d, %d, %d, %d)", xpos, ypos, unk1, frameReset);
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
	
	int x = _currentCharacter->x1 & 0xFFFC;
	int y = _currentCharacter->y1 & 0xFFFE;
	xpos &= 0xFFFC;
	ypos &= 0xFFFE;
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
	debug(9, "processSceneChange(0x%X, %d, %d)", table, unk1, frameReset);
	if (queryGameFlag(0xEF)) {
		unk1 = 0;
	}
	int *tableStart = table;
	_sceneChangeState = 0;
	_loopFlag2 = 0;
	bool running = true;
	int returnValue = 0;
	uint32 nextFrame = 0;
	while (running) {
		// XXX
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
		
		returnValue = (changeScene(_currentCharacter->facing) != 0);
		if (returnValue) {
			running = false;
		}
		
		if (unk1) {
			// XXX running = false;
			_sceneChangeState = 1;
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
			updateAllObjectShapes();
			updateTextFade();
			if (_currentCharacter->sceneId == 210) {
				// XXX updateKyragemFading
				// XXX playEnd
				// XXX
			}
			if ((nextFrame - _system->getMillis()) >= 10)
				delay(10);
		}
	}
	
	if (frameReset && !(_brandonStatusBit & 2)) {
		_currentCharacter->currentAnimFrame = 7;
	}
	animRefreshNPC(0);
	updateAllObjectShapes();
	return returnValue;
}

int KyraEngine::changeScene(int facing) {
	debug(9, "changeScene(%d)", facing);
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
			updateAllObjectShapes();
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
#pragma mark - Timer handling
#pragma mark -

void KyraEngine::setupTimers() {
	debug(9, "setupTimers()");
	memset(_timers, 0, sizeof(_timers));

	for (int i = 0; i < 34; i++)
		_timers[i].active = 1;

	_timers[0].func = _timers[1].func = _timers[2].func = _timers[3].func = _timers[4].func = 0; //Unused.
	_timers[5].func = _timers[6].func = _timers[7].func = _timers[8].func = _timers[9].func = 0; //_nullsub51;
	_timers[10].func = _timers[11].func = _timers[12].func = _timers[13].func = 0; //_nullsub50;
	_timers[14].func = &KyraEngine::timerCheckAnimFlag2; //_nullsub52;
	_timers[15].func = &KyraEngine::timerUpdateHeadAnims; //_nullsub48;
	_timers[16].func = &KyraEngine::timerSetFlags1; //_nullsub47;
	_timers[17].func = 0; //sub_15120;
	_timers[18].func = &KyraEngine::timerCheckAnimFlag1; //_nullsub53;
	_timers[19].func = &KyraEngine::timerRedrawAmulet; //_nullsub54;
	_timers[20].func = 0; //offset _timerDummy1
	_timers[21].func = 0; //sub_1517C; 
	_timers[22].func = 0; //offset _timerDummy2
	_timers[23].func = 0; //offset _timerDummy3, 
	_timers[24].func = 0; //_nullsub45;
	_timers[25].func = 0; //offset _timerDummy4
	_timers[26].func = 0; //_nullsub46;
	_timers[27].func = 0; //offset _timerDummy5, 
	_timers[28].func = 0; //offset _timerDummy6
	_timers[29].func = 0; //offset _timerDummy7, 
	_timers[30].func = 0; //offset _timerDummy8, 
	_timers[31].func = &KyraEngine::timerFadeText; //sub_151F8;
	_timers[32].func = 0; //_nullsub61;
	_timers[33].func = 0; //_nullsub62;

	_timers[0].countdown = _timers[1].countdown = _timers[2].countdown = _timers[3].countdown = _timers[4].countdown = -1;
	_timers[5].countdown = 5;
	_timers[6].countdown = 7;
	_timers[7].countdown = 8;
	_timers[8].countdown = 9;
	_timers[9].countdown = 7;
	_timers[10].countdown = _timers[11].countdown = _timers[12].countdown = _timers[13].countdown = 420;
	_timers[14].countdown = 600;
	_timers[15].countdown = 11;
	_timers[16].countdown = _timers[17].countdown = 7200;
	_timers[18].countdown = _timers[19].countdown = 600;
	_timers[20].countdown = 7200;
	_timers[21].countdown = 18000;
	_timers[22].countdown = 7200;
	_timers[23].countdown = _timers[24].countdown = _timers[25].countdown = _timers[26].countdown = _timers[27].countdown = 10800;
	_timers[28].countdown = 21600;
	_timers[29].countdown = 7200;
	_timers[30].countdown = 10800;
	_timers[31].countdown = -1;
	_timers[32].countdown = 9;
	_timers[33].countdown = 3;
}

void KyraEngine::updateGameTimers() {
	debug(9, "updateGameTimers()");
	
	if (_system->getMillis() < _timerNextRun)
		return;

	_timerNextRun += 99999;

	for (int i = 0; i < 34; i++) {
		if (_timers[i].active && _timers[i].countdown > -1) {
			if (_timers[i].nextRun <=_system->getMillis()) {
				if (i > 4 && _timers[i].func)
					(*this.*_timers[i].func)(i);

				_timers[i].nextRun = _system->getMillis() + _timers[i].countdown * _tickLength;

			}
		}
		if (_timers[i].nextRun < _timerNextRun)
			_timerNextRun = _timers[i].nextRun;
	}
}

void KyraEngine::clearNextEventTickCount() {
	debug(9, "clearNextEventTickCount()");
	_timerNextRun = 0;
}

int16 KyraEngine::getTimerDelay(uint8 timer) {
	debug(9, "getTimerDelay(%i)", timer);
	return _timers[timer].countdown;
}

void KyraEngine::setTimerCountdown(uint8 timer, int32 countdown) {
	debug(9, "setTimerCountdown(%i, %i)", timer, countdown);
	_timers[timer].countdown = countdown;

	uint32 nextRun = _system->getMillis() + countdown;
	if (nextRun < _timerNextRun)
		_timerNextRun = nextRun;
}

void KyraEngine::enableTimer(uint8 timer) {
	debug(9, "enableTimer(%i)", timer);
	_timers[timer].active = 1;
}

void KyraEngine::disableTimer(uint8 timer) {
	debug(9, "disableTimer(%i)", timer);
	_timers[timer].active = 0;
}

void KyraEngine::timerUpdateHeadAnims(int timerNum) {
	debug(9, "timerUpdateHeadAnims(%i)", timerNum);
	static int8 currentFrame = 0;
	static const int8 frameTable[] = {4, 5, 4, 5, 4, 5, 0, 1, 4, 5,
								4, 4, 6, 4, 8, 1, 9, 4, -1};

	if (_talkingCharNum < 0)
		return;

	_currHeadShape = frameTable[currentFrame];
	currentFrame++;

	if (frameTable[currentFrame] == -1)
		currentFrame = 0;

	animRefreshNPC(0);
	animRefreshNPC(_talkingCharNum);
}

void KyraEngine::timerSetFlags1(int timerNum) {
	debug(9, "timerSetFlags(%i)", timerNum);
	if (_currentCharacter->sceneId == 0x1C)
		return;

	int rndNr = _rnd.getRandomNumberRng(0, 3);

	for (int i = 0; i < 4; i++) {
		if (!queryGameFlag(rndNr + 17)) {
			setGameFlag(rndNr + 17);
			break;
		} else {
			rndNr++;
			if (rndNr > 3)
				rndNr = 0;
		}
	}
}

void KyraEngine::timerFadeText(int timerNum) {
	debug(9, "timerFadeText(%i)", timerNum);
	_fadeText = true;
}

void KyraEngine::setTextFadeTimerCountdown(int16 countdown) {
	debug(9, "setTextFadeTimerCountdown(%i)", countdown);
	//if (countdown == -1)
		//countdown = 32000;

	setTimerCountdown(31, countdown*60);
}

void KyraEngine::timerSetFlags2(int timerNum) {
	debug(9, "timerSetFlags2(%i)", timerNum);
	if (!((uint32*)(_flagsTable+0x2D))[timerNum])
		((uint32*)(_flagsTable+0x2D))[timerNum] = 1;	
}

void KyraEngine::timerCheckAnimFlag1(int timerNum) {
	debug(9, "timerCheckAnimFlag1(%i)", timerNum);
	if (_brandonStatusBit & 0x20) {
		checkAmuletAnimFlags();
		setTimerCountdown(18, -1);
	}
}

void KyraEngine::timerCheckAnimFlag2(int timerNum) {
	debug(9, "timerCheckAnimFlag1(%i)", timerNum);
	if (_brandonStatusBit & 0x2) {
		checkAmuletAnimFlags();
		setTimerCountdown(14, -1);
	}
}

void KyraEngine::checkAmuletAnimFlags() {
	debug(9, "checkSpecialAnimFlags()");
	if (_brandonStatusBit & 2) {
		warning("STUB: playSpecialAnim1");
		// XXX
		setTimerCountdown(19, 300);
	}

	if (_brandonStatusBit & 0x20) {
		warning("STUB: playSpecialAnim2");
		// XXX
		setTimerCountdown(19, 300);
	}
}

void KyraEngine::timerRedrawAmulet(int timerNum) {
	debug(9, "timerRedrawAmulet(%i)", timerNum);
	if (queryGameFlag(241)) {
		drawAmulet();
		setTimerCountdown(19, -1);
	}
}

void KyraEngine::drawAmulet() {
	debug(9, "drawAmulet()");
	static const int16 amuletTable1[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x150, 0x155, 0x15A, 0x15F, 0x164, 0x145, -1};
	static const int16 amuletTable3[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x14F, 0x154, 0x159, 0x15E, 0x163, 0x144, -1};
	static const int16 amuletTable2[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x152, 0x157, 0x15C, 0x161, 0x166, 0x147, -1};
	static const int16 amuletTable4[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x151, 0x156, 0x15B, 0x160, 0x165, 0x146, -1};

	_screen->hideMouse();

	int i = 0;
	while (amuletTable1[i] != -1) {
		if (queryGameFlag(87))
			_screen->drawShape(0, _shapes[4+amuletTable1[i]], _amuletX[0], _amuletY[0], 0, 0);

		if (queryGameFlag(89))
			_screen->drawShape(0, _shapes[4+amuletTable2[i]], _amuletX[1], _amuletY[1], 0, 0);

		if (queryGameFlag(86))
			_screen->drawShape(0, _shapes[4+amuletTable3[i]], _amuletX[2], _amuletY[2], 0, 0);

		if (queryGameFlag(88))
			_screen->drawShape(0, _shapes[4+amuletTable4[i]], _amuletX[3], _amuletY[3], 0, 0);

		delay(3 * _tickLength);
		i++;
	}
	_screen->showMouse();
}

#pragma mark -
#pragma mark - Input
#pragma mark -

void KyraEngine::processInput(int xpos, int ypos) {
	debug(9, "processInput(%d, %d)", xpos, ypos);
	if (processInputHelper(xpos, ypos)) {
		return;
	}
	uint8 item = findItemAtPos(xpos, ypos);
	if (item == 0xFF) {
		if (clickEventHandler(xpos, ypos))
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
			handleSceneChange(xpos, ypos, 1, 1);
			return;
		} else {
			if (_itemInHand != -1) {
				if (ypos < 155) {
					if (hasClickedOnExit(xpos, ypos)) {
						handleSceneChange(xpos, ypos, 1, 1);
						return;
					}
					dropItem(0, _itemInHand, xpos, ypos, 1);
				}
			} else {
				if (ypos <= 155) {
					handleSceneChange(xpos, ypos, 1, 1);
				}
			}
		}
	}	
}

int KyraEngine::processInputHelper(int xpos, int ypos) {
	debug(9, "processInputHelper(%d, %d)", xpos, ypos);
	uint8 item = findItemAtPos(xpos, ypos);
	if (item != 0xFF) {
		if (_itemInHand == -1) {
			_screen->hideMouse();
			animRemoveGameItem(item);
			// XXX call kyraPlaySound(53)
			assert(_currentCharacter->sceneId < _roomTableSize);
			Room *currentRoom = &_roomTable[_currentCharacter->sceneId];
			int item2 = currentRoom->itemsTable[item];
			currentRoom->itemsTable[item] = 0xFF;
			setMouseItem(item2);
			// XXX updateSentenceCommand
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
	debug(9, "clickEventHandler(%d, %d)", xpos, ypos);
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

void KyraEngine::updateMousePointer() {
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
	
	if (newMouseState && _mouseState != newMouseState) {
		_mouseState = newMouseState;
		_screen->hideMouse();
		_screen->setMouseCursor(newX, newY, _shapes[4+shape]);
		_screen->showMouse();
	}
	
	if (!newMouseState) {
		if (_mouseState != _itemInHand) {
			if (_mouseY > 158 || (_mouseX >= 12 && _mouseX < 308 && _mouseY < 136 && _mouseY >= 12)) {
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
	debug(9, "hasClickedOnExit(%d, %d)", xpos, ypos);
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
	debug(9, "clickEventHandler2()");
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

} // End of namespace Kyra
