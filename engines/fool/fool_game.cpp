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

#include "common/util.h"
#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

ZBasic *g_zbasic;
Toolbox *g_toolbox;

// Based on m68k disassembly of the Fool's Errand v2.0, (c) 1988 Cliff Johnson.

// v1.0 - original release, single-density disks
// v2.0 - fixes full-screen rendering on higher-resolution displays
// v3.0 - newer ZBasic, changed a few graphics assets, removed custom menu font and sounds for compatibility

void FoolGame::run() {
	g_toolbox = new Toolbox();
	g_zbasic = new ZBasic(g_toolbox);
	Common::MacFinderInfo finfo;
	if (g_toolbox->GetFInfo(Common::U32String("The Fool's Errand"), 0, finfo) == kNoErr) {
		g_zbasic->loadProgram(Common::Path("The Fool's Errand", ':'));
	// v1.0 filename ends with "tm"
	} else if (g_toolbox->GetFInfo(Common::U32String("xn--The Fool's Errand-306j"), 0, finfo) == kNoErr) {
		g_zbasic->loadProgram(Common::Path("xn--The Fool's Errand-306j", ':'));
	} else {
		error("FoolGame::run: Fool's Errand program not found");
	}
	this->sub_128_004();
	delete g_zbasic;
	delete g_toolbox;
}

void FoolGame::sub_128_004() {
	// zero out variables
	storyCurrentChapter = 0;
	sunMapRestored = 0;
	menuDisabled = false;
	menuHidesPlayfield = false;
	// zero out pattern buffer; sub_129_123a uses entry
	// 0 for filling the top bar
	for (int i = 0; i <= 0x50; i++) {
		Common::fill(arr_pat_58f4[i].data, arr_pat_58f4[i].data + 8, 0);
	}

	// Define the bitmap surfaces (normally pointers to raw memory)
	this->arr_bmp_5dfc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_b3ec = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_bbbc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_c38c = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_fa3c = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_109dc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));
	this->arr_bmp_138bc = BitMap(new Graphics::ManagedSurface(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8()));

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
	// 128:1786: JMP - [0x1c28]
	// 128:1c28: JMP - [0x1c46]
	// 128:1c46: JMP - [0x1c7a]

	// 128:1c7a
	this->sub_129_068();
	this->var_i32_7c8 = g_zbasic->mem(-1);
	// 128:1c88: JMP - [0x1ee2]
	while (this->stateFlags != kStateQuit) {
		do {
			this->getNextEvent(-1);
			if (_quit)
				return;

			if ((this->var_ev_46.where.y >= 0x8c) && (this->var_ev_46.where.y <= 0x11d)) {
				this->var_i16_7cc = -1;
			}
			// 128:1cbe
			if (this->var_ev_46.where.y > 0x11d) {
				this->var_i16_7cc = 1;
			}
			// 128:1cce
			if ((this->var_ev_46.where.x <= 0x8c) || (this->var_ev_46.where.y < 0xdc)) {
				this->var_i16_7cc = 0;
			}
			// 128:1cf6
			if (this->var_i16_7cc != this->var_i16_7b2) {
				this->var_i16_7b2 = this->var_i16_7cc;
				if (this->var_i16_7b2 == 0) {
					if ((this->var_i16_7ce & 2) == 0) {
						g_toolbox->InitCursor();
					} else {
						g_toolbox->SetCursor(this->arr_curs_4d88[0x10]);
					}
					// 128:1d42
				} else {
					// 128:1d46
					if (this->var_i16_7b2 == -1) {
						// arrow pointing up
						g_toolbox->SetCursor(this->arr_curs_4d88[0x2]);
					}
					// 128:1d5e
					if (this->var_i16_7b2 == 1) {
						// arrow pointing down
						g_toolbox->SetCursor(this->arr_curs_4d88[0x1]);
					}
				}
			}
			// 128:1d76
			if (this->var_ev_46.what == kMouseDown) {
				this->sub_128_1f76();
			}
			if ((this->storyCurrentChapter == 0x10) &&
					(this->puzzleCompletionStatus[0x10] < 0x64) &&
					(this->var_ev_46.where.y > 0x113) &&
					(this->var_ev_46.where.x < 0x8c)) {
				this->setStateBits(kStatePuzzleSelect);
			}
			// 128:1de0
			if (this->sunMapRestored && (this->keyLastPressed == 0x20)) {
				this->setStateBits(0x100);
			}
		// 128:1e06
		} while (this->stateFlags == 0);

		if (this->stateFlags & kStateReturn) {
			this->clearStateBits(kStateReturn);
		}
		if (this->stateFlags & kStateSaveGame) {
			this->saveGame();
		}
		if (this->stateFlags & kStateNewGame) {
			this->newGame();
		}
		if (this->stateFlags & kStateOpenGame) {
			this->openGame();
		}
		if (this->stateFlags & kStateChapterSelect) {
			this->menuChapterSelect();
		}
		if (this->stateFlags & 0x200) {
			this->sub_128_1f44();
		}
		if (this->stateFlags & kStatePuzzleSelect) {
			this->puzzleRun();
		}
		if (this->stateFlags & kStateMetapuzzleSelect) {
			this->metapuzzleRun();
		}
		if (this->stateFlags & 0x400) {
			this->sub_128_2b0a();
		}
		if (this->stateFlags & 0x800) {
			this->sub_144_004();
		}
	// 128:1ee2
	}
	// 128:1eec
	this->sub_128_1f1e();
	g_zbasic->unk_158();
	g_zbasic->unk_4();
	g_toolbox->SetPort(this->var_i32_8);
	this->fillRect(0, 0, 0x14, this->var_i16_5a, 2);
	g_toolbox->_defaultMenu->setOverlayDirty(true);
	g_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::copyScreen(int16 put, BitMap &bmp) {
	// 128:00a2
	// the original code would use the memory at 5dfc + 2*arg2.
	// to make this less bad, our version passes a BitMap pointer
	warning("copyScreen: put %d, bmp %p", put, (void *)&bmp);
	if (put == 0) {
		g_zbasic->get(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bmp);
	}

	// 128:00e6
	if (put == 1) {
		g_zbasic->put(0, 0, bmp, kPutCopy);
	}
}

void FoolGame::sub_128_11c(const Common::U32String &unk2, const Common::U32String &unk1) {
	// 128:011c
	g_zbasic->stringCopy(this->var_str_272, unk1);
	g_zbasic->stringCopy(this->var_str_172, unk2);
	this->var_str_384 = g_zbasic->str(3);

	this->var_i16_586 = 0;

	this->copyScreen(0, this->arr_bmp_138bc);

	// 128:015a
	this->var_ev_46.where.y = this->var_i16_58 + 0x3d;
	this->var_ev_46.where.x = this->var_i16_56 + 0x68;
	this->var_i16_16c = 1;
	this->var_i32_16e = 0;

	// 128:017e
	g_toolbox->SFPutFile(this->var_ev_46.where, this->var_str_172, this->var_str_272, this->var_i32_16e, this->var_sfr_5e);
	this->sub_128_6244();

	this->copyScreen(1, this->arr_bmp_138bc);

	// 128:01b0
	if (this->var_sfr_5e.good == 1) {
		this->var_str_486 = this->var_sfr_5e.fName;
		this->var_i16_586 = this->var_sfr_5e.vRefNum;
	}

}

void FoolGame::sub_128_1e4(const Common::U32String &unk1) {
	// 128:01e4
	g_zbasic->stringCopy(this->var_str_172, unk1);
	this->var_str_588 = g_zbasic->str(4);
	this->var_i16_688 = 0;

	this->copyScreen(0, this->arr_bmp_138bc);
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
	this->copyScreen(1, this->arr_bmp_138bc);

	if (this->var_sfr_5e.good == 1) {
		// 128:02a2
		this->var_str_588 += this->var_sfr_5e.fName;
		this->var_i16_688 = this->var_sfr_5e.vRefNum;
	}
}

void FoolGame::sub_128_2be(int16 unk2, int16 unk1) {
	// convert mouse coordinates to grid coordinates
	this->var_i16_68c = unk1;
	this->var_i16_68a = unk2;
	this->var_i16_68a = (this->var_ev_46.where.x - this->arr_i16_1eb8[4]) / (this->arr_i16_1eb8[6]);

	this->var_i16_68c = (this->var_ev_46.where.y - this->arr_i16_1eb8[5]) / (this->arr_i16_1eb8[7]);
}

void FoolGame::sub_128_342(int16 unk2, int16 unk1) {
	// 128:0342
	this->var_i16_68c = unk1;
	this->var_i16_68a = unk2;
	if (this->var_i16_68a < 1) {
		this->var_i16_68a = 1;
	}
	// 128:035c
	if (this->var_i16_68a > this->arr_i16_1eb8[0]) {
		this->var_i16_68a = this->arr_i16_1eb8[0];
	}
	if (this->var_i16_68c < 1) {
		this->var_i16_68c = 1;
	}
	if (this->var_i16_68c > this->arr_i16_1eb8[1]) {
		this->var_i16_68c = this->arr_i16_1eb8[1];
	}
}

void FoolGame::sub_128_3da(int16 unk1) {
	// 128:03da
	this->var_i32_68e = g_toolbox->TickCount();

	do {
		g_toolbox->Delay(0);
	} while (g_toolbox->TickCount() < (this->var_i32_68e + unk1));
}

void FoolGame::sub_128_406(int16 unk1) {
	// 128:0402
	do {
		g_toolbox->Delay(0);
	} while (g_toolbox->TickCount() < (this->var_i32_692 + unk1));
}

int16 FoolGame::puzzlesReadByte() {
	// 128:0428
	// read a byte
	this->var_i16_30 = *(byte *)(&var_bytes_696->data()[this->var_ptr_696]);
	debugC(8, kDebugLoading, "Read[%04x]: %02x", this->var_ptr_696, this->var_i16_30);
	this->var_ptr_696 += 1;
	return this->var_i16_30;
	return 0;
}

int16 FoolGame::puzzlesReadShort() {
	// 128:0446
	// read a short
	this->var_i16_30 = READ_BE_INT16(&var_bytes_696->data()[this->var_ptr_696]);
	debugC(8, kDebugLoading, "Read[%04x]: %04x", this->var_ptr_696, this->var_i16_30);
	this->var_ptr_696 += 2;
	return this->var_i16_30;
}

int32 FoolGame::puzzlesReadLong() {
	// 128:0462
	// read a long
	this->var_i32_68e = READ_BE_INT32(&var_bytes_696->data()[this->var_ptr_696]);
	debugC(8, kDebugLoading, "Read[%04x]: %08x", this->var_ptr_696, this->var_i32_68e);
	this->var_ptr_696 += 4;
	return this->var_i32_68e;
}

Common::String FoolGame::puzzlesReadString() {
	// 128:049a
	// read a pascal string
	this->var_i16_79e = *(byte *)(&var_bytes_696->data()[this->var_ptr_696]);
	this->var_str_69a = Common::String((const char *)&var_bytes_696->data()[this->var_ptr_696+1], this->var_i16_79e);
	debugC(8, kDebugLoading, "Read[%04x]: %s", this->var_ptr_696, this->var_str_69a.c_str());
	this->var_ptr_696 += this->var_i16_79e + 1;
	return this->var_str_69a;
}

void FoolGame::sub_128_4da(int16 unk1) {
	// toggle the mouse cursor off or on
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

void FoolGame::sub_128_50e(int16 freq, int16 duration, int16 wait) {
	// 128:050e
	// audio tone
	this->var_i16_34 = wait;
	this->var_i16_32 = duration;
	this->var_i16_30 = freq;
	if (this->soundEnabled == 1) {
		// the volume was originally 150, this is crazy loud.
		g_zbasic->sound(this->var_i16_30, this->var_i16_32, 20, 0);
		if (this->var_i16_34 == 1) {
			while (g_zbasic->soundBusy()) {
				g_toolbox->Delay(0);
			}
		}
	}
}

void FoolGame::sub_128_55c(const Common::U32String &unk1) {
	// 128:055c
	g_zbasic->stringCopy(this->var_str_172, unk1);
	g_toolbox->PenNormal();
	g_toolbox->SetRect(this->arr_rect_1910c, 0x6c, 0x127, 0x84, 0x137);
	g_toolbox->EraseRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	g_toolbox->FrameRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo(0x78 - (this->var_i16_30 / 2), 0x133);
	g_toolbox->DrawString(this->var_str_172);
}

int16 FoolGame::sub_128_5fe() {
	// 128:05fe
	ParamBlockRec pb;
	g_toolbox->PBGetVol(pb);
	this->var_i16_30 = pb.ioVRefNum;
	return this->var_i16_30;
}

OSErr FoolGame::sub_128_64c(int16 unk1) {
	// 128:064c
	ParamBlockRec pb;
	pb.ioVRefNum = unk1;
	return g_toolbox->PBSetVol(pb);
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

void FoolGame::drawTarotCard(int16 rectID, int16 deckPos, int16 type) {
	// 128:0712
	g_toolbox->PenNormal();
	if (type == 0) {
		g_toolbox->PenSize(0x3, 0x3);
		g_toolbox->PenPat(this->arr_pat_58f4[0]);
		g_toolbox->FrameRoundRect(this->arr_rect_1f38[rectID], 0xf, 0xf);
		g_toolbox->PenSize(1, 1);
		g_toolbox->PenPat(this->arr_pat_58f4[2]);
		g_toolbox->FrameRoundRect(this->arr_rect_1f38[rectID], 0xf, 0xf);
		// 128:079e
		g_zbasic->picture(this->arr_rect_1f38[rectID].left + 3, this->arr_rect_1f38[rectID].top + 3, this->arr_i32_192c0[this->arr_i16_5cbc[deckPos]]);
	}
	// 128:0806
	if (type == 1) {
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		g_toolbox->PaintRoundRect(this->arr_rect_1f38[rectID], 0xc, 0xc);
	}
	if (type == 2) {
		g_toolbox->PenMode(kPatOr);
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		g_toolbox->PaintRoundRect(this->arr_rect_1f38[rectID], 0xc, 0xc);
	}
	if (type == 3) {
		g_toolbox->InvertRoundRect(this->arr_rect_1f38[rectID], 0xc, 0xc);
	}
	g_toolbox->PenNormal();
}

void FoolGame::fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID) {
	// 128:08b4
	g_toolbox->SetRect(this->arr_rect_5b7c, left, top, right, bottom);
	g_toolbox->FillRect(this->arr_rect_5b7c, this->arr_pat_58f4[patternID]);
}

void FoolGame::sub_128_918(const Common::U32String &unk1) {
	// 128:0918
	g_zbasic->stringCopy(this->var_str_172, unk1);
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo(0x100 - (this->var_i16_30 / 2), this->var_i16_7a2);
	g_toolbox->DrawString(this->var_str_172);
}

void FoolGame::sub_128_962(int16 unk11, int16 unk10, int16 unk9, int16 unk8, int16 unk7, int16 unk6, int16 unk5, int16 unk4, int16 unk3, PatternMode unk2, int16 unk1) {
	// Zoom-fill the screen with checkerboard pattern
	// after e.g. clicking a story button.
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
	this->arr_bcd_5dbc[4] = (float)((this->arr_rect_5b84.top) - (this->arr_rect_5b7c.top))/(float)(unk1);
	this->arr_bcd_5dbc[5] = (float)((this->arr_rect_5b84.left) - (this->arr_rect_5b7c.left))/(float)(unk1);
	this->arr_bcd_5dbc[6] = (float)((this->arr_rect_5b84.bottom) - (this->arr_rect_5b7c.bottom))/(float)(unk1);
	this->arr_bcd_5dbc[7] = (float)((this->arr_rect_5b84.right) - (this->arr_rect_5b7c.right))/(float)(unk1);
	// 128:0af0
	g_toolbox->PaintRect(this->arr_rect_5b7c);
	for (int i = 1; i < unk1-1; i++) {
		for (int j = 0; j <= 3; j++) {
			this->arr_bcd_5dbc[j] = (float)this->arr_bcd_5dbc[j] + (float)this->arr_bcd_5dbc[j+4];
		}
		this->arr_rect_5b92.top = (int)this->arr_bcd_5dbc[0];
		this->arr_rect_5b92.left = (int)this->arr_bcd_5dbc[1];
		this->arr_rect_5b92.bottom = (int)this->arr_bcd_5dbc[2];
		this->arr_rect_5b92.right = (int)this->arr_bcd_5dbc[3];
		// 128:0ba6
		g_toolbox->PaintRect(this->arr_rect_5b92);
		// new: force a redraw delay
		g_toolbox->Delay(0);
	}
	// 128:0bc8
	g_toolbox->PaintRect(this->arr_rect_5b84);
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

void FoolGame::getNextEvent(uint32 unk1) {
	// 128:0c6a

	// This function is usually called at the start of an event processing loop,
	// so yield to the event pump/display update when necessary.
	if (this->var_ev_46.what == kNullEvent)
		g_toolbox->Delay(0);

	this->var_i16_78a = g_toolbox->GetNextEvent(unk1, this->var_ev_46);
	if ((this->var_ev_46.what == kMouseDown) && (this->var_ev_46.where.y < 0x14)) {
		this->sub_128_5b30();
	}
	// 128:0caa
	g_toolbox->GlobalToLocal(this->var_ev_46.where);
	if (this->var_ev_46.what == kKeyDown) {
		// the original just checked the command key,
		// non-mac PCs expect the control key to work
		if ((this->var_ev_46.modifiers & (kModCommandKeyDown | kModLControlKeyDown)) == 0) {
			this->sub_128_5f9e();
		} else {
			this->sub_128_5baa();
		}
	}
	// 128:0ce0
	if ((this->var_ev_46.what == kAutoKey) && ((this->var_ev_46.modifiers & (kModCommandKeyDown | kModLControlKeyDown)) == 0)) {
		this->sub_128_5f9e();
	}
	if (this->var_ev_46.what == kUpdateEvt) {
		this->sub_128_5fb4();
	}
	if (this->var_ev_46.what == kDiskEvt) {
		this->sub_128_6154();
	}
	if ((this->var_ev_46.what == kScummVMQuitEvt) || (this->var_ev_46.what == kScummVMReturnToLauncherEvt)) {
		this->menuQuit();
	}
}

void FoolGame::sub_128_d34(int16 unk5, int16 unk4, int16 unk3, int16 unk2, int16 unk1) {
	// flash a rectangle on the screen until we get a click
	// 128:0d34
	this->arr_rect_5b7c.top = unk5;
	this->arr_rect_5b7c.left = unk4;
	this->arr_rect_5b7c.bottom = unk3;
	this->arr_rect_5b7c.right = unk2;
	while (this->var_ev_46.modifiers & kModMouseButtonUp) {
		// 128:0d94
		do {
			// FIXME: Flashing far too intense
			g_toolbox->InvertRect(this->arr_rect_5b7c);
			this->var_i16_3a = 0;
			do {
				// originally this used getNextEvent, but we avoid that here
				// so that menu events don't get intercepted.
				// originally this mask was 0, but we change it here to
				// intercept all events, and fall back to NullEvent +
				// wait for vsync if no events were received.
				this->var_i16_78a = g_toolbox->GetNextEvent(-1, this->var_ev_46);
				if (this->var_ev_46.what == kNullEvent) {
					g_toolbox->Delay(0);
					this->var_i16_3a += 1;
				}
			} while (!((this->var_i16_3a >= (unk1*60/1000)) || ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0)));
		} while ((this->var_ev_46.modifiers & kModMouseButtonUp) != 0);
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
	this->copyScreen(0, this->arr_bmp_138bc);
	g_zbasic->text(this->var_i16_7aa, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_i16_7b4 = this->var_i16_7ae*0x46;
	this->var_i16_7b6 = 0;
	for (int i = 0; i <= this->var_i16_7ac; i++) {
	// 128:0e86
		this->var_i16_7ba = g_toolbox->StringWidth(this->arr_str_1a8d8[i]);
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
	for (int i = 0; i <= this->var_i16_7ac; i++) {
		this->var_str_384 = this->arr_str_1a8d8[i];
		this->sub_128_918(this->var_str_384);
		// 128:1086
		// 128:1086: CLR.W - -0x772(A5)
		this->var_i16_7a2 += 0x11;
	}
	// 128:10a0
	if (this->var_i16_7ae != 0) {
		g_toolbox->PenNormal();
		g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);

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
			if (this->var_ev_46.what == kMouseDown) {
				// 128:154a
				this->var_i16_7be = 0;
				Common::Rect target;
				if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_5b7c)) {
					this->var_i16_7be = 1;
					target = this->arr_rect_5b7c;
				}
				if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_5b84)) {
					this->var_i16_7be = 2;
					target = this->arr_rect_5b84;
				}
				if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_5b8c)) {
					this->var_i16_7be = 3;
					target = this->arr_rect_5b8c;
				}

				if (this->var_i16_7be > 0) {
					// 128:15d2
					this->var_i16_30 = (this->var_i16_7be - 1)*4;
					do {
						g_toolbox->InvertRoundRect(target, 0xa, 0xa);

						// 128:1624
						while ((this->var_ev_46.what != kMouseUp) && (g_toolbox->PtInRect(this->var_ev_46.where, target))) {

							this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
							g_toolbox->GlobalToLocal(this->var_ev_46.where);
							if (this->var_ev_46.what == kNullEvent) {
								g_toolbox->Delay(0);
							}
						}

						g_toolbox->InvertRoundRect(target, 0xa, 0xa);
						// 128:1686
						while ((this->var_ev_46.what != kMouseUp) && (!g_toolbox->PtInRect(this->var_ev_46.where, target))) {
							this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
							g_toolbox->GlobalToLocal(this->var_ev_46.where);
							if (this->var_ev_46.what == kNullEvent) {
								g_toolbox->Delay(0);
							}
						}
						// 128:16ea
					} while (this->var_ev_46.what != kMouseUp);

					if (!g_toolbox->PtInRect(this->var_ev_46.where, target)) {
						this->var_i16_7be = 0;
					}
					this->sub_128_61ec();
				}
			}
			// 128:172c
			if (this->var_ev_46.what == kKeyDown) {
				this->keyLastPressed = this->var_ev_46.message & 0xff;
				if (this->keyLastPressed == 0xd) {
					this->var_i16_7be = 1;
				}
			}
			if (this->var_ev_46.what == kNullEvent)
				g_toolbox->Delay(0);
		// 128:175c
		} while (this->var_i16_7be == 0);

		this->sub_128_61ec();
		this->copyScreen(1, this->arr_bmp_138bc);
		g_toolbox->SetPort(this->var_i32_0);
	}
	// 128:1784

}

void FoolGame::sub_128_178a(int16 unk2, int16 unk1) {
	// behold the Nth key of thoth animation
	this->var_i16_32 = unk1;
	this->var_i16_30 = unk2;
	if (this->var_i16_30 > 0) {
		if ((this->puzzleFlags[this->var_i16_30] & kFlagMenuDiamond)) {
			this->puzzleFlags[this->var_i16_30] ^= kFlagMenuDiamond;
			this->puzzleFlags[this->var_i16_30] |= kFlagMenuEnabled;
		}
		// 128:1818
		this->var_i16_34 = 3 + ((this->var_i16_30 - 1) / 0x10);
		this->var_i16_36 = 1 + ((this->var_i16_30 - 1) % 0x10);
		this->var_str_172 = this->arr_str_195e8[this->var_i16_30] + g_zbasic->str(9);
		// add the wadjet eye next to the chapter name
		g_zbasic->menu(this->var_i16_34, this->var_i16_36, 1, this->var_str_172);
	}
	// 128:188a
	this->sub_128_4da(0);
	g_zbasic->picture(0, 0x14, this->var_pic_7c2);
	g_zbasic->text(0xfa, 0x18, Graphics::kMacFontShadow | Graphics::kMacFontOutline, kSrcBic);
	for (int j = 0; j <= 1; j++) {
		// 128:18c4
		for (int i = 0; i <= 0x64; i += 4) {
			this->var_i32_692 = g_toolbox->TickCount();
			g_toolbox->SetRect(
				this->arr_rect_4338,
				(0x105 - (int)(i*2.2f)),
				i + 0xa5,
				(0x105 + (int)(i*2.2f)),
				(0xa6 + (int)(i*1.6f))
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
	// loop count was 0x4d
	for (int i = 0; i <= 0x24; i++) {
		this->var_i32_692 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_rect_4338);
		// this was 1, however the flashing was far too intense
		this->sub_128_406(8);
	}
	for (int i = 1; i <= 0x24; i++) {
		// 128:1a4c
		this->var_i32_692 = g_toolbox->TickCount();
		this->arr_rect_4338.top -= (int)(i*0.4f);
		this->arr_rect_4338.left -= (int)(i*0.1f);
		this->arr_rect_4338.bottom += (int)(i*0.05f);
		this->arr_rect_4338.right += (int)(i*0.1f);
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
			g_toolbox->FillRect(this->arr_rect_4338, this->arr_pat_58f4[0x47]);
		}
		// 128:1c0a
		// was 1
		this->sub_128_406(2);
	}
	this->sub_128_4da(1);
	g_toolbox->PenNormal();
}

void FoolGame::setStateBits(uint16 bits) {
	// 128:1c2c
	this->stateFlags |= bits;
}

void FoolGame::clearStateBits(uint16 bits) {
	// 128:1c4a
	if (this->stateFlags & bits) {
		this->stateFlags ^= bits;
	}
	// 128:1c78
}

void FoolGame::sub_128_1ef8() {
	// 128:1ef8
	g_toolbox->SetPort(this->var_i32_8);
	this->fillRect(0, 0, 0x14, this->var_i16_5a, 2);
	g_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_128_1f1e() {
	// 128:1f1e
	g_toolbox->InitCursor();
	this->sub_128_4da(0);
	this->sub_128_bde(1, 1, kSrcCopy, 1, 1, 1);
}

void FoolGame::sub_128_1f44() {
	// 128:1f44
	this->sub_128_61ec();
	this->storyCurrentPage = 0;
	this->clearStateBits(0x200);
	this->sub_128_5fea();
	this->copyScreen(1, this->arr_bmp_5dfc);
	this->storyRenderPage();
	g_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_128_1f76() {
	// 128:1f76
	// hold down mouse on scroll
	if (this->var_i16_7cc != 0) {
		do {
			this->var_i32_692 = g_toolbox->TickCount();
			this->sub_128_20d0();
			// original was 5 ticks
			this->sub_128_406(10);
			// new: empty the event queue, so feedback is instant
			g_toolbox->FlushEvents(-1, 0);
			this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
		} while ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0);
	} else {
		// 128:1fee
		if ((this->storyCurrentChapter == 0x10) && (this->puzzleCompletionStatus[0x10] < 0x64) && (this->var_ev_46.where.y > 0x113) && (this->var_ev_46.where.x < 0x8c)) {
			return;
		}
		// 128:2042
		if (this->var_i16_7d6 != 0) {
			if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1910c)) {
				this->sub_128_2078();
			}
		}
		// 128:2076
	}
}


void FoolGame::sub_128_2078() {
	// clicking the story button
	// 128:2078
	g_toolbox->InvertRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	this->sub_128_6186();
	g_toolbox->InvertRoundRect(this->arr_rect_1910c, 0x8, 0x7);
	if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1910c)) {
		this->sub_128_21c8();
	}
}

void FoolGame::sub_128_20d0() {
	// 128:20d0
	// change page on scroll
	do {
		this->storyNextPage += this->var_i16_7cc;
		if (this->storyNextPage < 1) {
			this->storyNextPage = 1;
			return;
		}
		if (this->storyNextPage > this->storyPageCount) {
			this->storyNextPage = this->storyPageCount;
			return;
		}
	} while (this->pageVisible[this->storyNextPage] == 0);
	this->storyRenderPage();
}

void FoolGame::menuChapterSelect() {
	this->clearStateBits(kStateChapterSelect);
	// 128:2132
	for (int i = this->selectedMenuChapter; i <= this->storyPageCount; i++) {
		if (this->selectedMenuChapter == this->pageToChapter[i]) {
			this->storyNextPage = i;
			i = this->storyPageCount;
		}
		// 128:2160
	}
	// 128:2170
	this->storyCurrentChapter = this->pageToChapter[this->storyNextPage];
	if (this->var_i16_7ce & 4) {
		this->var_i16_7ce ^= 4;
		if (this->puzzleType[this->storyCurrentChapter] > 0) {
			// 128:21bc
			this->sub_128_21c8();
			return;
		}
	}
	// 128:21c2
	this->storyRenderPage();
}

void FoolGame::sub_128_21c8() {
	// 128:21c8
	if ((this->storyCurrentChapter == 1) && (this->sunMapRestored > 0)) {
		this->setStateBits(0x100);
	} else {
		this->setStateBits(kStatePuzzleSelect);
	}
}

void FoolGame::storyRenderPage() {
	// 128:2202
	if (this->storyNextPage != this->storyCurrentPage) {
		if ((this->arr_i16_1dee[this->storyNextPage] < 0xe) && (this->var_i16_7de > 0)) {
			this->sub_128_69c(1, kPatBic, this->var_i16_7de, 0x37, 0x11d, 0x1db);
			// 128:226e
			// grab the last N-1 lines of text from the screen
			g_zbasic->get(0x3c, 0x122 - (0xf - this->arr_i16_1dee[this->storyNextPage]) * 0xf, 0x1d6, 0x11d, this->arr_bmp_b3ec);
			this->var_i16_7e0 = 1;
		} else {
			// 128:22d4
			this->var_i16_7e0 = 0;
		}
	}
	// 128:22da
	// erase lower controls on scroll
	this->fillRect(0x127, 0x69, 0x138, 0x190, 0);
	this->storyCurrentChapter = this->pageToChapter[this->storyNextPage];
	// if we've solved a puzzle, fill the scroll with crazy patterns
	if ((this->var_i16_7ce & 0x8) != 0) {
		this->var_i16_7ce ^= 0x8;
		this->sub_128_bde(1, 0, 0, 0, 1, 0);
		this->sub_128_bde(1, 0, 0, 0, 0, 0);
	}
	// 128:235e
	if (this->puzzleType[this->storyCurrentChapter] > 0) {
		this->var_str_384 = g_zbasic->str(10); // ?
		if (this->puzzleCompletionStatus[this->storyCurrentChapter] == 0x63) {
			this->var_str_384 = g_zbasic->str(11); // ~
		}
		// 128:23b8
		if (this->puzzleCompletionStatus[this->storyCurrentChapter] >= 0x64) {
			this->var_str_384 = g_zbasic->str(12); // *
		}
		// 128:23e8
		this->sub_128_55c(this->var_str_384);
		// 128:23f0: CLR.W - -0x772(A5)
		this->var_i16_7d6 = 1;
		g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
		g_toolbox->MoveTo(0x8a, 0x133);
		g_toolbox->DrawString(this->arr_str_195e8[this->storyCurrentChapter]);
	} else {
		// 128:2430
		this->var_i16_7d6 = 0;
		g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
		g_toolbox->MoveTo(0x70, 0x133);
		g_toolbox->DrawString(this->arr_str_195e8[this->storyCurrentChapter]);
	}
	// 128:2468
	if (this->arr_str_1a288[this->storyNextPage] != g_zbasic->str(13)) { // blank
		g_zbasic->text(0xfa, 0xc, Graphics::kMacFontRegular, kSrcOr);
		g_toolbox->DrawString(this->arr_str_1a288[this->storyNextPage]);
	}
	// 128:24be
	if (this->storyNextPage != this->storyCurrentPage) {
		g_zbasic->text(0xfa, 0xc, Graphics::kMacFontRegular, kSrcOr);
		this->fillRect(0x2f, 0x37, 0x11f, 0x1db, 0);
		if (this->var_i16_7e0 != 0) {
			// Paste the previous lines of text, if we copied them
			g_zbasic->put(0x3c, 0x32, this->arr_bmp_b3ec, kPutCopy);
		}
		// 128:252a
		// y-position where the story text should start
		this->var_i16_7a2 = 0x11d - (this->arr_i16_1dee[this->storyNextPage] * 0xf);
		debugC(5, kDebugLoading, "FoolGame::storyRenderPage: storyNextPage %d, lines %d", this->storyNextPage, this->arr_i16_1dee[this->storyNextPage]);
		if (this->arr_i16_1dee[this->storyNextPage] < 0xf) {
			g_toolbox->MoveTo(0xfa, this->var_i16_7a2-0xf);
			g_toolbox->DrawString(g_zbasic->str(14)); // infinity symbol
			this->var_i16_7de = this->var_i16_7a2 - 0x1e;
		} else {
		// 128:25a6
			this->var_i16_7de = this->var_i16_7a2 - 0xf;
		}
		// 128:25b6
		// printing the story to the screen
		g_toolbox->MoveTo(0x41, this->var_i16_7a2);
		for (int i = this->pageLineRanges[this->storyNextPage*2]; i <= this->pageLineRanges[this->storyNextPage*2+1]; i++) {
			g_toolbox->TextFace(this->pageLineFace[i]);
			this->var_str_384 = g_zbasic->index(0, i);
			g_toolbox->DrawString(this->var_str_384);
			if (this->pageLineBreak[i] != 0) {
				// 128:262a
				this->var_i16_7a2 += 0xf;
				g_toolbox->MoveTo(0x41, this->var_i16_7a2);
			}
			// 128:263a
		}
		// 128:265c
		this->storyCurrentPage = this->storyNextPage;
	}
	// 128:2662
}

void FoolGame::sub_128_2664() {
	// 128:2664
	// hide menu and replace with "click mouse to continue" message
	g_toolbox->SetPort(this->var_i32_8);
	this->fillRect(0, 7, 0x13, this->var_i16_5a-7, 0);
	g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_str_172 = g_zbasic->str(15); // click mouse to continue
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo((this->var_i16_5a / 2) - (this->var_i16_30 / 2), 0xf);
	g_toolbox->DrawString(this->var_str_172);
	g_toolbox->_defaultMenu->setOverlayDirty(true);
	g_toolbox->SetPort(this->var_i32_0);
}

void FoolGame::sub_128_26f6() {
	// 128:26f6
	for (int i = 0; i <= 0x28; i++) {
		this->arr_i16_1eb8[i] = 0;
	}
}

void FoolGame::sub_128_271a() {
	int position = this->puzzleDataOffsets[this->activePuzzle] - 1;
	// record index
	this->var_i16_484 = (int16)(position / 1000);
	// offset at record
	this->var_i16_7e4 = (int16)(position % 1000);
	g_zbasic->record(1, this->var_i16_484, this->var_i16_7e4);
	this->var_ptr_696 = 0;
	int length = this->puzzleDataOffsets[this->activePuzzle + 1] - this->puzzleDataOffsets[this->activePuzzle];
	debugC(8, kDebugLoading, "sub_128_271a: seek puzzles to puzzle %d at position %x (rec: %d, offset: %d), length %x",
			this->activePuzzle,
			position,
			this->var_i16_484,
			this->var_i16_7e4,
			length
	);
	this->arr_bytes_109dc = g_zbasic->readFile(1, length);
	this->var_bytes_696 = this->arr_bytes_109dc;

}

// new game
void FoolGame::menuNewGame() {
	// 128:27d6
	this->savePrompt();
	if (this->var_i16_7be == 3)
		return;
	if (this->var_i16_7ce & 1) {
		this->setStateBits(kStateNewGame | kStateReturn);
	} else {
		this->setStateBits(kStateNewGame);
	}
}


void FoolGame::sub_128_2808() {
	// 128:2808
	this->var_i16_7e6 = 0;
	// 128:280e: LEA - [0x3808],A0
	// 128:2812: MOVE.L - A0,-0x8ee(A5)
	g_zbasic->openR(2, this->var_str_588, 0x400, this->var_i16_688);
	this->var_str_7e8 = g_zbasic->readFileStr(2, 0x11);
	if (this->var_str_7e8 == g_zbasic->str(16)) { // The Fool's Errand
		this->var_i16_8e8 = g_zbasic->readFileInt(2);
		this->var_i16_8e8 = g_zbasic->readFileInt(2);
		this->var_i16_8e8 = g_zbasic->readFileInt(2);
		this->var_i16_8e8 = g_zbasic->readFileInt(2);
		for (int i = 1; i <= 0x64; i++) {
			this->var_i16_8e8 = g_zbasic->readFileInt(2);
			this->var_i16_8e8 = g_zbasic->readFileInt(2);
			this->var_i16_8e8 = g_zbasic->readFileInt(2);
			this->var_i16_8e8 = g_zbasic->readFileInt(2);
			this->var_str_7e8 = g_zbasic->readFileStr(2, this->var_i16_8e8);
		}
		// 128:28c8
		for (int i = 1; i <= 0x51; i++) {
			this->var_i16_8e8 = g_zbasic->readFileInt(2);
		}

	} else {
		// 128:28ec
		this->var_i16_7e6 = 1;
	}
	g_zbasic->close(2);
	// 128:28f8: LEA - [0x3818],A0
	// 128:28fc: MOVE.L - A0,-0x8ee(A5)
	if (this->var_i16_7e6 != 0) {
		this->arr_str_1a8d8[0] = g_zbasic->str(17) + this->var_str_588 + g_zbasic->str(18);
		this->arr_str_1a8d8[1] = g_zbasic->str(19);
		this->sub_128_dfe(0, 0, 1, 1);
		this->var_str_588 = g_zbasic->str(20);
		this->var_i16_7e6 = 0;
	}
}

void FoolGame::menuOpenGame() {
	// 128:2988
	// File -> Open
	this->sub_128_1e4(g_zbasic->str(21)); // FOOL
	if (this->var_str_588 == g_zbasic->str(22)) { // empty
		return;
	}
	this->sub_128_2808();
	if (this->var_str_588 == g_zbasic->str(23)) {
		return;
	}
	this->savePrompt();
	// 128:29d4
	if (this->var_i16_7be != 3) {
		if ((this->var_i16_7ce & 1) != 0) {
			this->setStateBits(kStateOpenGame | kStateReturn);
		} else {
			// 128:29fa
			this->setStateBits(kStateOpenGame);
		}
		this->sub_128_3774();
	}
}

void FoolGame::menuSaveGame() {
	// 128:2a06
	// File -> Save
	this->setStateBits(kStateSaveGame);
}

void FoolGame::sub_128_2a0e() {
	// 128:2a0e
	if (this->var_str_8ec == g_zbasic->str(24)) {
		// 128:2a28
		this->sub_128_11c(g_zbasic->str(25), g_zbasic->str(26)); // name of game, blank
	} else {
		// 128:2a48
		this->sub_128_11c(g_zbasic->str(27), this->var_str_8ec); // New Name?
	}
	// 128:2a60
	if (this->var_str_486 != g_zbasic->str(28)) { // empty
		this->var_str_8ec = this->var_str_486;
		this->var_i16_9ec = this->var_i16_586;
		// 128:2a90
	}
}

void FoolGame::menuSaveGameAs() {
	// 128:2a92
	// File -> Save As
	this->sub_128_2a0e();
	if (this->var_str_486 != g_zbasic->str(29)) { // empty
		this->setStateBits(kStateSaveGame);
	}

}

// quit
void FoolGame::menuQuit() {
	// 128:2ab6
	this->savePrompt();
	if (this->var_i16_7be == 3) {
		return;
	}
	if ((this->var_i16_7ce & 1) != 0) {
		this->setStateBits(kStateQuit | kStateReturn);
	} else {
		this->setStateBits(kStateQuit);
	}
}

// print story
void FoolGame::menuPrintStory() {
	// 128:2ae8
	if (this->var_i16_7ce & 1) {
		this->stateFlags = 0x401;
	} else {
		this->stateFlags = 0x400;
	}
}



void FoolGame::sub_128_2b0a() {
	// 128:2b0a
	this->stateFlags = 0;
	// printing the story
	warning("STUB: %s", __func__);
}

void FoolGame::newGame() {
	// 128:2bc6
	this->clearStateBits(kStateNewGame);
	this->sub_128_3774();
	this->sub_128_3744();
	this->var_str_8ec = g_zbasic->str(33);

	this->var_i16_9ec = 0;
	// set up the initially visible story chapters
	this->arr_i16_4758[0] = 0x103;
	this->arr_i16_4758[1] = 0;
	this->arr_i16_4758[2] = 0x1e;
	this->arr_i16_4758[3] = 0x1fc;
	this->arr_i16_4758[4] = (int16)0xa050;
	this->arr_i16_4758[5] = 0x3f7;
	this->var_i16_484 = 0;
	for (int j = 0; j <= 0x5; j++) {
		// 128:2c5e
		for (int i = 0; i <= 0xf; i++) {
			this->var_i16_484++;
			if (this->arr_i16_4758[j] & this->bitLUT[i]) {
				this->pageVisible[this->var_i16_484] = 1;
			} else {
				// 128:2ca6
				this->pageVisible[this->var_i16_484] = 0;
			}
			// 128:2cb6
		}
	}
	// 128:2cce
	for (int16 i = 1; i <= 0x51; i++) {
		this->sunMapTileID[i] = i;
	}
	g_zbasic->unk_20();
	// 128:2cf6
	// shuffle around the pieces of the sun's map
	for (int i = 1; i <= 0x64; i++) {
		this->var_i16_484 = g_zbasic->rndInt(0x51);
		this->var_i16_7e4 = g_zbasic->rndInt(0x51);
		g_zbasic->swapInt(this->sunMapTileID[this->var_i16_484], this->sunMapTileID[this->var_i16_7e4]);
		this->puzzleCompletionStatus[i] = 0;
		this->puzzleFlags[i] = kFlagMenuDisabled;
	}
	// 128:2d6a
	this->var_i16_484 = 0;
	for (int i = 1; i <= this->storyPageCount; i++) {
		if (this->pageToChapter[i] == this->var_i16_484+1) {
			// 128:2d9c
			this->var_i16_484++;
			if (this->pageVisible[i] == 1) {
				if (this->puzzleUnlockChapter[this->var_i16_484] != 0) {
					this->var_i16_9f2 = kFlagMenuDiamond;
				} else {
					this->var_i16_9f2 = kFlagMenuEnabled;
				}
				// 128:2de2
				this->puzzleFlags[this->var_i16_484] = this->var_i16_9f2 + 4;
			}
			// 128:2e02
		}
		// 128:2e02
	}
	// 128:2e14
	g_zbasic->indexClear(0x2);
	this->storyNextPage = 1;
	this->var_i16_7ce = 0;
	this->activePuzzle = 0;
	this->sunMapRestored = 0;
	this->var_i16_7b2 = 0;
	this->sub_128_3032();
	// 128:2e3c
}

void FoolGame::openGame() {
	// 128:2e3e
	// save game loading code?
	this->clearStateBits(kStateOpenGame);
	this->sub_128_3774();
	this->sub_128_3744();
	this->var_str_8ec = this->var_str_588;
	this->var_i16_9ec = this->var_i16_688;
	this->var_i16_7e6 = 0;
	// FIXME: disk error handler??
	// 128:2e68: LEA - [0x3808],A0
	// 128:2e6c: MOVE.L - A0,-0x8ee(A5)
	// 128:2e70: SF - 0x8,D0
	g_zbasic->openR(2, this->var_str_8ec, 0x400, this->var_i16_9ec);

	this->var_str_384 = g_zbasic->readFileStr(2, 0x11);
	this->storyNextPage = g_zbasic->readFileInt(2);
	this->activePuzzle = g_zbasic->readFileInt(2);
	this->var_i16_7ce = g_zbasic->readFileInt(2);
	this->sunMapRestored = g_zbasic->readFileInt(2);
	for (int i = 1; i <= 0x64; i++) {
		this->puzzleFlags[i] = g_zbasic->readFileInt(2);
		this->puzzleCompletionStatus[i] = g_zbasic->readFileInt(2);
		this->pageVisible[i] = g_zbasic->readFileInt(2);
		int16 payloadSize = g_zbasic->readFileInt(2);
		debugC(5, kDebugLoading, "openGame: puzzle %d:, puzzleFlags: %d, puzzleCompletionStatus: %d, pageVisible: %d, payload size: %d", i, this->puzzleFlags[i], this->puzzleCompletionStatus[i], this->pageVisible[i], payloadSize);
		Common::String state = g_zbasic->readFileStr(2, payloadSize);
		if (debugChannelSet(5, kDebugLoading)) {
			Common::hexdump((const byte *)state.c_str(), payloadSize);
		}
		g_zbasic->indexRawSet(state, 2, i);
	}
	// 128:2f54
	debugCN(5, kDebugLoading, "openGame: sun map tile IDs: ");
	for (int i = 1; i <= 0x51; i++) {
		this->sunMapTileID[i] = g_zbasic->readFileInt(2);
		debugCN(5, kDebugLoading, "%d, ", this->sunMapTileID[i]);
	}
	debugCN(5, kDebugLoading, "\n");
	// 128:2f84
	g_zbasic->close(2);
	if (this->var_i16_7e6 != 0) {
		this->arr_str_1a8d8[0] = g_zbasic->str(34) + this->var_str_8ec + g_zbasic->str(35); // file cannot be opened
		this->arr_str_1a8d8[1] = g_zbasic->str(36);
		this->var_i16_7e6 = 0;
		if ((this->var_i16_7ce & 1) == 0) {
			this->setStateBits(kStateNewGame | kStateReturn);
		} else {
			this->setStateBits(kStateNewGame);
		}
	} else {
		// 128:302c
		this->sub_128_3032();
	}

}

void FoolGame::sub_128_3032() {
	this->storyCurrentChapter = 0;
	this->stateFlags = 0;
	this->var_i16_484 = 0;
	this->var_i16_68c = 3;
	// render chapter menu headings
	for (int j = 3; j <= 7; j++) {
		this->var_str_384 = g_zbasic->str(37) + g_zbasic->chr(0x7d+j) + g_zbasic->str(38);
		g_zbasic->menu(j, 0, 1, this->var_str_384);
		for (int i = 1; i <= 0x10; i++) {
			this->var_i16_484++;
			this->var_i16_9f2 = this->puzzleFlags[this->var_i16_484] & (kFlagMenuDiamond | kFlagMenuEnabled);
			if (this->puzzleCompletionStatus[this->var_i16_484] == 0x63) {
				this->var_str_384 = g_zbasic->str(39); // " ~ "
			} else {
			// 128:30e4
				this->var_str_384 = g_zbasic->str(40); // " "
			}
			// 128:30f8
			this->var_str_9f4 = this->arr_str_195e8[this->var_i16_484] + this->var_str_384;
			if ((j == 3) && (i == 1)) {
				this->var_str_9f4 = g_zbasic->str(41) + this->var_str_9f4;
			}
			// 128:315e
			g_zbasic->menu(j, i, this->var_i16_9f2, this->var_str_9f4);
		}
		// 128:3180
	}
	// 128:318e
	this->fillRect(0x1e, 0x37, 0x32, 0xc8, 0);
	this->fillRect(0x2f, 0x37, 0x11f, 0x1db, 0x0);
	g_zbasic->text(0xfa, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_i16_7a2 = 0x3c;
	this->var_i16_7de = 0x2d;
	// draw the first page of text
	for (int i = 1; i <= 0xf; i++) {
		// 128:31ec
		g_toolbox->MoveTo(0x41, this->var_i16_7a2);
		this->var_str_384 = g_zbasic->index(0, i);
		g_toolbox->DrawString(this->var_str_384);
		this->var_i16_7a2 += 0xf;
	}
	// 128:3228
	this->copyScreen(0, this->arr_bmp_5dfc);
	this->storyCurrentPage = 1;
	this->storyRenderPage();
	if ((this->var_i16_7ce & 1) == 0) {
		this->cursorExplodingWatch();
	} else {
		if ((this->activePuzzle > 0) && (this->activePuzzle <= 0x50)) {
			this->setStateBits(kStatePuzzleSelect);
		} else {
			this->setStateBits(0x100);
		}
	}
	// 128:3294
	this->sub_128_32c8();
	if (this->sunMapRestored == 0) {
		g_zbasic->menu(2, 7, 0, Common::U32String());
	} else {
		g_zbasic->menu(2, 7, 1, Common::U32String());
	}
}

void FoolGame::sub_128_32c8() {
	if (this->soundEnabled == 0) {
		g_zbasic->menu(2, 6, 1, Common::U32String());
	} else {
		g_zbasic->menu(2, 6, 0x86, Common::U32String());
	}
}

void FoolGame::savePrompt() {
	// 128:32fa
	if (this->var_i16_7ce == 666) {
		return;
	}
	this->var_i16_7be = 0;
	if (this->var_str_8ec == g_zbasic->str(42)) {
		this->arr_str_1a8d8[0] = g_zbasic->str(43); // do you wish to save this game
		this->arr_str_1a8d8[1] = g_zbasic->str(44); // yes
		this->arr_str_1a8d8[2] = g_zbasic->str(45); // no
		this->arr_str_1a8d8[3] = g_zbasic->str(46); // cancel
		this->sub_128_dfe(0, 0, 3, 0);
		if (this->var_i16_7be > 1) {
			return;
		}
		this->sub_128_2a0e();
		if (this->var_str_8ec == g_zbasic->str(47)) {
			// 128:33da
			this->var_i16_7be = 3;
			return;
		}
	}
	// 128:33e2
	this->setStateBits(kStateSaveGame);

}

void FoolGame::saveGame() {
	// write save file
	this->clearStateBits(kStateSaveGame);
	if (this->var_i16_7ce == 666)
		return;
	// 128:3548
	if (this->var_str_8ec == g_zbasic->str(57)) { // empty
		this->sub_128_2a0e();
		if (this->var_str_8ec == g_zbasic->str(58)) { // empty
			return;
		}
	}
	// 128:3580
	if ((this->var_i16_7ce & 1) != 0) {
		this->puzzleSaveContext();
	}
	// 128:3594
	do {
		this->sub_128_3774();
		// 128:3598
		this->var_i16_7e6 = 0;
		// 128:359e: LEA - [0x3808],A0
		// 128:35a2: MOVE.L - A0,-0x8ee(A5)
		g_zbasic->defOpen(g_zbasic->str(59)); // FOOLgf87
		g_zbasic->openW(2, this->var_str_8ec, 0x400, this->var_i16_9ec);

		if (this->var_i16_7e6 != 0xa) {
			this->var_str_af4 = g_zbasic->str(60); // The Fool's Errand
			g_zbasic->writeFileStr(2, this->var_str_af4.encode(Common::kMacRoman));
			g_zbasic->writeFileInt(2, this->storyNextPage);
			g_zbasic->writeFileInt(2, this->activePuzzle);
			g_zbasic->writeFileInt(2, this->var_i16_7ce);
			g_zbasic->writeFileInt(2, this->sunMapRestored);
			for (int i = 1; i <= 0x64; i++) {
				g_zbasic->writeFileInt(2, this->puzzleFlags[i]);
				g_zbasic->writeFileInt(2, this->puzzleCompletionStatus[i]);
				g_zbasic->writeFileInt(2, this->pageVisible[i]);
				Common::String state = g_zbasic->indexRaw(2, i);
				g_zbasic->writeFileInt(2, (int16)state.size());
				g_zbasic->writeFileStr(2, state);
			}
			// 128:36ba
			for (int i = 1; i <= 0x51; i++) {
				g_zbasic->writeFileInt(2, this->sunMapTileID[i]);
			}
			g_zbasic->close(2);
		}
		// 128:36fe
		if (this->var_i16_7e6 != 0) {
			// problem with saving
			this->sub_128_388a();
			do {
				this->sub_128_2a0e();
			} while (this->var_str_8ec == g_zbasic->str(61));  // empty
			// 128:3726: BRA - [0x3594]

		} else {
			break;
		}
	} while (true);
	// 128:372a
	// 128:372a: LEA - [0x3818],A0
	// 128:372e: MOVE.L - A0,-0x8ee(A5)
	if (this->stateFlags == 0) {
		this->sub_128_37ce();
		g_toolbox->InitCursor();
	}
}

void FoolGame::sub_128_3744() {
	// 128:3744
	for (int i = 3; i <= 7; i++) {
		this->var_menu_bf8 = g_toolbox->GetMHandle(i);
		g_toolbox->DeleteMenu(i);
		g_toolbox->DisposeMenu(this->var_menu_bf8);
	}
	g_toolbox->DrawMenuBar();
}

// watch cursor
void FoolGame::sub_128_3774() {
	// 128:3774
	g_toolbox->SetCursor(this->arr_curs_4d88[3]);
	this->var_i16_7b2 = 0xa;
}

void FoolGame::cursorExplodingWatch() {
	// 128:378a
	for (int j = 0; j <= 1; j++) {
		for (int i = 3; i <= 6; i++) {
			g_toolbox->SetCursor(this->arr_curs_4d88[i]);
			this->sub_128_3da(3);
		}
	// 128:37c2
	}
	for (int i = 7; i <= 0xf; i++) {
		g_toolbox->SetCursor(this->arr_curs_4d88[i]);
		this->sub_128_3da(4);
	}

	this->var_i16_7b2 = 0xa;
}

void FoolGame::sub_128_37ce() {
	// 128:37ce
	for (int i = 7; i <= 0xf; i++) {
		g_toolbox->SetCursor(this->arr_curs_4d88[i]);
		this->sub_128_3da(4);
	}
	this->var_i16_7b2 = 0xa;
}

void FoolGame::sub_128_388a() {
	// 128:388a
	if ((this->var_i16_7e6 & 0xff) == 2) {
		// problem with saving
	}
	warning("STUB: %s", __func__);
}

void FoolGame::puzzleRun() {
	// 128:39a0
	this->fillRect(0x127, 0x69, 0x138, 0x190, 0);
	this->stateFlags = 0;
	this->var_i16_c00 = 0;
	this->var_i16_7d6 = 0;
	this->var_i16_7b2 = 0xa;
	this->activePuzzle = this->storyCurrentChapter;
	this->var_i16_7ce |= 1;
	g_toolbox->InitCursor();
	this->copyScreen(0, this->arr_bmp_5dfc);
	// 128:39fa
	if (this->arr_i16_15e8[this->storyCurrentChapter] > 0) {
		this->sub_128_41d8();
	}
	// 128:3a12
	this->sub_128_3fb6();
	this->sub_128_26f6();
	this->sub_128_61ec();
	this->puzzleLoadContext();
	// 128:3a22
	// 128:3a38: JSR - "ZBASIC_115"
	debugC(5, kDebugLoading, "puzzleRun: chapter 0x%x, puzzle module %d", this->storyCurrentChapter, this->puzzleType[this->storyCurrentChapter]-1);
	switch (this->puzzleType[this->storyCurrentChapter]-1) {
	case 0:
	case 1:
	case 2:
	case 3:
		this->jumbleRun();
		break;
	case 4:
		this->wordSearchRun();
		break;
	case 5:
		this->jigsawRun();
		break;
	case 6:
		this->polyominoRun();
		break;
	case 7:
		this->revealRun();
		break;
	case 8:
		this->sentenceRun();
		break;
	case 9:
		this->mazeRun();
		break;
	case 10:
		this->jumbleRun();
		break;
	case 11:
		this->sunMapRun();
		break;
	case 12:
		this->cardsRun();
		break;
	case 13:
		this->highPriestessRun();
		break;
	case 14:
		this->deathRun();
		break;
	case 15:
		this->shipsRun();
		break;
	case 16:
		this->humbugRun();
		break;
	case 17:
		this->justiceRun();
		break;
	case 18:
		this->hermitRun();
		break;
	default:
		warning("%s: broke out of switch", __func__);
		break;
	}
	// 128:3a8a
	this->sub_128_5fea();
	if (this->sunMapRestored == 1) {
		this->setStateBits(0x100);
		return;
	}
	// 128:3aa4
	this->puzzleSaveContext();
	if ((this->stateFlags & kStateQuit) == 0) {
		g_toolbox->PenNormal();
		this->copyScreen(1, this->arr_bmp_5dfc);
		this->var_menu_bf8 = g_toolbox->GetMHandle(8);
		g_toolbox->DeleteMenu(8);
		g_toolbox->DisposeMenu(this->var_menu_bf8);
		g_toolbox->DrawMenuBar();
	}
	// 128:3aea
	g_zbasic->indexClear(1);
	if (this->puzzleCompletionStatus[this->storyCurrentChapter] == 0x63) {
		this->storyUnlockChapter();
	}
	if (this->puzzleCompletionStatus[this->storyCurrentChapter] == 0x64) {
		// bodge for completing The Chariot
		if (this->storyCurrentChapter == 0x41) {
			if ((this->puzzleFlags[0x41] & kFlagMenuDiamond) != 0) {
				this->puzzleFlags[0x41] ^= kFlagMenuDiamond;
				this->puzzleFlags[0x41] |= kFlagMenuEnabled;
			}
			// 128:3b9e
			for (int i = 1; i <= 5; i++) {
				g_zbasic->menu(7, i, 1, Common::U32String());
				if (i > 1) {
					this->puzzleFlags[0x40 + i] |= 0x5;
				}
			}
			// 128:3c04
			this->storyNextPage = 0;
			// enable the four King Of ... chapters
			for (int i = 0x42; i <= 0x45; i++) {
				for (int j = 1; j <= this->storyPageCount; j++) {
					if (this->pageToChapter[j] == i) {
						this->pageVisible[j] = 1;
						if ((i == 0x42) && (this->storyNextPage == 0)) {
							this->storyNextPage = j;
						}
					}
					// 128:3c6c
					if (this->pageToChapter[j] > i) {
						j = this->storyPageCount;
					}
					// 128:3c8e
				}
			}
			// 128:3cae
			this->var_i16_7ce |= 8;
		} else {
			// 128:3cc0
			if ((this->puzzleFlags[this->storyCurrentChapter] & 2) != 0) {
				this->puzzleFlags[this->storyCurrentChapter] ^= 2;
				this->puzzleFlags[this->storyCurrentChapter] |= 1;
			}
			// 128:3d3a
			this->var_i16_484 = 3 + ((this->storyCurrentChapter - 1) / 0x10);
			this->var_i16_7e4 = 1 + ((this->storyCurrentChapter - 1) % 0x10);
			g_zbasic->menu(this->var_i16_484, this->var_i16_7e4, 1, Common::U32String());
			this->storyUnlockChapter();
		}
	}
	// 128:3d82
	this->var_i16_c02 = 0;
	this->var_i16_c00 = 0;
	g_toolbox->SetPort(this->var_i32_0);
	if ((this->stateFlags & 0x38) == 0) {
		if ((this->var_i16_7ce & 1) == 0) {
			this->var_i16_7ce ^= 1;
		}
		if ((this->stateFlags & kStateChapterSelect) == 0) {
			this->storyRenderPage();
		}
	}
	// 128:3de0
	this->sub_128_61ec();
	// 128:3de4
}

void FoolGame::storyUnlockChapter() {
	// 128:3de6
	if ((this->puzzleUnlockChapter[this->storyCurrentChapter] > 0) &&
		((this->puzzleFlags[this->puzzleUnlockChapter[this->storyCurrentChapter]] & 3) == 0)) {
		this->storyNextPage = 0;
		this->var_i16_7ce |= 8;
		for (int i = 1; i <= this->storyPageCount; i++ ) {
			// 128:3e56
			if (this->pageToChapter[i] == this->puzzleUnlockChapter[this->storyCurrentChapter]) {
				this->pageVisible[i] = 1;
				if (this->storyNextPage == 0) {
					this->storyNextPage = i;
				}
			}
			// 128:3ea6
			if (this->pageToChapter[i] > this->puzzleUnlockChapter[this->storyCurrentChapter]) {
				i = this->storyPageCount;
			}
			// 128:3eda
		}
		// 128:3eec
		this->storyCurrentChapter = this->pageToChapter[this->storyNextPage];
		int16 menuNo = 3 + ((this->storyCurrentChapter - 1) / 0x10);
		int16 itemNo = 1 + ((this->storyCurrentChapter - 1) % 0x10);
		if (this->puzzleUnlockChapter[this->storyCurrentChapter] == 0) {
			this->var_i16_9f2 = 1;
			this->puzzleCompletionStatus[this->storyCurrentChapter] = 0x64;
		} else {
		// 128:3f66
			this->var_i16_9f2 = 2;
		}
		this->puzzleFlags[this->storyCurrentChapter] |= (this->var_i16_9f2 + 4);
		g_zbasic->menu(menuNo, itemNo, this->var_i16_9f2, Common::U32String());
	}
	// 128:3fb4
}

void FoolGame::sub_128_3fb6() {
	// 128:3fb6
	// new: delete existing menu 8
	this->var_menu_bf8 = g_toolbox->GetMHandle(8);
	g_toolbox->DeleteMenu(8);
	g_toolbox->DisposeMenu(this->var_menu_bf8);

	g_zbasic->menu(8, 0, 1, this->arr_str_195e8[this->activePuzzle]);
	this->var_str_384 = g_zbasic->str(72); // return to scroll
	if ((this->activePuzzle == 0x34) || (this->activePuzzle == 0x35)) {
		this->var_str_384 = g_zbasic->str(73); // run for your life
	}
	// 128:4024
	if (this->activePuzzle > 0x50) {
		this->var_str_384 = g_zbasic->str(74); // return to map
	}
	g_zbasic->menu(8, 1, 1, this->var_str_384);
	// 128:4056
	if (this->arr_i16_1b10[this->puzzleType[this->activePuzzle]*2] <= this->arr_i16_1b10[this->puzzleType[this->activePuzzle]*2 + 1]) {
		g_zbasic->menu(8, 2, 0, g_zbasic->str(75)); // line
		// 128:40c6
		this->var_i16_484 = 2;
		for (int i = this->arr_i16_1b10[this->puzzleType[this->activePuzzle]*2]; i <= this->arr_i16_1b10[this->puzzleType[this->activePuzzle]*2 + 1]; i++) {
			// 128:40f6
			this->var_i16_484++;
			this->var_str_384 = g_zbasic->index(0, i) + g_zbasic->str(76);
			g_zbasic->menu(8, this->var_i16_484, 1, this->var_str_384);
		}
	}
	// 128:4166
}

void FoolGame::puzzleLoadContext() {
	// 128:4168
	this->var_i16_c04 = this->puzzleCompletionStatus[this->activePuzzle];
	if (this->var_i16_c04 == 0x64) {
		this->var_i16_c04 = 0x65;
	}
	this->activePuzzleBuffer = g_zbasic->indexRaw(2, this->activePuzzle);
	if (debugChannelSet(5, kDebugLoading)) {
		Common::String inter = this->activePuzzleBuffer;
		debugC(5, kDebugLoading, "puzzleLoadContext: loading puzzle %d context, state %d", this->activePuzzle, this->var_i16_c04);
		Common::hexdump((const byte *)inter.c_str(), inter.size());
	}
}

void FoolGame::puzzleSaveContext() {
	// 128:41aa
	this->puzzleCompletionStatus[this->activePuzzle] = this->var_i16_c04;
	g_zbasic->indexRawSet(this->activePuzzleBuffer, 2, this->activePuzzle);
	if (debugChannelSet(5, kDebugLoading)) {
		Common::String inter = this->activePuzzleBuffer;
		debugC(5, kDebugLoading, "puzzleSaveContext: saving puzzle %d context, state %d", this->activePuzzle, this->var_i16_c04);
		Common::hexdump((const byte *)inter.c_str(), inter.size());
	}
}

void FoolGame::sub_128_41d8() {
	// 128:41d8
	for (int i = 0; i <= 5; i++) {
		this->arr_i16_4758[i] = this->arr_i16_16b2[i + (this->arr_i16_15e8[this->activePuzzle]*8)];
	}
	// 128:422e
	if ((this->activePuzzle == 0x34) && (this->puzzleCompletionStatus[0x34] > 1)) {
		// 128:4262
		if (this->puzzleCompletionStatus[0x34] == 2) {
			this->arr_i16_4758[0] = 0;
			this->arr_i16_4758[1] = 2;
			this->arr_i16_4758[2] = 0xa;
		}
		// 128:42aa
		if (this->puzzleCompletionStatus[0x34] == 3) {
			this->arr_i16_4758[0] = 0;
			this->arr_i16_4758[1] = 3;
			this->arr_i16_4758[2] = 0;
		}
		// 128:42f0
		if (this->puzzleCompletionStatus[0x34] >= 4) {
			if (this->sunMapRestored == 0) {
				this->arr_i16_4758[0] = 0;
				this->arr_i16_4758[1] = 0x47;
				this->arr_i16_4758[2] = 0;
			} else {
				// 128:4346
				return;
			}
		}
		// 128:4348
	}
	// 128:4348
	if ((this->activePuzzle == 0x17) && (this->puzzleCompletionStatus[0x17] == 0x63)) {
		this->arr_i16_4758[0] = 0;
		this->arr_i16_4758[1] = 2;
		this->arr_i16_4758[1] = 0xa;
	}
	// 128:43aa
	if ((this->activePuzzle == 0x3f) && (this->puzzleCompletionStatus[0x3f] == 0x63)) {
		this->arr_i16_4758[0] = 0;
		this->arr_i16_4758[1] = 3;
		this->arr_i16_4758[2] = 0;
	}
	// 128:440a
	if (this->activePuzzle == 0x48) {
		if (this->puzzleCompletionStatus[0x48] == 0x63) {
			this->arr_i16_4758[0] = 0;
			this->arr_i16_4758[1] = 0x47;
			this->arr_i16_4758[2] = 0;
		} else {
			// 128:4462
			this->arr_i16_4758[1] = 0x31;
		}
	}
	sub_128_4472();
}

void FoolGame::sub_128_4472() {
	// 128:4472
	if (this->arr_i16_4758[1] == 0) {
		this->arr_i16_4758[1] = this->activePuzzle;
	}
	if (this->arr_i16_4758[2] == 0) {
		this->arr_i16_4758[2] = 0x8;
	}
	if (this->arr_i16_4758[0] == 0) {
		// user clicked the puzzle button, zoom-fill with pattern
		sub_128_962(0x130, 0x76, 0x130, 0x76, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, this->arr_i16_4758[1], (PatternMode)this->arr_i16_4758[2], 0x19);
		return;
	}
	// 128:4520
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[this->arr_i16_4758[1]]);
	g_toolbox->PenMode((PatternMode)this->arr_i16_4758[2]);
	if (this->arr_i16_4758[5] == 0) {
		// 128:456a
		this->arr_i16_4758[12] = 0x1d;
		this->arr_i16_4758[6] = 0x4;
		this->arr_i16_4758[7] = 0x7;
	}
	// 128:459a
	if (this->arr_i16_4758[5] == 1) {
		this->arr_i16_4758[12] = 0x2a;
		this->arr_i16_4758[6] = 0x4;
		this->arr_i16_4758[7] = 0x6;
	}
	// 128:45e4
	g_toolbox->PenSize(this->arr_i16_4758[7], this->arr_i16_4758[6]);
	if ((this->arr_i16_4758[4] == 0) && (this->arr_i16_4758[5] == 0)) {
		this->arr_i16_4758[8] = 0xa7;
		this->arr_i16_4758[9] = 0x109;
		this->arr_i16_4758[10] = 0xa7;
		this->arr_i16_4758[11] = 0x109;
	}
	// 128:467c
	if ((this->arr_i16_4758[4] == 0) && (this->arr_i16_4758[5] == 1)) {
		this->arr_i16_4758[8] = 0xb5;
		this->arr_i16_4758[9] = 0x100;
		this->arr_i16_4758[10] = 0xb5;
		this->arr_i16_4758[11] = 0x100;
	}
	// 128:46f6
	if (this->arr_i16_4758[4] == 1) {
		this->arr_i16_4758[6] *= -1;
		this->arr_i16_4758[7] *= -1;
	}
	// 128:4754
	if ((this->arr_i16_4758[4] == 1) && (this->arr_i16_4758[5] == 0)) {
		this->arr_i16_4758[8] = 0x2b;
		this->arr_i16_4758[9] = 0x31;
		this->arr_i16_4758[10] = 0x123;
		this->arr_i16_4758[11] = 0x1e1;
	}
	// 128:47ce
	if ((this->arr_i16_4758[4] == 1) && (this->arr_i16_4758[5] == 1)) {
		this->arr_i16_4758[8] = 0x10;
		this->arr_i16_4758[9] = -0x7;
		this->arr_i16_4758[10] = 0x15a;
		this->arr_i16_4758[11] = 0x207;
	}
	// 128:4850
	if ((this->arr_i16_15e8[this->activePuzzle] == 0xd) || (this->arr_i16_15e8[this->activePuzzle] == 0xe)) {
		this->fillRect(0x14f, 0, 0x156, 0x7, 1);
		this->fillRect(0x14f, 0x1f9, 0x156, 0x200, 1);
	}
	// 128:48d6
	for (int i = 0; i <= this->arr_i16_4758[12]; i++) {
		this->arr_i16_4758[8] -= this->arr_i16_4758[6];
		this->arr_i16_4758[9] -= this->arr_i16_4758[7];
		this->arr_i16_4758[10] += this->arr_i16_4758[6];
		this->arr_i16_4758[11] += this->arr_i16_4758[7];
		if (this->arr_i16_4758[8] < 0x14) {
			this->arr_i16_4758[8] = 0x14;
		}
		// 128:49e2
		if (this->arr_i16_4758[3] == 0) {
			this->arr_i16_4758[1] += 1;
			if (this->arr_i16_4758[1] > 0x50) {
				this->arr_i16_4758[1] = 1;
			}
			// 128:4a42
			g_toolbox->PenPat(this->arr_pat_58f4[this->arr_i16_4758[1]]);
		}
		// 128:4a64
		// FIXME: These can sometimes be invalid.
		// Create manually to avoid the assert crash in the constructor.
		Common::Rect temp;
		temp.left = this->arr_i16_4758[9];
		temp.top = this->arr_i16_4758[8];
		temp.right = this->arr_i16_4758[11];
		temp.bottom = this->arr_i16_4758[10];
		g_toolbox->FrameRect(temp);
		// add fake delay for drawing visibility
		if ((i % 4) == 0) {
			g_toolbox->Delay(0);
		}
	}
	// 128:4a8e
	g_toolbox->PenNormal();
}

// about screen
void FoolGame::menuAbout() {
	// 128:4a92
	if (this->var_str_8ec == g_zbasic->str(77)) {
		// untitled game in progress
		this->arr_str_1a8d8[0] = g_zbasic->chr(0x22) + g_zbasic->str(78) + g_zbasic->chr(0x22) + g_zbasic->str(79);
	} else {
		// 128:4aee
		this->arr_str_1a8d8[0] = g_zbasic->chr(0x22) + this->var_str_8ec + g_zbasic->chr(0x22) + g_zbasic->str(80);
	}
	// 128:4b28
	this->arr_str_1a8d8[1] = g_zbasic->str(81);
	if (this->sunMapRestored == 0) {

		this->var_i16_68c = 0x51;
		this->var_i16_68a = 1;
		for (int i = 1; i <= 0x51; i++) {
			if ((this->puzzleFlags[i] & 3) != 0) {
				this->var_i16_68c--;
			}
		}
		// 128:4b88
		if (this->var_i16_68c >= 0) {
			// sun's map is incomplete
			this->arr_str_1a8d8[2] = g_zbasic->str(82);
			if (this->var_i16_68c == 1) {
				// there is 1 missing piece
				this->arr_str_1a8d8[3] = g_zbasic->str(83);
			} else {
				// 128:4bde
				// there are N missing pieces
				this->arr_str_1a8d8[3] = Common::U32String::format("%s %d%s", g_zbasic->str(84).encode().c_str(), this->var_i16_68c, g_zbasic->str(85).encode().c_str());
			}
			// 128:4c1a
			this->var_i16_68c = 0;
			for (int i = 2; i <= 0x50; i++) {
				this->var_i16_68a = 2;
				if ((this->puzzleCompletionStatus[i] < 0x63) && (this->puzzleType[i] > 0)) {
					this->var_i16_68c++;
				}
			}
			// 128:4c74
			// blank
			this->arr_str_1a8d8[4] = g_zbasic->str(86);
			if (this->var_i16_68c == 1) {
				// There is 1 unsolved puzzle.
				this->arr_str_1a8d8[5] = g_zbasic->str(87);
			} else {
				// 128:4cc0
				// There are X unsolved puzzles.
				this->arr_str_1a8d8[5] = Common::U32String::format("%s %d%s", g_zbasic->str(88).encode().c_str(), this->var_i16_68c, g_zbasic->str(89).encode().c_str());
			}
			// 128:4cfc
			this->var_i16_d06 = 5;

		} else {
			// 128:4d06
			// You have all the map pieces, but they are not arranged in the correct order.
			this->arr_str_1a8d8[2] = g_zbasic->str(90);
			this->arr_str_1a8d8[3] = g_zbasic->str(91);
			this->arr_str_1a8d8[4] = g_zbasic->str(92);
			this->arr_str_1a8d8[5] = g_zbasic->str(93);
			// Only the Book of Thoth remains unsolved.
			this->arr_str_1a8d8[6] = g_zbasic->str(94);
			this->arr_str_1a8d8[7] = g_zbasic->str(95);
			this->var_i16_d06 = 7;
		}
		// 128:4dc0
	} else {
		// 128:4dc4
		// The sun's map is restored.
		this->arr_str_1a8d8[2] = g_zbasic->str(96);
		this->arr_str_1a8d8[3] = g_zbasic->str(97);
		// Only the Book of Thoth remains unsolved.
		this->arr_str_1a8d8[4] = g_zbasic->str(98);
		this->arr_str_1a8d8[5] = g_zbasic->str(99);
		this->var_i16_d06 = 5;
	}
	// 128:4e42
	g_toolbox->SetPort(this->var_i32_4);
	this->var_i16_7b2 = 0xa;
	g_toolbox->InitCursor();
	this->sub_128_4da(1);
	this->copyScreen(0, this->arr_bmp_138bc);
	g_zbasic->text(0xfa, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_i16_7b4 = 0;
	this->var_i16_7b6 = 0;
	this->var_i16_7b8 = 0;
	for (int i = 0; i <= this->var_i16_d06 ; i++) {
		this->var_i16_7ba = g_toolbox->StringWidth(this->arr_str_1a8d8[i]);
		if (this->var_i16_7ba > this->var_i16_7b4) {
			this->var_i16_7b4 = this->var_i16_7ba;
		}
		// 128:4ec4
		this->var_i16_7b6 += 0x11;
	}
	// 128:4eda
	this->var_i16_7b6 += 0xd;
	this->var_i16_7b4 = (this->var_i16_7b4/2) + 0xf;
	this->var_i16_7b6 = (this->var_i16_7b6/2);
	g_toolbox->SetRect(
		this->arr_rect_5b7c,
		0xf5 - this->var_i16_7b4,
		0x6e - this->var_i16_7b6,
		0x10b + this->var_i16_7b4,
		0xcf + this->var_i16_7b6
	);
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_58f4[0]);
	g_toolbox->FrameRect(this->arr_rect_5b7c);
	// 128:4f6a
	g_toolbox->InsetRect(this->arr_rect_5b7c, 1, 1);
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenPat(this->arr_pat_58f4[2]);
	g_toolbox->FrameRect(this->arr_rect_5b7c);
	g_toolbox->InsetRect(this->arr_rect_5b7c, 5, 5);
	g_toolbox->PenSize(5, 5);
	g_toolbox->PenPat(this->arr_pat_58f4[1]);
	g_toolbox->FrameRect(this->arr_rect_5b7c);
	// 128:4fe2
	g_toolbox->InsetRect(this->arr_rect_5b7c, 5, 5);
	g_toolbox->FillRect(this->arr_rect_5b7c, this->arr_pat_58f4[0]);
	g_toolbox->PenNormal();
	// 128:5014
	g_toolbox->SetRect(
		this->arr_rect_5b7c,
		0xb5,
		0x82 - this->var_i16_7b6,
		0xd5,
		0x82 - this->var_i16_7b6 + 0x20
	);
	g_toolbox->PlotIcon(this->arr_rect_5b7c, this->var_i32_c);
	// the fool's errand
	g_toolbox->MoveTo(0xde, 0x8c - this->var_i16_7b6);
	g_toolbox->DrawString(g_zbasic->str(100));
	// by Cliff Johnson
	g_toolbox->MoveTo(0xee, 0x9b - this->var_i16_7b6);
	g_toolbox->DrawString(g_zbasic->str(101));

	// 128:509a
	this->var_i16_7a2 = 0xbe - this->var_i16_7b6;
	for (int i = 0; i <= this->var_i16_d06; i++) {
		this->var_str_384 = this->arr_str_1a8d8[i];
		this->sub_128_918(this->var_str_384);
		this->var_i16_7a2 += 0x11;
	}
	// 128:50f4
	g_zbasic->text(0xfc, 0x9, Graphics::kMacFontRegular, kSrcOr);
	this->var_i16_7a2 += 0xe;
	this->sub_128_918(g_zbasic->str(102)); // version string
	this->sub_128_2664();
	this->sub_128_61c2();
	g_toolbox->DrawMenuBar();
	this->copyScreen(0x1, this->arr_bmp_138bc);
	g_toolbox->SetPort(this->var_i32_0);
}

// three freakin' ships
void FoolGame::shipsRun() {
	// 128:5140
	this->fillRect(0x127, 0x6c, 0x137, 0x84, 0);
	this->sub_128_6186();
	this->sub_128_55c(g_zbasic->str(103)); // "?"
	g_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	//warning("sub_128_5140: b3ec surface");
	//byte fakePal[768];
	//Common::fill(fakePal, fakePal+3, 0xff);
	//Common::fill(fakePal+3, fakePal+768, 0x00);
	//this->arr_bmp_b3ec->rawSurface().debugPrint(5, 0, 0, 0, 0, -1, 160, fakePal);

	this->fillRect(0x127, 0x6c, 0x137, 0x84, 0);

	g_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_bbbc);
	//warning("sub_128_5140: bbbc surface");
	//this->arr_bmp_bbbc->rawSurface().debugPrint(5, 0, 0, 0, 0, -1, 160, fakePal);
	// 138:51f0
	this->var_i16_d08 = g_zbasic->rndInt(5) + 5;
	this->var_i16_d0a = g_zbasic->rndInt(5) - 0xa;
	for (int16 i = 0; i <= 5; i++) {
		g_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kPutCopy);
		g_toolbox->OffsetRect(this->arr_rect_1910c, this->var_i16_d08, this->var_i16_d0a);
		this->sub_128_5a6c();
	}
	// 128:5274
	this->arr_rect_19114.top = this->arr_rect_1910c.top - 0xa;
	this->arr_rect_19114.left = this->arr_rect_1910c.left - 0xa;
	this->arr_rect_19114.bottom = this->arr_rect_1910c.bottom + 0xa;
	this->arr_rect_19114.right = this->arr_rect_1910c.right + 0xa;
	// 128:530c
	this->var_i16_d0c = 0;
	this->keyLastPressed = 0;
	while (((this->stateFlags & kStateReturn) == 0) && (this->var_i16_d0c == 0)) {
		this->getNextEvent(-1);
		if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_19114) != 0) {
			this->sub_128_55ac();
		}
		// 128:5348
		this->var_i16_d0e = this->var_ev_46.where.y;
		this->var_i16_d10 = this->var_ev_46.where.x;
		if (this->keyLastPressed > 0) {
			this->sub_128_57a2();
		}
		if (this->stateFlags == kStateSaveGame) {
			this->saveGame();
		}
	}
	// 128:5396
	if (this->var_i16_d0c != 0) {
		this->sub_128_6186();
		this->sub_128_50e(0x14, 0x64, 0);
		g_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kPutCopy);
		this->var_ev_46.where.x += 5;
		this->var_ev_46.where.y -= 5;
		if (this->var_i16_c04 < 0x64) {
			g_toolbox->SetCursor(this->arr_curs_4d88[0x10]);
		}
		for (int16 i = 1; i <= 1; i++) {
			// 128:541c
			this->arr_i16_4758[0] = this->var_ev_46.where.y;
			this->arr_i16_4758[1] = this->var_ev_46.where.x;
			this->arr_i16_4758[2] = this->var_ev_46.where.y;
			this->arr_i16_4758[3] = this->var_ev_46.where.x;
			for (int16 j = 1; j <= 0x19; j++) {
				// 128:5462
				this->arr_i16_4758[0]--;
				this->arr_i16_4758[1]--;
				this->arr_i16_4758[2]++;
				this->arr_i16_4758[3]++;
				Common::Rect temp;
				temp.top = this->arr_i16_4758[0];
				temp.left = this->arr_i16_4758[1];
				temp.bottom = this->arr_i16_4758[2];
				temp.right = this->arr_i16_4758[3];
				g_toolbox->InvertOval(temp);
			}
		}
		// 128:5514
		this->sub_128_3da(0x3c);
		if (this->var_i16_c04 < 0x64) {
			// 128:5526
			this->sub_128_6186();
			this->var_i16_c04 = 0x64;
			this->var_i16_7ce |= 2;
			this->sub_128_962(
				this->var_ev_46.where.y - 5,
				this->var_ev_46.where.x - 5,
				this->var_ev_46.where.y + 5,
				this->var_ev_46.where.x + 5,
				0x14,
				0,
				SCREEN_HEIGHT,
				SCREEN_WIDTH,
				2,
				kPatCopy,
				0x42
			);
			this->var_str_384 = g_zbasic->str(104); // behold the 1st key of thoth
			this->sub_128_178a(0, 0);
		}
		// 128:55aa
	}
	// 128:55aa
	return;
}

void FoolGame::sub_128_55ac() {
	// 128:55ac
	if ((this->var_i16_d10 == this->var_ev_46.where.x) && (this->var_i16_d0e == this->var_ev_46.where.y)) {
		return;
	}
	this->var_i16_d08 = g_zbasic->rndInt(5) + 5;
	this->var_i16_d0a = g_zbasic->rndInt(5) + 5;
	if (g_zbasic->maybe()) {
		this->var_i16_d08 *= -1;
	}
	// 128:560a
	if (g_zbasic->maybe()) {
		this->var_i16_d0a *= -1;
	}
	// 128:5620
	if (this->arr_rect_1910c.top < 0x50) {
		this->var_i16_d0a = 0xa;
	}
	// 128:5640
	if (this->arr_rect_1910c.left < 0x3c) {
		this->var_i16_d08 = 0xa;
	}
	if (this->arr_rect_1910c.bottom > 0x104) {
		this->var_i16_d0a = -0xa;
	}
	if (this->arr_rect_1910c.right > 0x1a4) {
		this->var_i16_d08 = -0xa;
	}
	// 128:56a0
	for (this->var_i16_68a = 0; this->var_i16_68a <= 0x5; this->var_i16_68a++) {
		g_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kPutCopy);
		g_toolbox->OffsetRect(this->arr_rect_1910c, this->var_i16_d08, this->var_i16_d0a);
		this->sub_128_5a6c();
	}
	// 128:5708
	this->arr_rect_19114.top = this->arr_rect_1910c.top - 0xa;
	this->arr_rect_19114.left = this->arr_rect_1910c.left - 0xa;
	this->arr_rect_19114.bottom = this->arr_rect_1910c.bottom + 0xa;
	this->arr_rect_19114.right = this->arr_rect_1910c.right + 0xa;

}

void FoolGame::sub_128_57a2() {
	// 128:57a2
	this->var_str_d12 = g_zbasic->ucase(g_zbasic->chr(this->keyLastPressed));
	if (g_zbasic->instr(1, g_zbasic->str(105), this->var_str_d12) > 0) { // NESW
		// 128:57e0
		g_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_bbbc, kPutCopy);
		this->var_i16_d08 = 0;
		this->var_i16_d0a = 0;
		if (this->var_str_d12 == g_zbasic->str(106)) { // N
			this->var_i16_d0a = -0x10;
		}
		if (this->var_str_d12 == g_zbasic->str(107)) { // S
			this->var_i16_d0a = 0x10;
		}
		if (this->var_str_d12 == g_zbasic->str(108)) { // W
			this->var_i16_d08 = -0x18;
		}
		if (this->var_str_d12 == g_zbasic->str(109)) { // E
			this->var_i16_d08 = 0x18;
		}
		g_toolbox->OffsetRect(this->arr_rect_1910c, this->var_i16_d08, this->var_i16_d0a);
		this->var_i16_d08 = 0;
		this->var_i16_d0a = 0;
		// 128:58c2
		if (this->arr_rect_1910c.top < 0x14) {
			this->arr_rect_1910c.top = 0x146;
			this->arr_rect_1910c.bottom = SCREEN_HEIGHT;
		}
		if (this->arr_rect_1910c.left < 0) {
			this->arr_rect_1910c.left = 0x1e8;
			this->arr_rect_1910c.right = SCREEN_WIDTH;
		}
		// 128:592e
		if (this->arr_rect_1910c.bottom > SCREEN_HEIGHT) {
			this->arr_rect_1910c.top = 0x14;
			this->arr_rect_1910c.bottom = 0x24;
		}
		if (this->arr_rect_1910c.right > SCREEN_WIDTH) {
			this->arr_rect_1910c.left = 0;
			this->arr_rect_1910c.right = 0x18;
		}
		this->sub_128_5a6c();
		if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1910c)) {
			this->var_i16_d0c = 1;
		}
	}
	// 128:59cc
	this->keyLastPressed = 0;
	this->arr_rect_19114.top = this->arr_rect_1910c.top - 0xa;
	this->arr_rect_19114.left = this->arr_rect_1910c.left - 0xa;
	this->arr_rect_19114.bottom = this->arr_rect_1910c.bottom + 0xa;
	this->arr_rect_19114.right = this->arr_rect_1910c.right + 0xa;

}

void FoolGame::sub_128_5a6c() {
	// 128:5a6c
	this->var_i32_692 = g_toolbox->TickCount();
	g_zbasic->get(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_rect_1910c.right, this->arr_rect_1910c.bottom, this->arr_bmp_bbbc);
	//warning("sub_128_5a6c: bbbc surface");
	//byte fakePal[768];
	//Common::fill(fakePal, fakePal+3, 0xff);
	//Common::fill(fakePal+3, fakePal+768, 0x00);
	//this->arr_bmp_bbbc->rawSurface().debugPrint(5, 0, 0,0, 0, -1, 160, fakePal);
	g_toolbox->EraseRoundRect(this->arr_rect_1910c, 8, 7);
	g_zbasic->put(this->arr_rect_1910c.left, this->arr_rect_1910c.top, this->arr_bmp_b3ec, kPutXOR);
	this->sub_128_406(0);
}

void FoolGame::sub_128_5b30() {
	// 128:5b30
	if (this->menuDisabled)
		return;
	this->var_ev_46.what = kNullEvent;
	if (this->menuHidesPlayfield) {
		this->thothHidePlayfield();
	}
	this->var_i32_bf8 = g_toolbox->MenuSelect(this->var_ev_46.where);
	this->selectedMenuID = (uint16)(this->var_i32_bf8 >> 16);
	this->selectedMenuItem = (uint16)(this->var_i32_bf8 & 0xffff);
	if (this->selectedMenuID > 0) {
		// 128:5b8c
		this->sub_128_5c20();
		g_toolbox->HiliteMenu(0);
	}
	// 128:5b94
	if (this->menuHidesPlayfield) {
		this->thothShowPlayfield();
	}
	this->sub_128_61ec();
}

void FoolGame::sub_128_5baa() {
	if (this->menuDisabled)
		return;
	this->var_i32_bf8 = g_toolbox->MenuKey((char)(this->var_ev_46.message & 0xff));
	g_toolbox->Delay(0);
	this->selectedMenuID = this->var_i32_bf8 >> 16;
	this->selectedMenuItem = this->var_i32_bf8 & 0xffff;
	if (this->selectedMenuID > 0) {
		if (this->menuHidesPlayfield) {
			this->thothHidePlayfield();
		}
		this->sub_128_5c20();
		if (this->menuHidesPlayfield) {
			this->thothShowPlayfield();
		}
		g_toolbox->HiliteMenu(0);
	}
	// 128:5c1a
	this->sub_128_61ec();
}

void FoolGame::sub_128_5c20() {
	this->stateFlags = 0;
	if (this->selectedMenuID == 1) { // Eye menu
		this->menuAbout();
	}
	if (this->selectedMenuID == 2) { // File menu
		if (this->selectedMenuItem == 1) {
			this->menuNewGame();
		} else if (this->selectedMenuItem == 2) {
		// 128:5c5c
			this->menuOpenGame();
		} else if (this->selectedMenuItem == 3) {
		// 128:5c6c
			this->menuSaveGame();
		} else if (this->selectedMenuItem == 4) {
			this->menuSaveGameAs();
		} else if (this->selectedMenuItem == 6) {
			if (this->soundEnabled == 0) {
				this->soundEnabled = 1;
			} else {
				this->soundEnabled = 0;
			}
			// 128:5cbc
			this->sub_128_32c8();
		// 128:5cc4
		} else if (this->selectedMenuItem == 7) {
			this->menuPrintStory();
		} else if (this->selectedMenuItem == 9) {
			this->menuQuit();
		}
	}
	// 128:5cea
	if ((this->selectedMenuID >= 3) && (this->selectedMenuID <= 7)) { // Chapter menus
		this->var_i16_e1a = 0;
		this->selectedMenuChapter = this->selectedMenuItem + (this->selectedMenuID - 3)*0x10;
		if ((this->selectedMenuChapter == 1) && (this->var_ev_46.what == kKeyDown)) {
			this->var_i16_e1a = 1;
		}
		// 128:5d56
		if ((this->var_ev_46.modifiers & kModLOptionKeyDown) != 0) {
			this->var_i16_e1a = 1;
		} else {
			// 128:5d74
			do {
				this->var_i16_7a8 = g_toolbox->GetNextEvent(1 << kKeyDown, this->var_ev_46);
				g_toolbox->GlobalToLocal(this->var_ev_46.where);
				if ((this->var_ev_46.modifiers & kModLOptionKeyDown) != 0) {
					this->var_i16_e1a = 1;
				}
			} while (this->var_ev_46.what != kNullEvent);
		}
		// 128:5dae
		if (((this->var_i16_7ce & 1) == 0) && (this->var_i16_e1a == 0) && (this->storyCurrentChapter != this->selectedMenuChapter)) {
			this->setStateBits(kStateChapterSelect);
		}
		// 128:5df6
		if (((this->var_i16_7ce & 1) == 0) && (this->var_i16_e1a == 1)) {
			this->setStateBits(kStateChapterSelect);

		}
		// 128:5e38
		if (((this->var_i16_7ce & 1) != 0) && (this->var_i16_e1a == 0)) {
			this->setStateBits(kStateChapterSelect | kStateReturn);
		}
		// 128:5e5e
		if (((this->var_i16_7ce & 1) == 0) && (this->var_i16_e1a == 1) && (this->storyCurrentChapter != this->selectedMenuChapter)) {
			this->setStateBits(kStateChapterSelect | kStateReturn);
			this->var_i16_7ce |= 0x4;
		}
	}
	// 128:5eaa
	if (this->selectedMenuID == 8) { // Puzzle menu
		if (this->selectedMenuItem == 1) {
			// return to scroll / run for your life
			this->setStateBits(kStateReturn);
		}
		// 128:5ec6
		if ((this->selectedMenuItem == 3) && (this->var_i16_c00 == 1)) {
			this->setStateBits(kStateUndo);
		}
	}
	// 128:5eee
}

void FoolGame::thothHidePlayfield() {
	// 128:5ef0
	this->copyScreen(0, this->arr_bmp_b3ec);
	g_zbasic->picture(0, 0x14, this->var_pic_7c2);
}

void FoolGame::thothShowPlayfield() {
	// 128:5f16
	if (this->stateFlags == kStateSaveGame) {
		if (this->arr_i16_1eb8[0] < 1) {
			this->arr_i16_1eb8[0] = 1;
		}
		// 128:5f4c
		this->activePuzzleBuffer = g_zbasic->encodeInt(this->arr_i16_1eb8[0]);
		this->saveGame();
	}
	// 128:5f70
	if ((this->stateFlags & 1) == 0) {
		this->sub_128_6186();
		this->copyScreen(1, this->arr_bmp_b3ec);
	}
}

void FoolGame::sub_128_5f9e() {
	// 128:5f9e
	this->keyLastPressed = this->var_ev_46.message & 0xff;
}

void FoolGame::sub_128_5fb4() {
	// 128:5fb4
	g_toolbox->BeginUpdate(*this->var_ev_46.windowPtr);
	g_toolbox->EndUpdate(*this->var_ev_46.windowPtr);
	if ((this->var_i16_7ce & 1) != 0) {
		this->setStateBits(0x200 | kStateReturn);
	} else {
		this->setStateBits(0x200);
	}
}

void FoolGame::sub_128_5fea() {
	if (this->var_i16_37a == 1) {
		g_toolbox->SetPort(this->var_i32_8);
		g_toolbox->PenNormal();
		this->fillRect(0x14, 0, this->var_i16_5c, this->var_i16_56-3, 2);
		this->fillRect(0x14, 0, this->var_i16_58+0x11, this->var_i16_5a, 2);
		this->fillRect(0x14, this->var_i16_56+0x203, this->var_i16_5c, this->var_i16_5a, 2);
		this->fillRect(this->var_i16_58 + 0x159, 0, this->var_i16_5c, this->var_i16_5a, 2);
		g_toolbox->SetRect(this->arr_rect_5b7c, this->var_i16_56-2, this->var_i16_58+0x12, this->var_i16_56+0x202, this->var_i16_58+0x158);
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		g_toolbox->FrameRect(this->arr_rect_5b7c);
		g_toolbox->SetRect(this->arr_rect_5b7c, this->var_i16_56-1, this->var_i16_58+0x13, this->var_i16_56+0x201, this->var_i16_58+0x157);
		g_toolbox->PenPat(this->arr_pat_58f4[2]);
		g_toolbox->FrameRect(this->arr_rect_5b7c);
		g_toolbox->_defaultMenu->setOverlayDirty(true);
		g_toolbox->SetPort(this->var_i32_0);
	}
	// 128:6152
}

void FoolGame::sub_128_6154() {
	// 128:6154
	this->var_i16_e1c = this->var_ev_46.message >> 16;
	this->var_i16_e1e = this->var_ev_46.message & 0xffff;
	if (this->var_i16_e1c != 0) {
		g_zbasic->unk_11(this->var_i16_e1e);
	}
}


void FoolGame::sub_128_6186() {
	// 128:6186
	// wait until mouse button is up
	do {
		// was originally a mask of 6
		this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
		g_toolbox->GlobalToLocal(this->var_ev_46.where);
		if (this->var_ev_46.what == kNullEvent)
			g_toolbox->Delay(0);
	} while ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0);
}

void FoolGame::sub_128_61c2() {
	// 128:61c2
	this->sub_128_6186();
	do {
		// was originally a mask of 2
		this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
		g_toolbox->GlobalToLocal(this->var_ev_46.where);
		if (this->var_ev_46.what == kNullEvent)
			g_toolbox->Delay(0);
	} while ((this->var_ev_46.what != kMouseDown));
	this->sub_128_6186();
}


void FoolGame::sub_128_61ec() {
	// 128:61ec
	do {
		this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
		if (this->var_ev_46.what == kUpdateEvt) {
			this->sub_128_5fb4();
		}
		if (this->var_ev_46.what == kDiskEvt) {
			this->sub_128_6154();
		}
		if (this->var_ev_46.what == kNullEvent)
			g_toolbox->Delay(0);
	} while (!((this->var_ev_46.what == kNullEvent) && (this->var_ev_46.modifiers & kModMouseButtonUp)));
	this->keyLastPressed = 0;
}

void FoolGame::sub_128_6244() {
	do {
		this->var_i16_7a8 = g_toolbox->GetNextEvent(-1, this->var_ev_46);
		if (this->var_ev_46.what == kUpdateEvt) {
			g_toolbox->BeginUpdate(*this->var_ev_46.windowPtr);
			g_toolbox->EndUpdate(*this->var_ev_46.windowPtr);
		}
		// 128:6272
		if (this->var_ev_46.what == kDiskEvt) {
			this->sub_128_6154();
		}
		g_toolbox->Delay(0);
	} while ((this->var_ev_46.what == kNullEvent) && ((this->var_ev_46.modifiers & kModMouseButtonUp) == 0));
	// SEGMENT_RETURN
}

// end
// 128:62ae

void FoolGame::sub_129_004() {
	this->var_i16_7e6 = 0;
	// 129:000a: LEA - 0x3ea(A5),A0
	// 129:000e: MOVE.L - A0,-0x8ee(A5)
	// 129:0012: SF - 0x8,D0
	g_zbasic->openR(1, g_zbasic->str(110), 0x3e8, this->var_i16_e20);
	if (this->var_i16_7e6 == 0) {
		this->var_str_e22 = g_zbasic->str(111);
		this->var_i16_f22 = this->var_i16_e20;
		g_zbasic->close(1);
		this->var_i16_7e6 = 0;
	}

	// 129:005e: LEA - 0x3f2(A5),A0
	// 129:0062: MOVE.L - A0,-0x8ee(A5)
}

void FoolGame::sub_129_068() {
	this->var_i16_7ce = 666;
	this->var_i16_372 = { 0, 0, 0 };
	this->soundEnabled = 1;
	this->var_i16_7a0 = 1;
	this->sub_128_4da(0);
	for (size_t i = 0; i <= 3; i++) {
		this->arr_pat_58f4[1].data[2*i] = 0xaa;
		this->arr_pat_58f4[1].data[2*i+1] = 0x55;
		this->arr_pat_58f4[2].data[2*i] = 0xff;
		this->arr_pat_58f4[2].data[2*i+1] = 0xff;
	}
	// 129:00d8
	// get current A5 - 0x72
	// 129:00d8: MOVE.L - 0x904,D0
	// 129:00de: JSR - "PEEKLONG"
	// 129:00e2: SUBI.L - 0x72,D0
	// 129:00e8: JSR - "PEEKWORD"
	this->var_i16_5a = g_engine->_wm._screenDims.width();  // window width?
	// 129:00f0: MOVE.L - 0x904,D0
	// 129:00f6: JSR - "PEEKLONG"
	// 129:00fa: SUBI.L - 0x74,D0
	// 129:0100: JSR - "PEEKWORD"
	this->var_i16_5c = g_engine->_wm._screenDims.height(); // window height?
	this->var_i16_56 = (this->var_i16_5a - SCREEN_WIDTH)/2;
	this->var_i16_58 = (this->var_i16_5c - SCREEN_HEIGHT)/2;
	// 129:0138
	if ((this->var_i16_56 == 0) && (this->var_i16_58 == 0)) {
		this->var_i16_37a = 0;
	} else {
		this->var_i16_37a = 1;
	}
	// 129:0166
	g_zbasic->window(1, g_zbasic->str(112), 0, 0, this->var_i16_5a, this->var_i16_5c, kWindowDialogOneLine);
	g_zbasic->coordinateWindow();
	g_toolbox->ClearMenuBar();
	g_toolbox->DrawMenuBar();

	// very cursed check for the ROM85 flag
	// 129:0196: MOVE.L - 0x28e,D0
	// 129:019c: JSR - "BCD_FROM_INT"
	// 129:01a0: JSR - "CAST_INT"
	// 129:01a4: JSR - "PEEKWORD"
	// 129:01a8: CMPI.L - 0x3fff,D0
	if (true) {
		this->var_i16_372 = { 0x4e20, 0x4e20, 0x4e20 };
		g_toolbox->SetCPixel(0x64, 0x64, this->var_i16_372);
		g_toolbox->GetCPixel(0x64, 0x64, this->var_i16_372);

	}

	if ((this->var_i16_5a != SCREEN_WIDTH) || (this->var_i16_5c != SCREEN_HEIGHT)) {
		this->fillRect(0, 0, this->var_i16_5c, this->var_i16_5a, 2);
	} else {
		// 129:0224
		this->fillRect(0, 0, this->var_i16_5c, this->var_i16_5a, 1);
	}
	// 129:023e
	g_toolbox->GetPort(this->var_i32_f24);

	this->var_i32_8 = &this->arr_grafport_19042;
	g_toolbox->OpenPort(this->var_i32_8);
	// this grafport is used for drawing on the menu bar area;
	// on normal hardware this isn't a problem, as the changes are
	// made directly to the screen framebuffer.
	// in ScummVM, the window and the menu bar are widgets, with surfaces
	// that are composed to the screen framebuffer.
	// as such, to avoid a clash, we need to draw onto the menu bar widget.
	this->var_i32_8->portBits = g_toolbox->_defaultMenuBits;
	this->var_i32_8->portRect = g_toolbox->_defaultMenuBits->getBounds();

	// Thoth, on the other hand, expects to be able to draw to the screen.
	this->var_i32_8_thoth = &this->arr_grafport_19042_thoth;
	g_toolbox->OpenPort(this->var_i32_8_thoth);
	g_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->MovePortTo(this->var_i16_56, this->var_i16_58);
	g_toolbox->ClipRect(this->arr_rect_1ae06);

	this->var_i32_4 = &this->arr_grafport_18f78;
	g_toolbox->OpenPort(this->var_i32_4);
	g_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->MovePortTo(this->var_i16_56, this->var_i16_58);
	g_toolbox->ClipRect(this->arr_rect_1ae06);

	this->var_i32_0 = &this->arr_grafport_18eae;
	g_toolbox->OpenPort(this->var_i32_0);
	g_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->MovePortTo(this->var_i16_56, this->var_i16_58);
	g_toolbox->ClipRect(this->arr_rect_1ae06);

	this->sub_128_5fea();
	// 129:02c8
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1);

	if (false) {
	//if (this->var_i16_372.red + this->var_i16_372.blue + this->var_i16_372.green != 0) {
		// we're in colour mode, chide the user
		this->arr_str_1a8d8[0] = g_zbasic->str(113);
		this->arr_str_1a8d8[1] = g_zbasic->str(114);
		this->arr_str_1a8d8[2] = g_zbasic->str(115);
		this->arr_str_1a8d8[3] = g_zbasic->str(116);
		this->sub_128_dfe(0, 2, 1, 0);
		g_zbasic->unk_4();
	}
	// 129:0390
	this->sub_128_61ec();
	this->var_i32_37c = g_zbasic->mem(-1);
	if (this->var_i32_37c < 0x1d4c0) {
		// not enough memory, chide the user
		this->arr_str_1a8d8[0] = g_zbasic->str(117);
		this->arr_str_1a8d8[1] = g_zbasic->str(118);
		this->arr_str_1a8d8[2] = g_zbasic->str(119);
		this->arr_str_1a8d8[3] = g_zbasic->str(120);
		this->arr_str_1a8d8[4] = g_zbasic->str(121);
		this->arr_str_1a8d8[5] = g_zbasic->str(122);
		this->arr_str_1a8d8[6] = g_zbasic->str(123);
		this->sub_128_dfe(0, 5, 1, 1);
		g_zbasic->unk_4();
	}
	// 129:0496
	this->var_i16_7e4 = 1;
	this->var_str_e22 = g_zbasic->str(124);
	this->var_str_8ec = g_zbasic->str(125);

	this->var_i16_484 = g_zbasic->finderInfo(this->var_i16_7e4, this->var_str_588, this->var_i32_f28, this->var_i16_688);

	if (this->var_str_588 != g_zbasic->str(126)) {
		if (this->var_str_588 == g_zbasic->str(127)) { // Fool's Puzzles
			this->var_str_e22 = g_zbasic->str(128);
			this->var_i16_f22 = this->var_i16_688;
		} else {
			// 129:052e
			this->sub_128_2808();
			if (this->var_str_588 != g_zbasic->str(129)) {
				// 129:054c
				this->var_str_8ec = this->var_str_588;
				this->var_i16_9ec = this->var_i16_688;
			}
		}
	}
	// 129:0562
	this->var_i16_f2c = this->sub_128_5fe(); // volume ref num
	this->var_i16_e20 = this->var_i16_f2c;
	if (this->var_str_e22 == g_zbasic->str(130)) {
		this->sub_129_004();
	}
	// 129:058c
	if (this->var_str_e22 == g_zbasic->str(131)) {
		for (this->var_i16_f2e = 1; this->var_i16_f2e < 0x3e8; this->var_i16_f2e++) {
			this->var_str_f30 = g_zbasic->files(-this->var_i16_f2e, g_zbasic->str(132), Common::U32String(), this->var_i16_e20);
			if ((this->var_i16_e20 == 0) || (this->var_str_f30 == g_zbasic->str(133))) {
				this->var_i16_f2e = 0x3e8;
			}
			// 129:0606
			this->sub_129_004();
			if (this->var_str_e22 != g_zbasic->str(134)) {
				this->var_i16_f2e = 0x3e8;
			}
		}
	}
	// 129:0636
	if (this->var_str_e22 == g_zbasic->str(135)) {
		do {
			g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
			this->fillRect(0x17, 0x92, 0x31, 0x16e, 2);
			this->var_i16_7a2 = 0x28;
			// prompt to locate the fool's puzzles file
			// 129:0690: CLR.W - -0x772(A5)
			this->sub_128_4da(1);
			Common::U32String PUZZ = g_zbasic->str(137);
			this->sub_128_1e4(PUZZ);
			// 129:06a6: CLR.W - -0x772(A5)
			this->fillRect(0x17, 0x92, 0x31, 0x16e, 1);
			this->sub_128_4da(0);
			if (this->var_str_588 == g_zbasic->str(138)) {
				g_zbasic->unk_4();
			}
			this->var_str_e22 = this->var_str_588;
			this->var_i16_f22 = this->var_i16_688;
		} while (this->var_str_e22 != g_zbasic->str(139));
	}
	// 129:0718
	this->sub_128_64c(this->var_i16_f22);
	this->var_i16_1030 = g_toolbox->OpenResFile(Common::Path(this->var_str_e22, ':'));
	g_toolbox->UseResFile(this->var_i16_1030);
	this->var_i16_484 = this->sub_128_64c(this->var_i16_f2c);
	for (int i = 0; i <= 1; i++) {
		// scroll background
		this->arr_i32_192c0[i] = g_toolbox->GetPicture(i + 0x54);
		PicHandle h = this->arr_i32_192c0[i];
		g_toolbox->DetachResource(h);
	}
	// 129:07a2
	g_zbasic->picture(0, 0x14, this->arr_i32_192c0[0]);
	g_zbasic->get(0x0, 0x59, SCREEN_WIDTH, 0xb4, this->arr_bmp_5dfc);
	g_zbasic->get(0x0, 0x14f, 0x7, SCREEN_HEIGHT, this->arr_bmp_b3ec);
	g_zbasic->get(0x1f9, 0x14f, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_bmp_109dc);
	// 129:0846
	// unfurl the scroll by blitting the lower half a bunch of times
	for (int i = 0; i <= 0x15; i++) {
		this->var_i16_484 = g_zbasic->readDataInt();
		g_zbasic->put(0, this->var_i16_484, this->arr_bmp_5dfc, kPutCopy);
		this->sub_128_3da(2);
	}
	// 129:0888
	g_zbasic->put(0, 0x14f, this->arr_bmp_b3ec, kPutCopy);
	g_zbasic->put(0x1f9, 0x14f, this->arr_bmp_109dc, kPutCopy);
	g_zbasic->picture(0x39, 0x29, this->arr_i32_192c0[1]);

	// 129:08ee
	for (int i = 0; i <= 1; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_192c0[i]);
	}

	// 129:091a
	this->var_i32_692 = g_toolbox->TickCount();
	this->var_i16_68a = 0x28;
	this->sub_129_123a();

	// load sun's map tiles
	//byte fakePal[768];
	//Common::fill(fakePal, fakePal+3, 0xff);
	//Common::fill(fakePal+3, fakePal+768, 0x00);

	for (int i = 1; i <= 0x53; i++) {
		this->arr_i32_1912c[i] = g_toolbox->GetPicture(i);
		//this->arr_i32_1912c[i]->getSurface()->debugPrint(0, 0, 0, 0, 0, -1, 160, fakePal);

		g_toolbox->DetachResource(this->arr_i32_1912c[i]);
	}
	// 129:097c
	this->var_i16_68a = 0x3c;
	this->sub_129_123a();

	// wadjet eye
	this->var_pic_7c2 = g_toolbox->GetPicture(0x56);
	g_toolbox->DetachResource(this->var_pic_7c2);
	metapuzzleWheelPic = g_toolbox->GetPicture(0xac);
	g_toolbox->DetachResource(metapuzzleWheelPic);
	// icon of a scroll
	this->var_i32_c = g_toolbox->GetIcon(0x101);
	g_toolbox->DetachResource(this->var_i32_c);
	// 129:09c2: SF - 0x8,D0
	g_zbasic->openR(1, g_zbasic->str(140), 1000, this->var_i16_f22);
	this->var_i32_1036 = g_zbasic->readFileDblInt(1);

	this->var_ptr_696 = 0;

	// read into pointer
	this->arr_bytes_5dfc = g_zbasic->readFile(1, this->var_i32_1036);
	this->var_bytes_696 = this->arr_bytes_5dfc;

	// 129:0a0a
	this->var_i16_68a = 0x50;
	this->sub_129_123a();
	for (int i = 1; i <= 0x64; i++) {
		this->puzzleDataOffsets[i] = this->puzzlesReadLong();
	}
	// 129:0a4e
	// quickdraw patterns
	// reworked slightly to fill the pattern buffer directly
	for (int i = 0; i <= 0x50; i++) {
		for (int j = 0; j <= 0x7; j++) {
			this->arr_pat_58f4[i].data[j] = (byte)this->puzzlesReadByte();
		}
	}
	// 129:0a8a
	for (int i = 1; i <= 0x2a; i++) {
		this->arr_curs_4d88[i] = Common::SharedPtr<Cursor>(new Cursor());
		for (int j = 0; j < 16; j++) {
			this->arr_curs_4d88[i]->data[j] = (uint16)this->puzzlesReadShort();
		}
		for (int j = 0; j < 16; j++) {
			this->arr_curs_4d88[i]->mask[j] = (uint16)this->puzzlesReadShort();
		}
		this->arr_curs_4d88[i]->mouse.y = this->puzzlesReadShort();
		this->arr_curs_4d88[i]->mouse.x = this->puzzlesReadShort();
		this->arr_curs_4d88[i]->render();
	}
	// 129:0ad4
	for (int i = 0; i <= 0xf; i++) {
		// bitLUT is 1 << i
		this->bitLUT[i] = this->puzzlesReadShort();
	}
	// 129:0b02
	for (int i = 1; i <= 0x51; i++) {
		this->arr_i16_4c7c[i] = this->puzzlesReadByte();
	}
	// 129:0b30
	for (int i = 1; i <= 0xc; i++) {
		for (int j = 0; j <= 3; j++) {
			this->arr_i16_4d20[i*4 + j] = this->puzzlesReadShort();
		}
	}
	// 129:0b78
	this->var_i16_103a = this->puzzlesReadShort();
	// story menu entries + endgame puzzles
	for (int i = 1; i <= this->var_i16_103a; i++) {
		this->puzzleType[i] = this->puzzlesReadShort();
		this->arr_i16_15e8[i] = this->puzzlesReadShort();
		this->puzzleUnlockChapter[i] = this->puzzlesReadShort();
		this->arr_str_195e8[i] = this->puzzlesReadString().decode(Common::kMacRoman);
	}
	// 129:0c0a
	this->var_i16_103a = this->puzzlesReadShort();
	for (int i = 1; i <= this->var_i16_103a; i++) {
		for (int j = 0; j <= 5; j++) {
			this->arr_i16_16b2[i*8+j] = this->puzzlesReadShort();
		}
	}
	// 129:0c5e
	this->storyPageCount = this->puzzlesReadShort();
	for (int i = 1; i <= this->storyPageCount; i++) {
		this->pageToChapter[i] = this->puzzlesReadShort();
	}
	// 129:0c98
	for (int i = 1; i <= this->storyPageCount; i++) {
		this->pageLineRanges[i*2] = this->puzzlesReadShort();
		this->pageLineRanges[i*2 + 1] = this->puzzlesReadShort();
	}
	// 129:0ce0
	this->var_i16_103a = this->puzzlesReadShort();
	for (int i = 1; i <= this->var_i16_103a; i++) {
		this->arr_i16_1b10[i*2] = this->puzzlesReadShort();
		this->arr_i16_1b10[i*2 + 1] = this->puzzlesReadShort();
	}
	// 129:0d3c
	// story text
	this->var_i16_103c = this->puzzlesReadShort();
	this->var_i16_68a = 1;
	Common::fill(this->pageLineFace, this->pageLineFace + 1301, 0);
	Common::fill(this->pageLineBreak, this->pageLineBreak + 1301, 0);
	for (int i = 1; i <= this->var_i16_103c; i++) {
		this->var_i16_103e = this->puzzlesReadByte();
		// indent at start of story paragraph
		if (this->var_i16_103e & 0x8) {
			this->var_i16_103e ^= 0x8;
			// 129:0d7e
			g_zbasic->indexRawSet(g_zbasic->space(4).encode(Common::kMacRoman) + this->puzzlesReadString(), 0, i);
		} else {
		// 129:0da4
			g_zbasic->indexRawSet(this->puzzlesReadString(), 0, i);
		}
		// 129:0db8
		// 0x4 == newline
		// 0x1 == normal formatting
		if ((this->var_i16_103e == 1) || (this->var_i16_103e == 4)) {
			this->pageLineFace[i] = Graphics::kMacFontRegular;
		}

		// 129:0dea
		// 0x2 = bold formatting
		if ((this->var_i16_103e == 2) || (this->var_i16_103e == 5)) {
			this->pageLineFace[i] = Graphics::kMacFontBold;
		}
		// 129:0e1e
		// 0x3 = outlined
		if ((this->var_i16_103e == 3) || (this->var_i16_103e == 6)) {
			this->pageLineFace[i] = Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontShadow;
		}
		// 129:0e52
		if (this->var_i16_103e > 3) {
			this->pageLineBreak[i] = 1;
		}
		// 129:0e6e
	}
	// 129:0e80
	this->var_i16_68a = 0x64;
	this->sub_129_123a();
	this->var_i16_103a = this->puzzlesReadShort();
	for (int i = this->var_i16_103c + 1; i <= this->var_i16_103a; i++) {
		g_zbasic->indexRawSet(this->puzzlesReadString(), 0, i);
	}
	// 129:0ec2
	for (int j = 1; j <= 0x50; j++) {
		this->var_i16_1040 = 0;
		for (int i = 1; i <= this->storyPageCount; i++) {
			if (this->pageToChapter[i] == j) {
				this->var_i16_1040++;
			}
			// 129:0ef4
			if (this->pageToChapter[i] > j) {
				i = this->storyPageCount;
			}
			// 129:0f16
		}
		if (this->var_i16_1040 > 1) {
			this->var_i16_484 = 0;
			for (int i = 1; i <= this->storyPageCount; i++) {
				if (this->pageToChapter[i] == j) {
					this->var_i16_484++;
					// - X of N
					this->arr_str_1a288[i] = Common::U32String::format("%s %d%s %d", g_zbasic->str(141).encode().c_str(), this->var_i16_484, g_zbasic->str(142).encode().c_str(), this->var_i16_1040);
				}
				// 129:0fb6
				if (this->pageToChapter[i] > j) {
					i = this->storyPageCount;
				}
				// 129:0fd8
			}
		}
		// 129:0fea
	}
	// 129:0ff8
	Common::fill(this->arr_i16_1dee, this->arr_i16_1dee + 202, 0);
	for (int j = 1; j <= this->storyPageCount; j++) {
		for (int i = this->pageLineRanges[j*2]; i <= this->pageLineRanges[j*2 + 1]; i++) {
			if (this->pageLineBreak[i] != 0) {
				// 129:102e
				this->arr_i16_1dee[j]++;
			}
			// 129:105a
		}
	// 129:107a
	}
	// 129:108c
	g_toolbox->UseResFile(this->var_i16_1030);
	this->sub_128_406(0xc8);
	this->var_i16_7ce = 0;
	this->var_i16_7b2 = 0xa;
	g_toolbox->InitCursor();
	this->sub_128_4da(1);
	// Apple menu
	g_zbasic->menu(1, 0, 1, g_zbasic->str(143)); // wadjet eye
	g_zbasic->menu(1, 1, 0, g_zbasic->str(144)); // sep
	g_zbasic->menu(1, 2, 1, g_zbasic->str(145)); // about fool's errand
	g_zbasic->menu(1, 3, 0, g_zbasic->str(146)); // sep

	// File menu
	g_zbasic->menu(2, 0, 1, g_zbasic->str(147)); // file
	g_zbasic->menu(2, 1, 1, g_zbasic->str(148)); // new
	g_zbasic->menu(2, 2, 1, g_zbasic->str(149)); // open
	g_zbasic->menu(2, 3, 1, g_zbasic->str(150)); // save
	g_zbasic->menu(2, 4, 1, g_zbasic->str(151)); // save as
	g_zbasic->menu(2, 5, 0, g_zbasic->str(152)); // sep
	g_zbasic->menu(2, 6, 1, g_zbasic->str(153)); // sound
	g_zbasic->menu(2, 7, 1, g_zbasic->str(154)); // print story
	g_zbasic->menu(2, 8, 0, g_zbasic->str(155)); // sep
	g_zbasic->menu(2, 9, 1, g_zbasic->str(156)); // quit

	this->sub_128_6244();
	// 129:11f6
	if (g_zbasic->str(157) == this->var_str_8ec) { // empty
		// cold start
		this->stateFlags = kStateNewGame;
	} else {
		// loading a save game
		this->stateFlags = kStateOpenGame;
		this->var_str_588 = this->var_str_8ec;
		this->var_i16_688 = this->var_i16_9ec;
	}
	// 129:1236
	// JMP - [0x19f6]
	// SEGMENT_RETURN
}

void FoolGame::sub_129_123a() {
	// draw the loading text on the menu bar
	g_toolbox->SetPort(this->var_i32_8);
	this->fillRect(0, 7, 0x13, this->var_i16_5a - 7, 0);
	g_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// Loading Game text during initial puzzle load
	this->var_str_172 = Common::U32String::format("%s %d%s", g_zbasic->str(158).encode().c_str(), this->var_i16_68a, g_zbasic->str(159).encode().c_str());
	this->var_i16_30 = g_toolbox->StringWidth(this->var_str_172);
	g_toolbox->MoveTo((this->var_i16_5a / 2) - (this->var_i16_30 / 2), 0xe);
	g_toolbox->DrawString(this->var_str_172);
	g_toolbox->_defaultMenu->setOverlayDirty(true);
	g_toolbox->SetPort(this->var_i32_0);
	g_toolbox->Delay(0);
}


// hermit key trace
void FoolGame::hermitRun() {
	// 142:12ac
	if (this->puzzleCompletionStatus[0x34] < 5) {
		g_zbasic->menu(8, 3, 1, g_zbasic->str(362)); // to find a hidden secret
		g_zbasic->menu(8, 4, 1, g_zbasic->str(363));
	} else {
		// 142:12fc
		g_zbasic->menu(8, 3, 1, g_zbasic->str(364)); // to find the straight path
		g_zbasic->menu(8, 4, 1, g_zbasic->str(365));
	}
	// 142:132c
	if (this->var_i16_c04 == 0) {
		this->hermitPathStage = 1;
	}
	if ((this->var_i16_c04 > 0) && (this->var_i16_c04 < 0x63)) {
		this->hermitPathStage = this->var_i16_c04;
	}
	// 142:1360
	if (this->var_i16_c04 == 0x63) {
		this->hermitPathStage = 6;
	}
	if (this->var_i16_c04 == 0x64) {
		this->hermitPathStage = 1;
	}
	if (this->var_i16_c04 > 0x64) {
		this->hermitPathStage = this->var_i16_c04 - 0x64;
	}
	this->thothKeyLast();
	if (this->hermitPathStage == 6) {
		this->var_i16_c04 = 0x63;
	} else {
		if (this->var_i16_c04 < 0x63) {
			// 142:13be
			this->var_i16_c04 = this->hermitPathStage;
			if (this->var_i16_d0c != 0) {
				this->var_i16_c04 = 0x64;
			}
		} else {
			// 142:13da
			if (this->var_i16_c04 >= 0x64) {
				this->var_i16_c04 = this->hermitPathStage + 0x64;
				if (this->var_i16_d0c != 0) {
					this->var_i16_c04 = 0x65;
				}
			}
		}
	}
	// 142:1404
}

void FoolGame::sub_144_004() {
	warning("STUB: %s", __func__);
}

};
