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

#include "tetraedge/te/te_xml_parser.h"

namespace Tetraedge {

TeVector3f32 TeXmlParser::parsePoint(const ParserNode *node) const {
	float x = atof(node->values["x"].c_str());
	float y = atof(node->values["y"].c_str());
	float z = atof(node->values["z"].c_str());
	return TeVector3f32(x, y, z);
}

bool TeXmlParser::parseCol(const ParserNode *node, TeColor &colout) {
	uint r = node->values["r"].asUint64();
	uint g = node->values["g"].asUint64();
	uint b = node->values["b"].asUint64();
	uint a;
	if (node->values.contains("a"))
		a = node->values["a"].asUint64();
	else
		a = 0xff;

	if (r > 255 || g > 255 || b > 255 || a > 255) {
		parserError("Invalid color values");
		return false;
	}
	colout = TeColor(r, g, b, a);
	return true;
}

double TeXmlParser::parseDouble(const ParserNode *node, const char *attr) const {
	if (!attr)
		attr = "value";
	return atof(node->values[attr].c_str());
}

int TeXmlParser::parseUint(const ParserNode *node, const char *attr) const {
	if (!attr)
		attr = "value";
	return node->values[attr].asUint64();
}

} // end namespace Tetraedge
