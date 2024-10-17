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

#ifndef AGS_SHARED_GUI_GUI_TEXTBOX_H
#define AGS_SHARED_GUI_GUI_TEXTBOX_H

#include "common/std/vector.h"
#include "ags/shared/gui/gui_object.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class GUITextBox : public GUIObject {
public:
	GUITextBox();

	bool HasAlphaChannel() const override;
	bool IsBorderShown() const;

	// Operations
	Rect CalcGraphicRect(bool clipped) override;
	void Draw(Bitmap *ds, int x = 0, int y = 0) override;
	void SetShowBorder(bool on);

	// Events
	void OnKeyPress(const KeyInput &ki) override;

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Stream *out) const override;

	// TODO: these members are currently public; hide them later
public:
	int32_t Font;
	String  Text;
	color_t TextColor;

private:
	int32_t TextBoxFlags;
	String  _textToDraw;

	void DrawTextBoxContents(Bitmap *ds, int x, int y, color_t text_color);
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
