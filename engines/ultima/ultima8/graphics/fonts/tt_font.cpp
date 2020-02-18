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

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TTFont, Font)

// various unicode characters which look like small black circles
static const uint16 BULLETS[] = { 0x2022, 0x30FB, 0x25CF, 0 };


TTFont::TTFont(Graphics::Font *font, uint32 rgb_, int bordersize_,
               bool antiAliased_, bool SJIS_)
	: _ttfFont(font), _antiAliased(antiAliased_), _SJIS(SJIS_) {
//	_rgb = PACK_RGB8( (rgb_>>16)&0xFF , (rgb_>>8)&0xFF , rgb_&0xFF );
	// This should be performed by PACK_RGB8, but it is not initialized at this point.
	_rgb = ((rgb_ >> 16) & 0xFF) | (((rgb_ >> 8) & 0xFF) << 8) | ((rgb_ & 0xFF) << 16);
	_borderSize = bordersize_;

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
	return _ttfFont->getFontHeight() + 2 * _borderSize; // constant (border)
}

int TTFont::getBaseline() {
//	return TTF_FontAscent(_ttfFont);
	// TODO: TTF ascent isn't publically accessible
	return 0;
}

int TTFont::getBaselineSkip() {
//	return TTF_FontLineSkip(_ttfFont);
	// TODO: What should be returned for this?
	return 0;
}

#ifdef TODO
template<class T>
static uint16 *toUnicode(const Std::string &text, uint16 bullet) {
	Std::string::size_type l = T::length(text);
	Common::U32String unicodeText = new uint16[l + 1];
	Std::string::const_iterator iter = text.begin();
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
static Common::U32String toUnicode(const Std::string &text, uint16 bullet) {
	return Common::U32String(text);
}
#endif

void TTFont::getStringSize(const Std::string &text, int32 &width, int32 &height) {
	// convert to unicode
	Common::U32String unicodeText;
	if (!_SJIS)
		unicodeText = toUnicode<Traits>(text, bullet);
	else
		unicodeText = toUnicode<SJISTraits>(text, bullet);

	width = _ttfFont->getStringWidth(unicodeText);
	height = _ttfFont->getFontHeight();

	width += 2 * _borderSize;
	height += 2 * _borderSize;
}

void TTFont::getTextSize(const Std::string &text,
                         int32 &resultwidth, int32 &resultheight,
                         unsigned int &remaining,
                         int32 width, int32 height, TextAlign align,
                         bool u8specials) {
	Std::list<PositionedText> tmp;
	if (!_SJIS)
		tmp = typesetText<Traits>(this, text, remaining,
		                          width, height, align, u8specials,
		                          resultwidth, resultheight);
	else
		tmp = typesetText<SJISTraits>(this, text, remaining,
		                              width, height, align, u8specials,
		                              resultwidth, resultheight);
}


RenderedText *TTFont::renderText(const Std::string &text,
                                 unsigned int &remaining,
                                 int32 width, int32 height,
                                 TextAlign align, bool u8specials,
                                 Std::string::size_type cursor) {
	int32 resultwidth, resultheight;
	Std::list<PositionedText> lines;
	if (!_SJIS)
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
	texture->_buffer = buf;
	texture->_width = resultwidth;
	texture->_height = resultheight;

#if 0
	pout << "Total size: " << resultwidth << "," << resultheight << Std::endl;
#endif

	Std::list<PositionedText>::iterator iter;
	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		// convert to unicode
		Common::U32String unicodeText;
		if (!_SJIS)
			unicodeText = toUnicode<Traits>(iter->_text, bullet);
		else
			unicodeText = toUnicode<SJISTraits>(iter->_text, bullet);

		// Create a surface and render the text
		Graphics::ManagedSurface textSurf;
		
		if (!_antiAliased) {
			// When not in antialiased mode, use a paletted surface where '1' is
			// used for pixels of the text
			textSurf.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
			_ttfFont->drawString(&textSurf, unicodeText, 0, 0, width, 1);
		} else {
			// Use a high color surface with the specified _rgb color for text
			textSurf.create(width, height,
				Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
			uint32 color = textSurf.format.RGBToColor(TEX32_R(_rgb), TEX32_G(_rgb), TEX32_B(_rgb));

			_ttfFont->drawString(&textSurf, unicodeText, 0, 0, width, color);
		};

#if 0
		pout << iter->_dims.w << "," << iter->_dims.h << " vs. "
			    << textSurf.w << "," << textSurf.h << ": " << iter->_text
			    << Std::endl;
#endif

		// render the text surface into our texture buffer
		for (int y = 0; y < textSurf.h; y++) {
			byte *surfrow = (byte *)textSurf.getBasePtr(0, y);

			// CHECKME: _borderSize!
			uint32 *bufrow = buf + (iter->_dims.y + y + _borderSize) * resultwidth;
			for (int x = 0; x < textSurf.w; x++) {

				if (!_antiAliased && surfrow[x] == 1) {
					bufrow[iter->_dims.x + x + _borderSize] = _rgb | 0xFF000000;
					if (_borderSize <= 0) continue;
					if (_borderSize == 1) {
						// optimize common case
						for (int dx = -1; dx <= 1; dx++) {
							for (int dy = -1; dy <= 1; dy++) {
								if (x + 1 + iter->_dims.x + dx >= 0 &&
									    x + 1 + iter->_dims.x + dx < resultwidth &&
									    y + 1 + dy >= 0 && y + 1 + dy < resultheight) {
									if (buf[(y + iter->_dims.y + dy + 1)*resultwidth + x + 1 + iter->_dims.x + dx] == 0) {
										buf[(y + iter->_dims.y + dy + 1)*resultwidth + x + 1 + iter->_dims.x + dx] = 0xFF000000;
									}
								}
							}
						}
						continue;
					}
					for (int dx = -_borderSize; dx <= _borderSize; dx++) {
						for (int dy = -_borderSize; dy <= _borderSize; dy++) {
							if (x + _borderSize + iter->_dims.x + dx >= 0 &&
								    x + _borderSize + iter->_dims.x + dx < resultwidth &&
								    y + _borderSize + dy >= 0 && y + _borderSize + dy < resultheight) {
								if (buf[(y + iter->_dims.y + dy + _borderSize)*resultwidth + x + _borderSize + iter->_dims.x + dx] == 0) {
									buf[(y + iter->_dims.y + dy + _borderSize)*resultwidth + x + _borderSize + iter->_dims.x + dx] = 0xFF000000;
								}
							}
						}
					}
				} else if (_antiAliased) {
					uint32 pixColor = *((uint32 *)(surfrow + x * 4));
					if (pixColor == 0)
						continue;

					byte pixR, pixG, pixB, pixA;
					textSurf.format.colorToARGB(pixColor, pixA, pixR, pixG, pixB);
					int idx = pixA;

					if (_borderSize <= 0) {
						bufrow[iter->_dims.x + x + _borderSize] = TEX32_PACK_RGBA(pixR, pixG, pixB, pixA);
					} else {
						bufrow[iter->_dims.x + x + _borderSize] = TEX32_PACK_RGBA(pixR, pixG, pixB, 0xFF);

						// optimize common case
						if (_borderSize == 1) for (int dx = -1; dx <= 1; dx++) {
							for (int dy = -1; dy <= 1; dy++) {
								if (x + 1 + iter->_dims.x + dx >= 0 &&
										x + 1 + iter->_dims.x + dx < resultwidth &&
										y + 1 + dy >= 0 && y + 1 + dy < resultheight) {
									uint32 alpha = TEX32_A(buf[(y + iter->_dims.y + dy + 1) * resultwidth + x + 1 + iter->_dims.x + dx]);
									if (alpha != 0xFF) {
										alpha = 255 - (((255 - alpha) * (255 - idx)) >> 8);
										buf[(y + iter->_dims.y + dy + 1)*resultwidth + x + 1 + iter->_dims.x + dx] = alpha << TEX32_A_SHIFT;
									}
								}
							}
						} else {
							for (int dx = -_borderSize; dx <= _borderSize; dx++) {
								for (int dy = -_borderSize; dy <= _borderSize; dy++) {
									if (x + _borderSize + iter->_dims.x + dx >= 0 &&
											x + _borderSize + iter->_dims.x + dx < resultwidth &&
											y + _borderSize + dy >= 0 && y + _borderSize + dy < resultheight) {
										uint32 alpha = TEX32_A(buf[(y + iter->_dims.y + dy + _borderSize) * resultwidth + x + _borderSize + iter->_dims.x + dx]);
										if (alpha != 0xFF) {
											alpha = 255 - (((255 - alpha) * (255 - idx)) >> 8);
											buf[(y + iter->_dims.y + dy + _borderSize)*resultwidth + x + _borderSize + iter->_dims.x + dx] = alpha << TEX32_A_SHIFT;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (iter->_cursor != Std::string::npos) {
			assert(iter->_cursor <= iter->_text.size());
			unicodeText = Common::U32String(unicodeText.c_str(), iter->_cursor);

			int w = _ttfFont->getStringWidth(unicodeText);

			for (int y = 0; y < iter->_dims.h; y++) {
				uint32 *bufrow = buf + (iter->_dims.y + y) * resultwidth;
				bufrow[iter->_dims.x + w + _borderSize] = 0xFF000000;
//				if (_borderSize > 0)
//					bufrow[iter->_dims.x+w+_borderSize-1] = 0xFF000000;
			}
		}
	}

	return new TTFRenderedText(texture, resultwidth, resultheight,
	                           getBaselineSkip() - getHeight(), this);
}

} // End of namespace Ultima8
} // End of namespace Ultima
