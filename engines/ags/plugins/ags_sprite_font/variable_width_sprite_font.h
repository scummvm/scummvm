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

#ifndef AGS_PLUGINS_AGS_SPRITE_FONT_VAR_WIDTH_SPR_FONT_H
#define AGS_PLUGINS_AGS_SPRITE_FONT_VAR_WIDTH_SPR_FONT_H

#include "ags/plugins/plugin_base.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/ags_sprite_font/variable_width_font.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

class VariableWidthSpriteFontRenderer : public IAGSFontRenderer2 {
protected:
	IAGSEngine *_engine;
	std::vector<VariableWidthFont *> _fonts;

	VariableWidthFont *getFontFor(int fontNum);
	void Draw(BITMAP *src, BITMAP *dest, int destx, int desty, int srcx, int srcy, int width, int height, int colour);
public:
	VariableWidthSpriteFontRenderer(IAGSEngine *engine);
	virtual ~VariableWidthSpriteFontRenderer();

	void SetGlyph(int fontNum, int charNum, int x, int y, int width, int height);
	void SetSprite(int fontNum, int spriteNum);
	void SetSpacing(int fontNum, int spacing);
	// Clifftop Games custom plugin support
	void SetLineHeightAdjust(int fontNum, int LineHeight, int SpacingHeight, int SpacingOverride);

	// IAGSFontRenderer implementation
	bool LoadFromDisk(int fontNumber, int fontSize) override {
		return true;
	}

	void FreeMemory(int fontNumber) override;
	bool SupportsExtendedCharacters(int fontNumber) override;
	int GetTextWidth(const char *text, int fontNumber) override;
	int GetTextHeight(const char *text, int fontNumber) override;
	void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
	void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override { }
	void EnsureTextValidForFont(char *text, int fontNumber) override;

	// IAGSFontRenderer2 implementation
	int GetVersion() override { return 26; /* compatible engine API ver */ }
	const char *GetRendererName() override { return "VariableWidthSpriteFontRenderer"; }
	const char *GetFontName(int fontNumber) override { return ""; /* not supported */ }
	int GetFontHeight(int fontNumber) override;
	int GetLineSpacing(int fontNumber) override;
};

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3

#endif
