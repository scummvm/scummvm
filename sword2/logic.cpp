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
#include "bs2/sword2.h"
#include "bs2/build_display.h"
#include "bs2/console.h"
#include "bs2/debug.h"
#include "bs2/interpreter.h"
#include "bs2/logic.h"
#include "bs2/router.h"		// for clearWalkGridList()
#include "bs2/sound.h"
#include "bs2/sync.h"

namespace Sword2 {

logic LLogic;

#define LEVEL (_curObjectHub->logic_level)

// this must allow for the largest number of objects in a screen
#define OBJECT_KILL_LIST_SIZE	50

uint32 object_kill_list[OBJECT_KILL_LIST_SIZE];

// keeps note of no. of objects in the kill list
uint32 kills = 0;

int logic::processSession(void) {
	// do one cycle of the current session

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
		head = (_standardHeader*) res_man.open(run_list);

		if (head->fileType != RUN_LIST)
			Con_fatal_error("Logic_engine %d not a run_list", run_list);

		game_object_list = (uint32 *) (head + 1);

		// read the next id
		ID = game_object_list[_pc++];
		id = ID;

		// release the list again so it can float in memory - at this
		// point not one thing should be locked

		res_man.close(run_list);

		debug(5, "%d", ID);

		// null terminated
		if (!ID) {
			// end the session naturally
			return 0;
		}

		head = (_standardHeader*) res_man.open(ID);

		if (head->fileType != GAME_OBJECT)
			Con_fatal_error("Logic_engine %d not an object", ID);

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
				ret = RunScript(raw_script_ad, raw_script_ad, &_curObjectHub->script_pc[LEVEL]);
			} else {
				// we're running the script of another game
				// object - get our data object address

				// get the foreign objects script data address

				raw_data_ad = (char*) head;

				far_head = (_standardHeader*) res_man.open(script / SIZE);

				if (far_head->fileType != GAME_OBJECT && far_head->fileType != SCREEN_MANAGER)
					Con_fatal_error("Logic_engine %d not a far object (its a %d)", script / SIZE, far_head->fileType);

				// raw_script_ad = (char*) (head + 1) + sizeof(_standardHeader);

				// get our objects data address
				// raw_data_ad = (char*) (_curObjectHub + 1);

				raw_script_ad = (char*) far_head;

				ret = RunScript(raw_script_ad, raw_data_ad, &_curObjectHub->script_pc[LEVEL]);

				// close foreign object again
				res_man.close(script / SIZE);

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
				Con_fatal_error("processSession: illegal script return type %d", ret);
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

			RunScript(raw_script_ad, raw_script_ad, &null_pc);
		}

		// made for all live objects

		// and that's it so close the object resource

		res_man.close(ID);
	}

	// leaving a room so remove all ids that must reboot correctly
	processKillList();

	debug(5, "RESTART the loop");

	// means restart the loop
	return 1;
}

void logic::expressChangeSession(uint32 sesh_id) {
	// a game-object can bring an immediate halt to the session and cause
	// a new one to start without a screen update

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

	// reset walkgrid list (see FN_register_walkgrid)
	router.clearWalkGridList();

	// stops all fx & clears the queue
	Clear_fx_queue();

	// free all the route memory blocks from previous game
	router.freeAllRouteMem();
}

void logic::naturalChangeSession(uint32 sesh_id) {
	// FIXME: This function doesn't seem to be used anywhere

	// A new session will begin next game cycle. The current cycle will
	// conclude and build the screen and flip into view as normal

	_currentRunList = sesh_id;
}

uint32 logic::getRunList(void) {
	// pass back the private _currentRunList variable
	return _currentRunList;
}

int32 FN_set_session(int32 *params) {
	// used by player invoked start scripts
	// param	0 id of new run list

	LLogic.expressChangeSession(params[0]);
	return IR_CONT;
}

int32 FN_end_session(int32 *params) {
	// causes no more objects in this logic loop to be processed
	// the logic engine will restart at the beginning of the new list
	// !!the current screen will not be drawn!!

	// param	0 id of new run-list

	// terminate current and change to next run-list
	LLogic.expressChangeSession(params[0]);

	// stop the script - logic engine will now go around and the new
	// screen will begin
	return IR_STOP;
}

void logic::logicUp(uint32 new_script) {
	// move the current object up a level
	// called by FN_gosub command - remember, only the logic object has
	// access to _curObjectHub

	// going up a level - and we'll keeping going this cycle
	LEVEL++;

	// can be 0, 1, 2
	if (LEVEL == 3)
		Con_fatal_error("logicUp id %d has run off script tree! :-O", ID);

	// setup new script on next level (not the current level)

	debug(5, "new pc = %d", new_script & 0xffff);

	_curObjectHub->script_id[LEVEL] = new_script;
	_curObjectHub->script_pc[LEVEL] = new_script & 0xffff;
}

void logic::logicOne(uint32 new_script) {
	// force to level one

	LEVEL = 1;

	// setup new script on level 1
	_curObjectHub->script_id[1] = new_script;
	_curObjectHub->script_pc[1] = new_script & 0xffff;
}

void logic::logicReplace(uint32 new_script) {
	// change current logic - script must quit with a TERMINATE directive
	// - which does not write to &pc

	// setup new script on this level
	_curObjectHub->script_id[LEVEL] = new_script;
	_curObjectHub->script_pc[LEVEL] = new_script & 0xffff;
}

uint32 logic::examineRunList(void) {
	uint32 *game_object_list;
	_standardHeader *file_header;
	int scrolls = 0;
	_keyboardEvent ke;

	if (_currentRunList) {
		// open and lock in place
		game_object_list = (uint32 *) (res_man.open(_currentRunList) + sizeof(_standardHeader));

		Print_to_console("runlist number %d", _currentRunList);

		while(*(game_object_list)) {
			file_header = (_standardHeader*) res_man.open(*(game_object_list));
			Print_to_console(" %d %s", *(game_object_list), file_header->name);
			res_man.close(*(game_object_list++));

			scrolls++;
			Build_display();

			if (scrolls == 18) {
				Temp_print_to_console("- Press ESC to stop or any other key to continue");
				Build_display();

				do {
					g_display->updateDisplay();
				} while (!KeyWaiting());

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

		res_man.close(_currentRunList);
	} else
		Print_to_console("no run list set");

	Scroll_console();
	return 1;
}

void logic::totalRestart(void) {
	// reset the object restart script 1 on level 0

	LEVEL = 0;
	// _curObjectHub->script_id[0] = 1;

	// reset to rerun
	_curObjectHub->script_pc[0] = 1;
}

int32 FN_total_restart(int32 *params) {
	// mega runs this to restart its base logic again - like being cached
	// in again

	LLogic.totalRestart();

	// drop out without saving pc and go around again
	return IR_TERMINATE;
}

int32 FN_add_to_kill_list(int32 *params) {
	// call *once* from object's logic script - ie. in startup code
	// - so not re-called every time script drops off & restarts!

	// Mark this object for killing - to be killed when player leaves
	// this screen. Object reloads & script restarts upon re-entry to
	// screen, which causes this object's startup logic to be re-run
	// every time we enter the screen. "Which is nice"

	// params: none

	uint32 entry;

	// DON'T EVER KILL GEORGE!
	if (ID != 8) {
		// first, scan list to see if this object is already included

		entry = 0;
		while (entry < kills && object_kill_list[entry] != ID)
			entry++;

		// if this ID isn't already in the list, then add it,
		// (otherwise finish)

		if (entry == kills) {
#ifdef _SWORD2_DEBUG
			// no room at the inn
			if (kills == OBJECT_KILL_LIST_SIZE)
				Con_fatal_error("List full in FN_add_to_kill_list(%u)", ID);
#endif

			// add this 'ID' to the kill list
			object_kill_list[kills] = ID;
			kills++;

			// "another one bites the dust"

			// when we leave the screen, all these object
			// resources are to be cleaned out of memory and the
			// kill list emptied by doing 'kills = 0', ensuring
			// that all resources are in fact still in memory &
			// more importantly closed before killing!
		}
	}

	// continue script
	return IR_CONT;
}

void logic::processKillList(void) {
	for (uint32 j = 0; j < kills; j++)
		res_man.remove(object_kill_list[j]);

	kills = 0;
}

void logic::resetKillList(void) {
	kills = 0;
}

} // End of namespace Sword2
