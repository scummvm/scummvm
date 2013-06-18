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

#include "graph.h"
/*#include "Crt.h"*/

namespace Avalanche {

typedef matrix < '\0', '\377', 0, 15, byte > fonttype;

struct hypertype {
	char trigger;
	byte line;
	byte start, finish;
	word ref;
};

integer gd, gm;
array<1, 2, fonttype> font;
matrix<0, 7, 1, 80, byte> current;
array<1, 250, varying_string<79> > data;
integer fv, position, size;
varying_string<79> title;
array<1, 20, hypertype> link;
byte numlinks;
char r;
array<0, 9, byte> reverse;
array<0, 9, byte> revpos;

void loadfont() {
	file<fonttype> fontfile;
	assign(fontfile, "c:\\thomas\\ttsmall.fnt");
	reset(fontfile);
	fontfile >> font[1];
	close(fontfile);
	/* assign(fontfile,'c:\avalot\avalot.fnt'); reset(fontfile);
	 read(fontfile,font[2]); close(fontfile);*/
	/* NB: We'll put BOTH of these fonts one after the other, in the same
	  file, in the final compilation. */
}

void scribe(byte which, byte what);

static void underline(byte &x) {
	x = x | 177;
}

void scribe(byte which, byte what) {
	byte fv, ff;
	string x;
	x = data[what];
	fillchar(current, sizeof(current), '\0');
	for (ff = 1; ff <= length(x); ff ++)
		for (fv = 0; fv <= 7; fv ++) {
			current[fv][ff] = font[which][x[ff]][fv];
		}

	for (fv = 1; fv <= numlinks; fv ++) {
		hypertype &with = link[fv];
		if (with.line == what)
			for (ff = with.start; ff <= with.finish; ff ++)
				underline(current[7][ff]);
	}

}

void display(word y) {
	for (fv = 0; fv <= 7; fv ++)
		move(current[fv], mem[0xa000 * (y + fv) * 80], 79);
}

void update_link(char which, byte whence, byte whither) {
	byte fv;
	for (fv = 1; fv <= numlinks; fv ++) {
		hypertype &with = link[fv];
		if (with.trigger == which) {
			with.line = size;
			with.start = whence;
			with.finish = whither;
		}
	}
}

void getlinks(string &x) {
	byte p, q;
	do {
		p = pos("[", x);
		if (p == 0)  return; /* lousy, huh? */
		q = pos("]", x);
		update_link(x[p + 1], p, q - 3);
		Delete(x, q, 1);
		Delete(x, p, 2);
	} while (!false);
}

void loaddata(byte which) {
	text t;
	string x;
	integer e;
	revpos[9] = position;
	fillchar(data, sizeof(data), '\0');
	move(reverse[1], reverse[0], 9);
	move(revpos[1], revpos[0], 9);
	reverse[9] = which;
	revpos[9] = 1;

	str(which, x);
	assign(t, string('h') + x + ".raw");
	reset(t);
	t >> title >> NL;
	size = 0;
	numlinks = 0;
	while (! eof(t)) {
		t >> x >> NL;
		if (x[1] == ':') {
			numlinks += 1;
			{
				hypertype &with = link[numlinks];
				with.trigger = x[2];
				Delete(x, 1, 3);
				Delete(x, pos(" ", x), 255);
				val(x, with.ref, e);
			}
		} else {
			size += 1;
			getlinks(x);
			data[size] = x;
		}
	}
	position = 1;
	size -= 15;
	close(t);
}

void screen() {
	setbkcolor(1);
	setfillstyle(1, 1);
	bar(0, 0, 640, 38);
	setfillstyle(1, 14);
	bar(0, 39, 640, 39);
}

void showscreen() {
	byte fv;
	if (position < 1)  position = 1;
	for (fv = 0; fv <= 15; fv ++) {
		scribe(1, fv + position);
		display(41 + fv * 10);
	}
}

void up() {
	byte fv;
	position -= 1;
	scribe(1, position);
	for (fv = 0; fv <= 9; fv ++) {
		move(mem[0xa000 * 3200], mem[0xa000 * 3280], 12720);
		if (set::of(0, 9, eos).has(fv))  fillchar(mem[0xa000 * 3200], 79, '\0');
		else
			move(current[8 - fv], mem[0xa000 * 3200], 80);
	}
}

void down() {
	byte fv;
	position += 1;
	scribe(1, position + 15);
	for (fv = 0; fv <= 9; fv ++) {
		move(mem[0xa000 * 3280], mem[0xa000 * 3200], 12720);
		if (set::of(0, 9, eos).has(fv))  fillchar(mem[0xa000 * 15920], 79, '\0');
		else
			move(current[fv - 1], mem[0xa000 * 15920], 80);
	}
}

void newpage(char c) {
	byte fv;
	for (fv = 1; fv <= numlinks; fv ++) {
		hypertype &with = link[fv];
		if (with.trigger == c) {
			loaddata(with.ref);
			showscreen();
		}
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	loadfont();
	screen();
	loaddata(0);
	showscreen();
	fillchar(reverse, sizeof(reverse), '\0');
	fillchar(revpos, sizeof(revpos), '\1');
	do {
		r = upcase(readkey());
		switch (r) {
		case '\0':
			switch (readkey()) {
			case 'H':
				if (position > 1)  up();
				break;
			case 'P':
				if (position < size)  down();
				break;
			case 'I': {
				position -= 16;
				showscreen();
			}
			break;
			case 'Q': {
				position += 16;
				showscreen();
			}
			break;
			case 'G': {
				position = 1;
				showscreen();
			}
			break;
			case 'O': {
				position = size;
				showscreen();
			}
			break;
			}
			break;
		case 'B': {
			; /* go Back */
			gd = reverse[8];
			gm = revpos[8];
			move(reverse[0], reverse[2], 8);
			move(revpos[0], revpos[2], 8);
			loaddata(gd);
			position = gm;
			showscreen();
		}
		break;
		case 'C': {
			; /* Contents */
			loaddata(0);
			showscreen();
		}
		break;
		case 'H': {
			loaddata(7); /* help on help */
			showscreen();
		}
		break;
		case '\33':
			exit(0);
			break;
		default:
			newpage(r);
		}
	} while (!false);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.