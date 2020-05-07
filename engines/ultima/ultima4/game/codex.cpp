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

#include "ultima/ultima4/game/codex.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/gfx/imagemgr.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

Codex *g_codex;

Codex::Codex() {
	g_codex = this;
}

Codex::~Codex() {
	g_codex = nullptr;
}

int Codex::init() {
	_virtueQuestions = u4read_stringtable("virtue");
	_endgameText1 = u4read_stringtable("endgame1");
	_endgameText2 = u4read_stringtable("endgame2");

	return 1;
}

void Codex::deinit() {
	_virtueQuestions.clear();
	_endgameText1.clear();
	_endgameText2.clear();
}

void Codex::start() {
	init();

	// Disable the whirlpool cursor and black out the screen	
#ifdef IOS_ULTIMA4
	U4IOS::IOSHideGameControllerHelper hideControllsHelper;
#endif
	g_screen->screenDisableCursor();
	g_screen->screenUpdate(&g_game->_mapArea, false, true);

	// Make the avatar alone
	
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	g_context->_stats->update(true);     // show just the avatar
	g_screen->update();

	// Change the view mode so the dungeon doesn't get shown
	
	gameSetViewMode(VIEW_CODEX);

	g_screen->screenMessage("\n\n\n\nThere is a sudden darkness, and you find yourself alone in an empty chamber.\n");
	EventHandler::sleep(4000);

	// Check to see if you have the 3-part key	
	if ((g_ultima->_saveGame->_items & (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T)) != (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T)) {
		eject(CODEX_EJECT_NO_3_PART_KEY);
		return;
	}

	g_screen->screenDrawImageInMapArea(BKGD_KEY);
	g_screen->screenRedrawMapArea();

	g_screen->screenMessage("\nYou use your key of Three Parts.\n");
	EventHandler::sleep(3000);

	g_screen->screenMessage("\nA voice rings out:\n\"What is the Word of Passage?\"\n\n");

	// Get the Word of Passage	
#ifdef IOS_ULTIMA4
	U4IOS::IOSConversationHelper::setIntroString("What is the Word of Passage?");
#endif
	handleWOP(gameGetInput());
}

void Codex::eject(CodexEjectCode code) {
	const struct {
		int x, y;
	} startLocations[] = {
		{ 231, 136 },
		{ 83, 105 },
		{ 35, 221 },
		{ 59, 44 },
		{ 158, 21 },
		{ 105, 183 },
		{ 23, 129 },
		{ 186, 171 }
	};

	switch (code) {
	case CODEX_EJECT_NO_3_PART_KEY:
		g_screen->screenMessage("\nThou dost not have the Key of Three Parts.\n\n");
		break;
	case CODEX_EJECT_NO_FULL_PARTY:
		g_screen->screenMessage("\nThou hast not proved thy leadership in all eight virtues.\n\n");
		EventHandler::sleep(2000);
		g_screen->screenMessage("\nPassage is not granted.\n\n");
		break;
	case CODEX_EJECT_NO_FULL_AVATAR:
		g_screen->screenMessage("\nThou art not ready.\n");
		EventHandler::sleep(2000);
		g_screen->screenMessage("\nPassage is not granted.\n\n");
		break;
	case CODEX_EJECT_BAD_WOP:
		g_screen->screenMessage("\nPassage is not granted.\n\n");
		break;
	case CODEX_EJECT_HONESTY:
	case CODEX_EJECT_COMPASSION:
	case CODEX_EJECT_VALOR:
	case CODEX_EJECT_JUSTICE:
	case CODEX_EJECT_SACRIFICE:
	case CODEX_EJECT_HONOR:
	case CODEX_EJECT_SPIRITUALITY:
	case CODEX_EJECT_HUMILITY:
	case CODEX_EJECT_TRUTH:
	case CODEX_EJECT_LOVE:
	case CODEX_EJECT_COURAGE:
		g_screen->screenMessage("\nThy quest is not yet complete.\n\n");
		break;
	case CODEX_EJECT_BAD_INFINITY:
		g_screen->screenMessage("\nThou dost not know the true nature of the Universe.\n\n");
		break;
	default:
		g_screen->screenMessage("\nOops, you just got too close to beating the game.\nBAD AVATAR!\n");
		break;
	}

	EventHandler::sleep(2000);

	// Free memory associated with the Codex
	deinit();

	// Re-enable the cursor and show it
	g_screen->screenEnableCursor();
	g_screen->screenShowCursor();

	// Return view to normal and exit the Abyss
	gameSetViewMode(VIEW_NORMAL);
	g_game->exitToParentMap();
	g_music->playMapMusic();

	/*
	 * if being ejected because of a missed virtue question,
	 * then teleport the party to the starting location for
	 * that virtue.
	 */
	if (code >= CODEX_EJECT_HONESTY && code <= CODEX_EJECT_HUMILITY) {
		int virtue = code - CODEX_EJECT_HONESTY;
		g_context->_location->_coords.x = startLocations[virtue].x;
		g_context->_location->_coords.y = startLocations[virtue].y;
	}

	// finally, finish the turn
	g_context->_location->_turnCompleter->finishTurn();
	eventHandler->setController(g_game);
}

void Codex::handleWOP(const Common::String &word) {
	static int tries = 1;
	int i;

	eventHandler->popKeyHandler();

	// slight pause before continuing
	g_screen->screenMessage("\n");
	g_screen->screenDisableCursor();
	EventHandler::sleep(1000);

	// entered correctly
	if (scumm_stricmp(word.c_str(), "veramocor") == 0) {
		tries = 1; // reset 'tries' in case we need to enter this again later

		// eject them if they don't have all 8 party members
		if (g_ultima->_saveGame->_members != 8) {
			eject(CODEX_EJECT_NO_FULL_PARTY);
			return;
		}

		// eject them if they're not a full avatar at this point
		for (i = 0; i < VIRT_MAX; i++) {
			if (g_ultima->_saveGame->_karma[i] != 0) {
				eject(CODEX_EJECT_NO_FULL_AVATAR);
				return;
			}
		}

		g_screen->screenMessage("\nPassage is granted.\n");
		EventHandler::sleep(4000);

		g_screen->screenEraseMapArea();
		g_screen->screenRedrawMapArea();

		// Ask the Virtue questions
		g_screen->screenMessage("\n\nThe voice asks:\n");
		EventHandler::sleep(2000);
		g_screen->screenMessage("\n%s\n\n", _virtueQuestions[0].c_str());

		handleVirtues(gameGetInput());

		return;
	}

	// entered incorrectly - give 3 tries before ejecting
	else if (tries++ < 3) {
		impureThoughts();
		g_screen->screenMessage("\"What is the Word of Passage?\"\n\n");
#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationHelper::setIntroString("Which virtue?");
#endif
		handleWOP(gameGetInput());
	}

	// 3 tries are up... eject!
	else {
		tries = 1;
		eject(CODEX_EJECT_BAD_WOP);
	}
}

void Codex::handleVirtues(const Common::String &virtue) {
	static const char *codexImageNames[] = {
		BKGD_HONESTY, BKGD_COMPASSN, BKGD_VALOR, BKGD_JUSTICE,
		BKGD_SACRIFIC, BKGD_HONOR, BKGD_SPIRIT, BKGD_HUMILITY,
		BKGD_TRUTH, BKGD_LOVE, BKGD_COURAGE
	};

	static int current = 0;
	static int tries = 1;

	eventHandler->popKeyHandler();

	// slight pause before continuing
	g_screen->screenMessage("\n");
	g_screen->screenDisableCursor();
	EventHandler::sleep(1000);

	// answered with the correct one of eight virtues
	if ((current < VIRT_MAX) &&
	        (scumm_stricmp(virtue.c_str(), getVirtueName(static_cast<Virtue>(current))) == 0)) {

		g_screen->screenDrawImageInMapArea(codexImageNames[current]);
		g_screen->screenRedrawMapArea();

		current++;
		tries = 1;

		EventHandler::sleep(2000);

		if (current == VIRT_MAX) {
			g_screen->screenMessage("\nThou art well versed in the virtues of the Avatar.\n");
			EventHandler::sleep(5000);
		}

		g_screen->screenMessage("\n\nThe voice asks:\n");
		EventHandler::sleep(2000);
		g_screen->screenMessage("\n%s\n\n", _virtueQuestions[current].c_str());
#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationHelper::setIntroString((current != VIRT_MAX) ? "Which virtue?" : "Which principle?");
#endif
		handleVirtues(gameGetInput());
	}

	// answered with the correct base virtue (truth, love, courage)
	else if ((current >= VIRT_MAX) &&
	         (scumm_stricmp(virtue.c_str(), getBaseVirtueName(static_cast<BaseVirtue>(1 << (current - VIRT_MAX)))) == 0)) {

		g_screen->screenDrawImageInMapArea(codexImageNames[current]);
		g_screen->screenRedrawMapArea();

		current++;
		tries = 1;

		if (current < VIRT_MAX + 3) {
			g_screen->screenMessage("\n\nThe voice asks:\n");
			EventHandler::sleep(2000);
			g_screen->screenMessage("\n%s\n\n", _virtueQuestions[current].c_str());
#ifdef IOS_ULTIMA4
			U4IOS::IOSConversationHelper::setIntroString("Which principle?");
#endif
			handleVirtues(gameGetInput());
		} else {
			g_screen->screenMessage("\nThe ground rumbles beneath your feet.\n");
			EventHandler::sleep(1000);
			g_screen->screenShake(10);

			EventHandler::sleep(3000);
			g_screen->screenEnableCursor();
			g_screen->screenMessage("\nAbove the din, the voice asks:\n\nIf all eight virtues of the Avatar combine into and are derived from the Three Principles of Truth, Love and Courage...");
#ifdef IOS_ULTIMA4
			// Ugh, we now enter happy callback land, so I know how to do these things manually. Good thing I kept these separate functions.
			U4IOS::beginChoiceConversation();
			U4IOS::updateChoicesInDialog(" ", "", -1);
#endif
			eventHandler->pushKeyHandler(&handleInfinityAnyKey);
		}
	}

	// give them 3 tries to enter the correct virtue, then eject them!
	else if (tries++ < 3) {
		impureThoughts();
		g_screen->screenMessage("%s\n\n", _virtueQuestions[current].c_str());
#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationHelper::setIntroString("Which virtue?");
#endif
		handleVirtues(gameGetInput());
	}

	// failed 3 times... eject!
	else {
		eject(static_cast<CodexEjectCode>(CODEX_EJECT_HONESTY + current));

		tries = 1;
		current = 0;
	}
}

bool Codex::handleInfinityAnyKey(int key, void *data) {
	eventHandler->popKeyHandler();

	g_screen->screenMessage("\n\nThen what is the one thing which encompasses and is the whole of all undeniable Truth, unending Love, and unyielding Courage?\n\n");
#ifdef IOS_ULTIMA4
	U4IOS::endChoiceConversation();
	U4IOS::IOSConversationHelper::setIntroString("What is the whole of all undeniable Truth, unending Love, and unyielding Courage?");
#endif
	g_codex->handleInfinity(gameGetInput());
	return true;
}

void Codex::handleInfinity(const Common::String &answer) {
	static int tries = 1;

	eventHandler->popKeyHandler();
#ifdef IOS_ULTIMA4
	U4IOS::IOSHideGameControllerHelper hideControllsHelper;
#endif
	// slight pause before continuing
	g_screen->screenMessage("\n");
	g_screen->screenDisableCursor();
	EventHandler::sleep(1000);

	if (scumm_stricmp(answer.c_str(), "infinity") == 0) {
		EventHandler::sleep(2000);
		g_screen->screenShake(10);

		g_screen->screenEnableCursor();
		g_screen->screenMessage("\n%s", _endgameText1[0].c_str());
#ifdef IOS_ULTIMA4
		// Ugh, we now enter happy callback land, so I know how to do these things manually. Good thing I kept these separate functions.
		U4IOS::hideGameButtons();
		U4IOS::beginChoiceConversation();
		U4IOS::updateChoicesInDialog(" ", "", -1);
		U4IOS::testFlightPassCheckPoint("Game won!");
#endif
		eventHandler->pushKeyHandler(&handleEndgameAnyKey);
	} else if (tries++ < 3) {
		impureThoughts();
		g_screen->screenMessage("\nAbove the din, the voice asks:\n\nIf all eight virtues of the Avatar combine into and are derived from the Three Principles of Truth, Love and Courage...");
		eventHandler->pushKeyHandler(&handleInfinityAnyKey);
	} else eject(CODEX_EJECT_BAD_INFINITY);
}

bool Codex::handleEndgameAnyKey(int key, void *data) {
	static int index = 1;

	eventHandler->popKeyHandler();

	if (index < 10) {

		if (index < 7) {
			if (index == 6) {
				g_screen->screenEraseMapArea();
				g_screen->screenRedrawMapArea();
			}
			g_screen->screenMessage("%s", g_codex->_endgameText1[index].c_str());
		} else if (index == 7) {
			g_screen->screenDrawImageInMapArea(BKGD_STONCRCL);
			g_screen->screenRedrawMapArea();
			g_screen->screenMessage("\n\n%s", g_codex->_endgameText2[index - 7].c_str());
		} else if (index > 7)
			g_screen->screenMessage("%s", g_codex->_endgameText2[index - 7].c_str());

		index++;
		eventHandler->pushKeyHandler(&g_codex->handleEndgameAnyKey);
	} else {
		// CONGRATULATIONS!... you have completed the game in x turns
		g_screen->screenDisableCursor();
		g_screen->screenMessage("%s%d%s", g_codex->_endgameText2[index - 7].c_str(),
			g_ultima->_saveGame->_moves, g_codex->_endgameText2[index - 6].c_str());
#ifdef IOS_ULTIMA4
		U4IOS::endChoiceConversation();
#endif
		eventHandler->pushKeyHandler(&KeyHandler::ignoreKeys);
	}

	return true;
}

void Codex::impureThoughts() {
	g_screen->screenMessage("\nThy thoughts are not pure.\nI ask again.\n");
	EventHandler::sleep(2000);
}

} // End of namespace Ultima4
} // End of namespace Ultima
