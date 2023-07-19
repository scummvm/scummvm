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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_TEXTMANAGER_H
#define CRAB_TEXTMANAGER_H

#include "crab/GameParam.h"
#include "crab/image/Image.h"
#include "crab/text/color.h"
#include "crab/common_header.h"
#include "crab/vectors.h"

namespace Crab {

// We use this object as the key for all fonts
typedef unsigned int FontKey;

// Since we use unsigned int as a key for images, our loadImgKey function is loadNum
#define LoadFontKey loadNum

namespace pyrodactyl {
namespace text {
class TextManager {
	// The collection of stored fonts
	Common::Array<Graphics::Font *> _font;

	// The size of the cache
	int _cacheSize;

	// The padding at the end of the background - this needs to be loaded from file later
	Vector2i _padBg;

	// The data stored in our cache - text and texture
	struct TextCacheUnit {
		Common::String _text;
		int _col;
		FontKey _font;

		pyrodactyl::image::Image _img;
		bool _empty;

		TextCacheUnit() {
			_empty = true;
			_col = 0;
			_font = 0;
		}
		~TextCacheUnit() {
			_img.deleteImage();
		}

		bool EqualCol(int color) {
			return _col == color;
		}
	};

	// Text Cache - done to avoid having to render a texture every time text is drawn
	Common::Array<TextCacheUnit> _cache;

	// The oldest element in the text cache
	int _oldest;

	// The place to store all colors
	ColorPool _colpool;

	// The rectangle used to store the darkened rectangle coordinates
	Rect _rect;

	int search(const Common::String &text, int col, FontKey fontid);
	int findFreeSlot();

public:
	TextManager(void) {
		_oldest = 0;
		_cacheSize = 30;
	}

	~TextManager(void) {}

	void init();
	void quit();
	void reset();

	Graphics::Font *getFont(const FontKey &fontid) {
		return _font[fontid];
	}

	Graphics::ManagedSurface *renderTextBlended(const FontKey &font, const Common::String &text, const int &color);
#if 0
	SDL_Surface *RenderTextBlended(const FontKey &font, const Common::String &text, const int &color);
#endif

	void draw(const int &x, const int &y, const Common::String &text, const int &color,
			  const FontKey &font = 0, const Align &align = ALIGN_LEFT, const bool &background = false);

	void draw(const int &x, int y, const Common::String &text, const int &color, const FontKey &font, const Align &align,
			  const unsigned int &lineWidth, const unsigned int &lineHeight, const bool &background = false);
};

} // End of namespace text
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TEXTMANAGER_H
