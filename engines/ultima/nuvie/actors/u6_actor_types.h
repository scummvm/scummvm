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

#ifndef ULTIMA_ULTIMA6_ACTORS_U6_ACTOR_TYPES_H
#define ULTIMA_ULTIMA6_ACTORS_U6_ACTOR_TYPES_H

#include "ultima/nuvie/actors/u6_actor.h"

namespace Ultima {
namespace Nuvie {

/*
base_obj_n
frames_per_direction
tiles_per_direction
tiles_per_frame
tile_start_offset
dead_obj_n
dead_frame_n // 255 means same frame as they died
can_laydown
can_sit
tile_type
movetype
twitch_rand
body_armor_class
*/

const U6ActorType u6ActorTypes[] = {
// 4x1 tile actors                                                                                            AC
	{OBJ_U6_INSECTS,          0, 0, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_AIR_LOW,      2,  0},
	{OBJ_U6_GIANT_SQUID,      0, 0, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_WATER_HIGH, 50,  0},
	{OBJ_U6_GHOST,            0, 0, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,       45,  0},
	{OBJ_U6_ACID_SLUG,        0, 0, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_WISP,             0, 0, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_AIR_LOW,    50,  0},
	{OBJ_U6_GIANT_BAT,        0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_AIR_LOW,      30,  0},
	{OBJ_U6_REAPER,           0, 0, 1, 0, OBJ_U6_REAPER, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,      40,  4},
	{OBJ_U6_GREMLIN,          0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         20,  0},
	{OBJ_U6_GAZER,            0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         30,  0},
	{OBJ_U6_BIRD,             0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_AIR_LOW,      20,  0},
	{OBJ_U6_CORPSER,          0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         50,  0},
	{OBJ_U6_RABBIT,           0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         25,  0},
	{OBJ_U6_ROT_WORMS,        0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         50,  0},
	{OBJ_U6_HYDRA,            0, 0, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_MT, MOVETYPE_U6_LAND,          5,  0},
	{OBJ_U6_MOUSE,            1, 1, 1, 0, OBJ_U6_MOUSE, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,        0,  0},
	{OBJ_U6_CAT,              1, 1, 1, 0, OBJ_U6_CAT, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         40,  0},
	{OBJ_U6_TANGLE_VINE,      1, 1, 1, 0, OBJ_U6_TANGLE_VINE, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,  0,  4},

// 4x2
	{OBJ_U6_SEA_SERPENT,      2, 2, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_WATER_HIGH,  60,  2},
	{OBJ_U6_GIANT_RAT,        2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,          40,  0},
	{OBJ_U6_SHEEP,            2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,          35,  0},
	{OBJ_U6_DOG,              2, 2, 1, 0, OBJ_U6_DOG, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,          35,  0},
	{OBJ_U6_DEER,             2, 2, 1, 0, OBJ_U6_DEER, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         20,  0},
	{OBJ_U6_WOLF,             2, 2, 1, 0, OBJ_U6_WOLF, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,         20,  0},
	{OBJ_U6_SNAKE,            2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,          20,  1},
	{OBJ_U6_GIANT_SPIDER,     2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,          20,  0},
	{OBJ_U6_DRAKE,            2, 2, 1, 0, OBJ_U6_DRAKE, 255, false, false, ACTOR_ST, MOVETYPE_U6_AIR_LOW,     15,  4},
	{OBJ_U6_MONGBAT,          2, 2, 1, 0, OBJ_U6_MONGBAT, 255, false, false, ACTOR_ST, MOVETYPE_U6_LAND,      15,  4},
	{OBJ_U6_DAEMON,           2, 2, 1, 0, OBJ_U6_DEAD_BODY, 0, true, false, ACTOR_ST, MOVETYPE_U6_LAND,       30, 10},
	{OBJ_U6_SKELETON,         2, 2, 1, 0, OBJ_U6_DEAD_BODY, 9, true, false, ACTOR_ST, MOVETYPE_U6_LAND,       30,  0},
	{OBJ_U6_HEADLESS,         2, 2, 1, 0, OBJ_U6_DEAD_BODY, 1, true, false, ACTOR_ST, MOVETYPE_U6_LAND,       30,  2},
	{OBJ_U6_TROLL,            2, 2, 1, 0, OBJ_U6_DEAD_BODY, 0, true, false, ACTOR_ST, MOVETYPE_U6_LAND,       40,  4},
	{OBJ_U6_CYCLOPS,          2, 8, 4, 0, OBJ_U6_DEAD_CYCLOPS, 3, true, false, ACTOR_QT, MOVETYPE_U6_LAND,    45,  4},

// 4x3 fix dead frame
	{OBJ_U6_WINGED_GARGOYLE,  3, 12, 4, 0, OBJ_U6_DEAD_GARGOYLE, 3, true, false, ACTOR_QT, MOVETYPE_U6_LAND,  60, 10},
	{OBJ_U6_GARGOYLE,         3, 3, 1, 0, OBJ_U6_DEAD_BODY, 0, true, false, ACTOR_ST, MOVETYPE_U6_LAND,       50,  5},

// 4x5
	{OBJ_U6_FIGHTER,          3, 4, 1, 0, OBJ_U6_DEAD_BODY, 6, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_SWASHBUCKLER,     3, 4, 1, 0, OBJ_U6_DEAD_BODY, 5, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_MAGE,             3, 4, 1, 0, OBJ_U6_DEAD_BODY, 3, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_VILLAGER,         3, 4, 1, 0, OBJ_U6_DEAD_BODY, 2, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_MERCHANT,         3, 4, 1, 0, OBJ_U6_DEAD_BODY, 2, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_CHILD,            3, 4, 1, 0, OBJ_U6_DEAD_BODY, 8, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       30,  0},
	{OBJ_U6_GUARD,            3, 4, 1, 0, OBJ_U6_DEAD_BODY, 4, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       35,  0},
	{OBJ_U6_JESTER,           3, 4, 1, 0, OBJ_U6_DEAD_BODY, 8, true, true, ACTOR_ST, MOVETYPE_U6_LAND,        5,  0},
	{OBJ_U6_PEASANT,          3, 4, 1, 0, OBJ_U6_DEAD_BODY, 5, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0}, //not sure of stats here
	{OBJ_U6_FARMER,           3, 4, 1, 0, OBJ_U6_DEAD_BODY, 8, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       40,  0},
	{OBJ_U6_MUSICIAN,         3, 4, 1, 0, OBJ_U6_DEAD_BODY, 7, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_WOMAN,            3, 4, 1, 0, OBJ_U6_DEAD_BODY, 3, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},
	{OBJ_U6_LORD_BRITISH,     3, 4, 1, 0, OBJ_U6_DEAD_BODY, 2, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       60, 30}, //does LB have a dead frame!? ;)
	{OBJ_U6_AVATAR,           3, 4, 1, 0, OBJ_U6_DEAD_BODY, 7, true, true, ACTOR_ST, MOVETYPE_U6_LAND,       50,  0},

	{OBJ_U6_MUSICIAN_PLAYING, 2, 2, 1, 0, OBJ_U6_NOTHING, 0, false, true, ACTOR_ST, MOVETYPE_U6_LAND,        3,  0},

	{OBJ_U6_SHIP,             1, 2, 2, 8, OBJ_U6_NOTHING, 0, false, false, ACTOR_MT, MOVETYPE_U6_WATER_HIGH,  0, 30},
	{OBJ_U6_SKIFF,            1, 1, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_WATER_LOW,   0,  0},
	{OBJ_U6_INFLATED_BALLOON, 0, 0, 0, 4, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_AIR_LOW,     0,  0},

	{OBJ_U6_GIANT_SCORPION,   2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_DT, MOVETYPE_U6_LAND,         30,  3},
	{OBJ_U6_GIANT_ANT,        2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_DT, MOVETYPE_U6_LAND,         30,  3},
	{OBJ_U6_COW,              2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_DT, MOVETYPE_U6_LAND,         40,  0},
	{OBJ_U6_ALLIGATOR,        2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_DT, MOVETYPE_U6_LAND,         30,  6},
	{OBJ_U6_HORSE,            2, 2, 1, 0, OBJ_U6_HORSE_CARCASS, 1, false, false, ACTOR_DT, MOVETYPE_U6_LAND, 20,  0},
	{OBJ_U6_HORSE_WITH_RIDER, 2, 2, 1, 0, OBJ_U6_BLOOD, 0, false, false, ACTOR_DT, MOVETYPE_U6_LAND,         20,  0},

	{OBJ_U6_DRAGON,           2, 2, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_MT, MOVETYPE_U6_AIR_LOW,    10, 12},
	{OBJ_U6_SILVER_SERPENT,   1, 2, 1, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_MT, MOVETYPE_U6_LAND,        0, 15},

// 2x1 FIXME
	{OBJ_U6_RAFT,             0, 0, 0, 1, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_WATER_LOW,   0,  0}, // FIX might need to fix this
	{OBJ_U6_TANGLE_VINE_POD,  2, 2, 1, 0, OBJ_U6_TANGLE_VINE_POD, 255, false, false, ACTOR_ST, MOVETYPE_U6_NONE, 20, 4}, // Movement type is probably a temp fix

	{OBJ_U6_SLIME,            0, 0, 0, 0, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,        0,  0},

// 1x1
	{OBJ_U6_CHEST,            0, 0, 0, 1, OBJ_U6_CHEST, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND,          5,  0},

	{OBJ_U6_NOTHING, 0, 0, 0, 1, OBJ_U6_NOTHING, 0, false, false, ACTOR_ST, MOVETYPE_U6_LAND, 0, 0} //end indicator
};

// A list of readiable objects and their readied location.

static const struct {
	uint16 obj_n;
	uint8 readiable_location;
	uint8 defence;
	uint8 attack;
}
readiable_objects[] = {
	{OBJ_U6_LEATHER_HELM, ACTOR_HEAD, 1, 0 },
	{OBJ_U6_CHAIN_COIF, ACTOR_HEAD, 2, 0 },
	{OBJ_U6_IRON_HELM, ACTOR_HEAD, 3, 0 },
	{OBJ_U6_SPIKED_HELM, ACTOR_HEAD, 3, 4 },
	{OBJ_U6_WINGED_HELM, ACTOR_HEAD, 2, 0 },
	{OBJ_U6_BRASS_HELM, ACTOR_HEAD, 2, 0 },
	{OBJ_U6_GARGOYLE_HELM, ACTOR_HEAD, 3, 0 },
	{OBJ_U6_MAGIC_HELM, ACTOR_HEAD, 5, 0 },
	{OBJ_U6_WOODEN_SHIELD, ACTOR_ARM, 2, 0 },
	{OBJ_U6_CURVED_HEATER, ACTOR_ARM, 3, 0 },
	{OBJ_U6_WINGED_SHIELD, ACTOR_ARM, 3, 0 },
	{OBJ_U6_KITE_SHIELD, ACTOR_ARM, 3, 0 },
	{OBJ_U6_SPIKED_SHIELD, ACTOR_ARM, 2, 4 },
	{OBJ_U6_BLACK_SHIELD, ACTOR_ARM, 2, 0 },
	{OBJ_U6_DOOR_SHIELD, ACTOR_ARM, 4, 0 },
	{OBJ_U6_MAGIC_SHIELD, ACTOR_ARM, 5, 0 },
	{OBJ_U6_CLOTH_ARMOUR, ACTOR_BODY, 1, 0 },
	{OBJ_U6_LEATHER_ARMOR, ACTOR_BODY, 2, 0 },
	{OBJ_U6_RING_MAIL, ACTOR_BODY, 3, 0 },
	{OBJ_U6_SCALE_MAIL, ACTOR_BODY, 4, 0 },
	{OBJ_U6_CHAIN_MAIL, ACTOR_BODY, 5, 0 },
	{OBJ_U6_PLATE_MAIL, ACTOR_BODY, 7, 0 },
	{OBJ_U6_MAGIC_ARMOUR, ACTOR_BODY, 10, 0 },
	{OBJ_U6_SPIKED_COLLAR, ACTOR_NECK, 2, 0 },
	{OBJ_U6_GUILD_BELT, ACTOR_BODY, 0, 0 },
	{OBJ_U6_GARGOYLE_BELT, ACTOR_BODY, 0, 0 },
	{OBJ_U6_LEATHER_BOOTS, ACTOR_FOOT, 0, 0 },
	{OBJ_U6_SWAMP_BOOTS, ACTOR_FOOT, 0, 0 },

	{OBJ_U6_SLING, ACTOR_ARM, 0, 6 },
	{OBJ_U6_CLUB, ACTOR_ARM, 0, 8 },
	{OBJ_U6_MAIN_GAUCHE, ACTOR_ARM, 1, 8 },
	{OBJ_U6_SPEAR, ACTOR_ARM, 0, 10 },
	{OBJ_U6_THROWING_AXE, ACTOR_ARM, 0, 10 },
	{OBJ_U6_DAGGER, ACTOR_ARM, 0, 6 },
	{OBJ_U6_MACE, ACTOR_ARM, 0, 15 },
	{OBJ_U6_MORNING_STAR, ACTOR_ARM, 0, 15 },
	{OBJ_U6_BOW, ACTOR_ARM_2, 0, 10 },
	{OBJ_U6_CROSSBOW, ACTOR_ARM_2, 0, 12 },
	{OBJ_U6_SWORD, ACTOR_ARM, 0, 15 },
	{OBJ_U6_TWO_HANDED_HAMMER, ACTOR_ARM_2, 0, 20 },
	{OBJ_U6_TWO_HANDED_AXE, ACTOR_ARM_2, 0, 20 },
	{OBJ_U6_TWO_HANDED_SWORD, ACTOR_ARM_2, 0, 20 },
	{OBJ_U6_HALBERD, ACTOR_ARM_2, 0, 30 },
	{OBJ_U6_GLASS_SWORD, ACTOR_ARM, 0, 255 },
	{OBJ_U6_BOOMERANG, ACTOR_ARM, 0, 8 },
	{OBJ_U6_TRIPLE_CROSSBOW, ACTOR_ARM_2, 0, 12 * 3 }, // ?? how to handle this

	{OBJ_U6_MAGIC_BOW, ACTOR_ARM_2, 0, 20 },
	{OBJ_U6_SPELLBOOK, ACTOR_ARM, 0, 0 },

	{OBJ_U6_ANKH_AMULET, ACTOR_NECK, 0, 0 },
	{OBJ_U6_SNAKE_AMULET, ACTOR_NECK, 0, 0 },
	{OBJ_U6_AMULET_OF_SUBMISSION, ACTOR_NECK, 0, 0 },

	{OBJ_U6_STAFF, ACTOR_ARM, 0, 4 },
	{OBJ_U6_LIGHTNING_WAND, ACTOR_ARM, 0, 30 },
	{OBJ_U6_FIRE_WAND, ACTOR_ARM, 0, 20 },
	{OBJ_U6_STORM_CLOAK, ACTOR_BODY, 0, 0 },
	{OBJ_U6_RING, ACTOR_HAND, 0, 0 },
	{OBJ_U6_FLASK_OF_OIL, ACTOR_ARM, 0, 4 },

	{OBJ_U6_TORCH, ACTOR_ARM, 0, 0 },

	{OBJ_U6_SCYTHE, ACTOR_ARM, 0, 0 },
	{OBJ_U6_PITCHFORK, ACTOR_ARM, 0, 0 },
	{OBJ_U6_RAKE, ACTOR_ARM, 0, 0 },
	{OBJ_U6_PICK, ACTOR_ARM, 0, 0 },
	{OBJ_U6_SHOVEL, ACTOR_ARM, 0, 0 },
	{OBJ_U6_HOE, ACTOR_ARM, 0, 0 },

	{OBJ_U6_ROLLING_PIN, ACTOR_ARM, 0, 2 },

	{OBJ_U6_CLEAVER, ACTOR_ARM, 0, 4 },
	{OBJ_U6_KNIFE, ACTOR_ARM, 0, 4 },

	{OBJ_U6_TUNIC, ACTOR_BODY, 0, 0 },
	{OBJ_U6_DRESS, ACTOR_BODY, 0, 0 },
	{OBJ_U6_PANTS, ACTOR_BODY, 0, 0 },

	{OBJ_U6_LUTE, ACTOR_ARM, 0, 0 },

	{OBJ_U6_PLIERS, ACTOR_ARM, 0, 0 },
	{OBJ_U6_HAMMER, ACTOR_ARM, 0, 0 },

	{OBJ_U6_PROTECTION_RING, ACTOR_HAND, 0, 0 },
	{OBJ_U6_REGENERATION_RING, ACTOR_HAND, 0, 0 },
	{OBJ_U6_INVISIBILITY_RING, ACTOR_HAND, 0, 0 },

	{OBJ_U6_ZU_YLEM, ACTOR_ARM, 0, 0},

	{OBJ_U6_NOTHING, ACTOR_NOT_READIABLE, 0, 0 }
}; // this last element terminates the array.


// obj_n, defence, attack, hit_range, attack_type, missle_tile_num, thrown_obj_n, breaks_on_contact

const CombatType u6combat_hand = {OBJ_U6_NOTHING, {0}, {4}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false};
const CombatType u6combat_ship_cannon = {OBJ_U6_SHIP, {0}, {30}, 4, ATTACK_TYPE_MISSLE, TILE_U6_LIGHTNING, OBJ_U6_NOTHING, false };

const CombatType u6combat_objects[] = {
	{OBJ_U6_LEATHER_HELM, {1}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_CHAIN_COIF, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_IRON_HELM, {3}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_SPIKED_HELM, {3}, {4}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_WINGED_HELM, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_BRASS_HELM, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_GARGOYLE_HELM, {3}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_MAGIC_HELM, {5}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_WOODEN_SHIELD, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_CURVED_HEATER, {3}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_WINGED_SHIELD, {3}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_KITE_SHIELD, {3}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_SPIKED_SHIELD, {2}, {4}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_BLACK_SHIELD, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_DOOR_SHIELD, {4}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_MAGIC_SHIELD, {5}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_CLOTH_ARMOUR, {1}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_LEATHER_ARMOR, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_RING_MAIL, {3}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_SCALE_MAIL, {4}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_CHAIN_MAIL, {5}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_PLATE_MAIL, {7}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_MAGIC_ARMOUR, {10}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_SPIKED_COLLAR, {2}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },

	{OBJ_U6_SLING, {0}, {6}, 3, ATTACK_TYPE_MISSLE, TILE_U6_SLING_STONE, OBJ_U6_NOTHING, false },
	{OBJ_U6_CLUB, {0}, {8}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_MAIN_GAUCHE, {1}, {8}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_SPEAR, {0}, {10}, 4, ATTACK_TYPE_THROWN, 0, OBJ_U6_SPEAR, false },
	{OBJ_U6_THROWING_AXE, {0}, {10}, 3, ATTACK_TYPE_THROWN, 0, OBJ_U6_THROWING_AXE, false },
	{OBJ_U6_DAGGER, {0}, {6}, 2, ATTACK_TYPE_THROWN, 0, OBJ_U6_DAGGER, false }, //melee weapon if at range 1
	{OBJ_U6_MACE, {0}, {15}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_MORNING_STAR, {0}, {15}, 2, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_BOW, {0}, {10}, 5, ATTACK_TYPE_MISSLE, TILE_U6_ARROW, OBJ_U6_ARROW, false },
	{OBJ_U6_CROSSBOW, {0}, {12}, 0, ATTACK_TYPE_MISSLE, TILE_U6_BOLT, OBJ_U6_BOLT, false },
	{OBJ_U6_SWORD, {0}, {15}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_TWO_HANDED_HAMMER, {0}, {20}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_TWO_HANDED_AXE, {0}, {20}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_TWO_HANDED_SWORD, {0}, {20}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_HALBERD, {0}, {30}, 2, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_GLASS_SWORD, {0}, {255}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, true },
	{OBJ_U6_BOOMERANG, {0}, {8}, 0, ATTACK_TYPE_THROWN, 0, OBJ_U6_BOOMERANG, false },
	{OBJ_U6_TRIPLE_CROSSBOW, {0}, {12 * 3}, 0, ATTACK_TYPE_MISSLE, TILE_U6_BOLT, OBJ_U6_NOTHING, false }, // ?? how to handle this

	{OBJ_U6_MAGIC_BOW, {0}, {20}, 0, ATTACK_TYPE_MISSLE, TILE_U6_ARROW, OBJ_U6_NOTHING, false },

	{OBJ_U6_STAFF, {0}, {4}, 0, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_LIGHTNING_WAND, {0}, {30}, 0, ATTACK_TYPE_MISSLE, TILE_U6_LIGHTNING, OBJ_U6_NOTHING, false },
	{OBJ_U6_FIRE_WAND, {0}, {20}, 0, ATTACK_TYPE_MISSLE, TILE_U6_FIREBALL, OBJ_U6_NOTHING, false },
	{OBJ_U6_FLASK_OF_OIL, {0}, {4}, 0, ATTACK_TYPE_THROWN, 0, OBJ_U6_FLASK_OF_OIL, false },

	{OBJ_U6_ROLLING_PIN, {0}, {2}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },

	{OBJ_U6_CLEAVER, {0}, {4}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },
	{OBJ_U6_KNIFE, {0}, {4}, 1, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },

	{OBJ_U6_PROTECTION_RING, {5}, {0}, 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false },

	{OBJ_U6_ZU_YLEM, {0}, {1}, 0, ATTACK_TYPE_HAND, 0, OBJ_U6_NOTHING, false },

	{OBJ_U6_NOTHING, {0}, {0} , 0, ATTACK_TYPE_NONE, 0, OBJ_U6_NOTHING, false} // this last element terminates the array.
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
