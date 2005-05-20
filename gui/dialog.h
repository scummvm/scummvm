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

#ifndef DIALOG_H
#define DIALOG_H

#include "common/scummsys.h"
#include "common/str.h"

#include "gui/object.h"
#include "gui/widget.h"

namespace GUI {

class NewGui;
class PopUpWidget;

// Some "common" commands sent to handleCommand()
enum {
	kCloseCmd = 'clos'
};

class Dialog : public GuiObject {
	friend class NewGui;
protected:
	Widget	*_mouseWidget;
	Widget  *_focusedWidget;
	Widget  *_dragWidget;
	bool	_visible;

private:
	int		_result;

public:
	Dialog(int x, int y, int w, int h);
	virtual ~Dialog();

	virtual int runModal();

	bool 	isVisible() const	{ return _visible; }

	void	releaseFocus();

	/**
	 * We can optionally scale dialogs by a factor of two. This is the
	 * default behaviour if the GUI is displayed on a 640x400 or bigger
	 * screen. However, some dialogs can cope with multiple screen sizes,
	 * and thus do not want automatic scaling.
	 *
	 * @return true if the dialog adjusts itself to the screen size
	 */
	virtual bool	wantsScaling() const;

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
	virtual void handleScreenChanged() {}
	
	Widget *findWidget(int x, int y); // Find the widget at pos x,y if any

	ButtonWidget *addButton(GuiObject *boss, int x, int y, const Common::String &label, uint32 cmd, char hotkey, WidgetSize ws = kDefaultWidgetSize);
	CheckboxWidget *addCheckbox(GuiObject *boss, int x, int y, const Common::String &label, uint32 cmd, char hotkey, WidgetSize ws = kDefaultWidgetSize);
	SliderWidget *addSlider(GuiObject *boss, int x, int y, uint32 cmd, WidgetSize ws = kDefaultWidgetSize);
	PopUpWidget *addPopUp(GuiObject *boss, int x, int y, int w, const Common::String &label, uint labelWidth = 0, WidgetSize ws = kDefaultWidgetSize);

	void setResult(int result) { _result = result; }
	int getResult() const { return _result; }
};

} // End of namespace GUI

#endif
