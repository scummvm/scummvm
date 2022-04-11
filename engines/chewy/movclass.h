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

#ifndef CHEWY_MOVCLASS_H
#define CHEWY_MOVCLASS_H

#include "common/scummsys.h"

namespace Chewy {

#define MAX_KNOTEN_PKT 15
#define MOV_LINE_IDX 28
#define MOV_LINE_KNOTEN 29
#define MOV_LINE_CROSS 30
#define MLE_UP 0
#define MLE_RIGHT 1
#define MLE_DOWN 2
#define MLE_LEFT 3

struct MovInfo {
	int16 XyzStart[3] = { 0 };
	int16 XyzEnd[3] = { 0 };

	int16 HotX = 0;
	int16 HotY = 0;
	int16 HotMovX = 0;
	int16 HotMovY = 0;
	int16 Vorschub = 0;
	int16 Delay = 0;
	uint8 Id = 0;

	uint8 Mode = 0;
};

struct ObjMov {
	int16 Xypos[3] = { 0 };
	int16 Xyvo[3] = { 0 };
	int16 Xyna[3][2] = {{ 0 }};
	int16 Count = 0;

	int16 Delay = 0;

	int16 _delayCount = 0;

	int16 Phase = 0;
	int16 PhNr = 0;
	int16 PhAnz = 0;
	int16 Xzoom = 0;
	int16 Yzoom = 0;
	int16 StNr = 0;
};

struct GotoPkt {
	int16 Sx = 0;
	int16 Sy = 0;
	int16 Dx = 0;
	int16 Dy = 0;
	byte *Mem = nullptr;
	int16 Breite = 0;
	int16 Hoehe = 0;
	int16 Ebenen = 0;
	int16 AkMovEbene = 0;
	int16 Vorschub = 0;
};

struct GotoMovLine {
	int16 GotoFeld = 0;
	int16 MLineFeld = 0;
};

struct MovLineExit {
	int16 Steps = 0;
	int16 Direction = 0;
	int16 FNr = 0;
	int16 KnPkt[MAX_KNOTEN_PKT] = { 0 };
};

struct AutoGoVar {
	bool AutoGo = false;
	bool Continue = false;

	int16 PktAnz = 0;

	int16 Start = 0;
	int16 X_End = 0;
	int16 Y_End = 0;
	int16 LastFeld = 0;

	int16 AkFeld = 0;
	int16 AkKnoten = 0;
};

class MovClass {
public:
	MovClass();
	~MovClass();

	void goto_xy(GotoPkt *gpkt);
	int16 calc_auto_go(int16 x, int16 y, int16 *auto_x, int16 *auto_y);
	void stop_auto_go();
	void continue_auto_go();
	bool auto_go_status();
	void get_mov_vector(int16 *startXyz, int16 *endXyz, int16 vorschub, ObjMov *om);

private:
	void calc_xy();
	int16 get_feld_nr(int16 x, int16 y);
	void get_feld_xy(int16 fnr, int16 *x, int16 *y);
	int16 calc_go(int16 src_feld, int16 *dst_feld);
	void get_mov_line();
	void calc_mov_line_xit(int16 start_feld);

	int16 _feld1knoten = 0;
	int16 _plotDelay = 0;
	AutoGoVar _agv;
	MovLineExit _mle;
	GotoMovLine _gml;
	GotoPkt *_gpkt = nullptr;
	int16 _vecTbl[4];
};

} // namespace Chewy

#endif
