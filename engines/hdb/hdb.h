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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HDB_HDB_H
#define HDB_HDB_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/error.h"
#include "common/file.h"
#include "common/events.h"
#include "common/str.h"
#include "common/random.h"
#include "graphics/surface.h"

#include "gui/debugger.h"
#include "engines/engine.h"
#include "engines/util.h"
#include "console.h"

#include "hdb/mpc.h"
#include "hdb/gfx.h"
#include "hdb/ai.h"
#include "hdb/ai-player.h"
#include "hdb/file-manager.h"
#include "hdb/input.h"
#include "hdb/lua-script.h"
#include "hdb/map-loader.h"
#include "hdb/window.h"

#define MAX_SNDCACHE_MEM	0x400000	// 4Mb of sounds in memory
#define MAX_TILES_CACHED	3500		// Max no of tiles in memory at once
#define GFX_CACHE_LIMIT		0x800000

struct ADGameDescription;

namespace HDB {

enum GameFeatures {
	ADGF_TALKIE = 1 << 0
};

enum HDBDebugChannels {
	kDebugExample1 = 1 << 0,
	kDebugExample2 = 1 << 1
};

enum GameState {
	GAME_TITLE,
	GAME_MENU,
	GAME_PLAY,
	GAME_LOADING
};

enum Flag {
	kFlagOK				= 0x0,
	kFlagPlayerBlock	= 0x1,
	kFlagMonsterBlock	= 0x2,
	kFlagSolid			= 0x3,
	kFlagItemDie		= 0x4,
	kFlagPlayerDie		= 0x8,
	kFlagMonsterDie		= 0x10,
	kFlagInvisible		= 0x20,
	kFlagMetal			= 0x40,
	kFlagForeground		= 0x80,
	kFlagMonsterHurt	= 0x100,
	kFlagPushUp			= 0x200,
	kFlagPushRight		= 0x400,
	kFlagPushDown		= 0x800,
	kFlagPushLeft		= 0x1000,
	kFlagLightSink		= 0x2000,
	kFlagSlime			= 0x201C,
	kFlagHeavySink		= 0x4000,
	kFlagWater			= 0x401C,
	kFlagLightMelt		= 0x8000,
	kFlagHeavyMelt		= 0x10000,
	kFlagSlide			= 0x20000,
	kFlagEnergyFloor	= 0x40000,
	kFlagPlasmaFloor	= 0x6000D,
	kFlagRadFloor		= 0x6800D,
	kFlagTeleport		= 0x80000,
	kFlagSpecial		= 0x100000,
	kFlagIce			= 0x120000,
	kFlagStairBot		= 0x200000,
	kFlagStairTop		= 0x400000,
	kFlagAnimSlow		= 0x800000,
	kFlagAnimMedium		= 0x1000000,
	kFlagAnimFast		= 0x1800000,
	kFlagMasked			= 0x2000000,
	kFlagGrating		= 0x4000000,
	kFlagPlummet		= 0x8000000
};

class HDBGame : public Engine {
public:
	HDBGame(OSystem *syst, const ADGameDescription *gameDesc);
	~HDBGame();

	virtual Common::Error run();

	// Detection related members;
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	const char *getGameFile() const;
	uint32 getGameFlags() const;
	Common::Platform getPlatform() const;
	bool isDemo() const;

	/*
	Game System Pointers
	*/

	FileMan *_fileMan;
	DrawMan *_drawMan;
	LuaScript *_lua;
	Map *_map;
	AI *_ai;
	Input *_input;
	Window *_window;

	// Random Source
	Common::RandomSource *_rnd;

	// Game related members;

	bool init();

	void start();
	void setGameState(GameState gs) {
		_gameState = gs;
	}
	GameState getGameState() {
		return _gameState;
	}
	void changeGameState();
	void paint();

	void setTargetXY(int x, int y);
	void useEntity(AIEntity *e);

	int getActionMode() {
		return _actionMode;
	}
	void setActionMode(int status) {
		_actionMode = status;
	}

	void togglePause() {
		_pauseFlag ^= 1;
	}
	int getPause() {
		return _pauseFlag;
	}

	void resetTimer() {
		_timePlayed = _timeSeconds = 0;
	}

	uint16 getTime() {
		return _timePlayed / 1000;
	}

	uint16 getTimeSlice() {
		return _timeSlice;
	}

	uint16 getTimeSliceDelta() {
		return _timeSlice - _prevTimeSlice;
	}

	bool _gameShutdown;
	Graphics::PixelFormat _format;

private:

	uint32 _timePlayed;
	uint32 _timeSlice, _prevTimeSlice;
	uint32 _timeSeconds;

	Console *_console;

	// Game Variables

	bool _systemInit;
	GameState _gameState;
	int _actionMode; // 0 or 1
	int _pauseFlag;

};

extern HDBGame *g_hdb;

}// End of namespace HDB

#endif
