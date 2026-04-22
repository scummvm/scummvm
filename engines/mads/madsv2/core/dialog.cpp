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

#include "common/textconsole.h"
#include "mads/madsv2/core/dialog.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/cursor.h"
#include "mads/madsv2/core/window.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/timer.h"

namespace MADS {
namespace MADSV2 {


/* public global variables */

int dialog_error;
bool dialog_quicksearch_flag = true;
bool dialog_wildcard_exits = false;

/* private global variables */

static int mouse_stroke_type;           /* Where the stroke originated    */
static int mouse_orig_stroke_type;      /* First radio button pressed     */
static int mouse_override;              /* Override repeat damper         */
static int stroke_going;                /* Currently processing a stroke  */
static int resolved_mouse;              /* Have resolved stroke this pass */
static long mouse_timing_clock;         /* Stroke timer for logitech      */
static ItemPtr mouse_scroll_item;       /* Recent mouse item              */
static ListPtr mouse_scroll_list;       /* Recent mouse list              */
static char temp_buf[80];               /* A convenient buffer            */

static MouseCallback mouse_callback;	/* Mouse callback vector          */
static int mouse_callback_double_only;  /* Mouse callback double only flag*/

/* Page callback vector           */
static PageCallback page_callback;

static int allow_list_callbacks;        /* Flag if list callbacks allowed */

static int dialog_default_normal = DD_DEFAULT_NORMAL_COLOR;
static int dialog_default_select = DD_DEFAULT_SELECT_COLOR;
static int dialog_default_hilite = DD_DEFAULT_HILITE_COLOR;
static int dialog_default_greyed = DD_DEFAULT_GREYED_COLOR;

static long dialog_string_space = DIALOG_DEFAULT_STRING_SPACE;

long dialog_timeout = 0;

int dialog_language = DIALOG_ENGLISH;

#define ATTR_DIR        10h             /* Attribute for subdirectory */
#define DTA_ATTR        15h             /* offset in DTA of attribute */
#define DTA_NAME        1eh             /* offset in DTA of filename  */

int alert_normal_color = DD_DEFAULT_NORMAL_COLOR;
int alert_select_color = DD_DEFAULT_SELECT_COLOR;
int alert_hilite_color = DD_DEFAULT_HILITE_COLOR;

#define SAY_DIALOG_SIZE 1024

static Dialog say_dialog_box;
static DialogPtr say_dialog = &say_dialog_box;
static char say_dialog_work[SAY_DIALOG_SIZE];

#define write_flag      0x01
#define error_mask      0x06
#define fail_flag       0x08
#define retry_flag      0x10
#define ignore_flag     0x20

#define ignore_option   0x00
#define retry_option    0x01
#define abort_option    0x02
#define fail_option     0x03

Dialog crit_dialog;
char crit_work[256];

extern void dialog_24_server(void);

static int   dialog_server_installed = false;
static dword dialog_old_24_server;


int dialog_read_dir_to_list(ListPtr target, const char *wild, int dirflag) {
	warning("TODO: dialog_read_dir_to_list");
	target->elements = 0;
	return 0;
}

/**
 * Appends a list of valid drive letters to the end of the specified
 * list structure.  This is done by attempting to "change drive" to
 * each drive letter beginning with "A", and stopping when an attempt fails.
 *
 * @param dirslist		Directory list structure
 */
static void dialog_read_drives_to_list(ListPtr dirslist) {
	char *myptr;

	myptr = dirslist->list + (dirslist->elements * dirslist->element_offset);
	Common::strcpy_s(myptr, 4, "C:\\");
	dirslist->elements++;
}

/**
 * Loads directory (and subdirectory list w/ drives) for a filename item
 *
 * @param dialog	Dialog
 * @param item		Filename item
 */
static void dialog_load_directory(DialogPtr dialog, ItemPtr item) {
	ItemPtr fileitem, dirsitem;
	ListPtr filelist, dirslist;

	// Get our file list item and dirs list item
	fileitem = &dialog->item[item->status];
	dirsitem = &dialog->item[fileitem->id + 1];

	// Get the associated list structures
	filelist = &dialog->lists[fileitem->buf_id];
	dirslist = &dialog->lists[dirsitem->buf_id];

	filelist->elements = 0;
	dirslist->elements = 0;

	// Try to read them in
	dialog_read_dir_to_list(dirslist, "*.*", true);
	dialog_read_dir_to_list(filelist, dialog->buffer[item->buf_id], false);

	dialog_read_drives_to_list(dirslist);

	// Reset window cursors
	filelist->base_entry = 0;
	dirslist->base_entry = 0;

	filelist->picked_entry = -1;
	dirslist->picked_entry = -1;
}

/**
 * Item to determine the proper X and Y values for an item based
 * on the X and Y values passed by the user (handles the various
 * special codes).
 *
 * @param dialog	Dialog
 * @param item		Filename item
 * @param x			X position
 * @param y			Y position
 */
static void item_locate(DialogPtr dialog, ItemPtr item, int x, int y) {
	switch (x) {
	case DD_IX_CENTER:
		x = 0;
		break;

	case DD_IX_LEFT:
		if (y == DD_IY_BUTTON) {
			x = dialog->button_left;
			dialog->button_left += item->width + 1;
		} else {
			x = 1;
		}
		break;

	case DD_IX_RIGHT:
		if (y == DD_IY_BUTTON) {
			x = dialog->button_right - (item->width + 1);
			dialog->button_right = x;
		} else {
			x = 0 - (item->width + 1);
		}
		break;
	}

	switch (y) {
	case DD_IY_AUTOFILL:
		y = dialog->fill_marker++;
		break;

	default:
		if (y >= dialog->fill_marker) {
			dialog->fill_marker = y + 1;
		}
		break;
	}

	item->x = x;
	item->y = y;
}

/**
 * Attempts to allocate a new item handle from the dialog box's
 * item array. Returns NULL if fails.
 *
 * @param dialog	Dialog	Dialog
 * @param item_type		Item type
 * @return	New item
 */
static ItemPtr item_allocate(DialogPtr dialog, int item_type) {
	ItemPtr item;

	if (dialog->num_items < DIALOG_MAX_ITEM) {
		item = &dialog->item[dialog->num_items];
		item->type = item_type;
		item->id = (dialog->num_items++);
		return item;
	} else {
		dialog_error = DD_ERR_NOMOREITEMS;
		return NULL;
	}
}

static char *string_allocate(DialogPtr dialog, int length) {
	char *result = NULL;

	if (dialog->string_space_remaining < (length + 1)) {
		dialog_error = DD_ERR_NOMORESTRINGS;
		goto done;
	}

	result = dialog->string_marker;

	dialog->string_marker += (length + 1);
	dialog->string_space_remaining -= (length + 1);

done:
	return result;
}

/**
 * Attempts to allocate one of the dialog box's string buffers.
 * Returns -1 if no buffer was available.
 *
 * @param dialog	Dialog
 * @return	Buffer handle
 */
static int buffer_allocate(DialogPtr dialog) {
	int my_buffer = -1;
	char *space;

	if (dialog->buffers_allocated >= DIALOG_MAX_BUFFER) {
		dialog_error = DD_ERR_NOMOREBUFFERS;
		goto done;
	}

	space = string_allocate(dialog, 80);
	if (space == NULL) goto done;

	my_buffer = dialog->buffers_allocated;
	dialog->buffers_allocated++;

	dialog->buffer[my_buffer] = space;

done:
	return my_buffer;
}

/**
 * Attempts to allocate one of the dialog box's list window structures.
 * Returns -1 if all are in use.
 *
 * @param dialog	Dialog
 * @return	List window structure handle
 */
static int list_allocate(DialogPtr dialog) {
	int my_list;

	if (dialog->lists_allocated < DIALOG_MAX_LIST) {
		my_list = dialog->lists_allocated;
		dialog->lists_allocated++;
	} else {
		dialog_error = DD_ERR_NOMORELISTS;
		my_list = -1;
	}

	return my_list;
}

/**
 * Returns the "hotkey" keystroke value, if any, for the
 * passed string.  Returns 0 if no hotkey found.  Hotkeys
 * are marked by preceding "~" characters.
 *
 * @param prompt	Prompt
 * @return	Keystroke value
 */
static int get_keystroke(const char *prompt) {
	const char *key_finder;
	int key_stroke;

	key_finder = strchr(prompt, '~');
	if (key_finder != NULL) {
		key_finder++;
		key_stroke = *key_finder;
	} else {
		key_stroke = 0;
	}

	return key_stroke;
}

ItemPtr dialog_add_button(DialogPtr dialog, int x, int y, const char *prompt) {
	ItemPtr item = NULL;
	char *space;
	int space_look;

	space = string_allocate(dialog, strlen(prompt));
	if (space == NULL) goto done;

	item = item_allocate(dialog, DD_I_BUTTON);

	if (item != NULL) {

		item->prompt = space;
		Common::strcpy_s(item->prompt, 65536, prompt);

		item->width = strlen(prompt) + 2;

		item_locate(dialog, item, x, y);

		item->x2 = -1;

		// Adjust cursor location to appear over the first letter (not
		// over a space).
		for (space_look = 0; ((unsigned)space_look < strlen(prompt)) && (item->x2 < 0); space_look++) {
			if (prompt[space_look] != ' ') item->x2 = space_look + 1;
		}

		if (item->x2 < 0) item->x2 = 0;

		item->status = 0;

		item->keystroke = get_keystroke(prompt);

		Common::strcpy_s(item->prompt, 65536, prompt);

		if (dialog_language == DIALOG_GERMAN) {
			if (strcmp(prompt, CANCEL_GERMAN) == 0) {
				dialog->cancel_item = item;
			}
		} else {
			if (strcmp(prompt, CANCEL_BUTTON) == 0) {
				dialog->cancel_item = item;
			}
		}

	}

done:
	return item;
}

ItemPtr dialog_add_checkbox(DialogPtr dialog, int x, int y, const char *prompt,
		int default_val, int class_) {
	ItemPtr item = NULL;
	char *space;

	space = string_allocate(dialog, strlen(prompt));
	if (space == NULL) goto done;

	item = item_allocate(dialog, DD_I_CHECKBOX);

	if (item != NULL) {
		item->width = strlen(prompt) + 4;

		item_locate(dialog, item, x, y);

		item->x2 = 1;
		item->status = (!!default_val) & DD_CHECK_TRUE;  // (sic)

		dialog->width = MAX<short>(dialog->width, (item->x + item->width + 1));

		item->prompt = space;
		Common::strcpy_s(item->prompt, 65536, prompt);

		item->keystroke = get_keystroke(prompt);

		item->_class = class_;

	}

done:
	return item;
}

void dialog_grey_checkbox(ItemPtr item) {
	if (item->type == DD_I_CHECKBOX) {
		item->status |= DD_CHECK_GREY;
	}
}

ItemPtr dialog_add_message(DialogPtr dialog, int x, int y, const char *prompt) {
	ItemPtr item = NULL;
	char *space = NULL;

	if (prompt != NULL) {
		space = string_allocate(dialog, strlen(prompt));
		if (space == NULL) goto done;
	}

	item = item_allocate(dialog, DD_I_MESSAGE);

	if (item != NULL) {
		item->prompt = space;
		if (prompt == NULL) {
			item->width = DD_LINEACROSS;
		} else {
			item->width = strlen(prompt);
			Common::strcpy_s(item->prompt, 65536, prompt);
		}
		item_locate(dialog, item, x, y);
		dialog->width = MAX<short>(dialog->width, (item->x + item->width + 1));
		item->status = 0;
	}

done:
	return item;
}

void dialog_add_blank(DialogPtr dialog) {
	dialog->fill_marker++;
}

ItemPtr dialog_add_string(DialogPtr dialog, int x, int y, const char *prompt,
		const char *default_val, int width) {
	ItemPtr item = NULL;
	char *space;

	space = string_allocate(dialog, strlen(prompt));
	if (space == NULL) goto done;

	item = item_allocate(dialog, DD_I_STRING);

	if (item != NULL) {

		item->width = strlen(prompt) + 2 + width;

		item_locate(dialog, item, x, y);

		dialog->width = MAX<short>(dialog->width, (item->x + item->width));

		item->x2 = strlen(prompt) + 1;

		// Account for any hotkey stroke in setting proper cursor location
		if (strchr(prompt, '~') != NULL) {
			item->width--;
			item->x2--;
		}

		item->status = 0;
		item->keystroke = get_keystroke(prompt);

		item->prompt = space;
		Common::strcpy_s(item->prompt, 65536, prompt);

		// Set up a buffer in the dialog's buffer space
		item->buf_width = width;
		item->buf_id = buffer_allocate(dialog);

		if (item->buf_id < 0) {

			dialog->num_items--;
			item = NULL;

		} else {

			// Copy the default value (if any) into the buffer
			if (default_val != NULL) {
				Common::strcpy_s(dialog->buffer[item->buf_id], 65536, default_val);
				if (strlen(default_val) > (unsigned)width) {
					dialog->buffer[item->buf_id][width] = 0;
				}
				dialog->buf_cursor[item->buf_id] = strlen(default_val);
			} else {
				dialog->buffer[item->buf_id][0] = 0;
				default_val = &dialog->buffer[item->buf_id][0];
			}

			// Mark the entire buffer contents as "selected"
			dialog->buf_status[item->buf_id] = 0;
			dialog->buf_base_x[item->buf_id] = item->x2;
			if (strlen(default_val) > 0) {
				dialog->buf_select[item->buf_id] = true;
				dialog->buf_selbase[item->buf_id] = 0;
				dialog->buf_seltarget[item->buf_id] = strlen(default_val);
			} else {
				dialog->buf_select[item->buf_id] = false;
			}
		}
	}

done:
	return item;
}

ItemPtr dialog_add_listbased(DialogPtr dialog, int x, int y, const char *prompt,
		const char *default_val, int width, const char *title, char *mylist,
		int elements, int element_offset, int entry_width, int rows, int columns) {
	ItemPtr item, listitem;
	ListPtr list;
	int listnum;
	int temp_width;

	// First, just add the normal string entry area part
	item = dialog_add_string(dialog, x, y, prompt, default_val, width);

	// Then, do all the additional stuff for a list window
	if (item != NULL) {
		item->type = DD_I_LISTBASED;

		listitem = item_allocate(dialog, DD_I_LIST);

		if (listitem != NULL) {

			listnum = list_allocate(dialog);

			if (listnum >= 0) {

				dialog_add_blank(dialog);
				dialog_add_message(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, title);

				list = &(dialog->lists[listnum]);

				list->rows = rows;
				list->columns = columns;

				temp_width = (columns * (entry_width + 1)) + 2;

				dialog->width = MAX<short>(dialog->width, (temp_width + 4));

				list->window.ul_x = 0;
				list->window.ul_y = dialog->fill_marker;

				dialog->fill_marker += (rows + 2);

				listitem->keystroke = get_keystroke(title);

				list->elements = elements;
				list->element_offset = element_offset;
				list->entry_width = entry_width;

				list->base_entry = 0;
				list->picked_entry = -1;
				list->thumb = -1;

				// Set items up to point at one another
				item->status = listitem->id;
				listitem->status = item->id;
				listitem->buf_id = listnum;

				list->list = mylist;

			} else {
				dialog->num_items -= 2;
				item = NULL;
			}
		} else {
			dialog->num_items--;
			item = NULL;
		}
	}

	return item;
}

ItemPtr dialog_append_list(DialogPtr dialog, int x, int y, ItemPtr base_string,
	char *title,
	char *mylist, int elements, int element_offset,
	int entry_width, int rows, int columns) {
	ItemPtr item, listitem = nullptr;
	ListPtr list;
	int listnum;
	int temp_width;

	// First, just add the normal string entry area part
	item = base_string;

	// Then, do all the additional stuff for a list window
	if (item != NULL) {
		listitem = item_allocate(dialog, DD_I_LIST);

		if (listitem != NULL) {

			listnum = list_allocate(dialog);

			if (listnum >= 0) {

				dialog_add_blank(dialog);
				dialog_add_message(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, title);

				list = &(dialog->lists[listnum]);

				list->rows = rows;
				list->columns = columns;

				temp_width = (columns * (entry_width + 1)) + 2;

				dialog->width = MAX<short>(dialog->width, (temp_width + 4));

				list->window.ul_x = 0;
				list->window.ul_y = dialog->fill_marker;

				dialog->fill_marker += (rows + 2);

				listitem->keystroke = get_keystroke(title);

				list->elements = elements;
				list->element_offset = element_offset;
				list->entry_width = entry_width;

				list->base_entry = 0;
				list->picked_entry = -1;
				list->thumb = -1;

				// Set items up to point at one another
				listitem->status = item->id;
				listitem->buf_id = listnum;

				list->list = mylist;

			} else {
				dialog->num_items -= 1;
			}
		}
	}

	return listitem;
}

ItemPtr dialog_add_filename(DialogPtr dialog, int x, int y, const char *prompt,
		const char *default_val, const char *path, int rows,
		char *filebuf, int max_file_elements,
		char *dirsbuf, int max_dirs_elements) {
	ItemPtr item, listitem, dirsitem;
	ListPtr list, dirs;
	int list_num, dirs_num;
	int base_item;
	int message_line;
	int savedpath;
	int savedrive;
	char savepath[80];
	char mypath[80];
	char mypath2[80];
	int mydrive;

	dialog->status |= DD_FILEMENU;  // Note that we do in fact use files

	base_item = dialog->num_items;

	item = dialog_add_string(dialog, x, y, prompt, default_val, DIALOG_FILE_BUFFER);

	if (item != NULL) {

		listitem = item_allocate(dialog, DD_I_FILELIST);
		dirsitem = item_allocate(dialog, DD_I_DIRSLIST);

		if ((listitem == NULL) || (dirsitem == NULL) || (dialog->lists_allocated > 0)) {
			item = NULL;
		} else {

			savedpath = (mads_getcwd(savepath, 80) != NULL);
			savedrive = mads_getdrive();

			list_num = list_allocate(dialog);
			dirs_num = list_allocate(dialog);

			list = &dialog->lists[list_num];
			dirs = &dialog->lists[dirs_num];

			item->type = DD_I_FILENAME;
			listitem->type = DD_I_FILELIST;
			dirsitem->type = DD_I_DIRSLIST;

			listitem->keystroke = (int)'L';
			dirsitem->keystroke = (int)'D';

			dialog_add_blank(dialog);
			dialog->path_item = dialog_add_message(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, path);
			dialog->string_marker += (80 - strlen(path));
			dialog_add_blank(dialog);

			message_line = dialog->fill_marker;
			dialog_add_message(dialog, DD_IX_LEFT, message_line, "File ~List:");
			dialog_add_message(dialog, 44, message_line, "~Drives / Dirs:");

			list->rows = rows;
			list->columns = 3;
			dirs->rows = rows;
			dirs->columns = 1;

			dialog->width = MAX<short>(dialog->width, DIALOG_FILE_WIDTH);

			list->entry_width = 12;
			list->element_offset = 13;
			list->max_elements = max_file_elements;
			list->base_entry = 0;
			list->picked_entry = -1;
			list->thumb = -1;

			dirs->entry_width = 12;
			dirs->element_offset = 13;
			dirs->max_elements = max_dirs_elements;
			dirs->base_entry = 0;
			dirs->picked_entry = -1;
			dirs->thumb = -1;

			list->window.ul_x = 0;
			dirs->window.ul_x = 43;

			list->window.ul_y = dialog->fill_marker;
			dirs->window.ul_y = dialog->fill_marker;

			item->status = listitem->id;
			listitem->status = item->id;
			dirsitem->status = item->id;

			listitem->buf_id = list_num;
			dirsitem->buf_id = dirs_num;

			list->list = filebuf;
			dirs->list = dirsbuf;

			dialog->fill_marker += (rows + 2);

			if (savedpath) {
				Common::strcpy_s(mypath, path);
				mads_strupr(mypath);
				mads_fullpath(mypath2, mypath, 80);
				mydrive = ((int)mypath2[0]) - '@';
				mads_chdir(mypath2);
				mads_chdrive(mydrive);
			}

			dialog_load_directory(dialog, item);

			if (savedpath) {
				mads_chdir(savepath);
				mads_chdrive(savedrive);
			}
		}
	}

	if (item == NULL) {
		dialog->num_items = base_item;
	}

	return item;
}

void dialog_set_colors(int normal, int select, int hilite, int greyed) {
	dialog_default_normal = normal;
	dialog_default_select = select;
	dialog_default_hilite = hilite;
	dialog_default_greyed = greyed;
}

void dialog_set_workspace_size(long workspace) {
	dialog_string_space = workspace;
}

void dialog_set_string_space(DialogPtr dialog, char *space, long size) {
	dialog->string_space = space;
	dialog->string_space_remaining = size;
	dialog->string_marker = space;
}

DialogPtr dialog_create(DialogPtr dialog, int ul_x, int ul_y, int width,
	int normal_color, int select_color, int hilite_color) {

	// Allocate memory for dialog if necessary
	if (dialog == NULL) {
		dialog = (DialogPtr)mem_get(sizeof(struct DialogBox));
		dialog->status = DD_DYNAMIC;
		dialog->string_space = NULL;
	} else {
		dialog->status = DD_STATIC;
	}

	mouse_callback = NULL;
	page_callback = NULL;

	if (dialog != NULL) {

		if (dialog->string_space == NULL) {
			dialog->status |= DD_DYNAMICSTRINGS;
			dialog->string_space = (char *)mem_get(dialog_string_space);
			dialog->string_space_remaining = dialog_string_space;
			if (dialog->string_space == NULL) {
				dialog_error = DD_ERR_NOMOREMEMORY;
				if (dialog->status & DD_DYNAMIC) {
					mem_free(dialog);
				}
				dialog = NULL;
			}
		}

		if (normal_color == DD_DEFAULT) {
			dialog->normal_color = dialog_default_normal;
		} else {
			dialog->normal_color = normal_color;
		}

		if (select_color == DD_DEFAULT) {
			dialog->select_color = dialog_default_select;
		} else {
			dialog->select_color = select_color;
		}

		if (hilite_color == DD_DEFAULT) {
			if (screen_video_mode == mono_text_mode) {
				dialog->hilite_color = dialog_default_select;
			} else {
				dialog->hilite_color = dialog_default_hilite;
			}
		} else {
			dialog->hilite_color = hilite_color;
		}

		dialog->width = width;

		window_set(&dialog->window, ul_x, ul_y, ul_x + width + 1, ul_y);

		dialog->base_x = ul_x + 1;
		dialog->base_y = ul_y + 1;

		dialog->num_items = 0;
		dialog->fill_marker = 0;
		dialog->active_button = dialog->active_item = 0;

		dialog->cursor_x = dialog->cursor_y = 0;

		dialog->button_left = 1;
		dialog->button_right = 0;

		dialog->buffers_allocated = 0;
		dialog->lists_allocated = 0;

		dialog->callback = NULL;
		dialog->checkbox_callback = NULL;

		dialog->cancel_item = NULL;
		dialog->path_item = NULL;
		dialog->default_item = NULL;

		dialog->string_marker = dialog->string_space;

	} else {
		dialog_error = DD_ERR_NOMOREMEMORY;
	}

	return  dialog;
}

void dialog_destroy(DialogPtr dialog) {
	if (dialog != NULL) {
		if (dialog->status & DD_SCREENSAVED) {
			window_destroy(&dialog->window);
			dialog->status &= ~DD_SCREENSAVED;
		}
		if (dialog->status & DD_DYNAMICSTRINGS) {
			mem_free(dialog->string_space);
			dialog->string_space = NULL;
		}
		if (dialog->status & DD_DYNAMIC) {
			mem_free(dialog);
		}
	}
}

void dialog_destroy_persist(DialogPtr dialog) {
	if (dialog != NULL) {
		if (dialog->status & DD_SCREENSAVED) {
			mem_free(dialog->window.storage);
			dialog->status &= ~DD_SCREENSAVED;
		}
		if (dialog->status & DD_DYNAMICSTRINGS) {
			mem_free(dialog->string_space);
			dialog->string_space = NULL;
		}
		if (dialog->status & DD_DYNAMIC) {
			mem_free(dialog);
		}
	}
}

DialogPtr dialog_file_create(DialogPtr dialog, int ul_x, int ul_y,
		int normal_color, int select_color, int hilite_color,
		ItemPtr *ok_item, ItemPtr *first_item, const char *default_val,
		const char *path, int rows, char *filebuf, int maxfiles,
		char *dirsbuf, int maxdirs, const char *prompt) {
	dialog = dialog_create(dialog, ul_x, ul_y, DIALOG_FILE_WIDTH,
		normal_color, select_color, hilite_color);

	dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, prompt);

	*ok_item = dialog_add_button(dialog, DD_IX_LEFT, DD_IY_BUTTON, "  OK  ");
	dialog_add_button(dialog, DD_IX_RIGHT, DD_IY_BUTTON, CANCEL_BUTTON);
	*first_item = dialog_add_filename(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, "~File Name: ",
		default_val, path, rows,
		filebuf, maxfiles,
		dirsbuf, maxdirs);

	return dialog;
}

void dialog_set_list_callback(DialogPtr dialog, void (*(callback))()) {
	dialog->callback = callback;
}

void dialog_set_mouse_callback(MouseCallback callback, int double_only) {
	mouse_callback = callback;
	mouse_callback_double_only = double_only;
}

void dialog_set_page_callback(PageCallback callback) {
	page_callback = callback;
}

void dialog_set_checkbox_callback(DialogPtr dialog, void (*(callback))()) {
	dialog->checkbox_callback = callback;
}

/**
 * Computes all necessary item coordinates and sizes => resolves
 * all "indefinite" coordinates (i.e. centered dialogs, etc) into
 * proper absolute screen coordinates (or dialog-relative coordinates)
 * in preparation for actually executing the dialog.
 *
 * @param dialog	Dialog
 */
static void dialog_compute_window(DialogPtr dialog) {
	int height;
	int count;
	int count2;
	int center;
	int button_width;
	int extra_height;
	ItemPtr item;
	ListPtr list;

	// Find out if there's anything scheduled for the "button row"
	dialog->button_flag = false;
	extra_height = 2;

	for (count = 0; count < dialog->num_items; count++) {
		if (dialog->item[count].y == DD_IY_BUTTON) {
			dialog->button_flag = true;
			extra_height = 4;
		}
	}

	// Figure out which checkbox items are in the same class as another
	for (count = 0; count < dialog->num_items; count++) {
		if (dialog->item[count].type == DD_I_CHECKBOX) {
			for (count2 = 0; count2 < dialog->num_items; count2++) {
				if (count != count2) {
					if (dialog->item[count2].type == DD_I_CHECKBOX) {
						if (dialog->item[count]._class == dialog->item[count2]._class) {
							dialog->item[count].status |= DD_CHECK_RADIO;
							dialog->item[count2].status |= DD_CHECK_RADIO;
						}
					}
				}
			}
		}
	}

	// Conform the dialog to the width of the button row, if necessary
	button_width = (dialog->button_left + abs(dialog->button_right) + 1);
	dialog->width = MAX<short>(dialog->width, button_width);

	// Center the dialog box, if requested
	if (dialog->window.ul_x == DD_CENTER) {
		center = screen_center_x ? screen_center_x : ((screen_max_x + 1) >> 1);
		dialog->window.ul_x = (center - ((dialog->width + 3) >> 1));
		dialog->base_x = dialog->window.ul_x + 1;
	}

	height = dialog->fill_marker + extra_height;

	if (dialog->window.ul_y == DD_CENTER) {
		center = screen_center_y ? screen_center_y : ((screen_max_y + 1) >> 1);
		dialog->window.ul_y = (center - ((height + 1) >> 1));
		dialog->base_y = dialog->window.ul_y + 1;
	}

	// Get the lower left dialog coordinates
	dialog->window.lr_x = dialog->window.ul_x + dialog->width + 1;
	dialog->window.lr_y = dialog->window.ul_y + height - 1;

	// Resolve any right-margin relative items and compute all list windows
	for (count = 0; count < dialog->num_items; count++) {
		item = &dialog->item[count];

		if (item->x < 0) {
			item->x += dialog->width;
		} else if (item->x == 0) {
			item->x = ((dialog->width + 1) >> 1) - ((item->width + 1) >> 1);
		}

		switch (item->type) {

		case DD_I_LIST:
		case DD_I_FILELIST:
		case DD_I_DIRSLIST:
			list = &dialog->lists[item->buf_id];
			item->x = (list->window.ul_x += dialog->window.ul_x + 2);
			item->y = (list->window.ul_y += dialog->base_y);
			list->window.lr_x = list->window.ul_x + (list->columns * (list->entry_width + 1)) + 2;
			list->window.lr_y = list->window.ul_y + list->rows + 1;
			item->x2 = list->base_x = list->window.ul_x + 1;
			list->base_y = list->window.ul_y + 1;
			break;

		default:
			item->x2 += item->x;
			break;

		}
	}
}

int dialog_read_checkbox(DialogPtr dialog, ItemPtr item) {
	return item->status & DD_CHECK_TRUE;
}

char *dialog_read_string(DialogPtr dialog, ItemPtr item) {
	return dialog->buffer[item->buf_id];
}

char *dialog_read_list(DialogPtr dialog, ItemPtr item) {
	return dialog->buffer[item->buf_id];
}

char *dialog_read_pathname(DialogPtr dialog, ItemPtr item) {
	return (dialog->path_item)->prompt;
}

char *dialog_read_filename(DialogPtr dialog, ItemPtr item) {
	return dialog->buffer[item->buf_id];
}

char *dialog_read_filepath(DialogPtr dialog, ItemPtr item) {
	int mylen;

	Common::strcpy_s(temp_buf, (dialog->path_item)->prompt);
	mylen = strlen(temp_buf);

	if (temp_buf[mylen - 1] != '\\') {
		Common::strcat_s(temp_buf, "\\");
	}

	Common::strcat_s(temp_buf, dialog->buffer[item->buf_id]);

	return (temp_buf);
}

/**
 * Updates the status of a dialog box based on its status flag
 *
 * @param dialog	Dialog
 * @param item	
 */
static void dialog_update_checkbox(DialogPtr dialog, ItemPtr item) {
	char temp1[2];
	int color;

	mouse_hide();

	Common::strcpy_s(temp1, "X");

	if (item->status & DD_CHECK_RADIO) {
		temp1[0] = (char)0x07;
	}

	color = (item->status & DD_CHECK_GREY) ? dialog_default_greyed : dialog->normal_color;

	if (item->status & DD_CHECK_TRUE) {
		screen_put(temp1, color, 0, item->x2 + dialog->base_x, item->y + dialog->base_y);
	} else {
		screen_put(" ", color, 0, item->x2 + dialog->base_x, item->y + dialog->base_y);
	}

	mouse_show();
}

/**
 * Displays the specified dialog string item's buffer, highlighting
 * any selected text if the item is currently active.
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_update_string(DialogPtr dialog, ItemPtr item) {
	int buf;
	int cx, cy;
	int out_count;
	char *textptr;
	byte *screenptr;

	mouse_hide();

	buf = item->buf_id;
	cx = item->x2 + dialog->base_x;
	cy = item->y + dialog->base_y;

	out_count = 0;

	screenptr = screen + screen_char_add(cx, cy);
	textptr = dialog->buffer[buf];

	// First, output as much of the string as we can
	while ((*textptr != 0) && (out_count < item->buf_width)) {
		*(screenptr++) = *(textptr++);
		if (dialog->buf_select[buf] && (item->id == dialog->active_item) &&
			(((out_count >= dialog->buf_selbase[buf]) && (out_count < dialog->buf_seltarget[buf])) ||
				((out_count >= dialog->buf_seltarget[buf]) && (out_count < dialog->buf_selbase[buf])))) {
			*(screenptr++) = (char)dialog->select_color;
		} else {
			*(screenptr++) = (char)dialog->normal_color;
		}
		out_count++;
	}

	// Then, be sure the rest of the entry area is cleared
	while (out_count < item->buf_width) {
		*(screenptr++) = 0x20;
		if (dialog->buf_select[buf] &&
			(((out_count >= dialog->buf_selbase[buf]) && (out_count < dialog->buf_seltarget[buf])) ||
				((out_count >= dialog->buf_seltarget[buf]) && (out_count < dialog->buf_selbase[buf])))) {
			*(screenptr++) = (char)dialog->select_color;
		} else {
			*(screenptr++) = (char)dialog->normal_color;
		}
		out_count++;
	}

	mouse_show();
}

/**
 * Updates the list window associated with an item.
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_update_window(DialogPtr dialog, ItemPtr item) {
	int row, col;
	ListPtr list;
	int out_count;
	int my_color;
	int id;
	int start, dist, new_thumb;
	int temp_pick;
	int return_code;
	byte *screenptr;
	char *textptr;
	typedef int (*CallbackFn)(ListPtr list, DialogPtr dialog);
	CallbackFn callback;

	mouse_hide();

	if ((item->type == DD_I_LISTBASED) || (item->type == DD_I_FILENAME)) {
		item = &dialog->item[item->status];
	}

	list = &dialog->lists[item->buf_id];

	// Clear our old cursor off by wipe-ing the window in a single color
	window_color(&list->window, dialog->normal_color);

	// Now, go through the rows and columns one by one, displaying each entry
	for (col = 0; col < list->columns; col++) {
		for (row = 0; row < list->rows; row++) {
			screenptr = screen + screen_char_add(list->base_x + (col * (list->entry_width + 1)),
				list->base_y + row);
			id = list->base_entry + row + (col * list->rows);

			// Point at the text for this entry
			textptr = list->list + (id * list->element_offset);

			// Check if the cursor is on this item
			if (id != list->picked_entry) {
				my_color = dialog->normal_color;
			} else {
				my_color = dialog->select_color;
			}
			*screenptr++;
			if (*screenptr != (char)dialog->select_color) {
				*(screenptr++) = (char)my_color;
			} else {
				screenptr++;
			}
			out_count = 0;
			if (id < list->elements) {
				while ((out_count < list->entry_width) && (*textptr != 0)) {
					*(screenptr++) = *(textptr++);
					*(screenptr++) = (char)my_color;
					out_count++;
				}
			}
			while (out_count < list->entry_width) {
				*(screenptr++) = 0x20;
				*(screenptr++) = (char)my_color;
				out_count++;
			}
			*screenptr++;
			if (*screenptr != (char)dialog->select_color) {
				*(screenptr++) = (char)my_color;
			}
		}
	}


	// Update the thumb mark in a scroll bar; routine is different
	// depending on whether bar is vertical or horizontal
	temp_pick = list->picked_entry;
	if (temp_pick < 0) temp_pick = 0;

	if (list->elements > 0) {

		if (list->columns > 1) {

			start = list->window.ul_x + 2;
			dist = list->window.lr_x - list->window.ul_x - 3;

			new_thumb = (dist * temp_pick) / list->elements;

			if (new_thumb != list->thumb) {
				if (list->thumb >= 0) {
					screenptr = screen + screen_char_add(start + list->thumb, list->window.lr_y);
					*screenptr = scroll_bar;
				}
				screenptr = screen + screen_char_add(start + new_thumb, list->window.lr_y);
				*screenptr = thumb_mark;
				list->thumb = new_thumb;
			}

		} else {

			start = list->window.ul_y + 2;
			dist = list->window.lr_y - list->window.ul_y - 3;

			new_thumb = (dist * temp_pick) / list->elements;

			if (new_thumb != list->thumb) {
				if (list->thumb >= 0) {
					screenptr = screen + screen_char_add(list->window.lr_x, start + list->thumb);
					*screenptr = scroll_bar;
				}
				screenptr = screen + screen_char_add(list->window.lr_x, start + new_thumb);
				*screenptr = thumb_mark;
				list->thumb = new_thumb;
			}

		}

	}

	if ((dialog->callback != NULL) && (allow_list_callbacks)) {
		callback = (CallbackFn)dialog->callback;
		return_code = callback(list, dialog);
		if (return_code) {
			dialog->status |= DD_EXITFLAG;
			dialog_error = DD_ERR_ABORTEDBYCALLBACK;
		}
	}

	mouse_show();
}

/**
 * Shows a dialog button.  "Selected" flags whether the item is
 * currently being picked by the mouse.  Otherwise, the angle
 * brackets are automatically highlighted if the button is the
 * active button.
 *
 * @param dialog	Dialog
 * @param item		Button item
 * @param selected	Selected flag
 */
static void dialog_show_button(DialogPtr dialog, ItemPtr item, int selected) {
	int angle_color, text_color, hi_color;
	int cx, cy;

	mouse_hide();

	if (selected) {
		angle_color = text_color = hi_color = dialog->select_color;
	} else {
		text_color = dialog->normal_color;
		hi_color = dialog->hilite_color;
		if (item->id == dialog->active_button) {
			angle_color = dialog->hilite_color;
		} else {
			angle_color = dialog->normal_color;
		}
	}

	if (item->y == DD_IY_BUTTON) {
		cy = dialog->window.lr_y - 1;
	} else {
		cy = item->y + dialog->base_y;
	}
	cx = item->x + dialog->base_x;

	cx = screen_put("<", angle_color, 0, cx, cy);
	cx = screen_put(item->prompt, text_color, hi_color, cx, cy);
	cx = screen_put(">", angle_color, 0, cx, cy);

	mouse_show();
}

/**
 * Outputs a dialog message string at the appropriate location
 * in the dialog box.
 *
 * @param dialog	Dialog
 * @param item		Message string
 */
static void dialog_show_message(DialogPtr dialog, ItemPtr item) {
	mouse_hide();

	if (item->width != DD_LINEACROSS) {
		screen_put(item->prompt, dialog->normal_color, dialog->hilite_color,
			item->x + dialog->base_x, item->y + dialog->base_y);
	} else {
		window_line_across(&(dialog->window), item->y + dialog->base_y);
	}

	mouse_show();
}

/**
 * Clears a message previously output with dialog_show_message ().
 * Currently, the only kind of message that could change (and thus
 * need to be cleared) is the pathname for file items, which is
 * handled internally.
 *
 * @param dialog	Dialog
 * @param item		Message item
 */
static void dialog_clear_message(DialogPtr dialog, ItemPtr item) {
	int cx, cy;
	int count, bound;

	mouse_hide();

	cx = item->x + dialog->base_x;
	cy = item->y + dialog->base_y;
	bound = strlen(item->prompt);

	for (count = 0; count < bound; count++) {
		cx = screen_put(" ", dialog->normal_color, dialog->normal_color, cx, cy);
	}

	mouse_show();
}

/**
 * Displays (and updates) a checkbox item
 *
 * @param dialog	Dialog
 * @param item		Checkbox item
 */
static void dialog_show_checkbox(DialogPtr dialog, ItemPtr item) {
	int cx;
	int color;
	int hi_color;
	char temp1[4];
	char temp2[4];

	mouse_hide();

	if (item->status & DD_CHECK_RADIO) {
		Common::strcpy_s(temp1, "(");
		Common::strcpy_s(temp2, ") ");
	} else {
		Common::strcpy_s(temp1, "[");
		Common::strcpy_s(temp2, "] ");
	}

	color = (item->status & DD_CHECK_GREY) ? dialog_default_greyed : dialog->normal_color;
	hi_color = (item->status & DD_CHECK_GREY) ? dialog_default_greyed : dialog->hilite_color;

	cx = screen_put(temp1, color, 0, item->x + dialog->base_x, item->y + dialog->base_y);
	cx++;
	cx = screen_put(temp2, color, 0, cx, item->y + dialog->base_y);
	cx = screen_put(item->prompt, color, hi_color,
		cx, item->y + dialog->base_y);

	dialog_update_checkbox(dialog, item);

	mouse_show();
}

/**
 * Displays (and updates) a string item
 *
 * @param dialog	Dialog
 * @param item		String item
 */
static void dialog_show_string(DialogPtr dialog, ItemPtr item) {
	int cx;

	mouse_hide();

	cx = screen_put(item->prompt, dialog->normal_color, dialog->hilite_color,
		item->x + dialog->base_x, item->y + dialog->base_y);
	cx = screen_put("[", dialog->normal_color, 0, cx, item->y + dialog->base_y);

	cx = screen_put("]", dialog->normal_color, 0, cx + item->buf_width, item->y + dialog->base_y);

	dialog_update_string(dialog, item);

	mouse_show();
}

/**
 * Displays (and updates) a list-based item (including file lists)
 *
 * @param dialog	Dialog
 * @param item		List-based item
 */
static void dialog_show_window(DialogPtr dialog, ItemPtr item) {
	ListPtr list;

	mouse_hide();

	if ((item->type == DD_I_LISTBASED) || (item->type == DD_I_FILENAME)) {
		item = &dialog->item[item->status];
	}

	list = &dialog->lists[item->buf_id];

	window_wipe(&list->window);
	window_color(&list->window, dialog->normal_color);
	window_draw_box(&list->window, WINDOW_SINGLE);

	if (list->columns == 1) {
		window_vert_scrollbar(&list->window, dialog->normal_color);
	} else {
		window_horiz_scrollbar(&list->window, dialog->normal_color);
	}

	dialog_update_window(dialog, item);

	mouse_show();
}

/**
 * Displays the item whose handle is passed, no matter what type
 * of item it is (calls the appropriate one of the above routines)
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_show_any(DialogPtr dialog, ItemPtr item) {
	switch (item->type) {
	case DD_I_BUTTON:
		dialog_show_button(dialog, item, false);
		break;

	case DD_I_MESSAGE:
		dialog_show_message(dialog, item);
		break;

	case DD_I_CHECKBOX:
		dialog_show_checkbox(dialog, item);
		break;

	case DD_I_STRING:
		dialog_show_string(dialog, item);
		break;

	case DD_I_FILENAME:
		dialog_show_string(dialog, item);
		break;

	case DD_I_LISTBASED:
		dialog_show_string(dialog, item);
		break;

	case DD_I_LIST:
	case DD_I_FILELIST:
	case DD_I_DIRSLIST:
		dialog_show_window(dialog, item);
		break;
	}
}

void dialog_show_all(DialogPtr dialog) {
	ItemPtr item;
	int item_num;

	if (!(dialog->status & DD_COMPUTED)) {
		dialog_compute_window(dialog);
		dialog->status |= DD_COMPUTED;
	}

	mouse_hide();

	if (!(dialog->status & DD_SCREENSAVED)) {
		window_create(&dialog->window);
		dialog->status |= DD_SCREENSAVED;
	}

	window_wipe(&dialog->window);
	window_color(&dialog->window, dialog->normal_color);
	window_draw_box(&dialog->window, WINDOW_SINGLE);
	window_shadow(&dialog->window);

	if (dialog->button_flag) {
		window_line_across(&dialog->window, dialog->window.lr_y - 2);
	}

	for (item_num = 0; item_num < dialog->num_items; item_num++) {
		item = &dialog->item[item_num];
		dialog_show_any(dialog, item);
	}

	mouse_show();
}

/**
 * Routine to update the item whose handle is passed to it, no matter
 * what type of item it is.
 *
 * @param dialog	Dialog
 * @param item		Item
 * @param mouse_button_flag		Mouse button flag
 */
static void dialog_update_any(DialogPtr dialog, ItemPtr item, int mouse_button_flag) {
	switch (item->type) {
	case DD_I_BUTTON:
		dialog_show_button(dialog, item, mouse_button_flag);
		break;

	case DD_I_CHECKBOX:
		dialog_update_checkbox(dialog, item);
		break;

	case DD_I_STRING:
	case DD_I_FILENAME:
	case DD_I_LISTBASED:
		if (item->id != dialog->active_item) {
			dialog->buf_select[item->buf_id] = false;
		}
		dialog_update_string(dialog, item);
		break;

	case DD_I_LIST:
	case DD_I_FILELIST:
	case DD_I_DIRSLIST:
		dialog_update_window(dialog, item);
		break;
	}
}

/**
 * Moves the cursor to the active item, at the appropriate place
 * within that item (appropriate place depends on item type)
 *
 * @param dialog	Dialog
 */
static void dialog_update_cursor(DialogPtr dialog) {
	ListPtr list;
	int relval, relcol, relrow, relx, rely;

	switch (dialog->item[dialog->active_item].type) {
	case DD_I_BUTTON:
		if (dialog->item[dialog->active_item].y == DD_IY_BUTTON) {
			cursor_set_pos(dialog->base_x + dialog->item[dialog->active_item].x2,
				dialog->window.lr_y - 1);
		} else {
			cursor_set_pos(dialog->base_x + dialog->item[dialog->active_item].x2,
				dialog->base_y + dialog->item[dialog->active_item].y);
		}
		break;

	case DD_I_STRING:
	case DD_I_LISTBASED:
	case DD_I_FILENAME:
		cursor_set_pos(dialog->base_x + dialog->item[dialog->active_item].x2
			+ dialog->buf_cursor[dialog->item[dialog->active_item].buf_id],
			dialog->base_y + dialog->item[dialog->active_item].y);
		break;

	case DD_I_LIST:
	case DD_I_FILELIST:
	case DD_I_DIRSLIST:
		list = &dialog->lists[dialog->item[dialog->active_item].buf_id];
		if (list->picked_entry >= list->base_entry) {
			relval = list->picked_entry - list->base_entry;
		} else {
			relval = 0;
		}
		relcol = relval / list->rows;
		relrow = relval - (relcol * list->rows);
		relx = list->base_x + 1 + (relcol * (list->entry_width + 1));
		rely = list->base_y + relrow;
		cursor_set_pos(relx, rely);
		break;

	default:
		cursor_set_pos(dialog->base_x + dialog->item[dialog->active_item].x2,
			dialog->base_y + dialog->item[dialog->active_item].y);
		break;
	}
}

/**
 * Routine to update & move the cursor to the active item, after
 * possibly changing the active item.
 * If active item changes, then the old active item will be
 * updated also, to make sure its appearance conforms to its
 * new, inactive, status.
 *
 * @param dialog				Dialog
 * @param displace				"Displacement" by which to change the active
 * item #. 0 to leave active item unchanged.  Item #'s will loop around.
 * @param mouse_button_flag		Mouse button flag
 * @param out_of_class			Out of class flag
 */
static void dialog_update_active(DialogPtr dialog, int displace, int mouse_button_flag, int out_of_class) {
	int old_item;
	int old_button;
	int buf_num;
	int prohibited;
	ItemPtr item;
	ItemPtr old;

	old_item = dialog->active_item;
	old_button = dialog->active_button;

	do {
		dialog->active_item += displace;
		if (dialog->active_item < 0) dialog->active_item = dialog->num_items - 1;
		if (dialog->active_item >= dialog->num_items) dialog->active_item = 0;
		item = &dialog->item[dialog->active_item];
		old = &dialog->item[old_item];
		prohibited = (item->type == DD_I_MESSAGE) ||
			(out_of_class &&
				((item->type == DD_I_CHECKBOX) &&
					(old->type == DD_I_CHECKBOX) &&
					(item->_class == old->_class)));
	} while (prohibited && (displace != 0));

	if ((displace != 0) && (!mouse_button_flag)) {
		if (DialogString(dialog)) {
			buf_num = dialog->item[dialog->active_item].buf_id;
			dialog->buf_select[buf_num] = true;
			dialog->buf_selbase[buf_num] = 0;
			dialog->buf_seltarget[buf_num] = strlen(dialog->buffer[buf_num]);
			dialog->buf_cursor[buf_num] = dialog->buf_seltarget[buf_num];
		}
	}

	if (item->type == DD_I_BUTTON) {
		dialog->active_button = dialog->active_item;
	}

	if (old_item != dialog->active_item) {
		dialog_update_any(dialog, &dialog->item[old_item], false);
		dialog_update_any(dialog, item, mouse_button_flag);
		if ((old_button == old_item) && (item->type != DD_I_BUTTON)) {
			if (dialog->default_item != NULL) {
				dialog->active_button = (dialog->default_item)->id;
				dialog_update_any(dialog, dialog->default_item, false);
				dialog_update_any(dialog, &dialog->item[old_button], false);
			}
		}
		if ((old_button != old_item) && (old_button != dialog->active_button)) {
			dialog_update_any(dialog, &dialog->item[old_button], false);
		}
	} else {
		dialog_update_any(dialog, item, mouse_button_flag);
	}

	dialog_update_cursor(dialog);
}

void dialog_go_sideways(DialogPtr dialog, int direction) {
	ItemPtr current;
	ItemPtr item;
	int where_to, nearest_x, nearest_y;
	int x, y;
	int count;
	int dx, dy;

	direction = sgn(direction);

	where_to = -1;
	nearest_x = 99;
	nearest_y = 99;

	current = &dialog->item[dialog->active_item];
	x = current->x;
	y = current->y;

	for (count = 0; count < dialog->num_items; count++) {
		item = &dialog->item[count];
		dx = item->x - x;
		dy = item->y - y;
		if (item->type != DD_I_MESSAGE) {
			if (count != current->id) {
				if ((sgn(dx) == direction) && (dy <= 0)) {
					if (abs(dx) <= nearest_x) {
						if ((abs(dx) < nearest_x) || (abs(dy) < nearest_y)) {
							where_to = count;
							nearest_x = abs(dx);
							nearest_y = abs(dy);
						}
					}
				}
			}
		}
	}

	if (where_to >= 0) {
		dialog_update_active(dialog, where_to - dialog->active_item, false, false);
	} else {
		dialog_update_active(dialog, direction, false, true);
	}
}

void dialog_toggle_checkbox(DialogPtr dialog, ItemPtr item) {
	int count;
	int status, class_;
	int return_code;
	typedef int(*CallbackFn)(ItemPtr item, DialogPtr dialog);
	CallbackFn callback;

	if (item->status & DD_CHECK_RADIO) {
		if (item->status & DD_CHECK_TRUE) goto done;
	}

	item->status ^= DD_CHECK_TRUE;
	status = (item->status & DD_CHECK_TRUE);

	dialog_update_active(dialog, 0, false, false);

	if (status) {
		class_ = item->_class;

		for (count = 0; count < dialog->num_items; count++) {
			if (dialog->item[count]._class == class_) {
				if (dialog->item[count].id != item->id) {
					if (dialog->item[count].status & DD_CHECK_TRUE) {
						dialog->item[count].status &= (~DD_CHECK_TRUE);
						dialog_update_any(dialog, &dialog->item[count], false);
					}
				}
			}
		}
	}

	if (dialog->checkbox_callback != NULL) {
		callback = (CallbackFn)dialog->checkbox_callback;
		return_code = callback(item, dialog);

		if (return_code) {
			dialog->status |= DD_EXITFLAG;
			dialog_error = DD_ERR_ABORTEDBYCALLBACK;
		}
	}

done:
	;
}

/**
 * If a sub-string was being selected in the dialog box, cancel
 * that selection.
 *
 * @param dialog	Dialog
 */
static void dialog_cancel_selection(DialogPtr dialog) {
	ItemPtr item;
	int buf;

	item = &dialog->item[dialog->active_item];
	buf = item->buf_id;

	if (DialogString(dialog)) {
		if (dialog->buf_select[buf]) {
			dialog->buf_select[buf] = false;
			if (dialog->buf_selbase[buf] != dialog->buf_seltarget[buf]) {
				dialog_update_string(dialog, item);
			}
		}
	}
}

/**
 * If a substring was being selected in the dialog, erase that substring.
 *
 * @param dialog	Dialog
 */
static void dialog_wipe_selection(DialogPtr dialog) {
	ItemPtr item;
	int buf;
	int buflen;
	int from, unto;
	char *src, *dest;
	item = &dialog->item[dialog->active_item];
	buf = item->buf_id;

	if (DialogString(dialog)) {
		if (dialog->buf_select[buf]) {
			dialog_cancel_selection(dialog);
			if (dialog->buf_selbase[buf] != dialog->buf_seltarget[buf]) {
				if (dialog->buf_selbase[buf] < dialog->buf_seltarget[buf]) {
					from = dialog->buf_selbase[buf];
					unto = dialog->buf_seltarget[buf] - 1;
				} else {
					from = dialog->buf_seltarget[buf];
					unto = dialog->buf_selbase[buf] - 1;
				}
				buflen = strlen(dialog->buffer[buf]);
				if (from >= buflen) {
					from = buflen - 1;
				}
				if (unto >= buflen) {
					unto = buflen - 1;
				}
				if ((from >= 0) && (unto >= 0)) {
					dest = &dialog->buffer[buf][from];
					src = &dialog->buffer[buf][unto + 1];
					*dest = 0x00;
					Common::strcpy_s(dest, 65536, src);
					dialog->buf_cursor[buf] = from;
					dialog_update_string(dialog, item);
				}
			}
		}
	}
}

/**
 * Routine to move the cursor within a string item
 *
 * @param dialog	Dialog
 * @param displace	Displacement amount
 */
static void dialog_move_cursor(DialogPtr dialog, int displace) {
	ItemPtr item;
	int buf;

	item = &dialog->item[dialog->active_item];
	buf = item->buf_id;

	dialog_cancel_selection(dialog);

	switch (displace) {
	case DD_C_HOME:
		dialog->buf_cursor[buf] = 0;
		break;

	case DD_C_END:
		dialog->buf_cursor[buf] = strlen(dialog->buffer[buf]);
		break;

	default:
		dialog->buf_cursor[buf] += displace;
		break;
	}

	if (dialog->buf_cursor[buf] < 0) dialog->buf_cursor[buf] = 0;
	if (dialog->buf_cursor[buf] > item->buf_width - 1) dialog->buf_cursor[buf] = item->buf_width - 1;

	dialog_update_cursor(dialog);
}

/**
 * Routine to insert a character into the active string item,
 * at the current cursor location.
 *
 * @param dialog	Dialog
 * @param mykey		Character
 */
static void dialog_string_insert(DialogPtr dialog, int mykey) {
	ItemPtr item;
	int buf;
	char *dest, *src;
	int buflen, count;

	item = &dialog->item[dialog->active_item];
	buf = item->buf_id;

	if (DialogString(dialog)) {
		if (dialog->buf_select[buf]) {
			dialog_wipe_selection(dialog);
		}

		buflen = strlen(dialog->buffer[buf]);

		if (dialog->buf_cursor[buf] > buflen) {
			for (count = buflen; count < dialog->buf_cursor[buf]; count++) {
				dialog->buffer[buf][count] = 0x20;
			}
			buflen = dialog->buf_cursor[buf];
		}

		if (dialog->buf_cursor[buf] == buflen) {
			if (buflen < item->buf_width) {
				dialog->buffer[buf][buflen++] = (char)mykey;
				dialog->buffer[buf][buflen] = 0x00;
				dialog->buf_cursor[buf]++;
			}
		} else {
			if (cursor_mode == CURSOR_OVERWRITE) {
				dialog->buffer[buf][dialog->buf_cursor[buf]++] = (char)mykey;
			} else if (buflen < item->buf_width) {
				dest = dialog->buffer[buf] + buflen + 1;
				src = dest - 1;
				count = (buflen - dialog->buf_cursor[buf]) + 1;
				memmove(dest, src, count);
				dialog->buffer[buf][dialog->buf_cursor[buf]++] = (char)mykey;
			}
		}

		if (item->buf_width == 1) {
			dialog->buf_select[buf] = true;
			dialog->buf_selbase[buf] = 0;
			dialog->buf_seltarget[buf] = 1;
			dialog->buf_cursor[buf] = 0;
		}

		dialog_update_string(dialog, item);
		dialog_update_cursor(dialog);
	}
}

/**
 * Routine to delete a character or selection from the active
 * string item, at the current cursor location.
 *
 * @param dialog	Dialog
 */
static void dialog_string_delete(DialogPtr dialog) {
	ItemPtr item;
	int buf;

	item = &dialog->item[dialog->active_item];
	buf = item->buf_id;

	if (DialogString(dialog)) {
		if (!dialog->buf_select[buf]) {
			if ((unsigned)dialog->buf_cursor[buf] < strlen(dialog->buffer[buf])) {
				dialog->buf_select[buf] = true;
				dialog->buf_selbase[buf] = dialog->buf_cursor[buf];
				dialog->buf_seltarget[buf] = dialog->buf_cursor[buf] + 1;
			}
		}
		dialog_wipe_selection(dialog);
		dialog_update_cursor(dialog);
	}
}

/**
 * Routine to execute the "backspace key" function on the active
 * string item
 *
 * @param dialog	Dialog
 */
static void dialog_string_backspace(DialogPtr dialog) {
	ItemPtr item;
	int buf;
	char *dest, *src;
	int buflen;

	item = &dialog->item[dialog->active_item];
	buf = item->buf_id;

	if (DialogString(dialog)) {
		if ((dialog->buf_cursor[buf] >= dialog->buf_selbase[buf]) &&
			(dialog->buf_cursor[buf] >= dialog->buf_seltarget[buf])) {
			dialog_cancel_selection(dialog);
		}
		dialog_wipe_selection(dialog);
		if (dialog->buf_cursor[buf] > 0) {
			buflen = strlen(dialog->buffer[buf]);
			if (dialog->buf_cursor[buf] > buflen) {
				dialog->buf_cursor[buf]--;
			} else {
				dest = &dialog->buffer[buf][dialog->buf_cursor[buf] - 1];
				src = dest + 1;
				Common::strcpy_s(dest, 65536, src);
				dialog->buf_cursor[buf]--;
			}
			dialog_update_string(dialog, item);
		}
		dialog_update_cursor(dialog);
	}
}

/**
 * Routine to check if two strings are equivalent for at least
 * the entire length of the second, shorter, string.  Compare
 * is done without regard for case.
 *
 * @param big		Big string
 * @param little	Substring
 * @return Comparison result
 */
static int dialog_quick_compare(const char *big, const char *little) {
	int biglen, litlen;
	int count, going;
	char biggie, littlie;

	biglen = strlen(big);
	litlen = strlen(little);

	if (biglen >= litlen) {
		going = true;
		for (count = 0; going && (count < litlen); count++) {
			biggie = *(big++);
			littlie = *(little++);
			going = (toupper(biggie) == toupper(littlie));
		}
		return going;
	} else {
		return false;
	}
}

/**
 * Routine to find the item in a list window that most closely matches
 * a given string.
 *
 * @param dialog	Dialog
 * @param item		Item
 * @param checkstring	Matching string
 * @return	Item index
 */
static int dialog_quick_search(DialogPtr dialog, ItemPtr item, const char *checkstring) {
	ListPtr list;
	char *search;
	int result;
	int count;

	list = &dialog->lists[item->buf_id];

	result = -1;
	if (!dialog_quicksearch_flag) goto done;

	for (count = 0; (count < list->elements) && (result < 0); count++) {
		search = list->list + (count * list->element_offset);
		if (dialog_quick_compare(search, checkstring)) {
			result = count;
		}
	}

done:
	return result;
}

/**
 * Routine to perform the above quick-search function and load
 * the appropriate string into the item's string entry area.  The
 * extra text of the found entry is loaded to the right of the cursor
 * and is "selected" so that if the user types another letter key the
 * extraneous text will be automatically removed.
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_do_search(DialogPtr dialog, ItemPtr item) {
	ItemPtr listitem;
	ListPtr list;
	int buf;
	int match;
	int mylen;
	int newlen;
	int count;

	int case_magic;
	int magic_number = 0;
	int magic_upper = 0;

	buf = item->buf_id;
	listitem = &dialog->item[item->status];
	list = &dialog->lists[listitem->buf_id];

	match = dialog_quick_search(dialog, listitem, dialog->buffer[buf]);

	// Get an appropriate view of the window which will make our entry visible
	if (match >= 0) {
		if ((match >= list->base_entry) && (match < (list->base_entry + (list->rows * list->columns)))) {
			list->picked_entry = match;
		} else {
			list->picked_entry = match;
			if (list->columns == 1) {
				list->base_entry = list->picked_entry;
			} else {
				if (list->base_entry < list->picked_entry) {
					while (list->picked_entry >= (list->base_entry + (list->rows * list->columns))) {
						list->base_entry += list->rows;
					}
				} else {
					while (list->picked_entry < list->base_entry) {
						list->base_entry -= list->rows;
						if (list->base_entry < 0) list->base_entry = 0;
					}
				}
			}
		}

		allow_list_callbacks = true;

		dialog_update_any(dialog, listitem, false);

		allow_list_callbacks = false;

		mylen = strlen(dialog->buffer[buf]);

		Common::strcpy_s(temp_buf, dialog->buffer[buf]);
		strncpy(dialog->buffer[buf], list->list + (list->element_offset * list->picked_entry), item->buf_width);
		dialog->buffer[buf][item->buf_width] = 0;

		for (count = 0; count < mylen; count++) {
			dialog->buffer[buf][count] = temp_buf[count];
		}

		newlen = strlen(dialog->buffer[buf]);

		// Now, "case magic" attempts to conform the case of the searched-in
		// string to the case format that the user is typing.
		case_magic = false;

		if (mylen > 0) {
			if ((mylen > 1) && (Common::isAlpha(dialog->buffer[buf][1]))) {
				magic_number = 1;
				case_magic = true;
			} else if (Common::isAlpha(dialog->buffer[buf][0])) {
				magic_number = 0;
				case_magic = true;
			}
		}

		if (case_magic) {
			magic_upper = (Common::isUpper(dialog->buffer[buf][magic_number]));
		}

		for (count = mylen; (case_magic) && (count < newlen); count++) {
			if (magic_upper) {
				dialog->buffer[buf][count] = (char)toupper((char)dialog->buffer[buf][count]);
			} else {
				dialog->buffer[buf][count] = (char)tolower((char)dialog->buffer[buf][count]);
			}
		}

		dialog->buf_select[buf] = true;
		dialog->buf_selbase[buf] = mylen;
		dialog->buf_seltarget[buf] = newlen;

		if (item->buf_width == 1) {
			dialog->buf_selbase[buf] = 0;
			dialog->buf_seltarget[buf] = 1;
		}

		dialog_update_any(dialog, item, false);
	}
}

/**
 * Routine to change the directory of the filename window
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_set_new_directory(DialogPtr dialog, ItemPtr item) {
	int buf;
	//int newdrive;
	ItemPtr baseitem, fileitem, pathitem;

	baseitem = &dialog->item[item->status];
	fileitem = &dialog->item[baseitem->status];
	buf = baseitem->buf_id;

	Common::strcpy_s(temp_buf, dialog->buffer[buf]);
	pathitem = dialog->path_item;
#ifdef TODO
	if (temp_buf[1] == ':') {
		// Change to a new drive, if requested
		newdrive = ((int)temp_buf[0]) - 64;
		mads_chdrive(newdrive);

		dialog_clear_message(dialog, pathitem);
		//mads_getcwd(temp_buf, DIALOG_MAX_PROMPT_CHARS);
		Common::strcpy_s(temp_buf, "");
		Common::strcpy_s(pathitem->prompt, 65536, temp_buf);

	} else {
		// otherwise, change to a new directory
		mads_chdir(temp_buf);

		dialog_clear_message(dialog, pathitem);

		if (mads_getcwd(temp_buf, DIALOG_MAX_PROMPT_CHARS) != NULL) {
			Common::strcpy_s(pathitem->prompt, temp_buf);
		} else {
			Common::strcpy_s(temp_buf, pathitem->prompt);
			mads_chdir(temp_buf);
		}
	}

	// Load a wildcard into the entry area
	if ((strchr(dialog->buffer[buf], '*') == NULL) &&
		(strchr(dialog->buffer[buf], '?') == NULL)) {
		Common::strcpy_s(dialog->buffer[buf], "*.*");
		dialog->buf_select[buf] = true;
		dialog->buf_cursor[buf] = 0;
		dialog->buf_selbase[buf] = 0;
		dialog->buf_seltarget[buf] = 3;
	}

	// Load up the new directory and display
	dialog_load_directory(dialog, baseitem);
	dialog_show_any(dialog, pathitem);
	dialog_update_any(dialog, baseitem, false);
	dialog_update_any(dialog, fileitem, false);
	dialog_update_any(dialog, item, false);
	dialog_update_active(dialog, 0, false, false);
#else
	warning("TODO: dialog_set_new_directory");
#endif
}

/**
 * Routine to handle scrolling around in a list window.  Used to
 * drive both mouse and cursor scroll routines.
 *
 * @param dialog	Dialog
 * @param item		Item
 * @param direction	Scroll direction
 */
static void dialog_scroll_list(DialogPtr dialog, ItemPtr item, int direction) {
	int my_dif, my_col, my_row;
	int scroll_factor;
	ListPtr list;

	list = &dialog->lists[item->buf_id];

	if (list->picked_entry < 0) {
		list->picked_entry = list->base_entry;
	} else {
		my_dif = list->picked_entry - list->base_entry;
		my_col = my_dif / list->rows;
		my_row = my_dif % list->rows;

		switch (direction) {
		case DD_MC_SCROLL_UP:
			if (my_row > 0) {
				list->picked_entry--;
			} else {
				if ((list->columns == 1) && (list->base_entry > 0)) {
					list->base_entry--;
					list->picked_entry--;
				}
			}
			break;

		case DD_MC_SCROLL_DOWN:
			if (my_row < list->rows - 1) {
				if (list->picked_entry < list->elements - 1) {
					list->picked_entry++;
				} else {
					if ((list->columns == 1) && (list->base_entry < list->elements - 1)) {
						list->base_entry++;
					}
				}
			} else {
				if ((list->columns == 1) && (list->base_entry < list->elements - 1)) {
					list->base_entry++;
					list->picked_entry++;
					if (list->picked_entry >= list->elements) list->picked_entry = list->elements - 1;
				}
			}
			break;

		case DD_MC_SCROLL_LEFT:
			if (my_col > 0) {
				list->picked_entry -= list->rows;
			} else {
				if ((list->columns > 1) && (list->base_entry > 0)) {
					list->base_entry -= list->rows;
					list->picked_entry -= list->rows;
					if (list->base_entry < 0) list->base_entry = 0;
					if (list->picked_entry < 0) list->picked_entry = 0;
				}
			}
			break;

		case DD_MC_SCROLL_RIGHT:
			if (my_col < list->columns - 1) {
				list->picked_entry += list->rows;
				if (list->picked_entry >= list->elements) {
					list->picked_entry = list->elements - 1;
				}
			} else {
				if ((list->columns > 1) && (list->base_entry < list->elements - 1)) {
					list->base_entry += list->rows;
					list->picked_entry += list->rows;
					if (list->base_entry >= list->elements) {
						list->base_entry -= list->rows;
					}
					if (list->picked_entry >= list->elements) {
						list->picked_entry = list->elements - 1;
					}
				}
			}
			break;

		case DD_MC_SCROLL_PAGE_UP:
			list->base_entry -= list->rows;
			if (list->base_entry < 0) {
				list->base_entry = 0;
				list->picked_entry = 0;
			} else {
				if ((list->picked_entry - list->base_entry) >= list->rows) {
					list->picked_entry = list->base_entry + list->rows - 1;
				} else {
					list->picked_entry = list->base_entry;
				}
			}
			break;

		case DD_MC_SCROLL_PAGE_DOWN:
			list->base_entry += list->rows;
			if (list->base_entry >= list->elements) {
				list->base_entry = list->elements - 1;
				list->picked_entry = list->elements - 1;
			} else {
				if (list->picked_entry < list->base_entry) {
					list->picked_entry = list->base_entry;
				} else {
					list->picked_entry = list->base_entry + list->rows - 1;
					if (list->picked_entry >= list->elements) {
						list->picked_entry = list->elements;
					}
				}
			}
			break;

		case DD_MC_SCROLL_PAGE_LEFT:
			scroll_factor = list->rows * (list->columns - 1);
			list->base_entry -= scroll_factor;
			list->picked_entry -= scroll_factor;
			if (list->base_entry < 0) list->base_entry = 0;
			if (list->picked_entry < list->base_entry) list->picked_entry = list->base_entry;
			break;

		case DD_MC_SCROLL_PAGE_RIGHT:
			scroll_factor = list->rows * (list->columns - 1);
			if (list->base_entry + scroll_factor < list->elements) {
				list->base_entry += scroll_factor;
				list->picked_entry += scroll_factor;
				if (list->picked_entry >= list->elements) {
					list->picked_entry = list->elements - 1;
				}
			} else {
				list->picked_entry = list->elements - 1;
			}
			break;
		}
	}
}

/**
 * Routine to allow keyboard scrolling within a list window
 *
 * @param dialog	Dialog
 * @param item		Item
 * @param mykey		Scrolling character
 */
static void dialog_key_exec_list(DialogPtr dialog, ItemPtr item, int mykey) {
	ItemPtr baseitem;
	ListPtr list;
	int buf;

	switch (mykey) {
	case up_key:
		dialog_scroll_list(dialog, item, DD_MC_SCROLL_UP);
		break;

	case down_key:
		dialog_scroll_list(dialog, item, DD_MC_SCROLL_DOWN);
		break;

	case left_key:
		dialog_scroll_list(dialog, item, DD_MC_SCROLL_LEFT);
		break;

	case right_key:
		dialog_scroll_list(dialog, item, DD_MC_SCROLL_RIGHT);
		break;

	case pgup_key:
		list = &dialog->lists[item->buf_id];
		if (list->columns > 1) {
			dialog_scroll_list(dialog, item, DD_MC_SCROLL_PAGE_LEFT);
		} else {
			dialog_scroll_list(dialog, item, DD_MC_SCROLL_PAGE_UP);
		}
		break;

	case pgdn_key:
		list = &dialog->lists[item->buf_id];
		if (list->columns > 1) {
			dialog_scroll_list(dialog, item, DD_MC_SCROLL_PAGE_RIGHT);
		} else {
			dialog_scroll_list(dialog, item, DD_MC_SCROLL_PAGE_DOWN);
		}
		break;
	}

	baseitem = &dialog->item[item->status];
	list = &dialog->lists[item->buf_id];
	buf = baseitem->buf_id;
	strncpy(dialog->buffer[buf], list->list + (list->picked_entry * list->element_offset), baseitem->buf_width);
	dialog->buffer[buf][baseitem->buf_width] = 0;
	fileio_purge_trailing_spaces(dialog->buffer[buf]);
	allow_list_callbacks = true;
	dialog_update_any(dialog, baseitem, false);
	dialog_update_active(dialog, 0, false, false);
	allow_list_callbacks = false;
}

/**
 * Returns true if the mouse is in the specified item.
 *
 * @param dialog	Dialog
 * @param count		Item index
 * @return True if mouse is within the item
 */
static int in_item(DialogPtr dialog, int count) {
	ItemPtr item;
	ListPtr list;
	int result;

	item = &dialog->item[count];

	switch (item->type) {
	case DD_I_MESSAGE:
		result = false;
		break;

	case DD_I_BUTTON:
		if (item->y == DD_IY_BUTTON) {
			result = mouse_in_box(dialog->base_x + item->x, dialog->window.lr_y - 1,
				dialog->base_x + item->x + item->width - 1, dialog->window.lr_y);
		} else {
			result = mouse_in_box(dialog->base_x + item->x, dialog->base_y + item->y,
				dialog->base_x + item->x + item->width - 1, dialog->base_y + item->y);
		}
		break;

	case DD_I_CHECKBOX:
		result = mouse_in_box(dialog->base_x + item->x, dialog->base_y + item->y,
			dialog->base_x + item->x + item->width - 1, dialog->base_y + item->y);
		break;

	case DD_I_STRING:
	case DD_I_LISTBASED:
	case DD_I_FILENAME:
		result = mouse_in_box(dialog->base_x + item->x2, dialog->base_y + item->y,
			dialog->base_x + item->x2 + item->buf_width, dialog->base_y + item->y);
		break;

	case DD_I_LIST:
	case DD_I_FILELIST:
	case DD_I_DIRSLIST:
		list = &dialog->lists[item->buf_id];
		result = mouse_in_box(list->base_x, list->base_y,
			list->window.lr_x, list->window.lr_y);
		if ((mouse_x == list->window.lr_x) && (mouse_y == list->window.lr_y)) {
			result = false;
		}
		if (list->elements == 0) result = false;

		break;

	default:
		result = false;
		break;
	}

	return result;
}

/**
 * If the mouse has been pressed within a scroll bar area, returns
 * the proper type of action to be taken.
 *
 * @param list		Listbox
 * @param button	Button number
 * @return	Action to be taken
 */
static int in_scroll_box(ListPtr list, int button) {
	int result;
	int relx, rely;

	result = 0;

	if (list->columns > 1) {
		if (mouse_in_box(list->window.ul_x + 1, list->window.lr_y,
			list->window.lr_x - 1, list->window.lr_y)) {
			if (mouse_x == list->window.ul_x + 1) {
				result = DD_MC_SCROLL_LEFT;
			} else if (mouse_x == list->window.lr_x - 1) {
				result = DD_MC_SCROLL_RIGHT;
			} else {
				relx = mouse_x - (list->window.ul_x + 2);
				if (((relx < list->thumb) || ((relx == list->thumb) && (mouse_stroke_type == DD_MC_SCROLL_PAGE_LEFT) && (!mouse_start_stroke))) && (button == 0)) {
					result = DD_MC_SCROLL_PAGE_LEFT;
				} else if (((relx > list->thumb) || ((relx == list->thumb) && (mouse_stroke_type == DD_MC_SCROLL_PAGE_RIGHT) && (!mouse_start_stroke))) && (button == 0)) {
					result = DD_MC_SCROLL_PAGE_RIGHT;
				} else {
					result = DD_MC_SCROLL;
				}
			}
		}
	} else {
		if (mouse_in_box(list->window.lr_x, list->window.ul_y + 1,
			list->window.lr_x, list->window.lr_y - 1)) {
			if (mouse_y == list->window.ul_y + 1) {
				result = DD_MC_SCROLL_UP;
			} else if (mouse_y == list->window.lr_y - 1) {
				result = DD_MC_SCROLL_DOWN;
			} else {
				rely = mouse_y - (list->window.ul_y + 2);
				if (((rely < list->thumb) || ((rely == list->thumb) && (mouse_stroke_type == DD_MC_SCROLL_PAGE_UP) && (!mouse_start_stroke))) && (button == 0)) {
					result = DD_MC_SCROLL_PAGE_UP;
				} else if (((rely > list->thumb) || ((rely == list->thumb) && (mouse_stroke_type == DD_MC_SCROLL_PAGE_DOWN) && (!mouse_start_stroke))) && (button == 0)) {
					result = DD_MC_SCROLL_PAGE_DOWN;
				} else {
					result = DD_MC_SCROLL;
				}
			}
		}
	}

	return result;
}

/**
 * Routine to handle selection of a substring with the mouse
 *
 * @param dialog	Dialog
 * @param item		String item
 */
static void dialog_exec_mouse_string(DialogPtr dialog, ItemPtr item) {
	int buf;

	if (mouse_y == (dialog->base_y + item->y)) {
		buf = item->buf_id;
		dialog->buf_cursor[buf] = mouse_x - (dialog->base_x + item->x2);
		if (dialog->buf_cursor[buf] < 0) dialog->buf_cursor[buf] = 0;
		if (dialog->buf_cursor[buf] > item->buf_width) dialog->buf_cursor[buf] = item->buf_width;
		if (mouse_start_stroke) {
			if (dialog->active_item != item->id) {
				dialog->buf_select[buf] = true;
				dialog->buf_selbase[buf] = 0;
				dialog->buf_seltarget[buf] = strlen(dialog->buffer[buf]);
				dialog->buf_cursor[buf] = dialog->buf_seltarget[buf];
			} else {
				dialog->buf_select[buf] = true;
				dialog->buf_selbase[buf] = dialog->buf_cursor[buf];
				dialog->buf_seltarget[buf] = dialog->buf_cursor[buf];
			}
		} else {
			dialog->buf_seltarget[buf] = dialog->buf_cursor[buf];
		}
	}
}

/**
 * Routine to deal with the mouse when it is doing something
 * in a list window (clicking, double-clicking, or dragging).
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_exec_mouse_list(DialogPtr dialog, ItemPtr item) {
	ListPtr list;
	ItemPtr baseitem;
	int test;
	int looking, look_x, look_y, look_x2, look_y2;
	int row, col;
	int buf;
	int old_pick;
	int abort;
	static long mouse_list_timing;
	static long mouse_drag_timing;

	if (mouse_start_stroke) mouse_drag_timing = timer_read_dos() + MOUSE_TIMING_TWO;

	list = &dialog->lists[item->buf_id];
	test = in_scroll_box(list, mouse_button);
	if (test && mouse_start_stroke) {
		mouse_stroke_type = test;
		mouse_scroll_list = list;
		mouse_scroll_item = item;
	} else {
		looking = true;

		// Find out which list entry is being picked
		for (col = 0; (col < list->columns) && (looking); col++) {
			for (row = 0; (row < list->rows) && (looking); row++) {
				look_x = list->base_x + ((list->entry_width + 1) * col);
				look_x2 = look_x + list->entry_width + 1;
				look_y = list->base_y + row;
				look_y2 = look_y;
				if (list->columns == 1) {
					if (col == 0) look_x = 0;
					if (col == list->columns - 1) look_x2 = screen_max_x;
				} else {
					if (row == 0) look_y = 0;
					if (row == list->rows - 1) look_y2 = screen_max_y;
				}

				if (mouse_in_box(look_x, look_y, look_x2, look_y2)) {
					looking = false;
					old_pick = list->picked_entry;
					list->picked_entry = list->base_entry + row + (col * list->rows);
					if (list->picked_entry >= list->elements) {
						list->picked_entry = list->elements - 1;
					}

					abort = false;

					// Handle double-click if any
					if (mouse_start_stroke && (old_pick == list->picked_entry)) {
						if ((long)timer_read_dos() <= mouse_list_timing + MOUSE_DOUBLE_TIMING) {
							if (item->type == DD_I_DIRSLIST) {
								dialog_set_new_directory(dialog, item);
								abort = true;
							} else {
								dialog->active_item = (dialog->default_item)->id;
								dialog->status |= DD_EXITFLAG;
							}
						}
					}
					if (mouse_start_stroke) mouse_list_timing = timer_read_dos();

					if (!abort) {
						baseitem = &dialog->item[item->status];
						buf = baseitem->buf_id;
						strncpy(dialog->buffer[buf], list->list + (list->picked_entry * list->element_offset), baseitem->buf_width);
						dialog->buffer[buf][baseitem->buf_width] = 0;
						fileio_purge_trailing_spaces(dialog->buffer[buf]);
						dialog_update_any(dialog, baseitem, false);
					}
				}
			}
		}

		// Handle click-and-drag
		if (looking && (mouse_start_stroke || ((long)timer_read_dos() >= mouse_drag_timing))) {
			mouse_override = true;
			mouse_drag_timing = (long)timer_read_dos() + MOUSE_TIMING_TWO;
			if (list->columns > 1) {
				if (mouse_x < list->base_x) {
					dialog_scroll_list(dialog, item, DD_MC_SCROLL_LEFT);
				} else if (mouse_x >= list->base_x + ((list->entry_width + 1) * list->columns)) {
					dialog_scroll_list(dialog, item, DD_MC_SCROLL_RIGHT);
				}
			} else {
				if (mouse_y < list->base_y) {
					dialog_scroll_list(dialog, item, DD_MC_SCROLL_UP);
				} else if (mouse_y >= list->base_y + list->rows) {
					dialog_scroll_list(dialog, item, DD_MC_SCROLL_DOWN);
				}
			}
			baseitem = &dialog->item[item->status];
			buf = baseitem->buf_id;
			strncpy(dialog->buffer[buf], list->list + (list->picked_entry * list->element_offset), baseitem->buf_width);
			dialog->buffer[buf][baseitem->buf_width] = 0;
			fileio_purge_trailing_spaces(dialog->buffer[buf]);
			dialog_update_any(dialog, baseitem, false);
		}
	}
}

/**
 * Deals with the mouse when it is in a scroll bar.  Calls
 * dialog_scroll_list () for most functions, but handles click-and-drag
 * on the thumb mark itself.
 *
 * @param dialog	Dialog
 * @param item		Item
 */
static void dialog_exec_mouse_scroll(DialogPtr dialog, ItemPtr item) {
	ListPtr list;
	ItemPtr baseitem;
	int buf;
	int mouse_thumb, dist;
	int now_scroll;

	list = mouse_scroll_list;
	now_scroll = in_scroll_box(mouse_scroll_list, mouse_button);
	if (mouse_stroke_type != DD_MC_SCROLL) {
		if (list->picked_entry < 0) list->picked_entry = list->base_entry;
		if (mouse_stroke_type == now_scroll) {
			dialog_scroll_list(dialog, item, mouse_stroke_type);
		}
	} else {
		if (list->columns > 1) {
			mouse_thumb = mouse_x - (list->window.ul_x + 2);
			dist = list->window.lr_x - list->window.ul_x - 4;

			if (mouse_thumb < 0)     mouse_thumb = 0;
			if (mouse_thumb >= dist) mouse_thumb = dist;

			if ((mouse_thumb != list->thumb) || (mouse_button == 1) || (mouse_start_stroke)) {
				if (mouse_thumb == 0) {
					list->base_entry = 0;
					list->picked_entry = 0;
				} else if (mouse_thumb >= dist - 1) {
					list->picked_entry = list->elements - 1;
					while (list->picked_entry >= list->base_entry + (list->rows * list->columns)) {
						list->base_entry += list->rows;
					}
				} else {
					list->picked_entry = (mouse_thumb * list->elements) / dist;
					while (list->picked_entry < list->base_entry) {
						list->base_entry -= list->rows;
						if (list->base_entry < 0) list->base_entry = 0;
					}
					while (list->picked_entry >= (list->base_entry + (list->rows * list->columns))) {
						list->base_entry += list->rows;
					}
				}
			}

		} else {
			mouse_thumb = mouse_y - (list->window.ul_y + 2);
			dist = list->window.lr_y - list->window.ul_y - 4;

			if (mouse_thumb < 0)     mouse_thumb = 0;
			if (mouse_thumb >= dist) mouse_thumb = dist;

			if ((mouse_thumb != list->thumb) || (mouse_button == 1)) {
				if (mouse_thumb == 0) {
					list->base_entry = 0;
					list->picked_entry = 0;
				} else if (mouse_thumb >= dist - 1) {
					list->base_entry = list->elements - list->rows;
					list->picked_entry = list->elements - 1;
					if (list->base_entry < 0) list->base_entry = 0;
				} else {
					list->picked_entry = (mouse_thumb * list->elements) / dist;
					if (list->picked_entry < list->base_entry) list->base_entry = list->picked_entry;
					if (list->picked_entry >= (list->base_entry + list->rows)) {
						list->base_entry = (list->picked_entry - list->rows) + 1;
					}
				}
			}
		}
	}

	baseitem = &dialog->item[mouse_scroll_item->status];
	buf = baseitem->buf_id;
	strncpy(dialog->buffer[buf], list->list + (list->picked_entry * list->element_offset), baseitem->buf_width);
	dialog->buffer[buf][baseitem->buf_width] = 0;
	fileio_purge_trailing_spaces(dialog->buffer[buf]);
	dialog_update_any(dialog, baseitem, false);
}

void dialog_set_stroke_type(DialogPtr dialog, int type, int toggle) {
	mouse_stroke_type = type;
	if (toggle) {
		dialog_toggle_checkbox(dialog, &dialog->item[type]);
	}
}

/**
 * Called whenever a mouse stroke is detected; loops until the
 * mouse button is released.  Performs whatever actions are necessary
 * to execute the mouse function.
 *
 * @param dialog	Dialog
 */
static void dialog_exec_mouse(DialogPtr dialog) {
	int count, count2;
	int refresh;
	int radio, s1, s2;
	ItemPtr item = nullptr;
	long mouse_scroll_clock;
	long mouse_ballistic_clock;

	mouse_init_cycle();

	mouse_stroke_going = false;  // (so that "start stroke" will be true)

	mouse_stroke_type = DD_MC_NONE;
	mouse_override = false;

	mouse_scroll_clock = 0;
	mouse_ballistic_clock = 0;

	do {
		mouse_begin_cycle(false);

		if (mouse_changed ||
			(mouse_stroke_type < DD_MC_SCROLL) || (mouse_override)) {

			resolved_mouse = false;

			for (count = 0; (count < dialog->num_items) && (!resolved_mouse); count++) {
				if (in_item(dialog, count)) {
					if (mouse_start_stroke) {
						radio = (dialog->item[count].type == DD_I_CHECKBOX) &&
							(dialog->item[count].status & DD_CHECK_RADIO) != 0;
						if (radio) {
							for (count2 = 0; count2 < dialog->num_items; count2++) {
								if (dialog->item[count2].type == DD_I_CHECKBOX) {
									if (dialog->item[count2]._class == dialog->item[count]._class) {
										if (dialog->item[count2].status & DD_CHECK_TRUE) {
											mouse_orig_stroke_type = count2;
										}
									}
								}
							}
						}
						dialog_set_stroke_type(dialog, count, radio);
					} else if ((dialog->item[count].type == DD_I_BUTTON) &&
						(dialog->item[mouse_stroke_type].type == DD_I_BUTTON)) {
						dialog_set_stroke_type(dialog, count, false);
					} else if ((dialog->item[count].type == DD_I_CHECKBOX) &&
						(dialog->item[mouse_stroke_type].type == DD_I_CHECKBOX)) {
						s1 = dialog->item[count].status;
						s2 = dialog->item[mouse_stroke_type].status;
						if ((s1 & DD_CHECK_RADIO) == (s2 & DD_CHECK_RADIO)) {
							if (s1 & DD_CHECK_RADIO) {
								if (dialog->item[count]._class == dialog->item[mouse_stroke_type]._class) {
									dialog_set_stroke_type(dialog, count, !mouse_stop_stroke);
								}
							} else {
								dialog_set_stroke_type(dialog, count, false);
							}
						}
					}
					if (mouse_stroke_type == count) {
						resolved_mouse = true;
					}
				}
			}

			if (mouse_stroke_type != DD_MC_NONE) {
				refresh = false;
				if (mouse_stroke_type >= 0) {
					item = &dialog->item[mouse_stroke_type];
					if ((item->type == DD_I_BUTTON) || (item->type == DD_I_CHECKBOX)) {
						refresh = true;
					}
					if ((item->type == DD_I_STRING) || (item->type == DD_I_LISTBASED) || (item->type == DD_I_FILENAME)) {
						dialog_exec_mouse_string(dialog, item);
						refresh = true;
					}
					if ((item->type == DD_I_LIST) || (item->type == DD_I_FILELIST) || (item->type == DD_I_DIRSLIST)) {
						dialog_exec_mouse_list(dialog, item);
						refresh = true;
					}
				}
				if (mouse_stroke_type <= DD_MC_SCROLL) {

					item = mouse_scroll_item;

					if (mouse_start_stroke ||
						((long)timer_read_dos() >= mouse_scroll_clock)) {
						dialog_exec_mouse_scroll(dialog, item);
						refresh = true;
					}

					if (refresh) {
						if (mouse_start_stroke) {
							mouse_scroll_clock = timer_read_dos() + MOUSE_TIMING_ONE;
							mouse_ballistic_clock = timer_read_dos() + MOUSE_BALLISTIC_TIMING;
						} else {
							if ((long)timer_read_dos() < mouse_ballistic_clock) {
								mouse_scroll_clock = timer_read_dos() + MOUSE_TIMING_TWO;
							}
						}
					}
				}
				if (refresh && (!(dialog->status & DD_EXITFLAG)))
					dialog_update_active(dialog, item->id - dialog->active_item, true, false);
			}
		}

		stroke_going = mouse_stroke_going && (!(dialog->status & DD_EXITFLAG));

		mouse_end_cycle(false, true);

	} while (stroke_going);

	allow_list_callbacks = true;

	dialog_update_active(dialog, 0, false, false);

	allow_list_callbacks = false;

	if (mouse_stroke_type >= 0) {
		if (in_item(dialog, mouse_stroke_type)) {
			item = &dialog->item[mouse_stroke_type];

			switch (item->type) {
			case DD_I_BUTTON:
				dialog->status |= DD_EXITFLAG;
				break;

			case DD_I_CHECKBOX:
				dialog_toggle_checkbox(dialog, item);
				break;

			}
		} else {
			if ((dialog->item[mouse_orig_stroke_type].type == DD_I_CHECKBOX) &&
				(dialog->item[mouse_orig_stroke_type].status & DD_CHECK_RADIO)) {
				dialog_toggle_checkbox(dialog, &dialog->item[mouse_orig_stroke_type]);
				dialog_update_any(dialog, &dialog->item[mouse_orig_stroke_type], false);
			}
		}
	}
}

ItemPtr dialog_execute(DialogPtr dialog, ItemPtr active_item, ItemPtr default_button, KeyPtr key_buffer) {
	int mykey;
	int count;
	int buf;
	int savedpath;
	int savedrive = 0;
	int return_code;
	char savepath[80];
	char mypath[80];
	char mypath2[80];
	long base_time;
	long now_time;
	long my_timeout;
	long diff;
	int mydrive;
	ItemPtr item;
	struct KeyBuffer tempkey;

	allow_list_callbacks = false;

	if (key_buffer == NULL) {
		key_buffer = &tempkey;
		tempkey.len = 0;
	}

	if (dialog->status & DD_FILEMENU) {
		savedpath = (mads_getcwd(savepath, 80) != NULL);
		savedrive = mads_getdrive();

		if (dialog->path_item != NULL) {
			Common::strcpy_s(mypath, (dialog->path_item)->prompt);
			mads_fullpath(mypath2, mypath, 80);
			mydrive = ((int)mypath2[0]) - '@';
			mads_chdrive(mydrive);
			mads_chdir(mypath2);
		}
	}

	return_code = false;

	if (dialog->cancel_item == NULL) dialog->cancel_item = default_button;

	if (default_button == NULL) {
		dialog->default_item = dialog->cancel_item;
		dialog->active_button = (dialog->cancel_item)->id;
	} else {
		dialog->default_item = default_button;
		dialog->active_button = default_button->id;
	}

	if (active_item != NULL) {
		dialog->active_item = active_item->id;
	}

	dialog_show_all(dialog);

	dialog_update_active(dialog, 0, false, false);

	mouse_init_cycle();

	base_time = timer_read_dos();

	my_timeout = dialog_timeout;

	while (!(dialog->status & DD_EXITFLAG)) {

		if (my_timeout) {
			now_time = timer_read_dos();
			diff = (now_time - base_time);
			if (diff >= 0) my_timeout--;
			if (!my_timeout) dialog->status |= DD_EXITFLAG;
			base_time = now_time;
		}

		mouse_begin_cycle(true);

		keys_fill_buffer(key_buffer);

		mykey = keys_read_buffer(key_buffer);

		item = &dialog->item[dialog->active_item];

		if (mykey && !DialogString(dialog)) {

			switch (mykey) {
			case esc_key:
				dialog->status |= DD_EXITFLAG;
				dialog->active_item = (dialog->cancel_item)->id;
				break;

			case tab_key:
				dialog_update_active(dialog, +1, false, true);
				break;

			case backtab_key:
				dialog_update_active(dialog, -1, false, true);
				break;

			case space_key:
				switch (item->type) {
				case DD_I_BUTTON:
					dialog->status |= DD_EXITFLAG;
					break;
				case DD_I_CHECKBOX:
					dialog_toggle_checkbox(dialog, item);
					break;
				}
				break;

			case up_key:
			case down_key:
			case left_key:
			case right_key:
			case pgup_key:
			case pgdn_key:
				switch (item->type) {
				case DD_I_LIST:
				case DD_I_FILELIST:
				case DD_I_DIRSLIST:
					dialog_key_exec_list(dialog, item, mykey);
					break;

				default:
					switch (mykey) {
					case up_key:
						dialog_update_active(dialog, -1, false, false);
						break;

					case down_key:
						dialog_update_active(dialog, +1, false, false);
						break;

					case right_key:
						dialog_go_sideways(dialog, +1);
						break;

					case left_key:
						dialog_go_sideways(dialog, -1);
						break;
					}
					break;
				}
				break;

			case enter_key:
				switch (item->type) {
				case DD_I_DIRSLIST:
					dialog_set_new_directory(dialog, item);
					break;

				case DD_I_BUTTON:
					dialog->status |= DD_EXITFLAG;
					break;

				default:
					dialog->active_item = (dialog->default_item)->id;
					dialog->status |= DD_EXITFLAG;
					break;
				}
				break;

			case ctrl_q_key:
				dialog_quicksearch_flag = !dialog_quicksearch_flag;
				break;

			default:
				mykey = keys_fix_alt(mykey);
				if (Common::isPrint(mykey) && (mykey < 0x100)) {
					for (count = 0; count < dialog->num_items; count++) {
						if (toupper(mykey) == toupper(dialog->item[count].keystroke)) {
							dialog_update_active(dialog, count - dialog->active_item, false, false);
							item = &dialog->item[dialog->active_item];
							switch (item->type) {

							case DD_I_BUTTON:
								dialog->status |= DD_EXITFLAG;
								break;

							case DD_I_CHECKBOX:
								dialog_toggle_checkbox(dialog, item);
								break;
							}
						}
					}
				}
				break;
			}

		} else if (mykey) {
			switch (mykey) {
			case esc_key:
				dialog->status |= DD_EXITFLAG;
				dialog->active_item = (dialog->cancel_item)->id;
				break;

			case enter_key:
				switch (item->type) {
				case DD_I_FILENAME:
					buf = item->buf_id;
					if (((strchr(dialog->buffer[buf], '*') != NULL) ||
						(strchr(dialog->buffer[buf], '?') != NULL)) &&
						!dialog_wildcard_exits) {
						dialog_load_directory(dialog, item);
						dialog->buf_select[buf] = true;
						dialog->buf_selbase[buf] = 0;
						dialog->buf_seltarget[buf] = strlen(dialog->buffer[buf]);
						dialog_update_active(dialog, 0, false, false);
						dialog_update_any(dialog, &dialog->item[item->status], false);
						dialog_update_any(dialog, &dialog->item[item->status + 1], false);
					} else {
						dialog->status |= DD_EXITFLAG;
					}
					break;

				default:
					dialog->status |= DD_EXITFLAG;
					break;
				}
				break;

			case pgup_key:
			case pgdn_key:
				if (page_callback != NULL) {
					page_callback(mykey, item, dialog);
					dialog->active_item -= 1;
					dialog_update_active(dialog, 1, false, false);
				}
				break;

			case tab_key:
				dialog_update_active(dialog, +1, false, true);
				break;

			case down_key:
				dialog_update_active(dialog, +1, false, false);
				break;

			case backtab_key:
				dialog_update_active(dialog, -1, false, true);
				break;

			case up_key:
				dialog_update_active(dialog, -1, false, false);
				break;

			case ins_key:
				cursor_toggle_insert();
				break;

			case del_key:
				dialog_string_delete(dialog);
				if ((item->type == DD_I_LISTBASED) || (item->type == DD_I_FILENAME)) {
					dialog_do_search(dialog, item);
				}
				break;

			case bksp_key:
				dialog_string_backspace(dialog);
				if ((item->type == DD_I_LISTBASED) || (item->type == DD_I_FILENAME)) {
					dialog_do_search(dialog, item);
				}
				break;

			case left_key:
				dialog_move_cursor(dialog, -1);
				break;

			case right_key:
				dialog_move_cursor(dialog, 1);
				break;

			case home_key:
				dialog_move_cursor(dialog, DD_C_HOME);
				break;

			case end_key:
				dialog_move_cursor(dialog, DD_C_END);
				break;

			case ctrl_q_key:
				dialog_quicksearch_flag = !dialog_quicksearch_flag;
				break;

			default:
				if ((mykey >= 0x100) && (keys_fix_alt(mykey) != mykey)) {
					mykey = keys_fix_alt(mykey);
					for (count = 0; count < dialog->num_items; count++) {
						if (toupper(mykey) == toupper(dialog->item[count].keystroke)) {
							dialog_update_active(dialog, count - dialog->active_item, false, false);
							item = &dialog->item[dialog->active_item];
							switch (item->type) {

							case DD_I_BUTTON:
								dialog->status |= DD_EXITFLAG;
								break;

							case DD_I_CHECKBOX:
								dialog_toggle_checkbox(dialog, item);
								break;
							}
						}
					}
				} else if (Common::isPrint(mykey) && (mykey < 0x100)) {
					dialog_string_insert(dialog, mykey);
					if ((item->type == DD_I_LISTBASED) || (item->type == DD_I_FILENAME)) {
						dialog_do_search(dialog, item);
					}
				}
				break;

			}
		}

		if (mouse_status == 0) {
			mouse_button = -1;
		}

		if ((mouse_button != -1) && mouse_start_stroke) {
			if ((mouse_video_mode == screen_video_mode) &&
				(mouse_in_box(dialog->window.ul_x, dialog->window.ul_y,
					dialog->window.lr_x, dialog->window.lr_y))) {

				dialog_exec_mouse(dialog);

			} else if ((mouse_video_mode != screen_video_mode) || (!mouse_callback_double_only)) {
				if (mouse_callback != NULL) {
					return_code = mouse_callback(mouse_button, mouse_status, mouse_x, mouse_y, mouse_video_mode);
					if (return_code) {
						dialog->status |= DD_EXITFLAG;
						dialog_error = DD_ERR_ABORTEDBYCALLBACK;
					}
				}
			}
		}

		mouse_end_cycle((!(dialog->status & DD_EXITFLAG)) && !mouse_stroke_going, true);
	}

	if (dialog->status & DD_FILEMENU) {
		mads_chdir(savepath);
		mads_chdrive(savedrive);
	}

	if (!return_code) {
		item = &dialog->item[dialog->active_item];
	} else {
		item = NULL;
	}

	return item;
}

char *dialog_select_file(const char *prompt, const char *path, const char *filespec, char *output) {
	DialogPtr dialog;
	ItemPtr ok, first_item, result;
	char *filebuf;
	char *dirbuf;

	filebuf = (char *)mem_get(3000);
	if (filebuf == NULL) return NULL;

	dirbuf = (char *)mem_get(1500);
	if (dirbuf == NULL) {
		mem_free(filebuf);
		return NULL;
	}

	dialog = dialog_file_create(NULL, DD_CENTER, DD_CENTER,
		DD_DEFAULT, DD_DEFAULT, DD_DEFAULT,
		&ok, &first_item,
		filespec, path, screen_max_y - 18,
		filebuf, 200, dirbuf, 100, prompt);

	result = dialog_execute(dialog, first_item, ok, NULL);

	if (result != dialog->cancel_item) {
		Common::strcpy_s(output, 65536, dialog_read_filepath(dialog, first_item));
	} else {
		output = NULL;
	}

	dialog_destroy(dialog);

	mem_free(filebuf);
	mem_free(dirbuf);

	return output;
}

char *dialog_enter_string(char *reply, const char *top_prompt, const char *left_prompt,
		char *my_default, int maxlen) {
	char work[100];

	DialogPtr dialog;
	ItemPtr string, ok, cancel, result;

	if (strlen(my_default) > (size_t)maxlen)
		my_default[maxlen - 1] = 0;  // Trim default

	if (left_prompt == NULL)
		Common::strcpy_s(work, "~Reply: ");
	else
		Common::strcpy_s(work, left_prompt);

	dialog = dialog_create(NULL, DD_CENTER, DD_CENTER, DD_AUTO,
		DD_DEFAULT, DD_DEFAULT, DD_DEFAULT);

	dialog_add_blank(dialog);
	dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, top_prompt);
	dialog_add_blank(dialog);
	string = dialog_add_string(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, work,
		my_default, maxlen);
	dialog_add_blank(dialog);
	ok = dialog_add_button(dialog, DD_IX_LEFT, DD_IY_BUTTON, "  OK  ");
	cancel = dialog_add_button(dialog, DD_IX_RIGHT, DD_IY_BUTTON, "Cancel");

	result = dialog_execute(dialog, string, ok, NULL);

	Common::strcpy_s(work, dialog_read_string(dialog, string));

	dialog_destroy(dialog);

	if (result == cancel)
		return(NULL);

	Common::strcpy_s(reply, 65536, work);
	return(reply);
}

int dialog_enter_int(char *top_prompt, int my_default) {
	char work[20], work1[20];

	if (my_default != 0)
		mads_itoa(my_default, work, 10);
	else
		Common::strcpy_s(work, "");

	if (dialog_enter_string(work1, top_prompt, "~Value:", work, 12) == NULL)
		return(0);

	return atoi(work1);
}

DialogPtr dialog_create_default(void) {
	return(dialog_create(NULL, DD_CENTER, DD_CENTER, DD_AUTO,
		DD_DEFAULT, DD_DEFAULT, DD_DEFAULT));
}

void dialog_set_alert_colors(int normal, int select, int hilite) {
	alert_normal_color = normal;
	alert_select_color = select;
	alert_hilite_color = hilite;
}

int dialog_alert(int x, int y, int buttons, const char *string1,
		const char *string2, const char *string3, const char *string4) {
	DialogPtr dialog;
	int num_buttons;
	int button_width;
	int num_strings;
	int string_width;
	int dialog_width;
	int sw1, sw2, sw3, sw4;
	int returnval;
	int normal_x, cancel_x, no_x;
	char buf[80];
	struct KeyBuffer key_buf;
	ItemPtr yesitem, noitem, okitem, cancelitem;
	ItemPtr begoneitem = nullptr;
	ItemPtr firstitem, returnitem;

	num_buttons = 0;
	if (buttons & DD_OK_BUTTON) num_buttons++;
	if (buttons & DD_YES_BUTTON) num_buttons++;
	if (buttons & DD_NO_BUTTON) num_buttons++;
	if (buttons & DD_CANCEL_BUTTON) num_buttons++;
	if (buttons & DD_BEGONE_BUTTON) num_buttons++;

	button_width = num_buttons * 10;

	num_strings = 1;
	sw1 = strlen(string1);
	sw2 = sw3 = sw4 = 0;

	if (string2 != NULL) {
		sw2 = strlen(string2);
		num_strings++;
	}

	if (string3 != NULL) {
		sw3 = strlen(string3);
		num_strings++;
	}

	if (string4 != NULL) {
		sw4 = strlen(string4);
		num_strings++;
	}

	string_width = (MAX(MAX(sw1, sw2), MAX(sw3, sw4))) + 2;

	dialog_width = MAX(string_width, button_width);

	dialog = dialog_create(NULL, x, y, dialog_width,
		alert_normal_color,
		alert_select_color,
		alert_hilite_color);

	if (dialog != NULL) {

		firstitem = NULL;

		if (num_buttons == 1) {
			normal_x = DD_IX_CENTER;
			cancel_x = DD_IX_CENTER;
			no_x = DD_IX_CENTER;
		} else {
			normal_x = DD_IX_LEFT;
			cancel_x = DD_IX_RIGHT;
			no_x = (buttons & DD_CANCEL_BUTTON) ? DD_IX_LEFT : DD_IX_RIGHT;
		}

		okitem = NULL;
		yesitem = NULL;
		noitem = NULL;
		cancelitem = NULL;

		if (buttons & DD_OK_BUTTON) {
			switch (dialog_language) {
			case DIALOG_GERMAN:
				Common::strcpy_s(buf, "  Ok  ");
				break;

			case DIALOG_ENGLISH:
			default:
				Common::strcpy_s(buf, "  OK  ");
				break;
			}
			okitem = dialog_add_button(dialog, normal_x, DD_IY_BUTTON, buf);
			firstitem = okitem;
		}

		if (buttons & DD_YES_BUTTON) {
			switch (dialog_language) {
			case DIALOG_GERMAN:
				Common::strcpy_s(buf, "  ~Ja  ");
				break;

			case DIALOG_ENGLISH:
			default:
				Common::strcpy_s(buf, " ~Yes ");
				break;
			}
			yesitem = dialog_add_button(dialog, normal_x, DD_IY_BUTTON, buf);
			if (firstitem == NULL) firstitem = yesitem;
		}

		if (buttons & DD_NO_BUTTON) {
			switch (dialog_language) {
			case DIALOG_GERMAN:
				Common::strcpy_s(buf, " ~Nein ");
				break;

			case DIALOG_ENGLISH:
			default:
				Common::strcpy_s(buf, "  ~No  ");
				break;
			}

			noitem = dialog_add_button(dialog, no_x, DD_IY_BUTTON, buf);
			if (firstitem == NULL) firstitem = noitem;
		}

		if (buttons & DD_BEGONE_BUTTON) {
			begoneitem = dialog_add_button(dialog, normal_x, DD_IY_BUTTON, "~Begone");
			if (firstitem == NULL) firstitem = begoneitem;
		}

		if (buttons & DD_CANCEL_BUTTON) {
			switch (dialog_language) {
			case DIALOG_GERMAN:
				Common::strcpy_s(buf, CANCEL_GERMAN);
				break;

			case DIALOG_ENGLISH:
			default:
				Common::strcpy_s(buf, CANCEL_BUTTON);
				break;
			}
			cancelitem = dialog_add_button(dialog, cancel_x, DD_IY_BUTTON, buf);
			if (firstitem == NULL) firstitem = cancelitem;
		}

		dialog_add_blank(dialog);
		if (string1 != NULL) dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, string1);
		if (string2 != NULL) dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, string2);
		if (string3 != NULL) dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, string3);
		if (string4 != NULL) dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, string4);
		dialog_add_blank(dialog);

		key_buf.len = 0;

		returnitem = dialog_execute(dialog, firstitem, firstitem, &key_buf);

		dialog_destroy(dialog);

		returnval = 0;

		if (returnitem == cancelitem) returnval = DD_CANCEL_BUTTON;
		if (returnitem == okitem)     returnval = DD_OK_BUTTON;
		if (returnitem == yesitem)    returnval = DD_YES_BUTTON;
		if (returnitem == noitem)     returnval = DD_NO_BUTTON;
		if (returnitem == begoneitem) returnval = DD_BEGONE_BUTTON;

	} else {
		returnval = dialog_error;
	}

	return returnval;
}

int dialog_alert_center(int buttons,
	char *string1, char *string2,
	char *string3, char *string4) {
	return dialog_alert(DD_CENTER, DD_CENTER, buttons, string1, string2, string3, string4);
}

int dialog_alert_ok(const char *string1, const char *string2,
		const char *string3, const char *string4) {
	return dialog_alert(DD_CENTER, DD_CENTER, DD_OK_BUTTON, string1, string2, string3, string4);
}

void dialog_newsay(int x, int y) {
	dialog_set_string_space(say_dialog, &say_dialog_work[0], SAY_DIALOG_SIZE);
	dialog_create(say_dialog, x, y, DD_AUTO, DD_DEFAULT, DD_DEFAULT, DD_DEFAULT);
}

void dialog_say(const char *message, int x) {
	dialog_add_message(say_dialog, x, DD_IY_AUTOFILL, message);
}

Window dialog_sayit(int saymode) {
	ItemPtr ok_item;
	struct KeyBuffer key_buf;
	char buf[80];

	switch (saymode) {
	case DD_PAUSE:
		switch (dialog_language) {
		case DIALOG_GERMAN:
			Common::strcpy_s(buf, "  Ok  ");
			break;
		case DIALOG_ENGLISH:
		default:
			Common::strcpy_s(buf, "  OK  ");
			break;
		}
		ok_item = dialog_add_button(say_dialog, DD_IX_CENTER, DD_IY_BUTTON,
			buf);
		say_dialog->cancel_item = ok_item;
		key_buf.len = 0;
		dialog_execute(say_dialog, ok_item, ok_item, &key_buf);
		dialog_destroy(say_dialog);
		break;

	case DD_PERMANENT:
		dialog_show_all(say_dialog);
		dialog_destroy_persist(say_dialog);
		break;

	case DD_MANUAL:
		dialog_show_all(say_dialog);
		break;
	}

	return say_dialog->window;
}

ItemPtr dialog_left_message(DialogPtr dialog, const char *prompt) {
	return dialog_add_message(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, prompt);
}

ItemPtr dialog_center_message(DialogPtr dialog, const char *prompt) {
	return dialog_add_message(dialog, DD_IX_CENTER, DD_IY_AUTOFILL, prompt);
}

ItemPtr dialog_left_string(DialogPtr dialog, const char *prompt, const char *string, int width) {
	return dialog_add_string(dialog, DD_IX_LEFT, DD_IY_AUTOFILL, prompt, string, width);
}

ItemPtr dialog_left_button(DialogPtr dialog, const char *prompt) {
	return dialog_add_button(dialog, DD_IX_LEFT, DD_IY_BUTTON, prompt);
}

ItemPtr dialog_ok_button(DialogPtr dialog) {
	char buf[80];

	switch (dialog_language) {
	case DIALOG_GERMAN:
		Common::strcpy_s(buf, "  Ok  ");
		break;
	case DIALOG_ENGLISH:
	default:
		Common::strcpy_s(buf, "  OK  ");
		break;
	}

	return dialog_left_button(dialog, buf);
}


ItemPtr dialog_cancel_button(DialogPtr dialog) {
	char buf[80];

	switch (dialog_language) {
	case DIALOG_GERMAN:
		Common::strcpy_s(buf, CANCEL_GERMAN);
		break;
	case DIALOG_ENGLISH:
	default:
		Common::strcpy_s(buf, CANCEL_BUTTON);
		break;
	}

	return dialog_add_button(dialog, DD_IX_RIGHT, DD_IY_BUTTON, buf);
}


char *dialog_get_string(DialogPtr dialog, ItemPtr item, char *target) {
	Common::strcpy_s(target, 65536, dialog_read_string(dialog, item));
	return target;
}


ItemPtr dialog_add_number(DialogPtr dialog, int x, int y, char *prompt, int num, int width) {
	char buf[80];
	Common::sprintf_s(buf, "%d", num);
	return dialog_add_string(dialog, x, y, prompt, buf, width);
}


ItemPtr dialog_left_number(DialogPtr dialog, char *prompt, int num, int width) {
	char buf[80];
	Common::sprintf_s(buf, "%d", num);
	return dialog_left_string(dialog, prompt, buf, width);
}

int dialog_get_number(DialogPtr dialog, ItemPtr item) {
	char buf[80];
	dialog_get_string(dialog, item, buf);

	return atoi(buf);
}

static void dialog_critical_error_handler() {
	// No implementation in ScummVM
}

void dialog_trap_critical() {
	dialog_server_installed = true;
}

void dialog_restore_critical(void) {
	dialog_server_installed = false;
}

} // namespace MADSV2
} // namespace MADS
