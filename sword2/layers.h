/* Copyright (C) 1994-2004 Revolution Software Ltd
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

namespace Sword2 {

struct ScreenInfo {
	uint16 scroll_offset_x;		// Position x
	uint16 scroll_offset_y;		// Position y
	uint16 max_scroll_offset_x;	// Calc'ed in fnInitBackground
	uint16 max_scroll_offset_y;
	int16 player_feet_x;		// Feet coordinates to use - cant just
	int16 player_feet_y;		// fetch the player compact anymore
	int16 feet_x;			// Special offset-to-player position -
	int16 feet_y;			// tweek as desired - always set in
					// screen manager object startup
	uint16 screen_wide;		// Size of background layer - hence
	uint16 screen_deep;		// size of back buffer itself (Paul
					// actually malloc's it)
	uint32 background_layer_id;	// Id of the normal background layer
					// from the header of the main
					// background layer
	uint16 number_of_layers;
	uint8 new_palette;		// Set to non zero to start the
					// palette held within layer file
					// fading up after a build_display
	uint8 scroll_flag;		// Scroll mode 0 off 1 on
	uint8 mask_flag;		// Using shading mask
};

} // End of namespace Sword2

#endif
