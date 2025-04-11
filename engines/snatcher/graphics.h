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

#include "common/func.h"
#include "common/platform.h"
#include "snatcher/info.h"
#include "snatcher/resource.h"

class OSystem;

namespace Graphics {
	struct PixelFormat;
}

namespace Snatcher {

class Palette;
class ScrollManager;
class Renderer;
class SceneModule;

class GraphicsEngine {
public:
	GraphicsEngine(const Graphics::PixelFormat *pxf, OSystem *system, Common::Platform platform, const VMInfo &vmstate);
	~GraphicsEngine();

	void runScript(ResourcePointer res, int func);
	void enqueuePaletteEvent(ResourcePointer res);
	bool enqueueCopyCommands(ResourcePointer res);

	enum ScrollMode : uint8 {
		kHorzA		= 0,
		kVertA		= 1,
		kHorzB		= 2,
		kVertB		= 3,
		kSingleStep = 4
	};

	void setScrollStep(uint8 mode, int16 step);
	void scrollCommand(uint8 cmd);

	void updateAnimations();
	void nextFrame();

	enum ResetType : uint16 {
		kResetPalEvents			=	1 << 0,
		kResetSetDefaults		=	1 << 1,
		kResetSetDefaultsExt	=	1 << 2,
		kResetCopyCmds			=	1 << 3,
		kResetSprites			=	1 << 4,
		kResetScrollState		=	1 << 5
	};

	void reset(int mode);

	void setVar(uint8 var, uint8 val);

	enum AnimFlags : int {
		kAnimPause				=	1 << 0,
		kAnimHide				=	1 << 1,
		kAnimAudioSync			=	1 << 2
	};

	void setAnimControlFlags(uint8 animObjId, int flags);
	void clearAnimControlFlags(uint8 animObjId, int flags);
	void setAnimFrame(uint8 animObjId, uint16 frameNo);
	uint16 getAnimCurFrame(uint8 animObjId) const;
	bool isAnimEnabled(uint8 animObjId) const;

	uint16 screenWidth() const;
	uint16 screenHeight() const;

	bool busy(int type) const;

public:
	struct GfxState {
		GfxState(const VMInfo &vminfo) : vm(vminfo) {
			memset(vars, 0, sizeof(vars));
			frameCounter = 0;
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
			return frameCounter;
		}

		void nextFrame() {
			++frameCounter;
		}

		uint32 getAudioSync() const {
			return vm.audioSync;
		}

		uint16 getDropFrames() const {
			return vm.dropFrames;
		}
	
	private:
		uint8 vars[11];
		uint16 frameCounter;
		const VMInfo &vm;
	};

private:
	void restoreDefaultsExt();
	void restoreDefaults();

	GfxState _state;
	const uint8 _bpp;
	uint8 _dataMode;
	uint8 _flags;

	byte *_screen;
	Renderer *_renderer;
	Palette *_palette;
	ScrollManager *_scroll;
	OSystem *_system;
};

} // End of namespace Snatcher

#endif // SNATCHER_GRAPHICS_H
