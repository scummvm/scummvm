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

// Font management and font drawing module

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/font.h"
#include "saga/render.h"

#include "graphics/sjis.h"
#include "common/unicode-bidi.h"

#include "saga/ite8.h"
#include "saga/small8.h"

namespace Saga {

static const GameFontDescription ITEDEMO_GameFonts[]    = { {0}, {1} };
static const GameFontDescription ITEWINDEMO_GameFonts[] = { {2}, {0} };
static const GameFontDescription ITE_GameFonts[]        = { {2}, {0}, {1} };
static const GameFontDescription IHNMDEMO_GameFonts[]   = { {2}, {3}, {4} };
// Font 6 is kIHNMFont8, font 8 is kIHNMMainFont
static const GameFontDescription IHNMCD_GameFonts[]     = { {2}, {3}, {4}, {5}, {6}, {7}, {8} };
// Resource 2 is a CJK font. Resource 3 looks like some image. 4 to 8 are single-byte
// fonts (not really useful)
static const GameFontDescription IHNMZH_GameFonts[]     = { {2}, {4}, {5}, {6}, {7}, {8} };

static struct {
	const GameFontDescription *list;
	int count;
} FontLists[FONTLIST_MAX] = {
	/* FONTLIST_NONE */         { nullptr,              0                               },
	/* FONTLIST_ITE */          { ITE_GameFonts,        ARRAYSIZE(ITE_GameFonts)        },
	/* FONTLIST_ITE_DEMO */     { ITEDEMO_GameFonts,    ARRAYSIZE(ITEDEMO_GameFonts)    },
	/* FONTLIST_ITE_WIN_DEMO */ { ITEWINDEMO_GameFonts, ARRAYSIZE(ITEWINDEMO_GameFonts) },
	/* FONTLIST_IHNM_DEMO */    { IHNMDEMO_GameFonts,   ARRAYSIZE(IHNMDEMO_GameFonts)   },
	/* FONTLIST_IHNM_CD */      { IHNMCD_GameFonts,     ARRAYSIZE(IHNMCD_GameFonts)     },
	/* FONTLIST_IHNM_ZH */      { IHNMZH_GameFonts,     ARRAYSIZE(IHNMZH_GameFonts)     },
};

Font::FontId Font::knownFont2FontIdx(KnownFont font) {
	FontId fontId = kSmallFont;

	// The demo version of IHNM has 3 font types (like ITE), not 6 (like the full version of IHNM)
	if (_vm->getGameId() == GID_ITE || _vm->isIHNMDemo()) {
		switch (font) {
		case (kKnownFontSmall):
		default:
		fontId = kSmallFont;
		break;
		case (kKnownFontMedium):
		fontId = kMediumFont;
		break;
		case (kKnownFontBig):
		fontId = kBigFont;
		break;

		case (kKnownFontVerb):
		fontId = kSmallFont;
		break;
		case (kKnownFontScript):
		fontId = kMediumFont;
		break;
		case (kKnownFontPause):
		fontId = _vm->_font->valid(kBigFont) ? kBigFont : kMediumFont;
		break;
		}
#ifdef ENABLE_IHNM
	} else if (_vm->getGameId() == GID_IHNM && _vm->getLanguage() == Common::ZH_TWN) {
		// There is only one Chinese font in Chinese version AFAICT.
		// And very little non-Chinese characters to care about them
		fontId = kSmallFont;
	} else if (_vm->getGameId() == GID_IHNM && !_vm->isIHNMDemo()) {
		switch (font) {
		case (kKnownFontSmall):
		default:
		fontId = kSmallFont;
		break;
		case (kKnownFontMedium):
		fontId = kMediumFont;
		break;
		case (kKnownFontBig):
		fontId = kBigFont;
		break;

		case (kKnownFontVerb):
		fontId = kIHNMFont8;
		break;
		case (kKnownFontScript):
		fontId = kIHNMMainFont;
		break;
		case (kKnownFontPause):
		fontId = kMediumFont; // unchecked
		break;
		}
#endif
	}
	return fontId;
}

void Font::textDraw(FontId fontId, const char *text, const Common::Point &point, int color, int effectColor, FontEffectFlags flags) {
	int textWidth;
	int textLength;
	int fitWidth;
	Common::Point textPoint(point);

	textLength = getStringLength(text);

	if (!(flags & kFontCentered)) {
		// Text is not centered; No formatting required
		draw(fontId, text, textLength, point, color, effectColor, flags);
		return;
	}

	// Text is centered... format output
	// Enforce minimum and maximum center points for centered text
	if (textPoint.x < TEXT_CENTERLIMIT) {
		textPoint.x = TEXT_CENTERLIMIT;
	}

	if (textPoint.x > _vm->_gfx->getBackBufferWidth() - TEXT_CENTERLIMIT) {
		textPoint.x = _vm->_gfx->getBackBufferWidth() - TEXT_CENTERLIMIT;
	}

	if (textPoint.x < (TEXT_MARGIN * 2)) {
		// Text can't be centered if it's too close to the margin
		return;
	}

	textWidth = getStringWidth(fontId, text, textLength, flags);

	if (textPoint.x < (_vm->_gfx->getBackBufferWidth() / 2)) {
		// Fit to right side
		fitWidth = (textPoint.x - TEXT_MARGIN) * 2;
	} else {
		// Fit to left side
		fitWidth = ((_vm->_gfx->getBackBufferWidth() - TEXT_MARGIN) - textPoint.x) * 2;
	}

	if (fitWidth < textWidth) {
		warning("text too long to be displayed in one line");
		textWidth = fitWidth;
	}
	// Entire string fits, draw it
	textPoint.x = textPoint.x - (textWidth / 2);
	draw(fontId, text, textLength, textPoint, color, effectColor, flags);
}

DefaultFont::DefaultFont(SagaEngine *vm) : Font(vm), _fontMapping(0), _chineseFont(nullptr), _cjkFontWidth(0), _cjkFontHeight(0), _koreanFont(nullptr) {
	int i;

	// Load font module resource context

	GameFontList index = _vm->getFontList();
	assert(index < FONTLIST_MAX && index >= FONTLIST_NONE);
	assert(FontLists[index].list || FontLists[index].count == 0);
	assert(FontLists[index].count > 0 || (_vm->getFeatures() & GF_EMBED_FONT));

	_fonts.resize(MAX<int>(FontLists[index].count, (_vm->getFeatures() & GF_EMBED_FONT) ? 2 : 0));

	for (i = 0; i < FontLists[index].count; i++) {
#ifdef __DS__
		_fonts[i].outline.font = NULL;
		_fonts[i].normal.font = NULL;
#endif
		if (i == 0 && index == FONTLIST_IHNM_ZH)
			loadChineseFontIHNM(&_fonts[i],	FontLists[index].list[i].fontResourceId);
		else
			loadFont(&_fonts[i], FontLists[index].list[i].fontResourceId);
	}

	if (_vm->getFeatures() & GF_EMBED_FONT) {
		loadFont(&_fonts[kSmallFont], ByteArray(font_small8, sizeof(font_small8)), true);
		loadFont(&_fonts[kMediumFont], ByteArray(font_ite8, sizeof(font_ite8)), true);
	}

	if (_vm->getGameId() == GID_ITE && _vm->getLanguage() == Common::ZH_TWN)
		loadChineseFontITE("ite.fnt");

	if (_vm->getGameId() == GID_IHNM && _vm->getLanguage() == Common::KO_KOR)
		loadKoreanFontIHNM("sbh1616.fnt");
}

DefaultFont::~DefaultFont() {
	debug(8, "DefaultFont::~DefaultFont(): Freeing fonts.");

#ifdef __DS__
	for (uint i = 0; i < _fonts.size(); i++) {
		if (_fonts[i].outline.font) {
			free(_fonts[i].outline.font);
		}

		if (_fonts[i].normal.font) {
			free(_fonts[i].normal.font);
		}
	}
#endif
	if (_chineseFont) {
		delete[] _chineseFont;
		_chineseFont = nullptr;
	}

	if (_koreanFont) {
		delete[] _koreanFont;
		_koreanFont = nullptr;
	}
}

void DefaultFont::loadChineseFontITE(const Common::String& fileName) {
	Common::File f;
	if (!f.open(fileName))
		return;
	_cjkFontWidth = 16;
	_cjkFontHeight = 14;
	_chineseFontIndex = Common::move(Common::Array<int>(0x8000, -1));
	size_t sz = f.size();
	_chineseFont = new byte[sz];
	f.read(_chineseFont, sz);
	static const int kGlyphSize = 30;
	for (unsigned i = 0; i < sz / kGlyphSize; i++) {
		uint16 ch = READ_BE_UINT16(_chineseFont + kGlyphSize * i);
		if (!(ch & 0x8000))
			continue;
		_chineseFontIndex[ch&0x7fff] = kGlyphSize * i + 2;
	}
}

void DefaultFont::loadKoreanFontIHNM(const Common::String& fileName) {
	Common::File f;
	if (!f.open(fileName))
		return;
	size_t sz = f.size();
	if (sz < kIHNMKoreanNonJamoOffset * kIHNMKoreanGlyphBytes)
		return;

	_cjkFontWidth = 16;
	_cjkFontHeight = 16;

	_koreanFont = new byte[sz];
	f.read(_koreanFont, sz);
}


void DefaultFont::saveBig5Index(byte head, byte tail, uint curIdx) {
	_chineseFontIndex[((head & 0x7f) << 8) | tail] = curIdx;
}

void DefaultFont::loadChineseFontIHNM(FontData *font, uint32 fontResourceId) {
	ByteArray fontResourceData;
	int c;
	ResourceContext *fontContext;

	debug(1, "Font::loadChineseFontIHNM(): Reading fontResourceId %d...", fontResourceId);

	fontContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (fontContext == nullptr) {
		error("DefaultFont::Font() resource context not found");
	}

	// Load font resource
	_vm->_resource->loadResource(fontContext, fontResourceId, fontResourceData);

	ByteArrayReadStreamEndian readS(fontResourceData, fontContext->isBigEndian());

	// Read font header
	font->normal.header.charHeight = 15;
	font->normal.header.charWidth = 8;
	font->normal.header.rowLength = 1;

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].width = 8;
		font->normal.fontCharEntry[c].byteWidth = 1;
		font->normal.fontCharEntry[c].flag = 0;
		font->normal.fontCharEntry[c].tracking = 8;
	}

	_chineseFont = new byte[fontResourceData.size()];
	memcpy(_chineseFont, fontResourceData.getBuffer(), fontResourceData.size());
	_cjkFontWidth = 16;
	_cjkFontHeight = 15;
	_chineseFontIndex = Common::move(Common::Array<int>(0x8000, -1));

	// No idea what is the beginning, some 3 values and then some bitmask,
	// anyway file is constant and as long as we know how to interpret
	// and match glyphs we're good
	int curIdx = 1286;

	// It's just sequential big5 codepoints by compartments specified in
	// Big5 specification. Compartments are out of order
	// 0x8140 to 0xA0FE	Reserved for user-defined characters 造字
	// Not present
	// 0xA440 to 0xC67E	Frequently used characters 常用字
	for (byte head = 0xa4; head <= 0xc5; head++) {
		for (byte tail = 0x40; tail <= 0x7e; tail++, curIdx += 30)
			saveBig5Index(head, tail, curIdx);
		for (byte tail = 0xa1; tail <= 0xfe; tail++, curIdx += 30)
			saveBig5Index(head, tail, curIdx);
	}

	for (byte tail = 0x40; tail <= 0x7e; tail++, curIdx += 30)
		saveBig5Index(0xc6, tail, curIdx);

	// 0xC6A1 to 0xC8FE	Reserved for user-defined characters.
	// Not present

	// 0xC940 to 0xF9D5	Less frequently used characters 次常用字
	// Rounded up to F9FE with pseudographics characters
	for (byte head = 0xc9; head <= 0xf9; head++) {
		for (byte tail = 0x40; tail <= 0x7e; tail++, curIdx += 30)
			saveBig5Index(head, tail, curIdx);
		for (byte tail = 0xa1; tail <= 0xfe; tail++, curIdx += 30)
			saveBig5Index(head, tail, curIdx);
	}

	// 0xFA40 to 0xFEFE	Reserved for user-defined characters
	// Not present

	// Then comes back to a140
	// 0xA140 to 0xA3BF	"Graphical characters" 圖形碼
	for (byte head = 0xa1; head <= 0xa2; head++) {
		for (byte tail = 0x40; tail <= 0x7e; tail++, curIdx += 30)
			saveBig5Index(head, tail, curIdx);
		for (byte tail = 0xa1; tail <= 0xfe; tail++, curIdx += 30)
			saveBig5Index(head, tail, curIdx);
	}

	for (byte tail = 0x40; tail <= 0x7e; tail++, curIdx += 30)
		saveBig5Index(0xa3, tail, curIdx);
	for (byte tail = 0xa1; tail <= 0xbf; tail++, curIdx += 30)
		saveBig5Index(0xa3, tail, curIdx);

	// 0xA3C0 to 0xA3FE	Reserved, not for user-defined characters
	// Not present

	// Then single-width ASCII
	int startASCII = curIdx;

	for (c = 0; c < FONT_CHARCOUNT; c++, curIdx += 15) {
		font->normal.fontCharEntry[c].index = curIdx - startASCII;
	}

#ifndef __DS__
	font->normal.font.resize(fontResourceData.size() - startASCII);
	memcpy(font->normal.font.getBuffer(), fontResourceData.getBuffer() + startASCII, fontResourceData.size() - startASCII);
#else
	if (font->normal.font) {
		free(font->normal.font);
	}

	font->normal.font = (byte *)malloc(fontResourceData.size() - startASCII);
	memcpy(font->normal.font, fontResourceData.getBuffer() + startASCII, fontResourceData.size() - startASCII);
#endif

	// Create outline font style
	createOutline(font);
}

void DefaultFont::textDrawRect(FontId fontId, const char *text, const Common::Rect &rect, int color, int effectColor, FontEffectFlags flags) {
	int textWidth;
	int textLength;
	int fitWidth;
	const char *startPointer;
	const char *searchPointer;
	const char *measurePointer;
	const char *foundPointer;
	int len;
	int w;
	const char *endPointer;
	int h;
	int wc;
	int w_total;
	int len_total;
	Common::Point textPoint;
	Common::Point textPoint2;

	textLength = getStringLength(text);

	textWidth = getStringWidth(fontId, text, textLength, flags);
	fitWidth = rect.width();

	textPoint.x = rect.left + (fitWidth / 2);
	textPoint.y = rect.top;

	if (fitWidth >= textWidth) {
		// Entire string fits, draw it
		textPoint.x -= (textWidth / 2);
		draw(fontId, text, textLength, textPoint, color, effectColor, flags);
		return;
	}

	// String won't fit on one line
	h = getHeight(fontId, text);
	w_total = 0;
	len_total = 0;
	wc = 0;

	startPointer = text;
	measurePointer = text;
	searchPointer = text;
	endPointer = text + textLength;

	// IHNM korean uses spaces, so we use western algorithm for it.
	bool isBig5 = !!_chineseFont;

	for (;;) {
		if (isBig5) {
			if (*searchPointer & 0x80)
				foundPointer = searchPointer + 2;
			else if (*searchPointer)
				foundPointer = searchPointer + 1;
			else
				foundPointer = nullptr;
		} else
			foundPointer = strchr(searchPointer, ' ');
		if (foundPointer == nullptr) {
			// Ran to the end of the buffer
			len = endPointer - measurePointer;
		} else {
			len = foundPointer - measurePointer;
		}

		w = getStringWidth(fontId, measurePointer, len, flags);
		measurePointer = foundPointer;

		if ((w_total + w) > fitWidth) {
			// This word won't fit
			if (wc == 0) {
				w_total = fitWidth;
				len_total = len;
			}

			// Wrap what we've got and restart
			textPoint2.x = textPoint.x - (w_total / 2);
			textPoint2.y = textPoint.y;
			draw(fontId, startPointer, len_total, textPoint2, color, effectColor, flags);
			textPoint.y += h + TEXT_LINESPACING;
			if (textPoint.y >= rect.bottom) {
				return;
			}
			w_total = 0;
			len_total = 0;
			if (wc == 0 && measurePointer) {
				if (isBig5 && (*measurePointer & 0x80))
					searchPointer = measurePointer + 2;
				else
					searchPointer = measurePointer + 1;
			}
			wc = 0;

			// Advance the search pointer to the next non-space.
			// Otherwise, the first "word" to be measured will be
			// an empty string. Measuring or drawing a string of
			// length 0 is interpreted as measure/draw the entire
			// buffer, which certainly is not what we want here.
			//
			// This happes because a string may contain several
			// spaces in a row, e.g. after a period.

			while (*searchPointer == ' ')
				searchPointer++;

			measurePointer = searchPointer;
			startPointer = searchPointer;
		} else {
			// Word will fit ok
			w_total += w;
			len_total += len;
			wc++;
			if (foundPointer == nullptr) {
				// Since word hit NULL but fit, we are done
				textPoint2.x = textPoint.x - (w_total / 2);
				textPoint2.y = textPoint.y;
				draw(fontId, startPointer, len_total, textPoint2, color,
					effectColor, flags);
				return;
			}
			if (isBig5 && (*measurePointer & 0x80))
				searchPointer = measurePointer + 2;
			else
				searchPointer = measurePointer + 1;
		}
	}
}

int DefaultFont::translateChar(int charId) {
	if (charId <= 127 || (_vm->getLanguage() == Common::RU_RUS && charId <= 255) || (_vm->getLanguage() == Common::HE_ISR && charId <= 255))
		return charId;					// normal character
	else
		return _charMap[charId - 128];	// extended character
}

// Returns the horizontal length in pixels of the graphical representation
// of at most 'count' characters of the string 'text', taking
// into account any formatting options specified by 'flags'.
// If 'count' is 0, all characters of 'test' are counted.
int DefaultFont::getStringWidth(FontId fontId, const char *text, size_t count, FontEffectFlags flags) {
	size_t ct;
	int width = 0;
	int ch;
	const byte *txt;
	FontData *font = getFont(fontId);
	txt = (const byte *) text;
	bool isCJK = _chineseFont || _koreanFont;

	for (ct = count; *txt && (!count || ct > 0); txt++, ct--) {
		ch = *txt & 0xFFU;
		if ((ch & 0x80) && isCJK) {
			byte trailing = *++txt & 0xFFU;
			ct--;
			if (ct == 0 || trailing == 0)
				break;
			width += _cjkFontWidth;
			continue;
		}

		// Translate character
		ch = translateChar(ch);
		assert(ch < FONT_CHARCOUNT);
		width += font->normal.fontCharEntry[ch].tracking;
	}

	if ((flags & kFontBold) || (flags & kFontOutline)) {
		width += 1;
	}

	return width;
}

int DefaultFont::getHeight(FontId fontId, const char *text, int width, FontEffectFlags flags) {
	int textWidth;
	int textLength;
	int fitWidth;
	const char *searchPointer;
	const char *measurePointer;
	const char *foundPointer;
	int len;
	int w;
	const char *endPointer;
	int h;
	int wc;
	int w_total;
	Common::Point textPoint;

	textLength = getStringLength(text);
	textWidth = getStringWidth(fontId, text, textLength, flags);
	h = getHeight(fontId, text);
	fitWidth = width;

	textPoint.x = (fitWidth / 2);
	textPoint.y = 0;

	if (fitWidth >= textWidth) {
		return h;
	}

	// String won't fit on one line
	w_total = 0;
	wc = 0;

	measurePointer = text;
	searchPointer = text;
	endPointer = text + textLength;

	// IHNM korean uses spaces, so we use western algorithm for it.
	bool isBig5 = !!_chineseFont;

	for (;;) {
		if (isBig5) {
			if (*searchPointer & 0x80)
				foundPointer = searchPointer + 2;
			else if (*searchPointer)
				foundPointer = searchPointer + 1;
			else
				foundPointer = nullptr;
		} else
			foundPointer = strchr(searchPointer, ' ');
		if (foundPointer == nullptr) {
			// Ran to the end of the buffer
			len = endPointer - measurePointer;
		} else {
			len = foundPointer - measurePointer;
		}

		w = getStringWidth(fontId, measurePointer, len, flags);
		measurePointer = foundPointer;

		if ((w_total + w) > fitWidth) {
			// This word won't fit
			if (wc == 0) {
				// The first word in the line didn't fit. Still print it
				if (isBig5 && (*measurePointer & 0x80))
					searchPointer = measurePointer + 2;
				else
					searchPointer = measurePointer + 1;
			}
			// Wrap what we've got and restart
			textPoint.y += h + TEXT_LINESPACING;
			if (foundPointer == nullptr) {
				// Since word hit NULL but fit, we are done
				return textPoint.y + h;
			}
			w_total = 0;
			wc = 0;
			measurePointer = searchPointer;
		} else {
			// Word will fit ok
			w_total += w;
			wc++;
			if (foundPointer == nullptr) {
				// Since word hit NULL but fit, we are done
				return textPoint.y + h;
			}
			if (isBig5 && (*measurePointer & 0x80))
				searchPointer = measurePointer + 2;
			else
				searchPointer = measurePointer + 1;
		}
	}
}

void DefaultFont::draw(FontId fontId, const char *text, size_t count, const Common::Point &point,
			   int color, int effectColor, FontEffectFlags flags) {

	Point offsetPoint(point);
	FontData *font = getFont(fontId);

	if (_vm->getLanguage() == Common::HE_ISR) {
		Common::String textstr(text, count);
		text = Common::convertBiDiString(textstr, Common::kWindows1255).c_str();
	}

	if (flags & kFontOutline) {
		offsetPoint.x--;
		offsetPoint.y--;
		outFont(font->outline, text, count, offsetPoint, effectColor, flags);
		outFont(font->normal, text, count, point, color, flags);
	} else if (flags & kFontShadow) {
		offsetPoint.x--;
		offsetPoint.y++;
		outFont(font->normal, text, count, offsetPoint, effectColor, flags);
		outFont(font->normal, text, count, point, color, flags);
	} else { // FONT_NORMAL
		outFont(font->normal, text, count, point, color, flags);
	}
}

int DefaultFont::getHeight(FontId fontId, const char *text) {
	int singleByteHeight = getHeight(fontId);
	if ((!_chineseFont && !_koreanFont) || _cjkFontHeight < singleByteHeight)
		return singleByteHeight;

	for (const byte *textPointer = (const byte *)text; *textPointer; textPointer++)
		if (*textPointer & 0x80)
			return _cjkFontHeight;

	return singleByteHeight;
}

void DefaultFont::blitGlyph(const Common::Point &textPoint, const byte* bitmap, int charWidth, int charHeight, int rowLength, byte color) {
	// Get length of character in bytes
	int c_byte_len = ((charWidth - 1) / 8) + 1;
	int rowLimit = (_vm->_gfx->getBackBufferHeight() < (textPoint.y + charHeight)) ? _vm->_gfx->getBackBufferHeight() : textPoint.y + charHeight;
	int charRow = 0;

	for (int row = textPoint.y; row < rowLimit; row++, charRow++) {
		// Clip negative rows */
		if (row < 0) {
			continue;
		}

		byte *outputPointer = _vm->_gfx->getBackBufferPixels() + (_vm->_gfx->getBackBufferPitch() * row) + textPoint.x;
		byte *outputPointer_min = _vm->_gfx->getBackBufferPixels() + (_vm->_gfx->getBackBufferPitch() * row) + (textPoint.x > 0 ? textPoint.x : 0);
		byte *outputPointer_max = outputPointer + (_vm->_gfx->getBackBufferPitch() - textPoint.x);

		// If character starts off the screen, jump to next character
		if (outputPointer < outputPointer_min) {
			break;
		}

		const byte *c_dataPointer = bitmap + charRow * rowLength;

		for (int c_byte = 0; c_byte < c_byte_len; c_byte++, c_dataPointer++) {
			// Check each bit, draw pixel if bit is set
			for (int c_bit = 7; c_bit >= 0 && (outputPointer < outputPointer_max); c_bit--) {
				if ((*c_dataPointer >> c_bit) & 0x01) {
					*outputPointer = (byte)color;
				}
				outputPointer++;
			} // end per-bit processing
		} // end per-byte processing
	} // end per-row processing
}

void DefaultFont::outFont(const FontStyle &drawFont, const char *text, size_t count, const Common::Point &point, int color, FontEffectFlags flags) {
	const byte *textPointer;
	int c_code;
	Point textPoint(point);

	int ct;

	if ((point.x > _vm->_gfx->getBackBufferWidth()) || (point.y > _vm->_gfx->getBackBufferHeight())) {
		// Output string can't be visible
		return;
	}

	textPointer = (const byte *)text;
	ct = count;

	bool isBig5 = !!_chineseFont;
	bool isJohab = !!_koreanFont;

	// Draw string one character at a time, maximum of 'draw_str'_ct
	// characters, or no limit if 'draw_str_ct' is 0
	for (; *textPointer && (!count || ct); textPointer++, ct--) {
		c_code = *textPointer & 0xFFU;

		if ((c_code & 0x80) && isJohab) {
			byte leading = c_code;
			byte trailing = *++textPointer & 0xFFU;
			ct--;
			if (ct == 0 || trailing == 0)
				break;
			uint16 full = ((leading & 0x7f) << 8) | trailing;
			int initial = (full >> 10) & 0x1f;
			int mid = (full >> 5) & 0x1f;
			int fin = full & 0x1f;
			int initidx = initial - 1;
			static const int mididxlut[0x20] = {
				-1, -1, 0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, 9,
				10, 11, -1, -1, 12, 13, 14, 15, 16, 17, -1, -1,
				18, 19, 20, 21, -1, -1};
			int mididx = mididxlut[mid];
			int finidx = fin >= 0x12 ? fin - 2 : fin - 1;

			// Validate character
			if (initial >= 0x15 || initidx < 0 || mididx < 0 || fin == 0 || fin == 0x12 || fin >= 0x1e) {
				// Characters with initial over 0x15 means "non-jamo-based", e.g. Hanja, pictograms
				// and so on. They are present in the font but not supported by renderer neither in
				// the original nor in the scummvm
				textPoint.x += _cjkFontWidth;
				continue;
			}

			static const int mid2inivariant[2][32] = {
				// Special case: empty final
				{
					0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 1, 3, 3,
					0, 0, 3, 1, 2, 4, 4, 4,
					0, 0, 2, 1, 3, 0, 0, 0,
				},
				// Otherwise we have a final
				{
					0, 0, 0, 5, 5, 5, 5, 5,
					0, 0, 5, 5, 5, 6, 7, 7,
					0, 0, 7, 6, 6, 7, 7, 7,
					0, 0, 6, 6, 7, 5, 0, 0,
				}
			};
			int inivariant = mid2inivariant[fin != 1][mid];
			int midvariant = 0;

			if (fin != 1)
				midvariant += 2;
			if (initial == 2 || initial == 17)
				midvariant++;

			static const int mid2finvariant[32] = {
				0, 0, 0, 0, 2, 0, 2, 1,
				0, 0, 2, 1, 2, 3, 0, 2,
				0, 0, 1, 3, 3, 1, 2, 1,
				0, 0, 3, 3, 1, 1, 0, 0,
			};
			int finvariant = mid2finvariant[mid];

			int initialoff = kIHNMKoreanGlyphBytes * (initidx + inivariant * kIHNMKoreanInitials);
			blitGlyph(textPoint, _koreanFont + initialoff, _cjkFontWidth, _cjkFontHeight, _cjkFontWidth / 8, (byte)color);
			int midoff = kIHNMKoreanGlyphBytes * (mididx + midvariant * kIHNMKoreanMids + kIHNMKoreanMidOffset);
			blitGlyph(textPoint, _koreanFont + midoff, _cjkFontWidth, _cjkFontHeight, _cjkFontWidth / 8, (byte)color);
			int finoff = kIHNMKoreanGlyphBytes * (finidx + finvariant * kIHNMKoreanFinals + kIHNMKoreanFinalsOffset);
			blitGlyph(textPoint, _koreanFont + finoff, _cjkFontWidth, _cjkFontHeight, _cjkFontWidth / 8, (byte)color);

			// Advance tracking position
			textPoint.x += _cjkFontWidth;
			continue;
		}


		if ((c_code & 0x80) && isBig5) {
			byte leading = c_code;
			byte trailing = *++textPointer & 0xFFU;
			ct--;
			if (ct == 0 || trailing == 0)
				break;
			int idx = _chineseFontIndex[((leading & 0x7f) << 8) | trailing];
			if (idx < 0) {
				textPoint.x += _cjkFontWidth;
				continue;
			}
			blitGlyph(textPoint, _chineseFont + idx, _cjkFontWidth, _cjkFontHeight, _cjkFontWidth / 8, (byte)color);
			// Advance tracking position
			textPoint.x += _cjkFontWidth;
			continue;
		}
		
		// Translate character
		if (_fontMapping == 0) {	// Check font mapping debug flag
			// Default game behavior

			// It seems that this font mapping causes problems with non-english
			// versions of IHNM, so it has been changed to apply for ITE only.
			// It doesn't make any difference for the English version of IHNM.
			// Fixes bug #3405: "IHNM: Spanish font wrong".
			if (!(flags & kFontDontmap) && _vm->getGameId() == GID_ITE) {
				if (_vm->getLanguage() != Common::IT_ITA) {
					c_code = translateChar(c_code);
				} else {
					// The in-game fonts of the Italian version should not be mapped.
					// The ones in the intro are hardcoded and should be mapped normally.
					if (_vm->_scene->isInIntro())
						c_code = translateChar(c_code);
				}
			}
		} else if (_fontMapping == 1) {
			// Force font mapping
			c_code = translateChar(c_code);
		} else {
			// In all other cases, ignore font mapping
		}
		assert(c_code < FONT_CHARCOUNT);

		// Check if character is defined
		if ((drawFont.fontCharEntry[c_code].index == 0) && (c_code != FONT_FIRSTCHAR)) {
#if FONT_SHOWUNDEFINED
			// A tab character appears in the IHNM demo instructions screen, so filter
			// it out here
			if (c_code == FONT_CH_SPACE || c_code == FONT_CH_TAB) {
				textPoint.x += drawFont.fontCharEntry[c_code].tracking;
				continue;
			}
			c_code = FONT_CH_QMARK;
#else
			// Character code is not defined, but advance tracking
			// ( Not defined if offset is 0, except for 33 ('!') which
			//   is defined )
			textPoint.x += drawFont.fontCharEntry[c_code].tracking;
			continue;
#endif
		}

		blitGlyph(textPoint, &drawFont.font[drawFont.fontCharEntry[c_code].index], drawFont.fontCharEntry[c_code].width, drawFont.header.charHeight,
			drawFont.header.rowLength, (byte)color);

		// Advance tracking position
		textPoint.x += drawFont.fontCharEntry[c_code].tracking;
	} // end per-character processing

	int rowLimit = (_vm->_gfx->getBackBufferHeight() < (textPoint.y + drawFont.header.charHeight)) ? _vm->_gfx->getBackBufferHeight() : textPoint.y + drawFont.header.charHeight;
	_vm->_render->addDirtyRect(Common::Rect(point.x, point.y, textPoint.x, rowLimit));
}

void DefaultFont::loadFont(FontData *font, uint32 fontResourceId) {
	ByteArray fontResourceData;
	ResourceContext *fontContext;

	debug(1, "Font::loadFont(): Reading fontResourceId %d...", fontResourceId);

	fontContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (fontContext == nullptr) {
		error("DefaultFont::Font() resource context not found");
	}

	// Load font resource
	_vm->_resource->loadResource(fontContext, fontResourceId, fontResourceData);

	loadFont(font, fontResourceData, fontContext->isBigEndian());
}

void DefaultFont::loadFont(FontData *font, const ByteArray& fontResourceData, bool isBigEndian) {
	int numBits;
	int c;

	if (fontResourceData.size() < FONT_DESCSIZE) {
		error("DefaultFont::loadFont() Invalid font length (%i < %i)", (int)fontResourceData.size(), FONT_DESCSIZE);
	}

	ByteArrayReadStreamEndian readS(fontResourceData, isBigEndian);

	// Read font header
	font->normal.header.charHeight = readS.readUint16();
	font->normal.header.charWidth = readS.readUint16();
	font->normal.header.rowLength = readS.readUint16();


	debug(2, "Character width: %d", font->normal.header.charWidth);
	debug(2, "Character height: %d", font->normal.header.charHeight);
	debug(2, "Row padding: %d", font->normal.header.rowLength);

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].index = readS.readUint16();
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		numBits = font->normal.fontCharEntry[c].width = readS.readByte();
		font->normal.fontCharEntry[c].byteWidth = getByteLen(numBits);
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].flag = readS.readByte();
	}

	for (c = 0; c < FONT_CHARCOUNT; c++) {
		font->normal.fontCharEntry[c].tracking = readS.readByte();
	}

	if (readS.pos() != FONT_DESCSIZE) {
		error("Invalid font resource size");
	}

#ifndef __DS__
	font->normal.font.resize(fontResourceData.size() - FONT_DESCSIZE);
	memcpy(font->normal.font.getBuffer(), fontResourceData.getBuffer() + FONT_DESCSIZE, fontResourceData.size() - FONT_DESCSIZE);
#else
	if (font->normal.font) {
		free(font->normal.font);
	}

	font->normal.font = (byte *)malloc(fontResourceData.size() - FONT_DESCSIZE);
	memcpy(font->normal.font, fontResourceData.getBuffer() + FONT_DESCSIZE, fontResourceData.size() - FONT_DESCSIZE);
#endif

	// Create outline font style
	createOutline(font);
}

void DefaultFont::createOutline(FontData *font) {
	int i;
	int row;
	int newByteWidth;
	int newRowLength = 0;
	int currentByte;
	byte *basePointer;
	byte *srcPointer;
	byte *destPointer1;
	byte *destPointer2;
	byte *destPointer3;
	byte charRep;

	// Populate new font style character data
	for (i = 0; i < FONT_CHARCOUNT; i++) {
		newByteWidth = 0;

		font->outline.fontCharEntry[i].index = newRowLength;
		font->outline.fontCharEntry[i].tracking = font->normal.fontCharEntry[i].tracking;
		font->outline.fontCharEntry[i].flag = font->normal.fontCharEntry[i].flag;

		if (font->normal.fontCharEntry[i].width != 0)
			newByteWidth = getByteLen(font->normal.fontCharEntry[i].width + 2);

		font->outline.fontCharEntry[i].width = font->normal.fontCharEntry[i].width + 2;
		font->outline.fontCharEntry[i].byteWidth = newByteWidth;

		newRowLength += newByteWidth;
	}

	debug(2, "New row length: %d", newRowLength);

	font->outline.header = font->normal.header;
	font->outline.header.charWidth += 2;
	font->outline.header.charHeight += 2;
	font->outline.header.rowLength = newRowLength;

	// Allocate new font representation storage
#ifdef __DS__
	if (font->outline.font) {
		free(font->outline.font);
	}

	font->outline.font = (byte *)calloc(newRowLength * font->outline.header.charHeight, 1);
#else
	font->outline.font.resize(newRowLength * font->outline.header.charHeight);
#endif


	// Generate outline font representation
	for (i = 0; i < FONT_CHARCOUNT; i++) {
		for (row = 0; row < font->normal.header.charHeight; row++) {
			for (currentByte = 0; currentByte < font->outline.fontCharEntry[i].byteWidth; currentByte++) {
				basePointer = &font->outline.font[font->outline.fontCharEntry[i].index + currentByte];
				destPointer1 = basePointer + newRowLength * row;
				destPointer2 = basePointer + newRowLength * (row + 1);
				destPointer3 = basePointer + newRowLength * (row + 2);
				if (currentByte > 0) {
					// Get last two columns from previous byte
					srcPointer = &font->normal.font[font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + (currentByte - 1)];
					charRep = *srcPointer;
					*destPointer1 |= ((charRep << 6) | (charRep << 7));
					*destPointer2 |= ((charRep << 6) | (charRep << 7));
					*destPointer3 |= ((charRep << 6) | (charRep << 7));
				}

				if (currentByte < font->normal.fontCharEntry[i].byteWidth) {
					srcPointer = &font->normal.font[font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + currentByte];
					charRep = *srcPointer;
					*destPointer1 |= charRep | (charRep >> 1) | (charRep >> 2);
					*destPointer2 |= charRep | (charRep >> 1) | (charRep >> 2);
					*destPointer3 |= charRep | (charRep >> 1) | (charRep >> 2);
				}
			}
		}

		// "Hollow out" character to prevent overdraw
		for (row = 0; row < font->normal.header.charHeight; row++) {
			for (currentByte = 0; currentByte < font->outline.fontCharEntry[i].byteWidth; currentByte++) {
				destPointer2 = &font->outline.font[font->outline.header.rowLength * (row + 1) + font->outline.fontCharEntry[i].index + currentByte];
				if (currentByte > 0) {
					// Get last two columns from previous byte
					srcPointer = &font->normal.font[font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + (currentByte - 1)];
					*destPointer2 &= ((*srcPointer << 7) ^ 0xFFU);
				}

				if (currentByte < font->normal.fontCharEntry[i].byteWidth) {
					srcPointer = &font->normal.font[font->normal.header.rowLength * row + font->normal.fontCharEntry[i].index + currentByte];
					*destPointer2 &= ((*srcPointer >> 1) ^ 0xFFU);
				}
			}
		}
	}
}

SJISFont::SJISFont(SagaEngine *vm) : Font(vm), _font(nullptr) {
	_font = Graphics::FontSJIS::createFont(vm->getPlatform());
	assert(_font);
}

SJISFont::~SJISFont() {
	delete _font;
}

void SJISFont::textDrawRect(FontId fontId, const char *text, const Common::Rect &rect, int color, int effectColor, FontEffectFlags flags) {
	Common::Point textPoint(rect.left, rect.top);
	int curW = 0;
	int numChar = 0;
	const char *pos = text;
	const char *last = nullptr;
	int checkWidth = (rect.width() - 16) & ~7;

	for (uint16 c = fetchChar(pos); c; c = fetchChar(pos)) {
		curW += (_font->getCharWidth(c) >> 1);
		if ((curW > checkWidth && !preventLineBreakForCharacter(c)) || c == (uint16)'\r' || c == (uint16)'\n') {
			draw(fontId, text, numChar, textPoint, color, effectColor, flags);
			numChar = 0;
			textPoint.x = rect.left;
			textPoint.y += (getHeight(fontId));
			// Abort if there is no more space inside the rect
			if (textPoint.y + getHeight(fontId) > rect.bottom)
				return;
			// Skip linebreak characters
			if (c == (uint16)'\r' || c == (uint16)'\n')
				last++;
			pos = text = last;
			last = nullptr;
			curW = 0;
		} else {
			numChar++;
			last = pos;
		}
	}

	// If the whole string fits into one line it gets aligned to the center
	if (textPoint.y == rect.top)
		textPoint.x = textPoint.x + (rect.width() - getStringWidth(fontId, text, 0, flags)) / 2;

	draw(fontId, text, numChar, textPoint, color, effectColor, flags);
}

int SJISFont::getStringLength(const char *text) {
	int res = 0;
	while (fetchChar(text))
		res++;

	return res;
}

int SJISFont::getStringWidth(FontId fontId, const char *text, size_t count, FontEffectFlags flags) {
	// The spacing is always the same regardless of the fontId and font style
	_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
	int curW = 0;
	int maxW = 0;

	for (uint16 c = fetchChar(text); c; c = fetchChar(text)) {
		if (c == (uint16)'\r' || c == (uint16)'\n') {
			maxW = MAX<int>(curW, maxW);
			curW = 0;
			continue;
		}
		curW += _font->getCharWidth(c);
		if (!--count)
			break;
	}

	return MAX<int>(curW, maxW) >> 1;
}

int SJISFont::getHeight(FontId fontId, const char *text, int width, FontEffectFlags flags) {
	Graphics::FontSJIS::DrawingMode mode = Graphics::FontSJIS::kDefaultMode;
	if (flags & kFontOutline)
		mode = Graphics::FontSJIS::kOutlineMode;
	else if (flags & kFontShadow)
		mode = Graphics::FontSJIS::kShadowRightMode;

	_font->setDrawingMode(mode);
	int res = _font->getFontHeight();
	int checkWidth = (width - 16) & ~7;
	int tmpWidth = 0;

	for (uint16 c = fetchChar(text); c; c = fetchChar(text)) {
		// The spacing is always the same (regardless of the fontId and font style) for the char spacing, but not for the line spacing.
		_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
		tmpWidth += (_font->getCharWidth(c) >> 1);
		if ((tmpWidth > checkWidth && !preventLineBreakForCharacter(c)) || c == (uint16)'\r' || c == (uint16)'\n') {
			tmpWidth = tmpWidth > width ? _font->getCharWidth(c) >> 1 : 0;
			_font->setDrawingMode(mode);
			res += _font->getFontHeight();
		}
	}

	return (res + 1) >> 1;
}

int SJISFont::getHeight(FontId fontId) {
	// The spacing here is always the same regardless of the style
	_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
	return (_font->getFontHeight() >> 1) + 1;
}

void SJISFont::draw(FontId fontId, const char *text, size_t count, const Common::Point &point, int color, int effectColor, FontEffectFlags flags) {
	int16 x = point.x << 1;
	int16 y = point.y << 1;

	Graphics::FontSJIS::DrawingMode mode = Graphics::FontSJIS::kDefaultMode;
	if (effectColor != 0x80) {
		if (flags & kFontOutline)
			mode = Graphics::FontSJIS::kOutlineMode;
		else if (flags & kFontShadow)
			mode = Graphics::FontSJIS::kShadowRightMode;
	}

	// DEBUG: The Graphics::FontSJIS code currently does not allow glyphs to be outlined and shaded at the same time. I'll implement it if I have to, but currently I don't think that this is the case...
	assert((flags & 3) != 3);
	_font->setDrawingMode(mode);
	Common::Rect dirtyRect((flags & kFontShadow) ? MAX<int16>(point.x - 1, 0) : point.x, point.y, point.x + 1, point.y + (_font->getFontHeight() >> 1));

	while (*text) {
		uint16 ch = fetchChar(text);
		_font->setDrawingMode(mode);
		if (ch == (uint16)'\r' || ch == (uint16)'\n') {
			dirtyRect.right = MAX<int16>(x >> 1, dirtyRect.right);
			y += _font->getFontHeight();
			x = point.x << 1;
			continue;
		}
		_font->drawChar(_vm->_gfx->getSJISBackBuffer(), ch, x, y, color, effectColor);
		// Reset drawing mode for the shadow mode extra drawing and for the character spacing (not line spacing)
		_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
		if (flags & kFontShadow)
			_font->drawChar(_vm->_gfx->getSJISBackBuffer(), ch, MAX<int16>(x - 1, 0), y, color, 0);
		x += _font->getCharWidth(ch);
		if (!--count)
			break;
	}

	dirtyRect.right = MAX<int16>(x >> 1, dirtyRect.right);
	dirtyRect.bottom = (y + _font->getFontHeight()) >> 1;
	_vm->_render->addDirtyRect(dirtyRect);
}

uint16 SJISFont::fetchChar(const char *&s) const {
	uint16 ch = (uint8)*s++;

	if (ch <= 0x7F || (ch >= 0xA1 && ch <= 0xDF))
		return ch;

	ch |= (uint8)(*s++) << 8;
	return ch;
}

bool SJISFont::preventLineBreakForCharacter(uint16 ch) const {
	uint8 c = (ch >> 8) & 0xFF;
	return c && ((c >= 0x81 && c <= 0x9F) || c >= 0xE0);
}

} // End of namespace Saga
