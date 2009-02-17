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
 * $URL$
 * $Id$
 *
 */

/* Header for SCI0 menu bar management */

#ifndef _SCI_MENUBAR_H_
#define _SCI_MENUBAR_H_

#include "sci/include/gfx_operations.h"
#include "sci/include/gfx_widgets.h"

struct _state;

#define MENU_FREESCI_BLATANT_PLUG 0xfff0
/* This adds an "About FreeSCI" menu option to the first menu */


#define MENU_HBAR_STRING_1 "--!"
#define MENU_HBAR_STRING_2 "-!"
#define MENU_HBAR_STRING_3 "!--"
/* These strings are used in SCI to determine an empty menu line */

#define MENU_BORDER_SIZE 0
/* The number of pixels added to the left and right to the text of a menu on the menu bar */

#define MENU_LEFT_BORDER 5
/* The number of pixels added to the left of the first menu */

#define MENU_BOX_CENTER_PADDING 10
/* Number of pixels to leave in between the left and the right centered text content in boxes
** that use right centered content
*/

#define MENU_BOX_LEFT_PADDING 0
/* Number of pixels to pad to the left */
#define MENU_BOX_RIGHT_PADDING 2
/* Number of pixels to pad to the right */

#define MENU_BAR_HEIGHT 10


#define MENU_TYPE_NORMAL 0
#define MENU_TYPE_HBAR 1 /* Horizontal bar */

/* Special characters used while building the menu bar */
#define SCI_SPECIAL_CHAR_FUNCTION 'F'
#define SCI_SPECIAL_CHAR_CTRL 3
#define SCI_SPECIAL_CHAR_ALT 2

/* Maximum number of bytes per SAID spec */
#define MENU_SAID_SPEC_SIZE 64

#define MENU_ATTRIBUTE_SAID 0x6d
#define MENU_ATTRIBUTE_TEXT 0x6e
#define MENU_ATTRIBUTE_KEY 0x6f
#define MENU_ATTRIBUTE_ENABLED 0x70
#define MENU_ATTRIBUTE_TAG 0x71

/* Those flags determine whether the corresponding menu_item_t entries are valid */
#define MENU_ATTRIBUTE_FLAGS_KEY 0x01
#define MENU_ATTRIBUTE_FLAGS_SAID 0x02

typedef struct {
	int type; /* Normal or hbar */
	char *keytext; /* right-centered part of the text (the key) */
	int keytext_size; /* Width of the right-centered text */

	int flags;
	byte said[MENU_SAID_SPEC_SIZE]; /* Said spec for this item */
	reg_t said_pos;
	char *text;
	reg_t text_pos;
	int modifiers, key; /* Hotkey for this item */
	int enabled;
	int tag;

} menu_item_t;


typedef struct {
	char *title;

	int title_width; /* Width of the title in pixels */
	int width; /* Pixel width of the menu window */

	int items_nr; /* Window height equals to intems_nr * 10 */
	menu_item_t *items; /* Actual entries into the menu */

} menu_t;



typedef struct {

	int menus_nr;
	menu_t *menus; /* The actual menus */

} menubar_t;

struct gfx_port;
struct gfx_picture; /* forward declarations for graphics.h */


/********** function definitions *********/

menubar_t *
menubar_new(void);
/* Creates a new menubar struct
** Parameters: (void)
** Returns   : (menubar_t *) A pointer to the new menubar entity
** To free the entity, call menubar_free.
*/


void
menubar_free(menubar_t *menubar);
/* Frees all memory associated with a menubar
** Parameters: (menubar_t *) menubar: The menubar to free
** Returns   : (void)
*/


void
menubar_add_menu(gfx_state_t *state, menubar_t *menubar, char *title, char *entries, int font, reg_t entries_base);
/* Adds a menu to the menubar.
** Parameters: (gfx_state_t *) state: The state the fonts are stored in
**             (menubar_t *) menubar: The menubar to operate on
**             (char *) title: The menu title
**             (char *) entries: A string of menu entries
**             (int) font: The font which is to be used for drawing
**             (reg_t) entries_base: Segmented VM address of the entries string
** Returns   : (void)
** The menu entries use the following special characters:
** '`' : Right justify the following part
** ':' : End of this entry
** '#' : Function key (replaced by 'F')
** '^' : Control key (replaced by \002, which looks like "CTRL")
** '=' : Initial tag value
** and the special string "--!", which represents a horizontal bar in the menu.
**
** If MENU_FREESCI_BLATANT_PLUG is defined, an additional option "About FreeSCI" will be
** added if this was the first menu to be added to the menu bar.
*/


int
menubar_set_attribute(struct _state *s, int menu, int item, int attribute, reg_t value);
/* Sets the (currently unidentified) foo and bar values.
** Parameters: (state_t *) s: The current state
**             (int) menu: The menu number to edit
**             (int) item: The menu item to change
**             (int) attribute: The attribute to modify
**             (int) value: The value the attribute should be set to
** Returns   : (int) 0 on success, 1 if either menu or item were invalid
*/


reg_t
menubar_get_attribute(struct _state *s, int menu, int item, int attribute);
/* Sets the (currently unidentified) foo and bar values.
** Parameters: (state_t *) s: The current state
**             (int) menu: The menu number
**             (int) item: The menu item to read
**             (int) attribute: The attribute to read from
** Returns   : (int) The attribute value, or -1 on error
*/


int
menubar_item_valid(struct _state *s, int menu, int item);
/* Determines whether the specified menu entry may be activated
** Parameters: (state_t *) s: The current state
**             (int x int) (menu, item): The menu item to check
** Returns   : (int) 1 if the menu item may be selected, 0 otherwise
*/


int
menubar_map_pointer(struct _state *s, int *menu_nr, int *item_nr, gfxw_port_t *port);
/* Maps the pointer position to a (menu,item) tuple.
** Parameters: (state_t *) s: The current state
**             ((int *) x (int *)) (menu_nr, item_nr): Pointers to the current menu/item tuple
**             (port_t *) port: The port of the currently active menu (if any)
** Returns   : (int) 1 if the pointer is outside a valid port, 0 otherwise.
*/

int
menubar_match_key(menu_item_t *item, int message, int modifiers);
/* Determines whether a message/modifiers key pair matches a menu item's key parameters
** Parameters: (menu_item_t *) item: The menu item to match
**             (int x int) message, modifiers: The input to compare
** Returns   : (int) 1 on match, 0 otherwise
*/

#endif /* !_SCI_MENUBAR_H_ */

