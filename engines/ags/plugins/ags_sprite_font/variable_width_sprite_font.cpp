/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_sprite_font/variable_width_sprite_font.h"
#include "ags/plugins/ags_sprite_font/color.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

VariableWidthSpriteFontRenderer::VariableWidthSpriteFontRenderer(IAGSEngine *engine) {
	_engine = engine;
}

VariableWidthSpriteFontRenderer::~VariableWidthSpriteFontRenderer(void) {
}

void VariableWidthSpriteFontRenderer::FreeMemory(int fontNum) {
	for(auto it = _fonts.begin(); it != _fonts.end() ; ++it) {
		VariableWidthFont *font = *it;
		if (font->FontReplaced == fontNum) {
			_fonts.erase(it);
			delete font;
			return;
		}
	}
}

bool VariableWidthSpriteFontRenderer::SupportsExtendedCharacters(int fontNumber) {
	return false;
}

int VariableWidthSpriteFontRenderer::GetTextWidth(const char *text, int fontNumber) {
	int total = 0;
	VariableWidthFont *font = getFontFor(fontNumber);
	int len_text = (int)strlen(text);
	for (int i = 0; i < len_text; i++) {
		if (font->characters.count(text[i]) > 0) {
			total += font->characters[text[i]].Width;
			if (text[i] != ' ') total += font->Spacing;
		}
	}
	return total;
}

int VariableWidthSpriteFontRenderer::GetTextHeight(const char *text, int fontNumber) {
	VariableWidthFont *font = getFontFor(fontNumber);
	int len_text = (int)strlen(text);
	for (int i = 0; i < len_text; i++) {
		if (font->characters.count(text[i]) > 0) {
			return font->characters[text[i]].Height;
		}
	}
	return 0;
}

int VariableWidthSpriteFontRenderer::GetFontHeight(int fontNumber) {
	VariableWidthFont *font = getFontFor(fontNumber);
	if (font->characters.size() > 0) {
		return font->characters.begin()->_value.Height + font->LineSpacingAdjust;
	}
	return 0;
 }

int VariableWidthSpriteFontRenderer::GetLineSpacing(int fontNumber) {
	// CHECKME: it's not clear whether LineSpacingOverride was ever meant as an
	// actual, normal line spacing. In Clifftop's custom engine this value has
	// been used specifically to tell the spacing for *empty lines* when
	// printing a wrapped text on a GUI Label. Official engine does not have
	// such functionality.
	return 0; // use default (font height)
}

void VariableWidthSpriteFontRenderer::SetSpacing(int fontNum, int spacing) {
	VariableWidthFont *font = getFontFor(fontNum);
	font->Spacing = spacing;
}

void VariableWidthSpriteFontRenderer::SetLineHeightAdjust(int fontNum, int lineHeight, int spacingHeight, int spacingOverride) {
	VariableWidthFont *font = getFontFor(fontNum);
	font->LineHeightAdjust = lineHeight;
	font->LineSpacingAdjust = spacingHeight;
	font->LineSpacingOverride = spacingOverride;

	char buf[1024];
	snprintf(buf, sizeof(buf),
		"VariableWidth::SetLineHeightAdjust: font %d, lineHeight %d, spacingHeight %d, spacingOverride %d",
		fontNum, lineHeight, spacingHeight, spacingOverride);
	_engine->PrintDebugConsole(buf);

	if (_engine->version >= 26)
		_engine->NotifyFontUpdated(fontNum);
}

void VariableWidthSpriteFontRenderer::EnsureTextValidForFont(char *text, int fontNumber) {
	VariableWidthFont *font = getFontFor(fontNumber);
	Common::String s(text);
	size_t ln = s.size();

	for (int i = (int)s.size() - 1; i >= 0 ; i--) {
		if (font->characters.count(s[i]) == 0) {
			s.erase(i, 1);
		}
	}
	// We never grow the text
	Common::strcpy_s(text, ln + 1, s.c_str());

}

void VariableWidthSpriteFontRenderer::SetGlyph(int fontNum, int charNum, int x, int y, int width, int height) {
	VariableWidthFont *font = getFontFor(fontNum);
	font->SetGlyph(charNum, x, y, width, height);

	// Only notify engine at the first engine glyph,
	// that should be enough for calculating font height metrics,
	// and will reduce work load (sadly there's no Begin/EndUpdate functions).
	if ((_engine->version >= 26) && (font->characters.size() == 1))
		_engine->NotifyFontUpdated(fontNum);
}


void VariableWidthSpriteFontRenderer::SetSprite(int fontNum, int spriteNum) {
	VariableWidthFont *font = getFontFor(fontNum);
	font->SpriteNumber = spriteNum;
}

VariableWidthFont *VariableWidthSpriteFontRenderer::getFontFor(int fontNum) {
	VariableWidthFont *font;
	for (int i = 0; i < (int)_fonts.size(); i ++) {
		font = _fonts.at(i);
		if (font->FontReplaced == fontNum) return font;
	}
	//not found
	font = new VariableWidthFont;
	font->FontReplaced = fontNum;
	_fonts.push_back(font);
	return font;
}

void VariableWidthSpriteFontRenderer::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) {
	VariableWidthFont *font = getFontFor(fontNumber);
	int totalWidth = 0;
	int len_text = (int)strlen(text);
	for (int i = 0; i < len_text; i++) {
		char c = text[i];

		BITMAP *src = _engine->GetSpriteGraphic(font->SpriteNumber);
		Draw(src, destination, x + totalWidth, y, font->characters[c].X, font->characters[c].Y, font->characters[c].Width, font->characters[c].Height, colour);
		totalWidth += font->characters[c].Width;
		if (text[i] != ' ') totalWidth += font->Spacing;
	}
}


void VariableWidthSpriteFontRenderer::Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height, int colour) {

	int32 srcWidth, srcHeight, destWidth, destHeight, srcColDepth, destColDepth;

	uint8 *srccharbuffer = _engine->GetRawBitmapSurface(src);
	uint8 *destcharbuffer = _engine->GetRawBitmapSurface(dest);

	uint32 transColor = _engine->GetBitmapTransparentColor(src);
	int srcPitch = _engine->GetBitmapPitch(src);
	int destPitch = _engine->GetBitmapPitch(dest);

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, &srcColDepth);
	_engine->GetBitmapDimensions(dest, &destWidth, &destHeight, &destColDepth);

	int bpp = destColDepth / 8;

	if (srcy + height > srcHeight || srcx + width > srcWidth || srcx < 0 || srcy < 0) return;

	if (width + destx > destWidth) width = destWidth - destx;
	if (height + desty > destHeight) height = destHeight - desty;

	int startx = MAX(0, (-1 * destx));
	int starty = MAX(0, (-1 * desty));


	int srca, srcr, srcg, srcb, desta, destr, destg, destb, finalr, finalg, finalb, finala, col, col_r, col_g, col_b;;
	col_r = getr32(colour);
	col_g = getg32(colour);
	col_b = getb32(colour);

	int srcxx = (startx + srcx) * bpp;
	int destxx = (startx + destx) * bpp;
	for (int x = startx; x < width; ++x, srcxx += bpp, destxx += bpp) {

		int srcyy = (starty + srcy) * srcPitch;
		int destyy = (starty + desty) * destPitch;
		for (int y = starty; y <  height; ++y, srcyy += srcPitch, destyy += destPitch) {
			uint8 *srcCol = srccharbuffer + srcyy + srcxx;
			uint8 *destCol = destcharbuffer + destyy + destxx;
			if (destColDepth == 8) {
				if (*srcCol != transColor)
					*destCol = *srcCol;
			} else if (destColDepth == 16) {
				if (*((uint16 *)srcCol) != transColor)
					*((uint16 *)destCol) = *((uint16 *)srcCol);
			} else if (destColDepth == 32) {
				//if (*((uint32*)srcCol) != transColor)
				//  *((uint32*)destCol) = *((uint32*)srcCol);

				uint32 srcargb = *((uint32 *)srcCol);
				uint32 &destargb = *((uint32 *)destCol);

				srca = (geta32(srcargb));

				if (srca != 0) {

					srcr =  getr32(srcargb);
					srcg =  getg32(srcargb);
					srcb =  getb32(srcargb);

					destr =  getr32(destargb);
					destg =  getg32(destargb);
					destb =  getb32(destargb);
					desta =  geta32(destargb);

					finalr = (col_r * srcr) / 255;
					finalg = (col_g * srcg) / 255;
					finalb = (col_b * srcb) / 255;

					finala = 255 - (255 - srca) * (255 - desta) / 255;
					finalr = srca * finalr / finala + desta * destr * (255 - srca) / finala / 255;
					finalg = srca * finalg / finala + desta * destg * (255 - srca) / finala / 255;
					finalb = srca * finalb / finala + desta * destb * (255 - srca) / finala / 255;
					col = makeacol32(finalr, finalg, finalb, finala);
					destargb = col;
				}

			}
		}
	}

	_engine->ReleaseBitmapSurface(src);
	_engine->ReleaseBitmapSurface(dest);
}

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3
