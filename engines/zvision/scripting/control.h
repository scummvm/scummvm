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

#ifndef ZVISION_CONTROL_H
#define ZVISION_CONTROL_H

#include "common/keyboard.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
struct Point;
class WriteStream;
}

namespace ZVision {

class ZVision;

/**
 * The base class for all Controls.
 *
 * Controls are the things that the user interacts with. Ex: A lever on the door
 */
class Control {
public:

	enum ControlType {
		CONTROL_UNKNOW,
		CONTROL_INPUT,
		CONTROL_PUSHTGL,
		CONTROL_SLOT,
		CONTROL_LEVER,
		CONTROL_SAVE,
		CONTROL_SAFE,
		CONTROL_FIST,
		CONTROL_TITLER,
		CONTROL_HOTMOV,
		CONTROL_PAINT
	};

	Control(ZVision *engine, uint32 key, ControlType type) : _engine(engine), _key(key), _type(type), _venusId(-1) {}
	virtual ~Control() {}

	uint32 getKey() {
		return _key;
	}

	ControlType getType() {
		return _type;
	}

	virtual void focus() {}
	virtual void unfocus() {}
	/**
	 * Called when LeftMouse is pushed. Default is NOP.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	virtual bool onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
		return false;
	}
	/**
	 * Called when LeftMouse is lifted. Default is NOP.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	virtual bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
		return false;
	}
	/**
	 * Called on every MouseMove. Default is NOP.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 * @return                           Was the cursor changed?
	 */
	virtual bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
		return false;
	}
	/**
	 * Called when a key is pressed. Default is NOP.
	 *
	 * @param keycode    The key that was pressed
	 */
	virtual bool onKeyDown(Common::KeyState keyState) {
		return false;
	}
	/**
	 * Called when a key is released. Default is NOP.
	 *
	 * @param keycode    The key that was pressed
	 */
	virtual bool onKeyUp(Common::KeyState keyState) {
		return false;
	}
	/**
	 * Processes the node given the deltaTime since last frame. Default is NOP.
	 *
	 * @param deltaTimeInMillis    The number of milliseconds that have passed since last frame
	 * @return                     If true, the node can be deleted after process() finishes
	 */
	virtual bool process(uint32 deltaTimeInMillis) {
		return false;
	}

	void setVenus();

protected:
	ZVision *_engine;
	uint32 _key;
	int32 _venusId;

	void getParams(const Common::String &inputStr, Common::String &parameter, Common::String &values);
// Static member functions
public:
	static void parseFlatControl(ZVision *engine);
	static void parsePanoramaControl(ZVision *engine, Common::SeekableReadStream &stream);
	static void parseTiltControl(ZVision *engine, Common::SeekableReadStream &stream);
private:
	ControlType _type;
};

} // End of namespace ZVision

#endif
