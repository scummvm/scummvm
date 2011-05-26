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

#include "base/version.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/opcodes.h"
#include "agi/menu.h"

#include "common/random.h"
#include "common/textconsole.h"

namespace Agi {

#define p0	(p[0])
#define p1	(p[1])
#define p2	(p[2])
#define p3	(p[3])
#define p4	(p[4])
#define p5	(p[5])
#define p6	(p[6])

#define ip	_curLogic->cIP
#define vt	_game.viewTable[p0]
#define vt_v _game.viewTable[_game.vars[p0]]

#define _v _game.vars

void AgiEngine::cmd_increment(uint8 *p) {
	if (_v[p0] != 0xff)
		++_v[p0];
}

void AgiEngine::cmd_decrement(uint8 *p) {
	if (_v[p0] != 0)
		--_v[p0];
}

void AgiEngine::cmd_assignn(uint8 *p) {
	_v[p0] = p1;

	// WORKAROUND for a bug in fan _game "Get outta SQ"
	// Total number of points is stored in variable 7, which
	// is then incorrectly assigned to 0. Thus, when the _game
	// is restarted, "Points 0 of 0" is shown. We set the
	// variable to the correct value here
	// Fixes bug #1942476 - "AGI: Fan(Get Outta SQ) - Score
	// is lost on restart"
	if (getGameID() == GID_GETOUTTASQ && p0 == 7)
		_v[p0] = 8;
}

void AgiEngine::cmd_addn(uint8 *p) {
	_v[p0] += p1;
}

void AgiEngine::cmd_subn(uint8 *p) {
	_v[p0] -= p1;
}

void AgiEngine::cmd_assignv(uint8 *p) {
	_v[p0] = _v[p1];
}

void AgiEngine::cmd_addv(uint8 *p) {
	_v[p0] += _v[p1];
}

void AgiEngine::cmd_subv(uint8 *p) {
	_v[p0] -= _v[p1];
}

void AgiEngine::cmd_mul_n(uint8 *p) {
	_v[p0] *= p1;
}

void AgiEngine::cmd_mul_v(uint8 *p) {
	_v[p0] *= _v[p1];
}

void AgiEngine::cmd_div_n(uint8 *p) {
	_v[p0] /= p1;
}

void AgiEngine::cmd_div_v(uint8 *p) {
	_v[p0] /= _v[p1];
}

void AgiEngine::cmd_random(uint8 *p) {
	_v[p2] = _rnd->getRandomNumber(p1 - p0) + p0;
}

void AgiEngine::cmd_lindirectn(uint8 *p) {
	_v[_v[p0]] = p1;
}

void AgiEngine::cmd_lindirectv(uint8 *p) {
	_v[_v[p0]] = _v[p1];
}

void AgiEngine::cmd_rindirect(uint8 *p) {
	_v[p0] = _v[_v[p1]];
}

void AgiEngine::cmd_set(uint8 *p) {
	setflag(*p, true);
}

void AgiEngine::cmd_reset(uint8 *p) {
	setflag(*p, false);
}

void AgiEngine::cmd_toggle(uint8 *p) {
	setflag(*p, !getflag(*p));
}

void AgiEngine::cmd_set_v(uint8 *p) {
	setflag(_v[p0], true);
}

void AgiEngine::cmd_reset_v(uint8 *p) {
	setflag(_v[p0], false);
}

void AgiEngine::cmd_toggle_v(uint8 *p) {
	setflag(_v[p0], !getflag(_v[p0]));
}

void AgiEngine::cmd_new_room(uint8 *p) {
	newRoom(p0);

	// WORKAROUND: Works around intro skipping bug (#1737343) in Gold Rush.
	// Intro was skipped because the enter-keypress finalizing the entering
	// of the copy protection string (Copy protection is in logic.128) was
	// left over to the intro scene (Starts with room 73 i.e. logic.073).
	// The intro scene checks for any keys pressed and if it finds any it
	// jumps to the _game's start (Room 1 i.e. logic.001). We clear the
	// keyboard buffer when the intro sequence's first room (Room 73) is
	// loaded so that no keys from the copy protection scene can be left
	// over to cause the intro to skip to the _game's start.
	if (getGameID() == GID_GOLDRUSH && p0 == 73)
		_game.keypress = 0;
}

void AgiEngine::cmd_new_room_f(uint8 *p) {
	newRoom(_v[p0]);
}

void AgiEngine::cmd_load_view(uint8 *p) {
	agiLoadResource(rVIEW, p0);
}

void AgiEngine::cmd_load_logic(uint8 *p) {
	agiLoadResource(rLOGIC, p0);
}

void AgiEngine::cmd_load_sound(uint8 *p) {
	agiLoadResource(rSOUND, p0);
}

void AgiEngine::cmd_load_view_f(uint8 *p) {
	agiLoadResource(rVIEW, _v[p0]);
}

void AgiEngine::cmd_load_logic_f(uint8 *p) {
	agiLoadResource(rLOGIC, _v[p0]);
}

void AgiEngine::cmd_discard_view(uint8 *p) {
	agiUnloadResource(rVIEW, p0);
}

void AgiEngine::cmd_object_on_anything(uint8 *p) {
	vt.flags &= ~(ON_WATER | ON_LAND);
}

void AgiEngine::cmd_object_on_land(uint8 *p) {
	vt.flags |= ON_LAND;
}

void AgiEngine::cmd_object_on_water(uint8 *p) {
	vt.flags |= ON_WATER;
}

void AgiEngine::cmd_observe_horizon(uint8 *p) {
	vt.flags &= ~IGNORE_HORIZON;
}

void AgiEngine::cmd_ignore_horizon(uint8 *p) {
	vt.flags |= IGNORE_HORIZON;
}

void AgiEngine::cmd_observe_objs(uint8 *p) {
	vt.flags &= ~IGNORE_OBJECTS;
}

void AgiEngine::cmd_ignore_objs(uint8 *p) {
	vt.flags |= IGNORE_OBJECTS;
}

void AgiEngine::cmd_observe_blocks(uint8 *p) {
	vt.flags &= ~IGNORE_BLOCKS;
}

void AgiEngine::cmd_ignore_blocks(uint8 *p) {
	vt.flags |= IGNORE_BLOCKS;
}

void AgiEngine::cmd_set_horizon(uint8 *p) {
	_game.horizon = p0;
}

void AgiEngine::cmd_get_priority(uint8 *p) {
	_v[p1] = vt.priority;
}

void AgiEngine::cmd_set_priority(uint8 *p) {
	vt.flags |= FIXED_PRIORITY;
	vt.priority = p1;

	// WORKAROUND: this fixes bug #1712585 in KQ4 (dwarf sprite priority)
	// For this scene, ego (Rosella) hasn't got a fixed priority till script 54
	// explicitly sets priority 8 for her, so that she can walk back to the table
	// without being drawn over the other dwarfs
	// It seems that in this scene, ego's priority is set to 8, but the priority of
	// the last dwarf with the soup bowls (view 152) is also set to 8, which causes
	// the dwarf to be drawn behind ego
	// With this workaround, when the _game scripts set the priority of view 152
	// (seventh dwarf with soup bowls), ego's priority is set to 7
	// The _game script itself sets priotity 8 for ego before she starts walking,
	// and then releases the fixed priority set on ego after ego is seated
	// Therefore, this workaround only affects that specific part of this scene
	// Ego is set to object 19 by script 54
	if (getGameID() == GID_KQ4 && vt.currentView == 152) {
		_game.viewTable[19].flags |= FIXED_PRIORITY;
		_game.viewTable[19].priority = 7;
	}
}

void AgiEngine::cmd_set_priority_f(uint8 *p) {
	vt.flags |= FIXED_PRIORITY;
	vt.priority = _v[p1];
}

void AgiEngine::cmd_release_priority(uint8 *p) {
	vt.flags &= ~FIXED_PRIORITY;
}

void AgiEngine::cmd_set_upper_left(uint8 *p) {				// do nothing (AGI 2.917)
}

void AgiEngine::cmd_start_update(uint8 *p) {
	startUpdate(&vt);
}

void AgiEngine::cmd_stop_update(uint8 *p) {
	stopUpdate(&vt);
}

void AgiEngine::cmd_current_view(uint8 *p) {
	_v[p1] = vt.currentView;
}

void AgiEngine::cmd_current_cel(uint8 *p) {
	_v[p1] = vt.currentCel;
	debugC(4, kDebugLevelScripts, "v%d=%d", p1, _v[p1]);
}

void AgiEngine::cmd_current_loop(uint8 *p) {
	_v[p1] = vt.currentLoop;
}

void AgiEngine::cmd_last_cel(uint8 *p) {
	_v[p1] = vt.loopData->numCels - 1;
}

void AgiEngine::cmd_set_cel(uint8 *p) {
	setCel(&vt, p1);
	vt.flags &= ~DONTUPDATE;
}

void AgiEngine::cmd_set_cel_f(uint8 *p) {
	setCel(&vt, _v[p1]);
	vt.flags &= ~DONTUPDATE;
}

void AgiEngine::cmd_set_view(uint8 *p) {
	setView(&vt, p1);
}

void AgiEngine::cmd_set_view_f(uint8 *p) {
	setView(&vt, _v[p1]);
}

void AgiEngine::cmd_set_loop(uint8 *p) {
	setLoop(&vt, p1);
}

void AgiEngine::cmd_set_loop_f(uint8 *p) {
	setLoop(&vt, _v[p1]);
}

void AgiEngine::cmd_number_of_loops(uint8 *p) {
	_v[p1] = vt.numLoops;
}

void AgiEngine::cmd_fix_loop(uint8 *p) {
	vt.flags |= FIX_LOOP;
}

void AgiEngine::cmd_release_loop(uint8 *p) {
	vt.flags &= ~FIX_LOOP;
}

void AgiEngine::cmd_step_size(uint8 *p) {
	vt.stepSize = _v[p1];
}

void AgiEngine::cmd_step_time(uint8 *p) {
	vt.stepTime = vt.stepTimeCount = _v[p1];
}

void AgiEngine::cmd_cycle_time(uint8 *p) {
	vt.cycleTime = vt.cycleTimeCount = _v[p1];
}

void AgiEngine::cmd_stop_cycling(uint8 *p) {
	vt.flags &= ~CYCLING;
}

void AgiEngine::cmd_start_cycling(uint8 *p) {
	vt.flags |= CYCLING;
}

void AgiEngine::cmd_normal_cycle(uint8 *p) {
	vt.cycle = CYCLE_NORMAL;
	vt.flags |= CYCLING;
}

void AgiEngine::cmd_reverse_cycle(uint8 *p) {
	vt.cycle = CYCLE_REVERSE;
	vt.flags |= CYCLING;
}

void AgiEngine::cmd_set_dir(uint8 *p) {
	vt.direction = _v[p1];
}

void AgiEngine::cmd_get_dir(uint8 *p) {
	_v[p1] = vt.direction;
}

void AgiEngine::cmd_get_room_f(uint8 *p) {
	_v[p1] = objectGetLocation(_v[p0]);
}

void AgiEngine::cmd_put(uint8 *p) {
	objectSetLocation(p0, _v[p1]);
}

void AgiEngine::cmd_put_f(uint8 *p) {
	objectSetLocation(_v[p0], _v[p1]);
}

void AgiEngine::cmd_drop(uint8 *p) {
	objectSetLocation(p0, 0);
}

void AgiEngine::cmd_get(uint8 *p) {
	objectSetLocation(p0, EGO_OWNED);
}

void AgiEngine::cmd_get_f(uint8 *p) {
	objectSetLocation(_v[p0], EGO_OWNED);
}

void AgiEngine::cmd_word_to_string(uint8 *p) {
	strcpy(_game.strings[p0], _game.egoWords[p1].word);
}

void AgiEngine::cmd_open_dialogue(uint8 *p) {
	_game.hasWindow = true;
}

void AgiEngine::cmd_close_dialogue(uint8 *p) {
	_game.hasWindow = false;
}

void AgiEngine::cmd_close_window(uint8 *p) {
	closeWindow();
}

void AgiEngine::cmd_status_line_on(uint8 *p) {
	_game.statusLine = true;
	writeStatus();
}

void AgiEngine::cmd_status_line_off(uint8 *p) {
	_game.statusLine = false;
	writeStatus();
}

void AgiEngine::cmd_show_obj(uint8 *p) {
	_sprites->showObj(p0);
}

void AgiEngine::cmd_show_obj_v(uint8 *p) {
	_sprites->showObj(_v[p0]);
}

void AgiEngine::cmd_sound(uint8 *p) {
	_sound->startSound(p0, p1);
}

void AgiEngine::cmd_stop_sound(uint8 *p) {
	_sound->stopSound();
}

void AgiEngine::cmd_menu_input(uint8 *p) {
	newInputMode(INPUT_MENU);
}

void AgiEngine::cmd_enable_item(uint8 *p) {
	_menu->setItem(p0, true);
}

void AgiEngine::cmd_disable_item(uint8 *p) {
	_menu->setItem(p0, false);
}

void AgiEngine::cmd_submit_menu(uint8 *p) {
	_menu->submit();
}

void AgiEngine::cmd_set_scan_start(uint8 *p) {
	_curLogic->sIP = _curLogic->cIP;
}

void AgiEngine::cmd_reset_scan_start(uint8 *p) {
	_curLogic->sIP = 2;
}

void AgiEngine::cmd_save_game(uint8 *p) {
	_game.simpleSave ? saveGameSimple() : saveGameDialog();
}

void AgiEngine::cmd_load_game(uint8 *p) {
	assert(1);
	_game.simpleSave ? loadGameSimple() : loadGameDialog();
}

void AgiEngine::cmd_init_disk(uint8 *p) {				// do nothing
}

void AgiEngine::cmd_log(uint8 *p) {				// do nothing
}

void AgiEngine::cmd_trace_on(uint8 *p) {				// do nothing
}

void AgiEngine::cmd_trace_info(uint8 *p) {				// do nothing
}

void AgiEngine::cmd_show_mem(uint8 *p) {
	messageBox("Enough memory");
}

void AgiEngine::cmd_init_joy(uint8 *p) { // do nothing
}

void AgiEngine::cmd_script_size(uint8 *p) {
	debug(0, "script.size(%d)", p0);
}

void AgiEngine::cmd_cancel_line(uint8 *p) {
	_game.inputBuffer[0] = 0;
	_game.cursorPos = 0;
	writePrompt();
}

// This implementation is based on observations of Amiga's Gold Rush.
// You can try this out (in the original and in ScummVM) by writing "bird man"
// to enter Gold Rush's debug mode and then writing "show position" or "sp".
// TODO: Make the cycle and motion status lines more like in Amiga's Gold Rush.
// TODO: Add control status line (After stepsize, before cycle status).
// Don't know what the control status means yet, possibly flags?
// Examples of the control-value (Taken in the first screen i.e. room 1):
// 4051 (When ego is stationary),
// 471 (When walking on the first screen's bridge),
// 71 (When walking around, using the mouse or the keyboard).
void AgiEngine::cmd_obj_status_f(uint8 *p) {
	const char *cycleDesc;  // Object's cycle description line
	const char *motionDesc; // Object's motion description line
	char msg[256];          // The whole object status message

	// Generate cycle description line
	switch (vt_v.cycle) {
	case CYCLE_NORMAL:
		cycleDesc = "normal cycle";
		break;
	case CYCLE_END_OF_LOOP:
		cycleDesc = "end of loop";
		break;
	case CYCLE_REV_LOOP:
		cycleDesc = "reverse loop";
		break;
	case CYCLE_REVERSE:
		cycleDesc = "reverse cycle";
		break;
	default:
		cycleDesc = "unknown cycle type";
		break;
	}

	// Generate motion description line
	switch (vt_v.motion) {
	case MOTION_NORMAL:
		motionDesc = "normal motion";
		break;
	case MOTION_WANDER:
		motionDesc = "wandering";
		break;
	case MOTION_FOLLOW_EGO:
		motionDesc = "following ego";
		break;
	case MOTION_MOVE_OBJ:
		// Amiga's Gold Rush! most probably uses "move to (x, y)"
		// here with real values for x and y. The same output
		// is used when moving the ego around using the mouse.
		motionDesc = "moving to a point";
		break;
	default:
		motionDesc = "unknown motion type";
		break;
	}

	sprintf(msg,
		"Object %d:\n" \
		"x: %d  xsize: %d\n" \
		"y: %d  ysize: %d\n" \
		"pri: %d\n" \
		"stepsize: %d\n" \
		"%s\n" \
		"%s",
		_v[p0],
		vt_v.xPos, vt_v.xSize,
		vt_v.yPos, vt_v.ySize,
		vt_v.priority,
		vt_v.stepSize,
		cycleDesc,
		motionDesc);
	messageBox(msg);
}

// unknown commands:
// unk_170: Force savegame name -- j5
// unk_171: script save -- j5
// unk_172: script restore -- j5
// unk_173: Activate keypressed control (ego only moves while key is pressed)
// unk_174: Change priority table (used in KQ4) -- j5
// unk_177: Disable menus completely -- j5
// unk_181: Deactivate keypressed control (default control of ego)
void AgiEngine::cmd_set_simple(uint8 *p) {
	if (!(getFeatures() & (GF_AGI256 | GF_AGI256_2))) {
		_game.simpleSave = true;
	} else { // AGI256 and AGI256-2 use this unknown170 command to load 256 color pictures.
		// Load the picture. Similar to void AgiEngine::cmd_load_pic(uint8 *p).
		_sprites->eraseBoth();
		agiLoadResource(rPICTURE, _v[p0]);

		// Draw the picture. Similar to void AgiEngine::cmd_draw_pic(uint8 *p).
		_picture->decodePicture(_v[p0], false, true);
		_sprites->blitBoth();
		_game.pictureShown = 0;

		// Show the picture. Similar to void AgiEngine::cmd_show_pic(uint8 *p).
		setflag(fOutputMode, false);
		closeWindow();
		_picture->showPic();
		_game.pictureShown = 1;

		// Simulate slowww computer. Many effects rely on this
		pause(kPausePicture);
	}
}

void AgiEngine::cmd_pop_script(uint8 *p) {
	if (getVersion() >= 0x2915) {
		debug(0, "pop.script");
	}
}

void AgiEngine::cmd_hold_key(uint8 *p) {
	if (getVersion() >= 0x3098) {
		_egoHoldKey = true;
	}
}

void AgiEngine::cmd_discard_sound(uint8 *p) {
	if (getVersion() >= 0x2936) {
		debug(0, "discard.sound");
	}
}

void AgiEngine::cmd_hide_mouse(uint8 *p) {
	// WORKAROUND: Turns off current movement that's being caused with the mouse.
	// This fixes problems with too many popup boxes appearing in the Amiga
	// Gold Rush's copy protection failure scene (i.e. the hanging scene, logic.192).
	// Previously multiple popup boxes appeared one after another if you tried
	// to walk somewhere else than to the right using the mouse.
	// FIXME: Write a proper implementation using disassembly and
	//        apply it to other games as well if applicable.
	_game.viewTable[0].flags &= ~ADJ_EGO_XY;

	g_system->showMouse(false);
}

void AgiEngine::cmd_allow_menu(uint8 *p) {
	if (getVersion() >= 0x3098) {
		setflag(fMenusWork, ((p0 != 0) ? true : false));
	}
}

void AgiEngine::cmd_show_mouse(uint8 *p) {
	g_system->showMouse(true);
}

void AgiEngine::cmd_fence_mouse(uint8 *p) {
	_game.mouseFence.moveTo(p0, p1);
	_game.mouseFence.setWidth(p2 - p0);
	_game.mouseFence.setHeight(p3 - p1);
}

void AgiEngine::cmd_release_key(uint8 *p) {
	if (getVersion() >= 0x3098) {
		_egoHoldKey = false;
	}
}

void AgiEngine::cmd_adj_ego_move_to_x_y(uint8 *p) {
	int8 x, y;

	switch (logicNamesCmd[182].numArgs) {
	// The 2 arguments version is used at least in Amiga Gold Rush!
	// (v2.05 1989-03-09, Amiga AGI 2.316) in logics 130 and 150
	// (Using arguments (0, 0), (0, 7), (0, 8), (9, 9) and (-9, 9)).
	case 2:
		// Both arguments are signed 8-bit (i.e. in range -128 to +127).
		x = (int8) p0;
		y = (int8) p1;

		// Turn off ego's current movement caused with the mouse if
		// adj.ego.move.to.x.y is called with other arguments than previously.
		// Fixes weird looping behavior when walking to a ladder in the mines
		// (Rooms 147-162) in Gold Rush using the mouse. Sometimes the ego didn't
		// stop when walking to a ladder using the mouse but kept moving on the
		// ladder in a horizontally looping manner i.e. from right to left, from
		// right to left etc. In the Amiga Gold Rush the ego stopped when getting
		// onto the ladder so this is more like it (Although that may be caused
		// by something else because this command doesn't do any flag manipulations
		// in the Amiga version - checked it with disassembly).
		if (x != _game.adjMouseX || y != _game.adjMouseY)
			_game.viewTable[EGO_VIEW_TABLE].flags &= ~ADJ_EGO_XY;

		_game.adjMouseX = x;
		_game.adjMouseY = y;

		debugC(4, kDebugLevelScripts, "adj.ego.move.to.x.y(%d, %d)", x, y);
		break;
	// TODO: Check where (if anywhere) the 0 arguments version is used
	case 0:
	default:
		_game.viewTable[0].flags |= ADJ_EGO_XY;
		break;
	}
}

void AgiEngine::cmd_parse(uint8 *p) {
	_v[vWordNotFound] = 0;
	setflag(fEnteredCli, false);
	setflag(fSaidAcceptedInput, false);

	dictionaryWords(agiSprintf(_game.strings[p0]));
}

void AgiEngine::cmd_call(uint8 *p) {
	int oldCIP;
	int oldLognum;

	// CM: we don't save sIP because set.scan.start can be
	//     used in a called script (fixes xmas demo)
	oldCIP = _curLogic->cIP;
	oldLognum = _game.lognum;

	runLogic(p0);

	_game.lognum = oldLognum;
	_curLogic = &_game.logics[_game.lognum];
	_curLogic->cIP = oldCIP;
}

void AgiEngine::cmd_call_f(uint8 *p) {
	cmd_call(&_v[p0]);
}

void AgiEngine::cmd_draw_pic(uint8 *p) {
	debugC(6, kDebugLevelScripts, "=== draw pic %d ===", _v[p0]);
	_sprites->eraseBoth();
	_picture->decodePicture(_v[p0], true);
	_sprites->blitBoth();
	_sprites->commitBoth();
	_game.pictureShown = 0;
	debugC(6, kDebugLevelScripts, "--- end of draw pic %d ---", _v[p0]);

	// WORKAROUND for a script bug which exists in SQ1, logic scripts
	// 20 and 110. Flag 103 is not reset correctly, which leads to erroneous
	// behavior from view 46 (the spider droid). View 46 is supposed to
	// follow ego and explode when it comes in contact with him. However, as
	// flag 103 is not reset correctly, when the player goes down the path
	// and back up, the spider is always at the base of the path (since it
	// can't go up) and kills the player when he goes down at ground level
	// (although the spider droid sprite itself seems to be correctly positioned).
	// With this workaround, when the player goes back to picture 20 (1 screen
	// above the ground), flag 103 is reset, thereby fixing this issue. Note
	// that this is a script bug and occurs in the original interpreter as well.
	// Fixes bug #1658514: AGI: SQ1 (2.2 DOS ENG) bizzare exploding roger
	if (getGameID() == GID_SQ1 && _v[p0] == 20)
		setflag(103, false);

	// Simulate slowww computer. Many effects rely on this
	pause(kPausePicture);
}

void AgiEngine::cmd_show_pic(uint8 *p) {
	debugC(6, kDebugLevelScripts, "=== show pic ===");

	setflag(fOutputMode, false);
	closeWindow();
	_picture->showPic();
	_game.pictureShown = 1;

	debugC(6, kDebugLevelScripts, "--- end of show pic ---");
}

void AgiEngine::cmd_load_pic(uint8 *p) {
	_sprites->eraseBoth();
	agiLoadResource(rPICTURE, _v[p0]);
	_sprites->blitBoth();
	_sprites->commitBoth();
}

void AgiEngine::cmd_discard_pic(uint8 *p) {
	debugC(6, kDebugLevelScripts, "--- discard pic ---");
	// do nothing
}

void AgiEngine::cmd_overlay_pic(uint8 *p) {
	debugC(6, kDebugLevelScripts, "--- overlay pic ---");

	_sprites->eraseBoth();
	_picture->decodePicture(_v[p0], false);
	_sprites->blitBoth();
	_game.pictureShown = 0;
	_sprites->commitBoth();

	// Simulate slowww computer. Many effects rely on this
	pause(kPausePicture);
}

void AgiEngine::cmd_show_pri_screen(uint8 *p) {
	_debug.priority = 1;
	_sprites->eraseBoth();
	_picture->showPic();
	_sprites->blitBoth();

	waitKey();

	_debug.priority = 0;
	_sprites->eraseBoth();
	_picture->showPic();
	_sprites->blitBoth();
}

void AgiEngine::cmd_animate_obj(uint8 *p) {
	if (vt.flags & ANIMATED)
		return;

	debugC(4, kDebugLevelScripts, "animate vt entry #%d", p0);
	vt.flags = ANIMATED | UPDATE | CYCLING;
	vt.motion = MOTION_NORMAL;
	vt.cycle = CYCLE_NORMAL;
	vt.direction = 0;
}

void AgiEngine::cmd_unanimate_all(uint8 *p) {
	int i;

	for (i = 0; i < MAX_VIEWTABLE; i++)
		_game.viewTable[i].flags &= ~(ANIMATED | DRAWN);
}

void AgiEngine::cmd_draw(uint8 *p) {
	if (vt.flags & DRAWN)
		return;

	if (vt.ySize <= 0 || vt.xSize <= 0)
		return;

	debugC(4, kDebugLevelScripts, "draw entry %d", vt.entry);

	vt.flags |= UPDATE;
	if (getVersion() >= 0x3000) {
		setLoop(&vt, vt.currentLoop);
		setCel(&vt, vt.currentCel);
	}

	fixPosition(p0);
	vt.xPos2 = vt.xPos;
	vt.yPos2 = vt.yPos;
	vt.celData2 = vt.celData;
	_sprites->eraseUpdSprites();
	vt.flags |= DRAWN;

	// WORKAROUND: This fixes a bug with AGI Fanmade _game Space Trek.
	// The original workaround checked if AGI version was <= 2.440, which could
	// cause regressions with some AGI games. The original workaround no longer
	// works for Space Trek in ScummVM, as all fanmade games are set to use
	// AGI version 2.917, but it applies to all other games where AGI version is
	// <= 2.440, which was not the original purpose of this workaround. It is
	// assumed that this bug is caused by AGI Studio, so this applies to all
	// fanmade games only.
	// TODO: Investigate this further and check if any other fanmade AGI
	// games are affected. If yes, then it'd be best to set this for Space
	// Trek only
	if (getFeatures() & GF_FANMADE)	// See Sarien bug #546562
		vt.flags |= ANIMATED;

	_sprites->blitUpdSprites();
	vt.flags &= ~DONTUPDATE;

	_sprites->commitBlock(vt.xPos, vt.yPos - vt.ySize + 1, vt.xPos + vt.xSize - 1, vt.yPos, true);

	debugC(4, kDebugLevelScripts, "vt entry #%d flags = %02x", p0, vt.flags);
}

void AgiEngine::cmd_erase(uint8 *p) {
	if (~vt.flags & DRAWN)
		return;

	_sprites->eraseUpdSprites();

	if (vt.flags & UPDATE) {
		vt.flags &= ~DRAWN;
	} else {
		_sprites->eraseNonupdSprites();
		vt.flags &= ~DRAWN;
		_sprites->blitNonupdSprites();
	}
	_sprites->blitUpdSprites();

	int x1, y1, x2, y2;

	x1 = MIN((int)MIN(vt.xPos, vt.xPos2), MIN(vt.xPos + vt.celData->width, vt.xPos2 + vt.celData2->width));
	x2 = MAX((int)MAX(vt.xPos, vt.xPos2), MAX(vt.xPos + vt.celData->width, vt.xPos2 + vt.celData2->width));
	y1 = MIN((int)MIN(vt.yPos, vt.yPos2), MIN(vt.yPos - vt.celData->height, vt.yPos2 - vt.celData2->height));
	y2 = MAX((int)MAX(vt.yPos, vt.yPos2), MAX(vt.yPos - vt.celData->height, vt.yPos2 - vt.celData2->height));

	_sprites->commitBlock(x1, y1, x2, y2, true);
}

void AgiEngine::cmd_position(uint8 *p) {
	vt.xPos = vt.xPos2 = p1;
	vt.yPos = vt.yPos2 = p2;

	// WORKAROUND: Part of the fix for bug #1659209 "AGI: Space Trek sprite duplication"
	// with an accompanying identical workaround in position.v-command (i.e. command 0x26).
	// These two workarounds together make up the whole fix. The bug was caused by
	// wrongly written script data in Space Trek v1.0's scripts (At least logics 4 and 11).
	// Position-command was called with horizontal values over 200 (Outside the screen!).
	// Clipping the coordinates so the views stay wholly on-screen seems to fix the problems.
	//   It is probable (Would have to check better with disassembly to be completely sure)
	// that AGI 2.440 clipped its coordinates in its position and position.v-commands
	// although AGI 2.917 certainly doesn't (Checked that with disassembly) and that's why
	// Space Trek may have worked better with AGI 2.440 than with some other AGI versions.
	//   I haven't checked but if Space Trek solely abuses the position-command we wouldn't
	// strictly need the identical workaround in the position.v-command but it does make
	// for a nice symmetry.
	if (getFeatures() & GF_CLIPCOORDS)
		clipViewCoordinates(&vt);
}

void AgiEngine::cmd_position_f(uint8 *p) {
	vt.xPos = vt.xPos2 = _v[p1];
	vt.yPos = vt.yPos2 = _v[p2];

	// WORKAROUND: Part of the fix for bug #1659209 "AGI: Space Trek sprite duplication"
	// with an accompanying identical workaround in position-command (i.e. command 0x25).
	// See that workaround's comment for more in-depth information.
	if (getFeatures() & GF_CLIPCOORDS)
		clipViewCoordinates(&vt);
}

void AgiEngine::cmd_get_posn(uint8 *p) {
	_game.vars[p1] = (unsigned char)vt.xPos;
	_game.vars[p2] = (unsigned char)vt.yPos;
}

void AgiEngine::cmd_reposition(uint8 *p) {
	int dx = (int8) _v[p1], dy = (int8) _v[p2];

	debugC(4, kDebugLevelScripts, "dx=%d, dy=%d", dx, dy);
	vt.flags |= UPDATE_POS;

	if (dx < 0 && vt.xPos < -dx)
		vt.xPos = 0;
	else
		vt.xPos += dx;

	if (dy < 0 && vt.yPos < -dy)
		vt.yPos = 0;
	else
		vt.yPos += dy;

	fixPosition(p0);
}

void AgiEngine::cmd_reposition_to(uint8 *p) {
	vt.xPos = p1;
	vt.yPos = p2;
	vt.flags |= UPDATE_POS;
	fixPosition(p0);
}

void AgiEngine::cmd_reposition_to_f(uint8 *p) {
	vt.xPos = _v[p1];
	vt.yPos = _v[p2];
	vt.flags |= UPDATE_POS;
	fixPosition(p0);
}

void AgiEngine::cmd_add_to_pic(uint8 *p) {
	_sprites->addToPic(p0, p1, p2, p3, p4, p5, p6);
}

void AgiEngine::cmd_add_to_pic_f(uint8 *p) {
	_sprites->addToPic(_v[p0], _v[p1], _v[p2], _v[p3], _v[p4], _v[p5], _v[p6]);
}

void AgiEngine::cmd_force_update(uint8 *p) {
	_sprites->eraseBoth();
	_sprites->blitBoth();
	_sprites->commitBoth();
}

void AgiEngine::cmd_reverse_loop(uint8 *p) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = CYCLE_REV_LOOP;
	vt.flags |= (DONTUPDATE | UPDATE | CYCLING);
	vt.parm1 = p1;
	setflag(p1, false);
}

void AgiEngine::cmd_end_of_loop(uint8 *p) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = CYCLE_END_OF_LOOP;
	vt.flags |= (DONTUPDATE | UPDATE | CYCLING);
	vt.parm1 = p1;
	setflag(p1, false);
}

void AgiEngine::cmd_block(uint8 *p) {
	debugC(4, kDebugLevelScripts, "x1=%d, y1=%d, x2=%d, y2=%d", p0, p1, p2, p3);
	_game.block.active = true;
	_game.block.x1 = p0;
	_game.block.y1 = p1;
	_game.block.x2 = p2;
	_game.block.y2 = p3;
}

void AgiEngine::cmd_unblock(uint8 *p) {
	_game.block.active = false;
}

void AgiEngine::cmd_normal_motion(uint8 *p) {
	vt.motion = MOTION_NORMAL;
}

void AgiEngine::cmd_stop_motion(uint8 *p) {
	vt.direction = 0;
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		// ego only
		_v[vEgoDir] = 0;
		_game.playerControl = false;
	}
}

void AgiEngine::cmd_start_motion(uint8 *p) {
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		// ego only
		_v[vEgoDir] = 0;
		_game.playerControl = true;
	}
}

void AgiEngine::cmd_player_control(uint8 *p) {
	_game.playerControl = true;
	_game.viewTable[0].motion = MOTION_NORMAL;
}

void AgiEngine::cmd_program_control(uint8 *p) {
	_game.playerControl = false;
}

void AgiEngine::cmd_follow_ego(uint8 *p) {
	vt.motion = MOTION_FOLLOW_EGO;
	vt.parm1 = p1 > vt.stepSize ? p1 : vt.stepSize;
	vt.parm2 = p2;
	vt.parm3 = 0xff;
	setflag(p2, false);
	vt.flags |= UPDATE;
}

void AgiEngine::cmd_move_obj(uint8 *p) {
	// _D (_D_WARN "o=%d, x=%d, y=%d, s=%d, f=%d", p0, p1, p2, p3, p4);

	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = p1;
	vt.parm2 = p2;
	vt.parm3 = vt.stepSize;
	vt.parm4 = p4;

	if (p3 != 0)
		vt.stepSize = p3;

	setflag(p4, false);
	vt.flags |= UPDATE;

	if (p0 == 0)
		_game.playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call move_obj!
	if (getVersion() > 0x2272)
		moveObj(&vt);
}

void AgiEngine::cmd_move_obj_f(uint8 *p) {
	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = _v[p1];
	vt.parm2 = _v[p2];
	vt.parm3 = vt.stepSize;
	vt.parm4 = p4;

	if (_v[p3] != 0)
		vt.stepSize = _v[p3];

	setflag(p4, false);
	vt.flags |= UPDATE;

	if (p0 == 0)
		_game.playerControl = false;

	// AGI 2.272 (ddp, xmas) doesn't call move_obj!
	if (getVersion() > 0x2272)
		moveObj(&vt);
}

void AgiEngine::cmd_wander(uint8 *p) {
	if (p0 == 0)
		_game.playerControl = false;

	vt.motion = MOTION_WANDER;
	vt.flags |= UPDATE;
}

void AgiEngine::cmd_set_game_id(uint8 *p) {
	if (_curLogic->texts && (p0 - 1) <= _curLogic->numTexts)
		strncpy(_game.id, _curLogic->texts[p0 - 1], 8);
	else
		_game.id[0] = 0;

	debug(0, "Game ID: \"%s\"", _game.id);
}

void AgiEngine::cmd_pause(uint8 *p) {
	int tmp = _game.clockEnabled;
	const char *b[] = { "Continue", NULL };
	const char *b_ru[] = { "\x8f\xe0\xae\xa4\xae\xab\xa6\xa8\xe2\xec", NULL };

	_game.clockEnabled = false;

	switch (getLanguage()) {
	case Common::RU_RUS:
		selectionBox("  \x88\xa3\xe0\xa0 \xae\xe1\xe2\xa0\xad\xae\xa2\xab\xa5\xad\xa0.  \n\n\n", b_ru);
		break;
	default:
		selectionBox("  Game is paused.  \n\n\n", b);
		break;
	}
	_game.clockEnabled = tmp;
}

void AgiEngine::cmd_set_menu(uint8 *p) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, _curLogic->numTexts);

	if (_curLogic->texts != NULL && p0 <= _curLogic->numTexts)
		_menu->add(_curLogic->texts[p0 - 1]);
}

void AgiEngine::cmd_set_menu_item(uint8 *p) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, _curLogic->numTexts);

	if (_curLogic->texts != NULL && p0 <= _curLogic->numTexts)
		_menu->addItem(_curLogic->texts[p0 - 1], p1);
}

void AgiEngine::cmd_version(uint8 *p) {
	char verMsg[64];
	char ver2Msg[] =
	    "\n"
	    "                               \n\n"
	    "  Emulating Sierra AGI v%x.%03x\n";
	char ver3Msg[] =
	    "\n"
	    "                             \n\n"
	    "  Emulating AGI v%x.002.%03x\n";
	// no Sierra as it wraps textbox
	char *r, *q;
	int ver, maj, min;
	char msg[256];
	int gap;
	int len;

	sprintf(verMsg, TITLE " v%s", gScummVMVersion);

	ver = getVersion();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	q = maj == 2 ? ver2Msg : ver3Msg;
	r = strchr(q + 1, '\n');

	// insert our version into the other version
	len = strlen(verMsg);
	gap = r - q;

	if (gap < 0)
		gap = 0;
	else
		gap = (gap - len) / 2;

	strncpy(q + 1 + gap, verMsg, strlen(verMsg));
	sprintf(msg, q, maj, min);
	messageBox(msg);
}

void AgiEngine::cmd_configure_screen(uint8 *p) {
	_game.lineMinPrint = p0;
	_game.lineUserInput = p1;
	_game.lineStatus = p2;
}

void AgiEngine::cmd_text_screen(uint8 *p) {
	debugC(4, kDebugLevelScripts, "switching to text mode");
	_game.gfxMode = false;

	// Simulates the "bright background bit" of the PC video
	// controller.
	if (_game.colorBg)
		_game.colorBg |= 0x08;

	_gfx->clearScreen(_game.colorBg);
}

void AgiEngine::cmd_graphics(uint8 *p) {
	debugC(4, kDebugLevelScripts, "switching to graphics mode");

	if (!_game.gfxMode) {
		_game.gfxMode = true;
		_gfx->clearScreen(0);
		_picture->showPic();
		writeStatus();
		writePrompt();
	}
}

void AgiEngine::cmd_set_text_attribute(uint8 *p) {
	_game.colorFg = p0;
	_game.colorBg = p1;

	if (_game.gfxMode) {
		if (_game.colorBg != 0) {
			_game.colorFg = 0;
			_game.colorBg = 15;
		}
	}
}

void AgiEngine::cmd_status(uint8 *p) {
	inventory();
}

void AgiEngine::cmd_quit(uint8 *p) {
	const char *buttons[] = { "Quit", "Continue", NULL };

	_sound->stopSound();
	if (p0) {
		quitGame();
	} else {
		if (selectionBox(" Quit the game, or continue? \n\n\n", buttons) == 0) {
			quitGame();
		}
	}
}

void AgiEngine::cmd_restart_game(uint8 *p) {
	const char *buttons[] = { "Restart", "Continue", NULL };
	int sel;

	_sound->stopSound();
	sel = getflag(fAutoRestart) ? 0 :
		selectionBox(" Restart _game, or continue? \n\n\n", buttons);

	if (sel == 0) {
		_restartGame = true;
		setflag(fRestartGame, true);
		_menu->enableAll();
	}
}

void AgiEngine::cmd_distance(uint8 *p) {
	int16 x1, y1, x2, y2, d;
	VtEntry *v0 = &_game.viewTable[p0];
	VtEntry *v1 = &_game.viewTable[p1];

	if (v0->flags & DRAWN && v1->flags & DRAWN) {
		x1 = v0->xPos + v0->xSize / 2;
		y1 = v0->yPos;
		x2 = v1->xPos + v1->xSize / 2;
		y2 = v1->yPos;
		d = ABS(x1 - x2) + ABS(y1 - y2);
		if (d > 0xfe)
			d = 0xfe;
	} else {
		d = 0xff;
	}

	// WORKAROUND: Fixes King's Quest IV's script bug #1660424 (KQ4: Zombie bug).
	// In the graveyard (Rooms 16 and 18) at night if you had the Obsidian Scarab (Item 4)
	// and you were very close to a spot where a zombie was going to rise up from the
	// ground you could reproduce the bug. Just standing there and letting the zombie
	// try to rise up the Obsidian Scarab would repel the zombie immediately and that
	// would make the script bug so that the zombie would still come up but it just
	// wouldn't chase Rosella around anymore. If it had worked correctly the zombie
	// wouldn't have come up at all or it would have come up and gone back down
	// immediately. The latter approach is the one implemented here.
	if (getGameID() == GID_KQ4 && (_v[vCurRoom] == 16 || _v[vCurRoom] == 18) && p2 >= 221 && p2 <= 223) {
		// Rooms 16 and 18 are graveyards where three zombies come up at night. They use logics 16 and 18.
		// Variables 221-223 are used to save the distance between each zombie and Rosella.
		// Variables 155, 156 and 162 are used to save the state of each zombie in room 16.
		// Variables 155, 156 and 158 are used to save the state of each zombie in room 18.
		// Rosella gets turned to a zombie only if any of the zombies is under 10 units away
		// from her and she doesn't have the Obsidian Scarab (Item 4). Likewise Rosella makes
		// a zombie go back into the ground if the zombie comes under 15 units away from her
		// and she has the Obsidian Scarab. To ensure a zombie always first rises up before
		// checking for either of the aforementioned conditions (Rosella getting turned to
		// a zombie or the zombie getting turned away by the scarab) we make it appear the
		// zombie is far away from Rosella if the zombie is not already up and chasing her.
		enum zombieStates {ZOMBIE_SET_TO_RISE_UP, ZOMBIE_RISING_UP, ZOMBIE_CHASING_EGO};
		uint8 zombieStateVarNumList[] = {155, 156, (_v[vCurRoom] == 16) ? 162 : 158};
		uint8 zombieNum         = p2 - 221;                         // Zombie's number (In range 0-2)
		uint8 zombieStateVarNum = zombieStateVarNumList[zombieNum]; // Number of the variable containing zombie's state
		uint8 zombieState       = _v[zombieStateVarNum];            // Zombie's state
		// If zombie is not chasing Rosella then set its distance from Rosella to the maximum
		if (zombieState != ZOMBIE_CHASING_EGO)
			d = 0xff;
	}

	_v[p2] = (unsigned char)d;
}

void AgiEngine::cmd_accept_input(uint8 *p) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "input normal");

	newInputMode(INPUT_NORMAL);
	_game.inputEnabled = true;
	writePrompt();
}

void AgiEngine::cmd_prevent_input(uint8 *p) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "no input");

	newInputMode(INPUT_NONE);
	_game.inputEnabled = false;

	clearPrompt();
}

void AgiEngine::cmd_get_string(uint8 *p) {
	int tex, row, col;

	debugC(4, kDebugLevelScripts, "%d %d %d %d %d", p0, p1, p2, p3, p4);

	tex = p1 - 1;
	row = p2;
	col = p3;

	// Workaround for SQLC bug.
	// See Sarien bug #792125 for details
	if (row > 24)
		row = 24;
	if (col > 39)
		col = 39;

	newInputMode(INPUT_GETSTRING);

	if (_curLogic->texts != NULL && _curLogic->numTexts >= tex) {
		int len = strlen(_curLogic->texts[tex]);

		printText(_curLogic->texts[tex], 0, col, row, len, _game.colorFg, _game.colorBg);
		getString(col + len - 1, row, p4, p0);

		// SGEO: display input char
		_gfx->printCharacter((col + len), row, _game.cursorChar, _game.colorFg, _game.colorBg);
	}

	do {
		mainCycle();
	} while (_game.inputMode == INPUT_GETSTRING && !(shouldQuit() || _restartGame));
}

void AgiEngine::cmd_get_num(uint8 *p) {
	debugC(4, kDebugLevelScripts, "%d %d", p0, p1);

	newInputMode(INPUT_GETSTRING);

	if (_curLogic->texts != NULL && _curLogic->numTexts >= (p0 - 1)) {
		int len = strlen(_curLogic->texts[p0 - 1]);

		printText(_curLogic->texts[p0 - 1], 0, 0, 22, len, _game.colorFg, _game.colorBg);
		getString(len - 1, 22, 3, MAX_STRINGS);

		// CM: display input char
		_gfx->printCharacter((p3 + len), 22, _game.cursorChar, _game.colorFg, _game.colorBg);
	}

	do {
		mainCycle();
	} while (_game.inputMode == INPUT_GETSTRING && !(shouldQuit() || _restartGame));

	_v[p1] = atoi(_game.strings[MAX_STRINGS]);

	debugC(4, kDebugLevelScripts, "[%s] -> %d", _game.strings[MAX_STRINGS], _v[p1]);

	clearLines(22, 22, _game.colorBg);
	flushLines(22, 22);
}

void AgiEngine::cmd_set_cursor_char(uint8 *p) {
	if (_curLogic->texts != NULL && (p0 - 1) <= _curLogic->numTexts) {
		_game.cursorChar = *_curLogic->texts[p0 - 1];
	} else {
		// default
		_game.cursorChar = '_';
	}
}

void AgiEngine::cmd_set_key(uint8 *p) {
	int key;

	if (_game.lastController >= MAX_CONTROLLERS) {
		warning("Number of set.keys exceeded %d", MAX_CONTROLLERS);
		return;
	}

	debugC(4, kDebugLevelScripts, "%d %d %d", p0, p1, p2);

	key = 256 * p1 + p0;

	_game.controllers[_game.lastController].keycode = key;
	_game.controllers[_game.lastController].controller = p2;
	_game.lastController++;

	_game.controllerOccured[p2] = false;
}

void AgiEngine::cmd_set_string(uint8 *p) {
	// CM: to avoid crash in Groza (str = 150)
	if (p0 > MAX_STRINGS)
		return;
	strcpy(_game.strings[p0], _curLogic->texts[p1 - 1]);
}

void AgiEngine::cmd_display(uint8 *p) {
	int len = 40;

	char *s = wordWrapString(_curLogic->texts[p2 - 1], &len);

	printText(s, p1, 0, p0, 40, _game.colorFg, _game.colorBg);

	free(s);
}

void AgiEngine::cmd_display_f(uint8 *p) {
	printText(_curLogic->texts[_v[p2] - 1], _v[p1], 0, _v[p0], 40, _game.colorFg, _game.colorBg);
}

void AgiEngine::cmd_clear_text_rect(uint8 *p) {
	int c, x1, y1, x2, y2;

	if ((c = p4) != 0)
		c = 15;

	x1 = p1 * CHAR_COLS;
	y1 = p0 * CHAR_LINES;
	x2 = (p3 + 1) * CHAR_COLS - 1;
	y2 = (p2 + 1) * CHAR_LINES - 1;

	// Added to prevent crash with x2 = 40 in the iigs demo
	if (x1 > GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (x2 > GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y1 > GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (y2 > GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	_gfx->drawRectangle(x1, y1, x2, y2, c);
	_gfx->flushBlock(x1, y1, x2, y2);
}

void AgiEngine::cmd_toggle_monitor(uint8 *p) {
	debug(0, "toggle.monitor");
}

void AgiEngine::cmd_echo_line(uint8 *p) {
	strcpy((char *)_game.inputBuffer, (const char *)_game.echoBuffer);
	_game.cursorPos = strlen((char *)_game.inputBuffer);
	_game.hasPrompt = 0;
}

void AgiEngine::cmd_clear_lines(uint8 *p) {
	uint8 l;

	// Residence 44 calls clear.lines(24,0,0), see Sarien bug #558423
	l = p1 ? p1 : p0;

	// Agent06 incorrectly calls clear.lines(1,150,0), see ScummVM bugs
	// #1935838 and #1935842
	l = (l <= 24) ? l : 24;

	clearLines(p0, l, p2);
	flushLines(p0, l);
}

void AgiEngine::cmd_print(uint8 *p) {
	int n = p0 < 1 ? 1 : p0;

	print(_curLogic->texts[n - 1], 0, 0, 0);
}

void AgiEngine::cmd_print_f(uint8 *p) {
	int n = _v[p0] < 1 ? 1 : _v[p0];

	print(_curLogic->texts[n - 1], 0, 0, 0);
}

void AgiEngine::cmd_print_at(uint8 *p) {
	int n = p0 < 1 ? 1 : p0;

	debugC(4, kDebugLevelScripts, "%d %d %d %d", p0, p1, p2, p3);

	print(_curLogic->texts[n - 1], p1, p2, p3);
}

void AgiEngine::cmd_print_at_v(uint8 *p) {
	int n = _v[p0] < 1 ? 1 : _v[p0];

	print(_curLogic->texts[n - 1], p1, p2, p3);
}

void AgiEngine::cmd_push_script(uint8 *p) {
	// We run AGIMOUSE always as a side effect
	if (getFeatures() & GF_AGIMOUSE || true) {
		_game.vars[27] = _mouse.button;
		_game.vars[28] = _mouse.x / 2;
		_game.vars[29] = _mouse.y;
	} else {
		if (getVersion() >= 0x2915) {
			debug(0, "push.script");
		}
	}
}

void AgiEngine::cmd_set_pri_base(uint8 *p) {
	int i, x, pri;

	debug(0, "Priority base set to %d", p0);

	// _game.alt_pri = true;
	x = (_HEIGHT - p0) * _HEIGHT / 10;

	for (i = 0; i < _HEIGHT; i++) {
		pri = (i - p0) < 0 ? 4 : (i - p0) * _HEIGHT / x + 5;
		if (pri > 15)
			pri = 15;
		_game.priTable[i] = pri;
	}
}

void AgiEngine::cmd_mouse_posn(uint8 *p) {
	_v[p0] = WIN_TO_PIC_X(_mouse.x);
	_v[p1] = WIN_TO_PIC_Y(_mouse.y);
}

void AgiEngine::cmd_shake_screen(uint8 *p) {
	int i;

	// AGIPAL uses shake.screen values between 100 and 109 to set the palette
	// (Checked the original AGIPAL-hack's shake.screen-routine's disassembly).
	if (p0 >= 100 && p0 < 110) {
		if (getFeatures() & GF_AGIPAL) {
			_gfx->setAGIPal(p0);
			return;
		} else {
			warning("It looks like GF_AGIPAL flag is missing");
		}
	}

	// Disables input while shaking to prevent bug
	// #1678230: AGI: Entering text while screen is shaking
	bool originalValue = _game.inputEnabled;
	_game.inputEnabled = false;

	_gfx->shakeStart();

	_sprites->commitBoth();		// Fixes SQ1 demo
	for (i = 4 * p0; i; i--) {
		_gfx->shakeScreen(i & 1);
		_gfx->flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
		mainCycle();
	}
	_gfx->shakeEnd();

	// Sets input back to what it was
	_game.inputEnabled = originalValue;
}

void AgiEngine::setupOpcodes() {
	AgiCommand tmp[] = {
		NULL,			// 0x00
		&AgiEngine::cmd_increment,
		&AgiEngine::cmd_decrement,
		&AgiEngine::cmd_assignn,
		&AgiEngine::cmd_assignv,
		&AgiEngine::cmd_addn,
		&AgiEngine::cmd_addv,
		&AgiEngine::cmd_subn,
		&AgiEngine::cmd_subv,	// 0x08
		&AgiEngine::cmd_lindirectv,
		&AgiEngine::cmd_rindirect,
		&AgiEngine::cmd_lindirectn,
		&AgiEngine::cmd_set,
		&AgiEngine::cmd_reset,
		&AgiEngine::cmd_toggle,
		&AgiEngine::cmd_set_v,
		&AgiEngine::cmd_reset_v,	// 0x10
		&AgiEngine::cmd_toggle_v,
		&AgiEngine::cmd_new_room,
		&AgiEngine::cmd_new_room_f,
		&AgiEngine::cmd_load_logic,
		&AgiEngine::cmd_load_logic_f,
		&AgiEngine::cmd_call,
		&AgiEngine::cmd_call_f,
		&AgiEngine::cmd_load_pic,	// 0x18
		&AgiEngine::cmd_draw_pic,
		&AgiEngine::cmd_show_pic,
		&AgiEngine::cmd_discard_pic,
		&AgiEngine::cmd_overlay_pic,
		&AgiEngine::cmd_show_pri_screen,
		&AgiEngine::cmd_load_view,
		&AgiEngine::cmd_load_view_f,
		&AgiEngine::cmd_discard_view,	// 0x20
		&AgiEngine::cmd_animate_obj,
		&AgiEngine::cmd_unanimate_all,
		&AgiEngine::cmd_draw,
		&AgiEngine::cmd_erase,
		&AgiEngine::cmd_position,
		&AgiEngine::cmd_position_f,
		&AgiEngine::cmd_get_posn,
		&AgiEngine::cmd_reposition,	// 0x28
		&AgiEngine::cmd_set_view,
		&AgiEngine::cmd_set_view_f,
		&AgiEngine::cmd_set_loop,
		&AgiEngine::cmd_set_loop_f,
		&AgiEngine::cmd_fix_loop,
		&AgiEngine::cmd_release_loop,
		&AgiEngine::cmd_set_cel,
		&AgiEngine::cmd_set_cel_f,	// 0x30
		&AgiEngine::cmd_last_cel,
		&AgiEngine::cmd_current_cel,
		&AgiEngine::cmd_current_loop,
		&AgiEngine::cmd_current_view,
		&AgiEngine::cmd_number_of_loops,
		&AgiEngine::cmd_set_priority,
		&AgiEngine::cmd_set_priority_f,
		&AgiEngine::cmd_release_priority,	// 0x38
		&AgiEngine::cmd_get_priority,
		&AgiEngine::cmd_stop_update,
		&AgiEngine::cmd_start_update,
		&AgiEngine::cmd_force_update,
		&AgiEngine::cmd_ignore_horizon,
		&AgiEngine::cmd_observe_horizon,
		&AgiEngine::cmd_set_horizon,
		&AgiEngine::cmd_object_on_water,	// 0x40
		&AgiEngine::cmd_object_on_land,
		&AgiEngine::cmd_object_on_anything,
		&AgiEngine::cmd_ignore_objs,
		&AgiEngine::cmd_observe_objs,
		&AgiEngine::cmd_distance,
		&AgiEngine::cmd_stop_cycling,
		&AgiEngine::cmd_start_cycling,
		&AgiEngine::cmd_normal_cycle,	// 0x48
		&AgiEngine::cmd_end_of_loop,
		&AgiEngine::cmd_reverse_cycle,
		&AgiEngine::cmd_reverse_loop,
		&AgiEngine::cmd_cycle_time,
		&AgiEngine::cmd_stop_motion,
		&AgiEngine::cmd_start_motion,
		&AgiEngine::cmd_step_size,
		&AgiEngine::cmd_step_time,	// 0x50
		&AgiEngine::cmd_move_obj,
		&AgiEngine::cmd_move_obj_f,
		&AgiEngine::cmd_follow_ego,
		&AgiEngine::cmd_wander,
		&AgiEngine::cmd_normal_motion,
		&AgiEngine::cmd_set_dir,
		&AgiEngine::cmd_get_dir,
		&AgiEngine::cmd_ignore_blocks,	// 0x58
		&AgiEngine::cmd_observe_blocks,
		&AgiEngine::cmd_block,
		&AgiEngine::cmd_unblock,
		&AgiEngine::cmd_get,
		&AgiEngine::cmd_get_f,
		&AgiEngine::cmd_drop,
		&AgiEngine::cmd_put,
		&AgiEngine::cmd_put_f,	// 0x60
		&AgiEngine::cmd_get_room_f,
		&AgiEngine::cmd_load_sound,
		&AgiEngine::cmd_sound,
		&AgiEngine::cmd_stop_sound,
		&AgiEngine::cmd_print,
		&AgiEngine::cmd_print_f,
		&AgiEngine::cmd_display,
		&AgiEngine::cmd_display_f,	// 0x68
		&AgiEngine::cmd_clear_lines,
		&AgiEngine::cmd_text_screen,
		&AgiEngine::cmd_graphics,
		&AgiEngine::cmd_set_cursor_char,
		&AgiEngine::cmd_set_text_attribute,
		&AgiEngine::cmd_shake_screen,
		&AgiEngine::cmd_configure_screen,
		&AgiEngine::cmd_status_line_on,	// 0x70
		&AgiEngine::cmd_status_line_off,
		&AgiEngine::cmd_set_string,
		&AgiEngine::cmd_get_string,
		&AgiEngine::cmd_word_to_string,
		&AgiEngine::cmd_parse,
		&AgiEngine::cmd_get_num,
		&AgiEngine::cmd_prevent_input,
		&AgiEngine::cmd_accept_input,	// 0x78
		&AgiEngine::cmd_set_key,
		&AgiEngine::cmd_add_to_pic,
		&AgiEngine::cmd_add_to_pic_f,
		&AgiEngine::cmd_status,
		&AgiEngine::cmd_save_game,
		&AgiEngine::cmd_load_game,
		&AgiEngine::cmd_init_disk,
		&AgiEngine::cmd_restart_game,	// 0x80
		&AgiEngine::cmd_show_obj,
		&AgiEngine::cmd_random,
		&AgiEngine::cmd_program_control,
		&AgiEngine::cmd_player_control,
		&AgiEngine::cmd_obj_status_f,
		&AgiEngine::cmd_quit,
		&AgiEngine::cmd_show_mem,
		&AgiEngine::cmd_pause,	// 0x88
		&AgiEngine::cmd_echo_line,
		&AgiEngine::cmd_cancel_line,
		&AgiEngine::cmd_init_joy,
		&AgiEngine::cmd_toggle_monitor,
		&AgiEngine::cmd_version,
		&AgiEngine::cmd_script_size,
		&AgiEngine::cmd_set_game_id,
		&AgiEngine::cmd_log,	// 0x90
		&AgiEngine::cmd_set_scan_start,
		&AgiEngine::cmd_reset_scan_start,
		&AgiEngine::cmd_reposition_to,
		&AgiEngine::cmd_reposition_to_f,
		&AgiEngine::cmd_trace_on,
		&AgiEngine::cmd_trace_info,
		&AgiEngine::cmd_print_at,
		&AgiEngine::cmd_print_at_v,	// 0x98
		&AgiEngine::cmd_discard_view,	// Opcode repeated from 0x20 ?
		&AgiEngine::cmd_clear_text_rect,
		&AgiEngine::cmd_set_upper_left,
		&AgiEngine::cmd_set_menu,
		&AgiEngine::cmd_set_menu_item,
		&AgiEngine::cmd_submit_menu,
		&AgiEngine::cmd_enable_item,
		&AgiEngine::cmd_disable_item,	// 0xa0
		&AgiEngine::cmd_menu_input,
		&AgiEngine::cmd_show_obj_v,
		&AgiEngine::cmd_open_dialogue,
		&AgiEngine::cmd_close_dialogue,
		&AgiEngine::cmd_mul_n,
		&AgiEngine::cmd_mul_v,
		&AgiEngine::cmd_div_n,
		&AgiEngine::cmd_div_v,	// 0xa8
		&AgiEngine::cmd_close_window,
		&AgiEngine::cmd_set_simple,
		&AgiEngine::cmd_push_script,
		&AgiEngine::cmd_pop_script,
		&AgiEngine::cmd_hold_key,
		&AgiEngine::cmd_set_pri_base,
		&AgiEngine::cmd_discard_sound,
		&AgiEngine::cmd_hide_mouse,	// 0xb0
		&AgiEngine::cmd_allow_menu,
		&AgiEngine::cmd_show_mouse,
		&AgiEngine::cmd_fence_mouse,
		&AgiEngine::cmd_mouse_posn,
		&AgiEngine::cmd_release_key,
		&AgiEngine::cmd_adj_ego_move_to_x_y
	};
	assert(ARRAYSIZE(_agiCommands) == ARRAYSIZE(tmp));
	for (int i = 0; i < ARRAYSIZE(tmp); ++i)
		_agiCommands[i] = tmp[i];
}

/**
 * Execute a logic script
 * @param n  Number of the logic resource to execute
 */
int AgiEngine::runLogic(int n) {
	uint8 op = 0;
	uint8 p[CMD_BSIZE] = { 0 };
	uint8 *code = NULL;
	int num = 0;
	ScriptPos sp;

	debugC(2, kDebugLevelScripts, "=================");
	debugC(2, kDebugLevelScripts, "runLogic(%d)", n);

	sp.script = n;
	sp.curIP = 0;
	_game.execStack.push_back(sp);

	// If logic not loaded, load it
	if (~_game.dirLogic[n].flags & RES_LOADED) {
		debugC(4, kDebugLevelScripts, "logic %d not loaded!", n);
		agiLoadResource(rLOGIC, n);
	}

	_game.lognum = n;
	_curLogic = &_game.logics[_game.lognum];

	code = _curLogic->data;
	_curLogic->cIP = _curLogic->sIP;

	_timerHack = 0;
	while (ip < _game.logics[n].size && !(shouldQuit() || _restartGame)) {
		if (_debug.enabled) {
			if (_debug.steps > 0) {
				if (_debug.logic0 || n) {
					debugConsole(n, lCOMMAND_MODE, NULL);
					_debug.steps--;
				}
			} else {
				_sprites->blitBoth();
				_sprites->commitBoth();
				do {
					mainCycle();
				} while (!_debug.steps && _debug.enabled);
				_sprites->eraseBoth();
			}
		}

		_game.execStack.back().curIP = ip;

		char st[101];
		int sz = MIN(_game.execStack.size(), 100u);
		memset(st, '.', sz);
		st[sz] = 0;

		switch (op = *(code + ip++)) {
		case 0xff:	// if (open/close)
			testIfCode(n);
			break;
		case 0xfe:	// goto
			// +2 covers goto size
			ip += 2 + ((int16)READ_LE_UINT16(code + ip));

			// timer must keep running even in goto loops,
			// but AGI engine can't do that :(
			if (_timerHack > 20) {
				pollTimer();
				updateTimer();
				_timerHack = 0;
			}
			break;
		case 0x00:	// return
			debugC(2, kDebugLevelScripts, "%sreturn() // Logic %d", st, n);
			debugC(2, kDebugLevelScripts, "=================");

			_game.execStack.pop_back();
			return 1;
		default:
			num = logicNamesCmd[op].numArgs;
			memmove(p, code + ip, num);
			memset(p + num, 0, CMD_BSIZE - num);

			debugC(2, kDebugLevelScripts, "%s%s(%d %d %d)", st, logicNamesCmd[op].name, p[0], p[1], p[2]);

			(this->*_agiCommands[op])(p);
			ip += num;
		}

		if (_game.exitAllLogics)
			break;
	}

	_game.execStack.pop_back();

	return 0;		// after executing new.room()
}

void AgiEngine::executeAgiCommand(uint8 op, uint8 *p) {
	debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logicNamesCmd[op].name, p[0], p[1], p[2]);

	(this->*_agiCommands[op])(p);
}

} // End of namespace Agi
