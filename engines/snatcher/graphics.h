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

#ifndef SNATCHER_GRAPHICS_H
#define SNATCHER_GRAPHICS_H

#include "common/platform.h"
#include "snatcher/resource.h"

class OSystem;

namespace Snatcher {

class Palette;
class Renderer;
class SceneModule;

class GraphicsEngine {
public:
	GraphicsEngine(OSystem *system, Common::Platform platform);
	~GraphicsEngine();

	void runScript(ResourcePointer res, int func);
	void enqueuePaletteEvent(ResourcePointer res);
	void enqueueCopyCommands(ResourcePointer res);
	void doCommand(uint8 cmd);

	void updateAnimations();
	void nextFrame();

	enum ResetType : uint16 {
		kClearPalEvents			=	1 << 0,
		kRestoreDefaults		=	1 << 1,
		kRestoreDefaultsExt		=	1 << 2,
		kResetGfxStructs6		=	1 << 3,
		kClearSprites			=	1 << 4
	};

	void reset(int mode);

	void setVar(uint8 var, uint8 val);

	uint16 screenWidth() const;
	uint16 screenHeight() const;

	bool busy() const;

public:
	struct State {
		State() {
			memset(vars, 0, sizeof(vars));
			_frameCounter = 0;
		}

		uint8 getVar(uint8 var) const {
			assert(var < ARRAYSIZE(vars));
			return vars[var];
		}

		void setVar(uint8 var, uint8 val) {
			assert(var < ARRAYSIZE(vars));
			vars[var] = val;
		}

		bool testFlag(uint8 var, uint8 bit) const {
			assert(var < ARRAYSIZE(vars));
			return vars[var] & (1 << bit);
		}

		void setFlag(uint8 var, uint8 bit) {
			assert(var < ARRAYSIZE(vars));
			vars[var] |= (1 << bit);
		}

		void clearFlag(uint8 var, uint8 bit) {
			assert(var < ARRAYSIZE(vars));
			vars[var] &= ~(1 << bit);
		}

		uint16 frameCount() const {
			return _frameCounter;
		}

		void nextFrame() {
			++_frameCounter;
		}
	
	private:
		uint8 vars[10];
		uint16 _frameCounter;
	};

private:
	void restoreDefaultsExt();
	void restoreDefaults();

	State _state;
	uint8 _dataMode;
	uint8 _flags;

	byte *_screen;
	Renderer *_renderer;
	Palette *_palette;
	OSystem *_system;
};

} // End of namespace Snatcher

#endif // SNATCHER_GRAPHICS_H
