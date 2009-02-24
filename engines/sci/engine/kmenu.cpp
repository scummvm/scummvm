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

#include "sci/include/sciresource.h"
#include "sci/include/engine.h"
#include "sci/gfx/sci_widgets.h"
#include "sci/gfx/menubar.h"
#include "sci/engine/kernel.h"

namespace Sci {

reg_t
kAddMenu(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *name = kernel_dereference_char_pointer(s, argv[0], 0);
	char *contents = kernel_dereference_char_pointer(s, argv[1], 0);

	menubar_add_menu(s->gfx_state, s->menubar, name,
	                 contents, s->titlebar_port->font_nr, argv[1]);

	return s->r_acc;

}


reg_t
kSetMenu(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int index = UKPV(0);
	int i = 2;

	while (i < argc) {
		menubar_set_attribute(s, (index >> 8) - 1, (index & 0xff) - 1, UKPV(i - 1), argv[i]);
		i += 2;
	}

	return s->r_acc;
}

reg_t
kGetMenu(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int index = UKPV(0);

	return menubar_get_attribute(s, (index >> 8) - 1, (index & 0xff) - 1, UKPV(1));
}


reg_t
kDrawStatus(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t text = argv[0];
	int fgcolor = SKPV_OR_ALT(1, s->status_bar_foreground);
	int bgcolor = SKPV_OR_ALT(2, s->status_bar_background);

	s->titlebar_port->color.visual = *(get_pic_color(s, fgcolor));
	s->titlebar_port->color.mask = GFX_MASK_VISUAL;
	s->titlebar_port->bgcolor.visual = *(get_pic_color(s, bgcolor));
	s->titlebar_port->bgcolor.mask = GFX_MASK_VISUAL;

	s->status_bar_foreground = fgcolor;
	s->status_bar_background = bgcolor;

	if (NULL != s->status_bar_text) {
		free(s->status_bar_text);
		s->status_bar_text = NULL;
	}

	if (text.segment)
		s->status_bar_text = sci_strdup(kernel_dereference_char_pointer(s, text, 0));

	sciw_set_status_bar(s, s->titlebar_port, s->status_bar_text, fgcolor, bgcolor);

	gfxop_update(s->gfx_state);

	return s->r_acc;
}


reg_t
kDrawMenuBar(EngineState *s, int funct_nr, int argc, reg_t *argv) {

	if (SKPV(0))
		sciw_set_menubar(s, s->titlebar_port, s->menubar, -1);
	else
		sciw_set_status_bar(s, s->titlebar_port, NULL, 0, 0);

	s->titlebar_port->draw(GFXW(s->titlebar_port), Common::Point(0, 0));
	gfxop_update(s->gfx_state);

	return s->r_acc;
}


#define ABOUT_FREESCI_PAGES_NR 7

static struct {
	const char *title;
	const char *body;
	int fgcolor, bgcolor;
} _about_freesci_pages[ABOUT_FREESCI_PAGES_NR] = {
	{"FreeSCI hackers and contributors",
		"Alexander R. Angas\nDirectX 8 driver, Event sound server, Generic memory functions"
		"\n\n"
		"Anders Baden Nielsen\nPPC testing"
		"\n\n"
		"Bas Zoetekouw\nMan pages, debian package management, CVS maintenance"
		"\n\n"
		"Carl Muckenhoupt\nSources to the SCI resource viewer tools that started it all"
		"\n\n"
		"Chris Kehler\nMakefile enhancements"
		"\n\n"
		"Christoph Reichenbach\nUN*X code, VM/Graphics/Sound/other infrastructure"
		"\n\n"
		"Christopher T. Lansdown\nOriginal CVS maintainer, Alpha compatibility fixes"
		, 0, 15},
	{"More FreeSCI hackers and contributors",
	 "Claudio Matsuoka\nCVS snapshots, daily builds, BeOS and cygwin ports"
	 "\n\n"
	 "Dark Minister\nSCI research (bytecode and parser)"
	 "\n\n"
	 "Dmitry Jemerov\nPort to the Win32 platform, numerous bugfixes"
	 "\n\n"
	 "Emmanuel Jeandel\nBugfixes and bug reports"
	 "\n\n"
	 "Francois-R Boyer\nMT-32 information and mapping code"
	 "\n\n"
	 "George Reid\nFreeBSD package management"
	 "\n\n"
	 "Hubert Maier\nAmigaOS 4 port"
	 , 0, 15},
	{"Even more FreeSCI hackers & contributors",
	 "Hugues Valois\nGame selection menu"
	 "\n\n"
	 "Johannes Manhave\nDocument format translation"
	 "\n\n"
	 "Jordi Vilalta\nNumerous code and website clean-up patches"
	 "\n\n"
	 "Lars Skovlund\nProject maintenance, most documentation, bugfixes, SCI1 support"
	 "\n\n"
	 "Magnus Reftel\nHeap implementation, Python class viewer, bugfixes"
	 "\n\n"
	 "Matt Hargett\nClean-ups, bugfixes, Hardcore QA, Win32"
	 "\n\n"
	 "Max Horn\nSetJump implementation"
	 , 0, 15},
	{"Still more of them",
	 "Paul David Doherty\nGame version information"
	 "\n\n"
	 "Petr Vyhnak\nThe DCL-INFLATE algorithm, many Win32 improvements"
	 "\n\n"
	 "Rainer Canavan\nIRIX MIDI driver and bug fixes"
	 "\n\n"
	 "Rainer De Temple\nSCI research"
	 "\n\n"
	 "Ravi I.\nSCI0 sound resource specification"
	 "\n\n"
	 "Ruediger Hanke\nPort to the MorphOS platform"
	 "\n\n"
	 "Rune Orsval\nConfiguration file editor"
	 , 0, 15},
	{"Is there no end to these contributors?",
	 "Rickard Lind\nMT32->GM MIDI mapping magic, sound research"
	 "\n\n"
	 "Rink Springer\nPort to the DOS platform, several bug fixes"
	 "\n\n"
	 "Robey Pointer\nBug tracking system hosting"
	 "\n\n"
	 "Sergey Lapin\nPort of Carl's type 2 decompression code"
	 "\n\n"
	 "Solomon Peachy\nSDL ports and much of the sound subsystem"
	 "\n\n"
	 "Vyacheslav Dikonov\nConfig script improvements"
	 "\n\n"
	 "Walter van Niftrik\nPorts to the Dreamcast and GP32 platforms",
	 0, 15},
	{"The CSCI5573 Team at CU Boulder",
	 "Xiaojun Chen\nSean Terrell\nChristoph Reichenbach\n\n"
	 "Special thanks to Prof. Dr. Gary Nutt\n\nfor allowing the FreeSCI VM extension as a\ncourse project in his Advanced OS course"
	 , 0, 15},
	{"Special Thanks",
	 "Special Thanks as well\n\n\nto the linuxgames.com and telefragged.com crew\nfor hosting us\n\n"
	 "To the savannah.gnu.org staff\nfor hosting our mailing list\n\n"
	 "To Bob Heitman and Corey Cole for their support"
	 , 0, 15}
};


void
about_freesci(EngineState *s) {
	int page;
	gfxw_port_t *port;
	int bodyfont, titlefont;
	resource_t *bodyfont_res = NULL;
	int i;

	titlefont = s->titlebar_port->font_nr;

	i = 999;
	while (!bodyfont_res && (i > -1))
		bodyfont_res = scir_test_resource(s->resmgr, sci_font, i--);

	if (i == -1) {
		sciprintf("Sorry, couldn't find a font...\n");
		return;
	}

	bodyfont = i + 1;
	for (page = 0; page < ABOUT_FREESCI_PAGES_NR; ++page) {
		sci_event_t event;
		int cont = 2;
		int width, height, width2, foo;

		_about_freesci_pages[page].fgcolor = 0;
		_about_freesci_pages[page].bgcolor = 15;

		gfxop_get_text_params(s->gfx_state, bodyfont, _about_freesci_pages[page].body, 300, &width, &height, 0,
		                      NULL, NULL, NULL);
		gfxop_get_text_params(s->gfx_state, titlefont, _about_freesci_pages[page].title, 300, &width2, &foo, 0,
		                      NULL, NULL, NULL);

		width += 4;
		width2 += 4;
		height += 12;

		if (width2 > width)
			width = width2;

		port = sciw_new_window(s, gfx_rect(156 - (width >> 1), 100 - (height >> 1), width, height),
		                       bodyfont, s->ega_colors[_about_freesci_pages[page].fgcolor],
		                       s->ega_colors[_about_freesci_pages[page].bgcolor],
		                       titlefont, s->ega_colors[15], s->ega_colors[0],
		                       _about_freesci_pages[page].title, WINDOW_FLAG_TITLE);

		port->add(GFXWC(port), GFXW(gfxw_new_text(s->gfx_state, gfx_rect(0, 0, width, height), bodyfont,
		                            _about_freesci_pages[page].body,
		                            ALIGN_CENTER, ALIGN_CENTER, port->color, port->color,
		                            port->bgcolor, 0)
		                           ));

		s->visual->add(GFXWC(s->visual), GFXW(port));

		port->add_dirty_abs(GFXWC(port), gfx_rect_fullscreen, 1);
		s->visual->draw(GFXW(s->visual), Common::Point(0, 0));
		gfxop_update(s->gfx_state);

		while (cont) {
			event = gfxop_get_event(s->gfx_state, SCI_EVT_ANY);

			if (event.type == SCI_EVT_MOUSE_RELEASE || event.type == SCI_EVT_MOUSE_PRESS)
				--cont;

			if (event.type == SCI_EVT_QUIT) {
				quit_vm();
				return;
			}

			if (event.type == SCI_EVT_KEYBOARD)
				cont = 0;

			gfxop_usleep(s->gfx_state, 25000);
		}


		port->widfree(GFXW(port));
		s->visual->draw(GFXW(s->visual), Common::Point(0, 0));
		gfxop_update(s->gfx_state);

	}
}


static inline int
_menu_go_down(EngineState *s, int menu_nr, int item_nr) {
	int seeker, max = s->menubar->menus[menu_nr].items_nr;
	seeker = item_nr + 1;

	while ((seeker < max) && !menubar_item_valid(s, menu_nr, seeker))
		++seeker;

	if (seeker != max)
		return seeker;
	else return item_nr;
}

#define FULL_REDRAW \
	s->visual->draw(GFXW(s->visual), Common::Point(0, 0)); \
	gfxop_update(s->gfx_state);


reg_t
kMenuSelect(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t event = argv[0];
	/*int pause_sound = UKPV_OR_ALT(1, 1);*/ /* FIXME: Do this eventually */
	int claimed = 0;
	int type = GET_SEL32V(event, type);
	int message = GET_SEL32V(event, message);
	int modifiers = GET_SEL32V(event, modifiers);
	int menu_nr = -1, item_nr = 0;
	menu_item_t *item;
	int menu_mode = 0; /* Menu is active */
	int mouse_down = 0;

	gfxop_set_clip_zone(s->gfx_state, gfx_rect_fullscreen);

	/* Check whether we can claim the event directly as a keyboard or said event */
	if (type & (SCI_EVT_KEYBOARD | SCI_EVT_SAID)) {
		int menuc, itemc;

		if ((type == SCI_EVT_KEYBOARD)
		        && (message == SCI_K_ESC))
			menu_mode = 1;

		else if ((type == SCI_EVT_SAID) || message) { /* Don't claim 0 keyboard event */
			SCIkdebug(SCIkMENU, "Menu: Got %s event: %04x/%04x\n",
			          ((type == SCI_EVT_SAID) ? "SAID" : "KBD"), message, modifiers);

			for (menuc = 0; menuc < s->menubar->menus_nr; menuc++)
				for (itemc = 0; itemc < s->menubar->menus[menuc].items_nr; itemc++) {
					item = s->menubar->menus[menuc].items + itemc;

					SCIkdebug(SCIkMENU, "Menu: Checking against %s: %04x/%04x (type %d, %s)\n",
					          item->text ? item->text : "--bar--", item->key, item->modifiers,
					          item->type, item->enabled ? "enabled" : "disabled");

					if (((item->type == MENU_TYPE_NORMAL)
					        && (item->enabled))
					        && (((type == SCI_EVT_KEYBOARD) /* keyboard event */
					             && menubar_match_key(item, message, modifiers))
					            || ((type == SCI_EVT_SAID) /* Said event */
					                && (item->flags & MENU_ATTRIBUTE_FLAGS_SAID)
					                && (said(s, item->said, (s->debug_mode & (1 << SCIkPARSER_NR))) != SAID_NO_MATCH)
					               )
					           )
					   ) {
						/* Claim the event */
						SCIkdebug(SCIkMENU, "Menu: Event CLAIMED for %d/%d\n", menuc, itemc);
						claimed = 1;
						menu_nr = menuc;
						item_nr = itemc;
					}
				}
		}
	}

	if ((type == SCI_EVT_MOUSE_PRESS) && (s->gfx_state->pointer_pos.y < 10)) {
		menu_mode = 1;
		mouse_down = 1;
	}

	if (menu_mode) {
		int old_item;
		int old_menu;
		gfxw_port_t *port = NULL;

		item_nr = -1;

		/* Default to menu 0, unless the mouse was used to generate this effect */
		if (mouse_down)
			menubar_map_pointer(s, &menu_nr, &item_nr, port);
		else
			menu_nr = 0;

		sciw_set_menubar(s, s->titlebar_port, s->menubar, menu_nr);
		FULL_REDRAW;

		old_item = -1;
		old_menu = -1;

		while (menu_mode) {
			sci_event_t ev = gfxop_get_event(s->gfx_state, SCI_EVT_ANY);

			claimed = 0;

			switch (ev.type) {
			case SCI_EVT_QUIT:
				quit_vm();
				return NULL_REG;

			case SCI_EVT_KEYBOARD:
				switch (ev.data) {

				case '`':
					if (ev.buckybits & SCI_EVM_CTRL)
						s->visual->print(GFXW(s->visual), 0);
					break;

				case SCI_K_ESC:
					menu_mode = 0;
					break;

				case SCI_K_ENTER:
					menu_mode = 0;
					if ((item_nr >= 0) && (menu_nr >= 0))
						claimed = 1;
					break;

				case SCI_K_LEFT:
					if (menu_nr > 0)
						--menu_nr;
					else
						menu_nr = s->menubar->menus_nr - 1;

					item_nr = _menu_go_down(s, menu_nr, -1);
					break;

				case SCI_K_RIGHT:
					if (menu_nr < (s->menubar->menus_nr - 1))
						++menu_nr;
					else
						menu_nr = 0;

					item_nr = _menu_go_down(s, menu_nr, -1);
					break;

				case SCI_K_UP:
					if (item_nr > -1) {

						do { --item_nr; }
						while ((item_nr > -1) && !menubar_item_valid(s, menu_nr, item_nr));
					}
					break;

				case SCI_K_DOWN: {
					item_nr = _menu_go_down(s, menu_nr, item_nr);
				}
				break;

				}
				break;

			case SCI_EVT_MOUSE_RELEASE:
				menu_mode = (s->gfx_state->pointer_pos.y < 10);
				claimed = !menu_mode && !menubar_map_pointer(s, &menu_nr, &item_nr, port);
				mouse_down = 0;
				break;

			case SCI_EVT_MOUSE_PRESS:
				mouse_down = 1;
				break;

			case SCI_EVT_NONE:
				gfxop_usleep(s->gfx_state, 2500);
				break;
			}

			if (mouse_down)
				menubar_map_pointer(s, &menu_nr, &item_nr, port);

			if ((item_nr > -1 && old_item == -1) || (menu_nr != old_menu)) { /* Update menu */

				sciw_set_menubar(s, s->titlebar_port, s->menubar, menu_nr);

				if (port)
					port->widfree(GFXW(port));

				port = sciw_new_menu(s, s->titlebar_port, s->menubar, menu_nr);
				s->wm_port->add(GFXWC(s->wm_port), GFXW(port));

				if (item_nr > -1)
					old_item = -42; /* Enforce redraw in next step */
				else {
					FULL_REDRAW;
				}

			} /* ...if the menu changed. */

			/* Remove the active menu item, if neccessary */
			if (item_nr != old_item) {
				port = sciw_unselect_item(s, port, s->menubar->menus + menu_nr, old_item);
				port = sciw_select_item(s, port, s->menubar->menus + menu_nr, item_nr);
				FULL_REDRAW;
			}

			old_item = item_nr;
			old_menu = menu_nr;

		} /* while (menu_mode) */

		if (port) {
			port->widfree(GFXW(port));
			port = NULL;

			sciw_set_status_bar(s, s->titlebar_port, s->status_bar_text, s->status_bar_foreground, s->status_bar_background);
			gfxop_update(s->gfx_state);
		}
		FULL_REDRAW;
	}

	if (claimed) {
		PUT_SEL32(event, claimed, make_reg(0, 1));

		if (menu_nr > -1) {
			s->r_acc = make_reg(0, ((menu_nr + 1) << 8) | (item_nr + 1));
#ifdef MENU_FREESCI_BLATANT_PLUG
			if (s->menubar->menus[menu_nr].items[item_nr].flags == MENU_FREESCI_BLATANT_PLUG)
				about_freesci(s);
#endif

		} else
			s->r_acc = NULL_REG;

		SCIkdebug(SCIkMENU, "Menu: Claim -> %04x\n", s->r_acc.offset);
	} else s->r_acc = NULL_REG; /* Not claimed */

	return s->r_acc;
}

} // End of namespace Sci
