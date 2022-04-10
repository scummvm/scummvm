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

#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_main.h" // TODO: extract helper functions
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {

namespace AGS {
namespace Shared {

FrameAlignment ConvertLegacyButtonAlignment(LegacyButtonAlignment align) {
	switch (align) {
	case kLegacyButtonAlign_TopCenter:
		return kAlignTopCenter;
	case kLegacyButtonAlign_TopLeft:
		return kAlignTopLeft;
	case kLegacyButtonAlign_TopRight:
		return kAlignTopRight;
	case kLegacyButtonAlign_CenterLeft:
		return kAlignMiddleLeft;
	case kLegacyButtonAlign_Centered:
		return kAlignMiddleCenter;
	case kLegacyButtonAlign_CenterRight:
		return kAlignMiddleRight;
	case kLegacyButtonAlign_BottomLeft:
		return kAlignBottomLeft;
	case kLegacyButtonAlign_BottomCenter:
		return kAlignBottomCenter;
	case kLegacyButtonAlign_BottomRight:
		return kAlignBottomRight;
	default:
		break;
	}
	return kAlignNone;
}


GUIButton::GUIButton() {
	Image = -1;
	MouseOverImage = -1;
	PushedImage = -1;
	CurrentImage = -1;
	Font = 0;
	TextColor = 0;
	TextAlignment = kAlignTopCenter;
	ClickAction[kMouseLeft] = kGUIAction_RunScript;
	ClickAction[kMouseRight] = kGUIAction_RunScript;
	ClickData[kMouseLeft] = 0;
	ClickData[kMouseRight] = 0;

	IsPushed = false;
	IsMouseOver = false;
	_placeholder = kButtonPlace_None;
	_unnamed = false;

	_scEventCount = 1;
	_scEventNames[0] = "Click";
	_scEventArgs[0] = "GUIControl *control, MouseButton button";
}

const String &GUIButton::GetText() const {
	return _text;
}

bool GUIButton::IsImageButton() const {
	return Image != 0;
}

bool GUIButton::IsClippingImage() const {
	return (Flags & kGUICtrl_Clip) != 0;
}

void GUIButton::Draw(Bitmap *ds) {
	bool draw_disabled = !IsGUIEnabled(this);

	// if it's "Unchanged when disabled" or "GUI Off", don't grey out
	if ((GUI::Options.DisabledStyle == kGuiDis_Unchanged) ||
		(GUI::Options.DisabledStyle == kGuiDis_Off)) {
		draw_disabled = false;
	}
	// TODO: should only change properties in reaction to particular events
	if (CurrentImage <= 0 || draw_disabled)
		CurrentImage = Image;

	if (draw_disabled && (GUI::Options.DisabledStyle == kGuiDis_Blackout))
		// buttons off when disabled - no point carrying on
		return;

	// CHECKME: why testing both CurrentImage and Image?
	if (CurrentImage > 0 && IsImageButton())
		DrawImageButton(ds, draw_disabled);
	// CHECKME: why don't draw frame if no Text? this will make button completely invisible!
	else if (!_text.IsEmpty())
		DrawTextButton(ds, draw_disabled);
}

void GUIButton::SetClipImage(bool on) {
	if (on)
		Flags |= kGUICtrl_Clip;
	else
		Flags &= ~kGUICtrl_Clip;
	NotifyParentChanged();
}

void GUIButton::SetText(const String &text) {
	_text = text;
	// Active inventory item placeholders
	if (_text.CompareNoCase("(INV)") == 0)
		// Stretch to fit button
		_placeholder = kButtonPlace_InvItemStretch;
	else if (_text.CompareNoCase("(INVNS)") == 0)
		// Draw at actual size
		_placeholder = kButtonPlace_InvItemCenter;
	else if (_text.CompareNoCase("(INVSHR)") == 0)
		// Stretch if too big, actual size if not
		_placeholder = kButtonPlace_InvItemAuto;
	else
		_placeholder = kButtonPlace_None;

	// TODO: find a way to remove this bogus limitation ("New Button" is a valid Text too)
	_unnamed = _text.Compare("New Button") == 0;
	NotifyParentChanged();
}


bool GUIButton::OnMouseDown() {
	int new_image = (PushedImage > 0) ? PushedImage : CurrentImage;
	if (CurrentImage != new_image || !IsImageButton())
		NotifyParentChanged();
	CurrentImage = new_image;
	IsPushed = true;
	return false;
}

void GUIButton::OnMouseEnter() {
	int new_image = (IsPushed && PushedImage > 0) ? PushedImage :
		(MouseOverImage > 0) ? MouseOverImage : Image;
	if ((CurrentImage != new_image) || (IsPushed && !IsImageButton())) {
		CurrentImage = new_image;
		NotifyParentChanged();
	}
	IsMouseOver = true;
}

void GUIButton::OnMouseLeave() {
	if ((CurrentImage != Image) || (IsPushed && !IsImageButton())) {
		CurrentImage = Image;
		NotifyParentChanged();
	}
	IsMouseOver = false;
}

void GUIButton::OnMouseUp() {
	int new_image;
	if (IsMouseOver) {
		new_image = MouseOverImage;
		if (IsGUIEnabled(this) && IsClickable())
			IsActivated = true;
	} else {
		new_image = Image;
	}

	if ((CurrentImage != new_image) || (IsPushed && !IsImageButton())) {
		CurrentImage = new_image;
		NotifyParentChanged();
	}
	IsPushed = false;
}

// TODO: replace string serialization with StrUtil::ReadString and WriteString
// methods in the future, to keep this organized.
void GUIButton::WriteToFile(Stream *out) const {
	GUIObject::WriteToFile(out);

	out->WriteInt32(Image);
	out->WriteInt32(MouseOverImage);
	out->WriteInt32(PushedImage);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	out->WriteInt32(ClickAction[kMouseLeft]);
	out->WriteInt32(ClickAction[kMouseRight]);
	out->WriteInt32(ClickData[kMouseLeft]);
	out->WriteInt32(ClickData[kMouseRight]);

	StrUtil::WriteString(_text, out);
	out->WriteInt32(TextAlignment);
}

void GUIButton::ReadFromFile(Stream *in, GuiVersion gui_version) {
	GUIObject::ReadFromFile(in, gui_version);

	Image = in->ReadInt32();
	MouseOverImage = in->ReadInt32();
	PushedImage = in->ReadInt32();
	if (gui_version < kGuiVersion_350) { // NOTE: reading into actual variables only for old savegame support
		CurrentImage = in->ReadInt32();
		IsPushed = in->ReadInt32() != 0;
		IsMouseOver = in->ReadInt32() != 0;
	}
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	ClickAction[kMouseLeft] = (GUIClickAction)in->ReadInt32();
	ClickAction[kMouseRight] = (GUIClickAction)in->ReadInt32();
	ClickData[kMouseLeft] = in->ReadInt32();
	ClickData[kMouseRight] = in->ReadInt32();
	if (gui_version < kGuiVersion_350)
		SetText(String::FromStreamCount(in, GUIBUTTON_LEGACY_TEXTLENGTH));
	else
		SetText(StrUtil::ReadString(in));

	if (gui_version >= kGuiVersion_272a) {
		if (gui_version < kGuiVersion_350) {
			TextAlignment = ConvertLegacyButtonAlignment((LegacyButtonAlignment)in->ReadInt32());
			in->ReadInt32(); // reserved1
		} else {
			TextAlignment = (FrameAlignment)in->ReadInt32();
		}
	} else {
		TextAlignment = kAlignTopCenter;
	}

	if (TextColor == 0)
		TextColor = 16;
	CurrentImage = Image;
	// All buttons are translated at the moment
	Flags |= kGUICtrl_Translated;
}

void GUIButton::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	// Properties
	Image = in->ReadInt32();
	MouseOverImage = in->ReadInt32();
	PushedImage = in->ReadInt32();
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	SetText(StrUtil::ReadString(in));
	if (svg_ver >= kGuiSvgVersion_350)
		TextAlignment = (FrameAlignment)in->ReadInt32();
	// Dynamic state
	CurrentImage = in->ReadInt32();

	// Update current state after reading
	IsPushed = false;
	IsMouseOver = false;
}

void GUIButton::WriteToSavegame(Stream *out) const {
	// Properties
	GUIObject::WriteToSavegame(out);
	out->WriteInt32(Image);
	out->WriteInt32(MouseOverImage);
	out->WriteInt32(PushedImage);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	StrUtil::WriteString(GetText(), out);
	out->WriteInt32(TextAlignment);
	// Dynamic state
	out->WriteInt32(CurrentImage);
}

void GUIButton::DrawImageButton(Bitmap *ds, bool draw_disabled) {
	// NOTE: the CLIP flag only clips the image, not the text
	if (IsClippingImage() && !GUI::Options.ClipControls)
		ds->SetClip(RectWH(X, Y, Width, Height));
	if (_GP(spriteset)[CurrentImage] != nullptr)
		draw_gui_sprite(ds, CurrentImage, X, Y, true);

	// Draw active inventory item
	if (_placeholder != kButtonPlace_None && _G(gui_inv_pic) >= 0) {
		GUIButtonPlaceholder place = _placeholder;
		if (place == kButtonPlace_InvItemAuto) {
			if ((get_adjusted_spritewidth(_G(gui_inv_pic)) > Width - 6) ||
			        (get_adjusted_spriteheight(_G(gui_inv_pic)) > Height - 6)) {
				place = kButtonPlace_InvItemStretch;
			} else {
				place = kButtonPlace_InvItemCenter;
			}
		}

		if (place == kButtonPlace_InvItemStretch) {
			ds->StretchBlt(_GP(spriteset)[_G(gui_inv_pic)], RectWH(X + 3, Y + 3, Width - 6, Height - 6), Shared::kBitmap_Transparency);
		} else if (place == kButtonPlace_InvItemCenter) {
			draw_gui_sprite(ds, _G(gui_inv_pic),
			                X + Width / 2 - get_adjusted_spritewidth(_G(gui_inv_pic)) / 2,
			                Y + Height / 2 - get_adjusted_spriteheight(_G(gui_inv_pic)) / 2,
			                true);
		}
	}

	if ((draw_disabled) && (GUI::Options.DisabledStyle == kGuiDis_Greyout)) {
		// darken the button when disabled
		GUI::DrawDisabledEffect(ds, RectWH(X, Y,
		                                   _GP(spriteset)[CurrentImage]->GetWidth(),
		                                   _GP(spriteset)[CurrentImage]->GetHeight()));
	}

	// Don't print Text of (INV) (INVSHR) (INVNS)
	if (_placeholder == kButtonPlace_None && !_unnamed)
		DrawText(ds, draw_disabled);

	if (IsClippingImage() && !GUI::Options.ClipControls)
		ds->ResetClip();
}

void GUIButton::DrawText(Bitmap *ds, bool draw_disabled) {
	if (_text.IsEmpty())
		return;
	// TODO: need to find a way to cache Text prior to drawing;
	// but that will require to update all gui controls when translation is changed in game
	PrepareTextToDraw();

	Rect frame = RectWH(X + 2, Y + 2, Width - 4, Height - 4);
	if (IsPushed && IsMouseOver) {
		// move the Text a bit while pushed
		frame.Left++;
		frame.Top++;
	}
	color_t text_color = ds->GetCompatibleColor(TextColor);
	if (draw_disabled)
		text_color = ds->GetCompatibleColor(8);
	GUI::DrawTextAligned(ds, _textToDraw.GetCStr(), Font, text_color, frame, TextAlignment);
}

void GUIButton::DrawTextButton(Bitmap *ds, bool draw_disabled) {
	color_t draw_color = ds->GetCompatibleColor(7);
	ds->FillRect(Rect(X, Y, X + Width - 1, Y + Height - 1), draw_color);
	if (Flags & kGUICtrl_Default) {
		draw_color = ds->GetCompatibleColor(16);
		ds->DrawRect(Rect(X - 1, Y - 1, X + Width, Y + Height), draw_color);
	}

	// TODO: use color constants instead of literal numbers
	if (!draw_disabled && IsMouseOver && IsPushed)
		draw_color = ds->GetCompatibleColor(15);
	else
		draw_color = ds->GetCompatibleColor(8);

	ds->DrawLine(Line(X, Y + Height - 1, X + Width - 1, Y + Height - 1), draw_color);
	ds->DrawLine(Line(X + Width - 1, Y, X + Width - 1, Y + Height - 1), draw_color);

	if (draw_disabled || (IsMouseOver && IsPushed))
		draw_color = ds->GetCompatibleColor(8);
	else
		draw_color = ds->GetCompatibleColor(15);

	ds->DrawLine(Line(X, Y, X + Width - 1, Y), draw_color);
	ds->DrawLine(Line(X, Y, X, Y + Height - 1), draw_color);

	DrawText(ds, draw_disabled);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
