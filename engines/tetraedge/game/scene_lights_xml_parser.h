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

#ifndef TETRAEDGE_GAME_SCENE_LIGHTS_XML_PARSER_H
#define TETRAEDGE_GAME_SCENE_LIGHTS_XML_PARSER_H

#include "common/formats/xmlparser.h"
#include "tetraedge/te/te_light.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_xml_parser.h"

namespace Tetraedge {

class SceneLightsXmlParser : public TeXmlParser {
public:
	SceneLightsXmlParser(Common::Array<Common::SharedPtr<TeLight>> *lights) :
		_lights(lights), _shadowLightNo(-1), _shadowFarPlane(0),
		_shadowNearPlane(0), _shadowFov(0), _parent(ParentNone) {}

	TeColor getShadowColor() { return _shadowColor; }
	int getShadowLightNo() { return _shadowLightNo; }
	float getShadowFarPlane() { return _shadowFarPlane; }
	float getShadowNearPlane() { return _shadowNearPlane; }
	float getShadowFov() { return _shadowFov; }

	// Parser
	CUSTOM_XML_PARSER(SceneLightsXmlParser) {
		XML_KEY(Global)
			XML_KEY(Ambient)
				XML_PROP(r, true)
				XML_PROP(g, true)
				XML_PROP(b, true)
			KEY_END()
		KEY_END()
		XML_KEY(Lights)
			XML_KEY(Light)
				XML_PROP(Type, true)
				XML_KEY(Position)
					XML_PROP(x, true)
					XML_PROP(y, true)
					XML_PROP(z, true)
				KEY_END()
				XML_KEY(Direction)
					XML_PROP(h, true)
					XML_PROP(v, true)
				KEY_END()
				XML_KEY(Ambient)
					XML_PROP(r, true)
					XML_PROP(g, true)
					XML_PROP(b, true)
				KEY_END()
				XML_KEY(Diffuse)
					XML_PROP(r, true)
					XML_PROP(g, true)
					XML_PROP(b, true)
				KEY_END()
				XML_KEY(Specular)
					XML_PROP(r, true)
					XML_PROP(g, true)
					XML_PROP(b, true)
				KEY_END()
				XML_KEY(Attenuation)
					XML_PROP(constant, true)
					XML_PROP(linear, true)
					XML_PROP(quadratic, true)
				KEY_END()
				XML_KEY(Cutoff)
					XML_PROP(value, true)
				KEY_END()
				XML_KEY(Exponent)
					XML_PROP(value, true)
				KEY_END()
				XML_KEY(DisplaySize)
					XML_PROP(value, true)
				KEY_END()
			KEY_END()
		KEY_END()
		XML_KEY(Shadow)
			XML_KEY(SourceLight)
				XML_PROP(number, true)
			KEY_END()
			XML_KEY(Fov)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(NearPlane)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(FarPlane)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(Color)
				XML_PROP(r, true)
				XML_PROP(g, true)
				XML_PROP(b, true)
				XML_PROP(a, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

private:
	Common::Array<Common::SharedPtr<TeLight>> *_lights;

	enum ParentNodeType {
		ParentNone,
		ParentGlobal,
		ParentLight,
		ParentShadow
	};

	TeColor _shadowColor;
	int _shadowLightNo;
	float _shadowFarPlane;
	float _shadowNearPlane;
	float _shadowFov;
	ParentNodeType _parent;

	// Parser callback methods
	bool parserCallback_Global(ParserNode *node);
	bool parserCallback_Ambient(ParserNode *node);

	bool parserCallback_Lights(ParserNode *node);
	bool parserCallback_Light(ParserNode *node);
	bool parserCallback_Position(ParserNode *node);
	bool parserCallback_Direction(ParserNode *node);
	//bool parserCallback_Ambient(ParserNode *node);
	bool parserCallback_Diffuse(ParserNode *node);
	bool parserCallback_Specular(ParserNode *node);
	bool parserCallback_Attenuation(ParserNode *node);
	bool parserCallback_Cutoff(ParserNode *node);
	bool parserCallback_Exponent(ParserNode *node);
	bool parserCallback_DisplaySize(ParserNode *node);

	bool parserCallback_Shadow(ParserNode *node);
	bool parserCallback_SourceLight(ParserNode *node);
	bool parserCallback_Fov(ParserNode *node);
	bool parserCallback_NearPlane(ParserNode *node);
	bool parserCallback_FarPlane(ParserNode *node);
	bool parserCallback_Color(ParserNode *node);

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_SCENE_LIGHTS_XML_PARSER_H
