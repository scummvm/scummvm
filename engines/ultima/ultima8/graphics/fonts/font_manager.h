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

#ifndef ULTIMA8_GRAPHICS_FONTS_FONTMANAGER_H
#define ULTIMA8_GRAPHICS_FONTS_FONTMANAGER_H

#include "ultima/shared/std/containers.h"
#include "graphics/font.h"

namespace Ultima {
namespace Ultima8 {

class Font;

// This is TTF_Font struct
typedef struct _TTF_Font TTF_Font;

class TTFont;


class FontManager {
private:
	struct TTFId {
		Std::string _filename;
		int _pointSize;
		bool operator<(const TTFId &other) const {
			return (_pointSize < other._pointSize ||
			        (_pointSize == other._pointSize &&
			         _filename < other._filename));
		}
	};

	struct TTFHash {
		uint operator()(const TTFId &x) const {
			// TODO: See if something better can be used as a hash key
			int64 val = (int64)&x;
			return (uint)val;
		}
	};
	struct TTFEqual {
		bool operator()(const TTFId &x, const TTFId &y) const {
			return x._filename == y._filename && x._pointSize == y._pointSize;
		}
	};

	typedef Common::HashMap<TTFId, Graphics::Font *, TTFHash, TTFEqual> TTFFonts;
	TTFFonts _ttfFonts;

	//! Get a (possibly cached) TTF_Font structure for filename/pointsize,
	//! loading it if necessary.
	Graphics::Font *getTTF_Font(const Std::string &filename, int pointsize, bool antialiasing);

	//! Override fontnum with specified font
	void setOverride(unsigned int fontnum, Font *newFont);

	Std::vector<Font *> _overrides;

	Std::vector<Font *> _ttFonts;

	static FontManager *_fontManager;
public:
	FontManager();
	~FontManager();

	static FontManager *get_instance() {
		return _fontManager;
	}

	//! get a Font by fontnum (for game fonts)
	//! \param fontnum the number of the font
	//! \param allowOverride if true, allow an override font to be used
	Font *getGameFont(unsigned int fontnum,
	                             bool allowOverride = false);

	//! get a TTF font (for non-game fonts)
	Font *getTTFont(unsigned int ttfnum);

	//! override a game font with a TTF.
	//! \param fontnum the font to override
	//! \param filename the filename of the TTF
	//! \param pointsize the pointsize to use
	//! \param rgb the color to use for the font
	//! \param bordersize the size of the black border to add
	//! \param SJIS true for a Japanese game font
	bool addTTFOverride(unsigned int fontnum, const Std::string &filename,
	                    int pointsize, uint32 rgb, int bordersize,
	                    bool SJIS = false);

	//! override a game font with a Japanese font.
	//! \param fontnum the font to override
	//! \param jpfont the fontnum of the Japanese font to use
	//! \param rgb the color to use
	bool addJPOverride(unsigned int fontnum, unsigned int jpfont, uint32 rgb);

	//! load a TTF (for non-game fonts)
	bool loadTTFont(unsigned int ttfnum, const Std::string &filename,
	                int pointsize, uint32 rgb, int bordersize);

	// Reset the game fonts
	void resetGameFonts();
};

} // End of namespace Ultima8
} // End of namespace Ultima // End of namespace Ultima8

#endif
