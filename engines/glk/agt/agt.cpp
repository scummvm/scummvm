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

#include "glk/agt/agt.h"
#include "glk/quetzal.h"
#include "common/config-manager.h"
#include "glk/agt/agility.h"
#include "glk/agt/interp.h"
#include "glk/agt/exec.h"

namespace Glk {
namespace AGT {

AGT *g_vm;

extern void glk_main();
extern int glk_startup_code();
extern void gagt_finalizer();

AGT::AGT(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		gagt_main_window(nullptr), gagt_status_window(nullptr), gagt_gamefile(nullptr),
		gagt_game_message(nullptr), gagt_delay_mode(DELAY_SHORT), gagt_font_mode(FONT_AUTOMATIC),
		gagt_transcript_stream(nullptr), gagt_inputlog_stream(nullptr),
		gagt_readlog_stream(nullptr), gagt_replacement_enabled(true),
		gagt_extended_status_enabled(true), gagt_abbreviations_enabled(true),
		gagt_commands_enabled(true), gagt_clean_exit_test(false) {
	g_vm = this;
}

void AGT::runGame() {
	initialize();
	glk_main();

	gagt_finalizer();
}

void AGT::initialize() {
	_gameFile.close();
	gagt_gamefile = getFilename().c_str();

	initializeSettings();
	glk_startup_code();
}

void AGT::initializeSettings() {
	// Delay
	if (ConfMan.hasKey("delay")) {
		Common::String delay = ConfMan.get("delay");
		switch (tolower(delay.firstChar())) {
		case 'f':
			// Full
			gagt_delay_mode = DELAY_FULL;
			break;
		case 's':
			// Short
			gagt_delay_mode = DELAY_SHORT;
			break;
		case 'n':
		case 'o':
			// None/off
			gagt_delay_mode = DELAY_OFF;
			break;
		default:
			break;
		}
	}

	// Boolean flags
	if (ConfMan.hasKey("replacement"))
		gagt_replacement_enabled = ConfMan.getBool("replacement");
	if (ConfMan.hasKey("abbreviations"))
		gagt_abbreviations_enabled = ConfMan.getBool("abbreviations");
	if (ConfMan.hasKey("extended_status"))
		gagt_extended_status_enabled = ConfMan.getBool("extended_status");
	if (ConfMan.hasKey("commands"))
		gagt_commands_enabled = ConfMan.getBool("commands");
}

Common::Error AGT::readSaveData(Common::SeekableReadStream *rs) {
	return loadgame(rs);
}

Common::Error AGT::writeGameData(Common::WriteStream *ws) {
	return savegame(ws);
}

} // End of namespace AGT
} // End of namespace Glk
