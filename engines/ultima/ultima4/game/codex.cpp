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
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/graphics/imagemgr.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/screen.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

using namespace std;

int codexInit();
void codexDelete();
void codexEject(CodexEjectCode code);
void codexHandleWOP(const Common::String &word);
void codexHandleVirtues(const Common::String &virtue);
void codexHandleInfinity(const Common::String &answer);
void codexImpureThoughts();

/**
 * Key handlers
 */
bool codexHandleInfinityAnyKey(int key, void *data);
bool codexHandleEndgameAnyKey(int key, void *data);

Std::vector<Common::String> codexVirtueQuestions;
Std::vector<Common::String> codexEndgameText1;
Std::vector<Common::String> codexEndgameText2;

/**
 * Initializes the Chamber of the Codex sequence (runs from codexStart())
 */
int codexInit() {
	U4FILE *avatar;

	avatar = u4fopen("avatar.exe");
	if (!avatar)
		return 0;

	codexVirtueQuestions = u4read_stringtable(avatar, 0x0fc7b, 11);
	codexEndgameText1 = u4read_stringtable(avatar, 0x0fee4, 7);
	codexEndgameText2 = u4read_stringtable(avatar, 0x10187, 5);

	u4fclose(avatar);

	return 1;
}

/**
 * Frees all memory associated with the Codex sequence
 */
void codexDelete() {
	codexVirtueQuestions.clear();
	codexEndgameText1.clear();
	codexEndgameText2.clear();
}

/**
 * Begins the Chamber of the Codex sequence
 */
void codexStart() {
	codexInit();

	/**
	 * disable the whirlpool cursor and black out the screen
	 */
#ifdef IOS
	U4IOS::IOSHideGameControllerHelper hideControllsHelper;
#endif
	screenDisableCursor();
	screenUpdate(&g_game->_mapArea, false, true);

	/**
	 * make the avatar alone
	 */
	g_context->_stats->setView(STATS_PARTY_OVERVIEW);
	g_context->_stats->update(true);     /* show just the avatar */
	screenRedrawScreen();

	/**
	 * change the view mode so the dungeon doesn't get shown
	 */
	gameSetViewMode(VIEW_CODEX);

	screenMessage("\n\n\n\nThere is a sudden darkness, and you find yourself alone in an empty chamber.\n");
	EventHandler::sleep(4000);

	/**
	 * check to see if you have the 3-part key
	 */
	if ((g_context->_saveGame->_items & (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T)) != (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T)) {
		codexEject(CODEX_EJECT_NO_3_PART_KEY);
		return;
	}

	screenDrawImageInMapArea(BKGD_KEY);
	screenRedrawMapArea();

	screenMessage("\nYou use your key of Three Parts.\n");
	EventHandler::sleep(3000);

	screenMessage("\nA voice rings out:\n\"What is the Word of Passage?\"\n\n");

	/**
	 * Get the Word of Passage
	 */
#ifdef IOS
	U4IOS::IOSConversationHelper::setIntroString("What is the Word of Passage?");
#endif
	codexHandleWOP(gameGetInput());
}

/**
 * Ejects you from the chamber of the codex (and the Abyss, for that matter)
 * with the correct message.
 */
void codexEject(CodexEjectCode code) {
	struct {
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
		screenMessage("\nThou dost not have the Key of Three Parts.\n\n");
		break;
	case CODEX_EJECT_NO_FULL_PARTY:
		screenMessage("\nThou hast not proved thy leadership in all eight virtues.\n\n");
		EventHandler::sleep(2000);
		screenMessage("\nPassage is not granted.\n\n");
		break;
	case CODEX_EJECT_NO_FULL_AVATAR:
		screenMessage("\nThou art not ready.\n");
		EventHandler::sleep(2000);
		screenMessage("\nPassage is not granted.\n\n");
		break;
	case CODEX_EJECT_BAD_WOP:
		screenMessage("\nPassage is not granted.\n\n");
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
		screenMessage("\nThy quest is not yet complete.\n\n");
		break;
	case CODEX_EJECT_BAD_INFINITY:
		screenMessage("\nThou dost not know the true nature of the Universe.\n\n");
		break;
	default:
		screenMessage("\nOops, you just got too close to beating the game.\nBAD AVATAR!\n");
		break;
	}

	EventHandler::sleep(2000);

	/* free memory associated with the Codex */
	codexDelete();

	/* re-enable the cursor and show it */
	screenEnableCursor();
	screenShowCursor();

	/* return view to normal and exit the Abyss */
	gameSetViewMode(VIEW_NORMAL);
	g_game->exitToParentMap();
	musicMgr->play();

	/**
	 * if being ejected because of a missed virtue question,
	 * then teleport the party to the starting location for
	 * that virtue.
	 */
	if (code >= CODEX_EJECT_HONESTY && code <= CODEX_EJECT_HUMILITY) {
		int virtue = code - CODEX_EJECT_HONESTY;
		g_context->_location->_coords.x = startLocations[virtue].x;
		g_context->_location->_coords.y = startLocations[virtue].y;
	}

	/* finally, finish the turn */
	g_context->_location->_turnCompleter->finishTurn();
	eventHandler->setController(g_game);
}

/**
 * Handles entering the Word of Passage
 */
void codexHandleWOP(const Common::String &word) {
	static int tries = 1;
	int i;

	eventHandler->popKeyHandler();

	/* slight pause before continuing */
	screenMessage("\n");
	screenDisableCursor();
	EventHandler::sleep(1000);

	/* entered correctly */
	if (scumm_stricmp(word.c_str(), "veramocor") == 0) {
		tries = 1; /* reset 'tries' in case we need to enter this again later */

		/* eject them if they don't have all 8 party members */
		if (g_context->_saveGame->_members != 8) {
			codexEject(CODEX_EJECT_NO_FULL_PARTY);
			return;
		}

		/* eject them if they're not a full avatar at this point */
		for (i = 0; i < VIRT_MAX; i++) {
			if (g_context->_saveGame->_karma[i] != 0) {
				codexEject(CODEX_EJECT_NO_FULL_AVATAR);
				return;
			}
		}

		screenMessage("\nPassage is granted.\n");
		EventHandler::sleep(4000);

		screenEraseMapArea();
		screenRedrawMapArea();

		/* Ask the Virtue questions */
		screenMessage("\n\nThe voice asks:\n");
		EventHandler::sleep(2000);
		screenMessage("\n%s\n\n", codexVirtueQuestions[0].c_str());

		codexHandleVirtues(gameGetInput());

		return;
	}

	/* entered incorrectly - give 3 tries before ejecting */
	else if (tries++ < 3) {
		codexImpureThoughts();
		screenMessage("\"What is the Word of Passage?\"\n\n");
#ifdef IOS
		U4IOS::IOSConversationHelper::setIntroString("Which virtue?");
#endif
		codexHandleWOP(gameGetInput());
	}

	/* 3 tries are up... eject! */
	else {
		tries = 1;
		codexEject(CODEX_EJECT_BAD_WOP);
	}
}

/**
 * Handles naming of virtues in the Chamber of the Codex
 */
void codexHandleVirtues(const Common::String &virtue) {
	static const char *codexImageNames[] = {
		BKGD_HONESTY, BKGD_COMPASSN, BKGD_VALOR, BKGD_JUSTICE,
		BKGD_SACRIFIC, BKGD_HONOR, BKGD_SPIRIT, BKGD_HUMILITY,
		BKGD_TRUTH, BKGD_LOVE, BKGD_COURAGE
	};

	static int current = 0;
	static int tries = 1;

	eventHandler->popKeyHandler();

	/* slight pause before continuing */
	screenMessage("\n");
	screenDisableCursor();
	EventHandler::sleep(1000);

	/* answered with the correct one of eight virtues */
	if ((current < VIRT_MAX) &&
	        (scumm_stricmp(virtue.c_str(), getVirtueName(static_cast<Virtue>(current))) == 0)) {

		screenDrawImageInMapArea(codexImageNames[current]);
		screenRedrawMapArea();

		current++;
		tries = 1;

		EventHandler::sleep(2000);

		if (current == VIRT_MAX) {
			screenMessage("\nThou art well versed in the virtues of the Avatar.\n");
			EventHandler::sleep(5000);
		}

		screenMessage("\n\nThe voice asks:\n");
		EventHandler::sleep(2000);
		screenMessage("\n%s\n\n", codexVirtueQuestions[current].c_str());
#ifdef IOS
		U4IOS::IOSConversationHelper::setIntroString((current != VIRT_MAX) ? "Which virtue?" : "Which principle?");
#endif
		codexHandleVirtues(gameGetInput());
	}

	/* answered with the correct base virtue (truth, love, courage) */
	else if ((current >= VIRT_MAX) &&
	         (scumm_stricmp(virtue.c_str(), getBaseVirtueName(static_cast<BaseVirtue>(1 << (current - VIRT_MAX)))) == 0)) {

		screenDrawImageInMapArea(codexImageNames[current]);
		screenRedrawMapArea();

		current++;
		tries = 1;

		if (current < VIRT_MAX + 3) {
			screenMessage("\n\nThe voice asks:\n");
			EventHandler::sleep(2000);
			screenMessage("\n%s\n\n", codexVirtueQuestions[current].c_str());
#ifdef IOS
			U4IOS::IOSConversationHelper::setIntroString("Which principle?");
#endif
			codexHandleVirtues(gameGetInput());
		} else {
			screenMessage("\nThe ground rumbles beneath your feet.\n");
			EventHandler::sleep(1000);
			screenShake(10);

			EventHandler::sleep(3000);
			screenEnableCursor();
			screenMessage("\nAbove the din, the voice asks:\n\nIf all eight virtues of the Avatar combine into and are derived from the Three Principles of Truth, Love and Courage...");
#ifdef IOS
			// Ugh, we now enter happy callback land, so I know how to do these things manually. Good thing I kept these separate functions.
			U4IOS::beginChoiceConversation();
			U4IOS::updateChoicesInDialog(" ", "", -1);
#endif
			eventHandler->pushKeyHandler(&codexHandleInfinityAnyKey);
		}
	}

	/* give them 3 tries to enter the correct virtue, then eject them! */
	else if (tries++ < 3) {
		codexImpureThoughts();
		screenMessage("%s\n\n", codexVirtueQuestions[current].c_str());
#ifdef IOS
		U4IOS::IOSConversationHelper::setIntroString("Which virtue?");
#endif
		codexHandleVirtues(gameGetInput());
	}

	/* failed 3 times... eject! */
	else {
		codexEject(static_cast<CodexEjectCode>(CODEX_EJECT_HONESTY + current));

		tries = 1;
		current = 0;
	}
}

bool codexHandleInfinityAnyKey(int key, void *data) {
	eventHandler->popKeyHandler();

	screenMessage("\n\nThen what is the one thing which encompasses and is the whole of all undeniable Truth, unending Love, and unyielding Courage?\n\n");
#ifdef IOS
	U4IOS::endChoiceConversation();
	U4IOS::IOSConversationHelper::setIntroString("What is the whole of all undeniable Truth, unending Love, and unyielding Courage?");
#endif
	codexHandleInfinity(gameGetInput());
	return true;
}

void codexHandleInfinity(const Common::String &answer) {
	static int tries = 1;

	eventHandler->popKeyHandler();
#ifdef IOS
	U4IOS::IOSHideGameControllerHelper hideControllsHelper;
#endif
	/* slight pause before continuing */
	screenMessage("\n");
	screenDisableCursor();
	EventHandler::sleep(1000);

	if (scumm_stricmp(answer.c_str(), "infinity") == 0) {
		EventHandler::sleep(2000);
		screenShake(10);

		screenEnableCursor();
		screenMessage("\n%s", codexEndgameText1[0].c_str());
#ifdef IOS
		// Ugh, we now enter happy callback land, so I know how to do these things manually. Good thing I kept these separate functions.
		U4IOS::hideGameButtons();
		U4IOS::beginChoiceConversation();
		U4IOS::updateChoicesInDialog(" ", "", -1);
		U4IOS::testFlightPassCheckPoint("Game won!");
#endif
		eventHandler->pushKeyHandler(&codexHandleEndgameAnyKey);
	} else if (tries++ < 3) {
		codexImpureThoughts();
		screenMessage("\nAbove the din, the voice asks:\n\nIf all eight virtues of the Avatar combine into and are derived from the Three Principles of Truth, Love and Courage...");
		eventHandler->pushKeyHandler(&codexHandleInfinityAnyKey);
	} else codexEject(CODEX_EJECT_BAD_INFINITY);
}

bool codexHandleEndgameAnyKey(int key, void *data) {
	static int index = 1;

	eventHandler->popKeyHandler();

	if (index < 10) {

		if (index < 7) {
			if (index == 6) {
				screenEraseMapArea();
				screenRedrawMapArea();
			}
			screenMessage("%s", codexEndgameText1[index].c_str());
		} else if (index == 7) {
			screenDrawImageInMapArea(BKGD_STONCRCL);
			screenRedrawMapArea();
			screenMessage("\n\n%s", codexEndgameText2[index - 7].c_str());
		} else if (index > 7)
			screenMessage("%s", codexEndgameText2[index - 7].c_str());

		index++;
		eventHandler->pushKeyHandler(&codexHandleEndgameAnyKey);
	} else {
		/* CONGRATULATIONS!... you have completed the game in x turns */
		screenDisableCursor();
		screenMessage("%s%d%s", codexEndgameText2[index - 7].c_str(), g_context->_saveGame->_moves, codexEndgameText2[index - 6].c_str());
#ifdef IOS
		U4IOS::endChoiceConversation();
#endif
		eventHandler->pushKeyHandler(&KeyHandler::ignoreKeys);
	}

	return true;
}

/**
 * Pretty self-explanatory
 */
void codexImpureThoughts() {
	screenMessage("\nThy thoughts are not pure.\nI ask again.\n");
	EventHandler::sleep(2000);
}

} // End of namespace Ultima4
} // End of namespace Ultima
