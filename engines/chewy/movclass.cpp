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

#include "chewy/movclass.h"
#include "chewy/defines.h"
#include "chewy/globals.h"

namespace Chewy {

#define LEFT_VECT -1
#define RIGHT_VECT 1
#define UP_VECT -_gpkt->Breite
#define DOWN_VECT _gpkt->Breite
#define MOV_START 0
#define MOV_FOUND 1
#define MOV_UNPASSABLE 2
#define Y_DOWN 1
#define Y_UP 2
#define SOURCE_COL 14
#define DEST_COL 19
#define CHECK_COL 255
#define MIN_COL 8
#define UNBEGEHBAR_COL 3
#define CONNECT_COL 15
#define CUR_COL 13
#define S_OBEN 0
#define S_RECHTS 1
#define S_UNTEN 2
#define S_LINKS 3
#define MOV_X 1
#define MOV_Y 2

MovClass::MovClass() {
	_vecTbl[0] = -40;
	_vecTbl[1] = 1;
	_vecTbl[2] = 40;
	_vecTbl[3] = -1;
}

MovClass::~MovClass() {
}

void MovClass::goto_xy(GotoPkt *gp) {
	_gpkt = gp;
	_agv.AutoGo = false;
	_agv.Continue = false;
	_vecTbl[0] = -_gpkt->Breite;
	_vecTbl[2] = _gpkt->Breite;
	_plotDelay = 20;
	calc_xy();
	_plotDelay = 0;

	int16 start_feld = get_feld_nr(_gpkt->Sx, _gpkt->Sy);
	int16 ziel_feld = get_feld_nr(_gpkt->Dx, _gpkt->Dy);
	int16 tmp = ziel_feld;
	calc_go(start_feld, &ziel_feld);

	if (tmp != ziel_feld) {
		get_mov_line();
		if (_gml.MLineFeld != -1) {
			calc_mov_line_xit(_gml.MLineFeld);
			if (_mle.FNr != -1) {
				_agv.AutoGo = true;
				_agv.Start = 0;
				_agv.LastFeld = -1;
				_agv.AkKnoten = 0;
				start_feld = get_feld_nr(_gpkt->Sx, _gpkt->Sy);
				ziel_feld = _gml.MLineFeld;
				calc_go(start_feld, &ziel_feld);
				if (_gml.MLineFeld != ziel_feld) {
					_agv.PktAnz = 2;
				} else {
					_agv.PktAnz = 1;
				}
			}
		}
	}
}

int16 MovClass::calc_auto_go(int16 x, int16 y, int16 *auto_x, int16 *auto_y) {
	int16 ret = -1;

	if (_agv.AutoGo == true) {
		byte *speicher = _gpkt->Mem;
		speicher += (_gpkt->Breite * _gpkt->Hoehe) * _gpkt->AkMovEbene;
		if (!_agv.Start) {
			_agv.Start = _agv.PktAnz;
			if (_agv.PktAnz == 1) {
				_agv.Start = 3;
				get_feld_xy(_gml.MLineFeld, &_agv.X_End, &_agv.Y_End);
				_agv.AkFeld = _gml.MLineFeld;
			} else {
				get_feld_xy(_gml.GotoFeld, &_agv.X_End, &_agv.Y_End);
				_agv.AkFeld = _gml.GotoFeld;
			}
			*auto_x = _agv.X_End + 4;
			*auto_y = _agv.Y_End + 4;
			ret = true;
		} else {
			if (get_feld_nr(x, y) == _agv.AkFeld) {
				if (_agv.Start == 2) {
					_agv.Start = 3;
					get_feld_xy(_gml.GotoFeld, &_agv.X_End, &_agv.Y_End);
					*auto_x = _agv.X_End + 4;
					*auto_y = _agv.Y_End + 4;
					ret = true;
				} else if (_agv.Start == 3) {
					if (_agv.LastFeld == -1) {
						_agv.LastFeld = _gml.MLineFeld;
						_agv.AkFeld = _gml.MLineFeld + _mle.Direction;
						if (speicher[_agv.AkFeld] == MOV_LINE_KNOTEN) {
							_feld1knoten = true;
						} else
							_feld1knoten = false;
						get_feld_xy(_agv.AkFeld, &_agv.X_End, &_agv.Y_End);
						*auto_x = _agv.X_End + 4;
						*auto_y = _agv.Y_End + 4;
						ret = true;
					} else if (get_feld_nr(x, y) == _agv.AkFeld) {
						if (_agv.AkFeld == _mle.FNr) {
							_agv.AutoGo = false;
							*auto_x = _gpkt->Dx;
							*auto_y = _gpkt->Dy;
							ret = true;
						} else {
							int16 ok = 0;
							if (_feld1knoten) {
								_feld1knoten = false;
								_agv.LastFeld = _agv.AkFeld;
								_agv.AkFeld = _mle.KnPkt[_agv.AkKnoten];
								++_agv.AkKnoten;
								if (_agv.AkFeld != -1)
									ok = 1;
							} else {
								for (int16 i = 0; i < 4 && !ok; i++) {
									if (speicher[_agv.AkFeld + _vecTbl[i]] == MOV_LINE_IDX) {
										if (_agv.AkFeld + _vecTbl[i] != _agv.LastFeld) {
											_agv.LastFeld = _agv.AkFeld;
											_agv.AkFeld += _vecTbl[i];
											ok = 1;
										}
									} else if (speicher[_agv.AkFeld + _vecTbl[i]] == MOV_LINE_KNOTEN) {
										if (_agv.AkFeld + _vecTbl[i] != _agv.LastFeld) {
											_agv.LastFeld = _agv.AkFeld + _vecTbl[i];
											_agv.AkFeld = _mle.KnPkt[_agv.AkKnoten];
											++_agv.AkKnoten;
											if (_agv.AkFeld == -1)
												ok = 0;
											else
												ok = 1;
										}
									}
								}
							}
							if (ok) {
								get_feld_xy(_agv.AkFeld, &_agv.X_End, &_agv.Y_End);
								*auto_x = _agv.X_End + 4;
								*auto_y = _agv.Y_End + 4;
							} else {
								_agv.AutoGo = false;
								*auto_x = _gpkt->Dx;
								*auto_y = _gpkt->Dy;
							}
							ret = true;
						}
					}
				}
			}
		}
	}

	return ret;
}

void MovClass::stop_auto_go() {
	_agv.AutoGo = false;
	_agv.Continue = true;
}

void MovClass::continue_auto_go() {
	_agv.AutoGo = _agv.Continue;
}

bool MovClass::auto_go_status() {
	return _agv.AutoGo;
}

void MovClass::calc_xy() {
	int16 xvector = 0;
	int16 yvector = 0;

	if (!_G(ged)->ged_idx(_gpkt->Dx, _gpkt->Dy, _gpkt->Breite, _gpkt->Mem)) {
		int16 ende = 0;
		int16 count = 0;
		int16 min_x = -1;
		int16 min_y = -1;

		while (!ende) {
			int16 min_steps = 30000;
			int16 x = _gpkt->Dx - (count + 1) * 8;
			int16 y = _gpkt->Dy - (count + 1) * 8;

			for (int16 seite = 0; seite < 4; seite++) {
				int16 anz = 2 + (count * 2);

				switch (seite) {
				case S_OBEN:
					xvector = 8;
					yvector = 0;
					break;

				case S_RECHTS:
					xvector = 0;
					yvector = 8;
					break;

				case S_UNTEN:
					xvector = -8;
					yvector = 0;
					break;

				case S_LINKS:
					xvector = 0;
					yvector = -8;
					break;

				default:
					break;
				}

				for (int16 i = 0; i < anz; i++) {
					if (x >= 0 && x < _gpkt->Breite * 8 &&
					        y >= 0 && y < _gpkt->Hoehe * 8) {
						if (_G(ged)->ged_idx(x, y, _gpkt->Breite, _gpkt->Mem)) {
							int16 aksteps = abs(_gpkt->Dx - x);
							aksteps += abs(_gpkt->Dy - y);
							if (aksteps < min_steps) {
								min_steps = aksteps;
								min_x = x;
								min_y = y;
							}
						}
					}
					x += xvector;
					y += yvector;
				}
			}

			++count;
			if (min_steps != 30000) {
				ende = 1;
				int16 tmp_feld = get_feld_nr(min_x, min_y);
				get_feld_xy(tmp_feld, &_gpkt->Dx, &_gpkt->Dy);
				_gpkt->Dx += 4;
				_gpkt->Dy += 4;
			}
		}
	}
}

short MovClass::calc_go(int16 src_feld, int16 *dst_feld) {
	int16 new_vector = true;
	ObjMov om;
	MovInfo mi;

	mi.Vorschub = _gpkt->Vorschub;
	get_feld_xy(src_feld, &om.Xypos[0], &om.Xypos[1]);
	get_feld_xy(*dst_feld, &mi.XyzEnd[0], &mi.XyzEnd[1]);
	mi.XyzStart[0] = om.Xypos[0];
	mi.XyzStart[1] = om.Xypos[1];
	get_mov_vector(mi.XyzStart, mi.XyzEnd, mi.Vorschub, &om);
	bool endFl = false;
	int16 mov_ok = 0;
	int16 steps = 0;
	int16 abbruch = MOV_START;

	while (!endFl && !abbruch) {
		if (om.Count > 0) {
			--om.Count;
			int16 tmpx = om.Xyvo[0];
			int16 tmpy = om.Xyvo[1];
			int16 tmpz = om.Xyvo[2];
			om.Xyna[0][0] += om.Xyna[0][1];
			if (om.Xyna[0][1] < 0) {
				if (om.Xyna[0][0] < -1000) {
					om.Xyna[0][0] += 1000;
					--tmpx;
				}
			} else if (om.Xyna[0][0] > 1000) {
				om.Xyna[0][0] -= 1000;
				++tmpx;
			}
			om.Xyna[1][0] += om.Xyna[1][1];
			if (om.Xyna[1][1] < 0) {
				if (om.Xyna[1][0] < -1000) {
					om.Xyna[1][0] += 1000;
					--tmpy;
				}
			} else if (om.Xyna[1][0] > 1000) {
				om.Xyna[1][0] -= 1000;
				++tmpy;
			}
			om.Xyna[2][0] += om.Xyna[2][1];
			if (om.Xyna[2][1] < 0) {
				if (om.Xyna[2][0] < -1000) {
					om.Xyna[2][0] += 1000;
				}
			} else if (om.Xyna[2][0] > 1000) {
				om.Xyna[2][0] -= 1000;
			}

			tmpz = 0;
			if (!_G(ged)->ged_idx(om.Xypos[0] + tmpx + tmpz,
			                  om.Xypos[1] + tmpy + tmpz,
			                  _gpkt->Breite, _gpkt->Mem)) {

				if (!_G(ged)->ged_idx(om.Xypos[0] + tmpx + tmpz,
				                  om.Xypos[1] + tmpz,
				                  _gpkt->Breite, _gpkt->Mem)) {

					if (!_G(ged)->ged_idx(om.Xypos[0] + tmpz,
					                  om.Xypos[1] + tmpy + tmpz,
					                  _gpkt->Breite, _gpkt->Mem)) {

						abbruch = MOV_UNPASSABLE;
					} else {
						if (!tmpy) {
							abbruch = MOV_UNPASSABLE;
						} else if (abs(om.Xypos[1] - mi.XyzEnd[1]) <= abs(tmpy)) {
							abbruch = MOV_FOUND;
						} else {
							mov_ok = MOV_Y;
						}
					}
				} else {
					if (!tmpx) {
						abbruch = MOV_UNPASSABLE;
					} else  if (abs(om.Xypos[0] - mi.XyzEnd[0]) <= abs(tmpx)) {
							abbruch = MOV_FOUND;
					} else {
						mov_ok = MOV_X;
					}
				}
			} else {
				++steps;
				om.Xypos[0] += tmpx;
				om.Xypos[1] += tmpy;
				om.Xypos[2] += tmpz;
				if (new_vector) {
					new_vector = false;
					mi.XyzStart[0] = om.Xypos[0];
					mi.XyzStart[1] = om.Xypos[1];
					get_mov_vector(mi.XyzStart, mi.XyzEnd, mi.Vorschub, &om);
				}
				if (om.Count == 0) {

					abbruch = MOV_FOUND;
				}
			}

			if (mov_ok) {
				++steps;
				if (mov_ok == MOV_X)
					om.Xypos[0] += tmpx;
				else
					om.Xypos[1] += tmpy;
				om.Xypos[2] += tmpz;
				mi.XyzStart[0] = om.Xypos[0];
				mi.XyzStart[1] = om.Xypos[1];
				get_mov_vector(mi.XyzStart, mi.XyzEnd, mi.Vorschub, &om);
				if (mov_ok == MOV_X) {
					if (om.Xyvo[0] != 0)
						new_vector = true;
				} else {
					if (om.Xyvo[1] != 0)
						new_vector = true;
				}
				if (om.Count == 0) {

					abbruch = MOV_FOUND;
				}
				mov_ok = false;
			}
		} else
			endFl = true;
	}
	if (abbruch == MOV_UNPASSABLE)
		*dst_feld = get_feld_nr(om.Xypos[0], om.Xypos[1]);

	return steps;
}

void MovClass::get_mov_line() {
	int16 ak_steps;
	_gml.GotoFeld = -1;
	_gml.MLineFeld = -1;
	if (_gpkt->Ebenen > 1) {
		byte *speicher = _gpkt->Mem;

		speicher += (_gpkt->Breite * _gpkt->Hoehe) * _gpkt->AkMovEbene;

		int16 start_feld = get_feld_nr(_gpkt->Sx, _gpkt->Sy);
		int16 ende = 0;
		int16 count = 0;
		int16 min_steps = 30000;
		int16 count_vect = DOWN_VECT;
		int16 y_richtung = Y_DOWN;
		while (!ende) {

			int16 tmp_feld = start_feld + count * count_vect;
			if (_G(ged)->ged_idx(tmp_feld, _gpkt->Breite, _gpkt->Mem)) {
				int16 abbruch = 0;
				int16 vector = 0;
				while ((tmp_feld % _gpkt->Breite) < (_gpkt->Breite - 1) && !abbruch)
				{
					if (_G(ged)->ged_idx(tmp_feld + vector, _gpkt->Breite, _gpkt->Mem)) {
						tmp_feld += vector;
						if (speicher[tmp_feld] == MOV_LINE_IDX) {
							ak_steps = abs((tmp_feld % _gpkt->Breite) - (start_feld % _gpkt->Breite));
							ak_steps += abs((tmp_feld / _gpkt->Breite) - (start_feld / _gpkt->Breite));
							if (ak_steps < min_steps) {
								min_steps = ak_steps;
								_gml.GotoFeld = start_feld + count * count_vect;
								_gml.MLineFeld = tmp_feld;
							}
						}
						vector = RIGHT_VECT;
					} else {
						abbruch = MOV_UNPASSABLE;
					}
				}

				tmp_feld = start_feld + count * count_vect;
				vector = LEFT_VECT;
				abbruch = 0;
				while ((tmp_feld % _gpkt->Breite) > 0 && !abbruch) {
					if (_G(ged)->ged_idx(tmp_feld + vector, _gpkt->Breite, _gpkt->Mem)) {
						tmp_feld += vector;
						if (speicher[tmp_feld] == MOV_LINE_IDX) {
							ak_steps = abs((tmp_feld % _gpkt->Breite) - (start_feld % _gpkt->Breite));
							ak_steps += abs((tmp_feld / _gpkt->Breite) - (start_feld / _gpkt->Breite));
							if (ak_steps < min_steps) {
								min_steps = ak_steps;
								_gml.GotoFeld = start_feld + count * count_vect;
								_gml.MLineFeld = tmp_feld;
							}
						}
					} else {
						abbruch = MOV_UNPASSABLE;
					}
				}
				++count;
			} else {
				if (y_richtung == Y_DOWN) {

					y_richtung = Y_UP;
					count = 1;
					count_vect = UP_VECT;
				} else
					ende = 1;
			}
		}
	}
}

void MovClass::calc_mov_line_xit(int16 start_feld) {
	int16 tmp_feld;
	int16 i;
	int16 dest_steps;

	int16 k;
	//int16 found_min;
	int16 TmpKnPkt[MAX_KNOTEN_PKT];
	int16 dir_unerreicht = 0;
	int16 KnPkt_unrreicht[MAX_KNOTEN_PKT];
	int16 feld_unerreicht = 0;
	int16 knoten_felder[MAX_KNOTEN_PKT][6];

	for (i = 0; i < MAX_KNOTEN_PKT; i++)
		TmpKnPkt[i] = -1;
	if (_gpkt->Ebenen > 1) {
		memset((char *)knoten_felder, -1, MAX_KNOTEN_PKT * 6);
		byte *speicher = _gpkt->Mem;
		speicher += (_gpkt->Breite * _gpkt->Hoehe) * _gpkt->AkMovEbene;
		_mle.Steps = 30000;
		int16 min_step_unerreicht = 30000;
		_mle.FNr = -1;
		int16 ziel_feld = get_feld_nr(_gpkt->Dx, _gpkt->Dy);
		int16 calc_feld = start_feld;
		int16 last_feld = start_feld;
		int16 steps = 0;
		int16 found = 0;
		int16 ak_knoten = -1;
		int16 tbl_dir = 1;
		int16 tbl_ptr = 0;
		int16 knoten_flag = 0;

		//found_min = 0;
		int16 dir = 30000;
		while (!found) {
			tmp_feld = ziel_feld;
			calc_go(calc_feld, &tmp_feld);
			if (tmp_feld == ziel_feld) {
				dest_steps = abs((ziel_feld % _gpkt->Breite) - (calc_feld % _gpkt->Breite));
				dest_steps += abs((ziel_feld / _gpkt->Breite) - (calc_feld / _gpkt->Breite));
				if (dest_steps < _mle.Steps) {
					_mle.Steps = dest_steps;
					_mle.FNr = calc_feld;
					if (dir == 30000)
						_mle.Direction = 0;
					else
						_mle.Direction = dir;
					for (i = 0; i < MAX_KNOTEN_PKT; i++)
						_mle.KnPkt[i] = TmpKnPkt[i];
				}
			} else {
				dest_steps = abs((ziel_feld % _gpkt->Breite) - (tmp_feld % _gpkt->Breite));
				dest_steps += abs((ziel_feld / _gpkt->Breite) - (tmp_feld / _gpkt->Breite));
				if (dest_steps < min_step_unerreicht) {
					min_step_unerreicht = dest_steps;
					feld_unerreicht = tmp_feld;
					if (dir == 30000)
						dir_unerreicht = 0;
					else
						dir_unerreicht = dir;
					for (i = 0; i < MAX_KNOTEN_PKT; i++)
						KnPkt_unrreicht[i] = TmpKnPkt[i];
				}
			}
			int16 ok = 0;
			for (i = 0; i < 4 && !ok; i++) {
				if (speicher[calc_feld + _vecTbl[tbl_ptr + i * tbl_dir]] == MOV_LINE_IDX)
				{
					if (calc_feld + _vecTbl[tbl_ptr + i * tbl_dir] != last_feld)
					{
						last_feld = calc_feld;
						calc_feld += _vecTbl[tbl_ptr + i * tbl_dir];
						++steps;
						ok = 1;
						if (dir == 30000) {
							dir = _vecTbl[tbl_ptr + i * tbl_dir];
						}
						if (knoten_flag) {
							TmpKnPkt[ak_knoten] = calc_feld;
							knoten_felder[ak_knoten][tbl_ptr + i * tbl_dir] = calc_feld;
							knoten_flag = false;
						}
					}
				} else if (speicher[calc_feld + _vecTbl[tbl_ptr + i * tbl_dir]] == MOV_LINE_KNOTEN) {
					if (calc_feld + _vecTbl[tbl_ptr + i * tbl_dir] != last_feld) {
						if (dir == 30000) {
							dir = _vecTbl[tbl_ptr + i * tbl_dir];
						}
						++ak_knoten;
						for (k = 0; k < 6; k++)
							knoten_felder[ak_knoten][k] = -1;
						switch (tbl_ptr + i * tbl_dir) {
						case MLE_UP:
							k = MLE_DOWN;
							break;
						case MLE_RIGHT:
							k = MLE_LEFT;
							break;
						case MLE_LEFT:
							k = MLE_RIGHT;
							break;
						case MLE_DOWN:
							k = MLE_UP;
							break;
						}
						knoten_felder[ak_knoten][k] = calc_feld;
						last_feld = calc_feld;
						calc_feld += _vecTbl[tbl_ptr + i * tbl_dir];
						knoten_felder[ak_knoten][4] = calc_feld;
						++steps;
						knoten_felder[ak_knoten][5] = steps;
						knoten_flag = true;
						ok = 1;
					}
				}
			}
			if (!ok) {

				if (ak_knoten != -1)
				{
					ok = 0;
					for (i = 0; i < 4 && !ok; i++) {
						if (knoten_felder[ak_knoten][tbl_ptr + i * tbl_dir] == -1) {
							tmp_feld = knoten_felder[ak_knoten][4];
							if (speicher[tmp_feld + _vecTbl[tbl_ptr + i * tbl_dir]] == MOV_LINE_IDX)
							{
								last_feld = knoten_felder[ak_knoten][4];
								calc_feld = tmp_feld + _vecTbl[tbl_ptr + i * tbl_dir];

								TmpKnPkt[ak_knoten] = calc_feld;
								knoten_felder[ak_knoten][tbl_ptr + i * tbl_dir] = calc_feld;
								steps = knoten_felder[ak_knoten][5];
								ok = 1;
							}
						}
					}
					if (!ok) {
						--ak_knoten;
					}
				} else if (tbl_dir == 1) {
					tbl_dir = -1;
					tbl_ptr = 3;
					dir = 30000;
					//found_min = 0;
					calc_feld = start_feld;
					last_feld = start_feld;
				} else {
					found = 1;
					if (_mle.FNr == -1) {
						_mle.Steps = min_step_unerreicht;
						_mle.FNr = feld_unerreicht;
						_mle.Direction = dir_unerreicht;
						for (i = 0; i < MAX_KNOTEN_PKT; i++)
							_mle.KnPkt[i] = KnPkt_unrreicht[i];

						get_feld_xy(feld_unerreicht, &_gpkt->Dx, &_gpkt->Dy);
						_gpkt->Dx += 4;
						_gpkt->Dy += 4;
					}
				}
			}
		}
	}
}

int16 MovClass::get_feld_nr(int16 x, int16 y) {
	x >>= 3;
	y >>= 3;

	return x + (y * _gpkt->Breite);
}

void MovClass::get_feld_xy(int16 fnr, int16 *x, int16 *y) {
	*x = (fnr % _gpkt->Breite);
	*y = (fnr / _gpkt->Breite);
	*x <<= 3;
	*y <<= 3;
}

void MovClass::get_mov_vector(int16 *startXyz, int16 *endXyz, int16 vorschub, ObjMov *om) {
	if (vorschub > 0) {
		int16 x = startXyz[0];
		int16 y = startXyz[1];
		int16 z = startXyz[2];
		int16 x1 = endXyz[0];
		int16 y1 = endXyz[1];
		int16 z1 = endXyz[2];
		om->Xypos[0] = x;
		om->Xypos[1] = y;
		om->Xypos[2] = z;
		x = x - x1;
		y = y - y1;
		z = z - z1;

		uint8 xvector;
		if (x < 0) {
			xvector = 1;
			--x;
		} else {
			xvector = 0;
			++x;
		}


		uint8 yvector;
		if (y < 0) {
			yvector = 1;
			--y;
		} else {
			yvector = 0;
			++y;
		}

		uint8 zvector;
		if (z < 0) {
			zvector = 1;
			--z;
		} else {
			zvector = 0;
			++z;
		}

		int16 absx = abs(x);
		int16 absy = abs(y);
		int16 absz = abs(z);
		float xstep;
		float ystep;
		if (absx < absy) {
			xstep = (float)absx / ((float)absy / (float)vorschub);
			ystep = vorschub;
			om->Count = (int16)((float) y / (float)ystep);
		} else {
			xstep = vorschub;
			ystep = (float)absy / ((float)absx / (float)vorschub);
			om->Count = (int16)((float) x / (float)xstep);
		}

		float zstep;
		om->Count = abs(om->Count);
		if (!om->Count) {
			zstep = (float)absz / 1.0;
		} else
			zstep = (float)absz / ((float)((abs)(om->Count)));

		if (!xvector)
			xstep = -xstep;
		if (!yvector)
			ystep = -ystep;
		if (!zvector)
			zstep = -zstep;

		om->Xyvo[0] = (int16)xstep;
		om->Xyvo[1] = (int16)ystep;
		om->Xyvo[2] = (int16)zstep;
		int16 tmp = (int16)((float)xstep * 1000.0);
		om->Xyna[0][1] = tmp - (om->Xyvo[0] * 1000);
		om->Xyna[0][0] = 0;
		tmp = (int16)((float)ystep * 1000.0);
		om->Xyna[1][1] = tmp - (om->Xyvo[1] * 1000);
		om->Xyna[1][0] = 0;
		tmp = (int16)((float)zstep * 1000.0);
		om->Xyna[2][1] = tmp - (om->Xyvo[2] * 1000);
		om->Xyna[2][0] = 0;
		if (!om->Count) {
			startXyz[0] = endXyz[0];
			startXyz[1] = endXyz[1];
			om->Xypos[0] = endXyz[0];
			om->Xypos[1] = endXyz[1];
		}
	} else {
		om->Count = 0;

		om->Xypos[0] = startXyz[0];
		om->Xypos[1] = startXyz[1];
		om->Xypos[2] = startXyz[2];
		om->Xyvo[0] = 0;
		om->Xyvo[1] = 0;
		om->Xyvo[2] = 0;
		om->Xyna[0][1] = 0;
		om->Xyna[0][0] = 0;
		om->Xyna[1][1] = 0;
		om->Xyna[1][0] = 0;
		om->Xyna[2][1] = 0;
		om->Xyna[2][0] = 0;
	}
}

} // namespace Chewy
