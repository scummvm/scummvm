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
#include "common/system.h"

#include "base/engine.h"

#include "kyra/kyra.h"
#include "kyra/resource.h"
#include "kyra/screen.h"
#include "kyra/sound.h"
#include "kyra/wsamovie.h"

#include "kyra/seqplayer.h"

#define SEQOP(n, x) { n, &SeqPlayer::x, #x }

namespace Kyra {

SeqPlayer::SeqPlayer(KyraEngine* vm, OSystem* system) {
	_vm = vm;
	_system = system;

	_screen = vm->screen();
	_midi = vm->midi();
	_res = vm->resource();

	_copyViewOffs = false;

	for (int i = 0; i < ARRAYSIZE(_handShapes); ++i)
		_handShapes[i] = 0;
}

SeqPlayer::~SeqPlayer() {
	freeHandShapes();

	for (int i = 0; i < ARRAYSIZE(_seqMovies); ++i) {
		_seqMovies[i].movie->close();
		delete _seqMovies[i].movie;
		_seqMovies[i].movie = 0;
	}
}

uint8 *SeqPlayer::setPanPages(int pageNum, int shape) {
	debug(9, "SeqPlayer::setPanPages(%d, %d)", pageNum, shape);
	uint8 *panPage = 0;
	const uint8 *data = _screen->getPagePtr(pageNum);
	uint16 numShapes = READ_LE_UINT16(data);
	if (shape < numShapes) {
		uint32 offs = 0;
		if (_vm->features() & GF_TALKIE) {
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

void SeqPlayer::makeHandShapes() {
	debug(9, "SeqPlayer::makeHandShapes()");
	_vm->loadBitmap("WRITING.CPS", 3, 3, 0);
	for (int i = 0; i < ARRAYSIZE(_handShapes); ++i) {
		if (_handShapes[i])
			free(_handShapes[i]);
		_handShapes[i] = setPanPages(3, i);
	}
}

void SeqPlayer::freeHandShapes() {
	debug(9, "SeqPlayer::freeHandShapes()");
	for (int i = 0; i < ARRAYSIZE(_handShapes); ++i) {
		free(_handShapes[i]);
		_handShapes[i] = 0;
	}
}

void SeqPlayer::s1_wsaOpen() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < ARRAYSIZE(_seqMovies));
	uint8 offscreenDecode = *_seqData++;
	_seqWsaCurDecodePage = _seqMovies[wsaObj].page = (offscreenDecode == 0) ? 0 : 3;
	if (!_seqMovies[wsaObj].movie)
		_seqMovies[wsaObj].movie = _vm->createWSAMovie();
	_seqMovies[wsaObj].movie->_drawPage = _seqMovies[wsaObj].page;
	_seqMovies[wsaObj].movie->open(_vm->seqWSATable()[wsaObj], offscreenDecode, 0);
	_seqMovies[wsaObj].frame = 0;
	_seqMovies[wsaObj].numFrames = _seqMovies[wsaObj].movie->frames() - 1;
}

void SeqPlayer::s1_wsaClose() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < ARRAYSIZE(_seqMovies));
	if (_seqMovies[wsaObj].movie) {
		_seqMovies[wsaObj].movie->close();
	}
}

void SeqPlayer::s1_wsaPlayFrame() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < ARRAYSIZE(_seqMovies));
	int16 frame = (int8)*_seqData++;
	_seqMovies[wsaObj].pos.x = READ_LE_UINT16(_seqData); _seqData += 2;
	_seqMovies[wsaObj].pos.y = *_seqData++;
	assert(_seqMovies[wsaObj].movie);
	_seqMovies[wsaObj].movie->_x = _seqMovies[wsaObj].pos.x;
	_seqMovies[wsaObj].movie->_y = _seqMovies[wsaObj].pos.y;
	_seqMovies[wsaObj].movie->displayFrame(frame);
	_seqMovies[wsaObj].frame = frame;
}

void SeqPlayer::s1_wsaPlayNextFrame() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < ARRAYSIZE(_seqMovies));
	int16 frame = ++_seqMovies[wsaObj].frame;
	if (frame > _seqMovies[wsaObj].numFrames) {
		frame = 0;
		_seqMovies[wsaObj].frame = 0;
	}
	_seqMovies[wsaObj].movie->displayFrame(frame);
}

void SeqPlayer::s1_wsaPlayPrevFrame() {
	uint8 wsaObj = *_seqData++;
	assert(wsaObj < ARRAYSIZE(_seqMovies));
	int16 frame = --_seqMovies[wsaObj].frame;
	if (frame < 0) {
		frame = _seqMovies[wsaObj].numFrames;
		_seqMovies[wsaObj].frame = frame;
	} else {
		_seqMovies[wsaObj].movie->displayFrame(frame);
	}
}

void SeqPlayer::s1_drawShape() {
	uint8 shapeNum = *_seqData++;
	int x = READ_LE_UINT16(_seqData); _seqData += 2;
	int y = *_seqData++;
	_screen->drawShape(2, _handShapes[shapeNum], x, y, 0, 0, 0);
}

void SeqPlayer::s1_waitTicks() {
	uint16 ticks = READ_LE_UINT16(_seqData); _seqData += 2;
	_vm->waitTicks(ticks);
}

void SeqPlayer::s1_copyWaitTicks() {
	s1_copyView();
	s1_waitTicks();
}

void SeqPlayer::s1_shuffleScreen() {
	_screen->shuffleScreen(0, 16, 320, 128, 2, 0, 0, false);
	_screen->_curPage = 2;
}

void SeqPlayer::s1_copyView() {
	int y = 128;
	if (!_copyViewOffs) {
		y -= 8;
	}
	_screen->copyRegion(0, 16, 0, 16, 320, y, 2, 0);
}

void SeqPlayer::s1_loopInit() {
	uint8 seqLoop = *_seqData++;
	if (seqLoop < ARRAYSIZE(_seqLoopTable)) {
		_seqLoopTable[seqLoop].ptr = _seqData;
	} else {
		_seqQuitFlag = true;
	}
}

void SeqPlayer::s1_loopInc() {
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

void SeqPlayer::s1_skip() {
	uint8 a = *_seqData++;
	warning("STUB: s1_skip(%d)\n", a);
}

void SeqPlayer::s1_loadPalette() {
	uint8 colNum = *_seqData++;
	uint32 fileSize;
	uint8 *srcData;
	srcData = _res->fileData(_vm->seqCOLTable()[colNum], &fileSize);
	memcpy(_screen->_currentPalette, srcData, fileSize);
	delete[] srcData;
}

void SeqPlayer::s1_loadBitmap() {
	uint8 cpsNum = *_seqData++;
	_vm->loadBitmap(_vm->seqCPSTable()[cpsNum], 3, 3, 0);
}

void SeqPlayer::s1_fadeToBlack() {
	_screen->fadeToBlack();
}

void SeqPlayer::s1_printText() {
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	uint8 txt = *_seqData++;
	_screen->fillRect(0, 180, 319, 195, 12);
	_screen->setTextColorMap(colorMap);
	if (!_seqDisplayTextFlag) {
		const char *str = _vm->seqTextsTable()[txt];
		int x = (Screen::SCREEN_W - _screen->getTextWidth(str)) / 2;
		_screen->printText(str, x, 180, 0xF, 0xC);
	} else {
		_seqDisplayedTextTimer = _system->getMillis() + 1000 / ((_vm->features() & GF_FRENCH) ? 120 : 60);
		_seqDisplayedText = txt;
		_seqDisplayedChar = 0;
		const char *str = _vm->seqTextsTable()[_seqDisplayedText];
		_seqDisplayedTextX = (Screen::SCREEN_W - _screen->getTextWidth(str)) / 2;
	}
}

void SeqPlayer::s1_printTalkText() {
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
		_vm->restoreTalkTextMessageBkgd(2, b);
	}
	_seqTalkTextPrinted = true;
	_seqTalkTextRestored = false;
	if (_seqWsaCurDecodePage != 0) {
		b = 2;
	} else {
		b = 0;
	}
	_vm->printTalkTextMessage(_vm->seqTextsTable()[txt], x, y, fillColor, b, 2);
}

void SeqPlayer::s1_restoreTalkText() {
	if (_seqTalkTextPrinted && !_seqTalkTextRestored) {
		int b;
		if (_seqWsaCurDecodePage != 0) {
			b = 2;
		} else {
			b = 0;
		}
		_vm->restoreTalkTextMessageBkgd(2, b);
		_seqTalkTextRestored = true;
	}
}

void SeqPlayer::s1_clearCurrentScreen() {
	_screen->fillRect(10, 180, 319, 196, 0xC);
}

void SeqPlayer::s1_break() {
	// Do nothing
}

void SeqPlayer::s1_fadeFromBlack() {
	_screen->fadeFromBlack();
}

void SeqPlayer::s1_copyRegion() {
	uint8 srcPage = *_seqData++;
	uint8 dstPage = *_seqData++;
	_screen->copyRegion(0, 0, 0, 0, 320, 200, srcPage, dstPage);
}

void SeqPlayer::s1_copyRegionSpecial() {
	static const uint8 colorMap[] = { 0, 0, 0, 0, 0, 12, 12, 0, 0, 0, 0, 0 };
	const char *copyStr = 0;
	if (_vm->features() & GF_FLOPPY || _vm->features() & GF_DEMO) {
		copyStr = "Copyright (c) 1992 Westwood Studios";
	} else if (_vm->features() & GF_TALKIE) {
		copyStr = "Copyright (c) 1992,1993 Westwood Studios";
	}
	
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
	case 4: {
		_screen->_charWidth = -2;
		const int x = (Screen::SCREEN_W - _screen->getTextWidth(copyStr)) / 2;
		const int y = 179;
		_screen->setTextColorMap(colorMap);
		_screen->printText(copyStr, x + 1, y + 1, 0xB, 0xC);
		_screen->printText(copyStr, x, y, 0xF, 0xC);
	}	break;
	case 5:
		_screen->_curPage = 2;
		break;
	default:
		error("Invalid subopcode %d for s1_copyRegionSpecial", so);
		break;
	}
}

void SeqPlayer::s1_fillRect() {
	int x1 = READ_LE_UINT16(_seqData); _seqData += 2;
	int y1 = *_seqData++;
	int x2 = READ_LE_UINT16(_seqData); _seqData += 2;
	int y2 = *_seqData++;
	uint8 color = *_seqData++;
	uint8 page = *_seqData++;
	_screen->fillRect(x1, y1, x2, y2, color, page);
}

void SeqPlayer::s1_playEffect() {
	uint8 track = *_seqData++;
	_vm->waitTicks(3);
	_midi->playSoundEffect(track);
}

void SeqPlayer::s1_playTrack() {
	uint8 msg = *_seqData++;
/*	
	// we do not have audio cd support for now
	if (_vm->features() & GF_AUDIOCD) {
		switch (msg) {
		case 0:
			// nothing to do here...
			break;
		case 1:
			_midi->beginFadeOut();
			break;
		case 56:
			_vm->snd_playTheme(KyraEngine::MUSIC_INTRO, 3);
			break;
		case 57:
			_vm->snd_playTheme(KyraEngine::MUSIC_INTRO, 4);
			break;
		case 58:
			_vm->snd_playTheme(KyraEngine::MUSIC_INTRO, 5);
			break;
		default:
			warning("Unknown seq. message: %.02d", msg);
			break;
		}
	} else {*/
		if (msg == 0) {
			// nothing to do here...
		} else if (msg == 1) {
			_midi->beginFadeOut();
		} else {
			_vm->snd_playTrack(msg);
		}
//	}
}

void SeqPlayer::s1_allocTempBuffer() {
	if (_vm->features() & GF_DEMO) {
		_seqQuitFlag = true;
	} else {
		// allocate offscreen buffer, not needed
	}
}

void SeqPlayer::s1_textDisplayEnable() {
	_seqDisplayTextFlag = true;
}

void SeqPlayer::s1_textDisplayDisable() {
	_seqDisplayTextFlag = false;
}

void SeqPlayer::s1_endOfScript() {
	_seqQuitFlag = true;
}

void SeqPlayer::s1_miscUnk1() {
	warning("STUB: s1_miscUnk1");
}

void SeqPlayer::s1_playVocFile() {
	while (_vm->snd_voicePlaying()) {
		_system->delayMillis(10);
	}
	uint8 a = *_seqData++;
	_vm->snd_playVoiceFile(a);
}

void SeqPlayer::s1_displayStory() {
	// nothing to do here since we handle it in
	// KyraEngine::seq_introStory
}

void SeqPlayer::s1_prefetchVocFile() {
	*_seqData++;
	// we do not have to prefetch the vocfiles on modern systems
}

bool SeqPlayer::playSequence(const uint8 *seqData, bool skipSeq) {
	debug(9, "SeqPlayer::seq_playSequence(0x%X, %d)", seqData, skipSeq);
	assert(seqData);
	
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
		SEQOP(2, s1_playEffect),
		SEQOP(2, s1_playTrack),
		SEQOP(1, s1_allocTempBuffer),
		SEQOP(1, s1_textDisplayEnable),
		// 0x1C
		SEQOP(1, s1_textDisplayDisable),
		SEQOP(1, s1_endOfScript)
	};

	static SeqEntry cdromSeqProcs[] = {
		// 0x00
		SEQOP(3, s1_wsaOpen),
		SEQOP(2, s1_wsaClose),
		SEQOP(6, s1_wsaPlayFrame),
		SEQOP(2, s1_wsaPlayNextFrame),
		// 0x04
		SEQOP(2, s1_wsaPlayPrevFrame),
		SEQOP(5, s1_drawShape),
		SEQOP(3, s1_waitTicks),
		SEQOP(3, s1_waitTicks),
		// 0x08
		SEQOP(3, s1_copyWaitTicks),
		SEQOP(1, s1_shuffleScreen),
		SEQOP(1, s1_copyView),
		SEQOP(2, s1_loopInit),
		// 0x0C
		SEQOP(4, s1_loopInc),
		SEQOP(4, s1_loopInc), // Again?
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
		SEQOP(2, s1_playEffect),
		// 0x1C
		SEQOP(2, s1_playTrack),
		SEQOP(1, s1_allocTempBuffer),
		SEQOP(1, s1_textDisplayEnable),
		SEQOP(1, s1_textDisplayDisable),
		// 0x20
		SEQOP(1, s1_endOfScript),
		SEQOP(1, s1_miscUnk1),
		SEQOP(2, s1_playVocFile),
		SEQOP(1, s1_displayStory),
		// 0x24
		SEQOP(2, s1_prefetchVocFile)
	};

	const SeqEntry* commands;
	int numCommands;

	if (_vm->features() & GF_FLOPPY || _vm->features() & GF_DEMO) {
		commands = floppySeqProcs;
		numCommands = ARRAYSIZE(floppySeqProcs);
	} else if (_vm->features() & GF_TALKIE) {
		commands = cdromSeqProcs;
		numCommands = ARRAYSIZE(cdromSeqProcs);
	} else {
		error("No commandlist found");
	}

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
		if (skipSeq && _vm->seq_skipSequence()) {
			while (1) {
				uint8 code = *_seqData;
				if (commands[code].proc == &SeqPlayer::s1_endOfScript || commands[code].proc == &SeqPlayer::s1_break) {
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
				charStr[0] = _vm->seqTextsTable()[_seqDisplayedText][_seqDisplayedChar];
				charStr[1] = '\0';
				_screen->printText(charStr, _seqDisplayedTextX, 180, 0xF, 0xC);
				_seqDisplayedTextX += _screen->getCharWidth(charStr[0]);
				++_seqDisplayedChar;
				if (_vm->seqTextsTable()[_seqDisplayedText][_seqDisplayedChar] == '\0') {
					_seqDisplayedTextTimer = 0xFFFFFFFF;
				} else {
					_seqDisplayedTextTimer = _system->getMillis() + 1000 / ((_vm->features() & GF_FRENCH) ? 120 : 60);
				}
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

} // End of namespace Kyra
