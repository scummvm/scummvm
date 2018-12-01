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

#ifndef GLK_TADS_TADS2_OS
#define GLK_TADS_TADS2_OS

#include "glk/tads/tads.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/**
 * Allocate a memory block
 */
#define mchalo(CTX, SIZE, COMMENT) ((byte *)new byte[SIZE])

/**
 * Free a memory block
 */
#define mchfre(PTR) delete[] (byte *)PTR


/**
 * The character (or characters) which mark the beginning of a special fileref string.
 * The important thing is that the string be one that is either not allowed in
 * filenames on your platform or is unlikely to be the first part of a filename.
 */
#define OSS_FILEREF_STRING_PREFIX ":"

/**
 * The character (or characters) which mark the end of a special fileref string.
 * Using this and OSS_FILEREF_STRING_PREFIX, you should be able to come up with
 * something which forms an invalid filename
 */
#define OSS_FILEREF_STRING_SUFFIX ""

/**
 * Maximum length of status line text
 */
#define OSS_STATUS_STRING_LEN 80

/**
 * Important note: do not change these values when porting TADS.  These
 * values can be used by games, so they must be the same on all platforms.  
 */
enum {
	OS_AFP_OPEN = 1,			///< choose an existing file to open for reading
	OS_AFP_SAVE = 2				///< choose a filename for saving to a file
};

/**
 * File types.These type codes are used when opening or creating a file,
 * so that the OS routine can set appropriate file system metadata
 * to describe or find the file type.
 *
 * The type os_filetype_t is defined for documentary purposes; it's
 * always just an int.
 */
enum os_filetype_t {
	OSFTGAME  =  0,		///< a game data file (.gam)
	OSFTSAVE  =  1,     ///< a saved game (.sav)
	OSFTLOG   =  2,     ///< a transcript (log) file
	OSFTSWAP  =  3,     ///< swap file
	OSFTDATA  =  4,     ///< user data file (used with the TADS fopen() call)
	OSFTCMD   =  5,     ///< QA command/log file
	OSFTERRS  =  6,     ///< error message file
	OSFTTEXT  =  7,     ///< text file - used for source files
	OSFTBIN   =  8,     ///< binary file of unknown type - resources, etc
	OSFTCMAP  =  9,     ///< character mapping file
	OSFTPREF  = 10,     ///< preferences file
	OSFTUNK   = 11,     ///< unknown - as a filter, matches any file type
	OSFTT3IMG = 12,     ///< T3 image file (.t3 - formerly .t3x)
	OSFTT3OBJ = 13,     ///< T3 object file (.t3o)
	OSFTT3SYM = 14,     ///< T3 symbol export file (.t3s)
	OSFTT3SAV = 15      ///< T3 saved state file (.t3v)
};

/**
 * Constants for os_getc() when returning commands.  When used for command line
 * editing, special keys (arrows, END, etc.)  should cause os_getc() to return 0,
 * and return the appropriate CMD_ value on the NEXT call.  Hence, os_getc() must
 * keep the appropriate information around statically for the next call when a
 * command key is issued.
 *
 * The comments indicate which CMD_xxx codes are "translated" codes and which are
 * "raw"; the difference is that, when a particular keystroke could be interpreted
 * as two different CMD_xxx codes, one translated and the other raw, os_getc()
 * should always return the translated version of the key, and os_getc_raw()
 * should return the raw version.
 */
enum KeyCmd {
	CMD_UP    =  1,		///< move up/up arrow (translated)
	CMD_DOWN  =  2,		///< move down/down arrow (translated)
	CMD_RIGHT =  3,		///< move right/right arrow (translated)
	CMD_LEFT  =  4,		///< move left/left arrow (translated)
	CMD_END   =  5,		///< move cursor to end of line (translated)
	CMD_HOME  =  6,		///< move cursor to start of line (translated)
	CMD_DEOL  =  7,		///< delete to end of line (translated)
	CMD_KILL  =  8,		///< delete entire line (translated)
	CMD_DEL   =  9,		///< delete current character (translated)
	CMD_SCR   = 10,		///< toggle scrollback mode (translated)
	CMD_PGUP  = 11,		///< page up (translated)
	CMD_PGDN  = 12,		///< page down (translated)
	CMD_TOP   = 13,		///< top of file (translated)
	CMD_BOT   = 14,		///< bottom of file (translated)
	CMD_F1    = 15,		///< function key F1 (raw)
	CMD_F2    = 16,		///< function key F2 (raw)
	CMD_F3    = 17,		///< function key F3 (raw)
	CMD_F4    = 18,		///< function key F4 (raw)
	CMD_F5    = 19,		///< function key F5 (raw)
	CMD_F6    = 20,		///< function key F6 (raw)
	CMD_F7    = 21,		///< function key F7 (raw)
	CMD_F8    = 22,		///< function key F8 (raw)
	CMD_F9    = 23,		///< function key F9 (raw)
	CMD_F10   = 24,		///< function key F10 (raw)
	CMD_CHOME = 25,		///< control-home (raw)
	CMD_TAB   = 26,		///< tab (translated)
	CMD_SF2   = 27,		///< shift-F2 (raw)
	///< not used (obsolete) - 28
	CMD_WORD_LEFT  = 29,	///< word left (ctrl-left on dos) (translated)
	CMD_WORD_RIGHT = 30,	///< word right (ctrl-right on dos) (translated)
	CMD_WORDKILL   = 31,	///< delete word right (translated)
	CMD_EOF        = 32,	///< end-of-file (raw)
	CMD_BREAK      = 33,	///< break (Ctrl-C or local equivalent) (translated)
	CMD_INS        = 34,	///< insert key (raw)

	/**
	 * ALT-keys - add alphabetical code to CMD_ALT: ALT-A == CMD_ALT + 0,
	 * ALT-B == CMD_ALT + 1, ALT-C == CMD_ALT + 2, etc
	 *
	 * These keys are all raw (untranslated).
	 */
	CMD_ALT   = 128		///< start of ALT keys
};

/**
 * Status mode codes
 */
enum StatusMode {
	OSS_STATUS_MODE_STORY = 0,
	OSS_STATUS_MODE_STATUS = 1
};

typedef Common::SeekableReadStream osfildef;

/**
 * Operating system compatibility layer
 */
class OS : public TADS {
protected:
	char status_left[OSS_STATUS_STRING_LEN];
	char status_right[OSS_STATUS_STRING_LEN];
	int status_mode;
protected:
	/**
	 * Constructor
	 */
	OS(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Terminates the game
	 */
	void os_terminate(int rc);

	/**
	 * \defgroup Type Conversions
	 * @{
	 */

	/**
	 * Change a TADS prompt type (OS_AFP_*) into a Glk prompt type.
	 */
	glui32 oss_convert_prompt_type(int type);
	
	/**
	 * Change a TADS file type (OSFT*) into a Glk file type.
	 */
	glui32 oss_convert_file_type(int type);

	/**
	 * Change a fileref ID (frefid_t) to a special string and put it in the
	 * buffer which is passed to it. The string is given by
	 *   OSS_FILEREF_STRING_PREFIX + 'nnnnn' + OSS_FILEREF_STRING_SUFFIX
	 * where 'nnnnn' is the frefid_t pointer converted into a string of decimal
	 * numbers. This is only really practical for 32-bit pointers; if we use
	 * 64-bit pointers I'll have to start using a hash table or use hex
	 * numbers.
	 */
	glui32 oss_convert_fileref_to_string(frefid_t file_to_convert, char *buffer, int buf_len);

	/**
	 * Turn a filename or a special fileref string into an actual fileref.
	 * Notice that, since Glk doesn't know paths, we take this opportunity to
	 * call oss_check_path, which should do the OS-dependent path changing
	 * in the event that the filename contains path information
	 */
	frefid_t oss_convert_string_to_fileref(char *buffer, glui32 usage);

	/**
	 * Tell us if the passed string is a hashed fileref or not
	 */
	bool oss_is_string_a_fileref(char *buffer);

	/**
	 * Change a Glk key into a TADS one, using the CMD_xxx codes
	 */
	unsigned char oss_convert_keystroke_to_tads(glui32 key);

	/**@}*/

	/**
	 * \defgroup Directory/File methods
	 * @{
	 */

	/**
	 * If a filename contains path information, change dirs to that path.
	 * Returns true if the path was fiddled with
	 */
	bool oss_check_path(char *filename);

	/**
	 * In case we changed directories in oss_check_path, change back to the
	 * original executable directory
	 */
	void oss_revert_path();

	/**
	 * Open a stream, given a string, usage, and a filemode. tadsusage is the
	 * TADS filemode (OSFT*); tbusage is either fileusage_TextMode or
	 * fileusage_BinaryMode (from Glk).
	 */
	osfildef *oss_open_stream(char *buffer, glui32 tadsusage, glui32 tbusage,
		glui32 fmode, glui32 rock);

	/**
	 * Get a pointer to the root name portion of a filename.  This is the part
	 * of the filename after any path or directory prefix.  For example, on
	 * Unix, given the string "/home/mjr/deep.gam", this function should return
	 * a pointer to the 'd' in "deep.gam".  If the filename doesn't appear to
	 * have a path prefix, it should simply return the argument unchanged.
	 */
	const char *os_get_root_name(const char *buf) const { return buf; }

	/**@}*/

	/**
	 * \defgroup The main text area print routines
	 * @{
	 */

	/**
	 * Process hilighting codes while printing a string
	 */
	void oss_put_string_with_hilite(winid_t win, const char *str, size_t len);

	/**
	 * Status line handling
	 */
	void oss_draw_status_line();

	void oss_change_status_string(char *dest, const char *src, size_t len);
	void oss_change_status_left(const char *str, size_t len);
	void oss_change_status_right(const char *str);
	int os_get_status() const { return status_mode; }

	/**
	 * Flush the output
	 */
	void os_flush();

	/**
	 * Print a null terminated string
	 */
	void os_printz(const char *str) { os_print(str, strlen(str)); }

	/**
	 * Print a string
	 */
	void os_print(const char *str, size_t len);

	/**@}*/

	/**
	 * \defgroup Memory routines
	 * @{
	 */

	/**
	 * Compare two strings
	 */
	int memicmp(const char *s1, const char *s2, int len);

	/**@}*/

	/**
	 * \defgroup Input routines
	 * @{
	 */

	/**
	 * Wait for a key to be hit
	 */
	void os_waitc(void) { os_getc(); }

	/**
	 * Get a character from the keyboard. For extended characters, return 0,
	 * then return the extended key at the next call to this function
	 */
	int os_getc() {
		return oss_getc_from_window(story_win);
	}

	/**
	 * Accept a keystroke in the passed window
	 */
	int oss_getc_from_window(winid_t win);


	/**
	 * Print a message (with os_print) and wait for a key
	 */
	void os_expause();

	/**@}*/
};

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
