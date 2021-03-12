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
 */

#ifndef NANCY_UI_VIEWPORT_H
#define NANCY_UI_VIEWPORT_H

#include "engines/nancy/renderobject.h"

#include "engines/nancy/video.h"
#include "engines/nancy/time.h"

#include "common/str.h"
#include "common/rect.h"

namespace Nancy {

class Scene;
struct NancyInput;

namespace UI {

class Viewport : public Nancy::RenderObject {
public:
    Viewport() :
        RenderObject(),
        _movementLastFrame(0),
        _edgesMask(0) {}
    virtual ~Viewport() { _decoder.close(); _fullFrame.free(); }

    virtual void init() override;
    void handleInput(NancyInput &input);

    void loadVideo(const Common::String &filename, uint frameNr = 0, uint verticalScroll = 0, const Common::String &palette = Common::String());
    
    void setFrame(uint frameNr);
    void setNextFrame();
    void setPreviousFrame();

    void setVerticalScroll(uint scroll);
    void scrollUp(uint delta);
    void scrollDown(uint delta);

    uint16 getFrameCount() const { return _decoder.isVideoLoaded() ? _decoder.getFrameCount() : 0; }
    uint16 getCurFrame() const { return _currentFrame; }
    uint16 getCurVerticalScroll() const { return _drawSurface.getOffsetFromOwner().y - 1; }
    uint16 getMaxScroll() const { return _fullFrame.h - _drawSurface.h - 1; }

    Common::Rect getBoundsByFormat(uint format) const; // used by video

    Common::Rect convertViewportToScreen(const Common::Rect &viewportRect) const;
    Common::Rect convertScreenToViewport(const Common::Rect &viewportRect) const;

    // 0 is inactive, -1 is keep unchanged
    void setEdgesSize(uint16 upSize, uint16 downSize, uint16 leftSize, uint16 rightSize);
    void disableEdges(byte edges);
    void enableEdges(byte edges);

protected:
    virtual uint16 getZOrder() const override { return 6; }

    Common::Rect _upHotspot;
    Common::Rect _downHotspot;
    Common::Rect _leftHotspot;
    Common::Rect _rightHotspot;

    byte _edgesMask;
    
    byte _movementLastFrame;
    Time _nextMovementTime;

    AVFDecoder _decoder;
    uint16 _currentFrame;
    Graphics::ManagedSurface _fullFrame;
    Common::Rect _format1Bounds;
    Common::Rect _format2Bounds;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_VIEWPORT_H
