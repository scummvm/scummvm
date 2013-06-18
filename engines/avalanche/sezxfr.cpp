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

/*#include "Crt.h"*/
/*$V-*/

namespace Avalanche {

text t;
string x;
matrix<1, 100, 1, 2, string> data;
word dp, ip, fv;
array<1, 20, word> index1;
array<1, 20, varying_string<10> > names;
array<1, 1777, longint> thumb;
longint total;
untyped_file f;

void strip() {
	;
	x = copy(x, 2, 255);
}

void squish(string &q) {
	string n;
	boolean ctrl;
	byte fv;
	;
	ctrl = false;
	n = "";
	for (fv = 1; fv <= length(q); fv ++)
		if (q[fv] == '^')  ctrl = true;
		else {
			;     /* not a caret */
			if (ctrl)  q[fv] = chr(ord(upcase(q[fv])) - 64);
			n = n + q[fv];
			ctrl = false;
		}
	while (n[length(n)] == '\40')  n[0] -= 1; /* strip trailing spaces */
	for (fv = 1; fv <= length(n); fv ++) n[fv] += 177; /* scramble */
	q = n;
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	dp = 0;
	ip = 0;
	fillchar(data, sizeof(data), '\0');
	fillchar(thumb, sizeof(thumb), '\261');
	fillchar(index1, sizeof(index1), '\3');
	assign(t, "v:sez.dat");
	reset(t);
	while (! eof(t)) {
		;
		t >> x >> NL;
		switch (x[1]) {
		case ';': {
			;
			textattr = lightred;
			strip();
		}
		break;
		case ':': {
			;
			textattr = lightblue;
			strip();
			if (dp > 0)  squish(data[dp][2]);
			dp += 1;
			data[dp][1] = x;
			if (pos("*", x) > 0) {
				;     /* index */
				ip += 1;
				index1[ip] = dp;
				names[ip] = copy(x, 1, pos("*", x) - 1);
			}
		}
		break;
		default: {
			;
			textattr = white;
			data[dp][2] = data[dp][2] + x + '\40';
		}
		}
		output << x << NL;
	}
	squish(data[dp][2]);

	total = 1;
	for (fv = 1; fv <= dp; fv ++) {
		;
		thumb[fv] = total;
		total += length(data[fv][2]) + 1;
	}

	thumb[dp + 1] = total;

	/* save it all! Firstly, the Sez file... */

	assign(f, "v:avalot.sez");
	rewrite(f, 1);
	x = string("This is a Sez file for an Avvy game, and it's copyright!") + '\32';
	blockwrite(f, x[1], 57);
	blockwrite(f, dp, 2);
	blockwrite(f, ip, 2);
	blockwrite(f, index1, 40);
	blockwrite(f, thumb, dp * 4 + 4);
	for (fv = 1; fv <= dp; fv ++)
		blockwrite(f, data[fv][2], length(data[fv][2]) + 1);
	close(f);

	/* ...then the Sed file. */

	assign(t, "v:avalot.sed");
	rewrite(t);
	for (fv = 1; fv <= ip; fv ++) t << names[fv] << NL;
	for (fv = 1; fv <= dp; fv ++) t << data[fv][1] << NL;
	close(t);

	/* Done! */
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.