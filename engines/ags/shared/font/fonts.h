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

#ifndef AGS_SHARED_FONT_FONTS_H
#define AGS_SHARED_FONT_FONTS_H

#include "common/std/vector.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/util/string.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/font/ags_font_renderer.h"
#include "ags/shared/gfx/allegro_bitmap.h"

namespace AGS3 {

class IAGSFontRenderer;
class IAGSFontRenderer2;
class IAGSFontRendererInternal;
struct FontInfo;
struct FontRenderParams;

namespace AGS {
namespace Shared {

struct Font {
	// Classic font renderer interface
	IAGSFontRenderer *Renderer = nullptr;
	// Extended font renderer interface (optional)
	IAGSFontRenderer2 *Renderer2 = nullptr;
	// Internal interface (only for built-in renderers)
	IAGSFontRendererInternal *RendererInt = nullptr;
	FontInfo            Info;
	// Values received from the renderer and saved for the reference
	FontMetrics       Metrics;
	// Precalculated linespacing, based on font properties and compat settings
	int                 LineSpacingCalc = 0;

	// Outline buffers
	Bitmap TextStencil, TextStencilSub;
	Bitmap OutlineStencil, OutlineStencilSub;

	Font() {}
};

} // namespace Shared
} // namespace AGS

using namespace AGS;

void init_font_renderer();
void shutdown_font_renderer();
void adjust_y_coordinate_for_text(int *ypos, size_t fontnum);
IAGSFontRenderer *font_replace_renderer(size_t fontNumber, IAGSFontRenderer *renderer);
IAGSFontRenderer *font_replace_renderer(size_t fontNumber, IAGSFontRenderer2 *renderer);
 void font_recalc_metrics(size_t fontNumber);
bool font_first_renderer_loaded();
bool is_font_loaded(size_t fontNumber);
bool is_bitmap_font(size_t fontNumber);
bool font_supports_extended_characters(size_t fontNumber);
// Get font's name, if it's available, otherwise returns empty string
const char *get_font_name(size_t fontNumber);
// Get a collection of FFLG_* flags corresponding to this font
int get_font_flags(size_t fontNumber);
// TODO: with changes to WFN font renderer that implemented safe rendering of
// strings containing invalid chars (since 3.3.1) this function is not
// important, except for (maybe) few particular cases.
// Furthermore, its use complicated things, because AGS could modify some texts
// at random times (usually - drawing routines).
// Need to check whether it is safe to completely remove it.
void ensure_text_valid_for_font(char *text, size_t fontnum);
// Get font's scaling multiplier
int get_font_scaling_mul(size_t fontNumber);
// Calculate actual width of a line of text
int get_text_width(const char *texx, size_t fontNumber);
// Get the maximal width of the given font, with corresponding outlining
int get_text_width_outlined(const char *text, size_t font_number);
// Get font's height; this value is used for logical arrangement of UI elements;
// note that this is a "formal" font height, that may have different value
// depending on compatibility mode (used when running old games);
int get_font_height(size_t fontNumber);
// Get the maximal height of the given font, with corresponding outlining
int get_font_height_outlined(size_t fontNumber);
// Get font's surface height: this always returns the height enough to accommodate
// font letters on a bitmap or a texture; the distinction is needed for compatibility reasons
int get_font_surface_height(size_t fontNumber);
// Get font's maximal graphical extent: this means the farthest vertical positions of glyphs,
// relative to the "pen" position. Besides letting to calculate the surface height,
// this information also lets to detect if some of the glyphs may appear above y0.
std::pair<int, int> get_font_surface_extent(size_t fontNumber);
// Get font's line spacing
int get_font_linespacing(size_t fontNumber);
// Set font's line spacing
void set_font_linespacing(size_t fontNumber, int spacing);
// Get font's outline type
int  get_font_outline(size_t font_number);
// Get font's automatic outline thickness (if set)
int  get_font_outline_thickness(size_t font_number);
// Gets the total maximal height of the given number of lines printed with the given font;
// note that this uses formal font height, for compatibility purposes
int get_text_lines_height(size_t fontNumber, size_t numlines);
// Gets the height of a graphic surface enough to accommodate this number of text lines;
// note this accounts for the real pixel font height
int get_text_lines_surf_height(size_t fontNumber, size_t numlines);
// Set font's outline type
void set_font_outline(size_t font_number, int outline_type,
	enum FontInfo::AutoOutlineStyle style = FontInfo::kSquared, int thickness = 1);
bool is_font_antialiased(size_t font_number);
// Outputs a single line of text on the defined position on bitmap, using defined font, color and parameters
void wouttextxy(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, const char *texx);
// Assigns FontInfo to the font
void set_fontinfo(size_t fontNumber, const FontInfo &finfo);
// Gets full information about the font
FontInfo get_fontinfo(size_t font_number);
// Loads a font from disk
bool load_font_size(size_t fontNumber, const FontInfo &font_info); void wgtprintf(Shared::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, char *fmt, ...);
// Allocates two outline stencil buffers, or returns previously creates ones;
// these buffers are owned by the font, they should not be deleted by the caller.
void alloc_font_outline_buffers(size_t font_number,
	Shared::Bitmap **text_stencil, Shared::Bitmap **outline_stencil,
	int text_width, int text_height, int color_depth);
// Perform necessary adjustments on all fonts in case the text render mode changed (anti-aliasing etc)
void adjust_fonts_for_render_mode(bool aa_mode);
// Free particular font's data
void wfreefont(size_t fontNumber);
// Free all fonts data
void free_all_fonts();

// Tells if the text should be antialiased when possible
bool ShouldAntiAliasText();

// SplitLines class represents a list of lines and is meant to reduce
// subsequent memory (de)allocations if used often during game loops
// and drawing. For that reason it is not equivalent to std::vector,
// but keeps constructed String buffers intact for most time.
// TODO: implement proper strings pool.
class SplitLines {
public:
	inline size_t Count() const {
		return _count;
	}
	inline const Shared::String &operator[](size_t i) const {
		return _pool[i];
	}
	inline Shared::String &operator[](size_t i) {
		return _pool[i];
	}
	inline void Clear() {
		_pool.clear();
		_count = 0;
	}
	inline void Reset() {
		_count = 0;
	}
	inline void Add(const char *cstr) {
		if (_pool.size() == _count) _pool.resize(_count + 1);
		_pool[_count++].SetString(cstr);
	}

	// An auxiliary line processing buffer
	std::vector<char> LineBuf;

private:
	std::vector<Shared::String> _pool;
	size_t _count; // actual number of lines in use
};

// Break up the text into lines restricted by the given width;
// returns number of lines, or 0 if text cannot be split well to fit in this width
size_t split_lines(const char *texx, SplitLines &lines, int width, int fontNumber, size_t max_lines = -1);

} // namespace AGS3

#endif
