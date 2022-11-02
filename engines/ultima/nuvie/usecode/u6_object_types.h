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

namespace Ultima {
namespace Nuvie {

class U6UseCode;

const U6ObjectType U6ObjectTypes[] = {
// (object,frame,distance to trigger,event(s),function,properties)
	{ &U6UseCode::use_egg,              OBJ_U6_EGG, 0, 0, USE_EVENT_USE/*|USE_EVENT_LOAD*/, OBJTYPE_CONTAINER },

	{ &U6UseCode::use_door,             OBJ_U6_OAKEN_DOOR,   255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_door,             OBJ_U6_WINDOWED_DOOR, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_door,             OBJ_U6_CEDAR_DOOR,   255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_door,             OBJ_U6_STEEL_DOOR,   255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_key,              OBJ_U6_KEY,          255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_key,              OBJ_U6_LOCK_PICK,    255, 0, USE_EVENT_USE | USE_EVENT_SEARCH | USE_EVENT_GET, OBJTYPE_NONE },

	{ &U6UseCode::magic_ring,           OBJ_U6_PROTECTION_RING, 0, 0, USE_EVENT_READY, OBJTYPE_NONE },
	{ &U6UseCode::magic_ring,           OBJ_U6_REGENERATION_RING, 0, 0, USE_EVENT_READY, OBJTYPE_NONE },
	{ &U6UseCode::magic_ring,           OBJ_U6_INVISIBILITY_RING, 0, 0, USE_EVENT_READY, OBJTYPE_NONE },

	{ &U6UseCode::use_staff,            OBJ_U6_STAFF,        255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::storm_cloak,          OBJ_U6_STORM_CLOAK, 0, 0, USE_EVENT_READY, OBJTYPE_NONE },

	{ &U6UseCode::look_sign,            OBJ_U6_SIGN,      255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_BOOK,      255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_SCROLL,    255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_PICTURE,   255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_SIGN_ARROW, 255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_TOMBSTONE, 255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_CROSS,     255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_CODEX,       0, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },
	{ &U6UseCode::look_sign,            OBJ_U6_BOOK_OF_CIRCLES, 255, 0, USE_EVENT_LOOK, OBJTYPE_BOOK },

	{ &U6UseCode::use_container,        OBJ_U6_CRATE,        0, 0, USE_EVENT_SEARCH, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_CRATE,      255, 0, USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_BARREL,       0, 0, USE_EVENT_SEARCH, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_BARREL,     255, 0, USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_CHEST,        0, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_CHEST,      255, 0, USE_EVENT_USE | USE_EVENT_GET, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_secret_door,      OBJ_U6_SECRET_DOOR, 255, 0, USE_EVENT_USE | USE_EVENT_SEARCH, OBJTYPE_NONE },
	{ &U6UseCode::use_container,        OBJ_U6_BAG,        255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_BACKPACK,   255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_BASKET,     255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DRAWER,     255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DESK,     255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_STONE_LION,   1, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_container,        OBJ_U6_MOUSE,        255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_GRAVE,        0, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_container,        OBJ_U6_DEAD_ANIMAL, 255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DEAD_BODY,  255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DRAKE,      255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DEAD_CYCLOPS, 255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DEAD_GARGOYLE, 255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_REAPER,     255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_REMAINS,    255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_DEER,       255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_container,        OBJ_U6_MONGBAT,    255, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },

	{ &U6UseCode::use_passthrough,      OBJ_U6_V_PASSTHROUGH, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_passthrough,      OBJ_U6_H_PASSTHROUGH, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_switch,           OBJ_U6_LEVER, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_switch,           OBJ_U6_SWITCH, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_churn,            OBJ_U6_CHURN, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_fishing_pole,     OBJ_U6_FISHING_POLE, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },


	{ &U6UseCode::use_crank,            OBJ_U6_CRANK, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_container,        OBJ_U6_BED,       255, 0, USE_EVENT_SEARCH, OBJTYPE_CONTAINER },

	{ &U6UseCode::use_firedevice,       OBJ_U6_FIREPLACE, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_firedevice,       OBJ_U6_CANDLE,    255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_firedevice,       OBJ_U6_CANDELABRA, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_firedevice,       OBJ_U6_BRAZIER,   0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_firedevice,       OBJ_U6_BRAZIER,   1, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::holy_flame,           OBJ_U6_BRAZIER,   2, 0, USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::torch,                OBJ_U6_TORCH,     255, 0, USE_EVENT_USE | USE_EVENT_READY | USE_EVENT_GET | USE_EVENT_DROP, OBJTYPE_NONE },
	{ &U6UseCode::use_spellbook,        OBJ_U6_SPELLBOOK, 255, 0, USE_EVENT_USE | USE_EVENT_LOOK, OBJTYPE_NONE },

	{ &U6UseCode::use_moonstone,        OBJ_U6_MOONSTONE, 255, 0, USE_EVENT_USE | USE_EVENT_GET, OBJTYPE_NONE },
	{ &U6UseCode::use_orb,              OBJ_U6_ORB_OF_THE_MOONS, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::enter_moongate,       OBJ_U6_MOONGATE,  1, 0, USE_EVENT_PASS, OBJTYPE_NONE },
	{ &U6UseCode::enter_moongate,       OBJ_U6_RED_GATE,  1, 0, USE_EVENT_PASS, OBJTYPE_NONE },
	{ &U6UseCode::use_ladder,           OBJ_U6_LADDER, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::enter_dungeon,        OBJ_U6_CAVE,  255, 0, USE_EVENT_PASS, OBJTYPE_NONE },
	{ &U6UseCode::enter_dungeon,        OBJ_U6_HOLE,  255, 0, USE_EVENT_USE | USE_EVENT_PASS, OBJTYPE_NONE },

	{ &U6UseCode::look_clock,           OBJ_U6_CLOCK,     255, 0, USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::look_clock,           OBJ_U6_SUNDIAL,   255, 0, USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::look_mirror,          OBJ_U6_MIRROR,    255, 0, USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::use_well,             OBJ_U6_WELL,      255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_powder_keg,       OBJ_U6_POWDER_KEG, 255, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_peer_gem,         OBJ_U6_GEM,         0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },

	{ &U6UseCode::use_beehive,          OBJ_U6_BEEHIVE,   255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_potion,           OBJ_U6_POTION,    255, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_food,             OBJ_U6_BUTTER,      0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_WINE,        0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_MEAD,        0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_ALE,         0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_BREAD,       0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_MEAT_PORTION, 0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_ROLLS,       0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_CAKE,        0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_CHEESE,      0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_RIBS,        0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_MEAT,        0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_GRAPES,      0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_HAM,         0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_GARLIC,      0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_HORSE_CHOPS, 0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_JAR_OF_HONEY, 0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_DRAGON_EGG,  0, 0, USE_EVENT_USE, OBJTYPE_FOOD },
	{ &U6UseCode::use_food,             OBJ_U6_FISH,        0, 0, USE_EVENT_USE, OBJTYPE_FOOD },

	{ &U6UseCode::use_fan,              OBJ_U6_FAN, 0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_sextant,          OBJ_U6_SEXTANT, 0, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_cow,              OBJ_U6_COW, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_horse,            OBJ_U6_HORSE, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_horse,            OBJ_U6_HORSE_WITH_RIDER, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_boat,             OBJ_U6_SHIP, 255, 0, USE_EVENT_USE | USE_EVENT_SEARCH | USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::use_boat,             OBJ_U6_SKIFF, 255, 0, USE_EVENT_USE | USE_EVENT_GET | USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::use_boat,             OBJ_U6_RAFT,   0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	// don't want OBJTYPE_BOOK so that we will always use the plans when double clicking
	{ &U6UseCode::use_balloon_plans,    OBJ_U6_BALLOON_PLANS, 0, 0, USE_EVENT_USE | USE_EVENT_LOOK, OBJTYPE_NONE },
	{ &U6UseCode::use_balloon,          OBJ_U6_BALLOON, 0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_balloon,          OBJ_U6_INFLATED_BALLOON, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::pass_quest_barrier,   OBJ_U6_QUEST_GATE,  0, 0, USE_EVENT_PASS, OBJTYPE_NONE },

	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_HONESTY,      0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_COMPASSION,   0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_VALOR,        0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_JUSTICE,      0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_SACRIFICE,    0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_HONOR,        0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_SPIRITUALITY, 0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rune,             OBJ_U6_RUNE_HUMILITY,     0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },

	{ &U6UseCode::use_firedevice,       OBJ_U6_CAMPFIRE, 255, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_vortex_cube,      OBJ_U6_VORTEX_CUBE, 0, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_CONTAINER },
	{ &U6UseCode::use_bell,             OBJ_U6_PULL_CHAIN,  0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_bell,             OBJ_U6_BELL,      255, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_shovel,           OBJ_U6_SHOVEL,      0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_shovel,           OBJ_U6_PICK,        0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_fountain,         OBJ_U6_FOUNTAIN,    0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::use_rubber_ducky,     OBJ_U6_RUBBER_DUCKY, 0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_cannon,           OBJ_U6_CANNON,    255, 0, USE_EVENT_USE | USE_EVENT_MOVE | USE_EVENT_MESSAGE, OBJTYPE_NONE },
	{ &U6UseCode::amulet_of_submission, OBJ_U6_AMULET_OF_SUBMISSION, 0, 0, USE_EVENT_READY | USE_EVENT_DROP | USE_EVENT_MOVE, OBJTYPE_NONE },
	{ &U6UseCode::gargish_vocabulary,   OBJ_U6_GARGISH_VOCABULARY,   0, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ &U6UseCode::use_crystal_ball,     OBJ_U6_CRYSTAL_BALL, 0, 0, USE_EVENT_USE | USE_EVENT_MESSAGE, OBJTYPE_NONE },

	{ &U6UseCode::play_instrument,      OBJ_U6_PANPIPES,   0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::use_harpsichord,      OBJ_U6_HARPSICHORD, 0, 0, USE_EVENT_SEARCH | USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::play_instrument,      OBJ_U6_HARP,       0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::play_instrument,      OBJ_U6_LUTE,       0, 0, USE_EVENT_USE, OBJTYPE_NONE },
	{ &U6UseCode::play_instrument,      OBJ_U6_XYLOPHONE,  0, 0, USE_EVENT_USE, OBJTYPE_NONE },

	{ nullptr,                          OBJ_U6_NOTHING, 0, 0, 0, OBJTYPE_NONE } // end indicator
};

} // End of namespace Nuvie
} // End of namespace Ultima
