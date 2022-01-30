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

#include "chewy/defines.h"
#include "chewy/global.h"
#include "chewy/ngshext.h"
#include "chewy/objekte.h"

namespace Chewy {

static const int16 ACTION_OBJ_TBL[] = {
	30000,    5,    8,   13,   38,   41,
	30001,   28,   29,
	30002,   31,   35,   56, 5088,
	30003,   38,   39,   11,   15,   13,   41,   42,
	30004,   43,   61,
	30005,   67, 5040, 5088,
	30006,   77,
	30007,    4,
	30008, 5004,
	32000
};

static const int16 SIB_ZUSTAND_TBL[] = {
	30000,   46,    0,
	30001,    6,
	30002,    7,
	30003,    8,
	30004,   12,   13,
	30005,    2,    3,
	30006,    1,
	30007,   41,   45,   46,   47,   48,
	30008,    7,   60,   23,
	30009,   40,   41,   42,   43,   44,   45,   46,   47,   48,
	30010,    4,    5,
	30011,    8,    9,
	30012,    7,   10,
	30013,    3,    5,    6,
	32000
};

objekt::objekt(Spieler *sp) {
	max_inventar_obj = 0;
	max_static_inventar = 0;
	max_exit = 0;
	Player = sp;
	Rmo = sp->room_m_obj;
	Rsi = sp->room_s_obj;
	Re = sp->room_e_obj;
}
objekt::~objekt() {
}

int16 objekt::load(const char *fname_, RoomMovObjekt *rmo) {
	Common::File f;
	bool valid = true;

	if (f.open(fname_)) {
		if (!iib_datei_header.load(&f)) {
			fcode = READFEHLER;
			modul = DATEI;
		} else if (!scumm_strnicmp(iib_datei_header.Id, "IIB", 3)) {
			if (iib_datei_header.Size) {
				assert(iib_datei_header.Size % RoomMovObjekt::SIZE() == 0);

				for (uint i = 0; i < iib_datei_header.Size / RoomMovObjekt::SIZE() && valid;
						++i, ++rmo) {
					valid = rmo->load(&f);
				}

				if (!valid) {
					fcode = READFEHLER;
					modul = DATEI;
				} else {
					max_inventar_obj = (int16)iib_datei_header.Size / RoomMovObjekt::SIZE();
				}
			} else
				max_inventar_obj = 0;
		} else {
			fcode = READFEHLER;
			modul = DATEI;
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	return max_inventar_obj;
}

int16 objekt::load(const char *fname_, RoomStaticInventar *rsi) {
	Common::File f;
	bool valid = true;

	if (f.open(fname_)) {
		if (!sib_datei_header.load(&f)) {
			fcode = READFEHLER;
			modul = DATEI;
		} else if (!scumm_strnicmp(sib_datei_header.Id, "SIB", 3)) {
			if (sib_datei_header.Anz) {
				for (int i = 0; i < sib_datei_header.Anz && valid; ++i, ++rsi) {
					valid = rsi->load(&f);
				}

				if (!valid) {
					fcode = READFEHLER;
					modul = DATEI;
				} else {
					max_static_inventar = sib_datei_header.Anz;
				}
			} else
				max_static_inventar = 0;
		} else {
			fcode = READFEHLER;
			modul = DATEI;
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	return max_static_inventar;
}

int16 objekt::load(const char *fname_, RoomExit *RoomEx) {
	Common::File f;
	bool valid = true;

	if (f.open(fname_)) {
		if (!eib_datei_header.load(&f)) {
			fcode = READFEHLER;
			modul = DATEI;
		} else if (!scumm_strnicmp(eib_datei_header.Id, "EIB", 3)) {
			if (sib_datei_header.Anz) {
				for (int i = 0; i < eib_datei_header.Anz && valid; ++i, ++RoomEx) {
					valid = RoomEx->load(&f);
				}

				if (!valid) {
					fcode = READFEHLER;
					modul = DATEI;
				} else {
					max_exit = eib_datei_header.Anz;
				}
			} else
				max_exit = 0;
		} else {
			fcode = READFEHLER;
			modul = DATEI;
		}

		f.close();
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	return max_exit;
}

void objekt::sort() {
	short i;
	mov_obj_room[0] = 0;

	spieler_invnr[0] = 0;
	for (i = 0; i < MAX_MOV_OBJ; i++) {
		if (Rmo[i].RoomNr != -1) {
			if (Rmo[i].RoomNr == 255) {
				++spieler_invnr[0];
				spieler_invnr[spieler_invnr[0]] = i;
			} else if (Rmo[i].RoomNr == Player->PersonRoomNr[P_CHEWY]) {
				++mov_obj_room[0];
				mov_obj_room[mov_obj_room[0]] = i;
			}
		}
	}
}

void objekt::free_inv_spr(byte **inv_spr_adr) {
	int16 i;
	for (i = 0; i < MAX_MOV_OBJ; i++)
		if (inv_spr_adr[i] != 0) {
			free(inv_spr_adr[i]);
			inv_spr_adr[i] = 0;
		}
}

int16 objekt::is_sib_mouse(int16 mouse_x, int16 mouse_y) {
	int16 i;
	int16 ret;
	ret = -1;
	for (i = 0; i < max_static_inventar && ret == -1; i++) {
		if (Rsi[i].RoomNr == Player->PersonRoomNr[P_CHEWY] && Rsi[i].HideSib == false) {
			if (mouse_x >= Rsi[i].X &&
			        mouse_x <= (Rsi[i].X + Rsi[i].XOff) &&
			        mouse_y >= Rsi[i].Y &&
			        mouse_y <= (Rsi[i].Y + Rsi[i].YOff))
				ret = i;
		}
	}
	return ret;
}

int16 objekt::is_iib_mouse(int16 mouse_x, int16 mouse_y) {
	int16 i;
	int16 ret;
	ret = -1;
	for (i = 1; i < mov_obj_room[0] + 1 && ret == -1; i++) {
		if (Rmo[mov_obj_room[i]].X != -1 &&
		        mouse_x >= Rmo[mov_obj_room[i]].X &&
		        mouse_x <= (Rmo[mov_obj_room[i]].X + Rmo[mov_obj_room[i]].XOff) &&
		        mouse_y >= Rmo[mov_obj_room[i]].Y &&
		        mouse_y <= (Rmo[mov_obj_room[i]].Y + Rmo[mov_obj_room[i]].YOff))
			ret = mov_obj_room[i];
	}
	return ret;
}

int16 objekt::iib_txt_nr(int16 inv_nr) {
	return Rmo[inv_nr].TxtNr;
}

int16 objekt::sib_txt_nr(int16 sib_nr) {
	return Rsi[sib_nr].TxtNr;
}

int16 objekt::action_iib_iib(int16 maus_obj_nr, int16 test_obj_nr) {
	int16 ret;
	int16 i, j;
	int16 ok;
	int16 tmp1, tmp2;
	int16 action_flag;
	ret = NO_ACTION;

	tmp1 = maus_obj_nr;
	tmp2 = test_obj_nr;
	action_flag = 0;

	for (j = 0; j < 2 && !action_flag; j++) {
		if (j) {
			tmp1 = test_obj_nr;
			tmp2 = maus_obj_nr;
		}

		if (Rmo[tmp1].ActionObj != -1) {
			if (Rmo[tmp1].ActionObj < 30000 &&
			        Rmo[tmp1].ActionObj == tmp2) {
				action_flag = 1;
			} else if (Rmo[tmp1].ActionObj >= 30000) {
				i = 0;
				while (ACTION_OBJ_TBL[i] != Rmo[tmp1].ActionObj &&
				        ACTION_OBJ_TBL[i] != 32000) {
					++i;
				}
				if (ACTION_OBJ_TBL[i] != 32000) {
					++i;
					ok = 0;
					while (ACTION_OBJ_TBL[i] < 30000 && !ok) {
						if (ACTION_OBJ_TBL[i] == tmp2 + 5000) {
							ok = 1;
							action_flag = 1;
						}
						++i;
					}
				}
			}
		}
	}

	if (action_flag == 1) {
		if (!calc_rmo_flip_flop(tmp2)) {
			action_flag = NO_ACTION;
		} else {
			if (tmp2 == test_obj_nr)
				ret = OBJEKT_2;
			else
				ret = OBJEKT_1;
		}
	}

	return ret;
}

int16 objekt::action_iib_sib(int16 maus_obj_nr, int16 test_obj_nr) {
	int16 action_flag;
	int16 i;
	int16 ok;
	action_flag = NO_ACTION;

	if (Rmo[maus_obj_nr].ActionObj != -1) {
		if (Rmo[maus_obj_nr].ActionObj < 30000 &&
		        Rmo[maus_obj_nr].ActionObj == test_obj_nr) {
			action_flag = OBJEKT_2;
		} else if (Rmo[maus_obj_nr].ActionObj >= 30000) {
			i = 0;
			while (ACTION_OBJ_TBL[i] != Rmo[maus_obj_nr].ActionObj &&
			        ACTION_OBJ_TBL[i] != 32000) {
				++i;
			}
			if (ACTION_OBJ_TBL[i] != 32000) {
				++i;
				ok = 0;
				while (ACTION_OBJ_TBL[i] < 30000 && !ok) {
					if (ACTION_OBJ_TBL[i] == test_obj_nr) {
						ok = 1;
						action_flag = OBJEKT_2;
					}
					++i;
				}
			}
		}
	}
	if (action_flag == OBJEKT_1) {

		if (!calc_rmo_flip_flop(maus_obj_nr))
			action_flag = NO_ACTION;
	} else if (action_flag == OBJEKT_2) {

		if (!calc_rsi_flip_flop(test_obj_nr))
			action_flag = NO_ACTION;
	}
	return action_flag;
}

void objekt::hide_sib(int16 nr) {
	Rsi[nr].HideSib = true;
}

void objekt::show_sib(int16 nr) {
	Rsi[nr].HideSib = false;
}

void objekt::calc_all_static_detail() {
	int16 i;
	for (i = 0; i < max_static_inventar; i++) {
		calc_static_detail(i);
	}
}

void objekt::calc_static_detail(int16 det_nr) {
	int16 nr;
	int16 i;
	int16 n;
	ani_detail_info *adi;

	if (Rsi[det_nr].RoomNr == Player->PersonRoomNr[P_CHEWY]) {
		nr = Rsi[det_nr].StaticAk;
		if (nr != -1) {
			if (nr >= 30000) {
				i = 0;
				while (SIB_ZUSTAND_TBL[i] != nr && SIB_ZUSTAND_TBL[i] != 32000) {
					++i;
				}
				if (SIB_ZUSTAND_TBL[i] != 32000) {
					++i;
					while (SIB_ZUSTAND_TBL[i] < 30000) {
						nr = SIB_ZUSTAND_TBL[i];

						if (nr >= 40) {
							n = nr - 40;
							adi = det->get_ani_detail(n);
							if (adi->repeat)
								det->start_detail(n, 0, ANI_VOR);
							else
								det->start_detail(n, 1, ANI_VOR);
						} else
							det->show_static_spr(nr);
						++i;
					}
				}
			} else if (nr >= 40) {
				n = nr - 40;
				adi = det->get_ani_detail(n);
				if (adi->repeat)
					det->start_detail(n, 0, ANI_VOR);
				else
					det->start_detail(n, 1, ANI_VOR);
			} else {
				det->show_static_spr(nr);
			}
		}

		nr = Rsi[det_nr].StaticOff;
		if (nr != -1) {
			if (nr >= 30000) {
				i = 0;
				while (SIB_ZUSTAND_TBL[i] != nr && SIB_ZUSTAND_TBL[i] != 32000) {
					++i;
				}
				if (SIB_ZUSTAND_TBL[i] != 32000) {
					++i;
					while (SIB_ZUSTAND_TBL[i] < 30000) {
						nr = SIB_ZUSTAND_TBL[i];

						if (nr >= 40)
							det->stop_detail(nr - 40);
						else
							det->hide_static_spr(nr);
						++i;
					}
				}
			} else if (nr >= 40)
				det->stop_detail(nr - 40);
			else {
				det->hide_static_spr(nr);
			}
		}
	}
}

int16 objekt::calc_static_use(int16 nr) {
	int16 ret;
	switch (Rsi[nr].ZustandAk) {
	case OBJZU_AUF:
	case OBJZU_ZU:
	case OBJZU_AN:
	case OBJZU_AUS:
		if (calc_rsi_flip_flop(nr))
			ret = OBJEKT_1;
		else
			ret = NO_ACTION;
		break;

	case SIB_GET_INV:

		ret = SIB_GET_INV;
		break;

	default:
		ret = NO_ACTION;
		break;

	}
	return ret;
}

int16 objekt::calc_rsi_flip_flop(int16 nr) {
	int16 tmp;
	int16 ret;
	ret = true;
	if (Rsi[nr].ZustandFlipFlop > 0 && Rsi[nr].HideSib == false) {
		tmp = Rsi[nr].ZustandAk;
		Rsi[nr].ZustandAk = Rsi[nr].ZustandOff;
		Rsi[nr].ZustandOff = tmp;
		tmp = Rsi[nr].StaticAk;
		Rsi[nr].StaticAk = Rsi[nr].StaticOff;
		Rsi[nr].StaticOff = tmp;
		if (Rsi[nr].AniFlag == 255 && Rsi[nr].AutoMov == 255)
			calc_static_detail(nr);
		if (Rsi[nr].ZustandFlipFlop != ENDLOS_FLIP_FLOP) {
			--Rsi[nr].ZustandFlipFlop;
		}
	} else {
		ret = false;
	}

	return ret;
}

void objekt::set_rsi_flip_flop(int16 nr, int16 anz) {
	Rsi[nr].ZustandFlipFlop = anz;
}

int16 objekt::calc_rmo_flip_flop(int16 nr) {
	int16 tmp;
	int16 ret;
	if (Rmo[nr].ZustandFlipFlop > 0) {
		ret = true;
		tmp = Rmo[nr].ZustandAk;
		Rmo[nr].ZustandAk = Rmo[nr].ZustandOff;
		Rmo[nr].ZustandOff = tmp;
		if (Rmo[nr].ZustandFlipFlop != ENDLOS_FLIP_FLOP) {
			--Rmo[nr].ZustandFlipFlop;
		}
	} else
		ret = false;
	return ret;
}

int16 objekt::del_obj_use(int16 nr) {
	int16 ret;
	if (Rmo[nr].Del == 1) {
		Rmo[nr].RoomNr = -1;
		sort();
		ret = true;
	} else {
		ret = false;
		if (Rmo[nr].Del != 255) {
			--Rmo[nr].Del;
		}
	}
	return ret;
}

void objekt::add_inventar(int16 nr, RaumBlk *Rb) {
	Player->room_m_obj[nr].RoomNr = 255;
	sort();
	room->calc_invent(Rb, Player);

}

void objekt::del_inventar(int16 nr, RaumBlk *Rb) {

	Player->room_m_obj[nr].RoomNr = -1;
	sort();
}

void objekt::change_inventar(int16 old_inv, int16 new_inv, RaumBlk *Rb) {
	Player->room_m_obj[old_inv].RoomNr = -1;
	Player->room_m_obj[new_inv].RoomNr = 255;
	sort();
	room->calc_invent(Rb, Player);

}

void objekt::set_inventar(int16 nr, int16 x, int16 y, int16 automov,
                          RaumBlk *Rb) {

	++mov_obj_room[0];
	mov_obj_room[mov_obj_room[0]] = nr;
	Player->room_m_obj[nr].RoomNr = Player->PersonRoomNr[P_CHEWY];
	Player->room_m_obj[nr].X = x;
	Player->room_m_obj[nr].Y = y;
	Player->room_m_obj[nr].AutoMov = automov;
	room->calc_invent(Rb, Player);
	sort();
}

int16 objekt::check_inventar(int16 nr) {
	int16 i;
	int16 ret;
	ret = false;
	for (i = 0; i < spieler_invnr[0] && !ret; i++) {
		if (spieler_invnr[i + 1] == nr)
			ret = true;
	}
	return ret;
}

int16 objekt::is_exit(int16 mouse_x, int16 mouse_y) {
	int16 ret;
	int16 i;
	ret = -1;

	for (i = 0; i < max_exit && ret == -1; i++) {
		if (Re[i].RoomNr == Player->PersonRoomNr[P_CHEWY]) {
			if (mouse_x >= Re[i].X &&
			        mouse_x <= (Re[i].X + Re[i].XOff) &&
			        mouse_y >= Re[i].Y &&
			        mouse_y <= (Re[i].Y + Re[i].YOff)) {
				ret = i;
			}
		}
	}

	return ret;
}

} // namespace Chewy
