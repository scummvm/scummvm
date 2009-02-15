/***************************************************************************
 sci_widgets.h Copyright (C) 2000 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* SCI-specific widget handling */

#ifndef _SCI_WIDGETS_H_
#define _SCI_WIDGETS_H_

#include "sci/include/engine.h"

/* The following flags are applicable to windows in SCI0: */
#define WINDOW_FLAG_TRANSPARENT 0x01


#define WINDOW_FLAG_NOFRAME 0x02
/* No frame is drawn around the window onto wm_view */

#define WINDOW_FLAG_TITLE 0x04
/* Add titlebar to window (10 pixels high, framed, text is centered and written
** in white on dark gray
*/

#define WINDOW_FLAG_DONTDRAW 0x80
/* Don't draw anything */

#define WINDOW_FLAG_NO_DROP_SHADOW 0x1000000


/* Used in kgraphics to flag text surrounded by a dithered frame */
#define CONTROL_STATE_DITHER_FRAMED 0x1000

/* Used by the interpreter to flag buttons that are grayed out */
#define CONTROL_STATE_GRAY 0x0004
/* Used by the interpreter to flag some widgets to determine whether they should be surrounded by a frame */
#define CONTROL_STATE_FRAMED 0x0008
/* Used by the interpreter to flag buttons that are enabled */
#define CONTROL_STATE_ENABLED 0x0001

void
sciw_set_status_bar(state_t *s, gfxw_port_t *status_bar, char *text, int fgcolor, int bgcolor);
/* Sets the contents of a port used as status bar
** Parmeters: (state_t *) s: The affected game state
**            (gfxw_port_t *) status_bar: The status bar port
**            (char *) text: The text to draw
** Returns  : (void)
*/

gfxw_port_t *
sciw_new_window(state_t *s, rect_t area, int font, gfx_color_t color, gfx_color_t bgcolor,
		int title_font, gfx_color_t title_color, gfx_color_t title_bg_color,
		const char *title, int flags);
/* Creates a new SCI style window
** Parameters: (state_t *) s: The affected game state
**             (rect_t) area: The screen area to frame (not including a potential window title)
**             (int) font: Default font number to use
**             (gfx_color_t) color: The foreground color to use for drawing
**             (gfx_color_t) bgcolor: The background color to use
**             (int) title_font: The font to use for the title bar (if any)
**             (gfx_color_t) title_color: Color to use for the title bar text
**             (gfx_color_t) title_bg_color: Color to use for the title bar background
**             (const char *) title: The text to write into the title bar
**             (int) flags: Any ORred combination of window flags
** Returns   : (gfxw_port_t *) A newly allocated port with the requested characteristics
*/

/*---------------------*/
/*** Control widgets ***/
/*---------------------*/

gfxw_list_t *
sciw_new_button_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font, char selected, char inverse, char gray);
/* Creates a new button control list
** Parameters: (gfxw_port_t *) port: The port containing the color values to use for the
**                                   button (the button is /not/ appended to the port there)
**             (reg_t) ID: Button's ID
**             (rect_t) zone: The area occupied by the button
**             (char *) text: The text to write into the button
**             (int) font: The font to use for the button
**             (char) selected: Whether the button should be marked as being selected by the keyboard focus
**             (char) inverse: Whether to inverse the color scheme
**             (char) gray: Whether the button should be grayed out
** Returns   : (gfxw_list_t *) The button
*/

gfxw_list_t *
sciw_new_text_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font,
		      gfx_alignment_t align, char frame, char inverse);
/* Creates a new text control list
** Parameters: (gfxw_port_t *) port: The port containing the color values to use
**             (reg_t) ID: Text widget ID
**             (rect_t) zone: Area occupied by the text
**             (char *) text: The text
**             (int) font: The font the text is to be drawn in
**             (gfx_alignment_t) align: Horizontal text alignment to use
**             (char) frame: Whether a dithered frame should surround the text
**             (char) inverse: Whether the text colors should be inversed
** Returns   : (gfxw_list_t *) The text control widget list
*/

gfxw_list_t *
sciw_new_edit_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font, unsigned int cursor,
		      char inverse);
/* Creates a new edit control list
** Parameters: (gfxw_port_t *) port: The port containing the color values to use
**             (reg_t) ID: Text widget ID
**             (rect_t) zone: Area occupied by the text
**             (char *) text: The text
**             (int) font: The font the text is to be drawn in
**             (int) cursor: Cursor position
**             (char) inverse: Whether the edit widget should be reversed
** Returns   : (gfxw_list_t *) An appropriate widget list
*/

gfxw_list_t *
sciw_new_icon_control(gfxw_port_t *port, reg_t ID, rect_t zone, int view, int loop, int cel,
		      char frame, char inverse);
/* Creates a new icon control list
** Parameters: (gfxw_port_t *) port: The port containing the color values to use
**             (reg_t) ID: Text widget ID
**             (rect_t) zone: Area occupied by the text
**             (int x int x int) view, loop, cel: The cel to display
**             (char) frame: Whether the widget should be surrounded by a frame
**             (char) lina inverse: Whether colors should be inversed
** Returns   : (gfxw_list_t *) An appropriate widget list
*/

gfxw_list_t *
sciw_new_list_control(gfxw_port_t *port, reg_t ID, rect_t zone, int font_nr, char **entries_list,
		      int entries_nr, int list_top, int selection, char inverse);
/* Creates a new list control list
** Parameters: (gfxw_port_t *) port: The port containing the color values to use
**             (int) ID: Text widget ID
**             (rect_t) zone: Area occupied by the text
**             (int) font_nr: number of the font to use
**             (char **) entries_list: List of strings to contain within the list
**             (int) entries_nr: Number of entries in entries_list
**             (int) list_top: First list item that is visible
**             (int) selection: The list item that is selected
**             (char) invserse: The usual meaning
** Returns   : (gfxw_list_t *) An appropriate widget list
*/

/*---------------------*/
/*** Menubar widgets ***/
/*---------------------*/

void
sciw_set_menubar(state_t *s, gfxw_port_t *status_bar, menubar_t *menubar, int selection);
/* Draws the menu bar
** Parameters: (state_t *) s: The state to operate on
**             (gfxw_port_t *) status_bar: The status bar port to modify
**             (menubar_t *) menubar: The menu bar to use
**             (int) selection: Number of the menu to hightlight, or -1 for 'none'
** Returns   : (void)
*/

gfxw_port_t *
sciw_new_menu(state_t *s, gfxw_port_t *status_bar, menubar_t *menubar, int selection);
/* Creates a menu port
** Parameters: (state_t *) s: The state to operate on
**             (gfxw_port_t *) status_bar: The status bar
**             (menubar_t *) menubar: The menu bar to use
**             (int) selection: Number of the menu to interpret
** Returns   : (gfxw_port_t *) The result port
*/

gfxw_port_t *
sciw_unselect_item(state_t *s, gfxw_port_t *menu_port, menu_t *menu, int selection);
/* Unselects a previously selected item from a menu port
** Parameters: (state_t *) s: The state to operate on
**             (gfxw_port_t *) menu_port: The port modify
**             (menu_t *) menu: The menu the menu port corresponds to
**             (int) selection: Number of the menu entry to unselect, or -1 to do a NOP
** Returns   : (gfxw_port_t *) The modified menu
*/

gfxw_port_t *
sciw_select_item(state_t *s, gfxw_port_t *menu_port, menu_t *menu, int selection);
/* Selects a menu item from a menu port
** Parameters: (state_t *) s: The state to operate on
**             (gfxw_port_t *) menu_port: The port modify
**             (menu_t *) menu: The menu the menu port corresponds to
**             (int) selection: Number of the menu entry to select, or -1 to do a NOP
** Returns   : (gfxw_port_t *) The modified menu
*/

#endif /* _!SCI_WIDGETS_H_ */

