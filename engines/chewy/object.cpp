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
#include "chewy/globals.h"
#include "chewy/object.h"

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

Object::Object(GameState *sp) {
	_maxInventoryObj = 0;
	_maxStaticInventory = 0;
	_maxExit = 0;
	_player = sp;
	_rmo = sp->room_m_obj;
	_rsi = sp->room_s_obj;
	_roomExit = sp->room_e_obj;
}
Object::~Object() {
}

int16 Object::load(const char *filename, RoomMovObject *rmo) {
	Common::File f;

	if (f.open(filename)) {
		if (!_iibFileHeader.load(&f)) {
			error("Object::load error");
		}

		if (!scumm_strnicmp(_iibFileHeader.Id, "IIB", 3)) {
			if (_iibFileHeader.Size) {
				assert(_iibFileHeader.Size % RoomMovObject::SIZE() == 0);

				bool valid = true;
				for (uint i = 0; i < _iibFileHeader.Size / RoomMovObject::SIZE() && valid; ++i, ++rmo) {
					valid = rmo->load(&f);
				}

				if (!valid)
					error("Object::load error");

				_maxInventoryObj = (int16)_iibFileHeader.Size / RoomMovObject::SIZE();
			} else
				_maxInventoryObj = 0;
		} else {
			error("Object::load error");
		}

		f.close();
	} else {
		error("Object::load error");
	}

	return _maxInventoryObj;
}

int16 Object::load(const char *filename, RoomStaticInventory *rsi) {
	Common::File f;

	if (f.open(filename)) {
		if (!_sibFileHeader.load(&f)) {
			error("Object::load error");
		}

		if (!scumm_strnicmp(_sibFileHeader._id, "SIB", 3)) {
			if (_sibFileHeader._nr) {
				bool valid = true;
				for (int i = 0; i < _sibFileHeader._nr && valid; ++i, ++rsi) {
					valid = rsi->load(&f);
				}

				if (!valid)
					error("Object::load error");

				_maxStaticInventory = _sibFileHeader._nr;
			} else
				_maxStaticInventory = 0;
		} else {
			error("Object::load error");
		}

		f.close();
	} else {
		error("Object::load error");
	}

	return _maxStaticInventory;
}

int16 Object::load(const char *filename, RoomExit *roomExit) {
	Common::File f;

	if (f.open(filename)) {
		if (!_eibFileHeader.load(&f)) {
			error("Object::load error");
		}

		if (!scumm_strnicmp(_eibFileHeader._id, "EIB", 3)) {
			if (_sibFileHeader._nr) {
				bool valid = true;
				for (int i = 0; i < _eibFileHeader._nr && valid; ++i, ++roomExit) {
					valid = roomExit->load(&f);
				}

				if (!valid)
					error("Object::load error");

				_maxExit = _eibFileHeader._nr;
			} else
				_maxExit = 0;
		} else {
			error("Object::load error");
		}

		f.close();
	} else {
		error("Object::load error");
	}

	return _maxExit;
}

void Object::sort() {
	mov_obj_room[0] = 0;

	spieler_invnr[0] = 0;
	for (short i = 0; i < MAX_MOV_OBJ; i++) {
		if (_rmo[i].RoomNr != -1) {
			if (_rmo[i].RoomNr == 255) {
				++spieler_invnr[0];
				spieler_invnr[spieler_invnr[0]] = i;
			} else if (_rmo[i].RoomNr == _player->_personRoomNr[P_CHEWY]) {
				++mov_obj_room[0];
				mov_obj_room[mov_obj_room[0]] = i;
			}
		}
	}
}

void Object::free_inv_spr(byte **inv_spr_adr) {
	for (int16 i = 0; i < MAX_MOV_OBJ; i++)
		if (inv_spr_adr[i] != 0) {
			free(inv_spr_adr[i]);
			inv_spr_adr[i] = 0;
		}
}

int16 Object::is_sib_mouse(int16 mouse_x, int16 mouse_y) {
	int16 ret = -1;
	for (int16 i = 0; i < _maxStaticInventory && ret == -1; i++) {
		if (_rsi[i].RoomNr == _player->_personRoomNr[P_CHEWY] && _rsi[i].HideSib == false) {
			if (mouse_x >= _rsi[i].X &&
			        mouse_x <= (_rsi[i].X + _rsi[i].XOff) &&
			        mouse_y >= _rsi[i].Y &&
			        mouse_y <= (_rsi[i].Y + _rsi[i].YOff))
				ret = i;
		}
	}
	return ret;
}

int16 Object::is_iib_mouse(int16 mouse_x, int16 mouse_y) {
	int16 ret = -1;
	for (int16 i = 1; i < mov_obj_room[0] + 1 && ret == -1; i++) {
		if (_rmo[mov_obj_room[i]].X != -1 &&
		        mouse_x >= _rmo[mov_obj_room[i]].X &&
		        mouse_x <= (_rmo[mov_obj_room[i]].X + _rmo[mov_obj_room[i]].XOff) &&
		        mouse_y >= _rmo[mov_obj_room[i]].Y &&
		        mouse_y <= (_rmo[mov_obj_room[i]].Y + _rmo[mov_obj_room[i]].YOff))
			ret = mov_obj_room[i];
	}
	return ret;
}

int16 Object::iib_txt_nr(int16 inv_nr) {
	return _rmo[inv_nr].TxtNr;
}

int16 Object::sib_txt_nr(int16 sib_nr) {
	return _rsi[sib_nr].TxtNr;
}

int16 Object::action_iib_iib(int16 maus_obj_nr, int16 test_obj_nr) {
	int16 ret = NO_ACTION;

	int16 tmp1 = maus_obj_nr;
	int16 tmp2 = test_obj_nr;
	bool actionFl = false;

	for (int16 j = 0; j < 2 && !actionFl; j++) {
		if (j) {
			tmp1 = test_obj_nr;
			tmp2 = maus_obj_nr;
		}

		if (_rmo[tmp1].ActionObj != -1) {
			if (_rmo[tmp1].ActionObj < 30000 && _rmo[tmp1].ActionObj == tmp2) {
				actionFl = true;
			} else if (_rmo[tmp1].ActionObj >= 30000) {
				int16 i = 0;
				while (ACTION_OBJ_TBL[i] != _rmo[tmp1].ActionObj &&
				        ACTION_OBJ_TBL[i] != 32000) {
					++i;
				}
				if (ACTION_OBJ_TBL[i] != 32000) {
					++i;
					int16 ok = 0;
					while (ACTION_OBJ_TBL[i] < 30000 && !ok) {
						if (ACTION_OBJ_TBL[i] == tmp2 + 5000) {
							ok = 1;
							actionFl = true;
						}
						++i;
					}
				}
			}
		}
	}

	if (actionFl && calc_rmo_flip_flop(tmp2))
		ret = (tmp2 == test_obj_nr) ? OBJECT_2 : OBJECT_1;

	return ret;
}

int16 Object::action_iib_sib(int16 maus_obj_nr, int16 test_obj_nr) {
	int16 action_flag = NO_ACTION;

	if (_rmo[maus_obj_nr].ActionObj != -1) {
		if (_rmo[maus_obj_nr].ActionObj < 30000 &&
		        _rmo[maus_obj_nr].ActionObj == test_obj_nr) {
			action_flag = OBJECT_2;
		} else if (_rmo[maus_obj_nr].ActionObj >= 30000) {
			int16 i = 0;
			while (ACTION_OBJ_TBL[i] != _rmo[maus_obj_nr].ActionObj &&
			        ACTION_OBJ_TBL[i] != 32000) {
				++i;
			}
			if (ACTION_OBJ_TBL[i] != 32000) {
				++i;
				int16 ok = 0;
				while (ACTION_OBJ_TBL[i] < 30000 && !ok) {
					if (ACTION_OBJ_TBL[i] == test_obj_nr) {
						ok = 1;
						action_flag = OBJECT_2;
					}
					++i;
				}
			}
		}
	}

	if (action_flag == OBJECT_2 && !calc_rsi_flip_flop(test_obj_nr))
		action_flag = NO_ACTION;

	return action_flag;
}

void Object::hide_sib(int16 nr) {
	_rsi[nr].HideSib = true;
}

void Object::show_sib(int16 nr) {
	_rsi[nr].HideSib = false;
}

void Object::calc_all_static_detail() {
	for (int16 i = 0; i < _maxStaticInventory; i++) {
		calc_static_detail(i);
	}
}

void Object::calc_static_detail(int16 det_nr) {
	int16 i;
	int16 n;

	if (_rsi[det_nr].RoomNr == _player->_personRoomNr[P_CHEWY]) {
		int16 nr = _rsi[det_nr].StaticAk;
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
							AniDetailInfo *adi = _G(det)->getAniDetail(n);
							if (adi->repeat)
								_G(det)->startDetail(n, 0, ANI_FRONT);
							else
								_G(det)->startDetail(n, 1, ANI_FRONT);
						} else
							_G(det)->showStaticSpr(nr);
						++i;
					}
				}
			} else if (nr >= 40) {
				n = nr - 40;
				AniDetailInfo *adi = _G(det)->getAniDetail(n);
				if (adi->repeat)
					_G(det)->startDetail(n, 0, ANI_FRONT);
				else
					_G(det)->startDetail(n, 1, ANI_FRONT);
			} else {
				_G(det)->showStaticSpr(nr);
			}
		}

		nr = _rsi[det_nr].StaticOff;
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
							_G(det)->stopDetail(nr - 40);
						else
							_G(det)->hideStaticSpr(nr);
						++i;
					}
				}
			} else if (nr >= 40)
				_G(det)->stopDetail(nr - 40);
			else {
				_G(det)->hideStaticSpr(nr);
			}
		}
	}
}

int16 Object::calc_static_use(int16 nr) {
	int16 ret;
	switch (_rsi[nr].ZustandAk) {
	case OBJZU_AUF:
	case OBJZU_ZU:
	case OBJZU_AN:
	case OBJZU_AUS:
		if (calc_rsi_flip_flop(nr))
			ret = OBJECT_1;
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

int16 Object::calc_rsi_flip_flop(int16 nr) {
	int16 ret = true;
	if (_rsi[nr].ZustandFlipFlop > 0 && _rsi[nr].HideSib == false) {
		int16 tmp = _rsi[nr].ZustandAk;
		_rsi[nr].ZustandAk = _rsi[nr].ZustandOff;
		_rsi[nr].ZustandOff = tmp;
		tmp = _rsi[nr].StaticAk;
		_rsi[nr].StaticAk = _rsi[nr].StaticOff;
		_rsi[nr].StaticOff = tmp;
		if (_rsi[nr].AniFlag == 255 && _rsi[nr].AutoMov == 255)
			calc_static_detail(nr);
		if (_rsi[nr].ZustandFlipFlop != ENDLOS_FLIP_FLOP) {
			--_rsi[nr].ZustandFlipFlop;
		}
	} else {
		ret = false;
	}

	return ret;
}

void Object::set_rsi_flip_flop(int16 nr, int16 anz) {
	_rsi[nr].ZustandFlipFlop = anz;
}

int16 Object::calc_rmo_flip_flop(int16 nr) {
	int16 ret;
	if (_rmo[nr].ZustandFlipFlop > 0) {
		ret = true;
		int16 tmp = _rmo[nr].ZustandAk;
		_rmo[nr].ZustandAk = _rmo[nr].ZustandOff;
		_rmo[nr].ZustandOff = tmp;
		if (_rmo[nr].ZustandFlipFlop != ENDLOS_FLIP_FLOP) {
			--_rmo[nr].ZustandFlipFlop;
		}
	} else
		ret = false;
	return ret;
}

int16 Object::del_obj_use(int16 nr) {
	int16 ret;
	if (_rmo[nr].Del == 1) {
		_rmo[nr].RoomNr = -1;
		sort();
		ret = true;
	} else {
		ret = false;
		if (_rmo[nr].Del != 255) {
			--_rmo[nr].Del;
		}
	}
	return ret;
}

void Object::addInventory(int16 nr, RaumBlk *Rb) {
	_player->room_m_obj[nr].RoomNr = 255;
	sort();
	_G(room)->calc_invent(Rb, _player);

}

void Object::delInventory(int16 nr, RaumBlk *Rb) {
	_player->room_m_obj[nr].RoomNr = -1;
	sort();
}

void Object::changeInventory(int16 old_inv, int16 new_inv, RaumBlk *Rb) {
	_player->room_m_obj[old_inv].RoomNr = -1;
	_player->room_m_obj[new_inv].RoomNr = 255;
	sort();
	_G(room)->calc_invent(Rb, _player);
}

void Object::setInventory(int16 nr, int16 x, int16 y, int16 automov, RaumBlk *Rb) {
	++mov_obj_room[0];
	mov_obj_room[mov_obj_room[0]] = nr;
	_player->room_m_obj[nr].RoomNr = _player->_personRoomNr[P_CHEWY];
	_player->room_m_obj[nr].X = x;
	_player->room_m_obj[nr].Y = y;
	_player->room_m_obj[nr].AutoMov = automov;
	_G(room)->calc_invent(Rb, _player);
	sort();
}

bool Object::checkInventory(int16 nr) {
	bool ret = false;
	for (int16 i = 0; i < spieler_invnr[0] && !ret; i++) {
		if (spieler_invnr[i + 1] == nr)
			ret = true;
	}
	return ret;
}

int16 Object::is_exit(int16 mouse_x, int16 mouse_y) {
	int16 ret = -1;
	for (int16 i = 0; i < _maxExit && ret == -1; i++) {
		if (_roomExit[i].RoomNr == _player->_personRoomNr[P_CHEWY]) {
			if (mouse_x >= _roomExit[i].X &&
			        mouse_x <= (_roomExit[i].X + _roomExit[i].XOff) &&
			        mouse_y >= _roomExit[i].Y &&
			        mouse_y <= (_roomExit[i].Y + _roomExit[i].YOff)) {
				ret = i;
			}
		}
	}
	return ret;
}

} // namespace Chewy
