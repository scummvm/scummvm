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
#include "hugo/console.h"
#include "hugo/menu.h"

// This include is here temporarily while the engine is being refactored.
#include "hugo/game.h"
#include "hugo/file.h"

#define HUGO_DAT_VER_MAJ 0                          // 1 byte
#define HUGO_DAT_VER_MIN 41                         // 1 byte
#define DATAALIGNMENT    4

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Hugo engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Hugo's House of Horror
 * - Whodunit?
 * - Jungle of Doom
 * - Hugo's Horrific Adventure
 * - Hugo's Mystery Adventure
 * - Hugo's Amazon Adventure
 */
namespace Hugo {

static const int kSavegameVersion = 3;
static const int kInvDx = 32;                       // Width of an inventory icon
static const int kInvDy = 32;                       // Height of inventory icon
static const int kMaxTunes = 16;                    // Max number of tunes
static const int kStepDx = 5;                       // Num pixels moved in x by HERO per step
static const int kStepDy = 4;                       // Num pixels moved in y by HERO per step
static const int kXPix = 320;                       // Width of pcx background file
static const int kYPix = 200;                       // Height of pcx background file
static const int kViewSizeX = kXPix;                // Width of window view
static const int kViewSizeY = 184;                  // Height of window view
static const int kDibOffY = 0;                      // Offset into dib SrcY (old status line area). In original game: 8
static const int kCompLineSize = 40;                // number of bytes in a compressed line
static const int kMaxLineSize = kCompLineSize - 2;  // Max length of user input line
static const int kMaxTextRows = 25;                 // Number of text lines in display
static const int kMaxBoxChar = kMaxLineSize * kMaxTextRows; // Max chars on screen
static const int kOvlSize = kCompLineSize * kYPix;  // Size of an overlay file
static const int kStateDontCare = 0xFF;             // Any state allowed in command verb
static const int kHeroIndex = 0;                    // In all enums, HERO is the first element
static const int kArrowNumb = 2;                    // Number of arrows (left/right)
static const int kLeftArrow = -2;                   // Cursor over Left arrow in inventory icon bar
static const int kRightArrow = -3;                  // Cursor over Right arrow in inventory icon bar
static const int kMaxPath = 256;                    // Max length of a full path name
static const int kHeroMaxWidth = 24;                // Maximum width of hero
static const int kHeroMinWidth = 16;                // Minimum width of hero
static const int kNumColors = 16;                   // Num colors to save in palette

typedef char fpath_t[kMaxPath];                     // File path
typedef char command_t[kMaxLineSize + 8];           // Command line (+spare for prompt,cursor)

struct PCC_header_t {                               // Structure of PCX file header
	byte   mfctr, vers, enc, bpx;
	uint16  x1, y1, x2, y2;                         // bounding box
	uint16  xres, yres;
	byte   palette[3 * kNumColors];                 // EGA color palette
	byte   vmode, planes;
	uint16 bytesPerLine;                            // Bytes per line
	byte   fill2[60];
};                                                  // Header of a PCC file

struct config_t {                                   // User's config (saved)
	bool musicFl;                                   // State of Music button/menu item
	bool soundFl;                                   // State of Sound button/menu item
	bool turboFl;                                   // State of Turbo button/menu item
	bool playlist[kMaxTunes];                       // Tune playlist
};

typedef byte icondib_t[kXPix * kInvDy];             // Icon bar dib
typedef byte viewdib_t[(long)kXPix * kYPix];        // Viewport dib
typedef byte overlay_t[kOvlSize];                   // Overlay file

enum GameType {
	kGameTypeNone  = 0,
	kGameTypeHugo1,
	kGameTypeHugo2,
	kGameTypeHugo3
};

enum GameVariant {
	kGameVariantH1Win = 0,
	kGameVariantH2Win,
	kGameVariantH3Win,
	kGameVariantH1Dos,
	kGameVariantH2Dos,
	kGameVariantH3Dos
};

enum HugoDebugChannels {
	kDebugSchedule  = 1 <<  0,
	kDebugEngine    = 1 <<  1,
	kDebugDisplay   = 1 <<  2,
	kDebugMouse     = 1 <<  3,
	kDebugParser    = 1 <<  4,
	kDebugFile      = 1 <<  5,
	kDebugRoute     = 1 <<  6,
	kDebugInventory = 1 <<  7,
	kDebugObject    = 1 <<  8,
	kDebugMusic     = 1 <<  9
};

/**
* Ways to dismiss a text/prompt box
*/
enum box_t {kBoxAny, kBoxOk, kBoxPrompt, kBoxYesNo};

/**
* Inventory icon bar states
*/
enum istate_t {kInventoryOff, kInventoryUp, kInventoryDown, kInventoryActive};

/**
* Game view state machine
*/
enum vstate_t {kViewIdle, kViewIntroInit, kViewIntro, kViewPlay, kViewInvent, kViewExit};

/**
* Purpose of an automatic route
*/
enum go_t {kRouteSpace, kRouteExit, kRouteLook, kRouteGet};

/**
* Enumerate whether object is foreground, background or 'floating'
* If floating, HERO can collide with it and fore/back ground is determined
* by relative y-coord of object base.  This is the general case.
* If fore or background, no collisions can take place and object is either
* behind or in front of all others, although can still be hidden by the
* the overlay plane.  OVEROVL means the object is FLOATING (to other
* objects) but is never hidden by the overlay plane
*/
enum {kPriorityForeground, kPriorityBackground, kPriorityFloating, kPriorityOverOverlay};

/**
* Display list functions
*/
enum dupdate_t {kDisplayInit, kDisplayAdd, kDisplayDisplay, kDisplayRestore};

/**
* Priority for sound effect
*/
enum priority_t {kSoundPriorityLow, kSoundPriorityMedium, kSoundPriorityHigh};

enum HugoGameFeatures {
	GF_PACKED = (1 << 0) // Database
};

// Strings used by the engine
enum seqTextEngine {
	kEsAdvertise = 0
};

struct HugoGameDescription;

struct status_t {                                   // Game status (not saved)
	bool     storyModeFl;                           // Game is telling story - no commands
	bool     gameOverFl;                            // Game is over - hero knobbled
	bool     demoFl;                                // Game is in demo mode
	bool     textBoxFl;                             // Game is (halted) in text box
	bool     lookFl;                                // Toolbar "look" button pressed
	bool     recallFl;                              // Toolbar "recall" button pressed
	bool     leftButtonFl;                          // Left mouse button pressed
	bool     rightButtonFl;                         // Right button pressed
	bool     newScreenFl;                           // New screen just loaded in dib_a
	bool     jumpExitFl;                            // Allowed to jump to a screen exit
	bool     godModeFl;                             // Allow DEBUG features in live version
	bool     helpFl;                                // Calling WinHelp (don't disable music)
	bool     doQuitFl;
	bool     skipIntroFl;
	uint32   tick;                                  // Current time in ticks
	vstate_t viewState;                             // View state machine
	istate_t inventoryState;                        // Inventory icon bar state
	int16    inventoryHeight;                       // Inventory icon bar height
	int16    inventoryObjId;                        // Inventory object selected, or -1
	int16    routeIndex;                            // Index into route list, or -1
	go_t     go_for;                                // Purpose of an automatic route
	int16    go_id;                                 // Index of exit of object walking to
	fpath_t  path;                                  // Alternate path for saved files
	int16    song;                                  // Current song
	int16    cx, cy;                                // Cursor position (dib coords)

// Strangerke - Suppress as related to playback
//	bool     playbackFl;                            // Game is in playback mode
//	bool     recordFl;                              // Game is in record mode
// Strangerke - Not used ?
//	bool     mmtimeFl;                              // Multimedia timer supported
//	int16    screenWidth;                           // Desktop screen width
//	uint32   saveTick;                              // Time of last save in ticks
//	int16    saveSlot;                              // Current slot to save/restore game
};

/**
* Structure to define an EXIT or other collision-activated hotspot
*/
struct hotspot_t {
	int        screenIndex;                         // Screen in which hotspot appears
	int        x1, y1, x2, y2;                      // Bounding box of hotspot
	uint16     actIndex;                            // Actions to carry out if a 'hit'
	int16      viewx, viewy, direction;             // Used in auto-route mode
};

class FileManager;
class Scheduler;
class Screen;
class MouseHandler;
class InventoryHandler;
class Parser;
class Route;
class SoundHandler;
class IntroHandler;
class ObjectHandler;
class TextHandler;

class HugoEngine : public Engine {
public:
	HugoEngine(OSystem *syst, const HugoGameDescription *gd);
	~HugoEngine();

	OSystem *_system;

	byte   _numVariant;
	byte   _gameVariant;
	byte   _maxInvent;
	byte   _numBonuses;
	int8   _soundSilence;
	int8   _soundTest;
	int8   _tunesNbr;
	uint16 _numScreens;
	int8   _normalTPS;                              // Number of ticks (frames) per second.
	                                                // 8 for Win versions, 9 for DOS versions
	object_t *_hero;
	byte  *_screen_p;
	byte  _heroImage;

	byte  *_introX;
	byte  *_introY;
	byte  *_screenStates;
	command_t _line;                                // Line of user text input
	config_t  _config;                              // User's config
	uint16    **_arrayReqs;
	hotspot_t *_hotspots;
	int16     *_invent;
	uses_t    *_uses;
	uint16     _usesSize;
	background_t *_catchallList;
	background_t **_backgroundObjects;
	uint16    _backgroundObjectsSize;
	point_t   *_points;
	cmd       **_cmdList;
	uint16    _cmdListSize;
	uint16    **_screenActs;
	uint16    _screenActsSize;
	int16     *_defltTunes;
	uint16    _look;
	uint16    _take;
	uint16    _drop;

	GUI::Debugger *getDebugger() { return _console; }

	Common::RandomSource *_rnd;

	const char *_episode;
	const char *_picDir;

	Common::String _saveFilename;

	command_t _statusLine;
	command_t _scoreLine;

	const HugoGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	GameType getGameType() const;
	Common::Platform getPlatform() const;
	bool isPacked() const;

	// Temporary, until the engine is fully objectified.
	static HugoEngine &get() {
		assert(s_Engine != 0);
		return *s_Engine;
	}

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	bool loadHugoDat();

	char *useBG(char *name);

	int  deltaX(int x1, int x2, int vx, int y);
	int  deltaY(int x1, int x2, int vy, int y);

	int8 getTPS();

	void initGame(const HugoGameDescription *gd);
	void initGamePart(const HugoGameDescription *gd);
	void boundaryCollision(object_t *obj);
	void clearBoundary(int x1, int x2, int y);
	void clearScreenBoundary(int x1, int x2, int y);
	void endGame();
	void initStatus();
	void readScreenFiles(int screen);
	void screenActions(int screen);
	void setNewScreen(int screen);
	void shutdown();
	void storeBoundary(int x1, int x2, int y);
	void syncSoundSettings();

	int getMouseX() const {
		return _mouseX;
	}
	int getMouseY() const {
		return _mouseY;
	}

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
	Common::Error saveGameState(int slot, const char *desc) {
		return (_file->saveGame(slot, desc) ? Common::kWritingFailed : Common::kNoError);
	}

	Common::Error loadGameState(int slot) {
		return (_file->restoreGame(slot) ? Common::kReadingFailed : Common::kNoError);
	}

	bool hasFeature(EngineFeature f) const {
		return (f == kSupportsRTL) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
	}

	const char *getCopyrightString() { return "Copyright 1989-1997 David P Gray, All Rights Reserved."; }


	FileManager *_file;
	Scheduler *_scheduler;
	Screen *_screen;
	MouseHandler *_mouse;
	InventoryHandler *_inventory;
	Parser *_parser;
	Route *_route;
	SoundHandler *_sound;
	IntroHandler *_intro;
	ObjectHandler *_object;
	TextHandler *_text;

	TopMenu *_topMenu;

protected:

	// Engine APIs
	Common::Error run();

private:
	static const int kTurboTps = 16;                // This many in turbo mode

	int _mouseX;
	int _mouseY;
	byte _introXSize;
	status_t _status;                               // Game status structure

	static HugoEngine *s_Engine;

	HugoConsole *_console;

// The following are bit plane display overlays which mark travel boundaries,
// foreground stationary objects and baselines for those objects (used to
// determine foreground/background wrt moving objects)

// Vinterstum: These shouldn't be static, but we get weird pathfinding issues (and Valgrind warnings) without.
// Needs more investigation. Alignment issues?

	static overlay_t _boundary;                     // Boundary overlay file
	static overlay_t _overlay;                      // First overlay file
	static overlay_t _ovlBase;                      // First overlay base file
	static overlay_t _objBound;                     // Boundary file marks object baselines

	GameType _gameType;
	Common::Platform _platform;
	bool _packedFl;

	int _score;                                     // Holds current score
	int _maxscore;                                  // Holds maximum score

	uint16 **loadLongArray(Common::File &in);

	void initPlaylist(bool playlist[kMaxTunes]);
	void initConfig();
	void initialize();
	void initMachine();
	void calcMaxScore();
	void resetConfig();
	void runMachine();

};

} // End of namespace Hugo

#endif // Hugo_H
