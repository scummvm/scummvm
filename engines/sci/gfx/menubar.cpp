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

#include "sci/sci_memory.h"
#include "sci/include/engine.h"
#include "sci/gfx/menubar.h"
#include "sci/engine/kernel.h"

namespace Sci {

#define SIZE_INF 32767

menubar_t *menubar_new() {
	menubar_t *tmp = (menubar_t*)sci_malloc(sizeof(menubar_t));
	tmp->menus_nr = 0;

	return tmp;
}

void menubar_free(menubar_t *menubar) {
	int i;

	for (i = 0; i < menubar->menus_nr; i++) {
		menu_t *menu = &(menubar->menus[i]);
		int j;

		for (j = 0; j < menu->items_nr; j++) {
			if (menu->items[j].keytext)
				free(menu->items[j].keytext);
			if (menu->items[j].text)
				free(menu->items[j].text);
		}

		free(menu->items);
		free(menu->title);
	}

	if (menubar->menus_nr)
		free(menubar->menus);

	free(menubar);
}

int _menubar_add_menu_item(gfx_state_t *state, menu_t *menu, int type, char *left, char *right,
						   int font, int key, int modifiers, int tag, reg_t text_pos) {
// Returns the total text size, plus MENU_BOX_CENTER_PADDING if (right != NULL)
	menu_item_t *item;
	int total_left_size = 0;
	int width, height;

	if (menu->items_nr == 0) {
		menu->items = (menu_item_t *)sci_malloc(sizeof(menu_item_t));
		menu->items_nr = 1;
	} else
		menu->items = (menu_item_t *)sci_realloc(menu->items, sizeof(menu_item_t) * ++(menu->items_nr));

	item = &(menu->items[menu->items_nr - 1]);

	memset(item, 0, sizeof(menu_item_t));

	if ((item->type = type) == MENU_TYPE_HBAR)
		return 0;

	// else assume MENU_TYPE_NORMAL
	item->text = left;
	if (right) {
		int end = strlen(right);
		item->keytext = right;
		while (end && isspace(right[end]))
			right[end--] = 0; // Remove trailing whitespace
		item->flags = MENU_ATTRIBUTE_FLAGS_KEY;
		item->key = key;
		item->modifiers = modifiers;
	} else {
		item->keytext = NULL;
		item->flags = 0;
	}

	if (right) {
		gfxop_get_text_params(state, font, right, SIZE_INF, &width, &height, 0, NULL, NULL, NULL);
		total_left_size = MENU_BOX_CENTER_PADDING + (item->keytext_size = width);
	}

	item->enabled = 1;
	item->tag = tag;
	item->text_pos = text_pos;
	gfxop_get_text_params(state, font, left, SIZE_INF, &width, &height, 0, NULL, NULL, NULL);

	return total_left_size + width;
}

void menubar_add_menu(gfx_state_t *state, menubar_t *menubar, char *title, char *entries, int font, reg_t entries_base) {
	int i, add_freesci = 0;
	menu_t *menu;
	char tracker;
	char *left = NULL, *right;
	reg_t left_origin = entries_base;
	int string_len = 0;
	int tag = 0, c_width, max_width = 0;
	int height;

	if (menubar->menus_nr == 0) {
#ifdef MENU_FREESCI_BLATANT_PLUG
		add_freesci = 1;
#endif
		menubar->menus = (menu_t *)sci_malloc(sizeof(menu_t));
		menubar->menus_nr = 1;
	} else
		menubar->menus = (menu_t *)sci_realloc(menubar->menus, ++(menubar->menus_nr) * sizeof(menu_t));

	menu = &(menubar->menus[menubar->menus_nr-1]);
	memset(menu, 0, sizeof(menu_t));
	menu->items_nr = 0;
	menu->title = sci_strdup(title);

	gfxop_get_text_params(state, font, menu->title, SIZE_INF, &(menu->title_width), &height, 0, NULL, NULL, NULL);

	do {
		tracker = *entries++;
		entries_base.offset++;

		if (!left) { // Left string not finished?
			if (tracker == '=') { // Hit early-SCI tag assignment?
				left = sci_strndup(entries - string_len - 1, string_len);
				tag = atoi(entries++);
				tracker =  *entries++;
			}
			if ((tracker == 0 && string_len > 0) || (tracker == '=') || (tracker == ':')) { // End of entry
				int entrytype = MENU_TYPE_NORMAL;
				char *inleft;
				reg_t beginning;

				if (!left)
					left = sci_strndup(entries - string_len - 1, string_len);

				inleft = left;
				while (isspace(*inleft))
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
				c_width = _menubar_add_menu_item(state, menu, entrytype, left, NULL, font, 0, 0, tag, beginning);
				if (c_width > max_width)
					max_width = c_width;

				string_len = 0;
				left = NULL; // Start over
			} else if (tracker == '`') { // Start of right string
				if (!left) {
					left_origin = entries_base;
					left_origin.offset -= string_len + 1;
					left = sci_strndup(entries - string_len - 1, string_len);
				}
				string_len = 0; // Continue with the right string
			} else
				string_len++; // Nothing special

		} else { // Left string finished => working on right string
			if ((tracker == ':') || (tracker == 0)) { // End of entry
				int key, modifiers = 0;

				right = sci_strndup(entries - string_len - 1, string_len);

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
					};
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

				i = strlen(right);

				while (i > 0 && right[--i] == ' ')
					right[i] = 0; // Cut off chars to the right

				c_width = _menubar_add_menu_item(state, menu, MENU_TYPE_NORMAL, left, right, font, key,
				                                 modifiers, tag, left_origin);
				tag = 0;
				if (c_width > max_width)
					max_width = c_width;

				string_len = 0;
				left = NULL;  // Start over

			} else
				string_len++; // continuing entry
		} // right string finished
	} while (tracker);

#ifdef MENU_FREESCI_BLATANT_PLUG
	if (add_freesci) {
		char *freesci_text = sci_strdup("About FreeSCI");
		c_width = _menubar_add_menu_item(state, menu, MENU_TYPE_NORMAL, freesci_text, NULL, font, 0, 0, 0, NULL_REG);
		if (c_width > max_width)
			max_width = c_width;

		menu->items[menu->items_nr-1].flags = MENU_FREESCI_BLATANT_PLUG;
	}
#endif
	menu->width = max_width;
}

int menubar_match_key(menu_item_t *item, int message, int modifiers) {
	if ((item->key == message) && ((modifiers & (SCI_EVM_CTRL | SCI_EVM_ALT)) == item->modifiers))
		return 1;

	if (message == '\t' && item->key == 'i' && ((modifiers & (SCI_EVM_CTRL | SCI_EVM_ALT)) == 0) && item->modifiers == SCI_EVM_CTRL)
		return 1; // Match TAB to ^I

	return 0;
}

int menubar_set_attribute(EngineState *s, int menu_nr, int item_nr, int attribute, reg_t value) {
	menubar_t *menubar = s->menubar;
	menu_item_t *item;

	if ((menu_nr < 0) || (item_nr < 0))
		return 1;

	if ((menu_nr >= menubar->menus_nr) || (item_nr >= menubar->menus[menu_nr].items_nr))
		return 1;

	item = menubar->menus[menu_nr].items + item_nr;

	switch (attribute) {

	case MENU_ATTRIBUTE_SAID:
		if (value.segment) {
			item->said_pos = value;
			memcpy(item->said, kernel_dereference_bulk_pointer(s, value, 0), MENU_SAID_SPEC_SIZE); // Copy Said spec
			item->flags |= MENU_ATTRIBUTE_FLAGS_SAID;

		} else
			item->flags &= ~MENU_ATTRIBUTE_FLAGS_SAID;

		break;

	case MENU_ATTRIBUTE_TEXT:
		free(item->text);
		assert(value.segment);
		item->text = sci_strdup(kernel_dereference_char_pointer(s, value, 0));
		item->text_pos = value;
		break;

	case MENU_ATTRIBUTE_KEY:
		if (item->keytext)
			free(item->keytext);

		if (value.segment) {

			// FIXME: What happens here if <value> is an extended key? Potential bug. LS
			item->key = value.offset;
			item->modifiers = 0;
			item->keytext = (char *)sci_malloc(2);
			item->keytext[0] = value.offset;
			item->keytext[1] = 0;
			item->flags |= MENU_ATTRIBUTE_FLAGS_KEY;
			if ((item->key >= 'A') && (item->key <= 'Z'))
				item->key = item->key - 'A' + 'a'; // Lowercase the key
		} else {
			item->keytext = NULL;
			item->flags &= ~MENU_ATTRIBUTE_FLAGS_KEY;
		}
		break;

	case MENU_ATTRIBUTE_ENABLED:
		item->enabled = value.offset;
		break;

	case MENU_ATTRIBUTE_TAG:
		item->tag = value.offset;
		break;

	default:
		sciprintf("Attempt to set invalid attribute of menu %d, item %d: 0x%04x\n", menu_nr, item_nr, attribute);
		return 1;
	}

	return 0;
}

reg_t menubar_get_attribute(EngineState *s, int menu_nr, int item_nr, int attribute) {
	menubar_t *menubar = s->menubar;
	menu_item_t *item;

	if ((menu_nr < 0) || (item_nr < 0))
		return make_reg(0, -1);

	if ((menu_nr >= menubar->menus_nr) || (item_nr >= menubar->menus[menu_nr].items_nr))
		return make_reg(0, -1);

	item = menubar->menus[menu_nr].items + item_nr;

	switch (attribute) {
	case MENU_ATTRIBUTE_SAID:
		return item->said_pos;

	case MENU_ATTRIBUTE_TEXT:
		return item->text_pos;

	case MENU_ATTRIBUTE_KEY:
		return make_reg(0, item->key);

	case MENU_ATTRIBUTE_ENABLED:
		return make_reg(0, item->enabled);

	case MENU_ATTRIBUTE_TAG:
		return make_reg(0, item->tag);

	default:
		sciprintf("Attempt to read invalid attribute from menu %d, item %d: 0x%04x\n", menu_nr, item_nr, attribute);
		return make_reg(0, -1);
	}
}

int menubar_item_valid(EngineState *s, int menu_nr, int item_nr) {
	menubar_t *menubar = s->menubar;
	menu_item_t *item;

	if ((menu_nr < 0) || (item_nr < 0))
		return 0;

	if ((menu_nr >= menubar->menus_nr) || (item_nr >= menubar->menus[menu_nr].items_nr))
		return 0;

	item = menubar->menus[menu_nr].items + item_nr;

	if ((item->type == MENU_TYPE_NORMAL) && item->enabled)
		return 1;

	return 0; // May not be selected
}

int menubar_map_pointer(EngineState *s, int *menu_nr, int *item_nr, gfxw_port_t *port) {
	menubar_t *menubar = s->menubar;
	menu_t *menu;

	if (s->gfx_state->pointer_pos.y <= 10) { // Re-evaulate menu
		int x = MENU_LEFT_BORDER;
		int i;

		for (i = 0; i < menubar->menus_nr; i++) {
			int newx = x + MENU_BORDER_SIZE * 2 + menubar->menus[i].title_width;

			if (s->gfx_state->pointer_pos.x < x)
				return 0;

			if (s->gfx_state->pointer_pos.x < newx) {
				*menu_nr = i;
				*item_nr = -1;
			}

			x = newx;
		}
		return 0;
	} else {
		int row = (s->gfx_state->pointer_pos.y / 10) - 1;

		if ((*menu_nr < 0) || (*menu_nr >= menubar->menus_nr))
			return 1; // No menu
		else
			menu = menubar->menus + *menu_nr; // Menu is valid, assume that it's popped up

		if (menu->items_nr <= row)
			return 1;

		if ((s->gfx_state->pointer_pos.x < port->bounds.x) || (s->gfx_state->pointer_pos.x > port->bounds.x + port->bounds.xl))
			return 1;

		if (menubar_item_valid(s, *menu_nr, row))
			*item_nr = row; // Only modify if we'll be hitting a valid element

		return 0;
	}
}

} // End of namespace Sci
