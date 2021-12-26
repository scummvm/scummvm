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

#ifndef AGS_SHARED_GUI_GUI_SLIDER_H
#define AGS_SHARED_GUI_GUI_SLIDER_H

#include "ags/lib/std/vector.h"
#include "ags/shared/gui/gui_object.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class GUISlider : public GUIObject {
public:
	GUISlider();

	// Tells if the slider is horizontal (otherwise - vertical)
	bool IsHorizontal() const;
	bool IsOverControl(int x, int y, int leeway) const override;

	// Operations
	void Draw(Bitmap *ds) override;

	// Events
	bool OnMouseDown() override;
	void OnMouseMove(int xp, int yp) override;
	void OnMouseUp() override;

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Stream *out) const override;

	// TODO: these members are currently public; hide them later
public:
	int32_t MinValue;
	int32_t MaxValue;
	int32_t Value;
	int32_t BgImage;
	int32_t HandleImage;
	int32_t HandleOffset;
	bool    IsMousePressed;

private:
	// The following variables are not persisted on disk
	// Cached coordinates of slider handle
	Rect    _cachedHandle;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
