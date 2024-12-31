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

#ifndef TETRAEDGE_GAME_CHARACTER_SETTINGS_XML_PARSER_H
#define TETRAEDGE_GAME_CHARACTER_SETTINGS_XML_PARSER_H

#include "common/formats/xmlparser.h"
#include "tetraedge/game/character.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class CharacterSettingsXmlParser : public Common::XMLParser {
public:
	void setCharacterSettings(Common::HashMap<Common::String, Character::CharacterSettings> *settings) {
		_characterSettings = settings;
	};

	// Parser
	CUSTOM_XML_PARSER(CharacterSettingsXmlParser) {
		XML_KEY(ModelsSettings)
			XML_KEY(Model)
				XML_PROP(name, true)
				XML_KEY(modelFileName)
				KEY_END()
				XML_KEY(defaultScale)
				KEY_END()
				XML_KEY(invertNormals)
				KEY_END()
				XML_KEY(walk)
					XML_KEY(animationFileName)
					KEY_END()
					XML_KEY(walkType)
						XML_PROP(name, true)
						XML_KEY(start)
							XML_PROP(file, true)
							XML_PROP(stepRight, false)
							XML_PROP(stepLeft, false)
						KEY_END()
						XML_KEY(loop)
							XML_PROP(file, true)
							XML_PROP(stepRight, false)
							XML_PROP(stepLeft, false)
						KEY_END()
						XML_KEY(endD)
							XML_PROP(file, true)
							XML_PROP(stepRight, false)
							XML_PROP(stepLeft, false)
						KEY_END()
						XML_KEY(endG)
							XML_PROP(file, true)
							XML_PROP(stepRight, false)
							XML_PROP(stepLeft, false)
						KEY_END()
					KEY_END()
					XML_KEY(speed)
					KEY_END()
				KEY_END()
				XML_KEY(cutSceneCurveDemi)
					XML_KEY(position)
					KEY_END()
				KEY_END()
				XML_KEY(face)
					XML_PROP(name, true)
					XML_KEY(eyes)
					KEY_END()
					XML_KEY(mouth)
					KEY_END()
				KEY_END()
				XML_KEY(body)
					XML_PROP(name, true)
				KEY_END()
				XML_KEY(rippleTexture)
					XML_PROP(path, true)
				KEY_END()
			KEY_END()
		KEY_END()
	} PARSER_END()

	// Parser callback methods
	bool parserCallback_ModelsSettings(ParserNode *node);
	bool parserCallback_Model(ParserNode *node);
	bool parserCallback_modelFileName(ParserNode *node);
	bool parserCallback_defaultScale(ParserNode *node);
	bool parserCallback_walk(ParserNode *node);
	bool parserCallback_animationFileName(ParserNode *node);
	bool parserCallback_walkType(ParserNode *node);
	bool parserCallback_start(ParserNode *node);	// walk anim
	bool parserCallback_loop(ParserNode *node);		// walk anim
	bool parserCallback_endD(ParserNode *node);		// for walk anim
	bool parserCallback_endG(ParserNode *node);		// for walk anim
	bool parserCallback_speed(ParserNode *node);	// walk speed
	bool parserCallback_cutSceneCurveDemi(ParserNode *node);
	bool parserCallback_position(ParserNode *node);	// position of cutSceneCurveDemi
	bool parserCallback_face(ParserNode *node);
	bool parserCallback_eyes(ParserNode *node);
	bool parserCallback_mouth(ParserNode *node);
	bool parserCallback_body(ParserNode *node);
	bool parserCallback_invertNormals(ParserNode *node);
	bool parserCallback_rippleTexture(ParserNode *node);

	bool textCallback(const Common::String &val) override;
	bool handleUnknownKey(ParserNode *node) override;

private:
	Character::AnimSettings parseWalkAnimSettings(const ParserNode *node) const;

	enum TextTagType {
		TagModelFileName,
		TagDefaultScale,
		TagAnimationFileName,
		TagEyes,
		TagMouth,
		TagSpeed,
		TagPosition,
		TagBody
	};

	TextTagType _curTextTag;
	Character::CharacterSettings *_curCharacter;
	Character::WalkSettings *_curWalkSettings;
	Common::HashMap<Common::String, Character::CharacterSettings> *_characterSettings;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_CHARACTER_SETTINGS_XML_PARSER_H
