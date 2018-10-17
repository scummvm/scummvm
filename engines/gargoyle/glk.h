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

#include "graphics/managed_surface.h"
#include "gargoyle/interpreter.h"

namespace Gargoyle {

typedef uint32 glui32;
typedef int32 glsi32;

/**
 * These are the compile-time conditionals that reveal various Glk optional modules.
 */
#define GLK_MODULE_LINE_ECHO
#define GLK_MODULE_LINE_TERMINATORS
#define GLK_MODULE_UNICODE
#define GLK_MODULE_UNICODE_NORM
#define GLK_MODULE_IMAGE
#define GLK_MODULE_SOUND
#define GLK_MODULE_SOUND2
#define GLK_MODULE_HYPERLINKS
#define GLK_MODULE_DATETIME
#define GLK_MODULE_GARGLKTEXT

/**
 * These types are opaque object identifiers. They're pointers to opaque
 * C structures, which are defined differently by each library.
 */
typedef struct glk_window_struct  *winid_t;
typedef struct glk_stream_struct  *strid_t;
typedef struct glk_fileref_struct *frefid_t;
typedef struct glk_schannel_struct *schanid_t;

enum Gestalt {
	gestalt_Version                = 0,
	gestalt_CharInput              = 1,
	gestalt_LineInput              = 2,
	gestalt_CharOutput             = 3,
	gestalt_CharOutput_CannotPrint = 0,
	gestalt_CharOutput_ApproxPrint = 1,
	gestalt_CharOutput_ExactPrint  = 2,
	gestalt_MouseInput             = 4,
	gestalt_Timer                  = 5,
	gestalt_Graphics               = 6,
	gestalt_DrawImage              = 7,
	gestalt_Sound                  = 8,
	gestalt_SoundVolume            = 9,
	gestalt_SoundNotify            = 10,
	gestalt_Hyperlinks             = 11,
	gestalt_HyperlinkInput         = 12,
	gestalt_SoundMusic             = 13,
	gestalt_GraphicsTransparency   = 14,
	gestalt_Unicode                = 15,
	gestalt_UnicodeNorm            = 16,
	gestalt_LineInputEcho          = 17,
	gestalt_LineTerminators        = 18,
	gestalt_LineTerminatorKey      = 19,
	gestalt_DateTime               = 20,
	gestalt_Sound2                 = 21,
	gestalt_GarglkText             = 0x1100,
};

enum EvType {
	evtype_None         = 0,
	evtype_Timer        = 1,
	evtype_CharInput    = 2,
	evtype_LineInput    = 3,
	evtype_MouseInput   = 4,
	evtype_Arrange      = 5,
	evtype_Redraw       = 6,
	evtype_SoundNotify  = 7,
	evtype_Hyperlink    = 8,
	evtype_VolumeNotify = 9,
};

enum Keycode {
	keycode_Unknown  = 0xffffffffU,
	keycode_Left     = 0xfffffffeU,
	keycode_Right    = 0xfffffffdU,
	keycode_Up       = 0xfffffffcU,
	keycode_Down     = 0xfffffffbU,
	keycode_Return   = 0xfffffffaU,
	keycode_Delete   = 0xfffffff9U,
	keycode_Escape   = 0xfffffff8U,
	keycode_Tab      = 0xfffffff7U,
	keycode_PageUp   = 0xfffffff6U,
	keycode_PageDown = 0xfffffff5U,
	keycode_Home     = 0xfffffff4U,
	keycode_End      = 0xfffffff3U,
	keycode_Func1    = 0xffffffefU,
	keycode_Func2    = 0xffffffeeU,
	keycode_Func3    = 0xffffffedU,
	keycode_Func4    = 0xffffffecU,
	keycode_Func5    = 0xffffffebU,
	keycode_Func6    = 0xffffffeaU,
	keycode_Func7    = 0xffffffe9U,
	keycode_Func8    = 0xffffffe8U,
	keycode_Func9    = 0xffffffe7U,
	keycode_Func10   = 0xffffffe6U,
	keycode_Func11   = 0xffffffe5U,
	keycode_Func12   = 0xffffffe4U,

	// non standard keycodes
	keycode_Erase          = 0xffffef7fU,
	keycode_MouseWheelUp   = 0xffffeffeU,
	keycode_MouseWheelDown = 0xffffefffU,
	keycode_SkipWordLeft   = 0xfffff000U,
	keycode_SkipWordRight  = 0xfffff001U,

	// The last keycode is always = 0x100000000 - keycode_MAXVAL)
	keycode_MAXVAL = 28U
};

enum Style {
	style_Normal       = 0,
	style_Emphasized   = 1,
	style_Preformatted = 2,
	style_Header       = 3,
	style_Subheader    = 4,
	style_Alert        = 5,
	style_Note         = 6,
	style_BlockQuote   = 7,
	style_Input        = 8,
	style_User1        = 9,
	style_User2        = 10,
	style_NUMSTYLES    = 11,
};

enum WinType {
	wintype_AllTypes   = 0,
	wintype_Pair       = 1,
	wintype_Blank      = 2,
	wintype_TextBuffer = 3,
	wintype_TextGrid   = 4,
	wintype_Graphics   = 5,
};

enum WinMethod {
	winmethod_Left    = 0x00,
	winmethod_Right   = 0x01,
	winmethod_Above   = 0x02,
	winmethod_Below   = 0x03,
	winmethod_DirMask = 0x0f,

	winmethod_Fixed        = 0x10,
	winmethod_Proportional = 0x20,
	winmethod_DivisionMask = 0xf0,

	winmethod_Border     = 0x000,
	winmethod_NoBorder   = 0x100,
	winmethod_BorderMask = 0x100,
};

enum FileUsage {
	fileusage_Data        = 0x00,
	fileusage_SavedGame   = 0x01,
	fileusage_Transcript  = 0x02,
	fileusage_InputRecord = 0x03,
	fileusage_TypeMask    = 0x0f,

	fileusage_TextMode    = 0x100,
	fileusage_BinaryMode  = 0x000,
};

enum FileMode {
	filemode_Write       = 0x01,
	filemode_Read        = 0x02,
	filemode_ReadWrite   = 0x03,
	filemode_WriteAppend = 0x05,
};

enum SeekMode {
	seekmode_Start   = 0,
	seekmode_Current = 1,
	seekmode_End     = 2,
};

enum StyleHint {
	stylehint_Indentation     = 0,
	stylehint_ParaIndentation = 1,
	stylehint_Justification  = 2,
	stylehint_Size            = 3,
	stylehint_Weight          = 4,
	stylehint_Oblique         = 5,
	stylehint_Proportional    = 6,
	stylehint_TextColor       = 7,
	stylehint_BackColor       = 8,
	stylehint_ReverseColor    = 9,
	stylehint_NUMHINTS        = 10,

	stylehint_just_LeftFlush  = 0,
	stylehint_just_LeftRight  = 1,
	stylehint_just_Centered   = 2,
	stylehint_just_RightFlush = 3,
};

#ifdef GLK_MODULE_IMAGE

enum ImageAlign {
	imagealign_InlineUp     = 1,
	imagealign_InlineDown   = 2,
	imagealign_InlineCenter = 3,
	imagealign_MarginLeft   = 4,
	imagealign_MarginRight  = 5
};

#endif /* GLK_MODULE_IMAGE */

struct event_struct {
	glui32 type;
	winid_t win;
	glui32 val1, val2;
};
typedef event_struct *event_t;

struct stream_result_struct {
	glui32 readcount;
	glui32 writecount;
};
typedef stream_result_struct *stream_result_t;

#ifdef GLK_MODULE_DATETIME

struct glktimeval_struct {
	glsi32 high_sec;
	glui32 low_sec;
	glsi32 microsec;
};
typedef glktimeval_struct *glktimeval_t;

struct glkdate_struct {
	glsi32 year;     /* full (four-digit) year */
	glsi32 month;    /* 1-12, 1 is January */
	glsi32 day;      /* 1-31 */
	glsi32 weekday;  /* 0-6, 0 is Sunday */
	glsi32 hour;     /* 0-23 */
	glsi32 minute;   /* 0-59 */
	glsi32 second;   /* 0-59, maybe 60 during a leap second */
	glsi32 microsec; /* 0-999999 */
};
typedef glkdate_struct *glkdate_t;

#endif /* GLK_MODULE_DATETIME */

/**
 * Implements the GLK interface
 */
class Glk : public Interpreter {
private:
	Graphics::ManagedSurface *_surface;
public:
	/**
	 * Constructor
	 */
	Glk() {}

	void glk_exit(void);
	void glk_set_interrupt_handler(void(*func)(void));
	void glk_tick(void);

	glui32 glk_gestalt(glui32 sel, glui32 val);
	glui32 glk_gestalt_ext(glui32 sel, glui32 val, glui32 *arr, glui32 arrlen);

	unsigned char glk_char_to_lower(unsigned char ch);
	unsigned char glk_char_to_upper(unsigned char ch);

	winid_t glk_window_get_root(void);
	winid_t glk_window_open(winid_t split, glui32 method, glui32 size,
		glui32 wintype, glui32 rock);
	void glk_window_close(winid_t win, stream_result_t *result);
	void glk_window_get_size(winid_t win, glui32 *widthptr,
		glui32 *heightptr);
	void glk_window_set_arrangement(winid_t win, glui32 method,
		glui32 size, winid_t keywin);
	void glk_window_get_arrangement(winid_t win, glui32 *methodptr,
		glui32 *sizeptr, winid_t *keywinptr);
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

	strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode,
		glui32 rock);
	strid_t glk_stream_open_memory(char *buf, glui32 buflen, glui32 fmode,
		glui32 rock);
	void glk_stream_close(strid_t str, stream_result_t *result);
	strid_t glk_stream_iterate(strid_t str, glui32 *rockptr);
	glui32 glk_stream_get_rock(strid_t str);
	void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode);
	glui32 glk_stream_get_position(strid_t str);
	void glk_stream_set_current(strid_t str);
	strid_t glk_stream_get_current(void);

	void glk_put_char(unsigned char ch);
	void glk_put_char_stream(strid_t str, unsigned char ch);
	void glk_put_string(char *s);
	void glk_put_string_stream(strid_t str, char *s);
	void glk_put_buffer(char *buf, glui32 len);
	void glk_put_buffer_stream(strid_t str, char *buf, glui32 len);
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
	frefid_t glk_fileref_create_by_prompt(glui32 usage, glui32 fmode,
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
	void glk_put_string_stream_uni(strid_t str, glui32 *s);
	void glk_put_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len);

	glsi32 glk_get_char_stream_uni(strid_t str);
	glui32 glk_get_buffer_stream_uni(strid_t str, glui32 *buf, glui32 len);
	glui32 glk_get_line_stream_uni(strid_t str, glui32 *buf, glui32 len);

	strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode,
		glui32 rock);
	strid_t glk_stream_open_memory_uni(glui32 *buf, glui32 buflen,
		glui32 fmode, glui32 rock);

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
