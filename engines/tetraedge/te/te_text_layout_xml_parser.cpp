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

#include "tetraedge/te/te_text_layout_xml_parser.h"

namespace Tetraedge {

bool TeTextLayoutXmlParser::parserCallback_section(ParserNode *node) {
	_style = node->values["style"];
	return true;
}

bool TeTextLayoutXmlParser::parserCallback_color(ParserNode *node) {
	return parseCol(node, _color);
}

bool TeTextLayoutXmlParser::parserCallback_font(ParserNode *node) {
	_fontFile = node->values["file"];
	if (node->values.contains("size"))
		_fontSize = node->values["size"].asUint64();
	else {
		warning("default font size to 16");
		_fontSize = 16;
	}
	return true;
}

bool TeTextLayoutXmlParser::textCallback(const Common::String &str) {
	_textContent += str;
	return true;
}

bool TeTextLayoutXmlParser::parserCallback_br(ParserNode *node) {
	_lineBreaks.push_back(_textContent.size());
	return true;
}

bool TeTextLayoutXmlParser::parserCallback_b(ParserNode *node) {
	//
	// WORKAROUND: There is a <b /> in the The Syberia credits text xml.
	// It's almost certainly a typo for <br />, bold fonts are not supported.
	//
	_lineBreaks.push_back(_textContent.size());
	return true;
}

} // end namespace Tetraedge
