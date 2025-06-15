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

#ifndef SNATCHER_ANIMATOR_H
#define SNATCHER_ANIMATOR_H

#include "common/platform.h"
#include "snatcher/graphics.h"
#include "snatcher/resource.h"

namespace Snatcher {

class SoundEngine;
class Animator {
public:
	virtual ~Animator() {}

	virtual bool enqueueDrawCommands(ResourcePointer &res) = 0;
	virtual void clearDrawCommands() = 0;
	virtual void initData(ResourcePointer &res, uint8 mode) = 0;
	virtual void initAnimations(ResourcePointer &res, uint16 len, bool dontUpdate) = 0;
	virtual void linkAnimations(ResourcePointer &res, uint16 len) = 0;
	virtual void clearAnimations(int mode = 0) = 0;
	virtual void setPlaneMode(uint16 mode) = 0;

	virtual void resetTextFields() = 0;
	virtual void clearJordanInputField() = 0;
	virtual uint8 *getTextRenderBuffer() const = 0;
	virtual void renderTextBuffer(uint16 firstLine, uint8 numLines) = 0;

	virtual void updateScreen(uint8 *screen) = 0;
	virtual void updateAnimations() = 0;

	virtual void setAnimParameter(uint8 animObjId, int param, int32 value) = 0;
	virtual void setAnimGroupParameter(uint8 animObjId, int groupOp, int32 value = 0) = 0;
	virtual int32 getAnimParameter(uint8 animObjId, int param) const = 0;
	virtual uint8 getAnimScriptByte(uint8 animObjId, uint16 offset) const = 0;
	virtual void animCopySpec(uint8 srcAnimObjId, uint8 dstAnimObjId) = 0;

	virtual uint16 screenWidth() const = 0;
	virtual uint16 screenHeight() const = 0;
	virtual uint16 realScreenWidth() const = 0;
	virtual uint16 realScreenHeight() const = 0;

	// The original game does not have mouse control on any platform. But
	// the Sega CD version supports a lightgun. So we do have some fundamentals
	// on which we can set this this up...
	virtual void createMouseCursor() = 0;

	// This is special code from the CD-ROM boot sectors that does not
	// share anything with the rest of the game. So it makes sense to
	// keep it separate.
	virtual int drawBootLogoFrame(uint8 *screen, int frameNo) = 0;

protected:
	Animator(GraphicsEngine::GfxState &state) : _gfxState(state) {}
	GraphicsEngine::GfxState &_gfxState;

private:
	static Animator *createSCDAnimator(const Graphics::PixelFormat *pxf, GraphicsEngine::GfxState &state, Palette *pal, TransitionManager *scr, SoundEngine *snd, bool enableAspectRatioCorrection);

public:
	static Animator *create(const Graphics::PixelFormat *pxf, Common::Platform platform, GraphicsEngine::GfxState &state, Palette *pal, TransitionManager *scr, SoundEngine *snd, bool enableAspectRatioCorrection) {
		switch (platform) {
		case Common::kPlatformSegaCD:
			return createSCDAnimator(pxf, state, pal, scr, snd, enableAspectRatioCorrection);
		default:
			break;
		};
		return 0;
	}
};

} // End of namespace Snatcher

#endif // SNATCHER_ANIMATOR_H
