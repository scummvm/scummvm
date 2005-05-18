/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "common/scummsys.h"
#include "common/str.h"
#include "graphics/font.h"
#include "graphics/surface.h"
#include "gui/object.h"

namespace Graphics {
	class Font;
}

namespace GUI {

class Dialog;

enum {
	WIDGET_ENABLED		= 1 << 0,
	WIDGET_INVISIBLE	= 1 << 1,
	WIDGET_HILITED		= 1 << 2,
	WIDGET_BORDER		= 1 << 3,
	WIDGET_INV_BORDER	= 1 << 4,
	WIDGET_CLEARBG		= 1 << 5,
	WIDGET_WANT_TICKLE	= 1 << 7,
	WIDGET_TRACK_MOUSE	= 1 << 8,
	WIDGET_RETAIN_FOCUS	= 1 << 9		// Retain focus on mouse up. By default widgets lose focus on mouseup, but some widgets might want to retain it - widgets where you enter text, for instance

};

enum {
	kStaticTextWidget	= 'TEXT',
	kEditTextWidget		= 'EDIT',
	kButtonWidget		= 'BTTN',
	kCheckboxWidget		= 'CHKB',
	kSliderWidget		= 'SLDE',
	kListWidget			= 'LIST',
	kScrollBarWidget	= 'SCRB',
	kPopUpWidget		= 'POPU',
	kTabWidget			= 'TABW',
	kGraphicsWidget		= 'GFXW'
};

enum {
	kCaretBlinkTime = 300
};

enum WidgetSize {
	kDefaultWidgetSize,
	kNormalWidgetSize,
	kBigWidgetSize
};

enum {
	kButtonWidth = 72,
	kButtonHeight = 16,
	kSliderWidth = 85,
	kSliderHeight = 12,

	kBigButtonWidth = 108,
	kBigButtonHeight = 24,
	kBigSliderWidth = 128,
	kBigSliderHeight = 18
};


/* Widget */
class Widget : public GuiObject {
	friend class Dialog;
protected:
	uint32		_type;
	GuiObject	*_boss;
	Widget		*_next;
	uint16		_id;
	uint16		_flags;
	bool		_hasFocus;

public:
	static Widget *findWidgetInChain(Widget *start, int x, int y);

public:
	Widget(GuiObject *boss, int x, int y, int w, int h);
	virtual ~Widget();

	virtual int16	getAbsX() const	{ return _x + _boss->getChildX(); }
	virtual int16	getAbsY() const	{ return _y + _boss->getChildY(); }
	
//	virtual void setPos(int x, int y);
//	virtual void setSize(int w, int h);

	virtual void handleMouseDown(int x, int y, int button, int clickCount) {}
	virtual void handleMouseUp(int x, int y, int button, int clickCount) {}
	virtual void handleMouseEntered(int button) {}
	virtual void handleMouseLeft(int button) {}
	virtual void handleMouseMoved(int x, int y, int button) {}
	virtual void handleMouseWheel(int x, int y, int direction) {}
	virtual bool handleKeyDown(uint16 ascii, int keycode, int modifiers) { return false; }	// Return true if the event was handled
	virtual bool handleKeyUp(uint16 ascii, int keycode, int modifiers) { return false; }	// Return true if the event was handled
	virtual void handleTickle() {}

	void draw();
	void receivedFocus() { _hasFocus = true; receivedFocusWidget(); }
	void lostFocus() { _hasFocus = false; lostFocusWidget(); }
	virtual bool wantsFocus() { return false; }

	void setFlags(int flags)	{ _flags |= flags; }
	void clearFlags(int flags)	{ _flags &= ~flags; }
	int getFlags() const		{ return _flags; }

	void setEnabled(bool e)		{ if (e) setFlags(WIDGET_ENABLED); else clearFlags(WIDGET_ENABLED); }
	bool isEnabled() const		{ return _flags & WIDGET_ENABLED; }
	bool isVisible() const		{ return !(_flags & WIDGET_INVISIBLE); }

protected:
	virtual void drawWidget(bool hilite) {}

	virtual void receivedFocusWidget() {}
	virtual void lostFocusWidget() {}

	virtual Widget *findWidget(int x, int y) { return this; }

	void releaseFocus() { assert(_boss); _boss->releaseFocus(); }

	// By default, delegate unhandled commands to the boss
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) { assert(_boss); _boss->handleCommand(sender, cmd, data); }
};

/* StaticTextWidget */
class StaticTextWidget : public Widget {
protected:
	typedef Common::String String;
	typedef Graphics::TextAlignment TextAlignment;

	String					_label;
	TextAlignment			_align;
	const WidgetSize		_ws;
	const Graphics::Font	*_font;
public:
	StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, TextAlignment align, WidgetSize ws = kDefaultWidgetSize);
	void setValue(int value);
	void setLabel(const String &label);
	const String &getLabel() const		{ return _label; }
	void setAlign(TextAlignment align);
	TextAlignment getAlign() const		{ return _align; }

protected:
	void drawWidget(bool hilite);
};

/* ButtonWidget */
class ButtonWidget : public StaticTextWidget, public CommandSender {
	friend class Dialog;	// Needed for the hotkey handling
protected:
	uint32	_cmd;
	uint8	_hotkey;
public:
	ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd = 0, uint8 hotkey = 0, WidgetSize ws = kDefaultWidgetSize);

	void setCmd(uint32 cmd)				{ _cmd = cmd; }
	uint32 getCmd() const				{ return _cmd; }

	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); draw(); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); draw(); }

protected:
	void drawWidget(bool hilite);
};

/* CheckboxWidget */
class CheckboxWidget : public ButtonWidget {
protected:
	bool	_state;
public:
	CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd = 0, uint8 hotkey = 0, WidgetSize ws = kDefaultWidgetSize);

	void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleMouseEntered(int button)	{}
	virtual void handleMouseLeft(int button)	{}

	void setState(bool state);
	void toggleState()			{ setState(!_state); }
	bool getState() const		{ return _state; }

protected:
	void drawWidget(bool hilite);
};

/* SliderWidget */
class SliderWidget : public Widget, public CommandSender {
protected:
	uint32	_cmd;
	int		_value, _oldValue;
	int		_valueMin, _valueMax;
	bool	_isDragging;
	uint	_labelWidth;
public:
	SliderWidget(GuiObject *boss, int x, int y, int w, int h, uint32 cmd = 0);

	void setCmd(uint32 cmd)		{ _cmd = cmd; }
	uint32 getCmd() const		{ return _cmd; }

	void setValue(int value)	{ _value = value; }
	int getValue() const		{ return _value; }

	void setMinValue(int value)	{ _valueMin = value; }
	int getMinValue() const		{ return _valueMin; }
	void setMaxValue(int value)	{ _valueMax = value; }
	int getMaxValue() const		{ return _valueMax; }

	void handleMouseMoved(int x, int y, int button);
	void handleMouseDown(int x, int y, int button, int clickCount);
	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); draw(); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); draw(); }

protected:
	void drawWidget(bool hilite);

	int valueToPos(int value);
	int posToValue(int pos);
};

/* GraphicsWidget */
class GraphicsWidget : public Widget {
public:
	GraphicsWidget(GuiObject *boss, int x, int y, int w, int h);
	~GraphicsWidget();
	
	void setGfx(const Graphics::Surface *gfx);
protected:
	void drawWidget(bool hilite);
	
	Graphics::Surface _gfx;
};

} // End of namespace GUI

#endif
