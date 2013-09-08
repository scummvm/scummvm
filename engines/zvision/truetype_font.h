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

// This file is based on engines/wintermute/base/fonts/base_font_truetype.h/.cpp

#ifndef ZVISION_TRUETYPE_FONT_H
#define ZVISION_TRUETYPE_FONT_H

#include "common/types.h"

#include "graphics/font.h"


namespace Graphics {
struct Surface;
}

namespace ZVision {

class ZVision;

class TruetypeFont {
public:
	TruetypeFont(ZVision *engine, int32 fontHeight, const Graphics::PixelFormat pixelFormat);
	~TruetypeFont();

public:
	enum {
		NUM_CACHED_TEXTS = 30
	};

	enum TextAlign {
		ALIGN_LEFT = 0,
		ALIGN_RIGHT,
		ALIGN_CENTER
	};

	class CachedText {
	public:
		Common::String _text;
		int32 _width;
		TextAlign _align;
		int32 _maxHeight;
		Graphics::Surface *_surface;
		int32 _textOffset;
		bool _marked;
		uint32 _lastUsed;

		CachedText() {
			_width = _maxHeight = -1;
			_align = ALIGN_LEFT;
			_surface = 0;
			_lastUsed = 0;
			_marked = false;
		}

		virtual ~CachedText() {
			delete _surface;
		}
	};

private:
	ZVision *_engine;
	const Graphics::PixelFormat _pixelFormat;

	Graphics::Font *_font;

	float _lineHeight;

	size_t _maxCharWidth;
	size_t _maxCharHeight;

	CachedText *_cachedTexts[NUM_CACHED_TEXTS];

public:
	bool _isBold;
	bool _isItalic;
	bool _isUnderline;
	bool _isStriked;
	int32 _fontHeight;

	//BaseArray<BaseTTFontLayer *> _layers;

public:
	int getTextWidth(const byte *text, int maxLength = -1);
	int getTextHeight(const byte *text, int width);
	void drawText(const Common::String &text, int x, int y, int width, TextAlign align = ALIGN_LEFT, int max_height = -1);
	int getLetterHeight();

	bool loadFile(const Common::String &filename);

	float getLineHeight() const { return _lineHeight; }
	
	void clearCache();

	static TruetypeFont *createFromFile(ZVision *game, const Common::String &filename);

private:
	void measureText(const Common::String &text, int maxWidth, int maxHeight, int &textWidthOut, int &textHeightOut);
	Graphics::Surface *renderTextToTexture(const Common::String &text, int width, TextAlign align, int maxHeight);
};

} // End of namespace ZVision

#endif
