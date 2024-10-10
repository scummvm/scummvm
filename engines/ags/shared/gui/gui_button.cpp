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
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/font/fonts.h"
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
	_image = -1;
	_mouseOverImage = -1;
	_pushedImage = -1;
	_currentImage = -1;
	Font = 0;
	TextColor = 0;
	TextAlignment = kAlignTopCenter;
	ClickAction[kGUIClickLeft] = kGUIAction_RunScript;
	ClickAction[kGUIClickRight] = kGUIAction_RunScript;
	ClickData[kGUIClickLeft] = 0;
	ClickData[kGUIClickRight] = 0;

	IsPushed = false;
	IsMouseOver = false;
	_placeholder = kButtonPlace_None;
	_unnamed = true;

	_scEventCount = 1;
	_scEventNames[0] = "Click";
	_scEventArgs[0] = "GUIControl *control, MouseButton button";
}

bool GUIButton::HasAlphaChannel() const {
	return ((_currentImage > 0) && is_sprite_alpha(_currentImage)) ||
		(!_unnamed && is_font_antialiased(Font));
}

int32_t GUIButton::GetCurrentImage() const {
	return _currentImage;
}

int32_t GUIButton::GetNormalImage() const {
	return _image;
}

int32_t GUIButton::GetMouseOverImage() const {
	return _mouseOverImage;
}

int32_t GUIButton::GetPushedImage() const {
	return _pushedImage;
}

GUIButtonPlaceholder GUIButton::GetPlaceholder() const {
	return _placeholder;
}

const String &GUIButton::GetText() const {
	return _text;
}

bool GUIButton::IsImageButton() const {
	return _image > 0;
}

bool GUIButton::IsClippingImage() const {
	return (Flags & kGUICtrl_Clip) != 0;
}

Rect GUIButton::CalcGraphicRect(bool clipped) {
	if (clipped)
		return RectWH(0, 0, _width, _height);

	// TODO: need to find a way to cache image and text position, or there'll be some repetition
	Rect rc = RectWH(0, 0, _width, _height);
	if (IsImageButton()) {
		if (IsClippingImage())
			return rc;
		// Main button graphic
		if (_currentImage >= 0 && _GP(spriteset).DoesSpriteExist(_currentImage))
			rc = SumRects(rc, RectWH(0, 0, get_adjusted_spritewidth(_currentImage), get_adjusted_spriteheight(_currentImage)));
		// Optionally merge with the inventory pic
		if (_placeholder != kButtonPlace_None && _G(gui_inv_pic) >= 0) {
			Size inv_sz = Size(get_adjusted_spritewidth(_G(gui_inv_pic)),
				get_adjusted_spriteheight(_G(gui_inv_pic)));
			GUIButtonPlaceholder place = _placeholder;
			if (place == kButtonPlace_InvItemAuto) {
				place = ((inv_sz.Width > _width - 6) || (inv_sz.Height > _height - 6)) ?
					kButtonPlace_InvItemStretch : kButtonPlace_InvItemCenter;
			}

			Rect inv_rc = (place == kButtonPlace_InvItemStretch) ?
				RectWH(0 + 3, 0 + 3, _width - 6, _height - 6) :
				RectWH(0 + _width / 2 - inv_sz.Width / 2,
					0 + _height / 2 - inv_sz.Height / 2,
					inv_sz.Width, inv_sz.Height);
			rc = SumRects(rc, inv_rc);
		}
	}
	// Optionally merge with the button text
	if (!IsImageButton() || ((_placeholder == kButtonPlace_None) && !_unnamed)) {
		PrepareTextToDraw();
		Rect frame = RectWH(0 + 2, 0 + 2, _width - 4, _height - 4);
		if (IsPushed && IsMouseOver) {
			frame.Left++;
			frame.Top++;
		}
		rc = SumRects(rc, GUI::CalcTextGraphicalRect(_textToDraw.GetCStr(), Font, frame, TextAlignment));
	}
	return rc;
}

void GUIButton::Draw(Bitmap *ds, int x, int y) {
	bool draw_disabled = !IsGUIEnabled(this);

	// if it's "Unchanged when disabled" or "GUI Off", don't grey out
	if ((GUI::Options.DisabledStyle == kGuiDis_Unchanged) ||
		(GUI::Options.DisabledStyle == kGuiDis_Off)) {
		draw_disabled = false;
	}
	// TODO: should only change properties in reaction to particular events
	if (_currentImage <= 0 || draw_disabled)
		_currentImage = _image;

	if (draw_disabled && (GUI::Options.DisabledStyle == kGuiDis_Blackout))
		// buttons off when disabled - no point carrying on
		return;

	if (IsImageButton())
		DrawImageButton(ds, x, y, draw_disabled);
	// CHECKME: why don't draw frame if no Text? this will make button completely invisible!
	else if (!_text.IsEmpty())
		DrawTextButton(ds, x, y, draw_disabled);
}

void GUIButton::SetClipImage(bool on) {
	if (on != ((Flags & kGUICtrl_Clip) != 0))
		MarkChanged();
	if (on)
		Flags |= kGUICtrl_Clip;
	else
		Flags &= ~kGUICtrl_Clip;
}

void GUIButton::SetCurrentImage(int32_t image) {
	if (_currentImage == image)
		return;

	_currentImage = image;
	MarkChanged();
}

void GUIButton::SetMouseOverImage(int32_t image) {
	if (_mouseOverImage == image)
		return;

	_mouseOverImage = image;
	UpdateCurrentImage();
}

void GUIButton::SetNormalImage(int32_t image) {
	if (_image == image)
		return;

	_image = image;
	UpdateCurrentImage();
}

void GUIButton::SetPushedImage(int32_t image) {
	if (_pushedImage == image)
		return;

	_pushedImage = image;
	UpdateCurrentImage();
}

void GUIButton::SetImages(int32_t normal, int32_t over, int32_t pushed) {
	_image = normal;
	_mouseOverImage = over;
	_pushedImage = pushed;
	UpdateCurrentImage();
}

void GUIButton::SetText(const String &text) {
	if (_text == text)
		return;

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
	_unnamed = _text.IsEmpty() || _text.Compare("New Button") == 0;
	MarkChanged();
}

bool GUIButton::OnMouseDown() {
	if (!IsImageButton())
		MarkChanged();
	IsPushed = true;
	UpdateCurrentImage();
	return false;
}

void GUIButton::OnMouseEnter() {
	if (IsPushed && !IsImageButton())
		MarkChanged();
	IsMouseOver = true;
	UpdateCurrentImage();
}

void GUIButton::OnMouseLeave() {
	if (IsPushed && !IsImageButton())
		MarkChanged();
	IsMouseOver = false;
	UpdateCurrentImage();
}

void GUIButton::OnMouseUp() {
	if (IsMouseOver) {
		if (IsGUIEnabled(this) && IsClickable())
			IsActivated = true;
	}
	if (IsPushed && !IsImageButton())
		MarkChanged();
	IsPushed = false;
	UpdateCurrentImage();
}

void GUIButton::UpdateCurrentImage() {
	int new_image = _currentImage;

	if (IsPushed && (_pushedImage > 0)) {
		new_image = _pushedImage;
	} else if (IsMouseOver && (_mouseOverImage > 0)) {
		new_image = _mouseOverImage;
	} else {
		new_image = _image;
	}

	SetCurrentImage(new_image);
}

void GUIButton::WriteToFile(Stream *out) const {
	GUIObject::WriteToFile(out);

	out->WriteInt32(_image);
	out->WriteInt32(_mouseOverImage);
	out->WriteInt32(_pushedImage);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	out->WriteInt32(ClickAction[kGUIClickLeft]);
	out->WriteInt32(ClickAction[kGUIClickRight]);
	out->WriteInt32(ClickData[kGUIClickLeft]);
	out->WriteInt32(ClickData[kGUIClickRight]);

	StrUtil::WriteString(_text, out);
	out->WriteInt32(TextAlignment);
}

void GUIButton::ReadFromFile(Stream *in, GuiVersion gui_version) {
	GUIObject::ReadFromFile(in, gui_version);

	_image = in->ReadInt32();
	_mouseOverImage = in->ReadInt32();
	_pushedImage = in->ReadInt32();
	if (gui_version < kGuiVersion_350) { // NOTE: reading into actual variables only for old savegame support
		_currentImage = in->ReadInt32();
		IsPushed = in->ReadInt32() != 0;
		IsMouseOver = in->ReadInt32() != 0;
	}
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	ClickAction[kGUIClickLeft] = (GUIClickAction)in->ReadInt32();
	ClickAction[kGUIClickRight] = (GUIClickAction)in->ReadInt32();
	ClickData[kGUIClickLeft] = in->ReadInt32();
	ClickData[kGUIClickRight] = in->ReadInt32();
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
	_currentImage = _image;
}

void GUIButton::ReadFromSavegame(Stream *in, GuiSvgVersion svg_ver) {
	GUIObject::ReadFromSavegame(in, svg_ver);
	// Properties
	_image = in->ReadInt32();
	_mouseOverImage = in->ReadInt32();
	_pushedImage = in->ReadInt32();
	Font = in->ReadInt32();
	TextColor = in->ReadInt32();
	SetText(StrUtil::ReadString(in));
	if (svg_ver >= kGuiSvgVersion_350)
		TextAlignment = (FrameAlignment)in->ReadInt32();
	// Dynamic state
	_currentImage = in->ReadInt32();

	// Update current state after reading
	IsPushed = false;
	IsMouseOver = false;
}

void GUIButton::WriteToSavegame(Stream *out) const {
	// Properties
	GUIObject::WriteToSavegame(out);
	out->WriteInt32(_image);
	out->WriteInt32(_mouseOverImage);
	out->WriteInt32(_pushedImage);
	out->WriteInt32(Font);
	out->WriteInt32(TextColor);
	StrUtil::WriteString(GetText(), out);
	out->WriteInt32(TextAlignment);
	// Dynamic state
	out->WriteInt32(_currentImage);
}

void GUIButton::DrawImageButton(Bitmap *ds, int x, int y, bool draw_disabled) {
	assert(_currentImage >= 0);
	// NOTE: the CLIP flag only clips the image, not the text
	if (IsClippingImage() && !GUI::Options.ClipControls)
		ds->SetClip(RectWH(x, y, _width, _height));
	if (_GP(spriteset).DoesSpriteExist(_currentImage))
		draw_gui_sprite(ds, _currentImage, x, y, true);

	// Draw active inventory item
	if (_placeholder != kButtonPlace_None && _G(gui_inv_pic) >= 0) {
		Size inv_sz = Size(get_adjusted_spritewidth(_G(gui_inv_pic)), get_adjusted_spriteheight(_G(gui_inv_pic)));
		GUIButtonPlaceholder place = _placeholder;
		if (place == kButtonPlace_InvItemAuto) {
			place = ((inv_sz.Width > _width - 6) || (inv_sz.Height > _height - 6)) ?
				kButtonPlace_InvItemStretch : kButtonPlace_InvItemCenter;
		}

		if (place == kButtonPlace_InvItemStretch) {
			ds->StretchBlt(_GP(spriteset)[_G(gui_inv_pic)], RectWH(x + 3, y + 3, _width - 6, _height - 6),
				kBitmap_Transparency);
		} else {
			draw_gui_sprite(ds, _G(gui_inv_pic),
				x + _width / 2 - inv_sz.Width / 2,
				y + _height / 2 - inv_sz.Height / 2,
				true);
		}
	}

	if ((draw_disabled) && (GUI::Options.DisabledStyle == kGuiDis_Greyout)) {
		// darken the button when disabled
		const Size sz = _GP(spriteset).GetSpriteResolution(_currentImage);
		GUI::DrawDisabledEffect(ds, RectWH(x, y, sz.Width, sz.Height));
	}

	// Don't print Text of (INV) (INVSHR) (INVNS)
	if ((_placeholder == kButtonPlace_None) && !_unnamed)
		DrawText(ds, x, y, draw_disabled);

	if (IsClippingImage() && !GUI::Options.ClipControls)
		ds->ResetClip();
}

void GUIButton::DrawText(Bitmap *ds, int x, int y, bool draw_disabled) {
	// TODO: need to find a way to cache Text prior to drawing;
	// but that will require to update all gui controls when translation is changed in game
	PrepareTextToDraw();

	Rect frame = RectWH(x + 2, y + 2, _width - 4, _height - 4);
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

void GUIButton::DrawTextButton(Bitmap *ds, int x, int y, bool draw_disabled) {
	color_t draw_color = ds->GetCompatibleColor(7);
	ds->FillRect(Rect(x, y, x + _width - 1, y + _height - 1), draw_color);
	if (Flags & kGUICtrl_Default) {
		draw_color = ds->GetCompatibleColor(16);
		ds->DrawRect(Rect(x - 1, y - 1, x + _width, y + _height), draw_color);
	}

	// TODO: use color constants instead of literal numbers
	if (!draw_disabled && IsMouseOver && IsPushed)
		draw_color = ds->GetCompatibleColor(15);
	else
		draw_color = ds->GetCompatibleColor(8);

	ds->DrawLine(Line(x, y + _height - 1, x + _width - 1, y + _height - 1), draw_color);
	ds->DrawLine(Line(x + _width - 1, y, x + _width - 1, y + _height - 1), draw_color);

	if (draw_disabled || (IsMouseOver && IsPushed))
		draw_color = ds->GetCompatibleColor(8);
	else
		draw_color = ds->GetCompatibleColor(15);

	ds->DrawLine(Line(x, y, x + _width - 1, y), draw_color);
	ds->DrawLine(Line(x, y, x, y + _height - 1), draw_color);

	DrawText(ds, x, y, draw_disabled);
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
