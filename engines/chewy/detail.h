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

#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/ngshext.h"

namespace Chewy {

#define SAVE_ROOM 0
#define RESTORE_ROOM 1
#define MAX_SOUNDS 3

#define MAX_AUTO_MOV 20

struct RoomInfo {
	uint8 _roomNr;
	uint8 BildNr;
	uint8 AutoMovAnz;
	uint8 TafLoad;
	char TafName[14];
	uint8 _zoomFactor;
	uint8 Dummy;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 4 * 1 + 14 + 1 + 1;
	}
};

struct RoomAutoMov {
	int16 X;
	int16 Y;
	uint8 SprNr;
	uint8 dummy;

	bool load(Common::SeekableReadStream *src);
	static constexpr int SIZE() {
		return 2 + 2 + 1 + 1;
	}
};

#define MAXTAF 10
#define MAXDETAILS 32
#define MAX_M_ITEMS 50
#define MAX_RDI_ROOM 100

struct SprInfo {
	byte *Image = nullptr;
	int16 ZEbene = 0;
	int16 X = 0;
	int16 Y = 0;
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
	int16 StaticDetailAnz = 0;
	int16 AniDetailAnz = 0;
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
	int16 Anz;

	bool load(Common::SeekableReadStream *src);
};

struct DeteditPrj {
	char Id[7];
	char IibFile[MAXPATH];
	char SibFile[MAXPATH];
	char RdiFile[MAXPATH];
	char TafFile[MAXPATH];
	char TgpFile[MAXPATH];
	char Workdir[MAXPATH];
	char InventFile[MAXPATH];
	char EibFile[MAXPATH];
	char AtsRoomFile[MAXPATH];
	char AadFile[MAXPATH];
	char AtsRoomSteuer[MAXPATH];
	char AdsFile[MAXPATH];
	char AtdsFont[MAXPATH];
	char AdhFile[MAXPATH];
	char TvpFile[MAXPATH];
	char DummyFile[MAXPATH];
};

class Detail {
public:
	Detail();
	~Detail();

	void load_rdi(const char *fname, int16 room_nr);

	void load_rdi_taf(const char *fname, int16 load_flag);

	void setStaticSpr(int16 nr, int16 sprNr);
	byte *getStaticImage(int16 detNr);

	void setStaticPos(int16 detNr, int16 x, int16 y, bool hideFl, bool correctionFlag);
	void setSetailPos(int16 detNr, int16 x, int16 y);
	void hideStaticSpr(int16 nr);
	void showStaticSpr(int16 nr);
	void freezeAni();
	void unfreeze_ani();
	void getAniValues(int16 aniNr, int16 *start, int16 *end);
	void setAni(int16 aniNr, int16 start, int16 end);
	byte *getImage(int16 sprNr);
	AniDetailInfo *getAniDetail(int16 aniNr);
	int16 *getCorrectionArray();
	void init_taf(TafInfo *dptr);
	TafInfo *get_taf_info();
	RoomDetailInfo *getRoomDetailInfo();

	void set_static_ani(int16 ani_nr, int16 static_nr);

	void del_static_ani(int16 ani_nr);
	void set_ani_delay(int16 nr, int16 del);
	void startDetail(int16 nr, int16 rep, int16 reverse);
	void stop_detail(int16 nr);
	void plot_ani_details(int16 scrx, int16 scry, int16 start, int16 end,
	                      int16 zoomx, int16 zoomy);
	void plot_static_details(int16 scrx, int16 scry, int16 start, int16 end);
	void init_list(int16 *mv);
	void get_list(int16 *mv);
	int16 maus_vector(int16 x, int16 y);
	int16 get_ani_status(int16 det_nr);

	SprInfo plot_detail_sprite(int16 scrx, int16 scry, int16 det_nr,
	                           int16 spr_nr, int16 mode);

	void set_global_delay(int16 delay);

	TafInfo *init_taf_tbl(const char *fname);

	void del_taf_tbl(TafInfo *Tt);

	void del_taf_tbl(int16 start, int16 anz, TafInfo *Tt);

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
	char _tafName[80];
	byte *_tafLoadBuffer = nullptr;
	int16 _directTafAni = 0;
};

} // namespace Chewy

#endif
