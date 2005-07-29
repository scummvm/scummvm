/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Main rendering loop - private header

#ifndef SAGA_RENDER_H_
#define SAGA_RENDER_H_

#include "saga/sprite.h"

namespace Saga {

enum RENDER_FLAGS {
	RF_SHOW_FPS = (1 << 0),
	RF_PALETTE_TEST = (1 << 1),
	RF_TEXT_TEST = (1 << 2),
	RF_OBJECTMAP_TEST = (1 << 3),
	RF_RENDERPAUSE = (1 << 4),
	RF_GAMEPAUSE = (1 << 5),
	RF_PLACARD = (1 << 6),
	RF_ACTOR_PATH_TEST = (1 << 7),
	RF_MAP = (1 << 8),
	RF_DISABLE_ACTORS = (1 << 9),
	RF_DEMO_SUBST = (1 << 10),
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
	unsigned int _frameCount;
	uint32 _flags;
};

} // End of namespace Saga

#endif
