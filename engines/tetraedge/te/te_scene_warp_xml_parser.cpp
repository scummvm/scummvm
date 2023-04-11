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

#include "tetraedge/te/te_scene_warp_xml_parser.h"

namespace Tetraedge {

bool TeSceneWarpXmlParser::parserCallback_exit(ParserNode *node) {
	TeSceneWarp::Exit exit;
	exit._name = Common::String::format("Exit_%02d", _sceneWarp->_exits.size());
	exit._linkedWarp = node->values.getVal("linkedWarp");
	exit._nbWarpBlock = parseUint(node, "nbWarpBlock");
	_sceneWarp->_exits.push_back(exit);
	_objType = kObjExit;
	return true;
}

bool TeSceneWarpXmlParser::parserCallback_camera(ParserNode *node) {
	if (_objType != kObjExit) {
		parserError("block should only appear inside exit");
		return false;
	}
	TeSceneWarp::Exit &exit = _sceneWarp->_exits.back();
	exit._camAngleX = (float)parseDouble(node, "angleX");
	exit._camAngleY = (float)parseDouble(node, "angleY");
	return true;
}

bool TeSceneWarpXmlParser::parserCallback_marker(ParserNode *node) {
	TeVector3f32 vec = parsePoint(node);
	if (_objType == kObjExit) {
		TeSceneWarp::Exit &exit = _sceneWarp->_exits.back();
		TeMarker *marker = _sceneWarp->_warp->allocMarker(&exit._markerId);
		marker->loc() = vec;
		if (_flag) {
			marker->loc().normalize();
			marker->loc() *= 500.0f;
			marker->button().load("2D/Menus/InGame/Marker_2.png", "2D/Menus/InGame/Marker_2_over.png", "");
		} else {
			marker->visible(false);
		}
		marker->setSomeFloat(999.0f);
		_sceneWarp->_warp->sendMarker(exit._name, exit._markerId);
	} else if (_objType == kObjObject) {
		TeSceneWarp::Object &obj = _sceneWarp->_objects.back();
		TeMarker *marker = _sceneWarp->_warp->allocMarker(&obj._markerId);
		marker->loc() = vec;
		if (_flag) {
			marker->loc().normalize();
			marker->loc() *= 500.0f;
			marker->button().load("2D/Menus/InGame/Marker_2.png", "2D/Menus/InGame/Marker_2_over.png", "");
		} else {
			marker->visible(false);
		}
		marker->setSomeFloat(999.0f);
		if (_sceneWarp->_warp->hasObjectOrAnim(obj._name)) {
			_sceneWarp->_warp->sendMarker(obj._name, obj._markerId);
		}
	} else {
		parserError("marker tag under invalid parent.");
		return false;
	}
	return true;
}

bool TeSceneWarpXmlParser::parserCallback_block(ParserNode *node) {
	if (_objType != kObjExit) {
		parserError("block should only appear inside exit");
		return false;
	}
	TeWarpBloc::CubeFace face = static_cast<TeWarpBloc::CubeFace>(parseUint(node, "face"));
	TeVector2s32 offset;
	offset._x = parseUint(node, "offsetX");
	offset._y = parseUint(node, "offsetY");
	TeSceneWarp::Exit &exit = _sceneWarp->_exits.back();
	if (_flag) {
		TeWarpBloc bloc;
		bloc.create(face, exit._nbWarpBlock, exit._nbWarpBlock, offset);
		bloc.color(TeColor(0, 0, 0xff, 0x80));
		exit._warpBlocs.push_back(bloc);
	} // else, create a TeWarp::Block which we do after parsing.
	error("TODO: Finish TeSceneWarpXmlParser::parserCallback_block");
}

bool TeSceneWarpXmlParser::parserCallback_object(ParserNode *node) {
	TeSceneWarp::Object obj;
	obj._name = node->values.getVal("name");
	_sceneWarp->_objects.push_back(obj);
	_objType = kObjObject;
	return true;
}

bool TeSceneWarpXmlParser::parserCallback_animation(ParserNode *node) {
	TeSceneWarp::Animation anim;
	anim._name = node->values.getVal("name");
	anim._fps = (float)parseDouble(node, "fps");
	_sceneWarp->_animations.push_back(anim);
	_objType = kObjNone;
	return true;
}

} // end namespace Tetraedge
