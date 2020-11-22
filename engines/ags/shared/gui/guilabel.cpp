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

#include "ac/game_version.h"
#include "font/fonts.h"
#include "gui/guilabel.h"
#include "gui/guimain.h"
#include "util/stream.h"
#include "util/string_utils.h"

std::vector<AGS::Shared::GUILabel> guilabels;
int numguilabels = 0;

#define GUILABEL_TEXTLENGTH_PRE272 200

namespace AGS {
namespace Shared {

GUILabel::GUILabel() {
	Font = 0;
	TextColor = 0;
	TextAlignment = kHAlignLeft;

	_scEventCount = 0;
}

String GUILabel::GetText() const {
	return Text;
}

void GUILabel::Draw(Common::Bitmap *ds) {
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
	        ++i, at_y += linespacing) {
		GUI::DrawTextAlignedHor(ds, Lines[i], Font, text_color, X, X + Width - 1, at_y,
		                        (FrameAlignment)TextAlignment);
	}
}

void GUILabel::SetText(const String &text) {
	Text = text;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUILabel::WriteToFile(Stream *out) const {
	GUIObject::WriteToFile(out);
	StrUtil::WriteString(Text, out);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	out->WriteInt32(TextAlignment);
}

void GUILabel::ReadFromFile(Stream *in, GuiVersion gui_version) {
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

void GUILabel::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	Text = StrUtil::ReadString(in);
	if (svg_ver >= kGuiSvgVersion_350)
		TextAlignment = (HorAlignment)in->ReadInt32();
}

void GUILabel::WriteToSavegame(Stream *out) const {
	GUIObject::WriteToSavegame(out);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	StrUtil::WriteString(Text, out);
	out->WriteInt32(TextAlignment);
}

} // namespace Shared
} // namespace AGS
