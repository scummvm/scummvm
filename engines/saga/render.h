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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Main rendering loop - private header

#ifndef SAGA_RENDER_H
#define SAGA_RENDER_H

#include "saga/sprite.h"

namespace Saga {

enum RENDER_FLAGS {
#ifdef SAGA_DEBUG
	RF_SHOW_FPS = (1 << 0),
	RF_PALETTE_TEST = (1 << 1),
	RF_TEXT_TEST = (1 << 2),
	RF_OBJECTMAP_TEST = (1 << 3),
#endif
	RF_RENDERPAUSE = (1 << 4),
	RF_GAMEPAUSE = (1 << 5),
#ifdef SAGA_DEBUG
	RF_ACTOR_PATH_TEST = (1 << 6),
#endif
	RF_MAP = (1 << 7),
	RF_DISABLE_ACTORS = (1 << 8),
	RF_DEMO_SUBST = (1 << 9)
};

class Render {
public:
	Render(SagaEngine *vm, OSystem *system);
	~Render(void);
	bool initialized();
	void drawScene(void);

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

private:
	static void fpsTimerCallback(void *refCon);
	void fpsTimer(void);

	SagaEngine *_vm;
	OSystem *_system;
	bool _initialized;

	// Module data
	Surface _backGroundSurface;

	unsigned int _fps;
	unsigned int _renderedFrameCount;
	uint32 _flags;
};

} // End of namespace Saga

#endif
