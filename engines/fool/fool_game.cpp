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

#include "fool/fool_game.h"

namespace Fool {

void FoolGame::sub_128_004() {
	// 128:0004
	g_zbasic->unk_331(0xdac0, 0);
	g_zbasic->unk_331(0x1b58, 1);
	g_zbasic->unk_331(0x1b58, 2);

	g_toolbox->SetRect(this->arr_i16_1ae06, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);

	// copyright + zbasic notice
	this->var_str_384 = g_zbasic->str(0);
	this->var_str_384 += g_zbasic->str(1);
	this->var_str_384 += g_zbasic->str(2);

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
	this->var_str_69a += g_zbasic->str(6);
	this->var_i32_79a = &this->var_str_69a;
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
		g_zbasic->get(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_i16_5dfc[this->var_i16_32]);
	}

	// 128:00e6
	if (this->var_i16_30 == 1) {
		g_zbasic->put(0, 0, 0, this->arr_i16_5dfc[this->var_i16_32], 0);

	}
}

void FoolGame::sub_128_11c() {
	// 128:011c
	g_zbasic->unk_110(&this->var_i16_272);
	g_zbasic->unk_110(&this->var_i16_172);
	this->var_str_384 = g_zbasic->str(3);

	this->var_i16_586 = 0;

	this->sub_128_0a2(0, 0x6260);

	// 128:015a
	this->var_i16_50 = this->var_i16_58 + 0x3d;
	this->var_i16_52 = this->var_i16_56 + 0x68;
	this->var_i16_16c = 1;
	this->var_i16_16e = 0;

	// 128:017e
	g_toolbox->Pack3(&this->var_i16_50, &this->var_i16_172, &this->var_i16_272, this->var_i32_16e, this->var_i16_5e, 1);
	this->sub_128_6244();

	this->sub_128_0a2(1, 0x6d60);

	// 128:01b0
	if (this->var_i8_5e[0] == 1) {
		this->var_str_486 += this->var_str_68;
		this->var_i16_586 = this->var_i16_64;
	}

}

void FoolGame::sub_128_01e4() {
	// 128:01e4
	g_zbasic->unk_110(&this->var_i16_172);
	this->var_str_588 = g_zbasic->str(4);
	this->var_i16_688 = 0;

	this->sub_128_0a2(0, 0x6260);
	// 128:0218
	this->var_i16_50 = this->var_i16_58 + 0x3d;
	this->var_i16_52 = this->var_i16_56 + 0x53;
	this->var_i16_16c = 1;
	this->var_i32_16e = 0;
	// 128:023c
	this->var_i32_168 = *(this->var_ptr_172 + 1);
	g_toolbox->Pack3(*this->var_i32_50, g_zbasic->str(5), this->var_i32_16e, this->var_i16_16c, &this->var_i32_168, this->var_i32_16e, &this->var_i16_5e, 2);
	this->sub_128_6244();
	this->sub_128_0a2(1, 0x6d60);

	if (this->var_i16_5e[0] == 1) {
		// 128:02a2
		this->var_str_588 += this->var_str_68;
		this->var_i16_688 = this->var_i16_64;
	}
}

void FoolGame::sub_128_2be(int16 unk2, int16 unk1) {
	this->var_i16_68c = unk1;
	this->var_i16_68a = unk2;
	this->var_i16_68a = (this->var_i16_52 - *this->arr_i16_1ec0) / (*this->arr_i16_1ec4);

	this->var_i16_68c = (this->var_i16_50 - *this->arr_i16_1ec2) / (*this->arr_i16_1ec6);
}

void FoolGame::sub_128_342(int16 unk2, int16 unk1) {
	// 128:0342
	if (unk2 < 1) {
		unk2 = 1;
	}
	if (*this->arr_i16_1eb8 > unk2) {
		unk2 = *this->arr_i16_1eb8;
	}
	if (unk1 < 1) {
		unk1 = 1;
	}
	if (*this->arr_i16_1eba > unk1) {
		unk1 = *this->arr_i16_1eba;
	}
}

void FoolGame::sub_128_3da(int16 unk1) {
	// 128:03da
	this->var_i32_68e = g_toolbox->TickCount();

	do {
		g_toolbox->Delay(1);
	} while (g_toolbox->TickCount() < (this->var_i32_68e + unk1))
}

void FoolGame::sub_128_406(int16 unk1) {
	// 128:0402
	do {
		g_toolbox->Delay(1);
	} while (g_toolbox->TickCount() < (this->var_i32_692 + unk1))
}

int16 FoolGame::sub_128_428() {
	// 128:0428
	this->var_i16_30 = (byte *)this->var_i32_696;
	this->var_i32_696 += 1;
	return this->var_i16_30;
}

int16 FoolGame::sub_128_446() {
	// 128:0446
	this->var_i16_30 = *(int16 *)this->var_i32_696;
	this->var_i32_696 += 2;
	return this->var_i16_30;
}

int16 FoolGame::sub_128_462() {
	// 128:0462
	this->var_i32_68e = *(int32 *)this->var_i32_696;
	this->var_i32_696 += 4;
	return this->var_i32_68e;
}

void FoolGame::sub_128_49a() {
	// 128:049a
	this->var_i16_79e = *(byte *)this->var_i32_696;
	g_zbasic->blockMove(this->var_i32_696, this->var_i32_79a, this->var_i32_79e+1);
	this-var_i32_696 += this->var_i16_79e + 1;

	// 128:04cc: LEA - VAR(0x69a),A0
	// 128:04d0: JSR - "BUFFER_CONCAT"
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
		g_zbasic->unk6(this->var_i16_30, this->var_i16_32, 0x96, 0);
		if (this->var_i16_34 == 1) {
			while (g_zbasic->unk5()) {
				g_toolbox->Delay(1);
			}
		}
	}
}

void FoolGame::sub_128_55c() {
	// 128:055c
	g_zbasic->unk_110(&this->var_i32_172);
	g_toolbox->PenNormal();
	g_toolbox->SetRect(this->arr_i32_1910c, 0x6c, 0x127, 0x84, 0x137);
	g_toolbox->EraseRoundRect(this->arr_i32_1910c, 0x8, 0x7);
	g_toolbox->FrameRoundRect(this->arr_i32_1910c, 0x8, 0x7);
	g_zbasic->text(0, 0xc, 0, 0x1);
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo(0x78 - (this->var_i16_30 / 2), 0x133)
	g_toolbox->DrawString(this->var_str_172);
}

void FoolGame::sub_128_5fe() {
	// 128:05fe
	ParamBlockRec pb;
	g_toolbox->GetVol(pb);
	this->var_i16_30 = pb.ioVRefNum;
	return this->var_i16_30;
}

void FoolGame::sub_128_64c(int16 unk1) {
	// 128:064c
	ParamBlockRec pb;
	pb.ioVRefNum = unk1;
	g_toolbox->PBSetVol(pb);
}

void FoolGame::sub_128_69c(int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:069c
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[unk6]);
	g_toolbox->PenMode(unk5);
	g_toolbox->SetRect(this->arr_i16_5b7c, unk3, unk4, unk1, unk2);
	g_toolbox->PaintRect(this->arr_i16_5b7c);
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
		g_zbasic->picture(this->arr_rect_1f38[unk3].left + 3, this->arr_rect_1f38[unk3].top + 3, -1, 0, this->arr_i32_192c0[this->arr_i16_5cbc[unk2/2]]);
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
	g_toolbox->SetRect(this->arr_i16_5b7c, unk4, unk5, unk2, unk3);
	g_toolbox->FillRect(this->arr_i16_5b7c, this->arr_pat_58f4[unk1]);
}

void FoolGame::sub_128_918() {
	// 128:0918
	g_zbasic->unk_110(this->var_str_172);
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo(0x100 - (this->var_i16_30 / 2), this->var_i16_7a2);
	g_toolbox->DrawString(this->var_str_172);
}

void FoolGame::sub_128_962(int16 unk11, int16 unk10, int16 unk9, int16 unk8, int16 unk7, int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:0962
	this->arr_i16_5b7c.top = unk11;
	this->arr_i16_5b7c.left = unk10;
	this->arr_i16_5b7c.bottom = unk9;
	this->arr_i16_5b7c.right = unk8;
	this->arr_i16_5b84.top = unk7;
	this->arr_i16_5b84.left = unk6;
	this->arr_i16_5b84.bottom = unk5;
	this->arr_i16_5b84.right = unk4;
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[unk3]);
	g_toolbox->PenMode(unk2);
	// 128:0a42
	for (int i = 0; i <= 3; i++) {
		this->arr_bcd_5dbc[i] = (float)(this->arr_i32_5b7c+i);
		this->arr_bcd_5dbc[i+4], (float)((this->arr_i32_5b84+i) - (this->arr_i32_5b7c+i))/(float)(this->var_i16_44);

	}
	// 128:0af0
	g_toolbox->PaintRect(this->arr_i16_5b7c);
	for (int i = 1; i < this->var_i16_44-1; i++) {
		for (int j = 0; j <= 3; j++) {
			this->arr_bcd_5dbc[j] = (float)this->arr_bcd_5dbc[j] + (float)this->arr_bcd_5dbc[j+4];
			this->arr_i16_5b7c[j] = (int)this->arr_bcd_5dbc[j];
		}
		// 128:0ba6
		g_toolbix->PaintRect(this->arr_i16_5b92);
	}
	// 128:0bc8
	g_toolbox->PaintRect(this->arr_i16_5b84);
	g_toolbox->PenNormal();
}

void FoolGame::sub_128_bde(int16 unk6, int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
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
	EventRecord event;
	this->var_i16_78a = g_toolbox->GetNextEvent(unk1, event);
	if ((this->var_i16_46 == 1) && (this->var_i16_50 < 0x14)) {
		this->sub_128_5b30();
	}
	// 128:0caa
	g_toolbox->GlobalToLocal(&this->var_i32_50);
	if (this->var_i16_46 == 3) {
		if (this->var_i16_54 & 0x100 == 0) {
			this->sub_128_5f9e();
		} else {
			this->sub_128_5baa();
		}
	}
	// 128:0ce0
	if ((this->var_i16_46 == 5) && (this->var_i16_54 & 0x100 == 0)) {
		this->sub_128_5f9e();
	}
	if (this->var_i16_46 == 6) {
		this->sub_128_5fb4();
	}
	if (this->var_i16_46 == 7) {
		this->sub_128_6154();
	}
}

void FoolGame::sub_128_d34(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// 128:0d34
	this->arr_i16_5b7c.top = unk5;
	this->arr_i16_5b7c.left = unk4;
	this->arr_i16_5b7c.bottom = unk3;
	this->arr_i16_57bc.right = unk2;
	while (this->var_i16_54 & 0x80) {
		// 128:0d94
		do {
			g_toolbox->InvertRect(this->arr_i16_5b7c);
			this->var_i16_3a = 0;
			do {
				this->sub_128_c6a(0);
				this->var_i16_3a += 1;
			} while ((this->var_i16_38 == this->var_i16_3a) || (this->var_i16_54 & 0x80 == 0))
		} while (this->var_i16_54 & 0x80 != 0)
	}
}



// end
// 128:62ae
};
