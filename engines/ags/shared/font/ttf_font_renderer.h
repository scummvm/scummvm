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

#ifndef AGS_SHARED_FONT_TTF_FONT_RENDERER_H
#define AGS_SHARED_FONT_TTF_FONT_RENDERER_H

#include "common/std/map.h"
#include "ags/shared/font/ags_font_renderer.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

struct ALFONT_FONT;

class TTFFontRenderer : public IAGSFontRendererInternal {
public:
	virtual ~TTFFontRenderer() {}

	// IAGSFontRenderer implementation
	bool LoadFromDisk(int fontNumber, int fontSize) override;
	void FreeMemory(int fontNumber) override;
	bool SupportsExtendedCharacters(int /*fontNumber*/) override {
		return true;
	}
	int GetTextWidth(const char *text, int fontNumber) override;
	int GetTextHeight(const char *text, int fontNumber) override;
	void RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) override;
	void AdjustYCoordinateForFont(int *ycoord, int fontNumber) override;
	void EnsureTextValidForFont(char *text, int fontNumber) override;

	// IAGSFontRenderer2 implementation
	int GetVersion() override { return 26; /* first compatible engine API version */ }
	const char *GetRendererName() override { return "TTFFontRenderer"; }
	const char *GetFontName(int fontNumber) override;
	int GetFontHeight(int fontNumber) override;
	int GetLineSpacing(int fontNumber) override { return 0; /* no specific spacing */ }

	// IAGSFontRendererInternal implementation
	bool IsBitmapFont() override;
	bool LoadFromDiskEx(int fontNumber, int fontSize, AGS::Shared::String *src_filename,
						const FontRenderParams *params, FontMetrics *metrics) override;
	void GetFontMetrics(int fontNumber, FontMetrics *metrics) override;
	void AdjustFontForAntiAlias(int fontNumber, bool aa_mode) override;

	//
	// Utility functions
	//
	// Try load the TTF font using provided point size, and report its metrics
	static bool MeasureFontOfPointSize(const AGS::Shared::String &filename, int size_pt, FontMetrics *metrics);
	// Try load the TTF font, find the point size which results in pixel height
	// as close to the requested as possible; report its metrics
	static bool MeasureFontOfPixelHeight(const AGS::Shared::String &filename, int pixel_height, FontMetrics *metrics);

private:
	struct FontData {
		ALFONT_FONT *AlFont;
		FontRenderParams Params;
	};
	std::map<int, FontData> _fontData;
};

} // namespace AGS3

#endif
