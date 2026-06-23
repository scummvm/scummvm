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



// Based on m68k disassembly of the Fool's Errand v2.0, (c) 1988 Cliff Johnson.

// v1.0 - original release, single-density disks
// v2.0 - fixes full-screen rendering on higher-resolution displays
// v3.0 - newer ZBasic, changed a few graphics assets, removed custom menu font and sounds for compatibility

void FoolPrologue::run(bool finale) {
	_toolbox = new Toolbox();
	_zbasic = new ZBasic(_toolbox);
	_zbasic->loadProgram(Common::Path("Prologue - Finale"));
	this->sub_128_004(finale);
	delete _zbasic;
	_zbasic = nullptr;
	delete _toolbox;
	_toolbox = nullptr;
}

void FoolPrologue::sub_128_004(bool finale) {
	var_i16_1aa = finale ? 2 : 1;

	// 128:0004
	_zbasic->coordinateWindow();
	_toolbox->ClearMenuBar();
	_toolbox->HideCursor();
	_toolbox->UseResFile(_toolbox->CurResFile());

	// 128:001e
	// b54 must be the 1-bit screen buffer
	// 41296 is an array of pointers to screen pages within the buffer
	for (int i = 0; i < 16; i++) {
		this->arr_i32_1e3fc[i] = BitMap(new Graphics::ManagedSurface());
		this->arr_i32_1e3fc[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	for (int i = 0; i < 12; i++) {
		//this->arr_i32_41296[i] = &this->arr_i32_b54[SCREEN_PAGE_SIZE*i];
		this->arr_i32_41296[i] = BitMap(new Graphics::ManagedSurface());
		this->arr_i32_41296[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	this->arr_i32_3bca4 = BitMap(new Graphics::ManagedSurface());
	this->arr_i32_3bca4->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

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
	this->var_str_76 = _zbasic->str(1);
	this->var_str_76 = _zbasic->str(2);

	// fill graphics pages with white
	for (int16 i = 0; i < 12; i++) {
		setPortBitsToPage(i);
		fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	}

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
	_toolbox->ShowCursor();
}

void FoolPrologue::copyScreenToPage(int16 screenPage) {
	// 128:01ba
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	_toolbox->CopyBits(this->var_i32_32, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
}

void FoolPrologue::setPortBitsToPage(int16 screenPage) {
	// 128:01f4
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	_toolbox->SetPortBits(this->var_i32_40);
}

void FoolPrologue::delay(int16 numTicks) {
	// 128:021e
	// original code would poll TickCount in a loop,
	// effectively the same as Delay
	_toolbox->Delay(numTicks);
}

void FoolPrologue::delayFromMarker(int16 numTicks) {
	// 128:024a
	uint32 delay = (uint32)MAX<int>(this->var_i32_2 + numTicks + 1 - _toolbox->TickCount(), 0);
	// again, polling TickCount in a loop.
	_toolbox->Delay(delay);
}

void FoolPrologue::drawTextRightAlign(int16 x, int16 y) {
	// 128:026c
	int16 width = _toolbox->StringWidth(this->var_str_76);
	_toolbox->MoveTo(x - width, y);
	_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::drawTextCenterAlign(int16 x, int16 y) {
	// 128:02a6
	int16 width = _toolbox->StringWidth(this->var_str_76);
	_toolbox->MoveTo(x - (width / 2), y);
	_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID) {
	// 128:02f0
	_toolbox->SetRect(this->arr_rect_41af4, left, top, right, bottom);
	_toolbox->FillRect(this->arr_rect_41af4, this->arr_pat_194[patternID]);
}

void FoolPrologue::zoomClose(int16 patternID, PatternMode mode) {
	// 128:0354
	_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenPat(this->arr_pat_194[patternID]);
	_toolbox->PenMode(mode);
	_toolbox->PenSize(6, 4);

	for (int i = 0; i <= 130; i += 3) {
		_toolbox->FrameRect(this->arr_i16_1bc);
		_toolbox->InsetRect(this->arr_i16_1bc, 6, 4);
		this->delay(0);
	}
	_toolbox->PenNormal();
}

void FoolPrologue::drawTreasurePhaseIn(int16 unk1) {
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
	for (int i = 1; i <= this->var_i16_18e; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		_toolbox->MoveTo(
			this->arr_i16_1e8[i] - unk1,
			this->arr_i16_1e8[i + 0x1f6] - unk1
		);
		_toolbox->LineTo(
			this->arr_i16_1e8[i + 0xfb] + unk1,
			this->arr_i16_1e8[i + 0x1f6] - unk1
		);
		if (i % this->var_i16_18c == 0) {
			this->delayFromMarker(0);
		};
	}

}

void FoolPrologue::sub_128_50a(int16 screenPage, int16 left, int16 right, int16 updatePeriod) {
	// 128:050a
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	this->var_i16_5c.left = left;
	this->var_i16_5c.right = right;
	if (updatePeriod == 0) {
		updatePeriod = 0x14;
	}
	if (updatePeriod == 1) {
		updatePeriod = 0x8;
	}
	if (updatePeriod == 2) {
		updatePeriod = 0x6;
	}
	if (updatePeriod == 3) {
		updatePeriod = 0x5;
	}
	// 128:0584
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		this->var_i16_5c.top = this->arr_i16_412ea[i];
		this->var_i16_5c.bottom = this->arr_i16_412ea[i] + 1;
		_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_5c, this->var_i16_5c, kSrcCopy, nullptr);
		if (i % updatePeriod == 0) {
			this->delayFromMarker(0);
		}
	}
}

void FoolPrologue::blitPageToScreen(int16 screenPage) {
	// 128:0610
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
}

void FoolPrologue::scanlineTransition(int16 patternID) {
	// 128:064a
	_toolbox->PenNormal();
	_toolbox->PenPat(this->arr_pat_194[patternID]);
	// 128:0668
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		_toolbox->MoveTo(0, this->arr_i16_412ea[i]);
		_toolbox->LineTo(SCREEN_WIDTH, this->arr_i16_412ea[i]);
		if (i % 5 == 0) {
			this->delayFromMarker(0);
		}
	}
	_toolbox->PenNormal();
}

void FoolPrologue::zoomTransition(int16 screenPage) {
	// 128:06e4
	this->var_i32_40 = this->arr_i32_41296[screenPage];
	for (int i = 1; i <= 0x36; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		this->var_i16_5c.top = (SCREEN_HEIGHT/2) - (int)(i*3.33);
		this->var_i16_5c.left = (SCREEN_WIDTH/2) - i*5;
		this->var_i16_5c.bottom = (SCREEN_HEIGHT/2) + (int)(i*3.33);
		this->var_i16_5c.right = (SCREEN_WIDTH/2) + i*5;
		_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_5c, this->var_i16_5c, kSrcCopy, nullptr);
		_toolbox->Delay(0);
	}
	_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
	_toolbox->Delay(0);
}

void FoolPrologue::sub_128_800(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4, int16_t unk5, int16_t unk6, int16_t unk7, int16_t unk8, int16_t unk9) {
	// 128:0800
	this->arr_rect_41af4.top = unk1;
	this->arr_rect_41af4.left = unk2;
	this->arr_rect_41af4.bottom = unk3;
	this->arr_rect_41af4.right = unk4;
	this->arr_i16_41afc.top = unk5;
	this->arr_i16_41afc.left = unk6;
	this->arr_i16_41afc.bottom = unk7;
	this->arr_i16_41afc.right = unk8;
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);

	// 128:08bc
	this->arr_f64_41bbe[0] = (double)this->arr_rect_41af4.top;
	this->arr_f64_41bbe[1] = (double)this->arr_rect_41af4.left;
	this->arr_f64_41bbe[2] = (double)this->arr_rect_41af4.bottom;
	this->arr_f64_41bbe[3] = (double)this->arr_rect_41af4.right;
	this->arr_f64_41bbe[4] = (double)(this->arr_i16_41afc.top - this->arr_rect_41af4.top)/(double)(unk9);
	this->arr_f64_41bbe[5] = (double)(this->arr_i16_41afc.left - this->arr_rect_41af4.left)/(double)(unk9);
	this->arr_f64_41bbe[6] = (double)(this->arr_i16_41afc.bottom - this->arr_rect_41af4.bottom)/(double)(unk9);
	this->arr_f64_41bbe[7] = (double)(this->arr_i16_41afc.right - this->arr_rect_41af4.right)/(double)(unk9);

	// 128:096e
	_toolbox->PaintRect(this->arr_rect_41af4);
	for (int i = 1; i <= (unk9 - 1); i++) {
		this->var_i32_2 = _toolbox->TickCount();

		// 128:098c
		for (int j = 0; j <= 3; j++) {
			this->arr_f64_41bbe[j] += this->arr_f64_41bbe[j+4];
		}
		this->arr_i16_41b0a.top = (int16)this->arr_f64_41bbe[0];
		this->arr_i16_41b0a.left = (int16)this->arr_f64_41bbe[1];
		this->arr_i16_41b0a.bottom = (int16)this->arr_f64_41bbe[2];
		this->arr_i16_41b0a.right = (int16)this->arr_f64_41bbe[3];
		// 128:0a2e
		_toolbox->PaintRect(this->arr_i16_41b0a);
		this->delayFromMarker(0);
	}
	_toolbox->PaintRect(this->arr_i16_41afc);
	_toolbox->PenNormal();
}

void FoolPrologue::drawText(int16_t x, int16_t y) {
	// 128:0a6c
	_toolbox->MoveTo(x, y);
	_toolbox->DrawString(this->var_str_76);
}

void FoolPrologue::drawRainRecycle(int16_t unk) {
	// 128:0a8c
	this->var_i16_1a4 = unk;
	_toolbox->PenMode(kPatXor);
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
			_toolbox->Delay(0);
		}
	} while (this->var_i32_2 + this->var_i16_1a4 > _toolbox->TickCount());
	_toolbox->Delay(0);
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

void FoolPrologue::drawClickMessage() {
	// 128:0de2
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcXor);
	this->var_str_76 = _zbasic->str(3);
	this->drawText(5, 0x151);
}

void FoolPrologue::sub_128_e1c() {
	// 128:0e1c
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcXor);
	this->var_str_76 = _zbasic->str(4);
	this->drawTextRightAlign(0x1fb, 0x154);
}

void FoolPrologue::sub_128_e58() {
	// 128:0e58
	this->sub_128_e80();
	while (true) {
		// was: 2
		this->var_i16_1ba = _toolbox->GetNextEvent(-1, this->var_ev_22);
		if (this->var_ev_22.what == kMouseDown)
			break;
		if ((this->var_ev_22.what == kScummVMQuitEvt) || (this->var_ev_22.what == kScummVMReturnToLauncherEvt)) {
			_quit = true;
			return;
		}
		// wait until next redraw
		if (this->var_ev_22.what == kNullEvent)
			_toolbox->Delay(0);
	}
	this->sub_128_e80();
}

void FoolPrologue::sub_128_e80() {
	// 128:0e80
	while (true) {
		this->var_i16_1ba = (int)_toolbox->GetNextEvent(-1, this->var_ev_22);
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
			_toolbox->Delay(0);
	}
}

void FoolPrologue::sub_128_ed2() {
	// 128:0ed2
	_toolbox->BeginUpdate(this->var_window_24);
	_toolbox->EndUpdate(this->var_window_24);
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
		_toolbox->SetCPixel(0x64, 0x64, this->var_i16_1c);
		_toolbox->GetCPixel(0x64, 0x64, this->var_i16_1c);
	}

	// 129:00ee
	// used for drawing on the menu bar
	this->var_i32_c = &this->arr_grafport_a8a;
	_toolbox->OpenPort(this->var_i32_c);
	//this->var_i32_c->portBits = _toolbox->_defaultMenuBits;
	//this->var_i32_c->portRect = _toolbox->_defaultMenuBits->getBounds();

	this->var_i32_8 = &this->arr_grafport_9c0;
	_toolbox->OpenPort(this->var_i32_8);

	if ((this->var_i16_10 != SCREEN_WIDTH || this->var_i16_12 != SCREEN_HEIGHT)) {
		// 129:013a
		this->fillRect(0x14, 0, this->var_i16_12, this->var_i16_10, 2);
		_toolbox->SetRect(this->arr_i16_1bc, this->var_i16_14 - 2, this->var_i16_16 - 2, this->var_i16_14 + SCREEN_WIDTH + 2, this->var_i16_16 + SCREEN_HEIGHT + 2);
		_toolbox->PenPat(this->arr_pat_194[1]);
		_toolbox->FrameRect(this->arr_i16_1bc);
	}

	// 129:01b0
	_toolbox->SetPort(this->var_i32_8);
	_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->MovePortTo(this->var_i16_14, this->var_i16_16);
	_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->ClipRect(this->arr_i16_1bc);

	// 129:01f2
	if (this->var_i16_12 == SCREEN_HEIGHT) {
		_toolbox->SetRect(this->arr_i16_1bc, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		_toolbox->FillRect(this->arr_i16_1bc, this->arr_pat_194[1]);
	} else {
		_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_toolbox->FillRect(this->arr_i16_1bc, this->arr_pat_194[1]);
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
		_toolbox->InitCursor();
		_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_i32_41296[0], true);
		_zbasic->sound(0x19, 0x64, 0xff, 0x0);
		this->fillRect(0x46, 0x64, 0xdc, 0x19c, 2);
		this->fillRect(0x4b, 0x69, 0xd7, 0x197, 1);

		// 129:0386
		this->fillRect(0x50, 0x6e, 0xd2, 0x192, 2);
		_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
		// "not enough memory" message
		this->var_str_76 = _zbasic->str(6);
		this->drawTextCenterAlign(0x100, 0x64);
		this->var_str_76 = _zbasic->str(7);
		this->drawTextCenterAlign(0x100, 0x78);
		this->var_str_76 = _zbasic->str(8);
		this->drawTextCenterAlign(0x100, 0x96);
		this->var_str_76 = _zbasic->str(9);
		this->drawTextCenterAlign(0x100, 0xaa);
		this->var_str_76 = _zbasic->str(10);
		this->drawTextCenterAlign(0x100, 0xcb);
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

			for (int i = 1; i <= 0xa; i++) {
				this->var_str_76 += _zbasic->chr(i);
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
			_toolbox->InitCursor();
			_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, this->arr_i32_41296[0], true);
			_zbasic->sound(0x19, 0x64, 0xff, 0x00);
			this->fillRect(0x64, 0x82, 0xdc, 0x17e, 2);
			this->fillRect(0x69, 0x87, 0xd7, 0x179, 1);
			this->fillRect(0x6e, 0x8c, 0xd2, 0x174, 2);
			// 129:0662
			_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
			// "set your monitor to black and white" message"
			this->var_str_76 = _zbasic->str(13);
			this->drawTextCenterAlign(0x100, 0x82);
			this->var_str_76 = _zbasic->str(14);
			this->drawTextCenterAlign(0x100, 0x96);

			this->var_str_76 = _zbasic->str(15);
			// if this is the finale
			if (this->var_i16_1aa == 2) {
				this->var_str_76 = _zbasic->str(16);
			}
			this->drawTextCenterAlign(0x100, 0xaa);

			this->var_str_76 = _zbasic->str(17);
			this->drawTextCenterAlign(0x100, 0xcb);
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
	this->glob_i32_2ce = _toolbox->GetPicture(0x47);
	this->var_i16_3ce = 10;
	this->prologueDrawLoadingMsg();
	this->setPortBitsToPage(0xb);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x74, 0xaa, this->glob_i32_2ce);
	_toolbox->ReleaseResource(this->glob_i32_2ce);

	// 130:007a
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcBic);
	this->var_str_76 = _zbasic->str(18);
	this->drawTextCenterAlign(0x103, 0x10d);
	_toolbox->SetPortBits(this->var_i32_32);
	// cliffside images with lightning
	for (int i = 1; i <= 5; i++) {
		this->prologueBufferNextPicture();
	}
	this->setPortBitsToPage(7);

	// 130:00d6
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x96, this->arr_i32_0[1]);
	_toolbox->ReleaseResource(this->arr_i32_0[1]);
	this->setPortBitsToPage(8);

	// 130:011c
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x96, this->arr_i32_0[2]);
	_toolbox->ReleaseResource(this->arr_i32_0[2]);

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
	_toolbox->ReleaseResource(this->arr_i32_0[6]);
	// high priestess face
	this->prologueBufferNextPicture();

	//_zbasic->blockMove(this->arr_i32_41296[0], this->arr_i32_41296[1], 0x5580);
	this->arr_i32_41296[1]->copyFrom(*this->arr_i32_41296[0]);
	this->setPortBitsToPage(1);

	// 130:0202
	_zbasic->picture(0xa2, 0x3c, this->arr_i32_0[7]);
	_toolbox->ReleaseResource(this->arr_i32_0[7]);
	// priestess on mountain
	this->prologueBufferNextPicture();
	this->setPortBitsToPage(2);

	// 130:0234
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x5a, 0x2e, this->arr_i32_0[8]);
	_toolbox->ReleaseResource(this->arr_i32_0[8]);

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
		_toolbox->ReleaseResource(this->arr_i32_0[j]);
		_toolbox->ReleaseResource(this->arr_i32_0[j - 6]);
	}
	// priestess lightning closeup
	this->prologueBufferNextPicture();

	// 130:0320
	this->setPortBitsToPage(6);
	_zbasic->picture(0, 0, this->arr_i32_0[0xc]);
	_toolbox->ReleaseResource(this->arr_i32_0[0xc]);

	// 130:034a
	for (int i = 0xd; i <= 0x12; i++) {
		// priestess silhouette -> fool on the cliff.
		this->prologueBufferNextPicture();
	}

	// fool looking up at sun
	this->arr_i32_0[0x48] = _toolbox->GetPicture(0x48);
	_toolbox->SetPortBits(this->var_i32_32);

	// 130:0380
	if ((this->var_i16_10 == SCREEN_WIDTH) && (this->var_i16_12 == SCREEN_HEIGHT)) {
		this->fillRect(0, 0, 0x14, SCREEN_WIDTH, 1);
	} else {
		// FIXME: menu draw code not used
		/*_toolbox->SetPort(this->var_i32_c);
		this->fillRect(0, 0, 0x14, this->var_i16_10, 2);
		_toolbox->_defaultMenu->setOverlayDirty(true);
		_toolbox->SetPort(this->var_i32_8);*/
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

	_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->SetRect(this->arr_i32_1c4, 0, 0x96, SCREEN_WIDTH, 0x118);

	// 130:043c
	for (int i = 1; i <= 2; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		this->drawRainRecycle(0xb4);
		this->var_i32_2 = _toolbox->TickCount();
		_toolbox->InvertRect(this->arr_i16_1bc);
		this->delayFromMarker(1);
		_toolbox->InvertRect(this->arr_i16_1bc);
		// 130:04ca
		for (int j = 0; j <= 3; j++) {
			this->var_i32_2 = _toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[8];
			// 130:049e
			_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(4);
			this->var_i16_192 = 0;
			this->var_i32_2 = _toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[3];
			_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(5);
			// 130:0518
			this->var_i32_2 = _toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[4];
			_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(5);
			this->var_i32_2 = _toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[5];
			_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(5);
		}
		// 130:05b6
		this->var_i32_40 = this->arr_i32_41296[7];
		_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);

	}

	// 130:05fc
	this->blitPageToScreen(0);
	this->drawRain();
	this->var_i32_2 = _toolbox->TickCount();
	this->drawRainRecycle(0xb4);
	this->var_i32_40 = this->arr_i32_41296[1];

	// scanline fade in priestess face

	// 130:062c
	int y = 0;
	while (y != SCREEN_HEIGHT) {
		this->var_i16_5c.right = SCREEN_WIDTH;
		this->var_i32_2 = _toolbox->TickCount();
		for (int j = 1; j < 0x7; j++) {
			this->var_i16_192 = 1;
			if (y < SCREEN_HEIGHT) {
				y += 1;
			}
			this->var_i16_5c.top = this->arr_i16_412ea[y];
			this->var_i16_5c.bottom = this->arr_i16_412ea[y] + 1;
			// 130:068e
			_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->var_i16_5c, this->var_i16_5c, kSrcCopy, nullptr);
		}
		// 130:06b0
		this->drawRainRecycle(1);
	}

	// lightning flash over priestess face

	this->var_i32_2 = _toolbox->TickCount();
	this->drawRainRecycle(0xa);
	this->blitPageToScreen(1);
	// 130:06d4
	for (int i = 1; i <= 4; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		_toolbox->InvertRect(this->arr_i16_1bc);
		this->drawRainRecycle(0x2);
	}
	// 130:0704
	this->var_i32_2 = _toolbox->TickCount();
	_toolbox->InvertRect(this->arr_i16_1bc);
	this->drawRainRecycle(0xf);

	this->var_i32_2 = _toolbox->TickCount();
	_toolbox->InvertRect(this->arr_i16_1bc);
	this->drawRainRecycle(0x78);

	// cut to shot of cliff
	this->blitPageToScreen(2);

	// 130:0740
	this->drawRain();
	this->var_i32_2 = _toolbox->TickCount();
	this->drawRainRecycle(0xb4);

	_toolbox->SetRect(this->arr_i32_1c4, 0, 0, 0x150, 0x96);

	// priestess zap
	// 130:0778
	for (int j = 0; j <= 8; j++) {
		for (int i = 3; i <= 5; i++) {
			this->var_i32_2 = _toolbox->TickCount();
			this->var_i32_40 = this->arr_i32_41296[i];
			_toolbox->CopyBits(this->var_i32_40, this->var_i32_32, this->arr_i32_1c4, this->arr_i32_1c4, kSrcCopy, nullptr);
			this->drawRainRecycle(3);
		}
	}

	// cut to closeup of priestess mid-zap
	// 130:07ec
	this->blitPageToScreen(6);
	_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int i = 0; i <= 0x2d; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		// seizure warning
		_toolbox->InvertRect(this->arr_i16_1bc);
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

	_toolbox->ReleaseResource(this->arr_i32_0[0xd]);
	_toolbox->ReleaseResource(this->arr_i32_0[0xe]);

	// start loading some new resources from the floppy disk into the screen buffers
	this->var_i32_2 = _toolbox->TickCount();
	// 130:0890
	_zbasic->restore(0);
	this->setPortBitsToPage(0);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0x17, this->arr_i32_0[0xf]);
	_toolbox->ReleaseResource(this->arr_i32_0[0xf]);
	// 130:08dc
	this->prologueRenderNextText();
	this->drawClickMessage();
	this->setPortBitsToPage(1);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0xf0, 0xba, this->arr_i32_0[0x10]);
	_toolbox->ReleaseResource(this->arr_i32_0[0x10]);
	this->prologueRenderNextText();
	this->drawClickMessage();
	// 130:0934
	for (int i = 2; i <= 5; i++) {
		//_zbasic->blockMove(this->arr_i32_41296[1], this->arr_i32_41296[i], 0x5580);
		this->arr_i32_41296[i]->copyFrom(*this->arr_i32_41296[1]);
	}
	// 130:0978
	// draw the sun pulsing to pages 1-5
	for (int i = 1; i <= 5; i++) {
		this->setPortBitsToPage(i);
		_toolbox->PenPat(this->arr_pat_194[1]);
		_toolbox->SetRect(this->arr_i16_1bc, 0x3c, 0x3c, 0x8c, 0x8c);
		_toolbox->FrameOval(this->arr_i16_1bc);
		_toolbox->PenPat(this->arr_pat_194[4]);
		// 130:09d0
		_toolbox->InsetRect(this->arr_i16_1bc, -i, -i);
		for (int j = 2; j <= 0xd; j++) {
			_toolbox->InsetRect(this->arr_i16_1bc, -j, -j);
			_toolbox->FrameOval(this->arr_i16_1bc);
		}
	}
	// 130:0a42
	this->setPortBitsToPage(6);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x166, 0x77, this->arr_i32_0[0x11]);
	this->drawClickMessage();
	//_zbasic->blockMove(this->arr_i32_41296[6], this->arr_i32_41296[7], 0x5580);
	this->arr_i32_41296[7]->copyFrom(*this->arr_i32_41296[6]);
	this->prologueRenderNextText();
	_toolbox->ReleaseResource(this->arr_i32_0[0x11]);
	this->setPortBitsToPage(7);
	// 130:0ac2
	_zbasic->picture(-0x14, -0x1e, this->arr_i32_0[0x12]);
	this->prologueRenderNextText();
	_toolbox->ReleaseResource(this->arr_i32_0[0x12]);
	this->setPortBitsToPage(8);
	//_zbasic->blockMove(this->arr_i32_41296[7], this->arr_i32_41296[8], 0x5580);
	this->arr_i32_41296[8]->copyFrom(*this->arr_i32_41296[7]);
	this->fillRect(0x73, 0x163, 0xfa, 0x1e0, 0);
	_zbasic->picture(0x16d, 0x80, this->arr_i32_0[0x48]);
	_toolbox->ReleaseResource(this->arr_i32_0[0x48]);
	_toolbox->SetPortBits(this->var_i32_32);

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
		this->var_i32_2 = _toolbox->TickCount();
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
		this->var_i16_1ba = _toolbox->GetNextEvent(2, this->var_ev_22);
	}
	while (!((this->var_ev_22.what == kNullEvent) && (this->var_ev_22.modifiers & kModMouseButtonUp))) {
		// 130:0c0c
		this->var_i32_2 = _toolbox->TickCount();
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
		this->var_i16_1ba = _toolbox->GetNextEvent(-1, this->var_ev_22);
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

	this->sub_128_50a(7, 0, SCREEN_WIDTH, 0);
	this->delay(0xa);
	// "I dare"
	this->blitPageToScreen(0x8);
	this->sub_128_e58();
	if (_quit)
		return;

	// zoom to close

	zoomClose(1, kPatCopy);
	// 130:0ce6
	// JMP 1002
	// _zbasic->pushOldCodeResource(0x82);
	//this->finaleRun();
}

void FoolPrologue::prologueBufferNextPicture() {
	// 130:0cea
	this->var_i16_3d4 += 1;
	this->arr_i32_0[this->var_i16_3d4] = _toolbox->GetPicture(this->var_i16_3d4);
	// 130:0d0a
	this->var_i16_3ce = this->var_i16_3d4 * 5 + 10;
	this->prologueDrawLoadingMsg();
}

void FoolPrologue::prologueDrawLoadingMsg() {
	// FIXME: we don't share the menu surface here yet
	return;
	// 130:0d28
	_toolbox->SetPort(this->var_i32_c);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// "loading prologue" message
	this->var_str_76 = _zbasic->str(19);
	this->var_str_76 += Common::U32String::format(" %d", this->var_i16_3ce);
	this->var_str_76 += _zbasic->str(20);
	// 130:0d70
	// white out the contents of the top menu bar.
	// the 7px offset on either side seems to be to avoid hitting the rounded screen corners?
	this->fillRect(0, 0x7, 0x13, this->var_i16_10 - 7, 0);
	this->drawTextCenterAlign(this->var_i16_10/2, 0xe);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(this->var_i32_8);
	_toolbox->Delay(0);
}

void FoolPrologue::drawRain() {
	// 130:0db0
	_zbasic->unk_20();
	_toolbox->PenMode(kPatXor);
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
	_toolbox->Delay(0);
}

void FoolPrologue::drawRainDrop() {
	// 130:0e82
	_toolbox->MoveTo(this->arr_i16_1e8[this->var_i16_6], this->arr_i16_1e8[this->var_i16_6 + 0xfb]);
	// 130:0ec0
	_toolbox->LineTo(
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
		this->drawText(this->var_i16_176, this->var_i16_180);
	}
}


} // End of namespace Fool
