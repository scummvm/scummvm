/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "struc.h"

//This file is incomplete, several flags still missing.

#define KEY_BUFFER_SIZE	80
#define SEQUENCE_COUNT	3

//screen/grid defines
#define GAME_SCREEN_WIDTH	320
#define GAME_SCREEN_HEIGHT	192
#define FULL_SCREEN_WIDTH	320
#define FULL_SCREEN_HEIGHT	200

#define TOT_NO_GRIDS	70	//total no. of grids supported
#define GRID_SIZE	120	//grid size in bytes

#define GRID_X	20
#define GRID_Y	24
#define GRID_W	16
#define GRID_H	8

#define GRID_W_SHIFT	4
#define GRID_H_SHIFT	3

#define TOP_LEFT_X	128
#define TOP_LEFT_Y	136

//item list defines
#define section_0_item	119

#define c_base_mode	0
#define c_base_mode56	56
#define c_action_mode	4
#define c_sp_colour	90
#define c_mega_set	112
#define c_grid_width	114

//#define next_mega_set

#define send_sync	-1
#define lf_start_fx	-2
#define safe_start_screen	0

//autoroute defines
#define upy	0
#define downy	1
#define lefty	2
#define righty	3

#define route_space	64

#define l_script	1
#define l_ar	2
#define l_ar_anim	3
#define l_ar_turning	4
#define l_alt	5
#define l_mod_animate	6
#define l_turning	7
#define l_cursor	8
#define l_talk	9
#define l_listen	10
#define l_stopped	11
#define l_choose	12
#define l_frames	13
#define l_pause	14
#define l_wait_synch	15
#define l_simple_mod	16
