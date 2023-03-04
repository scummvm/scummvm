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

bool SceneLightsXmlParser::parserCallback_Ambient(ParserNode *node) {
	// can appear under either global or light
	TeColor col;
	if (!parseCol(node, col))
		return false;

	if (_parent == Parent_Global) {
		TeLight::setGlobalAmbient(col);
	} else {
		_lights->back()->setAmbient(col);
	}
	return true;
}

bool SceneLightsXmlParser::parserCallback_Lights(ParserNode *node) {
	// Nothing to do, data handled in the child keys.
	return true;
}

bool SceneLightsXmlParser::parserCallback_Light(ParserNode *node) {
	_parent = Parent_Light;
	_lights->push_back(Common::SharedPtr<TeLight>(TeLight::makeInstance()));
	TeLightType ltype = TeLightType::LightTypeDirectional;
	if (node->values["Type"] == "Spot")
		ltype = TeLightType::LightTypeSpot;
	else if (node->values["Type"] == "Point")
		ltype = TeLightType::LightTypePoint;
	_lights->back()->setType(ltype);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Position(ParserNode *node) {
	_lights->back()->setPosition3d(parsePoint(node));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Direction(ParserNode *node) {
	float h = (float)((parseDouble(node, "h") * M_PI) / 180.0);
	float v = (float)((parseDouble(node, "v") * M_PI) / 180.0);
	_lights->back()->setPositionRadial(TeVector2f32(h, v));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Diffuse(ParserNode *node) {
	TeColor col;
	if (!parseCol(node, col))
		return false;

	_lights->back()->setDiffuse(col);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Specular(ParserNode *node) {
	TeColor col;
	if (!parseCol(node, col))
		return false;

	_lights->back()->setSpecular(col);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Attenuation(ParserNode *node) {
	float c = parseDouble(node, "constant");
	float l = parseDouble(node, "linear");
	float q = parseDouble(node, "quadratic");
	if (c < 0 || l < 0 || q < 0)
		warning("Loaded invalid lighting attenuation vals %f %f %f", c, l, q);
	_lights->back()->setConstAtten(c);
	_lights->back()->setLinearAtten(l);
	_lights->back()->setQuadraticAtten(q);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Cutoff(ParserNode *node) {
	float cutoff = parseDouble(node);
	if (cutoff < 0.0f || (cutoff > 90.0f && cutoff != 180.0f))
		warning("Loaded invalid lighting cutoff value %f", cutoff);
	_lights->back()->setCutoff((cutoff * M_PI) / 180.0);
	return true;
}

bool SceneLightsXmlParser::parserCallback_Exponent(ParserNode *node) {
	float expon = parseDouble(node);
	if (expon < 0.0f || expon > 128.0f) {
		// Print debug but don't bother warning - the value is not used anyway.
		debug("Loaded invalid lighting exponent value %f, default to 1.0", expon);
		expon = 1.0;
	}
	_lights->back()->setExponent(expon);
	return true;
}

bool SceneLightsXmlParser::parserCallback_DisplaySize(ParserNode *node) {
	_lights->back()->setDisplaySize(parseDouble(node));
	return true;
}

bool SceneLightsXmlParser::parserCallback_Shadow(ParserNode *node) {
	_parent = Parent_Shadow;
	return true;
}

bool SceneLightsXmlParser::parserCallback_SourceLight(ParserNode *node) {
	// Note: must be signed, -1 to disable shadows.
	_shadowLightNo = atoi(node->values["number"].c_str());
	return true;
}

bool SceneLightsXmlParser::parserCallback_Fov(ParserNode *node) {
	_shadowFov = parseDouble(node);
	return true;
}

bool SceneLightsXmlParser::parserCallback_NearPlane(ParserNode *node) {
	_shadowNearPlane = parseDouble(node);
	return true;
}

bool SceneLightsXmlParser::parserCallback_FarPlane(ParserNode *node) {
	_shadowFarPlane = parseDouble(node);
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
