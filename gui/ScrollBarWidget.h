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

#ifndef SCROLLBARWIDGET_H
#define SCROLLBARWIDGET_H

#include "widget.h"

enum {
	kScrollBarWidth		= 9
};


enum {
	kSetPositionCmd		= 'SETP'
};


class ScrollBarWidget : public Widget, public CommandSender {
protected:
	typedef enum {
		kNoPart,
		kUpArrowPart,
		kDownArrowPart,
		kSliderPart,
		kPageUpPart,
		kPageDownPart
	} Part;
	
	Part	_part;
	int		_sliderHeight;
	int		_sliderPos;

	Part	_draggingPart;
	int		_sliderDeltaMouseDownPos;

public:
	int		_numEntries;
	int		_entriesPerPage;
	int		_currentPos;

public:
	ScrollBarWidget(GuiObject *boss, int x, int y, int w, int h);

	void handleMouseDown(int x, int y, int button, int clickCount);
	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseWheel(int x, int y, int direction);
	void handleMouseMoved(int x, int y, int button);
	void handleMouseEntered(int button)	{ setFlags(WIDGET_HILITED); }
	void handleMouseLeft(int button)	{ clearFlags(WIDGET_HILITED); _part = kNoPart; draw(); }
	void handleTickle();

	// FIXME - this should be private, but then we also have to add accessors
	// for _numEntries, _entriesPerPage and _currentPos. This again leads to the question:
	// should these accessors force a redraw?
	void recalc();

protected:
	void drawWidget(bool hilite);
	void checkBounds(int old_pos);
};


#endif
