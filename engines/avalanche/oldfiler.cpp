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
#include "graph.h"
/*#include "Dos.h"*/
/*#include "Rodent.h"*/

/*$V-*/
namespace Avalanche {

const char more[] = " (more) ";
const string up = string('-') + '\30' + more + '\30';
const string down = string('+') + '\31' + more + '\31';

const graphcursmasktype fletch = /* Mask 4 in Avalot */
{
	(
	    (255, 511, 1023, 2047, 1023, 4607, 14591, 31871, 65031, 65283, 65281, 65280, 65280, 65409, 65473, 65511),
	    (0, 10240, 20480, 24576, 26624, 17408, 512, 256, 128, 88, 32, 86, 72, 20, 16, 0)),
	0,
	0
};

string cdir;
matrix<0, 2, 1, 18, varying_string<15> > info;
matrix<0, 2, 1, 100, varying_string<15> > possible;
byte fv;
array<0, 2, byte> light, page_;
array<0, 2, boolean> blank;
byte chtcode, where;
string answer;

void split(string x) {
	byte fv;
	x = copy(x, 4, 255);
	if (x == "")  {
		blank[0] = true;
		return;
	}
	x = x + '\\';
	possible[0][1] = "b\\";
	fv = 2;
	while (pos("\\", x) != 0) {
		possible[0][fv] = string('b') + copy(x, 1, pos("\\", x) - 1);
		fv += 1;
		x = copy(x, pos("\\", x) + 1, 255);
	}
	possible[0][fv - 1] = "";
}

void block(integer x1, integer y1, integer x2, integer y2, string x) {
	bar(x1, y1, x2, y2);
	setcolor(9);
	outtextxy(x1 + (x2 - x1) / 2 - 1, y1 + 5, x);
	setcolor(15);
	outtextxy(x1 + (x2 - x1) / 2 + 1, y1 + 6, x);
}

void message(string x) {
	block(5, 189, 640, 200, x);
}

void bigbar(byte x) {
	bar(15 + 210 * x, 36, 210 + 210 * x, 187);
}

void getem();


static void sub_getem(char prefix, string spec, byte attrib, byte infonum) {
	searchrec s;
	byte fv;
	fv = 0;
	findfirst(spec, attrib, s);
	while ((doserror == 0) && (fv < 100)) {
		if (((s.attr & attrib) > 0) && (s.name[1] != '.')) {
			;     /* circumvent inclusive searching! */
			fv += 1;
			possible[infonum][fv] = string(prefix) + s.name;
		}
		findnext(s);
	}
	if (fv == 0)  blank[infonum] = true;
}

void getem()

{
	message("Please wait... scanning directory...");
	sub_getem('a', "*.asg", archive + hidden, 1); /* Scan for .ASG files */
	sub_getem('f', "*.*", directory, 2); /* Scan for sub-directories */
}

void minisc(string &x) {        /* Converts to lower-case */
	byte fv;
	for (fv = 1; fv <= length(x); fv ++)
		if ((x[fv] >= 'A') && (x[fv] <= 'Z'))  x[fv] += 32;
}

void showall() {
	byte fv, ff;
	for (fv = 0; fv <= 2; fv ++) {
		bigbar(fv); /* blank out anything else */
		if (blank[fv]) {
			;     /* nothing here at all */
			/*   setcolor(14);
			   outtextxy(113+210*fv,43,'(Nothing here!)'); */
			setcolor(14);
			settextstyle(0, 0, 2);
			outtextxy(113 + 210 * fv, 77, "Nothing");
			outtextxy(113 + 210 * fv, 100, "here!");
			settextstyle(0, 0, 1);
		} else {
			;     /* something here- what? */
			setcolor(11);
			for (ff = 0; ff <= 15; ff ++) {
				info[fv][ff + 2] = possible[fv][page_[fv] * 15 + ff + 1];
				minisc(info[fv][ff + 2]);
			}
			if (page_[fv] > 0)  info[fv][1] = up;
			else info[fv][1] = "";
			if (possible[fv][page_[fv] * 15 + 17] != "")
				info[fv][18] = down;
			else info[fv][18] = "";
			for (ff = 1; ff <= 18; ff ++) {
				outtextxy(113 + 210 * fv, 35 + ff * 8, copy(info[fv][ff], 2, 255));
			}
		}
	}
	block(5, 12, 640, 22, cdir);
}

void changedir(string x) {

	chdir(x);
	getdir(0, cdir);
}

void drawup() {
	integer gd;
	block(15, 0, 630, 10, "Choose an .ASG file to load or save.");
	block(15, 24, 210, 34, "Looking back:");
	block(225, 24, 420, 34, "Here:");
	block(435, 24, 630, 34, "Looking forwards:");
	for (gd = 0; gd <= 2; gd ++) bigbar(gd); /* just to tide us over the wait... */
	showall();
}

void setup() {
	settextjustify(1, 1);
	setfillstyle(1, 1);
	fillchar(blank, sizeof(blank), '\0');
	fillchar(info, sizeof(info), '\0');
	fillchar(possible, sizeof(possible), '\0');
	fillchar(page_, sizeof(page_), '\0');
	split(cdir);
	getem();
	drawup();
}

void setup1() {
	integer gd, gm;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
	answer = "";
	getdir(0, cdir);
	resetmouse;
	setgraphicscursor(fletch);
	fillchar(light, sizeof(light), '\0');
	setup();
}

void clickwait() {
	const array<1, 4, varying_string<30> > msg = {
		{
			"change to another drive.",
			"return to a lower directory.",
			"use the file named.",
			"enter a sub-directory."
		}
	};
	byte oldcht; /* Click Here To... code */
	showmousecursor;
	oldcht = 177;
	do {
		if (mousey < 38)  chtcode = 1;
		else
			switch (mousex) {
			case 0 ... 210:
				chtcode = 2;
				break;
			case 211 ... 421:
				chtcode = 3;
				break;
			default:
				chtcode = 4;
			}
		if (oldcht != chtcode) {
			hidemousecursor;
			message(string("Click here to ") + msg[chtcode]);
			showmousecursor;
			oldcht = chtcode;
		}
	} while (!leftmousekeypressed);
	hidemousecursor;
	where = ((mousey - 39) / 8) + 1;
}

void blip() {
	sound(32);
	delay(3);
	nosound;
}

void do_cht() {
	char r;
	byte fv;
	string x;
	if (chtcode == 1) {
		;     /* change drives */
		message("Enter the drive letter (e.g. A)...");
		r = readkey();
		changedir(string(r) + ':');
		setup();
	} else {
		x = info[chtcode - 2][where];
		r = x[1];
		x = copy(x, 2, 255);
		switch (r) {
		case 'b': {
			; /* back some dirs */
			if (x == '\\')  x = "";
			for (fv = where - 1; fv >= 3; fv --)
				x = copy(info[0][fv], 2, 255) + '\\' + x;
			changedir(string('\\') + x);
			setup();
		}
		break;
		case 'f': {
			; /* sub-directory */
			changedir(x);
			setup();
		}
		break;
		case '+': {
			; /* scroll one panel down */
			page_[chtcode - 2] += 1;
			drawup();
		}
		break;
		case '-': {
			; /* scroll one panel up */
			page_[chtcode - 2] -= 1;
			drawup();
		}
		break;
		case 'a':
			answer = x;
			break;
		}
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	setup1();
	do {
		clickwait();
		do_cht();
	} while (!(answer != ""));
	if (length(cdir) > 3)  cdir = cdir + '\\';
	answer = cdir + answer;
	closegraph();
	output << "Routine completed." << NL;
	output << "Answer: " << answer << NL;
	output << "Hit Enter:";
	input >> NL;
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.