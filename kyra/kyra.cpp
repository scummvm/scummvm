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
	_charactersAnimState = &_animStates[0];
	_animObjects =  &_animStates[5];
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
	
	memset(_shapes, 0, sizeof(_shapes));
	memset(_wsaObjects, 0, sizeof(_wsaObjects));

	_fastMode = false;
	_talkCoords.y = 0x88;
	_talkCoords.x = 0;
	_talkCoords.w = 0;
	_talkMessageY = 0xC;
	_talkMessageH = 0;
	_talkMessagePrinted = false;
	
	_mouseX = _mouseY = -1;
	_needMouseUpdate = true;
	
	_brandonPosX = _brandonPosY = -1;
	_brandonDrawFrame = 113;
	
	memset(_itemTable, 0, sizeof(_itemTable));
	memset(_exitList, 0xFFFF, sizeof(_exitList));
	_exitListPtr = 0;
	_pathfinderFlag = 0;
	
	_sceneChangeState = _loopFlag2 = 0;
	
	_movFacingTable = new int[150];
	assert(_movFacingTable);
	_movFacingTable[0] = 8;

	return 0;
}

KyraEngine::~KyraEngine() {
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
		seq_intro();
		startup();
		mainLoop();
	}
	res_unloadResources();
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
			_roomTable[i].itemsXPos[item] = 0;
			_roomTable[i].itemsYPos[item] = 0;
			_roomTable[i].unkField3[item] = 0;
		}
	}
	loadCharacterShapes();
	loadSpecialEffectShapes();
	loadItems();
	loadMainScreen();
	loadPalette("PALETTE.COL", _screen->_currentPalette);
	_screen->setScreenPalette(_screen->_currentPalette);
	// XXX
	initAnimStateList();
	setCharactersInDefaultScene();
	
	_gameSpeed = 50;
	memset(_flagsTable, 0, sizeof(_flagsTable));
	
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
				} else {
					++_currentRoom;
					if (_currentRoom > ARRAYSIZE(_shapes))
						_currentRoom = 3;
				}
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_mouseX = event.mouse.x;
				_mouseY = event.mouse.y;
				_needMouseUpdate = true;
				break;
			case OSystem::EVENT_QUIT:
				_quitFlag = true;
				break;
			default:
				break;
			}
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

		if (_needMouseUpdate) {
			_screen->hideMouse();
			_screen->showMouse();
			_needMouseUpdate = false;
		}
		_screen->updateScreen();

		delay((frameTime + _gameSpeed) - _system->getMillis());
	}
}

void KyraEngine::loadPalette(const char *filename, uint8 *palData) {
	debug(9, "KyraEngine::loadPalette('%s' 0x%X)", filename, palData);
	uint32 fileSize = 0;
	uint8 *srcData = _res->fileData(filename, &fileSize);

	if (palData && fileSize) {
		debug(9, "Loading a palette of size %i from '%s'", fileSize, filename);
		memcpy(palData, srcData, fileSize);		
	}
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

void KyraEngine::waitTicks(int ticks) {
	debug(9, "KyraEngine::waitTicks(%d)", ticks);
	const uint32 end = _system->getMillis() + ticks * 1000 / 60;
	do {
		OSystem::Event event;
		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_QUIT:
				_quitFlag = true;
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
		
		moveCharacterToPos(0, 2, xpos, ypos);
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
	assert(sceneId < _roomFilenameTableSize);
	
	assert(sceneId < _roomTableSize);
	Room *currentRoom = &_roomTable[sceneId];

	assert(_currentCharacter->sceneId < _roomTableSize);
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);
	char datFileNameBuffer[32];
	strcpy(datFileNameBuffer, _roomFilenameTable[tableId]);
	strcat(datFileNameBuffer, ".DAT");
	_sprites->loadDAT(datFileNameBuffer);
	_scriptInterpreter->unloadScript(_scriptClickData);
	loadSceneMSC();
	
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
	// XXX setupSceneItems
	initSceneData(facing, unk2, brandonAlive);
		
	// XXX setTextFadeTimerCountdown
	_scriptClick->variables[3] = 1;
	
	_screen->showMouse();
	if (!brandonAlive) {
		// XXX seq_poisionDeathNow
	}
}

void KyraEngine::moveCharacterToPos(int character, int facing, int xpos, int ypos) {
	debug(9, "moveCharacterToPos(%d, %d, %d, %d)", character, facing, xpos, ypos);
	Character *ch = &_characterList[character];
	_screen->hideMouse();
	xpos &= 0xFFFC;
	ypos &= 0xFFFE;
	switch (facing) {
		case 0:
			if (ypos < ch->y1) {
				setCharacterPositionWithUpdate(character);
			}
			break;
		
		case 2:	
			if (ch->x1 < xpos) {
				setCharacterPositionWithUpdate(character);
			}
			break;
		
		case 4:
			if (ypos > ch->y1) {
				setCharacterPositionWithUpdate(character);
			}
			break;
		
		case 6:
			if (ch->x1 > xpos) {
				setCharacterPositionWithUpdate(character);
			}
			break;
		
		default:
			break;
	}
	_screen->showMouse();
}

void KyraEngine::setCharacterPositionWithUpdate(int character) {
	debug(9, "setCharacterPositionWithUpdate(%d)", character);
	// XXX game_updateAnimsFlags
	setCharacterPosition(character, 0);
	updateAllObjectShapes();
	// XXX processPalette
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
						if (facingIsFour[character] < 2) {
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
		if (maxAnimationFrame[36+character] < ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[36+character];
		}
		if (maxAnimationFrame[30+character] < ch->currentAnimFrame) {
			ch->currentAnimFrame = maxAnimationFrame[36+character];
		}
	} else if (facing == 4) {
		if (maxAnimationFrame[18+character] < ch->currentAnimFrame) {
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
	loadBitmap(fileNameBuffer, 3, 5, 0);
}

// maybe move this two functions to Screen
void KyraEngine::blockInRegion(int x, int y, int width, int height) {
	debug(9, "blockInRegion(%d, %d, %d, %d, %d)", x, y, width, height);
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
	debug(9, "blockOutRegion(%d, %d, %d, %d, %d)", x, y, width, height);
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
	// TODO: check there it is done normally
	_screen->setScreenPalette(_screen->_currentPalette);
	_screen->copyRegion(4, 4, 4, 4, 308, 132, 3, 0);

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
	
	int xpos2 = 0;
	int setFacing = 1;
	
	int xpos = 0, ypos = 0;
	
	if (_brandonPosX == -1 && _brandonPosY == -1) {
		switch (facing) {
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
		
		if ((_northExitHeight & 0xFF) >= ypos) {
			ypos = (_northExitHeight & 0xFF);
		}
		if (xpos >= 308) {
			xpos = 304;
		}
		if ((_northExitHeight >> 8) - 2 <= ypos) {
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
	
	int ypos2 = 0;
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
				break;
				
			case 3:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 2;
				xpos2 = 204;
				ypos2 = 94;
				break;
				
			case 26:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 4;
				xpos2 = 192;
				ypos2 = 128;
				break;
				
			case 44:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 6;
				xpos2 = 156;
				ypos2 = 96;
				break;
				
			case 37:
				_currentCharacter->x1 = xpos;
				_currentCharacter->x2 = xpos;
				_currentCharacter->y1 = ypos;
				_currentCharacter->y2 = ypos;
				facing = 2;
				xpos2 = 148;
				ypos2 = 114;
				break;
				
			default:
				break;
		}
		
		setFacing = 0;
		unk1 = 1;
	}
	
	_brandonPosX = _brandonPosY = -1;
	
	if (unk1 != 0 && setFacing != 0) {
		xpos2 = xpos;
		switch (facing) {
			case 0:
				ypos = 142;
				break;
				
			case 2:
				xpos = -16;
				break;
				
			case 4:
				ypos = (_northExitHeight & 0xFF) - 4;
				break;
				
			case 6:
				xpos = 335;
				break;
				
			default:
				break;
		}
	}
	
	xpos2 &= 0xFFFC;
	ypos2 &= 0xFE;
	xpos &= 0xFFFC;
	ypos &= 0xFFFE;
	_currentCharacter->facing = facing;
	_currentCharacter->x1 = xpos;
	_currentCharacter->x2 = xpos;
	_currentCharacter->y1 = ypos;
	_currentCharacter->y2 = ypos;
	
	initSceneObjectList(brandonAlive);
	
	if (unk1 != 0 && brandonAlive == 0) {
		moveCharacterToPos(0, facing, xpos2, ypos2);
	}
	
	// XXX _mousePointerFlag
	_scriptClick->variables[4] = -1;
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
		curAnimState = &_animObjects[i];
		curAnimState->active = 0;
		curAnimState->refreshFlag = 0;
		curAnimState->bkgdChangeFlag = 0;
		// XXX this needs the dat loader
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
			
			curAnimState->x1 -= (_scaleTable[curAnimState->drawY] >> 1);
			curAnimState->y1 -= _scaleTable[curAnimState->drawY];
			
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
	// XXX game_unkScreen
	_screen->showMouse();
	copyChangedObjectsForward(0);
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
		if (room->itemsTable[i] != 0xFF)
			return i;
	}
	return 0xFF;
}

byte KyraEngine::findItemAtPos(int x, int y) {
	debug(9, "findItemAtPos(%d, %d)", x, y);
	assert(_currentCharacter->sceneId < _roomTableSize);
	uint8 *itemsTable = _roomTable[_currentCharacter->sceneId].itemsTable;
	uint16 *xposOffset = _roomTable[_currentCharacter->sceneId].itemsXPos;
	uint8 *yposOffset = _roomTable[_currentCharacter->sceneId].itemsYPos;
	
	int highestYPos = -1;
	byte returnValue = 0xFF;
	
	for (int i = 0; i < 12; ++i) {
		if (*itemsTable != 0xFF) {
			int xpos = *xposOffset - 8;
			int xpos2 = *xposOffset + 10;
			if (x > xpos && x < xpos2) {
				assert(*itemsTable < ARRAYSIZE(_itemTable));
				int itemHeight = _itemTable[*itemsTable].height;
				int ypos = *yposOffset;
				int ypos2 = ypos - itemHeight - 3;
				
				if (y < ypos2 && (ypos+3) > y) {
					if (highestYPos <= ypos) {
						returnValue = i;
						highestYPos = ypos;
					}
				}
			}
		}
		xposOffset += 2;
		yposOffset += 1;
		itemsTable += 1;
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
			warning("placeItemInGenericMapScene: placing an item is NOT implemented");
			//if (!sub_B010(room, item, -1, -1, 2, 0))
			//	continue;
			break;
		}
	}
}

#pragma mark -
#pragma mark - Animation specific code
#pragma mark -

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
			
			int temp = 0;
			if (curObject->flags & 0x800) {
				temp = 7;
			} else if (!curObject->unk1) {
				temp = 0;
			} else {
				// XXX temp = sub_13368(curObject->drawY)
				temp = 0;
			}
			
			// talking head functionallity
			if (!true) {
				// XXX
			}
			
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
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x100, (uint8*)_unkBrandonPoisonFlags, 1, temp);
					} else if (flagUnk3 & 0x4000) {
						// XXX
						int hackVar = 0;
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4000, hackVar, 0);
					} else {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1, temp);
					}
				} else {
					if (flagUnk3 & 0x100) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x104, (uint8*)_unkBrandonPoisonFlags, 1, temp, _brandonScaleX, _brandonScaleY);
					} else if (flagUnk3 & 0x4000) {
						// XXX
						int hackVar = 0;
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4004, 0, temp, hackVar, _brandonScaleX, _brandonScaleY);
					} else {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4, temp, _brandonScaleX, _brandonScaleY);
					}
				}
			} else {
				if (curObject->index >= 16 && curObject->index <= 27) {
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | 4, temp, (int)_scaleTable[curObject->drawY], (int)_scaleTable[curObject->drawY]);
				} else {
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags, temp);
				}
			}
		}
		curObject = curObject->nextAnimObject;
	}
}

void KyraEngine::copyChangedObjectsForward(int refreshFlag) {
	debug(9, "copyChangedObjectsForward(%d)", refreshFlag);
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		if (curObject->active) {
			if (curObject->refreshFlag || refreshFlag) {
				int xpos = 0, ypos = 0, width = 0, height = 0;
				xpos = curObject->x1 - (curObject->width2+1);
				ypos = curObject->y1 - curObject->height2;				
				width = curObject->width + curObject->width2*2;
				height = curObject->height + curObject->height2*2;
				
				_screen->copyRegion(xpos, ypos, xpos, ypos, width<<3, height, 2, 0, Screen::CR_CLIPPED);
				curObject->refreshFlag = 0;
			}
		}
		curObject = curObject->nextAnimObject;
	}
}

void KyraEngine::updateAllObjectShapes() {
	debug(9, "updateAllObjectShapes()");
	restoreAllObjectBackgrounds();
	preserveAnyChangedBackgrounds();
	prepDrawAllObjects();
	copyChangedObjectsForward(0);

	_screen->updateScreen();
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
	// XXX
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
		
	} else {
		_objectQueue = objectAddHead(_objectQueue, animObj);
	}
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
		if (!prev) {
			return 0;
		} else {
			prev->nextAnimObject = 0;
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
	return 0;
}

#pragma mark -
#pragma mark - Misc stuff
#pragma mark -

int16 KyraEngine::fetchAnimWidth(const uint8 *shape, int16 mult) {
	debug(9, "fetchAnimWidth(0x%X, %d)", shape, mult);
	if (_features & GF_TALKIE)
		shape += 2;
	return ((int16)READ_LE_UINT16((shape+3))) * mult;
}

int8 KyraEngine::fetchAnimHeight(const uint8 *shape, int8 mult) {
	debug(9, "fetchAnimHeight(0x%X, %d)", shape, mult);
	if (_features & GF_TALKIE)
		shape += 2;
	return ((int8)*(shape+2)) * mult;
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
	
	while (true) {
		int newFacing = getFacingFromPointToPoint(x, y, toX, toY);
		changePosTowardsFacing(curX, curY, newFacing);
		
		if (curX == toX && curY == toY) {
			if (!lineIsPassable(curX, curY))
				break;
			moveTable[lastUsedEntry++] = newFacing;
			x = curX;
			y = curY;
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
			
			assert(pathTable1 && pathTable2);
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
			memcpy(&moveTable[lastUsedEntry], pathTable1, temp);
			lastUsedEntry += temp;
		} else {
			if (lastUsedEntry + tempValue > moveTableSize) {
				delete [] pathTable1;
				delete [] pathTable2;
				return 0x7D00;
			}
			memcpy(&moveTable[lastUsedEntry], pathTable2, tempValue);
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
	static const int8 addPosTable1[] = { -1,  0, -1,  4, -1,  0, -1, -4, -1, -4, -1,  0, -1,  4, -1,  0 };
	static const int8 addPosTable2[] = { -1,  2, -1,  0, -1, -2, -1,  0, -1,  0, -1,  2, -1,  0, -1, -2 };
	
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
		changePosTowardsFacing(xpos1, ypos1, facingTable1[start<<3 + newFacing]);
		while (true) {
			if (!lineIsPassable(xpos1, ypos1)) {
				if (facingTable1[start<<3 + newFacing] == newFacing) {
					return 0x7D00;
				}
				newFacing = facingTable1[start<<3 + newFacing];
				xpos1 = x;
				ypos1 = x;
				continue;
			}
			break;
		}
		// debug drawing
		//if (xpos1 >= 0 && ypos1 >= 0 && xpos1 < 320 && ypos1 < 200) {
		//	_screen->setPagePixel(0, xpos1,ypos1, unkTable[start]);
		//	_screen->updateScreen();
		//	waitTicks(5);
		//}
		if (newFacing & 1) {
			int temp = xpos1 + addPosTable1[newFacing + start * 8];
			if (toX == temp) {
				temp = ypos1 + addPosTable2[newFacing + start * 8];
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
		
		newFacing = facingTable3[start<<3 + newFacing];
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
		int temp = xdiff;
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
		if (y < _northExitHeight || y > 135)
			return true;
	}
	
	if (y > 137) {
		return false;
	}
	
	int ypos = 8;
	if (_scaleMode) {
		int scaling = (_scaleTable[y] >> 5) + 1;
		if (8 < scaling)
			ypos = scaling;
	}
	
	x -= (ypos >> 1);
	if (y < 0)
		y = 0;
	
	int xpos = x;
	int xtemp = xpos + ypos - 1;
	if (xpos < 0)
		xpos = 0;
		
	if (xtemp > 319)
		xtemp = 319;
		
	for (; xpos < xtemp; ++xpos) {
		if (!(_screen->getShapeFlag1(xpos, y) & 0xFF))
			return false;
	}
	
	return true;
}

int KyraEngine::getMoveTableSize(int *moveTable) {
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
	int *curPosition = &moveTable[1];

	while (*curPosition != 8) {
		if (*curPosition == facingTable[*oldPosition]) {
			retValue -= 2;
			*oldPosition = 9;
			*curPosition = 9;
			
			while (tempPosition != moveTable) {
				--tempPosition;
				if (*tempPosition != 9)
					break;
			}
			
			if (tempPosition == moveTable && *tempPosition == 9) {
				while (*tempPosition == 8 || *tempPosition != 9) {
					++tempPosition;
				}
				if (*tempPosition == 8) {
					return 0;
				}
			}
			
			while (*curPosition == 8 || *curPosition != 9) {
				++curPosition;
			}
		}
		
		if (unkTable[*curPosition+(*oldPosition*8)] != -1) {
			--retValue;
			*oldPosition = unkTable[*curPosition+(*oldPosition*8)];
			*curPosition = 9;
			
			if (tempPosition != oldPosition) {
				curPosition = oldPosition;
				oldPosition = tempPosition;
				if (tempPosition != moveTable) {
					--tempPosition;
					while (*tempPosition == 9) {
						++tempPosition;
					}
				}
			} else {
				++curPosition;
				while (*curPosition == 9) {
					++curPosition;
				}
			}
			continue;
		}
		
		tempPosition = oldPosition;
		oldPosition = curPosition;
		++retValue;
		++curPosition;
		while (*curPosition == 9) {
			++curPosition;
		}
	}
	
	return retValue;
}

#pragma mark -
#pragma mark - Scene handling
#pragma mark -

int KyraEngine::handleSceneChange(int xpos, int ypos, int unk1, int frameReset) {
	debug(9, "handleSceneChange(%d, %d, %d, %d)", xpos, ypos, unk1, frameReset);
	if (!queryGameFlag(0xEF)) {
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
	
	if (ypos < _northExitHeight+2) {
		if (_roomTable[sceneId].northExit != 0xFFFF) {
			xpos = _northExitHeight;
			ypos = _sceneExits.northXPos;
			_pathfinderFlag = 14;
		}
	} else if (ypos >= 136) {
		if (_roomTable[sceneId].southExit != 0xFFFF) {
			xpos = 136;
			ypos = _sceneExits.southXPos;
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
	debug(9, "handleSceneChange(0x%X, %d, %d)", table, unk1, frameReset);
	if (queryGameFlag(0xEF)) {
		unk1 = 0;
	}
	int *tableStart = table;
	_sceneChangeState = 0;
	_loopFlag2 = 0;
	bool running = true;
	int returnValue = 0;
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
		
		if (unk1) {
			// XXX
		}
		
		if (forceContinue || !running) {
			continue;
		}
		
		int temp = 0;
		if (table == tableStart) {
			temp = setCharacterPosition(0, 0);
		} else {
			temp = setCharacterPosition(0, table);
		}
		if (!temp)
			continue;
		++table;
		// XXX updateAnimFlags
		// XXX updateMousePointer
		// XXX updateGameTimers
		updateAllObjectShapes();
		// XXX processPalette
		if (_currentCharacter->sceneId == 210) {
			// XXX updateKyragemFading
			// XXX playEnd
			// XXX
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
		uint16 *ptr = _exitListPtr;
		// this loop should be only entered on time, seems to be some hack in the original
		while (true) {
			if (*ptr == 0xFFFF)
				break;
			
			if (*ptr > _currentCharacter->x1 || _currentCharacter->y1 < *(ptr + 1) || _currentCharacter->x1 > *(ptr + 2) || _currentCharacter->y1 > *(ptr + 3)) {
				ptr += 10;
				continue;
			}
			_brandonPosX = *(ptr + 6);
			_brandonPosY = *(ptr + 7);
			uint16 sceneId = *(ptr + 5);
			facing = *(ptr + 4);
			int unk1 = *(ptr + 8);
			int unk2 = *(ptr + 9);
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
			resetGameFlag(0xEF);
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
	
	if (((_northExitHeight >> 8) & 0xFF) - 2 >= ypos || ((_northExitHeight >> 8) & 0xFF) - 2 < _currentCharacter->y1) {
		facing = 4;
		returnValue = 1;
	}
	
	if (xpos <= 12 || _currentCharacter->y1 <= 12) {
		facing = 6;
		returnValue = 1;
	}
	
	if (!returnValue)
		return 0;
	
	int sceneId = 0xFFFF;
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
} // End of namespace Kyra
