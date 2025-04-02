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

#ifndef SNATCHER_RENDER_H
#define SNATCHER_RENDER_H

#include "common/platform.h"
#include "snatcher/graphics.h"
#include "snatcher/resource.h"

namespace Snatcher {

class Renderer {
public:
	virtual ~Renderer() {}

	virtual bool enqueueCopyCommands(ResourcePointer &res) = 0;
	virtual void clearCopyCommands() = 0;
	virtual void initData(ResourcePointer &res, uint8 mode) = 0;
	virtual void initAnimations(ResourcePointer &res, uint16 len) = 0;
	virtual void linkAnimations(ResourcePointer &res, uint16 len) = 0;
	virtual void clearAnimations(int mode = 0) = 0;
	virtual void setPlaneMode(uint16 mode) = 0;
	virtual void updateScreen(uint8 *screen) = 0;
	virtual void updateAnimations() = 0;

	virtual void anim_setControlFlags(uint8 animObjId, int flags) = 0;
	virtual void anim_clearControlFlags(uint8 animObjId, int flags) = 0;
	virtual void anim_setFrame(uint8 animObjId, uint16 frameNo) = 0;
	virtual uint16 anim_getCurFrameNo(uint8 animObjId) const = 0;
	virtual bool anim_isEnabled(uint8 animObjId) const = 0;

	virtual uint16 screenWidth() const = 0;
	virtual uint16 screenHeight() const = 0;

protected:
	Renderer(GraphicsEngine::GfxState &state) : _gfxState(state) {}
	GraphicsEngine::GfxState &_gfxState;

private:
	static Renderer *createSegaRenderer(GraphicsEngine::GfxState &state, Palette *pal, ScrollManager *scr);

public:
	static Renderer *create(Common::Platform platform, GraphicsEngine::GfxState &state, Palette *pal, ScrollManager *scr) {
		switch (platform) {
		case Common::kPlatformSegaCD:
			return createSegaRenderer(state, pal, scr);
		default:
			break;
		};
		return 0;
	}
};

} // End of namespace Snatcher

#endif // SNATCHER_RENDER_H
