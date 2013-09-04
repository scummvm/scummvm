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

//#include "fileunit.h"
//#include "basher.h"


namespace Avalanche {

Enid::Enid(AvalancheEngine *vm) {
	_vm = vm;
}

const Common::String Enid::crlf = Common::String(char(15)) + Common::String(char(12));
const char Enid::tab = '\t';
const char Enid::eof_ = '\n';

const Common::String Enid::ednafirst =
	Common::String("This is an EDNA-based file, saved by a Thorsoft game. Good luck!") + /*64*/
	crlf + eof_ + crlf + crlf + /*7*/
	tab + "Glory to God in the highest," + crlf + /*31*/
	tab + "and on earth peace, goodwill toward men." + /*42*/
	crlf + tab + tab + tab + tab + /*6*/
	"Luke 2:14." + /*10*/
	crlf + crlf + crlf + /* 6 */
	"1234567890" +crlf; /*11*/

const Common::String Enid::ednaid = Common::String("TT") + char(261) + char(60) + char(1) + char(165) + char(261) + char(231) + char(261);

const int16 Enid::ttage = 18;
const Common::String Enid::ttwashere = "Thomas was here ";

const Enid::fourtype Enid::avaricius_file = "Avvy";



void Enid::addon(Common::String x) {
	//month[0]--;
	month = month + x;
}

Common::String Enid::expanddate(byte d, byte m, uint16 y) {
	const Common::String months[12] = {
		"Jan#", "Febr#", "March", "April", "May", "June", "July", "August",
		"Septem*", "Octo*", "Novem*", "Decem*"		
	};

	Common::String expanddate_result;
	month = months[m];
	switch (month[month.size()]) {
	case '#':
		addon("uary");
		break;
	case '*':
		addon("ber");
		break;
	}

	day = _vm->_gyro->strf(d);

	if (((d >= 0) && (d <= 9)) || ((d >= 21) && (d <= 31))) {
		switch (d % 10) {
		case 1:
			day = day + "st";
			break;
		case 2:
			day = day + "nd";
			break;
		case 3:
			day = day + "rd";
			break;
		default:
			day = day + "th";
		}
	}

	expanddate_result = day + ' ' + month + ' ' + _vm->_gyro->strf(y);
	return expanddate_result;
}

void Enid::show_bug(char icon, Common::String strn) {
	_vm->_scrolls->display(Common::String("\7\6\23") + icon + "\26\r" + strn + '\15');
}

bool Enid::test_bug(byte what) {
	bool test_bug_result;
	if (what == 0)  {
		test_bug_result = false;
		return test_bug_result;
	}
	switch (what) {
	case 2:
		show_bug('7', "Error in filename!");
		break;
	case 101:
		show_bug('6', "Disk full!");
		break;
	case 150:
		show_bug('4', "Disk is write-protected!");
		break;
	default:
		show_bug('B', "Saving error!");
	}
	test_bug_result = true;
	return test_bug_result;
}

void Enid::edna_save(Common::String name) {
	warning("STUB: Enid::edna_save()");
}

void Enid::loaderror(Common::String x, char icon) {
	if (_vm->_gyro->holdthedawn) {
		_vm->_gyro->holdthedawn = false;
		_vm->_lucerna->dawn();
	}
	_vm->_scrolls->display(Common::String('\7') + '\6' + '\23' + icon + '\26' + "Loading error:  " + "\r\r\22" + x);
	bug = true;
}

void Enid::edna_load(Common::String name) {
	warning("STUB: Enid::edna_load()");
}

void Enid::showheader() {
	_vm->_scrolls->display(Common::String("Dir: ") + path + "\r\r\4");
}

void Enid::dir(Common::String where) { /* OK, it worked in Avaricius, let's do it in Avalot! */
	warning("STUB: Enid::dir()");
}

void Enid::avvy_background() {    /* Not really a filing procedure,
	but it's only called just before edna_load, so I thought I'd put it
	in Enid instead of, say, Lucerna. */
	
	/* port[$3c4]:=2; port[$3ce]:=4; port[$3C5]:=1; port[$3CF]:=1; { Blue. }*/
	/*
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
	*/
}

void Enid::to_sundry(sundry &sund) {
	sund.qenid_filename = _vm->_gyro->enid_filename;
	sund.qsoundfx = _vm->_gyro->soundfx;
	sund.qthinks = _vm->_gyro->thinks;
	sund.qthinkthing = _vm->_gyro->thinkthing;
}

void Enid::from_sundry(sundry sund) {
	_vm->_gyro->enid_filename = sund.qenid_filename;
	_vm->_gyro->soundfx = sund.qsoundfx;
	_vm->_gyro->thinks = sund.qthinks;
	_vm->_gyro->thinkthing = sund.qthinkthing;
}

void Enid::restore_dna() {
//	uint16 here, fv;
//	sundry sund;

	warning("STUB: Enid::restore_dna()");
}

void Enid::edna_reload() {

	restore_dna();

	_vm->_gyro->seescroll = true;  /* This prevents display of the new sprites before the
	new picture is loaded. */

	_vm->_lucerna->major_redraw();

	_vm->_gyro->whereis[_vm->_gyro->pavalot - 150] = _vm->_gyro->dna.room;

	_vm->_gyro->alive = true;

	_vm->_lucerna->objectlist();

	if (_vm->_gyro->holdthedawn) {
		_vm->_gyro->holdthedawn = false;
		_vm->_lucerna->dawn();
	}
}

void Enid::back_to_bootstrap(byte what) {
	warning("STUB: Enid::back_to_bootstrap()");
}

bool Enid::there_was_a_problem() {
	return bug;
}


} // End of namespace Avalanche.
