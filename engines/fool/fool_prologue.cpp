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

	Common::MacFinderInfo finfo;
	if (_toolbox->GetFInfo(Common::U32String("Prologue - Finale"), 0, finfo) == kNoErr) {
		_zbasic->loadProgram(Common::Path("Prologue - Finale", ':'));
	} else if (_toolbox->GetFInfo(Common::U32String("Prologue & Finale"), 0, finfo) == kNoErr) {
		_zbasic->loadProgram(Common::Path("Prologue & Finale", ':'));
	} else {
		error("FoolGame::run: Prologue - Finale program not found");
		return;
	}

	// The fonts in this game are a bit of a mess.
	// Both copies of the Fool and Small fonts have missing characters,
	// so we need to have them coexisting.
	int16 exec = _zbasic->getFileId();
	Handle foolFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), kPrologueFontFool + 10);
	// These second copies can't clash, so we need to patch in new family IDs.
	foolFOND->data()[2] = 0;
	foolFOND->data()[3] = (byte)kPrologueFontFool;
	_toolbox->_injectFOND(exec, foolFOND->data(), foolFOND->size(), Common::String("Fool Prologue"));
	Handle smallFOND = _toolbox->GetResource(MKTAG('F', 'O', 'N', 'D'), kPrologueFontSmall + 10);
	smallFOND->data()[2] = 0;
	smallFOND->data()[3] = (byte)kPrologueFontSmall;
	_toolbox->_injectFOND(exec, smallFOND->data(), smallFOND->size(), Common::String("Small Prologue"));

	sub_128_004(finale);
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
		arr_i32_1e3fc[i] = BitMap(new Graphics::ManagedSurface());
		arr_i32_1e3fc[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	for (int i = 0; i < 12; i++) {
		//arr_i32_41296[i] = &arr_i32_b54[SCREEN_PAGE_SIZE*i];
		arr_i32_41296[i] = BitMap(new Graphics::ManagedSurface());
		arr_i32_41296[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	arr_i32_3bca4 = BitMap(new Graphics::ManagedSurface());
	arr_i32_3bca4->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	// 128:0066
	var_i32_4e = arr_i32_41296[11];
	// 128:0078
	// pattern buffer
	_patterns[0] = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }; // solid white
	_patterns[1] = { { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 } }; // checkerboard
	_patterns[2] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }; // solid black


	// 128:00d6
	_patterns[3] = { { 0x01, 0x07, 0x0e, 0x0c, 0x7c, 0x70, 0xc0, 0xc0 } };
	_patterns[4] = { { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 } };

	// 128:017a
	// about message
	var_str_76 = _zbasic->str(0);
	var_str_76 = _zbasic->str(1);
	var_str_76 = _zbasic->str(2);

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
	sub_129_004();
	if (_quit)
		return;
	var_i32_1a6 = _zbasic->mem(-1);
	if (var_i16_1aa == 1) {
		prologueRun();
		var_i32_1a6 = _zbasic->mem(-1);
	}
	// 128:0cb0
	if (var_i16_1aa == 2) {
		finaleRun();
		var_i32_1a6 = _zbasic->mem(-1);
	}
	// 128:0cc8
	//_zbasic->unk4(); // exit command?
	_toolbox->ShowCursor();
}

void FoolPrologue::copyScreenToPage(int16 screenPage) {
	// 128:01ba
	var_i32_40 = arr_i32_41296[screenPage];
	_toolbox->CopyBits(var_i32_32, var_i32_40, var_i16_38, var_i16_38, kSrcCopy, nullptr);
}

void FoolPrologue::setPortBitsToPage(int16 screenPage) {
	// 128:01f4
	var_i32_40 = arr_i32_41296[screenPage];
	_toolbox->SetPortBits(var_i32_40);
}

void FoolPrologue::delay(int16 numTicks) {
	// 128:021e
	// original code would poll TickCount in a loop,
	// effectively the same as Delay
	_toolbox->Delay(numTicks);
}

void FoolPrologue::delayFromMarker(int16 numTicks) {
	// 128:024a
	uint32 delay = (uint32)MAX<int>(var_i32_2 + numTicks + 1 - _toolbox->TickCount(), 0);
	// again, polling TickCount in a loop.
	_toolbox->Delay(delay);
}

void FoolPrologue::drawTextRight(const Common::U32String &str, int16 x, int16 y) {
	// 128:026c
	int16 width = _toolbox->StringWidth(str);
	_toolbox->MoveTo(x - width, y);
	_toolbox->DrawString(str);
}

void FoolPrologue::drawTextCenter(const Common::U32String &str, int16 x, int16 y) {
	// 128:02a6
	int16 width = _toolbox->StringWidth(str);
	_toolbox->MoveTo(x - (width / 2), y);
	_toolbox->DrawString(str);
}

void FoolPrologue::fillRect(int16 top, int16 left, int16 bottom, int16 right, int16 patternID) {
	// 128:02f0
	_toolbox->SetRect(arr_rect_41af4, left, top, right, bottom);
	_toolbox->FillRect(arr_rect_41af4, _patterns[patternID]);
}

void FoolPrologue::zoomClose(int16 patternID, PatternMode mode) {
	// 128:0354
	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenPat(_patterns[patternID]);
	_toolbox->PenMode(mode);
	_toolbox->PenSize(6, 4);

	for (int i = 0; i <= 130; i += 3) {
		_toolbox->FrameRect(arr_i16_1bc);
		_toolbox->InsetRect(arr_i16_1bc, 6, 4);
		delay(0);
	}
	_toolbox->PenNormal();
}

void FoolPrologue::drawTreasurePhaseIn(int16 unk1) {
	// 128:03ee
	var_i16_18c = 0x5;
	if (var_i16_18e > 0x28) {
		var_i16_18c = 0x8;
	}
	if (var_i16_18e > 0x50) {
		var_i16_18c = 0xb;
	}
	if (var_i16_18e > 0x64) {
		var_i16_18c = 0xe;
	}
	// 128:0428
	for (int i = 1; i <= var_i16_18e; i++) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->MoveTo(
			arr_i16_1e8[i] - unk1,
			arr_i16_1e8[i + 0x1f6] - unk1
		);
		_toolbox->LineTo(
			arr_i16_1e8[i + 0xfb] + unk1,
			arr_i16_1e8[i + 0x1f6] - unk1
		);
		if (i % var_i16_18c == 0) {
			delayFromMarker(0);
		};
	}

}

void FoolPrologue::sub_128_50a(int16 screenPage, int16 left, int16 right, int16 updatePeriod) {
	// 128:050a
	var_i32_40 = arr_i32_41296[screenPage];
	var_i16_5c.left = left;
	var_i16_5c.right = right;
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
		var_i32_2 = _toolbox->TickCount();
		var_i16_5c.top = arr_i16_412ea[i];
		var_i16_5c.bottom = arr_i16_412ea[i] + 1;
		_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_5c, var_i16_5c, kSrcCopy, nullptr);
		if (i % updatePeriod == 0) {
			delayFromMarker(0);
		}
	}
}

void FoolPrologue::blitPageToScreen(int16 screenPage) {
	// 128:0610
	var_i32_40 = arr_i32_41296[screenPage];
	_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_38, var_i16_38, kSrcCopy, nullptr);
}

void FoolPrologue::scanlineTransition(int16 patternID) {
	// 128:064a
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[patternID]);
	// 128:0668
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->MoveTo(0, arr_i16_412ea[i]);
		_toolbox->LineTo(SCREEN_WIDTH, arr_i16_412ea[i]);
		if (i % 5 == 0) {
			delayFromMarker(0);
		}
	}
	_toolbox->PenNormal();
}

void FoolPrologue::zoomTransition(int16 screenPage) {
	// 128:06e4
	var_i32_40 = arr_i32_41296[screenPage];
	for (int i = 1; i <= 0x36; i++) {
		var_i32_2 = _toolbox->TickCount();
		var_i16_5c.top = (SCREEN_HEIGHT/2) - (int)(i*3.33);
		var_i16_5c.left = (SCREEN_WIDTH/2) - i*5;
		var_i16_5c.bottom = (SCREEN_HEIGHT/2) + (int)(i*3.33);
		var_i16_5c.right = (SCREEN_WIDTH/2) + i*5;
		_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_5c, var_i16_5c, kSrcCopy, nullptr);
		_toolbox->Delay(0);
	}
	_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_38, var_i16_38, kSrcCopy, nullptr);
	_toolbox->Delay(0);
}

void FoolPrologue::sub_128_800(int16_t unk1, int16_t unk2, int16_t unk3, int16_t unk4, int16_t unk5, int16_t unk6, int16_t unk7, int16_t unk8, int16_t unk9) {
	// 128:0800
	arr_rect_41af4.top = unk1;
	arr_rect_41af4.left = unk2;
	arr_rect_41af4.bottom = unk3;
	arr_rect_41af4.right = unk4;
	arr_i16_41afc.top = unk5;
	arr_i16_41afc.left = unk6;
	arr_i16_41afc.bottom = unk7;
	arr_i16_41afc.right = unk8;
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);

	// 128:08bc
	arr_f64_41bbe[0] = (double)arr_rect_41af4.top;
	arr_f64_41bbe[1] = (double)arr_rect_41af4.left;
	arr_f64_41bbe[2] = (double)arr_rect_41af4.bottom;
	arr_f64_41bbe[3] = (double)arr_rect_41af4.right;
	arr_f64_41bbe[4] = (double)(arr_i16_41afc.top - arr_rect_41af4.top)/(double)(unk9);
	arr_f64_41bbe[5] = (double)(arr_i16_41afc.left - arr_rect_41af4.left)/(double)(unk9);
	arr_f64_41bbe[6] = (double)(arr_i16_41afc.bottom - arr_rect_41af4.bottom)/(double)(unk9);
	arr_f64_41bbe[7] = (double)(arr_i16_41afc.right - arr_rect_41af4.right)/(double)(unk9);

	// 128:096e
	_toolbox->PaintRect(arr_rect_41af4);
	for (int i = 1; i <= (unk9 - 1); i++) {
		var_i32_2 = _toolbox->TickCount();

		// 128:098c
		for (int j = 0; j <= 3; j++) {
			arr_f64_41bbe[j] += arr_f64_41bbe[j+4];
		}
		arr_i16_41b0a.top = (int16)arr_f64_41bbe[0];
		arr_i16_41b0a.left = (int16)arr_f64_41bbe[1];
		arr_i16_41b0a.bottom = (int16)arr_f64_41bbe[2];
		arr_i16_41b0a.right = (int16)arr_f64_41bbe[3];
		// 128:0a2e
		_toolbox->PaintRect(arr_i16_41b0a);
		delayFromMarker(0);
	}
	_toolbox->PaintRect(arr_i16_41afc);
	_toolbox->PenNormal();
}

void FoolPrologue::drawText(const Common::U32String &str, int16_t x, int16_t y) {
	// 128:0a6c
	_toolbox->MoveTo(x, y);
	_toolbox->DrawString(str);
}

void FoolPrologue::drawRainRecycle(int16_t unk) {
	// 128:0a8c
	var_i16_1a4 = unk;
	_toolbox->PenMode(kPatXor);
	do {
		// 128:0a96
		drawRainDrop();
		arr_i16_1e8[var_i16_6] =
		arr_i16_1e8[0x2f1+var_i16_6]
		+ arr_i16_1e8[var_i16_6];

		arr_i16_1e8[0xfb+var_i16_6] =
		arr_i16_1e8[0xfb+var_i16_6]
		+ arr_i16_1e8[0x2f1+var_i16_6];

		// 128:0b52
		if ((arr_i16_1e8[var_i16_6] > 0x1f4) || (arr_i16_1e8[var_i16_6 + 0xfb] > 0x140)) {

		    // 128:0bae
			arr_i16_1e8[var_i16_6] = _zbasic->rndInt(0x264) - 0xc8;
			arr_i16_1e8[0xfb + var_i16_6] = _zbasic->rndInt(0x1ba) - 0xc8;
			arr_i16_1e8[0x1f6 + var_i16_6] = _zbasic->rndInt(0x5) + 0x1;
			arr_i16_1e8[0x2f1 + var_i16_6] = _zbasic->rndInt(0xa) + 0x19;
		}
		// 128:0c56
		drawRainDrop();
		var_i16_6 += 1;
		if (var_i16_6 > 0xb5) {
			var_i16_6 = 0;
			_toolbox->Delay(0);
		}
	} while (var_i32_2 + var_i16_1a4 > _toolbox->TickCount());
	_toolbox->Delay(0);
}

void FoolPrologue::sub_128_c8a() {

}

void FoolPrologue::shuffleScanlines() {
	_zbasic->unk_20();
	var_i32_1ac = (byte *)&arr_i16_41598[0];
	var_i32_1b0 = (byte *)&arr_i16_41846[0];
	var_i16_1b4 = var_i16_18e * 2;
	// 128:0d00
	for (int i=0; i < var_i16_18e; i++) {
		arr_i16_41598[i] = i;
	}
	// 128:0d2a
	for (int i=var_i16_18e - 1; i >= 0; i--) {
		var_i16_1b6 = _zbasic->rndInt(i + 1) - 1;
		arr_i16_412ea[i] = arr_i16_41598[var_i16_1b6];
		var_i16_1b8 = var_i16_1b6 * 2 + 2;
		_zbasic->blockMove(var_i32_1ac + var_i16_1b8, var_i32_1b0 + var_i16_1b8, var_i16_1b4 - var_i16_1b8);
		_zbasic->blockMove(var_i32_1b0 + var_i16_1b8, var_i32_1ac + var_i16_1b8 - 2, var_i16_1b4 - var_i16_1b8);
	}
}

void FoolPrologue::drawClickMessage() {
	// 128:0de2
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcXor);
	// Click Mouse to Continue
	drawText(_zbasic->str(3), 5, 0x151);
}

void FoolPrologue::drawClickMessageRightAlign() {
	// 128:0e1c
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcXor);
	// Click Mouse to Continue
	drawTextRight(_zbasic->str(4), 0x1fb, 0x154);
}

void FoolPrologue::sub_128_e58() {
	// 128:0e58
	sub_128_e80();
	while (true) {
		// was: 2
		var_i16_1ba = _toolbox->GetNextEvent(-1, var_ev_22);
		if (var_ev_22.what == kMouseDown)
			break;
		if ((var_ev_22.what == kScummVMQuitEvt) || (var_ev_22.what == kScummVMReturnToLauncherEvt)) {
			_quit = true;
			return;
		}
		// wait until next redraw
		if (var_ev_22.what == kNullEvent)
			_toolbox->Delay(0);
	}
	sub_128_e80();
}

void FoolPrologue::sub_128_e80() {
	// 128:0e80
	while (true) {
		var_i16_1ba = (int)_toolbox->GetNextEvent(-1, var_ev_22);
		if (var_ev_22.what == kUpdateEvt) {
			sub_128_ed2();
		}
		if (var_ev_22.what == kDiskEvt) {
			sub_128_ee0();
		}
		if (var_ev_22.what == kScummVMQuitEvt || var_ev_22.what == kScummVMReturnToLauncherEvt) {
			_quit = true;
			return;
		}
		// keep looping until mouse is seen as up??
		// see I-252
		if ((var_ev_22.modifiers & kModMouseButtonUp) && (var_ev_22.what == kNullEvent)) {
			break;
		}
		// wait until next redraw
		if (var_ev_22.what == kNullEvent)
			_toolbox->Delay(0);
	}
}

void FoolPrologue::sub_128_ed2() {
	// 128:0ed2
	_toolbox->BeginUpdate(var_window_24);
	_toolbox->EndUpdate(var_window_24);
}

void FoolPrologue::sub_128_ee0() {
	// 128:0ee0
	// done by using PEEKWORD into fixed offsets of the window pointer
	//var_i16_1bc = &var_window_24; // +0
	//var_i16_1be = &var_window_24; // +2
	//_zbasic->unk11(var_i16_1be);
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
	var_i16_10 = g_engine->_wm._screenDims.width();  // window width?
	var_i16_12 = g_engine->_wm._screenDims.height(); // window height?

	// 129:0034
	// set left and top offsets based on a drawable area of 512x342
	var_i16_14 = (var_i16_10 - SCREEN_WIDTH)/2;
	var_i16_16 = (var_i16_12 - SCREEN_HEIGHT)/2;

	// 129:0064
	_zbasic->window(1, "", 0, 0, var_i16_10, var_i16_12, kWindowDialogOneLine);
	_zbasic->coordinateWindow();

	var_i16_1c = { 0, 0, 0 };

	// code checks Rom85 for presence of 128K ROM before doing next bit
	// 129:00a2
	if (false) {
		var_i16_1c = { 0x4e20, 0x4e20, 0x4e20 };
		_toolbox->SetCPixel(0x64, 0x64, var_i16_1c);
		_toolbox->GetCPixel(0x64, 0x64, var_i16_1c);
	}

	// 129:00ee
	// used for drawing on the menu bar
	var_i32_c = &arr_grafport_a8a;
	_toolbox->OpenPort(var_i32_c);
	//var_i32_c->portBits = _toolbox->_defaultMenuBits;
	//var_i32_c->portRect = _toolbox->_defaultMenuBits->getBounds();

	var_i32_8 = &arr_grafport_9c0;
	_toolbox->OpenPort(var_i32_8);

	if ((var_i16_10 != SCREEN_WIDTH || var_i16_12 != SCREEN_HEIGHT)) {
		// 129:013a
		fillRect(0x14, 0, var_i16_12, var_i16_10, 2);
		_toolbox->SetRect(arr_i16_1bc, var_i16_14 - 2, var_i16_16 - 2, var_i16_14 + SCREEN_WIDTH + 2, var_i16_16 + SCREEN_HEIGHT + 2);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->FrameRect(arr_i16_1bc);
	}

	// 129:01b0
	_toolbox->SetPort(var_i32_8);
	_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->MovePortTo(var_i16_14, var_i16_16);
	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->ClipRect(arr_i16_1bc);

	// 129:01f2
	if (var_i16_12 == SCREEN_HEIGHT) {
		_toolbox->SetRect(arr_i16_1bc, 0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT);
		_toolbox->FillRect(arr_i16_1bc, _patterns[1]);
	} else {
		_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_toolbox->FillRect(arr_i16_1bc, _patterns[1]);
	}

	// 129:026c
	var_i32_32 = var_i32_8->portBits;
	var_i16_36 = 0x40;
	var_i16_38.top = -var_i16_16;
	var_i16_38.left = -var_i16_14;
	var_i16_38.bottom = var_i16_16 + SCREEN_HEIGHT;
	var_i16_38.right = var_i16_14 + SCREEN_WIDTH;
	var_i16_44 = 0x40;
	var_i16_46.top = 0x0;
	var_i16_46.left = 0x0;
	var_i16_46.bottom = SCREEN_HEIGHT;
	var_i16_46.right = SCREEN_WIDTH;
	var_i16_52 = 0x40;
	var_i16_54.top = 0x0;
	var_i16_54.left  = 0x0;
	var_i16_54.bottom = SCREEN_HEIGHT;
	var_i16_54.right = SCREEN_WIDTH;

	var_i32_1c0 = _zbasic->mem(-1);

	// 129:02fe
	if (var_i32_1c0 < 0xd6d8) {
		_toolbox->InitCursor();
		_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, arr_i32_41296[0], true);
		_zbasic->sound(0x19, 0x64, 0xff, 0x0);
		fillRect(0x46, 0x64, 0xdc, 0x19c, 2);
		fillRect(0x4b, 0x69, 0xd7, 0x197, 1);

		// 129:0386
		fillRect(0x50, 0x6e, 0xd2, 0x192, 2);
		_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
		// "not enough memory" message
		drawTextCenter(_zbasic->str(6), 0x100, 0x64);
		drawTextCenter(_zbasic->str(7), 0x100, 0x78);
		drawTextCenter(_zbasic->str(8), 0x100, 0x96);
		drawTextCenter(_zbasic->str(9), 0x100, 0xaa);
		drawTextCenter(_zbasic->str(10), 0x100, 0xcb);
		sub_128_e58();
		if (_quit)
			return;

		_zbasic->put(0x0, 0x14, arr_i32_41296[0], kSrcCopy);
		var_i16_1aa = 0;

	} else {
		// 129:04a0
		var_i16_180 = 1;
		// this next blob of code is for determining whether to
		// open the prologue or the finale.
		// both are in the same EXE, however the finale is accessed
		// by opening a file with the right associated application code
		// that contains the words "And now it's time to show the finale of the Fool's Errand" and a sequence of numbers.
		// we don't need to worry about files as we add a start trigger
		// in the game's menu.
		/*
		var_i16_1aa = 1;
		// 180 == number, possibly file number
		// 1c4 == string
		// 2c4 == unused
		// 2c8 == volume number?
		var_i16_176 = _zbasic->finderInfo(var_i16_180, var_i16_1c4, var_i32_2c4, var_i16_2c8);
		if (Common::U32String("") != var_i16_1c4) {
			// 129:04de
			// "and now it is time to show the finale" message
			var_str_76 = _zbasic->str(12);

			for (int i = 1; i <= 0xa; i++) {
				var_str_76 += _zbasic->chr(i);
			}

			// 129:0522
			var_i16_2ca = var_str_76.size();
			var_i16_2cc = 0;
			// 129:0534
			// what is this???
			// 0x0534: LEA - [0x0764],A0
			// 0x0538: MOVE.L - A0,-0x8ee(A5)
			// 0x053c: SF - 0x8,D0
			_zbasic->openR(1, var_i16_1c4, 0x400, var_i16_2c8);

			Handle payload = _zbasic->readFile(1, var_i16_2ca);
			var_i16_2ce = Common::U32String((char *)payload->data(), var_i16_2ca, Common::kMacRoman);
			_zbasic->close(1);
			// 0x0570: CLR.L - -0x8ee(A5)
			// 129:0582
			if ((var_i16_2ce == var_str_76) &&
				var_i16_2cc == 0) {
				var_i16_1aa = 2;
			} else {
				var_i16_1aa = 0;
			}
		}
		*/
		// 129:05a6
		if (var_i16_1c.red + var_i16_1c.blue + var_i16_1c.green != 0) {
			_toolbox->InitCursor();
			_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, arr_i32_41296[0], true);
			_zbasic->sound(0x19, 0x64, 0xff, 0x00);
			fillRect(0x64, 0x82, 0xdc, 0x17e, 2);
			fillRect(0x69, 0x87, 0xd7, 0x179, 1);
			fillRect(0x6e, 0x8c, 0xd2, 0x174, 2);
			// 129:0662
			_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcBic);
			// "set your monitor to black and white" message"
			drawTextCenter(_zbasic->str(13), 0x100, 0x82);
			drawTextCenter(_zbasic->str(14), 0x100, 0x96);

			// if this is the finale
			drawTextCenter(var_i16_1aa == 2 ? _zbasic->str(16): _zbasic->str(15), 0x100, 0xaa);

			drawTextCenter(_zbasic->str(17), 0x100, 0xcb);
			// 129:0730
			sub_128_e58();
			if (_quit)
				return;
			_zbasic->put(0x0, 0x14, arr_i32_41296[0], kSrcCopy);
			var_i16_1aa = 0;
		} else {
			// 129:075c
			sub_128_e80();
		}
		// 129:0772

	}
}

void FoolPrologue::sub_129_764() {
	// 129:0764
	var_i16_2cc = 1;
	_zbasic->unk_130(0);
}

void FoolPrologue::prologueRun() {
	// 130:0004
	var_i16_3ce = 5;
	prologueDrawLoadingMsg();
	var_i16_18e = SCREEN_HEIGHT;
	// fill a buffer with random screen y-positions, to use for the scanline transition
	shuffleScanlines();

	// Because of floppy disk read times, the game pre-loads as many PICT resources
	// as it can stuff into memory. The below process happens a bunch of times:
	// - Increment an ID by 1
	// - Call GetPicture to fetch the next PICT resource for that ID (prologueBufferNextPicture)
	// - Increment the loading progress % by 5 (prologueDrawLoadingMsg)
	// - Blit it to an offscreen page with ZBasic's PICTURE command
	// - Release the resource

	// "The Fool's Errand" title card
	glob_i32_2ce = _toolbox->GetPicture(0x47);
	var_i16_3ce = 10;
	prologueDrawLoadingMsg();
	setPortBitsToPage(0xb);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x74, 0xaa, glob_i32_2ce);
	_toolbox->ReleaseResource(glob_i32_2ce);

	// 130:007a
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, kSrcBic);
	drawTextCenter(_zbasic->str(18), 0x103, 0x10d);
	_toolbox->SetPortBits(var_i32_32);
	// cliffside images with lightning
	for (int i = 1; i <= 5; i++) {
		prologueBufferNextPicture();
	}
	setPortBitsToPage(7);

	// 130:00d6
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x96, _pics[1]);
	_toolbox->ReleaseResource(_pics[1]);
	setPortBitsToPage(8);

	// 130:011c
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0, 0x96, _pics[2]);
	_toolbox->ReleaseResource(_pics[2]);

	// 130:015c
	for (int i = 3; i <= 5; i++) {
		//_zbasic->blockMove(arr_i32_41296[7], arr_i32_41296[i], 0x5580);
		arr_i32_41296[i]->copyFrom(*arr_i32_41296[7]);
	}
	// high priestess skull
	prologueBufferNextPicture();
	setPortBitsToPage(0);

	// 130:01aa
	_zbasic->picture(0, 0, _pics[6]);
	_toolbox->ReleaseResource(_pics[6]);
	// high priestess face
	prologueBufferNextPicture();

	//_zbasic->blockMove(arr_i32_41296[0], arr_i32_41296[1], 0x5580);
	arr_i32_41296[1]->copyFrom(*arr_i32_41296[0]);
	setPortBitsToPage(1);

	// 130:0202
	_zbasic->picture(0xa2, 0x3c, _pics[7]);
	_toolbox->ReleaseResource(_pics[7]);
	// priestess on mountain
	prologueBufferNextPicture();
	setPortBitsToPage(2);

	// 130:0234
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x5a, 0x2e, _pics[8]);
	_toolbox->ReleaseResource(_pics[8]);

	// 130:0276
	for (int j = 9; j <= 0xb; j++) {
		// priestess shooting lightning
		prologueBufferNextPicture();

		if (j == 9) {
			var_i16_74 = 0;
		} else {
			var_i16_74 = 0x81;
		}

		// 130:029a
		setPortBitsToPage(j - 6);
		_zbasic->picture(0, 0, _pics[j]);
		_zbasic->picture(var_i16_74, 0x96, _pics[j - 6]);
		_toolbox->ReleaseResource(_pics[j]);
		_toolbox->ReleaseResource(_pics[j - 6]);
	}
	// priestess lightning closeup
	prologueBufferNextPicture();

	// 130:0320
	setPortBitsToPage(6);
	_zbasic->picture(0, 0, _pics[0xc]);
	_toolbox->ReleaseResource(_pics[0xc]);

	// 130:034a
	for (int i = 0xd; i <= 0x12; i++) {
		// priestess silhouette -> fool on the cliff.
		prologueBufferNextPicture();
	}

	// fool looking up at sun
	_pics[0x48] = _toolbox->GetPicture(0x48);
	_toolbox->SetPortBits(var_i32_32);

	// 130:0380
	if ((var_i16_10 == SCREEN_WIDTH) && (var_i16_12 == SCREEN_HEIGHT)) {
		fillRect(0, 0, 0x14, SCREEN_WIDTH, 1);
	} else {
		// FIXME: menu draw code not used
		/*_toolbox->SetPort(var_i32_c);
		fillRect(0, 0, 0x14, var_i16_10, 2);
		_toolbox->_defaultMenu->setOverlayDirty(true);
		_toolbox->SetPort(var_i32_8);*/
	}

	// We're done loading, start the intro.
	// Erase the screen with a bunch of cool scanlines.

	// 130:03e4
	delay(0x3c);
	scanlineTransition(0x2);
	delay(0x1e);

	// draw logo
	blitPageToScreen(0xb);
	delay(0x5a);

	// animate rain
	drawRain();

	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->SetRect(arr_i32_1c4, 0, 0x96, SCREEN_WIDTH, 0x118);

	// 130:043c
	for (int i = 1; i <= 2; i++) {
		var_i32_2 = _toolbox->TickCount();
		drawRainRecycle(0xb4);
		var_i32_2 = _toolbox->TickCount();
		_toolbox->InvertRect(arr_i16_1bc);
		delayFromMarker(1);
		_toolbox->InvertRect(arr_i16_1bc);
		// 130:04ca
		for (int j = 0; j <= 3; j++) {
			var_i32_2 = _toolbox->TickCount();
			var_i32_40 = arr_i32_41296[8];
			// 130:049e
			_toolbox->CopyBits(var_i32_40, var_i32_32, arr_i32_1c4, arr_i32_1c4, kSrcCopy, nullptr);
			drawRainRecycle(4);
			var_i16_192 = 0;
			var_i32_2 = _toolbox->TickCount();
			var_i32_40 = arr_i32_41296[3];
			_toolbox->CopyBits(var_i32_40, var_i32_32, arr_i32_1c4, arr_i32_1c4, kSrcCopy, nullptr);
			drawRainRecycle(5);
			// 130:0518
			var_i32_2 = _toolbox->TickCount();
			var_i32_40 = arr_i32_41296[4];
			_toolbox->CopyBits(var_i32_40, var_i32_32, arr_i32_1c4, arr_i32_1c4, kSrcCopy, nullptr);
			drawRainRecycle(5);
			var_i32_2 = _toolbox->TickCount();
			var_i32_40 = arr_i32_41296[5];
			_toolbox->CopyBits(var_i32_40, var_i32_32, arr_i32_1c4, arr_i32_1c4, kSrcCopy, nullptr);
			drawRainRecycle(5);
		}
		// 130:05b6
		var_i32_40 = arr_i32_41296[7];
		_toolbox->CopyBits(var_i32_40, var_i32_32, arr_i32_1c4, arr_i32_1c4, kSrcCopy, nullptr);

	}

	// 130:05fc
	blitPageToScreen(0);
	drawRain();
	var_i32_2 = _toolbox->TickCount();
	drawRainRecycle(0xb4);
	var_i32_40 = arr_i32_41296[1];

	// scanline fade in priestess face

	// 130:062c
	int y = 0;
	while (y != SCREEN_HEIGHT) {
		var_i16_5c.right = SCREEN_WIDTH;
		var_i32_2 = _toolbox->TickCount();
		for (int j = 1; j < 0x7; j++) {
			var_i16_192 = 1;
			if (y < SCREEN_HEIGHT) {
				y += 1;
			}
			var_i16_5c.top = arr_i16_412ea[y];
			var_i16_5c.bottom = arr_i16_412ea[y] + 1;
			// 130:068e
			_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_5c, var_i16_5c, kSrcCopy, nullptr);
		}
		// 130:06b0
		drawRainRecycle(1);
	}

	// lightning flash over priestess face

	var_i32_2 = _toolbox->TickCount();
	drawRainRecycle(0xa);
	blitPageToScreen(1);
	// 130:06d4
	for (int i = 1; i <= 4; i++) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->InvertRect(arr_i16_1bc);
		drawRainRecycle(0x2);
	}
	// 130:0704
	var_i32_2 = _toolbox->TickCount();
	_toolbox->InvertRect(arr_i16_1bc);
	drawRainRecycle(0xf);

	var_i32_2 = _toolbox->TickCount();
	_toolbox->InvertRect(arr_i16_1bc);
	drawRainRecycle(0x78);

	// cut to shot of cliff
	blitPageToScreen(2);

	// 130:0740
	drawRain();
	var_i32_2 = _toolbox->TickCount();
	drawRainRecycle(0xb4);

	_toolbox->SetRect(arr_i32_1c4, 0, 0, 0x150, 0x96);

	// priestess zap
	// 130:0778
	for (int j = 0; j <= 8; j++) {
		for (int i = 3; i <= 5; i++) {
			var_i32_2 = _toolbox->TickCount();
			var_i32_40 = arr_i32_41296[i];
			_toolbox->CopyBits(var_i32_40, var_i32_32, arr_i32_1c4, arr_i32_1c4, kSrcCopy, nullptr);
			drawRainRecycle(3);
		}
	}

	// cut to closeup of priestess mid-zap
	// 130:07ec
	blitPageToScreen(6);
	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int i = 0; i <= 0x2d; i++) {
		var_i32_2 = _toolbox->TickCount();
		// seizure warning
		_toolbox->InvertRect(arr_i16_1bc);
		delayFromMarker(1);
	}

	// 130:083c

	// Draw the priestess silhouette
	_zbasic->picture(0, 0, _pics[0xd]);

	// Image two in this sequence is a PICT that draws the words
	// "...for no-one can undo the treachery I have inflicted upon the land!"
	// using the ShortLine opcode to fill in random rows of pixels.
	// Because the lines are drawn in white, and the default background colour
	// is white, this is an example of a PICT that needs to be drawn to the
	// target buffer live, or with a mask.
	// On original (i.e. slow) hardware, this image takes a while to render to the
	// screen with the line tool and gives a cool fade-in effect, which we
	// simulate here.
	_pics[0xe]->_opsPerTick = 32;
	_zbasic->picture(5, 0, _pics[0xe]);

	_toolbox->ReleaseResource(_pics[0xd]);
	_toolbox->ReleaseResource(_pics[0xe]);

	// start loading some new resources from the floppy disk into the screen buffers
	var_i32_2 = _toolbox->TickCount();
	// 130:0890
	_zbasic->restore(0);
	setPortBitsToPage(0);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0x17, _pics[0xf]);
	_toolbox->ReleaseResource(_pics[0xf]);
	// 130:08dc
	prologueRenderNextText();
	drawClickMessage();
	setPortBitsToPage(1);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0xf0, 0xba, _pics[0x10]);
	_toolbox->ReleaseResource(_pics[0x10]);
	prologueRenderNextText();
	drawClickMessage();
	// 130:0934
	for (int i = 2; i <= 5; i++) {
		//_zbasic->blockMove(arr_i32_41296[1], arr_i32_41296[i], 0x5580);
		arr_i32_41296[i]->copyFrom(*arr_i32_41296[1]);
	}
	// 130:0978
	// draw the sun pulsing to pages 1-5
	for (int i = 1; i <= 5; i++) {
		setPortBitsToPage(i);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->SetRect(arr_i16_1bc, 0x3c, 0x3c, 0x8c, 0x8c);
		_toolbox->FrameOval(arr_i16_1bc);
		_toolbox->PenPat(_patterns[4]);
		// 130:09d0
		_toolbox->InsetRect(arr_i16_1bc, -i, -i);
		for (int j = 2; j <= 0xd; j++) {
			_toolbox->InsetRect(arr_i16_1bc, -j, -j);
			_toolbox->FrameOval(arr_i16_1bc);
		}
	}
	// 130:0a42
	setPortBitsToPage(6);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x166, 0x77, _pics[0x11]);
	drawClickMessage();
	//_zbasic->blockMove(arr_i32_41296[6], arr_i32_41296[7], 0x5580);
	arr_i32_41296[7]->copyFrom(*arr_i32_41296[6]);
	prologueRenderNextText();
	_toolbox->ReleaseResource(_pics[0x11]);
	setPortBitsToPage(7);
	// 130:0ac2
	_zbasic->picture(-0x14, -0x1e, _pics[0x12]);
	prologueRenderNextText();
	_toolbox->ReleaseResource(_pics[0x12]);
	setPortBitsToPage(8);
	//_zbasic->blockMove(arr_i32_41296[7], arr_i32_41296[8], 0x5580);
	arr_i32_41296[8]->copyFrom(*arr_i32_41296[7]);
	fillRect(0x73, 0x163, 0xfa, 0x1e0, 0);
	_zbasic->picture(0x16d, 0x80, _pics[0x48]);
	_toolbox->ReleaseResource(_pics[0x48]);
	_toolbox->SetPortBits(var_i32_32);

	// 130:0b74
	delayFromMarker(0xd2);
	sub_128_e80();
	if (_quit)
		return;
	scanlineTransition(1);

	// rectangle zoom into the clifftop scene

	delay(0x14);
	zoomTransition(0);

	// stall until mouse click
	sub_128_e58();
	if (_quit)
		return;
	// 130:0b98
	var_i16_74 = 0;
	var_i16_192 = 1;
	while (var_ev_22.what != kMouseDown) {
		// 130:0ba4
		var_i32_2 = _toolbox->TickCount();
		var_i16_74 += var_i16_192;
		// 130:0bbe
		if (var_i16_74 == 0x5) {
			var_i16_192 = -1;
		}
		if (var_i16_74 == 1) {
			var_i16_192 = 1;
		}
		blitPageToScreen(var_i16_74);
		delayFromMarker(0xa);
		// 130:0bee
		var_i16_1ba = _toolbox->GetNextEvent(2, var_ev_22);
	}
	while (!((var_ev_22.what == kNullEvent) && (var_ev_22.modifiers & kModMouseButtonUp))) {
		// 130:0c0c
		var_i32_2 = _toolbox->TickCount();
		var_i16_74 += var_i16_192;
		// 130:0c26
		if (var_i16_74 == 0x5) {
			var_i16_192 = -1;
		}
		if (var_i16_74 == 1) {
			var_i16_192 = 1;
		}
		blitPageToScreen(var_i16_74);
		delayFromMarker(0xa);
		// 130:0c56
		var_i16_1ba = _toolbox->GetNextEvent(-1, var_ev_22);
		if (var_ev_22.what == kUpdateEvt) {
			sub_128_ed2();
		}
		if (var_ev_22.what == kDiskEvt) {
			sub_128_ee0();
		}
	}
	// 130:0ca8
	// "who dares to interrupt my errand"
	blitPageToScreen(0x6);
	sub_128_e58();
	if (_quit)
		return;

	sub_128_50a(7, 0, SCREEN_WIDTH, 0);
	delay(0xa);
	// "I dare"
	blitPageToScreen(0x8);
	sub_128_e58();
	if (_quit)
		return;

	// zoom to close

	zoomClose(1, kPatCopy);
	// 130:0ce6
	// JMP 1002
	// _zbasic->pushOldCodeResource(0x82);
	//finaleRun();
}

void FoolPrologue::prologueBufferNextPicture() {
	// 130:0cea
	var_i16_3d4 += 1;
	_pics[var_i16_3d4] = _toolbox->GetPicture(var_i16_3d4);
	// 130:0d0a
	var_i16_3ce = var_i16_3d4 * 5 + 10;
	prologueDrawLoadingMsg();
}

void FoolPrologue::prologueDrawLoadingMsg() {
	// FIXME: we don't share the menu surface here yet
	return;
	// 130:0d28
	_toolbox->SetPort(var_i32_c);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// "loading prologue" message
	var_str_76 = _zbasic->str(19);
	var_str_76 += Common::U32String::format(" %d", var_i16_3ce);
	var_str_76 += _zbasic->str(20);
	// 130:0d70
	// white out the contents of the top menu bar.
	// the 7px offset on either side seems to be to avoid hitting the rounded screen corners?
	fillRect(0, 0x7, 0x13, var_i16_10 - 7, 0);
	drawTextCenter(var_str_76, var_i16_10/2, 0xe);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(var_i32_8);
	_toolbox->Delay(0);
}

void FoolPrologue::drawRain() {
	// 130:0db0
	_zbasic->unk_20();
	_toolbox->PenMode(kPatXor);
	var_i16_6 = 0x1;
	while (var_i16_6 <= 0xb5) {
		// 130:0dc0
		arr_i16_1e8[var_i16_6] = _zbasic->rndInt(0x264) - 0x64;
		arr_i16_1e8[var_i16_6 + 0xfb] = _zbasic->rndInt(0x1ba) - 0x64;
		arr_i16_1e8[var_i16_6 + 0x1f6] = _zbasic->rndInt(0x5) + 1;
		arr_i16_1e8[var_i16_6 + 0x2f1] = _zbasic->rndInt(0xa) + 0x19;
		// 130:0e68
		drawRainDrop();
		var_i16_6 += 1;
	}
	var_i16_6 = 1;
	_toolbox->Delay(0);
}

void FoolPrologue::drawRainDrop() {
	// 130:0e82
	_toolbox->MoveTo(arr_i16_1e8[var_i16_6], arr_i16_1e8[var_i16_6 + 0xfb]);
	// 130:0ec0
	_toolbox->LineTo(
		arr_i16_1e8[var_i16_6] + arr_i16_1e8[var_i16_6 + 0x1f6],
		arr_i16_1e8[var_i16_6 + 0xfb] + arr_i16_1e8[var_i16_6 + 0x1f6]
	);

}

void FoolPrologue::prologueRenderNextText() {
	// 130:0f48
	var_i16_3d2 = _zbasic->readDataInt();
	for (int i = 1; i <= var_i16_3d2; i++) {
		// 130:0f56
		var_i16_176 = _zbasic->readDataInt();
		var_i16_180 = _zbasic->readDataInt();
		var_str_76 = _zbasic->readDataStr();
		var_i16_1ba = 1;
		// JMP 0xfc2

		while (var_i16_1ba > 0) {
			// 130:0f78
			var_i16_1ba = _zbasic->instr(var_i16_1ba, var_str_76, _zbasic->str(21));
			if (var_i16_1ba > 0) {
				var_str_76.replace(var_i16_1ba-1, 1, Common::U32String("\""));
			}
		}
		// 130:0fc8
		_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
		drawText(var_str_76, var_i16_176, var_i16_180);
	}
}


} // End of namespace Fool
