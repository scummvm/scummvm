/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef GUI_OBJECT_H
#define GUI_OBJECT_H

class CommandReceiver;
class CommandSender;

class CommandReceiver {
	friend class CommandSender;
protected:
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {}
};

class CommandSender {
	// TODO - allow for multiple targets, i.e. store targets in a list
	// and add methods addTarget/removeTarget.
protected:
	CommandReceiver	*_target;
public:
	CommandSender(CommandReceiver *target) : _target(target) {}

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
	int16		_x, _y;
	uint16		_w, _h;

	Widget		*_firstWidget;

public:
	GuiObject(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h), _firstWidget(0) { }

	virtual bool 	isVisible() const = 0;

	int16			getX() const	{ return _x; }
	int16			getY() const	{ return _y; }
	uint16			getW() const	{ return _w; }
	uint16			getH() const	{ return _h; }

protected:
	virtual void	releaseFocus() = 0;
};


#endif
