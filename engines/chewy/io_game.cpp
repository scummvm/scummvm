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

#include "common/system.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/file.h"
#include "chewy/io_game.h"

namespace Chewy {

static const int16 FILE_XY_G[7][4] = {
	{ 140,  6, 156, 51 },
	{ 140, 55, 156, 100 },
	{ 163,  6, 241, 18 },
	{ 163, 22, 241, 34 },
	{ 163, 38, 241, 50 },
	{ 163, 54, 241, 66 },
	{   6,  6, 133, 97 }
};

static const byte CODE_TBL_G[6] = { 72, 80, 59, 60, 61, 62 };

static const char AUF_TBL_G[3][3] = {
	{ FSTRING8 }, {FSTRING9 }, { FSTRING10 }
};

static const char AB_TBL_G[4][3] = {
	{ FSTRING11 }, { FSTRING12 }, { FSTRING13 }, { FSTRING14 }
};

IOGame::IOGame(McgaGraphics *iout, InputMgr *iin, Cursor *curp) {
	_out = iout;
	_in = iin;
	_cur = curp;
}

IOGame::~IOGame() {
}

int16 IOGame::io_menu(iog_init *iostruc) {
	int16 max_scroll = 20;
	int16 i, j, y;
	char ende = 0, mouse_f = 1;
	char a = 0;
	int16 y1 = 106;
	int16 ret = 0;
	char io_flag = 0;

	_io = iostruc;
	_inzeig = _in->get_in_zeiger();
	_cur->hide_cur();
	if (_inzeig->minfo)
		_minfo = _inzeig->minfo;
	if (!_inzeig->kbinfo)
		_in->neuer_kb_handler(_kbinfo);
	else
		_kbinfo = _inzeig->kbinfo;

	_scrWidth = _G(scr_w) << 2;
	_dblClick = DOPPEL_KLICK;
	int16 cur_y1 = _io->popy + 4;
	int16 cur_y = _io->popy + 8;
	plot_io();
	schalter_aus();

	for (i = 0; i < 20; i++)
		_fileFind[i][0] = 0;

	int16 scroll_flag = 0;
	i = 0;

	int16 auswahl;
	if (!_io->key_nr) {
		_out->box(_io->popx + 161, _io->popy + 4, _io->popx + 243, _io->popy + 21, _io->m_col[1]);
		auswahl = 1;
	} else {
		_out->box(_io->popx + 161, _io->popy + 4 + ((_io->key_nr - 1) * 16),
		          _io->popx + 243, _io->popy + 21 + ((_io->key_nr - 1) * 16), _io->m_col[1]);
		auswahl = (int16)_io->key_nr;
		cur_y1 = (auswahl - 1) * 16 + _io->popy + 4;
	}

	_cur->show_cur();
	_switchCode = 1;
	_cur->wait_taste_los(true);
	_kbinfo->key_code = '\0';
	_kbinfo->scan_code = Common::KEYCODE_INVALID;

	switch ((int16)_io->key_nr) {
	case 0:
		_cur->hide_cur();
		get_savegame_files();
		i = 0;
		cur_y = _io->popy + 8;
		scroll_flag = 0;
		if (_fileFind[0][0] != 0) {
			if (strlen(&_fileFind[0][1]) >= 17)
				_out->
				printnxy(_io->popx + 28, _io->popy + 10, _io->m_col[5], 300, 17, _scrWidth, &_fileFind[0][1]
				        );
			else
				_out->
				printxy(_io->popx + 28, _io->popy + 10, _io->m_col[5], 300, _scrWidth, &_fileFind[0][1]);
		}
		plot_dir_liste(_io->popy + 18, i + 1);
		_cur->show_cur();
		break;
	case 1:
		_in->_hotkey = 59;
		break;
	case 2:
		_in->_hotkey = 60;
		break;
	case 3:
		_in->_hotkey = 61;
		break;
	case 4:
		_in->_hotkey = 62;
		break;
	}

	_switchCode = 0;
	while (ende == 0) {
		if (mouse_f != 0) {
			mouse_f = 0;
			_kbinfo->key_code = '\0';
		}
		_cur->plot_cur();

		if (_minfo->button == 1) {
			int16 x = _minfo->x;
			y = _minfo->y;

			j = _in->maus_vector(x - _io->popx, y - _io->popy, (int16 *)FILE_XY_G, 7);

			if (j != -1) {
				mouse_f = 1;
				if (j < 6)

					_in->_hotkey = CODE_TBL_G[j];
				if ((j == 6) && (y >= _io->popy + 8)) {
					y -= _io->popy + 8;
					y /= 10;

					if (y < (max_scroll)) {

						if (y + scroll_flag != i) {
							mouse_links_los = false;
							cur_move = true;
							_cur->hide_cur();

							if (io_flag != 0)
								unmark_eintrag(cur_y, i);
							_dblClickStart = g_system->getMillis(); //clock();
							i = y;
							cur_y = i * 10 + _io->popy + 8;
							i += scroll_flag;
							if (io_flag != 0)
								mark_eintrag(cur_y, i);
							_cur->show_cur();
						} else if (mouse_links_los) {
							mouse_links_los = false;
							_dblClickEnd = g_system->getMillis(); //clock();
							if ((_dblClickEnd - _dblClickStart) / 1000 < _dblClick)
								_kbinfo->key_code = 28;
							else
								_dblClickStart = g_system->getMillis(); //clock();
						}
					}
				} else {
					if (j > 2) {
						y -= _io->popy + 8;
						y /= 15;
						if (auswahl != (y + 1)) {
							_cur->hide_cur();
							_out->box(_io->popx + 161, cur_y1, _io->popx + 243, cur_y1 + 17, _io->m_col[5]);
							auswahl = y + 1;
							cur_y1 = (auswahl - 1) * 16 + _io->popy + 4;

							_out->box(_io->popx + 161, cur_y1, _io->popx + 243, cur_y1 + 17, _io->m_col[1]);
							_cur->show_cur();
						}
					}
				}
			}
		}

		_switchCode = _in->get_switch_code();
		switch (_switchCode) {

		case 72:
			_cur->hide_cur();
			if (!io_flag) {
				if (auswahl > 1) {
					_out->box(_io->popx + 161, cur_y1, _io->popx + 243, cur_y1 + 17, _io->m_col[5]);
					--auswahl;
					cur_y1 -= 16;
					_out->box(_io->popx + 161, cur_y1, _io->popx + 243, cur_y1 + 17, _io->m_col[1]);
				}
			} else {

				if (a != 2) {
					_out->pop_box(_io->popx + 140, _io->popy + 55, _io->popx + 156, _io->popy + y1 - 6,
					              _io->m_col[0], _io->m_col[1], _io->m_col[5]);
					plot_ab_txt(_io->m_col[1]);
					a = 2;

					_out->pop_box(_io->popx + 140, _io->popy + 6, _io->popx + 156, _io->popy + 51, _io->m_col[1], _io->m_col[0], _io->m_col[3]);
					plot_auf_txt(_io->m_col[2]);
				}
				if (cur_y > _io->popy + 8) {
					unmark_eintrag(cur_y, i);
					--i;
					cur_y -= 10;
					mark_eintrag(cur_y, i);
				} else if (i > 0) {
					--i;
					--scroll_flag;
					mark_eintrag(cur_y, i);
					plot_dir_liste(_io->popy + 18, i + 1);
				}
			}
			_cur->show_cur();
			break;

		case 80:
			_cur->hide_cur();
			if (!io_flag) {
				if (auswahl < 4) {
					_out->box(_io->popx + 161, cur_y1, _io->popx + 243, cur_y1 + 17, _io->m_col[5]);
					++auswahl;
					cur_y1 += 16;
					_out->box(_io->popx + 161, cur_y1, _io->popx + 243, cur_y1 + 17, _io->m_col[1]);
				}
			} else {

				if (a != 1) {
					_out->pop_box(_io->popx + 140, _io->popy + 6, _io->popx + 156, _io->popy + 51, _io->m_col[0], _io->m_col[1], _io->m_col[5]);
					plot_auf_txt(_io->m_col[1]);
					a = 1;

					_out->pop_box(_io->popx + 140, _io->popy + 55, _io->popx + 156, _io->popy + y1 - 6, _io->m_col[1], _io->m_col[0], _io->m_col[3]);
					plot_ab_txt(_io->m_col[2]);
				}
				if ((cur_y < _io->popy + 6 + 9 * 9) && (i < (max_scroll - 1))) {
					unmark_eintrag(cur_y, i);
					++i;
					cur_y += 10;
					mark_eintrag(cur_y, i);
				} else if (i < (max_scroll - 1)) {
					++i;
					++scroll_flag;
					plot_dir_liste(_io->popy + 8, i - 8);
					mark_eintrag(cur_y, i);
				}
			}
			_cur->show_cur();
			break;

		case 59:
			if (_io->f1) {
				io_flag = 1;
				_cur->hide_cur();
				schalter_aus();
				_out->pop_box(_io->popx + 163, _io->popy + 6, _io->popx + 241, _io->popy + 18, _io->m_col[1], _io->m_col[0], _io->m_col[3]);
				_out->printxy(_io->popx + 167, _io->popy + 9, _io->m_col[2], 300, _scrWidth, FSTRING1);

				get_savegame_files();
				i = 0;
				cur_y = _io->popy + 8;
				scroll_flag = 0;
				plot_dir_liste(_io->popy + 18, i + 1);
				mark_eintrag(cur_y, i);
				_cur->show_cur();
				_cur->wait_taste_los(true);
			}
			break;

		case 60:
			if (_io->f2) {
				io_flag = 2;
				_cur->hide_cur();
				schalter_aus();
				_out->pop_box(_io->popx + 163, _io->popy + 6 + 1 * 16, _io->popx + 241, _io->popy + 18 + 1 * 16, _io->m_col[1], _io->m_col[0], _io->m_col[3]);
				_out->printxy(_io->popx + 167, _io->popy + 25, _io->m_col[2], 300, _scrWidth, FSTRING2);

				get_savegame_files();
				i = 0;
				cur_y = _io->popy + 8;
				scroll_flag = 0;
				plot_dir_liste(_io->popy + 18, i + 1);
				mark_eintrag(cur_y, i);
				_cur->show_cur();
				_cur->wait_taste_los(true);
			}
			break;

		case 61:
			if (_io->f3) {
				io_flag = 0;
				_cur->hide_cur();
				schalter_aus();
				unmark_eintrag(cur_y, i);
				_out->pop_box(_io->popx + 163, _io->popy + 6 + 2 * 16, _io->popx + 241, _io->popy + 18 + 2 * 16, _io->m_col[1], _io->m_col[0], _io->m_col[3]);
				_out->printxy(_io->popx + 167, _io->popy + 41, _io->m_col[2], 300, _scrWidth, FSTRING3);

				_out->printxy(_io->popx + 167, _io->popy + 75, _io->m_col[1], 300, _scrWidth, FSTRING5);
				_out->printxy(_io->popx + 167, _io->popy + 85, _io->m_col[1], 300, _scrWidth, FSTRING6);
				_cur->show_cur();
				ende = 0;
				_kbinfo->key_code = '\0';
				_kbinfo->scan_code = Common::KEYCODE_INVALID;
				while (!ende) {
					_cur->plot_cur();
					SHOULD_QUIT_RETURN0;

					_switchCode = _in->get_switch_code();
					if ((_kbinfo->scan_code == 36) ||
					        (_kbinfo->scan_code == 44) ||
					        (_kbinfo->scan_code == 21)) {
						ende = 1;
						ret = IOG_END;
					} else if ((_switchCode == 1) ||
					           (_kbinfo->scan_code == 49))
						break;
				}
				_cur->wait_taste_los(true);
				_cur->hide_cur();
				_out->box_fill(_io->popx + 167, _io->popy + 70, _io->popx + 244, _io->popy + 100, _io->m_col[5]);
				_out->pop_box(_io->popx + 163, _io->popy + 6 + 2 * 16, _io->popx + 241, _io->popy + 18 + 2 * 16, _io->m_col[0], _io->m_col[1], _io->m_col[5]);
				_out->printxy(_io->popx + 167, _io->popy + 41, _io->m_col[1], 300, _scrWidth, FSTRING3);

				_cur->show_cur();
			}
			break;

		case 62:
			if (_io->f4) {
				io_flag = 0;
				ende = 1;
				ret = IOG_BACK;
			}
			break;

		case 28:
			if (io_flag != 0) {

				if (io_flag == 1) {
					io_flag = 0;
					save(cur_y, i, _io->save_path);
					_cur->hide_cur();
					get_savegame_files();
					mark_eintrag(cur_y, i);
					schalter_aus();

					_cur->show_cur();
					ret = IOG_SAVE;
					ende = 1;
				}

				if ((io_flag == 2) && (_fileFind[i][0] == 1)) {

					load(i, _io->save_path);

					ret = IOG_LOAD;
					ende = 1;
					io_flag = 0;
				}
			} else
				_in->_hotkey = auswahl + 58;
			g_events->delay(200);
			_kbinfo->key_code = '\0';
			break;

		case 1:
			if (io_flag != 0) {
				io_flag = 0;
				_cur->hide_cur();
				schalter_aus();
				unmark_eintrag(cur_y, i);
				_cur->show_cur();
				_cur->wait_taste_los(true);
				_kbinfo->key_code = '\0';
			} else {
				ende = 1;
				ret = IOG_BACK;
			}
			break;

		default:
			if (a != 0) {
				a = 0;
				_cur->hide_cur();
				_out->pop_box(_io->popx + 140, _io->popy + 6, _io->popx + 156, _io->popy + 51, _io->m_col[0], _io->m_col[1], _io->m_col[5]);
				plot_auf_txt(_io->m_col[1]);
				_out->pop_box(_io->popx + 140, _io->popy + 55, _io->popx + 156, _io->popy + y1 - 6, _io->m_col[0], _io->m_col[1], _io->m_col[5]);
				plot_ab_txt(_io->m_col[1]);
				_cur->show_cur();
			}

		}
	}
	_cur->wait_taste_los(true);
	_cur->hide_cur();
	return ret;
}

void IOGame::mark_eintrag(int16 y, int16 nr) {
	_out->pop_box(_io->popx + 8, y, _io->popx + 131, y + 10,
	              _io->m_col[1], _io->m_col[0], _io->m_col[4]);
	_out->printxy(_io->popx + 10, y + 2, _io->m_col[2], 300, _scrWidth, "%d.", nr + 1);
	if (_fileFind[nr][0] != 0) {
		if (strlen(&_fileFind[nr][1]) >= 17)
			_out->printnxy(_io->popx + 28, y + 2, _io->m_col[2], 300, 17,
			               _scrWidth, &_fileFind[nr][1]);
		else
			_out->printxy(_io->popx + 28, y + 2, _io->m_col[2], 300,
			              _scrWidth, &_fileFind[nr][1]);
		_out->box_fill(_io->popx + 8, _io->popy + 106, _io->popx + 244, _io->popy + 117,
		               _io->m_col[5]);
		print_shad(_io->popx + 8, _io->popy + 106, _io->m_col[0], 300, _io->m_col[1],
		           _scrWidth, &_fileFind[nr][1]);
	} else {
		_out->move(_io->popx + 8, _io->popy + 106);
		_out->box_fill(_io->popx + 8, _io->popy + 106, _io->popx + 244, _io->popy + 117,
		               _io->m_col[5]);

	}
}

void IOGame::unmark_eintrag(int16 y, int16 nr) {
	_out->pop_box(_io->popx + 8, y, _io->popx + 131, y + 10, _io->m_col[3],
	              _io->m_col[3], _io->m_col[3]);
	_out->printxy(_io->popx + 10, y + 2, _io->m_col[5], 300, _scrWidth, "%d.", nr + 1);
	if (_fileFind[nr][0] != 0) {
		if (strlen(&_fileFind[nr][1]) >= 17)
			_out->printnxy(_io->popx + 28, y + 2, _io->m_col[5], 300, 17,
			               _scrWidth, &_fileFind[nr][1]);
		else
			_out->printxy(_io->popx + 28, y + 2, _io->m_col[5], 300,
			              _scrWidth, &_fileFind[nr][1]);
	}
}

void IOGame::plot_dir_liste(int16 cur_y, int16 start) {
	for (int16 i = start; i < start + 8; i++) {
		if (i < 20) {

			_out->pop_box(_io->popx + 8, cur_y + 1, _io->popx + 131, cur_y + 10,
			              _io->m_col[3], _io->m_col[3], _io->m_col[3]);
			_out->printxy(_io->popx + 10, cur_y + 2, _io->m_col[5], 300,
			              _scrWidth, "%d.", i + 1);
			if (_fileFind[i][0] != 0) {
				if (strlen(&_fileFind[i][1]) >= 17)
					_out->printnxy(_io->popx + 28, cur_y + 2, _io->m_col[5], 300, 17,
					               _scrWidth, &_fileFind[i][1]);
				else
					_out->printxy(_io->popx + 28, cur_y + 2, _io->m_col[5], 300,
					              _scrWidth, &_fileFind[i][1]);
			}
			cur_y += 10;
		}
	}
}

void IOGame::schalter_aus() {
	_out->pop_box(_io->popx + 163, _io->popy + 6, _io->popx + 241, _io->popy + 18,
	              _io->m_col[0], _io->m_col[1], _io->m_col[5]);
	if (_io->f1)
		_out->printxy(_io->popx + 167, _io->popy + 9, _io->m_col[1], 300, _scrWidth,
		              FSTRING1);
	_out->pop_box(_io->popx + 163, _io->popy + 6 + 16, _io->popx + 241, _io->popy + 18 + 16,
	              _io->m_col[0], _io->m_col[1], _io->m_col[5]);
	if (_io->f2)
		_out->printxy(_io->popx + 167, _io->popy + 25, _io->m_col[1], 300, _scrWidth,
		              FSTRING2);
	_out->pop_box(_io->popx + 163, _io->popy + 6 + 16 * 2, _io->popx + 241, _io->popy + 18 + 16 * 2,
	              _io->m_col[0], _io->m_col[1], _io->m_col[5]);
	if (_io->f3)
		_out->printxy(_io->popx + 167, _io->popy + 41, _io->m_col[1], 300, _scrWidth,
		              FSTRING3);
	_out->pop_box(_io->popx + 163, _io->popy + 6 + 16 * 3, _io->popx + 241, _io->popy + 18 + 16 * 3,
	              _io->m_col[0], _io->m_col[1], _io->m_col[5]);
	if (_io->f4)
		_out->printxy(_io->popx + 167, _io->popy + 57, _io->m_col[1], 300, _scrWidth,
		              FSTRING4);
}

void IOGame::plot_io() {
	const int16 y1 = 120;

	_out->pop_box(_io->popx, _io->popy, _io->popx + 248, _io->popy + y1, _io->m_col[0], _io->m_col[1], _io->m_col[5]);
	_out->pop_box(_io->popx + 1, _io->popy + 1, _io->popx - 1 + 248, _io->popy - 1 + y1, _io->m_col[0], _io->m_col[1], 300);
	_out->pop_box(_io->popx + 6, _io->popy + 6, _io->popx + 133, _io->popy + y1 - 20, _io->m_col[1], _io->m_col[0], _io->m_col[3]);
	_out->pop_box(_io->popx + 140, _io->popy + 6, _io->popx + 156, _io->popy + 51, _io->m_col[0], _io->m_col[1], 300);
	plot_auf_txt(_io->m_col[1]);

	_out->pop_box(_io->popx + 140, _io->popy + 55, _io->popx + 156, _io->popy + y1 - 20,_io->m_col[0], _io->m_col[1], 300);
	plot_ab_txt(_io->m_col[1]);

	for (int16 i = 0; i < 9; i++) {
		_out->printxy(_io->popx + 10, _io->popy + 10 + i * 10,_io->m_col[5], 300, _scrWidth, "%d.", i + 1);
	}
}

void IOGame::plot_auf_txt(int16 farbe) {
	for (int16 i = 0; i < 3; i++)
		_out->printxy(_io->popx + 146, _io->popy + 15 + i * 10, farbe, 300, _scrWidth, AUF_TBL_G[i]);
}

void IOGame::plot_ab_txt(int16 farbe) {
	for (int16 i = 0; i < 4; i++)
		_out->printxy(_io->popx + 146, _io->popy + 59 + i * 10, farbe, 300, _scrWidth, AB_TBL_G[i]);
}

void IOGame::itoa(int N, char *s, int base) {
	sprintf(s, "%d", N);
}

int16 IOGame::get_savegame_files() {
	SaveStateList saveList = g_engine->listSaves();
	int ret = 0;

	for (int i = 0; i < 20; i++) {
		_fileFind[i][0] = 0;

		for (uint j = 0; j < saveList.size(); ++j) {
			if (saveList[j].getSaveSlot() == i) {
				Common::String name = saveList[j].getDescription();
				_fileFind[i][0] = 1;
				strncpy(&_fileFind[i][1], name.c_str(), USER_NAME + 3);
				_fileFind[i][USER_NAME + 3] = '\0';
				++ret;
				break;
			}
		}
	}

	return ret;
}

void IOGame::save(int16 y, int16 slotNum, char *fname) {
	_cur->wait_taste_los(true);
	_in->alter_kb_handler();
	_cur->hide_cur();
	_out->pop_box
	(_io->popx + 8, y, _io->popx + 131, y + 10, _io->m_col[1], _io->m_col[0], _io->m_col[4]);

	_out->printxy(_io->popx + 10, y + 2, _io->m_col[0], 300, _scrWidth, "%d.", slotNum + 1);
	if (_fileFind[slotNum][0] == 0) {
		_out->scanxy(_io->popx + 28, y + 2, _io->m_col[0], _io->m_col[4], _io->m_col[2], _scrWidth, "%36s15", &_fileFind[slotNum][1]);
	} else {
		_out->printxy(_io->popx + 167, _io->popy + 85, _io->m_col[1], 300, _scrWidth, FSTRING7);
		_out->scanxy(_io->popx + 28, y + 2, _io->m_col[0], _io->m_col[4], _io->m_col[2], _scrWidth, "%36s15", &_fileFind[slotNum][1]);

	}
	_in->neuer_kb_handler(_kbinfo);
	mark_eintrag(y, slotNum);
	_out->box_fill(_io->popx + 167, _io->popy + 70, _io->popx + 244, _io->popy + 100, _io->m_col[5]);

	_cur->show_cur();

	Common::String desc(&_fileFind[slotNum][1]);
	(void)g_engine->saveGameState(slotNum, desc);
}

void IOGame::load(int16 slotNum, char *fname) {
	get_savegame_files();

	_cur->hide_cur();
	if (_fileFind[slotNum][0] == 1) {
		(void)g_engine->loadGameState(slotNum);
	}
}

void IOGame::print_shad(int16 x, int16 y, int16 fcol, int16 bcol, int16 scol, int16 width, char *name) {
	_out->printxy(x + 1, y + 1, scol, bcol, width, name);
	_out->printxy(x, y, fcol, bcol, width, name);
}

char *IOGame::io_init(iog_init *iostruc) {
	_io = iostruc;
	for (int16 i = 0; i < 20; i++)
		_fileFind[i][0] = 0;
	get_savegame_files();

	return &_fileFind[0][0];
}

void IOGame::save_entry(int16 slotNum, char *fname) {
	Common::String desc(&_fileFind[slotNum][1]);
	g_engine->saveGameState(slotNum, desc);
}

} // namespace Chewy
