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
	Common::Array<Graphics::Font *> font;

	// The size of the cache
	int cache_size;

	// The padding at the end of the background - this needs to be loaded from file later
	Vector2i pad_bg;

	// The data stored in our cache - text and texture
	struct TextCacheUnit {
		Common::String text;
		int col;
		FontKey font;

		pyrodactyl::image::Image img;
		bool empty;

		TextCacheUnit() {
			empty = true;
			col = 0;
			font = 0;
		}
		~TextCacheUnit() { img.Delete(); }

		bool EqualCol(int color) { return col == color; }
	};

	// Text Cache - done to avoid having to render a texture every time text is drawn
	Common::Array<TextCacheUnit> cache;

	// The oldest element in the text cache
	int oldest;

	// The place to store all colors
	ColorPool colpool;

	// The rectangle used to store the darkened rectangle coordinates
	Rect rect;

	int Search(const Common::String &text, int col, FontKey fontid);
	int FindFreeSlot();

public:
	TextManager(void) {
		oldest = 0;
		cache_size = 30;
	}
	~TextManager(void) {}

	void Init();
	void Quit();
	void Reset();


	Graphics::Font *GetFont(const FontKey &fontid) { return font[fontid]; }
	Graphics::ManagedSurface *RenderTextBlended(const FontKey &font, const Common::String &text, const int &color);
#if 0
	SDL_Surface *RenderTextBlended(const FontKey &font, const Common::String &text, const int &color);
#endif

	void draw(const int &x, const int &y, const Common::String &text, const int &color,
			  const FontKey &font = 0, const Align &align = ALIGN_LEFT, const bool &background = false);

	void draw(const int &x, int y, const Common::String &text, const int &color, const FontKey &font, const Align &align,
			  const unsigned int &line_width, const unsigned int &line_height, const bool &background = false);
};

} // End of namespace text
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TEXTMANAGER_H
