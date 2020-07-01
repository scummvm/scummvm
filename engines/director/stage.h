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

#ifndef DIRECTOR_STAGE_H
#define DIRECTOR_STAGE_H

#include "graphics/macgui/macwindow.h"

namespace Graphics {
class ManagedSurface;
class MacWindow;
class MacWindowManager;
}

namespace Director {

struct Channel;
struct TransParams;

class Stage : public Graphics::MacWindow {
 public:
	Stage(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm);
	// ~Stage();

	bool render(bool forceRedraw = false, Graphics::ManagedSurface *blitTo = nullptr);

	bool unionRectangle(Common::Rect &destRect, const Common::Rect &src1, const Common::Rect &src2);
	void setStageColor(uint stageColor);
	void addDirtyRect(const Common::Rect &r);
	void mergeDirtyRects();
	void reset();

	// transitions.cpp
	void playTransition(uint16 transDuration, uint8 transArea, uint8 transChunkSize, TransitionType transType, uint frame);
	void initTransParams(TransParams &t, Common::Rect &clipRect);
	void dissolveTrans(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);
	void dissolvePatternsTrans(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);
	void transMultiPass(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);
	void transZoom(TransParams &t, Common::Rect &clipRect, Graphics::ManagedSurface *tmpSurface);

public:
	Common::List<Common::Rect> _dirtyRects;
	Common::List<Channel *> _dirtyChannels;
	TransParams *_puppetTransition;

private:
	uint _stageColor;
	void inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo = nullptr);
	void drawReverseSprite(Channel *channel, Common::Rect &srcRect, Common::Rect &destRect, Graphics::ManagedSurface *blitTo);
	void drawMatteSprite(Channel *channel, Common::Rect &srcRect, Common::Rect &destRect, Graphics::ManagedSurface *blitTo);
};

} // end of namespace Director

#endif
