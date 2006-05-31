/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#ifndef GUI_EDITABLE_H
#define GUI_EDITABLE_H

#include "common/str.h"
#include "common/rect.h"
#include "gui/widget.h"
#include "gui/newgui.h"

namespace GUI {

/**
 * Base class for widgets which need to edit text, like ListWidget and
 * EditTextWidget.
 */
class EditableWidget : public Widget {
public:
	typedef Common::String String;
protected:
	String		_editString;

	bool		_caretVisible;
	uint32		_caretTime;
	int			_caretPos;

	bool		_caretInverse;

	int			_editScrollOffset;

	Theme::kFontStyle  _font;

public:
	EditableWidget(GuiObject *boss, int x, int y, int w, int h);
	EditableWidget(GuiObject *boss, String name);
	virtual ~EditableWidget();

	void init();

	virtual void setEditString(const String &str);
	virtual const String &getEditString() const		{ return _editString; }

	virtual void handleTickle();
	virtual bool handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:
	virtual void startEditMode() = 0;
	virtual void endEditMode() = 0;
	virtual void abortEditMode() = 0;

	virtual Common::Rect getEditRect() const = 0;
	virtual int getCaretOffset() const;
	void drawCaret(bool erase);
	bool setCaretPos(int newPos);
	bool adjustOffset();
	void makeCaretVisible();

	void setFontStyle(Theme::kFontStyle font) { _font = font; }

	virtual bool tryInsertChar(char c, int pos);
};

} // End of namespace GUI

#endif
