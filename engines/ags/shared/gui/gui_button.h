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

#ifndef AGS_SHARED_GUI_GUI_BUTTON_H
#define AGS_SHARED_GUI_GUI_BUTTON_H

#include "ags/lib/std/vector.h"
#include "ags/engine/ac/button.h"
#include "ags/shared/gui/gui_object.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

#define GUIBUTTON_LEGACY_TEXTLENGTH 50

namespace AGS {
namespace Shared {

enum MouseButton {
	kMouseNone = -1,
	kMouseLeft = 0,
	kMouseRight = 1,
};

enum GUIClickAction {
	kGUIAction_None = 0,
	kGUIAction_SetMode = 1,
	kGUIAction_RunScript = 2,
};

enum LegacyButtonAlignment {
	kLegacyButtonAlign_TopCenter = 0,
	kLegacyButtonAlign_TopLeft = 1,
	kLegacyButtonAlign_TopRight = 2,
	kLegacyButtonAlign_CenterLeft = 3,
	kLegacyButtonAlign_Centered = 4,
	kLegacyButtonAlign_CenterRight = 5,
	kLegacyButtonAlign_BottomLeft = 6,
	kLegacyButtonAlign_BottomCenter = 7,
	kLegacyButtonAlign_BottomRight = 8,
};

class GUIButton : public GUIObject {
public:
	GUIButton();

	const String &GetText() const;
	bool IsImageButton() const;
	bool IsClippingImage() const;

	// Operations
	void Draw(Bitmap *ds) override;
	void SetClipImage(bool on);
	void SetText(const String &text);

	// Events
	bool OnMouseDown() override;
	void OnMouseEnter() override;
	void OnMouseLeave() override;
	void OnMouseUp() override;

	// Serialization
	void ReadFromFile(Stream *in, GuiVersion gui_version) override;
	void WriteToFile(Stream *out) const override;
	void ReadFromSavegame(Shared::Stream *in, GuiSvgVersion svg_ver) override;
	void WriteToSavegame(Shared::Stream *out) const override;

	// TODO: these members are currently public; hide them later
	public:
	int32_t     Image;
	int32_t     MouseOverImage;
	int32_t     PushedImage;
	int32_t     CurrentImage;
	int32_t     Font;
	color_t     TextColor;
	FrameAlignment TextAlignment;
	// Click actions for left and right mouse buttons
	// NOTE: only left click is currently in use
	static const int ClickCount = kMouseRight + 1;
	GUIClickAction ClickAction[ClickCount];
	int32_t        ClickData[ClickCount];

	bool        IsPushed;
	bool        IsMouseOver;

private:
	void DrawImageButton(Bitmap *ds, bool draw_disabled);
	void DrawText(Bitmap *ds, bool draw_disabled);
	void DrawTextButton(Bitmap *ds, bool draw_disabled);
	void PrepareTextToDraw();

	// Defines button placeholder mode; the mode is set
	// depending on special tags found in button text
	enum GUIButtonPlaceholder {
		kButtonPlace_None,
		kButtonPlace_InvItemStretch,
		kButtonPlace_InvItemCenter,
		kButtonPlace_InvItemAuto
	};

	// Text property set by user
	String _text;
	// type of content placeholder, if any
	GUIButtonPlaceholder _placeholder;
	// A flag indicating unnamed button; this is a convenience trick:
	// buttons are created named "New Button" in the editor, and users
	// often do not clear text when they want a graphic button.
	bool _unnamed;
	// Prepared text buffer/cache
	String _textToDraw;
};

} // namespace Shared
} // namespace AGS

} // namespace AGS3

#endif
