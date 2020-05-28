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

#ifndef GUI_OBJECT_H
#define GUI_OBJECT_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/rect.h"

namespace GUI {

class CommandSender;

class CommandReceiver {
	friend class CommandSender;
protected:
	virtual ~CommandReceiver() {}
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {}
};

class CommandSender {
	// TODO - allow for multiple targets, i.e. store targets in a list
	// and add methods addTarget/removeTarget.
protected:
	CommandReceiver	*_target;
public:
	CommandSender(CommandReceiver *target) : _target(target) {}
	virtual ~CommandSender() {}

	void setTarget(CommandReceiver *target)	{ _target = target; }
	CommandReceiver *getTarget() const		{ return _target; }

	virtual void sendCommand(uint32 cmd, uint32 data) {
		if (_target && cmd)
			_target->handleCommand(this, cmd, data);
	}
};

class Widget;

class GuiObject : public CommandReceiver {
	friend class Widget;
protected:
	Common::Rect _textDrawableArea;

	int16		_x, _y;
	uint16		_w, _h;
	bool		_useRTL;
	const Common::String _name;

	Widget		*_firstWidget;

public:
	GuiObject(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h), _useRTL(true), _firstWidget(nullptr) { }
	GuiObject(const Common::String &name);
	~GuiObject() override;

	virtual void setTextDrawableArea(const Common::Rect &r) { _textDrawableArea = r; }

	virtual int16	getRelX() const		{ return _x; }
	virtual int16	getRelY() const		{ return _y; }
	virtual int16	getAbsX() const		{ return _x; }
	virtual int16	getAbsY() const		{ return _y; }
	virtual int16	getChildX() const	{ return getAbsX(); }
	virtual int16	getChildY() const	{ return getAbsY(); }
	virtual uint16	getWidth() const	{ return _w; }
	virtual uint16	getHeight() const	{ return _h; }

	virtual bool	isVisible() const = 0;

	virtual void	reflowLayout();

	virtual void	removeWidget(Widget *widget);

	virtual bool	isPointIn(int x, int y) {
		return (x >= _x && x < (_x + _w) && (y >= _y) && (y < _y + _h));
	}

	/**
	 * Returns the clipping rect to be used when drawing the children widgets of this object
	 */
	virtual Common::Rect getClipRect() const;

protected:
	virtual void	releaseFocus() = 0;
};

} // End of namespace GUI

#endif
