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

#include "common/std/algorithm.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

#define GUILABEL_TEXTLENGTH_PRE272 200

namespace AGS3 {
namespace AGS {
namespace Shared {

GUILabel::GUILabel() {
	Font = 0;
	TextColor = 0;
	TextAlignment = kHAlignLeft;

	_scEventCount = 0;
}

bool GUILabel::HasAlphaChannel() const {
	return is_font_antialiased(Font);
}

String GUILabel::GetText() const {
	return Text;
}

GUILabelMacro GUILabel::GetTextMacros() const {
	return _textMacro;
}

Rect GUILabel::CalcGraphicRect(bool clipped) {
	if (clipped)
		return RectWH(0, 0, _width, _height);

	// TODO: need to find a way to text position, or there'll be some repetition
	// have to precache text and size on some events:
	// - translation change
	// - macro value change (score, overhotspot etc)
	Rect rc = RectWH(0, 0, _width, _height);
	if (PrepareTextToDraw() == 0)
		return rc;
	const int linespacing = // Older engine labels used (font height + 1) as linespacing for some reason
		((_G(loaded_game_file_version) < kGameVersion_360) && (get_font_flags(Font) & FFLG_DEFLINESPACING)) ?
		(get_font_height(Font) + 1) :
		get_font_linespacing(Font);
	// < 2.72 labels did not limit vertical size of text
	const bool limit_by_label_frame = _G(loaded_game_file_version) >= kGameVersion_272;
	int at_y = 0;
	Line max_line;
	for (size_t i = 0;
		i < _GP(Lines).Count() && (!limit_by_label_frame || at_y <= _height);
		++i, at_y += linespacing) {
		Line lpos = GUI::CalcTextPositionHor(_GP(Lines)[i].GetCStr(), Font, 0, 0 + _width - 1, at_y,
			(FrameAlignment)TextAlignment);
		max_line.X2 = MAX(max_line.X2, lpos.X2);
	}
	// Include font fixes for the first and last text line,
	// in case graphical height is different, and there's a VerticalOffset
	Line vextent = GUI::CalcFontGraphicalVExtent(Font);
	Rect text_rc = RectWH(0, vextent.Y1, max_line.X2 - max_line.X1 + 1, at_y - linespacing + (vextent.Y2 - vextent.Y1));
	return SumRects(rc, text_rc);
}

void GUILabel::Draw(Bitmap *ds, int x, int y) {
	// TODO: need to find a way to cache text prior to drawing;
	// but that will require to update all gui controls when translation is changed in game
	if (PrepareTextToDraw() == 0)
		return;

	color_t text_color = ds->GetCompatibleColor(TextColor);
	const int linespacing = // Older engine labels used (font height + 1) as linespacing for some reason
		((_G(loaded_game_file_version) < kGameVersion_360) && (get_font_flags(Font) & FFLG_DEFLINESPACING)) ?
		(get_font_height(Font) + 1) :
		get_font_linespacing(Font);
	// < 2.72 labels did not limit vertical size of text
	const bool limit_by_label_frame = _G(loaded_game_file_version) >= kGameVersion_272;
	int at_y = y;
	for (size_t i = 0;
		i < _GP(Lines).Count() && (!limit_by_label_frame || at_y <= y + _height);
		++i, at_y += linespacing) {
		GUI::DrawTextAlignedHor(ds, _GP(Lines)[i].GetCStr(), Font, text_color, x, x + _width - 1, at_y,
			(FrameAlignment)TextAlignment);
	}
}

void GUILabel::SetText(const String &text) {
	if (text == Text)
		return;
	Text = text;
	// Check for macros within text
	_textMacro = GUI::FindLabelMacros(Text);
	MarkChanged();
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

	_textMacro = GUI::FindLabelMacros(Text);
}

void GUILabel::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	Text = StrUtil::ReadString(in);
	if (svg_ver >= kGuiSvgVersion_350)
		TextAlignment = (HorAlignment)in->ReadInt32();

	_textMacro = GUI::FindLabelMacros(Text);
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
} // namespace AGS3
