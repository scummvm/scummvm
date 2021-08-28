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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "graphics/surface.h"

#include "saga2/saga2.h"
#include "saga2/blitters.h"
#include "saga2/objects.h"
#include "saga2/tile.h"
#include "saga2/contain.h"
#include "saga2/grabinfo.h"
#include "saga2/intrface.h"
#include "saga2/fontlib.h"

namespace Saga2 {

const uint32            imageGroupID = MKTAG('I', 'M', 'A', 'G');

const int defaultStatusWait = 15;

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern BackWindow   *mainWindow;

extern APPFUNC(cmdClickSpeech);
extern StaticTextPallete genericTextPal;

APPFUNC(cmdHealthStar);

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

int16 ScreenDepth(TilePoint);


#ifdef FTA

void cleanupButtonImages(void);

#endif


/* ===================================================================== *
   Temporary
 * ===================================================================== */

//void drawInventory( gPort &port, GameObject &container, Rect16 displayRect );
void motionTest(void);
void drawBorders(void);

void windowTest(void);
void closeAllFloatingWindows(void);

void objectTest(void);

#ifdef hasReadyContainers
void readyContainerSetup(void);
#endif

/* ===================================================================== *
   PlayMode definition
 * ===================================================================== */
void PlayModeSetup(void);
void PlayModeCleanup(void);

//  The Mode object for the main "play" mode.

GameMode            PlayMode = {
	nullptr,                                   // no previous mode
	false,                                  // mode is not nestable
	PlayModeSetup,
	PlayModeCleanup,
	nullptr,
	nullptr,
	nullptr,
};

/* ===================================================================== *
   Timing Variables
 * ===================================================================== */

Alarm               frameAlarm;             // 10 fps frame rate

/* ===================================================================== *
   User controls
 * ===================================================================== */

//  list of all controls in playMode
gPanelList          *playControls,          // panelList of play controls
                    *tileControls,          // controls specific to tile mode
                    *trioControls,          // controls for all three brothers
                    *indivControls,         // controls for single brother
                    *speakButtonControls;   // controls for embedded speech button

gGenericControl     *speakButtonPanel;      // panel for hitting speech buttons

ContainerView       *invContainer;          // TEST inventory container

//  Drag and Drop variables

static StaticPixelMap objPointerMap = {{0, 0}, nullptr};          // bitmap for pointer

//  Resource handle for UI imagery

hResContext         *imageRes;              // image resource handle

//-----------------------------------------------------------------------
//	Initialize the Play mode

bool checkTileAreaPort(void) {
	if (g_vm->_gameRunning && g_vm->_tileDrawMap.data == nullptr) {
		//  Allocate back buffer for tile rendering
		g_vm->_tileDrawMap.size.x = (kTileRectWidth + kTileWidth - 1) & ~kTileDXMask;
		g_vm->_tileDrawMap.size.y = (kTileRectHeight + kTileWidth - 1) & ~kTileDXMask;
		g_vm->_tileDrawMap.data = new uint8[g_vm->_tileDrawMap.bytes()]();
	}

	return g_vm->_tileDrawMap.data != nullptr;
}

void clearTileAreaPort(void) {
	if (g_vm->_gameRunning && g_vm->_tileDrawMap.data != nullptr) {
		_FillRect(g_vm->_tileDrawMap.data, g_vm->_tileDrawMap.size.x, g_vm->_tileDrawMap.size.x, g_vm->_tileDrawMap.size.y, 0);
	}

	Rect16 rect(0, 0, 640, 480);
	mainWindow->invalidate(&rect);
}


void PlayModeSetup(void) {
	//  Init resources for images
	if (imageRes == nullptr)
		imageRes = resFile->newContext(imageGroupID, "image resources");

	//  Init resources needed by containers.
	initContainers();

	if (!checkTileAreaPort()) {
		error("Unable to allocate memory for tile draw map");
	}

	//  Setup the drawing port for the background map
	g_vm->_backPort.setMap(&g_vm->_tileDrawMap);

	//  Allocate bitmap for drag & drop mouse pointer
	objPointerMap.size.x = objPointerMap.size.y = 32;
	objPointerMap.data = new uint8[objPointerMap.bytes()];

	//  Create a panelList to contain all controls created
	//  for tile mode.
	//  NOTE: Make sure these are allocated first, so that they
	//  can over-ride other controls.
	speakButtonControls = new gPanelList(*mainWindow);

	//  Create a control covering the map area.
	speakButtonPanel = new gGenericControl(*speakButtonControls,
	                   Rect16(0, 0, screenWidth, screenHeight),
	                   0,
	                   cmdClickSpeech);
	speakButtonControls->enable(false);

	//  Create a panelList to contain all controls created
	//  for play mode.
	playControls = new gPanelList(*mainWindow);

	//  Create a panelList to contain all controls created
	//  for tile mode.
	tileControls = new gPanelList(*mainWindow);

	//  Create a panelList to contain all controls created
	//  for stage mode.

	tileControls->enable(false);

	// activate the status line
	// this will get deleted by parent panel
	StatusLine = new CStatusLine(*playControls,
	                        Rect16(49, 445, 407, 15),
	                        "",
	                        &Script10Font,
	                        0,
	                        genericTextPal,
	                        defaultStatusWait,    // in frames
	                        0,
	                        (void (*)(gEvent&))nullptr);

	// placement configurations
	Point16 massWeightIndicator = Point16(531, 265);

	// activate the indiv mode character mass and weight indicator
	MassWeightIndicator = new CMassWeightIndicator(indivControls, massWeightIndicator);

	// activate the plyaer health indicator
	HealthIndicator = new CHealthIndicator(cmdHealthStar);


	SetupUserControls();


	//  Set up mouse cursor
	g_vm->_mouseInfo = new GrabInfo;
	g_vm->_mouseInfo->setIntent(GrabInfo::WalkTo);

	//  Start by displaying first frame stright off, no delay
	frameAlarm.set(0);

	//  Test to draw borders.
	//  REM: We should actually have a routine to refresh the window...
	mainWindow->draw();

//	windowTest();
	objectTest();
//	controlSetup();
}

//-----------------------------------------------------------------------
//	Cleanup function for Play mode

void PlayModeCleanup(void) {
	closeAllFloatingWindows();
	if (playControls) {
		if (StatusLine)
			delete StatusLine;
		StatusLine = nullptr;
		delete playControls;
		playControls = nullptr;
	}
	if (speakButtonControls) {
		delete speakButtonControls;
		speakButtonControls = nullptr;
	}

	// delete standalone indicators
	delete MassWeightIndicator;
	delete HealthIndicator;

	delete g_vm->_mouseInfo;

	// dallocate compressed button images

	CleanupUserControls();

	//  Deallocate back buffer for tile rendering
	if (g_vm->_tileDrawMap.data) {
		delete[] g_vm->_tileDrawMap.data;
		g_vm->_tileDrawMap.data = nullptr;
	}

	if (objPointerMap.data) {
		delete[] objPointerMap.data;
		objPointerMap.data = nullptr;
	}

	mainWindow->removeDecorations();

	if (imageRes)
		resFile->disposeContext(imageRes);
	imageRes = nullptr;

	cleanupContainers();
}

/* ===================================================================== *
   Function to uncompress a run-length image into a pixel map
   (No clipping is done: The map must be big enough)

   REM: These should probably be moved elsewhere...
 * ===================================================================== */

extern void unpackImage(gPixelMap *map,
                                  int32 width,
                                  int32 rowCount,
                                  int8 *srcData);

typedef struct {
	Point16     size;
	int16       compress;
	int8        data[2];
} ImageHeader;

void drawCompressedImage(gPort &port, const Point16 pos, void *image) {

	ImageHeader     *hdr = (ImageHeader *)image;
	gPixelMap       map;

	map.size = hdr->size;

	if (hdr->compress) {
		map.data = new uint8[map.bytes()];
		if (map.data == nullptr)
			return;

		unpackImage(&map, map.size.x, map.size.y, hdr->data);
	} else
		map.data = (uint8 *)hdr->data;

	port.setMode(drawModeMatte);

	port.bltPixels(map, 0, 0,
	               pos.x, pos.y,
	               map.size.x, map.size.y);


	if (hdr->compress)
		delete[] map.data;
}

void drawCompressedImageGhosted(gPort &port, const Point16 pos, void *image) {
	ImageHeader     *hdr = (ImageHeader *)image;
	gPixelMap       map;
	uint8           *row;
	int16           x, y;

	map.size = hdr->size;

	map.data = new uint8[map.bytes()];
	if (map.data == nullptr)
		return;

	if (hdr->compress)
		unpackImage(&map, map.size.x, map.size.y, hdr->data);
	else
		memcpy(map.data, hdr->data, map.bytes());

	for (y = 0, row = map.data; y < map.size.y; y++, row += map.size.x) {
		for (x = (y & 1); x < map.size.x; x += 2) row[x] = 0;
	}

	port.setMode(drawModeMatte);
	port.bltPixels(map, 0, 0,
	               pos.x, pos.y,
	               map.size.x, map.size.y);

	delete[] map.data;
}

void drawCompressedImageToMap(gPixelMap &map, void *image) {
	// get the header for the image pointer passed
	ImageHeader     *hdr = (ImageHeader *)image;

	// set the buffer blit area to the image size
	map.size = hdr->size;

	// see if it's compressed
	if (hdr->compress) {
		// if it is then upack it to spec'ed coords.
		unpackImage(&map, map.size.x, map.size.y, hdr->data);
	} else
		map.data = (uint8 *)hdr->data;
}


int16 ScreenDepth(TilePoint tp) {
	return (tp.u + tp.v - tp.z);
}

} // end of namespace Saga2
