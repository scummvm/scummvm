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

//include <math.h>
#include "ags/shared/ac/game_version.h"
#include "ags/shared/gui/guiinv.h"
#include "ags/shared/gui/guimain.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

std::vector<AGS::Shared::GUIInvWindow> guiinv;
int numguiinv = 0;

namespace AGS {
namespace Shared {

GUIInvWindow::GUIInvWindow() {
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

void GUIInvWindow::OnMouseEnter() {
	IsMouseOver = true;
}

void GUIInvWindow::OnMouseLeave() {
	IsMouseOver = false;
}

void GUIInvWindow::OnMouseUp() {
	if (IsMouseOver)
		IsActivated = true;
}

void GUIInvWindow::OnResized() {
	CalculateNumCells();
}

void GUIInvWindow::WriteToFile(Stream *out) const {
	GUIObject::WriteToFile(out);
	out->WriteInt32(CharId);
	out->WriteInt32(ItemWidth);
	out->WriteInt32(ItemHeight);
}

void GUIInvWindow::ReadFromFile(Stream *in, GuiVersion gui_version) {
	GUIObject::ReadFromFile(in, gui_version);
	if (gui_version >= kGuiVersion_unkn_109) {
		CharId = in->ReadInt32();
		ItemWidth = in->ReadInt32();
		ItemHeight = in->ReadInt32();
		if (gui_version < kGuiVersion_350) {
			// NOTE: reading into actual variables only for old savegame support
			TopItem = in->ReadInt32();
		}
	} else {
		CharId = -1;
		ItemWidth = 40;
		ItemHeight = 22;
		TopItem = 0;
	}

	if (loaded_game_file_version >= kGameVersion_270) {
		// ensure that some items are visible
		if (ItemWidth > Width)
			ItemWidth = Width;
		if (ItemHeight > Height)
			ItemHeight = Height;
	}

	CalculateNumCells();
}

void GUIInvWindow::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	ItemWidth = in->ReadInt32();
	ItemHeight = in->ReadInt32();
	CharId = in->ReadInt32();
	TopItem = in->ReadInt32();
	CalculateNumCells();
}

void GUIInvWindow::WriteToSavegame(Stream *out) const {
	GUIObject::WriteToSavegame(out);
	out->WriteInt32(ItemWidth);
	out->WriteInt32(ItemHeight);
	out->WriteInt32(CharId);
	out->WriteInt32(TopItem);
}

void GUIInvWindow::CalculateNumCells() {
	if (ItemWidth <= 0 || ItemHeight <= 0) {
		ColCount = 0;
		RowCount = 0;
	} else if (loaded_game_file_version >= kGameVersion_270) {
		ColCount = Width / data_to_game_coord(ItemWidth);
		RowCount = Height / data_to_game_coord(ItemHeight);
	} else {
		ColCount = floor((float)Width / (float)data_to_game_coord(ItemWidth) + 0.5f);
		RowCount = floor((float)Height / (float)data_to_game_coord(ItemHeight) + 0.5f);
	}
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
