/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* ENID		Edna's manager. Loads/saves files. */

#include "avalanche/avalanche.h"

#include "avalanche/enid2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/trip6.h"
#include "avalanche/timeout2.h"
#include "avalanche/celer2.h"
#include "avalanche/sequence2.h"

#include "common/textconsole.h"

namespace Avalanche {

Enid::Enid(AvalancheEngine *vm) {
	_vm = vm;
}

//  CHECKME: useless?
#if 0
const Common::String Enid::kCrlf = Common::String(char(15)) + Common::String(char(12));
const char Enid::kTab = '\t';
const char Enid::kEof = '\n';

const Common::String Enid::kEdnaFirst =
	Common::String("This is an EDNA-based file, saved by a Thorsoft game. Good luck!") + // 64
	kCrlf + kEof + kCrlf + kCrlf + // 7
	kTab + "Glory to God in the highest," + kCrlf + // 31
	kTab + "and on earth peace, goodwill toward men." + // 42
	kCrlf + kTab + kTab + kTab + kTab + // 6
	"Luke 2:14." + // 10
	kCrlf + kCrlf + kCrlf + // 6
	"1234567890" +kCrlf; // 11

const Common::String Enid::kEdnaId = Common::String("TT\xB1\x30\x01\x75\xB1\x99\xB1");
const int16 Enid::kAge = 18;
const Common::String Enid::kWasHere = "Thomas was here ";
const Enid::FourType Enid::kAvariciusFile = "Avvy";

/**
 * Expand month name
 * @remarks	Originally called 'addon'
 */
void Enid::expandMonthName(Common::String x) {
	_month += x;
}

Common::String Enid::expandDate(byte d, byte m, uint16 y) {
	const Common::String months[12] = {
		"Jan#", "Febr#", "March", "April", "May", "June", "July", "August",
		"Septem*", "Octo*", "Novem*", "Decem*"		
	};

	Common::String expanddate_result;
	_month = months[m];
	switch (_month[_month.size()]) {
	case '#':
		expandMonthName("uary");
		break;
	case '*':
		expandMonthName("ber");
		break;
	}

	_day = _vm->_gyro->strf(d);

	// d is always positive
	if ((d <= 9) || ((d >= 21) && (d <= 31))) {
		switch (d % 10) {
		case 1:
			_day = _day + "st";
			break;
		case 2:
			_day = _day + "nd";
			break;
		case 3:
			_day = _day + "rd";
			break;
		default:
			_day = _day + "th";
		}
	}

	expanddate_result = _day + ' ' + _month + ' ' + _vm->_gyro->strf(y);
	return expanddate_result;
}

void Enid::showBug(char icon, Common::String strn) {
	_vm->_scrolls->display(Common::String("\7\6\23") + icon + "\26\r" + strn + '\15');
}

bool Enid::testBug(byte what) {
	bool test_bug_result;
	if (what == 0)  {
		test_bug_result = false;
		return test_bug_result;
	}
	switch (what) {
	case 2:
		showBug('7', "Error in filename!");
		break;
	case 101:
		showBug('6', "Disk full!");
		break;
	case 150:
		showBug('4', "Disk is write-protected!");
		break;
	default:
		showBug('B', "Saving error!");
	}
	test_bug_result = true;
	return test_bug_result;
}

void Enid::ednaSave(Common::String name) {
	warning("STUB: Enid::ednaSave()");
}

void Enid::loadError(Common::String x, char icon) {
	if (_vm->_gyro->holdthedawn) {
		_vm->_gyro->holdthedawn = false;
		_vm->_lucerna->dawn();
	}
	_vm->_scrolls->display(Common::String('\7') + '\6' + '\23' + icon + '\26' + "Loading error:  " + "\r\r\22" + x);
	_bug = true;
}

void Enid::ednaLoad(Common::String name) {
	warning("STUB: Enid::ednaLoad()");
}

void Enid::showHeader() {
	_vm->_scrolls->display(Common::String("Dir: ") + _path + "\r\r\4");
}

void Enid::avvyBackground() {    
	// Not really a filing procedure,
	// but it's only called just before edna_load, so I thought I'd put it
	// in Enid instead of, say, Lucerna.
	
#if 0
	port[$3c4]:=2; port[$3ce]:=4; port[$3C5]:=1; port[$3CF]:=1; { Blue. }

	asm
		mov dx,$3c4; mov al,2; out dx,al; { Set up the VGA to use the "blue" }
		mov dx,$3ce; mov al,4; out dx,al; { register. }
		mov dx,$3c5; mov al,1; out dx,al;
		mov dx,$3cf;           out dx,al;

		mov bx,$A000; call far ptr @drawup;
		mov bx,$A400; call far ptr @drawup;

		jmp @the_end;

		@drawup:

		mov es,bx; { The segment to copy it to... }
		mov di,$370;  { The offset (10 pixels down, plus 1 offset.) }

		mov cx,10;
		mov ax,$AA4A; call far ptr @sameline; { Print "AVVY" }
		mov ax,$AEAA; call far ptr @sameline;
		mov ax,$A4EA; call far ptr @sameline;
		mov ax,$44A4; call far ptr @sameline;

		mov cx,9;
		mov ax,$AAA4; call far ptr @sameline; { Print "YAVV" }
		mov ax,$AAEA; call far ptr @sameline;
		mov ax,$AA4E; call far ptr @sameline;
		mov ax,$444A; call far ptr @sameline;

		mov ax,$4AAA; call far ptr @sameline; { Print "VYAV" }
		mov ax,$AAAE; call far ptr @sameline;
		mov ax,$EAA4; call far ptr @sameline;
		mov ax,$A444; call far ptr @sameline;

		mov ax,$A4AA; call far ptr @sameline; { Print "VVYA" }
		mov ax,$EAAA; call far ptr @sameline;
		mov ax,$4EAA; call far ptr @sameline;
		mov ax,$4A44; call far ptr @sameline;

		ret;


		{ Replicate the same line many times. }

		@sameline:
		{ Requires:
			what to copy in AX,
			how many lines in CX, and
			original offset in DI. }
		push cx;
		push di;

		@samelineloop:

		push cx;
		mov cx,40; { No. of times to repeat it on one line. }

		repz stosw; { Fast uint16-copying }

		pop cx;

		add di,1200; { The next one will be 16 lines down. }

		loop @samelineloop;
		pop di;
		add di,80;
		pop cx;

		ret;

		@the_end:
		end;
#endif
}

void Enid::toSundry(sundry &sund) {
	sund.qenid_filename = _vm->_gyro->enid_filename;
	sund.qsoundfx = _vm->_gyro->soundfx;
	sund.qthinks = _vm->_gyro->thinks;
	sund.qthinkthing = _vm->_gyro->thinkthing;
}

void Enid::fromSundry(sundry sund) {
	_vm->_gyro->enid_filename = sund.qenid_filename;
	_vm->_gyro->soundfx = sund.qsoundfx;
	_vm->_gyro->thinks = sund.qthinks;
	_vm->_gyro->thinkthing = sund.qthinkthing;
}

void Enid::restoreDna() {
//	uint16 here, fv;
//	sundry sund;

	warning("STUB: Enid::restoreDna()");
}

void Enid::ednaReload() {

	restoreDna();

	_vm->_gyro->seescroll = true;  // This prevents display of the new sprites before the
								   // new picture is loaded.

	_vm->_lucerna->major_redraw();

	_vm->_gyro->_whereIs[_vm->_gyro->pavalot - 150] = _vm->_gyro->dna.room;

	_vm->_gyro->alive = true;

	_vm->_lucerna->objectlist();

	if (_vm->_gyro->holdthedawn) {
		_vm->_gyro->holdthedawn = false;
		_vm->_lucerna->dawn();
	}
}

bool Enid::thereWasAProblem() {
	return _bug;
}

#endif

void Enid::dir(Common::String where) { // OK, it worked in Avaricius, let's do it in Avalot!
	warning("STUB: Enid::dir()");
}
void Enid::backToBootstrap(byte what) {
	warning("STUB: Enid::back_to_bootstrap()");
}

} // End of namespace Avalanche.
