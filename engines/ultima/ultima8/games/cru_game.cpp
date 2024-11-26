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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/ultima8/games/cru_game.h"
#include "ultima/ultima8/games/start_crusader_process.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/gumps/main_menu_process.h"
#include "ultima/ultima8/gumps/cru_credits_gump.h"
#include "ultima/ultima8/gumps/cru_demo_gump.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/gfx/xform_blend.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/npc_dat.h"
#include "common/memstream.h"

#include "gui/message.h"

namespace Ultima {
namespace Ultima8 {

CruGame::CruGame() : Game(), _skipIntroMovie(false) {
}

CruGame::~CruGame() {

}

static bool loadPalette(const char *path, PaletteManager::PalIndex index) {
	Common::File pf;
	if (!pf.open(path)) {
		warning("Unable to load %s", path);
		return false;
	}

	Common::MemoryReadStream xfds(CruXFormPal, 1024);
	PaletteManager::get_instance()->load(index, pf, xfds);

	return true;
}

bool CruGame::loadFiles() {
	// Load palette
	debug(1, "Load Palettes");

	if (!loadPalette("static/gamepal.pal", PaletteManager::Pal_Game))
		return false;
	// This one is not always present and only needed for the credits,
	// let it fail if needed.
	loadPalette("static/cred.pal", PaletteManager::Pal_Cred);
	if (!loadPalette("static/diff.pal", PaletteManager::Pal_Diff))
		return false;
	if (!loadPalette("static/misc.pal", PaletteManager::Pal_Misc))
		return false;
	if (!loadPalette("static/misc2.pal", PaletteManager::Pal_Misc2))
		return false;
	// We don't use his one at the moment, ok to fail.
	loadPalette("static/star.pal", PaletteManager::Pal_Star);

	debug(1, "Load GameData");
	GameData::get_instance()->loadRemorseData();

	return true;
}

bool CruGame::startGame() {
	// NOTE: assumes the entire engine has been reset!
	debug(1, "Starting new Crusader: No Remorse game.");

	ObjectManager *objman = ObjectManager::get_instance();

	// reserve a number of objids just in case we'll need them sometime
	for (uint16 i = 384; i < 512; ++i)
		objman->reserveObjId(i);

	Actor *actor = ItemFactory::createActor(1, 0, 0, Item::FLG_IN_NPC_LIST,
	                                        1, 1, Item::EXT_PERMANENT_NPC, false);
	if (!actor)
		error("Couldn't create MainActor");

	const NPCDat *npcData = GameData::get_instance()->getNPCDataForShape(1);

	actor->setStr(75);
	actor->setHP(npcData->getMaxHp());
	actor->setInt(5000); // max mana (energy) is 2x intelligence, or 10000.
	actor->setMana(2500);

	ObjectManager::get_instance()->assignActorObjId(actor, 1);

	actor->setLocation(0, 0, 0); // Map 1 (mission 1)

	// Some useful points to warp into for testing No Remorse
	//actor->setLocation(60716, 59400, 16); // Map 1 (mission 1)
	//actor->setLocation(42493, 26621, 16); // Map 2 (mission 1 / level 4)
	//actor->setLocation(34302, 32254, 16); // Map 3 (mission 2)
	//actor->setLocation(34813, 33789, 16); // Map 4
	//actor->setLocation(37373, 30205, 16); // Map 5
	//actor->setLocation(37373, 30205, 16); // Map 6
	//actor->setLocation(35070, 26142, 96); // Map 7
	//actor->setLocation(29693, 32253, 0); // Map 8 - unfinished area?
	//actor->setLocation(2046, 2046, 0); // Map 9
	//actor->setLocation(14845, 6141, 0); // Map 22 - debugging map
	//actor->setLocation(34302, 32254, 16); // Map 40 (Rebel base)

	World::get_instance()->switchMap(0);

	return true;
}

bool CruGame::startInitialUsecode(int saveSlot) {
	if (saveSlot >= 0 && ConfMan.getBool("skip_intro"))
		_skipIntroMovie = true;
	Process* proc = new StartCrusaderProcess(saveSlot);
	Kernel::get_instance()->addProcess(proc);
	return true;
}


static ProcId playMovie(const char *movieID, bool fade, bool noScale) {
	MovieGump *gump = MovieGump::CruMovieViewer(movieID, 640, 480, nullptr, nullptr, 0);
	if (!gump) {
		debug(1, "RemorseGame::playIntro: movie %s not found.", movieID);
		return 0;
	}
	gump->CreateNotifier();
	return gump->GetNotifyProcess()->getPid();
}

ProcId CruGame::playIntroMovie(bool fade) {
	if (_skipIntroMovie)
		return 0;
	const char *name = (GAME_IS_REMORSE ? "T01" : "origin");
	ProcId pid = playMovie(name, fade, true);
	if (!pid) {
		GUI::MessageDialogWithURL dialog(_("Crusader intro movie file missing - check that the FLICS and SOUND directories have been copied from the CD.  More instructions are on the wiki: https://wiki.scummvm.org/index.php?title=Crusader:_No_Remorse."), "https://wiki.scummvm.org/index.php?title=Crusader:_No_Remorse");
		dialog.runModal();
	}
	return pid;
}

ProcId CruGame::playIntroMovie2(bool fade) {
	if (_skipIntroMovie)
		return 0;
	const char *name = (GAME_IS_REMORSE ? "T02" : "ANIM01");
	return playMovie(name, fade, false);
}

ProcId CruGame::playEndgameMovie(bool fade) {
	return playMovie("O01", fade, false);
}

void CruGame::playDemoScreen() {
	Process *menuproc = new MainMenuProcess();
	Kernel::get_instance()->addProcess(menuproc);

	const char *bmp_filename = "static/buyme.dat";
	auto *bmprs = new Common::File();
	if (!bmprs->open(bmp_filename)) {
		warning("RemorseGame::playDemoScreen: error opening demo background: %s", bmp_filename);
		delete bmprs;
		return;
	}
	Gump *gump = new CruDemoGump(bmprs);
	gump->InitGump(0);
	gump->CreateNotifier();
	Process *notifyproc = gump->GetNotifyProcess();

	if (notifyproc) {
		menuproc->waitFor(notifyproc);
	}
}

ProcId CruGame::playCreditsNoMenu() {
	const char *txt_filename = "static/credits.dat";
	const char *bmp_filename = "static/cred.dat";
	auto *txtrs = new Common::File();
	auto *bmprs = new Common::File();

	if (!txtrs->open(txt_filename)) {
		warning("RemorseGame::playCredits: error opening credits text: %s", txt_filename);
		delete txtrs;
		delete bmprs;
		return 0;
	}

	if (!bmprs->open(bmp_filename)) {
		warning("RemorseGame::playCredits: error opening credits background: %s", bmp_filename);
		delete txtrs;
		delete bmprs;
		return 0;
	}

	Gump *creditsgump = new CruCreditsGump(txtrs, bmprs);
	creditsgump->InitGump(nullptr);
	creditsgump->CreateNotifier();
	Process *notifyproc = creditsgump->GetNotifyProcess();
	return notifyproc->getPid();
}


void CruGame::playCredits() {
	Process *menuproc = new MainMenuProcess();
	Kernel::get_instance()->addProcess(menuproc);

	ProcId creditsnotify = playCreditsNoMenu();

	if (creditsnotify) {
		menuproc->waitFor(creditsnotify);
	}
}

void CruGame::writeSaveInfo(Common::WriteStream *ws) {
}

} // End of namespace Ultima8
} // End of namespace Ultima
