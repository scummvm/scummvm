/* Copyright (C) 1994-2003 Revolution Software Ltd
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

//mouse stuff

#ifndef MOUSE_H
#define MOUSE_H

#include "object.h"

#define	TOTAL_mouse_list	50

#define	MOUSE_normal		0
#define	MOUSE_top		1
#define	MOUSE_drag		2
#define	MOUSE_system_menu	3
#define	MOUSE_holding		4

// mouse unit - like Object_mouse, but with anim resource & pc (needed if
// sprite is to act as mouse detection mask)

typedef	struct {
	// Top-left and bottom-right of mouse area. These coords are inclusive
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;

	int32 priority;

	// type (or resource id?) of pointer used over this area
	int32 pointer;

	// up to here, this is basically a copy of the Object_mouse
	// structure, but then we have...

	// object id, used when checking mouse list
	int32 id;

	// resource id of animation file (if sprite to be used as mask) -
	// otherwise 0
	int32 anim_resource;

	// current frame number of animation
	int32 anim_pc;

	// local id of text line to print when pointer highlights an object
	int32 pointer_text;
} Mouse_unit;

extern uint32 cur_mouse;
extern Mouse_unit mouse_list[TOTAL_mouse_list];
extern uint32 mouse_touching;
extern uint32 mouse_mode;
extern uint8 examining_menu_icon;

// human 0 on/1 off
extern uint32 mouse_status;

// 0 not !0 mode cannot be changed from normal mouse to top menu (i.e. when
// carrying big objects)
extern uint32 mouse_mode_locked;

//last minute for pause mode
extern uint32 real_luggage_item;

extern uint32 pointerTextSelected;

void Reset_mouse_list(void);

void Normal_mouse(void);
void Top_menu_mouse(void);
void Drag_mouse(void);
void System_menu(void);

void Mouse_on_off(void);
uint32 Check_mouse_list(void);
void Mouse_engine(void);

void Set_mouse(uint32 res);
void Set_luggage(uint32 res);

int32 FN_no_human(int32 *params);
int32 FN_add_human(int32 *params);

void ClearPointerText(void);

#endif
