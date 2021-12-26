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

#ifndef AGS_SHARED_GUI_GUI_LABEL_H
#define AGS_SHARED_GUI_GUI_LABEL_H

#include "ags/lib/std/vector.h"
#include "ags/shared/gui/gui_object.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

class SplitLines;

namespace AGS {
namespace Shared {

class GUILabel : public GUIObject {
public:
	GUILabel();

	// Gets label's text property in original set form (with macros etc)
	String       GetText() const;
	// Gets which macro are contained within label's text
	GUILabelMacro GetTextMacros() const;

	// Operations
	void Draw(Bitmap *ds) override;
	void SetText(const String &text);

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Shared::Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Shared::Stream *out) const override;

	// TODO: these members are currently public; hide them later
public:
	String  Text;
	int32_t Font;
	color_t TextColor;
	HorAlignment TextAlignment;

private:
	void PrepareTextToDraw();
	size_t SplitLinesForDrawing(SplitLines &lines);

	// Information on macros contained within Text field
	GUILabelMacro _textMacro;
	// prepared text buffer/cache
	String _textToDraw;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
