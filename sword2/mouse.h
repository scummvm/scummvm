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

#ifndef MOUSE_H
#define MOUSE_H

#define	TOTAL_mouse_list	50

namespace Sword2 {

enum {
	MOUSE_normal		= 0,	// normal in game
	MOUSE_menu		= 1,	// menu chooser
	MOUSE_drag		= 2,	// dragging luggage
	MOUSE_system_menu	= 3,	// system menu chooser
	MOUSE_holding		= 4	// special
};

// The MOUSE_holding mode is entered when the conversation menu is closed, and
// exited when the mouse cursor moves off that menu area. I don't know why yet.
 
// mouse unit - like ObjectMouse, but with anim resource & pc (needed if
// sprite is to act as mouse detection mask)

struct MouseUnit {
	// Top-left and bottom-right of mouse area. These coords are inclusive
	int32 x1;
	int32 y1;
	int32 x2;
	int32 y2;

	int32 priority;

	// type (or resource id?) of pointer used over this area
	int32 pointer;

	// up to here, this is basically a copy of the ObjectMouse
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
};

} // End of namespace Sword2

#endif
