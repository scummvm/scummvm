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

#ifndef GUI_EDITTEXTWIDGET_H
#define GUI_EDITTEXTWIDGET_H

#include "gui/editable.h"
#include "common/str.h"

namespace GUI {

/* EditTextWidget */
class EditTextWidget : public EditableWidget {
protected:
	typedef Common::String String;

	String		_backupString;

public:
	EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, WidgetSize ws = kNormalWidgetSize);

	void setEditString(const String &str);

	virtual void handleMouseDown(int x, int y, int button, int clickCount);

	virtual bool wantsFocus() { return true; }

protected:
	void drawWidget(bool hilite);
	void receivedFocusWidget();
	void lostFocusWidget();

	void startEditMode();
	void endEditMode();
	void abortEditMode();

	Common::Rect getEditRect() const;
};

} // End of namespace GUI

#endif
