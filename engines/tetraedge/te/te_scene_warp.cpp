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

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_scene_warp.h"
#include "tetraedge/te/te_scene_warp_xml_parser.h"

namespace Tetraedge {

TeSceneWarp::TeSceneWarp() : _warp(nullptr), _numExitsCreated(0) {
}

TeSceneWarp::~TeSceneWarp() {
	close();
}

void TeSceneWarp::close() {
	_objects.clear();
	_animations.clear();
	_exits.clear();
	_warpEvents.clear();
	_warp = nullptr;
}

const TeSceneWarp::Animation *TeSceneWarp::getAnimation(const Common::String &name) {
	for (const auto &anim : _animations)
		if (anim._name == name)
			return &anim;
	return nullptr;
}

const TeSceneWarp::Exit *TeSceneWarp::getExit(const Common::String &linkedWarp) {
	for (const auto &exit : _exits)
		if (exit._linkedWarp == linkedWarp)
			return &exit;
	return nullptr;
}

const TeSceneWarp::Object *TeSceneWarp::getObject(const Common::String &name) {
	for (const auto &object : _objects)
		if (object._name == name)
			return &object;
	return nullptr;
}

const TeSceneWarp::WarpEvent *TeSceneWarp::getWarpEvent(const Common::String &name) {
	for (const auto &warpEvent : _warpEvents)
		if (warpEvent._name == name)
			return &warpEvent;
	return nullptr;
}

bool TeSceneWarp::load(const Common::Path &name, TeWarp *warp, bool flag) {
	close();
	_warp = warp;
	_numExitsCreated = 0;
	_name = name;

	TeSceneWarpXmlParser parser(this, flag);
	TeCore *core = g_engine->getCore();
	TetraedgeFSNode node = core->findFile(name);
	if (!node.loadXML(parser))
		error("TeSceneWarp::load: failed to load data from %s", name.toString(Common::Path::kNativeSeparator).c_str());
	if (!parser.parse())
		error("TeSceneWarp::load: failed to parse data from %s", name.toString(Common::Path::kNativeSeparator).c_str());

	if (flag) {
		// Line 357 ~ 426, plus other fixups
		error("TODO: Finish TeSceneWarp::load for flag == true");
	} else {
		// This is done during parsing but this should work too.
		for (const auto &sceneExit : _exits) {
			TeWarp::Exit warpExit;
			warpExit._name = sceneExit._name;
			warpExit._camAngleX = sceneExit._camAngleX;
			warpExit._camAngleY = sceneExit._camAngleY;
			warpExit._linkedWarpPath = Common::String("3D\\") + sceneExit._linkedWarp;
			warpExit._markerId = sceneExit._markerId;
			for (const auto &bloc : sceneExit._warpBlocs) {
				TeWarp::Block block;
				block._offset = bloc.offset();
				block._face = bloc.face();
				block._x = sceneExit._nbWarpBlock;
				block._y = sceneExit._nbWarpBlock;
				warpExit._warpBlockList.push_back(block);
			}
			warp->sendExit(warpExit);
		}
		_warp->activeMarkers(_warp->markersActive());
	}
	return true;
}

} // end namespace Tetraedge
