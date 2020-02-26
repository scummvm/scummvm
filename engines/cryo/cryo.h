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
 */

#ifndef CRYO_CRYO_H
#define CRYO_CRYO_H

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"
#include "graphics/surface.h"
#include "graphics/screen.h"

#include "cryo/eden.h"
#include "cryo/video.h"
#include "cryo/debugger.h"

struct ADGameDescription;

namespace Cryo {

class Console;

// our engine debug channels
enum {
	kCryoDebugExample = 1 << 0,
	kCryoDebugExample2 = 1 << 1
	                     // next new channel must be 1 << 2 (4)
	                     // the current limitation is 32 debug channels (1 << 31 is the last one)
};

class CryoEngine : public Engine {
public:
	CryoEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~CryoEngine() override;

	Common::Error run() override;

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;
	bool isDemo() const;

	// We need random numbers
	Common::RandomSource *_rnd;

	Graphics::Surface _screen;
	EdenGame *_game;
	HnmPlayer *_video;

	View *_screenView;
	volatile int32 _timerTicks;

	bool _showHotspots;

	void pollEvents();

	void hideMouse();
	void showMouse();
	void getMousePosition(int16 *x, int16 *y);
	void setMousePosition(int16 x, int16 y);
	bool isMouseButtonDown();
};

extern CryoEngine *g_ed;

} // End of namespace Cryo

#endif
