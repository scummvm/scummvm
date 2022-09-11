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

#include "common/macresman.h"

#include "scumm/charset.h"
#include "scumm/file.h"
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

bool ScummEngine::isScummvmKorTarget() {
	if (_language == Common::KO_KOR && (_game.version < 7 || _game.id == GID_FT)) {
		return true;
	}
	return false;
}

void ScummEngine::loadCJKFont() {
	_useCJKMode = false;
	_textSurfaceMultiplier = 1;
	_newLineCharacter = 0;

	_useMultiFont = false;	// Korean Multi-Font

	// Special case for Korean
	if (isScummvmKorTarget()) {
		loadKorFont();

		return;
	}

	ScummFile fp;

	if (_game.version <= 5 && _game.platform == Common::kPlatformFMTowns && _language == Common::JA_JPN) { // FM-TOWNS v3 / v5 Kanji
#if defined(DISABLE_TOWNS_DUAL_LAYER_MODE) || !defined(USE_RGB_COLOR)
		GUIErrorMessage("FM-Towns Kanji font drawing requires dual graphics layer support which is disabled in this build");
		error("FM-Towns Kanji font drawing requires dual graphics layer support which is disabled in this build");
#else
		// use FM-TOWNS font rom, since game files don't have kanji font resources
		_cjkFont = Graphics::FontSJIS::createFont(_game.platform);
		if (!_cjkFont)
			error("SCUMM::Font: Could not open file 'FMT_FNT.ROM'");
		_textSurfaceMultiplier = 2;
		_useCJKMode = true;
#endif
	} else if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && _language == Common::JA_JPN) {
#ifdef USE_RGB_COLOR
		// use PC-Engine System Card, since game files don't have kanji font resources
		_cjkFont = Graphics::FontSJIS::createFont(_game.platform);
		if (!_cjkFont)
			error("SCUMM::Font: Could not open file 'pce.cdbios'");

		_cjkFont->setDrawingMode(Graphics::FontSJIS::kShadowRightMode);
		_2byteWidth = _2byteHeight = 12;
		_useCJKMode = true;
#endif
	} else if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD && _language == Common::JA_JPN) {
		int numChar = 1413;
		_2byteWidth = 16;
		_2byteHeight = 16;
		_useCJKMode = true;
		_newLineCharacter = 0x5F;
		// charset resources are not inited yet, load charset later
		_2byteFontPtr = new byte[_2byteWidth * _2byteHeight * numChar / 8];
		// set byte 0 to 0xFF (0x00 when loaded) to indicate that the font was not loaded
		_2byteFontPtr[0] = 0xFF;
	} else if (_language == Common::KO_KOR ||
			   (_game.version >= 7 && (_language == Common::JA_JPN || _language == Common::ZH_TWN)) ||
			   (_game.version >= 3 && _language == Common::ZH_CHN)) {
		int numChar = 0;
		const char *fontFile = nullptr;

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
			// Both The DIG and COMI use same font
			fontFile = "chinese.fnt";
			numChar = 13630;
			break;
		case Common::ZH_CHN:
			if (_game.id == GID_FT || _game.id == GID_LOOM || _game.id == GID_INDY3 ||
				_game.id == GID_INDY4 || _game.id == GID_MONKEY || _game.id == GID_MONKEY2 ||
				_game.id == GID_TENTACLE) {
				fontFile = "chinese_gb16x12.fnt";
				numChar = 8178;
			}
			break;
		default:
			break;
		}
		if (fontFile && openFile(fp, fontFile)) {
			debug(2, "Loading CJK Font");
			_useCJKMode = true;
			_textSurfaceMultiplier = 1; // No multiplication here

			switch (_language) {
			case Common::KO_KOR:
				fp.seek(2, SEEK_CUR);
				_2byteWidth = fp.readByte();
				_2byteHeight = fp.readByte();
				_newLineCharacter = (_game.id == GID_CMI) ? 0xff : 0xfe;
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
			case Common::ZH_CHN:
				_2byteWidth = 12;
				_2byteHeight = 12;
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
				error("SCUMM::Font: Could not open %s",fontFile);
			else
				error("SCUMM::Font: Could not load any font");
		}
	}
}

void ScummEngine::loadKorFont() {
	Common::File fp;
	int numChar = 2350;
	_useCJKMode = true;

	if (_game.version < 7 || _game.id == GID_FT)
		_useMultiFont = true;

	if (_useMultiFont) {
		debug("Loading Korean Multi Font System");
		_numLoadedFont = 0;
		_2byteFontPtr = nullptr;
		_2byteWidth = 0;
		_2byteHeight = 0;
		for (int i = 0; i < 20; i++) {
			char fontFile[256];
			snprintf(fontFile, sizeof(fontFile), "korean%02d.fnt", i);
			_2byteMultiFontPtr[i] = nullptr;
			if (fp.open(fontFile)) {
				_numLoadedFont++;
				fp.readByte();
				_2byteMultiShadow[i] = fp.readByte();
				_2byteMultiWidth[i] = fp.readByte();
				_2byteMultiHeight[i] = fp.readByte();

				int fontSize = ((_2byteMultiWidth[i] + 7) / 8) * _2byteMultiHeight[i] * numChar;
				_2byteMultiFontPtr[i] = new byte[fontSize];
				warning("#%d, size %d, height =%d", i, fontSize, _2byteMultiHeight[i]);
				fp.read(_2byteMultiFontPtr[i], fontSize);
				fp.close();
				if (_2byteFontPtr == nullptr) {	// for non-initialized Smushplayer drawChar
					_2byteFontPtr = _2byteMultiFontPtr[i];
					_2byteWidth = _2byteMultiWidth[i];
					_2byteHeight = _2byteMultiHeight[i];
					_2byteShadow = _2byteMultiShadow[i];
				}
			}
		}
		if (_numLoadedFont == 0) {
			warning("Cannot load any font for multi font");
			_useMultiFont = false;
		} else {
			debug("%d fonts are loaded", _numLoadedFont);
		}
	}

	if (!_useMultiFont) {
		debug("Loading Korean Single Font System");
		if (fp.open("korean.fnt")) {
			fp.seek(2, SEEK_CUR);
			_2byteWidth = fp.readByte();
			_2byteHeight = fp.readByte();
			_2byteFontPtr = new byte[((_2byteWidth + 7) / 8) * _2byteHeight * numChar];
			fp.read(_2byteFontPtr, ((_2byteWidth + 7) / 8) * _2byteHeight * numChar);
			fp.close();
		} else {
			error("Couldn't load any font: %s", fp.getName());
		}
	}
	return;
}

byte *ScummEngine::get2byteCharPtr(int idx) {
	if (_game.platform == Common::kPlatformFMTowns || _game.platform == Common::kPlatformPCEngine)
		return nullptr;

	switch (_language) {
	case Common::KO_KOR:
		idx = ((idx % 256) - 0xb0) * 94 + (idx / 256) - 0xa1;
		break;
	case Common::JA_JPN:
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD && _language == Common::JA_JPN) {
			// init pointer to charset resource
			if (_2byteFontPtr[0] == 0xFF) {
				int charsetId = 5;
				int numChar = 1413;
				byte *charsetPtr = getResourceAddress(rtCharset, charsetId);
				if (charsetPtr == nullptr)
					error("ScummEngine::get2byteCharPtr: charset %d not found", charsetId);
				memcpy(_2byteFontPtr, charsetPtr + 46, _2byteWidth * _2byteHeight * numChar / 8);
			}

			idx = (SWAP_CONSTANT_16(idx) & 0x7fff) - 1;
		} else {
			idx = Graphics::FontTowns::getCharFMTChunk(idx);
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
		}
	case Common::ZH_CHN:
		idx = ((idx % 256) - 0xa1)* 94  + ((idx / 256) - 0xa1);
		break;
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
	_enableShadow = false;
	_shadowColor = 0;
}

void CharsetRendererCommon::setCurID(int32 id) {
	if (id == -1)
		return;

	assertRange(0, id, _vm->_numCharsets - 1, "charset");

	_curId = id;

	_fontPtr = _vm->getResourceAddress(rtCharset, id);
	if (_fontPtr == nullptr)
		error("CharsetRendererCommon::setCurID: charset %d not found", id);

	if (_vm->_game.version == 4)
		_fontPtr += 17;
	else
		_fontPtr += 29;

	_bytesPerPixel = _fontPtr[0];
	_fontHeight = _fontPtr[1];
	_numChars = READ_LE_UINT16(_fontPtr + 2);

	if (_vm->_useMultiFont) {
		if (id == 6)    // HACK: Fix monkey1cd/monkey2/dott font error
			id = 0;

		if (_vm->_2byteMultiFontPtr[id]) {
			_vm->_2byteFontPtr = _vm->_2byteMultiFontPtr[id];
			_vm->_2byteWidth = _vm->_2byteMultiWidth[id];
			_vm->_2byteHeight = _vm->_2byteMultiHeight[id];
			_vm->_2byteShadow = _vm->_2byteMultiShadow[id];
		} else {
			// Get nearest font set (by height)
			debug(7, "Cannot find matching font set for charset #%d, use nearest font set", id);
			int dstHeight = _fontHeight;
			int nearest = 0;
			for (int i = 0; i < _vm->_numLoadedFont; i++) {
				if (ABS(_vm->_2byteMultiHeight[i] - dstHeight) <= ABS(_vm->_2byteMultiHeight[nearest] - dstHeight)) {
					nearest = i;
				}
			}
			debug(7, "Found #%d", nearest);
			_vm->_2byteFontPtr = _vm->_2byteMultiFontPtr[nearest];
			_vm->_2byteWidth = _vm->_2byteMultiWidth[nearest];
			_vm->_2byteHeight = _vm->_2byteMultiHeight[nearest];
			_vm->_2byteShadow = _vm->_2byteMultiShadow[nearest];
		}
	}
}

void CharsetRendererV3::setCurID(int32 id) {
	if (id == -1)
		return;

	assertRange(0, id, _vm->_numCharsets - 1, "charset");

	_curId = id;

	_fontPtr = _vm->getResourceAddress(rtCharset, id);
	if (_fontPtr == nullptr)
		error("CharsetRendererCommon::setCurID: charset %d not found", id);

	_bytesPerPixel = 1;
	_numChars = _fontPtr[4];
	_fontHeight = _fontPtr[5];

	_fontPtr += 6;
	_widthTable = _fontPtr;
	_fontPtr += _numChars;

	if (_vm->_useMultiFont) {
		if (_vm->_2byteMultiFontPtr[id]) {
			_vm->_2byteFontPtr = _vm->_2byteMultiFontPtr[id];
			_vm->_2byteWidth = _vm->_2byteMultiWidth[id];
			_vm->_2byteHeight = _vm->_2byteMultiHeight[id];
			_vm->_2byteShadow = _vm->_2byteMultiShadow[id];
		} else {
			// Get nearest font set (by height)
			debug(7, "Cannot find matching font set for charset #%d, use nearest font set", id);
			int dstHeight = _fontHeight;
			int nearest = 0;
			for (int i = 0; i < _vm->_numLoadedFont; i++) {
				if (ABS(_vm->_2byteMultiHeight[i] - dstHeight) <= ABS(_vm->_2byteMultiHeight[nearest] - dstHeight)) {
					nearest = i;
				}
			}
			debug(7, "Found #%d", nearest);
			_vm->_2byteFontPtr = _vm->_2byteMultiFontPtr[nearest];
			_vm->_2byteWidth = _vm->_2byteMultiWidth[nearest];
			_vm->_2byteHeight = _vm->_2byteMultiHeight[nearest];
			_vm->_2byteShadow = _vm->_2byteMultiShadow[nearest];
		}
	}
}

int CharsetRendererCommon::getFontHeight() const {
	if (_vm->_useCJKMode)
		return MAX(_vm->_2byteHeight + 1, _fontHeight);
	else
		return _fontHeight;
}

// do spacing for variable width old-style font
int CharsetRendererClassic::getCharWidth(uint16 chr) const {
	int spacing = 0;

	if (_vm->_useCJKMode && chr >= 0x80)
		return _vm->_2byteWidth / 2;

	int offs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
	if (offs)
		spacing = _fontPtr[offs] + (signed char)_fontPtr[offs + 2];

	return spacing;
}

int CharsetRenderer::getStringWidth(int arg, const byte *text) {
	int pos = 0;

	// I have confirmed from disasm that neither LOOM EGA and FM-TOWNS (EN/JP) nor any other games withing the
	// v0-v3 version range add 1 to the width. There isn't even a getStringWidth method. And the v0-2 games don't
	// even support text rendering over strip borders. However, LOOM VGA Talkie and MONKEY1 EGA do have the
	// getStringWidth method and they do add 1 to the width. So that seems to have been introduced with version 4.
	int width = (_vm->_game.version < 4 || _vm->_game.id == GID_FT) ? 0 : 1;

	int chr;
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
			if (chr == '@')
				continue;
			if (chr == 255 || (_vm->_game.version <= 6 && chr == 254)) {
				chr = text[pos++];
				if (chr == 3)	// 'WAIT'
					break;
				if (chr == 8) { // 'Verb on next line'
					if (arg == 1)
						break;
					while (text[pos++] == ' ') {}
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
		}

		if (_vm->_useCJKMode) {
			if (_vm->_game.platform == Common::kPlatformFMTowns) {
				if (checkSJISCode(chr))
					// This strange character conversion is the exact way the original does it here.
					// This is the only way to get an accurate text formatting in the MI1 intro.
					chr = (int8)text[pos++] | (chr << 8);
			} else if (chr & 0x80) {
				pos++;
				width += _vm->_2byteWidth;
				// Original keeps glyph width and character dimensions separately
				if (_vm->_language == Common::KO_KOR || _vm->_language == Common::ZH_TWN) {
					width++;
				}
				continue;
			}
		}
		width += getCharWidth(chr);
	}

	setCurID(oldID);

	return width;
}

void CharsetRenderer::addLinebreaks(int a, byte *str, int pos, int maxwidth) {
	int lastKoreanLineBreak = -1;
	int origPos = pos;
	int lastspace = -1;
	int curw = 1;
	int chr;
	int oldID = getCurID();
	int code = (_vm->_game.heversion >= 80) ? 127 : 64;

	int strLength = _vm->resStrLen(str);

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

		if (_vm->_useCJKMode) {
			if (_vm->_game.platform == Common::kPlatformFMTowns) {
				if (checkSJISCode(chr))
					// This strange character conversion is the exact way the original does it here.
					// This is the only way to get an accurate text formatting in the MI1 intro.
					chr = (int8)str[pos++] | (chr << 8);
				curw += getCharWidth(chr);
			} else if (chr & 0x80) {
				pos++;
				curw += _vm->_2byteWidth;
				// Original keeps glyph width and character dimensions separately
				if (_vm->_language == Common::KO_KOR || _vm->_language == Common::ZH_TWN) {
					curw++;
				}
			} else if (chr != _vm->_newLineCharacter) {
				curw += getCharWidth(chr);
			}

			if (_vm->isScummvmKorTarget() && !_center) {
				// Break Korean words at any character
				// Used in Korean fan translated games
				if (chr & 0x80) {
					if (checkKSCode(chr, str[pos - 1])
					    && !(pos - 4 >= origPos && str[pos - 3] == '`' && str[pos - 4] == ' ')  // prevents hanging quotation mark at the end of line
					    && !(pos - 4 >= origPos && str[pos - 3] == '\'' && str[pos - 4] == ' ') // prevents hanging single quotation mark at the end of line
					    && !(pos - 3 >= origPos && str[pos - 3] == '('))  // prevents hanging parenthesis at the end of line
						lastKoreanLineBreak = pos - 2;
				} else {
					if (chr == '(' && pos - 3 >= origPos && checkKSCode(str[pos - 3], str[pos - 2]))
						lastKoreanLineBreak = pos - 1;
				}
			}
		} else {
			curw += getCharWidth(chr);
		}
		if (lastspace == -1) {
			if (!_vm->isScummvmKorTarget() || lastKoreanLineBreak == -1) {
				continue;
			}
		}
		if (curw > maxwidth) {
			if (!_vm->isScummvmKorTarget()) {
				str[lastspace] = 0xD;
				curw = 1;
				pos = lastspace + 1;
				lastspace = -1;
			} else {
				// Handle Korean line break mode (break Korean words at any character)
				// Used in Korean fan translated games
				if (lastspace >= lastKoreanLineBreak) {
					str[lastspace] = 0xD;
					curw = 1;
					pos = lastspace + 1;
					lastspace = -1;
					lastKoreanLineBreak = -1;
				} else {
					byte *breakPtr = str + lastKoreanLineBreak;
					memmove(breakPtr + 1, breakPtr, strLength - lastKoreanLineBreak + 1);
					str[lastKoreanLineBreak] = 0xD;
					curw = 1;
					pos = lastKoreanLineBreak + 1;
					lastspace = -1;
					lastKoreanLineBreak = -1;
				}
			}
		}
	}

	setCurID(oldID);
}

int CharsetRendererV3::getCharWidth(uint16 chr) const {
	int spacing = 0;

	if (_vm->_useCJKMode && (chr & 0x80))
		spacing = _vm->_2byteWidth / 2;

	if (!spacing)
		spacing = *(_widthTable + chr);

	return spacing;
}

void CharsetRendererPC::enableShadow(bool enable) {
	_shadowColor = 0;
	_enableShadow = enable;
	_shadowType = kNormalShadowType;
}

void CharsetRendererPC::drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height) {
	if (_vm->_useCJKMode && _vm->isScummvmKorTarget()) {
		drawBits1Kor(dest, x, y, src, drawTop, width, height);
		return;
	}

	byte *dst = (byte *)dest.getBasePtr(x, y);
	byte bits = 0;
	uint8 col = _color;
	int pitch = dest.pitch - width * dest.format.bytesPerPixel;
	byte *dst2 = dst + dest.pitch;

	for (y = 0; y < height && y + drawTop < dest.h; y++) {
		for (x = 0; x < width; x++) {
			if ((x % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask(x % 8)) && y + drawTop >= 0) {
				if (_enableShadow) {
					if (_shadowType == kNormalShadowType)
						dst[1] = dst2[0] = dst2[1] = _shadowColor;
					else if (_shadowType == kHorizontalShadowType)
						dst[1] = _shadowColor;
				}
				dst[0] = col;
			}
			dst += dest.format.bytesPerPixel;
			dst2 += dest.format.bytesPerPixel;
		}

		dst += pitch;
		dst2 += pitch;
	}
}

void CharsetRendererPC::drawBits1Kor(Graphics::Surface &dest, int x1, int y1, const byte *src, int drawTop, int width, int height) {
	byte *dst = (byte *)dest.getBasePtr(x1, y1);

	int y, x;
	byte bits = 0;

	// HACK: Since Korean fonts don't have shadow/stroke information,
	//	   we use NUT-Renderer-like shadow drawing method.

	int offsetX[14] = {-2, -2, -2, -1, 0, -1, 0, 1, -1, 1, -1, 0, 1, 0};
	int offsetY[14] = {0, 1, 2, 2, 2, -1, -1, -1, 0, 0, 1, 1, 1, 0};
	int cTable[14] = {_shadowColor, _shadowColor, _shadowColor,
						_shadowColor, _shadowColor, _shadowColor, _shadowColor,
						_shadowColor, _shadowColor, _shadowColor, _shadowColor,
						_shadowColor, _shadowColor, _color};
	int i = 0;

	switch (_vm->_2byteShadow) {
	case 1: // No shadow
		i = 13;
		break;
	case 2: // SE direction shadow
		i = 12;
		break;
	case 3: // Stroke & SW direction shadow ("Monkey2", "Indy4")
		i = 0;
		break;
	default: // Stroke
		i = 5;
	}

	const byte *origSrc = src;
	byte *origDst = dst;

	for (; i < 14; i++) {
		src = origSrc;
		dst = origDst;

		for (y = 0; y < height && y + drawTop + offsetY[i] < dest.h; y++) {
			for (x = 0; x < width && x + x1 + offsetX[i] < dest.w; x++) {
				if ((x % 8) == 0)
					bits = *src++;
				if ((bits & revBitMask(x % 8)) && y + drawTop + offsetY[i] >= 0 && x + x1 + offsetX[i] >= 0) {
					*(dst + (dest.pitch * offsetY[i]) + offsetX[i]) = cTable[i];
				}
				dst++;
			}

			dst += dest.pitch - width;
		}
	}
}

int CharsetRendererV3::getDrawWidthIntern(uint16 chr) {
	return getCharWidth(chr);
}

int CharsetRendererV3::getDrawHeightIntern(uint16) {
	return 8;
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

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_vm->_game.platform == Common::kPlatformFMTowns) {
		_color = (_color & 0x0f) | ((_color & 0x0f) << 4);
		if (_color == 0)
			_color = 0x88;
	}
#endif

	enableShadow(useShadow);

	translateColor();
}

#ifdef USE_RGB_COLOR
void CharsetRendererPCE::setColor(byte color) {
	_vm->setPCETextPalette(color);
	_color = 15;

	enableShadow(true);
}
#endif

void CharsetRendererV3::printChar(int chr, bool ignoreCharsetMask) {
	// WORKAROUND for bug #2703: Indy3 Mac does not show black
	// characters (such as in the grail diary) if ignoreCharsetMask
	// is true. See also bug #8759.
	if (_vm->_game.id == GID_INDY3 && _vm->_game.platform == Common::kPlatformMacintosh && _color == 0)
		ignoreCharsetMask = false;

	// Indy3 / Zak256 / Loom
	int width, height, origWidth = 0, origHeight;
	VirtScreen *vs;
	const byte *charPtr;
	int is2byte = (chr >= 256 && _vm->_useCJKMode) ? 1 : 0;

	assertRange(0, _curId, _vm->_numCharsets - 1, "charset");

	if ((vs = _vm->findVirtScreen(_top)) == nullptr) {
		warning("findVirtScreen(%d) failed, therefore printChar cannot print '%c'", _top, chr);
		return;
	}

	if (chr == '@')
		return;

	if (_vm->isScummvmKorTarget()) {
		if (is2byte) {
			charPtr = _vm->get2byteCharPtr(chr);
			width = _vm->_2byteWidth;
			height = _vm->_2byteHeight;
		} else {
			charPtr = _fontPtr + chr * 8;
			width = getDrawWidthIntern(chr);
			height = getDrawHeightIntern(chr);
		}
	} else {
		charPtr = (_vm->_useCJKMode && chr > 127) ? _vm->get2byteCharPtr(chr) : _fontPtr + chr * 8;
		width = getDrawWidthIntern(chr);
		height = getDrawHeightIntern(chr);
	}
	setDrawCharIntern(chr);

	origWidth = width;
	origHeight = height;

	// Clip at the right side (to avoid drawing "outside" the screen bounds).
	if (_left + origWidth > _right + 1)
		return;

	if (_enableShadow) {
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

	if ((ignoreCharsetMask || !vs->hasTwoBuffers)
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
		&& (_vm->_game.platform != Common::kPlatformFMTowns)
#endif
		)
		drawBits1(*vs, _left + vs->xstart, drawTop, charPtr, drawTop, origWidth, origHeight);
	else
		drawBits1(_vm->_textSurface, _left * _vm->_textSurfaceMultiplier, _top * _vm->_textSurfaceMultiplier, charPtr, drawTop, origWidth, origHeight);

	if (is2byte) {
		origWidth /= _vm->_textSurfaceMultiplier;
		height /= _vm->_textSurfaceMultiplier;
	}

	if (_str.left > _left)
		_str.left = _left;

	_left += origWidth;

	if (_str.right < _left) {
		_str.right = _left;
		if (_enableShadow)
			_str.right++;
	}

	if (_str.bottom < _top + height)
		_str.bottom = _top + height;
}

void CharsetRendererV3::drawChar(int chr, Graphics::Surface &s, int x, int y) {
	const byte *charPtr;
	int width;
	int height;
	int is2byte = (chr > 0xff && _vm->_useCJKMode) ? 1 : 0;

	if (_vm->isScummvmKorTarget()) {
		if (is2byte) {
			charPtr = _vm->get2byteCharPtr(chr);
			width = _vm->_2byteWidth;
			height = _vm->_2byteHeight;
		} else {
			charPtr = _fontPtr + chr * 8;
			width = getDrawWidthIntern(chr);
			height = getDrawHeightIntern(chr);
		}
	} else {
		charPtr = (_vm->_useCJKMode && chr > 127) ? _vm->get2byteCharPtr(chr) : _fontPtr + chr * 8;
		width = getDrawWidthIntern(chr);
		height = getDrawHeightIntern(chr);
	}
	setDrawCharIntern(chr);
	drawBits1(s, x, y, charPtr, y, width, height);
}

void CharsetRenderer::translateColor() {
	// Don't do anything for v1 and v2 CGA and Hercules modes
	// here (and v0 doesn't have any of these modes).
	if (_vm->_game.version < 3)
		return;

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

void CharsetRenderer::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncAsByte(_curId, VER(73), VER(73));
	ser.syncAsSint32LE(_curId, VER(74));
	ser.syncAsByte(_color, VER(73));

	if (ser.isLoading()) {
		setCurID(_curId);
		setColor(_color);
	}
}

void CharsetRendererClassic::printChar(int chr, bool ignoreCharsetMask) {
	VirtScreen *vs;
	bool is2byte = (chr >= 256 && _vm->_useCJKMode);

	assertRange(1, _curId, _vm->_numCharsets - 1, "charset");

	if ((vs = _vm->findVirtScreen(_top)) == nullptr && (vs = _vm->findVirtScreen(_top + getFontHeight())) == nullptr)
		return;

	if (chr == '@')
		return;

	translateColor();

	_vm->_charsetColorMap[1] = _color;
	if (_vm->isScummvmKorTarget() && is2byte) {
		enableShadow(true);
		_charPtr = _vm->get2byteCharPtr(chr);
		_width = _vm->_2byteWidth;
		_height = _vm->_2byteHeight;
		_offsX = _offsY = 0;
	} else {
		if (!prepareDraw(chr))
			return;
	}

	if (_vm->isScummvmKorTarget()) {
		_origWidth = _width;
		_origHeight = _height;
	}

	if (_firstChar) {
		_str.left = 0;
		_str.top = 0;
		_str.right = 0;
		_str.bottom = 0;
	}

	_top += _offsY;
	_left += _offsX;

	if (_left + _origWidth > _right + 1 || _left < 0) {
		_left += _origWidth;
		_top -= _offsY;
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

	_vm->markRectAsDirty(vs->number, _left, _left + _width, drawTop, drawTop + _height);

	// This check for kPlatformFMTowns and kMainVirtScreen is at least required for the chat with
	// the navigator's head in front of the ghost ship in Monkey Island 1
	if (!ignoreCharsetMask || (_vm->_game.platform == Common::kPlatformFMTowns && vs->number == kMainVirtScreen)) {
		_hasMask = true;
		_textScreenID = vs->number;
	}

	// We need to know the virtual screen we draw on for Indy 4 Amiga, since
	// it selects the palette map according to this. We furthermore can not
	// use _textScreenID here, since that will cause inventory graphics
	// glitches.
	if (_vm->_game.platform == Common::kPlatformAmiga && _vm->_game.id == GID_INDY4)
		_drawScreen = vs->number;

	printCharIntern(is2byte, _charPtr, _origWidth, _origHeight, _width, _height, vs, ignoreCharsetMask);

	// Original keeps glyph width and character dimensions separately
	if ((_vm->_language == Common::ZH_TWN || _vm->_language == Common::KO_KOR) && is2byte)
		_origWidth++;

	_left += _origWidth;

	if (_str.right < _left) {
		_str.right = _left;
		if (_vm->_game.platform != Common::kPlatformFMTowns && _enableShadow)
			_str.right++;
	}

	if (_str.bottom < _top + _origHeight)
		_str.bottom = _top + _origHeight;

	_top -= _offsY;
}

void CharsetRendererClassic::printCharIntern(bool is2byte, const byte *charPtr, int origWidth, int origHeight, int width, int height, VirtScreen *vs, bool ignoreCharsetMask) {
	byte *dstPtr;
	byte *back = nullptr;
	int drawTop = _top - vs->topline;

	if ((_vm->_game.heversion >= 71 && _bytesPerPixel >= 8) || (_vm->_game.heversion >= 90 && _bytesPerPixel == 0)) {
#ifdef ENABLE_HE
		if (ignoreCharsetMask || !vs->hasTwoBuffers) {
			dstPtr = vs->getPixels(0, 0);
		} else {
			dstPtr = (byte *)_vm->_textSurface.getPixels();
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
		if (ignoreCharsetMask || !vs->hasTwoBuffers) {
			dstSurface = *vs;
			dstPtr = vs->getPixels(_left, drawTop);
		} else {
			dstSurface = _vm->_textSurface;
			dstPtr = (byte *)_vm->_textSurface.getBasePtr(_left * _vm->_textSurfaceMultiplier, (_top - _vm->_screenTop) * _vm->_textSurfaceMultiplier);
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

		if (is2byte && _vm->_game.platform != Common::kPlatformFMTowns)
			drawBits1(dstSurface, (ignoreCharsetMask || !vs->hasTwoBuffers) ? _left + vs->xstart : _left, drawTop, charPtr, drawTop, origWidth, origHeight);
		else
			drawBitsN(dstSurface, dstPtr, charPtr, *_fontPtr, drawTop, origWidth, origHeight);

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

bool CharsetRendererClassic::prepareDraw(uint16 chr) {
	bool is2byte = (chr >= 256 && _vm->_useCJKMode);
	if (is2byte) {
		_charPtr = _vm->get2byteCharPtr(chr);
		_width = _origWidth = _vm->_2byteWidth;
		_height = _origHeight = _vm->_2byteHeight;
		_offsX = _offsY = 0;

		if (_enableShadow) {
			_width++;
			_height++;
		}

		return true;
	} else {
		enableShadow(false);
	}

	uint32 charOffs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
	assert(charOffs < 0x14000);
	if (!charOffs)
		return false;
	_charPtr = _fontPtr + charOffs;

	_width = _origWidth = _charPtr[0];
	_height = _origHeight = _charPtr[1];

	if (_disableOffsX) {
		_offsX = 0;
	} else {
		_offsX = (signed char)_charPtr[2];
	}

	_offsY = (signed char)_charPtr[3];

	_charPtr += 4;	// Skip over char header
	return true;
}

void CharsetRendererClassic::drawChar(int chr, Graphics::Surface &s, int x, int y) {
	if (!prepareDraw(chr))
		return;

	byte *dst = (byte *)s.getBasePtr(x, y);

	bool is2byte = (_vm->_useCJKMode && chr >= 256);
	if (is2byte)
		drawBits1(s, x, y, _charPtr, y, _width, _height);
	else
		drawBitsN(s, dst, _charPtr, *_fontPtr, y, _width, _height);
}

void CharsetRendererClassic::drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height) {
	int y, x;
	int color;
	byte numbits, bits;

	int pitch = s.pitch - width;

	assert(bpp == 1 || bpp == 2 || bpp == 4 || bpp == 8);
	bits = *src++;
	numbits = 8;
	byte *cmap = _vm->_charsetColorMap;

	// Indy4 Amiga always uses the room or verb palette map to match colors to
	// the currently setup palette, thus we need to select it over here too.
	// Done like the original interpreter.
	byte *amigaMap = nullptr;
	if (_vm->_game.platform == Common::kPlatformAmiga && _vm->_game.id == GID_INDY4) {
		if (_drawScreen == kVerbVirtScreen)
			amigaMap = _vm->_verbPalette;
		else
			amigaMap = _vm->_roomPalette;
	}

	for (y = 0; y < height && y + drawTop < s.h; y++) {
		for (x = 0; x < width; x++) {
			color = (bits >> (8 - bpp)) & 0xFF;

			if (color && y + drawTop >= 0) {
				if (amigaMap)
					*dst = amigaMap[cmap[color]];
				else
					*dst = cmap[color];
			}
			dst++;
			bits <<= bpp;
			numbits -= bpp;
			if (numbits == 0) {
				bits = *src++;
				numbits = 8;
			}
		}
		dst += pitch;
	}
}

CharsetRendererTownsV3::CharsetRendererTownsV3(ScummEngine *vm) : CharsetRendererV3(vm), _sjisCurChar(0) {
}

int CharsetRendererTownsV3::getCharWidth(uint16 chr) const {
	if (_vm->isScummvmKorTarget()) {
		return CharsetRendererV3::getCharWidth(chr);
	}

	int spacing = 0;

	if (_vm->_useCJKMode) {
		if (chr >= 256)
			spacing = 8;
		else if (chr >= 128)
			spacing = 4;
	}

	if (!spacing)
		spacing = *(_widthTable + chr);

	return spacing;
}

int CharsetRendererTownsV3::getFontHeight() const {
	if (_vm->isScummvmKorTarget()) {
		return CharsetRendererV3::getFontHeight();
	}

	return _vm->_useCJKMode ? 8 : _fontHeight;
}

void CharsetRendererTownsV3::enableShadow(bool enable) {
	if (_vm->isScummvmKorTarget()) {
		CharsetRendererV3::enableShadow(enable);
		return;
	}

	_shadowColor = 8;
	_enableShadow = enable;

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	_shadowColor = 0x88;
#ifdef USE_RGB_COLOR
	if (_vm->_cjkFont)
		_vm->_cjkFont->setDrawingMode(enable ? Graphics::FontSJIS::kFMTownsShadowMode : Graphics::FontSJIS::kDefaultMode);
#endif
#endif
}

void CharsetRendererTownsV3::drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height) {
	if (_vm->isScummvmKorTarget()) {
		CharsetRendererV3::drawBits1(dest, x, y, src, drawTop, width, height);
		return;
	}

	if (y + height > dest.h)
		error("Trying to draw below screen boundaries");

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
#ifdef USE_RGB_COLOR
	if (_sjisCurChar) {
		assert(_vm->_cjkFont);
		_vm->_cjkFont->drawChar(dest, _sjisCurChar, x, y, _color, _shadowColor);
		return;
	}
#endif
	bool scale2x = ((&dest == &_vm->_textSurface) && (_vm->_textSurfaceMultiplier == 2) && !(_sjisCurChar >= 256 && _vm->_useCJKMode));
#endif

	byte bits = 0;
	uint8 col = _color;
	int pitch = dest.pitch - width * dest.format.bytesPerPixel;
	byte *dst = (byte *)dest.getBasePtr(x, y);
	byte *dst2 = dst + dest.pitch;

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	byte *dst3 = dst2;
	byte *dst4 = dst2;
	if (scale2x) {
		dst3 = dst2 + dest.pitch;
		dst4 = dst3 + dest.pitch;
		pitch <<= 1;
	}
#endif

	for (y = 0; y < height && y + drawTop < dest.h; y++) {
		for (x = 0; x < width; x++) {
			if ((x % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask(x % 8)) && y + drawTop >= 0) {
				if (dest.format.bytesPerPixel == 2) {
					if (_enableShadow) {
						WRITE_UINT16(dst + 2, _vm->_16BitPalette[_shadowColor]);
						WRITE_UINT16(dst + dest.pitch, _vm->_16BitPalette[_shadowColor]);
					}
					WRITE_UINT16(dst, _vm->_16BitPalette[_color]);
				} else {
					if (_enableShadow) {
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
						if (scale2x) {
							dst[2] = dst[3] = dst2[2] = dst2[3] = _shadowColor;
							dst3[0] = dst4[0] = dst3[1] = dst4[1] = _shadowColor;
						} else
#endif
						{
							dst[1] = dst2[0] = _shadowColor;
						}
					}
					dst[0] = col;

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
					if (scale2x)
						dst[1] = dst2[0] = dst2[1] = col;
#endif
				}
			}
			dst += dest.format.bytesPerPixel;
			dst2 += dest.format.bytesPerPixel;
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
			if (scale2x) {
				dst++;
				dst2++;
				dst3 += 2;
				dst4 += 2;
			}
#endif
		}

		dst += pitch;
		dst2 += pitch;
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
		dst3 += pitch;
		dst4 += pitch;
#endif
	}
}
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
int CharsetRendererTownsV3::getDrawWidthIntern(uint16 chr) {
	if (_vm->isScummvmKorTarget()) {
		return CharsetRendererV3::getDrawWidthIntern(chr);
	}

#ifdef USE_RGB_COLOR
	if (_vm->_useCJKMode && chr > 127) {
		assert(_vm->_cjkFont);
		return _vm->_cjkFont->getCharWidth(chr);
	}
#endif
	return CharsetRendererV3::getDrawWidthIntern(chr);
}

int CharsetRendererTownsV3::getDrawHeightIntern(uint16 chr) {
	if (_vm->isScummvmKorTarget()) {
		return CharsetRendererV3::getDrawHeightIntern(chr);
	}

#ifdef USE_RGB_COLOR
	if (_vm->_useCJKMode && chr > 127) {
		assert(_vm->_cjkFont);
		return _vm->_cjkFont->getFontHeight();
	}
#endif
	return CharsetRendererV3::getDrawHeightIntern(chr);
}

void CharsetRendererTownsV3::setDrawCharIntern(uint16 chr) {
	_sjisCurChar = (_vm->_useCJKMode && chr > 127) ? chr : 0;
}
#endif

#ifdef USE_RGB_COLOR
void CharsetRendererPCE::drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height) {
	byte *dst = (byte *)dest.getBasePtr(x, y);
	if (_sjisCurChar) {
		assert(_vm->_cjkFont);
		uint16 col1 = _color;
		uint16 col2 = _shadowColor;

		if (dest.format.bytesPerPixel == 2) {
			col1 = _vm->_16BitPalette[col1];
			col2 = _vm->_16BitPalette[col2];
		}

		_vm->_cjkFont->drawChar(dst, _sjisCurChar, dest.pitch, dest.format.bytesPerPixel, col1, col2, -1, -1);
		return;
	}

	byte bits = 0;

	for (y = 0; y < height && y + drawTop < dest.h; y++) {
		int bitCount = 0;
		for (x = 0; x < width; x++) {
			if ((bitCount % 8) == 0)
				bits = *src++;
			if ((bits & revBitMask(bitCount % 8)) && y + drawTop >= 0) {
				if (dest.format.bytesPerPixel == 2) {
					if (_enableShadow)
						WRITE_UINT16(dst + dest.pitch + 2, _vm->_16BitPalette[_shadowColor]);
					WRITE_UINT16(dst, _vm->_16BitPalette[_color]);
				} else {
					if (_enableShadow)
						*(dst + dest.pitch + 1) = _shadowColor;
					*dst = _color;
				}
			}
			dst += dest.format.bytesPerPixel;
			bitCount++;
		}

		dst += dest.pitch - width * dest.format.bytesPerPixel;
	}
}

int CharsetRendererPCE::getDrawWidthIntern(uint16 chr) {
	if (_vm->_useCJKMode && chr > 127)
		return _vm->_2byteWidth;
	return CharsetRendererV3::getDrawWidthIntern(chr);
}

int CharsetRendererPCE::getDrawHeightIntern(uint16 chr) {
	if (_vm->_useCJKMode && chr > 127)
		return _vm->_2byteHeight;
	return CharsetRendererV3::getDrawHeightIntern(chr);
}

void CharsetRendererPCE::setDrawCharIntern(uint16 chr) {
	_sjisCurChar = (_vm->_useCJKMode && chr > 127) ? chr : 0;
}
#endif

CharsetRendererMac::CharsetRendererMac(ScummEngine *vm, const Common::String &fontFile)
	 : CharsetRendererCommon(vm) {

	// The original Macintosh interpreter didn't use the correct spacing
	// between characters for some of the text, e.g. the Grail Diary. This
	// appears to have been because of rounding errors, and was apparently
	// fixed in Loom. Enabling this allows ScummVM to draw the text more
	// correctly, at the cost of not matching the original quite as well.
	// (At the time of writing, there are still cases, at least in Loom,
	// where text isn't correctly positioned.)

	_correctFontSpacing = _vm->_game.id == GID_LOOM || _vm->_enableEnhancements;
	_pad = false;
	_glyphSurface = nullptr;

	// Indy 3 provides an "Indy" font in two sizes, 9 and 12, which are
	// used for the text boxes. The smaller font can be used for a
	// headline. The rest of the Mac GUI seems to use a system font, but
	// that is not implemented.

	// As far as I can tell, Loom uses only font size 13 for in-game text.
	// The font is also provided in sizes 9 and 12, and it's possible that
	// 12 is used for system messages, e.g. the original pause dialog. We
	// don't support that.
	//
	// I have no idea what size 9 is used for. Possibly the original About
	// dialog?
	//
	// As far as I can tell, the game does not use anything fancy, like
	// different styles, and the font does not appear to have a kerning
	// table.
	//
	// Special characters:
	//
	// 16-23 are the note names c through c'.
	// 60 is an upside-down note, i.e. the one used for c'.
	// 95 is a used for the rest of the notes.

	Common::MacResManager resource;
	resource.open(fontFile);

	Common::String fontFamilyName = (_vm->_game.id == GID_LOOM) ? "Loom" : "Indy";

	Common::SeekableReadStream *fond = resource.getResource(MKTAG('F', 'O', 'N', 'D'), fontFamilyName);

	if (!fond)
		return;

	Graphics::MacFontFamily fontFamily;
	if (!fontFamily.load(*fond)) {
		delete fond;
		return;
	}

	Common::Array<Graphics::MacFontFamily::AsscEntry> *assoc = fontFamily.getAssocTable();
	for (uint i = 0; i < assoc->size(); i++) {
		int fontId = -1;
		int fontSize = (*assoc)[i]._fontSize;

		if (_vm->_game.id == GID_INDY3) {
			if (fontSize == 9)
				fontId = 1;
			else if (fontSize == 12)
				fontId = 0;
		} else {
			if (fontSize == 13)
				fontId = 0;
		}
		if (fontId != -1) {
			Common::SeekableReadStream *font = resource.getResource(MKTAG('F', 'O', 'N', 'T'), (*assoc)[i]._fontID);
			_macFonts[fontId].loadFont(*font, &fontFamily, fontSize, 0);
			delete font;
		}
	}

	delete fond;

	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		int numFonts = (_vm->_game.id == GID_INDY3) ? 2 : 1;
		int maxHeight = -1;
		int maxWidth = -1;

		for (int i = 0; i < numFonts; i++) {
			maxHeight = MAX(maxHeight, _macFonts[i].getFontHeight());
			maxWidth = MAX(maxWidth, _macFonts[i].getMaxCharWidth());
		}

		_glyphSurface = new Graphics::Surface();
		_glyphSurface->create(maxWidth, maxHeight, Graphics::PixelFormat::createFormatCLUT8());
	}
}

CharsetRendererMac::~CharsetRendererMac() {
	if (_glyphSurface) {
		_glyphSurface->free();
		delete _glyphSurface;
	}
}

void CharsetRendererMac::setCurID(int32 id) {
	if  (id == -1)
		return;

	// Indiana Jones and the Last Crusade uses font id 1 in a number of
	// places. In the DOS version, this is a bolder font than font 0, but
	// by the looks of it the Mac version uses the same font for both
	// cases. In ScummVM, we match id 0 and 1 to font 0 and id 2 (which is
	// only used to print the text box caption) to font 1.
	if (_vm->_game.id == GID_INDY3) {
		if (id == 1) {
			id = 0;
		} else if (id == 2) {
			id = 1;
		}
	}

	int maxId = (_vm->_game.id == GID_LOOM) ? 0 : 1;

	if (id > maxId) {
		warning("CharsetRendererMac::setCurID(%d) - invalid charset", id);
		id = 0;
	}

	_curId = id;
}

int CharsetRendererMac::getStringWidth(int arg, const byte *text) {
	int pos = 0;
	int width = 0;
	int chr;

	while ((chr = text[pos++]) != 0) {
		// The only control codes I've seen in use are line breaks in
		// Loom. In Indy 3, I haven't seen anything at all like it.
		if (chr == 255) {
			chr = text[pos++];
			if (chr == 1) // 'Newline'
				break;
			warning("getStringWidth: Unexpected escape sequence %d", chr);
		} else {
			width += getDrawWidthIntern(chr);
		}
	}

	return width / 2;
}

int CharsetRendererMac::getDrawWidthIntern(uint16 chr) const {
	return _macFonts[_curId].getCharWidth(chr);
}

// HACK: Usually, we want the approximate width and height in the unscaled
//       graphics resolution. But for font 1 in Indiana Jones and the Last
//       crusade we want the actual dimensions for drawing the text boxes.

int CharsetRendererMac::getFontHeight() const {
	int height = _macFonts[_curId].getFontHeight();

        // If we ever need the height for font 1 in Last Crusade (we don't at
	// the moment), we need the actual height.
	if (_curId == 0 || _vm->_game.id != GID_INDY3)
		height /= 2;

	return height;
}

int CharsetRendererMac::getCharWidth(uint16 chr) const {
	int width = getDrawWidthIntern(chr);

	// For font 1 in Last Crusade, we want the real width. It is used for
	// text box titles, which are drawn outside the normal font rendering.
	if (_curId == 0 || _vm->_game.id != GID_INDY3)
		width /= 2;

	return width;
}

void CharsetRendererMac::printChar(int chr, bool ignoreCharsetMask) {
	// This function does most of the heavy lifting printing the game
	// text. It's the only function that needs to be able to handle
	// disabled text.

	// If this is the beginning of a line, assume the position will be
	// correct without any padding.

	if (_firstChar || _top != _lastTop) {
		_pad = false;
	}

	VirtScreen *vs;

	if ((vs = _vm->findVirtScreen(_top)) == nullptr) {
		warning("findVirtScreen(%d) failed, therefore printChar cannot print '%c'", _top, chr);
		return;
	}

	if (chr == '@')
		return;

	// Scale up the virtual coordinates to get the high resolution ones.

	int macLeft = 2 * _left;
	int macTop = 2 * _top;

	// The last character ended on an odd X coordinate. This information
	// was lost in the rounding, so we compensate for it here.

	if (_pad) {
		macLeft++;
		_pad = false;
	}

	bool enableShadow = _enableShadow;
	int color = _color;

	// HACK: Notes and their names should always be drawn with a shadow.
	//       Actually, this doesn't quite match the original but I can't
	//       figure out what the original does here. The "c" looks like
	//       it's shadowed in the normal way, but everything else looks
	//       kind-of-but-not-quite outlined instead. Weird.
	//
	//       Even weirder, I've seen screenshots where there is no
	//       shadowing at all. I'll just keep it like this for now,
	//       because it makes the notes stand out a bit better.

	if (_vm->_game.id == GID_LOOM) {
		if ((chr >= 16 && chr <= 23) || chr == 60 || chr == 95) {
			enableShadow = true;
		}
	}

	// HACK: Apparently, note names are never drawn in light gray. Only
	//       white for known notes, and dark gray for unknown ones. This
	//       hack ensures that we won't be left with a mix of white and
	//       light gray note names, because apparently the game never
	//       changes them back to light gray once the draft is done?

	if (_vm->_game.id == GID_LOOM) {
		if (chr >= 16 && chr <= 23 && _color == 7)
			color = 15;
	}

	bool drawToTextBox = (vs->number == kTextVirtScreen && _vm->_game.id == GID_INDY3);

	if (drawToTextBox)
		printCharToTextBox(chr, color, macLeft, macTop);
	else
		printCharInternal(chr, color, enableShadow, macLeft, macTop);

	// HACK: The way we combine high and low resolution graphics means
	//       that sometimes, when a note name is drawn on the distaff, the
	//       note itself gets overdrawn by the low-resolution graphics.
	//
	//       The only workaround I can think of is to force the note to be
	//       redrawn along with its name. It's enough to redraw it on the
	//       text surface. We can assume the correct color is already on
	//       screen.
	//
	//       Note that this will not affect the Practice Mode box, since
	//       this note names are drawn by drawChar(), not printChar().

	if (_vm->_game.id == GID_LOOM) {
		if (chr >= 16 && chr <= 23) {
			int xOffset[] = { 16, 14, 12, 8, 6, 2, 0, 8 };

			int note = (chr == 23) ? 60 : 95;
			printCharInternal(note, -1, enableShadow, macLeft + 18, macTop + xOffset[chr - 16]);
		}
	}

	// Mark the virtual screen as dirty, using downscaled coordinates.

	int left, right, top, bottom, width;

	width = getDrawWidthIntern(chr);

	// HACK: Indiana Jones and the Last Crusade uses incorrect spacing
	// betweeen letters. Note that this incorrect spacing does not extend
	// to the text boxes, nor does it seem to be used when figuring out
	// the width of a string (e.g. to center text on screen). It is,
	// however, used for things like the Grail Diary.

	if (!_correctFontSpacing && !drawToTextBox && (width & 1))
		width++;

	if (enableShadow) {
		left = macLeft / 2;
		right = (macLeft + width + 3) / 2;
		top = macTop / 2;
		bottom = (macTop + _macFonts[_curId].getFontHeight() + 3) / 2;
	} else {
		left = (macLeft + 1) / 2;
		right = (macLeft + width + 1) / 2;
		top = (macTop + 1) / 2;
		bottom = (macTop + _macFonts[_curId].getFontHeight() + 1) / 2;
	}

	if (_firstChar) {
		_str.left = left;
		_str.top = top;
		_str.right = right;
		_str.bottom = top;
		_firstChar = false;
	} else {
		if (_str.left > left)
			_str.left = left;
		if (_str.right < right)
			_str.right = right;
		if (_str.bottom < bottom)
			_str.bottom = bottom;
	}

	if (!drawToTextBox)
		_vm->markRectAsDirty(vs->number, left, right, top - vs->topline, bottom - vs->topline);

	if (!ignoreCharsetMask) {
		_hasMask = true;
		_textScreenID = vs->number;
	}

	// The next character may have to be adjusted to compensate for
	// rounding errors.

	macLeft += width;
	if (macLeft & 1)
		_pad = true;

	_left = macLeft / 2;
	_lastTop = _top;
}

byte CharsetRendererMac::getTextColor() {
	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		// White and black can be rendered as is, and 8 is the color
		// used for disabled text (verbs in Indy 3, notes in Loom).
		// Everything else should be white.

		if (_color == 0 || _color == 15 || _color == 8)
			return _color;
		return 15;
	}
	return _color;
}

byte CharsetRendererMac::getTextShadowColor() {
	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		if (getTextColor() == 0)
			return 15;
		return 0;
	}
	return _shadowColor;
}

void CharsetRendererMac::printCharInternal(int chr, int color, bool shadow, int x, int y) {
	if (_vm->_game.id == GID_LOOM) {
		x++;
		y++;
	}

	if (shadow) {
		byte shadowColor = getTextShadowColor();

		if (_vm->_game.id == GID_LOOM) {
			// Shadowing is a bit of guesswork. It doesn't look
			// like it's using the Mac's built-in form of shadowed
			// text (which, as I recall it, never looked
			// particularly good anyway). This seems to match the
			// original look for normal text.

			_macFonts[_curId].drawChar(&_vm->_textSurface, chr, x + 1, y - 1, 0);
			_macFonts[_curId].drawChar(&_vm->_textSurface, chr, x - 1, y + 1, 0);
			_macFonts[_curId].drawChar(&_vm->_textSurface, chr, x + 2, y + 2, 0);

			if (color != -1) {
				_macFonts[_curId].drawChar(_vm->_macScreen, chr, x + 1, y - 1, shadowColor);
				_macFonts[_curId].drawChar(_vm->_macScreen, chr, x - 1, y + 1, shadowColor);
				_macFonts[_curId].drawChar(_vm->_macScreen, chr, x + 2, y + 2, shadowColor);
			}
		} else {
			// Indy 3 uses simpler shadowing, and doesn't need the
			// "draw only on text surface" hack.

			_macFonts[_curId].drawChar(&_vm->_textSurface, chr, x + 1, y + 1, 0);
			_macFonts[_curId].drawChar(_vm->_macScreen, chr, x + 1, y + 1, shadowColor);
		}
	}

	_macFonts[_curId].drawChar(&_vm->_textSurface, chr, x, y, 0);

	if (color != -1) {
		color = getTextColor();

		if (_vm->_renderMode == Common::kRenderMacintoshBW && color != 0 && color != 15) {
			_glyphSurface->fillRect(Common::Rect(_glyphSurface->w, _glyphSurface->h), 0);
			_macFonts[_curId].drawChar(_glyphSurface, chr, 0, 0, 15);

			byte *src = (byte *)_glyphSurface->getBasePtr(0, 0);
			byte *dst = (byte *)_vm->_macScreen->getBasePtr(x, y);

			for (int h = 0; h < _glyphSurface->h; h++) {
				bool pixel = ((y + h + 1) & 1) == 0;

				for (int w = 0; w < _glyphSurface->w; w++) {
					if (src[w]) {
						if (pixel)
							dst[w] = 15;
						else
							dst[w] = 0;
					}
					pixel = !pixel;
				}
				src += _glyphSurface->pitch;
				dst += _vm->_macScreen->pitch;
			}
		} else {
			_macFonts[_curId].drawChar(_vm->_macScreen, chr, x, y, color);
		}
	}
}

void CharsetRendererMac::printCharToTextBox(int chr, int color, int x, int y) {
	// This function handles printing most of the text in the text boxes
	// in Indiana Jones and the last crusade. In black and white mode, all
	// text is white. Text is never disabled.

	if (_vm->_renderMode == Common::kRenderMacintoshBW)
		color = 15;

	// Since we're working with unscaled coordinates most of the time, the
	// lines of the text box weren't spaced quite as much as in the
	// original. I thought no one would notice, but I was wrong. This is
	// the best way I can think of to fix that.

	if (y > 0)
		y = 17;

	_macFonts[_curId].drawChar(_vm->_macIndy3TextBox, chr, x + 5, y + 11, color);
}

void CharsetRendererMac::drawChar(int chr, Graphics::Surface &s, int x, int y) {
	// This function is used for drawing most of the text outside of what
	// the game scripts request. It's used for the text box captions in
	// Indiana Jones and the Last Crusade, and for the practice mode box
	// in Loom.
	int color = _color;

	if (_vm->_renderMode == Common::kRenderMacintoshBW)
		color = 15;

	_macFonts[_curId].drawChar(&s, chr, x, y, color);
}

void CharsetRendererMac::setColor(byte color) {
	_color = color;
	_enableShadow = false;
	_shadowColor = 0;

	_enableShadow = ((color & 0xF0) != 0);
	// Anything outside the ordinary palette should be fine.
	_shadowColor = 255;
	_color &= 0x0F;
}

#ifdef ENABLE_SCUMM_7_8
CharsetRendererV7::CharsetRendererV7(ScummEngine *vm) : CharsetRendererClassic(vm),
	_spacing(vm->_useCJKMode && vm->_language != Common::JA_JPN ? 1 : 0),
	_direction(vm->_language == Common::HE_ISR ? -1 : 1),
	_newStyle(vm->_useCJKMode) {
}

int CharsetRendererV7::draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) {
	// I am aware of not doing anything with the clipRect here, but I currently see no need to upgrade the old rendering with that.
	const byte *src = _vm->get2byteCharPtr(chr);
	buffer += (y * pitch + x);
	_origWidth = _vm->_2byteWidth;
	_origHeight = _vm->_2byteHeight;
	uint8 bits = 0;
	pitch -= _origWidth;
	while (_origHeight--) {
		for (x = 0; x < _origWidth; ++x) {
			if ((x % 8) == 0)
				bits = *src++;
			if (bits & revBitMask(x % 8)) {
				buffer[0] = col;
				buffer[1] = _shadowColor;
			}
			buffer++;
		}
		buffer += pitch;
	}
	return _origWidth + _spacing;
}

int CharsetRendererV7::drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) {
	if (!prepareDraw(chr))
		return 0;

	_width = getCharWidth(chr);

	if (_direction < 0)
		x -= _width;

	int width = MIN(_origWidth, clipRect.right - x);
	int height = MIN(_origHeight, clipRect.bottom - y);

	_vm->_charsetColorMap[1] = col;
	byte *cmap = _vm->_charsetColorMap;
	const byte *src = _charPtr;
	byte *dst = buffer + (y + _offsY) * pitch + x;
	uint8 bpp = *_fontPtr;
	byte bits = *src++;
	byte numbits = 8;
	pitch -= _origWidth;

	while (height--) {
		for (int dx = x; dx < x + _origWidth; ++dx) {
			byte color = (bits >> (8 - bpp)) & 0xFF;
			if (color && dx >= 0 && dx < x + width && y >= 0)
				*dst = cmap[color];
			dst++;
			bits <<= bpp;
			numbits -= bpp;
			if (numbits == 0) {
				bits = *src++;
				numbits = 8;
			}
		}
		dst += pitch;
		++y;
	}

	return _direction * width;
}


int CharsetRendererV7::getCharWidth(uint16 chr) const {
	if ((chr & 0x80) && _vm->_useCJKMode)
		return _vm->_2byteWidth + _spacing;

	int offs = READ_LE_UINT32(_fontPtr + (chr & 0xFF) * 4 + 4);
	// SCUMM7 does not use the "kerning" from _fontPtr[offs + 2] here (compare CharsetRendererClassic::getCharWidth()
	// to see the difference. Verfied from disasm and comparison with DOSBox (hard to notice, but e. g. the 'a' character
	// used to be too narrow by 1 pixel, so all lines containing that character were slightly off).
	return offs ? _fontPtr[offs] : 0;
}

CharsetRendererNut::CharsetRendererNut(ScummEngine *vm) : CharsetRenderer(vm) {
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

int CharsetRendererNut::setFont(int id) {
	int old = _curId;
	if (id >= 0)
		setCurID(id);
	return old;
}

int CharsetRendererNut::getCharHeight(uint16 chr) const {
	assert(_current);
	return _current->getCharHeight(chr & 0xFF);
}

int CharsetRendererNut::getCharWidth(uint16 chr) const {
	assert(_current);
	return _current->getCharWidth(chr & 0xFF);
}

int CharsetRendererNut::getFontHeight() const {
	assert(_current);
	return _current->getFontHeight();
}

int CharsetRendererNut::draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) {
	assert(_current);
	return _current->draw2byte(buffer, clipRect, x, y, pitch, col, chr);
}

int CharsetRendererNut::drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) {
	assert(_current);
	return _current->drawCharV7(buffer, clipRect, x, y, pitch, col, flags, chr);
}
#endif

void CharsetRendererNES::printChar(int chr, bool ignoreCharsetMask) {
	int width, height, origWidth, origHeight;
	VirtScreen *vs;
	byte *charPtr;

	// Init it here each time since it is cheap and fixes bug with
	// charset after game load
	_trTable = _vm->getResourceAddress(rtCostume, 77) + 2;

	// HACK: how to set it properly?
	if (_top == 0)
		_top = 16;

	if ((vs = _vm->findVirtScreen(_top)) == nullptr)
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

	if (ignoreCharsetMask || !vs->hasTwoBuffers)
		drawBits1(*vs, _left + vs->xstart, drawTop, charPtr, drawTop, origWidth, origHeight);
	else
		drawBits1(_vm->_textSurface, _left, _top, charPtr, drawTop, origWidth, origHeight);

	if (_str.left > _left)
		_str.left = _left;

	_left += origWidth;

	if (_str.right < _left) {
		_str.right = _left;
		if (_enableShadow)
			_str.right++;
	}

	if (_str.bottom < _top + height)
		_str.bottom = _top + height;
}

void CharsetRendererNES::drawChar(int chr, Graphics::Surface &s, int x, int y) {
	byte *charPtr;
	int width, height;

	if (!_trTable)
		_trTable = _vm->getResourceAddress(rtCostume, 77) + 2;

	charPtr = _vm->_NESPatTable[1] + _trTable[chr - 32] * 16;
	width = getCharWidth(chr);
	height = 8;

	drawBits1(s, x, y, charPtr, y, width, height);
}

#ifdef USE_RGB_COLOR
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
CharsetRendererTownsClassic::CharsetRendererTownsClassic(ScummEngine *vm) : CharsetRendererClassic(vm), _sjisCurChar(0) {
	assert(vm->_game.platform == Common::kPlatformFMTowns);
}

int CharsetRendererTownsClassic::getCharWidth(uint16 chr) const {
	int spacing = 0;

	if (_vm->_useCJKMode) {
		if ((chr & 0xff00) == 0xfd00) {
			chr &= 0xff;
		} else if (chr >= 256) {
			spacing = 8;
		} else if (useFontRomCharacter(chr)) {
			spacing = 4;
		}

		if (spacing) {
			if (_vm->_game.id == GID_MONKEY) {
				spacing++;
				if (_curId == 2)
					spacing++;
			} else if (_vm->_game.id != GID_INDY4 && _curId == 1) {
				spacing++;
			}
		}
	}

	if (!spacing) {
		int offs = READ_LE_UINT32(_fontPtr + chr * 4 + 4);
		if (offs)
			spacing = _fontPtr[offs] + (signed char)_fontPtr[offs + 2];
	}

	return spacing;
}

int CharsetRendererTownsClassic::getFontHeight() const {
	static const uint8 sjisFontHeightM1[] = { 0, 8, 9, 8, 9, 8, 9, 0, 0, 0 };
	static const uint8 sjisFontHeightM2[] = { 0, 8, 9, 9, 9, 8, 9, 9, 9, 8 };
	static const uint8 sjisFontHeightI4[] = { 0, 8, 9, 9, 9, 8, 8, 8, 8, 8 };
	const uint8 *htbl = (_vm->_game.id == GID_MONKEY) ? sjisFontHeightM1 : ((_vm->_game.id == GID_INDY4) ? sjisFontHeightI4 : sjisFontHeightM2);
	return _vm->_useCJKMode ? htbl[_curId] : _fontHeight;
}

void CharsetRendererTownsClassic::drawBitsN(const Graphics::Surface&, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height) {
	if (_sjisCurChar) {
		assert(_vm->_cjkFont);
		_vm->_cjkFont->drawChar(_vm->_textSurface, _sjisCurChar, _left * _vm->_textSurfaceMultiplier, (_top - _vm->_screenTop) * _vm->_textSurfaceMultiplier, _vm->_townsCharsetColorMap[1], _shadowColor);
		return;
	}

	bool scale2x = (_vm->_textSurfaceMultiplier == 2);
	dst = (byte *)_vm->_textSurface.getBasePtr(_left * _vm->_textSurfaceMultiplier, (_top - _vm->_screenTop) * _vm->_textSurfaceMultiplier);

	int y, x;
	int color;
	byte numbits, bits;

	int pitch = _vm->_textSurface.pitch - width;

	assert(bpp == 1 || bpp == 2 || bpp == 4 || bpp == 8);
	bits = *src++;
	numbits = 8;
	byte *cmap = _vm->_townsCharsetColorMap;
	byte *dst2 = dst;

	if (scale2x) {
		dst2 += _vm->_textSurface.pitch;
		pitch <<= 1;
	}

	for (y = 0; y < height && y + drawTop < _vm->_textSurface.h; y++) {
		for (x = 0; x < width; x++) {
			color = (bits >> (8 - bpp)) & 0xFF;

			if (color && y + drawTop >= 0) {
				*dst = cmap[color];
				if (scale2x)
					dst[1] = dst2[0] = dst2[1] = dst[0];
			}
			dst++;

			if (scale2x) {
				dst++;
				dst2 += 2;
			}

			bits <<= bpp;
			numbits -= bpp;
			if (numbits == 0) {
				bits = *src++;
				numbits = 8;
			}
		}
		dst += pitch;
		dst2 += pitch;
	}
}

bool CharsetRendererTownsClassic::prepareDraw(uint16 chr) {
	processCharsetColors();
	bool noSjis = false;

	if (_vm->_useCJKMode) {
		if ((chr & 0x00ff) == 0x00fd) {
			chr >>= 8;
			noSjis = true;
		}
	}

	if (useFontRomCharacter(chr) && !noSjis) {
		setupShadowMode();
		_charPtr = nullptr;
		_sjisCurChar = chr;

		_width = getCharWidth(chr);
		// For whatever reason MI1 uses a different font width
		// for alignment calculation and for drawing when
		// charset 2 is active. This fixes some subtle glitches.
		if (_vm->_game.id == GID_MONKEY && _curId == 2)
			_width--;
		_origWidth = _width;

		_origHeight = _height = getFontHeight();
		_offsX = _offsY = 0;
	} else if (_vm->_useCJKMode && (chr >= 128) && !noSjis) {
		setupShadowMode();
		_origWidth = _width = _vm->_2byteWidth;
		_origHeight = _height = _vm->_2byteHeight;
		_charPtr = _vm->get2byteCharPtr(chr);
		_offsX = _offsY = 0;
		if (_enableShadow) {
			_width++;
			_height++;
		}
	} else {
		_sjisCurChar = 0;
		return CharsetRendererClassic::prepareDraw(chr);
	}
	return true;
}

void CharsetRendererTownsClassic::setupShadowMode() {
	_enableShadow = true;
	_shadowColor = _vm->_townsCharsetColorMap[0];
	assert(_vm->_cjkFont);

	if (((_vm->_game.id == GID_MONKEY) && (_curId == 2 || _curId == 4 || _curId == 6)) ||
		((_vm->_game.id == GID_MONKEY2) && (_curId != 1 && _curId != 5 && _curId != 9)) ||
		((_vm->_game.id == GID_INDY4) && (_curId == 2 || _curId == 3 || _curId == 4))) {
			_vm->_cjkFont->setDrawingMode(Graphics::FontSJIS::kOutlineMode);
	} else {
		_vm->_cjkFont->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
	}

	_vm->_cjkFont->toggleFlippedMode((_vm->_game.id == GID_MONKEY || _vm->_game.id == GID_MONKEY2) && _curId == 3);
}

bool CharsetRendererTownsClassic::useFontRomCharacter(uint16 chr) const {
	if (!_vm->_useCJKMode)
		return false;

	// Some SCUMM 5 games contain hard coded logic to determine whether to use
	// the SCUMM fonts or the FM-Towns font rom to draw a character. For the other
	// games we will simply check for a character greater 127.
	if (chr < 128) {
		if (((_vm->_game.id == GID_MONKEY2 && _curId != 0) || (_vm->_game.id == GID_INDY4 && _curId != 3)) && (chr > 31 && chr != 94 && chr != 95 && chr != 126 && chr != 127))
			return true;
		return false;
	}
	return true;
}

void CharsetRendererTownsClassic::processCharsetColors() {
	for (int i = 0; i < (1 << _bytesPerPixel); i++) {
		uint8 c = _vm->_charsetColorMap[i];

		if (c > 16) {
			uint8 t = (_vm->_currentPalette[c * 3] < 32) ? 4 : 12;
			t |= ((_vm->_currentPalette[c * 3 + 1] < 32) ? 2 : 10);
			t |= ((_vm->_currentPalette[c * 3 + 2] < 32) ? 1 : 9);
			c = t;
		}

		if (c == 0)
			c = _vm->_townsOverrideShadowColor;

		c = ((c & 0x0f) << 4) | (c & 0x0f);
		_vm->_townsCharsetColorMap[i] = c;
	}
}
#endif
#endif

void CharsetRendererNES::drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height) {
	byte *dst = (byte *)dest.getBasePtr(x, y);
	for (int i = 0; i < 8; i++) {
		byte c0 = src[i];
		byte c1 = src[i + 8];
		for (int j = 0; j < 8; j++)
			dst[j] = _vm->_NESPalette[0][((c0 >> (7 - j)) & 1) | (((c1 >> (7 - j)) & 1) << 1) |
			(_color ? 12 : 8)];
		dst += dest.pitch;
	}
}

} // End of namespace Scumm
