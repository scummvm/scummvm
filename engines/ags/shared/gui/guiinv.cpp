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

#include <math.h>
#include "ac/game_version.h"
#include "gui/guiinv.h"
#include "gui/guimain.h"
#include "util/stream.h"

std::vector<AGS::Common::GUIInvWindow> guiinv;
int numguiinv = 0;

namespace AGS
{
namespace Common
{

GUIInvWindow::GUIInvWindow()
{
    IsMouseOver = false;
    CharId = -1;
    ItemWidth = 40;
    ItemHeight = 22;
    ColCount = 0;
    RowCount = 0;
    TopItem = 0;
    CalculateNumCells();

    _scEventCount = 0;
}

void GUIInvWindow::OnMouseEnter()
{
    IsMouseOver = true;
}

void GUIInvWindow::OnMouseLeave()
{
    IsMouseOver = false;
}

void GUIInvWindow::OnMouseUp()
{
    if (IsMouseOver)
        IsActivated = true;
}

void GUIInvWindow::OnResized()
{
    CalculateNumCells();
}

void GUIInvWindow::WriteToFile(Stream *out) const
{
    GUIObject::WriteToFile(out);
    out->WriteInt32(CharId);
    out->WriteInt32(ItemWidth);
    out->WriteInt32(ItemHeight);
}

void GUIInvWindow::ReadFromFile(Stream *in, GuiVersion gui_version)
{
    GUIObject::ReadFromFile(in, gui_version);
    if (gui_version >= kGuiVersion_unkn_109)
    {
        CharId = in->ReadInt32();
        ItemWidth = in->ReadInt32();
        ItemHeight = in->ReadInt32();
        if (gui_version < kGuiVersion_350)
        { // NOTE: reading into actual variables only for old savegame support
            TopItem = in->ReadInt32();
        }
    }
    else
    {
        CharId = -1;
        ItemWidth = 40;
        ItemHeight = 22;
        TopItem = 0;
    }

    if (loaded_game_file_version >= kGameVersion_270)
    {
        // ensure that some items are visible
        if (ItemWidth > Width)
            ItemWidth = Width;
        if (ItemHeight > Height)
            ItemHeight = Height;
    }

    CalculateNumCells();
}

void GUIInvWindow::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver)
{
    GUIObject::ReadFromSavegame(in, svg_ver);
    ItemWidth = in->ReadInt32();
    ItemHeight = in->ReadInt32();
    CharId = in->ReadInt32();
    TopItem = in->ReadInt32();
    CalculateNumCells();
}

void GUIInvWindow::WriteToSavegame(Stream *out) const
{
    GUIObject::WriteToSavegame(out);
    out->WriteInt32(ItemWidth);
    out->WriteInt32(ItemHeight);
    out->WriteInt32(CharId);
    out->WriteInt32(TopItem);
}

void GUIInvWindow::CalculateNumCells()
{
    if (ItemWidth <= 0 || ItemHeight <= 0)
    {
        ColCount = 0;
        RowCount = 0;
    }
    else if (loaded_game_file_version >= kGameVersion_270)
    {
        ColCount = Width / data_to_game_coord(ItemWidth);
        RowCount = Height / data_to_game_coord(ItemHeight);
    }
    else
    {
        ColCount = floor((float)Width / (float)data_to_game_coord(ItemWidth) + 0.5f);
        RowCount = floor((float)Height / (float)data_to_game_coord(ItemHeight) + 0.5f);
    }
}

} // namespace Common
} // namespace AGS
