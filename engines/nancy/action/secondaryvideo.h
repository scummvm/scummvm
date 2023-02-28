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
#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// ActionRecord that shows NPC animations outside of dialogue. Supports
// different animations depending on whether the NPC is hovered by the mouse
class PlaySecondaryVideo : public ActionRecord, public RenderObject {
public:
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
	NancyFlag _enableHotspot = kTrue;

	uint16 _loopFirstFrame = 0; // 0x1E
	uint16 _loopLastFrame = 0; // 0x20
	uint16 _onHoverFirstFrame = 0; // 0x22
	uint16 _onHoverLastFrame = 0; // 0x24
	uint16 _onHoverEndFirstFrame = 0; // 0x26
	uint16 _onHoverEndLastFrame = 0; // 0x28
	SceneChangeDescription _sceneChange; // 0x2A
	// unknown byte
	Common::Array<SecondaryVideoDescription> _videoDescs; // 0x35

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
