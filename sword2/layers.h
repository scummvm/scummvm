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

#ifndef	_LAYERS
#define	_LAYERS

//#include "src\driver96.h"
#include "memory.h"








typedef	struct
{
	uint16	scroll_offset_x;		// position x
	uint16	scroll_offset_y;		// position y
	uint16	max_scroll_offset_x;	// calc'ed in FN_init_background
	uint16	max_scroll_offset_y;	//
	int16	player_feet_x;			// feet coordinates to use - cant just fetch the player compact anymore
	int16	player_feet_y;
	int16	feet_x;					// special offset-to-player position - tweek as desired - always set in screen manager object startup
	int16	feet_y;
	uint16	screen_wide;			// size of background layer - hense size of back buffer itself (Paul actually malloc's it)
	uint16	screen_deep;
	uint32	background_layer_id;	//id of the normal background layer
	uint16	number_of_layers;		// from the header of the main background layer
	uint8	new_palette;			// set to non zero to start the palette held within layer file fading up after a build_display
	uint8	scroll_flag;			// scroll mode 0 off 1 on
	uint8	mask_flag;				// using shading mask
} screen_info;


extern	screen_info	this_screen;


int32 FN_init_background(int32 *params);	// Tony11Sept96
void SetUpBackgroundLayers(void);			// James(13jun97)	called from control panel (as well as inside FN_init_background)



#endif
