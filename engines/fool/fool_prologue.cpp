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
#include "graphics/mactoolbox/toolbox.h"

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/fool_prologue.h"
#include "fool/zbasic.h"

#define OFF(x) (_zstrOffset[kOffsetPrologue] + (x))

namespace Fool {

// Based on m68k disassembly of the Fool's Errand v2.0, (c) 1988 Cliff Johnson.

// v1.0 - original release, single-density disks
// v2.0 - fixes full-screen rendering on higher-resolution displays
// v3.0 - newer ZBasic, changed a few graphics assets, removed custom menu font and sounds for compatibility

void FoolPrologue::run(bool finale, const BitMap &prevWindow) {
	_toolbox = new Toolbox(&g_engine->_wm);
	_zbasic = new ZBasic(_toolbox);

	Common::MacFinderInfo finfo;
	if (_toolbox->GetFInfo(Common::U32String("Prologue - Finale"), 0, finfo) == Graphics::MacToolbox::kNoErr) {
		_zbasic->loadProgram(Common::Path("Prologue - Finale", ':'));
	} else if (_toolbox->GetFInfo(Common::U32String("Prologue-Finale"), 0, finfo) == Graphics::MacToolbox::kNoErr) {
		_zbasic->loadProgram(Common::Path("Prologue-Finale", ':'));
	} else if (_toolbox->GetFInfo(Common::U32String("Prologue & Finale"), 0, finfo) == Graphics::MacToolbox::kNoErr) {
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

	setup(finale, prevWindow);
	delete _zbasic;
	_zbasic = nullptr;
	delete _toolbox;
	_toolbox = nullptr;
}

void FoolPrologue::setup(bool finale, const BitMap &prevWindow) {
	_mode = finale ? 2 : 1;

	// 128:0004
	_zbasic->coordinateWindow();
	_toolbox->ClearMenuBar();
	_toolbox->HideCursor();
	_toolbox->UseResFile(_toolbox->CurResFile());

	// 128:001e
	// b54 must be the 1-bit screen buffer
	// 41296 is an array of pointers to screen pages within the buffer
	for (int i = 0; i < 16; i++) {
		_cardSlices[i] = BitMap(new Graphics::ManagedSurface());
		_cardSlices[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	for (int i = 0; i < 12; i++) {
		//_screenPages[i] = &arr_i32_b54[SCREEN_PAGE_SIZE*i];
		_screenPages[i] = BitMap(new Graphics::ManagedSurface());
		_screenPages[i]->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	}
	_foolSlice = BitMap(new Graphics::ManagedSurface());
	_foolSlice->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	// 128:0066
	_bgPage = _screenPages[11];
	// 128:0078
	// pattern buffer
	_patterns[0] = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }; // solid white
	_patterns[1] = { { 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 } }; // checkerboard
	_patterns[2] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }; // solid black


	// 128:00d6
	_patterns[3] = { { 0x01, 0x07, 0x0e, 0x0c, 0x7c, 0x70, 0xc0, 0xc0 } };
	_patterns[4] = { { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 } };

	// 128:017a
	//var_str_76 = _zbasic->str(0);
	//var_str_76 = _zbasic->str(1);
	//var_str_76 = _zbasic->str(2);

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
	setupWindow();
	if (_quit)
		return;

	if (prevWindow) {
		_grafPtrWindow->portBits->copyFrom(*prevWindow);
	}

	_zbasic->mem(-1);
	if (_mode == 1) {
		prologueRun();
		_zbasic->mem(-1);
	}
	// 128:0cb0
	if (_mode == 2) {
		finaleRun();
		_zbasic->mem(-1);
	}
	// 128:0cc8
	//_zbasic->unk4(); // exit command?
	_toolbox->ShowCursor();
}

void FoolPrologue::copyScreenToPage(int16 screenPage) {
	// 128:01ba
	_srcPage = _screenPages[screenPage];
	_toolbox->CopyBits(_dstPage, _srcPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, nullptr);
}

void FoolPrologue::setPortBitsToPage(int16 screenPage) {
	// 128:01f4
	_srcPage = _screenPages[screenPage];
	_toolbox->SetPortBits(_srcPage);
}

void FoolPrologue::delay(int16 numTicks) {
	// 128:021e
	// original code would poll TickCount in a loop,
	// effectively the same as Delay
	_toolbox->Delay(numTicks);
}

void FoolPrologue::delayFromMarker(int16 numTicks) {
	// 128:024a
	uint32 delay = (uint32)MAX<int>(_tickMarker + numTicks + 1 - _toolbox->TickCount(), 0);
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
	Common::Rect rect;
	_toolbox->SetRect(rect, left, top, right, bottom);
	_toolbox->FillRect(rect, _patterns[patternID]);
}

void FoolPrologue::zoomClose(int16 patternID, PatternMode mode) {
	// 128:0354
	Common::Rect rect;
	_toolbox->SetRect(rect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenPat(_patterns[patternID]);
	_toolbox->PenMode(mode);
	_toolbox->PenSize(6, 4);

	for (int i = 0; i <= 130; i += 3) {
		_toolbox->FrameRect(rect);
		_toolbox->InsetRect(rect, 6, 4);
		delay(0);
	}
	_toolbox->PenNormal();
}

void FoolPrologue::drawTreasurePhaseIn(int16 offset, int16 count) {
	// 128:03ee
	int period = 0x5;
	if (count > 0x28) {
		period = 0x8;
	}
	if (count > 0x50) {
		period = 0xb;
	}
	if (count > 0x64) {
		period = 0xe;
	}
	// 128:0428
	for (int i = 1; i <= count; i++) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->MoveTo(
			_treasure[i].xLeft - offset,
			_treasure[i].yPos - offset
		);
		_toolbox->LineTo(
			_treasure[i].xRight + offset,
			_treasure[i].yPos - offset
		);
		if (i % period == 0) {
			delayFromMarker(0);
		};
	}

}

void FoolPrologue::scanlineBlitPageToScreen(int16 screenPage, int16 left, int16 right, int16 updatePeriod) {
	// 128:050a
	_srcPage = _screenPages[screenPage];
	Common::Rect srcRect; // var_i16_5c
	srcRect.left = left;
	srcRect.right = right;
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
		_tickMarker = _toolbox->TickCount();
		srcRect.top = _randScanline[i];
		srcRect.bottom = _randScanline[i] + 1;
		_toolbox->CopyBits(_srcPage, _dstPage, srcRect, srcRect, Graphics::MacToolbox::kSrcCopy, nullptr);
		if (i % updatePeriod == 0) {
			delayFromMarker(0);
		}
	}
}

void FoolPrologue::blitPageToScreen(int16 screenPage) {
	// 128:0610
	_srcPage = _screenPages[screenPage];
	_toolbox->CopyBits(_srcPage, _dstPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, nullptr);
}

void FoolPrologue::scanlineTransition(int16 patternID) {
	// 128:064a
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[patternID]);
	// 128:0668
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->MoveTo(0, _randScanline[i]);
		_toolbox->LineTo(SCREEN_WIDTH, _randScanline[i]);
		if (i % 5 == 0) {
			delayFromMarker(0);
		}
	}
	_toolbox->PenNormal();
}

void FoolPrologue::zoomTransition(int16 screenPage) {
	// 128:06e4
	_srcPage = _screenPages[screenPage];
	Common::Rect srcRect; // var_rect_5c
	for (int i = 1; i <= 0x36; i++) {
		_tickMarker = _toolbox->TickCount();
		srcRect.top = (SCREEN_HEIGHT/2) - (int)(i*3.33);
		srcRect.left = (SCREEN_WIDTH/2) - i*5;
		srcRect.bottom = (SCREEN_HEIGHT/2) + (int)(i*3.33);
		srcRect.right = (SCREEN_WIDTH/2) + i*5;
		_toolbox->CopyBits(_srcPage, _dstPage, srcRect, srcRect, Graphics::MacToolbox::kSrcCopy, nullptr);
		_toolbox->Delay(0);
	}
	_toolbox->CopyBits(_srcPage, _dstPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, nullptr);
	_toolbox->Delay(0);
}

void FoolPrologue::zoomFlash(int16 startTop, int16 startLeft, int16 startBottom, int16 startRight, int16 endTop, int16 endLeft, int16 endBottom, int16 endRight, int16 steps) {
	// 128:0800
	Common::Rect start; // arr_rect_41af4
	Common::Rect end; // arr_rect_41afc
	start.top = startTop;
	start.left = startLeft;
	start.bottom = startBottom;
	start.right = startRight;
	end.top = endTop;
	end.left = endLeft;
	end.bottom = endBottom;
	end.right = endRight;
	_toolbox->PenNormal();
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);

	// 128:08bc
	double buffer[16] = { 0 }; // arr_f64_41bbe
	buffer[0] = (double)start.top;
	buffer[1] = (double)start.left;
	buffer[2] = (double)start.bottom;
	buffer[3] = (double)start.right;
	buffer[4] = (double)(end.top - start.top)/(double)(steps);
	buffer[5] = (double)(end.left - start.left)/(double)(steps);
	buffer[6] = (double)(end.bottom - start.bottom)/(double)(steps);
	buffer[7] = (double)(end.right - start.right)/(double)(steps);

	// 128:096e
	_toolbox->PaintRect(start);
	for (int i = 1; i <= (steps - 1); i++) {
		_tickMarker = _toolbox->TickCount();

		// 128:098c
		for (int j = 0; j <= 3; j++) {
			buffer[j] += buffer[j+4];
		}
		Common::Rect intermediate; // arr_rect_41b0a
		intermediate.top = (int16)buffer[0];
		intermediate.left = (int16)buffer[1];
		intermediate.bottom = (int16)buffer[2];
		intermediate.right = (int16)buffer[3];
		// 128:0a2e
		_toolbox->PaintRect(intermediate);
		delayFromMarker(0);
	}
	_toolbox->PaintRect(end);
	_toolbox->PenNormal();
}

void FoolPrologue::drawText(const Common::U32String &str, int16 x, int16 y) {
	// 128:0a6c
	_toolbox->MoveTo(x, y);
	_toolbox->DrawString(str);
}

void FoolPrologue::drawRainRecycle(int16 ticks) {
	// 128:0a8c
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	do {
		// 128:0a96
		drawRainDrop();
		_rain[_rainIndex].xPos += _rain[_rainIndex].veloc;
		_rain[_rainIndex].yPos += _rain[_rainIndex].veloc;

		// 128:0b52
		if ((_rain[_rainIndex].xPos > 0x1f4) || (_rain[_rainIndex].yPos > 0x140)) {
		    // 128:0bae
			_rain[_rainIndex].xPos = _zbasic->rndInt(0x264) - 0xc8;
			_rain[_rainIndex].yPos = _zbasic->rndInt(0x1ba) - 0xc8;
			_rain[_rainIndex].size = _zbasic->rndInt(0x5) + 0x1;
			_rain[_rainIndex].veloc = _zbasic->rndInt(0xa) + 0x19;
		}
		// 128:0c56
		drawRainDrop();
		_rainIndex += 1;
		if (_rainIndex > 0xb5) {
			_rainIndex = 0;
			_toolbox->Delay(0);
		}
	} while (_tickMarker + ticks > _toolbox->TickCount());
	_toolbox->Delay(0);
}

void FoolPrologue::shuffleScanlines() {
	_zbasic->unk_20();
	int16 buf1[SCREEN_HEIGHT] = { 0 }; // arr_i16_41598
	int16 buf2[SCREEN_HEIGHT] = { 0 }; // arr_i16_41846
	byte *ptr1 = (byte *)&buf1[0]; // var_i32_1ac
	byte *ptr2 = (byte *)&buf2[0]; // var_i32_1b0
	// 128:0d00
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		buf1[i] = i;
	}
	// 128:0d2a
	for (int i = SCREEN_HEIGHT - 1; i >= 0; i--) {
		int16 index = _zbasic->rndInt(i + 1) - 1;
		_randScanline[i] = buf1[index];
		int16 offset = index * 2 + 2;
		_zbasic->blockMove(ptr1 + offset, ptr2 + offset, SCREEN_HEIGHT * 2 - offset);
		_zbasic->blockMove(ptr2 + offset, ptr1 + offset - 2, SCREEN_HEIGHT * 2 - offset);
	}
}

void FoolPrologue::drawClickMessage() {
	// 128:0de2
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcXor);
	// Click Mouse to Continue
	drawText(_zbasic->str(_zstrOffset[kOffsetPrologueClick]), 5, 0x151);
}

void FoolPrologue::drawClickMessageRightAlign() {
	// 128:0e1c
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcXor);
	// Click Mouse to Continue
	drawTextRight(_zbasic->str(_zstrOffset[kOffsetPrologueClick]+1), 0x1fb, 0x154);
}

void FoolPrologue::waitForClick() {
	// 128:0e58
	waitForMouseUp();
	while (true) {
		// was: 2
		_toolbox->GetNextEvent(-1, _event);
		if (_event.what == Graphics::MacToolbox::kMouseDown)
			break;
		if ((_event.what == Graphics::MacToolbox::kScummVMQuitEvt) || (_event.what == Graphics::MacToolbox::kScummVMReturnToLauncherEvt)) {
			_quit = true;
			return;
		}
		// wait until next redraw
		if (_event.what == Graphics::MacToolbox::kNullEvent)
			_toolbox->Delay(0);
	}
	waitForMouseUp();
}

void FoolPrologue::waitForMouseUp() {
	// 128:0e80
	while (true) {
		_toolbox->GetNextEvent(-1, _event);
		if (_event.what == Graphics::MacToolbox::kUpdateEvt) {
			onUpdateEvent();
		}
		if (_event.what == Graphics::MacToolbox::kDiskEvt) {
			onDiskEvent();
		}
		if (_event.what == Graphics::MacToolbox::kScummVMQuitEvt || _event.what == Graphics::MacToolbox::kScummVMReturnToLauncherEvt) {
			_quit = true;
			return;
		}
		// keep looping until mouse is seen as up??
		// see I-252
		if ((_event.modifiers & Graphics::MacToolbox::kModMouseButtonUp) && (_event.what == Graphics::MacToolbox::kNullEvent)) {
			break;
		}
		// wait until next redraw
		if (_event.what == Graphics::MacToolbox::kNullEvent)
			_toolbox->Delay(0);
	}
}

void FoolPrologue::onUpdateEvent() {
	// 128:0ed2
	_toolbox->BeginUpdate(_windowRecord);
	_toolbox->EndUpdate(_windowRecord);
}

void FoolPrologue::onDiskEvent() {
	// 128:0ee0
	// done by using PEEKWORD into fixed offsets of the window pointer
	//var_i16_1bc = &_windowRecord; // +0
	//var_i16_1be = &_windowRecord; // +2
	//_zbasic->unk11(var_i16_1be);
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

void FoolPrologue::setupWindow() {
	// 129:0004
	// get the screen width and height from QuickDraw
	// 0x0004: MOVE.L - 0x904,D0
	// 0x000a: JSR - "PEEKLONG"   # current A5
	// 0x000e: SUBI.L - 0x72,D0
	// 0x0014: JSR - "PEEKWORD" # quickdraw globals,
	_windowWidth = g_engine->_wm._screenDims.width();  // window width?
	_windowHeight = g_engine->_wm._screenDims.height(); // window height?

	// 129:0034
	// set left and top offsets based on a drawable area of 512x342
	_windowLeft = (_windowWidth - SCREEN_WIDTH)/2;
	_windowTop = (_windowHeight - SCREEN_HEIGHT)/2;

	// 129:0064
	_zbasic->window(1, "", 0, 0, _windowWidth, _windowHeight, kWindowDialogOneLine);
	_zbasic->coordinateWindow();

	RGBColor testColour = { 0, 0, 0 };

	// code checks Rom85 for presence of 128K ROM before doing next bit
	// 129:00a2
	if (false) {
		testColour = { 0x4e20, 0x4e20, 0x4e20 };
		_toolbox->SetCPixel(0x64, 0x64, testColour);
		_toolbox->GetCPixel(0x64, 0x64, testColour);
	}

	// 129:00ee
	// used for drawing on the menu bar
	_grafPtrMenu = &_grafPortMenu;
	_toolbox->OpenPort(_grafPtrMenu);
	//_grafPtrMenu->portBits = _toolbox->_defaultMenuBits;
	//_grafPtrMenu->portRect = _toolbox->_defaultMenuBits->getBounds();

	_grafPtrWindow = &_grafPortWindow;
	_toolbox->OpenPort(_grafPtrWindow);

	Common::Rect temp;
	if ((_windowWidth != SCREEN_WIDTH || _windowHeight != SCREEN_HEIGHT)) {
		// 129:013a
		fillRect(MENU_HEIGHT, 0, _windowHeight, _windowWidth, 2);
		_toolbox->SetRect(temp, _windowLeft - 2, _windowTop - 2, _windowLeft + SCREEN_WIDTH + 2, _windowTop + SCREEN_HEIGHT + 2);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->FrameRect(temp);
	}

	// 129:01b0
	_toolbox->SetPort(_grafPtrWindow);
	_toolbox->PortSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->MovePortTo(_windowLeft, _windowTop);
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->ClipRect(temp);

	// 129:01f2
	if (_windowHeight == SCREEN_HEIGHT) {
		_toolbox->SetRect(temp, 0, MENU_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
		_toolbox->FillRect(temp, _patterns[1]);
	} else {
		_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		_toolbox->FillRect(temp, _patterns[1]);
	}

	// 129:026c
	_dstPage = _grafPtrWindow->portBits;
	_pageRect.top = -_windowTop;
	_pageRect.left = -_windowLeft;
	_pageRect.bottom = _windowTop + SCREEN_HEIGHT;
	_pageRect.right = _windowLeft + SCREEN_WIDTH;

	uint32 memFree = _zbasic->mem(-1);

	// 129:02fe
	if (memFree < 0xd6d8) {
		_toolbox->InitCursor();
		_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, _screenPages[0], true);
		_zbasic->sound(0x19, 0x64, 0xff, 0x0);
		fillRect(0x46, 0x64, 0xdc, 0x19c, 2);
		fillRect(0x4b, 0x69, 0xd7, 0x197, 1);

		// 129:0386
		fillRect(0x50, 0x6e, 0xd2, 0x192, 2);
		_zbasic->text(0, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
		// "not enough memory" message
		int memoryOff = _zstrOffset[kOffsetPrologueMemory];
		drawTextCenter(_zbasic->str(memoryOff + 0), 0x100, 0x64);
		drawTextCenter(_zbasic->str(memoryOff + 1), 0x100, 0x78);
		drawTextCenter(_zbasic->str(memoryOff + 2), 0x100, 0x96);
		drawTextCenter(_zbasic->str(memoryOff + 3), 0x100, 0xaa);
		drawTextCenter(_zbasic->str(memoryOff + 4), 0x100, 0xcb);
		waitForClick();
		if (_quit)
			return;

		_zbasic->put(0x0, 0x14, _screenPages[0], Graphics::MacToolbox::kSrcCopy);
		_mode = 0;

	} else {
		// 129:04a0
		// this next blob of code is for determining whether to
		// open the prologue or the finale.
		// both are in the same EXE, however the finale is accessed
		// by opening a file with the right associated application code
		// that contains the words "And now it's time to show the finale of the Fool's Errand" and a sequence of numbers.
		// we don't need to worry about files as we add a start trigger
		// in the game's menu.
		/*
		//var_i16_180 = 1;
		_mode = 1;
		// 180 == number, possibly file number
		// 1c4 == string
		// 2c4 == unused
		// 2c8 == volume number?
		var_i16_176 = _zbasic->finderInfo(var_i16_180, var_i16_1c4, _tickMarkerc4, var_i16_2c8);
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
				_mode = 2;
			} else {
				_mode = 0;
			}
		}
		*/
		// 129:05a6
		if (testColour.red + testColour.blue + testColour.green != 0) {
			_toolbox->InitCursor();
			_zbasic->get(0x0, 0x14, SCREEN_WIDTH, SCREEN_HEIGHT, _screenPages[0], true);
			_zbasic->sound(0x19, 0x64, 0xff, 0x00);
			fillRect(0x64, 0x82, 0xdc, 0x17e, 2);
			fillRect(0x69, 0x87, 0xd7, 0x179, 1);
			fillRect(0x6e, 0x8c, 0xd2, 0x174, 2);
			// 129:0662
			_zbasic->text(0, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
			// not present in all versions of the game
			// was: str(13)
			drawTextCenter(Common::U32String("Set your monitor to"), 0x100, 0x82);
			drawTextCenter(Common::U32String("2 color black and white"), 0x100, 0x96);

			// if this is the finale
			drawTextCenter(_mode == 2 ? Common::U32String("and start the Finale again."): Common::U32String("and start the Prologue again."), 0x100, 0xaa);

			drawTextCenter(Common::U32String("(click mouse to quit)"), 0x100, 0xcb);
			// 129:0730
			waitForClick();
			if (_quit)
				return;
			_zbasic->put(0x0, 0x14, _screenPages[0], Graphics::MacToolbox::kSrcCopy);
			_mode = 0;
		} else {
			// 129:075c
			waitForMouseUp();
		}
		// 129:0772

	}
}

void FoolPrologue::unk1() {
	// 129:0764
	// var_i16_2cc = 1;
	_zbasic->unk_130(0);
}

void FoolPrologue::prologueRun() {
	// 130:0004
	_prologueLoading = 5;
	_prologuePicIndex = 0;
	prologueDrawLoadingMsg();
	//var_i16_18e = SCREEN_HEIGHT;
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
	PicHandle picTitle = _toolbox->GetPicture(0x47);
	_prologueLoading = 10;
	prologueDrawLoadingMsg();
	setPortBitsToPage(0xb);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	if (_version == kFool30) {
		_zbasic->picture(0x4c, 0xc8, picTitle);
	} else {
		_zbasic->picture(0x74, 0xaa, picTitle);
	}
	_toolbox->ReleaseResource(picTitle);

	// 130:007a
	_zbasic->text(kPrologueFontSmall, 0x9, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	if (_version == kFool30) {
		drawTextCenter(_zbasic->str(OFF(0)), 0x103, 0x112);
	} else {
		drawTextCenter(_zbasic->str(OFF(0)), 0x103, 0x10d);
	}
	_toolbox->SetPortBits(_dstPage);
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
		_screenPages[i]->copyFrom(*_screenPages[7]);
	}
	// high priestess skull
	prologueBufferNextPicture();
	setPortBitsToPage(0);

	// 130:01aa
	_zbasic->picture(0, 0, _pics[6]);
	_toolbox->ReleaseResource(_pics[6]);
	// high priestess face
	prologueBufferNextPicture();

	_screenPages[1]->copyFrom(*_screenPages[0]);
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

		int16 xPos = (j == 9) ? 0 : 0x81;

		// 130:029a
		setPortBitsToPage(j - 6);
		_zbasic->picture(0, 0, _pics[j]);
		_zbasic->picture(xPos, 0x96, _pics[j - 6]);
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
	_toolbox->SetPortBits(_dstPage);

	// 130:0380
	if ((_windowWidth == SCREEN_WIDTH) && (_windowHeight == SCREEN_HEIGHT)) {
		fillRect(0, 0, 0x14, SCREEN_WIDTH, 1);
	} else {
		// NOTE: menu draw code not used
		_toolbox->SetPort(_grafPtrMenu);
		fillRect(0, 0, 0x14, _windowWidth, 2);
		_toolbox->_defaultMenu->setOverlayDirty(true);
		_toolbox->SetPort(_grafPtrWindow);
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

	Common::Rect temp1; // arr_rect_1bc
	Common::Rect temp2; // arr_rect_1c4
	_toolbox->SetRect(temp1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->SetRect(temp2, 0, 0x96, SCREEN_WIDTH, 0x118);

	// 130:043c
	for (int i = 1; i <= 2; i++) {
		_tickMarker = _toolbox->TickCount();
		drawRainRecycle(0xb4);
		// 130:0456
		_tickMarker = _toolbox->TickCount();
		_toolbox->InvertRect(temp1);
		delayFromMarker(1);
		_toolbox->InvertRect(temp1);
		_tickMarker = _toolbox->TickCount();
		_srcPage = _screenPages[8];
		// 130:049e
		_toolbox->CopyBits(_srcPage, _dstPage, temp2, temp2, Graphics::MacToolbox::kSrcCopy, nullptr);
		drawRainRecycle(4);

		for (int j = 0; j <= 3; j++) {
			// 130:04ca
			_tickMarker = _toolbox->TickCount();
			_srcPage = _screenPages[3];
			_toolbox->CopyBits(_srcPage, _dstPage, temp2, temp2, Graphics::MacToolbox::kSrcCopy, nullptr);
			drawRainRecycle(5);
			// 130:0518
			_tickMarker = _toolbox->TickCount();
			_srcPage = _screenPages[4];
			_toolbox->CopyBits(_srcPage, _dstPage, temp2, temp2, Graphics::MacToolbox::kSrcCopy, nullptr);
			drawRainRecycle(5);
			_tickMarker = _toolbox->TickCount();
			_srcPage = _screenPages[5];
			_toolbox->CopyBits(_srcPage, _dstPage, temp2, temp2, Graphics::MacToolbox::kSrcCopy, nullptr);
			drawRainRecycle(5);
		}
		// 130:05b6
		_srcPage = _screenPages[7];
		_toolbox->CopyBits(_srcPage, _dstPage, temp2, temp2, Graphics::MacToolbox::kSrcCopy, nullptr);

	}

	// 130:05fc
	blitPageToScreen(0);
	drawRain();
	_tickMarker = _toolbox->TickCount();
	drawRainRecycle(0xb4);
	_srcPage = _screenPages[1];

	// scanline fade in priestess face

	// 130:062c
	int y = 0;
	while (y != SCREEN_HEIGHT) {
		Common::Rect scanline;
		scanline.left = 0;
		scanline.right = SCREEN_WIDTH;
		_tickMarker = _toolbox->TickCount();
		for (int j = 1; j < 0x7; j++) {
			if (y < SCREEN_HEIGHT) {
				y += 1;
			}
			scanline.top = _randScanline[y];
			scanline.bottom = _randScanline[y] + 1;
			// 130:068e
			_toolbox->CopyBits(_srcPage, _dstPage, scanline, scanline, Graphics::MacToolbox::kSrcCopy, nullptr);
		}
		// 130:06b0
		drawRainRecycle(1);
	}

	// lightning flash over priestess face

	_tickMarker = _toolbox->TickCount();
	drawRainRecycle(0xa);
	blitPageToScreen(1);
	// 130:06d4
	for (int i = 1; i <= 4; i++) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->InvertRect(temp1);
		drawRainRecycle(0x2);
	}
	// 130:0704
	_tickMarker = _toolbox->TickCount();
	_toolbox->InvertRect(temp1);
	drawRainRecycle(0xf);

	_tickMarker = _toolbox->TickCount();
	_toolbox->InvertRect(temp1);
	drawRainRecycle(0x78);

	// cut to shot of cliff
	blitPageToScreen(2);

	// 130:0740
	drawRain();
	_tickMarker = _toolbox->TickCount();
	drawRainRecycle(0xb4);

	_toolbox->SetRect(temp2, 0, 0, 0x150, 0x96);

	// priestess zap
	// 130:0778
	for (int j = 0; j <= 8; j++) {
		for (int i = 3; i <= 5; i++) {
			_tickMarker = _toolbox->TickCount();
			_srcPage = _screenPages[i];
			_toolbox->CopyBits(_srcPage, _dstPage, temp2, temp2, Graphics::MacToolbox::kSrcCopy, nullptr);
			drawRainRecycle(3);
		}
	}

	// cut to closeup of priestess mid-zap
	// 130:07ec
	blitPageToScreen(6);
	Common::Rect temp;
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int i = 0; i <= 41; i++) { // was: 45
		_tickMarker = _toolbox->TickCount();
		// limit flashing to WCAG recommendation of 6/second
		if (((i % 10) == 0) || ((i % 10) == 2)) {
			_toolbox->InvertRect(temp);
		}
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
	_tickMarker = _toolbox->TickCount();
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
		_screenPages[i]->copyFrom(*_screenPages[1]);
	}
	// 130:0978
	// draw the sun pulsing to pages 1-5
	for (int i = 1; i <= 5; i++) {
		setPortBitsToPage(i);
		_toolbox->PenPat(_patterns[1]);
		_toolbox->SetRect(temp1, 0x3c, 0x3c, 0x8c, 0x8c);
		_toolbox->FrameOval(temp1);
		_toolbox->PenPat(_patterns[4]);
		// 130:09d0
		_toolbox->InsetRect(temp1, -i, -i);
		for (int j = 2; j <= 0xd; j++) {
			_toolbox->InsetRect(temp1, -j, -j);
			_toolbox->FrameOval(temp1);
		}
	}
	// 130:0a42
	setPortBitsToPage(6);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x166, 0x77, _pics[0x11]);
	drawClickMessage();
	_screenPages[7]->copyFrom(*_screenPages[6]);
	prologueRenderNextText();
	_toolbox->ReleaseResource(_pics[0x11]);
	setPortBitsToPage(7);
	// 130:0ac2
	_zbasic->picture(-0x14, -0x1e, _pics[0x12]);
	prologueRenderNextText();
	_toolbox->ReleaseResource(_pics[0x12]);
	setPortBitsToPage(8);
	_screenPages[8]->copyFrom(*_screenPages[7]);
	fillRect(0x73, 0x163, 0xfa, 0x1e0, 0);
	_zbasic->picture(0x16d, 0x80, _pics[0x48]);
	_toolbox->ReleaseResource(_pics[0x48]);
	_toolbox->SetPortBits(_dstPage);

	// 130:0b74
	delayFromMarker(0xd2);
	waitForMouseUp();
	if (_quit)
		return;
	scanlineTransition(1);

	// rectangle zoom into the clifftop scene

	delay(0x14);
	zoomTransition(0);

	// stall until mouse click
	waitForClick();
	if (_quit)
		return;
	// 130:0b98
	int16 pageIndex = 0;
	int16 increment = 1;
	while (_event.what != Graphics::MacToolbox::kMouseDown) {
		// 130:0ba4
		_tickMarker = _toolbox->TickCount();
		pageIndex += increment;
		// 130:0bbe
		if (pageIndex == 0x5) {
			increment = -1;
		}
		if (pageIndex == 1) {
			increment = 1;
		}
		blitPageToScreen(pageIndex);
		delayFromMarker(0xa);
		// 130:0bee
		_toolbox->GetNextEvent(2, _event);
	}
	while (!((_event.what == Graphics::MacToolbox::kNullEvent) && (_event.modifiers & Graphics::MacToolbox::kModMouseButtonUp))) {
		// 130:0c0c
		_tickMarker = _toolbox->TickCount();
		pageIndex += increment;
		// 130:0c26
		if (pageIndex == 0x5) {
			increment = -1;
		}
		if (pageIndex == 1) {
			increment = 1;
		}
		blitPageToScreen(pageIndex);
		delayFromMarker(0xa);
		// 130:0c56
		_toolbox->GetNextEvent(-1, _event);
		if (_event.what == Graphics::MacToolbox::kUpdateEvt) {
			onUpdateEvent();
		}
		if (_event.what == Graphics::MacToolbox::kDiskEvt) {
			onDiskEvent();
		}
	}
	// 130:0ca8
	// "who dares to interrupt my errand"
	blitPageToScreen(0x6);
	waitForClick();
	if (_quit)
		return;

	scanlineBlitPageToScreen(7, 0, SCREEN_WIDTH, 0);
	delay(0xa);
	// "I dare"
	blitPageToScreen(0x8);
	waitForClick();
	if (_quit)
		return;

	// zoom to close

	zoomClose(1, Graphics::MacToolbox::kPatCopy);
	// 130:0ce6
	// JMP 1002
	// _zbasic->pushOldCodeResource(0x82);
	//finaleRun();
}

void FoolPrologue::prologueBufferNextPicture() {
	// 130:0cea
	_prologuePicIndex += 1;
	_pics[_prologuePicIndex] = _toolbox->GetPicture(_prologuePicIndex);
	// 130:0d0a
	_prologueLoading = _prologuePicIndex * 5 + 10;
	prologueDrawLoadingMsg();
}

void FoolPrologue::prologueDrawLoadingMsg() {
	// FIXME: we don't share the menu surface here yet
	return;
	// 130:0d28
	_toolbox->SetPort(_grafPtrMenu);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcOr);
	// "loading prologue" message
	Common::U32String msg = _zbasic->str(OFF(1));
	msg += Common::U32String::format(" %d", _prologueLoading);
	msg += _zbasic->str(OFF(2));
	// 130:0d70
	// white out the contents of the top menu bar.
	// the 7px offset on either side seems to be to avoid hitting the rounded screen corners?
	fillRect(0, 0x7, 0x13, _windowWidth - 7, 0);
	drawTextCenter(msg, _windowWidth/2, 0xe);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(_grafPtrWindow);
	_toolbox->Delay(0);
}

void FoolPrologue::drawRain() {
	// 130:0db0
	_zbasic->unk_20();
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	_rainIndex = 0x1;
	while (_rainIndex <= 0xb5) {
		// 130:0dc0
		_rain[_rainIndex].xPos = _zbasic->rndInt(0x264) - 0x64;
		_rain[_rainIndex].yPos = _zbasic->rndInt(0x1ba) - 0x64;
		_rain[_rainIndex].size = _zbasic->rndInt(0x5) + 1;
		_rain[_rainIndex].veloc = _zbasic->rndInt(0xa) + 0x19;
		// 130:0e68
		drawRainDrop();
		_rainIndex += 1;
	}
	_rainIndex = 1;
	_toolbox->Delay(0);
}

void FoolPrologue::drawRainDrop() {
	// 130:0e82
	_toolbox->MoveTo(_rain[_rainIndex].xPos, _rain[_rainIndex].yPos);
	// 130:0ec0
	_toolbox->LineTo(
		_rain[_rainIndex].xPos + _rain[_rainIndex].size,
		_rain[_rainIndex].yPos + _rain[_rainIndex].size
	);

}

void FoolPrologue::prologueRenderNextText() {
	// 130:0f48
	int16 count = _zbasic->readDataInt();
	for (int i = 1; i <= count; i++) {
		// 130:0f56
		int16 xPos = _zbasic->readDataInt();
		int16 yPos = _zbasic->readDataInt();
		Common::U32String text = _zbasic->readDataStr();
		int16 caratPos = 1;
		// JMP 0xfc2

		while (caratPos > 0) {
			// 130:0f78
			caratPos = _zbasic->instr(caratPos, text, Common::U32String("^")); // was: str(21)
			if (caratPos > 0) {
				text.replace(caratPos - 1, 1, Common::U32String("\""));
			}
		}
		// 130:0fc8
		_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcOr);
		drawText(text, xPos, yPos);
	}
}


} // End of namespace Fool
