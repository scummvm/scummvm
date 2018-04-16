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

#include "common/scummsys.h"

#include "zvision/scripting/controls/push_toggle_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"

#include "common/stream.h"

namespace ZVision {

PushToggleControl::PushToggleControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_PUSHTGL),
	  _countTo(2),
	  _cursor(CursorIndex_Active),
	  _event(Common::EVENT_LBUTTONUP) {

	_hotspots.clear();

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("*_hotspot", true)) {
			uint x;
			uint y;
			uint width;
			uint height;

			sscanf(values.c_str(), "%u,%u,%u,%u", &x, &y, &width, &height);

			_hotspots.push_back(Common::Rect(x, y, x + width + 1, y + height + 1));
		} else if (param.matchString("cursor", true)) {
			_cursor = _engine->getCursorManager()->getCursorId(values);
		} else if (param.matchString("animation", true)) {
			// Not used
		} else if (param.matchString("sound", true)) {
			// Not used
		} else if (param.matchString("count_to", true)) {
			sscanf(values.c_str(), "%u", &_countTo);
		} else if (param.matchString("mouse_event", true)) {
			if (values.equalsIgnoreCase("up")) {
				_event = Common::EVENT_LBUTTONUP;
			} else if (values.equalsIgnoreCase("down")) {
				_event = Common::EVENT_LBUTTONDOWN;
			} else if (values.equalsIgnoreCase("double")) {
				// Not used
			}
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}

	if (_hotspots.size() == 0) {
		warning("Push_toggle %u was parsed incorrectly", key);
	}
}

PushToggleControl::~PushToggleControl() {
	_hotspots.clear();
}

bool PushToggleControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_event != Common::EVENT_LBUTTONUP)
		return false;

	if (contain(backgroundImageSpacePos)) {
		setVenus();
		int32 val = _engine->getScriptManager()->getStateValue(_key);
		val = (val + 1) % _countTo;
		_engine->getScriptManager()->setStateValue(_key, val);
		return true;
	}
	return false;
}

bool PushToggleControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_event != Common::EVENT_LBUTTONDOWN)
		return false;

	if (contain(backgroundImageSpacePos)) {
		setVenus();
		int32 val = _engine->getScriptManager()->getStateValue(_key);
		val = (val + 1) % _countTo;
		_engine->getScriptManager()->setStateValue(_key, val);
		return true;
	}
	return false;
}

bool PushToggleControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (contain(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_cursor);
		return true;
	}

	return false;
}

bool PushToggleControl::contain(const Common::Point &point) {
	for (uint i = 0; i < _hotspots.size(); i++)
		if (_hotspots[i].contains(point))
			return true;
	return false;
}

} // End of namespace ZVision
