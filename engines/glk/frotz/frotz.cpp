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

#include "glk/frotz/frotz.h"
#include "glk/frotz/frotz_types.h"
#include "common/config-manager.h"

namespace Glk {
namespace Frotz {

Frotz *g_vm;

Frotz::Frotz(OSystem *syst, const GlkGameDescription &gameDesc) :
		Processor(syst, gameDesc) {
	g_vm = this;
}

Frotz::~Frotz() {
	reset_memory();
}

void Frotz::runGame(Common::SeekableReadStream *gameFile) {
	story_fp = gameFile;
	initialize();

	// Game loop
	interpret();

	if (!shouldQuit()) {
		flush_buffer();
		glk_exit();
	}
}

void Frotz::initialize() {
	// Call process initialization
	Processor::initialize();

	// Restart the game
	z_restart();
}

Common::Error Frotz::saveGameData(strid_t file) {
#ifdef TODO
	long pc;
	zword addr;
	zword nsp, nfp;
	int skip;
	int i;

	// Open game file

	if ((gfp = frotzopenprompt(FILE_SAVE)) == nullptr)
		goto finished;

	if (_quetzal) {
		success = save_quetzal(gfp, story_fp, blorb_ofs);
	}
	else {
		// Write game file

		fputc((int)hi(h_release), gfp);
		fputc((int)lo(h_release), gfp);
		fputc((int)hi(h_checksum), gfp);
		fputc((int)lo(h_checksum), gfp);

		GET_PC(pc)

			fputc((int)(pc >> 16) & 0xff, gfp);
		fputc((int)(pc >> 8) & 0xff, gfp);
		fputc((int)(pc)& 0xff, gfp);

		nsp = (int)(_sp - _stack);
		nfp = (int)(_fp - _stack);

		fputc((int)hi(nsp), gfp);
		fputc((int)lo(nsp), gfp);
		fputc((int)hi(nfp), gfp);
		fputc((int)lo(nfp), gfp);

		for (i = nsp; i < STACK_SIZE; i++) {
			fputc((int)hi(_stack[i]), gfp);
			fputc((int)lo(_stack[i]), gfp);
		}

		fseek(story_fp, blorb_ofs, SEEK_SET);

		for (addr = 0, skip = 0; addr < h_dynamic_size; addr++)
			if (zmp[addr] != fgetc(story_fp) || skip == 255 || addr + 1 == h_dynamic_size) {
				fputc(skip, gfp);
				fputc(zmp[addr], gfp);
				skip = 0;
			}
			else skip++;
	}

	// Close game file and check for errors

	if (fclose(gfp) == EOF || ferror(story_fp)) {
		print_string("Error writing save file\n");
		goto finished;
	}

	// Success
	success = 1;
#endif
	return Common::kNoError;
}

Common::Error Frotz::loadGameData(strid_t file) {
#ifdef TODO
	long pc;
	zword release;
	zword addr;
	int i;

	// Open game file
	if ((gfp = frotzopenprompt(FILE_RESTORE)) == nullptr)
		goto finished;

	if (_quetzal) {
		success = restore_quetzal (gfp, story_fp, blorb_ofs);

	} else {
		// Load game file

		release = (unsigned) fgetc (gfp) << 8;
		release |= fgetc (gfp);

		() fgetc (gfp);
		() fgetc (gfp);

		// Check the release number
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

			// Check for errors
			if (ferror (gfp) || ferror (story_fp) || addr != h_dynamic_size)
				success = -1;
			else

				// Success
				success = 2;

		} else print_string ("Invalid save file\n");
	}

	if ((short) success >= 0) {

		// Close game file
		fclose (gfp);

		if ((short) success > 0) {
			zbyte old_screen_rows;
			zbyte old_screen_cols;

			// In V3, reset the upper window.
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
#endif
	return Common::kNoError;
}

} // End of namespace Scott
} // End of namespace Glk
