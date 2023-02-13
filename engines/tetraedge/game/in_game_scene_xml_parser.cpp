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

#include "tetraedge/game/in_game_scene_xml_parser.h"
#include "tetraedge/game/in_game_scene.h"

namespace Tetraedge {

bool InGameSceneXmlParser::parserCallback_camera(ParserNode *node) {
	_scene->loadCamera(node->values["name"]);
	return true;
}

bool InGameSceneXmlParser::parserCallback_pathZone(ParserNode *node) {
	_fmzGridSize = TeVector2f32();
	// Handled in closedKeyCallback
	return true;
}

bool InGameSceneXmlParser::parserCallback_gridSize(ParserNode *node) {
	_textNodeType = TextNodeGridSize;
	return true;
}

bool InGameSceneXmlParser::parserCallback_curve(ParserNode *node) {
	_scene->loadCurve(node->values["name"]);
	return true;
}

bool InGameSceneXmlParser::parserCallback_dummy(ParserNode *node) {
	_scene->_dummies.push_back(InGameScene::Dummy());
	_scene->_dummies.back()._name = node->values["name"];
	return true;
}

bool InGameSceneXmlParser::parserCallback_position(ParserNode *node) {
	_textNodeType = TextNodePosition;
	return true;
}

bool InGameSceneXmlParser::parserCallback_mask(ParserNode *node) {
	_scene->loadMask(node->values["name"], node->values["texture"],
		_scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_dynamicLight(ParserNode *node) {
	_scene->loadDynamicLightBloc(node->values["name"], node->values["texture"],
		_scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_rippleMask(ParserNode *node) {
	_scene->loadRippleMask(node->values["name"], node->values["texture"],
		_scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_snowCone(ParserNode *node) {
	// doesn't call the function in the game..
	/*_scene->loadSnowCone(node->values["name"], node->values["texture"],
		_scene->getZoneName(), _scene->getSceneName());*/
	return true;
}

bool InGameSceneXmlParser::parserCallback_shadowMask(ParserNode *node) {
	_scene->loadShadowMask(node->values["name"], node->values["texture"],
		_scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_shadowReceivingObject(ParserNode *node) {
	return true;
}

bool InGameSceneXmlParser::parserCallback_zBufferObject(ParserNode *node) {
	_scene->loadZBufferObject(node->values["name"], _scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_rObject(ParserNode *node) {
	_scene->loadRObject(node->values["name"], _scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_rBB(ParserNode *node) {
	_scene->loadRBB(node->values["name"], _scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_light(ParserNode *node) {
	_scene->loadLight(node->values["name"], _scene->getZoneName(), _scene->getSceneName());
	return true;
}

bool InGameSceneXmlParser::parserCallback_collisionSlide(ParserNode *node) {
	_scene->setCollisionSlide(true);
	return true;
}

bool InGameSceneXmlParser::parserCallback_noCollisionSlide(ParserNode *node) {
	_scene->setCollisionSlide(false);
	return true;
}

bool InGameSceneXmlParser::closedKeyCallback(ParserNode *node) {
	_textNodeType = TextNodeNone;
	if (node->name == "pathZone") {
		_scene->loadFreeMoveZone(node->values["name"], _fmzGridSize);
	}
	return true;
}

bool InGameSceneXmlParser::textCallback(const Common::String &val) {
	switch (_textNodeType) {
	case TextNodePosition: {
		TeVector3f32 pos;
		if (!pos.parse(val)) {
			parserError("Can't parse dummy position");
			return false;
		}
		_scene->_dummies.back()._position = pos;
		break;
	}
	case TextNodeGridSize: {
		TeVector2f32 sz;
		if (!sz.parse(val)) {
			parserError("Can't parse gridSize");
			return false;
		}
		_fmzGridSize = sz;
	}
	// fall through
	// FIXME: Is this intentional or break missing?
	default:
		parserError("Unexpected text block");
		return false;
		break;
	}
	return true;
}


} // end namespace Tetraedge
