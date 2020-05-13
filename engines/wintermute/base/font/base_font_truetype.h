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

#ifndef WINTERMUTE_BASE_FONTTT_H
#define WINTERMUTE_BASE_FONTTT_H

#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "graphics/font.h"

#define NUM_CACHED_TEXTS 30

namespace Wintermute {

class BaseFontTT : public BaseFont {
private:
	//////////////////////////////////////////////////////////////////////////
	class BaseCachedTTFontText {
	public:
		WideString _text;
		int32 _width;
		TTextAlign _align;
		int32 _maxHeight;
		int32 _maxLength;
		BaseSurface *_surface;
		int32 _priority;
		int32 _textOffset;
		bool _marked;
		uint32 _lastUsed;

		BaseCachedTTFontText() : _text() {
			//_text = L"";
			_width = _maxHeight = _maxLength = -1;
			_align = TAL_LEFT;
			_surface = nullptr;
			_textOffset = 0;
			_lastUsed = 0;
			_marked = false;
		}

		virtual ~BaseCachedTTFontText() {
			if (_surface) {
				delete _surface;
			}
		}
	};

public:
	//////////////////////////////////////////////////////////////////////////
	class BaseTTFontLayer {
	public:
		BaseTTFontLayer() {
			_offsetX = _offsetY = 0;
			_color = 0x00000000;
		}

		bool persist(BasePersistenceManager *persistMgr) {
			persistMgr->transferSint32(TMEMBER(_offsetX));
			persistMgr->transferSint32(TMEMBER(_offsetY));
			persistMgr->transferUint32(TMEMBER(_color));
			return STATUS_OK;
		}

		int32 _offsetX;
		int32 _offsetY;
		uint32 _color;
	};

public:
	DECLARE_PERSISTENT(BaseFontTT, BaseFont)
	BaseFontTT(BaseGame *inGame);
	~BaseFontTT(void) override;

	int getTextWidth(const byte *text, int maxLength = -1) override;
	int getTextHeight(const byte *text, int width) override;
	void drawText(const byte *text, int x, int y, int width, TTextAlign align = TAL_LEFT, int max_height = -1, int maxLength = -1) override;
	int getLetterHeight() override;

	bool loadBuffer(char *buffer);
	bool loadFile(const Common::String &filename);

	float getLineHeight() const {
		return _lineHeight;
	}

	void afterLoad() override;
	void initLoop() override;

private:
	bool parseLayer(BaseTTFontLayer *layer, char *buffer);

	void measureText(const WideString &text, int maxWidth, int maxHeight, int &textWidth, int &textHeight);

	BaseSurface *renderTextToTexture(const WideString &text, int width, TTextAlign align, int maxHeight, int &textOffset);

	BaseCachedTTFontText *_cachedTexts[NUM_CACHED_TEXTS];

	bool initFont();

	Graphics::Font *_deletableFont;
	const Graphics::Font *_font;
	const Graphics::Font *_fallbackFont;

	float _lineHeight;

	size_t _maxCharWidth;
	size_t _maxCharHeight;

private:
	bool _isBold;
	bool _isItalic;
	bool _isUnderline;
	bool _isStriked;
	int32 _fontHeight;
	char *_fontFile;
	TTextCharset _charset;

	BaseArray<BaseTTFontLayer *> _layers;
	void clearCache();

};

} // End of namespace Wintermute

#endif
