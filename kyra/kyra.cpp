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
#include "kyra/wsamovie.h"

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

	int midiDrv = MidiDriver::detectMusicDriver(MDT_NATIVE | MDT_ADLIB | MDT_PREFER_NATIVE);
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
	memset(_flagsTable, 0, sizeof(_flagsTable));
	// XXX
}

void KyraEngine::mainLoop() {
	debug(9, "KyraEngine::mainLoop()");
	// XXX
}

void KyraEngine::loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData) {
	debug(9, "KyraEngine::copyBitmap('%s', %d, %d, 0x%X)", filename, tempPage, dstPage, palData);
	uint32 fileSize;
	uint8 *srcData = _res->fileData(filename, &fileSize);
	uint8 compType = srcData[2];
	uint32 imgSize = READ_LE_UINT32(srcData + 4);
	uint16 palSize = READ_LE_UINT16(srcData + 8);
	if (palData && palSize) {
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

	// TODO: Display START.CPS

	_screen->clearPage(0);
	loadBitmap("TOP.CPS", 7, 7, NULL);
	loadBitmap("BOTTOM.CPS", 5, 5, _screen->_currentPalette);
	_screen->_curPage = 0;
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

	// TODO: Display FINAL.CPS
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
//	snd_kyraPlayTheme(0);
	setTalkCoords(144);
	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		(this->*introProcTable[i])();
	}
	setTalkCoords(136);
	waitTicks(0x1E);
	_seq_copyViewOffs = false;
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
	return _quitFlag;
}

bool KyraEngine::seq_playSpecialSequence(const uint8 *seqData, bool skipSeq) {
	debug(9, "KyraEngine::seq_playSpecialSequence(0x%X, %d)", seqData, skipSeq);
	WSAMovieV1 *wsaMovieTable[12];
	uint32 displayedTextTimer = 0xFFFFFFFF;
	bool displayTextFlag = false;
	bool seqSkippedFlag = false;
	uint16 displayedTextX = 0;
	uint8 displayedText = 0;
	uint8 displayedChar = 0;
	int16 wsaCurFramesTable[12];
	uint16 wsaYPosTable[12];
	uint16 wsaXPosTable[12];
	bool talkTextRestored = false;
	bool talkTextPrinted = false;
	int16 wsaNumFramesTable[12];
	bool quitFlag = false;
	uint16 wsaCurDecodePage = 0;
	uint32 wsaDecodePage[12];
	SeqLoop seqLoopTable[20];
	
	for (int i = 0; i < 20; ++i) {
		seqLoopTable[i].ptr = 0;
		seqLoopTable[i].count = 0xFFFF;
	}
	memset(wsaMovieTable, 0, sizeof(wsaMovieTable));

	_screen->_curPage = 0;
	while (!quitFlag) {
		if (skipSeq && seq_skipSequence()) {
			while (1) {
				uint8 code = *seqData;
				if (code == 29 || code == 19) {
					break;
				}
				seqData += _seq_codeSizeTable[code] + 1;
			}
			skipSeq = false;
			seqSkippedFlag = true;
		}
		// used in Kallak writing intro
		if (displayTextFlag && displayedTextTimer != 0xFFFFFFFF) {
			if (displayedTextTimer < _system->getMillis()) {
				char charStr[2];
				charStr[0] = _seq_textsTableEN[displayedText][displayedChar];
				charStr[1] = '\0';
				_screen->printText(charStr, displayedTextX, 180, 0xF, 0xC);
				displayedTextX += _screen->getCharWidth(charStr[0]);
				++displayedChar;
				if (_seq_textsTableEN[displayedText][displayedChar] == '\0') {
					displayedTextTimer = 0xFFFFFFFF;
				}
			} else {
				displayedTextTimer = _system->getMillis() + 1000 / 60;
			}
		}
		uint8 seqCode = *seqData++;
		debug(5, "seqCode = %d", seqCode);
		switch (seqCode) {
		case 0: {
				uint8 wsaObj = *seqData++;
				assert(wsaObj < 12);
				uint8 offscreenDecode = *seqData++;
				wsaCurDecodePage = wsaDecodePage[wsaObj] = (offscreenDecode == 0) ? 0 : 3;				
				if (_game == KYRA1DEMO) {
					wsaMovieTable[wsaObj] = wsa_open(_seq_demo_WSATable[wsaObj], offscreenDecode, 0);
				} else {
					wsaMovieTable[wsaObj] = wsa_open(_seq_WSATable[wsaObj], offscreenDecode, 0);
				}
				wsaCurFramesTable[wsaObj] = 0;
				wsaNumFramesTable[wsaObj] = wsa_getNumFrames(wsaMovieTable[wsaObj]) - 1;
			}
			break;
		case 1: {
				uint8 wsaObj = *seqData++;
				assert(wsaObj < 12);
				if (wsaMovieTable[wsaObj]) {
					wsa_close(wsaMovieTable[wsaObj]);
					wsaMovieTable[wsaObj] = 0;
				}
			}
			break;
		case 2: {
				uint8 wsaObj = *seqData++;
				assert(wsaObj < 12);
				int16 frame = (int8)*seqData++;
				wsaXPosTable[wsaObj] = READ_LE_UINT16(seqData); seqData += 2;
				wsaYPosTable[wsaObj] = *seqData++;
				wsa_play(wsaMovieTable[wsaObj], frame, wsaXPosTable[wsaObj], wsaYPosTable[wsaObj], wsaDecodePage[wsaObj]);
				wsaCurFramesTable[wsaObj] = frame;
			}
			break;
		case 3: {
				uint8 wsaObj = *seqData++;
				assert(wsaObj < 12);
				++wsaCurFramesTable[wsaObj];
				int16 frame = wsaCurFramesTable[wsaObj];
				if (frame > wsaNumFramesTable[wsaObj]) {
					frame = 0;
					wsaCurFramesTable[wsaObj] = 0;
				}
				wsa_play(wsaMovieTable[wsaObj], frame, wsaXPosTable[wsaObj], wsaYPosTable[wsaObj], wsaDecodePage[wsaObj]);
			}
			break;
		case 4: {
				uint8 wsaObj = *seqData++;
				assert(wsaObj < 12);
				--wsaCurFramesTable[wsaObj];
				int16 frame = wsaCurFramesTable[wsaObj];
				if (wsaCurFramesTable[wsaObj] < 0) {
					frame = wsaNumFramesTable[wsaObj];
					wsaCurFramesTable[wsaObj] = wsaNumFramesTable[wsaObj];
				} else {
					wsa_play(wsaMovieTable[wsaObj], frame, wsaXPosTable[wsaObj], wsaYPosTable[wsaObj], wsaDecodePage[wsaObj]);
				}
			}
			break;
		case 5: {
				uint8 shapeNum = *seqData++;
				int x = READ_LE_UINT16(seqData); seqData += 2;
				int y = *seqData++;
				_screen->drawShape(2, _seq_handShapes[shapeNum], x, y, 0, 0, 0);
			}
			break;
		case 6:
		case 7: {
				if (seqCode == 7) {
					seq_copyView();
				}
				uint16 ticks = READ_LE_UINT16(seqData); seqData += 2;
				waitTicks(ticks);
			}
			break;
		case 8:
			_screen->shuffleScreen(0, 16, 320, 128, 2, 0, 0, false);
			_screen->_curPage = 2;
			break;
		case 9:
			seq_copyView();
			break;
		case 10: {
				uint8 seqLoop = *seqData++;
				if (seqLoop < 20) {
					seqLoopTable[seqLoop].ptr = seqData;
				} else {
					quitFlag = true;
				}
			}
			break;
		case 11: {
				uint8 seqLoop = *seqData++;
				uint16 seqLoopCount = READ_LE_UINT16(seqData); seqData += 2;
				if (seqLoopTable[seqLoop].count == 0xFFFF) {
					seqLoopTable[seqLoop].count = seqLoopCount - 1;
					seqData = seqLoopTable[seqLoop].ptr;
				} else if (seqLoopTable[seqLoop].count == 0) {
					seqLoopTable[seqLoop].count = 0xFFFF;
					seqLoopTable[seqLoop].ptr = 0;
				} else {
					--seqLoopTable[seqLoop].count;
					seqData = seqLoopTable[seqLoop].ptr;
				}
			}
			break;
		case 12: {
				uint8 colNum = *seqData++;
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
			break;
		case 13: {
				uint8 cpsNum = *seqData++;
				loadBitmap(_seq_CPSTable[cpsNum], 3, 3, 0);
			}
			break;
		case 14:
			_screen->fadeToBlack();
			break;
		case 15: {
				static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
				uint8 txt = *seqData++;
				_screen->fillRect(0, 180, 319, 195, 12);
				_screen->setTextColorMap(colorMap);
				if (!displayTextFlag) {
					const char *str = _seq_textsTableEN[txt];
					int x = (Screen::SCREEN_W - _screen->getTextWidth(str)) / 2;
					_screen->printText(str, x, 180, 0xF, 0xC);
				} else {
					displayedTextTimer = _system->getMillis() + 1000 / 60;
					displayedText = txt;
					displayedChar = 0;
					const char *str = _seq_textsTableEN[displayedText];
					displayedTextX = (Screen::SCREEN_W - _screen->getTextWidth(str)) / 2;
				}
			}
			break;
		case 16: {
				uint8 txt = *seqData++;
				int x = READ_LE_UINT16(seqData); seqData += 2;
				int y = *seqData++;
				uint8 fillColor = *seqData++;
				int b;
				if (talkTextPrinted && !talkTextRestored) {
					if (wsaCurDecodePage != 0) {
						b = 2;
					} else {
						b = 0;
					}
					restoreTalkTextMessageBkgd(2, b);
				}
				talkTextPrinted = true;
				talkTextRestored = false;
				if (wsaCurDecodePage != 0) {
					b = 2;
				} else {
					b = 0;
				}
				printTalkTextMessage(_seq_textsTableEN[txt], x, y, fillColor, b, 2);
			}
			break;
		case 17:
			if (talkTextPrinted && !talkTextRestored) {
				int b;
				if (wsaCurDecodePage != 0) {
					b = 2;
				} else {
					b = 0;
				}
				restoreTalkTextMessageBkgd(2, b);
				talkTextRestored = true;
			}
			break;
		case 18:
			_screen->fillRect(10, 180, 319, 196, 0xC);
			break;
		case 19:
			break;
		case 20:
			_screen->fadeFromBlack();
			break;
		case 21: {
				uint8 srcPage = *seqData++;
				uint8 dstPage = *seqData++;
				_screen->copyRegion(0, 0, 0, 0, 320, 200, srcPage, dstPage);
			}
			break;
		case 22: {
				uint8 so = *seqData++;
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
				case 4: {
						static const uint8 colorMap[] = { 0, 0, 0, 0, 0, 12, 12, 0, 0, 0, 0, 0 };
						_screen->_charWidth = -2;
						const char *copyStr = "Copyright (c) 1992 Westwood Studios";
						_screen->setTextColorMap(colorMap);
						const int x = (Screen::SCREEN_W - _screen->getTextWidth(copyStr)) / 2;
						const int y = 179;
						_screen->printText(copyStr, x + 1, y + 1, 0xB, 0xC);
						_screen->printText(copyStr, x, y, 0xF, 0xC);
					}
					break;
				case 5:
					_screen->_curPage = 2;
					break;
				default:
					error("Invalid subopcode %d for sequence opcode 22", so);
					break;
				}
			}
			break;
		case 23: {
				int x1 = READ_LE_UINT16(seqData); seqData += 2;
				int y1 = *seqData++;
				int x2 = READ_LE_UINT16(seqData); seqData += 2;
				int y2 = *seqData++;
				uint8 color = *seqData++;
				uint8 page = *seqData++;
				_screen->fillRect(x1, y1, x2, y2, color, page);
			}
			break;
		case 24: { // sound related
				seqData++;
				warning("Sequence opcode 24 skipped");
			}
			break;
		case 25: { // sound related
				seqData++;
				warning("Sequence opcode 25 skipped");
			}
			break;
		case 26:
			if (_game == KYRA1DEMO) {
				quitFlag = true;
			} else {
				// allocate offscreen buffer, not needed
			}
			break;
		case 27:
			displayTextFlag = true;
			break;
		case 28:
			displayTextFlag = false;
			break;
		case 29:
			quitFlag = true;
			break;
		default:
			error("Invalid sequence opcode %d", seqCode);
			break;	
		}
		_screen->updateScreen();
	}
	return seqSkippedFlag;
}

} // End of namespace Kyra
