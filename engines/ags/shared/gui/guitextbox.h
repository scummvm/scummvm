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

#ifndef AGS_SHARED_GUI_GUITEXTBOX_H
#define AGS_SHARED_GUI_GUITEXTBOX_H

#include "ags/lib/std/vector.h"
#include "ags/shared/gui/guiobject.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class GUITextBox : public GUIObject {
public:
	GUITextBox();

	bool IsBorderShown() const;

	// Operations
	void Draw(Bitmap *ds) override;
	void SetShowBorder(bool on);

	// Events
	void OnKeyPress(int keycode) override;

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

	void DrawTextBoxContents(Bitmap *ds, color_t text_color);
};

} // namespace Shared
} // namespace AGS

extern std::vector<AGS::Shared::GUITextBox> guitext;
extern int numguitext;

} // namespace AGS3

#endif
