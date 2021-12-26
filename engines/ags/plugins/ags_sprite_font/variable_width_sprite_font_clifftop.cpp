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

#include "ags/plugins/ags_sprite_font/variable_width_sprite_font_clifftop.h"
#include "ags/plugins/ags_sprite_font/color.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

VariableWidthSpriteFontRendererClifftop::VariableWidthSpriteFontRendererClifftop(IAGSEngine *engine) : VariableWidthSpriteFontRenderer(engine) {
}

VariableWidthSpriteFontRendererClifftop::~VariableWidthSpriteFontRendererClifftop(void) {
}

int VariableWidthSpriteFontRendererClifftop::GetTextHeight(const char *text, int fontNumber) {
	VariableWidthFont *font = getFontFor(fontNumber);
	if (strcmp("<LINE_SPACING>", text) == 0)
		return font->LineSpacingOverride;

	for (int i = 0; i < (int)strlen(text); i++) {
		if (font->characters.count(text[i]) > 0) {
			int height = font->characters[text[i]].Height;

			if (strcmp("ZHwypgfjqhkilIK", text) == 0 || strcmp("ZhypjIHQFb", text) == 0 || strcmp("YpyjIHgMNWQ", text) == 0 || strcmp("BigyjTEXT", text) == 0)
				height += font->LineSpacingAdjust;
			else
				height += font->LineHeightAdjust;

			return height;
		}
	}
	return 0;
}

void VariableWidthSpriteFontRendererClifftop::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) {
	VariableWidthFont *font = getFontFor(fontNumber);
	int totalWidth = 0;
	for (int i = 0; i < (int)strlen(text); i++) {
		char c = text[i];

		BITMAP *src = _engine->GetSpriteGraphic(font->SpriteNumber);
		Draw(src, destination, x + totalWidth, y, font->characters[c].X, font->characters[c].Y, font->characters[c].Width, font->characters[c].Height, colour);
		totalWidth += font->characters[c].Width;
		if (text[i] != ' ') totalWidth += font->Spacing;
	}
}

void VariableWidthSpriteFontRendererClifftop::Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height, int colour) {

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


	int srca, srcr, srcg, srcb, desta, destr, destg, destb, finalr, finalg, finalb, finala, col, col_r, col_g, col_b;
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
