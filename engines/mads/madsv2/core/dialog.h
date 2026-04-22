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

#ifndef MADS_CORE_DIALOG_H
#define MADS_CORE_DIALOG_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/window.h"
#include "mads/madsv2/core/keys.h"

namespace MADS {
namespace MADSV2 {

#define DIALOG_ENGLISH          0       /* English language                */
#define DIALOG_GERMAN           1       /* German  language                */

#define DIALOG_MAX_ITEM         50      /* Maximum items in a box          */
#define DIALOG_MAX_BUFFER       16      /* Maximum buffer areas in box     */
#define DIALOG_MAX_LIST         4       /* Maximum number of list windows  */
#define DIALOG_MAX_PROMPT_CHARS 72      /* Maximum length of prompt string */
#define DIALOG_MAX_LIST_CHARS   13      /* Maximum length of list string   */

#define DIALOG_DEFAULT_STRING_SPACE 1024/* Default workspace size          */

#define DIALOG_FILE_WIDTH       61      /* Proper width for file dialog box*/
#define DIALOG_FILE_BUFFER      46      /* Proper width for file name buf. */

#define DD_DYNAMIC              0x80    /* Dialog allocated dynamically*/
#define DD_STATIC               0x00    /* Dialog pre-allocated static */
#define DD_EXITFLAG             0x40    /* Dialog is terminating       */
#define DD_COMPUTED             0x20    /* Dialog boundaries computed  */
#define DD_SCREENSAVED          0x10    /* Dialog window has saved     */
#define DD_FILEMENU             0x08    /* Dialog uses file functions  */
#define DD_DYNAMICSTRINGS       0x04    /* Dialog has dynamic strings  */

#define DD_CENTER               -1      /* Dialog centered on axis     */

#define DD_AUTO                 -1      /* Auto-compute (width, etc.)  */

#define DD_DEFAULT              -1      /* Default color               */

#define DD_LINEACROSS           0       /* Line across the box         */

#define DD_PAUSE                1       /* Use OK button to exit       */
#define DD_PERMANENT            2       /* No window cleanup           */
#define DD_MANUAL               3       /* Caller calls window_destroy */

#define DD_I_NONE               0       /* Blank item type             */
#define DD_I_STRING             1       /* Item is a string input line */
#define DD_I_FILENAME           2       /* Item is a filename input    */
#define DD_I_LISTBASED          3       /* Item is a list-based string */
#define DD_I_CHECKBOX           4       /* Item is a checkoff box      */
#define DD_I_BUTTON             5       /* Item is a bottom-row button */
#define DD_I_MESSAGE            6       /* Item is a message string    */

#define DD_I_FILELIST           7       /* Item is a file list         */
#define DD_I_DIRSLIST           8       /* Item is a directory list    */
#define DD_I_LIST               9       /* Item is a string list       */

#define DD_IX_CENTER           -1      /* X control: Center this item */
#define DD_IX_LEFT             -2      /* X control: Left margin item */
#define DD_IX_RIGHT            -3      /* X control: Right margin itm */

#define DD_IY_BUTTON           -1      /* Y control: button line      */
#define DD_IY_AUTOFILL         -2      /* Y control: autofill from top*/

#define DD_C_HOME              -257    /* Move cursor home            */
#define DD_C_END               -258    /* Move cursor to end          */

#define DD_MC_NONE              -1      /* Mouse not in any item       */
#define DD_MC_SCROLL            -2      /* Mouse in a scroll bar       */
#define DD_MC_SCROLL_UP         -3      /* Mouse in scroll up arrow    */
#define DD_MC_SCROLL_DOWN       -4      /* Mouse in scroll down arrow  */
#define DD_MC_SCROLL_LEFT       -5      /* Mouse in scroll left arrow  */
#define DD_MC_SCROLL_RIGHT      -6      /* Mouse in scroll right arrow */
#define DD_MC_SCROLL_PAGE_UP    -7      /* Mouse pageing up            */
#define DD_MC_SCROLL_PAGE_DOWN  -8      /* Mouse pageing down          */
#define DD_MC_SCROLL_PAGE_LEFT  -9      /* Mouse pageing left          */
#define DD_MC_SCROLL_PAGE_RIGHT -10     /* Mouse pageing right         */

#define CANCEL_BUTTON           "Cancel"
#define CANCEL_GERMAN           "Abbrechen"

#define DD_CANCEL_BUTTON        0x01    /* Cancel button bit mask      */
#define DD_OK_BUTTON            0x02    /* OK button bit mask          */
#define DD_YES_BUTTON           0x04    /* Yes button bit mask         */
#define DD_NO_BUTTON            0x08    /* No button bit mask          */
#define DD_BEGONE_BUTTON        0x10    /* Begone button bit mask      */

#define DD_CHECK_GREY           0x80    /* Greyed out checkbox         */
#define DD_CHECK_RADIO          0x40    /* Checkbox is part of an ex-  */
/* clusive (radio button) group*/
#define DD_CHECK_TRUE           0x01    /* Checkbox is selected.       */

#define DD_ERR_NOMOREITEMS      -1      /* Ran out of items            */
#define DD_ERR_NOMOREBUFFERS    -2      /* Ran out of string buffers   */
#define DD_ERR_NOMORELISTS      -3      /* Ran out of string lists     */
#define DD_ERR_NOMOREMEMORY     -4      /* Ran out of memory           */
#define DD_ERR_ABORTEDBYCALLBACK -5     /* Aborted by callback routine */
#define DD_ERR_NOMORESTRINGS    -6      /* Ran out of string space     */

#define DD_DEFAULT_NORMAL_COLOR  colorbyte (black, white)
#define DD_DEFAULT_SELECT_COLOR  colorbyte (white, black)
#define DD_DEFAULT_HILITE_COLOR  colorbyte (hi_white, white)
#define DD_DEFAULT_GREYED_COLOR  colorbyte (hi_black, white)


/* Quick dialog definition macros */

#define dialog_declare(dd)		DialogPtr dd = NULL; ItemPtr ok_item = nullptr, cancel_item = nullptr, result_item = nullptr
#define dialog_declare_ok(dd)	DialogPtr dd = NULL; ItemPtr ok_item = nullptr, result_item = nullptr
#define dialog_buttons(dd)		ok_item = dialog_ok_button(dd);cancel_item = dialog_cancel_button(dd)
#define dialog_exec(dd, ii)		result_item = dialog_execute(dd, ii, ok_item, NULL);


struct DialogItem {
	short id;                             /* Item ID code                */
	short type;                           /* Item type                   */
	short x, y;                           /* Item base location          */
	short x2;                             /* Item secondary x value      */
	short width;                          /* Item total width            */
	short buf_id;                         /* Item buffer identifier      */
	short buf_width;                      /* Item input area width       */
	short status;                         /* Item status                 */
	short keystroke;                      /* Item keystroke activator    */
	short _class;                         /* Item checkbox class         */
	char *prompt;                     /* Item prompt/string          */
};

typedef struct DialogItem Item;
typedef Item *ItemPtr;

struct DialogList {
	char *list;
	Window window;
	short base_x, base_y;
	short rows, columns;
	short thumb;
	short base_entry;
	short picked_entry;
	short entry_width;
	short elements, element_offset;
	short max_elements;
};

typedef struct DialogList List;
typedef List *ListPtr;

struct DialogBox {
	Window window;                        /* window boundaries           */
	short base_x, base_y;                 /* upper left text coords      */
	short width;                          /* width                       */

	short normal_color;                   /* dialog display colors       */
	short select_color;
	short hilite_color;

	short num_items;                      /* number of dialog items      */
	short status;                         /* general status flags        */
	short fill_marker;                    /* vertical fill marker        */
	short active_button;                  /* currently active button     */
	short active_item;                    /* currently active item       */

	short cursor_x, cursor_y;             /* current cursor coordinates  */

	void  (*callback)();                  /* callback routine for list   */
	/* selection.                  */

	void  (*checkbox_callback)();         /* callback routine for check  */
	/* box modification.           */

	Item item[DIALOG_MAX_ITEM];           /* items in this dialog  */
	ItemPtr default_item;                 /* Item for default button     */
	ItemPtr cancel_item;                  /* Item for "Cancel" button    */
	ItemPtr path_item;                    /* Item holding filename path  */

	short button_left, button_right;      /* button horiz fill markers   */
	short button_flag;                    /* button row occupied flag    */
	short buffers_allocated;              /* # buffers now in use        */

	char *buffer[DIALOG_MAX_BUFFER];   /* string i/o buffers          */
	int  buf_cursor[DIALOG_MAX_BUFFER]; /* string i/o cursors          */
	int  buf_selbase[DIALOG_MAX_BUFFER]; /* string i/o select base      */
	int  buf_seltarget[DIALOG_MAX_BUFFER]; /* string i/o select target    */
	int  buf_base_x[DIALOG_MAX_BUFFER]; /* string i/o base locations   */
	int  buf_status[DIALOG_MAX_BUFFER]; /* string i/o status           */
	int  buf_select[DIALOG_MAX_BUFFER]; /* string i/o selection status */

	short lists_allocated;                 /* # windows now in use        */

	List lists[DIALOG_MAX_LIST];           /* list windows                */

	char *string_space;                    /* String space for prompts    */
	char *string_marker;                   /* Marker for next string      */
	long string_space_remaining;           /* Amount of string space left */
};

typedef struct DialogBox Dialog;
typedef Dialog *DialogPtr;

#define StringMode(x)   (((x).type == DD_I_STRING) || ((x).type == DD_I_FILENAME) || ((x).type == DD_I_LISTBASED))
#define DialogString(x) StringMode(x->item[x->active_item])

extern int dialog_error;
extern bool dialog_quicksearch_flag;
extern bool dialog_wildcard_exits;
extern long dialog_timeout;
extern int dialog_language;

/**
 * Adds a button to the dialog box (see external docs).
 */
extern ItemPtr dialog_add_button(DialogPtr dialog, int x, int y, const char *prompt);

/**
 * Adds a check box to the dialog.
 */
extern ItemPtr dialog_add_checkbox(DialogPtr dialog, int x, int y,
	const char *prompt, int default_val, int class_);

/**
 * Causes the specified checkbox to be displayed "greyed out".
 */
extern void dialog_grey_checkbox(ItemPtr item);

/**
 * Adds a message string to the dialog
 */
extern ItemPtr dialog_add_message(DialogPtr dialog, int x, int y, const char *prompt);

/**
 * Adds a blank line to an AUTOFILLing dialog
 */
extern void dialog_add_blank(DialogPtr dialog);

/**
 * Adds a string entry item to the dialog box
 */
extern ItemPtr dialog_add_string(DialogPtr dialog, int x, int y,
	const char *prompt, const char *default_val, int width);

/**
 * Add a list-based string item to the dialog box
 */
extern ItemPtr dialog_add_listbased(DialogPtr dialog, int x, int y, const char *prompt,
	const char *default_val, int width, const char *title, char *mylist,
	int elements, int element_offset, int entry_width, int rows, int columns);

/**
 * Adds a filename-base string item to the dialog box
 */
extern ItemPtr dialog_add_filename(DialogPtr dialog, int x, int y, const char *prompt,
	const char *default_val, const char *path, int rows, char *filebuf, int max_file_elements,
	char *dirsbuf, int max_dirs_elements);

/**
 * Appends a second list to a listbased string
 */
extern ItemPtr dialog_append_list(DialogPtr dialog, int x, int y,
	ItemPtr base_string, const char *title, const char *mylist, int elements,
	int element_offset, int entry_width, int rows, int columns);

extern void dialog_set_colors(int normal, int select, int hilite, int greyed);
extern void dialog_set_workspace_size(long workspace);
extern void dialog_set_string_space(DialogPtr dialog, const char *space, long size);

/**
 * Begins a new dialog box, and allocates memory for it if necessary
 */
extern DialogPtr dialog_create(DialogPtr dialog, int ul_x, int ul_y,
	int width, int normal_color, int select_color, int hilite_color);

/**
 * Deallocates memory for a dynamically allocated dialog box
*/
extern void dialog_destroy(DialogPtr dialog);

/**
 * Deallocates memory for dialog, but does not restore screen
 */
extern void dialog_destroy_persist(DialogPtr dialog);

/**
 * Creates a dialog box and adds a filename entry to it
 */
extern DialogPtr dialog_file_create(DialogPtr dialog, int ul_x, int ul_y,
	int normal_color, int select_color, int hilite_color, ItemPtr *ok_item,
	ItemPtr *first_item, const char *default_val, const char *path, int rows,
	char *filebuf, int maxfiles, char *dirsbuf, int maxdirs, const char *prompt);

/**
 * Assembly routine to read a file directory into the specified
 * list structure.  "Wild" is the wildcard spec to match (e.g. "*.*")
 * in searching, "dirflag" determines whether the list should receive
 * files or subdirectories (true = just subdirectories, false = just files).
 */
extern int dialog_read_dir_to_list(ListPtr target, const char *wild, int dirflag);

/**
 * Sets up a callback function which is activated each time an
 * item is highlighted in a list box.
 *
 * The proper prototype for a callback function is:
 * void callback (DialogPtr dialog, ListPtr list);
 */
extern void dialog_set_list_callback(DialogPtr dialog, void (*(callback))());

/**
 * Sets up a callback function which is activated each time a mouse
 * event is detected on a different video screen from the dialog box
 * (use when mouse is in double screen mode).  The prototype for the
 * callback function is:
 *
 * int   callback (int mouse_button, int mouse_status,
 *                int mouse_x, int mouse_y, int mouse_video_mode);
 *         If function returns "false", dialog box will continue
 *         to run as usual; if function returns true, dialog immediately
 *         aborts with NULL item, and dialog_error contains
 *         DD_ERR_ABORTEDBYCALLBACK.
 *
 * double_only     If true, then callback is only activated when
 *                 cursor event occurs on a different *screen* from
 *                 the dialog box.  If false, then callback occurs
 *                 even when cursor is simply outside the dialog box
 *                 (on the same screen).
 *
 *
 * Mouse cursor will be visible at the time the callback routine is
 * activated.
 */
typedef int (*MouseCallback)(int mouse_button, int mouse_status, int mouse_x, int mouse_y, int mouse_video_mode);
extern void dialog_set_mouse_callback(MouseCallback callback, int double_only);

typedef void (*PageCallback)(int mykey, ItemPtr item, DialogPtr dialog);
extern void dialog_set_page_callback(PageCallback fn);

/**
 * Sets up a callback function which is activated each time a
 * check box item is modified.
 *
 * The proper prototype for a callback function is:
 * int callback (DialogPtr dialog, ItemPtr item);
 *
 * If the callback routine returns "true", the dialog is
 * immediately aborted (returns NULL handle, and dialog_error
 * is set to DD_ERR_ABORTEDBYCALLBACK).
 */
extern void dialog_set_checkbox_callback(DialogPtr dialog, void (*(callback))());

/**
 * Returns the value of the specified checkbox
 */
extern int dialog_read_checkbox(DialogPtr dialog, ItemPtr item);

/**
 * Returns a pointer to the specified string buffer
 */
extern char *dialog_read_string(DialogPtr dialog, ItemPtr item);

/**
 * Returns a pointer to the specified list-based string
 */
extern char *dialog_read_list(DialogPtr dialog, ItemPtr item);

/**
 * Returns the dialog box's path
 */
extern char *dialog_read_pathname(DialogPtr dialog, ItemPtr item);

/**
 * Returns the filename selected by the user
 */
extern char *dialog_read_filename(DialogPtr dialog, ItemPtr item);

/**
 * Returns the filename and full path selected by the user
 */
extern char *dialog_read_filepath(DialogPtr dialog, ItemPtr item);

/**
 * Routine to display the entire dialog box
 */
extern void dialog_show_all(DialogPtr dialog);

/**
 * The main loop for executing a dialog.  Includes keyboard
 * handler and mouse scanner.
 */
extern ItemPtr dialog_execute(DialogPtr dialog, ItemPtr active_item,
	ItemPtr default_button, KeyPtr key_buffer);

extern char *dialog_select_file(const char *prompt, const char *path,
	const char *filespec, char *output);
extern char *dialog_enter_string(char *reply, const char *top_prompt,
	const char *left_prompt, char *my_default, int maxlen);

/**
 * Prompts user to enter an integer value (displays default).
 * Returns 0 if user aborted, or entry was out of range.
 */
extern int dialog_enter_int(const char *top_prompt, int my_default);

extern DialogPtr dialog_create_default();

/**
 * Pops up a quick dialog to inform user of an error or to ask
 * a yes/no question.
 */
extern int dialog_alert(int x, int y, int buttons, const char *string1,
	const char *string2, const char *string3, const char *string4);

extern void dialog_set_alert_colors(int normal, int select, int hilite);
extern int dialog_alert_center(int buttons, const char *string1,
	const char *string2, const char *string3, const char *string4);
extern int dialog_alert_ok(const char *string1, const char *string2,
	const char *string3, const char *string4);
extern void dialog_newsay(int x, int y);
extern void dialog_say(const char *message, int x);
extern Window dialog_sayit(int saymode);

extern ItemPtr dialog_left_message(DialogPtr dialog, const char *prompt);
extern ItemPtr dialog_center_message(DialogPtr dialog, const char *prompt);
extern ItemPtr dialog_left_string(DialogPtr dialog, const char *prompt, const char *string, int width);
extern ItemPtr dialog_left_button(DialogPtr dialog, const char *prompt);
extern ItemPtr dialog_ok_button(DialogPtr dialog);
extern ItemPtr dialog_cancel_button(DialogPtr dialog);
extern char *dialog_get_string(DialogPtr dialog, ItemPtr item, char *target);

extern ItemPtr dialog_add_number(DialogPtr dialog, int x, int y,
	const char *prompt, int num, int width);
extern ItemPtr dialog_left_number(DialogPtr dialog, char *prompt,
	int num, int width);
extern int dialog_get_number(DialogPtr dialog, ItemPtr item);

extern void dialog_trap_critical();
extern void dialog_restore_critical();

extern void dialog_watch_point(const char *string, long x, long y);

} // namespace MADSV2
} // namespace MADS

#endif
