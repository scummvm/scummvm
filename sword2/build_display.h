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

#ifndef	_BUILD_DISPLAY
#define	_BUILD_DISPLAY

#include "bs2/driver/driver96.h"

namespace Sword2 {

// structure filled out by each object to register its graphic printing
// requrements

typedef	struct {
	int16 x;
	int16 y;
	uint16 scaled_width;
	uint16 scaled_height;
	int16 sort_y;
	uint32 anim_resource;
	uint16 anim_pc;

	// denotes a scaling sprite at print time - and holds the scaling
	// value for the shrink routine

	uint16 scale;

	// non zero means this item is a layer - retrieve from background
	// layer and send to special renderer

	uint16 layer_number;

	// non zero means we want this frame to be affected by the shading mask

	uint8 shadingFlag;

	// if none zero the shrinker should write coordinates to this
	// mouse_list number
	// uint32 write_mouse_list;
} buildit;

// declared externally so that debug.cpp can display these in the info

#define MAX_bgp0_sprites	6
#define MAX_bgp1_sprites	6
#define MAX_back_sprites	30
#define MAX_sort_sprites	30
#define MAX_fore_sprites	30
#define MAX_fgp0_sprites	6
#define MAX_fgp1_sprites	6

// declared externally so that debug.cpp can display these in the info

extern uint32 cur_bgp0;
extern uint32 cur_bgp1;
extern uint32 cur_back;
extern uint32 cur_sort;
extern uint32 cur_fore;
extern uint32 cur_fgp0;
extern uint32 cur_fgp1;

#ifdef _SWORD2_DEBUG
extern char largest_layer_info[128];
extern char largest_sprite_info[128];
#endif

// the only build list needed externally - by layers.cpp - for adding layers
// to sort list

extern	buildit	sort_list[];

// function prototypes needed externally

void Reset_render_lists(void);
void Build_display(void);
int32 FN_fade_down(int32 *params);
int32 FN_fade_up(int32 *params);
void Process_image(buildit *build_unit);
void DisplayMsg( uint8 *text, int time );
void RemoveMsg(void);
void SetFullPalette(int32 palRes);

// needed by debug.cpp for displaying as part of top-screen info

extern uint32 fps;

} // End of namespace Sword2

#endif
