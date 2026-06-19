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
#include "fool/fool.h"
#include "fool/fool_prologue.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

// run finale
void FoolPrologue::finaleRun() {
	// 131:0004
	this->var_i16_3ce = 0x2d;
	this->finaleDrawLoadingMsg();
	this->var_i32_3d6 = _toolbox->OpenPoly();
	_toolbox->MoveTo(0xc8, 0x8d);
	_toolbox->LineTo(0x1df, 0xc8);
	_toolbox->LineTo(0x1d2, 0x11b);
	_toolbox->LineTo(0xcd, 0x93);
	_toolbox->LineTo(0xc8, 0x8d);
	_toolbox->ClosePoly();
	// 131:004c
	this->arr_i32_0[0x1f] = _toolbox->GetPicture(0x1f);
	this->setPortBitsToPage(0x4);
	_zbasic->picture(0x0, 0x0, this->arr_i32_0[0x1f]);
	_toolbox->ReleaseResource(this->arr_i32_0[0x1f]);
	this->finaleDrawLoadingMsg();
	for (int i = 0x14; i <= 0x1e; i++) {
		// 131:009c
		this->arr_i32_0[i] = _toolbox->GetPicture(i);
		if ((i % 3) == 0) {
			this->var_i16_3ce += 5;
			this->finaleDrawLoadingMsg();
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
		_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}
	// 131:01aa
	this->arr_i32_0[0x13] = _toolbox->GetPicture(0x13);
	this->setPortBitsToPage(0x5);
	_zbasic->picture(0, 0xb4, this->arr_i32_0[0x13]);
	_zbasic->picture(0, 0, this->arr_i32_0[0x18]);
	_toolbox->ReleaseResource(this->arr_i32_0[0x13]);
	_toolbox->ReleaseResource(this->arr_i32_0[0x18]);
	this->finaleStartText();
	this->var_i16_3ce = 0x4b;
	this->finaleDrawLoadingMsg();
	// 131:021e
	for (int i = 0x20; i <= 0x2f; i++) {
		this->arr_i32_0[i] = _toolbox->GetPicture(i);
		if (i % 3) {
			this->var_i16_3ce += 5;
			this->finaleDrawLoadingMsg();
		}
	}
	// 131:026e
	this->var_i16_18e = SCREEN_HEIGHT;
	this->shuffleScanlines();
	this->var_i32_1a6 = _zbasic->mem(-1);
	_toolbox->SetPortBits(this->var_i32_32);
	if (!((this->var_i16_10 == SCREEN_WIDTH) && (this->var_i16_12 == SCREEN_HEIGHT))) {
		// 131:02ac
		this->fillRect(0, 0, 0x14, SCREEN_WIDTH, 1);
	} else {
		_toolbox->SetPort(this->var_i32_c);
		this->fillRect(0, 0, 0x14, this->var_i16_10, 2);
		_toolbox->_defaultMenu->setOverlayDirty(true);
		_toolbox->SetPort(this->var_i32_8);
	}
	// 131:02ec
	this->delay(0x3c);
	this->zoomTransition(0x5);
	this->sub_128_e58();
	if (_quit)
		return;

	// xor out the start text
	this->finaleStartText();
	_zbasic->restore(0x4c);
	for (int j = 1; j <= 0xe; j++) {
		this->var_i16_3da = 1;

		// 131:030c
		this->var_i16_18e = _zbasic->readDataInt();
		for (int i = 1; i <= this->var_i16_18e; i++) {
			// 131:031a
			this->arr_i16_1e8[i] = _zbasic->readDataInt();
			this->arr_i16_1e8[i + 0xfb] = _zbasic->readDataInt();
			this->arr_i16_1e8[i + 0x1f6] = _zbasic->readDataInt();
		}

		// 131:0380
		_toolbox->PenMode(kPatXor);
		this->drawTreasurePhaseIn(0x14);
		this->drawTreasurePhaseIn(0xa);
		this->drawTreasurePhaseIn(0x14);
		this->drawTreasurePhaseIn(0x5);
		this->drawTreasurePhaseIn(0xa);
		this->drawTreasurePhaseIn(0x5);
		_toolbox->PenNormal();
		this->drawTreasurePhaseIn(0x0);
	}

	// 131:03c0
	this->delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// well, that's that!
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(22).encode().c_str());

	// 131:0400
	this->drawText(0x17c, 0xb4);
	this->delay(0x1e);

	// 131:041a
	// fool nods and puts away map
	_zbasic->picture(0x1cd, 0xbd, this->arr_i32_0[0x14]);
	this->delay(0x2);
	_zbasic->picture(0x1cd, 0xbf, this->arr_i32_0[0x15]);
	this->delay(0x3c);
	_zbasic->picture(0x1cd, 0xbd, this->arr_i32_0[0x14]);
	this->delay(0x6);
	_zbasic->picture(0x1a6, 0xbf, this->arr_i32_0[0x16]);
	this->delay(0x6);
	_zbasic->picture(0x1b3, 0xc5, this->arr_i32_0[0x17]);
	this->delay(0);

	// 131:04b4
	for (int i = 0x14; i <= 0x17; i++) {
		_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	// 131:04da
	this->var_i32_2 = _toolbox->TickCount();

	this->var_i32_40 = this->arr_i32_41296[5];
	_toolbox->CopyBits(this->var_i32_32, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
	setPortBitsToPage(5);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	this->finaleDrawFoolUhOh();

	// 131:0538
	_toolbox->CopyBits(this->var_i32_32, this->var_i32_4e, this->var_i16_38, this->var_i16_38, kSrcCopy, nullptr);
	_toolbox->SetPortBits(this->var_i32_4e);

	// 131:0554
	this->fillRect(0, 0, 0xa0, 0x15e, 0);
	this->fillRect(0xa0, 0x5a, 0xd2, 0xaa, 0);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);

	// 131:05b0
	this->setPortBitsToPage(6);
	_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);
	this->setPortBitsToPage(7);
	_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);
	this->setPortBitsToPage(8);
	_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);
	this->setPortBitsToPage(9);
	_toolbox->CopyBits(this->var_i32_4e, this->var_i32_40, this->var_i16_38, this->var_i16_38, kSrcOr, nullptr);

	// 131:0630
	// tree morphing into priestess
	_toolbox->SetPortBits(this->var_i32_32);
	_zbasic->get(0x1bc, 0xc1, 0x1f2, 0x11b, this->arr_i32_3bca4);
	this->delayFromMarker(0x3c);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	sub_128_50a(0x6, 0, SCREEN_WIDTH, 0x3);
	this->finaleDrawFoolUhOh();

	// 131:06aa
	this->sub_128_50a(5, 0, SCREEN_WIDTH, 2);
	this->sub_128_50a(7, 0, SCREEN_WIDTH, 2);
	this->sub_128_50a(8, 0, SCREEN_WIDTH, 1);
	this->fillRect(0xaa, 0x177, 0xb9, 0x200, 0);
	this->sub_128_50a(0x9, 0, SCREEN_WIDTH, 1);

	// the wind picks up
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);
	_zbasic->unk_20();

	// 131:0730
	this->var_i16_3dc = 0;
	this->var_i16_18e = 0;
	for (int j = 1; j <= 0x63; j++) {
		if (this->var_i16_3dc < SCREEN_HEIGHT) {
			// 131:074e
			this->var_i16_3dc += _zbasic->maybe() ? 0x4 : 0x5;
		}
		if (this->var_i16_18e < 0x41) {
			// 131:0770
			this->var_i16_18e += 1;
			this->arr_i16_1e8[this->var_i16_18e] = _zbasic->rndInt(32);
			this->arr_i16_1e8[this->var_i16_18e+0xfb] = _zbasic->rndInt(this->var_i16_3dc);
			this->arr_i16_1e8[this->var_i16_18e+0x1f6] = _zbasic->rndInt(0xa) + 0xa;
			this->sub_131_4f96(this->var_i16_18e);
		}
		// 131:07ea
		for (int i = 1; i <= this->var_i16_18e; i++) {
			this->sub_131_4f96(i);
			this->arr_i16_1e8[i] += this->arr_i16_1e8[0x1f6+i]*2;

			// 131:0870
			if (this->arr_i16_1e8[i] > SCREEN_WIDTH) {
				this->arr_i16_1e8[i] = 1;
				this->arr_i16_1e8[0xfb+i] = _zbasic->rndInt(this->var_i16_3dc);
				this->arr_i16_1e8[0x1f6+i] = _zbasic->rndInt(0xa) + 0xa;
			}

			// 131:0902
			this->sub_131_4f96(i);
		}
		// Simulate slow redraw
		_toolbox->Delay(0);
	}


	// 131:0926
	_toolbox->SetRect(this->arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenPat(this->arr_pat_194[3]);
	_zbasic->picture(0x6d, 0x49, this->arr_i32_0[0x1c]);
	_zbasic->picture(0x65, 0x48, this->arr_i32_0[0x1d]);
	_zbasic->picture(0x64, 0x54, this->arr_i32_0[0x1e]);
	// 131:099c

	// zzzzap
	_toolbox->PaintPoly(this->var_i32_3d6);
	for (int i = 0; i <= 0x21; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		_toolbox->InvertRect(this->arr_i16_1bc);
		this->delayFromMarker(0);
	}

	// 131:09d2
	this->sub_128_800(0xc1, 0x1bc, 0x11b, 0x1f2, 0x0, 0x0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	this->sub_128_800(0x156, 0, 0x156, 0, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	this->sub_128_800(0, 0, 0, 0, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);
	this->sub_128_800(0, SCREEN_WIDTH, 0, SCREEN_WIDTH, 0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0x21);

	// 131:0a7a
	// horrible flashing fool duplication spree
	for (int i = 1; i <= 0x63; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		this->var_i16_74 = _zbasic->rndInt(0x1ca);
		this->var_i16_192 = _zbasic->rndInt(0xfc);
		_zbasic->put(this->var_i16_74, this->var_i16_192, this->arr_i32_3bca4, (i % 2) ? kSrcCopy : kNotSrcCopy);
		// limit flashing to WCAG recommendation
		if ((i % 10) == 0) {
			_toolbox->InvertRect(this->arr_i16_1bc);
		}
		this->delayFromMarker(0);
	}

	// 131:0aec
	// scrumble the contents of the screen for a few seconds
	this->var_i32_2 = _toolbox->TickCount();

	// 131:0af6
	while (_toolbox->TickCount() < (this->var_i32_2 + 0x78)) {
		this->var_i16_74 = _zbasic->rndInt(0x1ca);
		this->var_i16_192 = _zbasic->rndInt(0xfc);
		_zbasic->get(this->var_i16_74, this->var_i16_192, this->var_i16_74 + 0x36, this->var_i16_192 + 0x5a, this->arr_i32_3bca4);

		// 131:0b54
		this->var_i16_74 = _zbasic->rndInt(0x1ca);
		this->var_i16_192 = _zbasic->rndInt(0xfc);
		_zbasic->put(this->var_i16_74, this->var_i16_192, this->arr_i32_3bca4, kSrcCopy);
		_toolbox->Delay(0);
	}

	// 131:0bb0
	// fade in to seascape
	this->sub_128_50a(4, 0, SCREEN_WIDTH, 3);
	this->setPortBitsToPage(5);
	this->copyScreenToPage(5);
	// fade in the fool
	_zbasic->picture(0x198, 0x102, this->arr_i32_0[0x25]);
	_toolbox->SetPortBits(this->var_i32_32);
	this->delay(0x3c);
	this->sub_128_50a(5, 0, SCREEN_WIDTH, 0);

	// fool looks around
	_zbasic->picture(0x197, 0x101, this->arr_i32_0[0x26]);

	// 131:0c36
	this->var_i32_2 = _toolbox->TickCount();
	_toolbox->KillPoly(this->var_i32_3d6);
	for (int i = 0x1c; i < 0x1e; i++) {
		_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	// 131:0c6c
	// high priestess becomes giant
	for (int i = 0x6; i <= 0xa; i++) {
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
	for (int i = 0x20; i <= 0x23; i++) {
		_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}
	_toolbox->SetPortBits(this->var_i32_32);
	this->delayFromMarker(0x64);
	_zbasic->picture(0x198, 0x102, this->arr_i32_0[0x25]);
	for (int i = 0x6; i <= 0xa; i++) {
		this->delay(0xf);
		this->sub_128_50a(i, 0, SCREEN_WIDTH, 1);
	}
	_zbasic->picture(0x191, 0x102, this->arr_i32_0[0x27]);
	this->delay(0x3c);

	// 131:0e08
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	// Are you still angry with me?
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(23).encode().c_str());
	this->drawText(0x1f4 - _toolbox->StringWidth(this->var_str_76), 0xd2);
	this->var_i32_2 = _toolbox->TickCount();

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
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);
	//_zbasic->blockMove(this->arr_i32_41296[4], this->arr_i32_41296[9], 0x5580);
	this->arr_i32_41296[9]->copyFrom(*this->arr_i32_41296[4]);
	this->setPortBitsToPage(0x9);
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);
	this->setPortBitsToPage(0xa);
	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);

	// 131:1056
	_toolbox->ReleaseResource(this->arr_i32_0[0x24]);
	_toolbox->SetPortBits(this->var_i32_32);
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

	// wadjet eye appears from horizon
	for (int i = 0xdc; i >= 0x19; i -= 5) {
		// 131:10de
		this->var_i32_2 = _toolbox->TickCount();
		//_zbasic->blockMove(this->arr_i32_41296[5], *this->var_i32_4e, 0x5580);
		this->var_i32_4e->copyFrom(*this->arr_i32_41296[5]);
		this->var_i16_64.top = i;
		this->var_i16_5c.bottom = 0x19 + (0xdf - i);
		// 131:1124
		this->var_i32_40 = this->arr_i32_41296[6];
		_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i16_5c, this->var_i16_64, kSrcBic, nullptr);
		this->var_i32_40 = this->arr_i32_41296[7];
		_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i16_5c, this->var_i16_64, kSrcOr, nullptr);
		this->delayFromMarker(0x2);
		_toolbox->CopyBits(this->var_i32_4e, this->var_i32_32, this->var_i16_64, this->var_i16_64, kSrcCopy, nullptr);
	}
	_toolbox->Delay(0);

	// 131:11a4
	this->copyScreenToPage(0x5);
	this->setPortBitsToPage(0x5);
	this->fillRect(0xc8, 0x145, 0xd7, SCREEN_WIDTH, 2);
	_zbasic->picture(0x18f, 0xc0, this->arr_i32_0[0x2b]);
	_toolbox->SetPortBits(this->var_i32_32);
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);
	_toolbox->PenSize(0x5, 0x5);

	// 131:1208
	for (int i = 0; i <= 0x1b8; i += 6) {
		this->var_i32_2 = _toolbox->TickCount();
		_toolbox->MoveTo(0x172, 0x33);
		_toolbox->LineTo(i, 0x14a);
		if (i == 0x4e) {
			_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
			// yes, apparently so
			this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(24).encode().c_str());
			this->drawTextRightAlign(0x1f4, 0xd2);
			_zbasic->picture(0x198, 0x102, this->arr_i32_0[0x25]);
		}
		this->delayFromMarker(0x2);
		_toolbox->MoveTo(0x172, 0x33);
		_toolbox->LineTo(i, 0x14a);
	}

	// 131:12cc
	this->sub_128_800(0x14a, 0x1b8, 0x14a, 0x1b8, 0xc0, 0x18f, 0x156, 0x1e6, 0x10);
	// get turned into card
	this->sub_128_50a(0x5, 0x12c, SCREEN_WIDTH, 0x1);
	this->delay(0xec);

	// fade out eye, then priestess, then seascape
	for (int i = 8; i <= 0xa; i++) {
		this->sub_128_50a(i, 0, SCREEN_WIDTH, 1);
	}

	this->var_i32_2 = _toolbox->TickCount();
	this->setPortBitsToPage(0x5);

	this->fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(0, 0, this->arr_i32_0[0x2f]);
	_toolbox->SetPortBits(this->var_i32_32);
	this->delayFromMarker(0xa);
	this->var_i16_5c.left = 0x18b;
	this->var_i16_5c.right = 0x1ef;
	this->var_i16_64.left = 0x18b;
	this->var_i16_64.right = 0x1ef;

	// 131:13b8
	// move the fool card up the screen
	for (int i = 0xc0; i >= 0x15; i--) {
		this->var_i32_2 = _toolbox->TickCount();
		this->var_i16_5c.top = i;
		this->var_i16_5c.bottom = this->var_i16_5c.top + 0x96;
		this->var_i16_64.top = i - 1;
		this->var_i16_64.bottom = this->var_i16_64.top + 0x96;
		_toolbox->CopyBits(this->var_i32_32, this->var_i32_32, this->var_i16_5c, this->var_i16_64, kSrcCopy, nullptr);
		this->delayFromMarker(0);
	}

	// 131:1424
	// fool turns
	_zbasic->picture(0x193, 0x2d, this->arr_i32_0[0x2c]);
	this->delay(0x1);
	_zbasic->picture(0x193, 0x2d, this->arr_i32_0[0x2d]);
	this->delay(0x1);
	_zbasic->picture(0x193, 0x2d, this->arr_i32_0[0x2e]);
	this->copyScreenToPage(0x6);
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);
	_toolbox->PenSize(0x3, 0x3);
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
	// fool drops bindle
	this->var_i32_40 = this->arr_i32_41296[5];
	_toolbox->SetPortBits(this->var_i32_4e);
	for (int i = 1; i <= 0xf; i++) {
		this->var_i32_2 = _toolbox->TickCount();
		//_zbasic->blockMove(this->arr_i32_41296[6], *this->var_i32_4e, 0x5580);
		this->var_i32_4e->copyFrom(*this->arr_i32_41296[6]);
		this->var_i16_3e0 += 5 + i*2;
		this->var_i16_64.left = 0x1e0;
		this->var_i16_64.right = this->var_i16_64.left + 0x32;
		this->var_i16_64.top = this->var_i16_3e0;
		this->var_i16_64.bottom = this->var_i16_64.top + 0x32;
		_toolbox->CopyBits(this->var_i32_40, this->var_i32_4e, this->var_i16_5c, this->var_i16_64, kSrcXor, nullptr);
		// 131:1582
		_toolbox->MoveTo(0x190 - i, this->var_i16_3e0 - 0x1e + i*3);
		_toolbox->LineTo(0x1f4, this->var_i16_3e0 + 0xa - i);
		_toolbox->CopyBits(this->var_i32_4e, this->var_i32_32, this->var_i16_6c, this->var_i16_6c, kSrcCopy, nullptr);
		this->delayFromMarker(0x1);
	}

	_toolbox->SetPortBits(this->var_i32_32);
	_toolbox->PenNormal();
	_toolbox->PenMode(kPatXor);
	// 131:1606
	for (int j = 0; j <= 1; j++) {

		for (int i = 1; i <= 0xfa; i++) {
			this->var_i32_2 = _toolbox->TickCount();

			if (j == 0) {
				this->arr_i16_1e8[i] = 0x1f4 - i + _zbasic->rndInt(i);
				this->arr_i16_1e8[i+0xfb] =  SCREEN_HEIGHT - _zbasic->rndInt(i);
			}
			// 131:1692
			_toolbox->MoveTo(this->arr_i16_1e8[i], this->arr_i16_1e8[i+0xfb]);
			_toolbox->LineTo(this->arr_i16_1e8[i], this->arr_i16_1e8[i+0xfb]);
			if (i % 0x19 == 0) {
				this->delayFromMarker(0x1);
			}
		}
	}
	// 131:1748
	this->var_i32_2 = _toolbox->TickCount();
	for (int i = 0x25; i <= 0x2f; i++) {
		_toolbox->ReleaseResource(this->arr_i32_0[i]);
	}

	this->delayFromMarker(0x3c);
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcBic);
	// well, this won't do
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(25).encode().c_str());
	this->drawTextRightAlign(0x181, 0x3c);

	// 131:17c8
	var_i32_2 = _toolbox->TickCount();
	for (int16 i = 0x30; i <= 0x31; i++) {
		arr_i32_0[i] = _toolbox->GetPicture(i);
	}
	for (int16 i = 0x4b; i <= 0x52; i++) {
		arr_i32_0[i] = _toolbox->GetPicture(i);
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
	var_str_76 = _zbasic->str(28) + _zbasic->chr(0x22); // the fourteen lost treasures of the land?
	drawTextRightAlign(0x181, 0x5e);
	for (int16 i = 0x30; i <= 0x31; i++) {
		_toolbox->ReleaseResource(arr_i32_0[i]);
	}
	for (int16 i = 0x32; i <= 0x38; i++) {
		arr_i32_0[i] = _toolbox->GetPicture(i);
		if (i == 0x37) {
			setPortBitsToPage(5);
			_zbasic->picture(0, 0, arr_i32_0[0x37]);
			_toolbox->ReleaseResource(arr_i32_0[0x37]);
			_toolbox->SetPortBits(var_i32_32);
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
	_toolbox->PenPat(arr_pat_194[1]);
	_toolbox->PenMode(kPatXor);
	for (int16 j = 6; j <= 9; j++) {
		arr_i32_41296[j]->copyFrom(*arr_i32_41296[j - 1]);
		// 131:1c7a
		setPortBitsToPage(j);
		if (j == 6) {
			_toolbox->PaintOval(arr_i16_41af4);
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
			_toolbox->PaintOval(arr_i16_41b0a);
		}
	}
	arr_i32_41296[10]->copyFrom(*arr_i32_41296[9]);
	// 131:1d98
	setPortBitsToPage(0xa);
	_toolbox->PaintOval(arr_i16_41afc);
	setPortBitsToPage(5);
	_toolbox->SetRect(arr_i16_1bc, 0xae, 0x8c, 0xb6, 0x94);
	_toolbox->InvertOval(arr_i16_1bc);
	_toolbox->SetRect(arr_i16_1bc, 0x151, 0x86, 0x159, 0x8e);
	_toolbox->InvertOval(arr_i16_1bc);
	_toolbox->SetPortBits(var_i32_32);
	sub_128_e1c();
	sub_128_e58();

	// 131:1e18
	// card gets vivisected
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
			_toolbox->InvertRect(var_i16_38);
		}
		var_i32_2 = _toolbox->TickCount();
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
		var_i32_2 = _toolbox->TickCount();
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
		var_i32_2 = _toolbox->TickCount();
		_zbasic->text(kPrologueFontFool, i, 0, kSrcXor);
		var_i16_1b6 += i;
		drawText(5 + i*2, var_i16_1b6);
		delayFromMarker(3);
	}
	// 131:2282
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x34]);
	var_i32_2 = _toolbox->TickCount();
	setPortBitsToPage(4);
	_toolbox->SetRect(arr_i16_1bc, 0, 0, 0x200, 0xdf);
	_toolbox->InvertRect(arr_i16_1bc);
	_toolbox->SetRect(arr_i16_1bc, 0, 0xdf, SCREEN_WIDTH, SCREEN_HEIGHT);
	_toolbox->PenMode(kPatOr);
	_toolbox->PenPat(arr_pat_194[1]);
	_toolbox->PaintRect(arr_i16_1bc);
	// 131:2316
	arr_i32_41296[0xb]->copyFrom(*arr_i32_41296[0x4].get());
	setPortBitsToPage(0xb);
	_zbasic->picture(0xb5, 0xa, arr_i32_0[0x38]);
	_toolbox->SetPortBits(var_i32_32);
	delayFromMarker(0x1e);
	_toolbox->PenNormal();
	var_i16_3e6 = 0x17c;
	for (int16 j = 0; j <= 0x14a; j++) {
		var_i32_2 = _toolbox->TickCount();
		if (j > 0xb4) {
			var_i16_3e6 = SCREEN_WIDTH;
		}
		// 131:2398
		for (int16 i = 0xa; i <= 0x14; i += 5) {
			_toolbox->MoveTo(0, j + _zbasic->rndInt(i));
			_toolbox->LineTo(var_i16_3e6, j + _zbasic->rndInt(i));
			// 131:23d8
		}
		_toolbox->MoveTo(0, j);
		_toolbox->LineTo(var_i16_3e6, j);
		if ((j % 4) == 0) {
			delayFromMarker(0);
		}
		// 131:2414
	}

	// 131:2422
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x35]);
	delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcBic);
	// well I for one am not content
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(34).encode().c_str());
	drawTextRightAlign(0x181, 0x3c);
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(35).encode().c_str());
	drawTextRightAlign(0x181, 0x4d);
	sub_128_e1c();
	sub_128_e58();
	_zbasic->picture(0x193, 0x2d, arr_i32_0[0x36]);

	// 131:24da
	fillRect(0, 0, 0x96, 0x18b, 2);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcBic);
	// show me one last puzzle if you dare
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(36).encode().c_str());
	drawTextRightAlign(0x181, 0x3c);
	var_str_76 = _zbasic->str(37);
	drawTextRightAlign(0x181, 0x4d);
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(38).encode().c_str());
	drawTextRightAlign(0x181, 0x5e);
	sub_128_e58();

	// 131:258a
	var_i32_40 = arr_i32_41296[5];

	for (int16 i = 1; i <= 0x1a; i++) {
		var_i16_5c.top = 0xab - (int16)(i*6.66f);
		var_i16_5c.left = 0x100 - i*0xa;
		var_i16_5c.bottom = 0xab + (int16)(i*6.66f);
		var_i16_5c.right = 0x100 + i*0xa;
		_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_5c, var_i16_5c, kSrcCopy, nullptr);
		_toolbox->Delay(0);
	}
	// 131:266e
	_toolbox->CopyBits(var_i32_40, var_i32_32, var_i16_38, var_i16_38, kSrcCopy, nullptr);
	_toolbox->SetRect(arr_i16_1bc, 0xae, 0x8c, 0xb6, 0x94);
	_toolbox->SetRect(arr_i32_1c4, 0x151, 0x86, 0x159, 0x8e);
	_toolbox->InsetRect(arr_i16_1bc, 3, 3);
	_toolbox->InsetRect(arr_i32_1c4, 3, 3);
	_toolbox->FillOval(arr_i16_1bc, arr_pat_194[0]);
	_toolbox->FillOval(arr_i32_1c4, arr_pat_194[0]);
	_toolbox->Delay(0);
	// 131:2720
	for (int i = 1; i <= 3; i++) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->InsetRect(arr_i16_1bc, -1, -1);
		_toolbox->InsetRect(arr_i32_1c4, -1, -1);
		_toolbox->FillOval(arr_i16_1bc, arr_pat_194[0]);
		_toolbox->FillOval(arr_i32_1c4, arr_pat_194[0]);
		delayFromMarker(1);
	}
	// 131:27a2
	copyScreenToPage(5);
	_toolbox->SetRect(arr_i16_1bc, 0xd0, 6, 0x133, 0x6c);
	_toolbox->PenMode(kPatXor);
	_toolbox->PenSize(3, 3);
	var_i16_18e = 10;
	for (int16 i = 355; i >= 180; i -= 5) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->FrameArc(arr_i16_1bc, i, var_i16_18e);
		var_i16_18e += 10;
		delayFromMarker(1);
	}
	// 131:281c
	for (int16 i = 0; i <= 6; i++) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->FrameOval(arr_i16_1bc);
		delayFromMarker(1);
	}
	// 131:284c
	for (int16 i = 5; i <= 0xa; i++) {
		var_i32_2 = _toolbox->TickCount();
		blitPageToScreen(i);
		delayFromMarker(1);
	}
	// 131:2878
	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	for (int16 i = 0; i <= 0x1e; i++) {
		var_i32_2 = _toolbox->TickCount();
		_toolbox->InvertRect(arr_i16_1bc);
		delayFromMarker(1);
	}
	// 131:28c2
	sub_128_50a(0xb, 0, SCREEN_WIDTH, 1);
	arr_i32_41296[5]->copyFrom(*arr_i32_41296[11]);
	setPortBitsToPage(5);
	_zbasic->picture(0x16, 0xde, arr_i32_0[0x4b]);
	// 131:2926
	for (int16 i = 6; i <= 7; i++) {
		arr_i32_41296[i]->copyFrom(*arr_i32_41296[5]);
	}
	setPortBitsToPage(6);
	_zbasic->picture(0x10, 0x54, arr_i32_0[0x4c]);
	setPortBitsToPage(7);
	_zbasic->picture(0x10, 0x54, arr_i32_0[0x4c]);
	_zbasic->picture(0x2, 0x50, arr_i32_0[0x52]);
	_toolbox->SetPortBits(var_i32_32);
	delay(0x1e);
	sub_128_50a(0x6, 0, SCREEN_WIDTH, 1);
	delay(0x1e);
	blitPageToScreen(7);
	delay(0x48);
	blitPageToScreen(6);
	delay(0x1e);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcOr);
	// 131:2a22
	// now what have I gotten myself into?
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(39).encode().c_str());
	drawText(0x3c, 0x2d);
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(40).encode().c_str());
	drawText(0x46, 0x3e);
	var_i32_2 = _toolbox->TickCount();
	for (int16 i = 6; i <= 0xa; i++) {
		arr_i32_41296[i]->copyFrom(*arr_i32_41296[5]);
	}
	setPortBitsToPage(6);
	_zbasic->picture(0x15, 0x55, arr_i32_0[0x4d]);
	setPortBitsToPage(7);
	_zbasic->picture(0x1c, 0x57, arr_i32_0[0x4e]);
	setPortBitsToPage(8);
	_zbasic->picture(0x1c, 0x57, arr_i32_0[0x4f]);
	setPortBitsToPage(9);
	_zbasic->picture(0x1d, 0x58, arr_i32_0[0x50]);
	setPortBitsToPage(0xa);
	_zbasic->picture(0x1d, 0x58, arr_i32_0[0x51]);
	// 131:2b7c
	for (int16 i = 1; i <= 0xe; i++) {
		arr_i16_1e8[i] = _zbasic->readDataInt();
		arr_i16_1e8[i + 0xfb] = _zbasic->readDataInt();
		arr_i16_1e8[i + 0x1f6] = _zbasic->readDataInt();
		arr_i16_1e8[i + 0x2f1] = _zbasic->readDataInt();
	}
	// 131:2c02
	_zbasic->text(kPrologueFontSmall, 0x9, 0, kSrcOr);
	for (int16 i = 1; i <= 0xe; i++) {
		// THEBOOKOFTHOTH
		var_str_76 = _zbasic->midStr(_zbasic->str(41), i, 1);
		drawText(arr_i16_1e8[i], arr_i16_1e8[i + 0xfb]);
	}
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcOr);
	// that's it! the answer is the book of thoth
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(42).encode().c_str());
	drawText(0x32, 0x2d);
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(43).encode().c_str());
	drawText(0x3c, 0x3e);
	// 131:2cf6
	_toolbox->SetPortBits(var_i32_32);
	delayFromMarker(0xd2);
	sub_128_e1c();
	sub_128_e58();
	sub_128_e1c();
	blitPageToScreen(6);
	delay(3);
	blitPageToScreen(7);
	delay(3);
	blitPageToScreen(8);
	var_i32_2 = _toolbox->TickCount();
	// 131:2d3a
	for (int16 i = 0x30; i <= 0x36; i++) {
		_toolbox->ReleaseResource(arr_i32_0[i]);
	}
	_toolbox->ReleaseResource(arr_i32_0[0x38]);
	for (int16 i = 0x4b; i <= 0x52; i++) {
		_toolbox->ReleaseResource(arr_i32_0[i]);
	}
	for (int16 i = 0x29; i <= 0x2a; i++) {
		arr_i32_0[i] = _toolbox->GetPicture(i);
	}
	for (int16 i = 0x53; i <= 0x5b; i++) {
		arr_i32_0[i] = _toolbox->GetPicture(i);
	}
	delayFromMarker(0x96);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcOr);
	// 131:2e1a
	// wait, what's this?
	var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(44).encode().c_str());
	_toolbox->MoveTo(0x46, 0x3e);
	_toolbox->DrawString(var_str_76);
	delay(0x96);
	blitPageToScreen(0x9);
	// 131:2e58
	for (int16 i = 1; i <= 0xe; i++) {
		var_str_76 = _zbasic->midStr(_zbasic->str(45), i, 1);
		for (int16 l = 0; l <= 1; l++) {
			var_i16_18e = 0x21;
			for (int16 j = 0; j <= 1; j++) {
				arr_f64_41bbe[j] = arr_i16_1e8[j*0xfb + i];
				arr_f64_41bbe[j + 2] = (arr_i16_1e8[(j+2)*0xfb + i] - arr_i16_1e8[j*0xfb + i]) / var_i16_18e;
			}
			// 131:2f66
			if (l == 0) {
				_zbasic->text(kPrologueFontSmall, 9, 0, kSrcOr);
				drawText(arr_i16_1e8[i], arr_i16_1e8[i + 0xfb]);
			}
			// 131:2fc4
			_zbasic->text(kPrologueFontSmall, 0x9, 0, kSrcXor);
			for (int16 k = 1; k <= var_i16_18e-1; k++) {
				arr_f64_41bbe[0] += arr_f64_41bbe[0 + 2];
				int16 x = (int16)arr_f64_41bbe[0];
				arr_f64_41bbe[1] += arr_f64_41bbe[1 + 2];
				int16 y = (int16)arr_f64_41bbe[1];
				// 131:3078
				drawText(x, y);
			}
			_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcOr);
			drawText(arr_i16_1e8[0x1f6 + i], arr_i16_1e8[0x2f1 + i]);
			delay(1);
		}
	}
	// 131:3130
	delay(0x1e);
	blitPageToScreen(0xa);
	var_i32_2 = _toolbox->TickCount();
	copyScreenToPage(0xa);
	setPortBitsToPage(0xa);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcBic);
	// that's it! the answer is the book of thoth
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(46).encode().c_str());
	drawText(0x32, 0x2d);
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(47).encode().c_str());
	drawText(0x3c, 0x3e);
	arr_i32_41296[0xb]->copyFrom(*arr_i32_41296[0xa]);
	setPortBitsToPage(0xb);
	arr_i16_41af4.top = 0xa;
	arr_i16_41af4.left = 0xb4;
	arr_i16_41af4.bottom = 0x14c;
	arr_i16_41af4.right = 0x1f6;
	_toolbox->InvertOval(arr_i16_41af4);
	for (int16 i = 5; i <= 0x9; i++) {
		arr_i32_41296[i]->copyFrom(*arr_i32_41296[4]);
	}
	// 131:327e
	setPortBitsToPage(4);
	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, 0xdf);
	_toolbox->InvertRect(arr_i16_1bc);
	_zbasic->picture(0xd, 0xf0, arr_i32_0[0x58]);
	_zbasic->picture(0x186, 0xc, arr_i32_0[0x59]);
	setPortBitsToPage(5);
	_zbasic->picture(0xf, 0x68, arr_i32_0[0x53]);
	setPortBitsToPage(6);
	_zbasic->picture(0x11, 0x8a, arr_i32_0[0x54]);
	setPortBitsToPage(7);
	_zbasic->picture(0x13, 0xa9, arr_i32_0[0x55]);
	setPortBitsToPage(8);
	_zbasic->picture(0x11, 0xcd, arr_i32_0[0x56]);
	setPortBitsToPage(9);
	_zbasic->picture(0xf, 0xf3, arr_i32_0[0x57]);
	// 131:339c
	_toolbox->SetPortBits(var_i32_32);
	delayFromMarker(0x96);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcBic);
	// that's it! the answer is the book of thoth
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(48).encode().c_str());
	drawText(0x32, 0x2d);
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(49).encode().c_str());
	drawText(0x3c, 0x3e);
	// 131:341c
	sub_128_50a(0xb, 0xb0, 0x1f6, 0);
	sub_128_50a(0x5, 0xb0, 0x1f6, 0);
	delay(0xa);
	for (int16 i = 5; i <= 9; i++) {
		blitPageToScreen(i);
		delay(3);
	}
	_zbasic->picture(0xd, 0xf0, arr_i32_0[0x58]);
	_toolbox->SetRect(arr_i16_1bc, 0, 0, SCREEN_WIDTH, 0xdf);
	// lightning flash in sky
	for (int16 i = 0x1b; i >= 0; i--) {
		_toolbox->InvertRect(arr_i16_1bc);
		// this is fudged considerably
		// 131:34c4
		_toolbox->Delay(i/4);
	}
	// 131:34dc
	blitPageToScreen(4);
	var_i32_2 = _toolbox->TickCount();
	var_i16_3ec = 0x8c;
	var_i16_3ee = 0x3c;
	setPortBitsToPage(9);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(var_i16_3ec, var_i16_3ee, arr_i32_0[0x29]);
	setPortBitsToPage(0xa);
	fillRect(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 0);
	_zbasic->picture(var_i16_3ec, var_i16_3ee, arr_i32_0[0x2a]);
	_toolbox->SetPortBits(var_i32_32);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcXor);
	// the book of thoth?
	var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(50).encode().c_str());
	drawTextRightAlign(0x1a4, 0x2d);
	var_i32_2 = _toolbox->TickCount();
	arr_i32_41296[5]->copyFrom(*arr_i32_41296[4]);
	setPortBitsToPage(5);
	_zbasic->picture(0x171, 0xb, arr_i32_0[0x5b]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcXor);
	// here is the book of thoth
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(51).encode().c_str());
	drawTextRightAlign(0x1a4, 0x2d);
	var_i16_5c.top = var_i16_3ee;
	var_i16_5c.left = var_i16_3ec;

	// 131:366e
	var_i16_5c.bottom = var_i16_3ee + 100;
	var_i16_5c.right = var_i16_3ec + 200;
	var_i32_4e->copyFrom(*arr_i32_41296[4]);
	var_i32_40 = arr_i32_41296[9];
	_toolbox->CopyBits(var_i32_40, var_i32_4e, var_i16_5c, var_i16_5c, kSrcBic, 0);
	var_i32_40 = arr_i32_41296[0xa];
	_toolbox->CopyBits(var_i32_40, var_i32_4e, var_i16_5c, var_i16_5c, kSrcOr, 0);
	for (int16 i = 6; i <= 8; i++) {
		arr_i32_41296[i]->copyFrom(*var_i32_4e);
	}
	setPortBitsToPage(6);
	_zbasic->picture(0x171, 0xb, arr_i32_0[0x5b]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcXor);

	// here is the book of thoth
	var_str_76 = Common::U32String::format("\"%s", _zbasic->str(52).encode().c_str());
	drawTextRightAlign(0x1a4, 0x2d);
	setPortBitsToPage(7);
	_zbasic->picture(0x181, 0xf, arr_i32_0[0x5a]);
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcXor);
	// and your answer is incorrect
	var_str_76 = Common::U32String::format("%s\"", _zbasic->str(53).encode().c_str());
	drawTextRightAlign(0x1a4, 0x2d);
	setPortBitsToPage(8);

	// 131:3806
	_zbasic->text(kPrologueFontFool, 0xc, 0, kSrcXor);

}

void FoolPrologue::finaleDrawLoadingMsg() {
	// FIXME: we don't share the menu surface here yet
	return;
	// 131:4dc0
	_toolbox->SetPort(this->var_i32_c);
	_zbasic->text(0, 0xc, Graphics::kMacFontRegular, kSrcOr);
	// Loading Finale
	this->var_str_76 = _zbasic->str(78);
	this->var_str_76 += Common::U32String::format("%d%%", this->var_i16_3ce);
	// 131:4e08
	this->fillRect(0, 0, 7, 0x13, this->var_i16_10 - 7);

	this->drawTextCenterAlign(this->var_i16_10 / 2, 0xe);
	_toolbox->_defaultMenu->setOverlayDirty(true);
	_toolbox->SetPort(this->var_i32_8);
}

void FoolPrologue::finaleDrawFoolUhOh() {
	// 131:4e48
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcOr);

	// Uh oh . . .
	this->var_str_76 = Common::U32String::format("\"%s\"", _zbasic->str(80).encode().c_str());

	this->drawText(0x1ae, 0xb4);
}

void FoolPrologue::finaleStartText() {
	// 131:4e98
	_zbasic->text(kPrologueFontFool, 0xc, Graphics::kMacFontRegular, kSrcXor);
	this->var_i16_3fc = 0xa0;
	// "And so the fool heeded the advice of the magician..."
	this->var_str_76 = _zbasic->str(81);
	this->drawText(0x96, this->var_i16_3fc);

	this->var_str_76 = _zbasic->str(82);
	this->drawText(0xa0, this->var_i16_3fc + 0xf);

	this->var_str_76 = _zbasic->str(83);
	this->drawText(0xaa, this->var_i16_3fc + 0x1e);

	this->var_str_76 = _zbasic->str(84);
	this->drawText(0xc8, this->var_i16_3fc + 0x2d);

	this->var_str_76 = _zbasic->str(85);
	this->drawText(0xe6, this->var_i16_3fc + 0x3c);

	this->drawClickMessage();
}

void FoolPrologue::sub_131_4f96(int16 offset) {
	// 131:4f96
	_toolbox->MoveTo(
		this->arr_i16_1e8[offset],
		this->arr_i16_1e8[offset+0xfb]
	);

	// 131:4fd4
	_toolbox->LineTo(
		this->arr_i16_1e8[offset] + this->arr_i16_1e8[offset+0x1f6],
		this->arr_i16_1e8[offset+0xfb]
	);
}


} // End of namespace Fool
