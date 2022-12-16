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

#include "tetraedge/game/scene_lights_xml_parser.h"
#include "tetraedge/te/te_light.h"

namespace Tetraedge {

bool SceneLightsXmlParser::parserCallback_Global(ParserNode *node) {
	_parent = Parent_Global;
	return true;
}

bool SceneLightsXmlParser::parseCol(ParserNode *node, TeColor &colout) {
	uint r = node->values["r"].asUint64();
	uint g = node->values["g"].asUint64();
	uint b = node->values["b"].asUint64();
	uint a;
	if (node->values.contains("a"))
		a = node->values["a"].asUint64();
	else
		a = 0xff;

	if (r > 255 || g > 255 || b > 255 | a > 255) {
		parserError("Invalid color values");
		return false;
	}
	colout = TeColor(r, g, b, a);
	return true;
}


bool SceneLightsXmlParser::parserCallback_Ambient(ParserNode *node) {
	// can appear under either global or light
	TeColor col;
	if (!parseCol(node, col))
		return false;

	if (_parent == Parent_Global) {
		TeLight::setGlobalAmbient(col);
	} else {
		_lights->back().setAmbient(col);
	}
	return true;
}

bool SceneLightsXmlParser::parserCallback_Lights(ParserNode *node) {
	// Nothing to do, data handled in the child keys.
	return true;
}

bool SceneLightsXmlParser::parserCallback_Light(ParserNode *node) {
	_parent = Parent_Light;
	_lights->push_back(TeLight());
	TeLightType ltype = TeLightType::LightTypeDirectional;
	if (node->values["Type"] == "Spot")
		ltype = TeLightType::LightTypeSpot;
	else if (node->values["Type"] == "Point")
		ltype = TeLightType::LightTypePoint;
	_lights->back().setType(ltype);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Position(ParserNode *node) {
	float x = atof(node->values["x"].c_str());
	float y = atof(node->values["y"].c_str());
	float z = atof(node->values["z"].c_str());
	_lights->back().setPosition3d(TeVector3f32(x, y, z));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Direction(ParserNode *node) {
	float h = (atof(node->values["h"].c_str()) * M_PI) / 180.0;
	float v = (atof(node->values["v"].c_str()) * M_PI) / 180.0;
	_lights->back().setPositionRadial(TeVector2f32(h, v));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Diffuse(ParserNode *node) {
	TeColor col;
	if (!parseCol(node, col))
		return false;

	_lights->back().setDiffuse(col);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Specular(ParserNode *node) {
	TeColor col;
	if (!parseCol(node, col))
		return false;

	_lights->back().setSpecular(col);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Attenuation(ParserNode *node) {
	_lights->back().setConstAtten(atof(node->values["constant"].c_str()));
	_lights->back().setLinearAtten(atof(node->values["linear"].c_str()));
	_lights->back().setQuadraticAtten(atof(node->values["quadratic"].c_str()));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Cutoff(ParserNode *node) {
	float f = atof(node->values["value"].c_str());
	_lights->back().setCutoff((f * M_PI) / 180.0);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Exponent(ParserNode *node) {
	_lights->back().setExponent(atof(node->values["value"].c_str()));
	return true;
}

bool SceneLightsXmlParser::parserCallback_DisplaySize(ParserNode *node) {
	_lights->back().setDisplaySize(atof(node->values["value"].c_str()));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Shadow(ParserNode *node) {
	_parent = Parent_Shadow;
	return true;
}

bool SceneLightsXmlParser::parserCallback_SourceLight(ParserNode *node) {
	_shadowLightNo = atof(node->values["number"].c_str());
	return true;
}

bool SceneLightsXmlParser::parserCallback_Fov(ParserNode *node) {
	_shadowFov = atof(node->values["value"].c_str());
	return true;
}

bool SceneLightsXmlParser::parserCallback_NearPlane(ParserNode *node) {
	_shadowNearPlane = atof(node->values["value"].c_str());
	return true;
}

bool SceneLightsXmlParser::parserCallback_FarPlane(ParserNode *node) {
	_shadowFarPlane = atof(node->values["value"].c_str());
	return true;
}

bool SceneLightsXmlParser::parserCallback_Color(ParserNode *node) {
	TeColor col;
	if (parseCol(node, col)) {
		_shadowColor = col;
		return true;
	}
	return false;
}



}
 // end namespace Tetraedge
