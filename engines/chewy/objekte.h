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

#ifndef CHEWY_OBJEKTE_H
#define CHEWY_OBJEKTE_H

#include "engines/chewy/objekt.h"

#define ENDLOS_FLIP_FLOP 255

#define LOAD_NEW_OBJ 1

#define NO_ACTION 0
#define OBJEKT_1 1
#define OBJEKT_2 2

class objekt {

public:
	objekt(Spieler *sp);
	~objekt();

	short load(const char *fname, RoomMovObjekt *Rmo);

	int16 load(const char *fname, RoomStaticInventar *Rsi);

	int16 load(const char *fname, RoomExit *RoomEx);

	void sort();

	void free_inv_spr(char **inv_spr_adr);

	int16 is_sib_mouse(int16 mouse_x, int16 mouse_y);

	int16 is_iib_mouse(int16 mouse_x, int16 mouse_y);

	int16 iib_txt_nr(int16 inv_nr);

	int16 sib_txt_nr(int16 sib_nr);

	int16 action_iib_iib(int16 maus_obj_nr, int16 test_obj_nr);

	int16 action_iib_sib(int16 maus_obj_nr, int16 test_obj_nr);

	void hide_sib(int16 nr);

	void show_sib(int16 nr);

	void calc_all_static_detail();

	void calc_static_detail(int16 det_nr);

	int16 calc_static_use(int16 nr);

	int16 del_obj_use(int16 nr);

	int16 calc_rsi_flip_flop(int16 nr);

	void set_rsi_flip_flop(int16 nr, int16 anz);

	void add_inventar(int16 nr, RaumBlk *Rb);

	void del_inventar(int16 nr, RaumBlk *Rb);

	void change_inventar(int16 old_inv, int16 new_inv, RaumBlk *Rb);

	void set_inventar(int16 nr, int16 x, int16 y, int16 automov,
	                  RaumBlk *Rb);

	int16 check_inventar(int16 nr);

	int16 is_exit(int16 mouse_x, int16 mouse_y);

	uint8 mov_obj_room[MAX_MOV_OBJ + 1];

	uint8 spieler_invnr[MAX_MOV_OBJ + 1];

private:

	int16 calc_rmo_flip_flop(int16 nr);

	IibDateiHeader iib_datei_header;
	SibDateiHeader sib_datei_header;
	EibDateiHeader eib_datei_header;
	int16 max_inventar_obj;
	int16 max_static_inventar;
	int16 max_exit;
	Spieler *Player;
	RoomMovObjekt *Rmo;
	RoomStaticInventar *Rsi;
	RoomExit *Re;

};
#endif
