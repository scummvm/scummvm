/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KYRA_SEQPLAYER_H
#define KYRA_SEQPLAYER_H

#include "kyra/engine/kyra_lok.h"

namespace Kyra {

class SeqPlayer {
public:
	SeqPlayer(KyraEngine_LoK *vm, OSystem *system);
	~SeqPlayer();

	void setCopyViewOffs(bool offs) {
		_copyViewOffs = offs;
	}

	void makeHandShapes();
	void freeHandShapes();

	bool playSequence(const uint8 *seqData, bool skipSeq);

	uint8 *setPanPages(int pageNum, int shape);
protected:
	KyraEngine_LoK *_vm;
	OSystem *_system;
	Screen *_screen;
	Sound *_sound;
	Resource *_res;

	uint8 *_handShapes[3];
	bool _copyViewOffs;

	typedef void (SeqPlayer::*SeqProc)();
	struct SeqEntry {
		uint8 len;
		SeqProc proc;
		const char *desc;
	};

	// the sequence procs
	void s1_wsaOpen();
	void s1_wsaClose();
	void s1_wsaPlayFrame();
	void s1_wsaPlayNextFrame();
	void s1_wsaPlayPrevFrame();
	void s1_drawShape();
	void s1_waitTicks();
	void s1_copyWaitTicks();
	void s1_shuffleScreen();
	void s1_copyView();
	void s1_loopInit();
	void s1_loopInc();
	void s1_skip();
	void s1_loadPalette();
	void s1_loadBitmap();
	void s1_fadeToBlack();
	void s1_printText();
	void s1_printTalkText();
	void s1_restoreTalkText();
	void s1_clearCurrentScreen();
	void s1_break();
	void s1_fadeFromBlack();
	void s1_copyRegion();
	void s1_copyRegionSpecial();
	void s1_fillRect();
	void s1_playEffect();
	void s1_playTrack();
	void s1_allocTempBuffer();
	void s1_textDisplayEnable();
	void s1_textDisplayDisable();
	void s1_endOfScript();
	void s1_loadIntroVRM();
	void s1_playVocFile();
	void s1_miscUnk3();
	void s1_prefetchVocFile();
	void s1_textDisplayWait();

	struct SeqMovie {
		Movie *movie;
		int32 page;
		int16 frame;
		int16 numFrames;
		Common::Point pos;

		void clear() {
			movie = nullptr;
			page = 0;
			frame = 0;
			numFrames = 0;
			pos.x = 0;
			pos.y = 0;
		}
	};

	const uint8 *_seqData;
	uint8 _seqCode;
	uint8 *_specialBuffer;
	SeqMovie _seqMovies[12];
	SeqLoop _seqLoopTable[20];
	uint8 _seqLoopPos;
	uint16 _seqWsaCurDecodePage;
	uint32 _seqDisplayedTextTimer;
	uint32 _seqVocStartTimer;
	bool _seqDisplayTextFlag;
	uint8 _seqDisplayedText;
	uint8 _seqDisplayedChar;
	uint16 _seqDisplayedTextX;
	bool _seqTalkTextPrinted;
	bool _seqTalkTextRestored;
	bool _seqQuitFlag;
	bool _seqSkipCommand;
};

} // End of namespace Kyra

#endif
