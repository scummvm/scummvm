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
#include "glk/alan2/main.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/error.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "glk/glk.h"
#include "glk/streams.h"

namespace Glk {
namespace Alan2 {

Alan2 *g_vm = nullptr;

Alan2::Alan2(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		vm_exited_cleanly(false) {
	g_vm = this;
}

void Alan2::runGame() {
	Common::String gameFileName = _gameFile.getName();

	if (!is_gamefile_valid())
		return;

	Common::String filename = getFilename();
	while (filename.contains('.'))
		filename.deleteLastChar();
	advnam = filename.c_str();

	codfil = &_gameFile;
	strncpy(codfnm, getFilename().c_str(), 255);
	codfnm[255] = '\0';

	Glk::Alan2::run();
}

Common::Error Alan2::readSaveData(Common::SeekableReadStream *rs) {
	// TODO
	return Common::kReadingFailed;
}

Common::Error Alan2::writeGameData(Common::WriteStream *ws) {
	// TODO
	return Common::kWritingFailed;
}

bool Alan2::is_gamefile_valid() {
	if (_gameFile.size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Alan2 file."));
		return false;
	}

	if (_gameFile.readUint32BE() != MKTAG(2, 8, 1, 0)) {
		GUIErrorMessage(_("This is not a valid Alan2 file."));
		return false;
	}

	return true;
}

} // End of namespace Alan2
} // End of namespace Glk
