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

#ifndef DGDS_FONT_H
#define DGDS_FONT_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/func.h"
#include "graphics/font.h"

#include "dgds/dgds.h"

namespace Graphics {
class Font;
struct Surface;
}

namespace Common {
class SeekableReadStream;
}

namespace Dgds {

class ResourceManager;
class Decompressor;

class DgdsFont : public Graphics::Font {
public:
	DgdsFont(byte w, byte h, byte start, byte count, const byte *glyphs);
	virtual ~DgdsFont();
	virtual int getFontHeight() const override { return _h; }
	virtual int getMaxCharWidth() const override { return _w; }
	virtual int getCharWidth(uint32 chr) const override = 0;
	static DgdsFont *load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor);

protected:
	byte _w;
	byte _h;
	byte _start;
	byte _count;
	const byte *_glyphs;

	void drawDgdsChar(Graphics::Surface* dst, int pos, int x, int y, int w, uint32 color) const;
	bool hasChar(byte chr) const;
	virtual int charOffset(byte chr) const = 0;
};

/* Proportional font (each char has its own width and so data is a different size) */
class PFont : public DgdsFont {
public:
	PFont(byte w, byte h, byte start, byte count, byte *data);
	~PFont();
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	static PFont *load(Common::SeekableReadStream &input, Decompressor *decompressor);

protected:
	const uint16 *_offsets;
	const byte *_widths;
	byte *_rawData;

	int charOffset(byte chr) const override;
};

/* Fixed-width font */
class FFont : public DgdsFont {
public:
	FFont(byte w, byte h, byte start, byte count, byte *data);
	~FFont();
	int getCharWidth(uint32 chr) const override { return _w; }
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	static FFont *load(Common::SeekableReadStream &input);

protected:
	byte *_rawData;

	int charOffset(byte chr) const override;
};

class FontManager {
public:
	enum FontType {
		kDefaultFont = 0,
		k8x8Font,
		k6x6Font,
		k4x5Font,
		kGameFont, 		// DRAGON for Rise of the Dragon, WILLY for Willy Beamish, HOC for Heart of China.
		kGameDlgFont,	// P6x6 for Rise of the Dragon, COMIX_16 for Willy Beamish, CHINESE for Heart of China
		k7x8Font,	  	// Rise of the Dragon only
		kVCRFont,    	// Willy Beamish only
		kChinaFont,   	// Heart of China only
	};

	FontManager() {}
	~FontManager();

	const DgdsFont *getFont(FontType) const;
	FontType fontTypeByName(const Common::String &filename) const;
	void loadFonts(DgdsGameId gameId, ResourceManager *resourceManager, Decompressor *decompressor);

private:
	void tryLoadFont(const char *filename, ResourceManager *resourceManager, Decompressor *decompressor);

	struct FontTypeHash {
		Common::Hash<const char *> hash;

		uint operator()(FontType val) const {
			return (uint)val;
		}
	};

	Common::HashMap<FontType, DgdsFont*, FontTypeHash> _fonts;
};

} // End of namespace Dgds


#endif // DGDS_FONT_H
