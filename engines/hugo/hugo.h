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

#ifndef HUGO_H
#define HUGO_H

#include "engines/engine.h"
#include "common/file.h"

// This include is here temporarily while the engine is being refactored.
#include "hugo/game.h"

#define HUGO_DAT_VER_MAJ 0  // 1 byte
#define HUGO_DAT_VER_MIN 16 // 1 byte
#define DATAALIGNMENT 4

namespace Common {
class RandomSource;
}

namespace Hugo {
enum GameType {
	kGameTypeNone  = 0,
	kGameTypeHugo1,
	kGameTypeHugo2,
	kGameTypeHugo3
};

enum HugoebugChannels {
	kDebugSchedule   = 1 <<  0,
	kDebugEngine     = 1 <<  1,
	kDebugDisplay    = 1 <<  2,
	kDebugMouse      = 1 <<  3,
	kDebugParser     = 1 <<  4,
	kDebugFile       = 1 <<  5,
	kDebugRoute      = 1 <<  6,
	kDebugInventory  = 1 <<  7
};

enum HugoGameFeatures {
	GF_PACKED = (1 << 0) // Database
};

struct HugoGameDescription;

class FileManager;
class Scheduler;
class Screen;
class MouseHandler;
class InventoryHandler;
class Parser;
class Route;
class SoundHandler;
class IntroHandler;

class HugoEngine : public Engine {
public:
	HugoEngine(OSystem *syst, const HugoGameDescription *gd);
	~HugoEngine();

	byte   _numVariant;
	byte   _gameVariant;
	byte   _maxInvent;
	byte   _numBonuses;
	byte   _soundSilence;
	byte   _soundTest;
	byte   _tunesNbr;
	uint16 _numScreens;

	object_t *_hero;
	byte  *_screen_p;
	byte  _heroImage;

	byte  *_palette;
	byte  *_introX;
	byte  *_introY;
	byte  *_screenStates;
	char  **_textData;
	char  **_stringtData;
	char  **_screenNames;
	char  **_textEngine;
	char  **_textIntro;
	char  **_textMouse;
	char  **_textParser;
	char  **_textSchedule;
	char  **_textUtil;
	char  ***_arrayNouns;
	char  ***_arrayVerbs;
	uint16    **_arrayReqs;
	hotspot_t *_hotspots;
	int16     *_invent;
	uses_t    *_uses;
	background_t *_catchallList;
	background_t **_backgroundObjects;
	point_t   *_points;
	cmd       **_cmdList;
	uint16    **_screenActs;
	object_t  *_objects;
	act       **_actListArr;
	int16     *_defltTunes;
	uint16    _look;
	uint16    _take;
	uint16    _drop;
	uint16    _numObj;
	uint16    _alNewscrIndex;

	Common::RandomSource *_rnd;

	const char *_episode;
	const char *_picDir;

	char      _initFilename[20];
	char      _saveFilename[20];

	const HugoGameDescription *_gameDescription;
	uint32 getFeatures() const;

	GameType getGameType() const;
	Common::Platform getPlatform() const;
	bool isPacked() const;

	// Temporary, until the engine is fully objectified.
	static HugoEngine &get() {
		assert(s_Engine != NULL);
		return *s_Engine;
	}

	FileManager &file() {
		return *_fileManager;
	}
	Scheduler &scheduler() {
		return *_scheduler;
	}
	Screen &screen() {
		return *_screen;
	}
	MouseHandler &mouse() {
		return *_mouseHandler;
	}
	InventoryHandler &inventory() {
		return *_inventoryHandler;
	}
	Parser &parser() {
		return *_parser;
	}
	Route &route() {
		return *_route;
	}
	SoundHandler &sound() {
		return *_soundHandler;
	}
	IntroHandler &intro() {
		return *_introHandler;
	}

	void initGame(const HugoGameDescription *gd);
	void initGamePart(const HugoGameDescription *gd);
	bool loadHugoDat();

	int getMouseX() const {
		return _mouseX;
	}
	int getMouseY() const {
		return _mouseY;
	}

	void initStatus();
	void readObjectImages();
	void readUIFImages();
	void updateImages();
	void moveObjects();
	void useObject(int16 objId);
	bool findObjectSpace(object_t *obj, int16 *destx, int16 *desty);
	int16 findObject(uint16 x, uint16 y);
	void lookObject(object_t *obj);
	void storeBoundary(int x1, int x2, int y);
	void clearBoundary(int x1, int x2, int y);
	void endGame();
	void readScreenFiles(int screen);
	void setNewScreen(int screen);
	void initNewScreenDisplay();
	void screenActions(int screen);
	void shutdown();

	overlay_t &getBoundaryOverlay() {
		return _boundary;
	}
	overlay_t &getObjectBoundaryOverlay() {
		return _objBound;
	}
	overlay_t &getBaseBoundaryOverlay() {
		return _ovlBase;
	}
	overlay_t &getFirstOverlay() {
		return _overlay;
	}

	status_t &getGameStatus() {
		return _status;
	}

	int getScore() const {
		return _score;
	}
	void setScore(int newScore) {
		_score = newScore;
	}
	void adjustScore(int adjustment) {
		_score += adjustment;
	}
	int getMaxScore() const {
		return _maxscore;
	}
	void setMaxScore(int newScore) {
		_maxscore = newScore;
	}
	byte getIntroSize() {
		return _introXSize;
	}

protected:

	// Engine APIs
	Common::Error run();

private:
	int _mouseX;
	int _mouseY;
	byte _paletteSize;
	byte _introXSize;
	status_t _status;                               // Game status structure

	static HugoEngine *s_Engine;

// The following are bit plane display overlays which mark travel boundaries,
// foreground stationary objects and baselines for those objects (used to
// determine foreground/background wrt moving objects)

// Vinterstum: These shouldn't be static, but we get weird pathfinding issues (and Valgrind warnings) without.
// Needs more investigation. Alignment issues?

	static overlay_t _boundary;                             // Boundary overlay file
	static overlay_t _overlay;                              // First overlay file
	static overlay_t _ovlBase;                              // First overlay base file
	static overlay_t _objBound;                             // Boundary file marks object baselines

	GameType _gameType;
	Common::Platform _platform;
	bool _packedFl;

	FileManager *_fileManager;
	Scheduler *_scheduler;
	Screen *_screen;
	MouseHandler *_mouseHandler;
	InventoryHandler *_inventoryHandler;
	Parser *_parser;
	Route *_route;
	SoundHandler *_soundHandler;
	IntroHandler *_introHandler;

	int _score;                         // Holds current score
	int _maxscore;                      // Holds maximum score

	char **loadTextsVariante(Common::File &in, uint16 *arraySize);
	char ***loadTextsArray(Common::File &in);
	uint16 **loadLongArray(Common::File &in);
	char **loadTexts(Common::File &in);
	void freeTexts(char **ptr);

	void initPlaylist(bool playlist[MAX_TUNES]);
	void initConfig(inst_t action);
	void initialize();
	int deltaX(int x1, int x2, int vx, int y);
	int deltaY(int x1, int x2, int vy, int y);
	void processMaze();
	//int y2comp (const void *a, const void *b);
	char *useBG(char *name);
	void freeObjects();
	void boundaryCollision(object_t *obj);
	void calcMaxScore();

	void initMachine();
	void runMachine();
};

} // End of namespace Hugo

#endif // Hugo_H
