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

/*
 * Savegame support by Vasyl Tsvirkunov <vasyl@pacbell.net>
 * Multi-slots by Claudio Matsuoka <claudio@helllabs.org>
 */

#include "common/stdafx.h"
#include "common/file.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

#define SAVEGAME_VERSION 2

/*
 * Version 0 (Sarien): view table has 64 entries
 * Version 1 (Sarien): view table has 256 entries (needed in KQ3)
 * Version 2 (ScummVM): first ScummVM version
 */

namespace Agi {

static const uint32 AGIflag=MKID_BE('AGI:');

int AgiEngine::saveGame(const char *fileName, const char *description) {
	char gameIDstring[8]="gameIDX";
	int i;
	struct image_stack_element *ptr = image_stack;
	Common::OutSaveFile *out;

	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "AgiEngine::saveGame(%s, %s)", fileName, description);
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName);
		return err_BadFileOpen;
	} else {
		debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for writing", fileName);
	}

	out->writeUint32BE(AGIflag);
	out->write(description, 31);

	out->writeByte(SAVEGAME_VERSION);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing save game version (%d)", SAVEGAME_VERSION);

	out->writeByte(game.state);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing game state (%d)", game.state);
	
	strcpy(gameIDstring, game.id);
	out->write(gameIDstring, 8);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing game id (%s, %s)", gameIDstring, game.id);

	for (i = 0; i < MAX_FLAGS; i++)
		out->writeByte(game.flags[i]);
	for (i = 0; i < MAX_VARS; i++)
		out->writeByte(game.vars[i]);

	out->writeSint16BE((int8)game.horizon);
	out->writeSint16BE((int16)game.line_status);
	out->writeSint16BE((int16)game.line_user_input);
	out->writeSint16BE((int16)game.line_min_print);

	out->writeSint16BE((int16)game.input_mode);
	out->writeSint16BE((int16)game.lognum);

	out->writeSint16BE((int16)game.player_control);
	out->writeSint16BE((int16)game.quit_prog_now);
	out->writeSint16BE((int16)game.status_line);
	out->writeSint16BE((int16)game.clock_enabled);
	out->writeSint16BE((int16)game.exit_all_logics);
	out->writeSint16BE((int16)game.picture_shown);
	out->writeSint16BE((int16)game.has_prompt);
	out->writeSint16BE((int16)game.game_flags);

	out->writeSint16BE((int16)game.input_enabled);

	for (i = 0; i < _HEIGHT; i++)
		out->writeByte(game.pri_table[i]);
	
	out->writeSint16BE((int16)game.gfx_mode);
	out->writeByte(game.cursor_char);
	out->writeSint16BE((int16)game.color_fg);
	out->writeSint16BE((int16)game.color_bg);

	/* game.hires */
	/* game.sbuf */
	/* game.ego_words */
	/* game.num_ego_words */

	out->writeSint16BE((int16)game.num_objects);
	for (i = 0; i < (int16)game.num_objects; i++)
		out->writeSint16BE((int16)object_get_location(i));

	/* game.ev_keyp */
	for (i = 0; i < MAX_STRINGS; i++)
		out->write(game.strings[i], MAX_STRINGLEN);

	/* record info about loaded resources */
	for (i = 0; i < MAX_DIRS; i++) {
		out->writeByte(game.dir_logic[i].flags);
		out->writeSint16BE((int16)game.logics[i].sIP);
		out->writeSint16BE((int16)game.logics[i].cIP);
	}
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(game.dir_pic[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(game.dir_view[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(game.dir_sound[i].flags);

	/* game.pictures */
	/* game.logics */
	/* game.views */
	/* game.sounds */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &game.view_table[i];

		out->writeByte(v->step_time);
		out->writeByte(v->step_time_count);
		out->writeByte(v->entry);
		out->writeSint16BE(v->x_pos);
		out->writeSint16BE(v->y_pos);
		out->writeByte(v->current_view);

		/* v->view_data */

		out->writeByte(v->current_loop);
		out->writeByte(v->num_loops);

		/* v->loop_data */

		out->writeByte(v->current_cel);
		out->writeByte(v->num_cels);

		/* v->cel_data */
		/* v->cel_data_2 */

		out->writeSint16BE(v->x_pos2);
		out->writeSint16BE(v->y_pos2);

		/* v->s */

		out->writeSint16BE(v->x_size);
		out->writeSint16BE(v->y_size);
		out->writeByte(v->step_size);
		out->writeByte(v->cycle_time);
		out->writeByte(v->cycle_time_count);
		out->writeByte(v->direction);

		out->writeByte(v->motion);
		out->writeByte(v->cycle);
		out->writeByte(v->priority);

		out->writeUint16BE(v->flags);

		out->writeByte(v->parm1);
		out->writeByte(v->parm2);
		out->writeByte(v->parm3);
		out->writeByte(v->parm4);
	}

	/* Save image stack */

	for (i = 0; i < image_stack_pointer; i++) {
		ptr = &image_stack[i];
		out->writeByte(ptr->type);
		out->writeSint16BE(ptr->parm1);
		out->writeSint16BE(ptr->parm2);
		out->writeSint16BE(ptr->parm3);
		out->writeSint16BE(ptr->parm4);
		out->writeSint16BE(ptr->parm5);
		out->writeSint16BE(ptr->parm6);
		out->writeSint16BE(ptr->parm7);
	}
	out->writeByte(0);

	out->flush();
	if (out->ioFailed())
		warning("Can't write file '%s'. (Disk full?)", fileName);
	else
		debugC(1, kDebugLevelMain | kDebugLevelSavegame, "Saved game %s in file %s", description, fileName);
	
	delete out;
	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Closed %s", fileName);
	return err_OK;
}

int AgiEngine::loadGame(const char *fileName) {
	char description[31], saveVersion, loadId[8];
	int i, vt_entries = MAX_VIEWTABLE;
	uint8 t;
	int16 parm[7];
	Common::InSaveFile *in;

	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "AgiEngine::loadGame(%s)", fileName);

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName);
		return err_BadFileOpen;
	} else {
		debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for reading", fileName);
	}

	uint32 typea = in->readUint32BE();
	if (typea == AGIflag) {
		debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Has AGI flag, good start");
	} else {
		warning("This doesn't appear to be an AGI savegame, game not restored");
		delete in;	
		return err_OK;
	} 

	in->read(description, 31);
	
	debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Description is: %s", description);
	
	saveVersion = in->readByte();
	if (saveVersion != SAVEGAME_VERSION)
		warning("Old save game version (%d, current version is %d). Will try and read anyway, but don't be surprised if bad things happen", saveVersion, SAVEGAME_VERSION);

	game.state = in->readByte();
	
	in->read(loadId, 8);
	if (strcmp(loadId, game.id)) {
		delete in;	
		warning("This save seems to be from a different AGI game (save from %s, running %s), not loaded", loadId, game.id);
		return err_BadFileOpen;
	}

	for (i = 0; i < MAX_FLAGS; i++)
		game.flags[i] = in->readByte();
	for (i = 0; i < MAX_VARS; i++)
		game.vars[i] = in->readByte();

	game.horizon = in->readSint16BE();
	game.line_status = in->readSint16BE();
	game.line_user_input = in->readSint16BE();
	game.line_min_print = in->readSint16BE();
	
	/* These are never saved */
	game.cursor_pos = 0;
	game.input_buffer[0] = 0;
	game.echo_buffer[0] = 0;
	game.keypress = 0;

	game.input_mode = in->readSint16BE();
	game.lognum = in->readSint16BE();

	game.player_control = in->readSint16BE();
	game.quit_prog_now = in->readSint16BE();
	game.status_line = in->readSint16BE();
	game.clock_enabled = in->readSint16BE();
	game.exit_all_logics = in->readSint16BE();
	game.picture_shown = in->readSint16BE();
	game.has_prompt = in->readSint16BE();
	game.game_flags = in->readSint16BE();
	game.input_enabled = in->readSint16BE();

	for (i = 0; i < _HEIGHT; i++)
		game.pri_table[i] = in->readByte();

	if (game.has_window)
		close_window();

	game.msg_box_ticks = 0;
	game.block.active = false;
	/* game.window - fixed by close_window() */
	/* game.has_window - fixed by close_window() */

	game.gfx_mode = in->readSint16BE();
	game.cursor_char = in->readByte();
	game.color_fg = in->readSint16BE();
	game.color_bg = in->readSint16BE();

	/* game.hires - rebuilt from image stack */
	/* game.sbuf - rebuilt from image stack */

	/* game.ego_words - fixed by clean_input */
	/* game.num_ego_words - fixed by clean_input */

	game.num_objects = in->readSint16BE();
	for (i = 0; i < (int16)game.num_objects; i++)
		object_set_location(i, in->readSint16BE());

	/* Those are not serialized */
	for (i = 0; i < MAX_DIRS; i++) {
		game.ev_keyp[i].occured = false;
	}

	for (i = 0; i < MAX_STRINGS; i++)
		in->read(game.strings[i], MAX_STRINGLEN);

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rLOGIC, i);
		else
			agiUnloadResource(rLOGIC, i);
		game.logics[i].sIP = in->readSint16BE();
		game.logics[i].cIP = in->readSint16BE();
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rPICTURE, i);
		else
			agiUnloadResource(rPICTURE, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rVIEW, i);
		else
			agiUnloadResource(rVIEW, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rSOUND, i);
		else
			agiUnloadResource(rSOUND, i);
	}

	/* game.pictures - loaded above */
	/* game.logics - loaded above */
	/* game.views - loaded above */
	/* game.sounds - loaded above */

	for (i = 0; i < vt_entries; i++) {
		struct vt_entry *v = &game.view_table[i];

		v->step_time = in->readByte();
		v->step_time_count = in->readByte();
		v->entry = in->readByte();
		v->x_pos = in->readSint16BE();
		v->y_pos = in->readSint16BE();
		v->current_view = in->readByte();

		/* v->view_data - fixed below  */

		v->current_loop = in->readByte();
		v->num_loops = in->readByte();

		/* v->loop_data - fixed below  */

		v->current_cel = in->readByte();
		v->num_cels = in->readByte();

		/* v->cel_data - fixed below  */
		/* v->cel_data_2 - fixed below  */

		v->x_pos2 = in->readSint16BE();
		v->y_pos2 = in->readSint16BE();

		/* v->s - fixed below */

		v->x_size = in->readSint16BE();
		v->y_size = in->readSint16BE();
		v->step_size = in->readByte();
		v->cycle_time = in->readByte();
		v->cycle_time_count = in->readByte();
		v->direction = in->readByte();

		v->motion = in->readByte();
		v->cycle = in->readByte();
		v->priority = in->readByte();

		v->flags = in->readUint16BE();

		v->parm1 = in->readByte();
		v->parm2 = in->readByte();
		v->parm3 = in->readByte();
		v->parm4 = in->readByte();
	}
	for (i = vt_entries; i < MAX_VIEWTABLE; i++) {
		memset(&game.view_table[i], 0, sizeof(struct vt_entry));
	}

	/* Fix some pointers in viewtable */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &game.view_table[i];

		if (game.dir_view[v->current_view].offset == _EMPTY)
			continue;

		if (!(game.dir_view[v->current_view].flags & RES_LOADED))
			agiLoadResource(rVIEW, v->current_view);

		set_view(v, v->current_view);	/* Fix v->view_data */
		set_loop(v, v->current_loop);	/* Fix v->loop_data */
		set_cel(v, v->current_cel);	/* Fix v->cel_data */
		v->cel_data_2 = v->cel_data;
		v->s = NULL;	/* not sure if it is used... */
	}

	_sprites->erase_both();

	/* Clear input line */
	_gfx->clearScreen(0);
	write_status();

	/* Recreate background from saved image stack */
	clear_image_stack();
	while ((t = in->readByte()) != 0) {
		for (i = 0; i < 7; i++)
			parm[i] = in->readSint16BE();
		replay_image_stack_call(t, parm[0], parm[1], parm[2],
				parm[3], parm[4], parm[5], parm[6]);
	}

	delete in;	
	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Closed %s", fileName);

	setflag(F_restore_just_ran, true);

	game.has_prompt = 0;	/* force input line repaint if necessary */
	clean_input();

	_sprites->erase_both();
	_sprites->blit_both();
	_sprites->commit_both();
	_picture->show_pic();
	_gfx->doUpdate();

	return err_OK;
}

#define NUM_SLOTS 12

const char *AgiEngine::getSavegameFilename(int num) {
	static char saveLoadSlot[12];
	sprintf(saveLoadSlot, "%s.%.3d", _targetName.c_str(), num);
	return saveLoadSlot;
}

int AgiEngine::selectSlot() {
	int i, key, active = 0;
	int rc = -1;
	int hm = 2, vm = 3;	/* box margins */
	char desc[NUM_SLOTS][40];

	for (i = 0; i < NUM_SLOTS; i++) {
		char fileName[MAX_PATH];
		Common::InSaveFile *in;
		
		debugC(4, kDebugLevelMain | kDebugLevelSavegame, "Game id seems to be %s", _targetName.c_str());
		sprintf(fileName, "%s", getSavegameFilename(i));
		if (!(in = _saveFileMan->openForLoading(fileName))) {
			debugC(4, kDebugLevelMain | kDebugLevelSavegame, "File %s does not exist", fileName);
			strcpy(desc[i], "          (empty slot)");
		} else {
			debugC(4, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for reading", fileName);
			uint32 type = in->readUint32BE();
			if (type == AGIflag) {
				debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Has AGI flag, good start");
				in->read(desc[i], 31);
			} else {
				warning("This doesn't appear to be an AGI savegame");
				strcpy(desc[i], "(corrupt file)");
			} 

			delete in;
		}
	}

	for (;;) {
		char dstr[64];
		for (i = 0; i < NUM_SLOTS; i++) {
			sprintf(dstr, "[%-32.32s]", desc[i]);
			print_text(dstr, 0, hm + 1, vm + 4 + i,
					(40 - 2 * hm) - 1, i == active ? MSG_BOX_COLOUR : MSG_BOX_TEXT,
					i == active ? MSG_BOX_TEXT : MSG_BOX_COLOUR);

		}

		_gfx->pollTimer();	/* msdos driver -> does nothing */
		key = do_poll_keyboard();
		switch (key) {
		case KEY_ENTER:
			rc = active;
			strncpy(game.strings[MAX_STRINGS], desc[i], MAX_STRINGLEN);
			goto press;
		case KEY_ESCAPE:
			rc = -1;
			goto getout;
		case BUTTON_LEFT:
			break;
		case KEY_DOWN:
			active++;
			active %= NUM_SLOTS;
			break;
		case KEY_UP:
			active--;
			if (active < 0)
				active = NUM_SLOTS - 1;
			break;
		}
		_gfx->doUpdate();
	}

press:
	debugC(8, kDebugLevelMain | kDebugLevelInput, "Button pressed: %d", rc);

getout:
	close_window();
	return rc;
}

int AgiEngine::saveGameDialog() {
	char fileName[MAX_PATH];
	char *desc;
	const char *buttons[] = { "Do as I say!", "I regret", NULL };
	char dstr[200];
	int rc, slot = 0;
	int hm, vm, hp, vp;	
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(fileName, "%s", getSavegameFilename(slot));

	draw_window(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	print_text("Select a slot in which you wish to save the game:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	print_text("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = selectSlot();
	if (slot < 0)	
		return err_OK;

	draw_window(hp, vp + 5 * CHAR_LINES, GFX_WIDTH - hp,
			GFX_HEIGHT - vp - 9 * CHAR_LINES);
	print_text("Enter a description for this game:",
			0, hm + 1, vm + 6, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	_gfx->drawRectangle(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES, MSG_BOX_TEXT);
	_gfx->flushBlock(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES);

	get_string(2, 11, 33, MAX_STRINGS);
	_gfx->printCharacter(3, 11, game.cursor_char, MSG_BOX_COLOUR, MSG_BOX_TEXT);
	do {
		main_cycle();
	} while (game.input_mode == INPUT_GETSTRING);
	close_window();

	desc = game.strings[MAX_STRINGS];
	sprintf(dstr, "Are you sure you want to save the game "
			"described as:\n\n%s\n\nin slot %d?\n\n\n", desc, slot);

	rc = selection_box(dstr, buttons);

	if (rc != 0) {
		message_box("Game NOT saved.");
		return err_OK;
	}

	sprintf(fileName, "%s", getSavegameFilename(slot));
	debugC(8, kDebugLevelMain | kDebugLevelResources, "file is [%s]", fileName);

	saveGame(fileName, desc);

	message_box("Game saved.");

	return err_OK;
}

int AgiEngine::saveGameSimple() {
	char fileName[MAX_PATH];

	sprintf(fileName, "%s", getSavegameFilename(0));
	saveGame(fileName, "Default savegame");

	return err_OK;
}

int AgiEngine::loadGameDialog() {
	char fileName[MAX_PATH];
	int rc, slot = 0;
	int hm, vm, hp, vp;	/* box margins */
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(fileName, "%s", getSavegameFilename(slot));

	_sprites->erase_both();
	_sound->stop_sound();

	draw_window(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	print_text("Select a game which you wish to\nrestore:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	print_text("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = selectSlot();

	if (slot < 0) {
		message_box("Game NOT restored.");
		return err_OK;
	}

	sprintf(fileName, "%s", getSavegameFilename(slot));

	if ((rc = loadGame(fileName)) == err_OK) {
		message_box("Game restored.");
		game.exit_all_logics = 1;
		menu->enable_all();
	} else {
		message_box("Error restoring game.");
	}

	return rc;
}

int AgiEngine::loadGameSimple() {
	char fileName[MAX_PATH];
	int rc = 0;

	sprintf(fileName, "%s", getSavegameFilename(0));

	_sprites->erase_both();
	_sound->stop_sound();
	close_window();

	if ((rc = loadGame(fileName)) == err_OK) {
		message_box("Game restored.");
		game.exit_all_logics = 1;
		menu->enable_all();
	} else {
		message_box("Error restoring game.");
	}

	return rc;
}

}                             // End of namespace Agi
