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

                 LOGGER           Handles the logging. */

#define __logger_implementation__


#include "logger.h"


/*#include "Gyro.h"*/
/*#include "Trip5.h"*/

namespace Avalanche {

const char divide[] = "--- oOo ---";

/* Epson codes:

 startwith='';
 endwith='';
 double_width = #14; { shift out (SO) }
 double_off = #20; { device control 4 (DC4) }
 italic = #27+'4'; { switches italics on... }
 italic_off = #27+'5'; { and off. }
 emph_on = #27+#69;
 emph_off = #27+#70;
 divide_indent = 15;

*/

/* L'jet codes: */

const string startwith = string('\33') + '\50' + "10J" + '\130';
const string endwith = string('\33') + '\105';
const string italic = string('\33') + '\50' + '\163' + '\61' + '\123'; /* switches italics on... */
const string italic_off = string('\33') + '\50' + '\163' + '\60' + '\123'; /* and off. */
const string emph_on = string('\33') + '\50' + '\163' + '\63' + '\102';
const string emph_off = string('\33') + '\50' + '\163' + '\60' + '\102';
const string double_width = emph_on; /* There IS no double-width. */
const string double_off = emph_off; /* So we'll have to use bold. */
const string quote = '™';
const string unquote = '∫';
const string copyright = "(c)";
const integer divide_indent = 30;

string scroll_line;
byte scroll_line_length;

void centre(byte size, byte x) { /* Prints req'd number of spaces. */
	byte fv;
	;
	if (~ logging)  return;
	for (fv = 1; fv <= size - (x / 2); fv ++)
		output << logfile << ' ';
}

void log_setup(string name, boolean printing) {     /* Sets up. */
	;
	assign(logfile, name);
	rewrite(logfile);
	output << logfile << startwith;
	log_epson = printing;
	logging = true;

	if (! printing)  {
		quote = '"';
		unquote = '"';
		copyright = "(c)";
	}
}

void log_divider() {   /* Prints the divider sign. */
	byte fv;
	;
	if (~ logging)  return;
	if (log_epson) {
		;
		output << logfile << string(' ') + double_width;
		for (fv = 1; fv <= divide_indent; fv ++) output << logfile << ' ';
		output << logfile << string(' ') + double_off;
	} else
		for (fv = 1; fv <= 36; fv ++) output << logfile << ' ';
	output << logfile << divide << NL;
}

void log_command(string x) {     /* Prints a command */
	;
	if (~ logging)  return;
	if (log_epson)
		output << logfile << double_width + '>' + double_off + ' ' + italic + x + italic_off << NL;
	else
		output << logfile << string("> ") + x << NL;
}

void log_addstuff(string x) {
	;
	if (~ logging)  return;
	scroll_line = scroll_line + x;
}

void log_scrollchar(string x) {     /* print one character */
	varying_string<2> z;
	;
	if (~ logging)  return;
	switch (x[1]) {
	case '`':
		z = quote;
		break; /* Open quotes: "66" */
	case '"':
		z = unquote;
		break; /* Close quotes: "99" */
	case '\357':
		z = copyright;
		break; /* Copyright sign. */
	default:
		z = x;
	}
	log_addstuff(z);
	scroll_line_length += length(z);
}

void log_italic() {
	;
	if (~ logging)  return;
	if (log_epson)
		log_addstuff(italic);
	else
		log_addstuff("*");
}

void log_roman() {
	;
	if (~ logging)  return;
	if (log_epson)
		log_addstuff(italic_off);
	else
		log_addstuff("*");
}

void log_epsonroman() {   /* This only sends the Roman code if you're on Epson.*/
	;
	if (~ logging)  return;
	if (log_epson)  log_addstuff(italic_off);
}

void log_scrollline() {   /* Set up a line for the scroll driver */
	;
	scroll_line_length = 0;
	scroll_line = "";
}

void log_scrollendline(boolean centred) {
	byte x, fv;
	;
	if (~ logging)  return;
	x = 17;
	if (centred)  x += (50 - scroll_line_length) / 2;
	for (fv = 1; fv <= x; fv ++) output << logfile << ' ';
	output << logfile << scroll_line << NL;
}

void log_bubbleline(byte linenum, byte whom, string x) {
	byte fv;
	;
	if (~ logging)  return;
	if (linenum == 1) {
		;
		for (fv = 1; fv <= 15; fv ++) output << logfile << ' ';
		output << logfile << italic + tr[whom].a.name + ": " + italic_off + x << NL;
	} else {
		;
		for (fv = 1; fv <= 17; fv ++) output << logfile << ' ';
		output << logfile << x << NL;
	}
}

void log_newline() {
	;
	if (logging)  output << logfile << NL;
}

void log_newroom(string where) {
	byte fv;
	;
	if (~ logging)  return;
	for (fv = 1; fv <= 20; fv ++) output << logfile << ' ';
	if (log_epson)  output << logfile << emph_on;
	output << logfile << string('(') + where + ')';
	if (log_epson)  output << logfile << emph_off;
	output << logfile << NL;
}

void log_aside(string what)
/* This writes "asides" to the printer. For example, moves in Nim. */
{
	;
	if (~ logging)  return;
	output << logfile << "   (" << italic << what << italic_off << ')' << NL;
	/* "What" is what to write. */
}

void log_score(word credit, word now) {
	byte fv;
	;
	if (~ logging)  return;
	for (fv = 1; fv <= 50; fv ++) output << logfile << ' ';
	output << logfile << "Score " << italic << "credit : " << credit << italic_off << " total : " << now << NL;
}

} // End of namespace Avalanche.