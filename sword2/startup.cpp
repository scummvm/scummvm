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

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

uint32 Logic::initStartMenu(void) {
	// Print out a list of all the start points available.
	// There should be a linc produced file called startup.txt.
	// This file should contain ascii numbers of all the resource game
	// objects that are screen managers.
	// We query each in turn and setup an array of start structures.
	// If the file doesn't exist then we say so and return a 0.

	File fp;

	uint32 pos = 0;
	char *raw_script;
	uint32 null_pc;

	char ascii_start_ids[MAX_starts][7];

	// ok, load in the master screen manager file

	_totalStartups = 0;	// no starts

	debug(5, "initialising start menu");

	if (!fp.open("startup.inf")) {
		warning("initStartMenu: cannot open startup.inf - the debugger won't have a start menu");
		return 0;
	}

	// The startup.inf file which contains a list of all the files. Now
	// extract the filenames

	while (1) {
		bool done = false;

		while (1) {
			byte b = fp.readByte();

			if (fp.ioFailed()) {
				done = true;
				break;
			}

			// Each item ends with CRLF
			if (b == 13) {
				fp.readByte();
				break;
			}

			if (pos < 7)
				ascii_start_ids[_totalScreenManagers][pos] = b;
			pos++;
		}

		if (done)
			break;

		// NULL terminate our extracted string
		ascii_start_ids[_totalScreenManagers][pos] = 0;

		// reset position in current slot between entries
		pos = 0;

		// done another
		_totalScreenManagers++;

		if (_totalScreenManagers == MAX_starts) {
			debug(5, "WARNING MAX_starts exceeded!");
			break;
		}
	}

	fp.close();

	// using this method the Gode generated resource.inf must have #0d0a
	// on the last entry

	debug(5, "%d screen manager objects", _totalScreenManagers);

	// Open each object and make a query call. The object must fill in a
	// startup structure. It may fill in several if it wishes - for
	// instance a startup could be set for later in the game where
	// specific vars are set

	for (uint i = 0; i < _totalScreenManagers; i++) {
		_startRes = atoi(ascii_start_ids[i]);

		debug(5, "+querying screen manager %d", _startRes);

		// resopen each one and run through the interpretter
		// script 0 is the query request script

		// if the resource number is within range & it's not a null
		// resource
		// - need to check in case un-built sections included in
		// start list

		if (_vm->_resman->checkValid(_startRes)) {
			debug(5, "- resource %d ok", _startRes);
			raw_script = (char *) _vm->_resman->openResource(_startRes);
			null_pc = 0;
			runScript(raw_script, raw_script, &null_pc);
			_vm->_resman->closeResource(_startRes);
		} else
			debug(5, "- resource %d invalid", _startRes);
	}

	return 1;
}

int32 Logic::fnRegisterStartPoint(int32 *params) {
	// params:	0 id of startup script to call - key
	// 		1 pointer to ascii message

#ifdef _SWORD2_DEBUG
	if (_totalStartups == MAX_starts)
		error("ERROR: _startList full");

	// +1 to allow for NULL terminator
	if (strlen((const char *) _vm->_memory->intToPtr(params[1])) + 1 > MAX_description)
		error("ERROR: startup description too long");
#endif

	// this objects id
	_startList[_totalStartups].start_res_id	= _startRes;

	// a key code to be passed to a script via a script var to SWITCH in
	// the correct start
	_startList[_totalStartups].key = params[0];

	strcpy(_startList[_totalStartups].description, (const char *) _vm->_memory->intToPtr(params[1]));

	// point to next
	_totalStartups++;

	return 1;
}

void Logic::conPrintStartMenu(void) {
	// the console 'starts' (or 's') command which lists out all the
	// registered start points in the game

	if (!_totalStartups) {
		Debug_Printf("Sorry - no startup positions registered?\n");

		if (!_totalScreenManagers)
			Debug_Printf("There is a problem with startup.inf\n");
		else
			Debug_Printf(" (%d screen managers found in startup.inf)\n", _totalScreenManagers);
	} else {
		for (uint i = 0; i < _totalStartups; i++)
			Debug_Printf("%d  (%s)\n", i, _startList[i].description);
	}
}

void Logic::conStart(int start) {
	char *raw_script;
	char *raw_data_ad;
	uint32 null_pc;

	if (!_totalStartups)
		Debug_Printf("Sorry - there are no startups!\n");
	else if (start >= 0 && start < (int) _totalStartups) {
		// do the startup as we've specified a legal start

		// restarting - stop sfx, music & speech!

		_vm->clearFxQueue();

		// fade out any music that is currently playing
		fnStopMusic(NULL);

		// halt the sample prematurely
		_vm->_sound->unpauseSpeech();
		_vm->_sound->stopSpeech();

		// clean out all resources & flags, ready for a total
		// restart

		// remove all resources from memory, including player
		// object & global variables

		_vm->_resman->removeAll();

		// reopen global variables resource & send address to
		// interpreter - it won't be moving
		setGlobalInterpreterVariables((int32 *) (_vm->_resman->openResource(1) + sizeof(StandardHeader)));
		_vm->_resman->closeResource(1);

		// free all the route memory blocks from previous game
		_router->freeAllRouteMem();

		// if there was speech text, kill the text block
		if (_speechTextBlocNo) {
			_vm->_fontRenderer->killTextBloc(_speechTextBlocNo);
			_speechTextBlocNo = 0;
		}

		// set the key

		// Open George
		raw_data_ad = (char *) _vm->_resman->openResource(8);
		raw_script = (char *) _vm->_resman->openResource(_startList[start].start_res_id);

		// denotes script to run
		null_pc = _startList[start].key & 0xffff;

		Debug_Printf("Running start %d\n", start);
		runScript(raw_script, raw_data_ad, &null_pc);

		_vm->_resman->closeResource(_startList[start].start_res_id);

		// Close George
		_vm->_resman->closeResource(8);

		// make sure thre's a mouse, in case restarting while
		// mouse not available
		fnAddHuman(NULL);
	} else
		Debug_Printf("Not a legal start position\n");
}

} // End of namespace Sword2
