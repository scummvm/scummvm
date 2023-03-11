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

#ifndef TETRAEDGE_GAME_IN_GAME_SCENE_XML_PARSER_H
#define TETRAEDGE_GAME_IN_GAME_SCENE_XML_PARSER_H

#include "tetraedge/te/te_xml_parser.h"
#include "tetraedge/game/in_game_scene.h"

namespace Tetraedge {

/**
 * XML Parser for in game scene files in Syberia 2.
 * Sybeira 1 uses a binary format, see InGameScene::load.
 */
class InGameSceneXmlParser : public TeXmlParser {
public:
	InGameSceneXmlParser(InGameScene *scene)
		: _scene(scene), _textNodeType(TextNodeNone) {}

	// NOTE: This doesn't handle snowCustom tag which was
	// added in original but commented out in every place.
	CUSTOM_XML_PARSER(InGameSceneXmlParser) {
		XML_KEY(scene)
			XML_KEY(camera)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(pathZone)
				XML_PROP(name, true)
				XML_KEY(gridSize)
				KEY_END()
			KEY_END()
			XML_KEY(curve)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(dummy)
				XML_PROP(name, true)
				XML_KEY(position)
				KEY_END()
			KEY_END()
			XML_KEY(mask)
				XML_PROP(name, true)
				XML_PROP(texture, true)
			KEY_END()
			XML_KEY(dynamicLight)
				XML_PROP(name, true)
				XML_PROP(texture, true)
			KEY_END()
			XML_KEY(rippleMask)
				XML_PROP(name, true)
				XML_PROP(texture, true)
			KEY_END()
			XML_KEY(snowCone)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(shadowMask)
				XML_PROP(name, true)
				XML_PROP(texture, true)
			KEY_END()
			XML_KEY(shadowReceivingObject)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(zBufferObject)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(rObject)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(rBB)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(light)
				XML_PROP(name, true)
			KEY_END()
			XML_KEY(flamme)
				XML_KEY(name)
					XML_PROP(value, true)
				KEY_END()
				XML_KEY(center)
					XML_PROP(x, true)
					XML_PROP(y, true)
					XML_PROP(z, true)
				KEY_END()
				XML_KEY(yMax)
					XML_PROP(x, true)
					XML_PROP(y, true)
					XML_PROP(z, true)
				KEY_END()
				XML_KEY(offsetMin)
					XML_PROP(x, true)
					XML_PROP(y, true)
					XML_PROP(z, true)
				KEY_END()
				XML_KEY(offsetMax)
					XML_PROP(x, true)
					XML_PROP(y, true)
					XML_PROP(z, true)
				KEY_END()
			KEY_END()
			XML_KEY(collisionSlide)
			KEY_END()
			XML_KEY(coliisionSlide)
			KEY_END()
			XML_KEY(noCollisionSlide)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_scene(ParserNode *node) { return true; }
	bool parserCallback_camera(ParserNode *node);
	bool parserCallback_pathZone(ParserNode *node);
	bool parserCallback_gridSize(ParserNode *node);
	bool parserCallback_curve(ParserNode *node);
	bool parserCallback_dummy(ParserNode *node);
	bool parserCallback_position(ParserNode *node);
	bool parserCallback_mask(ParserNode *node);
	bool parserCallback_dynamicLight(ParserNode *node);
	bool parserCallback_rippleMask(ParserNode *node);
	bool parserCallback_snowCone(ParserNode *node);
	bool parserCallback_shadowMask(ParserNode *node);
	bool parserCallback_shadowReceivingObject(ParserNode *node);
	bool parserCallback_zBufferObject(ParserNode *node);
	bool parserCallback_rObject(ParserNode *node);
	bool parserCallback_rBB(ParserNode *node);
	bool parserCallback_light(ParserNode *node);
	bool parserCallback_collisionSlide(ParserNode *node);
	bool parserCallback_coliisionSlide(ParserNode *node);
	bool parserCallback_noCollisionSlide(ParserNode *node);

	// Flamme and its children.
	bool parserCallback_flamme(ParserNode *node);
	bool parserCallback_name(ParserNode *node);
	bool parserCallback_center(ParserNode *node);
	bool parserCallback_yMax(ParserNode *node);
	bool parserCallback_offsetMin(ParserNode *node);
	bool parserCallback_offsetMax(ParserNode *node);

	virtual bool closedKeyCallback(ParserNode *node) override;
	virtual bool textCallback(const Common::String &val) override;

public:
	InGameScene *_scene;

	// Free Move Zones have to be handled separately just to handle a single
	// corner case where the grid size is overridden.
	TeVector2f32 _fmzGridSize;

	enum TextNodeType {
		TextNodeNone,
		TextNodePosition,
		TextNodeGridSize
	};

	TextNodeType _textNodeType;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_IN_GAME_SCENE_XML_PARSER_H
