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

#include "chewy/chewy.h"
#include "chewy/mcga.h"

namespace Chewy {

#define WAIT asm { NOP; NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;}
int16 FuNr;
real_regs rr;
real_regs *rp = &rr;
maus_info *m_info;
VesaInfo vi;

int16 clipx1, clipx2, clipy1, clipy2;
int16 gcurx, gcury, fvorx, fvory;
int16 fontbr, fonth;
int16 scr_w, scr_h;
int16 fontfirst, fontlast;
char writemode;

char MausMode;
char enter_key;
char escape_key;
char einfuegen = 0;

int16 crlfx = 0, crlfy = 0;
int16 r_gef = -1, r_end = false;
int16 *rlist = 0;
byte pal_table[768];
bool mono = false;
uint8 svga;

void init_mcga() {
	// No implementation needed
}

void old_mode() {
	// No implementation needed
}

void vflyback_start() { warning("STUB - vflyback_start"); }

void vflyback_end() { warning("STUB - vflyback_end"); }

void hflyback_start() { warning("STUB - hflyback_start"); }

void hflyback_end() { warning("STUB - hflyback_end"); }

void set_pointer(byte *ptr) { warning("STUB - set_pointer"); }

byte *get_dispoff() { warning("STUB - get_dispoff"); return nullptr; }

void setpalette(byte *palette) { warning("STUB - setpalette"); }

void save_palette(byte *pal) { warning("STUB - save_palette"); }

void restore_palette() { warning("STUB - restore_palette"); }

void rastercol(int16 color, int16 rot, int16 gruen, int16 blau) { warning("STUB - rastercol"); }

void set_palpart(byte *palette, int16 startcol, int16 anz) { warning("STUB - set_palpart"); }

void clear_mcga() { warning("STUB - clear_mcga"); }

void setpixel_mcga(int16 x, int16 y, int16 farbe) { warning("STUB - setpixel_mcga"); }

uint8 getpix(int16 x, int16 y) { warning("STUB - getpix"); return 0; }

void line_mcga(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) { warning("STUB - line_mcga"); }

void mem2mcga(byte *ptr) { warning("STUB - mem2mcga"); }

void mem2mcga_masked(byte *ptr, int16 maske) { warning("STUB - mem2mcga_masked"); }

void mcga2mem(byte *ptr) { warning("STUB - mcga2mem"); }

void mem2mem(byte *ptr1, byte *ptr2) { warning("STUB - mem2mem"); }

void mem2mem_masked(byte *ptr1, byte *ptr2, int16 maske) { warning("STUB - mem2mem_masked"); }

void map_spr_2screen(byte *sptr, int16 x, int16 y) { warning("STUB - map_spr_2screen"); }

void spr_save_mcga(byte *sptr, int16 x, int16 y, int16 breite, int16 hoehe, int16 scrwidth) { warning("STUB - spr_save_mcga"); }

void spr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrwidth) { warning("STUB - spr_set_mcga"); }

void mspr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrwidth) { warning("STUB - mspr_set_mcga"); }

void setfont(byte *adr, int16 breite, int16 hoehe, int16 first, int16 last) { warning("STUB - setfont"); }

void upd_scr() { warning("STUB - upd_scr"); }

void vors() { warning("STUB - vors"); }

void zoom_img(byte *source, byte *dest, int16 xdiff_, int16 ydiff_) { warning("STUB - zoom_img"); }

void zoom_set(byte *source, int16 x, int16 y, int16 xdiff_, int16 ydiff_, int16 scrwidth) { warning("STUB - zoom_set"); }

void putcxy(int16 x, int16 y, char zeichen, int16 forcol, int16 backcol, int16 scrwidth) { warning("STUB - putcxy"); }

void putz(char zeichen, int16 forcol, int16 backcol, int16 scrwidth) { warning("STUB - putz"); }

void init_svga(VesaInfo *vi_, byte *virt_screen) { warning("STUB - init_svga"); }

} // namespace Chewy
