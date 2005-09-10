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

#ifndef KYRA_H
#define KYRA_H

#include "base/engine.h"
#include "common/util.h"

namespace Kyra {

enum {
	GF_FLOPPY  = 1 << 0,
	GF_TALKIE  = 1 << 1,
	GF_KYRA1   = 1 << 2,
	GF_KYRA2   = 1 << 3,
	GF_KYRA3   = 1 << 4,
	GF_AUDIOCD = 1 << 5  // FM-Towns versions seems to use audio CD
};

enum {
	KYRA1   = 0,
	KYRA1CD = 1,
	KYRA2   = 2,
	KYRA2CD = 3,
	KYRA3   = 4
};

struct Character {
	uint16 sceneId;
	uint8 height;
	uint8 facing;
	uint16 currentAnimFrame;
	uint32 unk6;
	uint8 inventoryItems[10];
	int16 x1, y1, x2, y2;
};

struct Shape {
	uint8 unk0;
	uint8 unk1;
	uint8 imageNum;
	uint8 x, y, w, h;
};

struct Room {
	uint8 id;
	uint16 northExit;
	uint16 eastExit;
	uint16 southExit;
	uint16 westExit;
	uint8 itemsTable[12];
};

struct TalkCoords {
	uint16 y, x, w;
};

struct SeqLoop {
	const uint8 *ptr;
	uint16 count;
};

struct WSAMovieV1;

class MusicPlayer;
class Resource;
class Screen;

class KyraEngine : public Engine {
public:

	enum {
		TALK_SUBSTRING_LEN = 80,
		TALK_SUBSTRING_NUM = 3
	};

	KyraEngine(GameDetector *detector, OSystem *system);
	~KyraEngine();
	
	void errorString(const char *buf_input, char *buf_output);

	Resource *resource() { return _res; }
	Screen *screen() { return _screen; }

	uint8 game() const { return _game; }
	
	Common::RandomSource _rnd;

	typedef void (KyraEngine::*IntroProc)();

protected:

	int go();
	int init(GameDetector &detector);

	void startup();
	void mainLoop();
	void loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData);
	void setTalkCoords(uint16 y);
	int getCenterStringX(const char *str, int x1, int x2);
	int getCharLength(const char *str, int len);
	int dropCRIntoString(char *str, int offs);
	char *preprocessString(const char *str);
	int buildMessageSubstrings(const char *str);
	int getWidestLineWidth(int linesCount);
	void calcWidestLineBounds(int &x1, int &x2, int w, int cx);
	void restoreTalkTextMessageBkgd(int srcPage, int dstPage);
	void printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage);
	void printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2);
	void waitTicks(int ticks);
	
	void seq_intro();
	void seq_introLogos();
	void seq_introStory();
	void seq_introMalcomTree();
	void seq_introKallakWriting();
	void seq_introKallakMalcom();
	uint8 *seq_setPanPages(int pageNum, int shape);
	void seq_makeHandShapes();
	void seq_freeHandShapes();
	void seq_copyView();
	bool seq_skipSequence() const;
	bool seq_playSpecialSequence(const uint8 *seqData, bool skipSeq);

	WSAMovieV1 *wsa_open(const char *filename, int offscreenDecode, uint8 *palBuf);
	void wsa_close(WSAMovieV1 *wsa);
	uint16 wsa_getNumFrames(WSAMovieV1 *wsa) const;
	void wsa_play(WSAMovieV1 *wsa, int frameNum, int x, int y, int pageNum);
	void wsa_processFrame(WSAMovieV1 *wsa, int frameNum, uint8 *dst);
		
	uint8 _game;
	bool _fastMode;
	bool _quitFlag;
	bool _skipIntroFlag;
	char _talkBuffer[300];
	char _talkSubstrings[TALK_SUBSTRING_LEN * TALK_SUBSTRING_NUM];
	TalkCoords _talkCoords;
	uint16 _talkMessageY;
	uint16 _talkMessageH;
	bool _talkMessagePrinted;
	uint8 _flagsTable[51];

	bool _seq_copyViewOffs;
	uint8 *_seq_handShapes[3];

	Resource *_res;
	Screen *_screen;
	MusicPlayer *_midi;

	// these tables are specific to the floppy version
	static const uint8 _seq_introData_Forest[];
	static const uint8 _seq_introData_KallakWriting[];
	static const uint8 _seq_introData_KyrandiaLogo[];
	static const uint8 _seq_introData_KallakMalcom[];
	static const uint8 _seq_introData_MalcomTree[];
	static const uint8 _seq_introData_WestwoodLogo[];
	static const uint8 _seq_codeSizeTable[];
	static const char *_seq_WSATable[];
	static const char *_seq_CPSTable[];
	static const char *_seq_COLTable[];
	static const char *_seq_textsTableEN[];	
};

} // End of namespace Kyra

#endif
