/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "ultima/ultima8/gumps/main_menu_process.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(MainMenuProcess)

MainMenuProcess::MainMenuProcess() : Process() {
	_flags |= PROC_PREVENT_SAVE;
}


void MainMenuProcess::run() {
	MainActor *avatar = getMainActor();
	if (avatar && avatar->isDead()) {
		// stop death music
		MusicProcess::get_instance()->playCombatMusic(0);
	}

	MenuGump::showMenu();

	terminate();
}

void MainMenuProcess::saveData(Common::WriteStream *ws) {
	warning("Attempted save of process with prevent save flag");

	Process::saveData(ws);
}

bool MainMenuProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
