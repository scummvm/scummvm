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

#ifndef SAGA2_GPOINTER_H
#define SAGA2_GPOINTER_H

#include "saga2/vdraw.h"

namespace Saga2 {

class gMousePointer {
	gPixelMap           *pointerImage,      // pointer to current mouse image
	                    saveMap;            // memory to backsave to
	gPort               savePort;           // port to save to
	gDisplayPort        *videoPort;         // port to render to
	Rect16              saveExtent;         // extent of backsave
	int16               hideCount;          // mouse hiding nesting level
	Point16             currentPosition,    // where real coords are
	                    offsetPosition;     // center of mouse image
	bool                shown;              // mouse currently shown

	void draw();
	void restore();
public:
	gMousePointer(gDisplayPort &);       // constructor
	~gMousePointer();                       // destructor

	bool init(Point16 pointerLimits);
	bool init(uint16 xLimit, uint16 yLimit) {
		return init(Point16(xLimit, yLimit));
	}
	void show();                         // show the pointer
	void hide();                         // hide the pointer
	void show(gPort &port, Rect16 r);        // show the pointer
	void hide(gPort &port, Rect16 r);        // hide the pointer
	int manditoryShow();
	void move(Point16 pos);                  // move the pointer
	void setImage(gPixelMap &img, int x, int y);     // set the pointer imagery
	bool isShown() {
		return shown;
	}
	int16 hideDepth() {
		return hideCount;
	}
	gPixelMap *getImage(Point16 &offset) {
		offset = offsetPosition;
		return pointerImage;
	}
	gPixelMap *getImageCurPos(Point16 &curPos) {
		curPos = currentPosition;
		return pointerImage;
	}
	gPixelMap *getSaveMap(Rect16 &save) {
		save = saveExtent;
		return &saveMap;
	}
};

} // end of namespace Saga2

#endif  // SAGA2_POINTER_H
