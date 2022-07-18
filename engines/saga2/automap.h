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

#ifndef SAGA2_AUTOMAP_H
#define SAGA2_AUTOMAP_H

#include "saga2/intrface.h"

namespace Saga2 {

class AutoMap : public ModalWindow {
private:

	enum summaryMapEnum {
		kTileSumWidth        = 8,
		kTileSumHeight       = 8,
		kSumMapAreaWidth     = 544,  // the sumMap number could be more efficient
		kSumMapAreaHeight    = 324,

		kSummaryDiameter     = 62,
		kSummaryRadius       = kSummaryDiameter / 2
	};

public:
	TilePoint   _trackPos,
	            _centerCoords,
	            _baseCoords;
	TileRegion  _localAreaRegion;
private:

	// used as a temporary blit surface
	gPort   _tPort;

	// tile summary data
	uint8   *_summaryData;
	Rect16  _sumMapArea;

	bool _autoMapCheat;

public:
	AutoMap(const Rect16 box,
	         uint8 *summary,
	         uint16 ident,
	         AppFunc *cmd);
	~AutoMap();

	void drawClipped(gPort &port,
	                 const Point16 &offset,
	                 const Rect16  &clipRect);
	void draw();             // redraw the window

	void createSmallMap();
	void locateRegion();
	APPFUNCV(cmdAutoMapEsc);
	APPFUNCV(cmdAutoMapHome);
	APPFUNCV(cmdAutoMapEnd);
	APPFUNCV(cmdAutoMapPgUp);
	APPFUNCV(cmdAutoMapPgDn);

	gPanel *keyTest(int16 key);
private:
	bool activate(gEventType why);       // activate the control
	void deactivate();

	void pointerMove(gPanelMessage &msg);
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);
	bool keyStroke(gPanelMessage &msg);

};

//
// object access routines
//

int16 openAutoMap();

} // end of namespace Saga2

#endif // SAGA2_AUTOMAP_H
