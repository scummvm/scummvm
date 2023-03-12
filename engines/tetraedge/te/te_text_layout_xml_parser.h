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

#ifndef TETRAEDGE_TE_TE_TEXT_LAYOUT_XML_PARSER_H
#define TETRAEDGE_TE_TE_TEXT_LAYOUT_XML_PARSER_H

#include "common/array.h"
#include "common/str.h"

#include "tetraedge/te/te_xml_parser.h"
#include "tetraedge/te/te_color.h"

namespace Tetraedge {

class TeTextLayoutXmlParser : public TeXmlParser {
public:
	TeTextLayoutXmlParser() : TeXmlParser(), _fontSize(0) {}

	CUSTOM_XML_PARSER(TeTextLayoutXmlParser) {
		XML_KEY(document)
			XML_KEY(section)
				XML_PROP(style, true)
			KEY_END()
			XML_KEY(color)
				XML_PROP(r, true)
				XML_PROP(g, true)
				XML_PROP(b, true)
			KEY_END()
			XML_KEY(font)
				XML_PROP(file, true)
				XML_PROP(size, true)
			KEY_END()
			XML_KEY(br)
			KEY_END()
			XML_KEY(b)
			KEY_END()
		KEY_END()
	} PARSER_END()

private:
	// Parser callback methods
	bool parserCallback_document(ParserNode *node) { return true; };
	bool parserCallback_section(ParserNode *node);
	bool parserCallback_color(ParserNode *node);
	bool parserCallback_font(ParserNode *node);
	bool parserCallback_br(ParserNode *node);
	bool parserCallback_b(ParserNode *node);

	virtual bool textCallback(const Common::String &str) override;

public:
	const TeColor &color() const { return _color; }
	const Common::String &fontFile() const { return _fontFile; }
	int fontSize() const { return _fontSize; }
	const Common::String &style() const { return _style; }
	const Common::String &textContent() const { return _textContent; }
	const Common::Array<uint> &lineBreaks() const { return _lineBreaks; }

private:
	TeColor _color;
	Common::String _fontFile;
	int _fontSize;
	Common::String _style;
	Common::String _textContent;
	Common::Array<uint> _lineBreaks;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TEXT_LAYOUT_XML_PARSER_H
