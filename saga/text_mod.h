/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
/*
 Description:   
 
    Text / dialogue display management module public header

 Notes: 
*/

#ifndef SAGA_TEXT_MOD_H_
#define SAGA_TEXT_MOD_H_

namespace Saga {

enum R_TEXT_FLAGS {
	TEXT_TIMEOUT = 0x01
};

struct R_TEXTLIST_ENTRY {
	struct R_TEXTLIST_ENTRY_tag *next;
	struct R_TEXTLIST_ENTRY_tag *prev;
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
	
	R_TEXTLIST_ENTRY() { memset(this, 0, sizeof(*this)); }
};

typedef struct R_TEXTLIST_tag R_TEXTLIST;

R_TEXTLIST *TEXT_CreateList(void);
void TEXT_DestroyList(R_TEXTLIST *textlist);
void TEXT_ClearList(R_TEXTLIST *textlist);
int TEXT_DrawList(R_TEXTLIST *textlist, R_SURFACE *ds);
R_TEXTLIST_ENTRY *TEXT_AddEntry(R_TEXTLIST *textlist, R_TEXTLIST_ENTRY *entry);
int TEXT_DeleteEntry(R_TEXTLIST *textlist, R_TEXTLIST_ENTRY *entry);
int TEXT_SetDisplay(R_TEXTLIST_ENTRY *entry, int val);
int TEXT_Draw(int font_id, R_SURFACE *ds, const char *string, int text_x, 
			  int text_y, int color, int effect_color, int flags);
int TEXT_ProcessList(R_TEXTLIST *textlist, long ms);

}				// End of namespace Saga

#endif				/* SAGA_TEXT_MOD_H_ */
