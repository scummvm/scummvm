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

#ifndef AGS_SHARED_GUI_GUILABEL_H
#define AGS_SHARED_GUI_GUILABEL_H

#include "ags/lib/std/vector.h"
#include "ags/shared/gui/guiobject.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

class SplitLines;

namespace AGS {
namespace Shared {

class GUILabel : public GUIObject {
public:
	GUILabel();

	String       GetText() const;

	// Operations
	void Draw(Bitmap *ds) override;
	void SetText(const String &text);

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Stream *out) const override;

	// TODO: these members are currently public; hide them later
public:
	String  Text;
	int32_t Font;
	color_t TextColor;
	HorAlignment TextAlignment;

private:
	void PrepareTextToDraw();
	size_t SplitLinesForDrawing(SplitLines &lines);

	// prepared text buffer/cache
	String _textToDraw;
};

} // namespace Shared
} // namespace AGS

extern std::vector<AGS::Shared::GUILabel> guilabels;
extern int numguilabels;

} // namespace AGS3

#endif
