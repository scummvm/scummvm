//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AC_FONT_H
#define __AC_FONT_H

#include <vector>
#include "core/types.h"
#include "util/string.h"

// TODO: we need to make some kind of TextManager class of this module

namespace AGS { namespace Common { class Bitmap; } }
using namespace AGS;

class IAGSFontRenderer;
class IAGSFontRenderer2;
struct FontInfo;
struct FontRenderParams;

void init_font_renderer();
void shutdown_font_renderer();
void adjust_y_coordinate_for_text(int* ypos, size_t fontnum);
IAGSFontRenderer* font_replace_renderer(size_t fontNumber, IAGSFontRenderer* renderer);
bool font_first_renderer_loaded();
bool is_font_loaded(size_t fontNumber);
bool is_bitmap_font(size_t fontNumber);
bool font_supports_extended_characters(size_t fontNumber);
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
int wgettextwidth(const char *texx, size_t fontNumber);
// Calculates actual height of a line of text
int wgettextheight(const char *text, size_t fontNumber);
// Get font's height (maximal height of any line of text printed with this font)
int getfontheight(size_t fontNumber);
// Get font's line spacing
int getfontlinespacing(size_t fontNumber);
// Get is font is meant to use default line spacing
bool use_default_linespacing(size_t fontNumber);
// Get font's outline type
int  get_font_outline(size_t font_number);
// Get font's automatic outline thickness (if set)
int  get_font_outline_thickness(size_t font_number);
// Set font's outline type
void set_font_outline(size_t font_number, int outline_type);
// Outputs a single line of text on the defined position on bitmap, using defined font, color and parameters
int getfontlinespacing(size_t fontNumber);
// Print text on a surface using a given font
void wouttextxy(Common::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, const char *texx);
// Assigns FontInfo to the font
void set_fontinfo(size_t fontNumber, const FontInfo &finfo);
// Loads a font from disk
bool wloadfont_size(size_t fontNumber, const FontInfo &font_info);
void wgtprintf(Common::Bitmap *ds, int xxx, int yyy, size_t fontNumber, color_t text_color, char *fmt, ...);
// Free particular font's data
void wfreefont(size_t fontNumber);
// Free all fonts data
void free_all_fonts();

// SplitLines class represents a list of lines and is meant to reduce
// subsequent memory (de)allocations if used often during game loops
// and drawing. For that reason it is not equivalent to std::vector,
// but keeps constructed String buffers intact for most time.
// TODO: implement proper strings pool.
class SplitLines
{
public:
    inline size_t Count() const { return _count; }
    inline const Common::String &operator[](size_t i) const { return _pool[i]; }
    inline Common::String &operator[](size_t i) { return _pool[i]; }
    inline void Clear() { _pool.clear(); _count = 0; }
    inline void Reset() { _count = 0; }
    inline void Add(const char *cstr)
    {
        if (_pool.size() == _count) _pool.resize(_count + 1);
        _pool[_count++].SetString(cstr);
    }

    // An auxiliary line processing buffer
    std::vector<char> LineBuf;

private:
    std::vector<Common::String> _pool;
    size_t _count; // actual number of lines in use
};

// Break up the text into lines restricted by the given width;
// returns number of lines, or 0 if text cannot be split well to fit in this width
size_t split_lines(const char *texx, SplitLines &lines, int width, int fontNumber, size_t max_lines = -1);

namespace AGS { namespace Common { extern SplitLines Lines; } }

#endif // __AC_FONT_H
