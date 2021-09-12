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

#ifndef CHEWY_IOGAME_H
#define CHEWY_IOGAME_H

#include "chewy/mcga.h"
#include "chewy/cursor.h"
#include "chewy/ngstypes.h"

#ifdef ENGLISCH

#define FSTRING1 "F1 Save\0"
#define FSTRING2 "F2 Load\0"
#define FSTRING3 "F3 Quit\0"
#define FSTRING4 "F4 Return\0"
#define FSTRING5 " Are You \0"
#define FSTRING6 "Sure (Y/N)?\0"
#define FSTRING7 "ESC Cancel\0"
#define FSTRING8 "U\0"
#define FSTRING9 " \0"
#define FSTRING10 "P\0"
#define FSTRING11 "D\0"
#define FSTRING12 "O\0"
#define FSTRING13 "W\0"
#define FSTRING14 "N\0"
#else

#define FSTRING1 "F1 Speichern\0"
#define FSTRING2 "F2 Laden\0"
#define FSTRING3 "F3 Beenden \0"
#define FSTRING4 "F4 ZurÃ¼ck\0"
#define FSTRING5 " Bist Du \0"
#define FSTRING6 "sicher (J/N)?\0"
#define FSTRING7 "ESC Abbrechen\0"
#define FSTRING8 "A\0"
#define FSTRING9 "U\0"
#define FSTRING10 "F\0"
#define FSTRING11 " \0"
#define FSTRING12 "A\0"
#define FSTRING13 "B\0"
#define FSTRING14 " \0"
#endif

#define IOG_END 1
#define IOG_LOAD 2
#define IOG_SAVE 3
#define IOG_BACK 4
#define USER_NAME 36

#define DOPPEL_KLICK 0.5

class io_game {

public:
	io_game(mcga_grafik *out, maus *in, cursor *curp);
	~io_game();

	int16 io_menu(iog_init *iostruc);

	void print_shad(int16 x, int16 y,
	                int16 fcol, int16 bcol,
	                int16 scol, int16 width,
	                char *name);
	char *io_init(iog_init *iostruc);
	void load(int16 nr, char *fname);
	void save_entry(int16 nr, char *fname);

private:
	void save(int16 y, int16 nr, char *fname);
	void plot_io();
	void plot_auf_txt(int16 farbe);
	void plot_ab_txt(int16 farbe);
	void mark_eintrag(int16 y, int16 nr);
	void unmark_eintrag(int16 y, int16 nr);
	void plot_dir_liste(int16 cur_y, int16 start);
	void schalter_aus();
	int16 get_files(char *fname);
	void get_switch_code();

	void itoa(int N, char *str, int base);

	mcga_grafik *out;
	maus *in;
	cursor *cur;
	iog_init *io;
	long dklick_start, dklick_end;
	kb_info *kbinfo;
	maus_info *minfo;
	in_zeiger *inzeig;
	int16 scr_width;
	float d_klick;
	int16 switch_code;

};
#endif
