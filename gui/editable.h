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

#ifndef GUI_EDITABLE_H
#define GUI_EDITABLE_H

#include "common/str.h"
#include "common/rect.h"
#include "gui/widget.h"

namespace GUI {


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

public:
	EditableWidget(GuiObject *boss, int x, int y, int w, int h);
	virtual ~EditableWidget();

	virtual void handleTickle();

protected:
	virtual void startEditMode() = 0;
	virtual void endEditMode() = 0;
	virtual void abortEditMode() = 0;

	virtual Common::Rect getEditRect() const = 0;
	virtual int getCaretOffset() const = 0;
	void drawCaret(bool erase);
};

} // End of namespace GUI

#endif
