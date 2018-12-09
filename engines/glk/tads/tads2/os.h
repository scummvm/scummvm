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
#include "glk/tads/tads2/types.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

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
	uint oss_convert_prompt_type(int type);
	
	/**
	 * Change a TADS file type (OSFT*) into a Glk file type.
	 */
	uint oss_convert_file_type(int type);

	/**
	 * Change a fileref ID (frefid_t) to a special string and put it in the
	 * buffer which is passed to it. The string is given by
	 *   OSS_FILEREF_STRING_PREFIX + 'nnnnn' + OSS_FILEREF_STRING_SUFFIX
	 * where 'nnnnn' is the frefid_t pointer converted into a string of decimal
	 * numbers. This is only really practical for 32-bit pointers; if we use
	 * 64-bit pointers I'll have to start using a hash table or use hex
	 * numbers.
	 */
	uint oss_convert_fileref_to_string(frefid_t file_to_convert, char *buffer, int buf_len);

	/**
	 * Turn a filename or a special fileref string into an actual fileref.
	 * Notice that, since Glk doesn't know paths, we take this opportunity to
	 * call oss_check_path, which should do the OS-dependent path changing
	 * in the event that the filename contains path information
	 */
	frefid_t oss_convert_string_to_fileref(char *buffer, uint usage);

	/**
	 * Tell us if the passed string is a hashed fileref or not
	 */
	bool oss_is_string_a_fileref(char *buffer);

	/**
	 * Change a Glk key into a TADS one, using the CMD_xxx codes
	 */
	unsigned char oss_convert_keystroke_to_tads(uint key);

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
	osfildef *oss_open_stream(char *buffer, uint tadsusage, uint tbusage,
		uint fmode, uint rock);

	/**
	 * Get a pointer to the root name portion of a filename.  This is the part
	 * of the filename after any path or directory prefix.  For example, on
	 * Unix, given the string "/home/mjr/deep.gam", this function should return
	 * a pointer to the 'd' in "deep.gam".  If the filename doesn't appear to
	 * have a path prefix, it should simply return the argument unchanged.
	 */
	const char *os_get_root_name(const char *buf) const { return buf; }

	/**
	 * Open a file for access
	 */
	osfildef *osfoprb(const char *fname, uint typ = 0);

	/**
	 * Receive notification that a character mapping file has been loaded.  We
	 * don't need to do anything with this information, since we we're relying
	 * on the Glk layer and ScummVM backend to handle all that
	 */
	void os_advise_load_charmap(const char *id, const char *ldesc, const char *sysinfo) {
		// No implementation needed
	}

	/**
	 * Generate a filename for a character mapping table.  On Windows, the
	 * filename is always simply "win" plus the internal ID plus ".tcp".
	 */
	void os_gen_charmap_filename(char *filename, const char *internal_id,
		const char *argv0);

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
