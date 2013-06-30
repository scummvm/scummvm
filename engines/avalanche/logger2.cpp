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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

/* LOGGER	Handles the logging. */

#include "avalanche/avalanche.h"

#include "avalanche/logger2.h"
#include "avalanche/gyro2.h"
#include "avalanche/trip6.h"

#include "common/textconsole.h"

namespace Avalanche {
const char Logger::divide[] = "--- oOo ---";

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

//// Not sure if these will be needed at all...
//
//const Common::String startwith = Common::String('\33') + '\50' + "10J" + '\130';
//const Common::String endwith = Common::String('\33') + '\105';
//const Common::String italic = Common::String('\33') + '\50' + '\163' + '\61' + '\123'; /* switches italics on... */
//const Common::String italic_off = Common::String('\33') + '\50' + '\163' + '\60' + '\123'; /* and off. */
//const Common::String emph_on = Common::String('\33') + '\50' + '\163' + '\63' + '\102';
//const Common::String emph_off = Common::String('\33') + '\50' + '\163' + '\60' + '\102';
//
//const Common::String double_width = emph_on; /* There IS no double-width. */
//const Common::String double_off = emph_off; /* So we'll have to use bold. */



const Common::String Logger::quote = "ª";
const Common::String Logger::unquote = "º";
const Common::String Logger::copyright = "(c)";
const int16 Logger::divide_indent = 30;



void Logger::setParent(AvalancheEngine *vm) {
	_vm = vm;
}

void Logger::centre(byte size, byte x) { /* Prints req'd number of spaces. */
	byte fv;
	
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::centre()");
	/*for (fv = 1; fv <= size - (x / 2); fv++)
		output << logfile << ' ';*/
}

void Logger::log_setup(Common::String name, bool printing) {     /* Sets up. */

	warning("STUB: Logger::log_setup()");

	/*	assign(logfile, name);
	rewrite(logfile);
	output << logfile << startwith;
	log_epson = printing;
	logging = true;

	if (!printing)  {
		quote = '"';
		unquote = '"';
		copyright = "(c)";
	}*/
}

void Logger::log_divider() {   /* Prints the divider sign. */
	byte fv;
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_divider()");

	/*
	if (_vm->_gyro.log_epson) {
		output << logfile << Common::String(' ') + double_width;
		for (fv = 1; fv <= divide_indent; fv++)
			output << logfile << ' ';
		output << logfile << Common::String(' ') + double_off;
	} else {
		for (fv = 1; fv <= 36; fv++)
			output << logfile << ' ';
	}
	output << logfile << divide << NL;
	*/
}

void Logger::log_command(Common::String x) {     /* Prints a command */
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_command()");

	/*if (_vm->_gyro.log_epson)
		output << logfile << double_width + '>' + double_off + ' ' + italic + x + italic_off << NL;
	else
		output << logfile << Common::String("> ") + x << NL;*/
}

void Logger::log_addstuff(Common::String x) {
	if (!_vm->_gyro.logging)
		return;

	scroll_line += x;
}

void Logger::log_scrollchar(Common::String x) {     /* print one character */
	Common::String z;
	if (!_vm->_gyro.logging)
		return;

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
	scroll_line_length += z.size();
}

void Logger::log_italic() {
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_italic()");

	/*if (_vm->_gyro.log_epson)
		log_addstuff(italic);
	else
		log_addstuff("*");*/
}

void Logger::log_roman() {
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_roman()");

	/* if (_vm->_gyro.log_epson)
		log_addstuff(italic_off);
	else
		log_addstuff("*");*/
}

void Logger::log_epsonroman() {   /* This only sends the Roman code if you're on Epson.*/
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_epsonroman()");

	//if (_vm->_gyro.log_epson)
	//	log_addstuff(italic_off);
}

void Logger::log_scrollline() {   /* Set up a line for the scroll driver */
	scroll_line_length = 0;
	scroll_line = "";
}

void Logger::log_scrollendline(bool centred) {
	byte x, fv;
	if (!_vm->_gyro.logging)
		return;
	x = 17;
	if (centred)
		x += (50 - scroll_line_length) / 2;

	warning("STUB: Logger::log_scrollendline()");

	/*for (fv = 1; fv <= x; fv++)
		output << logfile << ' ';
	output << logfile << scroll_line << NL;*/
}

void Logger::log_bubbleline(byte linenum, byte whom, Common::String x) {
	byte fv;
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_bubbleline()");

	/*if (linenum == 1) {
		for (fv = 1; fv <= 15; fv++)
			output << logfile << ' ';
		output << logfile << italic + tr[whom].a.name + ": " + italic_off + x << NL;
	} else {
		for (fv = 1; fv <= 17; fv++)
			output << logfile << ' ';
		output << logfile << x << NL;
	}*/
}

void Logger::log_newline() {
	warning("STUB: Logger::log_newline()");

	//if (_vm->_gyro.logging)
	//	output << logfile << NL;
}

void Logger::log_newroom(Common::String where) {
	byte fv;
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_newroom()");

	/*for (fv = 1; fv <= 20; fv++)
		output << logfile << ' ';
	if (_vm->_gyro.log_epson)
		output << logfile << emph_on;
	output << logfile << Common::String('(') + where + ')';
	if (_vm->_gyro.log_epson)
		output << logfile << emph_off;
	output << logfile << NL;*/
}

void Logger::log_aside(Common::String what) {
/* This writes "asides" to the printer. For example, moves in Nim. */
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_aside()");

	//output << logfile << "   (" << italic << what << italic_off << ')' << NL;
	/* "What" is what to write. */
}

void Logger::log_score(uint16 credit, uint16 now) {
	byte fv;
	if (!_vm->_gyro.logging)
		return;

	warning("STUB: Logger::log_score()");

	/*for (fv = 1; fv <= 50; fv++)
		output << logfile << ' ';
	output << logfile << "Score " << italic << "credit : " << credit << italic_off << " total : " << now << NL;*/
}

} // End of namespace Avalanche
