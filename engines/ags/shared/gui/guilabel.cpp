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

#include "ac/game_version.h"
#include "font/fonts.h"
#include "gui/guilabel.h"
#include "gui/guimain.h"
#include "util/stream.h"
#include "util/string_utils.h"

std::vector<AGS::Common::GUILabel> guilabels;
int numguilabels = 0;

#define GUILABEL_TEXTLENGTH_PRE272 200

namespace AGS
{
namespace Common
{

GUILabel::GUILabel()
{
    Font = 0;
    TextColor = 0;
    TextAlignment = kHAlignLeft;

    _scEventCount = 0;
}

String GUILabel::GetText() const
{
    return Text;
}

void GUILabel::Draw(Common::Bitmap *ds)
{
    check_font(&Font);

    // TODO: need to find a way to cache text prior to drawing;
    // but that will require to update all gui controls when translation is changed in game
    PrepareTextToDraw();
    if (SplitLinesForDrawing(Lines) == 0)
        return;

    color_t text_color = ds->GetCompatibleColor(TextColor);
    const int linespacing = getfontlinespacing(Font) + 1;
    // < 2.72 labels did not limit vertical size of text
    const bool limit_by_label_frame = loaded_game_file_version >= kGameVersion_272;
    int at_y = Y;
    for (size_t i = 0;
        i < Lines.Count() && (!limit_by_label_frame || at_y <= Y + Height);
        ++i, at_y += linespacing)
    {
        GUI::DrawTextAlignedHor(ds, Lines[i], Font, text_color, X, X + Width - 1, at_y,
            (FrameAlignment)TextAlignment);
    }
}

void GUILabel::SetText(const String &text)
{
    Text = text;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUILabel::WriteToFile(Stream *out) const
{
    GUIObject::WriteToFile(out);
    StrUtil::WriteString(Text, out);
    out->WriteInt32(Font);
    out->WriteInt32(TextColor);
    out->WriteInt32(TextAlignment);
}

void GUILabel::ReadFromFile(Stream *in, GuiVersion gui_version)
{
    GUIObject::ReadFromFile(in, gui_version);

    if (gui_version < kGuiVersion_272c)
        Text.ReadCount(in, GUILABEL_TEXTLENGTH_PRE272);
    else
        Text = StrUtil::ReadString(in);

    Font = in->ReadInt32();
    TextColor = in->ReadInt32();
    if (gui_version < kGuiVersion_350)
        TextAlignment = ConvertLegacyGUIAlignment((LegacyGUIAlignment)in->ReadInt32());
    else
        TextAlignment = (HorAlignment)in->ReadInt32();

    if (TextColor == 0)
        TextColor = 16;
    // All labels are translated at the moment
    Flags |= kGUICtrl_Translated;
}

void GUILabel::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver)
{
    GUIObject::ReadFromSavegame(in, svg_ver);
    Font = in->ReadInt32();
    TextColor = in->ReadInt32();
    Text = StrUtil::ReadString(in);
    if (svg_ver >= kGuiSvgVersion_350)
        TextAlignment = (HorAlignment)in->ReadInt32();
}

void GUILabel::WriteToSavegame(Stream *out) const
{
    GUIObject::WriteToSavegame(out);
    out->WriteInt32(Font);
    out->WriteInt32(TextColor);
    StrUtil::WriteString(Text, out);
    out->WriteInt32(TextAlignment);
}

} // namespace Common
} // namespace AGS
