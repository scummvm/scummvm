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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/shrine.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/graphics/imagemgr.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/portal.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/types.h"

namespace Ultima {
namespace Ultima4 {

using Common::String;
using Std::vector;

int cycles, completedCycles;
Std::vector<Common::String> shrineAdvice;

/**
 * Returns true if the player can use the portal to the shrine
 */
bool shrineCanEnter(const Portal *p) {
	Shrine *shrine = dynamic_cast<Shrine *>(mapMgr->get(p->_destid));
	if (!g_context->_party->canEnterShrine(shrine->getVirtue())) {
		screenMessage("Thou dost not bear the rune of entry!  A strange force keeps you out!\n");
		return 0;
	}
	return 1;
}

/**
 * Returns true if 'map' points to a Shrine map
 */
bool isShrine(Map *punknown) {
	Shrine *ps;
	if ((ps = dynamic_cast<Shrine *>(punknown)) != NULL)
		return true;
	else
		return false;
}

/**
 * Shrine class implementation
 */
Shrine::Shrine() {}

Common::String Shrine::getName() {
	if (_name.empty()) {
		_name = "Shrine of ";
		_name += getVirtueName(_virtue);
	}
	return _name;
}
Virtue Shrine::getVirtue() const    {
	return _virtue;
}
Common::String Shrine::getMantra() const    {
	return _mantra;
}

void Shrine::setVirtue(Virtue v)    {
	_virtue = v;
}
void Shrine::setMantra(Common::String m)    {
	_mantra = m;
}

/**
 * Enter the shrine
 */
void Shrine::enter() {

	if (shrineAdvice.empty()) {
		U4FILE *avatar = u4fopen("avatar.exe");
		if (!avatar)
			return;
		shrineAdvice = u4read_stringtable(avatar, 93682, 24);
		u4fclose(avatar);
	}
#ifdef IOS
	U4IOS::IOSHideGameControllerHelper hideControllsHelper;
#endif
	if (settings._enhancements && settings._enhancementsOptions._u5shrines)
		enhancedSequence();
	else
		screenMessage("You enter the ancient shrine and sit before the altar...");

	screenMessage("\nUpon which virtue dost thou meditate?\n");
	Common::String virtue;
#ifdef IOS
	{
		U4IOS::IOSConversationHelper inputVirture;
		inputVirture.beginConversation(U4IOS::UIKeyboardTypeDefault, "Upon which virtue dost thou meditate?");
#endif
		virtue = ReadStringController::get(32, TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
#ifdef IOS
	}
#endif

	int choice;
	screenMessage("\n\nFor how many Cycles (0-3)? ");
#ifdef IOS
	{
		U4IOS::IOSConversationChoiceHelper cyclesChoice;
		cyclesChoice.updateChoices("0123 \015\033");
#endif
		choice = ReadChoiceController::get("0123\015\033");
#ifdef IOS
	}
#endif
	if (choice == '\033' || choice == '\015')
		cycles = 0;
	else
		cycles = choice - '0';
	completedCycles = 0;

	screenMessage("\n\n");

	// ensure the player chose the right virtue and entered a valid number for cycles
	if (scumm_strnicmp(virtue.c_str(), getVirtueName(getVirtue()), 6) != 0 || cycles == 0) {
		screenMessage("Thou art unable to focus thy thoughts on this subject!\n");
		eject();
		return;
	}

	if (((g_context->_saveGame->_moves / SHRINE_MEDITATION_INTERVAL) >= 0x10000) ||
	        (((g_context->_saveGame->_moves / SHRINE_MEDITATION_INTERVAL) & 0xffff) != g_context->_saveGame->_lastMeditation)) {
		screenMessage("Begin Meditation\n");
		meditationCycle();
	} else {
		screenMessage("Thy mind is still weary from thy last Meditation!\n");
		eject();
	}
}

void Shrine::enhancedSequence() {
	/* replace the 'static' avatar tile with grass */
	_annotations->add(Coords(5, 6, g_context->_location->_coords.z), _tileset->getByName("grass")->getId(), false, true);

	screenDisableCursor();
	screenMessage("You approach\nthe ancient\nshrine...\n");
	gameUpdateScreen();
	EventHandler::wait_cycles(settings._gameCyclesPerSecond);

	Object *obj = addCreature(creatureMgr->getById(BEGGAR_ID), Coords(5, 10, g_context->_location->_coords.z));
	obj->setTile(_tileset->getByName("avatar")->getId());

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

	screenMessage("\n...and kneel before the altar.\n");
	EventHandler::wait_cycles(settings._gameCyclesPerSecond);
	screenEnableCursor();
}

void Shrine::meditationCycle() {
	/* find our interval for meditation */
	int interval = (settings._shrineTime * 1000) / MEDITATION_MANTRAS_PER_CYCLE;
	interval -= (interval % eventTimerGranularity);
	interval /= eventTimerGranularity;
	if (interval <= 0)
		interval = 1;

	g_context->_saveGame->_lastMeditation = (g_context->_saveGame->_moves / SHRINE_MEDITATION_INTERVAL) & 0xffff;

	screenDisableCursor();
	for (int i = 0; i < MEDITATION_MANTRAS_PER_CYCLE; i++) {
		WaitController controller(interval);
		eventHandler->pushController(&controller);
		controller.wait();
		screenMessage(".");
		screenRedrawScreen();
	}
	askMantra();
}

void Shrine::askMantra() {
	screenEnableCursor();
	screenMessage("\nMantra: ");
	screenRedrawScreen();       // FIXME: needed?
	Common::String mantra;
#ifdef IOS
	{
		U4IOS::IOSConversationHelper mantraHelper;
		mantraHelper.beginConversation(U4IOS::UIKeyboardTypeASCIICapable, "Mantra?");
#endif
		mantra = ReadStringController::get(4, TEXT_AREA_X + g_context->col, TEXT_AREA_Y + g_context->_line);
		screenMessage("\n");
#ifdef IOS
	}
#endif

	if (scumm_stricmp(mantra.c_str(), getMantra().c_str()) != 0) {
		g_context->_party->adjustKarma(KA_BAD_MANTRA);
		screenMessage("Thou art not able to focus thy thoughts with that Mantra!\n");
		eject();
	} else if (--cycles > 0) {
		completedCycles++;
		g_context->_party->adjustKarma(KA_MEDITATION);
		meditationCycle();
	} else {
		completedCycles++;
		g_context->_party->adjustKarma(KA_MEDITATION);

		bool elevated = completedCycles == 3 && g_context->_party->attemptElevation(getVirtue());
		if (elevated)
			screenMessage("\nThou hast achieved partial Avatarhood in the Virtue of %s\n\n",
			              getVirtueName(getVirtue()));
		else
			screenMessage("\nThy thoughts are pure. "
			              "Thou art granted a vision!\n");

#ifdef IOS
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
		screenMessage("Thou art granted a vision!\n");
		gameSetViewMode(VIEW_RUNE);
		screenDrawImageInMapArea(visionImageNames[getVirtue()]);
	} else {
		screenMessage("\n%s", shrineAdvice[getVirtue() * 3 + completedCycles - 1].c_str());
	}
}

void Shrine::eject() {
	g_game->exitToParentMap();
	musicMgr->play();
	g_context->_location->_turnCompleter->finishTurn();
}

} // End of namespace Ultima4
} // End of namespace Ultima
