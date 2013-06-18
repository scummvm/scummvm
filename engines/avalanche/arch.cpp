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

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 ENID             Edna's manager. */

#define __arch_implementation__
/* Loads/ saves files. */

#include "arch.h"


/*#include "Dos.h"*/
#include "scrolls.h"

namespace Avalanche {

struct ednahead { /* Edna header */
	/* This header starts at byte offset 177 in the .ASG file. */
	array<1, 9, char> id;   /* signature */
	word revision; /* EDNA revision, here 2 (1=dna256) */
	varying_string<50> game; /* Long name, eg Lord Avalot D'Argent */
	varying_string<15> shortname; /* Short name, eg Avalot */
	word number; /* Game's code number, here 2 */
	word ver; /* Version number as integer (eg 1.00 = 100) */
	varying_string<5> verstr; /* Vernum as string (eg 1.00 = "1.00" */
	varying_string<12> filename; /* Filename, eg AVALOT.EXE */
	byte os; /* Saving OS (here 1=DOS. See below for others.) */

	/* Info on this particular game */

	varying_string<8> fn; /* Filename (not extension ('cos that's .ASG)) */
	byte d, m; /* D, M, Y are the Day, Month & Year this game was... */
	word y;  /* ...saved on. */
	varying_string<40> desc; /* Description of game (same as in Avaricius!) */
	word len; /* Length of DNA (it's not going to be above 65535!) */

	/* Quick reference & miscellaneous */

	word saves; /* no. of times this game has been saved */
	integer cash; /* contents of your wallet in numerical form */
	varying_string<20> money; /* ditto in string form (eg 5/-, or 1 denarius)*/
	word points; /* your score */

	/* DNA values follow, then footer (which is ignored) */
};
/* Possible values of edhead.os:
   1 = DOS        4 = Mac
   2 = Windows    5 = Amiga
   3 = OS/2       6 = ST */
const string crlf = string('\15') + '\12';

const array<1, 177, char> ednafirst =
    string("This is an EDNA-based file, saved by a Thorsoft game. Good luck!") + crlf +
    '\32' + /* 67 bytes... */
    crlf + crlf + /* 71 bytes... */
    "12345678901234567890123456789012345678901234567890" +
    "12345678901234567890123456789012345678901234567890" +
    "123456";

const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
const string ednaid = string("TT") + '\261' + '\60' + '\1' + '\165' + '\261' + '\231' + '\261';

const integer ttage = 18;
const char ttwashere[] = "Thomas was here ";

void save(string name) {
	untyped_file f;
	edhead eh;
	word groi;
	string groi2;
	;
	fillchar(eh, sizeof(eh), '\261'); /* Fill up the edhead */

	dna.saves += 1; /* It's been saved one more time... */

	{
		;

		/* Info about this program */

		id = ednaid; /* Edna's signature */
		revision = 2; /* Second revision of .ASG format */
		game = "Lord Avalot d'Argent"; /* Title of game */
		shortname = "Avalot";
		number = 2; /* Second Avvy game */
		ver = 100; /* Version 1.00 */
		verstr = "1.00"; /* ditto */
		filename = "AVALOT.EXE"; /* program's filename */
		os = 1; /* Saved under DOS */

		/* Info on this particular game */

		fsplit(name, groi2, fn, groi2); /* fn = filename of this game */
		getdate(d, m, y, groi); /* Day, month & year when the game was saved */
		desc = roomname; /* Description of game (same as in Avaricius!) */
		len = sizeof(dna); /* Length of DNA (it's not going to be above 65535!) */

		/* Quick reference & miscellaneous */

		saves = dna.saves; /* no. of times this game has been saved */
		cash = dna.pence; /* contents of your wallet in numerical form */
		money = lsd(); /* ditto in string form (eg 5/-, or 1 denarius)*/
		points = dna.score; /* your score */
	}

	assign(f, name);
	rewrite(f, 1);

	blockwrite(f, ednafirst, 177);
	blockwrite(f, eh, sizeof(eh));
	blockwrite(f, dna, sizeof(dna));

	for (groi = 1; groi <= ttage; groi ++)
		blockwrite(f, ttwashere, sizeof(ttwashere));

	close(f);
}

} // End of namespace Avalanche.