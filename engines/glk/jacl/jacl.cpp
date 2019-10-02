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
#include "glk/jacl/common/config-manager.h"

namespace Glk {
namespace JACL {

JACL *g_vm;

JACL::JACL(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
		_saveSlot(-1) {
	g_vm = this;
}

void JACL::runGame() {
	// Check for savegame
	_saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

}

bool JACL::initialize() {
	return true;
}

void JACL::deinitialize() {
}

Common::Error JACL::readSaveData(Common::SeekableReadStream *rs) {
	return Common::kNoError;
}

Common::Error JACL::writeGameData(Common::WriteStream *ws) {
	return Common::kNoError;
}

} // End of namespace JACL
} // End of namespace Glk
