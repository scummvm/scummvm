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

#ifndef ZVISION_LEVER_CONTROL_H
#define ZVISION_LEVER_CONTROL_H

#include "zvision/scripting/control.h"

#include "common/list.h"
#include "common/rect.h"

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

	struct Direction {
		Direction(uint a, uint t) : angle(a), toFrame(t) {}

		uint angle;
		uint toFrame;
	};

	struct FrameInfo {
		Common::Rect hotspot;
		Common::List<Direction> directions;
		Common::List<uint> returnRoute;
	};

	enum {
		ANGLE_DELTA = 30, // How far off a mouse angle can be and still be considered valid. This is in both directions, so the total buffer zone is (2 * ANGLE_DELTA)
		ANIMATION_FRAME_TIME = 30 // In millis
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
	Common::Point _lastMousePos;
	Common::List<uint>::iterator _returnRoutesCurrentProgress;
	uint _returnRoutesCurrentFrame;
	uint32 _accumulatedTime;

public:
	bool onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool process(uint32 deltaTimeInMillis) override;

private:
	void parseLevFile(const Common::String &fileName);
	/**
	 * Calculates the angle a vector makes with the negative y-axis
	 *
	 *                 90
	 *  pointTwo *     ^
	 *            \    |
	 *             \   |
	 *              \  |
	 *               \ |
	 *        angle ( \|pointOne
	 * 180 <-----------*-----------> 0
	 *                 |
	 *                 |
	 *                 |
	 *                 |
	 *                 |
	 *                 ^
	 *                270
	 *
	 * @param pointOne    The origin of the vector
	 * @param pointTwo    The end of the vector
	 * @return            The angle the vector makes with the negative y-axis
	 */
	static int calculateVectorAngle(const Common::Point &pointOne, const Common::Point &pointTwo);
	void renderFrame(uint frameNumber);
	void getLevParams(const Common::String &inputStr, Common::String &parameter, Common::String &values);
};

} // End of namespace ZVision

#endif
