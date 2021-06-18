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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_MUSIC_H
#define SAGA2_MUSIC_H

namespace Saga2 {

const int16 NoEnemy = -1;
const int16 MaxThemes = 16;
const uint32 FarAway = 250;

/* ===================================================================== *
   Types
 * ===================================================================== */

//-----------------------------------------------------------------------
// Music selection brain

class Deejay {
private:
	int     enemy;
	bool    aggr;
	bool    day;
	bool    ugd;
	bool    susp;

	static int current;
	static int currentID;

public:
	Deejay() {
		enemy = -1;
		aggr = false;
		day = true;
		susp = false;
		ugd = false;
	}
	~Deejay() {}

private:
	void select(void);

public:
	void setEnemy(int16 enemyType = -1) {
		enemy = enemyType;
		select();
	}
	void setAggression(bool aggressive) {
		aggr = aggressive;
		select();
	}
	void setDaytime(bool daytime) {
		day = daytime;
		select();
	}
	void setSuspend(bool suspended) {
		susp = suspended;
		select();
	}
	void setWorld(bool underground) {
		ugd = underground;
		select();
	}
};

//-----------------------------------------------------------------------
// General environmental audio selection brain

typedef uint32 themeFrame;
typedef uint16 ThemeID;

inline ThemeID theme(themeFrame tf) {
	return tf & 0x0000FFFF;
}

inline TilePoint metaTileOffset(themeFrame tf) {
	TilePoint tp = TilePoint(0, 0, 0);
	tp.u = (tf & 0xFF000000) >> 24;
	tp.v = (tf & 0x00FF0000) >> 16;
	return tp;
}

inline TilePoint coordsInMetaTile(TilePoint tp) {
	return TilePoint(tp.u % kPlatShift, tp.v % kPlatShift, 0);
}




struct AudioSurroundings {
	int16 distance;
	int16 counts[MaxThemes];
	TilePoint avgDir[MaxThemes];

	void clear(void) {
		distance = FarAway;
		for (int i = 0; i < MaxThemes; i++) {
			counts[i] = 0;
			avgDir[i] = TilePoint(0, 0, 0);
		}
	}

	void addIn(int16 theme, TilePoint where) {
		avgDir[theme] = (avgDir[theme] * counts[theme]) + where;
		counts[theme]++;
		avgDir[theme] = avgDir[theme] / counts[theme];
	}

	int16 strongest(void) {
		int16 str = 0;
		int16 cnt = 0;
		for (int i = 1; i < MaxThemes; i++) {
			if (counts[i] > cnt) {
				str = i;
				cnt = counts[i];
			}
		}
		return str;
	}

	int16 relativeStrength(int16 theme) {
		int16 rs = counts[theme];
		for (int i = 1; i < MaxThemes; i++) {
			if (i != theme)
				rs -= counts[i];
		}
		return rs;
	}

};

} // end of namespace Saga2

#endif
