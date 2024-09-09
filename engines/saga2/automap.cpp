/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/keyboard.h"

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

AutoMap *pAutoMap = nullptr;

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
	{autoMapTopPanelRect, nullptr, autoMapTopPanelResID},
	{autoMapMidPanelRect, nullptr, autoMapMidPanelResID},
	{autoMapBotPanelRect, nullptr, autoMapBotPanelResID}
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
   AutoMap class implementation
 * ===================================================================== */

// ------------------------------------------------------------------------
// ctor

AutoMap::AutoMap(const Rect16 box,
                   uint8 *summary,
                   uint16 ident,
                   AppFunc *cmd)
	: ModalWindow(box, ident, cmd) {
	_autoMapCheat = false; // FIXME: Allow setting from debug console

	// setup boundary definitions
	_sumMapArea  = Rect16(0, 0, kSumMapAreaWidth, kSumMapAreaHeight);
	_summaryData = summary;

	// init the temporary blit surface port
	if (!NewTempPort(_tPort, _sumMapArea.width, _sumMapArea.height)) {
		return;
	}

	_trackPos = getCenterActor()->getLocation();
}

// ------------------------------------------------------------------------
// dtor

AutoMap::~AutoMap() {
	// dispose of temporary pixelmap
	DisposeTempPort(_tPort);
}

// ------------------------------------------------------------------------
// read map data

void AutoMap::locateRegion() {
	Common::SeekableReadStream *stream;
	hResContext *areaRes;       // tile resource handle
	int16 regionCount;
	WorldMapData *wMap = &mapList[currentWorld->_mapNum];

	areaRes = auxResFile->newContext(MKTAG('A', 'M', 'A', 'P'), "AreaList");
	assert(areaRes != nullptr);

	stream = loadResourceToStream(areaRes, MKTAG('Z', 'O', 'N', currentWorld->_mapNum), "AreaList");
	regionCount = stream->readUint16LE();

	_centerCoords = _trackPos >> (kTileUVShift + kPlatShift);

	_localAreaRegion.min.u = _localAreaRegion.min.v = 0;
	_localAreaRegion.max.u = _localAreaRegion.max.v = wMap->mapSize;

	for (int i = 0; i < regionCount; i++) {
		int uMin, vMin, uMax, vMax;
		uMin = stream->readSint16LE();
		vMin = stream->readSint16LE();
		uMax = stream->readSint16LE();
		vMax = stream->readSint16LE();
		if (_centerCoords.u >= uMin
		        &&  _centerCoords.u <= uMax
		        &&  _centerCoords.v >= vMin
		        &&  _centerCoords.v <= vMax) {
			_localAreaRegion.min.u = uMin;
			_localAreaRegion.max.u = uMax;
			_localAreaRegion.min.v = vMin;
			_localAreaRegion.max.v = vMax;
			break;
		}
	}

	delete stream;

	auxResFile->disposeContext(areaRes);

	_baseCoords.u = _centerCoords.u - kSummaryRadius;
	_baseCoords.v = _centerCoords.v - kSummaryRadius;
}

// ------------------------------------------------------------------------
// deactivation

void AutoMap::deactivate() {
	_selected = 0;
	gPanel::deactivate();
}

// ------------------------------------------------------------------------
// activation

bool AutoMap::activate(gEventType why) {
	if (why == kEventMouseDown) {           // momentarily depress
		_selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	return false;
}

bool AutoMap::keyStroke(gPanelMessage &msg) {
	gEvent ev;
	switch (msg._key) {
	case Common::ASCII_ESCAPE:
		cmdAutoMapEsc(ev);
		return true;
	case Common::KEYCODE_HOME:
		cmdAutoMapHome(ev);
		return true;
	case Common::KEYCODE_END:
		cmdAutoMapEnd(ev);
		return true;
	case Common::KEYCODE_PAGEUP:
		cmdAutoMapPgUp(ev);
		return true;
	case Common::KEYCODE_PAGEDOWN:
		cmdAutoMapPgDn(ev);
		return true;
	default:
		return false;
	}
}



gPanel *AutoMap::keyTest(int16 key) {
	switch (key) {
	case Common::ASCII_ESCAPE:
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_END:
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_PAGEDOWN:
		return this;
	default:
		return nullptr;
	}
}

// ------------------------------------------------------------------------
// mouse movement event handler
void AutoMap::pointerMove(gPanelMessage &msg) {
	Point16 pos     = msg._pickAbsPos;

	if (Rect16(_extent.x, _extent.y, _extent.width, _extent.height).ptInside(pos)) {
		// mouse hit inside autoMap
		TileRegion      viewRegion;
		//  Calculate the actual region we are going to draw as the intersection of
		//  the local dungeon rectangle, and the rectangle of the scrolling map display
		//  in metatile coords.
		viewRegion.min.u = MAX(_localAreaRegion.min.u, _baseCoords.u);
		viewRegion.max.u = MIN<int16>(_localAreaRegion.max.u, _baseCoords.u + (int16)kSummaryDiameter) - 1;
		viewRegion.min.v = MAX(_localAreaRegion.min.v, _baseCoords.v);
		viewRegion.max.v = MIN<int16>(_localAreaRegion.max.v, _baseCoords.v + (int16)kSummaryDiameter) - 1;
		char *mtext = getMapFeaturesText(viewRegion, currentWorld->_mapNum, _baseCoords, pos) ;
		g_vm->_mouseInfo->setText(mtext);
	} else {
		notify(kEventMouseMove, 0);
	}
}

// ------------------------------------------------------------------------
// mouse click event handler

bool AutoMap::pointerHit(gPanelMessage &msg) {
	Point16 pos     = msg._pickAbsPos;

	if (Rect16(0, 0, _extent.width, _extent.height).ptInside(pos)) {
		// mouse hit inside autoMap

		if (g_vm->_teleportOnMap) {
			TilePoint centerPt = TilePoint(((259 - pos.y) << (kTileUVShift + kPlatShift - 2)) + ((pos.x - 265) << (kTileUVShift + kPlatShift - 3)),
			                               ((259 - pos.y) << (kTileUVShift + kPlatShift - 2)) - ((pos.x - 265) << (kTileUVShift + kPlatShift - 3)),
			                               0);

			TilePoint pt = centerPt + (_baseCoords << (kTileUVShift + kPlatShift));

			Actor *a = getCenterActor();

			int du = pt.u - a->getLocation().u;
			int dv = pt.v - a->getLocation().v;

			for (ObjectID pid = ActorBaseID; pid < ActorBaseID + kPlayerActors; ++pid) {
				Actor *p = (Actor *)GameObject::objectAddress(pid);
				TilePoint curLoc = p->getLocation();
				p->setLocation(TilePoint(curLoc.u + du, curLoc.v + dv, 8));
			}
		}
	} else {
		// mouse hit outside autoMap area, close it
		gWindow         *win;
		requestInfo     *ri;

		win = getWindow();      // get the window pointer

		if (win)
			ri = (requestInfo *)win->_userData;
		else
			ri = nullptr;

		if (ri) {
			ri->running = 0;
			ri->result  = _id;
		}
	}

	activate(kEventMouseDown);
	return true;
}

// ------------------------------------------------------------------------
// mouse drag event handler

void AutoMap::pointerDrag(gPanelMessage &) {
	if (_selected) {
		notify(kEventMouseDrag, 0);
	}
}

// ------------------------------------------------------------------------
// mouse click release event handler

void AutoMap::pointerRelease(gPanelMessage &) {
	if (_selected) notify(kEventMouseUp, 0);   // notify App of successful hit
	deactivate();
}

// ------------------------------------------------------------------------
// blit

// this drawclipped does not call all inherited drawclip
void AutoMap::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &clipRect) {
	// return if no change
	if (!_extent.overlap(clipRect)) return;

	// clear out the buffer
	memset(_tPort._map->_data, 0, _sumMapArea.width * _sumMapArea.height);

	// draw the parts of the panel
	WindowDecoration *dec;
	int16           i;

	g_vm->_pointer->hide();


	//  For each "decorative panel" within the frame of the window
	for (dec = _decorations, i = 0; i < _numDecorations; i++, dec++) {
		//  If the decorative panel overlaps the area we are
		//  rendering

		if (dec->extent.overlap(clipRect)) {
			Point16 pos(dec->extent.x - _extent.x - offset.x,
			            dec->extent.y - _extent.y - offset.y);

			drawCompressedImage(_tPort, pos, dec->image);
		}
	}

	// draw other stuff
	gWindow::drawClipped(port, offset, clipRect);

	// build the summary image
	createSmallMap();

	//  Blit the pixelmap to the main screen
	port.setMode(kDrawModeMatte);
	port.bltPixels(*_tPort._map,
	               0, 0,
	               _extent.x, _extent.y,
	               _sumMapArea.width, _sumMapArea.height);

	// show the cursor again
	g_vm->_pointer->show();
}

// ------------------------------------------------------------------------
// draw

void AutoMap::draw() {          // redraw the window
	// draw the entire panel
	drawClipped(g_vm->_mainPort, Point16(0, 0), _extent);
}

// ------------------------------------------------------------------------
// build summary

// create a summary map on the tPort gPixelMap buffer
void AutoMap::createSmallMap() {
	WorldMapData    *wMap = &mapList[currentWorld->_mapNum];

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
	int16           sumSize = kTileSumWidth * kTileSumHeight;
	int16           tileSumWidthHalved = kTileSumWidth / 2;

	//  Set up pixel map to blit summary data from
	map._size = Point16(kTileSumWidth, kTileSumHeight);

	// optimizations done based on these numbers
	assert(sumSize  == 64);     // opt:2

	_baseCoords.u = clamp(0, _baseCoords.u, wMap->mapSize - kSummaryDiameter);
	_baseCoords.v = clamp(0, _baseCoords.v, wMap->mapSize - kSummaryDiameter);

	//  Calculate the actual region we are going to draw as the intersection of
	//  the local dungeon rectangle, and the rectangle of the scrolling map display
	//  in metatile coords.
	viewRegion.min.u = MAX(_localAreaRegion.min.u, _baseCoords.u);
	viewRegion.max.u = MIN<int16>(_localAreaRegion.max.u, _baseCoords.u + (int16)kSummaryDiameter) - 1;
	viewRegion.min.v = MAX(_localAreaRegion.min.v, _baseCoords.v);
	viewRegion.max.v = MIN<int16>(_localAreaRegion.max.v, _baseCoords.v + (int16)kSummaryDiameter) - 1;

	topPt = viewRegion.max - _baseCoords;

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

			if (mtile & kMetaTileVisited)
				if (_autoMapCheat || (mtile & kMetaTileVisited)) {
					// get the tile data
					map._data = &_summaryData[(mtile & ~kMetaTileVisited) << 6];

					// blit this tile onto the temp surface
					TBlit(_tPort._map,
					      &map,
					      x,
					      y);
				}
		}
	}

	drawMapFeatures(viewRegion, currentWorld->_mapNum, _baseCoords, _tPort);


//	if (blink)
	if (_centerCoords.u >= viewRegion.min.u
	        &&  _centerCoords.u <= viewRegion.max.u
	        &&  _centerCoords.v >= viewRegion.min.v
	        &&  _centerCoords.v <= viewRegion.max.v) {
		//  Calculate the position of the cross-hairs showing the position of
		//  the center actor.
		centerPt = _trackPos - (_baseCoords << (kTileUVShift + kPlatShift));

		x = ((centerPt.u - centerPt.v) >> (kTileUVShift + kPlatShift - 2)) + 261 + 4;
		y = 255 + 4 - ((centerPt.u + centerPt.v) >> (kTileUVShift + kPlatShift - 1));

		_tPort.setColor(9 + 15);      //  black
		_tPort.fillRect(x - 3, y - 1, 7, 3);
		_tPort.fillRect(x - 1, y - 3, 3, 7);
		_tPort.setColor(9 + 1);       //  white
		_tPort.hLine(x - 2, y, 5);
		_tPort.vLine(x, y - 2, 5);
	}
}

/* ===================================================================== *
   Public interface
 * ===================================================================== */

int16 openAutoMap() {
	rInfo.result    = -1;
	rInfo.running   = true;

	hResContext     *decRes;
	void            *_summaryData;
	void            **closeBtnImage;
	void            **scrollBtnImage;

	uint16          closeButtonResID        = 0;
	uint16          scrollButtonResID       = 2;

	updateMapFeatures(g_vm->_currentMapNum);

	// init the resource context handle
	decRes = resFile->newContext(MKTAG('A', 'M', 'A', 'P'), "Automap Resources");

	// debug
	_summaryData = LoadResource(decRes, MKTAG('S', 'U', 'M', g_vm->_currentMapNum), "summary data");

	// get the graphics associated with the buttons
	closeBtnImage = loadButtonRes(decRes, closeButtonResID, kNumBtnImages);
	scrollBtnImage = loadButtonRes(decRes, scrollButtonResID, 2);

	pAutoMap = new AutoMap(autoMapRect, (uint8 *)_summaryData, 0, nullptr);

	new GfxCompButton(*pAutoMap, closeAutoMapBtnRect, closeBtnImage, kNumBtnImages, 0, cmdAutoMapQuit);

	new GfxCompButton(*pAutoMap, scrollBtnRect, scrollBtnImage, kNumBtnImages, 0, cmdAutoMapScroll);

	pAutoMap->setDecorations(autoMapDecorations,
	                         ARRAYSIZE(autoMapDecorations),
	                         decRes, 'M', 'A', 'P');

	// attach the structure to the book
	pAutoMap->_userData = &rInfo;

	//  locate where the center actor is, and open the map
	pAutoMap->locateRegion();
	pAutoMap->open();

	EventLoop(rInfo.running, false);

	// delete stuff
	delete pAutoMap;

	unloadImageRes(closeBtnImage, kNumBtnImages);
	unloadImageRes(scrollBtnImage, 2);
	free(_summaryData);
	resFile->disposeContext(decRes);
	decRes = nullptr;

	// clean up the backwindow
	mainWindow->invalidate(&autoMapRect);

	return rInfo.result;
}

// ------------------------------------------------------------------------
// event handler

APPFUNC(cmdAutoMapQuit) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && ev.eventType == kEventNewValue && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->_userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->_id;
		}
	}
}

// ------------------------------------------------------------------------
// event handler

APPFUNC(cmdAutoMapScroll) {
	if (ev.panel && ev.eventType == kEventNewValue && ev.value) {
		static const Rect16             vPosRect(0, 0, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             uPosRect(scrollBtnWidth / 2, 0, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             uNegRect(0, scrollBtnHeight / 2, scrollBtnWidth / 2, scrollBtnHeight / 2);
		static const Rect16             vNegRect(scrollBtnWidth / 2, scrollBtnHeight / 2, scrollBtnWidth / 2, scrollBtnHeight / 2);

		if (uPosRect.ptInside(ev.mouse))      pAutoMap->_baseCoords.u += 2;
		else if (uNegRect.ptInside(ev.mouse)) pAutoMap->_baseCoords.u -= 2;
		else if (vPosRect.ptInside(ev.mouse)) pAutoMap->_baseCoords.v += 2;
		else if (vNegRect.ptInside(ev.mouse)) pAutoMap->_baseCoords.v -= 2;

		pAutoMap->draw();
	}
}

APPFUNC(cmdAutoMapAppFunc) {
	if (ev.panel && ev.eventType == kEventMouseMove) {
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

APPFUNCV(AutoMap::cmdAutoMapEsc) {
	requestInfo     *ri = (requestInfo *)_userData;
	if (ri) {
		ri->running = 0;
		ri->result = 0;
	}
}

APPFUNCV(AutoMap::cmdAutoMapHome) {
	_baseCoords.v += 2;
	draw();
}
APPFUNCV(AutoMap::cmdAutoMapEnd) {
	_baseCoords.u -= 2;
	draw();
}
APPFUNCV(AutoMap::cmdAutoMapPgUp) {
	_baseCoords.u += 2;
	draw();
}
APPFUNCV(AutoMap::cmdAutoMapPgDn) {
	_baseCoords.v -= 2;
	draw();
}

} // end of namespace Saga2
