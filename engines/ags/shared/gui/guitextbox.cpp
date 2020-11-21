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

#include "font/fonts.h"
#include "gui/guimain.h"
#include "gui/guitextbox.h"
#include "util/stream.h"
#include "util/string_utils.h"

#define GUITEXTBOX_LEGACY_TEXTLEN 200

std::vector<AGS::Common::GUITextBox> guitext;
int numguitext = 0;

namespace AGS
{
namespace Common
{

GUITextBox::GUITextBox()
{
    Font = 0;
    TextColor = 0;
    TextBoxFlags = kTextBox_DefFlags;

    _scEventCount = 1;
    _scEventNames[0] = "Activate";
    _scEventArgs[0] = "GUIControl *control";
}

bool GUITextBox::IsBorderShown() const
{
    return (TextBoxFlags & kTextBox_ShowBorder) != 0;
}

void GUITextBox::Draw(Bitmap *ds)
{
    check_font(&Font);
    color_t text_color = ds->GetCompatibleColor(TextColor);
    color_t draw_color = ds->GetCompatibleColor(TextColor);
    if (IsBorderShown())
    {
        ds->DrawRect(RectWH(X, Y, Width, Height), draw_color);
        if (get_fixed_pixel_size(1) > 1)
        {
            ds->DrawRect(Rect(X + 1, Y + 1, X + Width - get_fixed_pixel_size(1), Y + Height - get_fixed_pixel_size(1)), draw_color);
        }
    }
    DrawTextBoxContents(ds, text_color);
}

void GUITextBox::OnKeyPress(int keycode)
{
    guis_need_update = 1;
    // TODO: use keycode constants
    // backspace, remove character
    if (keycode == 8)
    {
        Text.ClipRight(1);
        return;
    }
    // other key, continue
    if ((keycode >= 128) && (!font_supports_extended_characters(Font)))
        return;
    // return/enter
    if (keycode == 13)
    {
        IsActivated = true;
        return;
    }

    Text.AppendChar(keycode);
    // if the new string is too long, remove the new character
    if (wgettextwidth(Text, Font) > (Width - (6 + get_fixed_pixel_size(5))))
        Text.ClipRight(1);
}

void GUITextBox::SetShowBorder(bool on)
{
    if (on)
        TextBoxFlags |= kTextBox_ShowBorder;
    else
        TextBoxFlags &= ~kTextBox_ShowBorder;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUITextBox::WriteToFile(Stream *out) const
{
    GUIObject::WriteToFile(out);
    StrUtil::WriteString(Text, out);
    out->WriteInt32(Font);
    out->WriteInt32(TextColor);
    out->WriteInt32(TextBoxFlags);
}

void GUITextBox::ReadFromFile(Stream *in, GuiVersion gui_version)
{
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

void GUITextBox::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver)
{
    GUIObject::ReadFromSavegame(in, svg_ver);
    Font = in->ReadInt32();
    TextColor = in->ReadInt32();
    Text = StrUtil::ReadString(in);
    if (svg_ver >= kGuiSvgVersion_350)
        TextBoxFlags = in->ReadInt32();
}

void GUITextBox::WriteToSavegame(Stream *out) const
{
    GUIObject::WriteToSavegame(out);
    out->WriteInt32(Font);
    out->WriteInt32(TextColor);
    StrUtil::WriteString(Text, out);
    out->WriteInt32(TextBoxFlags);
}

} // namespace Common
} // namespace AGS
