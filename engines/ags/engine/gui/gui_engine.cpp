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
#include "ags/shared/gui/gui_inv.h"
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
	return _GP(game).SpriteInfos[spr].Width;
}

int get_adjusted_spriteheight(int spr) {
	return _GP(game).SpriteInfos[spr].Height;
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

String GUI::ApplyTextDirection(const String &text) {
	if (_GP(game).options[OPT_RIGHTLEFTWRITE] == 0)
		return text;
	String res_text = text;
	(get_uformat() == U_UTF8) ? res_text.ReverseUTF8() : res_text.Reverse();
	return res_text;
}

String GUI::TransformTextForDrawing(const String &text, bool translate, bool apply_direction) {
	String res_text = translate ? String(get_translation(text.GetCStr())) : text;
	if (translate && apply_direction)
		res_text = ApplyTextDirection(res_text);
	return res_text;
}

size_t GUI::SplitLinesForDrawing(const char *text, bool is_translated, SplitLines &lines, int font, int width, size_t max_lines) {
	// Use the engine's word wrap tool, to have RTL writing and other features
	return break_up_text_into_lines(text, is_translated, lines, width, font);
}

void GUIObject::MarkChanged() {
	_hasChanged = true;
	_GP(guis)[ParentId].MarkControlChanged();
}

void GUIObject::MarkParentChanged() {
	_GP(guis)[ParentId].MarkControlChanged();
}

void GUIObject::MarkPositionChanged(bool self_changed) {
	_hasChanged |= self_changed;
	_GP(guis)[ParentId].NotifyControlPosition();
}

void GUIObject::MarkStateChanged(bool self_changed, bool parent_changed) {
	_hasChanged |= self_changed;
	_GP(guis)[ParentId].NotifyControlState(Id, self_changed | parent_changed);
}

void GUIObject::ClearChanged() {
	_hasChanged = false;
}

int GUILabel::PrepareTextToDraw() {
	const bool is_translated = (Flags & kGUICtrl_Translated) != 0;
	replace_macro_tokens(is_translated ? get_translation(Text.GetCStr()) : Text.GetCStr(), _textToDraw);
	return GUI::SplitLinesForDrawing(_textToDraw.GetCStr(), is_translated, _GP(Lines), Font, _width);
}

void GUITextBox::DrawTextBoxContents(Bitmap *ds, int x, int y, color_t text_color) {
	_textToDraw = Text;
	bool reverse = false;
	// Text boxes input is never "translated" in regular sense,
	// but they use this flag to apply text direction
	if ((_G(loaded_game_file_version) >= kGameVersion_361) && ((Flags & kGUICtrl_Translated) != 0)) {
		_textToDraw = GUI::ApplyTextDirection(Text);
		reverse = _GP(game).options[OPT_RIGHTLEFTWRITE] != 0;
	}

	Line tpos = GUI::CalcTextPositionHor(_textToDraw.GetCStr(), Font,
										 x + 1 + get_fixed_pixel_size(1), x + _width - 1, y + 1 + get_fixed_pixel_size(1),
										 reverse ? kAlignTopRight : kAlignTopLeft);
	wouttext_outline(ds, tpos.X1, tpos.Y1, Font, text_color, _textToDraw.GetCStr());

	if (IsGUIEnabled(this)) {
		// draw a cursor
		const int cursor_width = get_fixed_pixel_size(5);
		int draw_at_x = reverse ? tpos.X1 - 3 - cursor_width : tpos.X2 + 3;
		int draw_at_y = y + 1 + get_font_height(Font);
		ds->DrawRect(Rect(draw_at_x, draw_at_y, draw_at_x + cursor_width, draw_at_y + (get_fixed_pixel_size(1) - 1)), text_color);
	}
}

void GUIListBox::PrepareTextToDraw(const String &text) {
	_textToDraw = GUI::TransformTextForDrawing(text, (Flags & kGUICtrl_Translated) != 0, (_G(loaded_game_file_version) >= kGameVersion_361));
}

void GUIButton::PrepareTextToDraw() {
	_textToDraw = GUI::TransformTextForDrawing(_text, (Flags & kGUICtrl_Translated) != 0, (_G(loaded_game_file_version) >= kGameVersion_361));
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
