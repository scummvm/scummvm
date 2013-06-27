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

/* ENID		Edna's manager. */

#include "common/scummsys.h"
#include "common/textconsole.h"

#include "avalanche/enid2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/trip6.h"
#include "avalanche/timeout2.h"
#include "avalanche/celer2.h"
#include "avalanche/sequence2.h"
//#include "fileunit.h"
//#include "basher.h"


namespace Avalanche {

	namespace Enid {

	const Common::String crlf = Common::String(char(15)) + Common::String(char(12));
	const char tab = '\t';
	const char eof_ = '\n';

	const Common::String ednafirst =
		Common::String("This is an EDNA-based file, saved by a Thorsoft game. Good luck!") + /*64*/
		crlf + eof_ + crlf + crlf + /*7*/
		tab + "Glory to God in the highest," + crlf + /*31*/
		tab + "and on earth peace, goodwill toward men." + /*42*/
		crlf + tab + tab + tab + tab + /*6*/
		"Luke 2:14." + /*10*/
		crlf + crlf + crlf + /* 6 */
		"1234567890" +crlf; /*11*/

	const Common::String ednaid = Common::String("TT") + char(261) + char(60) + char(1) + char(165) + char(261) + char(231) + char(261);

	const int16 ttage = 18;
	const Common::String ttwashere = "Thomas was here ";

	bool bug;



	Common::String expanddate(byte d, byte m, uint16 y);
 
	static Common::String month;

	static Common::String day;

	static void addon(Common::String x) {
		//month[0]--;
		month = month + x;
	}

	Common::String expanddate(byte d, byte m, uint16 y) {
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

		day = Gyro::strf(d);

		if (((d >= 0) && (d <= 9)) || ((d >= 21) && (d <= 31)))
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

		expanddate_result = day + ' ' + month + ' ' + Gyro::strf(y);
		return expanddate_result;
	}



	void edna_save(Common::String name);

	static void show_bug(char icon, Common::String strn) {
		Scrolls::display(Common::String("\7\6\23") + icon + "\26\r" + strn + '\15');
	}

	static bool test_bug(byte what) {
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

	void edna_save(Common::String name) {
		warning("STUB: Enid::edna_save()");
	}

	void loaderror(Common::String x, char icon) {
		if (Gyro::holdthedawn) {
			Gyro::holdthedawn = false;
			Lucerna::dawn();
		}
		Scrolls::display(Common::String('\7') + '\6' + '\23' + icon + '\26' + "Loading error:  " + "\r\r\22" + x);
		bug = true;
	}



	typedef char fourtype[5];



	const fourtype avaricius_file = "Avvy";

	void edna_load(Common::String name) {
		warning("STUB: Enid::edna_load()");
	}



	void dir(Common::String where);

	static Common::String path, groi;

	static void showheader() {
		Scrolls::display(Common::String("Dir: ") + path + "\r\r\4");
	}

	void dir(Common::String where) { /* OK, it worked in Avaricius, let's do it in Avalot! */
		warning("STUB: Enid::dir()");
	}

	void avvy_background() {    /* Not really a filing procedure,
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
		Lucerna::blitfix();
	}

	void to_sundry(Gyro::sundry &sund) {
		{
			sund.qenid_filename = Gyro::enid_filename;
			sund.qsoundfx = Gyro::soundfx;
			sund.qthinks = Gyro::thinks;
			sund.qthinkthing = Gyro::thinkthing;
		}
	}

	void from_sundry(Gyro::sundry sund) {
		{
			Gyro::enid_filename = sund.qenid_filename;
			Gyro::soundfx = sund.qsoundfx;
			Gyro::thinks = sund.qthinks;
			Gyro::thinkthing = sund.qthinkthing;
		}
	}

	void restore_dna() {
		uint16 here, fv;
		Gyro::sundry sund;

		warning("STUB: Enid::restore_dna()");
	}

	void edna_reload() {

		restore_dna();

		Gyro::seescroll = true;  /* This prevents display of the new sprites before the
	  new picture is loaded. */

		Lucerna::major_redraw();

		Gyro::whereis[Gyro::pavalot] = Gyro::dna.room;

		Gyro::alive = true;

		Lucerna::objectlist();

		if (Gyro::holdthedawn) {
			Gyro::holdthedawn = false;
			Lucerna::dawn();
		}
	}

	void back_to_bootstrap(byte what) {
		warning("STUB: Enid::back_to_bootstrap()");
	}

	bool there_was_a_problem() {
		return bug;
	}

	} // End of namespace Enid.

} // End of namespace Avalanche.
