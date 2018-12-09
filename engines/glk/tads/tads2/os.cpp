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

#include "glk/tads/tads2/os.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

OS::OS(OSystem *syst, const GlkGameDescription &gameDesc) : TADS(syst, gameDesc),
		status_mode(0) {
	Common::fill(&status_left[0], &status_left[OSS_STATUS_STRING_LEN], '\0');
	Common::fill(&status_right[0], &status_right[OSS_STATUS_STRING_LEN], '\0');
}

void OS::os_terminate(int rc) {
	glk_exit();
}

uint OS::oss_convert_prompt_type(int type) {
	if (type == OS_AFP_OPEN)
		return filemode_Read;
	return filemode_ReadWrite;
}

uint OS::oss_convert_file_type(int type) {
	if (type == OSFTSAVE)
		return fileusage_SavedGame;
	if (type == OSFTLOG || type == OSFTTEXT)
		return fileusage_Transcript;
	return fileusage_Data;
}

uint OS::oss_convert_fileref_to_string(frefid_t file_to_convert, char *buffer, int buf_len) {
#ifdef TODO
	char   temp_string[32];
	uint value, i = 0, digit,
		digit_flag = false,     // Have we put a digit in the string yet?
		divisor = 1e9;          // The max 32-bit integer is 4294967295

	// This could probably be done by using sprintf("%s%ld%s") but I don't want to risk it
	value = (uint)file_to_convert;
	while (divisor != 1) {
		digit = (char)(value / divisor);
		if (digit != 0 || digit_flag) {     // This lets us handle, eg, 102
			temp_string[i++] = digit + '0';
			digit_flag = true;
		}
		value = value % divisor;
		divisor /= 10;
	}
	temp_string[i++] = (char)value + '0';
	temp_string[i] = 0;
	if (strlen(temp_string) + strlen(OSS_FILEREF_STRING_PREFIX) +
		strlen(OSS_FILEREF_STRING_SUFFIX) > (size_t)buf_len)
		return false;
	sprintf(buffer, "%s%s%s", OSS_FILEREF_STRING_PREFIX,
		temp_string, OSS_FILEREF_STRING_SUFFIX);
#endif
	return true;
}

frefid_t OS::oss_convert_string_to_fileref(char *buffer, uint usage) {
#ifdef TODO
	char temp_string[32];
	uint value = 0, i, multiplier = 1;

	// Does the buffer contain a hashed fileref?
	if (oss_is_string_a_fileref(buffer)) {
		// If so, we need only decode the string in the middle and return its value
		strcpy(temp_string, buffer + strlen(OSS_FILEREF_STRING_PREFIX));
		i = strlen(temp_string) - strlen(OSS_FILEREF_STRING_SUFFIX);
		temp_string[i] = 0;
		while (i != 0) {
			i--;
			value += ((uint)(temp_string[i] - '0') * multiplier);
			multiplier *= 10;
		}
		return ((frefid_t)value);
	}

	// If not, return the new fileref
	return (glk_fileref_create_by_name(usage, os_get_root_name(buffer), 0));
#else
	return nullptr;
#endif
}

bool OS::oss_is_string_a_fileref(char *buffer) {
	if ((strncmp(buffer, OSS_FILEREF_STRING_PREFIX,
		strlen(OSS_FILEREF_STRING_PREFIX)) == 0) &&
		(strncmp(buffer + strlen(buffer) - strlen(OSS_FILEREF_STRING_SUFFIX),
			OSS_FILEREF_STRING_SUFFIX,
			strlen(OSS_FILEREF_STRING_SUFFIX)) == 0))
		return true;
	return false;
}

unsigned char OS::oss_convert_keystroke_to_tads(uint key) {
	// Characters 0 - 255 we return per normal */
	if (key <= 255)
		return ((unsigned char)key);
	switch (key) {
	case keycode_Up:
		return CMD_UP;
	case keycode_Down:
		return CMD_DOWN;
	case keycode_Left:
		return CMD_LEFT;
	case keycode_Right:
		return CMD_RIGHT;
	case keycode_PageUp:
		return CMD_PGUP;
	case keycode_PageDown:
		return CMD_PGDN;
	case keycode_Home:
		return CMD_HOME;
	case keycode_End:
		return CMD_END;
	case keycode_Func1:
		return CMD_F1;
	case keycode_Func2:
		return CMD_F2;
	case keycode_Func3:
		return CMD_F3;
	case keycode_Func4:
		return CMD_F4;
	case keycode_Func5:
		return CMD_F5;
	case keycode_Func6:
		return CMD_F6;
	case keycode_Func7:
		return CMD_F7;
	case keycode_Func8:
		return CMD_F8;
	case keycode_Func9:
		return CMD_F9;
	case keycode_Func10:
		return CMD_F10;
	default:
		return 0;
	}
}

bool OS::oss_check_path(char *filename) {
	return false;
}

void OS::oss_revert_path() {
	// No implementation
}

osfildef *OS::oss_open_stream(char *buffer, uint tadsusage, uint tbusage,
		uint fmode, uint rock) {
	frefid_t fileref;
	strid_t osf;
	int      changed_dirs;

	fileref = oss_convert_string_to_fileref(buffer,
		oss_convert_file_type(tadsusage) | tbusage);
	changed_dirs = oss_check_path(buffer);
	osf = glk_stream_open_file(fileref, (FileMode)fmode, rock);
	if (changed_dirs)
		oss_revert_path();
	return *osf;
}

osfildef *OS::osfoprb(const char *fname, uint typ) {
	Common::File *f = new Common::File();
	if (f->open(fname))
		return f;

	f->close();
	delete f;
	return nullptr;
}

void OS::os_gen_charmap_filename(char *filename, const char *internal_id,
		const char *argv0) {
	const char *p;
	const char *rootname;
	size_t pathlen;

	// find the path prefix of the original executable filename
	for (p = rootname = argv0; *p != '\0'; ++p) {
		if (*p == '/' || *p == '\\' || *p == ':')
			rootname = p + 1;
	}

	// copy the path prefix
	pathlen = rootname - argv0;
	memcpy(filename, argv0, pathlen);

	// if there's no trailing backslash, add one
	if (pathlen == 0 || filename[pathlen - 1] != '\\')
		filename[pathlen++] = '\\';

	// add "win_", plus the character set ID, plus the extension
	strcpy(filename + pathlen, "win_");
	strcat(filename + pathlen, internal_id);
	strcat(filename + pathlen, ".tcp");
}

void OS::oss_put_string_with_hilite(winid_t win, const char *str, size_t len) {
	glk_set_window(win);
	glk_put_buffer(str, len);
}

void OS::oss_draw_status_line(void) {
	uint width, height, division;

	if (status_win == nullptr) return;  // In case this is a CheapGlk port

	glk_window_get_size(status_win, &width, &height);
	if (height == 0) return;  // Don't bother if status is invisible
	division = width - strlen(status_right) - 1;
	glk_set_window(status_win);
	glk_window_clear(status_win);
	oss_put_string_with_hilite(status_win, status_left, strlen(status_left));
	glk_window_move_cursor(status_win, division, 0);
	glk_put_string(status_right);
}

void OS::oss_change_status_string(char *dest, const char *src, size_t len) {
	if (len > OSS_STATUS_STRING_LEN - 1)
		len = OSS_STATUS_STRING_LEN - 1;
	memcpy(dest, src, len);
	dest[len] = '\0';
}

void OS::oss_change_status_left(const char *str, size_t len) {
	oss_change_status_string(status_left, str, len);
	oss_draw_status_line();
}

void OS::oss_change_status_right(const char *str) {
	oss_change_status_string(status_right, str, strlen(str));
	oss_draw_status_line();
}

int OS::memicmp(const char *s1, const char *s2, int len) {
	char *x1, *x2;
	int i, result;

	x1 = (char *)malloc(len); x2 = (char *)malloc(len);

	if (!x1 || !x2) {
		glk_set_window(story_win);
		glk_put_string("memicmp has run out of memory. Quitting.\n");
		os_waitc();
		glk_exit();
	}

	for (i = 0; i < len; i++) {
		if (Common::isUpper(s1[i]))
			x1[i] = glk_char_to_lower((unsigned char)s1[i]);
		else x1[i] = s1[i];

		if (Common::isUpper(s2[i]))
			x2[i] = glk_char_to_lower((unsigned char)s2[i]);
		else x2[i] = s2[i];
	}

	result = memcmp(x1, x2, len);
	free(x1);
	free(x2);
	return result;
}

void OS::os_flush() {
	glk_tick();
}

void OS::os_print(const char *str, size_t len) {
	int current_status_mode;

	// Decide what to do based on our status mode
	current_status_mode = os_get_status();
	if (current_status_mode == OSS_STATUS_MODE_STORY) {
		oss_put_string_with_hilite(story_win, str, len);
	} else if (current_status_mode == OSS_STATUS_MODE_STATUS) {
		const char *p;
		size_t      rem;

		// The string requires some fiddling for the status window
		for (p = str, rem = len; rem != 0 && *p == '\n'; p++, --rem);
		if (rem != 0 && p[rem - 1] == '\n')
			--rem;

		// if that leaves anything, update the statusline
		if (rem != 0)
			oss_change_status_left(p, rem);
	}
}

void OS::os_expause() {
	os_printz("(Strike any key to exit...)");
	os_flush();
	os_waitc();
}

int OS::oss_getc_from_window(winid_t win) {
	static unsigned char buffered_char = 0;
	int i;
	event_t ev;

	if (buffered_char != 0) {
		i = (int)buffered_char;
		buffered_char = 0;
		return i;
	}
	glk_request_char_event(win);
	do {
		glk_select(&ev);
		if (ev.type == evtype_Arrange)
			oss_draw_status_line();
	} while (ev.type != evtype_CharInput);
	if (ev.val1 == keycode_Return)
		ev.val1 = '\n';
	else if (ev.val1 == keycode_Tab)
		ev.val1 = '\t';
	if (ev.val1 <= 255)
		return ((int)ev.val1);

	// We got a special character, so handle it appropriately
	buffered_char = oss_convert_keystroke_to_tads(ev.val1);
	return 0;
}

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk
