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

#include "common/util.h"

#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/UI_TextParser.h"


namespace QDEngine {

UI_TextParser::UI_TextParser(const grFont *font) : _font(font) {
	_outNodes.reserve(8);
	init();
}

UI_TextParser::UI_TextParser(const UI_TextParser &src) {
	(*this) = src;
}

void UI_TextParser::operator= (const UI_TextParser &src) {
	_font = src._font;

	_outNodes.reserve(8);
	init();
}

void UI_TextParser::init() {
	_tagWidth = 0;
	_lineWidth = 0;

	_lineBegin = 0;
	_pstr = 0;

	_fitIn = -1;
	_lastSpace = 0;
	_lastTagWidth = 0;

	_outNodes.clear();
	_outNodes.push_back(OutNode());
	_prevLineIndex = _outNodes.size() - 1;

	_size.set(0, fontHeight());

	_lineCount = 1;
}

void UI_TextParser::setFont(const grFont *font) {
	_font = font;
	init();
}

OutNodes::const_iterator UI_TextParser::getLineBegin(int lineNum) const {
	assert(lineNum >= 0);

	if (!lineNum)
		return _outNodes.begin();

	if (lineNum >= _lineCount)
		return _outNodes.end();

	for (auto it = _outNodes.begin(); it != _outNodes.end(); it++) {
		if (it->type == OutNode::NEW_LINE)
			if (lineNum-- == 0)
				return it;
	}

	assert(lineNum == 0);
	return _outNodes.end();
}

bool UI_TextParser::testWidth(int width) {
	if (_fitIn < 0)
		return true;

	if (_lineWidth + _tagWidth + width > _fitIn) {
		if (_lastSpace != _lineBegin) {
			_outNodes.push_back(OutNode(_lineBegin, _lastSpace, _lastTagWidth));

			_lineWidth += _lastTagWidth;
			endLine();

			_lineBegin = _lastSpace + 1;
			_lastSpace = _lineBegin;
			_tagWidth -= _lastTagWidth;
			_lastTagWidth = 0;
		} else if (_lineWidth > 0) {
			assert(_lastTagWidth == 0);
			endLine();
			testWidth(width);
		} else if (_tagWidth > 0) {
			putText();
			endLine();
			skipNode();
		}
		return false;
	}
	return true;
}

void UI_TextParser::parseString(const char *text, int color, int fitIn) {
	if (!_font)
		setFont(grDispatcher::get_default_font());

	assert(_font);
	init();

	_fitIn = fitIn > 2 * fontHeight() ? fitIn : -1;

	_pstr = text;

	_lineBegin = text;
	_lastSpace = _lineBegin;

	while (byte cc = *_pstr) {
		if (cc == '\n') {
			putText();
			++_pstr;

			endLine();
			skipNode();

			continue;
		}

		if (cc < 32) {
			++_pstr;
			continue;
		}

		if (cc == ' ') {
			_lastTagWidth = _tagWidth;
			_lastSpace = _pstr;
		}

		//if(useWildChars)
		if (cc == '&') {
			if (_pstr[1] != '&') {
				putText();
				++_pstr;
				getColor(color);
				continue;
			} else {
				addChar('&');
				putText();
				++_pstr;
				skipNode();
				continue;
			}
		} else if (cc == '<') {
			if (_pstr[1] != '<') {
				putText();
				++_pstr;
				_lineWidth += getToken();
				continue;
			} else {
				addChar('<');
				putText();
				++_pstr;
				skipNode();
				continue;
			}
		}

		addChar((byte)cc);
	}

	putText();
	_size.x = MAX(_size.x, _lineWidth);
	_outNodes[_prevLineIndex].width = _lineWidth;

	_size.y = fontHeight() * _lineCount;
}

int UI_TextParser::getToken() {
	char cc;
	while ((cc = *_pstr) && cc != '=' && cc != '>')
		++_pstr;

	if (cc != '>') {
		while ((cc = *_pstr) && cc != ';' && cc != '>')
			++_pstr;
		if (cc == ';') {
			while ((cc = *_pstr) && cc != '>')
				++_pstr;
		}
	}

	if (!cc) {
		skipNode();
		return 0;
	}

	/*  switch(tag_len){
	    case 3:
	        if(!strncmp(begin_tag, "img=", 4)){
	            string img_name(begin_tag + 4, begin_style ? begin_style : _pstr);
	            if(const UI_Sprite* sprite = UI_SpriteReference(img_name.c_str()))
	                if(!sprite->isEmpty()){
	                    OutNode node;
	                    node.type = OutNode::SPRITE;
	                    node.sprite = sprite;
	                    node.style = getStyle(begin_style, _pstr);
	                    if((node.style & 0x03) != 2)
	                        node.width = sprite->size().xi();
	                    else{
	                        Vect2f size = sprite->size();
	                        node.width = round(size.x / size.y * fontHeight());
	                    }
	                    ++_pstr;
	                    testWidth(node.width);
	                    putNode(node);
	                    return node.width;
	                }
	        }
	        break;
	    }*/

	++_pstr;
	skipNode();
	return 0;
}

int UI_TextParser::getStyle(const char *styleptr, const char *end) {
	if (!styleptr || *end != '>')
		return 0;

	char cc;
	while ((cc = *(++styleptr)) && cc != '=' && cc != '>');

	if (cc != '=')
		return 0;

	int style = 0;
	while ((cc = *(++styleptr)) >= '0' && cc <= '9')
		style = style * 10 + (int)(cc - '0');

	return style;
}

void UI_TextParser::getColor(int defColor) {
	int color = defColor;

	if (*_pstr != '>') {
		int s = 0;
		int i = 0;
		for (; i < 6; ++i, ++_pstr)
			if (char k = *_pstr) {
				int a = fromHex(k);
				if (a < 0)
					break;
				s |= a << (i * 4);
			} else
				break;

		if (i > 5) {
			color &= 0xFF000000;
			color |= s;
		} else {
			skipNode();
			return;
		}
	} else
		++_pstr;

	OutNode node(color);
	putNode(node);
}

} // namespace QDEngine
