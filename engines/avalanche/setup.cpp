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
/*#include "Tommys.h"*/
#include "joystick.h"

namespace Avalanche {

const integer selected = 0x60; /* Background for selected lines of text. */

typedef array<1, 400, varying_string<80> > mobytype;

typedef set byteset;   /* Define its typemark. */

enum option {_overrideegacheck, _zoomystart, _loadfirst, _numlockhold, _usemouse,
             _controller, _logging, _logfile,

             _joystickinstalled, _joytop, _joybottom, _joyleft, _joyright, _joymidx,
             _joymidy, _joycentringfactor, _whichjoy, _quiet, _soundcard,
             _baseaddress, _irq, _dma,
             _samplerate, _keyboardclick,

             _printer,

             option_error, last_option
            };


mobytype *moby;
word mobylength;
byte background; /* The current background colour, times 16. */
byte line; /* Which line you're on in the menus. */

string registrant, reginum;

byte num_printers;
array<1, 10, string> printers;
string this_printer;

/* THE STATUS VARIABLES: */

/* general */

boolean override_ega;
boolean skip_loading_screens;
string load_particular;
boolean force_numlock;
boolean ignore_mouse;
boolean use_keyboard;
boolean log_to_printer;
boolean log_to_file;
string log_filename;
string autotype;

/* joystick */

boolean has_a_joystick;
word jtop, jbottom, jleft, jright, jmidx, jmidy;
byte jcentre;
word whichjoy;

/* sound */

boolean suppress_sfx;
byte your_card;
longint samplerate;
longint sound_addr, sound_irq, sound_dma;
boolean wants_keyclick;

/* registration */

string regname, regnum, chkname, chknum;


string trim_and_caps(string this_) {
	byte fv;

	string trim_and_caps_result;
	while ((this_[1] == ' ') && (this_ != ""))
		Delete(this_, 1, 1);       /* Strip leading blanks. */
	while ((this_[length(this_)] == ' ') && (this_ != ""))
		this_[0] -= 1;             /* Strip trailing blanks. */
	for (fv = 1; fv <= length(this_); fv ++) this_[fv] = upcase(this_[fv]); /* And capitalise. */
	trim_and_caps_result = this_;
	return trim_and_caps_result;
}

option string_2_option(string field) {

	option string_2_option_result;
	field = trim_and_caps(field);

	if (field == "OVERRIDEEGACHECK")  string_2_option_result = _overrideegacheck;
	else if (field == "ZOOMYSTART")        string_2_option_result = _zoomystart;
	else if (field == "LOADFIRST")         string_2_option_result = _loadfirst;
	else if (field == "NUMLOCKHOLD")       string_2_option_result = _numlockhold;
	else if (field == "USEMOUSE")          string_2_option_result = _usemouse;
	else if (field == "CONTROLLER")        string_2_option_result = _controller;
	else if (field == "LOGGING")           string_2_option_result = _logging;
	else if (field == "LOGFILE")           string_2_option_result = _logfile;
	else

		/* joystick */

		if (field == "JOYSTICKINSTALLED")  string_2_option_result = _joystickinstalled;
		else if (field == "JOYTOP")             string_2_option_result = _joytop;
		else if (field == "JOYBOTTOM")          string_2_option_result = _joybottom;
		else if (field == "JOYLEFT")            string_2_option_result = _joyleft;
		else if (field == "JOYRIGHT")           string_2_option_result = _joyright;
		else if (field == "JOYMIDX")            string_2_option_result = _joymidx;
		else if (field == "JOYMIDY")            string_2_option_result = _joymidy;
		else if (field == "JOYCENTRINGFACTOR")  string_2_option_result = _joycentringfactor;
		else if (field == "WHICHJOY")           string_2_option_result = _whichjoy;
		else

			/* sound */

			if (field == "QUIET")              string_2_option_result = _quiet;
			else if (field == "SOUNDCARD")          string_2_option_result = _soundcard;
			else if (field == "SAMPLERATE")         string_2_option_result = _samplerate;
			else if (field == "KEYBOARDCLICK")      string_2_option_result = _keyboardclick;
			else if (field == "BASEADDRESS")        string_2_option_result = _baseaddress;
			else if (field == "IRQ")                string_2_option_result = _irq;
			else if (field == "DMA")                string_2_option_result = _dma;
			else

				/* printer */

				if (field == "PRINTER")            string_2_option_result = _printer;
				else


					string_2_option_result = option_error;

	return string_2_option_result;
}

void clear_to(byte colour) {
	window(1, 1, 80, 24);
	background = colour * 16;
	textattr = background;
	clrscr;
}

void centre(byte where, byte colour, string what) {
	textattr = background + colour;
	gotoxy(40 - length(what) / 2, where);
	output << what;
}

void load_file() {
	text t;

	mobylength = 0;
	/*$I-*/
	assign(t, "avalot.ini");
	reset(t);
	/*$I+*/

	if (ioresult != 0) { /* No file. */
		output << "SETUP: Avalot.ini not found!" << NL;
		exit(255);
	}

	while (! eof(t)) {
		mobylength += 1; /* Preincrement mode. */
		t >> (*moby)[mobylength] >> NL;
	}

	close(t);

}

string strhf(longint x) {          /* assume x is +ve */
	const array<0, 16, char> hexdigits = "0123456789ABCDEF";
	string y;
	longint v;

	string strhf_result;
	v = x;
	y = "";

	while (v != 0) {
		y = string(hexdigits[v % longint(16) - 1]) + y;
		v = v / longint(16);
	}

	strhf_result = string('$') + y;
	return strhf_result;
}

void update_moby();

static byte fv;

static string field;


static void get_field(string x) {
	if (pos(";", x) != 0)  x = copy(x, 1, pos(";", x) - 1);

	if (pos("=", x) == 0)
		field = "";
	else {
		field = copy(x, 0, pos("=", x) - 1);
		while (field[1] == ' ')  field = copy(field, 2, 255);
	}
}



static string yn1(boolean x) {
	string yn1_result;
	if (x)  yn1_result = "Yes";
	else yn1_result = "No";
	return yn1_result;
}



static string kj(boolean x) {
	string kj_result;
	if (x)  kj_result = "Keyboard";
	else kj_result = "Joystick";
	return kj_result;
}



static string put_logcodes() {
	byte q;

	string put_logcodes_result;
	q = (byte)(log_to_file) + (byte)(log_to_printer) * 2;

	switch (q) {
	case 0:
		put_logcodes_result = "No";
		break;
	case 1:
		put_logcodes_result = "Disk";
		break;
	case 2:
		put_logcodes_result = "Printer";
		break;
	}
	return put_logcodes_result;
}



static string card() {
	string card_result;
	switch (your_card) {
	case 0:
		card_result = "None";
		break;
	case 1:
		card_result = "SB";
		break;
	case 2:
		card_result = "SBPro";
		break;
	case 3:
		card_result = "SB16";
		break;
	case 4:
		card_result = "Pas";
		break;
	case 5:
		card_result = "PasPlus";
		break;
	case 6:
		card_result = "Pas16";
		break;
	case 7:
		card_result = "Aria";
		break;
	case 8:
		card_result = "WinSound";
		break;
	case 9:
		card_result = "Gravis";
		break;
	case 10:
		card_result = "DacLPT";
		break;
	case 11:
		card_result = "StereoDacs";
		break;
	case 12:
		card_result = "StereoOn1";
		break;
	case 13:
		card_result = "Speaker";
		break;
	}
	return card_result;
}




static void entail(string x) {
	string before, after;

	before = copy((*moby)[fv], 1, pos("=", (*moby)[fv]) - 1);

	if (pos(";", (*moby)[fv]) == 0) {
		(*moby)[fv] = before + '=' + x;
	} else {
		after = copy((*moby)[fv], pos(";", (*moby)[fv]), 255);

		(*moby)[fv] = before + '=' + x + ' ';
		while (length((*moby)[fv]) < 25)(*moby)[fv] = (*moby)[fv] + ' ';

		(*moby)[fv] = (*moby)[fv] + after;
	}
}

void update_moby() {
	option o;





	for (fv = 1; fv <= mobylength; fv ++) {
		get_field((*moby)[fv]);

		if (field != "") {
			o = string_2_option(field);

			if (o == _overrideegacheck)  entail(yn1(override_ega));
			else if (o == _zoomystart)  entail(yn1(skip_loading_screens));
			else if (o == _loadfirst)  entail(load_particular);
			else if (o == _numlockhold)  entail(yn1(force_numlock));
			else if (o == _usemouse)  entail(yn1(ignore_mouse));
			else if (o == _controller)  entail(kj(use_keyboard));
			else if (o == _logging)  entail(put_logcodes());
			else if (o == _logfile)  entail(log_filename);
			else

				/* joystick */

				if (o == _joystickinstalled)  entail(yn1(has_a_joystick));
				else if (o == _joytop)  entail(strf(jtop));
				else if (o == _joybottom)  entail(strf(jbottom));
				else if (o == _joyleft)  entail(strf(jleft));
				else if (o == _joyright)  entail(strf(jright));
				else if (o == _joymidx)  entail(strf(jmidx));
				else if (o == _joymidy)  entail(strf(jmidy));
				else if (o == _joycentringfactor)  entail(strf(jcentre));
				else if (o == _whichjoy)  entail(strf(whichjoy));
				else

					/* sound */

					if (o == _quiet)  entail(yn1(suppress_sfx));
					else if (o == _soundcard)  entail(card());
					else if (o == _samplerate)  entail(strf(samplerate));
					else if (o == _baseaddress)  entail(strhf(sound_addr));
					else if (o == _irq)  entail(strf(sound_irq));
					else if (o == _dma)  entail(strf(sound_dma));
					else if (o == _keyboardclick)  entail(yn1(wants_keyclick));
					else

						/* printer */

						if (o == _printer)  entail(this_printer);

		}
	}
}

void save_file() {
	text t;
	word fv;

	textattr = 10;
	update_moby();

	clear_to(black);
	centre(14, 14, "Saving...");

	assign(t, "avalot.ini");
	rewrite(t);

	for (fv = 1; fv <= mobylength; fv ++) {
		t << (*moby)[fv] << NL;
	}

	close(t);
}

boolean detect() {
	word x, y, xo, yo;
	byte count;

	boolean detect_result;
	count = 0;
	if (joystickpresent()) {
		detect_result = true;
		return detect_result;
	}
	readjoya(xo, yo);
	do {
		if (count < 7)  count += 1; /* Take advantage of "flutter" */
		if (count == 6) {
			centre(7, 1, "The Bios says you don't have a joystick. However, it's often wrong");
			centre(8, 1, "about such matters. So, do you? If you do, move joystick A to");
			centre(9, 1, "continue. If you don't, press any key to cancel.");
		}
		readjoya(x, y);
	} while (!((keypressed()) | (x != xo) || (y != yo)));
	detect_result = ~ keypressed();
	return detect_result;
}

void display() {
	gotoxy(28, 10);
	output << jleft << "  ";
	gotoxy(28, 11);
	output << jright;
	gotoxy(28, 12);
	output << jtop << "  ";
	gotoxy(28, 13);
	output << jbottom;
}

void readjoy(word &x, word &y) {
	if (whichjoy == 1)  readjoya(x, y);
	else readjoyb(x, y);
}

void getmaxmin() {
	word x, y;
	char r;

	clear_to(green);
	centre(5, 1, "Rotate the joystick around in a circle, as far from the centre as it");
	centre(6, 1, "can get. Then press any key.");
	centre(7, 1, "Press Esc to cancel this part.");
	centre(16, 1, "(To reset these figures, set \"Do you have a joystick?\" to No, then Yes.)");

	gotoxy(20, 10);
	output << "Left  :";
	gotoxy(20, 11);
	output << "Right :";
	gotoxy(20, 12);
	output << "Top   :";
	gotoxy(20, 13);
	output << "Bottom:";


	if (jleft == 0)  jleft = maxint;
	if (jtop == 0)   jtop = maxint;
	do {
		readjoy(x, y);
		if (x < jleft)  jleft = x;
		if (y < jtop)  jtop = y;
		if (x > jright)  jright = x;
		if (y > jbottom)  jbottom = y;
		display();
	} while (!keypressed());

	do {
		r = readkey();
	} while (!(~ keypressed()));
	if (r == '\33')  return;

	centre(19, 1, "Thank you. Now please centre your joystick and hit a button.");
	do {
	} while (!(buttona1() || buttona2()));

	readjoya(jmidx, jmidy);

	has_a_joystick = true;
}

void joysetup() {
	clear_to(green);
	if (! detect())  return;
	getmaxmin();
}

byte choose_one_of(byteset which);

static shortint direction;


static void move(shortint d) {
	direction = d;
	line = line + d;
}



static void highlight(word where, word how) {
	byte fv;

	where = where * 160 - 159;
	for (fv = 0; fv <= 79; fv ++)
		mem[0xb800 * where + fv * 2] = (mem[0xb800 * where + fv * 2] & 0xf) + how;
}

byte choose_one_of(byteset which)

{
	const integer upwards = -1;
	const integer downwards = 1;

	boolean done;
	char r;


	byte choose_one_of_result;
	done = false;
	direction = 1;
	do {
		while (!(which.has(line))) {
			line = line + direction;
			if (line > 26)  line = 1;
			if (line == 0)  line = 26;
		}

		highlight(line, selected);
		r = readkey();
		highlight(line, background);
		switch (r) {
		case '\0':
			switch (readkey()) {
			case cup:
				move(upwards);
				break;
			case cdown:
				move(downwards);
				break;
			}
			break;
		case creturn:
			done = true;
			break;
		case cescape: {
			choose_one_of_result = 15; /* bottom line is always 15. */
			return choose_one_of_result;
		}
		break;
		}

	} while (!done);

	choose_one_of_result = line;
	return choose_one_of_result;
}

void bottom_bar();
static void load_regi_info();


static char decode1(char c) {
	byte b;

	char decode1_result;
	b = ord(c) - 32;
	decode1_result = chr(((b & 0xf) << 3) + ((cardinal)(b & 0x70) >> 4));
	return decode1_result;
}



static char decode2(char c) {
	char decode2_result;
	decode2_result = chr(((ord(c) & 0xf) << 2) + 0x43);
	return decode2_result;
}



static boolean checker(string proper, string check) {
	byte fv;
	boolean ok;

	boolean checker_result;
	ok = true;
	for (fv = 1; fv <= length(proper); fv ++)
		if ((ord(proper[fv]) & 0xf) != ((cardinal)(ord(check[fv]) - 0x43) >> 2))
			ok = false;

	checker_result = ok;
	return checker_result;
}


static void load_regi_info() {
	text t;
	byte fv;
	string x;
	byte namelen, numlen;
	string namechk, numchk;


	/*$I-*/
	assign(t, "register.dat");
	reset(t);
	/*$I+*/

	if (ioresult != 0) {
		registrant = "";
		return;
	}

	for (fv = 1; fv <= 53; fv ++) t >> NL;
	t >> x >> NL;
	close(t);

	namelen = 107 - ord(x[1]);
	numlen = 107 - ord(x[2]);

	registrant = copy(x, 3, namelen);
	reginum = copy(x, 4 + namelen, numlen);
	namechk = copy(x, 4 + namelen + numlen, namelen);
	numchk = copy(x, 4 + namelen + numlen + namelen, numlen);

	for (fv = 1; fv <= namelen; fv ++) registrant[fv] = decode1(registrant[fv]);
	for (fv = 1; fv <= numlen; fv ++) reginum[fv] = decode1(reginum[fv]);

	if ((! checker(registrant, namechk)) || (! checker(reginum, numchk))) {
		registrant = "\?\"!?";
		reginum = "(.";
	}
}

void bottom_bar() {
	load_regi_info();
	textattr = 96;
	background = 96;
	window(1, 1, 80, 25);
	gotoxy(1, 25);
	clreol;
	if (registrant == "")
		centre(25, 15, "Unregistered copy.");
	else
		centre(25, 15, string("Registered to ") + registrant + " (" + reginum + ").");
}

void new_menu() {
	line = 1; /* now that we've got a new menu. */
}

string two_answers(string ans_true, string ans_false, boolean which) {
	string two_answers_result;
	if (which)
		two_answers_result = string(" (") + ans_true + ')';
	else
		two_answers_result = string(" (") + ans_false + ')';
	return two_answers_result;
}

string yes_or_no(boolean which) {
	string yes_or_no_result;
	yes_or_no_result = two_answers("yes", "no", which);
	return yes_or_no_result;
}

string give_name(string what) {
	string give_name_result;
	if (what == "")
		give_name_result = " (none)";
	else
		give_name_result = string(" (\"") + what + "\")";
	return give_name_result;
}

string sound_card(byte which) {
	string sound_card_result;
	switch (which) {
	case 0:
		sound_card_result = "none";
		break;
	case 1:
		sound_card_result = "SoundBlaster";
		break;
	case 2:
		sound_card_result = "SoundBlaster Pro";
		break;
	case 3:
		sound_card_result = "SoundBlaster 16";
		break;
	case 4:
		sound_card_result = "Pro Audio Spectrum";
		break;
	case 5:
		sound_card_result = "Pro Audio Spectrum+";
		break;
	case 6:
		sound_card_result = "Pro Audio Spectrum 16";
		break;
	case 7:
		sound_card_result = "Aria";
		break;
	case 8:
		sound_card_result = "Windows Sound System or compatible";
		break;
	case 9:
		sound_card_result = "Gravis Ultrasound";
		break;
	case 10:
		sound_card_result = "DAC on LPT1";
		break;
	case 11:
		sound_card_result = "Stereo DACs on LPT1 and LPT2";
		break;
	case 12:
		sound_card_result = "Stereo-on-1 DAC on LPT";
		break;
	case 13:
		sound_card_result = "PC speaker";
		break;
	}
	return sound_card_result;
}

void get_str(string &n) {
	string x;
	char r;

	clear_to(black);
	centre(3, 3, "Enter the new value. Press Enter to accept, or Esc to cancel.");
	x = "";

	do {

		r = readkey();

		switch (r) {
		case cbackspace:
			if (x[0] > '\0')  x[0] -= 1;
			break;
		case creturn: {
			n = x;
			return;
		}
		break;
		case cescape:
			return;
			break;

		default:
			if (x[0] < '\106')  x = x + r;
		}


		centre(7, 2, string(' ') + x + ' ');

	} while (!false);

}

void get_num(longint &n) {
	string x;
	char r;
	integer e;

	clear_to(black);
	centre(3, 3, "Enter the new value. Press Enter to accept, or Esc to cancel.");
	centre(4, 3, "Precede with $ for a hex value.");
	x = "";

	do {

		r = upcase(readkey());

		switch (r) {
		case cbackspace:
			if (x[0] > '\0')  x[0] -= 1;
			break;
		case creturn: {
			val(x, n, e);
			return;
		}
		break;
		case cescape:
			return;
			break;

		default:
			if ((x[0] < '\106') &&
			        ((set::of(range('0', '9'), eos).has(r)) || ((x[1] == '$') && (set::of(range('A', 'F'), eos).has(r))) ||
			         ((x == "") && (r == '$'))))
				x = x + r;
		}


		centre(7, 2, string(' ') + x + ' ');

	} while (!false);

}

void general_menu() {
	new_menu();
	do {
		clear_to(blue);

		centre(3, 15, "General Menu");

		centre(5, 7, string("Override EGA check?") + yes_or_no(override_ega));
		centre(6, 7, string("Skip loading screens?") + yes_or_no(skip_loading_screens));
		centre(7, 7, string("Load a particular file by default?") + give_name(load_particular));
		centre(8, 7, string("Force NumLock off?") + yes_or_no(force_numlock));
		/*  centre( 9, 7,'Ignore the mouse?'+yes_or_no(ignore_mouse));*/
		centre(10, 7, string("Default controller?") + two_answers("keyboard", "joystick", use_keyboard));
		centre(12, 7, string("Log to printer?") + yes_or_no(log_to_printer));
		centre(13, 7, string("Log to file?") + yes_or_no(log_to_file));
		centre(14, 7, string("Filename to log to?") + give_name(log_filename));

		centre(15, 15, "Return to main menu.");

		switch (choose_one_of(set::of(5, 6, 7, 8,/*9,*/10, 12, 13, 14, 15, eos))) {
		case 5:
			override_ega = ! override_ega;
			break;
		case 6:
			skip_loading_screens = ! skip_loading_screens;
			break;
		case 7:
			get_str(load_particular);
			break;
		case 8:
			force_numlock = ! force_numlock;
			break;
			/*    9: ignore_mouse:=not ignore_mouse;*/
		case 10:
			use_keyboard = ! use_keyboard;
			break;
		case 12: {
			log_to_printer = ! log_to_printer;
			if (log_to_file && log_to_printer)  log_to_file = false;
		}
		break;
		case 13: {
			log_to_file = ! log_to_file;
			if (log_to_file && log_to_printer)  log_to_printer = false;
		}
		break;
		case 14:
			get_str(log_filename);
			break;
		case 15: {
			new_menu();
			return;
		}
		break;
		}

	} while (!false);
}

void joystick_menu() {
	new_menu();
	do {
		clear_to(green);

		centre(3, 15, "Joystick Menu");

		centre(5, 14, string("Do you have a joystick?") + yes_or_no(has_a_joystick));
		centre(6, 14, string("Which joystick to use? ") + chr(whichjoy + 48));
		centre(7, 14, "Select this one to set it up.");

		centre(15, 15, "Return to main menu");

		switch (choose_one_of(set::of(5, 6, 7, 15, eos))) {
		case 5: {
			has_a_joystick = ! has_a_joystick;
			if (! has_a_joystick) {
				jleft = 0;
				jright = 0;
				jtop = 0;
				jbottom = 0;
			}
		}
		break;
		case 6:
			whichjoy = 3 - whichjoy;
			break; /* Flips between 2 and 1. */
		case 7:
			joysetup();
			break;
		case 15: {
			new_menu();
			return;
		}
		break;
		}

	} while (!false);
}

void cycle(byte &what, byte upper_limit) {
	if (what == upper_limit)
		what = 0;
	else
		what += 1;
}

void sound_menu() {
	new_menu();
	do {
		clear_to(cyan);

		centre(3, 0, "Sound menu");

		centre(5, 0, string("Do you want to suppress sound effects?") + yes_or_no(suppress_sfx));
		centre(6, 0, string("Sound output device\? (") + sound_card(your_card) + ')');
		centre(7, 0, string("Sampling rate\? (") + strf(samplerate) + "Hz)");
		centre(8, 0, string("Base address\? (") + strhf(sound_addr) + " *hex*)");
		centre(9, 0, string("IRQ\? (") + strf(sound_irq) + ')');
		centre(10, 0, string("DMA\? (") + strf(sound_dma) + ')');
		centre(11, 0, string("Do you want keyclick?") + yes_or_no(wants_keyclick));

		centre(15, 15, "Return to main menu");

		centre(17, 1, "WARNING: Incorrect values of IRQ and DMA may damage your computer!");
		centre(18, 1, "Read AVALOT.INI for the correct values.");

		switch (choose_one_of(set::of(5, 6, 7, 8, 9, 10, 11, 15, eos))) {
		case 5:
			suppress_sfx = ! suppress_sfx;
			break;
		case 6:
			cycle(your_card, 13);
			break;
		case 7:
			get_num(samplerate);
			break;
		case 8:
			get_num(sound_addr);
			break;
		case 9:
			get_num(sound_irq);
			break;
		case 10:
			get_num(sound_dma);
			break;
		case 11:
			wants_keyclick = ! wants_keyclick;
			break;
		case 15: {
			new_menu();
			return;
		}
		break;
		}

	} while (!false);
}

void printer_menu() {
	byte fv;
	byteset chooseable_lines;

	new_menu();

	chooseable_lines = set::of(15, eos);
	for (fv = 1; fv <= num_printers; fv ++)
		chooseable_lines = chooseable_lines + set::of(fv + 8, eos);

	do {
		clear_to(red);

		centre(3, 15, "Printer menu");

		centre(5, 15, "Select one of the following printers:");
		centre(6, 15, string("The current choice is ") + this_printer + '.');

		for (fv = 1; fv <= num_printers; fv ++)
			centre(8 + fv, 14, printers[fv]);

		centre(15, 15, "Return to main menu");

		fv = choose_one_of(chooseable_lines);

		if (fv == 15)  {
			new_menu();
			return;
		}

		this_printer = printers[fv - 8];

	} while (!false);
}

void regi_split(string x) {
	byte fv;

	regname[0] = chr(107 - ord(x[1]));
	chkname[0] = regname[0];
	regnum[0] = chr(107 - ord(x[2]));
	chknum[0] = chknum[0];

	move(x[3], regname[1], ord(regname[0]));
	for (fv = 1; fv <= length(regname); fv ++)
		regname[fv] = chr(abs(((ord(regname[fv]) - 33) - 177 * fv) % 94) + 33);
}


void registration_menu() {
	char r;
	text t, o;
	string x;
	byte fv;

	clear_to(black);

	centre(3, 15, "REGISTRATION");
	centre(5, 14, "Please insert the disk you were sent when you registered");
	centre(6, 14, "into any drive, and press its letter. For example, if the");
	centre(7, 14, "disk is in drive A:, press A.");
	centre(9, 14, "Press Esc to cancel this menu.");

	do {
		r = upcase(readkey());
	} while (!(set::of('\33', range('A', 'Z'), eos).has(r)));
	if (r == '\33')  return;

	/*$I-*/
	assign(t, string(r) + ":\\REGISTER.DAT");
	reset(t);
	/*$I+*/
	if (ioresult != 0) {
		centre(17, 15, "But it isn't in that drive...");
		centre(19, 15, "Press any key.");
		r = readkey();
		return;
	}
	for (fv = 1; fv <= 54; fv ++) t >> x >> NL;
	regi_split(x);

	/* Copy the file... */

	assign(o, "register.dat");
	rewrite(o);
	reset(t);

	while (! eof(t)) {
		t >> x >> NL;
		o << x << NL;
	}
	close(t);
	close(o);

	centre(17, 15, "Done! Press any key...");
	bottom_bar();
	r = readkey();

}

void menu() {
	bottom_bar();
	new_menu();
	do {
		clear_to(black);

		centre(3, 15, "Avalot Setup - Main Menu");

		centre(5, 9, "General setup");
		centre(6, 10, "Joystick setup");
		centre(7, 11, "Sound setup");
		centre(8, 12, "Printer setup");
		centre(9, 14, "REGISTRATION setup");

		centre(15, 15, "--- EXIT SETUP ---");

		switch (choose_one_of(set::of(5, 6, 7, 8, 9, 15, eos))) {
		case 5:
			general_menu();
			break;
		case 6:
			joystick_menu();
			break;
		case 7:
			sound_menu();
			break;
		case 8:
			printer_menu();
			break;
		case 9:
			registration_menu();
			break;
		case 15: {
			new_menu();
			clear_to(lightgray);
			centre(3, 0, "Quit: would you like to save changes?");
			centre(5, 1, "Quit and SAVE changes.");
			centre(6, 1, "Quit and DON'T save changes.");
			centre(15, 0, "Cancel and return to the main menu.");
			switch (choose_one_of(set::of(5, 6, 15, eos))) {
			case 5: {
				save_file();
				return;
			}
			break;
			case 6:
				return;
				break;
			}
			new_menu();
		}
		break;
		}

	} while (!false);
}

void defaults() {   /* Sets everything to its default value. */
	/* general */

	override_ega = false;
	skip_loading_screens = false;
	load_particular = "";
	force_numlock = true;
	ignore_mouse = false;
	use_keyboard = true;
	log_to_printer = false;
	log_to_file = false;
	log_filename = "avalot.log";

	/* joystick */

	has_a_joystick = false;
	/* jtop,jbottom,jleft,jright,jmidx,jmidy need no initialisation. */

	/* sound */

	suppress_sfx = false;
	your_card = 0; /* none */
	wants_keyclick = false;

	/* other stuff */

	registrant = "";

	num_printers = 0;
	this_printer = "??";
}

void parse_file();

const integer parse_weird_field = 1;

const integer parse_not_yes_or_no = 2;

const integer parse_not_numeric = 3;

const integer parse_not_kbd_or_joy = 4;

const integer parse_weird_logcode = 5;

const integer parse_weird_card = 6;


static varying_string<80> this_, thiswas;

static boolean error_found, ignoring;


static void error(byte what) {
	textattr = 15;
	if (! error_found) {
		clrscr;
		textattr = 12;
		output << "SETUP: *** ERROR FOUND IN AVALOT.INI! ***" << NL;
		textattr = 15;
	}
	output << ' ';
	switch (what) {
	case parse_weird_field:
		output << "Unknown identifier on the left";
		break;
	case parse_not_yes_or_no:
		output << "Value on the right should be Yes or No";
		break;
	case parse_not_numeric:
		output << "Value on the right is not numeric";
		break;
	case parse_not_kbd_or_joy:
		output << "Value on the right should be Keyboard or Joystick";
		break;
	case parse_weird_logcode:
		output << "Value on the right should be No, Printer or Disk";
		break;
	case parse_weird_card:
		output << "Never heard of the card";
		break;
	}
	output << " in:" << NL;
	textattr = 10;
	output << thiswas << NL;
	error_found = true;
}



static boolean yesno(string x) {
	boolean yesno_result;
	if (x == "YES")
		yesno_result = true;
	else if (x == "NO")
		yesno_result = false;
	else {
		error(parse_not_yes_or_no);
		yesno_result = false;
	}
	return yesno_result;
}



static boolean kbdjoy(string x) {
	boolean kbdjoy_result;
	if (x == "KEYBOARD")
		kbdjoy_result = true;
	else if (x == "JOYSTICK")
		kbdjoy_result = false;
	else {
		error(parse_not_kbd_or_joy);
		kbdjoy_result = false;
	}
	return kbdjoy_result;
}



static word numeric(string x) {
	const varying_string<15> hexdigits = "0123456789ABCDEF";
	word answer;
	integer e;

	word numeric_result;
	if (x[1] == '$') {
		answer = 0;
		for (e = 2; e <= length(x); e ++) {
			answer = answer << 4;
			answer += pos(upcase(x[e]), hexdigits) - 1;
		}
	} else {
		val(x, answer, e);
		if (e != 0)  error(parse_not_numeric);
	}
	numeric_result = answer;
	return numeric_result;
}



static void get_logcodes(string x) {
	if (x == "NO")  {
		log_to_file = false;
		log_to_printer = false;
	} else if (x == "DISK")  {
		log_to_file = true;
		log_to_printer = false;
	} else if (x == "PRINTER")  {
		log_to_file = false;
		log_to_printer = true;
	} else
		error(parse_weird_logcode);
}



static void get_card(string x) {
	if (x == "NONE")  your_card = 0;
	else if (x == "SB")  your_card = 1;
	else if (x == "SBPRO")  your_card = 2;
	else if (x == "SB16")  your_card = 3;
	else if (x == "PAS")  your_card = 4;
	else if (x == "PASPLUS")  your_card = 5;
	else if (x == "PAS16")  your_card = 6;
	else if (x == "ARIA")  your_card = 7;
	else if (x == "WINSOUND")  your_card = 8;
	else if (x == "GRAVIS")  your_card = 9;
	else if (x == "DACLPT")  your_card = 10;
	else if (x == "STEREODACS")  your_card = 11;
	else if (x == "STEREOON1")  your_card = 12;
	else if (x == "SPEAKER")  your_card = 13;
	else
		error(parse_weird_card);
}

void parse_file() {


	word where;
	byte position;
	varying_string<80> field, data, pure_data;
	option o;


	error_found = false;
	ignoring = false;

	for (where = 1; where <= mobylength; where ++) {
		this_ = (*moby)[where];
		thiswas = this_;

		position = pos(";", this_);
		if (position > 0)  this_ = copy(this_, 1, position - 1);

		if (this_ == "")  continue_; /* Don't carry on if by now it's empty. */

		if (this_[1] == '[') {
			ignoring = !(trim_and_caps(this_) == "[END]");

			if (copy(this_, 1, 8) == "[printer") {
				num_printers += 1;
				printers[num_printers] = copy(this_, 10, length(this_) - 10);
			}
		}

		if (ignoring)  continue_;

		position = pos("=", this_);
		field = trim_and_caps(copy(this_, 1, position - 1));
		if (field == "")  continue_;
		pure_data = copy(this_, position + 1, 255);
		data = trim_and_caps(pure_data);
		o = string_2_option(field);

		/* general */

		if (o == _overrideegacheck)  override_ega = yesno(data);
		else if (o == _zoomystart)  skip_loading_screens = yesno(data);
		else if (o == _loadfirst)  load_particular = data;
		else if (o == _numlockhold)  force_numlock = yesno(data);
		else if (o == _usemouse)  ignore_mouse = yesno(data);
		else if (o == _controller)  use_keyboard = kbdjoy(data);
		else if (o == _logging)  get_logcodes(data);
		else if (o == _logfile)  log_filename = data;
		else

			/* joystick */

			if (o == _joystickinstalled)  has_a_joystick = yesno(data);
			else if (o == _joytop)  jtop = numeric(data);
			else if (o == _joybottom)  jbottom = numeric(data);
			else if (o == _joyleft)  jleft = numeric(data);
			else if (o == _joyright)  jright = numeric(data);
			else if (o == _joymidx)  jmidx = numeric(data);
			else if (o == _joymidy)  jmidy = numeric(data);
			else if (o == _joycentringfactor)  jcentre = numeric(data);
			else if (o == _whichjoy)  whichjoy = numeric(data);
			else

				/* sound */

				if (o == _quiet)  suppress_sfx = yesno(data);
				else if (o == _soundcard)  get_card(data);
				else if (o == _samplerate)  samplerate = numeric(data);
				else if (o == _baseaddress)  sound_addr = numeric(data);
				else if (o == _irq)  sound_irq = numeric(data);
				else if (o == _dma)  sound_dma = numeric(data);
				else if (o == _keyboardclick)  wants_keyclick = yesno(data);
				else

					/* printer */

					if (o == _printer)  this_printer = pure_data;
					else

						/* others */

						error(parse_weird_field);
	}

	if (error_found) {
		textattr = 15;
		output << " Try and fix the above errors. As a last resort, try deleting or" << NL;
		output << " renaming AVALOT.INI, and the default values will be used. Good luck." << NL;
		exit(177);
	}
}

void clear_up() {
	window(1, 1, 80, 25);
	textattr = 31;
	clrscr;
	output << NL;
	output << "Enjoy the game..." << NL;
	output << NL;
	cga_cursor_on;
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	cursor_off;

	moby = new mobytype; /* Allocate memory space */

	defaults();

	load_file();

	parse_file();

	menu();

	delete moby;   /* Deallocate memory space again */

	clear_up();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.