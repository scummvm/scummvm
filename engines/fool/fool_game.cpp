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

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

ZBasic *g_zbasic;
Toolbox *g_toolbox;

void FoolGame::run() {
	g_toolbox = new Toolbox();
	g_zbasic = new ZBasic(g_toolbox);
	g_zbasic->loadProgram(Common::Path("The Fool's Errand"));
	g_engine->_menu->setVisible(false);
	this->sub_128_004();
	delete g_zbasic;
	delete g_toolbox;
}

void FoolGame::sub_128_004() {
	// 128:0004
	g_zbasic->unk_331(0xdac0, 0);
	g_zbasic->unk_331(0x1b58, 1);
	g_zbasic->unk_331(0x1b58, 2);

	g_toolbox->SetRect(this->arr_rect_1ae06, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);

	// copyright + zbasic notice
	this->var_str_384 = g_zbasic->str(0);
	this->var_str_384 = g_zbasic->str(1);
	this->var_str_384 = g_zbasic->str(2);

	// 128:0086
	// 128:0086: MOVEQ - 0x0,D0
	// 128:0088: SNE - -0x98c(A5)
	g_zbasic->unk_44(0);

	this->var_i16_484 = 0;

	// 128:0096: LEA - [0x3818],A0
	// 128:009a: MOVE.L - A0,-0x8ee(A5)
	// 128:009e: JMP - [0x118]
	// 128:0118: JMP - [0x1e0]
	// 128:01e0: JMP - [0x2ba]
	// 128:02ba: JMP - [0x33e]
	// 128:033e: JMP - [0x3d6]
	// 128:03d6: JMP - [0x402]
	// 128:0402: JMP - [0x424]
	// 128:0424: JMP - [0x442]
	// 128:0442: JMP - [0x45e]
	// 128:045e: JMP - [0x478]
	// 128:0478
	this->var_str_69a = g_zbasic->str(6);
	//this->var_i32_79a = &this->var_str_69a;
	// 128:0496: JMP - [0x4d6]
	// 128:04d6: JMP - [0x50a]
	// 128:050a: JMP - [0x558]
	// 128:0558: JMP - [0x5fa]
	// 128:05fa: JMP - [0x648]
	// 128:0648: JMP - [0x698]
	// 128:0698: JMP - [0x70e]
	// 128:070e: JMP - [0x8b0]
	// 128:08b0: JMP - [0x914]
	// 128:0914: JMP - [0x95e]
	// 128:095e: JMP - [0xbda]
	// 128:0bda: JMP - [0xc66]
	// 128:0c66: JMP - [0xd30]
	// 128:0d30: JMP - [0xdfa]
	// 128:0dfa: JMP - [0x1786]
	//
}

void FoolGame::sub_128_0a2(int16 unk2, int16 unk1) {
	// 128:00a2
	this->var_i16_32 = unk1;
	this->var_i16_30 = unk2;
	if (this->var_i16_30 == 0) {
		// FIXME: use var_i16_32 to choose start offset??
		g_zbasic->get(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_bmp_5dfc);
	}

	// 128:00e6
	if (this->var_i16_30 == 1) {
		// FIXME: use var_i16_32 to choose start offset??
		g_zbasic->put(0, 0, 0, 0, this->arr_bmp_5dfc);

	}
}

void FoolGame::sub_128_11c(Common::U32String &unk2, Common::U32String &unk1) {
	// 128:011c
	g_zbasic->unk_110(this->var_str_272, unk1);
	g_zbasic->unk_110(this->var_str_172, unk2);
	this->var_str_384 = g_zbasic->str(3);

	this->var_i16_586 = 0;

	this->sub_128_0a2(0, 0x6260);

	// 128:015a
	this->var_ev_46.where.y = this->var_i16_58 + 0x3d;
	this->var_ev_46.where.x = this->var_i16_56 + 0x68;
	this->var_i16_16c = 1;
	this->var_i32_16e = 0;

	// 128:017e
	g_toolbox->SFPutFile(this->var_ev_46.where, this->var_str_172, this->var_str_272, this->var_i32_16e, this->var_sfr_5e);
	this->sub_128_6244();

	this->sub_128_0a2(1, 0x6d60);

	// 128:01b0
	if (this->var_sfr_5e.good == 1) {
		this->var_str_486 += this->var_sfr_5e.fName;
		this->var_i16_586 = this->var_sfr_5e.vRefNum;
	}

}

void FoolGame::sub_128_1e4(Common::U32String &unk1) {
	// 128:01e4
	g_zbasic->unk_110(this->var_str_172, unk1);
	this->var_str_588 = g_zbasic->str(4);
	this->var_i16_688 = 0;

	this->sub_128_0a2(0, 0x6260);
	// 128:0218
	this->var_ev_46.where.y = this->var_i16_58 + 0x3d;
	this->var_ev_46.where.x = this->var_i16_56 + 0x53;
	this->var_i16_16c = 1;
	this->var_i32_16e = 0;
	// 128:023c
	// get offset of bytes in string and pretend it's an OSType
	// this->var_i32_168 = *(this->var_str_172 + 1);
	SFTypeList typeList = { { 0 } };
	typeList.types[0] = this->var_str_172.at(0) << 24;
	typeList.types[0] += this->var_str_172.at(1) << 16;
	typeList.types[0] += this->var_str_172.at(2) << 8;
	typeList.types[0] += this->var_str_172.at(3) << 0;

	g_toolbox->SFGetFile(this->var_ev_46.where, g_zbasic->str(5), this->var_i32_16e, this->var_i16_16c, typeList, this->var_i32_16e, this->var_sfr_5e);
	this->sub_128_6244();
	this->sub_128_0a2(1, 0x6d60);

	if (this->var_sfr_5e.good == 1) {
		// 128:02a2
		this->var_str_588 += this->var_sfr_5e.fName;
		this->var_i16_688 = this->var_sfr_5e.vRefNum;
	}
}

void FoolGame::sub_128_2be(int16 unk2, int16 unk1) {
	// FIXME: why is the point inverted in the code??
	this->var_i16_68c = unk1;
	this->var_i16_68a = unk2;
	this->var_i16_68a = (this->var_ev_46.where.x - this->arr_rect_1ec0.top) / (this->arr_rect_1ec0.bottom);

	this->var_i16_68c = (this->var_ev_46.where.y - this->arr_rect_1ec0.left) / (this->arr_rect_1ec0.right);
}

void FoolGame::sub_128_342(int16 unk2, int16 unk1) {
	// 128:0342
	if (unk2 < 1) {
		unk2 = 1;
	}
	if (this->arr_i16_1eb8[0] > unk2) {
		unk2 = this->arr_i16_1eb8[0];
	}
	if (unk1 < 1) {
		unk1 = 1;
	}
	if (this->arr_i16_1eb8[1] > unk1) {
		unk1 = this->arr_i16_1eb8[1];
	}
}

void FoolGame::sub_128_3da(int16 unk1) {
	// 128:03da
	this->var_i32_68e = g_toolbox->TickCount();

	do {
		g_toolbox->Delay(1);
	} while (g_toolbox->TickCount() < (this->var_i32_68e + unk1));
}

void FoolGame::sub_128_406(int16 unk1) {
	// 128:0402
	do {
		g_toolbox->Delay(1);
	} while (g_toolbox->TickCount() < (this->var_i32_692 + unk1));
}

int16 FoolGame::sub_128_428() {
	// 128:0428
	// read a byte
	// FIXME
/*	this->var_i16_30 = *(byte *)this->var_i32_696;
	this->var_i32_696 += 1;
	return this->var_i16_30; */
	return 0;
}

int16 FoolGame::sub_128_446() {
	// 128:0446
	// read a short
	// FIXME
/*	this->var_i16_30 = *(int16 *)this->var_i32_696;
	this->var_i32_696 += 2;
	return this->var_i16_30;*/
	return 0;
}

int16 FoolGame::sub_128_462() {
	// 128:0462
	// read a long
	// FIXME
/*	this->var_i32_68e = *(int32 *)this->var_i32_696;
	this->var_i32_696 += 4;
	return this->var_i32_68e;*/
	return 0;
}

void FoolGame::sub_128_49a() {
	// 128:049a
	// read a pascal string
	// FIXME
/*	this->var_i16_79e = *(byte *)this->var_i32_696;
	g_zbasic->blockMove(this->var_i32_696, this->var_str_69a, this->var_i16_79e+1);
	this->var_i32_696 += this->var_i16_79e + 1;

	// 128:04cc: LEA - VAR(0x69a),A0
	// 128:04d0: JSR - "BUFFER_CONCAT"*/
}

void FoolGame::sub_128_4da(int16 unk1) {
	// 128:04da
	if (unk1 == 0) {
		if (this->var_i16_7a0 == 1) {
			g_toolbox->HideCursor();
		}
	} else {
		if (this->var_i16_7a0 == 0) {
			g_toolbox->ShowCursor();
		}
	}
	this->var_i16_7a0 = unk1;
}

void FoolGame::sub_128_50e(int16 unk3, int16 unk2, int16 unk1) {
	// 128:050e
	this->var_i16_34 = unk1;
	this->var_i16_32 = unk2;
	this->var_i16_30 = unk3;
	if (this->var_i16_378 != 1) {
		g_zbasic->unk_6(this->var_i16_30, this->var_i16_32, 0x96, 0);
		if (this->var_i16_34 == 1) {
			while (g_zbasic->unk_5()) {
				g_toolbox->Delay(1);
			}
		}
	}
}

void FoolGame::sub_128_55c(Common::U32String &unk1) {
	// 128:055c
	g_zbasic->unk_110(this->var_str_172, unk1);
	g_toolbox->PenNormal();
	g_toolbox->SetRect(this->arr_rect_1910c, 0x6c, 0x127, 0x84, 0x137);
	g_toolbox->EraseRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	g_toolbox->FrameRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	g_zbasic->text(0, 0xc, 0, kSrcOr);
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo(0x78 - (this->var_i16_30 / 2), 0x133);
	g_toolbox->DrawString(this->var_str_172);
}

int32 FoolGame::sub_128_5fe() {
	// 128:05fe
	ParamBlockRec pb;
	g_toolbox->PBGetVol(pb);
	this->var_i16_30 = pb.ioVRefNum;
	return this->var_i16_30;
}

void FoolGame::sub_128_64c(int16 unk1) {
	// 128:064c
	ParamBlockRec pb;
	pb.ioVRefNum = unk1;
	g_toolbox->PBSetVol(pb);
}

void FoolGame::sub_128_69c(int16 unk6, PatternMode unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:069c
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[unk6]);
	g_toolbox->PenMode(unk5);
	g_toolbox->SetRect(this->arr_rect_5b7c, unk3, unk4, unk1, unk2);
	g_toolbox->PaintRect(this->arr_rect_5b7c);
	g_toolbox->PenNormal();
}

void FoolGame::sub_128_712(int16 unk3, int16 unk2, int16 unk1) {
	// 128:0712
	g_toolbox->PenNormal();
	if (unk1 == 0) {
		g_toolbox->PenSize(0x3, 0x3);
		g_toolbox->PenPat(this->arr_pat_58f4[0]);
		g_toolbox->FrameRoundRect(this->arr_rect_1f38[unk3], 0xf, 0xf);
		g_toolbox->PenSize(1, 1);
		g_toolbox->PenPat(this->arr_pat_58f4[2]);
		g_toolbox->FrameRoundRect(this->arr_rect_1f38[unk3], 0xf, 0xf);
		// 128:079e
		g_zbasic->picture(this->arr_rect_1f38[unk3].left + 3, this->arr_rect_1f38[unk3].top + 3, this->arr_i32_192c0[this->arr_i16_5cbc[unk2/2]]);
	}
	// 128:0806
	if (unk1 == 1) {
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		g_toolbox->PaintRoundRect(this->arr_rect_1f38[unk3], 0xc, 0xc);
	}
	if (unk1 == 2) {
		g_toolbox->PenMode(kPatOr);
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		g_toolbox->PaintRoundRect(this->arr_rect_1f38[unk3], 0xc, 0xc);
	}
	if (unk1 == 3) {
		g_toolbox->InvertRoundRect(this->arr_rect_1f38[unk3], 0xc, 0xc);
	}
	g_toolbox->PenNormal();
}

void FoolGame::sub_128_8b4(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:08b4
	g_toolbox->SetRect(this->arr_rect_5b7c, unk4, unk5, unk2, unk3);
	g_toolbox->FillRect(this->arr_rect_5b7c, this->arr_pat_58f4[unk1]);
}

void FoolGame::sub_128_918(Common::U32String &unk1) {
	// 128:0918
	g_zbasic->unk_110(unk1, this->var_str_172);
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo(0x100 - (this->var_i16_30 / 2), this->var_i16_7a2);
	g_toolbox->DrawString(this->var_str_172);
}

void FoolGame::sub_128_962(int16 unk11, int16 unk10, int16 unk9, int16 unk8, int16 unk7, int16 unk6, int16 unk5, int16 unk4, int16 unk3, PatternMode unk2, int16 unk1) {
	// 128:0962
	this->arr_rect_5b7c.top = unk11;
	this->arr_rect_5b7c.left = unk10;
	this->arr_rect_5b7c.bottom = unk9;
	this->arr_rect_5b7c.right = unk8;
	this->arr_rect_5b84.top = unk7;
	this->arr_rect_5b84.left = unk6;
	this->arr_rect_5b84.bottom = unk5;
	this->arr_rect_5b84.right = unk4;
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[unk3]);
	g_toolbox->PenMode(unk2);
	// 128:0a42
	// unrolled loop
	this->arr_bcd_5dbc[0] = (float)(this->arr_rect_5b7c.top);
	this->arr_bcd_5dbc[1] = (float)(this->arr_rect_5b7c.left);
	this->arr_bcd_5dbc[2] = (float)(this->arr_rect_5b7c.bottom);
	this->arr_bcd_5dbc[3] = (float)(this->arr_rect_5b7c.right);
	this->arr_bcd_5dbc[4] = (float)((this->arr_rect_5b84.top) - (this->arr_rect_5b7c.top))/(float)(this->var_i16_44);
	this->arr_bcd_5dbc[5] = (float)((this->arr_rect_5b84.left) - (this->arr_rect_5b7c.left))/(float)(this->var_i16_44);
	this->arr_bcd_5dbc[6] = (float)((this->arr_rect_5b84.bottom) - (this->arr_rect_5b7c.bottom))/(float)(this->var_i16_44);
	this->arr_bcd_5dbc[7] = (float)((this->arr_rect_5b84.right) - (this->arr_rect_5b7c.right))/(float)(this->var_i16_44);
	// 128:0af0
	g_toolbox->PaintRect(this->arr_rect_5b7c);
	for (int i = 1; i < this->var_i16_44-1; i++) {
		for (int j = 0; j <= 3; j++) {
			this->arr_bcd_5dbc[j] = (float)this->arr_bcd_5dbc[j] + (float)this->arr_bcd_5dbc[j+4];
		}
		this->arr_rect_5b7c.top = (int)this->arr_bcd_5dbc[0];
		this->arr_rect_5b7c.left = (int)this->arr_bcd_5dbc[1];
		this->arr_rect_5b7c.bottom = (int)this->arr_bcd_5dbc[2];
		this->arr_rect_5b7c.right = (int)this->arr_bcd_5dbc[3];
		// 128:0ba6
		g_toolbox->PaintRect(this->arr_rect_5b92);
	}
	// 128:0bc8
	g_toolbox->PaintRect(this->arr_rect_5b84);
	g_toolbox->PenNormal();
}

void FoolGame::sub_128_bde(int16 unk6, int16 unk5, PatternMode unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:0bde
	this->arr_i16_4758[0] = unk6;
	this->arr_i16_4758[1] = unk5;
	this->arr_i16_4758[2] = unk4;
	this->arr_i16_4758[3] = unk3;
	this->arr_i16_4758[4] = unk2;
	this->arr_i16_4758[5] = unk1;
	this->sub_128_4472();
}

void FoolGame::sub_128_c6a(int16 unk1) {
	// 128:0c6a
	this->var_i16_78a = g_toolbox->GetNextEvent(unk1, this->var_ev_46);
	if ((this->var_ev_46.what == 1) && (this->var_ev_46.where.y < 0x14)) {
		this->sub_128_5b30();
	}
	// 128:0caa
	g_toolbox->GlobalToLocal(this->var_ev_46.where);
	if (this->var_ev_46.what == 3) {
		if ((this->var_ev_46.modifiers & 0x100) == 0) {
			this->sub_128_5f9e();
		} else {
			this->sub_128_5baa();
		}
	}
	// 128:0ce0
	if ((this->var_ev_46.what == 5) && ((this->var_ev_46.modifiers & 0x100) == 0)) {
		this->sub_128_5f9e();
	}
	if (this->var_ev_46.what == 6) {
		this->sub_128_5fb4();
	}
	if (this->var_ev_46.what == 7) {
		this->sub_128_6154();
	}
}

void FoolGame::sub_128_d34(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:0d34
	this->arr_rect_5b7c.top = unk5;
	this->arr_rect_5b7c.left = unk4;
	this->arr_rect_5b7c.bottom = unk3;
	this->arr_rect_5b7c.right = unk2;
	while (this->var_ev_46.modifiers & 0x80) {
		// 128:0d94
		do {
			g_toolbox->InvertRect(this->arr_rect_5b7c);
			this->var_i16_3a = 0;
			do {
				this->sub_128_c6a(0);
				this->var_i16_3a += 1;
			} while ((this->var_i16_38 == this->var_i16_3a) || ((this->var_ev_46.modifiers & 0x80) == 0));
		} while ((this->var_ev_46.modifiers & 0x80) != 0);
	}
}

void FoolGame::sub_128_dfe(int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:0dfe
	this->var_i16_7b0 = unk1;
	this->var_i16_7ae = unk2;
	this->var_i16_7ac = unk3;
	this->var_i16_7aa = unk4;
	g_toolbox->SetPort(this->var_i32_4);
	this->var_i16_7b2 = 0xa;
	g_toolbox->InitCursor();
	this->sub_128_4da(1);
	if (this->var_i16_7b0 != 0) {
		this->sub_128_50e(0x19, 0x64, 0);
	}
	// 128:0e46
	this->sub_128_0a2(0, 0x6060);
	g_zbasic->text(this->var_i16_7aa, 0xc, 0, kSrcBic);
	this->var_i16_7b4 = this->var_i16_7ae*0x46;
	this->var_i16_7b6 = 0;
	this->var_i16_7b8 = 0;
	for (int i = 0; i < this->var_i16_7ac; i++) {
	// 128:0e86
		this->var_i16_7ba = g_toolbox->StringWidth(this->arr_str_1a8d8[this->var_i16_7ba]);
		if (this->var_i16_7ba > this->var_i16_7b4) {
			this->var_i16_7b4 = this->var_i16_7ba;
		}
		this->var_i16_7b6 += 0x11;
	}
	// 128:0ed6
	if (this->var_i16_7ae >= 0) {
		this->var_i16_7bc = 0xa4 + (this->var_i16_7b6 / 2);
		this->var_i16_7b6 += 0x28;
	} else {
		this->var_i16_7b6 += 0xd;
	}
	// 128:0f08
	this->var_i16_7b4 = (this->var_i16_7b4 / 2) + 0xf;
	this->var_i16_7b6 = (this->var_i16_7b6 / 2);
	g_toolbox->PenNormal();
	g_toolbox->SetRect(this->arr_rect_5b7c, 0xf5-this->var_i16_7b4, 0xa0-this->var_i16_7b6, 0x10b+this->var_i16_7b4, 0xb6+this->var_i16_7b6);
	g_toolbox->PenPat(this->arr_pat_58f4[0]);
	g_toolbox->FrameRect(this->arr_rect_5b7c);
	g_toolbox->InsetRect(this->arr_rect_5b7c, 1, 1);
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenPat(this->arr_pat_58f4[2]);
	g_toolbox->FrameRect(this->arr_rect_5b7c);
	g_toolbox->InsetRect(this->arr_rect_5b7c, 5, 5);
	g_toolbox->PenSize(5, 5);
	// 128:0ff8
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	g_toolbox->FrameRect(this->arr_rect_5b7c);
	g_toolbox->InsetRect(this->arr_rect_5b7c, 5, 5);
	g_toolbox->FillRect(this->arr_rect_5b7c, this->arr_pat_58f4[2]);
	g_toolbox->PenNormal();
	this->var_i16_7a2 = 0xbe - this->var_i16_7b6;
	// 128:1056
	for (int i = 0; i < this->var_i16_7ac; i++) {
		this->var_i16_7b8 = 0;
		this->var_str_384 = this->arr_str_1a8d8[this->var_i16_7b8];
		this->sub_128_918(this->var_str_384);
		// 128:1086
		// 128:1086: CLR.W - -0x772(A5)
		this->var_i16_7a2 += 0x11;
	}
	// 128:10a0
	if (this->var_i16_7ae != 0) {
		g_toolbox->PenNormal();
		g_zbasic->text(0, 0xc, 0, kSrcOr);

		// 128:10c0
		// loop to zero out three rectangles??
		this->arr_rect_5b7c = Common::Rect(0, 0, 0, 0);
		this->arr_rect_5b84 = Common::Rect(0, 0, 0, 0);
		this->arr_rect_5b8c = Common::Rect(0, 0, 0, 0);
		// 128:10e2
		if (this->var_i16_7ae == 1) {
			g_toolbox->SetRect(this->arr_rect_5b7c, 0xe2, this->var_i16_7bc, 0x11e, this->var_i16_7bc+0x14);
			// 128:1122
		} else if (this->var_i16_7ae == 2) {
			g_toolbox->SetRect(this->arr_rect_5b7c, 0xbf, this->var_i16_7bc, 0xfb, this->var_i16_7bc+0x14);
			g_toolbox->SetRect(this->arr_rect_5b84, 0x105, this->var_i16_7bc, 0x141, this->var_i16_7bc+0x14);
			// 128:1182
		} else if (this->var_i16_7ae == 3) {
			g_toolbox->SetRect(this->arr_rect_5b7c, 0x9c, this->var_i16_7bc, 0xd8, this->var_i16_7bc+0x14);
			g_toolbox->SetRect(this->arr_rect_5b84, 0xe2, this->var_i16_7bc, 0x11e, this->var_i16_7bc+0x14);
			g_toolbox->SetRect(this->arr_rect_5b8c, 0x128, this->var_i16_7bc, 0x164, this->var_i16_7bc+0x14);
		}
		// 128:1208
		if (this->var_i16_7ae >= 1) {
			g_toolbox->EraseRoundRect(this->arr_rect_5b7c, 0xa, 0xa);
			g_toolbox->FrameRoundRect(this->arr_rect_5b7c, 0xa, 0xa);
			this->var_str_172 = this->arr_str_1a8d8[(this->var_i16_7ac + 1)];
			this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
			// 128:1274
			this->var_i16_30 = this->arr_rect_5b7c.left + ((this->arr_rect_5b7c.right - this->arr_rect_5b7c.left) / 2) - (this->var_i16_30 / 2);
			// 128:12d4
			g_toolbox->MoveTo(this->var_i16_30, this->var_i16_7bc + 0xe);
			g_toolbox->DrawString(this->var_str_172);
			if (this->var_i16_7ae > 1) {
				g_toolbox->InsetRect(this->arr_rect_5b7c, -2, -2);
				g_toolbox->PenPat(this->arr_pat_58f4[0]);
				g_toolbox->FrameRoundRect(this->arr_rect_5b7c, 0xa, 0xa);
				g_toolbox->PenNormal();
				g_toolbox->InsetRect(this->arr_rect_5b7c, 2, 2);
			}
		}
		// 128:134c
		if (this->var_i16_7ae >= 2) {
			g_toolbox->EraseRoundRect(this->arr_rect_5b84, 0xa, 0xa);
			g_toolbox->FrameRoundRect(this->arr_rect_5b84, 0xa, 0xa);
			this->var_str_172 = this->arr_str_1a8d8[(this->var_i16_7ac+2)];
			this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
			this->var_i16_30 = this->arr_rect_5b84.left + ((this->arr_rect_5b84.right - this->arr_rect_5b84.left)/2) - (this->var_i16_30 / 2);
			// 128:1418
			g_toolbox->MoveTo(this->var_i16_30, this->var_i16_7bc + 0xe);
			g_toolbox->DrawString(this->var_str_172);
		}
		// 128:1432
		if (this->var_i16_7ae == 3) {
			g_toolbox->EraseRoundRect(this->arr_rect_5b8c, 0xa, 0xa);
			g_toolbox->FrameRoundRect(this->arr_rect_5b8c, 0xa, 0xa);
			this->var_str_172 = this->arr_str_1a8d8[this->var_i16_7ac+3];
			this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
			this->var_i16_30 = (this->arr_rect_5b8c.left + ((this->arr_rect_5b8c.right - this->arr_rect_5b8c.left)/2)) - (this->var_i16_30/2);

			g_toolbox->MoveTo(this->var_i16_30, this->var_i16_7bc + 0xe);
			g_toolbox->DrawString(this->var_str_172);
		}
		// 128:1518
		this->sub_128_61ec();
		this->var_i16_7be = 0;
		// 128:1522
		do {
			this->var_i16_7a8 = g_toolbox->GetNextEvent(0xa, this->var_ev_46);
			g_toolbox->GlobalToLocal(this->var_ev_46.where);
			if (this->var_ev_46.what == 1) {
				// 128:154a
				this->var_i16_7be = 0;
				Common::Rect target;
				if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_5b7c) != 0) {
					this->var_i16_7be = 1;
					target = this->arr_rect_5b7c;
				}
				if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_5b84) != 0) {
					this->var_i16_7be = 2;
					target = this->arr_rect_5b84;
				}
				if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_5b8c) != 0) {
					this->var_i16_7be = 3;
					target = this->arr_rect_5b8c;
				}

				if (this->var_i16_7be > 0) {
					// 128:15d2
					this->var_i16_30 = (this->var_i16_7be - 1)*4;
					do {
						g_toolbox->InvertRoundRect(target, 0xa, 0xa);

						// 128:1624
						while ((this->var_ev_46.what != 2) && (g_toolbox->PtInRect(this->var_ev_46.where, target) != 0)) {

							this->var_i16_7a8 = g_toolbox->GetNextEvent(0xffff, this->var_ev_46);
							g_toolbox->GlobalToLocal(this->var_ev_46.where);
						}

						g_toolbox->InvertRoundRect(target, 0xa, 0xa);
						// 128:1686
						while ((this->var_ev_46.what != 2) && (g_toolbox->PtInRect(this->var_ev_46.where, target))) {
							this->var_i16_7a8 = g_toolbox->GetNextEvent(0xffff, this->var_ev_46);
							g_toolbox->GlobalToLocal(this->var_ev_46.where);
						}
						// 128:16ea
					} while (this->var_ev_46.what != 2);

					if (g_toolbox->PtInRect(this->var_ev_46.where, target) == 0) {
						this->var_i16_7be = 0;
					}
					this->sub_128_61ec();
				}
			}
			// 128:172c
			if (this->var_ev_46.what == 3) {
				this->var_i16_7c0 = this->var_ev_46.message & 0xff;
				if (this->var_i16_7c0 == 0xd) {
					this->var_i16_7be = 1;
				}
			}
		// 128:175c
		} while (this->var_i16_7be == 0);

		this->sub_128_61ec();
		this->sub_128_0a2(1, 0x6d60);
		g_toolbox->SetPort(this->var_i32_0);
	}
	// 128:1784

}

void FoolGame::sub_128_178a(int16 unk2, int16 unk1) {
	this->var_i16_32 = unk1;
	this->var_i16_30 = unk2;
	if (this->var_i16_30 > 0) {
		if ((this->arr_i16_1d24[this->var_i16_30] & 2) == 0) {
			this->arr_i16_1d24[this->var_i16_30] ^= 0x2;
			this->arr_i16_1d24[this->var_i16_30] |= 0x1;
		}
		// 128:1818
		this->var_i16_34 = 3 + (this->var_i16_30 - 1)/0x10;
		this->var_i16_36 = 1 + (this->var_i16_30 - 1) % 10;
		this->var_str_172 = this->arr_str_195e8[this->var_i16_30] + g_zbasic->str(9);
		g_zbasic->menu(this->var_i16_34, this->var_i16_36, 1, this->var_str_172);
	}
	// 128:188a
	this->sub_128_4da(0);
	g_zbasic->picture(0, 0x14, this->var_pic_7c2);
	g_zbasic->text(0xfa, 0x18, 0x18, kSrcBic);
	for (int j = 0; j <= 1; j++) {
		// 128:18c4
		for (int i = 0; i <= 0x64; i += 4) {
			this->var_i32_692 = g_toolbox->TickCount();
			g_toolbox->SetRect(
				this->arr_rect_4338,
				((int)(i*2.2f) - 0x105),
				i + 0xa5,
				((int)(i*2.2f) + 0x105),
				((int)(i*1.6f) + 0xa6)
			);
			// 128:1982
			g_toolbox->InvertRect(this->arr_rect_4338);
			this->sub_128_406(0);
		}
		// 128:19a4
	}
	// 128:19b2
	g_toolbox->PenNormal();
	g_toolbox->PaintRect(this->arr_rect_4338);
	g_toolbox->PenMode(kPatXor);
	g_toolbox->FrameRect(this->arr_rect_4338);
	g_toolbox->PenMode(kPatCopy);
	this->var_i16_484 = g_toolbox->StringWidth(this->var_str_384);
	// 128:19ec
	g_toolbox->MoveTo(0x105 - (this->var_i16_484/2), 0x12e);
	g_toolbox->DrawString(this->var_str_384);
	for (int i = 0; i <= 0x4d; i++) {
		this->var_i32_692 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_rect_4338);
		this->sub_128_406(0x1);
	}
	for (int i = 1; i <= 0x24; i++) {
		// 128:1a4c
		this->var_i32_692 = g_toolbox->TickCount();
		this->arr_rect_4338.top -= (int)(this->var_i16_68a*0.4f);
		this->arr_rect_4338.left -= (int)(this->var_i16_68a*0.1f);
		this->arr_rect_4338.bottom += (int)(this->var_i16_68a*0.05f);
		this->arr_rect_4338.right += (int)(this->var_i16_68a*0.1f);
		// 128:1b82
		if (this->arr_rect_4338.top < 0x14) {
			this->arr_rect_4338.top = 0x14;
		}
		// 128:1bac
		if (this->var_i16_32 == 0) {
			g_toolbox->InvertRect(this->arr_rect_4338);
		}
		// 128:1bc2
		if (this->var_i16_32 == 1) {
			g_toolbox->FillRect(this->arr_rect_4338, this->arr_pat_58f4[3]);
		}
		// 128:1be6
		if (this->var_i16_32 == 2) {
			g_toolbox->FillRect(this->arr_rect_4338, this->arr_pat_5b2c);
		}
		// 128:1c0a
		this->sub_128_406(1);
	}
	this->sub_128_4da(1);
	g_toolbox->PenNormal();
}

void FoolGame::sub_128_1c30() {
	// 128:1c30
	this->var_i16_7c6 |= this->var_i16_30;
}

void FoolGame::sub_128_1c4a(int16 unk1) {
	// 128:1c4a
	//
}


void FoolGame::sub_128_4472() {
	// 128:4472
	if (this->arr_i16_4758[1] == 0) {
		this->arr_i16_4758[1] = this->var_i16_7e2;
	}
	if (this->arr_i16_4758[2] == 0) {
		this->arr_i16_4758[2] = 0x8;
	}
	if (this->arr_i16_4758[0] == 0) {
		sub_128_962(0x130, 0x76, 0x130, 0x76, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, this->arr_i16_4758[1], (PatternMode)this->arr_i16_4758[2], 0x19);
		return;
	}
	// 128:4520
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[this->arr_i16_4758[1]]);
	g_toolbox->PenMode((PatternMode)this->arr_i16_4758[2]);
	if (this->arr_i16_4758[5] == 0) {
		// 128:456a
		// FIXME
	}
	// 128:459a

}

void FoolGame::sub_128_5b30() {
	// FIXME
}

void FoolGame::sub_128_5baa() {
	// FIXME
}

void FoolGame::sub_128_5f9e() {
	// FIXME
}

void FoolGame::sub_128_5fb4() {
	// FIXME
}

void FoolGame::sub_128_6154() {
	// FIXME
}

void FoolGame::sub_128_61ec() {
	// FIXME
}

void FoolGame::sub_128_6244() {
	// FIXME
}

// end
// 128:62ae
};
