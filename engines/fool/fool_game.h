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

#include "common/ustr.h"
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
	int32 sub_128_462();
	Common::U32String sub_128_49a();
	void sub_128_4da(int16 unk1);
	void sub_128_50e(int16 unk3, int16 unk2, int16 unk1);
	void sub_128_55c(Common::U32String &unk1);
	int16 sub_128_5fe();
	OSErr sub_128_64c(int16 unk1);
	void sub_128_69c(int16 unk6, PatternMode unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_712(int16 unk3, int16 unk2, int16 unk1);
	void sub_128_8b4(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_918(Common::U32String &unk1);
	void sub_128_962(int16 unk11, int16 unk10, int16 unk9, int16 unk8, int16 unk7, int16 unk6, int16 unk5, int16 unk4, int16 unk3, PatternMode unk2, int16 unk1);
	void sub_128_bde(int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_c6a(int16 unk1);
	void sub_128_d34(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1);
	void sub_128_dfe(int16 unk4, int16 unk3, int16 unk2, int16 unk1);

	void sub_128_178a(int16 unk2, int16 unk1);
	void sub_128_1c2c(int16 unk1);
	void sub_128_1c30();
	void sub_128_1c4a(int16 unk1);
	void sub_128_1f1e();
	void sub_128_1f44();
	void sub_128_1f76();
	void sub_128_1fee();
	void sub_128_2078();
	void sub_128_20d0();
	void sub_128_2126();
	void sub_128_21c8();
	void sub_128_2202();
	void sub_128_271a();
	void sub_128_2808();
	void sub_128_2b0a();
	void sub_128_2bc6();
	void sub_128_2e3e();

	void sub_128_3032();
	void sub_128_32c8();
	void sub_128_3536();
	void sub_128_3744();
	void sub_128_3774();
	void sub_128_378a();
	void sub_128_39a0();

	void sub_128_4472();
	void sub_128_5b30();
	void sub_128_5baa();
	void sub_128_5c20();
	void sub_128_5ef0();
	void sub_128_5f16();
	void sub_128_5f9e();
	void sub_128_5fb4();
	void sub_128_5fea();
	void sub_128_6154();
	void sub_128_6186();
	void sub_128_61ec();
	void sub_128_6244();

	void sub_129_004();
	void sub_129_068();
	void sub_129_123a();

	void sub_130_004();

	void sub_138_004();
	void sub_144_004();

private:
	bool _quit = false;

	GrafPtr var_i32_0;
	GrafPtr var_i32_4;
	GrafPtr var_i32_8;
	Handle var_i32_c;
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
	int16 var_i16_5a;
	int16 var_i16_5c;
	SFReply var_sfr_5e; // -> aa
	int16 var_i16_16c;
	ProcPtr var_i32_16e;
	Common::U32String var_str_172;
	Common::U32String var_str_272;
	RGBColor var_i16_372;
	int16 var_i16_378;
	int16 var_i16_37a;
	uint32 var_i32_37c;
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
	uint32 var_ptr_696;
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
	uint32 var_i32_7c8;
	int16 var_i16_7cc;
	int16 var_i16_7ce;
	int16 var_i16_7d0;
	int16 var_i16_7d2;
	int16 var_i16_7d4;
	int16 var_i16_7d6;
	int16 var_i16_7d8;
	int16 var_i16_7da;
	int16 var_i16_7dc;
	int16 var_i16_7de;
	int16 var_i16_7e0;
	int16 var_i16_7e2;
	int16 var_i16_7e4;
	int16 var_i16_7e6;
	Common::U32String var_str_8ec;
	int16 var_i16_9ec;
	int16 var_i16_9f2;
	Common::U32String var_str_9f4;

	// Used as both a pointer and a bitstore
	uint32 var_i32_bf8;
	MenuHandle var_menu_bf8;

	int16 var_i16_d0c;
	int16 var_i16_e12;
	int16 var_i16_e14;
	int16 var_i16_e16;
	int16 var_i16_e18;
	int16 var_i16_e20;
	Common::U32String var_str_e22;
	int16 var_i16_f22;
	GrafPtr var_i32_f24;
	OSType var_i32_f28;
	int16 var_i16_f2c;
	int16 var_i16_f2e;
	Common::U32String var_str_f30;
	int16 var_i16_1030;
	int32 var_i32_1036;
	int16 var_i16_103a;
	int16 var_i16_103c;
	int16 var_i16_103e;
	int16 var_i16_1040;
	int16 var_i16_1056;
	int16 var_i16_1058;
	int16 var_i16_105a;
	int16 var_i16_105c;
	int16 var_i16_105e;
	int16 var_i16_1060;
	int16 var_i16_1062;
	int16 var_i16_1064;
	int16 var_i16_1066;
	int16 var_i16_1068;
	int16 var_i16_106a;
	int16 var_i16_106c;
	int16 var_i16_106e;
	Common::U32String var_str_1070;
	Common::U32String var_str_1170;
	int16 var_i16_1270;
	Common::U32String var_str_1272;
	int16 var_i16_1372;


	// FIXME: bounds on the following are guessed! need to trace index range
	int16 arr_i16_0[202];
	int16 arr_i16_194[202];
	int16 arr_i16_bbe[202];
	int16 arr_i16_15e8[101];
	int16 arr_i16_16b2[0x100];
	int16 arr_i16_18b2[101];
	int16 arr_i16_197c[101];
	int16 arr_i16_1a46[101];
	int16 arr_i16_1b10[6969];
	int16 arr_i16_1b90[101];
	int16 arr_i16_1c5a[16];
	int16 arr_i16_1c7a[16];
	int16 arr_i16_1d24[202];
	int16 arr_i16_1dee[202];
	int16 arr_i16_1eb8[4];
	Common::Rect arr_rect_1ec0;
	Common::Rect arr_rect_1ec8;
	Common::Point arr_pt_1ed0;
	Common::Rect arr_rect_1f38[3];
	int16 arr_i16_2f38[0xc00]; // 96x32?
	int16 arr_i16_3b38[0xc00]; // 96x32?
	Common::Rect arr_rect_4338;
	uint16 arr_i16_4738[16];
	uint16 arr_i16_4758[6];
	int16 arr_i16_4bd8[0x52];
	int16 arr_i16_4c7c[0x52];
	int16 arr_i16_4d20[48];
	Common::SharedPtr<Cursor> arr_curs_4d88[0x2b]; // 0x44
	Pattern arr_pat_58f4[0x51];
	Common::Rect arr_rect_5b7c;
	Common::Rect arr_rect_5b84;
	Common::Rect arr_rect_5b8c;
	Common::Rect arr_rect_5b92;
	int16 arr_i16_5cbc[26];
	float arr_bcd_5dbc[8];
	Handle arr_bytes_5dfc; // used for data buffering
	BitMap arr_bmp_5dfc; // 22000, close to a screen page
	BitMap arr_bmp_b3ec;
	BitMap arr_bmp_109dc;
	BitMap arr_bmp_15fcc;
	GrafPort arr_grafport_18eae;
	GrafPort arr_grafport_18f78;
	GrafPort arr_grafport_19042;
							// Also 109dc and 15fcc
	Common::Rect arr_rect_1910c;
	PicHandle arr_i32_1912c[0x54];
	PicHandle arr_i32_192c0[0x18];
	int32 arr_i32_19454[0x65];
	Common::U32String arr_str_195e8[101]; // 0x20 each

	Common::U32String arr_str_1a288[101]; // 0x10 each

	Common::U32String arr_str_1a8d8[7]; // 0x66 each
	Common::Rect arr_rect_1ae06;
};

}

#endif // FOOL_GAME_H
