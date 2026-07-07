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
#include "common/ustr.h"
#include "graphics/mactoolbox/toolbox.h"

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/fool_prologue.h"
#include "fool/zbasic.h"

#define OFF(x) (_zstrOffset[kOffsetFinale] + (x))

namespace Fool {

// run finale
void FoolPrologue::finaleRun() {


	// 131:0004
	_prologueLoading = 45;
	finaleDrawLoadingMsg();
	PolyHandle zapShape = _toolbox->OpenPoly(); // var_i32_3d6
	_toolbox->MoveTo(0xc8, 0x8d);
	_toolbox->LineTo(0x1df, 0xc8);
	_toolbox->LineTo(0x1d2, 0x11b);
	_toolbox->LineTo(0xcd, 0x93);
	_toolbox->LineTo(0xc8, 0x8d);
	_toolbox->ClosePoly();
	// 131:004c
	_pics[0x1f] = _toolbox->GetPicture(0x1f);
	setPortBitsToPage(0x4);
	_zbasic->picture(0x0, 0x0, _pics[0x1f]);
	_toolbox->ReleaseResource(_pics[0x1f]);
	finaleDrawLoadingMsg();
	for (int i = 0x14; i <= 0x1e; i++) {
		// 131:009c
		_pics[i] = _toolbox->GetPicture(i);
		if ((i % 3) == 0) {
			_prologueLoading += 5;
			finaleDrawLoadingMsg();
		}
	}
	// 131:00e6
	for (int i = 6; i <= 9; i++) {
		setPortBitsToPage(i);
		fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
		// 131:0110
		if (i < 8) {
			_zbasic->picture(0, 0x3, _pics[0x19]);
		}
		if (i == 8) {
			_zbasic->picture(0, 0, _pics[0x1a]);
		}
		if (i == 9) {
			_zbasic->picture(0x60, 0x4c, _pics[0x1b]);
		}
	}
	// 131:0184
	for (int i = 0x19; i <= 0x1b; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
	// 131:01aa
	_pics[0x13] = _toolbox->GetPicture(0x13);
	setPortBitsToPage(0x5);
	_zbasic->picture(0, 0xb4, _pics[0x13]);
	_zbasic->picture(0, 0, _pics[0x18]);
	_toolbox->ReleaseResource(_pics[0x13]);
	_toolbox->ReleaseResource(_pics[0x18]);
	finaleStartText();
	_prologueLoading = 75;
	finaleDrawLoadingMsg();
	// 131:021e
	for (int i = 0x20; i <= 0x2f; i++) {
		_pics[i] = _toolbox->GetPicture(i);
		if (i % 3) {
			_prologueLoading += 5;
			finaleDrawLoadingMsg();
		}
	}
	// 131:026e
	//var_i16_18e = SCREEN_HEIGHT;
	shuffleScanlines();
	_zbasic->mem(-1);
	_toolbox->SetPortBits(_dstPage);
	if (!((_windowWidth == SCREEN_WIDTH) && (_windowHeight == SCREEN_HEIGHT))) {
		// 131:02ac
		fillRect(0, 0, MENU_HEIGHT, SCREEN_WIDTH, 1);
	} else {
		_toolbox->SetPort(_grafPtrMenu);
		fillRect(0, 0, MENU_HEIGHT, _windowWidth, 2);
		_toolbox->_defaultMenu->setOverlayDirty(true);
		_toolbox->SetPort(_grafPtrWindow);
	}
	// 131:02ec
	delay(0x3c);
	zoomTransition(0x5);
	waitForClick();
	if (_quit)
		return;

	// xor out the start text
	finaleStartText();
	_zbasic->restore(0x4c);
	for (int j = 1; j <= 0xe; j++) {

		// 131:030c
		int16 count = _zbasic->readDataInt();
		for (int i = 1; i <= count; i++) {
			// 131:031a
			_treasure[i].xLeft = _zbasic->readDataInt();
			_treasure[i].xRight = _zbasic->readDataInt();
			_treasure[i].yPos = _zbasic->readDataInt();
		}

		// 131:0380
		_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
		drawTreasurePhaseIn(0x14, count);
		drawTreasurePhaseIn(0xa, count);
		drawTreasurePhaseIn(0x14, count);
		drawTreasurePhaseIn(0x5, count);
		drawTreasurePhaseIn(0xa, count);
		drawTreasurePhaseIn(0x5, count);
		_toolbox->PenNormal();
		drawTreasurePhaseIn(0x0, count);
	}

	// 131:03c0
	delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcOr);

	// 131:0400
	// well, that's that!
	drawText(Common::U32String::format("\"%s\"", _zbasic->str(OFF(0)).encode().c_str()), 0x17c, 0xb4);
	delay(0x1e);

	// 131:041a
	// fool nods and puts away map
	_zbasic->picture(0x1cd, 0xbd, _pics[0x14]);
	delay(0x2);
	_zbasic->picture(0x1cd, 0xbf, _pics[0x15]);
	delay(0x3c);
	_zbasic->picture(0x1cd, 0xbd, _pics[0x14]);
	delay(0x6);
	_zbasic->picture(0x1a6, 0xbf, _pics[0x16]);
	delay(0x6);
	_zbasic->picture(0x1b3, 0xc5, _pics[0x17]);
	delay(0);

	// 131:04b4
	for (int i = 0x14; i <= 0x17; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}

	// 131:04da
	_tickMarker = _toolbox->TickCount();

	_srcPage = _screenPages[5];
	_toolbox->CopyBits(_dstPage, _srcPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, nullptr);
	setPortBitsToPage(5);
	fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	finaleDrawFoolUhOh();

	// 131:0538
	_toolbox->CopyBits(_dstPage, _bgPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, nullptr);
	_toolbox->SetPortBits(_bgPage);

	// 131:0554
	fillRect(0, 0, 0xa0, 0x15e, 0);
	fillRect(0xa0, 0x5a, 0xd2, 0xaa, 0);
	fillRect(0xaa, 0x177, 0xb9, 0x200, 0);

	// 131:05b0
	setPortBitsToPage(6);
	_toolbox->CopyBits(_bgPage, _srcPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcOr, nullptr);
	setPortBitsToPage(7);
	_toolbox->CopyBits(_bgPage, _srcPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcOr, nullptr);
	setPortBitsToPage(8);
	_toolbox->CopyBits(_bgPage, _srcPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcOr, nullptr);
	setPortBitsToPage(9);
	_toolbox->CopyBits(_bgPage, _srcPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcOr, nullptr);

	// 131:0630
	// tree morphing into priestess
	_toolbox->SetPortBits(_dstPage);
	_zbasic->get(0x1bc, 0xc1, 0x1f2, 0x11b, _foolSlice);
	delayFromMarker(0x3c);
	fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	scanlineBlitPageToScreen(0x6, 0, SCREEN_WIDTH, 0x3);
	finaleDrawFoolUhOh();

	// 131:06aa
	scanlineBlitPageToScreen(5, 0, SCREEN_WIDTH, 2);
	scanlineBlitPageToScreen(7, 0, SCREEN_WIDTH, 2);
	scanlineBlitPageToScreen(8, 0, SCREEN_WIDTH, 1);
	fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	scanlineBlitPageToScreen(0x9, 0, SCREEN_WIDTH, 1);

	// the wind picks up
	_toolbox->PenNormal();
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	_zbasic->unk_20();

	// 131:0730
	int16 windHeight = 0; // windHeight
	int16 windCount = 0;
	for (int j = 1; j <= 0x63; j++) {
		if (windHeight < SCREEN_HEIGHT) {
			// 131:074e
			windHeight += _zbasic->maybe() ? 0x4 : 0x5;
		}
		if (windCount < 0x41) {
			// 131:0770
			windCount++;
			_rain[windCount].xPos = _zbasic->rndInt(32);
			_rain[windCount].yPos = _zbasic->rndInt(windHeight);
			_rain[windCount].size = _zbasic->rndInt(0xa) + 0xa;
			_rain[windCount].veloc = _rain[windCount].size*2;
			finaleDrawWind(windCount);
		}
		// 131:07ea
		for (int i = 1; i <= windCount; i++) {
			finaleDrawWind(i);
			_rain[i].xPos += _rain[i].veloc;

			// 131:0870
			if (_rain[i].xPos > SCREEN_WIDTH) {
				_rain[i].xPos = 1;
				_rain[i].yPos = _zbasic->rndInt(windHeight);
				_rain[i].size = _zbasic->rndInt(0xa) + 0xa;
				_rain[i].veloc = _rain[i].size*2;
			}

			// 131:0902
			finaleDrawWind(i);
		}
		// Simulate slow redraw
		_toolbox->Delay(0);
	}


	// 131:0926
	Common::Rect temp;
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenPat(_patterns[3]);
	_zbasic->picture(0x6d, 0x49, _pics[0x1c]);
	_zbasic->picture(0x65, 0x48, _pics[0x1d]);
	_zbasic->picture(0x64, 0x54, _pics[0x1e]);
	// 131:099c

	// zzzzap
	_toolbox->PaintPoly(zapShape);
	for (int i = 0; i <= 0x21; i++) {
		_tickMarker = _toolbox->TickCount();
		// limit flashing to WCAG recommendation
		if ((i % 10) == 0) {
			_toolbox->InvertRect(temp);
		}
		delayFromMarker(0);
	}

	// 131:09d2
	zoomFlash(0xc1, 0x1bc, 0x11b, 0x1f2, 0x0, 0x0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	zoomFlash(0x156, 0, 0x156, 0, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	zoomFlash(0, 0, 0, 0, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	zoomFlash(0, SCREEN_WIDTH, 0, SCREEN_WIDTH, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);

	// 131:0a7a
	// horrible flashing fool duplication spree
	for (int i = 1; i <= 0x63; i++) {
		_tickMarker = _toolbox->TickCount();
		_zbasic->put(_zbasic->rndInt(0x1ca), _zbasic->rndInt(0xfc), _foolSlice, (i % 2) ? Graphics::MacToolbox::kSrcCopy : Graphics::MacToolbox::kNotSrcCopy);
		// limit flashing to WCAG recommendation
		if ((i % 10) == 0) {
			_toolbox->InvertRect(temp);
		}
		delayFromMarker(0);
	}

	// 131:0aec
	// scrumble the contents of the screen for a few seconds
	_tickMarker = _toolbox->TickCount();

	// 131:0af6
	while (_toolbox->TickCount() < (_tickMarker + 0x78)) {
		int16 xPos = _zbasic->rndInt(0x1ca);
		int16 yPos = _zbasic->rndInt(0xfc);
		_zbasic->get(xPos, yPos, xPos + 0x36, yPos + 0x5a, _foolSlice);

		// 131:0b54
		xPos = _zbasic->rndInt(0x1ca);
		yPos = _zbasic->rndInt(0xfc);
		_zbasic->put(xPos, yPos, _foolSlice, Graphics::MacToolbox::kSrcCopy);
		_toolbox->Delay(0);
	}

	// 131:0bb0
	// fade in to seascape
	scanlineBlitPageToScreen(4, 0, SCREEN_WIDTH, 3);
	setPortBitsToPage(5);
	copyScreenToPage(5);
	// fade in the fool
	_zbasic->picture(0x198, 0x102, _pics[0x25]);
	_toolbox->SetPortBits(_dstPage);
	delay(0x3c);
	scanlineBlitPageToScreen(5, 0, SCREEN_WIDTH, 0);

	// fool looks around
	_zbasic->picture(0x197, 0x101, _pics[0x26]);

	// 131:0c36
	_tickMarker = _toolbox->TickCount();
	_toolbox->KillPoly(zapShape);
	for (int i = 0x1c; i < 0x1e; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}

	// 131:0c6c
	// high priestess becomes giant
	for (int i = 0x6; i <= 0xa; i++) {
		_screenPages[i]->copyFrom(*_screenPages[5]);
	}
	setPortBitsToPage(0x6);
	_zbasic->picture(0x6, 0x8f, _pics[0x20]);
	setPortBitsToPage(0x7);
	_zbasic->picture(0x1a, 0x7e, _pics[0x21]);
	setPortBitsToPage(0x8);
	_zbasic->picture(0x2e, 0x6a, _pics[0x22]);
	setPortBitsToPage(0x9);
	_zbasic->picture(0x45, 0x4e, _pics[0x23]);
	setPortBitsToPage(0xa);
	_zbasic->picture(0x73, 0x28, _pics[0x24]);

	// 131:0d64
	for (int i = 0x20; i <= 0x23; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0x64);
	_zbasic->picture(0x198, 0x102, _pics[0x25]);
	for (int i = 0x6; i <= 0xa; i++) {
		delay(0xf);
		scanlineBlitPageToScreen(i, 0, SCREEN_WIDTH, 1);
	}
	_zbasic->picture(0x191, 0x102, _pics[0x27]);
	delay(0x3c);

	// 131:0e08
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	// Are you still angry with me?
	Common::U32String dialog = Common::U32String::format("\"%s\"", _zbasic->str(OFF(1)).encode().c_str());
	drawText(
		dialog,
		0x1f4 - _toolbox->StringWidth(dialog),
		0xd2
	);
	_tickMarker = _toolbox->TickCount();

	// 131:0e76
	_screenPages[5]->copyFrom(*_screenPages[4]);
	setPortBitsToPage(0x5);
	_zbasic->picture(0x73, 0x28, _pics[0x24]);
	_zbasic->picture(0x191, 0x102, _pics[0x27]);
	_zbasic->picture(0x8a, 0x24, _pics[0x28]);
	setPortBitsToPage(0x6);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x118, 0x19, _pics[0x29]); // wadjet eye mask
	setPortBitsToPage(0x7);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0x118, 0x19, _pics[0x2a]); // wadjet eye
	_screenPages[8]->copyFrom(*_screenPages[5]);
	setPortBitsToPage(0x8);
	_zbasic->picture(0x18f, 0xc0, _pics[0x2b]);
	_screenPages[9]->copyFrom(*_screenPages[4]);
	setPortBitsToPage(0x9);
	_zbasic->picture(0x18f, 0xc0, _pics[0x2b]);
	setPortBitsToPage(0xa);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x18f, 0xc0, _pics[0x2b]);

	// 131:1056
	_toolbox->ReleaseResource(_pics[0x24]);
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0x78);
	fillRect(0xc8, 0x145, 0xd7, SCREEN_WIDTH, 2);
	scanlineBlitPageToScreen(0x5, 0x64, 0x19c, 0x0);
	delay(0x28);

	Common::Rect srcRect; // var_rect_5c
	Common::Rect dstRect; // var_rect_64
	srcRect.left = 0x118;
	srcRect.right = 0x1d1;
	srcRect.top = 0x19;
	dstRect.left = 0x118;
	dstRect.right = 0x1d1;
	dstRect.bottom = 0xdf;

	// wadjet eye appears from horizon
	for (int i = 0xdc; i >= 0x19; i -= 5) {
		// 131:10de
		_tickMarker = _toolbox->TickCount();
		_bgPage->copyFrom(*_screenPages[5]);
		dstRect.top = i;
		srcRect.bottom = 0x19 + (0xdf - i);
		// 131:1124
		_srcPage = _screenPages[6];
		_toolbox->CopyBits(_srcPage, _bgPage, srcRect, dstRect, Graphics::MacToolbox::kSrcBic, nullptr);
		_srcPage = _screenPages[7];
		_toolbox->CopyBits(_srcPage, _bgPage, srcRect, dstRect, Graphics::MacToolbox::kSrcOr, nullptr);
		delayFromMarker(0x2);
		_toolbox->CopyBits(_bgPage, _dstPage, dstRect, dstRect, Graphics::MacToolbox::kSrcCopy, nullptr);
	}
	_toolbox->Delay(0);

	// 131:11a4
	copyScreenToPage(0x5);
	setPortBitsToPage(0x5);
	fillRect(0xc8, 0x145, 0xd7, SCREEN_WIDTH, 2);
	_zbasic->picture(0x18f, 0xc0, _pics[0x2b]); // fool card
	_toolbox->SetPortBits(_dstPage);
	_toolbox->PenNormal();
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	_toolbox->PenSize(0x5, 0x5);

	// 131:1208
	for (int i = 0; i <= 0x1b8; i += 6) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->MoveTo(0x172, 0x33);
		_toolbox->LineTo(i, 0x14a);
		if (i == 0x4e) {
			_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
			// yes, apparently so
			drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(2)).encode().c_str()), 0x1f4, 0xd2);
			_zbasic->picture(0x198, 0x102, _pics[0x25]);
		}
		delayFromMarker(0x2);
		_toolbox->MoveTo(0x172, 0x33);
		_toolbox->LineTo(i, 0x14a);
	}

	// 131:12cc
	zoomFlash(0x14a, 0x1b8, 0x14a, 0x1b8, 0xc0, 0x18f, 0x156, 0x1e6, 0x10);
	// get turned into card
	scanlineBlitPageToScreen(0x5, 0x12c, SCREEN_WIDTH, 0x1);
	delay(0xec);

	// fade out eye, then priestess, then seascape
	for (int i = 8; i <= 0xa; i++) {
		scanlineBlitPageToScreen(i, 0, SCREEN_WIDTH, 1);
	}

	_tickMarker = _toolbox->TickCount();
	setPortBitsToPage(0x5);

	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0, _pics[0x2f]);
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0xa);
	srcRect.left = 0x18b;
	srcRect.right = 0x1ef;
	dstRect.left = 0x18b;
	dstRect.right = 0x1ef;

	// 131:13b8
	// move the fool card up the screen
	for (int i = 0xc0; i >= 0x15; i--) {
		_tickMarker = _toolbox->TickCount();
		srcRect.top = i;
		srcRect.bottom = srcRect.top + 0x96;
		dstRect.top = i - 1;
		dstRect.bottom = dstRect.top + 0x96;
		_toolbox->CopyBits(_dstPage, _dstPage, srcRect, dstRect, Graphics::MacToolbox::kSrcCopy, nullptr);
		delayFromMarker(0);
	}

	// 131:1424
	// fool turns
	_zbasic->picture(0x193, 0x2d, _pics[0x2c]);
	delay(0x1);
	_zbasic->picture(0x193, 0x2d, _pics[0x2d]);
	delay(0x1);
	_zbasic->picture(0x193, 0x2d, _pics[0x2e]);
	copyScreenToPage(0x6);
	_toolbox->PenNormal();
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	_toolbox->PenSize(0x3, 0x3);

	Common::Rect bindleRect; // var_i16_6c
	bindleRect.left = 0x190;
	bindleRect.right = SCREEN_WIDTH;
	bindleRect.top = 0x50;
	bindleRect.bottom = SCREEN_HEIGHT;
	srcRect.left = 0;
	srcRect.right = 0x32;
	srcRect.top = 0;
	srcRect.bottom = 0x32;
	int16 bindleHeight = 0x64;

	// 131:14d8
	// fool drops bindle
	_srcPage = _screenPages[5];
	_toolbox->SetPortBits(_bgPage);
	for (int i = 1; i <= 0xf; i++) {
		_tickMarker = _toolbox->TickCount();
		_bgPage->copyFrom(*_screenPages[6]);
		bindleHeight += 5 + i*2;
		dstRect.left = 0x1e0;
		dstRect.right = dstRect.left + 0x32;
		dstRect.top = bindleHeight;
		dstRect.bottom = dstRect.top + 0x32;
		_toolbox->CopyBits(_srcPage, _bgPage, srcRect, dstRect, Graphics::MacToolbox::kSrcXor, nullptr);
		// 131:1582
		_toolbox->MoveTo(0x190 - i, bindleHeight - 0x1e + i*3);
		_toolbox->LineTo(0x1f4, bindleHeight + 0xa - i);
		_toolbox->CopyBits(_bgPage, _dstPage, bindleRect, bindleRect, Graphics::MacToolbox::kSrcCopy, nullptr);
		delayFromMarker(0x1);
	}

	_toolbox->SetPortBits(_dstPage);
	_toolbox->PenNormal();
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	// 131:1606
	Particle splash[251];
	for (int j = 0; j <= 1; j++) {

		for (int i = 1; i <= 0xfa; i++) {
			_tickMarker = _toolbox->TickCount();

			if (j == 0) {
				splash[i].xPos = 0x1f4 - i + _zbasic->rndInt(i);
				splash[i].yPos =  SCREEN_HEIGHT - _zbasic->rndInt(i);
			}
			// 131:1692
			_toolbox->MoveTo(splash[i].xPos, splash[i].yPos);
			_toolbox->LineTo(splash[i].xPos, splash[i].yPos);
			if (i % 0x19 == 0) {
				delayFromMarker(0x1);
			}
		}
	}
	// 131:1748
	_tickMarker = _toolbox->TickCount();
	for (int i = 0x25; i <= 0x2f; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}

	delayFromMarker(0x3c);
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcBic);
	// well, this won't do
	drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(3)).encode().c_str()), 0x181, 0x3c);

	// 131:17c8
	_tickMarker = _toolbox->TickCount();
	for (int16 i = 0x30; i <= 0x31; i++) {
		_pics[i] = _toolbox->GetPicture(i);
	}
	for (int16 i = 0x4b; i <= 0x52; i++) {
		_pics[i] = _toolbox->GetPicture(i);
	}
	delayFromMarker(0xd2);
	fillRect(0, 0, 0x96, 0x18b, 2);
	_zbasic->picture(0x193, 0x2d, _pics[0x30]);
	delay(1);
	_zbasic->picture(0x193, 0x2d, _pics[0x31]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(4)).encode().c_str()), 0x181, 0x3c);
	// am I to suffer this fate merely for uncovering
	drawTextRight(_zbasic->str(OFF(5)), 0x181, 0x4d);
	drawTextRight(Common::U32String::format("%s\"", _zbasic->str(OFF(6)).encode().c_str()), 0x181, 0x5e);
	for (int16 i = 0x30; i <= 0x31; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
	for (int16 i = 0x32; i <= 0x38; i++) {
		_pics[i] = _toolbox->GetPicture(i);
		if (i == 0x37) {
			setPortBitsToPage(5);
			_zbasic->picture(0, 0, _pics[0x37]);
			_toolbox->ReleaseResource(_pics[0x37]);
			_toolbox->SetPortBits(_dstPage);
		}
	}
	// 131:19be
	drawClickMessageRightAlign();
	waitForClick();
	if (_quit)
		return;

	drawClickMessageRightAlign();
	fillRect(0, 0, 0x96, 0x18b, 2);
	_zbasic->picture(0x193, 0x2d, _pics[0x32]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	// for if that be true
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(7)).encode().c_str()), 0x181, 0x3c);
	// I wish you had made the challenge more difficult
	drawTextRight(_zbasic->str(OFF(8)), 0x181, 0x4d);
	// after all, I am just a fool and I should not
	drawTextRight(_zbasic->str(OFF(9)), 0x181, 0x5e);
	// have been able to find them so easily
	drawTextRight(Common::U32String::format("%s\"", _zbasic->str(OFF(10)).encode().c_str()), 0x181, 0x6f);

	Common::Rect circleStart, circleEnd;
	float circleMid[8] = { 0 }; // arr_f64_41bbe
	circleStart.top = 6;
	circleStart.left = 0xd0;
	circleStart.bottom = 0x6c;
	circleStart.right = 0x133;
	circleEnd.top = 0xa;
	circleEnd.left = 0xb4;
	circleEnd.bottom = 0x14c;
	circleEnd.right = 0x1f6;
	int16 steps = 0xd;

	// 131:1b6e
	// unroll loops
	circleMid[0] = (float)circleStart.top;
	circleMid[0+4] = (float)(circleEnd.top - circleStart.top) / steps;
	circleMid[1] = (float)circleStart.left;
	circleMid[1+4] = (float)(circleEnd.left - circleStart.left) / steps;
	circleMid[2] = (float)circleStart.bottom;
	circleMid[2+4] = (float)(circleEnd.bottom - circleStart.bottom) / steps;
	circleMid[3] = (float)circleStart.right;
	circleMid[3+4] = (float)(circleEnd.right - circleStart.right) / steps;
	// 131:1c24
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	// pre-draw the puzzle circles coming off the high priestess' head
	for (int16 j = 6; j <= 9; j++) {
		_screenPages[j]->copyFrom(*_screenPages[j - 1]);
		// 131:1c7a
		setPortBitsToPage(j);
		if (j == 6) {
			_toolbox->PaintOval(circleStart);
		}
		// 131:1c9c
		for (int16 i = 0; i <= 2; i++) {
			// unrolled loop
			circleMid[0] += circleMid[0+4];
			temp.top = (int16)circleMid[0];
			circleMid[1] += circleMid[1+4];
			temp.left = (int16)circleMid[1];
			circleMid[2] += circleMid[2+4];
			temp.bottom = (int16)circleMid[2];
			circleMid[3] += circleMid[3+4];
			temp.right = (int16)circleMid[3];
			// 131:1d44
			_toolbox->PaintOval(temp);
		}
	}
	_screenPages[10]->copyFrom(*_screenPages[9]);
	// 131:1d98
	setPortBitsToPage(0xa);
	_toolbox->PaintOval(circleEnd);
	setPortBitsToPage(5);
	_toolbox->SetRect(temp, 0xae, 0x8c, 0xb6, 0x94);
	_toolbox->InvertOval(temp);
	_toolbox->SetRect(temp, 0x151, 0x86, 0x159, 0x8e);
	_toolbox->InvertOval(temp);
	_toolbox->SetPortBits(_dstPage);
	drawClickMessageRightAlign();
	waitForClick();
	if (_quit)
		return;

	// 131:1e18
	// card gets vivisected
	fillRect(0, 0, 0x96, 0x18b, 2);
	fillRect(0x12c, 0x15e, 0x156, 0x200, 2);
	int16 cardSliceOffset = 0;
	Particle cardSliceShift[16];
	for (int16 i = 0xa; i <= 0x96; i += 0xa) {
		_zbasic->get(
			0x190,
			i + 0xa,
			0x1ea,
			i + 0x14,
			_cardSlices[i/0xa]
		);
		cardSliceShift[i/0xa].xPos = 0x190;
		cardSliceOffset++;
		if (cardSliceOffset > 4) {
			cardSliceOffset = 1;
		}
		cardSliceShift[i/0xa].veloc = cardSliceOffset;
	}
	// 131:1f1a
	for (int16 j = 1; j <= 0xf; j++) {
		if (j < 9) {
			_toolbox->InvertRect(_pageRect);
		}
		_tickMarker = _toolbox->TickCount();
		for (int16 i = 0xa; i <= 0x96; i += 0xa) {
			int16 index = i / 0xa;
			_zbasic->put(cardSliceShift[index].xPos, i + 0xa, _cardSlices[index], Graphics::MacToolbox::kNotSrcXor);
			cardSliceShift[index].xPos -= cardSliceShift[index].veloc*j;
			_zbasic->put(cardSliceShift[index].xPos, i + 0xa, _cardSlices[index], Graphics::MacToolbox::kSrcCopy);
			// 131:2062
		}
		// 131:2072
		delayFromMarker(2);
	}
	for (int16 j = 1; j <= 0xf; j++) {
		_tickMarker = _toolbox->TickCount();
		for (int16 i = 0xa; i <= 0x96; i += 0xa) {
			int16 index = i / 0xa;
			_zbasic->put(cardSliceShift[index].xPos, i + 0xa, _cardSlices[index], Graphics::MacToolbox::kNotSrcXor);
			cardSliceShift[index].xPos += cardSliceShift[index].veloc*j;
			_zbasic->put(cardSliceShift[index].xPos, i + 0xa, _cardSlices[index], Graphics::MacToolbox::kSrcCopy);
		}
		delayFromMarker(2);
	}

	// 131:21e0
	_zbasic->picture(0x193, 0x2d, _pics[0x33]);
	int16 yPos = 0x14;
	dialog = _zbasic->str(OFF(11)); // be content that you still live
	for (int16 i = 1; i <= 0x18; i++) {
		_tickMarker = _toolbox->TickCount();
		_zbasic->text(kPrologueFontFool, i, 0, Graphics::MacToolbox::kSrcXor);
		yPos += i;
		drawText(dialog, 5 + i*2, yPos);
		delayFromMarker(3);
	}
	// 131:2282
	_zbasic->picture(0x193, 0x2d, _pics[0x34]);
	_tickMarker = _toolbox->TickCount();
	setPortBitsToPage(4);
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, 0xdf);
	_toolbox->InvertRect(temp);
	_toolbox->SetRect(temp, 0, 0xdf, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenMode(Graphics::MacToolbox::kPatOr);
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PaintRect(temp);
	// 131:2316
	_screenPages[0xb]->copyFrom(*_screenPages[0x4].get());
	setPortBitsToPage(0xb);
	_zbasic->picture(0xb5, 0xa, _pics[0x38]);
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0x1e);
	_toolbox->PenNormal();
	int16 xPos = 0x17c;
	for (int16 j = 0; j <= 0x14a; j++) {
		_tickMarker = _toolbox->TickCount();
		if (j > 0xb4) {
			xPos = SCREEN_WIDTH;
		}
		// 131:2398
		for (int16 i = 0xa; i <= 0x14; i += 5) {
			_toolbox->MoveTo(0, j + _zbasic->rndInt(i));
			_toolbox->LineTo(xPos, j + _zbasic->rndInt(i));
			// 131:23d8
		}
		_toolbox->MoveTo(0, j);
		_toolbox->LineTo(xPos, j);
		if ((j % 4) == 0) {
			delayFromMarker(0);
		}
		// 131:2414
	}

	// 131:2422
	_zbasic->picture(0x193, 0x2d, _pics[0x35]);
	delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	// well I for one am not content
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(12)).encode().c_str()), 0x181, 0x3c);
	drawTextRight(Common::U32String::format("%s\"", _zbasic->str(OFF(13)).encode().c_str()), 0x181, 0x4d);
	drawClickMessageRightAlign();
	waitForClick();
	if (_quit)
		return;

	_zbasic->picture(0x193, 0x2d, _pics[0x36]);

	// 131:24da
	fillRect(0, 0, 0x96, 0x18b, 2);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	// show me one last puzzle if you dare
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(14)).encode().c_str()), 0x181, 0x3c);
	drawTextRight(_zbasic->str(OFF(15)), 0x181, 0x4d);
	drawTextRight(Common::U32String::format("%s\"", _zbasic->str(OFF(16)).encode().c_str()), 0x181, 0x5e);
	waitForClick();
	if (_quit)
		return;

	// 131:258a
	_srcPage = _screenPages[5];

	for (int16 i = 1; i <= 0x1a; i++) {
		srcRect.top = 0xab - (int16)(i*6.66f);
		srcRect.left = 0x100 - i*0xa;
		srcRect.bottom = 0xab + (int16)(i*6.66f);
		srcRect.right = 0x100 + i*0xa;
		_toolbox->CopyBits(_srcPage, _dstPage, srcRect, srcRect, Graphics::MacToolbox::kSrcCopy, nullptr);
		_toolbox->Delay(0);
	}
	// 131:266e
	_toolbox->CopyBits(_srcPage, _dstPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, nullptr);
	Common::Rect eye1, eye2;
	_toolbox->SetRect(eye1, 0xae, 0x8c, 0xb6, 0x94);
	_toolbox->SetRect(eye2, 0x151, 0x86, 0x159, 0x8e);
	_toolbox->InsetRect(eye1, 3, 3);
	_toolbox->InsetRect(eye2, 3, 3);
	_toolbox->FillOval(eye1, _patterns[0]);
	_toolbox->FillOval(eye2, _patterns[0]);
	_toolbox->Delay(0);
	// 131:2720
	for (int i = 1; i <= 3; i++) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->InsetRect(eye1, -1, -1);
		_toolbox->InsetRect(eye2, -1, -1);
		_toolbox->FillOval(eye1, _patterns[0]);
		_toolbox->FillOval(eye2, _patterns[0]);
		delayFromMarker(1);
	}
	// 131:27a2
	copyScreenToPage(5);
	_toolbox->SetRect(temp, 0xd0, 6, 0x133, 0x6c);
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	_toolbox->PenSize(3, 3);
	int16 endAngle = 10;
	for (int16 i = 355; i >= 180; i -= 5) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->FrameArc(temp, i, endAngle);
		endAngle += 10;
		delayFromMarker(1);
	}
	// 131:281c
	for (int16 i = 0; i <= 6; i++) {
		_tickMarker = _toolbox->TickCount();
		_toolbox->FrameOval(temp);
		delayFromMarker(1);
	}
	// 131:284c
	for (int16 i = 5; i <= 0xa; i++) {
		_tickMarker = _toolbox->TickCount();
		blitPageToScreen(i);
		delayFromMarker(1);
	}
	// 131:2878
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int16 i = 0; i <= 32; i++) { // was: 30
		_tickMarker = _toolbox->TickCount();
		// limit flashing to WCAG recommendation
		if ((i % 8) == 0) {
			_toolbox->InvertRect(temp);
		}
		delayFromMarker(1);
	}
	// 131:28c2
	scanlineBlitPageToScreen(0xb, 0, SCREEN_WIDTH, 1);
	_screenPages[5]->copyFrom(*_screenPages[11]);
	setPortBitsToPage(5);
	_zbasic->picture(0x16, 0xde, _pics[0x4b]);
	// 131:2926
	for (int16 i = 6; i <= 7; i++) {
		_screenPages[i]->copyFrom(*_screenPages[5]);
	}
	setPortBitsToPage(6);
	_zbasic->picture(0x10, 0x54, _pics[0x4c]);
	setPortBitsToPage(7);
	_zbasic->picture(0x10, 0x54, _pics[0x4c]);
	_zbasic->picture(0x2, 0x50, _pics[0x52]);
	_toolbox->SetPortBits(_dstPage);
	delay(0x1e);
	scanlineBlitPageToScreen(0x6, 0, SCREEN_WIDTH, 1);
	delay(0x1e);
	blitPageToScreen(7);
	delay(0x48);
	blitPageToScreen(6);
	delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	// 131:2a22
	// now what have I gotten myself into?
	drawText(Common::U32String::format("\"%s", _zbasic->str(OFF(17)).encode().c_str()), 0x3c, 0x2d);
	drawText(Common::U32String::format("%s\"", _zbasic->str(OFF(18)).encode().c_str()), 0x46, 0x3e);
	_tickMarker = _toolbox->TickCount();
	for (int16 i = 6; i <= 0xa; i++) {
		_screenPages[i]->copyFrom(*_screenPages[5]);
	}
	setPortBitsToPage(6);
	_zbasic->picture(0x15, 0x55, _pics[0x4d]);
	setPortBitsToPage(7);
	_zbasic->picture(0x1c, 0x57, _pics[0x4e]);
	setPortBitsToPage(8);
	_zbasic->picture(0x1c, 0x57, _pics[0x4f]);
	setPortBitsToPage(9);
	_zbasic->picture(0x1d, 0x58, _pics[0x50]);
	setPortBitsToPage(0xa);
	_zbasic->picture(0x1d, 0x58, _pics[0x51]);
	// 131:2b7c
	Line smears[251]; // arr_i16_1e8
	for (int16 i = 1; i <= 0xe; i++) {
		smears[i].xStart = _zbasic->readDataInt();
		smears[i].yStart = _zbasic->readDataInt();
		smears[i].xEnd = _zbasic->readDataInt();
		smears[i].yEnd = _zbasic->readDataInt();
	}
	// 131:2c02
	_zbasic->text(kPrologueFontSmall, 0x9, 0, Graphics::MacToolbox::kSrcOr);
	for (int16 i = 1; i <= 0xe; i++) {
		// THEBOOKOFTHOTH
		drawText(_zbasic->midStr(_zbasic->str(OFF(19)), i, 1), smears[i].xStart, smears[i].yStart);
	}
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	// that's it! the answer is the book of thoth
	drawText(Common::U32String::format("\"%s", _zbasic->str(OFF(20)).encode().c_str()), 0x32, 0x2d);
	drawText(Common::U32String::format("%s\"", _zbasic->str(OFF(21)).encode().c_str()), 0x3c, 0x3e);
	// 131:2cf6
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0xd2);
	drawClickMessageRightAlign();
	waitForClick();
	if (_quit)
		return;

	drawClickMessageRightAlign();
	blitPageToScreen(6);
	delay(3);
	blitPageToScreen(7);
	delay(3);
	blitPageToScreen(8);
	_tickMarker = _toolbox->TickCount();
	// 131:2d3a
	for (int16 i = 0x30; i <= 0x36; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
	_toolbox->ReleaseResource(_pics[0x38]);
	for (int16 i = 0x4b; i <= 0x52; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
	for (int16 i = 0x29; i <= 0x2a; i++) {
		_pics[i] = _toolbox->GetPicture(i);
	}
	for (int16 i = 0x53; i <= 0x5b; i++) {
		_pics[i] = _toolbox->GetPicture(i);
	}
	delayFromMarker(0x96);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	// 131:2e1a
	_toolbox->MoveTo(0x46, 0x3e);
	// wait, what's this?
	_toolbox->DrawString(Common::U32String::format("\"%s\"", _zbasic->str(OFF(22)).encode().c_str()));
	delay(0x96);
	blitPageToScreen(0x9);
	// 131:2e58
	for (int16 i = 1; i <= 0xe; i++) {
		// THEBOOKOFTHOTH
		dialog = _zbasic->midStr(_zbasic->str(OFF(23)), i, 1);
		for (int16 l = 0; l <= 1; l++) {
			steps = 0x21;
			float smearMid[4] = { 0 }; // arr_f64_41bbe
			smearMid[0] = (float)smears[i].xStart;
			smearMid[2] = (float)(smears[i].xEnd - smears[i].xStart) / steps;
			smearMid[1] = (float)smears[i].yStart;
			smearMid[3] = (float)(smears[i].yEnd - smears[i].yStart) / steps;
			// 131:2f66
			if (l == 0) {
				_zbasic->text(kPrologueFontSmall, 9, 0, Graphics::MacToolbox::kSrcOr);
				drawText(dialog, smears[i].xStart, smears[i].yStart);
			}
			// 131:2fc4
			_zbasic->text(kPrologueFontSmall, 0x9, 0, Graphics::MacToolbox::kSrcXor);
			for (int16 k = 1; k <= steps-1; k++) {
				smearMid[0] += smearMid[0 + 2];
				int16 x = (int16)smearMid[0];
				smearMid[1] += smearMid[1 + 2];
				int16 y = (int16)smearMid[1];
				// 131:3078
				drawText(dialog, x, y);
			}
			_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
			drawText(dialog, smears[i].xEnd, smears[i].yEnd);
			delay(1);
		}
	}
	// 131:3130
	delay(0x1e);
	blitPageToScreen(0xa);
	_tickMarker = _toolbox->TickCount();
	copyScreenToPage(0xa);
	setPortBitsToPage(0xa);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	// that's it! the answer is the book of thoth
	drawText(Common::U32String::format("\"%s", _zbasic->str(OFF(24)).encode().c_str()), 0x32, 0x2d);
	drawText(Common::U32String::format("%s\"", _zbasic->str(OFF(25)).encode().c_str()), 0x3c, 0x3e);
	_screenPages[0xb]->copyFrom(*_screenPages[0xa]);
	setPortBitsToPage(0xb);
	temp.top = 0xa;
	temp.left = 0xb4;
	temp.bottom = 0x14c;
	temp.right = 0x1f6;
	_toolbox->InvertOval(temp);
	for (int16 i = 5; i <= 0x9; i++) {
		_screenPages[i]->copyFrom(*_screenPages[4]);
	}
	// 131:327e
	setPortBitsToPage(4);
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, 0xdf);
	_toolbox->InvertRect(temp);
	_zbasic->picture(0xd, 0xf0, _pics[0x58]);
	_zbasic->picture(0x186, 0xc, _pics[0x59]);
	setPortBitsToPage(5);
	_zbasic->picture(0xf, 0x68, _pics[0x53]);
	setPortBitsToPage(6);
	_zbasic->picture(0x11, 0x8a, _pics[0x54]);
	setPortBitsToPage(7);
	_zbasic->picture(0x13, 0xa9, _pics[0x55]);
	setPortBitsToPage(8);
	_zbasic->picture(0x11, 0xcd, _pics[0x56]);
	setPortBitsToPage(9);
	_zbasic->picture(0xf, 0xf3, _pics[0x57]);
	// 131:339c
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0x96);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	// that's it! the answer is the book of thoth
	drawText(Common::U32String::format("\"%s", _zbasic->str(OFF(26)).encode().c_str()), 0x32, 0x2d);
	drawText(Common::U32String::format("%s\"", _zbasic->str(OFF(27)).encode().c_str()), 0x3c, 0x3e);
	// 131:341c
	scanlineBlitPageToScreen(0xb, 0xb0, 0x1f6, 0);
	scanlineBlitPageToScreen(0x5, 0xb0, 0x1f6, 0);
	delay(0xa);
	for (int16 i = 5; i <= 9; i++) {
		blitPageToScreen(i);
		delay(3);
	}
	_zbasic->picture(0xd, 0xf0, _pics[0x58]);
	_toolbox->SetRect(temp, 0, 0, SCREEN_WIDTH, 0xdf);
	// lightning flash in sky
	for (int16 i = 0x1b; i >= 0; i--) {
		_toolbox->InvertRect(temp);
		// this is fudged considerably
		// 131:34c4
		_toolbox->Delay(i/4);
	}
	// 131:34dc
	blitPageToScreen(4);
	_tickMarker = _toolbox->TickCount();
	int16 wadjetX = 0x8c;
	int16 wadjetY = 0x3c;
	setPortBitsToPage(9);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(wadjetX, wadjetY, _pics[0x29]); // wadjet eye mask
	setPortBitsToPage(0xa);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(wadjetX, wadjetY, _pics[0x2a]); // wadjet eye
	_toolbox->SetPortBits(_dstPage);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
	// the book of thoth?
	drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(28)).encode().c_str()), 0x1a4, 0x2d);
	_tickMarker = _toolbox->TickCount();
	_screenPages[5]->copyFrom(*_screenPages[4]);
	setPortBitsToPage(5);
	_zbasic->picture(0x171, 0xb, _pics[0x5b]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
	// here is the book of thoth
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(29)).encode().c_str()), 0x1a4, 0x2d);
	srcRect.top = wadjetY;
	srcRect.left = wadjetX;

	// 131:366e
	srcRect.bottom = wadjetY + 100;
	srcRect.right = wadjetX + 200;
	_bgPage->copyFrom(*_screenPages[4]);
	_srcPage = _screenPages[9];
	_toolbox->CopyBits(_srcPage, _bgPage, srcRect, srcRect, Graphics::MacToolbox::kSrcBic, 0);
	_srcPage = _screenPages[0xa];
	_toolbox->CopyBits(_srcPage, _bgPage, srcRect, srcRect, Graphics::MacToolbox::kSrcOr, 0);
	for (int16 i = 6; i <= 8; i++) {
		_screenPages[i]->copyFrom(*_bgPage);
	}
	setPortBitsToPage(6);
	_zbasic->picture(0x171, 0xb, _pics[0x5b]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);

	// here is the book of thoth
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(30)).encode().c_str()), 0x1a4, 0x2d);
	setPortBitsToPage(7);
	_zbasic->picture(0x181, 0xf, _pics[0x5a]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
	// and your answer is incorrect
	drawTextRight(Common::U32String::format("%s\"", _zbasic->str(OFF(31)).encode().c_str()), 0x1a4, 0x2d);
	setPortBitsToPage(8);

	// 131:3806
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
	// destroy him
	drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(32)).encode().c_str()), 0x1a4, 0x2d);
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0xb4);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);

	// the book of thoth?
	drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(33)).encode().c_str()), 0x1a4, 0x2d);
	delay(0xf);
	blitPageToScreen(0x5);
	srcRect.left = wadjetX;
	srcRect.right = wadjetX + 0xc8;
	dstRect.left = wadjetX;
	dstRect.right = wadjetX + 0xc8;
	srcRect.top = 0x19;
	dstRect.bottom = 0xdf;

	// 131:38f4
	// wadjet eye rises over horizon
	for (int16 i = 0xdc; i >= 0x19; i -= 5) {
		_tickMarker = _toolbox->TickCount();
		_bgPage->copyFrom(*_screenPages[5]);
		dstRect.top = i;
		srcRect.bottom = 0x19 + 0xdf - i;

		_srcPage = _screenPages[9];
		_toolbox->CopyBits(_srcPage, _bgPage, srcRect, dstRect, Graphics::MacToolbox::kSrcBic, 0);

		_srcPage = _screenPages[10];
		_toolbox->CopyBits(_srcPage, _bgPage, srcRect, dstRect, Graphics::MacToolbox::kSrcOr, 0);
		delayFromMarker(2);

		_toolbox->CopyBits(_bgPage, _dstPage, dstRect, dstRect, Graphics::MacToolbox::kSrcCopy, 0);
	}
	// 131:39ba

	_tickMarker = _toolbox->TickCount();
	_toolbox->ReleaseResource(_pics[0x29]);
	_toolbox->ReleaseResource(_pics[0x2a]);

	// 131:39e8
	for (int16 i = 0x53; i <= 0x59; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
	_toolbox->ReleaseResource(_pics[0x5b]);

	// 131:3a20
	_screenPages[10]->copyFrom(*_screenPages[8]);
	delayFromMarker(0x3c);

	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
	// here is the book of thoth
	drawTextRight(Common::U32String::format("\"%s", _zbasic->str(OFF(34)).encode().c_str()), 0x1a4, 0x2d);

	delay(0xf);
	blitPageToScreen(0x7);
	_tickMarker = _toolbox->TickCount();

	for (int16 i = 0x39; i <= 0x45; i++) {
		_pics[i] = _toolbox->GetPicture(i);
	}

	_pics[0x47] = _toolbox->GetPicture(0x47);
	delayFromMarker(0xc8);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
	// and your answer is incorrect
	drawTextRight(Common::U32String::format("%s\"", _zbasic->str(OFF(35)).encode().c_str()), 0x1a4, 0x2d);
	delay(0xf);

	blitPageToScreen(0xa);
	_tickMarker = _toolbox->TickCount();
	setPortBitsToPage(0x7);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0xaa, _pics[0x39]); // treasure pile
	// 131:3b8a
	fillRect(0x124, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_screenPages[8]->copyFrom(*_screenPages[7]);
	_screenPages[9]->copyFrom(*_screenPages[8]);
	_toolbox->InvertRect(_pageRect);
	setPortBitsToPage(0x9);
	_zbasic->picture(0x5d, 0x22, _pics[0x3a]); // priestess card
	_zbasic->picture(0x1b3, 0xb5, _pics[0x3b]); // fool head
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0x5a);
	for (int16 k = 0; k <= 8; k++) {
		for (int16 j = 1; j <= 8; j++) {
			_toolbox->SetRect(temp, 0xe9, 0x58, 0xe9, 0x58);
			_tickMarker = _toolbox->TickCount();
			for (int16 i = 1; i <= j; i++) {
				_toolbox->InsetRect(temp, -1, -1);
				_toolbox->InvertOval(temp);
			}
			// 131:3cbe
			for (int16 i = 1; i <= j; i++) {
				_toolbox->InsetRect(temp, 1, 1);
				_toolbox->InvertOval(temp);
			}
			delayFromMarker(1);
		}
		// 131:3d0c
		if (k == 7) {
			_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcXor);
			// destroy him
			drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(36)).encode().c_str()), 0x1a4, 0x2d);
			_zbasic->picture(0x181, 0xf, _pics[0x5a]); // angry priestess
			// destroy him!!!
			drawTextRight(Common::U32String::format("\"%s\"", _zbasic->str(OFF(37)).encode().c_str()), 0x1a4, 0x2d);
		}
		// 131:3db8
		if (k < 8) {
			for (int16 j = 8; j >= 1; j--) {
				_tickMarker = _toolbox->TickCount();
				for (int16 i = 1; i <= j; i++) {
					_toolbox->InsetRect(temp, -1, -1);
					_toolbox->InvertOval(temp);
				}
				for (int16 i = 1; i <= j; i++) {
					_toolbox->InsetRect(temp, 1, 1);
					_toolbox->InvertOval(temp);
				}
				delayFromMarker(1);
			}
		}
		// 131:3e5e
	}
	// 131:3e6c
	_toolbox->PenPat(_patterns[2]);
	_toolbox->PenSize(5, 5);
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	int16 beamX = 0xe5;
	int16 beamY = 0x58;
	for (int16 i = 1; i <= 0x13; i++) {
		_tickMarker = _toolbox->TickCount();
		beamX -= i;
		beamY += (int16)(i*0.9f);
		_toolbox->MoveTo(0xe5, 0x58);
		_toolbox->LineTo(beamX, beamY);
		delayFromMarker(0);
		_toolbox->MoveTo(0xe5, 0x58);
		_toolbox->LineTo(beamX, beamY);
	}
	_toolbox->MoveTo(0xe5, 0x58);
	_toolbox->LineTo(0x34, 0xf2);
	beamX = 0x34;
	beamY = 0xf2;
	for (int16 i = 5; i <= 0x1c; i++) {
		// 131:3f4c
		_tickMarker = _toolbox->TickCount();
		beamX += i;
		beamY -= (int16)(i*0.5f);
		_toolbox->MoveTo(0x34, 0xf2);
		_toolbox->LineTo(beamX, beamY);
		delayFromMarker(0);
		_toolbox->MoveTo(0x34, 0xf2);
		_toolbox->LineTo(beamX, beamY);
	}
	// 131:3fd8
	_toolbox->MoveTo(0x34, 0xf2);
	_toolbox->LineTo(beamX, beamY);
	for (int16 i = 0x1e; i >= 0xa; i -= 5) {
		zoomFlash(beamY - 3, beamX - 3, beamY + 3, beamX + 3, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, i);
	}

	for (int16 k = 7; k <= 8; k++) {
		// 131:4088
		steps = (k == 7) ? 0x19 : 0x23;

		// 131:40a2
		_srcPage = _screenPages[k];
		Common::Rect start; // arr_rect_41af4
		Common::Rect end; // arr_rect_41afc
		start.top = beamY - 3;
		start.left = beamX - 3;
		start.bottom = beamY + 3;
		start.right = beamX + 3;
		end.top = 0;
		end.left = 0;
		end.bottom = SCREEN_HEIGHT;
		end.right = SCREEN_WIDTH;
		// unroll loop
		float mid[8] = { 0 }; // arr_f64_41bbe
		mid[0] = (float)start.top;
		mid[0+4] = (float)(end.top - start.top)/steps;
		mid[1] = (float)start.left;
		mid[1+4] = (float)(end.left - start.left)/steps;
		mid[2] = (float)start.bottom;
		mid[2+4] = (float)(end.bottom - start.bottom)/steps;
		mid[3] = (float)start.right;
		mid[3+4] = (float)(end.right - start.right)/steps;

		// 131:420a
		srcRect = start;

		// 131:425a
		_toolbox->CopyBits(_srcPage, _dstPage, srcRect, srcRect, Graphics::MacToolbox::kSrcCopy, 0);

		for (int16 j = 1; j <= steps - 1; j++) {
			_tickMarker = _toolbox->TickCount();
			for (int16 i = 0; i <= 3; i++) {
				mid[i] += mid[i+4];
			}
			// 131:42e6
			srcRect.top = (int16)mid[0];
			srcRect.left = (int16)mid[1];
			srcRect.bottom = (int16)mid[2];
			srcRect.right = (int16)mid[3];
			_toolbox->CopyBits(_srcPage, _dstPage, srcRect, srcRect, Graphics::MacToolbox::kSrcCopy, 0);
			delayFromMarker(0);
			// 131:4362
		}
		// 131:4376
		_toolbox->CopyBits(_srcPage, _dstPage, _pageRect, _pageRect, Graphics::MacToolbox::kSrcCopy, 0);

	}
	// 131:439a
	_zbasic->picture(0x1b3, 0xb5, _pics[0x3b]);
	_cardLines[0] = 0;
	_cardLines[1] = 0;
	_cardLines[2] = SCREEN_WIDTH;
	_cardLines[3] = 0;
	_cardLines[4] = SCREEN_WIDTH;
	_cardLines[5] = SCREEN_HEIGHT;
	_cardLines[6] = 0;
	_cardLines[7] = SCREEN_HEIGHT;
	_cardLines[8] = 0xb5;
	_cardLines[9] = 0x22;
	_cardLines[10] = 0xb5;
	_cardLines[11] = 0xb5;
	_cardLines[12] = 0x5d;
	_cardLines[13] = 0xb5;
	_cardLines[14] = 0x5d;
	_cardLines[15] = 0x22;

	finaleCardRotate();
	finaleCardRotate();

	// 131:44b8
	scanlineBlitPageToScreen(0x9, 0, SCREEN_WIDTH, 0);
	copyScreenToPage(0);
	setPortBitsToPage(0);
	_zbasic->picture(0x1b5, 0xba, _pics[0x3c]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	// but the fool had gained the gift of wisdom
	drawText(_zbasic->str(OFF(38)), 0xd2, 0x61);
	// and was able to trick the high priestess
	drawText(_zbasic->str(OFF(39)), 0xd2, 0x72);
	drawClickMessageRightAlign();
	_toolbox->SetPortBits(_dstPage);
	blitPageToScreen(0);
	setPortBitsToPage(0);
	_zbasic->picture(0x1b8, 0xb3, _pics[0x3d]);
	fillRect(0, 0xc8, 0xa5, SCREEN_WIDTH, 0);

	// 131:45a4
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	// he had remembered well the words
	drawText(_zbasic->str(OFF(40)), 0xd2, 0x3f);
	// of the magician
	drawText(_zbasic->str(OFF(41)), 0xd2, 0x50);
	// the high priestess may have learned how
	drawText(Common::U32String::format("\"%s", _zbasic->str(OFF(42)).encode().c_str()), 0xd7, 0x6d);
	// to command the sacred book of thoth
	drawText(_zbasic->str(OFF(43)), 0xde, 0x7e);
	// but even she cannot force it to do
	drawText(_zbasic->str(OFF(44)), 0xde, 0x8f);
	// anything inherently evil
	drawText(Common::U32String::format("%s\"", _zbasic->str(OFF(45)).encode().c_str()), 0xde, 0xa0);
	// 131:46aa
	_toolbox->SetPortBits(_dstPage);
	waitForClick();
	if (_quit)
		return;
	blitPageToScreen(0);
	copyScreenToPage(0);
	setPortBitsToPage(0);
	_zbasic->picture(0x1c4, 0xb3, _pics[0x3e]);
	fillRect(0, 0xc8, 0xa5, SCREEN_WIDTH, 0);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);

	// some say that the story of the fool and his
	drawText(_zbasic->str(OFF(46)), 0xd2, 0x5c);
	// adventures inspired the creation of the
	drawText(_zbasic->str(OFF(47)), 0xd2, 0x6d);
	// modern day tarot deck
	drawText(_zbasic->str(OFF(48)), 0xd2, 0x7e);

	// 131:4780
	_toolbox->SetPortBits(_dstPage);
	waitForClick();
	if (_quit)
		return;
	blitPageToScreen(0);
	copyScreenToPage(0);
	setPortBitsToPage(0);
	_zbasic->picture(0x1be, 0xb4, _pics[0x3f]);
	fillRect(0, 0xc8, 0x96, SCREEN_WIDTH, 0);
	fillRect(0x12c, 0x12c, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcOr);
	// but I doubt it
	drawText(Common::U32String::format("\"%s\"", _zbasic->str(OFF(49)).encode().c_str()), 0x184, 0xb1);
	// 131:4848
	_toolbox->SetPortBits(_dstPage);
	waitForClick();
	if (_quit)
		return;
	blitPageToScreen(0);
	_tickMarker = _toolbox->TickCount();
	setPortBitsToPage(0);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_toolbox->SetPortBits(_dstPage);
	delayFromMarker(0x78);

	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PenSize(4, 3);
	_toolbox->SetRect(temp, 0, 0, 0x201, 0x156);
	for (int16 i = 0; i <= 0x200; i++) {
		if ((i % 4) == 0) {
			_tickMarker = _toolbox->TickCount();
		}
		// 131:48ea
		if ((i % 2) == 0) {
			temp.top = (int16)(i/2.5);
		}
		// 131:4936
		temp.left = i;
		if ((i % 10) == 0) {
			temp.bottom = SCREEN_HEIGHT - (i/10);
		}
		// 131:498a
		if (i > 0x1bd) {
			temp.right -= 1;
		}
		// 131:49b6
		_toolbox->FrameRect(temp);
		if ((i % 4) == 3) {
			delayFromMarker(0);
		}
		// 131:49e2
	}
	// 131:49f0
	fillRect(0, 0, 0x3e8, 0x3e8, 1);
	scanlineBlitPageToScreen(0, 0, SCREEN_WIDTH, 2);
	delay(0x28);
	// var_i16_3f2 = 0x10a;
	int16 yPic = 0x5a;
	for (int16 j = 0x40; j <= 0x45; j++) {
		_tickMarker = _toolbox->TickCount();
		_srcPage = _screenPages[0];
		_toolbox->SetPortBits(_srcPage);
		fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
		_zbasic->picture(0x78, yPic, _pics[j]);
		int16 textCount = _zbasic->readDataInt();
		int16 textWidth = _zbasic->readDataInt();
		int16 yOffset = 0x41 - (textWidth / 2);
		for (int16 i = 1; i <= textCount; i++) {
			int16 textSize = _zbasic->readDataInt();
			Common::U32String label = _zbasic->readDataStr();
			_zbasic->text(kPrologueFontFool, textSize, 0, Graphics::MacToolbox::kSrcBic);
			if (textSize == 0xc) {
				yOffset += 0x19;
			} else {
				// 131:4afa
				yOffset += 0x20;
			}
			// 131:4b00
			drawText(label, 0xe6, yOffset + yPic);
		}
		// 131:4b2a
		delayFromMarker(0x14);
		_toolbox->SetPortBits(_dstPage);
		blitPageToScreen(0);
		if (j == 0x40) {
			delay(0xd2);
		} else {
			// 131:4b54
			delay(0xb4);
		}
		// 131:4b5e
		fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	}
	// 131:4b86
	_tickMarker = _toolbox->TickCount();
	_srcPage = _screenPages[0];
	_toolbox->SetPortBits(_srcPage);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->text(kPrologueFontSmall, 9, 0, Graphics::MacToolbox::kSrcBic);
	// the fool's errand
	drawTextCenter(_zbasic->str(OFF(50)), 0x100, 0x19 + yPic + 0x19);
	// (c) 1987 by cliff johnson. all rights reserved
	drawTextCenter(_zbasic->str(OFF(51)), 0x100, 0x32 + yPic + 0x19);
	// portions of this code are copyrighted 1985 zedcor inc
	drawTextCenter(_zbasic->str(OFF(52)), 0x100, 0x4b + yPic + 0x19);
	// the fool's errand is a registered trademark of cliff johnson
	drawTextCenter(_zbasic->str(OFF(53)), 0x100, 0x64 + yPic + 0x19);
	// 131:4c94
	delayFromMarker(0x14);
	_toolbox->SetPortBits(_dstPage);
	blitPageToScreen(0);
	delay(0xb4);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_tickMarker = _toolbox->TickCount();
	_srcPage = _screenPages[0];
	_toolbox->SetPortBits(_srcPage);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);

	_zbasic->text(kPrologueFontFool, 0xc, 0, Graphics::MacToolbox::kSrcBic);
	// and yes, the fool will return in
	drawTextCenter(_zbasic->str(OFF(54)), 0x100, 0xa0);
	_zbasic->text(kPrologueFontFool, 0x18, 0, Graphics::MacToolbox::kSrcBic);
	// the fool and his money
	drawTextCenter(_zbasic->str(OFF(55)), 0x100, 0xc8);
	delayFromMarker(0x14);
	_toolbox->SetPortBits(_dstPage);
	blitPageToScreen(0);
	delay(0xd2);
	for (int16 i = 0x40; i <= 0x47; i++) {
		_toolbox->ReleaseResource(_pics[i]);
	}
}

void FoolPrologue::finaleDrawLoadingMsg() {
	// 131:4dc0
	// FIXME: we don't share the menu surface here yet
	return;
	_toolbox->SetPort(_grafPtrMenu);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcOr);
	// Loading Finale
	Common::U32String str = Common::U32String::format("%s%d%%", _zbasic->str(OFF(56)).encode().c_str(), _prologueLoading);
	// 131:4e08
	fillRect(0, 0, 7, 0x13, _windowWidth - 7);

	drawTextCenter(str, _windowWidth / 2, 0xe);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(_grafPtrWindow);
}

void FoolPrologue::finaleDrawFoolUhOh() {
	// 131:4e48
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcOr);

	// Uh oh . . .
	drawText(Common::U32String::format("\"%s\"", _zbasic->str(OFF(58)).encode().c_str()), 0x1ae, 0xb4);
}

void FoolPrologue::finaleStartText() {
	// 131:4e98
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, Graphics::MacToolbox::kSrcXor);
	int16 yStart = 0xa0;
	// "And so the fool heeded the advice of the magician..."
	drawText(_zbasic->str(OFF(59)), 0x96, yStart);
	drawText(_zbasic->str(OFF(60)), 0xa0, yStart + 0xf);
	drawText(_zbasic->str(OFF(61)), 0xaa, yStart + 0x1e);
	drawText(_zbasic->str(OFF(62)), 0xc8, yStart + 0x2d);
	drawText(_zbasic->str(OFF(63)), 0xe6, yStart + 0x3c);

	drawClickMessage();
}

void FoolPrologue::finaleDrawWind(int16 offset) {
	// 131:4f96
	_toolbox->MoveTo(
		_rain[offset].xPos,
		_rain[offset].yPos
	);

	// 131:4fd4
	_toolbox->LineTo(
		_rain[offset].xPos + _rain[offset].size,
		_rain[offset].yPos
	);
}

void FoolPrologue::finaleCardRotate() {
	// 131:5038
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PenMode(Graphics::MacToolbox::kPatXor);
	int16 steps = 0x21;
	float cardLinesMid[16] = { 0 }; // arr_f64_41bbe
	for (int16 i = 0; i <= 7; i++) {
		cardLinesMid[i] = (float)_cardLines[i];
		cardLinesMid[i+8] = (float)(_cardLines[i+8] - _cardLines[i])/steps;
	}
	// 131:5106
	_toolbox->MoveTo(_cardLines[0], _cardLines[1]);
	_toolbox->LineTo(_cardLines[2], _cardLines[3]);
	_toolbox->LineTo(_cardLines[4], _cardLines[5]);
	_toolbox->LineTo(_cardLines[6], _cardLines[7]);
	_toolbox->LineTo(_cardLines[0], _cardLines[1]);

	for (int16 i = 1; i <= steps - 1; i++) {
		// 131:51ca
		_tickMarker = _toolbox->TickCount();
		for (int16 j = 0; j <= 7; j++) {
			cardLinesMid[j] += cardLinesMid[j+8];
			_cardLines[j + 0x10] = (int16)cardLinesMid[j];
		}
		// 131:5276
		_toolbox->MoveTo(_cardLines[0x10], _cardLines[0x11]);
		_toolbox->LineTo(_cardLines[0x12], _cardLines[0x13]);
		_toolbox->LineTo(_cardLines[0x14], _cardLines[0x15]);
		_toolbox->LineTo(_cardLines[0x16], _cardLines[0x17]);
		_toolbox->LineTo(_cardLines[0x10], _cardLines[0x11]);
		delayFromMarker(1);
	}
	// 131:534e
	_toolbox->MoveTo(_cardLines[0x8], _cardLines[0x9]);
	_toolbox->LineTo(_cardLines[0xa], _cardLines[0xb]);
	_toolbox->LineTo(_cardLines[0xc], _cardLines[0xd]);
	_toolbox->LineTo(_cardLines[0xe], _cardLines[0xf]);
	_toolbox->LineTo(_cardLines[0x8], _cardLines[0x9]);
}

} // End of namespace Fool
