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

#include "gargoyle/frotz/processor.h"

namespace Gargoyle {
namespace Frotz {

zchar Processor::console_read_input(int max, zchar *buf, zword timeout, bool continued) {
	return os_read_line(max, buf, timeout, max, continued);
}

zchar Processor::console_read_key(zword timeout) {
	return os_read_key(timeout, 0);
}

void Processor::scrollback_char(zchar c) {
    if (c == ZC_INDENT)
        { scrollback_char (' '); scrollback_char (' '); scrollback_char (' '); return; }
    if (c == ZC_GAP)
        { scrollback_char (' '); scrollback_char (' '); return; }

    os_scrollback_char(c);
}

void Processor::scrollback_word(const zchar *s) {
    int i;

	for (i = 0; s[i] != 0; i++) {
		if (s[i] == ZC_NEW_FONT || s[i] == ZC_NEW_STYLE)
			i++;
		else
			scrollback_char(s[i]);
	}
}

void Processor::scrollback_write_input(const zchar *buf, zchar key) {
    int i;

    for (i = 0; buf[i] != 0; i++)
        scrollback_char (buf[i]);

    if (key == ZC_RETURN)
        scrollback_char ('\n');
}

void Processor::scrollback_erase_input(const zchar *buf) {
    int width;
    int i;

    for (i = 0, width = 0; buf[i] != 0; i++)
        width++;

    os_scrollback_erase(width);

}

void Processor::stream_mssg_on() {
    flush_buffer();

    if (ostream_screen)
	    screen_mssg_on();
    if (ostream_script && enable_scripting)
	script_mssg_on();

    message = true;
}

void Processor::stream_mssg_off() {
    flush_buffer();

    if (ostream_screen)
		screen_mssg_off();
    if (ostream_script && enable_scripting)
		script_mssg_off();

    message = false;
}

void Processor::stream_char(zchar c) {
    if (ostream_screen)
		screen_char(c);
    if (ostream_script && enable_scripting)
		script_char(c);
    if (enable_scripting)
		scrollback_char(c);
}

void Processor::stream_word(const zchar *s) {
    if (ostream_memory && !message)
		memory_word(s);
    else {
		if (ostream_screen)
			screen_word(s);
		if (ostream_script && enable_scripting)
			script_word(s);
		if (enable_scripting)
			scrollback_word(s);
    }
}

void Processor::stream_new_line() {
    if (ostream_memory && !message)
		memory_new_line();
    else {
		if (ostream_screen)
			screen_new_line();
		if (ostream_script && enable_scripting)
			script_new_line();
		if (enable_scripting)
			os_scrollback_char ('\n');
    }
}

zchar Processor::stream_read_key(zword timeout, zword routine, bool hot_keys) {
    zchar key = ZC_BAD;

    flush_buffer();

    // Read key from current input stream
continue_input:

    do {
		if (istream_replay)
			key = replay_read_key();
		else
			key = console_read_key(timeout);
    } while (key == ZC_BAD);

    // Copy key to the command file
    if (ostream_record && !istream_replay)
		record_write_key(key);

    // Handle timeouts
    if (key == ZC_TIME_OUT)
		if (direct_call (routine) == 0)
			goto continue_input;

    // Return key
    return key;
}

zchar Processor::stream_read_input(int max, zchar *buf, zword timeout, zword routine,
			  bool hot_keys, bool no_scripting) {
    zchar key = ZC_BAD;
    bool no_scrollback = no_scripting;

    if (h_version == V6 && _storyId == UNKNOWN && !ostream_script)
		no_scrollback = false;

    flush_buffer();

    // Remove initial input from the transscript file or from the screen
    if (ostream_script && enable_scripting && !no_scripting)
	script_erase_input(buf);

    // Read input line from current input stream
continue_input:

    do {
		if (istream_replay)
			key = replay_read_input(buf);
		else
			key = console_read_input(max, buf, timeout, key != ZC_BAD);
    } while (key == ZC_BAD);

    // Copy input line to the command file
    if (ostream_record && !istream_replay)
		record_write_input(buf, key);

    // Handle timeouts
    if (key == ZC_TIME_OUT)
	if (direct_call(routine) == 0)
	    goto continue_input;

    // Copy input line to transscript file or to the screen
    if (ostream_script && enable_scripting && !no_scripting)
		script_write_input(buf, key);

    // Return terminating key
    return key;
}

void Processor::script_open() {
	h_flags &= ~SCRIPTING_FLAG;

	frefid_t fref = glk_fileref_create_by_prompt(fileusage_Transcript,
		filemode_WriteAppend);
	sfp = glk_stream_open_file(fref, filemode_WriteAppend);

	if (sfp != nullptr) {
		sfp->setPosition(0, seekmode_End);

		h_flags |= SCRIPTING_FLAG;

		script_valid = true;
		ostream_script = true;

		script_width = 0;
	} else {
		print_string("Cannot open file\n");
	}

	SET_WORD(H_FLAGS, h_flags);
}

void Processor::script_close() {
	h_flags &= ~SCRIPTING_FLAG;
	SET_WORD(H_FLAGS, h_flags);

	glk_stream_close(sfp);
	ostream_script = false;
}

void Processor::script_new_line() {
	script_char('\n');
	script_width = 0;
}

void Processor::script_char(zchar c) {
	if (c == ZC_INDENT && script_width != 0)
		c = ' ';

	if (c == ZC_INDENT) {
		script_char(' ');
		script_char(' ');
		script_char(' ');
		return;
	}
	if (c == ZC_GAP) {
		script_char(' ');
		script_char(' ');
		return;
	}

	sfp->putCharUni(c);
	script_width++;
}

void Processor::script_word(const zchar *s) {
	int width;
	int i;

	if (*s == ZC_INDENT && script_width != 0)
		script_char(*s++);

	for (i = 0, width = 0; s[i] != 0; i++) {
		if (s[i] == ZC_NEW_STYLE || s[i] == ZC_NEW_FONT)
			i++;
		else if (s[i] == ZC_GAP)
			width += 3;
		else if (s[i] == ZC_INDENT)
			width += 2;
		else
			width += 1;
	}

	if (_script_cols != 0 && script_width + width > _script_cols) {
		if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
			s++;

		script_new_line();
	}

	for (i = 0; s[i] != 0; i++) {
		if (s[i] == ZC_NEW_FONT || s[i] == ZC_NEW_STYLE)
			i++;
		else
			script_char(s[i]);
	}
}

void Processor::script_write_input(const zchar *buf, zchar key) {
	int width;
	int i;

	for (i = 0, width = 0; buf[i] != 0; i++)
		width++;

	if (_script_cols != 0 && script_width + width > _script_cols)
		script_new_line();

	for (i = 0; buf[i] != 0; i++)
		script_char(buf[i]);

	if (key == ZC_RETURN)
		script_new_line();
}

void Processor::script_erase_input(const zchar *buf) {
	int width;
	int i;

	for (i = 0, width = 0; buf[i] != 0; i++)
		width++;

	sfp->setPosition(-width, seekmode_Current);
	script_width -= width;
}

void Processor::script_mssg_on() {
	if (script_width != 0)
		script_new_line();

	script_char(ZC_INDENT);
}

void Processor::script_mssg_off() {
	script_new_line();
}

void Processor::record_open() {
	frefid_t fref = glk_fileref_create_by_prompt(fileusage_Transcript, filemode_Write);
	if ((rfp = glk_stream_open_file(fref, filemode_Write)) != nullptr)
		ostream_record = true;
	else
		print_string("Cannot open file\n");
}

void Processor::record_close() {
	glk_stream_close(rfp);
	ostream_record = false;
}

void Processor::record_code(int c, bool force_encoding) {
	if (force_encoding || c == '[' || c < 0x20 || c > 0x7e) {
		int i;

		rfp->putChar('[');

		for (i = 10000; i != 0; i /= 10)
			if (c >= i || i == 1)
				rfp->putChar('0' + (c / i) % 10);

		rfp->putChar(']');
	} else {
		rfp->putChar(c);
	}
}

void Processor::record_char(zchar c) {
	if (c != ZC_RETURN) {
		if (c < ZC_HKEY_MIN || c > ZC_HKEY_MAX) {
			record_code(translate_to_zscii(c), false);
			if (c == ZC_SINGLE_CLICK || c == ZC_DOUBLE_CLICK) {
				record_code(mouse_x, true);
				record_code(mouse_y, true);
			}
		} else {
			record_code(1000 + c - ZC_HKEY_MIN, true);
		}
	}
}

void Processor::record_write_key(zchar key) {
	record_char(key);
	rfp->putChar('\n');
}

void Processor::record_write_input(const zchar *buf, zchar key) {
	zchar c;

	while ((c = *buf++) != 0)
		record_char(c);

	record_write_key(key);
}

void Processor::replay_open() {
	frefid_t fref = glk_fileref_create_by_prompt(fileusage_Transcript, filemode_Read);
	if ((pfp = glk_stream_open_file(fref, filemode_Read)) != nullptr)
		istream_replay = true;
	else
		print_string("Cannot open file\n");
}

void Processor::replay_close() {
	glk_stream_close(pfp);
	istream_replay = false;
}

int Processor::replay_code() {
	int c;

	if ((c = pfp->getChar()) == '[') {
		int c2;

		c = 0;

		while ((c2 = pfp->getChar()) != EOF && c2 >= '0' && c2 <= '9')
			c = 10 * c + c2 - '0';

		return (c2 == ']') ? c : EOF;
	} else {
		return c;
	}
}

zchar Processor::replay_char() {
	int c;

	if ((c = replay_code()) != EOF) {
		if (c != '\n') {
			if (c < 1000) {

				c = translate_from_zscii(c);

				if (c == ZC_SINGLE_CLICK || c == ZC_DOUBLE_CLICK) {
					mouse_x = replay_code();
					mouse_y = replay_code();
				}

				return c;
			} else {
				return ZC_HKEY_MIN + c - 1000;
			}
		}

		pfp->unputBuffer("\n", 1);
		return ZC_RETURN;

	} else {
		return ZC_BAD;
	}
}

zchar Processor::replay_read_key() {
	zchar key = replay_char();

	if (pfp->getChar() != '\n') {
		replay_close();
		return ZC_BAD;
	} else {
		return key;
	}
}

zchar Processor::replay_read_input(zchar *buf) {
	zchar c;

	for (;;) {
		c = replay_char();

		if (c == ZC_BAD || is_terminator(c))
			break;

		*buf++ = c;
	}

	*buf = 0;

	if (pfp->getChar() != '\n') {
		replay_close();
		return ZC_BAD;
	} else {
		return c;
	}
}


void Processor::z_input_stream() {
	flush_buffer();

	if (zargs[0] == 0 && istream_replay)
		replay_close();
	if (zargs[0] == 1 && !istream_replay)
		replay_open();
}

void Processor::z_output_stream() {
    flush_buffer();

    switch ((short) zargs[0]) {
    case  1: ostream_screen = true;
	     break;
    case -1: ostream_screen = false;
	     break;
    case  2: if (!ostream_script) script_open();
	     break;
    case -2: if (ostream_script) script_close();
	     break;
    case  3: memory_open(zargs[1], zargs[2], zargc >= 3);
	     break;
    case -3: memory_close();
	     break;
    case  4: if (!ostream_record) record_open();
	     break;
    case -4: if (ostream_record) record_close();
	     break;
	default:
		break;
    }
}

void Processor::z_restart() {
	flush_buffer();

	os_restart_game(RESTART_BEGIN);

	seed_random(0);

	if (!first_restart) {
		story_fp->seek(blorb_ofs);

		if (story_fp->read(zmp, h_dynamic_size) != h_dynamic_size)
			error("Story file read error");

	} else {
		first_restart = false;
	}

	restart_header();
	restart_screen();

	_sp = _fp = _stack + STACK_SIZE;
	_frameCount = 0;

	if (h_version != V6 && h_version != V9) {
		long pc = (long)h_start_pc;
		SET_PC(pc);
	} else {
		call(h_start_pc, 0, nullptr, 0);
	}

	os_restart_game(RESTART_END);
}

void Processor::z_save() {
#ifdef TODO
	bool success = false;

	if (zargc != 0) {
		// Open auxilary file
		frefid_t ref = glk_fileref_create_by_prompt(fileusage_Data | fileusage_BinaryMode,
			filemode_Write, 0);
		if (ref == nullptr)
			goto finished;

		// Write data
		strid_t f = glk_stream_open_file(ref, filemode_Write);

		glk_put_buffer_stream(f, (const char *)zmp + zargs[0], zargs[1]);

		glk_stream_close(f);

	} else {
		long pc;
		zword addr;
		zword nsp, nfp;
		int skip;
		int i;

		/* Open game file */

		if ((gfp = frotzopenprompt (FILE_SAVE)) == nullptr)
			goto finished;

		if (option_save_quetzal) {
			success = save_quetzal (gfp, story_fp, blorb_ofs);
		} else {
			/* Write game file */

			fputc ((int) hi (h_release), gfp);
			fputc ((int) lo (h_release), gfp);
			fputc ((int) hi (h_checksum), gfp);
			fputc ((int) lo (h_checksum), gfp);

			GET_PC (pc)

				fputc ((int) (pc >> 16) & 0xff, gfp);
			fputc ((int) (pc >> 8) & 0xff, gfp);
			fputc ((int) (pc) & 0xff, gfp);

			nsp = (int) (_sp - _stack);
			nfp = (int) (_fp - _stack);

			fputc ((int) hi (nsp), gfp);
			fputc ((int) lo (nsp), gfp);
			fputc ((int) hi (nfp), gfp);
			fputc ((int) lo (nfp), gfp);

			for (i = nsp; i < STACK_SIZE; i++) {
				fputc ((int) hi (_stack[i]), gfp);
				fputc ((int) lo (_stack[i]), gfp);
			}

			fseek (story_fp, blorb_ofs, SEEK_SET);

			for (addr = 0, skip = 0; addr < h_dynamic_size; addr++)
				if (zmp[addr] != fgetc (story_fp) || skip == 255 || addr + 1 == h_dynamic_size) {
					fputc (skip, gfp);
					fputc (zmp[addr], gfp);
					skip = 0;
				} else skip++;
		}

		/* Close game file and check for errors */

		if (fclose (gfp) == EOF || ferror (story_fp)) {
			print_string ("Error writing save file\n");
			goto finished;
		}

		/* Success */

		success = 1;

	}

finished:

	if (h_version <= V3)
		branch (success);
	else
		store (success);
#endif
}

void Processor::z_restore() {
#ifdef TODO
	FILE *gfp;

	zword success = 0;

	if (zargc != 0) {

		/* Get the file name */

		/* Open auxilary file */

		if ((gfp = frotzopenprompt(FILE_LOAD_AUX)) == nullptr)
			goto finished;

		/* Load auxilary file */

		success = fread (zmp + zargs[0], 1, zargs[1], gfp);

		/* Close auxilary file */

		fclose (gfp);

	} else {

		long pc;
		zword release;
		zword addr;
		int i;

		/* Open game file */

		if ((gfp = frotzopenprompt(FILE_RESTORE)) == nullptr)
			goto finished;

		if (option_save_quetzal) {
			success = restore_quetzal (gfp, story_fp, blorb_ofs);

		} else {
			/* Load game file */

			release = (unsigned) fgetc (gfp) << 8;
			release |= fgetc (gfp);

			() fgetc (gfp);
			() fgetc (gfp);

			/* Check the release number */

			if (release == h_release) {

				pc = (long) fgetc (gfp) << 16;
				pc |= (unsigned) fgetc (gfp) << 8;
				pc |= fgetc (gfp);

				SET_PC (pc);

				_sp = _stack + (fgetc (gfp) << 8);
				_sp += fgetc (gfp);
				_fp = _stack + (fgetc (gfp) << 8);
				_fp += fgetc (gfp);

				for (i = (int) (_sp - _stack); i < STACK_SIZE; i++) {
					_stack[i] = (unsigned) fgetc (gfp) << 8;
					_stack[i] |= fgetc (gfp);
				}

				fseek (story_fp, blorb_ofs, SEEK_SET);

				for (addr = 0; addr < h_dynamic_size; addr++) {
					int skip = fgetc (gfp);
					for (i = 0; i < skip; i++)
						zmp[addr++] = fgetc (story_fp);
					zmp[addr] = fgetc (gfp);
					() fgetc (story_fp);
				}

				/* Check for errors */

				if (ferror (gfp) || ferror (story_fp) || addr != h_dynamic_size)
					success = -1;
				else

					/* Success */

					success = 2;

			} else print_string ("Invalid save file\n");
		}

		if ((short) success >= 0) {

			/* Close game file */

			fclose (gfp);

			if ((short) success > 0) {
				zbyte old_screen_rows;
				zbyte old_screen_cols;

				/* In V3, reset the upper window. */
				if (h_version == V3)
					split_window (0);

				LOW_BYTE (H_SCREEN_ROWS, old_screen_rows);
				LOW_BYTE (H_SCREEN_COLS, old_screen_cols);

				/* Reload cached header fields. */
				restart_header ();

				/*
				 * Since QUETZAL files may be saved on many different machines,
				 * the screen sizes may vary a lot. Erasing the status window
				 * seems to cover up most of the resulting badness.
				 */
				if (h_version > V3 && h_version != V6
						&& (h_screen_rows != old_screen_rows
							|| h_screen_cols != old_screen_cols))
					erase_window (1);
			}
		} else
			os_fatal ("Error reading save file");
	}

finished:

	if (h_version <= V3)
		branch (success);
	else
		store (success);
#endif
}

void Processor::z_verify() {
	zword checksum = 0;

	// Sum all bytes in story file except header bytes
	story_fp->seek(blorb_ofs + 64);

	for (uint i = 64; i < story_size; i++)
		checksum += story_fp->readByte();

	// Branch if the checksums are equal
	branch(checksum == h_checksum);
}

} // End of namespace Scott
} // End of namespace Gargoyle
