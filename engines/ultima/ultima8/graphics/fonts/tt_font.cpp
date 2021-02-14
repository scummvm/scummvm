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
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/graphics/fonts/ttf_rendered_text.h"
#include "ultima/ultima8/graphics/texture.h"


//include iomanip

namespace Ultima {
namespace Ultima8 {

// various unicode characters which look like small black circles
static const uint16 BULLETS[] = { 0x2022, 0x30FB, 0x25CF, 0 };

static const Graphics::PixelFormat PF_RGBA(4, 8, 8, 8, 8, 24, 16, 8, 0);

TTFont::TTFont(Graphics::Font *font, uint32 rgb, int borderSize,
		bool antiAliased, bool SJIS) :
		_borderSize(borderSize), _ttfFont(font), _antiAliased(antiAliased), _SJIS(SJIS) {
	_color = PF_RGBA.RGBToColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xff, rgb & 0xff);

	_bullet = 0;
	// scan for a character to use as a conversation option _bullet
	for (int i = 0; BULLETS[i]; ++i) {
		Common::Rect box = font->getBoundingBox(BULLETS[i]);

		if (!box.isEmpty()) {
			_bullet = BULLETS[i];
			break;
		}
	}

	if (_bullet == 0) {
		_bullet = '*';
	}
}

TTFont::~TTFont() {

}

int TTFont::getHeight() {
	return _ttfFont->getFontHeight() + 2 * _borderSize; // constant (border)
}

int TTFont::getBaseline() {
	Common::Rect box = _ttfFont->getBoundingBox('W');
	return box.bottom;
}

int TTFont::getBaselineSkip() {
	// TODO: Come up with something more generic than just hardcoding 2 pixel line separation
	return getHeight() + 2;
}

template<class T>
static Common::U32String toUnicode(const Std::string &text, uint16 bullet) {
	Std::string::size_type len = T::length(text);
	Common::U32String result = Common::U32String(text.c_str(), len);

	for (uint idx = 0; idx < result.size(); ++idx) {
		if (result[idx] == '@')
			result.setChar(bullet, idx);
	}

	return result;
}

void TTFont::getStringSize(const Std::string &text, int32 &width, int32 &height) {
	// convert to unicode
	Common::U32String unicodeText;
	if (!_SJIS)
		unicodeText = toUnicode<Traits>(text, _bullet);
	else
		unicodeText = toUnicode<SJISTraits>(text, _bullet);

	width = _ttfFont->getStringWidth(unicodeText);
	height = _ttfFont->getFontHeight();

	width += 2 * _borderSize;
	height += 2 * _borderSize;
}

void TTFont::getTextSize(const Std::string &text,
                         int32 &resultWidth, int32 &resultHeight,
                         unsigned int &remaining,
                         int32 width, int32 height, TextAlign align,
                         bool u8specials) {
	Std::list<PositionedText> tmp;
	if (!_SJIS)
		tmp = typesetText<Traits>(this, text, remaining,
		                          width, height, align, u8specials,
		                          resultWidth, resultHeight);
	else
		tmp = typesetText<SJISTraits>(this, text, remaining,
		                              width, height, align, u8specials,
		                              resultWidth, resultHeight);
}


RenderedText *TTFont::renderText(const Std::string &text, unsigned int &remaining,
		int32 width, int32 height, TextAlign align, bool u8specials,
		Std::string::size_type cursor) {
	int32 resultWidth, resultHeight, lineHeight;
	Std::list<PositionedText> lines;
	if (!_SJIS)
		lines = typesetText<Traits>(this, text, remaining, width, height, align, u8specials,
			resultWidth, resultHeight, cursor);
	else
		lines = typesetText<SJISTraits>(this, text, remaining, width, height, align, u8specials,
			resultWidth, resultHeight, cursor);
	lineHeight = _ttfFont->getFontHeight();

	uint32 borderColor = PF_RGBA.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	Graphics::ManagedSurface *texture = new Graphics::ManagedSurface(resultWidth, resultHeight, PF_RGBA);
	uint32 *texBuf = (uint32 *)texture->getPixels();

	Std::list<PositionedText>::const_iterator iter;
	for (iter = lines.begin(); iter != lines.end(); ++iter) {
		// convert to unicode
		Common::U32String unicodeText;
		if (!_SJIS)
			unicodeText = toUnicode<Traits>(iter->_text, _bullet);
		else
			unicodeText = toUnicode<SJISTraits>(iter->_text, _bullet);

		// Create a surface and render the text
		Graphics::ManagedSurface textSurf;

		if (!_antiAliased) {
			// When not in antialiased mode, use a paletted surface where '1' is
			// used for pixels of the text
			textSurf.create(resultWidth, lineHeight, Graphics::PixelFormat::createFormatCLUT8());
			_ttfFont->drawString(&textSurf, unicodeText, 0, 0, resultWidth, 1);
		} else {
			// Use a high color surface with the specified _color color for text
			textSurf.create(resultWidth, lineHeight, PF_RGBA);
			_ttfFont->drawString(&textSurf, unicodeText, 0, 0, resultWidth, _color);
		};

		// render the text surface into our texture buffer
		for (int y = 0; y < textSurf.h; y++) {
			const byte *surfrow = (const byte *)textSurf.getBasePtr(0, y);

			// CHECKME: _borderSize!
			int ty = iter->_dims.top + y + _borderSize;
			for (int x = 0; x < textSurf.w; x++) {
				int tx = iter->_dims.left + x + _borderSize;
				if (!_antiAliased && surfrow[x] == 1) {
					texBuf[ty * resultWidth + tx] = _color;
					
					if (_borderSize <= 0) continue;

					for (int dx = -_borderSize; dx <= _borderSize; dx++) {
						for (int dy = -_borderSize; dy <= _borderSize; dy++) {
							int bx = iter->_dims.left + x + _borderSize + dx;
							int by = iter->_dims.top + y + _borderSize + dy;
							if (bx >= 0 && bx < resultWidth && by >= 0 && by < resultHeight) {
								if (texBuf[by * resultWidth + bx] == 0) {
									// Add border within radius.
									int sqrSize = (_borderSize + 1) * (_borderSize + 1);
									int sqrDist = (dx * dx) + (dy * dy);
									if (sqrDist < sqrSize) {
										texBuf[by * resultWidth + bx] = borderColor;
									}
								}
							}
						}
					}
				} else if (_antiAliased) {
					uint32 sColor = *((const uint32 *)(surfrow + x * 4));
					uint8 sR, sG, sB, sA;
					PF_RGBA.colorToARGB(sColor, sA, sR, sG, sB);

					if (sA == 0x00)
						continue;

					if (_borderSize <= 0) {
						texBuf[ty * resultWidth + tx] = sColor;
					} else {
						uint8 dA, dR, dG, dB;
						double alpha = (double)sA / 255.0;

						switch (sA) {
						case 0xFF:
							texBuf[ty * resultWidth + tx] = sColor;
							break;
						default:
							// Blend color with border color
							PF_RGBA.colorToRGB(borderColor, dR, dG, dB);

							dR = static_cast<uint8>((sR * alpha) + (dR * (1.0 - alpha)));
							dG = static_cast<uint8>((sG * alpha) + (dG * (1.0 - alpha)));
							dB = static_cast<uint8>((sB * alpha) + (dB * (1.0 - alpha)));
							
							texBuf[ty * resultWidth + tx] = PF_RGBA.RGBToColor(dR, dG, dB);
							break;
						}

						// Add border around pixel if alpha is greater than threshold  
						if (sA > 0x04) {
							for (int dx = -_borderSize; dx <= _borderSize; dx++) {
								for (int dy = -_borderSize; dy <= _borderSize; dy++) {
									int bx = iter->_dims.left + x + _borderSize + dx;
									int by = iter->_dims.top + y + _borderSize + dy;
									if (bx >= 0 && bx < resultWidth && by >= 0 && by < resultHeight) {
										int32 dColor = texBuf[by * resultWidth + bx];
										PF_RGBA.colorToARGB(dColor, dA, dR, dG, dB);
										if (dA < 255) {
											// Add border within radius. Pixels on the line are alpha blended 
											int sqrSize = (_borderSize + 1) * (_borderSize + 1);
											int sqrDist = (dx * dx) + (dy * dy);
											if (sqrDist < sqrSize) {
												sqrSize = _borderSize * _borderSize;
												uint8 bA = sqrDist < sqrSize ? 255 : static_cast<uint8>((255 * sqrSize) / sqrDist);

												if (bA > dA) {
													PF_RGBA.colorToRGB(borderColor, dR, dG, dB);
													texBuf[by * resultWidth + bx] = PF_RGBA.ARGBToColor(bA, dR, dG, dB);
												}
											}
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
			unicodeText = unicodeText.substr(0, iter->_cursor);

			int w = _ttfFont->getStringWidth(unicodeText);

			for (int y = 0; y < iter->_dims.height(); y++) {
				int tx = iter->_dims.left + w + _borderSize;
				int ty = iter->_dims.top + y;
				texBuf[ty * resultWidth + tx] = borderColor;
			}
		}
	}

	return new TTFRenderedText(texture, resultWidth, resultHeight,
		getBaselineSkip() - getHeight(), getBaseline(), isAntialiased());
}

} // End of namespace Ultima8
} // End of namespace Ultima
