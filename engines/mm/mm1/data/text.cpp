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

#include "mm/mm1/data/text.h"

namespace MM {
namespace MM1 {

const char *const CLASS_NAMES[6] = {
	"Knight",
	"Paladin",
	"Archer",
	"Cleric",
	"Sorcerer",
	"Robber"
};

const char *const RACE_NAMES[5] = {
	"Human",
	"Elf",
	"Dwarf",
	"Gnome",
	"H-Orc"
};

const char *const ALIGNMENT_NAMES[3] = {
	"Good",
	"Neut",
	"Evil"
};

const char *const SEX_NAMES[3] = {
	"Male",
	"Female",
	"Yes Please"
};

const char *const TOWN_NAMES[5] = {
	"Sorpigal",
	"Portsmith",
	"Algary",
	"Dusk",
	"Erliquin"
};

const char *const COLOR_NAMES[8] = {
	"Red Thorac",
	"Blue Ogram",
	"Green Bagar",
	"Yellow Limra",
	"Purple Sagran",
	"Orange Oolak",
	"Black Dresidion",
	"White Dilithium"
};

const char *const CLERIC_SPELL_NAMES[47] = {
	"Awaken",
	"Bless",
	"Blind",
	"First Aid",
	"Light",
	"Power Cure",
	"Prot. From Fear",
	"Turn Undead",
	"Cure Wounds",
	"Heroism",
	"Pain",
	"Prot. From Cold",
	"Prot. From Ice",
	"Prot. From Poison",
	"Silence",
	"Suggestion",
	"Create Food",
	"Cure Blindness",
	"Cure Paralysis",
	"Lasting Light",
	"Produce Flame",
	"Produce Frost",
	"Remove Quest",
	"Walk on Water",
	"Cure Disease",
	"Neutralize Poison",
	"Prot. From Acid",
	"Prot. From Electricity",
	"Restore Alignment",
	"Summon Lightning",
	"Super Heroism",
	"Surface",
	"Deadly Swarm",
	"Dispel Magic",
	"Paralyze",
	"Remove Condition",
	"Restore Energy",
	"Moon Ray",
	"Raise Dead",
	"Rejuvinate",
	"Stone to Flesh",
	"Town Portal",
	"Divine Intervention",
	"Holy Word",
	"Prot. From Elements",
	"Ressurection",
	"Sun Ray"
};

const char *const WIZARD_SPELL_NAMES[47] = {
	"Awaken",
	"Detect Magic",
	"Energy Blast",
	"Flame Arrow",
	"Leather Skin",
	"Light",
	"Location",
	"Sleep",
	"Electric Arrow",
	"Hypnotize",
	"identify Monster",
	"Jump",
	"Levitate",
	"Power",
	"Quickness",
	"Scare",
	"Fireball",
	"Fly",
	"Invisibility",
	"Lightning Bolt",
	"Make Room",
	"Slow",
	"Weaken",
	"Web",
	"Acid Arrow",
	"Cold Beam",
	"Feeble Mind",
	"Freeze",
	"Guard Dog",
	"Psychic Protection",
	"Shield",
	"Time Distortion",
	"Acid Rain",
	"Dispel Magic",
	"Finger of Death",
	"Shelter",
	"Teleport",
	"Dancing Sword",
	"Disintegration",
	"Etherialize",
	"Prot. From Magic",
	"Recharge Item",
	"Astral Spell",
	"Duplication",
	"Meteor Shower",
	"Power Shield",
	"Prismatic Light"
};

const char *const MONSTER_EFFECTS[9] = {
	"is scared",
	"is blinded",
	"is silenced",
	"is mindless",
	"is slept",
	"is held",
	"is webbed",
	"is paralyzed",
	"is affected"
};

const char *const CHAR_EFFECTS[11] = {
	"is slept",
	"is blinded",
	"is silenced",
	"is diseased",
	"is poisoned",
	"is paralyzed",
	"is rendered unconscious",
	"is killed",
	"is turned to stone",
	"is eradicated",
	"is affected"
};

const char *const LOCATION_TITLES[5] = {
	"--training--",
	"--market--",
	"--temple--",
	"--blacksmith--",
	"--tavern--"
};

const char *const OBSTRUCTED_MESSAGES[6] = {
	"   solid!   ",
	"   locked!  ",
	" too dense! ",
	" impassable ",
	" rough seas ",
	" too windy! "
};

const char *const COMBAT_STATUS[9] = {
	"(paralyze)",
	"(webbed)  ",
	"(held)    ",
	"(asleep)  ",
	"(mindless)",
	"(silenced)",
	"(blinded) ",
	"(afraid)  ",
	"(dead)    "
};

const char *const ATTACK_TYPES[15] = {
	"attacks",
	"fights",
	"charges",
	"assaults",
	"battles",
	"stabs at",
	"flails at",
	"lunges at",
	"swings at",
	"chops at",
	"hacks at",
	"thrusts at",
	"slashes at",
	"strikes at",
	"thrashes at"
};

const char *const GAME_COMMANDS[16] = {
	" commands",
	" ",
	"[ forward",
	"\\ back",
	"] turn",
	"   left",
	"^ turn",
	"   right",
	"o order",
	"p protect",
	"r rest",
	"s search",
	"b bash",
	"u unlock",
	"q quikref",
	"# view ch"
};

const char *const PROTECTION_NAMES[18] = {
	"Fear",
	"Cold",
	"Fire",
	"Poison",
	"Acid",
	"Elec",
	"Magic",
	"Light (",
	"Leather Skin",
	"Levitate",
	"Walk on Water",
	"Guard Dog",
	"Psychic Protection",
	"Bless",
	"Invisibility",
	"Shield",
	"Power Shield",
	"*** Cursed -"
};

const char *const SEARCH_CONTAINERS[11] = {
	" Cloth Sack ",
	"Leather Sack",
	" Wooden Box ",
	"Wooden Chest",
	"  Iron Box  ",
	" Iron Chest ",
	" Silver Box ",
	"Silver Chest",
	"  Gold Box  ",
	" Gold Chest ",
	" Black Box  "
};

const char *const STATUE_NAMES[8] = {
	"a human knight",
	"an elven wizard",
	"a gnome robber",
	"a dwarf paladin\npainted a black & white checkered motif",
	"a h-orc archer",
	"a human cleric",
	"a blue dragon",
	"a gray minotaur"
};

const char *const STATUE_MESSAGES[8] = {
	"Services rendered, secrets unfold\nthe brothers together\nlead to treasures untold\n\nfive towns you must travel\nfor this quest to unravel",
	"Seek the wizard Ranalou\nin his lair at the Korin Bluffs\nsix castles he will send you to\nbut Doom will be quite tough!\ncruelty and kindness\nmeasured throughout\njudgement day is then sought out",
	"One by water, one by land\none by air, and one by sand\nthe wheel of luck\nwill favorably pay, the more of these\nmenacing beasts you slay!\nAlthough wishes may come true\nall the beasts will become anew",
	"There are many dungeons like me\nfind the right pair\nand you'll discover the key\nthe ancient seer Og has lost his sight\nthe idols will help to end his plight",
	"In honor of Corak...\n\nfor his mapping expedition\nof the land of Varn and rediscovery\nof the lost town of Dusk",
	"In honor of Gala...\n\nfor her brave attempt to work with the\nsavages of the Volcanic Isles",
	"In memory of a time long ago...\n\nbefore the days when the towns\nmoved underground, dragons were\nfew and far between",
	"This beast once roamed the\nEnchanted Forest and now rules\na great fortress there"
};

const char *const TAVERN_RUMORS[16] = {
	"All portals are connected",
	"Sorpigal has 8 statues",
	"The brothers live by docks",
	"Some caves have more than one exit",
	"The ice princess has the key",
	"The desert has many oases",
	"The swamp was once a city of gold",
	"Portsmith and men don't mix",
	"Most towns have caverns below",
	"Varn is not what it appears to be",
	"Kilburn is near the wyvern peaks",
	"A dragons breath is only as strong\x8b""as it's condition",
	"The king is in seclusion",
	"The inner sanctum is a myth",
	"The canine has the key",
	"ScummVM is totally awesome!"
};

const char *const INGREDIENTS[7] = {
	"garlic",
	"wolfsbane",
	"belladonna",
	"the head of a medusa",
	"an eye of a wyvern",
	"a dragons tooth",
	"the ring of Okrim"
};

const char *const INSPECTRON_QUESTS[7] = {
	"Find the ancient ruins in the\nQuivering Forest",
	"Visit Blithes Peak, and report",
	"The people of the desert have much to\ntrade,bring me a sample of their goods",
	"Find the Shrine of Okzar in the\ncaves below Dusk",
	"Find the Fabled Fountain in Dragadune",
	"Solve the Riddle of the Ruby",
	"Defeat the stronghold\nin the Enchanted Forest"
};

const char *const IRONFIST_QUESTS[7] = {
	"Find the stronghold in Ravens Wood",
	"Find Lord Kilburn",
	"Discover the secret of Portsmith",
	"Find the pirates secret cove",
	"Find the shipwreck of the Jolly Raven",
	"Defeat the Pirate Ghost Ship Anarchist",
	"Defeat the stronghold in Ravens Wood"
};

const char *const TRIVIA_QUESTIONS[5] = {
	"Who rules Castle W.W.?",
	"Who is the voluptuous one?",
	"Who's lost sight?",
	"Where's the very latest?",
	"Who be ye?"
};

const char *const TRIVIA_ANSWERS[5] = {
	"Lord Ironfist",
	"Laura",
	"Og",
	"Erliquin",
	"I be me"
};

const char *const TAVERN_TIPS[5][4] = {
	{
		"See man in cave below (1,2)",
		"Check walls near (12,3)",
		"Statue at (2,4) is your first job",
		"Similar pieces of a puzzle may not\x8d""belong to the same puzzle"
	},
	{
		"Seek quests behind moons",
		"The queen can be helpful",
		"The beasts are on the map",
		"Islands like colors are unknown\x8duntil discovered"
	},
	{
		"Attacks should be concentrated",
		"Due north is the cave of square magic",
		"Secret doors are abundant",
		"The demon and the maze are one"
	},
	{
		"Telgoran is in s.e. maze",
		"Some items give protection",
		"Dragadune holds many gems",
		"Donations help on judgement day"
	},
	{
		"Agar lives behind the inn",
		"Find the prisoner",
		"Barriers guard wealth",
		"The magic total is 34"
	}
};

} // namespace MM1
} // namespace MM
