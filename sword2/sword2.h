/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "base/engine.h"
#include "sword2/driver/driver96.h"
#include "sword2/driver/d_sound.h"
#include "sword2/driver/d_draw.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/icons.h"
#include "sword2/layers.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/resman.h"
#include "sword2/save_rest.h"
#include "sword2/sound.h"

class GameDetector;

namespace Sword2 {

enum {
	GF_DEMO	= 1 << 0
};

class Sword2Engine : public Engine {
private:
	bool _quit;
	uint32 _bootParam;
	int32 _saveSlot;

	// structure filled out by each object to register its graphic printing
	// requrements

	struct buildit {
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

	buildit	_bgp0List[MAX_bgp0_sprites];
	buildit	_bgp1List[MAX_bgp1_sprites];
	buildit	_backList[MAX_back_sprites];
	buildit	_sortList[MAX_sort_sprites];
	buildit	_foreList[MAX_fore_sprites];
	buildit	_fgp0List[MAX_fgp0_sprites];
	buildit	_fgp1List[MAX_fgp1_sprites];

	// Holds the order of the sort list, i.e. the list stays static and we
	// sort this array.

	uint16 _sortOrder[MAX_sort_sprites];

	// Last palette used - so that we can restore the correct one after a
	// pause (which dims the screen) and it's not always the main screen
	// palette that we want, eg. during the eclipse

	// This flag gets set in startNewPalette() and setFullPalette()

	uint32 _lastPaletteRes;

	void sendBackPar0Frames(void);
	void sendBackPar1Frames(void);
	void sendBackFrames(void);
	void sendSortFrames(void);
	void sendForeFrames(void);
	void sendForePar0Frames(void);
	void sendForePar1Frames(void);

	void sortTheSortList(void);

	void startNewPalette(void);
	void processLayer(uint32 layer_number);

	void getPlayerStructures(void);
	void putPlayerStructures(void);

	uint32 saveData(uint16 slotNo, uint8 *buffer, uint32 bufferSize);
	uint32 restoreData(uint16 slotNo, uint8 *buffer, uint32 bufferSize);

	uint32 calcChecksum(uint8 *buffer, uint32 size);

	void pauseGame(void);
	void unpauseGame(void);

public:
	Sword2Engine(GameDetector *detector, OSystem *syst);
	~Sword2Engine();
	void go(void);
	int32 initialiseGame(void);
	uint32 _features;
	char *_targetName; // target name for saves

	MemoryManager *_memory;
	ResourceManager	*_resman;
	Input *_input;
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

	void resetRenderLists(void);
	void buildDisplay(void);
	void processImage(buildit *build_unit);
	void displayMsg(uint8 *text, int time);
	void removeMsg(void);
	void setFullPalette(int32 palRes);

	int32 registerFrame(int32 *params);
	void registerFrame(int32 *params, buildit *build_unit);

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
	bool _grabbingSequences;

	int32 _gameCycle;
	bool _renderSkip;

	int32 initBackground(int32 res, int32 new_palette);

	// Set by fnPassMega()
	Object_mega _engineMega;

	menu_object _tempList[TOTAL_engine_pockets];
	uint32 _totalTemp;

	menu_object _masterMenuList[TOTAL_engine_pockets];
	uint32 _totalMasters;

	int menuClick(int menu_items);

	void buildMenu(void);
	void buildSystemMenu(void);

	// _thisScreen describes the current back buffer and its in-game scroll
	// positions, etc.

	screen_info _thisScreen;

	void setUpBackgroundLayers(void);

	uint32 _curMouse;
	Mouse_unit _mouseList[TOTAL_mouse_list];

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

	void registerMouse(Object_mouse *ob_mouse);

	uint8 *fetchPalette(uint8 *screenFile);
	_screenHeader *fetchScreenHeader(uint8 *screenFile);
	_layerHeader *fetchLayerHeader(uint8 *screenFile, uint16 layerNo);
	uint8 *fetchShadingMask(uint8 *screenFile);

	_animHeader *fetchAnimHeader(uint8 *animFile);
	_cdtEntry *fetchCdtEntry(uint8 *animFile, uint16 frameNo);
	_frameHeader *fetchFrameHeader(uint8 *animFile, uint16 frameNo);
	_parallax *fetchBackgroundParallaxLayer(uint8 *screenFile, int layer);
	_parallax *fetchBackgroundLayer(uint8 *screenFile);
	_parallax *fetchForegroundParallaxLayer(uint8 *screenFile, int layer);
	uint8 *fetchTextLine(uint8 *file, uint32 text_line);
	bool checkTextLine(uint8 *file, uint32 text_line);
	uint8 *fetchPaletteMatchTable(uint8 *screenFile);
	uint8 *fetchObjectName(int32 resourceId);

	// savegame file header

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
		_object_hub player_hub;	// copy of player object's object_hub structure
		Object_logic logic;	// copy of player character logic structure

		// copy of player character graphic structure
		Object_graphic	graphic;

		// copy of player character mega structure
		Object_mega mega;
	};

	SaveGameHeader _saveGameHeader;

	uint32 saveGame(uint16 slotNo, uint8 *description);
	uint32 restoreGame(uint16 slotNo);
	uint32 getSaveDescription(uint16 slotNo, uint8 *description);
	bool saveExists(uint16 slotNo);
	void fillSaveBuffer(mem *buffer, uint32 size, uint8 *desc);
	uint32 restoreFromBuffer(mem *buffer, uint32 size);
	uint32 findBufferSize(void);

	uint8 _scrollFraction;

	void setScrolling(void);

	struct _fxq_entry {
		uint32 resource;	// resource id of sample
		uint32 fetchId;		// Id of resource in PSX CD queue. :)
		uint16 delay;		// cycles to wait before playing (or 'random chance' if FX_RANDOM)
		uint8 volume;		// 0..16
		int8 pan;		// -16..16
		uint8 type;		// FX_SPOT, FX_RANDOM or FX_LOOP
	};

	_fxq_entry _fxQueue[FXQ_LENGTH];

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
