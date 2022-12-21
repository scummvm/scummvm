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
 */

// Main rendering loop - private header

#ifndef SAGA_RENDER_H
#define SAGA_RENDER_H

#include "saga/sprite.h"
#include "saga/gfx.h"
#include "common/list.h"

namespace Saga {

enum RENDER_FLAGS {
	RF_RENDERPAUSE      = (1 << 0),
	RF_MAP              = (1 << 1),
	RF_DISABLE_ACTORS   = (1 << 2),
	RF_DEMO_SUBST       = (1 << 3)
};

// Extra render flags used for debugging
#ifdef SAGA_DEBUG
enum RENDER_DEBUG_FLAGS {
	RF_SHOW_FPS         = (1 << 4),
	RF_PALETTE_TEST     = (1 << 5),
	RF_TEXT_TEST        = (1 << 6),
	RF_OBJECTMAP_TEST   = (1 << 7),
	RF_ACTOR_PATH_TEST  = (1 << 8)
};
#endif

class Render {
public:
	Render(SagaEngine *vm, OSystem *system);
	~Render();
	bool initialized();
	void drawScene();

	unsigned int getFlags() const {
		return _flags;
	}

	void setFlag(unsigned int flag) {
		_flags |= flag;
	}

	void clearFlag(unsigned int flag) {
		_flags &= ~flag;
	}

	void toggleFlag(unsigned int flag) {
		_flags ^= flag;
	}

	Surface *getBackGroundSurface() {
		return &_backGroundSurface;
	}

	void addDirtyRect(Common::Rect rect);

	void clearDirtyRects() {
		_dirtyRects.clear();
	}

	void setFullRefresh(bool flag) {
		_fullRefresh = flag;
	}

	bool isFullRefresh() {
		return _fullRefresh;
	}

	/**
	 * ECS has only 32-color palette. ITE switches palette
	 * after line 137 on most screens but not on intro. Set
	 * whether to change palette after line 137.
	 */
	void setSplitScreen(bool flag) {
		_splitScreen = flag;
	}

	void maskSplitScreen();
	void drawDirtyRects();
	void scale2xAndMergeOverlay(int x, int y, int w, int h);
	void restoreChangedRects();

private:
#ifdef SAGA_DEBUG
	static void fpsTimerCallback(void *refCon);
	void fpsTimer();
	unsigned int _fps;
	unsigned int _renderedFrameCount;
#endif

	SagaEngine *_vm;
	OSystem *_system;
	bool _initialized;
	Common::List<Common::Rect> _dirtyRects;
	bool _fullRefresh;
	bool _dualSurface;
	bool _splitScreen;

	// Module data
	Surface _backGroundSurface;
	Surface _mergeSurface;

	uint32 _flags;
};

} // End of namespace Saga

#endif
