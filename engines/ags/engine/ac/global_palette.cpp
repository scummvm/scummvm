/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ac/common.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_palette.h"

extern GameSetupStruct game;
extern GameState play;
extern color palette[256];


void CyclePalette(int strt,int eend) {
    // hi-color game must invalidate screen since the palette changes
    // the effect of the drawing operations
    if (game.color_depth > 1)
        invalidate_screen();

    if ((strt < 0) || (strt > 255) || (eend < 0) || (eend > 255))
        quit("!CyclePalette: start and end must be 0-255");

    if (eend > strt) {
        // forwards
        wcolrotate(strt, eend, 0, palette);
        set_palette_range(palette, strt, eend, 0);
    }
    else {
        // backwards
        wcolrotate(eend, strt, 1, palette);
        set_palette_range(palette, eend, strt, 0);
    }

}
void SetPalRGB(int inndx,int rr,int gg,int bb) {
    if (game.color_depth > 1)
        invalidate_screen();

    wsetrgb(inndx,rr,gg,bb,palette);
    set_palette_range(palette, inndx, inndx, 0);
}
/*void scSetPal(color*pptr) {
wsetpalette(0,255,pptr);
}
void scGetPal(color*pptr) {
get_palette(pptr);
}*/

void UpdatePalette() {
    if (game.color_depth > 1)
        invalidate_screen();

    if (!play.fast_forward)  
        setpal();
}
