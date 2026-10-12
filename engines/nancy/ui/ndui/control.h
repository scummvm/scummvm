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

#ifndef NANCY_UI_NDUI_CONTROL_H
#define NANCY_UI_NDUI_CONTROL_H

#include "common/array.h"

#include "engines/nancy/ui/ndui/element.h"

namespace Nancy {
namespace UI {
namespace NDUI {

class Panel;

// A widget inside a panel, drawn with its elements. The player's clicks, drags and key
// presses turn into events, and each event fires the actions bound to it in the NDUI
// data; the panel hands the control the actions that name it.
class Control {
public:
	Control(Panel *panel) : _panel(panel) {}
	virtual ~Control() { delete _eventHandler; }

	void read(Common::SeekableReadStream &stream);

	// Nancy19: the control's events go to the handler, which the control takes over
	void setEventHandler(EventHandler *handler) { delete _eventHandler; _eventHandler = handler; }

	// Returns true when the action was meant for this control
	bool takeAction(const Action &action);

	// Advances the color blending; returns true when the control looks different
	virtual bool update(float seconds) = 0;
	virtual void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const = 0;

	// The mouse, in the coordinates of the panel's controls. A control that returns
	// true from onMouseDown() keeps getting the mouse until the button is released.
	virtual bool containsPoint(const Common::Point &pos) const { return false; }

	// The area the control draws to, which can reach past its bounds
	virtual Common::Rect getDrawBounds() const { return _bounds; }
	virtual void onMouseHover(const Common::Point &pos) {}
	virtual bool onMouseDown(const Common::Point &pos) { return false; }
	virtual void onMouseMove(const Common::Point &pos) {}
	virtual void onMouseUp(const Common::Point &pos) {}
	virtual bool onWheel(int notches) { return false; }

	void setVisible(bool visible);
	void setEnabled(bool enabled);
	void setText(const Common::String &text);

	const Common::String &getName() const { return _name; }
	void setName(const Common::String &name) { _name = name; }
	const Common::String &getTooltip() const { return _tooltip; }
	bool isVisible() const { return _visible; }
	bool isEnabled() const { return _enabled; }
	int32 getInitialState() const { return _initialState; }

	// Fires the actions bound to an event; the ones without a value of their own
	// get the given one
	void fireEvents(int32 eventType, bool isOwnEvent, double value, const Common::String &text);

	int _id = 0;
	Common::Rect _bounds;		// relative to the panel
	bool _isMouseOver = false;
	bool _isPressed = false;

protected:
	virtual void readExtra(Common::SeekableReadStream &stream) {}
	virtual bool doAction(const Action &action);

	// Fires the events for something the player did to the control
	void sendEvent(int32 eventType);
	virtual bool getValue(int32 eventType, double &value, Common::String &text) const { return false; }
	virtual bool isOwnEvent(int32 eventType) const { return false; }

	ControlState getState() const;
	void drawSprite(Graphics::ManagedSurface &surface, uint element, const Common::Rect &dest) const;
	void drawSprite(Graphics::ManagedSurface &surface, uint element, const Common::Rect &src, const Common::Rect &dest) const;

	// The width the text gets wrapped to
	virtual int getTextWidth() const { return _bounds.width(); }
	virtual int getMinHeight() const { return 0; }
	void fitHeightToText();

	Panel *_panel;
	Common::String _name;
	Common::String _text;
	Common::String _tooltip;	// Nancy17
	int32 _initialState = kStartVisible;
	bool _visible = true;
	bool _enabled = true;
	bool _autoHeight = false;	// controls without a height are as tall as their text
	Common::Array<Action> _events;
	EventHandler *_eventHandler = nullptr;
	Common::Array<Element> _elements;
};

// Draws its element and text while visible; used for panel backgrounds, labels and pictures
class Static : public Control {
public:
	Static(Panel *panel) : Control(panel) {}

	bool update(float seconds) override;
	void draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const override;

protected:
	bool doAction(const Action &action) override;
};

// Nancy17. Every panel has one, to show the tooltip of the control under the mouse.
// It comes up once the mouse has been over the control for a while, and gives up
// after another while.
class Tooltip : public Static {
public:
	Tooltip(Panel *panel) : Static(panel) {}

	bool isTimeToShow(float hoverSeconds) const { return hoverSeconds >= _showAfter && hoverSeconds <= _hideAfter; }
	bool isPrevented() const;

	// Draws the tooltip into a surface that gets just large enough for it
	void render(Graphics::ManagedSurface &surface, const Common::String &text);

protected:
	void readExtra(Common::SeekableReadStream &stream) override;

	double _showAfter = 0;
	double _hideAfter = 0;
	int16 _preventFlag = -1;
};

// A static that shows a number
class VarStatic : public Static {
public:
	VarStatic(Panel *panel) : Static(panel) {}

	// Nancy19: the text that goes before and after the number
	void setAffixes(const Common::String &prefix, const Common::String &suffix) { _prefix = prefix; _suffix = suffix; }

protected:
	void readExtra(Common::SeekableReadStream &stream) override;
	bool doAction(const Action &action) override;

	int32 _width = 0;
	int32 _precision = 0;
	Common::String _prefix;
	Common::String _suffix;
};

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NDUI_CONTROL_H
