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
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/dcgf.h"

#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"
#include "common/unicode-bidi.h"
#include "common/compression/unzip.h"

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
BaseFontTT::~BaseFontTT() {
	clearCache();

	for (int32 i = 0; i < _layers.getSize(); i++) {
		delete _layers[i];
	}
	_layers.removeAll();

	SAFE_DELETE_ARRAY(_fontFile);

	SAFE_DELETE(_deletableFont);

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
}

//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getTextWidth(const byte *text, int maxLength) {
	WideString textStr;

	if (_game->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((const char *)text);
	} else {
		textStr = StringUtil::ansiToWide((const char *)text, _charset);
	}

	if (maxLength >= 0 && (int)textStr.size() > maxLength) {
		textStr = textStr.substr(0, (uint32)maxLength);
	}

	int textWidth, textHeight;
	measureText(textStr, -1, -1, textWidth, textHeight);

	return textWidth;
}

//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getTextHeight(const byte *text, int width) {
	WideString textStr;

	if (_game->_textEncoding == TEXT_UTF8) {
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

	if (_game->_textEncoding == TEXT_UTF8) {
		textStr = StringUtil::utf8ToWide((const char *)text);
	} else {
		textStr = StringUtil::ansiToWide((const char *)text, _charset);
	}

	if (maxLength >= 0 && textStr.size() > (uint32)maxLength) {
		textStr = textStr.substr(0, (uint32)maxLength);
	}

	BaseRenderer *renderer = _game->_renderer;

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
				_cachedTexts[i]->_lastUsed = BasePlatform::getTime();
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
			_cachedTexts[minIndex]->_lastUsed = BasePlatform::getTime();
		}
	}


	// and paint it
	if (surface) {
		Common::Rect32 rc;
		BasePlatform::setRect(&rc, 0, 0, surface->getWidth(), surface->getHeight());
		for (int32 i = 0; i < _layers.getSize(); i++) {
			uint32 color = _layers[i]->_color;
			uint32 origForceAlpha = renderer->_forceAlphaColor;
			if (renderer->_forceAlphaColor != 0) {
				color = BYTETORGBA(RGBCOLGetR(color), RGBCOLGetG(color), RGBCOLGetB(color), RGBCOLGetA(renderer->_forceAlphaColor));
				renderer->_forceAlphaColor = 0;
			}
			surface->displayTrans(x, y - textOffset, rc, color, Graphics::BLEND_NORMAL, false, false, _layers[i]->_offsetX, _layers[i]->_offsetY);

			renderer->_forceAlphaColor = origForceAlpha;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
BaseSurface *BaseFontTT::renderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset) {
	TextLineList lines;
	int32 heightAfterWrapping;

	/* The text will be wrapped to fit into the width and height as specified.
	 * In case there was too much text, it is truncated as soon as maxHeight is exceeded.
	 * Here, the distance between two lines is taken from the _lineHeight value (coming from the font definition).
	 */
	heightAfterWrapping = wrapText(text, width, maxHeight, lines);

	TextLineList::iterator it;

	/* The surface to render the text onto is equal in width, but the height is computed differently.
	 * _maxCharHeight is the Y size of the bounding box of all characters.
	 *
	 * There could be fonts that have incorrect parameters set. The freetype doc says that the value used
	 * for _lineHeight does not assure that all glyphs will "fit" into this.
	 *
	 * It won't be possible to "fix" everything, but at least try our best with obvious failures.
	 * So if the resulting text height from the computation of "WrapText" is bigger than the
	 * size computed below, adjust it appropriately. Later when drawing the glyphs, the _lineHeight
	 * is used anyway as line distance, so checking the "textHeight" for sanity is not a bad idea.
	 *
	 */
	int32 textHeight = lines.size() * (_lineHeight + _font->getFontAscent());
	if (heightAfterWrapping > textHeight) {
		_game->LOG(0, "Strange font definitions. Text height %d smaller than line height %d.", textHeight, heightAfterWrapping);
		textHeight = heightAfterWrapping;
	}

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create((uint16)width, (uint16)(textHeight), _game->_renderer->getPixelFormat());

	Graphics::TextAlign alignment = Graphics::kTextAlignInvalid;
	if (align == TAL_LEFT) {
		alignment = Graphics::kTextAlignLeft;
	} else if (align == TAL_CENTER) {
		alignment = Graphics::kTextAlignCenter;
	} else if (align == TAL_RIGHT) {
		alignment = Graphics::kTextAlignRight;
	}

	// TODO: _isUnderline, _isBold, _isItalic, _isStriked

	uint32 useColor = 0xffffffff;
	int heightOffset = 0;
	// W/A for 'Shadows on the Vatican - Act I: Greed'
	if (BaseEngine::instance().getGameId() == "sotv1" &&
		Common::String(_fontFile).equals("fonts\\Laffayette_Comic_Pro.ttf")) {
		heightOffset = 2;
	}
	// W/A for 'Shadows on the Vatican - Act II: Wrath'
	if (BaseEngine::instance().getGameId() == "sotv2" &&
		Common::String(_fontFile).equals("fonts\\Laffayette_Comic_Pro.ttf")) {
		heightOffset = 2;
	}

	for (it = lines.begin(); it != lines.end(); ++it) {
		TextLine *line = (*it);
		WideString str, lineStr = line->getText();
		if (_game->_textRTL) {
			str = Common::convertBiDiU32String(lineStr, Common::BIDI_PAR_RTL);
		} else {
			str = Common::convertBiDiU32String(lineStr, Common::BIDI_PAR_LTR);
		}
		_font->drawAlphaString(surface, str, 0, heightOffset, width, useColor, alignment);
		heightOffset += (int)_lineHeight;
	}

	BaseSurface *retSurface = _game->_renderer->createSurface();
	retSurface->create(surface->w, surface->h);
	retSurface->putSurface(*surface, true);
	surface->free();
	delete surface;
	return retSurface;
}


//////////////////////////////////////////////////////////////////////////
int BaseFontTT::getLetterHeight() {
	return (int)_lineHeight;
}


//////////////////////////////////////////////////////////////////////
bool BaseFontTT::loadFile(const char *filename) {
	char *buffer = (char *)_game->_fileManager->readWholeFile(filename);
	if (buffer == nullptr) {
		_game->LOG(0, "BaseFontTT::loadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer))) {
		_game->LOG(0, "Error parsing TTFONT file '%s'", filename);
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
	BaseParser parser(_game);

	if (parser.getCommand(&buffer, commands, &params) != TOKEN_TTFONT) {
		_game->LOG(0, "'TTFONT' keyword expected.");
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
				SAFE_DELETE(layer);
				cmd = PARSERR_TOKENNOTFOUND;
			}
		}
		break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_game->LOG(0, "Syntax error in TTFONT definition");
		return STATUS_FAILED;
	}

	// create at least one layer
	if (_layers.getSize() == 0) {
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
	BaseParser parser(_game);

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
		numLayers = _layers.getSize();
		persistMgr->transferSint32(TMEMBER(numLayers));
		for (int i = 0; i < numLayers; i++) {
			_layers[i]->persist(persistMgr);
		}
	} else {
		numLayers = _layers.getSize();
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
	}

	// initialise to defaults
	if (!persistMgr->getIsSaving()) {
		_fallbackFont = _font = _deletableFont = nullptr;
		_lineHeight = 0;
		_maxCharWidth = _maxCharHeight = 0;
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
	Common::SeekableReadStream *file;
	if (BaseEngine::instance().getGameId() == "alimardan1" &&
		Common::String(_fontFile).contains("Children.ttf")) {
		// In the game 'Alimardan's Mischief', file 'Asap-Regular.font'
		// points to "Children.ttf", however there is 'Asap-Regular.ttf'
		// in game data directory.
		file = BaseFileManager::getEngineInstance()->openFile("fonts\\Asap-Regular.ttf", true, false);
	} else if (BaseEngine::instance().getGameId() == "nosebound1" &&
		   Common::String(_fontFile).hasPrefix("fonts\\")) {
		// The game 'Nose Bound Episode 1' is points to 'fonts' directory,
		// however fonts located are in game root data directory.
		Common::String font = Common::String(_fontFile);
		font = font.substr(sizeof("fonts\\") - 1);
		file = BaseFileManager::getEngineInstance()->openFile(font, true, false);
	} else if (BaseEngine::instance().getGameId() == "nosebound1" &&
		   Common::String(_fontFile).contains("bettynoir.ttf")) {
		// The game 'Nose Bound Episode 1' is points to 'fonts\Andes.ttf',
		// however there is in game root data directory.
		file = BaseFileManager::getEngineInstance()->openFile("bettynoir.ttf", true, false);
	} else if (BaseEngine::instance().getGameId() == "todaymama") {
		// Fonts from the game 'Today, Mama!' are not working. Using fallback.
		file = nullptr;
	} else {
		// Load a file, but avoid having the File-manager handle the disposal of it.
		file = BaseFileManager::getEngineInstance()->openFile(_fontFile, true, false);
	}
	if (file) {
		_deletableFont = Graphics::loadTTFFont(file, DisposeAfterUse::YES, _fontHeight, Graphics::kTTFSizeModeCharacter, 96); // Use the same dpi as WME (96 vs 72).
		_font = _deletableFont;
	}

	// Fallback1: Try load the font from the common fonts archive:
	if (!_font) {
		const char *fallbackFilename;
		if (Common::String(_fontFile).contains("cyberbit.ttf")) {
			if (_isBold) {
				fallbackFilename = "LiberationSerif-Bold.ttf";
			} else {
				fallbackFilename = "LiberationSerif-Regular.ttf";
			}
		} else if (Common::String(_fontFile).contains("arial.ttf")) {
			if (_isBold) {
				fallbackFilename = "LiberationSans-Bold.ttf";
			} else {
				fallbackFilename = "LiberationSans-Regular.ttf";
			}
		} else if (Common::String(_fontFile).contains("Oceania-begular.ttf")) {
			if (_isBold) {
				fallbackFilename = "LiberationSans-Bold.ttf";
			} else {
				fallbackFilename = "LiberationSans-Regular.ttf";
			}
		} else {
			if (BaseEngine::instance().getGameId() == "alphapolaris" &&
			    BaseEngine::instance().getLanguage() == Common::Language::PL_POL &&
			    Common::String(_fontFile).contains("Oceania")) {
				// Polish version of 'Alpha Polaris' has missing 'Oceania' fonts.
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("framd.ttf")) {
				// Several 'Carol Reed' games has missing 'framd.ttf' fonts
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("BDAVAT.TTF")) {
				// 'Forgotten Sound 1 - Revelation' game has missing 'BDAVAT.TTF' font
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("verdana.ttf")) {
				// 'Nose Bound Episode 1' game has missing 'verdana.ttf' font
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("Sansation")) {
				// 'Shadow Of Nebula' game has missing 'Sansation' fonts
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("comic.ttf")) {
				// 'The Trader of Stories' game has missing 'Comic' font
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("BREEZE.TTF") ||
				   Common::String(_fontFile).contains("PRN55__C.TTF")) {
				// Fonts from the game 'Today, Mama!' are not working.
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
			} else if (Common::String(_fontFile).contains("ITCBLKAD.ttf")) {
				// 'The Trader of Stories' game has missing 'ITCBLKAD' font
				fallbackFilename = "NotoSerif-Italic.ttf";
			} else if (Common::String(_fontFile).contains("phalls_khodkar.font")) {
				// 'The Way Of Love Sub Zero' game ask for wrong resource font.
				// falback to default
				fallbackFilename = "LiberationSans-Regular.ttf";
			} else {
				if (_isBold) {
					fallbackFilename = "LiberationSans-Bold.ttf";
				} else {
					fallbackFilename = "LiberationSans-Regular.ttf";
				}
				warning("%s has no replacement font yet, using %s for now (if available)", _fontFile, fallbackFilename);
			}
		}

		_deletableFont = Graphics::loadTTFFontFromArchive(fallbackFilename, _fontHeight, Graphics::kTTFSizeModeCharacter, 96); // Use the same dpi as WME (96 vs 72).
		_font = _deletableFont;
	}
#else
	warning("BaseFontTT::InitFont - FreeType2-support not compiled in, TTF-fonts will not be loaded");
#endif // USE_FREETYPE2

	// Fallback2: Just use the Big GUI-font. (REALLY undesirable)
	if (!_font) {
		_font = _fallbackFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		warning("BaseFontTT::InitFont - Couldn't load font: %s", _fontFile);
	}

	auto box = _font->getBoundingBox("Ay");
	_lineHeight = MAX(box.bottom - box.top, _font->getFontHeight());

#ifdef ENABLE_FOXTAIL
	if (BaseEngine::instance().isFoxTail(FOXTAIL_1_2_896, FOXTAIL_LATEST_VERSION)) {
		_lineHeight -= 1;
	}
#endif

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
int32 BaseFontTT::wrapText(const WideString &text, int32 maxWidth, int32 maxHeight, TextLineList &lines) {
	int32 currWidth = 0;
	wchar_t prevChar = L'\0';
	int32 prevSpaceIndex = -1;
	int32 prevSpaceWidth = 0;
	int32 lineStartIndex = 0;

	for (size_t i = 0; i < text.size(); i++) {
		wchar_t ch = text[i];

		/* remember the last space character in the string
		 * for wrapping the line later if necessary
		 */
		if (ch == L' ') {
			prevSpaceIndex = i;
			prevSpaceWidth = currWidth;
		}

		int32 charWidth = 0;

		/* measure width of this char
		 * (advanceX + kerning)
		 */
		if (ch != L'\n') {
			float kerning = 0;
			if (prevChar != L'\0') {
				kerning = getKerning(prevChar, ch);
			}
			prevChar = ch;

			/* Small, but important difference! The computation of width must
			 * match the one from the rendering EXACTLY, including precision
			 * loss from casting. Otherwise, the bounds of the surface will
			 * be exceeded.
			 *
			 */
			charWidth = (((int32)_font->getCharWidth(ch)) + ((int32)kerning));
		}

		bool lineTooLong = maxWidth >= 0 && currWidth + charWidth > maxWidth;
		bool breakOnSpace = false;

		// we can't fit even a single character
		if (lineTooLong && currWidth == 0) {
			break;
		}

		/* check if the text shall be wrapped
		 */
		if (ch == L'\n' || i == text.size() - 1 || lineTooLong) {
			int32 breakPoint, breakWidth;

			if (prevSpaceIndex >= 0 && lineTooLong) {
				/* we have a previous space character that we can wrap the text at */
				breakPoint = prevSpaceIndex;
				breakWidth = prevSpaceWidth;
				breakOnSpace = true;
			} else {
				/* need to break at the current position */
				breakPoint = i;
				breakWidth = currWidth;

				breakOnSpace = (ch == L'\n');

				// we're at the end, so "consume" the last character as well
				if (i == text.size() - 1) {
					breakPoint++;
					breakWidth += charWidth;
				}
			}

			/* max. height exceeded --> "discard" this line and all following text
			 * i.e. do not add it to the text line list, return immediately
			 */
			if (maxHeight >= 0 && ((int32)lines.size() + 1) * getLineHeight() > maxHeight) {
				// W/A,FIXME: font height can be bigger, do not exit if one line
				if (lines.size() != 0)
					break;
			}

			WideString line = text.substr(lineStartIndex, breakPoint - lineStartIndex);
			lines.push_back(new TextLine(line, breakWidth));

			/* reset all values for the next line */
			currWidth = 0;
			prevChar = L'\0';
			prevSpaceIndex = -1;

			/* swallow (discard) spaces when breaking line */
			if (breakOnSpace) {
				breakPoint++;
			}

			lineStartIndex = breakPoint;
			i = breakPoint - 1;

			continue;
		}

		// if (ch == L' ' && currLine.empty()) continue;
		currWidth += charWidth;
	}

	// return the height of the "accepted" text
	return (lines.size() * getLineHeight());
}

//////////////////////////////////////////////////////////////////////////
void BaseFontTT::measureText(const WideString &text, int maxWidth, int maxHeight, int &textWidth, int &textHeight) {
	TextLineList lines;
	wrapText(text, maxWidth, maxHeight, lines);

	textHeight = lines.size() * getLineHeight();
	textWidth = 0;

	TextLineList::iterator it;
	for (it = lines.begin(); it != lines.end(); ++it) {
		TextLine *line = (*it);
		textWidth = MAX<int>(textWidth, line->getWidth());
		SAFE_DELETE(line);
	}
}

//////////////////////////////////////////////////////////////////////////
float BaseFontTT::getKerning(wchar_t leftChar, wchar_t rightChar) {
	return _font->getKerningOffset(leftChar, rightChar);
}

} // End of namespace Wintermute
