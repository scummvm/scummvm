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

#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include "widget.h"

// FIXME - use own list class later, this is for rapid development
//#include <string>
//#include <vector>
//typedef	std::vector<std::string>	StringList;
typedef int StringList;	// FIXME placeholder


enum {
	kListNumberingOff	= -1,
	kListNumberingZero	= 0,
	kListNumberingOne	= 1
};

/* ListWidget */
class ListWidget : public Widget {
protected:
	StringList	_list;
	bool		_editable;
	int			_numberingMode;
	int			_currentPos;
	
public:
	ListWidget(Dialog *boss, int x, int y, int w, int h);
	virtual ~ListWidget();
	
	void setList(const StringList& list)	{ _list = list; }
	const StringList& getList()	const		{ return _list; }
	
	void setNumberingMode(int numberingMode)	{ _numberingMode = numberingMode; }
	
	virtual void handleClick(int button);
	virtual void handleMouseMoved(int x, int y, int button);
	virtual void handleKey(char key, int modifiers);

protected:
	void drawWidget(bool hilite);
};

#endif
