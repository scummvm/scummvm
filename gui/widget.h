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

#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str.h"
#include "common/keyboard.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "gui/object.h"
#include "gui/ThemeEngine.h"
#include "common/text-to-speech.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/events.h"

namespace GUI {

class ScrollContainerWidget;

enum {
	WIDGET_ENABLED		= 1 <<  0,
	WIDGET_INVISIBLE	= 1 <<  1,
	WIDGET_HILITED		= 1 <<  2,
	WIDGET_BORDER		= 1 <<  3,
	WIDGET_PRESSED		= 1 <<	4,
	//WIDGET_INV_BORDER	= 1 <<  4,
	WIDGET_CLEARBG		= 1 <<  5,
	WIDGET_WANT_TICKLE	= 1 <<  7,
	WIDGET_TRACK_MOUSE	= 1 <<  8,
	// Retain focus on mouse up. By default widgets lose focus on mouseup,
	// but some widgets might want to retain it - widgets where you enter
	// text, for instance
	WIDGET_RETAIN_FOCUS	= 1 <<  9,
	// Usually widgets would lock mouse input when the user pressed the
	// left mouse button till the user releases it.
	// The PopUpWidget for example does not want this behavior, since the
	// mouse down will open up a new dialog which silently eats the mouse
	// up event for its own purposes.
	WIDGET_IGNORE_DRAG	= 1 << 10,
	WIDGET_DYN_TOOLTIP  = 1 << 11, // Widgets updates tooltip by coordinates
};

enum {
	kStaticTextWidget	= 'TEXT',
	kEditTextWidget		= 'EDIT',
	kButtonWidget		= 'BTTN',
	kCheckboxWidget		= 'CHKB',
	kRadiobuttonWidget	= 'RDBT',
	kSliderWidget		= 'SLDE',
	kListWidget			= 'LIST',
	kScrollBarWidget	= 'SCRB',
	kPopUpWidget		= 'POPU',
	kTabWidget			= 'TABW',
	kGraphicsWidget		= 'GFXW',
	kContainerWidget	= 'CTNR',
	kScrollContainerWidget = 'SCTR',
	kRichTextWidget		= 'RTXT',
	kEEWidget			= 'EEGG',
};

enum {
	kCaretBlinkTime = 300
};

enum {
	kPressedButtonTime = 200
};

enum {
	kPicButtonStateEnabled = 0,
	kPicButtonHighlight = 1,
	kPicButtonStateDisabled = 2,
	kPicButtonStatePressed = 3,

	kPicButtonStateMax = 3
};

/* Widget */
class Widget : public GuiObject {
	friend class Dialog;
protected:
	uint32		_type;
	GuiObject	*_boss;
	Widget		*_next;
	bool		_hasFocus;
	ThemeEngine::WidgetStateInfo _state;
	Common::U32String _tooltip;

private:
	uint16		_flags;
	bool		_needsRedraw;

public:
	static Widget *findWidgetInChain(Widget *start, int x, int y);
	static Widget *findWidgetInChain(Widget *start, const char *name);
	static bool containsWidgetInChain(Widget *start, Widget *search);

public:
	Widget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &tooltip = Common::U32String());
	Widget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &tooltip = Common::U32String());
	Widget(GuiObject *boss, const Common::String &name, const Common::U32String &tooltip = Common::U32String());
	~Widget() override;

	void init();

	void setNext(Widget *w) { _next = w; }
	void setBoss(GuiObject *newBoss) { _boss = newBoss; }
	Widget *next() { return _next; }

	int16	getAbsX() const override	{ return _x + _boss->getChildX(); }
	int16	getAbsY() const override	{ return _y + _boss->getChildY(); }

	virtual void setPos(int x, int y) { _x = x; _y = y; }
	virtual void setSize(int w, int h) { _w = w; _h = h; }

	/** Returns the minimal size the widget needs to have for its contents to fit */
	virtual void getMinSize(int &minWidth, int &minHeight) { minHeight = -1; minWidth = -1; }

	virtual void handleMouseDown(int x, int y, int button, int clickCount) {}
	virtual void handleMouseUp(int x, int y, int button, int clickCount) {}
	virtual void handleMouseEntered(int button) {}
	virtual void handleMouseLeft(int button) {}
	virtual void handleMouseMoved(int x, int y, int button) {}
	void handleMouseWheel(int x, int y, int direction) override { assert(_boss); _boss->handleMouseWheel(x, y, direction); }
	virtual bool handleKeyDown(Common::KeyState state) { return false; }	// Return true if the event was handled
	virtual bool handleKeyUp(Common::KeyState state) { return false; }	// Return true if the event was handled
	virtual void handleOtherEvent(const Common::Event &evt) {}
	virtual void handleTickle() {}

	/** Mark the widget and its children as dirty so they are redrawn on the next screen update */
	virtual void markAsDirty();

	/** Redraw the widget if it was marked as dirty, and recursively proceed with its children */
	virtual void draw();

	void receivedFocus() { _hasFocus = true; receivedFocusWidget(); }
	void lostFocus() { _hasFocus = false; lostFocusWidget(); }
	virtual bool wantsFocus() { return false; }

	uint32 getType() const { return _type; }

	void setFlags(int flags);
	void clearFlags(int flags);
	int getFlags() const		{ return _flags; }

	void setEnabled(bool e);
	bool isEnabled() const;

	void setVisible(bool e);
	bool isVisible() const override;

	bool useRTL() const;

	uint8 parseHotkey(const Common::U32String &label);
	Common::U32String cleanupHotkey(const Common::U32String &label);

	bool hasTooltip() const { return !_tooltip.empty(); }
	const Common::U32String &getTooltip() const { return _tooltip; }
	void setTooltip(const Common::U32String &tooltip) { _tooltip = tooltip; }
	void setTooltip(const Common::String &tooltip) { _tooltip = Common::U32String(tooltip); }

	virtual bool containsWidget(Widget *) const { return false; }

	void read(const Common::U32String &str);

protected:
	void updateState(int oldFlags, int newFlags);

	virtual void drawWidget() = 0;

	virtual void receivedFocusWidget() {}
	virtual void lostFocusWidget() {}

	virtual Widget *findWidget(int x, int y) { return this; }

	void releaseFocus() override { assert(_boss); _boss->releaseFocus(); }

	// By default, delegate unhandled commands to the boss
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override { assert(_boss); _boss->handleCommand(sender, cmd, data); }
};

/* StaticTextWidget */
class StaticTextWidget : public Widget {
protected:
	Common::U32String		_label;
	Graphics::TextAlign		_align;
	ThemeEngine::FontStyle	_font;
	ThemeEngine::FontColor  _fontColor;
	bool _useEllipsis;

public:
	StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &text, Graphics::TextAlign align, const Common::U32String &tooltip = Common::U32String(), ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold, Common::Language lang = Common::UNK_LANG, bool useEllipsis = true);
	StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &text, Graphics::TextAlign align, const Common::U32String &tooltip = Common::U32String(), ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold, Common::Language lang = Common::UNK_LANG, bool useEllipsis = true);
	StaticTextWidget(GuiObject *boss, const Common::String &name, const Common::U32String &text, const Common::U32String &tooltip = Common::U32String(), ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold, Common::Language lang = Common::UNK_LANG, bool useEllipsis = true);
	void setValue(int value);
	void setLabel(const Common::U32String &label);
	void handleMouseEntered(int button) override	{ readLabel(); }
	const Common::U32String &getLabel() const		{ return _label; }
	void setAlign(Graphics::TextAlign align);
	Graphics::TextAlign getAlign() const		{ return _align; }
	void readLabel() { read(_label); }
	void setFontColor(ThemeEngine::FontColor color);

protected:
	void reflowLayout() override;
	void drawWidget() override;
	void setFont(ThemeEngine::FontStyle font, Common::Language lang);
};

/* ButtonWidget */
class ButtonWidget : public StaticTextWidget, public CommandSender {
	friend class Dialog;	// Needed for the hotkey handling
protected:
	uint32	_cmd;
	uint8	_hotkey;
	uint8	_highresHotkey;
	uint8	_lowresHotkey;
	Common::U32String _lowresLabel;
public:
	ButtonWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0, const Common::U32String &lowresLabel = Common::U32String());
	ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0, const Common::U32String &lowresLabel = Common::U32String());
	ButtonWidget(GuiObject *boss, const Common::String &name, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0, const Common::U32String &lowresLabel = Common::U32String());

	void getMinSize(int &minWidth, int &minHeight) override;

	void setCmd(uint32 cmd)				{ _cmd = cmd; }
	uint32 getCmd() const				{ return _cmd; }

	void setLabel(const Common::U32String &label);
	void setLabel(const Common::String &label);
	void setLowresLabel(const Common::U32String &label);
	const Common::U32String &getLabel();

	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseEntered(int button) override	{ readLabel(); if (_duringPress) { setFlags(WIDGET_PRESSED); } else { setFlags(WIDGET_HILITED); } markAsDirty(); }
	void handleMouseLeft(int button) override	{ clearFlags(WIDGET_HILITED | WIDGET_PRESSED); markAsDirty(); }

	void setHighLighted(bool enable);
	void setPressedState();
	void setUnpressedState();
protected:
	void drawWidget() override;
	bool _duringPress;
};

/* DropdownButtonWidget */
class DropdownButtonWidget : public ButtonWidget {
public:
	DropdownButtonWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0, const Common::U32String &lowresLabel = Common::U32String());
	DropdownButtonWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0, const Common::U32String &lowresLabel = Common::U32String());
	DropdownButtonWidget(GuiObject *boss, const Common::String &name, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0, const Common::U32String &lowresLabel = Common::U32String());

	void handleMouseMoved(int x, int y, int button) override;
	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void reflowLayout() override;
	void getMinSize(int &minWidth, int &minHeight) override;


	void appendEntry(const Common::U32String &label, uint32 cmd);
	void clearEntries();

protected:
	struct Entry {
		Common::U32String label;
		uint32 cmd;
	};
	typedef Common::Array<Entry> EntryList;

	// Widget API
	void drawWidget() override;

	void reset();
	bool isInDropDown(int x, int y) const;

	EntryList _entries;

	uint32 _dropdownWidth;
	bool _inDropdown;
	bool _inButton;
};

/* PicButtonWidget */
class PicButtonWidget : public ButtonWidget {
public:
	PicButtonWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0);
	PicButtonWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0);
	PicButtonWidget(GuiObject *boss, const Common::String &name, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0);
	~PicButtonWidget() override;

	void setGfx(const Graphics::ManagedSurface *gfx, int statenum = kPicButtonStateEnabled, bool scale = true);
	void setGfx(const Graphics::Surface *gfx, int statenum = kPicButtonStateEnabled, bool scale = true);
	void setGfxFromTheme(const char *name, int statenum = kPicButtonStateEnabled, bool scale = true);
	void setGfx(int w, int h, int r, int g, int b, int statenum = kPicButtonStateEnabled);

	void setButtonDisplay(bool enable) {_showButton = enable; }

protected:
	void drawWidget() override;

	Graphics::ManagedSurface _gfx[kPicButtonStateMax + 1];
	Graphics::AlphaType _alphaType[kPicButtonStateMax + 1];
	bool _showButton;
};

/* CheckboxWidget */
class CheckboxWidget : public ButtonWidget {
protected:
	bool	_state;
	bool _overrideText;
	int _spacing;
public:
	CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0);
	CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0);
	CheckboxWidget(GuiObject *boss, const Common::String &name, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0, uint8 hotkey = 0);

	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseEntered(int button) override	{ readLabel(); setFlags(WIDGET_HILITED); markAsDirty(); }
	void handleMouseLeft(int button) override	{ clearFlags(WIDGET_HILITED); markAsDirty(); }

	void setState(bool state);
	void toggleState()			{ setState(!_state); }
	bool getState() const		{ return _state; }

	void setOverride(bool enable);

protected:
	void drawWidget() override;
};

class RadiobuttonWidget;

class RadiobuttonGroup : public CommandSender {
public:
	RadiobuttonGroup(GuiObject *boss, uint32 cmd = 0);
	~RadiobuttonGroup() override {}

	void addButton(RadiobuttonWidget *button) { _buttons.push_back(button); }
	Common::Array<RadiobuttonWidget *> getButtonList() const { return _buttons; }

	void setValue(int state);
	int getValue() const { return _value; }

	void setEnabled(bool ena);

	void setCmd(uint32 cmd)				{ _cmd = cmd; }
	uint32 getCmd() const				{ return _cmd; }

protected:
	Common::Array<RadiobuttonWidget *> _buttons;
	int _value;
	uint32	_cmd;
};

/* RadiobuttonWidget */
class RadiobuttonWidget : public ButtonWidget {
protected:
	bool	_state;
	int _spacing;
	int _value;

public:
	RadiobuttonWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, RadiobuttonGroup *group, int value, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint8 hotkey = 0);
	RadiobuttonWidget(GuiObject *boss, int x, int y, int w, int h, RadiobuttonGroup *group, int value, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint8 hotkey = 0);
	RadiobuttonWidget(GuiObject *boss, const Common::String &name, RadiobuttonGroup *group, int value, const Common::U32String &label, const Common::U32String &tooltip = Common::U32String(), uint8 hotkey = 0);

	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseEntered(int button) override	{ readLabel(); setFlags(WIDGET_HILITED); markAsDirty(); }
	void handleMouseLeft(int button) override	{ clearFlags(WIDGET_HILITED); markAsDirty(); }

	void setState(bool state, bool setGroup = true);
	void toggleState()			{ setState(!_state); }
	bool getState() const		{ return _state; }
	int getValue() const			{ return _value; }

protected:
	void drawWidget() override;

	RadiobuttonGroup *_group;
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
	SliderWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0);
	SliderWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0);
	SliderWidget(GuiObject *boss, const Common::String &name, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0);

	void setCmd(uint32 cmd)		{ _cmd = cmd; }
	uint32 getCmd() const		{ return _cmd; }

	void setValue(int value)	{ _value = value; }
	int getValue() const		{ return _value; }

	void setMinValue(int value)	{ _valueMin = value; }
	int getMinValue() const		{ return _valueMin; }
	void setMaxValue(int value)	{ _valueMax = value; }
	int getMaxValue() const		{ return _valueMax; }

	void handleMouseMoved(int x, int y, int button) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseEntered(int button) override	{ setFlags(WIDGET_HILITED); markAsDirty(); }
	void handleMouseLeft(int button) override	{ clearFlags(WIDGET_HILITED); markAsDirty(); }
	void handleMouseWheel(int x, int y, int direction) override;

protected:
	void drawWidget() override;

	int valueToPos(int value);
	int posToValue(int pos);
	int valueToBarWidth(int value);
};

/* GraphicsWidget */
class GraphicsWidget : public Widget {
public:
	GraphicsWidget(GuiObject *boss, int x, int y, int w, int h, bool scale, const Common::U32String &tooltip = Common::U32String());
	GraphicsWidget(GuiObject *boss, int x, int y, int w, int h, const Common::U32String &tooltip = Common::U32String());
	GraphicsWidget(GuiObject *boss, const Common::String &name, const Common::U32String &tooltip = Common::U32String());
	~GraphicsWidget() override;

	void setGfx(const Graphics::ManagedSurface *gfx, bool scale = false);
	void setGfx(const Graphics::Surface *gfx, bool scale = false);
	void setGfx(int w, int h, int r, int g, int b);
	void setGfxFromTheme(const char *name);

protected:
	void drawWidget() override;

	Graphics::ManagedSurface _gfx;
	Graphics::AlphaType _alphaType;
};

class PathWidget : public StaticTextWidget {
public:
	PathWidget(GuiObject *boss, int x, int y, int w, int h, bool scale,
			const Common::Path &path, Graphics::TextAlign align,
			const Common::U32String &placeholder = Common::U32String(),
			const Common::U32String &tooltip = Common::U32String(),
			ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold,
			bool useEllipsis = true) :
		StaticTextWidget(boss, x, y, w, h, scale,
				path.empty() ? placeholder : Common::U32String(path.toString(Common::Path::kNativeSeparator)),
				align, tooltip, font, Common::UNK_LANG, useEllipsis),
		_path(path),
		_placeholder(placeholder) { }
	PathWidget(GuiObject *boss, int x, int y, int w, int h,
			const Common::Path &path, Graphics::TextAlign align,
			const Common::U32String &placeholder = Common::U32String(),
			const Common::U32String &tooltip = Common::U32String(),
			ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold,
			bool useEllipsis = true) :
		StaticTextWidget(boss, x, y, w, h,
				path.empty() ? placeholder : Common::U32String(path.toString(Common::Path::kNativeSeparator)),
				align, tooltip, font, Common::UNK_LANG, useEllipsis),
		_path(path),
		_placeholder(placeholder) {}
	PathWidget(GuiObject *boss, const Common::String &name,
			const Common::Path &path,
			const Common::U32String &placeholder = Common::U32String(),
			const Common::U32String &tooltip = Common::U32String(),
			ThemeEngine::FontStyle font = ThemeEngine::kFontStyleBold,
			bool useEllipsis = true) :
		StaticTextWidget(boss, name,
				path.empty() ? placeholder : Common::U32String(path.toString(Common::Path::kNativeSeparator)),
				tooltip, font, Common::UNK_LANG, useEllipsis),
		_path(path),
		_placeholder(placeholder) {}
	void setLabel(const Common::Path &path) {
		_path = path;
		if (path.empty()) {
			StaticTextWidget::setLabel(_placeholder);
		} else {
			StaticTextWidget::setLabel(path.toString(Common::Path::kNativeSeparator));
		}
	}
	const Common::Path &getLabel() const { return _path; }
	void setEmptyPlaceHolder(const Common::U32String &placeholder) { _placeholder = placeholder; }
protected:
	Common::Path _path;
	Common::U32String _placeholder;
};

/* ContainerWidget */
class ContainerWidget : public Widget {
public:
	ContainerWidget(GuiObject *boss, int x, int y, int w, int h, bool scale = false);
	ContainerWidget(GuiObject *boss, const Common::String &name);
	~ContainerWidget() override;

	bool containsWidget(Widget *) const override;
	Widget *findWidget(int x, int y) override;
	void removeWidget(Widget *widget) override;

	void setBackgroundType(ThemeEngine::WidgetBackground backgroundType);
protected:
	void drawWidget() override;

	ThemeEngine::WidgetBackground _backgroundType;
};

/* OptionsContainerWidget */
class OptionsContainerWidget : public Widget {
public:
	/**
	 * @param widgetsBoss  parent widget for the container widget
	 * @param name         name of the container widget in the layout system
	 * @param dialogLayout name of the layout used by the contained widgets, empty string for manually laid out widgets
	 * @param scrollable   whether the container is made scrollable through a ScrollContainerWidget
	 * @param domain       the configuration manager domain this widget is meant to edit
	 */
	OptionsContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &dialogLayout,
	                       const Common::String &domain);
	~OptionsContainerWidget() override;

	/** Implementing classes should (re)initialize their widgets with state from the configuration domain */
	virtual void load() = 0;

	/**
	 * Implementing classes should save their widget's state to the configuration domain
	 *
	 * @return true if changes were made to the configuration since the last call to load()
	 */
	virtual bool save() = 0;

	/** Implementing classes should return if there are relevant keys set in the configuration domain
	 *
	 * @return true if there are relevant keys set in the configuration domain
	 */
	virtual bool hasKeys() { return true; }

	/** Implementing classes should enable or disable all active widgets */
	virtual void setEnabled(bool e) {}

	void setParentDialog(Dialog *parentDialog) { _parentDialog = parentDialog; }
	void setDomain(const Common::String &domain) { _domain = domain; }

protected:
	// Widget API
	void reflowLayout() override;
	void drawWidget() override {}
	bool containsWidget(Widget *widget) const override;
	Widget *findWidget(int x, int y) override;
	void removeWidget(Widget *widget) override;

	/** The pareent object to use when creating child widgets */
	GuiObject *widgetsBoss() { return this; }

	/**
	 * Child classes can override this method to define the layout used by the contained widgets in the layout system
	 *
	 * This is called only when the layout was not found in the theme definition files.
	 */
	virtual void defineLayout(ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {}

	Common::String _domain;
	const Common::String _dialogLayout;

	Dialog *_parentDialog;
};

ButtonWidget *addClearButton(GuiObject *boss, const Common::String &name, uint32 cmd, int x=0, int y=0, int w=0, int h=0, bool scale = false);
const Graphics::ManagedSurface *scaleGfx(const Graphics::ManagedSurface *gfx, int w, int h, bool filtering = false);

} // End of namespace GUI

#endif
