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

/* Management and drawing operations for the SCI0 menu bar */
/* I currently assume that the hotkey information used in the menu bar is NOT
** used for any actual actions on behalf of the interpreter.
*/

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX

#include "sci/engine/state.h"
#include "sci/gfx/menubar.h"
#include "sci/gui/gui.h"
#include "sci/engine/kernel.h"

namespace Sci {


/* Copies a string into a newly allocated memory part, up to a certain length.
** Parameters: (char *) src: The source string
**             (int) length: The maximum length of the string (not counting
**                           a trailing \0).
** Returns   : (char *) The resulting copy, allocated with malloc().
** To free this string, use the free() command.
** See _SCI_MALLOC() for more information if call fails.
*/
char *sci_strndup(const char *src, size_t length) {
	assert(src);

	size_t rlen = (int)MIN(strlen(src), length) + 1;
	char *strres = (char *)malloc(rlen);
	assert(strres);

	strncpy(strres, src, rlen);
	strres[rlen - 1] = 0;

	return strres;
}


#define SIZE_INF 32767

Menu::Menu() {
	_titleWidth = 0;
	_width = 0;
}

MenuItem::MenuItem() {
	_type = MENU_TYPE_NORMAL;
	_flags = 0;
	memset(_said, 0, sizeof(_said));
	_saidPos = NULL_REG;
	_textPos = NULL_REG;
	_modifiers = 0;
	_key = 0;
	_enabled = 0;
	_tag = 0;
}

int Menu::addMenuItem(GfxState *state, MenuType type, const char *left, const char *right,
						   int font, int key, int modifiers, int tag, reg_t text_pos) {
// Returns the total text size, plus MENU_BOX_CENTER_PADDING if (right != NULL)
	MenuItem newItem;
	MenuItem *item;
	int total_left_size = 0;
	int16 width = 10, height = 10;
	EngineState *s = ((SciEngine *)g_engine)->getEngineState();	// HACK

	item = &newItem;
	item->_type = type;

	if (type == MENU_TYPE_HBAR) {
		_items.push_back(newItem);
		return 0;
	}

	// else assume MENU_TYPE_NORMAL
	item->_text = left;
	if (right) {
		item->_keytext = right;
		item->_keytext.trim();	// Remove trailing whitespace
		item->_flags = MENU_ATTRIBUTE_FLAGS_KEY;
		item->_key = key;
		item->_modifiers = modifiers;
	} else {
		item->_flags = 0;
	}

	if (right) {
		s->_gui->textSize(item->_keytext.c_str(), font, -1, &width, &height);
		total_left_size = MENU_BOX_CENTER_PADDING + width;
	}

	item->_enabled = 1;
	item->_tag = tag;
	item->_textPos = text_pos;
	s->_gui->textSize(left, font, -1, &width, &height);

	_items.push_back(newItem);

	return total_left_size + width;
}

void Menubar::addMenu(GfxState *state, const Common::String &title, const Common::String &entries, int font, reg_t entries_base) {
	char tracker;
	char *left = NULL;
	reg_t left_origin = entries_base;
	int string_len = 0;
	int tag = 0, c_width, max_width = 0;
	int16 height = 10;
	EngineState *s = ((SciEngine *)g_engine)->getEngineState();	// HACK

	Menu menu;

	menu._title = title;
	menu._titleWidth = 10;

	s->_gui->textSize(title.c_str(), font, -1, &(menu._titleWidth), &height);

	const char *entries_p = entries.c_str();

	do {
		tracker = *entries_p++;
		entries_base.offset++;

		if (!left) { // Left string not finished?
			if (tracker == '=') { // Hit early-SCI tag assignment?
				left = sci_strndup(entries_p - string_len - 1, string_len);
				tag = atoi(entries_p++);
				tracker =  *entries_p++;
			}
			if ((tracker == 0 && string_len > 0) || (tracker == '=') || (tracker == ':')) { // End of entry
				MenuType entrytype = MENU_TYPE_NORMAL;
				char *inleft;
				reg_t beginning;

				if (!left)
					left = sci_strndup(entries_p - string_len - 1, string_len);

				inleft = left;
				while (isspace((unsigned char)*inleft))
					inleft++; // Seek beginning of actual string

				if (!strncmp(inleft, MENU_HBAR_STRING_1, strlen(MENU_HBAR_STRING_1))
				        || !strncmp(inleft, MENU_HBAR_STRING_2, strlen(MENU_HBAR_STRING_2))
				        || !strncmp(inleft, MENU_HBAR_STRING_3, strlen(MENU_HBAR_STRING_3))) {
					entrytype = MENU_TYPE_HBAR; // Horizontal bar
					free(left);
					left = NULL;
				}

				beginning = entries_base;
				beginning.offset -= string_len + 1;
#ifdef INCLUDE_OLDGFX
				c_width = menu.addMenuItem(state, entrytype, left, NULL, font, 0, 0, tag, beginning);
#endif
				if (c_width > max_width)
					max_width = c_width;

				string_len = 0;
				free(left);
				left = NULL; // Start over
			} else if (tracker == '`') { // Start of right string
				if (!left) {
					left_origin = entries_base;
					left_origin.offset -= string_len + 1;
					left = sci_strndup(entries_p - string_len - 1, string_len);
				}
				string_len = 0; // Continue with the right string
			} else
				string_len++; // Nothing special

		} else { // Left string finished => working on right string
			if ((tracker == ':') || (tracker == 0)) { // End of entry
				int key, modifiers = 0;

				char *right = sci_strndup(entries_p - string_len - 1, string_len);

				if (right[0] == '#') {
					right[0] = SCI_SPECIAL_CHAR_FUNCTION; // Function key

					key = SCI_K_F1 + ((right[1] - '1') << 8);

					if (right[1] == '0')
						key = SCI_K_F10; // F10

					if (right[2] == '=') {
						tag = atoi(right + 3);
						right[2] = 0;
					};
				} else if (right[0] == '@') { // Alt key
					right[0] = SCI_SPECIAL_CHAR_ALT; // ALT
					key = right[1];
					modifiers = SCI_EVM_ALT;

					if ((key >= 'a') && (key <= 'z'))
						right[1] = key - 'a' + 'A';

					if (right[2] == '=') {
						tag = atoi(right + 3);
						right[2] = 0;
					}
				} else {
					if (right[0] == '^') {
						right[0] = SCI_SPECIAL_CHAR_CTRL; // Control key - there must be a replacement...
						key = right[1];
						modifiers = SCI_EVM_CTRL;

						if ((key >= 'a') && (key <= 'z'))
							right[1] = key - 'a' + 'A';

						if (right[2] == '=') {
							tag = atoi(right + 3);
							right[2] = 0;
						}
					} else {
						key = right[0];
						if ((key >= 'a') && (key <= 'z'))
							right[0] = key - 'a' + 'A';

						if (right[1] == '=') {
							tag = atoi(right + 2);
							right[1] = 0;
						}
					}
					if ((key >= 'A') && (key <= 'Z'))
						key = key - 'A' + 'a'; // Lowercase the key
				}

				int i = strlen(right);

				while (i > 0 && right[--i] == ' ')
					right[i] = 0; // Cut off chars to the right

				c_width = menu.addMenuItem(state, MENU_TYPE_NORMAL, left, right, font, key,
				                                 modifiers, tag, left_origin);
				tag = 0;
				if (c_width > max_width)
					max_width = c_width;

				string_len = 0;
				free(right);
				free(left);
				left = NULL;  // Start over

			} else
				string_len++; // continuing entry
		} // right string finished
	} while (tracker);

	menu._width = max_width;

	_menus.push_back(menu);
}

bool MenuItem::matchKey(int message, int modifiers) {
	if ((_key == message) && ((modifiers & (SCI_EVM_CTRL | SCI_EVM_ALT)) == _modifiers))
		return true;

	if (message == '\t' && _key == 'i' && ((modifiers & (SCI_EVM_CTRL | SCI_EVM_ALT)) == 0) && _modifiers == SCI_EVM_CTRL)
		return true; // Match TAB to ^I

	return 0;
}

int Menubar::setAttribute(EngineState *s, int menu_nr, int item_nr, int attribute, reg_t value) {
	MenuItem *item;

	if ((menu_nr < 0) || (item_nr < 0))
		return 1;

	if ((menu_nr >= (int)_menus.size()) || (item_nr >= (int)_menus[menu_nr]._items.size()))
		return 1;

	item = &_menus[menu_nr]._items[item_nr];

	switch (attribute) {

	case MENU_ATTRIBUTE_SAID:
		if (value.segment) {
			item->_saidPos = value;
			s->_segMan->memcpy(item->_said, value, MENU_SAID_SPEC_SIZE); // Copy Said spec
			item->_flags |= MENU_ATTRIBUTE_FLAGS_SAID;

		} else
			item->_flags &= ~MENU_ATTRIBUTE_FLAGS_SAID;

		break;

	case MENU_ATTRIBUTE_TEXT:
		assert(value.segment);
		item->_text = s->_segMan->getString(value);
		item->_textPos = value;
		break;

	case MENU_ATTRIBUTE_KEY:
		item->_keytext.clear();

		if (value.segment) {

			// FIXME: What happens here if <value> is an extended key? Potential bug. LS
			item->_key = value.offset;
			item->_modifiers = 0;
			item->_keytext = value.offset;
			item->_flags |= MENU_ATTRIBUTE_FLAGS_KEY;
			if ((item->_key >= 'A') && (item->_key <= 'Z'))
				item->_key = item->_key - 'A' + 'a'; // Lowercase the key
		} else {
			item->_flags &= ~MENU_ATTRIBUTE_FLAGS_KEY;
		}
		break;

	case MENU_ATTRIBUTE_ENABLED:
		item->_enabled = value.offset;
		break;

	case MENU_ATTRIBUTE_TAG:
		item->_tag = value.offset;
		break;

	default:
		error("Attempt to set invalid attribute of menu %d, item %d: 0x%04x", menu_nr, item_nr, attribute);
		return 1;
	}

	return 0;
}

reg_t Menubar::getAttribute(int menu_nr, int item_nr, int attribute) const {
	if ((menu_nr < 0) || (item_nr < 0))
		return SIGNAL_REG;

	if ((menu_nr >= (int)_menus.size()) || (item_nr >= (int)_menus[menu_nr]._items.size()))
		return SIGNAL_REG;

	const MenuItem &item = _menus[menu_nr]._items[item_nr];

	switch (attribute) {
	case MENU_ATTRIBUTE_SAID:
		return item._saidPos;

	case MENU_ATTRIBUTE_TEXT:
		return item._textPos;

	case MENU_ATTRIBUTE_KEY:
		return make_reg(0, item._key);

	case MENU_ATTRIBUTE_ENABLED:
		return make_reg(0, item._enabled);

	case MENU_ATTRIBUTE_TAG:
		return make_reg(0, item._tag);

	default:
		warning("Attempt to read invalid attribute from menu %d, item %d: 0x%04x", menu_nr, item_nr, attribute);
		return SIGNAL_REG;
	}
}

bool Menubar::itemValid(int menu_nr, int item_nr) const {
	if ((menu_nr < 0) || (item_nr < 0))
		return false;

	if ((menu_nr >= (int)_menus.size()) || (item_nr >= (int)_menus[menu_nr]._items.size()))
		return false;

	const MenuItem &item = _menus[menu_nr]._items[item_nr];

	if ((item._type == MENU_TYPE_NORMAL) && item._enabled)
		return true;

	return false; // May not be selected
}

bool Menubar::mapPointer(const Common::Point &pointerPos, int &menu_nr, int &item_nr, Common::Rect portBounds) const {

	if (pointerPos.y <= 10) { // Re-evaulate menu
		int x = MENU_LEFT_BORDER;

		for (uint i = 0; i < _menus.size(); i++) {
			int newx = x + MENU_BORDER_SIZE * 2 + _menus[i]._titleWidth;

			if (pointerPos.x < x)
				return false;

			if (pointerPos.x < newx) {
				menu_nr = i;
				item_nr = -1;
			}

			x = newx;
		}
	} else {
		int row = (pointerPos.y / 10) - 1;

		if ((menu_nr < 0) || (menu_nr >= (int)_menus.size()))
			return true; // No menu

		const Menu &menu = _menus[menu_nr]; // Menu is valid, assume that it's popped up

		if ((int)menu._items.size() <= row)
			return true;

		if ((pointerPos.x < portBounds.left) || (pointerPos.x > portBounds.right))
			return true;

		if (itemValid(menu_nr, row))
			item_nr = row; // Only modify if we'll be hitting a valid element

	}

	return false;
}

} // End of namespace Sci

#endif
