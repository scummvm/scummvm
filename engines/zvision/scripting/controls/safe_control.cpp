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

#include "common/scummsys.h"

#include "zvision/scripting/controls/safe_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/cursors/cursor_manager.h"
#include "zvision/animation/meta_animation.h"
#include "zvision/utility/utility.h"

#include "common/stream.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/system.h"

#include "graphics/surface.h"


namespace ZVision {

SafeControl::SafeControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_SAFE) {
	_num_states = 0;
	_cur_state = 0;
	_animation = NULL;
	_radius_inner = 0;
	_radius_inner_sq = 0;
	_radius_outer = 0;
	_radius_outer_sq = 0;
	_zero_pointer = 0;
	_start_pointer = 0;
	_cur_frame = -1;
	_to_frame = 0;
	_frame_time = 0;
	_lastRenderedFrame = -1;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("animation", true)) {
			if (values.hasSuffix(".avi") || values.hasSuffix(".rlf"))
				_animation = new MetaAnimation(values, _engine);
		} else if (param.matchString("rectangle", true)) {
			int x;
			int y;
			int width;
			int height;

			sscanf(values.c_str(), "%d %d %d %d", &x, &y, &width, &height);

			_rectangle = Common::Rect(x, y, width, height);
		} else if (param.matchString("num_states", true)) {
			_num_states = atoi(values.c_str());
		} else if (param.matchString("center", true)) {
			int x;
			int y;

			sscanf(values.c_str(), "%d %d", &x, &y);
			_center = Common::Point(x,y);
		} else if (param.matchString("dial_inner_radius", true)) {
			_radius_inner = atoi(values.c_str());
			_radius_inner_sq = _radius_inner * _radius_inner;
		} else if (param.matchString("radius", true)) {
			_radius_outer = atoi(values.c_str());
			_radius_outer_sq = _radius_outer * _radius_outer;
		} else if (param.matchString("zero_radians_offset", true)) {
			_zero_pointer = atoi(values.c_str());
		} else if (param.matchString("pointer_offset", true)) {
			_start_pointer = atoi(values.c_str());
			_cur_state = _start_pointer;
		} else if (param.matchString("cursor", true)) {
			// Not used
		} else if (param.matchString("mirrored", true)) {
			// Not used
		} else if (param.matchString("venus_id", true)) {
			_venus_id = atoi(values.c_str());
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}
	renderFrame(_cur_state);
}

SafeControl::~SafeControl() {
	if (_animation)
		delete _animation;

}

void SafeControl::renderFrame(uint frameNumber) {
	if (frameNumber == 0) {
		_lastRenderedFrame = frameNumber;
	} else if ((int16)frameNumber < _lastRenderedFrame) {
		_lastRenderedFrame = frameNumber;
		frameNumber = (_num_states * 2) - frameNumber;
	} else {
		_lastRenderedFrame = frameNumber;
	}

	const Graphics::Surface *frameData;
	int x = _rectangle.left;
	int y = _rectangle.top;

	frameData = _animation->getFrameData(frameNumber);
	if (frameData)
		_engine->getRenderManager()->blitSurfaceToBkg(*frameData, x, y);
}

bool SafeControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_cur_frame != _to_frame) {
		_frame_time -= deltaTimeInMillis;

		if (_frame_time <= 0) {
			if (_cur_frame < _to_frame) {
				_cur_frame++;
				renderFrame(_cur_frame);
			} else if (_cur_frame > _to_frame) {
				_cur_frame--;
				renderFrame(_cur_frame);
			}
			_frame_time = _animation->frameTime();
		}
	}

	return false;
}

bool SafeControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_rectangle.contains(backgroundImageSpacePos)) {
		int32 mR = backgroundImageSpacePos.sqrDist(_center);
		if (mR <= _radius_outer_sq && mR >= _radius_inner_sq) {
			_engine->getCursorManager()->changeCursor(CursorIndex_Active);
			return true;
		}
	}

	return false;
}

bool SafeControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_rectangle.contains(backgroundImageSpacePos)) {
		int32 mR = backgroundImageSpacePos.sqrDist(_center);
		if (mR <= _radius_outer_sq && mR >= _radius_inner_sq) {

			Common::Point tmp = backgroundImageSpacePos - _center;

			float dd = atan2(tmp.x, tmp.y) * 57.29578;

			int16 dp_state = 360 / _num_states;

			int16 m_state = (_num_states - ((((int16)dd + 540) % 360) / dp_state)) % _num_states;

			int16 tmp2 = (m_state + _cur_state - _zero_pointer + _num_states - 1 ) % _num_states;

			_cur_frame = (_cur_state + _num_states - _start_pointer) % _num_states;

			_cur_state = (_num_states * 2 + tmp2) % _num_states;

			_to_frame = (_cur_state + _num_states - _start_pointer) % _num_states;

			_engine->getScriptManager()->setStateValue(_key, _cur_state);
			return true;
		}
	}

	return false;
}

} // End of namespace ZVision
