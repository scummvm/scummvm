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
#include "bs2/build_display.h"
#include "bs2/console.h"
#include "bs2/debug.h"
#include "bs2/defs.h"
#include "bs2/header.h"
#include "bs2/interpreter.h"
#include "bs2/maketext.h"	// for Kill_text_bloc()
#include "bs2/memory.h"
#include "bs2/mouse.h"		// for FN_add_human()
#include "bs2/object.h"
#include "bs2/resman.h"
#include "bs2/router.h"
#include "bs2/sound.h"
#include "bs2/speech.h"		// for 'speech_text_bloc_no' - so that speech text can be cleared when running a new start-script
#include "bs2/startup.h"
#include "bs2/sword2.h"		// for CloseGame()
#include "bs2/sync.h"
#include "bs2/tony_gsdk.h"

namespace Sword2 {

uint32 total_startups = 0;
uint32 total_screen_managers = 0;
uint32 res;

_startup start_list[MAX_starts];

uint32 Init_start_menu(void) {
	// Print out a list of all the start points available.
	// There should be a linc produced file called startup.txt.
	// This file should contain ascii numbers of all the resource game
	// objects that are screen managers.
	// We query each in turn and setup an array of start structures.
	// If the file doesn't exist then we say so and return a 0.

	uint32 end;
	mem *temp;
	uint32 pos = 0;
	uint32 j = 0;
	char *raw_script;
	uint32 null_pc;

	char ascii_start_ids[MAX_starts][7];

	// ok, load in the master screen manager file

	total_startups = 0;	// no starts

	debug(5, "initialising start menu");

	if (!(end = Read_file("startup.inf", &temp, UID_temp))) {
		debug(5, "Init_start_menu cannot open startup.inf");
		return 0;	// meaning no start menu available
	}

	// Ok, we've loaded in the startup.inf file which contains a list of
	// all the files now extract the filenames

	do {
		while (temp->ad[j] != 13) {	// item must have an #0d0a
			ascii_start_ids[total_screen_managers][pos] = temp->ad[j];
			j++;
			pos++;
		}

		// NULL terminate our extracted string
		ascii_start_ids[total_screen_managers][pos] = 0;

		// reset position in current slot between entries
		pos = 0;

		// past the 0a
		j += 2;

		// done another
		total_screen_managers++;

		if (total_screen_managers == MAX_starts) {
			debug(5, "WARNING MAX_starts exceeded!");
			break;
		}
	} while (j <end);

	// using this method the Gode generated resource.inf must have #0d0a
	// on the last entry

	debug(5, "%d screen manager objects", total_screen_managers);

	// Open each object and make a query call. The object must fill in a
	// startup structure. It may fill in several if it wishes - for
	// instance a startup could be set for later in the game where
	// specific vars are set

	for (j = 0; j < total_screen_managers; j++) {
		res = atoi(ascii_start_ids[j]);

		debug(5, "+querying screen manager %d", res);

		// resopen each one and run through the interpretter
		// script 0 is the query request script

		// if the resource number is within range & it's not a null
		// resource (James 12mar97)
		// - need to check in case un-built sections included in
		// start list

		if (res_man.checkValid(res)) {
			debug(5, "- resource %d ok", res);
			raw_script = (char*) res_man.open(res);
			null_pc = 0;
			RunScript(raw_script, raw_script, &null_pc);
			res_man.close(res);
		} else
			debug(5, "- resource %d invalid", res);
	}

	memory.freeMemory(temp);

	return 1;
}

int32 FN_register_start_point(int32 *params) {
	// params:	0 id of startup script to call - key
	// 		1 pointer to ascii message

	debug(5, " FN_register_start_point %d %s", params[0], params[1]);

#ifdef _SWORD2_DEBUG
	if (total_startups == MAX_starts)
		Con_fatal_error("ERROR: start_list full");

	// +1 to allow for NULL terminator
	if (strlen((char*) params[1]) + 1 > MAX_description)
		Con_fatal_error("ERROR: startup description too long");
#endif

	// this objects id
	start_list[total_startups].start_res_id	= res;

	// a key code to be passed to a script via a script var to SWITCH in
	// the correct start
	start_list[total_startups].key = params[0];

	strcpy(start_list[total_startups].description, (char*) params[1]);

	//point to next
	total_startups++;

	return 1;
}

uint32 Con_print_start_menu(void) {
	// the console 'starts' (or 's') command which lists out all the
	// registered start points in the game

	uint32 j;
	int scrolls = 0;
	_keyboardEvent ke;

	if (!total_startups) {
		Print_to_console("Sorry - no startup positions registered?");

		if (!total_screen_managers)
			Print_to_console("There is a problem with startup.inf");
		else
			Print_to_console(" (%d screen managers found in startup.inf)", total_screen_managers);
	} else {
		for(j = 0; j < total_startups; j++) {
			Print_to_console("%d  (%s)", j, start_list[j].description);
			Build_display();
			scrolls++;

			if (scrolls == 18) {
				Temp_print_to_console("- Press ESC to stop or any other key to continue");
				Build_display();

				do {
					// Service windows
					ServiceWindows();
				} while(!KeyWaiting());

				// kill the key we just pressed
				ReadKey(&ke);
				if (ke.keycode == 27)
					break;

				// clear the Press Esc message ready for the
				// new line

				Clear_console_line();
				scrolls = 0;
			}	
		}
	}
	return 1;
}

uint32 Con_start(uint8 *input) {
	// if the second word id is a numeric that can be applied to a
	// genuine startup then do it

	uint32 j = 0;
	uint32 start;
	char *raw_script;
	char *raw_data_ad;
	uint32 null_pc;

	// so that typing 'S' then <enter> works on NT
	if (input[0] == 0) {
		Con_print_start_menu();
		return 1;
	}

	while (input[j]) {
		if (isdigit(input[j]))
			j++;
		else
			break;
	}

	//didn't quit out of loop on a non numeric chr$
	if (!input[j]) {
		start = atoi((char*) input);

		if (!total_startups)
			Print_to_console("Sorry - there are no startups!");
		else if (start < total_startups) {
			// do the startup as we've specified a legal start

			// restarting - stop sfx, music & speech!

			Clear_fx_queue();

			// fade out any music that is currently playing
			FN_stop_music(NULL);

			// halt the sample prematurely
			g_sound->unpauseSpeech();
			g_sound->stopSpeech();

			// clean out all resources & flags, ready for a total
			// restart (James24mar97)

			// remove all resources from memory, including player
			// object & global variables

			res_man.removeAll();

			// reopen global variables resource & send address to
			// interpreter - it won't be moving
			SetGlobalInterpreterVariables((int32 *) (res_man.open(1) + sizeof(_standardHeader)));
			res_man.close(1);

			// free all the route memory blocks from previous game
			router.freeAllRouteMem();

			// if there was speech text, kill the text block
			if (speech_text_bloc_no) {
				fontRenderer.killTextBloc(speech_text_bloc_no);
				speech_text_bloc_no = 0;
			}

			// set the key

			// Open George
			raw_data_ad = (char *) (res_man.open(8));
			raw_script = (char *) (res_man.open(start_list[start].start_res_id));

			// denotes script to run
			null_pc = start_list[start].key & 0xffff;

			Print_to_console("running start %d", start);
			RunScript(raw_script, raw_data_ad, &null_pc);

			res_man.close(start_list[start].start_res_id);

			// Close George
			res_man.close(8);

			// make sure thre's a mouse, in case restarting while
			// mouse not available
			FN_add_human(NULL);
		} else
			Print_to_console("not a legal start position");
	} else {
		// so that typing 'S' then <enter> works under Win95
		Con_print_start_menu();
	}

	return 1;
}

} // End of namespace Sword2
