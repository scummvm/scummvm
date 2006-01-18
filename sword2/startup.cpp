/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/file.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"

namespace Sword2 {

bool Sword2Engine::initStartMenu() {
	// Print out a list of all the start points available.
	// There should be a linc produced file called startup.txt.
	// This file should contain ascii numbers of all the resource game
	// objects that are screen managers.
	// We query each in turn and setup an array of start structures.
	// If the file doesn't exist then we say so and return a 0.

	Common::File fp;

	// ok, load in the master screen manager file

	_totalStartups = 0;
	_totalScreenManagers = 0;

	if (!fp.open("startup.inf")) {
		warning("Cannot open startup.inf - the debugger won't have a start menu");
		return false;
	}

	// The startup.inf file which contains a list of all the files. Now
	// extract the filenames

	int start_ids[MAX_starts];

	while (1) {
		bool done = false;

		start_ids[_totalScreenManagers] = 0;

		// Scan the string until the LF in CRLF

		int b;

		do {
			b = fp.readByte();

			if (fp.ioFailed()) {
				done = true;
				break;
			}

			if (isdigit(b)) {
				start_ids[_totalScreenManagers] *= 10;
				start_ids[_totalScreenManagers] += (b - '0');
			}
		} while (b != 10);

		if (done)
			break;

		_totalScreenManagers++;

		if (_totalScreenManagers == MAX_starts) {
			warning("MAX_starts exceeded");
			break;
		}
	}

	fp.close();

	// Using this method the Gode generated resource.inf must have #0d0a
	// on the last entry

	debug(1, "%d screen manager objects", _totalScreenManagers);

	// Open each object and make a query call. The object must fill in a
	// startup structure. It may fill in several if it wishes - for
	// instance a startup could be set for later in the game where
	// specific vars are set

	for (uint i = 0; i < _totalScreenManagers; i++) {
		_startRes = start_ids[i];

		debug(2, "Querying screen manager %d", _startRes);

		// Open each one and run through the interpreter. Script 0 is
		// the query request script

		// if the resource number is within range & it's not a null
		// resource
		// - need to check in case un-built sections included in
		// start list

		if (_resman->checkValid(_startRes)) {
			char *raw_script = (char *)_resman->openResource(_startRes);
			uint32 null_pc = 0;

			_logic->runScript(raw_script, raw_script, &null_pc);
			_resman->closeResource(_startRes);
		} else
			warning("Start menu resource %d invalid", _startRes);
	}

	return 1;
}

void Sword2Engine::registerStartPoint(int32 key, char *name) {
	assert(_totalStartups < MAX_starts);

	_startList[_totalStartups].start_res_id	= _startRes;
	_startList[_totalStartups].key = key;

	strncpy(_startList[_totalStartups].description, name, MAX_description);
	_startList[_totalStartups].description[MAX_description - 1] = 0;

	_totalStartups++;
}

void Sword2Engine::runStart(int start) {
	// Restarting - stop sfx, music & speech!

	_sound->clearFxQueue();
	_logic->fnStopMusic(NULL);
	_sound->unpauseSpeech();
	_sound->stopSpeech();

	// Remove all resources from memory, including player object and global
	// variables

	_resman->removeAll();

	// Reopen global variables resource and player object
	setupPersistentResources();

	// Free all the route memory blocks from previous game
	_logic->_router->freeAllRouteMem();

	// If there was speech text, kill the text block
	if (_logic->_speechTextBlocNo) {
		_fontRenderer->killTextBloc(_logic->_speechTextBlocNo);
		_logic->_speechTextBlocNo = 0;
	}

	// Open George
	char *raw_data_ad = (char *)_resman->openResource(CUR_PLAYER_ID);
	char *raw_script = (char *)_resman->openResource(_startList[start].start_res_id);

	// Denotes script to run
	uint32 null_pc = _startList[start].key & 0xffff;

	_logic->runScript(raw_script, raw_data_ad, &null_pc);

	_resman->closeResource(_startList[start].start_res_id);
	_resman->closeResource(CUR_PLAYER_ID);

	// Make sure there's a mouse, in case restarting while mouse not
	// available
	_logic->fnAddHuman(NULL);
}

} // End of namespace Sword2
