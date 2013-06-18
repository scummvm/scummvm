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

#define __sezunit_implementation__


#include "sezunit.h"

namespace Avalanche {

struct markertype {
	word length;
	longint offset;
	byte checksum;
};

untyped_file f;
longint number;
markertype marker;
word fv;
byte sum;

void sez_setup() {   /* This procedure sets up the Sez system (obviously!) */
	;

	/* Set up variables */

	fillchar(chain, sizeof(chain), '\261'); /* blank out gunk in "chain" */
	chainsize = 0; /* it's empty (for now...) */
	sezerror = sezok; /* everything's fine! */

	/* Set up AVALOT.SEZ */

	assign(f, "avalot.sez");
	reset(f, 1);
	seek(f, 255);
	blockread(f, sezhead, sizeof(sezhead));
	if (ioresult != 0)  {
		;    /* too short */
		sezerror = sezgunkyfile;
		return;
	}
	{
		;
		if ((sezhead.initials != "TT") || (sezhead.gamecode != thisgamecode)
		        || (sezhead.revision != thisvercode)) {
			;
			sezerror = sezgunkyfile; /* not a valid file */
			return;
		}
	}
}

byte sumup() {
	word fv;
	byte total;
	byte sumup_result;
	;
	total = 0;
	for (fv = 0; fv <= chainsize; fv ++) {
		;
		total += ord(chain[fv]);
	}
	sumup_result = total;
	return sumup_result;
}

void getchain(longint number) {
	;
	seek(f, longint(262) + number * longint(7));
	blockread(f, marker, 7);
	{
		;
		seek(f, longint(270) + sezhead.chains * longint(7) + marker.offset);
		blockread(f, chain, marker.length + 1);
		for (fv = 0; fv <= marker.length; fv ++) chain[fv] -= 3 + 177 * fv * marker.length; /* unscramble */
		chainsize = marker.length;
		if (sumup() != marker.checksum) {
			;
			sezerror = sezhacked;
			return;
		}
	}
	close(f);
	sezerror = sezok; /* nowt went wrong */
}

} // End of namespace Avalanche.