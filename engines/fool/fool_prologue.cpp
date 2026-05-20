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


#include "common/str.h"
#include "common/str-enc.h"
#include "graphics/macgui/macwindow.h"

#include "fool/fool.h"
#include "fool/fool_prologue.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

extern Toolbox *g_toolbox;

// Based on m68k disassembly of the Fool's Errand v2.0, (c) 1988 Cliff Johnson.

// v1.0 - original release, single-density disks
// v2.0 - fixes full-screen rendering on higher-resolution displays
// v3.0 - newer ZBasic, changed a few graphics assets, removed custom menu font and sounds for compatibility

void FoolPrologue::run() {
	_zbasic = new ZBasic(g_toolbox);
	_zbasic->loadProgram(Common::Path("Prologue - Finale"));
	this->sub_128_004(true);
	delete _zbasic;
}

void FoolPrologue::sub_128_004(bool finale) {
	var_i16_1aa = finale ? 2 : 1;

	// 128:0004
	_zbasic->coordinateWindow();
	g_toolbox->ClearMenuBar();
	g_toolbox->HideCursor();
	g_toolbox->UseResFile(g_toolbox->CurResFile());

	// 128:001e
	// b54 must be the 1-bit screen buffer
	// 41296 is an array of pointers to screen pages within the buffer
	for (int i = 0; i < 12; i++) {
		//this->arr_i32_41296[i] = &this->arr_i32_b54[SCREEN_PAGE_SIZE*i];
		this->arr_i32_41296[i] = BitMap(new Graphics::ManagedSurface());
		this->arr_i32_41296[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	// 128:0066
	this->var_i32_4e = this->arr_i32_41296[11];
	// 128:0078
	// pattern buffer
	this->arr_pat_194[0] = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }; // solid white
	this->arr_pat_194[1] = { { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 } }; // checkerboard
	this->arr_pat_194[2] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }; // solid black


	// 128:00d6
	this->arr_pat_194[3] = { { 0x01, 0x07, 0x0e, 0x0c, 0x7c, 0x70, 0xc0, 0xc0 } };
	this->arr_pat_194[4] = { { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 } };

	// 128:017a
	// about message
	this->var_str_76 = _zbasic->str(0);
	_zbasic->bufferFlush(this->var_str_76);
	this->var_str_76 = _zbasic->str(1);
	_zbasic->bufferFlush(this->var_str_76);
	this->var_str_76 = _zbasic->str(2);
	_zbasic->bufferFlush(this->var_str_76);

	// 128:01b6
	// JMP 0x1f0
	// JMP 0x21a
	// JMP 0x246
	// JMP 0x268
	// JMP 0x2a2
	// JMP 0x2ec
	// JMP 0x350
	// JMP 0x3ea
	// JMP 0x506
	// JMP 0x60c
	// JMP 0x646
	// JMP 0x6e0
	// JMP 0x7fc
	// JMP 0xa68
	// JMP 0xa88
	// JMP 0xc8a
	// 128:0c8a
	this->sub_129_004();
	if (_quit)
		return;
	this->var_i32_1a6 = _zbasic->mem(-1);
	if (this->var_i16_1aa == 1) {
		this->prologueRun();
		this->var_i32_1a6 = _zbasic->mem(-1);
	}
	// 128:0cb0
	if (this->var_i16_1aa == 2) {
		this->finaleRun();
		this->var_i32_1a6 = _zbasic->mem(-1);
	}
	// 128:0cc8
	//_zbasic->unk4(); // exit command?
}

void FoolPrologue::sub_128_1ba(int16 screenPage) {
	// 128:01ba
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	g_toolbox->CopyBits(this->var_i32_32, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
}

void FoolPrologue::setPortBitsToPage(int16 screenPage) {
	// 128:01f4
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	g_toolbox->SetPortBits(this->var_i32_40);
}

void FoolPrologue::delay(int16 numTicks) {
	// 128:021e
	// original code would poll TickCount in a loop,
	// effectively the same as Delay
	g_toolbox->Delay(numTicks);
}

void FoolPrologue::delayFromMarker(int16 numTicks) {
	// 128:024a
	uint32 delay = (uint32)MAX<int>(this->var_i32_2 + numTicks + 1 - g_toolbox->TickCount(), 0);
	// again, polling TickCount in a loop.
	g_toolbox->Delay(delay);
}

void FoolPrologue::drawTextRightAlign(int16 y, int16 x) {
	// 128:026c
	this->var_i32_182 = g_toolbox->StringWidth(this->var_str_76);
	g_toolbox->MoveTo(x - this->var_i32_182, y);
	g_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::drawTextCenterAlign(int16 y, int16 x) {
	// 128:02a6
	this->var_i32_182 = g_toolbox->StringWidth(this->var_str_76);
	g_toolbox->MoveTo(x - (this->var_i32_182 / 2), y);
	g_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID) {
	// 128:02f0
	g_toolbox->SetRect(this->arr_i16_41af4, left, top, right, bottom);
	g_toolbox->FillRect(this->arr_i16_41af4, this->arr_pat_194[patternID]);
}

void FoolPrologue::sub_128_354(PatternMode mode, uint16 unk2) {
	// 128:0354
	g_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->PenPat(this->arr_pat_194[unk2]);
	g_toolbox->PenMode(mode);
	g_toolbox->PenSize(6, 4);

	for (int i = 0; i <= 130; i += 3) {
		this->var_i32_2 = g_toolbox->TickCount();

		g_toolbox->FrameRect(this->arr_i16_1bc);
		g_toolbox->InsetRect(this->arr_i16_1bc, 6, 4);
		this->delayFromMarker(1);
	}
	g_toolbox->PenNormal();
}

void FoolPrologue::sub_128_3ee(int16 unk1) {
	// 128:03ee
	this->var_i16_18c = 0x5;
	if (this->var_i16_18e > 0x28) {
		this->var_i16_18c = 0x8;
	}
	if (this->var_i16_18e > 0x50) {
		this->var_i16_18c = 0xb;
	}
	if (this->var_i16_18e > 0x64) {
		this->var_i16_18c = 0xe;
	}
	// 128:0428
	for (int i = 1; i <= this->var_i16_18e + 1; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->MoveTo(
			unk1 - this->arr_i16_1e8[i],
			unk1 - this->arr_i16_1e8[i + 0x1f6]
		);
		g_toolbox->LineTo(
			unk1 + this->arr_i16_1e8[i + 0xfb],
			unk1 - this->arr_i16_1e8[i + 0x1f6]
		);
		if (i % this->var_i16_18c == 0) {
			this->delayFromMarker(0);
		};
	}

}

void FoolPrologue::sub_128_50a(int16 unk1, int16 unk2, int16 unk3, int16 screenPage) {
	// 128:050a
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	this->var_i16_5c.left = unk2;
	this->var_i16_5c.right = unk3;
	if (unk1 == 0) {
		unk1 = 0x14;
	}
	if (unk1 == 1) {
		unk1 = 0x8;
	}
	if (unk1 == 2) {
		unk1 = 0x6;
	}
	if (unk1 == 3) {
		unk1 = 0x5;
	}
	// 128:0584
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_5c.top = this->arr_i16_412ea[i];
		this->var_i16_5c.bottom = this->arr_i16_412ea[i] + 1;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_5c, this->var_i16_5c, kSrcCopy, nullptr);
		if (i % unk1 == 0) {
			this->delayFromMarker(0);
		}
	}
}

void FoolPrologue::blitPageToScreen(int16 screenPage) {
	// 128:0610
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
}

void FoolPrologue::scanlineTransition(int16 patternID) {
	// 128:064a
	g_toolbox->PenNormal();
	g_toolbox->PenPat(this->arr_pat_194[patternID]);
	// 128:0668
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->MoveTo(0, this->arr_i16_412ea[i]);
		g_toolbox->LineTo(SCREEN_WIDTH, this->arr_i16_412ea[i]);
		if (i % 5 == 0) {
			this->delayFromMarker(0);
		}
	}
	g_toolbox->PenNormal();
}

void FoolPrologue::zoomTransition(int16 screenPage) {
	// 128:06e4
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	for (int i = 1; i <= 0x36; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_5c.top = (SCREEN_HEIGHT/2) - (int)(i*3.33);
		this->var_i16_5c.left = (SCREEN_WIDTH/2) - i*5;
		this->var_i16_5c.bottom = (SCREEN_HEIGHT/2) + (int)(i*3.33);
		this->var_i16_5c.right = (SCREEN_WIDTH/2) + i*5;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_5c, this->var_i16_5c, kSrcCopy, nullptr);
		g_toolbox->Delay(0);
	}
	g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
	g_toolbox->Delay(0);
}

void FoolPrologue::sub_128_800(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4, int16_t unk5, int16_t unk6, int16_t unk7, int16_t unk8, int16_t unk9) {
	// 128:0800
	this->arr_i16_41af4.top = unk9;
	this->arr_i16_41af4.left = unk8;
	this->arr_i16_41af4.bottom = unk7;
	this->arr_i16_41af4.right = unk6;
	this->arr_i16_41afc.top = unk5;
	this->arr_i16_41afc.left = unk4;
	this->arr_i16_41afc.bottom = unk3;
	this->arr_i16_41afc.right = unk2;
	g_toolbox->PenNormal();
	g_toolbox->PenMode(kPatXor);

	// 128:08bc
	this->arr_f64_41bbe[0] = (double)this->arr_i16_41af4.top;
	this->arr_f64_41bbe[1] = (double)this->arr_i16_41af4.left;
	this->arr_f64_41bbe[2] = (double)this->arr_i16_41af4.bottom;
	this->arr_f64_41bbe[3] = (double)this->arr_i16_41af4.right;
	this->arr_f64_41bbe[4] = (double)(this->arr_i16_41af4.top - this->arr_i16_41afc.top)/(double)(this->var_i32_182);
	this->arr_f64_41bbe[5] = (double)(this->arr_i16_41af4.left - this->arr_i16_41afc.left)/(double)(this->var_i32_182);
	this->arr_f64_41bbe[6] = (double)(this->arr_i16_41af4.bottom - this->arr_i16_41afc.bottom)/(double)(this->var_i32_182);
	this->arr_f64_41bbe[7] = (double)(this->arr_i16_41af4.right - this->arr_i16_41afc.right)/(double)(this->var_i32_182);

	// 128:096e
	g_toolbox->PaintRect(this->arr_i16_41af4);
	for (int i = 1; i < unk1 + 1; i++) {
		this->var_i32_2 = g_toolbox->TickCount();

		for (int j = 0; j < 4; j++) {
			this->arr_f64_41bbe[j] = this->arr_f64_41bbe[j] + this->arr_f64_41bbe[j+4];
		}
		this->arr_i16_41b0a.top = (int16_t)this->arr_f64_41bbe[0];
		this->arr_i16_41b0a.left = (int16_t)this->arr_f64_41bbe[1];
		this->arr_i16_41b0a.bottom = (int16_t)this->arr_f64_41bbe[2];
		this->arr_i16_41b0a.right = (int16_t)this->arr_f64_41bbe[3];
		g_toolbox->PaintRect(this->arr_i16_41b0a);
		this->delayFromMarker(0);
	}
	g_toolbox->PaintRect(this->arr_i16_41afc);
	g_toolbox->PenNormal();
}

void FoolPrologue::drawText(int16_t y, int16_t x) {
	// 128:0a6c
	g_toolbox->MoveTo(x, y);
	g_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::drawRainRecycle(int16_t unk) {
	// 128:0a8c
	this->var_i16_1a4 = unk;
	g_toolbox->PenMode(kPatXor);
	do {
		// 128:0a96
		this->drawRainDrop();
		this->arr_i16_1e8[this->var_i16_6] =
		this->arr_i16_1e8[0x2f1+this->var_i16_6]
		+ this->arr_i16_1e8[this->var_i16_6];

		this->arr_i16_1e8[0xfb+this->var_i16_6] =
		this->arr_i16_1e8[0xfb+this->var_i16_6]
		+ this->arr_i16_1e8[0x2f1+this->var_i16_6];

		// 128:0b52
		if ((this->arr_i16_1e8[this->var_i16_6] > 0x1f4) || (this->arr_i16_1e8[this->var_i16_6 + 0xfb] > 0x140)) {

		    // 128:0bae
			this->arr_i16_1e8[this->var_i16_6] = _zbasic->rndInt(0x264) - 0xc8;
			this->arr_i16_1e8[0xfb + this->var_i16_6] = _zbasic->rndInt(0x1ba) - 0xc8;
			this->arr_i16_1e8[0x1f6 + this->var_i16_6] = _zbasic->rndInt(0x5) + 0x1;
			this->arr_i16_1e8[0x2f1 + this->var_i16_6] = _zbasic->rndInt(0xa) + 0x19;
		}
		// 128:0c56
		this->drawRainDrop();
		this->var_i16_6 += 1;
		if (this->var_i16_6 > 0xb5) {
			this->var_i16_6 = 0;
			g_toolbox->Delay(0);
		}
	} while (this->var_i32_2 + this->var_i16_1a4 > g_toolbox->TickCount());
	g_toolbox->Delay(0);
}

void FoolPrologue::sub_128_c8a() {

}

void FoolPrologue::shuffleScanlines() {
	_zbasic->unk_20();
	this->var_i32_1ac = (byte *)&this->arr_i16_41598[0];
	this->var_i32_1b0 = (byte *)&this->arr_i16_41846[0];
	this->var_i16_1b4 = this->var_i16_18e * 2;
	// 128:0d00
	for (int i=0; i < this->var_i16_18e; i++) {
		this->arr_i16_41598[i] = i;
	}
	// 128:0d2a
	for (int i=this->var_i16_18e - 1; i >= 0; i--) {
		this->var_i16_1b6 = _zbasic->rndInt(i + 1) - 1;
		this->arr_i16_412ea[i] = this->arr_i16_41598[this->var_i16_1b6];
		this->var_i16_1b8 = this->var_i16_1b6 * 2 + 2;
		_zbasic->blockMove(this->var_i32_1ac + this->var_i16_1b8, this->var_i32_1b0 + this->var_i16_1b8, this->var_i16_1b4 - this->var_i16_1b8);
		_zbasic->blockMove(this->var_i32_1b0 + this->var_i16_1b8, this->var_i32_1ac + this->var_i16_1b8 - 2, this->var_i16_1b4 - this->var_i16_1b8);
	}
}

void FoolPrologue::sub_128_de2() {
	// 128:0de2
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcXor);
	this->var_str_76 = _zbasic->str(3);
	this->drawText(0x151, 0x5);
}

void FoolPrologue::sub_128_e1c() {
	// 128:0e1c
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcXor);
	this->var_str_76 = _zbasic->str(4);
	this->drawTextRightAlign(0x154, 0x1fb);
}

void FoolPrologue::sub_128_e58() {
	// 128:0e58
	this->sub_128_e80();
	while (true) {
		// was: 2
		this->var_i16_1ba = g_toolbox->GetNextEvent(-1, this->var_ev_22);
		if (this->var_ev_22.what == kMouseDown)
			break;
		if ((this->var_ev_22.what == kScummVMQuitEvt) || (this->var_ev_22.what == kScummVMReturnToLauncherEvt)) {
			_quit = true;
			return;
		}
		// wait until next redraw
		if (this->var_ev_22.what == kNullEvent)
			g_toolbox->Delay(0);
	}
	this->sub_128_e80();
}

void FoolPrologue::sub_128_e80() {
	// 128:0e80
	while (true) {
		this->var_i16_1ba = (int)g_toolbox->GetNextEvent(-1, this->var_ev_22);
		if (this->var_ev_22.what == kUpdateEvt) {
			this->sub_128_ed2();
		}
		if (this->var_ev_22.what == kDiskEvt) {
			this->sub_128_ee0();
		}
		if (this->var_ev_22.what == kScummVMQuitEvt || this->var_ev_22.what == kScummVMReturnToLauncherEvt) {
			_quit = true;
			return;
		}
		// keep looping until mouse is seen as up??
		// see I-252
		if ((this->var_ev_22.modifiers & kModMouseButtonUp) && (this->var_ev_22.what == kNullEvent)) {
			break;
		}
		// wait until next redraw
		if (this->var_ev_22.what == kNullEvent)
			g_toolbox->Delay(0);
	}
}

void FoolPrologue::sub_128_ed2() {
	// 128:0ed2
	g_toolbox->BeginUpdate(this->var_window_24);
	g_toolbox->EndUpdate(this->var_window_24);
}

void FoolPrologue::sub_128_ee0() {
	// 128:0ee0
	// done by using PEEKWORD into fixed offsets of the window pointer
	//this->var_i16_1bc = &this->var_window_24; // +0
	//this->var_i16_1be = &this->var_window_24; // +2
	//_zbasic->unk11(this->var_i16_1be);
}

void FoolPrologue::sub_128_f0a() {
	// _zbasic->pushOldCodeResource(0x80); // switch CODE resource?
}

// Sources:
// QuickDraw types, Inside Macintosh I-201
// QuickDraw global list, Inside Macintosh I-162
// Memory organization: Inside Macintosh II-19

// QuickDraw globals; offset below the A5 address.
// GrafPtr thePort = 0x00
// Pattern white = 0x04
// Pattern black = 0x0c
// Pattern gray = 0x14
// Pattern ltGray = 0x1c
// Pattern dkGray = 0x24
// Arrow cursor = 0x2c
// - Bits16 cursor.data = 0x2c
// - Bits16 cursor.mask = 0x4c
// - Point cursor.hotSpot = 0x6c
// BitMap screenBits = 0x70
// - Ptr screenBits.baseAddr = 0x70
// - INTEGER screenBits.rowBytes = 0x74
// - Rect screenBits.bounds = 0x76
// LONGINT randSeed = 0x7e

void FoolPrologue::sub_129_004() {
	// get the screen width and height from QuickDraw
	// 0x0004: MOVE.L - 0x904,D0
	// 0x000a: JSR - "PEEKLONG"   # current A5
	// 0x000e: SUBI.L - 0x72,D0
	// 0x0014: JSR - "PEEKWORD" # quickdraw globals,
	this->var_i16_10 = g_engine->_wm._screenDims.width();  // window width?
	this->var_i16_12 = g_engine->_wm._screenDims.height(); // window height?

	// 129:0034
	// set left and top offsets based on a drawable area of 512x342
	this->var_i16_14 = (this->var_i16_10 - SCREEN_WIDTH)/2;
	this->var_i16_16 = (this->var_i16_12 - SCREEN_HEIGHT)/2;

	// 129:0064
	_zbasic->window(1, "", 0, 0, this->var_i16_10, this->var_i16_12, kWindowDialogOneLine);
	_zbasic->coordinateWindow();

	this->var_i16_1c = { 0, 0, 0 };

	// code checks Rom85 for presence of 128K ROM before doing next bit
	// 129:00a2
	if (false) {
		this->var_i16_1c = { 0x4e20, 0x4e20, 0x4e20 };
		g_toolbox->SetCPixel(0x64, 0x64, this->var_i16_1c);
		g_toolbox->GetCPixel(0x64, 0x64, this->var_i16_1c);
	}

	// 129:00ee
	// used for drawing on the menu bar
	this->var_i32_c = &this->arr_grafport_a8a;
	g_toolbox->OpenPort(this->var_i32_c);
	this->var_i32_c->portBits = g_toolbox->_defaultMenuBits;
	this->var_i32_c->portRect = g_toolbox->_defaultMenuBits->getBounds();

	this->var_i32_8 = &this->arr_grafport_9c0;
	g_toolbox->OpenPort(this->var_i32_8);

	if ((this->var_i16_10 != SCREEN_WIDTH || this->var_i16_12 != SCREEN_HEIGHT)) {
		// 129:013a
		this->fillRect(0x14, 0, this->var_i16_12, this->var_i16_10, 2);
		g_toolbox->SetRect(this->arr_i16_1bc, this->var_i16_14 - 2, this->var_i16_16 - 2, this->var_i16_14 + SCREEN_WIDTH + 2, this->var_i16_16 + SCREEN_HEIGHT + 2);
		g_toolbox->PenPat(this->arr_pat_194[1]);
		g_toolbox->FrameRect(this->arr_i16_1bc);
	}

	// 129:01b0
	g_toolbox->SetPort(this->var_i32_8);
	g_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->MovePortTo(this->var_i16_14, this->var_i16_16);
	g_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->ClipRect(this->arr_i16_1bc);

	// 129:01f2
	if (this->var_i16_12 == SCREEN_HEIGHT) {
		g_toolbox->SetRect(this->arr_i16_1bc, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_toolbox->FillRect(this->arr_i16_1bc, this->arr_pat_194[1]);
	} else {
		g_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		g_toolbox->FillRect(this->arr_i16_1bc, this->arr_pat_194[1]);
	}

	// 129:026c
	this->var_i32_32 = this->var_i32_8->portBits;
	this->var_i16_36 = 0x40;
	this->var_i16_38.top = -this->var_i16_16;
	this->var_i16_38.left = -this->var_i16_14;
	this->var_i16_38.bottom = this->var_i16_16 + SCREEN_HEIGHT;
	this->var_i16_38.right = this->var_i16_14 + SCREEN_WIDTH;
	this->var_i16_44 = 0x40;
	this->var_i16_46.top = 0x0;
	this->var_i16_46.left = 0x0;
	this->var_i16_46.bottom = SCREEN_HEIGHT;
	this->var_i16_46.right = SCREEN_WIDTH;
	this->var_i16_52 = 0x40;
	this->var_i16_54.top = 0x0;
	this->var_i16_54.left  = 0x0;
	this->var_i16_54.bottom = SCREEN_HEIGHT;
	this->var_i16_54.right = SCREEN_WIDTH;

	this->var_i32_1c0 = _zbasic->mem(-1);

	// 129:02fe
	if (this->var_i32_1c0 < 0xd6d8) {
		g_toolbox->InitCursor();
		_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_i32_41296[0], true);
		_zbasic->sound(0x19, 0x64, 0xff, 0x0);
		this->fillRect(0x46, 0x64, 0xdc, 0x19c, 2);
		this->fillRect(0x4b, 0x69, 0xd7, 0x197, 1);

		// 129:0386
		this->fillRect(0x50, 0x6e, 0xd2, 0x192, 2);
		_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
		// "not enough memory" message
		this->var_str_76 = _zbasic->str(6);
		this->drawTextCenterAlign(0x64, 0x100);
		this->var_str_76 = _zbasic->str(7);
		this->drawTextCenterAlign(0x78, 0x100);
		this->var_str_76 = _zbasic->str(8);
		this->drawTextCenterAlign(0x96, 0x100);
		this->var_str_76 = _zbasic->str(9);
		this->drawTextCenterAlign(0xaa, 0x100);
		this->var_str_76 = _zbasic->str(10);
		this->drawTextCenterAlign(0xcb, 0x100);
		this->sub_128_e58();
		if (_quit)
			return;

		_zbasic->put(0x0, 0x14, this->arr_i32_41296[0], kSrcCopy);
		this->var_i16_1aa = 0;

	} else {
		// 129:04a0
		this->var_i16_180 = 1;
		// this next blob of code is for determining whether to
		// open the prologue or the finale.
		// both are in the same EXE, however the finale is accessed
		// by opening a file with the right associated application code
		// that contains the words "And now it's time to show the finale of the Fool's Errand" and a sequence of numbers.
		// we don't need to worry about files as we add a start trigger
		// in the game's menu.
		/*
		this->var_i16_1aa = 1;
		// 180 == number, possibly file number
		// 1c4 == string
		// 2c4 == unused
		// 2c8 == volume number?
		this->var_i16_176 = _zbasic->finderInfo(this->var_i16_180, this->var_i16_1c4, this->var_i32_2c4, this->var_i16_2c8);
		if (Common::U32String("") != this->var_i16_1c4) {
			// 129:04de
			// "and now it is time to show the finale" message
			this->var_str_76 = _zbasic->str(12);
			_zbasic->bufferFlush(this->var_str_76);

			for (int i = 1; i <= 0xa; i++) {
				this->var_str_76 += _zbasic->chr(i);
				_zbasic->bufferFlush(this->var_str_76);
			}

			// 129:0522
			this->var_i16_2ca = this->var_str_76.size();
			this->var_i16_2cc = 0;
			// 129:0534
			// what is this???
			// 0x0534: LEA - [0x0764],A0
			// 0x0538: MOVE.L - A0,-0x8ee(A5)
			// 0x053c: SF - 0x8,D0
			_zbasic->openR(1, this->var_i16_1c4, 0x400, this->var_i16_2c8);

			Handle payload = _zbasic->readFile(1, this->var_i16_2ca);
			this->var_i16_2ce = Common::U32String((char *)payload->data(), this->var_i16_2ca, Common::kMacRoman);
			_zbasic->close(1);
			// 0x0570: CLR.L - -0x8ee(A5)
			// 129:0582
			if ((this->var_i16_2ce == this->var_str_76) &&
				this->var_i16_2cc == 0) {
				this->var_i16_1aa = 2;
			} else {
				this->var_i16_1aa = 0;
			}
		}
		*/
		// 129:05a6
		if (this->var_i16_1c.red + this->var_i16_1c.blue + this->var_i16_1c.green != 0) {
			g_toolbox->InitCursor();
			_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_i32_41296[0], true);
			_zbasic->sound(0x19, 0x64, 0xff, 0x00);
			this->fillRect(0x64, 0x82, 0xdc, 0x17e, 2);
			this->fillRect(0x69, 0x87, 0xd7, 0x179, 1);
			this->fillRect(0x6e, 0x8c, 0xd2, 0x174, 2);
			// 129:0662
			_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
			// "set your monitor to black and white" message"
			this->var_str_76 = _zbasic->str(13);
			this->drawTextCenterAlign(0x82, 0x100);
			this->var_str_76 = _zbasic->str(14);
			this->drawTextCenterAlign(0x96, 0x100);

			this->var_str_76 = _zbasic->str(15);
			// if this is the finale
			if (this->var_i16_1aa == 2) {
				this->var_str_76 = _zbasic->str(16);
			}
			this->drawTextCenterAlign(0xaa, 0x100);

			this->var_str_76 = _zbasic->str(17);
			this->drawTextCenterAlign(0xcb, 0xfc);
			// 129:0730
			this->sub_128_e58();
			if (_quit)
				return;
			_zbasic->put(0x0, 0x14, this->arr_i32_41296[0], kSrcCopy);
			this->var_i16_1aa = 0;
		} else {
			// 129:075c
			this->sub_128_e80();
		}
		// 129:0772
		// _zbasic->pushOldCodeResource(0x81);
		this->prologueRun();

	}
}

void FoolPrologue::sub_129_764() {
	// 129:0764
	this->var_i16_2cc = 1;
	_zbasic->unk_130(0);
}

void FoolPrologue::sub_129_772() {
	// _zbasic->pushOldCodeResource(0x81);
}

void FoolPrologue::prologueRun() {
	// 130:0004
	this->var_i16_3ce = 5;
	this->prologueDrawLoadingMsg();
	this->var_i16_18e = SCREEN_HEIGHT;
	// fill a buffer with random screen y-positions, to use for the scanline transition
	this->shuffleScanlines();

	// Because of floppy disk read times, the game pre-loads as many PICT resources
	// as it can stuff into memory. The below process happens a bunch of times:
	// - Increment an ID by 1
	// - Call GetPicture to fetch the next PICT resource for that ID (prologueBufferNextPicture)
	// - Increment the loading progress % by 5 (prologueDrawLoadingMsg)
	// - Blit it to an offscreen page with ZBasic's PICTURE command
	// - Release the resource

	// "The Fool's Errand" title card
	this->glob_i32_2ce = g_toolbox->GetPicture(0x47);
	this->var_i16_3ce = 10;
	this->prologueDrawLoadingMsg();
	this->setPortBitsToPage(0xb);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x74, 0xaa, this->glob_i32_2ce);
	g_toolbox->ReleaseResource(this->glob_i32_2ce);

	// 130:007a
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcBic);
	this->var_str_76 = _zbasic->str(18);
	this->drawTextCenterAlign(0x10d, 0x103);
	g_toolbox->SetPortBits(this->var_i32_32);
	// cliffside images with lightning
	for (int i = 1; i <= 5; i++) {
		this->prologueBufferNextPicture();
	}
	this->setPortBitsToPage(7);

	// 130:00d6
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x96, this->arr_i32_0[1]);
	g_toolbox->ReleaseResource(this->arr_i32_0[1]);
	this->setPortBitsToPage(8);

	// 130:011c
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x96, this->arr_i32_0[2]);
	g_toolbox->ReleaseResource(this->arr_i32_0[2]);

	// 130:015c
	for (int i = 3; i <= 5; i++) {
		//_zbasic->blockMove(this->arr_i32_41296[7], this->arr_i32_41296[i], 0x5580);
		this->arr_i32_41296[i]->copyFrom(*this->arr_i32_41296[7]);
	}
	// high priestess skull
	this->prologueBufferNextPicture();
	this->setPortBitsToPage(0);

	// 130:01aa
	_zbasic->picture(0, 0, this->arr_i32_0[6]);
	g_toolbox->ReleaseResource(this->arr_i32_0[6]);
	// high priestess face
	this->prologueBufferNextPicture();

	//_zbasic->blockMove(this->arr_i32_41296[0], this->arr_i32_41296[1], 0x5580);
	this->arr_i32_41296[1]->copyFrom(*this->arr_i32_41296[0]);
	this->setPortBitsToPage(1);

	// 130:0202
	_zbasic->picture(0xa2, 0x3c, this->arr_i32_0[7]);
	g_toolbox->ReleaseResource(this->arr_i32_0[7]);
	// priestess on mountain
	this->prologueBufferNextPicture();
	this->setPortBitsToPage(2);

	// 130:0234
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x5a, 0x2e, this->arr_i32_0[8]);
	g_toolbox->ReleaseResource(this->arr_i32_0[8]);

	// 130:0276
	for (int j = 9; j <= 0xb; j++) {
		// priestess shooting lightning
		this->prologueBufferNextPicture();

		if (j == 9) {
			this->var_i16_74 = 0;
		} else {
			this->var_i16_74 = 0x81;
		}

		// 130:029a
		this->setPortBitsToPage(j - 6);
		_zbasic->picture(0, 0, this->arr_i32_0[j]);
		_zbasic->picture(this->var_i16_74, 0x96, this->arr_i32_0[j - 6]);
		g_toolbox->ReleaseResource(this->arr_i32_0[j]);
		g_toolbox->ReleaseResource(this->arr_i32_0[j - 6]);
	}
	// priestess lightning closeup
	this->prologueBufferNextPicture();

	// 130:0320
	this->setPortBitsToPage(6);
	_zbasic->picture(0, 0, this->arr_i32_0[0xc]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0xc]);

	// 130:034a
	for (int i = 0xd; i <= 0x12; i++) {
		// priestess silhouette -> fool on the cliff.
		this->prologueBufferNextPicture();
	}

	// fool looking up at sun
	this->arr_i32_0[0x48] = g_toolbox->GetPicture(0x48);
	g_toolbox->SetPortBits(this->var_i32_32);

	// 130:0380
	if ((this->var_i16_10 == SCREEN_WIDTH) && (this->var_i16_12 == SCREEN_HEIGHT)) {
		this->fillRect(0, 0, 0x14, SCREEN_WIDTH, 1);
	} else {
		g_toolbox->SetPort(this->var_i32_c);
		this->fillRect(0, 0, 0x14, this->var_i16_10, 2);
		g_toolbox->_defaultMenu->setOverlayDirty(true);
		g_toolbox->SetPort(this->var_i32_8);
	}

	// We're done loading, start the intro.
	// Erase the screen with a bunch of cool scanlines.

	// 130:03e4
	this->delay(0x3c);
	this->scanlineTransition(0x2);
	this->delay(0x1e);

	// draw logo
	this->blitPageToScreen(0xb);
	this->delay(0x5a);

	// animate rain
	this->drawRain();

	g_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->SetRect(this->arr_i32_1c4, 0, 0x96, SCREEN_WIDTH, 0x118);

	// 130:043c
	for (int i = 1; i <= 2; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->drawRainRecycle(0xb4);
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_i16_1bc);
		this->delayFromMarker(1);
		g_toolbox->InvertRect(this->arr_i16_1bc);
		// 130:04ca
		for (int j = 0; j <= 3; j++) {
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[8];
			// 130:049e
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(4);
			this->var_i16_192 = 0;
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[3];
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(5);
			// 130:0518
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[4];
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(5);
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[5];
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(5);
		}
		// 130:05b6
		this->var_i32_40 = this->arr_i32_41296[7];
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);

	}

	// 130:05fc
	this->blitPageToScreen(0);
	this->drawRain();
	this->var_i32_2 = g_toolbox->TickCount();
	this->drawRainRecycle(0xb4);
	this->var_i32_40 = this->arr_i32_41296[1];

	// scanline fade in priestess face

	// 130:062c
	int y = 0;
	while (y != SCREEN_HEIGHT) {
		this->var_i16_5c.right = SCREEN_WIDTH;
		this->var_i32_2 = g_toolbox->TickCount();
		for (int j = 1; j < 0x7; j++) {
			this->var_i16_192 = 1;
			if (y < SCREEN_HEIGHT) {
				y += 1;
			}
			this->var_i16_5c.top = this->arr_i16_412ea[y];
			this->var_i16_5c.bottom = this->arr_i16_412ea[y] + 1;
			// 130:068e
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_5c, this->var_i16_5c, kSrcCopy, nullptr);
		}
		// 130:06b0
		this->drawRainRecycle(1);
	}

	// lightning flash over priestess face

	this->var_i32_2 = g_toolbox->TickCount();
	this->drawRainRecycle(0xa);
	this->blitPageToScreen(1);
	// 130:06d4
	for (int i = 1; i <= 4; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_i16_1bc);
		this->drawRainRecycle(0x2);
	}
	// 130:0704
	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->InvertRect(this->arr_i16_1bc);
	this->drawRainRecycle(0xf);

	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->InvertRect(this->arr_i16_1bc);
	this->drawRainRecycle(0x78);

	// cut to shot of cliff
	this->blitPageToScreen(2);

	// 130:0740
	this->drawRain();
	this->var_i32_2 = g_toolbox->TickCount();
	this->drawRainRecycle(0xb4);

	g_toolbox->SetRect(this->arr_i32_1c4, 0, 0, 0x150, 0x96);

	// priestess zap
	// 130:0778
	for (int j = 0; j <= 8; j++) {
		for (int i = 3; i <= 5; i++) {
			this->var_i32_2 = g_toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[i];
			g_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(3);
		}
	}

	// cut to closeup of priestess mid-zap
	// 130:07ec
	this->blitPageToScreen(6);
	g_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int i = 0; i <= 0x2d; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		// seizure warning
		g_toolbox->InvertRect(this->arr_i16_1bc);
		this->delayFromMarker(1);
	}

	// 130:083c

	// Draw the priestess silhouette
	_zbasic->picture(0, 0, this->arr_i32_0[0xd]);

	// Image two in this sequence is a PICT that draws the words
	// "...for no-one can undo the treachery I have inflicted upon the land!"
	// using the ShortLine opcode to fill in random rows of pixels.
	// Because the lines are drawn in white, and the default background colour
	// is white, this is an example of a PICT that needs to be drawn to the
	// target buffer live, or with a mask.
	// On original (i.e. slow) hardware, this image takes a while to render to the
	// screen with the line tool and gives a cool fade-in effect, which we
	// simulate here.
	this->arr_i32_0[0xe]->_opsPerTick = 32;
	_zbasic->picture(5, 0, this->arr_i32_0[0xe]);

	g_toolbox->ReleaseResource(this->arr_i32_0[0xd]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0xe]);

	// start loading some new resources from the floppy disk into the screen buffers
	this->var_i32_2 = g_toolbox->TickCount();
	// 130:0890
	// _zbasic->52(0);
	this->setPortBitsToPage(0);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0x17, this->arr_i32_0[0xf]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0xf]);
	// 130:08dc
	this->prologueRenderNextText();
	this->sub_128_de2();
	this->setPortBitsToPage(1);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0xf0, 0xba, this->arr_i32_0[0x10]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0x10]);
	this->prologueRenderNextText();
	this->sub_128_de2();
	// 130:0934
	for (int i = 2; i <= 5; i++) {
		//_zbasic->blockMove(this->arr_i32_41296[1], this->arr_i32_41296[i], 0x5580);
		this->arr_i32_41296[i]->copyFrom(*this->arr_i32_41296[1]);
	}
	// 130:0978
	// draw the sun pulsing to pages 1-5
	for (int i = 1; i <= 5; i++) {
		this->setPortBitsToPage(i);
		g_toolbox->PenPat(this->arr_pat_194[1]);
		g_toolbox->SetRect(this->arr_i16_1bc, 0x3c, 0x3c, 0x8c, 0x8c);
		g_toolbox->FrameOval(this->arr_i16_1bc);
		g_toolbox->PenPat(this->arr_pat_194[4]);
		// 130:09d0
		g_toolbox->InsetRect(this->arr_i16_1bc, -i, -i);
		for (int j = 2; j <= 0xd; j++) {
			g_toolbox->InsetRect(this->arr_i16_1bc, -j, -j);
			g_toolbox->FrameOval(this->arr_i16_1bc);
		}
	}
	// 130:0a42
	this->setPortBitsToPage(6);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x166, 0x77, this->arr_i32_0[0x11]);
	this->sub_128_de2();
	//_zbasic->blockMove(this->arr_i32_41296[6], this->arr_i32_41296[7], 0x5580);
	this->arr_i32_41296[7]->copyFrom(*this->arr_i32_41296[6]);
	this->prologueRenderNextText();
	g_toolbox->ReleaseResource(this->arr_i32_0[0x11]);
	this->setPortBitsToPage(7);
	// 130:0ac2
	_zbasic->picture(-0x14, -0x1e, this->arr_i32_0[0x12]);
	this->prologueRenderNextText();
	g_toolbox->ReleaseResource(this->arr_i32_0[0x12]);
	this->setPortBitsToPage(8);
	//_zbasic->blockMove(this->arr_i32_41296[7], this->arr_i32_41296[8], 0x5580);
	this->arr_i32_41296[8]->copyFrom(*this->arr_i32_41296[7]);
	this->fillRect(0x73, 0x163, 0xfa, 0x1e0, 0);
	_zbasic->picture(0x16d, 0x80, this->arr_i32_0[0x48]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0x48]);
	g_toolbox->SetPortBits(this->var_i32_32);

	// 130:0b74
	this->delayFromMarker(0xd2);
	this->sub_128_e80();
	if (_quit)
		return;
	this->scanlineTransition(1);

	// rectangle zoom into the clifftop scene

	this->delay(0x14);
	this->zoomTransition(0);

	// stall until mouse click
	this->sub_128_e58();
	if (_quit)
		return;
	// 130:0b98
	this->var_i16_74 = 0;
	this->var_i16_192 = 1;
	while (this->var_ev_22.what != kMouseDown) {
		// 130:0ba4
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_74 += this->var_i16_192;
		// 130:0bbe
		if (this->var_i16_74 == 0x5) {
			this->var_i16_192 = -1;
		}
		if (this->var_i16_74 == 1) {
			this->var_i16_192 = 1;
		}
		this->blitPageToScreen(this->var_i16_74);
		this->delayFromMarker(0xa);
		// 130:0bee
		this->var_i16_1ba = g_toolbox->GetNextEvent(2, this->var_ev_22);
	}
	while (!((this->var_ev_22.what == kNullEvent) && (this->var_ev_22.modifiers & kModMouseButtonUp))) {
		// 130:0c0c
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_74 += this->var_i16_192;
		// 130:0c26
		if (this->var_i16_74 == 0x5) {
			this->var_i16_192 = -1;
		}
		if (this->var_i16_74 == 1) {
			this->var_i16_192 = 1;
		}
		this->blitPageToScreen(this->var_i16_74);
		this->delayFromMarker(0xa);
		// 130:0c56
		this->var_i16_1ba = g_toolbox->GetNextEvent(-1, this->var_ev_22);
		if (this->var_ev_22.what == kUpdateEvt) {
			this->sub_128_ed2();
		}
		if (this->var_ev_22.what == kDiskEvt) {
			this->sub_128_ee0();
		}
	}
	// 130:0ca8
	// "who dares to interrupt my errand"
	this->blitPageToScreen(0x6);
	this->sub_128_e58();
	if (_quit)
		return;

	this->sub_128_50a(0x0, 0, SCREEN_HEIGHT, 7);
	this->delay(0xa);
	// "I dare"
	this->blitPageToScreen(0x8);
	this->sub_128_e58();
	if (_quit)
		return;

	// zoom to close

	this->sub_128_354(kPatCopy, 1);
	// 130:0ce6
	// JMP 1002
	// _zbasic->pushOldCodeResource(0x82);
	//this->finaleRun();
}

void FoolPrologue::prologueBufferNextPicture() {
	// 130:0cea
	this->var_i16_3d4 += 1;
	this->arr_i32_0[this->var_i16_3d4] = g_toolbox->GetPicture(this->var_i16_3d4);
	// 130:0d0a
	this->var_i16_3ce = this->var_i16_3d4 * 5 + 10;
	this->prologueDrawLoadingMsg();
}

void FoolPrologue::prologueDrawLoadingMsg() {
	// 130:0d28
	g_toolbox->SetPort(this->var_i32_c);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// "loading prologue" message
	this->var_str_76 = _zbasic->str(19);
	this->var_str_76 += Common::U32String::format(" %d", this->var_i16_3ce);
	this->var_str_76 += _zbasic->str(20);
	// 130:0d70
	// white out the contents of the top menu bar.
	// the 7px offset on either side seems to be to avoid hitting the rounded screen corners?
	this->fillRect(0, 0x7, 0x13, this->var_i16_10 - 7, 0);
	this->drawTextCenterAlign(0xe, this->var_i16_10/2);
	g_toolbox->_defaultMenu->setOverlayDirty(true);
	g_toolbox->SetPort(this->var_i32_8);
	g_toolbox->Delay(0);
}

void FoolPrologue::drawRain() {
	// 130:0db0
	_zbasic->unk_20();
	g_toolbox->PenMode(kPatXor);
	this->var_i16_6 = 0x1;
	while (this->var_i16_6 <= 0xb5) {
		// 130:0dc0
		this->arr_i16_1e8[this->var_i16_6] = _zbasic->rndInt(0x264) - 0x64;
		this->arr_i16_1e8[this->var_i16_6 + 0xfb] = _zbasic->rndInt(0x1ba) - 0x64;
		this->arr_i16_1e8[this->var_i16_6 + 0x1f6] = _zbasic->rndInt(0x5) + 1;
		this->arr_i16_1e8[this->var_i16_6 + 0x2f1] = _zbasic->rndInt(0xa) + 0x19;
		// 130:0e68
		this->drawRainDrop();
		this->var_i16_6 += 1;
	}
	this->var_i16_6 = 1;
	g_toolbox->Delay(0);
}

void FoolPrologue::drawRainDrop() {
	// 130:0e82
	g_toolbox->MoveTo(this->arr_i16_1e8[this->var_i16_6], this->arr_i16_1e8[this->var_i16_6 + 0xfb]);
	// 130:0ec0
	g_toolbox->LineTo(
		this->arr_i16_1e8[this->var_i16_6] + this->arr_i16_1e8[this->var_i16_6 + 0x1f6],
		this->arr_i16_1e8[this->var_i16_6 + 0xfb] + this->arr_i16_1e8[this->var_i16_6 + 0x1f6]
	);

}

void FoolPrologue::prologueRenderNextText() {
	// 130:0f48
	this->var_i16_3d2 = _zbasic->readDataInt();
	for (int i = 1; i <= this->var_i16_3d2; i++) {
		// 130:0f56
		this->var_i16_176 = _zbasic->readDataInt();
		this->var_i16_180 = _zbasic->readDataInt();
		this->var_str_76 = _zbasic->readDataStr();
		this->var_i16_1ba = 1;
		// JMP 0xfc2

		while (this->var_i16_1ba > 0) {
			// 130:0f78
			this->var_i16_1ba = _zbasic->instr(this->var_i16_1ba, this->var_str_76, _zbasic->str(21));
			if (this->var_i16_1ba > 0) {
				this->var_str_76.replace(this->var_i16_1ba-1, 1, Common::U32String("\""));
			}
		}
		// 130:0fc8
		_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
		this->drawText(this->var_i16_180, this->var_i16_176);
	}
}


// run finale
void FoolPrologue::finaleRun() {
	// 131:0004
	this->var_i16_3ce = 0x2d;
	this->sub_131_4dc0();
	this->var_i32_3d6 = g_toolbox->OpenPoly();
	g_toolbox->MoveTo(0xc8, 0x8d);
	g_toolbox->LineTo(0x1df, 0xc8);
	g_toolbox->LineTo(0x1d2, 0x11b);
	g_toolbox->LineTo(0xcd, 0x93);
	g_toolbox->LineTo(0xc8, 0x8d);
	g_toolbox->ClosePoly();
	// 131:004c
	this->arr_i32_0[0x1f] = g_toolbox->GetPicture(0x1f);
	this->setPortBitsToPage(0x4);
	_zbasic->picture(0x0, 0x0, this->arr_i32_0[0x1f]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0x1f]);
	this->sub_131_4dc0();
	for (int i = 0x14; i < 0x1e; i++) {
		// 131:009c
		this->arr_i32_0[i] = g_toolbox->GetPicture(i);
		if ((i % 3) == 0) {
			this->var_i16_3ce += 5;
			this->sub_131_4dc0();
		}
	}
	// 131:00e6
	for (int i = 6; i <= 9; i++) {
		this->setPortBitsToPage(i);
		this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
		// 131:0110
		if (i < 8) {
			_zbasic->picture(0, 0x3, this->arr_i32_0[0x19]);
		}
		if (i == 8) {
			_zbasic->picture(0, 0, this->arr_i32_0[0x1a]);
		}
		if (i == 9) {
			_zbasic->picture(0x60, 0x4c, this->arr_i32_0[0x1b]);
		}
	}
	// 131:0184
	for (int i = 0x19; i <= 0x1b; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}
	// 131:01aa
	this->arr_i32_0[0x13] = g_toolbox->GetPicture(0x13);
	this->setPortBitsToPage(0x5);
	_zbasic->picture(0, 0xb4, this->arr_i32_0[0x13]);
	_zbasic->picture(0, 0, this->arr_i32_0[0x18]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0x13]);
	g_toolbox->ReleaseResource(this->arr_i32_0[0x18]);
	this->sub_131_4e98();
	this->var_i16_3ce = 0x4b;
	this->sub_131_4dc0();
	// 131:021e
	for (int i = 0x20; i <= 0x2f; i++) {
		this->arr_i32_0[i] = g_toolbox->GetPicture(i);
		if (i % 3) {
			this->var_i16_3ce += 5;
			this->sub_131_4dc0();
		}
	}
	// 131:026e
	this->var_i16_18e = SCREEN_HEIGHT;
	this->shuffleScanlines();
	this->var_i32_1a6 = _zbasic->mem(-1);
	g_toolbox->SetPortBits(this->var_i32_32);
	if (!((this->var_i16_10 == SCREEN_WIDTH) && (this->var_i16_12 == SCREEN_HEIGHT))) {
		// 131:02ac
		this->fillRect(0, 0, 0x14, SCREEN_WIDTH, 1);
	} else {
		g_toolbox->SetPort(this->var_i32_c);
		this->fillRect(0, 0, 0x14, this->var_i16_10, 2);
		g_toolbox->_defaultMenu->setOverlayDirty(true);
		g_toolbox->SetPort(this->var_i32_8);
	}
	// 131:02ec
	this->delay(0x3c);
	this->zoomTransition(0x5);
	this->sub_128_e58();
	if (_quit)
		return;

	this->sub_131_4e98();
	// _zbasic->52(0x4c);
	for (int j = 1; j < 0xe; j++) {
		this->var_i16_3da = 1;

		// 131:030c
		this->var_i16_18e = _zbasic->readDataInt();
		for (int i = 1; i < this->var_i16_18e; i++) {
			// 131:031a
			this->arr_i16_1e8[i] = _zbasic->readDataInt();
			this->arr_i16_1e8[i + 0xfb] = _zbasic->readDataInt();
			this->arr_i16_1e8[i + 0x1f6] = _zbasic->readDataInt();
		}

		// 131:0380
		g_toolbox->PenMode(kPatXor);
		this->sub_128_3ee(0x14);
		this->sub_128_3ee(0xa);
		this->sub_128_3ee(0x14);
		this->sub_128_3ee(0x5);
		this->sub_128_3ee(0xa);
		this->sub_128_3ee(0x5);
		g_toolbox->PenNormal();
		this->sub_128_3ee(0x0);
	}

	// 131:03c0
	this->delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(22));
	_zbasic->bufferFlush(this->var_str_76);

	// 131:0400
	this->drawText(0x17c, 0xb4);
	this->delay(0x1e);

	// 131:041a
	_zbasic->picture(0x1cd, 0xbd, this->arr_i32_0[0x14]);
	this->delay(0x2);
	_zbasic->picture(0x1cd, 0xbf, this->arr_i32_0[0x15]);
	this->delay(0x3c);
	_zbasic->picture(0x1cd, 0xbd, this->arr_i32_0[0x14]);
	this->delay(0x6);
	_zbasic->picture(0x1a6, 0xbf, this->arr_i32_0[0x16]);
	this->delay(0x6);
	_zbasic->picture(0x1b3, 0xc5, this->arr_i32_0[0x17]);

	// 131:04b4
	for (int i = 0x14; i < 0x17; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	// 131:04da
	this->var_i32_2 = g_toolbox->TickCount();

	this->var_i32_40 = this->arr_i32_41296[5];
	g_toolbox->CopyBits(this->var_i32_32, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	this->sub_131_4e48();

	// 131:0538
	g_toolbox->CopyBits(this->var_i32_32, this->var_i32_4e, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
	g_toolbox->SetPortBits(this->var_i32_4e);

	// 131:0554
	this->fillRect(0, 0, 0xa0, 0x15e, 0);
	this->fillRect(0xa0, 0x5a, 0xd2, 0xaa, 0);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);

	// 131:05b0
	this->setPortBitsToPage(6);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);
	this->setPortBitsToPage(7);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);
	this->setPortBitsToPage(8);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);
	this->setPortBitsToPage(9);
	g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);

	// 131:0630
	g_toolbox->SetPortBits(this->var_i32_32);
	_zbasic->get(0x1bc, 0xc1, 0x1f2, 0x11b, this->arr_i32_3bca4);
	this->delayFromMarker(0x3c);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	this->sub_128_50a(0x6, 0, 0x200, 0x3);
	this->sub_131_4e48();

	// 131:06aa
	this->sub_128_50a(5, 0, 0x200, 2);
	this->sub_128_50a(7, 0, 0x200, 2);
	this->sub_128_50a(8, 0, 0x200, 1);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	this->sub_128_50a(0x9, 0, 0x200, 1);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(kPatXor);
	_zbasic->unk_20();

	// 131:0730
	this->var_i16_3dc = 0;
	this->var_i16_18e = 0;
	for (int j = 1; j < 0x63; j++) {
		if (this->var_i16_3dc > SCREEN_HEIGHT) {
			// 131:074e
			this->var_i16_3dc += _zbasic->maybe() ? 0x4 : 0x5;
		}
		if (this->var_i16_18e > 0x41) {
			// 131:0770
			this->var_i16_18e += 1;
			this->arr_i16_1e8[this->var_i16_18e] = _zbasic->rndInt(32);
			this->arr_i16_1e8[this->var_i16_18e+0xfb] = _zbasic->rndInt(this->var_i16_3dc);
			this->arr_i16_1e8[this->var_i16_18e+0x1f6] = _zbasic->rndInt(0xa) + 0xa;
			this->sub_131_4f96();
		}
		// 131:07ea
		for (int i = 1; i < this->var_i16_18e; i++) {
			this->sub_131_4f96();
			this->arr_i16_1e8[i] = ((this->arr_i16_1e8[i] + this->arr_i16_1e8[0x1f6+i]) + this->arr_i16_1e8[0x1f6+i]);

			// 131:0870
			if (this->arr_i16_1e8[i] < SCREEN_WIDTH) {
				this->arr_i16_1e8[i] = 1;
				this->arr_i16_1e8[0xfb+i] = _zbasic->rndInt(this->var_i16_3dc);
				this->arr_i16_1e8[0x1f6+i] = _zbasic->rndInt(0xa) + 0xa;
			}

			// 131:0902
			this->sub_131_4f96();
		}
	}


	// 131:0926
	g_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->PenPat(this->arr_pat_194[3]);
	_zbasic->picture(0x6d, 0x49, this->arr_i32_0[0x1c]);
	_zbasic->picture(0x65, 0x48, this->arr_i32_0[0x1d]);
	_zbasic->picture(0x64, 0x54, this->arr_i32_0[0x1e]);
	// 131:099c

	g_toolbox->PaintPoly(this->var_i32_3d6);
	for (int i = 0; i < 0x21; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->InvertRect(this->arr_i16_1bc);
		this->delayFromMarker(1);
	}

	// 131:09d2
	this->sub_128_800(0xc1, 0x1bc, 0x11b, 0x1f2, 0x0, 0x0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	this->sub_128_800(0x156, 0, 0x156, 0, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	this->sub_128_800(0, 0, 0, 0, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	this->sub_128_800(0, SCREEN_WIDTH, 0, SCREEN_WIDTH, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);

	// 131:0a7a
	for (int i = 1; i < 0x63; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_74 = _zbasic->rndInt(0x1ca);
		this->var_i16_192 = _zbasic->rndInt(0xfc);
		_zbasic->put(this->var_i16_74, this->var_i16_192, this->arr_i32_3bca4, kSrcCopy);
		g_toolbox->InvertRect(this->arr_i16_1bc);
		this->delayFromMarker(1);
	}

	// 131:0aec
	this->var_i32_2 = g_toolbox->TickCount();

	// 131:0af6
	while (g_toolbox->TickCount() < (this->var_i32_2 + 0x78)) {
		this->var_i16_74 = _zbasic->rndInt(0x1ca);
		this->var_i16_192 = _zbasic->rndInt(0xfc);
		_zbasic->get(this->var_i16_74, this->var_i16_192, this->var_i16_74 + 0x36, this->var_i16_192 + 0x5a, this->arr_i32_3bca4);

		// 131:0b54
		this->var_i16_74 = _zbasic->rndInt(0x1ca);
		this->var_i16_192 = _zbasic->rndInt(0xfc);
		_zbasic->put(this->var_i16_74, this->var_i16_192, this->arr_i32_3bca4, kSrcCopy);
	}

	// 131:0bb0
	this->sub_128_50a(4, 0, SCREEN_WIDTH, 3);
	this->sub_128_1ba(5);
	_zbasic->picture(0x198, 0x102, this->arr_i32_0[0x25]);
	g_toolbox->SetPortBits(this->var_i32_32);
	this->delay(0x3c);
	this->sub_128_50a(5, 0, SCREEN_WIDTH, 0);
	_zbasic->picture(0x197, 0x101, this->arr_i32_0[0x26]);

	// 131:0c36
	this->var_i32_2 = g_toolbox->TickCount();
	g_toolbox->KillPoly(this->var_i32_3d6);
	for (int i = 0x1c; i < 0x1e; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	// 131:0c6c
	for (int i = 0x6; i < 0xa; i++) {
		//_zbasic->blockMove(*this->arr_i32_41296[5], this->arr_i32_41296[i], 0x5580);
		this->arr_i32_41296[i]->copyFrom(*this->arr_i32_41296[5]);
	}
	this->setPortBitsToPage(0x6);
	_zbasic->picture(0x6, 0x8f, this->arr_i32_0[0x20]);
	this->setPortBitsToPage(0x7);
	_zbasic->picture(0x1a, 0x7e, this->arr_i32_0[0x21]);
	this->setPortBitsToPage(0x8);
	_zbasic->picture(0x2e, 0x6a, this->arr_i32_0[0x22]);
	this->setPortBitsToPage(0x9);
	_zbasic->picture(0x45, 0x4e, this->arr_i32_0[0x23]);
	this->setPortBitsToPage(0xa);
	_zbasic->picture(0x73, 0x28, this->arr_i32_0[0x24]);

	// 131:0d64
	for (int i = 0x20; i < 0x23; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}
	g_toolbox->SetPortBits(this->var_i32_32);
	this->delayFromMarker(0x64);
	_zbasic->picture(0x198, 0x102, this->arr_i32_0[0x25]);
	for (int i = 0x6; i < 0xa; i++) {
		this->delay(0xf);
		this->sub_128_50a(i, 0, SCREEN_WIDTH, 1);
	}
	_zbasic->picture(0x191, 0x102, this->arr_i32_0[0x27]);
	this->delay(0x3c);

	// 131:0e08
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(23));
	_zbasic->bufferFlush(this->var_str_76);
	this->drawText(0x1f4 - g_toolbox->StringWidth(this->var_str_76), 0xd2);
	this->var_i32_2 = g_toolbox->TickCount();

	// 131:0e76
	//_zbasic->blockMove(this->arr_i32_41296[4], this->arr_i32_41296[5], 0x5580);
	this->arr_i32_41296[5]->copyFrom(*this->arr_i32_41296[4]);
	this->setPortBitsToPage(0x5);
	_zbasic->picture(0x73, 0x28, this->arr_i32_0[0x24]);
	_zbasic->picture(0x191, 0x102, this->arr_i32_0[0x27]);
	_zbasic->picture(0x8a, 0x24, this->arr_i32_0[0x28]);
	this->setPortBitsToPage(0x6);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x118, 0x19, this->arr_i32_0[0x29]);
	this->setPortBitsToPage(0x7);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x118, 0x19, this->arr_i32_0[0x2a]);
	//_zbasic->blockMove(this->arr_i32_41296[5], this->arr_i32_41296[8], 0x5580);
	this->arr_i32_41296[8]->copyFrom(*this->arr_i32_41296[5]);
	this->setPortBitsToPage(0x8);
	_zbasic->picture(0x18f, 0xc0, -1, 0, this->arr_i32_0[0x2b]);
	//_zbasic->blockMove(this->arr_i32_41296[4], this->arr_i32_41296[9], 0x5580);
	this->arr_i32_41296[9]->copyFrom(*this->arr_i32_41296[4]);
	this->setPortBitsToPage(0x9);
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);
	this->setPortBitsToPage(0xa);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);

	// 131:1056
	g_toolbox->ReleaseResource(this->arr_i32_0[0x24]);
	g_toolbox->SetPortBits(this->var_i32_32);
	this->delayFromMarker(0x78);
	this->fillRect(0xc8, 0x145, 0xd7, SCREEN_WIDTH, 2);
	this->sub_128_50a(0x5, 0x64, 0x19c, 0x0);
	this->delay(0x28);

	this->var_i16_5c.left = 0x118;
	this->var_i16_5c.right = 0x1d1;
	this->var_i16_5c.top = 0x19;
	this->var_i16_64.left = 0x118;
	this->var_i16_64.right = 0x1d1;
	this->var_i16_64.bottom = 0xdf;

	for (int i = 0xdc; i > 0x19; i -= 5) {
		// 131:10de
		this->var_i32_2 = g_toolbox->TickCount();
		//_zbasic->blockMove(this->arr_i32_41296[5], *this->var_i32_4e, 0x5580);
		this->var_i32_4e->copyFrom(*this->arr_i32_41296[5]);
		this->var_i16_64.top = i;
		this->var_i16_5c.bottom = 0x19 + (i - 0xdf);
		this->var_i32_40 = this->arr_i32_41296[6];
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i16_5c, this->var_i16_64, kSrcBic, nullptr);
		this->var_i32_40 = this->arr_i32_41296[7];
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i16_5c, this->var_i16_64, kSrcOr, nullptr);
		this->delayFromMarker(0x2);
		g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_32, this->var_i16_64, this->var_i16_64, kSrcCopy, nullptr);
	}

	// 131:11a4
	this->sub_128_1ba(0x5);
	this->setPortBitsToPage(0x5);
	this->fillRect(0xc8, 0x145, 0xd7, SCREEN_WIDTH, 2);
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);
	g_toolbox->SetPortBits(this->var_i32_32);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(kPatXor);
	g_toolbox->PenSize(0x5, 0x5);

	// 131:1208
	for (int i = 0; i < 0x1b8; i += 6) {
		this->var_i32_2 = g_toolbox->TickCount();
		g_toolbox->MoveTo(0x172, 0x33);
		g_toolbox->LineTo(i, 0x14a);
		if (i == 0x43) {
			_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
			this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(24).encode().c_str());
			_zbasic->bufferFlush(this->var_str_76);
			this->drawTextRightAlign(0x1f4, 0xd2);
			_zbasic->picture(0x198, 0x102, this->arr_i32_0[0x25]);
		}
		this->delayFromMarker(0x2);
		g_toolbox->MoveTo(0x172, 0x33);
		g_toolbox->LineTo(i, 0x14a);
	}

	// 131:12cc
	this->sub_128_800(0x14a, 0x1b8, 0x14a, 0x1b8, 0xc0, 0x18f, 0x156, 0x1e6, 0x10);
	this->sub_128_50a(0x5, 0x12c, SCREEN_WIDTH, 0x1);
	this->delay(0xec);
	for (int i = 8; i < 0xa; i++) {
		this->sub_128_50a(i, 0, SCREEN_WIDTH, 1);
	}

	this->var_i32_2 = g_toolbox->TickCount();
	this->setPortBitsToPage(0x5);

	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0, this->arr_i32_0[0x2f]);
	g_toolbox->SetPortBits(this->var_i32_32);
	this->delayFromMarker(0xa);
	this->var_i16_5c.left = 0x18b;
	this->var_i16_5c.right = 0x1ef;
	this->var_i16_64.left = 0x18b;
	this->var_i16_64.right = 0x1ef;

	// 131:13b8
	for (int i = 0xc0; i > 0x15; i--) {
		this->var_i32_2 = g_toolbox->TickCount();
		this->var_i16_5c.top = i;
		this->var_i16_5c.bottom = this->var_i16_5c.top + 0x96;
		this->var_i16_64.top = i - 1;
		this->var_i16_64.bottom = this->var_i16_64.top + 0x96;
		g_toolbox->CopyBits(this->var_i32_32, this->var_i32_32, this->var_i16_5c, this->var_i16_64, kSrcCopy, nullptr);
		this->delayFromMarker(0x1);
	}

	// 131:1424
	_zbasic->picture(0x193, 0x2d, this->arr_i32_0[0x2c]);
	this->delay(0x1);
	_zbasic->picture(0x193, 0x2d, this->arr_i32_0[0x2d]);
	this->delay(0x1);
	_zbasic->picture(0x193, 0x2d, this->arr_i32_0[0x2e]);
	this->sub_128_1ba(0x6);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(kPatXor);
	g_toolbox->PenSize(0x3, 0x3);
	this->var_i16_6c.left = 0x190;
	this->var_i16_6c.right = SCREEN_WIDTH;
	this->var_i16_6c.top = 0x50;
	this->var_i16_6c.bottom = SCREEN_HEIGHT;
	this->var_i16_5c.left = 0;
	this->var_i16_5c.right = 0x32;
	this->var_i16_5c.top = 0;
	this->var_i16_5c.bottom = 0x32;
	this->var_i16_3e0 = 0x64;

	// 131:14d8
	this->var_i32_40 = this->arr_i32_41296[5];
	g_toolbox->SetPortBits(this->var_i32_4e);
	for (int i = 1; i < 0xf; i++) {
		this->var_i32_2 = g_toolbox->TickCount();
		//_zbasic->blockMove(this->arr_i32_41296[6], *this->var_i32_4e, 0x5580);
		this->var_i32_4e->copyFrom(*this->arr_i32_41296[6]);
		this->var_i16_3e0 += 5 + i*2;
		this->var_i16_64.left = 0x1e0;
		this->var_i16_64.right = this->var_i16_64.left + 0x32;
		this->var_i16_64.top = this->var_i16_3e0;
		this->var_i16_64.bottom = this->var_i16_64.top + 0x32;
		g_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i16_5c, this->var_i16_64, kSrcXor, nullptr);
		// 131:1582
		g_toolbox->MoveTo(0x190 - i, this->var_i16_3e0 - 0x1e + i*3);
		g_toolbox->LineTo(0x1f4, this->var_i16_3e0 + 0xa - i);
		this->delayFromMarker(0x1);
		g_toolbox->CopyBits(this->var_i32_4e, this->var_i32_32, this->var_i16_6c, this->var_i16_6c, kSrcCopy, nullptr);
	}

	g_toolbox->SetPortBits(this->var_i32_32);
	g_toolbox->PenNormal();
	g_toolbox->PenMode(kPatXor);
	for (int j = 0; j < 1; j++) {

		for (int i = 1; i < 0xfa; i++) {
			this->var_i32_2 = g_toolbox->TickCount();

			if (j == 0) {
				this->arr_i16_1e8[i] =  0x1f4 - i + _zbasic->rndInt(i);
				this->arr_i16_1e8[i+0xfb] =  _zbasic->rndInt(i)- SCREEN_HEIGHT;
			}
			// 131:1692
			g_toolbox->MoveTo(this->arr_i16_1e8[i], this->arr_i16_1e8[i+0xfb]);
			g_toolbox->LineTo(this->arr_i16_1e8[i], this->arr_i16_1e8[i+0xfb]);
			if (i % 0x19 == 0) {
				this->delayFromMarker(0x1);
			}
		}
	}
	// 131:1748
	this->var_i32_2 = g_toolbox->TickCount();
	for (int i = 0x25; i < 0x2f; i++) {
		g_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	this->delayFromMarker(0x3c);
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(25).c_str());
	_zbasic->bufferFlush(this->var_str_76);
	this->drawTextRightAlign(0x181, 0x3c);

	// 131:17c8
	var_i32_2 = g_toolbox->TickCount();
	for (int16 i = 0x30; i <= 0x31; i++) {
		arr_i32_0[i] = g_toolbox->GetPicture(i);
	}
	for (int16 i = 0x4b; i <= 0x52; i++) {
		arr_i32_0[i] = g_toolbox->GetPicture(i);
	}
	delayFromMarker(0xd2);
	fillRect(0, 0, 0x96, 0x18b, 2);
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x30]);
	delay(1);
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x31]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcBic);
	var_str_76 = _zbasic->chr(0x22) + _zbasic->str(26); // high priestess!
	drawTextRightAlign(0x181, 0x3c);
	var_str_76 = _zbasic->str(27); // am I to suffer this fate merely for uncovering
	drawTextRightAlign(0x181, 0x4d);
	var_str_76 = _zbasic->chr(0x22) + _zbasic->str(28); // the fourteen lost treasures of the land?
	drawTextRightAlign(0x181, 0x5e);
	for (int16 i = 0x30; i <= 0x31; i++) {
		g_toolbox->ReleaseResource(arr_i32_0[i]);
	}
	for (int16 i = 0x32; i <= 0x38; i++) {
		arr_i32_0[i] = g_toolbox->GetPicture(i);
		if (i == 0x37) {
			setPortBitsToPage(5);
			_zbasic->picture(0, 0, arr_i32_0[0x37]);
			g_toolbox->ReleaseResource(arr_i32_0[0x37]);
			g_toolbox->SetPortBits(var_i32_32);
		}
	}
	// 131:19be
	sub_128_e1c();
	sub_128_e58();
	sub_128_e1c();
	fillRect(0, 0, 0x96, 0x18b, 2);
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x32]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcBic);
	var_str_76 = _zbasic->chr(0x22) + _zbasic->str(29); // for if that be true
	drawTextRightAlign(0x181, 0x3c);
	var_str_76 = _zbasic->str(30); // I wish you had made the challenge more difficult
	drawTextRightAlign(0x181, 0x4d);
	var_str_76 = _zbasic->str(31); // after all, I am just a fool and I should not
	drawTextRightAlign(0x181, 0x5e);
	var_str_76 = _zbasic->str(32) + _zbasic->chr(0x22); // have been able to find them so easily
	drawTextRightAlign(0x181, 0x6f);
	arr_i16_41af4.top = 6;
	arr_i16_41af4.left = 0xd0;
	arr_i16_41af4.bottom = 0x6c;
	arr_i16_41af4.right = 0x133;
	arr_i16_41afc.top = 0xa;
	arr_i16_41afc.left = 0xb4;
	arr_i16_41afc.bottom = 0x1fc;
	arr_i16_41afc.right = 0x1f6;
	arr_i16_41b04[0] = 2;
	arr_i16_41b04[1] = 0xa;
	arr_i16_41b04[2] = 0xd;

	// 131:1b6e
	// unroll loops
	arr_f64_41bbe[0] = (float)arr_i16_41af4.top;
	arr_f64_41bbe[0+4] = (float)(arr_i16_41afc.top - arr_i16_41af4.top) / arr_i16_41b04[2];
	arr_f64_41bbe[1] = (float)arr_i16_41af4.left;
	arr_f64_41bbe[1+4] = (float)(arr_i16_41afc.left - arr_i16_41af4.left) / arr_i16_41b04[2];
	arr_f64_41bbe[2] = (float)arr_i16_41af4.bottom;
	arr_f64_41bbe[2+4] = (float)(arr_i16_41afc.bottom - arr_i16_41af4.bottom) / arr_i16_41b04[2];
	arr_f64_41bbe[3] = (float)arr_i16_41af4.right;
	arr_f64_41bbe[3+4] = (float)(arr_i16_41afc.right - arr_i16_41af4.right) / arr_i16_41b04[2];
	// 131:1c24
	g_toolbox->PenPat(arr_pat_194[1]);
	g_toolbox->PenMode(kPatXor);
	for (int16 j = 6; j <= 9; j++) {
		arr_i32_41296[j]->copyFrom(*arr_i32_41296[j - 1]);
		// 131:1c7a
		setPortBitsToPage(j);
		if (j == 6) {
			g_toolbox->PaintOval(arr_i16_41af4);
		}
		// 131:1c9c
		for (int16 i = 0; i <= 2; i++) {
			// unrolled loop
			arr_f64_41bbe[0] += arr_f64_41bbe[0+4];
			arr_i16_41b0a.top = (int16)arr_f64_41bbe[0];
			arr_f64_41bbe[1] += arr_f64_41bbe[1+4];
			arr_i16_41b0a.left = (int16)arr_f64_41bbe[1];
			arr_f64_41bbe[2] += arr_f64_41bbe[2+4];
			arr_i16_41b0a.bottom = (int16)arr_f64_41bbe[2];
			arr_f64_41bbe[3] += arr_f64_41bbe[3+4];
			arr_i16_41b0a.right = (int16)arr_f64_41bbe[3];
			// 131:1d44
			g_toolbox->PaintOval(arr_i16_41b0a);
		}
	}
	arr_i32_41296[10]->copyFrom(*arr_i32_41296[9]);
	// 131:1d98
	setPortBitsToPage(0xa);
	g_toolbox->PaintOval(arr_i16_41afc);
	setPortBitsToPage(5);
	g_toolbox->SetRect(arr_i16_1bc, 0xae, 0x8c, 0xb6, 0x94);
	g_toolbox->InvertOval(arr_i16_1bc);
	g_toolbox->SetRect(arr_i16_1bc, 0x151, 0x86, 0x159, 0x8e);
	g_toolbox->InvertOval(arr_i16_1bc);
	g_toolbox->SetPortBits(var_i32_32);
	sub_128_e1c();
	sub_128_e58();
	// 131:1e18
	fillRect(0, 0, 0x96, 0x18b, 2);
	fillRect(0x12c, 0x15e, 0x156, 0x200, 2);
	var_i16_18e = 0;
	for (int16 i = 0xa; i <= 0x96; i += 0xa) {
		_zbasic->get(
			0x190,
			i + 0xa,
			0x1ea,
			i + 0x14,
			arr_i32_1e3fc[i/0xa]
		);
		arr_i16_1e8[i + 0] = 0x190;
		var_i16_18e++;
		if (var_i16_18e > 4) {
			var_i16_18e = 1;
		}
		arr_i16_1e8[i + 0xfb] = var_i16_18e;
	}
	// 131:1f1a
	for (int16 j = 1; j <= 0xf; j++) {
		if (j < 9) {
			g_toolbox->InvertRect(var_i16_38);
		}
		var_i32_2 = g_toolbox->TickCount();
		for (int16 i = 0xa; i <= 0x96; i += 0xa) {
			_zbasic->put(arr_i16_1e8[i], i + 0xa, arr_i32_1e3fc[i/0xa], kNotSrcXor);
			arr_i16_1e8[i] -= arr_i16_1e8[0xfb + i]*j;
			_zbasic->put(arr_i16_1e8[i], i + 0xa, arr_i32_1e3fc[i/0xa], kSrcCopy);
			// 131:2062
		}
		// 131:2072
		delayFromMarker(2);
	}
	for (int16 j = 1; j <= 0xf; j++) {
		var_i32_2 = g_toolbox->TickCount();
		for (int16 i = 0xa; i <= 0x96; i += 0xa) {
			_zbasic->put(arr_i16_1e8[i], i+0xa, arr_i32_1e3fc[i/0xa], kNotSrcXor);
			arr_i16_1e8[i] += arr_i16_1e8[0xfb + i]*j;
			_zbasic->put(arr_i16_1e8[i], i+0xa, arr_i32_1e3fc[i/0xa], kSrcCopy);
		}
		delayFromMarker(2);
	}
	// 131:21e0
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x33]);
	var_i16_1b6 = 0x14;
	var_str_76 = _zbasic->str(33); // be content that you still live
	for (int16 i = 1; i <= 0x18; i++) {
		var_i32_2 = g_toolbox->TickCount();
		_zbasic->text(kPrologueFontFool, i, 0, kSrcXor);
		var_i16_1b6 += i;
		drawText(5 + i*2, var_i16_1b6);
		delayFromMarker(3);
	}
	// 131:2282
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x34]);
	var_i32_2 = g_toolbox->TickCount();
	setPortBitsToPage(4);
	g_toolbox->SetRect(arr_i16_1bc, 0, 0, 0x200, 0xdf);
	g_toolbox->InvertRect(arr_i16_1bc);
	g_toolbox->SetRect(arr_i16_1bc, 0, 0xdf, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_toolbox->PenMode(kPatOr);
	g_toolbox->PenPat(arr_pat_194[1]);
	g_toolbox->PaintRect(arr_i16_1bc);
	// 131:2316
	// 131:2316: MOVE.L - HEAP(0x412a6),D0
	// 131:2320: MOVEA.L - D0,A0
	// 131:2322: MOVE.L - (A0),D0
	// 131:2324: PUSH.L - D0
	// 131:2326: MOVE.L - HEAP(0x412c2),D0
	// 131:2330: MOVEA.L - D0,A0
	// 131:2332: MOVE.L - (A0),D0
	// 131:2334: PUSH.L - D0
	// 131:2336: MOVE.L - 0x5580,D0
	setPortBitsToPage(0xb);
}

void FoolPrologue::sub_131_4dc0() {
	// 131:4dc0
	g_toolbox->SetPort(this->var_i32_c);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// Loading Finale
	this->var_str_76 += _zbasic->str(78);
	this->var_str_76 += Common::U32String::format("%d%%", this->var_i16_3ce);
	_zbasic->bufferFlush(this->var_str_76);
	// 131:4e08
	this->fillRect(0, 0, 7, 0x13, this->var_i16_10 - 7);

	this->drawTextCenterAlign(this->var_i16_10 / 2, 0xe);
	g_toolbox->_defaultMenu->setOverlayDirty(true);
	g_toolbox->SetPort(this->var_i32_8);
}

void FoolPrologue::sub_131_4e48() {
	// 131:4e48
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);

	// Uh oh . . .
	this->var_str_76 += Common::U32String::format("\"%s\"", _zbasic->str(80).c_str());
	_zbasic->bufferFlush(this->var_str_76);

	this->drawText(0x1aa, 0xb4);
}

void FoolPrologue::sub_131_4e98() {
	// 131:4e98
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcXor);
	this->var_i16_3fc = 0xa0;
	// "And so the fool heeded the advice of the magician..."
	this->var_str_76 += _zbasic->str(81);
	_zbasic->bufferFlush(this->var_str_76);
	this->drawText(0x92, this->var_i16_3fc);

	this->var_str_76 += _zbasic->str(82);
	_zbasic->bufferFlush(this->var_str_76);
	this->drawText(0x9c, this->var_i16_3fc + 0xf);

	this->var_str_76 += _zbasic->str(83);
	_zbasic->bufferFlush(this->var_str_76);
	this->drawText(0xa6, this->var_i16_3fc + 0x1e);

	this->var_str_76 += _zbasic->str(84);
	_zbasic->bufferFlush(this->var_str_76);
	this->drawText(0xc4, this->var_i16_3fc + 0x2d);

	this->var_str_76 += _zbasic->str(85);
	_zbasic->bufferFlush(this->var_str_76);
	this->drawText(0xe2, this->var_i16_3fc + 0x3c);

	this->sub_128_de2();
}

void FoolPrologue::sub_131_4f96() {
	// 131:4f96
	g_toolbox->MoveTo(
		this->arr_i16_1e8[this->var_i16_74],
		this->arr_i16_1e8[this->var_i16_74+0xfb]
	);

	// 131:4fd4
	g_toolbox->LineTo(
		this->arr_i16_1e8[this->var_i16_74] + this->arr_i16_1e8[this->var_i16_74+0x1f6],
		this->arr_i16_1e8[this->var_i16_74+0xfb]
	);
}

} // End of namespace Fool
