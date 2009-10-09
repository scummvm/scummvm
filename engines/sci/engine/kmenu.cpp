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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/gfx/gfx_gui.h"
#include "sci/gfx/menubar.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxVisual
#include "sci/gui/gui.h"
#include "sci/gui/gui_cursor.h"

namespace Sci {

reg_t kAddMenu(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	Common::String contents = s->_segMan->getString(argv[1]);

	s->_menubar->addMenu(s->gfx_state, name,
	                 contents, s->titlebar_port->_font, argv[1]);

	return s->r_acc;

}


reg_t kSetMenu(EngineState *s, int argc, reg_t *argv) {
	int index = argv[0].toUint16();
	int i = 2;

	while (i < argc) {
		s->_menubar->setAttribute(s, (index >> 8) - 1, (index & 0xff) - 1, argv[i - 1].toUint16(), argv[i]);
		i += 2;
	}

	return s->r_acc;
}

reg_t kGetMenu(EngineState *s, int argc, reg_t *argv) {
	int index = argv[0].toUint16();

	return s->_menubar->getAttribute((index >> 8) - 1, (index & 0xff) - 1, argv[1].toUint16());
}


reg_t kDrawStatus(EngineState *s, int argc, reg_t *argv) {
	reg_t textReference = argv[0];
	Common::String text;
	int16 colorPen = (argc > 1) ? argv[1].toSint16() : 0; // old code was: s->status_bar_foreground;
	int16 colorBack = (argc > 2) ? argv[2].toSint16() : s->resMan->isVGA() ? 255 : 15; // s->status_bar_background;

	if (!textReference.isNull()) {
		// Sometimes this is called without giving text, if thats the case dont process it
		text = s->_segMan->getString(textReference);

		s->_gui->drawStatus(s->strSplit(text.c_str(), NULL).c_str(), colorPen, colorBack);
	}
	return s->r_acc;
}

reg_t kDrawMenuBar(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].toSint16())
		s->_gui->drawMenuBar();
	else
		s->_gui->clearMenuBar();
	return s->r_acc;
}


static int _menu_go_down(Menubar *menubar, int menu_nr, int item_nr) {
	int seeker;
	const int max = menubar->_menus[menu_nr]._items.size();
	seeker = item_nr + 1;

	while ((seeker < max) && !menubar->itemValid(menu_nr, seeker))
		++seeker;

	if (seeker != max)
		return seeker;
	else
		return item_nr;
}

#define FULL_REDRAW \
	s->visual->draw(Common::Point(0, 0)); \
	gfxop_update(s->gfx_state);


reg_t kMenuSelect(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t event = argv[0];
	/*int pause_sound = (argc > 1) ? argv[1].toUint16() : 1;*/ /* FIXME: Do this eventually */
	bool claimed = false;
	int type = GET_SEL32V(event, type);
	int message = GET_SEL32V(event, message);
	int modifiers = GET_SEL32V(event, modifiers);
	int menu_nr = -1, item_nr = 0;
	MenuItem *item;
	int menu_mode = 0; /* Menu is active */
	int mouse_down = 0;

#ifdef DEBUG_PARSER
	const int debug_parser = 1;
#else
	const int debug_parser = 0;
#endif

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	/* Check whether we can claim the event directly as a keyboard or said event */
	if (type & (SCI_EVT_KEYBOARD | SCI_EVT_SAID)) {
		int menuc, itemc;

		if ((type == SCI_EVT_KEYBOARD)
		        && (message == SCI_K_ESC))
			menu_mode = 1;

		else if ((type == SCI_EVT_SAID) || message) { /* Don't claim 0 keyboard event */
			debugC(2, kDebugLevelMenu, "Menu: Got %s event: %04x/%04x\n",
			          ((type == SCI_EVT_SAID) ? "SAID" : "KBD"), message, modifiers);

			for (menuc = 0; menuc < (int)s->_menubar->_menus.size(); menuc++)
				for (itemc = 0; itemc < (int)s->_menubar->_menus[menuc]._items.size(); itemc++) {
					item = &s->_menubar->_menus[menuc]._items[itemc];

					debugC(2, kDebugLevelMenu, "Menu: Checking against %s: %04x/%04x (type %d, %s)\n",
					          !item->_text.empty() ? item->_text.c_str() : "--bar--", item->_key, item->_modifiers,
					          item->_type, item->_enabled ? "enabled" : "disabled");

					if ((item->_type == MENU_TYPE_NORMAL && item->_enabled)
					    && ((type == SCI_EVT_KEYBOARD
					           && item->matchKey(message, modifiers)
					        )
					      ||
					        (type == SCI_EVT_SAID
					           && (item->_flags & MENU_ATTRIBUTE_FLAGS_SAID)
					           && said(s, item->_said, debug_parser) != SAID_NO_MATCH
					        )
					       )
					   ) {
						/* Claim the event */
						debugC(2, kDebugLevelMenu, "Menu: Event CLAIMED for %d/%d\n", menuc, itemc);
						claimed = true;
						menu_nr = menuc;
						item_nr = itemc;
					}
				}
		}
	}

	Common::Point cursorPos = s->_cursor->getPosition();

	if ((type == SCI_EVT_MOUSE_PRESS) && (cursorPos.y < 10)) {
		menu_mode = 1;
		mouse_down = 1;
	}

	if (menu_mode) {
		int old_item;
		int old_menu;
		GfxPort *port = NULL;

		item_nr = -1;

		/* Default to menu 0, unless the mouse was used to generate this effect */
		if (mouse_down)
			s->_menubar->mapPointer(cursorPos, menu_nr, item_nr, port);
		else
			menu_nr = 0;

		sciw_set_menubar(s, s->titlebar_port, s->_menubar, menu_nr);
		FULL_REDRAW;

		old_item = -1;
		old_menu = -1;

		while (menu_mode) {
			sci_event_t ev = gfxop_get_event(s->gfx_state, SCI_EVT_ANY);

			claimed = false;

			switch (ev.type) {
			case SCI_EVT_QUIT:
				quit_vm();
				return NULL_REG;

			case SCI_EVT_KEYBOARD:
				switch (ev.data) {

				case '`':
					if (ev.buckybits & SCI_EVM_CTRL)
						s->visual->print(0);
					break;

				case SCI_K_ESC:
					menu_mode = 0;
					break;

				case SCI_K_ENTER:
					menu_mode = 0;
					if ((item_nr >= 0) && (menu_nr >= 0))
						claimed = true;
					break;

				case SCI_K_LEFT:
					if (menu_nr > 0)
						--menu_nr;
					else
						menu_nr = s->_menubar->_menus.size() - 1;

					item_nr = _menu_go_down(s->_menubar, menu_nr, -1);
					break;

				case SCI_K_RIGHT:
					if (menu_nr < ((int)s->_menubar->_menus.size() - 1))
						++menu_nr;
					else
						menu_nr = 0;

					item_nr = _menu_go_down(s->_menubar, menu_nr, -1);
					break;

				case SCI_K_UP:
					if (item_nr > -1) {

						do { --item_nr; }
						while ((item_nr > -1) && !s->_menubar->itemValid(menu_nr, item_nr));
					}
					break;

				case SCI_K_DOWN: {
					item_nr = _menu_go_down(s->_menubar, menu_nr, item_nr);
				}
				break;

				}
				break;

			case SCI_EVT_MOUSE_RELEASE:
				{
				Common::Point curMousePos = s->_cursor->getPosition();
				menu_mode = (curMousePos.y < 10);
				claimed = !menu_mode && !s->_menubar->mapPointer(curMousePos, menu_nr, item_nr, port);
				mouse_down = 0;
				}
				break;

			case SCI_EVT_MOUSE_PRESS:
				mouse_down = 1;
				break;

			case SCI_EVT_NONE:
				gfxop_sleep(s->gfx_state, 2500 / 1000);
				break;
			}

			if (mouse_down)
				s->_menubar->mapPointer(s->_cursor->getPosition(), menu_nr, item_nr, port);

			if ((item_nr > -1 && old_item == -1) || (menu_nr != old_menu)) { /* Update menu */

				sciw_set_menubar(s, s->titlebar_port, s->_menubar, menu_nr);

				delete port;

				port = sciw_new_menu(s, s->titlebar_port, s->_menubar, menu_nr);
				s->wm_port->add((GfxContainer *)s->wm_port, port);

				if (item_nr > -1)
					old_item = -42; /* Enforce redraw in next step */
				else {
					FULL_REDRAW;
				}

			} /* ...if the menu changed. */

			/* Remove the active menu item, if neccessary */
			if (item_nr != old_item) {
				port = sciw_toggle_item(port, &(s->_menubar->_menus[menu_nr]), old_item, false);
				port = sciw_toggle_item(port, &(s->_menubar->_menus[menu_nr]), item_nr, true);
				FULL_REDRAW;
			}

			old_item = item_nr;
			old_menu = menu_nr;

		} /* while (menu_mode) */

		if (port) {
			delete port;
			port = NULL;

			sciw_set_status_bar(s, s->titlebar_port, s->_statusBarText, s->status_bar_foreground, s->status_bar_background);
			gfxop_update(s->gfx_state);
		}
		FULL_REDRAW;
	}

	if (claimed) {
		PUT_SEL32(event, claimed, make_reg(0, 1));

		if (menu_nr > -1) {
			s->r_acc = make_reg(0, ((menu_nr + 1) << 8) | (item_nr + 1));
		} else
			s->r_acc = NULL_REG;

		debugC(2, kDebugLevelMenu, "Menu: Claim -> %04x\n", s->r_acc.offset);
	} else
		s->r_acc = NULL_REG; /* Not claimed */

	return s->r_acc;
}

} // End of namespace Sci
