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

#include "glk/glk_api.h"
#include "glk/conf.h"
#include "glk/events.h"
#include "glk/picture.h"
#include "glk/sound.h"
#include "glk/streams.h"
#include "glk/unicode.h"
#include "glk/windows.h"
#include "glk/window_graphics.h"
#include "glk/window_text_buffer.h"
#include "glk/window_pair.h"
#include "common/translation.h"

namespace Glk {

GlkAPI::GlkAPI(OSystem *syst, const GlkGameDescription &gameDesc) :
		GlkEngine(syst, gameDesc), _gliFirstEvent(false) {
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

void GlkAPI::glk_exit(void) {
	glk_put_string_uni(_("[ press any key to exit ]").u32_str());
	_events->waitForPress();

	// Trigger a ScumMVM shutdown of game
	quitGame();
	Common::Event e;
	g_system->getEventManager()->pollEvent(e);
}

void GlkAPI::glk_set_interrupt_handler(void(*func)(void)) {
	// This library doesn't handle interrupts.
}

void GlkAPI::glk_tick(void) {
	// Nothing needed
}

uint GlkAPI::glk_gestalt(uint id, uint val) {
	return glk_gestalt_ext(id, val, nullptr, 0);
}

uint GlkAPI::glk_gestalt_ext(uint id, uint val, uint *arr, uint arrlen) {
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
	case gestalt_Sound2:
	case gestalt_SoundVolume:
	case gestalt_SoundMusic:
	case gestalt_SoundNotify:
		return g_conf->_sound;

	case gestalt_LineTerminatorKey:
		return Window::checkBasicTerminators(val);

	case gestalt_Timer:
	case gestalt_Unicode:
	case gestalt_UnicodeNorm:
	case gestalt_Hyperlinks:
	case gestalt_HyperlinkInput:
	case gestalt_LineInputEcho:
	case gestalt_LineTerminators:
	case gestalt_DateTime:
	case gestalt_GarglkText:
		return true;

	default:
		return false;
	}
}

unsigned char GlkAPI::glk_char_to_lower(unsigned char ch) {
	return _charTolowerTable[ch];
}

unsigned char GlkAPI::glk_char_to_upper(unsigned char ch) {
	return _charToupperTable[ch];
}

winid_t GlkAPI::glk_window_get_root(void) const {
	return _windows->getRoot();
}

winid_t GlkAPI::glk_window_open(winid_t split, uint method, uint size, uint wintype, uint rock) const {
	return _windows->windowOpen(split, method, size, wintype, rock);
}

void GlkAPI::glk_window_close(winid_t win, stream_result_t *result) {
	if (win) {
		_windows->windowClose(win, result);
	} else {
		warning("glk_window_close: invalid ref");
	}
}

void GlkAPI::glk_window_get_size(winid_t win, uint *width, uint *height) {
	if (win) {
		win->getSize(width, height);
	} else {
		warning("window_get_size: invalid ref");
	}
}

void GlkAPI::glk_window_set_arrangement(winid_t win, uint method, uint size, winid_t keywin) {
	if (win) {
		win->setArrangement(method, size, keywin);
	} else {
		warning("window_set_arrangement: invalid ref");
	}
}

void GlkAPI::glk_window_get_arrangement(winid_t win, uint *method,
                                     uint *size, winid_t *keyWin) {
	if (win) {
		win->getArrangement(method, size, keyWin);
	} else {
		warning("window_get_arrangement: invalid ref");
	}
}

winid_t GlkAPI::glk_window_iterate(winid_t win, uint *rock) {
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

uint GlkAPI::glk_window_get_rock(winid_t win) {
	if (win) {
		return win->_rock;
	} else {
		warning("window_get_rock: invalid ref.");
		return 0;
	}
}

uint GlkAPI::glk_window_get_type(winid_t win) {
	if (win) {
		return win->_type;
	} else {
		warning("window_get_parent: invalid ref");
		return 0;
	}
}

winid_t GlkAPI::glk_window_get_parent(winid_t win) {
	if (!win) {
		warning("window_get_parent: invalid ref");
		return 0;
	}

	return win->_parent;
}

winid_t GlkAPI::glk_window_get_sibling(winid_t win) {
	if (!win) {
		warning("window_get_sibling: invalid ref");
		return nullptr;
	}

	PairWindow *parentWin = dynamic_cast<PairWindow *>(win->_parent);
	if (!parentWin)
		return nullptr;

	int index = parentWin->_children.indexOf(win);
	if (index == ((int)parentWin->_children.size() - 1))
		return parentWin->_children.front();
	else if (index >= 0)
		return parentWin->_children[index + 1];

	return nullptr;
}

void GlkAPI::glk_window_clear(winid_t win) {
	if (!win) {
		warning("window_clear: invalid ref");
	} else {
		if (win->_lineRequest || win->_lineRequestUni) {
			if (g_conf->_safeClicks && _events->_forceClick) {
				glk_cancel_line_event(win, nullptr);
				_events->_forceClick = false;

				win->clear();
			} else {
				warning("window_clear: window has pending line request");
				return;
			}
		}

		// Clear the window
		win->clear();
	}
}

void GlkAPI::glk_window_move_cursor(winid_t win, uint xpos, uint ypos) {
	if (win) {
		win->moveCursor(Point(xpos, ypos));
	} else {
		warning("window_move_cursor: invalid ref");
	}
}

strid_t GlkAPI::glk_window_get_stream(winid_t win) {
	if (win) {
		return win->_stream;
	} else {
		warning("window_get_stream: invalid ref");
		return nullptr;
	}
}

void GlkAPI::glk_window_set_echo_stream(winid_t win, strid_t str) {
	if (win) {
		win->_echoStream = str;
	} else {
		warning("window_set_echo_stream: invalid window id");
	}
}

strid_t GlkAPI::glk_window_get_echo_stream(winid_t win) {
	if (!win) {
		warning("window_get_echo_stream: invalid ref");
		return nullptr;
	}

	return win->_echoStream;
}

void GlkAPI::glk_set_window(winid_t win) {
	_streams->setCurrent(win ? win->_stream : nullptr);
}

strid_t GlkAPI::glk_stream_open_file(frefid_t fileref, FileMode fmode, uint rock) {
	return _streams->openFileStream(fileref, fmode, rock, false);
}

strid_t GlkAPI::glk_stream_open_memory(char *buf, uint buflen, FileMode fmode, uint rock) {
	return _streams->openMemoryStream(buf, buflen, fmode, rock, false);
}

void GlkAPI::glk_stream_close(strid_t str, stream_result_t *result) {
	str->close(result);
}

strid_t GlkAPI::glk_stream_iterate(strid_t str, uint *rockptr) const {
	return str ? str->getNext(rockptr) : _streams->getFirst(rockptr);
}

uint GlkAPI::glk_stream_get_rock(strid_t str) const {
	if (!str) {
		warning("stream_get_rock: invalid ref");
		return 0;
	}

	return str->getRock();
}

void GlkAPI::glk_stream_set_position(strid_t str, int pos, uint seekMode) {
	if (str) {
		str->setPosition(pos, seekMode);
	} else {
		warning("stream_set_position: invalid ref");
	}
}

uint GlkAPI::glk_stream_get_position(strid_t str) const {
	if (str) {
		return str->getPosition();
	} else {
		warning("stream_get_position: invalid ref");
		return 0;
	}
}

void GlkAPI::glk_stream_set_current(strid_t str) {
	_streams->setCurrent(str);
}

strid_t GlkAPI::glk_stream_get_current(void) {
	return _streams->getCurrent();
}

void GlkAPI::glk_put_char(unsigned char ch) {
	Stream *str = _streams->getCurrent();
	if (str)
		str->putChar(ch);
	else
		warning("glk_put_char: no stream set");
}

void GlkAPI::glk_put_char_stream(strid_t str, unsigned char ch) {
	if (str) {
		str->putChar(ch);
	} else {
		warning("put_char_stream: invalid ref");
	}
}

void GlkAPI::glk_put_string(const char *s) {
	_streams->getCurrent()->putBuffer(s, strlen(s));
}

void GlkAPI::glk_put_string_stream(strid_t str, const char *s) {
	str->putBuffer(s, strlen(s));
}

void GlkAPI::glk_put_buffer(const char *buf, uint len) {
	_streams->getCurrent()->putBuffer(buf, len);
}

void GlkAPI::glk_put_buffer_stream(strid_t str, const char *buf, uint len) {
	str->putBuffer(buf, len);
}

void GlkAPI::glk_set_style(uint styl) {
	_streams->getCurrent()->setStyle(styl);
}

void GlkAPI::glk_set_style_stream(strid_t str, uint styl) {
	if (str) {
		str->setStyle(styl);
	} else {
		warning("set_style_stream: invalid ref");
	}
}

int GlkAPI::glk_get_char_stream(strid_t str) {
	if (str) {
		return str->getChar();
	} else {
		warning("get_char_stream: invalid ref");
		return -1;
	}
}

uint GlkAPI::glk_get_line_stream(strid_t str, char *buf, uint len) {
	if (str) {
		return str->getLine(buf, len);
	} else {
		warning("get_line_stream: invalid ref");
		return 0;
	}
}

uint GlkAPI::glk_get_buffer_stream(strid_t str, char *buf, uint len) {
	if (str) {
		return str->getBuffer(buf, len);
	} else {
		warning("get_line_stream: invalid ref");
		return 0;
	}
}

void GlkAPI::glk_stylehint_set(uint wintype, uint style, uint hint, int val) {
	WindowStyle *styles;
	bool p, b, i;

	if (wintype == wintype_AllTypes) {
		glk_stylehint_set(wintype_TextGrid, style, hint, val);
		glk_stylehint_set(wintype_TextBuffer, style, hint, val);
		return;
	}

	if (wintype == wintype_TextGrid)
		styles = g_conf->_gStyles;
	else if (wintype == wintype_TextBuffer)
		styles = g_conf->_tStyles;
	else
		return;

	if (!g_conf->_styleHint)
		return;

	switch (hint) {
	case stylehint_TextColor:
		styles[style].fg = val;
		break;

	case stylehint_BackColor:
		styles[style].bg = val;
		break;

	case stylehint_ReverseColor:
		styles[style].reverse = (val != 0);
		break;

	case stylehint_Proportional:
		if (wintype == wintype_TextBuffer) {
			p = val > 0;
			b = styles[style].isBold();
			i = styles[style].isItalic();
			styles[style].font = WindowStyle::makeFont(p, b, i);
		}
		break;

	case stylehint_Weight:
		p = styles[style].isProp();
		b = val > 0;
		i = styles[style].isItalic();
		styles[style].font = WindowStyle::makeFont(p, b, i);
		break;

	case stylehint_Oblique:
		p = styles[style].isProp();
		b = styles[style].isBold();
		i = val > 0;
		styles[style].font = WindowStyle::makeFont(p, b, i);
		break;

	default:
		break;
	}

	if (wintype == wintype_TextBuffer && style == style_Normal && hint == stylehint_BackColor) {
		g_conf->_windowColor = styles[style].bg;
	}

	if (wintype == wintype_TextBuffer && style == style_Normal && hint == stylehint_TextColor) {
		g_conf->_propInfo._moreColor = styles[style].fg;
		g_conf->_propInfo._caretColor = styles[style].fg;
	}
}

void GlkAPI::glk_stylehint_clear(uint wintype, uint style, uint hint) {
	WindowStyle *styles;
	const WindowStyle *defaults;

	if (wintype == wintype_AllTypes) {
		glk_stylehint_clear(wintype_TextGrid, style, hint);
		glk_stylehint_clear(wintype_TextBuffer, style, hint);
		return;
	}

	if (wintype == wintype_TextGrid) {
		styles = g_conf->_gStyles;
		defaults = g_conf->_gStylesDefault;
	} else if (wintype == wintype_TextBuffer) {
		styles = g_conf->_tStyles;
		defaults = g_conf->_tStylesDefault;
	} else {
		return;
	}

	if (!g_conf->_styleHint)
		return;

	switch (hint) {
	case stylehint_TextColor:
		styles[style].fg = defaults[style].fg;
		break;

	case stylehint_BackColor:
		styles[style].bg = defaults[style].bg;
		break;

	case stylehint_ReverseColor:
		styles[style].reverse = defaults[style].reverse;
		break;

	case stylehint_Proportional:
	case stylehint_Weight:
	case stylehint_Oblique:
		styles[style].font = defaults[style].font;
		break;

	default:
		break;
	}
}

uint GlkAPI::glk_style_distinguish(winid_t win, uint style1, uint style2) {
	const WindowStyle *styles = win->getStyles();
	if (!styles)
		return false;

	return styles[style1] == styles[style2] ? 0 : 1;
}

bool GlkAPI::glk_style_measure(winid_t win, uint style, uint hint, uint *result) {
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
		*result = styles[style].fg;
		break;

	case stylehint_BackColor:
		*result = styles[style].bg;
		break;

	case stylehint_ReverseColor:
		*result = styles[style].reverse;
		break;

	default:
		return false;
	}

	return true;
}

frefid_t GlkAPI::glk_fileref_create_temp(uint usage, uint rock) {
	return _streams->createTemp(usage, rock);
}

frefid_t GlkAPI::glk_fileref_create_by_name(uint usage, const char *name, uint rock) {
	// Take out all dangerous characters
	Common::String tempName(name);
	for (uint idx = 0; idx < tempName.size(); ++idx) {
		if (tempName[idx] == '/' || tempName[idx] == '\\' || tempName[idx] == ':')
			tempName.setChar(idx, '-');
	}

	return _streams->createRef(tempName, usage, rock);
}

frefid_t GlkAPI::glk_fileref_create_by_prompt(uint usage, FileMode fmode, uint rock) {
	return _streams->createByPrompt(usage, fmode, rock);
}

frefid_t GlkAPI::glk_fileref_create_from_fileref(uint usage, frefid_t fref, uint rock) {
	if (!fref) {
		warning("fileref_create_from_fileref: invalid ref");
		return nullptr;
	} else {
		return _streams->createFromRef(fref, usage, rock);
	}
}

void GlkAPI::glk_fileref_destroy(frefid_t fref) {
	_streams->deleteRef(fref);
}

frefid_t GlkAPI::glk_fileref_iterate(frefid_t fref, uint *rockptr) {
	return _streams->iterate(fref, rockptr);
}

uint GlkAPI::glk_fileref_get_rock(frefid_t fref) {
	if (!fref) {
		warning("fileref_get_rock: invalid ref.");
		return 0;
	} else {
		return fref->_rock;
	}
}

void GlkAPI::glk_fileref_delete_file(frefid_t fref) {
	fref->deleteFile();
}

uint GlkAPI::glk_fileref_does_file_exist(frefid_t fref) {
	return fref->exists();
}

void GlkAPI::glk_select(event_t *event) {
	if (!_gliFirstEvent) {
		_windows->inputGuessFocus();
		_gliFirstEvent = true;
	}

	_events->getEvent(event, false);
}

void GlkAPI::glk_select_poll(event_t *event) {
	if (!_gliFirstEvent) {
		_windows->inputGuessFocus();
		_gliFirstEvent = true;
	}

	_events->getEvent(event, true);
}

void GlkAPI::glk_request_timer_events(uint millisecs) {
	_events->setTimerInterval(millisecs);
}

void GlkAPI::glk_request_line_event(winid_t win, char *buf, uint maxlen, uint initlen) {
	if (!win) {
		warning("request_line_event: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
	           || win->_lineRequestUni) {
		warning("request_line_event: window already has keyboard request");
	} else {
		win->requestLineEvent(buf, maxlen, initlen);
	}
}

void GlkAPI::glk_request_char_event(winid_t win) {
	if (!win) {
		warning("request_char_event: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
	           || win->_lineRequestUni) {
		warning("request_char_event: window already has keyboard request");
	} else {
		win->requestCharEvent();
	}
}

void GlkAPI::glk_request_mouse_event(winid_t win) {
	if (!win) {
		warning("request_mouse_event: invalid ref");
	} else {
		win->requestMouseEvent();
	}
}

void GlkAPI::glk_cancel_line_event(winid_t win, event_t *event) {
	if (!win) {
		warning("cancel_line_event: invalid ref");
	} else {
		win->cancelLineEvent(event);
	}
}

void GlkAPI::glk_cancel_char_event(winid_t win) {
	if (!win) {
		warning("glk_cancel_char_event: invalid ref");
	} else {
		win->cancelCharEvent();
	}
}

void GlkAPI::glk_cancel_mouse_event(winid_t win) {
	if (!win) {
		warning("cancel_mouse_event: invalid ref");
	} else {
		win->cancelMouseEvent();
	}
}

void GlkAPI::glk_set_echo_line_event(winid_t win, uint val) {
	if (!win) {
		warning("set_echo_line_event: invalid ref");
	} else {
		win->setEchoLineEvent(val);
	}
}

void GlkAPI::glk_set_terminators_line_event(winid_t win, const uint32 *keycodes, uint count) {
	if (!win) {
		warning("set_terminators_line_event: invalid ref");
	} else {
		win->setTerminatorsLineEvent(keycodes, count);
	}
}

uint GlkAPI::glk_buffer_to_lower_case_uni(uint32 *buf, uint len, uint numchars) {
	return bufferChangeCase(buf, len, numchars, CASE_LOWER, COND_ALL, true);
}

uint GlkAPI::glk_buffer_to_upper_case_uni(uint32 *buf, uint len, uint numchars) {
	return bufferChangeCase(buf, len, numchars, CASE_UPPER, COND_ALL, true);
}

uint GlkAPI::glk_buffer_to_title_case_uni(uint32 *buf, uint len,
        uint numchars, uint lowerrest) {
	return bufferChangeCase(buf, len, numchars, CASE_TITLE, COND_LINESTART, lowerrest);
}

void GlkAPI::glk_put_char_uni(uint32 ch) {
	Stream *str = _streams->getCurrent();
	if (str)
		str->putCharUni(ch);
	else
		warning("glk_put_char_uni: no stream set");
}

void GlkAPI::glk_put_string_uni(const uint32 *s) {
	_streams->getCurrent()->putBufferUni(s, strlen_uni(s));
}

void GlkAPI::glk_put_buffer_uni(const uint32 *buf, uint len) {
	_streams->getCurrent()->putBufferUni(buf, len);
}

void GlkAPI::glk_put_char_stream_uni(strid_t str, uint32 ch) {
	if (str) {
		str->putCharUni(ch);
	} else {
		warning("put_char_stream_uni: invalid ref");
	}
}

void GlkAPI::glk_put_string_stream_uni(strid_t str, const uint32 *s) {
	if (str) {
		str->putBufferUni(s, strlen_uni(s));
	} else {
		warning("put_string_stream_uni: invalid ref");
	}
}

void GlkAPI::glk_put_buffer_stream_uni(strid_t str, const uint32 *buf, uint len) {
	if (str) {
		str->putBufferUni(buf, len);
	} else {
		warning("put_buffer_stream_uni: invalid ref");
	}
}

int GlkAPI::glk_get_char_stream_uni(strid_t str) {
	if (str) {
		return str->getCharUni();
	} else {
		warning("get_char_stream_uni: invalid ref");
		return -1;
	}
}

uint GlkAPI::glk_get_buffer_stream_uni(strid_t str, uint32 *buf, uint len) {
	if (str) {
		return str->getBufferUni(buf, len);
	} else {
		warning("get_buffer_stream_uni: invalid ref");
		return 0;
	}
}

uint GlkAPI::glk_get_line_stream_uni(strid_t str, uint32 *buf, uint len) {
	if (str) {
		return str->getLineUni(buf, len);
	} else  {
		warning("get_line_stream_uni: invalid ref");
		return (uint) - 1;
	}
}

strid_t GlkAPI::glk_stream_open_file_uni(frefid_t fileref, FileMode fmode, uint rock) {
	return _streams->openFileStream(fileref, fmode, rock, true);
}

strid_t GlkAPI::glk_stream_open_memory_uni(uint32 *buf, uint buflen, FileMode fmode, uint rock) {
	return _streams->openMemoryStream(buf, buflen, fmode, rock, true);
}

void GlkAPI::glk_request_char_event_uni(winid_t win) {
	if (!win) {
		warning("request_char_event_uni: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
	           || win->_lineRequestUni) {
		warning("request_char_event_uni: window already has keyboard request");
	} else {
		win->requestCharEvent();
	}
}

void GlkAPI::glk_request_line_event_uni(winid_t win, uint32 *buf, uint maxlen, uint initlen) {
	if (!win) {
		warning("request_line_event_uni: invalid ref");
	} else if (win->_charRequest || win->_lineRequest || win->_charRequestUni
	           || win->_lineRequestUni) {
		warning("request_line_event_uni: window already has keyboard request");
	} else {
		win->requestLineEventUni(buf, maxlen, initlen);
	}
}

uint GlkAPI::glk_buffer_canon_decompose_uni(uint32 *buf, uint len, uint numchars) {
	// TODO
	return 0;
}

uint GlkAPI::glk_buffer_canon_normalize_uni(uint32 *buf, uint len, uint numchars) {
	return 0;
}

bool GlkAPI::glk_image_draw(winid_t win, uint image, int val1, int val2) {
	return glk_image_draw(win, Common::String::format("%d", image),
		val1, val2);
}

bool GlkAPI::glk_image_draw_scaled(winid_t win, uint image, int val1, int val2,
                                  uint width, uint height) {
	return glk_image_draw_scaled(win, Common::String::format("%d", image),
		val1, val2, width, height);
}

bool GlkAPI::glk_image_draw(winid_t win, const Graphics::Surface &image, uint transColor,
		int xp, int yp) {
	if (!win) {
		warning("image_draw: invalid ref");
	} else if (g_conf->_graphics) {
		GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);

		if (gfxWin)
			gfxWin->drawPicture(image, 0xff, xp, yp, 0, 0);
	}

	return true;
}

bool GlkAPI::glk_image_draw_scaled(winid_t win, const Graphics::Surface &image, uint transColor,
		int xp, int yp, uint width, uint height) {
	if (!win) {
		warning("image_draw_scaled: invalid ref");
	} else if (g_conf->_graphics) {
		if (image.w == width && image.h == height) {
			return glk_image_draw(win, image, transColor, xp, yp);

		} else {
			GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);

			Graphics::ManagedSurface s(width, height, image.format);
			s.transBlitFrom(image, Common::Rect(0, 0, image.w, image.h),
				Common::Rect(0, 0, width, height));

			if (gfxWin)
				gfxWin->drawPicture(s, transColor, xp, yp, s.w, s.h);
		}
	}

	return true;
}

bool GlkAPI::glk_image_draw(winid_t win, const Common::String &image, int val1, int val2) {
	if (!win) {
		warning("image_draw: invalid ref");
	} else if (g_conf->_graphics) {
		TextBufferWindow *textWin = dynamic_cast<TextBufferWindow *>(win);
		GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);

		if (textWin)
			return textWin->drawPicture(image, val1, false, 0, 0);
		else if (gfxWin)
			return gfxWin->drawPicture(image, val1, val2, false, 0, 0);
	}

	return false;
}

bool GlkAPI::glk_image_draw_scaled(winid_t win, const Common::String &image,
		int val1, int val2, uint width, uint height) {
	if (!win) {
		warning("image_draw_scaled: invalid ref");
	} else if (g_conf->_graphics) {
		TextBufferWindow *textWin = dynamic_cast<TextBufferWindow *>(win);
		GraphicsWindow *gfxWin = dynamic_cast<GraphicsWindow *>(win);

		if (textWin)
			return textWin->drawPicture(image, val1, true, width, height);
		else if (gfxWin)
			return gfxWin->drawPicture(image, val1, val2, true, width, height);
	}

	return false;
}

bool GlkAPI::glk_image_get_info(uint image, uint *width, uint *height) {
	return glk_image_get_info(Common::String::format("%u", image),
		width, height);
}

bool GlkAPI::glk_image_get_info(const Common::String &name, uint *width, uint *height) {
	if (!g_conf->_graphics)
		return false;

	Picture *pic = g_vm->_pictures->load(name);
	if (!pic)
		return false;

	if (width)
		*width = pic->w;
	if (height)
		*height = pic->h;

	return true;
}

void GlkAPI::glk_window_flow_break(winid_t win) {
	if (!win) {
		warning("window_erase_rect: invalid ref");
	} else {
		win->flowBreak();
	}
}

void GlkAPI::glk_window_erase_rect(winid_t win, int left, int top, uint width, uint height) {
	if (!win) {
		warning("window_erase_rect: invalid ref");
	} else {
		win->eraseRect(false, Rect(left, top, left + width, top + height));
	}
}

void GlkAPI::glk_window_fill_rect(winid_t win, uint color, int left, int top,
                               uint width, uint height) {
	if (!win) {
		warning("window_fill_rect: invalid ref");
	} else {
		uint c = _conf->parseColor(color);
		win->fillRect(c, Rect(left, top, left + width, top + height));
	}
}

void GlkAPI::glk_window_set_background_color(winid_t win, uint color) {
	if (!win) {
		warning("window_set_background_color: invalid ref");
	} else {
		uint c = _conf->parseColor(color);
		win->setBackgroundColor(c);
	}
}

schanid_t GlkAPI::glk_schannel_create(uint rock) {
	return _sounds->create(rock, GLK_MAXVOLUME);
}

void GlkAPI::glk_schannel_destroy(schanid_t chan) {
	if (chan) {
		delete chan;
	} else {
		warning("schannel_dest roy: invalid ref");
	}
}

schanid_t GlkAPI::glk_schannel_iterate(schanid_t chan, uint *rockptr) {
	return _sounds->iterate(chan, rockptr);
}

uint GlkAPI::glk_schannel_get_rock(schanid_t chan) {
	if (chan) {
		return chan->_rock;
	} else {
		warning("schannel_get_rock: invalid ref");
		return 0;
	}
}

uint GlkAPI::glk_schannel_play(schanid_t chan, uint snd) {
	if (chan) {
		return chan->play(snd);
	} else {
		warning("schannel_play_ext: invalid ref");
		return 0;
	}
}

uint GlkAPI::glk_schannel_play_ext(schanid_t chan, uint snd, uint repeats, uint notify) {
	if (chan) {
		return chan->play(snd, repeats, notify);
	} else {
		warning("schannel_play_ext: invalid ref");
		return 0;
	}
}

void GlkAPI::glk_schannel_stop(schanid_t chan) {
	if (chan) {
		chan->stop();
	} else {
		warning("schannel_stop: invalid ref");
	}
}

void GlkAPI::glk_schannel_set_volume(schanid_t chan, uint vol) {
	if (chan) {
		chan->setVolume(vol);
	} else {
		warning("schannel_set_volume: invalid ref");
	}
}

void GlkAPI::glk_sound_load_hint(uint snd, uint flag) {
	// No implementation
}

schanid_t GlkAPI::glk_schannel_create_ext(uint rock, uint volume) {
	return _sounds->create(rock, volume);
}

uint GlkAPI::glk_schannel_play_multi(schanid_t *chanarray, uint chancount,
                                    uint *sndarray, uint soundcount, uint notify) {
	// No implementation
	return 0;
}

void GlkAPI::glk_schannel_pause(schanid_t chan) {
	if (chan) {
		chan->pause();
	} else {
		warning("schannel_pause: invalid ref");
	}
}

void GlkAPI::glk_schannel_unpause(schanid_t chan) {
	if (chan) {
		chan->unpause();
	} else {
		warning("schannel_unpause: invalid ref");
	}
}

void GlkAPI::glk_schannel_set_volume_ext(schanid_t chan, uint vol,
                                      uint duration, uint notify) {
	if (chan) {
		chan->setVolume(vol, duration, notify);
	} else {
		warning("schannel_set_volume_ext: invalid ref");
	}
}

void GlkAPI::glk_set_hyperlink(uint linkval) {
	_streams->getCurrent()->setHyperlink(linkval);
}

void GlkAPI::glk_set_hyperlink_stream(strid_t str, uint linkval) {
	if (str)
		str->setHyperlink(linkval);
}

void GlkAPI::glk_request_hyperlink_event(winid_t win) {
	if (!win) {
		warning("request_hyperlink_event: invalid ref");
	} else {
		win->requestHyperlinkEvent();
	}
}

void GlkAPI::glk_cancel_hyperlink_event(winid_t win) {
	if (win) {
		win->cancelHyperlinkEvent();
	} else {
		warning("cancel_hyperlink_event: invalid ref");
	}
}

/*--------------------------------------------------------------------------*/

void GlkAPI::glk_current_time(glktimeval_t *time) {
	TimeAndDate td;
	*time = td;
}

int GlkAPI::glk_current_simple_time(uint factor) {
	assert(factor);
	TimeAndDate td;

	return td / factor;
}

void GlkAPI::glk_time_to_date_utc(const glktimeval_t *time, glkdate_t *date) {
	// TODO: timezones aren't currently supported
	*date = TimeAndDate(*time);
}

void GlkAPI::glk_time_to_date_local(const glktimeval_t *time, glkdate_t *date) {
	*date = TimeAndDate(*time);
}

void GlkAPI::glk_simple_time_to_date_utc(int time, uint factor, glkdate_t *date) {
	TimeSeconds secs = (int64)time * factor;
	*date = TimeAndDate(secs);
}

void GlkAPI::glk_simple_time_to_date_local(int time, uint factor, glkdate_t *date) {
	TimeSeconds secs = (int64)time * factor;
	*date = TimeAndDate(secs);
}

void GlkAPI::glk_date_to_time_utc(const glkdate_t *date, glktimeval_t *time) {
	// WORKAROUND: timezones aren't currently supported
	*time = TimeAndDate(*date);
}

void GlkAPI::glk_date_to_time_local(const glkdate_t *date, glktimeval_t *time) {
	*time = TimeAndDate(*date);
}

int GlkAPI::glk_date_to_simple_time_utc(const glkdate_t *date, uint factor) {
	// WORKAROUND: timezones aren't currently supported
	assert(factor);
	TimeSeconds ts = TimeAndDate(*date);
	return ts / factor;
}

int GlkAPI::glk_date_to_simple_time_local(const glkdate_t *date, uint factor) {
	assert(factor);
	TimeSeconds ts = TimeAndDate(*date);
	return ts / factor;
}

/*--------------------------------------------------------------------------*/

/* XXX non-official Glk functions */

const char *GlkAPI::garglk_fileref_get_name(frefid_t fref) const {
	return fref->_filename.c_str();
}

void GlkAPI::garglk_set_program_name(const char *name) {
	// Program name isn't displayed
}

void GlkAPI::garglk_set_program_info(const char *info) {
	// Program info isn't displayed
}

void GlkAPI::garglk_set_story_name(const char *name) {
	// Story name isn't displayed
}

void GlkAPI::garglk_set_story_title(const char *title) {
	// Story title isn't displayed
}

void GlkAPI::garglk_set_config(const char *name) {
	// No implementation
}

void GlkAPI::garglk_unput_string(const char *str) {
	_streams->getCurrent()->unputBuffer(str, strlen(str));
}

void GlkAPI::garglk_unput_string_uni(const uint32 *str) {
	_streams->getCurrent()->unputBufferUni(str, strlen_uni(str));
}

void GlkAPI::garglk_set_zcolors(uint fg, uint bg) {
	_streams->getCurrent()->setZColors(fg, bg);
}

void GlkAPI::garglk_set_zcolors_stream(strid_t str, uint fg, uint bg) {
	if (str) {
		str->setZColors(fg, bg);
	} else {
		warning("set_style_stream: Invalid ref");
	}
}

void GlkAPI::garglk_set_reversevideo(uint reverse) {
	_streams->getCurrent()->setReverseVideo(reverse != 0);
}

void GlkAPI::garglk_set_reversevideo_stream(strid_t str, uint reverse) {
	if (str) {
		str->setReverseVideo(reverse != 0);
	} else {
		warning("set_reversevideo: Invalid ref");
	}
}

void GlkAPI::garglk_window_get_cursor(winid_t win, uint *xpos, uint *ypos) {
	Point pos = win->getCursor();
	*xpos = pos.x;
	*ypos = pos.y;
}

void GlkAPI::garglk_window_get_cursor_current(uint *xpos, uint *ypos) {
	garglk_window_get_cursor(_windows->getFocusWindow(), xpos, ypos);
}

} // End of namespace Glk
