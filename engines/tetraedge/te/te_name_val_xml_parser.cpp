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

#include "tetraedge/te/te_name_val_xml_parser.h"

namespace Tetraedge {

// Parser callback methods
bool TeNameValXmlParser::parserCallback_value(ParserNode *node) {
	Common::String valStr = node->values["value"];

	// Replace "&quot;" with " character.  This is the only character
	// entity used in the game files.
	uint qpos = valStr.find("&quot;");
	while (qpos != Common::String::npos) {
		valStr.replace(qpos, 6, "\"");
		qpos = valStr.find("&quot;");
	}

	Common::String name;
	if (_curGroup.empty())
		name = node->values["name"];
	else
		name = _curGroup + "." + node->values["name"];

	_map.setVal(name, valStr);
	return true;
}

bool TeNameValXmlParser::parserCallback_group(ParserNode *node) {
	_curGroup = node->values["name"];
	_groupNames.push_back(_curGroup);
	return true;
}

bool TeNameValXmlParser::closedKeyCallback(ParserNode *node) {
	if (node->name == "group")
		_curGroup.clear();
	return true;
}


} // end namespace Tetraedge
