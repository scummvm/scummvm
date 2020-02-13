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

#include "glk/jacl/jacl.h"
#include "glk/jacl/prototypes.h"
#include "glk/jacl/types.h"
#include "common/config-manager.h"

namespace Glk {
namespace JACL {

JACL *g_vm;
extern strid_t game_stream;
extern void glk_main();

JACL::JACL(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
	_saveSlot(-1) {
	g_vm = this;
}

void JACL::runGame() {
	// Check for savegame
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	// Open up the game file as a stream, and play the game
	game_stream = _streams->openStream(&_gameFile);
	glk_main();
}

bool JACL::initialize() {
	return true;
}

void JACL::deinitialize() {
}

Common::Error JACL::readSaveData(Common::SeekableReadStream *rs) {
	strid_t data_stream = _streams->openStream(rs);
	bool success = restore_game(data_stream);
	_streams->deleteStream(data_stream);

	return success ? Common::kNoError : Common::kReadingFailed;
}

Common::Error JACL::writeGameData(Common::WriteStream *ws) {
	strid_t data_stream = _streams->openStream(ws);
	bool success = save_game(data_stream);
	_streams->deleteStream(data_stream);

	return success ? Common::kNoError : Common::kWritingFailed;
}

bool JACL::loadLauncherSavegame() {
	int saveSlot = _saveSlot;
	_saveSlot = -1;

	if (loadGameState(saveSlot).getCode() == Common::kNoError)
		return true;

	write_text(cstring_resolve("CANT_RESTORE")->value);
	return false;
}

} // End of namespace JACL
} // End of namespace Glk
