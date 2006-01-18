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

#define TEST_SPRITES 1

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
	{ "kyra1", "Legend of Kyrandia (Floppy, English)",	GI_KYRA1, GF_ENGLISH | GF_FLOPPY, 
										"796e44863dd22fa635b042df1bf16673", "GEMCUT.EMC" },
	{ "kyra1", "Legend of Kyrandia (Floppy, French)",	GI_KYRA1, GF_FRENCH  | GF_FLOPPY,
										"abf8eb360e79a6c2a837751fbd4d3d24", "GEMCUT.EMC" },
	{ "kyra1", "Legend of Kyrandia (Floppy, German)",	GI_KYRA1, GF_GERMAN  | GF_FLOPPY, 
										"6018e1dfeaca7fe83f8d0b00eb0dd049", "GEMCUT.EMC"},
	{ "kyra1", "Legend of Kyrandia (CD, English)",		GI_KYRA1, GF_ENGLISH | GF_TALKIE, 
										"fac399fe62f98671e56a005c5e94e39f", "GEMCUT.PAK" },
	{ "kyra1", "Legend of Kyrandia (CD, German)",		GI_KYRA1, GF_GERMAN | GF_TALKIE, 
										"230f54e6afc007ab4117159181a1c722", "GEMCUT.PAK" },
	{ "kyra1", "Legend of Kyrandia (CD, French)",		GI_KYRA1, GF_FRENCH | GF_TALKIE, 
										"b037c41768b652a040360ffa3556fd2a", "GEMCUT.PAK" },
	{ "kyra1", "Legend of Kyrandia (Demo)",			GI_KYRA1, GF_DEMO,
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
	{ "kyra1", "Legend of Kyrandia (Unknown)", 0 },
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
		// a bit hacky but should work fine for now
		debug("Assuming an english floppy version for now");
		_features = GF_FLOPPY | GF_ENGLISH;
		_game = GI_KYRA1;
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
	delete _seq;
}

void KyraEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);
}

int KyraEngine::go() {
	_quitFlag = false;
	uint32 sz;

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
	if (_features & GF_ENGLISH && _features & GF_TALKIE) 
		loadBitmap("MAIN_ENG.CPS", 10, 10, 0);
	else if(_features & GF_FRENCH)
		loadBitmap("MAIN_FRE.CPS", 10, 10, 0);
	else if(_features & GF_GERMAN)
		loadBitmap("MAIN_GER.CPS", 10, 10, 0);
	else
		loadBitmap("MAIN15.CPS", 10, 10, 0);

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
	
	_seq->playSequence(_seq_demoData_WestwoodLogo, true);
	waitTicks(60);

	_seq->playSequence(_seq_demoData_KyrandiaLogo, true);

	_screen->fadeToBlack();
	_screen->clearPage(2);
	_screen->clearPage(0);

	_seq->playSequence(_seq_demoData_Demo1, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_demoData_Demo2, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_demoData_Demo3, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_demoData_Demo4, true);

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
		&KyraEngine::seq_introMalcomTree,
		&KyraEngine::seq_introKallakWriting,
		&KyraEngine::seq_introKallakMalcom
	};
	_skipIntroFlag = true; // only true if user already played the game once
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
	
	if (_features & GF_FLOPPY) {
		if (_seq->playSequence(_seq_floppyData_WestwoodLogo, _skipIntroFlag)) {
			_screen->fadeToBlack();
			_screen->clearPage(0);
			return;
		}
		waitTicks(60);
		if (_seq->playSequence(_seq_floppyData_KyrandiaLogo, _skipIntroFlag)) {
			_screen->fadeToBlack();
			_screen->clearPage(0);
			return;
		}
	} else if (_features & GF_TALKIE) {
		if (_seq->playSequence(_seq_cdromData_WestwoodLogo, _skipIntroFlag)) {
			_screen->fadeToBlack();
			_screen->clearPage(0);
			return;
		}
		waitTicks(60);
		if (_seq->playSequence(_seq_cdromData_KyrandiaLogo, _skipIntroFlag)) {
			_screen->fadeToBlack();
			_screen->clearPage(0);
			return;
		}
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

	if (_features & GF_FLOPPY) {
		_seq->playSequence(_seq_floppyData_Forest, true);
	} else if (_features & GF_TALKIE) {
		_seq->playSequence(_seq_cdromData_Forest, true);
	}
}

void KyraEngine::seq_introStory() {
	debug(9, "KyraEngine::seq_introStory()");
	// this is only needed for floppy versions
	// since CD version has its own opcode for that
	if (_features & GF_FLOPPY) {
		_screen->clearPage(3);
		_screen->clearPage(0);
		if (_features & GF_ENGLISH) {
			loadBitmap("TEXT.CPS", 3, 3, 0);
		} else if (_features & GF_GERMAN) {
			loadBitmap("TEXT_GER.CPS", 3, 3, 0);
		} else if (_features & GF_FRENCH) {
			loadBitmap("TEXT_FRE.CPS", 3, 3, 0);
		}
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 3, 0);
		_screen->updateScreen();
		waitTicks(360);
	}
}

void KyraEngine::seq_introMalcomTree() {
	debug(9, "KyraEngine::seq_introMalcomTree()");
	_screen->_curPage = 0;
	_screen->clearPage(3);
	if (_features & GF_FLOPPY) {
		_seq->playSequence(_seq_floppyData_MalcomTree, true);
	} else if (_features & GF_TALKIE) {
		_seq->playSequence(_seq_cdromData_MalcomTree, true);
	}
}

void KyraEngine::seq_introKallakWriting() {
	debug(9, "KyraEngine::seq_introKallakWriting()");
	_seq->makeHandShapes();
	_screen->setAnimBlockPtr(5060);
	_screen->_charWidth = -2;
	_screen->clearPage(3);
	if (_features & GF_FLOPPY) {
		_seq->playSequence(_seq_floppyData_KallakWriting, true);
	} else if (_features & GF_TALKIE) {
		_seq->playSequence(_seq_cdromData_KallakWriting, true);
	}
	_seq->freeHandShapes();
}

void KyraEngine::seq_introKallakMalcom() {
	debug(9, "KyraEngine::seq_introKallakMalcom()");
	_screen->clearPage(3);
	if (_features & GF_FLOPPY) {
		_seq->playSequence(_seq_floppyData_KallakMalcom, true);
	} else if (_features & GF_TALKIE) {
		_seq->playSequence(_seq_cdromData_KallakMalcom, true);
	}
}

bool KyraEngine::seq_skipSequence() const {
	debug(9, "KyraEngine::seq_skipSequence()");
	return _quitFlag || _abortIntroFlag;
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

} // End of namespace Kyra
