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
#include "common/system.h"
#include "common/savefile.h"
#include "common/fs.h"

#include "engines/engine.h"

namespace Common {
	class RandomSource;
}

namespace HDB {
class AI;
struct AIEntity;
class FileMan;
class Gfx;
class Input;
class LuaScript;
class Map;
class Menu;
class Tile;
class Picture;
class Sound;
class Window;

enum {
	kTileWidth = 32,
	kTileHeight = 32,
	kMaxSkies = 10,
	kNum3DStars = 300,
	kFontSpace = 5,
	kFontIncrement = 1,
	kGameFPS = 60,
	kAnimFrameDelay = kGameFPS / 30,
	kAnimSlowFrames = kAnimFrameDelay * 10,
	kAnimMediumFrames = kAnimFrameDelay * 6,
	kAnimFastFrames = kAnimFrameDelay * 2
};

}

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

struct Save {
	char saveID[12];
	int fileSlot;
	char mapName[32];
	uint32 seconds;

	Save() : fileSlot(0), seconds(0) {
		saveID[0] = 0;
		mapName[0] = 0;
	}
};

class HDBGame : public Engine {
public:
	HDBGame(OSystem *syst, const ADGameDescription *gameDesc);
	~HDBGame() override;

	bool hasFeature(Engine::EngineFeature f) const override;
	void initializePath(const Common::FSNode &gamePath) override;

	Common::Error run() override;

	// Detection related members;
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	const char *getGameFile() const;
	uint32 getGameFlags() const;
	Common::Platform getPlatform() const;
	bool isDemo() const;
	bool isPPC() const;
	bool isHandango() const;

	// Platform-Specific Constants

	int _screenWidth;
	int _screenHeight;
	int _screenDrawWidth; // visible pixels wide
	int _screenDrawHeight;
	int _progressY;

	/*
	Game System Pointers
	*/

	FileMan *_fileMan;
	Gfx *_gfx;
	LuaScript *_lua;
	Map *_map;
	AI *_ai;
	Input *_input;
	Menu *_menu;
	Sound *_sound;
	Window *_window;

	// Random Source
	Common::RandomSource *_rnd;

	// Game related members;

	bool init();
	void save(Common::OutSaveFile *out);
	void loadSaveFile(Common::InSaveFile *in);

	void start();
	bool restartMap();
	bool startMap(const char *name);

	void changeMap(const char *name) {
		Common::strlcpy(_changeMapname, name, 64);
		_changeLevel = true;
	}

	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	void saveGame(Common::OutSaveFile *out);
	void loadGame(Common::InSaveFile *in);

	Common::String genSaveFileName(uint slot, bool lua);

	void saveWhenReady(int slot) {
		_saveInfo.active = true;
		_saveInfo.slot = slot;
	}

	void loadWhenReady(int slot) {
		_loadInfo.active = true;
		_loadInfo.slot = slot;
	}

	void setGameState(GameState gs) {
		_gameState = gs;
	}
	GameState getGameState() {
		return _gameState;
	}
	void changeGameState();
	void paint();
	void moveMap(int x, int y);	// Get Stylus Coords and Scroll
	void startMoveMap(int x, int y);	// Start Dragging Map

	void setTargetXY(int x, int y);
	void useEntity(AIEntity *e);

	void setupProgressBar(int maxCount);
	void drawProgressBar();
	void makeProgress() {
		_progressCurrent++;
		drawProgressBar();
	}
	void checkProgress();
	void stopProgress() {
		_progressActive = false;
	}
	void drawLoadingScreen();

	int getActionMode() {
		return _actionMode;
	}
	void setActionMode(int status) {
		_actionMode = status;
	}

	void togglePause() {
		_pauseFlag ^= true;
	}

	bool getPause() {
		return _pauseFlag;
	}

	void resetTimer() {
		_timePlayed = _timeSeconds = 0;
	}

	uint32 getTime() {
		return _timePlayed / 1000;
	}

	uint32 getTimeSlice() {
		return _timeSlice;
	}

	uint32 getTimeSliceDelta() {
		return _timeSlice - _prevTimeSlice;
	}

	const Common::String *getTargetName() {
		return &_targetName;
	}

	int getDebug() { return _debugFlag; }
	void setDebug(int flag) { _debugFlag = flag; }

	bool isVoiceless() {
		/*
			FIXME: Add hyperspace-nv.mpc to gameDescriptions[]
			in detection.cpp, and add a flag check for it.
			Until then, the voiceless version is unsupported.
		*/
		return false;
	}

	char *lastMapName() { return _lastMapname; }
	char *currentMapName() { return _currentMapname; }
	char *getInMapName() { return _inMapName; }
	void setInMapName(const char *name);

	void changeLevel(const char *name) {
		Common::strlcpy(_changeMapname, name, 64);
		_changeLevel = true;
	}

	//
	// monkeystone secret stars
	//
	int32 getStarsMonkeystone7() { return _monkeystone7; }
	int32 getStarsMonkeystone14() { return _monkeystone14; }
	int32 getStarsMonkeystone21() { return _monkeystone21; }

	void setStarsMonkeystone7(int32 value) { _monkeystone7 = value; }
	void setStarsMonkeystone14(int32 value) { _monkeystone14 = value; }
	void setStarsMonkeystone21(int32 value) { _monkeystone21 = value; }

	void setCheatingOn() {
		_cheating = true;
	}
	bool getCheatingOn() {
		return _cheating;
	}

	Save _saveHeader;
	bool _gameShutdown;
	Graphics::PixelFormat _format;

	Picture *_progressGfx, *_progressMarkGfx;
	Picture *_loadingScreenGfx, *_logoGfx;
	bool _progressActive;
	int _progressCurrent, _progressXOffset, _progressMax;

	// FPS Variables
	Common::Array<uint32> _frames;

	Common::OutSaveFile *_currentOutSaveFile;
	Common::InSaveFile *_currentInSaveFile;

private:

	uint32 _timePlayed;
	uint32 _timeSlice, _prevTimeSlice;
	uint32 _timeSeconds;

	uint32 _tiempo;

	// Game Variables

	bool _systemInit;
	GameState _gameState;
	int _actionMode; // 0 or 1

	// Misc Variables
	bool _pauseFlag;
	bool _cheating;
	int _debugFlag;
	Tile *_debugLogo;
	int _dx, _dy;	// DEBUG : for dragging map

	char _currentMapname[64];
	char _lastMapname[64];

	char _currentLuaName[64];
	char _lastLuaName[64];

	char _inMapName[32];	// Name Inside Map file

	int32 _monkeystone7;
	int32 _monkeystone14;
	int32 _monkeystone21;

	bool _changeLevel;
	char _changeMapname[64];

	struct {
		bool active;
		int  slot;
	} _saveInfo, _loadInfo;

};

extern HDBGame *g_hdb;

}// End of namespace HDB

#endif
