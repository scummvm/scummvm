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

#include "glk/alan2/alan2.h"
#include "glk/alan2/decode.h"
#include "glk/alan2/execute.h"
#include "glk/alan2/interpreter.h"
#include "glk/alan2/saveload.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "glk/glk.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan2 {

Alan2 *_vm = nullptr;

Alan2::Alan2(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		vm_exited_cleanly(false) {
	_vm = this;
}

void Alan2::runGame(Common::SeekableReadStream *gameFile) {
	_gameFile = gameFile;

	// TODO: Initialize these properly
	int tmp = 0;
	Common::String gameFileName;
	_decode = new Decode(nullptr, nullptr);
	_execute = new Execute();
	_saveLoad = new SaveLoad(gameFileName, nullptr, nullptr, nullptr, nullptr, &tmp);
	_interpreter = new Interpreter(_execute, _saveLoad, _stack);

	if (!is_gamefile_valid())
		return;

	// TODO
}

Common::Error Alan2::loadGameData(strid_t file) {
	// TODO
	return Common::kNoError;
}

Common::Error Alan2::saveGameData(strid_t file, const Common::String &desc) {
	// TODO
	return Common::kNoError;
}

bool Alan2::is_gamefile_valid() {
	if (_gameFile->size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Alan2 file."));
		return false;
	}

	if (_gameFile->readUint32BE() != MKTAG(2, 8, 1, 0)) {
		GUIErrorMessage(_("This is not a valid Alan2 file."));
		return false;
	}

	return true;
}

void Alan2::output(const Common::String str) {
	// TODO
}

void Alan2::printMessage(MsgKind msg) {
	// TODO
}

void Alan2::printError(MsgKind msg) {
	// TODO
}

void Alan2::paragraph() {
	if (col != 1)
		newLine();
	newLine();
}

void Alan2::newLine() {
	// TODO
}

} // End of namespace Alan2
} // End of namespace Glk
