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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_UI_VIEWPORT_H
#define NANCY_UI_VIEWPORT_H

#include "engines/nancy/time.h"
#include "engines/nancy/video.h"

#include "engines/nancy/renderobject.h"

namespace Common {
class String;
}

namespace Nancy {

class Scene;
struct NancyInput;

namespace UI {

class Viewport : public Nancy::RenderObject {
public:
	static const byte kPanNone		= 0;
	static const byte kPan360		= 1;
	static const byte kPanLeftRight	= 2;

	Viewport() :
		RenderObject(6),
		_movementLastFrame(0),
		_edgesMask(0),
		_currentFrame(0),
		_videoFormat(0),
		_stickyCursorPos(-1, -1),
		_panningType(kPanNone) {}

	virtual ~Viewport() { _decoder.close(); _fullFrame.free(); }

	void init() override;
	void handleInput(NancyInput &input);

	void loadVideo(const Common::String &filename, uint frameNr = 0, uint verticalScroll = 0, byte panningType = kPanNone, uint16 format = 2, const Common::String &palette = Common::String());
	void setPalette(const Common::String &paletteName);
	void setPalette(const Common::String &paletteName, uint paletteStart, uint paletteSize);

	void setFrame(uint frameNr);
	void setNextFrame();
	void setPreviousFrame();

	void setVerticalScroll(uint scroll);
	void scrollUp(uint delta);
	void scrollDown(uint delta);

	uint16 getFrameCount() const { return _decoder.isVideoLoaded() ? _decoder.getFrameCount() : 0; }
	uint16 getCurFrame() const { return _currentFrame; }
	uint16 getCurVerticalScroll() const { return _drawSurface.getOffsetFromOwner().y; }
	uint16 getMaxScroll() const;

	Common::Rect getBoundsByFormat(uint format) const; // used by video

	Common::Rect convertViewportToScreen(const Common::Rect &viewportRect) const;
	Common::Rect convertScreenToViewport(const Common::Rect &viewportRect) const;

	void disableEdges(byte edges);
	void enableEdges(byte edges);

protected:
	void setEdgesSize(uint16 upSize, uint16 downSize, uint16 leftSize, uint16 rightSize);

	Common::Rect _nonScrollZone;
	byte _edgesMask;

	byte _movementLastFrame;
	Time _nextMovementTime;

	byte _panningType;

	AVFDecoder _decoder;
	uint16 _currentFrame;
	uint16 _videoFormat;
	Graphics::ManagedSurface _fullFrame;
	Common::Rect _format1Bounds;
	Common::Rect _format2Bounds;
	Common::Point _stickyCursorPos;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_VIEWPORT_H
