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

#ifndef CHEWY_DETAIL_H
#define CHEWY_DETAIL_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "chewy/ngstypes.h"

namespace Chewy {

#define SAVE_ROOM 0
#define RESTORE_ROOM 1
#define MAX_SOUNDS 3

#define MAX_AUTO_MOV 20

struct RoomInfo {
	uint8 _roomNr;
	uint8 _imageNr;
	uint8 _autoMovNr;
	uint8 _tafLoad;
	char _tafName[14];
	uint8 _zoomFactor;
	uint8 _dummy;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 4 * 1 + 14 + 1 + 1;
	}
};

struct RoomAutoMov {
	int16 _x;
	int16 _y;
	uint8 _sprNr;
	uint8 _dummy;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 1 + 1;
	}
};

#define MAXDETAILS 32
#define MAX_M_ITEMS 50

struct SprInfo {
	byte *_image = nullptr;
	int16 _zLevel = 0;
	int16 _x = 0;
	int16 _y = 0;
	int16 X1 = 0;
	int16 Y1 = 0;
};

struct SoundDefBlk {
	bool sound_enable[MAX_SOUNDS];
	int16 sound_index[MAX_SOUNDS];
	int16 sound_start[MAX_SOUNDS];
	int16 kanal[MAX_SOUNDS];
	int16 volume[MAX_SOUNDS];
	int16 repeats[MAX_SOUNDS];
	int16 stereo[MAX_SOUNDS];

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 7 * 2 * MAX_SOUNDS;
	}
};

struct AniDetailInfo {
	int16 x;
	int16 y;
	uint8 start_flag;
	uint8 repeat;
	int16 start_ani;
	int16 end_ani;
	int16 ani_count;
	int16 delay;
	int16 delay_count;
	int16 reverse;
	int16 timer_start;
	int16 z_ebene;
	uint8 load_flag;
	uint8 zoom;
	SoundDefBlk sfx;
	int16 show_1_phase;
	int16 phase_nr;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 1 + 1 + 2 * 8 + 1 + 1 +
			SoundDefBlk::SIZE() + 2 + 2;
	}
};

struct StaticDetailInfo {
	int16 x;
	int16 y;
	int16 SprNr;
	int16 z_ebene;
	bool Hide;
	uint8 Dummy;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 2 + 2 + 1 + 1;
	}
};

struct RoomDetailInfo {
	int16 _staticDetailNr = 0;
	int16 _aniDetailNr = 0;
	TafInfo *dptr = nullptr;
	AniDetailInfo Ainfo[MAXDETAILS];
	StaticDetailInfo Sinfo[MAXDETAILS];
	int16 mvect[MAX_M_ITEMS * 4] = { 0 };
	int16 mtxt[MAX_M_ITEMS] = { 0 };
	RoomInfo Ri;
	RoomAutoMov AutoMov[MAX_AUTO_MOV];
	int16 tvp_index[MAXDETAILS * MAX_SOUNDS] = { 0 };
	byte *sample[MAXDETAILS * MAX_SOUNDS] = { 0 };

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 4 +
			(AniDetailInfo::SIZE() * MAXDETAILS) +
			(StaticDetailInfo::SIZE() * MAXDETAILS) +
			(2 * MAX_M_ITEMS * 4) +
			(2 * MAX_M_ITEMS) +
			RoomInfo::SIZE() +
			(RoomAutoMov::SIZE() * MAX_AUTO_MOV) +
			(2 * MAXDETAILS * MAX_SOUNDS) +
			(4 * MAXDETAILS * MAX_SOUNDS);
	}
};

struct RdiDataHeader {
	char _id[4];
	int16 _nr;

	bool load(Common::SeekableReadStream *src);
};

class Detail {
public:
	Detail();
	~Detail();

	void load_rdi(const char *fname, int16 room_nr);

	void load_rdi_taf(const char *fname, int16 load_flag);

	void setStaticPos(int16 detNr, int16 x, int16 y, bool hideFl, bool correctionFlag);
	void setDetailPos(int16 detNr, int16 x, int16 y);
	void hideStaticSpr(int16 detNr);
	void showStaticSpr(int16 detNr);
	void freezeAni();
	void unfreeze_ani();
	void getAniValues(int16 aniNr, int16 *start, int16 *end);
	AniDetailInfo *getAniDetail(int16 aniNr);
	void init_taf(TafInfo *dptr);
	TafInfo *get_taf_info();
	RoomDetailInfo *getRoomDetailInfo();

	void set_static_ani(int16 ani_nr, int16 static_nr);

	void del_static_ani(int16 ani_nr);
	void startDetail(int16 nr, int16 rep, int16 reverse);
	void stop_detail(int16 nr);
	void plot_ani_details(int16 scrx, int16 scry, int16 start, int16 end,
	                      int16 zoomx, int16 zoomy);
	void plot_static_details(int16 scrx, int16 scry, int16 start, int16 end);
	int16 maus_vector(int16 x, int16 y);
	int16 get_ani_status(int16 det_nr);

	SprInfo plot_detail_sprite(int16 scrx, int16 scry, int16 det_nr,
	                           int16 spr_nr, int16 mode);

	void set_global_delay(int16 delay);

	TafInfo *init_taf_tbl(const char *fname);

	void del_taf_tbl(TafInfo *Tt);

	void del_taf_tbl(int16 start, int16 nr, TafInfo *Tt);

	void load_taf_seq(int16 sprNr, int16 sprCount, TafInfo *Tt);
	void load_taf_tbl(TafInfo *fti);

	void del_dptr();

	void calc_zoom_kor(int16 *kx, int16 *ky, int16 xzoom,
	                   int16 yzoom);

	int16 mouse_on_detail(int16 mouse_x, int16 mouse_y,
	                      int16 scrx, int16 scry);

	void disable_detail_sound(int16 nr);
	void enable_detail_sound(int16 nr);
	void clear_detail_sound(int16 nr);
	void play_detail_sound(int16 nr);
	void disable_room_sound();
	void enable_room_sound();
	void clear_room_sound();
	void set_taf_ani_mem(byte *load_area);

private:
	void load_taf_ani_sprite(int16 nr);

	void removeUnusedSamples();
	RoomDetailInfo _rdi;
	RdiDataHeader _rdiDataHeader;
	SprInfo _sprInfo;

	int16 _globalDelay = 0;
	bool _aniFreezeflag = false;
	bool _fullTaf = false;
	Common::String _tafName;
	byte *_tafLoadBuffer = nullptr;
	int16 _directTafAni = 0;
};

} // namespace Chewy

#endif
