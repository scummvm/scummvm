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
#include "glk/frotz/screen.h"
#include "glk/frotz/quetzal.h"
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

Screen *Frotz::createScreen() {
	return new FrotzScreen();
}

void Frotz::runGame(Common::SeekableReadStream *gameFile) {
	story_fp = gameFile;
	initialize();

	// If save was selected from the launcher, handle loading it
	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	if (saveSlot != -1) {
		bool success = loadGameState(saveSlot).getCode() == Common::kNoError;

		if (h_version <= V3)
			branch(success);
		else
			store(success);
	}

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

Common::Error Frotz::saveGameData(strid_t file, const Common::String &desc) {
	Quetzal q(story_fp);
	bool success = q.save(*file, this, desc);

	if (!success)
		print_string("Error writing save file\n");

	return Common::kNoError;
}

Common::Error Frotz::loadGameData(strid_t file) {
	Quetzal q(story_fp);
	bool success = q.restore(*file, this) == 2;

	if (success) {
		zbyte old_screen_rows;
		zbyte old_screen_cols;

		// In V3, reset the upper window.
		if (h_version == V3)
			split_window(0);

		LOW_BYTE(H_SCREEN_ROWS, old_screen_rows);
		LOW_BYTE(H_SCREEN_COLS, old_screen_cols);

		// Reload cached header fields
		restart_header();

		/* Since QUETZAL files may be saved on many different machines,
		 * the screen sizes may vary a lot. Erasing the status window
		 * seems to cover up most of the resulting badness.
		 */
		if (h_version > V3 && h_version != V6 && (h_screen_rows != old_screen_rows
					|| h_screen_cols != old_screen_cols))
			erase_window(1);
	} else {
		error("Error reading save file");
	}

	return Common::kNoError;
}

} // End of namespace Scott
} // End of namespace Glk
