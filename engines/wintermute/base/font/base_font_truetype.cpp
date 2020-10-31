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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/font/base_font_truetype.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/wintermute.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include "common/unzip.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseFontTT, false)

//////////////////////////////////////////////////////////////////////////
BaseFontTT::BaseFontTT(BaseGame *inGame) : BaseFont(inGame) {
	_fontHeight = 12;
	_isBold = _isItalic = _isUnderline = _isStriked = false;
	_charset = CHARSET_ANSI;

	_fontFile = nullptr;
	_font = nullptr;
	_fallbackFont = nullptr;
	_deletableFont = nullptr;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		_cachedTexts[i] = nullptr;
	}

	_lineHeight = 0;
	_maxCharWidth = _maxCharHeight = 0;
}

//////////////////////////////////////////////////////////////////////////
BaseFontTT::~BaseFontTT(void) {
	clearCache();

	for (uint32 i = 0; i < _layers.size(); i++) {
		delete _layers[i];
	}
	_layers.clear();

	delete[] _fontFile;
	_fontFile = nullptr;

	delete _deletableFont;
	_font = nullptr;
}


//////////////////////////////////////////////////////////////////////////
void BaseFontTT::clearCache() {
	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i]) {
			delete _cachedTexts[i];
		}
		_cachedTexts[i] = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseFontTT::initLoop() {
	// we need more aggressive cache management on iOS not to waste too much memory on fonts
	if (_gameRef->_constrainedMemory) {
		// purge all cached images not used in the last frame
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
			if (_cachedTexts[i] == nullptr) {
				continue;
			}

			if (!_cachedTexts[i]->_marked) {
				delete _cachedTexts[i];
				_cachedTexts[i] = nullptr;
			} else {
				_cachedTexts[i]->_marked = false;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getTextWidth(const byte *text, int maxLength) {
	WideString textStr;

	if (_gameRef->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((const char *)text);
	} else {
		textStr = StringUtil::ansiToWide((const char *)text, _charset);
	}

	if (maxLength >= 0 && textStr.size() > (uint32)maxLength) {
		textStr = textStr.substr(0, (uint32)maxLength);
	}
	//text = text.substr(0, MaxLength); // TODO: Remove

	int textWidth, textHeight;
	measureText(textStr, -1, -1, textWidth, textHeight);

	return textWidth;
}

//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getTextHeight(const byte *text, int width) {
	WideString textStr;

	if (_gameRef->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((const char *)text);
	} else {
		textStr = StringUtil::ansiToWide((const char *)text, _charset);
	}


	int textWidth, textHeight;
	measureText(textStr, width, -1, textWidth, textHeight);

	return textHeight;
}


//////////////////////////////////////////////////////////////////////////
void BaseFontTT::drawText(const byte *text, int x, int y, int width, TTextAlign align, int maxHeight, int maxLength) {
	if (text == nullptr || strcmp((const char *)text, "") == 0) {
		return;
	}

	WideString textStr;

	// TODO: Why do we still insist on Widestrings everywhere?
	// HACK: J.U.L.I.A. uses CP1252, we need to fix that,
	// And we still don't have any UTF8-support.
	if (_gameRef->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((const char *)text);
	} else {
		textStr = StringUtil::ansiToWide((const char *)text, _charset);
	}

	if (maxLength >= 0 && textStr.size() > (uint32)maxLength) {
		textStr = textStr.substr(0, (uint32)maxLength);
	}
	//text = text.substr(0, MaxLength); // TODO: Remove

	BaseRenderer *renderer = _gameRef->_renderer;

	// find cached surface, if exists
	uint32 minUseTime = INT_MAX_VALUE;
	int minIndex = -1;
	BaseSurface *surface = nullptr;
	int textOffset = 0;

	for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
		if (_cachedTexts[i] == nullptr) {
			minUseTime = 0;
			minIndex = i;
		} else {
			if (_cachedTexts[i]->_text == textStr && _cachedTexts[i]->_align == align && _cachedTexts[i]->_width == width && _cachedTexts[i]->_maxHeight == maxHeight && _cachedTexts[i]->_maxLength == maxLength) {
				surface = _cachedTexts[i]->_surface;
				textOffset = _cachedTexts[i]->_textOffset;
				_cachedTexts[i]->_marked = true;
				_cachedTexts[i]->_lastUsed = g_system->getMillis();
				break;
			} else {
				if (_cachedTexts[i]->_lastUsed < minUseTime) {
					minUseTime = _cachedTexts[i]->_lastUsed;
					minIndex = i;
				}
			}
		}
	}

	// not found, create one
	if (!surface) {
		debugC(kWintermuteDebugFont, "Draw text: %s", text);
		surface = renderTextToTexture(textStr, width, align, maxHeight, textOffset);
		if (surface) {
			// write surface to cache
			if (_cachedTexts[minIndex] != nullptr) {
				delete _cachedTexts[minIndex];
			}
			_cachedTexts[minIndex] = new BaseCachedTTFontText;

			_cachedTexts[minIndex]->_surface = surface;
			_cachedTexts[minIndex]->_align = align;
			_cachedTexts[minIndex]->_width = width;
			_cachedTexts[minIndex]->_maxHeight = maxHeight;
			_cachedTexts[minIndex]->_maxLength = maxLength;
			_cachedTexts[minIndex]->_text = textStr;
			_cachedTexts[minIndex]->_textOffset = textOffset;
			_cachedTexts[minIndex]->_marked = true;
			_cachedTexts[minIndex]->_lastUsed = g_system->getMillis();
		}
	}


	// and paint it
	if (surface) {
		Rect32 rc;
		rc.setRect(0, 0, surface->getWidth(), surface->getHeight());
		for (uint32 i = 0; i < _layers.size(); i++) {
			uint32 color = _layers[i]->_color;
			uint32 origForceAlpha = renderer->_forceAlphaColor;
			if (renderer->_forceAlphaColor != 0) {
				color = BYTETORGBA(RGBCOLGetR(color), RGBCOLGetG(color), RGBCOLGetB(color), RGBCOLGetA(renderer->_forceAlphaColor));
				renderer->_forceAlphaColor = 0;
			}
			surface->displayTransOffset(x, y - textOffset, rc, color, Graphics::BLEND_NORMAL, false, false, _layers[i]->_offsetX, _layers[i]->_offsetY);

			renderer->_forceAlphaColor = origForceAlpha;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
BaseSurface *BaseFontTT::renderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset) {
	//TextLineList lines;
	// TODO: Use WideString-conversion here.
	//WrapText(text, width, maxHeight, lines);
	Common::Array<WideString> lines;
	_font->wordWrapText(text, width, lines);

	while (maxHeight > 0 && lines.size() * _lineHeight > maxHeight) {
		lines.pop_back();
	}
	if (lines.size() == 0) {
		return nullptr;
	}

	Graphics::TextAlign alignment = Graphics::kTextAlignInvalid;
	if (align == TAL_LEFT) {
		alignment = Graphics::kTextAlignLeft;
	} else if (align == TAL_CENTER) {
		alignment = Graphics::kTextAlignCenter;
	} else if (align == TAL_RIGHT) {
		alignment = Graphics::kTextAlignRight;
	}

	// TODO: This debug call does not work with WideString because text.c_str() returns an uint32 array.
	//debugC(kWintermuteDebugFont, "%s %d %d %d %d", text.c_str(), RGBCOLGetR(_layers[0]->_color), RGBCOLGetG(_layers[0]->_color), RGBCOLGetB(_layers[0]->_color), RGBCOLGetA(_layers[0]->_color));
//	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create((uint16)width, (uint16)(_lineHeight * lines.size()), _gameRef->_renderer->getPixelFormat());
	uint32 useColor = 0xffffffff;
	Common::Array<WideString>::iterator it;
	int heightOffset = 0;
	for (it = lines.begin(); it != lines.end(); ++it) {
		_font->drawString(surface, *it, 0, heightOffset, width, useColor, alignment);
		heightOffset += (int)_lineHeight;
	}

	BaseSurface *retSurface = _gameRef->_renderer->createSurface();

	if (_deletableFont) {
		// Reconstruct the alpha channel of the font.

		// Since we painted it with color 0xFFFFFFFF onto a black background,
		// the alpha channel is gone, but the color value of each pixel corresponds
		// to its original alpha value.

		Graphics::PixelFormat format = _gameRef->_renderer->getPixelFormat();
		uint32 *pixels = (uint32 *)surface->getPixels();

		// This is a Surface we created ourselves, so no empty space between rows.
		for (int i = 0; i < surface->w * surface->h; ++i) {
			uint8 a, r, g, b;
			format.colorToRGB(*pixels, r, g, b);
			a = r;
			*pixels++ = format.ARGBToColor(a, r, g, b);
		}
	}

	retSurface->putSurface(*surface, true);
	surface->free();
	delete surface;
	return retSurface;
	// TODO: _isUnderline, _isBold, _isItalic, _isStriked
}


//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getLetterHeight() {
	return (int)getLineHeight();
}


//////////////////////////////////////////////////////////////////////
bool BaseFontTT::loadFile(const Common::String &filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "BaseFontTT::LoadFile failed for file '%s'", filename.c_str());
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename.c_str());

	if (DID_FAIL(ret = loadBuffer(buffer))) {
		_gameRef->LOG(0, "Error parsing TTFONT file '%s'", filename.c_str());
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TTFONT)
TOKEN_DEF(SIZE)
TOKEN_DEF(FACE)
TOKEN_DEF(FILENAME)
TOKEN_DEF(BOLD)
TOKEN_DEF(ITALIC)
TOKEN_DEF(UNDERLINE)
TOKEN_DEF(STRIKE)
TOKEN_DEF(CHARSET)
TOKEN_DEF(COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(LAYER)
TOKEN_DEF(OFFSET_X)
TOKEN_DEF(OFFSET_Y)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
bool BaseFontTT::loadBuffer(char *buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TTFONT)
	TOKEN_TABLE(SIZE)
	TOKEN_TABLE(FACE)
	TOKEN_TABLE(FILENAME)
	TOKEN_TABLE(BOLD)
	TOKEN_TABLE(ITALIC)
	TOKEN_TABLE(UNDERLINE)
	TOKEN_TABLE(STRIKE)
	TOKEN_TABLE(CHARSET)
	TOKEN_TABLE(COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	if (parser.getCommand(&buffer, commands, &params) != TOKEN_TTFONT) {
		_gameRef->LOG(0, "'TTFONT' keyword expected.");
		return STATUS_FAILED;
	}
	buffer = params;

	uint32 baseColor = 0x00000000;

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_SIZE:
			parser.scanStr(params, "%d", &_fontHeight);
			break;

		case TOKEN_FACE:
			// we don't need this anymore
			break;

		case TOKEN_FILENAME:
			BaseUtils::setString(&_fontFile, params);
			break;

		case TOKEN_BOLD:
			parser.scanStr(params, "%b", &_isBold);
			break;

		case TOKEN_ITALIC:
			parser.scanStr(params, "%b", &_isItalic);
			break;

		case TOKEN_UNDERLINE:
			parser.scanStr(params, "%b", &_isUnderline);
			break;

		case TOKEN_STRIKE:
			parser.scanStr(params, "%b", &_isStriked);
			break;

		case TOKEN_CHARSET:
			parser.scanStr(params, "%d", &_charset);
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			baseColor = BYTETORGBA(r, g, b, RGBCOLGetA(baseColor));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.scanStr(params, "%d", &a);
			baseColor = BYTETORGBA(RGBCOLGetR(baseColor), RGBCOLGetG(baseColor), RGBCOLGetB(baseColor), a);
		}
		break;

		case TOKEN_LAYER: {
			BaseTTFontLayer *layer = new BaseTTFontLayer;
			if (layer && DID_SUCCEED(parseLayer(layer, params))) {
				_layers.add(layer);
			} else {
				delete layer;
				layer = nullptr;
				cmd = PARSERR_TOKENNOTFOUND;
			}
		}
		break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in TTFONT definition");
		return STATUS_FAILED;
	}

	// create at least one layer
	if (_layers.size() == 0) {
		BaseTTFontLayer *layer = new BaseTTFontLayer;
		layer->_color = baseColor;
		_layers.add(layer);
	}

	if (!_fontFile) {
		BaseUtils::setString(&_fontFile, "arial.ttf");
	}

	return initFont();
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontTT::parseLayer(BaseTTFontLayer *layer, char *buffer) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(OFFSET_X)
	TOKEN_TABLE(OFFSET_Y)
	TOKEN_TABLE(COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_OFFSET_X:
			parser.scanStr(params, "%d", &layer->_offsetX);
			break;

		case TOKEN_OFFSET_Y:
			parser.scanStr(params, "%d", &layer->_offsetY);
			break;

		case TOKEN_COLOR: {
			int r, g, b;
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			layer->_color = BYTETORGBA(r, g, b, RGBCOLGetA(layer->_color));
		}
		break;

		case TOKEN_ALPHA: {
			int a;
			parser.scanStr(params, "%d", &a);
			layer->_color = BYTETORGBA(RGBCOLGetR(layer->_color), RGBCOLGetG(layer->_color), RGBCOLGetB(layer->_color), a);
		}
		break;

		default:
			break;
		}
	}
	if (cmd != PARSERR_EOF) {
		return STATUS_FAILED;
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseFontTT::persist(BasePersistenceManager *persistMgr) {
	BaseFont::persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_isBold));
	persistMgr->transferBool(TMEMBER(_isItalic));
	persistMgr->transferBool(TMEMBER(_isUnderline));
	persistMgr->transferBool(TMEMBER(_isStriked));
	persistMgr->transferSint32(TMEMBER(_fontHeight));
	persistMgr->transferCharPtr(TMEMBER(_fontFile));
	persistMgr->transferSint32(TMEMBER_INT(_charset));


	// persist layers
	int32 numLayers;
	if (persistMgr->getIsSaving()) {
		numLayers = _layers.size();
		persistMgr->transferSint32(TMEMBER(numLayers));
		for (int i = 0; i < numLayers; i++) {
			_layers[i]->persist(persistMgr);
		}
	} else {
		numLayers = _layers.size();
		persistMgr->transferSint32(TMEMBER(numLayers));
		for (int i = 0; i < numLayers; i++) {
			BaseTTFontLayer *layer = new BaseTTFontLayer;
			layer->persist(persistMgr);
			_layers.add(layer);
		}
	}

	if (!persistMgr->getIsSaving()) {
		for (int i = 0; i < NUM_CACHED_TEXTS; i++) {
			_cachedTexts[i] = nullptr;
		}
		_fallbackFont = _font = _deletableFont = nullptr;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void BaseFontTT::afterLoad() {
	initFont();
}

//////////////////////////////////////////////////////////////////////////
bool BaseFontTT::initFont() {
	if (!_fontFile) {
		return STATUS_FAILED;
	}
#ifdef USE_FREETYPE2
	Common::String fallbackFilename;
	// Handle Bold atleast for the fallback-case.
	// TODO: Handle italic. (Needs a test-case)
	if (_isBold) {
		fallbackFilename = "FreeSansBold.ttf";
	} else {
		fallbackFilename = "FreeSans.ttf";
	}

	Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(_fontFile);
	if (!file) {
		if (Common::String(_fontFile) != "arial.ttf") {
			warning("%s has no replacement font yet, using FreeSans for now (if available)", _fontFile);
		}
		// Fallback1: Try to find FreeSans.ttf
		file = SearchMan.createReadStreamForMember(fallbackFilename);
	}

	if (file) {
		_deletableFont = Graphics::loadTTFFont(*file, _fontHeight, Graphics::kTTFSizeModeCharacter, 96); // Use the same dpi as WME (96 vs 72).
		_font = _deletableFont;
		BaseFileManager::getEngineInstance()->closeFile(file);
		file = nullptr;
	}

	// Fallback2: Try load the font from the common fonts archive:
	if (!_font) {
		_deletableFont = Graphics::loadTTFFontFromArchive(fallbackFilename, _fontHeight, Graphics::kTTFSizeModeCharacter, 96); // Use the same dpi as WME (96 vs 72).
		_font = _deletableFont;
	}

	// Fallback3: Try to ask FontMan for the FreeSans.ttf ScummModern.zip uses:
	if (!_font) {
		// Really not desireable, as we will get a font with dpi-72 then
		Common::String fontName = Common::String::format("%s-%s@%d", fallbackFilename.c_str(), "ASCII", _fontHeight);
		warning("Looking for %s", fontName.c_str());
		_font = FontMan.getFontByName(fontName);
	}
#else
	warning("BaseFontTT::InitFont - FreeType2-support not compiled in, TTF-fonts will not be loaded");
#endif // USE_FREETYPE2

	// Fallback4: Just use the Big GUI-font. (REALLY undesireable)
	if (!_font) {
		_font = _fallbackFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		warning("BaseFontTT::InitFont - Couldn't load font: %s", _fontFile);
	}
	_lineHeight = _font->getFontHeight();
#ifdef ENABLE_FOXTAIL
	if (BaseEngine::instance().isFoxTail(FOXTAIL_1_2_896, FOXTAIL_LATEST_VERSION)) {
		_lineHeight -= 1;
	}
#endif
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseFontTT::measureText(const WideString &text, int maxWidth, int maxHeight, int &textWidth, int &textHeight) {
	//TextLineList lines;

	if (maxWidth >= 0) {
		Common::Array<WideString> lines;
		_font->wordWrapText(text, maxWidth, lines);
		Common::Array<WideString>::iterator it;
		textWidth = 0;
		for (it = lines.begin(); it != lines.end(); ++it) {
			textWidth = MAX(textWidth, _font->getStringWidth(*it));
		}

		//WrapText(text, maxWidth, maxHeight, lines);

		textHeight = (int)(lines.size() * getLineHeight());
	} else {
		textWidth = _font->getStringWidth(text);
		textHeight = _fontHeight;
	}
	/*
	    TextLineList::iterator it;
	    for (it = lines.begin(); it != lines.end(); ++it) {
	        TextLine *line = (*it);
	        textWidth = MAX(textWidth, line->GetWidth());
	        delete line;
	        line = nullptr;
	    }*/
}

} // End of namespace Wintermute
