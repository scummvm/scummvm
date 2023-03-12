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

#ifndef TETRAEDGE_GAME_OBJECT_SETTINGS_XML_PARSER_H
#define TETRAEDGE_GAME_OBJECT_SETTINGS_XML_PARSER_H

#include "common/formats/xmlparser.h"
#include "tetraedge/game/object3d.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class ObjectSettingsXmlParser : public Common::XMLParser {
public:
	ObjectSettingsXmlParser(Common::HashMap<Common::String, Object3D::ObjectSettings> *settings) :
		Common::XMLParser(), _textTagType(TagNone), _objectSettings(settings) {}

	void finalize();

	// Parser
	CUSTOM_XML_PARSER(ObjectSettingsXmlParser) {
		XML_KEY(ObjectsSettings)
			XML_KEY(Object)
				XML_PROP(name, true)
				XML_KEY(modelFileName)
				KEY_END()
				XML_KEY(defaultScale)
				KEY_END()
				XML_KEY(originOffset)
				KEY_END()
				XML_KEY(invertNormals)
				KEY_END()
			KEY_END()
		KEY_END()
	} PARSER_END()

private:
	// Parser callback methods
	bool parserCallback_ObjectsSettings(ParserNode *node);
	bool parserCallback_Object(ParserNode *node);
	bool parserCallback_modelFileName(ParserNode *node);
	bool parserCallback_defaultScale(ParserNode *node);
	bool parserCallback_originOffset(ParserNode *node);
	bool parserCallback_invertNormals(ParserNode *node);
	bool textCallback(const Common::String &val) override;

	enum TextTagType {
		TagNone,
		TagModelFileName,
		TagDefaultScale,
		TagOriginOffset
	};

	TextTagType _textTagType;
	Object3D::ObjectSettings _curObject;
	Common::HashMap<Common::String, Object3D::ObjectSettings> *_objectSettings;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_OBJECT_SETTINGS_XML_PARSER_H
