/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_KYRA_H
#define KYRA_KYRA_H

#include "engines/engine.h"
#include "common/rect.h"
#include "common/array.h"
#include "common/events.h"

namespace Kyra {

struct Opcode;

struct GameFlags {
	Common::Language lang;
	Common::Platform platform;
	bool isDemo;
	bool useAltShapeHeader;	// alternative shape header (uses 2 bytes more, those are unused though)
	bool isTalkie;
	bool useHiResOverlay;
	byte gameID;
};

struct Rect {
	int x, y;
	int x2, y2;
};

enum {
	GI_KYRA1 = 0,
	GI_KYRA2 = 1,
	GI_KYRA3 = 2
};

// TODO: this is just the start of makeing the debug output of the kyra engine a bit more useable
// in the future we maybe merge some flags  and/or create new ones
enum kDebugLevels {
	kDebugLevelScriptFuncs = 1 << 0,		// prints debug output of o1_* functions
	kDebugLevelScript = 1 << 1,				// prints debug output of "ScriptHelper" functions
	kDebugLevelSprites = 1 << 2,			// prints debug output of "Sprites" functions
	kDebugLevelScreen = 1 << 3,				// prints debug output of "Screen" functions
	kDebugLevelSound = 1 << 4,				// prints debug output of "Sound" functions
	kDebugLevelAnimator = 1 << 5,			// prints debug output of "ScreenAnimator" functions
	kDebugLevelMain = 1 << 6,				// prints debug output of common "KyraEngine*" functions && "TextDisplayer" functions
	kDebugLevelGUI = 1 << 7,				// prints debug output of "KyraEngine*" gui functions
	kDebugLevelSequence = 1 << 8,			// prints debug output of "SeqPlayer" functions
	kDebugLevelMovie = 1 << 9				// prints debug output of movie specific funtions
};

class Screen;
class Resource;
class Sound;
class Movie;
class TextDisplayer;
class StaticResource;

class KyraEngine : public Engine {
public:
	KyraEngine(OSystem *system, const GameFlags &flags);
	virtual ~KyraEngine();
	
	bool quit() const { return _quitFlag; }
	
	uint8 game() const { return _flags.gameID; }
	const GameFlags &gameFlags() const { return _flags; }
	
	// access to Kyra specific functionallity
	Resource *resource() { return _res; }
	virtual Screen *screen() = 0;
	TextDisplayer *text() { return _text; }
	Sound *sound() { return _sound; }
	StaticResource *staticres() { return _staticres; }
	
	uint32 tickLength() const { return _tickLength; }
	
	virtual Movie *createWSAMovie() = 0;
	
	Common::RandomSource _rnd;
	
	// quit handling
	virtual void quitGame();

	// game flag handling	
	int setGameFlag(int flag);
	int queryGameFlag(int flag) const;
	int resetGameFlag(int flag);
	
	// delay functionallity
	virtual void delayUntil(uint32 timestamp, bool updateGameTimers = false, bool update = false, bool isMainLoop = false);
	virtual void delay(uint32 millis, bool update = false, bool isMainLoop = false);
	virtual void delayWithTicks(int ticks);

protected:
	virtual int go() = 0;
	virtual int init();
	
	// quit Handling
	bool _quitFlag;
	
	// intern
	Resource *_res;
	Screen *_screen;
	Sound *_sound;
	TextDisplayer *_text;
	StaticResource *_staticres;
	
	// game speed
	bool _skipFlag;
	uint16 _tickLength;
	
	// detection
	GameFlags _flags;
	int _lang;

	// opcode
	virtual void setupOpcodeTable() = 0;
	Common::Array<const Opcode*> _opcodes;
	
	// game flags
	uint8 _flagsTable[100];	// TODO: check this value
	
	// input	
	Common::Point getMousePos() const;
};

} // End of namespace Kyra

#endif

