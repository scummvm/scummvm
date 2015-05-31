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

#include "zvision/scripting/control.h"
#include "zvision/scripting/script_manager.h"

#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"

#include "common/stream.h"

namespace ZVision {

void Control::parseFlatControl(ZVision *engine) {
	engine->getRenderManager()->getRenderTable()->setRenderState(RenderTable::FLAT);
}

void Control::parsePanoramaControl(ZVision *engine, Common::SeekableReadStream &stream) {
	RenderTable *renderTable = engine->getRenderManager()->getRenderTable();
	renderTable->setRenderState(RenderTable::PANORAMA);

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);

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
			uint point;
			sscanf(line.c_str(), "zeropoint(%u)", &point);
			renderTable->setPanoramaZeroPoint(point);
		}

		line = stream.readLine();
		engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	}

	renderTable->generateRenderTable();
}

// Only used in Zork Nemesis, handles tilt controls (ZGI doesn't have a tilt view)
void Control::parseTiltControl(ZVision *engine, Common::SeekableReadStream &stream) {
	RenderTable *renderTable = engine->getRenderManager()->getRenderTable();
	renderTable->setRenderState(RenderTable::TILT);

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);

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
		engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	}

	renderTable->generateRenderTable();
}

void Control::getParams(const Common::String &inputStr, Common::String &parameter, Common::String &values) {
	const char *chrs = inputStr.c_str();
	uint lbr;

	for (lbr = 0; lbr < inputStr.size(); lbr++)
		if (chrs[lbr] == '(')
			break;

	if (lbr >= inputStr.size())
		return;

	uint rbr;

	for (rbr = lbr + 1; rbr < inputStr.size(); rbr++)
		if (chrs[rbr] == ')')
			break;

	if (rbr >= inputStr.size())
		return;

	parameter = Common::String(chrs, chrs + lbr);
	values = Common::String(chrs + lbr + 1, chrs + rbr);
}

void Control::setVenus() {
	if (_venusId >= 0)
		if (_engine->getScriptManager()->getStateValue(_venusId) > 0)
			_engine->getScriptManager()->setStateValue(StateKey_Venus, _venusId);
}

} // End of namespace ZVision
