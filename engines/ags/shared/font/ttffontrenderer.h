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

#ifndef AGS_SHARED_FONT_TTFFONTRENDERER_H
#define AGS_SHARED_FONT_TTFFONTRENDERER_H

#include <map>
#include "font/agsfontrenderer.h"

namespace AGS3 {

struct ALFONT_FONT;

class TTFFontRenderer : public IAGSFontRenderer, public IAGSFontRenderer2 {
public:
	// IAGSFontRenderer implementation
	bool LoadFromDisk(int fontNumber, int fontSize) override;
	void FreeMemory(int fontNumber) override;
	bool SupportsExtendedCharacters(int fontNumber) override {
		return true;
	}
	int GetTextWidth(const char *text, int fontNumber) override;
	int GetTextHeight(const char *text, int fontNumber) override;
	void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
	void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override;
	void EnsureTextValidForFont(char *text, int fontNumber) override;

	// IAGSFontRenderer2 implementation
	bool IsBitmapFont() override;
	bool LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params) override;

private:
	struct FontData {
		ALFONT_FONT *AlFont;
		FontRenderParams Params;
	};
	std::map<int, FontData> _fontData;
};

} // namespace AGS3

#endif
