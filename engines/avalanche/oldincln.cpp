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

                 INCLINE          The command-line parser. */

#define __incline_implementation__


#include "oldincln.h"

#include "gyro.h"
#include "logger.h"

namespace Avalanche {

const integer bug_twonames = 255;
const integer bug_pandl = 254;
const integer bug_weirdswitch = 253;
const integer bug_invalidini = 252;
const integer bug_notyesorno = 251;

byte fv;
char t;
string bugline;
boolean usingp, usingl;
boolean zoomy, numlockhold;

boolean filename_specified;

string inihead, initail; /* For reading the .INI file. */


void linebug(byte which) {
	;
	output << "AVALOT : ";
	switch (which) {
	case bug_twonames :
		output << "You may only specify ONE filename." << NL;
		break;
	case bug_pandl :
		output << "/p and /l cannot be used together." << NL;
		break;
	case bug_weirdswitch :
		output << "Unknown switch (\"" << bugline <<
		       "\"). Type AVALOT /? for a list of switches." << NL;
		break;
	case bug_invalidini:
		output << "Invalid line in AVALOT.INI (\"" << bugline << "\")" << NL;
		break;
	case bug_notyesorno:
		output << "Error in AVALOT.INI: \"" << inihead << "\" must be \"yes\" or \"no.\"" << NL;
		break;
	}
	exit(which);
}

void syntax() {
	;
	assign(output, "");
	rewrite(output);
	output << NL;
	output << string("Lord Avalot d'Argent\n\n(c) ") + copyright + " Mark, Mike and Thomas Thurman." << NL;
	output << string("~~~~~~~~~~~~~~~~~~~~~\n\n") + vernum << NL;
	output << NL;
	output << "Syntax:" << NL;
	output << "\n/?\ndisplays this screen," << NL;
	output << "\n/O\noverrides EGA check," << NL;
	output << "\n/L<f>\nlogs progress to <f>, default AVVY.LOG," << NL;
	output << "\n/P<x>\nlogs with Epson codes to <x>, default PRN," << NL;
	output << "\n/Q\ncancels sound effects," << NL;
	output << "\n/S\ndisables Soundblaster," << NL;
	output << "\n/Z\ngoes straight into the game." << NL;
	output << NL;
	output << "\n\n\n\n\n\n\n... Have fun!" << NL;
	exit(177);
}

void upstr(string &x) {
	byte fv;
	;
	for (fv = 1; fv <= length(x); fv ++) x[fv] = upcase(x[fv]);
}

boolean yesno() {
	boolean yesno_result;
	;
	if (initail == "YES")  yesno_result = true;
	else if (initail == "NO")  yesno_result = false;
	else
		linebug(bug_notyesorno);
	return yesno_result;
}

void ini_parse() {
	;
	upstr(inihead);
	upstr(initail);

	if (inihead == "QUIET")  soundfx = ! yesno();
	else if (inihead == "ZOOMYSTART")  zoomy = yesno();
	else if (inihead == "NUMLOCKHOLD")  numlockhold = yesno();
	else if (inihead == "LOADFIRST")  filetoload = initail;
	else if (inihead == "OVERRIDEEGACHECK")  cl_override = yesno();
	else if (inihead == "KEYBOARDCLICK")  keyboardclick = yesno();
}

void strip_ini() {
	byte fv;
	;
	if (inihead == "")  return;

	/* Firstly, delete any comments. */
	fv = pos(";", inihead);
	if (fv > 0)  Delete(inihead, fv, 255);

	/* Lose the whitespace... */

	while (inihead[length(inihead)] == ' ')  inihead[0] -= 1;
	while ((inihead != "") && (inihead[1] == ' '))  Delete(inihead, 1, 1);

	/* It's possible that now we'll end up with a blank line. */

	if ((inihead == "") || (inihead[1] == '['))  return;

	fv = pos("=", inihead);

	if (fv == 0) {
		;     /* No "="! Weird! */
		bugline = inihead;
		linebug(bug_invalidini);
	}

	initail = copy(inihead, fv + 1, 255);
	inihead[0] = chr(fv - 1);
}

void load_ini() {
	text ini;
	;
	assign(ini, "AVALOT.INI");
	reset(ini);

	while (! eof(ini)) {
		;
		ini >> inihead >> NL;
		strip_ini();
		if (inihead != "")  ini_parse();
	}

	close(ini);
}

void parse(string x);
static string arg;

static string getarg(string otherwis) {
	string getarg_result;
	;
	if (arg == "")  getarg_result = otherwis;
	else getarg_result = "";
	return getarg_result;
}

void parse(string x)

{
	;
	switch (x[1]) {
	case '/':
	case '-': {
		;
		arg = copy(x, 3, 255);
		switch (upcase(x[2])) {
		case '?':
			syntax();
			break;
		case 'O':
			cl_override = true;
			break;
		case 'L':
			if (! usingp) {
				;
				log_setup(getarg("avvy.log"), false);
				usingl = true;
			} else {
				;
				close(logfile);
				linebug(bug_pandl);
			}
			break;
		case 'P':
			if (! usingl) {
				;
				log_setup(getarg("prn"), true);
				usingp = true;
			} else {
				;
				close(logfile);
				linebug(bug_pandl);
			}
			break;
		case 'Q':
			soundfx = false;
			break;
		case 'Z':
			zoomy = true;
			break;
		case 'K':
			keyboardclick = true;
			break;
		case 'D':
			demo = true;
			break;
		default: {
			;
			bugline = x;
			linebug(bug_weirdswitch);
		}
		}
	}
	break;
	case '*': {
		;
		inihead = copy(x, 2, 255);
		strip_ini();
		if (inihead != "")  ini_parse();
	}
	break;
	default: {
		; /* filename */
		if (filename_specified)
			linebug(bug_twonames);
		else
			filetoload = x;
		filename_specified = true;
	}
	}
}

void not_through_bootstrap() {
	output << "Avalot must be loaded through the bootstrap." << NL;
	exit(0);
}

void get_storage_addr() {
	integer e;

	val(paramstr(2), storage_seg, e);
	if (e != 0)  not_through_bootstrap();
	val(paramstr(3), storage_ofs, e);
	if (e != 0)  not_through_bootstrap();
	skellern = storage_ofs + 1;
}

class unit_incline_initialize {
public:
	unit_incline_initialize();
};
static unit_incline_initialize incline_constructor;

unit_incline_initialize::unit_incline_initialize() {
	;
	filetoload = "";
	usingl = false;
	usingp = false;
	logging = false;
	cl_override = false;
	soundfx = true;
	zoomy = false;
	numlockhold = false;
	filename_specified = false;
	keyboardclick = false;

	load_ini();

	if ((paramcount < 3) ||
	        ((paramstr(1) != "Go") && (paramstr(1) != "et")))  not_through_bootstrap();

	reloaded = paramstr(1) == "et";

	get_storage_addr();

	for (fv = 4; fv <= paramcount; fv ++)
		parse(paramstr(fv));
}

} // End of namespace Avalanche.