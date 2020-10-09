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

#include "ultima/ultima4/core/debugger_actions.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/controllers/read_choice_controller.h"
#include "ultima/ultima4/controllers/read_int_controller.h"
#include "ultima/ultima4/controllers/reagents_menu_controller.h"
#include "ultima/ultima4/conversation/conversation.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/gfx/textcolor.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/city.h"

namespace Ultima {
namespace Ultima4 {

void DebuggerActions::summonCreature(const Common::String &name) {
	const Creature *m = nullptr;
	Common::String creatureName = name;

	creatureName.trim();
	if (creatureName.empty()) {
		print("\n");
		return;
	}

	/* find the creature by its id and spawn it */
	uint id = atoi(creatureName.c_str());
	if (id > 0)
		m = creatureMgr->getById(id);

	if (!m)
		m = creatureMgr->getByName(creatureName);

	if (m) {
		if (gameSpawnCreature(m))
			print("\n%s summoned!\n", m->getName().c_str());
		else
			print("\n\nNo place to put %s!\n\n", m->getName().c_str());

		return;
	}

	print("\n%s not found\n", creatureName.c_str());
}

Direction DebuggerActions::directionFromName(const Common::String &dirStr) {
	Common::String dir = dirStr;
	dir.toLowercase();

	if (dir == "up" || dir == "north")
		return DIR_NORTH;
	else if (dir == "down" || dir == "south")
		return DIR_SOUTH;
	else if (dir == "right" || dir == "east")
		return DIR_EAST;
	else if (dir == "left" || dir == "west")
		return DIR_WEST;

	return DIR_NONE;
}

bool DebuggerActions::destroyAt(const Coords &coords) {
	Object *obj = g_context->_location->_map->objectAt(coords);

	if (obj) {
		if (isCreature(obj)) {
			Creature *c = dynamic_cast<Creature *>(obj);
			assert(c);
			g_screen->screenMessage("%s Destroyed!\n", c->getName().c_str());
		} else {
			Tile *t = g_context->_location->_map->_tileSet->get(obj->getTile()._id);
			g_screen->screenMessage("%s Destroyed!\n", t->getName().c_str());
		}

		g_context->_location->_map->removeObject(obj);
		g_screen->screenPrompt();

		return true;
	}

	return false;
}

bool DebuggerActions::getChestTrapHandler(int player) {
	TileEffect trapType;
	int randNum = xu4_random(4);

	/* Do we use u4dos's way of trap-determination, or the original intended way? */
	int passTest = (settings._enhancements && settings._enhancementsOptions._c64ChestTraps) ?
		(xu4_random(2) == 0) : /* xu4-enhanced */
		((randNum & 1) == 0); /* u4dos original way (only allows even numbers through, so only acid and poison show) */

/* Chest is trapped! 50/50 chance */
	if (passTest) {
		/* Figure out which trap the chest has */
		switch (randNum & xu4_random(4)) {
		case 0:
			trapType = EFFECT_FIRE;
			break;   /* acid trap (56% chance - 9/16) */
		case 1:
			trapType = EFFECT_SLEEP;
			break;  /* sleep trap (19% chance - 3/16) */
		case 2:
			trapType = EFFECT_POISON;
			break; /* poison trap (19% chance - 3/16) */
		case 3:
			trapType = EFFECT_LAVA;
			break;   /* bomb trap (6% chance - 1/16) */
		default:
			trapType = EFFECT_FIRE;
			break;
		}

		/* apply the effects from the trap */
		if (trapType == EFFECT_FIRE)
			g_screen->screenMessage("%cAcid%c Trap!\n", FG_RED, FG_WHITE);
		else if (trapType == EFFECT_POISON)
			g_screen->screenMessage("%cPoison%c Trap!\n", FG_GREEN, FG_WHITE);
		else if (trapType == EFFECT_SLEEP)
			g_screen->screenMessage("%cSleep%c Trap!\n", FG_PURPLE, FG_WHITE);
		else if (trapType == EFFECT_LAVA)
			g_screen->screenMessage("%cBomb%c Trap!\n", FG_RED, FG_WHITE);

		// player is < 0 during the 'O'pen spell (immune to traps)
		//
		// if the chest was opened by a PC, see if the trap was
		// evaded by testing the PC's dex
		//
		if ((player >= 0) &&
			(g_ultima->_saveGame->_players[player]._dex + 25 < xu4_random(100))) {
			if (trapType == EFFECT_LAVA) /* bomb trap */
				g_context->_party->applyEffect(trapType);
			else
				g_context->_party->member(player)->applyEffect(trapType);
		} else {
			g_screen->screenMessage("Evaded!\n");
		}

		return true;
	}

	return false;
}

bool DebuggerActions::jimmyAt(const Coords &coords) {
	MapTile *tile = g_context->_location->_map->tileAt(coords, WITH_OBJECTS);

	if (!tile->getTileType()->isLockedDoor())
		return false;

	if (g_ultima->_saveGame->_keys) {
		Tile *door = g_context->_location->_map->_tileSet->getByName("door");
		assertMsg(door, "no door tile found in tileset");
		g_ultima->_saveGame->_keys--;
		g_context->_location->_map->_annotations->add(coords, door->getId());
		g_screen->screenMessage("\nUnlocked!\n");
	} else
		g_screen->screenMessage("%cNo keys left!%c\n", FG_GREY, FG_WHITE);

	return true;
}

bool DebuggerActions::mixReagentsForSpellU4(int spell) {
	Ingredients ingredients;

	g_screen->screenMessage("Reagent: ");

	while (1) {
		int choice = ReadChoiceController::get("abcdefgh\n\r \033");

		// done selecting reagents? mix it up and prompt to mix
		// another spell
		if (choice == '\n' || choice == '\r' || choice == ' ') {
			g_screen->screenMessage("\n\nYou mix the Reagents, and...\n");

			if (g_spells->spellMix(spell, &ingredients))
				g_screen->screenMessage("Success!\n\n");
			else
				g_screen->screenMessage("It Fizzles!\n\n");

			return false;
		}

		// escape: put ingredients back and quit mixing
		if (choice == -1 || choice == '\033') {
			ingredients.revert();
			return true;
		}

		g_screen->screenMessage("\n");
		if (!ingredients.addReagent((Reagent)(choice - 'a')))
			g_screen->screenMessage("%cNone Left!%c\n", FG_GREY, FG_WHITE);
		g_screen->screenMessage("Reagent: ");
	}

	return true;
}

bool DebuggerActions::mixReagentsForSpellU5(int spell) {
	Ingredients ingredients;

	g_screen->screenDisableCursor();

	g_context->_stats->getReagentsMenu()->reset(); // reset the menu, highlighting the first item
	ReagentsMenuController getReagentsController(g_context->_stats->getReagentsMenu(), &ingredients, g_context->_stats->getMainArea());
	eventHandler->pushController(&getReagentsController);
	getReagentsController.waitFor();

	g_context->_stats->getMainArea()->disableCursor();
	g_screen->screenEnableCursor();

	printN("How many? ");

	int howmany = ReadIntController::get(2, TEXT_AREA_X + g_context->_col, TEXT_AREA_Y + g_context->_line);
	gameSpellMixHowMany(spell, howmany, &ingredients);

	return true;
}

bool DebuggerActions::gameSpellMixHowMany(int spell, int num, Ingredients *ingredients) {
	int i;

	// Entered 0 mixtures, don't mix anything!
	if (num == 0) {
		print("\nNone mixed!");
		ingredients->revert();
		return false;
	}

	// If they ask for more than will give them 99, only use what they need
	if (num > 99 - g_ultima->_saveGame->_mixtures[spell]) {
		num = 99 - g_ultima->_saveGame->_mixtures[spell];
		print("\n%cOnly need %d!%c", FG_GREY, num, FG_WHITE);
	}

	print("\nMixing %d...", num);

	// See if there's enough reagents to make number of mixtures requested
	if (!ingredients->checkMultiple(num)) {
		print("\n%cYou don't have enough reagents to mix %d spells!%c", FG_GREY, num, FG_WHITE);
		ingredients->revert();
		return false;
	}

	print("\nYou mix the Reagents, and...");
	if (g_spells->spellMix(spell, ingredients)) {
		print("Success!\n");
		// Mix the extra spells
		ingredients->multiply(num);
		for (i = 0; i < num - 1; i++)
			g_spells->spellMix(spell, ingredients);
	} else {
		print("It Fizzles!\n");
	}

	return true;
}

bool DebuggerActions::openAt(const Coords &coords) {
	const Tile *tile = g_context->_location->_map->tileTypeAt(coords, WITH_OBJECTS);

	if (!tile->isDoor() &&
		!tile->isLockedDoor())
		return false;

	if (tile->isLockedDoor()) {
		g_screen->screenMessage("%cCan't!%c\n", FG_GREY, FG_WHITE);
		return true;
	}

	Tile *floor = g_context->_location->_map->_tileSet->getByName("brick_floor");
	assertMsg(floor, "no floor tile found in tileset");
	g_context->_location->_map->_annotations->add(coords, floor->getId(), false, true)->setTTL(4);

	g_screen->screenMessage("\nOpened!\n");

	return true;
}

void DebuggerActions::gameCastSpell(uint spell, int caster, int param) {
	SpellCastError spellError;
	Common::String msg;

	if (!g_spells->spellCast(spell, caster, param, &spellError, true)) {
		msg = g_spells->spellGetErrorMessage(spell, spellError);
		if (!msg.empty())
			g_screen->screenMessage("%s", msg.c_str());
	}
}

bool DebuggerActions::talkAt(const Coords &coords) {
	extern int personIsVendor(const Person * person);
	City *city;

	/* can't have any conversations outside of town */
	if (!isCity(g_context->_location->_map)) {
		g_screen->screenMessage("Funny, no response!\n");
		return true;
	}

	city = dynamic_cast<City *>(g_context->_location->_map);
	assert(city);
	Person *talker = city ? city->personAt(coords) : nullptr;

	// Make sure we have someone we can talk with
	if (!talker || !talker->canConverse())
		return false;

	/* No response from alerted guards... does any monster both
	   attack and talk besides Nate the Snake? */
	if (talker->getMovementBehavior() == MOVEMENT_ATTACK_AVATAR &&
		talker->getId() != PYTHON_ID)
		return false;

	// If we're talking to Lord British and the avatar is dead, LB resurrects them!
	if (talker->getNpcType() == NPC_LORD_BRITISH &&
		g_context->_party->member(0)->getStatus() == STAT_DEAD) {
		g_screen->screenMessage("%s, Thou shalt live again!\n", g_context->_party->member(0)->getName().c_str());

		g_context->_party->member(0)->setStatus(STAT_GOOD);
		g_context->_party->member(0)->heal(HT_FULLHEAL);
		gameSpellEffect('r', -1, SOUND_LBHEAL);
	}

	Conversation conv;
	conv._script->addProvider("party", g_context->_party);
	conv._script->addProvider("context", g_context);

	conv._state = Conversation::INTRO;
	conv._reply = talker->getConversationText(&conv, "");
	conv._playerInput.clear();
	talkRunConversation(conv, talker, false);

	// Ensure the end of the conversation ends the line
	if (g_context->_col != 0)
		g_screen->screenMessage("\n");

	return true;
}

void DebuggerActions::talkRunConversation(Conversation &conv, Person *talker, bool showPrompt) {
	while (conv._state != Conversation::DONE) {
		// TODO: instead of calculating linesused again, cache the
		// result in person.cpp somewhere.
		int linesused = linecount(conv._reply.front(), TEXT_AREA_W);
		g_screen->screenMessage("%s", conv._reply.front().c_str());
		conv._reply.pop_front();

		/* if all chunks haven't been shown, wait for a key and process next chunk*/
		int size = conv._reply.size();
		if (size > 0) {
#ifdef IOS_ULTIMA4
			U4IOS::IOSConversationChoiceHelper continueDialog;
			continueDialog.updateChoices(" ");
#endif
			ReadChoiceController::get("");
			continue;
		}

		/* otherwise, clear current reply and proceed based on conversation state */
		conv._reply.clear();

		/* they're attacking you! */
		if (conv._state == Conversation::ATTACK) {
			conv._state = Conversation::DONE;
			talker->setMovementBehavior(MOVEMENT_ATTACK_AVATAR);
		}

		if (conv._state == Conversation::DONE)
			break;

		/* When Lord British heals the party */
		else if (conv._state == Conversation::FULLHEAL) {
			int i;

			for (i = 0; i < g_context->_party->size(); i++) {
				g_context->_party->member(i)->heal(HT_CURE);        // cure the party
				g_context->_party->member(i)->heal(HT_FULLHEAL);    // heal the party
			}
			gameSpellEffect('r', -1, SOUND_MAGIC); // same spell effect as 'r'esurrect

			conv._state = Conversation::TALK;
		}
		/* When Lord British checks and advances each party member's level */
		else if (conv._state == Conversation::ADVANCELEVELS) {
			gameLordBritishCheckLevels();
			conv._state = Conversation::TALK;
		}

		if (showPrompt) {
			Common::String prompt = talker->getPrompt(&conv);
			if (!prompt.empty()) {
				if (linesused + linecount(prompt, TEXT_AREA_W) > TEXT_AREA_H) {
#ifdef IOS_ULTIMA4
					U4IOS::IOSConversationChoiceHelper continueDialog;
					continueDialog.updateChoices(" ");
#endif
					ReadChoiceController::get("");
				}

				g_screen->screenMessage("%s", prompt.c_str());
			}
		}

		int maxlen;
		switch (conv.getInputRequired(&maxlen)) {
		case Conversation::INPUT_STRING: {
			conv._playerInput = gameGetInput(maxlen);
#ifdef IOS_ULTIMA4
			g_screen->screenMessage("%s", conv.playerInput.c_str()); // Since we put this in a different window, we need to show it again.
#endif
			conv._reply = talker->getConversationText(&conv, conv._playerInput.c_str());
			conv._playerInput.clear();
			showPrompt = true;
			break;
		}
		case Conversation::INPUT_CHARACTER: {
			char message[2];
#ifdef IOS_ULTIMA4
			U4IOS::IOSConversationChoiceHelper yesNoHelper;
			yesNoHelper.updateChoices("yn ");
#endif
			int choice = ReadChoiceController::get("");


			message[0] = choice;
			message[1] = '\0';

			conv._reply = talker->getConversationText(&conv, message);
			conv._playerInput.clear();

			showPrompt = true;
			break;
		}

		case Conversation::INPUT_NONE:
			conv._state = Conversation::DONE;
			break;
		}
	}
	if (conv._reply.size() > 0)
		g_screen->screenMessage("%s", conv._reply.front().c_str());
}

void DebuggerActions::gameLordBritishCheckLevels() {
	bool advanced = false;

	for (int i = 0; i < g_context->_party->size(); i++) {
		PartyMember *player = g_context->_party->member(i);
		if (player->getRealLevel() <
			player->getMaxLevel())

			// add an extra space to separate messages
			if (!advanced) {
				g_screen->screenMessage("\n");
				advanced = true;
			}

		player->advanceLevel();
	}

	g_screen->screenMessage("\nWhat would thou\nask of me?\n");
}

bool DebuggerActions::isCombat() const {
	return dynamic_cast<CombatController *>(eventHandler->getController()) != nullptr;
}

int DebuggerActions::getCombatFocus() const {
	CombatController *cc = dynamic_cast<CombatController *>(eventHandler->getController());
	assert(cc);
	return cc->getFocus();
}

} // End of namespace Ultima4
} // End of namespace Ultima
