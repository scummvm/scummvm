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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/automap.h"
#include "saga2/blitters.h"
#include "saga2/tile.h"
#include "saga2/grequest.h"
#include "saga2/mapfeatr.h"
#include "saga2/grabinfo.h"

namespace Saga2 {

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern BackWindow           *mainWindow;
extern WorldMapData         *mapList;
extern GameWorld            *currentWorld;

/* ===================================================================== *
   Locals
 * ===================================================================== */

requestInfo     rInfo;

#if DEBUG
bool autoMapCheat = false;
#endif

static CAutoMap     *pAutoMap = NULL;

/* ===================================================================== *
   Constants
 * ===================================================================== */

// ------------------------------------------------------------------------
// Resource IDs

enum autoMapPanelResIDs {
	autoMapTopPanelResID = 0,
	autoMapMidPanelResID,
	autoMapBotPanelResID
};


// ------------------------------------------------------------------------
// Panel and control boundaries

const   int16       numAutoMapPanels    = 3;


const   int16       autoMapBoxXSize     = 544,
                    autoMapBoxXNSSize   = 535,  // without shadow
                    autoMapBoxYNSSize   = 318,
                    autoMapBoxYSize     = 324,
                    autoMapBoxX         = (640 - autoMapBoxXNSSize) / 2, //16,
                    autoMapBoxY         = (480 - autoMapBoxYNSSize) / 3; //16;

const   int16       autoMapTPHeight     = 108;
const   int16       autoMapMDHeight     = 108;
const   int16       autoMapBTHeight     = autoMapBoxYSize
        - autoMapTPHeight
        - autoMapMDHeight;      // 108

const   int16       autoMapTPWidth  = 544;
const   int16       autoMapMDWidth  = 544;
const   int16       autoMapBTWidth  = 544;


static const StaticRect autoMapRect = {
	autoMapBoxX,
	autoMapBoxY,
	autoMapBoxXSize,
	autoMapBoxYSize
};

static const StaticRect autoMapTopPanelRect = {
	autoMapBoxX,
	autoMapBoxY,
	autoMapTPWidth,
	autoMapTPHeight
};

static const StaticRect autoMapMidPanelRect = {
	autoMapBoxX,
	autoMapBoxY + autoMapTPHeight,
	autoMapMDWidth,
	autoMapMDHeight
};

static const StaticRect autoMapBotPanelRect = {
	autoMapBoxX,
	autoMapBoxY + autoMapTPHeight + autoMapMDHeight,
	autoMapBTWidth,
	autoMapBTHeight
};


static const StaticRect *autoMapPanelRects[numAutoMapPanels] = {
	&autoMapTopPanelRect,
	&autoMapMidPanelRect,
	&autoMapBotPanelRect
};


const int16     scrollBtnWidth  = 42;
const int16     scrollBtnHeight = 42;


static const StaticRect closeAutoMapBtnRect = {-1, 276, 42, 41};

static const StaticRect scrollBtnRect = {
	493,
	275,
	scrollBtnWidth,
	scrollBtnHeight
};


StaticWindow autoMapDecorations[numAutoMapPanels] = {
	{*(autoMapPanelRects[0]), NULL, autoMapTopPanelResID},
	{*(autoMapPanelRects[1]), NULL, autoMapMidPanelResID},
	{*(autoMapPanelRects[2]), NULL, autoMapBotPanelResID}
};


/* ===================================================================== *
   Prototypes
 * ===================================================================== */

APPFUNC(cmdAutoMapQuit);
APPFUNC(cmdAutoMapScroll);
APPFUNC(cmdAutoMapAffFunc);

//  Debugging status
//void WriteStatusF( int16 lin, char *msg, ... );



/* ===================================================================== *
   CAutoMap class implementation
 * ===================================================================== */

// ------------------------------------------------------------------------
// ctor

CAutoMap::CAutoMap(const Rect16 box,
                   uint8 *summary,
                   uint16 ident,
                   AppFunc *cmd)
	: ModalWindow(box, ident, cmd) {
	// setup boundry definitions
	sumMapArea  = Rect16(0, 0, sumMapAreaWidth, sumMapAreaHeight);
	summaryData = summary;

	// init the temporary blit surface port
	if (!NewTempPort(tPort, sumMapArea.width, sumMapArea.height)) {
		return;
	}

	trackPos = getCenterActor()->getLocation();

}

// ------------------------------------------------------------------------
// dtor

CAutoMap::~CAutoMap() {
	// dispose of temporary pixelmap
	DisposeTempPort(tPort);
}

// ------------------------------------------------------------------------
// read map data

void CAutoMap::locateRegion(void) {
	hResContext     *areaRes;       // tile resource handle
	uint16          *trRes;
	int16           regionCount;
	struct TileRect {
		int16       uMin, vMin, uMax, vMax;
	} *tr;
	WorldMapData    *wMap = &mapList[currentWorld->mapNum];
	int             i;

	areaRes = auxResFile->newContext(MKTAG('A', 'M', 'A', 'P'), "AreaList");
	assert(areaRes != NULL);

	trRes = (uint16 *)LoadResource(areaRes, MKTAG('Z', 'O', 'N', currentWorld->mapNum), "AreaList");
	assert(trRes != NULL);
	regionCount = *trRes;

	centerCoords = trackPos >> (kTileUVShift + kPlatShift);

	localAreaRegion.min.u = localAreaRegion.min.v = 0;
	localAreaRegion.max.u = localAreaRegion.max.v = wMap->mapSize;

	for (i = 0, tr = (TileRect *)(trRes + 1); i < regionCount; i++, tr++) {
		if (centerCoords.u >= tr->uMin
		        &&  centerCoords.u <= tr->uMax
		        &&  centerCoords.v >= tr->vMin
		        &&  centerCoords.v <= tr->vMax) {
			localAreaRegion.min.u = tr->uMin;
			localAreaRegion.max.u = tr->uMax;
			localAreaRegion.min.v = tr->vMin;
			localAreaRegion.max.v = tr->vMax;
			break;
		}
	}

	free(trRes);

	auxResFile->disposeContext(areaRes);

	baseCoords.u = centerCoords.u - summaryRadius;
	baseCoords.v = centerCoords.v - summaryRadius;
}

// ------------------------------------------------------------------------
// deactivation

void CAutoMap::deactivate(void) {
	selected = 0;
	gPanel::deactivate();
}

// ------------------------------------------------------------------------
// activation

bool CAutoMap::activate(gEventType why) {
	if (why == gEventMouseDown) {           // momentarily depress
		selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	return false;
}

#define SpecialKey(k) ((k>>8)+0x80)

bool CAutoMap::keyStroke(gPanelMessage &msg) {
	gEvent ev;
	switch (msg.key) {
	case 0x1B:
		cmdAutoMapEsc(ev);
		return true;
	case SpecialKey(homeKey):
		cmdAutoMapHome(ev);
		return true;
	case SpecialKey(endKey):
		cmdAutoMapEnd(ev);
		return true;
	case SpecialKey(pageUpKey):
		cmdAutoMapPgUp(ev);
		return true;
	case SpecialKey(pageDownKey):
		cmdAutoMapPgDn(ev);
		return true;
	default:
		return false;
	}
}



gPanel *CAutoMap::keyTest(int16 key) {
	switch (key) {
	case 0x1B:
	case SpecialKey(homeKey):
	case SpecialKey(endKey):
	case SpecialKey(pageUpKey):
	case SpecialKey(pageDownKey):
		return this;
	default:
		return NULL;
	}
}

// ------------------------------------------------------------------------
// mouse movement event handler
void CAutoMap::pointerMove(gPanelMessage &msg) {
	Point16 pos     = msg.pickAbsPos;

	if (Rect16(extent.x, extent.y, extent.width, extent.height).ptInside(pos)) {
		// mouse hit inside autoMap
		TileRegion      viewRegion;
		//  Calculate the actual region we are going to draw as the intersection of
		//  the local dungeon rectangle, and the rectangle of the scrolling map display
		//  in metatile coords.
		viewRegion.min.u = MAX(localAreaRegion.min.u, baseCoords.u);
		viewRegion.max.u = MIN<int16>(localAreaRegion.max.u, baseCoords.u + (int16)summaryDiameter) - 1;
		viewRegion.min.v = MAX(localAreaRegion.min.v, baseCoords.v);
		viewRegion.max.v = MIN<int16>(localAreaRegion.max.v, baseCoords.v + (int16)summaryDiameter) - 1;
		char *mtext = getMapFeaturesText(viewRegion, currentWorld->mapNum, baseCoords, pos) ;
		g_vm->_mouseInfo->setText(mtext);
	} else {
		notify(gEventMouseMove, 0);
	}
}

// ------------------------------------------------------------------------
// mouse click event handler

bool CAutoMap::pointerHit(gPanelMessage &msg) {
	Point16 pos     = msg.pickAbsPos;

	if (Rect16(0, 0, extent.width, extent.height).ptInside(pos)) {
		// mouse hit inside autoMap
	} else {
		// mouse hit outside autoMap area, close it
		gWindow         *win;
		requestInfo     *ri;

		win = getWindow();      // get the window pointer

		if (win)
			ri = (requestInfo *)win->userData;
		else
			ri = NULL;

		if (ri) {
			ri->running = 0;
			ri->result  = id;
		}
	}

	activate(gEventMouseDown);
	return true;
}

// ------------------------------------------------------------------------
// mouse drag event handler

void CAutoMap::pointerDrag(gPanelMessage &) {
	if (selected) {
		notify(gEventMouseDrag, 0);
	}
}

// ------------------------------------------------------------------------
// mouse click release event handler

void CAutoMap::pointerRelease(gPanelMessage &) {
	if (selected) notify(gEventMouseUp, 0);   // notify App of successful hit
	deactivate();
}

// ------------------------------------------------------------------------
// blit

// this drawclipped does not call all inherited drawclip
void CAutoMap::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &clipRect) {
	// return if no change
	if (!extent.overlap(clipRect)) return;

	// clear out the buffer
	memset(tPort.map->data, 0, sumMapArea.width * sumMapArea.height);

	// draw the parts of the panel
	WindowDecoration *dec;
	int16           i;

	pointer.hide();


	//  For each "decorative panel" within the frame of the window
	for (dec = decorations, i = 0; i < numDecorations; i++, dec++) {
		//  If the decorative panel overlaps the area we are
		//  rendering

		if (dec->extent.overlap(clipRect)) {
			Point16 pos(dec->extent.x - extent.x - offset.x,
			            dec->extent.y - extent.y - offset.y);

			drawCompressedImage(tPort, pos, dec->image);
		}
	}

	// draw other stuff
	gWindow::drawClipped(port, offset, clipRect);

	// build the summary image
	createSmallMap();

	//  Blit the pixelmap to the main screen
	port.setMode(drawModeMatte);
	port.bltPixels(*tPort.map,
	               0, 0,
	               extent.x, extent.y,
	               sumMapArea.width, sumMapArea.height);

	// show the cursor again
	pointer.show();
}

// ------------------------------------------------------------------------
// draw

void CAutoMap::draw(void) {          // redraw the window
	// draw the entire panel
	drawClipped(g_vm->_mainPort, Point16(0, 0), extent);
}

// ------------------------------------------------------------------------
// build summary

// create a summary map on the tPort gPixelMap buffer
void CAutoMap::createSmallMap(void) {
	WorldMapData    *wMap = &mapList[currentWorld->mapNum];

	uint16          *mapData = wMap->map->mapData;
	uint16          *mapRow;

	//  Info about region being drawn
	TileRegion      viewRegion;
	TilePoint       topPt,
	                centerPt;
	int32           u, v,
	                xBase, yBase,
	                x0, y0,
	                x, y;

	//  Info about summary data
	gPixelMap       map;
	int16           sumSize = tileSumWidth * tileSumHeight;
	int16           tileSumWidthHalved = tileSumWidth / 2;

	//  Set up pixel map to blit summary data from
	map.size = Point16(tileSumWidth, tileSumHeight);

	// optimizations done based on these numbers
	assert(sumSize  == 64);     // opt:2

	baseCoords.u = clamp(0, baseCoords.u, wMap->mapSize - summaryDiameter);
	baseCoords.v = clamp(0, baseCoords.v, wMap->mapSize - summaryDiameter);

	//  Calculate the actual region we are going to draw as the intersection of
	//  the local dungeon rectangle, and the rectangle of the scrolling map display
	//  in metatile coords.
	viewRegion.min.u = MAX(localAreaRegion.min.u, baseCoords.u);
	viewRegion.max.u = MIN<int16>(localAreaRegion.max.u, baseCoords.u + (int16)summaryDiameter) - 1;
	viewRegion.min.v = MAX(localAreaRegion.min.v, baseCoords.v);
	viewRegion.max.v = MIN<int16>(localAreaRegion.max.v, baseCoords.v + (int16)summaryDiameter) - 1;

	topPt = viewRegion.max - baseCoords;

	xBase = (topPt.u - topPt.v) * tileSumWidthHalved + 261;
	yBase = 255 - (topPt.u + topPt.v) * 2;
	mapRow = &mapData[viewRegion.max.u * wMap->mapSize];

	for (u = viewRegion.max.u, x0 = xBase, y0 = yBase;
	        u >= viewRegion.min.u;
	        u--, x0 -= tileSumWidthHalved, y0 += 2, mapRow -= wMap->mapSize) {
		for (v = viewRegion.max.v, x = x0, y = y0;
		        v >= viewRegion.min.v;
		        v--, x += tileSumWidthHalved, y += 2) {
			uint16  mtile = mapRow[v];

			if (mtile & metaTileVisited)
				if (
#if DEBUG
				    autoMapCheat ||
#endif
				    (mtile & metaTileVisited)) {
					// get the tile data
					map.data = &summaryData[(mtile & ~metaTileVisited) << 6];

					// blit this tile onto the temp surface
					TBlit(tPort.map,
					      &map,
					      x,
					      y);
				}
		}
	}

	drawMapFeatures(viewRegion, currentWorld->mapNum, baseCoords, tPort);


//	if (blink)
	if (centerCoords.u >= viewRegion.min.u
	        &&  centerCoords.u <= viewRegion.max.u
	        &&  centerCoords.v >= viewRegion.min.v
	        &&  centerCoords.v <= viewRegion.max.v) {
		//  Calculate the position of the cross-hairs showing the position of
		//  the center actor.
		centerPt = trackPos - (baseCoords << (kTileUVShift + kPlatShift));

		x = ((centerPt.u - centerPt.v) >> (kTileUVShift + kPlatShift - 2)) + 261 + 4;
		y = 255 + 4 - ((centerPt.u + centerPt.v) >> (kTileUVShift + kPlatShift - 1));

		tPort.setColor(9 + 15);      //  black
		tPort.fillRect(x - 3, y - 1, 7, 3);
		tPort.fillRect(x - 1, y - 3, 3, 7);
		tPort.setColor(9 + 1);       //  white
		tPort.hLine(x - 2, y, 5);
		tPort.vLine(x, y - 2, 5);
	}
}

/* ===================================================================== *
   Public interface
 * ===================================================================== */

int16 openAutoMap() {
	rInfo.result    = -1;
	rInfo.running   = true;

	hResContext     *decRes;
	void            *summaryData;
	void            **closeBtnImage;
	void            **scrollBtnImage;

	uint16          closeButtonResID        = 0;
	uint16          scrollButtonResID       = 2;

	extern int16    currentMapNum;

	updateMapFeatures(currentMapNum);

	// init the resource context handle
	decRes = resFile->newContext(MKTAG('A', 'M', 'A', 'P'), "Automap Resources");

	// debug
	summaryData = LoadResource(decRes, MKTAG('S', 'U', 'M', currentMapNum), "summary data");

	// get the graphics associated with the buttons
	closeBtnImage = loadButtonRes(decRes, closeButtonResID, numBtnImages);
	scrollBtnImage = loadButtonRes(decRes, scrollButtonResID, 2);

	pAutoMap = new CAutoMap(autoMapRect, (uint8 *)summaryData, 0, NULL);

	new gCompButton(*pAutoMap, closeAutoMapBtnRect, closeBtnImage, numBtnImages, 0, cmdAutoMapQuit);

	new gCompButton(*pAutoMap, scrollBtnRect, scrollBtnImage, numBtnImages, 0, cmdAutoMapScroll);

	pAutoMap->setDecorations(autoMapDecorations,
	                         ARRAYSIZE(autoMapDecorations),
	                         decRes, 'M', 'A', 'P');

	// attach the structure to the book
	pAutoMap->userData = &rInfo;

	//  locate where the center actor is, and open the map
	pAutoMap->locateRegion();
	pAutoMap->open();

	EventLoop(rInfo.running, false);

	// delete stuff
	delete pAutoMap;

	unloadImageRes(closeBtnImage, numBtnImages);
	unloadImageRes(scrollBtnImage, 2);
	free(summaryData);
	resFile->disposeContext(decRes);
	decRes = NULL;

	// clean up the backwindow
	mainWindow->invalidate(&autoMapRect);

	return rInfo.result;
}

// ------------------------------------------------------------------------
// event handler

APPFUNC(cmdAutoMapQuit) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && ev.eventType == gEventNewValue && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->userData : NULL;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->id;
		}
	}
}

// ------------------------------------------------------------------------
// event handler

APPFUNC(cmdAutoMapScroll) {
	if (ev.panel && ev.eventType == gEventNewValue && ev.value) {
		static const Rect16             vPosRect(0, 0, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             uPosRect(scrollBtnWidth / 2, 0, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             uNegRect(0, scrollBtnHeight / 2, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             vNegRect(scrollBtnWidth / 2, scrollBtnHeight / 2, scrollBtnWidth / 2, scrollBtnHeight / 2);

		if (uPosRect.ptInside(ev.mouse))      pAutoMap->baseCoords.u += 2;
		else if (uNegRect.ptInside(ev.mouse)) pAutoMap->baseCoords.u -= 2;
		else if (vPosRect.ptInside(ev.mouse)) pAutoMap->baseCoords.v += 2;
		else if (vNegRect.ptInside(ev.mouse)) pAutoMap->baseCoords.v -= 2;

		pAutoMap->draw();
	}
}

APPFUNC(cmdAutoMapAppFunc) {
	if (ev.panel && ev.eventType == gEventMouseMove) {
		// mouse moved

		static const Rect16             vPosRect(0, 0, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             uPosRect(scrollBtnWidth / 2, 0, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             uNegRect(0, scrollBtnHeight / 2, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             vNegRect(scrollBtnWidth / 2, scrollBtnHeight / 2, scrollBtnWidth / 2, scrollBtnHeight / 2);

		if (uPosRect.ptInside(ev.mouse))      pAutoMap->cmdAutoMapHome(ev);     //baseCoords.u += 2;
		else if (uNegRect.ptInside(ev.mouse)) pAutoMap->cmdAutoMapEnd(ev);      //baseCoords.u -= 2;
		else if (vPosRect.ptInside(ev.mouse)) pAutoMap->cmdAutoMapPgUp(ev);     //baseCoords.v += 2;
		else if (vNegRect.ptInside(ev.mouse)) pAutoMap->cmdAutoMapPgDn(ev);     //baseCoords.v -= 2;

		pAutoMap->draw();
	}
}

APPFUNCV(CAutoMap::cmdAutoMapEsc) {
	requestInfo     *ri = (requestInfo *) userData;
	if (ri) {
		ri->running = 0;
		ri->result = 0;
	}
}

APPFUNCV(CAutoMap::cmdAutoMapHome) {
	baseCoords.v += 2;
	draw();
}
APPFUNCV(CAutoMap::cmdAutoMapEnd) {
	baseCoords.u -= 2;
	draw();
}
APPFUNCV(CAutoMap::cmdAutoMapPgUp) {
	baseCoords.u += 2;
	draw();
}
APPFUNCV(CAutoMap::cmdAutoMapPgDn) {
	baseCoords.v -= 2;
	draw();
}

} // end of namespace Saga2
