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

#include "ultima/ultima8/games/start_crusader_process.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/games/remorse_game.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/gumps/cru_pickup_area_gump.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/graphics/palette_fader_process.h"
#include "ultima/ultima8/audio/music_process.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(StartCrusaderProcess)

StartCrusaderProcess::StartCrusaderProcess(int saveSlot) : Process(),
		_initStage(PlayFirstMovie), _saveSlot(saveSlot), _skipStart(saveSlot >= 0) {
}


void StartCrusaderProcess::run() {
	if (!_skipStart && _initStage == PlayFirstMovie) {
		_initStage = PlaySecondMovie;
		ProcId moviepid = Game::get_instance()->playIntroMovie(false);
		Process *movieproc = Kernel::get_instance()->getProcess(moviepid);
		if (movieproc) {
			waitFor(movieproc);
			return;
		}
	} else if (!_skipStart && _initStage == PlaySecondMovie) {
		_initStage = ShowMenu;
		RemorseGame *game = dynamic_cast<RemorseGame *>(Game::get_instance());
		assert(game);
		ProcId moviepid = game->playIntroMovie2(false);
		Process *movieproc = Kernel::get_instance()->getProcess(moviepid);
		if (movieproc) {
			waitFor(movieproc);
			return;
		}
	}

	Gump *statusGump = new CruStatusGump(true);
	statusGump->InitGump(nullptr, false);

	Gump *cruPickupAreaGump = new CruPickupAreaGump(true);
	cruPickupAreaGump->InitGump(nullptr, false);

	// Try to load the save game, if succeeded this pointer will no longer be valid
	if (_saveSlot >= 0 && Ultima8Engine::get_instance()->loadGameState(_saveSlot).getCode() == Common::kNoError) {
		//PaletteFaderProcess::I_fadeFromBlack(0, 0);
		return;
	}

	Ultima8Engine::get_instance()->setCheatMode(true);

	if (!_skipStart) {
		MainActor *avatar = getMainActor();
		int mapnum = avatar->getMapNum();

		// These items are the same in Regret and Remorse
		Item *datalink = ItemFactory::createItem(0x4d4, 0, 0, 0, 0, mapnum, 0, true);
		avatar->addItemCru(datalink, false);
		Item *smiley = ItemFactory::createItem(0x598, 0, 0, 0, 0, mapnum, 0, true);
		smiley->moveToContainer(avatar);

		if (GAME_IS_REMORSE) {
			// TODO: The game actually teleports to egg 0x1e (30) which has another
			// egg to teleport to egg 99.  Is there any purpose to that?
			Kernel::get_instance()->addProcess(new TeleportToEggProcess(1, 99));
		} else if (GAME_IS_REGRET) {
			Kernel::get_instance()->addProcess(new TeleportToEggProcess(1, 0x1e));
		}

		Process *fader = new PaletteFaderProcess(0x003F3F3F, true, 0x7FFF, 60, false);
		Kernel::get_instance()->addProcess(fader);
	}

	Ultima8Engine::get_instance()->setAvatarInStasis(false);

	terminate();
}

void StartCrusaderProcess::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN();

	Process::saveData(ws);
}

bool StartCrusaderProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
