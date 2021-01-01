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

#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/controllers/read_choice_controller.h"
#include "ultima/ultima4/controllers/read_string_controller.h"
#include "ultima/ultima4/controllers/wait_controller.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

Shrines *g_shrines;

Shrines::Shrines() : _cycles(0), _completedCycles(0) {
	g_shrines = this;
}

Shrines::~Shrines() {
	g_shrines = nullptr;
}

void Shrines::loadAdvice() {
	_advice = u4read_stringtable("shrines");
}

/*-------------------------------------------------------------------*/

bool shrineCanEnter(const Portal *p) {
	Shrine *shrine = dynamic_cast<Shrine *>(mapMgr->get(p->_destid));
	assert(shrine);

	if (!g_context->_party->canEnterShrine(shrine->getVirtue())) {
		g_screen->screenMessage("Thou dost not bear the rune of entry!  A strange force keeps you out!\n");
		return 0;
	}
	return 1;
}

bool isShrine(Map *punknown) {
	Shrine *ps;
	if ((ps = dynamic_cast<Shrine *>(punknown)) != nullptr)
		return true;
	else
		return false;
}

/*-------------------------------------------------------------------*/

Shrine::Shrine() : _virtue(VIRT_HONESTY) {}

Common::String Shrine::getName() {
	if (_name.empty()) {
		_name = "Shrine of ";
		_name += getVirtueName(_virtue);
	}
	return _name;
}

Virtue Shrine::getVirtue() const {
	return _virtue;
}

Common::String Shrine::getMantra() const {
	return _mantra;
}

void Shrine::setVirtue(Virtue v) {
	_virtue = v;
}

void Shrine::setMantra(Common::String m) {
	_mantra = m;
}

void Shrine::enter() {
	if (!g_shrines->isAdviceLoaded())
		g_shrines->loadAdvice();

#ifdef IOS_ULTIMA4
	U4IOS::IOSHideGameControllerHelper hideControllsHelper;
#endif
	if (settings._enhancements && settings._enhancementsOptions._u5Shrines)
		enhancedSequence();
	else
		g_screen->screenMessage("You enter the ancient shrine and sit before the altar...");

	g_screen->screenMessage("\nUpon which virtue dost thou meditate?\n");
	Common::String virtue;
#ifdef IOS_ULTIMA4
	{
		U4IOS::IOSConversationHelper inputVirture;
		inputVirture.beginConversation(U4IOS::UIKeyboardTypeDefault, "Upon which virtue dost thou meditate?");
#endif
		virtue = ReadStringController::get(32, TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);
#ifdef IOS_ULTIMA4
	}
#endif

	int choice;
	g_screen->screenMessage("\n\nFor how many Cycles (0-3)? ");
#ifdef IOS_ULTIMA4
	{
		U4IOS::IOSConversationChoiceHelper cyclesChoice;
		cyclesChoice.updateChoices("0123 \015\033");
#endif
		choice = ReadChoiceController::get("0123\015\033");
#ifdef IOS_ULTIMA4
	}
#endif
	if (choice == '\033' || choice == '\015')
		g_shrines->_cycles = 0;
	else
		g_shrines->_cycles = choice - '0';
	g_shrines->_completedCycles = 0;

	g_screen->screenMessage("\n\n");

	// ensure the player chose the right virtue and entered a valid number for cycles
	if (scumm_strnicmp(virtue.c_str(), getVirtueName(getVirtue()), 6) != 0 || g_shrines->_cycles == 0) {
		g_screen->screenMessage("Thou art unable to focus thy thoughts on this subject!\n");
		eject();
		return;
	}

	if (((g_ultima->_saveGame->_moves / SHRINE_MEDITATION_INTERVAL) >= 0x10000) ||
	        (((g_ultima->_saveGame->_moves / SHRINE_MEDITATION_INTERVAL) & 0xffff) != g_ultima->_saveGame->_lastMeditation)) {
		g_screen->screenMessage("Begin Meditation\n");
		meditationCycle();
	} else {
		g_screen->screenMessage("Thy mind is still weary from thy last Meditation!\n");
		eject();
	}
}

void Shrine::enhancedSequence() {
	// Replace the 'static' avatar tile with grass
	_annotations->add(Coords(5, 6, g_context->_location->_coords.z), _tileSet->getByName("grass")->getId(), false, true);

	g_screen->screenDisableCursor();
	g_screen->screenMessage("You approach\nthe ancient\nshrine...\n");
	gameUpdateScreen();
	EventHandler::wait_cycles(settings._gameCyclesPerSecond);

	Object *obj = addCreature(creatureMgr->getById(BEGGAR_ID), Coords(5, 10, g_context->_location->_coords.z));
	obj->setTile(_tileSet->getByName("avatar")->getId());

	gameUpdateScreen();
	EventHandler::wait_msecs(400);
	g_context->_location->_map->move(obj, DIR_NORTH);
	gameUpdateScreen();
	EventHandler::wait_msecs(400);
	g_context->_location->_map->move(obj, DIR_NORTH);
	gameUpdateScreen();
	EventHandler::wait_msecs(400);
	g_context->_location->_map->move(obj, DIR_NORTH);
	gameUpdateScreen();
	EventHandler::wait_msecs(400);
	g_context->_location->_map->move(obj, DIR_NORTH);
	gameUpdateScreen();
	EventHandler::wait_msecs(800);
	obj->setTile(creatureMgr->getById(BEGGAR_ID)->getTile());
	gameUpdateScreen();

	g_screen->screenMessage("\n...and kneel before the altar.\n");
	EventHandler::wait_cycles(settings._gameCyclesPerSecond);
	g_screen->screenEnableCursor();
}

void Shrine::meditationCycle() {
	// Find our interval for meditation
	int interval = (settings._shrineTime * 1000) / MEDITATION_MANTRAS_PER_CYCLE;
	interval -= (interval % settings._eventTimerGranularity);
	interval /= settings._eventTimerGranularity;
	if (interval <= 0)
		interval = 1;

	g_ultima->_saveGame->_lastMeditation = (g_ultima->_saveGame->_moves / SHRINE_MEDITATION_INTERVAL) & 0xffff;

	g_screen->screenDisableCursor();
	for (int i = 0; i < MEDITATION_MANTRAS_PER_CYCLE; i++) {
		WaitController controller(interval);
		eventHandler->pushController(&controller);
		controller.wait();
		g_screen->screenMessage(".");
		g_screen->update();
	}
	askMantra();
}

void Shrine::askMantra() {
	g_screen->screenEnableCursor();
	g_screen->screenMessage("\nMantra: ");
	g_screen->update();       // FIXME: needed?
	Common::String mantra;
#ifdef IOS_ULTIMA4
	{
		U4IOS::IOSConversationHelper mantraHelper;
		mantraHelper.beginConversation(U4IOS::UIKeyboardTypeASCIICapable, "Mantra?");
#endif
		mantra = ReadStringController::get(4, TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);
		g_screen->screenMessage("\n");
#ifdef IOS_ULTIMA4
	}
#endif

	if (scumm_stricmp(mantra.c_str(), getMantra().c_str()) != 0) {
		g_context->_party->adjustKarma(KA_BAD_MANTRA);
		g_screen->screenMessage("Thou art not able to focus thy thoughts with that Mantra!\n");
		eject();
	} else if (--g_shrines->_cycles > 0) {
		g_shrines->_completedCycles++;
		g_context->_party->adjustKarma(KA_MEDITATION);
		meditationCycle();
	} else {
		g_shrines->_completedCycles++;
		g_context->_party->adjustKarma(KA_MEDITATION);

		bool elevated = g_shrines->_completedCycles == 3 && g_context->_party->attemptElevation(getVirtue());
		if (elevated)
			g_screen->screenMessage("\nThou hast achieved partial Avatarhood in the Virtue of %s\n\n",
			              getVirtueName(getVirtue()));
		else
			g_screen->screenMessage("\nThy thoughts are pure. "
			              "Thou art granted a vision!\n");

#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationChoiceHelper choiceDialog;
		choiceDialog.updateChoices(" ");
		U4IOS::testFlightPassCheckPoint(Common::String("Gained avatarhood in: ")
		                                + getVirtueName(getVirtue()));
#endif
		ReadChoiceController::get("");
		showVision(elevated);
		ReadChoiceController::get("");
		gameSetViewMode(VIEW_NORMAL);
		eject();
	}
}

void Shrine::showVision(bool elevated) {
	static const char *visionImageNames[] = {
		BKGD_SHRINE_HON, BKGD_SHRINE_COM, BKGD_SHRINE_VAL, BKGD_SHRINE_JUS,
		BKGD_SHRINE_SAC, BKGD_SHRINE_HNR, BKGD_SHRINE_SPI, BKGD_SHRINE_HUM
	};

	if (elevated) {
		g_screen->screenMessage("Thou art granted a vision!\n");
		gameSetViewMode(VIEW_RUNE);
		g_screen->screenDrawImageInMapArea(visionImageNames[getVirtue()]);
	} else {
		g_screen->screenMessage("\n%s", g_shrines->_advice[
			getVirtue() * 3 + g_shrines->_completedCycles - 1].c_str());
	}
}

void Shrine::eject() {
	g_game->exitToParentMap();
	g_music->playMapMusic();
	g_context->_location->_turnCompleter->finishTurn();
}

} // End of namespace Ultima4
} // End of namespace Ultima
