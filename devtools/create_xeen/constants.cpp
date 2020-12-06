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

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include "file.h"
#include "constants.h"

enum MagicSpell {
	MS_AcidSpray = 0, MS_Awaken = 1, MS_BeastMaster = 2, MS_Bless = 3,
	MS_Clairvoyance = 4, MS_ColdRay = 5, MS_CreateFood = 6,
	MS_CureDisease = 7, MS_CureParalysis = 8, MS_CurePoison = 9,
	MS_CureWounds = 10, MS_DancingSword = 11, MS_DayOfProtection = 12,
	MS_DayOfSorcery = 13, MS_DeadlySwarm = 14, MS_DetectMonster = 15,
	MS_DivineIntervention = 16, MS_DragonSleep = 17, MS_ElementalStorm = 18,
	MS_EnchantItem = 19, MS_EnergyBlast = 20, MS_Etheralize = 21,
	MS_FantasticFreeze = 22, MS_FieryFlail = 23, MS_FingerOfDeath = 24,
	MS_Fireball = 25, MS_FirstAid = 26, MS_FlyingFist = 27,
	MS_FrostBite = 28, MS_GolemStopper = 29, MS_Heroism = 30,
	MS_HolyBonus = 31, MS_HolyWord = 32, MS_Hynotize = 33,
	MS_IdentifyMonster = 34, MS_Implosion = 35, MS_Incinerate = 36,
	MS_Inferno = 37, MS_InsectSpray = 38, MS_ItemToGold = 39,
	MS_Jump = 40, MS_Levitate = 41, MS_Light = 42, MS_LightningBolt = 43,
	MS_LloydsBeacon = 44, MS_MagicArrow = 45, MS_MassDistortion = 46,
	MS_MegaVolts = 47, MS_MoonRay = 48, MS_NaturesCure = 49, MS_Pain = 50,
	MS_PoisonVolley = 51, MS_PowerCure = 52, MS_PowerShield = 53,
	MS_PrismaticLight = 54, MS_ProtFromElements = 55, MS_RaiseDead = 56,
	MS_RechargeItem = 57, MS_Resurrection = 58, MS_Revitalize = 59,
	MS_Shrapmetal = 60, MS_Sleep = 61, MS_Sparks = 62, MS_StarBurst = 63,
	MS_StoneToFlesh = 64, MS_SunRay = 65, MS_SuperShelter = 66,
	MS_SuppressDisease = 67, MS_SuppressPoison = 68, MS_Teleport = 69,
	MS_TimeDistortion = 70, MS_TownPortal = 71, MS_ToxicCloud = 72,
	MS_TurnUndead = 73, MS_WalkOnWater = 74, MS_WizardEye = 75,
	NO_SPELL = 76
};

static const char *const CLOUDS_CREDITS =
	"\v012\t000\x3""c\f35Designed and Directed By:\n"
	"\f17Jon Van Caneghem\x3l\n"
	"\n"
	"\t025\f35Programming:\n"
	"\t035\f17Mark Caldwell\n"
	"\t035Dave Hathaway\n"
	"\n"
	"\t025\f35Sound System & FX:\n"
	"\t035\f17Todd Hendrix\n"
	"\n"
	"\t025\f35Music & Speech:\n"
	"\t035\f17Tim Tully\n"
	"\n"
	"\t025\f35Writing:\n"
	"\t035\f17Paul Rattner\n"
	"\t035Debbie Murphy\n"
	"\t035Jon Van Caneghem\v012\n"
	"\n"
	"\n"
	"\t180\f35Graphics:\n"
	"\t190\f17Louis Johnson\n"
	"\t190Jonathan P. Gwyn\n"
	"\t190Bonita Long-Hemsath\n"
	"\t190Julia Ulano\n"
	"\t190Ricardo Barrera\n"
	"\n"
	"\t180\f35Testing:\n"
	"\t190\f17Benjamin Bent\n"
	"\t190Mario Escamilla\n"
	"\t190Richard Espy\n"
	"\t190Scott McDaniel\n"
	"\t190Clayton Retzer\n"
	"\t190Michael Suarez\x3""c";

static const char *const DARK_SIDE_CREDITS =
	"\v012\t000\x3""c\f35Designed and Directed By:\n"
	"\f17Jon Van Caneghem\x3l\n"
	"\n"
	"\t025\f35Programming:\n"
	"\t035\f17Mark Caldwell\n"
	"\t035Dave Hathaway\n"
	"\n"
	"\t025\f35Sound System & FX:\n"
	"\t035\f17Mike Heilemann\n"
	"\n"
	"\t025\f35Music & Speech:\n"
	"\t035\f17Tim Tully\n"
	"\n"
	"\t025\f35Writing:\n"
	"\t035\f17Paul Rattner\n"
	"\t035Debbie Van Caneghem\n"
	"\t035Jon Van Caneghem\v012\n"
	"\n"
	"\n"
	"\t180\f35Graphics:\n"
	"\t190\f17Jonathan P. Gwyn\n"
	"\t190Bonita Long-Hemsath\n"
	"\t190Julia Ulano\n"
	"\t190Ricardo Barrera\n"
	"\n"
	"\t180\f35Testing:\n"
	"\t190\f17Benjamin Bent\n"
	"\t190Christian Dailey\n"
	"\t190Mario Escamilla\n"
	"\t190Marco Hunter\n"
	"\t190Robert J. Lupo\n"
	"\t190Clayton Retzer\n"
	"\t190David Vela\x3""c";
static const char *const SWORDS_CREDITS1 =
	"\v012\x3""c\f35Published By New World Computing, Inc.\f17\n"
	"Developed By CATware, Inc.\x3l\n"
	"\f01Design and Direction\t180Series Created by\n"
	"\t020Bill Fawcett\t190John Van Caneghem\n"
	"\n"
	"\t010Story Contributions\t180Producer & Manual\n"
	"\t020Ellen Guon\t190Dean Rettig\n"
	"\n"
	"\t010Programming & Ideas\t180Original Programming\n"
	"\t020David Potter\t190Mark Caldwell\n"
	"\t020Rod Retterath\t190Dave Hathaway\n"
	"\n"
	"\t010Manual Illustrations\t180Graphic Artists\n"
	"\t020Todd Cameron Hamilton\t190Jonathan P. Gwyn\n"
	"\t020James Clouse\t190Bonnie Long-Hemsath\n"
	"\t190Julia Ulano\n"
	"\t190Ricardo Barrera\n";
static const char *const SWORDS_CREDITS2 =
	"\f05\v012\t000\x3l\n"
	"\t100Sound Programming\n"
	"\t110Todd Hendrix\n"
	"\n"
	"\t100Music\n"
	"\t110Tim Tully\n"
	"\t110Quality Assurance Manager\n"
	"\t110Peter Ryu\n"
	"\t100Testers\n"
	"\t110Walter Johnson\n"
	"\t110Bryan Farina\n"
	"\t110David Baton\n"
	"\t110Jack Nalls\n";

static const char *const OPTIONS_MENU =
	"\r\x1\x3""c\fdMight and Magic Options\n"
	"%s of Xeen\x2\n"
	"\v%.3dCopyright (c) %d NWC, Inc.\n"
	"All Rights Reserved\x1";
static const char *const GAME_NAMES[3] = { "Clouds", "Darkside", "World" };

static const char *const THE_PARTY_NEEDS_REST = "\v012The Party needs rest!";

static const char *const WHO_WILL = "\x3""c\v000\t000%s\n\n"
	"Who will\n%s?\n\v055F1 - F%d";

static const char *const HOW_MUCH = "\x3""cHow Much\n\n";

static const char *const WHATS_THE_PASSWORD = "\x3""cWhat's the Password?\n"
	"\n"
	"Please turn to page %u, go to\n"
	"line %u, and type in word %u.\v067\t000Spaces are not counted as words or lines.  "
	"Hyphenated words are treated as one word.  Any line that has any text is considered a line."
	"\x3""c\v040\t000\n";

static const char *const PASSWORD_INCORRECT = "\x3""c\v040\n"
	"\f32Incorrect!\fd";

static const char *const IN_NO_CONDITION = "\v007%s is not in any condition to perform actions!";

static const char *const NOTHING_HERE = "\x3""c\v010Nothing here.";

static const char *const TERRAIN_TYPES[6] = {
	"town", "cave", "towr", "cstl", "dung", "scfi"
};

static const char *const OUTDOORS_WALL_TYPES[16] = {
	nullptr, "mount", "ltree", "dtree", "grass", "snotree", "dsnotree",
	"snomnt", "dedltree", "mount", "lavamnt", "palm", "dmount", "dedltree",
	"dedltree", "dedltree"
};

static const char *const SURFACE_NAMES[16] = {
	"water.srf", "dirt.srf", "grass.srf", "snow.srf", "swamp.srf",
	"lava.srf", "desert.srf", "road.srf", "dwater.srf", "tflr.srf",
	"sky.srf", "croad.srf", "sewer.srf", "cloud.srf", "scortch.srf",
	"space.srf"
};

static const char *const WHO_ACTIONS[32] = {
	"search", "open", "drink", "mine", "touch", "read", "learn", "take",
	"bang", "steal", "bribe", "pay", "sit", "try", "turn", "bathe",
	"destroy", "pull", "descend", "toss a coin", "pray", "join", "act",
	"play", "push", "rub", "pick", "eat", "sign", "close", "look", "try"
};

static const char *const WHO_WILL_ACTIONS[4] = {
	"Open Grate", "Open Door", "Open Scroll", "Select Char"
};

static const byte SYMBOLS[20][64] = {
	{ // 0
		0x00, 0x00, 0xA8, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x00, 0xA8, 0x9E, 0x9C, 0x9C, 0x9E, 0x9E, 0x9E,
		0xAC, 0x9C, 0xA4, 0xAC, 0xAC, 0x9A, 0x9A, 0x9A, 0xAC, 0x9E, 0xAC, 0xA8, 0xA8, 0xA6, 0x97, 0x98,
		0xAC, 0xA0, 0xAC, 0xAC, 0xA4, 0xA6, 0x98, 0x99, 0x00, 0xAC, 0xA0, 0xA0, 0xA8, 0xAC, 0x9A, 0x9A,
		0x00, 0x00, 0xAC, 0xAC, 0xAC, 0xA4, 0x9B, 0x9A, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xA0, 0x9B, 0x9B,
	},
	{ // 1
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x99, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x99, 0x98, 0x98, 0x98, 0x97, 0x97, 0x97, 0x97, 0x97,
		0x99, 0x98, 0x98, 0x99, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9A, 0x9B, 0x9B, 0x9C, 0x9B, 0x9A, 0x9C, 0x9A, 0x9B, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x9A, 0x9B,
	},
	{ // 2
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x99, 0x98, 0x98, 0x99, 0x98, 0x98, 0x97, 0x98, 0x98,
		0x99, 0x98, 0x98, 0x98, 0x99, 0x99, 0x98, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9B, 0x9B, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9B, 0x99, 0x9A, 0x9B, 0x9B, 0x9A, 0x9A, 0x99, 0x9A,
	},
	{ // 3
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x99, 0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x98, 0x98, 0x97, 0x97, 0x98, 0x98, 0x98, 0x98,
		0x99, 0x99, 0x98, 0x99, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9B, 0x9C, 0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0x9C, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x9A,
	},
	{ // 4
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x99, 0x9A, 0x97, 0x97, 0x97, 0x97, 0x97, 0x98, 0x98, 0x98,
		0x99, 0x99, 0x98, 0x99, 0x99, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9A, 0x9C, 0x9B, 0x9B, 0x9C, 0x9B, 0x9B, 0x9B, 0x9A, 0x99, 0x9B, 0x9B, 0x9A, 0x99, 0x9A, 0x9A,
	},
	{ // 5
		0xA4, 0xA4, 0xA8, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x9E, 0x9E, 0xA0, 0xA8, 0xAC, 0x00, 0x00,
		0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9E, 0xAC, 0x00, 0x97, 0x97, 0x97, 0x98, 0x9C, 0x9C, 0xA0, 0xAC,
		0x99, 0x98, 0x99, 0x99, 0x99, 0x9B, 0xA0, 0xAC, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9B, 0xA0, 0xAC,
		0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x9A, 0x9A, 0x9B, 0x9B, 0xA4, 0xAC, 0x00,
	},
	{ // 6
		0x00, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x00, 0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x9B, 0x99,
		0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x98, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x9C, 0xA0, 0x9C, 0x9A, 0x00, 0x00, 0xAC, 0xA4, 0xA0, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99,
	},
	{ // 7
		0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x9C, 0x99, 0x99,
		0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99,
		0x00, 0x00, 0xAC, 0xA0, 0x9B, 0xA0, 0x9E, 0x9C, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x9C, 0x99, 0x99,
	},
	{ // 8
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x9B, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x9C, 0x99, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x9E, 0x9C, 0x99,
	},
	{ // 9
		0x00, 0x00, 0xAC, 0xA4, 0xA0, 0x9C, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x9C, 0xA0, 0x9C, 0x9A,
		0xAC, 0xA4, 0x9C, 0x9A, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA4, 0x9C, 0x9A, 0x9C, 0x99, 0x99, 0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x9A, 0x99, 0x99,
	},
	{ // 10
		0x99, 0x99, 0x99, 0x9A, 0xA0, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
		0x99, 0x99, 0x9C, 0x9E, 0xA4, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x9C, 0x99, 0x9C, 0xA4, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
		0x99, 0x99, 0x99, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x9A, 0x9B, 0x9E, 0x9C, 0x9C, 0xA4, 0xAC, 0x00,
	},
	{ // 11
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0x9E, 0xAC,
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		0x9C, 0x99, 0x99, 0x99, 0x9C, 0x9C, 0xA4, 0xAC, 0x99, 0x9E, 0x9E, 0x9C, 0x9C, 0xA0, 0xAC, 0x00,
	},
	{ // 12
		0x99, 0x99, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x9B, 0x9C, 0x9E, 0x9C, 0x9C, 0xA4, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x99, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		0x99, 0x99, 0x99, 0x99, 0x9C, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xA4, 0xAC, 0x00,
		0x99, 0x99, 0x9C, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
	},
	{ // 13
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
		0x99, 0x9B, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x9A, 0x99, 0x9C, 0xA0, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		0x99, 0x99, 0x99, 0x99, 0x9A, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x9A, 0x9C, 0xA4, 0xAC, 0x00,
	},
	{ // 14
		0x00, 0x00, 0xAC, 0x9E, 0x9C, 0x9C, 0x9C, 0x9B, 0x00, 0xAC, 0x9C, 0xA0, 0x9E, 0xA4, 0xA4, 0xA4,
		0xAC, 0x9C, 0xA4, 0xAC, 0xAC, 0xAC, 0x9C, 0x9E, 0xAC, 0xA0, 0xAC, 0xA8, 0x9E, 0xA8, 0xAC, 0x99,
		0xAC, 0x9E, 0xAC, 0xA8, 0xAC, 0x9E, 0xA4, 0xAC, 0xAC, 0xA4, 0xA0, 0xAC, 0xAC, 0xA0, 0xA4, 0xAC,
		0x00, 0xAC, 0xA4, 0xA0, 0xA0, 0xA4, 0xAC, 0xA4, 0x00, 0x00, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 15
		0x9C, 0x9C, 0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9E, 0x9E, 0x9E, 0x9C, 0x9E, 0x9E, 0x9E, 0x9E, 0x99, 0x99, 0x99, 0x99, 0x99, 0x98, 0x99, 0x98,
		0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0x9E, 0x9E, 0xA0,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 16
		0x9B, 0x9B, 0x9B, 0x9B, 0x9C, 0x9B, 0x9C, 0x9C, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9E, 0x98, 0x98, 0x98, 0x98, 0x99, 0x99, 0x99, 0x99,
		0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0xA0, 0xA0, 0xA0, 0x9E, 0xA0, 0x9E, 0x9E, 0xA0,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 17
		0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9C, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9E, 0x9E, 0x9E, 0x9C, 0x9C, 0x9C, 0x9E, 0x9E, 0x98, 0x98, 0x98, 0x99, 0x9A, 0x9A, 0x99, 0x98,
		0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9C, 0xA0, 0x9E, 0x9E, 0xA0, 0xA0, 0xA0, 0xA0, 0x9E,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 18
		0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9E, 0x9E, 0x9E, 0x9E, 0x9C, 0x9C, 0x9C, 0x9E, 0x98, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x98, 0x99,
		0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9B, 0x9C, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0xA0, 0xA0, 0xA0,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 19
		0x9C, 0x9B, 0x9C, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0x00, 0x00,
		0x9E, 0x9E, 0x9C, 0x9C, 0x9E, 0xA0, 0xAC, 0x00, 0x99, 0x98, 0x98, 0x99, 0x9A, 0x9A, 0xA0, 0xAC,
		0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0xA0, 0xAC, 0xA0, 0xA0, 0x9E, 0xA0, 0xA0, 0xA0, 0xA0, 0xAC,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0x00, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0x00, 0x00,
	}
};

static const byte TEXT_COLORS[40][4] = {
	{ 0x00, 0x19, 0x19, 0x19 },
	{ 0x00, 0x08, 0x08, 0x08 },
	{ 0x00, 0x0F, 0x0F, 0x0F },
	{ 0x00, 0x15, 0x15, 0x15 },
	{ 0x00, 0x01, 0x01, 0x01 },
	{ 0x00, 0x21, 0x21, 0x21 },
	{ 0x00, 0x26, 0x26, 0x26 },
	{ 0x00, 0x2B, 0x2B, 0x2B },
	{ 0x00, 0x31, 0x31, 0x31 },
	{ 0x00, 0x36, 0x36, 0x36 },
	{ 0x00, 0x3D, 0x3D, 0x3D },
	{ 0x00, 0x41, 0x41, 0x41 },
	{ 0x00, 0x46, 0x46, 0x46 },
	{ 0x00, 0x4C, 0x4C, 0x4C },
	{ 0x00, 0x50, 0x50, 0x50 },
	{ 0x00, 0x55, 0x55, 0x55 },
	{ 0x00, 0x5D, 0x5D, 0x5D },
	{ 0x00, 0x60, 0x60, 0x60 },
	{ 0x00, 0x65, 0x65, 0x65 },
	{ 0x00, 0x6C, 0x6C, 0x6C },
	{ 0x00, 0x70, 0x70, 0x70 },
	{ 0x00, 0x75, 0x75, 0x75 },
	{ 0x00, 0x7B, 0x7B, 0x7B },
	{ 0x00, 0x80, 0x80, 0x80 },
	{ 0x00, 0x85, 0x85, 0x85 },
	{ 0x00, 0x8D, 0x8D, 0x8D },
	{ 0x00, 0x90, 0x90, 0x90 },
	{ 0x00, 0x97, 0x97, 0x97 },
	{ 0x00, 0x9D, 0x9D, 0x9D },
	{ 0x00, 0xA4, 0xA4, 0xA4 },
	{ 0x00, 0xAB, 0xAB, 0xAB },
	{ 0x00, 0xB0, 0xB0, 0xB0 },
	{ 0x00, 0xB6, 0xB6, 0xB6 },
	{ 0x00, 0xBD, 0xBD, 0xBD },
	{ 0x00, 0xC0, 0xC0, 0xC0 },
	{ 0x00, 0xC6, 0xC6, 0xC6 },
	{ 0x00, 0xCD, 0xCD, 0xCD },
	{ 0x00, 0xD0, 0xD0, 0xD0 },
	{ 0x00, 0xD6, 0xD6, 0xD6 },
	{ 0x00, 0xDB, 0xDB, 0xDB },
};

static const byte TEXT_COLORS_STARTUP[40][4] = {
	{ 0x00, 0x19, 0x19, 0x19 },
	{ 0x00, 0x08, 0x08, 0x08 },
	{ 0x00, 0x0F, 0x0F, 0x0F },
	{ 0x00, 0x15, 0x15, 0x15 },
	{ 0x00, 0x01, 0x01, 0x01 },
	{ 0x00, 0x1F, 0x1F, 0x1F },
	{ 0x00, 0x26, 0x26, 0x26 },
	{ 0x00, 0x2B, 0x2B, 0x2B },
	{ 0x00, 0x31, 0x31, 0x31 },
	{ 0x00, 0x36, 0x36, 0x36 },
	{ 0x00, 0x3D, 0x3D, 0x3D },
	{ 0x00, 0x42, 0x42, 0x42 },
	{ 0x00, 0x46, 0x46, 0x46 },
	{ 0x00, 0x4C, 0x4C, 0x4C },
	{ 0x00, 0x50, 0x50, 0x50 },
	{ 0x00, 0x55, 0x55, 0x55 },
	{ 0x00, 0x5D, 0x5D, 0x5D },
	{ 0x00, 0x60, 0x60, 0x60 },
	{ 0x00, 0x65, 0x65, 0x65 },
	{ 0x00, 0x6C, 0x6C, 0x6C },
	{ 0x00, 0x70, 0x70, 0x70 },
	{ 0x00, 0x75, 0x75, 0x75 },
	{ 0x00, 0x7B, 0x7B, 0x7B },
	{ 0x00, 0x80, 0x80, 0x80 },
	{ 0x00, 0x85, 0x85, 0x85 },
	{ 0x00, 0x8D, 0x8D, 0x8D },
	{ 0x00, 0x90, 0x90, 0x90 },
	{ 0x00, 0x97, 0x97, 0x97 },
	{ 0x00, 0x9D, 0x9D, 0x9D },
	{ 0x00, 0xA4, 0xA4, 0xA4 },
	{ 0x00, 0xAB, 0xAB, 0xAB },
	{ 0x00, 0xB0, 0xB0, 0xB0 },
	{ 0x00, 0xB6, 0xB6, 0xB6 },
	{ 0x00, 0xBD, 0xBD, 0xBD },
	{ 0x00, 0xC0, 0xC0, 0xC0 },
	{ 0x00, 0xC6, 0xC6, 0xC6 },
	{ 0x00, 0xCD, 0xCD, 0xCD },
	{ 0x00, 0xD0, 0xD0, 0xD0 },
	{ 0x00, 0x19, 0x19, 0x19 },
	{ 0x00, 0x31, 0x31, 0x31 }
};

static const char *const DIRECTION_TEXT_UPPER[4] = { "NORTH", "EAST", "SOUTH", "WEST" };

static const char *const DIRECTION_TEXT[4] = { "North", "East", "South", "West" };

static const char *const RACE_NAMES[5] = { "Human", "Elf", "Dwarf", "Gnome", "H-Orc" };

static const int RACE_HP_BONUSES[5] = { 0, -2, 1, -1, 2 };

static const int RACE_SP_BONUSES[5][2] = {
	{ 0, 0 }, { 2, 0 }, { -1, -1 }, { 1, 1 }, { -2, -2 }
};

static const char *const ALIGNMENT_NAMES[3] = { "Good", "Neutral", "Evil" };

static const char *const SEX_NAMES[2] = { "Male", "Female" };

static const char *const SKILL_NAMES[18] = {
	"Thievery\t100", "Arms Master", "Astrologer", "Body Builder", "Cartographer",
	"Crusader", "Direction Sense", "Linguist", "Merchant", "Mountaineer",
	"Navigator", "Path Finder", "Prayer Master", "Prestidigitator",
	"Swimmer", "Tracker", "Spot Secret Door", "Danger Sense"
};

static const char *const CLASS_NAMES[11] = {
	"Knight", "Paladin", "Archer", "Cleric", "Sorcerer", "Robber",
	"Ninja", "Barbarian", "Druid", "Ranger", nullptr
};

static const int CLASS_EXP_LEVELS[10] = {
	1500, 2000, 2000, 1500, 2000, 1000, 1500, 1500, 1500, 2000
};

static const char *const CONDITION_NAMES[17] = {
	"Cursed", "Heart Broken", "Weak", "Poisoned", "Diseased",
	"Insane", "In Love", "Drunk", "Asleep", "Depressed", "Confused",
	"Paralyzed", "Unconscious", "Dead", "Stone", "Eradicated", "Good"
};

static const int CONDITION_COLORS[17] = {
	9, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 6, 6, 6, 6, 15
};

static const char *const GOOD = "Good";

static const char *const BLESSED = "\n\t020Blessed\t095%+d";

static const char *const POWER_SHIELD = "\n\t020Power Shield\t095%+d";

static const char *const HOLY_BONUS = "\n\t020Holy Bonus\t095%+d";

static const char *const HEROISM = "\n\t020Heroism\t095%+d";

static const char *const IN_PARTY = "\014""15In Party\014""d";

static const char *const PARTY_DETAILS = "\015\003l\002\014""00"
	"\013""001""\011""035%s"
	"\013""009""\011""035%s"
	"\013""017""\011""035%s"
	"\013""025""\011""035%s"
	"\013""001""\011""136%s"
	"\013""009""\011""136%s"
	"\013""017""\011""136%s"
	"\013""025""\011""136%s"
	"\013""044""\011""035%s"
	"\013""052""\011""035%s"
	"\013""060""\011""035%s"
	"\013""068""\011""035%s"
	"\013""044""\011""136%s"
	"\013""052""\011""136%s"
	"\013""060""\011""136%s"
	"\013""068""\011""136%s";
static const char *const PARTY_DIALOG_TEXT =
	"%s\x2\x3""c\v106\t013Up\t048Down\t083\f37D\fdel\t118\f37R\fdem"
	"\t153\f37C\fdreate\t188E\f37x\fdit\x1";

static const int FACE_CONDITION_FRAMES[17] = {
	2, 2, 2, 1, 1, 4, 4, 4, 3, 2, 4, 3, 3, 5, 6, 7, 0
};

static const int CHAR_FACES_X[6] = { 10, 45, 81, 117, 153, 189 };

static const int HP_BARS_X[6] = { 13, 50, 86, 122, 158, 194 };

static const char *const NO_ONE_TO_ADVENTURE_WITH = "You have no one to adventure with";

static const char *const YOUR_ROSTER_IS_FULL = "Your Roster is full!";

static const byte DARKNESS_XLAT[3][256] = {
	{
	0, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	92, 93, 94, 95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	124, 125, 126, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	140, 141, 142, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	168, 169, 170, 171, 172, 173, 174, 175, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	188, 189, 190, 191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	204, 205, 206, 207, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	220, 221, 222, 223, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	236, 237, 238, 239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	252, 253, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}, {
	0, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 0, 0,
	56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0,
	72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 0, 0,
	88, 89, 90, 91, 92, 93, 94, 95, 0, 0, 0, 0, 0, 0, 0, 0,
	104, 105, 106, 107, 108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0,
	120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	136, 137, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0, 0, 0, 0, 0,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	184, 185, 186, 187, 188, 189, 190, 191, 0, 0, 0, 0, 0, 0, 0, 0,
	200, 201, 202, 203, 204, 205, 206, 207, 0, 0, 0, 0, 0, 0, 0, 0,
	216, 217, 218, 219, 220, 221, 222, 223, 0, 0, 0, 0, 0, 0, 0, 0,
	232, 233, 234, 235, 236, 237, 238, 239, 0, 0, 0, 0, 0, 0, 0, 0,
	248, 249, 250, 251, 252, 253, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0
	}, {
	0, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0,
	68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0,
	84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 0, 0, 0, 0,
	100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 0, 0, 0, 0,
	116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0,
	132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0,
	152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175, 0, 0, 0, 0, 0, 0, 0, 0,
	180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 0, 0, 0, 0,
	196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 0, 0, 0, 0,
	212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 0, 0, 0, 0,
	228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 0, 0, 0, 0,
	244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 0, 0, 0
	}
};

static const char *const PLEASE_WAIT = "\014""d\003""c\011""000"
	"\013""002Please Wait...";

static const char *const OOPS = "\003""c\011""000\013""002Oops...";

static const int SCREEN_POSITIONING_X[4][48] = {
	{
	-1,  0,  0,  0,  1, -1,  0,  0,  0,  1, -2, -1,
	-1,  0,  0,  0,  1,  1,  2, -4, -3, -3, -2, -2,
	-1, -1,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,
	-3, -2, -1,  0,  0,  1,  2,  3, -4,  4,  0,  0
	}, {
	 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  1
	}, {
	 1,  0,  0,  0, -1,  1,  0,  0,  0, -1,  2,  1,
	 1,  0,  0,  0, -1, -1, -2,  4,  3,  3,  2,  2,
	 1,  1,  0,  0,  0, -1, -1, -2, -2, -3, -3, -4,
	 3,  2,  1,  0,  0, -1, -2, -3,  4, -4,  0,  0
	}, {
	 0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3,
	-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  0, -1
	}
};

static const int SCREEN_POSITIONING_Y[4][48] = {
	{
	 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  1
	}, {
	 1,  0,  0,  0, -1,  1,  0,  0,  0, -1,  2,  1,
	 1,  0,  0,  0, -1, -1, -2,  4,  3,  3,  2,  2,
	 1,  1,  0,  0,  0, -1, -1, -2, -2, -3, -3, -4,
	 3,  2,  1,  0,  0, -1, -2, -3,  4, -4,  0,  0
	}, {
	 0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3,
	-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  0, -1
	}, {
	-1,  0,  0,  0,  1, -1,  0,  0,  0,  1, -2, -1,
	-1,  0,  0,  0,  1,  1,  2, -4, -3, -3, -2, -2,
	-1, -1,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,
	-3, -2, -1,  0,  0,  1,  2,  3, -4,  4,  0,  0
	}
};

static const int MONSTER_GRID_BITMASK[12] = {
	0xC, 8, 4, 0, 0xF, 0xF000, 0xF00, 0xF0, 0xF00, 0xF0, 0x0F, 0xF000
};

static const int INDOOR_OBJECT_X[2][12] = {
	{ -5, -7, -112, 98, -8, -65, 49, -9, -34, 16, -58, 40 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -14, -98, 16 }
};

static const int MAP_OBJECT_Y[2][12] = {
	{ 2, 25, 25, 25, 50, 50, 50, 58, 58, 58, 58, 58 },
	{ -65, -6, -6, -6, 36, 36, 36, 54, 54, 54, 54, 54 }
};

static const int INDOOR_MONSTERS_Y[4] = { 2, 34, 53, 59 };

static const int OUTDOOR_OBJECT_X[2][12] = {
	{ -5, -7, -112, 98, -8, -77, 61, -9, -43, 25, -74, 56 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -24, -98, 16 }
};

static const int OUTDOOR_MONSTER_INDEXES[26] = {
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 69, 70,
	71, 72, 73, 74, 75, 90, 91, 92, 93, 94, 112, 115, 118
};

static const int OUTDOOR_MONSTERS_Y[26] = {
	59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53, 53,
	53, 53, 53, 53, 53, 34, 34, 34, 34, 34, 2, 2, 2
};

static const int DIRECTION_ANIM_POSITIONS[4][4] = {
	{ 0, 1, 2, 3 }, { 3, 0, 1, 2 }, { 2, 3, 0, 1 }, { 1, 2, 3, 0 }
};

static const byte WALL_SHIFTS[4][48] = {
	{
		12, 0, 12, 8, 12, 12, 0, 12, 8, 12, 12, 0,
		12, 0, 12, 8, 12, 8, 12, 12, 0, 12, 0, 12,
		0, 12, 0, 12, 8, 12, 8, 12, 8, 12, 8, 12,
		0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 4, 4
	}, {
		8, 12, 8, 4, 8, 8, 12, 8, 4, 8, 8, 12,
		8, 12, 8, 4, 8, 4, 8, 8, 12, 8, 12, 8,
		12, 8, 12, 8, 4, 8, 4, 8, 4, 8, 4, 8,
		12, 12, 12, 12, 4, 4, 4, 4, 0, 0, 0, 0
	}, {
		4, 8, 4, 0, 4, 4, 8, 4, 0, 4, 4, 8,
		4, 8, 4, 0, 4, 0, 4, 4, 8, 4, 8, 4,
		8, 4, 8, 4, 0, 4, 0, 4, 0, 4, 0, 4,
		8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 12, 12
	}, {
		0, 4, 0, 12, 0, 0, 4, 0, 12, 0, 0, 4,
		0, 4, 0, 12, 0, 12, 0, 0, 4, 0, 4, 0,
		4, 0, 4, 0, 12, 0, 12, 0, 12, 0, 12, 0,
		4, 4, 4, 4, 12, 12, 12, 12, 0, 0, 8, 8
	}
};

static const int DRAW_NUMBERS[25] = {
	36, 37, 38, 43, 42, 41,
	39, 20, 22, 24, 33, 31,
	29, 26, 10, 11, 18, 16,
	13, 5, 9, 6, 0, 4, 1
};

static const int DRAW_FRAMES[25][2] = {
	{ 18, 24 }, { 19, 23 }, { 20, 22 }, { 24, 18 }, { 23, 19 }, { 22, 20 },
	{ 21, 21 }, { 11, 17 }, { 12, 16 }, { 13, 15 }, { 17, 11 }, { 16, 12 },
	{ 15, 13 }, { 14, 14 }, { 6, 10 }, { 7, 9 }, { 10, 6 }, { 9, 7 },
	{ 8, 8 }, { 3, 5 }, { 5, 3 }, { 4, 4 }, { 0, 2 }, { 2, 0 },
	{ 1, 1 }
};

static const int COMBAT_FLOAT_X[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };

static const int COMBAT_FLOAT_Y[8] = { -2, 0, 2, 0, -1, 0, 2, 0 };

static const int MONSTER_EFFECT_FLAGS[15][8] = {
	{ 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B },
	{ 0x10C, 0x10D, 0x10E, 0x10F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x110, 0x111, 0x112, 0x113, 0x0, 0x0, 0x0, 0x0 },
	{ 0x114, 0x115, 0x116, 0x117, 0x0, 0x0, 0x0, 0x0 },
	{ 0x200, 0x201, 0x202, 0x203, 0x0, 0x0, 0x0, 0x0 },
	{ 0x300, 0x301, 0x302, 0x303, 0x400, 0x401, 0x402, 0x403 },
	{ 0x500, 0x501, 0x502, 0x503, 0x0, 0x0, 0x0, 0x0 },
	{ 0x600, 0x601, 0x602, 0x603, 0x0, 0x0, 0x0, 0x0 },
	{ 0x604, 0x605, 0x606, 0x607, 0x608, 0x609, 0x60A, 0x60B },
	{ 0x60C, 0x60D, 0x60E, 0x60F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100 },
	{ 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101 },
	{ 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102 },
	{ 0x103, 0x103, 0x103, 0x103, 0x103, 0x103, 0x103, 0x103 },
	{ 0x108, 0x108, 0x108, 0x108, 0x108, 0x108, 0x108, 0x108 }
};

static const int SPELLS_ALLOWED[3][40] = {
	{
		0, 1, 2, 3, 5, 6, 7, 8, 9, 10,
		12, 14, 16, 23, 26, 27, 28, 30, 31, 32,
		33, 42, 46, 48, 49, 50, 52, 55, 56, 58,
		59, 62, 64, 65, 67, 68, 71, 73, 74, 76
	}, {
		1, 4, 11, 13, 15, 17, 18, 19, 20, 21,
		22, 24, 25, 29, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 47, 51, 53, 54,
		57, 60, 61, 63, 66, 69, 70, 72, 75, 76
	}, {
		0, 1, 2, 3, 4, 5, 7, 9, 10, 20,
		25, 26, 27, 28, 30, 31, 34, 38, 40, 41,
		42, 43, 44, 45, 49, 50, 52, 53, 55, 59,
		60, 61, 62, 67, 68, 72, 73, 74, 75, 76
	}
};

static const int BASE_HP_BY_CLASS[10] = { 10, 8, 7, 5, 4, 8, 7, 12, 6, 9 };

static const int AGE_RANGES[10] = { 1, 6, 11, 18, 36, 51, 76, 101, 201, 0xffff };

static const int AGE_RANGES_ADJUST[2][10] = {
	{ -250, -50, -20, -10, 0, -2, -5, -10, -20, -50 },
	{ -250, -50, -20, -10, 0, 2, 5, 10, 20, 50 }
};

static const int STAT_VALUES[24] = {
	3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 30, 35, 40,
	50, 75, 100, 125, 150, 175, 200, 225, 250, 65535
};

static const int STAT_BONUSES[24] = {
	-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20
};

static const int ELEMENTAL_CATEGORIES[6] = { 8, 15, 20, 25, 33, 36 };

static const int ATTRIBUTE_CATEGORIES[10] = {
	9, 17, 25, 33, 39, 45, 50, 56, 61, 72 };

static const int ATTRIBUTE_BONUSES[72] = {
	2, 3, 5, 8, 12, 17, 23, 30, 38, 47,	// Might bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// INT bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// PER bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// SPD bonus
	3, 5, 10, 15, 20, 30,				// ACC bonus
	5, 10, 15, 20, 25, 30,				// LUC bonus
	4, 6, 10, 20, 50,					// HP bonus
	4, 8, 12, 16, 20, 25,				// SP bonus
	2, 4, 6, 10, 16,					// AC bonus
	4, 6, 8, 10, 12, 14, 16, 18, 20, 25	// Thievery bonus
};

static const int ELEMENTAL_RESISTENCES[37] = {
	0, 5, 7, 9, 12, 15, 20, 25, 30, 5, 7, 9, 12, 15, 20, 25,
	5, 10, 15, 20, 25, 10, 15, 20, 25, 40, 5, 7, 9, 11, 13, 15, 20, 25,
	5, 10, 20
};

static const int ELEMENTAL_DAMAGE[37] = {
	0, 2, 3, 4, 5, 10, 15, 20, 30, 2, 3, 4, 5, 10, 15, 20, 2, 4, 5, 10, 20,
	2, 4, 8, 16, 32, 2, 3, 4, 5, 10, 15, 20, 30, 5, 10, 25
};

static const int WEAPON_DAMAGE_BASE[35] = {
	0, 3, 2, 3, 2, 2, 4, 1, 2, 4, 2, 3,
	2, 2, 1, 1, 1, 1, 4, 4, 3, 2, 4, 2,
	2, 2, 5, 3, 3, 3, 3, 5, 4, 2, 6
};

static const int WEAPON_DAMAGE_MULTIPLIER[35] = {
	0, 3, 3, 4, 5, 4, 2, 3, 3, 3, 3, 3,
	2, 4, 10, 6, 8, 9, 4, 3, 6, 8, 5, 6,
	4, 5, 3, 5, 6, 7, 2, 2, 2, 2, 4
};

static const int METAL_DAMAGE[22] = {
	-3, -6, -4, -2, 2, 4, 6, 8, 10, 0, 1,
	1, 2, 2, 3, 4, 5, 12, 15, 20, 30, 50
};

static const int METAL_DAMAGE_PERCENT[22] = {
	253, 252, 3, 2, 1, 2, 3, 4, 6, 0, 1,
	1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

static const int METAL_LAC[22] = {
	-3, 0, -2, -1, 1, 2, 4, 6, 8, 0, 1,
	1, 2, 2, 3, 4, 5, 10, 12, 14, 16, 20
};

static const int ARMOR_STRENGTHS[14] = { 0, 2, 4, 5, 6, 7, 8, 10, 4, 2, 1, 1, 1, 1 };

static const int MAKE_ITEM_ARR1[6] = { 0, 8, 15, 20, 25, 33 };

static const int MAKE_ITEM_ARR2[6][7][2] = {
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 7 }, { 7, 7 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 3 }, { 3, 3 } }
};

static const int MAKE_ITEM_ARR3[10][7][2] = {
	{ { 0, 0 }, { 1, 4 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 10 }, { 10, 10 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },
	{ { 0, 0 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 6 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 4 }, { 3, 6 }, { 5, 8 }, { 7, 10 }, { 10, 10 } }
};

static const int MAKE_ITEM_ARR4[2][7][2] = {
	{ { 0, 0 }, { 1, 4 }, { 3, 7 }, { 4, 8 }, { 5, 9 }, { 8, 9 }, { 9, 9 } },
	{ { 0, 0 }, { 1, 4 }, { 2, 6 }, { 4, 7 }, { 6, 10 }, { 9, 13 }, { 13, 13 } }
};

static const int MAKE_ITEM_ARR5[8][2] = {
	{ 0, 0 }, { 1, 15 }, { 16, 30 }, { 31, 40 }, { 41, 50 },
	{ 51, 60 }, { 61, 73 }, { 61, 73 }
};

static const int OUTDOOR_DRAWSTRUCT_INDEXES[44] = {
	37, 38, 39, 40, 41, 44, 42, 43, 47, 45, 46,
	48, 49, 52, 50, 51, 66, 67, 68, 69, 70, 71,
	72, 75, 73, 74, 87, 88, 89, 90, 91, 94, 92,
	93, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120
};

static const int TOWN_MAXES[2][11] = {
	{ 23, 13, 32, 16, 26, 16, 16, 16, 16, 16, 16 },
	{ 26, 19, 48, 27, 26, 37, 16, 16, 16, 16, 16 }
};

static const char *const TOWN_ACTION_MUSIC[2][7] = {
	{ "bank.m", "smith.m", "guild.m", "tavern.m",
	"temple.m", "grounds.m", "endgame.m" },
	{ "bank.m", "sf09.m", "guild.m", "tavern.m",
	"temple.m", "smith.m", "endgame.m" }
};

static const char *const TOWN_ACTION_SHAPES[7] = {
	"bnkr", "blck", "gild", "tvrn", "tmpl", "trng", "eface08"
};

static const int TOWN_ACTION_FILES[2][7] = {
	{ 3, 2, 4, 2, 4, 2, 1 }, { 5, 3, 7, 5, 4, 6, 1 }
};

static const char *const BANK_TEXT = "\r\x2\x3""c\v122\t013"
	"\f37D\fdep\t040\f37W\fdith\t067ESC"
	"\x1\t000\v000Bank of Xeen\v015\n"
	"Bank\x3l\n"
	"Gold\x3r\t000%s\x3l\n"
	"Gems\x3r\t000%s\x3""c\n"
	"\n"
	"Party\x3l\n"
	"Gold\x3r\t000%s\x3l\n"
	"Gems\x3r\t000%s";

static const char *const BLACKSMITH_TEXT = "\x1\r\x3""c\v000\t000"
	"Store Options for\t039\v027%s\x3l\v046\n"
	"\t011\f37B\fdrowse\n"
	"\t000\v090Gold\x3r\t000%s"
	"\x2\x3""c\v122\t040ESC\x1";

static const char *const GUILD_NOT_MEMBER_TEXT =
	"\n\nYou have to be a member to shop here.";

static const char *const GUILD_TEXT = "\x3""c\v027\t039%s"
	"\x3l\v046\n"
	"\t012\f37B\fduy Spells\n"
	"\t012\f37S\fdpell Info";

static const char *const TAVERN_TEXT =
	"\r\x3""c\v000\t000Tavern Options for\t039"
	"\v027%s%s\x3l\t000"
	"\v090Gold\x3r\t000%s\x2\x3""c\v122"
	"\t021\f37S\fdign in\t060ESC\x1";

static const char *const FOOD_AND_DRINK =
	"\x3l\t017\v046\f37D\fdrink\n"
	"\t017\f37F\fdood\n"
	"\t017\f37T\fdip\n"
	"\t017\f37R\fdumors";

static const char *const GOOD_STUFF = "\n"
	"\n"
	"Good Stuff\n"
	"\n"
	"Hit a key!";

static const char *const HAVE_A_DRINK = "\n\nHave a Drink\n\nHit a key!";

static const char *const YOURE_DRUNK = "\n\nYou're Drunk\n\nHit a key!";

static const int TAVERN_EXIT_LIST[2][6][5][2] = {
	{
		{ { 21, 17 }, { 0, 0 }, { 20, 3 }, { 0, 0 }, { 0, 0 } },
		{ { 13, 4 }, { 0, 0 }, { 19, 9 }, { 0, 0 }, { 0, 0 } },
		{ { 20, 10 }, { 12, 8 }, { 5, 26 }, { 3, 4 }, { 7, 5 } },
		{ { 18, 4 }, { 0, 0 }, { 19, 16 }, { 0, 0 }, { 11, 12 } },
		{ { 15, 21 }, { 0, 0 }, { 13, 21 }, { 0, 0 }, { 0, 0 } },
		{ { 10, 8 }, { 0, 0 }, { 15, 12 }, { 0, 0 }, { 0, 0 } },
	}, {
		{ { 21, 17 }, { 0, 0 }, { 20, 3 }, { 0, 0 }, { 0, 0 } },
		{ { 13, 4 }, { 0, 0 }, { 19, 9 }, { 0, 0 }, { 0, 0 } },
		{ { 20, 10 }, { 12, 8 }, { 5, 26 }, { 3, 4 }, { 7, 5 } },
		{ { 17, 24 }, { 14, 13 }, { 0, 0 }, { 0, 0 }, { 9, 4 } },
		{ { 15, 21 }, { 0, 0 }, { 13, 21 }, { 0, 0 }, { 0, 0 } },
		{ { 10, 8 }, { 0, 0 }, { 15, 12 }, { 0, 0 }, { 0, 0 } }
	}
};

static const char *const TEMPLE_TEXT =
	"\r\x3""c\v000\t000Temple Options for"
	"\t039\v027%s\x3l\t000\v046"
	"\f37H\fdeal\x3r\t000%u\x3l\n"
	"\f37D\fdonation\x3r\t000%u\x3l\n"
	"\f37U\fdnCurse\x3r\t000%s"
	"\x3l\t000\v090Gold\x3r\t000%s"
	"\x2\x3""c\v122\t040ESC\x1";

static const char *const EXPERIENCE_FOR_LEVEL =
	"%s needs %u experience for level %u.";

static const char *const TRAINING_LEARNED_ALL = "%s has learned all we can teach!";

static const char *const ELIGIBLE_FOR_LEVEL = "%s is eligible for level %u.\x3l\n"
	"\v081Cost\x3r\t000%u";

static const char *const TRAINING_TEXT =
	"\r\x3""cTraining Options\n"
	"\n"
	"%s\x3l\v090\t000Gold\x3r\t000%s\x2\x3""c\v122\t021"
	"\f37T\fdrain\t060ESC\x1";

static const char *const GOLD_GEMS =
	"\x3""c\v000\t000%s\x3l\n"
	"\n"
	"Gold\x3r\t000%s\x3l\n"
	"Gems\x3r\t000%s\x2\x3""c\v096\t013G\f37o\fdld\t040G\f37e\fdms\t067ESC\x1";

static const char *const GOLD_GEMS_2 =
	"\x3""c\v000\t000%s\x3l\n"
	"\n"
	"\x4""077Gold\x3r\t000%s\x3l\n"
	"\x4""077Gems\x3r\t000%s\x3l\t000\v051\x4""077\n"
	"\x4""077";

static const char *const DEPOSIT_WITHDRAWL[2] = { "Deposit", "Withdrawl" };

static const char *const NOT_ENOUGH_X_IN_THE_Y =
	"\x3""c\v012Not enough %s in the %s!\x3l";

static const char *const NO_X_IN_THE_Y = "\x3""c\v012No %s in the %s!\x3l";

static const char *const STAT_NAMES[16] = {
	"Might", "Intellect", "Personality", "Endurance", "Speed",
	"Accuracy", "Luck", "Age", "Level", "Armor Class", "Hit Points",
	"Spell Points", "Resistances", "Skills", "Awards", "Experience"
};

static const char *const CONSUMABLE_NAMES[4] = { "Gold", "Gems", "Food", "Condition" };

static const char *const WHERE_NAMES[2] = { "Party", "Bank" };

static const char *const AMOUNT = "\x3""c\t000\v051Amount\x3l\n";

static const char *const FOOD_PACKS_FULL = "\v007Your food packs are already full!";

static const char *const BUY_SPELLS =
	"\x3""c\v027\t039%s\x3l\v046\n"
	"\t012\f37B\fduy Spells\n"
	"\t012\f37S\fdpell Info";

static const char *const GUILD_OPTIONS =
	"\r\f00\x3""c\v000\t000Guild Options for%s"
	"\x3l\t000\v090Gold"
	"\x3r\t000%s\x2\x3""c\v122\t040ESC\x1";

static const int MISC_SPELL_INDEX[74] = {
	NO_SPELL, MS_Light, MS_Awaken, MS_MagicArrow,
	MS_FirstAid, MS_FlyingFist, MS_EnergyBlast, MS_Sleep,
	MS_Revitalize, MS_CureWounds, MS_Sparks, MS_Shrapmetal,
	MS_InsectSpray, MS_ToxicCloud, MS_ProtFromElements, MS_Pain,
	MS_Jump, MS_BeastMaster, MS_Clairvoyance, MS_TurnUndead,
	MS_Levitate, MS_WizardEye, MS_Bless, MS_IdentifyMonster,
	MS_LightningBolt, MS_HolyBonus, MS_PowerCure, MS_NaturesCure,
	MS_LloydsBeacon, MS_PowerShield, MS_Heroism, MS_Hynotize,
	MS_WalkOnWater, MS_FrostBite, MS_DetectMonster, MS_Fireball,
	MS_ColdRay, MS_CurePoison, MS_AcidSpray, MS_TimeDistortion,
	MS_DragonSleep, MS_CureDisease, MS_Teleport, MS_FingerOfDeath,
	MS_CureParalysis, MS_GolemStopper, MS_PoisonVolley, MS_DeadlySwarm,
	MS_SuperShelter, MS_DayOfProtection, MS_DayOfSorcery, MS_CreateFood,
	MS_FieryFlail, MS_RechargeItem, MS_FantasticFreeze, MS_TownPortal,
	MS_StoneToFlesh, MS_RaiseDead, MS_Etheralize, MS_DancingSword,
	MS_MoonRay, MS_MassDistortion, MS_PrismaticLight, MS_EnchantItem,
	MS_Incinerate, MS_HolyWord, MS_Resurrection, MS_ElementalStorm,
	MS_MegaVolts, MS_Inferno, MS_SunRay, MS_Implosion,
	MS_StarBurst, MS_DivineIntervention
};

static const int SPELL_COSTS[77] = {
	8, 1, 5, -2, 5, -2, 20, 10, 12, 8, 3,
	- 3, 75, 40, 12, 6, 200, 10, 100, 30, -1, 30,
	15, 25, 10, -2, 1, 2, 7, 20, -2, -2, 100,
	15, 5, 100, 35, 75, 5, 20, 4, 5, 1, -2,
	6, 2, 75, 40, 60, 6, 4, 25, -2, -2, 60,
	- 1, 50, 15, 125, 2, -1, 3, -1, 200, 35, 150,
	15, 5, 4, 10, 8, 30, 4, 5, 7, 5, 0
};

static const int DARK_SPELL_RANGES[12][2] = {
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 },
	{ 0, 17 }, { 14, 34 }, { 26, 37 }, { 29, 39 },
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 }
};

static const int SWORDS_SPELL_RANGES[12][2] = {
	{ 0, 20 },{ 16, 35 },{ 27, 39 },{ 29, 39 },
	{ 0, 17 },{ 14, 34 },{ 26, 39 },{ 29, 39 },
	{ 0, 20 },{ 16, 35 },{ 27, 39 },{ 29, 39 }
};

static const int CLOUDS_GUILD_SPELLS[5][20] = {
	{
		1, 10, 20, 26, 27, 38, 40, 42, 45, 50,
		55, 59, 60, 61, 62, 68, 72, 75, 77, 77
	}, {
		3, 4, 5, 14, 15, 25, 30, 31, 34, 41,
		49, 51, 53, 67, 73, 75, -1, -1, -1, -1
	}, {
		4, 8, 9, 12, 13, 22, 23, 24, 28, 34,
		41, 44, 52, 70, 73, 74, -1, -1, -1, -1
	}, {
		6, 7, 9, 11, 12, 13, 17, 21, 22, 24,
		29, 36, 56, 58, 64, 71, -1, -1, -1, -1
	}, {
		6, 7, 9, 11, 12, 13, 18, 21, 29, 32,
		36, 37, 46, 51, 56, 58, 69, -1, -1, -1
	}
};

static const int DARK_SPELL_OFFSETS[3][39] = {
	{
		42, 1, 26, 59, 27, 10, 50, 68, 55, 62, 67, 73, 2,
		5, 3, 31, 30, 52, 49, 28, 74, 0, 9, 7, 14, 8,
		33, 6, 23, 71, 64, 56, 48, 46, 12, 32, 58, 65, 16
	}, {
		42, 1, 45, 61, 72, 40, 20, 60, 38, 41, 75, 34, 4,
		43, 25, 53, 44, 15, 70, 17, 24, 69, 22, 66, 57, 11,
		29, 39, 51, 21, 19, 36, 47, 13, 54, 37, 18, 35, 63
	}, {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38
	}
};

static const int SPELL_GEM_COST[77] = {
	0, 0, 2, 1, 2, 4, 5, 0, 0, 0, 0, 10, 10, 10, 0, 0, 20, 4, 10, 20, 1, 10,
	5, 5, 4, 2, 0, 0, 0, 10, 3, 1, 20, 4, 0, 20, 10, 10, 1, 10, 0, 0, 0, 2,
	2, 0, 10, 10, 10, 0, 0, 10, 3, 2, 10, 1, 10, 10, 20, 0, 0, 1, 1, 20, 5, 20,
	5, 0, 0, 0, 0, 5, 1, 2, 0, 2, 0
};

static const char *const NOT_A_SPELL_CASTER = "Not a spell caster...";

static const char *const SPELLS_LEARNED_ALL = "You have learned all we\n"
	"\t010can teach you.";

static const char *const SPELLS_FOR = "\r\fd%s\x2\x3""c\t000\v002Spells for %s";

static const char *const SPELL_LINES_0_TO_9 =
	"\x2\x3l\v015\t0011\n2\n3\n4\n5\n6\n7\n8\n9\n0";

static const char *const SPELLS_DIALOG_SPELLS = "\x3l\v015"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l\n"
	"\t010\f%2u%s\fd\x3l"
	"\t004\v110%s - %u\x1";

static const char *const SPELL_PTS = "Spell Pts";

static const char *const GOLD = "Gold";

static const char *const SPELL_INFO =
	"\x3""c\f09%s\fd\x3l\n"
	"\n"
	"%s\x3""c\t000\v100Press a Key!";

static const char *const SPELL_PURCHASE =
	"\x3l\v000\t000\fd%s  Do you wish to purchase "
	"\f09%s\fd for %u?";

static const char *const MAP_TEXT =
	"\x3""c\v000\t000%s\x3l\v139"
	"\t000X = %d\x3r\t000Y = %d\x3""c\t000%s";

static const char *const LIGHT_COUNT_TEXT = "\x3l\n\n\t024Light\x3r\t124%d";

static const char *const FIRE_RESISTENCE_TEXT = "%c%sFire%s%u";

static const char *const ELECRICITY_RESISTENCE_TEXT = "%c%sElectricity%s%u";

static const char *const COLD_RESISTENCE_TEXT = "%c%sCold%s%u";

static const char *const POISON_RESISTENCE_TEXT = "%c%sPoison/Acid%s%u";

static const char *const CLAIRVOYANCE_TEXT = "%c%sClairvoyance%s";

static const char *const LEVITATE_TEXT = "%c%sLevitate%s";

static const char *const WALK_ON_WATER_TEXT = "%c%sWalk on Water";

static const char *const GAME_INFORMATION =
	"\r\x3""c\t000\v001\f37%s of Xeen\fd\n"
	"Game Information\n"
	"\n"
	"Today is \f37%ssday\fd\n"
	"\n"
	"\t032Time\t072Day\t112Year\n"
	"\t032\f37%d:%02d%c\t072%u\t112%u\fd%s";

static const char *const WORLD_GAME_TEXT = "World";
static const char *const DARKSIDE_GAME_TEXT = "Darkside";
static const char *const CLOUDS_GAME_TEXT = "Clouds";
static const char *const SWORDS_GAME_TEXT = "Swords";

static const char *const WEEK_DAY_STRINGS[10] = {
	"Ten", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"
};

static const char *const CHARACTER_DETAILS =
	"\x3l\v041\t196%s\t000\v002%s : %s %s %s"
	"\x3r\t053\v028\f%02u%u\fd\t103\f%02u%u\fd"
	"\x3l\t131\f%02u%d\fd\t196\f15%u\fd\x3r"
	"\t053\v051\f%02u%u\fd\t103\f%02u%u\fd"
	"\x3l\t131\f%02u%u\fd\t196\f15%u\fd"
	"\x3r\t053\v074\f%02u%u\fd\t103\f%02u%u\fd"
	"\x3l\t131\f15%u\fd\t196\f15%u\fd"
	"\x3r\t053\v097\f%02u%u\fd\t103\f%02u%u\fd"
	"\x3l\t131\f15%u\fd\t196\f15%u day%c\fd"
	"\x3r\t053\v120\f%02u%u\fd\t103\f%02u%u\fd"
	"\x3l\t131\f15%u\fd\t196\f%02u%s\fd"
	"\t230%s%s%s%s\fd";

static const char *const PARTY_GOLD = "Party Gold";

static const char *const PLUS_14 = "\f14+";

static const char *const CHARACTER_TEMPLATE =
	"\x1\f00\r\x3l\t029\v018Mgt\t080Acy\t131H.P.\t196Experience"
	"\t029\v041Int\t080Lck\t131S.P.\t029\v064Per\t080Age"
	"\t131Resis\t196Party Gems\t029\v087End\t080Lvl\t131Skills"
	"\t196Party Food\t029\v110Spd\t080AC\t131Awrds\t196Condition\x3""c"
	"\t290\v025\f37I\fdtem\t290\v057\f37Q"
	"\fduick\t290\v089\f37E\fdxch\t290\v121Exit\x3l%s";

static const char *const EXCHANGING_IN_COMBAT = "\x3""c\v007\t000Exchanging in combat is not allowed!";

static const char *const CURRENT_MAXIMUM_RATING_TEXT = "\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\t054%u\x3l\t058/ %u\n"
	"\x3""cRating: %s";

static const char *const CURRENT_MAXIMUM_TEXT = "\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\t054%u\x3l\t058/ %u";

static const char *const RATING_TEXT[24] = {
	"Nonexistant", "Very Poor", "Poor", "Very Low", "Low", "Average", "Good",
	"Very Good", "High", "Very High", "Great", "Super", "Amazing", "Incredible",
	"Gigantic", "Fantastic", "Astoundig", "Astonishing", "Monumental", "Tremendous",
	"Collosal", "Awesome", "Awe Inspiring", "Ultimate"
};

static const char *const AGE_TEXT = "\x2\x3""c%s\n"
	"Current / Natural\n"
	"\x3r\t057%u\x3l\t061/ %u\n"
	"\x3""cBorn: %u / %u\x1";

static const char *const LEVEL_TEXT =
	"\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\t054%u\x3l\t058/ %u\n"
	"\x3""c%u Attack%s/Round\x1";

static const char *const RESISTENCES_TEXT =
	"\x2\x3""c%s\x3l\n"
	"\t020Fire\t100%u\n"
	"\t020Cold\t100%u\n"
	"\t020Electricity\t100%u\n"
	"\t020Poison\t100%u\n"
	"\t020Energy\t100%u\n"
	"\t020Magic\t100%u";

static const char *const NONE = "\n\t020None";

static const char *const EXPERIENCE_TEXT = "\x2\x3""c%s\x3l\n"
	"\t010Current:\t070%u\n"
	"\t010Next Level:\t070%s\x1";

static const char *const ELIGIBLE = "\f12Eligible\fd";

static const char *const IN_PARTY_IN_BANK =
	"\x2\x3""cParty %s\n"
	"%u on hand\n"
	"%u in bank\x1\x3l";

static const char *const FOOD_TEXT =
	"\x2\x3""cParty %s\n"
	"%u on hand\n"
   "Enough for %u day%s\x3l";

static const char *const EXCHANGE_WITH_WHOM = "\t010\v005Exchange with whom?";

static const char *const QUICK_REF_LINE =
	"\v%3d\t007%u)\t027%s\t110%c%c%c\x3r\t160\f%02u%u\fd"
	"\x3l\t170\f%02u%d\fd\t208\f%02u%u\fd\t247\f"
	"%02u%u\fd\t270\f%02u%c%c%c%c\fd";

static const char *const QUICK_REFERENCE =
	"\r\x3""cQuick Reference Chart\v012\x3l"
	"\t007#\t027Name\t110Cls\t140Lvl\t176H.P."
	"\t212S.P.\t241A.C.\t270Cond"
	"%s%s%s%s%s%s%s%s"
	"\v110\t064\x3""cGold\t144Gems\t224Food\v119"
	"\t064\f15%u\t144%u\t224%u day%s\fd";

static const int BLACKSMITH_MAP_IDS[2][4] = { { 28, 30, 73, 49 }, { 29, 31, 37, 43 } };

static const char *const ITEMS_DIALOG_TEXT1 =
	"\r\x2\x3""c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
	"\f37c\fdces\t119\f37M\fdisc\t153%s\t187%s\t221%s"
	"\t255%s\t289Exit";
static const char *const ITEMS_DIALOG_TEXT2 =
	"\r\x2\x3""c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
	"\f37c\fdces\t119\f37M\fdisc\t153\f37%s\t289Exit";
static const char *const ITEMS_DIALOG_LINE1 = "\x3r\f%02u\t023%2d)\x3l\t028%s\n";
static const char *const ITEMS_DIALOG_LINE2 = "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%u\n";

static const char *const BTN_BUY = "\f37B\fduy";
static const char *const BTN_SELL = "\f37S\fdell";
static const char *const BTN_IDENTIFY = "\f37I\fddentify";
static const char *const BTN_FIX = "\f37F\fdix";
static const char *const BTN_USE = "\f37U\fdse";
static const char *const BTN_EQUIP = "\f37E\fdquip";
static const char *const BTN_REMOVE = "\f37R\fdem";
static const char *const BTN_DISCARD = "\f37D\fdisc";
static const char *const BTN_QUEST = "\f37Q\fduest";
static const char *const BTN_ENCHANT = "E\fdnchant";
static const char *const BTN_RECHARGE = "R\fdechrg";
static const char *const BTN_GOLD = "G\fdold";

static const char *const ITEM_BROKEN = "\f32broken ";
static const char *const ITEM_CURSED = "\f09cursed ";
static const char *const ITEM_OF = "of ";

static const char *const BONUS_NAMES[7] = {
	"", "Dragon Slayer", "Undead Eater", "Golem Smasher",
	"Bug Zapper", "Monster Masher", "Beast Bopper"
};

static const char *const WEAPON_NAMES[41] = {
	nullptr, "long sword ", "short sword ", "broad sword ", "scimitar ",
	"cutlass ", "sabre ", "club ", "hand axe ", "katana ", "nunchakas ",
	"wakazashi ", "dagger ", "mace ", "flail ", "cudgel ", "maul ", "spear ",
	"bardiche ", "glaive ", "halberd ", "pike ", "flamberge ", "trident ",
	"staff ", "hammer ", "naginata ", "battle axe ", "grand axe ", "great axe ",
	"short bow ", "long bow ", "crossbow ", "sling ", "Xeen Slayer Sword ",
	"Elder LongSword ", "Elder Dagger ", "Elder Mace ", "Elder Spear ",
	"Elder Staff ", "Elder LongBow "
};

static const char *const ARMOR_NAMES[14] = {
	nullptr, "robes ", "scale armor ", "ring mail ", "chain mail ",
	"splint mail ", "plate mail ", "plate armor ", "shield ",
	"helm ", "boots ", "cloak ", "cape ", "gauntlets "
};

static const char *const ACCESSORY_NAMES[11] = {
	nullptr, "ring ", "belt ", "brooch ", "medal ", "charm ", "cameo ",
	"scarab ", "pendant ", "necklace ", "amulet "
};

static const char *const MISC_NAMES[22] = {
	nullptr, "rod ", "jewel ", "gem ", "box ", "orb ", "horn ", "coin ",
	"wand ", "whistle ", "potion ", "scroll ", "bogus", "bogus", "bogus",
	"bogus", "bogus", "bogus", "bogus", "bogus", "bogus", "bogus"
};

static const char *const SPECIAL_NAMES[74] = {
	nullptr, "light", "awakening", "magic arrows", "first aid", "fists", "energy blasts", "sleeping",
	"revitalization", "curing", "sparking", "shrapmetal", "insect repellent", "toxic clouds", "elemental protection",
	"pain", "jumping", "beast control", "clairvoyance", "undead turning", "levitation", "wizard eyes", "blessing",
	"monster identification",  "lightning", "holy bonuses", "power curing", "nature's cures", "beacons",
	"shielding", "heroism", "hypnotism", "water walking", "frost biting", "monster finding", "fireballs",
	"cold rays", "antidotes", "acid spraying", "time distortion", "dragon sleep", "vaccination", "teleportation",
	"death", "free movement", "golem stopping", "poison volleys", "deadly swarms", "shelter", "daily protection",
	"daily sorcerery", "feasting", "fiery flails", "recharging", "freezing", "town portals", "stone to flesh",
	"raising the dead", "etherealization", "dancing swords", "moon rays", "mass distortion", "prismatic light",
	"enchant item", "incinerating", "holy words", "resurrection", "storms", "megavoltage", "infernos",
	"sun rays", "implosions", "star bursts", "the GODS!"
};

static const char *const ELEMENTAL_NAMES[6] = {
	"Fire", "Elec", "Cold", "Acid/Poison", "Energy", "Magic"
};

static const char *const ATTRIBUTE_NAMES[10] = {
	"might", "Intellect", "Personality", "Speed", "accuracy", "Luck",
	"Hit Points", "Spell Points", "Armor Class", "Thievery"
};

static const char *const EFFECTIVENESS_NAMES[7] = {
	nullptr, "Dragons", "Undead", "Golems", "Bugs", "Monsters", "Beasts"
};

static const char *const QUEST_ITEM_NAMES[85] = {
	"Deed to New Castle",
	"Crystal Key to Witch Tower",
	"Skeleton Key to Darzog's Tower",
	"Enchanted Key to Tower of High Magic",
	"Jeweled Amulet of the Northern Sphinx",
	"Stone of a Thousand Terrors",
	"Golem Stone of Admittance",
	"Yak Stone of Opening",
	"Xeen's Scepter of Temporal Distortion",
	"Alacorn of Falista",
	"Elixir of Restoration",
	"Wand of Faery Magic",
	"Princess Roxanne's Tiara",
	"Holy Book of Elvenkind",
	"Scarab of Imaging",
	"Crystals of Piezoelectricity",
	"Scroll of Insight",
	"Phirna Root",
	"Orothin's Bone Whistle",
	"Barok's Magic Pendant",
	"Ligono's Missing Skull",
	"Last Flower of Summer",
	"Last Raindrop of Spring",
	"Last Snowflake of Winter",
	"Last Leaf of Autumn",
	"Ever Hot Lava Rock",
	"King's Mega Credit",
	"Excavation Permit",
	"Cupie Doll",
	"Might Doll",
	"Speed Doll",
	"Endurance Doll",
	"Accuracy Doll",
	"Luck Doll",
	"Widget",
	"Pass to Castleview",
	"Pass to Sandcaster",
	"Pass to Lakeside",
	"Pass to Necropolis",
	"Pass to Olympus",
	"Key to Great Western Tower",
	"Key to Great Southern Tower",
	"Key to Great Eastern Tower",
	"Key to Great Northern Tower",
	"Key to Ellinger's Tower",
	"Key to Dragon Tower",
	"Key to Darkstone Tower",
	"Key to Temple of Bark",
	"Key to Dungeon of Lost Souls",
	"Key to Ancient Pyramid",
	"Key to Dungeon of Death",
	"Amulet of the Southern Sphinx",
	"Dragon Pharoah's Orb",
	"Cube of Power",
	"Chime of Opening",
	"Gold ID Card",
	"Silver ID Card",
	"Vulture Repellant",
	"Bridle",
	"Enchanted Bridle",
	"Treasure Map (Goto E1 x1, y11)",
	"",
	"Fake Map",
	"Onyx Necklace",
	"Dragon Egg",
	"Tribble",
	"Golden Pegasus Statuette",
	"Golden Dragon Statuette",
	"Golden Griffin Statuette",
	"Chalice of Protection",
	"Jewel of Ages",
	"Songbird of Serenity",
	"Sandro's Heart",
	"Ector's Ring",
	"Vespar's Emerald Handle",
	"Queen Kalindra's Crown",
	"Caleb's Magnifying Glass",
	"Soul Box",
	"Soul Box with Corak inside",
	"Ruby Rock",
	"Emerald Rock",
	"Sapphire Rock",
	"Diamond Rock",
	"Monga Melon",
	"Energy Disk"
};

static const char *QUEST_ITEM_NAMES_SWORDS[51] = {
	"Pass to Hart", "Pass to Impery", "Pass to town3", "Pass to town4", "Pass to town5",
	"Key to Hart Sewers", "Key to Rettig's Pyramid", "Key to the Old Temple",
	"Key to Canegtut's Pyramid", "Key to Ascihep's Pyramid", "Key to Dragon Tower",
	"Key to Darkstone Tower", "Key to Temple of Bark", "Key to Dungeon of Lost Souls",
	"Key to Ancient Pyramid", "Key to Dungeon of Death", "Red Magic Hammer",
	"Green Magic Hammer", "Golden Magic Wand", "Silver Magic Hammer", "Magic Coin",
	"Ruby", "Diamond Mineral", "Emerald", "Sapphire", "Treasure Map (Goto E1 x1, y11)",
	"NOTUSED", "Melon", "Princess Crown", "Emerald Wand", "Druid Carving", "High Sign",
	"Holy Wheel", "Double Cross", "Sky Hook", "Sacred Cow", "Staff of the Mountain",
	"Hard Rock", "Soft Rock", "Rock Candy", "Ivy Plant", "Spirit Gem", "Temple of Sun holy lamp oil",
	"Noams Hammer", "Positive Orb", "Negative Orb", "FireBane Staff", "Diamond Edged Pick",
	"Monga Melon", "Energy Disk", "Old XEEN Quest Item" 
};

static const int WEAPON_BASE_COSTS[35] = {
	0, 50, 15, 100, 80, 40, 60, 1, 10, 150, 30, 60, 8, 50,
	100, 15, 30, 15, 200, 80, 250, 150, 400, 100, 40, 120,
	300, 100, 200, 300, 25, 100, 50, 15, 0
};
static const int ARMOR_BASE_COSTS[14] = {
	0, 20, 100, 200, 400, 600, 1000, 2000, 100, 60, 40, 250, 200, 100
};
static const int ACCESSORY_BASE_COSTS[11] = {
	0, 100, 100, 250, 100, 50, 300, 200, 500, 1000, 2000
};
static const int MISC_MATERIAL_COSTS[22] = {
	0, 50, 1000, 500, 10, 100, 20, 10, 50, 10, 10, 100,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const int MISC_BASE_COSTS[76] = {
	0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200, 200,
	200, 200, 200, 200, 200, 200, 200, 300, 300, 300, 300, 300,
	300, 300, 300, 300, 300, 400, 400, 400, 400, 400, 400, 400,
	400, 400, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
	600, 600, 600, 600
};
static const int METAL_BASE_MULTIPLIERS[22] = {
	10, 25, 5, 75, 2, 5, 10, 20, 50, 2, 3, 5, 10, 20, 30, 40,
	50, 60, 70, 80, 90, 100
};
static const int ITEM_SKILL_DIVISORS[4] = { 1, 2, 100, 10 };

static const int RESTRICTION_OFFSETS[4] = { 0, 35, 49, 60 };

static const int ITEM_RESTRICTIONS[86] = {
	0, 86, 86, 86, 86, 86, 86, 0, 6, 239, 239, 239, 2, 4, 4, 4, 4,
	6, 70, 70, 70, 70, 94, 70, 0, 4, 239, 86, 86, 86, 70, 70, 70, 70,
	0, 0, 0, 68, 100, 116, 125, 255, 255, 85, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char *const NOT_PROFICIENT =
	"\t000\v007\x3""c%ss are not proficient with a %s!";

static const char *const NO_ITEMS_AVAILABLE = "\x3""c\n"
	"\t000No items available.";

static const char *const CATEGORY_NAMES[4] = { "Weapons", "Armor", "Accessories", "Miscellaneous" };

static const char *const X_FOR_THE_Y =
	"\x1\fd\r%s\v000\t000%s for %s the %s%s\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";

static const char *const X_FOR_Y =
	"\x1\fd\r\x3l\v000\t000%s for %s\x3r\t000%s\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";

static const char *const X_FOR_Y_GOLD =
	"\x1\fd\r\x3l\v000\t000%s for %s\t150Gold - %u%s\x3l\v011"
	"\x2%s%s%s%s%s%s%s%s%s\x1\fd";

static const char *const FMT_CHARGES = "\x3rr\t000Charges\x3l";

static const char *const AVAILABLE_GOLD_COST =
	"\x1\fd\r\x3l\v000\t000Available %s\t150Gold - %u\x3r\t000Cost"
	"\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";

static const char *const CHARGES = "Charges";

static const char *const COST = "Cost";

static const char *const ITEM_ACTIONS[7] = {
	"Equip", "Remove", "Use", "Discard", "Enchant", "Recharge", "Gold"
};
static const char *const WHICH_ITEM = "\t010\v005%s which item?";

static const char *const WHATS_YOUR_HURRY = "\v007What's your hurry?\n"
	"Wait till you get out of here!";

static const char *const USE_ITEM_IN_COMBAT =
	"\v007To use an item in Combat, invoke the Use command on your turn!";

static const char *const NO_SPECIAL_ABILITIES = "\v005\x3""c%s\fdhas no special abilities!";

static const char *const CANT_CAST_WHILE_ENGAGED = "\x3""c\v007Can't cast %s while engaged!";

static const char *const EQUIPPED_ALL_YOU_CAN = "\x3""c\v007You have equipped all the %ss you can!";
static const char *const REMOVE_X_TO_EQUIP_Y = "\x3""c\v007You must remove %sto equip %s\b!";
static const char *const RING = "ring";
static const char *const MEDAL = "medal";

static const char *const CANNOT_REMOVE_CURSED_ITEM = "\x3""You cannot remove a cursed item!";

static const char *const CANNOT_DISCARD_CURSED_ITEM = "\3x""cYou cannot discard a cursed item!";

static const char *const PERMANENTLY_DISCARD = "\v000\t000\x3lPermanently discard %s\fd?";

static const char *const BACKPACK_IS_FULL = "\v005\x3""c\fd%s's backpack is full.";

static const char *const CATEGORY_BACKPACK_IS_FULL[4] = {
	"\v010\t000\x3""c%s's weapons backpack is full.",
	"\v010\t000\x3""c%s's armor backpack is full.",
	"\v010\t000\x3""c%s's accessories backpack is full.",
	"\v010\t000\x3""c%s's miscellaneous backpack is full."
};

static const char *const BUY_X_FOR_Y_GOLD = "\x3l\v000\t000\fdBuy %s\fd for %u gold?";

static const char *const SELL_X_FOR_Y_GOLD = "\x3l\v000\t000\fdSell %s\fd for %u gold?";

static const char *const NO_NEED_OF_THIS = "\v005\x3""c\fdWe have no need of this %s\f!";

static const char *const NOT_RECHARGABLE = "\v012\x3""c\fdNot Rechargeable.  %s";

static const char *const NOT_ENCHANTABLE = "\v012\t000\x3""cNot Enchantable.  %s";

static const char *const SPELL_FAILED = "Spell Failed!";

static const char *const ITEM_NOT_BROKEN =  "\fdThat item is not broken!";

static const char *const FIX_IDENTIFY[2] = { "Fix", "Identify" };

static const char *const FIX_IDENTIFY_GOLD = "\x3l\v000\t000%s %s\fd for %u gold?";

static const char *const IDENTIFY_ITEM_MSG = "\fd\v000\t000\x3""cIdentify Item\x3l\n"
	"\n"
	"\v012%s\fd\n"
	"\n"
	"%s";

static const char *const ITEM_DETAILS =
	"Proficient Classes\t132:\t140%s\n"
	"to Hit Modifier\t132:\t140%s\n"
	"Physical Damage\t132:\t140%s\n"
	"Elemental Damage\t132:\t140%s\n"
	"Elemental Resistance\t132:\t140%s\n"
	"Armor Class Bonus\t132:\t140%s\n"
	"Attribute Bonus\t132:\t140%s\n"
	"Special Power\t132:\t140%s";

static const char *const ALL = "All";
static const char *const FIELD_NONE = "None";
static const char *const DAMAGE_X_TO_Y = "%d to %d";
static const char *const ELEMENTAL_XY_DAMAGE = "%+d %s Damage";
static const char *const ATTR_XY_BONUS = "%+d %s";
static const char *const EFFECTIVE_AGAINST = "x3 vs %s";

static const char *const QUESTS_DIALOG_TEXT =
	"\r\x2\x3""c\v021\t017\f37I\fdtems\t085\f37Q\fduests\t153"
	"\f37A\fduto Notes	221\f37U\fdp\t255\f37D\fdown"
	"\t289Exit";
static const char *const CLOUDS_OF_XEEN_LINE = "\b \b*-- \f04Clouds of Xeen\fd --";
static const char *const DARKSIDE_OF_XEEN_LINE = "\b \b*-- \f04Darkside of Xeen\fd --";
static const char *const SWORDS_OF_XEEN_LINE = "\b \b*-- \f04Swords of Xeen\fd --";

static const char *const NO_QUEST_ITEMS =
	"\r\x3""c\v000	000Quest Items\x3l\x2\n"
	"\n"
	"\x3""cNo Quest Items";
static const char *const NO_CURRENT_QUESTS =
	"\x3""c\v000\t000\n"
	"\n"
	"No Current Quests";
static const char *const NO_AUTO_NOTES = "\x3""cNo Auto Notes";

static const char *const QUEST_ITEMS_DATA =
	"\r\x1\fd\x3""c\v000\t000Quest Items\x3l\x2\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s";
static const char *const CURRENT_QUESTS_DATA =
	"\r\x1\fd\x3""c\t000\v000Current Quests\x3l\x2\n"
	"%s\n"
	"\n"
	"%s\n"
	"\n"
	"%s";
static const char *const AUTO_NOTES_DATA =
	"\r\x1\fd\x3""c\t000\v000Auto Notes\x3l\x2\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l";

static const char *const REST_COMPLETE =
	"\v000\t0008 hours pass.  Rest complete.\n"
	"%s\n"
	"%d food consumed.";
static const char *const PARTY_IS_STARVING = "\f07The Party is Starving!\fd";
static const char *const HIT_SPELL_POINTS_RESTORED = "Hit Pts and Spell Pts restored.";
static const char *const TOO_DANGEROUS_TO_REST = "Too dangerous to rest here!";
static const char *const SOME_CHARS_MAY_DIE = "Some Chars may die. Rest anyway?";

static const char *const DISMISS_WHOM = "\t010\v005Dismiss whom?";
static const char *const CANT_DISMISS_LAST_CHAR = "You cannot dismiss your last character!";

static const char *const DELETE_CHAR_WITH_ELDER_WEAPON = "\v000\t000This character has an Elder Weapon and cannot be deleted!";

static const char *const REMOVE_DELETE[2] = { "Remove", "Delete" };

static const char *const REMOVE_OR_DELETE_WHICH = "\x3l\t010\v005%s which character?";

static const char *const YOUR_PARTY_IS_FULL = "\v007Your party is full!";

static const char *const HAS_SLAYER_SWORD =
	"\v000\t000This character has the Xeen Slayer Sword and cannot be deleted!";
static const char *const SURE_TO_DELETE_CHAR =
	"Are you sure you want to delete %s the %s?";

static const char *const CREATE_CHAR_DETAILS =
	"\f04\x3""c\x2\t144\v119\f37R\f04oll\t144\v149\f37C\f04reate"
	"\t144\v179\f37ESC\f04\x3l\x1\t195\v021\f37M\f04gt"
	"\t195\v045\f37I\f04nt\t195\v069\f37P\f04er\t195\v093\f37E\f04nd"
	"\t195\v116\f37S\f04pd\t195\v140\f37A\f04cy\t195\v164\f37L\f04ck%s";

static const char *const NEW_CHAR_STATS =
	"\f04\x3l\t022\v148Race\t055: %s\n"
	"\t022Sex\t055: %s\n"
	"\t022Class\t055:\n"
	"\x3r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
	"\t215\v151%d\t215\v175%d\x3l\t242\v020\f%.2dKnight\t242\v031\f%.2d"
	"Paladin\t242\v042\f%.2dArcher\t242\v053\f%.2dCleric\t242\v064\f%.2d"
	"Sorcerer\t242\v075\f%.2dRobber\t242\v086\f%.2dNinja\t242\v097\f%.2d"
	"Barbarian\t242\v108\f%.2dDruid\t242\v119\f%.2dRanger\f04\x3""c"
	"\t265\v142Skills\x3l\t223\v155%s\t223\v170%s%s";

static const char *const NAME_FOR_NEW_CHARACTER =
	"\x3""cEnter a Name for this Character\n\n";
static const char *const SELECT_CLASS_BEFORE_SAVING =
	"\v006\x3""cSelect a Class before saving.\x3l";
static const char *const EXCHANGE_ATTR_WITH = "Exchange %s with...";

static const int NEW_CHAR_SKILLS[10] = { 1, 5, -1, -1, 4, 0, 0, -1, 6, 11 };
static const int NEW_CHAR_SKILLS_OFFSET[10] = { 0, 0, 0, 5, 0, 0, 0, 0, 0, 0 };
static const int NEW_CHAR_SKILLS_LEN[10] = { 11, 8, 0, 0, 12, 8, 8, 0, 9, 11 };
static const int NEW_CHAR_RACE_SKILLS[10] = { 14, -1, 17, 16, -1, 0, 0, 0, 0, 0 };

static const int RACE_MAGIC_RESISTENCES[5] = { 7, 5, 20, 0, 0 };
static const int RACE_FIRE_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
static const int RACE_ELECTRIC_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
static const int RACE_COLD_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
static const int RACE_ENERGY_RESISTENCES[5] = { 7, 5, 2, 5, 0 };
static const int RACE_POISON_RESISTENCES[5] = { 7, 0, 2, 20, 0 };
static const int NEW_CHARACTER_SPELLS[10][4] = {
	{ -1, -1, -1, -1 },
	{ 21, -1, -1, -1 },
	{ 22, -1, -1, -1 },
	{ 21, 1, 14, -1 },
	{ 22, 0, 25, -1 },
	{ -1, -1, -1, -1 },
	{ -1, -1, -1, -1 },
	{ -1, -1, -1, -1 },
	{ 20, 1, 11, 23 },
	{ 20, 1, -1, -1 }
};

static const char *const COMBAT_DETAILS = "\r\f00\x3""c\v000\t000\x2""Combat%s%s%s\x1";

static const char *NOT_ENOUGH_TO_CAST = "\x3""c\v010Not enough %s to Cast %s";
static const char *SPELL_CAST_COMPONENTS[2] = { "Spell Points", "Gems" };

static const char *const CAST_SPELL_DETAILS =
	"\r\x2\x3""c\v122\t013\f37C\fdast\t040\f37N\fdew"
	"\t067ESC\x1\t000\v000\x3""cCast Spell\n"
	"\n"
	"%s\x3l\n"
	"\n"
	"Spell Ready:\x3""c\n"
	"\n"
	"\f09%s\fd\x2\x3l\n"
	"\v082Cost\x3r\t000%u/%u\x3l\n"
	"Cur SP\x3r\t000%u\x1";

static const char *const PARTY_FOUND =
	"\x3""cThe Party Found:\n"
	"\n"
	"\x3r\t000%u Gold\n"
	"%u Gems";

static const char *const BACKPACKS_FULL_PRESS_KEY =
	"\v007\f12Warning!  BackPacks Full!\fd\n"
	"Press a Key";

static const char *const HIT_A_KEY = "\x3l\v120\t000\x4""077\x3""c\f37Hit a key\fd";

static const char *const GIVE_TREASURE_FORMATTING =
	"\x3l\v060\t000\x4""077\n"
	"\x4""077\n"
	"\x4""077\n"
	"\x4""077\n"
	"\x4""077\n"
	"\x4""077";

static const char *const X_FOUND_Y = "\v060\t000\x3""c%s found: %s";

static const char *const ON_WHO = "\x3""c\v009On Who?";

static const char *const WHICH_ELEMENT1 =
	"\r\x3""c\x1Which Element?\x2\v034\t014\f15F\fdire\t044"
	"\f15E\fdlec\t074\f15C\fdold\t104\f15A\fdcid\x1";

static const char *const WHICH_ELEMENT2 =
	"\r\x3""cWhich Element?\x2\v034\t014\f15F\fdire\t044"
	"\f15E\fdlec\t074\f15C\fdold\t104\f15A\fdcid\x1";

static const char *const DETECT_MONSTERS = "\x3""cDetect Monsters";

static const char *const LLOYDS_BEACON =
	"\r\x3""c\v000\t000\x1Lloyd's Beacon\n"
	"\n"
	"Last Location\n"
	"\n"
	"%s\x3l\n"
	"x = %d\x3r\t000y = %d\x3""c\x2\v122\t021\f15S\fdet\t060\f15R\fdeturn\x1";

static const char *const HOW_MANY_SQUARES = "\x3""cTeleport\nHow many squares %s (1-9)\n";

static const char *const TOWN_PORTAL =
	"\x3""cTown Portal\x3l\n"
	"\n"
	"\t0101. %s\n"
	"\t0102. %s\n"
	"\t0103. %s\n"
	"\t0104. %s\n"
	"\t0105. %s\x3""c\n"
	"\n"
	"To which Town (1-5)\n"
	"\n";

static const char *const TOWN_PORTAL_SWORDS =
	"\x3""cTown Portal\x3l\n"
	"\n"
	"\t0101. %s\n"
	"\t0102. %s\n"
	"\t0103. %s\x3""c\n"
	"\n"
	"To which Town (1-3)\n"
	"\n";

static const int TOWN_MAP_NUMBERS[3][5] = {
	{ 28, 29, 30, 31, 32 }, { 29, 31, 33, 35, 37 }, { 53, 92, 63, 0, 0 }
};

static const char *const MONSTER_DETAILS =
	"\x3l\n"
	"%s\x3""c\t100%s\t140%u\t180%u\x3r\t000%s";

static const char *const MONSTER_SPECIAL_ATTACKS[23] = {
	"None", "Magic", "Fire", "Elec", "Cold", "Poison", "Energy", "Disease",
	"Insane", "Asleep", "CurseItm", "InLove", "DrnSPts", "Curse", "Paralys",
	"Uncons", "Confuse", "BrkWpn", "Weak", "Erad", "Age+5", "Dead", "Stone"
};

static const char *const IDENTIFY_MONSTERS =
	"Name\x3""c\t100HP\t140AC\t177#Atks\x3r\t000Special%s%s%s";

static const char *const EVENT_SAMPLES[6] = {
	"ahh.voc", "whereto.voc", "gulp.voc", "null.voc", "scream.voc", "laff1.voc"
};

static const char *const MOONS_NOT_ALIGNED =
"\x3""c\v012\t000The moons are not aligned. Passage to the %s is unavailable";

static const char *const AWARDS_FOR =
	"\r\x1\fd\x3""c\v000\t000Awards for %s the %s\x3l\x2\n"
	"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";

static const char *const AWARDS_TEXT =
	"\r\x2\x3""c\v021\t221\f37U\fdp\t255\f37D\fdown\t289Exit";

static const char *const NO_AWARDS = "\x3""cNo Awards";

static const char *const WARZONE_BATTLE_MASTER = "The Warzone\n\t125Battle Master";

static const char *const WARZONE_MAXED = "What!  You again?  Go pick on someone your own size!";

static const char *const WARZONE_LEVEL = "What level of monsters? (1-10)\n";

static const char *const WARZONE_HOW_MANY = "How many monsters? (1-20)\n";

static const char *const PICKS_THE_LOCK = "\x3""c\v010%s picks the lock!\nPress any key.";

static const char *const UNABLE_TO_PICK_LOCK = "\x3""c\v010%s was unable to pick the lock!\nPress any key.";

static const char *const CONTROL_PANEL_TEXT =
	"\x1\f00\x3""c\v000\t000Control Panel\x3r"
	"\v022\t045\f06L\fdoad:\t124\f06E\fdfx:"
	"\v041\t045\f06S\fdave:\t124\f06M\fdusic:"
	"\v060\t045\f06Q\fduit:"
	"\v080\t084Mr \f06W\fdizard:%s\t000\x1";
static const char *const CONTROL_PANEL_BUTTONS =
	"\x3""c\f11"
	"\v022\t062load\t141%s"
	"\v041\t062save\t141%s"
	"\v060\t062exit"
	"\v079\t102Help\fd";
static const char *const ON = "\f15on\f11";
static const char *const OFF = "\f32off\f11";
static const char *const CONFIRM_QUIT = "Are you sure you want to quit?";
static const char *const MR_WIZARD =
	"Are you sure you want Mr.Wizard's Help ?";
static const char *const NO_LOADING_IN_COMBAT =
	"No Loading Allowed in Combat!";
static const char *const NO_SAVING_IN_COMBAT =
	"No Saving Allowed in Combat!";
static const char *const QUICK_FIGHT_TEXT = "\r\fd\x3""c\v000\t000QuickFight Options\n\n"
	"%s\x3l\n\n"
	"Current\x3r\n"
	"\t000%s\x2\x3""c\v122\t021\f37N\f04ext\t060Exit\x1";
static const char *const QUICK_FIGHT_OPTIONS[4] = { "Attack", "Cast", "Block", "Run" };

static const char *const WORLD_END_TEXT[9] = {
	"\n\n\n\n\n\n\n"
	"Congratulations Adventurers!\n\n"
	"Let the unification ceremony begin!",
	"And so the call went out to the people throughout the lands of Xeen"
	" that the prophecy was nearing completion.",
	"They came in great numbers to witness the momentous occasion.",
	"\v026The Dragon Pharoah presided over the ceremony.",
	"\v026Queen Kalindra presented the Cube of Power.",
	"\v026Prince Roland presented the Xeen Sceptre.",
	"\v026Together, they placed the Cube of Power...",
	"\v026and the Sceptre, onto the Altar of Joining.",
	"With the prophecy complete, the two sides of Xeen were united as one",
};

static const char *const WORLD_CONGRATULATIONS =
	"\x3""cCongratulations\n\n"
	"Your Final Score is:\n\n"
	"%010lu\n"
	"\x3l\n"
	"Please send this score to the Ancient's Headquarters where "
	"you'll be added to the Hall of Legends!\n\n"
	"Ancient's Headquarters\n"
	"New World Computing, Inc.\n"
	"P.O. Box 4302\n"
	"Hollywood, CA 90078";
static const char *const WORLD_CONGRATULATIONS2 =
	"\n\n\n\n\n\n"
	"But wait... there's more!\n"
	"\n\n"
	"Include the message\n"
	"\"%s\"\n"
	"with your final score and receive a special bonus.";
static const char *const CLOUDS_CONGRATULATIONS1 =
	"\f23\x3l"
	"\v000\t000Please send this score to the Ancient's Headquarters "
	"where you'll be added to the Hall of Legends!\f33\x3""c"
	"\v070\t000Press a Key";
static const char *const CLOUDS_CONGRATULATIONS2 =
	"\f23\x3l"
	"\v000\t000Ancient's Headquarters\n"
	"New World Computing, Inc.\n"
	"P.O. Box 4302\n"
	"Hollywood, CA 90078-4302\f33\x3""c"
	"\v070\t000Press a Key";
static const char *const GOOBER[3] = {
	"", "I am a Goober!", "I am a Super Goober!"
};

static const char *const MUSIC_FILES1[5] = {
	"outdoors.m", "town.m", "cavern.m", "dungeon.m", "castle.m"
};

static const char *const MUSIC_FILES2[6][7] = {
	{ "outday1.m", "outday2.m", "outday4.m", "outnght1.m",
	"outnght2.m", "outnght4.m", "daydesrt.m" },
	{ "townday1.m", "twnwlk.m", "newbrigh.m", "twnnitea.m",
	"twnniteb.m", "twnwlk.m", "townday1.m" },
	{ "cavern1.m", "cavern2.m", "cavern3a.m", "cavern1.m",
	"cavern2.m", "cavern3a.m", "cavern1.m" },
	{ "dngon1.m", "dngon2.m", "dngon3.m", "dngon1.m",
	"dngon2.m", "dngon3.m", "dngon1.m" },
	{ "cstl1rev.m", "cstl2rev.m", "cstl3rev.m", "cstl1rev.m",
	"cstl2rev.m", "cstl3rev.m", "cstl1rev.m" },
	{ "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m" }
};

static const char *const DIFFICULTY_TEXT = "\v000\t000\x3""cSelect Game Preference";
static const char *const SAVE_OFF_LIMITS = "\x3""c\v002\t000The Gods of Game Restoration deem this area off limits!\n"
	"Sorry, no saving in this maze.";

static const char *const CLOUDS_INTRO1 = "\f00\v082\t040\x3"
	"cKing Burlock\v190\t040Peasants\v082\t247"
	"Lord Xeen\v190\t258Xeen's Pet\v179\t150Crodo";

static const char *const DARKSIDE_ENDING1 = "\n\x3" "cCongratulations\n"
	"\n"
	"Your Final Score is:\n"
	"\n"
	"%010lu\n"
	"\x3" "l\n"
	"Please send this score to the Ancient's Headquarters "
	"where you'll be added to the Hall of Legends!\n"
	"\n"
	"Ancient's Headquarters\n"
	"New World Computing, Inc.\n"
	"P.O. Box 4302\n"
	"Hollywood, CA 90078";

static const char *const DARKSIDE_ENDING2 = "\n"
	"Adventurers,\n"
	"\n"
	"I will save your game in Castleview.\n"
	"\n"
	"The World of Xeen still needs you!\n"
	"\n"
	"Load your game afterwards and come visit me in the "
	"Great Pyramid for further instructions";

static const char *const PHAROAH_ENDING_TEXT1 = "\fd\v001\t001%s\x3" "c\t000\v180Press a Key!\x3" "l";
static const char *const PHAROAH_ENDING_TEXT2 = "\f04\v000\t000%s\x3" "c\t000\v180Press a Key!\x3" "l\fd";


void writeConstants(CCArchive &cc) {
	Common::MemFile file;
	file.syncString(CLOUDS_CREDITS);
	file.syncString(DARK_SIDE_CREDITS);
	file.syncString(SWORDS_CREDITS1);
	file.syncString(SWORDS_CREDITS2);
	file.syncString(OPTIONS_MENU);
	file.syncStrings(GAME_NAMES, 3);
	file.syncString(THE_PARTY_NEEDS_REST);
	file.syncString(WHO_WILL);
	file.syncString(HOW_MUCH);
	file.syncString(WHATS_THE_PASSWORD);
	file.syncString(PASSWORD_INCORRECT);
	file.syncString(IN_NO_CONDITION);
	file.syncString(NOTHING_HERE);
	file.syncStrings(TERRAIN_TYPES, 6);
	file.syncStrings(OUTDOORS_WALL_TYPES, 16);
	file.syncStrings(SURFACE_NAMES, 16);
	file.syncStrings(WHO_ACTIONS, 32);
	file.syncStrings(WHO_WILL_ACTIONS, 4);
	file.syncBytes2D((const byte *)SYMBOLS, 20, 64);
	file.syncBytes2D((const byte *)TEXT_COLORS, 40, 4);
	file.syncBytes2D((const byte *)TEXT_COLORS_STARTUP, 40, 4);
	file.syncStrings(DIRECTION_TEXT_UPPER, 4);
	file.syncStrings(DIRECTION_TEXT, 4);
	file.syncStrings(RACE_NAMES, 5);
	file.syncNumbers(RACE_HP_BONUSES, 5);
	file.syncNumbers2D((const int *)RACE_SP_BONUSES, 5, 2);
	file.syncStrings(CLASS_NAMES, 11);
	file.syncNumbers(CLASS_EXP_LEVELS, 10);
	file.syncStrings(ALIGNMENT_NAMES, 3);
	file.syncStrings(SEX_NAMES, 2);
	file.syncStrings(SKILL_NAMES, 18);
	file.syncStrings(CONDITION_NAMES, 17);
	file.syncNumbers(CONDITION_COLORS, 17);
	file.syncString(GOOD);
	file.syncString(BLESSED);
	file.syncString(POWER_SHIELD);
	file.syncString(HOLY_BONUS);
	file.syncString(HEROISM);
	file.syncString(IN_PARTY);
	file.syncString(PARTY_DETAILS);
	file.syncString(PARTY_DIALOG_TEXT);
	file.syncNumbers(FACE_CONDITION_FRAMES, 17);
	file.syncNumbers(CHAR_FACES_X, 6);
	file.syncNumbers(HP_BARS_X, 6);
	file.syncString(NO_ONE_TO_ADVENTURE_WITH);
	file.syncBytes2D((const byte *)DARKNESS_XLAT, 3, 256);
	file.syncString(YOUR_ROSTER_IS_FULL);
	file.syncString(PLEASE_WAIT);
	file.syncString(OOPS);
	file.syncNumbers2D((const int *)SCREEN_POSITIONING_X, 4, 48);
	file.syncNumbers2D((const int *)SCREEN_POSITIONING_Y, 4, 48);
	file.syncNumbers(MONSTER_GRID_BITMASK, 12);
	file.syncNumbers2D((const int *)INDOOR_OBJECT_X, 2, 12);
	file.syncNumbers2D((const int *)MAP_OBJECT_Y, 2, 12);
	file.syncNumbers(INDOOR_MONSTERS_Y, 4);
	file.syncNumbers2D((const int *)OUTDOOR_OBJECT_X, 2, 12);
	file.syncNumbers(OUTDOOR_MONSTER_INDEXES, 26);
	file.syncNumbers(OUTDOOR_MONSTERS_Y, 26);
	file.syncNumbers2D((const int *)DIRECTION_ANIM_POSITIONS, 4, 4);
	file.syncBytes2D((const byte *)WALL_SHIFTS, 4, 48);
	file.syncNumbers(DRAW_NUMBERS, 25);
	file.syncNumbers2D((const int *)DRAW_FRAMES, 25, 2);
	file.syncNumbers(COMBAT_FLOAT_X, 8);
	file.syncNumbers(COMBAT_FLOAT_Y, 8);
	file.syncNumbers2D((const int *)MONSTER_EFFECT_FLAGS, 15, 8);
	file.syncNumbers2D((const int *)SPELLS_ALLOWED, 3, 40);
	file.syncNumbers(BASE_HP_BY_CLASS, 10);
	file.syncNumbers(AGE_RANGES, 10);
	file.syncNumbers2D((const int *)AGE_RANGES_ADJUST, 2, 10);
	file.syncNumbers(STAT_VALUES, 24);
	file.syncNumbers(STAT_BONUSES, 24);
	file.syncNumbers(ELEMENTAL_CATEGORIES, 6);
	file.syncNumbers(ATTRIBUTE_CATEGORIES, 10);
	file.syncNumbers(ATTRIBUTE_BONUSES, 72);
	file.syncNumbers(ELEMENTAL_RESISTENCES, 37);
	file.syncNumbers(ELEMENTAL_DAMAGE, 37);
	file.syncNumbers(WEAPON_DAMAGE_BASE, 35);
	file.syncNumbers(WEAPON_DAMAGE_MULTIPLIER, 35);
	file.syncNumbers(METAL_DAMAGE, 22);
	file.syncNumbers(METAL_DAMAGE_PERCENT, 22);
	file.syncNumbers(METAL_LAC, 22);
	file.syncNumbers(ARMOR_STRENGTHS, 14);
	file.syncNumbers(MAKE_ITEM_ARR1, 6);
	file.syncNumbers3D((const int *)MAKE_ITEM_ARR2, 6, 7, 2);
	file.syncNumbers3D((const int *)MAKE_ITEM_ARR3, 10, 7, 2);
	file.syncNumbers3D((const int *)MAKE_ITEM_ARR4, 2, 7, 2);
	file.syncNumbers2D((const int *)MAKE_ITEM_ARR5, 8, 2);
	file.syncNumbers(OUTDOOR_DRAWSTRUCT_INDEXES, 44);
	file.syncNumbers2D((const int *)TOWN_MAXES, 2, 11);
	file.syncStrings2D((const char *const *)TOWN_ACTION_MUSIC, 2, 7);
	file.syncStrings(TOWN_ACTION_SHAPES, 7);
	file.syncNumbers2D((const int *)TOWN_ACTION_FILES, 2, 7);
	file.syncString(BANK_TEXT);
	file.syncString(BLACKSMITH_TEXT);
	file.syncString(GUILD_NOT_MEMBER_TEXT);
	file.syncString(GUILD_TEXT);
	file.syncString(TAVERN_TEXT);
	file.syncString(GOOD_STUFF);
	file.syncString(HAVE_A_DRINK);
	file.syncString(YOURE_DRUNK);
	file.syncNumbers4D((const int *)TAVERN_EXIT_LIST, 2, 6, 5, 2);
	file.syncString(FOOD_AND_DRINK);
	file.syncString(TEMPLE_TEXT);
	file.syncString(EXPERIENCE_FOR_LEVEL);
	file.syncString(TRAINING_LEARNED_ALL);
	file.syncString(ELIGIBLE_FOR_LEVEL);
	file.syncString(TRAINING_TEXT);
	file.syncString(GOLD_GEMS);
	file.syncString(GOLD_GEMS_2);
	file.syncStrings(DEPOSIT_WITHDRAWL, 2);
	file.syncString(NOT_ENOUGH_X_IN_THE_Y);
	file.syncString(NO_X_IN_THE_Y);
	file.syncStrings(STAT_NAMES, 16);
	file.syncStrings(CONSUMABLE_NAMES, 4);
	file.syncStrings(WHERE_NAMES, 2);
	file.syncString(AMOUNT);
	file.syncString(FOOD_PACKS_FULL);
	file.syncString(BUY_SPELLS);
	file.syncString(GUILD_OPTIONS);
	file.syncNumbers((const int *)MISC_SPELL_INDEX, 74);
	file.syncNumbers((const int *)SPELL_COSTS, 77);
	file.syncNumbers2D((const int *)CLOUDS_GUILD_SPELLS, 5, 20);
	file.syncNumbers2D((const int *)DARK_SPELL_OFFSETS, 3, 39);
	file.syncNumbers2D((const int *)DARK_SPELL_RANGES, 12, 2);
	file.syncNumbers2D((const int *)SWORDS_SPELL_RANGES, 12, 2);
	file.syncNumbers((const int *)SPELL_GEM_COST, 77);
	file.syncString(NOT_A_SPELL_CASTER);
	file.syncString(SPELLS_LEARNED_ALL);
	file.syncString(SPELLS_FOR);
	file.syncString(SPELL_LINES_0_TO_9);
	file.syncString(SPELLS_DIALOG_SPELLS);
	file.syncString(SPELL_PTS);
	file.syncString(GOLD);
	file.syncString(SPELL_INFO);
	file.syncString(SPELL_PURCHASE);
	file.syncString(MAP_TEXT);
	file.syncString(LIGHT_COUNT_TEXT);
	file.syncString(FIRE_RESISTENCE_TEXT);
	file.syncString(ELECRICITY_RESISTENCE_TEXT);
	file.syncString(COLD_RESISTENCE_TEXT);
	file.syncString(POISON_RESISTENCE_TEXT);
	file.syncString(CLAIRVOYANCE_TEXT);
	file.syncString(LEVITATE_TEXT);
	file.syncString(WALK_ON_WATER_TEXT);
	file.syncString(GAME_INFORMATION);
	file.syncString(WORLD_GAME_TEXT);
	file.syncString(DARKSIDE_GAME_TEXT);
	file.syncString(CLOUDS_GAME_TEXT);
	file.syncString(SWORDS_GAME_TEXT);
	file.syncStrings(WEEK_DAY_STRINGS, 10);
	file.syncString(CHARACTER_DETAILS);
	file.syncString(PARTY_GOLD);
	file.syncString(PLUS_14);
	file.syncString(CHARACTER_TEMPLATE);
	file.syncString(EXCHANGING_IN_COMBAT);
	file.syncString(CURRENT_MAXIMUM_RATING_TEXT);
	file.syncString(CURRENT_MAXIMUM_TEXT);
	file.syncStrings(RATING_TEXT, 24);
	file.syncString(AGE_TEXT);
	file.syncString(LEVEL_TEXT);
	file.syncString(RESISTENCES_TEXT);
	file.syncString(NONE);
	file.syncString(EXPERIENCE_TEXT);
	file.syncString(ELIGIBLE);
	file.syncString(IN_PARTY_IN_BANK);
	file.syncString(FOOD_TEXT);
	file.syncString(EXCHANGE_WITH_WHOM);
	file.syncString(QUICK_REF_LINE);
	file.syncString(QUICK_REFERENCE);
	file.syncNumbers2D((const int *)BLACKSMITH_MAP_IDS, 2, 4);
	file.syncString(ITEMS_DIALOG_TEXT1);
	file.syncString(ITEMS_DIALOG_TEXT2);
	file.syncString(ITEMS_DIALOG_LINE1);
	file.syncString(ITEMS_DIALOG_LINE2);
	file.syncString(BTN_BUY);
	file.syncString(BTN_SELL);
	file.syncString(BTN_IDENTIFY);
	file.syncString(BTN_FIX);
	file.syncString(BTN_USE);
	file.syncString(BTN_EQUIP);
	file.syncString(BTN_REMOVE);
	file.syncString(BTN_DISCARD);
	file.syncString(BTN_QUEST);
	file.syncString(BTN_ENCHANT);
	file.syncString(BTN_RECHARGE);
	file.syncString(BTN_GOLD);
	file.syncString(ITEM_BROKEN);
	file.syncString(ITEM_CURSED);
	file.syncString(ITEM_OF);
	file.syncStrings(BONUS_NAMES, 7);
	file.syncStrings(WEAPON_NAMES, 41);
	file.syncStrings(ARMOR_NAMES, 14);
	file.syncStrings(ACCESSORY_NAMES, 11);
	file.syncStrings(MISC_NAMES, 22);
	file.syncStrings(SPECIAL_NAMES, 74);
	file.syncStrings(ELEMENTAL_NAMES, 6);
	file.syncStrings(ATTRIBUTE_NAMES, 10);
	file.syncStrings(EFFECTIVENESS_NAMES, 7);
	file.syncStrings(QUEST_ITEM_NAMES, 85);
	file.syncStrings(QUEST_ITEM_NAMES_SWORDS, 51);
	file.syncNumbers((const int *)WEAPON_BASE_COSTS, 35);
	file.syncNumbers((const int *)ARMOR_BASE_COSTS, 14);
	file.syncNumbers((const int *)ACCESSORY_BASE_COSTS, 11);
	file.syncNumbers((const int *)MISC_MATERIAL_COSTS, 22);
	file.syncNumbers((const int *)MISC_BASE_COSTS, 76);
	file.syncNumbers((const int *)METAL_BASE_MULTIPLIERS, 22);
	file.syncNumbers((const int *)ITEM_SKILL_DIVISORS, 4);
	file.syncNumbers((const int *)RESTRICTION_OFFSETS, 4);
	file.syncNumbers((const int *)ITEM_RESTRICTIONS, 86);
	file.syncString(NOT_PROFICIENT);
	file.syncString(NO_ITEMS_AVAILABLE);
	file.syncStrings(CATEGORY_NAMES, 4);
	file.syncString(X_FOR_THE_Y);
	file.syncString(X_FOR_Y);
	file.syncString(X_FOR_Y_GOLD);
	file.syncString(FMT_CHARGES);
	file.syncString(AVAILABLE_GOLD_COST);
	file.syncString(CHARGES);
	file.syncString(COST);
	file.syncStrings(ITEM_ACTIONS, 7);
	file.syncString(WHICH_ITEM);
	file.syncString(WHATS_YOUR_HURRY);
	file.syncString(USE_ITEM_IN_COMBAT);
	file.syncString(NO_SPECIAL_ABILITIES);
	file.syncString(CANT_CAST_WHILE_ENGAGED);
	file.syncString(EQUIPPED_ALL_YOU_CAN);
	file.syncString(REMOVE_X_TO_EQUIP_Y);
	file.syncString(RING);
	file.syncString(MEDAL);
	file.syncString(CANNOT_REMOVE_CURSED_ITEM);
	file.syncString(CANNOT_DISCARD_CURSED_ITEM);
	file.syncString(PERMANENTLY_DISCARD);
	file.syncString(BACKPACK_IS_FULL);
	file.syncStrings(CATEGORY_BACKPACK_IS_FULL, 4);
	file.syncString(BUY_X_FOR_Y_GOLD);
	file.syncString(SELL_X_FOR_Y_GOLD);
	file.syncString(NO_NEED_OF_THIS);
	file.syncString(NOT_RECHARGABLE);
	file.syncString(SPELL_FAILED);
	file.syncString(NOT_ENCHANTABLE);
	file.syncString(ITEM_NOT_BROKEN);
	file.syncStrings(FIX_IDENTIFY, 2);
	file.syncString(FIX_IDENTIFY_GOLD);
	file.syncString(IDENTIFY_ITEM_MSG);
	file.syncString(ITEM_DETAILS);
	file.syncString(ALL);
	file.syncString(FIELD_NONE);
	file.syncString(DAMAGE_X_TO_Y);
	file.syncString(ELEMENTAL_XY_DAMAGE);
	file.syncString(ATTR_XY_BONUS);
	file.syncString(EFFECTIVE_AGAINST);
	file.syncString(QUESTS_DIALOG_TEXT);
	file.syncString(CLOUDS_OF_XEEN_LINE);
	file.syncString(DARKSIDE_OF_XEEN_LINE);
	file.syncString(SWORDS_OF_XEEN_LINE);
	file.syncString(NO_QUEST_ITEMS);
	file.syncString(NO_CURRENT_QUESTS);
	file.syncString(NO_AUTO_NOTES);
	file.syncString(QUEST_ITEMS_DATA);
	file.syncString(CURRENT_QUESTS_DATA);
	file.syncString(AUTO_NOTES_DATA);
	file.syncString(REST_COMPLETE);
	file.syncString(PARTY_IS_STARVING);
	file.syncString(HIT_SPELL_POINTS_RESTORED);
	file.syncString(TOO_DANGEROUS_TO_REST);
	file.syncString(SOME_CHARS_MAY_DIE);
	file.syncString(DISMISS_WHOM);
	file.syncString(CANT_DISMISS_LAST_CHAR);
	file.syncString(DELETE_CHAR_WITH_ELDER_WEAPON);
	file.syncStrings(REMOVE_DELETE, 2);
	file.syncString(REMOVE_OR_DELETE_WHICH);
	file.syncString(YOUR_PARTY_IS_FULL);
	file.syncString(HAS_SLAYER_SWORD);
	file.syncString(SURE_TO_DELETE_CHAR);
	file.syncString(CREATE_CHAR_DETAILS);
	file.syncString(NEW_CHAR_STATS);
	file.syncString(NAME_FOR_NEW_CHARACTER);
	file.syncString(SELECT_CLASS_BEFORE_SAVING);
	file.syncString(EXCHANGE_ATTR_WITH);
	file.syncNumbers((const int *)NEW_CHAR_SKILLS, 10);
	file.syncNumbers((const int *)NEW_CHAR_SKILLS_OFFSET, 10);
	file.syncNumbers((const int *)NEW_CHAR_SKILLS_LEN, 10);
	file.syncNumbers((const int *)NEW_CHAR_RACE_SKILLS, 10);
	file.syncNumbers((const int *)RACE_MAGIC_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_FIRE_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_ELECTRIC_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_COLD_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_ENERGY_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_POISON_RESISTENCES, 5);
	file.syncNumbers2D((const int *)NEW_CHARACTER_SPELLS, 10, 4);
	file.syncString(COMBAT_DETAILS);
	file.syncString(NOT_ENOUGH_TO_CAST);
	file.syncStrings(SPELL_CAST_COMPONENTS, 2);
	file.syncString(CAST_SPELL_DETAILS);
	file.syncString(PARTY_FOUND);
	file.syncString(BACKPACKS_FULL_PRESS_KEY);
	file.syncString(HIT_A_KEY);
	file.syncString(GIVE_TREASURE_FORMATTING);
	file.syncString(X_FOUND_Y);
	file.syncString(ON_WHO);
	file.syncString(WHICH_ELEMENT1);
	file.syncString(WHICH_ELEMENT2);
	file.syncString(DETECT_MONSTERS);
	file.syncString(LLOYDS_BEACON);
	file.syncString(HOW_MANY_SQUARES);
	file.syncString(TOWN_PORTAL);
	file.syncString(TOWN_PORTAL_SWORDS);
	file.syncNumbers2D((const int *)TOWN_MAP_NUMBERS, 3, 5);
	file.syncString(MONSTER_DETAILS);
	file.syncStrings(MONSTER_SPECIAL_ATTACKS, 23);
	file.syncString(IDENTIFY_MONSTERS);
	file.syncStrings(EVENT_SAMPLES, 6);
	file.syncString(MOONS_NOT_ALIGNED);
	file.syncString(AWARDS_FOR);
	file.syncString(AWARDS_TEXT);
	file.syncString(NO_AWARDS);
	file.syncString(WARZONE_BATTLE_MASTER);
	file.syncString(WARZONE_MAXED);
	file.syncString(WARZONE_LEVEL);
	file.syncString(WARZONE_HOW_MANY);
	file.syncString(PICKS_THE_LOCK);
	file.syncString(UNABLE_TO_PICK_LOCK);
	file.syncString(CONTROL_PANEL_TEXT);
	file.syncString(CONTROL_PANEL_BUTTONS);
	file.syncString(ON);
	file.syncString(OFF);
	file.syncString(CONFIRM_QUIT);
	file.syncString(MR_WIZARD);
	file.syncString(NO_LOADING_IN_COMBAT);
	file.syncString(NO_SAVING_IN_COMBAT);
	file.syncString(QUICK_FIGHT_TEXT);
	file.syncStrings(QUICK_FIGHT_OPTIONS, 4);
	file.syncStrings(WORLD_END_TEXT, 9);
	file.syncString(WORLD_CONGRATULATIONS);
	file.syncString(WORLD_CONGRATULATIONS2);
	file.syncString(CLOUDS_CONGRATULATIONS1);
	file.syncString(CLOUDS_CONGRATULATIONS2);
	file.syncStrings(GOOBER, 3);
	file.syncStrings(MUSIC_FILES1, 5);
	file.syncStrings2D((const char *const *)MUSIC_FILES2, 6, 7);
	file.syncString(DIFFICULTY_TEXT);
	file.syncString(SAVE_OFF_LIMITS);
	file.syncString(CLOUDS_INTRO1);
	file.syncString(DARKSIDE_ENDING1);
	file.syncString(DARKSIDE_ENDING2);
	file.syncString(PHAROAH_ENDING_TEXT1);
	file.syncString(PHAROAH_ENDING_TEXT2);

	cc.add("CONSTANTS", file);
}
