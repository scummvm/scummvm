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
#include "common/file.h"
#include "common/map.h"
#include "common/rect.h"
#include "common/str.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/driver/d_sound.h"
#include "sword2/driver/d_draw.h"

enum {
	GF_DEMO	= 1 << 0
};

class NewGui;

namespace Sword2 {

// Bodge for PCF76 version so that their demo CD can be labelled "PCF76"
// rather than "RBSII1"

#ifdef _PCF76
	#define CD1_LABEL	"PCF76"
#else
	#define CD1_LABEL	"RBSII1"
#endif

#define	CD2_LABEL		"RBSII2"

void Close_game();

void PauseGame(void);
void UnpauseGame(void);

void sleepUntil(int32 time);

// TODO move stuff into class

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

public:
	Sword2Engine(GameDetector *detector, OSystem *syst);
	~Sword2Engine();
	void go(void);
	void parseEvents(void);
	void Start_game(void);
	int32 InitialiseGame(void);
	GameDetector *_detector;
	uint32 _features;
	char *_targetName; // target name for saves

	Sound *_sound;
	Display *_display;

	NewGui *_newgui;
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

	// So I know if the control panel can be activated
	int32 _mouseStatus;

	// Debugging stuff

	uint32 _largestLayerArea;
	uint32 _largestSpriteArea;
	char _largestLayerInfo[128];
	char _largestSpriteInfo[128];

	// 'frames per second' counting stuff
	uint32 _fps;
	uint32 _cycleTime;
	uint32 _frameCount;

	int32 initBackground(int32 res, int32 new_palette);

	void errorString(const char *buf_input, char *buf_output);
	void initialiseFontResourceFlags(void);
	void initialiseFontResourceFlags(uint8 language);
};

extern Sword2Engine *g_sword2;
extern Sound *g_sound;
extern Display *g_display; 

} // End of namespace Sword2

#endif
