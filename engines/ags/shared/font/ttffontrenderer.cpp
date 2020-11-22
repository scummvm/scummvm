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

//include <alfont.h>
#include "ags/shared/core/platform.h"

#define AGS_OUTLINE_FONT_FIX (!AGS_PLATFORM_OS_WINDOWS)

#include "ags/shared/core/assetmanager.h"
#include "ags/shared/font/ttffontrenderer.h"
#include "ags/shared/util/stream.h"

#if AGS_OUTLINE_FONT_FIX // TODO: factor out the hack in LoadFromDiskEx
#include "ags/shared/ac/gamestructdefines.h"
#include "ags/shared/font/fonts.h"
#endif

namespace AGS3 {

using namespace AGS::Shared;

// project-specific implementation
extern bool ShouldAntiAliasText();

ALFONT_FONT *tempttffnt;
ALFONT_FONT *get_ttf_block(unsigned char *fontptr) {
	memcpy(&tempttffnt, &fontptr[4], sizeof(tempttffnt));
	return tempttffnt;
}


// ***** TTF RENDERER *****
void TTFFontRenderer::AdjustYCoordinateForFont(int *ycoord, int fontNumber) {
	// TTF fonts already have space at the top, so try to remove the gap
	// TODO: adding -1 was here before (check the comment above),
	// but how universal is this "space at the top"?
	// Also, why is this function used only in one case of text rendering?
	// Review this after we upgrade the font library.
	ycoord[0]--;
}

void TTFFontRenderer::EnsureTextValidForFont(char *text, int fontNumber) {
	// do nothing, TTF can handle all characters
}

int TTFFontRenderer::GetTextWidth(const char *text, int fontNumber) {
	return alfont_text_length(_fontData[fontNumber].AlFont, text);
}

int TTFFontRenderer::GetTextHeight(const char *text, int fontNumber) {
	return alfont_text_height(_fontData[fontNumber].AlFont);
}

void TTFFontRenderer::RenderText(const char *text, int fontNumber, BITMAP *destination, int x, int y, int colour) {
	if (y > destination->cb)  // optimisation
		return;

	// Y - 1 because it seems to get drawn down a bit
	if ((ShouldAntiAliasText()) && (bitmap_color_depth(destination) > 8))
		alfont_textout_aa(destination, _fontData[fontNumber].AlFont, text, x, y - 1, colour);
	else
		alfont_textout(destination, _fontData[fontNumber].AlFont, text, x, y - 1, colour);
}

bool TTFFontRenderer::LoadFromDisk(int fontNumber, int fontSize) {
	return LoadFromDiskEx(fontNumber, fontSize, nullptr);
}

bool TTFFontRenderer::IsBitmapFont() {
	return false;
}

bool TTFFontRenderer::LoadFromDiskEx(int fontNumber, int fontSize, const FontRenderParams *params) {
	String file_name = String::FromFormat("agsfnt%d.ttf", fontNumber);
	Stream *reader = AssetManager::OpenAsset(file_name);
	char *membuffer;

	if (reader == nullptr)
		return false;

	long lenof = AssetManager::GetLastAssetSize();

	membuffer = (char *)malloc(lenof);
	reader->ReadArray(membuffer, lenof, 1);
	delete reader;

	ALFONT_FONT *alfptr = alfont_load_font_from_mem(membuffer, lenof);
	free(membuffer);

	if (alfptr == nullptr)
		return false;

	// TODO: move this somewhere, should not be right here
#if AGS_OUTLINE_FONT_FIX
	// FIXME: (!!!) this fix should be done differently:
	// 1. Find out which OUTLINE font was causing troubles;
	// 2. Replace outline method ONLY if that troublesome font is used as outline.
	// 3. Move this fix somewhere else!! (right after game load routine?)
	//
	// Check for the LucasFan font since it comes with an outline font that
	// is drawn incorrectly with Freetype versions > 2.1.3.
	// A simple workaround is to disable outline fonts for it and use
	// automatic outline drawing.
	if (get_font_outline(fontNumber) >= 0 &&
		strcmp(alfont_get_name(alfptr), "LucasFan-Font") == 0)
		set_font_outline(fontNumber, FONT_OUTLINE_AUTO);
#endif
	if (fontSize == 0)
		fontSize = 8; // compatibility fix
	if (params && params->SizeMultiplier > 1)
		fontSize *= params->SizeMultiplier;
	if (fontSize > 0)
		alfont_set_font_size(alfptr, fontSize);

	_fontData[fontNumber].AlFont = alfptr;
	_fontData[fontNumber].Params = params ? *params : FontRenderParams();
	return true;
}

void TTFFontRenderer::FreeMemory(int fontNumber) {
	alfont_destroy_font(_fontData[fontNumber].AlFont);
	_fontData.erase(fontNumber);
}

} // namespace AGS3
