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

//=============================================================================
//
// Implementation from acgui.h and acgui.cpp specific to Engine runtime
//
//=============================================================================

#include "ags/shared/ac/game_version.h"
#include "ags/engine/ac/system.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/blender.h"
#include "ags/globals.h"

namespace AGS3 {

extern void wouttext_outline(Shared::Bitmap *ds, int xxp, int yyp, int usingfont, color_t text_color, const char *texx);

using namespace AGS::Shared;

// For engine these are defined in ac.cpp
extern void replace_macro_tokens(const char *, String &);

// For engine these are defined in acfonts.cpp
extern void ensure_text_valid_for_font(char *, int);
//

// in ac_runningame


bool GUIMain::HasAlphaChannel() const {
	if (this->BgImage > 0) {
		// alpha state depends on background image
		return is_sprite_alpha(this->BgImage);
	}
	if (this->BgColor > 0) {
		// not alpha transparent if there is a background color
		return false;
	}
	// transparent background, enable alpha blending
	return _GP(game).GetColorDepth() >= 24 &&
	       // transparent background have alpha channel only since 3.2.0;
	       // "classic" gui rendering mode historically had non-alpha transparent backgrounds
	       // (3.2.0 broke the compatibility, now we restore it)
	       _G(loaded_game_file_version) >= kGameVersion_320 && _GP(game).options[OPT_NEWGUIALPHA] != kGuiAlphaRender_Legacy;
}

//=============================================================================
// Engine-specific implementation split out of acgui.cpp
//=============================================================================

int get_adjusted_spritewidth(int spr) {
	return _GP(spriteset)[spr]->GetWidth();
}

int get_adjusted_spriteheight(int spr) {
	return _GP(spriteset)[spr]->GetHeight();
}

bool is_sprite_alpha(int spr) {
	return ((_GP(game).SpriteInfos[spr].Flags & SPF_ALPHACHANNEL) != 0);
}

void set_eip_guiobj(int eip) {
	_G(eip_guiobj) = eip;
}

int get_eip_guiobj() {
	return _G(eip_guiobj);
}

namespace AGS {
namespace Shared {

bool GUIObject::IsClickable() const {
	return (Flags & kGUICtrl_Clickable) != 0;
}

void GUIObject::NotifyParentChanged() {
	_GP(guis)[ParentId].MarkChanged();
}

void GUILabel::PrepareTextToDraw() {
	replace_macro_tokens((Flags & kGUICtrl_Translated) ? get_translation(Text.GetCStr()) : Text.GetCStr(), _textToDraw);
}

size_t GUILabel::SplitLinesForDrawing(SplitLines &lines) {
	// Use the engine's word wrap tool, to have hebrew-style writing and other features
	return break_up_text_into_lines(_textToDraw.GetCStr(), lines, Width, Font);
}

void GUITextBox::DrawTextBoxContents(Bitmap *ds, color_t text_color) {
	wouttext_outline(ds, X + 1 + get_fixed_pixel_size(1), Y + 1 + get_fixed_pixel_size(1), Font, text_color, Text.GetCStr());
	if (IsGUIEnabled(this)) {
		// draw a cursor
		int draw_at_x = get_text_width(Text.GetCStr(), Font) + X + 3;
		int draw_at_y = Y + 1 + get_font_height(Font);
		ds->DrawRect(Rect(draw_at_x, draw_at_y, draw_at_x + get_fixed_pixel_size(5), draw_at_y + (get_fixed_pixel_size(1) - 1)), text_color);
	}
}

void GUIListBox::DrawItemsFix() {
	// do nothing
}

void GUIListBox::DrawItemsUnfix() {
	// do nothing
}

void GUIListBox::PrepareTextToDraw(const String &text) {
	if (Flags & kGUICtrl_Translated)
		_textToDraw = get_translation(text.GetCStr());
	else
		_textToDraw = text;
}

void GUIButton::PrepareTextToDraw() {
	if (Flags & kGUICtrl_Translated)
		_textToDraw = get_translation(_text.GetCStr());
	else
		_textToDraw = _text;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
