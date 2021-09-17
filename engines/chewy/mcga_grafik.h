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

#ifndef CHEWY_MCGA_GRAFIK_H
#define CHEWY_MCGA_GRAFIK_H

#include "graphics/screen.h"
#include "chewy/maus.h"
#include "chewy/ngstypes.h"

namespace Chewy {

extern byte pal_table[PALETTE_SIZE];

class mcga_grafik {
public:
	mcga_grafik();
	~mcga_grafik();

	void init();
	void restore_mode();
	void set_writemode(char wm);
	void init_mausmode(maus_info *minfo);
	void exit_mausmode();

	void set_clip(int16 x1, int16 y1, int16 x2, int16 y2);
	void vsync_start();
	void vsync_end();
	void hsync_start();
	void hsync_end();
	void skip_line(int16 lines);
	void skip_frame(int16 frames);
	void setze_zeiger(byte *ptr);
	byte *get_zeiger();
	void set_bildbreite(int16 breite);

	void set_mono();
	void calc_mono(byte *pal, int16 startcol, int16 anz);
	void set_palette(byte *palette);
	void palette_save(byte *pal = nullptr);
	void rest_palette();
	void raster_col(int16 c, int16 r, int16 g, int16 b);
	void einblenden(byte *palette, int16 frames);
	void aufhellen(byte *palette, int16 startcol, int16 anz, int16 stufen,
	               int16 frames);
	void ausblenden(int16 frames);
	void abblenden(int16 startcol, int16 anz, int16 stufen, int16 frames);
	void set_teilpalette(byte *palette, int16 startcol, int16 anz);

	void cls();
	void punkt(int16 xpos, int16 ypos, int16 farbn);
	uint8 get_pixel(int16 xpos, int16 ypos);
	void linie(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);
	void box(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);
	void box_fill(int16 x1, int16 y1, int16 x2, int16 y2, int16 farbe);
	void pop_box(int16 x, int16 y, int16 x1, int16 y1,
	             int16 col1, int16 col2, int16 back_col);
	void kreis(int16 x, int16 y, int16 r, int16 farbe);
	void fkreis(int16 x, int16 y, int16 r, int16 farbe);

#ifdef EFFEKTE
	void upscroll();
	void downscroll();
	void leftscroll();
	void rightscroll();
	void set_dispoff(int16 offset);
	void seit_in(byte *source);
	void falling_in(byte *source);
	void ueberblend(byte *source);
	void y_shrumpf(byte *source, byte *dest, int16 faktor,
	               int16 zeile);
#endif

	void back2screen(byte *ptr);
	void back2screen_maskiert(byte *ptr, int16 maske);
	void screen2back(byte *ptr);
	void back2back(byte *ptr1, byte *ptr2);
	void back2back_maskiert(byte *ptr1, byte *ptr2, int16 maske);

	void sprite_save(byte *sptr, int16 x, int16 y, int16 breite,
	                 int16 hoehe, int16 scrwidth);
	void blockcopy(byte *sptr, int16 x, int16 y, int16 scrwidth);
	void sprite_set(byte *sptr, int16 x, int16 y, int16 scrwidth);
	void scale_image(byte *source, byte *dest, int16 xdiff,
	                 int16 ydiff);
	void scale_set(byte *sptr, int16 x, int16 y, int16 xdiff,
	               int16 ydiff, int16 scrwidth);
	void map_spr2screen(byte *sptr, int16 x, int16 y);

	void set_fontadr(byte *adr);
	void plot_scan_cur(int16 x, int16 y, int16 fcol, int16 bcol, int16 scrwidth,
	                   char cursor);
	int16 scanxy(int16 x, int16 y, int16 fcol, int16 bcol, int16 cur_col, int16 scrwidth, const char *string, ...);
	void printxy(int16 x, int16 y, int16 forcol, int16 backcol, int16 scrwidth,
	             const char *format, ...);
	void speed_printxy(int16 x, int16 y, int16 forcol, int16 backcol,
	                   int16 scrwidth, char *string);
	void print(int16 forcol, int16 backcol, int16 scrwidth, char *format, ...);
	void printnxy(int16 x, int16 y, int16 forcol, int16 backcol, int16 menge,
	              int16 scrwidth, char *format, ...);
	void printcharxy(int16 x, int16 y, char zeichen, int16 forcol, int16 backcol,
	                 int16 scrwidth);
	void printchar(char zeichen, int16 forcol, int16 backcol, int16 scrwidth);
	void vorschub();
	void set_vorschub(int16 x, int16 y);
	void move(int16 x, int16 y);
	void get_fontinfo(int16 *vorx, int16 *vory, int16 *fntbr, int16 *fnth);

	int16 get_vesa_info(uint16 mode, byte *info_blk);
	void init(uint16 mode, byte *info_blk, byte *vscreen);
	void update_screen();
private:
	int16 check_stellen_anz(char *zstring, int16 *pos, int16 stellen);
	int16 devices();
	float sinus[360];
	float cosinus[360];

	void ltoa(long N, char *str, int base);
	void ultoa(uint32 N, char *str, int base);
	void itoa(int N, char *str, int base);

};

} // namespace Chewy

#endif
