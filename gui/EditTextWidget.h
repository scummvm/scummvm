/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
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

	int				_leftPadding;
	int				_rightPadding;

public:
	EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, uint32 cmd = 0);
	EditTextWidget(GuiObject *boss, const String &name, const String &text, uint32 cmd = 0);

	void setEditString(const String &str);

	virtual void handleMouseDown(int x, int y, int button, int clickCount);

	virtual bool wantsFocus() { return true; }

	virtual void reflowLayout();

protected:
	void drawWidget();
	void receivedFocusWidget();
	void lostFocusWidget();

	void startEditMode();
	void endEditMode();
	void abortEditMode();

	Common::Rect getEditRect() const;
};

} // End of namespace GUI

#endif
