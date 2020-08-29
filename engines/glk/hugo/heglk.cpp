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

#include "glk/hugo/hugo.h"

namespace Glk {
namespace Hugo {

void Hugo::hugo_init_screen() {
	// Open the main window...
	mainwin = currentwin = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	assert(mainwin);

	// ...and set it up for default output
	glk_set_window(mainwin);

	// By setting the width and height so high, we're basically forcing the Glk library
	// to deal with text-wrapping and page ends
	SCREENWIDTH = 0x7fff;
	SCREENHEIGHT = 0x7fff;
	FIXEDCHARWIDTH = 1;
	FIXEDLINEHEIGHT = 1;

	hugo_settextwindow(1, 1,
		SCREENWIDTH/FIXEDCHARWIDTH, SCREENHEIGHT/FIXEDLINEHEIGHT);
}

void Hugo::hugo_getline(const char *prmpt) {
	event_t ev;
	char gotline = 0;

	/* Just in case we try to get line input from a Glk-illegal
	window that hasn't been created, switch as a failsafe
	to mainwin
	*/
	if (currentwin == nullptr)
		glk_set_window(currentwin = mainwin);

	/* Print prompt */
	glk_put_string(prmpt);

	/* Request line input */
	glk_request_line_event(currentwin, buffer, MAXBUFFER, 0);

	while (!gotline) {
		if (shouldQuit())
			return;

		/* Grab an event */
		glk_select(&ev);

		switch (ev.type)
		{
		case evtype_LineInput:
			/* (Will always be currentwin, but anyway) */
			if (ev.window == currentwin) {
				gotline = true;
			}
			break;
		default:
			break;
		}
	}

	/* The line we have received in commandbuf is not null-terminated */
	buffer[ev.val1] = '\0';	/* i.e., the length */

							/* Copy the input to the script file (if open) */
	if (script) {
		Common::String text = Common::String::format("%s%s\n", prmpt, buffer);
		script->putBuffer(text.c_str(), text.size());
	}
}

int Hugo::hugo_waitforkey() {
	event_t ev;
	char gotchar = 0;

	/* Just in case we try to get key input from a Glk-illegal
	window that hasn't been created, switch as a failsafe
	to mainwin
	*/
	if (currentwin == nullptr)
		glk_set_window(currentwin = mainwin);

#if defined (NO_KEYPRESS_CURSOR)
	if (currentwin != mainwin)
	{
		glk_window_move_cursor(currentwin, currentpos / CHARWIDTH, currentline - 1);
		hugo_print("*");
		glk_window_move_cursor(currentwin, currentpos / CHARWIDTH, currentline - 1);
	}
#endif

	glk_request_char_event(currentwin);

	while (!gotchar)
	{
		/* Grab an event */
		glk_select(&ev);

		switch (ev.type) {
		case evtype_CharInput:
			/* (Will always be mainwin, but anyway) */
			if (ev.window == currentwin) {
				gotchar = true;
			}
			break;
		case evtype_Quit:
			return 0;
		default:
			break;
		}
	}

	/* Convert Glk special keycodes: */
	switch (ev.val1)
	{
	case keycode_Left:	ev.val1 = 8;	break;
	case keycode_Right:	ev.val1 = 21;	break;
	case keycode_Up:	ev.val1 = 11;	break;
	case keycode_Down:	ev.val1 = 10;	break;
	case keycode_Return:	ev.val1 = 13;	break;
	case keycode_Escape:	ev.val1 = 27;	break;
	}

#if defined (NO_KEYPRESS_CURSOR)
	if (currentwin != mainwin)
	{
		glk_window_move_cursor(currentwin, currentpos / CHARWIDTH, currentline - 1);
		hugo_print(" ");
		glk_window_move_cursor(currentwin, currentpos / CHARWIDTH, currentline - 1);
	}
#endif

	return ev.val1;
}

int Hugo::hugo_iskeywaiting() {
	var[system_status] = STAT_UNAVAILABLE;
	return 0;
}

int Hugo::hugo_timewait(int n) {
	uint32 millisecs;
	event_t ev;

	if (!glk_gestalt(gestalt_Timer, 0))
		return false;
	if (n == 0) return true;


	millisecs = 1000 / n;
	if (millisecs == 0)
		millisecs = 1;

	// For the time being, we're going to disallow
	// millisecond delays in Glk (1) because there's no
	// point, and (2) so that we can tell we're running
	// under Glk.
	if (millisecs < 1000) return false;

	glk_request_timer_events(millisecs);
	while (1)
	{
		glk_select(&ev);
		if (ev.type == evtype_Timer)
			break;
	}
	glk_request_timer_events(0);
	return true;
}

void Hugo::hugo_clearfullscreen() {
	glk_window_clear(mainwin);
	if (secondwin) glk_window_clear(secondwin);
	if (auxwin) glk_window_clear(auxwin);

	/* See hugo_print() for the need for this */
	if (currentwin == mainwin) mainwin_bgcolor = glk_bgcolor;

	/* Must be set: */
	currentpos = 0;
	currentline = 1;

	if (!inwindow) just_cleared_screen = true;
}

void Hugo::hugo_clearwindow() {
	/* Clears the currently defined window, moving the cursor to the top-left
	corner of the window */

	/* If the engine thinks we're in a window, but Glk was
	unable to comply, don't clear the window, because it's
	not really a window
	*/
	if (inwindow && currentwin == mainwin) return;
	if (currentwin == nullptr) return;

	glk_window_clear(currentwin);

	/* See hugo_print() for the need for this */
	if (currentwin == mainwin) mainwin_bgcolor = glk_bgcolor;

	/* If we're in a fixed-font (i.e., textgrid) auxiliary
	window when we call for a clear, close auxwin and reset
	the current window to mainwin
	*/
	if (auxwin)
	{
		stream_result_t sr;

		glk_window_close(auxwin, &sr);
		auxwin = nullptr;
		glk_set_window(currentwin = mainwin);
	}

	/* Must be set: */
	currentpos = 0;
	currentline = 1;

	if (!inwindow) just_cleared_screen = true;
}

void Hugo::hugo_settextmode() {
	charwidth = FIXEDCHARWIDTH;
	lineheight = FIXEDLINEHEIGHT;
}

void Hugo::hugo_settextwindow(int left, int top, int right, int bottom) {
	/* Hugo's arbitrarily positioned windows don't currently
	mesh with what Glk has to offer, so we have to ignore any
	non-Glk-ish Windows and just maintain the current
	parameters
	*/
	if ((top != 1 || bottom >= physical_windowbottom / FIXEDLINEHEIGHT + 1)
		/* Pre-v2.4 didn't support proper windowing */
		&& (game_version >= 24 || !inwindow))
	{
		in_valid_window = false;

		/* Glk-illegal floating window; setting currentwin
		to nullptr will tell hugo_print() not to print in it:
		*/
		if (bottom<physical_windowbottom / FIXEDLINEHEIGHT + 1)
		{
			currentwin = nullptr;
			glk_set_window(mainwin);
			return;
		}
		else
			glk_set_window(currentwin = mainwin);
	}

	/* Otherwise this is a valid window (positioned along the
	top of the screen a la a status window), so... */
	else
	{
		/* Arbitrary height of 4 lines for pre-v2.4 windows */
		if (game_version < 24) bottom = 4;

		/* ...either create a new window if none exists... */
		if (!secondwin)
		{
			glk_stylehint_set(wintype_TextGrid, style_Normal, stylehint_ReverseColor, 1);
			glk_stylehint_set(wintype_TextGrid, style_Subheader, stylehint_ReverseColor, 1);
			glk_stylehint_set(wintype_TextGrid, style_Emphasized, stylehint_ReverseColor, 1);

			//winid_t p = glk_window_get_parent(mainwin);
			secondwin = glk_window_open(mainwin,//p,
				winmethod_Above | winmethod_Fixed,
				bottom,
				wintype_TextGrid,
				0);
		}

		/* ...or resize the existing one if necessary */
		else if (bottom != secondwin_bottom)
		{
			winid_t p;

			p = glk_window_get_parent(secondwin);
			glk_window_set_arrangement(p,
				winmethod_Above | winmethod_Fixed,
				bottom,
				secondwin);
		}

		if (secondwin)
		{
			if (game_version < 24)
				glk_window_clear(secondwin);

			glk_set_window(currentwin = secondwin);
			in_valid_window = true;
			secondwin_bottom = bottom;
		}
		else
		{
			currentwin = nullptr;
			glk_set_window(mainwin);
			secondwin_bottom = 0;
			return;
		}
	}

	physical_windowleft = (left - 1)*FIXEDCHARWIDTH;
	physical_windowtop = (top - 1)*FIXEDLINEHEIGHT;
	physical_windowright = right*FIXEDCHARWIDTH - 1;
	physical_windowbottom = bottom*FIXEDLINEHEIGHT - 1;
	physical_windowwidth = (right - left + 1)*FIXEDCHARWIDTH;
	physical_windowheight = (bottom - top + 1)*FIXEDLINEHEIGHT;
}

int Hugo::heglk_get_linelength() {
	static uint width;

	// Try to use whatever fixed-width linelength is available
	if (secondwin)
		glk_window_get_size(secondwin, &width, nullptr);
	else if (auxwin)
		glk_window_get_size(auxwin, &width, nullptr);

	// Otherwise try to approximate it by the proportionally spaced linelength
	else
		glk_window_get_size(mainwin, &width, nullptr);

	// -1 to override automatic line wrapping
	return width - 1;
}

int Hugo::heglk_get_screenheight() {
	static uint height = 0, mainheight = 0;

	if (secondwin)
		glk_window_get_size(secondwin, nullptr, &height);
	else if (auxwin)
		glk_window_get_size(auxwin, nullptr, &height);

	glk_window_get_size(mainwin, nullptr, &mainheight);

	return height + mainheight;
}

void Hugo::hugo_settextpos(int x, int y) {
	if (currentwin == nullptr) return;

	// Try to determine if we're trying to position fixed-width text in the main window,
	// as in a menu, for example
	if (!just_cleared_screen && !inwindow &&
		!(glk_current_font & PROP_FONT)
		&& y != 1			/* not just cls */
		&& y < SCREENHEIGHT - 0x0f)	/* 0x0f is arbitrary */
	{
		/* See if we're already in the auxiliary window */
		if (currentwin != auxwin)
		{
			/* If not, create it, making it 100% of
			mainwin's height
			*/
			if (auxwin == nullptr)
			{
				auxwin = glk_window_open(mainwin,
					winmethod_Below | winmethod_Proportional,
					100,
					wintype_TextGrid,
					0);
			}
			else
				glk_window_clear(auxwin);

			glk_set_window(currentwin = auxwin);
		}
	}

	/* On the other hand, if we were in a textgrid window and
	no longer need to be, get out
	*/
	else if (auxwin)
	{
		stream_result_t sr;

		/* Close auxwin */
		glk_window_close(auxwin, &sr);
		auxwin = nullptr;

		/* Clear the screen (both windows) */
		glk_window_clear(mainwin);
		glk_window_clear(secondwin);

		glk_set_window(currentwin = mainwin);
	}

	just_cleared_screen = false;

	/* Can only move the Glk cursor in a textgrid window */
	if (currentwin != mainwin)
		glk_window_move_cursor(currentwin, x - 1, y - 1);

	/* Must be set: */
	currentline = y;
	currentpos = (x - 1)*CHARWIDTH;   /* Note:  zero-based */
}

void Hugo::hugo_print(const char *a) {
	static char just_printed_linefeed = false;
	/* static already_modified_style = false; */

	/* Can't print in a Glk-illegal window since it hasn't been
	created
	*/
	if (currentwin == nullptr) return;

	/* In lieu of colors, in case we're highlighting something
	such as a menu selection:
	*/
	/*
	if (!inwindow and glk_bgcolor!=mainwin_bgcolor)
	{
	if (!already_modified_style)
	{
	if (glk_current_font & BOLD_FONT)
	glk_set_style(style_Normal);
	else
	glk_set_style(style_Emphasized);
	}
	already_modified_style = true;
	}
	else
	already_modified_style = false;
	*/

	if (a[0] == '\n')
	{
		if (!just_printed_linefeed)
		{
			glk_put_string("\n");
		}
		else
			just_printed_linefeed = false;
	}
	else if (a[0] == '\r')
	{
		if (!just_printed_linefeed)
		{
			glk_put_string("\n");
			just_printed_linefeed = true;
		}
		else
			just_printed_linefeed = false;
	}
	else
	{
		glk_put_string(a);
		just_printed_linefeed = false;
	}
}

void Hugo::hugo_font(int f) {
	static char using_prop_font = false;

	glk_current_font = f;

	glk_set_style(style_Normal);

	if (f & BOLD_FONT)
		glk_set_style(style_Subheader);

	if (f & UNDERLINE_FONT)
		glk_set_style(style_Emphasized);

	if (f & ITALIC_FONT)
		glk_set_style(style_Emphasized);

	if (f & PROP_FONT)
		using_prop_font = true;

	/* Have to comment this out, it seems, because it will mess up the
	alignment of the input in the main window
	if (!(f & PROP_FONT))
	glk_set_style(style_Preformatted);
	*/

	/* Workaround to decide if we have to open auxwin for positioned
	non-proportional text:
	*/
	if (!(f & PROP_FONT))
	{
		/* If at top of screen, and changing to a fixed-
		width font (a situation which wouldn't normally
		be adjusted for by hugo_settextpos())
		*/
		if (!inwindow && currentline == 1 && currentpos == 0 && using_prop_font)
		{
			just_cleared_screen = false;
			hugo_settextpos(1, 2);
			glk_window_move_cursor(currentwin, 0, 0);
		}
	}
}

void Hugo::hugo_settextcolor(int c) {
	// Set the foreground color to hugo_color(c)
	glk_fcolor = hugo_color(c);
}

void Hugo::hugo_setbackcolor(int c) {
	// Set the background color to hugo_color(c)
	glk_bgcolor = hugo_color(c);
}

int Hugo::hugo_color(int c) {
	if (c == 16)      c = DEF_FCOLOR;
	else if (c == 17) c = DEF_BGCOLOR;
	else if (c == 18) c = DEF_SLFCOLOR;
	else if (c == 19) c = DEF_SLBGCOLOR;
	else if (c == 20) c = hugo_color(fcolor);	/* match foreground */

												/* Uncomment this block of code and change "c = ..." values if the system
												palette differs from the Hugo palette.

												If colors are unavailable on the system in question, it may suffice
												to have black, white, and brightwhite (i.e. boldface).  It is expected
												that colored text will be visible on any other-colored background.

												switch (c)
												{
												case HUGO_BLACK:	 c = 0;  break;
												case HUGO_BLUE:		 c = 1;  break;
												case HUGO_GREEN:	 c = 2;  break;
												case HUGO_CYAN:		 c = 3;  break;
												case HUGO_RED:		 c = 4;  break;
												case HUGO_MAGENTA:	 c = 5;  break;
												case HUGO_BROWN:	 c = 6;  break;
												case HUGO_WHITE:	 c = 7;  break;
												case HUGO_DARK_GRAY:	 c = 8;  break;
												case HUGO_LIGHT_BLUE:	 c = 9;  break;
												case HUGO_LIGHT_GREEN:	 c = 10; break;
												case HUGO_LIGHT_CYAN:	 c = 11; break;
												case HUGO_LIGHT_RED:	 c = 12; break;
												case HUGO_LIGHT_MAGENTA: c = 13; break;
												case HUGO_YELLOW:	 c = 14; break;
												case HUGO_BRIGHT_WHITE:	 c = 15; break;
												*/
	return c;
}

int Hugo::hugo_charwidth(char a) const {
	if (a == FORCED_SPACE)
		return CHARWIDTH;         /* same as ' ' */

	else if ((unsigned char)a >= ' ') /* alphanumeric characters */

		return CHARWIDTH;         /* for non-proportional */

	return 0;
}

int Hugo::hugo_textwidth(const char *a) const {
	int i, slen, len = 0;

	slen = (int)strlen(a);

	for (i = 0; i<slen; i++)
	{
		if (a[i] == COLOR_CHANGE) i += 2;
		else if (a[i] == FONT_CHANGE) i++;
		else
			len += hugo_charwidth(a[i]);
	}

	return len;
}

int Hugo::hugo_strlen(const char *a) const {
	int i, slen, len = 0;

	slen = (int)strlen(a);

	for (i = 0; i<slen; i++)
	{
		if (a[i] == COLOR_CHANGE) i += 2;
		else if (a[i] == FONT_CHANGE) i++;
		else len++;
	}

	return len;
}

/*
 * Replacements for things the Glk port doesn't support:
 *
 */

void Hugo::hugo_setgametitle(const char *t) {}

int Hugo::hugo_hasvideo() const { return false; }

int Hugo::hugo_playvideo(HUGO_FILE infile, long reslength, char loop_flag, char background, int volume) {
	delete infile;
	return true;
}

void Hugo::hugo_stopvideo(void) {}

} // End of namespace Hugo
} // End of namespace Glk
