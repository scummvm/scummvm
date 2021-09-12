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

#define SETZEN 0
#define UND 1
#define ODER 2
#define XODER 3
#define GRAFIK 1
#define SPEICHER 2
#define DATEI 3
#define MAXSTRING 255

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
char enter_key ;
char escape_key;
char einfuegen = 0;

int16 crlfx = 0, crlfy = 0;
int16 r_gef = -1, r_end = false;
int16 *rlist = 0;
byte pal_table[768];
bool mono = false;

// Missing STUBs
uint8 svga;
void init_mcga() { warning ("STUB - init_mcga"); }
void old_mode() { warning("STUB - old_mode"); }
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



void mcga_grafik::ltoa(long N, char *str, int base) {
	sprintf(str, "%ld", N);
}

void mcga_grafik::ultoa(uint32 N, char *str, int base) {
	sprintf(str, "%u", N);
}

void mcga_grafik::itoa(int N, char *str, int base) {
	sprintf(str, "%d", N);
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

mcga_grafik::mcga_grafik() {
	int16 i;

#if 0
	char *enstr;

	char a[] = {92, 85, 97, 59, 96, 83, 100, 83, 79, 90, 0}; // "NGS-REVEAL"
	char b[] = {85, 96, 79, 94, 86, 87, 81, 97, 109, 82, 96, 87, 100, 83, 96, 109,
		54, 81, 55, 109, 92, 83, 101, 109, 85, 83, 92, 83, 96, 79, 98, 87, 93, 92,
		109, 97, 93, 84, 98, 101, 79, 96, 83, 0 }; // "GRAPHICS_DRIVER_(C)_NEW_GENERATION_SOFTWARE"
	for (i = 0; a[i] != 0; i++)
		a[i] -= 14;
	for (i = 0; b[i] != 0; i++)
		b[i] -= 14;
	enstr = (char *)getenv(a);
	if (enstr) {
		printf(b);
		printf("\n");
		delay(800);
	}
#endif

	for (i = 0; i < 360; i++) {
		sinus[i] = sin(i * 3.14159265359 / 180);
		cosinus[i] = cos(i * 3.14159265359 / 180);
	}
	scr_w = 0;
	scr_h = 200;
	mono = false;
	MausMode = false;
}
mcga_grafik::~mcga_grafik() {
}

void mcga_grafik::init() {
	scr_w = 320;
	scr_h = 200;
	init_mcga();
}

void mcga_grafik::set_writemode(char wm) {
	writemode = wm;
}

void mcga_grafik::restore_mode() {
	old_mode();
}

void mcga_grafik::set_clip(int16 x1, int16 y1, int16 x2, int16 y2) {
	clipx1 = x1;
	clipx2 = x2;
	clipy1 = y1;
	clipy2 = y2;
}

void mcga_grafik::vsync_start() {
	vflyback_start();
}

void mcga_grafik::vsync_end() {
	vflyback_end();
}

void mcga_grafik::hsync_start() {
	hflyback_start();
}

void mcga_grafik::hsync_end() {
	hflyback_end();
}

void mcga_grafik::skip_frame(int16 frames) {
	int16 i = 0;
	for (i = 0; i < frames; i++) {
		vflyback_start();
		vflyback_end();
	}
}

void mcga_grafik::skip_line(int16 lines) {
	int16 i = 0;
	for (i = 0; i < lines; i++) {
		hflyback_start();
		hflyback_end();
	}
}

void mcga_grafik::setze_zeiger(byte *ptr) {
	set_pointer(ptr);
}

byte *mcga_grafik::get_zeiger() {
	return get_dispoff();
}

void mcga_grafik::set_bildbreite(int16 breite) {
	scr_w = breite;
	if (!scr_w)
		scr_w = 320;
}

void mcga_grafik::set_mono() {
	mono = true;
}

void mcga_grafik::calc_mono(byte *pal, int16 startcol, int16 anz) {
	int16 i, k;
	uint8 r, g, b, grau;
	k = startcol * 3;
	for (i = 0; i < anz; i++) {
		r = pal[k];
		g = pal[k + 1];
		b = pal[k + 2];
		grau = (((float)r) * 0.30) + (((float)g) * 0.59) + (((float)b) * 0.11);
		pal[k] = grau;
		pal[k + 1] = grau;
		pal[k + 2] = grau;
		k += 3;
	}
}

void mcga_grafik::set_palette(byte *palette) {
	int16 i;
	for (i = 0; i < 768; i++)
		pal_table[i] = palette[i];
	if (mono)
		calc_mono(palette, 0, 256);
	setpalette(palette);
}

void mcga_grafik::palette_save(byte *pal) {
	save_palette(pal);
}

void mcga_grafik::rest_palette() {
	restore_palette();
}

void mcga_grafik::raster_col(int16 c, int16 r, int16 g, int16 b) {
	int16 index;
	index = c * 3;
	pal_table[index] = r;
	pal_table[index + 1] = g;
	pal_table[index + 2] = b;
	if (mono)
		calc_mono(pal_table, c, 1);
	rastercol(c, r, g, b);
}

void mcga_grafik::einblenden(byte *palette, int16 frames) {
	int16 i, j, k;
	int16 r, g, b;
	int16 r1, g1, b1;
	if (mono)
		calc_mono(palette, 0, 256);
	for (j = 63; j >= 0; j--) {
		k = 0;
		for (i = 0; i < 256; i++) {
			r = palette[k];
			g = palette[k + 1];
			b = palette[k + 2];
			r1 = r - j;
			g1 = g - j;
			b1 = b - j;
			if ((r1 > 0) && (r1 <= r))
				pal_table[k] = r1;
			if ((g1 > 0) && (r1 <= r))
				pal_table[k + 1] = g1;
			if ((b1 > 0) && (r1 <= r))
				pal_table[k + 2] = b1;
			k += 3;
		}
		setpalette(pal_table);
		if (frames)
			skip_frame(frames);
	}
}

void mcga_grafik::aufhellen(byte *palette, int16 startcol, int16 anz, int16 stufen, int16
                            frames) {
	int16 i = 0, j, k;
	int16 r, g, b;
	int16 endcol = 0;
	if (mono)
		calc_mono(palette, startcol, anz);
	endcol = startcol + anz;
	for (j = stufen; j >= 0; j--) {
		endcol = startcol + anz;
		k = startcol * 3;
		for (i = startcol; i < endcol; i++) {
			r = pal_table[k];
			g = pal_table[k + 1];
			b = pal_table[k + 2];
			if ((r + 1) <= palette[k]) ++r;
			if ((g + 1) <= palette[k + 1]) ++g;
			if ((b + 1) <= palette[k + 2]) ++b;
			pal_table[k] = r;
			pal_table[k + 1] = g;
			pal_table[k + 2] = b;
			k += 3;
		}
		set_palpart(pal_table, startcol, anz);
		if (frames)
			skip_frame(frames);
	}
}

void mcga_grafik::ausblenden(int16 frames) {
	int16 i, j, k;
	int16 r, g, b;
	for (j = 0; j < 64; j++) {
		k = 0;
		for (i = 0; i < 256; i++) {
			r = pal_table[k];
			g = pal_table[k + 1];
			b = pal_table[k + 2];
			if (r > 0) --r;
			if (g > 0) --g;
			if (b > 0) --b;
			pal_table[k] = r;
			pal_table[k + 1] = g;
			pal_table[k + 2] = b;
			k += 3;
		}
		setpalette(pal_table);
		if (frames)
			skip_frame(frames);
	}
}

void mcga_grafik::abblenden(int16 startcol, int16 anz, int16 stufen, int16 frames) {
	int16 i, j, k;
	int16 r, g, b;
	int16 endcol = 0;
	endcol = startcol + anz;
	for (j = 0; j < stufen; j++) {
		k = startcol * 3;
		for (i = startcol; i < endcol; i++) {
			r = pal_table[k];
			g = pal_table[k + 1];
			b = pal_table[k + 2];
			if (r > 0) --r;
			if (g > 0) --g;
			if (b > 0) --b;
			pal_table[k] = r;
			pal_table[k + 1] = g;
			pal_table[k + 2] = b;
			k += 3;
		}
		set_palpart(pal_table, startcol, anz);
		if (frames)
			skip_frame(frames);
	}
}

void mcga_grafik::set_teilpalette(byte *palette, int16 startcol, int16 anz) {
	int16 i;
	int16 k, endcol;
	k = startcol * 3;
	endcol = startcol + anz;
	for (i = startcol; i < endcol; i++) {
		pal_table[k] = palette[k];
		pal_table[k + 1] = palette[k + 1];
		pal_table[k + 2] = palette[k + 2];
		k += 3;
	}
	if (mono)
		calc_mono(pal_table, startcol, anz);
	set_palpart(pal_table, startcol, anz);
}

void mcga_grafik::cls() {
	clear_mcga();
}

void mcga_grafik::punkt(int16 xpos, int16 ypos, int16 farbn) {
	setpixel_mcga(xpos, ypos, farbn);
}

uint8 mcga_grafik::get_pixel(int16 xpos, int16 ypos) {
	uint8 pix;
	pix = getpix(xpos, ypos);
	return (pix);
}

void mcga_grafik::linie(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	line_mcga(x1, y1, x2, y2, farbe);
}

void mcga_grafik::box(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	line_mcga(x1, y1, x2, y1, farbe);
	line_mcga(x1, y2 - 1, x2, y2 - 1, farbe);
	line_mcga(x1, y1, x1, y2, farbe);
	line_mcga(x2, y1, x2, y2, farbe);
}

void mcga_grafik::box_fill(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe) {
	int16 h, i;
	if (x2 == x1)x2++;
	h = abs(y2 - y1);
	if (h == 0)
		h = 1;
	for (i = 0; i < h; i++)
		line_mcga(x1, y1 + i, x2, y1 + i, farbe);
}

void mcga_grafik::pop_box(int16 x, int16 y, int16 x1, int16 y1,
                          int16 col1, int16 col2, int16 back_col) {
	if (back_col < 255)
		box_fill(x, y, x1, y1, back_col);
	linie(x, y1, x1, y1, col2);
	linie(x1, y, x1, y1, col2);
	linie(x, y, x1, y, col1);
	linie(x, y, x, y1 + 1, col1);
}

void mcga_grafik::kreis(int16 x, int16 y, int16 r, int16 farbe) {
	int16 a = 0, b = 0, alt = 0, diff;
	int16 w, i;
	for (w = 0; w <= 91; w++) {
		a = (int16)(sinus[w] * ((float)r * 0.85));
		if ((a - alt) > 1) {
			diff = a - alt;
			for (i = 0; i <= diff; i++) {
				setpixel_mcga(x - b, (y - (alt + i)), farbe);
				setpixel_mcga(x + b, (y - (alt + i)), farbe);
				setpixel_mcga(x - b, (y + (alt + i)), farbe);
				setpixel_mcga(x + b, (y + (alt + i)), farbe);
			}
		}
		b = (int16)(cosinus[w] * (float)r);
		setpixel_mcga(x - b, y - a, farbe);
		setpixel_mcga(x + b, y - a, farbe);
		setpixel_mcga(x - b, y + a, farbe);
		setpixel_mcga(x + b, y + a, farbe);
		alt = a;
	}
}

void mcga_grafik::fkreis(int16 x, int16 y, int16 r, int16 farbe) {
	int16 a = 0, b = 0, alt = 0, i = 0, diff;
	int16 w;
	for (w = 0; w <= 90; w++) {
		a = (int16)(sinus[w] * ((float)r * 0.85));
		if ((a - alt) > 1) {
			diff = a - alt;
			for (i = 0; i < diff; i++) {
				line_mcga(x - b, (y - (alt + i)), x + b, (y - (alt + i)), farbe);
				line_mcga(x - b, (y + (alt + i)), x + b, (y + (alt + i)), farbe);
			}
		}
		b = (int16)(cosinus[w] * ((float)r));
		line_mcga(x - b, (y - (alt + i)), x + b, (y - (alt + i)), farbe);
		line_mcga(x - b, (y + (alt + i)), x + b, (y + (alt + i)), farbe);
		alt = a;
	}
}

#ifdef EFFEKTE
void mcga_grafik::seit_in(char *source) {
	split_in(source);
}

void mcga_grafik::falling_in(char *source) {
	fall_in(source);
}

void mcga_grafik::ueberblend(char *source) {
	over_in(source);
}

void mcga_grafik::y_shrumpf(char *source, char *dest, int16 faktor,
                            int16 zeile) {
	y_shrink(source, dest, faktor, zeile);
}
#endif

void mcga_grafik::back2screen(byte *ptr) {
	mem2mcga(ptr);
}

void mcga_grafik::back2back(byte *ptr1, byte *ptr2) {
	mem2mem(ptr1, ptr2);
}

void mcga_grafik::back2screen_maskiert(byte *ptr, int16 maske) {
	mem2mcga_masked(ptr, maske);
}

void mcga_grafik::back2back_maskiert(byte *ptr1, byte *ptr2, int16 maske) {
	mem2mem_masked(ptr1, ptr2, maske);
}

void mcga_grafik::screen2back(byte *ptr) {
	mcga2mem(ptr);
}

void mcga_grafik::sprite_save(byte *sptr, int16 x,
                              int16 y, int16 breite, int16 hoehe, int16 scrwidth) {
	if (breite < 4)breite = 4;
	if (hoehe <= 0)hoehe = 1;
	if (x < clipx1) {
		x = clipx1;
		breite -= (clipx1 - x);
	}
	if ((x + breite) > clipx2 + 1)
		breite = clipx2 - x;
	if (y < clipy1) {
		y = clipy1;
		hoehe -= (clipy1 - y);
	}
	if ((y + hoehe) > clipy2 + 1)
		hoehe = clipy2 - y;
	if (breite < 1)breite = 0;
	if (hoehe <= 0)hoehe = 0;
	spr_save_mcga(sptr, x, y, breite, hoehe, scrwidth);
}

void mcga_grafik::sprite_set(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	mspr_set_mcga(sptr, x, y, scrwidth);
}

void mcga_grafik::blockcopy(byte *sptr, int16 x, int16 y, int16 scrwidth) {
	spr_set_mcga(sptr, x, y, scrwidth);
}

void mcga_grafik::map_spr2screen(byte *sptr, int16 x, int16 y) {
	int16 br, h;
	br = ((int16 *)sptr)[0];
	h = ((int16 *)sptr)[1];
	if ((br >= 320) || (h >= 200))
		map_spr_2screen(sptr, x, y);
}

void mcga_grafik::set_fontadr(byte *adr) {
	tff_header *tff;
	tff = (tff_header *) adr;
	setfont(adr + sizeof(tff_header), (int16)tff->width, (int16)tff->height,
	        (int16)tff->first, (int16)tff->last);
	fvorx = (int16)tff->width;
	fvory = 0;
}

int16 mcga_grafik::scanxy(int16 x, int16 y, int16 fcol, int16 bcol, int16 cur_col, int16 scrwidth,
                          const char *string, ...) {
	int16 i, j, stelle, stellemax, mode = 0;
	int16 disp_stelle = 0, disp_stellemax = 0, disp_akt = 0;
	int16 ret = 0;
	int16 delay_flag;
	char ende = 0, zeichen, zaehler, cursor_z;
	char zstring[81], z1string[81];
	char einfuege_cur = 0;
	char char_anf = 0, char_end = 0;
	uint16 vorzeichen = 0;
	char x_mode = 0;
	char eing = 0;
	uint16 izahl = 0;
	uint16 *intzahl = nullptr;
	int16 *intzahl1 = nullptr;
	uint32 *longzahl = nullptr;
	char *charstr = nullptr;
	uint32 luzahl;
	va_list parptr;
	va_start(parptr, string);
	if ((x == -1) || (y == -1)) {
		x = gcurx;
		y = gcury;
	} else {
		gcurx = x;
		gcury = y;
	}
	move(x, y);
	i = 0;
	while (!ende) {
		zeichen = string[i];
		++i;

		if ((zeichen >= fontfirst) && (zeichen <= fontlast) && (zeichen != 127)) {
			if (zeichen == '%') {

				zeichen = string[i];
				++i;
				zaehler = 0;

				if (zeichen == '[') {
					char_anf = string[i];
					++i;
					++i;
					char_end = string[i];
					++i;
					++i;
					zeichen = string[i];
					++i;
				}

				while ((zeichen >= '0') && (zeichen <= '9')) {
					zaehler *= 10;
					zaehler += (zeichen - 48);
					if (zaehler > 80)
						zaehler = 80;
					zeichen = string[i];
					++i;
				}
				if (einfuegen == 1) {
					einfuege_cur = 125;
					cursor_z = '_' + einfuege_cur;
				} else {
					einfuege_cur = 0;
					cursor_z = '_';
				}

				switch (zeichen) {
				case 'd':
					vorzeichen = '-';
				case 'u':
					if ((char_anf == 0) && (char_end == 0)) {
						char_anf = '0';
						char_end = '9';
					}
					if (!zaehler)
						zaehler = 4;
					if (zeichen == 'd') {
						mode = 4;
						intzahl1 = va_arg(parptr, int16 *);
						luzahl = intzahl1[0];
					} else {
						mode = 1;
						intzahl = va_arg(parptr, uint16 *);
						luzahl = intzahl[0];
					}
					if (luzahl != 0)
						ltoa(luzahl, zstring, 10);
					else {
						zstring[0] = '0';
						zstring[1] = 0 ;
					}
					break;
				case 'l':
					vorzeichen = '-';
					if ((char_anf == 0) && (char_end == 0))
						vorzeichen = '-';
					{
						char_anf = '0';
						char_end = '9';
					}
					mode = 2;
					if (!zaehler)
						zaehler = 9;
					longzahl = va_arg(parptr, uint32 *);
					luzahl = longzahl[0];
					if (luzahl != 0)
						ultoa(luzahl, zstring, 10);
					else {
						zstring[0] = '0';
						zstring[1] = 0 ;
					}
					break;
				case 's':
					if ((char_anf == 0) && (char_end == 0)) {
						char_anf = ' ';
						char_end = '\xdc';
					}
					mode = 3;
					vorzeichen = 0xFF;
					if (!zaehler)
						zaehler = 81;
					charstr = va_arg(parptr, char *);
					strcpy(zstring, charstr);
					break;
				}
				zeichen = string[i];
				++i;
				if (zeichen == 'x') {
					x_mode = 1;
					zeichen = string[i];
					++i;
				}
				disp_stellemax = 0;

				while ((zeichen >= '0') && (zeichen <= '9')) {
					disp_stellemax *= 10;
					disp_stellemax += (zeichen - 48);
					if (disp_stellemax > 80)
						disp_stellemax = 80;
					zeichen = string[i];
					++i;
				}
				if (!disp_stellemax)
					disp_stellemax = zaehler;

				stelle = 0;
				disp_stelle = 0;
				while (zstring[stelle] != 0) {
					++stelle;
				}
				stellemax = stelle;
				if (stelle > disp_stellemax) {
					disp_stelle = stelle - disp_stellemax;
					disp_akt = disp_stellemax;
				} else {
					disp_akt = stelle;
					disp_stelle = 0;
				}
				for (j = stelle; j < j + 1; j++) {
					while ((!kbhit()) || (eing != 0)) {

						eing = 0;
						move(x, y);
						vsync_start();
						for (i = disp_stelle; i <= disp_stellemax + disp_stelle; ++i) {
							if (zstring[i] != 0) {
								putz(zstring[i], fcol, bcol, scrwidth);
								vors();
							} else {
								putz(zstring[i], fcol, bcol, scrwidth);
								vors();
								break;
							}
						}
						if (svga == ON)
							upd_scr();
						for (delay_flag = 0; (delay_flag < 10) && (!kbhit()); delay_flag++)

						{
							skip_frame(1);
							izahl = devices();
							if (izahl == 13) {
								eing = 2;
								break;
							} else if (izahl == 27) {
								eing = 2;
								break;
							}
						}
						plot_scan_cur((x + (disp_akt)*fvorx), gcury, cur_col, 300, scrwidth, cursor_z);
						if (svga == ON)
							upd_scr();
						for (delay_flag = 0; (delay_flag < 10) && (!kbhit()); delay_flag++)

						{
							skip_frame(1);
							izahl = devices();
							if (izahl == 13) {
								eing = 2;
								break;
							} else if (izahl == 27) {
								eing = 2;
								break;
							}
						}
						if (eing == 2)
							break;
					}
					if (eing < 2)
						izahl = (uint16)getch();
					if (izahl == 13) {

						ret = 13;
						ende = 1;
						break;
					}
					if (izahl == 27) {

						ret = 27;
						ende = 2;
						break;
					}

					if (izahl == 8) {
						eing = 1;
						while (kbhit())
							getch();
						if (stelle > 0) {
							strcpy(zstring + stelle - 1, zstring + stelle);
							plot_scan_cur((x + disp_akt * fvorx), gcury, bcol, bcol, scrwidth, cursor_z);
							if (svga == ON)
								upd_scr();
							--stelle;
							--stellemax;
							if (disp_stelle > 0)
								--disp_stelle;
							else if (disp_akt > 0) {
								--disp_akt;
							}
						} else
							putch(7);
					}

					if (izahl == 0) {
						izahl = getch();

						if (izahl == 75) {
							while (kbhit())
								getch();
							eing = 1;
							if (stelle <= 0) {
								putch(7);
								stelle = 0;

							} else {
								if (disp_akt > 0) {
									--disp_akt;
								} else {
									if (disp_stelle > 0)
										--disp_stelle;
								}
								--stelle;
							}
						}

						if (izahl == 77) {
							eing = 1;
							while (kbhit())
								getch();
							++j;
							if (stelle >= stellemax) {
								stelle = stellemax;
								putch(7);
							} else {
								if (disp_akt < disp_stellemax) {
									++disp_akt;
								} else {
									if (disp_stelle <= (zaehler - disp_stellemax)) {
										++disp_stelle;
									}
								}
								++stelle;
							}
						}

						if (izahl == 71) {
							stelle = 0;
							disp_akt = 0;
							disp_stelle = 0;
						}

						if (izahl == 79) {
							stelle = stellemax;
							if (stellemax < disp_stellemax) {
								disp_stelle = 0;
								disp_akt = stellemax;
							}
							else {
								disp_akt = disp_stellemax;
								disp_stelle = stelle - disp_stellemax;
							}
						}

						if ((izahl == 83) && (stelle < stellemax)) {
							while (kbhit())
								getch();
							eing = 1;
							strcpy(zstring + stelle, zstring + stelle + 1);
							--stellemax;
						}

						if (izahl == 82) {
							//delay(200);
							if (!einfuegen) {
								einfuegen = 1;
								einfuege_cur = 125;
								cursor_z = '_' + einfuege_cur;
							} else {
								einfuegen = 0;
								einfuege_cur = 0;
								cursor_z = '_';
							}
						}
						izahl = 0;
					}

					if (izahl < char_anf || izahl > char_end) {
						if (vorzeichen == izahl && !stelle);
						else {
							if (x_mode == 1) {
								ende = 1;
								ret = izahl;
								break;
							}
							continue;
						}
					}
					if ((stelle == stellemax) && (stellemax >= zaehler)) {
						stellemax = zaehler;
						putch(7);
					} else {

						if (disp_akt < disp_stellemax) {
							++disp_akt;
						} else {
							if (disp_stelle <= (zaehler - disp_stellemax)) {
								++disp_stelle;
								disp_akt = disp_stellemax;
							}
						}
						if (!einfuegen) {
							zstring[stelle] = izahl;
							zstring[stellemax + 1] = 0;
							if (stelle == stellemax) {
								++stelle;
								++stellemax;
							} else {
								++stelle;
							}
						}
						else {
							strcpy(z1string, zstring);
							strcpy(zstring + stelle + 1, z1string + stelle);
							zstring[stelle] = izahl;
							zstring[(int)zaehler] = 0;
							if (stellemax < zaehler) {
								++stelle;
								++stellemax;
							} else {
								++stelle;
							}
						}
					}
					if (x_mode == 1) {
						ende = 1;
						ret = izahl;

						break;
					}
				}
			}

		}
	}

	move(x, y);
	for (i = disp_stelle; i <= disp_stellemax + disp_stelle; ++i) {
		if (zstring[i] != 0) {
			putz(zstring[i], fcol, bcol, scrwidth);
			vors();
		} else {
			putz(zstring[i], fcol, bcol, scrwidth);
			vors();
			break;
		}
	}
	switch (ende) {

	case 1:
		switch (mode) {
		case 1:
			intzahl[0] = atoi(zstring);
			break;
		case 2:
			longzahl[0] = atol(zstring);
			break;
		case 3:
			strcpy(charstr, zstring);
			break;
		case 4:
			intzahl1[0] = atoi(zstring);
			break;
		}
		break;

	case 2:
		break;
	}
	if (svga == ON)
		upd_scr();
	return (ret);
}

void mcga_grafik::plot_scan_cur(int16 x, int16 y, int16 fcol, int16 bcol, int16 scrwidth,
                                char cursor_z) {
	move(x, y);
	putz(cursor_z, fcol, bcol, scrwidth);
}

void mcga_grafik::printxy(int16 x, int16 y, int16 forcol, int16 backcol, int16 scrwidth,
                          const char *string, ...) {
	int16 i = 0, k = 0, l;
	char zeichen, zstring[35];
	int16 izahl;
	int16 count, diff;
	uint16 uzahl;
	long lzahl;
	uint32 luzahl;
	char *tempptr;
	va_list parptr;
	va_start(parptr, string);
	crlfx = x;
	crlfy = y + (int16)fonth + 2;
	gcurx = x;
	gcury = y;
	i = 0;
	do {
		zeichen = string[i];
		++i;
		if ((zeichen < 32) || (zeichen == 127)) {
			switch (zeichen) {
			case 8:
				gcurx -= fvorx;
				gcury -= fvory;
				putz(32, forcol, backcol, scrwidth);
				break;

			case 10:
				gcury = crlfy;
				gcurx = crlfx;
				crlfx = gcurx;
				crlfy = gcury + fonth + 2;
				break;

			case 13:
				gcurx = crlfx;
				break;

			case 127 :
				putz(32, forcol, backcol, scrwidth);
				break;

			default :
				if (zeichen >= fontfirst)
					putz(zeichen, forcol, backcol, scrwidth);
				break;
			}
		}
		else if ((zeichen >= fontfirst) && (zeichen <= fontlast)) {
			if (zeichen != '%') {
				putz(zeichen, forcol, backcol, scrwidth);
				vors();
			}
			else {
				zeichen = string[i];
				count = 0;
				if ((zeichen >= 0x30) && (zeichen <= 0x39))
					count = atoi(string + i);
				while ((zeichen >= 0x30) && (zeichen <= 0x39)) {
					++i;
					zeichen = string[i];
				}
				++i;
				switch (zeichen) {
				case '%':
					putz(zeichen, forcol, backcol, scrwidth);
					vors();
					break;

				case 'd':
				case 'u':
					if (zeichen == 'd') {
						izahl = va_arg(parptr, int);
						itoa(izahl, zstring, 10);
					} else {
						uzahl = va_arg(parptr, unsigned int);
						itoa(uzahl, zstring, 10);
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, forcol, backcol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], forcol, backcol, scrwidth);
						vors();
						++k;
					}
					break;

				case 'l':
					if (string[i] != 'u') {
						lzahl = va_arg(parptr, long);
						ltoa(lzahl, zstring, 10);
					} else {
						luzahl = va_arg(parptr, uint32);
						ultoa(luzahl, zstring, 10);
						++i;
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, forcol, backcol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], forcol, backcol, scrwidth);
						vors();
						++k;
					}
					break;

				case 's':
					tempptr = va_arg(parptr, char *);
					if (!count) {
						while (*tempptr != 0) {
							putz(*tempptr, forcol, backcol, scrwidth);
							++tempptr;
							vors();
						}
					}
					else {
						for (l = 0; l < count; l++) {
							putz(*tempptr, forcol, backcol, scrwidth);
							++tempptr;
							vors();
						}
					}
					break;

				}
			}
		}
	} while ((i < MAXSTRING) && (zeichen != 0));
}

void mcga_grafik::speed_printxy(int16 x, int16 y, int16 forcol, int16 backcol,
                                int16 scrwidth, char *string) {
	int16 i = 0;
	char zeichen;
	gcurx = x;
	gcury = y;
	i = 0;
	do {
		zeichen = string[i];
		++i;
		if ((zeichen >= fontfirst) && (zeichen <= fontlast)) {
			putz(zeichen, forcol, backcol, scrwidth);
			vors();
		}
	} while ((i < MAXSTRING) && (zeichen != 0));
}

void mcga_grafik::print(int16 forcol, int16 backcol, int16 scrwidth, char *string, ...) {
	int16 i = 0, k = 0, l;
	char zeichen, zstring[35];
	char *tempptr;
	int16 izahl, count, diff;
	uint16 uzahl;
	long lzahl;
	uint32 luzahl;
	va_list parptr;
	va_start(parptr, string);
	crlfx = gcurx;
	crlfy = gcury + fonth + 2;
	i = 0;
	do {
		zeichen = string[i];
		++i;
		if ((zeichen > 0 && zeichen < 32) || (zeichen == 127)) {
			switch (zeichen) {
			case 8:
				gcurx -= fvorx;
				gcury -= fvory;
				putz(32, forcol, backcol, scrwidth);
				break;

			case 10:
				gcury = crlfy;
				gcurx = crlfx;
				crlfx = gcurx;
				crlfy = gcury + fonth + 2;
				break;

			case 13:
				gcurx = crlfx;
				break;

			case 127 :
				putz(32, forcol, backcol, scrwidth);
				break;

			default :
				if (zeichen >= fontfirst) {
					putz(zeichen, forcol, backcol, scrwidth);
					vors();
				}
				break;
			}
		} else if ((zeichen >= fontfirst) && (zeichen <= fontlast) && (zeichen != 0)) {
			if (zeichen != '%') {
				putz(zeichen, forcol, backcol, scrwidth);
				vors();
			} else {
				zeichen = string[i];
				count = 0;
				if ((zeichen >= 0x30) && (zeichen <= 0x39))
					count = atoi(string + i);
				while ((zeichen >= 0x30) && (zeichen <= 0x39)) {
					++i;
					zeichen = string[i];
				}
				++i;
				switch (zeichen) {
				case '%':
					putz(zeichen, forcol, backcol, scrwidth);
					vors();
					break;

				case 'd':
				case 'u':
					if (zeichen == 'd') {
						izahl = va_arg(parptr, int);
						itoa(izahl, zstring, 10);
					} else {
						uzahl = va_arg(parptr, unsigned int);
						itoa(uzahl, zstring, 10);
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, forcol, backcol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], forcol, backcol, scrwidth);
						vors();
						++k;
					}
					break;

				case 'l':
					if (string[i] != 'u') {
						lzahl = va_arg(parptr, long);
						ltoa(lzahl, zstring, 10);
					} else {
						luzahl = va_arg(parptr, unsigned long);
						ultoa(luzahl, zstring, 10);
						++i;
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, forcol, backcol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], forcol, backcol, scrwidth);
						vors();
						++k;
					}
					break;

				case 's':
					tempptr = va_arg(parptr, char *);
					if (!count) {
						while (*tempptr != 0) {
							putz(*tempptr, forcol, backcol, scrwidth);
							++tempptr;
							vors();
						}
					} else {
						for (l = 0; l < count; l++) {
							putz(*tempptr, forcol, backcol, scrwidth);
							++tempptr;
							vors();
						}
					}

					break;

				}
			}
		}
	} while ((i < MAXSTRING) && (zeichen != 0));
}

void mcga_grafik::printnxy(int16 x, int16 y, int16 forcol, int16 backcol, int16 menge,
                           int16 scrwidth, char *string, ...) {
	int16 i = 0, k = 0, l;
	char zeichen, zstring[35];
	char *tempptr;
	int16 izahl, count, diff;
	uint16 uzahl;
	long lzahl;
	uint32 luzahl;
	va_list parptr;
	va_start(parptr, string);
	gcurx = x;
	gcury = y;
	crlfx = gcurx;
	crlfy = y + fonth + 2;
	for (i = 0; i < menge;) {
		zeichen = string[i];
		++i;
		if ((zeichen < 32) || (zeichen == 127)) {
			switch (zeichen) {
			case 8:
				gcurx -= fvorx;
				gcury -= fvory;
				putz(32, forcol, backcol, scrwidth);
				break;

			case 10:
				gcury = crlfy;
				gcurx = crlfx;
				crlfx = gcurx;
				crlfy = gcury + fonth + 2;
				break;

			case 13:
				gcurx = crlfx;
				break;

			case 127 :
				putz(32, forcol, backcol, scrwidth);
				break;

			default :
				if (zeichen >= fontfirst)
					putz(zeichen, forcol, backcol, scrwidth);
				break;
			}
		}
		else if ((zeichen >= fontfirst) && (zeichen <= fontlast)) {
			if (zeichen != '%') {
				putz(zeichen, forcol, backcol, scrwidth);
				vors();
			}
			else {
				zeichen = string[i];
				count = 0;
				if ((zeichen >= 0x30) && (zeichen <= 0x39))
					count = atoi(string + i);
				while ((zeichen >= 0x30) && (zeichen <= 0x39)) {
					++i;
					zeichen = string[i];
				}
				++i;
				switch (zeichen) {
				case '%':
					putz(zeichen, forcol, backcol, scrwidth);
					vors();
					break;

				case 'd':
				case 'u':
					if (zeichen == 'd') {
						izahl = va_arg(parptr, int);
						itoa(izahl, zstring, 10);
					} else {
						uzahl = va_arg(parptr, unsigned int);
						itoa(uzahl, zstring, 10);
					}
					k = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, forcol, backcol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], forcol, backcol, scrwidth);
						vors();
						++k;
					}
					break;

				case 'l':
					if (string[i] != 'u') {
						lzahl = va_arg(parptr, long);
						ltoa(lzahl, zstring, 10);
					} else {
						luzahl = va_arg(parptr, uint32);
						ultoa(luzahl, zstring, 10);
						++i;
					} k
					    = 0;
					if (count) {
						diff = check_stellen_anz(zstring, &k, count);
						for (l = 0; l < diff; l++) {
							putz(0x30, forcol, backcol, scrwidth);
							vors();
						}
					}
					while (zstring[k] != 0) {
						putz(zstring[k], forcol, backcol, scrwidth);
						vors();
						++k;
					}
					break;

				case 's':
					tempptr = va_arg(parptr, char *);
					if (!count) {
						while (*tempptr != 0) {
							putz(*tempptr, forcol, backcol, scrwidth);
							++tempptr;
							vors();
						}
					}
					else {
						for (l = 0; l < count; l++) {
							putz(*tempptr, forcol, backcol, scrwidth);
							++tempptr;
							vors();
						}
					}

					break;

				}
			}
		}
	}
}

void mcga_grafik::printcharxy(int16 x, int16 y, char zeichen, int16 forcol, int16 backcol,
                              int16 scrwidth) {
	crlfx = x;
	crlfy = y + fonth + 2;
	if ((zeichen < 32) || (zeichen == 127)) {
		switch (zeichen) {
		case 8:
			x -= fvorx;
			y -= fvory;
			putcxy(x, y, 32, forcol, backcol, scrwidth);
			break;

		case 10:
			gcury = crlfy;
			gcurx = crlfx;
			crlfx = gcurx;
			crlfy = gcury + fonth + 2;
			break;

		case 13:
			gcurx = crlfx;
			gcury = crlfy;
			break;

		case 127:
			putcxy(x, y, 32, forcol, backcol, scrwidth);
			break;
		}
	}
	else if ((zeichen >= fontfirst) && (zeichen <= fontlast) && (zeichen != 127)) {
		putcxy(x, y, zeichen, forcol, backcol, scrwidth);
		vors();
	}
}

void mcga_grafik::printchar(char zeichen, int16 forcol, int16 backcol, int16 scrwidth) {
	crlfx = gcurx;
	crlfy = gcury + fonth + 2;
	if ((zeichen < 32) || (zeichen == 127)) {
		switch (zeichen) {
		case 8:
			gcurx -= fvorx;
			gcury -= fvory;
			putz(32, forcol, backcol, scrwidth);
			break;

		case 10:
			gcury = crlfy;
			gcurx = crlfx;
			crlfx = gcurx;
			crlfy = gcury + fonth + 2;
			break;

		case 13:
			gcurx = crlfx;
			gcury = crlfy;
			break;

		case 127:
			putz(32, forcol, backcol, scrwidth);
			break;
		}
	}
	else if ((zeichen >= fontfirst) && (zeichen <= fontlast) && (zeichen != 127)) {
		putz(zeichen, forcol, backcol, scrwidth);
		vors();
	}
}

void mcga_grafik::set_vorschub(int16 x, int16 y) {
	if (fvorx != -255)
		fvorx = x;
	if (fvory != -255)
		fvory = y;
}

void mcga_grafik::get_fontinfo(int16 *vorx, int16 *vory, int16 *fntbr, int16 *fnth) {
	*vorx = fvorx;
	*vory = fvory;
	*fntbr = fontbr;
	*fnth = fonth;
}

void mcga_grafik::vorschub() {
	vors();
}

void mcga_grafik::move(int16 x, int16 y) {
	gcurx = x;
	gcury = y;
}

void mcga_grafik::init_mausmode(maus_info *minfo) {
	MausMode = true;
	m_info = minfo;
}

void mcga_grafik::exit_mausmode() {
	MausMode = false;
	m_info = 0;
}

int16 mcga_grafik::devices() {
	int16 i;
	i = 0;
	if (MausMode != false) {
		if (i > 1) {
			i = 27;
		} else if (i == 1)
			i = 13;
		else
			i = 0;
	}
	return (i);
}

int16 mcga_grafik::check_stellen_anz(char *zstring, int16 *pos, int16 stellen) {
	int16 k, diff;
	k = 0;
	while (zstring[k] != 0)
		++k;
	if (k >= stellen) {
		while ((k) && (stellen)) {
			--k;
			--stellen;
		}
		diff = 0;
	} else {
		diff = stellen - k;
		k = 0;
	}
	*pos = k;
	return (diff);
}

void mcga_grafik::scale_image(byte *source, byte *dest, int16 xdiff_, int16 ydiff_) {
	zoom_img(source, dest, xdiff_, ydiff_);
}

void mcga_grafik::scale_set(byte *sptr, int16 x, int16 y, int16 xdiff_, int16 ydiff_, int16 scrwidth) {
	if ((xdiff_) || (ydiff_))
		zoom_set(sptr, x, y, xdiff_, ydiff_, scrwidth);
	else
		mspr_set_mcga(sptr, x, y, scrwidth);
}

void mcga_grafik::init(uint16 mode, byte *info_blk, byte *vscreen) {
	if (!get_vesa_info(mode, info_blk)) {
		init_svga(&vi, vscreen);
		//delay(250);
	}
}

void mcga_grafik::update_screen() {
	if (svga == ON)
		upd_scr();
}

int16 mcga_grafik::get_vesa_info(uint16 mode, byte *iblk) {
	int16 error = 0;
#if 0
	vesa_status_block *vsb;
	vesa_modus_block *vmb ;
	uint16 vsb_seg, vsb_off;
	char *tmp;
	uint16 *code_list;
	dword screensize;
	int16 i;
	uint16 akt_page;
	uint16 StepGr;
	error = 0;

	vsb = (vesa_status_block *) iblk;
	vmb = (vesa_modus_block *) iblk;
	rr.eax = 0x4f00;
	rr.edi = (dword)vsb & 15;
	rr.es = (uint16)(((dword)vsb) >> 4);
	FuNr = 0x10;
	r_int();
	if ((rr.eax != 0x4f) && (strnicmp((char *)vsb->id, "VESA", 4)))
		error = 1;
	if (!error) {
		printf("VESA Version : %d.%d \n", vsb->ver_high, vsb->ver_low);
		vsb_seg = (uint16)(((dword)vsb->name) >> 16);
		vsb_off = (uint16)(((dword)vsb->name) & 0xffff);
		tmp = (char *)((dword)vsb_seg << 4) + ((dword)vsb_off);
		printf("Hersteller : %s \n", tmp);
		if (vsb->ver_high >= 1)
			printf("SPEICHER : %u * 64k = %uk\n", vsb->memory, vsb->memory << 6);
		else
			error = 1;
	}

	if (!error) {
		vsb_seg = (uint16)(((dword)vsb->codenrs) >> 16);
		vsb_off = (uint16)(((dword)vsb->codenrs) & 0xffff);
		code_list = (uint16 *)(((dword)vsb_seg << 4) + ((dword)vsb_off));
		while ((*code_list != mode) && (*code_list != 0xffff))
			++code_list;
		if (*code_list == 0xffff)
			error = 1;
	}
	if (!error) {
		printf("Mode %4x : ist verfÃ¼gbar\n", mode);

		memset(iblk, 0, sizeof(vesa_status_block));
		rr.eax = 0x4f01;
		rr.ecx = (dword) mode;
		rr.edi = (dword)vmb & 15;
		rr.es = (uint16)(((dword)vmb) >> 4);
		FuNr = 0x10;
		r_int();
		if (rr.eax != 0x4f)
			error = 1;
		if (!error) {
			vi.ModeNr = mode;
			if (vmb->mflag & 1) {
				if (vmb->mflag & 1)printf("Monitor kann Modus darstellen\n");
				if (vmb->mflag & 2)printf("Optionale Infos geliefert\n");
				if (vmb->mflag & 4)printf("BIOS-Textfunktionen mÃ¶glich\n");
				if (vmb->mflag & 0x8)printf("Farbmodus\n");
				if (vmb->mflag & 0x10)printf("Grafikmodus\n");
				if (vmb->fw_flag & 1) {
					if (vmb->fw_flag & 4) {
						vi.WriteWin = 0;
						vi.WriteSeg = vmb->fw_seg;
					}
				}

				if (vmb->fs_flag & 1) {
					if (vmb->fs_flag & 4) {
						vi.WriteWin = 1;
						vi.WriteSeg = vmb->fs_seg;
					}
				}

				switch (mode) {
				case 0x100:
					screensize = 640l * 400l;
					scr_w = 640;
					scr_h = 400;
					break;

				case 0x101:
					screensize = 640l * 480l;
					scr_w = 640;
					scr_h = 480;
					break;

				case 0x102:
					screensize = (800l * 600l) >> 1;
					scr_w = 800;
					scr_h = 600;
					break;

				case 0x103:
					screensize = 800l * 600l;
					scr_w = 800;
					scr_h = 600;
					break;

				case 0x104:
					screensize = (1024l * 768l) >> 1;
					scr_w = 1024;
					scr_h = 768;
					break;

				case 0x105:
					screensize = 1024l * 768l;
					scr_w = 1024;
					scr_h = 768;
					break;

				case 0x106:
					screensize = (1280l * 1024l) >> 1;
					scr_w = 1280;
					scr_h = 1280;
					break;

				case 0x107:
					screensize = 1280l * 1024l;
					scr_w = 1280;
					scr_h = 1280;
					break;

				}
				akt_page = 0;
				StepGr = vmb->wsize / vmb->stepgr;
				if (StepGr == 0)
					StepGr = 1;
				vi.ScreenSize = screensize;
				for (i = 0; (i < 20); i++) {
					vi.Page[i] = akt_page;
					akt_page += StepGr;
				}
				printf("GranularitÃ¤t = %d Kbyte\n", vmb->stepgr);
				printf("GrÃ¶Ãe der Fenster = %d Kbyte\n", vmb->wsize);
				printf("Segmentadresse erstes Fenster = %xh\n", vmb->fw_seg);
				printf("Segmentadresse zweites Fenster = %xh\n", vmb->fs_seg);
				printf("Zeiger auf Page-Select-Routine = %p\n", vmb->page_set);
				vi.WinSize = ((dword)vmb->wsize) * 1024l;
				vi.CallSeg = FP_SEG(vmb->page_set);
				vi.CallOff = FP_OFF(vmb->page_set);
				vi.PageAnz = vi.ScreenSize / vi.WinSize;
				printf("Pages = %d\n", vi.PageAnz);

				vi.CopyRest = vi.ScreenSize % vi.WinSize;
			}
			else
				vi.ModeAvail = FALSE;
		}
	}
#endif
	return (error);
}
