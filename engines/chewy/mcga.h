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

#ifndef CHEWY_MCGA_H
#define CHEWY_MCGA_H

#include "chewy/maus.h"
#include "chewy/ngstypes.h"

namespace Chewy {

void init_mcga();
void old_mode();

void vflyback_start();
void vflyback_end();
void hflyback_start();
void hflyback_end();
void set_pointer(byte *ptr);
byte *get_dispoff();

void setpalette(byte *palette);
void save_palette(byte *pal);
void restore_palette();
void rastercol(int16 color, int16 rot, int16 gruen, int16 blau);
void set_palpart(byte *palette, int16 startcol, int16 anz);

void clear_mcga();
void setpixel_mcga(int16 x, int16 y, int16 farbe);
uint8 getpix(int16 x, int16 y);
void line_mcga(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);

#ifdef EFFEKTE
void split_in(byte *source);
void fall_in(byte *source);
void over_in(byte *source);
void y_shrink(byte *source, byte *dest, int16 faktor, int16 zeile);
#endif

void mem2mcga(byte *ptr);
void mem2mcga_masked(byte *ptr, int16 maske);
void mcga2mem(byte *ptr);
void mem2mem(byte *ptr1, byte *ptr2);
void mem2mem_masked(byte *ptr1, byte *ptr2, int16 maske);
void map_spr_2screen(byte *sptr, int16 x, int16 y);

void spr_save_mcga(byte *sptr, int16 x, int16 y, int16 breite, int16 hoehe,
                   int16 scrwidth);
void spr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrwidth);
void mspr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrwidth);
void zoom_img(byte *source, byte *dest, int16 xdiff,
              int16 ydiff);
void zoom_set(byte *source, int16 x, int16 y, int16 xdiff,
              int16 ydiff, int16 scrwidth);

void putcxy(int16 x, int16 y, char zeichen, int16 forcol, int16 backcol,
            int16 scrwidth);
void putz(char zeichen, int16 forcol, int16 backcol, int16 scrwidth);
void setfont(byte *adr, int16 breite, int16 hoehe, int16 first,
             int16 last);
void vors();
void movecur(int16 x, int16 y);

uint8 joystick();

void init_svga(VesaInfo *vi, byte *virt_screen);
void upd_scr();

bool kbhit();
char getch();
void putch(char c);

} // namespace Chewy

#endif
