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

#ifndef AGS_SHARED_GUI_GUIINV_H
#define AGS_SHARED_GUI_GUIINV_H

#include "ags/lib/std/vector.h"
#include "ags/shared/gui/guiobject.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class GUIInvWindow : public GUIObject {
public:
	GUIInvWindow();

	// This function has distinct implementations in Engine and Editor
	int GetCharacterId() const;

	// Operations
	// This function has distinct implementations in Engine and Editor
	void Draw(Bitmap *ds) override;

	// Events
	void OnMouseEnter() override;
	void OnMouseLeave() override;
	void OnMouseUp() override;
	void OnResized() override;

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Stream *out) const override;

	// TODO: these members are currently public; hide them later
public:
	bool    IsMouseOver;
	int32_t CharId; // whose inventory (-1 = current player)
	int32_t ItemWidth;
	int32_t ItemHeight;
	int32_t ColCount;
	int32_t RowCount;
	int32_t TopItem;

private:
	void CalculateNumCells();
};

} // namespace Shared
} // namespace AGS

extern std::vector<AGS::Shared::GUIInvWindow> guiinv;
extern int numguiinv;

} // namespace AGS3

#endif
