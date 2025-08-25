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

#ifndef ZVISION_LEVER_CONTROL_H
#define ZVISION_LEVER_CONTROL_H

#include "common/list.h"
#include "common/path.h"
#include "common/rect.h"
#include "math/vector2d.h"
#include "zvision/scripting/control.h"

namespace Video {
	class VideoDecoder;
}

namespace ZVision {

// Only used in Zork Nemesis, handles draggable levers (te2e, tm7e, tp2e, tt2e, tz2e)
class LeverControl : public Control {
public:
	LeverControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~LeverControl() override;

private:

	struct PathSegment {
		PathSegment(uint a, uint t) : angle(Math::deg2rad<float>(a)), toFrame(t), direction(cos(angle), -sin(angle)) {}

		float angle;	// Radians
		uint toFrame;
		Math::Vector2d direction;	// NB unit vector upon initialisation
		float distance = 1.0f;
	};

	struct FrameInfo {
		Common::Rect hotspot;
		Common::List<PathSegment> paths;
		Common::List<uint> returnRoute;
	};

private:
	Video::VideoDecoder *_animation;

	int _cursor;
	Common::Rect _animationCoords;
	bool _mirrored;
	uint _frameCount;
	uint _startFrame;
	Common::Point _hotspotDelta;
	FrameInfo *_frameInfo;

	uint _currentFrame;
	uint _lastRenderedFrame;
	bool _mouseIsCaptured;
	bool _isReturning;
	Common::Point _gripOffset;
	Common::List<uint>::iterator _returnRoutesCurrentProgress;
	uint _returnRoutesCurrentFrame;
	const uint8 _returnFramePeriod = 60;	// milliseconds
	uint32 _accumulatedTime;

public:
	bool onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool process(uint32 deltaTimeInMillis) override;

private:
	void parseLevFile(const Common::Path &fileName);
	void renderFrame(uint frameNumber);
	void getLevParams(const Common::String &inputStr, Common::String &parameter, Common::String &values);
	uint getNextFrame(Common::Point &deltaPos);
};

} // End of namespace ZVision

#endif
