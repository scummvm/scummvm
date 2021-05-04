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


#include "ultima/ultima8/games/start_crusader_process.h"
#include "ultima/ultima8/games/remorse_game.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/gumps/cru_pickup_area_gump.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/graphics/palette_fader_process.h"

namespace Ultima {
namespace Ultima8 {

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

	// Try to load the save game, if succeeded this pointer will no longer be valid
	if (_saveSlot >= 0 && Ultima8Engine::get_instance()->loadGameState(_saveSlot).getCode() == Common::kNoError) {
		//PaletteFaderProcess::I_fadeFromBlack(0, 0);
		return;
	}

	Gump *statusGump = new CruStatusGump(true);
	statusGump->InitGump(nullptr, false);

	Gump *cruPickupAreaGump = new CruPickupAreaGump(true);
	cruPickupAreaGump->InitGump(nullptr, false);

	Ultima8Engine::get_instance()->setCheatMode(true);

	if (!_skipStart) {
		MainActor *avatar = getMainActor();
		int mapnum = avatar->getMapNum();

		// These items are the same in Regret and Remorse
		Item *datalink = ItemFactory::createItem(0x4d4, 0, 0, 0, 0, mapnum, 0, true);
		avatar->addItemCru(datalink, false);
		Item *smiley = ItemFactory::createItem(0x598, 0, 0, 0, 0, mapnum, 0, true);
		smiley->moveToContainer(avatar);

		avatar->setShieldType(1);

#if 0
		// Give the avatar *all the weapons and ammo*.. (handy for testing)
		uint32 wpnshapes[] = {
			// Weapons
			0x032E, 0x032F, 0x0330, 0x038C, 0x0332, 0x0333, 0x0334,
			0x038E, 0x0388, 0x038A, 0x038D, 0x038B, 0x0386,
			// Ammo
			0x033D, 0x033E, 0x033F, 0x0340, 0x0341
		};
		for (int i = 0; i < ARRAYSIZE(wpnshapes); i++) {
			for (int j = 0; j < 5; j++) {
				Item *wpn = ItemFactory::createItem(wpnshapes[i], 0, 0, 0, 0, mapnum, 0, true);
				avatar->addItemCru(wpn, false);
			}
		}
#endif

		avatar->teleport(1, 0x1e);
		// The first level 0x1e teleporter in No Remorse goes straight to another
		// teleport, so undo the flag that normally stops that.
		avatar->setJustTeleported(false);

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
