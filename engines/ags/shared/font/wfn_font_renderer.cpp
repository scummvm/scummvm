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

#include "ags/shared/font/wfn_font_renderer.h"
#include "ags/shared/ac/common.h" // our_eip
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/font/wfn_font.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void WFNFontRenderer::AdjustYCoordinateForFont(int *ycoord, int fontNumber) {
	// Do nothing
}

void WFNFontRenderer::EnsureTextValidForFont(char *text, int fontNumber) {
	// Do nothing
}

int WFNFontRenderer::GetTextWidth(const char *text, int fontNumber) {
	const WFNFont *font = _fontData[fontNumber].Font;
	const FontRenderParams &params = _fontData[fontNumber].Params;
	int text_width = 0;

	for (int code = ugetxc(&text); code; code = ugetxc(&text)) {
		text_width += font->GetChar(code).Width;
	}
	return text_width * params.SizeMultiplier;
}

int WFNFontRenderer::GetTextHeight(const char *text, int fontNumber) {
	const WFNFont *font = _fontData[fontNumber].Font;
	const FontRenderParams &params = _fontData[fontNumber].Params;
	int max_height = 0;

	for (int code = ugetxc(&text); code; code = ugetxc(&text)) {
		const uint16_t height = font->GetChar(code).Height;
		max_height = std::max(max_height, static_cast<int>(height));
	}
	return max_height * params.SizeMultiplier;
}

static int RenderChar(Bitmap *ds, const int at_x, const int at_y, Rect clip,
	const WFNChar &wfn_char, const int scale, const color_t text_color);

void WFNFontRenderer::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) {
	int oldeip = get_our_eip();
	set_our_eip(415);

	const WFNFont *font = _fontData[fontNumber].Font;
	const FontRenderParams &params = _fontData[fontNumber].Params;
	Bitmap ds(destination, true);

	// NOTE: allegro's putpixel ignores clipping (optimization),
	// so we'll have to accommodate for that ourselves
	Rect clip = ds.GetClip();
	for (int code = ugetxc(&text); code; code = ugetxc(&text))
		x += RenderChar(&ds, x, y, clip, font->GetChar(code), params.SizeMultiplier, colour);

	set_our_eip(oldeip);
}

static int RenderChar(Bitmap *ds, const int at_x, const int at_y, Rect clip,
	const WFNChar &wfn_char, const int scale, const color_t text_color) {
	const int width = wfn_char.Width;
	const int height = wfn_char.Height;
	const unsigned char *actdata = wfn_char.Data;
	const int bytewid = wfn_char.GetRowByteCount();

	int sx = std::max(at_x, clip.Left), ex = clip.Right + 1;
	int sy = std::max(at_y, clip.Top), ey = clip.Bottom + 1;
	int sw = std::max(0, clip.Left - at_x);
	int sh = std::max(0, clip.Top - at_y);
	for (int h = sh, y = sy; h < height && y < ey; ++h, y += scale) {
		for (int w = sw, x = sx; w < width && x < ex; ++w, x += scale) {
			if (((actdata[h * bytewid + (w / 8)] & (0x80 >> (w % 8))) != 0)) {
				if (scale > 1) {
					ds->FillRect(RectWH(x, y, scale, scale), text_color);
				} else {
					ds->PutPixel(x, y, text_color);
				}
			}
		}
	}
	return width * scale;
}

bool WFNFontRenderer::LoadFromDisk(int fontNumber, int fontSize) {
	return LoadFromDiskEx(fontNumber, fontSize, nullptr, nullptr, nullptr);
}

bool WFNFontRenderer::IsBitmapFont() {
	return true;
}

bool WFNFontRenderer::LoadFromDiskEx(int fontNumber, int /*fontSize*/, String *src_filename,
									 const FontRenderParams *params, FontMetrics *metrics) {
	String file_name;
	Stream *ffi = nullptr;

	file_name.Format("agsfnt%d.wfn", fontNumber);
	ffi = _GP(AssetMgr)->OpenAsset(file_name);
	if (ffi == nullptr) {
		// actual font not found, try font 0 instead
		// FIXME: this should not be done here in this font renderer implementation,
		// but somewhere outside, when whoever calls this method
		file_name = "agsfnt0.wfn";
		ffi = _GP(AssetMgr)->OpenAsset(file_name);
		if (ffi == nullptr)
			return false;
	}

	WFNFont *font = new WFNFont();
	WFNError err = font->ReadFromFile(ffi);
	delete ffi;
	if (err == kWFNErr_HasBadCharacters)
		Debug::Printf(kDbgMsg_Warn, "WARNING: font '%s' has mistakes in data format, some characters may be displayed incorrectly", file_name.GetCStr());
	else if (err != kWFNErr_NoError) {
		delete font;
		return false;
	}
	_fontData[fontNumber].Font = font;
	_fontData[fontNumber].Params = params ? *params : FontRenderParams();
	if (src_filename)
		*src_filename = file_name;
	if (metrics)
		*metrics = FontMetrics();
	return true;
}

void WFNFontRenderer::FreeMemory(int fontNumber) {
	delete _fontData[fontNumber].Font;
	_fontData.erase(fontNumber);
}

bool WFNFontRenderer::SupportsExtendedCharacters(int fontNumber) {
	return _fontData[fontNumber].Font->GetCharCount() > 128;
}

} // namespace AGS3
