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
#include "glk/frotz/frotz.h"
#include "glk/conf.h"
#include "glk/events.h"

namespace Glk {
namespace Frotz {

void Processor::screen_mssg_on() {
	if (gos_curwin == _wp._lower) {
		oldstyle = curstyle;
		glk_set_style(style_Preformatted);
		glk_put_string("\n    ");
	}
}

void Processor::screen_mssg_off() {
	if (gos_curwin == _wp._lower) {
		glk_put_char('\n');
		zargs[0] = 0;
		z_set_text_style();
		zargs[0] = oldstyle;
		z_set_text_style();
	}
}

static const uint32 zchar_runes[] = {
	// This mapping is based on the Amiga font in the Z-Machine
	// specification, with some liberties taken.

	0x16AA, // RUNIC LETTER AC A
	0x16D2, // RUNIC LETTER BERKANAN BEORC BJARKAN B
	0x16C7, // RUNIC LETTER IWAZ EOH
	0x16D1, // RUNIC LETTER DAGAZ DAEG D
	0x16D6, // RUNIC LETTER EHWAZ EH E
	0x16A0, // RUNIC LETTER FEHU FEOH FE F
	0x16B7, // RUNIC LETTER GEBO GYFU G
	0x16BB, // RUNIC LETTER HAEGL H
	0x16C1, // RUNIC LETTER ISAZ IS ISS I
	0x16C4, // RUNIC LETTER GER
	0x16E6, // RUNIC LETTER LONG-BRANCH-YR
	0x16DA, // RUNIC LETTER LAUKAZ LAGU LOGR L
	0x16D7, // RUNIC LETTER MANNAZ MAN M
	0x16BE, // RUNIC LETTER NAUDIZ NYD NAUD N
	0x16A9, // RUNIC LETTER OS O
	0x16C8, // RUNIC LETTER PERTHO PEORTH P
	0x16B3, // RUNIC LETTER CEN
	0x16B1, // RUNIC LETTER RAIDO RAD REID R
	0x16CB, // RUNIC LETTER SIGEL LONG-BRANCH-SOL S
	0x16CF, // RUNIC LETTER TIWAZ TIR TYR T
	0x16A2, // RUNIC LETTER URUZ UR U
	0x16E0, // RUNIC LETTER EAR
	0x16B9, // RUNIC LETTER WUNJO WYNN W
	0x16C9, // RUNIC LETTER ALGIZ EOLHX
	0x16A5, // RUNIC LETTER W
	0x16DF  // RUNIC LETTER OTHALAN ETHEL O
};

uint32 Processor::zchar_to_unicode_rune(zchar c) {
	// There are only runic characters for a-z. Some versions of Beyond
 	// Zork will render the conversation between Prince Foo and the black
	// rider in runic script, even though it contained upper case letters.
	// This produced an ugly mix of runes and map-drawing characters, etc.
	// which is probably why it was removed in later versions.
	//
	// Still, it's probably a good idea to convert the upper case letters
	// to lower case to get an appropriate rune. As far as I can tell, the
	// upper case letters are all used for drawing maps and progress bars.
	// I don't think they're ever intended for the lower window.
	//
	// Apart from the runes, the arrow glyphs could perhaps also be
	// sensibly converted to Unicode?
	if (c >= 'a' && c <= 'z')
		return zchar_runes[c - 'a'];
	else if (c >= 'A' && c <= 'Z')
		return zchar_runes[c - 'A'];
	else
		return 0;
}

void Processor::screen_char(zchar c) {
	if (gos_linepending && (gos_curwin == gos_linewin)) {
		gos_cancel_pending_line();
		if (gos_curwin == _wp._upper) {
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

	if (_wp._upper && gos_curwin == _wp._upper) {
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
	} else if (gos_curwin == _wp._lower) {
		if (c == ZC_RETURN)
			glk_put_char('\n');
		else {
			if (curr_font == GRAPHICS_FONT) {
				uint32 runic_char = zchar_to_unicode_rune(c);
				if (runic_char != 0) {
					glk_set_style(style_User2);
					glk_put_char_uni(runic_char);
					glk_set_style(style_User1);
				} else
					glk_put_char_uni(c);
			} else
				glk_put_char_uni(c);
		}
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

void Processor::erase_screen(zword win) {
	int curr_fg = _wp[1][TRUE_FG_COLOR];
	int curr_bg = _wp[1][TRUE_BG_COLOR];

	if ((short)win == -1) {
		if (_wp._upper) {
			glk_set_window(_wp._upper);
#ifdef GARGLK
			garglk_set_zcolors(curr_fg, curr_bg);
#endif /* GARGLK */
			glk_window_clear(_wp._upper);
		}

		glk_window_clear(_wp._lower);
		split_window(0);
		glk_set_window(_wp._lower);
		gos_curwin = _wp._lower;
	}
}

void Processor::erase_window(zword win) {
	if (h_version == V6 && win != cwin && h_interpreter_number != INTERP_AMIGA)
		garglk_set_zcolors(_wp[win][TRUE_FG_COLOR], _wp[win][TRUE_BG_COLOR]);

	if (_wp[win])
		glk_window_clear(_wp[win]);

	if (h_version == V6 && win != cwin && h_interpreter_number != INTERP_AMIGA)
		garglk_set_zcolors(_wp[cwin][TRUE_FG_COLOR], _wp[cwin][TRUE_BG_COLOR]);
}

void Processor::z_buffer_mode() {
	// No implementation
}

void Processor::z_buffer_screen() {
	store(0);
}

void Processor::z_erase_line() {
	int i;

	if (_wp._upper && gos_curwin == _wp._upper) {
		for (i = 0; i < h_screen_cols + 1 - curx; i++)
			glk_put_char(' ');
		glk_window_move_cursor(gos_curwin, curx - 1, cury - 1);
	}
}

void Processor::z_erase_window() {
	short w = (short)zargs[0];
	
	flush_buffer();

	if (w == -1 || w == -2)
		erase_screen(w);
	else
		erase_window(winarg0());
}

void Processor::z_get_cursor() {
	storew((zword)(zargs[0] + 0), cury);
	storew((zword)(zargs[0] + 2), curx);
}

void Processor::z_print_table() {
	zword addr = zargs[0];
	zword xs = curx;
	int i, j;
	zbyte c;

	// Supply default arguments
	if (zargc < 3)
		zargs[2] = 1;
	if (zargc < 4)
		zargs[3] = 0;

	// Write text in width x height rectangle
	for (i = 0; i < zargs[2]; i++, curx = xs, cury++) {
		glk_window_move_cursor(_wp[cwin], xs - 1, cury - 1);

		for (j = 0; j < zargs[1]; j++) {
			LOW_BYTE(addr, c);
			addr++;

			print_char(c);
		}

		addr += zargs[3];
	}
}

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
}

void Processor::z_set_colour() {
	int fg = (short)zargs[0];
	int bg = (short)zargs[1];
	zword win = (h_version == V6) ? winarg2() : 0;

	if (win == 1 && h_version == V6)
		bg = zcolor_Transparent;

	flush_buffer();

	if (fg == -1)
		// Get color at cursor
		fg = os_peek_color();
	if (bg == -1)
		bg = zcolor_Transparent;

	if (fg == 0)
		// keep current colour
		fg = _wp[win][TRUE_FG_COLOR];
	if (bg == 0)
		bg = _wp[win][TRUE_BG_COLOR];

	if (fg == 1)
		fg = h_default_foreground;
	if (bg == 1)
		bg = h_default_background;

	if (fg >= 0 && fg < zcolor_NUMCOLORS)
		fg = zcolors[fg];
	if (bg >= 0 && bg < zcolor_NUMCOLORS)
		bg = zcolors[bg];

	if (h_version == V6 && h_interpreter_number == INTERP_AMIGA) {
		// Changing colours of window 0 affects the entire screen
		if (win == 0) {
			for (int i = 1; i < 8; ++i) {
				int bg2 = _wp[i][TRUE_BG_COLOR];
				int fg2 = _wp[i][TRUE_FG_COLOR];

				if (bg2 < 16)
					bg2 = (bg2 == (int)_wp[0][TRUE_BG_COLOR]) ? fg : bg;
				if (fg2 < 16)
					fg2 = (fg2 == (int)_wp[0][TRUE_FG_COLOR]) ? fg : bg;

				_wp[i][TRUE_FG_COLOR] = fg2;
				_wp[i][TRUE_BG_COLOR] = bg2;
			}
		}
	}

	_wp[win][TRUE_FG_COLOR] = fg;
	_wp[win][TRUE_BG_COLOR] = bg;

	if (win == cwin || h_version != V6)
		garglk_set_zcolors(fg, bg);
}

void Processor::z_set_font() {
	zword font = zargs[0];

	switch (font) {
	case PREVIOUS_FONT:
		// previous font
		temp_font = curr_font;
		curr_font = prev_font;
		prev_font = temp_font;
		zargs[0] = 0xf000;	// tickle tickle!
		z_set_text_style();
		store(curr_font);
		break;

	case TEXT_FONT:
	case GRAPHICS_FONT:
	case FIXED_WIDTH_FONT:
		prev_font = curr_font;
		curr_font = font;
		zargs[0] = 0xf000;	// tickle tickle!
		z_set_text_style();
		store(prev_font);
		break;

	case PICTURE_FONT: // picture font, undefined per 1.1
	default:           // unavailable
		store(0);
		break;
	}

	PropFontInfo &pi = g_conf->_propInfo;
	if (curr_font == GRAPHICS_FONT) {
		_quotes = pi._quotes;
		_dashes = pi._dashes;
		_spaces = pi._spaces;
		pi._quotes = 0;
		pi._dashes = 0;
		pi._spaces = 0;
	} else {
		pi._quotes = _quotes;
		pi._dashes = _dashes;
		pi._spaces = _spaces;
	}
}

void Processor::z_set_cursor() {
	int x = (int16)zargs[1], y = (int16)zargs[0];
	assert(_wp._upper);

	flush_buffer();

	if (y < 0) {
		// Cursor on/off
		if (y == -2)
			g_vm->_events->showMouseCursor(true);
		else if (y == -1)
			g_vm->_events->showMouseCursor(false);
		return;
	}

	if (!x || !y) {
		winid_t win = _wp._upper;
		Point cursorPos = win->getCursor();
		if (!x)
			x = cursorPos.x;
		if (!y)
			y = cursorPos.y;
	}

	curx = x;
	cury = y;

	if (cury > mach_status_ht) {
		mach_status_ht = cury;
		reset_status_ht();
	}

	glk_window_move_cursor(_wp._upper, curx - 1, cury - 1);
}

void Processor::z_set_text_style() {
	int style;

	if (zargs[0] == 0)
		curstyle = 0;
	else if (zargs[0] != 0xf000)
		// not tickle time
		curstyle |= zargs[0];

	if (h_flags & FIXED_FONT_FLAG || curr_font == FIXED_WIDTH_FONT || curr_font == GRAPHICS_FONT)
		style = curstyle | FIXED_WIDTH_STYLE;
	else
		style = curstyle;

	if (gos_linepending && gos_curwin == gos_linewin)
		return;

	if (style & REVERSE_STYLE) {
		os_set_reverse_video(true);
	}

	if (style & FIXED_WIDTH_STYLE) {
		if (curr_font == GRAPHICS_FONT)
			glk_set_style(style_User1);			// character graphics
		else if (style & BOLDFACE_STYLE && style & EMPHASIS_STYLE)
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
		os_set_reverse_video(false);
	}
}

void Processor::z_set_window() {
	cwin = zargs[0];

	if (cwin == 0) {
		glk_set_window(_wp._lower);
		gos_curwin = _wp._lower;
	} else {
		if (_wp._upper)
			glk_set_window(_wp._upper);
		gos_curwin = _wp._upper;
	}

	if (cwin == 0)
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

	if (!_wp._upper)
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
	glk_set_window(_wp._upper);
	gos_curwin = _wp._upper;

	os_set_reverse_video(true);

	curx = cury = 1;
	glk_window_move_cursor(_wp._upper, 0, 0);

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
	glk_set_window(_wp._lower);
	gos_curwin = _wp._lower;
}

void Processor::z_split_window() {
	split_window(zargs[0]);
}

} // End of namespace Frotz
} // End of namespace Glk
