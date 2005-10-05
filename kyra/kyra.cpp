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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#include "sound/mixer.h"
#include "sound/mididrv.h"

#include "kyra/kyra.h"
#include "kyra/resource.h"
#include "kyra/screen.h"
#include "kyra/script.h"
#include "kyra/sound.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"

#define TEST_SPRITES 1

#define SEQOP(n, x) { n, &KyraEngine::x, #x }

using namespace Kyra;

struct KyraGameSettings {
	const char *name;
	const char *description;
	uint32 features;
	const char *detectName;
	GameSettings toGameSettings() const {
		GameSettings dummy = { name, description, features };
		return dummy;
	}
};

static const KyraGameSettings kyra_settings[] = {
	{ "kyra1", "Legend of Kyrandia (Floppy)", GF_FLOPPY | GF_KYRA1, "INTRO.SND" },
	{ "kyra1cd", "Legend of Kyrandia (CD)",  GF_TALKIE | GF_KYRA1,  "CHAPTER1.VRM" },
	{ "kyra1demo", "Legend of Kyrandia (Demo)", GF_DEMO | GF_FLOPPY | GF_KYRA1, "DEMO1.WSA" },
//	{ "kyra2", "Hand of Fate (Floppy)", GF_FLOPPY | GF_KYRA2, 0 },
//	{ "kyra2cd", "Hand of Fate (CD)", GF_TALKIE | GF_KYRA2, "AUDIO.PAK" },
//	{ "kyra3", "Malcolm's Revenge", GF_TALKIE | GF_KYRA3, "K3INTRO0.VQA" },
	{ 0, 0, 0, 0 }
};

GameList Engine_KYRA_gameList() {
	GameList games;
	const KyraGameSettings *g = kyra_settings;
	while (g->name) {
		games.push_back(g->toGameSettings());
		g++;
	}
	return games;
}

DetectedGameList Engine_KYRA_detectGames(const FSList &fslist) {
	const KyraGameSettings *game;
	DetectedGameList detectedGames;

	for (game = kyra_settings; game->name; ++game) {
		if (game->detectName == NULL)
			continue;

		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				const char *name = file->displayName().c_str();
				if ((!scumm_stricmp(game->detectName, name))) {
					detectedGames.push_back(game->toGameSettings());
					break;
				}
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

	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	// gets the game
	if (detector->_game.features & GF_KYRA1) {
		if (detector->_game.features & GF_DEMO) {
			_game = KYRA1DEMO;
		} else if (detector->_game.features & GF_FLOPPY) {
			_game = KYRA1;
		} else {
			_game = KYRA1CD;
		}
	} else if (detector->_game.features & GF_KYRA2) {
		if (detector->_game.features & GF_FLOPPY) {
			_game = KYRA2;
		} else {
			_game = KYRA2CD;
		}
	} else if (detector->_game.features & GF_KYRA3) {
		_game = KYRA3;
	} else {
		error("unknown game");
	}
}

int KyraEngine::init(GameDetector &detector) {
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

	_fastMode = false;
	_talkCoords.y = 0x88;
	_talkCoords.x = 0;
	_talkCoords.w = 0;
	_talkMessageY = 0xC;
	_talkMessageH = 0;
	_talkMessagePrinted = false;

	return 0;
}

KyraEngine::~KyraEngine() {
	delete _sprites;
	delete _screen;
	delete _res;
	delete _midi;
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int KyraEngine::go() {
	_quitFlag = false;
	uint32 sz;

	if (_game == KYRA1) {
		_screen->loadFont(Screen::FID_6_FNT, _res->fileData("6.FNT", &sz));
	}
	_screen->loadFont(Screen::FID_8_FNT, _res->fileData("8FAT.FNT", &sz));
	_screen->setScreenDim(0);

	_abortIntroFlag = false;

	if (_game == KYRA1DEMO) {
		seq_demo();
	} else {
		seq_intro();
		startup();
		mainLoop();
	}
	return 0;
}

void KyraEngine::startup() {
	debug(9, "KyraEngine::startup()");
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	_screen->setTextColorMap(colorMap);
//	_screen->setFont(Screen::FID_6_FNT);
	_screen->setAnimBlockPtr(3750);
	_gameSpeed = 50;
	memset(_flagsTable, 0, sizeof(_flagsTable));
	setupRooms();
	// XXX
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
				} else if (event.kbd.keycode == ' ') {
					loadRoom((++_currentRoom) % MAX_NUM_ROOMS);
				}
				break;
			// XXX
			case OSystem::EVENT_LBUTTONDOWN:
				loadRoom((++_currentRoom) % MAX_NUM_ROOMS);
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				loadRoom((--_currentRoom) % MAX_NUM_ROOMS);
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

void KyraEngine::drawRoom() {
	//_screen->clearPage(0);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 10, 0);
	_screen->copyRegion(4, 4, 4, 4, 308, 132, 14, 0);
	_sprites->doAnims();
	_sprites->drawSprites(14, 0);
}

void KyraEngine::setCursor(uint8 cursorID) {
	debug(9, "KyraEngine::setCursor(%i)", cursorID);
	assert(cursorID < _cursorsCount);

	loadBitmap("mouse.cps", 2, 2, _screen->_currentPalette); 
	uint8 *cursor = new uint8[_cursors[cursorID].w * _cursors[cursorID].h];

	_screen->copyRegionToBuffer(2, _cursors[cursorID].x, _cursors[cursorID].y, _cursors[cursorID].w, _cursors[cursorID].h, cursor);
	_system->setMouseCursor(cursor, _cursors[cursorID].w, _cursors[cursorID].h, 0, 0, 0);
	_system->showMouse(true);

	delete[] cursor;
}

void KyraEngine::setupRooms() {
	// XXX 
	// Just a few sample rooms, most with sprite anims.
	memset(_rooms, 0, sizeof(_rooms));
	_rooms[0].filename = "gemcut";
	_rooms[1].filename = "arch";
	_rooms[2].filename = "sorrow";
	_rooms[3].filename = "emcav";
	_rooms[4].filename = "extpot";
	_rooms[5].filename = "spell";
	_rooms[6].filename = "song";
	_rooms[7].filename = "belroom";
	_rooms[8].filename = "kyragem";
	_rooms[9].filename = "lephole";
	_rooms[10].filename = "sickwil";
	_rooms[11].filename = "temple";
}

void KyraEngine::loadRoom(uint16 roomID) {
	debug(9, "KyraEngine::loadRoom(%i)", roomID);
	char buf[12];
	
	loadPalette("palette.col", _screen->_currentPalette);

	//loadPalette(_rooms[roomID].palFilename, _screen->_currentPalette);
	//_screen->setScreenPalette(_screen->_currentPalette);

	_screen->clearPage(14);
	_screen->clearPage(0);
	_screen->clearPage(10);

	// Loading GUI bitmap
	if (_game == KYRA1CD) {
		loadBitmap("MAIN_ENG.CPS", 10, 10, 0);
	} else {
		loadBitmap("MAIN15.CPS", 10, 10, 0);
	}

	// Loading main room background
	strncpy(buf, _rooms[roomID].filename, 8);
	strcat(buf, ".cps");
	loadBitmap( buf, 14, 14, 0); 

	// Loading the room mask
	strncpy(buf, _rooms[roomID].filename, 8);
	strcat(buf, ".msc");
	loadBitmap( buf, 12, 12, 0); 

	// Loading room data
	strncpy(buf, _rooms[roomID].filename, 8);
	strcat(buf, ".dat");
	_sprites->loadDAT(buf); 

	setCursor(0);
}

void KyraEngine::mainLoop() {
	debug(9, "KyraEngine::mainLoop()");
#ifdef TEST_SPRITES
	_currentRoom = 0;
	loadRoom(_currentRoom);

	while (!_quitFlag) {
		int32 frameTime = (int32)_system->getMillis();

		drawRoom();
		_screen->updateScreen();

		delay((frameTime + _gameSpeed) - _system->getMillis());
	}
#endif
}

void KyraEngine::loadPalette(const char *filename, uint8 *palData) {
	debug(9, "KyraEngine::loadPalette('%s' 0x%X)", filename, palData);
	uint32 fileSize = 0;
	uint8 *srcData = _res->fileData(filename, &fileSize);

	if (palData && fileSize) {
		debug(9, "Loading a palette of size %i from %s", fileSize, filename);
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
	
	seq_playSpecialSequence(_seq_demoData_WestwoodLogo, true);
	waitTicks(60);

	seq_playSpecialSequence(_seq_demoData_KyrandiaLogo, true);

	_screen->fadeToBlack();
	_screen->clearPage(2);
	_screen->clearPage(0);

	seq_playSpecialSequence(_seq_demoData_Demo1, true);

	_screen->clearPage(0);
	seq_playSpecialSequence(_seq_demoData_Demo2, true);

	_screen->clearPage(0);
	seq_playSpecialSequence(_seq_demoData_Demo3, true);

	_screen->clearPage(0);
	seq_playSpecialSequence(_seq_demoData_Demo4, true);

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
	static const IntroProc introProcTable[] = {
		&KyraEngine::seq_introLogos,
//		&KyraEngine::seq_introStory,
		&KyraEngine::seq_introMalcomTree,
		&KyraEngine::seq_introKallakWriting,
		&KyraEngine::seq_introKallakMalcom
	};
	_skipIntroFlag = true; // only true if user already played the game once
	_seq_copyViewOffs = true;
	_screen->setFont(Screen::FID_8_FNT);
	snd_playTheme(MUSIC_INTRO, 2);
 	snd_setSoundEffectFile(MUSIC_INTRO);
	setTalkCoords(144);
	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		(this->*introProcTable[i])();
	}
	setTalkCoords(136);
	waitTicks(30);
	_seq_copyViewOffs = false;
	_midi->stopMusic();
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
	
	if (seq_playSpecialSequence(_seq_floppyData_WestwoodLogo, _skipIntroFlag)) {
		_screen->fadeToBlack();
		_screen->clearPage(0);
		return;
	}
	waitTicks(60);
	if (seq_playSpecialSequence(_seq_floppyData_KyrandiaLogo, _skipIntroFlag)) {
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

	seq_playSpecialSequence(_seq_floppyData_Forest, true);
}

void KyraEngine::seq_introStory() {
	debug(9, "KyraEngine::seq_introStory()");
	loadBitmap("MAIN_ENG.CPS", 3, 3, 0);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
	// XXX wait 360 ticks
}

void KyraEngine::seq_introMalcomTree() {
	debug(9, "KyraEngine::seq_introMalcomTree()");
	_screen->_curPage = 0;
	_screen->clearPage(3);
	seq_playSpecialSequence(_seq_floppyData_MalcomTree, true);
}

void KyraEngine::seq_introKallakWriting() {
	debug(9, "KyraEngine::seq_introKallakWriting()");
	seq_makeHandShapes();
	_screen->setAnimBlockPtr(5060);
	_screen->_charWidth = -2;
	_screen->clearPage(3);
	seq_playSpecialSequence(_seq_floppyData_KallakWriting, true);
	seq_freeHandShapes();
}

void KyraEngine::seq_introKallakMalcom() {
	debug(9, "KyraEngine::seq_introKallakMalcom()");
	_screen->clearPage(3);
	seq_playSpecialSequence(_seq_floppyData_KallakMalcom, true);
}

uint8 *KyraEngine::seq_setPanPages(int pageNum, int shape) {
	debug(9, "KyraEngine::seq_setPanPages(%d, %d)", pageNum, shape);
	uint8 *panPage = 0;
	const uint8 *data = _screen->getPagePtr(pageNum);
	uint16 numShapes = READ_LE_UINT16(data);
	if (shape < numShapes) {
		uint32 offs = 0;
		if (_game == KYRA1CD) {
			offs = READ_LE_UINT32(data + 2 + shape * 4);
		} else {
			offs = READ_LE_UINT16(data + 2 + shape * 2);
		}
		if (offs != 0) {
			data += offs;
			uint16 sz = READ_LE_UINT16(data + 6);
			panPage = (uint8 *)malloc(sz);
			if (panPage) {
				memcpy(panPage, data, sz);
			}
		}
	}
	return panPage;
}

void KyraEngine::seq_makeHandShapes() {
	debug(9, "KyraEngine::seq_makeHandShapes()");
	loadBitmap("WRITING.CPS", 3, 3, 0);
	for (int i = 0; i < 3; ++i) {
		_seq_handShapes[i] = seq_setPanPages(3, i);
	}
}

void KyraEngine::seq_freeHandShapes() {
	debug(9, "KyraEngine::seq_freeHandShapes()");
	for (int i = 0; i < 3; ++i) {
		free(_seq_handShapes[i]);
		_seq_handShapes[i] = 0;
	}
}

void KyraEngine::seq_copyView() {
	debug(9, "KyraEngine::seq_copyView()");
	int y = 128;
	if (!_seq_copyViewOffs) {
		y -= 8;
	}
	_screen->copyRegion(0, 16, 0, 16, 320, y, 2, 0);
}

bool KyraEngine::seq_skipSequence() const {
	debug(9, "KyraEngine::seq_skipSequence()");
	return _quitFlag || _abortIntroFlag;
}

void KyraEngine::s1_wsaOpen() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < 12);
	uint8 offscreenDecode = *_seqData++;
	_seqWsaCurDecodePage = _seqMovies[wsaObj].page = (offscreenDecode == 0) ? 0 : 3;				
	if (_game == KYRA1DEMO) {
		_seqMovies[wsaObj].wsa = wsa_open(_seq_demo_WSATable[wsaObj], offscreenDecode, 0);
	} else {
		_seqMovies[wsaObj].wsa = wsa_open(_seq_WSATable[wsaObj], offscreenDecode, 0);
	}
	_seqMovies[wsaObj].frame = 0;
	_seqMovies[wsaObj].numFrames = wsa_getNumFrames(_seqMovies[wsaObj].wsa) - 1;
}

void KyraEngine::s1_wsaClose() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < 12);
	if (_seqMovies[wsaObj].wsa) {
		wsa_close(_seqMovies[wsaObj].wsa);
		_seqMovies[wsaObj].wsa = 0;
	}
}

void KyraEngine::s1_wsaPlayFrame() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < 12);
	int16 frame = (int8)*_seqData++;
	_seqMovies[wsaObj].pos.x = READ_LE_UINT16(_seqData); _seqData += 2;
	_seqMovies[wsaObj].pos.y = *_seqData++;
	wsa_play(_seqMovies[wsaObj].wsa, frame, _seqMovies[wsaObj].pos.x, _seqMovies[wsaObj].pos.y, _seqMovies[wsaObj].page);
	_seqMovies[wsaObj].frame = frame;
}

void KyraEngine::s1_wsaPlayNextFrame() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < 12);
	int16 frame = ++_seqMovies[wsaObj].frame;
	if (frame > _seqMovies[wsaObj].numFrames) {
		frame = 0;
		_seqMovies[wsaObj].frame = 0;
	}
	wsa_play(_seqMovies[wsaObj].wsa, frame, _seqMovies[wsaObj].pos.x, _seqMovies[wsaObj].pos.y, _seqMovies[wsaObj].page);
}

void KyraEngine::s1_wsaPlayPrevFrame() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < 12);
	int16 frame = --_seqMovies[wsaObj].frame;
	if (frame < 0) {
		frame = _seqMovies[wsaObj].numFrames;
		_seqMovies[wsaObj].frame = frame;
	} else {
		wsa_play(_seqMovies[wsaObj].wsa, frame, _seqMovies[wsaObj].pos.x, _seqMovies[wsaObj].pos.y, _seqMovies[wsaObj].page);
	}
}

void KyraEngine::s1_drawShape() {
	uint8 shapeNum = *_seqData++;
	int x = READ_LE_UINT16(_seqData); _seqData += 2;
	int y = *_seqData++;
	_screen->drawShape(2, _seq_handShapes[shapeNum], x, y, 0, 0, 0);
}

void KyraEngine::s1_maybeWaitTicks() {
	uint16 a = READ_LE_UINT16(_seqData); _seqData += 2;
	warning("STUB: s1_maybeWaitTicks(%d)\n", a);
}

void KyraEngine::s1_waitTicks() {
	uint16 ticks = READ_LE_UINT16(_seqData); _seqData += 2;
	waitTicks(ticks);
}

void KyraEngine::s1_copyWaitTicks() {
	seq_copyView();
	s1_waitTicks();
}

void KyraEngine::s1_shuffleScreen() {
	_screen->shuffleScreen(0, 16, 320, 128, 2, 0, 0, false);
	_screen->_curPage = 2;
}

void KyraEngine::s1_copyView() {
	seq_copyView();
}

void KyraEngine::s1_loopInit() {
	uint8 seqLoop = *_seqData++;
	if (seqLoop < 20) {
		_seqLoopTable[seqLoop].ptr = _seqData;
	} else {
		_seqQuitFlag = true;
	}
}

void KyraEngine::s1_maybeLoopInc() {
	uint8 a = *_seqData++;
	int16 b = (int16)READ_LE_UINT16(_seqData); _seqData += 2;
	warning("STUB: s1_maybeLoopInc(%d, %d)\n", a, b);
}

void KyraEngine::s1_loopInc() {
	uint8 seqLoop = *_seqData++;
	uint16 seqLoopCount = READ_LE_UINT16(_seqData); _seqData += 2;
	if (_seqLoopTable[seqLoop].count == 0xFFFF) {
		_seqLoopTable[seqLoop].count = seqLoopCount - 1;
		_seqData = _seqLoopTable[seqLoop].ptr;
	} else if (_seqLoopTable[seqLoop].count == 0) {
		_seqLoopTable[seqLoop].count = 0xFFFF;
		_seqLoopTable[seqLoop].ptr = 0;
	} else {
		--_seqLoopTable[seqLoop].count;
		_seqData = _seqLoopTable[seqLoop].ptr;
	}
}

void KyraEngine::s1_skip() {
	uint8 a = *_seqData++;
	warning("STUB: s1_skip(%d)\n", a);
}

void KyraEngine::s1_loadPalette() {
	uint8 colNum = *_seqData++;
	uint32 fileSize;
	uint8 *srcData;
	if (_game == KYRA1DEMO) {
		srcData = _res->fileData(_seq_demo_COLTable[colNum], &fileSize);
	} else {
		srcData = _res->fileData(_seq_COLTable[colNum], &fileSize);
	}
	memcpy(_screen->_currentPalette, srcData, fileSize);
	delete[] srcData;
}

void KyraEngine::s1_loadBitmap() {
	uint8 cpsNum = *_seqData++;
	loadBitmap(_seq_CPSTable[cpsNum], 3, 3, 0);
}

void KyraEngine::s1_fadeToBlack() {
	_screen->fadeToBlack();
}

void KyraEngine::s1_printText() {
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	uint8 txt = *_seqData++;
	_screen->fillRect(0, 180, 319, 195, 12);
	_screen->setTextColorMap(colorMap);
	if (!_seqDisplayTextFlag) {
		const char *str = _seq_textsTableEN[txt];
		int x = (Screen::SCREEN_W - _screen->getTextWidth(str)) / 2;
		_screen->printText(str, x, 180, 0xF, 0xC);
	} else {
		_seqDisplayedTextTimer = _system->getMillis() + 1000 / 60;
		_seqDisplayedText = txt;
		_seqDisplayedChar = 0;
		const char *str = _seq_textsTableEN[_seqDisplayedText];
		_seqDisplayedTextX = (Screen::SCREEN_W - _screen->getTextWidth(str)) / 2;
	}
}

void KyraEngine::s1_printTalkText() {
	uint8 txt = *_seqData++;
	int x = READ_LE_UINT16(_seqData); _seqData += 2;
	int y = *_seqData++;
	uint8 fillColor = *_seqData++;
	int b;
	if (_seqTalkTextPrinted && !_seqTalkTextRestored) {
		if (_seqWsaCurDecodePage != 0) {
			b = 2;
		} else {
			b = 0;
		}
		restoreTalkTextMessageBkgd(2, b);
	}
	_seqTalkTextPrinted = true;
	_seqTalkTextRestored = false;
	if (_seqWsaCurDecodePage != 0) {
		b = 2;
	} else {
		b = 0;
	}
	printTalkTextMessage(_seq_textsTableEN[txt], x, y, fillColor, b, 2);
}

void KyraEngine::s1_restoreTalkText() {
	if (_seqTalkTextPrinted && !_seqTalkTextRestored) {
		int b;
		if (_seqWsaCurDecodePage != 0) {
			b = 2;
		} else {
			b = 0;
		}
		restoreTalkTextMessageBkgd(2, b);
		_seqTalkTextRestored = true;
	}
}

void KyraEngine::s1_clearCurrentScreen() {
	_screen->fillRect(10, 180, 319, 196, 0xC);
}

void KyraEngine::s1_break() {
	// Do nothing
}

void KyraEngine::s1_fadeFromBlack() {
	_screen->fadeFromBlack();
}

void KyraEngine::s1_copyRegion() {
	uint8 srcPage = *_seqData++;
	uint8 dstPage = *_seqData++;
	_screen->copyRegion(0, 0, 0, 0, 320, 200, srcPage, dstPage);
}

void KyraEngine::s1_copyRegionSpecial() {
	static const uint8 colorMap[] = { 0, 0, 0, 0, 0, 12, 12, 0, 0, 0, 0, 0 };
	const char *copyStr = "Copyright (c) 1992 Westwood Studios";
	const int x = (Screen::SCREEN_W - _screen->getTextWidth(copyStr)) / 2;
	const int y = 179;

	uint8 so = *_seqData++;
	switch (so) {
	case 0:
		_screen->copyRegion(0, 0, 0, 47, 320, 77, 2, 0);
		break;
	case 1:
		_screen->copyRegion(0, 0, 0, 47, 320, 56, 2, 0);
		break;			
	case 2:
		_screen->copyRegion(107, 72, 107, 72, 43, 87, 2, 0);
		_screen->copyRegion(130, 159, 130, 159, 35, 17, 2, 0);
		_screen->copyRegion(165, 105, 165, 105, 32, 9, 2, 0);
		_screen->copyRegion(206, 83, 206, 83, 94, 93, 2, 0);
		break;
	case 3:
		_screen->copyRegion(152, 56, 152, 56, 48, 48, 2, 0);
		break;
	case 4:
		_screen->_charWidth = -2;
		_screen->setTextColorMap(colorMap);
		_screen->printText(copyStr, x + 1, y + 1, 0xB, 0xC);
		_screen->printText(copyStr, x, y, 0xF, 0xC);
		break;
	case 5:
		_screen->_curPage = 2;
		break;
	default:
		error("Invalid subopcode %d for s1_copyRegionSpecial", so);
		break;
	}
}

void KyraEngine::s1_fillRect() {
	int x1 = READ_LE_UINT16(_seqData); _seqData += 2;
	int y1 = *_seqData++;
	int x2 = READ_LE_UINT16(_seqData); _seqData += 2;
	int y2 = *_seqData++;
	uint8 color = *_seqData++;
	uint8 page = *_seqData++;
	_screen->fillRect(x1, y1, x2, y2, color, page);
}

void KyraEngine::s1_soundUnk1() {
	uint8 param = *_seqData++;
	waitTicks(3);
	snd_playSoundEffect(param);
}

void KyraEngine::s1_soundUnk2() {
	uint8 param = *_seqData++;
	snd_seqMessage(param);
}

void KyraEngine::s1_allocTempBuffer() {
	if (_game == KYRA1DEMO) {
		_seqQuitFlag = true;
	} else {
		// allocate offscreen buffer, not needed
	}
}

void KyraEngine::s1_textDisplayEnable() {
	_seqDisplayTextFlag = true;
}

void KyraEngine::s1_textDisplayDisable() {
	_seqDisplayTextFlag = false;
}

void KyraEngine::s1_endOfScript() {
	_seqQuitFlag = true;
}

void KyraEngine::s1_miscUnk1() {
	warning("STUB: s1_miscUnk1\n");
}

void KyraEngine::s1_miscUnk2() {
	uint8 a = *_seqData++;
	warning("STUB: s1_miscUnk2(%d)\n", a);
}

void KyraEngine::s1_miscUnk3() {
	warning("STUB: s1_miscUnk3\n");
}

void KyraEngine::s1_miscUnk4() {
	uint8 a = *_seqData++;
	warning("STUB: s1_miscUnk4(%d)\n", a);
}

bool KyraEngine::seq_playSpecialSequence(const uint8 *seqData, bool skipSeq) {
	static SeqEntry floppySeqProcs[] = {
		// 0x00
		SEQOP(3, s1_wsaOpen),
		SEQOP(2, s1_wsaClose),
		SEQOP(6, s1_wsaPlayFrame),
		SEQOP(2, s1_wsaPlayNextFrame),
		// 0x04
		SEQOP(2, s1_wsaPlayPrevFrame),
		SEQOP(5, s1_drawShape),
		SEQOP(3, s1_waitTicks),
		SEQOP(3, s1_copyWaitTicks),
		// 0x08
		SEQOP(1, s1_shuffleScreen),
		SEQOP(1, s1_copyView),
		SEQOP(2, s1_loopInit),
		SEQOP(4, s1_loopInc),
		// 0x0C
		SEQOP(2, s1_loadPalette),
		SEQOP(2, s1_loadBitmap),
		SEQOP(1, s1_fadeToBlack),
		SEQOP(2, s1_printText),
		// 0x10
		SEQOP(6, s1_printTalkText),
		SEQOP(1, s1_restoreTalkText),
		SEQOP(1, s1_clearCurrentScreen),
		SEQOP(1, s1_break),
		// 0x14
		SEQOP(1, s1_fadeFromBlack),
		SEQOP(3, s1_copyRegion),
		SEQOP(2, s1_copyRegionSpecial),
		SEQOP(9, s1_fillRect),
		// 0x18
		SEQOP(2, s1_soundUnk1),
		SEQOP(2, s1_soundUnk2),
		SEQOP(1, s1_allocTempBuffer),
		SEQOP(1, s1_textDisplayEnable),
		// 0x1C
		SEQOP(1, s1_textDisplayDisable),
		SEQOP(1, s1_endOfScript)
	};

#if 0
	static SeqEntry cdromSeqProcs[] = {
		// 0x00
		SEQOP(3, s1_wsaOpen),
		SEQOP(2, s1_wsaClose),
		SEQOP(6, s1_wsaPlayFrame),
		SEQOP(2, s1_wsaPlayNextFrame),
		// 0x04
		SEQOP(2, s1_wsaPlayPrevFrame),
		SEQOP(5, s1_drawShape),
		SEQOP(3, s1_maybeWaitTicks),
		SEQOP(3, s1_waitTicks),
		// 0x08
		SEQOP(3, s1_copyWaitTicks),
		SEQOP(1, s1_shuffleScreen),
		SEQOP(1, s1_copyView),
		SEQOP(2, s1_loopInit),
		// 0x0C
		SEQOP(4, s1_maybeLoopInc),
		SEQOP(4, s1_maybeLoopInc), // Again?
		SEQOP(2, s1_skip),
		SEQOP(2, s1_loadPalette),
		// 0x10
		SEQOP(2, s1_loadBitmap),
		SEQOP(1, s1_fadeToBlack),
		SEQOP(2, s1_printText),
		SEQOP(6, s1_printTalkText),
		// 0x14
		SEQOP(1, s1_restoreTalkText),
		SEQOP(1, s1_clearCurrentScreen),
		SEQOP(1, s1_break),
		SEQOP(1, s1_fadeFromBlack),
		// 0x18
		SEQOP(3, s1_copyRegion),
		SEQOP(2, s1_copyRegionSpecial),
		SEQOP(9, s1_fillRect),
		SEQOP(2, s1_soundUnk1),
		// 0x1C
		SEQOP(2, s1_soundUnk2),
		SEQOP(1, s1_allocTempBuffer),
		SEQOP(1, s1_textDisplayEnable),
		SEQOP(1, s1_textDisplayDisable),
		// 0x20
		SEQOP(1, s1_endOfScript),
		SEQOP(1, s1_miscUnk1),
		SEQOP(2, s1_miscUnk2),
		SEQOP(1, s1_miscUnk3),
		// 0x24
		SEQOP(2, s1_miscUnk4)
	};
#endif

	const SeqEntry* commands;
	int numCommands;

	debug(9, "KyraEngine::seq_playSpecialSequence(0x%X, %d)", seqData, skipSeq);

	commands = floppySeqProcs;
	numCommands = ARRAYSIZE(floppySeqProcs);

	bool seqSkippedFlag = false;

	_seqData = seqData;

	_seqDisplayedTextTimer = 0xFFFFFFFF;
	_seqDisplayTextFlag = false;
	_seqDisplayedTextX = 0;
	_seqDisplayedText = 0;
	_seqDisplayedChar = 0;
	_seqTalkTextRestored = false;
	_seqTalkTextPrinted = false;

	_seqQuitFlag = false;
	_seqWsaCurDecodePage = 0;

	for (int i = 0; i < 20; ++i) {
		_seqLoopTable[i].ptr = 0;
		_seqLoopTable[i].count = 0xFFFF;
	}

	memset(_seqMovies, 0, sizeof(_seqMovies));

	_screen->_curPage = 0;
	while (!_seqQuitFlag) {
		if (skipSeq && seq_skipSequence()) {
			while (1) {
				uint8 code = *_seqData;
				if (commands[code].proc == &KyraEngine::s1_endOfScript || commands[code].proc == &KyraEngine::s1_break) {
					break;
				}
				_seqData += commands[code].len;
			}
			skipSeq = false;
			seqSkippedFlag = true;
		}
		// used in Kallak writing intro
		if (_seqDisplayTextFlag && _seqDisplayedTextTimer != 0xFFFFFFFF) {
			if (_seqDisplayedTextTimer < _system->getMillis()) {
				char charStr[2];
				charStr[0] = _seq_textsTableEN[_seqDisplayedText][_seqDisplayedChar];
				charStr[1] = '\0';
				_screen->printText(charStr, _seqDisplayedTextX, 180, 0xF, 0xC);
				_seqDisplayedTextX += _screen->getCharWidth(charStr[0]);
				++_seqDisplayedChar;
				if (_seq_textsTableEN[_seqDisplayedText][_seqDisplayedChar] == '\0') {
					_seqDisplayedTextTimer = 0xFFFFFFFF;
				}
			} else {
				_seqDisplayedTextTimer = _system->getMillis() + 1000 / 60;
			}
		}

		uint8 seqCode = *_seqData++;
		if (seqCode < numCommands) {
			SeqProc currentProc = commands[seqCode].proc;
			debug(5, "seqCode = %d (%s)", seqCode, commands[seqCode].desc);
			(this->*currentProc)();
		} else {
			error("Invalid sequence opcode %d", seqCode);
		}

		_screen->updateScreen();
	}
	return seqSkippedFlag;
}

void KyraEngine::snd_playTheme(int file, int track) {
	debug(9, "KyraEngine::snd_playTheme(%d)", file);
	assert(file < _xmidiFilesCount);
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

void KyraEngine::snd_startTrack() {
	debug(9, "KyraEngine::snd_startTrack()");
	_midi->startTrack();
}

void KyraEngine::snd_haltTrack() {
	debug(9, "KyraEngine::snd_haltTrack()");
	_midi->haltTrack();
}

void KyraEngine::snd_seqMessage(int msg) {
	debug(9, "KyraEngine::snd_seqMessage(%.02d)", msg);
	switch (msg) {
	case 0:
		// nothing to do here...
		break;
	case 1:
		_midi->beginFadeOut();
		break;
	case 56:
		snd_playTheme(MUSIC_INTRO, 3);
		break;
	case 57:
		snd_playTheme(MUSIC_INTRO, 4);
		break;
	case 58:
		snd_playTheme(MUSIC_INTRO, 5);
		break;
	default:
		warning("Unknown seq. message: %.02d", msg);
		break;
	}
}

} // End of namespace Kyra
