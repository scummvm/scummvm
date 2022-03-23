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

#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

GUIListBox::GUIListBox() {
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

int GUIListBox::GetItemAt(int x, int y) const {
	if (RowHeight <= 0 || IsInRightMargin(x))
		return -1;

	int index = y / RowHeight + TopItem;
	if (index < 0 || index >= ItemCount)
		return -1;
	return index;
}

bool GUIListBox::AreArrowsShown() const {
	return (ListBoxFlags & kListBox_ShowArrows) != 0;
}

bool GUIListBox::IsBorderShown() const {
	return (ListBoxFlags & kListBox_ShowBorder) != 0;
}

bool GUIListBox::IsSvgIndex() const {
	return (ListBoxFlags & kListBox_SvgIndex) != 0;
}

bool GUIListBox::IsInRightMargin(int x) const {
	if (x >= (Width - get_fixed_pixel_size(6)) && IsBorderShown() && AreArrowsShown())
		return 1;
	return 0;
}

int GUIListBox::AddItem(const String &text) {
	Items.push_back(text);
	SavedGameIndex.push_back(-1);
	ItemCount++;
	NotifyParentChanged();
	return ItemCount - 1;
}

void GUIListBox::Clear() {
	Items.clear();
	SavedGameIndex.clear();
	ItemCount = 0;
	SelectedItem = 0;
	TopItem = 0;
	NotifyParentChanged();
}

void GUIListBox::Draw(Shared::Bitmap *ds) {
	const int width = Width - 1;
	const int height = Height - 1;
	const int pixel_size = get_fixed_pixel_size(1);

	color_t text_color = ds->GetCompatibleColor(TextColor);
	color_t draw_color = ds->GetCompatibleColor(TextColor);
	if (IsBorderShown()) {
		ds->DrawRect(Rect(X, Y, X + width + (pixel_size - 1), Y + height + (pixel_size - 1)), draw_color);
		if (pixel_size > 1)
			ds->DrawRect(Rect(X + 1, Y + 1, X + width, Y + height), draw_color);
	}

	int right_hand_edge = (X + width) - pixel_size - 1;

	// update the RowHeight and VisibleItemCount
	// FIXME: find a way to update this whenever relevant things change in the engine
	UpdateMetrics();

	// draw the scroll bar in if necessary
	if (ItemCount > VisibleItemCount && IsBorderShown() && AreArrowsShown()) {
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

	// FIXME: cut this out, and let editor add real items for display
	DrawItemsFix();

	for (int item = 0; item < VisibleItemCount; ++item) {
		if (item + TopItem >= ItemCount)
			break;

		int at_y = Y + pixel_size + item * RowHeight;
		if (item + TopItem == SelectedItem) {
			text_color = ds->GetCompatibleColor(SelectedTextColor);
			if (SelectedBgColor > 0) {
				int stretch_to = (X + width) - pixel_size;
				// draw the SelectedItem item bar (if colour not transparent)
				draw_color = ds->GetCompatibleColor(SelectedBgColor);
				if ((VisibleItemCount < ItemCount) && IsBorderShown() && AreArrowsShown())
					stretch_to -= get_fixed_pixel_size(7);

				ds->FillRect(Rect(X + pixel_size, at_y, stretch_to, at_y + RowHeight - pixel_size), draw_color);
			}
		} else
			text_color = ds->GetCompatibleColor(TextColor);

		int item_index = item + TopItem;
		PrepareTextToDraw(Items[item_index]);

		GUI::DrawTextAlignedHor(ds, _textToDraw.GetCStr(), Font, text_color, X + 1 + pixel_size, right_hand_edge, at_y + 1,
		                        (FrameAlignment)TextAlignment);
	}

	DrawItemsUnfix();
}

int GUIListBox::InsertItem(int index, const String &text) {
	if (index < 0 || index > ItemCount)
		return -1;

	Items.insert(Items.begin() + index, text);
	SavedGameIndex.insert(SavedGameIndex.begin() + index, -1);
	if (SelectedItem >= index)
		SelectedItem++;

	ItemCount++;
	NotifyParentChanged();
	return ItemCount - 1;
}

void GUIListBox::RemoveItem(int index) {
	if (index < 0 || index >= ItemCount)
		return;

	Items.erase(Items.begin() + index);
	SavedGameIndex.erase(SavedGameIndex.begin() + index);
	ItemCount--;

	if (SelectedItem > index)
		SelectedItem--;
	if (SelectedItem >= ItemCount)
		SelectedItem = -1;
	NotifyParentChanged();
}

void GUIListBox::SetShowArrows(bool on) {
	if (on)
		ListBoxFlags |= kListBox_ShowArrows;
	else
		ListBoxFlags &= ~kListBox_ShowArrows;
	NotifyParentChanged();
}

void GUIListBox::SetShowBorder(bool on) {
	if (on)
		ListBoxFlags |= kListBox_ShowBorder;
	else
		ListBoxFlags &= ~kListBox_ShowBorder;
	NotifyParentChanged();
}

void GUIListBox::SetSvgIndex(bool on) {
	if (on)
		ListBoxFlags |= kListBox_SvgIndex;
	else
		ListBoxFlags &= ~kListBox_SvgIndex;
}

void GUIListBox::SetFont(int font) {
	Font = font;
	UpdateMetrics();
	NotifyParentChanged();
}

void GUIListBox::SetItemText(int index, const String &text) {
	if (index >= 0 && index < ItemCount) {
		Items[index] = text;
		NotifyParentChanged();
	}
}

bool GUIListBox::OnMouseDown() {
	if (IsInRightMargin(MousePos.X)) {
		int top_item = TopItem;
		if (MousePos.Y < Height / 2 && TopItem > 0)
			top_item = TopItem - 1;
		if (MousePos.Y >= Height / 2 && ItemCount > TopItem + VisibleItemCount)
			top_item = TopItem + 1;
		if (TopItem != top_item) {
			TopItem = top_item;
			NotifyParentChanged();
		}
		return false;
	}

	int sel = GetItemAt(MousePos.X, MousePos.Y);
	if (sel < 0)
		return false;
	if (sel != SelectedItem) {
		SelectedItem = sel;
		NotifyParentChanged();
	}
	IsActivated = true;
	return false;
}

void GUIListBox::OnMouseMove(int x_, int y_) {
	MousePos.X = x_ - X;
	MousePos.Y = y_ - Y;
}

void GUIListBox::OnResized() {
	UpdateMetrics();
	NotifyParentChanged();
}

void GUIListBox::UpdateMetrics() {
	RowHeight = get_font_height(Font) + get_fixed_pixel_size(2);
	VisibleItemCount = Height / RowHeight;
	if (ItemCount <= VisibleItemCount)
		TopItem = 0; // reset scroll if all items are visible
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUIListBox::WriteToFile(Stream *out) const {
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

void GUIListBox::ReadFromFile(Stream *in, GuiVersion gui_version) {
	Clear();

	GUIObject::ReadFromFile(in, gui_version);
	ItemCount = in->ReadInt32();
	if (gui_version < kGuiVersion_350) { // NOTE: reading into actual variables only for old savegame support
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

	if (gui_version >= kGuiVersion_272b) {
		if (gui_version < kGuiVersion_350) {
			TextAlignment = ConvertLegacyGUIAlignment((LegacyGUIAlignment)in->ReadInt32());
			in->ReadInt32(); // reserved1
		} else {
			TextAlignment = (HorAlignment)in->ReadInt32();
		}
	} else {
		TextAlignment = kHAlignLeft;
	}

	if (gui_version >= kGuiVersion_unkn_107) {
		SelectedBgColor = in->ReadInt32();
	} else {
		SelectedBgColor = TextColor;
		if (SelectedBgColor == 0)
			SelectedBgColor = 16;
	}

	// NOTE: we leave items in game data format as a potential support for defining
	// ListBox contents at design-time, although Editor does not support it as of 3.5.0.
	Items.resize(ItemCount);
	SavedGameIndex.resize(ItemCount, -1);
	for (int i = 0; i < ItemCount; ++i) {
		Items[i].Read(in);
	}

	if (gui_version >= kGuiVersion_272d && gui_version < kGuiVersion_350 &&
	        (ListBoxFlags & kListBox_SvgIndex)) { // NOTE: reading into actual variables only for old savegame support
		for (int i = 0; i < ItemCount; ++i)
			SavedGameIndex[i] = in->ReadInt16();
	}

	if (TextColor == 0)
		TextColor = 16;
}

void GUIListBox::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	// Properties
	ListBoxFlags = in->ReadInt32();
	Font = in->ReadInt32();
	if (svg_ver < kGuiSvgVersion_350) {
		// reverse particular flags from older format
		ListBoxFlags ^= kListBox_OldFmtXorMask;
	} else {
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

void GUIListBox::WriteToSavegame(Stream *out) const {
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

} // namespace Shared
} // namespace AGS
} // namespace AGS3
