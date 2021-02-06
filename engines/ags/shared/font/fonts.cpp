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

#include "ags/lib/std/vector.h"
#include "ags/lib/alfont/alfont.h"
#include "ags/shared/ac/common.h" // set_our_eip
#include "ags/shared/ac/gamestructdefines.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/font/ttffontrenderer.h"
#include "ags/shared/font/wfnfontrenderer.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gui/guidefines.h" // MAXLINE
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

#define STD_BUFFER_SIZE 3000

using namespace AGS::Shared;

namespace AGS {
namespace Shared {

struct Font {
	IAGSFontRenderer *Renderer;
	IAGSFontRenderer2 *Renderer2;
	FontInfo            Info;

	Font();
};

Font::Font()
	: Renderer(nullptr)
	, Renderer2(nullptr) {
}

} // Common
} // AGS

static std::vector<Font> fonts;
static TTFFontRenderer ttfRenderer;
static WFNFontRenderer wfnRenderer;


FontInfo::FontInfo()
	: Flags(0)
	, SizePt(0)
	, SizeMultiplier(1)
	, Outline(FONT_OUTLINE_NONE)
	, YOffset(0)
	, LineSpacing(0)
	, AutoOutlineStyle(kRounded)
	, AutoOutlineThickness(1) {
}


void init_font_renderer() {
	alfont_init();
	alfont_text_mode(-1);
}

void shutdown_font_renderer() {
	set_our_eip(9919);
	alfont_exit();
}

void adjust_y_coordinate_for_text(int *ypos, size_t fontnum) {
	if (fontnum >= fonts.size() || !fonts[fontnum].Renderer)
		return;
	fonts[fontnum].Renderer->AdjustYCoordinateForFont(ypos, fontnum);
}

bool font_first_renderer_loaded() {
	return fonts.size() > 0 && fonts[0].Renderer != nullptr;
}

bool is_font_loaded(size_t fontNumber) {
	return fontNumber < fonts.size() && fonts[fontNumber].Renderer != nullptr;;
}

IAGSFontRenderer *font_replace_renderer(size_t fontNumber, IAGSFontRenderer *renderer) {
	if (fontNumber >= fonts.size())
		return nullptr;
	IAGSFontRenderer *oldRender = fonts[fontNumber].Renderer;
	fonts[fontNumber].Renderer = renderer;
	fonts[fontNumber].Renderer2 = nullptr;
	return oldRender;
}

bool is_bitmap_font(size_t fontNumber) {
	if (fontNumber >= fonts.size() || !fonts[fontNumber].Renderer2)
		return false;
	return fonts[fontNumber].Renderer2->IsBitmapFont();
}

bool font_supports_extended_characters(size_t fontNumber) {
	if (fontNumber >= fonts.size() || !fonts[fontNumber].Renderer)
		return false;
	return fonts[fontNumber].Renderer->SupportsExtendedCharacters(fontNumber);
}

void ensure_text_valid_for_font(char *text, size_t fontnum) {
	if (fontnum >= fonts.size() || !fonts[fontnum].Renderer)
		return;
	fonts[fontnum].Renderer->EnsureTextValidForFont(text, fontnum);
}

int get_font_scaling_mul(size_t fontNumber) {
	if (fontNumber >= fonts.size() || !fonts[fontNumber].Renderer)
		return 0;
	return fonts[fontNumber].Info.SizeMultiplier;
}

int wgettextwidth(const char *texx, size_t fontNumber) {
	if (fontNumber >= fonts.size() || !fonts[fontNumber].Renderer)
		return 0;
	return fonts[fontNumber].Renderer->GetTextWidth(texx, fontNumber);
}

int wgettextheight(const char *text, size_t fontNumber) {
	if (fontNumber >= fonts.size() || !fonts[fontNumber].Renderer)
		return 0;
	return fonts[fontNumber].Renderer->GetTextHeight(text, fontNumber);
}

int get_font_outline(size_t font_number) {
	if (font_number >= fonts.size())
		return FONT_OUTLINE_NONE;
	return fonts[font_number].Info.Outline;
}

int get_font_outline_thickness(size_t font_number) {
	if (font_number >= fonts.size())
		return 0;
	return fonts[font_number].Info.AutoOutlineThickness;
}

void set_font_outline(size_t font_number, int outline_type) {
	if (font_number >= fonts.size())
		return;
	fonts[font_number].Info.Outline = FONT_OUTLINE_AUTO;
}

int getfontheight(size_t fontNumber) {
	if (fontNumber >= fonts.size() || !fonts[fontNumber].Renderer)
		return 0;
	// There is no explicit method for getting maximal possible height of any
	// random font renderer at the moment; the implementations of GetTextHeight
	// are allowed to return varied results depending on the text parameter.
	// We use special line of text to get more or less reliable font height.
	const char *height_test_string = "ZHwypgfjqhkilIK";
	return fonts[fontNumber].Renderer->GetTextHeight(height_test_string, fontNumber);
}

int getfontlinespacing(size_t fontNumber) {
	if (fontNumber >= fonts.size())
		return 0;
	int spacing = fonts[fontNumber].Info.LineSpacing;
	// If the spacing parameter is not provided, then return default
	// spacing, that is font's height.
	return spacing > 0 ? spacing : getfontheight(fontNumber);
}

bool use_default_linespacing(size_t fontNumber) {
	if (fontNumber >= fonts.size())
		return false;
	return fonts[fontNumber].Info.LineSpacing == 0;
}

// Project-dependent implementation
extern int wgettextwidth_compensate(const char *tex, int font);

namespace AGS {
namespace Shared {
SplitLines Lines;
}
}

// Replaces AGS-specific linebreak tags with common '\n'
void unescape_script_string(const char *cstr, std::vector<char> &out) {
	out.clear();
	// Handle the special case of the first char
	if (cstr[0] == '[') {
		out.push_back('\n');
		cstr++;
	}
	// Replace all other occurrences as they're found
	const char *off;
	for (off = cstr; *off; ++off) {
		if (*off != '[') continue;
		if (*(off - 1) == '\\') {
			// convert \[ into [
			out.insert(out.end(), cstr, off - 1);
			out.push_back('[');
		} else {
			// convert [ into \n
			out.insert(out.end(), cstr, off);
			out.push_back('\n');
		}
		cstr = off + 1;
	}
	out.insert(out.end(), cstr, off + 1);
}

// Break up the text into lines
size_t split_lines(const char *todis, SplitLines &lines, int wii, int fonnt, size_t max_lines) {
	// NOTE: following hack accomodates for the legacy math mistake in split_lines.
	// It's hard to tell how cruicial it is for the game looks, so research may be needed.
	// TODO: IMHO this should rely not on game format, but script API level, because it
	// defines necessary adjustments to game scripts. If you want to fix this, find a way to
	// pass this flag here all the way from game.options[OPT_BASESCRIPTAPI] (or game format).
	//
	// if (game.options[OPT_BASESCRIPTAPI] < $Your current version$)
	wii -= 1;

	lines.Reset();
	unescape_script_string(todis, lines.LineBuf);
	char *theline = &lines.LineBuf.front();

	size_t i = 0;
	size_t splitAt;
	char nextCharWas;
	while (1) {
		splitAt = (size_t)-1;

		if (theline[i] == 0) {
			// end of the text, add the last line if necessary
			if (i > 0) {
				lines.Add(theline);
			}
			break;
		}

		// temporarily terminate the line here and test its width
		nextCharWas = theline[i + 1];
		theline[i + 1] = 0;

		// force end of line with the \n character
		if (theline[i] == '\n')
			splitAt = i;
		// otherwise, see if we are too wide
		else if (wgettextwidth_compensate(theline, fonnt) > wii) {
			int endline = i;
			while ((theline[endline] != ' ') && (endline > 0))
				endline--;

			// single very wide word, display as much as possible
			if (endline == 0)
				endline = i - 1;

			splitAt = endline;
		}

		// restore the character that was there before
		theline[i + 1] = nextCharWas;

		if (splitAt != (size_t)-1) {
			if (splitAt == 0 && !((theline[0] == ' ') || (theline[0] == '\n'))) {
				// cannot split with current width restriction
				lines.Reset();
				break;
			}
			// add this line
			nextCharWas = theline[splitAt];
			theline[splitAt] = 0;
			lines.Add(theline);
			theline[splitAt] = nextCharWas;
			if (lines.Count() >= max_lines) {
				lines[lines.Count() - 1].Append("...");
				break;
			}
			// the next line starts from here
			theline += splitAt;
			// skip the space or new line that caused the line break
			if ((theline[0] == ' ') || (theline[0] == '\n'))
				theline++;
			i = (size_t)-1;
		}

		i++;
	}
	return lines.Count();
}

void wouttextxy(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, const char *texx) {
	if (fontNumber >= fonts.size())
		return;
	yyy += fonts[fontNumber].Info.YOffset;
	if (yyy > ds->GetClip().Bottom)
		return;                   // each char is clipped but this speeds it up

	if (fonts[fontNumber].Renderer != nullptr) {
		fonts[fontNumber].Renderer->RenderText(texx, fontNumber, (BITMAP *)ds->GetAllegroBitmap(), xxx, yyy, text_color);
	}
}

void set_fontinfo(size_t fontNumber, const FontInfo &finfo) {
	if (fontNumber < fonts.size() && fonts[fontNumber].Renderer)
		fonts[fontNumber].Info = finfo;
}

// Loads a font from disk
bool wloadfont_size(size_t fontNumber, const FontInfo &font_info) {
	if (fonts.size() <= fontNumber)
		fonts.resize(fontNumber + 1);
	else
		wfreefont(fontNumber);
	FontRenderParams params;
	params.SizeMultiplier = font_info.SizeMultiplier;

	if (ttfRenderer.LoadFromDiskEx(fontNumber, font_info.SizePt, &params)) {
		fonts[fontNumber].Renderer = &ttfRenderer;
		fonts[fontNumber].Renderer2 = &ttfRenderer;
	} else if (wfnRenderer.LoadFromDiskEx(fontNumber, font_info.SizePt, &params)) {
		fonts[fontNumber].Renderer = &wfnRenderer;
		fonts[fontNumber].Renderer2 = &wfnRenderer;
	}

	if (fonts[fontNumber].Renderer) {
		fonts[fontNumber].Info = font_info;
		return true;
	}
	return false;
}

void wgtprintf(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, char *fmt, ...) {
	if (fontNumber >= fonts.size())
		return;

	char tbuffer[2000];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(tbuffer, fmt, ap);
	va_end(ap);
	wouttextxy(ds, xxx, yyy, fontNumber, text_color, tbuffer);
}

void wfreefont(size_t fontNumber) {
	if (fontNumber >= fonts.size())
		return;

	if (fonts[fontNumber].Renderer != nullptr)
		fonts[fontNumber].Renderer->FreeMemory(fontNumber);

	fonts[fontNumber].Renderer = nullptr;
}

void free_all_fonts() {
	for (size_t i = 0; i < fonts.size(); ++i) {
		if (fonts[i].Renderer != nullptr)
			fonts[i].Renderer->FreeMemory(i);
	}
	fonts.clear();
}

} // namespace AGS3
