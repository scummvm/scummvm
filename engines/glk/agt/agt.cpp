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
#include "common/translation.h"

namespace Glk {
namespace AGT {

AGT *g_vm;

extern void glk_main();
extern int glk_startup_code();
extern void gagt_finalizer();

AGT::AGT(OSystem *syst, const GlkGameDescription &gameDesc) : GlkAPI(syst, gameDesc),
	gagt_gamefile(nullptr), gagt_game_message(nullptr) {
	g_vm = this;
}

void AGT::runGame() {
	_gameFile.close();
	gagt_gamefile = getFilename().c_str();

	glk_startup_code();
	glk_main();

	gagt_finalizer();
}

Common::Error AGT::readSaveData(Common::SeekableReadStream *rs) {
	return Common::kReadingFailed;
}

Common::Error AGT::writeGameData(Common::WriteStream *ws) {
	return Common::kWritingFailed;
}

} // End of namespace AGT
} // End of namespace Glk
