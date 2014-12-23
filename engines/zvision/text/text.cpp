/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/debug.h"
#include "common/rect.h"
#include "graphics/fontman.h"
#include "graphics/colormasks.h"
#include "graphics/surface.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

#include "zvision/text/text.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/text/truetype_font.h"
#include "zvision/scripting/script_manager.h"

namespace ZVision {

cTxtStyle::cTxtStyle() {
	_fontname = "Arial";
	_blue = 255;
	_green = 255;
	_red = 255;
	_bold = false;
#if 0
	_newline = false;
	_escapement = 0;
#endif
	_italic = false;
	_justify = TXT_JUSTIFY_LEFT;
	_size = 12;
	_skipcolor = false;
	_strikeout = false;
	_underline = false;
	_statebox = 0;
	_sharp = false;
}

txtReturn cTxtStyle::parseStyle(const Common::String &strin, int16 ln) {
	Common::String buf = Common::String(strin.c_str(), ln);

	int8 retval = TXT_RET_NOTHING;

	Common::StringTokenizer tokenizer(buf, " ");
	Common::String token;

	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();

		if (token.matchString("font", true)) {
			token = tokenizer.nextToken();
			if (token[0] == '"') {
				Common::String _tmp = Common::String(token.c_str() + 1);

				while (token.lastChar() != '"' && !tokenizer.empty()) {
					token = tokenizer.nextToken();
					_tmp += " " + token;
				}

				if (_tmp.lastChar() == '"')
					_tmp.deleteLastChar();

				_fontname = _tmp;
			} else {
				if (!tokenizer.empty())
					_fontname = token;
			}
			retval |= TXT_RET_FNTCHG;

		} else if (token.matchString("blue", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_blue != tmp) {
					_blue = tmp;
					retval |= TXT_RET_FNTSTL;
				}
			}
		} else if (token.matchString("red", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_red != tmp) {
					_red = tmp;
					retval |= TXT_RET_FNTSTL;
				}
			}
		} else if (token.matchString("green", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_green != tmp) {
					_green = tmp;
					retval |= TXT_RET_FNTSTL;
				}
			}
		} else if (token.matchString("newline", true)) {
#if 0
			if ((retval & TXT_RET_NEWLN) == 0)
				_newline = 0;

			_newline++;
#endif
			retval |= TXT_RET_NEWLN;
		} else if (token.matchString("point", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_size != tmp) {
					_size = tmp;
					retval |= TXT_RET_FNTCHG;
				}
			}
		} else if (token.matchString("escapement", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
#if 0
				int32 tmp = atoi(token.c_str());
				_escapement = tmp;
#endif
			}
		} else if (token.matchString("italic", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_italic != true) {
						_italic = true;
						retval |= TXT_RET_FNTSTL;
					}
				} else if (token.matchString("off", true)) {
					if (_italic != false) {
						_italic = false;
						retval |= TXT_RET_FNTSTL;
					}
				}
			}
		} else if (token.matchString("underline", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_underline != true) {
						_underline = true;
						retval |= TXT_RET_FNTSTL;
					}
				} else if (token.matchString("off", true)) {
					if (_underline != false) {
						_underline = false;
						retval |= TXT_RET_FNTSTL;
					}
				}
			}
		} else if (token.matchString("strikeout", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_strikeout != true) {
						_strikeout = true;
						retval |= TXT_RET_FNTSTL;
					}
				} else if (token.matchString("off", true)) {
					if (_strikeout != false) {
						_strikeout = false;
						retval |= TXT_RET_FNTSTL;
					}
				}
			}
		} else if (token.matchString("bold", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_bold != true) {
						_bold = true;
						retval |= TXT_RET_FNTSTL;
					}
				} else if (token.matchString("off", true)) {
					if (_bold != false) {
						_bold = false;
						retval |= TXT_RET_FNTSTL;
					}
				}
			}
		} else if (token.matchString("skipcolor", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					_skipcolor = true;
				} else if (token.matchString("off", true)) {
					_skipcolor = false;
				}
			}
		} else if (token.matchString("image", true)) {
			// Not used
		} else if (token.matchString("statebox", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				_statebox = atoi(token.c_str());
				retval |= TXT_RET_HASSTBOX;
			}
		} else if (token.matchString("justify", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("center", true))
					_justify = TXT_JUSTIFY_CENTER;
				else if (token.matchString("left", true))
					_justify = TXT_JUSTIFY_LEFT;
				else if (token.matchString("right", true))
					_justify = TXT_JUSTIFY_RIGHT;
			}
		}
	}
	return (txtReturn)retval;
}

void cTxtStyle::readAllStyle(const Common::String &txt) {
	int16 startTextPosition = -1;
	int16 endTextPosition = -1;

	for (uint16 i = 0; i < txt.size(); i++) {
		if (txt[i] == '<')
			startTextPosition = i;
		else if (txt[i] == '>') {
			endTextPosition = i;
			if (startTextPosition != -1)
				if ((endTextPosition - startTextPosition - 1) > 0)
					parseStyle(Common::String(txt.c_str() + startTextPosition + 1), endTextPosition - startTextPosition - 1);
		}

	}
}

void cTxtStyle::setFontStyle(StyledTTFont &font) {
	uint tempStyle = 0;

	if (_bold)
		tempStyle |= StyledTTFont::STTF_BOLD;

	if (_italic)
		tempStyle |= StyledTTFont::STTF_ITALIC;

	if (_underline)
		tempStyle |= StyledTTFont::STTF_UNDERLINE;

	if (_strikeout)
		tempStyle |= StyledTTFont::STTF_STRIKEOUT;

	if (_sharp)
		tempStyle |= StyledTTFont::STTF_SHARP;

	font.setStyle(tempStyle);
}

void cTxtStyle::setFont(StyledTTFont &font) {
	uint tempStyle = 0;

	if (_bold)
		tempStyle |= StyledTTFont::STTF_BOLD;

	if (_italic)
		tempStyle |= StyledTTFont::STTF_ITALIC;

	if (_underline)
		tempStyle |= StyledTTFont::STTF_UNDERLINE;

	if (_strikeout)
		tempStyle |= StyledTTFont::STTF_STRIKEOUT;

	if (_sharp)
		tempStyle |= StyledTTFont::STTF_SHARP;

	font.loadFont(_fontname, _size, tempStyle);
}

Graphics::Surface *TextRenderer::render(StyledTTFont &fnt, const Common::String &txt, cTxtStyle &style) {
	style.setFontStyle(fnt);
	uint32 clr = _engine->_pixelFormat.RGBToColor(style._red, style._green, style._blue);
	return fnt.renderSolidText(txt, clr);
}

void TextRenderer::drawTxtWithJustify(const Common::String &txt, StyledTTFont &fnt, uint32 color, Graphics::Surface &dst, int lineY, txtJustify justify) {
	if (justify == TXT_JUSTIFY_LEFT)
		fnt.drawString(&dst, txt, 0, lineY, dst.w, color, Graphics::kTextAlignLeft);
	else if (justify == TXT_JUSTIFY_CENTER)
		fnt.drawString(&dst, txt, 0, lineY, dst.w, color, Graphics::kTextAlignCenter);
	else if (justify == TXT_JUSTIFY_RIGHT)
		fnt.drawString(&dst, txt, 0, lineY, dst.w, color, Graphics::kTextAlignRight);
}

int32 TextRenderer::drawTxt(const Common::String &txt, cTxtStyle &fontStyle, Graphics::Surface &dst) {
	StyledTTFont font(_engine);
	fontStyle.setFont(font);

	dst.fillRect(Common::Rect(dst.w, dst.h), 0);

	uint32 clr = _engine->_pixelFormat.RGBToColor(fontStyle._red, fontStyle._green, fontStyle._blue);

	int16 w;

	w = font.getStringWidth(txt);

	drawTxtWithJustify(txt, font, clr, dst, 0, fontStyle._justify);

	return w;
}

void TextRenderer::drawTxtInOneLine(const Common::String &text, Graphics::Surface &dst) {
	const int16 TXT_CFG_TEXTURES_LINES = 256; // For now I don't want remake it
	const int TXT_CFG_TEXTURES_PER_LINE = 6;
	cTxtStyle style, style2;
	int16 startTextPosition = -1;
	int16 endTextPosition = -1;
	int16 i = 0;
	int16 dx = 0, dy = 0;
	int16 textPixelWidth;
	int16 textPosition = 0;
	Common::String buf;
	Common::String buf2;

	Graphics::Surface *TxtSurfaces[TXT_CFG_TEXTURES_LINES][TXT_CFG_TEXTURES_PER_LINE];
	int16 currentline = 0, currentlineitm = 0;

	int TxtJustify[TXT_CFG_TEXTURES_LINES];
	int TxtPoint[TXT_CFG_TEXTURES_LINES];

	for (int16 k = 0; k < TXT_CFG_TEXTURES_LINES; k++) {
		TxtPoint[k] = 0;
		for (int j = 0; j < TXT_CFG_TEXTURES_PER_LINE; j++)
			TxtSurfaces[k][j] = NULL;
	}

	int16 stringlen = text.size();

	StyledTTFont font(_engine);

	style.setFont(font);

	int16 prevbufspace = 0, prevtxtspace = 0;

	while (i < stringlen) {
		TxtJustify[currentline] = style._justify;
		if (text[i] == '<') {
			int16 ret = 0;

			startTextPosition = i;
			while (i < stringlen && text[i] != '>')
				i++;
			endTextPosition = i;
			if (startTextPosition != -1)
				if ((endTextPosition - startTextPosition - 1) > 0) {
					style2 = style;
					ret = style.parseStyle(Common::String(text.c_str() + startTextPosition + 1), endTextPosition - startTextPosition - 1);
				}

			if (ret & (TXT_RET_FNTCHG | TXT_RET_FNTSTL | TXT_RET_NEWLN)) {
				if (buf.size() > 0) {
					textPixelWidth = font.getStringWidth(buf);

					TxtSurfaces[currentline][currentlineitm] = render(font, buf, style2);
					TxtPoint[currentline] = MAX(font.getFontHeight(), TxtPoint[currentline]);

					currentlineitm++;

					buf.clear();
					prevbufspace = 0;
					textPosition = 0;
					dx += textPixelWidth;

				}
				if (ret & TXT_RET_FNTCHG) {
					style.setFont(font);
				}
				if (ret & TXT_RET_FNTSTL)
					style.setFontStyle(font);

				if (ret & TXT_RET_NEWLN) {
					currentline++;
					currentlineitm = 0;
					dx = 0;
				}
			}

			if (ret & TXT_RET_HASSTBOX) {
				Common::String buf3;
				buf3 = Common::String::format("%d", _engine->getScriptManager()->getStateValue(style._statebox));
				buf += buf3;
				textPosition += buf3.size();
			}

		} else {

			buf += text[i];
			textPosition++;

			if (text[i] == ' ') {
				prevbufspace = textPosition - 1;
				prevtxtspace = i;
			}

			if (font.isLoaded()) {
				textPixelWidth = font.getStringWidth(buf);
				if (textPixelWidth + dx > dst.w) {
					if (prevbufspace == 0) {
						prevtxtspace = i;
						prevbufspace = textPosition - 1;
					}
					buf2 = Common::String(buf.c_str(), prevbufspace + 1);

					if (buf2.size() > 0) {
						TxtSurfaces[currentline][currentlineitm] = render(font, buf2, style);
						TxtPoint[currentline] = MAX(font.getFontHeight(), TxtPoint[currentline]);
					}

					buf.clear();
					i = prevtxtspace;
					prevbufspace = 0;
					textPosition = 0;
					currentline++;
					currentlineitm = 0;
					dx = 0;
				}
			}
		}
		i++;
	}

	if (buf.size() > 0) {
		TxtSurfaces[currentline][currentlineitm] = render(font, buf, style);
		TxtPoint[currentline] = MAX(font.getFontHeight(), TxtPoint[currentline]);
	}

	dy = 0;
	for (i = 0; i <= currentline; i++) {
		int16 j = 0;
		int16 width = 0;
		while (TxtSurfaces[i][j] != NULL) {
			width += TxtSurfaces[i][j]->w;
			j++;
		}
		dx = 0;
		for (int32 jj = 0; jj < j; jj++) {
			if (TxtJustify[i] == TXT_JUSTIFY_LEFT)
				_engine->getRenderManager()->blitSurfaceToSurface(*TxtSurfaces[i][jj], dst, dx, dy + TxtPoint[i] - TxtSurfaces[i][jj]->h, 0);

			else if (TxtJustify[i] == TXT_JUSTIFY_CENTER)
				_engine->getRenderManager()->blitSurfaceToSurface(*TxtSurfaces[i][jj], dst, ((dst.w - width) / 2) + dx,  dy + TxtPoint[i] - TxtSurfaces[i][jj]->h, 0);

			else if (TxtJustify[i] == TXT_JUSTIFY_RIGHT)
				_engine->getRenderManager()->blitSurfaceToSurface(*TxtSurfaces[i][jj], dst, dst.w - width + dx, dy + TxtPoint[i] - TxtSurfaces[i][jj]->h, 0);

			dx += TxtSurfaces[i][jj]->w;
		}

		dy += TxtPoint[i];
	}

	for (i = 0; i < TXT_CFG_TEXTURES_LINES; i++)
		for (int32 j = 0; j < TXT_CFG_TEXTURES_PER_LINE; j++)
			if (TxtSurfaces[i][j] != NULL) {
				TxtSurfaces[i][j]->free();
				delete TxtSurfaces[i][j];
			}
}

Common::String readWideLine(Common::SeekableReadStream &stream) {
	Common::String asciiString;

	while (!stream.eos()) {
		uint32 value = stream.readUint16LE();
		// Check for CRLF
		if (value == 0x0A0D) {
			// Read in the extra NULL char
			stream.readByte(); // \0
			// End of the line. Break
			break;
		}

		// Crush each octet pair to a UTF-8 sequence
		if (value < 0x80) {
			asciiString += (char)(value & 0x7F);
		} else if (value >= 0x80 && value < 0x800) {
			asciiString += (char)(0xC0 | ((value >> 6) & 0x1F));
			asciiString += (char)(0x80 | (value & 0x3F));
		} else if (value >= 0x800 && value < 0x10000 && value != 0xCCCC) {
			asciiString += (char)(0xE0 | ((value >> 12) & 0xF));
			asciiString += (char)(0x80 | ((value >> 6) & 0x3F));
			asciiString += (char)(0x80 | (value & 0x3F));
		} else if (value == 0xCCCC) {
			// Ignore, this character is used as newline sometimes
		} else if (value >= 0x10000 && value < 0x200000) {
			asciiString += (char)(0xF0);
			asciiString += (char)(0x80 | ((value >> 12) & 0x3F));
			asciiString += (char)(0x80 | ((value >> 6) & 0x3F));
			asciiString += (char)(0x80 | (value & 0x3F));
		}
	}

	return asciiString;
}

int8 getUtf8CharSize(char chr) {
	if ((chr & 0x80) == 0)
		return 1;
	else if ((chr & 0xE0) == 0xC0)
		return 2;
	else if ((chr & 0xF0) == 0xE0)
		return 3;
	else if ((chr & 0xF8) == 0xF0)
		return 4;
	else if ((chr & 0xFC) == 0xF8)
		return 5;
	else if ((chr & 0xFE) == 0xFC)
		return 6;

	return 1;
}

uint16 readUtf8Char(const char *chr) {
	uint16 result = 0;
	if ((chr[0] & 0x80) == 0)
		result = chr[0];
	else if ((chr[0] & 0xE0) == 0xC0)
		result = ((chr[0] & 0x1F) << 6) | (chr[1] & 0x3F);
	else if ((chr[0] & 0xF0) == 0xE0)
		result = ((chr[0] & 0x0F) << 12) | ((chr[1] & 0x3F) << 6) | (chr[2] & 0x3F);
	else
		result = chr[0];

	return result;
}

} // End of namespace ZVision
