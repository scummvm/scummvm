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

#include "common/hashmap.h"
#include "common/str.h"
#include "common/formats/xmlparser.h"
#include "tetraedge/game/inventory.h"

#ifndef TETRAEDGE_GAME_INVENTORY_OBJECTS_XML_PARSER_H
#define TETRAEDGE_GAME_INVENTORY_OBJECTS_XML_PARSER_H

namespace Tetraedge {

class InventoryObjectsXmlParser : public Common::XMLParser {
public:
	// Parser
	CUSTOM_XML_PARSER(InventoryObjectsXmlParser) {
		XML_KEY(document)
			XML_KEY(Object)
				XML_PROP(id, true)
				XML_PROP(name, true)
				XML_PROP(isDocument, false)
				XML_PROP(description, false)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_document(ParserNode *node) { return true; };
	bool parserCallback_Object(ParserNode *node);

public:
	Common::HashMap<Common::String, Inventory::InventoryObjectData> _objects;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_INVENTORY_OBJECTS_XML_PARSER_H
