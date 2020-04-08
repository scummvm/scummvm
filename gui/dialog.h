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

#ifndef GUI_DIALOG_H
#define GUI_DIALOG_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/keyboard.h"

#include "gui/object.h"
#include "gui/ThemeEngine.h"

namespace Common {
struct Event;
}

namespace GUI {

class EventRecorder;

class Widget;

// Some "common" commands sent to handleCommand()
enum {
	kCloseWithResultCmd  = 'clsr',
	kCloseCmd            = 'clos',
	kOKCmd               = 'ok  '
};

class Dialog : public GuiObject {
	friend class GuiManager;
	friend class EventRecorder;
	friend class Tooltip;
protected:
	Widget	*_mouseWidget;
	Widget  *_focusedWidget;
	Widget  *_dragWidget;
	Widget 	*_tickleWidget;
	bool	_visible;

	ThemeEngine::DialogBackground _backgroundType;

private:
	int		_result;

public:
	Dialog(int x, int y, int w, int h);
	Dialog(const Common::String &name);

	virtual int runModal();

	bool	isVisible() const override	{ return _visible; }

	void	releaseFocus() override;
	void	setFocusWidget(Widget *widget);
	Widget *getFocusWidget() { return _focusedWidget; }

	void setTickleWidget(Widget *widget) { _tickleWidget = widget; }
	void unSetTickleWidget() { _tickleWidget = nullptr; }
	Widget *getTickleWidget() { return _tickleWidget; }

	void reflowLayout() override;
	virtual void lostFocus();
	virtual void receivedFocus(int x = -1, int y = -1) { if (x >= 0 && y >= 0) handleMouseMoved(x, y, 0); }

protected:
	virtual void open();
	virtual void close();

	/** Recursively mark all the widgets in this dialog as dirty so they are redrawn */
	void markWidgetsAsDirty();

	/** Draw the dialog in its entirety (background and widgets) */
	virtual void drawDialog(DrawLayer layerToDraw);

	/** Draw only the dialog's widgets */
	void drawWidgets();

	virtual void handleTickle(); // Called periodically (in every guiloop() )
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleMouseWheel(int x, int y, int direction);
	virtual void handleKeyDown(Common::KeyState state);
	virtual void handleKeyUp(Common::KeyState state);
	virtual void handleMouseMoved(int x, int y, int button);
	virtual void handleMouseLeft(int button) {}
	virtual void handleOtherEvent(const Common::Event &evt) {}
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	Widget *findWidget(int x, int y); // Find the widget at pos x,y if any
	Widget *findWidget(const char *name);
	void removeWidget(Widget *widget) override;

	void setDefaultFocusedWidget();

	void setResult(int result) { _result = result; }
	int getResult() const { return _result; }
};

} // End of namespace GUI

#endif
