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

#ifndef CHEWY_MOVCLASS_H
#define CHEWY_MOVCLASS_H

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
	int16 XyzStart[3];
	int16 XyzEnd[3];

	int16 HotX;
	int16 HotY;
	int16 HotMovX;
	int16 HotMovY;
	int16 Vorschub;
	int16 Delay;
	uint8 Id;

	uint8 Mode;
};

struct ObjMov {
	int16 Xypos[3];
	int16 Xyvo [3];
	int16 Xyna [3][2];
	int16 Count;

	int16 Delay;

	int16 DelayCount;

	int16 Phase;
	int16 PhNr;
	int16 PhAnz;
	int16 Xzoom;
	int16 Yzoom;
	int16 StNr;
};

struct GotoPkt {
	int16 Sx;
	int16 Sy;
	int16 Dx;
	int16 Dy;
	byte *Mem;
	int16 Breite;
	int16 Hoehe;
	int16 Ebenen;
	int16 AkMovEbene;
	int16 Vorschub;
};

struct GotoMovLine {
	int16 GotoFeld;

	int16 MLineFeld;
};

struct MovLineExit {
	int16 Steps;
	int16 Direction;
	int16 FNr;
	int16 KnPkt[MAX_KNOTEN_PKT];
};

struct AutoGoVar {
	bool AutoGo;
	bool Continue;

	int16 PktAnz;

	int16 Start;
	int16 X_End;
	int16 Y_End;
	int16 LastFeld;

	int16 AkFeld;
	int16 AkKnoten;
};

class movclass {
public:
	movclass();
	~movclass();

	void goto_xy(GotoPkt *gpkt);
	int16 calc_auto_go(int16 x, int16 y, int16 *auto_x, int16 *auto_y);
	void stop_auto_go();
	void continue_auto_go();
	bool auto_go_status();
	void get_mov_vector(int16 *xyz, int16 vorschub, ObjMov *om);

private:
	void calc_xy();
	int16 get_feld_nr(int16 x, int16 y);
	void get_feld_xy(int16 fnr, int16 *x, int16 *y);
	int16 calc_go(int16 src_feld, int16 *dst_feld);
	void get_mov_line();
	void calc_mov_line_xit(int16 start_feld);
	int16 feld1knoten;
	int16 plot_delay;
	AutoGoVar agv;
	MovLineExit mle;
	GotoMovLine gml;
	GotoPkt *gpkt;
};

} // namespace Chewy

#endif
