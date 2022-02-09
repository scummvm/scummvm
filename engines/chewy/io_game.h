/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_IOGAME_H
#define CHEWY_IOGAME_H

#include "chewy/mcga.h"
#include "chewy/cursor.h"
#include "chewy/ngstypes.h"

namespace Chewy {

#ifdef ENGLISCH

#define FSTRING1 "F1 Save"
#define FSTRING2 "F2 Load"
#define FSTRING3 "F3 Quit"
#define FSTRING4 "F4 Return"
#define FSTRING5 " Are You "
#define FSTRING6 "Sure (Y/N)?"
#define FSTRING7 "ESC Cancel"
#define FSTRING8 "U"
#define FSTRING9 " "
#define FSTRING10 "P"
#define FSTRING11 "D"
#define FSTRING12 "O"
#define FSTRING13 "W"
#define FSTRING14 "N"
#else

#define FSTRING1 "F1 Speichern"
#define FSTRING2 "F2 Laden"
#define FSTRING3 "F3 Beenden "
#define FSTRING4 "F4 ZurÃ¼ck"
#define FSTRING5 " Bist Du "
#define FSTRING6 "sicher (J/N)?"
#define FSTRING7 "ESC Abbrechen"
#define FSTRING8 "A"
#define FSTRING9 "U"
#define FSTRING10 "F"
#define FSTRING11 " "
#define FSTRING12 "A"
#define FSTRING13 "B"
#define FSTRING14 " "
#endif

#define IOG_END 1
#define IOG_LOAD 2
#define IOG_SAVE 3
#define IOG_BACK 4
#define USER_NAME 36

#define DOPPEL_KLICK 0.5

class io_game {

public:
	io_game(McgaGraphic *out, InputMgr *in, cursor *curp);
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
	int16 get_savegame_files();
	void get_switch_code();

	void itoa(int N, char *str, int base);

	McgaGraphic *out;
	InputMgr *in;
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

} // namespace Chewy

#endif
