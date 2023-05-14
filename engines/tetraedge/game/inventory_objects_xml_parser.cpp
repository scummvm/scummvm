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

#include "tetraedge/game/inventory_objects_xml_parser.h"

namespace Tetraedge {

bool InventoryObjectsXmlParser::parserCallback_Object(ParserNode *node) {
	Inventory::InventoryObjectData data;
	data._id = node->values["id"];
	data._name = node->values["name"];
	data._isDocument = node->values.contains("isDocument");
	_objects.setVal(data._id, data);
	return true;
}

bool InventoryObjectsXmlParser::handleUnknownKey(ParserNode *node) {
	if (node->name == "value") {
		warning("Garbage entry <value> in inventory file");
		return true;
	}
	parserError("Unknown key");
	return false;
}

} // end namespace Tetraedge
