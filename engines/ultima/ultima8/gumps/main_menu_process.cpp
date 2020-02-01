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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/gumps/main_menu_process.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(MainMenuProcess, Process)

MainMenuProcess::MainMenuProcess() : Process() {
	init = false;
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

void MainMenuProcess::saveData(ODataSource *ods) {
	CANT_HAPPEN();

	Process::saveData(ods);
}

bool MainMenuProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	return true;
}

} // End of namespace Ultima8
