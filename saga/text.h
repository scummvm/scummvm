/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Text / dialogue display management module private header

#ifndef SAGA_TEXT_H__
#define SAGA_TEXT_H__

#include "saga/list.h"

namespace Saga {

#define TEXT_CENTERLIMIT 50
#define TEXT_MARGIN 10
#define TEXT_LINESPACING 2


enum TEXT_FLAGS {
	TEXT_TIMEOUT = 0x01
};

struct TEXTLIST_ENTRY {
	int display;
	int id;
	int text_x;
	int text_y;
	int color;
	int effect_color;
	int flags;
	int font_id;
	long time;
	const char *string;
	TEXTLIST_ENTRY() { memset(this, 0, sizeof(*this)); }
};

typedef SortedList<TEXTLIST_ENTRY> TEXTLIST;

#define TEXTLISTITERATOR TEXTLIST::iterator

}				// End of namespace Saga
#endif
