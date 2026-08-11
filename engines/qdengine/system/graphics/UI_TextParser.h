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

#ifndef QDENGINE_SYSTEM_GRAPHICS_UI_TEXT_PARSER_H
#define QDENGINE_SYSTEM_GRAPHICS_UI_TEXT_PARSER_H


#include "qdengine/system/graphics/gr_font.h"


namespace QDEngine {

struct OutNode {
	enum {
		NEW_LINE,
		TEXT,
		COLOR
	} type;
	int width;
	union {
		struct {
			const char *begin;
			const char *end;
		} nl;
		struct {
			int style;
		} t;
		int color;
	};
	OutNode() : type(NEW_LINE), width(0) { nl.begin = 0; nl.end = 0; }
	OutNode(const char *b, const char *e, int wd) : type(TEXT), width(wd) { nl.begin = b; nl.end = e; }
	OutNode(int clr) : type(COLOR), width(0), color(clr) {}
};

typedef Std::vector<OutNode> OutNodes;

class UI_TextParser {
public:
	UI_TextParser(const grFont *font = 0);
	UI_TextParser(const UI_TextParser &src);

	void operator= (const UI_TextParser &src);

	void setFont(const grFont *font);

	void parseString(const char *text, int color = 0, int fitIn = -1);

	const OutNodes &outNodes() const {
		return _outNodes;
	}

	int fontHeight() const {
		return _font ? _font->size_y() : 1;
	}
	const Vect2i &size() const {
		return _size;
	}

	int lineCount() const {
		return _lineCount;
	}
	OutNodes::const_iterator getLineBegin(int lineNum) const;

private:
	void init();

	inline int fromHex(char a) {
		if (a >= '0' && a <= '9')
			return a - '0';
		if (a >= 'A' && a <= 'F')
			return a - 'A' + 10;
		if (a >= 'a' && a <= 'f')
			return a - 'a' + 10;
		return -1;
	}

	inline void addChar(byte cc) {
		int width = _font->char_width(cc);
		if (testWidth(width) || cc != ' ')
			_tagWidth += width;
		++_pstr;
	}

	inline void skipNode() {
		_lineBegin = _pstr;
		_lastSpace = _lineBegin;
		_lastTagWidth = 0;
		_tagWidth = 0;
	}

	inline void putNode(OutNode &node) {
		_outNodes.push_back(node);
		skipNode();
	}

	void putText() {
		if (_pstr == _lineBegin)
			return;
		_lineWidth += _tagWidth;
		OutNode node(_lineBegin, _pstr, _tagWidth);
		putNode(node);
	}

	void endLine() {
		_size.x = MAX(_size.x, _lineWidth);

		_outNodes[_prevLineIndex].width = _lineWidth;
		_lineWidth = 0;

		_outNodes.push_back(OutNode());
		_prevLineIndex = _outNodes.size() - 1;

		++_lineCount;
	}

	void getColor(int defColor);
	int getStyle(const char *styleptr, const char *end);
	int getToken();
	bool testWidth(int width);

	OutNodes _outNodes;

	int _prevLineIndex;
	const char *_lastSpace;
	int _lastTagWidth;

	const char *_lineBegin;
	const char *_pstr;
	int _tagWidth;
	int _lineWidth;

	int _fitIn;

	Vect2i _size;
	int _lineCount;

	const grFont *_font;

};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_UI_TEXT_PARSER_H
