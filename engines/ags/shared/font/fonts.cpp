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

#include "common/std/algorithm.h"
#include "ags/lib/alfont/alfont.h"
#include "common/std/vector.h"
#include "ags/shared/ac/common.h" // set_our_eip
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/font/ttf_font_renderer.h"
#include "ags/shared/font/wfn_font_renderer.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gui/gui_defines.h" // MAXLINE
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

FontInfo::FontInfo()
	: Flags(0)
	, Size(0)
	, SizeMultiplier(1)
	, Outline(FONT_OUTLINE_NONE)
	, YOffset(0)
	, LineSpacing(0)
	, AutoOutlineStyle(kSquared)
	, AutoOutlineThickness(0) {
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
	return fontNumber < _GP(fonts).size() && _GP(fonts)[fontNumber].Renderer != nullptr;
}

// Finish font's initialization
static void font_post_init(size_t fontNumber) {
	Font &font = _GP(fonts)[fontNumber];
	// If no font height property was provided, then try several methods,
	// depending on which interface is available
	if (font.Metrics.NominalHeight == 0 && font.Renderer) {
		int height = 0;
		if (font.Renderer2)
			height = font.Renderer2->GetFontHeight(fontNumber);
		if (height <= 0) {
			// With the old renderer we have to rely on GetTextHeight;
			// the implementations of GetTextHeight are allowed to return varied
			// results depending on the text parameter.
			// We use special line of text to get more or less reliable font height.
			const char *height_test_string = "ZHwypgfjqhkilIK";
			height = font.Renderer->GetTextHeight(height_test_string, fontNumber);
		}

		font.Metrics.NominalHeight = std::max(0, height);
		font.Metrics.RealHeight = font.Metrics.NominalHeight;
		font.Metrics.VExtent = std::make_pair(0, font.Metrics.RealHeight);
	}
	// Use either nominal or real pixel height to define font's logical height
	// and default linespacing; logical height = nominal height is compatible with the old games
	font.Metrics.CompatHeight = (font.Info.Flags & FFLG_REPORTNOMINALHEIGHT) != 0 ?
		font.Metrics.NominalHeight : font.Metrics.RealHeight;

	if (font.Info.Outline != FONT_OUTLINE_AUTO) {
		font.Info.AutoOutlineThickness = 0;
	}

	// If no linespacing property was provided, then try several methods,
	// depending on which interface is available
	font.LineSpacingCalc = font.Info.LineSpacing;
	if (font.Info.LineSpacing == 0) {
		int linespacing = 0;
		if (font.Renderer2)
			linespacing = font.Renderer2->GetLineSpacing(fontNumber);
		if (linespacing > 0) {
			font.LineSpacingCalc = linespacing;
		} else {
			// Calculate default linespacing from the font height + outline thickness.
			font.Info.Flags |= FFLG_DEFLINESPACING;
			font.LineSpacingCalc = font.Metrics.CompatHeight + 2 * font.Info.AutoOutlineThickness;
		}
	}
}

static void font_replace_renderer(size_t fontNumber, IAGSFontRenderer* renderer, IAGSFontRenderer2* renderer2) {
	_GP(fonts)[fontNumber].Renderer = renderer;
	_GP(fonts)[fontNumber].Renderer2 = renderer2;
	// If this is one of our built-in font renderers, then correctly
	// reinitialize interfaces and font metrics
	if ((renderer == &_GP(ttfRenderer)) || (renderer == &_GP(wfnRenderer))) {
		_GP(fonts)[fontNumber].RendererInt = static_cast<IAGSFontRendererInternal*>(renderer);
		_GP(fonts)[fontNumber].RendererInt->GetFontMetrics(fontNumber, &_GP(fonts)[fontNumber].Metrics);
	} else {
		// Otherwise, this is probably coming from plugin
		_GP(fonts)[fontNumber].RendererInt = nullptr;
		_GP(fonts)[fontNumber].Metrics = FontMetrics(); // reset to defaults
	}
	font_post_init(fontNumber);
}


IAGSFontRenderer *font_replace_renderer(size_t fontNumber, IAGSFontRenderer *renderer) {
	if (fontNumber >= _GP(fonts).size())
		return nullptr;
	IAGSFontRenderer* old_render = _GP(fonts)[fontNumber].Renderer;
	font_replace_renderer(fontNumber, renderer, nullptr);
	return old_render;
}

IAGSFontRenderer *font_replace_renderer(size_t fontNumber, IAGSFontRenderer2 *renderer) {
	if (fontNumber >= _GP(fonts).size())
		return nullptr;
	IAGSFontRenderer* old_render = _GP(fonts)[fontNumber].Renderer;
	font_replace_renderer(fontNumber, renderer, renderer);
	return old_render;
}

void font_recalc_metrics(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return;
	_GP(fonts)[fontNumber].Metrics = FontMetrics();
	font_post_init(fontNumber);
}

bool is_bitmap_font(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].RendererInt)
		return false;
	return _GP(fonts)[fontNumber].RendererInt->IsBitmapFont();
}

bool font_supports_extended_characters(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return false;
	return _GP(fonts)[fontNumber].Renderer->SupportsExtendedCharacters(fontNumber);
}

const char *get_font_name(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer2)
		return "";
	const char *name = _GP(fonts)[fontNumber].Renderer2->GetFontName(fontNumber);
	return name ? name : "";
}

int get_font_flags(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return 0;
	return _GP(fonts)[fontNumber].Info.Flags;
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

int get_text_width(const char *texx, size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	return _GP(fonts)[fontNumber].Renderer->GetTextWidth(texx, fontNumber);
}

int get_text_width_outlined(const char *text, size_t font_number) {
	if (font_number >= _GP(fonts).size() || !_GP(fonts)[font_number].Renderer)
		return 0;
	if (text == nullptr || text[0] == 0) // we ignore outline width since the text is empty
		return 0;
	int self_width = _GP(fonts)[font_number].Renderer->GetTextWidth(text, font_number);
	int outline = _GP(fonts)[font_number].Info.Outline;
	if (outline < 0 || static_cast<size_t>(outline) > _GP(fonts).size()) { // FONT_OUTLINE_AUTO or FONT_OUTLINE_NONE
		return self_width + 2 * _GP(fonts)[font_number].Info.AutoOutlineThickness;
	}
	int outline_width = _GP(fonts)[outline].Renderer->GetTextWidth(text, outline);
	return MAX(self_width, outline_width);
}

int get_font_outline(size_t font_number) {
	if (font_number >= _GP(fonts).size())
		return FONT_OUTLINE_NONE;
	return _GP(fonts)[font_number].Info.Outline;
}

int get_font_outline_thickness(size_t font_number) {
	if (font_number >= _GP(fonts).size())
		return 0;
	return _GP(fonts)[font_number].Info.AutoOutlineThickness;
}

void set_font_outline(size_t font_number, int outline_type,
		enum FontInfo::AutoOutlineStyle style, int thickness) {
	if (font_number >= _GP(fonts).size())
		return;
	_GP(fonts)[font_number].Info.Outline = outline_type;
	_GP(fonts)[font_number].Info.AutoOutlineStyle = style;
	_GP(fonts)[font_number].Info.AutoOutlineThickness = thickness;
}

bool is_font_antialiased(size_t font_number) {
	if (font_number >= _GP(fonts).size())
		return false;
	return ShouldAntiAliasText() && !is_bitmap_font(font_number);
}

int get_font_height(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	return _GP(fonts)[fontNumber].Metrics.CompatHeight;
}

int get_font_height_outlined(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	int self_height = _GP(fonts)[fontNumber].Metrics.CompatHeight;
	int outline = _GP(fonts)[fontNumber].Info.Outline;
	if (outline < 0 || static_cast<size_t>(outline) > _GP(fonts).size()) { // FONT_OUTLINE_AUTO or FONT_OUTLINE_NONE
		return self_height + 2 * _GP(fonts)[fontNumber].Info.AutoOutlineThickness;
	}
	int outline_height = _GP(fonts)[outline].Metrics.CompatHeight;
	return MAX(self_height, outline_height);
}

int get_font_surface_height(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return 0;
	return _GP(fonts)[fontNumber].Metrics.ExtentHeight();
}

std::pair<int, int> get_font_surface_extent(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size() || !_GP(fonts)[fontNumber].Renderer)
		return std::make_pair(0, 0);
	return _GP(fonts)[fontNumber].Metrics.VExtent;
}

int get_font_linespacing(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return 0;
	return _GP(fonts)[fontNumber].LineSpacingCalc;
}

void set_font_linespacing(size_t fontNumber, int spacing) {
	if (fontNumber < _GP(fonts).size()) {
		_GP(fonts)[fontNumber].Info.Flags &= ~FFLG_DEFLINESPACING;
		_GP(fonts)[fontNumber].Info.LineSpacing = spacing;
		_GP(fonts)[fontNumber].LineSpacingCalc = spacing;
	}
}

int get_text_lines_height(size_t fontNumber, size_t numlines) {
	if (fontNumber >= _GP(fonts).size() || numlines == 0)
		return 0;
	return _GP(fonts)[fontNumber].LineSpacingCalc * (numlines - 1) +
		(_GP(fonts)[fontNumber].Metrics.CompatHeight +
			2 * _GP(fonts)[fontNumber].Info.AutoOutlineThickness);
}

int get_text_lines_surf_height(size_t fontNumber, size_t numlines) {
	if (fontNumber >= _GP(fonts).size() || numlines == 0)
		return 0;
	return _GP(fonts)[fontNumber].LineSpacingCalc * (numlines - 1) +
		(_GP(fonts)[fontNumber].Metrics.RealHeight +
			2 * _GP(fonts)[fontNumber].Info.AutoOutlineThickness);
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
	// NOTE: following hack accommodates for the legacy math mistake in split_lines.
	// It's hard to tell how crucial it is for the game looks, so research may be needed.
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

			if (get_text_width_outlined(theline, fonnt) > wii) {
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
			const int next_chwas = ugetc(split_at);
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
		if (text_color == makeacol32(255, 0, 255, 255)) { // transparent color (magenta)
			// WORKAROUND: Some Allegro routines are not implemented and alfont treats some magenta texts as invisible
			// even if the alpha channel is fully opaque
			// Slightly change the value if the game uses that color for fonts, so that they don't turn invisible
			debug(0, "Overriding transparent text color!");
			text_color--;
		}
		_GP(fonts)[fontNumber].Renderer->RenderText(texx, fontNumber, (BITMAP *)ds->GetAllegroBitmap(), xxx, yyy, text_color);
	}
}

void set_fontinfo(size_t fontNumber, const FontInfo &finfo) {
	if (fontNumber < _GP(fonts).size() && _GP(fonts)[fontNumber].Renderer) {
		_GP(fonts)[fontNumber].Info = finfo;
		font_post_init(fontNumber);
	}
}

FontInfo get_fontinfo(size_t font_number) {
	if (font_number < _GP(fonts).size())
		return _GP(fonts)[font_number].Info;
	return FontInfo();
}

// Loads a font from disk
bool load_font_size(size_t fontNumber, const FontInfo &font_info) {
	if (_GP(fonts).size() <= fontNumber)
		_GP(fonts).resize(fontNumber + 1);
	else
		wfreefont(fontNumber);
	FontRenderParams params;
	params.SizeMultiplier = font_info.SizeMultiplier;
	params.LoadMode = (font_info.Flags & FFLG_LOADMODEMASK);
	FontMetrics metrics;

	Font &font = _GP(fonts)[fontNumber];
	String src_filename;
	if (_GP(ttfRenderer).LoadFromDiskEx(fontNumber, font_info.Size, &src_filename, &params, &metrics)) {
		font.Renderer = &_GP(ttfRenderer);
		font.Renderer2 = &_GP(ttfRenderer);
		font.RendererInt = &_GP(ttfRenderer);
	} else if (_GP(wfnRenderer).LoadFromDiskEx(fontNumber, font_info.Size, &src_filename, &params, &metrics)) {
		font.Renderer = &_GP(wfnRenderer);
		font.Renderer2 = &_GP(wfnRenderer);
		font.RendererInt = &_GP(wfnRenderer);
	}

	if (!font.Renderer)
		return false;

	font.Info = font_info;
	font.Metrics = metrics;
	font_post_init(fontNumber);

	Debug::Printf("Loaded font %d: %s, req size: %d; nominal h: %d, real h: %d, extent: %d,%d",
				  fontNumber, src_filename.GetCStr(), font_info.Size, font.Metrics.NominalHeight, font.Metrics.RealHeight,
				  font.Metrics.VExtent.first, font.Metrics.VExtent.second);
	return true;
}

void wgtprintf(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, char *fmt, ...) {
	if (fontNumber >= _GP(fonts).size())
		return;

	char tbuffer[2000];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(tbuffer, sizeof(tbuffer), fmt, ap);
	va_end(ap);
	wouttextxy(ds, xxx, yyy, fontNumber, text_color, tbuffer);
}

void alloc_font_outline_buffers(size_t font_number,
	Bitmap **text_stencil, Bitmap **outline_stencil,
	int text_width, int text_height, int color_depth) {
	if (font_number >= _GP(fonts).size())
		return;
	Font &f = _GP(fonts)[font_number];
	const int thick = 2 * f.Info.AutoOutlineThickness;
	if (f.TextStencil.IsNull() || (f.TextStencil.GetColorDepth() != color_depth) ||
		(f.TextStencil.GetWidth() < text_width) || (f.TextStencil.GetHeight() < text_height)) {
		int sw = f.TextStencil.IsNull() ? 0 : f.TextStencil.GetWidth();
		int sh = f.TextStencil.IsNull() ? 0 : f.TextStencil.GetHeight();
		sw = MAX(text_width, sw);
		sh = MAX(text_height, sh);
		f.TextStencil.Create(sw, sh, color_depth);
		f.OutlineStencil.Create(sw, sh + thick, color_depth);
		f.TextStencilSub.CreateSubBitmap(&f.TextStencil, RectWH(Size(text_width, text_height)));
		f.OutlineStencilSub.CreateSubBitmap(&f.OutlineStencil, RectWH(Size(text_width, text_height + thick)));
	} else {
		f.TextStencilSub.ResizeSubBitmap(text_width, text_height);
		f.OutlineStencilSub.ResizeSubBitmap(text_width, text_height + thick);
	}
	*text_stencil = &f.TextStencilSub;
	*outline_stencil = &f.OutlineStencilSub;
}

void adjust_fonts_for_render_mode(bool aa_mode) {
	for (size_t i = 0; i < _GP(fonts).size(); ++i) {
		if (_GP(fonts)[i].RendererInt != nullptr)
			_GP(fonts)[i].RendererInt->AdjustFontForAntiAlias(i, aa_mode);
	}
}

void wfreefont(size_t fontNumber) {
	if (fontNumber >= _GP(fonts).size())
		return;

	_GP(fonts)[fontNumber].TextStencilSub.Destroy();
	_GP(fonts)[fontNumber].OutlineStencilSub.Destroy();
	_GP(fonts)[fontNumber].TextStencil.Destroy();
	_GP(fonts)[fontNumber].OutlineStencil.Destroy();

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
