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

#ifndef AGS_PLUGINS_AGS_SPRITE_FONT_SPR_FONT_RENDERER_H
#define AGS_PLUGINS_AGS_SPRITE_FONT_SPR_FONT_RENDERER_H

#include "ags/plugins/ags_sprite_font/sprite_font.h"
#include "ags/plugins/agsplugin.h"
#include "ags/lib/std/vector.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

class SpriteFontRenderer : public IAGSFontRenderer {
public:
	SpriteFontRenderer(IAGSEngine *engine);
	virtual ~SpriteFontRenderer(void);
	bool LoadFromDisk(int fontNumber, int fontSize) override {
		return true;
	}
	void FreeMemory(int fontNumber) override { }
	bool SupportsExtendedCharacters(int fontNumber) override;
	int GetTextWidth(const char *text, int fontNumber) override;
	int GetTextHeight(const char *text, int fontNumber) override;
	void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
	void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override { }
	void EnsureTextValidForFont(char *text, int fontNumber) override;
	void SetSpriteFont(int fontNum, int sprite, int rows, int columns, int charWidth, int charHeight, int charMin, int charMax, bool use32bit);

private:
	SpriteFont *getFontFor(int fontNum);
	void Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height);
	std::vector<SpriteFont * > _fonts;
	IAGSEngine *_engine;
};

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3

#endif
