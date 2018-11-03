/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gargoyle/glk.h"
#include "gargoyle/conf.h"
#include "gargoyle/events.h"
#include "gargoyle/picture.h"
#include "gargoyle/streams.h"
#include "gargoyle/unicode.h"
#include "gargoyle/windows.h"
#include "gargoyle/window_graphics.h"
#include "gargoyle/window_text_buffer.h"
#include "gargoyle/window_pair.h"


namespace Gargoyle {

Glk::Glk(OSystem *syst, const GargoyleGameDescription *gameDesc) : 
		GargoyleEngine(syst, gameDesc), _gliFirstEvent(false) {
	// Set uppercase/lowercase tables
	int ix, res;
	for (ix = 0; ix < 256; ix++) {
		_charToupperTable[ix] = ix;
		_charTolowerTable[ix] = ix;
	}

	for (ix = 0; ix < 256; ix++) {
		if (ix >= 'A' && ix <= 'Z')
			res = ix + ('a' - 'A');
		else if (ix >= 0xC0 && ix <= 0xDE && ix != 0xD7)
			res = ix + 0x20;
		else
			res = 0;

		if (res) {
			_charTolowerTable[ix] = res;
			_charToupperTable[res] = ix;
		}
	}
}

void Glk::glk_exit(void) {
	glk_put_string("[ press any key to exit ]");
	_events->waitForPress();

	quitGame();
}

void Glk::glk_set_interrupt_handler(void(*func)(void)) {
	// This library doesn't handle interrupts.
}

void Glk::glk_tick(void) {
	// Nothing needed
}

glui32 Glk::glk_gestalt(glui32 id, glui32 val) {
	return glk_gestalt_ext(id, val, nullptr, 0);
}

glui32 Glk::glk_gestalt_ext(glui32 id, glui32 val, glui32 *arr, glui32 arrlen) {
	switch (id) {
	case gestalt_Version:
		return 0x00000703;

	case gestalt_LineInput:
		if (val >= 32 && val < 0x10ffff)
			return true;
		else
			return false;

	case gestalt_CharInput:
		if (val >= 32 && val < 0x10ffff)
			return true;
		else if (val == keycode_Return)
			return true;
		else
			return false;

	case gestalt_CharOutput:
		if (val >= 32 && val < 0x10ffff) {
			if (arr && arrlen >= 1)
				arr[0] = 1;
			return gestalt_CharOutput_ExactPrint;
		} else {
			// cheaply, we don't do any translation of printed characters,
			// so the output is always one character even if it's wrong.
			if (arr && arrlen >= 1)
				arr[0] = 1;
			return gestalt_CharOutput_CannotPrint;
		}

	case gestalt_MouseInput:
		if (val == wintype_TextGrid)
			return true;
		if (val == wintype_Graphics)
			return true;
		return false;

	case gestalt_Timer:
		return true;

	case gestalt_Graphics:
	case gestalt_GraphicsTransparency:
		return g_conf->_graphics;

	case gestalt_DrawImage:
		if (val == wintype_TextBuffer)
			return g_conf->_graphics;
		if (val == wintype_Graphics)
			return g_conf->_graphics;
		return false;

	case gestalt_Sound:
	case gestalt_SoundVolume:
	case gestalt_SoundMusic:
	case gestalt_SoundNotify:
		return g_conf->_sound;

	case gestalt_Sound2:
		return false;

	case gestalt_Unicode:
		return true;
	case gestalt_UnicodeNorm:
		return true;

	case gestalt_Hyperlinks:
		return true;
	case gestalt_HyperlinkInput:
		return true;

	case gestalt_LineInputEcho:
		return true;
	case gestalt_LineTerminators:
		return true;
	case gestalt_LineTerminatorKey:
		return Window::checkTerminator(val);

	case gestalt_DateTime:
		return true;

	case gestalt_GarglkText:
		return true;

	default:
		return false;
	}
}

unsigned char Glk::glk_char_to_lower(unsigned char ch) {
	return _charTolowerTable[ch];
}

unsigned char Glk::glk_char_to_upper(unsigned char ch) {
	return _charToupperTable[ch];
}

winid_t Glk::glk_window_get_root(void) const {
	return _windows->getRoot();
}

winid_t Glk::glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock) const {
	return _windows->windowOpen(split, method, size, wintype, rock);
}

void Glk::glk_window_close(winid_t win, stream_result_t *result) {
	if (!win) {
		warning("glk_window_close: invalid ref");
	} else {
		_windows->windowClose(win, result);
	}
}

void Glk::glk_window_get_size(winid_t win, glui32 *width, glui32 *height) {
	if (!win) {
		warning("window_get_size: invalid ref");
	} else {
		win->getSize(width, height);
	}
}

void Glk::glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin) {
	if (!win) {
		warning("window_set_arrangement: invalid ref");
	} else {
		win->setArrangement(method, size, keywin);
	}
}

void Glk::glk_window_get_arrangement(winid_t win, glui32 *method,
		glui32 *size, winid_t *keyWin) {
	if (!win) {
		warning("window_get_arrangement: invalid ref");
	} else {
		win->getArrangement(method, size, keyWin);
	}
}

winid_t Glk::glk_window_iterate(winid_t win, glui32 *rock) {
	win = win ? win->_next : _windows->getRoot();

	if (win) {
		if (rock)
			*rock = win->_rock;
		return win;
	}

	if (rock)
		*rock = 0;

	return nullptr;
}

glui32 Glk::glk_window_get_rock(winid_t win) {
	if (!win) {
		warning("window_get_rock: invalid ref.");
		return 0;
	}

	return win->_rock;
}

glui32 Glk::glk_window_get_type(winid_t win) {
	if (!win) {
		warning("window_get_parent: invalid ref");
		return 0;
	}

	return win->_type;
}

winid_t Glk::glk_window_get_parent(winid_t win) {
	if (!win) {
		warning("window_get_parent: invalid ref");
		return 0;
	}

	return win->_parent;
}

winid_t Glk::glk_window_get_sibling(winid_t win) {
	if (!win) {
		warning("window_get_sibling: invalid ref");
		return nullptr;
	}

	PairWindow *parentWin = dynamic_cast<PairWindow *>(win->_parent);
	if (!parentWin)
		return nullptr;

	if (parentWin->_child1 == win)
		return parentWin->_child2;
	else if (parentWin->_child2 == win)
		return parentWin->_child1;

	return nullptr;
}

void Glk::glk_window_clear(winid_t win) {
	if (!win) {
		warning("window_clear: invalid ref");
	} else if (win->_lineRequest || win->_lineRequestUni) {
		if (g_conf->_safeClicks && _events->_forceClick) {
			glk_cancel_line_event(win, nullptr);
			_events->_forceClick = false;

			win->clear();
		} else {
			warning("window_clear: window has pending line request");
			return;
		}
	}
}

void Glk::glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos) {
	if (!win) {
		warning("window_move_cursor: invalid ref");
	} else {
		win->moveCursor(Point(xpos, ypos));
	}
}

strid_t Glk::glk_window_get_stream(winid_t win) {
	if (!win) {
		warning("window_get_stream: invalid ref");
		return nullptr;
	}

	return win->_stream;
}

void Glk::glk_window_set_echo_stream(winid_t win, strid_t str) {
	if (!win) {
		warning("window_set_echo_stream: invalid window id");
	} else {
		win->_echoStream = str;
	}
}

strid_t Glk::glk_window_get_echo_stream(winid_t win) {
	if (!win) {
		warning("window_get_echo_stream: invalid ref");
		return nullptr;
	}

	return win->_echoStream;
}

void Glk::glk_set_window(winid_t win) {
	_streams->setCurrent(win ? win->_stream : nullptr);
}

strid_t Glk::glk_stream_open_file(frefid_t fileref, FileMode fmode, glui32 rock) {
	// TODO
	return nullptr;
}

strid_t Glk::glk_stream_open_memory(char *buf, glui32 buflen, FileMode fmode, glui32 rock) {
	return _streams->addMemoryStream(buf, buflen, fmode, rock, false);
}

void Glk::glk_stream_close(strid_t str, stream_result_t *result) {
	str->close(result);
}

strid_t Glk::glk_stream_iterate(strid_t str, glui32 *rockptr) const {
	return str ? str->getNext(rockptr) : _streams->getFirst(rockptr);
}

glui32 Glk::glk_stream_get_rock(strid_t str) const {
	if (!str) {
		warning("stream_get_rock: invalid ref");
		return 0;
	}

	return str->getRock();
}

void Glk::glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekMode) {
	if (!str) {
		warning("stream_set_position: invalid ref");
	} else {
		str->setPosition(pos, seekMode);
	}
}

glui32 Glk::glk_stream_get_position(strid_t str) const {
	if (!str) {
		warning("stream_get_position: invalid ref");
		return 0;
	} else {
		return str->getPosition();
	}
}

void Glk::glk_stream_set_current(strid_t str) {
	_streams->setCurrent(str);
}

strid_t Glk::glk_stream_get_current(void) {
	return _streams->getCurrent();
}

void Glk::glk_put_char(unsigned char ch) {
	_streams->getCurrent()->putChar(ch);
}

void Glk::glk_put_char_stream(strid_t str, unsigned char ch) {
	if (str) {
		str->putChar(ch);
	} else {
		warning("put_char_stream: invalid ref");
	}
}

void Glk::glk_put_string(const char *s) {
	_streams->getCurrent()->putBuffer(s, strlen(s));
}

void Glk::glk_put_string_stream(strid_t str, const char *s) {
	str->putBuffer(s, strlen(s));
}

void Glk::glk_put_buffer(char *buf, glui32 len) {
	_streams->getCurrent()->putBuffer(buf, len);
}

void Glk::glk_put_buffer_stream(strid_t str, const char *buf, glui32 len) {
	str->putBuffer(buf, len);
}

void Glk::glk_set_style(glui32 styl) {
	_streams->getCurrent()->setStyle(styl);
}

void Glk::glk_set_style_stream(strid_t str, glui32 styl) {
	if (str) {
		str->setStyle(styl);
	} else {
		warning("set_style_stream: invalid ref");
	}
}

glsi32 Glk::glk_get_char_stream(strid_t str) {
	if (str) {
		warning("get_char_stream: invalid ref");
		return -1;
	} else {
		return str->getChar();
	}
}

glui32 Glk::glk_get_line_stream(strid_t str, char *buf, glui32 len) {
	// TODO
	return 0;
}

glui32 Glk::glk_get_buffer_stream(strid_t str, char *buf, glui32 len) {
	// TODO
	return 0;
}

void Glk::glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val) {
	// TODO
}

void Glk::glk_stylehint_clear(glui32 wintype, glui32 style, glui32 hint) {
	// TODO
}

glui32 Glk::glk_style_distinguish(winid_t win, glui32 style1, glui32 style2) {
	const WindowStyle *styles = win->getStyles();
	if (!styles)
		return false;

	return styles[style1] == styles[style2] ? 0 : 1;
}

bool Glk::glk_style_measure(winid_t win, glui32 style, glui32 hint, glui32 *result) {
	const WindowStyle *styles = win->getStyles();
	if (!styles)
		return false;

	switch (hint) {
	case stylehint_Indentation:
	case stylehint_ParaIndentation:
		*result = 0;
		break;

	case stylehint_Justification:
		*result = stylehint_just_LeftFlush;
		break;

	case stylehint_Size:
		*result = 1;
		break;

	case stylehint_Weight:
		*result =
			(styles[style].font == PROPB || styles[style].font == PROPZ ||
				styles[style].font == MONOB || styles[style].font == MONOZ);
		break;

	case stylehint_Oblique:
		*result =
			(styles[style].font == PROPI || styles[style].font == PROPZ ||
				styles[style].font == MONOI || styles[style].font == MONOZ);
		break;

	case stylehint_Proportional:
		*result =
			(styles[style].font == PROPR || styles[style].font == PROPI ||
				styles[style].font == PROPB || styles[style].font == PROPZ);
		break;

	case stylehint_TextColor:
		*result =
			(styles[style].fg[0] << 16) | (styles[style].fg[1] << 8) | (styles[style].fg[2]);
		break;

	case stylehint_BackColor:
		*result =
			(styles[style].bg[0] << 16) | (styles[style].bg[1] << 8) | (styles[style].bg[2]);
		break;

	case stylehint_ReverseColor:
		*result = styles[style].reverse;
		break;

	default:
		return false;
	}

	return true;
}

frefid_t Glk::glk_fileref_create_temp(glui32 usage, glui32 rock) {
	return _files->createTemp(usage, rock);
}

frefid_t Glk::glk_fileref_create_by_name(glui32 usage, const char *name, glui32 rock) {
	// Take out all dangerous characters
	Common::String tempName(name);
	for (uint idx = 0; idx < tempName.size(); ++idx) {
		if (tempName[idx] == '/' || tempName[idx] == '\\' || tempName[idx] == ':')
			tempName.setChar(idx, '-');
	}

	return _files->createRef(tempName, usage, rock);
}

frefid_t Glk::glk_fileref_create_by_prompt(glui32 usage, FileMode fmode, glui32 rock) {
	return _files->createByPrompt(usage, fmode, rock);
}

frefid_t Glk::glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock) {
	if (!fref) {
		warning("fileref_create_from_fileref: invalid ref");
		return nullptr;
	} else {
		return _files->createFromRef(fref, usage, rock);
	}
}

void Glk::glk_fileref_destroy(frefid_t fref) {
	_files->deleteRef(fref);
}

frefid_t Glk::glk_fileref_iterate(frefid_t fref, glui32 *rockptr) {
	return _files->iterate(fref, rockptr);
}

glui32 Glk::glk_fileref_get_rock(frefid_t fref) {
	if (!fref) {
		warning("fileref_get_rock: invalid ref.");
		return 0;
	} else {
		return fref->_rock;
	}
}

void Glk::glk_fileref_delete_file(frefid_t fref) {
	fref->deleteFile();
}

glui32 Glk::glk_fileref_does_file_exist(frefid_t fref) {
	return fref->exists();
}

void Glk::glk_select(event_t *event) {
	if (!_gliFirstEvent) {
		_windows->inputGuessFocus();
		_gliFirstEvent = true;
	}
	
	_events->getEvent(event, false);
}

void Glk::glk_select_poll(event_t *event) {
	if (!_gliFirstEvent) {
		_windows->inputGuessFocus();
		_gliFirstEvent = true;
	}

	_events->getEvent(event, true);
}

void Glk::glk_request_timer_events(glui32 millisecs) {
	// TODO
}

void Glk::glk_request_line_event(winid_t win, char *buf, glui32 maxlen, glui32 initlen) {
	if (!win) {
		warning("request_line_event: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
			|| win->_lineRequestUni) {
		warning("request_line_event: window already has keyboard request");
	} else {
		win->requestLineEvent(buf, maxlen, initlen);
	}
}

void Glk::glk_request_char_event(winid_t win) {
	if (!win) {
		warning("request_char_event: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
			|| win->_lineRequestUni) {
		warning("request_char_event: window already has keyboard request");
	} else {
		win->requestCharEvent();
	}
}

void Glk::glk_request_mouse_event(winid_t win) {
	if (!win) {
		warning("request_mouse_event: invalid ref");
	} else {
		win->requestMouseEvent();
	}
}

void Glk::glk_cancel_line_event(winid_t win, event_t *event) {
	if (!win) {
		warning("cancel_line_event: invalid ref");
	} else {
		win->cancelLineEvent(event);
	}
}

void Glk::glk_cancel_char_event(winid_t win) {
	if (!win) {
		warning("glk_cancel_char_event: invalid ref");
	} else {
		win->cancelCharEvent();
	}
}

void Glk::glk_cancel_mouse_event(winid_t win) {
	if (!win) {
		warning("cancel_mouse_event: invalid ref");
	} else {
		win->cancelMouseEvent();
	}
}

void Glk::glk_set_echo_line_event(winid_t win, glui32 val) {
	if (!win) {
		warning("set_echo_line_event: invalid ref");
	} else {
		win->setEchoLineEvent(val);
	}
}

void Glk::glk_set_terminators_line_event(winid_t win, glui32 *keycodes, glui32 count) {
	if (!win) {
		warning("set_terminators_line_event: invalid ref");
	} else {
		win->setTerminatorsLineEvent(keycodes, count);
	}
}

glui32 Glk::glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len, glui32 numchars) {
	return bufferChangeCase(buf, len, numchars, CASE_LOWER, COND_ALL, true);
}

glui32 Glk::glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len, glui32 numchars) {
	return bufferChangeCase(buf, len, numchars, CASE_UPPER, COND_ALL, true);
}

glui32 Glk::glk_buffer_to_title_case_uni(glui32 *buf, glui32 len,
	glui32 numchars, glui32 lowerrest) {
	return bufferChangeCase(buf, len, numchars, CASE_TITLE, COND_LINESTART, lowerrest);
}

void Glk::glk_put_char_uni(glui32 ch) {
	_streams->getCurrent()->putCharUni(ch);
}

void Glk::glk_put_string_uni(glui32 *s) {
	_streams->getCurrent()->putBufferUni(s, strlen_uni(s));
}

void Glk::glk_put_buffer_uni(glui32 *buf, glui32 len) {
	_streams->getCurrent()->putBufferUni(buf, len);
}

void Glk::glk_put_char_stream_uni(strid_t str, glui32 ch) {
	if (str) {
		str->putCharUni(ch);
	} else {
		warning("put_char_stream_uni: invalid ref");
	}
}

void Glk::glk_put_string_stream_uni(strid_t str, const glui32 *s) {
	if (str) {
		str->putBufferUni(s, strlen_uni(s));
	} else {
		warning("put_string_stream_uni: invalid ref");
	}
}

void Glk::glk_put_buffer_stream_uni(strid_t str, const glui32 *buf, glui32 len) {
	if (str) {
		str->putBufferUni(buf, len);
	} else {
		warning("put_buffer_stream_uni: invalid ref");
	}
}

glsi32 Glk::glk_get_char_stream_uni(strid_t str) {
	if (str) {
		return str->getCharUni();
	} else {
		warning("get_char_stream_uni: invalid ref");
		return -1;
	}
}

glui32 Glk::glk_get_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len) {
	if (str) {
		return str->getBufferUni(buf, len);
	} else {
		warning("get_buffer_stream_uni: invalid ref");
		return 0;
	}
}

glui32 Glk::glk_get_line_stream_uni(strid_t str, glui32 *buf, glui32 len) {
	if (str) {
		return str->getLineUni(buf, len);
	} else  {
		warning("get_line_stream_uni: invalid ref");
		return (glui32 )-1;
	}
}

strid_t Glk::glk_stream_open_file_uni(frefid_t fileref, FileMode fmode, glui32 rock) {
	// TODO
	return nullptr;
}

strid_t Glk::glk_stream_open_memory_uni(glui32 *buf, glui32 buflen, FileMode fmode, glui32 rock) {
	return _streams->addMemoryStream(buf, buflen, fmode, rock, true);
}

void Glk::glk_request_char_event_uni(winid_t win) {
	if (!win) {
		warning("request_char_event_uni: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
			|| win->_lineRequestUni) {
		warning("request_char_event_uni: window already has keyboard request");
	} else {
		win->requestCharEvent();
	}
}

void Glk::glk_request_line_event_uni(winid_t win, glui32 *buf, glui32 maxlen, glui32 initlen) {
	if (!win) {
		warning("request_line_event_uni: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
			|| win->_lineRequestUni) {
		warning("request_line_event_uni: window already has keyboard request");
	} else {
		win->requestLineEventUni(buf, maxlen, initlen);
	}
}

glui32 Glk::glk_buffer_canon_decompose_uni(glui32 *buf, glui32 len,
	glui32 numchars) {
	// TODO
	return 0;
}

glui32 Glk::glk_buffer_canon_normalize_uni(glui32 *buf, glui32 len, glui32 numchars) {
	return 0;
}

glui32 Glk::glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2) {
	if (!win) {
		warning("image_draw: invalid ref");
	} else if (g_conf->_graphics) {
		TextBufferWindow *textWin = dynamic_cast<TextBufferWindow *>(win);
		GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);

		if (textWin)
			textWin->drawPicture(image, val1, false, 0, 0);
		else if (gfxWin)
			gfxWin->drawPicture(image, val1, val2, false, 0, 0);
	}

	return false;
}

glui32 Glk::glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2,
		glui32 width, glui32 height) {
	if (!win) {
		warning("image_draw_scaled: invalid ref");
	} else if (g_conf->_graphics) {
		TextBufferWindow *textWin = dynamic_cast<TextBufferWindow *>(win);
		GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);

		if (textWin)
			textWin->drawPicture(image, val1, true, width, height);
		else if (gfxWin)
			gfxWin->drawPicture(image, val1, val2, true, width, height);
	}

	return false;
}

glui32 Glk::glk_image_get_info(glui32 image, glui32 *width, glui32 *height) {
	if (!g_conf->_graphics)
		return false;

	Picture *pic = Picture::load(image);
	if (!pic)
		return false;

	if (width)
		*width = pic->w;
	if (height)
		*height = pic->h;

	return true;
}

void Glk::glk_window_flow_break(winid_t win) {
	if (!win) {
		warning("window_erase_rect: invalid ref");
	} else {
		win->flowBreak();
	}
}

void Glk::glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height) {
	if (!win) {
		warning("window_erase_rect: invalid ref");
	} else {
		win->eraseRect(false, Rect(left, top, left + width, top + height));
	}
}

void Glk::glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top,
		glui32 width, glui32 height) {
	if (!win) {
		warning("window_fill_rect: invalid ref");
	} else {
		win->eraseRect(color, Rect(left, top, left + width, top + height));
	}
}

void Glk::glk_window_set_background_color(winid_t win, glui32 color) {
	if (!win) {
		warning("window_set_background_color: invalid ref");
	} else {
		win->setBackgroundColor(color);
	}
}

schanid_t Glk::glk_schannel_create(glui32 rock) {
	// TODO
	return nullptr;
}

void Glk::glk_schannel_destroy(schanid_t chan) {
	// TODO
}

schanid_t Glk::glk_schannel_iterate(schanid_t chan, glui32 *rockptr) {
	// TODO
	return nullptr;
}

glui32 Glk::glk_schannel_get_rock(schanid_t chan) {
	// TODO
	return 0;
}

glui32 Glk::glk_schannel_play(schanid_t chan, glui32 snd) {
	// TODO
	return 0;
}

glui32 Glk::glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify) {
	// TODO
	return 0;
}

void Glk::glk_schannel_stop(schanid_t chan) {
	// TODO
}

void Glk::glk_schannel_set_volume(schanid_t chan, glui32 vol) {
	// TODO
}

void Glk::glk_sound_load_hint(glui32 snd, glui32 flag) {
	// TODO
}

schanid_t Glk::glk_schannel_create_ext(glui32 rock, glui32 volume) {
	// TODO
	return nullptr;
}

glui32 Glk::glk_schannel_play_multi(schanid_t *chanarray, glui32 chancount,
	glui32 *sndarray, glui32 soundcount, glui32 notify) {
	// TODO
	return 0;
}

void Glk::glk_schannel_pause(schanid_t chan) {
	// TODO
}

void Glk::glk_schannel_unpause(schanid_t chan) {
	// TODO
}

void Glk::glk_schannel_set_volume_ext(schanid_t chan, glui32 vol,
	glui32 duration, glui32 notify) {
	// TODO
}

void Glk::glk_set_hyperlink(glui32 linkval) {
	_streams->getCurrent()->setHyperlink(linkval);
}

void Glk::glk_set_hyperlink_stream(strid_t str, glui32 linkval) {
	if (str)
		str->setHyperlink(linkval);
}

void Glk::glk_request_hyperlink_event(winid_t win) {
	if (!win) {
		warning("request_hyperlink_event: invalid ref");
	} else {
		win->requestHyperlinkEvent();
	}
}

void Glk::glk_cancel_hyperlink_event(winid_t win) {
	if (win) {
		win->cancelHyperlinkEvent();
	} else {
		warning("cancel_hyperlink_event: invalid ref");
	}
}

/*--------------------------------------------------------------------------*/

void Glk::glk_current_time(glktimeval_t *time) {
	TimeAndDate td;
	*time = td;
}

glsi32 Glk::glk_current_simple_time(glui32 factor) {
	assert(factor);
	TimeAndDate td;

	return td / factor;
}

void Glk::glk_time_to_date_utc(const glktimeval_t *time, glkdate_t *date) {
	// TODO: timezones aren't currently supported
	*date = TimeAndDate(*time);
}

void Glk::glk_time_to_date_local(const glktimeval_t *time, glkdate_t *date) {
	*date = TimeAndDate(*time);
}

void Glk::glk_simple_time_to_date_utc(glsi32 time, glui32 factor, glkdate_t *date) {
	TimeSeconds secs = (int64)time * factor;
	*date = TimeAndDate(secs);
}

void Glk::glk_simple_time_to_date_local(glsi32 time, glui32 factor, glkdate_t *date) {
	TimeSeconds secs = (int64)time * factor;
	*date = TimeAndDate(secs);
}

void Glk::glk_date_to_time_utc(const glkdate_t *date, glktimeval_t *time) {
	// TODO: timezones aren't currently supported
	*time = TimeAndDate(*date);
}

void Glk::glk_date_to_time_local(const glkdate_t *date, glktimeval_t *time) {
	*time = TimeAndDate(*date);
}

glsi32 Glk::glk_date_to_simple_time_utc(const glkdate_t *date, glui32 factor) {
	// TODO: timezones aren't currently supported
	assert(factor);
	TimeSeconds ts = TimeAndDate(*date);
	return ts / factor;
}

glsi32 Glk::glk_date_to_simple_time_local(const glkdate_t *date, glui32 factor) {
	assert(factor);
	TimeSeconds ts = TimeAndDate(*date);
	return ts / factor;
}

/*--------------------------------------------------------------------------*/

/* XXX non-official Glk functions that may or may not exist */

char *garglk_fileref_get_name(frefid_t fref) {
	// TODO
	return nullptr;
}

void Glk::garglk_set_program_name(const char *name) {
	// TODO
}

void Glk::garglk_set_program_info(const char *info) {
	// TODO
}

void Glk::garglk_set_story_name(const char *name) {
	// TODO
}

void Glk::garglk_set_story_title(const char *title) {
	// TODO
}

void Glk::garglk_set_config(const char *name) {
	// TODO
}

/* garglk_unput_string - removes the specified string from the end of the output buffer, if
* indeed it is there. */
void Glk::garglk_unput_string(char *str) {
	// TODO
}

void Glk::garglk_unput_string_uni(glui32 *str) {
	// TODO
}

void Glk::garglk_set_zcolors(glui32 fg, glui32 bg) {
	// TODO
}

void Glk::garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg) {
	// TODO
}

void Glk::garglk_set_reversevideo(glui32 reverse) {
	// TODO
}

void Glk::garglk_set_reversevideo_stream(strid_t str, glui32 reverse) {
	// TODO
}

} // End of namespace Gargoyle
