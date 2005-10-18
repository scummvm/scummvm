/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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

#ifndef QUEEN_H
#define QUEEN_H

#include "base/engine.h"

class GameDetector;
namespace Common {
	class InSaveFile;
}

#if defined(_WIN32_WCE) && (_WIN32_WCE <= 300)

FORCEINLINE int16 READ_BE_INT16(const void *ptr) {
	uint16 result;
	char dummy[2];
	result = READ_BE_UINT16(ptr);
	strcpy(dummy, "x"); // Hello, I'm a drunk optimizer. Thanks for helping me.
	return result;
}

#else

#define READ_BE_INT16 READ_BE_UINT16

#endif

namespace Queen {

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct GameStateHeader {
	uint32 version;
	uint32 flags;
	uint32 dataSize;
	char description[32];
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

class BamScene;
class BankManager;
class Command;
class Debugger;
class Display;
class Graphics;
class Grid;
class Input;
class Logic;
class Music;
class Resource;
class Sound;
class Walk;

class QueenEngine : public Engine {
public:

	QueenEngine(GameDetector *detector, OSystem *syst);
	virtual ~QueenEngine();

	BamScene *bam() const { return _bam; }
	BankManager *bankMan() const { return _bankMan; }
	Command *command() const { return _command; }
	Debugger *debugger() const { return _debugger; }
	Display *display() const { return _display; }
	Graphics *graphics() const { return _graphics; }
	Grid *grid() const { return _grid; }
	Input *input() const { return _input; }
	Logic *logic() const { return _logic; }
	Music *music() const { return _music; }
	Resource *resource() const { return _resource; }
	Sound *sound() const { return _sound; }
	Walk *walk() const { return _walk; }

	Common::RandomSource randomizer;

	void registerDefaultSettings();
	void checkOptionSettings();
	void readOptionSettings();
	void writeOptionSettings();

	int talkSpeed() const { return _talkSpeed; }
	void talkSpeed(int speed) { _talkSpeed = speed; }
	bool subtitles() const { return _subtitles; }
	void subtitles(bool enable) { _subtitles = enable; }
	void quitGame() { _quit = true; }

	void update(bool checkPlayerInput = false);

	bool canLoadOrSave() const;
	void saveGameState(uint16 slot, const char *desc);
	void loadGameState(uint16 slot);
	void makeGameStateName(uint16 slot, char *buf);
	void findGameStateDescriptions(char descriptions[100][32]);
	Common::InSaveFile *readGameStateHeader(uint16 slot, GameStateHeader *gsh);

	enum {
		SAVESTATE_CUR_VER  = 1,
		SAVESTATE_MAX_NUM  = 100,
		SAVESTATE_MAX_SIZE = 30000,

		AUTOSAVE_INTERVAL  = 5 * 60 * 1000,
		AUTOSAVE_SLOT      = 0xFF,

		MIN_TEXT_SPEED     = 4,
		MAX_TEXT_SPEED     = 100,
		MAX_MUSIC_VOLUME   = 255
	};

protected:

	void errorString(const char *buf_input, char *buf_output);

	int go();
	int init(GameDetector &detector);


	int _talkSpeed;
	bool _subtitles;
	bool _quit;
	uint32 _lastSaveTime;

	BamScene *_bam;
	BankManager *_bankMan;
	Command *_command;
	Debugger *_debugger;
	Display *_display;
	Graphics *_graphics;
	Grid *_grid;
	Input *_input;
	Logic *_logic;
	Music *_music;
	Resource *_resource;
	Sound *_sound;
	Walk *_walk;
};

} // End of namespace Queen

#endif
