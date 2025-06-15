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

namespace Common {
class SeekableWriteStream;
} // namespace Common

namespace Graphics {
struct PixelFormat;
} // namespace Graphics

namespace Snatcher {

class Palette;
class TransitionManager;
class Animator;
class SceneModule;
class SoundEngine;
class TextRenderer;

class GraphicsEngine {
public:
	GraphicsEngine(const Graphics::PixelFormat *pxf, OSystem *system, Common::Platform platform, const VMInfo &vmstate, SoundEngine *snd, bool enableAspectRatioCorrection);
	~GraphicsEngine();

	void runScript(ResourcePointer res, int func);
	void enqueuePaletteEvent(ResourcePointer res);
	bool enqueueDrawCommands(ResourcePointer res);
	void initAnimations(ResourcePointer res, uint16 len, bool dontUpdate);

	enum ScrollMode : uint8 {
		kHorzA		= 0,
		kVertA		= 1,
		kHorzB		= 2,
		kVertB		= 3,
		kSingleStep = 4
	};

	void setScrollStep(uint8 mode, int16 step);
	void transitionCommand(uint8 cmd);
	bool transitionStateBusy() const;

	void setTextFont(const uint8 *font, uint32 fontSize, const uint8 *charWidthTable, uint32 charWidthTableSize);
	void printText(const uint8 *text);
	void setTextPrintDelay(uint16 delay);
	bool isTextInQueue() const;
	void resetTextFields();
	void clearJordanInputField();
	uint8 getVerbAreaType() const;
	bool isVerbsTabActive() const;

	void updateAnimations();
	void updateText();
	void nextFrame();

	enum ResetType : uint16 {
		kResetPalEvents			=	1 << 0,
		kResetSetDefaults		=	1 << 1,
		kResetSetDefaultsExt	=	1 << 2,
		kResetCopyCmds			=	1 << 3,
		kResetAnimations		=	1 << 4,
		kResetScrollState		=	1 << 5
	};

	void reset(int mode);

	void setVar(uint8 var, uint8 val);
	uint8 getVar(uint8 var) const;

	enum AnimParameters : int {
		kAnimParaEnable = 0,
		kAnimParaBlinkRate,
		kAnimParaDrawFlags,
		kAnimParaPosX,
		kAnimParaPosY,
		kAnimParaRelSpeedX,
		kAnimParaRelSpeedY,
		kAnimParaPalette,
		kAnimParaF17,
		kAnimParaF18,
		kAnimParaF1c,
		kAnimParaTimeStamp,
		kAnimParaF24,
		kAnimParaControlFlags,
		kAnimParaAllowFrameDrop,
		kAnimParaFrameSeqCounter,
		kAnimParaFrame,
		kAnimParaFrameDelay,
		kAnimParaF2c,
		kAnimParaF2d,
		kAnimParaBlink,
		kAnimParaBlinkCounter,
		kAnimParaBlinkDuration,
		kAnimParaAbsSpeedX,
		kAnimParaAbsSpeedY,
		kAnimParaScriptComFlags,
		kAnimFreezeFlag
	};

	enum AnimFlags : int {
		kAnimNone				=	0,
		kAnimPause				=	1 << 0,
		kAnimHide				=	1 << 1,
		kAnimAudioSync			=	1 << 2
	};

	void setAnimParameter(uint8 animObjId, int param, int32 value);
	void setAnimGroupParameter(uint8 animObjId, int groupOp, int32 value = 0);
	int32 getAnimParameter(uint8 animObjId, int param) const;
	uint8 getAnimScriptByte(uint8 animObjId, uint16 offset) const;
	void addAnimParameterFlags(uint8 animObjId, int param, int flags);
	void clearAnimParameterFlags(uint8 animObjId, int param, int flags);
	bool testAnimParameterFlags(uint8 animObjId, int param, int flags) const;
	void animCopySpec(uint8 srcAnimObjId, uint8 dstAnimObjId);

	uint16 screenWidth() const;
	uint16 screenHeight() const;
	uint16 realScreenWidth() const;
	uint16 realScreenHeight() const;

	bool busy(int type) const;
	uint16 frameCount() const;

	void loadState(Common::SeekableReadStream *in);
	void saveState(Common::SeekableWriteStream *out);
	void postLoadProcess();

	void createMouseCursor(bool show);

	int displayBootLogoFrame(int frameNo);

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

		uint16 frameCount2() const {
			return vm.frameCounter;
		}

	private:
		uint8 vars[12];
		uint16 frameCounter;
		const VMInfo &vm;
	};

private:
	void restoreDefaultsExt();
	void restoreDefaults();

	GfxState _state;
	const uint8 _bpp;
	uint8 _dataMode;
	uint8 _verbAreaType;
	uint8 _flags;

	uint8 *_animSaveLoadData;

	uint8 *_screen;
	Animator *_animator;
	Palette *_palette;
	TextRenderer *_text;
	TransitionManager *_trs;
	OSystem *_system;
};

} // End of namespace Snatcher

#endif // SNATCHER_GRAPHICS_H
