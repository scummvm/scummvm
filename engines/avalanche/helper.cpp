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

                 HELPER           The help system unit. */

#define __helper_implementation__


#include "helper.h"


/*#include "Crt.h"*/
/*#include "Lucerna.h"*/
/*#include "Pingo.h"*/

namespace Avalanche {

struct buttontype {
	char trigger;
	byte whither;
};

const integer buttonsize = 930;

const integer toolbar = 0;
const integer nim = 1;
const integer kbd = 2;
const integer credits = 3;
const integer joystick = 4;
const integer troubleshooting = 5;
const integer story = 6;
const integer mainscreen = 7;
const integer registering = 8;
const integer sound = 9;
const integer mouse = 10;
const integer filer = 11;
const integer back2game = 12;
const integer helponhelp = 13;
const integer pgdn = 14;
const integer pgup = 15;

array<1, 10, buttontype> buttons;
byte highlight_was;

void plot_button(integer y, byte which) {
	untyped_file f;
	pointer p;

	if (y > 200) {
		background(2);    /* Silly buttons. */
		delay(10);
		background(0);
		return;
	}
	getmem(p, buttonsize);
	assign(f, "buttons.avd");
	reset(f, 1);
	seek(f, which * buttonsize);
	blockread(f, p, buttonsize);

	if (y == -177)
		putimage(229, 5, p, 0);
	else
		putimage(470, y, p, 0);
	close(f);
	freemem(p, buttonsize);
}

void getme(byte which);

static untyped_file f;


static void chalk(byte y, string z) {
	outtextxy(16, 41 + y * 10, z);
}



static void getline(string &x) {
	byte fz;

	blockread(f, x[0], 1);
	blockread(f, x[1], (byte)(x[0]));
	for (fz = 1; fz <= length(x); fz ++)
		x[fz] = chr(ord(x[fz]) ^ 177);
}

void getme(byte which) {     /* Help icons are 80x20 */
	string x;
	byte y, fv;
	word offset;


	off;
	assign(f, "help.avd");
	y = 0;
	highlight_was = 177; /* Forget where the highlight was. */
	reset(f, 1);
	seek(f, which * 2);
	blockread(f, offset, 2);
	seek(f, offset);

	getline(x);
	setfillstyle(1, 1);
	bar(0, 0, 640, 200);
	setfillstyle(1, 15);
	bar(8, 40, 450, 200);
	settextjustify(2, 2);
	blockread(f, fv, 1);
	plot_button(-177, fv);

	setcolor(0);
	outtextxy(629, 26, x); /* Plot the title. */
	setcolor(3);
	outtextxy(630, 25, x);

	settextjustify(0, 2);
	settextstyle(0, 0, 2);
	setcolor(0);
	outtextxy(549, 1, "help!");
	setcolor(3);
	outtextxy(550, 0, "help!");
	/***/ settextstyle(0, 0, 1);

	do {
		getline(x);
		if (x == '!')  flush(); /* End of the help text is signalled with a !. */
		if (x[1] == '\\') {
			setcolor(4);
			chalk(y, copy(x, 2, 255));
		} else {
			setcolor(0);
			chalk(y, x);
		}
		y += 1;
	} while (!false);

	/* We are now at the end of the text. Next we must read the icons. */

	y = 0;
	settextjustify(1, 1);
	settextstyle(0, 0, 2);
	while (! eof(f)) {
		y += 1;
		blockread(f, buttons[y].trigger, 1);
		if (buttons[y].trigger == '\261')  flush();
		blockread(f, fv, 1);
		if (buttons[y].trigger != '\0')  plot_button(13 + y * 27, fv);
		blockread(f, buttons[y].whither, 1); /* this is the position to jump to */


		switch (buttons[y].trigger) {
		case '˛' :
			x = "Esc";
			break;
		case '÷' :
			x = '\30';
			break;
		case 'ÿ' :
			x = '\31';
			break;
		default:
			x = buttons[y].trigger;
		}
		setcolor(0);
		outtextxy(589, 26 + y * 27, x);
		setcolor(3);
		outtextxy(590, 25 + y * 27, x);

	}

	settextjustify(0, 2);
	settextstyle(0, 0, 1);
	close(f);
	on;
}


byte check_mouse();
/* Returns clicked-on button, or 0 if none. */
static void light(byte which, byte colour) {
	if (which == 177)  return; /* Dummy value for "no button at all". */
	setcolor(colour);
	which = which & 31;
	rectangle(466, 11 + which * 27, 555, 35 + which * 27);
}

byte check_mouse() {
	byte h_is;

	byte check_mouse_result;
	check;

	if (mrelease != 0) {
		/* Clicked *somewhere*... */
		if ((mx < 470) || (mx > 550) || (((my - 13) % 27) > 20))
			check_mouse_result = 0;
		else
			/* Clicked on a button. */
			check_mouse_result = ((my - 13) / 27);
	} else {
		if ((mx > 470) && (mx <= 550) && (((my - 13) % 27) <= 20)) {
			/* No click, so highlight. */
			h_is = (my - 13) / 27;
			if ((h_is < 1) || (h_is > 6))  h_is = 177; /* In case of silly values. */
		} else h_is = 177;

		if ((h_is != 177) && ((keystatus & 1) > 0))  h_is += 32;

		if (h_is != highlight_was) {
			off;
			light(highlight_was, 1);
			highlight_was = h_is;
			if (buttons[h_is & 31].trigger != '\0') {
				if (h_is > 31)  light(h_is, 11);
				else light(h_is, 9);
			}
			on;
		}

		check_mouse_result = 0;
	}
	return check_mouse_result;
}

void continue_help() {
	char r;
	byte fv;

	do {
		while (! keypressed()) {
			fv = check_mouse();

			if (fv > 0)
				switch (buttons[fv].trigger) {
				case '\0':/*null*/
					;
					break;
				case '\376':
					return;
					break;
				default: {
					dusk;
					getme(buttons[fv].whither);
					dawn;
					continue_;
				}
				}

		}
		r = upcase(readkey());
		switch (r) {
		case '\33' :
			return;
			break;
		case '\0'  :
			switch (readkey()) {
			case '\110':
			case '\111':
				r = '÷';
				break;
			case '\120':
			case '\121':
				r = 'ÿ';
				break;
			case '\73':
				r = 'H';
				break; /* Help on help */
			default:
				continue_;
			}
			break;
		}

		for (fv = 1; fv <= 10; fv ++) {
			buttontype &with = buttons[fv];
			if (with.trigger == r) {
				dusk;
				getme(with.whither);
				dawn;
				flush();
			}
		}

	} while (!false);
}

void boot_help() {
	byte groi;

	dusk;
	off;
	oncandopageswap = false;
	highlight_was = 177;
	copypage(3, 1 - cp); /* Store old screen. */ groi = getpixel(0, 0);

	/* Set up mouse. */
	off_virtual;
	newpointer(2);
	setactivepage(3);
	setvisualpage(3);

	getme(0);
	dawn;

	newpointer(9);
	on;
	mousepage(3);

	continue_help();

	mousepage(cp);
	dusk;
	off;
	oncandopageswap = true;
	copypage(1 - cp, 3); /* Restore old screen. */ groi = getpixel(0, 0);
	on_virtual;
	dawn;
	fix_flashers;

	setvisualpage(cp);
	setactivepage(1 - cp);
}

} // End of namespace Avalanche.