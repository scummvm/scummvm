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

#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/game/codex.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/weapon.h"
#include "ultima/ultima4/controllers/alpha_action_controller.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

Items *g_items;

const ItemLocation Items::ITEMS[N_ITEMS] = {
	{
		"Mandrake Root", nullptr, "mandrake1",
		&Items::isReagentInInventory, &Items::putReagentInInventory, nullptr, REAG_MANDRAKE, SC_NEWMOONS | SC_REAGENTDELAY
	},
	{
		"Mandrake Root", nullptr, "mandrake2",
		&Items::isReagentInInventory, &Items::putReagentInInventory, nullptr, REAG_MANDRAKE, SC_NEWMOONS | SC_REAGENTDELAY
	},
	{
		"Nightshade", nullptr, "nightshade1",
		&Items::isReagentInInventory, &Items::putReagentInInventory, nullptr, REAG_NIGHTSHADE, SC_NEWMOONS | SC_REAGENTDELAY
	},
	{
		"Nightshade", nullptr, "nightshade2",
		&Items::isReagentInInventory, &Items::putReagentInInventory, nullptr, REAG_NIGHTSHADE, SC_NEWMOONS | SC_REAGENTDELAY
	},
	{
		"the Bell of Courage", "bell", "bell",
		&Items::isItemInInventory, &Items::putItemInInventory, &Items::useBBC, ITEM_BELL, 0
	},
	{
		"the Book of Truth", "book", "book",
		&Items::isItemInInventory, &Items::putItemInInventory, &Items::useBBC, ITEM_BOOK, 0
	},
	{
		"the Candle of Love", "candle", "candle",
		&Items::isItemInInventory, &Items::putItemInInventory, &Items::useBBC, ITEM_CANDLE, 0
	},
	{
		"A Silver Horn", "horn", "horn",
		&Items::isItemInInventory, &Items::putItemInInventory, &Items::useHorn, ITEM_HORN, 0
	},
	{
		"the Wheel from the H.M.S. Cape", "wheel", "wheel",
		&Items::isItemInInventory, &Items::putItemInInventory, &Items::useWheel, ITEM_WHEEL, 0
	},
	{
		"the Skull of Modain the Wizard", "skull", "skull",
		&Items::isSkullInInventory, &Items::putItemInInventory, &Items::useSkull, ITEM_SKULL, SC_NEWMOONS
	},
	{
		"the Red Stone", "red", "redstone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_RED, 0
	},
	{
		"the Orange Stone", "orange", "orangestone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_ORANGE, 0
	},
	{
		"the Yellow Stone", "yellow", "yellowstone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_YELLOW, 0
	},
	{
		"the Green Stone", "green", "greenstone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_GREEN, 0
	},
	{
		"the Blue Stone", "blue", "bluestone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_BLUE, 0
	},
	{
		"the Purple Stone", "purple", "purplestone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_PURPLE, 0
	},
	{
		"the Black Stone", "black", "blackstone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_BLACK, SC_NEWMOONS
	},
	{
		"the White Stone", "white", "whitestone",
		&Items::isStoneInInventory, &Items::putStoneInInventory, &Items::useStone, STONE_WHITE, 0
	},

	/* handlers for using generic objects */
	{ nullptr, "stone",  nullptr, &Items::isStoneInInventory, nullptr, &Items::useStone, -1, 0 },
	{ nullptr, "stones", nullptr, &Items::isStoneInInventory, nullptr, &Items::useStone, -1, 0 },
	{ nullptr, "key",    nullptr, &Items::isItemInInventory, nullptr, &Items::useKey, (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T), 0 },
	{ nullptr, "keys",   nullptr, &Items::isItemInInventory, nullptr, &Items::useKey, (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T), 0 },

	/* Lycaeum telescope */
	{ nullptr, nullptr, "telescope", nullptr, &Items::useTelescope, nullptr, 0, 0 },

	{
		"Mystic Armor", nullptr, "mysticarmor",
		&Items::isMysticInInventory, &Items::putMysticInInventory, nullptr, ARMR_MYSTICROBES, SC_FULLAVATAR
	},
	{
		"Mystic Swords", nullptr, "mysticswords",
		&Items::isMysticInInventory, &Items::putMysticInInventory, nullptr, WEAP_MYSTICSWORD, SC_FULLAVATAR
	},
	{
		"the sulfury remains of an ancient Sosarian Laser Gun. It turns to ash in your fingers", nullptr, "lasergun", // lol, where'd that come from?
		//Looks like someone was experimenting with "maps.xml". It effectively increments sulfur ash by one due to '16' being an invalid weapon index.
		&Items::isWeaponInInventory, &Items::putWeaponInInventory, nullptr, 16, 0
	},
	{
		"the rune of Honesty", nullptr, "honestyrune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_HONESTY, 0
	},
	{
		"the rune of Compassion", nullptr, "compassionrune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_COMPASSION, 0
	},
	{
		"the rune of Valor", nullptr, "valorrune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_VALOR, 0
	},
	{
		"the rune of Justice", nullptr, "justicerune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_JUSTICE, 0
	},
	{
		"the rune of Sacrifice", nullptr, "sacrificerune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_SACRIFICE, 0
	},
	{
		"the rune of Honor", nullptr, "honorrune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_HONOR, 0
	},
	{
		"the rune of Spirituality", nullptr, "spiritualityrune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_SPIRITUALITY, 0
	},
	{
		"the rune of Humility", nullptr, "humilityrune",
		&Items::isRuneInInventory, &Items::putRuneInInventory, nullptr, RUNE_HUMILITY, 0
	}
};

Items::Items() : destroyAllCreaturesCallback(nullptr),
		needStoneNames(0), stoneMask(0) {
	g_items = this;
}

Items::~Items() {
	g_items = nullptr;
}

void Items::setDestroyAllCreaturesCallback(DestroyAllCreaturesCallback callback) {
	destroyAllCreaturesCallback = callback;
}

bool Items::isRuneInInventory(int virt) {
	return g_ultima->_saveGame->_runes & virt;
}

void Items::putRuneInInventory(int virt) {
	g_context->_party->member(0)->awardXp(100);
	g_context->_party->adjustKarma(KA_FOUND_ITEM);
	g_ultima->_saveGame->_runes |= virt;
#ifdef IOS_ULTIMA4
	Common::String virtueName;
	switch (virt) {
	default:
	case RUNE_HONESTY:
		virtueName = "Honesty";
		break;
	case RUNE_HONOR:
		virtueName = "Honor";
		break;
	case RUNE_HUMILITY:
		virtueName = "Humility";
		break;
	case RUNE_JUSTICE:
		virtueName = "Justice";
		break;
	case RUNE_SACRIFICE:
		virtueName = "Sacrifice";
		break;
	case RUNE_SPIRITUALITY:
		virtueName = "Spirituality";
		break;
	case RUNE_VALOR:
		virtueName = "Valor";
		break;
	case RUNE_COMPASSION:
		virtueName = "Compassion";
		break;
	}
	U4IOS::testFlightPassCheckPoint("Player got stone: " + virtueName);
#endif
	g_ultima->_saveGame->_lastReagent = g_ultima->_saveGame->_moves & 0xF0;
}

bool Items::isStoneInInventory(int virt) {
	/* generic test: does the party have any stones yet? */
	if (virt == -1)
		return (g_ultima->_saveGame->_stones > 0);
	/* specific test: does the party have a specific stone? */
	else
		return g_ultima->_saveGame->_stones & virt;
}

void Items::putStoneInInventory(int virt) {
	g_context->_party->member(0)->awardXp(200);
	g_context->_party->adjustKarma(KA_FOUND_ITEM);
	g_ultima->_saveGame->_stones |= virt;
#ifdef IOS_ULTIMA4
	Common::String stoneName;
	switch (virt) {
	default:
	case STONE_BLACK:
		stoneName = "Black";
		break;
	case STONE_BLUE:
		stoneName = "Blue";
		break;
	case STONE_GREEN:
		stoneName = "Green";
		break;
	case STONE_ORANGE:
		stoneName = "Orange";
		break;
	case STONE_PURPLE:
		stoneName = "Purple";
		break;
	case STONE_RED:
		stoneName = "Red";
		break;
	case STONE_WHITE:
		stoneName = "White";
		break;
	case STONE_YELLOW:
		stoneName = "Yellow";
		break;
	}
	U4IOS::testFlightPassCheckPoint("Player got rune: " + stoneName);
#endif
	g_ultima->_saveGame->_lastReagent = g_ultima->_saveGame->_moves & 0xF0;
}

bool Items::isItemInInventory(int item) {
	return g_ultima->_saveGame->_items & item;
}

bool Items::isSkullInInventory(int unused) {
	return (g_ultima->_saveGame->_items & (ITEM_SKULL | ITEM_SKULL_DESTROYED));
}

void Items::putItemInInventory(int item) {
	g_context->_party->member(0)->awardXp(400);
	g_context->_party->adjustKarma(KA_FOUND_ITEM);
	g_ultima->_saveGame->_items |= item;
#ifdef IOS_ULTIMA4
	Common::String itemName;
	switch (item) {
	default:
	case ITEM_BELL:
		itemName = "Bell";
		break;
	case ITEM_BOOK:
		itemName = "Book";
		break;
	case ITEM_CANDLE:
		itemName = "Candle";
		break;
	case ITEM_HORN:
		itemName = "Horn";
		break;
	case ITEM_KEY_C:
		itemName = "Key Courage";
		break;
	case ITEM_KEY_L:
		itemName = "Key Love";
		break;
	case ITEM_KEY_T:
		itemName = "Key Truth";
		break;
	case ITEM_SKULL:
		itemName = "Skull";
		break;
	case ITEM_WHEEL:
		itemName = "Wheel";
		break;

	}
	U4IOS::testFlightPassCheckPoint("Player got rune: " + itemName);
#endif
	g_ultima->_saveGame->_lastReagent = g_ultima->_saveGame->_moves & 0xF0;
}

void Items::useBBC(int item) {
	Coords abyssEntrance(0xe9, 0xe9);
	/* on top of the Abyss entrance */
	if (g_context->_location->_coords == abyssEntrance) {
		/* must use bell first */
		if (item == ITEM_BELL) {
#ifdef IOS_ULTIMA4
			U4IOS::testFlightPassCheckPoint("The Bell rings on and on!");
#endif
			g_screen->screenMessage("\nThe Bell rings on and on!\n");
			g_ultima->_saveGame->_items |= ITEM_BELL_USED;
		}
		/* then the book */
		else if ((item == ITEM_BOOK) && (g_ultima->_saveGame->_items & ITEM_BELL_USED)) {
#ifdef IOS_ULTIMA4
			U4IOS::testFlightPassCheckPoint("The words resonate with the ringing!");
#endif
			g_screen->screenMessage("\nThe words resonate with the ringing!\n");
			g_ultima->_saveGame->_items |= ITEM_BOOK_USED;
		}
		/* then the candle */
		else if ((item == ITEM_CANDLE) && (g_ultima->_saveGame->_items & ITEM_BOOK_USED)) {
			g_screen->screenMessage("\nAs you light the Candle the Earth Trembles!\n");
#ifdef IOS_ULTIMA4
			U4IOS::testFlightPassCheckPoint("As you light the Candle the Earth Trembles!");
#endif
			g_ultima->_saveGame->_items |= ITEM_CANDLE_USED;
		} else {
			g_screen->screenMessage("\nHmm...No effect!\n");
		}
	}
	/* somewhere else */
	else {
		g_screen->screenMessage("\nHmm...No effect!\n");
	}
}

void Items::useHorn(int item) {
	g_screen->screenMessage("\nThe Horn sounds an eerie tone!\n");
	g_context->_aura->set(Aura::HORN, 10);
}

void Items::useWheel(int item) {
	if ((g_context->_transportContext == TRANSPORT_SHIP) && (g_ultima->_saveGame->_shipHull == 50)) {
		g_screen->screenMessage("\nOnce mounted, the Wheel glows with a blue light!\n");
		g_context->_party->setShipHull(99);
	} else {
		g_screen->screenMessage("\nHmm...No effect!\n");
	}
}

void Items::useSkull(int item) {
	/* FIXME: check to see if the abyss must be opened first
	   for the skull to be *able* to be destroyed */

	/* We do the check here instead of in the table, because we need to distinguish between a
	   never-found skull and a destroyed skull. */
	if (g_ultima->_saveGame->_items & ITEM_SKULL_DESTROYED) {
		g_screen->screenMessage("\nNone owned!\n");
		return;
	}

	/* destroy the skull! pat yourself on the back */
	if (g_context->_location->_coords.x == 0xe9 && g_context->_location->_coords.y == 0xe9) {
		g_screen->screenMessage("\n\nYou cast the Skull of Mondain into the Abyss!\n");
#ifdef IOS_ULTIMA4
		U4IOS::testFlightPassCheckPoint("You cast the Skull of Mondain into the Abyss!");
#endif

		g_ultima->_saveGame->_items = (g_ultima->_saveGame->_items & ~ITEM_SKULL) | ITEM_SKULL_DESTROYED;
		g_context->_party->adjustKarma(KA_DESTROYED_SKULL);
	}

	/* use the skull... bad, very bad */
	else {
		g_screen->screenMessage("\n\nYou hold the evil Skull of Mondain the Wizard aloft...\n");
#ifdef IOS_ULTIMA4
		U4IOS::testFlightPassCheckPoint("You hold the evil Skull of Mondain the Wizard aloft...");
#endif

		/* destroy all creatures */
		(*destroyAllCreaturesCallback)();

		/* we don't lose the skull until we toss it into the abyss */
		//c->saveGame->_items = (c->saveGame->_items & ~ITEM_SKULL);
		g_context->_party->adjustKarma(KA_USED_SKULL);
	}
}

void Items::useStone(int item) {
	MapCoords coords;
	byte stone = static_cast<byte>(item);

	static const byte truth   = STONE_WHITE | STONE_PURPLE | STONE_GREEN  | STONE_BLUE;
	static const byte love    = STONE_WHITE | STONE_YELLOW | STONE_GREEN  | STONE_ORANGE;
	static const byte courage = STONE_WHITE | STONE_RED    | STONE_PURPLE | STONE_ORANGE;
	static const byte *attr   = nullptr;

	g_context->_location->getCurrentPosition(&coords);

	/**
	 * Named a specific stone (after using "stone" or "stones")
	 */
	if (item != -1) {
		CombatMap *cm = getCombatMap();

		if (needStoneNames) {
			/* named a stone while in a dungeon altar room */
			if (g_context->_location->_context & CTX_ALTAR_ROOM) {
				needStoneNames--;

				switch (cm->getAltarRoom()) {
				case VIRT_TRUTH:
					attr = &truth;
					break;
				case VIRT_LOVE:
					attr = &love;
					break;
				case VIRT_COURAGE:
					attr = &courage;
					break;
				default:
					break;
				}

				/* make sure we're in an altar room */
				if (attr) {
					/* we need to use the stone, and we haven't used it yet */
					if ((*attr & stone) && (stone & ~stoneMask))
						stoneMask |= stone;
					/* we already used that stone! */
					else if (stone & stoneMask) {
						g_screen->screenMessage("\nAlready used!\n");
						needStoneNames = 0;
						stoneMask = 0; /* reset the mask so you can try again */
						return;
					}
				} else {
					error("Not in an altar room!");
				}
				/* see if we have all the stones, if not, get more names! */
				if (attr && needStoneNames) {
					g_screen->screenMessage("\n%c:", 'E' - needStoneNames);
#ifdef IOS_ULTIMA4
					U4IOS::IOSConversationHelper::setIntroString("Which Color?");
#endif
					itemHandleStones(gameGetInput());
				}
				/* all the stones have been entered, verify them! */
				else {
					unsigned short key = 0xFFFF;
					switch (cm->getAltarRoom()) {
					case VIRT_TRUTH:
						key = ITEM_KEY_T;
						break;
					case VIRT_LOVE:
						key = ITEM_KEY_L;
						break;
					case VIRT_COURAGE:
						key = ITEM_KEY_C;
						break;
					default:
						break;
					}

					/* in an altar room, named all of the stones, and don't have the key yet... */
					if (attr && (stoneMask == *attr) && !(g_ultima->_saveGame->_items & key)) {
#ifdef IOS_ULTIMA4
						Common::String keyName;
						switch (key) {
						case ITEM_KEY_C:
							keyName = "Key Courage";
							break;
						case ITEM_KEY_L:
							keyName = "Key Love";
							break;
						case ITEM_KEY_T:
							keyName = "Key Truth";
							break;
						}
						U4IOS::testFlightPassCheckPoint("Receive a key: " + keyName);
#endif
						g_screen->screenMessage("\nThou doth find one third of the Three Part Key!\n");
						g_ultima->_saveGame->_items |= key;
					} else {
						g_screen->screenMessage("\nHmm...No effect!\n");
					}

					stoneMask = 0; /* reset the mask so you can try again */
				}
			} else {
				/* Otherwise, we're asking for a stone while in the abyss on top of an altar */
				/* see if they entered the correct stone */
				if (stone == (1 << g_context->_location->_coords.z)) {
					if (g_context->_location->_coords.z < 7) {
						/* replace the altar with a down-ladder */
						MapCoords pos;
						g_screen->screenMessage("\n\nThe altar changes before thyne eyes!\n");
						g_context->_location->getCurrentPosition(&pos);
						g_context->_location->_map->_annotations->add(pos, g_context->_location->_map->_tileSet->getByName("down_ladder")->getId());
					} else {
						// Start chamber of the codex sequence...
						g_codex->start();
					}
				} else {
					g_screen->screenMessage("\nHmm...No effect!\n");
				}
			}
		} else {
			g_screen->screenMessage("\nNot a Usable Item!\n");
			stoneMask = 0; /* reset the mask so you can try again */
		}
	}

	/**
	 * in the abyss, on an altar to place the stones
	 */
	else if ((g_context->_location->_map->_id == MAP_ABYSS) &&
			(g_context->_location->_context & CTX_DUNGEON) &&
			(static_cast<Dungeon *>(g_context->_location->_map)->currentToken() == DUNGEON_ALTAR)) {

		int virtueMask = getBaseVirtues((Virtue)g_context->_location->_coords.z);
		if (virtueMask > 0)
			g_screen->screenMessage("\n\nAs thou doth approach, a voice rings out: What virtue dost stem from %s?\n\n", getBaseVirtueName(virtueMask));
		else
			g_screen->screenMessage("\n\nA voice rings out:  What virtue exists independently of Truth, Love, and Courage?\n\n");
#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationHelper::setIntroString("Which virtue?");
#endif
		Common::String virtue = gameGetInput();

		if (scumm_strnicmp(virtue.c_str(), getVirtueName((Virtue)g_context->_location->_coords.z), 6) == 0) {
			/* now ask for stone */
			g_screen->screenMessage("\n\nThe Voice says: Use thy Stone.\n\nColor:\n");
			needStoneNames = 1;
#ifdef IOS_ULTIMA4
			U4IOS::IOSConversationHelper::setIntroString("Which color?");
#endif
			itemHandleStones(gameGetInput());
		} else {
			g_screen->screenMessage("\nHmm...No effect!\n");
		}
	}

	/**
	 * in a dungeon altar room, on the altar
	 */
	else if ((g_context->_location->_context & CTX_ALTAR_ROOM) &&
	         coords.x == 5 && coords.y == 5) {
		needStoneNames = 4;
		g_screen->screenMessage("\n\nThere are holes for 4 stones.\nWhat colors:\nA:");
#ifdef IOS_ULTIMA4
		U4IOS::IOSConversationHelper::setIntroString("Which color?");
#endif
		itemHandleStones(gameGetInput());
	} else {
		g_screen->screenMessage("\nNo place to Use them!\n");
		// This used to say "\nNo place to Use them!\nHmm...No effect!\n"
		// That doesn't match U4DOS; does it match another?
	}
}

void Items::useKey(int item) {
	g_screen->screenMessage("\nNo place to Use them!\n");
}

bool Items::isMysticInInventory(int mystic) {
	/* FIXME: you could feasibly get more mystic weapons and armor if you
	   have 8 party members and equip them all with everything,
	   then search for Mystic Weapons/Armor again

	   or, you could just sell them all and search again.  What an easy
	   way to make some cash!

	   This would be a good candidate for an xu4 "extended" savegame
	   format.
	*/
	if (mystic == WEAP_MYSTICSWORD)
		return g_ultima->_saveGame->_weapons[WEAP_MYSTICSWORD] > 0;
	else if (mystic == ARMR_MYSTICROBES)
		return g_ultima->_saveGame->_armor[ARMR_MYSTICROBES] > 0;
	else
		error("Invalid mystic item was tested in isMysticInInventory()");
	return false;
}

void Items::putMysticInInventory(int mystic) {
	g_context->_party->member(0)->awardXp(400);
	g_context->_party->adjustKarma(KA_FOUND_ITEM);
	if (mystic == WEAP_MYSTICSWORD)
		g_ultima->_saveGame->_weapons[WEAP_MYSTICSWORD] += 8;
	else if (mystic == ARMR_MYSTICROBES)
		g_ultima->_saveGame->_armor[ARMR_MYSTICROBES] += 8;
	else
		error("Invalid mystic item was added in putMysticInInventory()");
	g_ultima->_saveGame->_lastReagent = g_ultima->_saveGame->_moves & 0xF0;
}

bool Items::isWeaponInInventory(int weapon) {
	if (g_ultima->_saveGame->_weapons[weapon])
		return true;
	else {
		for (int i = 0; i < g_context->_party->size(); i++) {
			if (g_context->_party->member(i)->getWeapon()->getType() == weapon)
				return true;
		}
	}
	return false;
}

void Items::putWeaponInInventory(int weapon) {
	g_ultima->_saveGame->_weapons[weapon]++;
}

void Items::useTelescope(int notused) {
	g_screen->screenMessage("You see a knob\non the telescope\nmarked A-P\nYou Select:");
#ifdef IOS_ULTIMA4
	U4IOS::IOSConversationChoiceHelper telescopeHelper;
	telescopeHelper.updateChoices("abcdefghijklmnop ");
#endif
	int choice = AlphaActionController::get('p', "You Select:");

	if (choice == -1)
		return;

	gamePeerCity(choice, nullptr);
}

bool Items::isReagentInInventory(int reag) {
	return false;
}

void Items::putReagentInInventory(int reag) {
	g_context->_party->adjustKarma(KA_FOUND_ITEM);
	g_ultima->_saveGame->_reagents[reag] += xu4_random(8) + 2;
	g_ultima->_saveGame->_lastReagent = g_ultima->_saveGame->_moves & 0xF0;

	if (g_ultima->_saveGame->_reagents[reag] > 99) {
		g_ultima->_saveGame->_reagents[reag] = 99;
		g_screen->screenMessage("Dropped some!\n");
	}
}

bool Items::itemConditionsMet(byte conditions) {
	if ((conditions & SC_NEWMOONS) &&
	        !(g_ultima->_saveGame->_trammelPhase == 0 && g_ultima->_saveGame->_feluccaPhase == 0))
		return false;

	if (conditions & SC_FULLAVATAR) {
		for (int i = 0; i < VIRT_MAX; i++) {
			if (g_ultima->_saveGame->_karma[i] != 0)
				return false;
		}
	}

	if ((conditions & SC_REAGENTDELAY) &&
	        (g_ultima->_saveGame->_moves & 0xF0) == g_ultima->_saveGame->_lastReagent)
		return false;

	return true;
}

const ItemLocation *Items::itemAtLocation(const Map *map, const Coords &coords) {
	for (uint i = 0; i < N_ITEMS; i++) {
		if (!ITEMS[i]._locationLabel)
			continue;
		if (map->getLabel(ITEMS[i]._locationLabel) == coords &&
		        itemConditionsMet(ITEMS[i]._conditions))
			return &(ITEMS[i]);
	}
	return nullptr;
}

void Items::itemUse(const Common::String &shortName) {
	const ItemLocation *item = nullptr;

	for (uint i = 0; i < N_ITEMS; i++) {
		if (ITEMS[i]._shortName &&
		        scumm_stricmp(ITEMS[i]._shortName, shortName.c_str()) == 0) {

			item = &ITEMS[i];

			/* item name found, see if we have that item in our inventory */
			if (!ITEMS[i]._isItemInInventory || (this->*(ITEMS[i]._isItemInInventory))(ITEMS[i]._data)) {

				/* use the item, if we can! */
				if (!item || !item->_useItem)
					g_screen->screenMessage("\nNot a Usable item!\n");
				else
					(this->*(item->_useItem))(ITEMS[i]._data);
			} else
				g_screen->screenMessage("\nNone owned!\n");

			/* we found the item, no need to keep searching */
			break;
		}
	}

	/* item was not found */
	if (!item)
		g_screen->screenMessage("\nNot a Usable item!\n");
}

void Items::itemHandleStones(const Common::String &color) {
	bool found = false;

	for (int i = 0; i < 8; i++) {
		if (scumm_stricmp(color.c_str(), getStoneName((Virtue)i)) == 0 &&
		        isStoneInInventory(1 << i)) {
			found = true;
			itemUse(color.c_str());
		}
	}

	if (!found) {
		g_screen->screenMessage("\nNone owned!\n");
		stoneMask = 0; /* make sure stone mask is reset */
	}
}

bool Items::isAbyssOpened(const Portal *p) {
	/* make sure the bell, book and candle have all been used */
	int items = g_ultima->_saveGame->_items;
	int isopened = (items & ITEM_BELL_USED) && (items & ITEM_BOOK_USED) && (items & ITEM_CANDLE_USED);

	if (!isopened)
		g_screen->screenMessage("Enter Can't!\n");
	return isopened;
}

} // End of namespace Ultima4
} // End of namespace Ultima
