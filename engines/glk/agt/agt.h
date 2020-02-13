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

/* Based on Agility interpreter version 1.1.1 */

#ifndef GLK_AGT
#define GLK_AGT

#include "common/scummsys.h"
#include "glk/glk_api.h"

namespace Glk {
namespace AGT {

enum DelayMode {
	DELAY_FULL, DELAY_SHORT, DELAY_OFF
};

enum FontMode {
	FONT_AUTOMATIC, FONT_FIXED_WIDTH, FONT_PROPORTIONAL, FONT_DEBUG
};

/**
 * AGT Adams game interpreter
 */
class AGT : public GlkAPI {
public:
	const char *gagt_gamefile;      /* Name of game file. */
	const char *gagt_game_message;  /* Error message. */
	DelayMode gagt_delay_mode;

	/**
	 * We use two Glk windows; one is two lines at the top of the display area
	 * for status, and the other is the remainder of the display area, used for,
	 * well, everything else.  Where a particular Glk implementation won't do
	 * more than one window, the status window remains NULL.
	 */
	winid_t gagt_main_window, gagt_status_window;

	/**
	 * Transcript stream and input log.  These are NULL if there is no current
	 * collection of these strings.
	 */
	strid_t gagt_transcript_stream, gagt_inputlog_stream;

	/**
	 * Input read log stream, for reading back an input log
	 */
	strid_t gagt_readlog_stream;

	/* Options that may be turned off or set by command line flags. */
	FontMode gagt_font_mode;
	bool gagt_replacement_enabled, gagt_extended_status_enabled,
		gagt_abbreviations_enabled, gagt_commands_enabled;

	/**
	 * Flag to set if we want to test for a clean exit.  Without this it's a
	 * touch tricky sometimes to corner AGiliTy into calling exit() for us; it
	 * tends to require a broken game file.
	 */
	bool gagt_clean_exit_test;
private:
	/**
	 * Handles initialization
	 */
	void initialize();

	/**
	 * Handles flag setup from configuration
	 */
	void initializeSettings();
public:
	/**
	 * Constructor
	 */
	AGT(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override {
		return INTERPRETER_AGT;
	}

	/**
	 * Execute the game
	 */
	void runGame() override;

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;
};

extern AGT *g_vm;

} // End of namespace AGT
} // End of namespace Glk

#endif
