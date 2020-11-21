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
#include "gui/guilistbox.h"
#include "gui/guimain.h"
#include "util/stream.h"
#include "util/string_utils.h"

std::vector<AGS::Common::GUIListBox> guilist;
int numguilist = 0;

namespace AGS
{
namespace Common
{

GUIListBox::GUIListBox()
{
    ItemCount = 0;
    SelectedItem = 0;
    TopItem = 0;
    RowHeight = 0;
    VisibleItemCount = 0;
    Font = 0;
    TextColor = 0;
    SelectedTextColor = 7;
    ListBoxFlags = kListBox_DefFlags;
    SelectedBgColor = 16;
    TextAlignment = kHAlignLeft;

    _scEventCount = 1;
    _scEventNames[0] = "SelectionChanged";
    _scEventArgs[0] = "GUIControl *control";
}

int GUIListBox::GetItemAt(int x, int y) const
{
    if (RowHeight <= 0 || IsInRightMargin(x))
        return -1;

    int index = y / RowHeight + TopItem;
    if (index < 0 || index >= ItemCount)
        return -1;
    return index;
}

bool GUIListBox::AreArrowsShown() const
{
    return (ListBoxFlags & kListBox_ShowArrows) != 0;
}

bool GUIListBox::IsBorderShown() const
{
    return (ListBoxFlags & kListBox_ShowBorder) != 0;
}

bool GUIListBox::IsSvgIndex() const
{
    return (ListBoxFlags & kListBox_SvgIndex) != 0;
}

bool GUIListBox::IsInRightMargin(int x) const
{
    if (x >= (Width - get_fixed_pixel_size(6)) && IsBorderShown() && AreArrowsShown())
        return 1;
    return 0;
}

int GUIListBox::AddItem(const String &text)
{
    guis_need_update = 1;
    Items.push_back(text);
    SavedGameIndex.push_back(-1);
    ItemCount++;
    return ItemCount - 1;
}

void GUIListBox::Clear()
{
    Items.clear();
    SavedGameIndex.clear();
    ItemCount = 0;
    SelectedItem = 0;
    TopItem = 0;
    guis_need_update = 1;
}

void GUIListBox::Draw(Common::Bitmap *ds)
{
    const int width  = Width - 1;
    const int height = Height - 1;
    const int pixel_size = get_fixed_pixel_size(1);

    check_font(&Font);
    color_t text_color = ds->GetCompatibleColor(TextColor);
    color_t draw_color = ds->GetCompatibleColor(TextColor);
    if (IsBorderShown())
    {
        ds->DrawRect(Rect(X, Y, X + width + (pixel_size - 1), Y + height + (pixel_size - 1)), draw_color);
        if (pixel_size > 1)
            ds->DrawRect(Rect(X + 1, Y + 1, X + width, Y + height), draw_color);
    }

    int right_hand_edge = (X + width) - pixel_size - 1;

    // use SetFont to update the RowHeight and VisibleItemCount
    SetFont(Font);

    // draw the scroll bar in if necessary
    if (ItemCount > VisibleItemCount && IsBorderShown() && AreArrowsShown())
    {
        int xstrt, ystrt;
        ds->DrawRect(Rect(X + width - get_fixed_pixel_size(7), Y, (X + (pixel_size - 1) + width) - get_fixed_pixel_size(7), Y + height), draw_color);
        ds->DrawRect(Rect(X + width - get_fixed_pixel_size(7), Y + height / 2, X + width, Y + height / 2 + (pixel_size - 1)), draw_color);

        xstrt = (X + width - get_fixed_pixel_size(6)) + (pixel_size - 1);
        ystrt = (Y + height - 3) - get_fixed_pixel_size(5);

        draw_color = ds->GetCompatibleColor(TextColor);
        ds->DrawTriangle(Triangle(xstrt, ystrt, xstrt + get_fixed_pixel_size(4), ystrt, 
                 xstrt + get_fixed_pixel_size(2),
                 ystrt + get_fixed_pixel_size(5)), draw_color);

        ystrt = Y + 3;
        ds->DrawTriangle(Triangle(xstrt, ystrt + get_fixed_pixel_size(5), 
                 xstrt + get_fixed_pixel_size(4), 
                 ystrt + get_fixed_pixel_size(5),
                 xstrt + get_fixed_pixel_size(2), ystrt), draw_color);

        right_hand_edge -= get_fixed_pixel_size(7);
    }

    DrawItemsFix();

    for (int item = 0; item < VisibleItemCount; ++item)
    {
        if (item + TopItem >= ItemCount)
            break;

        int at_y = Y + pixel_size + item * RowHeight;
        if (item + TopItem == SelectedItem)
        {
            text_color = ds->GetCompatibleColor(SelectedTextColor);
            if (SelectedBgColor > 0)
            {
                int stretch_to = (X + width) - pixel_size;
                // draw the SelectedItem item bar (if colour not transparent)
                draw_color = ds->GetCompatibleColor(SelectedBgColor);
                if ((VisibleItemCount < ItemCount) && IsBorderShown() && AreArrowsShown())
                    stretch_to -= get_fixed_pixel_size(7);

                ds->FillRect(Rect(X + pixel_size, at_y, stretch_to, at_y + RowHeight - pixel_size), draw_color);
            }
        }
        else
            text_color = ds->GetCompatibleColor(TextColor);

        int item_index = item + TopItem;
        PrepareTextToDraw(Items[item_index]);

        GUI::DrawTextAlignedHor(ds, _textToDraw, Font, text_color, X + 1 + pixel_size, right_hand_edge, at_y + 1,
            (FrameAlignment)TextAlignment);
    }

    DrawItemsUnfix();
}

int GUIListBox::InsertItem(int index, const String &text)
{
    if (index < 0 || index > ItemCount)
        return -1;

    Items.insert(Items.begin() + index, text);
    SavedGameIndex.insert(SavedGameIndex.begin() + index, -1);
    if (SelectedItem >= index)
        SelectedItem++;

    ItemCount++;
    guis_need_update = 1;
    return ItemCount - 1;
}

void GUIListBox::RemoveItem(int index)
{
    if (index < 0 || index >= ItemCount)
        return;

    Items.erase(Items.begin() + index);
    SavedGameIndex.erase(SavedGameIndex.begin() + index);
    ItemCount--;

    if (SelectedItem > index)
        SelectedItem--;
    if (SelectedItem >= ItemCount)
        SelectedItem = -1;
    guis_need_update = 1;
}

void GUIListBox::SetShowArrows(bool on)
{
    if (on)
        ListBoxFlags |= kListBox_ShowArrows;
    else
        ListBoxFlags &= ~kListBox_ShowArrows;
}

void GUIListBox::SetShowBorder(bool on)
{
    if (on)
        ListBoxFlags |= kListBox_ShowBorder;
    else
        ListBoxFlags &= ~kListBox_ShowBorder;
}

void GUIListBox::SetSvgIndex(bool on)
{
    if (on)
        ListBoxFlags |= kListBox_SvgIndex;
    else
        ListBoxFlags &= ~kListBox_SvgIndex;
}

void GUIListBox::SetFont(int font)
{
    Font = font;
    RowHeight = getfontheight(Font) + get_fixed_pixel_size(2);
    VisibleItemCount = Height / RowHeight;
}

void GUIListBox::SetItemText(int index, const String &text)
{
    if (index >= 0 && index < ItemCount)
    {
        guis_need_update = 1;
        Items[index] = text;
    }
}

bool GUIListBox::OnMouseDown()
{
    if (IsInRightMargin(MousePos.X))
    {
        if (MousePos.Y < Height / 2 && TopItem > 0)
            TopItem--;
        if (MousePos.Y >= Height / 2 && ItemCount > TopItem + VisibleItemCount)
            TopItem++;
        return false;
    }

    int sel = GetItemAt(MousePos.X, MousePos.Y);
    if (sel < 0)
        return false;
    SelectedItem = sel;
    IsActivated = true;
    return false;
}

void GUIListBox::OnMouseMove(int x_, int y_)
{
    MousePos.X = x_ - X;
    MousePos.Y = y_ - Y;
}

void GUIListBox::OnResized() 
{
    if (RowHeight == 0)
    {
        check_font(&Font);
        SetFont(Font);
    }
    if (RowHeight > 0)
        VisibleItemCount = Height / RowHeight;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUIListBox::WriteToFile(Stream *out) const
{
    GUIObject::WriteToFile(out);
    out->WriteInt32(ItemCount);
    out->WriteInt32(Font);
    out->WriteInt32(TextColor);
    out->WriteInt32(SelectedTextColor);
    out->WriteInt32(ListBoxFlags);
    out->WriteInt32(TextAlignment);
    out->WriteInt32(SelectedBgColor);
    for (int i = 0; i < ItemCount; ++i)
        Items[i].Write(out);
}

void GUIListBox::ReadFromFile(Stream *in, GuiVersion gui_version)
{
    Clear();

    GUIObject::ReadFromFile(in, gui_version);
    ItemCount = in->ReadInt32();
    if (gui_version < kGuiVersion_350)
    { // NOTE: reading into actual variables only for old savegame support
        SelectedItem = in->ReadInt32();
        TopItem = in->ReadInt32();
        MousePos.X = in->ReadInt32();
        MousePos.Y = in->ReadInt32();
        RowHeight = in->ReadInt32();
        VisibleItemCount = in->ReadInt32();
    }
    Font = in->ReadInt32();
    TextColor = in->ReadInt32();
    SelectedTextColor = in->ReadInt32();
    ListBoxFlags = in->ReadInt32();
    // reverse particular flags from older format
    if (gui_version < kGuiVersion_350)
        ListBoxFlags ^= kListBox_OldFmtXorMask;

    if (gui_version >= kGuiVersion_272b)
    {
        if (gui_version < kGuiVersion_350)
        {
            TextAlignment = ConvertLegacyGUIAlignment((LegacyGUIAlignment)in->ReadInt32());
            in->ReadInt32(); // reserved1
        }
        else
        {
            TextAlignment = (HorAlignment)in->ReadInt32();
        }
    }
    else
    {
        TextAlignment = kHAlignLeft;
    }

    if (gui_version >= kGuiVersion_unkn_107)
    {
        SelectedBgColor = in->ReadInt32();
    }
    else
    {
        SelectedBgColor = TextColor;
        if (SelectedBgColor == 0)
            SelectedBgColor = 16;
    }

    // NOTE: we leave items in game data format as a potential support for defining
    // ListBox contents at design-time, although Editor does not support it as of 3.5.0.
    Items.resize(ItemCount);
    SavedGameIndex.resize(ItemCount, -1);
    for (int i = 0; i < ItemCount; ++i)
    {
        Items[i].Read(in);
    }

    if (gui_version >= kGuiVersion_272d && gui_version < kGuiVersion_350 &&
        (ListBoxFlags & kListBox_SvgIndex))
    { // NOTE: reading into actual variables only for old savegame support
        for (int i = 0; i < ItemCount; ++i)
            SavedGameIndex[i] = in->ReadInt16();
    }

    if (TextColor == 0)
        TextColor = 16;
}

void GUIListBox::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver)
{
    GUIObject::ReadFromSavegame(in, svg_ver);
    // Properties
    ListBoxFlags = in->ReadInt32();
    Font = in->ReadInt32();
    if (svg_ver < kGuiSvgVersion_350)
    {
        // reverse particular flags from older format
        ListBoxFlags ^= kListBox_OldFmtXorMask;
    }
    else
    {
        SelectedBgColor = in->ReadInt32();
        SelectedTextColor = in->ReadInt32();
        TextAlignment = (HorAlignment)in->ReadInt32();
        TextColor = in->ReadInt32();
    }

    // Items
    ItemCount = in->ReadInt32();
    Items.resize(ItemCount);
    SavedGameIndex.resize(ItemCount);
    for (int i = 0; i < ItemCount; ++i)
        Items[i] = StrUtil::ReadString(in);
    // TODO: investigate this, it might be unreasonable to save and read
    // savegame index like that because list of savegames may easily change
    // in between writing and restoring the game. Perhaps clearing and forcing
    // this list to update on load somehow may make more sense.
    if (ListBoxFlags & kListBox_SvgIndex)
        for (int i = 0; i < ItemCount; ++i)
            SavedGameIndex[i] = in->ReadInt16();
    TopItem = in->ReadInt32();
    SelectedItem = in->ReadInt32();
}

void GUIListBox::WriteToSavegame(Stream *out) const
{
    GUIObject::WriteToSavegame(out);
    // Properties
    out->WriteInt32(ListBoxFlags);
    out->WriteInt32(Font);
    out->WriteInt32(SelectedBgColor);
    out->WriteInt32(SelectedTextColor);
    out->WriteInt32(TextAlignment);
    out->WriteInt32(TextColor);

    // Items
    out->WriteInt32(ItemCount);
    for (int i = 0; i < ItemCount; ++i)
        StrUtil::WriteString(Items[i], out);
    if (ListBoxFlags & kListBox_SvgIndex)
        for (int i = 0; i < ItemCount; ++i)
            out->WriteInt16(SavedGameIndex[i]);
    out->WriteInt32(TopItem);
    out->WriteInt32(SelectedItem);
}

} // namespace Common
} // namespace AGS
