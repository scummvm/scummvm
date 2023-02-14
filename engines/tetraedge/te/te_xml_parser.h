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

#ifndef TETRAEDGE_TE_TE_XML_PARSER_H
#define TETRAEDGE_TE_TE_XML_PARSER_H

#include "common/hashmap.h"
#include "common/str.h"
#include "common/formats/xmlparser.h"

#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_color.h"

namespace Tetraedge {

/**
 * A small extension to the common XML parser to
 * add some convenience methods.
 **/
class TeXmlParser : public Common::XMLParser {
protected:
	/// Parse a point with x/y/z attributes
	TeVector3f32 parsePoint(const ParserNode *node) const;

	/// Parse a color with r/g/b and optionally a attributes
	/// ('a' defaults to 255 if not in the attributes).  Returns true on success.
	bool parseCol(const ParserNode *node, TeColor &colout);

	/// Parse a double value from an attribute.
	/// Default attribute name if left null is "value".
	double parseDouble(const ParserNode *node, const char *attr = nullptr) const;

	/// Parse an integer value from an attribute
	/// Default attribute name if left null is "value".
	int parseUint(const ParserNode *node, const char *attr = nullptr) const;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_XML_PARSER_H
