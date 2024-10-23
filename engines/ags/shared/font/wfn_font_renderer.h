/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef AGS_SHARED_FONT_WFN_FONT_RENDERER_H
#define AGS_SHARED_FONT_WFN_FONT_RENDERER_H

#include "common/std/map.h"
#include "ags/lib/std.h"
#include "ags/shared/font/ags_font_renderer.h"

namespace AGS3 {

class WFNFont;

class WFNFontRenderer : public IAGSFontRendererInternal {
public:
	// IAGSFontRenderer implementation
	virtual ~WFNFontRenderer() {}

	bool LoadFromDisk(int fontNumber, int fontSize) override;
	void FreeMemory(int fontNumber) override;
	bool SupportsExtendedCharacters(int fontNumber) override;
	int GetTextWidth(const char *text, int fontNumber) override;
	int GetTextHeight(const char *text, int fontNumber) override;
	void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
	void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override;
	void EnsureTextValidForFont(char *text, int fontNumber) override;

	// IAGSFontRenderer2 implementation
	int GetVersion() override { return 26; /* first compatible engine API version */ }
	const char *GetRendererName() override { return "WFNFontRenderer"; }
	const char *GetFontName(int /*fontNumber*/) override { return ""; }
	int GetFontHeight(int fontNumber) override { return 0; /* TODO? */ }
	int GetLineSpacing(int fontNumber) override { return 0; /* no specific spacing */ }

	// IAGSFontRendererInternal implementation
	bool IsBitmapFont() override;
	bool LoadFromDiskEx(int fontNumber, int fontSize, AGS::Shared::String *src_filename,
						const FontRenderParams *params, FontMetrics *metrics) override;
	void GetFontMetrics(int fontNumber, FontMetrics *metrics) override { *metrics = FontMetrics(); }
	void AdjustFontForAntiAlias(int /*fontNumber*/, bool /*aa_mode*/) override { /* do nothing */ }

private:
	struct FontData {
		WFNFont *Font;
		FontRenderParams Params;
	};
	std::map<int, FontData> _fontData;
};

} // namespace AGS3

#endif
