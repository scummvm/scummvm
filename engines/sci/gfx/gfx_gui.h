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

#ifndef SCI_INCLUDE_SCI_WIDGETS_H
#define SCI_INCLUDE_SCI_WIDGETS_H

#include "sci/engine/state.h"

namespace Sci {

class Menu;

/* SCI-specific widget handling */

/**
 * Flags for windows in SCI0.
 */
enum windowFlags {
	kWindowTransparent  = 0x01,			//!< 0000 0001
	kWindowNoFrame      = 0x02,			//!< 0000 0010 - a window without a frame
	kWindowTitle        = 0x04,			/**
										 * 0000 0100 - Add title bar to
										 * window (10 pixels high, framed,
										 * text is centered and written in
										 * white on dark gray), bits 3-6
										 * are unused
										 */
	kWindowDontDraw     = 0x80,			//!< 1000 0000 - don't draw anything
	kWindowNoDropShadow = 0x1000000,	//!< 0001 0000 0000 0000 0000 0000 0000 (not in SCI)
	kWindowAutoRestore  = 0x2000000
};

/** Button and frame control flags. */
enum controlStateFlags {
	kControlStateEnabled      = 0x0001,  //!< 0001 - enabled buttons (used by the interpreter)
	kControlStateDisabled     = 0x0004,  //!< 0010 - grayed out buttons (used by the interpreter)
	kControlStateFramed       = 0x0008,  //!< 1000 - widgets surrounded by a frame (used by the interpreter)
	kControlStateDitherFramed = 0x1000   //!< 0001 0000 0000 0000 - widgets surrounded by a dithered frame (used in kgraphics)
};

/**
 * Sets the contents of a port used as status bar.
 *
 * @param[in] s				The affected EngineState
 * @param[in] status_bar	The status bar port
 * @param[in] text			The text to draw
 * @param[in] fgcolor		The foreground color
 * @param[in] bgcolor		The background color
 */
void sciw_set_status_bar(EngineState *s, GfxPort *status_bar,
		const Common::String &text, int fgcolor, int bgcolor);

/**
 * Creates a new SCI style window.
 *
 * @param[in] s					The affected EngineState
 * @param[in] area				The screen area to frame (not including a
 * 								potential window title)
 * @param[in] font				Default font number to use
 * @param[in] color				The foreground color to use for drawing
 * @param[in] bgcolor			The background color to use
 * @param[in] title_font		The font to use for the title bar (if any)
 * @param[in] title_color		Color to use for the title bar text
 * @param[in] title_bg_color	Color to use for the title bar background
 * @param[in] title				The text to write into the title bar
 * @param[in] flags				Any ORred combination of window flags
 * @return	A newly allocated port with the requested characteristics
 */
GfxPort *sciw_new_window(EngineState *s, rect_t area, int font,
		gfx_color_t color, gfx_color_t bgcolor, int title_font,
		gfx_color_t title_color, gfx_color_t title_bg_color,
		const char *title, int flags);


/** @name Control widgets */
/** @{ */
/**
 * Creates a new button control list.
 *
 * @param[in] port		The port containing the color values to use for the
 * 						button (the button is /not/ appended to the port
 * 						there)
 * @param[in] ID		Button's ID
 * @param[in] zone		The area occupied by the button
 * @param[in] text		The text to write into the button
 * @param[in] font		The font to use for the button
 * @param[in] selected	Whether the button should be marked as being
 * 						selected by the keyboard focus
 * @param[in] inverse	Whether to inverse the color scheme
 * @param[in] gray		Whether the button should be grayed out
 * @return				The button
 */
GfxList *sciw_new_button_control(GfxPort *port, reg_t ID, rect_t zone,
		const char *text, int font, char selected, char inverse, char gray);

/**
 * Creates a new text control list.
 *
 * @param[in] port		The port containing the color values to use
 * @param[in] ID		Text widget ID
 * @param[in] zone		Area occupied by the text
 * @param[in] text		The text
 * @param[in] font		The font the text is to be drawn in
 * @param[in] align		Horizontal text alignment to use
 * @param[in] frame		Whether a dithered frame should surround the text
 * @param[in] inverse	Whether the text colors should be inversed
 * @return				The text control widget list
 */
GfxList *sciw_new_text_control(GfxPort *port, reg_t ID, rect_t zone,
		const char *text, int font, gfx_alignment_t align, char frame,
		char inverse);

/**
 * Creates a new edit control list.
 *
 * @param[in] port		The port containing the color values to use
 * @param[in] ID		Text widget ID
 * @param[in] zone		Area occupied by the text
 * @param[in] text		The text
 * @param[in] font		The font the text is to be drawn in
 * @param[in] cursor	Cursor position
 * @param[in] inverse	Whether the edit widget should be reversed
 * @return				An appropriate widget list
 */
GfxList *sciw_new_edit_control(GfxPort *port, reg_t ID, rect_t zone,
		const char *text, int font, unsigned int cursor, char inverse);

/**
 * Creates a new icon control list.
 *
 * @param[in] port		The port containing the color values to use
 * @param[in] ID		Text widget ID
 * @param[in] zone		Area occupied by the text
 * @param[in] view		The view index
 * @param[in] loop		The loop index
 * @param[in] cel		The cel to display
 * @param[in] frame		Whether the widget should be surrounded by a frame
 * @param[in] inverse	Whether colors should be inversed
 * @return				An appropriate widget list
 */
GfxList *sciw_new_icon_control(GfxPort *port, reg_t ID, rect_t zone,
		int view, int loop, int cel, char frame, char inverse);

/**
 * Creates a new list control list.
 *
 * @param[in] port:			The port containing the color values to use
 * @param[in] ID:			Text widget ID
 * @param[in] zone:			Area occupied by the text
 * @param[in] font_nr:		Number of the font to use
 * @param[in] entries_list:	List of strings to contain within the list
 * @param[in] entries_nr:	Number of entries in entries_list
 * @param[in] list_top:		First list item that is visible
 * @param[in] selection:	The list item that is selected
 * @param[in] inverse:		The usual meaning
 * @return					An appropriate widget list
 */
GfxList *sciw_new_list_control(GfxPort *port, reg_t ID, rect_t zone,
		int font_nr, const char **entries_list, int entries_nr,
		int list_top, int selection, char inverse);
/** @} */

/** @name Menubar widgets */
/** @{ */

/**
 * Draws the menu bar.
 *
 * @param[in] s:			The EngineState to operate on
 * @param[in] status_bar:	The status bar port to modify
 * @param[in] menubar:		The menu bar to use
 * @param[in] selection:	Number of the menu to hightlight, or -1 for
 * 							'none'
 */
void sciw_set_menubar(EngineState *s, GfxPort *status_bar, Menubar *menubar,
		int selection);

/**
 * Creates a menu port.
 *
 * @param[in] s				The state to operate on
 * @param[in] status_bar	The status bar
 * @param[in] menubar		The menu bar to use
 * @param[in] selection		Number of the menu to interpret
 * @return					The result port
 */
GfxPort *sciw_new_menu(EngineState *s, GfxPort *status_bar,
		Menubar *menubar, int selection);

/**
 * Toggle the selection of a menu item from a menu port.
 *
 * @param[in] menu_port	The port to modify
 * @param[in] menu		The menu the menu port corresponds to
 * @param[in] selection	Number of the menu entry to unselect, or -1 to do
 * 						a NOP
 * @param[in] selected	Whether to set the item's state to selected or not
 * @return				The modified menu
 */
GfxPort *sciw_toggle_item(GfxPort *menu_port, Menu *menu, int selection,
		bool selected);
/** @} */

} // End of namespace Sci

#endif // SCI_INCLUDE_SCI_WIDGETS_H
