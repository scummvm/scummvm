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
#include "agi/savegame.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

namespace Agi {

static const char *strSig = "AGI:";

void SaveGameMgr::write_string(Common::File *f, const char *s) {
	f->writeSint16BE((int16)strlen(s));
	f->write(s, strlen(s));
}

void SaveGameMgr::read_string(Common::File *f, char *s) {
	int16 size = f->readSint16BE();
	f->read(s, size);
	s[size] = (char)0;
}

void SaveGameMgr::write_bytes(Common::File *f, const char *s, int16 size) {
	f->write(s, size);
}

void SaveGameMgr::read_bytes(Common::File *f, char *s, int16 size) {
	f->read(s, size);
}

/*
 * Version 0: view table has 64 entries
 * Version 1: view table has 256 entries (needed in KQ3)
 */
#define SAVEGAME_VERSION 1

int SaveGameMgr::save_game(char *s, const char *d) {
	int16 i;
	struct image_stack_element *ptr = _vm->image_stack;
	Common::File f;

	// FIXME: Do *not* use Common::File to access savegames, it is not portable!
	f.open(s, Common::File::kFileWriteMode);

	if (!f.isOpen())
		return err_BadFileOpen;

	write_bytes(&f, strSig, 8);
	write_string(&f, d);

	f.writeByte((uint8)SAVEGAME_VERSION);
	f.writeByte((uint8)_vm->game.state);
	/* game.name */
	write_string(&f, _vm->game.id);
	/* game.crc */

	for (i = 0; i < MAX_FLAGS; i++)
		f.writeByte(_vm->game.flags[i]);
	for (i = 0; i < MAX_VARS; i++)
		f.writeByte(_vm->game.vars[i]);

	f.writeSint16BE((int8)_vm->game.horizon);
	f.writeSint16BE((int16)_vm->game.line_status);
	f.writeSint16BE((int16)_vm->game.line_user_input);
	f.writeSint16BE((int16)_vm->game.line_min_print);
	/* game.cursor_pos */
	/* game.input_buffer */
	/* game.echo_buffer */
	/* game.keypress */
	f.writeSint16BE((int16)_vm->game.input_mode);
	f.writeSint16BE((int16)_vm->game.lognum);

	f.writeSint16BE((int16)_vm->game.player_control);
	f.writeSint16BE((int16)_vm->game.quit_prog_now);
	f.writeSint16BE((int16)_vm->game.status_line);
	f.writeSint16BE((int16)_vm->game.clock_enabled);
	f.writeSint16BE((int16)_vm->game.exit_all_logics);
	f.writeSint16BE((int16)_vm->game.picture_shown);
	f.writeSint16BE((int16)_vm->game.has_prompt);
	f.writeSint16BE((int16)_vm->game.game_flags);

	/* Reversed to keep compatibility with old savegames */
	f.writeSint16BE((int16)!_vm->game.input_enabled);

	for (i = 0; i < _HEIGHT; i++)
		f.writeByte(_vm->game.pri_table[i]);

	/* game.msg_box_ticks */
	/* game.block */
	/* game.window */
	/* game.has_window */

	f.writeSint16BE((int16)_vm->game.gfx_mode);
	f.writeByte(_vm->game.cursor_char);
	f.writeSint16BE((int16)_vm->game.color_fg);
	f.writeSint16BE((int16)_vm->game.color_bg);

	/* game.hires */
	/* game.sbuf */
	/* game.ego_words */
	/* game.num_ego_words */

	f.writeSint16BE((int16)_vm->game.num_objects);
	for (i = 0; i < (int16)_vm->game.num_objects; i++)
		f.writeSint16BE((int16)_vm->object_get_location(i));

	/* game.ev_keyp */
	for (i = 0; i < MAX_STRINGS; i++)
		write_string(&f, _vm->game.strings[i]);

	/* record info about loaded resources */
	for (i = 0; i < MAX_DIRS; i++) {
		f.writeByte(_vm->game.dir_logic[i].flags);
		f.writeSint16BE((int16)_vm->game.logics[i].sIP);
		f.writeSint16BE((int16)_vm->game.logics[i].cIP);
	}
	for (i = 0; i < MAX_DIRS; i++)
		f.writeByte(_vm->game.dir_pic[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		f.writeByte(_vm->game.dir_view[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		f.writeByte(_vm->game.dir_sound[i].flags);

	/* game.pictures */
	/* game.logics */
	/* game.views */
	/* game.sounds */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &_vm->game.view_table[i];

		f.writeByte(v->step_time);
		f.writeByte(v->step_time_count);
		f.writeByte(v->entry);
		f.writeSint16BE(v->x_pos);
		f.writeSint16BE(v->y_pos);
		f.writeByte(v->current_view);

		/* v->view_data */

		f.writeByte(v->current_loop);
		f.writeByte(v->num_loops);

		/* v->loop_data */

		f.writeByte(v->current_cel);
		f.writeByte(v->num_cels);

		/* v->cel_data */
		/* v->cel_data_2 */

		f.writeSint16BE(v->x_pos2);
		f.writeSint16BE(v->y_pos2);

		/* v->s */

		f.writeSint16BE(v->x_size);
		f.writeSint16BE(v->y_size);
		f.writeByte(v->step_size);
		f.writeByte(v->cycle_time);
		f.writeByte(v->cycle_time_count);
		f.writeByte(v->direction);

		f.writeByte(v->motion);
		f.writeByte(v->cycle);
		f.writeByte(v->priority);

		f.writeUint16BE(v->flags);

		f.writeByte(v->parm1);
		f.writeByte(v->parm2);
		f.writeByte(v->parm3);
		f.writeByte(v->parm4);
	}

	/* Save image stack */

	for (i = 0; i < _vm->image_stack_pointer; i++) {
		ptr = &_vm->image_stack[i];
		f.writeByte(ptr->type);
		f.writeSint16BE(ptr->parm1);
		f.writeSint16BE(ptr->parm2);
		f.writeSint16BE(ptr->parm3);
		f.writeSint16BE(ptr->parm4);
		f.writeSint16BE(ptr->parm5);
		f.writeSint16BE(ptr->parm6);
		f.writeSint16BE(ptr->parm7);
	}
	f.writeByte(0);

	f.close();

	return err_OK;
}

int SaveGameMgr::load_game(char *s) {
	int i, ver, vt_entries = MAX_VIEWTABLE;
	uint8 t;
	int16 parm[7];
	char sig[8];
	char id[8];
	char description[256];
	Common::File f;

	// FIXME: Do *not* use Common::File to access savegames, it is not portable!
	f.open(s);

	if (!f.isOpen())
		return err_BadFileOpen;

	read_bytes(&f, sig, 8);
	if (strncmp(sig, strSig, 8)) {
		f.close();
		return err_BadFileOpen;
	}

	read_string(&f, description);

	ver = f.readByte();
	if (ver == 0)
		vt_entries = 64;
	_vm->game.state = f.readByte();
	/* game.name - not saved */
	read_string(&f, id);
	if (strcmp(id, _vm->game.id)) {
		f.close();
		return err_BadFileOpen;
	}
	/* game.crc - not saved */

	for (i = 0; i < MAX_FLAGS; i++)
		_vm->game.flags[i] = f.readByte();
	for (i = 0; i < MAX_VARS; i++)
		_vm->game.vars[i] = f.readByte();

	_vm->game.horizon = f.readSint16BE();
	_vm->game.line_status = f.readSint16BE();
	_vm->game.line_user_input = f.readSint16BE();
	_vm->game.line_min_print = f.readSint16BE();

	/* These are never saved */
	_vm->game.cursor_pos = 0;
	_vm->game.input_buffer[0] = 0;
	_vm->game.echo_buffer[0] = 0;
	_vm->game.keypress = 0;

	_vm->game.input_mode = f.readSint16BE();
	_vm->game.lognum = f.readSint16BE();

	_vm->game.player_control = f.readSint16BE();
	_vm->game.quit_prog_now = f.readSint16BE();
	_vm->game.status_line = f.readSint16BE();
	_vm->game.clock_enabled = f.readSint16BE();
	_vm->game.exit_all_logics = f.readSint16BE();
	_vm->game.picture_shown = f.readSint16BE();
	_vm->game.has_prompt = f.readSint16BE();
	_vm->game.game_flags = f.readSint16BE();
	_vm->game.input_enabled = !f.readSint16BE();

	for (i = 0; i < _HEIGHT; i++)
		_vm->game.pri_table[i] = f.readByte();

	if (_vm->game.has_window)
		_vm->close_window();

	_vm->game.msg_box_ticks = 0;
	_vm->game.block.active = false;
	/* game.window - fixed by close_window() */
	/* game.has_window - fixed by close_window() */

	_vm->game.gfx_mode = f.readSint16BE();
	_vm->game.cursor_char = f.readByte();
	_vm->game.color_fg = f.readSint16BE();
	_vm->game.color_bg = f.readSint16BE();

	/* game.hires - rebuilt from image stack */
	/* game.sbuf - rebuilt from image stack */

	/* game.ego_words - fixed by clean_input */
	/* game.num_ego_words - fixed by clean_input */

	_vm->game.num_objects = f.readSint16BE();
	for (i = 0; i < (int16)_vm->game.num_objects; i++)
		_vm->object_set_location(i, f.readSint16BE());

	/* Those are not serialized */
	for (i = 0; i < MAX_DIRS; i++) {
		_vm->game.ev_keyp[i].occured = false;
	}

	for (i = 0; i < MAX_STRINGS; i++)
		read_string(&f, _vm->game.strings[i]);

	for (i = 0; i < MAX_DIRS; i++) {
		if (f.readByte() & RES_LOADED)
			_vm->agiLoadResource(rLOGIC, i);
		else
			_vm->agiUnloadResource(rLOGIC, i);
		_vm->game.logics[i].sIP = f.readSint16BE();
		_vm->game.logics[i].cIP = f.readSint16BE();
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (f.readByte() & RES_LOADED)
			_vm->agiLoadResource(rPICTURE, i);
		else
			_vm->agiUnloadResource(rPICTURE, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (f.readByte() & RES_LOADED)
			_vm->agiLoadResource(rVIEW, i);
		else
			_vm->agiUnloadResource(rVIEW, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (f.readByte() & RES_LOADED)
			_vm->agiLoadResource(rSOUND, i);
		else
			_vm->agiUnloadResource(rSOUND, i);
	}

	/* game.pictures - loaded above */
	/* game.logics - loaded above */
	/* game.views - loaded above */
	/* game.sounds - loaded above */

	for (i = 0; i < vt_entries; i++) {
		struct vt_entry *v = &_vm->game.view_table[i];

		v->step_time = f.readByte();
		v->step_time_count = f.readByte();
		v->entry = f.readByte();
		v->x_pos = f.readSint16BE();
		v->y_pos = f.readSint16BE();
		v->current_view = f.readByte();

		/* v->view_data - fixed below  */

		v->current_loop = f.readByte();
		v->num_loops = f.readByte();

		/* v->loop_data - fixed below  */

		v->current_cel = f.readByte();
		v->num_cels = f.readByte();

		/* v->cel_data - fixed below  */
		/* v->cel_data_2 - fixed below  */

		v->x_pos2 = f.readSint16BE();
		v->y_pos2 = f.readSint16BE();

		/* v->s - fixed below */

		v->x_size = f.readSint16BE();
		v->y_size = f.readSint16BE();
		v->step_size = f.readByte();
		v->cycle_time = f.readByte();
		v->cycle_time_count = f.readByte();
		v->direction = f.readByte();

		v->motion = f.readByte();
		v->cycle = f.readByte();
		v->priority = f.readByte();

		v->flags = f.readUint16BE();

		v->parm1 = f.readByte();
		v->parm2 = f.readByte();
		v->parm3 = f.readByte();
		v->parm4 = f.readByte();
	}
	for (i = vt_entries; i < MAX_VIEWTABLE; i++) {
		memset(&_vm->game.view_table[i], 0, sizeof(struct vt_entry));
	}

	/* Fix some pointers in viewtable */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &_vm->game.view_table[i];

		if (_vm->game.dir_view[v->current_view].offset == _EMPTY)
			continue;

		if (!(_vm->game.dir_view[v->current_view].flags & RES_LOADED))
			_vm->agiLoadResource(rVIEW, v->current_view);

		_vm->set_view(v, v->current_view);	/* Fix v->view_data */
		_vm->set_loop(v, v->current_loop);	/* Fix v->loop_data */
		_vm->set_cel(v, v->current_cel);	/* Fix v->cel_data */
		v->cel_data_2 = v->cel_data;
		v->s = NULL;	/* not sure if it is used... */
	}

	_sprites->erase_both();

	/* Clear input line */
	_gfx->clearScreen(0);
	_vm->write_status();

	/* Recreate background from saved image stack */
	_vm->clear_image_stack();
	while ((t = f.readByte()) != 0) {
		for (i = 0; i < 7; i++)
			parm[i] = f.readSint16BE();
		_vm->replay_image_stack_call(t, parm[0], parm[1], parm[2],
				parm[3], parm[4], parm[5], parm[6]);
	}

	f.close();

	_vm->setflag(F_restore_just_ran, true);

	_vm->game.has_prompt = 0;	/* force input line repaint if necessary */
	_vm->clean_input();

	_sprites->erase_both();
	_sprites->blit_both();
	_sprites->commit_both();
	_picture->show_pic();
	_gfx->doUpdate();

	return err_OK;
}

#define NUM_SLOTS 12

int SaveGameMgr::select_slot() {
	int i, key, active = 0;
	int rc = -1;
	int hm = 2, vm = 3;	/* box margins */
	char desc[NUM_SLOTS][40];

	for (i = 0; i < NUM_SLOTS; i++) {
		char name[MAX_PATH];
		Common::File f;
		char sig[8];
		// FIXME: Do *not* use Common::File to access savegames, it is not portable!
		sprintf(name, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, i);
		f.open(name);
		if (!f.isOpen()) {
			strcpy(desc[i], "          (empty slot)");
		} else {
			read_bytes(&f, sig, 8);
			if (strncmp(sig, strSig, 8)) {
				strcpy(desc[i], "(corrupt file)");
			} else {
				read_string(&f, desc[i]);
			}
			f.close();
		}
	}

	for (;;) {
		char dstr[64];
		for (i = 0; i < NUM_SLOTS; i++) {
			sprintf(dstr, "[%-32.32s]", desc[i]);
			_vm->print_text(dstr, 0, hm + 1, vm + 4 + i,
					(40 - 2 * hm) - 1, i == active ? MSG_BOX_COLOUR : MSG_BOX_TEXT,
					i == active ? MSG_BOX_TEXT : MSG_BOX_COLOUR);

		}

		_gfx->pollTimer();	/* msdos driver -> does nothing */
		key = _vm->do_poll_keyboard();
		switch (key) {
		case KEY_ENTER:
			rc = active;
			strncpy(_vm->game.strings[MAX_STRINGS], desc[i], MAX_STRINGLEN);
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
	_vm->close_window();
	return rc;
}

int SaveGameMgr::savegame_simple() {
	char path[MAX_PATH];

	sprintf(path, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, 0);
	save_game(path, "Default savegame");

	return err_OK;
}

int SaveGameMgr::savegame_dialog() {
	char path[MAX_PATH];
	char *desc;
	const char *buttons[] = { "Do as I say!", "I regret", NULL };
	char dstr[200];
	int rc, slot = 0;
	int hm, vm, hp, vp;	/* box margins */
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(path, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, slot);

	_vm->draw_window(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	_vm->print_text("Select a slot in which you wish to save the game:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	_vm->print_text("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = select_slot();
	if (slot < 0)		/* ESC pressed */
		return err_OK;

	/* Get savegame description */
	_vm->draw_window(hp, vp + 5 * CHAR_LINES, GFX_WIDTH - hp,
			GFX_HEIGHT - vp - 9 * CHAR_LINES);
	_vm->print_text("Enter a description for this game:",
			0, hm + 1, vm + 6, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	_gfx->drawRectangle(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES, MSG_BOX_TEXT);
	_gfx->flushBlock(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES);

	_vm->get_string(2, 11, 33, MAX_STRINGS);
	_gfx->printCharacter(3, 11, _vm->game.cursor_char, MSG_BOX_COLOUR, MSG_BOX_TEXT);
	do {
		_vm->main_cycle();
	} while (_vm->game.input_mode == INPUT_GETSTRING);
	_vm->close_window();

	desc = _vm->game.strings[MAX_STRINGS];
	sprintf(dstr, "Are you sure you want to save the game "
			"described as:\n\n%s\n\nin slot %d?\n\n\n", desc, slot);

	rc = _vm->selection_box(dstr, buttons);

	if (rc != 0) {
		_vm->message_box("Game NOT saved.");
		return err_OK;
	}

	sprintf(path, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, slot);
	debugC(8, kDebugLevelMain | kDebugLevelResources, "file is [%s]", path);

	save_game(path, desc);

	_vm->message_box("Game saved.");

	return err_OK;
}

int SaveGameMgr::loadgame_simple() {
	char path[MAX_PATH];
	int rc = 0;

	sprintf(path, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, 0);

	_sprites->erase_both();
	_sound->stop_sound();
	_vm->close_window();

	if ((rc = load_game(path)) == err_OK) {
		_vm->message_box("Game restored.");
		_vm->game.exit_all_logics = 1;
		_vm->menu->enable_all();
	} else {
		_vm->message_box("Error restoring game.");
	}

	return rc;
}

int SaveGameMgr::loadgame_dialog() {
	char path[MAX_PATH];
	int rc, slot = 0;
	int hm, vm, hp, vp;	/* box margins */
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(path, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, slot);

	_sprites->erase_both();
	_sound->stop_sound();

	_vm->draw_window(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	_vm->print_text("Select a game which you wish to\nrestore:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	_vm->print_text("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = select_slot();

	if (slot < 0) {
		_vm->message_box("Game NOT restored.");
		return err_OK;
	}

	sprintf(path, "%s/%05X_%s_%02d.sav", _vm->_savePath, _vm->game.crc, _vm->game.id, slot);

	if ((rc = load_game(path)) == err_OK) {
		_vm->message_box("Game restored.");
		_vm->game.exit_all_logics = 1;
		_vm->menu->enable_all();
	} else {
		_vm->message_box("Error restoring game.");
	}

	return rc;
}

}                             // End of namespace Agi
