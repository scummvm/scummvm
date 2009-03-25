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

/* SCI-specific widget handling */

#ifndef SCI_INCLUDE_SCI_WIDGETS_H
#define SCI_INCLUDE_SCI_WIDGETS_H

#include "sci/engine/state.h"

namespace Sci {

class Menu;

// The following flags are applicable to windows in SCI0
enum windowFlags {
	kWindowTransparent  = 0x01,          // 0001
	kWindowNoFrame      = 0x02,          // 0010 - a window without a frame
	// Add title bar to window (10 pixels high, framed, text is centered and written in white on dark gray)
	kWindowTitle        = 0x04,          // 0100
	kWindowDontDraw     = 0x80,          // 1000 0000 - don't draw anything
	kWindowNoDropShadow = 0x1000000,     // 0001 0000 0000 0000 0000 0000 0000
};

enum controlStateFlags {
	kControlStateEnabled      = 0x0001,  // 0001 - enabled buttons (used by the interpreter)
	kControlStateDisabled     = 0x0004,  // 0010 - grayed out buttons (used by the interpreter)
	kControlStateFramed       = 0x0008,  // 1000 - widgets surrounded by a frame (used by the interpreter)
	kControlStateDitherFramed = 0x1000   // 0001 0000 0000 0000 - widgets surrounded by a dithered frame (used in kgraphics)
};

void sciw_set_status_bar(EngineState *s, gfxw_port_t *status_bar, const Common::String &text, int fgcolor, int bgcolor);
/* Sets the contents of a port used as status bar
** Parmeters: (EngineState *) s: The affected game state
**            (gfxw_port_t *) status_bar: The status bar port
**            (const char *) text: The text to draw
** Returns  : (void)
*/

gfxw_port_t *sciw_new_window(EngineState *s, rect_t area, int font, gfx_color_t color, gfx_color_t bgcolor,
	int title_font, gfx_color_t title_color, gfx_color_t title_bg_color,
	const char *title, int flags);
/* Creates a new SCI style window
** Parameters: (EngineState *) s: The affected game state
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

gfxw_list_t *sciw_new_button_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font, char selected, char inverse, char gray);
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

gfxw_list_t *sciw_new_text_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font,
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

gfxw_list_t *sciw_new_edit_control(gfxw_port_t *port, reg_t ID, rect_t zone, char *text, int font, unsigned int cursor,
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

gfxw_list_t *sciw_new_icon_control(gfxw_port_t *port, reg_t ID, rect_t zone, int view, int loop, int cel,
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

gfxw_list_t *sciw_new_list_control(gfxw_port_t *port, reg_t ID, rect_t zone, int font_nr, char **entries_list,
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

void sciw_set_menubar(EngineState *s, gfxw_port_t *status_bar, Menubar *menubar, int selection);
/* Draws the menu bar
** Parameters: (EngineState *) s: The state to operate on
**             (gfxw_port_t *) status_bar: The status bar port to modify
**             (Menubar *) menubar: The menu bar to use
**             (int) selection: Number of the menu to hightlight, or -1 for 'none'
** Returns   : (void)
*/

gfxw_port_t *sciw_new_menu(EngineState *s, gfxw_port_t *status_bar, Menubar *menubar, int selection);
/* Creates a menu port
** Parameters: (EngineState *) s: The state to operate on
**             (gfxw_port_t *) status_bar: The status bar
**             (Menubar *) menubar: The menu bar to use
**             (int) selection: Number of the menu to interpret
** Returns   : (gfxw_port_t *) The result port
*/

gfxw_port_t *sciw_unselect_item(EngineState *s, gfxw_port_t *menu_port, Menu *menu, int selection);
/* Unselects a previously selected item from a menu port
** Parameters: (EngineState *) s: The state to operate on
**             (gfxw_port_t *) menu_port: The port modify
**             (Menu *) menu: The menu the menu port corresponds to
**             (int) selection: Number of the menu entry to unselect, or -1 to do a NOP
** Returns   : (gfxw_port_t *) The modified menu
*/

gfxw_port_t *
sciw_select_item(EngineState *s, gfxw_port_t *menu_port, Menu *menu, int selection);
/* Selects a menu item from a menu port
** Parameters: (EngineState *) s: The state to operate on
**             (gfxw_port_t *) menu_port: The port modify
**             (Menu *) menu: The menu the menu port corresponds to
**             (int) selection: Number of the menu entry to select, or -1 to do a NOP
** Returns   : (gfxw_port_t *) The modified menu
*/

} // End of namespace Sci

#endif // SCI_INCLUDE_SCI_WIDGETS_H
