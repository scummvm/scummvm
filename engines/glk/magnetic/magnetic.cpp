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

#include "glk/magnetic/magnetic.h"
#include "common/config-manager.h"
#include "common/translation.h"

namespace Glk {
namespace Magnetic {

Magnetic::Magnetic(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		vm_exited_cleanly(false) {
}

void Magnetic::runGame() {
	if (!is_gamefile_valid())
		return;

	// TODO
}

Common::Error Magnetic::loadGameData(strid_t file) {
	// TODO
	return Common::kNoError;
}

Common::Error Magnetic::saveGameData(strid_t file, const Common::String &desc) {
	// TODO
	return Common::kNoError;
}

bool Magnetic::is_gamefile_valid() {
	if (_gameFile.size() < 8) {
		GUIErrorMessage(_("This is too short to be a valid Glulx file."));
		return false;
	}

	if (_gameFile.readUint32BE() != MKTAG('G', 'l', 'u', 'l')) {
		GUIErrorMessage(_("This is not a valid Glulx file."));
		return false;
	}

	// We support version 2.0 through 3.1.*
	uint version = _gameFile.readUint32BE();
	if (version < 0x20000) {
		GUIErrorMessage(_("This Glulx file is too old a version to execute."));
		return false;
	}
	if (version >= 0x30200) {
		GUIErrorMessage(_("This Glulx file is too new a version to execute."));
		return false;
	}

	return true;
}

} // End of namespace Magnetic
} // End of namespace Glk
