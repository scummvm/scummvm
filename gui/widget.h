/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#ifndef WIDGET_H
#define WIDGET_H

#include "scummsys.h"


class Dialog;

enum {
	WIDGET_ENABLED		= 1 << 0,
	WIDGET_INVISIBLE	= 1 << 1,
	WIDGET_HILITED		= 1 << 2,
	WIDGET_BORDER		= 1 << 3,
	WIDGET_CLEARBG		= 1 << 4,
	WIDGET_WANT_TICKLE	= 1 << 5,
	WIDGET_TRACK_MOUSE	= 1 << 6
};

enum {
	kStaticTextWidget	= 'TEXT',
	kButtonWidget		= 'BTTN',
	kCheckboxWidget		= 'CHKB',
	kSliderWidget		= 'SLDE',
	kListWidget			= 'LIST',
	kScrollBarWidget	= 'SCRB'
};

class CommandReceiver;
class CommandSender;

class CommandReceiver
{
friend class CommandSender;
protected:
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) = 0;
};

class CommandSender
{
protected:
	CommandReceiver	*_target;
public:
	CommandSender(CommandReceiver *target) : _target(target) {}

	void setTarget(CommandReceiver *target)	{ _target = target; }
	CommandReceiver *getTarget() const		{ return _target; }

	virtual void sendCommand(uint32 cmd, uint32 data)
	{
		if (_target && cmd)
			_target->handleCommand(this, cmd, data);
	}
};

/* Widget */
class Widget {
friend class Dialog;
protected:
	int			_type;
	Dialog		*_boss;
	Widget		*_next;
	int16		_x, _y;
	uint16		_w, _h;
	uint16		_id;
	int			_flags;
public:
	Widget(Dialog *boss, int x, int y, int w, int h);
	virtual ~Widget() {}

	virtual void handleClick(int x, int y, int button) {}
	virtual void handleMouseEntered(int button) {}
	virtual void handleMouseLeft(int button) {}
	virtual void handleMouseMoved(int x, int y, int button) {}
	virtual void handleKey(char key, int modifiers) {}
	void draw();

	void setFlags(int flags)	{ _flags |= flags; }
	void clearFlags(int flags)	{ _flags &= ~flags; }
	int getFlags() const		{ return _flags; }

protected:
	virtual void drawWidget(bool hilite) {}
};


/* StaticTextWidget */
class StaticTextWidget : public Widget {
protected:
	char	*_label;
public:
	StaticTextWidget(Dialog *boss, int x, int y, int w, int h, const char *text);
	~StaticTextWidget();
	void setLabel(const char *label);
	const char *getLabel() const	{ return _label; }

protected:
	void drawWidget(bool hilite);
};


/* ButtonWidget */
class ButtonWidget : public StaticTextWidget, public CommandSender {
	friend class Dialog;	// Needed for the hotkey handling
protected:
	uint32			_cmd;
	uint8			_hotkey;
public:
	ButtonWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd = 0, uint8 hotkey = 0);
	virtual ~ButtonWidget();

	void setCmd(uint32 cmd)					{ _cmd = cmd; }
	uint32 getCmd() const					{ return _cmd; }

	void handleClick(int x, int y, int button);
	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); draw(); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); draw(); }
};

/* CheckboxWidget */
class CheckboxWidget : public ButtonWidget {
protected:
	bool	_state;
public:
	CheckboxWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd = 0, uint8 hotkey = 0);
	void setState(bool state)	{ _state = state; }
	bool getState() const		{ return _state; }

	void handleClick(int x, int y, int button);
	virtual void handleMouseEntered(int button)	{}
	virtual void handleMouseLeft(int button)	{}

protected:
	void drawWidget(bool hilite);
};

/* SliderWidget */
class SliderWidget : public ButtonWidget {
protected:
	uint8	_value, _old_value;
public:
	SliderWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd = 0, uint8 hotkey = 0);
	void setValue(uint8 value)	{ _value = value; }
	uint8 getValue() const		{ return _value; }

	void handleMouseMoved(int x, int y, int button);

protected:
	void drawWidget(bool hilite);
};


#endif
