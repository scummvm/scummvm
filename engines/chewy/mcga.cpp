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

#include "common/system.h"
#include "graphics/palette.h"
#include "chewy/chewy.h"
#include "chewy/mcga.h"

namespace Chewy {

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
bool mono = false;
uint8 svga;
byte *screenP;

static byte saved_palette[PALETTE_SIZE];

void init_mcga() {
	crlfx = crlfy = 0;
	r_gef = -1;
	r_end = false;
	rlist = 0;
	mono = false;
	svga = 0;
	screenP = (byte *)g_engine->_screen->getPixels();
}

void old_mode() {
	// No implementation needed
}

void vflyback_start() {
	// No implementation needed
}

void vflyback_end() {
	// No implementation needed
}

void hflyback_start() {
	// No implementation needed
}

void hflyback_end() {
	// No implementation needed
}

void set_pointer(byte *ptr) {
	assert(ptr);
	screenP = ptr;
}

byte *get_dispoff() {
	return screenP;
}

void setpalette(byte *palette) {
	g_system->getPaletteManager()->setPalette(palette, 0, PALETTE_COUNT);
}

void save_palette(byte *palette) {
	if (!palette)
		palette = saved_palette;

	g_system->getPaletteManager()->grabPalette(palette, 0, PALETTE_COUNT);
}

void restore_palette() {
	setpalette(saved_palette);
}

void rastercol(int16 color, int16 rot, int16 gruen, int16 blau) {
	warning("STUB - rastercol");
}

void set_palpart(byte *palette, int16 startcol, int16 anz) {
	g_system->getPaletteManager()->setPalette(palette, startcol, anz);
}

void clear_mcga() {
	g_engine->_screen->clear();
}

void setpixel_mcga(int16 x, int16 y, int16 farbe) {
	line_mcga(x, y, x, y, farbe);
}

uint8 getpix(int16 x, int16 y) {
	byte *pixel = (byte *)g_engine->_screen->getBasePtr(x, y);
	return *pixel;
}

void line_mcga(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	g_engine->_screen->drawLine(x1, y1, x2, y2, farbe);
}

void mem2mcga(byte *ptr) {
	warning("STUB - mem2mcga");
}

void mem2mcga_masked(byte *ptr, int16 maske) {
	warning("STUB - mem2mcga_masked");
}

void mcga2mem(byte *ptr) {
	warning("STUB - mcga2mem");
}

void mem2mem(byte *ptr1, byte *ptr2) {
	warning("STUB - mem2mem");
}

void mem2mem_masked(byte *ptr1, byte *ptr2, int16 maske) {
	warning("STUB - mem2mem_masked");
}

void map_spr_2screen(byte *sptr, int16 x, int16 y) {
	warning("STUB - map_spr_2screen");
}

void spr_save_mcga(byte *sptr, int16 x, int16 y, int16 breite, int16 hoehe, int16 scrwidth) {
	warning("STUB - spr_save_mcga");
}

void spr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	warning("STUB - spr_set_mcga");
}

void mspr_set_mcga(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	warning("STUB - mspr_set_mcga");}

void setfont(byte *adr, int16 breite, int16 hoehe, int16 first, int16 last) {
	warning("STUB - setfont");
}

void upd_scr() {
	g_engine->_screen->update();
}

void vors() {
	warning("STUB - vors");
}

void zoom_img(byte *source, byte *dest, int16 xdiff_, int16 ydiff_) {
	warning("STUB - zoom_img");
}

void zoom_set(byte *source, int16 x, int16 y, int16 xdiff_, int16 ydiff_, int16 scrwidth) {
	warning("STUB - zoom_set");
}

void putcxy(int16 x, int16 y, char zeichen, int16 forcol, int16 backcol, int16 scrwidth) {
	warning("STUB - putcxy");
}

void putz(char zeichen, int16 forcol, int16 backcol, int16 scrwidth) {
	warning("STUB - putz");
}

void init_svga(VesaInfo *vi_, byte *virt_screen) {
	warning("STUB - init_svga");
}

bool kbhit() {
	warning("STUB: kbhit()");

	return false;
}

char getch() {
	warning("STUB: getch()");

	return ' ';
}

void putch(char c) {
	warning("STUB: putch()");
}

} // namespace Chewy
