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

#ifndef DIALOG_H
#define DIALOG_H

#include "common/scummsys.h"
#include "common/str.h"

#include "gui/object.h"

namespace GUI {

class NewGui;
class ButtonWidget;

// Some "common" commands sent to handleCommand()
enum {
	kCloseCmd = 'clos'
};

class Dialog : public GuiObject {
	friend class NewGui;
protected:
	Widget	*_mouseWidget;
	Widget  *_focusedWidget;
	bool	_visible;

private:
	int		_result;

public:
	Dialog(int x, int y, int w, int h)
		: GuiObject(x, y, w, h),
		  _mouseWidget(0), _focusedWidget(0), _visible(false) {
	}
	virtual ~Dialog();

	virtual int runModal();

	bool 	isVisible() const	{ return _visible; }

	void	releaseFocus();

protected:
	virtual void open();
	virtual void close();
	
	virtual void draw();
	virtual void drawDialog();

	virtual void handleTickle(); // Called periodically (in every guiloop() )
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleMouseWheel(int x, int y, int direction);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);
	virtual void handleKeyUp(uint16 ascii, int keycode, int modifiers);
	virtual void handleMouseMoved(int x, int y, int button);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	
	Widget *findWidget(int x, int y); // Find the widget at pos x,y if any

	ButtonWidget *addButton(int x, int y, const Common::String &label, uint32 cmd, char hotkey);

	void setResult(int result) { _result = result; }
	int getResult() const { return _result; }
};

} // End of namespace GUI

#endif
