/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_SCENE_BASE_H
#define BURIED_SCENE_BASE_H

#include "common/keyboard.h"
#include "common/rect.h"

#include "buried/navdata.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

// Generic result codes
enum {
	SC_FALSE = 0,          // Everything did not go well, error out
	SC_TRUE = 1,           // Everything is kosher, continue as normal
	SC_END_PROCESSING = 4  // Stop processing here
};

// Post-exit specific result codes
enum {
	SC_DEATH = 3           // We died, so do nothing further
};

// Paint-specific return values
enum {
	SC_REPAINT = 0,
	SC_DO_NOT_REPAINT = 1
};

// Movie status codes
enum {
	MOVIE_START = 0,
	MOVIE_STOPPED = 1,
	MOVIE_PLAYING = 2,
	MOVIE_ABORTED_BY_USER = 3,
	MOVIE_LOOPING_RESTART = 4
};

// Inventory item related return codes
enum {
	SIC_REJECT = 0,
	SIC_ACCEPT = 1
};

class BuriedEngine;
class Window;
class VideoWindow;

class SceneBase { // Wow, it's not a window!
public:
	SceneBase(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	virtual ~SceneBase() {}

	virtual void preDestructor() {}

	virtual int preEnterRoom(Window *viewWindow, const Location &priorLocation) { return SC_TRUE; }
	virtual int postEnterRoom(Window *viewWindow, const Location &priorLocation) { return SC_TRUE; }

	virtual int preExitRoom(Window *viewWindow, const Location &priorLocation) { return SC_TRUE; }
	virtual int postExitRoom(Window *viewWindow, const Location &priorLocation) { return SC_TRUE; }

	virtual int mouseDown(Window *viewWindow, const Common::Point &pointLocation) { return SC_TRUE; }
	virtual int mouseUp(Window *viewWindow, const Common::Point &pointLocation) { return SC_TRUE; }
	virtual int mouseMove(Window *viewWindow, const Common::Point &pointLocation) { return SC_TRUE; }

	virtual int onCharacter(Window *viewWindow, const Common::KeyState &character) { return SC_TRUE; }

	virtual int paint(Window *viewWindow, Graphics::Surface *preBuffer);
	virtual int gdiPaint(Window *viewWindow) { return SC_REPAINT; }

	virtual int movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status) { return SC_TRUE; }
	virtual int timerCallback(Window *viewWindow);

	virtual int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) { return 0; }
	virtual int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) { return SIC_REJECT; }

	virtual int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

	virtual int locateAttempted(Window *viewWindow, const Common::Point &pointLocation) { return 0; }

	LocationStaticData _staticData;
	int32 _frameCycleCount;

protected:
	BuriedEngine *_vm;
};

} // End of namespace Buried

#endif
