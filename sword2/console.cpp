/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "bs2/driver/driver96.h"

namespace Sword2 {
uint32	console_status = 0;	//0 off		// LEFT IN RELEASE VERSION
} // End of namespace Sword2

#ifdef _SWORD2_DEBUG

// its the console! <great>

#include "bs2/build_display.h"
#include "bs2/console.h"
#include "bs2/debug.h"
#include "bs2/defs.h"
#include "bs2/events.h"		// so we can display the event list in Con_display_events()
#include "bs2/header.h"
#include "bs2/layers.h"
#include "bs2/logic.h"
#include "bs2/maketext.h"	// for InitialiseFontResourceFlags()
#include "bs2/mouse.h"
#include "bs2/mem_view.h"
#include "bs2/memory.h"
#include "bs2/protocol.h"
#include "bs2/resman.h"
#include "bs2/save_rest.h"
#include "bs2/startup.h"
#include "bs2/sword2.h"
#include "bs2/time.h"

namespace Sword2 {

uint32 Parse_user_input(void);
void Clear_console_line(void);
void Con_help(void);

void Con_colour_block(int x, int width, int height, uint32 pen, uint32 paper, uint8 *sprite_data_ad);
void Con_print(uint8 *ascii, uint32 pen, uint32 paper);
uint32 Tconsole(uint32 mode);

void Con_list_savegames(void);
void Con_save_game(int total_commands, uint8 *slotString, uint8 *description);
void Con_restore_game(int total_commands, uint8 *slotString);
uint8 Is_number(uint8 *ascii);
void Con_start_timer(int total_commands, uint8 *slotString);
void ShowVar(uint8 *varNoPtr);
void HideVar(uint8 *varNoPtr);
void Con_display_version(void);

void Var_check(uint8 *pointer);
void Var_set(uint8 *pointer, uint8 *p2);

void Con_display_events();

uint8 wantSfxDebug = 0;	// sfx debug file enabled/disabled from console

#define	MAX_CONSOLE_BUFFER	70
#define	MAX_CONSOLE_PARAMS	5

#define	CON_PEN			187

#define VK_TAB			0x09
#define VK_RETURN		0x0D

char console_buffer[MAX_CONSOLE_BUFFER];

char last_command[MAX_CONSOLE_BUFFER];
int last_command_len = 0;

uint8 grabbingSequences = 0;

int console_pos = 0;

int console_mode = 0;	// 0 is the base command line
			// 1 means only parse for yes or no commands

#define	TOTAL_CONSOLE_COMMANDS	47

// note '9' is max command length including null-terminator
uint8 commands[TOTAL_CONSOLE_COMMANDS][9] = {
	"HELP",		// 0
	"MEM",		// 1
	"Q",		// 2
	"TONY",		// 3
	"YES",		// 4
	"NO",		// 5
	"RES",		// 6
	"STARTS",	// 7
	"START",	// 8
	"INFO",		// 9
	"WALKGRID",	// 10
	"MOUSE",	// 11
	"PLAYER",	// 12
	"RESLOOK",	// 13
	"CUR",		// 14
	"RUNLIST",	// 15
	"KILL",		// 16
	"NUKE",		// 17
	"S",		// 18
	"VAR",		// 19
	"RECT",		// 20
	"CLEAR",	// 21
	"DEBUGON",	// 22
	"DEBUGOFF",	// 23
	"SAVEREST",	// 24
	"SAVES",	// 25
	"SAVE",		// 26
	"RESTORE",	// 27
	"BLTFXON",	// 28
	"BLTFXOFF",	// 29
	"TIMEON",	// 30
	"TIMEOFF",	// 31
	"TEXT",		// 32
	"SHOWVAR",	// 33
	"HIDEVAR",	// 34
	"VERSION",	// 35
	"SOFT",		// 36
	"HARD",		// 37
	"ANIMTEST",	// 38
	"TEXTTEST",	// 39
	"LINETEST",	// 40
	"GRAB",		// 41
	"EVENTS",	// 42
	"SFX",		// 43
	"ENGLISH",	// 44
	"FINNISH",	// 45
	"POLISH"	// 46
};

mem *console_sprite;
uint32 con_y;
uint32 con_depth;
uint32 con_width;
uint32 con_chr_height;

#define	CON_lines 20	// 10 lines deep

void Init_console(void) {
	// grab the memory for the console sprite

	uint8 *ad;
	uint8 white[4] = { 255, 255, 255, 0 };

	con_chr_height = 12;
	con_width = screenWide;	//max across

	// Force a palatte for the console.
	BS2_SetPalette(CON_PEN, 1, white, RDPAL_INSTANT);

	console_sprite = memory.alloc(con_width * (CON_lines * con_chr_height), MEM_float, UID_con_sprite);

	if (!console_sprite) {
		ExitWithReport("Init_console alloc fail");
	}

	con_depth = CON_lines * con_chr_height;
	con_y = 399 - con_depth;

	// clear the buffer for a nice fresh start

	memset(console_sprite->ad, 0, con_width * CON_lines * con_chr_height);

	debug(5, "console height %d, y %d", con_depth, con_y);

	//first time in message
	Con_display_version();
}

void StartConsole(void) {
	// start console up and restart new line
	// can ne called for newline

	console_pos = 0;	// start of new line

	//we need to clear the whole buffer - else the cursor overwrites the
	// end 0

	console_status = 1;	// on
}

void EndConsole(void) {
	console_status = 0;	// off
}

uint32 Tconsole(uint32 mode) {
	// call from anywhere
	// returns a positive value of the token typed or 0 for windows
	// quiting - the caller should drop back

	uint32 ret, breakOut = 0;

	console_mode = mode;	//set command frame

	StartConsole();

	while (1) {
		ServiceWindows();

		if (breakOut)
			break;

		if ((ret = One_console()) != 0) {
			EndConsole();
			return ret;
		}

		Build_display();	// create and flip the screen
	}

	// a windows message is throwing us out of here

	EndConsole();	// switch off drawing
	return 0;
}

void Scroll_console(void) {
	// scroll the console sprite up 12 pixels

	uint32 *to_buffer;
	uint32 *from_buffer;

	// number of dwords
	x = ((con_depth - con_chr_height) * 640) / 4;

	to_buffer= (uint32 *) console_sprite->ad;
	from_buffer = to_buffer + ((con_chr_height * 640) / 4);

	memmove(to_buffer, from_buffer, (con_depth - con_chr_heigth) * 640);

	// blank the on-coming bottom line
	Clear_console_line();
}

void Clear_console_line(void) {
	// blank the bottom line

	uint32 *pbuffer;

	//base of our off-screen back buffer
	pbuffer= (uint32 *) console_sprite->ad;

	//index to console text position
	pbuffer += ((con_depth-con_chr_height) * con_width / 4);

	// clear the bottom text line
	memset(pbuffer, 0, con_chr_height * con_width);
}

void Print_to_console(const char *format, ...) {
	// print a NULL terminated string of ascii to the next console line
	// we can assume that the user has just entered a command by pressing
	// return - which means we're on a clean line so output the line and
	// line feed

	va_list arg_ptr;
	char buf[150];

	va_start(arg_ptr,format);
	_vsnprintf(buf, 150, format, arg_ptr);
	Con_print((uint8 *) buf, 2, 0);
	Scroll_console();
}

void Temp_print_to_console(const char *format, ...) {
	// print a NULL terminated string of ascii to the next console line
	// we can assume that the user has just entered a command by pressing
	// return - which means we're on a clean line so output the line and
	// line feed

	va_list arg_ptr;	// Variable argument pointer
	char buf[150];

	va_start(arg_ptr,format);
	_vsnprintf(buf, 150, format, arg_ptr);

	Con_print((uint8 *) buf, 2, 0);
}

uint32 One_console(void) {
	// its the console command line system
	// do an update - check keys and flash cursor and so on

	char c;
	static int flash = 0;	//controls the flashing cursor rate
	uint32 res;

	if (KeyWaiting()) {
		ReadKey(&c);

		if (c == VK_TAB) {
			if (last_command_len) {	// if anything stored in buffer
				// retrieve 'last_command' buffer
				// first clear the entire current buffer
				memset (console_buffer, 0, MAX_CONSOLE_BUFFER);

				// now copy in the last command
				memcpy (console_buffer, last_command, last_command_len);
				console_pos = last_command_len;
			}
		} else if (c == VK_RETURN) {
			// by putting a space in we'll always have a chr$ in
			// the buffer
			console_buffer[console_pos] = ' ';
			Clear_console_line();
			Print_to_console(console_buffer);

			// parse the input I guess

			if (console_pos) {
				// save to 'last_command' buffer, in case need
				// to repeat same command

				// get a copy of the current command, and its
				// length
				memcpy (last_command, console_buffer, console_pos);
				last_command_len = console_pos;

				res = Parse_user_input();
				if (res)
					return res;
			}

			StartConsole();		// reset buffer
		} else if (c == 8) {
			if (console_pos) {
				// delete cursor chr$
				console_buffer[console_pos] = 0;
				console_pos--;
				console_buffer[console_pos]=0;
			}
		} else if (c < 32 || c > 'z')
			debug(5, "console ignoring key - %d", c);
		else {
			// less one to leave room for the cursor
			if (console_pos < MAX_CONSOLE_BUFFER - 1) {
				console_buffer[console_pos++] = c;
			} else {
				// end of line has been reached, so keep
				// replacing last letter
				console_buffer[console_pos - 1] = c;
			}
		}
	}

	flash++;

	if (flash < 7)
		console_buffer[console_pos] = '_';
	else
		console_buffer[console_pos] = ' ';

	if (flash == 14)
		flash = 0;

	// update the real screen - done every cycle to keep the cursor
	// flashing

	Clear_console_line();
	Con_print((uint8 *) console_buffer, 2, 0);

	return 0;
}

uint32 Parse_user_input(void) {
	// pressed return and now we need to analyse whats been written and
	// call up the relevent commands

	uint8 input[MAX_CONSOLE_PARAMS][MAX_CONSOLE_BUFFER];
	int i, j, total_commands = 0;
	int index = 0;
	uint32 rv;
	uint8 pal[4] = { 255, 255, 255, 0 };

	// quick check for numbers here
	if (!isalpha(console_buffer[0])) {
		Print_to_console("Eh?");
		return(0);
	}

	j = 0;
	do {
		i = 0;
		do
			input[j][i++] = toupper(console_buffer[index++]);
		while (isgraph(console_buffer[index]));

		input[j][i] = 0;

		j++;
		total_commands++;

		if (index == console_pos)
			break;

		do
			index++;
		while (console_buffer[index] == ' ');
	} while (j < MAX_CONSOLE_PARAMS);

	// try to find the first word in the commands base

	for (j = 0; j < TOTAL_CONSOLE_COMMANDS; j++) {
		i = 0;
		while (input[0][i] == commands[j][i] && input[0][i])
			i++;

		// got to the end of an entry - so must have matched the whole
		// word
		if (!input[0][i] && !commands[j][i]) {
			// the console mode denotes the scope of the commands
			// accepted 0 is the base mode

			switch (console_mode) {
			// external console commands may only be
			// requiring a yes/no input for example
			// a different scope would only accept yes and
			// no and drop back out when found... see?

			case 0:
				// base command line
				switch(j) {
				case 0:		// HELP
					Con_help();
					return 0;
				case 1:		// MEM
					memory.displayMemory();
					return 0;
				case 2:		// Q
					// quit the console
					return 1;
				case 3:		// TONY
					Print_to_console("What about him?");
					return 0;
				case 6:		// RES
					res_man.printConsoleClusters();
					return 0;
				case 7:		// STARTS
					Con_print_start_menu();
					return 0;
				case 8:		// START
				case 18:	// S (same as START)
					Con_start(&input[1][0]);
					// force the palette
					BS2_SetPalette(187, 1, pal, RDPAL_INSTANT);
					return 0;
				case 9:		// INFO
					displayDebugText = 1 - displayDebugText;
					if (displayDebugText)
						Print_to_console("info text on");
					else
						Print_to_console("info text off");
					return 0;
				case 10:	// WALKGRID
					displayWalkGrid = 1 - displayWalkGrid;
					if (displayWalkGrid)
						Print_to_console("walk-grid display on");
					else
						Print_to_console("walk-grid display off");
					return 0;
				case 11:	// MOUSE
					displayMouseMarker = 1 - displayMouseMarker;
					if (displayMouseMarker)
						Print_to_console("mouse marker on");
					else
						Print_to_console("mouse marker off");
					return 0;
				case 12:	// PLAYER
					displayPlayerMarker = 1 - displayPlayerMarker;
					if (displayPlayerMarker)
						Print_to_console("player feet marker on");
					else
						Print_to_console("player feet marker off");
					return 0;
				case 13:	// RESLOOK
					res_man.examine(&input[1][0]);
					return 0;
				case 14:	// CUR
					Print_current_info();
					return 0;
				case 15:	// RUNLIST
					LLogic.examineRunList();
					return 0;
				case 16:	// KILL
					res_man.kill(&input[1][0]);
					return 0;
				case 17:	// NUKE
					Print_to_console("killing all resources except variable file & player object...");
					// '1' means we want output to console
					res_man.killAll(1);
					return 0;
				case 19:	// VAR
					if (total_commands == 2)
						Var_check(&input[1][0]);
					else
						Var_set(&input[1][0], &input[2][0]);
					return 0;
				case 20:	// RECT
					definingRectangles = 1 - definingRectangles;
					if (definingRectangles)
						Print_to_console("mouse rectangles enabled");
					else
						Print_to_console("mouse rectangles disabled");
					// reset (see debug.cpp & mouse.cpp)
					draggingRectangle = 0;
					return 0;
				case 21:	// CLEAR
					Print_to_console("killing all object resources except player...");
					// '1' means we want output to console
					res_man.killAllObjects(1);
					return 0;
				case 22:	// DEBUGON
					displayDebugText = 1;
					displayWalkGrid = 1;
					displayMouseMarker = 1;
					displayPlayerMarker = 1;
					displayTextNumbers = 1;

					Print_to_console("enabled all on-screen debug info");
					return 0;
				case 23:	// DEBUGOFF
					displayDebugText = 0;
					displayWalkGrid = 0;
					displayMouseMarker = 0;
					displayPlayerMarker = 0;
					displayTextNumbers = 0;
					definingRectangles = 0;
					draggingRectangle = 0;

					Print_to_console("disabled all on-screen debug info");
					return 0;
				case 24:	// SAVEREST
					testingSnR = 1 - testingSnR;
					if (testingSnR)
						Print_to_console("Enabled S&R logic_script stability checking");
					else
						Print_to_console("Disabled S&R logic_script stability checking");
					return 0;
				case 25:	// SAVES
					Print_to_console("Savegames:");
					Con_list_savegames();
					return 0;
				case 26:	// SAVE <slotNo> <description>
					Con_save_game(total_commands, &input[1][0], &input[2][0]);
					return 0;
				case 27:	// RESTORE <slotNo>
					Con_restore_game(total_commands, &input[1][0]);

					// quit the console
					return 1;
				case 28:	// BLTFXON
					SetBltFx();
					Print_to_console("blit fx enabled");
					return 0;
				case 29:	// BLTFXOFF
					ClearBltFx();
					Print_to_console("blit fx disabled");
					return 0;
				case 30:	// TIMEON
					Con_start_timer(total_commands, &input[1][0]);
					Print_to_console("timer display on");
					return 0;
				case 31:	// TIMEOFF
					displayTime = 0;
					Print_to_console("timer display off");
					return 0;
				case 32:	// TEXT
					displayTextNumbers = 1 - displayTextNumbers;
					if (displayTextNumbers)
						Print_to_console("text numbers on");
					else
						Print_to_console("text numbers off");
					return 0;
				case 33:	// SHOWVAR <varNo>
					// add variable to watch-list
					ShowVar(&input[1][0]);
					return 0;
				case 34:	// HIDEVAR <varNo>
					// remove variable from watch-list
					HideVar(&input[1][0]);
					return 0;
				case 35:	// VERSION
					Con_display_version();
					return(0);
				case 36:	// SOFT
				case 37:	// HARD
					// ScummVM doesn't distinguish between
					// software and hardware rendering
					Print_to_console("This command is no longer relevant");
					return 0;
				case 38:	// ANIMTEST
					// automatically do "s 32" to run the
					// text/speech testing start-script

					Con_start((uint8 *) "32");
					Print_to_console("Setting flag 'system_testing_anims'");

					// same as typing "VAR 912 <value>" at
					// the console

					Var_set((uint8 *) "912", &input[1][0]);
					return 1;
				case 39:	// TEXTTEST
					// automatically do "s 33" to run the
					// text/speech testing start-script

					Con_start((uint8 *) "33");

					Print_to_console("Setting flag 'system_testing_text'");

					// same as typing "VAR 1230 <value>" at
					// the console
					Var_set((uint8 *) "1230", &input[1][0]);

					displayTextNumbers = 1;
					Print_to_console("text numbers on");
					return 1;
				case 40:	// LINETEST
					// automatically do "s 33" to run the
					// text/speech testing start-script

					Con_start((uint8 *) "33");

					Print_to_console("Setting var 1230 (system_testing_text):");

					// same as typing "VAR 1230 <value>" at
					// the console
					Var_set((uint8 *) "1230", &input[1][0]);

					Print_to_console("Setting var 1264 (system_test_line_no):");

					// same as typing "VAR 1264 <value>" at
					// the console
					Var_set((uint8 *) "1264", &input[2][0]);

					displayTextNumbers = 1;
					Print_to_console("text numbers on");
					return 1;
				case 41:	// GRAB
					grabbingSequences = 1 - grabbingSequences;
					if (grabbingSequences)
						Print_to_console("PCX-grabbing enabled");
					else
						Print_to_console("PCX-grabbing disabled");
					return 0;
				case 42:	// EVENTS
					Con_display_events();
					return 0;
				case 43:	// SFX
					wantSfxDebug = 1 - wantSfxDebug;
					if (wantSfxDebug)
						Print_to_console("SFX logging activated (see zebug.txt)");
					else
						Print_to_console("SFX logging deactivated");
					return 0;
				case 44:	// ENGLISH
					InitialiseFontResourceFlags(DEFAULT_TEXT);
					Print_to_console("Default fonts selected");
					return 0;
				case 45:	// FINNISH
					InitialiseFontResourceFlags(FINNISH_TEXT);
					Print_to_console("Finnish fonts selected");
					return 0;
				case 46:	// POLISH
					InitialiseFontResourceFlags(POLISH_TEXT);
					Print_to_console("Polish fonts selected");
					return 0;
				default:
					Print_to_console("??");
					return 0;
				}
				break;
			case 1:
				// checks for YES and NO and returns the 1 or
				// 2 to the calling code
				switch (j) {
				case 4:	// YES
					return 1;
				case 5:	// NO
					return 2;
				default:
					Print_to_console("??");
					return 0;
				}
				break;
			}
			break;
		}
	}

	Print_to_console("?");	//couldn't find a proper match
	return 0;
}

void Con_help(void) {
	// print out a list of commands

	int command;
	int scrolls = 0;
	char c;

	Scroll_console();

	for (command = 0; command < TOTAL_CONSOLE_COMMANDS; command++) {
		Print_to_console((char *) commands[command]);
		Build_display();
		scrolls++;

		if (scrolls == 18) {
			Temp_print_to_console("- Press ESC to stop or any other key to continue");
			Build_display();

			do {
			  	ServiceWindows();
			} while (!KeyWaiting());

			ReadKey(&c);
			if (c == 27)
				break;

			// clear the Press Esc message ready for the new line
			Clear_console_line();
			scrolls = 0;
		}	
	}
}

void Con_print(uint8 *ascii, uint32 pen, uint32 paper) {
	// print pixels in closen pen number - no clipping on this one
	// where file is the graphic file of ascii characters

	_frameHeader *head;
	uint8 *charSet, *charPtr;
	int chr, x = 0;

	// open font file
	charSet = res_man.open(CONSOLE_FONT_ID);

	do {
		chr = (int) *(ascii);
		chr -= 32;

		head = (_frameHeader *) FetchFrameHeader(charSet, chr);
		charPtr = (uint8 *) (head + 1);

		Con_colour_block( x, head->width, head->height, pen, paper, charPtr);

		x += head->width + 1;
		ascii++;
	} while(*(ascii));

	// close font file
	res_man.close(CONSOLE_FONT_ID);
}

void Con_colour_block(int x, int width, int height, uint32 pen, uint32 paper, uint8 *sprite_data_ad) {
	int deltaX, xx, yy;
	char *ad;

	deltaX = con_width - width;

	ad = (char *) console_sprite->ad;

	// locate bottom character row
	ad += (con_width * (con_depth - con_chr_height)) + x;

	for (yy = 0; yy < height; yy++) {
		for (xx = 0; xx < width; xx++) {
			if (pen = *(sprite_data_ad++))	//color
				*(ad++)= (uint8) CON_PEN;
			else	*(ad++)= (uint8) paper;
		}
		ad += deltaX;
	}
}

void Con_fatal_error(const char *format, ...) {
	// Use this to alert the user of a major problem from which we cannot
	// allow the game to continue. While in console mode the user will
	// still be ble to use the console commands - which may be useful.
	// This message is also written with debug() in case the console
	// itself blows up

	va_list arg_ptr;
	char buf[150];
	uint8 white[4] = { 255, 255, 255, 0 };

	// set text colour in case screen is faded down! (James 05mar97)
	BS2_SetPalette(CON_PEN, 1, white, RDPAL_INSTANT);

	va_start(arg_ptr,format);
	_vsnprintf(buf, 150, format, arg_ptr);

	this_screen.background_layer_id = 0;	//in case error in display loop

	// write to file first in-case the screen is screwed up and we never
	// see the console

	debug(5, "CON_FATAL_ERROR: %s", buf);

	Print_to_console(buf);
	Print_to_console("fatal error, sword2 must terminate :-(  (%d)", ID);

	//mode 0 so all commands are available but quit will terminate the game
	Tconsole(0);

	Close_game();	// should down game services - free's mallocs, etc.
	CloseAppWindow();

	exit 0;
}

void Var_check(uint8 *pointer) {
	int var;

	sscanf((char *) pointer, "%d", &var);

	Print_to_console("%d", *(uint32 *) (res_man._resList[1]->ad + sizeof(_standardHeader) + 4 * var));
}

void Var_set(uint8 *pointer, uint8 *p2) {
	int var;
	int val;

	sscanf((char *) pointer, "%d", &var);
	sscanf((char *) p2, "%d", &val);

	Print_to_console("was %d", *(uint32 *) (res_man._resList[1]->ad + sizeof(_standardHeader) + 4 * var));

	*(uint32 *) (res_man._resList[1]->ad + sizeof(_standardHeader) + 4 * var) = val;

	Print_to_console("now %d", val);
}

void ShowVar(uint8 *varNoPtr) {
	int32 showVarNo = 0;
	int32 varNo;
	
	// 'varNo' is what we want to add
	sscanf((char *) varNoPtr, "%d", &varNo);

	// search for a spare slot in the watch-list, but also watch out for
	// this variable already being in the list

	while (showVarNo < MAX_SHOWVARS && showVar[showVarNo] != 0 && showVar[showVarNo] != varNo)
		showVarNo++;

	// if we've found a spare slot or the variable's already there
	if (showVarNo < MAX_SHOWVARS) {
		if (showVar[showVarNo] == 0) {
			// empty slot - add it to the list at this slot
			showVar[showVarNo] = varNo;
			Print_to_console("var(%d) added to the watch-list", varNo);
		} else
			Print_to_console("var(%d) already in the watch-list!", varNo);
	} else
		Print_to_console("Sorry - no more allowed - hide one or extend the system watch-list");
}

void HideVar(uint8 *varNoPtr) {
	int32 showVarNo = 0;
	int32 varNo;
	
	// 'varNo' is what we want to remove
	sscanf((char *) varNoPtr, "%d", &varNo);

	// search for 'varNo' in the watch-list
	while (showVarNo < MAX_SHOWVARS && showVar[showVarNo] != varNo)
		showVarNo++;

	if (showVarNo < MAX_SHOWVARS) {
		// We've found 'varNo' in the list - clear this slot
		showVar[showVarNo] = 0;
		Print_to_console("var(%d) removed from watch-list", varNo);
	} else
		Print_to_console("Sorry - can't find var(%d) in the list", varNo);
}

void Con_list_savegames(void) {
	uint8 description[SAVE_DESCRIPTION_LEN];
	int j, scrolls = 0;
	char c;

	for (j = 0; j < 100; j++) {
		// if there is a save game print the name
		if (GetSaveDescription(j, description) == SR_OK) {
			Print_to_console("%d: \"%s\"", j, description);

			scrolls++;
			Build_display();

			if (scrolls == 18) {
				Temp_print_to_console("- Press ESC to stop or any other key to continue");
				Build_display();

				do {
					ServiceWindows();
				} while (!KeyWaiting());

				ReadKey(&c);
				if (c == 27)
					break;

				// clear the Press Esc message ready for the
				// new line
				Clear_console_line();
				scrolls = 0;
			}
		}
}

void Con_save_game(int total_commands, uint8 *slotString, uint8 *description) {
	uint16 slotNo;
	uint32 rv;

	// if mouse if off, or system menu is locked off
	if (mouse_status || mouse_mode_locked) {
		Print_to_console("WARNING: Cannot save game while control menu unavailable!");
		return;
	}

	if (total_commands >= 3) {
		// SAVE <slot> <description>
		if (Is_number(slotString)) {
			slotNo = atoi((char *) slotString);
			rv = SaveGame(slotNo,description);

			if (rv == SR_OK)
				Print_to_console("Saved game \"%s\" to file \"savegame.%.3d\"", description, slotNo);
			else if (rv == SR_ERR_FILEOPEN)
				Print_to_console("ERROR: Cannot open file \"savegame.%.3d\"", slotNo);
			else	// SR_ERR_WRITEFAIL
				Print_to_console("ERROR: Write error on file \"savegame.%.3d\"", slotNo);
		}
	} else
		Print_to_console("Syntax Error: type SAVE (slot_number) (description)");
}

void Con_restore_game(int total_commands, uint8 *slotString) {
	uint16 slotNo;
	uint8 description[SAVE_DESCRIPTION_LEN];
	uint32 rv;

	// if mouse if off, or system menu is locked off
	if (mouse_status || mouse_mode_locked) {
		Print_to_console("WARNING: Cannot restore game while control menu unavailable!");
		return;
	}

	if (total_commands >= 2) {
		// RESTORE <slot>
		if (Is_number(slotString)) {
			slotNo = atoi((char *) slotString);
			rv = RestoreGame(slotNo);

			if (rv == SR_OK) {
				GetSaveDescription(slotNo, description);
				Print_to_console("Restored game \"%s\" from file \"savegame.%.3d\"", description, slotNo);
			} else if (rv == SR_ERR_FILEOPEN)
				Print_to_console("ERROR: Cannot open file \"savegame.%.3d\"", slotNo);

			else if (rv == SR_ERR_INCOMPATIBLE)
				Print_to_console("ERROR: \"savegame.%.3d\" is no longer compatible with current player/variable resources", slotNo);

			else	// SR_ERR_READFAIL
				Print_to_console("ERROR: Read error on file \"savegame.%.3d\"", slotNo);
		}
	} else
		Print_to_console("Syntax Error: type RESTORE (slot_number)");
}

void Con_start_timer(int total_commands, uint8 *slotString) {
	if (total_commands >= 2) {
		if (Is_number(slotString)) {
			startTime = timeGetTime() - (atoi((char *) slotString) * 1000);
		}
	} else {
		if (startTime = 0)
			startTime = timeGetTime();
	}
	displayTime = 1;
}

uint8 Is_number(uint8 *ascii) {
	// until we reach the null terminator
	while (*ascii) {
		if (*ascii >= '0' && *ascii <= '9')
			ascii++;
		else
			return 0;
	}

	return 1;
}

void Con_display_version(void) {
	struct tm *time;
	time_t t;
	char dateStamp[255];
	char version[6];

	strcpy(version, (char*) version_string + HEAD_LEN);
	*(((unsigned char *) &t)) = *(version_string + 14);
	*(((unsigned char *) &t) + 1) = *(version_string + 15);
	*(((unsigned char *) &t) + 2) = *(version_string + 16);
	*(((unsigned char *) &t) + 3) = *(version_string + 17);

	time = localtime(&t);
	sprintf(dateStamp, "%s", asctime(time));
	dateStamp[24] = 0;	// fudge over the newline character!

	Print_to_console("\"Broken Sword II\" (c) Revolution Software 1997.");
	Print_to_console("v%s created on %s for %s", version, dateStamp, unencoded_name + HEAD_LEN);
	Scroll_console();

	// THE FOLLOWING LINES ARE TO BE COMMENTED OUT OF THE FINAL VERSION
	// Print_to_console("This program has a personalised fingerprint encrypted into the code.");
	// Print_to_console("If this CD was not sent directly to you by Virgin Interactive or Revolution Software");
	// Print_to_console("then please contact James Long at Revolution on (+44) 1904 639698.");
	// Scroll_console();
}

void Con_display_events() {
	uint32 j;
	uint32 target;
	uint32 script;

	Print_to_console("EVENT LIST:");

	for (j = 0; j < MAX_events; j++) {
		if (event_list[j].id) {
			target = event_list[j].id;
			script = event_list[j].interact_id;

			Print_to_console("slot %d: id = %s (%d)", j, FetchObjectName(target), target);
			Print_to_console("         script = %s (%d) pos %d", FetchObjectName(script / 65536), script / 65536, script % 65536);
		}
	}
}

} // End of namespace Sword2

#else

namespace Sword2 {

void Print_to_console(const char *format, ...) {}
void Temp_print_to_console(const char *format, ...) {}
void Clear_console_line(void) {}
void Scroll_console(void) {}
void Init_console(void) {}
void StartConsole(void) {}

} // End of namespace Sword2

#endif
