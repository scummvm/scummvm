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

#ifndef KYRA_H
#define KYRA_H

#include "base/engine.h"
#include "common/rect.h"
#include "sound/mixer.h"
#include "common/file.h"

class AudioStream;

namespace Kyra {

#define MAX_NUM_ROOMS 12

enum {
	GF_FLOPPY	= 1 <<  0,
	GF_TALKIE	= 1 <<  1,
	GF_AUDIOCD	= 1 <<  2,  // FM-Towns versions seems to use audio CD
	GF_DEMO		= 1 <<  3,
	GF_ENGLISH	= 1 <<  4,
	GF_FRENCH	= 1 <<  5,
	GF_GERMAN	= 1 <<  6,
	GF_SPANISH	= 1 <<  7,
	// other languages here
	GF_LNGUNK	= 1 << 16
};

enum {
	GI_KYRA1 = 0
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
//	uint8 id;
	uint16 northExit;
	uint16 eastExit;
	uint16 southExit;
	uint16 westExit;
	uint8 itemsTable[12];
	const char *filename;
};

struct Cursor {
	int x, y;
	int w, h;
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
class SeqPlayer;
class Resource;
class PAKFile;
class Screen;
class Sprites;

class KyraEngine : public Engine {
	friend class MusicPlayer;
public:

	enum {
		TALK_SUBSTRING_LEN = 80,
		TALK_SUBSTRING_NUM = 3
	};
	
	enum {
		MUSIC_INTRO = 0
	};

	KyraEngine(GameDetector *detector, OSystem *system);
	~KyraEngine();
	
	void errorString(const char *buf_input, char *buf_output);

	Resource *resource() { return _res; }
	Screen *screen() { return _screen; }
	MusicPlayer *midi() { return _midi; }

	uint8 game() const { return _game; }
	uint32 features() const { return _features; }
	
	Common::RandomSource _rnd;

	typedef void (KyraEngine::*IntroProc)();

	const char **seqWSATable() { return (const char **)_seq_WSATable; }
	const char **seqCPSTable() { return (const char **)_seq_CPSTable; }
	const char **seqCOLTable() { return (const char **)_seq_COLTable; }
	const char **seqTextsTable() { return (const char **)_seq_textsTable; }

	bool seq_skipSequence() const;
	
	void loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData);

	void snd_playTheme(int file, int track = 0);
	void snd_playTrack(int track);
	void snd_playVoiceFile(int id);
	bool snd_voicePlaying();

	void printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage);
	void restoreTalkTextMessageBkgd(int srcPage, int dstPage);

	WSAMovieV1 *wsa_open(const char *filename, int offscreenDecode, uint8 *palBuf);
	void wsa_close(WSAMovieV1 *wsa);
	uint16 wsa_getNumFrames(WSAMovieV1 *wsa) const;
	void wsa_play(WSAMovieV1 *wsa, int frameNum, int x, int y, int pageNum);

	void waitTicks(int ticks);
	
	int mouseX() { return _mouseX; }
	int mouseY() { return _mouseY; }

protected:

	int go();
	int init(GameDetector &detector);

	void startup();
	void mainLoop();
	void setTalkCoords(uint16 y);
	int getCenterStringX(const char *str, int x1, int x2);
	int getCharLength(const char *str, int len);
	int dropCRIntoString(char *str, int offs);
	char *preprocessString(const char *str);
	int buildMessageSubstrings(const char *str);
	int getWidestLineWidth(int linesCount);
	void calcWidestLineBounds(int &x1, int &x2, int w, int cx);
	void printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2);
	
	void seq_demo();
	void seq_intro();
	void seq_introLogos();
	void seq_introStory();
	void seq_introMalcolmTree();
	void seq_introKallakWriting();
	void seq_introKallakMalcolm();

	void wsa_processFrame(WSAMovieV1 *wsa, int frameNum, uint8 *dst);

	void snd_startTrack();
	void snd_haltTrack();
	void snd_setSoundEffectFile(int file);
	void snd_playSoundEffect(int track);
	
	enum {
		RES_ALL = 0,
		RES_INTRO = (1 << 0)
	};
	
	void res_loadResources(int type = RES_ALL);
	void res_unloadResources(int type = RES_ALL);
	void res_loadLangTable(const char *filename, PAKFile *res, byte ***loadTo, int *size, bool nativ);
	void res_loadTable(const byte *src, byte ***loadTo, int *size);
		
	void loadRoom(uint16 roomID);
	void drawRoom();
	void delay(uint32 millis);
	void loadPalette(const char *filename, uint8 *palData);
	void loadMouseShapes();
	void setupRooms();

	uint8 _game;
	bool _fastMode;
	bool _quitFlag;
	bool _skipIntroFlag;
	bool _abortIntroFlag;
	char _talkBuffer[300];
	char _talkSubstrings[TALK_SUBSTRING_LEN * TALK_SUBSTRING_NUM];
	TalkCoords _talkCoords;
	uint16 _talkMessageY;
	uint16 _talkMessageH;
	bool _talkMessagePrinted;
	uint8 _flagsTable[51];
	uint8 *_itemShapes[377];
	uint16 _gameSpeed;
	uint32 _features;
	int _mouseX, _mouseY;
	bool _needMouseUpdate;

	uint16 _currentRoom;
	AudioStream *_currentVocFile;
	Audio::SoundHandle _vocHandle;

	Resource *_res;
	Screen *_screen;
	MusicPlayer *_midi;
	SeqPlayer *_seq;
	Sprites *_sprites;
	Room _rooms[MAX_NUM_ROOMS];
	
	uint8 *_seq_Forest;
	uint8 *_seq_KallakWriting;
	uint8 *_seq_KyrandiaLogo;
	uint8 *_seq_KallakMalcolm;
	uint8 *_seq_MalcolmTree;
	uint8 *_seq_WestwoodLogo;
	uint8 *_seq_Demo1;
	uint8 *_seq_Demo2;
	uint8 *_seq_Demo3;
	uint8 *_seq_Demo4;
	
	char **_seq_WSATable;
	char **_seq_CPSTable;
	char **_seq_COLTable;
	char **_seq_textsTable;
	
	int _seq_WSATable_Size;
	int _seq_CPSTable_Size;
	int _seq_COLTable_Size;
	int _seq_textsTable_Size;
	
	static const char *_xmidiFiles[];
	static const int _xmidiFilesCount;
};

} // End of namespace Kyra

#endif
