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

#ifndef NANCY_ACTION_SECONDARYVIDEO_H
#define NANCY_ACTION_SECONDARYVIDEO_H

#include "engines/nancy/video.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// ActionRecord that shows NPC animations outside of dialogue. Supports
// different animations depending on whether the NPC is hovered by the mouse
class PlaySecondaryVideo : public ActionRecord, public RenderObject {
public:
	static const byte kNoVideoHotspots	= 1;
	static const byte kVideoHotspots	= 2;

	enum HoverState { kNoHover, kHover, kEndHover };

	PlaySecondaryVideo(uint chan) : RenderObject(8), channel(chan) {}
	virtual ~PlaySecondaryVideo() { _decoder.close(); }

	void init() override;
	void updateGraphics() override;
	void onPause(bool pause) override;
	void handleInput(NancyInput &input) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	Common::String _filename;
	Common::String _paletteFilename;
	// Common::String _bitmapOverlayFilename

	// TVD only
	uint16 _videoFormat = kLargeVideoFormat;
	uint16 _videoHotspots = kVideoHotspots;

	uint16 _loopFirstFrame = 0;
	uint16 _loopLastFrame = 0;
	uint16 _onHoverFirstFrame = 0;
	uint16 _onHoverLastFrame = 0;
	uint16 _onHoverEndFirstFrame = 0;
	uint16 _onHoverEndLastFrame = 0;
	SceneChangeDescription _sceneChange;

	Common::Array<SecondaryVideoDescription> _videoDescs;

protected:
	Common::String getRecordTypeName() const override { return Common::String::format("PlaySecondaryVideoChan%i", channel); }
	bool isViewportRelative() const override { return true; }

	Graphics::ManagedSurface _fullFrame;
	HoverState _hoverState = kNoHover;
	AVFDecoder _decoder;
	int _currentViewportFrame = -1;
	int _currentViewportScroll = -1;
	bool _isInFrame = false;
	bool _isHovered = false;

	uint channel;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_SECONDARYVIDEO_H
