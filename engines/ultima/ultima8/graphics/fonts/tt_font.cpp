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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/graphics/fonts/ttf_rendered_text.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/misc/encoding.h"

#include "common/ustr.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

//include iomanip

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFont, Pentagram::Font)

// various unicode characters which look like small black circles
static const uint16 BULLETS[] = { 0x2022, 0x30FB, 0x25CF, 0 };


TTFont::TTFont(Graphics::Font *font, uint32 rgb_, int bordersize_,
               bool antiAliased_, bool SJIS_)
	: ttf_font(font), antiAliased(antiAliased_), SJIS(SJIS_) {
//	rgb = PACK_RGB8( (rgb_>>16)&0xFF , (rgb_>>8)&0xFF , rgb_&0xFF );
	// This should be performed by PACK_RGB8, but it is not initialized at this point.
	rgb = ((rgb_ >> 16) & 0xFF) | (((rgb_ >> 8) & 0xFF) << 8) | ((rgb_ & 0xFF) << 16);
	bordersize = bordersize_;

	bullet = 0;
	// scan for a character to use as a conversation option bullet
	for (int i = 0; BULLETS[i]; ++i) {
		Common::Rect box = font->getBoundingBox(BULLETS[i]);

		if (!box.isEmpty()) {
			bullet = BULLETS[i];
			break;
		}
	}

	if (bullet == 0) {
		bullet = '*';
	}
}

TTFont::~TTFont() {

}

int TTFont::getHeight() {
	return ttf_font->getFontHeight() + 2 * bordersize; // constant (border)
}

int TTFont::getBaseline() {
//	return TTF_FontAscent(ttf_font);
	// TODO: TTF ascent isn't publically accessible
	return 0;
}

int TTFont::getBaselineSkip() {
//	return TTF_FontLineSkip(ttf_font);
	// TODO: What should be returned for this?
	return 0;
}

#ifdef TODO
template<class T>
static uint16 *toUnicode(const std::string &text, uint16 bullet) {
	std::string::size_type l = T::length(text);
	Common::U32String unicodeText = new uint16[l + 1];
	std::string::const_iterator iter = text.begin();
	for (unsigned int i = 0; i < l; ++i) {
		uint32 u = T::unicode(iter);
		if (u > 0xFFFF) {
			perr.Print("Warning: unicode character out of range for SDL_ttf: %x\n", u);
			unicodeText[i] = '?';
		} else if (u == 64) {
			unicodeText[i] = bullet;
		} else {
			unicodeText[i] = u;
		}
	}
	unicodeText[l] = 0;
	return unicodeText;
}
#else
template<class T>
static Common::U32String toUnicode(const std::string &text, uint16 bullet) {
	return Common::U32String(text);
}
#endif

void TTFont::getStringSize(const std::string &text, int &width, int &height) {
	// convert to unicode
	Common::U32String unicodeText;
	if (!SJIS)
		unicodeText = toUnicode<Traits>(text, bullet);
	else
		unicodeText = toUnicode<SJISTraits>(text, bullet);

	width = ttf_font->getStringWidth(unicodeText);
	height = ttf_font->getFontHeight();

	width += 2 * bordersize;
	height += 2 * bordersize;
}

void TTFont::getTextSize(const std::string &text,
                         int &resultwidth, int &resultheight,
                         unsigned int &remaining,
                         int width, int height, TextAlign align,
                         bool u8specials) {
	std::list<PositionedText> tmp;
	if (!SJIS)
		tmp = typesetText<Traits>(this, text, remaining,
		                          width, height, align, u8specials,
		                          resultwidth, resultheight);
	else
		tmp = typesetText<SJISTraits>(this, text, remaining,
		                              width, height, align, u8specials,
		                              resultwidth, resultheight);
}


RenderedText *TTFont::renderText(const std::string &text,
                                 unsigned int &remaining,
                                 int width, int height,
                                 TextAlign align, bool u8specials,
                                 std::string::size_type cursor) {
	int resultwidth, resultheight;
	std::list<PositionedText> lines;
	if (!SJIS)
		lines = typesetText<Traits>(this, text, remaining,
		                            width, height, align, u8specials,
		                            resultwidth, resultheight, cursor);
	else
		lines = typesetText<SJISTraits>(this, text, remaining,
		                                width, height, align, u8specials,
		                                resultwidth, resultheight, cursor);

	// create 32bit RGBA texture buffer
	uint32 *buf = new uint32[resultwidth * resultheight];
	memset(buf, 0, 4 * resultwidth * resultheight);

	Texture *texture = new Texture();
	texture->buffer = buf;
	texture->width = resultwidth;
	texture->height = resultheight;

#if 0
	pout << "Total size: " << resultwidth << "," << resultheight << std::endl;
#endif

	std::list<PositionedText>::iterator iter;
	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		// convert to unicode
		Common::U32String unicodeText;
		if (!SJIS)
			unicodeText = toUnicode<Traits>(iter->text, bullet);
		else
			unicodeText = toUnicode<SJISTraits>(iter->text, bullet);

		// Create a surface and render the text
		Graphics::ManagedSurface textSurf;
		
		if (!antiAliased) {
			// When not in antialiased mode, use a paletted surface where '1' is
			// used for pixels of the text
			textSurf.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
			ttf_font->drawString(&textSurf, unicodeText, 0, 0, width, 1);
		} else {
			// Use a high color surface with the specified rgb color for text
			textSurf.create(width, height,
				Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
			uint32 color = textSurf.format.RGBToColor(TEX32_R(rgb), TEX32_G(rgb), TEX32_B(rgb));

			ttf_font->drawString(&textSurf, unicodeText, 0, 0, width, color);
		};

#if 0
		pout << iter->dims.w << "," << iter->dims.h << " vs. "
			    << textSurf.w << "," << textSurf.h << ": " << iter->text
			    << std::endl;
#endif

		// render the text surface into our texture buffer
		for (int y = 0; y < textSurf.w; y++) {
			byte *surfrow = (byte *)textSurf.getBasePtr(0, y);

			// CHECKME: bordersize!
			uint32 *bufrow = buf + (iter->dims.y + y + bordersize) * resultwidth;
			for (int x = 0; x < textSurf.w; x++) {

				if (!antiAliased && surfrow[x] == 1) {
					bufrow[iter->dims.x + x + bordersize] = rgb | 0xFF000000;
					if (bordersize <= 0) continue;
					if (bordersize == 1) {
						// optimize common case
						for (int dx = -1; dx <= 1; dx++) {
							for (int dy = -1; dy <= 1; dy++) {
								if (x + 1 + iter->dims.x + dx >= 0 &&
									    x + 1 + iter->dims.x + dx < resultwidth &&
									    y + 1 + dy >= 0 && y + 1 + dy < resultheight) {
									if (buf[(y + iter->dims.y + dy + 1)*resultwidth + x + 1 + iter->dims.x + dx] == 0) {
										buf[(y + iter->dims.y + dy + 1)*resultwidth + x + 1 + iter->dims.x + dx] = 0xFF000000;
									}
								}
							}
						}
						continue;
					}
					for (int dx = -bordersize; dx <= bordersize; dx++) {
						for (int dy = -bordersize; dy <= bordersize; dy++) {
							if (x + bordersize + iter->dims.x + dx >= 0 &&
								    x + bordersize + iter->dims.x + dx < resultwidth &&
								    y + bordersize + dy >= 0 && y + bordersize + dy < resultheight) {
								if (buf[(y + iter->dims.y + dy + bordersize)*resultwidth + x + bordersize + iter->dims.x + dx] == 0) {
									buf[(y + iter->dims.y + dy + bordersize)*resultwidth + x + bordersize + iter->dims.x + dx] = 0xFF000000;
								}
							}
						}
					}
				} else if (antiAliased) {
					uint32 pixColor = *((uint32 *)(surfrow + x * 4));
					if (pixColor == 0)
						continue;

					byte pixR, pixG, pixB, pixA;
					textSurf.format.colorToARGB(pixColor, pixA, pixR, pixG, pixB);
					int idx = pixA;

					if (bordersize <= 0) {
						bufrow[iter->dims.x + x + bordersize] = TEX32_PACK_RGBA(pixR, pixG, pixB, pixA);
					} else {
						bufrow[iter->dims.x + x + bordersize] = TEX32_PACK_RGBA(pixR, pixG, pixB, 0xFF);

						// optimize common case
						if (bordersize == 1) for (int dx = -1; dx <= 1; dx++) {
							for (int dy = -1; dy <= 1; dy++) {
								if (x + 1 + iter->dims.x + dx >= 0 &&
										x + 1 + iter->dims.x + dx < resultwidth &&
										y + 1 + dy >= 0 && y + 1 + dy < resultheight) {
									uint32 alpha = TEX32_A(buf[(y + iter->dims.y + dy + 1) * resultwidth + x + 1 + iter->dims.x + dx]);
									if (alpha != 0xFF) {
										alpha = 255 - (((255 - alpha) * (255 - idx)) >> 8);
										buf[(y + iter->dims.y + dy + 1)*resultwidth + x + 1 + iter->dims.x + dx] = alpha << TEX32_A_SHIFT;
									}
								}
							}
						} else {
							for (int dx = -bordersize; dx <= bordersize; dx++) {
								for (int dy = -bordersize; dy <= bordersize; dy++) {
									if (x + bordersize + iter->dims.x + dx >= 0 &&
											x + bordersize + iter->dims.x + dx < resultwidth &&
											y + bordersize + dy >= 0 && y + bordersize + dy < resultheight) {
										uint32 alpha = TEX32_A(buf[(y + iter->dims.y + dy + bordersize) * resultwidth + x + bordersize + iter->dims.x + dx]);
										if (alpha != 0xFF) {
											alpha = 255 - (((255 - alpha) * (255 - idx)) >> 8);
											buf[(y + iter->dims.y + dy + bordersize)*resultwidth + x + bordersize + iter->dims.x + dx] = alpha << TEX32_A_SHIFT;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (iter->cursor != std::string::npos) {
			assert(iter->cursor <= iter->text.size());
			unicodeText = Common::U32String(unicodeText.c_str(), iter->cursor);

			int w = ttf_font->getStringWidth(unicodeText);

			for (int y = 0; y < iter->dims.h; y++) {
				uint32 *bufrow = buf + (iter->dims.y + y) * resultwidth;
				bufrow[iter->dims.x + w + bordersize] = 0xFF000000;
//				if (bordersize > 0)
//					bufrow[iter->dims.x+w+bordersize-1] = 0xFF000000;
			}
		}
	}

	return new TTFRenderedText(texture, resultwidth, resultheight,
	                           getBaselineSkip() - getHeight(), this);
}

} // End of namespace Ultima8
