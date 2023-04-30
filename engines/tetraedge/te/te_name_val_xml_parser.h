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

#ifndef TETRAEDGE_TE_TE_NAME_VAL_XML_PARSER_H
#define TETRAEDGE_TE_TE_NAME_VAL_XML_PARSER_H

#include "common/formats/xmlparser.h"

namespace Tetraedge {

class TeNameValXmlParser : public Common::XMLParser {
public:
	// Parser
	CUSTOM_XML_PARSER(TeNameValXmlParser) {
		XML_KEY(value)
			XML_PROP(name, true)
			XML_PROP(value, true)
		KEY_END()
		XML_KEY(group)
			XML_PROP(name, true)
			XML_KEY(value)
				XML_PROP(name, true)
				XML_PROP(value, true)
			KEY_END()
		KEY_END()

	} PARSER_END()

	// Parser callback methods
	bool parserCallback_value(ParserNode *node);
	bool parserCallback_group(ParserNode *node);
	virtual bool closedKeyCallback(ParserNode *node) override;

public:
	const Common::StringMap &getMap() const { return _map; }

private:
	Common::StringMap _map;
	Common::String _curGroup;
	Common::Array<Common::String> _groupNames;
};
} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_NAME_VAL_XML_PARSER_H
