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

#ifndef GRAPHICS_KORFONT_H
#define GRAPHICS_KORFONT_H

#include "common/str.h"
#include "graphics/surface.h"

namespace Graphics {

/**
 * @defgroup graphics_fontman Korean font
 * @ingroup graphics
 *
 * @brief FontKorean class used to handle Korean characters.
 *
 * @{
 */

/**
 * A font that is able to draw Korean encoded characters.
 */
class FontKorean {
public:
	virtual ~FontKorean() {}

	/**
	 * Creates the first Korean font, which ROM/font file is present.
	 * It will also call loadData, so the user can just start
	 * using the font.
	 *
	 * The last file tried is ScummVM's Korean.FNT file.
	 */
	static FontKorean *createFont(const char * fontFile);

	/**
	 * Load the font data.
	 */
	virtual bool loadData(const char *fontFile) = 0;

	/**
	 * Enable drawing with outline or shadow if supported by the Font.
	 *
	 * After changing outline state, getFontHeight and getMaxFontWidth / getCharWidth might return
	 * different values!
	 */
	enum DrawingMode {
		kDefaultMode,
		kOutlineMode,
		kShadowMode
	};

	virtual void setDrawingMode(DrawingMode mode) {}

	/**
	 * Enable flipped character drawing if supported by the Font (e.g. in the MI1 circus scene after Guybrush gets shot out of the cannon).
	 */
	virtual void toggleFlippedMode(bool enable) {}

	/**
	 * Set spacing between characters and lines. This affects font height / char width
	 */
	virtual void setCharSpacing(int spacing) {}
	virtual void setLineSpacing(int spacing) {}

	/**
	 * Returns the height of the font.
	 */
	virtual uint getFontHeight() const = 0;

	/**
	 * Returns the max. width of the font.
	 */
	virtual uint getMaxFontWidth() const = 0;

	/**
	 * Returns the width of a specific character.
	 */
	virtual uint getCharWidth(uint16 ch) const = 0;

	/**
	 * Draws a Korean encoded character on the given surface.
	 */
	void drawChar(Graphics::Surface &dst, uint16 ch, int x, int y, uint32 c1, uint32 c2) const;

	/**
	 * Draws a Korean char on the given raw buffer.
	 *
	 * @param dst   pointer to the destination
	 * @param ch    character to draw (in little endian)
	 * @param pitch pitch of the destination buffer (size in *bytes*)
	 * @param bpp   bytes per pixel of the destination buffer
	 * @param c1    forground color
	 * @param c2    outline color
	 * @param maxW  max draw width (to ensure that character drawing takes place within surface boundaries), -1 = no check
	 * @param maxH  max draw height (to ensure that character drawing takes place within surface boundaries), -1 = no check
	 */
	virtual void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW, int maxH) const = 0;
};

/**
 * A base class to render monochrome Korean fonts.
 */
class FontKoreanBase : public FontKorean {
public:
	FontKoreanBase();

	virtual void setDrawingMode(DrawingMode mode);

	virtual void toggleFlippedMode(bool enable);

	virtual uint getFontHeight() const;

	virtual uint getMaxFontWidth() const;

	virtual uint getCharWidth(uint16 ch) const;

	virtual void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW, int maxH) const;
private:
	template<typename Color>
	void blitCharacter(const uint8 *glyph, const int w, const int h, uint8 *dst, int pitch, Color c) const;
	void createOutline(uint8 *outline, const uint8 *glyph, const int w, const int h) const;

protected:
	DrawingMode _drawMode;
	bool _flippedMode;
	int _fontWidth, _fontHeight;
	uint8 _bitPosNewLineMask;

	bool isASCII(uint16 ch) const;

	virtual const uint8 *getCharData(uint16 c) const = 0;

	enum DrawingFeature {
		kFeatDefault        = 1 << 0,
		kFeatOutline        = 1 << 1,
		kFeatShadow         = 1 << 2,
		kFeatFMTownsShadow  = 1 << 3,
		kFeatFlipped        = 1 << 4
	};

	virtual bool hasFeature(int feat) const = 0;
};

/**
 * Our custom Korean FNT.
 */
class FontKoreanSVM : public FontKoreanBase {
public:
	FontKoreanSVM();
	~FontKoreanSVM();
	/**
	 * Load the font data from "KOREAN.FNT".
	 */
	bool loadData(const char *fontFile);
private:
	uint8 *_fontData16x16;
	uint _fontData16x16Size;

	uint8 *_fontData8x16;
	uint _fontData8x16Size;

	uint8 *_fontData8x8;
	uint _fontData8x8Size;

	virtual const uint8 *getCharData(uint16 c) const;

	bool hasFeature(int feat) const;

	const uint8 *getCharDataPCE(uint16 c) const;
	const uint8 *getCharDataDefault(uint16 c) const;

	enum {
		kKoreanFontVersion = 3
	};
};

/**
 * Korean Wansung compatible font.
 */
class FontKoreanWansung : public FontKoreanBase {
public:
	FontKoreanWansung();
	~FontKoreanWansung();
	/**
	 * Loads the ROM data from "KOREAN#.FNT".
	 */
	bool loadData(const char *fontFile);
private:
	enum {
		eFontNumChars = 256,
		kFontNumChars = 2530
	};

	int _fontShadow;
	uint8 *_fontData;
	uint _fontDataSize;

	int _englishFontWidth;
	int _englishFontHeight;
	uint8 *_englishFontData;
	uint _englishFontDataSize;


	virtual const uint8 *getCharData(uint16 c) const;

	bool hasFeature(int feat) const;

	bool englishLoadData(const char *fontFile);
};
 /** @} */
} // End of namespace Graphics

#endif
