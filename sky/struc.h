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

typedef struct dataFileHeader
{
    uint16 flag; // bit 0: set for colour data, clear for not
		 // bit 1: set for compressed, clear for uncompressed
		 // bit 2: set for 32 colours, clear for 16 colours
    uint16 s_x;
    uint16 s_y;
    uint16 s_width;
    uint16 s_height;
    uint16 s_sp_size;
    uint16 s_tot_size;
    uint16 s_n_sprites;
    uint16 s_offset_x;
    uint16 s_offset_y;
    uint16 s_compressed_size;
}s;

