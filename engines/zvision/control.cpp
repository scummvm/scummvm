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

#include "common/stream.h"

#include "zvision/zvision.h"
#include "zvision/render_manager.h"
#include "zvision/render_table.h"
#include "zvision/script_manager.h"
#include "zvision/control.h"
#include "zvision/utility.h"

namespace ZVision {

void Control::parseFlatControl(ZVision *engine) {
	engine->getRenderManager()->getRenderTable()->setRenderState(RenderTable::FLAT);
}

void Control::parsePanoramaControl(ZVision *engine, Common::SeekableReadStream &stream) {
	RenderTable *renderTable = engine->getRenderManager()->getRenderTable();
	renderTable->setRenderState(RenderTable::PANORAMA);

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("angle*", true)) {
			float fov;
			sscanf(line.c_str(), "angle(%f)", &fov);
			renderTable->setPanoramaFoV(fov);
		} else if (line.matchString("linscale*", true)) {
			float scale;
			sscanf(line.c_str(), "linscale(%f)", &scale);
			renderTable->setPanoramaScale(scale);
		} else if (line.matchString("reversepana*", true)) {
			uint reverse;
			sscanf(line.c_str(), "reversepana(%u)", &reverse);
			if (reverse == 1) {
				renderTable->setPanoramaReverse(true);
			}
		} else if (line.matchString("zeropoint*", true)) {
			// TODO: Implement
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	renderTable->generateRenderTable();
}

void Control::parseTiltControl(ZVision *engine, Common::SeekableReadStream &stream) {
	RenderTable *renderTable = engine->getRenderManager()->getRenderTable();
	renderTable->setRenderState(RenderTable::TILT);

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("angle*", true)) {
			float fov;
			sscanf(line.c_str(), "angle(%f)", &fov);
			renderTable->setTiltFoV(fov);
		} else if (line.matchString("linscale*", true)) {
			float scale;
			sscanf(line.c_str(), "linscale(%f)", &scale);
			renderTable->setTiltScale(scale);
		} else if (line.matchString("reversepana*", true)) {
			uint reverse;
			sscanf(line.c_str(), "reversepana(%u)", &reverse);
			if (reverse == 1) {
				renderTable->setTiltReverse(true);
			}
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	renderTable->generateRenderTable();
}


//////////////////////////////////////////////////////////////////////////////
// PushToggleControl
//////////////////////////////////////////////////////////////////////////////

PushToggleControl::PushToggleControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
		: MouseEvent(key),
		  _engine(engine) {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

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
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	if (_hotspot.isEmpty() || _hoverCursor.empty()) {
		warning("Push_toggle cursor %u was parsed incorrectly", key);
	}
}

bool PushToggleControl::enable() {
	if (!_enabled) {
		_engine->registerMouseEvent(this);
		_enabled = true;
		return true;
	}

	debug("Control %u is already enabled", _key);
	return false;
}

bool PushToggleControl::disable() {
	if (_enabled) {
		_engine->removeMouseEvent(_key);
		_enabled = false;
		return true;
	}

	debug("Control %u is already disabled", _key);
	return false;
}

void PushToggleControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos) {
	_engine->getScriptManager()->setStateValue(_key, 1);
}

bool PushToggleControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos) {
	if (_hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_hoverCursor);
		return true;
	}

	return false;
}
} // End of namespace ZVision
