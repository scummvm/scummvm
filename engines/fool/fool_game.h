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

#ifndef FOOL_GAME_H
#define FOOL_GAME_H

#include "common/rect.h"
#include "common/str.h"

#include "fool/toolbox.h"

namespace Fool {

class FoolGame {
public:
	FoolGame() {}
	~FoolGame() {}

	void run();

	void sub_128_004();
	void sub_128_0a2(int16 unk2, int16 unk1);
	void sub_128_11c(Common::U32String &unk2, Common::U32String &unk1);
	void sub_128_1e4(Common::U32String &unk1);
	void sub_128_2be(int16 unk2, int16 unk1);
	void sub_128_342(int16 unk2, int16 unk1);
	void sub_128_3da(int16 unk1);
	void sub_128_406(int16 unk1);
	int16 sub_128_428();
	int16 sub_128_446();
	int16 sub_128_462();
	void sub_128_49a();
	void sub_128_4da(int16 unk1);
	void sub_128_50e(int16 unk3, int16 unk2, int16 unk1);
	void sub_128_55c(Common::U32String &unk1);
	int32 sub_128_5fe();
	void sub_128_64c(int16 unk1);
	void sub_128_69c(int16 unk6, PatternMode unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_712(int16 unk3, int16 unk2, int16 unk1);
	void sub_128_8b4(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_918(Common::U32String &unk1);
	void sub_128_962(int16 unk11, int16 unk10, int16 unk9, int16 unk8, int16 unk7, int16 unk6, int16 unk5, int16 unk4, int16 unk3, PatternMode unk2, int16 unk1);
	void sub_128_bde(int16 unk6, int16 unk5, PatternMode unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_c6a(int16 unk1);
	void sub_128_d34(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_dfe(int16 unk4, int16 unk3, int16 unk2, int16 unk1);

	void sub_128_178a(int16 unk2, int16 unk1);
	void sub_128_1c30();
	void sub_128_1c4a(int16 unk1);

	void sub_128_4472();
	void sub_128_5b30();
	void sub_128_5baa();
	void sub_128_5f9e();
	void sub_128_5fb4();
	void sub_128_6154();
	void sub_128_61ec();
	void sub_128_6244();

private:
	GrafPtr var_i32_0;
	GrafPtr var_i32_4;
	int16 var_i16_30;
	int16 var_i16_32;
	int16 var_i16_34;
	int16 var_i16_36;
	int16 var_i16_38;
	int16 var_i16_3a;
	int16 var_i16_44;
	EventRecord var_ev_46;
	int16 var_i16_56;
	int16 var_i16_58;
	SFReply var_sfr_5e; // -> aa
	int16 var_i16_16c;
	ProcPtr var_i32_16e;
	Common::U32String var_str_172;
	Common::U32String var_str_272;
	int16 var_i16_378;
	Common::U32String var_str_384;
	int16 var_i16_484;
	Common::U32String var_str_486;
	int16 var_i16_586;
	Common::U32String var_str_588;
	int16 var_i16_688;
	int16 var_i16_68a;
	int16 var_i16_68c;
	uint32 var_i32_68e;
	uint32 var_i32_692;
	int32 var_i32_696;
	Common::U32String var_str_69a;
	int16 var_i16_78a;
	int16 var_i16_79e;
	int16 var_i16_7a0;
	int16 var_i16_7a2;
	int16 var_i16_7a8;
	int16 var_i16_7aa;
	int16 var_i16_7ac;
	int16 var_i16_7ae;
	int16 var_i16_7b0;
	int16 var_i16_7b2;
	int16 var_i16_7b4;
	int16 var_i16_7b6;
	int16 var_i16_7b8;
	int16 var_i16_7ba;
	int16 var_i16_7bc;
	int16 var_i16_7be;
	int16 var_i16_7c0;
	PicHandle var_pic_7c2;
	int16 var_i16_7c6;
	int16 var_i16_7e2;

	int16 arr_i16_1d24[202];
	int16 arr_i16_1eb8[4];
	Common::Rect arr_rect_1ec0;
	Common::Rect arr_rect_1f38[3];
	Common::Rect arr_rect_4338;
	int16 arr_i16_4758[6];
	Pattern arr_pat_58f4[4];
	Pattern arr_pat_5b2c;
	Common::Rect arr_rect_5b7c;
	Common::Rect arr_rect_5b84;
	Common::Rect arr_rect_5b8c;
	Common::Rect arr_rect_5b92;
	int16 arr_i16_5cbc[26];
	float arr_bcd_5dbc[8];
	BitMap arr_bmp_5dfc; // 22000, close to a screen page
	BitMap arr_bmp_109dc;
	BitMap arr_bmp_15fcc;
							// Also 109dc and 15fcc
	Common::Rect arr_rect_1910c;
	PicHandle arr_i32_192c0[0x18];
	Common::U32String arr_str_195e8[101]; // 0x20 each

	Common::U32String arr_str_1a8d8[7]; // 0x66 each
	Common::Rect arr_rect_1ae06;
};

}

#endif // FOOL_GAME_H
