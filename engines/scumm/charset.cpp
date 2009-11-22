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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */


#include "scumm/charset.h"
#include "scumm/scumm.h"
#include "scumm/nut_renderer.h"
#include "scumm/util.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

/*
TODO:
Right now our charset renderers directly access _textSurface, as well as the
virtual screens of ScummEngine. Ideally, this would not be the case. Instead,
ScummVM would simply pass the appropriate Surface to the resp. methods.
Of course it is not quite as simple, various flags and offsets have to
be taken into account for that.

The advantage will be cleaner coder (easier to debug, in particular), and a
better separation of the various modules.
*/


void ScummEngine::loadCJKFont() {
	Common::File fp;
	_useCJKMode = false;
	_textSurfaceMultiplier = 1;
	_newLineCharacter = 0;

	if (_game.version <= 5 && _game.platform == Common::kPlatformFMTowns && _language == Common::JA_JPN) { // FM-TOWNS v3 / v5 Kanji
		int numChar = 256 * 32;
		_2byteWidth = 16;
		_2byteHeight = 16;
		// use FM-TOWNS font rom, since game files don't have kanji font resources
		if (!fp.open("fmt_fnt.rom")) {
			error("SCUMM::Font: Couldn't open fmt_fnt.rom");
		} else {
			_useCJKMode = true;
			debug(2, "Loading FM-TOWNS Kanji rom");
			_2byteFontPtr = new byte[((_2byteWidth + 7) / 8) * _2byteHeight * numChar];
			fp.read(_2byteFontPtr, ((_2byteWidth + 7) / 8) * _2byteHeight * numChar);
			fp.close();
		}
		_textSurfaceMultiplier = 2;
	} else if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && _language == Common::JA_JPN) {
		int numChar = 3418;
		_2byteWidth = 12;
		_2byteHeight = 12;
		// use PC-Engine System Card, since game files don't have kanji font resources
		if (!fp.open("pce.cdbios")) {
			error("SCUMM::Font: Couldn't open System Card pce.cdbios");
		} else {
			_useCJKMode = true;
			debug(2, "Loading PC-Engine System Card");
			fp.seek(0x30000);
			_2byteFontPtr = new byte[_2byteWidth * _2byteHeight * numChar / 8];
			fp.read(_2byteFontPtr, _2byteWidth * _2byteHeight * numChar / 8);
			fp.close();
		}
	} else if (_game.version >= 7 && (_language == Common::KO_KOR || _language == Common::JA_JPN || _language == Common::ZH_TWN)) {
		int numChar = 0;
		const char *fontFile = NULL;

		switch (_language) {
		case Common::KO_KOR:
			fontFile = "korean.fnt";
			numChar = 2350;
			break;
		case Common::JA_JPN:
			fontFile = (_game.id == GID_DIG) ? "kanji16.fnt" : "japanese.fnt";
			numChar = 8192;
			break;
		case Common::ZH_TWN:
			if (_game.id == GID_CMI) {
				fontFile = "chinese.fnt";
				numChar = 13630;
			}
			break;
		default:
			break;
		}
		if (fontFile && fp.open(fontFile)) {
			debug(2, "Loading CJK Font");
			_useCJKMode = true;
			_textSurfaceMultiplier = 1; // No multiplication here

			switch (_language) {
			case Common::KO_KOR:
				fp.seek(2, SEEK_CUR);
				_2byteWidth = fp.readByte();
				_2byteHeight = fp.readByte();
				_newLineCharacter = 0xff;
				break;
			case Common::JA_JPN:
				_2byteWidth = 16;
				_2byteHeight = 16;
				_newLineCharacter = 0xfe;
				break;
			case Common::ZH_TWN:
				_2byteWidth = 16;
				_2byteHeight = 15;
				_newLineCharacter = 0x21;
				break;
			default:
				break;
			}

			_2byteFontPtr = new byte[((_2byteWidth + 7) / 8) * _2byteHeight * numChar];
			fp.read(_2byteFontPtr, ((_2byteWidth + 7) / 8) * _2byteHeight * numChar);
			fp.close();
		} else {
			if (fontFile)
				error("SCUMM::Font: Couldn't open %s",fontFile);
			else
				error("SCUMM::Font: Couldn't load any font");
		}
	}
}

static int SJIStoFMTChunk(int f, int s) { //converts sjis code to fmt font offset
	enum {
		KANA = 0,
		KANJI = 1,
		EKANJI = 2
	};
	int base = s - ((s + 1) % 32);
	int c = 0, p = 0, chunk_f = 0, chunk = 0, cr = 0, kanjiType = KANA;

	if (f >= 0x81 && f <= 0x84) kanjiType = KANA;
	if (f >= 0x88 && f <= 0x9f) kanjiType = KANJI;
	if (f >= 0xe0 && f <= 0xea) kanjiType = EKANJI;

	if ((f > 0xe8 || (f == 0xe8 && base >= 0x9f)) || (f > 0x90 || (f == 0x90 && base >= 0x9f))) {
		c = 48; //correction
		p = -8; //correction
	}

	if (kanjiType == KANA) {//Kana
		chunk_f = (f - 0x81) * 2;
	} else if (kanjiType == KANJI) {//Standard Kanji
		p += f - 0x88;
		chunk_f = c + 2 * p;
	} else if (kanjiType == EKANJI) {//Enhanced Kanji
		p += f - 0xe0;
		chunk_f = c + 2 * p;
	}

	// Base corrections
	if (base == 0x7f && s == 0x7f)
		base -= 0x20;
	if (base == 0x9f && s == 0xbe)
		base += 0x20;
	if (base == 0xbf && s == 0xde)
		base += 0x20;
	//if (base == 0x7f && s == 0x9e)
	//	base += 0x20;

	switch (base) {
	case 0x3f:
		cr = 0; //3f
		if (kanjiType == KANA) chunk = 1;
		else if (kanjiType == KANJI) chunk = 31;
		else if (kanjiType == EKANJI) chunk = 111;
		break;
	case 0x5f:
		cr = 0; //5f
		if (kanjiType == KANA) chunk = 17;
		else if (kanjiType == KANJI) chunk = 47;
		else if (kanjiType == EKANJI) chunk = 127;
		break;
	case 0x7f:
		cr = -1; //80
		if (kanjiType == KANA) chunk = 9;
		else if (kanjiType == KANJI) chunk = 63;
		else if (kanjiType == EKANJI) chunk = 143;
		break;
	case 0x9f:
		cr = 1; //9e
		if (kanjiType == KANA) chunk = 2;
		else if (kanjiType == KANJI) chunk = 32;
		else if (kanjiType == EKANJI) chunk = 112;
		break;
	case 0xbf:
		cr = 1; //be
		if (kanjiType == KANA) chunk = 18;
		else if (kanjiType == KANJI) chunk = 48;
		else if (kanjiType == EKANJI) chunk = 128;
		break;
	case 0xdf:
		cr = 1; //de
		if (kanjiType == KANA) chunk = 10;
		else if (kanjiType == KANJI) chunk = 64;
		else if (kanjiType == EKANJI) chunk = 144;
		break;
	default:
		debug(4, "Invalid Char! f %x s %x base %x c %d p %d", f, s, base, c, p);
		return 0;
	}

	debug(6, "Kanji: %c%c f 0x%x s 0x%x base 0x%x c %d p %d chunk %d cr %d index %d", f, s, f, s, base, c, p, chunk, cr, ((chunk_f + chunk) * 32 + (s - base)) + cr);
	return ((chunk_f + chunk) * 32 + (s - base)) + cr;
}

static int SJIStoPCEChunk(int f, int s) { //converts sjis code to pce font offset
	// rangeTbl maps SJIS char-codes to the PCE System Card font rom.
	// Each pair {<upperBound>,<lowerBound>} in the array represents a SJIS range.
	const int rangeCnt = 45;
	static const uint16 rangeTbl[rangeCnt][2] = {
		// Symbols
		{0x8140,0x817E},{0x8180,0x81AC},
		// 0-9
		{0x824F,0x8258},
		// Latin upper
		{0x8260,0x8279},
		// Latin lower
		{0x8281,0x829A},
		// Kana
		{0x829F,0x82F1},{0x8340,0x837E},{0x8380,0x8396},
		// Greek upper
		{0x839F,0x83B6},
		// Greek lower
		{0x83BF,0x83D6},
		// Cyrillic upper
		{0x8440,0x8460},
		// Cyrillic lower
		{0x8470,0x847E},{0x8480,0x8491},
		// Kanji
		{0x889F,0x88FC},
		{0x8940,0x897E},{0x8980,0x89FC},
		{0x8A40,0x8A7E},{0x8A80,0x8AFC},
		{0x8B40,0x8B7E},{0x8B80,0x8BFC},
		{0x8C40,0x8C7E},{0x8C80,0x8CFC},
		{0x8D40,0x8D7E},{0x8D80,0x8DFC},
		{0x8E40,0x8E7E},{0x8E80,0x8EFC},
		{0x8F40,0x8F7E},{0x8F80,0x8FFC},
		{0x9040,0x907E},{0x9080,0x90FC},
		{0x9140,0x917E},{0x9180,0x91FC},
		{0x9240,0x927E},{0x9280,0x92FC},
		{0x9340,0x937E},{0x9380,0x93FC},
		{0x9440,0x947E},{0x9480,0x94FC},
		{0x9540,0x957E},{0x9580,0x95FC},
		{0x9640,0x967E},{0x9680,0x96FC},
		{0x9740,0x977E},{0x9780,0x97FC},
		{0x9840,0x9872}
	};

	int ch = (f << 8) | (s & 0xFF);
	int offset = 0;
	for (int i = 0; i < rangeCnt; ++i) {
		if (ch >= rangeTbl[i][0] && ch <= rangeTbl[i][1])
			return offset + ch - rangeTbl[i][0];
		offset += rangeTbl[i][1] - rangeTbl[i][0] + 1;
	}

	debug(4, "Invalid Char: 0x%x", ch);
	return 0;
}

byte *ScummEngine::get2byteCharPtr(int idx) {
	switch (_language) {
	case Common::KO_KOR:
		idx = ((idx % 256) - 0xb0) * 94 + (idx / 256) - 0xa1;
		break;
	case Common::JA_JPN:
		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
			idx = SJIStoPCEChunk((idx % 256), (idx / 256));
			return _2byteFontPtr + (_2byteWidth * _2byteHeight / 8) * idx;
		} else {
			idx = SJIStoFMTChunk((idx % 256), (idx / 256));
		}
		break;
	case Common::ZH_TWN:
		{
			int base = 0;
			byte low = idx % 256;
			int high = 0;

			if (low >= 0x20 && low <= 0x7e) {
				base = (3 * low + 81012) * 5;
			} else {
				if (low >= 0xa1 && low <= 0xa3) {
					base = 392820;
					low += 0x5f;
				} else if (low >= 0xa4 && low <= 0xc6) {
					base = 0;
					low += 0x5c;
				} else if (low >= 0xc9 && low <= 0xf9) {
					base = 162030;
					low += 0x37;
				} else {
					base = 392820;
					low = 0xff;
				}

				if (low != 0xff) {
					high = idx / 256;
					if (high >= 0x40 && high <= 0x7e) {
						high -= 0x40;
					} else {
						high -= 0x62;
					}

					base += (low * 0x9d + high) * 30;
				}
			}

			return _2byteFontPtr + base;
			break;
		}
	default:
		idx = 0;
	}
	return	_2byteFontPtr + ((_2byteWidth + 7) / 8) * _2byteHeight * idx;
}


#pragma mark -


CharsetRenderer::CharsetRenderer(ScummEngine *vm) {
	_top = 0;
	_left = 0;
	_startLeft = 0;
	_right = 0;

	_color = 0;

	_center = false;
	_hasMask = false;
	_textScreenID = kMainVirtScreen;
	_blitAlso = false;
	_firstChar = false;
	_disableOffsX = false;

	_vm = vm;
	_curId = -1;
}

CharsetRenderer::~CharsetRenderer() {
}

CharsetRendererCommon::CharsetRendererCommon(ScummEngine *vm)
	: CharsetRenderer(vm), _bytesPerPixel(0), _fontHeight(0), _numChars(0) {
	_shadowMode = kNoShadowMode;
	_shadowColor = 0;
}

void CharsetRendererCommon::setCurID(int32 id) {
	if (id == -1)
		return;

	assertRange(0, id, _vm->_numCharsets - 1, "charset");

	_curId = id;

	_fontPtr = _vm->getResourceAddress(rtCharset, id);
	if (_fontPtr == 0)
		error("CharsetRendererCommon::setCurID: charset %d not found", id);

	if (_vm->_game.version == 4)
		_fontPtr += 17;
	else
		_fontPtr += 29;

	_bytesPerPixel = _fontPtr[0];
	_fontHeight = _fontPtr[1];
	_numChars = READ_LE_UINT16(_fontPtr + 2);
}

void CharsetRendererV3::setCurID(int32 id) {
	if (id == -1)
		return;

	assertRange(0, id, _vm->_numCharsets - 1, "charset");

	_curId = id;

	_fontPtr = _vm->getResourceAddress(rtCharset, id);
	if (_fontPtr == 0)
		error("CharsetRendererCommon::setCurID: charset %d not found", id);

	_bytesPerPixel = 1;
	_numChars = _fontPtr[4];
	_fontHeight = _fontPtr[5];

	_fontPtr += 6;
	_widthTable = _fontPtr;
	_fontPtr += _numChars;
}

int CharsetRendererCommon::getFontHeight() {
	if (_vm->_useCJKMode)
		return MAX(_vm->_2byteHeight + 1, _fontHeight);
	else
		return _fontHeight;
}

// do spacing for variable width old-style font
int CharsetRendererClassic::getCharWidth(byte chr) {
	if (chr >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteWidth / 2;
	int spacing = 0;

	int offs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
	if (offs) {
		spacing = _fontPtr[offs] + (signed char)_fontPtr[offs + 2];
	}

	return spacing;
}

int CharsetRenderer::getStringWidth(int arg, const byte *text) {
	int pos = 0;
	int width = 1;
	byte chr;
	int oldID = getCurID();
	int code = (_vm->_game.heversion >= 80) ? 127 : 64;

	while ((chr = text[pos++]) != 0) {
		if (chr == '\n' || chr == '\r' || chr == _vm->_newLineCharacter)
			break;
		if (_vm->_game.heversion >= 72) {
			if (chr == code) {
				chr = text[pos++];
				if (chr == 84 || chr == 116) {  // Strings of speech offset/size
					while (chr != code)
						chr = text[pos++];
					continue;
				}
				if (chr == 119) // 'Wait'
					break;
				if (chr == 104|| chr == 110) // 'Newline'
					break;
			}
		} else {
			if (chr == '@' && !(_vm->_game.id == GID_CMI && _vm->_language == Common::ZH_TWN))
				continue;
			if (chr == 255 || (_vm->_game.version <= 6 && chr == 254)) {
				chr = text[pos++];
				if (chr == 3)	// 'WAIT'
					break;
				if (chr == 8) { // 'Verb on next line'
					if (arg == 1)
						break;
					while (text[pos++] == ' ')
					;
					continue;
				}
				if (chr == 10 || chr == 21 || chr == 12 || chr == 13) {
					pos += 2;
					continue;
				}
				if (chr == 9 || chr == 1 || chr == 2) // 'Newline'
					break;
				if (chr == 14) {
					int set = text[pos] | (text[pos + 1] << 8);
					pos += 2;
					setCurID(set);
					continue;
				}
			}

			// Some localizations may override colors
			// See credits in Chinese COMI
			if (_vm->_game.id == GID_CMI && _vm->_language == Common::ZH_TWN &&
			    chr == '^' && pos == 1) {
				if (text[pos] == 'c') {
					pos += 4;
					chr = text[pos++];
				}
			}
		}
		if ((chr & 0x80) && _vm->_useCJKMode) {
			pos++;
			width += _vm->_2byteWidth;
		} else {
			width += getCharWidth(chr);
		}
	}

	setCurID(oldID);

	return width;
}

void CharsetRenderer::addLinebreaks(int a, byte *str, int pos, int maxwidth) {
	int lastspace = -1;
	int curw = 1;
	byte chr;
	int oldID = getCurID();
	int code = (_vm->_game.heversion >= 80) ? 127 : 64;

	while ((chr = str[pos++]) != 0) {
		if (_vm->_game.heversion >= 72) {
			if (chr == code) {
				chr = str[pos++];
				if (chr == 84 || chr == 116) {  // Strings of speech offset/size
					while (chr != code)
						chr = str[pos++];
					continue;
				}
				if (chr == 119) // 'Wait'
					break;
				if (chr == 110) { // 'Newline'
					curw = 1;
					continue;
				}
				if (chr == 104) // 'Don't terminate with \n'
					break;
			}
		} else {
			if (chr == '@')
				continue;
			if (chr == 255 || (_vm->_game.version <= 6 && chr == 254)) {
				chr = str[pos++];
				if (chr == 3) // 'Wait'
					break;
				if (chr == 8) { // 'Verb on next line'
					if (a == 1) {
						curw = 1;
					} else {
						while (str[pos] == ' ')
							str[pos++] = '@';
					}
					continue;
				}
				if (chr == 10 || chr == 21 || chr == 12 || chr == 13) {
					pos += 2;
					continue;
				}
				if (chr == 1) { // 'Newline'
					curw = 1;
					continue;
				}
				if (chr == 2) // 'Don't terminate with \n'
					break;
				if (chr == 14) {
					int set = str[pos] | (str[pos + 1] << 8);
					pos += 2;
					setCurID(set);
					continue;
				}
			}
		}
		if (chr == ' ')
			lastspace = pos - 1;

		if (chr == _vm->_newLineCharacter)
			lastspace = pos - 1;

		if ((chr & 0x80) && _vm->_useCJKMode) {
			pos++;
			curw += _vm->_2byteWidth;
		} else {
			curw += getCharWidth(chr);
		}
		if (lastspace == -1)
			continue;
		if (curw > maxwidth) {
			str[lastspace] = 0xD;
			curw = 1;
			pos = lastspace + 1;
			lastspace = -1;
		}
	}

	setCurID(oldID);
}

int CharsetRendererV3::getCharWidth(byte chr) {
	if (chr & 0x80 && _vm->_useCJKMode)
		return _vm->_2byteWidth / 2;
	int spacing = 0;

	spacing = *(_widthTable + chr);

	return spacing;
}

void CharsetRendererV3::setColor(byte color) {
	bool useShadow = false;
	_color = color;

	// FM-TOWNS version of Loom uses old color method as well
	if ((_vm->_game.version >= 2) && ((_vm->_game.features & GF_16COLOR) || (_vm->_game.id == GID_LOOM && _vm->_game.version == 3))) {
		useShadow = ((_color & 0xF0) != 0);
		_color &= 0x0f;
	} else if (_vm->_game.features & GF_OLD256) {
		useShadow = ((_color & 0x80) != 0);
		_color &= 0x7f;
	} else
		useShadow = false;

	enableShadow(useShadow);

	translateColor();
}

void CharsetRendererCommon::enableShadow(bool enable) {
	if (enable) {
		if (_vm->_game.platform == Common::kPlatformFMTowns) {
			_shadowColor = 8;
			_shadowMode = kFMTOWNSShadowMode;
		} else {
			_shadowColor = 0;
			_shadowMode = kNormalShadowMode;
		}
	} else {
		_shadowMode = kNoShadowMode;
	}
}


void CharsetRendererV3::printChar(int chr, bool ignoreCharsetMask) {
	// WORKAROUND for bug #1509509: Indy3 Mac does not show black
	// characters (such as in the grail diary) if ignoreCharsetMask
	// is true. See also patch #1851568.
	if (_vm->_game.id == GID_INDY3 && _vm->_game.platform == Common::kPlatformMacintosh && _color == 0)
		ignoreCharsetMask = false;

	// Indy3 / Zak256 / Loom
	int width, height, origWidth = 0, origHeight;
	VirtScreen *vs;
	const byte *charPtr;
	byte *dst;
	int is2byte = (chr >= 0x80 && _vm->_useCJKMode) ? 1 : 0;

	assertRange(0, _curId, _vm->_numCharsets - 1, "charset");

	if ((vs = _vm->findVirtScreen(_top)) == NULL)
		return;

	if (chr == '@')
		return;

	if (is2byte) {
		charPtr = _vm->get2byteCharPtr(chr);
		width = _vm->_2byteWidth;
		height = _vm->_2byteHeight;
	} else {
		charPtr = _fontPtr + chr * 8;
		width = getCharWidth(chr);
		height = 8;
	}

	// Clip at the right side (to avoid drawing "outside" the screen bounds).
	if (_left + origWidth > _right + 1)
		return;

	origWidth = width;
	origHeight = height;

	if (_shadowMode != kNoShadowMode) {
		width++;
		height++;
	}

	if (_firstChar) {
		_str.left = _left;
		_str.top = _top;
		_str.right = _left;
		_str.bottom = _top;
		_firstChar = false;
	}

	int drawTop = _top - vs->topline;

	_vm->markRectAsDirty(vs->number, _left, _left + width, drawTop, drawTop + height);

	if (!ignoreCharsetMask) {
		_hasMask = true;
		_textScreenID = vs->number;
	}
	if ((ignoreCharsetMask || !vs->hasTwoBuffers) && !(_vm->_useCJKMode && _vm->_textSurfaceMultiplier == 2)) {
		dst = vs->getPixels(_left, drawTop);
		drawBits1(*vs, dst, charPtr, drawTop, origWidth, origHeight, vs->bytesPerPixel);
	} else {
		dst = (byte *)_vm->_textSurface.getBasePtr(_left * _vm->_textSurfaceMultiplier, _top * _vm->_textSurfaceMultiplier);
		drawBits1(_vm->_textSurface, dst, charPtr, drawTop, origWidth, origHeight, _vm->_textSurface.bytesPerPixel);
	}

	if (_str.left > _left)
		_str.left = _left;

	_left += origWidth / _vm->_textSurfaceMultiplier;

	if (_str.right < _left) {
		_str.right = _left;
		if (_shadowMode != kNoShadowMode)
			_str.right++;
	}

	if (_str.bottom < _top + height / _vm->_textSurfaceMultiplier)
		_str.bottom = _top + height / _vm->_textSurfaceMultiplier;
}

void CharsetRendererV3::drawChar(int chr, const Graphics::Surface &s, int x, int y) {
	const byte *charPtr;
	byte *dst;
	int width, height;
	int is2byte = (chr >= 0x80 && _vm->_useCJKMode) ? 1 : 0;
	if (is2byte) {
		charPtr = _vm->get2byteCharPtr(chr);
		width = _vm->_2byteWidth;
		height = _vm->_2byteHeight;
	} else {
		charPtr = _fontPtr + chr * 8;
//		width = height = 8;
		width = getCharWidth(chr);
		height = 8;
	}
	dst = (byte *)s.pixels + y * s.pitch + x;
	drawBits1(s, dst, charPtr, y, width, height, s.bytesPerPixel);
}

void CharsetRenderer::translateColor() {
	// Based on disassembly
	if (_vm->_renderMode == Common::kRenderCGA) {
		static const byte CGAtextColorMap[16] = {0,  3, 3, 3, 5, 5, 5,  15,
										   15, 3, 3, 3, 5, 5, 15, 15};
		_color = CGAtextColorMap[_color & 0x0f];
	}

	if (_vm->_renderMode == Common::kRenderHercA || _vm->_renderMode == Common::kRenderHercG) {
		static const byte HercTextColorMap[16] = {0, 15,  2, 15, 15,  5, 15,  15,
										   8, 15, 15, 15, 15, 15, 15, 15};
		_color = HercTextColorMap[_color & 0x0f];
	}
}

void CharsetRenderer::saveLoadWithSerializer(Serializer *ser) {
	static const SaveLoadEntry charsetRendererEntries[] = {
		MKLINE_OLD(CharsetRenderer, _curId, sleByte, VER(73), VER(73)),
		MKLINE(CharsetRenderer, _curId, sleInt32, VER(74)),
		MKLINE(CharsetRenderer, _color, sleByte, VER(73)),
		MKEND()
	};

	ser->saveLoadEntries(this, charsetRendererEntries);

	if (ser->isLoading()) {
		setCurID(_curId);
		setColor(_color);
	}
}

void CharsetRendererClassic::printChar(int chr, bool ignoreCharsetMask) {
	int width, height, origWidth, origHeight;
	int offsX, offsY;
	VirtScreen *vs;
	const byte *charPtr;
	bool is2byte = (chr >= 0x80 && _vm->_useCJKMode);

	assertRange(1, _curId, _vm->_numCharsets - 1, "charset");

	if ((vs = _vm->findVirtScreen(_top)) == NULL && (vs = _vm->findVirtScreen(_top + getFontHeight())) == NULL)
		return;

	if (chr == '@')
		return;

	translateColor();

	_vm->_charsetColorMap[1] = _color;

	if (is2byte) {
		enableShadow(true);
		charPtr = _vm->get2byteCharPtr(chr);
		width = _vm->_2byteWidth;
		height = _vm->_2byteHeight;
		offsX = offsY = 0;
	} else {
		uint32 charOffs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
		assert(charOffs < 0x10000);
		if (!charOffs)
			return;
		charPtr = _fontPtr + charOffs;

		width = charPtr[0];
		height = charPtr[1];

		if (_disableOffsX) {
			offsX = 0;
		} else {
			offsX = (signed char)charPtr[2];
		}

		offsY = (signed char)charPtr[3];

		charPtr += 4;	// Skip over char header
	}
	origWidth = width;
	origHeight = height;

	if (_shadowMode != kNoShadowMode) {
		width++;
		height++;
	}
	if (_firstChar) {
		_str.left = 0;
		_str.top = 0;
		_str.right = 0;
		_str.bottom = 0;
	}

	_top += offsY;
	_left += offsX;

	if (_left + origWidth / _vm->_textSurfaceMultiplier > _right + 1 || _left < 0) {
		_left += origWidth / _vm->_textSurfaceMultiplier;
		_top -= offsY;
		return;
	}

	_disableOffsX = false;

	if (_firstChar) {
		_str.left = _left;
		_str.top = _top;
		_str.right = _left;
		_str.bottom = _top;
		_firstChar = false;
	}

	if (_left < _str.left)
		_str.left = _left;

	if (_top < _str.top)
		_str.top = _top;

	int drawTop = _top - vs->topline;

	_vm->markRectAsDirty(vs->number, _left, _left + width, drawTop, drawTop + height);

	if (!ignoreCharsetMask) {
		_hasMask = true;
		_textScreenID = vs->number;
	}

	printCharIntern(is2byte, charPtr, origWidth, origHeight, width, height, vs, ignoreCharsetMask);

	_left += origWidth / _vm->_textSurfaceMultiplier;

	if (_str.right < _left) {
		_str.right = _left;
		if (_shadowMode != kNoShadowMode)
			_str.right++;
	}

	if (_str.bottom < _top + height / _vm->_textSurfaceMultiplier)
		_str.bottom = _top + height / _vm->_textSurfaceMultiplier;

	_top -= offsY;
}

void CharsetRendererClassic::printCharIntern(bool is2byte, const byte *charPtr, int origWidth, int origHeight, int width, int height, VirtScreen *vs, bool ignoreCharsetMask) {
	byte *dstPtr;
	byte *back = NULL;
	int drawTop = _top - vs->topline;

	if ((_vm->_game.heversion >= 71 && _bytesPerPixel >= 8) || (_vm->_game.heversion >= 90 && _bytesPerPixel == 0)) {
#ifdef ENABLE_HE
		if (ignoreCharsetMask || !vs->hasTwoBuffers) {
			dstPtr = vs->getPixels(0, 0);
		} else {
			dstPtr = (byte *)_vm->_textSurface.pixels;
		}

		if (_blitAlso && vs->hasTwoBuffers) {
			dstPtr = vs->getBackPixels(0, 0);
		}

		Common::Rect rScreen(vs->w, vs->h);
		if (_bytesPerPixel >= 8) {
			byte imagePalette[256];
			memset(imagePalette, 0, sizeof(imagePalette));
			memcpy(imagePalette, _vm->_charsetColorMap, 4);
			Wiz::copyWizImage(dstPtr, charPtr, vs->pitch, kDstScreen, vs->w, vs->h, _left, _top, origWidth, origHeight, &rScreen, 0, imagePalette, NULL, _vm->_bytesPerPixel);
		} else {
			Wiz::copyWizImage(dstPtr, charPtr, vs->pitch, kDstScreen, vs->w, vs->h, _left, _top, origWidth, origHeight, &rScreen, 0, NULL, NULL, _vm->_bytesPerPixel);
		}

		if (_blitAlso && vs->hasTwoBuffers) {
			Common::Rect dst(_left, _top, _left + origWidth, _top + origHeight);
			((ScummEngine_v71he *)_vm)->restoreBackgroundHE(dst);
		}
#endif
	} else {
		Graphics::Surface dstSurface;
		Graphics::Surface backSurface;
		if ((ignoreCharsetMask || !vs->hasTwoBuffers) && !(_vm->_useCJKMode && _vm->_textSurfaceMultiplier == 2)) {
			dstSurface = *vs;
			dstPtr = vs->getPixels(_left, drawTop);
		} else {
			dstSurface = _vm->_textSurface;
			dstPtr = (byte *)_vm->_textSurface.pixels + (_top - _vm->_screenTop) * _vm->_textSurface.pitch * _vm->_textSurfaceMultiplier + _left * _vm->_textSurfaceMultiplier;
		}

		if (_blitAlso && vs->hasTwoBuffers) {
			backSurface = dstSurface;
			back = dstPtr;
			dstSurface = *vs;
			dstPtr = vs->getBackPixels(_left, drawTop);
		}

		if (!ignoreCharsetMask && vs->hasTwoBuffers) {
			drawTop = _top - _vm->_screenTop;
		}

		if (is2byte) {
			drawBits1(dstSurface, dstPtr, charPtr, drawTop, origWidth, origHeight, dstSurface.bytesPerPixel);
		} else {
			drawBitsN(dstSurface, dstPtr, charPtr, *_fontPtr, drawTop, origWidth, origHeight);
		}

		if (_blitAlso && vs->hasTwoBuffers) {
			// FIXME: Revisiting this code, I think the _blitAlso mode is likely broken
			// right now -- we are copying stuff from "dstPtr" to "back", but "dstPtr" really
			// only conatains charset data...
			// One way to fix this: don't copy etc.; rather simply render the char twice,
			// once to each of the two buffers. That should hypothetically yield
			// identical results, though I didn't try it and right now I don't know
			// any spots where I can test this...
			if (!ignoreCharsetMask)
				error("This might be broken -- please report where you encountered this to Fingolfin");

			// Perform some clipping
			int w = MIN(width, dstSurface.w - _left);
			int h = MIN(height, dstSurface.h - drawTop);
			if (_left < 0) {
				w += _left;
				back -= _left;
				dstPtr -= _left;
			}
			if (drawTop < 0) {
				h += drawTop;
				back -= drawTop * backSurface.pitch;
				dstPtr -= drawTop * dstSurface.pitch;
			}

			// Blit the image data
			if (w > 0) {
				while (h-- > 0) {
					memcpy(back, dstPtr, w);
					back += backSurface.pitch;
					dstPtr += dstSurface.pitch;
				}
			}
		}
	}
}

void CharsetRendererClassic::drawChar(int chr, const Graphics::Surface &s, int x, int y) {
	const byte *charPtr;
	byte *dst;
	int width, height;
	int is2byte = (chr >= 0x80 && _vm->_useCJKMode) ? 1 : 0;

	if (is2byte) {
		enableShadow(true);
		charPtr = _vm->get2byteCharPtr(chr);
		width = _vm->_2byteWidth;
		height = _vm->_2byteHeight;
	} else {
		uint32 charOffs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
		assert(charOffs < 0x10000);
		if (!charOffs)
			return;
		charPtr = _fontPtr + charOffs;

		width = charPtr[0];
		height = charPtr[1];

		charPtr += 4;	// Skip over char header
	}

	dst = (byte *)s.pixels + y * s.pitch + x;

	if (is2byte) {
		drawBits1(s, dst, charPtr, y, width, height, s.bytesPerPixel);
	} else {
		drawBitsN(s, dst, charPtr, *_fontPtr, y, width, height);
	}
}

void CharsetRendererClassic::drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height) {
	int y, x;
	int color;
	byte numbits, bits;

	assert(bpp == 1 || bpp == 2 || bpp == 4 || bpp == 8);
	bits = *src++;
	numbits = 8;

	for (y = 0; y < height && y + drawTop < s.h; y++) {
		for (x = 0; x < width; x++) {
			color = (bits >> (8 - bpp)) & 0xFF;

			if (color && y + drawTop >= 0) {
				*dst = _vm->_charsetColorMap[color];
			}
			dst++;
			bits <<= bpp;
			numbits -= bpp;
			if (numbits == 0) {
				bits = *src++;
				numbits = 8;
			}
		}
		dst += s.pitch - width;
	}
}

void CharsetRendererCommon::drawBits1(const Graphics::Surface &s, byte *dst, const byte *src, int drawTop, int width, int height, uint8 bitDepth) {
	int y, x;
	byte bits = 0;

	for (y = 0; y < height && y + drawTop < s.h; y++) {
		for (x = 0; x < width; x++) {
			if ((x % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask(x % 8)) && y + drawTop >= 0) {
				if (bitDepth == 2) {
					if (_shadowMode != kNoShadowMode) {
						WRITE_UINT16(dst + 2, _vm->_16BitPalette[_shadowColor]);
						WRITE_UINT16(dst + s.pitch, _vm->_16BitPalette[_shadowColor]);
						if (_shadowMode != kFMTOWNSShadowMode)
							WRITE_UINT16(dst + s.pitch + 2, _vm->_16BitPalette[_shadowColor]);
					}
					WRITE_UINT16(dst, _vm->_16BitPalette[_color]);
				} else {
					if (_shadowMode != kNoShadowMode) {
						*(dst + 1) = _shadowColor;
						*(dst + s.pitch) = _shadowColor;
						if (_shadowMode != kFMTOWNSShadowMode)
							*(dst + s.pitch + 1) = _shadowColor;
					}
					*dst = _color;
				}
			}
			dst += bitDepth;
		}

		dst += s.pitch - width * bitDepth;
	}
}

void CharsetRendererPCE::drawBits1(const Graphics::Surface &s, byte *dst, const byte *src, int drawTop, int width, int height, uint8 bitDepth) {
	int y, x;
	int bitCount = 0;
	byte bits = 0;

	const bool resetLineBitCount = (_vm->_language != Common::JA_JPN || width != 12);

	for (y = 0; y < height && y + drawTop < s.h; y++) {
		if (resetLineBitCount)
			bitCount = 0;
		for (x = 0; x < width; x++) {
			if ((bitCount % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask(bitCount % 8)) && y + drawTop >= 0) {
				if (bitDepth == 2) {
					if (_shadowMode != kNoShadowMode) {
						WRITE_UINT16(dst + s.pitch + 2, 0);
					}
					WRITE_UINT16(dst, _vm->_16BitPalette[_color]);
				} else {
					if (_shadowMode != kNoShadowMode) {
						*(dst + 1) = _shadowColor;
						*(dst + s.pitch) = _shadowColor;
						*(dst + s.pitch + 1) = _shadowColor;
					}
					*dst = _color;
				}
			}
			dst += bitDepth;
			bitCount++;
		}

		dst += s.pitch - width * bitDepth;
	}
}

#ifdef ENABLE_SCUMM_7_8
CharsetRendererNut::CharsetRendererNut(ScummEngine *vm)
	 : CharsetRenderer(vm) {
	_current = 0;

	for (int i = 0; i < 5; i++) {
		_fr[i] = NULL;
	}
}

CharsetRendererNut::~CharsetRendererNut() {
	for (int i = 0; i < 5; i++) {
		delete _fr[i];
	}
}

void CharsetRendererNut::setCurID(int32 id) {
	if (id == -1)
		return;

	int numFonts = ((_vm->_game.id == GID_CMI) && (_vm->_game.features & GF_DEMO)) ? 4 : 5;
	assert(id < numFonts);
	_curId = id;
	if (!_fr[id]) {
		char fontname[11];
		sprintf(fontname, "font%d.nut", id);
		_fr[id] = new NutRenderer(_vm, fontname);
	}
	_current = _fr[id];
	assert(_current);
}

int CharsetRendererNut::getCharHeight(byte chr) {
	assert(_current);
	return _current->getCharHeight(chr);
}

int CharsetRendererNut::getCharWidth(byte chr) {
	assert(_current);
	return _current->getCharWidth(chr);
}

int CharsetRendererNut::getFontHeight() {
	// FIXME / TODO: how to implement this properly???
	assert(_current);
	return _current->getCharHeight('|');
}

void CharsetRendererNut::printChar(int chr, bool ignoreCharsetMask) {
	Common::Rect shadow;

	assert(_current);
	if (chr == '@')
		return;

	shadow.left = _left;
	shadow.top = _top;

	if (_firstChar) {
		_str.left = (shadow.left >= 0) ? shadow.left : 0;
		_str.top = (shadow.top >= 0) ? shadow.top : 0;
		_str.right = _str.left;
		_str.bottom = _str.top;
		_firstChar = false;
	}

	int width = _current->getCharWidth(chr);
	int height = _current->getCharHeight(chr);

	if (chr >= 256 && _vm->_useCJKMode)
		width = _vm->_2byteWidth;

	shadow.right = _left + width;
	shadow.bottom = _top + height;

	Graphics::Surface s;
	if (!ignoreCharsetMask) {
		_hasMask = true;
		_textScreenID = kMainVirtScreen;
	}

	int drawTop = _top;
	if (ignoreCharsetMask) {
		VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
		s = *vs;
		s.pixels = vs->getPixels(0, 0);
	} else {
		s = _vm->_textSurface;
		drawTop -= _vm->_screenTop;
	}

	if (chr >= 256 && _vm->_useCJKMode)
		_current->draw2byte(s, chr, _left, drawTop, _color);
	else
		_current->drawChar(s, (byte)chr, _left, drawTop, _color);
	_vm->markRectAsDirty(kMainVirtScreen, shadow);

	if (_str.left > _left)
		_str.left = _left;

	// Original keeps glyph width and character dimensions separately
	if (_vm->_language == Common::ZH_TWN && width == 16)
		width = 17;

	_left += width;

	if (_str.right < shadow.right)
		_str.right = shadow.right;

	if (_str.bottom < shadow.bottom)
		_str.bottom = shadow.bottom;
}
#endif

void CharsetRendererNES::printChar(int chr, bool ignoreCharsetMask) {
	int width, height, origWidth, origHeight;
	VirtScreen *vs;
	byte *charPtr, *dst;

	// Init it here each time since it is cheap and fixes bug with
	// charset after game load
	_trTable = _vm->getResourceAddress(rtCostume, 77) + 2;

	// HACK: how to set it properly?
	if (_top == 0)
		_top = 16;

	if ((vs = _vm->findVirtScreen(_top)) == NULL)
		return;

	if (chr == '@')
		return;

	charPtr = _vm->_NESPatTable[1] + _trTable[chr - 32] * 16;
	width = getCharWidth(chr);
	height = 8;

	origWidth = width;
	origHeight = height;

	if (_firstChar) {
		_str.left = _left;
		_str.top = _top;
		_str.right = _left;
		_str.bottom = _top;
		_firstChar = false;
	}

	int drawTop = _top - vs->topline;

	_vm->markRectAsDirty(vs->number, _left, _left + width, drawTop, drawTop + height);

	if (!ignoreCharsetMask) {
		_hasMask = true;
		_textScreenID = vs->number;
	}

	if (ignoreCharsetMask || !vs->hasTwoBuffers) {
		dst = vs->getPixels(_left, drawTop);
		drawBits1(*vs, dst, charPtr, drawTop, origWidth, origHeight, vs->bytesPerPixel);
	} else {
		dst = (byte *)_vm->_textSurface.pixels + _top * _vm->_textSurface.pitch + _left;
		drawBits1(_vm->_textSurface, dst, charPtr, drawTop, origWidth, origHeight, _vm->_textSurface.bytesPerPixel);
	}

	if (_str.left > _left)
		_str.left = _left;

	_left += origWidth;

	if (_str.right < _left) {
		_str.right = _left;
		if (_shadowMode != kNoShadowMode)
			_str.right++;
	}

	if (_str.bottom < _top + height)
		_str.bottom = _top + height;
}

void CharsetRendererNES::drawChar(int chr, const Graphics::Surface &s, int x, int y) {
	byte *charPtr, *dst;
	int width, height;

	if (!_trTable)
		_trTable = _vm->getResourceAddress(rtCostume, 77) + 2;

	charPtr = _vm->_NESPatTable[1] + _trTable[chr - 32] * 16;
	width = getCharWidth(chr);
	height = 8;

	dst = (byte *)s.pixels + y * s.pitch + x;
	drawBits1(s, dst, charPtr, y, width, height, s.bytesPerPixel);
}

void CharsetRendererNES::drawBits1(const Graphics::Surface &s, byte *dst, const byte *src, int drawTop, int width, int height, uint8 bitDepth) {
	for (int i = 0; i < 8; i++) {
		byte c0 = src[i];
		byte c1 = src[i + 8];
		for (int j = 0; j < 8; j++)
			dst[j] = _vm->_NESPalette[0][((c0 >> (7 - j)) & 1) | (((c1 >> (7 - j)) & 1) << 1) |
			(_color ? 12 : 8)];
		dst += s.pitch;
	}
}

} // End of namespace Scumm
