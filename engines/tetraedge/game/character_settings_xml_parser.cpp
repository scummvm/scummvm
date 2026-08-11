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

#include "tetraedge/game/character_settings_xml_parser.h"

namespace Tetraedge {

bool CharacterSettingsXmlParser::parserCallback_ModelsSettings(ParserNode *node) {
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_Model(ParserNode *node) {
	const Character::CharacterSettings emptySettings;
	const Common::String &name = node->values["name"];
	_characterSettings->setVal(name, emptySettings);
	_curCharacter = &_characterSettings->getVal(name);
	_curCharacter->_name = name;
	assert(_characterSettings != nullptr);
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_modelFileName(ParserNode *node) {
	_curTextTag = TagModelFileName;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_defaultScale(ParserNode *node) {
	_curTextTag = TagDefaultScale;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_walk(ParserNode *node) {
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_animationFileName(ParserNode *node) {
	_curTextTag = TagAnimationFileName;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_walkType(ParserNode *node) {
	Common::String walkName = node->values["name"];
	_curWalkSettings = &(_curCharacter->_walkSettings[walkName]);
	return true;
}

Character::AnimSettings CharacterSettingsXmlParser::parseWalkAnimSettings(const ParserNode *node) const {
	Character::AnimSettings settings;
	const Common::StringMap &map = node->values;
	settings._file = map["file"];
	if (map.contains("stepRight"))
		settings._stepRight = map["stepRight"].asUint64();

	if (map.contains("stepLeft"))
		settings._stepLeft = map["stepLeft"].asUint64();

	return settings;
}

bool CharacterSettingsXmlParser::parserCallback_start(ParserNode *node) {
	_curWalkSettings->_walkParts[0] = parseWalkAnimSettings(node);
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_loop(ParserNode *node) {
	_curWalkSettings->_walkParts[1] = parseWalkAnimSettings(node);
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_endD(ParserNode *node) {
	_curWalkSettings->_walkParts[2] = parseWalkAnimSettings(node);
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_endG(ParserNode *node) {
	_curWalkSettings->_walkParts[3] = parseWalkAnimSettings(node);
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_speed(ParserNode *node) {
	_curTextTag = TagSpeed;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_cutSceneCurveDemi(ParserNode *node) {
	// Handled in the "position" callback.
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_position(ParserNode *node) {
	_curTextTag = TagPosition;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_rippleTexture(ParserNode *node) {
	// Ignored
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_face(ParserNode *node) {
	// Handled in "face" and "eyes" callbacks.
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_eyes(ParserNode *node) {
	_curTextTag = TagEyes;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_mouth(ParserNode *node) {
	_curTextTag = TagMouth;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_body(ParserNode *node) {
	if (node->values["name"] != "default")
		error("CharacterSettingsXmlParser: Only default body supported.");
	_curTextTag = TagBody;
	return true;
}

bool CharacterSettingsXmlParser::parserCallback_invertNormals(ParserNode *node) {
	_curCharacter->_invertNormals = true;
	return true;
}

bool CharacterSettingsXmlParser::textCallback(const Common::String &val) {
	switch (_curTextTag) {
	case TagModelFileName:
		_curCharacter->_modelFileName = val;
		break;
	case TagDefaultScale:
		_curCharacter->_defaultScale.parse(val);
		break;
	case TagAnimationFileName:
		_curCharacter->_idleAnimFileName = val;
		break;
	case TagEyes:
		_curCharacter->_defaultEyes = val;
		break;
	case TagMouth:
		_curCharacter->_defaultMouth = val;
		break;
	case TagSpeed:
		_curCharacter->_walkSpeed = atof(val.c_str());
		break;
	case TagPosition:
		_curCharacter->_cutSceneCurveDemiPosition.parse(val);
		break;
	case TagBody:
		_curCharacter->_defaultBody = val;
		break;
	default:
		break;
	}
	return true;
}

bool CharacterSettingsXmlParser::handleUnknownKey(ParserNode *node) {
	if (node->values.contains("animFile")) {
		// The game actually does nothing with these, they seem to be
		// for debugging purposes only.
		//const Common::String &animFile = node->values["animFile"];
		//debug("TODO: CharacterSettingsXmlParser handle mapping %s -> %s",
		//	node->name.c_str(), animFile.c_str());
		return true;
	}
	parserError("Unknown key");
	return false;
}

} // end namespace Tetraedge
