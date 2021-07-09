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

#include "ags/lib/alfont/alfont.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/ac/common.h" // set_our_eip
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/font/ttf_font_renderer.h"
#include "ags/shared/font/wfn_font_renderer.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gui/gui_defines.h" // MAXLINE
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"


namespace AGS3 {

// Project-dependent implementation
extern int wgettextwidth_compensate(const char *tex, int font);

#define STD_BUFFER_SIZE 3000

using namespace AGS::Shared;

namespace AGS {
namespace Shared {

Font::Font()
	: Renderer(nullptr)
	, Renderer2(nullptr) {
}

} // namespace Shared
} // namespace AGS

FontInfo::FontInfo()
	: Flags(0)
	, SizePt(0)
	, SizeMultiplier(1)
	, Outline(FONT_OUTLINE_NONE)
	, YOffset(0)
	, LineSpacing(0) {
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
	if (fontnum >= _GP(fonts).size() || !_GP(fonts)[fontnum].Renderer)
		return;
	_GP(fonts)[fontnum].Renderer->AdjustYCoordinateForFont(ypos, fontnum);
}

bool font_first_renderer_loaded() {
	return _GP(fonts).size() > 0 && _GP(fonts)[0].Renderer != nullptr;
}

bool is_font_loaded(size_t fontNumber) {
	return fontNumber < _GP(fonts).size() && _GP(fonts)[fontNumber].Renderer != nullptr;;
}

IAGSFontRenderer *font_replace_renderer(size_t fontNumber, IAGSFontRenderer *renderer) {
	if (fontNumber >= _GP(fonts).size())
		return nullptr;
	IAGSFontRenderer *oldRender = _GP(fonts)[fontNumber].Renderer;
	_GP(fonts)[fontNumber].Renderer = renderer;
	_GP(fonts)[fontNumber].Renderer2 = nullptr;
	return oldRender;
}

bool is_bitmap_font(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer2)
		return false;
	return _GP(fonts)[fontNumber].Renderer2->IsBitmapFont();
}

bool font_supports_extended_characters(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return false;
	return _GP(fonts)[fontNumber].Renderer->SupportsExtendedCharacters(fontNumber);
}

void ensure_text_valid_for_font(char *text, size_t fontnum) {
	if (fontnum >= _GP(fonts).size() || !_GP(fonts)[fontnum].Renderer)
		return;
	_GP(fonts)[fontnum].Renderer->EnsureTextValidForFont(text, fontnum);
}

int get_font_scaling_mul(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	return _GP(fonts)[fontNumber].Info.SizeMultiplier;
}

int wgettextwidth(const char *texx, size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	return _GP(fonts)[fontNumber].Renderer->GetTextWidth(texx, fontNumber);
}

int wgettextheight(const char *text, size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	return _GP(fonts)[fontNumber].Renderer->GetTextHeight(text, fontNumber);
}

int get_font_outline(size_t font_number) {
	if (font_number >= _GP(fonts).size())
		return FONT_OUTLINE_NONE;
	return _GP(fonts)[font_number].Info.Outline;
}

void set_font_outline(size_t font_number, int outline_type) {
	if (font_number >= _GP(fonts).size())
		return;
	_GP(fonts)[font_number].Info.Outline = outline_type;
}

int getfontheight(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	// There is no explicit method for getting maximal possible height of any
	// random font renderer at the moment; the implementations of GetTextHeight
	// are allowed to return varied results depending on the text parameter.
	// We use special line of text to get more or less reliable font height.
	const char *height_test_string = "ZHwypgfjqhkilIK";
	return _GP(fonts)[fontNumber].Renderer->GetTextHeight(height_test_string, fontNumber);
}

int getfontlinespacing(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return 0;
	int spacing = _GP(fonts)[fontNumber].Info.LineSpacing;
	// If the spacing parameter is not provided, then return default
	// spacing, that is font's height.
	return spacing > 0 ? spacing : getfontheight(fontNumber);
}

bool use_default_linespacing(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return false;
	return _GP(fonts)[fontNumber].Info.LineSpacing == 0;
}

// Project-dependent implementation
extern int wgettextwidth_compensate(const char *tex, int font);

namespace AGS {
namespace Common {
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
	// NOTE: we do not need to decode utf8 here, because
	// we are only searching for low-code ascii chars.
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

	char *scan_ptr = theline;
	char *prev_ptr = theline;
	char *last_whitespace = nullptr;
	while (1) {
		char *split_at = nullptr;

		if (*scan_ptr == 0) {
			// end of the text, add the last line if necessary
			if (scan_ptr > theline) {
				lines.Add(theline);
			}
			break;
		}

		if (*scan_ptr == ' ')
			last_whitespace = scan_ptr;

		// force end of line with the \n character
		if (*scan_ptr == '\n') {
			split_at = scan_ptr;
			// otherwise, see if we are too wide
		} else {
			// temporarily terminate the line in the *next* char and test its width
			char *next_ptr = scan_ptr;
			ugetx(&next_ptr);
			const int next_chwas = ugetc(next_ptr);
			*next_ptr = 0;

			if (wgettextwidth_compensate(theline, fonnt) > wii) {
				// line is too wide, order the split
				if (last_whitespace)
					// revert to the last whitespace
					split_at = last_whitespace;
				else
					// single very wide word, display as much as possible
					split_at = prev_ptr;
			}

			// restore the character that was there before
			usetc(next_ptr, next_chwas);
		}

		if (split_at == nullptr) {
			prev_ptr = scan_ptr;
			ugetx(&scan_ptr);
		} else {
			// check if even one char cannot fit...
			if (split_at == theline && !((*theline == ' ') || (*theline == '\n'))) {
				// cannot split with current width restriction
				lines.Reset();
				break;
			}
			// add this line; do the temporary terminator trick again
			const int next_chwas = *split_at;
			*split_at = 0;
			lines.Add(theline);
			usetc(split_at, next_chwas);
			// check if too many lines
			if (lines.Count() >= max_lines) {
				lines[lines.Count() - 1].Append("...");
				break;
			}
			// the next line starts from the split point
			theline = split_at;
			// skip the space or new line that caused the line break
			if ((*theline == ' ') || (*theline == '\n'))
				theline++;
			scan_ptr = theline;
			prev_ptr = theline;
			last_whitespace = nullptr;
		}
	}
	return lines.Count();
}

void wouttextxy(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, const char *texx) {
	if (fontNumber >= _GP(fonts).size())
		return;
	yyy += _GP(fonts)[fontNumber].Info.YOffset;
	if (yyy > ds->GetClip().Bottom)
		return;                   // each char is clipped but this speeds it up

	if (_GP(fonts)[fontNumber].Renderer != nullptr) {
		_GP(fonts)[fontNumber].Renderer->RenderText(texx, fontNumber, (BITMAP *)ds->GetAllegroBitmap(), xxx, yyy, text_color);
	}
}

void set_fontinfo(size_t fontNumber, const FontInfo &finfo) {
	if (fontNumber < _GP(fonts).size() && _GP(fonts)[fontNumber].Renderer)
		_GP(fonts)[fontNumber].Info = finfo;
}

// Loads a font from disk
bool wloadfont_size(size_t fontNumber, const FontInfo &font_info) {
	if (_GP(fonts).size() <= fontNumber)
		_GP(fonts).resize(fontNumber + 1);
	else
		wfreefont(fontNumber);
	FontRenderParams params;
	params.SizeMultiplier = font_info.SizeMultiplier;

	if (_GP(ttfRenderer).LoadFromDiskEx(fontNumber, font_info.SizePt, &params)) {
		_GP(fonts)[fontNumber].Renderer = &_GP(ttfRenderer);
		_GP(fonts)[fontNumber].Renderer2 = &_GP(ttfRenderer);
	} else if (_GP(wfnRenderer).LoadFromDiskEx(fontNumber, font_info.SizePt, &params)) {
		_GP(fonts)[fontNumber].Renderer = &_GP(wfnRenderer);
		_GP(fonts)[fontNumber].Renderer2 = &_GP(wfnRenderer);
	}

	if (_GP(fonts)[fontNumber].Renderer) {
		_GP(fonts)[fontNumber].Info = font_info;
		return true;
	}
	return false;
}

void wgtprintf(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, char *fmt, ...) {
	if (fontNumber >= _GP(fonts).size())
		return;

	char tbuffer[2000];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(tbuffer, fmt, ap);
	va_end(ap);
	wouttextxy(ds, xxx, yyy, fontNumber, text_color, tbuffer);
}

void wfreefont(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return;

	if (_GP(fonts)[fontNumber].Renderer != nullptr)
		_GP(fonts)[fontNumber].Renderer->FreeMemory(fontNumber);

	_GP(fonts)[fontNumber].Renderer = nullptr;
}

void free_all_fonts() {
	for (size_t i = 0; i < _GP(fonts).size(); ++i) {
		if (_GP(fonts)[i].Renderer != nullptr)
			_GP(fonts)[i].Renderer->FreeMemory(i);
	}
	_GP(fonts).clear();
}

} // namespace AGS3
