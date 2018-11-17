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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/frotz/processor.h"

namespace Glk {
namespace Frotz {

void Processor::screen_mssg_on() {
	if (gos_curwin == gos_lower) {
		oldstyle = curstyle;
		glk_set_style(style_Preformatted);
		glk_put_string("\n    ");
	}
}

void Processor::screen_mssg_off() {
	if (gos_curwin == gos_lower) {
		glk_put_char('\n');
		zargs[0] = 0;
		z_set_text_style();
		zargs[0] = oldstyle;
		z_set_text_style();
	}
}

void Processor::screen_char(zchar c) {
	if (gos_linepending && (gos_curwin == gos_linewin)) {
		gos_cancel_pending_line();
		if (gos_curwin == gos_upper) {
			curx = 1;
			cury ++;
		}
		if (c == '\n')
			return;
	}

	// check fixed flag in header, game can change it at whim
	int forcefix = ((h_flags & FIXED_FONT_FLAG) != 0);
	int curfix = ((curstyle & FIXED_WIDTH_STYLE) != 0);
	if (forcefix && !curfix) {
		zargs[0] = 0xf000;	// tickle tickle!
		z_set_text_style();
		fixforced = true;
	} else if (!forcefix && fixforced) {
		zargs[0] = 0xf000;	// tickle tickle!
		z_set_text_style();
		fixforced = false;
	}

	if (gos_upper && gos_curwin == gos_upper) {
		if (c == '\n' || c == ZC_RETURN) {
			glk_put_char('\n');
			curx = 1;
			cury ++;
		} else {
			if (cury == 1) {
				if (curx <= (int)((sizeof statusline / sizeof(zchar)) - 1)) {
					statusline[curx - 1] = c;
					statusline[curx] = 0;
				}
				if (curx < h_screen_cols) {
					glk_put_char_uni(c);
				} else if (curx == h_screen_cols) {
					glk_put_char_uni(c);
					glk_window_move_cursor(gos_curwin, curx-1, cury-1);
				} else {
					smartstatusline();
				}

				curx++;
			} else {
				if (curx < h_screen_cols) {
					glk_put_char_uni(c);
				} else if (curx == (h_screen_cols)) {
					glk_put_char_uni(c);
					glk_window_move_cursor(gos_curwin, curx-1, cury-1);
				}

				curx++;
			}
		}
	} else if (gos_curwin == gos_lower) {
		if (c == ZC_RETURN)
			glk_put_char('\n');
		else glk_put_char_uni(c);
	}
}

void Processor::screen_new_line() {
	screen_char('\n');
}

void Processor::screen_word(const zchar *s) {
	zchar c;
	while ((c = *s++) != 0) {
		if (c == ZC_NEW_FONT)
			s++;
		else if (c == ZC_NEW_STYLE)
			s++;
		else
			screen_char(c);
	}
}


void Processor::z_buffer_mode() {
	// No implementation
}

void Processor::z_buffer_screen() {
	store(0);
}

void Processor::z_erase_line() {
	int i;

	if (gos_upper && gos_curwin == gos_upper) {
		for (i = 0; i < h_screen_cols + 1 - curx; i++)
			glk_put_char(' ');
		glk_window_move_cursor(gos_curwin, curx - 1, cury - 1);
	}
}

void Processor::z_erase_window() {
	short w = zargs[0];
	if (w == -2)
	{
		if (gos_upper) {
			glk_set_window(gos_upper);
#ifdef GARGLK
			garglk_set_zcolors(curr_fg, curr_bg);
#endif /* GARGLK */
			glk_window_clear(gos_upper);
			glk_set_window(gos_curwin);
		}
		glk_window_clear(gos_lower);
	}
	if (w == -1)
	{
		if (gos_upper) {
			glk_set_window(gos_upper);
#ifdef GARGLK
			garglk_set_zcolors(curr_fg, curr_bg);
#endif /* GARGLK */
			glk_window_clear(gos_upper);
		}
		glk_window_clear(gos_lower);
		split_window(0);
		glk_set_window(gos_lower);
		gos_curwin = gos_lower;
	}
	if (w == 0)
		glk_window_clear(gos_lower);
	if (w == 1 && gos_upper)
		glk_window_clear(gos_upper);
}

void Processor::z_get_cursor() {
	storew((zword) (zargs[0] + 0), cury);
	storew((zword) (zargs[0] + 2), curx);
}

void Processor::z_print_table() {
	zword addr = zargs[0];
	zword x;
	int i, j;

	// Supply default arguments
	if (zargc < 3)
		zargs[2] = 1;
	if (zargc < 4)
		zargs[3] = 0;

	// Write text in width x height rectangle
	x = curx;

	for (i = 0; i < zargs[2]; i++) {
		if (i != 0) {
			cury += 1;
			curx = x;
		}

		for (j = 0; j < zargs[1]; j++) {

			zbyte c;

			LOW_BYTE(addr, c);
			addr++;

			print_char(c);
		}

		addr += zargs[3];
	}
}

#define zB(i) ((((i >> 10) & 0x1F) << 3) | (((i >> 10) & 0x1F) >> 2))
#define zG(i) ((((i >>  5) & 0x1F) << 3) | (((i >>  5) & 0x1F) >> 2))
#define zR(i) ((((i      ) & 0x1F) << 3) | (((i      ) & 0x1F) >> 2))

#define zRGB(i) (zR(i) << 16 | zG(i) << 8 | zB(i))

void Processor::z_set_true_colour() {
	int zfore = zargs[0];
	int zback = zargs[1];

	if (!(zfore < 0))
		zfore = zRGB(zargs[0]);

	if (!(zback < 0))
		zback = zRGB(zargs[1]);

#ifdef GARGLK
	garglk_set_zcolors(zfore, zback);
#endif /* GARGLK */

	curr_fg = zfore;
	curr_bg = zback;
}

static const int zcolor_map[] = {
	-2,						///<  0 = current
	-1,						///<  1 = default
	0x0000,					///<  2 = black
	0x001D,					///<  3 = red
	0x0340,					///<  4 = green
	0x03BD,					///<  5 = yellow
	0x59A0,					///<  6 = blue
	0x7C1F,					///<  7 = magenta
	0x77A0,					///<  8 = cyan
	0x7FFF,					///<  9 = white
	0x5AD6,					///< 10 = light grey
	0x4631,					///< 11 = medium grey
	0x2D6B,					///< 12 = dark grey
};

#define zcolor_NUMCOLORS    (13)

void Processor::z_set_colour() {
	int zfore = zargs[0];
	int zback = zargs[1];

	switch (zfore) {
	case -1:
		zfore = -3;
		break;

	case 0:
	case 1:
		zfore = zcolor_map[zfore];
		break;

	default:
		if (zfore < zcolor_NUMCOLORS)
			zfore = zRGB(zcolor_map[zfore]);
		break;
	}

	switch (zback) {
	case -1:
		zback = -3;

	case 0:
	case 1:
		zback = zcolor_map[zback];
		break;

	default:
		if (zback < zcolor_NUMCOLORS)
			zback = zRGB(zcolor_map[zback]);
		break;
	}

#ifdef GARGLK
	garglk_set_zcolors(zfore, zback);
#endif /* GARGLK */

	curr_fg = zfore;
	curr_bg = zback;
}

void Processor::z_set_font() {
	zword font = zargs[0];

	switch (font) {
		case 0:
			// previous font
			temp_font = curr_font;
			curr_font = prev_font;
			prev_font = temp_font;
			zargs[0] = 0xf000;	// tickle tickle!
			z_set_text_style();
			store (curr_font);
			break;

		case 1: /* normal font */
			prev_font = curr_font;
			curr_font = 1;
			zargs[0] = 0xf000;	// tickle tickle!
			z_set_text_style();
			store (prev_font);
			break; 

		case 4: /* fixed-pitch font*/
			prev_font = curr_font;
			curr_font = 4;
			zargs[0] = 0xf000;	// tickle tickle!
			z_set_text_style();
			store (prev_font);
			break;

		case 2: // picture font, undefined per 1.1
		case 3: // character graphics font
		default: // unavailable
			store (0);
			break;
	}
}

void Processor::z_set_cursor() {
	cury = zargs[0];
	curx = zargs[1];

	if (gos_upper) {
		if (cury > mach_status_ht) {
			mach_status_ht = cury;
			reset_status_ht();
		}

		glk_window_move_cursor(gos_upper, curx - 1, cury - 1);
	}
}

void Processor::z_set_text_style() {
	int style;

	if (zargs[0] == 0)
		curstyle = 0;
	else if (zargs[0] != 0xf000) /* not tickle time */
		curstyle |= zargs[0];

	if (h_flags & FIXED_FONT_FLAG || curr_font == 4)
		style = curstyle | FIXED_WIDTH_STYLE;
	else
		style = curstyle;

	if (gos_linepending && gos_curwin == gos_linewin)
		return;

	if (style & REVERSE_STYLE) {
#ifdef GARGLK
		garglk_set_reversevideo(true);
#endif /* GARGLK */
	}

	if (style & FIXED_WIDTH_STYLE) {
		if (style & BOLDFACE_STYLE && style & EMPHASIS_STYLE)
			glk_set_style(style_BlockQuote);	// monoz
		else if (style & EMPHASIS_STYLE)
			glk_set_style(style_Alert);			// monoi
		else if (style & BOLDFACE_STYLE)
			glk_set_style(style_Subheader);		// monob
		else
			glk_set_style(style_Preformatted);	// monor
	} else {
		if (style & BOLDFACE_STYLE && style & EMPHASIS_STYLE)
			glk_set_style(style_Note);			// propz
		else if (style & EMPHASIS_STYLE)
			glk_set_style(style_Emphasized);	// propi
		else if (style & BOLDFACE_STYLE)
			glk_set_style(style_Header);		// propb
		else
			glk_set_style(style_Normal);		// propr
	}

	if (curstyle == 0) {
#ifdef GARGLK
		garglk_set_reversevideo(false);
#endif /* GARGLK */
	}
}

void Processor::z_set_window() {
	int win = zargs[0];

	if (win == 0) {
		glk_set_window(gos_lower);
		gos_curwin = gos_lower;
	} else {
		if (gos_upper)
			glk_set_window(gos_upper);
		gos_curwin = gos_upper;
	}

	if (win == 0)
		enable_scripting = true;
	else
		enable_scripting = false;

	zargs[0] = 0xf000;	// tickle tickle!
	z_set_text_style();
}

void Processor::pad_status_line(int column) {
	int spaces;
	spaces = (h_screen_cols + 1 - curx) - column;
	while (spaces-- > 0)
		print_char(' ');
}

void Processor::z_show_status() {
	zword global0;
	zword global1;
	zword global2;
	zword addr;

	bool brief = false;

	if (!gos_upper)
		return;

	// One V5 game (Wishbringer Solid Gold) contains this opcode by accident,
	// so just return if the version number does not fit
	if (h_version >= V4)
		return;

	// Read all relevant global variables from the memory of the Z-machine
	// into local variables

	addr = h_globals;
	LOW_WORD(addr, global0);
	addr += 2;
	LOW_WORD(addr, global1);
	addr += 2;
	LOW_WORD(addr, global2);

	// Move to top of the status window, and print in reverse style.
	glk_set_window(gos_upper);
	gos_curwin = gos_upper;

#ifdef GARGLK
	garglk_set_reversevideo(true);
#endif /* GARGLK */

	curx = cury = 1;
	glk_window_move_cursor(gos_upper, 0, 0);

	// If the screen width is below 55 characters then we have to use
	// the brief status line format
	if (h_screen_cols < 55)
		brief = true;

	// Print the object description for the global variable 0
	print_char (' ');
	print_object (global0);

	// A header flag tells us whether we have to display the current
	// time or the score/moves information
	if (h_config & CONFIG_TIME) {
		// print hours and minutes
		zword hours = (global1 + 11) % 12 + 1;

		pad_status_line (brief ? 15 : 20);

		print_string ("Time: ");

		if (hours < 10)
			print_char (' ');
		print_num (hours);

		print_char (':');

		if (global2 < 10)
			print_char ('0');
		print_num (global2);

		print_char (' ');

		print_char ((global1 >= 12) ? 'p' : 'a');
		print_char ('m');

	} else {
		// print score and moves
		pad_status_line (brief ? 15 : 30);

		print_string (brief ? "S: " : "Score: ");
		print_num (global1);

		pad_status_line (brief ? 8 : 14);

		print_string (brief ? "M: " : "Moves: ");
		print_num (global2);
	}

	// Pad the end of the status line with spaces
	pad_status_line (0);

	// Return to the lower window
	glk_set_window(gos_lower);
	gos_curwin = gos_lower;
}

void Processor::z_split_window() {
	split_window(zargs[0]);
}

} // End of namespace Scott
} // End of namespace Glk
