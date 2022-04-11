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

#ifndef CHEWY_ROOM_H
#define CHEWY_ROOM_H

#include "chewy/detail.h"
#include "chewy/mcga_graphics.h"
#include "chewy/object_extra.h"

namespace Chewy {

#define ANI_HIDE 0
#define ANI_SHOW 1
#define SURIMY_OBJ 0

extern const int16 SURIMY_TAF19_PHASES[4][2];

#define MAX_ABLAGE 4

#define ABLAGE_BLOCK_SIZE 64000l
#define GED_BLOCK_SIZE 3000l
#define MAX_ROOM_HANDLE 2
#define GED_LOAD 1

#define R_VOC_DATA 0
#define R_GEP_DATA 1


class JungleRoom {
protected:
	static void topEntry();
	static void leftEntry();
	static void rightEntry();

	static void setup_func();
};

struct RaumBlk {
	byte *LowPalMem;
	const char *InvFile;
	const char *DetFile;
	byte **InvSprAdr;

	RoomMovObject *Rmo;
	RoomStaticInventory *Rsi;

	int16 AkAblage;
	byte **_detImage;
	int16 *DetKorrekt;
	TafInfo *Fti;
	int16 AadLoad;
	int16 AtsLoad;
};

struct RaumTimer {
	int16 _timerStart;
	int16 _timerMaxNr;

	uint8 _objNr[MAX_ROOM_TIMER];
	uint8 _timerNr[MAX_ROOM_TIMER];
};

class Room {
public:
	Room();
	~Room();

	Common::Stream *open_handle(const char *fname, int16 mode);
	void loadRoom(RaumBlk *Rb, int16 room_nr, GameState *player);
	int16 load_tgp(int16 nr, RaumBlk *Rb, int16 tgp_idx, int16 mode, const char *fileName);
	byte *get_ablage(int16 nr);
	byte **get_ablage();
	byte **get_ged_mem();
	void set_timer_start(int16 timer_start);
	void add_timer_new_room();
	void del_timer_old_room();
	int16 set_timer(int16 ani_nr, int16 timer_end);
	void set_timer_status(int16 ani_nr, int16 status);
	void set_zoom(int16 zoom);
	void set_pal(const byte *src_pal, byte *dest_pal);
	void set_ak_pal(RaumBlk *Rb);
	void calc_invent(RaumBlk *Rb, GameState *player);

	RaumTimer _roomTimer;
	RoomInfo *_roomInfo;
	GedChunkHeader _gedInfo[MAX_ABLAGE];
	int16 _gedXNr[MAX_ABLAGE];
	int16 _gedYNr[MAX_ABLAGE];

private:
	void init_ablage();
	void free_ablage();
	int16 get_ablage(int16 pic_nr, uint32 pic_size);
	int16 get_ablage_g1(int16 ablage_bedarf, int16 ak_pos);
	void set_ablage_info(int16 ablagenr, int16 bildnr, uint32 pic_size);

	GedPoolHeader _gedPoolHeader;
	int16 _akAblage;
	int16 _lastAblageSave;

	byte *_ablage[MAX_ABLAGE];
	byte *_ablagePal[MAX_ABLAGE];
	int16 _ablageInfo[MAX_ABLAGE][2];

	byte *_gedMem[MAX_ABLAGE];
	Common::Stream *_roomHandle[MAX_ROOM_HANDLE];
};

void load_chewy_taf(int16 taf_nr);
extern void switchRoom(int16 nr);
extern void calc_person_look();

} // namespace Chewy

#endif
