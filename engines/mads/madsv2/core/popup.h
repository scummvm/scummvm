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

#ifndef MADS_CORE_POPUP_H
#define MADS_CORE_POPUP_H

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/heap.h"

namespace MADS {
namespace MADSV2 {

#define POPUP_UPPER_LEFT      1       /* Pieces of border box */
#define POPUP_UPPER_RIGHT     2
#define POPUP_LOWER_LEFT      3
#define POPUP_LOWER_RIGHT     4
#define POPUP_LEFT            5
#define POPUP_RIGHT           6
#define POPUP_TOP             7
#define POPUP_BOTTOM          8
#define POPUP_UPPER_CENTER    9

#define pop_xs(piece)         (box_param.series->index[piece-1].xs)
#define pop_ys(piece)         (box_param.series->index[piece-1].ys)

#define pop_y2(piece)         (box_param.series->index[piece-1].y)
#define pop_y(piece)          (pop_y2(piece) - pop_ys(piece) + 1)

#define pop_m(piece)          (box_param.series->index[piece-1].x)
#define pop_x(piece)          (pop_m(piece) - (pop_xs(piece) >> 1))
#define pop_x2(piece)         (pop_x(piece) + pop_xs(piece) - 1)

#define POPUP_BASE_COLOR      18      /* Popup palette location */
#define POPUP_NUM_COLORS      8

#define POPUP_FILL_COLOR      popup_colors[8] /* Popup fill color */
#define POPUP_FILL_COLOR_2    popup_colors[9]
#define POPUP_TEXT_COLOR      popup_colors[10]
#define POPUP_HILITE_COLOR    popup_colors[11]
#define POPUP_HILITE_COLOR_2  popup_colors[12]
#define POPUP_ASK_COLOR       popup_colors[13]

#define popup_normal_color_1    popup_colors[0]
#define popup_normal_color_2    popup_colors[1]
#define popup_shadow_color_1    popup_colors[2]
#define popup_shadow_color_2    popup_colors[3]
#define popup_hilite_color_1    popup_colors[4]
#define popup_hilite_color_2    popup_colors[5]
#define popup_ask_color         popup_colors[6]
#define popup_text_color        popup_colors[7]

#define POPUP_DIALOG_TEXT_COLOR       popup_colors[14]
#define POPUP_DIALOG_BUTTON_COLOR_1   popup_colors[15]
#define POPUP_DIALOG_BUTTON_COLOR_2   popup_colors[16]
#define POPUP_DIALOG_BORDER_COLOR_1   popup_colors[17]
#define POPUP_DIALOG_BORDER_COLOR_2   popup_colors[18]
#define POPUP_DIALOG_SELECT_COLOR     popup_colors[19]
#define POPUP_DIALOG_STRING_COLOR     popup_colors[20]

#define POPUP_DIALOG_MENU_TEXT_1      popup_colors[21]
#define POPUP_DIALOG_MENU_TEXT_2      popup_colors[22]
#define POPUP_DIALOG_MENU_TEXT_3      popup_colors[23]

#define POPUP_MAX_LINES         20
#define POPUP_SPACING           1

#define POPUP_CENTER            0x8000        /* General centering  */
#define POPUP_BUTTON_ROW        0x4000        /* Popup buttons only */
#define POPUP_RIGHT_JUST        0x2000        /* Right side         */

#define POPUP_BUTTON_RIGHT      0x0800
#define POPUP_BUTTON_LEFT       0x0400

#define POPUP_FILL              -1            /* Fill on next line  */

#define POPUP_BAR               0xffff  /* Bar across line */
#define POPUP_UNDERLINE         0x8000  /* Underline text  */
#define POPUP_DOWNPIXEL         0x4000  /* Down a pixel    */


#define popup_padding_width     3     /* Extra space on each side */



typedef struct {
	SeriesPtr series;           /* Sprite series for popup box */

	char name[16];              /* Sprite series name          */

	FontPtr font;               /* Box font                    */
	int font_spacing;           /* Box font spacing            */

	int top_adjust_y;           /* Coordinate adjustments for  */
	int center_adjust_y;        /* box pieces (to get them in  */
	int upper_right_adjust_y;   /* line).                      */
	int left_adjust_x;
	int lower_left_adjust_x;
	int bottom_adjust_y;
	int lower_right_adjust_y;
	int right_adjust_x;

	int offset_x;               /* Window offset within box    */
	int offset_y;

	int extra_x;                /* Extra width/height provided by */
	int extra_y;                /* corner pieces                  */

	int pieces_per_center;      /* Number of bottom pieces in center */

	int erase_on_first;         /* If true, erases default during ask */

	SeriesPtr logo;             /* Logo sprite series, if any         */
	SeriesPtr menu;             /* Menu sprite series, if any         */
	FontPtr menu_font;          /* Menu font                          */
	int menu_font_spacing;      /* Menu font spacing                  */

	int menu_left_width;        /* Menu sprite series size            */
	int menu_right_width;
	int menu_middle_width;

	int menu_text_x_offset;     /* Menu text offsets                  */
	int menu_text_y_offset;
	int menu_text_x_bonus;      /* Bonus offsets when selected        */
	int menu_text_y_bonus;

} BoxParam;


typedef struct {
	int active;

	int base_x, base_y;         /* Box base screen location       */

	int x, y;                   /* Box upper left extremity       */
	int xs, ys;                 /* Box size                       */

	int window_x, window_y;     /* Window screen location         */
	int window_xs, window_ys;   /* Window size                    */

	int text_xs;                /* Horizontal size of actual text */
	int text_extra;             /* Extra padding space (total)    */

	int depth_x, depth_xs;      /* Depth preserve coordinates     */

	int horiz_pieces;           /* # of horiz pieces each side    */
	int vert_pieces;            /* # of vertical pieces           */

	int text_width;
	int text_x, text_y;
	int ask_x, ask_y;
	int cursor_x;
	word tab[POPUP_MAX_LINES];
	char *text[POPUP_MAX_LINES];

	int dont_add_space;

	int preserve_handle;
	int depth_preserve_handle;

	Buffer screen_buffer;     /* Direct copy of screen area under popup */
	Buffer depth_buffer;      /* Direct copy of depth area under popup  */

	int screen_saved;
	int depth_saved;

	word fill_accum;

	SeriesPtr icon;             /* Sprite displayed in box */
	int icon_id;
	int icon_width;
	int icon_height;

	int dialog_system;          /* Dialog system hooks */
	long request_y_size;

	int icon_center;

} Box;


/* Item types defined so */
#define ITEM_BLANK            0       /* Null item      */
#define ITEM_BUTTON           1       /* Button         */
#define ITEM_MESSAGE          2       /* String message */
#define ITEM_SAVELIST         3       /* Saved game list*/
#define ITEM_SPRITE           4       /* Static sprite  */
#define ITEM_MENU             5       /* Menu item      */

/*
#define ITEM_CHECKBOX         5
#define ITEM_RADIO            6
#define ITEM_STRING           7
*/


typedef struct {
	char *data;             /* Heap pointer for buffer space */
	int max_length;             /* Buffer max string width       */
	int width;                  /* Buffer width in pixels        */
	int base_x;                 /* Buffer base position          */
	int text_x;                 /* Buffer base text position     */
	int cursor;                 /* Buffer cursor location        */
	int select_base;
	int select_target;
} PopupBuffer;


#define SCROLL_AXIS_HORIZONTAL        0
#define SCROLL_AXIS_VERTICAL          1

#define SCROLL_STATUS_UP              0x80
#define SCROLL_STATUS_DOWN            0x40

typedef struct {
	int axis;

	int status;

	int x, y;
	int xs, ys;

	int arrow_size;
	int arrow_1_base;
	int arrow_2_base;

	int thumb_base;
	int thumb_item;
	int thumb_size;

} PopupScrollBar;


typedef struct {
	int box_x, box_y;           /* Size of box around list */
	int box_xs, box_ys;

	int list_x, list_y;        /* List item base location   */
	int list_xs, list_ys;       /* List item size            */
	int list_xw, list_yw;       /* List item width of entry  */

	int font_x, font_y;         /* Font base location        */

	int extra_x, extra_y;       /* Extra item (number list)  */
	int extra_xs, extra_ys;

	int rows, columns;          /* List rows and columnts    */
	int element_max_length;     /* List text length of entry */

	int elements;               /* List # of elements        */
	int element_offset;         /* List offset to elements   */

	int base_element;           /* Upper left element        */
	int picked_element;         /* Selected element          */

	PopupScrollBar scroll;      /* ScrollBar info            */

	/* ItemPtr   attached_string;  */

	char *data;             /* List string data          */

	long double_clock;          /* Double click clock        */
	int  double_element;        /* Double click element      */

} PopupList;

#define POPUP_DOUBLE_CLICK_THRESHOLD  20


#define POPUP_ITEM_VECTORS    7       /* Item-specific function vectors */

#define VECTOR_X_SIZE         0       /* Compute item X size            */
#define VECTOR_Y_SIZE         1       /* Compute item Y size            */
#define VECTOR_ADJUST         2       /* Adjust coordinates to absolute */
#define VECTOR_DRAW           3       /* Draw item                      */
#define VECTOR_UPDATE         4       /* Update item                    */
#define VECTOR_MOUSE          5       /* Accept mouse input             */
#define VECTOR_KEY            6       /* Accept keyboard input          */

#define ITEM_STATUS_ACTIVE    0x01
#define ITEM_STATUS_CLOCK     0x02
#define ITEM_STATUS_FORCED    0x04
#define ITEM_STATUS_IN_RANGE  0x08

#define ITEM_STATUS_STRING    0x10

#define ITEM_STATUS_VIRGIN    0x20
#define ITEM_STATUS_DIRTY     0x40
#define ITEM_STATUS_INPUT     0x80

#define ITEM_STATUS_INERT     0x100

struct PopupItem {
	byte type;                  /* Type of item    */

	word status;                /* Status of item  */

	int misc[10];               /* Item specific   */

	int x, y;                   /* Item location   */
	int xs, ys;                 /* Item base size  */

	int font_x;                 /* Font X base     */
	int font_y;                 /* Font Y base     */

	int keystroke;              /* Hotkey for item */

	long mouse_interval;        /* Mouse interval  */

	char *prompt;           /* Item label      */

	PopupBuffer *buffer;    /* Item buffer     */
	PopupList   *list;      /* Item list       */

	SeriesPtr series;           /* Sprite series   */
	int sprite;                 /* Sprite id       */

	int (*vector[POPUP_ITEM_VECTORS])(PopupItem *item); /* Vectors */
};


#define POPUP_MOUSE_SCROLL    0x8000  /* Mouse in scroll bar       */
#define POPUP_MOUSE_UP        0x4000  /* Mouse scrolling up        */
#define POPUP_MOUSE_DOWN      0x2000  /* Mouse scrolling down      */


#define POPUP_STATUS_DYNAMIC  0x8000  /* Popup created dynamically */
#define POPUP_STATUS_VALID    0x4000  /* Popup valid               */
#define POPUP_STATUS_EXIT     0x2000  /* Popup ready to exit       */

#define POPUP_STATUS_BAR      0x0004  /* Popup bar should be drawn    */
#define POPUP_STATUS_STEAL    0x0002  /* Popup bar should steal color */
#define POPUP_STATUS_BUTTON   0x0001  /* Popup button row in use      */


typedef struct {
	Heap heap;                  /* Popup heap space         */

	word status;                /* Popup status flags       */

	uint16 x, y;                  /* X location               */
	int16 width;                  /* X minimum width          */
	int16 y_position;             /* Y current usage          */
	int16 y_spacing;              /* Y spacing                */
	int16 xs, ys;                 /* Window size information  */

	int16 button_y;               /* Button Y location        */
	int16 button_spacing;         /* Button spacing           */
	int16 button_left_fill;       /* Button row left fill     */
	int16 button_right_fill;      /* Button row right fill    */
	int16 button_bar_color;       /* Button bar color         */

	uint32 key;                   /* Last keyboard input      */
	int16 key_handled;            /* Flag if key handled      */

	int16 mouse_status;           /* Mouse status word        */
	long mouse_clock;           /* Mouse action clock       */

	PopupItem *enter_item;  /* ENTER key button         */
	PopupItem *cancel_item; /* CANCEL (ESC key) item    */

	PopupItem *string_item; /* STRING item, if any      */

	PopupItem *mouse_item;  /* Mouse active item        */
	PopupItem *active_item; /* Active item              */

	PopupItem *list_item;   /* List item                */
	PopupItem *clear_item;  /* Clear list item          */

	int16 max_items;              /* Maximum # of items       */
	int16 num_items;              /* Number of items in popup */

	PopupItem *item;        /* Item list                */
} Popup;


extern Box text_box;
extern Box *box;

extern int popup_key;
extern int popup_esc_key;
extern int popup_asking_number;

extern int popup_available;


extern int popup_preserve_initiator[3];

extern byte popup_colors[24];
extern byte popup_num_colors;

extern BoxParam box_param;


int  popup_create(int horiz_pieces, int x, int y);
void popup_add_icon(SeriesPtr series, int id, int center);
int  popup_draw(int save_screen, int depth_code);
void popup_destroy(void);

void popup_next_line(void);
void popup_set_ask(void);
void popup_add_string(const char *string);
void popup_write_string(const char *string);
void popup_bar(void);
void popup_underline(void);
void popup_downpixel(void);
void popup_tab(int tab_level);
void popup_center_string(const char *string, int underline);

int  popup_and_wait(int save_screen);
int  popup_and_dont_wait(int save_screen);

void popup_update_ask(char *string, int maxlen);
int  popup_ask_string(char *target, int maxlen, int save_screen);
int  popup_ask_number(long *value, int maxlen, int save_screen);


int popup_estimate_pieces(int maxlen);
int popup_get_string(char *target, const char *top, const char *left, int maxlen);
int popup_get_long(long *value, const char *top, const char *left, int maxlen);
int popup_get_number(int16 *value, const char *top, const char *left, int maxlen);

int popup_alert(int width, const char *message_line, ...);

int popup_box_load(void);

/*
 * popup_dialog_create()
 * Sets up popup dialog structure, allocating memory dynamically
 * if necessary.
 *
 * @param memory	
 * @param heap_size	
 * @param max_items	
 * @return 
 */
Popup *popup_dialog_create(void *memory, long heap_size, int max_items);
/*
 * popup_dialog_destroy()
 * Destroys a popup dialog, deallocating any dynamic memory.
 *
 * @return 
 */
Popup *popup_dialog_destroy(void);

PopupItem *popup_button(const char *prompt, int x);
PopupItem *popup_cancel_button(const char *prompt);
PopupItem *popup_message(const char *prompt, int x, int y);
PopupItem *popup_execute(void);

PopupItem *popup_savelist(const char *data,
	const char *empty_string,
	int elements,
	int element_offset,
	int element_max_length,
	int pixel_width,
	int rows,
	int accept_input,
	int default_element);

void popup_blank(int num_lines);
void popup_blank_line(void);
void popup_width_force(int width);


void popup_menu_option(PopupItem *item, char *option);
PopupItem *popup_menu(const char *prompt,
	int x, int y, int pixel_width,
	int off_center_x,
	int elements, int element_max_length,
	int default_element);

PopupItem *popup_sprite(SeriesPtr series, int sprite, int x, int y);

} // namespace MADSV2
} // namespace MADS

#endif
