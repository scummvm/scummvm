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

#include "glk/frotz/glk_interface.h"
#include "glk/frotz/pics.h"
#include "glk/frotz/sound_folder.h"
#include "glk/conf.h"
#include "glk/screen.h"
#include "common/config-manager.h"
#include "common/unzip.h"

namespace Glk {
namespace Frotz {

GlkInterface::GlkInterface(OSystem *syst, const GlkGameDescription &gameDesc) :
		GlkAPI(syst, gameDesc),
		_pics(nullptr), oldstyle(0), curstyle(0), cury(1), curx(1), fixforced(0), curr_fg(-2), curr_bg(-2),
		curr_font(1), prev_font(1), temp_font(0), curr_status_ht(0), mach_status_ht(0), gos_status(nullptr),
		gos_curwin(nullptr), gos_linepending(0), gos_linebuf(nullptr), gos_linewin(nullptr),
		gos_channel(nullptr), cwin(0), mwin(0), mouse_x(0), mouse_y(0), menu_selected(0), enable_wrapping(false),
		enable_scripting(false), enable_scrolling(false), enable_buffering(false), next_sample(0), next_volume(0),
		_soundLocked(false), _soundPlaying(false) {
	Common::fill(&statusline[0], &statusline[256], '\0');
}

GlkInterface::~GlkInterface() {
	delete _pics;
}

void GlkInterface::initialize() {
	uint width, height;

	/*
	 * Init glk stuff
	 */

	 // monor
	glk_stylehint_set(wintype_AllTypes, style_Preformatted, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Preformatted, stylehint_Weight, 0);
	glk_stylehint_set(wintype_AllTypes, style_Preformatted, stylehint_Oblique, 0);

	// monob
	glk_stylehint_set(wintype_AllTypes, style_Subheader, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Subheader, stylehint_Weight, 1);
	glk_stylehint_set(wintype_AllTypes, style_Subheader, stylehint_Oblique, 0);

	// monoi
	glk_stylehint_set(wintype_AllTypes, style_Alert, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Alert, stylehint_Weight, 0);
	glk_stylehint_set(wintype_AllTypes, style_Alert, stylehint_Oblique, 1);

	// monoz
	glk_stylehint_set(wintype_AllTypes, style_BlockQuote, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_BlockQuote, stylehint_Weight, 1);
	glk_stylehint_set(wintype_AllTypes, style_BlockQuote, stylehint_Oblique, 1);

	// propr
	glk_stylehint_set(wintype_TextBuffer, style_Normal, stylehint_Proportional, 1);
	glk_stylehint_set(wintype_TextGrid, style_Normal, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Normal, stylehint_Weight, 0);
	glk_stylehint_set(wintype_AllTypes, style_Normal, stylehint_Oblique, 0);

	// propb
	glk_stylehint_set(wintype_TextBuffer, style_Header, stylehint_Proportional, 1);
	glk_stylehint_set(wintype_TextGrid, style_Header, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Header, stylehint_Weight, 1);
	glk_stylehint_set(wintype_AllTypes, style_Header, stylehint_Oblique, 0);

	// propi
	glk_stylehint_set(wintype_TextBuffer, style_Emphasized, stylehint_Proportional, 1);
	glk_stylehint_set(wintype_TextGrid, style_Emphasized, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Emphasized, stylehint_Weight, 0);
	glk_stylehint_set(wintype_AllTypes, style_Emphasized, stylehint_Oblique, 1);

	// propi
	glk_stylehint_set(wintype_TextBuffer, style_Note, stylehint_Proportional, 1);
	glk_stylehint_set(wintype_TextGrid, style_Note, stylehint_Proportional, 0);
	glk_stylehint_set(wintype_AllTypes, style_Note, stylehint_Weight, 1);
	glk_stylehint_set(wintype_AllTypes, style_Note, stylehint_Oblique, 1);

	/*
	 * Get the screen size
	 */

	_wp._lower = glk_window_open(0, 0, 0, wintype_TextGrid, 0);
	if (!_wp._lower)
		_wp._lower = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	glk_window_get_size(_wp._lower, &width, &height);
	glk_window_close(_wp._lower, nullptr);

	gos_channel = nullptr;

	h_screen_cols = width;
	h_screen_rows = height;

	h_screen_height = h_screen_rows;
	h_screen_width = h_screen_cols;

	h_font_width = 1;
	h_font_height = 1;

	// Must be after screen dimensions are computed
	if (g_conf->_graphics) {
		if (_blorb)
			// Blorb file containers allow graphics
			h_flags |= GRAPHICS_FLAG;
		else if ((h_version == V6 || _storyId == BEYOND_ZORK) && initPictures())
			// Earlier Infocom game with picture files
			h_flags |= GRAPHICS_FLAG;
	}

	// Use the ms-dos interpreter number for v6, because that's the
	// kind of graphics files we understand
	h_interpreter_number = h_version == 6 ? INTERP_MSDOS : INTERP_AMIGA;
	h_interpreter_version = 'F';

	// Set these per spec 8.3.2.
	h_default_foreground = WHITE_COLOUR;
	h_default_background = BLACK_COLOUR;
	if (h_flags & COLOUR_FLAG)
		h_flags &= ~COLOUR_FLAG;

	/*
	 * Open the windows
	 */
	if (_storyId == BEYOND_ZORK)
		showBeyondZorkTitle();

	_wp.setup(h_version == 6);
	cwin = 0;
	gos_curwin = _wp._lower;

	/*
	 * Icky magic bit setting
	 */

	if (h_version == V3 && _tandyBit)
		h_config |= CONFIG_TANDY;

	if (h_version == V3 && _wp._upper)
		h_config |= CONFIG_SPLITSCREEN;

	if (h_version == V3 && !_wp._upper)
		h_config |= CONFIG_NOSTATUSLINE;

	if (h_version >= V4)
		h_config |= CONFIG_BOLDFACE | CONFIG_EMPHASIS |
		CONFIG_FIXED | CONFIG_TIMEDINPUT | CONFIG_COLOUR;

	if (h_version >= V5)
		h_flags &= ~(GRAPHICS_FLAG | MOUSE_FLAG | MENU_FLAG);

	if ((h_version >= 5) && (h_flags & SOUND_FLAG))
		h_flags |= SOUND_FLAG;

	if ((h_version == 3) && (h_flags & OLD_SOUND_FLAG))
		h_flags |= OLD_SOUND_FLAG;

	if ((h_version == 6) && (_sound != 0))
		h_config |= CONFIG_SOUND;

	if (h_version >= V5 && (h_flags & UNDO_FLAG))
		if (_undo_slots == 0)
			h_flags &= ~UNDO_FLAG;

	/*
	 * Miscellaneous
	 */

	// Set the screen colors
	garglk_set_zcolors(_defaultForeground, _defaultBackground);

	// Add any sound folder or zip
	addSound();

	// For Beyond Zork the Page Up/Down keys are remapped to scroll the description area,
	// since the arrow keys the original used are in use now for cycling prior commands
	if (_storyId == BEYOND_ZORK) {
		uint32 KEYCODES[2] = { keycode_PageUp, keycode_PageDown };
		glk_set_terminators_line_event(_wp._lower, KEYCODES, 2);
	}
}

void GlkInterface::addSound() {
	Common::FSNode gameDir(ConfMan.get("path"));
	SoundSubfolder::check(gameDir);
	SoundZip::check(gameDir, _storyId);
}

bool GlkInterface::initPictures() {
	if (Pics::exists()) {
		_pics = new Pics();
		SearchMan.add("Pics", _pics, 99, false);
		return true;
	}

	if (h_version == V6)
		warning("Could not locate MG1 file");
	return false;
}

int GlkInterface::os_char_width(zchar z) {
	// Note: I'm presuming this is 1 because Glk Text Grid windows take care of font sizes internally,
	// so we can pretend that any font has a 1x1 size
	return 1;
}

int GlkInterface::os_string_width(const zchar *s) {
	int width = 0;
	zchar c;
	while ((c = *s++) != 0)
		if (c == ZC_NEW_STYLE || c == ZC_NEW_FONT)
			s++;
		else
			width += os_char_width(c);
	return width;
}

int GlkInterface::os_string_length(zchar *s) {
	int length = 0;
	while (*s++) length++;
	return length;
}

void GlkInterface::os_prepare_sample(int a) {
	glk_sound_load_hint(a, 1);
}

void GlkInterface::os_finish_with_sample(int a) {
	glk_sound_load_hint(a, 0);
}

void GlkInterface::os_start_sample(int number, int volume, int repeats, zword eos) {
	int vol;

	if (!gos_channel) {
		gos_channel = glk_schannel_create(0);
		if (!gos_channel)
			return;
	}

	switch (volume) {
	case   1: vol = 0x02000; break;
	case   2: vol = 0x04000; break;
	case   3: vol = 0x06000; break;
	case   4: vol = 0x08000; break;
	case   5: vol = 0x0a000; break;
	case   6: vol = 0x0c000; break;
	case   7: vol = 0x0e000; break;
	case   8: vol = 0x10000; break;
	default:  vol = 0x20000; break;
	}

	glk_schannel_play_ext(gos_channel, number, repeats, eos);
	glk_schannel_set_volume(gos_channel, vol);
}

void GlkInterface::os_stop_sample(int a) {
	if (!gos_channel)
		return;
	glk_schannel_stop(gos_channel);
}

void GlkInterface::os_beep(int volume) {
}

bool GlkInterface::os_picture_data(int picture, uint *height, uint *width) {
	if (_pics && picture == 0) {
		*width = _pics->version();
		*height = _pics->size();
		return true;
	} else {
		bool result = glk_image_get_info(picture, width, height);

		int cellW = g_conf->_monoInfo._cellW;
		int cellH = g_conf->_monoInfo._cellH;
		*width = (*width + cellW - 1) / cellW;
		*height = (*height + cellH - 1) / cellH;

		return result;
	}
}

void GlkInterface::start_sample(int number, int volume, int repeats, zword eos) {
	static zbyte LURKING_REPEATS[] = {
		0x00, 0x00, 0x00, 0x01, 0xff,
		0x00, 0x01, 0x01, 0x01, 0x01,
		0xff, 0x01, 0x01, 0xff, 0x00,
		0xff, 0xff, 0xff, 0xff, 0xff
	};

	if (_storyId == LURKING_HORROR)
		repeats = LURKING_REPEATS[number];

	os_start_sample(number, volume, repeats, eos);

	_soundPlaying = true;
}

void GlkInterface::start_next_sample() {
	if (next_sample != 0)
		start_sample(next_sample, next_volume, 0, 0);

	next_sample = 0;
	next_volume = 0;
}

void GlkInterface::gos_update_width() {
	uint width;
	if (_wp._upper) {
		glk_window_get_size(_wp._upper, &width, nullptr);
		h_screen_cols = width;
		SET_BYTE(H_SCREEN_COLS, width);
		if ((uint)curx > width) {
			glk_window_move_cursor(_wp._upper, 0, cury - 1);
			curx = 1;
		}
	}
}

void GlkInterface::gos_update_height() {
	uint height_upper;
	uint height_lower;
	if (gos_curwin) {
		glk_window_get_size(_wp._upper, nullptr, &height_upper);
		glk_window_get_size(_wp._lower, nullptr, &height_lower);
		h_screen_rows = height_upper + height_lower + 1;
		SET_BYTE(H_SCREEN_ROWS, h_screen_rows);
	}
}

void GlkInterface::reset_status_ht() {
	uint height;
	if (_wp._upper && h_version != 6) {
		glk_window_get_size(_wp._upper, nullptr, &height);
		if ((uint)mach_status_ht != height) {
			glk_window_set_arrangement(glk_window_get_parent(_wp._upper),
				winmethod_Above | winmethod_Fixed, mach_status_ht, nullptr);
		}
	}
}

void GlkInterface::erase_window(zword w) {
	if (w == 0)
		glk_window_clear(_wp._lower);
	else if (_wp._upper) {
#ifdef GARGLK
		garglk_set_reversevideo_stream(glk_window_get_stream(_wp._upper), true);
#endif /* GARGLK */
		
		memset(statusline, ' ', sizeof statusline);
		glk_window_clear(_wp._upper);
		reset_status_ht();
		curr_status_ht = 0;
	}
}

void GlkInterface::split_window(zword lines) {
	if (!_wp._upper)
		return;

	// The top line is always set for V1 to V3 games
	if (h_version < V4)
		lines++;

	if (!lines || lines > curr_status_ht) {
		uint height;

		glk_window_get_size(_wp._upper, nullptr, &height);
		if (lines != height)
			glk_window_set_arrangement(
				glk_window_get_parent(_wp._upper),
				winmethod_Above | winmethod_Fixed,
				lines, nullptr);
		curr_status_ht = lines;
	}
	mach_status_ht = lines;
	if (cury > lines)
	{
		glk_window_move_cursor(_wp._upper, 0, 0);
		curx = cury = 1;
	}
	gos_update_width();

	if (h_version == V3)
		glk_window_clear(_wp._upper);
}

void GlkInterface::restart_screen() {
	erase_window(0);
	erase_window(1);
	split_window(0);
}

void GlkInterface::packspaces(zchar *src, zchar *dst) {
	int killing = 0;
	while (*src) {
		if (*src == 0x20202020)
			*src = ' ';
		if (*src == ' ')
			killing++;
		else
			killing = 0;
		if (killing > 2)
			src++;
		else
			*dst++ = *src++;
	}

	*dst = 0;
}

void GlkInterface::smartstatusline() {
	zchar packed[256];
	uint32 buf[256];
	zchar *a, *b, *c, *d;
	int roomlen, scorelen, scoreofs;
	int len, tmp;

	packspaces(statusline, packed);
	len = os_string_length(packed);

	a = packed;
	while (a[0] == ' ')
		a++;

	b = a;
	while (b[0] != 0 && !(b[0] == ' ' && b[1] == ' '))
		b++;

	c = b;
	while (c[0] == ' ')
		c++;

	d = packed + len - 1;
	while (d[0] == ' ' && d > c)
		d--;
	if (d[0] != ' ' && d[0] != 0)
		d++;
	if (d < c)
		d = c;

	roomlen = b - a;
	scorelen = d - c;
	scoreofs = h_screen_cols - scorelen - 2;
	if (scoreofs <= roomlen)
		scoreofs = roomlen + 2;

	for (tmp = 0; tmp < h_screen_cols; tmp++)
		buf[tmp] = ' ';

	memcpy(buf + 1 + scoreofs, c, scorelen * sizeof(zchar));
	memcpy(buf + 1, a, roomlen * sizeof(zchar));

	glk_window_move_cursor(_wp._upper, 0, 0);
	glk_put_buffer_uni(buf, h_screen_cols);
	glk_window_move_cursor(_wp._upper, cury - 1, curx - 1);
}

void GlkInterface::gos_cancel_pending_line() {
	event_t ev;
	glk_cancel_line_event(gos_linewin, &ev);
	gos_linebuf[ev.val1] = '\0';
	gos_linepending = 0;
}

void GlkInterface::showBeyondZorkTitle() {
	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	if (saveSlot == -1) {
		winid_t win = glk_window_open(0, 0, 0, wintype_Graphics, 0);
		if (glk_image_draw_scaled(win, 1, 0, 0, g_vm->_screen->w, g_vm->_screen->h))
			_events->waitForPress();

		glk_window_close(win, nullptr);
	}
}

void GlkInterface::os_draw_picture(int picture, const Common::Point &pos) {
	if (cwin == 0) {
		// Picture embedded within the lower text area
		glk_image_draw(_wp._lower, picture, imagealign_MarginLeft, 0);
	} else {
		glk_image_draw(_wp._background, picture,
			(pos.x - 1) * g_conf->_monoInfo._cellW,
			(pos.y - 1) * g_conf->_monoInfo._cellH);
	}
}

void GlkInterface::os_draw_picture(int picture, const Common::Rect &r) {
	Point cell(g_conf->_monoInfo._cellW, g_conf->_monoInfo._cellH);
	glk_image_draw_scaled(_wp._background, picture, (r.left - 1) * cell.x, (r.top - 1) * cell.y,
		r.width() * cell.x, r.height() * cell.y);
}

zchar GlkInterface::os_read_key(int timeout, bool show_cursor) {
	event_t ev;
	winid_t win = gos_curwin ? gos_curwin : _wp._lower;

	if (gos_linepending)
		gos_cancel_pending_line();

	glk_request_char_event_uni(win);
	if (timeout != 0)
		glk_request_timer_events(timeout * 100);

	while (!shouldQuit()) {
		glk_select(&ev);
		if (ev.type == evtype_Arrange) {
			gos_update_height();
			gos_update_width();
		} else if (ev.type == evtype_Timer) {
			glk_cancel_char_event(win);
			glk_request_timer_events(0);
			return ZC_TIME_OUT;
		} else if (ev.type == evtype_CharInput)
			break;
	}
	if (shouldQuit())
		return 0;

	glk_request_timer_events(0);

	if (_wp._upper && mach_status_ht < curr_status_ht)
		reset_status_ht();
	curr_status_ht = 0;

	switch (ev.val1) {
	case keycode_Escape: return ZC_ESCAPE;
	case keycode_PageUp: return ZC_ARROW_MIN;
	case keycode_PageDown: return ZC_ARROW_MAX;
	case keycode_Left: return ZC_ARROW_LEFT;
	case keycode_Right: return ZC_ARROW_RIGHT;
	case keycode_Up: return ZC_ARROW_UP;
	case keycode_Down: return ZC_ARROW_DOWN;
	case keycode_Return: return ZC_RETURN;
	case keycode_Delete: return ZC_BACKSPACE;
	case keycode_Tab: return ZC_INDENT;
	default:
		return ev.val1;
	}
}

zchar GlkInterface::os_read_line(int max, zchar *buf, int timeout, int width, int continued) {
	event_t ev;
	winid_t win = gos_curwin ? gos_curwin : _wp._lower;

	if (!continued && gos_linepending)
		gos_cancel_pending_line();

	if (!continued || !gos_linepending) {
		glk_request_line_event_uni(win, buf, max, os_string_length(buf));
		if (timeout != 0)
			glk_request_timer_events(timeout * 100);
	}

	gos_linepending = 0;

	while (!shouldQuit()) {
		glk_select(&ev);
		if (ev.type == evtype_Arrange) {
			gos_update_height();
			gos_update_width();
		} else if (ev.type == evtype_Timer) {
			gos_linewin = win;
			gos_linepending = 1;
			gos_linebuf = buf;
			return ZC_TIME_OUT;
		} else if (ev.type == evtype_LineInput) {
			break;
		}
	}
	if (shouldQuit())
		return 0;

	glk_request_timer_events(0);
	buf[ev.val1] = '\0';

	// If the upper status line area was expanded to show a text box/quotation, restore it back
	if (_wp._upper && mach_status_ht < curr_status_ht)
		reset_status_ht();
	curr_status_ht = 0;

	if (ev.val2) {
		// Line terminator specified, so return it
		if (_storyId == BEYOND_ZORK && ev.val2 == keycode_PageUp)
			return ZC_ARROW_UP;
		else if (_storyId == BEYOND_ZORK && ev.val2 == keycode_PageDown)
			return ZC_ARROW_DOWN;

		return ev.val2;
	}

	return ZC_RETURN;
}

} // End of namespace Frotz
} // End of namespace Glk
