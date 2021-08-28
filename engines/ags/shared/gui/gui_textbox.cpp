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

#include "ags/shared/ac/keycode.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

#define GUITEXTBOX_LEGACY_TEXTLEN 200

namespace AGS {
namespace Shared {

GUITextBox::GUITextBox() {
	Font = 0;
	TextColor = 0;
	TextBoxFlags = kTextBox_DefFlags;

	_scEventCount = 1;
	_scEventNames[0] = "Activate";
	_scEventArgs[0] = "GUIControl *control";
}

bool GUITextBox::IsBorderShown() const {
	return (TextBoxFlags & kTextBox_ShowBorder) != 0;
}

void GUITextBox::Draw(Bitmap *ds) {
	color_t text_color = ds->GetCompatibleColor(TextColor);
	color_t draw_color = ds->GetCompatibleColor(TextColor);
	if (IsBorderShown()) {
		ds->DrawRect(RectWH(X, Y, Width, Height), draw_color);
		if (get_fixed_pixel_size(1) > 1) {
			ds->DrawRect(Rect(X + 1, Y + 1, X + Width - get_fixed_pixel_size(1), Y + Height - get_fixed_pixel_size(1)), draw_color);
		}
	}
	DrawTextBoxContents(ds, text_color);
}

// TODO: a shared utility function
static void Backspace(String &text) {
	if (get_uformat() == U_UTF8) {// Find where the last utf8 char begins
		const char *ptr_end = text.GetCStr() + text.GetLength();
		const char *ptr = ptr_end - 1;
		for (; ptr > text.GetCStr() && ((*ptr & 0xC0) == 0x80); --ptr);
		text.ClipRight(ptr_end - ptr);
	} else {
		text.ClipRight(1);
	}
}

void GUITextBox::OnKeyPress(const KeyInput &ki) {
	eAGSKeyCode keycode = ki.Key;

	// other key, continue
	if ((keycode >= 128) && (!font_supports_extended_characters(Font)))
		return;
	// return/enter
	if (keycode == eAGSKeyCodeReturn) {
		IsActivated = true;
		return;
	}

	NotifyParentChanged();
	// backspace, remove character
	if (keycode == eAGSKeyCodeBackspace) {
		Backspace(Text);
		return;
	}

	Text.AppendChar(keycode);
	// if the new string is too long, remove the new character
	if (wgettextwidth(Text.GetCStr(), Font) > (Width - (6 + get_fixed_pixel_size(5))))
		Backspace(Text);
}

void GUITextBox::SetShowBorder(bool on) {
	if (on)
		TextBoxFlags |= kTextBox_ShowBorder;
	else
		TextBoxFlags &= ~kTextBox_ShowBorder;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUITextBox::WriteToFile(Stream *out) const {
	GUIObject::WriteToFile(out);
	StrUtil::WriteString(Text, out);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	out->WriteInt32(TextBoxFlags);
}

void GUITextBox::ReadFromFile(Stream *in, GuiVersion gui_version) {
	GUIObject::ReadFromFile(in, gui_version);
	if (gui_version < kGuiVersion_350)
		Text.ReadCount(in, GUITEXTBOX_LEGACY_TEXTLEN);
	else
		Text = StrUtil::ReadString(in);
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	TextBoxFlags = in->ReadInt32();
	// reverse particular flags from older format
	if (gui_version < kGuiVersion_350)
		TextBoxFlags ^= kTextBox_OldFmtXorMask;

	if (TextColor == 0)
		TextColor = 16;
}

void GUITextBox::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	Text = StrUtil::ReadString(in);
	if (svg_ver >= kGuiSvgVersion_350)
		TextBoxFlags = in->ReadInt32();
}

void GUITextBox::WriteToSavegame(Stream *out) const {
	GUIObject::WriteToSavegame(out);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	StrUtil::WriteString(Text, out);
	out->WriteInt32(TextBoxFlags);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
