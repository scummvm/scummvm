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

#ifndef	_SCRIPT_STRUCTURES
#define	_SCRIPT_STRUCTURES

namespace Sword2 {

// these structures represent the broken up compact components
// these here declared to the system must be the same as those declared to
// LINC (or it wont work)

// mouse structure - defines mouse detection area, detection priority &
// 'type' flag

typedef	struct {
	int32 x1;			// Top-left and bottom-right of mouse
	int32 y1;			// area. (These coords are inclusive.)
	int32 x2;
	int32 y2;
	int32 priority;
	int32 pointer;			// type (or resource id?) of pointer used over this area

} Object_mouse;

// logic structure - contains fields used in logic script processing

typedef	struct {
	int32 looping;			// 0 when first calling FN_<function>;
					// 1 when calling subsequent times in same loop
	int32 pause;			// pause count, used by FN_pause()
} Object_logic;

// status bits for 'type' field of Object_graphic)

// in low word:

#define	NO_SPRITE 	0x00000000	// don't print
#define	BGP0_SPRITE	0x00000001	// fixed to background parallax[0]
#define	BGP1_SPRITE	0x00000002	// fixed to background parallax[1]
#define	BACK_SPRITE	0x00000004	// 'background' sprite, fixed to main background
#define	SORT_SPRITE	0x00000008	// 'sorted' sprite, fixed to main background
#define	FORE_SPRITE	0x00000010	// 'foreground' sprite, fixed to main background
#define	FGP0_SPRITE	0x00000020	// fixed to foreground parallax[0]
#define	FGP1_SPRITE	0x00000040	// fixed to foreground parallax[0]

// in high word:

#define UNSHADED_SPRITE	0x00000000	// not to be shaded
#define SHADED_SPRITE	0x00010000	// to be shaded, based on shading mask

// graphic structure - contains fields appropriate to sprite output

typedef	struct {
	int32 type;			// see above
	int32 anim_resource;		// resource id of animation file
	int32 anim_pc;			// current frame number of animation
} Object_graphic;

// speech structure - contains fields used by speech scripts & text output

typedef	struct {
	int32 pen;			// colour to use for body of characters
	int32 width;			// max width of text sprite
	int32 command;			// speech script command id
	int32 ins1;			// speech script instruction parameters (may need more now?)
	int32 ins2;
	int32 ins3;
	int32 ins4;
	int32 ins5;
	int32 wait_state;		// 0 not waiting, 1 waiting for next speech command
} Object_speech;

// mega structure - contains fields used for mega-character & mega-set
// processing

typedef	struct {
	int32 NOT_USED_1;		// only free roaming megas need to check this before registering their graphics for drawing
	int32 NOT_USED_2;		// id of floor on which we are standing
	int32 NOT_USED_3;		// id of object which we are getting to
	int32 NOT_USED_4;		// pixel distance to stand from player character when in conversation
	int32 currently_walking;	// number given us by the auto router
	int32 walk_pc;			// current frame number of walk-anim
	int32 scale_a;			// current scale factors, taken from floor data
	int32 scale_b;
	int32 feet_x;			// mega feet coords - frame-offsets are added to these position mega frames
	int32 feet_y;
	int32 current_dir;		// current dirction faced by mega; used by autorouter to determine turns required
	int32 NOT_USED_5;		// means were currently avoiding a collision (see FN_walk)
	int32 megaset_res;		// resource id of mega-set file
	int32 NOT_USED_6;		// NOT USED
} Object_mega;

// walk-data structure - contains details of layout of frames in the
// mega-set, and how they are to be used

typedef struct {
	int32 nWalkFrames;		// no. of frames per walk-cycle
	int32 usingStandingTurnFrames;	// 0 = no  1 = yes
	int32 usingWalkingTurnFrames;	// 0 = no  1 = yes
	int32 usingSlowInFrames;	// 0 = no  1 = yes
	int32 usingSlowOutFrames;	// 0 = no  !0 = number of slow-out frames in each direction
	int32 nSlowInFrames[8];		// no. of slow-in frames in each direction
	int32 leadingLeg[8];		// leading leg for walk	in each direction  (0 = left  1 = right)
	int32 dx[8 * (12 + 1)];		// walk step distances in x direction
	int32 dy[8 * (12 + 1)];		// walk step distances in y direction
} Object_walkdata;

} // End of namespace Sword2

#endif
