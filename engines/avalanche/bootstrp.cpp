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

/*#include "Dos.h"*/


/*$M 8192,0,$6000, S-*/

namespace Avalanche {

enum elm {normal, musical, elmpoyten, regi, last_elm};

const integer run_shootemup = 1;
const integer run_dosshell = 2;
const integer run_ghostroom = 3;
const integer run_golden = 4;

const array<false, true, varying_string<2> > runcodes = {{"et", "Go"}};

const integer reset_ = 0;

const boolean jsb = true;
const boolean no_jsb = false;
const boolean bflight = true;
const boolean no_bflight = false;


struct {
	byte operation;
	word skellern;
	array<1, 10000, byte> contents;
} storage;

string arguments, demo_args, args_with_no_filename;

boolean first_time;

byte original_mode;
pointer old_1c;

string segofs;

boolean zoomy;

longint soundcard, speed, baseaddr, irq, dma;

void cursor_off() {   /*assembler; */
	;
	/* asm
	 mov ah,01; { Change cursor size. */
	/* mov cx,8224; { CH & CL are now 32. */
	/* int $10;  { Call the Bios */

}    /* That's all. */

void cursor_on() {   /*assembler; */
	;
	/*
	asm
	 mov ah,01; { Change cursor size. */
	/* mov ch,5;  { Top line is 5. */
	/* mov cl,7;  { Bottom line is 7. */
	/* int $10;   { Call the Bios. */

}

void quit() {
	cursor_on();
	exit(0);
}

string strf(longint x) {
	string q;
	string strf_result;
	;
	str(x, q);
	strf_result = q;
	return strf_result;
}

string command_com() {
	string temp;

	string command_com_result;
	temp = getenv("comspec");
	if (temp == "")
		output << "avvy_bootstrap: COMSPEC not defined, => cannot run Dos shell." << NL;
	command_com_result = temp;
	return command_com_result;
}

void explain(byte error) {
	output << " (";
	switch (error) {
	case 2:
		output << "it's not there";
		break;
	case 8:
		output << "out of memory";
		break;
	default:
		output << "error " << error;
	}
	output << ")." << NL;
}

/*$F+*/

void b_flight() {   /*interrupt;*/
	storage.skellern += 1;
}

/*$F-*/

void bflight_on() {
	storage.skellern = reset_;
	setintvec(0x1c, &b_flight);
}

void bflight_off() {
	setintvec(0x1c, old_1c);
}

void run(string what, boolean with_jsb, boolean with_bflight, elm how);


static string elm2str(elm how) {
	string elm2str_result;
	switch (how) {
	case normal:
	case musical:
		elm2str_result = "jsb";
		break;
	case regi:
		elm2str_result = "REGI";
		break;
	case elmpoyten:
		elm2str_result = "ELMPOYTEN";
		break;
	}
	return elm2str_result;
}

void run(string what, boolean with_jsb, boolean with_bflight, elm how) {
	string which_dir, args;
	integer error;


	if (what == "")  return;

	getdir(0, which_dir);

	if (with_jsb) {
		if (how == musical)
			args = elm2str(how) + ' ' + args_with_no_filename; /* FN is NOT given if musical*/
		else
			args = elm2str(how) + ' ' + arguments;
	} else args = "";

	if (how == musical)  args = args + strf(soundcard) + ' ' + strf(speed) + ' ' +
		                            strf(baseaddr) + ' ' + strf(dma) + ' ' + strf(irq);
	if (with_bflight)  bflight_on();

	swapvectors;
	exec(what, args);
	swapvectors;
	cursor_off();

	error = doserror;

	if (with_bflight)  bflight_off();

	chdir(which_dir);

	if (error != 0) {
		output << "avvy_bootstrap: cannot execute " << what << '!';
		explain(error);
		output << "Press Enter:";
		input >> NL;
		quit();
	}
}

void run_avalot() {
	integer error;


	bflight_on();

	swapvectors;
	exec("avalot.avx", runcodes[first_time] + arguments);
	swapvectors;

	error = doserror;

	bflight_off();

	if (error != 0) {
		output << "avvy_bootstrap: error in loading AVALOT.AVX!";
		explain(error);
		quit();
	}

	first_time = false;
}

void run_the_demo() {
	string args;

	args = arguments;
	arguments = demo_args; /* Force the demo. */

	run_avalot();

	arguments = args;  /* Put all back to normal again. */
	first_time = true;
}

void get_arguments() {
	byte fv;

	arguments = "";

	for (fv = 1; fv <= paramcount; fv ++)
		arguments = arguments + paramstr(fv) + ' ';

	arguments[0] -= 1; /* Get rid of the trailing space. */

	segofs = string(' ') + strf(seg(storage)) + ' ' + strf(ofs(storage));

	arguments = segofs + ' ' + arguments;
}

void dos_shell() {
	registers r;

	r.ax = original_mode;
	intr(0x10, r);
	output << NL;
	output << "The Avalot Dos Shell." << NL;
	output << "---------------------" << NL;
	output << NL;
	output << "Type EXIT to return to Avalot." << NL;
	output << NL;

	cursor_on();
	run(command_com(), no_jsb, no_bflight, normal);
	cursor_off();

	output << "Please wait, restoring your game..." << NL;
}

boolean keypressed1() {
	registers r;

	boolean keypressed1_result;
	r.ah = 0xb;
	msdos(r);
	keypressed1_result = r.al == 0xff;
	return keypressed1_result;
}

void flush_buffer() {
	registers r;
	r.ah = 7;
	while (keypressed1())  msdos(r);
}

void demo() {
	run_the_demo();
	if (keypressed1())  return;
	run("intro.avx", jsb, bflight, musical);
	if (keypressed1())  return;
	run("stars.avx", jsb, no_bflight, musical);
	if (keypressed1())  return;

	flush_buffer();
}

void call_menu() {
	run("stars.avx", jsb, no_bflight, musical);
	flush_buffer();
	do {
		run("avmenu.avx", jsb, no_bflight, normal);

		switch (storage.operation) {
		case 1:
			return;
			break; /* Play the game. */
		case 2:
			run("intro.avx", jsb, bflight, musical);
			break;
		case 3:
			run("preview1.avd", jsb, no_bflight, normal);
			break;
		case 4:
			run("viewdocs.avx", jsb, bflight, elmpoyten);
			break;
		case 5:
			run("viewdocs.avx", jsb, bflight, regi);
			break;
		case 6:
			quit();
			break;
		case 177:
			demo();
			break;
		}

		flush_buffer();
	} while (!false);
}

void get_slope() {
	run("slope.avx", jsb, no_bflight, normal);
	if (dosexitcode != 0) {
		cursor_on();
		exit(0);
	}

	move(storage.contents, arguments, sizeof(arguments));
	move(storage.contents[4998], soundcard, 4);
	move(storage.contents[5002], baseaddr, 4);
	move(storage.contents[5006], irq, 4);
	move(storage.contents[5010], dma, 4);
	move(storage.contents[5014], speed, 4);

	zoomy = (arguments[8] == 'y') || (arguments[2] == 'y');
	demo_args = arguments;
	demo_args[7] = 'y';
	arguments = segofs + ' ' + arguments;
	demo_args = segofs + ' ' + demo_args;

	args_with_no_filename = arguments;
	if (arguments[length(arguments)] != ' ') {
		/* Filename was given */
		args_with_no_filename = arguments;
		while ((args_with_no_filename != "")
		        && (args_with_no_filename[length(args_with_no_filename)] != ' '))
			args_with_no_filename[0] -= 1; /* Strip off the filename. */
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	original_mode = mem[seg0040 * 0x49];
	getintvec(0x1c, old_1c);
	first_time = true;
	cursor_off();

	get_arguments();
	get_slope();

	if (! zoomy)  call_menu();    /* Not run when zoomy. */

	do {
		run_avalot();

		if (dosexitcode != 77)  quit(); /* Didn't stop for us. */

		switch (storage.operation) {
		case run_shootemup:
			run("seu.avx", jsb, bflight, normal);
			break;
		case run_dosshell:
			dos_shell();
			break;
		case run_ghostroom:
			run("g-room.avx", jsb, no_bflight, normal);
			break;
		case run_golden:
			run("golden.avx", jsb, bflight, musical);
			break;
		}

	} while (!false);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.