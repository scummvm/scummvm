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

#ifndef KYRASEQPLAYER_H
#define KYRASEQPLAYER_H

namespace Kyra {

class SeqPlayer {
public:
	SeqPlayer(KyraEngine* vm, OSystem* system);
	~SeqPlayer();

	void setCopyViewOffs(bool offs) {
		_copyViewOffs = offs;
	}

	void makeHandShapes();
	void freeHandShapes();

	bool playSequence(const uint8 *seqData, bool skipSeq);

protected:
	KyraEngine *_vm;
	OSystem *_system;
	Screen *_screen;
	MusicPlayer *_midi;
	Resource *_res;

	uint8 *_handShapes[3];
	bool _copyViewOffs;

	typedef void (SeqPlayer::*SeqProc)();
	struct SeqEntry {
		uint8 len;
		SeqProc proc;
		const char* desc;
	};

	uint8 *setPanPages(int pageNum, int shape);

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
	void s1_soundUnk1();
	void s1_soundUnk2();
	void s1_allocTempBuffer();
	void s1_textDisplayEnable();
	void s1_textDisplayDisable();
	void s1_endOfScript();
	void s1_miscUnk1();
	void s1_playVocFile();
	void s1_miscUnk3();
	void s1_prefetchVocFile();

	struct SeqMovie {
		WSAMovieV1 *wsa;
		int32 page;
		int16 frame;
		int16 numFrames;
		Common::Point pos;
	};

	const uint8 *_seqData;
	SeqMovie _seqMovies[12];
	SeqLoop _seqLoopTable[20];
	uint16 _seqWsaCurDecodePage;
	uint32 _seqDisplayedTextTimer;
	bool _seqDisplayTextFlag;
	uint8 _seqDisplayedText;
	uint8 _seqDisplayedChar;
	uint16 _seqDisplayedTextX;
	bool _seqTalkTextPrinted;
	bool _seqTalkTextRestored;
	bool _seqQuitFlag;
};

} // End of namespace Kyra

#endif
