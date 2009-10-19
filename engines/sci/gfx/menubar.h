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

#ifndef SCI_GFX_SCI_MENUBAR_H
#define SCI_GFX_SCI_MENUBAR_H

#include "sci/engine/vm_types.h"
#include "sci/gfx/operations.h"

#include "common/array.h"
#include "common/rect.h"

namespace Sci {

struct EngineState;

#define MENU_HBAR_STRING_1 "--!"
#define MENU_HBAR_STRING_2 "-!"
#define MENU_HBAR_STRING_3 "!--"
/* These strings are used in SCI to determine an empty menu line */

#define MENU_BORDER_SIZE 0
/* The number of pixels added to the left and right to the text of a menu on the menu bar */

#define MENU_LEFT_BORDER 5
/* The number of pixels added to the left of the first menu */

#define MENU_BOX_CENTER_PADDING 10
/* Number of pixels to leave in between the left and the right centered text content in boxes that use right centered content */

#define MENU_BOX_LEFT_PADDING 0
/* Number of pixels to pad to the left */
#define MENU_BOX_RIGHT_PADDING 2
/* Number of pixels to pad to the right */

#define MENU_BAR_HEIGHT 10


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

/* Those flags determine whether the corresponding MenuItem entries are valid */
#define MENU_ATTRIBUTE_FLAGS_KEY 0x01
#define MENU_ATTRIBUTE_FLAGS_SAID 0x02


enum MenuType {
	MENU_TYPE_NORMAL = 0,
	MENU_TYPE_HBAR = 1 /* Horizontal bar */
};

class MenuItem : public Common::Serializable {
public:
	MenuType _type; /**< Normal or hbar */
	Common::String _keytext; /**< right-centered part of the text (the key) */

	int _flags;
	byte _said[MENU_SAID_SPEC_SIZE]; /**< Said spec for this item */
	reg_t _saidPos;
	Common::String _text;
	reg_t _textPos;
	int _modifiers; /**< Hotkey for this item */
	int _key; /**< Hotkey for this item */
	int _enabled;
	int _tag;

public:
	MenuItem();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	/**
	 * Determines whether a message/modifiers key pair matches a menu item's key parameters.
	 *
	 * @param[in] message		The message to match
	 * @param[in] modifiers		The modifier flags to match
	 * @return					true on match, false otherwise
	 */
	bool matchKey(int message, int modifiers);
};


class Menu : public Common::Serializable {
public:
	Common::String _title;

	/** Width of the title in pixels */
	int _titleWidth;

	/** Pixel width of the menu window */
	int _width;

	/**
	 * Actual entries into the menu.
	 * Window height equals to number of items times 10.
	 */
	Common::Array<MenuItem> _items;

public:
	Menu();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

//protected:
	// FIXME: This should be (partially) turned into a MenuItem constructor
	int addMenuItem(GfxState *state, MenuType type, const char *left, const char *right,
	                int font, int key, int modifiers, int tag, reg_t text_pos);
};




class Menubar : public Common::Serializable {
public:
	/** The actual menus. */
	Common::Array<Menu> _menus;

public:
	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	/**
	 * Adds a menu to the menubar.
	 *
	 * The menu entries use the following special characters:
	 * '`' : Right justify the following part
	 * ':' : End of this entry
	 * '#' : Function key (replaced by 'F')
	 * '^' : Control key (replaced by \002, which looks like "CTRL")
	 * '=' : Initial tag value
	 * and the special string "--!", which represents a horizontal bar in the
	 * menu.
	 *
	 * @param[in] state			The state the fonts are stored in
	 * @param[in] title			The menu title
	 * @param[in] entries		A string of menu entries
	 * @param[in] font			The font which is to be used for drawing
	 * @param[in] entries_base	Segmented VM address of the entries string
	 */
	void addMenu(GfxState *state, const Common::String &title, const Common::String &entries, int font, reg_t entries_base);


	/**
	 * Sets the attributes for a menu item.
	 *
	 * @param[in] s			The current state
	 * @param[in] menu		The menu number to edit
	 * @param[in] item		The menu item to change
	 * @param[in] attribute	The attribute to modify
	 * @param[in] value		The value the attribute should be set to
	 * @return				0 on success, 1 if either menu or item were invalid
	 */
	int setAttribute(EngineState *s, int menu, int item, int attribute, reg_t value);


	/**
	 * Gets an attribute for a menuitem.
	 *
	 * @param[in] menu		The menu number
	 * @param[in] item		The menu item to read
	 * @param[in] attribute	The attribute to read from
	 * @return				The attribute value, or -1 on error
	 */
	reg_t getAttribute(int menu, int item, int attribute) const;


	/**
	 * Determines whether the specified menu entry may be activated.
	 *
	 * @return	true if the menu item may be selected, false otherwise
	 */
	bool itemValid(int menu, int item) const;


	/**
	 * Maps the pointer position to a (menu,item) tuple.
	 *
	 * @param[in] pointerPos	the current pointer position
	 * @param[in] menu_nr		the current menu (updated by this function if
	 * 							necessary)
	 * @param[in] item_nr		the current menu item (updated by this function
	 * 							if necessary)
	 * @param[in] portBounds	the port bounds of the currently active menu (if any)
	 * @return 					true if the pointer is outside a valid port,
	 * 							false otherwise.
	 */
	bool mapPointer(const Common::Point &pointerPos, int &menu_nr, int &item_nr, Common::Rect portBounds) const;

};

} // End of namespace Sci

#endif // SCI_GFX_SCI_MENUBAR_H
