/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_sprite_font/sprite_font_renderer.h"
#include "ags/plugins/ags_sprite_font/color.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

SpriteFontRenderer::SpriteFontRenderer(IAGSEngine *engine) {
	_engine = engine;
}


SpriteFontRenderer::~SpriteFontRenderer(void) {}

void SpriteFontRenderer::SetSpriteFont(int fontNum, int sprite, int rows, int columns, int charWidth, int charHeight, int charMin, int charMax, bool use32bit) {
	SpriteFont *font = getFontFor(fontNum);
	font->SpriteNumber = sprite;
	font->Rows = rows;
	font->Columns = columns;
	font->MinChar = charMin;
	font->MaxChar = charMax;
	font->Use32bit = use32bit;
	font->CharHeight = charHeight;
	font->CharWidth = charWidth;

}

void SpriteFontRenderer::EnsureTextValidForFont(char *text, int fontNumber) {
	SpriteFont *font = getFontFor(fontNumber);
	for (int i = 0; i < (int)strlen(text); i++) {
		if (text[i] < font->MinChar || text[i] > font->MaxChar) {
			if (font->MinChar < 63 || font->MaxChar > 63) text[i] = 63;
			else text[i] = font->MinChar;

		}

	}
}

bool SpriteFontRenderer::SupportsExtendedCharacters(int fontNumber) {
	return false;
}

int SpriteFontRenderer::GetTextWidth(const char *text, int fontNumber) {
	SpriteFont *font = getFontFor(fontNumber);
	int len = (int)strlen(text);
	return font->CharWidth * len;

}

int SpriteFontRenderer::GetTextHeight(const char *text, int fontNumber) {
	SpriteFont *font = getFontFor(fontNumber);
	return font->CharHeight;
}

SpriteFont *SpriteFontRenderer::getFontFor(int fontNum) {
	SpriteFont *font;
	for (int i = 0; i < (int)_fonts.size(); i ++) {
		font = _fonts.at(i);
		if (font->FontReplaced == fontNum) return font;
	}
	//not found
	font = new SpriteFont();
	font->FontReplaced = fontNum;
	_fonts.push_back(font);
	return font;
}



void SpriteFontRenderer::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) {
	SpriteFont *font = getFontFor(fontNumber);
	//BITMAP *vScreen = _engine->GetVirtualScreen();

	//_engine->SetVirtualScreen(destination);

	for (int i = 0; i < (int)strlen(text); i++) {
		char c = text[i];
		c -= font->MinChar;
		int row = c / font->Columns;
		int column = c % font->Columns;
		BITMAP *src = _engine->GetSpriteGraphic(font->SpriteNumber);
		Draw(src, destination, x + (i * font->CharWidth), y, column * font->CharWidth, row * font->CharHeight, font->CharWidth, font->CharHeight);
	}

	//_engine->SetVirtualScreen(vScreen);
}




void SpriteFontRenderer::Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height) {

	int32 srcWidth, srcHeight, destWidth, destHeight, srcColDepth, destColDepth;

	unsigned char **srccharbuffer = _engine->GetRawBitmapSurface(src);  //8bit
	unsigned short **srcshortbuffer = (unsigned short **)srccharbuffer; //16bit;
	unsigned int **srclongbuffer = (unsigned int **)srccharbuffer; //32bit

	unsigned char **destcharbuffer = _engine->GetRawBitmapSurface(dest);  //8bit
	unsigned short **destshortbuffer = (unsigned short **)destcharbuffer; //16bit;
	unsigned int **destlongbuffer = (unsigned int **)destcharbuffer; //32bit

	int transColor = _engine->GetBitmapTransparentColor(src);

	_engine->GetBitmapDimensions(src, &srcWidth, &srcHeight, &srcColDepth);
	_engine->GetBitmapDimensions(dest, &destWidth, &destHeight, &destColDepth);

	if (srcy + height > srcHeight || srcx + width > srcWidth || srcx < 0 || srcy < 0) return;

	if (width + destx > destWidth) width = destWidth - destx;
	if (height + desty > destHeight) height = destHeight - desty;

	int startx = MAX(0, (-1 * destx));
	int starty = MAX(0, (-1 * desty));


	int srca, srcr, srcg, srcb, desta, destr, destg, destb, finalr, finalg, finalb, finala, col;

	for (int x = startx; x < width; x ++) {

		for (int y = starty; y <  height; y ++) {
			int srcyy = y + srcy;
			int srcxx = x + srcx;
			int destyy = y + desty;
			int destxx = x + destx;
			if (destColDepth == 8) {
				if (srccharbuffer[srcyy][srcxx] != transColor) destcharbuffer[destyy][destxx] = srccharbuffer[srcyy][srcxx];
			} else if (destColDepth == 16) {
				if (srcshortbuffer[srcyy][srcxx] != transColor) destshortbuffer[destyy][destxx] = srcshortbuffer[srcyy][srcxx];
			} else if (destColDepth == 32) {
				//if (srclongbuffer[srcyy][srcxx] != transColor) destlongbuffer[destyy][destxx] = srclongbuffer[srcyy][srcxx];

				srca = (geta32(srclongbuffer[srcyy][srcxx]));

				if (srca != 0) {

					srcr =  getr32(srclongbuffer[srcyy][srcxx]);
					srcg =  getg32(srclongbuffer[srcyy][srcxx]);
					srcb =  getb32(srclongbuffer[srcyy][srcxx]);

					destr =  getr32(destlongbuffer[destyy][destxx]);
					destg =  getg32(destlongbuffer[destyy][destxx]);
					destb =  getb32(destlongbuffer[destyy][destxx]);
					desta =  geta32(destlongbuffer[destyy][destxx]);


					finalr = srcr;
					finalg = srcg;
					finalb = srcb;


					finala = 255 - (255 - srca) * (255 - desta) / 255;
					finalr = srca * finalr / finala + desta * destr * (255 - srca) / finala / 255;
					finalg = srca * finalg / finala + desta * destg * (255 - srca) / finala / 255;
					finalb = srca * finalb / finala + desta * destb * (255 - srca) / finala / 255;
					col = makeacol32(finalr, finalg, finalb, finala);
					destlongbuffer[destyy][destxx] = col;
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
