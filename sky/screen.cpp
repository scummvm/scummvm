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

#include "stdafx.h"
#include <string.h>
#include "common/scummsys.h"
#include "sky/skydefs.h"
#include "sky/sky.h"

#define	fade_jump	2
#define	scroll_jump	16

#define	vga_colours	256
#define	game_colours	240

uint8 top_16_colours[] =
{
    0, 0, 0,
    38, 38, 38,
    63, 63, 63,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    54, 54, 54,
    45, 47, 49,
    32, 31, 41,
    29, 23, 37,
    23, 18, 30,
    49, 11, 11,
    39, 5, 5,
    29, 1, 1,
    63, 63, 63
};

void SkyState::initialise_screen(void)
{
    int i;
    uint8 tmp_pal[1024];
    
    _system->init_size(full_screen_width, full_screen_height);
    _backscreen = (uint8 *)malloc(full_screen_width*full_screen_height);
    _game_grid = (uint8 *)malloc(GRID_X*GRID_Y*2);
    _work_palette = (uint8 *)malloc(vga_colours*3);

    //blank the first 240 colors of the palette 
    memset(tmp_pal, 0, game_colours * 4);   

    //set the remaining colors
    for (i=0; i<(vga_colours-game_colours); i++) {
	tmp_pal[game_colours+i*4] = (top_16_colours[i*3] << 2) + (top_16_colours[i*3] & 3);	
	tmp_pal[game_colours+i*4+1] = (top_16_colours[i*3+1] << 2) + (top_16_colours[i*3+1] & 3);
	tmp_pal[game_colours+i*4+2] = (top_16_colours[i*3+2] << 2) + (top_16_colours[i*3+2] & 3);
	tmp_pal[game_colours+i*4+3] = 0x00; 
    }

    //set the palette
    _system->set_palette(tmp_pal, 0, 256);

}

//set a new palette, pal is a pointer to dos vga rgb components 0..63
void SkyState::set_palette(uint8 *pal)
{
    convert_palette(pal, _palette);
    _system->set_palette(_palette, 0, 256);
}

void SkyState::convert_palette(uint8 *inpal, uint8* outpal) //convert 3 byte 0..63 rgb to 4byte 0..255 rgbx
{
    int i;

    for (i = 0; i < vga_colours; i++) {
	outpal[4*i] = (inpal[3*i] << 2) + (inpal[3*i] & 3); 
	outpal[4*i+1] = (inpal[3*i+1] << 2) + (inpal[3*i+1] & 3); 
	outpal[4*i+2] = (inpal[3*i+2] << 2) + (inpal[3*i+2] & 3); 
	outpal[4*i+3] = 0x00;	
    }
}
