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

#ifndef HODJNPODJ_GFX_BUTTON_H
#define HODJNPODJ_GFX_BUTTON_H

#include "bagel/hodjnpodj/gfx/gfx_surface.h"
#include "bagel/hodjnpodj/events.h"
#include "bagel/boflib/palette.h"

namespace Bagel {
namespace HodjNPodj {

/*
 * Owner draw state
 */
enum DrawState {
	ODS_SELECTED    = 0x0001,
	ODS_GRAYED      = 0x0002,
	ODS_DISABLED    = 0x0004,
	ODS_CHECKED     = 0x0008,
	ODS_FOCUS       = 0x0010
};

#define BUTTON_EDGE_WIDTH		3
#define	FOCUS_RECT_DX			2
#define FOCUS_RECT_DY			1
#define SELECTED_TEXT_DX		1
#define SELECTED_TEXT_DY		1
#define CHECK_BOX_SIZE			13
#define CHECK_BOX_DX			4
#define CHECK_TEXT_DX			22
#define RADIO_BOX_SIZE			13
#define RADIO_BOX_DX			4
#define RADIO_TEXT_DX			22

#define	RGB_BUTTON_FACE         PALETTERGB(199,167,139) // PALETTERGB(207,159,115)
#define	RGB_BUTTON_HIGHLIGHT	PALETTERGB(223,199,175)
#define	RGB_BUTTON_SHADOW       PALETTERGB(155,123,95)
#define	RGB_BUTTON_TEXT         PALETTERGB(51,31,19) // PALETTERGB(83,59,1)
#define RGB_BUTTON_TEXT_DISABLE PALETTERGB(155,139,123)
#define	RGB_BUTTON_OUTLINE      PALETTERGB(83,59,51) // PALETTERGB(83,59,1)

#define	RGB_CHECK_FACE         	PALETTERGB(199,167,139) // PALETTERGB(207,159,115)
#define	RGB_CHECK_CONTROL      	PALETTERGB(99,15,7)
#define	RGB_CHECK_TEXT         	PALETTERGB(51,31,19) // PALETTERGB(83,59,1)
#define RGB_CHECK_TEXT_DISABLE 	PALETTERGB(155,139,123)
#define	RGB_CHECK_OUTLINE      	PALETTERGB(83,59,51) // PALETTERGB(207,159,115)

#define	RGB_RADIO_FACE         	PALETTERGB(199,167,139) // PALETTERGB(207,159,115)
#define	RGB_RADIO_CONTROL      	PALETTERGB(99,15,7)
#define	RGB_RADIO_TEXT         	PALETTERGB(51,31,19) // PALETTERGB(83,59,1)
#define RGB_RADIO_TEXT_DISABLE 	PALETTERGB(155,139,123)
#define	RGB_RADIO_OUTLINE      	PALETTERGB(83,59,51) // PALETTERGB(207,159,115)

class Button : public UIElement {
protected:
	uint _itemState = 0;
	Common::String _text;

	virtual void buttonPressed();

public:
	Button(const Common::String &name, UIElement *uiParent) :
		UIElement(name, uiParent) {}
	Button(const Common::String &name) :
		UIElement(name) {}
	Button(const Common::String &name, const Common::String &text, UIElement *uiParent) :
		UIElement(name, uiParent), _text(text) {
	}
	Button(const Common::String &name, const Common::String &text) :
		UIElement(name), _text(text) {
	}

	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	bool msgMouseEnter(const MouseEnterMessage &msg) override;
	bool msgMouseLeave(const MouseLeaveMessage &msg);
	bool msgMouseMove(const MouseMoveMessage &msg) override {
		return true;
	}
	bool msgKeypress(const KeypressMessage &msg) override;

	void setPressed(bool pressed) {
		_itemState = pressed ? ODS_SELECTED : 0;
		redraw();
	}

	void setText(const Common::String &text);
	void enableWindow(bool enabled) {
		_itemState = enabled ? 0 : ODS_DISABLED;
		redraw();
	}

	uint getItemState() const {
		return _itemState;
	}
	bool isDisabled() const {
		return (_itemState & ODS_DISABLED) != 0;
	}
	bool isEnabled() const {
		return !isDisabled();
	}
};

class BmpButton : public Button {
private:
	GfxSurface _base;
	GfxSurface _selected;
	GfxSurface _focused;
	GfxSurface _disabled;

public:
	BmpButton(const Common::String &name, UIElement *parent,
		const Common::Rect &r) :
		Button(name, parent) {
		setBounds(r);
	}

	void loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled);
	void clear();

	void draw() override;
};

class ColorButton : public Button {
private:
	COLORREF _cButtonFace = RGB_BUTTON_FACE;
	COLORREF _cButtonHighlight = RGB_BUTTON_HIGHLIGHT;
	COLORREF _cButtonShadow = RGB_BUTTON_SHADOW;
	COLORREF _cButtonText = RGB_BUTTON_TEXT;
	COLORREF _cButtonTextDisabled = RGB_BUTTON_TEXT_DISABLE;
	COLORREF _cButtonOutline = RGB_BUTTON_OUTLINE;

public:
	ColorButton(const Common::String &text, UIElement *parent = nullptr) :
		Button(text, "ColorButton", parent) {
	}
	ColorButton(const Common::String &text, const Common::Rect &r, UIElement *parent = nullptr) :
		Button(text, "ColorButton", parent) {
		setBounds(r);
	}
	ColorButton(const Common::String &name, const Common::String &text, UIElement *parent = nullptr) :
		Button(name, text, parent) {
	}
	ColorButton(const Common::String &name, const Common::String &text, const Common::Rect &r, UIElement *parent = nullptr) :
		Button(name, text, parent) {
		setBounds(r);
	}
	~ColorButton() override {
	}

	void draw() override;
};

class CheckButton : public Button {
private:
	const COLORREF _cButtonFace = RGB_CHECK_FACE;
	const COLORREF _cButtonControl = RGB_CHECK_CONTROL;
	const COLORREF _cButtonText = RGB_CHECK_TEXT;
	const COLORREF _cButtonTextDisabled = RGB_CHECK_TEXT_DISABLE;
	const COLORREF _cButtonOutline = RGB_CHECK_OUTLINE;
	bool _checked = false;

	Common::Rect getCheckRect() const;

protected:
	void buttonPressed() override;

public:
	CheckButton(const Common::String &name, UIElement *parent = nullptr) :
		Button(name, parent) {
	}
	CheckButton(const Common::String &name, const Common::Rect &r, UIElement *parent = nullptr) :
		Button(name, parent) {
		setBounds(r);
	}
	~CheckButton() override {
	}

	void draw() override;
	bool msgMouseDown(const MouseDownMessage &msg) override {
		return true;
	}
	bool msgMouseUp(const MouseUpMessage &msg) override;

	void setCheck(bool checked);
};


class RadioButton : public Button {
private:
	const COLORREF _cButtonFace = RGB_CHECK_FACE;
	const COLORREF _cButtonControl = RGB_CHECK_CONTROL;
	const COLORREF _cButtonText = RGB_CHECK_TEXT;
	const COLORREF _cButtonTextDisabled = RGB_CHECK_TEXT_DISABLE;
	const COLORREF _cButtonOutline = RGB_CHECK_OUTLINE;
	bool _checked = false;

	Common::Rect getCheckRect() const;

protected:
	void buttonPressed() override;

public:
	RadioButton(const Common::String &name, const Common::String &text,
			const Common::Rect &r, UIElement *parent) :
			Button(name, text, parent) {
		setBounds(r);
	}
	~RadioButton() override {
	}

	void draw() override;
	bool msgMouseDown(const MouseDownMessage &msg) override {
		return true;
	}
	bool msgMouseUp(const MouseUpMessage &msg) override;

	void setCheck(bool checked);
};

class OkButton : public ColorButton {
public:
	OkButton(UIElement *parent = nullptr);
	OkButton(const Common::Rect &r, UIElement *parent = nullptr);
	bool msgMouseEnter(const MouseEnterMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

class CancelButton : public ColorButton {
public:
	CancelButton(UIElement *parent = nullptr);
	CancelButton(const Common::Rect &r, UIElement *parent = nullptr);
	bool msgAction(const ActionMessage &msg) override;
};

class DefaultsButton : public ColorButton {
public:
	DefaultsButton(UIElement *parent = nullptr);
	DefaultsButton(const Common::Rect &r, UIElement *parent = nullptr);
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
