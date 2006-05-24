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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/text.h"
#include "agi/savegame.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

namespace Agi {

#if defined(WIN32)
#define MKDIR(a,b) mkdir(a)
#else
#define MKDIR(a,b) mkdir(a,b)
#endif

/* Image stack support */
struct image_stack_element {
	uint8 type;
	uint8 pad;
	int16 parm1;
	int16 parm2;
	int16 parm3;
	int16 parm4;
	int16 parm5;
	int16 parm6;
	int16 parm7;
};

#define INITIAL_IMAGE_STACK_SIZE 32
static int stack_size = 0;
static struct image_stack_element *image_stack = NULL;
static int image_stack_pointer = 0;

void clear_image_stack(void) {
	image_stack_pointer = 0;
}

void release_image_stack(void) {
	if (image_stack)
		free(image_stack);
	image_stack = NULL;
	stack_size = image_stack_pointer = 0;
}

void record_image_stack_call(uint8 type, int16 p1, int16 p2, int16 p3,
    int16 p4, int16 p5, int16 p6, int16 p7) {
	struct image_stack_element *pnew;

	if (image_stack_pointer == stack_size) {
		if (stack_size == 0) {	/* first call */
			image_stack = (struct image_stack_element *)
					malloc(INITIAL_IMAGE_STACK_SIZE * sizeof(struct image_stack_element));
			stack_size = INITIAL_IMAGE_STACK_SIZE;
		} else {	/* has to grow */
			struct image_stack_element *new_stack;
			new_stack = (struct image_stack_element *)
					malloc(2 * stack_size * sizeof(struct image_stack_element));
			memcpy(new_stack, image_stack, stack_size * sizeof(struct image_stack_element));
			free(image_stack);
			image_stack = new_stack;
			stack_size *= 2;
		}
	}

	pnew = &image_stack[image_stack_pointer];
	image_stack_pointer++;

	pnew->type = type;
	pnew->parm1 = p1;
	pnew->parm2 = p2;
	pnew->parm3 = p3;
	pnew->parm4 = p4;
	pnew->parm5 = p5;
	pnew->parm6 = p6;
	pnew->parm7 = p7;
}

void replay_image_stack_call(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	switch (type) {
	case ADD_PIC:
		debugC(8, kDebugLevelMain, "--- decoding picture %d ---", p1);
		agi_load_resource(rPICTURE, p1);
		decode_picture(p1, p2);
		break;
	case ADD_VIEW:
		agi_load_resource(rVIEW, p1);
		add_to_pic(p1, p2, p3, p4, p5, p6, p7);
		break;
	}
}

/* */

static char *strSig = "AGI:";

static void write_uint8(Common::File *f, int8 val) {
	f->write(&val, 1);
}

static void write_sint16(Common::File *f, int16 val) {
	static uint8 buf[2];
	buf[0] = (uint8) ((val >> 8) & 255);
	buf[1] = (uint8) (val & 255);
	f->write(buf, 2);
}

static void write_uint16(Common::File *f, uint16 val) {
	static uint8 buf[2];
	buf[0] = (uint8) ((val >> 8) & 255);
	buf[1] = (uint8) (val & 255);
	f->write(buf, 2);
}

static uint8 read_uint8(Common::File *f) {
	static uint8 buf[1];
	f->read(buf, 1);
	return buf[0];
}

static uint16 read_uint16(Common::File *f) {
	static uint8 buf[2];
	f->read(buf, 2);
	return (buf[0] << 8) | buf[1];
}

static int16 read_sint16(Common::File *f) {
	static uint8 buf[2];
	f->read(buf, 2);
	return (int16) ((buf[0] << 8) | buf[1]);
}

static void write_string(Common::File *f, char *s) {
	write_sint16(f, (int16) strlen(s));
	f->write(s, strlen(s));
}

static void read_string(Common::File *f, char *s) {
	int16 size = read_sint16(f);
	f->read(s, size);
	s[size] = (char)0;
}

static void write_bytes(Common::File *f, char *s, int16 size) {
	f->write(s, size);
}

static void read_bytes(Common::File *f, char *s, int16 size) {
	f->read(s, size);
}

/*
 * Version 0: view table has 64 entries
 * Version 1: view table has 256 entries (needed in KQ3)
 */
#define SAVEGAME_VERSION 1

int save_game(char *s, char *d) {
	int16 i;
	struct image_stack_element *ptr = image_stack;
	Common::File f;

	f.open(s, Common::File::kFileWriteMode);

	if (!f.isOpen())
		return err_BadFileOpen;

	write_bytes(&f, strSig, 8);
	write_string(&f, d);

	write_uint8(&f, (uint8) SAVEGAME_VERSION);
	write_uint8(&f, (uint8) game.state);
	/* game.name */
	write_string(&f, game.id);
	/* game.crc */

	for (i = 0; i < MAX_FLAGS; i++)
		write_uint8(&f, game.flags[i]);
	for (i = 0; i < MAX_VARS; i++)
		write_uint8(&f, game.vars[i]);

	write_sint16(&f, (int8) game.horizon);
	write_sint16(&f, (int16) game.line_status);
	write_sint16(&f, (int16) game.line_user_input);
	write_sint16(&f, (int16) game.line_min_print);
	/* game.cursor_pos */
	/* game.input_buffer */
	/* game.echo_buffer */
	/* game.keypress */
	write_sint16(&f, (int16) game.input_mode);
	write_sint16(&f, (int16) game.lognum);

	write_sint16(&f, (int16) game.player_control);
	write_sint16(&f, (int16) game.quit_prog_now);
	write_sint16(&f, (int16) game.status_line);
	write_sint16(&f, (int16) game.clock_enabled);
	write_sint16(&f, (int16) game.exit_all_logics);
	write_sint16(&f, (int16) game.picture_shown);
	write_sint16(&f, (int16) game.has_prompt);
	write_sint16(&f, (int16) game.game_flags);

	/* Reversed to keep compatibility with old savegames */
	write_sint16(&f, (int16)!game.input_enabled);

	for (i = 0; i < _HEIGHT; i++)
		write_uint8(&f, game.pri_table[i]);

	/* game.msg_box_ticks */
	/* game.block */
	/* game.window */
	/* game.has_window */

	write_sint16(&f, (int16)game.gfx_mode);
	write_uint8(&f, game.cursor_char);
	write_sint16(&f, (int16)game.color_fg);
	write_sint16(&f, (int16)game.color_bg);

	/* game.hires (#ifdef USE_HIRES) */
	/* game.sbuf */
	/* game.ego_words */
	/* game.num_ego_words */

	write_sint16(&f, (int16)game.num_objects);
	for (i = 0; i < (int16)game.num_objects; i++)
		write_sint16(&f, (int16)object_get_location(i));

	/* game.ev_keyp */
	for (i = 0; i < MAX_STRINGS; i++)
		write_string(&f, game.strings[i]);

	/* record info about loaded resources */
	for (i = 0; i < MAX_DIRS; i++) {
		write_uint8(&f, game.dir_logic[i].flags);
		write_sint16(&f, (int16)game.logics[i].sIP);
		write_sint16(&f, (int16)game.logics[i].cIP);
	}
	for (i = 0; i < MAX_DIRS; i++)
		write_uint8(&f, game.dir_pic[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		write_uint8(&f, game.dir_view[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		write_uint8(&f, game.dir_sound[i].flags);

	/* game.pictures */
	/* game.logics */
	/* game.views */
	/* game.sounds */

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		struct vt_entry *v = &game.view_table[i];

		write_uint8(&f, v->step_time);
		write_uint8(&f, v->step_time_count);
		write_uint8(&f, v->entry);
		write_sint16(&f, v->x_pos);
		write_sint16(&f, v->y_pos);
		write_uint8(&f, v->current_view);

		/* v->view_data */

		write_uint8(&f, v->current_loop);
		write_uint8(&f, v->num_loops);

		/* v->loop_data */

		write_uint8(&f, v->current_cel);
		write_uint8(&f, v->num_cels);

		/* v->cel_data */
		/* v->cel_data_2 */

		write_sint16(&f, v->x_pos2);
		write_sint16(&f, v->y_pos2);

		/* v->s */

		write_sint16(&f, v->x_size);
		write_sint16(&f, v->y_size);
		write_uint8(&f, v->step_size);
		write_uint8(&f, v->cycle_time);
		write_uint8(&f, v->cycle_time_count);
		write_uint8(&f, v->direction);

		write_uint8(&f, v->motion);
		write_uint8(&f, v->cycle);
		write_uint8(&f, v->priority);

		write_uint16(&f, v->flags);

		write_uint8(&f, v->parm1);
		write_uint8(&f, v->parm2);
		write_uint8(&f, v->parm3);
		write_uint8(&f, v->parm4);
	}

	/* Save image stack */

	for (i = 0; i < image_stack_pointer; i++) {
		ptr = &image_stack[i];
		write_uint8(&f, ptr->type);
		write_sint16(&f, ptr->parm1);
		write_sint16(&f, ptr->parm2);
		write_sint16(&f, ptr->parm3);
		write_sint16(&f, ptr->parm4);
		write_sint16(&f, ptr->parm5);
		write_sint16(&f, ptr->parm6);
		write_sint16(&f, ptr->parm7);
	}
	write_uint8(&f, 0);

	f.close();

	return err_OK;
}

int load_game(char *s) {
	int i, ver, vt_entries = MAX_VIEWTABLE;
	uint8 t;
	int16 parm[7];
	char sig[8];
	char id[8];
	char description[256];
	Common::File f;

	f.open(s);

	if (!f.isOpen())
		return err_BadFileOpen;

	read_bytes(&f, sig, 8);
	if (strncmp(sig, strSig, 8)) {
		f.close();
		return err_BadFileOpen;
	}

	read_string(&f, description);

	ver = read_uint8(&f);
	if (ver == 0)
		vt_entries = 64;
	game.state = read_uint8(&f);
	/* game.name - not saved */
	read_string(&f, id);
	if (strcmp(id, game.id)) {
		f.close();
		return err_BadFileOpen;
	}
	/* game.crc - not saved */

	for (i = 0; i < MAX_FLAGS; i++)
		game.flags[i] = read_uint8(&f);
	for (i = 0; i < MAX_VARS; i++)
		game.vars[i] = read_uint8(&f);

	game.horizon = read_sint16(&f);
	game.line_status = read_sint16(&f);
	game.line_user_input = read_sint16(&f);
	game.line_min_print = read_sint16(&f);

	/* These are never saved */
	game.cursor_pos = 0;
	game.input_buffer[0] = 0;
	game.echo_buffer[0] = 0;
	game.keypress = 0;

	game.input_mode = read_sint16(&f);
	game.lognum = read_sint16(&f);

	game.player_control = read_sint16(&f);
	game.quit_prog_now = read_sint16(&f);
	game.status_line = read_sint16(&f);
	game.clock_enabled = read_sint16(&f);
	game.exit_all_logics = read_sint16(&f);
	game.picture_shown = read_sint16(&f);
	game.has_prompt = read_sint16(&f);
	game.game_flags = read_sint16(&f);
	game.input_enabled = !read_sint16(&f);

	for (i = 0; i < _HEIGHT; i++)
		game.pri_table[i] = read_uint8(&f);

	if (game.has_window)
		close_window();
	game.msg_box_ticks = 0;
	game.block.active = false;
	/* game.window - fixed by close_window() */
	/* game.has_window - fixed by close_window() */

	game.gfx_mode = read_sint16(&f);
	game.cursor_char = read_uint8(&f);
	game.color_fg = read_sint16(&f);
	game.color_bg = read_sint16(&f);

	/* game.hires (#ifdef USE_HIRES) - rebuilt from image stack */
	/* game.sbuf - rebuilt from image stack */

	/* game.ego_words - fixed by clean_input */
	/* game.num_ego_words - fixed by clean_input */

	game.num_objects = read_sint16(&f);
	for (i = 0; i < (int16) game.num_objects; i++)
		object_set_location(i, read_sint16(&f));

	/* Those are not serialized */
	for (i = 0; i < MAX_DIRS; i++) {
		game.ev_keyp[i].occured = false;
	}

	for (i = 0; i < MAX_STRINGS; i++)
		read_string(&f, game.strings[i]);

	for (i = 0; i < MAX_DIRS; i++) {
		if (read_uint8(&f) & RES_LOADED)
			agi_load_resource(rLOGIC, i);
		else
			agi_unload_resource(rLOGIC, i);
		game.logics[i].sIP = read_sint16(&f);
		game.logics[i].cIP = read_sint16(&f);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (read_uint8(&f) & RES_LOADED)
			agi_load_resource(rPICTURE, i);
		else
			agi_unload_resource(rPICTURE, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (read_uint8(&f) & RES_LOADED)
			agi_load_resource(rVIEW, i);
		else
			agi_unload_resource(rVIEW, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (read_uint8(&f) & RES_LOADED)
			agi_load_resource(rSOUND, i);
		else
			agi_unload_resource(rSOUND, i);
	}

	/* game.pictures - loaded above */
	/* game.logics - loaded above */
	/* game.views - loaded above */
	/* game.sounds - loaded above */

	for (i = 0; i < vt_entries; i++) {
		struct vt_entry *v = &game.view_table[i];

		v->step_time = read_uint8(&f);
		v->step_time_count = read_uint8(&f);
		v->entry = read_uint8(&f);
		v->x_pos = read_sint16(&f);
		v->y_pos = read_sint16(&f);
		v->current_view = read_uint8(&f);

		/* v->view_data - fixed below  */

		v->current_loop = read_uint8(&f);
		v->num_loops = read_uint8(&f);

		/* v->loop_data - fixed below  */

		v->current_cel = read_uint8(&f);
		v->num_cels = read_uint8(&f);

		/* v->cel_data - fixed below  */
		/* v->cel_data_2 - fixed below  */

		v->x_pos2 = read_sint16(&f);
		v->y_pos2 = read_sint16(&f);

		/* v->s - fixed below */

		v->x_size = read_sint16(&f);
		v->y_size = read_sint16(&f);
		v->step_size = read_uint8(&f);
		v->cycle_time = read_uint8(&f);
		v->cycle_time_count = read_uint8(&f);
		v->direction = read_uint8(&f);

		v->motion = read_uint8(&f);
		v->cycle = read_uint8(&f);
		v->priority = read_uint8(&f);

		v->flags = read_uint16(&f);

		v->parm1 = read_uint8(&f);
		v->parm2 = read_uint8(&f);
		v->parm3 = read_uint8(&f);
		v->parm4 = read_uint8(&f);
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
			agi_load_resource(rVIEW, v->current_view);

		set_view(v, v->current_view);	/* Fix v->view_data */
		set_loop(v, v->current_loop);	/* Fix v->loop_data */
		set_cel(v, v->current_cel);	/* Fix v->cel_data */
		v->cel_data_2 = v->cel_data;
		v->s = NULL;	/* not sure if it is used... */
	}

	erase_both();

	/* Clear input line */
	clear_screen(0);
	write_status();

	/* Recreate background from saved image stack */
	clear_image_stack();
	while ((t = read_uint8(&f)) != 0) {
		for (i = 0; i < 7; i++)
			parm[i] = read_sint16(&f);
		replay_image_stack_call(t, parm[0], parm[1], parm[2],
				parm[3], parm[4], parm[5], parm[6]);
	}

	f.close();

	setflag(F_restore_just_ran, true);

	game.has_prompt = 0;	/* force input line repaint if necessary */
	clean_input();

	erase_both();
	blit_both();
	commit_both();
	show_pic();
	do_update();

	return err_OK;
}

#define NUM_SLOTS 12

static int select_slot() {
	int i, key, active = 0;
	int rc = -1;
	int hm = 2, vm = 3;	/* box margins */
	char desc[NUM_SLOTS][40];

	for (i = 0; i < NUM_SLOTS; i++) {
		char name[MAX_PATH];
		Common::File f;
		char sig[8];
		sprintf(name, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, i);
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

	while (42) {
		char dstr[64];
		for (i = 0; i < NUM_SLOTS; i++) {
			sprintf(dstr, "[%-32.32s]", desc[i]);
			print_text(dstr, 0, hm + 1, vm + 4 + i,
					(40 - 2 * hm) - 1, i == active ? MSG_BOX_COLOUR : MSG_BOX_TEXT,
					i == active ? MSG_BOX_TEXT : MSG_BOX_COLOUR);

		}

		poll_timer();	/* msdos driver -> does nothing */
		key = do_poll_keyboard();
		if (!console_keyhandler(key)) {
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
		}
		console_cycle();
	}

press:
	debugC(8, kDebugLevelMain | kDebugLevelInput, "Button pressed: %d", rc);

getout:
	close_window();
	return rc;
}

int savegame_simple() {
	char path[MAX_PATH];

	sprintf(path, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, 0);
	save_game(path, "Default savegame");

	return err_OK;
}

int savegame_dialog() {
	char path[MAX_PATH];
	char *desc;
	char *buttons[] = { "Do as I say!", "I regret", NULL };
	char dstr[200];
	int rc, slot = 0;
	int hm, vm, hp, vp;	/* box margins */
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(path, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, slot);

	draw_window(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	print_text("Select a slot in which you wish to save the game:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	print_text("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = select_slot();
	if (slot < 0)		/* ESC pressed */
		return err_OK;

	/* Get savegame description */
	draw_window(hp, vp + 5 * CHAR_LINES, GFX_WIDTH - hp,
			GFX_HEIGHT - vp - 9 * CHAR_LINES);
	print_text("Enter a description for this game:",
			0, hm + 1, vm + 6, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	draw_rectangle(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES, MSG_BOX_TEXT);
	flush_block(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES);

	get_string(2, 11, 33, MAX_STRINGS);
	print_character(3, 11, game.cursor_char, MSG_BOX_COLOUR, MSG_BOX_TEXT);
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

	sprintf(path, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, slot);
	debugC(8, kDebugLevelMain | kDebugLevelResources, "file is [%s]", path);

	save_game(path, desc);

	message_box("Game saved.");

	return err_OK;
}

int loadgame_simple() {
	char path[MAX_PATH];
	int rc = 0;

	sprintf(path, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, 0);

	erase_both();
	stop_sound();
	close_window();

	if ((rc = load_game(path)) == err_OK) {
		message_box("Game restored.");
		game.exit_all_logics = 1;
		menu_enable_all();
	} else {
		message_box("Error restoring game.");
	}

	return rc;
}

int loadgame_dialog() {
	char path[MAX_PATH];
	int rc, slot = 0;
	int hm, vm, hp, vp;	/* box margins */
	int w;

	hm = 2;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	sprintf(path, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, slot);

	erase_both();
	stop_sound();

	draw_window(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	print_text("Select a game which you wish to\nrestore:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);
	print_text("Press ENTER to select, ESC cancels",
			0, hm + 1, vm + 17, w, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	slot = select_slot();

	if (slot < 0) {
		message_box("Game NOT restored.");
		return err_OK;
	}

	sprintf(path, "%s/%05X_%s_%02d.sav", _savePath, game.crc, game.id, slot);

	if ((rc = load_game(path)) == err_OK) {
		message_box("Game restored.");
		game.exit_all_logics = 1;
		menu_enable_all();
	} else {
		message_box("Error restoring game.");
	}

	return rc;
}

}                             // End of namespace Agi
