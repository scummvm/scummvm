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
#include "zvision/cursors/cursor_manager.h"
#include "zvision/utility/utility.h"

#include "common/stream.h"


namespace ZVision {

PushToggleControl::PushToggleControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_PUSHTGL),
	  _countTo(2),
	  _event(Common::EVENT_LBUTTONUP) {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);
	line.toLowercase();

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("*_hotspot*", true)) {
			uint x;
			uint y;
			uint width;
			uint height;

			sscanf(line.c_str(), "%*[^(](%u,%u,%u,%u)", &x, &y, &width, &height);

			_hotspot = Common::Rect(x, y, x + width, y + height);
		} else if (line.matchString("cursor*", true)) {
			char nameBuffer[25];

			sscanf(line.c_str(), "%*[^(](%25[^)])", nameBuffer);

			_hoverCursor = Common::String(nameBuffer);
		} else if (line.matchString("animation*", true)) {
			// Not used
		} else if (line.matchString("sound*", true)) {
			// Not used
		} else if (line.matchString("count_to*", true)) {
			sscanf(line.c_str(), "%*[^(](%u)", &_countTo);
		} else if (line.matchString("mouse_event*", true)) {
			char nameBuffer[25];

			sscanf(line.c_str(), "%*[^(](%25[^)])", nameBuffer);

			Common::String evntStr(nameBuffer);
			if (evntStr.equalsIgnoreCase("up")) {
				_event = Common::EVENT_LBUTTONUP;
			} else if (evntStr.equalsIgnoreCase("down")) {
				_event = Common::EVENT_LBUTTONDOWN;
			} else if (evntStr.equalsIgnoreCase("double")) {
				// Not used
			}
		} else if (line.matchString("venus_id*", true)) {
			// Not used
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	if (_hotspot.isEmpty() || _hoverCursor.empty()) {
		warning("Push_toggle cursor %u was parsed incorrectly", key);
	}
}

PushToggleControl::~PushToggleControl() {
}

bool PushToggleControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_event != Common::EVENT_LBUTTONUP)
		return false;

	if (_hotspot.contains(backgroundImageSpacePos)) {
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

	if (_hotspot.contains(backgroundImageSpacePos)) {
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

	if (_hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_engine->getCursorManager()->getCursorId(_hoverCursor));
		return true;
	}

	return false;
}

} // End of namespace ZVision
