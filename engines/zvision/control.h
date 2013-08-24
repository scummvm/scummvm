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

#ifndef ZVISION_CONTROL_H
#define ZVISION_CONTROL_H

#include "common/types.h"

#include "zvision/mouse_event.h"
#include "zvision/action_node.h"

namespace Common {
class SeekableReadStream;
}

namespace ZVision {

class ZVision;
class RlfAnimation;
class ZorkAVIDecoder;

class Control {
public:
	Control() : _enabled(false) {}
	virtual ~Control() {}
	virtual bool enable() = 0;
	virtual bool disable() = 0;

protected:
	bool _enabled;

// Static member functions
public:
	static void parseFlatControl(ZVision *engine);
	static void parsePanoramaControl(ZVision *engine, Common::SeekableReadStream &stream);
	static void parseTiltControl(ZVision *engine, Common::SeekableReadStream &stream);
};


class PushToggleControl : public Control, public MouseEvent {
public:
	PushToggleControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	bool enable();
	bool disable();

	/**
	 * Called when LeftMouse is pushed. Calls ScriptManager::setStateValue(_key, 1);
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	void onMouseDown(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos);
	/**
	 * Called when LeftMouse is lifted. Does nothing
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	void onMouseUp(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos) {}
	/**
	 * Called on every MouseMove. Tests if the mouse is inside _hotspot, and if so, sets the cursor.
	 *
	 * @param engine                     The base engine
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 * @return                           Was the cursor changed?
	 */
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos);

private:
	ZVision * _engine;
	/**
	 * The area that will trigger the event
	 * This is in image space coordinates, NOT screen space
	 */
	Common::Rect _hotspot;
	/** The cursor to use when hovering over _hotspot */
	Common::String _hoverCursor;
};


class LeverControl : public Control, public MouseEvent, public ActionNode {
public:
	LeverControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~LeverControl();

private:
	enum FileType {
		RLF = 1,
		AVI = 2
	};

	struct Direction {
		Direction(uint angle, uint toFrame) : angle(angle), toFrame(toFrame) {}

		uint angle;
		uint toFrame;
	};

	struct FrameInfo {
		Common::Rect hotspot;
		Common::List<Direction> directions;
		Common::List<uint> returnRoute;
	};

	enum {
		ANGLE_DELTA = 30 // How far off a mouse angle can be and still be considered valid. This is in both directions, so the total buffer zone is (2 * ANGLE_DELTA)
	};

private:
	ZVision *_engine;

	union {
		RlfAnimation *rlf;
		ZorkAVIDecoder *avi;
	} _animation;
	FileType _fileType;

	Common::String _cursorName;
	Common::Rect _animationCoords;
	bool _mirrored;
	uint _frameCount;
	uint _startFrame;
	Common::Point _hotspotDelta;
	FrameInfo *_frameInfo;

	uint _currentFrame;
	bool _mouseIsCaptured;
	bool _isReturning;
	Common::Point _lastMousePos;
	Common::List<uint>::iterator _returnRoutesCurrentProgress;

public:
	bool enable();
	bool disable();
	void onMouseDown(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos);
	void onMouseUp(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos);
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos);
	bool process(uint32 deltaTimeInMillis);

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
};

// TODO: Implement InputControl
// TODO: Implement SaveControl
// TODO: Implement SlotControl
// TODO: Implement SafeControl
// TODO: Implement FistControl
// TODO: Implement HotMovieControl
// TODO: Implement PaintControl
// TODO: Implement TilterControl

} // End of namespace ZVision

#endif
