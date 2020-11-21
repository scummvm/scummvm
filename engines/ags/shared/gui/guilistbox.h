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

#ifndef AGS_SHARED_GUI_GUILISTBOX_H
#define AGS_SHARED_GUI_GUILISTBOX_H

#include <vector>
#include "gui/guiobject.h"
#include "util/string.h"

namespace AGS {
namespace Common {

class GUIListBox : public GUIObject {
public:
	GUIListBox();

	bool AreArrowsShown() const;
	bool IsBorderShown() const;
	bool IsSvgIndex() const;
	bool IsInRightMargin(int x) const;
	int  GetItemAt(int x, int y) const;

	// Operations
	int  AddItem(const String &text);
	void Clear();
	void Draw(Bitmap *ds) override;
	int  InsertItem(int index, const String &text);
	void RemoveItem(int index);
	void SetShowArrows(bool on);
	void SetShowBorder(bool on);
	void SetSvgIndex(bool on); // TODO: work around this
	void SetFont(int font);
	void SetItemText(int index, const String &textt);

	// Events
	bool OnMouseDown() override;
	void OnMouseMove(int x, int y) override;
	void OnResized() override;

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Common::Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Common::Stream *out) const override;

// TODO: these members are currently public; hide them later
public:
	int32_t               Font;
	color_t               TextColor;
	HorAlignment          TextAlignment;
	color_t               SelectedBgColor;
	color_t               SelectedTextColor;
	int32_t               RowHeight;
	int32_t               VisibleItemCount;

	std::vector<String>   Items;
	std::vector<int16_t>  SavedGameIndex;
	int32_t               SelectedItem;
	int32_t               TopItem;
	Point                 MousePos;

	// TODO: remove these later
	int32_t               ItemCount;

private:
	int32_t               ListBoxFlags;

	// A temporary solution for special drawing in the Editor
	void DrawItemsFix();
	void DrawItemsUnfix();
	void PrepareTextToDraw(const String &text);

	// prepared text buffer/cache
	String _textToDraw;
};

} // namespace Common
} // namespace AGS

extern std::vector<AGS::Common::GUIListBox> guilist;
extern int numguilist;

#endif
