/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "base/version.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/opcodes.h"
#include "agi/menu.h"

namespace Agi {

#define p0	(p[0])
#define p1	(p[1])
#define p2	(p[2])
#define p3	(p[3])
#define p4	(p[4])
#define p5	(p[5])
#define p6	(p[6])

#define game g_agi->_game
#define g_sprites g_agi->_sprites
#define g_sound g_agi->_sound
#define g_text g_agi->_text
#define g_gfx g_agi->_gfx
#define g_picture g_agi->_picture

#define ip	curLogic->cIP
#define vt	game.viewTable[p0]

static struct AgiLogic *curLogic;
static AgiEngine *g_agi;

int timerHack;			/* Workaround for timer loop in MH1 */

#define _v game.vars
#define cmd(x) static void cmd_##x (uint8 *p)

cmd(increment) {
	if (_v[p0] != 0xff)
		++_v[p0];
}

cmd(decrement) {
	if (_v[p0] != 0)
		--_v[p0];
}

cmd(assignn) {
	_v[p0] = p1;
}

cmd(addn) {
	_v[p0] += p1;
}

cmd(subn) {
	_v[p0] -= p1;
}

cmd(assignv) {
	_v[p0] = _v[p1];
}

cmd(addv) {
	_v[p0] += _v[p1];
}

cmd(subv) {
	_v[p0] -= _v[p1];
}

cmd(mul_n) {
	_v[p0] *= p1;
}

cmd(mul_v) {
	_v[p0] *= _v[p1];
}

cmd(div_n) {
	_v[p0] /= p1;
}

cmd(div_v) {
	_v[p0] /= _v[p1];
}

cmd(random) {
	_v[p2] = g_agi->_rnd->getRandomNumber(p1 - p0) + p0;
}

cmd(lindirectn) {
	_v[_v[p0]] = p1;
}

cmd(lindirectv) {
	_v[_v[p0]] = _v[p1];
}

cmd(rindirect) {
	_v[p0] = _v[_v[p1]];
}

cmd(set) {
	g_agi->setflag(*p, true);
}

cmd(reset) {
	g_agi->setflag(*p, false);
}

cmd(toggle) {
	g_agi->setflag(*p, !g_agi->getflag(*p));
}

cmd(set_v) {
	g_agi->setflag(_v[p0], true);
}

cmd(reset_v) {
	g_agi->setflag(_v[p0], false);
}

cmd(toggle_v) {
	g_agi->setflag(_v[p0], !g_agi->getflag(_v[p0]));
}

cmd(new_room) {
	g_agi->newRoom(p0);
}

cmd(new_room_f) {
	g_agi->newRoom(_v[p0]);
}

cmd(load_view) {
	g_agi->agiLoadResource(rVIEW, p0);
}

cmd(load_logic) {
	g_agi->agiLoadResource(rLOGIC, p0);
}

cmd(load_sound) {
	g_agi->agiLoadResource(rSOUND, p0);
}

cmd(load_view_f) {
	g_agi->agiLoadResource(rVIEW, _v[p0]);
}

cmd(load_logic_f) {
	g_agi->agiLoadResource(rLOGIC, _v[p0]);
}

cmd(discard_view) {
	g_agi->agiUnloadResource(rVIEW, p0);
}

cmd(object_on_anything) {
	vt.flags &= ~(ON_WATER | ON_LAND);
}

cmd(object_on_land) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= ON_LAND;
}

cmd(object_on_water) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= ON_WATER;
}

cmd(observe_horizon) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags &= ~IGNORE_HORIZON;
}

cmd(ignore_horizon) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= IGNORE_HORIZON;
}

cmd(observe_objs) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags &= ~IGNORE_OBJECTS;
}

cmd(ignore_objs) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= IGNORE_OBJECTS;
}

cmd(observe_blocks) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags &= ~IGNORE_BLOCKS;
}

cmd(ignore_blocks) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	vt.flags |= IGNORE_BLOCKS;
}

cmd(set_horizon) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	game.horizon = p0;
}

cmd(get_priority) {
	_v[p1] = vt.priority;
}

cmd(set_priority) {
	vt.flags |= FIXED_PRIORITY;
	vt.priority = p1;

	// WORKAROUND: this fixes bug #1712585 in KQ4 (dwarf sprite priority)
	// For this scene, ego (Rosella) hasn't got a fixed priority till script 54
	// explicitly sets priority 8 for her, so that she can walk back to the table
	// without being drawn over the other dwarfs
	// It seems that in this scene, ego's priority is set to 8, but the priority of
	// the last dwarf with the soup bowls (view 152) is also set to 8, which causes
	// the dwarf to be drawn behind ego
	// With this workaround, when the game scripts set the priority of view 152
	// (seventh dwarf with soup bowls), ego's priority is set to 7
	// The game script itself sets priotity 8 for ego before she starts walking, 
	// and then releases the fixed priority set on ego after ego is seated
	// Therefore, this workaround only affects that specific part of this scene
	// Ego is set to object 19 by script 54
	if (g_agi->getFeatures() & GF_KQ4 && vt.currentView == 152) {
		game.viewTable[19].flags |= FIXED_PRIORITY;
		game.viewTable[19].priority = 7;
	}
}

cmd(set_priority_f) {
	vt.flags |= FIXED_PRIORITY;
	vt.priority = _v[p1];
}

cmd(release_priority) {
	vt.flags &= ~FIXED_PRIORITY;
}

cmd(set_upper_left) {				/* do nothing (AGI 2.917) */
}

cmd(start_update) {
	g_agi->startUpdate(&vt);
}

cmd(stop_update) {
	g_agi->stopUpdate(&vt);
}

cmd(current_view) {
	_v[p1] = vt.currentView;
}

cmd(current_cel) {
	_v[p1] = vt.currentCel;
	debugC(4, kDebugLevelScripts, "v%d=%d", p1, _v[p1]);
}

cmd(current_loop) {
	_v[p1] = vt.currentLoop;
}

cmd(last_cel) {
	_v[p1] = vt.loopData->numCels - 1;
}

cmd(set_cel) {
	g_agi->setCel(&vt, p1);
	vt.flags &= ~DONTUPDATE;
}

cmd(set_cel_f) {
	g_agi->setCel(&vt, _v[p1]);
	vt.flags &= ~DONTUPDATE;
}

cmd(set_view) {
	debugC(4, kDebugLevelScripts, "o%d, %d", p0, p1);
	g_agi->setView(&vt, p1);
}

cmd(set_view_f) {
	g_agi->setView(&vt, _v[p1]);
}

cmd(set_loop) {
	g_agi->setLoop(&vt, p1);
}

cmd(set_loop_f) {
	g_agi->setLoop(&vt, _v[p1]);
}

cmd(number_of_loops) {
	_v[p1] = vt.numLoops;
}

cmd(fix_loop) {
	vt.flags |= FIX_LOOP;
}

cmd(release_loop) {
	vt.flags &= ~FIX_LOOP;
}

cmd(step_size) {
	vt.stepSize = _v[p1];
}

cmd(step_time) {
	vt.stepTime = vt.stepTimeCount = _v[p1];
}

cmd(cycle_time) {
	vt.cycleTime = vt.cycleTimeCount = _v[p1];
}

cmd(stop_cycling) {
	vt.flags &= ~CYCLING;
}

cmd(start_cycling) {
	vt.flags |= CYCLING;
}

cmd(normal_cycle) {
	vt.cycle = CYCLE_NORMAL;
	vt.flags |= CYCLING;
}

cmd(reverse_cycle) {
	vt.cycle = CYCLE_REVERSE;
	vt.flags |= CYCLING;
}

cmd(set_dir) {
	vt.direction = _v[p1];
}

cmd(get_dir) {
	_v[p1] = vt.direction;
}

cmd(get_room_f) {
	_v[p1] = g_agi->objectGetLocation(_v[p0]);
}

cmd(put) {
	g_agi->objectSetLocation(p0, _v[p1]);
}

cmd(put_f) {
	g_agi->objectSetLocation(_v[p0], _v[p1]);
}

cmd(drop) {
	g_agi->objectSetLocation(p0, 0);
}

cmd(get) {
	g_agi->objectSetLocation(p0, EGO_OWNED);
}

cmd(get_f) {
	g_agi->objectSetLocation(_v[p0], EGO_OWNED);
}

cmd(word_to_string) {
	strcpy(game.strings[p0], game.egoWords[p1].word);
}

cmd(open_dialogue) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	game.hasWindow = true;
}

cmd(close_dialogue) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	game.hasWindow = false;
}

cmd(close_window) {
	g_agi->closeWindow();
}

cmd(status_line_on) {
	game.statusLine = true;
	g_agi->writeStatus();
}

cmd(status_line_off) {
	game.statusLine = false;
	g_agi->writeStatus();
}

cmd(show_obj) {
	g_sprites->showObj(p0);
}

cmd(show_obj_v) {
	g_sprites->showObj(_v[p0]);
}

cmd(sound) {
	g_sound->startSound(p0, p1);
}

cmd(stop_sound) {
	g_sound->stopSound();
}

cmd(menu_input) {
	g_agi->newInputMode(INPUT_MENU);
}

cmd(enable_item) {
	g_agi->_menu->setItem(p0, true);
}

cmd(disable_item) {
	g_agi->_menu->setItem(p0, false);
}

cmd(submit_menu) {
	g_agi->_menu->submit();
}

cmd(set_scan_start) {
	curLogic->sIP = curLogic->cIP;
}

cmd(reset_scan_start) {
	curLogic->sIP = 2;
}

cmd(save_game) {
	game.simpleSave ? g_agi->saveGameSimple() : g_agi->saveGameDialog();
}

cmd(load_game) {
	assert(1);
	game.simpleSave ? g_agi->loadGameSimple() : g_agi->loadGameDialog();
}

cmd(init_disk) {				/* do nothing */
}

cmd(log) {				/* do nothing */
}

cmd(trace_on) {				/* do nothing */
}

cmd(trace_info) {				/* do nothing */
}

cmd(show_mem) {
	g_agi->messageBox("Enough memory");
}

cmd(init_joy) { /* do nothing */ ;
}

cmd(script_size) {
	report("script.size(%d)\n", p0);
}

cmd(cancel_line) {
	report("cancel.line\n");
}

cmd(obj_status_f) {
	report("obj.status.f\n");
}

/* unknown commands:
 * unk_170: Force savegame name -- j5
 * unk_171: script save -- j5
 * unk_172: script restore -- j5
 * unk_173: Activate keypressed control (ego only moves while key is pressed)
 * unk_174: Change priority table (used in KQ4) -- j5
 * unk_177: Disable menus completely -- j5
 * unk_181: Deactivate keypressed control (default control of ego)
 */
cmd(set_simple) {
	game.simpleSave = true;
}

cmd(pop_script) {
	report("pop.script\n");
}

cmd(hold_key) {
	report("hold.key\n");
}

cmd(discard_sound) {
	report("discard.sound\n");
}

cmd(hide_mouse) {
	report("hide.mouse\n");
}

cmd(allow_menu) {
	report("allow.menu\n");
}

cmd(show_mouse) {
	report("show.mouse\n");
}

cmd(fence_mouse) {
	report("fence.mouse\n");
}

cmd(release_key) {
	report("release.key\n");
}

cmd(adj_ego_move_to_x_y) {
	game.viewTable[0].flags |= ADJ_EGO_XY;
}

cmd(parse) {
	_v[vWordNotFound] = 0;
	g_agi->setflag(fEnteredCli, false);
	g_agi->setflag(fSaidAcceptedInput, false);

	g_agi->dictionaryWords(g_agi->agiSprintf(game.strings[p0]));
}

cmd(call) {
	int oldCIP;
	int oldLognum;

	/* CM: we don't save sIP because set.scan.start can be
	 *     used in a called script (fixes xmas demo)
	 */
	oldCIP = curLogic->cIP;
	oldLognum = game.lognum;

	g_agi->runLogic(p0);

	game.lognum = oldLognum;
	curLogic = &game.logics[game.lognum];
	curLogic->cIP = oldCIP;
}

cmd(call_f) {
	cmd_call(&_v[p0]);
}

cmd(draw_pic) {
	debugC(6, kDebugLevelScripts, "=== draw pic %d ===", _v[p0]);
	g_sprites->eraseBoth();
	g_picture->decodePicture(_v[p0], true);
	g_sprites->blitBoth();
	game.pictureShown = 0;
	debugC(6, kDebugLevelScripts, "--- end of draw pic %d ---", _v[p0]);
}

cmd(show_pic) {
	debugC(6, kDebugLevelScripts, "=== show pic ===");
	g_agi->setflag(fOutputMode, false);
	cmd_close_window(NULL);
	g_picture->showPic();
	game.pictureShown = 1;
	debugC(6, kDebugLevelScripts, "--- end of show pic ---");
}

cmd(load_pic) {
	g_sprites->eraseBoth();
	g_agi->agiLoadResource(rPICTURE, _v[p0]);
	g_sprites->blitBoth();
}

cmd(discard_pic) {
	debugC(6, kDebugLevelScripts, "--- discard pic ---");
	/* do nothing */
}

cmd(overlay_pic) {
	debugC(6, kDebugLevelScripts, "--- overlay pic ---");
	g_sprites->eraseBoth();
	g_picture->decodePicture(_v[p0], false);
	g_sprites->blitBoth();
	game.pictureShown = 0;
	g_sprites->commitBoth();
}

cmd(show_pri_screen) {
	g_agi->_debug.priority = 1;
	g_sprites->eraseBoth();
	g_picture->showPic();
	g_sprites->blitBoth();
	g_agi->waitKey();
	g_agi->_debug.priority = 0;
	g_sprites->eraseBoth();
	g_picture->showPic();
	g_sprites->blitBoth();
}

cmd(animate_obj) {
	if (vt.flags & ANIMATED)
		return;

	debugC(4, kDebugLevelScripts, "animate vt entry #%d", p0);
	vt.flags = ANIMATED | UPDATE | CYCLING;
	vt.motion = MOTION_NORMAL;
	vt.cycle = CYCLE_NORMAL;
	vt.direction = 0;
}

cmd(unanimate_all) {
	int i;
	for (i = 0; i < MAX_VIEWTABLE; i++)
		game.viewTable[i].flags &= ~(ANIMATED | DRAWN);
}

cmd(draw) {
	if (vt.flags & DRAWN)
		return;

	if (vt.ySize <= 0 || vt.xSize <= 0)
		return;

	debugC(4, kDebugLevelScripts, "draw entry %d", vt.entry);

	vt.flags |= UPDATE;
	if (g_agi->agiGetRelease() >= 0x3000) {
		g_agi->setLoop(&vt, vt.currentLoop);
		g_agi->setCel(&vt, vt.currentCel);
	}
	g_agi->fixPosition(p0);
	vt.xPos2 = vt.xPos;
	vt.yPos2 = vt.yPos;
	vt.celData2 = vt.celData;
	g_sprites->eraseUpdSprites();
	vt.flags |= DRAWN;

	if (g_agi->agiGetRelease() <= 0x2440)	/* See bug #546562 */
		vt.flags |= ANIMATED;

	g_sprites->blitUpdSprites();
	vt.flags &= ~DONTUPDATE;

	g_sprites->commitBlock(vt.xPos, vt.yPos - vt.ySize + 1, vt.xPos + vt.xSize - 1, vt.yPos);

	debugC(4, kDebugLevelScripts, "vt entry #%d flags = %02x", p0, vt.flags);
}

cmd(erase) {
	if (~vt.flags & DRAWN)
		return;

	g_sprites->eraseUpdSprites();
	if (vt.flags & UPDATE) {
		vt.flags &= ~DRAWN;
	} else {
		g_sprites->eraseNonupdSprites();
		vt.flags &= ~DRAWN;
		g_sprites->blitNonupdSprites();
	}
	g_sprites->blitUpdSprites();

	g_sprites->commitBlock(vt.xPos, vt.yPos - vt.ySize + 1, vt.xPos + vt.xSize - 1, vt.yPos);
}

cmd(position) {
	vt.xPos = vt.xPos2 = p1;
	vt.yPos = vt.yPos2 = p2;
}

cmd(position_f) {
	vt.xPos = vt.xPos2 = _v[p1];
	vt.yPos = vt.yPos2 = _v[p2];
}

cmd(get_posn) {
	game.vars[p1] = (unsigned char)vt.xPos;
	game.vars[p2] = (unsigned char)vt.yPos;
}

cmd(reposition) {
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

	g_agi->fixPosition(p0);
}

cmd(reposition_to) {
	vt.xPos = p1;
	vt.yPos = p2;
	vt.flags |= UPDATE_POS;
	g_agi->fixPosition(p0);
}

cmd(reposition_to_f) {
	vt.xPos = _v[p1];
	vt.yPos = _v[p2];
	vt.flags |= UPDATE_POS;
	g_agi->fixPosition(p0);
}

cmd(add_to_pic) {
	g_sprites->addToPic(p0, p1, p2, p3, p4, p5, p6);
}

cmd(add_to_pic_f) {
	g_sprites->addToPic(_v[p0], _v[p1], _v[p2], _v[p3], _v[p4], _v[p5], _v[p6]);
}

cmd(force_update) {
	g_sprites->eraseBoth();
	g_sprites->blitBoth();
	g_sprites->commitBoth();
}

cmd(reverse_loop) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = CYCLE_REV_LOOP;
	vt.flags |= (DONTUPDATE | UPDATE | CYCLING);
	vt.parm1 = p1;
	g_agi->setflag(p1, false);
}

cmd(end_of_loop) {
	debugC(4, kDebugLevelScripts, "o%d, f%d", p0, p1);
	vt.cycle = CYCLE_END_OF_LOOP;
	vt.flags |= (DONTUPDATE | UPDATE | CYCLING);
	vt.parm1 = p1;
	g_agi->setflag(p1, false);
}

cmd(block) {
	debugC(4, kDebugLevelScripts, "x1=%d, y1=%d, x2=%d, y2=%d", p0, p1, p2, p3);
	game.block.active = true;
	game.block.x1 = p0;
	game.block.y1 = p1;
	game.block.x2 = p2;
	game.block.y2 = p3;
}

cmd(unblock) {
	game.block.active = false;
}

cmd(normal_motion) {
	vt.motion = MOTION_NORMAL;
}

cmd(stop_motion) {
	vt.direction = 0;
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		/* ego only */
		_v[vEgoDir] = 0;
		game.playerControl = false;
	}
}

cmd(start_motion) {
	vt.motion = MOTION_NORMAL;
	if (p0 == 0) {		/* ego only */
		_v[vEgoDir] = 0;
		game.playerControl = true;
	}
}

cmd(player_control) {
	game.playerControl = true;
	game.viewTable[0].motion = MOTION_NORMAL;
}

cmd(program_control) {
	game.playerControl = false;
}

cmd(follow_ego) {
	vt.motion = MOTION_FOLLOW_EGO;
	vt.parm1 = p1 > vt.stepSize ? p1 : vt.stepSize;
	vt.parm2 = p2;
	vt.parm3 = 0xff;
	g_agi->setflag(p2, false);
	vt.flags |= UPDATE;
}

cmd(move_obj) {
	/* _D (_D_WARN "o=%d, x=%d, y=%d, s=%d, f=%d", p0, p1, p2, p3, p4); */

	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = p1;
	vt.parm2 = p2;
	vt.parm3 = vt.stepSize;
	vt.parm4 = p4;

	if (p3 != 0)
		vt.stepSize = p3;

	g_agi->setflag(p4, false);
	vt.flags |= UPDATE;

	if (p0 == 0)
		game.playerControl = false;

	/* AGI 2.272 (ddp, xmas) doesn't call move_obj! */
	if (g_agi->agiGetRelease() > 0x2272)
		g_agi->moveObj(&vt);
}

cmd(move_obj_f) {
	vt.motion = MOTION_MOVE_OBJ;
	vt.parm1 = _v[p1];
	vt.parm2 = _v[p2];
	vt.parm3 = vt.stepSize;
	vt.parm4 = p4;

	if (_v[p3] != 0)
		vt.stepSize = _v[p3];

	g_agi->setflag(p4, false);
	vt.flags |= UPDATE;

	if (p0 == 0)
		game.playerControl = false;

	/* AGI 2.272 (ddp, xmas) doesn't call move_obj! */
	if (g_agi->agiGetRelease() > 0x2272)
		g_agi->moveObj(&vt);
}

cmd(wander) {
	if (p0 == 0)
		game.playerControl = false;
	vt.motion = MOTION_WANDER;
	vt.flags |= UPDATE;
}

cmd(set_game_id) {
	if (curLogic->texts && (p0 - 1) <= curLogic->numTexts)
		strncpy(game.id, curLogic->texts[p0 - 1], 8);
	else
		game.id[0] = 0;

	report("Game ID: \"%s\"\n", game.id);
}

cmd(pause) {
	int tmp = game.clockEnabled;
	const char *b[] = { "Continue", NULL };

	game.clockEnabled = false;
	g_agi->selectionBox("  Game is paused.  \n\n\n", b);
	game.clockEnabled = tmp;
}

cmd(set_menu) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, curLogic->numTexts);
	if (curLogic->texts != NULL && p0 < curLogic->numTexts)
		g_agi->_menu->add(curLogic->texts[p0 - 1]);
}

cmd(set_menu_item) {
	debugC(4, kDebugLevelScripts, "text %02x of %02x", p0, curLogic->numTexts);
	if (curLogic->texts != NULL && p0 <= curLogic->numTexts)
		g_agi->_menu->addItem(curLogic->texts[p0 - 1], p1);
}

cmd(version) {
	char verMsg[64];
	char ver2Msg[] =
	    "\n"
	    "                               \n\n"
	    "  Emulating Sierra AGI v%x.%03x\n";
	char ver3Msg[] =
	    "\n"
	    "                             \n\n"
	    "  Emulating AGI v%x.002.%03x\n";
	/* no Sierra as it wraps textbox */
	char *r, *q;
	int ver, maj, min;
	char msg[256];
	int gap;
	int len;

	sprintf(verMsg, TITLE " v%s", gScummVMVersion);

	ver = g_agi->agiGetRelease();
	maj = (ver >> 12) & 0xf;
	min = ver & 0xfff;

	q = maj == 2 ? ver2Msg : ver3Msg;
	r = strchr(q + 1, '\n');

	/* insert our version into the other version */
	len = strlen(verMsg);
	gap = r - q;
	if(gap < 0)
		gap = 0;
	else
		gap = (gap - len) / 2;

	strncpy(q + 1 + gap, verMsg, strlen(verMsg));
	sprintf(msg, q, maj, min);
	g_agi->messageBox(msg);
}

cmd(configure_screen) {
	game.lineMinPrint = p0;
	game.lineUserInput = p1;
	game.lineStatus = p2;
}

cmd(text_screen) {
	debugC(4, kDebugLevelScripts, "switching to text mode");
	game.gfxMode = false;
	/*
	 * Simulates the "bright background bit" of the PC video
	 * controller.
	 */
	if (game.colorBg)
		game.colorBg |= 0x08;
	g_gfx->clearScreen(game.colorBg);
}

cmd(graphics) {
	debugC(4, kDebugLevelScripts, "switching to graphics mode");
	if (!game.gfxMode) {
		game.gfxMode = true;
		g_gfx->clearScreen(0);
		g_picture->showPic();
		g_agi->writeStatus();
		g_agi->writePrompt();
	}
}

cmd(set_text_attribute) {
	game.colorFg = p0;
	game.colorBg = p1;

	if (game.gfxMode) {
		if (game.colorBg != 0) {
			game.colorFg = 0;
			game.colorBg = 15;
		}
	}
}

cmd(status) {
	g_agi->inventory();
}

cmd(quit) {
	const char *buttons[] = { "Quit", "Continue", NULL };

	g_sound->stopSound();
	if (p0) {
		game.quitProgNow = true;
	} else {
		if (g_agi->selectionBox
				(" Quit the game, or continue? \n\n\n", buttons) == 0) {
			game.quitProgNow = true;
		}
	}
}

cmd(restart_game) {
	const char *buttons[] = { "Restart", "Continue", NULL };
	int sel;

	g_sound->stopSound();
	sel = g_agi->getflag(fAutoRestart) ? 1 :
		g_agi->selectionBox(" Restart game, or continue? \n\n\n", buttons);

	if (sel == 0) {
		game.quitProgNow = 0xff;
		g_agi->setflag(fRestartGame, true);
		g_agi->_menu->enableAll();
	}
}

cmd(distance) {
	int16 x1, y1, x2, y2, d;
	VtEntry *v0 = &game.viewTable[p0];
	VtEntry *v1 = &game.viewTable[p1];

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
	_v[p2] = (unsigned char)d;
}

cmd(accept_input) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "input normal");
	g_agi->newInputMode(INPUT_NORMAL);
	game.inputEnabled = true;
}

cmd(prevent_input) {
	debugC(4, kDebugLevelScripts | kDebugLevelInput, "no input");
	g_agi->newInputMode(INPUT_NONE);
	game.inputEnabled = false;
}

cmd(get_string) {
	int tex, row, col;

	debugC(4, kDebugLevelScripts, "%d %d %d %d %d", p0, p1, p2, p3, p4);

	tex = p1 - 1;
	row = p2;
	col = p3;

	/* Workaround for SQLC bug.
	 * See bug #792125 for details
	 */
	if (row > 24)
		row = 24;
	if (col > 39)
		col = 39;

	g_agi->newInputMode(INPUT_GETSTRING);

	if (curLogic->texts != NULL && curLogic->numTexts >= tex) {
		int len = strlen(curLogic->texts[tex]);
		g_agi->printText(curLogic->texts[tex], 0, col, row, len, game.colorFg, game.colorBg);
		g_agi->getString(col + len - 1, row, p4, p0);

		/* SGEO: display input char */
		g_gfx->printCharacter((col + len), row, game.cursorChar, game.colorFg, game.colorBg);
	}

	do {
		g_agi->mainCycle();
	} while (game.inputMode == INPUT_GETSTRING);
}

cmd(get_num) {
	debugC(4, kDebugLevelScripts, "%d %d", p0, p1);
	g_agi->newInputMode(INPUT_GETSTRING);

	if (curLogic->texts != NULL && curLogic->numTexts >= (p0 - 1)) {
		int len = strlen(curLogic->texts[p0 - 1]);
		g_agi->printText(curLogic->texts[p0 - 1], 0, 0, 22, len, game.colorFg, game.colorBg);
		g_agi->getString(len - 1, 22, 3, MAX_STRINGS);

		/* CM: display input char */
		g_gfx->printCharacter((p3 + len), 22, game.cursorChar, game.colorFg, game.colorBg);
	}

	do {
		g_agi->mainCycle();
	} while (game.inputMode == INPUT_GETSTRING);

	_v[p1] = atoi(game.strings[MAX_STRINGS]);
	debugC(4, kDebugLevelScripts, "[%s] -> %d", game.strings[MAX_STRINGS], _v[p1]);
	g_agi->clearLines(22, 22, game.colorBg);
	g_agi->flushLines(22, 22);
}

cmd(set_cursor_char) {
	if (curLogic->texts != NULL && (p0 - 1) <= curLogic->numTexts) {
		game.cursorChar = *curLogic->texts[p0 - 1];
	} else {
		/* default */
		game.cursorChar = '_';
	}
}

cmd(set_key) {
	int key;

	debugC(4, kDebugLevelScripts, "%d %d %d", p0, p1, p2);

	if (game.evKeyp[p2].data != 0)	/* TBC sets c23 (ESC) twice! */
		return;

	key = 256 * p1 + p0;
	game.evKeyp[p2].data = key;
	game.evKeyp[p2].occured = false;
}

cmd(set_string) {
	/* CM: to avoid crash in Groza (str = 150) */
	if (p0 > MAX_STRINGS)
		return;
	strcpy(game.strings[p0], curLogic->texts[p1 - 1]);
}

cmd(display) {
	g_agi->printText(curLogic->texts[p2 - 1], p1, 0, p0, 40, game.colorFg, game.colorBg);
}

cmd(display_f) {
	debugC(4, kDebugLevelScripts, "p0 = %d", p0);
	g_agi->printText(curLogic->texts[_v[p2] - 1], _v[p1], 0, _v[p0], 40, game.colorFg, game.colorBg);
}

cmd(clear_text_rect) {
	int c, x1, y1, x2, y2;

	if ((c = p4) != 0)
		c = 15;
	x1 = p1 * CHAR_COLS;
	y1 = p0 * CHAR_LINES;
	x2 = (p3 + 1) * CHAR_COLS - 1;
	y2 = (p2 + 1) * CHAR_LINES - 1;

	/* Added to prevent crash with x2 = 40 in the iigs demo */
	if (x1 > GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (x2 > GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y1 > GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (y2 > GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	g_gfx->drawRectangle(x1, y1, x2, y2, c);
	g_gfx->flushBlock(x1, y1, x2, y2);
}

cmd(toggle_monitor) {
	report("toggle.monitor\n");
}

cmd(echo_line) {
	strcpy((char *)game.inputBuffer, (const char *)game.echoBuffer);
	game.cursorPos = strlen((char *)game.inputBuffer);
	game.hasPrompt = 0;
}

cmd(clear_lines) {
	uint8 l;

	/* Residence 44 calls clear.lines(24,0,0), see bug #558423 */
	l = p1 ? p1 : p0;

	g_agi->clearLines(p0, l, p2);
	g_agi->flushLines(p0, l);
}

cmd(print) {
	int n = p0 < 1 ? 1 : p0;
	g_agi->print(curLogic->texts[n - 1], 0, 0, 0);
}

cmd(print_f) {
	int n = _v[p0] < 1 ? 1 : _v[p0];
	g_agi->print(curLogic->texts[n - 1], 0, 0, 0);
}

cmd(print_at) {
	int n = p0 < 1 ? 1 : p0;
	debugC(4, kDebugLevelScripts, "%d %d %d %d", p0, p1, p2, p3);
	g_agi->print(curLogic->texts[n - 1], p1, p2, p3);
}

cmd(print_at_v) {
	int n = _v[p0] < 1 ? 1 : _v[p0];
	g_agi->print(curLogic->texts[n - 1], p1, p2, p3);
}

cmd(push_script) {
	if (g_agi->getFeatures() & GF_AGIMOUSE) {
		game.vars[27] = g_mouse.button;
		game.vars[28] = g_mouse.x / 2;
		game.vars[29] = g_mouse.y;
	} else
		report("push.script\n");
}

cmd(set_pri_base) {
	int i, x, pri;

	report("Priority base set to %d\n", p0);

	/* game.alt_pri = true; */
	x = (_HEIGHT - p0) * _HEIGHT / 10;

	for (i = 0; i < _HEIGHT; i++) {
		pri = (i - p0) < 0 ? 4 : (i - p0) * _HEIGHT / x + 5;
		if (pri > 15)
			pri = 15;
		game.priTable[i] = pri;
	}
}

cmd(mouse_posn) {
	_v[p0] = WIN_TO_PIC_X(g_mouse.x);
	_v[p1] = WIN_TO_PIC_Y(g_mouse.y);
}

cmd(shake_screen) {
	int i;

	// AGIPAL uses shake.screen values between 101 and 109 to
	// set the palette.
	if (p0 >= 101 && p0 < 110) {
		if (g_agi->getFeatures() & GF_AGIPAL) {
			g_gfx->setAGIPal(p0);
			return;
		} else {
			warning("It looks like GF_AGIPAL flag is missing");
		}
	} 

	// Disables input while shaking to prevent bug
	// #1678230: AGI: Entering text while screen is shaking
	int originalValue = game.inputEnabled;
	game.inputEnabled = 0;

	g_gfx->shakeStart();

	g_sprites->commitBoth();		/* Fixes SQ1 demo */
	for (i = 4 * p0; i; i--) {
		g_gfx->shakeScreen(i & 1);
		g_gfx->flushBlock(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
		g_agi->mainCycle();
	}
	g_gfx->shakeEnd();

	//Sets input back to what it was
	game.inputEnabled = originalValue;
}

static void (*agiCommand[183])(uint8 *) = {
	NULL,			/* 0x00 */
	cmd_increment,
	cmd_decrement,
	cmd_assignn,
	cmd_assignv,
	cmd_addn,
	cmd_addv,
	cmd_subn,
	cmd_subv,	/* 0x08 */
	cmd_lindirectv,
	cmd_rindirect,
	cmd_lindirectn,
	cmd_set,
	cmd_reset,
	cmd_toggle,
	cmd_set_v,
	cmd_reset_v,	/* 0x10 */
	cmd_toggle_v,
	cmd_new_room,
	cmd_new_room_f,
	cmd_load_logic,
	cmd_load_logic_f,
	cmd_call,
	cmd_call_f,
	cmd_load_pic,	/* 0x18 */
	cmd_draw_pic,
	cmd_show_pic,
	cmd_discard_pic,
	cmd_overlay_pic,
	cmd_show_pri_screen,
	cmd_load_view,
	cmd_load_view_f,
	cmd_discard_view,	/* 0x20 */
	cmd_animate_obj,
	cmd_unanimate_all,
	cmd_draw,
	cmd_erase,
	cmd_position,
	cmd_position_f,
	cmd_get_posn,
	cmd_reposition,	/* 0x28 */
	cmd_set_view,
	cmd_set_view_f,
	cmd_set_loop,
	cmd_set_loop_f,
	cmd_fix_loop,
	cmd_release_loop,
	cmd_set_cel,
	cmd_set_cel_f,	/* 0x30 */
	cmd_last_cel,
	cmd_current_cel,
	cmd_current_loop,
	cmd_current_view,
	cmd_number_of_loops,
	cmd_set_priority,
	cmd_set_priority_f,
	cmd_release_priority,	/* 0x38 */
	cmd_get_priority,
	cmd_stop_update,
	cmd_start_update,
	cmd_force_update,
	cmd_ignore_horizon,
	cmd_observe_horizon,
	cmd_set_horizon,
	cmd_object_on_water,	/* 0x40 */
	cmd_object_on_land,
	cmd_object_on_anything,
	cmd_ignore_objs,
	cmd_observe_objs,
	cmd_distance,
	cmd_stop_cycling,
	cmd_start_cycling,
	cmd_normal_cycle,	/* 0x48 */
	cmd_end_of_loop,
	cmd_reverse_cycle,
	cmd_reverse_loop,
	cmd_cycle_time,
	cmd_stop_motion,
	cmd_start_motion,
	cmd_step_size,
	cmd_step_time,	/* 0x50 */
	cmd_move_obj,
	cmd_move_obj_f,
	cmd_follow_ego,
	cmd_wander,
	cmd_normal_motion,
	cmd_set_dir,
	cmd_get_dir,
	cmd_ignore_blocks,	/* 0x58 */
	cmd_observe_blocks,
	cmd_block,
	cmd_unblock,
	cmd_get,
	cmd_get_f,
	cmd_drop,
	cmd_put,
	cmd_put_f,	/* 0x60 */
	cmd_get_room_f,
	cmd_load_sound,
	cmd_sound,
	cmd_stop_sound,
	cmd_print,
	cmd_print_f,
	cmd_display,
	cmd_display_f,	/* 0x68 */
	cmd_clear_lines,
	cmd_text_screen,
	cmd_graphics,
	cmd_set_cursor_char,
	cmd_set_text_attribute,
	cmd_shake_screen,
	cmd_configure_screen,
	cmd_status_line_on,	/* 0x70 */
	cmd_status_line_off,
	cmd_set_string,
	cmd_get_string,
	cmd_word_to_string,
	cmd_parse,
	cmd_get_num,
	cmd_prevent_input,
	cmd_accept_input,	/* 0x78 */
	cmd_set_key,
	cmd_add_to_pic,
	cmd_add_to_pic_f,
	cmd_status,
	cmd_save_game,
	cmd_load_game,
	cmd_init_disk,
	cmd_restart_game,	/* 0x80 */
	cmd_show_obj,
	cmd_random,
	cmd_program_control,
	cmd_player_control,
	cmd_obj_status_f,
	cmd_quit,
	cmd_show_mem,
	cmd_pause,	/* 0x88 */
	cmd_echo_line,
	cmd_cancel_line,
	cmd_init_joy,
	cmd_toggle_monitor,
	cmd_version,
	cmd_script_size,
	cmd_set_game_id,
	cmd_log,	/* 0x90 */
	cmd_set_scan_start,
	cmd_reset_scan_start,
	cmd_reposition_to,
	cmd_reposition_to_f,
	cmd_trace_on,
	cmd_trace_info,
	cmd_print_at,
	cmd_print_at_v,	/* 0x98 */
	cmd_discard_view,
	cmd_clear_text_rect,
	cmd_set_upper_left,
	cmd_set_menu,
	cmd_set_menu_item,
	cmd_submit_menu,
	cmd_enable_item,
	cmd_disable_item,	/* 0xa0 */
	cmd_menu_input,
	cmd_show_obj_v,
	cmd_open_dialogue,
	cmd_close_dialogue,
	cmd_mul_n,
	cmd_mul_v,
	cmd_div_n,
	cmd_div_v,	/* 0xa8 */
	cmd_close_window,
	cmd_set_simple,
	cmd_push_script,
	cmd_pop_script,
	cmd_hold_key,
	cmd_set_pri_base,
	cmd_discard_sound,
	cmd_hide_mouse,	/* 0xb0 */
	cmd_allow_menu,
	cmd_show_mouse,
	cmd_fence_mouse,
	cmd_mouse_posn,
	cmd_release_key,
	cmd_adj_ego_move_to_x_y
};

#define CMD_BSIZE 12

/**
 * Execute a logic script
 * @param n  Number of the logic resource to execute
 */
int AgiEngine::runLogic(int n) {
	uint8 op = 0;
	uint8 p[CMD_BSIZE] = { 0 };
	uint8 *code = NULL;
	g_agi = this;
	int num = 0;

	/* If logic not loaded, load it */
	if (~_game.dirLogic[n].flags & RES_LOADED) {
		debugC(4, kDebugLevelScripts, "logic %d not loaded!", n);
		agiLoadResource(rLOGIC, n);
	}

	_game.lognum = n;
	curLogic = &_game.logics[game.lognum];

	code = curLogic->data;
	curLogic->cIP = curLogic->sIP;

	timerHack = 0;
	while (ip < _game.logics[n].size && !_game.quitProgNow) {
		if (_debug.enabled) {
			if (_debug.steps > 0) {
				if (_debug.logic0 || n) {
					debugConsole(n, lCOMMAND_MODE, NULL);
					_debug.steps--;
				}
			} else {
				_sprites->blitBoth();
				do {
					mainCycle();
				} while (!_debug.steps && _debug.enabled);
				_sprites->eraseBoth();
			}
		}

		switch (op = *(code + ip++)) {
		case 0xff:	/* if (open/close) */
			testIfCode(n);
			break;
		case 0xfe:	/* goto */
			/* +2 covers goto size */
			ip += 2 + ((int16)READ_LE_UINT16(code + ip));
			/* timer must keep running even in goto loops,
			 * but AGI engine can't do that :(
			 */
			if (timerHack > 20) {
				g_gfx->pollTimer();
				updateTimer();
				timerHack = 0;
			}
			break;
		case 0x00:	/* return */
			return 1;
		default:
			num = logicNamesCmd[op].numArgs;
			memmove(p, code + ip, num);
			memset(p + num, 0, CMD_BSIZE - num);

			debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logicNamesCmd[op].name, p[0], p[1], p[2]);
			agiCommand[op] (p);
			ip += num;
		}

		if (_game.exitAllLogics)
			break;
	}

	return 0;		/* after executing new.room() */
}

void AgiEngine::executeAgiCommand(uint8 op, uint8 *p) {
	debugC(2, kDebugLevelScripts, "%s(%d %d %d)", logicNamesCmd[op].name, p[0], p[1], p[2]);
	g_agi = this;

	agiCommand[op] (p);
}

} // End of namespace Agi
