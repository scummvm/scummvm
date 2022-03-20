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

#ifndef CHEWY_OBJECT_H
#define CHEWY_OBJECT_H

#include "chewy/object_extra.h"

namespace Chewy {

#define ENDLOS_FLIP_FLOP 255

#define LOAD_NEW_OBJ 1

#define NO_ACTION 0
#define OBJECT_1 1
#define OBJECT_2 2

class Object {
public:
	Object(GameState *sp);
	~Object();

	short load(const char *filename, RoomMovObject *rmo);
	int16 load(const char *filename, RoomStaticInventory *rsi);
	int16 load(const char *filename, RoomExit *roomExit);
	void sort();
	void free_inv_spr(byte **inv_spr_adr);
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
	void addInventory(int16 nr, RaumBlk *Rb);
	void delInventory(int16 nr, RaumBlk *Rb);
	void changeInventory(int16 old_inv, int16 new_inv, RaumBlk *Rb);
	void setInventory(int16 nr, int16 x, int16 y, int16 automov, RaumBlk *Rb);
	bool checkInventory(int16 nr);
	int16 is_exit(int16 mouse_x, int16 mouse_y);
	uint8 mov_obj_room[MAX_MOV_OBJ + 1];
	uint8 spieler_invnr[MAX_MOV_OBJ + 1];

private:
	int16 calc_rmo_flip_flop(int16 nr);

	IibFileHeader _iibFileHeader;
	SibFileHeader _sibFileHeader;
	EibFileHeader _eibFileHeader;
	int16 _maxInventoryObj;
	int16 _maxStaticInventory;
	int16 _maxExit;
	GameState *_player;
	RoomMovObject *_rmo;
	RoomStaticInventory *_rsi;
	RoomExit *_roomExit;
};

} // namespace Chewy

#endif
