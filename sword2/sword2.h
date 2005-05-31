/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	_SWORD2
#define	_SWORD2

// Enable this to make it possible to clear the mouse cursor luggage by
// right-clicking. The original didn't do this, but it feels natural to me.
// However, I'm afraid that it'll interfer badly with parts of the game, so
// for now I'll keep it disabled.

#define RIGHT_CLICK_CLEARS_LUGGAGE 0

#include "base/engine.h"

#include "common/util.h"

#include "sword2/build_display.h"
#include "sword2/header.h"
#include "sword2/icons.h"
#include "sword2/object.h"
#include "sword2/save_rest.h"

#define	MAX_starts	100
#define	MAX_description	100

class GameDetector;
class OSystem;

namespace Sword2 {

enum {
	GF_DEMO	= 1 << 0
};

class MemoryManager;
class ResourceManager;
class Sound;
class Screen;
class Mouse;
class Logic;
class FontRenderer;
class Gui;
class Debugger;

enum {
	RD_LEFTBUTTONDOWN		= 0x01,
	RD_LEFTBUTTONUP			= 0x02,
	RD_RIGHTBUTTONDOWN		= 0x04,
	RD_RIGHTBUTTONUP		= 0x08,
	RD_WHEELUP			= 0x10,
	RD_WHEELDOWN			= 0x20,
	RD_KEYDOWN			= 0x40,
	RD_MOUSEMOVE			= 0x80
};

struct MouseEvent {
	bool pending;
	uint16 buttons;
};

struct KeyboardEvent {
	bool pending;
	uint32 repeat;
	uint16 ascii;
	int keycode;
	int modifiers;
};

struct StartUp {
	char description[MAX_description];

	// id of screen manager object
	uint32 start_res_id;

	// Tell the manager which startup you want (if there are more than 1)
	// (i.e more than 1 entrance to a screen and/or separate game boots)
	uint32 key;
};

class Sword2Engine : public Engine {
private:
	uint32 _eventFilter;

	// The event "buffers"
	MouseEvent _mouseEvent;
	KeyboardEvent _keyboardEvent;

	uint32 _bootParam;
	int32 _saveSlot;

	void getPlayerStructures();
	void putPlayerStructures();

	uint32 saveData(uint16 slotNo, byte *buffer, uint32 bufferSize);
	uint32 restoreData(uint16 slotNo, byte *buffer, uint32 bufferSize);

	uint32 calcChecksum(byte *buffer, uint32 size);

	void pauseGame();
	void unpauseGame();

	uint32 _totalStartups;
	uint32 _totalScreenManagers;
	uint32 _startRes;

	bool _useSubtitles;

	StartUp _startList[MAX_starts];

public:
	Sword2Engine(GameDetector *detector, OSystem *syst);
	~Sword2Engine();
	int go();
	int init(GameDetector &detector);

	void registerDefaultSettings();
	void readSettings();
	void writeSettings();

	void setupPersistentResources();

	bool getSubtitles() { return _useSubtitles; }
	void setSubtitles(bool b) { _useSubtitles = b; }

	bool _quit;

	uint32 _features;
	Common::String _targetName; // target name for saves

	MemoryManager *_memory;
	ResourceManager	*_resman;
	Sound *_sound;
	Screen *_screen;
	Mouse *_mouse;
	Logic *_logic;
	FontRenderer *_fontRenderer;

	Debugger *_debugger;

	Common::RandomSource _rnd;

	uint32 _speechFontId;
	uint32 _controlsFontId;
	uint32 _redFontId;

	uint32 setEventFilter(uint32 filter);

	void parseEvents();

	bool checkForMouseEvents();
	MouseEvent *mouseEvent();
	KeyboardEvent *keyboardEvent();

	bool _wantSfxDebug;

	int32 _gameCycle;

#ifdef SWORD2_DEBUG
	bool _renderSkip;
	bool _stepOneCycle;
#endif

#if RIGHT_CLICK_CLEARS_LUGGAGE
	bool heldIsInInventory();
#endif

	byte *fetchPalette(byte *screenFile);
	ScreenHeader *fetchScreenHeader(byte *screenFile);
	LayerHeader *fetchLayerHeader(byte *screenFile, uint16 layerNo);
	byte *fetchShadingMask(byte *screenFile);

	AnimHeader *fetchAnimHeader(byte *animFile);
	CdtEntry *fetchCdtEntry(byte *animFile, uint16 frameNo);
	FrameHeader *fetchFrameHeader(byte *animFile, uint16 frameNo);
	Parallax *fetchBackgroundParallaxLayer(byte *screenFile, int layer);
	Parallax *fetchBackgroundLayer(byte *screenFile);
	Parallax *fetchForegroundParallaxLayer(byte *screenFile, int layer);
	byte *fetchTextLine(byte *file, uint32 text_line);
	bool checkTextLine(byte *file, uint32 text_line);
	byte *fetchPaletteMatchTable(byte *screenFile);
	byte *fetchObjectName(int32 resourceId, byte *buf);

	// savegame file header

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

	struct SaveGameHeader {
		// sum of all bytes in file, excluding this uint32
		uint32 checksum;

		// player's description of savegame
		char description[SAVE_DESCRIPTION_LEN];

		uint32 varLength;	// length of global variables resource
		uint32 screenId;	// resource id of screen file
		uint32 runListId;	// resource id of run list
		uint32 feet_x;		// copy of _thisScreen.feet_x
		uint32 feet_y;		// copy of _thisScreen.feet_y
		uint32 music_id;	// copy of 'looping_music_id'
		ObjectHub player_hub;	// copy of player object's object_hub structure
		ObjectLogic logic;	// copy of player character logic structure
		ObjectGraphic graphic;	// copy of player character graphic structure
		ObjectMega mega;	// copy of player character mega structure
	} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

	SaveGameHeader _saveGameHeader;

	uint32 saveGame(uint16 slotNo, byte *description);
	uint32 restoreGame(uint16 slotNo);
	uint32 getSaveDescription(uint16 slotNo, byte *description);
	bool saveExists();
	bool saveExists(uint16 slotNo);
	void fillSaveBuffer(byte *buffer, uint32 size, byte *desc);
	uint32 restoreFromBuffer(byte *buffer, uint32 size);
	uint32 findBufferSize();

	bool _gamePaused;
	bool _graphicsLevelFudged;

	void startGame();
	void gameCycle();
	void closeGame();
	void restartGame();

	void sleepUntil(uint32 time);

	void errorString(const char *buf_input, char *buf_output);
	void initialiseFontResourceFlags();
	void initialiseFontResourceFlags(uint8 language);

	bool initStartMenu();
	void registerStartPoint(int32 key, char *name);

	uint32 getNumStarts() { return _totalStartups; }
	uint32 getNumScreenManagers() { return _totalScreenManagers; }
	StartUp *getStartList() { return _startList; }

	void runStart(int start);

	// Convenience alias for OSystem::getMillis().
	// This is a bit hackish, of course :-).
	uint32 getMillis();
};

} // End of namespace Sword2

#endif
