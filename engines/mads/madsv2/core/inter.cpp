/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/vocabh.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/lbm.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/extra.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

#define disable_error_check

int extra_display_object;

int stroke_type = STROKE_NONE;          /* Current stroke type           */

int inter_auxiliary_click;              /* Clicks during downtime        */

AnimInterPtr inter_anim = NULL;       /* Background animation script   */

int inter_mouse_x;                      /* Cooked mouse location X       */
int inter_mouse_y;                      /* Cooked mouse location Y       */
int inter_mouse_type;                   /* Hotspot list identifier       */

int inter_object_sprite = 1;
int inter_object_series = -1;

int inter_spinning_objects = true;  /* Flag if objects rotating     */
int inter_animation_running = true;  /* Flag if background animation */

byte *inter_objects_block = NULL;  /* Place for loading object series */
long inter_objects_block_size = 0;

int inter_input_mode = INTER_BUILDING_SENTENCES;

char *inter_dialog_strings[inter_columns];
int       inter_dialog_results[inter_columns];


Verb command[INTER_COMMANDS] = {             /* Main Verb List */
	{ words_look, VERB_THAT, PREP_NONE },
	{ words_take, VERB_THAT, PREP_NONE },
	{ words_push, VERB_THAT, PREP_NONE },
	{ words_open, VERB_THAT, PREP_NONE },
	{ words_put,  VERB_THIS, PREP_RELATIONAL },
	{ words_talk_to, VERB_THAT, PREP_NONE },
	{ words_give, VERB_THIS, PREP_TO },
	{ words_pull, VERB_THAT, PREP_NONE },
	{ words_close, VERB_THAT, PREP_NONE },
	{ words_throw, VERB_THIS, PREP_AT }
};


char istring_prep_names[9][7] = {
	"", "with", "to", "at", "from", "on", "in", "under", "behind"
};

char istring_look_around[12] = "Look around";
char istring_use[5] = "Use ";
char istring_to[4] = "to ";
char istring_walk_to[9] = "Walk to ";
char istring_space[2] = " ";

/* "I like ____. "         */
char istring_object_pronoun[MAX_SYNTAX][8] = {
	"it", "them", "it", "him", "her", "it", "him", "her"
};

/* "_____ look(s) ornery." */
char istring_subject_pronoun[MAX_SYNTAX][8] = {
	"it", "they", "it", "he", "she", "it", "he", "she"
};

/* "I don't see _____ here." */
char istring_demonstrative[MAX_SYNTAX][8] = {
	"that", "those", "that", "him", "her", "it", "him", "her"
};

int inven[INVEN_MAX_OBJECTS];                   /* Inventory index */
int inven_num_objects = 0;                      /* Inventory size  */

int right_command = 0;           /* Right mouse main verb      */
int left_command = -1;          /* Left mouse main verb       */
int left_inven = -1;          /* Left mouse inventory       */
int left_action = -1;          /* Left mouse secondary verb  */
int right_action = -1;          /* Right mouse secondary verb */

int active_inven = -1;          /* The selected inventory item              */
int first_inven = 0;           /* First inventory item displayed on screen */

long scroll_base_time = 0;      /* Scrolling ballistic controls */
int  scroll_active = false;
int  scroll_quickly = false;

int  first_inven_changed = false;       /* Flag if inventory view has changed */
int  picked_word = 0;           /* Word most recently picked.         */

char inter_sentence[64];                /* Sentence building buffer            */
int  inter_sentence_handle = -1;       /* Sentence message handle (for matte) */
int  inter_sentence_changed = false;    /* Mark if sentence contents changed   */

int  inter_look_around;                 /* "Look around" command            */

int  inter_command;                     /* Vocab # of sentence's verb       */
int  inter_main_object;                 /* Vocab # of sentence's #1 object  */
int  inter_second_object;               /* Vocab # of sentence's #2 object  */
int  inter_prep;                        /* Prep # of sentence's preposition */

int  inter_main_syntax;                 /* Syntax # of sentence's #1 object */
int  inter_second_syntax;               /* Syntax # of sentence's #2 object */

byte inter_verb_type;                   /* Grammatical verb form            */
byte inter_prep_type;                   /* Grammatical preposition type     */

int  inter_command_source;              /* Stroke type for verb      */
int  inter_main_object_source;          /* Stroke type for object #1 */
int  inter_second_object_source;        /* Stroke type for object #2 */

int  inter_words[3];                    /* Interface vocabulary words */

int  inter_point_established;           /* Interface clicked point has been established */
int  inter_point_x;                     /* Clicked point in interface */
int  inter_point_y;

int  inter_recent_command;              /* Allows cancellation       */
int  inter_recent_command_source;

int  inter_sentence_ready = false;      /* Flag if a sentence ready to go */

int  inter_awaiting = AWAITING_COMMAND; /* Current awaiting status        */

int  inter_report_hotspots = false;     /* True = show hotspots even with no button pressed */

int  inter_force_rescan = false;        /* Force mouse rescan even if didnt change */
int  inter_base_hotspots;               /* Number of basic hotspots                */

int  inter_no_segments_active = true;
int  inter_some_segments_active = false;

int  inter_spot_class;
int  inter_spot_index;
int  inter_spot_id;

long inter_base_time;                   /* Timing marker for animation    */

int  end_of_selection = false;          /* We are on last pass of this stroke */

byte spot_base[STROKE_MAX];             /* Hotspot list base markers (by stroke type) */


int  scrollbar_active = 0;
int  scrollbar_elevator = 0;
int  scrollbar_old_active = -1;
int  scrollbar_old_elevator = -1;

int  scrollbar_stroke_type = 0;
int  scrollbar_quickly = false;
long scrollbar_base_timing;

int  inter_object_id;
char inter_object_buf[20];
void (*inter_object_routine)() = NULL;

int paul_object_showing = -1;

#define OUAF_OBJECT_X    140
#define OUAF_OBJECT_Y    5


void inter_spin_object(int object_id);
void inter_turn_off_object(void);


void VerbBuf::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(id);
	s.syncAsByte(verb_type);
	s.syncAsByte(prep_type);
}


/**
 * Picks out font colors based on the current mode.
 *
 * @param color_code	Color code
 */
static void inter_set_colors(int color_code) {
	switch (color_code) {
	case NORMAL:
		font_set_colors(-1, 4, 4, 0);
		break;
	case LEFT_SELECT:
		font_set_colors(-1, 5, 5, 0);
		break;
	case RIGHT_SELECT:
		font_set_colors(-1, 6, 6, 0);
		break;
	}
}

/**
 * Returns hot spot coordinates for a particular stroke class and spot #.
 */
static int inter_get_spot(int class_, int id, int *x1, int *y1, int *xs, int *ys) {
	int row, col;
	int base_x, base_y, delta_x;
	int valid_flag;

	valid_flag = false;

	switch (class_) {

	case STROKE_COMMAND:
		row = id % inter_columns;
		col = id / inter_columns;
		base_x = command_base_x;
		base_y = command_base_y;
		delta_x = command_delta_x;
		break;

	case STROKE_INVEN:
		if ((id < first_inven) || (id >= (first_inven + inter_columns))) {
			goto done;
		}
		row = id - first_inven;
		col = 0;
		base_x = inven_base_x;
		base_y = inven_base_y;
		delta_x = inven_delta_x;
		break;

	case STROKE_DIALOG:
		row = id;
		col = 0;
		base_x = command_base_x;
		base_y = command_base_y;
		delta_x = dialog_delta_x;
		break;

	case STROKE_SCROLL:
		row = 0;
		col = 0;
		base_x = inter_scroll_x1;
		base_y = 0;
		delta_x = (inter_scroll_x2 - inter_scroll_x1) + 1;
		if (id == SCROLL_THUMB) {
			base_x += 2;
		}
		break;

	case STROKE_ACTION:
	default:
		row = id;
		col = 0;
		base_x = action_base_x;
		base_y = action_base_y;
		delta_x = action_delta_x;
		break;
	}

	if (col > 0) {
		*x1 = base_x + (col * delta_x);
	} else {
		*x1 = base_x;
	}
	*xs = delta_x;

	*y1 = base_y + (row * inter_delta_y);
	*ys = inter_delta_y;

	if (class_ == STROKE_SCROLL) {
		switch (id) {
		case SCROLL_UP:
			*y1 = inter_up_y1;
			*ys = (inter_up_y2 - inter_up_y1) + 1;
			break;

		case SCROLL_DOWN:
			*y1 = inter_down_y1;
			*ys = (inter_down_y2 - inter_down_y1) + 1;
			break;

		case SCROLL_ELEVATOR:
			*y1 = inter_elevator_y1;
			*ys = (inter_elevator_y2 - inter_elevator_y1) + 1;
			break;

		case SCROLL_THUMB:
			*y1 = inter_thumb_y1 + scrollbar_elevator;
			*ys = 1;
			break;
		}
	}

	valid_flag = true;

done:
	return(valid_flag);
}

/**
 * Writes vocabulary word for the particular class_ & id # to the
 * interface work buffer.
 *
 * @param class_	Class
 * @param id		Id
 */
static void inter_show_word(int class_, int id) {
	int x, y, junk;
	int word_id = 0;
	char temp_buf[80];
	int write_it = true;

	if (!inter_get_spot(class_, id, &x, &y, &junk, &junk)) {
		goto done;
	}

	switch (class_) {
	case STROKE_COMMAND:
		word_id = command[id].id;
		if (id == left_command) {
			inter_set_colors(LEFT_SELECT);
		} else if (id == right_command) {
			inter_set_colors(RIGHT_SELECT);
		} else {
			inter_set_colors(NORMAL);
		}
		break;

	case STROKE_INVEN:
		word_id = object[inven[id]].vocab_id;
		if (id == left_inven) {
			inter_set_colors(LEFT_SELECT);
		} else if (id == active_inven) {
			inter_set_colors(RIGHT_SELECT);
		} else {
			inter_set_colors(NORMAL);
		}
		break;

	case STROKE_DIALOG:
		temp_buf[0] = 0;
		if (inter_dialog_strings[id] != NULL) {
			Common::strcpy_s(temp_buf, inter_dialog_strings[id]);
		}
		if (id == left_command) {
			inter_set_colors(LEFT_SELECT);
		} else {
			inter_set_colors(NORMAL);
		}
		goto write;
		break;

	case STROKE_SCROLL:
		switch (id) {
		case SCROLL_UP:
			Common::strcpy_s(temp_buf, "a");
			break;

		case SCROLL_DOWN:
			Common::strcpy_s(temp_buf, "b");
			break;

		case SCROLL_THUMB:
			Common::strcpy_s(temp_buf, "c");
			break;

		case SCROLL_ELEVATOR:
			Common::strcpy_s(temp_buf, "d");
			break;
		}
		if ((id == SCROLL_THUMB) && (scrollbar_active != SCROLL_ELEVATOR)) {
			inter_set_colors(NORMAL);
		} else {
			inter_set_colors(LEFT_SELECT);
		}
		font_write(font_misc, &scr_inter, temp_buf, x, y, 0);
		goto done;
		break;

	case STROKE_ACTION:
	default:
		word_id = object[inven[active_inven]].verb[id].id;
		if (id == left_action) {
			inter_set_colors(LEFT_SELECT);
		} else if (id == right_action) {
			inter_set_colors(RIGHT_SELECT);
		} else {
			inter_set_colors(NORMAL);
		}
		break;

	}

	Common::strcpy_s(temp_buf, vocab_string(word_id));
	temp_buf[0] = (char)toupper((int)temp_buf[0]);

write:
	if (write_it)
		font_write(font_inter, &scr_inter, temp_buf, x, y, 0);

done:
	;
}

static void inter_show_scrollbar(void) {
	if (scrollbar_active > 0) {
		inter_show_word(STROKE_SCROLL, scrollbar_active);
	}

	inter_show_word(STROKE_SCROLL, SCROLL_THUMB);
}

/**
 * Writes the current inventory list to the interface work buffer.
 */
static void inter_show_all_inven(void) {
	int count;

	for (count = first_inven; (count < (first_inven + inter_columns)) && (count < inven_num_objects); count++) {
		inter_show_word(STROKE_INVEN, count);
	}
}

/**
 * inter_show_all_actions()
 * Writes the current list of object-specific verbs to the interface
 * work buffer.
 */
static void inter_show_all_actions(void) {
	int count;
	int id;

	if (active_inven >= 0) {
		for (count = 0; count < (int)object[inven[active_inven]].num_verbs; count++) {
			id = object[inven[active_inven]].vocab_id;
			id = object_named(id);
			// id = object[inven[active_inven]].verb[count].count;
			if (id == 8) {  // pid doll
				if (global[86]) {  // heal_verbs_visible
					inter_show_word(STROKE_ACTION, count);
				} else if (count == 0) {
					inter_show_word(STROKE_ACTION, count);
				}

			} else {
				inter_show_word(STROKE_ACTION, count);
			}
		}
	}
}

static void inter_show_all_dialog(void) {
	int count;

	for (count = 0; count < inter_columns; count++) {
		if (inter_dialog_strings[count] != NULL) {
			inter_show_word(STROKE_DIALOG, count);
		}
	}
}

void inter_prepare_background(void) {
	int count;

	inter_auxiliary_click = false;

	if (inter_input_mode == INTER_BUILDING_SENTENCES) {

		for (count = 0; count < INTER_COMMANDS; count++) {
			inter_show_word(STROKE_COMMAND, count);
		}

		inter_show_all_inven();
		inter_show_all_actions();
		inter_show_scrollbar();

	} else if (inter_input_mode == INTER_CONVERSATION) {

		inter_show_all_dialog();

	}
}

static void inter_refresh(void) {
	// int count;
	// for (count = 0; count < (int)image_inter_marker; count++) {
	// if (image_inter_list[count].segment_id == INTER_SPINNING_OBJECT) {
	// image_inter_list[count].flags = IMAGE_REFRESH + IMAGE_UPDATE_ONLY;
	// }
	// }
	image_inter_marker = 0;
	matte_refresh_inter();

	matte_inter_frame(false, false);

	inter_prepare_background();
}

void inter_reset_dialog(void) {
	int count;

	for (count = 0; count < inter_columns; count++) {
		inter_dialog_strings[count] = NULL;
		inter_dialog_results[count] = -1;
	}
}

int inter_add_dialog(char *string, int result) {
	int count, id;
	char *mark;
	char *mark2;

	do {  // Double hyphens to dashes
		mark = strstr(string, "--");
		if (mark != NULL) {
			*mark = '{';
			mark++;
			mark2 = mark + 1;
			Common::strcpy_s(mark, 65536, mark2);
		}
	} while (mark != NULL);

	id = -1;
	for (count = 0; (id < 0) && (count < inter_columns); count++) {
		if (inter_dialog_strings[count] == NULL) {
			id = count;
		}
	}

	if (id >= 0) {
		inter_dialog_strings[id] = string;
		inter_dialog_results[id] = result;
	}

	return (id);
}

/*
 * Updates the specified region of the interface on the live screen.
 *
 * @param x1		Top-left X
 * @param y1		Top-left Y
 * @param xs		X size
 * @param ys		Y size
 */
static void inter_update(int x1, int y1, int xs, int ys) {
	int refresh_flag;
	int y1a;

	mouse_set_work_buffer(scr_inter.data, scr_inter.x);
	mouse_set_view_port_loc(0, inter_viewing_at_y, video_x - 1, inter_viewing_at_y + scr_inter.y - 1);
	mouse_set_view_port(0, 0);

	mouse_freeze();
	refresh_flag = mouse_refresh_view_port();

	y1a = y1 + inter_base_y;

	video_update(&scr_inter, x1, y1,
		x1 + inter_base_x, y1a,
		xs, ys);

#ifdef sixteen_colors
	if (video_mode == ega_mode) {
		video_flush_ega(y1a, (y1a + ys - 1));
	}
#endif

	if (refresh_flag) mouse_refresh_done();
	mouse_thaw();
}


static void inter_image(int x1, int y1, int xs, int ys) {
	if (image_inter_marker < IMAGE_INTER_LIST_SIZE) {
		image_inter_list[image_inter_marker].flags = IMAGE_OVERPRINT;
		image_inter_list[image_inter_marker].segment_id = INTER_TEXT_UPDATE;
		image_inter_list[image_inter_marker].x = x1;
		image_inter_list[image_inter_marker].y = (byte)y1;
		image_inter_list[image_inter_marker].sprite_id = (byte)xs;
		image_inter_list[image_inter_marker].series_id = (byte)ys;
		image_inter_marker++;
	}
#ifndef disable_error_check
	else {
		error_report(ERROR_IMAGE_INTER_LIST_FULL, WARNING, MODULE_INTER, IMAGE_INTER_LIST_SIZE, 1);
	}
#endif
}

static void inter_scrollbar_refresh(void) {
	int x1, y1, xs, ys;

	x1 = inter_scroll_x1;
	y1 = inter_up_y1;
	xs = (inter_scroll_x2 - inter_scroll_x1) + 1;
	ys = (inter_down_y2 - inter_up_y1) + 1;

	inter_image(x1, y1, xs, ys);

	matte_inter_frame(false, false);

	inter_show_scrollbar();

	inter_update(x1, y1, xs, ys);
}

/**
 * Sets an active word of the specific class_ to the "new" value.
 * "old" is a pointer to the main control variable for that value,
 * which also contains the old value of that variable.  The old
 * word is "turned off" automatically.
 */
static void inter_set_active_word(int class_, int *old, int new_) {
	int x1, y1, xs, ys;
	int temp;

	if ((class_ == STROKE_INVEN) && first_inven_changed) {
		*old = new_;
		x1 = inven_base_x;
		y1 = inven_base_y;
		xs = inven_delta_x;
		ys = (inter_delta_y * inter_columns);

		inter_image(x1, y1, xs, ys);

		matte_inter_frame(false, false);

		inter_show_all_inven();
		inter_update(x1, y1, xs, ys);

		first_inven_changed = false;

		if (inven_num_objects <= 1) {
			scrollbar_elevator = 0;
		} else {
			scrollbar_elevator = (first_inven * inter_thumb_range) / (inven_num_objects - 1);
			scrollbar_elevator = MIN(scrollbar_elevator, inter_thumb_range - 1);
		}
	} else {
		temp = *old;
		*old = new_;

		if (temp >= 0) {
			inter_show_word(class_, temp);
			if (inter_get_spot(class_, temp, &x1, &y1, &xs, &ys)) {
				inter_update(x1, y1, xs, ys);
			}
		}

		if (new_ >= 0) {
			inter_show_word(class_, new_);
			if (inter_get_spot(class_, new_, &x1, &y1, &xs, &ys)) {
				inter_update(x1, y1, xs, ys);
			}
		}
	}
}

void inter_set_active_inven(int new_active) {
	int new_verbs, old_verbs, max_verbs;
	int x1, y1, xs, ys;

	if ((new_active != active_inven) || first_inven_changed) {

		if ((kernel_mode == KERNEL_ACTIVE_CODE) && (inter_input_mode == INTER_BUILDING_SENTENCES)) {
			if (active_inven >= 0) {
				old_verbs = object[inven[active_inven]].num_verbs;
			} else {
				old_verbs = 0;
			}

			if (new_active >= 0) {
				new_verbs = object[inven[new_active]].num_verbs;
			} else {
				new_verbs = 0;
			}

			max_verbs = MAX(old_verbs, new_verbs);

			inter_set_active_word(STROKE_INVEN, &active_inven, new_active);

			left_action = -1;
			right_action = -1;

			if (max_verbs > 0) {
				x1 = action_base_x;
				y1 = action_base_y;
				xs = action_delta_x;
				ys = (inter_delta_y * max_verbs);



				if (extra_display_object) {
					if (image_inter_marker < IMAGE_INTER_LIST_SIZE) {
						image_inter_list[image_inter_marker].flags = IMAGE_OVERPRINT;
						image_inter_list[image_inter_marker].x = x1;
						image_inter_list[image_inter_marker].y = (byte)y1;
						image_inter_list[image_inter_marker].sprite_id = (byte)xs;
						image_inter_list[image_inter_marker].series_id = (byte)ys;
						image_inter_marker++;
					}
#ifndef disable_error_check
					else {
						error_report(ERROR_IMAGE_INTER_LIST_FULL, WARNING, MODULE_INTER, IMAGE_INTER_LIST_SIZE, 2);
					}
#endif

					matte_inter_frame(false, false);

					inter_show_all_actions();
					inter_update(x1, y1, xs, ys);
				}
			}
		}
	}



	if (new_active != -1) {
		if (extra_display_object) {
			inter_spin_object(inven[new_active]);
			mcga_setpal_range(&master_palette, 7, 1);
			mcga_setpal_range(&master_palette, 246, 2);
		}
	} else {
		inter_turn_off_object();
	}
}

static void inter_purge_inven_stroke(void) {
	left_inven = -1;
	mouse_init_cycle();
	stroke_type = STROKE_NONE;
}

void inter_give_to_player(int this_) {
	if (inven_num_objects >= INVEN_MAX_OBJECTS) {
		error_report(ERROR_PLAYER_INVENTORY_FULL, ERROR, MODULE_INTER, this_, INVEN_MAX_OBJECTS);
	}

	if (player_has(this_)) goto done;

	inven[inven_num_objects] = this_;

	// if (!inven_num_objects) {
	active_inven = inven_num_objects;

	// }
	first_inven = MAX(0, first_inven);

	if (first_inven > inven_num_objects) first_inven = inven_num_objects;

	if ((first_inven + inter_columns) <= inven_num_objects) {
		first_inven = inven_num_objects - (inter_columns - 1);
	}

	first_inven_changed = true;

	object[this_].location = PLAYER;

	inven_num_objects++;

	// Paul moved this from below
done:
	;

	// Paul here
	if ((kernel_mode == KERNEL_ACTIVE_CODE) &&
		(inter_input_mode == INTER_BUILDING_SENTENCES ||
			inter_input_mode == INTER_LIMITED_SENTENCES)) {
		inter_purge_inven_stroke();
		inter_set_active_inven(active_inven);
	}
}

void inter_take_from_player(int this_, int where_to) {
	int id = -1;
	int count;
	int active;
	int was_negative;

	for (count = 0; count < inven_num_objects; count++) {
		if (inven[count] == this_) {
			id = count;
		}
	}

	if (id < 0) {
		error_report(ERROR_NO_SUCH_OBJECT, WARNING, MODULE_INTER, id, inven_num_objects);
	} else {
		active = active_inven;
		was_negative = (active < 0);
		if ((kernel_mode == KERNEL_ACTIVE_CODE) &&
			(inter_input_mode == INTER_BUILDING_SENTENCES)) {
			inter_set_active_inven(-1);
		}
		for (count = id; count < inven_num_objects - 1; count++) {
			inven[count] = inven[count + 1];
		}
		inven_num_objects--;
		if (!was_negative) {
			if (active >= id) active--;
			if ((active < 0) && (inven_num_objects > 0)) active = 0;
		}
		if (first_inven >= id) first_inven = MAX(0, first_inven - 1);
		first_inven_changed = true;
		object[this_].location = where_to;
		if ((kernel_mode == KERNEL_ACTIVE_CODE) && (inter_input_mode == INTER_BUILDING_SENTENCES)) {
			inter_purge_inven_stroke();
			inter_set_active_inven(active);
		} else {
			active_inven = active;
			if (active_inven != -1) {
				inter_spin_object(inven[active_inven]);
				mcga_setpal_range(&master_palette, 7, 1);
				mcga_setpal_range(&master_palette, 246, 2);
			} else {
				inter_turn_off_object();
			}
		}
	}
}

void inter_move_object(int object_id, int location) {

	if (object_id == paul_object_showing &&
		location == 2) goto done;

	if (object[object_id].location == PLAYER) {
		inter_take_from_player(object_id, NOWHERE);
	}
	if (location == PLAYER) {
		inter_give_to_player(object_id);
	} else {
		object[object_id].location = location;
	}

	// extra_blank_knothole();
done:
	;

	extra_display_object = false;
}

/**
 * Takes a set of hotspot coordinates that is interface window relative
 * and converts it to absolute screen coordinates.
 */
static void inter_spot_correct(int *x1, int *y1, int *x2, int *y2, int xs, int ys) {
	*x1 += inter_base_x;
	*x2 = *x1 + xs - 1;
	*y1 += inter_base_y;
	*y2 = *y1 + ys - 1;
}

void inter_setup_hotspots(void) {
	int count;
	int x1, y1, xs, ys;
	int x2, y2;

	numspots = 0;

	if (inter_input_mode == INTER_BUILDING_SENTENCES) {

		spot_base[STROKE_SCROLL - 1] = (byte)(numspots + 1);
		for (count = SCROLL_UP; count <= SCROLL_ELEVATOR; count++) {
			inter_get_spot(STROKE_SCROLL, count, &x1, &y1, &xs, &ys);
			inter_spot_correct(&x1, &y1, &x2, &y2, xs, ys);
			hspot_add(x1, y1, x2, y2, STROKE_SCROLL, count, ABSOLUTE_MODE);
		}

		spot_base[STROKE_COMMAND - 1] = (byte)(numspots + 1);
		for (count = 0; count < INTER_COMMANDS; count++) {
			inter_get_spot(STROKE_COMMAND, count, &x1, &y1, &xs, &ys);
			inter_spot_correct(&x1, &y1, &x2, &y2, xs, ys);
			hspot_add(x1, y1, x2, y2, STROKE_COMMAND, count, ABSOLUTE_MODE);
		}

		spot_base[STROKE_INVEN - 1] = (byte)(numspots + 1);
		for (count = 0; count < inter_columns; count++) {
			inter_get_spot(STROKE_INVEN, count + first_inven, &x1, &y1, &xs, &ys);
			inter_spot_correct(&x1, &y1, &x2, &y2, xs, ys);
			hspot_add(x1, y1, x2, y2, STROKE_INVEN, count, ABSOLUTE_MODE);
		}

		spot_base[STROKE_ACTION - 1] = (byte)(numspots + 1);
		for (count = 0; count < inter_columns; count++) {
			inter_get_spot(STROKE_ACTION, count, &x1, &y1, &xs, &ys);
			inter_spot_correct(&x1, &y1, &x2, &y2, xs, ys);
			hspot_add(x1, y1, x2, y2, STROKE_ACTION, count, ABSOLUTE_MODE);
		}

		spot_base[STROKE_SPECIAL_INVEN - 1] = (byte)(numspots + 1);
		x1 = inter_base_x + inter_object_base_x;
		y1 = inter_base_y + inter_object_base_y;
		x2 = x1 + inter_object_size_x - 1;
		y2 = y1 + inter_object_size_y - 1;
		hspot_add(x1, y1, x2, y2, STROKE_SPECIAL_INVEN, 0, ABSOLUTE_MODE);
	}

	if ((inter_input_mode == INTER_BUILDING_SENTENCES) ||
		(inter_input_mode == INTER_LIMITED_SENTENCES)) {
		spot_base[STROKE_INTERFACE - 1] = (byte)(numspots + 1);
		for (count = room_num_spots - 1; count >= 0; count--) {
			hspot_add(room_spots[count].ul_x, room_spots[count].ul_y,
				room_spots[count].lr_x, room_spots[count].lr_y,
				STROKE_INTERFACE, count, RELATIVE_MODE);
			if (!room_spots[count].active) spot[numspots].active = false;
		}
	}


	if (inter_input_mode == INTER_CONVERSATION) {
		spot_base[STROKE_DIALOG - 1] = (byte)(numspots + 1);
		for (count = 0; count < inter_columns; count++) {
			inter_get_spot(STROKE_DIALOG, count, &x1, &y1, &xs, &ys);
			inter_spot_correct(&x1, &y1, &x2, &y2, xs, ys);
			hspot_add(x1, y1, x2, y2, STROKE_DIALOG, count, ABSOLUTE_MODE);
		}
	}

	inter_base_hotspots = numspots;
}

/**
 * Handles up/down "dragging" for inventory list.
 */
static void inter_drag_check(void) {
	long timing_level;
	long current_time;

	if (mouse_any_stroke && ((inter_mouse_y <= (inter_base_y + inven_base_y)) || (inter_mouse_y == 199))) {
		timing_level = (long)((scroll_quickly) ? MOUSE_TIMING_TWO : MOUSE_TIMING_ONE);
		current_time = timer_read_dos();
		scroll_active = true;
		if ((current_time - scroll_base_time) >= timing_level) {
			scroll_base_time = current_time;
			scroll_quickly = true;
			if (inter_mouse_y == (video_y - 1)) {
				if (first_inven < (inven_num_objects - 1)) {
					first_inven++;
					first_inven_changed = true;
				}
			} else {
				if (first_inven > 0) {
					first_inven--;
					first_inven_changed = true;
				}
			}
		}
	} else {
		scroll_active = false;
		scroll_quickly = false;
	}
}

/**
 * Determines which word is being picked off the screen, and makes
 * it active.
 */
static void inter_select_word(void) {
	int x1, y1, x2, y2;
	int junk;
	int max_x, max_y;
	int count, new_;
	int quantity;
	int mode;
	int limit = 0;
	int strict, delta;
	int tight_boxes;
	int difference = 0;
	int *selection;
	int base_spot, this_spot;
	int paul_id;

	switch (stroke_type) {
	case STROKE_COMMAND:
		quantity = INTER_COMMANDS;
		limit = INTER_COMMANDS - 1;
		strict = inter_columns;
		delta = 0;
		selection = mouse_button ? &right_command : &left_command;
		if (mouse_button && (right_action >= 0)) {
			inter_set_active_word(STROKE_ACTION, &right_action, -1);
		}
		// tight_boxes = (end_of_selection && !mouse_button);
		tight_boxes = true;
		break;

	case STROKE_INVEN:
		inter_drag_check();
		quantity = MIN(inter_columns, (inven_num_objects - first_inven));
		limit = inven_num_objects - 1;
		strict = 0;
		delta = first_inven;
		selection = &left_inven;
		// tight_boxes = (end_of_selection && ((!mouse_any_stroke) || !(inter_awaiting == AWAITING_COMMAND)));
		tight_boxes = true;
		break;

	case STROKE_ACTION:
		if (active_inven >= 0) {
			paul_id = object[inven[active_inven]].vocab_id;
			paul_id = object_named(paul_id);

			if (paul_id == 8 && !global[86]) {  // pid doll / global [heal_verbs_visible]
				quantity = 1;
			} else {
				quantity = object[inven[active_inven]].num_verbs;
			}

			limit = quantity - 1;

		} else {
			quantity = 0;
		}
		strict = 0;
		delta = 0;
		selection = mouse_button ? &right_action : &left_action;
		if (mouse_button && (right_command >= 0)) {
			inter_set_active_word(STROKE_COMMAND, &right_command, -1);
		}
		// tight_boxes = end_of_selection && !mouse_button;
		tight_boxes = true;
		break;

	case STROKE_SPECIAL_INVEN:
		quantity = 1;
		limit = inven_num_objects - 1;
		strict = 0;
		delta = active_inven;
		selection = &junk;
		tight_boxes = true;
		break;

	case STROKE_DIALOG:
		quantity = 0;
		for (count = 0; count < inter_columns; count++) {
			if (inter_dialog_strings[count] != NULL) {
				quantity++;
			}
		}
		limit = quantity - 1;
		strict = 0;
		delta = 0;
		selection = &left_command;
		tight_boxes = true;
		break;

	case STROKE_INTERFACE:
	default:
		difference = (numspots - inter_base_hotspots);
		quantity = room_num_spots + difference;
		limit = quantity - 1;
		strict = 0;
		delta = 0;
		selection = &junk;
		tight_boxes = true;
		break;
	}

	new_ = -1;
	max_y = 0;
	max_x = 0;
	base_spot = spot_base[stroke_type - 1];


	for (count = 0; (count < quantity) && (new_ < 0); count++) {
		if (stroke_type == STROKE_INTERFACE) {
			this_spot = base_spot + (quantity - (count + 1));
			// if (count >= difference) {
			// this_spot = base_spot + (room_num_spots - ((count - difference) + 1));
			// } else {
			// this_spot = base_spot + room_num_spots + count;
			// }
		} else {
			this_spot = base_spot + count;
		}
		x1 = spot[this_spot].ul_x;
		y1 = spot[this_spot].ul_y;
		x2 = spot[this_spot].lr_x;
		y2 = spot[this_spot].lr_y;
		mode = spot[this_spot].video_mode;
		max_y = MAX(max_y, y2);
		max_x = MAX(max_x, x1);
		if (spot[this_spot].active && (mode == inter_mouse_type)) {
			if ((inter_mouse_y >= y1) && (inter_mouse_y <= y2)) {
				if (tight_boxes) {
					if (((inter_mouse_x) >= x1) && ((inter_mouse_x) <= x2)) {
						new_ = this_spot - base_spot;
						if (stroke_type == STROKE_INTERFACE) {
							if (new_ < room_num_spots) {
								new_ = room_num_spots - (new_ + 1);
							}
						}
					}
				} else {
					if (strict) {
						if (count < strict) {
							if ((inter_mouse_x) <= x2) {
								new_ = count;
							}
						} else {
							if ((inter_mouse_x) >= x1) {
								new_ = count;
							}
						}
					} else {
						new_ = count;
					}
				}
			}
		}
	}

	if ((new_ < 0) && (quantity > 0) && (!tight_boxes)) {
		if (inter_mouse_y > max_y) {
			new_ = quantity - 1;
		} else {
			new_ = 0;
			if (strict && ((inter_mouse_x) >= max_x)) {
				new_ = strict;
			}
		}
	}

	if (new_ >= 0) {
		new_ = MIN((new_ + delta), limit);
	}

	picked_word = new_;

	if (((stroke_type == STROKE_INVEN) || (stroke_type == STROKE_SPECIAL_INVEN)) && (inter_awaiting == AWAITING_COMMAND) && (new_ >= 0)) {
		if (end_of_selection && (!mouse_stop_stroke || !inter_report_hotspots)) new_ = -1;
	}

	if (end_of_selection && !mouse_button && (!mouse_stop_stroke || !inter_report_hotspots)) {
		new_ = -1;
	}

	if ((stroke_type != STROKE_INTERFACE) && (stroke_type != STROKE_SPECIAL_INVEN)) {
		inter_set_active_word(stroke_type, selection, new_);
	}
}

static void inter_scroll_bar(void) {
	int junk;
	int y, new_first_inven;
	long now_time;
	long timing_level;

	if (inter_input_mode != INTER_BUILDING_SENTENCES) goto done;

	scrollbar_active = 0;

	if ((inter_spot_class == STROKE_SCROLL) ||
		((scrollbar_old_active == SCROLL_ELEVATOR) && mouse_stroke_going)) {
		if (mouse_stroke_going || inter_report_hotspots) {

			if (mouse_start_stroke || (inter_report_hotspots && !mouse_stroke_going)) {
				if (inter_spot_class == STROKE_SCROLL) {
					scrollbar_stroke_type = inter_spot_id;
				}
			}

			if ((scrollbar_stroke_type == inter_spot_id) ||
				(scrollbar_old_active == SCROLL_ELEVATOR)) {
				scrollbar_active = scrollbar_stroke_type;

				now_time = timer_read_dos();
				timing_level = scrollbar_quickly ? MOUSE_TIMING_TWO : MOUSE_TIMING_ONE;

				if (mouse_start_stroke || (mouse_stroke_going && (now_time > (scrollbar_base_timing + timing_level)))) {

					scrollbar_quickly = !mouse_start_stroke;
					scrollbar_base_timing = timer_read_dos();

					switch (scrollbar_stroke_type) {
					case SCROLL_UP:
						if ((first_inven > 0) && inven_num_objects) {
							first_inven--;
							first_inven_changed = true;
						}
						break;

					case SCROLL_DOWN:
						if ((first_inven < (inven_num_objects - 1)) && inven_num_objects) {
							first_inven++;
							first_inven_changed = true;
						}
						break;

					case SCROLL_ELEVATOR:
						y = inter_mouse_y;
						y -= inter_base_y;
						y -= inter_thumb_y1;
						y = MAX(0, y);
						y = MIN(inter_thumb_range - 1, y);
						new_first_inven = (y * inven_num_objects) / inter_thumb_range;
						new_first_inven = MIN(inven_num_objects - 1, new_first_inven);
						if (inven_num_objects) {
							first_inven_changed = (new_first_inven != first_inven);
							first_inven = new_first_inven;
						}
						break;
					}

					if (first_inven_changed) {
						inter_set_active_word(STROKE_INVEN, &junk, 0);
					}
				}
			}
		}
	}

	if ((scrollbar_active != scrollbar_old_active) ||
		(scrollbar_elevator != scrollbar_old_elevator)) {
		inter_scrollbar_refresh();
	}

	scrollbar_old_active = scrollbar_active;
	scrollbar_old_elevator = scrollbar_elevator;

done:
	;
}

void inter_init_sentence(void) {
	inter_awaiting = AWAITING_COMMAND;

	inter_command_source = STROKE_NONE;
	inter_main_object_source = STROKE_NONE;
	inter_second_object_source = STROKE_NONE;
	inter_recent_command_source = STROKE_NONE;

	inter_command = -1;
	inter_main_object = -1;
	inter_second_object = -1;
	inter_recent_command = -1;
	inter_prep = PREP_NONE;

	inter_look_around = false;

	inter_verb = -1;
	inter_main_noun = -1;
	inter_second_noun = -1;

	inter_point_established = false;

	inter_sentence[0] = 0;
	inter_sentence_changed = true;
}

/**
 * Adds the text for a vocabulary word to the current sentence.
 *
 * @param vocab_id		Vocabulary Id
 * @param capitalize	Capitalization flag
 */
static void inter_add_word_to_sentence(int vocab_id, int capitalize) {
	int  len;

	len = strlen(inter_sentence);
	Common::strcat_s(inter_sentence, vocab_string(vocab_id));

	if (capitalize) {
		inter_sentence[len] = (byte)toupper(inter_sentence[len]);
	}

	Common::strcat_s(inter_sentence, istring_space);
}

/**
 * Puts together a string for the current sentence, based on the
 * information currently available.
 */
static void inter_compile_sentence(void) {
	int prep_special;
	int verb;
	int len;
	char *mark;
	ObjectPtr my_object;

	prep_special = false;

	inter_sentence[0] = 0;
	inter_verb = -1;
	inter_main_noun = -1;
	inter_second_noun = -1;

	if (inter_command_source == STROKE_DIALOG) {
		if (inter_command >= 0) {
			inter_verb = inter_dialog_results[inter_command];
			if (inter_dialog_strings[inter_command] != NULL) {
				Common::strcpy_s(inter_sentence, inter_dialog_strings[inter_command]);
			}
		}
		goto done;
	}

	if (inter_look_around && (inter_command == 0)) {
		Common::strcat_s(inter_sentence, istring_look_around);
		goto done;
	}

	if ((inter_command_source == STROKE_ACTION) && (inter_command >= 0) &&
		(inter_verb_type == VERB_THAT) && (inter_prep_type == PREP_NONE)) {


		my_object = &object[inven[active_inven]];
		inter_main_noun = my_object->vocab_id;
		inter_main_syntax = my_object->syntax;
		inter_verb = my_object->verb[inter_command].id;

		Common::strcat_s(inter_sentence, istring_use);
		inter_add_word_to_sentence(inter_main_noun, false);
		Common::strcat_s(inter_sentence, istring_to);
		inter_add_word_to_sentence(inter_verb, false);

	} else {

		if (inter_command >= 0) {
			if (inter_command_source == STROKE_COMMAND) {
				inter_verb = command[inter_command].id;

			} else {
				inter_verb = object[inven[active_inven]].verb[inter_command].id;
			}

			inter_add_word_to_sentence(inter_verb, true);
			if (inter_verb == words_look) {
				// inter_prep = PREP_AT;
				Common::strcat_s(inter_sentence, istring_prep_names[PREP_AT]);
				Common::strcat_s(inter_sentence, istring_space);
			}
		}

		if ((inter_main_object >= 0) && (inter_command >= 0) && (inter_prep > 0) && (inter_verb_type == VERB_THAT)) {
			prep_special = true;
			Common::strcat_s(inter_sentence, istring_prep_names[inter_prep]);
			Common::strcat_s(inter_sentence, istring_space);
		}

		if (inter_main_object >= 0) {
			if (inter_command < 0) {
				verb = (inter_main_object < room_num_spots) ?
					room_spots[inter_main_object].verb :
					kernel_dynamic_hot[kernel_dynamic_consecutive(inter_main_object - room_num_spots)].verb_id;
				if (verb <= 0) {
					inter_verb = words_walk_to;
					Common::strcat_s(inter_sentence, istring_walk_to);
				} else {
					inter_verb = verb;
					inter_add_word_to_sentence(inter_verb, true);
				}
			}
			if ((inter_main_object_source == STROKE_INVEN) || (inter_main_object_source == STROKE_SPECIAL_INVEN)) {
				my_object = &object[inven[inter_main_object]];
				inter_main_noun = my_object->vocab_id;
				inter_main_syntax = my_object->syntax;
			} else {
				inter_main_noun = (inter_main_object < room_num_spots) ?
					room_spots[inter_main_object].vocab :
					kernel_dynamic_hot[kernel_dynamic_consecutive(inter_main_object - room_num_spots)].vocab_id;

				inter_main_syntax = (inter_main_object < room_num_spots) ?
					room_spots[inter_main_object].syntax :
					kernel_dynamic_hot[kernel_dynamic_consecutive(inter_main_object - room_num_spots)].syntax;

			}
			inter_add_word_to_sentence(inter_main_noun, false);
		}
	}

	if (inter_second_object >= 0) {
		if ((inter_second_object_source == STROKE_INVEN) || (inter_second_object_source == STROKE_SPECIAL_INVEN)) {
			my_object = &object[inven[inter_second_object]];
			inter_second_noun = my_object->vocab_id;
			inter_second_syntax = my_object->syntax;
		} else {
			inter_second_noun = (inter_second_object < room_num_spots) ?
				room_spots[inter_second_object].vocab :
				kernel_dynamic_hot[kernel_dynamic_consecutive(inter_second_object - room_num_spots)].vocab_id;
			inter_second_syntax = (inter_second_object < room_num_spots) ?
				room_spots[inter_second_object].syntax :
				kernel_dynamic_hot[kernel_dynamic_consecutive(inter_second_object - room_num_spots)].syntax;
		}
	}

	if ((inter_main_object >= 0) && (inter_prep > 0) && (!prep_special)) {
		if (inter_prep != PREP_RELATIONAL) {
			Common::strcat_s(inter_sentence, istring_prep_names[inter_prep]);
			Common::strcat_s(inter_sentence, istring_space);
		} else {
			if (inter_second_object >= 0) {
				if ((inter_second_object_source == STROKE_INVEN) || (inter_second_object_source == STROKE_SPECIAL_INVEN)) {
					inter_prep = object[inven[inter_second_object]].prep;
					Common::strcat_s(inter_sentence, istring_prep_names[inter_prep]);
				} else {
					inter_prep = (inter_second_object < room_num_spots) ?
						room_spots[inter_second_object].prep :
						kernel_dynamic_hot[kernel_dynamic_consecutive(inter_second_object - room_num_spots)].prep;
					Common::strcat_s(inter_sentence, istring_prep_names[inter_prep]);
				}
				Common::strcat_s(inter_sentence, istring_space);

				if (!mouse_any_stroke) inter_prep = PREP_RELATIONAL;
			}
		}
	}

	if (inter_second_object >= 0) {
		inter_add_word_to_sentence(inter_second_noun, false);
	}

	len = strlen(inter_sentence);
	if (strlen(inter_sentence)) {
		mark = &inter_sentence[len - 1];
		if (*mark == ' ') *mark = 0;
	}

done:
	inter_sentence_changed = true;
}

/**
 * Analyzes the potential effect of the current stroke on the sentence --
 * w/ the proviso that the stroke is not yet finished.
 */
static void inter_analyze_stroke(void) {
	if ((stroke_type == STROKE_COMMAND) || (stroke_type == STROKE_ACTION)) {
		if ((inter_awaiting != AWAITING_COMMAND) && (picked_word >= 0)) {
			if (!((stroke_type == inter_recent_command_source) && (picked_word == inter_recent_command) && ((inter_awaiting == AWAITING_THIS) || (stroke_type == STROKE_ACTION)))) {
				inter_init_sentence();
			} else {
				inter_look_around = ((inter_command == 0) && (stroke_type == STROKE_COMMAND));
			}
		}
	}

	if (mouse_button && mouse_any_stroke) {
		switch (stroke_type) {
		case STROKE_COMMAND:
		case STROKE_ACTION:
			goto done;
			break;

		case STROKE_INVEN:
		case STROKE_INTERFACE:
		case STROKE_SPECIAL_INVEN:
			if (inter_awaiting != AWAITING_THAT) {
				if (right_command >= 0) {
					inter_command_source = STROKE_COMMAND;
					inter_command = right_command;
					inter_verb_type = command[inter_command].verb_type;
					inter_prep_type = command[inter_command].prep_type;
					inter_awaiting = AWAITING_THIS;
				} else if (right_action >= 0) {
					inter_command_source = STROKE_ACTION;
					inter_command = right_action;
					inter_verb_type = object[inven[active_inven]].verb[right_action].verb_type;
					inter_prep_type = object[inven[active_inven]].verb[right_action].prep_type;
					inter_main_object_source = STROKE_INVEN;
					inter_main_object = active_inven;
					inter_prep = inter_prep_type;
					if ((inter_verb_type == VERB_THIS) && (inter_prep_type == PREP_NONE)) {
						inter_awaiting = AWAITING_RIGHT_MOUSE;
					} else if ((inter_verb_type == VERB_THAT) && (inter_prep_type != PREP_NONE)) {
						inter_awaiting = AWAITING_RIGHT_MOUSE;
					} else {
						inter_awaiting = AWAITING_THAT;
					}
				}
			}
			break;
		}
	}

	switch (inter_awaiting) {

	case AWAITING_COMMAND:
		inter_prep = PREP_NONE;
		switch (stroke_type) {

		case STROKE_COMMAND:
			inter_command_source = STROKE_COMMAND;
			inter_command = picked_word;
			if (inter_command >= 0) {
				inter_verb_type = command[inter_command].verb_type;
				inter_prep_type = command[inter_command].prep_type;
			}
			break;

		case STROKE_DIALOG:
			inter_command_source = STROKE_DIALOG;
			inter_command = picked_word;
			break;

		case STROKE_INVEN:
		case STROKE_SPECIAL_INVEN:
			break;

		case STROKE_ACTION:
			inter_command_source = STROKE_ACTION;
			inter_command = picked_word;

			if (inter_command >= 0) {
				inter_verb_type = object[inven[active_inven]].verb[inter_command].verb_type;
				inter_prep_type = object[inven[active_inven]].verb[inter_command].prep_type;
				inter_main_object = active_inven;
				inter_main_object_source = STROKE_INVEN;

				if (inter_verb_type == VERB_THAT) {
					inter_prep = inter_prep_type;
				}
			} else {
				inter_main_object = -1;
				inter_main_object_source = STROKE_NONE;
			}
			break;

		case STROKE_INTERFACE:
			inter_command = -1;
			inter_command_source = STROKE_NONE;
			inter_main_object_source = STROKE_INTERFACE;
			inter_main_object = picked_word;
			break;
		}
		break;

	case AWAITING_THIS:
		inter_prep = PREP_NONE;
		switch (stroke_type) {
		case STROKE_INTERFACE:
		case STROKE_INVEN:
		case STROKE_SPECIAL_INVEN:
			inter_main_object_source = stroke_type;
			inter_main_object = picked_word;
			break;
		}
		break;

	case AWAITING_THAT:
		switch (stroke_type) {
		case STROKE_INTERFACE:
		case STROKE_INVEN:
		case STROKE_SPECIAL_INVEN:
			inter_second_object_source = stroke_type;
			inter_second_object = picked_word;
			break;
		}
		break;

	case AWAITING_RIGHT_MOUSE:
		break;
	}

done:
	;
}

/**
 * Determines the final effect of a stroke on the sentence, after
 * that stroke has been completed.
 */
static void inter_complete_stroke(void) {
	int demand_abort;

	demand_abort = false;

	if ((stroke_type == STROKE_COMMAND) || (stroke_type == STROKE_ACTION)) {
		if ((inter_awaiting != AWAITING_COMMAND) && (picked_word >= 0)) {
			if ((stroke_type == inter_recent_command_source) && (picked_word == inter_recent_command) && ((inter_awaiting == AWAITING_THIS) || (stroke_type == STROKE_ACTION))) {
				demand_abort = true;
				inter_look_around = inter_sentence_ready = ((inter_command == 0) && (stroke_type == STROKE_COMMAND));
				if (!inter_look_around) {
					inter_init_sentence();
				}
			}
		}
	}

	if (demand_abort) goto done;

	if (mouse_button) {
		switch (stroke_type) {
		case STROKE_COMMAND:
		case STROKE_ACTION:
			goto done;
			break;
		}
	}

	switch (inter_awaiting) {

	case AWAITING_COMMAND:
		switch (stroke_type) {

		case STROKE_COMMAND:
			if (inter_command >= 0) {
				if (inter_verb_type == VERB_ONLY) {
					inter_sentence_ready = true;
				} else {
					inter_recent_command = inter_command;
					inter_recent_command_source = inter_command_source;
					inter_awaiting = AWAITING_THIS;
				}
			}
			break;

		case STROKE_DIALOG:
			if (inter_command >= 0) {
				inter_sentence_ready = true;
			}
			break;

		case STROKE_SPECIAL_INVEN:
			// (Remove break to have clicking on spinning object reset inven list)
			break;

		case STROKE_INVEN:
			if (picked_word >= 0) {
				inter_set_active_inven(picked_word);
			}
			break;

		case STROKE_ACTION:
			if (inter_command >= 0) {
				if ((inter_verb_type == VERB_THIS) && (inter_prep_type == PREP_NONE)) {
					inter_sentence_ready = true;
				} else if ((inter_verb_type == VERB_THAT) && (inter_prep_type != PREP_NONE)) {
					inter_prep = inter_prep_type;
					inter_sentence_ready = true;
				} else {
					inter_awaiting = AWAITING_THAT;
					inter_prep = inter_prep_type;
				}
				inter_recent_command = inter_command;
				inter_recent_command_source = inter_command_source;
			}
			break;

		case STROKE_INTERFACE:
			inter_recent_command = -1;
			inter_recent_command_source = STROKE_NONE;
			if (mouse_y < display_y) {
				inter_sentence_ready = true;

				inter_point_x = mouse_x + picture_view_x;
				inter_point_y = mouse_y + picture_view_y;
				inter_point_established = true;
			}
			break;
		}
		break;

	case AWAITING_THIS:
		switch (stroke_type) {

		case STROKE_INVEN:
		case STROKE_INTERFACE:
		case STROKE_SPECIAL_INVEN:
			if (inter_main_object >= 0) {
				if (inter_prep_type == PREP_NONE) {
					inter_sentence_ready = true;
				} else {
					inter_prep = inter_prep_type;
					inter_awaiting = AWAITING_THAT;
				}
				if (stroke_type == STROKE_INTERFACE) {
					inter_point_x = mouse_x + picture_view_x;
					inter_point_y = mouse_y + picture_view_y;
					inter_point_established = true;
				}
			}
			break;

		}
		break;

	case AWAITING_THAT:
		switch (stroke_type) {
		case STROKE_INVEN:
		case STROKE_INTERFACE:
		case STROKE_SPECIAL_INVEN:
			if (inter_second_object >= 0) {
				inter_sentence_ready = true;
				if ((stroke_type == STROKE_INTERFACE) && (!inter_point_established)) {
					inter_point_x = mouse_x + picture_view_x;
					inter_point_y = mouse_y + picture_view_y;
					inter_point_established = true;
				}
			}
			break;
		}
		break;

	case AWAITING_RIGHT_MOUSE:
		inter_sentence_ready = true;
		break;
	}

done:
	;
}

/**
 * Manages the background animation for the interface screen, by
 * advancing segment counters and changing matteing lists when
 * appropriate.
 */
static void inter_background_animation(void) {
	int count, count2;
	int temp;
	int prob_check;
	int image_scan;
	int myprob;

	if (inter_anim == NULL) goto done;

	inter_no_segments_active = !inter_some_segments_active;
	inter_some_segments_active = false;
	for (count = 0; count < inter_anim->num_series /* num_segments */; count++) {
		if (inter_anim->segment[count].counter < 0) {
			if (inter_anim->segment[count].counter == -1) {
				prob_check = imath_random(1, 30000);
				myprob = inter_anim->segment[count].probability;
				if (inter_anim->segment[count].probability > 30000) {
					if (inter_no_segments_active) {
						myprob -= 30000;
					} else {
						myprob = -1;
					}
				}
				if (prob_check <= myprob) {
					inter_anim->segment[count].counter = inter_anim->segment[count].first_image;
					inter_some_segments_active = true;
				}
			} else {
				inter_anim->segment[count].counter = inter_anim->segment[count].first_image;
				inter_some_segments_active = true;
			}
		} else {
			for (count2 = 0; count2 < AA_MAX_SPAWNED; count2++) {
				if (inter_anim->segment[count].spawn_frame[count2] ==
					(inter_anim->segment[count].counter - inter_anim->segment[count].first_image)) {
					temp = inter_anim->segment[count].spawn[count2];
					if (count >= temp) {
						inter_anim->segment[temp].counter = inter_anim->segment[temp].first_image;
					} else {
						inter_anim->segment[temp].counter = -2;
					}
					inter_some_segments_active = true;
				}
			}
			inter_anim->segment[count].counter++;
			if (inter_anim->segment[count].counter > inter_anim->segment[count].last_image) {
				inter_anim->segment[count].counter = -1;
			} else {
				inter_some_segments_active = true;
			}
		}
	}

	for (count = 0; count < inter_anim->num_series; count++) {
		image_scan = inter_anim->segment[count].counter;
		if (image_scan >= 0) {
			if (image_inter_marker < IMAGE_INTER_LIST_SIZE) {
				image_inter_list[image_inter_marker] = inter_anim->image[image_scan];
				image_inter_list[image_inter_marker].flags = IMAGE_UPDATE;
				image_inter_marker++;
			}
#ifndef disable_error_check
			else {
				error_report(ERROR_IMAGE_INTER_LIST_FULL, WARNING, MODULE_INTER, IMAGE_INTER_LIST_SIZE, 3);
			}
#endif
		}
	}

done:
	;
}

void inter_spinning_object(void) {
	int count;

	if ((inter_input_mode == INTER_CONVERSATION) ||
		(inter_input_mode == INTER_LIMITED_SENTENCES)) goto done;

	if (inter_object_series >= 0) {
		inter_object_sprite++;
		if (inter_object_sprite > series_list[inter_object_series]->num_sprites) inter_object_sprite = 1;

		for (count = 0; count < (int)image_inter_marker; count++) {
			if (image_inter_list[count].segment_id == INTER_SPINNING_OBJECT) {
				image_inter_list[count].flags = IMAGE_FULLUPDATE;
			}
		}

		if (image_inter_marker < IMAGE_INTER_LIST_SIZE) {
			image_inter_list[image_inter_marker].flags = IMAGE_UPDATE;
			image_inter_list[image_inter_marker].segment_id = INTER_SPINNING_OBJECT;
			image_inter_list[image_inter_marker].sprite_id = (byte)inter_object_sprite;
			image_inter_list[image_inter_marker].series_id = (byte)inter_object_series;
			image_inter_list[image_inter_marker].x = inter_object_base_x;
			image_inter_list[image_inter_marker].y = inter_object_base_y;
			image_inter_marker++;
		}
#ifndef disable_error_check
		else {
			error_report(ERROR_IMAGE_INTER_LIST_FULL, WARNING, MODULE_INTER, IMAGE_INTER_LIST_SIZE, 3);
		}
#endif
	}

done:
	;
}

void inter_turn_off_object(void) {
	if (inter_object_series >= 0) {
		delete_sprite_in_interface(inter_object_series);
		matte_deallocate_series(inter_object_series, false);
		inter_base_time = timer_read();
		inter_object_series = -1;
	}

	// if last inventory object, copy section of interface
	// over inventory object after removed from list (removed above)
	if (active_inven == -1) {
		if (kernel_mode == KERNEL_ACTIVE_CODE) {
			video_update(&scr_inter, OUAF_OBJECT_X, 0,
				OUAF_OBJECT_X, 156,
				59, 44);
		}
	}


	if (new_room == room_id) {
		paul_object_showing = -1;
	}

	// will wipe out other 4 sprites
	if (inter_input_mode == INTER_BUILDING_SENTENCES) inter_refresh();
}

int inter_allocate_objects(void) {
	int  error_flag = true;

	if (!inter_spinning_objects) goto done;

	inter_spinning_objects = false;

	inter_objects_block_size = INTER_OBJECT_SPACE;

	inter_objects_block = (byte *)mem_get_name(inter_objects_block_size, "$SPIN$");
	if (inter_objects_block == NULL) goto done;

	inter_spinning_objects = true;

	error_flag = false;

done:
	return error_flag;
}

void inter_deallocate_objects(void) {
	if (inter_objects_block != NULL) {
		mem_free(inter_objects_block);
		inter_objects_block = NULL;
	}
}

void inter_screen_update(void) {
	if (kernel_mode == KERNEL_ACTIVE_CODE) {
		if (inter_input_mode == INTER_BUILDING_SENTENCES) {
			matte_inter_frame(!kernel.fx, kernel.fx);
			if (kernel.fx) {
				inter_prepare_background();
			}
		}
	}
}

static void inter_exec_function(void (*(target))()) {
	target();
}

void inter_spin_object(int object_id) {
	int error_flag = true;
	char temp_buf[80];

	inter_turn_off_object();

	// Update to live screen chunk of interface that selected object
	// is on (it has been deleted in list by now)
	if (kernel_mode == KERNEL_ACTIVE_CODE) {
		video_update(&scr_inter, OUAF_OBJECT_X, 0,
			OUAF_OBJECT_X, 156,
			59, 44);
	}

	if (!inter_spinning_objects) goto done;

	if (inter_input_mode == INTER_BUILDING_SENTENCES ||
		inter_input_mode == INTER_LIMITED_SENTENCES) inter_screen_update();

	if (inter_object_routine == NULL) {
		Common::strcpy_s(temp_buf, "*OB");

		env_catint(temp_buf, object_id, 3);

		Common::strcat_s(temp_buf, "I");

	} else {
		inter_object_id = object_id;
		inter_exec_function(inter_object_routine);
		Common::strcpy_s(temp_buf, inter_object_buf);
	}

	sprite_force_memory = inter_objects_block;
	sprite_force_size = inter_objects_block_size;

	inter_object_series = matte_load_series(temp_buf,
		SPRITE_LOAD_SPINNING_OBJECT,
		SERIES_BONUS_OBJECT);

	if (g_engine->getGameID() == GType_Forest) {
		stamp_sprite_to_interface(OUAF_OBJECT_X, OUAF_OBJECT_Y, 1, inter_object_series);
		paul_object_showing = object_id;
	}

	if (inter_object_series < 0)
		goto done;

	mcga_setpal_range(&master_palette, 7, 1);
	mcga_setpal_range(&master_palette, 246, 6);

	inter_object_sprite = 1;

	error_flag = false;

done:
	sprite_force_memory = NULL;
	sprite_force_size = 0;
}

void inter_main_loop(int allow_input) {
	int width, x, y;
	int count;
	int end_of_this_selection;
	int flags;
	int use_spacing;
	long now_time;
	FontPtr use_font;

	end_of_this_selection = false;

	mouse_begin_cycle(false);

	inter_mouse_x = mouse_x;
	inter_mouse_y = mouse_y;
	inter_mouse_type = ABSOLUTE_MODE;

	if (allow_input) inter_auxiliary_click = false;

	if (mouse_start_stroke || !mouse_any_stroke) {
		if (mouse_y < display_y) {
			inter_mouse_x += picture_view_x;
			inter_mouse_y += picture_view_y;
			inter_mouse_type = RELATIVE_MODE;
		}
	} else {
		if (stroke_type == STROKE_INTERFACE) {
			inter_mouse_x += picture_view_x;
			inter_mouse_y += picture_view_y;
			inter_mouse_type = RELATIVE_MODE;
		}
	}

	if (!mouse_any_stroke || (inter_input_mode != INTER_BUILDING_SENTENCES)) {
		mouse_button = 0;
	}

	if ((mouse_changed || scroll_active || scrollbar_active || inter_force_rescan) && allow_input) {

		inter_spot_class = STROKE_NONE;
		inter_spot_index = hspot_which_reverse(inter_mouse_x,
			inter_mouse_y, inter_mouse_type);
		if (inter_spot_index > 0) {
			inter_spot_class = spot[inter_spot_index]._class & STROKE_MASK;
			inter_spot_id = spot[inter_spot_index].num;
		}

		if (inter_report_hotspots && !mouse_stroke_going && (inter_spot_class != stroke_type) && (stroke_type != STROKE_NONE)) {
			end_of_selection = end_of_this_selection = true;
			switch (stroke_type) {
			case STROKE_INTERFACE:
			case STROKE_COMMAND:
			case STROKE_INVEN:
			case STROKE_ACTION:
			case STROKE_SPECIAL_INVEN:
			case STROKE_DIALOG:
				inter_select_word();
				break;
			}
			inter_analyze_stroke();
		}

		if (mouse_start_stroke || (inter_report_hotspots && !mouse_stroke_going)) {
			stroke_type = inter_spot_class;
		}

		end_of_selection = mouse_stop_stroke;

		if (mouse_any_stroke || inter_report_hotspots) {
			switch (stroke_type) {
			case STROKE_INTERFACE:
			case STROKE_COMMAND:
			case STROKE_INVEN:
			case STROKE_ACTION:
			case STROKE_SPECIAL_INVEN:
			case STROKE_DIALOG:
				inter_select_word();
				break;
			}
		}

		if (mouse_any_stroke ||
			(inter_report_hotspots && (inter_awaiting > AWAITING_COMMAND) && (stroke_type == STROKE_INVEN)) ||
			(inter_report_hotspots && (stroke_type == STROKE_INTERFACE))) {
			inter_analyze_stroke();
		}

		if (mouse_stop_stroke) {
			inter_complete_stroke();
			stroke_type = STROKE_NONE;
		}

		if (mouse_any_stroke || inter_report_hotspots || scrollbar_active) {
			inter_scroll_bar();
		}

		if (mouse_any_stroke || inter_report_hotspots) {
			inter_compile_sentence();
		}

		inter_force_rescan = false;
	}

	if (!allow_input) {
		if (mouse_stop_stroke) {
			inter_auxiliary_click = true;
		}
	}

	if (inter_sentence_changed) {
		if (inter_sentence_handle >= 0) {
			matte_clear_message(inter_sentence_handle);
			inter_sentence_handle = -1;
		}
		if ((strlen(inter_sentence) > 0) && ((inter_input_mode == INTER_BUILDING_SENTENCES) || (inter_input_mode == INTER_LIMITED_SENTENCES))) {
			use_font = font_main;
			use_spacing = -1;
			width = font_string_width(use_font, inter_sentence, use_spacing);
			if (width > video_x) {
				use_font = font_inter;
				use_spacing = 0;
				width = font_string_width(font_inter, inter_sentence, use_spacing);
			}
			x = (video_x >> 1) - (width >> 1);
			y = (viewing_at_y + scr_work.y - 1) - 12;
			inter_sentence_handle = matte_add_message (use_font, inter_sentence, x, y, INTER_MESSAGE_COLOR, use_spacing);
		}
		inter_sentence_changed = false;
	}

	now_time = timer_read();
	if (now_time >= inter_base_time) {
		for (count = 0; count < (int)image_inter_marker; count++) {
			flags = image_inter_list[count].flags;
			if ((flags != IMAGE_REFRESH) &&
				(flags > IMAGE_UPDATE_ONLY) &&
				(image_inter_list[count].segment_id != INTER_SPINNING_OBJECT)) {
				image_inter_list[count].flags = IMAGE_ERASE;
			}
		}
		inter_background_animation();
		inter_spinning_object();
		inter_base_time = now_time + 6;
	}
}

int inter_load_background(const char *name, Buffer *target) {
	Color color[16];
	int count;
	int error_flag = true;
	Load load_handle;

	load_handle.open = false;

	if (loader_open(&load_handle, name, "rb", true)) goto done;

	if (!loader_read(&color[0], sizeof(Color) * 16, 1, &load_handle)) goto done;

	for (count = 0; count < 16; count++) {
		master_palette[count].r = color[count].r;
		master_palette[count].g = color[count].g;
		master_palette[count].b = color[count].b;
		color_status[count] |= PAL_RESERVED;
	}

	buffer_init_name(target, video_x, inter_size_y, "$scrintr");
	if (target->data == NULL) goto done;

	if (!loader_read(target->data, (video_x * inter_size_y), 1, &load_handle)) goto done;

	error_flag = false;

done:
	if (error_flag) {
		if (target->data != NULL) {
			buffer_free(target);
		}
	}
	if (load_handle.open) loader_close(&load_handle);
	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
