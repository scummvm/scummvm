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

//#include	"src\driver96.h"
#include	"object.h"

//---------------------------------------------------------------------------------
#define	TOTAL_mouse_list	50

#define	MOUSE_normal		0
#define	MOUSE_top			1
#define	MOUSE_drag			2
#define	MOUSE_system_menu	3
#define	MOUSE_holding		4
//---------------------------------------------------------------------------------
// mouse unit - like Object_mouse, but with anim resource & pc (needed if sprite is to act as mouse detection mask)
typedef	struct
{
	int32	x1;				// top-left of mouse area is (x1,y1)
	int32	y1;
	int32	x2;				// bottom-right of area is (x2,y2)	(these coords are inclusive)
	int32	y2;
	int32	priority;
	int32	pointer;		// type (or resource id?) of pointer used over this area
	// up to here, this is basically a copy of the Object_mouse structure, but then we have...
	int32	id;				// object id, used when checking mouse list
	int32	anim_resource;	// resource id of animation file (if sprite to be used as mask) - otherwise 0
	int32	anim_pc;		// current frame number of animation
	int32	pointer_text;	// local id of text line to print when pointer highlights an object
} Mouse_unit;
//---------------------------------------------------------------------------------
extern	uint32	cur_mouse;
extern	Mouse_unit mouse_list[TOTAL_mouse_list];
extern	uint32	mouse_touching;
extern	uint32	mouse_mode;
extern	uint8	examining_menu_icon;

extern	uint32	mouse_status;	//human 0 on/1 off
extern	uint32	mouse_mode_locked;	//0 not !0 mode cannot be changed from normal mouse to top menu (i.e. when carrying big objects)

extern	uint32	real_luggage_item;	//last minute for pause mode

extern	uint32	pointerTextSelected;

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
void	Reset_mouse_list(void);	//Tony26Sept96

void	Normal_mouse(void);	//Tony30Sept96
void	Top_menu_mouse(void);	//Tony3Oct96
void	Drag_mouse(void);	//Tony21Nov96
void	System_menu(void);	//Tony19Mar97

void	Mouse_on_off(void);	//Tony30Sept96
uint32	Check_mouse_list(void);	//Tony30Sept96
void	Mouse_engine(void);	//Tony30Sept96

void	Set_mouse(uint32 res);
void	Set_luggage(uint32	res);	//Tony26Nov96

int32 FN_no_human(int32 *params);	//Tony30Sept96
int32 FN_add_human(int32 *params);	//Tony30Sept96

void ClearPointerText(void);		// James16jun97
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
#endif
