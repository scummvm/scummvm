#pragma once

#include "GameParam.h"
#include "Image.h"
#include "color.h"
#include "common_header.h"
#include "vectors.h"

// We use this object as the key for all fonts
typedef unsigned int FontKey;

// Since we use unsigned int as a key for images, our LoadImgKey function is LoadNum
#define LoadFontKey LoadNum

namespace pyrodactyl {
namespace text {
class TextManager {
	// The collection of stored fonts
	std::vector<TTF_Font *> font;

	// The size of the cache
	int cache_size;

	// The padding at the end of the background - this needs to be loaded from file later
	Vector2i pad_bg;

	// The data stored in our cache - text and texture
	struct TextCacheUnit {
		std::string text;
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
	std::vector<TextCacheUnit> cache;

	// The oldest element in the text cache
	int oldest;

	// The place to store all colors
	ColorPool colpool;

	// The rectangle used to store the darkened rectangle coordinates
	SDL_Rect rect;

	int Search(const std::string &text, int col, FontKey fontid);
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

	TTF_Font *GetFont(const FontKey &fontid) { return font[fontid]; }
	SDL_Surface *RenderTextBlended(const FontKey &font, const std::string &text, const int &color);

	void Draw(const int &x, const int &y, const std::string &text, const int &color,
			  const FontKey &font = 0, const Align &align = ALIGN_LEFT, const bool &background = false);

	void Draw(const int &x, int y, const std::string &text, const int &color, const FontKey &font, const Align &align,
			  const unsigned int &line_width, const unsigned int &line_height, const bool &background = false);
};

extern TextManager gTextManager;
} // End of namespace text
} // End of namespace pyrodactyl