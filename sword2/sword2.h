/* Copyright (C) 1994-2004 Revolution Software Ltd
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

#include "sword2/driver/driver96.h"
#include "sword2/build_display.h"
#include "sword2/header.h"
#include "sword2/icons.h"
#include "sword2/layers.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/save_rest.h"
#include "sword2/sound.h"

class GameDetector;

namespace Sword2 {

enum {
	GF_DEMO	= 1 << 0
};

class ResourceManager;
class Sound;
class Graphics;
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
	uint16 ascii;
	int keycode;
	int modifiers;
};

class Sword2Engine : public Engine {
private:
	uint32 _eventFilter;

	// The event "buffers"
	MouseEvent _mouseEvent;
	KeyboardEvent _keyboardEvent;

	uint32 _bootParam;
	int32 _saveSlot;

	// structure filled out by each object to register its graphic printing
	// requrements

	struct BuildUnit {
		int16 x;
		int16 y;
		uint16 scaled_width;
		uint16 scaled_height;
		int16 sort_y;
		uint32 anim_resource;
		uint16 anim_pc;

		// Denotes a scaling sprite at print time - and holds the
		// scaling value for the shrink routine

		uint16 scale;

		// Non-zero means this item is a layer - retrieve from
		// background layer and send to special renderer

		uint16 layer_number;

		// True means we want this frame to be affected by the shading
		// mask

		bool shadingFlag;
	};

	BuildUnit _bgp0List[MAX_bgp0_sprites];
	BuildUnit _bgp1List[MAX_bgp1_sprites];
	BuildUnit _backList[MAX_back_sprites];
	BuildUnit _sortList[MAX_sort_sprites];
	BuildUnit _foreList[MAX_fore_sprites];
	BuildUnit _fgp0List[MAX_fgp0_sprites];
	BuildUnit _fgp1List[MAX_fgp1_sprites];

	// Holds the order of the sort list, i.e. the list stays static and we
	// sort this array.

	uint16 _sortOrder[MAX_sort_sprites];

	// Last palette used - so that we can restore the correct one after a
	// pause (which dims the screen) and it's not always the main screen
	// palette that we want, eg. during the eclipse

	// This flag gets set in startNewPalette() and setFullPalette()

	uint32 _lastPaletteRes;

	void drawBackPar0Frames(void);
	void drawBackPar1Frames(void);
	void drawBackFrames(void);
	void drawSortFrames(byte *file);
	void drawForeFrames(void);
	void drawForePar0Frames(void);
	void drawForePar1Frames(void);

	void startNewPalette(void);
	void processLayer(byte *file, uint32 layer_number);
	void processImage(BuildUnit *build_unit);

	void getPlayerStructures(void);
	void putPlayerStructures(void);

	uint32 saveData(uint16 slotNo, byte *buffer, uint32 bufferSize);
	uint32 restoreData(uint16 slotNo, byte *buffer, uint32 bufferSize);

	uint32 calcChecksum(byte *buffer, uint32 size);

	void pauseGame(void);
	void unpauseGame(void);

	MenuObject _tempList[TOTAL_engine_pockets];
	uint32 _totalTemp;

	MenuObject _masterMenuList[TOTAL_engine_pockets];
	uint32 _totalMasters;

public:
	Sword2Engine(GameDetector *detector, OSystem *syst);
	~Sword2Engine();
	void go(void);
	int32 initialiseGame(void);

	bool _quit;

	uint32 _features;
	char *_targetName; // target name for saves

	MemoryManager *_memory;
	ResourceManager	*_resman;
	Sound *_sound;
	Graphics *_graphics;
	Logic *_logic;
	FontRenderer *_fontRenderer;
	Gui *_gui;

	Debugger *_debugger;

	Common::RandomSource _rnd;

	uint32 _speechFontId;
	uint32 _controlsFontId;
	uint32 _redFontId;

	int16 _mouseX;
	int16 _mouseY;

	uint32 setEventFilter(uint32 filter);

	void parseEvents(void);

	bool checkForMouseEvents(void);
	MouseEvent *mouseEvent(void);
	KeyboardEvent *keyboardEvent(void);

	void resetRenderLists(void);
	void buildDisplay(void);
	void displayMsg(byte *text, int time);
	void removeMsg(void);
	void setFullPalette(int32 palRes);

	int32 registerFrame(int32 *params);
	void registerFrame(int32 *params, BuildUnit *build_unit);

	// The debugger wants to access these

	uint32 _curBgp0;
	uint32 _curBgp1;
	uint32 _curBack;
	uint32 _curSort;
	uint32 _curFore;
	uint32 _curFgp0;
	uint32 _curFgp1;

	// Debugging stuff

	uint32 _largestLayerArea;
	uint32 _largestSpriteArea;
	char _largestLayerInfo[128];
	char _largestSpriteInfo[128];

	// 'frames per second' counting stuff
	uint32 _fps;
	uint32 _cycleTime;
	uint32 _frameCount;

	bool _wantSfxDebug;

	int32 _gameCycle;
	bool _renderSkip;

	int32 initBackground(int32 res, int32 new_palette);

#if RIGHT_CLICK_CLEARS_LUGGAGE
	bool heldIsInInventory(void);
#endif

	int menuClick(int menu_items);

	void addMenuObject(MenuObject *obj);
	void buildMenu(void);
	void buildSystemMenu(void);

	// _thisScreen describes the current back buffer and its in-game scroll
	// positions, etc.

	ScreenInfo _thisScreen;

	uint32 _curMouse;
	MouseUnit _mouseList[TOTAL_mouse_list];

	// Set by checkMouseList()
	uint32 _mouseTouching;
	uint32 _oldMouseTouching;

	uint32 _menuSelectedPos;

	// If it's NORMAL_MOUSE_ID (ie. normal pointer) then it's over a floor
	// area (or hidden hot-zone)

	uint32 _mousePointerRes;

	uint32 _mouseMode;
	bool _examiningMenuIcon;

	bool _mouseStatus;		// Human 0 on/1 off
	bool _mouseModeLocked;		// 0 not !0 mode cannot be changed from
					// normal mouse to top menu (i.e. when
					// carrying big objects)
	uint32 _realLuggageItem;	// Last minute for pause mode
	uint32 _currentLuggageResource;
	uint32 _oldButton;		// For the re-click stuff - must be
					// the same button you see
	uint32 _buttonClick;
	uint32 _pointerTextBlocNo;
	uint32 _playerActivityDelay;	// Player activity delay counter

	void resetMouseList(void);

	void normalMouse(void);
	void menuMouse(void);
	void dragMouse(void);
	void systemMenuMouse(void);

	void mouseOnOff(void);
	uint32 checkMouseList(void);
	void mouseEngine(void);

	void setMouse(uint32 res);
	void setLuggage(uint32 res);

	void clearPointerText(void);

	void createPointerText(uint32 text_id, uint32 pointer_res);
	void monitorPlayerActivity(void);
	void noHuman(void);

	void registerMouse(ObjectMouse *ob_mouse);

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
	bool saveExists(void);
	bool saveExists(uint16 slotNo);
	void fillSaveBuffer(byte *buffer, uint32 size, byte *desc);
	uint32 restoreFromBuffer(byte *buffer, uint32 size);
	uint32 findBufferSize(void);

	uint8 _scrollFraction;

	void setScrolling(void);

	// used to store id of tunes that loop, for save & restore
	uint32 _loopingMusicId;

	// to be called during system initialisation
	void initFxQueue(void);

	// to be called from the main loop, once per cycle
	void processFxQueue(void);

	// stops all fx & clears the queue - eg. when leaving a location
	void clearFxQueue(void);

	void pauseAllSound(void);
	void unpauseAllSound(void);

	void killMusic(void);

	void triggerFx(uint8 j);

	bool _gamePaused;
	bool _graphicsLevelFudged;
	bool _stepOneCycle;		// for use while game paused

	void startGame(void);
	void gameCycle(void);
	void closeGame(void);

	void sleepUntil(uint32 time);

	void errorString(const char *buf_input, char *buf_output);
	void initialiseFontResourceFlags(void);
	void initialiseFontResourceFlags(uint8 language);
};

extern Sword2Engine *g_sword2;

} // End of namespace Sword2

#endif
