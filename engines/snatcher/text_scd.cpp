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


#include "snatcher/animator.h"
#include "snatcher/text.h"

namespace Snatcher {

class TextRenderer_SCD : public TextRenderer {
public:
	TextRenderer_SCD(Animator *animator);
	~TextRenderer_SCD() override;

	void setFont(const uint8 *font, uint32 fontSize) override;
	void setCharWidthTable(const uint8 *table, uint32 tableSize) override;

	void enqueuePrintJob(const uint8 *text) override;
	void setPrintDelay(int delay) override;
	//void setColor(uint8 color) override;

	void draw() override;
	void reset() override { _needDraw = false; }
	bool needsPrint() const override { return _needDraw; }

private:
	void drawGlyph(uint16 ch);

	bool _needDraw;
	bool _block;
	uint16 _curX;
	uint16 _curY;
	bool _fixedWith9;
	uint16 _printDelay;
	uint16 _printDelayCounter;
	uint8 _color;
	uint8 _pscr_byt1;
	uint8 _pscr_byt2;
	uint8 _pscr_byt3;
	uint8 _pscr_byt8;
	int16 _pscr_wd;
	const uint8 *_textPtr;
	uint8 *_renderBuffer;
	Animator *_animator;
	const uint8 *_font;
	uint32 _fontSize;
	const uint8 *_charWidthTable;
	uint32 _charWidthTableSize;
};

TextRenderer_SCD::TextRenderer_SCD(Animator *animator) : TextRenderer(), _animator(animator), _textPtr(nullptr), _renderBuffer(nullptr), _needDraw(false), _block(false), _printDelay(0), _printDelayCounter(0), _color(0),
	_curX(0), _curY(0), _fixedWith9(false), _font(nullptr), _fontSize(0), _charWidthTable(nullptr), _charWidthTableSize(0), _pscr_byt8(0), _pscr_wd(0), _pscr_byt1(0), _pscr_byt2(0), _pscr_byt3(0) {
	_renderBuffer = new uint8[0x2000]();
	assert(_renderBuffer);
}

TextRenderer_SCD::~TextRenderer_SCD() {
	delete[] _renderBuffer;
}

void TextRenderer_SCD::setFont(const uint8 *font, uint32 fontSize) {
	_font = font;
	_fontSize = fontSize;
}

void TextRenderer_SCD::setCharWidthTable(const uint8 *table, uint32 tableSize) {
	_charWidthTable = table;
	_charWidthTableSize = tableSize;
}

void TextRenderer_SCD::enqueuePrintJob(const uint8 *text) {
	_textPtr = text;
	_needDraw = true;
	_printDelayCounter = _printDelay = 0;
}

void TextRenderer_SCD::setPrintDelay(int delay) {
	_printDelay = delay;
}
/*
void TextRenderer_SCD::setColor(uint8 color) {
	_color = color;
}
*/
void TextRenderer_SCD::draw() {
	if (!_needDraw || _block)
		return;

	memset(_renderBuffer, 0, 0x2000);

	int firstRow = -1;
	int numRows = 0;
	const uint8 *s = _textPtr;

	for (bool lp1 = true; lp1; ) {
		if (_printDelayCounter) {
			_printDelayCounter--;
			lp1 = false;
		} else {
			for (bool lp2 = true; lp2; ) {
				_printDelayCounter = _printDelay;
				uint8 in = *s++;
				switch (in) {
				case 0xFF:
					_needDraw = lp1 = lp2 = false;
					break;
				case 0xFE:
					_color = *s++;
					break;
				case 0xFD:
					_curY += 9;
					_curX = 24;
					_pscr_byt8 += (*s++ << 2);
					break;
				case 0xFC:
					_printDelayCounter = _printDelay = *s++;
					lp2 = false;
					break;
				case 0xFB:
					_curX = *s++;
					_curY = *s++;
					break;
				case 0xFA:
					_pscr_byt1 = *s++;
					_pscr_byt2 = *s++;
					_block = true;
					lp1 = lp2 = false;
					break;
				case 0xF9:
					_pscr_byt3 = *s++;
					break;
				case 0xF8:
					_pscr_wd = (*s++ << 8) | (*s++);
					break;
				case 0xF7:
					_fixedWith9 = *s++ ? true : false;
					break;
				default:
					if (in < 32) {
						_pscr_byt8 += (in << 2);
					} else {
						in -= 32;

						firstRow = (firstRow == -1) ? (_curY >> 3) : MIN<int16>(firstRow, _curY >> 3);
						numRows = MAX<int16>(numRows, ((_curY + 7) >> 3) - firstRow + 1);

						drawGlyph(in);

						if (_fixedWith9) {
							_curX += 9;
						} else {
							if (in < _charWidthTableSize)
								_curX += _charWidthTable[in];
							else
								error("%s(): Invalid char %04X", __FUNCTION__, in);
						}
						lp2 = false;
					}
					break;
				}
			}
		}
	}
	_textPtr = s;
	if (numRows)
		_animator->renderTextBuffer(_renderBuffer, firstRow, numRows);
}

void TextRenderer_SCD::drawGlyph(uint16 ch) {
	uint16 x = _curX;
	if (_fixedWith9 && (ch == 17 || ch == 41))
		++x;

	const uint8 *src = _font + (ch << 3);
	uint8 *dst = _renderBuffer + ((_curY & ~7) << 7) + ((_curY & 7) << 2) + ((x & ~7) << 2) + ((x & 7) >> 1);
	assert(dst < _renderBuffer + 0x2000);

	if ((uint)(ch << 3) >= _fontSize)
		error("%s(): Invalid char %04X", __FUNCTION__, ch);

	uint8 dY = _curY & 7;
	uint8 dX = x & 7;
	uint8 c1 = _color & 0x0F;
	uint8 c2 = _color << 4;

	for (int h = 0; h < 8; ++h) {
		uint8 *d = dst;
		uint8 dXX = dX;
		uint8 in = *src++;

		for (int w = 7; w >= 0; --w) {
			if (in & (1 << w)) {
				if (dXX & 1)
					*d = (*d & 0xF0) | c1;
				else
					*d = (*d & 0x0F) | c2;
			}
			if (dXX & 1)
				++d;
			if (++dXX == 8)
				d += 28;
			assert(d < _renderBuffer + 0x2000);
		}

		dst += 4;
		if (++dY == 8) {
			dY = 0;
			dst += 992;
		}
		assert(dst < _renderBuffer + 0x2000);
	}
}

TextRenderer *TextRenderer::createSegaTextRenderer(Animator *animator) {
	return new TextRenderer_SCD(animator);
}

} // End of namespace Snatcher
