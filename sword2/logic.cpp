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
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/debug.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/router.h"		// for clearWalkGridList()
#include "sword2/sound.h"
#include "sword2/sync.h"

namespace Sword2 {

Logic g_logic;

#define LEVEL (_curObjectHub->logic_level)

/**
 * Do one cycle of the current session.
 */

int Logic::processSession(void) {
	uint32 run_list;
	uint32 ret, script;
	uint32 *game_object_list;
	char *raw_script_ad;
	char *raw_data_ad;
	uint32 null_pc;
	_standardHeader *head;
	_standardHeader *far_head;
	uint32 id;

	// might change during the session, so take a copy here
	run_list = _currentRunList;

	// point to first object in list
	_pc = 0;

	// by minusing the pc we can cause an immediate cessation of logic
	// processing on the current list

	while (_pc != 0xffffffff) {
		head = (_standardHeader*) res_man->openResource(run_list);

		if (head->fileType != RUN_LIST)
			error("Logic_engine %d not a run_list", run_list);

		game_object_list = (uint32 *) (head + 1);

		// read the next id
		ID = game_object_list[_pc++];
		id = ID;

		// release the list again so it can float in memory - at this
		// point not one thing should be locked

		res_man->closeResource(run_list);

		debug(5, "%d", ID);

		// null terminated
		if (!ID) {
			// end the session naturally
			return 0;
		}

		head = (_standardHeader*) res_man->openResource(ID);

		if (head->fileType != GAME_OBJECT)
			error("Logic_engine %d not an object", ID);

		_curObjectHub = (_object_hub *) (head + 1);

		debug(5, " %d id(%d) pc(%d)",
			LEVEL,
			_curObjectHub->script_id[LEVEL],
			_curObjectHub->script_pc[LEVEL]);

		// do the logic for this object
		// we keep going until a function says to stop - remember,
		// system operations are run via function calls to drivers now

		do {
			// get the script id as we may be running a script
			// from another object...

			script = _curObjectHub->script_id[LEVEL];

			// there is a distinction between running one of our
			// own scripts and that of another object
			if (script / SIZE == ID) {
				// its our script

				debug(5, "run script %d pc%d",
					script / SIZE,
					_curObjectHub->script_pc[LEVEL]);

				// this is the script data
				// raw_script_ad = (char *) (_curObjectHub + 1);

				raw_script_ad = (char*) head;

				// script and data object are us/same
				ret = runScript(raw_script_ad, raw_script_ad, &_curObjectHub->script_pc[LEVEL]);
			} else {
				// we're running the script of another game
				// object - get our data object address

				// get the foreign objects script data address

				raw_data_ad = (char*) head;

				far_head = (_standardHeader*) res_man->openResource(script / SIZE);

				if (far_head->fileType != GAME_OBJECT && far_head->fileType != SCREEN_MANAGER)
					error("Logic_engine %d not a far object (its a %d)", script / SIZE, far_head->fileType);

				// raw_script_ad = (char*) (head + 1) + sizeof(_standardHeader);

				// get our objects data address
				// raw_data_ad = (char*) (_curObjectHub + 1);

				raw_script_ad = (char*) far_head;

				ret = runScript(raw_script_ad, raw_data_ad, &_curObjectHub->script_pc[LEVEL]);

				// close foreign object again
				res_man->closeResource(script / SIZE);

				// reset to us for service script
				raw_script_ad = raw_data_ad;
			}

			// this script has finished - drop down a level

			if (ret == 1) {
				// check that it's not already on level 0 !
				if (LEVEL)
					LEVEL--;
				else {
					// Hmmm, level 0 terminated :-| Let's
					// be different this time and simply
					// let it restart next go :-)

					debug(5, "**WARNING object %d script 0 terminated!", id);

					// reset to rerun
					_curObjectHub->script_pc[LEVEL] = _curObjectHub->script_id[LEVEL] & 0xffff;

					// cause us to drop out for a cycle
					ret = 0;
				}
			} else if (ret > 2) {
				error("processSession: illegal script return type %d", ret);
			}

			// if ret == 2 then we simply go around again - a new
			// script or subroutine will kick in and run

			// keep processing scripts until 0 for quit is returned
		} while (ret);

		// any post logic system requests to go here

		// clear any syncs that were waiting for this character - it
		// has used them or now looses them

		Clear_syncs(ID);

		if (_pc != 0xffffffff) {
			// the session is still valid so run the service script
			null_pc = 0;

			// call the base script - this is the graphic/mouse
			// service call

			runScript(raw_script_ad, raw_script_ad, &null_pc);
		}

		// made for all live objects

		// and that's it so close the object resource

		res_man->closeResource(ID);
	}

	// leaving a room so remove all ids that must reboot correctly
	processKillList();

	debug(5, "RESTART the loop");

	// means restart the loop
	return 1;
}

/**
 * Bring an immediate halt to the session and cause a new one to start without
 * a screen update.
 */

void Logic::expressChangeSession(uint32 sesh_id) {
	//set to new
	_currentRunList = sesh_id;

	//causes session to quit
	_pc = 0xffffffff;

	// reset now in case we double-clicked an exit prior to changing screen
	EXIT_FADING = 0;

	// we're trashing the list - presumably to change room
	// in theory sync waiting in the list could be left behind and never
	// removed - so we trash the lot

	Init_sync_system();

	// reset walkgrid list (see fnRegisterWalkGrid)
	router.clearWalkGridList();

	// stops all fx & clears the queue
	g_sword2->clearFxQueue();

	// free all the route memory blocks from previous game
	router.freeAllRouteMem();
}

/**
 * A new session will begin next game cycle. The current cycle will conclude
 * and build the screen and flip into view as normal.
 *
 * @note This functino doesn't seem to be used anywhere.
 */

void Logic::naturalChangeSession(uint32 sesh_id) {
	_currentRunList = sesh_id;
}

/**
 * @return The private _currentRunList variable.
 */

uint32 Logic::getRunList(void) {
	return _currentRunList;
}

int32 Logic::fnSetSession(int32 *params) {
	// used by player invoked start scripts

	// params:	0 id of new run list

	expressChangeSession(params[0]);
	return IR_CONT;
}

/**
 * Causes no more objects in this logic loop to be processed. The logic engine
 * will restart at the beginning of the new list. The current screen will not
 * be drawn!
 */

int32 Logic::fnEndSession(int32 *params) {
	// params:	0 id of new run-list

	// terminate current and change to next run-list
	expressChangeSession(params[0]);

	// stop the script - logic engine will now go around and the new
	// screen will begin
	return IR_STOP;
}

/**
 * Move the current object up a level. Called by fnGosub command. Remember:
 * only the logic object has access to _curObjectHub.
 */

void Logic::logicUp(uint32 new_script) {
	// going up a level - and we'll keeping going this cycle
	LEVEL++;

	// can be 0, 1, 2
	if (LEVEL == 3)
		error("logicUp id %d has run off script tree! :-O", ID);

	// setup new script on next level (not the current level)

	debug(5, "new pc = %d", new_script & 0xffff);

	_curObjectHub->script_id[LEVEL] = new_script;
	_curObjectHub->script_pc[LEVEL] = new_script & 0xffff;
}

/**
 * Force the level to one.
 */

void Logic::logicOne(uint32 new_script) {
	LEVEL = 1;

	// setup new script on level 1
	_curObjectHub->script_id[1] = new_script;
	_curObjectHub->script_pc[1] = new_script & 0xffff;
}

/**
 * Change current logic. Script must quit with a TERMINATE directive, which
 * does not write to &pc
 */

void Logic::logicReplace(uint32 new_script) {
	// setup new script on this level
	_curObjectHub->script_id[LEVEL] = new_script;
	_curObjectHub->script_pc[LEVEL] = new_script & 0xffff;
}

void Logic::examineRunList(void) {
	uint32 *game_object_list;
	_standardHeader *file_header;

	if (_currentRunList) {
		// open and lock in place
		game_object_list = (uint32 *) (res_man->openResource(_currentRunList) + sizeof(_standardHeader));

		Debug_Printf("Runlist number %d\n", _currentRunList);

		for (int i = 0; game_object_list[i]; i++) {
			file_header = (_standardHeader *) res_man->openResource(game_object_list[i]);
			Debug_Printf("%d %s\n", game_object_list[i], file_header->name);
			res_man->closeResource(game_object_list[i]);
		}

		res_man->closeResource(_currentRunList);
	} else
		Debug_Printf("No run list set\n");
}

/**
 * Reset the object and restart script 1 on level 0
 */

void Logic::totalRestart(void) {
	LEVEL = 0;

	// reset to rerun
	_curObjectHub->script_pc[0] = 1;
}

int32 Logic::fnTotalRestart(int32 *params) {
	// mega runs this to restart its base logic again - like being cached
	// in again

	// params:	none

	totalRestart();

	// drop out without saving pc and go around again
	return IR_TERMINATE;
}

/**
 * Mark this object for killing - to be killed when player leaves this screen.
 * Object reloads and script restarts upon re-entry to screen, which causes
 * this object's startup logic to be re-run every time we enter the screen.
 * "Which is nice."
 *
 * @note Call ONCE from object's logic script, i.e. in startup code, so not
 * re-called every time script frops off and restarts!
 */

int32 Logic::fnAddToKillList(int32 *params) {
	// params:	none

	uint32 entry;

	// DON'T EVER KILL GEORGE!
	if (ID != 8) {
		// first, scan list to see if this object is already included

		entry = 0;
		while (entry < _kills && _objectKillList[entry] != ID)
			entry++;

		// if this ID isn't already in the list, then add it,
		// (otherwise finish)

		if (entry == _kills) {
#ifdef _SWORD2_DEBUG
			// no room at the inn
			if (_kills == OBJECT_KILL_LIST_SIZE)
				error("List full in fnAddToKillList(%u)", ID);
#endif

			// add this 'ID' to the kill list
			_objectKillList[_kills] = ID;
			_kills++;

			// "another one bites the dust"

			// when we leave the screen, all these object
			// resources are to be cleaned out of memory and the
			// kill list emptied by doing '_kills = 0', ensuring
			// that all resources are in fact still in memory &
			// more importantly closed before killing!
		}
	}

	// continue script
	return IR_CONT;
}

void Logic::processKillList(void) {
	for (uint32 i = 0; i < _kills; i++)
		res_man->remove(_objectKillList[i]);

	_kills = 0;
}

void Logic::resetKillList(void) {
	_kills = 0;
}

} // End of namespace Sword2
