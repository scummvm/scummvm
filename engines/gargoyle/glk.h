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

#ifndef GARGOYLE_GLK_H
#define GARGOYLE_GLK_H

#include "gargoyle/gargoyle.h"
#include "gargoyle/glk_types.h"
#include "gargoyle/windows.h"

namespace Gargoyle {


/**
 * Implements the GLK interface
 */
class Glk : public GargoyleEngine {
private:
	bool _gliFirstEvent;
private:
	/**
	 * Pick first window which might want input. This is called after every keystroke.
	 */
	void gliInputGuessFocus();

	void gliSelect(event_t *event, bool polled);
public:
	/**
	 * Constructor
	 */
	Glk(OSystem *syst, const GargoyleGameDescription *gameDesc);

	void glk_exit(void);
	void glk_set_interrupt_handler(void(*func)(void));
	void glk_tick(void);

	glui32 glk_gestalt(glui32 sel, glui32 val);
	glui32 glk_gestalt_ext(glui32 sel, glui32 val, glui32 *arr, glui32 arrlen);

	unsigned char glk_char_to_lower(unsigned char ch);
	unsigned char glk_char_to_upper(unsigned char ch);

	/**
	 * Get the root window of the window hierarchy
	 */
	winid_t glk_window_get_root(void) const;

	/**
	 * Open a new window
	 */
	winid_t glk_window_open(winid_t split, glui32 method, glui32 size,
		glui32 wintype, glui32 rock = 0) const;

	void glk_window_close(winid_t win, stream_result_t *result);
	void glk_window_get_size(winid_t win, glui32 *widthptr,
		glui32 *heightptr);
	void glk_window_set_arrangement(winid_t win, glui32 method,
		glui32 size, winid_t keywin);
	void glk_window_get_arrangement(winid_t win, glui32 *method,
		glui32 *size, winid_t *keywin);
	winid_t glk_window_iterate(winid_t win, glui32 *rockptr);
	glui32 glk_window_get_rock(winid_t win);
	glui32 glk_window_get_type(winid_t win);
	winid_t glk_window_get_parent(winid_t win);
	winid_t glk_window_get_sibling(winid_t win);
	void glk_window_clear(winid_t win);
	void glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos);

	strid_t glk_window_get_stream(winid_t win);
	void glk_window_set_echo_stream(winid_t win, strid_t str);
	strid_t glk_window_get_echo_stream(winid_t win);
	void glk_set_window(winid_t win);

	strid_t glk_stream_open_file(frefid_t fileref, FileMode fmode, glui32 rock);
	strid_t glk_stream_open_memory(char *buf, glui32 buflen, FileMode fmode, glui32 rock);
	void glk_stream_close(strid_t str, stream_result_t *result);
	strid_t glk_stream_iterate(strid_t str, glui32 *rockptr) const;
	glui32 glk_stream_get_rock(strid_t str) const;
	void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode);
	glui32 glk_stream_get_position(strid_t str);
	void glk_stream_set_current(strid_t str);
	strid_t glk_stream_get_current(void);

	void glk_put_char(unsigned char ch);
	void glk_put_char_stream(strid_t str, unsigned char ch);
	void glk_put_string(const char *s);
	void glk_put_string_stream(strid_t str, const char *s);
	void glk_put_buffer(char *buf, glui32 len);
	void glk_put_buffer_stream(strid_t str, const char *buf, glui32 len);
	void glk_set_style(glui32 styl);
	void glk_set_style_stream(strid_t str, glui32 styl);

	glsi32 glk_get_char_stream(strid_t str);
	glui32 glk_get_line_stream(strid_t str, char *buf, glui32 len);
	glui32 glk_get_buffer_stream(strid_t str, char *buf, glui32 len);

	void glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint,
		glsi32 val);
	void glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint);
	glui32 glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2);
	glui32 glk_style_measure(winid_t win, glui32 styl, glui32 hint,
		glui32 *result);

	frefid_t glk_fileref_create_temp(glui32 usage, glui32 rock);
	frefid_t glk_fileref_create_by_name(glui32 usage, char *name,
		glui32 rock);
	frefid_t glk_fileref_create_by_prompt(glui32 usage, FileMode fmode,
		glui32 rock);
	frefid_t glk_fileref_create_from_fileref(glui32 usage, frefid_t fref,
		glui32 rock);
	void glk_fileref_destroy(frefid_t fref);
	frefid_t glk_fileref_iterate(frefid_t fref, glui32 *rockptr);
	glui32 glk_fileref_get_rock(frefid_t fref);
	void glk_fileref_delete_file(frefid_t fref);
	glui32 glk_fileref_does_file_exist(frefid_t fref);

	void glk_select(event_t *event);
	void glk_select_poll(event_t *event);

	void glk_request_timer_events(glui32 millisecs);

	void glk_request_line_event(winid_t win, char *buf, glui32 maxlen,
		glui32 initlen);
	void glk_request_char_event(winid_t win);
	void glk_request_mouse_event(winid_t win);

	void glk_cancel_line_event(winid_t win, event_t *event);
	void glk_cancel_char_event(winid_t win);
	void glk_cancel_mouse_event(winid_t win);

#ifdef GLK_MODULE_LINE_ECHO
	void glk_set_echo_line_event(winid_t win, glui32 val);
#endif /* GLK_MODULE_LINE_ECHO */

#ifdef GLK_MODULE_LINE_TERMINATORS
	void glk_set_terminators_line_event(winid_t win, glui32 *keycodes,
		glui32 count);
#endif /* GLK_MODULE_LINE_TERMINATORS */

#ifdef GLK_MODULE_UNICODE

	glui32 glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len,
		glui32 numchars);
	glui32 glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len,
		glui32 numchars);
	glui32 glk_buffer_to_title_case_uni(glui32 *buf, glui32 len,
		glui32 numchars, glui32 lowerrest);

	void glk_put_char_uni(glui32 ch);
	void glk_put_string_uni(glui32 *s);
	void glk_put_buffer_uni(glui32 *buf, glui32 len);
	void glk_put_char_stream_uni(strid_t str, glui32 ch);
	void glk_put_string_stream_uni(strid_t str, const glui32 *s);
	void glk_put_buffer_stream_uni(strid_t str, const glui32 *buf, glui32 len);

	glsi32 glk_get_char_stream_uni(strid_t str);
	glui32 glk_get_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len);
	glui32 glk_get_line_stream_uni(strid_t str, glui32 *buf, glui32 len);

	strid_t glk_stream_open_file_uni(frefid_t fileref, FileMode fmode, glui32 rock);
	strid_t glk_stream_open_memory_uni(glui32 *buf, glui32 buflen, FileMode fmode, glui32 rock);

	void glk_request_char_event_uni(winid_t win);
	void glk_request_line_event_uni(winid_t win, glui32 *buf,
		glui32 maxlen, glui32 initlen);

#endif /* GLK_MODULE_UNICODE */

#ifdef GLK_MODULE_UNICODE_NORM

	glui32 glk_buffer_canon_decompose_uni(glui32 *buf, glui32 len,
		glui32 numchars);
	glui32 glk_buffer_canon_normalize_uni(glui32 *buf, glui32 len,
		glui32 numchars);

#endif /* GLK_MODULE_UNICODE_NORM */

#ifdef GLK_MODULE_IMAGE

	glui32 glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2);
	glui32 glk_image_draw_scaled(winid_t win, glui32 image,
		glsi32 val1, glsi32 val2, glui32 width, glui32 height);
	glui32 glk_image_get_info(glui32 image, glui32 *width, glui32 *height);

	void glk_window_flow_break(winid_t win);

	void glk_window_erase_rect(winid_t win,
		glsi32 left, glsi32 top, glui32 width, glui32 height);
	void glk_window_fill_rect(winid_t win, glui32 color,
		glsi32 left, glsi32 top, glui32 width, glui32 height);
	void glk_window_set_background_color(winid_t win, glui32 color);

#endif /* GLK_MODULE_IMAGE */

#ifdef GLK_MODULE_SOUND

	schanid_t glk_schannel_create(glui32 rock);
	void glk_schannel_destroy(schanid_t chan);
	schanid_t glk_schannel_iterate(schanid_t chan, glui32 *rockptr);
	glui32 glk_schannel_get_rock(schanid_t chan);

	glui32 glk_schannel_play(schanid_t chan, glui32 snd);
	glui32 glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats,
		glui32 notify);
	void glk_schannel_stop(schanid_t chan);
	void glk_schannel_set_volume(schanid_t chan, glui32 vol);

	void glk_sound_load_hint(glui32 snd, glui32 flag);

#ifdef GLK_MODULE_SOUND2
	/* Note that this section is nested inside the #ifdef GLK_MODULE_SOUND.
	GLK_MODULE_SOUND must be defined if GLK_MODULE_SOUND2 is. */

	schanid_t glk_schannel_create_ext(glui32 rock, glui32 volume);
	glui32 glk_schannel_play_multi(schanid_t *chanarray, glui32 chancount,
		glui32 *sndarray, glui32 soundcount, glui32 notify);
	void glk_schannel_pause(schanid_t chan);
	void glk_schannel_unpause(schanid_t chan);
	void glk_schannel_set_volume_ext(schanid_t chan, glui32 vol,
		glui32 duration, glui32 notify);

#endif /* GLK_MODULE_SOUND2 */
#endif /* GLK_MODULE_SOUND */

#ifdef GLK_MODULE_HYPERLINKS

	void glk_set_hyperlink(glui32 linkval);
	void glk_set_hyperlink_stream(strid_t str, glui32 linkval);
	void glk_request_hyperlink_event(winid_t win);
	void glk_cancel_hyperlink_event(winid_t win);

#endif /* GLK_MODULE_HYPERLINKS */

#ifdef GLK_MODULE_DATETIME

	void glk_current_time(glktimeval_t *time);
	glsi32 glk_current_simple_time(glui32 factor);
	void glk_time_to_date_utc(glktimeval_t *time, glkdate_t *date);
	void glk_time_to_date_local(glktimeval_t *time, glkdate_t *date);
	void glk_simple_time_to_date_utc(glsi32 time, glui32 factor,
		glkdate_t *date);
	void glk_simple_time_to_date_local(glsi32 time, glui32 factor,
		glkdate_t *date);
	void glk_date_to_time_utc(glkdate_t *date, glktimeval_t *time);
	void glk_date_to_time_local(glkdate_t *date, glktimeval_t *time);
	glsi32 glk_date_to_simple_time_utc(glkdate_t *date, glui32 factor);
	glsi32 glk_date_to_simple_time_local(glkdate_t *date, glui32 factor);

#endif /* GLK_MODULE_DATETIME */

	/* XXX non-official Glk functions that may or may not exist */
	#define GARGLK 1

	char* garglk_fileref_get_name(frefid_t fref);

	void garglk_set_program_name(const char *name);
	void garglk_set_program_info(const char *info);
	void garglk_set_story_name(const char *name);
	void garglk_set_story_title(const char *title);
	void garglk_set_config(const char *name);

	/* garglk_unput_string - removes the specified string from the end of the output buffer, if
	* indeed it is there. */
	void garglk_unput_string(char *str);
	void garglk_unput_string_uni(glui32 *str);

	void garglk_set_zcolors(glui32 fg, glui32 bg);
	void garglk_set_zcolors_stream(strid_t str, glui32 fg, glui32 bg);
	void garglk_set_reversevideo(glui32 reverse);
	void garglk_set_reversevideo_stream(strid_t str, glui32 reverse);
};

} // End of namespace Gargoyle

#endif
