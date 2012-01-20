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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MORTEVIELLE_H
#define MORTEVIELLE_H

#include "common/events.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/stack.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "common/error.h"
#include "graphics/surface.h"
#include "mortevielle/graphics.h"

namespace Mortevielle {

enum {
	kMortevielleCore = 1 << 0,
	kMortevielleGraphics = 1 << 1
};

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400
#define SCREEN_ORIG_HEIGHT 200
#define MORT_DAT_REQUIRED_VERSION 1
#define MORT_DAT "mort.dat"
#define GAME_FRAME_DELAY (1000 / 50)

class MortevielleEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::Stack<int> _keypresses;
	uint32 _lastGameFrame;
	int _mouseButtons;
	Common::Point _mousePos;

	Common::ErrorCode initialise();
	Common::ErrorCode loadMortDat();
	void loadFont(Common::File &f);
	bool handleEvents();
	void addKeypress(Common::Event &evt);
	void initMouse();
public:
	ScreenSurface _screenSurface;
	PaletteManager _paletteManager;
public:
	MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc);
	~MortevielleEngine();
	virtual bool hasFeature(EngineFeature f) const;
	virtual Common::Error run();
	uint32 getGameFlags() const;

	bool keyPressed();
	int getChar();
	Common::Point getMousePos() const { return _mousePos; }
	void setMousePos(const Common::Point &pt);
	int getMouseButtons() const { return _mouseButtons; }
};

extern MortevielleEngine *g_vm;

#define CHECK_QUIT if (g_vm->shouldQuit()) { return; }

} // End of namespace Mortevielle

#endif
