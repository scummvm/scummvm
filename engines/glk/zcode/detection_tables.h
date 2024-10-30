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

#include "engines/game.h"
#include "common/language.h"

namespace Glk {
namespace ZCode {

/**
 * Game descriptor for ZCode games
 */
struct FrotzGameDescription {
	const char *const _gameId;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
	const char *const _guiOptions;
};

/**
 * Original games from Infocom
 */
const PlainGameDescriptor INFOCOM_GAME_LIST[] = {
	// Infocom games
	{ "abyss",              "The Abyss" },
	{ "amfv",               "A Mind Forever Voyaging" },
	{ "ballyhoo",           "Ballyhoo" },
	{ "beyondzork",         "Beyond Zork" },
	{ "borderzone",         "Border Zone" },
	{ "bureaucracy",        "Bureaucracy" },
	{ "cutthroats",         "Cutthroats" },
	{ "deadline",           "Deadline" },
	{ "enchanter",          "Enchanter" },
	{ "genericinfocom",     "Infocom Template" },
	{ "hhgttg",             "The Hitchhiker's Guide to the Galaxy" },
	{ "hollywoodhijinx",    "Hollywood Hijinx" },
	{ "hypochondriac",      "Hypochondriac" },
	{ "infidel",            "Infidel" },
	{ "infocomsample1b",    "Infocom Sampler 1B" },
	{ "infocomsample1z",    "Infocom Sampler 1Z" },
	{ "infocomsample2zt",   "Infocom Sampler 2ZT" },
	{ "infocomsample3zpi",  "Infocom Sampler 3ZPI" },
	{ "infocomsample3ztl",  "Infocom Sampler 3ZTL" },
	{ "infocomsample4zpiw", "Infocom Sampler 4ZPIW" },
	{ "journey",            "Journey" },
	{ "lgop",               "Leather Goddesses of Phobos" },
	{ "lurkinghorror",      "The Lurking Horror" },
	{ "milliways",          "Milliways" },
	{ "minizork1",          "Mini Zork I: The Great Underground Empire" },
	{ "minizork2",          "Mini Zork II: The Wizrd of Frobozz" },
	{ "moonmist",           "Moonmist" },
	{ "nordbert",           "Nord and Bert Couldn't Make Head or Tail of It" },
	{ "planetfall",         "Planetfall" },
	{ "plunderedhearts",    "Plundered Hearts" },
	{ "questforexcalibur",  "Arthur: The Quest for Excalibur" },
	{ "restaurant",         "Restaurant at the End of the Universe" },
	{ "seastalker",         "Seastalker" },
	{ "sherlockriddle",     "Sherlock: The Riddle of the Crown Jewels" },
	{ "shogun",             "James Clavell's Shogun" },
	{ "sorcerer",           "Sorcerer" },
	{ "spellbreaker",       "Spellbreaker" },
	{ "starcross",          "Starcross" },
	{ "stationfall",        "Stationfall" },
	{ "suspect",            "Suspect" },
	{ "suspended",          "Suspended" },
	{ "thewitness",         "The Witness" },
	{ "trinity",            "Trinity" },
	{ "wishbringer",        "Wishbringer" },
	{ "ziptest",            "ZipTest" },
	{ "zork0",              "Zork Zero: The Revenge of Megaboz" },
	{ "zork1",              "Zork I: The Great Underground Empire" },
	{ "zork2",              "Zork II: The Wizard of Frobozz" },
	{ "zork3",              "Zork III: The Dungeon Master" },
	{ "ztuu",               "Zork: The Undiscovered Underground" },

	{ nullptr, nullptr }
};

/**
 * All the other subsequent non-Infocom games using the format
 */
const PlainGameDescriptor ZCODE_GAME_LIST[] = {
	{ "zcode",              "Unknown Z-code game" },

	// English games
	{ "404life",            "404 - Life Not Found" },
	{ "43",                 "43" },
	{ "69105keys",          "69,105 Keys" },
	{ "7doctors",           "The Seven Doctors" },
	{ "7kids",              "The Wolf and the Seven Kids" },
	{ "905",                "9:05" },
	{ "9dancers",           "The Nine Dancers" },
	{ "aasmasters",         "AAS Masters" },
	{ "aaaaaaaa",           "A AAAAA AAAAAAAAA" },
	{ "accuse",             "Accuse" },
	{ "acheton",            "Acheton" },
	{ "acorncourt",         "The Acorn Court" },
	{ "acrobat",            "The Mysterious Case of the Acrobat and His Peers" },
	{ "acrossstars",        "Across the Stars" },
	{ "acrossstarsclues",   "Across the Stars: InvisiClues" },
	{ "acrossstarshints",   "Across the Stars: InvisiHints" },
	{ "aciftotorial",       "Adam Cadre’s IF Tutorial" },
	{ "addendum",           "Flawed Addendum" },
	{ "adv",                "Adventure, Colossal Cave" },
	{ "adv350_zcode",       "Adventure, 350 Point Colossal Cave" },
	{ "adv440",             "Adventure II, 440 Point Colossal Cave" },
	{ "adv550",             "Adventure 3, 550 Point Colossal Cave" },
	{ "adv551",             "Adventure 6, 551 Point Colossal Cave" },
	{ "adventland",         "Adventureland" },
	{ "adventurelobjan",    "Adventure (Lobjan translation)" },
	{ "adventuretime",      "Adventure Time" },
	{ "adverbum",           "Ad Verbum" },
	{ "advhoudini",         "The Adventures of Houdini" },
	{ "affront",            "Annoyotron IV: Affrontotron" },
	{ "agreydayforalbert",  "A Grey Day for Albert" },
	{ "aisle",              "Aisle" },
	{ "alienextraction",    "Alien Extraction" },
	{ "alice",              "Alice Through the Looking Glass" },
	{ "alongtheriver",      "Along the River" },
	{ "alpha",              "Journey to Alpha Centauri (In Real Time)" },
	{ "amazingmaze",        "The Amazing Maze" },
	{ "ambassadorsdaught",  "The Ambassador Daughter" },
	{ "amish",              "Amishville" },
	{ "amiss",              "Amissville" },
	{ "anchor",             "Anchorhead" },
	{ "andrewplotkin",      "Being Andrew Plotkin" },
	{ "andromedagenesis",   "Andromeda Genesis" },
	{ "anotherterminal",    "Another Terminal Beach" },
	{ "animals",            "Animals" },
	{ "annoy",              "Annoyotron" },
	{ "aotyrz",             "Attack of the Yeti Robot Zombies" },
	{ "aphasiaquest",       "Aphasia Quest" },
	{ "apollo11b",          "Apollo 11 (by B)" },
	{ "arc3h",              "Alien Research Centre 3: Footprints in the Snow" },
	{ "argument",           "The Argument-Winner's Ghost" },
	{ "aridandpale",        "Arid and Pale" },
	{ "asylum_cg",          "Asylum (by Cpuguy)" },
	{ "atrocitron",         "Atrocitron" },
	{ "ats",                "A Tight Spot" },
	{ "atwork",             "Danger! Adventurer at Work!" },
	{ "aug4",               "Augmented Fourth" },
	{ "avon",               "Avon" },
	{ "awakening",          "The Awakening" },
	{ "awakeningpg",        "Awakening (by Pete Gardner)" },
	{ "awitl",              "A Week in the Life" },
	{ "ayac",               "Are You a Chef?" },
	{ "azteca",             "Azteca" },
	{ "b7snare",            "Snare" },
	{ "babysitter",         "The Babysitter" },
	{ "babytree",           "Baby Tree" },
	{ "backtowakeup",       "Back to WakeUp" },
	{ "backup",             "Backup" },
	{ "badguys",            "Bad Guys" },
	{ "bakingwithlizzie",   "Baking With Lizzie" },
	{ "balances_zcode",     "Balances" },
	{ "baldersdeath",       "Balder's Death" },
	{ "ballerina102",       "Not Just an Ordinary Ballerina" },
	{ "balt24",             "Baltimore:24" },
	{ "barber",             "The Barber of Sadville" },
	{ "bathtub",            "There's a Snake in the Bathtub" },
	{ "bazic",              "baZic version 0.1" },
	{ "bcd9",               "Behind Closed Doors 9B" },
	{ "beanstalker",        "The Bean Stalker" },
	{ "bear",               "A Bear's Night Out" },
	{ "bedlam_mz",          "Bedlam (by Michael Zey)" },
	{ "bedtime",            "Bed Time" },
	{ "beginning",          "A Beginning" },
	{ "beingsteve",         "Being Steve" },
	{ "beneathtransform",   "BENEATH: A Transformation" },
	{ "beyond_zcode",       "Beyond" },
	{ "bicon",              "BiCon" },
	{ "bignose",            "Big Nose on the Big Pyramid" },
	{ "biscuit",            "Biscuit" },
	{ "bj",                 "Blow Job Drifter" },
	{ "blair",              "Tales From the College Presents a Breath of Fresh Blair" },
	{ "bomber",             "The Mad Bomber" },
	{ "bonesaw",            "The Unstoppable Vengeance of Doctor Bonesaw" },
	{ "bookvol",            "Book and Volume" },
	{ "booth",              "Pick up the Phone Booth and Die" },
	{ "boothdye",           "Pick up the Phone Booth and Dye" },
	{ "boxes",              "Boxes" },
	{ "brandx",             "BrandX" },
	{ "breakin",            "Break-in" },
	{ "briantimmons",       "The Surprising Case of Brian Timmons" },
	{ "bronze",             "Bronze" },
	{ "bryantcollection",   "The Bryant Collection" },
	{ "building",           "Building" },
	{ "bureaucrocy_zcode",  "Bureaucrocy" },
	{ "burglar",            "Burglar!" },
	{ "burnkoran",          "Burn The Koran and Die" },
	{ "burnsnightsupper",   "Burns Night Supper" },
	{ "busted",             "Busted" },
	{ "byod",               "BYOD" },
	{ "cabal",              "The Cabal" },
	{ "cacharensdilemma",   "Cacharens Dilemma" },
	{ "cacophony",          "Cacophony" },
	{ "calendar",           "Calendar, An Inform 7 Abuse" },
	{ "calmmutemoving",     "Calm, Mute, Moving" },
	{ "calypso",            "Calypso" },
	{ "candy",              "Candy" },
	{ "capturesanta",       "Capture Santa!" },
	{ "carpathianvampire",  "Carpathian Vampire" },
	{ "cars",               "ASCII Cars!!!" },
	{ "casting",            "Casting" },
	{ "castleredprince",    "Castle of the Red Prince" },
	{ "catcherintherye",    "Catcher in the Rye" },
	{ "catseye",            "Miniventure #2: Cat's Eye" },
	{ "causality",          "Causality" },
	{ "caveadventure",      "Cave Adventure" },
	{ "cavernofdoom",       "Zork: The Cavern of Doom" },
	{ "cavernsofchaos",     "Caverns of Chaos" },
	{ "cavetrip",           "The Spelunking Trip" },
	{ "ccake",              "Arthur Yahtzee: The Curse of Hell's Cheesecake" },
	{ "cco",                "Construction Cancellation Order" },
	{ "charactercreation",  "Character Creator" },
	{ "chaos",              "Chaos" },
	{ "chaosgame",          "Chaos (by Bloodbath)" },
	{ "cheater",            "Cheater" },
	{ "checkerhaunt",       "A Checkered Haunting" },
	{ "cheesedoff_zcode",   "Cheesed Off!" },
	{ "cheeseshop",         "Cheeseshop" },
	{ "chengara",           "Chengara" },
	{ "cheshirecat_zcode",  "Save the Cheshire Cat!" },
	{ "chico",              "Chico and I Ran" },
	{ "childsplay",         "Child’s Play" },
	{ "chix",               "Chicks Dig Jerks" },
	{ "christminster",      "Christminster" },
	{ "cia",                "C.I.A. Adventure" },
	{ "classchallenge",     "Class Challenge" },
	{ "classroom",          "The Classroom Done" },
	{ "cleanair",           "Clean Air" },
	{ "cliffedge",          "Edge of the Cliff" },
	{ "cockandbull",        "A Cock and Bull Story" },
	{ "clockwork",          "A Clockwork Noir" },
	{ "codenamesilver",     "Code Name Silver Steel" },
	{ "cogs",               "The Cogs of Westminster" },
	{ "cointoss",           "Coin Toss" },
	{ "coke",               "Coke Is It!" },
	{ "colonists",          "Colonists" },
	{ "colmc_zcode",        "Color of Milk Coffee" },
	{ "coloursexploration", "An Exploration of Colour" },
	{ "conankill",          "Conan Kill Everything" },
	{ "conceptisproven",    "My Little Project Concept is Proven" },
	{ "cottage",            "Cottage" },
	{ "countdown1",         "Countdown 1 - The Body" },
	{ "countdown2",         "Countdown 2 - The Soul" },
	{ "countdown3",         "Countdown 3 - The Mind" },
	{ "crabhat",            "Oh No, Mr Crab Stole Your Hat!" },
	{ "cragnemanor_zcode",  "Cragne Manor" },
	{ "crazydiaper",        "The Tale of the Crazy Diaper Man" },
	{ "craverlyheights",    "Craverly Heights" },
	{ "crimescene",         "A Crime Scene" },
	{ "critbreach",         "Critical Breach" },
	{ "criticalhit",        "Critical Hit" },
	{ "crobe",              "Crobe" },
	{ "cryptographer",      "Cryptographer" },
	{ "csbb",               "Crystal and Stone Beetle and Bone" },
	{ "ctdoom",             "Countdown to Doom" },
	{ "cubicle",            "In the Cubicle" },
	{ "curses",             "Curses" },
	{ "curves",             "Dangerous Curves" },
	{ "cyclops",            "The Land of the Cyclops" },
	{ "damnmemoriae_zcode", "Damnatio Memoriae" },
	{ "dancebeat",          "Dance to the Beat of the Earth" },
	{ "darkiss1_zcode",     "Darkiss! Wrath of the Vampire - Chapter 1: The Awakening" },
	{ "darkiss2_zcode",     "Darkiss! Wrath of the Vampire - Chapter 2: Journey to Hell" },
	{ "dashslapney",        "Dash Slapney, Patrol Leader" },
	{ "daycreepylife",      "A Day in the Creepy Life of Bob" },
	{ "dayinlife",          "A Day in Life" },
	{ "dayishothitler",     "The Day I Shot Hitler" },
	{ "dd4",                "Dutch Dapper IV: The Final Voyage" },
	{ "dday",               "D-Day" },
	{ "deadmen",            "Down Among the Dead Men" },
	{ "deadmeat",           "Dead Meat in the Pit" },
	{ "deadreckon_zcode",   "Dead Reckoning" },
	{ "death",              "Death to my Enemies" },
	{ "debate",             "Debate" },
	{ "deephome",           "Deephome" },
	{ "deformuseum",        "Deformuseum" },
	{ "degeneracy",         "Degeneracy" },
	{ "dejavugn",           "Deja Vu (by Graham Nelson)" },
	{ "deliciousbreakfast", "Delicious Breakfast" },
	{ "desert_zcode",       "Desert Adventure" },
	{ "detective",          "Detective" },
	{ "detention",          "Detention" },
	{ "dewdrops",           "Within a Wreath of Dewdrops" },
	{ "dogshow",            "Dog Show" },
	{ "dogslife",           "It's a Dog's Life" },
	{ "dontgo_zcode",       "Don't Go" },
	{ "dontpeeyourself",    "Don't Pee Yourself!" },
	{ "dontpushthemailbox", "Don't Push The Mailbox" },
	{ "dorm",               "Dorm" },
	{ "dotd",               "Dawn of the Demon" },
	{ "downthematrix",      "Down The Matrix" },
	{ "downtowntokyo",      "Downtown Tokyo Present Day" },
	{ "dpod",               "Dracula - Prince of Darkness" },
	{ "dracula1_zcode",     "Dracula: Part 1, The First Night" },
	{ "dracula2_zcode",     "Dracula: Part 2, The Arrival" },
	{ "dragon_zcode",       "Dragon Adventure" },
	{ "dragontroll",        "The Dragon and the Troll" },
	{ "dramaclass",         "Drama Class" },
	{ "dreamhold",          "The Dreamhold" },
	{ "dreamtooreal",       "A Dream Too Real" },
	{ "dreamtrap",          "The Dream-Trap of Zzar" },
	{ "dumont",             "Dr. Dumont's Wild P.A.R.T.I" },
	{ "egcpc",              "EGC Paper Chase" },
	{ "eggstraordadv",      "Eggstraordinary Adventure" },
	{ "electricpuppet",     "The Electric Puppet" },
	{ "elephants1",         "When I Was Shot By Elephants" },
	{ "elephants2",         "When I Was Shot By Elephants 2: Super Elephants" },
	{ "elephants3",         "When I Was Shot By Elephants 3" },
	{ "elephantsdecision",  "When I Was Shot By Elephants Conclusion: The Decision" },
	{ "elephantsde",        "When I Was Shot By Elephants Definitive Edition" },
	{ "elfenmaiden",        "The Elfen Maiden: A Comedy of Error Messages" },
	{ "eliza",              "Eliza" },
	{ "emptyroom",          "The Empty Room" },
	{ "endofearth",         "The End of Earth" },
	{ "enemies",            "Enemies" },
	{ "enigma_sd",          "Enigma (by Simon Deimel)" },
	{ "enterprise",         "The Enterprise Incidents" },
	{ "entropy",            "Entropy" },
	{ "epyk",               "Eypk" },
	{ "eric",               "Eric The Power-Mad Dungeon Master" },
	{ "escapade",           "Escapade!" },
	{ "escape",             "Escape!" },
	{ "escaperemember",     "An Escape to Remember" },
	{ "escapetrollcave",    "Escape From the Troll's Cave" },
	{ "eto",                "ETO" },
	{ "everydaysamedream",  "Every Day the Same Dream" },
	{ "f209",               "Apartment 209" },
	{ "fable",              "A Fable" },
	{ "faculty",            "The Care and Feeding of Adjuncts" },
	{ "fade",               "Fade to Black" },
	{ "failsafe",           "FailSafe" },
	{ "fairyland",          "Fairyland" },
	{ "fajfeta",            "Fajfeta Sur la Luno" },
	{ "fallingpieces",      "Falling to Pieces" },
	{ "farm",               "The Farmer's Daughter" },
	{ "fifthcontinent",     "The Fifth Continent" },
	{ "figaro",             "Figaro" },
	{ "figueres",           "Figueres in My Basement" },
	{ "findesick",          "Fin de Sickleburg" },
	{ "findthemouse",       "Finding the Mouse" },
	{ "findthebed",         "Find the Bed" },
	{ "fingertipsfriend",   "Fingertips: I Found a New Friend" },
	{ "fingertipsmilk",     "Fingertips: Please Pass the Milk Please" },
	{ "firstdaymnl",        "The First Day of My New Life" },
	{ "flat",               "Flat" },
	{ "flowers",            "Flowers for Algernon" },
	{ "fmvpoker",           "Frobozz Magic Video Poker" },
	{ "forest",             "Forest" },
	{ "forgottengirls",     "The Forgotten Girls" },
	{ "forgottenpyramid",   "The Forgotten Pyramid" },
	{ "fork",               "Fork: The Great Underground Dining Room" },
	{ "fractalz",           "Fractalz" },
	{ "fracturemeta",       "Fractured Metamorphoses" },
	{ "fragileshells",      "Fragile Shells" },
	{ "frankie",            "Frankenstein Adventure" },
	{ "freefall",           "Free Fall" },
	{ "fridgetopia",        "Fridgetopia" },
	{ "frobozzi",           "The Encyclopedia Frobozzica (Abridged Edition)" },
	{ "frozen",             "Frozen: A Night at the Lab" },
	{ "fugue",              "Fugue" },
	{ "fyleet",             "Fyleet" },
	{ "ga",                 "Geocaching Adventure" },
	{ "galatea",            "Galatea" },
	{ "gamer",              "Gamer" },
	{ "gardening",          "Gardening for Beginners" },
	{ "garliccage",         "The Garlic Cage" },
	{ "gaucho",             "Gaucho" },
	{ "gbvb",               "Grandma Bethlinda's Variety Box" },
	{ "gd",                 "Goodbye Doggy" },
	{ "geist",              "Geist" },
	{ "gerbilriot",         "Gerbil Riot" },
	{ "ghandi",             "The Day I Hugged Ghandi" },
	{ "ghostsblackwood",    "The Ghosts of Blackwood Manor" },
	{ "ghosttrain",         "The Ghost Train" },
	{ "ghostmountain",      "Ghost Mountain" },
	{ "glass_zcode",        "Glass" },
	{ "glik1",              "Glik I" },
	{ "gnuzoo",             "Gnu in the Zoo" },
	{ "godot",              "Looking for Godot" },
	{ "goldilocks",         "Goldilocks is a Fox!" },
	{ "gourmetgaffe",       "Gourmet Gaffe" },
	{ "gowest",             "Go West" },
	{ "greaterthan",        "> by @" },
	{ "greatpancake",       "The Great Pancake Detectives - Case #27" },
	{ "greenblood",         "Green Blood" },
	{ "greenrain",          "A Green Rain" },
	{ "greensboro",         "Greensboro Sit-In" },
	{ "growingup",          "Growing Up" },
	{ "grumpytck_zcode",    "The Grumpy Cricket (And Other Enormous Creatures)" },
	{ "guestreet",          "Life on Gue Street" },
	{ "gumshoe",            "Gumshoe" },
	{ "gussdeath",          "Guss's Death" },
	{ "hadeanlands_zcode",  "Hadean Lands" },
	{ "halloweve",          "Hallow Eve" },
	{ "hamhouse",           "In the House of Professor Evil: The HAM HOUSE" },
	{ "hamil",              "Hamil" },
	{ "hangar22",           "Hangar 22" },
	{ "hangmansgulch",      "Hangman's Gulch" },
	{ "happyeverafter",     "Happy Ever After" },
	{ "hauntedhouse",       "Haunted House" },
	{ "hauntings",          "Hauntings" },
	{ "headcase",           "Head Case" },
	{ "heartice",           "Heart of Ice" },
	{ "heidi",              "Heidi" },
	{ "heist",              "Heist: The Crime of the Century" },
	{ "heliopause",         "Hoist Sail for the Heliopause and Home" },
	{ "hellsbasement",      "Hell's Basement" },
	{ "henribeauchamp",     "The Gallery of Henri Beauchamp" },
	{ "heroinesmantle",     "Heroine's Mantle" },
	{ "hibernated1",        "Hibernated 1 - This Place is Death (Director's Cut)" },
	{ "hiddennazi",         "The Game Formerly Known as Hidden Nazi Mode" },
	{ "hiddenverbiage",     "Hidden Verbiage" },
	{ "hideseek",           "Hide Seek" },
	{ "hidepachyderm",      "Hide a Pachyderm!" },
	{ "hippoelmstr_zcode",  "Hippo on Elm Street" },
	{ "hipponewyear",       "And a Hippo New Year" },
	{ "hlainform",          "HLA Adventure" },
	{ "hobbitredux_zcode",  "The Hobbit - The True Story - Redux, Director's Cut" },
	{ "homecoming",         "Homecoming" },
	{ "home",               "Home" },
	{ "hoosegow",           "Hoosegow" },
	{ "horatio",            "Horatio’s Story" },
	{ "hors",               "Hors Catégorie" },
	{ "house",              "The House" },
	{ "housedream",         "House of Dream of Moon" },
	{ "housekey1",          "Housekey, Part I" },
	{ "houseoffear",        "The House of Fear" },
	{ "humongouscave",      "Adventure in Humongous Cave" },
	{ "humongouscavehints", "Humongous Cave Hints" },
	{ "hyperrpg",           "Hyper RPG Game!" },
	{ "i0",                 "I-0" },
	{ "ibo",                "Ibo" },
	{ "icebreaker",         "Icebreaker" },
	{ "iceprincess",        "The Ice Princess" },
	{ "iceweb",             "Iceweb" },
	{ "ifaquarium",         "IF Aquarium" },
	{ "ifplayer",           "Interactive Fiction Player" },
	{ "ifquake",            "Text Adventure Quake Level 1" },
	{ "ifwhispers5",        "IF Whispers 5" },
	{ "ill",                "I'll" },
	{ "imiagination",       "Imiagination" },
	{ "inamanor",           "In a Manor of Speaking" },
	{ "informschool",       "Inform School" },
	{ "inhumane",           "Inhumane: An Infralogic Massacre" },
	{ "insidewoman",        "Inside Woman" },
	{ "insight",            "Insight" },
	{ "intangible",         "Intangible" },
	{ "interviewrockstar",  "Interview With a Rock Star" },
	{ "inthenavy",          "In the Navy" },
	{ "intruder",           "Intruder" },
	{ "iraqiinvasion",      "Iraqi Invasion: A Text Misadventure" },
	{ "island",             "The Island" },
	{ "islandsfaraway",     "Islands Far Away" },
	{ "jayisponies",        "Jay Is Ponies" },
	{ "jetblue",            "Jet-Blue" },
	{ "jewelalithia",       "The Jewel of Alithia" },
	{ "jewelofknowledge",   "The Jewel of Knowledge" },
	{ "jigsaw",             "Jigsaw" },
	{ "jigsawrules",        "Jigsaw: Rules and Footnotes" },
	{ "juicehead",          "Juicehead" },
	{ "justanotherday_tl",  "Just Another Day (by Theodore C. Lim)" },
	{ "karn",               "Return to Karn" },
	{ "kazooist",           "The Kazooist" },
	{ "kennykoala",         "Kenny Koala's Bushfire Survival Plan" },
	{ "kentishplover",      "Kentish Plover" },
	{ "kidnapsea",          "Kidnapped - On the Sea" },
	{ "kierkegaardsspider", "Kierkegaard's Spider" },
	{ "kiiwii",             "Kii!Wii! A Tiny Friend to Brighten Your Day" },
	{ "killingthedoctor",   "Killing the Doctor" },
	{ "kirby",              "The X-Child, Kevin Johnson Residence Hall Saga 2" },
	{ "kitten",             "Robot Finds Kitten" },
	{ "kooku",              "Kook U" },
	{ "lackofvision",       "Lack of Vision" },
	{ "lambs",              "Silence of the Lambs" },
	{ "lambs2",             "Silence of the Lambs 2" },
	{ "largemachine",       "Large Machine" },
	{ "lash",               "Local Asynchronous Satellite Hookup" },
	{ "ldodoom",            "Last Days of Doom" },
	{ "lecture",            "Lecture Feature" },
	{ "legacy",             "The Family Legacy" },
	{ "legendmisshat",      "The Legend of the Missing Hat" },
	{ "lex",                "LeX" },
	{ "librarian",          "Life of a Librarian" },
	{ "libraryhorror",      "The Library of Horror" },
	{ "lifewithocd",        "Life With OCD" },
	{ "lifeordeath",        "Life or Death" },
	{ "limp",               "Limp" },
	{ "lionskin",           "In the Skin of a Lion Quest: Caravaggio's Journey" },
	{ "livejoseph",         "LiveJoseph" },
	{ "llr3",               "A Little Like Rogue" },
	{ "lmstvg_zcode",       "LMS The Video Game" },
	{ "lobsteronaplane",    "Lobsters on a Plane" },
	{ "lockeddoor1",        "Locked Door I" },
	{ "lockeddoor2",        "Locked Door II" },
	{ "lockeddoor3",        "Locked Door III" },
	{ "lockeddoor4",        "Locked Door IV" },
	{ "lockeddoor5",        "Locked Door V" },
	{ "lockeddoor6",        "Locked Door VI" },
	{ "lockeddoor7",        "Locked Door VII" },
	{ "lockeddoor8",        "Locked Door VIII" },
	{ "lockeddoor9",        "Locked Door IX" },
	{ "lockedroom",         "The Locked Room" },
	{ "lookingtothesky",    "Looking to the Sky" },
	{ "lostpigandplace",    "Lost Pig and Place Under Ground" },
	{ "lostspellmaker",     "The Lost Spellmaker" },
	{ "ludicorp",           "The Ludicorp Mystery" },
	{ "magicmirror",        "Magic Mirror" },
	{ "magicmuffin",        "Magic Muffin - The Desert" },
	{ "makeitgood",         "Make it Good" },
	{ "mansion",            "Mansion" },
	{ "mapa_zcode",         "El Mapa" },
	{ "martyquest",         "Back to the Future - Marty Quest" },
	{ "medusa",             "Medusa" },
	{ "meetingrobb",        "Meeting Robb Sherwin" },
	{ "mehplace_zcode",     "The Meh Place" },
	{ "memorylane",         "Memory Lane" },
	{ "mercurytrucking",    "The Mercury Trucking Company" },
	{ "mercy",              "Mercy" },
	{ "metamorphoses",      "Metamorphoses" },
	{ "mhpquest_zcode",     "Quest for the Magic Healing Plant" },
	{ "micropuzzle",        "Micropuzzle" },
	{ "midpoints",          "Midpoints" },
	{ "mines",              "Mines" },
	{ "minimalistgame",     "The Minimalist Game" },
	{ "minimalistgame2",    "The Minimalist Game 2" },
	{ "misdirection",       "The Act of Misdirection" },
	{ "missinggrandpa",     "Missing Grandpa: Lost in Time" },
	{ "monkeybear",         "Monkey and Bear" },
	{ "monstermaker",       "Monster Maker" },
	{ "monzasphantom",      "Monza's Phantom" },
	{ "moonglow",           "Miniventure #1: Moonglow" },
	{ "moonshaped",         "Moon-Shaped" },
	{ "moonwrecked",        "Moonwrecked" },
	{ "mornelune",          "Morne Lune" },
	{ "mortalkombat",       "Mortal Kombat: Fire and Ice" },
	{ "mortlakemanor",      "Mortlake Manor" },
	{ "motelcalifornia",    "Motel California" },
	{ "mountain",           "Mountain" },
	{ "mousequest",         "Mouse Quest - Chapter 1: The Arrival of Winter" },
	{ "mousequest2",        "Mouse Quest - Chapter 2: Down the Coble Creek" },
	{ "mousequest3",        "Mouse Quest - Chapter 3: The Council of Mice" },
	{ "mousequest4",        "Mouse Quest - Chapter 4: The Forgotten Hero" },
	{ "mrscrabtree",        "Mrs. Crabtree's Geography Class" },
	{ "mst3k1",             "Mystery Science Theater 3000 Presents 'Detective'" },
	{ "mst3k2",             "Mystery Science Theater 3000 Presents 'A Fable'" },
	{ "muffinquest",        "The Quest for the Magic Muffin" },
	{ "muffinquest2",       "The Quest for the Magic Muffin 2" },
	{ "muffinquest3",       "Quest for the Magic Bagel...Err Muffin 3" },
	{ "mulldoon",           "The Mulldoon Legacy" },
	{ "mulldoonmurders",    "The Mulldoon Murders" },
	{ "murdac",             "Murdac" },
	{ "murdererleft",       "What the Murderer Had Left" },
	{ "musician",           "The Musician" },
	{ "mylastduchess",      "My Last Duchess" },
	{ "mymagictirehoax",    "My Magic Tire Hoax" },
	{ "mysterywinch",       "The Mystery of Winchester High" },
	{ "myunclegeorge",      "My Uncle George" },
	{ "nameless",           "Endless, Nameless" },
	{ "nascarexperience",   "The Realistic Nascar eXperience" },
	{ "necklace",           "Necklace of Skulls" },
	{ "nemeanlion",         "The Nemean Lion" },
	{ "nemesismacana",      "Nemesis Macana" },
	{ "neverplayed_zcode",  "So, You've Never Played a Text Adventure Before, Huh?" },
	{ "nidus",              "Nidus" },
	{ "nightbunnies",       "The Night of the Vampire Bunnies" },
	{ "nightchristmas",     "An Abbreviated Night Before Christmas" },
	{ "nightcomputer",      "Night at the Computer Center" },
	{ "nihilism",           "The Abyss" },
	{ "ninepoints",         "Nine Points" },
	{ "niney",              "Niney" },
	{ "njag",               "Not Just a Game" },
	{ "njag2",              "Not Just a Game 2" },
	{ "noroom",             "No Room" },
	{ "northnorth",         "The Northnorth Passage" },
	{ "noseguard",          "I Never Promised You a Nose Guard" },
	{ "nostrils",           "Nostrils of Flesh and Clay" },
	{ "notinvenice",        "Not in Venice" },
	{ "nudistsgonewild",    "Nudists Gone Wild" },
	{ "odieus",             "Odieus's Quest for the Magic Flingshot" },
	{ "ogisoas",            "One Game in Search of a Story" },
	{ "omniquest",          "Omniquest" },
	{ "once",               "Once" },
	{ "onegirl",            "One Girl" },
	{ "onlywar",            "Only War - Warhammer 40.000" },
	{ "onyourback",         "On Your Back" },
	{ "openthatvein",       "Open That Vein" },
	{ "ottumwa_zcode",      "PDFA Ottumwa" },
	{ "outofthepit",        "Out of the Pit" },
	{ "paddlingmania",      "Total Paddling Mania" },
	{ "paintandcorners",    "Paint and Corners" },
	{ "palimpsest",         "Palimpsest" },
	{ "paperbagprinc",      "The Paper Bag Princess" },
	{ "paperchase",         "Paper Chase" },
	{ "parallel",           "Parallel" },
	{ "paranoia",           "Paranoia" },
	{ "parc",               "Parc" },
	{ "pasdedeux",          "Pas De Deux" },
	{ "pathway_zcode",      "Pathway to Destruction" },
	{ "peacock",            "Not Made With Hands" },
	{ "pentari",            "Pentari" },
	{ "penury",             "The Algophilists' Penury" },
	{ "perilousmagic",      "Perilous Magic" },
	{ "perrysworld",        "Perry's World" },
	{ "philosophersstone",  "The Quest for the Philosopher's Stone" },
	{ "phlegm",             "Phlegm" },
	{ "phoenix",            "Phoenix's Landing: Destiny" },
	{ "phoneboothp2",       "Pick up the Phone Booth and Die II" },
	{ "photograph",         "Photograph" },
	{ "photopia_zcode",     "Photopia" },
	{ "piece",              "Piece of Mind" },
	{ "pigpancake",         "Pigpancake" },
	{ "piracy2",            "Piracy 2.0 - A Text Adventure in Space" },
	{ "piraterailroad",     "Pirate Railroad" },
	{ "ponderances",        "Ponderances" },
	{ "praser5",            "Praser 5" },
	{ "pressedon",          "I Pressed On, Being Chased by a Stapler With My Name on It" },
	{ "priceoffreedom",     "The Price of Freedom" },
	{ "primrose",           "The Primrose Path" },
	{ "prize",              "The Prize" },
	{ "probing",            "Offensive Probing" },
	{ "progressive1",       "IF Progressive One" },
	{ "promoted",           "Promoted!" },
	{ "pueblofantasma",     "Pueblo Fantasma" },
	{ "puerto",             "The Board Game Puerto Rico" },
	{ "punkpoints",         "Punk Points" },
	{ "puppetman",          "The Puppet-Man" },
	{ "putpbaa",            "Pick up the Phone Booth and Aisle" },
	{ "puzzle",             "Puzzle" },
	{ "pytho_zcode",        "Pytho's Mask" },
	{ "quidditch1954",      "The Quidditch Final of 1954" },
	{ "quitequeer",         "Quite Queer Night Near" },
	{ "rachaelbadday",      "Rachel Has a Bad Day" },
	{ "ralph",              "Ralph" },
	{ "rameses",            "Rameses" },
	{ "ranshints",          "RANS Hints" },
	{ "rans",               "RANS" },
	{ "readmay_zcode",      "A Reading in May" },
	{ "reddex",             "Reddex" },
	{ "relief",             "The Hunt for Relief" },
	{ "reorbushcave_zcode", "Reor's Bush-Cave (The Sprout Pouch pt 4)" },
	{ "reser",              "Rock'Em Sock'Em Robots" },
	{ "resident",           "The Resident" },
	{ "returncastle",       "Return to the Castle" },
	{ "revengebabes",       "Revenge of the Killer Surf Nazi Robot Babes From Hell" },
	{ "reverb",             "Reverberations" },
	{ "reverzi",            "Reverzi" },
	{ "ribbons",            "Ribbons" },
	{ "risenecropolis",     "Rise of the Necropolis" },
	{ "risorg_zcode",       "Risorgimento Represso" },
	{ "robodud",            "Robodud" },
	{ "robotgardening",     "Robot Gardening" },
	{ "robots",             "Robots - Another Abuse of the Z-Machine" },
	{ "rockinghorse",       "RockingHorse" },
	{ "rogue",              "zROGUE" },
	{ "roomserial",         "Room Serial" },
	{ "robopuppyredux",     "Robopuppy Redux" },
	{ "rota",               "The Reliques of Tolti-Aph" },
	{ "rpn",                "RPN" },
	{ "rtdoom",             "Return to Doom" },
	{ "safe_zcode",         "Safe" },
	{ "samegame",           "SameGame, Another Episode in the Z-Machine Abuse Saga" },
	{ "samhain",            "Samhain: Pick Up the Jack O' Lantern and Die" },
	{ "samurai",            "Samurai Tea Room" },
	{ "sanddancer_zcode",   "Sand-dancer" },
	{ "sangraal",           "Sangraal" },
	{ "santassleighride",   "Santa's Sleigh Ride" },
	{ "sauguslibrary",      "Saugus.net HSC 2002: Late Night in the Saugus Public Library" },
	{ "saugusfirstparish",  "Saugus.net HSC 2003: Below the First Parish Cemetery" },
	{ "saveprinceton",      "Save Princeton" },
	{ "savoirfaire",        "Savoir-Faire" },
	{ "scald",              "Scald" },
	{ "schooldays",         "Schooldays" },
	{ "scopa",              "Scopa" },
	{ "semid",              "Semi Intelligent Design" },
	{ "sgtguffysday",       "Sergeant Guffy's Day" },
	{ "shadowgate_dg",      "Shadowgate (by David Griffith)" },
	{ "shadowofmemories",   "Shadow of Memories" },
	{ "shadowsoldiers",     "Shadow Soldiers" },
	{ "shallow",            "Shallow" },
	{ "shattmem",           "Shattered Memory" },
	{ "sherlock1",          "Sherlock Gamebook #1: Murder at the Diogenes Club" },
	{ "sherlock2",          "Sherlock Gamebook #2: The Black River Emerald" },
	{ "sherlock3",          "Sherlock Gamebook #3: Death at Appledore Towers" },
	{ "sherlock4",          "Sherlock Gamebook #4: The Crown vs Dr. Watson" },
	{ "shrapnel",           "Shrapnel" },
	{ "simpletheft2",       "A Simple Theft 2: A Simple Theftier" },
	{ "slackerx",           "Slacker X" },
	{ "sleepcycle",         "Sleep Cycle" },
	{ "smallroom",          "Trapped in a Small Room" },
	{ "smallsimple",        "A Small & Simple Text Adventure" },
	{ "snafufun",           "The Adventure" },
	{ "snowedin",           "Snowed In" },
	{ "sofar",              "So Far" },
	{ "softporn",           "Softporn Adventure" },
	{ "solitary",           "Solitary" },
	{ "somewhere",          "Somewhere" },
	{ "soreality",          "So Reality" },
	{ "spaceinvaderz",      "Space InvaderZ" },
	{ "spacestation",       "Space Station" },
	{ "spadventure",        "SpAdventure" },
	{ "spiderandweb",       "Spider and Web" },
	{ "spiritwrak",         "SpiritWrak" },
	{ "sporkery1",          "Sporkery 1: There Will Be Sporking" },
	{ "spotlight",          "The Spotlight" },
	{ "spot",               "The Spot" },
	{ "spring",             "She's Got a Thing For a Spring" },
	{ "spring2020",         "Spring 2020" },
	{ "spycatcher",         "Spycatcher" },
	{ "starborn",           "Starborn" },
	{ "stargods",           "The Star Gods" },
	{ "starlight_zcode",    "Starlight" },
	{ "starrydepths_zcode", "Secret of the Starry Depths" },
	{ "starshit",           "Starshit" },
	{ "steine",             "Steine" },
	{ "stewgoing",          "You've Got a Stew Going!" },
	{ "stiffmst",           "Stiffy Makane: Mystery Science Theater 3000" },
	{ "stiffy",             "The Incredible Erotic Adventures of Stiffy Makane!" },
	{ "stinkorswim",        "Stink or Swim" },
	{ "stonecave",          "The Stone Cave" },
	{ "strainedtea",        "Strained Tea" },
	{ "strangeworld",       "Strange World" },
	{ "suicide",            "Suicide" },
	{ "sunburst",           "Sunburst: A C64 Science Fiction Adventure Game" },
	{ "sundayafternoon",    "Sunday Afternoon" },
	{ "surfboard",          "Surfboard" },
	{ "survive_zcode",      "Survive" },
	{ "sutwin",             "The Space Under the Window" },
	{ "suvehnux",           "Suveh Nux" },
	{ "swineback",          "Swineback Ridge" },
	{ "sycamoratree",       "Sycamora Tree" },
	{ "taipan",             "Taipan!" },
	{ "tatctae",            "Time: All Things Come to an End" },
	{ "tauntingdonut",      "Taunting Donut" },
	{ "tblw_zcode",         "The Blood Lust Warrior" },
	{ "tcomremake",         "The Colour of Magic Remake" },
	{ "tcoty",              "The Citizen of the Year" },
	{ "teacherfeature",     "Teacher Feature" },
	{ "teachher2dance",     "Teaching Her to Dance" },
	{ "telling",            "Whom the Telling Changed" },
	{ "terribleoldmanse",   "The Terrible Old Manse: 8bit Fun in 7bit ASCII" },
	{ "terrortabby",        "Attack of the Terror Tabby!" },
	{ "tesseract",          "Beyond the Tesseract" },
	{ "textgolf",           "Textfire Golf" },
	{ "tgm",                "The Great Machine - A Fragment" },
	{ "thanet",             "From Thanet to New Romney" },
	{ "thatdamnelevator",   "That Damn Elevator" },
	{ "thatdamnremote",     "That Damn Remote" },
	{ "the5continent",      "The Fifth Continent" },
	{ "theatre",            "Theatre" },
	{ "thebaron",           "The Baron" },
	{ "thecomputerlady",    "The Computer Lady" },
	{ "theenchantedcastle", "The Enchanted Castle" },
	{ "thegrandtour",       "The Grand Tour" },
	{ "thegreat",           "The Great" },
	{ "theinvestment",      "The Investment" },
	{ "thelighthousemm",    "The Lighthouse (by Marius Muller)" },
	{ "themall",            "The Mall" },
	{ "thesnowman",         "The Snowman" },
	{ "thesproutpouch",     "Reor's Bush-Cave: The Sprout Pouch" },
	{ "thorn",              "The Thorn" },
	{ "threecows",          "Three Cows and Two Doors" },
	{ "threediopolis",      "Threediopolis" },
	{ "timedwarves",        "Time and Dwarves" },
	{ "timefortea",         "Time For Tea: A Game of Tea, Cakes, and Deadly Secrets" },
	{ "tirehoax",           "My Magic Tire Hoax" },
	{ "tk1",                "Time Killer #1: Claustrophobia" },
	{ "tkatc",              "The King and the Crown, Special Edition" },
	{ "toask",              "Treasures of a Slaver's Kingdom" },
	{ "tok",                "ToK" },
	{ "tower",              "Tower" },
	{ "towersofhanoi",      "Towers of Hanoi" },
	{ "townmusicians",      "The Town Musicians" },
	{ "toxinx",             "Toxin X" },
	{ "trapped_zcode",      "Trapped" },
	{ "tristamisland",      "Tristam Island" },
	{ "trolleyeview",       "Zork: A Troll's Eye View" },
	{ "truehero",           "True Hero: Quest for the Banshee Heart" },
	{ "trunspecified",      "Treasures of an Unspecified Region or District" },
	{ "trw",                "Tull Road Warrior" },
	{ "trystoffate",        "Tryst of Fate" },
	{ "tundra",             "Tundra" },
	{ "tutorial",           "Tutorial" },
	{ "tutorialhotel",      "Hotel Tutorial" },
	{ "tuuli",              "Tuuli" },
	{ "txtadv",             "Txt Adventure" },
	{ "uhoh",               "Uh-Oh!" },
	{ "umw_zcode",          "UMW" },
	{ "underdoos",          "The Underoos That Ate New York!" },
	{ "underground_zcode",  "Underground" },
	{ "underthebed",        "Under the Bed" },
	{ "ungodlyhour",        "Ungodly Hour" },
	{ "unicornpool",        "The Unicorn Pool" },
	{ "uninvited",          "UNINVITED" },
	{ "vacation",           "Vacation Gone Awry" },
	{ "vagueness",          "What Happens in Vagueness" },
	{ "vampiresun",         "House of the Midnight Sun" },
	{ "varicella",          "Varicella" },
	{ "vengeance",          "Vengeance" },
	{ "vergingpaths",       "The Garden of Verging Paths" },
	{ "veryvile_zcode",     "Prime Pro-Rhyme Row #1: Very Vile Fairy File" },
	{ "vigilance",          "Internal Vigilance" },
	{ "vindaloo",           "Vindaloo" },
	{ "virtualgrandnation", "The Virtual Grand National" },
	{ "visualizing",        "Visualizing" },
	{ "vosr",               "Voices of Spoon River" },
	{ "wadewar2",           "The WadeWars Book II: Niska" },
	{ "wadewar3",           "The WadeWars Book III: Askin" },
	{ "warblersnest",       "The Warbler's Nest" },
	{ "warp",               "Warp!" },
	{ "weapon",             "The Weapon" },
	{ "weareunfinished",    "We Are Unfinished" },
	{ "weirdcity",          "Weird City Interloper" },
	{ "weirdness",          "Weirdness" },
	{ "welcome",            "Welcome" },
	{ "welcometohell",      "Welcome to Hell" },
	{ "welcometopuerto",    "Welcome to Puerto Rico" },
	{ "wernersquest1",      "Werner's Quest 1: The Black Ladder" },
	{ "wernersquest2",      "Werner's Quest 2: Revenge of the Thing-Fish" },
	{ "wernersquest3",      "Werner's Quest 3: Rance the Dungeonkeeper" },
	{ "wernersquest4",      "Werner's Quest 4: Unchanter" },
	{ "whispers",           "The Corn Identity" },
	{ "wildflowers",        "Wildflowers" },
	{ "williamtell",        "William Tell" },
	{ "winchester",         "Winchester's Nightmare" },
	{ "windhall",           "The Path to Fortune" },
	{ "winterwonder",       "Winter Wonderland" },
	{ "wir1",               "When in Rome 1: Accounting for Taste" },
	{ "wir2",               "When in Rome 2: Far From Home" },
	{ "wireless",           "Get Magazine. Open Magazine. Read Article" },
	{ "wizardscastle",      "Wizard's Castle" },
	{ "wizardsmagic",       "Wizards Magic" },
	{ "worldupsidedown",    "The World Turned Upside Down" },
	{ "wsp",                "Weishaupt Scholars: Prologue" },
	{ "wump2ka",            "Wumpus 2000, The Virulent Labyrinth of Yob-Shuggoth" },
	{ "wumpus",             "Hunt the Wumpus" },
	{ "wurm",               "Wurm" },
	{ "wwwanderer",         "Werewolves and Wanderer" },
	{ "xenophobia",         "Xenophobia" },
	{ "yakshaving",         "Yak Shaving for Kicks and Giggles!" },
	{ "yastanding",         "You Are Standing" },
	{ "yomomma",            "Raising the Flag on Mount Yo Momma" },
	{ "zassball",           "ZassBall, Another Abuse of the Z-Machine" },
	{ "zbefunge",           "ZBefunge 0.7" },
	{ "zcamel",             "Camel" },
	{ "zcatalog",           "The Z-Files, A Z-Code Catalog" },
	{ "zchess",             "Z-Chess" },
	{ "zdungeon",           "ZDungeon" },
	{ "zedfunge",           "ZedFunge 0.7.3" },
	{ "zedit",              "ZEdit" },
	{ "zegro",              "Zegrothenus" },
	{ "zenon",              "Escape From the Starship Zenon" },
	{ "zenspeak",           "Zen Speaks!" },
	{ "zlife",              "Z-Life" },
	{ "zokoban",            "Z-Machine Sokoban" },
	{ "zombies",            "Zombies, Yet Another Abuse of the Z-Machine" },
	{ "zork285",            "Zork 285 Points Replica" },
	{ "zorkianstories1",    "Zorkian Stories 1: G.U.E" },
	{ "zorknplus9",         "Zork N Plus 9" },
	{ "zpegasus",           "Pegasus" },
	{ "zracer",             "ZRacer" },
	{ "zsnake",             "Z-Snake" },
	{ "ztornado",           "Z-Tornado" },
	{ "ztrek",              "Super Z Trek" },
	{ "zugzwang",           "Zugzwang" },
	{ "zunidoll",           "The Zuni Doll" },

	// Painfull Little Stupid Games
	{ "plsg1",              "Painless Little Stupid Games #1: Dinnertime" },
	{ "plsg2",              "Painless Little Stupid Games #2: To Get To the Other Side" },
	{ "plsg3",              "Painless Little Stupid Games #3: They're After You!" },
	{ "plsg4",              "Painless Little Stupid Games #4: Mazemapper" },
	{ "plsg5",              "Painless Little Stupid Games #5: The Mean Story" },
	{ "plsg6",              "Painless Little Stupid Games #6: Mahadev" },
	{ "plsg7",              "Painless Little Stupid Games #7: Sturdlint" },
	{ "plsg8",              "Painless Little Stupid Games #8: The Last Dark Day" },
	{ "plsg9",              "Painless Little Stupid Games #9: Zork LXIX" },
	{ "plsg10",             "Painless Little Stupid Games #10: The Valley House" },

	// Converted Scott Adams Classic Adventures games
	{ "adventurelandi5",    "S.A.C.A. #1: Adventureland" },
	{ "pirateadventurei5",  "S.A.C.A. #2: Pirate Adventure" },
	{ "missionimpossibli5", "S.A.C.A. #3: Mission Impossible" },
	{ "voodoocastlei5",     "S.A.C.A. #4: Voodoo Castle" },
	{ "thecounti5",         "S.A.C.A. #5: The Count" },
	{ "strangeodysseyi5",   "S.A.C.A. #6: Strange Odyssey" },
	{ "mysteryfunhousei5",  "S.A.C.A. #7: Mystery Fun House" },
	{ "pyramidofdoomi5",    "S.A.C.A. #8: Pyramid of Doom" },
	{ "ghosttowni5",        "S.A.C.A. #9: Ghost Town" },
	{ "savageisland1i5",    "S.A.C.A. #10: Savage Island, Part I" },
	{ "savageisland2i5",    "S.A.C.A. #11: Savage Island, Part II" },
	{ "goldenvoyagei5",     "S.A.C.A. #12: The Golden Voyage" },
	{ "sorcererclaycstli5", "S.A.C.A. #13: Sorcerer of Claymorgue Castle" },
	{ "returnpirteislei5",  "S.A.C.A. #14A: Return to Pirate's Isle" },
	{ "buckaroobanzaii5",   "S.A.C.A. #14B: Buckaroo Banzai" },
	{ "adventlandsmpleri5", "S.A.C.A.: Adventureland Sampler" },
	{ "marveladventurei5",  "S.A.C.A.: The Hulk" },
	{ "questprobe2i5",      "S.A.C.A.: Spiderman" },

	// Mysterious Adventures by Brian Howarth
	{ "goldenbatoni5",      "Mysterious Adventures #01: The Golden Baton" },
	{ "timemachinei5",      "Mysterious Adventures #02: The Time Machine" },
	{ "arrowofdeath1i5",    "Mysterious Adventures #03: Arrow of Death Part 1" },
	{ "arrowofdeath2i5",    "Mysterious Adventures #04: Arrow of Death Part 2" },
	{ "pulsar7i5",          "Mysterious Adventures #05: Escape From Pulsar 7" },
	{ "circusi5",           "Mysterious Adventures #06: Circus" },
	{ "feasibilityi5",      "Mysterious Adventures #07: Feasibility Experiment" },
	{ "akyrzi5",            "Mysterious Adventures #08: The Wizard of Akyrz" },
	{ "perseusi5",          "Mysterious Adventures #09: Perseus and Andromeda" },
	{ "10indiansi5",        "Mysterious Adventures #10: Ten Little Indians" },
	{ "waxworksi5",         "Mysterious Adventures #11: Waxworks" },

	// Apollo 18+20: The IF Tribute Album
	{ "apollo1",            "Apollo 18+20 #01: Dig My Grave" },
	{ "apollo2",            "Apollo 18+20 #02: I Palindrome I" },
	{ "apollo4",            "Apollo 18+20 #04: My Evil Twin" },
	{ "apollo5",            "Apollo 18+20 #05: Mammal" },
	{ "apollo6",            "Apollo 18+20 #06: The Statue Got Me High" },
	{ "apollo7",            "Apollo 18+20 #07: Spider" },
	{ "apollo8",            "Apollo 18+20 #08: The Guitar(The Lion Sleeps Tonight)" },
	{ "apollo9",            "Apollo 18+20 #09: Dinner Bell" },
	{ "apollo11",           "Apollo 18+20 #11: Hall of Heads" },
	{ "apollo13",           "Apollo 18+20 #13: See the Constellation" },
	{ "apollo14",           "Apollo 18+20 #14: If I Wasn't Shy" },
	{ "apollo16",           "Apollo 18+20 #16: Hypnotist of Ladies" },
	{ "apollo17",           "Apollo 18+20 #17: Fingertips - Everything Is Catching on Fire" },
	{ "apollo18",           "Apollo 18+20 #18: Fingertips - Fingertips" },
	{ "apollo20",           "Apollo 18+20 #20: Fingertips - Hey Now, Everybody" },
	{ "apollo21",           "Apollo 18+20 #21: Fingertips - Who's That Standing Out the Window" },
	{ "apollo22",           "Apollo 18+20 #22: Fingertips - I Found a New Friend" },
	{ "apollo23",           "Apollo 18+20 #23: Fingertips - Come On and Wreck My Car" },
	{ "apollo24",           "Apollo 18+20 #24: Fingertips - Aren't You the Guy Who Hit Me in the Eye" },
	{ "apollo25",           "Apollo 18+20 #25: Fingertips - Please Pass the Milk Please" },
	{ "apollo26",           "Apollo 18+20 #26: Fingertips - Leave Me Alone" },
	{ "apollo27",           "Apollo 18+20 #27: Fingertips - Who's Knockin' on the Wall" },
	{ "apollo28",           "Apollo 18+20 #28: Fingertips - All Alone" },
	{ "apollo29",           "Apollo 18+20 #29: Fingertips - What's That Blue Thing Doing Here" },
	{ "apollo30",           "Apollo 18+20 #30: Fingertips - Something Grabbed Ahold of My Hand" },
	{ "apollo31",           "Apollo 18+20 #31: Fingertips - I Don't Understand You" },
	{ "apollo32",           "Apollo 18+20 #32: Fingertips - I Heard a Sound" },
	{ "apollo34",           "Apollo 18+20 #34: Fingertips - The Day That Love Came to Play" },
	{ "apollo35",           "Apollo 18+20 #35: Fingertips - I'm Having a Heart Attack" },
	{ "apollo36",           "Apollo 18+20 #36: Fingertips - Fingertips(Reprise)" },
	{ "apollo37",           "Apollo 18+20 #37: Fingertips - I Walk Along Darkened Corridors" },
	{ "apollo38",           "Apollo 18+20 #38: Space Suit" },

	// Acorn User 1996 IFComp
	{ "bse",                "BSE" },                                                                    //    1st Place
	{ "thewedding",         "The Wedding" },                                                            //    2nd Place
	{ "leopold",            "Leopold the Ministrel" },                                                  //    3rd Place
	{ "transporter",        "Trasnsporter" },                                                           //    Runner-Up
	{ "blacknwhiterag",     "Black’n’White Rag" },                                                      //    Runner-Up

	// IFComp 1995
	{ "if95_weather",       "A Change in the Weather" },                                                //    1st Place
	{ "if95_mindelectric",  "The Mind Electric" },                                                      //    2nd Place
	{ "if95_magictoyshop",  "The Magic Toyshop" },                                                      //    3rd Place
	{ "if95_mst3k1",        "Mystery Science Theater 3000 Presents 'Detective'" },                      //    4th Place
	{ "if95_libraryfront",  "All Quiet on the Library Front" },                                         //    5th Place
	{ "if95_tubetrouble",   "Tube Trouble" },                                                           //    6th Place

	// IFComp 1996
	{ "if96_sherbet",       "The Meteor, the Stone and a Long Glass of Sherbet" },                      //    1st Place
	{ "if96_tapestry",      "Tapestry" },                                                               //    2nd Place
	{ "if96_delusions",     "Delusions" },                                                              //    3rd Place
	{ "if96_fear",          "Fear" },                                                                   //    6th Place
	{ "if96_claw",          "Wearing the Claw" },                                                       //    8th Place
	{ "if96_lists",         "Lists and Lists" },                                                        //   11th Place
	{ "if96_ralph",         "Ralph" },                                                                  //   12th Place
	{ "if96_reverb",        "Reverberations" },                                                         //   13th Place
	{ "if96_intheend",      "In the End" },                                                             //   15th Place
	{ "if96_piece",         "Piece of Mind" },                                                          //   16th Place
	{ "if96_phlegm",        "Phlegm" },                                                                 //   17th Place
	{ "if96_stargaze",      "Stargazer" },                                                              //   19th Place
	{ "if96_forms",         "Of Forms Unknown" },                                                       //   20th Place
	{ "if96_housestalker",  "House of the Stalker" },                                                   //   23rd Place
	{ "if96_ripflesh",      "Rippled Flesh" },                                                          //   24th Place
	{ "if96_liquid",        "Punkirita Quest 1: Liquid" },                                              //   25th Place
	{ "comp96",             "IF Competition '96 Unofficial FE Game" },

	// IFComp 1997
	{ "if97_edifice",       "The Edifice" },                                                            //    1st Place
	{ "if97_spring",        "She's Got a Thing For a Spring" },                                         //    4th Place
	{ "if97_bear",          "A Bear's Night Out" },                                                     // 	  5th Place
	{ "if97_lostspellmake", "The Lost Spellmaker" },                                                    //    8th Place
	{ "if97_mimesis",       "Sins Against Mimesis" },                                                   //	  9th Place
	{ "if97_newday",        "A New Day" },                                                              //   10th Place
	{ "if97_erden",         "Travels in the Land of Erden: In Quest of the Adventure" },                //   14th Place
	{ "if97_friday",        "Friday Afternoon" },                                                       //   16th Place
	{ "if97_estrange",      "Madame L'Estrange and the Troubled Spirit" },                              //   17th Place
	{ "if97_sylenius",      "Sylenius Mysterium" },                                                     //   18th Place
	{ "if97_pizza",         "Phred Phontious and the Quest for Pizza" },                                //   19th Place
	{ "if97_agb",           "A Good Breakfast" },                                                       //   23rd Place
	{ "if97_tdragon",       "Town Dragon" },                                                            //   24th Place
	{ "if97_thetempest",    "The Tempest" },                                                            //   25th Place
	{ "if97_pintown",       "Pintown" },                                                                //   28th Place
	{ "if97_congrats",      "Congratulations!" },                                                       //   30th Place
	{ "if97_cask",          "Cask" },                                                                   //   31st Place
	{ "if97_symetry",       "Symetry" },                                                                //   32nd Place
	{ "if97_auntnancy",     "Aunt Nancy's House" },                                                     //   33rd Place
	{ "if97_cominghome",    "Coming Home" },                                                            //   34th Place
	{ "comp97",             "IF Competition '97 Unofficial FE Game" },

	// IFComp 1998
	{ "if98_photopia",      "Photopia" },                                                               //    1st Place
	{ "if98_muse",          "Muse: An Autumn Romance" },                                                //    2nd Place
	{ "if98_enlighte",      "Enlightenment" },                                                          // 	  5th Place
	{ "if98_motherloose",   "Mother Loose" },                                                           // 	  6th Place
	{ "if98_bluemen",       "Little Blue Men" },                                                        // 	  7th Place
	{ "if98_dilly",         "Trapped in a One-Room Dilly" },                                            // 	  8th Place
	{ "if98_downtowntokyo", "Downtown Tokyo Present Day" },                                             //   10th Place
	{ "if98_informatory",   "Informatory" },                                                            //   11th Place
	{ "if98_ritualpurific", "Ritual of Purification" },                                                 //   12th Place
	{ "if98_city",          "The City" },                                                               //   13th Place
	{ "if98_evildwells",    "Where Evil Dwells" },                                                      //   14th Place
	{ "if98_purple",        "Purple" },                                                                 //   15th Place
	{ "if98_research",      "Research Dig" },                                                           //   17th Place
	{ "if98_spacestation",  "Space Station" },                                                          //   19th Place
	{ "if98_cattusatrox",   "Cattus Atrox" },                                                           //   20th Place
	{ "if98_spotlite",      "In the Spotlight" },                                                       //   21st Place
	{ "if98_acidwhiplash",  "Acid Whiplash" },                                                          //   23rd Place
	{ "if98_fifteen",       "Fifteen" },                                                                //   25th Place
	{ "if98_hrs",           "Human Resources Stories" },                                                //   27th Place
	{ "comp98",             "IF Competition '98 Unofficial FE Game" },

	// IFComp 1999
	{ "if99_winterwonder",  "Winter Wonderland" },                                                      //    1st Place
	{ "if99_forachange",    "For a Change" },                                                           //    2nd Place
	{ "if99_softfood",      "A Day for Soft Food" },                                                    // 	  4th Place
	{ "if99_halo",          "Halothane" },                                                              //    5th Place
	{ "if99_hunterdark",    "Hunter, In Darkness" },                                                    // 	  8th Place
	{ "if99_beatdevil",     "Beat the Devil" },                                                         //    9th Place
	{ "if99_jacks",         "Jacks or Better to Murder, Aces to Win" },                                 //   10th Place
	{ "if99_oad",           "Only After Dark" },                                                        //   17th Place
	{ "if99_lomalow",       "Lomalow" },                                                                //   21st Place
	{ "if99_calliope",      "Calliope" },                                                               //   23rd Place
	{ "if99_museduction",   "Music Education" },                                                        //   24th Place
	{ "if99_wossname",      "Spodgeville Murphy and the Jewelled Eye of Wossname" },                    //   25th Place
	{ "if99_thorfinn",      "Thorfinn's Realm" },                                                       //   28th Place
	{ "if99_death",         "Death to my Enemies" },                                                    //   29th Place
	{ "if99_chix",          "Chicks Dig Jerks" },                                                       //   31st Place
	{ "if99_passbanana",    "Pass the Banana" },                                                        //   33th Place
	{ "if99_outsided",      "Outsided" },                                                               //   34th Place
	{ "if99_ludite",        "Lurk. Unite. Die. Invent. Think. Expire." },                               //   35th Place
	{ "if99_guard",         "Guard Duty" },                                                             //   36th Place
	{ "comp99",             "IF Competition '99 Unofficial FE Game" },

	// IFComp 2000
	{ "if00_metamorphoses", "Metamorphoses" },                                                          //    2nd Place
	{ "if00_andrewplotkin", "Being Andrew Plotkin" },                                                   //    3rd Place
	{ "if00_adverbum",      "Ad Verbum" },                                                              //    4th Place
	{ "if00_transfer",      "Transfer" },                                                               //    5th Place
	{ "if00_myangel",       "My Angel" },                                                               //    6th Place
	{ "if00_nevermore",     "Nevermore" },                                                              //    7th Place
	{ "if00_masquerade",    "Masquerade" },                                                             //    8th Place
	{ "if00_yagwad",        "YAGWAD: Yes, Another Game With a Dragon!" },                               //    9th Place
	{ "if00_shade",         "Shade" },                                                                  //   10th Place
	{ "if00_guess",         "Guess The Verb!" },                                                        //   11th Place
	{ "if00_letters",       "Letters From Home" },                                                      //   12th Place
	{ "if00_rameses",       "Rameses" },                                                                //   13th Place
	{ "if00_djinni",        "The Djinni Chronicles" },                                                  //   14th Place
	{ "if00_bestman",       "The Best Man" },                                                           //   15th Place
	{ "if00_atwctw",        "And the Waves Choke the Wind" },                                           //   16th Place
	{ "if00_dinner",        "Dinner With Andre" },                                                      //   18th Place
	{ "if00_bigmama",       "The Big Mama" },                                                           //   20th Place
	{ "if00_punkpoints",    "Punk Points" },                                                            //   22th Place
	{ "if00_enlisted",      "Enlisted" },                                                               //   23rd Place
	{ "if00_rtzas",         "Return to Zork: Another Story" },                                          //   26rd Place
	{ "if00_gotid",         "Got ID?" },                                                                //   29th Place
	{ "if00_castleamnos",   "Castle Amnos" },                                                           //   30th Place
	{ "if00_masque",        "Masque of the Last Faeries" },                                             //   31st Place
	{ "if00_ppocket",       "The Pickpocket" },                                                         //   32nd Place
	{ "if00_happyeveraft",  "Happy Ever After" },                                                       //   34th Place
	{ "if00_prodly",        "Prodly the Puffin" },                                                      //   35th Place
	{ "if00_withdraw",      "Withdrawal Symptoms" },                                                    //   36th Place
	{ "if00_123",           "1-2-3..." },                                                               //   42nd Place
	{ "if00_kitty",         "Stupid Kittens" },                                                         //   44th Place
	{ "if00_comp00tr",      "Comp00ter Game" },                                                         //   49th Place
	{ "if00_asendent",      "Asendent" },                                                               //   51st Place
	{ "if00_whatif",        "What-IF?" },                                                               //   52nd Place
	{ "comp00",             "IF Competition '00 Unofficial FE Game" },

	// IFComp 2001
	{ "if01_allroads",      "All Roads" },                                                              //    1st Place
	{ "if01_moments",       "Moments Out of Time" },                                                    //    2nd Place
	{ "if01_heroes",        "Heroes" },                                                                 //    3rd Place
	{ "if01_cycles",        "Vicious Cycles" },                                                         //    6th Place
	{ "if01_eas",           "Earth and Sky" },                                                          //    8th Place
	{ "if01_eleven",        "Film at Eleven" },                                                         //   10th Place
	{ "if01_prized",        "Prized Possession" },                                                      //   11th Place
	{ "if01_finetuned",     "Fine-Tuned" },                                                             //   18th Place
	{ "if01_evil",          "The Evil Sorcerer" },                                                      //   20th Place
	{ "if01_gostak",        "The Gostak" },                                                             //   21nd Place
	{ "if01_crusadejg",     "Crusade (by John Gorenfeld)" },                                            //   23rd Place
	{ "if01_urhere",        "You Are Here" },                                                           //   25th Place
	{ "if01_elements",      "Elements" },                                                               //   26th Place
	{ "if01_banebuilders",  "Bane of the Builders" },                                                   //   28th Place
	{ "if01_colours",       "Colours" },                                                                //   32nd Place
	{ "if01_siliconcastle", "Silicon Castles" },                                                        //   32nd Place
	{ "if01_timeout",       "Timeout" },                                                                //   35th Place
	{ "if01_schroed",       "Schroedinger's Cat" },                                                     //   39th Place
	{ "if01_jump",          "Jump" },                                                                   //   41th Place
	{ "if01_newcomer",      "The Newcomer" },                                                           //   49th Place
	{ "comp01",             "IF Competition '01 Unofficial FE Game" },

	// IFComp 2002
	{ "if02_photograph",    "Photograph" },                                                             //    3rd Place
	{ "if02_moonlittower",  "The Moonlit Tower" },                                                      //    4th Place
	{ "if02_janitor",       "Janitor" },                                                                //    5th Place
	{ "if02_tookiessong",   "TOOKiE'S SONG" },                                                          //    7th Place
	{ "if02_ftaegea",       "Fort Aegea" },                                                             //    8th Place
	{ "if02_thetemple",     "The Temple" },                                                             //    9th Place
	{ "if02_jane",          "Jane" },                                                                   //   10th Place
	{ "if02_mythtale",      "MythTale" },                                                               //   11th Place
	{ "if02_idthief",       "Identity Thief" },                                                         //   13th Place
	{ "if02_rentaspy",      "Rent-A-Spy" },                                                             //   15th Place
	{ "if02_whenhelp",      "When Help Collides" },                                                     //   18th Place
	{ "if02_constraints",   "Constraints" },                                                            //   19th Place
	{ "if02_hell0",         "Hell: A Comedy of Errors" },                                               //   23th Place
	{ "if02_bofh",          "The Bastard Operator From Hell" },                                         //   26th Place
	{ "if02_samuelgregor",  "The Case of Samuel Gregor" },                                              //   27th Place
	{ "if02_screen",        "Screen" },                                                                 //   29th Place
	{ "if02_shamulet",      "Scary House Amulet" },                                                     //   31st Place
	{ "if02_koan",          "KOAN" },                                                                   //   35th Place
	{ "if02_ramandjona",    "Ramón and Jonathan" },                                                     //   36th Place
	{ "comp02",             "IF Competition '02 Unofficial FE Game" },

	// IFComp 2003
	{ "if03_slouchbedlam",  "Slouching Towards Bedlam" },                                               //    1st Place
	{ "if03_risorg",        "Risorgimento Represso" },                                                  //    2nd Place
	{ "if03_gourmet",       "Gourmet" },                                                                //    5th Place
	{ "if03_baluthar",      "Baluthar" },                                                               //    9th Place
	{ "if03_atomicheart",   "The Atomic Heart" },                                                       //   10th Place
	{ "if03_papermoon",     "A Paper Moon" },                                                           //   12th Place
	{ "if03_caffeination",  "Caffeination" },                                                           //   14th Place
	{ "if03_adoo",          "Adoo's Stinky Story" },                                                    //   17th Place
	{ "if03_domicile",      "Domicile" },                                                               //   18th Place
	{ "if03_internaldocs",  "Internal Documents" },                                                     //   19th Place
	{ "if03_noroom",        "No Room" },                                                                //   22th Place
	{ "if03_lardo",         "The Fat Lardo and the Rubber Ducky" },                                     //   29th Place
	{ "comp03",             "IF Competition '03 Unofficial FE Game" },

	// IFComp 2004
	{ "if04_bluechairs",    "Blue Chairs" },                                                            //    2nd Place
	{ "if04_devours",       "All Things Devours" },                                                     //    3rd Place
	{ "if04_stingwasp",     "Sting of the Wasp" },                                                      //    4th Place
	{ "if04_orion",         "The Orion Agenda" },                                                       //    6th Place
	{ "if04_mingsheng",     "MingSheng" },                                                              //    7th Place
	{ "if04_splashdown",    "Splashdown" },                                                             //    8th Place
	{ "if04_gamlet",        "Gamlet" },                                                                 //    9th Place
	{ "if04_greatxavio",    "The Great Xavio" },                                                        //   11th Place
	{ "if04_geb",           "Goose, Egg, Badger" },                                                     //   12th Place
	{ "if04_bigscoop",      "The Big Scoop" },                                                          //   13th Place
	{ "if04_identity",      "Identity" },                                                               //   15th Place
	{ "if04_murderaeroclb", "Murder at the Aero Club" },                                                //   16th Place
	{ "if04_bellclap",      "Bellclap" },                                                               //   17th Place
	{ "if04_typo",          "Typo!" },                                                                  //   19th Place
	{ "if04_blink",         "Blink" },                                                                  //   21nd Place
	{ "if04_playtorn",      "Chronicle Play Torn" },                                                    //   22nd Place
	{ "if04_order00",       "Order" },                                                                  //   24th Place
	{ "if04_bluesky_hf",    "Blue Sky (by Hans Fugal)" },                                               //   26th Place
	{ "if04_stackoverflow", "Stack Overflow" },                                                         //   29th Place
	{ "comp04",             "IF Competition '04 Unofficial FE Game" },

	// IFComp 2005
	{ "if05_vespers",       "Vespers" },                                                                //    1st Place
	{ "if05_beyond_zcode",  "Beyond" },                                                                 //    2nd Place
	{ "if05_anewlife",      "A New Life" },                                                             //    3rd Place
	{ "if05_toughbeans",    "Tough Beans" },                                                            //    5th Place
	{ "if05_colourpink",    "The Colour Pink" },                                                        //    6th Place
	{ "if05_unforgotten",   "Unforgotten" },                                                            //    7th Place
	{ "if05_snatches",      "Snatches" },                                                               //    8th Place
	{ "if05_vigilance",     "Internal Vigilance" },                                                     //   10th Place
	{ "if05_historyrepeat", "History Repeating" },                                                      //   11th Place
	{ "if05_soa",           "Son of a..." },                                                            //   15th Place
	{ "if05_psycheslament", "Psyche's Lament" },                                                        //   21st Place
	{ "if05_onoptimism",    "On Optimism" },                                                            //   24th Place
	{ "if05_neonnirvana",   "Neon Nirvana" },                                                           //   27th Place
	{ "if05_swordofmalice", "The Sword of Malice" },                                                    //   28th Place
	{ "if05_drearylands",   "Dreary Lands" },                                                           //   29th Place
	{ "if05_hellosword",    "Hello Sword: The Journey" },                                               //   30th Place
	{ "if05_phantomcavern", "Phantom: Caverns of the killer" },                                         //   31st Place
	{ "if05_futuregame",    "FutureGame" },                                                             //   33rd Place
	{ "comp05",             "IF Competition '05 Unofficial FE Game" },

	// IFComp 2006
	{ "if06_primrose",      "The Primrose Path" },                                                      //    2nd Place
	{ "if06_moonshaped",    "Moon-Shaped" },                                                            //    5th Place
	{ "if06_wallpaper",     "Delightful Wallpaper" },                                                   //    6th Place
	{ "if06_legion",        "Legion" },                                                                 //    7th Place
	{ "if06_madamspider",   "Madam Spider's Web" },                                                     //    8th Place
	{ "if06_mobius",        "Möbius" },                                                                 //    9th Place
	{ "if06_gameproducer",  "Game Producer!" },                                                         //   11th Place
	{ "if06_starcity",      "Star City" },                                                              //   13th Place
	{ "if06_strangegeo",    "Strange Geometries" },                                                     //   14th Place
	{ "if06_towerelephant", "The Tower of the Elephant" },                                              //   15th Place
	{ "if06_labyrinth",     "Labyrinth" },                                                              //   18th Place
	{ "if06_cdst",          "Carmen Devine: Supernatural Troubleshooter" },                             //   20th Place
	{ "if06_bibleretold1",  "The Bible Retold: The Bread and the Fishes" },                             //   21st Place
	{ "if06_anothergodamn", "Another Goddamn Escape the Locked Room Game" },                            //   22nd Place
	{ "if06_fightorflight", "Fight or Flight" },                                                        //   23rd Place
	{ "if06_manalive1",     "Manalive I - Enigma" },                                                    //   24th Place
	{ "if06_hedge",         "Hedge" },                                                                  //   25th Place
	{ "if06_polendina",     "Polendina" },                                                              //   27th Place
	{ "if06_manalive2",     "Manalive II" },                                                            //   29th Place
	{ "if06_pathfinder",    "Pathfinder" },                                                             //   30th Place
	{ "if06_apocalypclock", "The Apocalypse Clock" },                                                   //   31st Place
	{ "if06_sisyphus",      "Sisyphus" },                                                               //   39th Place
	{ "comp06",             "IF Competition '06 Unofficial FE Game" },

	// IFComp 2007
	{ "if07_lostpig",       "Lost Pig" },                                                               //    1st Place
	{ "if07_actofmurder",   "An Act of Murder" },                                                       //    2nd Place
	{ "if07_bellwater",     "Lord Bellwater's Secret" },                                                //    3rd Place
	{ "if07_acrossstars",   "Across the Stars" },                                                       //    4th Place
	{ "if07_orevore",       "Orevore Courier" },                                                        //    8th Place
	{ "if07_jackmills",     "My Name is Jack Mills" },                                                  //    9th Place
	{ "if07_deadlinenchan", "Deadline Enchanter" },                                                     //   12th Place
	{ "if07_fff",           "Fox, Fowl and Feed" },                                                     //   16th Place
	{ "if07_wish",          "Wish" },                                                                   //   17th Place
	{ "if07_packrat",       "Packrat" },                                                                //   18th Place
	{ "if07_beneathtransf", "BENEATH: A Transformation" },                                              //   21st Place
	{ "if07_theimmortal",   "The Immortal" },                                                           //   22nd Place
	{ "if07_eduardseminar", "Eduard the Seminarist" },                                                  //   23rd Place
	{ "if07_pets",          "Press [Escape] to Save" },                                                 //   24th Place
	{ "comp07",             "IF Competition '07 Unofficial FE Game" },

	// IFComp 2008
	{ "if08_violet",        "Violet" },                                                                 //    1st Place
	{ "if08_afflicted",     "Afflicted" },                                                              //    2nd Place
	{ "if08_piracy2",       "Piracy 2.0 - A Text Adventure in Space" },                                 //    3rd Place
	{ "if08_snacktime",     "Snack Time!" },                                                            //    6th Place
	{ "if08_openingnight",  "Opening Night" },                                                          //    7th Place
	{ "if08_escunderworld", "Escape From the Underworld" },                                             //   12th Place
	{ "if08_bishoes",       "Buried in Shoes" },                                                        //   13th Place
	{ "if08_grief",         "Grief" },                                                                  //   16th Place
	{ "if08_trein",         "Trein" },                                                                  //   17th Place
	{ "if08_redmoon",       "Red Moon" },                                                               //   19th Place
	{ "if08_draculascrypt", "Dracula's Underground Crypt" },                                            //   20th Place
	{ "if08_anachronist",   "Anachronist" },                                                            //   22nd Place
	{ "if08_whenmachsatt",  "wHen mAchines aTtack" },                                                   //   23rd Place
	{ "if08_thelucubrator", "The Lucubrator" },                                                         //   23rd Place
	{ "if08_freedom",       "Freedom" },                                                                //   30th Place
	{ "if08_riverside",     "Riverside" },                                                              //   31st Place
	{ "if08_thelighthouse", "The Lighthouse" },                                                         //   34th Place
	{ "if08_absworstgame",  "The Absolute Worst IF Game in History" },                                  //   35th Place
	{ "comp08",             "IF Competition '08 Unofficial FE Game" },

	// IFComp 2009
	{ "if09_snowquest",     "Snowquest" },                                                              //    3rd Place
	{ "if09_duelspanned",   "The Duel that Spanned the Ages" },                                         //    4th Place
	{ "if09_duelinsnow",    "The Duel in the Snow" },                                                   //    6th Place
	{ "if09_interface",     "Interface" },                                                              //    8th Place
	{ "if09_byzantinpersp", "Byzantine Perspective" },                                                  //    9th Place
	{ "if09_condemned",     "Condemned" },                                                              //   12th Place
	{ "if09_eruption",      "Eruption" },                                                               //   13th Place
	{ "if09_betatester",    "Beta Tester" },                                                            //   14th Place
	{ "if09_spelunkquest",  "Spelunker's Quest" },                                                      //   16th Place
	{ "if09_invisibleman",  "The Believable Adventures of an Invisible Man" },                          //   17th Place
	{ "if09_thegrandquest", "The Grand Quest" },                                                        //   18th Place
	{ "if09_starhunter",    "Star Hunter" },                                                            //   19th Place
	{ "if09_gatoron",       "GATOR-ON, Friend to Wetlands!" },                                          //   20th Place
	{ "if09_gleamingverb",  "Gleaming the Verb" },                                                      //   21st Place
	{ "if09_zorkburychaos", "Welcome to Zork, Buried Chaos" },                                          //   22nd Place
	{ "comp09",             "IF Competition '09 Unofficial FE Game" },

	// IFComp 2010
	{ "if10_deathofftc",    "Death off the Cuff" },                                                     //    5th Place
	{ "if10_mite",          "Mite" },                                                                   //    6th Place
	{ "if10_gloriousrevol", "The People's Glorious Revolutionary Text Adventure Game" },                //    7th Place
	{ "if10_hummingbird",   "Flight of the Hummingbird" },                                              //    8th Place
	{ "if10_warblersnest",  "The Warbler's Nest" },                                                     //    9th Place
	{ "if10_asgard",        "The 12:54 to Asgard" },                                                    //   17th Place
	{ "if10_penandpaint",   "Pen and Paint" },                                                          //   17th Place
	{ "if10_bibleretold2",  "The Bible Retold: The Lost Sheep" },                                       //   19th Place
	{ "if10_heated",        "Heated" },                                                                 //   20th Place
	{ "if10_ninjasfate",    "Ninja's Fate" },                                                           //   21th Place
	{ "if10_eastgrovehill", "East Grove Hills" },                                                       //   23rd Place
	{ "if10_quietevening",  "A Quiet Evening at Home" },                                                //   25th Place
	{ "if10_chronicler0",   "The Chronicler" },                                                         //   26th Place
	{ "comp10",             "IF Competition '10 Unofficial FE Game" },

	// IFComp 2011
	{ "if11_santaland",     "Escape From Santaland" },                                                  //    4th Place
	{ "if11_canamicah",     "Cana According to Micah" },                                                //    9th Place
	{ "if11_comedyerror",   "A Comedy of Error Messages" },                                             //   10th Place
	{ "if11_tenthplague",   "Tenth Plague" },                                                           //   12th Place
	{ "if11_coldiron",      "Cold Iron" },                                                              //   15th Place
	{ "if11_androawake",    "Andromeda Awakening" },                                                    //   17th Place
	{ "if11_keepsake",      "Keepsake" },                                                               //   18th Place
	{ "if11_theguardian",   "The Guardian" },                                                           //   21st Place
	{ "if11_lastdaysummer", "Last Day of Summer" },                                                     //   25th Place
	{ "if11_shipofwhimsy",  "Ship of Whimsy" },                                                         //   32th Place
	{ "if11_pfrank",        "Professor Frank" },                                                        //   34th Place
	{ "if11_vestiges",      "Vestiges" },                                                               //   38th Place
	{ "comp11",             "IF Competition '11 Unofficial FE Game" },

	// IFComp 2012
	{ "if12_eurydice",      "Eurydice" },                                                               //    2nd Place
	{ "if12_guildedyouth",  "Guilded Youth" },                                                          //    3rd Place
	{ "if12_changes",       "Changes" },                                                                //    4th Place
	{ "if12_sundayafterno", "Sunday Afternoon" },                                                       //    5th Place
	{ "if12_spiral",        "Spiral" },                                                                 //    6th Place
	{ "if12_bodybargain",   "Body Bargain" },                                                           //    8th Place
	{ "if12_fishbowl",      "Fish Bowl" },                                                              //   12th Place
	{ "if12_summerland",    "Escape From Summerland" },                                                 //   13th Place
	{ "if12_lunarbase1",    "Lunar Base 1" },                                                           //   14th Place
	{ "if12_murphyslaw",    "Murphy's Law" },                                                           //   15th Place
	{ "if12_testisready",   "The Test is Now READY" },                                                  //   15th Place
	{ "if12_castleadvent",  "Castle Adventure!" },                                                      //   23rd Place
	{ "comp12",             "IF Competition '12 Unofficial FE Game" },

	// IFComp 2013
	{ "if13_threediopolis", "Threediopolis" },                                                          //    7th Place
	{ "if13_paperbagprinc", "The Paper Bag Princess" },                                                 //   17th Place
	{ "if13_further_zcode", "Further" },                                                                //   21st Place
	{ "if13_9lives",        "9Lives" },                                                                 //   30th Place

	// IFComp 2014
	{ "if14_15minutes",     "Fifteen Minutes" },                                                        //    6th Place
	{ "if14_teaceremony",   "Tea Ceremony" },                                                           //   10th Place
	{ "if14_enigma_sd",     "Enigma (by Simon Deimel)" },                                               //   12th Place
	{ "if14_blacklily",     "The Black Lily" },                                                         //   16th Place
	{ "if14_tower",         "Tower" },                                                                  //   21th Place
	{ "if14_excelsior",     "Excelsior" },                                                              //   35th Place

	// IFComp 2015
	{ "if15_finalexam",     "Final Exam" },                                                             //    9th Place
	{ "if15_darkiss1",      "Darkiss! Wrath of the Vampire - Chapter 1: The Awakening" },               //   12th Place
	{ "if15_lifeonmars",    "Life on Mars?" },                                                          //   13th Place
	{ "if15_gbvb",          "Grandma Bethlinda's Variety Box" },                                        //   21st Place
	{ "if15_5minutes",      "5 Minutes To Burn Something!" },                                           //   32nd Place
	{ "if15_kingcrown",     "The King and the Crown" },                                                 //   42nd Place
	{ "if15_pitcondemned",  "Pit of the Condemned" },                                                   //   45th Place

	// IFComp 2016
	{ "if16_insidefacility", "Inside the Facility" },                                                   //   13th Place
	{ "if16_darkiss2",      "Darkiss! Wrath of the Vampire - Chapter 2: Journey to Hell" },             //   17th Place
	{ "if16_zigamus_zcode", "Zigamus: Zombies at Vigamus!" },                                            //   31th Place
	{ "if16_slickercity",   "Slicker City" },                                                           //   39th Place
	{ "if16_youarestand",   "You Are Standing in a Cave..." },                                          //   48th Place
	{ "if16_toiletworld",   "Toiletworld" },                                                            //   58th Place

	// IFComp 2017
	{ "if17_tuuli",         "Tuuli" },                                                                  //   16th Place
	{ "if17_thecubecavern", "The Cube in the Cavern" },                                                 //   39th Place
	{ "if17_squirrel",      "Goodbye Cruel Squirrel" },                                                 //   42th Place
	{ "if17_grue",          "Grue" },                                                                   //   54th Place
	{ "if17_richardmines",  "The Richard Mines" },                                                      //   55th Place
	{ "if17_onewayout",     "One Way Out" },                                                            //   56th Place
	{ "if17_inevitamp",     "Inevitable (by Matthew Pfeiffer)" },                                       //   60th Place
	{ "if17_walkinthepark", "A Walk in the Park" },                                                     //   68th Place

	// IFComp 2018
	{ "if18_tethered",      "Tethered" },                                                               //   23th Place

	// IFComp 2019
	{ "if19_out",           "Out" },                                                                    //   25th Place
	{ "if19_oldjim",        "Old Jim's Convenience Store" },                                            //   34th Place
	{ "if19_extromnivore",  "Extreme Omnivore: Text Edition" },                                         //   74th Place

	// IFComp 2020
	{ "if20_impossbottle",  "The Impossible Bottle" },                                                  //    1st Place
	{ "if20_entangled",     "Entangled" },                                                              //   26th Place
	{ "if20_drego",         "Dr Ego and the Egg of ManToomba" },                                        //   44th Place
	{ "if20_sheepcrossing", "Sheep Crossing" },                                                         //   88th Place

	// IFComp 2021
	{ "if21_darkun",        "D'Arkun" },                                                                //   21st Place
	{ "if21_codexsadistic", "Codex Sadistica: A Heavy-Metal Minigame" },                                //   41st Place
	{ "if21_fourbyfourian", "Fourbyfourian Quarryin'" },                                                //   45th Place
	{ "if21_bravebear",     "Brave Bear" },                                                             //   46th Place

	// IFComp 2022
	{ "if22_intosun",       "Into the Sun" },                                                           //   25th Place
	{ "if22_lowkey",        "Prime Pro-Rhyme Row #3: Low-Key Learny Jokey Journey" },                   //   41st Place
	{ "if22_campusinvader", "Campus Invaders" },                                                        //   45th Place
	{ "if22_zerochance",    "Zero Chance of Recovery" },                                                //   52nd Place

	// IFComp 2023
	{ "if23_bbkk_zcode",    "Bright Brave Knight Knave" },                                             //    36th Place
	{ "if23_h2",            "Milliways: The Restaurant at the End of the Universe" },                   //   52nd Place
	{ "if23_thewitch",      "The Witch" },                                                              //   66nd Place

	// Spring IF Art Show 1999
	{ "ifas_tpold",         "The Possibility of Life's Destruction" },

	// Summer IF Art Show 1999
	{ "ifas_statuedac",     "Statue" },
	{ "ifas_wheel",         "Wheel" },

	// IF Art Show 2000
	{ "ifas_custard",       "Custard" },
	{ "ifas_galatea",       "Galatea" },
	{ "ifas_guitar",        "Guitar of the Immortal Bard" },
	{ "ifas_sparky",        "Sparky and Boots" },
	{ "ifas_cove",          "The Cove" },
	{ "ifas_statuette",     "The Statuette" },
	{ "ifas_visitor",       "The Visitor" },
	{ "ifas_words",         "Words Get Twisted Round and Tumble Down" },

	// IF Art Show 2001
	{ "ifas_esgarden",      "English Suburban Garden" },
	{ "ifas_lagoon",        "La Lagune de Montaigne" },
	{ "ifas_ribbons",       "Ribbons" },

	// IF Art Show 2003
	{ "ifas_stopnight",     "A Stop for the Night" },
	{ "ifas_queen",         "Queen of Swords" },
	{ "ifas_redeem",        "Redemption" },

	// IF Art Show 2004
	{ "ifas_flametop",      "Flametop" },
	{ "ifas_swanglass",     "Swanglass" },
	{ "ifas_firetower",     "The Fire Tower" },

	// IF Art Show 2007
	{ "ifas_rendition",     "Rendition" },
	{ "ifas_engine",        "The Symbolic Engine" },
	{ "ifas_varronismuseu", "Varronis Museum" },

	// The Mystery House Taken Over project
	{ "mhto1mhp",           "Mystery House Taken Over Ep. 1: House Possessed" },
	{ "mhto2shiovitz",      "Mystery House Taken Over Ep. 2: Where There's a Will" },

	// Arcade Collection
	{ "ifa_rox",            "Rox" },                                                                    //   Episode  1
	{ "ifa_centipede",      "Centipede" },                                                              //   Episode  2
	{ "ifa_donkeykong",     "Donkey Kong" },                                                            //   Episode  4
	{ "ifa_driver",         "Night Driver" },                                                           //   Episode  5
	{ "ifa_galaxian",       "Galaxian" },                                                               //   Episode  6
	{ "ifa_invaders",       "Invaders" },                                                               //   Episode  7
	{ "ifa_joust",          "Joust" },                                                                  //   Episode  9
	{ "ifa_loderunner",     "Lode Runner" },                                                            //   Episode 10
	{ "ifa_marble",         "Marble Madness" },                                                         //   Episode 11
	{ "ifa_pacman",         "Pac-Man" },                                                                //   Episode 12

	// Casual Gameplay Design Competition #7
	{ "cgdc7_hoosegow",     "Hoosegow" },                                                               //    1st Place
	{ "cgdc7_fragileshell", "Fragile Shells" },                                                         //    2nd Place
	{ "cgdc7_dual",         "Dual Transform" },                                                         //    3rd Place
	{ "cgdc7_partyfoul",    "Party Foul" },                                                             //    4th Place
	{ "cgdc7_roofed",       "Roofed" },                                                                 //    5th Place
	{ "cgdc7_ka",           "Ka" },                                                                     //    6th Place
	{ "cgdc7_monday",       "Monday, 16:30" },                                                          //    7th Place
	{ "cgdc7_theusher",     "The Usher" },                                                              //    8th Place
	{ "cgdc7_intoopensky",  "Into the Open Sky" },                                                      //    9th Place
	{ "cgdc7_luriddreams",  "Lurid Dreaming" },                                                         //   10th Place
	{ "cgdc7_containment",  "Containment" },                                                            //   11th Place
	{ "cgdc7_thecube",      "The Cube" },                                                               //   12th Place
	{ "cgdc7_blueprint",    "The Blueprint" },                                                          //   13th Place
	{ "cgdc7_critbreach",   "Critical Breach" },                                                        //   14th Place
	{ "cgdc7_manorwhitby",  "The Manor at Whitby" },                                                    //   15th Place
	{ "cgdc7_heavenly",     "Heavenly" },                                                               //   16th Place
	{ "cgdc7_expecttodie",  "I Expect You to Die" },                                                    //   17th Place
	{ "cgdc7_virtuality",   "Virtuality" },                                                             //   18th Place
	{ "cgdc7_dramaqueen",   "Drama Queen 7 - Mother Knows Best" },                                      //   19th Place
	{ "cgdc7_terminal",     "Terminal" },                                                               //   20th Place
	{ "cgdc7_couchofdoom",  "Couch of Doom" },                                                          //   21st Place
	{ "cgdc7_basictrain",   "Basic Train-ing" },                                                        //   22nd Place
	{ "cgdc7_paint",        "Paint" },                                                                  //   23rd Place
	{ "cgdc7_goldenshadow", "Golden Shadow" },                                                          //   24th Place
	{ "cgdc7_escapeindark", "Escape in the Dark" },                                                     //   25th Place
	{ "cgdc7_escapefict",   "Escape Into Fiction" },                                                    //   26th Place
	{ "cgdc7_zeroeth",      "A Zeroeth Dimension" },                                                    //   27th Place
	{ "cgdc7_openfield",    "An Open Field" },                                                          //   28th Place
	{ "cgdc7_survive",      "Survive" },                                                                //   29th Place
	{ "cgdc7_zegro",        "Zegrothenus" },                                                            //   30th Place

	// Chicken Competition
	{ "cc_chknmt",          "Are You Too Chicken to Make a Deal?" },
	{ "cc_behavior",        "Behavior" },
	{ "cc_chicken",         "Chicken!" },
	{ "cc_chickenegg",      "Chicken and Egg" },
	{ "cc_chickenunder",    "The Chicken Under the Window" },
	{ "cc_chickendist",     "Chickens of Distinction" },
	{ "cc_freerange",       "Hey, I'm Supposed to be Free Range" },
	{ "cc_lessonchicken",   "The Lesson of the Chicken" },
	{ "cc_aboutchicken",    "The One About the Chicken, the Lion and the Monkey?" },
	{ "cc_orpington",       "Orpington" },
	{ "cc_pollocamino",     "Pollo y Camino" },
	{ "cc_saied",           "Saied" },
	{ "cc_xchicken",        "The X Chicken" },

	// Commodore 32 Z-Machine Competition
	{ "c32_endgame",        "Endgame" },                                                                //    1st Place
	{ "c32_turningpoint",   "Turning Point" },                                                          //    2nd Place
	{ "c32_amusementpark",  "Amusement Park" },                                                         //    3rd Place
	{ "c32_downtowntrain",  "Downtown Train" },                                                         //    4th Place
	{ "c32_zombiescl",      "Zombies!" },                                                               //    5th Place
	{ "c32_paparazzi",      "Paparazzi" },                                                              //    6th Place
	{ "c32",                "C32 Contest 2004" },

	// Cover Stories Game-Jam
	{ "cs_antifascista",    "Antifascista" },
	{ "cs_leaves",          "Leaves" },
	{ "cs_legendmisshat",   "The Legend of the Missing Hat" },
	{ "cs_monkeybusiness",  "Monkey Business" },
	{ "cs_offering",        "Offering" },
	{ "cs_sloth",           "Sloth on a Stroller" },
	{ "cs_subtropservroom", "The Subtropical Server Room" },

	// Dinosaur Mini-Competition
	{ "dino_rowr",          "Rowr!" },
	{ "dino_dinoroad",      "Why Did the Dino Cross the Road?" },
	{ "dino_d2d",           "d2d: A Dinosaur Minicom Game" },
	{ "dino_dinohunt",      "Dino Hunt" },
	{ "dino_stegosaur",     "A Stegosaur's Night Out" },
	{ "dino_adno",          "A Dino's Night Out" },
	{ "dino_olddog",        "Very Old Dog" },
	{ "dino_revenger",      "Revenger" },
	{ "dino_dinnertime",    "Dinosaur Dinnertime!" },
	{ "dino_appoint",       "An Important Appointment" },

	// Dragon Mini-Competition
	{ "dr_atbottom",        "At the Bottom of the Garden" },
	{ "dr_damsel",          "The Damsel and the Dragon" },
	{ "dr_dragongeorge",    "Dragon George and The Man" },
	{ "dr_atdragon",        "Dragon!" },
	{ "dr_herebe",          "Here Be Dragons!!!" },
	{ "dr_lastlaugh",       "The Last Laugh" },

	// Ectocomp 2011
	{ "ec11_bloodless",     "Bloodless on the Orient Express" },                                        //    1st Place
	{ "ec11_blue",          "Blue" },                                                                   //    3rd Place
	{ "ec11_dashslapney",   "Dash Slapney, Patrol Leader" },                                            //    5th Place

	// Ectocomp 2012
	{ "ec12_ghosternight",  "Ghosterington Night" },                                                    //    1st Place
	{ "ec12_littlegirls",   "What Are Little Girls Made Of" },                                          //    2nd Place
	{ "ec12_parasites",     "Parasites" },                                                              //    4th Place

	// Ectocomp 2013
	{ "ec13_horpyr_zcode",  "The Horrible Pyramid" },                                                   //    1st Place
	{ "ec13_headleshaples", "Headless, Hapless" },                                                      //    3rd Place
	{ "ec13_faithfulcomp",  "Faithful Companion" },                                                     //    4th Place
	{ "ec13_deadpavane",    "Dead Pavane for a Princess" },                                             //    5th Place
	{ "ec13_icehouse",      "Ice House of Horrors" },                                                   //    6th Place
	{ "ec13_blackness",     "Blackness" },                                                              //    9th Place
	{ "ec13_argument",      "The Argument-Winner's Ghost" },                                            //   11th Place
	{ "ec13_fishdreams",    "Fish Dreams" },                                                            //   13th Place
	{ "ec13_cursedeagle",   "The Tale of the Cursed Eagle" },                                           //   14th Place
	{ "ec13_cenriccurse",   "The Cenric Family Curse" },                                                //   15th Place
	{ "ec13_nessaslasher",  "The Nessa Springs Slasher" },                                              //   18th Place
	{ "ec13_cratercreek",   "Crater Creek" },                                                           //   19th Place
	{ "ec13_hillofsouls",   "Hill of Souls" },                                                          //   22nd Place
	{ "ec13_wisp",          "Wisp" },                                                                   //   23th Place

	// Ectocomp 2014
	{ "ec14_weddingday",    "Wedding Day" },                                                            //    5th Place
	{ "ec14_limeergot",     "Lime Ergot" },                                                             //    6th Place
	{ "ec14_candyrushsaga", "Candy Rush Saga" },                                                        //    8th Place
	{ "ec14_flywall",       "A Fly on the Wall" },                                                      //   11th Place
	{ "ec14_monstermaker",  "Monster Maker" },                                                          //   16th Place

	// Ectocomp 2015 - La Petite Mort
	{ "ec15_openthatvein",  "Open That Vein" },                                                         //    1st Place
	{ "ec15_oldhangover",   "The Oldest Hangover on Earth" },                                           //    2nd Place
	{ "ec15_heezypark",     "Heezy Park" },                                                             //    3rd Place
	{ "ec15_storyshinoboo", "The Story of the Shinoboo" },                                              //    4th Place
	{ "ec15_ghostship",     "The Ghost Ship" },                                                         //    7th Place
	{ "ec15_physiognomist", "The Physiognomist's Office" },                                             //    8th Place

	// Ectocomp 2015 - Le Grand Guignol
	{ "ec15_ninelives",     "Nine Lives" },                                                             //    3rd Place

	// Ectocomp 2016 - La Petite Mort
	{ "ec16_lightdarkness", "Light into Darkness" },                                                    //    4th Place
	{ "ec16_bonesaw",       "The Unstoppable Vengeance of Doctor Bonesaw" },                            //    6th Place
	{ "ec16_checkerhaunt",  "A Checkered Haunting" },                                                   //   10th Place

	// Ectocomp 2017 - La Petite Mort
	{ "ec17_primer",        "Primer" },                                                                 //    1st Place
	{ "ec17_dreamcorrupt",  "Corrupter of Dreams" },                                                    //    4th Place
	{ "ec17_uxmulbrufyuz",  "Uxmulbrufyuz" },                                                           //   10th Place
	{ "ec17_civilmimic",    "Civil Mimic" },                                                            //   12th Place

	// Ectocomp 2018 - La Petite Mort
	{ "ec18_moongoon",      "Moon Goon" },                                                              //       Winner
	{ "ec18_wakeup_zcode",  "Wake Up" },                                                                //      Entrant
	{ "ec18_whoahcubswoe",  "Whoah Cubs Woe" },                                                         //      Entrant

	// Ectocomp 2018 - Le Grand Guignol
	{ "ec18_plshel_zcode",  "Please Help Me" },                                                         //      Entrant
	{ "ec18_walkamongus",   "Walk Among Us" },                                                          //      Entrant

	// Ectocomp 2019 - La Petite Mort
	{ "ec19_quitequeer",    "Prime Pro-Rhyme Row #2: Quite Queer Night Near" },                         //    9th Place

	// Ectocomp 2019 - Le Grand Guignol
	{ "ec19_onceuponwint",  "Once Upon a Winter Night, the Ragman Came Singing Under Your Window" },    //    9th Place

	// Ectocomp 2020 - La Petite Mort
	{ "ec20_fracture",      "Fracture" },                                                               //    8th Place

	// Ectocomp 2021 - La Petite Mort
	{ "ec21_wearyeerie",    "Weary Eerie Way" },
	{ "ec21_psyops",        "Psyops, Yo" },

	// Ectocomp 2022 - La Petite Mort
	{ "ec22_enigmamanor",   "The Enigma of the Old Manor House" },                                      //    1st Place
	{ "ec22_thosedare",     "Prime Pro-Rhyme Row #5: There Those Dare Doze" },                          //   17th Place

	// Ectocomp 2022 - Le Grand Guignol
	{ "ec22_civil",         "Prime Pro-Rhyme Row #4: Civil Seeming Drivel Dreaming" },                  //   15th Place

	// Ectocomp 2023 - La Petite Mort
	{ "ec23_tallertech",    "Taller Tech Mauler Mech" },                                                //   19th Place

	// EnvComp
	{ "ec_laseine",         "La Seine" },

	// GameplayComp 2009
	{ "gc_fantafinaleiv",   "Fantastic Finale IV" },

	// Ghost Town Redux
	{ "gtr_ghosttowntlt",   "Ghost Town: The Lost Treasure" },

	// HighlandComp
	{ "hc_highland",        "Highland Chef" },
	{ "hc_roots",           "Roots" },

	// 24 Hours of Inform Competition
	{ "24h_aesthetic",      "Aesthetic Deletions" },
	{ "24h_dastardly",      "Dastardly" },
	{ "24h_defra",          "The Man From DEFRA" },
	{ "24h_borgaris",       "Escape From the SS Borgaris" },
	{ "24h_ghostship",      "Ghost Ship" },

	// IFBeginnersComp 2008
	{ "ifb_connect",        "Connect" },
	{ "ifb_limelight",      "Limelight" },

	// IF Fan Fest 1998
	{ "ff_ite2",            "In the End II" },

	// Imaginary Games From Imaginary Universes Jam
	{ "ig_garbage",         "Garbage Collection" },

	// IntroComp 2002
	{ "ic02_waterhouse",    "The Waterhouse Women" },                                                   // 3° Runner-Up
	{ "ic02_timetrap",      "TimeTrap" },                                                               //      Entrant

	// IntroComp 2003
	{ "ic03_statue",        "The Mage Wars: Statue" },                                                  //       Winner
	{ "ic03_realend",       "Reality's End" },                                                          // 1° Runner-Up
	{ "ic03_agency",        "Agency" },                                                                 // 2° Runner-Up
	{ "ic03_harlequin",     "Harlequin Girl" },                                                         //      Entrant
	{ "ic03_harringthouse", "Harrington House" },                                                       //      Entrant

	// IntroComp 2004
	{ "ic04_jbbrwkyi",      "Intro to Jabberwocky" },                                                   //    1st Place
	{ "ic04_auden",         "Auden's Eden" },                                                           // 1° Runner-Up
	{ "ic04_passenger",     "Passenger" },                                                              // 2° Runner-Up
	{ "ic04_cross",         "On the Cross" },                                                           //      Entrant
	{ "ic04_runes",         "Runes" },                                                                  //      Entrant

	// IntroComp 2005
	{ "ic05_deadsville",    "Deadsville" },                                                             //    1st Place
	{ "ic05_wscholars",     "Weishaupt Scholars" },                                                     // 1° Runner-Up
	{ "ic05_stale",         "The Fox, The Dragon, and the Stale Loaf of Bread" },                       // 2° Runner-Up
	{ "ic05_hobbit",        "The Hobbit" },                                                             //      Entrant
	{ "ic05_somewhen",      "Somewhen" },                                                               //      Entrant

	// IntroComp 2006
	{ "ic06_southerngoth",  "Southern Gothic" },                                                        //       Winner
	{ "ic06_childsplay",    "Child's Play" },                                                           // 1° Runner-Up
	{ "ic06_artdeception",  "The Art of Deception" },                                                   //      Entrant
	{ "ic06_mechs",         "Mechs" },                                                                  // Hon. Mention
	{ "ic06_sabotage",      "Sabotage" },                                                               // Hon. Mention
	{ "ic06_ufury",         "Unyielding Fury" },                                                        // Hon. Mention

	// IntroComp 2007
	{ "ic07_shredspatches", "The King of Shreds and Patches" },                                         //       Winner
	{ "ic07_jacobst",       "Jacob's Travels" },                                                        // 2° Runner-Up
	{ "ic07_jackbox",       "Jack in the Box" },                                                        // Hon. Mention
	{ "ic07_trainstopping", "Trainstopping" },                                                          // Hon. Mention

	// IntroComp 2008
	{ "ic08_stormcellar",   "Storm Cellar" },                                                           // 1° Runner-Up
	{ "ic08_bedtimestory",  "Bedtime Story" },                                                          // Hon. Mention
	{ "ic08_fiendishzoo",   "Fiendish Zoo" },                                                           // Hon. Mention
	{ "ic08_ninetenths",    "Nine-tenths of the Law" },                                                 // Hon. Mention

	// IntroComp 2009
	{ "ic09_obituary",      "Obituary" },                                                               //       Winner
	{ "ic09_gossip",        "Gossip" },                                                                 // 1° Runner-Up
	{ "ic09_selves",        "Selves" },                                                                 // 2° Runner-Up

	// IntroComp 2010
	{ "ic10_touristtrap",   "Tourist Trap" },                                                           //       Winner
	{ "ic10_moratori",      "A Fleeting Case of Self-Possession, or, Memento Moratori" },               // 1° Runner-Up
	{ "ic10_plan6",         "Plan 6 From Inner Earth" },                                                // 2° Runner-Up
	{ "ic10_waker",         "Waker" },                                                                  // Hon. Mention

	// IntroComp 2011
	{ "ic11_sfiction",      "Speculative Fiction: Beginner's Lessons" },                                //    2nd Place
	{ "ic11_bender",        "Bender" },                                                                 // Hon. Mention
	{ "ic11_despondenidx",  "The Despondency Index" },                                                  // Hon. Mention
	{ "ic11_parthenon",     "Parthenon" },                                                              // Hon. Mention
	{ "ic11_stallingtime",  "Stalling for Time" },                                                      // Hon. Mention

	// IntroComp 2013
	{ "ic13_chickensexer",  "The Example of the Chicken Sexer" },                                       // Hon. Mention

	// IntroComp 2014
	{ "ic14_talesoulthief", "Tales of the Soul Thief" },                                                // Hon. Mention

	// IntroComp 2016
	{ "ic16_grubbyville",   "Grubbyville" },                                                            //    3nd Place

	// IntroComp 2018
	{ "ic18_napier",        "Napier’s Cache" },                                                         // Hon. Mention

	// IntroComp 2019
	{ "ic19_homeland",      "Homeland" },                                                               // Hon. Mention

	 // IntroComp 2020
	{ "ic20_navigatio",     "Navigatio" },                                                              //    3rd Place

	// IF Library Competition 2003
	{ "lc_serve",           "Serving Your Country" },
	{ "lc_trappedschool",   "Trapped in School" },

	// IF Logic Puzzle Mini-Competition
	{ "lp_logicpuz",        "Logic Puzzle Sampler" },
	{ "lp_traffic",         "The Traffic Light" },

	// LOTECH Competition 2001
	{ "lo01_doghouse",      "The Dog/House" },

	// LOTECH Competition 2005
	{ "lo05_stormy",        "A Dark and Stormy Entry" },                                                //    2nd Place
	{ "lo05_knapsack",      "The Knapsack Problem" },                                                   //    6th Place

	// Manos Mini-Competition
	{ "ma_manoz",           "ManoZ" },

	// MCDream Competition
	{ "mc_dreadwine",       "Dreadwine" },
	{ "mc_nofamous",        "No Famous" },
	{ "mc_retreat",         "The Retreat" },

	// Metafilter IF Competition 2009
	{ "me09_bunt",          "Bunt" },
	{ "me09_downout",       "Down and Out at the Big Creepy House on the Poison Lake" },
	{ "me09_underpig",      "Down With the Underpig" },
	{ "me09_ericsbender",   "Eric's Bender" },
	{ "me09_jeffrey",       "Jeffrey the Homicidal Mimic" },
	{ "me09_burndown",      "Let Us Burn Down Science" },
	{ "me09_rockrider",     "Rockrider" },

	// Metafilter IF Competition 2010
	{ "me10_greatrent",     "The Great Rent Money Adventure" },

	// IF Mini-Competition 1998
	{ "mc98_bloodline",     "Bloodline" },
	{ "mc98_inherit",       "Inherit!" },
	{ "mc98_sumbunny",      "Something About the Bunny" },

	// Minigames Minicomp
	{ "mm_hamhall",         "Hamilton Hall" },
	{ "mm_insider",         "Insider Information" },

	// 9th Annual New Year's Mincomp
	{ "ny09_egyptianwalk",  "Egyptian Walking Simulator" },

	// 11th Annual New Year's Mincomp
	{ "ny11_halflife",      "Half-Life 3 Confirmed" },

	// No-Inventory-Competition 1999
	{ "ni_annoyedundead",   "Annoyed Undead" },
	{ "ni_camping",         "Camping" },

	// One Room Game Competition 2002
	{ "1r02_artif",         "L'Artificiere" },
	{ "1r02_sottogiallo",   "Il Sottomarino Giallo" },

	// One Room Game Competition 2003
	{ "1r03_floppy",        "Due Dischetti, Un Incubo" },
	{ "1r03_successo",      "Successo: Non Aprite Quella Porta!" },

	// One Room Game Competition 2006
	{ "1r06_finalselect",   "Final Selection" },
	{ "1r06_sforacchiato",  "Lo Sforacchiato Giallo" },
	{ "1r06_devilatvenice", "Il Diavolo a Venezia" },
	{ "1r06_galeotto",      "Galeotto Fu il Canotto" },
	{ "1r06_easter",        "It's Easter, Peeps!" },
	{ "1r06_lazyjonescrst", "Lazy Jones e l’Ultima Crostata" },
	{ "1r06_dereditu",      "De Reditu" },
	{ "1r06_frankenstein3", "Frankenstein III" },

	// One Room Game Competition 2007
	{ "1r07_dreimp",        "A Dream Imprint" },
	{ "1r07_kinesis",       "Kinesis" },
	{ "1r07_necromante",    "Il Necromante" },
	{ "1r07_suvehnux",      "Suveh Nux" },
	{ "1r07_urbanconflict", "Urban Conflict" },

	// RomanceNovelComp 2002
	{ "rnc_chooseromance",  "Choose Your Own Romance" },

	// Segment Mini-Comp
	{ "snowmansp2",         "Snowman Sextet Part II" },
	{ "kaboot",             "Kaboot's Story" },
	{ "fandbsnowman",       "Fran and Bart Want a Snowman!" },

	// Shufflecomp Competition 2014
	{ "sh14_50shades",      "50 Shades of Jilting" },
	{ "sh14_flotsam",       "Flotsam & Driftwood" },
	{ "sh14_robotempire",   "Holy Robot Empire" },
	{ "sh14_illuminate",    "Illuminate" },
	{ "sh14_lobsterbucket", "Lobster Bucket" },
	{ "sh14_monkeyandbear", "Monkey and Bear" },
	{ "sh14_more",          "More" },
	{ "sh14_teaandtoast",   "Tea and Toast" },
	{ "sh14_truth",         "Truth" },
	{ "sh14_whitehouses",   "White Houses" },

	// ShuffleComp: Disc 2 Competition
	{ "shd_everythinggame", "Everything We Do Is Games" },
	{ "shd_headingeast",    "Heading East" },
	{ "shd_seeksorrow",     "Starry Seeksorrow" },

	// SmoochieComp 2001
	{ "sc_1981",            "1981" },
	{ "sc_august",          "August" },
	{ "sc_deadofwinter",    "Dead of Winter" },
	{ "sc_pytho",           "Pytho's Mask" },
	{ "sc_sparrow",         "Sparrow's Song" },
	{ "sc_bandit",          "The Tale of the Kissing Bandit" },
	{ "sc_voices",          "Voices" },

	// SpeedIF 1 Competition
	{ "sif_coffins",        "Coffins" },

	// SpeedIF 2 Competition
	{ "sif_speedif",        "SpeedIF #2" },
	{ "sif_hallhand",       "The Hand That Rocks the Pumpkin" },
	{ "sif_pumpkin",        "The Pumpkin" },

	// SpeedIF 3 Competition
	{ "sif_boygoat",        "A Boy and His Goat" },
	{ "sif_pryde",          "Pryde and the Pink Flamingo" },
	{ "sif_devildoit",      "The Devil Made Me Do It" },
	{ "sif_3steps",         "Three Steps to the Left" },

	// SpeedIF 4 Competition
	{ "sif_oniondig",       "Digging for Onions" },
	{ "sif_firstwave",      "FIRST WAVE, Then Jump Up and Down Screaming" },
	{ "sif_onion4",         "Nostradamus’s Onion Sandwich" },
	{ "sif_onnoef",         "OnNoEf" },
	{ "sif_tearsmayfall",   "Tears May Fall" },
	{ "sif_profesee",       "The Profesee" },

	// SpeedIF 5 Competition
	{ "sif_doomsday",       "Doomsday" },
	{ "sif_chalupa",        "Revenge of the Chalupa" },
	{ "sif_courier",        "The Courier Who Missed Me" },
	{ "sif_pspy5",          "The Spy Who Always Wears Gloves Now" },

	// SpeedIF 5.5 Competition
	{ "sif_carnival",       "Carnival" },
	{ "sif_lake",           "The Lake" },

	// SpeedIF 6 Competition
	{ "sif_kids",           "Kids Shouldn't Have to Save the World" },
	{ "sif_atlantic",       "One Night in the North Atlantic!" },
	{ "sif_speedif6",       "SpeedIF6: A Top Hat for Eddie" },
	{ "sif_titanic",        "Titanic: Leo's Revenge" },
	{ "sif_whengrow",       "When I Grow Up I Want to Be a Firetruck" },

	// SpeedIF 7 Competition
	{ "sif_lilyjane",       "Sinking the Lily Jane" },
	{ "sif_tankweb",        "Tanker and Webb" },
	{ "sif_tears",          "Tears Keep Getting in My Dr. Pepper" },
	{ "sif_oilydeeps",      "The Oily Deeps" },

	// SpeedIF 8 Competition
	{ "sif_hangers",        "A Freak Accident Leaves Seattle Pantsless" },
	{ "sif_seattle3",       "A Freak Accident Leaves Seattle Pantsless III: Endgame" },
	{ "sif_pantsless",      "Pantsless in Seattle" },

	// SpeedIF 9 Competition
	{ "sif_ucodarkness",    "Under Cover of Darkness" },

	// SpeedIF 10 Competition
	{ "sif_deathx4",        "Death Death Death Death" },
	{ "sif_taxes",          "Taxes" },

	// SpeedIF 10^-9 Competition
	{ "sif_krakatoa",       "Krakatoa Tuna Melt" },
	{ "sif_roadestruction", "The Road to Destruction (featuring Bob Hope)" },

	// SpeedIF 11 Competition
	{ "sif_spiders",        "EPISODE 2: Revenge of the Mutant Spiders" },
	{ "sif_fido",           "Fido and the Dead Body" },

	// SpeedIF 12 Competition
	{ "sif_pantheon",        "Pantheon, Party On" },
	{ "sif_plaque",          "Plaque" },

	// SpeedIF 13 Competition
	{ "sif_elephantaflife", "Elephants and the Afterlife" },
	{ "sif_garden",         "Garden of the Dragon" },
	{ "sif_inspector",      "Health Inspector" },
	{ "sif_potstick",       "Potsticker" },
	{ "sif_service",        "Service With a Smile" },

	// SpeedIF 14 Competition
	{ "sif_halloween",      "Hallowe'en" },
	{ "sif_bbp",            "The Blair Bee Project" },
	{ "sif_pcontest",       "The Pumpkin Contest" },
	{ "sif_worm",           "Worm" },

	// SpeedIF 15 Competition
	{ "sif_assignment",     "Assignment" },
	{ "sif_basketdestiny",  "Basket of Destiny" },
	{ "sif_broken",         "Help! My Vacuum Cleaner Is Broken" },
	{ "sif_butler",         "The Unfortunate Training of Frank Lee, Monkey Butler to Be" },

	// SpeedIF 16 Competition
	{ "sif_agent",          "Secret Agent" },
	{ "sif_marie",          "The Last Sonnet of Marie Antoinette" },

	// SpeedIF 17 Competition
	{ "sif_2604",           "2604" },
	{ "sif_roof",           "A Stroll on the Roof" },
	{ "sif_123102",         "December 31, 2002" },
	{ "sif_speedif17",      "Tooth Ow Zunden Won!" },

	// SpeedIF 18 Competition
	{ "sif_fishspaceships", "Fish and Spaceships" },
	{ "sif_glossary",       "Glossary" },

	// SpeedIF 19 Competition
	{ "sif_98769765",       "98769765" },
	{ "sif_cheerup",        "Cheer Up" },
	{ "sif_darksoul",       "Dark Soul" },
	{ "sif_falling",        "Falling Angel" },
	{ "sif_speedif19",      "SpeedIF 19" },
	{ "sif_angelcurse",     "The Angel Curse" },
	{ "sif_upon",           "Upwards and Onwards" },

	// SpeedIF 20 Competition
	{ "sif_lrc",            "Llangollen Rock City" },
	{ "sif_sellout",        "Sell-Out" },

	// SpeedIF -1 Competition
	{ "sif_discord",        "Discord" },
	{ "sif_generic",        "Generic Title" },
	{ "sif_hankbuzzcrack",  "Hank Buzzcrack Has a Job to Do, God Damn It" },
	{ "sif_literacy",       "Literacy" },
	{ "sif_woodencat",      "WOODEN CAT vs. ROBOT MONKEY" },

	// SpeedIF 2000 Competition
	{ "sif_ritual",         "The Great Ritual" },

	// SpeedIF 2001 Competition
	{ "sif_battleplanets",  "Battle of the Planets" },
	{ "sif_destinychihuah", "Destiny of the Chihuahua" },
	{ "sif_infiltrt",       "Infiltration on Io" },
	{ "sif_inspiration",    "Inspiration" },
	{ "sif_moonjupiter",    "Moon Over Jupiter" },
	{ "sif_crescent",       "The Crescent City at the Edge of Disaster" },
	{ "sif_crouton",        "The Crouton Caper" },
	{ "sif_speedif17v1",    "Tooth Ow Zunden Won!" },

	// SpeedIF 3rd Anniversary Competition
	{ "sif_henry",          "Finding Henry" },

	// SpeedIF 5th Anniversary Competition
	{ "sif_coughcough",     "**COUGH COUGH**" },

	// SpeedIF 11th Anniversary Competition
	{ "sif_nqaa",           "Not Quite an Anniversary" },
	{ "sif_reptile",        "Reptile" },
	{ "sif_teachher2dance", "Teaching Her to Dance" },

	// SpeedIF 18th Anniversary Competition
	{ "sif_farout",         "Far-Out Space Freaks" },

	// SpeedIF Argonaut Competition
	{ "sif_asciargo",       "ASCII and the Argonauts" },
	{ "sif_fleece",         "Jason Finds Fleece" },
	{ "sif_invisargo",      "The Invisible Argonaut" },

	// SpeedIF Autocratic Competition
	{ "sif_toeing",         "Toeing the Line" },

	// SpeedIF Century Competition
	{ "sif_mission",        "Mission From Short" },
	{ "sif_yaygames",       "Yay Games" },

	// SpeedIF Copyright Competition
	{ "sif_piratescaribou", "Pirates of the Caribou" },
	{ "sif_sharingallway",  "Sharing All the Way to the Bank" },

	// SpeedIF Crinkle Cut Competition
	{ "sif_travels",        "The Travels of Fitzwilliam Pound" },

	// SpeedIF Douglas Adams Tribute Competition
	{ "sif_nightmilliways", "A Night at Milliways" },
	{ "sif_deadlinebda",    "Deadline, or, Being Douglas Adams" },
	{ "sif_howmany",        "How Many Roads Must a Man Walk Down?" },
	{ "sif_minds",          "The Death of Two Great Minds" },
	{ "sif_sofa",           "The Sofa at the End of the Universe" },

	// SpeedIF EXTREME Competition
	{ "sif_andthen",        "And Then is Heard no More" },
	{ "sif_oth",            "Oth." },
	{ "sif_rjd",            "Romeo, Juliet and the Dog" },
	{ "sif_tightest",       "Tightest" },

	// SpeedIF Gruff Competition
	{ "sif_bronze",         "Bronze" },
	{ "sif_breath",         "Deep Breathing" },
	{ "sif_green",          "Little Green Robbing Hood" },
	{ "sif_gameiwrote",     "This Is the Game That I Wrote" },

	// SpeedIF Halloween 2002 Competition
	{ "sif_losing",         "Losing Your Step" },
	{ "sif_oldsherwoodcem", "The Old Sherwood Cemetary" },

	// SpeedIF Indigo New Language Competition
	{ "sif_hauntedhouse",    "Haunted House" },

	// SpeedIF Introcomp
	{ "sif_holygoat",       "AFGNCAAP - IF Agent! Todays Adventure: Holy Goat!" },
	{ "sif_alcohol",        "Alcohol Solves Everything" },
	{ "sif_djibouti",       "Djibouti Dirigible Discombobulation" },
	{ "sif_faetttiw",       "Fætt Tiw" },
	{ "sif_velocitasficts", "In Search of Velocitas Fictus" },
	{ "sif_baptist",        "The Twelve Heads of St. John the Baptist" },
	{ "sif_zefrench",       "Ze French Countryside Is Full of Fresh Air" },

	// SpeedIF Jacket Competition
	{ "sif_shangri",        "Shangri La" },
	{ "sif_soulsrch",       "Soul-Searching" },
	{ "sif_circussadness",  "The Circus of Sadness" },
	{ "sif_count",          "The Count of Monte Cristo" },
	{ "sif_sewage",         "Toxic Sewage (A (Love) Story)" },

	// SpeedIF Jacket 2 Competition
	{ "sif_beinglittleguy", "Being the Little Guy" },
	{ "sif_pirateninja",    "Pirates and Ninjas and Aliens, Oh My!" },
	{ "sif_body",           "The Body" },
	{ "sif_gwdmpete",       "The President, The Democrats, and Smelly Pete" },
	{ "sif_towerofbeef",    "The Tower of Beef" },
	{ "sif_travel",         "Vacationing in Scotland" },
	{ "sif_whatdreams",     "What Dreams May Come" },
	{ "sif_wormwood",       "Wormwood Days II: The Aftermath" },

	// SpeedIF Jacket 3 Competition
	{ "sif_fluid",          "The Fluid of Life" },
	{ "sif_hentai",         "The Hentai Adventures of Captain Cumshot" },
	{ "sif_upwards",        "Upwards" },

	// SpeedIF Jacket 4 Competition
	{ "sif_moondarkling",   "Moondarkling: Elfboon" },
	{ "sif_lachryma",       "The Forests of Lachryma" },
	{ "sif_gorilla",        "The Man-Eating, Halitosic Gorilla of Brazil" },
	{ "sif_oceantower",     "Love, Hate and the Mysterious Ocean Tower" },
	{ "sif_dreamtrap",      "The Dream-Trap of Zzar" },
	{ "sif_spectrum",       "Spectrum" },
	{ "sif_smoochiepoodle", "Smoochiepoodle and the Bastion of Science" },

	// SpeedIF Let's Make A Nightmare Competition
	{ "sif_sdoor",          "Schrödinger's Door" },

	// SpeedIF Late! Competition
	{ "sif_canape",         "The Canapés of Death" },
	{ "sif_lonestar",       "The Lone Star Menáçe" },

	// SpeedIF Neologism Competition
	{ "sif_minimumwage",    "A Minimum Wage Job" },
	{ "sif_almamater",      "AlmaMater" },
	{ "sif_hentai2",        "Captain Cumshot’s Second Adventure: The Rim Job" },
	{ "sif_definefar",      "Define Far" },
	{ "sif_expedition",     "Expedition" },
	{ "sif_speedifnockle",  "Revenge of the Nockle: A Speed IF" },

	// SpeedIF New Year's Speed Competition 2007
	{ "sif_nazimice",       "Nazi Mice" },
	{ "sif_notsameols",     "Not the Same Old Lang Syne" },
	{ "sif_shothitler",     "The Day I Shot Hitler" },
	{ "sif_newyearswar",    "The War on New Year’s" },

	// SpeedIF New Year's Speed Competition 2008
	{ "sif_leaptime",       "Leap Time" },
	{ "sif_mauled",         "The Day I Mauled Mao!" },
	{ "sif_escapist",       "The Escapist" },

	// SpeedIF New Year's Speed Competition 2009
	{ "sif_dullegriet",     "Dulle Griet and the Antenorian Icebox" },
	{ "sif_stabbed",        "The Day I Stabbed Stalin" },

	// SpeedIF New Year's Speed Competition 2011
	{ "sif_beingendbeg",    "Being the Ending of the Beginning" },
	{ "sif_birthmind",      "Birth of Mind" },
	{ "sif_dashslapney",    "Dash Slapney, Patrol Leader" },
	{ "sif_postchrist",     "Post-Christmas Letdown" },
	{ "sif_ragnarok",       "Ragnarok: Twilight of the Gods" },
	{ "sif_stupidcreek",    "Stupid Creek. Stupid Christmas" },
	{ "sif_savedelvis",     "The Day I Saved Elvis" },
	{ "sif_silencegods",    "The Silence of the Gods" },
	{ "sif_threemore",      "Three More Visitors" },

	// SpeedIF New Year's Speed Competition 2013
	{ "sif_faithfulcompv2", "Faithful Companion" },
	{ "sif_talemorning",    "The Mundane Tale of the Morning After" },
	{ "sif_dayicameback",   "The Day I Came Back" },
	{ "sif_youhavetoput",   "You Have to Put the Baby New Year in the Champagne Bottle" },

	// SpeedIF New Year's Speed Competition 2014
	{ "sif_youwerehere",    "You Were Here" },

	// SpeedIF O Competition
	{ "sif_speedo",         "A SpeedIF O Entry" },
	{ "sif_eschew",         "Eschew As If You Were She" },
	{ "sif_getting",        "Getting to Know the General" },
	{ "sif_astrology",      "Practical Astrology" },

	// SpeedIF PAX East 2010
	{ "sif_wizshop",        "A Wizard Goes Shopping" },
	{ "sif_apoptosis",      "Apoptosis" },
	{ "sif_foodmagic",      "Food Magic" },
	{ "sif_lobsteronplane", "Lobsters on a Plane" },
	{ "sif_mango",          "Mango" },
	{ "sif_midairmadness",  "Midair Madness" },
	{ "sif_paxeast",        "PAX East 2010 Speed-IF" },
	{ "sif_queuelty",       "Queuelty" },
	{ "sif_safehouse",      "Safehouse" },
	{ "sif_vortex2305",     "Vortex 2305" },

	// SpeedIF PAX East 2011
	{ "sif_scurvy",         "A Scurvy of Wonders" },
	{ "sif_shhelpfulman",   "A Shadow of Helpfulman" },
	{ "sif_boredladders",   "Bored Ladders" },
	{ "sif_brainightguest", "Brain of the Night Guest" },
	{ "sif_delusionsagain", "Delusions Again" },
	{ "sif_diggingtime",    "Digging Time!" },
	{ "sif_merk",           "Merk" },
	{ "sif_therighttool",   "The Right Tool" },

	// SpeedIF Orange Competition
	{ "sif_appall",         "Appallatron: Annoyotron 3" },
	{ "sif_descent",        "Descent of Man" },

	// SpeedIF Pi-Theta-Aleph-Parallax Competition
	{ "sif_parallaxdream",  "A Parallax Dream" },
	{ "sif_pethertheta",    "Peter Theta Fixes the Holodeck" },
	{ "sif_putiap",         "Pick up the IF-Archive and Pi" },
	{ "sif_squeaky",        "Squeaky on the Moon" },
	{ "sif_ptap",           "The Grade 3 Parallax" },
	{ "sif_moonbar",        "The Parallax Moon Bar Conspiracy" },
	{ "sif_thpoint",        "The Theta Point" },

	// SpeedIF `R*IF` Spam Competition
	{ "sif_htp",            "HTP" },

	// SpeedIF Scenario 1 Competition
	{ "sif_stars",          "The Stars Are Right" },

	// SpeedIF SchmeedIF Competition
	{ "sif_floyd",          "Floyd" },

	// SpeedIF Spring Competition
	{ "sif_exterminate",    "Exterminate!" },
	{ "sif_gardening",      "Gardening for Beginners" },
	{ "sif_springclean",    "Spring Cleaning" },

	// SpeedIF That Dare Not Speak Its Number Competition
	{ "sif_soitgoes",       "And So It Goes" },
	{ "sif_antioch",        "Jobs for Antioch!" },
	{ "sif_twilight",       "Twilight of the Dogs" },

	// SpeedIF Thanksgiving Competition
	{ "sif_dragonflies",    "Dragon Flies Like Labradorite" },
	{ "sif_crystalpalace",  "The Crystal Palace" },
	{ "sif_extraordevents", "The Extraordinary Events of the Last Day of Professor Mangleworth" },
	{ "sif_youareaturkey",  "You Are a Turkey!" },

	// SpeedIF ToasterComp II Competition
	{ "sif_stolen",         "A Monkey Stole Your Toast!" },
	{ "sif_needmoretoast",  "Need! More! Toast!" },
	{ "sif_samurai",        "Samurai Tea Room" },
	{ "sif_toastless",      "The Epitome of Toastlessness" },

	// SpeedIF U Competition
	{ "sif_cheatingdeath",  "Cheating Death" },
	{ "sif_rrr",            "Reality Railroad" },

	// SpeedIF Ultimate not numbered New Year's Competition
	{ "sif_shortof",        "Short of Sushi" },
	{ "sif_disenchantbay",  "Disenchantment Bay" },
	{ "sif_thedayidied",    "The Day I Died" },

	// SpeedIF Without Number Competition
	{ "sif_sushi",          "A Day for Fresh Sushi" },
	{ "sif_anotherday",     "Another Day, Another Sea Monster" },
	{ "sif_lobster",        "The Lobster" },
	{ "sif_grape",          "Triumphant Return of the Evil Sea Grape" },

	// SpeedIF XYZZY Competition
	{ "sif_nightxyzzyes",   "A Night at the XYZZYies" },
	{ "sif_behold",         "Behold!" },
	{ "sif_tragedy",        "Tragedy Strikes at the XYZZY Awards!" },

	// SpeedIF Y Competition
	{ "sif_bananapocalayp", "Banana Apocalypse and the Rocket Pants of Destiny" },
	{ "sif_barton",         "Barton" },
	{ "sif_bearsx3",        "Bears, Bears, Bears" },
	{ "sif_flexible",       "Flexible Pants" },
	{ "sif_pantsrun",       "Pants on the Run" },
	{ "sif_tripbeer",       "Triple Bear Beer" },

	// SpeedIF y=1/x Competition
	{ "sif_perfectday",     "A Perfect Day for Candiru" },
	{ "sif_apocolyptica",   "Apocolyptica" },

	// SpeedIF Zombie Competition
	{ "sif_speedifzombie",  "SpeedIF Zombie" },

	// SpeedIF Bouchercomp Competition
	{ "sif_lowellparadise", "Lowell’s Paradise" },
	{ "sif_putpbad",        "Pick up the Pine Box and Die" },
	{ "sif_seekingsolace",  "Seeking Solace" },
	{ "sif_wideopengate",   "The Wide-Open Gate" },

	// Swash Comp
	{ "swa_seacaptains",    "Sea Captains" },

	// Text Adventure Literacy Jam 2024
	{ "bakemono",           "Bakemono no Sekai" },                                                      //      Entrant
	{ "whokidmgoose",       "Who Kidnapped Mother Goose?" },                                            //      Entrant

	// Toaster Competition
	{ "toa_burnttoast",     "Burnt Toast" },
	{ "toa_friarbaconstr",  "Friar Bacon's Secret" },
	{ "toa_gottoast",       "Got Toast?" },
	{ "toa_spittingcrumbs", "Spitting Crumbs" },
	{ "toa_shrinkingwoman", "The Incredible Shrinking Woman, Too!" },
	{ "toa_tommytoaster",   "Tommy the Toaster" },

	// TWIFcomp
	{ "twif_putpbat",       "Putpbat" },
	{ "twif_sin1",          "Sin 1" },
	{ "twif_sin2",          "Sin 2" },
	{ "twif_sin3",          "Sin 3" },
	{ "twif_sin4",          "Sin 4" },
	{ "twif_sin5",          "Sin 5" },
	{ "twif_sin6",          "Sin 6" },
	{ "twif_sin7",          "Sin 7" },
	{ "twif_sin8",          "Sin 8" },
	{ "twif_war",           "War" },

	// Walkthrough Competition
	{ "wlk_wtf",            "A Walk Through Forever" },
	{ "wlk_dreams",         "Dreams Run Solid" },
	{ "wlk_fit",            "Fit for a Queen" },
	{ "wlk_jigsaw2",        "Jigsaw 2" },
	{ "wlk_exile",          "Twilight in the Garden of Exile" },

	// XComp 1999
	{ "xco_sixfoot",        "The Six-Foot-Tall Man-Eating Chicken" },

	// Spring Thing 2002
	{ "stc02_blues",        "Tinseltown Blues" },                                                       //       Winner

	// Spring Thing 2003
	{ "stc03_inevita",      "Inevitable" },                                                             //    Runner-Up
	{ "stc03_cofire",       "The Cross of Fire" },                                                      //      Entrant

	// Spring Thing 2005
	{ "stc05_telling",      "Whom the Telling Changed" },                                               //    1st Place
	{ "stc05_flatfeet",     "Flat Feet" },                                                              //    4th Place
	{ "stc05_authority",    "Authority" },                                                              //    6th Place

	// Spring Thing 2006
	{ "stc06_debaron",      "De Baron" },                                                               //    1st Place
	{ "stc06_thebaron",     "The Baron" },                                                              //    1st Place

	// Spring Thing 2007
	{ "stc07_fate",         "Fate" },                                                                   //    1st Place

	// Spring Thing 2008
	{ "stc08_pascal",       "Pascal's Wager" },                                                         //    1st Place

	// Spring Thing 2009
	{ "stc09_milkparadise", "The Milk of Paradise" },                                                   //    4th Place

	// Spring Thing 2012
	{ "stc12_rocketman",    "The Rocket Man From the Sea" },                                            //    1st Place

	// Spring Thing 2014
	{ "stc14_mrp",          "The Story of Mr. P." },                                                    //    7th Place

	// Spring Thing 2015 - Back Garden
	{ "stc15_dirk",         "Dirk" },                                                                   //      Entrant

	// Spring Thing 2016
	{ "stc16_harmonic",     "Harmonic Time-Bind Ritual Symphony" },                                     //      Entrant

	// Spring Thing 2017 - Back Garden
	{ "stc17_enlightened",  "Enlightened Master" },                                                     //      Entrant

	// Spring Thing 2018 - Main Festival
	{ "stc18_gopher",       "Best Gopher Ever" },                                                       //      Entrant
	{ "stc18_murderbig",    "Murder on the Big Nothing" },                                              //      Entrant

	// Spring Thing 2019 - Main Festival
	{ "stc19_pca",          "Porte Cave Adventure" },                                                   //      Entrant

	// Spring Thing 2019 - Back Garden
	{ "stc19_69105keys",    "69,105 Keys" },                                                            //      Entrant

	// Spring Thing 2021 - Main Festival
	{ "stc21_takethedogout", "Take the Dog Out" },                                                      //      Entrant

	// Spring Thing 2022 - Main Festival
	{ "stc22_hypercubic",   "Hypercubic Time Warp All-go-rhythmic Synchrony" },                         //      Entrant
	{ "stc22_wry",          "Wry" },                                                                    //      Entrant

	// Spring Thing 2023 - Main Festival
	{ "stc23_mariewaits",   "Marie Waits" },                                                            //      Entrant

	// Spring Thing 2024 - Main Festival
	{ "stc24_zomburbia",    "Zomburbia" },                                                              //      Entrant

	// Danish games
	{ "nissen",             "Pa Loftet Sidder Nissen" },

	// Dutch games
	{ "avontuur",           "Avontuur" },
	{ "draak",              "Draak" },
	{ "wraakzucht",         "Wraakzucht" },
	{ "zwijnsrug",          "Zwijnsrug" },

	// French games
	{ "agentpaix",          "Agent de la Paix Terrestre" },
	{ "ascenseur",          "Ascenseur" },
	{ "aventure",           "Aventure" },
	{ "balcon",             "Sorciere au Balcon" },
	{ "brume",              "Brume" },
	{ "castelrous",         "Castelrous" },
	{ "caverne",            "La Caverne des Morlocks" },
	{ "championbasketball", "Champion de Basket-ball" },
	{ "chatiment",          "Châtiment Divin" },
	{ "ciafr",              "C.I.A. Aventure" },
	{ "citeeaux",           "La Cité des Eaux" },
	{ "citronille",         "Citronille" },
	{ "enfant",             "Un Jeu d'Enfant" },
	{ "escaperoom",         "Escape Room: La pièce Vide" },
	{ "espions",            "Les Espions Ne Meurent Jamais" },
	{ "etoiles",            "Celui qui Voulait Décrocher les Etoiles" },
	{ "femme",              "La Femme Qui Ne Supportait pas Les Ordinateurs" },
	{ "fenetre",            "Fenêtre sur ma Cour" },
	{ "filaments",          "Filaments" },
	{ "grosscarabee",       "Mon Voisin Est un Gros Scarabée" },
	{ "histoire",           "Une Histoire" },
	{ "homelandsecurity",   "Homeland Security" },
	{ "iletait",            "Il Etait une Fois" },
	{ "initiation",         "Initiation" },
	{ "interra",            "INTERRA - L'Autre Monde" },
	{ "jourdechance",       "Jour de Chance" },
	{ "katana",             "Le Scarabee et le Katana" },
	{ "kheper",             "Kheper" },
	{ "lieuxcommuns_zcode", "Lieux Communs" },
	{ "lifeonmarsfr",       "Life on Mars?" },
	{ "livraisonexpress",   "Livraison Express" },
	{ "lmpsd",              "La Mort Pour Seul Destin" },
	{ "lupercalia_zcode",   "Lupercalia" },
	{ "magicien",           "Escape Room: Le Magicien" },
	{ "memelespommes",      "Même les Pommes de Terre Ont des Yeux!" },
	{ "mortblue",           "La Mort Bleue" },
	{ "ombre",              "Ombre" },
	{ "pantinelectrique",   "Le Pantin Electrique: Prologue" },
	{ "paranoiafr",         "Paranoia" },
	{ "petitgnome_zcode",   "Petit Gnome" },
	{ "plicploc",           "Plic Ploc" },
	{ "princesse",          "Ma Princesse Adoree" },
	{ "quetedubaal",        "La Quête du Baal" },
	{ "rats",               "Rats" },
	{ "recitsdegrandpere",  "Récits de Grand-Père" },
	{ "routedesvins",       "La Route es Vins" },
	{ "sarvegne_zcode",     "Sarvegne" },
	{ "sdlc",               "Sortir de la Chambre" },
	{ "secrets",            "Secrets de Pêcheurs" },
	{ "sorciereaubalcon",   "Sorcière au Balcon, Apprenti en Déraison!" },
	{ "spoutnik",           "Spoutnik" },
	{ "sueursfroides",      "Sueurs Froides à l’Institut François-Marie Raoult" },
	{ "templedefeu",        "Le Temple de Feu" },
	{ "templenaga",         "Le Temple Nâga" },
	{ "terres",             "Terres Etrangères" },
	{ "tristam",            "L'Ile Tristam" },
	{ "valleemysterieuse",  "La Vallée Mystérieuse" },
	{ "verdeterre",         "Le Butin du Capitaine Verdeterre" },
	{ "vindaloofr",         "Vindaloo" },

	// French Comp 2005 (French)
	{ "frc_cercledesgros",  "Le Cercle des Gros Geeks Disparus" },                                      //    1st Place
	{ "frc_dreamlands",     "Echappee Belle Dans Les Contrees du Reve" },                               //    2nd Place
	{ "frc_templedefeu",    "Le Temple de Feu" },                                                       //    3rd Place

	// French Comp 2006 (French)
	{ "frc_citeeaux",       "La Cité des Eaux" },                                                       //    1st Place
	{ "frc_sarvegne",       "Sarvegne" },                                                               //    2nd Place

	// French Comp 2007 (French)
	{ "frc_heuresduvent",   "Heures Du Vent" },                                                         //    1st Place
	{ "frc_divinebonace",   "Divine Bonace" },                                                          //    2nd Place
	{ "frc_brrr",           "Brrr!" },                                                                  //    3rd Place
	{ "frc_ilephare_zcode", "L'Ile du Phare Abandonné" },                                               //    5th Place

	// French Comp 2008 (French)
	{ "frc_brume",          "Brume" },                                                                  //    1st Place
	{ "frc_lettresvolees",  "Les Lettres Volées" },                                                     //    2nd Place
	{ "frc_louplachevre",   "Le Loup, la Chèvre, et la Salade" },                                       //    4th Place

	// French Comp 2009 (French)
	{ "frc_catapole",       "Catapole" },                                                               //    1st Place
	{ "frc_chambresyrion",  "La Chambre de Syrion" },                                                   //    2nd Place
	{ "frc_mechants",       "Les Méchants Meurent au Moins deux Fois" },                                //    3rd Place

	//French Comp 2011 (French)
	{ "frc_aventureszeus",  "Les Aventures de Zeus" },                                                  //    1st Place
	{ "frc_dardenfer",      "Dard d'Enfer" },                                                           //    2nd Place
	{ "frc_terreciel",      "Entre Terre et Ciel" },                                                    //    3rd Place
	{ "frc_astrologue",     "La Grande Prédiction ou l'Astrologue Etourdi" },                           //    4th Place
	{ "frc_homelandsec",    "Homeland Security" },                                                      //    5th Place

	// French Comp 2013 (French)
	{ "frc_lifeonmarsfr",   "Life on Mars?" },                                                          //    1st Place
	{ "frc_sourcedezig",    "La Source de Zig" },                                                       //    3rd Place
	{ "frc_trac",           "Trac" },                                                                   //    4th Place

	// French Comp 2015 (French)
	{ "frc_envol",          "L'Envol" },                                                                //    1st Place

	// French Comp 2016 (French)
	{ "frc_tipelau",        "Tipelau" },                                                                //    2nd Place
	{ "frc_diamantblanc",   "Le Diamant Blanc" },                                                       //    3rd Place

	// French Comp 2018 (French)
	{ "frc_exil",           "L'Exil" },                                                                 //    5th Place

	// French Comp 2021 (French)
	{ "frc_donjon",          "Le Donjon de BatteMan" },                                                 //      Entrant
	{ "frc_stationspatial",  "Station Spatiale S16: Prologue" },                                        //      Entrant

	// German games
	{ "abenteuer",          "Abenteuer" },
	{ "bearg",              "Ein Bar Geht Aus" },
	{ "dasspiel",           "Das Spiel, Zuvor Bekannt als Verborgener Nazi-Modus" },
	{ "debaron",            "De Baron" },
	{ "deklinator",         "Object Declination Tool" },
	{ "dermantel",          "Der Mantel der Finsternis" },
	{ "diegerechtenricht",  "Die Gerechten Richter" },
	{ "drakdiamanten_zcode","Drakulas Diamanten" },
	{ "eskmausden",         "Es Kam aus den Alpen" },
	{ "halb2",              "Halb Zwei" },
	{ "herr",               "Die Geschichte des Herrn P. von Hannes Schuller" },
	{ "karisma",            "Klub Karisma" },
	{ "kleine",             "Der Kleine Halbling" },
	{ "knack",              "Knack!" },
	{ "mchatton",           "Tutorial Eine Einfuhrung in Textadventures von Cooper McHatton" },
	{ "nurmengard",         "Nurmengard" },
	{ "o",                  "O" },
	{ "schiebefehl",        "Schiebefehl" },
	{ "seestrasse",         "Seestraße" },
	{ "starrider",          "Star Rider" },
	{ "trampelviecher",     "Trampelvieche: Eine Halloween-Kurzgeschichte" },
	{ "wasserhasser",       "Wasser-Hasser" },
	{ "whisperedworldta",   "The Whispered World" },
	{ "wichtel",            "Wichtel" },

	// Textfire Grand Prix 2002 (German)
	{ "tgp_eden",           "Eden" },                                                                   //    1st Place
	{ "tgp_bewerbung",      "Die Bewerbung" },                                                          //    3rd Place
	{ "tgp_seite",          "Mein Leben für Seite Drei" },                                              //    4th Place

	// Textfire Grand Prix 2003 (German)
	{ "tgp_linear",         "Linear" },                                                                 //    1st Place

	// Textfire Grand Prix 2004 (German)
	{ "tgp_jazteg",         "Jazz auf Tegemis" },                                                       //    1st Place
	{ "tgp_spater",         "Zwei Jahre später" },                                                      //    4th Place
	{ "tgp_unterwelt",      "Unterwelt" },                                                              //    5th Place
	{ "tgp_die5",           "Die 5 Kammer" },                                                           //    8th Place
	{ "tgp_dichter",        "Dichter" },                                                                //    9th Place

	// Textfire Grand Prix 2005 (German)
	{ "tgp_kopialbuch",     "Das Kopialbuch" },                                                         //    1st Place

	// Textfire Grand Prix 2006 (German)
	{ "tgp_felleisen",      "Das Felleisen" },                                                          //    1st Place
	{ "tgp_pmason",         "P. Mason und der Schlitzerhans und die Busenkathi" },                      //    2nd Place

	// Textfire Grand Prix 2010 (German)
	{ "tgp_hausaufgabe",    "Die Hausaufgabe" },                                                        //    3rd Place
	{ "tgp_absturzmomente", "Absturzmomente" },                                                         //    4th Place

	// Textfire Grand Prix 2011 (German)
	{ "tgp_gorgonir",       "Gorgonir" },                                                               //    2nd Place
	{ "tgp_ausgerechnet",   "Ausgerechnet Mamph Pamph!" },                                              //    4th Place

	// IF Grand Prix 2015 (German)
	{ "tgp_dieakte",        "Die Akte Paul Bennet" },                                                   //    1st Place
	{ "tgp_lilie",          "Die Schwarze Lilie" },                                                     //    1st Place

	// IF Grand Prix 2016 (German)
	{ "tgp_emilia",         "Der Tag an dem Emilia W. Verschwand" },                                    //    3rd Place

	// IF Grand Prix 2022 (German)
	{ "tgp_schief",         "Schief" },                                                                 //    1st Place
	{ "tgp_dieerstenacht",  "Die Erste Nacht" },                                                        //    2nd Place

	// IF Grand Prix 2023 (German)
	{ "tgp_fischstaebchen", "Fischstaebchen" },                                                         //    3rd Place
	{ "tgp_gennorden",      "Gen Norden" },                                                             //    4th Place

	// Italian games
	{ "africanescape",      "African Escape" },
	{ "anello",             "L'Anello" },
	{ "armando",            "L'Armando" },
	{ "attentiombra",       "Attenti a quell'Ombra" },
	{ "ayon_zcode",         "Nel Mondo di Ayon" },
	{ "beyondita_zcode",    "Beyond" },
	{ "bustadimenticata",   "La Busta Dimenticata" },
	{ "campusinvad_zcode",  "Campus Invaders" },
	{ "casamisteriosa",     "Quella Casa Misteriosa Laggiù nella Palude" },
	{ "cosmic",             "Cosmic Adventure" },
	{ "darkiss1ita_zcode",  "Darkiss! Il Bacio del Vampiro - Capitolo 1: Il Risveglio" },
	{ "darkiss2ita_zcode",  "Darkiss! Il Bacio del Vampiro - Capitolo 2: Viaggio all'Inferno" },
	{ "enigma_zcode",       "Enigma" },
	{ "filamit",            "Filaments" },
	{ "flamel",             "Flamel" },
	{ "fugacropoli_zcode",  "Fuga dall'Acropoli" },
	{ "giardino_zcode",     "Il Giardino Incantato" },
	{ "hellosword_ita",     "Hello Sword: Il Viaggio" },
	{ "intrappola",         "In Trappola" },
	{ "isolakiller",        "L'Isola del Killer" },
	{ "jibbidu",            "Jibbidu" },
	{ "kangourou",          "Kangourou dell'Informatica 2013" },
	{ "katzrfun",           "Katz R FuN" },
	{ "killer",             "The Killer His or Her..." },
	{ "koohinoor",          "Kooh-I-Noor" },
	{ "lazyjones",          "Lazy Jones e il Meritato Riposo" },
	{ "littlefalls_zcode",  "Little Falls" },
	{ "lucifinanza_zcode",  "Luci della Finanza" },
	{ "marconi_zcode",      "Visita al Marconi" },
	{ "noalpitour",         "No Alpitour" },
	{ "noavventura",        "Non Sarà un'Avventura" },
	{ "oldwest1",           "Pecos Town, Old West Episode I" },
	{ "ordinedragone",      "L'Ordine del Dragone" },
	{ "paosqura",           "Pasqua Oscura" },
	{ "pietraluna_zcode",   "La Pietra della Luna" },
	{ "poesia_zcode",       "Manca Solo un Verso a Quella Poesia" },
	{ "roccatuono",         "Guilhern di Ob: Ritorno a Rocca del Tuono" },
	{ "rovo",               "Il Rovo" },
	{ "scarafaggio",        "Lo Scarafaggio" },
	{ "sfida",              "Sfida all'Ignoto" },
	{ "sognosangue_zcode",  "Sogno di Sangue" },
	{ "sparviero",          "Lo Sparviero" },
	{ "sparviero2",         "Lo Sparviero II: Festa a Blanchefort" },
	{ "strega",             "La Strega di Maughn" },
	{ "stregatto_zcode",    "Salvate lo Stregatto!" },
	{ "terkhen",            "Ogul e lo Scettro di Terkhen" },
	{ "tesla_zcode",        "In Cerca di Tesla" },
	{ "toyshop",            "Toyshop: Negozio di Giocattoli" },
	{ "trincea",            "Trincea" },
	{ "villamorgana_zcode", "Villa Morgana" },
	{ "vita_zcode",         "La Vita? Non Venitemi a Parlare della Vita" },
	{ "volonta_zcode",      "La Volontà dei Morti" },
	{ "zazie",              "Zazie, Una Lettura Interattiva" },
	{ "zenfactorspa",       "ZenFactor Spa" },
	{ "zigamusita_zcode",   "Zigamus: Zombi al Vigamus!" },
	{ "zombie",             "Uno Zombie a Deadville" },
	{ "zorkita",            "Zork I: Il Grande Impero Sotterraneo" },

	// Marmellata d'Avventura 2018 (Italian)
	{ "ma_lastprigioniero", "I Cinque Feudi" },
	{ "ma_skepto",          "Skepto!" },
	{ "ma_dejavu_zcode",    "Déjà Vu" },
	{ "ma_lazystranocaso",  "Lazy Jones e lo Strano Caso" },
	{ "ma_pilastri",        "Sigehao - I Quattro Pilastri" },

	// Marmellata d'Avventura 2019 (Italian)
	{ "ma_salagamescastle", "La Sala dei Giochi del Castello del Signore di Ylourgne in Averoigne" },
	{ "ma_tesorosalagames", "Il Tesoro della Sala Giochi" },
	{ "ma_lazysalagiochi",  "Lazy Jones e la Sala Giochi" },
	{ "ma_5feudi",          "I Cinque Feudi" },

	// Russian games
	{ "allroads_rus",       "All Roads" },
	{ "bluechairs_rus",     "Blue Chairs" },
	{ "metamorphoses_rus",  "Metamorphoses" },
	{ "photopia_rus",       "Photopia" },
	{ "spiderandweb_rus",   "Spider and Web" },

	// Slovenian games
	{ "zenin",              "Zenin na Begu" },

	// Spanish games
	{ "abismo_zcode",       "El Abismo" },
	{ "acuario_zcode",      "Acuario" },
	{ "adso",               "Adso de Melk" },
	{ "alicia",             "A Trves del Espejo" },
	{ "anillo",             "El Anillo" },
	{ "anillo3",            "Anillo III" },
	{ "avent",              "Aventurilandia" },
	{ "aventura",           "Aventura" },
	{ "bicho",              "El Bicho" },
	{ "busqueda",           "Busqueda Desperada" },
	{ "byodspa",            "BYOD" },
	{ "casarisa",           "La Casa de la Risa" },
	{ "cavernad",           "La Caverna del Drgon de Bronce" },
	{ "corto",              "Corto" },
	{ "cronicasparaiso",    "Cronicas del Paraiso" },
	{ "cruzado",            "El Cruzado" },
	{ "cueva",              "La Oscura Cueva" },
	{ "dagon_zcode",        "Dagon" },
	{ "despert",            "El Despertar" },
	{ "draculasp",          "Dracula: Episodio 1, La Primera Noche" },
	{ "ecss",               "es.comp.sistemas.sinclair" },
	{ "edaylobo",           "Eda y el Lobo" },
	{ "elcontinente",       "El Continente" },
	{ "ellibro",            "El Libro" },
	{ "elpuzzle",           "El Puzzle" },
	{ "encierro",           "Encierro" },
	{ "ergotdelima_zcode",  "Ergot de Lima" },
	{ "estacion_zcode",     "Secuestro" },
	{ "excessus",           "Excessus" },
	{ "forrajeo",           "Forrajeo" },
	{ "fotopia",            "Fotopia" },
	{ "geo",                "Geo" },
	{ "gorron",             "El Gorron del Tren" },
	{ "hampa_zcode",        "Historias del Hampa" },
	{ "heredar",            "Heredar!" },
	{ "heresville",         "Heresville" },
	{ "imposibl",           "Imposible" },
	{ "islamisteriosa",     "La Isla Misteriosa" },
	{ "juguete",            "Jugueteria" },
	{ "laberinto",          "Un Laberinto Acordado" },
	{ "latorre_zcode",      "La Torre" },
	{ "laverja",            "La Verja" },
	{ "legion",             "Legin de las Tinieblas" },
	{ "logicinv",           "Lógica Inversa" },
	{ "maquina",            "Maquina" },
	{ "marsmenace",         "Mars Menace From Outer Space" },
	{ "memorias_zcode",     "Memorias de reXXe" },
	{ "mpdroidone",         "Operacion MPDroid1" },
	{ "nada",               "La Nada" },
	{ "pantalla",           "Pantallas y Más Pantallas..." },
	{ "paraiso",            "Llave Hacia el Paraíso" },
	{ "perseo",             "Perseo y Andromeda" },
	{ "peso",               "Una Cuestión de Peso" },
	{ "playera",            "En la Playa" },
	{ "premios",            "II Concurso de Mini-Aventuras" },
	{ "pyramid",            "Piramide de la Perdicion" },
	{ "quenoche",           "Qué noche!" },
	{ "quovadis",           "Un Ejemplo de QuoVadis" },
	{ "raro",               "Raro" },
	{ "redencionmomificad", "Redención Momificada" },
	{ "reflejos",           "Reflejos Blancos" },
	{ "regalo",             "El Regalo de Gorbag" },
	{ "relojes_zcode",      "Un Lugar en Ninguna Parte Pero en Algún Momento" },
	{ "roleando",           "Roleando" },
	{ "saee",               "Saee" },
	{ "sgw_zcode",          "Test Para SGW" },
	{ "sinclairmisterio",   "Sinclair y el Misterio de Glen Quaich" },
	{ "sinrumbo",           "Sin Rumbo" },
	{ "tiros",              "Un Día Duro en la Sala de Tiro" },
	{ "toma",               "Toma el Telfono y di E" },
	{ "trapero",            "El Trapero Cantando" },
	{ "tribu",              "La TrIbU" },
	{ "varenna",            "Varenna Quest I: El Castillo de la Rosa Negra" },
	{ "waxworksspa",        "Adventura de Misterio #11: Museo de Cera" },
	{ "werewolf",           "Lobos y Aldeanos" },
	{ "zipi",               "Zipi" },
	{ "zna",                "Zna No es una Aventura" },

	// Ectocomp 2017 - Le Grand Guignol (Spanish)
	{ "ec17_decolor",       "Decolor" },                                                                //    3rd Place

	// Ectocomp 2022 - Le Grand Guignol (Spanish)
	{ "ec22_estadop_zcode", "Estado Profundo" },                                                        //    2nd Place

	// Interactive Fiction Writing Month 2009
	{ "ifm_artifice",       "Artifice" },
	{ "ifm_assignone",      "Assignment Onde" },
	{ "ifm_blackberry",     "Blackberry" },
	{ "ifm_blasphemy",      "Blasphemy" },
	{ "ifm_breakenter",     "Breaking and Entering" },
	{ "ifm_busstation",     "Bus Station" },
	{ "ifm_checkpoint",     "Checkpoint" },
	{ "ifm_colophon",       "Colophon" },
	{ "ifm_fraterevan",     "Frater Evanesco" },
	{ "ifm_grocery",        "Grocery Shopping" },
	{ "ifm_heavensent",     "Heaven-Sent" },
	{ "ifm_helloworld",     "Hello World!" },
	{ "ifm_ifmonth",        "IF-Month-Zero" },
	{ "ifm_inheritance",    "Inheritance" },
	{ "ifm_latetrain",      "Late Train to Nowhere" },
	{ "ifm_laura",          "Laura" },
	{ "ifm_metrohobo",      "Metro Hobo" },
	{ "ifm_nomuse",         "No Muse" },
	{ "ifm_octopus",        "Octopus's Garden" },
	{ "ifm_office",         "The Office" },
	{ "ifm_ohmyeldergod",   "Oh My Elder God" },
	{ "ifm_painterly",      "Painterly" },
	{ "ifm_perilkeet",      "Perilkeet" },
	{ "ifm_rolled",         "Rolled" },
	{ "ifm_seadreams",      "Sea Dreams" },
	{ "ifm_searchanswer",   "Search for Answers" },
	{ "ifm_smugglers",      "Smugglers Rest" },
	{ "ifm_sparechange",    "Spare Some Change" },
	{ "ifm_tower",          "Tower" },
	{ "ifm_trails",         "Trails" },
	{ "ifm_upyoursleeve",   "Up Your Sleeve" },
	{ "ifm_winter",         "Winter" },

	// Premios Hispanos 2000 (Spanish)
	{ "ph00_abalanzate",    "Abalanzate" },
	{ "ph00_alemanes",      "Golpe a Los Nazis" },
	{ "ph00_aparato",       "El Aparato" },
	{ "ph00_aque1",         "Aquelarre" },
	{ "ph00_b1",            "B1" },
	{ "ph00_casi",          "Casi Muerto" },
	{ "ph00_csa",           "Cacahuetes, Sal y Aceite" },
	{ "ph00_cuentoch",      "Cuento Chino" },
	{ "ph00_faro00",        "A lo Lejos, Un Faro" },
	{ "ph00_fotones",       "Fotones Perdidos" },
	{ "ph00_gusano",        "El Sentir del Gusano" },
	{ "ph00_olvido_zcode",  "Olvido Mortal" },
	{ "ph00_papi",          "Papi, ¡Cómprame una PlayStation" },
	{ "ph00_senten",        "La Sentencia" },
	{ "ph00_torre",         "Misterio en la Torre" },
	{ "ph00_tokland",       "La Isla de Tokland" },

	// Premios Hispanos 2001 (Spanish)
	{ "ph01_aciegas",       "A Ciegas" },
	{ "ph01_celos",         "Un Asunto de Celos" },
	{ "ph01_ch3ch2oh",      "CH3-CH2-OH" },
	{ "ph01_churro",        "Churro Patatero" },
	{ "ph01_conrumb_zcode", "Con Rumbo" },
	{ "ph01_lldc",          "La Llamada de Cthulhu" },
	{ "ph01_necedad",       "Por la Necedad Humana" },
	{ "ph01_ocaso",         "Ocaso Mortal I: The Bug" },
	{ "ph01_segapark",      "Aventurero en el Segapark" },

	// Premios Hispanos 2002 (Spanish)
	{ "ph02_asesinato",     "Asesinato en el Continental" },
	{ "ph02_aveces",        "A Veces..." },
	{ "ph02_cv",            "Curriculum Vitae" },
	{ "ph02_ddddddcrj",     "Cirith Ungol" },
	{ "ph02_declina",       "Declinación Mortal" },
	{ "ph02_jugueteria",    "La Jugueteria del Mago Zacarias" },
	{ "ph02_ligue",         "Ligue en Ciutat la Nit" },
	{ "ph02_meeva",         "La Mediana Evasion" },
	{ "ph02_obituario",     "Obituario" },
	{ "ph02_osito",         "La Intrepida Noche del Osito" },
	{ "ph02_pruebafisica",  "Prueba Física" },
	{ "ph02_regente_zcode", "El Anillo Regente" },
	{ "ph02_sombras",       "Sombras de Moria" },
	{ "ph02_uhogar_zcode",  "Misterio en el Ultimo Hogar" },

	// Premios Hispanos 2003 (Spanish)
	{ "ph03_aluzine",       "Aluzine" },
	{ "ph03_array",         "Array" },
	{ "ph03_castillo",      "El Castillo de los Lamentos" },
	{ "ph03_cubo",          "Cubo" },
	{ "ph03_domador",       "El Caso del Misterioso Asesinato del Domador" },
	{ "ph03_dwight_zcode",  "El Extraño Caso de Randolph Dwight" },
	{ "ph03_libroaburria",  "El Libro que se Aburría" },
	{ "ph03_lunasp",        "La Sombra de la Luna Negra" },
	{ "ph03_mimoso",        "The Mimoso Adventure" },
	{ "ph03_otrpalab",      "En Otras Palabras" },
	{ "ph03_rural",         "La Aventura Rural" },

	// Premios Hispanos 2004 (Spanish)
	{ "ph04_islas",         "El Archipielago" },
	{ "ph04_lamansion",     "La Mansion" },
	{ "ph04_navidad",       "Una Pequena Historia de Navidad" },
	{ "ph04_oculta",        "La Cara Oculta de la Luna" },
	{ "ph04_remi",          "ReminiscenciaRol" },

	// Premios Hispanos 2005 (Spanish)
	{ "ph05_castilsilenco", "El Castillo del Silencio" },
	{ "ph05_pozo",          "Al Fondo del Pozo" },
	{ "ph05_elprotector",   "El Protector" },
	{ "ph05_leminscata",    "Leminscata" },
	{ "ph05_bardo",         "La Canción del Bardo" },
	{ "ph05_trono",         "El Trono de Inglaterra" },
	{ "ph05_romanfredo",    "Romanfredo" },

	// Premios Hispanos 2006 (Spanish)
	{ "ph06_draculasp2",    "Dracula: Episodio 2, La Llegada" },
	{ "ph06_elgatocheko",   "El Gato Cheko" },
	{ "ph06_goteras",       "Goteras" },
	{ "ph06_paee_zcode",    "Paee" },
	{ "ph06_regreso",       "El Regreso" },
	{ "ph06_resaca",        "Resaca" },

	// Premios Hispanos 2007 (Spanish)
	{ "ph07_030307",        "Unidad 030307" },
	{ "ph07_afuera",        "Afuera" },
	{ "ph07_aod",           "Antes o Despuses" },
	{ "ph07_boxman",        "Boxman" },
	{ "ph07_edc_zcode",     "Error de Copia" },
	{ "ph07_faro07",        "A lo Lejos, Un Faro" },
	{ "ph07_htec_zcode",    "Hierba Tras el Cristal" },
	{ "ph07_macetas",       "Macetas" },
	{ "ph07_marcopolo",     "Marco Polo" },
	{ "ph07_mausoleo",      "Yuriko in the Mausoleo" },
	{ "ph07_umami",         "El Día del Umami" },

	// Premios Hispanos 2008 (Spanish)
	{ "ph08_diana_zcode",   "Diana" },
	{ "ph08_emmy",          "Museo Poetico Emmy" },
	{ "ph08_gambito",       "El Gambito Slagar" },
	{ "ph08_mushahierba",   "Musha Hierba" },
	{ "ph08_pincoya",       "Ofrenda a La Pincoya" },
	{ "ph08_pronto",        "Pronto" },
	{ "ph08_sm1pesadilla",  "Serie Minúscula #1: Pesadilla Voraz" },
	{ "ph08_sm2konix",      "Serie Minúscula #2: Konix" },
	{ "ph08_sm3absolutos",  "Serie Minúscula #3: Los Sonidos Absolutos" },
	{ "ph08_sm4culpatuya",  "Serie Minúscula #4: No es Culpa Tuya Maria" },
	{ "ph08_sonrisas",      "Sonrisas... Y Lagrimas" },
	{ "ph08_venenaverbo",   "Venenarius Verborum" },

	// Premios Hispanos 2009 (Spanish)
	{ "ph09_amanda",        "Amanda" },
	{ "ph09_gorbag",        "El Regalo de Gorbag" },
	{ "ph09_hhorcus_zcode", "Homo Homini Orcus" },
	{ "ph09_kerulen",       "Ke Rulen los Petas" },
	{ "ph09_lae_zcode",     "La Aventura Espacial" },
	{ "ph09_megara",        "Los Placeres de Megara" },
	{ "ph09_panajo",        "Pan de Ajo" },
	{ "ph09_sm6ascenso",    "Serie Minúscula #6: El Ascenso de Kunelar" },
	{ "ph09_visit_zcode",   "Visitantes" },

	// Premios Hispanos 2010 (Spanish)
	{ "ph10_azul",          "Azul Fuerte" },
	{ "ph10_lpc_zcode",     "La Pequena Cerillera" },
	{ "ph10_modusvivendi",  "Modus Vivendi" },
	{ "ph10_panico",        "Pánico a la Caída" },
	{ "ph10_sobrevive",     "Sobrevive" },

	// Swedish games
	{ "aventyr",            "Aventyr" },
	{ "drakmagi_zcode",     "Drakmagi" },
	{ "hotellet",           "Hotel Noir" },
	{ "pangnyheten",        "Pangnyheten" },
	{ "storforsen",         "Storforsen" },
	{ "stuga",              "Stuga" },
	{ "vanyar_zcode",       "Vanyar" },

	// Playfic games
	{ "1hour52min15sec",    "1 Hour, 52 Minutes, 15 Seconds" },
	{ "4horsemenapocalyps", "Four Horsemen of the Apocalypse" },
	{ "5halfminutehallway", "The 5 and a Half Minute Hallway" },
	{ "5nightsbarrypalace", "Five Nights at the Barry Palace" },
	{ "8thgradehaleproj",   "8th Grade HALE Project" },
	{ "221bbakerstreet",    "221b Baker Street" },
	{ "721acaroline",       "721A Caroline" },
	{ "1012staffordave",    "1012 Stafford Ave" },
	{ "1156chapelstreet",   "1156 Chapel Street" },
	{ "1948",               "1948" },
	{ "1984",               "1984" },
	{ "1997experiment",     "The 1997 Experiment of Dr Dysnomia" },
	{ "2084",               "2084" },
	{ "2136",               "2136" },
	{ "a221",               "A-221" },
	{ "abandonedhouse",     "The Abandoned House" },
	{ "abandonedlab",       "The Abandoned Laboratory" },
	{ "abandonedneigh",     "The Abandoned Neighborhood" },
	{ "abandonhospital_ms", "Abandoned Hospital (by Michelle Santiago)" },
	{ "abandonhospital_an", "Abandoned Hospital (by Anonymous)" },
	{ "aboringroom",        "A Boring Room" },
	{ "abramelin",          "Abramelin" },
	{ "acadecwars",         "Acadec Wars" },
	{ "accusations",        "Accusations!" },
	{ "achangeofheart",     "A Change of Heart" },
	{ "achilles",           "Achilles" },
	{ "aclutteredroom",     "A Cluttered Room" },
	{ "acoldfrostymorning", "A Cold Frosty Morning" },
	{ "aconversation",      "A Conversation" },
	{ "adamsplashes",       "Adam Splashes" },
	{ "adayforspelling",    "A Day for Spelling" },
	{ "adayinthealex",      "A Day in the Alex" },
	{ "adayintheoffice",    "A Day in the Office" },
	{ "adaylifejeymarlowe", "A Day in the Life of Joey Marlowe" },
	{ "adayinthelife",      "A Day in the Life of a Teenage Wizard" },
	{ "advancedastronomy",  "Advanced Astronomy and Grief Counseling" },
	{ "adventcrowther",     "Will Crowther’s Adventure" },
	{ "adventurapf",        "Adventura" },
	{ "adventurenews",      "Adventure News" },
	{ "adventurerback",     "An Adventurer's Backyard" },
	{ "adventuresinspace",  "Adventures in Space" },
	{ "adventuresww",       "Adventures at WebWise" },
	{ "adwenturesprincess", "Adwentures of Princess" },
	{ "afairytaleoddity",   "A Fairytale Oddity" },
	{ "aforgottenmurder",   "A Forgotten Murder" },
	{ "aforgottenworld",    "A Forgotten World" },
	{ "againsthingsending", "Against All Things Ending" },
	{ "agame",              "A Game" },
	{ "agameaboutyou",      "A Game About You" },
	{ "agameoffiefs",       "A Game of Fiefs" },
	{ "agenerallybadday",   "A Generally Bad Day" },
	{ "agescat1stadv",      "AGES: Cat's First Adventure" },
	{ "agrovegame",         "A Grove Game" },
	{ "ahole",              "A Hole" },
	{ "aholenewworld",      "A Hole New World of Stupidity" },
	{ "ahorrificadvcomedy", "A Horrific Adventure in Comedy" },
	{ "ahouse",             "A House" },
	{ "ahousetour",         "A House Tour" },
	{ "ajourney2paradise",  "A Journey to Paradise" },
	{ "aknighthisquest",    "A Knight and His Quest" },
	{ "aknightsquest",      "A Knight's Quest" },
	{ "akonpliayiti",       "Akonpli Ayiti" },
	{ "alexeivsevilpeps",   "Alexei versus the Evil Peps" },
	{ "aliceintwistedland", "Alice in Twistedland" },
	{ "alienabductescape",  "Alien Abduction Escape" },
	{ "alilaboratoryroom",  "Ali Laboratory Room" },
	{ "allaboard",          "All Aboard!" },
	{ "allerton",           "Allerton" },
	{ "alleywaypossums",    "Alleyway Possums" },
	{ "alloneneedscarrot",  "All One Needs Is a Carrot" },
	{ "alone",              "Alone" },
	{ "alonglayover",       "A Long Layover" },
	{ "alostmemory",        "A Lost Memory" },
	{ "alternianforest",    "Alternian Forest" },
	{ "almostadream",       "Almost a Dream" },
	{ "amatteroftiming",    "A Matter of Timing" },
	{ "amazetwistypass",    "A Maze of Twisty Passages" },
	{ "ameadow",            "A Meadow" },
	{ "aminedorm",          "Amine's Dorm" },
	{ "amnesiapf",          "Amnesia" },
	{ "amnesie",            "Amnesie" },
	{ "amodestproposal",    "A Modest Proposal" },
	{ "amrnecklace",        "AMR Necklace" },
	{ "amurder",            "A Murder" },
	{ "amurdermystery",     "A Murder Mystery" },
	{ "analchemicaladvent", "An Alchemical Adventure" },
	{ "anadventurertale",   "An Adventurer's Tale" },
	{ "anaponthebeach",     "A Nap on The Beach" },
	{ "anartistneeds",      "An Artist Needs Supplies" },
	{ "angeldustartgal",    "Angel Dust Art Gallery" },
	{ "animalapocalypse",   "Animal Apocalypse" },
	{ "animalfarm",         "Animal Farm" },
	{ "annoyathon",         "Annoyathon" },
	{ "anormalday",         "A Normal Day" },
	{ "anotherartgallery",  "Another Art Gallery" },
	{ "anotherolddude",     "Another Old Dude From the 80’s" },
	{ "anotsonormaltemple", "A Not-So-Normal Temple" },
	{ "ant129finalproject", "ANT129 Final Project" },
	{ "antidotes",          "Antidotes" },
	{ "aperfectdayforit",   "A Perfect Day for It" },
	{ "apocalypse",         "Apocalypse" },
	{ "apocalypsech1",      "Apocalypse Now - Chapters 1: Just Another Day" },
	{ "apocalypsech12",     "Apocalypse Now - Chapters 1 + 2" },
	{ "apocalypserising",   "Apocalypse Rising" },
	{ "applemansion",       "Apple Mansion" },
	{ "applestwoapples",    "Apples, Two Apples" },
	{ "apprenticeschg",     "Apprentice's Challenge" },
	{ "arainynight",        "A Rainy Night" },
	{ "arcade",             "The Arcade" },
	{ "arcadeescape",       "Arcade Escape" },
	{ "arealbeach",         "A Real Beach" },
	{ "areaone",            "Area One" },
	{ "arlocompendiumch1",  "Arlo & the Compendium: Chapter 1 - Hunger Pains" },
	{ "arlocompendiumch2",  "Arlo & the Compendium: Chapter 2 - The Search for Shock Fruit" },
	{ "armorsandbuffs",     "Armors and Buffs" },
	{ "aroundthehouse",     "Around the House" },
	{ "artiststhenewdeal",  "Artists in the New Deal" },
	{ "aseparatepeace",     "A Separate Peace" },
	{ "ash",                "Ash" },
	{ "ashorti7demo",       "A Short I7 Demonstration" },
	{ "aslowdayatwork",     "A Slow Day at Work" },
	{ "assasin",            "Assasin" },
	{ "assessmentmatters",  "Assessment Matters" },
	{ "astrangesdventure",  "A Strange Adventure" },
	{ "asurpriseparty",     "A Surprise Party" },
	{ "asylumpf",           "Asylum" },
	{ "ataleofbarizan",     "A Tale of Barizan" },
	{ "atestgame",          "A Test Game" },
	{ "atlantic",           "Atlantic" },
	{ "atob",               "A to B" },
	{ "atomicmeat",         "Atomic Meat" },
	{ "atourofthelouvre",   "A Tour of the Louvre" },
	{ "atrailhead",         "A Trailhead" },
	{ "atraingame",         "A Train Game" },
	{ "atriptoucla",        "A Trip to UCLA" },
	{ "attackkillerjarod",  "Attack of the Killer Jarod in a Care Bear Suit" },
	{ "atthelibrary",       "At the Library" },
	{ "audeldesapparences", "Au-Del Des Apparences" },
	{ "audioguide",         "Audioguide" },
	{ "augh",               "Augh!" },
	{ "austinhyattlivedie", "Austin and Hyatt's Live or Die" },
	{ "averybadjoke",       "A Very Bad Joke" },
	{ "averyimpstory",      "A Very Important Story" },
	{ "averyniceman",       "A Very Nice Man" },
	{ "averysmissingtrain", "Avery's Missing Train" },
	{ "averystupidgame",    "A Very Stupid Game" },
	{ "awake",              "Awake" },
	{ "awakeningpf",        "Awakening" },
	{ "awarriorcatsmm",     "A Warrior Cats MM!" },
	{ "awarning",           "A Warning, and Then Silence" },
	{ "awayout",            "A Way Out" },
	{ "babysteps",          "Baby Steps" },
	{ "babytroubles",       "Baby Troubles" },
	{ "bakerbonanza",       "Baker Bonanza" },
	{ "bananaquest",        "Banana Quest" },
	{ "barroombrawl_zcode", "Barroom Brawl" },
	{ "bartholimules",      "Bartholimules Camp" },
	{ "base0",              "Base 0" },
	{ "bearsadventure",     "Bear's Adventure" },
	{ "beastmode",          "Beast Mode" },
	{ "beepboopboopbop",    "Beep Boop Boop Bop" },
	{ "bellsloveforhermom", "Bell's Love for Her Mom Awakens" },
	{ "bensappartment",     "Ben's Appartment" },
	{ "birthdaycake",       "Birthday Cake" },
	{ "birthdayquestp1",    "Birthday Quest: Part 1" },
	{ "birthdaysurprise",   "Birthday Surprise" },
	{ "blankgame",          "Blank Game" },
	{ "bloodlust",          "Bloodlust" },
	{ "breachofcode",       "Breach of Code" },
	{ "breathdragon",       "Breath of the Dragon" },
	{ "breakunderscorein",  "Break-in" },
	{ "bruh",               "Bruh" },
	{ "busstop",            "Bus Stop" },
	{ "bymannualsession",   "BYM Annual Session: The Game" },
	{ "cabbages",           "Cabbages" },
	{ "campywakegne",       "Camp Ywakegne" },
	{ "candyland",          "Candy Land" },
	{ "cannibals",          "Cannibals" },
	{ "carcinogenesis",     "Carcinogenesis" },
	{ "cardinal",           "Cardina!" },
	{ "castle",             "Castle" },
	{ "caveescape",         "Cave Escape" },
	{ "ceremony",           "Ceremony" },
	{ "chaldeabuffet",      "Chaldea Buffet" },
	{ "cheesequest",        "Cheese Quest" },
	{ "chipaguccicollar",   "Chip, A Gucci Collar & Paris" },
	{ "clarionsun",         "Clarion Sun" },
	{ "clarionsun2",        "Clarion Sun II" },
	{ "classmurder",        "Class Murder" },
	{ "clownsinthedark",    "Clowns in the Dark" },
	{ "coffequest",         "Coffee Quest" },
	{ "coffeetime",         "Coffee Time!" },
	{ "coffinofchaos",      "Coffin of Chaos" },
	{ "colbysunusualday",   "Colby's Unusual Day" },
	{ "coleandmichael",     "Cole and Michael" },
	{ "colinexplore1984",   "Colin Explore 1984" },
	{ "colonialproject",    "Colonial Project" },
	{ "coma",               "Coma" },
	{ "complimentarypean",  "Complimentary Peanuts" },
	{ "comprehenszombie",   "A Comprehensive Zombie Pedagogy" },
	{ "concealedpassages",  "Concealed Passages" },
	{ "cipande",            "Confederation, Identity, Promise and Empire" },
	{ "courtyardmania",     "Courtyardmania" },
	{ "coverletter",        "Cover Letter" },
	{ "crashcourse",        "Crash Course" },
	{ "crashlanding",       "Crash Landing" },
	{ "crater308",          "Crater 308" },
	{ "crazytalk",          "Crazy Talk" },
	{ "creativeproject",    "Creative Project" },
	{ "cruiseshipmystery",  "Cruise Ship Mystery" },
	{ "cupcakeadventure",   "Cupcake Adventure" },
	{ "damien",             "Damien" },
	{ "dankception",        "Dankception" },
	{ "darenotspeak",       "Dare Not Speak" },
	{ "dark",               "Dark" },
	{ "darkforest",         "Dark Forest" },
	{ "datenightmare",      "Date Nightmare" },
	{ "david",              "David" },
	{ "dayalpha",           "Day Alpha" },
	{ "dayoff",             "Day Off" },
	{ "daythyme",           "Day Thyme" },
	{ "dcblues",            "DC Blues" },
	{ "deadication",        "Dead-ication" },
	{ "deadmansgrave",      "Dead Man's Grave" },
	{ "deadsprint",         "Dead Sprint" },
	{ "decisionsdecisions", "Decisions, Decisions" },
	{ "demiddeleeuwen",     "De Middeleeuwen" },
	{ "demonetization",     "Demonetization" },
	{ "demonhunterpf",      "Demon Hunter" },
	{ "deoxiousevent2015",  "Deoxious Event 2015" },
	{ "depressed",          "Depressed" },
	{ "detectiveconan",     "Detective Conan" },
	{ "diamondcrime",       "Diamond Crime" },
	{ "dilemme",            "Dilemme: Essayer de Mourir en Paix" },
	{ "dingdong",           "Ding-Dong" },
	{ "disenchantbaycanda", "Disenchantment Bay With Cannabalism and Afterlife" },
	{ "distractedwizard",   "Distracted Wizard!" },
	{ "doctorwhoadventure", "Doctor Who Adventures in London" },
	{ "dogleashes",         "Dog Leashes" },
	{ "dollhouse",          "Dollhouse" },
	{ "dontmakeshootback",  "Don't Make Us Shoot Back" },
	{ "dontpeedarthvader",  "Don't Pee on Darth Vader" },
	{ "donttouchmyhat",     "Don't Touch My Hat!" },
	{ "donttrytoescape",    "Don't Try to Escape" },
	{ "doors",              "Doors" },
	{ "doorstopinvasion",   "Doorstop Invasion" },
	{ "downtownswingers",   "Downtown Swingers" },
	{ "dragongems",         "The Dragon Gems" },
	{ "dragonquest",        "Dragon Quest" },
	{ "dreampet",           "Dream Pet" },
	{ "dressingup",         "Dressing Up" },
	{ "drunkennight",       "Drunken Night" },
	{ "dryingup",           "Drying Up" },
	{ "duplicatebiopunkif", "Duplicate a Biopunk IF" },
	{ "eastereggs",         "Easter Eggs" },
	{ "eateverything",      "Eat Everything!" },
	{ "echoesepoch",        "Echoes of Epoch" },
	{ "edu5cedtaskat2b",    "EDU5CED Task AT2b" },
	{ "eldariasurvival",    "Eldaria Survival" },
	{ "electroroom",        "Electricity Room" },
	{ "embassy",            "Embassy" },
	{ "emptypalace",        "Empty Palace" },
	{ "endandwin",          "Ending and Winning" },
	{ "endinggamedeath",    "Ending the Game Death or Victory" },
	{ "entinieblas",        "En Tinieblas" },
	{ "endofdays",          "End of Days" },
	{ "environmentaladv",   "Environmental Adventure" },
	{ "envstewardship",     "Environmental Stewardship" },
	{ "epicdragonbattle3k", "Epic Dragonbattle 3000" },
	{ "epicfreakout",       "EpicFreakOut" },
	{ "escapechucky",       "Escape Chucky" },
	{ "escapeemmalam",      "Escape Emma Lam" },
	{ "escapeforniteisle",  "Escape Fornite Island" },
	{ "escapegrimwoods",    "Escape from the Grimwoods" },
	{ "escapeloganli",      "Escape Logan Li" },
	{ "escapetheartmuseum", "Escape the Art Museum" },
	{ "escapethecolony",    "Escape the Colony" },
	{ "escapethecolosseum", "Escape From the Colosseum" },
	{ "escapethehouse",     "Escape the House!" },
	{ "escapemcolorroom",   "Escape the Multicolored Room" },
	{ "escapethepyramid",   "Escape the Pyramid" },
	{ "escapetherooms",     "Escape the Rooms" },
	{ "escapethewizcastle", "Escape the Wizard's Castle" },
	{ "escapetsunami",      "Escape From a Tsunami" },
	{ "escapedprisoner",    "Escaped Prisoner" },
	{ "escapingprison",     "Escaping Prison" },
	{ "experimentpf",       "Experiment" },
	{ "exploratriceperdue", "L'Exploratrice Perdue" },
	{ "exploreexamine",     "Explore and Examine" },
	{ "exterminsim2014",    "Exterminator Simulator 2014" },
	{ "extremehs",          "Extreme Hide and Seek" },
	{ "faction11",          "Faction 11" },
	{ "fairyring",          "Fairy Ring" },
	{ "feverdream",         "Fever Dream" },
	{ "finallythefestival", "Finally the Festival" },
	{ "findairplanekey",    "Find the Airplane Key" },
	{ "findjoker",          "Find Joker" },
	{ "findmealiger",       "Find Me a Liger!" },
	{ "findmystethoscope",  "Find My Stethoscope" },
	{ "findpurplesteve",    "Find Purple Steve" },
	{ "findthatchinchilla", "Find That Chinchilla" },
	{ "findthecookie",      "Find the Cookie" },
	{ "findyourjuprof",     "Find Your JU Professor" },
	{ "findingdora",        "Finding Dora" },
	{ "flosadventure",      "Flo's Adventure" },
	{ "fogwick",            "Fogwick" },
	{ "forourclass",        "For Our Class" },
	{ "forgottennight",     "Forgotten Night" },
	{ "foulplay",           "Foul Play" },
	{ "foxgoosecorn",       "Fox Goose Corn" },
	{ "franknovelrecrch1",  "Frankenstein Novel Recreation - Chapter 1" },
	{ "franknovelrecrch2",  "Frankenstein Novel Recreation - Chapter 2" },
	{ "franknovelrecrch3",  "Frankenstein Novel Recreation - Chapter 3" },
	{ "freeparking",        "Free Parking" },
	{ "freighterraphus",    "Freighter Raphus" },
	{ "fridayafteroffice",  "Friday Afternoon in the Office" },
	{ "friendlydebat",      "The Friendly Debat" },
	{ "fromportlandxoxo",   "From Portland Get XOXO" },
	{ "fungus",             "Fungus" },
	{ "futurebookshop",     "#futurebookshop" },
	{ "galaxia",            "Galaxia" },
	{ "galisancommence",    "Galisan, Le Commencement" },
	{ "gamerstereotypes",   "Gamer Stereotypes" },
	{ "garysgetaway",       "Gary's Getaway" },
	{ "generations",        "Generations" },
	{ "generationsend",     "Generations With Ending" },
	{ "genesis",            "Genesis" },
	{ "genesisevil",        "Genesis Evil" },
	{ "getbackhomewoods",   "Get Back Home From the Woods" },
	{ "getontrain",         "Get On the Train!" },
	{ "getoutalive",        "Get Out Alive" },
	{ "getoutofthehouse",   "Get Out of the House" },
	{ "getthatcat",         "Get That Cat!" },
	{ "getyourlifeback",    "Get Your Life Back" },
	{ "ghosttownpf",        "Ghost Town" },
	{ "gilberttheelephant", "Gilbert the Elephant and Albert the Fish" },
	{ "giornogiovannas",    "Giorno Giovanna's Worst Dream" },
	{ "gotakeashower",      "Go Take a Shower" },
	{ "goblinmarket",       "Goblin Market" },
	{ "goddammitcthulhu",   "Goddammit, Cthulhu" },
	{ "goldrush",           "Gold Rush" },
	{ "gondoladvent",       "Gondola Adventure" },
	{ "goodmourning",       "Good Mourning" },
	{ "goofyahh",           "Goofy Ahh!" },
	{ "gosandwich",         "Go Get a Sandwich at 2am" },
	{ "grandtheftfood",     "Grand Theft Food" },
	{ "gravitationquand",   "Gravitational Quandary" },
	{ "greatescape",        "Great Escape" },
	{ "griffynch1",         "Griffyn - Chapter One" },
	{ "griffynch2",         "Griffyn - Chapter Two" },
	{ "grocerymadness",     "Grocery Madness" },
	{ "gustavosbusiness",   "Gustavo's Business" },
	{ "halfdead",           "Half-Dead" },
	{ "halloweenadv",       "Halloween Adventure" },
	{ "halloweenhangover",  "Halloween Hangover" },
	{ "hannahinwonderland", "Hannah in Wonderland" },
	{ "hanze",              "Hanze" },
	{ "happybirthdaygon",   "Happy Birthday, Gon!" },
	{ "happycycling",       "Happy Cycling" },
	{ "harrowingdefiance",  "Harrowing Defiance" },
	{ "hauntedhallway",     "The Haunted Hallway" },
	{ "hauntedhousescape",  "Haunted House Escape" },
	{ "hauntedmansion",     "The Haunted Mansion" },
	{ "hellsisland",        "Hell's Island" },
	{ "helpsherlock",       "Help Sherlock!" },
	{ "henryhudson",        "Henry Hudson" },
	{ "herculesjourney",    "Hercules’ Journey" },
	{ "highschoolsurvival", "High School Survival" },
	{ "historicalwln",      "Historical Williamsburg Living Narrative" },
	{ "hogwartshome",       "Hogwarts Homecoming" },
	{ "honeymoondisaster",  "Honeymoon Disaster" },
	{ "hospital",           "Hospital" },
	{ "hotelgrindell",      "Hotel Grindell" },
	{ "housekeys",          "The House of Keys" },
	{ "houseoflafayette",   "House of Lafayette Street" },
	{ "housetreasure",      "House of Treasure Boxes" },
	{ "howdidineverknow",   "How Did I Never Know" },
	{ "howlsmovingcastle",  "Howl's Moving Castle" },
	{ "hungoverinbelfast",  "Hungover in Belfast" },
	{ "idontevenknow",      "I Don't Even Know" },
	{ "iliketrains",        "I Like Trains" },
	{ "illusion",           "Illusion" },
	{ "imprieteneasca",     "Imprieteneasca - Chapter 1: INCEPE" },
	{ "infiltrator",        "The Infiltrator Present Within Our Midst" },
	{ "inception",          "Inception" },
	{ "incognizance",       "Incognizance" },
	{ "inescapableroom",    "The Inescapable Room" },
	{ "inferno",            "Inferno" },
	{ "infinitydestiny",    "Infinity Destiny" },
	{ "inheritancepf",      "Inheritance" },
	{ "insaneasylum",       "Insane Asylum" },
	{ "insidemindscape",    "Inside Mindscape" },
	{ "inthebasement",      "In the Basement" },
	{ "isthisacowfantasy",  "Is This a Cow or Just Fantasy" },
	{ "isiscrisis",         "ISIS Crisis" },
	{ "islandhopper",       "Island Hopper" },
	{ "itsjustyou",         "Its Just You and Your Dog" },
	{ "jaccuse",            "J'Accuse!" },
	{ "jailbreak",          "Jail Break" },
	{ "jailescape",         "Jail Escape" },
	{ "jakesgame",          "Jake's Game" },
	{ "jellen",             "Jellen" },
	{ "joeyskey",           "Joey's Key" },
	{ "johntory",           "John Tory: Austere Mayor" },
	{ "journeyofosiris",    "Journey of Osiris" },
	{ "journeyofosirisp2",  "Journey of Osiris - Part II" },
	{ "juliuscaesar",       "Julius Caesar" },
	{ "junglesearch",       "Jungle Search" },
	{ "jurassicescape",     "Jurassic Escape" },
	{ "keepingdido",        "Keeping Dido" },
	{ "killerbear",         "Killer Bear" },
	{ "killthedisease",     "Kill the Disease!" },
	{ "kindergarten",       "Kindergarten" },
	{ "kingheartbuildraft", "Kingdom Hearts: Building the Raft" },
	{ "kingheartdangerous", "Kingdom Hearts: It's Dangerous to Go Alone" },
	{ "kingdomofamphibia",  "Kingdom of Amphibia" },
	{ "lanote",             "La Note" },
	{ "laboratoriodetalf",  "Laboratorio de TALF" },
	{ "laboratory321",      "Laboratory 321" },
	{ "lacrimosa",          "Lacrimosa" },
	{ "lamia",              "Lamia" },
	{ "laska",              "Dobrodruzsvi Pro Lasku!" },
	{ "lastnight",          "Last Night" },
	{ "lepetiteprince",     "Le Petite Prince" },
	{ "learnhowtodie",      "Learn How to Die" },
	{ "legacies",           "Legacies" },
	{ "leosrage",           "Leo's Rage" },
	{ "lesgaleriemaudites", "Les Galeries Maudites" },
	{ "lieblingshause",     "Lieblings Hause" },
	{ "lifeaftertesting",   "Life After Testing" },
	{ "lifecat",            "Life as a Cat" },
	{ "lifegoals",          "Life Goals" },
	{ "lifepf",             "Life" },
	{ "lifesqpv",           "The Life of SQPV" },
	{ "limbo",              "Limbo" },
	{ "lines",              "Lines" },
	{ "littleharmoniclab",  "Little Harmonic Labyrinth" },
	{ "littlered",          "Little Red" },
	{ "littleredplumbboy",  "Little Red Plumber Boy" },
	{ "livesoundadv",       "Live Sound Adventure" },
	{ "ljsawesomeadvent",   "LJs Awesome Adventure" },
	{ "locategoldendonut",  "Locate the Golden Donut!" },
	{ "lookingatzombies",   "Looking at Zombies" },
	{ "lookingforfood",     "Looking for Food" },
	{ "lookingforfulton",   "Looking for Fulton" },
	{ "lostandafraid",      "Lost and Afraid" },
	{ "lostaxe",            "Lost Axe" },
	{ "lostchicken",        "Lost Chicken" },
	{ "lostduck",           "Lost Duck" },
	{ "lostintheforest",    "Lost in the Forest" },
	{ "lostinthejungle",    "Lost in the Jungle" },
	{ "lostinthewoods",     "Lost in the Woods" },
	{ "lostkeys",           "Lost Keys" },
	{ "lostsoul",           "Lost Soul" },
	{ "lozlanternshadows",  "Legend of Zelda Lantern of Shadows" },
	{ "lukefeverdream",     "Luke's part of Fever Dream" },
	{ "lunchbreak",         "Lunch Break" },
	{ "macandcheese",       "Mac and Cheese" },
	{ "machineprojectadv",  "Machine Project Adventure" },
	{ "magicway",           "Magic Way" },
	{ "makemeamandwich",    "Make Me a Sandwich" },
	{ "makingcoffee",       "Making Coffee" },
	{ "makingmwavepopcorn", "Making Microwave Popcorn" },
	{ "manvspanda",         "Man vs Panda" },
	{ "manicomio",          "Manicomio" },
	{ "manor",              "Manor" },
	{ "mansionmath",        "Mansion Math" },
	{ "maxcrazymom",        "Max and Her Crazy Mom" },
	{ "mayantempleescape",  "Mayan Temple Escape" },
	{ "mcmansion",          "McMansion" },
	{ "medievaljailbreak",  "Medieval Jail Break" },
	{ "meetingtime",        "Meeting Time!" },
	{ "memoriespf",         "Memories" },
	{ "micasa",             "Mi Casa" },
	{ "middeleeuwsdrama",   "Een Middeleeuws Drama" },
	{ "midnightlibrary",    "Midnight Library" },
	{ "mirointeractgaming", "Miro Interactive Gaming" },
	{ "misssticklebushs",   "Miss Sticklebush's" },
	{ "missingpenatschool", "Missing Pen at School" },
	{ "mistertulkinghorn",  "Mister Tulkinghorn" },
	{ "module4computing",   "Module 4: Computing" },
	{ "momentsnoticed",     "Moments Noticed" },
	{ "monhistoiretroptop", "Mon Histoire Trop Top" },
	{ "monalisa",           "Mona Lisa" },
	{ "mondaymorning",      "Monday Morning" },
	{ "moneyandwater",      "Money and Water" },
	{ "moonriver",          "Moon River" },
	{ "mossybricks",        "Mossy Bricks" },
	{ "motel6",             "Motel 6" },
	{ "mrmoore",            "Mr.Moore..." },
	{ "multimodalproject",  "Multimodal Project" },
	{ "mummiesroom",        "Mummy Room" },
	{ "murderinbigapple",   "Murder in the Big Apple" },
	{ "murderincourtyard",  "Murder in the Courtyard" },
	{ "murdermansion",      "Murder Mansion" },
	{ "murdermystery",      "Murder Mystery" },
	{ "murderspicemansion", "A Murder at the Spice Mansion" },
	{ "mushrooms",          "Mushrooms" },
	{ "myjapanesemyth",     "My Japanese Myth" },
	{ "mysterymanorpf",     "Mystery Manor" },
	{ "mysteriousadv",      "The Mysterious Adventure" },
	{ "mysterysottomarin",  "Mystery of the Sottomarin" },
	{ "nautilisia_zcode",   "Nautilisia" },
	{ "needingawee",        "Needing a Wee" },
	{ "neithersense",       "Neither Sense Nor Sensibility" },
	{ "newdealhitman",      "New Deal Hitman" },
	{ "newsheriff",         "The New Sheriff" },
	{ "nickeldrophalley",   "Nickeldrop Halley" },
	{ "nigthbeforedusk",    "The Night Before Dusk" },
	{ "nightshiftz",        "Night Shift Z" },
	{ "notalone",           "Not Alone" },
	{ "nowlookwhatyoudone", "Now Look What You've Done!" },
	{ "october17th",        "October 17th" },
	{ "odiasvirtreality",   "Odias Virtual Reality" },
	{ "oneday",             "One Day" },
	{ "onelostraena",       "One Lost Raena" },
	{ "oneroomandafish",    "One Room and a Fish" },
	{ "onewayhome",         "One Way Home" },
	{ "operblackcobra",     "Operation Black Cobra" },
	{ "opto",               "OPTO One Person Train Operation" },
	{ "overpoweringdark",   "Overpowering Darkness" },
	{ "oxygenbreach",       "Oxygen Breach" },
	{ "palebluelight",      "Pale Blue Light" },
	{ "passiveaggrdragon",  "Passive Aggressive Dragon" },
	{ "peasantscreed",      "Peasant's Creeds" },
	{ "pedaltothemedal",    "Pedal to the Medal" },
	{ "persecution",        "The Persecution of the Unknown" },
	{ "piecethingtogether", "Piecing Things Together" },
	{ "piratesofhighseas",  "Pirates of the High Seas" },
	{ "pitchblack",         "Pitch Black" },
	{ "playwithrocco",      "Play With Rocco" },
	{ "playfictutorial",    "Playfic Tutorial" },
	{ "ploc",               "PLOC" },
	{ "poisoned",           "Poisoned" },
	{ "policyhobo",         "Policy Hobo" },
	{ "powerkeys",          "The Power of the Keys" },
	{ "prankken",           "Prank Ken" },
	{ "presidentialrace",   "Presidential Race!" },
	{ "princtextfghturbo",  "Princess Text Fighter Turbo" },
	{ "prisonbreakpf",      "Prison Break" },
	{ "prisonescape",       "Prison Escape" },
	{ "prisonerdrunkenera", "Prisoner of the Drunken Era" },
	{ "projectalfa",        "Projet Alfa" },
	{ "projetinform",       "Projet Inform" },
	{ "puzzleescaperoom",   "Puzzle Escape Room" },
	{ "quarantine",         "Quarantine" },
	{ "quarantined",        "Quarantined" },
	{ "quentaria",          "Quentaria" },
	{ "questforadventure",  "Quest for Adventure" },
	{ "questofsword",       "Quest of Sword" },
	{ "readingbooks",       "Reading Books" },
	{ "readyforsummer",     "Ready for Summer" },
	{ "recherchedoudou",    "Recherche Doudou" },
	{ "r2verbosity",        "Recipe #2: Verbosity" },
	{ "r3slightlywrong",    "Recipe #3: Slightly Wrong" },
	{ "r4portroyal",        "Recipe #4: Port Royal" },
	{ "r5upandup",          "Recipe #5: Up and Up" },
	{ "r6starryvoid",       "Recipe #6: Starry Void" },
	{ "r7portroyal2",       "Recipe #7: Port Royal 2" },
	{ "r8unbuttonelaffair", "Recipe #8: The Unbuttoned Elevator Affair" },
	{ "r9portroyal3",       "Recipe #9: Port Royal 3" },
	{ "r10firstnamebasis",  "Recipe #10: First Name Basis" },
	{ "r11midsummerday",    "Recipe #11: Midsummer Day" },
	{ "r12tamed",           "Recipe #12: Tamed" },
	{ "r13disenchantbay",   "Recipe #13: Disenchantment Bay" },
	{ "r14replanting",      "Recipe #14: Replanting" },
	{ "r15disenchantbay2",  "Recipe #15: Disenchantment Bay 2" },
	{ "r16disenchantbay3",  "Recipe #16: Disenchantment Bay 3" },
	{ "r17disenchantbay4",  "Recipe #17: Disenchantment Bay 4" },
	{ "r18laura",           "Recipe #18: Laura" },
	{ "r19disenchantbay5",  "Recipe #19: Disenchantment Bay 5" },
	{ "r20escape",          "Recipe #20: Escape" },
	{ "r21garibaldi",       "Recipe #21: Garibaldi" },
	{ "r22disenchantbay6",  "Recipe #22: Disenchantment Bay 6" },
	{ "r23neighborwatch",   "Recipe #23: Neighborhood Watch" },
	{ "r24disenchantbay7",  "Recipe #24: Disenchantment Bay 7" },
	{ "r25downbelow",       "Recipe #25: Down Below" },
	{ "r26peugeot",         "Recipe #26: Peugeot" },
	{ "r27disenchantbay8",  "Recipe #27: Disenchantment Bay 8" },
	{ "r28hover",           "Recipe #28: Hover" },
	{ "r29disenchantbay9",  "Recipe #29: Disenchantment Bay 9" },
	{ "r30belfry",          "Recipe #30: Belfry" },
	{ "r31gopherwood",      "Recipe #31: Gopher-Wood" },
	{ "r32disenchantbay10", "Recipe #32: Disenchantment Bay 10" },
	{ "r33disenchantbay11", "Recipe #33: Disenchantment Bay 11" },
	{ "r34brown",           "Recipe #34: Brown" },
	{ "r35disenchantbay12", "Recipe #35: Disenchantment Bay 12" },
	{ "rabbitchase",        "Rabbit Chase" },
	{ "reallylatest",       "The Really Latest Sandbox" },
	{ "redsea",             "The Red Sea" },
	{ "refuge",             "Refuge" },
	{ "reign",              "Reign" },
	{ "retribution",        "Retribution" },
	{ "retrofatale",        "Retro Fatale" },
	{ "roadstoroam",        "Roads to Roam" },
	{ "romanadventure",     "Roman Adventure" },
	{ "romancolosseum",     "Roman Colosseum" },
	{ "ronswoodshop",       "Rons Wood-Shop" },
	{ "rooms",              "Rooms" },
	{ "ruinsremixed",       "Ruins Remixed" },
	{ "safadventurev1",     "Shimpy's Almost-Fantastic Adventure Volume 1: Rinsttripolous" },
	{ "safeskies",          "Safe Skies" },
	{ "samsonrobots",       "Samson and the Robots" },
	{ "scatterbrain",       "Scatterbrain" },
	{ "schooltingz",        "School Tingz" },
	{ "scifistorynewmedia", "Sci-Fi Story for New Media" },
	{ "scp173",             "SCP-173" },
	{ "seadeep",            "The Sea is Deep" },
	{ "searchmagicalbox",   "Search For the Magical Box" },
	{ "secretdoors",        "Secret Doors" },
	{ "secretswithin",      "Secrets Within Closed Doors" },
	{ "segradocorazon",     "Sagrado Corazon de la Molina" },
	{ "selkiesnacks",       "Selkie Snacks" },
	{ "sepiadreams",        "Sepia Dreams" },
	{ "sequencevents",      "Sequence od Events" },
	{ "shadowsflight",      "Shadow's Flight" },
	{ "sheepandshamans",    "Sheep and Shamans" },
	{ "sherlockic",         "Sherlock Holmes and the Indecipherable Cipher" },
	{ "sherylcrowadvent",   "Sheryl Crow Adventure" },
	{ "shootingstarfinder", "Shooting Star Finder" },
	{ "simulation",         "Simulation" },
	{ "sleepingpearl",      "The Sleeping Pearl" },
	{ "slenderforest",      "Slender Forest" },
	{ "smallgaim",          "Small Gaim" },
	{ "snufflewhereareyou", "Snuffles!! Where Are You?" },
	{ "sorcerersforest",    "Sorcerer's Forest" },
	{ "sorrymum",           "Sorry, Mum" },
	{ "soulwanderer",       "Soul Wanderer" },
	{ "sovietmarch",        "Soviet March" },
	{ "spaceescape",        "Space Escape" },
	{ "spatulasearch",      "Spatula Search" },
	{ "splittinganatom",    "Splitting an Atom" },
	{ "spy",                "S.P.Y." },
	{ "spyrothedragon",     "Spyro the Dragon" },
	{ "starcitizen",        "Star Citizen" },
	{ "stardefense1",       "Star Defence I: Attack of the Homeworld" },
	{ "staringintoabyss",   "Staring Into the Abyss" },
	{ "starsandbutts",      "Stars and Butts" },
	{ "startergame",        "Starter Game" },
	{ "stealingthestolen",  "Stealing the Stolen" },
	{ "steph",              "Steph" },
	{ "stolengem",          "The Stolen Gen" },
	{ "storything",         "Story Thing" },
	{ "strandedspcemonkst", "Stranded: A Space Monkey Story" },
	{ "strangertheseparts", "Stranger in These Parts" },
	{ "strife",             "Strife!" },
	{ "stuckinthesnow",     "Stuck in the Snow" },
	{ "suddenlyseeksusan",  "Suddenly (Seeking) Susan" },
	{ "sugarrush",          "Sugar Rush, The Sequel to Coffee Quest" },
	{ "suicideprevention",  "Suicide Prevention" },
	{ "summertimeshy",      "Summertime Shy" },
	{ "sunrise",            "Sunrise" },
	{ "survivalrun",        "Survival: Run From the Zombies" },
	{ "survival",           "Survival!" },
	{ "survivemathclass",   "Survive Math Class" },
	{ "survseafoamisle4",   "Survivor Seafoam Islands IV: Three Tribes, Three Shibes" },
	{ "sushichef",          "Sushi Chef" },
	{ "suspiciousinc4",     "Suspicious Incident #14" },
	{ "swordsstory",        "Sword's Story" },
	{ "takeyourmedicine",   "Take Your Medicine" },
	{ "takingcareyourself", "Taking Care of Yourself" },
	{ "teamvoyager",        "Where in the World Is Voyager Demo Prime" },
	{ "tearoom",            "Tea Room" },
	{ "templeescape",       "Temple Escape" },
	{ "terrorblood",        "Terrorblood" },
	{ "textadventinnyc",    "Text Adventuring in NYC" },
	{ "thanksgiving",       "Thanksgiving Silverware" },
	{ "thatdarnleprechaun", "That Darn Leprechaun" },
	{ "theabandonedmint",   "The Abandoned Mint" },
	{ "theanniversary",     "The Anniversary" },
	{ "thebabysitter",      "The Babysitter" },
	{ "thebambooheist",     "The Bamboo Heist" },
	{ "thebasement",        "The Basement" },
	{ "thebieberfevprob",   "The Bieber Fever Problem" },
	{ "thebiter",           "The Biter" },
	{ "theblue",            "The Blue" },
	{ "thebumblebeeband",   "The Bumblebee and the Bandaid" },
	{ "thecage",            "The Cage" },
	{ "thecellar",          "The Cellar" },
	{ "thechange",          "The Change" },
	{ "thechristgauntlet",  "The Christmas Gauntlet" },
	{ "thechristgauntlet2", "The Christmas Gauntlet - Part 2" },
	{ "thecraving",         "The Craving" },
	{ "thecursedfieldtrip", "The Cursed Field Trip" },
	{ "thedeathquentcomp",  "The Death of Quentin Compson!" },
	{ "thediabolocontrapt", "The Diabolical Contraption" },
	{ "thedream",           "The Dream?" },
	{ "thedreamterminal",   "The Dream Terminal" },
	{ "thedungdismember",   "The Dungeon of Dismemberment" },
	{ "theelements",        "The Elements" },
	{ "theescape",          "The Escape" },
	{ "thegameofsetanta",   "The Game of Setanta" },
	{ "thegreenhouse",      "The Greenhouse" },
	{ "thegumshoes1case",   "The Gumshoes First Case" },
	{ "thehardestdecision", "The Hardest Decision" },
	{ "theheist",           "The Heist" },
	{ "thehideworldmathew", "The Hidden World of Matthew Ford" },
	{ "thehospital",        "The Hospital" },
	{ "theinnermostproj",   "The Innermost Project" },
	{ "theislandpf",        "The Island" },
	{ "thejewelheist",      "The Jewel Heist" },
	{ "thejourneylifetime", "The Journey of a Life Time" },
	{ "thelabyrinthpf",     "The Labyrinth" },
	{ "thelibrarypf",       "The Library" },
	{ "thelightprince",     "The Light Prince" },
	{ "thelostbook",        "The Lost Book" },
	{ "themachine",         "The Machine" },
	{ "themafiagenerator",  "The Mafia Generator" },
	{ "themagiccircle",     "The Magic Circle" },
	{ "themountainfornost", "The Mountain of Fornost" },
	{ "themystdisappear",   "The Disappearance of the Livingstone Necklace" },
	{ "themythisisosiris",  "The Myth of Isis and Osiris" },
	{ "themythicalforest",  "The Mythical Forest" },
	{ "theonlywaytobesure", "The Only Way to Be Sure" },
	{ "thepackage",         "The Package" },
	{ "thepilot",           "The Pilot" },
	{ "theprogressivexorn", "The Progressive Xorn" },
	{ "thequestdragonlord", "The Quest for the Murderous Dragon Lord" },
	{ "theredenglishbind",  "The Red English Binder" },
	{ "theschool",          "The School" },
	{ "thesilence",         "The Silence" },
	{ "thesixgatewayhell",  "The Sixth Gateway to Hell" },
	{ "thesquawkingdead",   "The Squawking Dead" },
	{ "thesunwentdark",     "The Sun Went Dark" },
	{ "thesuperbowl50",     "The Super Bowl 50: Panthers vs. Patriots" },
	{ "thesustaintransp",   "The Sustainable Transportation" },
	{ "theterrosschism",    "The Terros Schism" },
	{ "thethingsivefelt",   "The Things I've Felt" },
	{ "thetwistedwood",     "The Twisted Wood" },
	{ "theultimatum",       "The Ultimatum" },
	{ "theuntoldstory",     "The Untold Story" },
	{ "thevirtgrandnation", "The Virtual Grand National" },
	{ "thewallet",          "The Wallet" },
	{ "thewizardcrystal",   "The Wizard and the Crystal Wand" },
	{ "thewizardslair",     "The Wizard's Lair" },
	{ "thewizardspotion",   "The Wizard's Potion" },
	{ "thewizardstrials",   "The Wizard's Trials" },
	{ "theworldgodyknows",  "The World God Only Knows" },
	{ "theyawninggrave",    "The Yawning Grave" },
	{ "thezoobreak",        "The Zoo Break" },
	{ "theseus",            "Theseus" },
	{ "thiswarofmine",      "This War of Mine" },
	{ "thosewhowander",     "Those Who Wander" },
	{ "threechoices3ways",  "Three Choices, Three Ways" },
	{ "thydungeonman",      "Thy Dungeonman" },
	{ "tiffanys",           "Tiffany's" },
	{ "timetraveller",      "The Time Traveller" },
	{ "timmyland",          "Timmy Land" },
	{ "tinygardens",        "Tiny Gardens" },
	{ "toavengethestars",   "To Avenge the Stars" },
	{ "toyourowndevices",   "To Your Own Devices" },
	{ "todayjustasalways",  "Today, Just as Always" },
	{ "tonyabottcrackcase", "Tony Abott Cracks the Case" },
	{ "toxicschool",        "Toxic School" },
	{ "trainhopneworleans", "Train Hopping in New Orleans" },
	{ "trainofthought",     "Train of Thought" },
	{ "trapped",            "Trapped" },
	{ "trappedcastle",      "Trapped in a Castle" },
	{ "treasurehunt",       "Treasure Hunt" },
	{ "treasurehunting",    "Treasure Hunting" },
	{ "ttcrats",            "TTC and Rats Old Game" },
	{ "tvdream",            "TV Dream" },
	{ "twilightmansion",    "Twilight Mansion" },
	{ "un1015lostsoul",     "UN 1015 Lost Soul" },
	{ "un1015tminus59min",  "UN1015 T-Minus 59 Minutes" },
	{ "unchartedisland",    "The Uncharted Island" },
	{ "undercoop",          "Undercoop" },
	{ "unejournecomeautre", "Une Journée Comme une Autre" },
	{ "unerebellion",       "Une Rebellion" },
	{ "unlock",             "Unlock" },
	{ "vacancesdeolie",     "Vacances de Folie" },
	{ "vengefulalexdmon",   "Vengeful Alex and D-mon" },
	{ "verge",              "Verge" },
	{ "villagelabyrinth",   "The Village and the Labyrinth" },
	{ "vinylrecords",       "Vinyl Records" },
	{ "voyageinteractif",   "Voyage Interactif" },
	{ "warrant",            "Warrant" },
	{ "weddinghunt",        "Wedding Hunt" },
	{ "wednesday",          "Wednesday" },
	{ "welcomeafterlife",   "Welcome to the Afterlife" },
	{ "whatswrong",         "What's Wrong With the Trees" },
	{ "whereiseverybody",   "Where is Everybody" },
	{ "whereismrhuman",     "Where is Mr. Human" },
	{ "whittonavenue",      "Whitton Avenue" },
	{ "wingsofthepigeon",   "Wings of the Pigeon" },
	{ "witchcamebridge",    "Witch Camebridge" },
	{ "wohingehstdu",       "Wohin Gehst Du?" },
	{ "wrenchedanddivine",  "Wrenched and Divine" },
	{ "wrenlaw_zcode",      "Wrenlaw" },
	{ "yemagykqueste",      "Ye Magyk Queste" },
	{ "youhavefreemanatee", "You Have to Free the Manatee" },
	{ "yourang",            "You Rang" },
	{ "yourgovneedsyou",    "Your Government Needs You" },
	{ "zombiefacuacademy",  "Zombies @Faculty Academy" },
	{ "zoocapture",         "Zoo Capture" },

	// ParserComp 2015
	{ "pc15_sixgrayrats",   "Six Gray Rats Crawl Up the Pillow" },                                      //      Entrant
	{ "pc15_sunburn",       "Sunburn" },                                                                //      Entrant

	// ParserComp 2021
	{ "pc21_acidrain",      "Acid Rain" },                                                              //    8th Place
	{ "pc21_fivebyfivia",   "Fivebyfivia Delenda Est" },                                                //    9th Place
	{ "pc21_dannydipstick", "Danny Dipstick" },                                                         //   13th Place

	// ParserComp 2022
	{ "pc22_imposstairs",   "The Impossible Stairs" },                                                  //    1st Place
	{ "pc22_alchemistgold", "Alchemist's Gold" },                                                       //    6th Place
	{ "pc22_yougetback",    "You Won’t Get Her Back" },                                                 //    9th Place

	// ParserComp 2023
	{ "pc23_searchlostark", "Search for the Lost Ark" },                                                //    1st Place
	{ "pc23_xou",           "Xenophobic Opposites, Unite!" },                                           //    7th Place

	// PunyJam #1
	{ "pj1_captaincutters", "Captain Cutter's Treasure" },                                              //    1st Place
	{ "pj1_djinniotr",      "Djinn on the Rocks" },                                                     //    2nd Place
	{ "pj1_pubhubbub",      "Pub Hubbub" },                                                             //    3rd Place
	{ "pj1_deathnumber4",   "Death Number Four" },                                                      //    4th Place
	{ "pj1_pubadventure",   "Pub Adventure!" },                                                         //    5th Place
	{ "pj1_arthurdayout",   "Arthur's Day Out" },                                                       //    7th Place
	{ "pj1_buccaneercache", "Buccaneer's Cache" },                                                      //    8th Place
	{ "pj1_thejob",         "The Job" },                                                                //      Entrant

	// PunyJam #2
	{ "pj2_morris",         "Morris" },                                                                 //    1st Place
	{ "pj2_opportunity",    "A Once in a Lifetime Opportunity" },                                       //    2nd Place
	{ "pj2_phobias",        "Face Your Fears" },                                                        //    3rd Place

	// PunyJam #3
	{ "pj3_submarinesabot", "Submarine Sabotage" },                                                     //    1st Place
	{ "pj3_a1rl0ck",        "A1RL0CK" },                                                                //    2nd Place
	{ "pj3_strikeforce",    "Strike Force" },                                                           //    3rd Place
	{ "pj3_lucidnight",     "Lucid Night" },                                                            //    4th Place
	{ "pj3_fallingpieces",  "Falling to Pieces" },                                                      //    5th Place
	{ "pj3_blorp",          "Blorp!" },                                                                 //    6th Place
	{ "pj3_seacoral",       "Sea Coral" },                                                              //    7th Place
	{ "pj3_cleangetaway",   "A Clean Getaway" },                                                        //    8th Place
	{ "pj3_fantadimension", "The Fantasy Dimension" },                                                  //      Entrant

	// PunyJam #4
	{ "pj4_rop",            "The Ruin of 0ceanus Pr1me" },                                              //    1st Place
	{ "pj4_cargobreach",    "Cargo Breach" },                                                           //    2nd Place
	{ "pj4_shaka",          "Shaka!" },                                                                 //    3rd Place
	{ "pj4_redux",          "Redux" },                                                                  //    4th Place
	{ "pj4_minimalgame",    "Minimal Game" },                                                           //    5th Place
	{ "pj4_pharaoh",        "Pharaoh" },                                                                //    6th Place

	// SeedComp! 2023
	{ "kingsball",          "The King's Ball" },                                                        //      Entrant

	{ nullptr, nullptr }
};
#if defined (hSE_TTS)
#define NONE GUIO3(GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES)
#define ENTRYS(ID, VERSION, MD5, FILESIZE) { ID, VERSION, MD5, FILESIZE, Common::EN_ANY, GUIO2(GUIO_NOMUSIC, GUIO_NOSUBTITLES) }
#else
#define NONE GUIO4(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES)
#define ENTRYS(ID, VERSION, MD5, FILESIZE) { ID, VERSION, MD5, FILESIZE, Common::EN_ANY, GUIO3(GUIO_NOSPEECH, GUIO_NOMUSIC, GUIO_NOSUBTITLES) }
#endif
#define ENTRY0(ID, VERSION, MD5, FILESIZE) { ID, VERSION, MD5, FILESIZE, Common::EN_ANY, NONE }
#define ENTRY1(ID, VERSION, MD5, FILESIZE, LANG) { ID, VERSION, MD5, FILESIZE, LANG, NONE }

#define FROTZ_TABLE_END_MARKER { nullptr, nullptr, nullptr, 0, Common::EN_ANY, "" }

const FrotzGameDescription FROTZ_GAMES[] = {
	// Infocom Games - English
	ENTRY0("abyss", "1-890320", "81bec49309ecd3fc19592ae1b9962af1", 109416),
	ENTRY0("amfv", "1-841226-FIRST", "b1cc4cb2ac0b88b69ee7ecbcf4b0382d", 33536),
	ENTRY0("amfv", "47-850313-PRELPHA", "d14659de5e36a80e9f3d3cdd66991ff8", 119622),
	ENTRY0("amfv", "84-850516-FULLALPHA", "21218f89a00bb983e54b83799349bd3f", 189346),
	ENTRY0("amfv", "131-850628-GAMMA", "2ff7e261155924014e8fb36177d319c9", 254918),
	ENTRY0("amfv", "77-850814", "b7ffaed0ca4a90450f92b34066133377", 262016),
	ENTRY0("amfv", "79-851122", "1e37dbcf7ccc9244dbfc3229796362f4", 262544),
	ENTRY0("ballyhoo", "97-851218", "7944e832a7d7b34037c7b6791de43dbd", 128556),
	ENTRY0("ballyhoo", "99-861014", "39113e174bd3779adcf40618b223d797", 128602),
	ENTRY0("beyondzork", "1-870412-ALPHA", "85d09ec960b302a07f2b0ff3be77ce26", 182636),
	ENTRY0("beyondzork", "1-870715-BETA", "9a069f0148d6d623bea59e6c0ceeaed0", 252548),
	ENTRY0("beyondzork", "47-870915", "ebad286696837d2f218d17289e4d4d60", 261952),
	ENTRY0("beyondzork", "49-870917", "a5547795def620d0a75a064f9a37ab2d", 261900),
	ENTRY0("beyondzork", "51-870923", "73948f415596fa4d9afe442b2c19e61f", 261548),
	ENTRY0("beyondzork", "57-871221", "c56cac07a500e5864a994b19286bc07c", 261388),
	ENTRY0("beyondzork", "60-880610", "b3667ab91363cc2c4727c1addeb1f54a", 261444),
	ENTRY0("borderzone", "9-871008", "189231ed0675f6be3be86856f49211af", 178372),
	ENTRY0("bureaucracy", "86-870212", "2bb00311d4c201082cfcd278ae5db921", 243144),
	ENTRY0("bureaucracy", "116-870602", "a8ae194257a989ed3d82648a507466f2", 243340),
	ENTRY0("bureaucracy", "160-880521", "2953a17183b14fda683fce764c639e13", 243524),
	ENTRY0("cutthroats", "23-840809", "059801d9f90fffeb3645816c37c7eda2", 112558),
	ENTRY0("cutthroats", "25-840917", "04fafe3602739e07696e48d57108b645", 112538),
	ENTRY0("deadline", "18-820311", "2fe2eae9e73f875955decb8db5ea9e7e", 111342),
	ENTRY0("deadline", "19-820427", "404c3f9bbea3236b68e6897fe7972352", 111420),
	ENTRY0("deadline", "21-820512", "09bed05d0dda9ca0a967569b946596c2", 111706),
	ENTRY0("deadline", "22-820809", "1610e84ca2505885566e648c1c525976", 111782),
	ENTRY0("deadline", "26-821108", "e1ae6af1098067b86076c34865ae713c", 108372),
	ENTRY0("deadline", "27-831006", "166ffb7cabc6b85f210655f371c89c46", 108454),
	ENTRY0("deadline", "28-850129", "5118a80c51407f741ecb8c8aec286124", 108420),
	ENTRY0("enchanter", "15-999999", "31dc4b6d146d0dbef02c81ffdf44425c", 109230),
	ENTRY0("enchanter", "10-830810", "7b41d915b4c2e31423d99925e9438aa4", 109126),
	ENTRY0("enchanter", "15-831107", "e70f21aad650dd196fa3601cab5e0fc5", 109230),
	ENTRY0("enchanter", "16-831118", "4de3468f358f88ad188c08ad213adefa", 109234),
	ENTRY0("enchanter", "16-840518-MAC", "7665971d211d70954a651bcb109db231", 109332),
	ENTRY0("enchanter", "24-851118", "9e84697a6b1141f7d6f10cf781b180b2", 108638),
	ENTRY0("enchanter", "29-860820", "f87cdafad3682ead25cfc473656ff713", 111126),
	ENTRY0("genericinfocom", "6-850705", "4942d656a6746cb346be13ab5c07beb8", 31772),
	ENTRY0("genericinfocom", "3-851007", "e16e63cd484ca075272f0d681d399af3", 26438),
	ENTRY0("genericinfocom", "5-870612", "1bb233979608d9a1e3340934c86b58cf", 30852),
	ENTRY0("hhgttg", "108-840809-BETA1", "e6eba2bf5e3c29fab537ca090ac3b347", 107452),
	ENTRY0("hhgttg", "119-840822-BETA2", "db2b4499fa6ac65be9260f591bac0eee", 110982),
	ENTRY0("hhgttg", "47-840914", "fdda8f4239819402c62db866bb61a648", 112622),
	ENTRY0("hhgttg", "56-841221", "a214fcb42bc9f554d07d983a12f6a062", 113444),
	ENTRY0("hhgttg", "42-850323", "f7799774b62873a1fd11b882734fbb6c", 113444),
	ENTRY0("hhgttg", "58-851002", "e867d49ad1fb9406ff4e0678a4ee2ac9", 113332),
	ENTRY0("hhgttg", "59-851108", "34f6abc1f2a42be127ef434fc475f0ee", 113334),
	ENTRY0("hhgttg", "60-861002", "425c640897c6ba0c175520e048fb4696", 113330),
	ENTRY0("hhgttg", "31-871119-INVCLUES", "379022bcd4ec74b90274c6100c33f579", 158412),
	ENTRY0("hollywoodhijinx", "235-861118-GAMMA", "0f8d16f1b9d00e1792bcf02822cc36ea", 118626),
	ENTRY0("hollywoodhijinx", "37-861215", "7b52824057ae24e098c228c41460ef75", 109650),
	ENTRY0("hypochondriac", "1-840427", "5de48da52ea773002d25562f074f5968", 122624),
	ENTRY0("hypochondriac", "2-840505", "fb767dcbe5436587b6f7a283ae9a3fa1", 123294),
	ENTRY0("hypochondriac", "10-840826", "aa4c0dcf120d6d39a7e7863db7342f2b", 100592),
	ENTRY0("hypochondriac", "11-870225", "a05a1e4b23d107cf3999fae25116ff49", 52376),
	ENTRY0("infidel", "22-830916", "38f713e53af720624434529ea780040c", 93556),
	ENTRY0("infidel", "22-840522-MAC", "1e5279c762c2521ff95dca6eea2177c0", 94144),
	ENTRY0("infocomsample1z", "15-840330", "0dea0c4d7e87b18a97c5bee0c61ac857", 62326),
	ENTRY0("infocomsample3zpi", "5-840512", "a72086f6e7c10aff5cdd199efca32f93", 105196),
	ENTRY0("infocomsample4zpiw", "24-840627", "1ba87ecd487866c4fd7775665dd383a8", 112478),
	ENTRY0("infocomsample4zpiw", "26-840731", "5483febc51abd55fb5e04c4c97a0b260", 112610),
	ENTRY0("infocomsample4zpiw", "52-850402", "239897f0eb9ba5995e9978684829047b", 126976),
	ENTRY0("infocomsample4zpiw", "53-850407", "47b8b8394e25faec870a798145529688", 126708),
	ENTRY0("infocomsample4zpiw", "55-850823", "05d9d1a1c3c73fce9e24ab695ece16c8", 126902),
	ENTRY0("infocomsample1b", "8-870119", "1fb0738cbf5d620cf8625ba9d9904f3b", 55472),
	ENTRY0("infocomsample2zt", "8-870601", "148ea96bfd110c87a96cb011007f0825", 106094),
	ENTRY0("infocomsample3ztl", "97-870601", "201fa230a942df5aa75bb5b5f609e8ce", 125314),
	ENTRY0("journey", "46-880603-DEV", "7bc3d57b3b6bf01788c07b2c4f007643", 280472),
	ENTRY0("journey", "142-890205-DEV", "2327731ae4a2f2c058ed88f9bfeb493c", 281088),
	ENTRY0("journey", "2-890303", "490df9e507aacba8fd113baabefb6e58", 280936),
	ENTRY0("journey", "11-890304", "66b69677c8d5635396d0c82a1d8bc7d8", 281632),
	ENTRY0("journey", "3-890310", "90dbbc0c9ae99722503f6c576924664e", 281376),
	ENTRY0("journey", "5-890310", "3276e33bce7ca80cf8ddc938038bddc0", 281896),
	ENTRY0("journey", "10-890313", "d14efde55d1708e6dbe512c52977a49c", 279792),
	ENTRY0("journey", "26-890316", "3720911d1374dbc4f39ea278d9bb425d", 279872),
	ENTRY0("journey", "30-890322", "c9893bc0399080bd3850d4db2120d110", 280472),
	ENTRY0("journey", "51-890522", "a8d6d5a2e02207077d6337117ff53d5b", 281600),
	ENTRY0("journey", "54-890526", "9de9ef986f216742d911e05c7a2d0698", 281600),
	ENTRY0("journey", "76-890615", "e292ce33c20239a78d80d131991da738", 282152),
	ENTRY0("journey", "77-890616", "8a4ab56f62e1b7c918b837794182dbcd", 282176),
	ENTRY0("journey", "79-890627", "154bca5d94fb6dd9a900dc369a78a2e7", 282624),
	ENTRY0("journey", "83-890706", "c33ea33ab8aec6c617734dcfe1211067", 282312),
	ENTRY0("lgop", "0-XXXXXX", "69b3534570851b90d7f53ebe9d224a6a", 128998),
	ENTRY0("lgop", "59-000001", "313ab8bdf57c53db77701cf92de1d9e1", 129022),
	ENTRY0("lgop", "1-851008-FIRST", "f7458650d0aebc9de098c38cb65c50f8", 30804),
	ENTRY0("lgop", "57-860121-ALPHA", "a9083c05dfaa578c4ba14141b52ece1b", 99484),
	ENTRY0("lgop", "118-860325-BETA", "98200100ddf825834b489cae3231b330", 129012),
	ENTRY0("lgop", "160-860521-GAMMA", "d825fd83de7e891b8a1327f2b4248fca", 128998),
	ENTRY0("lgop", "50-860711", "e7343e45923a9ffd92d34e55953c59ae", 128998),
	ENTRY0("lgop", "59-860730", "e81237e220a612c5a93fbcc1fdf85a0a", 129022),
	ENTRY0("lgop", "4-880405-INVCLUES", "6bdae7434df7c03f3589ece0bed3317d", 159928),
	ENTRYS("lurkinghorror", "203-870506", "e2d2505510479fec0405727e3d0abc10", 128986),
	ENTRYS("lurkinghorror", "219-870912", "83936d75c2cfd71fb64bf63c4696b9ac", 129704),
	ENTRYS("lurkinghorror", "221-870918", "c60cd0bf3c6eda867241378c7cb5464a", 129944),
	ENTRY0("minizork1", "2-840207", "9e228ce8bda609c92dfb7ab61226176a", 47676),
	ENTRY0("minizork1", "34-871124", "0d7700679e5e63dec97f712698610a46", 52216),
	ENTRY0("minizork2", "2-871123", "9f8a4e2d445bcdab19b84096e82e65e0", 55690),
	ENTRY0("moonmist", "65-86082X-BETA", "230a872dae12a68a7238a6812b70881c", 128986),
	ENTRY0("moonmist", "65-XXXXXX-BETA", "56f5a2e761230cd05704e81564c06578", 128986),
	ENTRY0("moonmist", "4-860918", "284797c3025ffaf76aecfa5c2bbffa86", 129002),
	ENTRY0("moonmist", "9-861022", "698475de2769c66bc5a1eca600c71561", 128866),
	ENTRY0("moonmist", "13-880501", "5e866f7089a294c4a65ae0e399f47079", 128828),
	ENTRY0("nordbert", "19-870722", "da1e189e19e3b24b2e35bd41fc32d261", 170284),
	ENTRY0("nordbert", "20-870722", "0236abbb84383dd989db31ef5b85a14f", 170300),
	ENTRY0("planetfall", "1-830517-BETA", "e042e9c418e799f95540a5849f4c240a", 107958),
	ENTRY0("planetfall", "20-830708", "15815c461a8548b7630d2aee46d07cc7", 107958),
	ENTRY0("planetfall", "26-831014", "cf6ce61eb2eff9d4f18d7bcba7c12cfb", 108674),
	ENTRY0("planetfall", "29-840118", "9facd8b974e658520fb762af4c4789dc", 109052),
	ENTRY0("planetfall", "37-851003", "01844816673414c97d21dc003304989b",109398),
	ENTRY0("planetfall", "39-880501", "4abc1e6cb494548fe3b0dac5a2bd82e2",109282),
	ENTRY0("planetfall", "10-880531-INVCLUES", "34c69f1d24418fd4d2de195a1d7546c4", 136560),
	ENTRY0("questforexcalibur", "40-890502", "dfa0fda9da21573cb0f116bd3646526e", 235520),
	ENTRY0("questforexcalibur", "41-890504", "5525991d5e8910340dd4f5b4f21e8c49", 235520),
	ENTRY0("questforexcalibur", "54-890606", "ced2c66d03a49de0e8190b468332f081", 271360),
	ENTRY0("questforexcalibur", "63s890622", "8545fdb7a35f20761b6ab79ffbccfe1e", 271304),
	ENTRY0("questforexcalibur", "74-890714", "13d13f375f85a874c82a8ac7ad69dc41", 269200),
	ENTRY0("plunderedhearts", "26-870730", "fe5b9eb91949d41838166364f1753b10", 128962),
	ENTRYS("restaurant", "15-880512", "12f5fe83d6fe2c1a6f50b2039bc5b28c", 62928),
	ENTRYS("restaurant", "184-890412", "7726dafb083dd86ee6fa7dfeec6e1ba6", 79152),
	ENTRY0("seastalker", "86-840320-BETA", "64fb27e7b9fd682ff4f0d0ec6616a468", 116456),
	ENTRY0("seastalker", "15-840501", "acbddf156f72a0484d69ff813a0c2215", 117738),
	ENTRY0("seastalker", "15-840522-MAC", "050961fa7788c309bbf40accbff2ffdf", 117728),
	ENTRY0("seastalker", "15-840612-COCO", "a4c9f678db4c2ecd28b5b9b933dbb2aa", 117736),
	ENTRY0("seastalker", "15-840716-TANDY", "cc9d1318bede361a932a9dfd180c825c", 117752),
	ENTRY0("seastalker", "17-850208-ATARI", "c193d3e8bffd7fec6ad2080f4a5c9319", 117180),
	ENTRY0("seastalker", "17-850208-NONATARI", "a5351a0091cf750f33a94101e5257fe8", 117180),
	ENTRY0("seastalker", "16-850515", "eb39dff7beb3589c8581dd2e3569eb78", 117752),
	ENTRY0("seastalker", "16-850603", "bccf194b1e823e37db2431b586662773", 117763),
	ENTRY0("seastalker", "16-850603-REG", "bccf194b1e823e37db2431b586662773", 117762),
	ENTRY0("seastalker", "18-850919", "0b005a2332ed2043b735391cf2bfa8eb", 116802),
	ENTRYS("sherlockriddle", "97-871026-DEV", "5f9e85dc0a3c327eb1b3c22075ff0d3c", 199248),
	ENTRYS("sherlockriddle", "4-880324-NOSOUND", "d615e87065ac281da5791f0e2762d2ca", 189220),
	ENTRYS("sherlockriddle", "22-880112", "fc12c3e68c53f0afe94e63fecbaf93ed", 189016),
	ENTRYS("sherlockriddle", "26-880127", "2cb2bda2e34eb7f9494cb585720e74cd", 190180),
	ENTRYS("sherlockriddle", "21-871214-NOSOUND", "69862f7f07a4e977159ea4da7f2f2ba6", 188444),
	ENTRY0("shogun", "278-890209", "08f2ebbae35188c8f5ff99fa94ea29db", 334248),
	ENTRY0("shogun", "278-890211", "cc824bc2ee24cfe4f9cc4bc06a375141", 332688),
	ENTRY0("shogun", "279-890217", "89be9830bfd23f2d9af4e498f85dea08", 333896),
	ENTRY0("shogun", "280-890217", "1e1e38dc396c163d6951a748d7b2d987", 333992),
	ENTRY0("shogun", "281-890222", "c264163a88862443bee4c30d69c545c9", 335024),
	ENTRY0("shogun", "282-890224", "afd0a53cf5539392eb8ec3019f70622b", 338128),
	ENTRY0("shogun", "283-890228", "5975af18a92941b6df733ff32a9f55db", 339104),
	ENTRY0("shogun", "284-890302", "1c6ee360c4afed30e0b264e3c6edacd6", 340080),
	ENTRY0("shogun", "286-890306", "3fb29606adfc8e02f3262ace301c30bd", 341224),
	ENTRY0("shogun", "288-890308", "059bc98469a4297ff7c27146ed7fed05", 341712),
	ENTRY0("shogun", "289-890309", "65ce020bc06fcaa842ca3d8de114e20c", 342272),
	ENTRY0("shogun", "290-890311", "6b4337e36dd44bb193430d2acca24075", 341312),
	ENTRY0("shogun", "291-890313", "30ce16727b322d0cf2bc81c9d44eeb0c", 341440),
	ENTRY0("shogun", "292-890314", "4efdca949654b0bfbc639091ee985478", 341416),
	ENTRY0("shogun", "295-890321", "561250669091e1d0df1e77482a38119a", 341912),
	ENTRY0("shogun", "311-890510", "8a2a436689f7337c307f74b70c368adf", 344224),
	ENTRY0("shogun", "320-890627", "54cc1148c99ee4847c493c3f1d7b6323", 345088),
	ENTRY0("shogun", "321-890629", "8d45abc0b07c9aa585dc0ebd28cf6595", 345088),
	ENTRY0("shogun", "322-890706", "62cca41feb94082442026f44f3e48e19", 344816),
	ENTRY0("sorcerer", "67-831208-BETA", "9a5d9d01d6a2210c85f2a71d3aced075", 107096),
	ENTRY0("sorcerer", "67-000000-BETA", "cfa5f1d06daf288879c704285b4837a7", 107096),
	ENTRY0("sorcerer", "85-840106-GAMMA", "39c4a0142f21d7a50b53ecee4579b970", 108520),
	ENTRY0("sorcerer", "4-840131", "d4a914fdfe90f5cd055a03b9aa24addd", 109734),
	ENTRY0("sorcerer", "6-840508", "7ee357c10a9e049fe7c641a4817ee575", 109482),
	ENTRY0("sorcerer", "13-851021", "7a076459806eaee72015b2b2882a89dc", 108692),
	ENTRY0("sorcerer", "15-851108", "cde88a011d2ba183ff69b47b0d8881c6", 108682),
	ENTRY0("sorcerer", "18-860904", "de8526f15c69462ec41918c4524666f8", 111052),
	ENTRY0("spellbreaker", "63-XXXXXX", "1f91a9573352377137b37afe44eb56f2", 128480),
	ENTRY0("spellbreaker", "63-850916", "b7b9eef231dee03fb40a9d98416fa0d5", 128480),
	ENTRY0("spellbreaker", "86-860829", "13b74da4b71586af34fb0974dbd470a5", 128928),
	ENTRY0("spellbreaker", "87-860904", "852286847f4cdd790075fa824260ff4e", 128916),
	ENTRY0("starcross", "17-XXXXXX", "57b93a825b3011c8b4fbef5337c06fbc", 83792),
	ENTRY0("starcross", "15-820901", "fb2e6d9a0ad5822f3a8d4aec949e4e3c", 84984),
	ENTRY0("starcross", "17-821021", "ed1e62e1f0eb9d819be45c076c5729f7", 83792),
	ENTRY0("starcross", "18-830114", "99b605d973984fbb3b5ec25c0f0dc8df", 84740),
	ENTRY0("stationfall", "1-861017-FIRST", "593b1578e9947b0ee15208cee9cdeba7", 41718),
	ENTRY0("stationfall", "87-870326-GAMMA", "30ab0408b85e4aa3d2f861d6a713a439", 127400),
	ENTRY0("stationfall", "63-870218-BETA", "fb18eec4bbfb1e4bb3e4a1afe42915c9", 121724),
	ENTRY0("stationfall", "107-870430", "cfadfb66afabaa2971ec9b4ae65266ca", 128934),
	ENTRY0("suspect", "14-000000", "94df6e910558b3c514e42f9a8d4075dd", 118692),
	ENTRY0("suspect", "14-841005", "3d759ccb19233f51968fa79d7374b393", 118692),
	ENTRY0("suspect", "14-841005-ATARI", "ed4b2cbc0f2dc0f078f80b74cd99ab8a", 118692),
	ENTRY0("suspect", "18-850222-I189", "5c440600a7a9257ec7d25e27c285b5ea", 118746),
	ENTRY0("suspect", "18-850222-I190", "a569eb6a0ba6f0cc8b9e233ab5a09bff", 118746),
	ENTRY0("suspended", "5-XXXXXX", "f2bb804283739b2d01a4d470f6aeaad7", 105418),
	ENTRY0("suspended", "5-830222", "73765e7eb2837bb0be5736cb8d0318ab", 105418),
	ENTRY0("suspended", "7-830419", "3103f15bba641368ebc936959ae1eef4", 105500),
	ENTRY0("suspended", "8-830521", "b749d42462dfec21831b69635cd9c5e8", 105492),
	ENTRY0("suspended", "8-840521-ALT", "b0d34d1ee50830649a768ab18bdd0b74", 105584),
	ENTRY0("suspended", "8-840521-MAC", "6088ad7cb553626b52875a9b8e801312", 105584),
	ENTRY0("thewitness", "13-830524", "d2297ddfe2c1b976c1b0c381ab01e2b3", 102608),
	ENTRY0("thewitness", "18-830910", "a6e441b0b92a72537c830fed201267af", 103728),
	ENTRY0("thewitness", "20-831119", "07be3335f793021997e9d92b72cd9cf0", 104740),
	ENTRY0("thewitness", "21-831208", "f54adff437313a2ca85c175a42a2ff17", 104704),
	ENTRY0("thewitness", "22-840924", "1019b9b1e1aa2c6eda945d7d92c2073a", 104664),
	ENTRY0("thewitness", "23-840925", "faa2742e46178d318673de62b18f3db8", 104260),
	ENTRY0("trinity", "1-851202-ALPHA", "9ede1b2fbe6cbb36a6bc29682b30254c", 154460),
	ENTRY0("trinity", "1-860221-BETA", "5b6719acfec03e807480ff1cabe93e7c", 230484),
	ENTRY0("trinity", "14-860313-BETA", "3bcbbba07c00fcb837ade8c7f0575bc4", 241564),
	ENTRY0("trinity", "11-860509", "60aa18070a0bf1b4866672483e980d5d", 262016),
	ENTRY0("trinity", "12-860926", "5377dc1ee39f1c8ed572944f89946eb2", 262064),
	ENTRY0("trinity", "15-870628", "e01e991b5c021281cad5bff16f04f507", 261484),
	ENTRY0("wishbringer", "68-850501", "898b9b157ce8e54a0953366d6317fbd5", 128952),
	ENTRY0("wishbringer", "69-850920", "e7c0412c4b3bda39de438a02cbae3816", 128904),
	ENTRY0("wishbringer", "X165-880609", "7ecd21a2a7836adf86df11777fbecd17", 127980),
	ENTRY0("wishbringer", "23-880706-INVCLUES", "bec823084c5622e88eca5a886278d2a5", 164712),
	ENTRY0("ziptest", "40-840613", "96f603534f3904bc095653f66377bb7c", 3998),
	ENTRY0("ziptest", "12-890607", "588b0d5053f7c8fa8109de7e0df360b9", 16896),
	ENTRY0("ziptest", "13-890619", "bb354ceff1c47e58c0c916738bdfdb97", 16896),
	ENTRY0("zork0", "0-870831-FIRST", "1154e6d21541f22f2dba86eeb58b17c6", 50144),
	ENTRY0("zork0", "1-871030-OLDPARSER", "8077da8c708727d9f315b71e3c677234", 133796),
	ENTRY0("zork0", "74-880114-PREALPHA", "78c6ad490204e7d2f8a92a9d4dd1e28e", 183740),
	ENTRY0("zork0", "96-880224-ALPHA", "465f88e850a58a0346d103f107428075", 201496),
	ENTRY0("zork0", "153-880510", "6794d8fb34dba853d86e14eb39ed4673", 245600),
	ENTRY0("zork0", "242-880830-BETA", "1b6a1c53e1a9937d4283f6b8d8cb3b5b", 269536),
	ENTRY0("zork0", "242-880901", "f2029ae5993f19670df31f80934316fc", 269528),
	ENTRY0("zork0", "296-881019", "fca554c21542729c9d154c319af6307e", 295536),
	ENTRY0("zork0", "66-890111", "8bd80f976375052060c8b34a16af2cab", 299440),
	ENTRY0("zork0", "343-890217", "edff0d2a0a1fb37a778f127ed6c141c8", 297152),
	ENTRY0("zork0", "366-890323", "e787b2cad2d6f29fd812e737f75646e8", 296376),
	ENTRY0("zork0", "366-890323-DEMO", "b58c35dc2ba36d48fade99564922c7c3", 296376),
	ENTRY0("zork0", "383-890602", "32e3e7ec438dabe77df2351af6ece324", 299392),
	ENTRY0("zork0", "387-890612", "a736799f46dffa70fc19506d7fe7c973", 300032),
	ENTRY0("zork0", "392-890714-IBM", "212c32a5793567a0bda0dd75cbce1525", 300032),
	ENTRY0("zork0", "393-890714", "29fb0e090bbff7bc8e9661e55da69ae7", 299968),
	ENTRY0("zork0", "393-890714-DEMO", "12ec3055ff0b8ee81a0b57bcddf0ca47", 299968),
	ENTRY0("zork1", "2-AS000C", "0d8a7c4801b18db4289062ae716e55c5", 82944),
	ENTRY0("zork1", "5-XXXXXX", "dd5ba502b30189d03abfcfb9817dffe0", 82836),
	ENTRY0("zork1", "15-UG3AU5", "fa2d22304700898cb8de921f46ca4bc9", 78566),
	ENTRY0("zork1", "15-XXXXXX", "b1fa36d05b2a7379d83d843118bd087c", 131072),
	ENTRY0("zork1", "20-XXXXXX", "b222bed4a0ab2650135cba7f4b1b1c67", 75734),
	ENTRY0("zork1", "23-820428", "6ad3d9ab2874beefcbc53630e9261118", 75780),
	ENTRY0("zork1", "25-820515", "287a1ce17f458fb2e776499a13796719", 75808),
	ENTRY0("zork1", "26-820803", "285f1d7c5deb1a2f23825f63823d0777", 75964),
	ENTRY0("zork1", "28-821013", "83bb70d73f3b4b5c4a32d8588b2d0707", 76018),
	ENTRY0("zork1", "30-830330", "d6d8b3ae49a683a6fce2383a8bab36a5", 76324),
	ENTRY0("zork1", "75-830929", "b35bca8dd18f6312c7e54dcd7958d7e5", 84868),
	ENTRY0("zork1", "76-840509", "50ebf3c0c959ac2571c23cb7f7907c70", 84874),
	ENTRY0("zork1", "88-840726", "d708b6751126f3b2b7612c760f080d41", 84876),
	ENTRY0("zork1", "52-871125-INVCLUES", "869ae9eba82b443beaba085c5b3e130c", 105264),
	ENTRY1("zork1", "3-880113-BETA", "9f336c92c1fd392fc7e81288e5c9b6ab", 116216, Common::DE_DEU),
	ENTRY0("zork1", "119-880429", "cb92225f77e6f3e57f0c14da4fc62245", 86838),
	ENTRY1("zork1", "15-890613", "bdb8d3987eb822dd283350e3b8b3c80e", 131072, Common::DE_DEU),
	ENTRY0("zork2", "7-UG3AU5", "8243ce12e7b3ce24b150f34cc2cb472c", 85260),
	ENTRY0("zork2", "15-820308", "4b6ecc8e40243ddbd4cc19ef82304c3b", 82110),
	ENTRY0("zork2", "17-820427", "386f2cd937e0ca316695d6ddca521c78", 82368),
	ENTRY0("zork2", "18-820512", "a019dd721134b57f5926ee2adf634b55", 82422),
	ENTRY0("zork2", "18-820517", "6cafa0e5239a74aa120bb8e2c33441be", 82422),
	ENTRY0("zork2", "19-820721", "a5236020509af26b47c737e51ce884aa", 82586),
	ENTRY0("zork2", "22-830331", "600264d62720731283454592261ec3fe", 82920),
	ENTRY0("zork2", "22-840518-MAC", "4ddcb80880cd7ce1f424b2a11c6ef78b", 83128),
	ENTRY0("zork2", "23-830411", "6c2e766b553c127bb07f7a0f8fe03ae2", 81876),
	ENTRY0("zork2", "48-840904", "a5064c9c3ce0bc02f16e01d745f39b67", 89912),
	ENTRY0("zork2", "63-860811", "e8c881f167d98478e00b3304815d6dc1", 92524),
	ENTRY0("zork3", "10-820818", "ba4897f4d82ba08906295dd3aebf501a", 82334),
	ENTRY0("zork3", "12-821025", "3358d49a8fdc783d0d9c09f05f76d16b", 82396),
	ENTRY0("zork3", "15-830331", "2fb29e6f5eebb643f42714ca9086e145", 82558),
	ENTRY0("zork3", "16-830410", "4717f8ec2f08da7d438c05f1351d28bd", 81626),
	ENTRY0("zork3", "15-840518-MAC", "672b54d8f457bd3be32e41fc9e069d71", 82642),
	ENTRY0("zork3", "17-840727", "c5dc520f469771c59d193558d405341d", 82714),
	ENTRY0("zork3", "25-860811", "321e6ab10f08789c95367712e24e03e3", 87984),
	ENTRY0("ztuu", "ztuu-970828", "3a55991be19943a13da58a91cd3615eb", 102524),

	// English games
	ENTRY0("404life", "110524", "a4ee7ba2cb611e0ae3e413a6eb9dc4da", 506770),
	ENTRY0("43", "100409", "d90d274327032d1f1c5396cdbbcdffcc", 140288),
	ENTRY0("69105keys", "090302/z5", "decb7dd967b358c54f63c3777e4e86f6", 76800),
	ENTRY0("69105keys", "090302/zblorb", "bc343936e0e9d79a3736b200eadfc6ee", 195162),
	ENTRY0("7doctors", "981219", "b0c9a1205401b578dd245ce3920b296c", 124928),
	ENTRY0("7kids", "160430", "d1897d9dc8ee886b6291d3f68ab8272b", 247758),
	ENTRY0("905", "160430", "d46825116a6f56e14076713230ada85d", 87552),
	ENTRY0("905", "120724", "885acc1cf4ae18428d8a1998d57f2925", 87040),
	ENTRY0("9dancers", "040718", "8a7faee46b0d6f35cb0cbeb7ac8631b2", 99328),
	ENTRY0("aaaaaaaa", "080817", "ead4d097d838d8ca05463ae61babc95d", 131072),
	ENTRY0("aasmasters", "030410", "bb6645f33130c7881da72ff0bfb6fb5b", 68608),
	ENTRY0("accuse", "070321", "3a1098524f8d6964a5d7a2579e9367f1", 129528),
	ENTRY0("acheton", "111115", "5abc5f815b7f4f6031781a3e6126ef03", 250880),
	ENTRY0("aciftotorial", "151031", "5111132f57b4c882051a122525bed51c", 100352),
	ENTRY0("acorncourt", "970904", "119dc6466da205261efc1ef8e00d26d1", 55296),
	ENTRY0("acrobat", "090111", "adaf3516d405d7dc65cbff1986f41af2", 2068718),
	ENTRY0("acrossstars", "100329", "ef910f930a01ac4d24051b8fce549180", 524288),
	ENTRY0("acrossstarsclues", "100213", "d23e9bf6019f212ff324f4595f8b6177", 122880),
	ENTRY0("acrossstarshints", "070929", "32904e85480632a1764f4654b8f561f6", 121344),
	ENTRY0("addendum", "080611", "e6ec3704b04d638f53aaf5f082efb424", 199420),
	ENTRY0("adv", "150118", "c776a1a9a8122967160fb51a39485113", 431616),
	ENTRY0("adv350_zcode", "951220", "2c90badea5a69c4dfb702f2722f5e675", 94208),
	ENTRY0("adv350_zcode", "961209", "c31773e62d01b51af2c633d212ac5b4a", 97280),
	ENTRY0("adv350_zcode", "011123", "5d4867b23e904d22453f031c87d6dcf0", 118272),
	ENTRY0("adv350_zcode", "060321", "d00c3717a46734c2ae96fb8d2ad0a226", 138240),
	ENTRY0("adv350_zcode", "151001", "88fe704848a77346d196a46f7cfb3b38", 66414),
	ENTRY0("adv350_zcode", "160307", "5f800280865d57cbfeb66695e79dc9b9", 147456),
	ENTRY0("adv440", "160307", "af1ab807e8c438d0e548c6b4903c33a1", 193536),
	ENTRY0("adv550", "160307", "631ab32f7370e1c1e5b4e9fe6ad539fb", 231424),
	ENTRY0("adv551", "160307", "14f0d78e0a1c447cdf8ad6daf4d22a5b", 334336),
	ENTRY0("adv551", "171110", "24449bf1875c0b75491173ba4243bc99", 334848),
	ENTRY0("adventland", "961111", "21f1bd1815a8b3ba5730fe168ff88e59", 49152),
	ENTRY0("adventurelobjan", "961209", "e8cdbc003c5bc0965ad2f04a4a7806db", 147968),
	ENTRY0("adventuretime", "110111", "b6e0e55780b699e37411f0eb176f7496", 359368),
	ENTRY0("adverbum", "060905", "e5c04bcadb953ad0a280b541f7510338", 138240),
	ENTRY0("advhoudini", "120511", "51982015d7f804d5bf5a25e2906b281a", 579228),
	ENTRY0("affront", "040226", "1b10a2dcedebdea772cea1c7b0407331", 50176),
	ENTRY0("agreydayforalbert", "120625", "03bad50bec2b361d2ab0fb16718e9626", 158720),
	ENTRY0("aisle", "990528", "a7af83193b4139f65c020ac49ff30447", 122368),
	ENTRY0("alice", "030501", "ce25288e736c1c420a990b609943a990", 86528),
	ENTRY0("alienextraction", "060921", "73b2a47c648afccbc784f412c3cedae6", 121344),
	ENTRY0("alongtheriver", "100630", "1b670d231c4b0cc10b51ba60196c6a68", 198144),
	ENTRY0("alpha", "981017", "f8fbbc64455efc259f9b41517d82b4ab", 51200),
	ENTRY0("amazingmaze", "200415", "0f881bf71c7c401367c0adb52d496fff", 169984),
	ENTRY0("amazingmaze", "201015", "bc1657b2778d09829102af96105eaa61", 169984),
	ENTRY0("ambassadorsdaught", "150525", "d415de88a7a756be04f5203f8b4668ee", 255488),
	ENTRY0("amish", "020603", "d4194b4de41c9663f464bb33f1917a7d", 50688),
	ENTRY0("amiss", "020409", "fa22d754f2b9daedc232bfbc6b28b6d1", 143872),
	ENTRY0("anchor", "990206", "2bc49d98d980bfefa064ae76f4a217e5", 520192),
	ENTRY0("andrewplotkin", "080423", "9679960dde743ff1f506c12db29c5214", 366740),
	ENTRY0("andromedagenesis", "130701", "3dc7a53837dfcfed4895aad148b78f27", 245248),
	ENTRY0("animals", "071201", "98e57ff531a97d26d16418b343d9682d", 32768),
	ENTRY0("animals", "150314", "d5e0c7c6641709eeb5c4c0797519962c", 32768),
	ENTRY0("annoy", "990127", "0383c40a28c606ddf8bfba3a66469055", 71168),
	ENTRY0("anotherterminal", "110724", "4ecb1f83a4ba8b797488bc32025a5bde", 324588),
	ENTRY0("aotyrz", "060825", "d9264cedc9cb438eedf67f611dbdbb3e", 159232),
	ENTRY0("aphasiaquest", "150426", "ee480b0a0983e636649777a745a8f6f0", 233896),
	ENTRY0("apollo11b", "080712", "00fa17987dec69c1b1a3927b23aeffd9", 150400),
	ENTRY0("arc3h", "201129", "b01104b86019675398ec40d44b24a3bb", 52224),
	ENTRY0("argument", "170327", "7acb2206d9ca8afcc734e2c6dc7ef782", 330198),
	ENTRY0("aridandpale", "090501", "d18d5d2b8ceef04234f64373958097e5", 199400),
	ENTRY0("asylum_cg", "090721", "1fcf07b963cd664cf11660882de99e47", 135680),
	ENTRY0("atrocitron", "161215", "6798fc939e1c6a92eb8e356e359a0ee4", 91648),
	ENTRY0("ats", "000001", "accd04838197c1e4937bac253349b623", 387072),
	ENTRY0("atwork", "000705", "44e22cd9dd5124dd149b761dfd6e93e8", 78336),
	ENTRY0("aug4", "130702", "0b9a2b7c37f3d56fcd8d0b531582f8df", 364544),
	ENTRY0("aug4", "200401", "d537c469fe6cc008286bf071045c66a7", 364544),
	ENTRY0("avon", "111115", "82a3d640af8cb7707a1b4301192679b1", 104960),
	ENTRY0("awakening", "980726", "66181a19b3316f6644c20a9ee18be40f", 99328),
	ENTRY0("awakeningpg", "091102", "a5f6a578fb1fc57423936707c07b2029", 268288),
	ENTRY0("awitl", "980215", "8cf72be55d75137cc435ab25f479c123", 56320),
	ENTRY0("ayac", "010107", "b503a9f2817ce97480160e454e470c58", 68608),
	ENTRY0("azteca", "070117", "e30320818aa935f1bff66fcaa862fd9f", 123904),
	ENTRY0("b7snare", "150118", "5e1057596f6fe7d0efb100f80ae65f27", 284672),
	ENTRY0("babysitter", "240319/z3", "db25e74cd9f8e0dde95e0e6be7facc49", 45056),
	ENTRY0("babysitter", "240319/z5", "af6d71122f7ebed7d44131d5e6665170", 46592),
	ENTRY0("babytree", "120512", "79e230657b07b2e8fa45ffbe4635d03f", 294352),
	ENTRY0("backtowakeup", "080313", "73ada6a69651bea315206d7994164b95", 640246),
	ENTRY0("backup", "091204", "52d1d40ff557c21b7d4464e12b6d646c", 429188),
	ENTRY0("badguys", "971113", "41fe87f9853672ab269ce0ab91de9046", 55800),
	ENTRY0("ballerina102", "991128", "9f92c9d90a536f62d7a535e7026c28bc", 508928),
	ENTRY0("bakingwithlizzie", "170813", "946db43fb6f8da8b6eefd61dc0c6a774", 451602),
	ENTRY0("bakingwithlizzie", "170924", "d289b6eb29982127b83e2f4254636e27", 380928),
	ENTRY0("balances_zcode", "961216", "2b740e4bf08e64580085640b40a0ead8", 75264),
	ENTRY0("baldersdeath", "111107", "cec997b2ba4917a2dab2640f276f1c7c", 254404),
	ENTRY0("balt24", "970706", "7c45c1ea2780a5eed643f35fb44dccda", 58368),
	ENTRY0("barber", "070830", "e19a510ad01afaf98ef72c9c59602d21", 180224),
	ENTRY0("bathtub", "060409", "580a828689b64b150d29c4247c873acc", 85504),
	ENTRY0("bazic", "010710", "a6c277d223557127c7d0678aa128d501", 55296),
	ENTRY0("bcd9", "201111", "9f96e8ae7aae26354a13521d60029733", 50176),
	ENTRY0("beanstalker", "180105", "6490a77e5f0d5e01b37c0ed29e017a56", 33812),
	ENTRY0("bear", "990224", "8e96c4de71e6fd4bdafb10bdea76893b", 109568),
	ENTRY0("bedlam_mz", "970711", "6c816efb1a7fc99345fdb85bb9e01ac6", 59392),
	ENTRY0("bedtime", "120324", "2a39f06bdad42384b026059d690cd779", 260120),
	ENTRY0("beginning", "200714", "557d3dccf7eb0bc223935f45a6d2f77a", 87552),
	ENTRY0("beingsteve", "060519", "a446f37fcd5945428797573295acbe03", 124810),
	ENTRY0("beneathtransform", "080107", "8e53f4d65b0b68ef92fb2fbd3592dcc5", 176128),
	ENTRY0("beyond_zcode", "100115", "61bd0b7eae9e81c7d01496d623a497e1", 336982),
	ENTRY0("beyond_zcode", "210810", "099c2b33efce521b811f3dc5287dc5e8", 336752),
	ENTRY0("bicon", "101006", "2f2d993faf161318c4b4c5ee0957e288", 366836),
	ENTRY0("bignose", "220405", "83c13edb932c08753e0ffc98c1ed47d0", 316392),
	ENTRY0("biscuit", "010310", "e4d8262dbeeab8e340e522e5cef740a8", 137728),
	ENTRY0("bj", "981103", "2787b18265a5bceccd0a4d311f94d97a", 158720),
	ENTRY0("blair", "971119", "39a811c3280594ef64942d73ab11f7d4", 83456),
	ENTRY0("bomber", "971123", "1c1740d9770d53d7a18cf56006e8d88b", 3584),
	ENTRY0("bonesaw", "180405", "cb4195df6cf1b44786d5edf81287a26a", 342016),
	ENTRY0("bookvol", "051225", "c949b9db879b10faea5cfca45ee6a657", 225792),
	ENTRY0("booth", "960409", "953578eeec2b85c92fd54d87f3e9c7fb", 44544),
	ENTRY0("boothdye", "345678", "772134a9ceb8c7dc5f2b10d6d139aa0d", 51712),
	ENTRY0("boxes", "140329", "f6726bb84741c1ef7400ccfe465faedf", 262594),
	ENTRY0("brandx", "111115", "2937e62eefe4f059ea9d8d85f4fd55a0", 112128),
	ENTRY0("breakin", "000926", "0cc04d377f04ba75f647c9a21688b9b6", 208896),
	ENTRY0("briantimmons", "130114", "94545a75f7c9f0bc33a171a925f49af6", 544936),
	ENTRY0("bronze", "101025", "ae7671168ced4412cc6307d106e1b23a", 538040),
	ENTRY0("bryantcollection", "090401", "2c331a7c183ef29ea13b68075a58c73b", 528620),
	ENTRY0("building", "030706", "cc5ab40754fde9739113ef2524ef66b5", 253440),
	ENTRY0("bureaucrocy_zcode", "141113", "7dcbbc724b7416ad0b80b3159c7e3525", 387606),
	ENTRY0("burglar", "010925", "ca9ae17d983afb2d79aad60486a5a859", 71680),
	ENTRY0("burnkoran", "101128", "216f61842ba2024b291b4e0ae7769500", 222746),
	ENTRY0("burnsnightsupper", "120124", "a1153aab71b7a3bc4bd1aa056f3145e8", 251342),
	ENTRY0("busted", "941223", "e164cfeed308fd25bc102a18b3c8f15c", 82944),
	ENTRY0("byod", "200928", "90b19dc00264bc1ae97c1338d5ea0bca", 549618),
	ENTRY0("byod", "201106", "fdcb10f14d32060c016b509ee40dae13", 550642),
	ENTRY0("byod", "230325", "16d57e585acf6de2b1aa6cd20a7959fa", 551154),
	ENTRY0("cabal", "041127", "b8f7f30bef898f4ed1814b0094c40b1e", 178176),
	ENTRY0("cacharensdilemma", "090526", "dd1531291506c98fa2e3f6ab25314a67", 346112),
	ENTRY0("cacophony", "091209", "a612e786ee256ecf14580bd7e922b27a", 364032),
	ENTRY0("calendar", "070805", "8f8f6f45daa40e640805b83279196cfe", 137728),
	ENTRY0("calmmutemoving", "110712", "c4f0e45605825341e566bbd4a1d52522", 173056),
	ENTRY0("calypso", "140421", "5fadae3572711526d7b61c90bab58087", 97792),
	ENTRY0("candy", "970621", "322249ca0d96df20c010d8c2226cf0f2", 59392),
	ENTRY0("capturesanta", "071220", "bb02e0523374a703007fecf42c8e05c3", 172432),
	ENTRY0("carpathianvampire", "220501/z3", "814489495640eceb65dddcb169c13114", 73216),
	ENTRY0("carpathianvampire", "220501/z5", "364754e505409cfb703209ec26ec10ca", 75776),
	ENTRY0("cars", "980923", "c857ca87a713ac8d3ae13a49ea05f994", 57344),
	ENTRY0("cars", "981010", "b6d057301718325b393e7e8e99bf09e6", 57344),
	ENTRY0("casting", "050707", "11ec0ca3c23c327e77a83985305c17f4", 247808),
	ENTRY0("castleredprince", "130227", "1dc1bc1337931b25d57594e566152772", 362880),
	ENTRY0("catcherintherye", "060611", "0bb71366efbf85f9a4700c6f121e7972", 198550),
	ENTRY0("catseye", "041018", "7ff8e2d98b56e7f79508f6e22d6383ae", 10239),
	ENTRY0("causality", "240304", "4ee72c8d430dc4d0e28fadf99b1dcd73", 117760),
	ENTRY0("caveadventure", "070718", "6d2fc2d639efba54382481ac319500b9", 550986),
	ENTRY0("cavernofdoom", "030312", "87769a24b64ec0dc2e261c7111d7662d", 133120),
	ENTRY0("cavernsofchaos", "990813", "05104b58bb5d6e1765b8a3be541381d3", 24576),
	ENTRY0("cavetrip", "041225", "5f0668876d3f8b03e62bd53d3ab276f3", 117760),
	ENTRY0("ccake", "000311", "2f0745fb253ef799472afb4e7e7f13a9", 80384),
	ENTRY0("cco", "220331", "214207a5cbdf28bc45cb2de95c059d29", 51200),
	ENTRY0("chaos", "090801", "ede5478e9a87906205d784181b33e8aa", 96768),
	ENTRY0("charactercreation", "141030", "a78d006b26941821fb9b2f8f32ebdf7c", 227840),
	ENTRY0("chaosgame", "090526", "0b902ce73efd76e5c57230a25e27f364", 176004),
	ENTRY0("cheater", "960920", "69753e7cb886a003615b8aa415702135", 48640),
	ENTRY0("checkerhaunt", "211209", "24920f69ad800a977555b5404c86d515", 261210),
	ENTRY0("cheesedoff_zcode", "160724", "f22a5e611479057236e0a8af31b62e70", 448080),
	ENTRY0("cheeseshop", "021230/v1", "88329068474b92abf4b4363c177f6971", 85504),
	ENTRY0("cheeseshop", "021230/v2", "cbf746a948b66ae8d2a65e52134654b4", 85504),
	ENTRY0("chengara", "090621", "1ae7fc0b20586b7ff76b4d4340dd3abc", 222720),
	ENTRY0("cheshirecat_zcode", "140803", "69dced03ab92e1efeae0ea4a496743d5", 78848),
	ENTRY0("cheshirecat_zcode", "141012", "0a8c570fb7499bcc35109fdf7320132b", 79872),
	ENTRY0("cheshirecat_zcode", "150918/z5", "97f1096761d27951a1715c627390b6ba", 76800),
	ENTRY0("chico", "000413", "b46bc30a1495c8cc9b1f9399f0ac5ed5", 162304),
	ENTRY0("childsplay", "080129", "d21a117c5a40afcbdb04a08aa109da57", 535396),
	ENTRY0("christminster", "961117", "86e6349dfa3ef6153cd1c0ecb9b4fc6a", 228352),
	ENTRY0("cia", "961218", "2cdacedf416e7a575de47412e1d164f8", 54784),
	ENTRY0("classchallenge", "190724", "d6caad7ab6a6964f0526f5b4c7a41316", 163840),
	ENTRY0("classchallenge", "201112", "cf1cc6017a784a8f52daaaa7c617772a", 163840),
	ENTRY0("classroom", "210320", "ee18db643cc029257dfe838d56aa5f86", 321536),
	ENTRY0("cleanair", "190608", "3e5a972edf102956f6973c9fc7a8d388", 410302),
	ENTRY0("cleanair", "220825", "a988a1fd11f394316632c1759b2eb4a5", 414916),
	ENTRY0("cliffedge", "111030", "2bcba042a3b56fb010c9ab0e64cecad5", 324608),
	ENTRY0("cockandbull", "170401", "6036a188e9def979678697ce7be14171", 895024),
	ENTRY0("clockwork", "200725", "e7a388799d227c9b8b114a4b398b500a", 40344),
	ENTRY0("codenamesilver", "170619", "8250de9dd95418d6cfa1b5f323254ac4", 357888),
	ENTRY0("codenamesilver", "220622", "dc5dc0208b955cf35f5c5c352745f737", 357888),
	ENTRY0("cogs", "201116", "70a02efa75d49fface1f3577e90278db", 176128),
	ENTRY0("cogs", "201113", "b4da5e421dca6932f2a39311fab8809b", 176128),
	ENTRY0("cointoss", "150217", "e19c6d27ccdae1fee28136f8efb829ec", 314426),
	ENTRY0("coke", "990331", "8ee8282eceb70c6bb6b9c427d1e01aff", 107008),
	ENTRY0("colonists", "130911", "2f5bfc75a420df67718f542d67e9c118", 534020),
	ENTRY0("colmc_zcode", "120110", "95a3d47250d6cdacfb2c9aa334ae71db", 194560),
	ENTRY0("coloursexploration", "980116", "8f8b4508b4f2e6633ceeac22942fdd19", 55296),
	ENTRY0("conankill", "050428", "eff6760b564715f763d6fc25fce8a7eb", 91136),
	ENTRY0("conceptisproven", "120620", "e5f443e775546084096fe11afb6a313f", 291292),
	ENTRY0("cottage", "090715/z5", "994f78a6f2e9f062b0a34cfdf5e44c34", 134656),
	ENTRY0("cottage", "090715/zblorb", "b2c6f964c0bd4d8a0c12009697ff7307", 616308),
	ENTRY0("countdown1", "061219", "a4a901a3cb0dcc2c7440e344a8e12822", 128512),
	ENTRY0("countdown2", "070110", "45adfc3347ba01e5bad33a848fd9bba4", 133120),
	ENTRY0("countdown3", "070110", "08be83ed93d1504f3dab14b8ec80beb2", 144384),
	ENTRY0("crabhat", "130216", "c7ccd1b68fd860cbaec5ee721e0a06e4", 154112),
	ENTRY0("cragnemanor_zcode", "180804", "f3553639e91a43a12f3dc083ebee67a9", 381952),
	ENTRY0("cragnemanor_zcode", "180812", "67d46450a8833c34d3231673a1f3dca9", 378880),
	ENTRY0("craverlyheights", "140414", "91e0e3f0e2a6393c7674f5cd3ddf3c52", 264192),
	ENTRY0("crazydiaper", "120430", "16f632238939474b22c96b3ec19b5f84", 220748),
	ENTRY0("crimescene", "130123", "508196f1cd974f789fdc2af69767a1d1", 320916),
	ENTRY0("critbreach", "130817", "348fc7ef47cab92d0b7f286da3c3ab90", 268800),
	ENTRY0("criticalhit", "091002", "87f825b236785d94302e1293791b9daa", 311296),
	ENTRY0("crobe", "111115", "d8b138a87f6226b8283c33c50c6f254c", 92160),
	ENTRY0("cryptographer", "071216", "5c306d5f1cee2d4eabf199012a354ff8", 40448),
	ENTRY0("csbb", "090625", "30bd815bc7d5943a0c8725e3b31ec373", 489984),
	ENTRY0("csbb", "130209", "87689146f6e8240d3ff054c688c1fc46", 500736),
	ENTRY0("ctdoom", "000920", "24e754f22952fa0dfd88fd1e50e4e4d0", 132608),
	ENTRY0("cubicle", "160411", "6a83df1b1ce0d799847c80741d2cc7df", 347684),
	ENTRY0("curses", "930428", "2d7bd98476f98df370d5b0ba9630b3cc", 117976),
	ENTRY0("curses", "930603", "df66b5c8ec66c86828f300db684d6e25", 126276),
	ENTRY0("curses", "931111", "91badf5f65f84490b68b71fdf3c2416c", 131072),
	ENTRY0("curses", "940120", "cb9e27ed9131b99d1bd6fffc3b319a39", 131072),
	ENTRY0("curses", "940604", "a414bc655f975cb3803d6a525fc14ded", 228864),
	ENTRY0("curses", "950522", "ff96fe6908dbb52d09ede64260885aca", 254464),
	ENTRY0("curses", "951024", "636ca27c82d3af77142ea92e6522b9ba", 259072),
	ENTRY0("curves", "010613", "c80a64ffb0a19c5cb89108fb36485d04", 524288),
	ENTRY0("cyclops", "020505", "f4ab4dc7f8f8e0fc1f716b3a43273b58", 78336),
	ENTRY0("damnmemoriae_zcode", "060503", "451e8c94438a3add4b16a24ed7e1127e", 388188),
	ENTRY0("damnmemoriae_zcode", "061008", "c69e98feed6b49dcecc040a6babab455", 166474),
	ENTRY0("damnmemoriae_zcode", "101025", "a2b94a1f955a125d19a230b247b99ee3", 427100),
	ENTRY0("damnmemoriae_zcode", "201217", "bcb03ab5af1dce02d5c9a9764a761d26", 634578),
	ENTRY0("dancebeat", "100923", "22361c0cb0047fad69d285ca9b51da4e", 240564),
	ENTRY0("darkiss1_zcode", "160130/z5/Fixed", "8c8f759915eedbf014b1c8d8ecc282db", 134144),
	ENTRY0("darkiss1_zcode", "160130/z5", "a05a25ebb25ade67ec9c7dcc09c10ab1", 134144),
	ENTRY0("darkiss2_zcode", "161016/z5", "bc35bf57cc74c18113386fc5bb0f2e82", 155136),
	ENTRY0("dashslapney", "120510", "d934c25530f689c8c25ce71bf184b82a", 338426),
	ENTRY0("daycreepylife", "100317/Demo", "e211ed2eef47d78f72330d98e0f9503f", 150016),
	ENTRY0("dayinlife", "080406", "ad0b46d14e3a8d577a58f0d93d700f4f", 173558),
	ENTRY0("dd4", "030207", "c5921ad782bc25cbd7e3f8c8b1412a4a", 163328),
	ENTRY0("dday", "110720", "897fbdf9cb2468b09c30961524d36ae5", 266364),
	ENTRY0("deadmen", "200825", "d6d9516a7f2ca84d3397df54504ceb68", 230340),
	ENTRY0("deadmeat", "170527", "c0f127f032ade1f46267028371e68e5b", 252374),
	ENTRY0("deadmeat", "170617", "beed7ee4185f9ddf31f3b94db8ff4ec0", 252374),
	ENTRY0("deadreckon_zcode", "030730", "1232dc599a00548bcc2d6453a01c5e50", 87040),
	ENTRY0("death", "030309", "4e013a77c165338cbf2662ba04465a9b", 120320),
	ENTRY0("debate", "070412", "bc1c6db5b5fe43d6d11982907debeed4", 162816),
	ENTRY0("deephome", "991210", "d30357d2b08ab21e8f2da0e74a3c87d7", 133120),
	ENTRY0("deformuseum", "051024", "cc7d7754d1f3e326e0ffbb1c08eaef12", 139264),
	ENTRY0("degeneracy", "010331", "d42d960bcfc2a8e1fadc809b31c96f02", 167424),
	ENTRY0("dejavugn", "930921", "ece489ece5cffa0b8575f44e094a115e", 22500),
	ENTRY0("deliciousbreakfast", "111015", "393e71995dbb13c23a1b5dd5403341d7", 152852),
	ENTRY0("desert_zcode", "070909", "3e23d39ba4cec9f14fb8b5c63396259b", 23040),
	ENTRY0("detective", "000715", "c09fde6c6777c2c422de18668cf986a2", 108032),
	ENTRY0("detention", "130827", "c2361faf8bbc8996fc0db8e71ef001da", 244684),
	ENTRY0("dewdrops", "050115", "32e07656bbfbc7b4d0b5fe8fd4d1ca7d", 109056),
	ENTRY0("dogshow", "101130", "6476e788310c44c52d5249a66d07d037", 712070),
	ENTRY0("dogslife", "981015", "e6f0adca898d757c49c9d81a67d3b6cc", 67584),
	ENTRY0("dontgo_zcode", "120119", "5b153263f946a6013d80d07fa1573dbf", 195584),
	ENTRY0("dontpeeyourself", "131107", "1eb2f9ebe332d9bdb3b6468b1fdab833", 298222),
	ENTRY0("dontpeeyourself", "150807", "ffbdd3b0860e5a5b5ffbbdbb6ac4fc46", 211968),
	ENTRY0("dontpushthemailbox", "191228", "d8fcacf89d5a5f1ec1b877775d137598", 332288),
	ENTRY0("dorm", "230319", "2c93e859a58d2e41e89a660d6baff06b", 129536),
	ENTRY0("dotd", "050619", "48b47df16c2d32612fe2f09f234d99dd", 245248),
	ENTRY0("downthematrix", "120521", "291d6e004fe0ace1dbaf79434fbd5403", 319976),
	ENTRY0("downtowntokyo", "000615", "8d9ef45f2bc4fdb8eccf7d9991a2c76b", 107520),
	ENTRY0("dpod", "051012", "b573b31e2df3f90b396a6ddaf27e120e", 303104),
	ENTRY0("dracula1_zcode", "070424", "42b5559ca5f01f7ef63ef4573d71326b", 87552),
	ENTRY0("dracula2_zcode", "080819", "446564750ebe364539c27f89a520bb08", 108544),
	ENTRY0("dragon_zcode", "040211", "744c35647fddfb830594596b4b350b71", 145408),
	ENTRY0("dragontroll", "070120", "44e48cb9689e156d322a1de0a14d420e", 3584),
	ENTRY0("dramaclass", "120521", "3b7d6bcecc2adf6921562d56217f07ac", 250324),
	ENTRY0("dreamhold", "041231", "78eb852b88f9424647a4e18caa4c6707", 386560),
	ENTRY0("dreamtrap", "130521", "d3d036c2895fd097b4c11efc6ad7e44d", 314368),
	ENTRY0("dreamtooreal", "061128", "ac2bc1ebcfd2c8b5eed405c4f7fcb10c", 138124),
	ENTRY0("dumont", "990223", "c5b6b0c7dcf3e771207bc44a7a3eaa0b", 224768),
	ENTRY0("egcpc", "100408", "ab75fb61ae23aff72c1dcd104e651766", 353280),
	ENTRY0("eggstraordadv", "160813", "80ddef3cc800e2ad11d00cb1172f8341", 293528),
	ENTRY0("eldariasurvival", "130213", "ffdd78885e5883dc86291294c8cd45b4", 156672),
	ENTRY0("electroroom", "221101", "f58523f22e3a91e62e3b0ee310362406", 157184),
	ENTRY0("electricpuppet", "080425", "9a98f65333d8efe621cb5e3688e60bf9", 188416),
	ENTRY0("elephants1", "120407", "c2594ae4986d460f5f47ec38d3759a24", 151040),
	ENTRY0("elephants2", "120407", "70263b9eee2e0c03111c3ebc529de760", 156672),
	ENTRY0("elephants3", "120409", "6e9cbdf3128079d42abef931964ed835", 236032),
	ENTRY0("elephantsdecision", "120409", "7bd2a483fa9a0338260273b813266f68", 157696),
	ENTRY0("elephantsde", "120501", "3c939fe13cc217530fd1db46841d41c7", 152576),
	ENTRY0("elfenmaiden", "110923", "039b1f8a3f6a210a165913cad05fe881", 375782),
	ENTRY0("eliza", "990207", "293e7e1e2790af326547cd7be6c44d92", 7680),
	ENTRY0("emptyroom", "100813", "9b3b2c5ae701ef457baaf3db6e2aa06e", 320960),
	ENTRY0("endofearth", "101201", "859cd2e3f72011202b1bf999c9438712", 154112),
	ENTRY0("enemies", "990115", "ff9fc17bf66ffbfba125d9a7c20c2c60", 289280),
	ENTRY0("enigma_sd", "160202", "743b7440094810ebe9b9d86d994ef07b", 418468),
	ENTRY0("enterprise", "020428", "791a68da0a0da75307ce60bcf6f07f55", 156672),
	ENTRY0("entropy", "090516", "2acf5da303368989679bbb8ee71da5ea", 125440),
	ENTRY0("epyk", "091111", "2d14b9b9499bc87ecad01215009fa2c7", 236988),
	ENTRY0("eric", "000001", "6ed92a1deef039921701441bc371883a", 116224),
	ENTRY0("escapade", "100111", "64f5e34efe18300476ea6e2dba0e4f81", 678318),
	ENTRY0("escape", "130210", "988b6899f8d3c9bc4bbd3153ce1ab31f", 275358),
	ENTRY0("escaperemember", "060705", "57b062c6f3e42cac8cb0775265c33c42", 284810),
	ENTRY0("escapetrollcave", "240319/z3", "fd79d6ef9f61061b3d4d9fba44da4d07", 36864),
	ENTRY0("escapetrollcave", "240319/z5", "80b6a16191e6a6bf912223a4b13d8905", 38400),
	ENTRY0("eto", "040123", "3ac8b0a5ee2f69c86a34008f92da2c13", 74752),
	ENTRY0("everydaysamedream", "100619", "0c4486b71b7a19e2220c85e055caf43f", 163840),
	ENTRY0("f209", "970806", "98d739b8f6f5df78cbdc338dd50f89c5", 56832),
	ENTRY0("fable", "000715", "603d4573ad89adabb7bf8d0b031705c0", 95744),
	ENTRY0("faculty", "080116", "8998ea6435611921f97bb67fff0dd130", 218612),
	ENTRY0("fade", "040228", "5ac4f6787fe3d57e9c0e2292f248bf4f", 42938),
	ENTRY0("failsafe", "001218", "05293830576ae81e9a61f38cab731d38", 78336),
	ENTRY0("fairyland", "190815", "b2dbb1186a00f46f7029df2278c7591a", 163328),
	ENTRY0("fajfeta", "020101", "284de92bd57df8398562dd2c659fef35", 51712),
	ENTRY0("fallingpieces", "180713", "6493ae5c940165d185ac17e2efce4eaa", 45056),
	ENTRY0("farm", "020101", "9838d582cdd87814598eff791775a53d", 135168),
	ENTRY0("fifthcontinent", "190816", "69d659a6910d04332a1f517d5630b01a", 172544),
	ENTRY0("fifthcontinent", "201104", "b8749102dd7be7e2ef31dcd8cbacd69a", 171520),
	ENTRY0("figaro", "070430", "1c70432445689cd4cd2a196285cb1aea", 136192),
	ENTRY0("figaro", "070910/z5", "4e661efb3a2aa9d1913b662b5c29a756", 151552),
	ENTRY0("figaro", "070910/zblorb", "fdab59d4bb51e64c1376b9bab4de94b0", 152932),
	ENTRY0("figaro", "100911", "b696b4578c9d4044edb137e3a87b0f50", 406596),
	ENTRY0("figueres", "1", "c0d9ff83a9ff5eba8938934a339aee6d", 277992),
	ENTRY0("findesick", "180802", "ccb98f2fa05f747e00c883c47e8ba31f", 331776),
	ENTRY0("findthebed", "100708", "02d3f5a2fba50f42c750a902f04f54c8", 289224),
	ENTRY0("findthemouse", "090810", "6703a5f5ffee588f2dc0ce216a545e83", 147456),
	ENTRY0("fingertipsfriend", "120514", "e8c7de7a01feaa665d97db2501160eb0", 386560),
	ENTRY0("fingertipsfriend", "140106", "44cad06c5c79a7f99e53974a7d08c273", 423480),
	ENTRY0("fingertipsmilk", "120515", "a908551dd98ef7df4436f674116a48d4", 370688),
	ENTRY0("fingertipsmilk", "140312/z8", "cbba526456d9305f3691408dd64f4340", 190464),
	ENTRY0("fingertipsmilk", "140312/zblorb", "01d3504181777b30f96de20dfedda359", 278664),
	ENTRY0("firstdaymnl", "110119", "58c46cbeeaf96c3302515cb3e9aedcec", 454606),
	ENTRY0("flat", "020922", "00e8daaacb7651adb9db0dd91e02bfd5", 78336),
	ENTRY0("flowers", "980301", "c948c03dc3b25f784f822cb623d5bffb", 47616),
	ENTRY0("fmvpoker", "000913", "3f4ee585aa91660d5483ef2f132fb1d3", 19456),
	ENTRY0("fmvpoker", "001227", "0d7bec942838ba2d42af2f836e2a8fc5", 24064),
	ENTRY0("forest", "121216/Demo", "f629bfa6755cd36f45dbc3fe12b97936", 243130),
	ENTRY0("forgottengirls", "120817", "421e83ead89a7a0f2fd05791f9b87db7", 410034),
	ENTRY0("forgottenpyramid", "230914/z3", "c9bf8ffc3e103d18e620d034eed9c089", 70144),
	ENTRY0("forgottenpyramid", "230914/z5", "bdf948698b4e4f638f45c8f212210b71", 73216),
	ENTRY0("fork", "071205", "cc07f97369a7f20da06eabf4fa8f2e4a", 201728),
	ENTRY0("fractalz", "000714", "847523b3d366b2621d593ee260f4d9a6", 6144),
	ENTRY0("fracturemeta", "010306/Demo", "d684caf97f01d447b37376ef7c708a85", 111104),
	ENTRY0("fragileshells", "100204", "1eb855145d3016bcf2f7a415fd9a9a01", 546586),
	ENTRY0("fragileshells", "111109", "6232ef15b7508aab6072a85afb86908c", 547610),
	ENTRY0("frankie", "040304", "1ce64a1c5a2d928cfb48b4d760246972", 91648),
	ENTRY0("freefall", "951111", "3e7898a1e767a2de61336ff2ff4bb1f4", 3584),
	ENTRY0("fridgetopia", "160415", "41f751cff5967147eb79fa41cf42dbcd", 179712),
	ENTRY0("frobozzi", "990129", "3ca33fdafd15fed40ce32de557a84195", 184320),
	ENTRY0("frozen", "960620", "e6185f87a3bfb42baf2f1de528a479a4", 63488),
	ENTRY0("fugue", "080220", "039a087fed837f81514f9f1b5d282af1", 217088),
	ENTRY0("fyleet", "111115", "df1bc4b00ad8bc376a29c74ad7c23a22", 141824),
	ENTRY0("ga", "120510", "ec52eee6bc32f783edc18088b6abf943", 177664),
	ENTRY0("galatea", "040208", "7d2f6474d49625ff3537f594f17d2469", 441992),
	ENTRY0("gamer", "120415", "f5cdd32b3c0c1f09fe7c659103317365", 293356),
	ENTRY0("gardening", "080412", "2d9ff32f10bc569977984c9042afee4f", 394156),
	ENTRY0("garliccage", "120209", "04a598fea9264a290e2c34f0b63f8042", 176640),
	ENTRY0("gaucho", "120805", "3ec2942af85d52ecd22c6ee3decf9b8b", 176640),
	ENTRY0("gbvb", "191106", "57f38d735b654bc24d20b9aeb7005442", 470478),
	ENTRY0("gd", "010806", "5be045c6983f16f2d44e660f9f0192a2", 50176),
	ENTRY0("geist", "140115/v1", "859ad972bac98e666c813f2390357e2a", 243712),
	ENTRY0("geist", "140115/v2", "96f54aaa29ddd0bbfa5db914150633a5", 244736),
	ENTRY0("gerbilriot", "000513", "69d3926fdd11464a3a79bc10c03cd324", 79360),
	ENTRY0("ghandi", "110603", "1f276ccb870b63c4c287d8dadd8b8aac", 177664),
	ENTRY0("ghostsblackwood", "240303", "847f77943687e6fa837d289ebf52e181", 111616),
	ENTRY0("ghosttrain", "290912", "3e737617d1eb7f551d858e1b9ad5dd3f", 210944),
	ENTRY0("ghostmountain", "210417", "9528c0c0996c41dd210bd023a6e1e28d", 43008),
	ENTRY0("ghostmountain", "210509", "3c09bc59d5d38fd11f3523cf330045a0", 43520),
	ENTRY0("glass_zcode", "060503", "94f15a91234bbc9f48f50e478363442c", 378420),
	ENTRY0("glass_zcode", "061008", "f687986dab7bd4e2f61d3f282c1ded4d", 373812),
	ENTRY0("glass_zcode", "101025", "059ca93ef8e309470b7e12325a43a52b", 412212),
	ENTRY0("glik1", "111211", "1cfc3dd21b01fa968d36b6d58fe80862", 353620),
	ENTRY0("gnuzoo", "160331", "1f9274506529a4c934f09cfe06cda804", 478060),
	ENTRY0("godot", "960599", "953905ec43f6e595d1634adb37ed879e", 49152),
	ENTRY0("goldilocks", "090410", "1e2b32a419bfc9703dda529f40c01782", 217600),
	ENTRY0("gourmetgaffe", "120624", "0c59f0c03fd36303ec02d370cee9b650", 257156),
	ENTRY0("gourmetgaffe", "120625", "e8b68d14298ab4e921985667ae74417d", 257156),
	ENTRY0("gowest", "110823", "26da8324bd67fa93bd25c4234f51073f", 251476),
	ENTRY0("gowest", "120425", "2c1e329d9cde395785f8323a740a361e", 251988),
	ENTRY0("greatpancake", "180117", "9f4e359b40bc53631937c2c44ee5082c", 264268),
	ENTRY0("greaterthan", "100415", "f7e6e82cda24f2e6f8f06f74323aa19f", 232844),
	ENTRY0("greenblood", "200825", "3a0b9d06df3ccbca6e2bdfcefa8a8c35", 184936),
	ENTRY0("greenrain", "100611", "f28e8e500d4fbc94d59f5bfdb9cf7f4d", 275516),
	ENTRY0("greensboro", "080712", "dd7bd20aa69092b009f9f5503c01f124", 141714),
	ENTRY0("growingup", "110825", "9bada495ce70bfcae2566f01b844d20a", 267720),
	ENTRY0("grumpytck_zcode", "231225/z8", "5798bf89bbce4493d325e12dc1f569a2", 341504),
	ENTRY0("guestreet", "100031", "2637075afb748532ebf851310158017e", 12800),
	ENTRY0("gumshoe", "960331", "935eabe90b3904a0cde9811a463fcc39", 143360),
	ENTRY0("gussdeath", "040131", "1c3d12957d1ecf3ce28b7fe36cb34111", 41472),
	ENTRY0("hadeanlands_zcode", "101116/Demo", "45ea9ca4ec326344f73e4dc2be26d3b8", 651964),
	ENTRY0("halloweve", "111028", "0a209a7dc47242f906d8b782e4efb887", 324608),
	ENTRY0("halloweve", "110403", "925323f2e14cd53574f60c9f8bde87a4", 379392),
	ENTRY0("hamhouse", "081124", "068b91ec4a227760e08f1ba48acaeb7a", 238080),
	ENTRY0("hamil", "111115", "3c6f7131752edfd7b78252112b3a6e66", 81920),
	ENTRY0("hangar22", "110617", "a96bff6dae1e76771688f73e6570550e", 152064),
	ENTRY0("hangmansgulch", "240319/z3", "9804519607ebca0a985f8f27c293677c", 58880),
	ENTRY0("hangmansgulch", "240319/z5", "a6dacaccc69cc0b605e9f17ff0f8007e", 61440),
	ENTRY0("happyeverafter", "130114", "921e9eec1c4049b630b0a00379bd7d5b", 121856),
	ENTRY0("hauntedhouse", "170103", "6d1a6caced27d901c09896ac74de6022", 400078),
	ENTRY0("hauntings", "111109", "931b910ab47044e8792c4d5adc5163e3", 225792),
	ENTRY0("headcase", "230310", "d05d6cef1edfa94fe52b76f25477b7f7", 613492),
	ENTRY0("heartice", "200825", "68b954b47816ee47297cf1a51f816f51", 232408),
	ENTRY0("heidi", "051112", "0b13ec8dec91b35662a62c7a60b54ddb", 83456),
	ENTRY0("heist", "990217", "d37eab3288f34d5673f72d8697835e8e", 358400),
	ENTRY0("heliopause", "100703", "423b5192b31eb0705a928b3900b6b208", 389388),
	ENTRY0("hellsbasement", "140826", "749bad5b6169b1cee259b9f21cfee75b", 275890),
	ENTRY0("henribeauchamp", "080621", "d9088b3260fdaf30fa81cde478472eca", 179712),
	ENTRY0("heroinesmantle", "001211", "4beb00dfd7abed53bb529946662fc252", 523776),
	ENTRY0("hibernated1", "220625", "6b64251594ac07e042bde09689a388c1", 94208),
	ENTRY0("hibernated1", "220814/z3", "a97e01c8d4545660ff4522bf2640f3af", 91136),
	ENTRY0("hibernated1", "220814/z5", "df80b9e7979960cab74ac95fe290eb3f", 94208),
	ENTRY0("hideseek", "210409", "a3a8cedd516bd3bf37b1cb3922709a07", 321536),
	ENTRY0("hiddennazi", "100911", "93521e3903e1fa63cfb7ababebd4c3e1", 367570),
	ENTRY0("hiddenverbiage", "181127", "736af995f8cb87601fe170695efa7180", 477130),
	ENTRY0("hidepachyderm", "140705", "5e46b32c2897c46097b379f2eef7f5fa", 215040),
	ENTRY0("hidepachyderm", "151011", "304cbcbdbd9338fc60872270905ed6f0", 215040),
	ENTRY0("hippoelmstr_zcode", "151031", "89805c07c2dfbb27acd9995aaf380eaf", 346112),
	ENTRY0("hipponewyear", "130108", "e71635f90b99d1d3362ff0ff9f91e43c", 376832),
	ENTRY0("hipponewyear", "130210", "b226fcbf871613684d8d2c7941e02e82", 317440),
	ENTRY0("hipponewyear", "140109/z8", "1c802115295be8d62863ec4c199a3fc3", 317440),
	ENTRY0("hipponewyear", "140109/zblorb", "e2b6f957b2f24874b7550181f655cad0", 493172),
	ENTRY0("hlainform", "050715", "e919e8ca4e2358b010d3a758a98609c8", 168960),
	ENTRY0("hobbitredux_zcode", "170420", "758273f92c520f56c727d9aa390d007e", 451072),
	ENTRY0("hobbitredux_zcode", "170421", "6ba4bb0546c1e7fb3024da8041f7852d", 413780),
	ENTRY0("hobbitredux_zcode", "170425", "5964f6951be8da16fb350b9d0982c89e", 374272),
	ENTRY0("hobbitredux_zcode", "170601", "037bdb4c4518a9335f86b71a11a673e9", 414292),
	ENTRY0("home", "110204", "454bbecf0847b69f17748a04e225fca6", 156672),
	ENTRY0("homecoming", "140903", "b3987678e4d9fcc95fa9718c3f9ce7a8", 1089176),
	ENTRY0("hoosegow", "130320", "5d8d2858bb803b227ac923a654dd367c", 2024678),
	ENTRY0("horatio", "120430", "374c64e310683d236e050d9700a1cd16", 249942),
	ENTRY0("hors", "071125", "9f08fc42729ab3b74f82c0536605d5e6", 95232),
	ENTRY0("house", "070714", "4b146b3d5b81a3b3359ed3f3059910ab", 468830),
	ENTRY0("housedream", "071204", "64709099677209a81c9edd196ef3e266", 312320),
	ENTRY0("housekey1", "080112", "ddc18dd2a69887dec6c1d4d994d35a8e", 134144),
	ENTRY0("houseoffear", "121023", "134026b9f6aa92ef0bec67307d8ce726", 544646),
	ENTRY0("houseoffear", "141121", "86812b0b8866038dcce67ba265e5bd5e", 572530),
	ENTRY0("humongouscave", "000001", "4d0a5a75192c8343315855f881874515", 332800),
	ENTRY0("humongouscavehints", "000001", "d6581a6fcf31851267e1acc00c466e4b", 124928),
	ENTRY0("hyperrpg", "090630", "46d18c446800ea7cdaa7f1d370ab7c05", 198628),
	ENTRY0("i0", "140603", "878cd0b3cb05340b49cf18999ba59fab", 219136),
	ENTRY0("ibo", "120109", "b592aa3222e86ee1baaa45107cca6f83", 196608),
	ENTRY0("icebreaker", "950912", "089919770f1b094dfdb111ca64ee7489", 49152),
	ENTRY0("iceprincess", "960901", "f00f7e3f96670613ee73ec51ef2b2957", 84480),
	ENTRY0("iceweb", "160229", "b4f294ede4bcf5134a49d3a8d47cd2a0", 506932),
	ENTRY0("ifaquarium", "100515", "5f99b211f5bef1c1d1e89762dd451cac", 235088),
	ENTRY0("ifplayer", "130325", "d471621d98969c2c3050586af788a88f", 251826),
	ENTRY0("ifquake", "040812", "4c75b746abbae0e869ede0b06501f823", 133120),
	ENTRY0("ifwhispers5", "120312", "ebdd62f2484dc87955055cb1093d9359", 326144),
	ENTRY0("ill", "000218", "34f194a208f828c4c77d3954082d75a7", 73216),
	ENTRY0("imiagination", "110608", "df766ac578a86f0b9d7231bddac5b09b", 252886),
	ENTRY0("inamanor", "180117", "65b795d35855bb12e8914eeedd522c2c", 506996),
	ENTRY0("informschool", "991217", "388c8e73dd3611e67ed335b6234f4e2e", 241664),
	ENTRY0("inhumane", "950817", "e07f6bde81912a655468010fa13ed9f9", 68096),
	ENTRY0("insidewoman", "090628", "c9505d9dc5badfd95a6be7b4b4edc2a8", 478720),
	ENTRY0("insight", "030209", "77cd4bd598e9f175faf8f84d6751c140", 151040),
	ENTRY0("intangible", "131229", "b1941de7a9ce4efc9c21e7ba11072954", 304060),
	ENTRY0("interviewrockstar", "120223", "559b8521d944c959739095fc3f0e06b7", 243226),
	ENTRY0("inthenavy", "040907", "065f26efdd214bad13d229db02441969", 103936),
	ENTRY0("intruder", "990210", "c42924c9879af8cb682d490aaf9841b0", 175104),
	ENTRY0("iraqiinvasion", "080131/z8", "c3ba7d2b115625e962e0614a5c381f4d", 259072),
	ENTRY0("iraqiinvasion", "080131/zblorb", "427ff378923f675915b95993b5858704", 260780),
	ENTRY0("island", "130226", "6ee80981b5921ad6f159ed740b54dd1a", 354882),
	ENTRY0("islandsfaraway", "100426", "67a890cf7b6bc8c2b40ff101a3a69784", 262616),
	ENTRY0("jayisponies", "120519", "bc418f85264a240f4aed02f7dadaf762", 239104),
	ENTRY0("jetblue", "051012", "e1b0f957e0a4967a0f5d66f9e7313944", 209920),
	ENTRY0("jewelalithia", "160621", "954b12ddba17e471ab708720d0f53ad4", 2378714),
	ENTRY0("jewelofknowledge", "990710", "38c384ce3d1c31791fe98859558dcb68", 225792),
	ENTRY0("jigsaw", "951129/z5", "faefa6d593cebdd177167c24f7cfd373", 262144),
	ENTRY0("jigsaw", "951129/z8", "44b8fdbadfc399f48a3367fade486e07", 304640),
	ENTRY0("jigsawrules", "951128", "a1f0f980850e6b7e4ef79f93d549a227", 64000),
	ENTRY0("juicehead", "140609", "8e0d1a5c30fe508cca4768d2772ae1e7", 272146),
	ENTRY0("juicehead", "150112", "54288e72711653d544da189152eaea0b", 272146),
	ENTRY0("justanotherday_tl", "170107", "e31df5e10118e3efa2714bbf39c032e2", 392704),
	ENTRY0("justanotherday_tl", "170129", "33a1880b1813fbc0d7ceac6ca75dfe59", 616844),
	ENTRY0("karn", "961207", "daa05c92a3a1b678c29fe8bbf64a09be", 165888),
	ENTRY0("kazooist", "130105", "9ae3841d39c8b6f8dffa9c84408d9744", 204308),
	ENTRY0("kennykoala", "220501/z3", "064f6ee27d2716e5056d27b77eb232a4", 74752),
	ENTRY0("kennykoala", "220501/z5", "467939dbb4493db7c91cc98b4c25d3f7", 77312 ),
	ENTRY0("kentishplover", "200118", "d06991d44cdedc0ce4cbf17b3aa79f76", 162816),
	ENTRY0("kentishplover", "200119", "07d9e4de03a8b80200d0e9124e162f57", 162304),
	ENTRY0("kentishplover", "200213", "2c93242398a841ad069e81fa3b2c80f1", 163328),
	ENTRY0("kidnapsea", "110608", "ddcf23cb530746544e0f416337dcf6bc", 191488),
	ENTRY0("kierkegaardsspider", "120413", "19f457515033fd938856e6507b8a6bc1", 254414),
	ENTRY0("kiiwii", "141113/z5", "091ab4e450db053a419fb4a8ffe87a22", 255488),
	ENTRY0("kiiwii", "141113/z8", "cd3a8e87c86044c8b75368aa8b1ef61f", 214016),
	ENTRY0("killingthedoctor", "010221", "6370cf6b47fee82e75f1edb6e4b7f7b0", 57344),
	ENTRY0("kirby", "062776", "7750af7ab62cb01dd04e5b665d299d1e", 143360),
	ENTRY0("kitten", "130320", "a55520ba5084b68bfb3d1ba9346dd2d6", 45056),
	ENTRY0("kitten", "220120", "716e5185056b4371ee5f3c6ebb4cb3a9", 46592),
	ENTRY0("kooku", "971119", "68dc431ab53ff87af1a3bc037122667e", 94208),
	ENTRY0("lackofvision", "980412", "25a2830653906a2225742505e1fd7cac", 47616),
	ENTRY0("lambs", "971010", "39b487be161e229f56badbec69466a5c", 47104),
	ENTRY0("lambs2", "021230", "2f6596397a42fdd83407cb2520cffaea", 59904),
	ENTRY0("largemachine", "010905", "abd265d7c2622ff6d9ca9ff5266cb802", 160768),
	ENTRY0("lash", "000806", "1ff1bffe7386b44fc4d34a34fc59ed3e", 370176),
	ENTRY0("ldodoom", "000801", "b4a663fc9adf6b344b7b64eb58ceff1f", 200192),
	ENTRY0("lecture", "140813", "b8f9441556f6e785fade1e41d75458d3", 493520),
	ENTRY0("lecture", "200414", "3fac793cf1a93f0f061dc6ca508315f5", 493520),
	ENTRY0("legacy", "971011", "1ff3c36992bb7d1961305942e7c13625", 228864),
	ENTRY0("legendmisshat", "140109", "8b83ea3984edc9d979be31e8c9562a9f", 383088),
	ENTRY0("lex", "120830", "29de1a490f09a9c756b28c00bcc95230", 368060),
	ENTRY0("librarian", "130211", "ff40eb307c9ec5b015531201cb0bcf00", 247754),
	ENTRY0("libraryhorror", "231010", "7178965ebaa5795e378c1b9c466e61cc", 35328),
	ENTRY0("lifeordeath", "980702", "aa820dffedafe40d7d3f895ec8f30840", 51200),
	ENTRY0("lifewithocd", "160814", "63997ca4dd1fcb724d9027f9467847d3", 292562),
	ENTRY0("limp", "980517", "9e36e35b17cbdebcd9d348de156b51df", 47104),
	ENTRY0("lionskin", "121106", "dfdc63b480603baf62a2ccb34533bf0a", 325570),
	ENTRY0("livejoseph", "111227", "08e7c0024e7735cc4ef7e8cf0e04c112", 264782),
	ENTRY0("llr3", "070314", "4c2e6391bf678f0563fee5492c7e73c7", 164864),
	ENTRY0("lmstvg_zcode", "081227", "971bdd7f99e59ea1afe0629c2adaa9f4", 189508),
	ENTRY0("lobsteronaplane", "100403", "e1cc80a0d0358a13dd390b26c15508e9", 264704),
	ENTRY0("lockeddoor1", "220116", "a3a7abc23d5688baed0e0ffbf4ee5d15", 597074),
	ENTRY0("lockeddoor2", "220118", "09d6443dad4bb757a9adc0d183e940c6", 624882),
	ENTRY0("lockeddoor3", "220119", "b9640c151b410ea5aad3ef5d7f3186a7", 638248),
	ENTRY0("lockeddoor4", "220119", "7a9e607e9abc5593e2512148187b1b5e", 646566),
	ENTRY0("lockeddoor5", "220121", "52325dc020187231f44e297fa1f8f05b", 641260),
	ENTRY0("lockeddoor6", "220123", "4efcf79e00edbcffa7a4d717b77b0602", 664596),
	ENTRY0("lockeddoor7", "220125", "81b489d13287d8d164acc3206b60a2ad", 705484),
	ENTRY0("lockeddoor8", "220126", "4fd859c247b554185eb7cb9a5c6bd7e0", 739832),
	ENTRY0("lockeddoor9", "220129", "912afeac21d21582db34073450ff8643", 711602),
	ENTRY0("lockedroom", "230523", "14178f6fe9cca08ef5c99511afd30c68", 29184),
	ENTRY0("lookingtothesky", "110313", "c83056d3ad3775af870fab9ef800a00e", 379738),
	ENTRY0("lostpigandplace", "080406/z8", "400ab3efad70e5b454452e89bf0625e5", 285184),
	ENTRY0("lostpigandplace", "080406/zblorb", "f7ede4fcda5d7ee1fd8831e7f8fd2256", 390256),
	ENTRY0("lostspellmaker", "980116", "f569560e28e57b0cc15abe69292ddb63", 128000),
	ENTRY0("ludicorp", "031010", "65250e5c5cfcb403f9fb870ac7e30041", 101888),
	ENTRY0("magicmirror", "190817", "7b0e556a7a85d6da09d10bb1b1771e9d", 16384),
	ENTRY0("magicmuffin", "120505", "b68fdf687db87217caef5e6c9de65f4a", 208660),
	ENTRY0("makeitgood", "090921", "1f4feb2361e05361ee27bf82f8a3c559", 512606),
	ENTRY0("makeitgood", "091227", "061af5747715511a5ac5a3476278a914", 495616),
	ENTRY0("mansion", "010505", "cb310588d3ebdfe419b16ed29f5bc8df", 107008),
	ENTRY0("mansion", "220406", "0bef5d345f25200423aa4828ab6e15c6", 108544),
	ENTRY0("mapa_zcode", "010711", "57e67b50fe180f9073da28548bd73a5f", 70144),
	ENTRY0("marconi_zcode", "150606", "5bd573961828acb20766ed203d373f60", 137728),
	ENTRY0("martyquest", "120430", "80b274b7feb7c19ee0aeba85dac0d688", 268272),
	ENTRY0("medusa", "030314", "f9e93b13a6f940ef0d25cfef559e36c8", 109056),
	ENTRY0("meetingrobb", "190928", "b3fb96f61f8c5a01f637cdbfe582612e", 464290),
	ENTRY0("meetingrobb", "191002", "9eb2560176aaaaa2f7fd820944891ec3", 464290),
	ENTRY0("mehplace_zcode", "?????\?/zblorb/v1", "407a01a7709a13dad6582c212ced97f4", 456130),
	ENTRY0("mehplace_zcode", "?????\?/zblorb/v2", "32aef84c45a635f3014e67a34ffcf217", 459714),
	ENTRY0("memorylane", "230311/z3", "34a1ce5f414e01c3e0a97c723cbca4d0", 104960),
	ENTRY0("memorylane", "230311/z5", "33ca0166735982befe7ca3569df99389", 108544),
	ENTRY0("mercy", "980217", "865d53d9764636ddf1bcaa2b703673a6", 97792),
	ENTRY0("mercurytrucking", "990424", "d4e2a55d878dec5878e4cf9d43361d9b", 150016),
	ENTRY0("metamorphoses", "020222", "cb23d62a4dce52afd6104683aface9f6", 177664),
	ENTRY0("mhpquest_zcode", "960613", "58b0832c491d961426b73a6b989e905a", 52736),
	ENTRY0("micropuzzle", "200511", "b10ce858dcaf942a8300a89205055a43", 784384),
	ENTRY0("midpoints", "010330", "1004502d7dea284be6c6298316c5f9c8", 94720),
	ENTRY0("mines", "101029", "fb2d0e729e8b50d7d10f71b06317b3b3", 65536),
	ENTRY0("minimalistgame", "101102", "4ea052eed3e86283912bff5e817151fb", 151040),
	ENTRY0("minimalistgame2", "101123", "ab0c93fce76a6009cd4060dd912ef3b1", 160768),
	ENTRY0("misdirection", "060304", "39a12ab5997240cf406f3c67001685fe", 221184),
	ENTRY0("missinggrandpa", "120319", "3e315935045c1cdd15bb37a8e8700cc1", 590198),
	ENTRY0("monkeybear", "070417", "e4222987e97eee25e85fb8b1d55cd3a7", 656838),
	ENTRY0("monstermaker", "171027", "6c2dbae638ed97060c789718906ce506", 352768),
	ENTRY0("mornelune", "100525", "280074bfad86ecaa56cf6813e3a031f4", 182784),
	ENTRY0("monzasphantom", "070219", "b5f91ecd114f3257ad269f1afb64b64e", 116092),
	ENTRY0("moonglow", "041011", "3ee29824d9875a4ae21e82b88813990e", 10239),
	ENTRY0("moonshaped", "080710", "1cfa040aace3a4cb249818684fb43b9f", 447840),
	ENTRY0("moonwrecked", "110309", "1cc71e3087cb4f92fc244a5c72b8a078", 264668),
	ENTRY0("mortalkombat", "210130", "f5da05b7cdca8a72866ecf97cb14f8a9", 160768),
	ENTRY0("mortlakemanor", "120304", "80d317fd404451e436e9c36de5445de8", 261632),
	ENTRY0("motelcalifornia", "110329", "67a1855ce59354e8eeff4f2a684811bc", 331712),
	ENTRY0("mountain", "030317", "b84bb15597f7aa53ae407e2d996c0c6c", 103424),
	ENTRY0("mousequest", "??????", "2993b235743e6a6a4d69063e80c187d9", 325062),
	ENTRY0("mousequest2", "??????", "32531caa725d44ddbbe720b05a364a11", 323014),
	ENTRY0("mousequest3", "??????", "4820cc5cf2aa9dbdc1c660146cdcf79e", 352198),
	ENTRY0("mousequest4", "??????", "8480dd58a0c6b3e0dae916f2aa8b19a2", 439238),
	ENTRY0("mrscrabtree", "130225", "3ce7aebc7cdad846e1bdc8f46823f4b3", 235520),
	ENTRY0("mst3k1", "000715", "0a0748937d23bf380cd139de874df32d", 141312),
	ENTRY0("mst3k2", "981104", "55dc31376ee9e99700e4ad144c5670af", 83456),
	ENTRY0("mst3k2", "000715", "cf16a0b763735b06cebef3cd5fdf2c78", 121344),
	ENTRY0("muffinquest", "120326", "58eedaa233021cbd8f90ea503b0c3fc1", 438082),
	ENTRY0("muffinquest2", "120428", "7125b6d59ce97674dd075510de6168b6", 442200),
	ENTRY0("muffinquest3", "120505", "41462801e25ca195c377ad34ee080c9c", 439162),
	ENTRY0("mulldoon", "000724", "84ef2f5f00c06d6cad6b0817844fade5", 468992),
	ENTRY0("mulldoonmurders", "020214", "1dfc1e3605f6eebfca49e8fca17e957b", 231424),
	ENTRY0("murdac", "111115", "790b5e8cf9677615d17ee75e89a99d4a", 78336),
	ENTRY0("murdererleft", "120117", "cb99b94b046bd7b7db8912617f3da9c5", 196608),
	ENTRY0("musician", "070820", "0b5aabdb180fdf0199f7779baab3e354", 142848),
	ENTRY0("mylastduchess", "140510", "82cd30149bdfec0ae3a149b1f612dfa9", 258998),
	ENTRY0("mymagictirehoax", "071224", "32940eaab8162a8a878bcff5dbe5c441", 227722),
	ENTRY0("mysterywinch", "230531/z3", "c159a01c46e81ee8a6faf540d68f8c97", 81408),
	ENTRY0("mysterywinch", "230531/z5", "2102dd60c7c7f94dc0284f81b21ccb7f", 84480),
	ENTRY0("myunclegeorge", "090714", "dbdf7f0ea10c10ae2654d58e6ab9d1a4", 179090),
	ENTRY0("nameless", "131206", "197a58d317be0e6060c490bd40baf8a1", 472064),
	ENTRY0("nascarexperience", "130827", "d45dc3ab1f9898cf85a09854db0a6b17", 152576),
	ENTRY0("necklace", "200825", "61cd8722a6030361c09e317189e27cd7", 217972),
	ENTRY0("nemeanlion", "081113", "c298fdabefb085ca297f0f0917e2fd4e", 77824),
	ENTRY0("nemesismacana", "120503/z8", "8f5ab052cc4f4f808a40add1be324c3b", 356352),
	ENTRY0("nemesismacana", "120503/zblorb", "d24a6aaa2d9430b668fb7bda7caf9b42", 397226),
	ENTRY0("nemesismacana", "120507", "5b60522a5845aa318f15387ae628f882", 397738),
	ENTRY0("neverplayed_zcode", "140327/zblorb", "69d214a7a657a8ea7f2cb951b8bc3edd", 980260),
	ENTRY0("nidus", "140416", "8eefda240f46c6b5516a3a1fdfa53497", 117248),
	ENTRY0("nightbunnies", "971205", "6a2cc6996865a4d1520ae467a5a802fd", 78848),
	ENTRY0("nightchristmas", "061224", "e4eea0aa89a8cb9a614c1e8122498425", 124894),
	ENTRY0("nightcomputer", "961118", "c2ecd9885f53897d253ff11cb80f62ad", 74752),
	ENTRY0("nihilism", "151001", "9157c2d79e12924a91ed2fd4b3bb490a", 317952),
	ENTRY0("ninepoints", "961127", "278e64924fed8e10b89819c0433daeb2", 74752),
	ENTRY0("niney", "170329", "28fef576ae455e19c73666a40745a3ae", 624574),
	ENTRY0("njag", "210603", "313bcf0f118737b34ab2507928e1f47c", 174080),
	ENTRY0("njag2", "000319", "b566c2127fdd479ae4afdb5f2d019403", 147456),
	ENTRY0("noroom", "112358", "5a99c89364f0d2bb624461a44ef0b071", 60416),
	ENTRY0("northnorth", "141114", "447ba8bf4e026bd1c6995a3d9306d207", 347136),
	ENTRY0("noseguard", "991212", "fdb20e1cc77aedb8350a54b68ca0ea92", 54784),
	ENTRY0("nostrils", "090402", "8155559fca293c99957ba4e18a651640", 454284),
	ENTRY0("notinvenice", "120117", "191d800215e21a764394ec46efe9a728", 194560),
	ENTRY0("nudistsgonewild", "120425", "5f98bc9a14ff9a8c2cabd78238fafcb3", 180224),
	ENTRY0("odieus", "941021/Beta", "b75120eda34c37c87201405426329892", 50176),
	ENTRY0("ogisoas", "101102", "673a5bb3476d15e3d54ddba3176d9568", 155648),
	ENTRY0("omniquest", "040127", "6d246b048e071dca1c2f47e64e76f10d", 95744),
	ENTRY0("once", "980324", "4ef97819cbc6adab0487ca1256ee6ddf", 55296),
	ENTRY0("onegirl", "020113", "7ff5749ec2881b1343c06fc5950f928e", 173568),
	ENTRY0("onlywar", "210331", "58dd2a0d423e51ed2b923bc153195dfe", 410568),
	ENTRY0("onyourback", "130506", "c942e2c31131f799481e61ebd878b4f2", 173056),
	ENTRY0("openthatvein", "080126", "8f83ab59d09f5f490e3fc1ec5f04e525", 449548),
	ENTRY0("ottumwa_zcode", "021409", "7bf33289ff4c6fbdfd467b387534b309", 123904),
	ENTRY0("outofthepit", "110515", "dc78b694b2ca682084ac7efd769acd9d", 276932),
	ENTRY0("paddlingmania", "314159", "8e995f6fdfcf7e96ee9cf6a142bc5053", 54784),
	ENTRY0("paintandcorners", "980316", "2028550b59c1b2f0892e90545ba15adb", 9216),
	ENTRY0("palimpsest", "200909", "0326e9b599e96f7d8ddcc36febce0620", 163328),
	ENTRY0("paperbagprinc", "150818", "fdf4a244b41e4a314cfa189ba85453cb", 398848),
	ENTRY0("paperchase", "950503", "dd421533f35d62d0da704958e5b33d51", 57344),
	ENTRY0("parallel", "080219", "b50ce87302a473152ae9d556dc5f9419", 158720),
	ENTRY0("parallel", "140414", "2e50a0ce61383d3c28c23f2b142a0f03", 210900),
	ENTRY0("paranoia", "981208", "1309a1b60d62af820d17ee812b0f8a61", 80384),
	ENTRY0("paranoia", "090803", "887280c21df983c85eef98080abd182e", 137090),
	ENTRY0("parc", "111115", "bd8698030864dacbf494226fc6b9dd0c", 98816),
	ENTRY0("pasdedeux", "190923", "b7210c938e19393802dd4628153c494f", 601196),
	ENTRY0("pathway_zcode", "080511", "8eb9bc735afa30d84fd4f913fc29bbc3", 264192),
	ENTRY0("peacock", "000208", "5a4cdec49d14f905013ad9ae35ba17f8", 78336),
	ENTRY0("pentari", "030206", "3879d37f7417f6ce2a0acb96faf00c86", 91136),
	ENTRY0("penury", "120528/z5", "58939b4506dc1e27b36cb7e75a1e2479", 231424),
	ENTRY0("penury", "120528/zblorb", "5d0cb16db2d9e72ce9f27d3764a375a9", 250436),
	ENTRY0("perilousmagic", "990821", "56901ae08078fc15fb74a50e48101bbf", 66560),
	ENTRY0("perrysworld", "070501", "6466fe74c657f960521237d5a3274b05", 124300),
	ENTRY0("philosophersstone", "427301", "3684b64c0b5972908af83893ab662bf1", 93696),
	ENTRY0("phoenix", "080430", "d0c50e5cff1098009fbb9f893e7dc75e", 890884),
	ENTRY0("phoneboothp2", "961017/Demo", "28d166262aa22fb55fdb9685a5a124ef", 65024),
	ENTRY0("photograph", "040827", "0949e8e4d3e1e035913fc75112f40459", 264012),
	ENTRY0("photopia_zcode", "120416", "3e9ea30956eb3e9494e6e5d9881a1307", 239616),
	ENTRY0("pigpancake", "110410", "7e6e0b1c90a9a01b4fbaea21ad729d55", 236446),
	ENTRY0("piracy2", "100408", "86e2ed40f55413f29fa97bd6e4c31260", 249856),
	ENTRY0("piraterailroad", "110920", "20d019cd1654f5d9d3c1bcffdbded2cc", 333500),
	ENTRY0("ponderances", "160220", "58c4f08222dfc27cab23036e9e37a6ea", 371214),
	ENTRY0("praser5", "050509", "d7f86984c2a35d0304a48989f56cb0aa", 35328),
	ENTRY0("pressedon", "130323", "26ece4d715035234a5d6d8275bd4de85", 246968),
	ENTRY0("priceoffreedom", "120919", "589c7d77bae5aaa502807021e506fb6b", 178072),
	ENTRY0("primrose", "091222", "47b4978d4709f5783a674de899e2d73e", 620384),
	ENTRY0("prize", "120827", "9dc341cb340b6950602c52bd8ef76305", 408786),
	ENTRY0("probing", "990919", "f7cbdd49ae15d380f66ab247c118f17d", 73216),
	ENTRY0("progressive1", "090623", "b734369549d094a71cb4fd0b3ffe8bb0", 278488),
	ENTRY0("promoted", "1.1", "b5ef006c83dcf2e2f175aebb64c7870a", 124928),
	ENTRY0("punkpoints", "010108", "ffb32fb83c4480a3109755572819623a", 136704),
	ENTRY0("puppetman", "040803", "ad3143a695c3406e7ede1f39dfb6ba63", 110080),
	ENTRY0("putpbaa", "010618", "458a35f2318afa617fe4679f8c0ec9a5", 132096),
	ENTRY0("puzzle", "200714", "d83d896abaa1b8f8883fb61b203fcd9e", 89600),
	ENTRY0("pytho_zcode", "020223", "a5e3d0ebd1f81ca341cf93a721f6ed3c", 293376),
	ENTRY0("quidditch1954", "121022", "ae9626ce114047e0d1247d226cd9cc4a", 330710),
	ENTRY0("quitequeer", "221122", "ad2599a939b3e8dbca5d6236e3a10a81", 374742),
	ENTRY0("rachaelbadday", "070328", "3147589b43da574303bffb26c5f317ba", 269312),
	ENTRY0("ralph", "040309", "ff6d29e293d82c49b4a5f4b21ce96655", 78848),
	ENTRY0("rameses", "061023", "6ea654d98a64cb50eff35d7a613cdfb1", 167166),
	ENTRY0("rans", "000918", "1577894d9022d221f9db27f7a96d4d4f", 180224),
	ENTRY0("ranshints", "000917", "b509a63314d10abf17272244db3f3bb0", 64000),
	ENTRY0("readmay_zcode", "120109", "9ae7c7c4f4162262ea3fdfe49c30c0e7", 196096),
	ENTRY0("reddex", "200124", "c393172b2250524493ff34ac767baac6", 379392),
	ENTRY0("relief", "000820", "91046311f44a670f51738fcf5dc5a6cd", 94720),
	ENTRY0("reorbushcave_zcode", "170406", "f84a6d0aaebc242b5b12fdf3f9315a33", 1058880),
	ENTRY0("reorbushcave_zcode", "170416", "1d010db82d499b06e27c28375f867fcb", 329216),
	ENTRY0("reorbushcave_zcode", "170610/v1", "3ba501733bc953188c91e58f98f821db", 947240),
	ENTRY0("reorbushcave_zcode", "170610/v2", "a06d903aed0ff68b7e03498bc44889dd", 1047792),
	ENTRY0("robopuppyredux", "071115", "f91a4a15c2ffe365e2e49b4869deade0", 181606),
	ENTRY0("reser", "050209", "ba7e7d973ce19279d7eacf9c5ef80136", 90112),
	ENTRY0("resident", "970908", "ae5612cc75ac734de51d864c9e8d9976", 136704),
	ENTRY0("returncastle", "201201", "ce8e19d4c40d620ed94dd32aff6e45aa", 51712),
	ENTRY0("revengebabes", "980318", "90207f5d68da1c2ef6cbc9bb8f2653aa", 52736),
	ENTRY0("reverb", "990110", "fb3d80d0f92857286a4fd7e6c2892293", 106496),
	ENTRY0("reverzi", "991218", "31252071da0fb3dde08ae45f74a768f9", 9728),
	ENTRY0("risenecropolis", "171208", "c01ccee1f849be82cccc85a1ab62d52b", 473566),
	ENTRY0("risorg_zcode", "060308", "f0110f1b749d2d82ca415585eaaeefb4", 428544),
	ENTRY0("risorg_zcode", "171024", "2fc1be68cc542a2ad0dd073626d88674", 442880),
	ENTRY0("risorg_zcode", "171114", "e6e6c0e75711a28aac39afe185bea1f6", 475616),
	ENTRY0("robodud", "031129", "0fdecf906b44be43436c548b22c4e22b", 84480),
	ENTRY0("robotgardening", "060604", "418ec7e179334b9bad657a0873bf226e", 126464),
	ENTRY0("robots", "980115", "0f9ad287635965ae521c62d29329eb2d", 6144),
	ENTRY0("rockinghorse", "120703", "a593bd476ac50764125377f29c89adda", 254886),
	ENTRY0("rogue", "980706", "c4e104c4dec6381ee1a8943aa4e008e5", 116736),
	ENTRY0("roomserial", "120627", "9470b6e7a2cdcb43422e75deba20b864", 302080),
	ENTRY0("roomserial", "120628", "1429eade57830627054e3fad32f6f3f8", 252416),
	ENTRY0("rota", "060430", "ca0f6e049bf7b17407e28a3f88e19416", 671830),
	ENTRY0("rpn", "090531", "d74a5da655a81e03447b4a241c1b5b21", 142720),
	ENTRY0("rtdoom", "000831", "5a8494839033e8c4dd036b6875e1641e", 179200),
	ENTRY0("ruinsremixed", "191201", "0da3f262dc221b4207bc697b55ade56a", 156672),
	ENTRY0("ruinsremixed", "159609", "25922952a8f45381870345cc2967c1f4", 158208),
	ENTRY0("safe_zcode", "120520", "a1da1ff07495f0af679652f4841a89a4", 195044),
	ENTRY0("samegame", "980731", "ffe1011cf0a3cae333825fcda58153da", 7168),
	ENTRY0("samhain", "001021", "9cb11fc72283939161b456c1592b00d0", 69120),
	ENTRY0("samurai", "050712", "177a4b89c1b08bd1edb3f07743342e75", 61440),
	ENTRY0("sanddancer_zcode", "200913", "bc633ea28dcfabd62385e3ab4151adfd", 215232),
	ENTRY0("sanddancer_zcode", "201004", "51f0263ed2ddc4916bedf7df03a316b3", 2157434),
	ENTRY0("sangraal", "111115", "b663b3dd73ca57b4ddd6fafe7ec4ac0d", 150016),
	ENTRY0("santassleighride", "081227", "a4b5276e5885ed3e93f1ae0319488404", 441730),
	ENTRY0("sauguslibrary", "020905", "04054a1d02d24a1374662b9fbb676c46", 71680),
	ENTRY0("saugusfirstparish", "031113", "4006f8f809c69a21f730dc2203b50795", 103936),
	ENTRY0("saveprinceton", "041125", "13f9e45a4cbf39d541dea08b774377d1", 288256),
	ENTRY0("savoirfaire", "040205", "eb122ee416ee8fa1e6f909b6de6ad9c0", 442212),
	ENTRY0("scald", "180206", "07fb2e2748c3835bf4d1aaba70a708c7", 194048),
	ENTRY0("schooldays", "092800", "2c3334c637e37b1b80ea089d6911477a", 192000),
	ENTRY0("scopa", "110321", "621b223a5f02c7e49d18ae0d6b588d19", 101376),
	ENTRY0("semid", "091103", "992482407f66e006ad5c23eaee627a66", 237056),
	ENTRY0("sueursfroides", "090104", "14609bd3ad9bd0b7b88c438ee57932b3", 173056),
	ENTRY0("sgtguffysday", "200613", "f4843d92f702814b2a043d857b3cb8a3", 127488),
	ENTRY0("shadowgate_dg", "040616", "9015104db32c046798870273f0754d3c", 141824),
	ENTRY0("shadowofmemories", "061124", "07b4c14e309c00f8adec901afbc0e58b", 179094),
	ENTRY0("shadowsoldiers", "070625", "985406d500afb5e29a3b174b28b1f21e", 150418),
	ENTRY0("shallow", "121109", "cc2d82b5453ec921e69a80c04e1f1ad7", 245696),
	ENTRY0("shattmem", "010928", "613eea3491158279446fe932eb9ef46d", 85504),
	ENTRY0("shattmem", "011007", "370921da26efd668774fa51f7ac5990b", 10752),
	ENTRY0("sherlock1", "021024", "97577dc5e6be837277acd5c134620d92", 230912),
	ENTRY0("sherlock2", "021101", "b4e67e63abe681449d5cc727b161e4ea", 285794),
	ENTRY0("sherlock3", "021114", "605303b5dfddc04e590e6f060369463f", 220672),
	ENTRY0("sherlock4", "021231", "7b231c7acafc9a4959c859b68578d528", 217600),
	ENTRY0("dayishothitler", "081206", "2bb145727b016ab76c35acfae729293e", 175616),
	ENTRY0("shrapnel", "000212", "cce4edfddaa7ce948cacdb0fde52fb2d", 98304),
	ENTRY0("simpletheft2", "110601", "21c13d6583c2ce9714e9bdc31adcab82", 357818),
	ENTRY0("slackerx", "971009", "c7014f074407ddebc685dcd7bce5bd37", 55296),
	ENTRY0("sleepcycle", "101217", "5c9c1b6c46c057099cdc6ad04f30407d", 253892),
	ENTRY0("smallroom", "100805", "5102ac48e17b26b69ab8737ceb00b3d5", 154112),
	ENTRY0("smallsimple", "190617", "38bd8e69d0fa19a2c3d7ce2b54afa955", 161792),
	ENTRY0("snafufun", "990925", "2b04ac88022c9df74b4ae59cb3f75f27", 13312),
	ENTRY0("snowedin", "220205", "7076c6c1c59a5d7f8abe227639bf30fc", 103936),
	ENTRY0("snowedin", "220601", "5e4956166d28daa7cd438101c7f9f261", 105472),
	ENTRY0("snowedin", "220610", "3538f77fdd09a1850752e90385e01ca4", 105472),
	ENTRY0("snowedin", "220809", "7de852ea444ca4bd21ab07ea41ccfd42", 105984),
	ENTRY0("sofar", "961218", "026e1c0b7ef555011df2a9f72a2db574", 300032),
	ENTRY0("softporn", "971018", "6624ea7bffdef1e14e28f57a90b621b0", 105984),
	ENTRY0("solitary", "040607", "18011f0b8e3cf6aa6403313bdec2df0e", 97280),
	ENTRY0("somewhere", "080129", "abbd658087b2c7d0a52a5a787c75c0a3", 189952),
	ENTRY0("soreality", "100127", "1a43e51209ae7a2fa51081aff804b3c6", 146944),
	ENTRY0("spaceinvaderz", "980710", "51ebab3a743c783ed284a582346b90ba", 22528),
	ENTRY0("spacestation", "040130", "1bcf00508a8054c366d07dd507f81b22", 137216),
	ENTRY0("spadventure", "971030", "93e10542f7a30aaf4380de9209572aed", 154112),
	ENTRY0("spiderandweb", "980226", "2bac499c020fcdd75b9c4b65e4c1d85d", 221184),
	ENTRY0("spiritwrak", "960606", "bfa367d09a262d4efc83cb1ba1a99efc", 260096),
	ENTRY0("sporkery1", "080111", "eebd7008071529686b367b6641b9c52b", 147968),
	ENTRY0("spot", "021025", "0f1feec607bd423d9f1a95748dbd1245", 55296),
	ENTRY0("spotlight", "140531", "ac25066421da9530f5f19c7245d0f6d9", 336384),
	ENTRY0("spring", "080207", "e74ccd9b57ee13df8ad2d20c14cb76b4", 330240),
	ENTRY0("spring2020", "200414", "a06ded7137168b94844ab002ee7aa7e8", 320000),
	ENTRY0("spycatcher", "111115", "628c860890e6393f22d81af5c96e9bcb", 126464),
	ENTRY0("starborn", "110116", "0f22fb91edc9e64ad292fd6a7ca39f2e", 418624),
	ENTRY0("starborn", "110129/z8", "2d73288c422d3b676aff42a0990c9732", 242176),
	ENTRY0("starborn", "110129/zblorb", "0b0bc6b79220ced866c672aa4b166ba0", 421696),
	ENTRY0("stargods", "111216", "92a0b939e5d12178f74b3e23ece89ca1", 268288),
	ENTRY0("starlight_zcode", "130802/z8", "86c1069f38ca76dad28c10d05dbdf59b", 359936),
	ENTRY0("starlight_zcode", "130802/zblorb", "a8e0e1a9fb1cecddde690342e01b6fba", 452000),
	ENTRY0("starrydepths_zcode", "202005", "db40ba5514b49fbb835621c8577da56b", 92672),
	ENTRY0("starshit", "120614", "c2d72febb61a9365d6c6b35f597bcf7e", 394688),
	ENTRY0("steine", "030831", "224de592a0fa832195e78b74f17635a0", 72192),
	ENTRY0("stewgoing", "120218", "0192783d5ba404a4876570dd19dc3c49", 324608),
	ENTRY0("stewgoing", "130422", "f7af48ec219fa3703d194f8fd8e82eb0", 378696),
	ENTRY0("stewgoing", "150410", "93fdaa375c1db772b27918f096f43f53", 1604582),
	ENTRY0("stiffmst", "690609", "33746a619e4ac30a1b9be82349187062", 82432),
	ENTRY0("stiffy", "059105", "285b8bb966075fc33a51bd842202f1b1", 91136),
	ENTRY0("stinkorswim", "090105", "7ed52b7400c044b4c3b47fa44154b5bb", 232448),
	ENTRY0("stonecave", "091103", "87858ce7d11ae814029c5b550682c255", 220672),
	ENTRY0("strainedtea", "120407", "628f7a272a28c825314fd207e50913b6", 166400),
	ENTRY0("strangeworld", "091012", "d7c09eadf30d77858c88dc0f6706ceaa", 247230),
	ENTRY0("suicide", "101103", "a936b12bd8f8c80e5948f6035e1eb86b", 343040),
	ENTRY0("sunburst", "070222", "9f1eee6db8dce7cfb803e5c430fe942a", 65532),
	ENTRY0("sundayafternoon", "121213", "73d9c96d54922bcd58781aff41e449ed", 347136),
	ENTRY0("surfboard", "201027", "b3e4c1784465d78111b3eda7b7969647", 180224),
	ENTRY0("survive_zcode", "120116", "5391b23b5d02a9ff853ef93f76bb73f6", 89088),
	ENTRY0("sutwin", "970402", "b4fe3b29aab816470906ce3ae0613ba4", 31744),
	ENTRY0("suvehnux", "150314", "c085af56acb090e7e4c8af6f69ed216f", 248320),
	ENTRY0("swineback", "060422", "853342a5b088a2998201123b0c5faa73", 111104),
	ENTRY0("swineback", "060507", "da84d38e1f95b22268be89780f4b7392", 135680),
	ENTRY0("sycamoratree", "980107", "335cb583a295cfd8a3de4e9cd2a267ee", 54272),
	ENTRY0("taipan", "020520", "0bd7a62517400c66fc961908688d1671", 37888),
	ENTRY0("tatctae", "970521", "21cf40691e0ba92d8e699b0fa3e18728", 378368),
	ENTRY0("tauntingdonut", "080612", "4d8443e896e7f294cc4a250c6090f6cf", 167482),
	ENTRY0("tblw_zcode", "121030/z8", "4b6744d2b13fd85db5020788e0888f0d", 295424),
	ENTRY0("tcomremake", "121103/z5", "2fd621df080f2b8c5fbe78ca99dc00c4", 176640),
	ENTRY0("tcomremake", "121103/zblorb", "aa9995de3edd044db0f10fee2ba0f3ba", 256500),
	ENTRY0("tcoty", "060908", "8bff4a19b0c1b9aa661f6e00d86df3b4", 471950),
	ENTRY0("teacherfeature", "071126", "be559856dd11f6b1fa5c5ed10f83f90a", 118784),
	ENTRY0("teachher2dance", "110603", "2ec7574f9fab0c9f85b6bdaf3605eb21", 162816),
	ENTRY0("terribleoldmanse", "100819", "518a610437203e063fa550d5c240e664", 891570),
	ENTRY0("terribleoldmanse", "100822", "6cefab1d2c3e2ba2e95e5e541b617ac4", 892080),
	ENTRY0("terrortabby", "080514", "86d448c209e93f1caa12a319406313fa", 204800),
	ENTRY0("tesseract", "031227", "583899fe706bd1367e31ad5a9f1c8ed2", 93696),
	ENTRY0("textgolf", "010114", "5dd8638e40d97806c5eb9c05618db775", 195584),
	ENTRY0("tgm", "050330", "e345e2527801880bffc74981571875c9", 44544),
	ENTRY0("thanet", "200313", "48029d6809ec3d3ddb5d6c69f8e88bb1", 176128),
	ENTRY0("thanet", "201113", "d0dfcc5c81ac97509731211b5733fdee", 176128),
	ENTRY0("thatdamnelevator", "160710/v1", "4986a2629b35b9092b5bddf40df5417b", 415184),
	ENTRY0("thatdamnelevator", "160710/v2", "825f9b4e3e5957a9ba1b36ed87c91a8f", 346034),
	ENTRY0("thatdamnremote", "091214", "2b10cdcc7a83f6e82d3909838e5b439f", 251490),
	ENTRY0("the5continent", "190719", "945907c269cf9e20a7584544df8db671", 171520),
	ENTRY0("theatre", "951203", "8b3db2f9039696ffa0114d55ac219ab9", 185856),
	ENTRY0("thebaron", "080228", "c33cf229f46f597755fd8f0cd6360d6e", 310272),
	ENTRY0("thebaron", "080504", "e0c22797af9e7f6aa07077e056e4c1d5", 311808),
	ENTRY0("thecomputerlady", "190702", "025908635a0028671f099b10e0ad5141", 165376),
	ENTRY0("thecomputerlady", "190715", "fd554c7932f724331a04754a4623b2b2", 166912),
	ENTRY0("thecomputerlady", "200804", "bddc350598549850e99c61484e711dfb", 166912),
	ENTRY0("theenchantedcastle", "190527", "d06c767152545fc81b06ac004389afbb", 455630),
	ENTRY0("thegrandtour", "190512", "d651655f0500e4f6997599d25bd23c2a", 159232),
	ENTRY0("thegreat", "010902", "285ad766a386886e814e6c22ed990a7f", 91648),
	ENTRY0("theinvestment", "200916", "d2375e3e699726214174dfe656894a02", 34664),
	ENTRY0("thelighthousemm", "091027", "7faade2abcd89e46142aeb6f0c7e004f", 246784),
	ENTRY0("themall", "190718", "c63d87266eb12e24308d9046da2aab22", 87552),
	ENTRY0("thesnowman", "080301", "ae3a9c15dee678e85e3569582a4522df", 367516),
	ENTRY0("thesproutpouch", "170462", "70d182102f3544fdb72968f942ccf73c", 1057454),
	ENTRY0("thorn", "030701", "63faf28ec7cad962816e9ed3a7310a74", 75776),
	ENTRY0("threecows", "120208", "449fd20d3b0981ba6f8a7d929e56c820", 153088),
	ENTRY0("threediopolis", "160814", "cc601a569e19166cfd3b379e7f6dc2f3", 590176),
	ENTRY0("timefortea", "100617", "af469380d1ace75480a80f578091ac4f", 313344),
	ENTRY0("timedwarves", "200714", "743e66961a9f4ada8ad4a31181009f25", 93184),
	ENTRY0("tirehoax", "080104", "09b696f73a1d2d37ee376fac97a2c406", 228746),
	ENTRY0("tk1", "950925", "7c93a305295c891ba9de5cad4c190f8c", 52224),
	ENTRY0("tkatc", "151121", "cecca5aa05f7ea35550b473b90c3766e", 408576),
	ENTRY0("toask", "130614", "1491b55bbcb2a0e7b1bfe99b94a9d387", 524288),
	ENTRY0("tok", "100227", "1b3b24c4616bf7e47e99eacca7308153", 241590),
	ENTRY0("tower", "151228", "04de5c49bb3eecb4e4d1cb0fd5ea4a93", 391156),
	ENTRY0("towersofhanoi", "220627", "49724abc6310ba9fa5f5cc32cf1c0b38", 354412),
	ENTRY0("townmusicians", "120125", "35624466eb61b2dcbc408cd6c75a6ab9", 414042),
	ENTRY0("toxinx", "110519", "2223181742216f1c1bb74bca88dda729", 200704),
	ENTRY0("trampelviecher", "090611", "4d7871da10dd418a9b4ed3e00fab97c9", 207360),
	ENTRY0("trapped_zcode", "110413", "b05f8909d1bbedb97f6c50eac19d6b2a", 254362),
	ENTRY0("tristamisland", "200925/Demo", "6fc31da7d0afb034f05b12da56d075a8", 56320),
	ENTRY0("tristamisland", "220107", "a0b1f6d3aa9167bb2c8039fe9e4c69c0", 56832),
	ENTRY0("trolleyeview", "980518", "3e0f69e678dd289cd32bf41b2be58bd6", 64000),
	ENTRY0("truehero", "130916", "3175612caba7b24c091ed4c102c8b902", 195072),
	ENTRY0("trunspecified", "101224", "cc1ee9ddc4b64f0e5a2b6b91bcf8f363", 273920),
	ENTRY0("trw", "021229", "1fdb2baeefcabb635ddbbb3433b1b125", 126464),
	ENTRY0("trystoffate", "970629", "7f7eb7e7df03025b9046139d042dd363", 165888),
	ENTRY0("tundra", "980127", "fd6f3bc3b4083ddbc7a090842bae2a9f", 73216),
	ENTRY0("tutorial", "111030", "36147ba605eb49902f9d514a08638cd1", 468142),
	ENTRY0("tutorialhotel", "100713", "2d57f52f53fa0845eb19ea97a29005fa", 154624),
	ENTRY0("txtadv", "051019", "c8bae5d42c33802db953d2ffb45216b0", 86016),
	ENTRY0("uhoh", "980218/Demo", "4e8b90354c96760f36bfbfa2ba4fc04c", 176640),
	ENTRY0("umw_zcode", "201215", "24c7e6f775b9c10d9579f2a77be872fd", 167936),
	ENTRY0("underdoos", "970329", "3ef9b348b4223b901bdfacb854fee16e", 58880),
	ENTRY0("underground_zcode", "190730", "18c3cc7098f9e613e6c108e84c8947df", 368128),
	ENTRY0("underground_zcode", "191215/zblorb", "c7eef6e46e4b4868fd140c0ec06f3e7d", 3458170),
	ENTRY0("underground_zcode", "191215/z8", "6d2d883d5cf75db5e026ebbf4988d663", 416256),
	ENTRY0("underthebed", "120522", "96659568d70385c022a9ecc41fd6219c", 246272),
	ENTRY0("ungodlyhour", "140302", "a8ac75f3bd1b7ea4a367d56b1c6e8793", 333874),
	ENTRY0("ungodlyhour", "140927", "d91b088567ff78a3caaa863cfc6db7b0", 333874),
	ENTRY0("unicornpool", "100914", "8bf72169c75e347b52aa592b544e1357", 111616),
	ENTRY0("uninvited", "121216/z5", "2f364062b6d1f05c8b518a11d58c399b", 193536),
	ENTRY0("uninvited", "121216/zblorb", "c2b16596310202bf40c512c5de571858", 623410),
	ENTRY0("uninvited", "190918/z5", "1e372c61557ec4a25a88f9eb8161c18c", 211456),
	ENTRY0("uninvited", "190918/zblorb", "e1da4739992b32ce7b8655f245fc9099", 641342),
	ENTRY0("vacation", "060527", "0a0da195fa5c41a59028e69eeae2d9e0", 248320),
	ENTRY0("vagueness", "080825", "6d3641b71d42516e573ff0cff90cd40e", 298402),
	ENTRY0("vampiresun", "020405", "3364b9d9d1a754e1e3bab61a96957ed2", 254464),
	ENTRY0("varicella", "990831", "f5791cd7d8ebfd568928eb2b888a5264", 501760),
	ENTRY0("vengeance", "000000", "cf68c5b27e9275a0962b622ed4662108", 83968),
	ENTRY0("verge", "120523", "c7e60b875d240ee83dfac37596cce177", 267776),
	ENTRY0("veryvile_zcode", "190928", "14f7780e689e0cf4c321904332238970", 490998),
	ENTRY0("vigilance", "070106", "97364e2e3f4197bf9dcfeef8fa6e6ee9", 183296),
	ENTRY0("villagelabyrinth", "240126", "03f1df1242899f17942e7e67254e874e", 159744),
	ENTRY0("vindaloo", "960613", "d4e69f29f435b55dff5057a1d0d5bd45", 53248),
	ENTRY0("vergingpaths", "151027", "c6df1e824df593e8c4995502e6704571", 1131672),
	ENTRY0("virtualgrandnation", "200426", "e8f662bdd08fa0f7520e8ef5615c12b5", 175104),
	ENTRY0("visualizing", "000218", "81458ba52d6deb8b30ee2628b45976dc", 97792),
	ENTRY0("vosr", "060925", "73746a1dd6938ca4b034eb424936ffe7", 267776),
	ENTRY0("wadewar2", "000806", "5a6370665b128e4688b810fae4393232", 124928),
	ENTRY0("wadewar3", "020512", "458b5b368a71329dcfc59223c449cf88", 143872),
	ENTRY0("warblersnest", "131221", "147d80b524cf8ee812459c0a7426cb87", 863268),
	ENTRY0("warp", "640101", "3554d76096e5d06d9417e20a21e95ea0", 70144),
	ENTRY0("weapon", "010706", "b28a2d2ab4a2ba54e5c5d980764c26a4", 142336),
	ENTRY0("weareunfinished", "160419", "28d307b40b627af2708c71aaf43a64d5", 413298),
	ENTRY0("weirdness", "030922", "e98bc679d94c0c1c6a241737f7c8ae28", 89600),
	ENTRY0("weirdcity", "140613", "0a434b5011389e47b391b707ccf6611a", 339786),
	ENTRY0("welcome", "160213", "71a3316b000ce44a1b0e2ecc4eaa96a0", 160256),
	ENTRY0("welcometohell", "980816", "1c7493e0ca533b9cc04097d90b112b24", 51200),
	ENTRY0("welcometopuerto", "050101", "11a2ac552be710a810ce0f41862c40f4", 62976),
	ENTRY0("wernersquest1", "020225", "e158f13e9f55eef58a0f7a6affec180e", 52736),
	ENTRY0("wernersquest1", "200213", "11e56816ebe10789a557f16e351805d4", 58132),
	ENTRY0("wernersquest2", "020225", "6ebf8ffd5687674754473f4851df4c01", 52736),
	ENTRY0("wernersquest2", "200213", "4e84e63eb06a2240e431352b38b1b42f", 55468),
	ENTRY0("wernersquest3", "020225", "690a16946cacccaf2d5fd3ca2cf9fc4a", 52736),
	ENTRY0("wernersquest3", "200213", "24dbc99bf6bc085ec6e91be050a3156a", 55368),
	ENTRY0("wernersquest4", "020225", "2098db329eff1b0d1cad56f89bd38723", 52736),
	ENTRY0("wernersquest4", "200213", "df9b7d3fba29b450fb4d666130693cdd", 56220),
	ENTRY0("whispers", "050905", "4cbba9cf2b5bce84ccef8e40482ef230", 205824),
	ENTRY0("whispers", "050926", "360c24ec5c47ced2b03ce38c6e4e2e8d", 177152),
	ENTRY0("wildflowers", "140825", "11df47a8f9ddebbe3ed903ef8c85597a", 518936),
	ENTRY0("williamtell", "021025", "4f098036ffdc7463000c4f81b6300123", 68096),
	ENTRY0("winchester", "020131", "9abc235bc791a3f38ee81aa6496112d0", 293888),
	ENTRY0("windhall", "960828", "eced9e2a38a5b694ceab8ffa4c120b90", 320000),
	ENTRY0("winterwonder", "030227", "0994f3c477e8e221d9c70df72fb936df", 194560),
	ENTRY0("wir1", "060503", "f9b1425b5c55f1b3f3caa50a7963be17", 652866),
	ENTRY0("wir1", "100530", "b74d7ef62857dda75b7f233d313bc734", 673026),
	ENTRY0("wir2", "060503", "c64dfefbc7a320126342f6a522a3b8bd", 514520),
	ENTRY0("wir2", "090410", "8edb7a609900d9c62d28f05b149336f6", 578234),
	ENTRY0("wireless", "040909", "2af916f6295f32533eadd97afccb11d8", 140800),
	ENTRY0("wizardscastle", "000918", "b205946471d687d83b8dfc988734ab43", 34304),
	ENTRY0("wizardsmagic", "190727", "4b1c54d572b50f428f2aa6d7b7a79b5f", 165888),
	ENTRY0("worldupsidedown", "151224", "1b7311638555848aaf3a50857ed4035b", 457496),
	ENTRY0("wsp", "050718", "b36c21f76893d6329d0d3a77e59792b8", 114176),
	ENTRY0("wump2ka", "041119", "64e790c40f04ab7fe2405807f0cecb9f", 174080),
	ENTRY0("wumpus", "991216", "abdd37af526d03538cbb20d91a941489", 12800),
	ENTRY0("wurm", "021126", "0ffbc60fcccaccf1abfa877acf293b2a", 4096),
	ENTRY0("wwwanderer", "080705", "a401b781048229b05b444bdbb68e5b71", 283592),
	ENTRY0("xenophobia", "111115", "d132c3b5defcef212f36e03a7c9a2e74", 124928),
	ENTRY0("yakshaving", "101127", "3c4114a0999f2a68bd08c58fe0494a97", 394958),
	ENTRY0("yakshaving", "210311", "388cf2afe8648e481b292ef435b099c1", 688198),
	ENTRY0("yastanding", "240105", "828fe02ff64d29f7cb80cbc235e9b157", 8857),
	ENTRY0("yomomma", "100228/z8", "a63d6aa5c9741b7c0ccb27756c741c12", 398336),
	ENTRY0("yomomma", "100228/zblorb", "325cd63060b5509300c71af58902670c", 1081664),
	ENTRY0("zassball", "980314", "ab906aa444b7fdd0a92119828616bb58", 12288),
	ENTRY0("zbefunge", "021128/Beta", "6ae16ce61922211922edf777bb6113ce", 56320),
	ENTRY0("zcamel", "000918", "96b316f9b7e133eaccbec98a4fb0cf46", 6656),
	ENTRY0("zcatalog", "980519", "f5ad7533e0d33ced32429a04473ec3d0", 126976),
	ENTRY0("zchess", "040124", "456fe3c2d3d986e8652c4a439e738686", 6656),
	ENTRY0("zdungeon", "990126", "13575669a2cae1d13b3cb22e868d8929", 188416),
	ENTRY0("zdungeon", "040826", "ddcf45ee10cc5f42ac1b273eb9a0d1f8", 188928),
	ENTRY0("zedfunge", "031111/Beta", "3198856622768685e17ffba6b8313e49", 108032),
	ENTRY0("zedit", "971103", "40a26af1581ebdbbd867b18229ce8187", 65024),
	ENTRY0("zegro", "041112", "bb7a0ef8d23cfb88afb2dc1e40be9613", 261632),
	ENTRY0("zenon", "100122", "2c82b81ac43fcfbf2abee4d8c4f98cad", 67584),
	ENTRY0("zenspeak", "990217", "53a344bef35a5778307aeb937a31f001", 123392),
	ENTRY0("zlife", "960121", "36bc7d2fdd0bbac996466e05af239924", 6656),
	ENTRY0("zokoban", "990810", "e599ec4ff1eee51afbd6a3f8e8d8680e", 18944),
	ENTRY0("zombies", "990524", "4535f316650ee6c76b95ced1100b12d0", 7680),
	ENTRY0("zork285", "211010", "b11e9810a7277a5aeddfdeb7ce5d90a8", 38600),
	ENTRY0("zorkianstories1", "121014", "dbf1900ac273a3051cc2bd405aef4620", 380454),
	ENTRY0("zorknplus9", "120517", "3d95fc9e6e60d372413bf10a90570b2e", 402874),
	ENTRY0("zpegasus", "130708", "f65a086805d473b3ccbfb9971383ef65", 417200),
	ENTRY0("zracer", "071203", "04d60a3f69b68955b008cf7a7b08017f", 40960),
	ENTRY0("zsnake", "010201", "f6aca196d9a1de4f05df6af74327f8f8", 11264),
	ENTRY0("ztornado", "030711", "dd3edc31ee39d186566a6386c29aafe1", 20992),
	ENTRY0("ztrek", "000229", "99653ccefa7203a37e8f0ed71c88fa98", 29696),
	ENTRY0("ztuu", "970828", "ee5f33204264a1316c6acdb1036d19a9", 229888),
	ENTRY0("zugzwang", "980228", "9b0dc30345325e87bb9f54ebbfb0112f", 58880),
	ENTRY0("zugzwang", "990710", "e538e90a90a7c280a078b50d2ad2e44d", 58880),
	ENTRY0("zunidoll", "971031", "128ad329e657c405f85ddbc19bd26538", 76800),

	// Painless Little Stupid Games
	ENTRY0("plsg1", "110205", "1cb5c04a2373bbda0bb6abcc3d49ba84", 168960),
	ENTRY0("plsg2", "110210", "c3868744b56fe4b3b8e8e5c1eac80864", 181760),
	ENTRY0("plsg3", "110310", "a640d77b82894fde6b6b3d3cc89553f7", 180224),
	ENTRY0("plsg4", "110414", "7eda62012f870848603ba669d3a3d305", 198144),
	ENTRY0("plsg5", "110420", "f5c7139c859b8f5209115b79479314e2", 181248),
	ENTRY0("plsg6", "110527", "e972666101acf79534d71686fa68f369", 185856),
	ENTRY0("plsg7", "110630", "dd0a2e670a56cd937671b94394bc0fbb", 182784),
	ENTRY0("plsg8", "110822", "24d0f12ead2634292dc07b634d65059a", 164864),
	ENTRY0("plsg9", "110901", "32cddc8302fb9b566c83d4c8d5d928be", 206336),
	ENTRY0("plsg10", "111118", "30705c933686364f9086a36aac9579df", 260096),

	// Converted Scott Adams Classic Adventures games
	ENTRY0("adventurelandi5", "941017", "cde66d37efaff46f18e67b0f39e4d0cd", 43008),
	ENTRY0("adventlandsmpleri5", "980329", "86c473c81e86637105108afa943c3ced", 23040),
	ENTRY0("buckaroobanzaii5", "980329", "cdbf748cdcee3cade378cf62cfe01d43", 24064),
	ENTRY0("ghosttowni5", "980329", "0240f4119bb9b8e8197f37049c9b4f82", 25600),
	ENTRY0("goldenvoyagei5", "980329", "d986f2ac673abdce741c90e8b9fc3acf", 26112),
	ENTRY0("marveladventurei5", "980329", "964dfa22fcd54d2674123951af79136b", 25600),
	ENTRY0("missionimpossibli5", "980329", "9c759b65e43e2d9d6aa02122248040ae", 24064),
	ENTRY0("mysteryfunhousei5", "980329", "4b78c1883356db8184b351c5a269fdce", 24064),
	ENTRY0("pirateadventurei5", "980329", "0c3d27eaa6563835bfb1aadd309e7a00", 24064),
	ENTRY0("pyramidofdoomi5", "980329", "a6fc7fd81b7330bc254afbac17b29058", 26112),
	ENTRY0("questprobe2i5", "980329", "6b9cb86c332c092b3a93973ba9f4c946", 27136),
	ENTRY0("returnpirteislei5", "980329", "56e5d7c33d5403ed59a62f67744f4d02", 26624),
	ENTRY0("savageisland1i5", "980329", "6f6cf307a97becb32524fe66a910587e", 24576),
	ENTRY0("savageisland2i5", "980329", "a43ab1063e6a8d4849ad3b69f1e4cacb", 25600),
	ENTRY0("sorcererclaycstli5", "980329", "54cc89bbead7dac21455b9c00f32f604", 25088),
	ENTRY0("strangeodysseyi5", "980329", "8216fc5ca7ed593d6a9c4265064d83a4", 24576),
	ENTRY0("thecounti5", "980329", "a0060ef9c9fa5cc3d3dbbc060f6451c2", 25088),
	ENTRY0("voodoocastlei5", "980329", "bff285e6c9291fc6ba77c9743b610c2d", 24064),

	// Mysterious Adventures by Brian Howarth
	ENTRY0("10indiansi5", "980406", "b3e4b8376f7c553064ceff8f25936385", 22528),
	ENTRY0("10indiansi5", "041209", "63b01fa007b977be144bcd3a6f6e8dcf", 29180),
	ENTRY0("10indiansi5", "110126", "0c5d25323a3b649ea432025001edb638", 170492),
	ENTRY0("akyrzi5", "980406", "61c29077bee55dce614a729705099282", 24064),
	ENTRY0("akyrzi5", "041209", "cfbef40e735057b6fbbde3991f6ee4c9", 31232),
	ENTRY0("akyrzi5", "110126", "92c5abee1a097f1e8e61f744ba9ddb3f", 119296),
	ENTRY0("arrowofdeath1i5", "980406", "a3827232bf54c339a5ec5ab906fd1857", 22016),
	ENTRY0("arrowofdeath1i5", "041209", "616f481469279a1184d1d8fcad84ed4e", 28156),
	ENTRY0("arrowofdeath1i5", "110126", "3deca9c9fce4fb995e0681ecdfb39cf2", 173056),
	ENTRY0("arrowofdeath2i5", "980406", "5a437f2cbc4f99d8cd741e83e2abe4cd", 24064),
	ENTRY0("arrowofdeath2i5", "041209", "058dbdf618b22e9dd47f42f7e98e6fdd", 30716),
	ENTRY0("arrowofdeath2i5", "110126", "4a0c494ff4564e659a29fbd3d67696f6", 172540),
	ENTRY0("circusi5", "980406", "be7a07e042f5d1b0cde3d3b1cd85dee3", 22528),
	ENTRY0("circusi5", "041209", "0c5a65e665b773fc39bdcbe194ad99cc", 29180),
	ENTRY0("circusi5", "110126", "72721053ed49e1d701146332215f63e9", 118780),
	ENTRY0("feasibilityi5", "980406", "e0c46523e043bc75f8e04714396e17ff", 22528),
	ENTRY0("feasibilityi5", "041209", "2159059c9b506af4f10c7cf9133fdd00", 28672),
	ENTRY0("feasibilityi5", "110126", "8f2f18c6fd76be74e612ee9b271055b1", 172540),
	ENTRY0("goldenbatoni5", "980406", "408b31a15c429f7ca83b2ac80764ffa8", 20992),
	ENTRY0("goldenbatoni5", "041209", "817ca85193d842b9716d4b688d6fe9d1", 27644),
	ENTRY0("goldenbatoni5", "110126", "9de3f1a8624e20409c92325b30a3b490", 156156),
	ENTRY0("perseusi5", "980406", "c1ab2f87658691f773599d9973bf72a1", 23040),
	ENTRY0("perseusi5", "041209", "1c0fef44034daa16ada548caac232337", 29696),
	ENTRY0("perseusi5", "110126", "39be8e28753aa0ac87c49b6dedb712a2", 132096),
	ENTRY0("pulsar7i5", "980406", "d93cc91cda58c75259b2c872921a17a8", 26112),
	ENTRY0("pulsar7i5", "041209", "41a2b9048af4600f43c829e2348b5fc6", 32764),
	ENTRY0("pulsar7i5", "110126", "5d6897ee80078c0286a52589ae305633", 135168),
	ENTRY0("timemachinei5", "980406", "e0a0335705aab9642b7625f26c00eca2", 22016),
	ENTRY0("timemachinei5", "041209", "460ad097aeb7b800f237692aaec8fda2", 28156),
	ENTRY0("timemachinei5", "110126", "684e96c6adaccfd5f4138dce069d3fc3", 137728),
	ENTRY0("waxworksi5", "980406", "98e52d813cb28f899916ef7129c85a0e", 24064),
	ENTRY0("waxworksi5", "041209", "859a006a14bd69b22135688248756ba0", 30720),
	ENTRY0("waxworksi5", "110126", "c51e911d1228d8adfc07ed138bc90079", 116220),

	// Apollo 18+20: The IF Tribute Album
	ENTRY0("apollo1", "120315", "1aec299147675a5c6e10d548a4eeba05", 251100),
	ENTRY0("apollo4", "120322", "83f92f1fc39bd699ac32935d8eefd35a", 426528),
	ENTRY0("apollo5", "120324", "6c889ec9c330169740afafe7694030cf", 199680),
	ENTRY0("apollo6", "120316", "a13d84152dc3b9ec5e6b489d987e8130", 906236),
	ENTRY0("apollo7", "120324", "a08c999b6408a9c17e2f3ab55a82e919", 363458),
	ENTRY0("apollo8", "120325", "1b0e0c32915e81873bc30f900bfdf371", 352706),
	ENTRY0("apollo9", "120323", "e48c6dad0b11bde066c4e29da68f71ef", 448972),
	ENTRY0("apollo11", "120304", "ad4b4d5a6d2c0c251a891cca5d47dfcd", 262406),
	ENTRY0("apollo13", "120323", "b45ad2f48f3da1bbe8650a2df70ba625", 350570),
	ENTRY0("apollo14", "120324", "dedd30cae688dc8a88bd79d8004ee6e0", 278016),
	ENTRY0("apollo16", "120324", "e7cc8f53c82196ef2281e1345c5048df", 155648),
	ENTRY0("apollo17", "120324", "cd6aef7581d5c7221d2b95b55d4d3961", 264822),
	ENTRY0("apollo18", "120324", "604b89f3066da9a62922ff2c45a669ea", 327008),
	ENTRY0("apollo20", "120324", "1cca9cd17cd354131ae0888307ec1ec2", 200192),
	ENTRY0("apollo21", "120324", "6d6fc49e004828df9ca9fa2013a0a231", 199680),
	ENTRY0("apollo22", "120323", "7284829f0999d3cfa9de81a97bc6f2e2", 385536),
	ENTRY0("apollo23", "120324", "f7a7021baaa261e70d284e2c97e54115", 345088),
	ENTRY0("apollo24", "120324", "c9bbd95fdfbe3fda922cfba0f57d201c", 310000),
	ENTRY0("apollo25", "120322", "2ae649402f52358473f1a61bd8f0cfd4", 369664),
	ENTRY0("apollo26", "120325", "1327ebd3016d873961f6ee35408a03fb", 251354),
	ENTRY0("apollo27", "120324", "1cc4273e5417578445a6b528dd3cdff7", 208896),
	ENTRY0("apollo28", "120324", "7c655dde263ec0e75289e4a77d2b53c3", 173056),
	ENTRY0("apollo29", "120304", "f4d87b9a51126d095a2af8de5bb6ad04", 494298),
	ENTRY0("apollo30", "120324", "53f36fb7ff374a59f77679cf3a5d1bd5", 207872),
	ENTRY0("apollo31", "120314", "fb8cad57d9305ffbfc2dd69e6406a0fd", 197120),
	ENTRY0("apollo32", "120323", "8ec4ee91cd1186c4deef126b40b78cfc", 151552),
	ENTRY0("apollo34", "120325", "78f0a5bfce8eb2801350793738a2355f", 281600),
	ENTRY0("apollo35", "120324", "bcce4073de352739be245f21685ef841", 336864),
	ENTRY0("apollo36", "120324", "01f6f3552fff6a8eaaf5ad31c4c57f64", 209408),
	ENTRY0("apollo37", "120323", "55b8607a451ab30d03314f815a30e40b", 327144),
	ENTRY0("apollo38", "120321", "a760169a6643d08ff3db22be84caed37", 353734),

	// Acorn User 1996 IFComp
	ENTRY0("bse", "970110", "d260f722540f8c24a8c5b88778c76261", 89088),
	ENTRY0("thewedding", "970602", "40b0b13d420f894ebac54106f0e92ff8", 151552),
	ENTRY0("thewedding", "970825", "b2f3bee12b17b4d905d05f72d83932a3", 152064),
	ENTRY0("thewedding", "100221", "b3749fb3d3999331bcd4415969cb6602", 255940),
	ENTRY0("leopold", "960726", "b396f6af14b77671bfcea4a711fd6f8c", 140800),
	ENTRY0("transporter", "960729", "4359c7d02e08f46a6262add4bd810bf4", 94208),
	ENTRY0("blacknwhiterag", "960730", "61a07a049c5160b95cacac9995cf38d6", 42496),

	// IFComp 1995
	ENTRY0("if95_weather", "950819", "0f8eca394c2956f12e0efc6027a60395", 93696),
	ENTRY0("if95_weather", "960613", "00f90d5b28604243708ad41cc6a7dcea", 93696),
	ENTRY0("if95_mindelectric", "941008", "900aae2e5ef17d957e26e8bc0ef90b86", 70656),
	ENTRY0("if95_mindelectric", "950830", "570cd97c66cea6097533fefbd9072f56", 70144),
	ENTRY0("if95_magictoyshop", "950726", "4ad2912aa56d0962a74c4fb51b472f10", 71680),
	ENTRY0("if95_magictoyshop", "951018", "68d597fc1a7b22419033628967dad19f", 73216),
	ENTRY0("if95_mst3k1", "950814", "de8b17e315820f75556530404bb4fac0", 81408),
	ENTRY0("if95_mst3k1", "960831", "362107be49493f6ed1ca7664d7884a66", 105984),
	ENTRY0("if95_libraryfront", "950829", "fc1d5c6b54efb2fd051eef2f1322afff", 55296),
	ENTRY0("if95_libraryfront", "951204", "e54ca81e93629a6e8ab2dbc84a6712b4", 72704),
	ENTRY0("if95_tubetrouble", "950831", "3651c39edb3d9cf0f63f32e6e52e705c", 50176),
	ENTRY0("if95_tubetrouble", "950901", "478a208e21def77097309c2b152a968e", 50176),

	// IFComp 1996
	ENTRY0("if96_sherbet", "960928", "d3539f871e1b25f279e98857ce8e514e", 157696),
	ENTRY0("if96_sherbet", "961216", "46a865c6f01a800536463ffe93fb3d9d", 174592),
	ENTRY0("if96_tapestry", "961010", "a8e97156b7211dea1aa94471f0509042", 185344),
	ENTRY0("if96_delusions", "961021", "c1f63eacbe78cb0e7721e8c0d403fab5", 166400),
	ENTRY0("if96_delusions", "971121", "8e78eef73d07048d99514bab624fb9aa", 193024),
	ENTRY0("if96_fear", "961012", "a346c031516cbd4bd493d178317e28e7", 102400),
	ENTRY0("if96_claw", "961003", "ed2820d0570051d4f7d025034514882c", 126976),
	ENTRY0("if96_claw", "970327", "4d3992e95530a301ca58939a197c6b1b", 130048),
	ENTRY0("if96_lists", "960823", "c4373396c5c8e499073b175349161359", 116224),
	ENTRY0("if96_ralph", "961005", "425c2253d6b083c20a0cb04ea6403a54", 59904),
	ENTRY0("if96_reverb", "961015", "297a86543115d3c3f5cd5710fea9213f", 100864),
	ENTRY0("if96_intheend", "961010", "f38c34e0d7a67271bb469c629db10cd0", 106496),
	ENTRY0("if96_piece", "961015", "2b308f878a82041aadc5b9f9f745221f", 103424),
	ENTRY0("if96_piece", "970819", "5703a209e660a58f456b0ec99707c3fd", 105984),
	ENTRY0("if96_phlegm", "961014", "4a10ddf90e3d3b5671b28a881e11a05a", 69632),
	ENTRY0("if96_phlegm", "970804", "2f6f7667a3c2782aa0f24eeef7c7fec0", 70656),
	ENTRY0("if96_stargaze", "961015", "5534fb19f4dbb71ec4fb8bd293340826", 61952),
	ENTRY0("if96_forms", "961021", "a8c58b3da486d76dfe933a82a0861dc0", 82944),
	ENTRY0("if96_forms", "970206", "7de27dd6539ed6ef1c6176ab030510dd", 91136),
	ENTRY0("if96_housestalker", "960930", "9a8eba9e5d3f88e603ee4e98ca4e2126", 75776),
	ENTRY0("if96_ripflesh", "960914", "81b3ce6c20eded07e831ee9c83508507", 83456),
	ENTRY0("if96_liquid", "960927", "5ff0e573df8126e6142a66716b90acf9", 68608),
	ENTRY0("comp96", "970626", "208b6f721e472f89654e6feb1b54b747", 84992),

	// IFComp 1997
	ENTRY0("if97_edifice", "970930", "15d3cb03378412c6c3d0b050e5d6392c", 173568),
	ENTRY0("if97_edifice", "980206", "e2fd79d86f7e77659ef60519d2423856", 181760),
	ENTRY0("if97_spring", "970929", "9b891a246a0a5869e73c42527950143b", 256000),
	ENTRY0("if97_bear", "971011", "71d95521937e8c4e6753d656ae264a28", 105984),
	ENTRY0("if97_lostspellmake", "970927", "e06790b6d56f8b3efafc24c990d5138b", 117248),
	ENTRY0("if97_mimesis", "970925", "c52b125a2fa69127500a1fbb35796c28", 87552),
	ENTRY0("if97_mimesis", "980110", "abd66d64b98d47ebc88f69d35816fdf8", 90112),
	ENTRY0("if97_newday", "970929", "7bebae12833d6253517493de19dd5f91", 121344),
	ENTRY0("if97_newday", "980227", "c43124915cd5677e0c2e89f081586e8b", 125440),
	ENTRY0("if97_erden", "970930", "964d7e321582389e1a43c9022876787d", 322560),
	ENTRY0("if97_erden", "980131", "6125e83116854253240b489a22e83b9f", 348160),
	ENTRY0("if97_friday", "970927", "cc15b58c93d67f52b23e5f8d2b9e3f3c", 94208),
	ENTRY0("if97_estrange", "970929", "091cda6fc1229d4e1815d968782583c6", 237056),
	ENTRY0("if97_sylenius", "970928", "d2858bdf7d92b2fac11db566bfa58b44", 184320),
	ENTRY0("if97_pizza", "970929", "b0b781abec7cab85a322dc24636f80ed", 140288),
	ENTRY0("if97_agb", "971107", "f62be557f60ea0b38a5a1f28b81286fd", 137728),
	ENTRY0("if97_tdragon", "970929", "830b18f092b4930720606f320d6b4d86", 107520),
	ENTRY0("if97_thetempest", "", "b66d923bb90e5c0c84b79d0934e9a7b6", 155648),
	ENTRY0("if97_pintown", "920902", "a30ad40683abd80799c83c834ee53a64", 95232),
	ENTRY0("if97_congrats", "970929", "1b461a7891a218485588e5890900360f", 56320),
	ENTRY0("if97_cask", "970917", "87014e2b9f3910b184b4d7b331993c0b", 59392),
	ENTRY0("if97_symetry", "970925", "386fe4c055155299f6613deb76231403", 58368),
	ENTRY0("if97_auntnancy", "970906", "ddb9e0a2a904ed9d7f177a5f12d77905", 72704),
	ENTRY0("if97_cominghome", "970719", "249537ad84a4dd9ee255eb8c9e91d7a7", 71168),
	ENTRY0("comp97", "971009", "28737e33e980aa77c9e52a3bbbeec6fb", 103424),

	// IFComp 1998
	ENTRY0("if98_photopia", "970918", "6e5e8f088a763de43516926b3e2dc3bc", 188416),
	ENTRY0("if98_photopia", "980914", "e07246e6a3961b7905e927dc5afe9601", 187904),
	ENTRY0("if98_photopia", "981223", "7c86e1a9aa2cf27b16b7b60e30d5b584", 194048),
	ENTRY0("if98_muse", "980927", "d4d3442d178c99a1ba88293a66c244bb", 162304),
	ENTRY0("if98_muse", "990922", "4dd315769b9d4c4782adaac1d42d0283", 175104),
	ENTRY0("if98_enlighte", "981001", "1d259d57bb8e4a84fee03b95554a9c7e", 111616),
	ENTRY0("if98_motherloose", "980928", "dca1f322ec6e290aecb9b0a6712dd942", 204288),
	ENTRY0("if98_motherloose", "981128", "87de736abcf52f27a00d09b74cfda240", 206336),
	ENTRY0("if98_bluemen", "980923", "65fb060e7c67d66dcd15c75b272d720c", 174592),
	ENTRY0("if98_dilly", "980917", "07e1ffdbfdaf760d77a97a78900c8557", 101376),
	ENTRY0("if98_downtowntokyo", "980930", "bd8868ae2b5467fa157483026d480fc9", 80384),
	ENTRY0("if98_informatory", "980929", "bdb68ba2a7b4cc2db474cfc4a6ad90bd", 139264),
	ENTRY0("if98_informatory", "981211", "c9eb276f103f83b8e7044c1f3930264b", 141312),
	ENTRY0("if98_ritualpurific", "980927", "3eb48bc2b376d8d3b2f4c3c4249525bb", 115712),
	ENTRY0("if98_city", "970930", "2b263a9ede155127d8c0d7af62fd1f91", 87552),
	ENTRY0("if98_evildwells", "980930", "2553619a35b7376f808ed2f60aa38bf8", 130048),
	ENTRY0("if98_purple", "980929", "760bd290430bd4056e3d139dfacac011", 133120),
	ENTRY0("if98_research", "980930", "815ff872b7a33cc2475d887172082aa7", 69632),
	ENTRY0("if98_spacestation", "980819", "471606e54416f53dd4edd6a5711b1a09", 72704),
	ENTRY0("if98_cattusatrox", "981001", "727dd7d736460a009442684067defcf6", 111616),
	ENTRY0("if98_spotlite", "980929", "8a0a97abac0455d26277b3ba6433e348", 65024),
	ENTRY0("if98_acidwhiplash", "980928", "6def2593db2cb87961cc389cdce1ea0b", 96256),
	ENTRY0("if98_fifteen", "980926", "185cbedfa98802e842de0daa0111bd9f", 57344),
	ENTRY0("if98_hrs", "980830", "685018bc292fec68de973567f47519bf", 19456),
	ENTRY0("comp98", "981002", "31cca005f642f01f741daba6e861df27", 122368),

	// IFComp 1999
	ENTRY0("if99_winterwonder", "990928", "62ed5cae5f7957cd7174d0053cc0aeb7", 190976),
	ENTRY0("if99_forachange", "990925", "e5e67ba8c7db64b0a58cf57a4d220807", 91648),
	ENTRY0("if99_forachange", "990930", "6cd6f17ea26ed42d56afc367b90de115", 91648),
	ENTRY0("if99_softfood", "990930", "c45d8a94151e6f3016f238450995dba6", 128000),
	ENTRY0("if99_halo", "990928", "4d6a77fa2f83e9292fc216273bf58bfa", 250368),
	ENTRY0("if99_hunterdark", "990918", "165dc72dc8787d6bf69bfd689e2a1338", 114688),
	ENTRY0("if99_hunterdark", "991119", "3c3288e7023f07e525d7301bb84ffba1", 116224),
	ENTRY0("if99_beatdevil", "990926", "fd98f48deb150f638f48202fe2f12fe5", 98816),
	ENTRY0("if99_jacks", "990924", "41779d02b0edae068ee21209f330d73b", 87040),
	ENTRY0("if99_oad", "990915", "6b8d4b73c5922b45e44caeaaa49b0e05", 75264),
	ENTRY0("if99_oad", "000913", "c4d1e88c95803bb9ae0e669c575feb67", 78336),
	ENTRY0("if99_lomalow", "888888", "3e0504b47e42a33d1dcb6d8f1559bc89", 85504),
	ENTRY0("if99_calliope", "991001", "f20eba0e0ad336554d3b95cc5bc374e1", 84992),
	ENTRY0("if99_museduction", "990911", "26a185c675df4ccaa7d58ad5ba4ba4b1", 98816),
	ENTRY0("if99_wossname", "990927", "e06e1accea289d011ab6612986ff8250", 71680),
	ENTRY0("if99_wossname", "000225", "ddf9652869c0892cc2d2150607dbf0c0", 77312),
	ENTRY0("if99_thorfinn", "990929", "49cf016252a16cd53cf45cbb737df564", 97280),
	ENTRY0("if99_death", "990930", "d180e8dc3b9737e3f6ccb9d33cd8ddd9", 126464),
	ENTRY0("if99_chix", "990930", "19bbc5e81c6a21510a6afbea2f128fae", 144384),
	ENTRY0("if99_chix", "991124", "760b7fb0fcd9c99ae1295c06ff52529d", 206848),
	ENTRY0("if99_passbanana", "990921", "a3834e3526f49d947a050bf64f4ba407", 69105),
	ENTRY0("if99_outsided", "990929", "bfdef202ef6f161f3e5124039328009f", 115200),
	ENTRY0("if99_ludite", "990930", "6934a4825c718e8d70ab79667e8a6f08", 82944),
	ENTRY0("if99_guard", "990930", "904703d1341fd6f26b867990a800c360", 346112),
	ENTRY0("if99_guard", "991231", "db56b6f0f381ba32f30749530ebdc25f", 410624),
	ENTRY0("comp99", "991005", "a8500f2cbb186a08fcad4d4bb1c71322", 161280),

	// IFComp 2000
	ENTRY0("if00_metamorphoses", "000930", "f8e84ac3d9f80397c8cbd1201b38ab12", 167424),
	ENTRY0("if00_andrewplotkin", "009999", "8e2294d36ba2668c83da94065633cade", 203264),
	ENTRY0("if00_adverbum", "000925", "f9bf871dc7059293b0c6a096d2b5e6ce", 135680),
	ENTRY0("if00_transfer", "000930", "36182bc6ef876ad9e905eb5cda1f9cd8", 185344),
	ENTRY0("if00_myangel", "000925", "b00787ab7786865c6102f03fdfedd713", 248320),
	ENTRY0("if00_myangel", "001218", "f478be18002189fe31df1c499ceab89f", 257536),
	ENTRY0("if00_nevermore", "000928", "4a536014bc819ebdafe08f10be8cd914", 134656),
	ENTRY0("if00_masquerade", "000930", "2e4541edb1d42b40b8babc4334475c3a", 223232),
	ENTRY0("if00_masquerade", "010122", "a88a4f7cc20883249db3aaae1aad871c", 225280),
	ENTRY0("if00_yagwad", "001001", "ad76da388918f38398139a7d41cdb068", 166400),
	ENTRY0("if00_yagwad", "001121", "6d74b0e48dc46e589efd071e3d329f45", 167936),
	ENTRY0("if00_shade", "000925", "3d7264ec38cbff7393b9625a082d3c75", 107008),
	ENTRY0("if00_shade", "001127", "e9ef7c17a40a4d0a30bb30c37ae1e4c5", 108544),
	ENTRY0("if00_guess", "001002", "7554e5275c00079c5ef5a7dae485d05e", 199680),
	ENTRY0("if00_guess", "001210", "df6dcaadb7f2d25cbbeb1b357c716a32", 235008),
	ENTRY0("if00_letters", "000925", "3a0d7445345279a7e54286845bb2bb4d", 183808),
	ENTRY0("if00_rameses", "000905", "3aa35b977781d22d1b0730e509fbaa32", 162816),
	ENTRY0("if00_djinni", "000927", "8bd611028351e78c8d428762bb50c88f", 105472),
	ENTRY0("if00_djinni", "001117", "107f0d7432596234db354dbe3cbb4b68", 105472),
	ENTRY0("if00_bestman", "", "976dea78b46816162a84a08a26e4e70b", 131584),
	ENTRY0("if00_atwctw", "000914", "3a2bcd7abd981603ad6f6e42862048c7", 176640),
	ENTRY0("if00_dinner", "000930", "ab3ef16685db619e89b76093ba2bf986", 158208),
	ENTRY0("if00_bigmama", "888888", "62128ad2e30f46d459d816bb8c00ae4c", 184320),
	ENTRY0("if00_punkpoints", "991113", "46dc2ffd39a8c8f7a019504c240651f1", 127488),
	ENTRY0("if00_enlisted", "000504", "35107859d4f5fd3cb9a08e09464929bc", 172544),
	ENTRY0("if00_rtzas", "000504", "a49da10f6d8573c40f023b953ca2720d", 277504),
	ENTRY0("if00_gotid", "000929", "5153e600d30c4895281cc6828e3be196", 126464),
	ENTRY0("if00_castleamnos", "000930", "6f2cb75c73c0f29ea67d8a7c817c63da", 124416),
	ENTRY0("if00_masque", "000929", "a445df49ab60a2295fbb42d74276c557", 162304),
	ENTRY0("if00_ppocket", "000929", "2b5bf3234652544ec6cc87f0d9ffd925", 129024),
	ENTRY0("if00_happyeveraft", "000930", "02c281fff208a4480237140b2c1e4d25", 127488),
	ENTRY0("if00_prodly", "000921", "d9bacb93727ab1876a1bac84472b2718", 116736),
	ENTRY0("if00_withdraw", "000930", "6d42f8a5f721b240b3eda80d4fdbf24f", 66560),
	ENTRY0("if00_123", "000929", "bada858e1a1ff3f5a712ca8658181496", 87040),
	ENTRY0("if00_kitty", "000930", "5ae16f7ceca4bd1eec36ba8788544def", 73728),
	ENTRY0("if00_comp00tr", "000930", "0ea1f6de4127fc6b04add637e6afa87b", 151552),
	ENTRY0("if00_asendent", "000925", "5cd09ea080fbb2ea7ad8fe81a562df8c", 61440),
	ENTRY0("if00_whatif", "000926", "c678e52cda08e06b709746b519fb58c3", 92160),
	ENTRY0("comp00", "001002", "e5bd908fb7735764916c195a8d07f9ab", 178688),

	// IFComp 2001
	ENTRY0("if01_allroads", "010908", "1259e45e9adf77ecbe34ae2279fdb5c2", 172032),
	ENTRY0("if01_allroads", "011119", "84ce94d8c6c77452537511f14739b612", 172032),
	ENTRY0("if01_moments", "010908", "06b906e9484d0b858dab813476b66922", 303616),
	ENTRY0("if01_heroes", "010928", "5a1da81f028ff0b81ca1b86771e5ff8b", 200192),
	ENTRY0("if01_heroes", "011130", "1bfe7f301e15816e46c60edfae9030a5", 204800),
	ENTRY0("if01_cycles", "010928", "c0b8938782a4e8bad45acbde871dbda5", 118784),
	ENTRY0("if01_cycles", "020222", "cfe1bb9275f72a63e6b4d96fb39907b3", 121344),
	ENTRY0("if01_eas", "010926", "17a98281223504a9b750cfca9156a4ac", 178688),
	ENTRY0("if01_eas", "011204", "5dffcdadd1f24333aa6cb03fefce22fe", 183296),
	ENTRY0("if01_eleven", "001231", "a573abd0e05318d72502e2ff3b48f5fa", 214528),
	ENTRY0("if01_eleven", "010928", "66de44d555d009a78f2653926462d6dd", 196096),
	ENTRY0("if01_prized", "010928", "bfe292ea9b0ae182afae6964039c4e2c", 218624),
	ENTRY0("if01_finetuned", "010928", "db40fa5a3a42006b70666c91c2de54ff", 271360),
	ENTRY0("if01_finetuned", "020520", "354ff8d5758cd27f054d5f575dd37742", 293888),
	ENTRY0("if01_evil", "010902", "14cecf79c9445057799632835034e560", 125440),
	ENTRY0("if01_gostak", "010926", "3aad7233e05fae59f499fedaaecf75f9", 88064),
	ENTRY0("if01_gostak", "020305", "06a39a234ada7e57733e1ab442909170", 88064),
	ENTRY0("if01_crusadejg", "010913", "cc05c6b1940ed5e5bb53a0abecfbe1bf", 128000),
	ENTRY0("if01_urhere", "010928", "2f0240d8c028d9c249b8892c0de7f547", 196096),
	ENTRY0("if01_elements", "010928", "cb2d89f904f8e4494e4b35561ca888be", 129024),
	ENTRY0("if01_banebuilders", "010925", "fa7780e270430b2959b49f57d780efb1", 90624),
	ENTRY0("if01_colours", "010928", "a2ac5c16802f1c0ed1dc5b32166f67d1", 123392),
	ENTRY0("if01_siliconcastle", "010910", "56f506aab6fd10750a279f565f2c2696", 217600),
	ENTRY0("if01_siliconcastle", "011119", "51d94f5e8684cae443e28c16e3e04fea", 211456),
	ENTRY0("if01_timeout", "200108", "25fd8875971dd2ae460e63c2ba71e25d", 119296),
	ENTRY0("if01_schroed", "010314", "48dc6a08ad6619d17dbf03b8dff0e2db", 59392),
	ENTRY0("if01_jump", "010928", "6bdc74ee5fccace64e18580c8ba8eb82", 69120),
	ENTRY0("if01_newcomer", "010928", "c618e4e185c85df8a1d4728537397a4d", 64000),
	ENTRY0("comp01", "010930", "82c631364cedf25ab4e78bbdc4034327", 169472),

	// IFComp 2002
	ENTRY0("if02_photograph", "020926", "8b5edf2a02123aadbd28bff37a324a95", 196608),
	ENTRY0("if02_moonlittower", "020927", "99126839757b62c48ddaaed723390f26", 138752),
	ENTRY0("if02_janitor", "020927", "61035b376a21cafc434d8a5657e8804b", 146944),
	ENTRY0("if02_janitor", "021210", "00750a8e1dc8c0e157be3465704b426e", 147456),
	ENTRY0("if02_tookiessong", "020927", "3e09bbcf92dfe6b72aab33ad9d0d572a", 187392),
	ENTRY0("if02_ftaegea", "020926", "d8a67ae37dbcb99d5edc18f318fd56c1", 343040),
	ENTRY0("if02_thetemple", "020925", "4d3bc8996e2af00aeac21d62c2278107", 107008),
	ENTRY0("if02_thetemple", "021118", "1592469a7f1f503789443d2f56f5113f", 107008),
	ENTRY0("if02_jane", "020927", "dec64545b4f4345b74a17537e65b4732", 171520),
	ENTRY0("if02_mythtale", "020830", "c3e89166105e986b6708ee470aa50b13", 212480),
	ENTRY0("if02_idthief", "020928", "eb85f09482a328a548dd96368eac50cc", 140288),
	ENTRY0("if02_rentaspy", "020923", "6bdaab7bdcac42ac5f150c3ed2600fdb", 143360),
	ENTRY0("if02_whenhelp", "020926", "07a8884764294f348784a615ad8ab69b", 285696),
	ENTRY0("if02_whenhelp", "030208", "2185bd75e8cc26ac7773effc300ec210", 286720),
	ENTRY0("if02_constraints", "020927", "fa88eccd76bd2e9e3108b70aafe537ba", 176128),
	ENTRY0("if02_hell0", "050929", "63130fb3a0f3c62217ec99c14ebb4ed5", 111616),
	ENTRY0("if02_bofh", "020927", "3da1806e505c38de29821f0e841248d3", 127488),
	ENTRY0("if02_bofh", "030202", "c43d7674558043729cf3916f7e1bfe4d", 101376),
	ENTRY0("if02_samuelgregor", "200108", "da6df678988e311dbdb9ecf3551ca482", 93696),
	ENTRY0("if02_screen", "020926", "74d0c7f54d1e9e387f91d88e889ec0bf", 83968),
	ENTRY0("if02_shamulet", "020926", "7bfdd75a3287c7b7874964e8e2480776", 74240),
	ENTRY0("if02_koan", "020904", "457eede8d32962bbf8e88938d3bf4685", 49152),
	ENTRY0("if02_ramandjona", "020926", "eea6527824140b1f6b0479e937ae3043", 64512),
	ENTRY0("comp02", "020928", "db8ba8f1596fb801021c99a92d111b38", 114176),

	// IFComp 2003
	ENTRY0("if03_slouchbedlam", "030925", "0f870671f3d7569d57eb79a8be9b4a28", 216576),
	ENTRY0("if03_risorg", "030925", "37c4b5b109c7005ae663e989024720d4", 443392),
	ENTRY0("if03_gourmet", "030927", "d10a75ea4d4485b24c513d9bfefdfe7f", 181760),
	ENTRY0("if03_gourmet", "031116/z5", "b7a2d1ce038c5ccc983fc3cec4cac682", 190464),
	ENTRY0("if03_gourmet", "031116/zblorb", "6a78f4aa50d962668a2bf7b0e788bafe", 360808),
	ENTRY0("if03_baluthar", "030926", "750377a64b3d502970e3bcf30adc9c5d", 121344),
	ENTRY0("if03_atomicheart", "030921", "8afc2962b86883919e89a4ea4db25516", 138752),
	ENTRY0("if03_papermoon", "030927", "d455488b4d5b59b4e3f2e6270c6c4e55", 130048),
	ENTRY0("if03_caffeination", "030914", "90a60bdac3c6df759d37eccaa4563181", 197632),
	ENTRY0("if03_adoo", "030731", "3f281cd7b79218c53c6aa2ad5886ce4b", 100352),
	ENTRY0("if03_domicile", "030928", "8a83149cb039ce6ebbb161b6007ca2bc", 149504),
	ENTRY0("if03_internaldocs", "030926", "7c1a2afee4f358c873b50fc504f59a96", 159232),
	ENTRY0("if03_noroom", "040117", "d3fd94d4560c15ff6bfb8327b56c97f4", 61952),
	ENTRY0("if03_lardo", "030830", "8741a6ccd5cb8123a0cd37c23f1721c9", 66048),
	ENTRY0("comp03", "030928", "55ee4ebe831b1f4dbc1c2865a6b59ef3", 112128),

	// IFComp 2004
	ENTRY0("if04_bluechairs", "040930", "a95e06ff11b68d1c6ab887e568cd0d60", 238080),
	ENTRY0("if04_bluechairs", "041229", "4ebedbd54fc2eae27fc7d867cadb37c3", 241152),
	ENTRY0("if04_devours", "040930", "646ce501f1a42de40fb48f945514e0c1", 155648),
	ENTRY0("if04_devours", "050325", "f9be89a5a26be53b52c08b6cea0ed3d1", 160768),
	ENTRY0("if04_stingwasp", "040622", "ca5589359f90fc20d9a6acb3312681f9", 239616),
	ENTRY0("if04_orion", "040930", "04b16a3b081bd49ee702025a7b862116", 202240),
	ENTRY0("if04_orion", "041123", "16995ce9a4abe4a4670471beda078611", 204288),
	ENTRY0("if04_mingsheng", "040907", "6a7b1758c832bf394f01882df288badd", 114176),
	ENTRY0("if04_splashdown", "040926", "518831b70285f9b6a2d57dac3aa55b93", 146944),
	ENTRY0("if04_gamlet", "040926", "2222246b74477d475064bab951fc78d6", 249856),
	ENTRY0("if04_gamlet", "041223", "c434581d222f89b85e67bea37aa702af", 260608),
	ENTRY0("if04_greatxavio", "040930", "7391523964a284b86eb0e3da3da63f33", 164352),
	ENTRY0("if04_greatxavio", "041204", "3725be7347d460cc3661b0268db55db5", 177664),
	ENTRY0("if04_geb", "040930", "85c3f43cfe5d7b86799efc2f596ef0aa", 163840),
	ENTRY0("if04_geb", "051028", "8b731db0410993b8421e06dd7f3e37b2", 165376),
	ENTRY0("if04_bigscoop", "040916", "2ca8203bd4ae6bfc4e76f5b430a8737d", 173056),
	ENTRY0("if04_identity", "040928", "1e459aceb4c0071c970dca5484c5617b", 126464),
	ENTRY0("if04_identity", "041121", "72fe1d48259be82d586bb4eb48ccaff9", 126976),
	ENTRY0("if04_murderaeroclb", "040921", "aff927b5be8d78a0900bf495f605395b", 108032),
	ENTRY0("if04_bellclap", "040930", "70be7c0e420f7236fae11c34589158db", 123904),
	ENTRY0("if04_typo", "040924", "827e420f98cfdc0b014203a53a11ff00", 96768),
	ENTRY0("if04_typo", "041119", "549875ea83b4230896fa7d3fa18ba0a3", 100864),
	ENTRY0("if04_blink", "040926", "7f8957455ed7187729239a73bd52b625", 88064),
	ENTRY0("if04_playtorn", "041001", "5c36cf0d222d564a5e10851b842beca3", 191488),
	ENTRY0("if04_order00", "040930", "3f365a5c640a60e8cbde6d71d3766f8b", 107520),
	ENTRY0("if04_bluesky_hf", "020408", "33502732a6b09eeacf61de7361307e10", 69120),
	ENTRY0("if04_stackoverflow", "040925", "5a6554d5af319b8b7e094da39ed6a3bb", 131072),
	ENTRY0("comp04", "041006", "8906d8be4439cfc8d2e34f7e1914462c", 116736),
	ENTRY0("comp04", "041007", "644fa97fda92e961969ca2525f7b947b", 116224),

	// IFComp 2005
	ENTRY0("if05_vespers", "050929", "a8ff87d7ab10b3e6825db32f093d1f08", 286720),
	ENTRY0("if05_vespers", "051128", "35377bb25ea1f3038377c8fb51795d6c", 309760),
	ENTRY0("if05_beyond_zcode", "050930", "7c40cfec00e949e0c104b7b415ecb04a", 320512),
	ENTRY0("if05_anewlife", "", "b767db9117f533064d3113e159ef03e8", 260608),
	ENTRY0("if05_toughbeans", "050930", "5fbe477520b8b9b9fd3873687d829460", 220160),
	ENTRY0("if05_colourpink", "050924", "1ed112b4d17b4a9f885f5cb7a9ead277", 268800),
	ENTRY0("if05_unforgotten", "050930", "94966251577ca5807102f1acab09750e", 249856),
	ENTRY0("if05_snatches", "050930", "c0d612653fc6a3f4bd01f1b7ed492058", 183808),
	ENTRY0("if05_vigilance", "051001", "93727f29c7a981ef006b4318e1231c3b", 182784),
	ENTRY0("if05_historyrepeat", "050930", "cd8fab96b41ee0949de6b769a635e2dc", 114176),
	ENTRY0("if05_soa", "050927", "49e31704b96971f0e51bfa3dcad39c0f", 115200),
	ENTRY0("if05_psycheslament", "050929", "67fd7b02ce32ef3da8ad54a5e483e382", 89088),
	ENTRY0("if05_onoptimism", "050930", "489ef2faedfca7255b3c5aa0fd71c462", 152576),
	ENTRY0("if05_neonnirvana", "050919", "3a39bb725d9414edf2a7a561d43706c2", 171008),
	ENTRY0("if05_swordofmalice", "050908", "9b43f402cc13e02f57a464ca6f843b32", 96256),
	ENTRY0("if05_drearylands", "050928", "25b26f262bdc8acca9e149d3ae2a0e4c", 97280),
	ENTRY0("if05_hellosword", "050929", "ccbda9f1be8df8e5f656db548a002ed1", 152576),
	ENTRY1("if05_hellosword", "060113", "7c5289ab479a5aace75a5518969fedf3", 153600, Common::EN_ANY),
	ENTRY0("if05_phantomcavern", "280727", "3dff9322c3f141ff17b47fc1e9d98929", 97792),
	ENTRY0("if05_futuregame", "", "94a44dc4c563b1f4468b13b32ef601f0", 10752),
	ENTRY0("comp05", "051001", "05e1dac27091bce08cc61a28ba71afa2", 116224),

	// IFComp 2006
	ENTRY0("if06_primrose", "060929", "d4a2ee947d42598eaa8f09c23c4ef2a4", 428544),
	ENTRY0("if06_moonshaped", "060929", "aa2bf7a42417827ac31ce7d8d3453a1f", 323584),
	ENTRY0("if06_wallpaper", "060928", "4e0b9814068b066ff8a7703f59902f93", 342912),
	ENTRY0("if06_wallpaper", "061117", "b25d80417e7b783bdc377302a3b53138", 344978),
	ENTRY0("if06_legion", "060929", "678b0e73b1fb8e3280b955ed587dfc98", 190464),
	ENTRY0("if06_madamspider", "060929", "6a48aed360accae28e7d21e702fa1b9e", 166400),
	ENTRY0("if06_mobius", "060929", "8535e1058ea7bd3ccbe098a40ddbdbd5", 425828),
	ENTRY0("if06_mobius", "061119", "ffafe18502f8c83789f904dde63f9938", 427364),
	ENTRY0("if06_gameproducer", "060928", "b4e0ff39edec9d291e78bfa95d6dcf22", 179200),
	ENTRY0("if06_starcity", "060928", "a7366438459f23661e8d178d8e690f49", 288256),
	ENTRY0("if06_strangegeo", "060929", "eead1528f100a3340167a909a9f5e97d", 308224),
	ENTRY0("if06_towerelephant", "060922", "74fa5a6e7d9b654ca2805edf6a7b0330", 281048),
	ENTRY0("if06_labyrinth", "060929", "abc2409764b3eee49a97ea90d58fc22d", 118272),
	ENTRY0("if06_cdst", "060929", "c91836c10f543523cd61a6b24020eb3d", 348152),
	ENTRY0("if06_bibleretold1", "100000", "e387804ec98ad477e6aa5911fbc74338", 191488),
	ENTRY0("if06_bibleretold1", "102283", "158f9c1818f7a80607a3b8281a9bc140", 169472),
	ENTRY0("if06_anothergodamn", "060927", "45a30649c871479e2060e95cc6bda3f4", 232448),
	ENTRY0("if06_fightorflight", "060929", "80db0fdf1d6b3951fc954b48de1e0c83", 270336),
	ENTRY0("if06_manalive1", "000002", "27e3e2afffe206fca0f0f78f97c2f175", 175616),
	ENTRY0("if06_hedge", "060929", "c7c17fffb437878b48990ed564960e5b", 216064),
	ENTRY0("if06_polendina", "060929", "e7e3e0ab767c330635feb9b009bc14ff", 111104),
	ENTRY0("if06_manalive2", "000002", "de9b6bae4432ef6585d06f65fc93a7ed", 149504),
	ENTRY0("if06_pathfinder", "060928", "f632512392d33c75fed4a0bc0f0c5fd5", 146432),
	ENTRY0("if06_apocalypclock", "060801", "fe06ae2560d153af32bb29e74813d436", 92672),
	ENTRY0("if06_sisyphus", "060915", "5af751766ddacab843bd190fe5a6fe46", 134144),
	ENTRY0("comp06", "061002", "8b72e6e68f62f53bb97646b108f31b0c", 149504),

	// IFComp 2007
	ENTRY0("if07_lostpig", "070917", "10dcc48e7312cc26eae89d1648af50c6", 317952),
	ENTRY0("if07_actofmurder", "070927", "2b8e594dc3d1d6b06153494bd47bb4db", 386048),
	ENTRY0("if07_bellwater", "070929", "67feb5edbb49ea993ca7a94f07556ce9", 348520),
	ENTRY0("if07_acrossstars", "070929", "4dd6a382a08b2865ad22a99cff8746e4", 492544),
	ENTRY0("if07_orevore", "070929", "aab0bd90d0a29605e8000590d0205c88", 410624),
	ENTRY0("if07_orevore", "071116", "fc4d75670412a50ffdc98e483adfaf5f", 415744),
	ENTRY0("if07_jackmills", "070923", "26689533940170a64107b27adce69cef", 422776),
	ENTRY0("if07_deadlinenchan", "07????", "0abf2e1124bf4da63a7efbd6cb239e57", 219648),
	ENTRY0("if07_fff", "070927", "bf7ad79002812c5b08ef0c98a3f73053", 99840),
	ENTRY0("if07_fff", "080121", "c29786d16b5498656e19f06eb3d58d94", 100864),
	ENTRY0("if07_wish", "070928", "c32139de52463c844ed063e0b0e6a227", 92160),
	ENTRY0("if07_packrat", "070928", "71ab8ff0a6ea40b11e5c0c4990358bc9", 120320),
	ENTRY0("if07_beneathtransf", "070927", "95b99d50b0c38cfd2907da023233ca82", 139776),
	ENTRY0("if07_theimmortal", "070928", "2fee83a968aa436d25be899d3bdcc02e", 293376),
	ENTRY0("if07_eduardseminar", "070926", "b02853e34c156c8feee8895c9f749490", 71168),
	ENTRY0("if07_pets", "070926", "c6a7ab54d9b9482cc459a9c975f48314", 146432),
	ENTRY0("comp07", "070930", "383572627817ce8cebd71080d5cc2245", 141312),

	// IFComp 2008
	ENTRY0("if08_violet", "080126", "df37a3ee028b439580ff3f138ff200cf", 598190),
	ENTRY0("if08_violet", "081123", "0941c8db96a7ef82ebf10fe6cdd1859e", 601262),
	ENTRY0("if08_afflicted", "080906", "20ecfc89ef0fca5a58d53f3bc6b7057e", 326656),
	ENTRY0("if08_piracy2", "080926", "e722ca46b86a2a86b979c791ac970646", 250880),
	ENTRY0("if08_snacktime", "080929/z8", "6cc88dd0f8c3fbea98fa16525a845b11", 339456),
	ENTRY0("if08_snacktime", "080929/zblorb", "35e2fb47b2a3ef13377be9556d673bf7", 1331682),
	ENTRY0("if08_openingnight", "080920", "56690c62b68ed95e6f5266c4bf5593de", 240128),
	ENTRY0("if08_escunderworld", "080922", "6e02c3844947612a8c6e3b0849568b2f", 108032),
	ENTRY0("if08_bishoes", "080912", "616d0a27ebeaddba73cbee9c7a8406d6", 73728),
	ENTRY0("if08_bishoes", "090513", "05c6dae3ea1896e756ce54bae26564ae", 85504),
	ENTRY0("if08_grief", "080929", "10ac0c11f6687f75d940c197dd79f88c", 287744),
	ENTRY0("if08_trein", "080718", "515fc480d1276c6a83e4af57fcb40670", 341618),
	ENTRY0("if08_redmoon", "080905", "cb8f73a536cfd474c548b38d03e08c65", 178176),
	ENTRY0("if08_draculascrypt", "080929", "e6be6ce15da3e50df3558c7c1a8a66b3", 238854),
	ENTRY0("if08_anachronist", "080927", "6e3816a87b03e9c82f92b87c2ceea361", 243092),
	ENTRY0("if08_whenmachsatt", "080927", "448e05c036c352e6def70a32ee68128a", 424960),
	ENTRY0("if08_thelucubrator", "080928", "110d123776a561ba8c14d72c8d534bf7", 191488),
	ENTRY0("if08_freedom", "080929", "6e37889524794022328582687754d173", 69120),
	ENTRY0("if08_riverside", "080903", "965457e0208af211943cb88707059bd0", 191488),
	ENTRY0("if08_thelighthouse", "080921", "8d29781a8de397439ba40645c9430803", 140800),
	ENTRY0("if08_absworstgame", "080614", "3c902f43a5393f41655df1f028057e65", 133632),
	ENTRY0("comp08", "081003", "0ea5f859a1aa09b42bd7229786160a5b", 144384),

	// IFComp 2009
	ENTRY0("if09_snowquest", "090792", "7a558ea909edca386a9f225a744a9cca", 1022942),
	ENTRY0("if09_snowquest", "100127", "2442363dd6f7f2ee6a6ca840ca9d9881", 1034206),
	ENTRY0("if09_duelspanned", "090925", "e7808064e6569e31be830b07be97cbb5", 379904),
	ENTRY0("if09_duelinsnow", "090930", "b669e90797eb4ce3adf37fba9f1447d8", 383488),
	ENTRY0("if09_interface", "090929", "afae3500034d118e0fd732ccd0d858f6", 293286),
	ENTRY0("if09_byzantinpersp", "090930", "dbb1fbf7245f6801d1a8619a95211340", 1690344),
	ENTRY0("if09_byzantinpersp", "091119", "61dcb1e79885f76f9f5f5c62ce2c7e8d", 1727208),
	ENTRY0("if09_condemned", "090930", "1a73a405571a1413481c47d63392ee3c", 257024),
	ENTRY0("if09_eruption", "090616", "a1136a2e48d40399bfa17200503dbd04", 113664),
	ENTRY0("if09_betatester", "090929", "157045ea9339e149dc4686ad229a27ef", 391168),
	ENTRY0("if09_spelunkquest", "090929", "81ce5c3f5aac8c3ac9a6c012cff4bda7", 297928),
	ENTRY0("if09_invisibleman", "090927", "fee503f8a8562bb7dc31ee606f23ca7d", 298496),
	ENTRY0("if09_invisibleman", "100130", "f42de996030f1caeb1695c6923db2249", 336896),
	ENTRY0("if09_thegrandquest", "090926", "bbb8e21c2c2f71ba29f74cd30940493e", 178176),
	ENTRY0("if09_thegrandquest", "091209", "3fd8b15a0282e9eb9ef642776bbd916e", 181760),
	ENTRY0("if09_starhunter", "090930", "60e33decd1e4a5c1f13298688d4eea39", 134656),
	ENTRY0("if09_gatoron", "090830", "99688a1be3791575a575c33fb98587b6", 122368),
	ENTRY0("if09_gatoron", "091116", "00ddc82b9268ea0968084e215057f21b", 123904),
	ENTRY0("if09_gleamingverb", "090930", "6971a0e1b91dffbe75b4efefe7f9540f", 148992),
	ENTRY0("if09_zorkburychaos", "080126", "dd1cb30fccde813a3301a324e5d6ac02", 163218),
	ENTRY0("comp09", "091001", "c937fadc20db485baa8f35e8d841d77d", 145408),

	// IFComp 2010
	ENTRY0("if10_deathofftc", "100926", "288dfb6066beba825f49d2c6080f9081", 311196),
	ENTRY0("if10_mite", "100930", "fa6eb58ea0db68211fa924fa1530d234", 186368),
	ENTRY0("if10_gloriousrevol", "100930", "123d7b02f338fe27f4b8f39ba2d15845", 854008),
	ENTRY0("if10_hummingbird", "100913", "4454cc4266f275a4d8a175aa21f5b0b5", 719972),
	ENTRY0("if10_hummingbird", "110101", "1b48ec7b38899b5747ad4a16be029889", 775268),
	ENTRY0("if10_warblersnest", "100930", "526aaf64cec2fb85b63b2d329befc086", 799282),
	ENTRY0("if10_asgard", "100928", "4b69b276e770d6ca6b0b7ff24c91bc13", 569460),
	ENTRY0("if10_penandpaint", "100930", "9a243dabd2227b95c49284dac825c946", 253440),
	ENTRY0("if10_bibleretold2", "130000", "9559cb4601fbfe195b36cf5970f7619c", 190976),
	ENTRY0("if10_heated", "100930", "148d0e98c403af802c3c8108a2e60d2d", 231424),
	ENTRY0("if10_ninjasfate", "100930", "a314c020fe002482c522c927d2012af1", 321024),
	ENTRY0("if10_eastgrovehill", "100930", "e627884c529415399883702fb5abf43c", 417266),
	ENTRY0("if10_quietevening", "100924", "14d27fc0f84c6eb9bbad0046938000db", 397964),
	ENTRY0("if10_chronicler0", "100930", "c16c8d34195c9f29c03629f20be7f800", 68608),
	ENTRY0("comp10", "101008", "113f48207ab14aed979b6034faf50959", 142336),

	// IFComp 2011
	ENTRY0("if11_santaland", "110913", "e0e8181bbfe04b6cf55c3903153d5ae2", 410112),
	ENTRY0("if11_canamicah", "110930", "309cde7bcb427088940ac701b7b18040", 363008),
	ENTRY0("if11_canamicah", "111013", "0fb93ce7d872ba0f0f0dc948d5d249a4", 364544),
	ENTRY0("if11_comedyerror", "111017", "9219e4a1ac32b67d508d661f0e00eb6c", 885682),
	ENTRY0("if11_tenthplague", "?????\?/v1", "358056bdf0d1df7dad3ebbfc46e4f4e0", 379176),
	ENTRY0("if11_tenthplague", "?????\?/v2", "7c8368a8fdbc8bace527efe5a46c9275", 378664),
	ENTRY0("if11_coldiron", "110909", "e917805948c99b02fdf1491542c2d8dc", 227328),
	ENTRY0("if11_coldiron", "111119", "82f24d7b6d66e76871c64ea47b4a6f5e", 230400),
	ENTRY0("if11_androawake", "110930", "1c38c3eba0c484048416cbff512b59ba", 512312),
	ENTRY0("if11_androawake", "111019", "7d82fa4cbcc943aefb3ddc65a38a0b7d", 527028),
	ENTRY0("if11_keepsake", "110906", "293a92fcd66e8351a7062ed954df5d9e", 462088),
	ENTRY0("if11_theguardian", "110929", "f707abfb7a69506d1d680a055abc21b3", 254976),
	ENTRY0("if11_theguardian", "111006", "e36bb34945160c5651d4539218bfad5e", 260096),
	ENTRY0("if11_lastdaysummer", "110930", "27a77f98d578a4ee5b0aadc6e81953e6", 372294),
	ENTRY0("if11_shipofwhimsy", "110930", "7cdedbedccd81556e59b4b8bad9e6123", 266274),
	ENTRY0("if11_pfrank", "110916", "f53fef42fe04091db9c901c41a926814", 398848),
	ENTRY0("if11_pfrank", "111103", "6e34d4ed01ef9548c960675e80de7f10", 351744),
	ENTRY0("if11_vestiges", "110930", "ce7506d0fe65344e502c857018f21b68", 265660),
	ENTRY0("if11_vestiges", "111005", "aadd8e8c1d72d67ce2edb9a260099113", 268230),
	ENTRY0("comp11", "111001", "b220acb7a5d268278bf4c665f5f693d7", 151552),

	// IFComp 2012
	ENTRY0("if12_eurydice", "120921", "a1e210d32afac8619fb8117c9826a4df", 792470),
	ENTRY0("if12_eurydice", "121206", "1f4112cdff2e5c924e54bfe06181ff49", 845206),
	ENTRY0("if12_guildedyouth", "12????", "75980e9b28e4c2b78e79f833e5829fe4", 322040),
	ENTRY0("if12_changes", "120930", "50d536eedd00ca0d566a369586f75db7", 450560),
	ENTRY0("if12_sundayafterno", "120929", "782342798ab1966ae14c740c2702e1e0", 327168),
	ENTRY0("if12_spiral", "120929", "5ae949fd69c4549d97402a3a7b3d6dfb", 347648),
	ENTRY0("if12_spiral", "121106", "c9af41ec32033424b58f08e5798fe86b", 348160),
	ENTRY0("if12_bodybargain", "120929", "3f6416efb106a73251b731bccd61e18c", 393344),
	ENTRY0("if12_bodybargain", "121021", "cf2fc83e9230a45317fdff11c3ee3fc5", 400000),
	ENTRY0("if12_fishbowl", "12???\?/v1", "d78a44277239ae80788a322e49109813", 392262),
	ENTRY0("if12_fishbowl", "12???\?/v2", "507c86a7456c7cfaf80fbaac1b716abc", 394822),
	ENTRY0("if12_summerland", "120930", "b57f5c37a65a09b7df789167a027624f", 392238),
	ENTRY0("if12_summerland", "121011", "fc9152c7eefa6f12d99f94a7916481eb", 426542),
	ENTRY0("if12_lunarbase1", "120929/z8", "8496b5fe70c738144e85031ea243f830", 311296),
	ENTRY0("if12_lunarbase1", "120929/zblorb", "f1fe0b501a7af58a305d85ce3738c1e6", 1371878),
	ENTRY0("if12_murphyslaw", "120929", "3ad61af558113bb14c67024bb7ec9bb6", 567000),
	ENTRY0("if12_testisready", "120929", "bcea384062b6cab396890e08e6e4d590", 510612),
	ENTRY0("if12_testisready", "121004", "f23aa28f71f47cdc0c5b024eb4270b16", 514196),
	ENTRY0("if12_castleadvent", "120315", "d748b6dd5c380a2f3438f8b957b17345", 240128),
	ENTRY0("if12_castleadvent", "121207", "7119fd8068a8e467e3e34c66cbee005b", 243200),
	ENTRY0("comp12", "121001", "a21991ba784590176bd0570647227fab", 149504),

	// IFComp 2013
	ENTRY0("if13_threediopolis", "130928", "942c317ad151633d5eff4a1fe5c0eb0e", 444252),
	ENTRY0("if13_threediopolis", "140817", "76929b10a1fddcbba4e704b9d7fecdc0", 584032),
	ENTRY0("if13_paperbagprinc", "130928", "f6b455548bfb5f40f9beeff98927c21d", 307712),
	ENTRY0("if13_further_zcode", "130929", "49f7ad5cd55692627c5535585368820e", 398556),
	ENTRY0("if13_9lives", "130817", "0fff213f0d61c01c281d7180e016711e", 337374),

	// IFComp 2014
	ENTRY0("if14_15minutes", "140929", "794388850a60ce63f181efb194519bee", 402308),
	ENTRY0("if14_15minutes", "141021", "8f99e176b2fd4387219b808515c75d0e", 402308),
	ENTRY0("if14_teaceremony", "140918", "f8b04f52fc3294539405893474413e16", 268352),
	ENTRY0("if14_enigma_sd", "140929", "3e2bfc7b4a45cb0bf3936cb7b164ee17", 414884),
	ENTRY0("if14_enigma_sd", "141020", "5f933e199785495bc884d003a4579b9b", 415396),
	ENTRY0("if14_blacklily", "140926", "16e3dee79b000c19e3f211af3bea9266", 188364),
	ENTRY0("if14_blacklily", "150318", "f74981accbff8eaa36e07bb175640ea2", 199124),
	ENTRY0("if14_tower", "140912", "9e4e1ffe01254b8f45f22c79d664cbb2", 390132),
	ENTRY0("if14_tower", "141012", "1c315beb9fe8447acb05a99b9fb7db9e", 390644),
	ENTRY0("if14_excelsior", "140924", "036fa721ea95f31da9a99b1bea4e1d99", 351232),

	// IFComp 2015
	ENTRY0("if15_finalexam", "150928", "bfdfd2d2daccc201e38f1234ccd6dc25", 165376),
	ENTRY0("if15_finalexam", "151101", "4e23b49e141f724f71e5e5bf0e1afc81", 177152),
	ENTRY0("if15_darkiss1", "150927", "5b0a30d18bdcca783d4348d58daa4bf4", 133120),
	ENTRY0("if15_lifeonmars", "150925", "69b0feca1bdd05f8d3240e6441941f98", 160256),
	ENTRY0("if15_gbvb", "150922", "8cca270442baf96ba01fae4d77de52c7", 378368),
	ENTRY0("if15_5minutes", "150929", "cd12b10aab062b1f1f13d113405d725f", 528346),
	ENTRY0("if15_5minutes", "151002", "de02600e870ea20f6666921d9bc6c61d", 528346),
	ENTRY0("if15_kingcrown", "150823", "76c9f8ffed03f81aa9d3d8e5aef77597", 380928),
	ENTRY0("if15_pitcondemned", "150927", "f8c86cd430512f4caae1ddc13e4e28a2", 468432),

	// IFComp 2016
	ENTRY0("if16_insidefacility", "160925", "3f2c1695de721218fdee36c34374e222", 463872),
	ENTRY0("if16_darkiss2", "160928", "405f407e2009e20317522375bf53e154", 179712),
	ENTRY0("if16_zigamus_zcode", "160928", "8d5ad5bd2aa48b2003b7c6bfc5803669", 89600),
	ENTRY0("if16_zigamus_zcode", "161005/z5", "f927bb50ece8682d1191ce1f56f41525", 93184),
	ENTRY0("if16_slickercity", "160928", "fa5141ef64408dbb2535b61355324284", 485840),
	ENTRY0("if16_slickercity", "161114", "d03d358208ba05db9e6e2068cb2cd149", 500176),
	ENTRY0("if16_youarestand", "160926", "8471b92b86d52dc9ce417adfac02fff7", 490456),
	ENTRY0("if16_toiletworld", "161009", "56b58c66b5ae7d9e7282a01924e24135", 418206),

	// IFComp 2017
	ENTRY0("if17_tuuli", "170929", "653637e9c257745d2ce77202fe14aa13", 590220),
	ENTRY0("if17_tuuli", "171008", "dbdcb2acfa69c6eb352343eb556f5d2b", 591756),
	ENTRY0("if17_tuuli", "180501/v1", "0ee525701efafda124902b1b26f84bdf", 611734),
	ENTRY0("if17_tuuli", "180501/v2", "0b7e2bb3f6887f30fb98b54230acf039", 611734),
	ENTRY0("if17_thecubecavern", "170928", "6afede8923f992bbb6849200b1672efb", 374824),
	ENTRY0("if17_thecubecavern", "171115", "c5f97f8958bf6f58705234f8612421e3", 300286),
	ENTRY0("if17_squirrel", "050102", "02c19fa852425c6593a4ef4ab383de4b", 112640),
	ENTRY0("if17_grue", "170810", "578ee64d95f854174a256e75e90f2306", 365056),
	ENTRY0("if17_richardmines", "170928", "f1e4b5f30a229e4b76c34bd852a66988", 674224),
	ENTRY0("if17_onewayout", "200829", "9d833823d5e9d49bf32f3caff9077d01", 169984),
	ENTRY0("if17_inevitamp", "170727", "8ef2c50928f72be5e131e43dff63754f", 433072),
	ENTRY0("if17_walkinthepark", "091120", "de3cbe750e75112d05d3a03d676cbb12", 123392),
	ENTRY0("if17_walkinthepark", "100213", "d116d7a9a5160fc336b36f7e8a077a42", 145408),

	// IFComp 2018
	ENTRY0("if18_tethered", "180926", "b458728de80b1510a29ed8f3f7ee9cef", 394086),
	ENTRY0("if18_tethered", "191125", "1e15d092a3729b88bb886625445a7f3f", 404064),

	// IFComp 2019
	ENTRY0("if19_out", "190929", "30a16ff0bf6ad33faa24c209edbe5bf8", 388608),
	ENTRY0("if19_out", "191112", "b297d0506353e2791d314399b52516f9", 392704),
	ENTRY0("if19_oldjim", "190928", "0e11c750f7490241a0e8c8286caf3b71", 413696),
	ENTRY0("if19_extromnivore", "190928", "e7a93522643ad1e1921fb13046ccc620", 428066),
	ENTRY0("if19_extromnivore", "190928", "e7a93522643ad1e1921fb13046ccc620", 428066),

	// IFComp 2020
	ENTRY0("if20_impossbottle", "200928", "ca6e77c892f34f602ea25277a8ac6a20", 759930),
	ENTRY0("if20_entangled", "200928", "773ad8d10e980531127a040679a4e3eb", 494080),
	ENTRY0("if20_drego", "400410", "8a983690928e36c8a77ed50bdbe98bd9", 132096),
	ENTRY0("if20_sheepcrossing", "200928", "22c2f8a7784272952125344f9e52ccd0", 411456),

	// IFComp 2021
	ENTRY0("if21_darkun", "210928", "1d86626167b93297f3d96cdf822d7594", 335512),
	ENTRY0("if21_codexsadistic", "210928", "956d15d106c982d3a50ef7069b07b84f", 581378),
	ENTRY0("if21_fourbyfourian", "210928", "593c4f5fb864cf5d43709f4028b37e15", 425190),
	ENTRY0("if21_bravebear", "210928", "f6b245a25d510c871671097a158cf215", 72192),

	// IFComp 2022
	ENTRY0("if22_campusinvader", "220926", "2067ac08742356f2e5f99ad417adab34", 82944),
	ENTRY0("if22_intosun", "220928", "16616858fd3c197b3c6f3e69df6b4bfa", 348160),
	ENTRY0("if22_lowkey", "220928", "59279a95782344dc98303a5e492664b8", 534510),
	ENTRY0("if22_zerochance", "220928", "fc11a5a21798584e3dd6441479dbc8d9", 259678),

	// IFComp 2023
	ENTRY0("if23_bbkk_zcode", "230928", "89f93705c53044276ef0da46f72cef18", 505188),
	ENTRY0("if23_h2", "231113", "31928f11d5ea6652a0dc59600df4d1fe", 237332),
	ENTRY0("if23_h2", "240106", "0e66f27d2cdc31ded2c8f2d0699c3272", 241744),
	ENTRY0("if23_thewitch", "230529", "9e02d2f514734c0553e55887091ac4cb", 167424),
	ENTRY0("if23_thewitch", "230924", "2b6e92d925964638837d4be0231091d2", 182272),

	// Spring IF Art Show 1999
	ENTRY0("ifas_tpold", "990417", "fdcd6216895f55cfc591f9d77c32bfed", 46080),

	// Summer IF Art Show 1999
	ENTRY0("ifas_statuedac", "990827", "16c2855284cbe8510e700d6455346326", 52736),
	ENTRY0("ifas_statuedac", "990903", "d6bbd50d6da814aa75206eb359dd75a1", 53248),
	ENTRY0("ifas_wheel", "001128", "7c582d943b29451caceb1140931a9b48", 54784),
	ENTRY0("ifas_wheel", "990824", "20d6b5c3931aded353868935e5433b85", 52224),

	// IF Art Show 2000
	ENTRY0("ifas_custard", "000314", "7a0d6ba2488c4998bb7c7f91342cfdcb", 290304),
	ENTRY0("ifas_galatea", "000324", "68e59da119f3c1a71c54710e24c8a243", 187392),
	ENTRY0("ifas_guitar", "000323", "20f80779fc85654e1831e5c2937bb1fa", 65024),
	ENTRY0("ifas_sparky", "000324", "ad58cd7b8b1313150419470570be9801", 73728),
	ENTRY0("ifas_cove", "000322", "732eb569ed375fe85fa40997e78bb350", 129536),
	ENTRY0("ifas_cove", "000525", "a84f448606c1b271a41b225244ef43a9", 130048),
	ENTRY0("ifas_statuette", "000324", "9be9383ef2a82c99d8404f4a51ac331a", 64512),
	ENTRY0("ifas_visitor", "000321", "9786e58864fdd7429c3218b07d023498", 56832),
	ENTRY0("ifas_words", "000225", "6cc34f6b4ed49a769d53a0ab48434112", 5632),

	// IF Art Show 2001
	ENTRY0("ifas_esgarden", "010417", "b77a37124c15e23b416d0ea9b74f9c86", 145408),
	ENTRY0("ifas_lagoon", "010406", "b7410c419427885c4b755e8cc555ab38", 81408),
	ENTRY0("ifas_ribbons", "010331", "5fb456f05ac161de3601dacfdf72075a", 109568),
	ENTRY0("ifas_ribbons", "010702", "7c45bffcbbd611a499e0803805fa87ea", 110080),

	// IF Art Show 2003
	ENTRY0("ifas_stopnight", "030425", "81b6332707329c64b5967e94238e84be", 136704),
	ENTRY0("ifas_queen", "030426", "dfd3a6dcb6c8a6b1becf6b69011ae3dd", 73728),
	ENTRY0("ifas_redeem", "030425", "f9ea63216ca6e20c2150ae3139e2aa61", 129536),

	// IF Art Show 2004
	ENTRY0("ifas_flametop", "040528", "58c67a57ac6f077a8d1a7be3fc3c73f6", 70144),
	ENTRY0("ifas_swanglass", "160702", "daf5f866bbaa6ec159456887e5945d29", 71680),
	ENTRY0("ifas_firetower", "040528", "fa8d9146120139c768a698e93f70812e", 270336),

	// IF Art Show 2007
	ENTRY0("ifas_rendition", "070407", "1a8fbde9c57754958882ddc4d5ed1bb3", 187392),
	ENTRY0("ifas_engine", "070518", "cb3434c4917d95b1ff3154acbf3a4dbc", 361578),
	ENTRY0("ifas_varronismuseu", "070518", "a1eeb2342999c2e3d394c9d718cc8b6f", 434176),
	ENTRY0("ifas_varronismuseu", "070602", "a6a8fdd36ad5bb4e390361738b86d8a9", 509148),

	// The Mystery House Taken Over project
	ENTRY0("mhto1mhp", "050315", "e2f93006c311f50932c974ba0b213a9c", 452096),
	ENTRY0("mhto2shiovitz", "050314", "aba2efcaa2394b315a461bc41a6bd04e", 107008),

	// Arcade Collection
	ENTRY0("ifa_rox", "001231", "0d473c955611eb9b6a07c8487bcb4914", 70656),
	ENTRY0("ifa_rox", "010105", "369f512671b6f317b033ea6ebe1c2f4e", 71168),
	ENTRY0("ifa_centipede", "001231", "fadf5f5ac0315307d881ad7d5a05f95c", 201216),
	ENTRY0("ifa_donkeykong", "001222", "c853bc1e4b4bc5c76fb152cde9508e68", 53248),
	ENTRY0("ifa_driver", "010107", "73b11a29d061beac540be8dd7b0c89b9", 57344),
	ENTRY0("ifa_galaxian", "010101", "e6e54bfd0a6abe32738c7059518209e4", 52736),
	ENTRY0("ifa_invaders", "001223", "aedd183fb142bb7501a2a5db9d4a59e9", 84480),
	ENTRY0("ifa_joust", "001231", "001e605d5292c4c2c4facf1816c0d8ce", 64000),
	ENTRY0("ifa_loderunner", "001222", "c3a2fe469222b8333154435656b83bce", 56320),
	ENTRY0("ifa_marble", "001125", "bc3349cd316b199d17d3fc804e10a780", 64000),
	ENTRY0("ifa_pacman", "001029", "45a2ba7ac1f00ad7c8d195d4c4a47ec1", 54784),

	// Casual Gameplay Design Competition #7
	ENTRY0("cgdc7_hoosegow", "100209", "d53ed83532308ca3ea01915e2e9ec05c", 1992422),
	ENTRY0("cgdc7_fragileshell", "110222", "7a9e1beed6ef25cc11ab5593e859d4cc", 547098),
	ENTRY0("cgdc7_dual", "100109", "dbee7f9e8c76d4810d2e8a99752bd9d0", 367526),
	ENTRY0("cgdc7_dual", "100225", "57f0e173be4258de1ce224e74172ea11", 430282),
	ENTRY0("cgdc7_partyfoul", "100208", "e890e53c4454315d4c7bc4ddcb4420f0", 507346),
	ENTRY0("cgdc7_roofed", "100201", "95f065bfc0203324e161127f9a182830", 292352),
	ENTRY0("cgdc7_roofed", "100223", "1e2cb89b420ba5c109169dd05a5dc011", 548342),
	ENTRY0("cgdc7_ka", "100212", "00401b846707deef82efb658be69dcc9", 321536),
	ENTRY0("cgdc7_monday", "100201", "fe45e5cde6bbb883ca8fd65eeafabfe3", 377856),
	ENTRY0("cgdc7_theusher", "100211", "d48b1b69a5beb07c62dfaf4fd2ba76a9", 354816),
	ENTRY0("cgdc7_intoopensky", "100208", "0d7771e5985fc17c09c78d881d529900", 438736),
	ENTRY0("cgdc7_luriddreams", "100211", "47d22333b821591bad87184c116541cf", 416256),
	ENTRY0("cgdc7_containment", "100206", "069d65c43c2c70064ad660a8475209c8", 221184),
	ENTRY0("cgdc7_thecube", "100130", "b37effeaf558c7d406db3efd2d4d68fe", 230400),
	ENTRY0("cgdc7_blueprint", "100208", "1f63cdc8d96f7c5c0408aaed9b9ca620", 183296),
	ENTRY0("cgdc7_critbreach", "100203", "e27b0ade8558173cd370d92f320be144", 267776),
	ENTRY0("cgdc7_manorwhitby", "100201", "8a965849cb2ec58cc7c89df0e046206f", 370084),
	ENTRY0("cgdc7_heavenly", "091213", "e5c9c808c47510348bdf9bcb14ef1f9d", 284672),
	ENTRY0("cgdc7_expecttodie", "100209", "242d660192fb79236647842f03ba1c12", 192000),
	ENTRY0("cgdc7_virtuality", "100131", "a68fe31bf3cce9b3dea7da0d733f45b0", 397244),
	ENTRY0("cgdc7_virtuality", "100203", "6690eaf9869b53c49696030c072728bc", 397756),
	ENTRY0("cgdc7_dramaqueen", "100130", "1cd2a0e48662a5bed209366aaad6c53e", 356938),
	ENTRY0("cgdc7_terminal", "100205", "544d65a88ac7965f5334cae7078f9cc8", 340930),
	ENTRY0("cgdc7_couchofdoom", "100210", "91e50fdfde64789bc6be1a786224c850", 259584),
	ENTRY0("cgdc7_basictrain", "090402", "d14365fc61e36672e781e8bbc37c6051", 272320),
	ENTRY0("cgdc7_paint", "100209", "fbe7f5356d171e92f4011b33dff06625", 205312),
	ENTRY0("cgdc7_goldenshadow", "100203", "a99de59ca4478ccc758aabc357931a36", 244682),
	ENTRY0("cgdc7_escapeindark", "100203", "acb5cba872017ed4acf6a97cca18e12c", 165376),
	ENTRY0("cgdc7_escapefict", "100203", "4f61e28d71955057ff7f7660f6557740", 273860),
	ENTRY0("cgdc7_zeroeth", "100221", "bfa89880cfe414dfd743c36c3186207d", 194048),
	ENTRY0("cgdc7_openfield", "100131", "e40d2c10d464ff2dfbf4090d2568bb9d", 247234),
	ENTRY0("cgdc7_survive", "100208", "4cfdfa675947460bb5faf56d5d96bc06", 86016),
	ENTRY0("cgdc7_zegro", "021310", "a113089b3256dbe600bd43e3616f1f40", 176640),

	// Chicken Competition
	ENTRY0("cc_chknmt", "980609", "a7461a48ec077946965496712ef3119b", 55296),
	ENTRY0("cc_behavior", "980611", "72b4c3eddf40183fa1c1cfdbdf7e99b7", 50688),
	ENTRY0("cc_chicken", "980616", "4c857164047f2babed601c1d351efda0", 48128),
	ENTRY0("cc_chickenegg", "980608", "f7378a3db38fd5cc1bca24fee8893cd8", 60928),
	ENTRY0("cc_chickenunder", "980616", "ad61d9b0bcff8c70c2ab2a23ec2ac62e", 52736),
	ENTRY0("cc_chickenunder", "980728", "6655312edc32a1115b3e66f6c50293f9", 52736),
	ENTRY0("cc_chickendist", "980615", "0ca941817c6e35af59144616d4014f87", 57856),
	ENTRY0("cc_freerange", "980614", "df1fd1ce15cc87f18a4868db7f749c6c", 49152),
	ENTRY0("cc_lessonchicken", "980616", "b6579c2706c3b01a7e9aef7b4bf468ac", 63488),
	ENTRY0("cc_aboutchicken", "980610", "6b3bbbee0344354171537b8ec6f9569f", 83456),
	ENTRY0("cc_orpington", "980611", "c1009259f7d0bf69a29a41b3000f84a3", 53248),
	ENTRY0("cc_pollocamino", "980615", "0608138a4687ef994cc9a6b2577c88fb", 53248),
	ENTRY0("cc_saied", "980608", "e9fc656cee21ac488bc72f17dbef0901", 56832),
	ENTRY0("cc_xchicken", "980614", "e0da060d09f8ec64223e7ee07c92cadf", 60928),

	// Commodore 32 Z-Machine Competition
	ENTRY0("c32_endgame", "041130", "e7bb3a957c8d80e5ab014630de53b046", 30720),
	ENTRY0("c32_turningpoint", "041127", "54a0aa0762e2a8cc8db3ef2498ab8153", 32768),
	ENTRY0("c32_amusementpark", "041201", "7f1c810417318d6a8d318335b14a95ee", 32768),
	ENTRY0("c32_downtowntrain", "041128", "95cd2cf31b0d510a5253ddd510ed8866", 32768),
	ENTRY0("c32_zombiescl", "041201", "ec35c2ba47572301423613bc9b5faa39", 29184),
	ENTRY0("c32_paparazzi", "041130", "7dd95b7f52e93f1ea3684f22d70ac645", 32256),
	ENTRY0("c32", "041130", "02f050055728d8081fe5afee18898c26", 27648),

	// Cover Stories Game-Jam
	ENTRY0("cs_antifascista", "120612", "ea02a0f92f7c2c40fa39abd48fd1ccf2", 406208),
	ENTRY0("cs_leaves", "120610", "d8f48c9a46689a07106dccdc165a3119", 523862),
	ENTRY0("cs_legendmisshat", "120610", "c12e8daaff707325e446ef0b557ab044", 586960),
	ENTRY0("cs_monkeybusiness", "120610", "da1805563d40cbed5d275e8438f108a4", 416856),
	ENTRY0("cs_offering", "120610", "6dad4b91d08dc6536acd1f9f7687beac", 670832),
	ENTRY0("cs_sloth", "120604", "55e327dcf77ca6d2217d008e71fc9613", 1365970),
	ENTRY0("cs_subtropservroom", "120611", "79a7fc6f52c2f6dfe5eb606f924f3b74", 797652),

	// Dinosaur Mini-Competition
	ENTRY0("dino_rowr", "000301", "41fce26b11af40f57cdedfe5e926eaf2", 52736),
	ENTRY0("dino_dinoroad", "000313", "5f8d37555c320a9c1a58c0a896465e34", 58368),
	ENTRY0("dino_d2d", "200319", "19240f96a2fde80767a170571022b55e", 50176),
	ENTRY0("dino_dinohunt", "00319", "e169354dcc5878d0f28a87a7b32d5a95", 60416),
	ENTRY0("dino_stegosaur", "000328", "573ff953c1d6d498652b0522a7b4e967", 64000),
	ENTRY0("dino_adno", "000331", "1bcdfe28b95349c2c0e645d918bdde17", 55808),
	ENTRY0("dino_olddog", "000331", "e88627fba490bd9a5897742d128392ee", 101888),
	ENTRY0("dino_revenger", "000331", "e3eeab3757cf4b23583433f91ab42c55", 88064),
	ENTRY0("dino_revenger", "000403", "1f658ebcb395be17154e732cc6306dc9", 88064),
	ENTRY0("dino_dinnertime", "000331", "7da3b7d01c9d0098a0049968ed3ffae7", 82944),
	ENTRY0("dino_appoint", "000331", "3b0ab1f1eda430f5334778d4e6d0b724", 90624),

	// Dragon Mini-Competition
	ENTRY0("dr_atbottom", "000414", "1f2fd5449f77ffe4bd356938a9a16eb7", 61952),
	ENTRY0("dr_damsel", "000414", "cad5018ee047e9ef7fe499ac25c6c1d2", 58880),
	ENTRY0("dr_dragongeorge", "000416", "81ce3ff455980543d2fe26c1fd7eb85e", 120320),
	ENTRY0("dr_atdragon", "000409", "7f005d88386c0b57531fff97c6b9534d", 80896),
	ENTRY0("dr_herebe", "000314", "dd605eff65c76194fe33ad8ce95def50", 51712),
	ENTRY0("dr_lastlaugh", "000414", "798b6d7697d27b83d2362e7cd2874f78", 83968),

	// Ectocomp 2011
	ENTRY0("ec11_bloodless", "111012", "9e3c069d69a6e240f161c16c317010fb", 184320),
	ENTRY0("ec11_bloodless", "111217", "bf633d44d9ae34fb006aa1c135d24fe2", 188928),
	ENTRY0("ec11_blue", "111030", "429a5d229c4d6fbf808ce8370ceb8128", 260028),
	ENTRY0("ec11_dashslapney", "111001", "35215328fcc344c2dd932d4d1a09f15f", 281916),

	// Ectocomp 2012
	ENTRY0("ec12_ghosternight", "121029", "c295d2c2dfe5f3feee2b833a32a2400d", 236544),
	ENTRY0("ec12_littlegirls", "121031", "d96c3cd0a7609166f70e67cc1ac5f055", 232960),
	ENTRY0("ec12_parasites", "121021", "bbdb3ff64bea5ab24b0c7dcc68f1f90a", 164864),
	ENTRY0("ec12_parasites", "130303", "55eadf4edd9aeeb680841bb6cf112ade", 168448),

	// Ectocomp 2013
	ENTRY0("ec13_horpyr_zcode", "131029", "d39833192d0c8e0c3872423a740683a5", 321958),
	ENTRY0("ec13_horpyr_zcode", "131201", "b3dfa0f543f7ca3343ca62cc5ec7847f", 365004),
	ENTRY0("ec13_headleshaples", "131030", "046e0b958f9900a8733d04e52ab6631b", 262594),
	ENTRY0("ec13_faithfulcomp", "131030", "a9be7589b843ef43b326cfe477ade7d4", 311716),
	ENTRY0("ec13_faithfulcomp", "131031", "eea6302f434a68e7951ced18b385e2a2", 311716),
	ENTRY0("ec13_deadpavane", "131031", "18e8bdfbbc704dbc582305b868a1216f", 269232),
	ENTRY0("ec13_deadpavane", "131204", "cf0f923ae3b079c29608770aecdefdf1", 269270),
	ENTRY0("ec13_icehouse", "131029", "c85bfbce5eaa4e0db3621c5e6515e5a5", 214528),
	ENTRY0("ec13_blackness", "131026/z5", "7b8ea77b533399adf669015b013ba4fb", 169472),
	ENTRY0("ec13_blackness", "131026/zblorb", "21d1829ae6bd6ee3b39bff021b89a22e", 276574),
	ENTRY0("ec13_argument", "131031", "9fe440e44c9c37b68715afb42bb51043", 324566),
	ENTRY0("ec13_fishdreams", "131030", "d9022595cff5bb6b3385680f09adfcf1", 259532),
	ENTRY0("ec13_cursedeagle", "131030", "189a5f517d2e39046f5e9cc5c688c8fd", 314326),
	ENTRY0("ec13_cenriccurse", "131022", "f0496ff6c8bc01931f6034373fbf1d44", 281730),
	ENTRY0("ec13_cenriccurse", "131218", "4fa3db6430e3a54f88962a75857ebb2f", 289096),
	ENTRY0("ec13_nessaslasher", "131013", "b9e68911fc59d06880ef56775aa4bc65", 167424),
	ENTRY0("ec13_cratercreek", "131018", "44be72462d07e4faa347daf1b2b06955", 276388),
	ENTRY0("ec13_hillofsouls", "131026", "ebffb813147946e416623c17e7b3ab86", 442546),
	ENTRY0("ec13_wisp", "131030", "dc4b6d8674622873f5d6fb26aee1a227", 307598),

	// Ectocomp 2014
	ENTRY0("ec14_weddingday", "141031", "bde9af024968d3d70615555a41e806fc", 319424),
	ENTRY0("ec14_limeergot", "141016", "d7f102df1f7ed0d0cc646190706dd331", 169984),
	ENTRY0("ec14_candyrushsaga", "141031", "d6a271282be8d91a073e9f86ea3d4b79", 263626),
	ENTRY0("ec14_flywall", "141023", "f263225a38d2eae685e5bb569f6c50b7", 463332),
	ENTRY0("ec14_monstermaker", "150817", "f0565d38e01ae2ec048b5f3b8ceb4de4", 285696),

	// Ectocomp 2015 - La Petite Mort
	ENTRY0("ec15_openthatvein", "15????", "e24844c7028262c8117c2ef07138c6f7", 449548),
	ENTRY0("ec15_oldhangover", "151030", "e53693405ea9f9e4a7f6bf70cb14ec87", 334848),
	ENTRY0("ec15_heezypark", "151030", "fd714eb77e36ec59083e922c52d61722", 316860),
	ENTRY0("ec15_storyshinoboo", "151030", "78988da1e65fa38328b01c569d585321", 202240),
	ENTRY0("ec15_storyshinoboo", "151103", "5722af4452a4a3463e6b489c9f836dd8", 203264),
	ENTRY0("ec15_ghostship", "151009", "f76aba5c59c6376edcfdccd76cb9e590", 634812),
	ENTRY0("ec15_physiognomist", "151030", "f050387c1eb46061c7aacc13c1a366ab", 252890),

	// Ectocomp 2015 - Le Grand Guignol
	ENTRY0("ec15_ninelives", "151030", "e7d97c11b47c0b59d2093700ad936264", 464836),

	// Ectocomp 2016 - La Petite Mort
	ENTRY0("ec16_bonesaw", "161027", "bbf02174af7d8891d435a37c1cc9b058", 342016),
	ENTRY0("ec16_checkerhaunt", "161031", "d8ddc04e13caf50e5bd7d3f8931e8cda", 305616),
	ENTRY0("ec16_lightdarkness", "161030", "03df7e40dca78a7e2aee74b5049136bb", 258516),

	// Ectocomp 2017 - La Petite Mort
	ENTRY0("ec17_primer", "171025", "744648014bb7291f42eae31a79410d59", 255430),
	ENTRY0("ec17_dreamcorrupt", "171029", "1f4f956de3054621f928204c0e6b0d6d", 429160),
	ENTRY0("ec17_uxmulbrufyuz", "171029", "85ee9e7844c6be74b26c1707255f110a", 236760),
	ENTRY0("ec17_civilmimic", "171031", "3e56399f54b6e0ae2c85f72ef3cc23a8", 305086),

	// Ectocomp 2018 - La Petite Mort
	ENTRY0("ec18_moongoon", "181029", "55fc4e3ccf26bdf636316220a77ffb10", 433662),
	ENTRY0("ec18_wakeup_zcode", "181006/z8", "c0ea31ea66528b08ef090813267c417e", 337920),
	ENTRY0("ec18_whoahcubswoe", "181030", "fb9e91fc58040e4ec2871b35e3194746", 244006),

	// Ectocomp 2018 - Le Grand Guignol
	ENTRY0("ec18_plshel_zcode", "180923", "d9a47f9750f47d9c08fdd800e450cb89", 342016),
	ENTRY0("ec18_walkamongus", "181031", "5b6b726c28b897d299d4add5fcd3943e", 2292016),
	ENTRY0("ec18_walkamongus", "181202/z8", "5f92efa407b998d60ad0bb636f0d729d", 367104),
	ENTRY0("ec18_walkamongus", "181202/zblorb", "0a79a4dec57d44b6ee09fec8e868ea8e", 780152),

	// Ectocomp 2019 - La Petite Mort
	ENTRY0("ec19_quitequeer", "191030", "d09302a713495016a0376fc8ce7d7237", 329686),

	// Ectocomp 2019 - Le Grand Guignol
	ENTRY0("ec19_onceuponwint", "191030", "0df83d2e0c56c3afabfdfacb22a9b34d", 613468),

	// Ectocomp 2020 - La Petite Mort
	ENTRY0("ec20_fracture", "201031/z8", "84d04db287befa9c46fbc94591b31bb2", 345600),
	ENTRY0("ec20_fracture", "201031/zblorb", "23744075265e1a0fae0453df0bb14c34", 363830),

	// Ectocomp 2021 - La Petite Mort
	ENTRY0("ec21_wearyeerie", "211030", "f8e693a21ccb457a9c5b58251b635060", 188824),
	ENTRY0("ec21_psyops", "211030", "fac6e93acb93bdd6420f49918f20267c", 374812),

	// Ectocomp 2022 - La Petite Mort
	ENTRY0("ec22_enigmamanor", "221121", "43b8411bc971e5aa83801099f7826044", 769044),
	ENTRY0("ec22_thosedare", "221212", "06ad94a70731550a4aa920e660a9d805", 308452),

	// Ectocomp 2022 - Le Grand Guignol
	ENTRY0("ec22_civil", "221122", "6dff9f0c6b642590a41daed3642a6ae7", 339172),
	ENTRY0("ec22_civil", "221212", "b9384e68587e6bec4890d62c40f47a5d", 339684),

	// Ectocomp 2023 - La Petite Mort
	ENTRY0("ec23_tallertech", "231031", "f8063c9804dca335ce5c9784f21d6f6c", 368666),

	// EnvComp
	ENTRY0("ec_laseine", "090320", "1073245f6178172de42f73208b8cfcc2", 167424),

	// GameplayComp 2009
	ENTRY0("gc_fantafinaleiv", "090914", "f0f2cf8c1be71de23cd08d7216f5feca", 160768),

	// Ghost Town Redux
	ENTRY0("gtr_ghosttowntlt", "070725", "f430038611ee776ab82af369d81b6bb6", 171412),

	// HighlandComp
	ENTRY0("hc_highland", "010225", "3257620cd54c43c842de8f6a398d358b", 95744),
	ENTRY0("hc_roots", "010227", "131dd528660c2eeeb19257ad1b43258d", 71168),

	// IF Fan Fest 1998
	ENTRY0("ff_ite2", "980726", "b7f93aa7570757b814b030d0d5e1759f", 68608),

	// 24 Hours of Inform Competition
	ENTRY0("24h_aesthetic", "310103", "dd1296e1c1ce31ea4321bad5b7a9959c", 107520),
	ENTRY0("24h_dastardly", "060122", "79b5c728180c097c3dd01c25ea2663aa", 79872),
	ENTRY0("24h_defra", "022228", "2866e4e634c136009303b8bce0dd0952", 98304),
	ENTRY0("24h_borgaris", "030204", "2800f77bb7d1db2c2f94d5c4360e9348", 147968),
	ENTRY0("24h_ghostship", "030505", "38b15466da3c4c20c6947c9974649e68", 101888),

	// IFBeginnersComp 2008
	ENTRY0("ifb_connect", "080219", "4c81a7fc404b50d36a47947c10fb7814", 221184),
	ENTRY0("ifb_limelight", "080215", "4fe735af2e7f328106d9674d8c29bd9e", 214656),

	// Imaginary Games From Imaginary Universes Jam
	ENTRY0("ig_garbage", "160207", "1d1d0dec9fc382dc4e3c303e37e56afe", 425936),

	// IntroComp 2002
	ENTRY0("ic02_waterhouse", "020319", "b36b7a0db435e1f2b28514f87bf1a756", 141824),
	ENTRY0("ic02_timetrap", "020319", "dc9d9b5c4af9d6a763465caf2770b22b", 141824),

	// IntroComp 2003
	ENTRY0("ic03_statue", "030321", "17652647e855fa7f94c7be0987aca13b", 276992),
	ENTRY0("ic03_realend", "030318", "cb8aa32666ba4e48cd191537e28a79f4", 114176),
	ENTRY0("ic03_agency", "030320", "1fd608f827bae4d5ff3cbfb913293d3c", 70144),
	ENTRY0("ic03_harlequin", "030321", "0aaeed0851aaa0338ed653ec3e485eb7", 92160),
	ENTRY0("ic03_harringthouse", "030321", "83e4af82d7077019597f6014b7c05b94", 124928),

	// IntroComp 2004
	ENTRY0("ic04_jbbrwkyi", "040714", "dfbf529105d6eff2610accf0e4e1e9ea", 119296),
	ENTRY0("ic04_auden", "040707", "e9468a39f7dc696db24c057819f8bfbd", 67072),
	ENTRY0("ic04_passenger", "040718", "8a8f0ba481aafd3f30f2650290f61ea7", 94720),
	ENTRY0("ic04_cross", "040718", "9de8aa41ff0ead846e55fb2d7dc609c2", 65024),
	ENTRY0("ic04_runes", "040706", "41fe4fc08bbbd803baf1b9edee61e095", 87040),

	// IntroComp 2005
	ENTRY0("ic05_deadsville", "072305", "f6666723bb7dcda541c2d591acc7a1fd", 119808),
	ENTRY0("ic05_deadsville", "081105", "58ddf8bfbd4a78aa722127193f6f54f5", 120320),
	ENTRY0("ic05_wscholars", "071107", "03d438cfb4920cc831da090e0c150f2a", 587032),
	ENTRY0("ic05_stale", "050723", "d6db29b6290672b7faa620f8ecce876f", 87552),
	ENTRY0("ic05_stale", "060113", "a813af22024cbc8f6113eb293c6d0287", 123904),
	ENTRY0("ic05_hobbit", "050621", "47f280042669ec7330e9378266c33558", 100352),
	ENTRY0("ic05_somewhen", "050705", "8796d65ae9dd1c540d5ecf24760f4ee6", 75264),

	// IntroComp 2006
	ENTRY0("ic06_southerngoth", "060702", "5c5fab8b67a94080e52d0f553a72410c", 192506),
	ENTRY0("ic06_childsplay", "060714", "fe8b789b87f715d63f678608db42ae92", 310628),
	ENTRY0("ic06_artdeception", "060713", "804cd3fd69b98c0f03fd39e0a2d4de12", 156160),
	ENTRY0("ic06_mechs", "060716", "4d7d7204634951bb6f999f03894f2148", 89600),
	ENTRY0("ic06_sabotage", "060512", "b73c47a5c80cd0ad14adf7393d50ae7e", 107008),
	ENTRY0("ic06_sabotage", "070107", "5ea8fbaf07b5704b79b412dbe06ac256", 108544),
	ENTRY0("ic06_ufury", "060716", "953ab8c4066d4362247bf444b9b36fe3", 125952),

	// IntroComp 2007
	ENTRY0("ic07_shredspatches", "070803", "a30d837fe5316756fe423afb198fa562", 276992),
	ENTRY0("ic07_jacobst", "070804", "8755b906f2732a0739c7a63b557294a4", 210944),
	ENTRY0("ic07_jackbox", "070731", "8569084f577156621c43d8b3dcf328e6", 184726),
	ENTRY0("ic07_trainstopping", "070804", "a24d31b0fffb5503cdca093007d84a4e", 179712),

	// IntroComp 2008
	ENTRY0("ic08_stormcellar", "080818", "f9ec8dd06ef3cd6b76d37083d6c20cad", 253338),
	ENTRY0("ic08_stormcellar", "081022", "c38434a33b97a9616956ce48a7d69404", 384600),
	ENTRY0("ic08_bedtimestory", "080819", "defc39612662db16cc20f905a577273f", 286720),
	ENTRY0("ic08_fiendishzoo", "080820", "31992ae38abe111e24f01cc299ec639c", 156562),
	ENTRY0("ic08_ninetenths", "080818", "b7972406a4065706b5a0a9bd09cfd228", 224768),

	// IntroComp 2009
	ENTRY0("ic09_gossip", "090828", "0de83be147d1b6de3ec0a02b9ffc0a8a", 148480),
	ENTRY0("ic09_gossip", "090918", "5b0be1c76155882fb0d1d9eac3339255", 148480),
	ENTRY0("ic09_obituary", "090829", "c3ec0a2225c71e901a6f52e4f56955bd", 219648),
	ENTRY0("ic09_selves", "090828", "899a425d9327b8252934ca73d78e86d4", 301502),

	// IntroComp 2010
	ENTRY0("ic10_touristtrap", "100705", "4166fc534ae96607232ee13bc9898eb2", 277504),
	ENTRY0("ic10_moratori", "100704", "34bfe10267a23efaf78b3167ad3ca004", 380058),
	ENTRY0("ic10_plan6", "100630", "68ac09ff777cff2f9078c4aa39acd98c", 85504),
	ENTRY0("ic10_waker", "100704", "5c814efafb5da7811c05b138ea5a896e", 506854),

	// IntroComp 2011
	ENTRY0("ic11_sfiction", "110630", "242c7a127c04f8a0e01d183dc9acb4b3", 98816),
	ENTRY0("ic11_sfiction", "120706", "662d83be00d1a832e37bdedc565c1a71", 275968),
	ENTRY0("ic11_bender", "110611", "1140eb6ddef1ef25d8c14868bcd342cd", 333512),
	ENTRY0("ic11_despondenidx", "110629", "ad62e7b6f5565ea6ba41057a357c0e0f", 262608),
	ENTRY0("ic11_parthenon", "110622", "f663f08ce250614ddcf6cbf1b5bc3580", 261576),
	ENTRY0("ic11_stallingtime", "110630", "939ae90866da68944a48c4e277c4c9e5", 326752),

	// IntroComp 2013
	ENTRY0("ic13_chickensexer", "130716", "f3f6d761378d1de0176624d6df85de3e", 370648),

	// IntroComp 2014
	ENTRY0("ic14_talesoulthief", "140720", "cfdeeacc120c9a2bc5702f996e59d645", 352720),

	// IntroComp 2016
	ENTRY0("ic16_grubbyville", "160805", "bfb73ae5770b3794bded32092c14cb64", 284218),

	// IntroComp 2018
	ENTRY0("ic18_napier", "180727", "ad181efef44aed7ab1162cc4f193e5ec", 921026),

	// IntroComp 2019
	ENTRY0("ic19_homeland", "190728", "a95092c8e4d929e5462f7e99896107c1", 99328),

	// IntroComp 2020
	ENTRY0("ic20_navigatio", "200613", "0a841691a8b7d704abf7b125b76e1b39", 457746),

	// IF Library Competition 2003
	ENTRY0("lc_serve", "030315", "77e8343a373c442074e6f832d904818d", 139264),
	ENTRY0("lc_trappedschool", "020920", "ba57356b20fa4ca3fd9bd1fd9c390abc", 135680),

	// IF Logic Puzzle Mini-Competition
	ENTRY0("lp_logicpuz", "030211", "6ec37cc157a28f2c5dafc0d0f80d30ba", 95232),
	ENTRY0("lp_traffic", "041303", "7c456f06a8b544b7e39f6504b0fdbd4b", 58880),

	// LOTECH Competition 2001
	ENTRY0("lo01_doghouse", "050630", "8cf8ceac10efec821d906ed6d618fd45", 66048),

	// LOTECH Competition 2005
	ENTRY0("lo05_stormy", "010415", "99d8cc39c86128559467cdcf4e030126", 124928),
	ENTRY0("lo05_knapsack", "010415", "9b82f92549f74d9f406cc4de91ad19a5", 15872),

	// Manos Mini-Competition
	ENTRY0("ma_manoz", "000506", "e1c2b5e831c7b65a7c84b4e6795a8234", 64512),

	// MCDream Competition
	ENTRY0("mc_dreadwine", "060723", "55734ff823a29f039b093e460ea2a071", 473876),
	ENTRY0("mc_nofamous", "060816", "2a5cfaa1b541ea238f4d29d4ad6f97cf", 227328),
	ENTRY0("mc_retreat", "060814", "72b81c21592c4f6fef1452fb7fdd3c3b", 213516),

	// Metafilter IF Competition 2009
	ENTRY0("me09_bunt", "090402", "67561ed3290dfcf350d3199d1c68e1f3", 77824),
	ENTRY0("me09_downout", "090402", "7cf5f82737e9477e0c4a203d92763187", 327010),
	ENTRY0("me09_underpig", "090413", "9b0b2394a0c9ccc05f0f9ec389cc1a30", 154112),
	ENTRY0("me09_ericsbender", "090331", "998bccbb0481ce74850597b4e88ba9b5", 269282),
	ENTRY0("me09_jeffrey", "090410", "0292a9b7ac4f4d6feac8bee093deb31b", 271872),
	ENTRY0("me09_burndown", "090404", "95d1d1650305e213317b13c1128b3688", 157556),
	ENTRY0("me09_rockrider", "090401", "4b38c9071633db404d19462196c729f6", 241506),

	// Metafilter IF Competition 2010
	ENTRY0("me10_greatrent", "100531", "9e68a7ff9194e38fcdeb0d5ce922f04a", 255964),

	// IF Mini-Competition 1998
	ENTRY0("mc98_bloodline", "980619", "574f0caa07476e82bcc6967e9ec70171", 58880),
	ENTRY0("mc98_inherit", "980622", "99cd60a8a1b0b02bffffd5ceba8f1dce", 49152),
	ENTRY0("mc98_sumbunny", "980623", "a13fa568408aa300adbce0b2d33ffaa0", 108032),

	// Minigames Minicomp
	ENTRY0("mm_hamhall", "030607", "565a327a90408ad167ea16dbfd683419", 30720),
	ENTRY0("mm_insider", "03????", "e38ce038ffd97c33cf9887cfef29d8f0", 42172),

	// 9th Annual New Year's Mincomp
	ENTRY0("ny09_egyptianwalk", "151231", "ef80dd50c700becf2b4b254ce3ad3c9f", 334848),

	// 11th Annual New Year's Mincomp
	ENTRY0("ny11_halflife", "180102", "982d29c2516cf97ed2b9dd2cdd0d7ffb", 489416),

	// No-Inventory-Competition 1999
	ENTRY0("ni_annoyedundead", "990220", "745496c251191b520688bdec23495b35", 64000),
	ENTRY0("ni_camping", "990127", "11780d271a71c52caebf73332df83be2", 71168),

	// One Room Game Competition 2002
	ENTRY1("1r02_artif", "020615", "57e9591631815f341356340eb3336994", 102912, Common::IT_ITA),
	ENTRY1("1r02_sottogiallo", "020611", "9e05ab30b21c2189359ebb87f07ed79e", 106496, Common::IT_ITA),

	// One Room Game Competition 2003
	ENTRY1("1r03_floppy", "040114", "178d3034872888adf68d6ea8d4e013d4", 67072, Common::IT_ITA),
	ENTRY1("1r03_successo", "040114", "bb4b43e17b1ee5faffe3824653c30ff9", 81920, Common::IT_ITA),

	// One Room Game Competition 2006
	ENTRY0("1r06_finalselect", "060419", "2690bfb5cc529c338ed6df923d44e28e", 151040),
	ENTRY0("1r06_finalselect", "061009", "2dd1a94db68abf9626f55e8bd6fa91a7", 151040),
	ENTRY1("1r06_sforacchiato", "051114", "32b25f95b3fa4d12df956d80483af22f", 128512, Common::IT_ITA),
	ENTRY1("1r06_devilatvenice", "060514", "fad46e7b7fcba0ec52a91d188c3b0c90", 87552, Common::IT_ITA),
	ENTRY1("1r06_galeotto", "060507", "f4fd45e121933080813f7616838a3292", 81920, Common::IT_ITA),
	ENTRY0("1r06_easter", "060506", "7824bb61c8f4476decfbc0cae326aa19", 79872),
	ENTRY1("1r06_lazyjonescrst", "040383", "c8d6a6982b29f8eb32ca269b8ad71959", 91648, Common::IT_ITA),
	ENTRY1("1r06_dereditu", "060502", "f7413ac65ef04a99d63688f1f0ad85b0", 96768, Common::IT_ITA),
	ENTRY1("1r06_frankenstein3", "000001", "afd4d53fcce2188d0a1fec585e56a690", 87040, Common::IT_ITA),

	// One Room Game Competition 2007
	ENTRY1("1r07_dreimp", "071117", "940f42557242f109cd87c3be791a056a", 70144, Common::IT_ITA),
	ENTRY1("1r07_kinesis", "071118", "b86d57a022ce0f0fd6881dd6558ae5e3", 179200, Common::IT_ITA),
	ENTRY1("1r07_necromante", "071107", "d38417af6003c64cefdb1cd59446c611", 109568, Common::IT_ITA),
	ENTRY0("1r07_suvehnux", "071114", "734690200a67137177928b93c7bbe7b8", 247296),
	ENTRY0("1r07_suvehnux", "071226", "e82d434c1c0a73a8755a9394a7c2e088", 248320),
	ENTRY0("1r07_urbanconflict", "071116/z8", "19d9fe507d8ab34b7583a3ace48dc372", 385024),
	ENTRY0("1r07_urbanconflict", "071116/zblorb", "3df26fcba4d30314f5d071a814e6525b", 886834),

	// RomanceNovelComp 2002
	ENTRY0("rnc_chooseromance", "020131", "c851c0eed65417b51cc2a21a7267d377", 38912),

	// Segment Mini-Comp
	ENTRY0("snowmansp2", "050318", "564cd46b498aff8bc07b9bebf120d8a6", 90112),
	ENTRY0("kaboot", "000001", "f62f20c9a96c93ebefdef4dadfdd3d31", 81920),
	ENTRY0("fandbsnowman", "050306", "88f22098e7794ac9415ddd95b8e3a713", 175104),

	// Shufflecomp Competition 2014
	ENTRY0("sh14_50shades", "140513", "35ba30fad41bdb74dbad8c5127bc657d", 252928),
	ENTRY0("sh14_flotsam", "140512", "a22df7cc5b1aa4e8d273b4e0d87e3ee1", 503778),
	ENTRY0("sh14_robotempire", "140511", "1b1adf587847239ef90eafdb2dfa756c", 431472),
	ENTRY0("sh14_robotempire", "140601", "0be65fa2571465799544009bbf9cc083", 431986),
	ENTRY0("sh14_illuminate", "140512", "95636ace9c03e03b414d3432aaa7d656", 278528),
	ENTRY0("sh14_lobsterbucket", "140507", "a56fe70772a0f57eaa10eef5de10afb9", 312492),
	ENTRY0("sh14_monkeyandbear", "090402", "b18cb887e833e22ba7cd4780428fa690", 542266),
	ENTRY0("sh14_more", "140511", "4340ca61065c79f0ca13dd20c7c6b48b", 315392),
	ENTRY0("sh14_more", "140604", "6e3ec8554b9c877c088fa586c965d896", 315392),
	ENTRY0("sh14_teaandtoast", "140513", "42f8e870d33b47ad097ee0996461ffe6", 416672),
	ENTRY0("sh14_truth", "140512", "713d190c6296a877de3d2a4fd48e93f6", 187904),
	ENTRY0("sh14_whitehouses", "140511", "bfa0206e9903d19551db483f810efa09", 423362),

	// ShuffleComp: Disc 2 Competition
	ENTRY0("shd_everythinggame", "150418/z8", "9be1a20d73ed0db81972287766aa3cbc", 315392),
	ENTRY0("shd_everythinggame", "150418/zblorb", "130aa6031169e969258ef8e6e1105f52", 1490056),
	ENTRY0("shd_headingeast", "150507", "ee6869fc4d02966a7ebc7681ae751dc2", 109568),
	ENTRY0("shd_seeksorrow", "150505", "0e37bd629858f51e2c83649973170dec", 1362330),
	ENTRY0("shd_seeksorrow", "160505", "e985ee4cff0da7f034cc76fdf5a65f6e", 1365918),
	ENTRY0("shd_seeksorrow", "160602", "6108059cd52dc1aba9112b554d6aee0f", 1365918),

	// SmoochieComp 2001
	ENTRY0("sc_1981", "001207", "8ffc6af37b119a05d9b0c92e63eaacce", 66048),
	ENTRY0("sc_august", "010201", "3d3f28c6bfb3884045aebb4cc2280c5d", 180736),
	ENTRY0("sc_deadofwinter", "010104", "20b4288688fd46267ebbad67bd83e18e", 59392),
	ENTRY0("sc_pytho", "010201", "bb3bfb384257c4fea46d7fd7fd3604fe", 267264),
	ENTRY0("sc_sparrow", "010207", "bbcb51f48f55802ac0c4e8bb8507cc91", 146432),
	ENTRY0("sc_bandit", "010210", "222817b5a8665dcd31ab4c5c0d70c189", 102912),
	ENTRY0("sc_voices", "010203", "a26ef705246c5e096886772363b316fc", 81920),
	ENTRY0("sc_voices", "010504", "9a78da996a62281db9047c9998cce0a2", 84992),

	// SpeedIF 1 Competition
	ENTRY0("sif_coffins", "981002", "dba12eaf1628bef4eae0ccbbdd9f0f0e", 49152),

	// SpeedIF 2 Competition
	ENTRY0("sif_speedif", "981028", "a05931b907599053238cb88ddb7431a3", 58880),
	ENTRY0("sif_hallhand", "981029", "a8b7e5e9c848fa54ed7c09d8f783c749", 53248),
	ENTRY0("sif_pumpkin", "981029", "9a2476e01f2cfc287e7e53b2007951c3", 50176),

	// SpeedIF 3 Competition
	ENTRY0("sif_boygoat", "981204", "082ffcff942327fae21328fb9bd1d8cf", 49664),
	ENTRY0("sif_pryde", "981204", "2562c4cafc2079e56aa7ae2391a82358", 52736),
	ENTRY0("sif_devildoit", "981203", "6e521967f04375cde72a8151c5516f18", 59904),
	ENTRY0("sif_devildoit", "000724", "c4c97ca9af421dc9a14849355e7f7a1f", 60416),
	ENTRY0("sif_3steps", "981203", "a7d8a011417486e12485efda8cef3bf0", 53248),

	// SpeedIF 4 Competition
	ENTRY0("sif_oniondig", "990910", "c162af826b6d6feaf7a6df4b60aa726d", 52224),
	ENTRY0("sif_firstwave", "990910", "c55a1bddc8f9427210a8d6de58c26a80", 52224),
	ENTRY0("sif_onion4", "990909", "d3d40c007343dd67aab6d59c031b8aaa", 56320),
	ENTRY0("sif_onnoef", "990910", "13c2f7b5092b21ca49558501e344f394", 51712),
	ENTRY0("sif_tearsmayfall", "990910", "13c612bac9b0b4794db811da1ec3419c", 82432),
	ENTRY0("sif_profesee", "990910", "7534fc3e48a64cfa0a41a80421aaa06e", 52224),

	// SpeedIF 5 Competition
	ENTRY0("sif_doomsday", "991121", "22b8c516621933dc7b0098e8542c8a80", 59392),
	ENTRY0("sif_chalupa", "991121", "2e57fc33f50760c1c5c8aaf5f2c1cae6", 53760),
	ENTRY0("sif_courier", "991122", "870674c88d5b4eb64478166e09843c66", 51712),
	ENTRY0("sif_pspy5", "991121", "1094fe46099bf7d736d94016500423cc", 57856),

	// SpeedIF 5.5 Competition
	ENTRY0("sif_carnival", "991126", "54c0bb36c976e1f9f7c53682e3988d45", 50688),
	ENTRY0("sif_lake", "991126", "765c03556cdd0ad1c10e7c80d9467eef", 79872),

	// SpeedIF 6 Competition
	ENTRY0("sif_kids", "991010", "b6f63ae17ea726fb95990ad3e9861462", 50688),
	ENTRY0("sif_atlantic", "991127", "2ae2d13b9e672c9485db0750940aa749", 53248),
	ENTRY0("sif_speedif6", "991126", "11aeda346d66e18ac4072b1023d239aa", 57856),
	ENTRY0("sif_titanic", "991127", "22303ca7c6a59f516e952995ec227d98", 56320),
	ENTRY0("sif_whengrow", "991127", "5d86fd8e5819ea5668ce1e459c5ae9a4", 88576),

	// SpeedIF 7 Competition
	ENTRY0("sif_lilyjane", "991203", "429c61757b877019112197153ce01763", 85504),
	ENTRY0("sif_tankweb", "991202", "44a2c2a05033b346e45f29e0f9b32b23", 48640),
	ENTRY0("sif_tears", "991006", "3a15e42b816b80e077f66b67cc83f6a8", 52224),
	ENTRY0("sif_oilydeeps", "991203", "db1dfc83d011e8df7a79f8fe133efa15", 83456),

	// SpeedIF 8 Competition
	ENTRY0("sif_hangers", "000108", "b3a563d19bcfb4d0d2bc4862dcd181f7", 54784),
	ENTRY0("sif_seattle3", "000115", "e0f4dd4efe16c53c2017883746f4c996", 52736),
	ENTRY0("sif_pantsless", "000114", "fa8bb85defb16e81aef67cf204b01e7c", 57344),

	// SpeedIF 9 Competition
	ENTRY0("sif_ucodarkness", "000414", "4c5b7da3ad90064214e96829f764fdf5", 51200),

	// SpeedIF 10 Competition
	ENTRY0("sif_deathx4", "000626", "4ef58e4cd15299b52075d5937b71d755", 51200),
	ENTRY0("sif_taxes", "123456", "4cfde3ed88043fce6bb953021be69287", 79360),

	// SpeedIF 10^-9 Competition
	ENTRY0("sif_krakatoa", "021215/v1", "3c62064723cfd8a89beb081c1703d133", 73728),
	ENTRY0("sif_krakatoa", "021215/v2", "3901a6e3da1bf57ccc33e9da662eb340", 73728),
	ENTRY0("sif_roadestruction", "021214", "34d12792dda75cd8036455b2b2d74855", 58368),

	// SpeedIF 11 Competition
	ENTRY0("sif_spiders", "000529", "0303538d9e2755d79d9f6e9bd4500487", 54272),
	ENTRY0("sif_fido", "000529", "d70e427426e3a39d21249cb43b69570f", 51200),

	// SpeedIF 12 Competition
	ENTRY0("sif_pantheon", "000708", "7db217734ac5c76a6bd0d60dfcc3fdc2", 84992),
	ENTRY0("sif_plaque", "000713", "56afa6230573c3f74aac8c4e9949e2de", 57856),

	// SpeedIF 13 Competition
	ENTRY0("sif_elephantaflife", "000728", "31f78d010a7ea81621ea606d3002901f", 53760),
	ENTRY0("sif_elephantaflife", "000729", "4de2b50d8f115693a1cfb3a17de51463", 53760),
	ENTRY0("sif_garden", "000801", "dd8d5ee2e9d4d251ab6a7710f808013e", 53248),
	ENTRY0("sif_inspector", "000728", "67ffeee55cf1f664e68ae28fd21bd14d", 90624),
	ENTRY0("sif_potstick", "000728", "3a6548770a04a2bfdf337fdcb30e4801", 86528),
	ENTRY0("sif_service", "123456", "5acd285b74847a2fe2d99ba72634dc31", 79872),

	// SpeedIF 14 Competition
	ENTRY0("sif_halloween", "001025", "43302fac137967573aa61119e0a6359a", 48640),
	ENTRY0("sif_bbp", "001024", "e346b6b69928ffd316fb9d71d6d10868", 54784),
	ENTRY0("sif_pcontest", "001024", "ecb1d0f122f8734bbab848df40f3fb91", 60416),
	ENTRY0("sif_worm", "001022", "19df66278ebf4f376fef567f77387547", 51712),

	// SpeedIF 15 Competition
	ENTRY0("sif_assignment", "010314", "4cf739e836a9010f0356673f5ee6b0c0", 82944),
	ENTRY0("sif_basketdestiny", "010314", "9cf783d83fa994ee9a335328e5ac7397", 84992),
	ENTRY0("sif_broken", "010314", "654be5a7a2f114e60020b505ef496d28", 55296),
	ENTRY0("sif_butler", "010314", "51833dae862be8d731a23525c9295752", 50176),

	// SpeedIF 16 Competition
	ENTRY0("sif_agent", "010407", "9cfa382463c779ab11c55407b624350b", 54784),
	ENTRY0("sif_marie", "010407", "cd34939dfbcdf73ce80532cec7102f00", 62976),

	// SpeedIF 17 Competition
	ENTRY0("sif_2604", "051001", "848dab033905a4b03bdfedf2f028ba19", 68096),
	ENTRY0("sif_roof", "010812", "e8484ee8c4e7ba76e423fc6471b89832", 55808),
	ENTRY0("sif_123102", "010812", "6e0e67a1ad9f4692cf3eac81c61ae976", 81408),
	ENTRY0("sif_speedif17", "010812", "a4afe7c2c5b6918baa061ec8d822a5d3", 50176),

	// SpeedIF 18 Competition
	ENTRY0("sif_fishspaceships", "020127", "974ceba7fed3308d36ed6e5610ccbfee", 56320),
	ENTRY0("sif_glossary", "020126", "0eb270d845f62e611981ed89f98f4fd9", 55296),

	// SpeedIF 19 Competition
	ENTRY0("sif_98769765", "030112", "5815c231b1cd75a1dcd4b00242787202", 54272),
	ENTRY0("sif_cheerup", "030112", "efc0104cdafc2166891b2fbd83f62619", 55296),
	ENTRY0("sif_darksoul", "030113", "f6635c0a0f1f7a4397615d09dc4dca9e", 111104),
	ENTRY0("sif_falling", "030112", "345015ecab1f95cb1511f8c4fb00b4b0", 55296),
	ENTRY0("sif_speedif19", "030113", "0fb4ee99c0e5ee4183c6d86e987300c5", 81408),
	ENTRY0("sif_angelcurse", "030112", "25734304c31c9332c778620175663a02", 55808),
	ENTRY0("sif_upon", "030112", "ed94f6dff892ca680e997abe7579994b", 59392),

	// SpeedIF 20 Competition
	ENTRY0("sif_lrc", "030629", "2e37e5a547675cb7eaea0840430b5738", 54784),
	ENTRY0("sif_sellout", "030628", "396d4a867a371e1b2ccde28b31aa3a26", 55808),

	// SpeedIF -1 Competition
	ENTRY0("sif_discord", "011118", "bf8d764ca8f63323da2679fe67344cef", 58368),
	ENTRY0("sif_generic", "011118", "b60d3b98341cc3a4fca4ac3ef68d502d", 82944),
	ENTRY0("sif_hankbuzzcrack", "011110", "f976da3c53f286f2e20e7f5f6d414599", 50688),
	ENTRY0("sif_literacy", "011118", "245b365f8ef81c146b3d091aa664848b", 52736),
	ENTRY0("sif_woodencat", "011119", "3628449c452f7d0795d89cdfcce7d6da", 56320),

	// SpeedIF 2000 Competition
	ENTRY0("sif_ritual", "991212", "f3a53b9e837a99e58a07133aef9cb23e", 55808),

	// SpeedIF 2001 Competition
	ENTRY0("sif_battleplanets", "010121", "3fcc87126411e841608884a01d136ccf", 51712),
	ENTRY0("sif_destinychihuah", "010121", "006a95783c9a801962b5d4a72da37df5", 56320),
	ENTRY0("sif_infiltrt", "010120", "58b67bb5aa44391f787d69af2d8c3406", 88064),
	ENTRY0("sif_inspiration", "010120", "4c06fd195b9b710d4a3776bbb70f65cb", 82432),
	ENTRY0("sif_moonjupiter", "010120", "54129d64d0b157c8160b67e86eae31e7", 88064),
	ENTRY0("sif_crescent", "010120", "0e6d919356caf07007b368c62b0b7c67", 65536),
	ENTRY0("sif_crouton", "010120", "e6d2fe9ce2aeee4943a165cf777e1ef3", 50688),
	ENTRY0("sif_speedif17v1", "010121", "b9a7b208928c7c71f3df61654e496240", 51200),

	// SpeedIF 3rd Anniversary Competition
	ENTRY0("sif_henry", "011130", "922ca75bdcb5f53b92dd951f0747f30e", 52224),

	// SpeedIF 5th Anniversary Competition
	ENTRY0("sif_coughcough", "090715", "b62aa1f1f815251d64f734ed991bf74c", 82432),

	// SpeedIF 11th Anniversary Competition
	ENTRY0("sif_nqaa", "090901", "50f7abd76e9f972b4ee07a6f22259513", 132096),
	ENTRY0("sif_reptile", "090901", "f919b917e924b9d4f5465e4f262082a0", 159232),
	ENTRY0("sif_teachher2dance", "090901/z5", "05f40a30e9fcbff5785042452b3d2c27", 146944),
	ENTRY0("sif_teachher2dance", "090901/zblorb", "d9c77a55fb2397f35e1de9f8bff92723", 239054),

	// SpeedIF 18th Anniversary Competition
	ENTRY0("sif_farout", "161009", "1ad837661bb8b49c4dde09f149ede7fe", 254422),

	// SpeedIF Argonaut Competition
	ENTRY0("sif_asciargo", "030309", "d660f5a1a71653e2701528ec9340ae20", 69120),
	ENTRY0("sif_fleece", "030309", "da453dfbaf8efeeb9c29179e7e6f2429", 4608),
	ENTRY0("sif_invisargo", "030310", "cb865e5adc7fed80b97590308000ddd6", 92672),

	// SpeedIF Autocratic Competition
	ENTRY0("sif_toeing", "040711", "85feee01a6241399e2ef036c0641c0ec", 78336),

	// SpeedIF Century Competition
	ENTRY0("sif_mission", "040228", "ce30115726113f1c471b0bd54babc8ae", 59904),
	ENTRY0("sif_yaygames", "040228", "6768fa6a09bd6d30bb2f615dacda165b", 67584),

	// SpeedIF Copyright Competition
	ENTRY0("sif_piratescaribou", "030413", "63864258392ef2294ee96bba367edfdc", 119186),
	ENTRY0("sif_sharingallway", "061023", "b21fcf0445928c6954968760a2977f56", 124284),

	// SpeedIF Crinkle Cut Competition
	ENTRY0("sif_travels", "020323", "e794ec45a8872fcf5125427c84590a26", 52736),

	// SpeedIF Douglas Adams Tribute Competition
	ENTRY0("sif_nightmilliways", "010514", "92e9a393aa3bc7dcad6e42ef38575dec", 84480),
	ENTRY0("sif_deadlinebda", "010513", "b9b5a235e36e70b5aeca511a9a02a9f8", 51712),
	ENTRY0("sif_howmany", "010513", "64156dd269c213a2f104a0fa1eaa83ef", 86016),
	ENTRY0("sif_minds", "010516", "071f8399f2a8e988930a117600cdaaa0", 68096),
	ENTRY0("sif_sofa", "010515", "7ae1c4b11be3b9c74217f7d4cb0c4299", 56832),

	// SpeedIF EXTREME Competition
	ENTRY0("sif_andthen", "020202", "5f2391978e4b6ff0629364aaa8c0a347", 84480),
	ENTRY0("sif_oth", "020202", "e12a6e6e267abc34be404d5c5d006094", 50688),
	ENTRY0("sif_rjd", "020201", "2f25d020b09810c1fd6ab8596be21a57", 53760),
	ENTRY0("sif_tightest", "020201", "92422f17e9eee691e3a00442f20a5744", 54272),

	// SpeedIF Gruff Competition
	ENTRY0("sif_bronze", "060121", "80086fa750c3a0702da93ddb11a464c7", 142848),
	ENTRY0("sif_bronze", "060503", "ef5b57f69601b8f5944699a6b31bc58d", 492472),
	ENTRY0("sif_breath", "060121", "317b7bb6a03fca27b306e60791bbe889", 76800),
	ENTRY0("sif_green", "060120", "dc913a1059a23eb95372692fd65c3078", 46404),
	ENTRY0("sif_gameiwrote", "060121", "d11c98805aa132e82893c789f82a5f0e", 52736),

	// SpeedIF Halloween 2002 Competition
	ENTRY0("sif_losing", "021031", "68073c13dddeb29ce8e9530d27fd28c0", 51712),
	ENTRY0("sif_oldsherwoodcem", "021031", "c40277f69d0d57f960bca6351043dabb", 85504),

	// SpeedIF Indigo New Language Competition
	ENTRY0("sif_hauntedhouse", "110507", "760df7716aecc0f1e457a2c22735c854", 298436),

	// SpeedIF Introcomp
	ENTRY0("sif_holygoat", "070827", "55a05768c3616ad296bf86ff392249db", 138164),
	ENTRY0("sif_alcohol", "070827", "539270360a14005cd2fc1eddfbb5b21e", 136192),
	ENTRY0("sif_djibouti", "070825", "fb22ff8e39ba2d46fa892370fda57b8d", 159102),
	ENTRY0("sif_faetttiw", "070824", "c14f57f0a8225ebf553730633dcc7d4a", 149246),
	ENTRY0("sif_faetttiw", "070825", "2c4c1da83791e33977b6cd355a40001c", 149768),
	ENTRY0("sif_velocitasficts", "070826", "3bf4260115172d5e5e595c0055a24b4f", 154664),
	ENTRY0("sif_baptist", "070826", "726d3eeee1b405005a6f01f3531d6f5a", 145408),
	ENTRY0("sif_zefrench", "070827", "62fbd05552dd52ea6959017b4c3a20f1", 140288),

	// SpeedIF Jacket Competition
	ENTRY0("sif_shangri", "020420", "aa085e22a5b5eaee3ea3b86931fc28c1", 56832),
	ENTRY0("sif_soulsrch", "020421", "5158b3c0e7a0b0e280a4272184b8f17b", 83456),
	ENTRY0("sif_circussadness", "020421", "078de628d462cd77ee8fa172f6d3a3d8", 51712),
	ENTRY0("sif_count", "020510", "7da6015e54c2e4dcdcb6f5d521047fee", 86528),
	ENTRY0("sif_sewage", "020428", "6544df1122c37fa2d8a5e8a385d393d5", 56832),

	// SpeedIF Jacket 2 Competition
	ENTRY0("sif_beinglittleguy", "030413", "72a6b7db7ef8083f681e3ca6e08ef310", 58880),
	ENTRY0("sif_pirateninja", "030413", "f51d74c8e8f1cff6e966cda7fb0e61f9", 61952),
	ENTRY0("sif_body", "030413", "aabc7787e3dee7a895c6262d079b4848", 63488),
	ENTRY0("sif_gwdmpete", "030412", "887e9cfe9df3b6fe09ed7fd2205a4662", 60416),
	ENTRY0("sif_towerofbeef", "030413", "8c404c72f375b406be6f733a99c68480", 89088),
	ENTRY0("sif_travel", "030413", "68e1ae3f9ed8f35c97c71d01bf9ea235", 94720),
	ENTRY0("sif_whatdreams", "030413", "1d02be8ff80d4d6d7ccde6688c5774ab", 60416),
	ENTRY0("sif_wormwood", "030413", "4ea85d81fecb1f483f392cbbcd070af3", 62464),

	// SpeedIF Jacket 3 Competition
	ENTRY0("sif_fluid", "050828", "d463d569d1344e3d34403ae25d0a5a08", 76800),
	ENTRY0("sif_hentai", "050828", "a798a864720740f1f7361a6d169aa8c6", 61352),
	ENTRY0("sif_upwards", "050828", "4aa17aaa257d2f375dfa04c7e6e45a38", 119808),

	// SpeedIF Jacket 4 Competition
	ENTRY0("sif_moondarkling", "110416", "0550a06426463c728d3b67b46959abf5", 279506),
	ENTRY0("sif_lachryma", "110415", "b13f6aa55f041792311301e4c8df4900", 162816),
	ENTRY0("sif_gorilla", "110416", "9f11aef3e13431c1e5385928843e6052", 168960),
	ENTRY0("sif_oceantower", "110416", "499631e0f669b7db9f80ba47fc61a664", 303670),
	ENTRY0("sif_oceantower", "110510", "27e6ac7767ac9dfe5f0082765f6a21b7", 306962),
	ENTRY0("sif_dreamtrap", "110415", "b29b6cbec814550dd55e8b2f423f1a52", 291840),
	ENTRY0("sif_spectrum", "110415", "cdc7a95c8bb40e64d0658709704444f7", 210432),
	ENTRY0("sif_smoochiepoodle", "110414", "605905b3574983313e11574951a83043", 318472),

	// SpeedIF Let's Make A Nightmare Competition
	ENTRY0("sif_sdoor", "030916", "81c369511e7ec797d56293485cb16abb", 60884),

	// SpeedIF Late! Competition
	ENTRY0("sif_canape", "030524", "6860a3aed364bf22670666fae0e8f1b1", 65536),
	ENTRY0("sif_lonestar", "030520", "99cf26aa1928080c25806ad817e1f662", 60928),

	// SpeedIF Neologism Competition
	ENTRY0("sif_minimumwage", "060514", "a77254f85405d7dde6299bc27ea73182", 162816),
	ENTRY0("sif_almamater", "060513", "282bf9877a7d556884755df485abe335", 124438),
	ENTRY0("sif_hentai2", "060514", "3130dedf0811b44da5429d79fc0c78fa", 147092),
	ENTRY0("sif_definefar", "060514", "8388bcb4a800f64a29e6e1d964d4b6ef", 120714),
	ENTRY0("sif_expedition", "060514", "c0851711fe1037eca6a3f3f66faaf5b5", 118640),
	ENTRY0("sif_speedifnockle", "060513", "fb7d8536e61e1f70dc641cee73846200", 121254),

	// SpeedIF New Year's Speed Competition 2007
	ENTRY0("sif_nazimice", "071229", "ee0520b15d0fdd8633498b42154d8486", 139264),
	ENTRY0("sif_notsameols", "071230", "ef765c232443f661c817d8f9a9bbd00d", 223232),
	ENTRY0("sif_shothitler", "071229", "6a2f3c72e4cf395b29c07ea88756a668", 173568),
	ENTRY0("sif_newyearswar", "071229", "6a9e22fe827900831f665a16132ffa81", 154624),

	// SpeedIF New Year's Speed Competition 2008
	ENTRY0("sif_leaptime", "081226", "42e6f54aabedcdb8ccf4caaff341b4ce", 228352),
	ENTRY0("sif_leaptime", "081230", "aa7363833f61f3b5ac21d2d681ab74bb", 233984),
	ENTRY0("sif_mauled", "081224", "bf6f8e0783aa8980e81c501d55f52bb9", 173056),
	ENTRY0("sif_escapist", "081227", "086efab633103cc9f2bf8c223df436c9", 148480),

	// SpeedIF New Year's Speed Competition 2009
	ENTRY0("sif_dullegriet", "100116", "00c9f618ec2299548ae5d961ad27a500", 169472),
	ENTRY0("sif_stabbed", "091225", "33b36976ba3a34ccd4f73ce7b95ed468", 187392),

	// SpeedIF New Year's Speed Competition 2011
	ENTRY0("sif_beingendbeg", "111230", "7bd6ecaf3861cd0f35798ad64a33fef7", 375758),
	ENTRY0("sif_birthmind", "111230", "3903332c627cba6350885c17b2bde4b0", 351772),
	ENTRY0("sif_dashslapney", "111230", "798921e044aaed9d16f8b2b238b7dbf9", 331726),
	ENTRY0("sif_postchrist", "111231", "8b63e6a1d662b58bc19bf0dfcdc6ea60", 272568),
	ENTRY0("sif_ragnarok", "111230", "2ecebb2b217411de0f4cd4920443e200", 82432),
	ENTRY0("sif_stupidcreek", "111225", "c69426c0a17abc2622a4dfade83a4266", 237568),
	ENTRY0("sif_savedelvis", "111231", "10ca0d67db593ee6c81cadd57b2043b2", 191488),
	ENTRY0("sif_silencegods", "111230", "0aeca7f64c8a551f4cadefca0ae57aca", 294828),
	ENTRY0("sif_threemore", "111230", "356403c4a1f4e86ae5ed1a26b20013f0", 152576),
	ENTRY0("sif_threemore", "120110", "d89d23f3cacd8541df1b1792362b206b", 155136),

	// SpeedIF New Year's Speed Competition 2013
	ENTRY0("sif_faithfulcompv2", "131231", "2aaed4f67468e3e269f211ba1caadd54", 343450),
	ENTRY0("sif_talemorning", "140103", "a189df712a5254487eb9a9d8cb8fffca", 253952),
	ENTRY0("sif_talemorning", "140109", "234a6da218d56ca47410f7e03c2b89a8", 286756),
	ENTRY0("sif_dayicameback", "140104", "c1abdfa44ae62d8194ce301f38f986c4", 356302),
	ENTRY0("sif_youhavetoput", "140103", "ce8c2720fd4adab602737e31e3c15554", 152844),

	// SpeedIF New Year's Speed Competition 2014
	ENTRY0("sif_youwerehere", "150101", "44d44bcc5e4a09b2b5a8c91ff6e6ffe9", 659824),

	// SpeedIF O Competition
	ENTRY0("sif_speedo", "010618", "b511f29c86a7e13de673926226383cbd", 49664),
	ENTRY0("sif_eschew", "010619", "5400d94e3b040b5b562edb459784cb11", 79872),
	ENTRY0("sif_getting", "010619", "c4ecc47413d22b405139e3e1aad71e31", 57344),
	ENTRY0("sif_astrology", "010619", "8b822d755dbd37c3bef517f7824c4ab2", 57344),

	// SpeedIF PAX East 2010
	ENTRY0("sif_wizshop", "100327", "3c288b1b9a0738c055949f77d7a1d904", 188416),
	ENTRY0("sif_apoptosis", "100327", "41a8f9ed1cda9df15ce634e6c95ef13f", 289728),
	ENTRY0("sif_foodmagic", "100327", "ea2ba1dec06b945d13509eb5a7cf4404", 279028),
	ENTRY0("sif_lobsteronplane", "100327", "60d4c46b482fa3e064bbb6f4ade216a4", 204288),
	ENTRY0("sif_mango", "010505", "d59a1a92f7855161a9d11df40eb14f33", 268226),
	ENTRY0("sif_midairmadness", "100327", "a3e636c4c0e87291e64dece9226a2c59", 78848),
	ENTRY0("sif_paxeast", "100327", "e457d1a18a37b2f33428cac8d8d8c4ab", 18772),
	ENTRY0("sif_queuelty", "100327/z5", "776595bd1cfed4c67ed8dc4baf8261e9", 218624),
	ENTRY0("sif_queuelty", "100327/zblorb", "98380afe4c3005eacb229cbe0c737480", 310940),
	ENTRY0("sif_safehouse", "100327", "e2b98a8b1dfc7364ef96b668ddcd281b", 190464),
	ENTRY0("sif_vortex2305", "100327", "539c5143e05f8ee96e724e88e2fc76ac", 196680),

	// SpeedIF PAX East 2011
	ENTRY0("sif_scurvy", "110313", "1345c0f052db46981d21dfca1f61164e", 266224),
	ENTRY0("sif_scurvy", "110316", "6fbeb4d7013e53830db77aed351a7715", 267760),
	ENTRY0("sif_shhelpfulman", "110312", "413e1a37570643c936bf5d953b23d792", 183808),
	ENTRY0("sif_boredladders", "110313", "67c2a4d28c2d9b4010eb5df84bf5ac23", 256416),
	ENTRY0("sif_brainightguest", "110312", "a53b823f032f6f24cf726cb3807126e0", 232448),
	ENTRY0("sif_delusionsagain", "110312", "d2f2e384a82561770b1ba346c4cecd48", 247242),
	ENTRY0("sif_diggingtime", "110312", "d61670df1a94d9272acdf9282e2ea587", 82432),
	ENTRY0("sif_merk", "110312", "1ac23aa5b9044df12be4c9a690e51ee5", 317392),
	ENTRY0("sif_therighttool", "110312", "0d6fa415b46bf15400f215beb212cbe6", 152576),

	// SpeedIF Orange Competition
	ENTRY0("sif_appall", "020223", "1659420ba9bdac0036857ceef6544826", 52736),
	ENTRY0("sif_appall", "020422", "3c33d5639fa341829dde7c4b9b1875fd", 52736),
	ENTRY0("sif_descent", "020223", "ed6f047a085184879b62cfbdd8b00bb6", 55296),

	// SpeedIF Pi-Theta-Aleph-Parallax Competition
	ENTRY0("sif_parallaxdream", "010722", "eec02151066a419e37945c4d0524febb", 74752),
	ENTRY0("sif_pethertheta", "010722", "fdbd5f400e09d787c41f9983e40e77dd", 50688),
	ENTRY0("sif_putiap", "010721", "b72c110cf4e03388bc5c3aeef9718adc", 53760),
	ENTRY0("sif_squeaky", "010721", "3dc567c99b6df70f258a50f4c8995998", 50176),
	ENTRY0("sif_squeaky", "010722", "7c50ba3f388d57e96afeabbc6fbd46c0", 51200),
	ENTRY0("sif_ptap", "010721", "be484b2ac6ca2915c558f0fe85f1f922", 82944),
	ENTRY0("sif_moonbar", "010722", "19d87eff290b6672ea6bcfeec864711c", 53248),
	ENTRY0("sif_thpoint", "010721", "28fcea7b45429830ec60a7aaefb66609", 110592),

	// SpeedIF `R*IF` Spam Competition
	ENTRY0("sif_htp", "090926", "d6ee7e9f0522592ebfdfc689a0639e9c", 148480),

	// SpeedIF Scenario 1 Competition
	ENTRY0("sif_stars", "040906", "a48ce62a62308d64c8b5c9617fc8e816", 57856),

	// SpeedIF SchmeedIF Competition
	ENTRY0("sif_floyd", "010527", "ed4966cfb5d563e798f2b209616a975b", 55808),

	// SpeedIF Spring Competition
	ENTRY0("sif_exterminate", "080330", "58e790f25c5ade94bd2cb7df6be11761", 64964),
	ENTRY0("sif_gardening", "080328", "63b4d6db132a306c6af20aa2cf593116", 209920),
	ENTRY0("sif_springclean", "080325", "be62d08593efb378187c36bfae66a6ac", 304926),

	// SpeedIF That Dare Not Speak Its Number Competition
	ENTRY0("sif_soitgoes", "040125", "f1ce20135cbfd98afcbb7671788d7f2e", 88064),
	ENTRY0("sif_antioch", "040125", "15a13547cb14d29b6851ca54928c02a8", 81408),
	ENTRY0("sif_twilight", "040124", "930e1be11b7f2e49e45bb99eae7c40b9", 58052),

	// SpeedIF Thanksgiving Competition
	ENTRY0("sif_dragonflies", "111126", "d2183016a28394ca8e3dca3e26abd68b", 208384),
	ENTRY0("sif_dragonflies", "111130", "ff692bffb014fe109c3bfe2e072ac98b", 565060),
	ENTRY0("sif_crystalpalace", "111125", "70b49b3ed49f022f73796765008e1e6a", 310316),
	ENTRY0("sif_extraordevents", "111127", "062ed8c5329ef1460a7662da4409de11", 252990),
	ENTRY0("sif_youareaturkey", "111127", "b3b0da101be635d2a8f0ee75dce5e599", 154624),

	// SpeedIF ToasterComp II Competition
	ENTRY0("sif_stolen", "030412", "26323e84ba1a62b6b5671938e5ff8a0d", 55296),
	ENTRY0("sif_needmoretoast", "030412", "ec4377d7ff8dac21f1032542428471c8", 52224),
	ENTRY0("sif_samurai", "030412", "956618a723c9d7c2a21d2cfddeadeb4d", 83456),
	ENTRY0("sif_toastless", "030412", "d41c03111539eb4842ff4be663ce6bc8", 81920),

	// SpeedIF U Competition
	ENTRY0("sif_cheatingdeath", "030525", "eda221a4306e0942e9e589504756fd20", 54272),
	ENTRY0("sif_rrr", "030525", "ecf72d85d6056bdaeed7de43942248dc", 59392),

	// SpeedIF Ultimate not numbered New Year's Competition
	ENTRY0("sif_disenchantbay", "130106", "4a982fea8dab9bebd65062f958a0d81f", 330634),
	ENTRY0("sif_disenchantbay", "130104", "dbf7fe64ac7a33b362eb9e8f2e40c29e", 282804),
	ENTRY0("sif_shortof", "130103", "180dbfc43c99c579b6409fcfaf86dd91", 131072),
	ENTRY0("sif_thedayidied", "130103", "d8bc1ebc8d76fd060ec370618514fd1c", 342474),

	// SpeedIF Without Number Competition
	ENTRY0("sif_sushi", "010416/z5", "609ff60f2a1791592ee41b8666d49410", 58880),
	ENTRY0("sif_sushi", "010416/zblorb", "a8a04fb57890e1cd1d13bae889a5e2af", 327668),
	ENTRY0("sif_anotherday", "010416", "d977d9553917b261a172044c202f34a3", 55808),
	ENTRY0("sif_lobster", "010416", "1f1fce901db554ba51ce74e7e6a4123b", 75776),
	ENTRY0("sif_grape", "010417", "ed563fe64ca2d13e6b8cc40c37273f3c", 53760),

	// SpeedIF XYZZY Competition
	ENTRY0("sif_nightxyzzyes", "961127", "04f8d837971ef50d0f254c28bb9a073a", 56320),
	ENTRY0("sif_behold", "010311", "96de1ba0a26a8131252f1cb4d5c42b5a", 55808),
	ENTRY0("sif_tragedy", "010312", "5bab2d25280f18504487fcf6b3312107", 51712),

	// SpeedIF Y Competition
	ENTRY0("sif_bananapocalayp", "010525", "2c0d8c77d046daeb54906b2a59e066a9", 58368),
	ENTRY0("sif_barton", "010524", "e7e96181e9bc2827f5124b54689854fa", 81408),
	ENTRY0("sif_bearsx3", "260702", "03b6e09ed4cff7c98b55c84986983caf", 56832),
	ENTRY0("sif_flexible", "010524", "756fc59448127e97929a5dc189ec1393", 81920),
	ENTRY0("sif_pantsrun", "010524", "e30bf30b3fdde3eb4bffbdf3c98451d8", 79872),
	ENTRY0("sif_tripbeer", "010524", "9f6f1fa4b3c64632debcd50d2a1a5c93", 81920),

	// SpeedIF y=1/x Competition
	ENTRY0("sif_perfectday", "011209", "842954587d51fe75d6c0f398ae9736d4", 82944),
	ENTRY0("sif_apocolyptica", "011208", "76238690cb0dbd77bf23a163521dabd3", 55296),

	// SpeedIF Zombie Competition
	ENTRY0("sif_speedifzombie", "050823", "389f6c2083b608fd64c5b98e745b9318", 77824),

	// SpeedIF Bouchercomp Competition
	ENTRY0("sif_lowellparadise", "080528", "7718a75e194ec0fdf84a2460733a4d0a", 142828),
	ENTRY0("sif_putpbad", "080528", "69a7ab928c7d1f5196ee8cb954276861", 131584),
	ENTRY0("sif_seekingsolace", "080528", "bda0aec03bd17c8816c053ea5b25acac", 183544),
	ENTRY0("sif_wideopengate", "080528", "1898e262d619428285276aa626ff8b6c", 147968),

	// Swash Comp
	ENTRY0("swa_seacaptains", "030902", "7fb736fb5ffd548d87e0e0d36475a451", 120832),

	// Text Adventure Literacy Jam 2024
	ENTRY0("bakemono", "240428", "476c0811f5b57104a50238e32f9b235f", 103424),
	ENTRY0("whokidmgoose", "240501/z3", "c3973f356cd94999d4158259d27d1b20", 97280),
	ENTRY0("whokidmgoose", "240501/z5", "d9093db101f6995a19e4c8e0fe6ac5ad", 101376),

	// Toaster Competition
	ENTRY0("toa_burnttoast", "000931", "70dba603317dff1feafe1e1941021c95", 80384),
	ENTRY0("toa_friarbaconstr", "000831", "b499ff3d6468e7f3dffba37bb2ecd458", 92160),
	ENTRY0("toa_gottoast", "000818", "9b9efc642110f9cf8cf02d427926c420", 128512),
	ENTRY0("toa_spittingcrumbs", "000831", "2af8723590a4c2d1525e6f390da92053", 54272),
	ENTRY0("toa_shrinkingwoman", "000901", "e856b8d56da1c76283159728ec75b636", 52224),
	ENTRY0("toa_tommytoaster", "000815", "e85536f32722533a9aaa3fd3d6f88037", 59392),

	// TWIFcomp
	ENTRY0("twif_putpbat", "100414", "331ac3c5bb168e99453e5e74043b37c2", 140288),
	ENTRY0("twif_sin1", "100411", "725d9ec5b537fbd089fa23a7171ad875", 140800),
	ENTRY0("twif_sin2", "100411", "651b1474b605e6cca52bc6cfeedd8040", 140288),
	ENTRY0("twif_sin3", "100411", "8fa88ae34651a9908db107a34a5ee2d6", 140288),
	ENTRY0("twif_sin4", "100411", "a513b8ad862ac98145c9c5f82f5631e2", 140288),
	ENTRY0("twif_sin5", "100411", "e50b8d5da951482b9e39c9a8dd580311", 140288),
	ENTRY0("twif_sin6", "100411", "b717ffff4811e376b94e6c48e6e89482", 140288),
	ENTRY0("twif_sin7", "100411", "78e83d58c8f4802204ff47e0987371d6", 141312),
	ENTRY0("twif_sin8", "100411", "f24f52f675b678db3608d52e6563bf4a", 140288),
	ENTRY0("twif_war", "100411", "f93714ad73e07cd75ad5d00afde37c8e", 140288),

	// Walkthrough Competition
	ENTRY0("wlk_wtf", "010522", "dc7a87253da4ff3c05b2217ed564137d", 94208),
	ENTRY0("wlk_dreams", "010516", "fd9d677f57c1fb0b8a0b4d00dd8d52cf", 110080),
	ENTRY0("wlk_fit", "010520", "8b735022d6a71cef1d746123193cf304", 133632),
	ENTRY0("wlk_jigsaw2", "010421", "de554e3d1faca74b0d4a02d7bacd2da0", 61440),
	ENTRY0("wlk_exile", "010520", "bd4e1fb0b26eb5577db72e5b8788f416", 134656),

	// XComp 1999
	ENTRY0("xco_sixfoot", "990323", "37724e6551435bdbd674e7ffc4937d7c", 113664),

	// Spring Thing 2002
	ENTRY0("stc02_blues", "020618", "dea57d7b1b41b5961143e7faa6125ad9", 261632),

	// Spring Thing 2003
	ENTRY0("stc03_inevita", "030225", "8eb1204dade23de158a71f68e81b0437", 213504),
	ENTRY0("stc03_inevita", "030428", "6152b19bbf3eb3dd4334004e7094dbe8", 213504),
	ENTRY0("stc03_cofire", "030329", "1f84d501e89c64f2e04f84996b8d0ca3", 252416),

	// Spring Thing 2005
	ENTRY0("stc05_authority", "050205", "8def535c876448e1cc532a6d6dec328f", 171008),
	ENTRY0("stc05_flatfeet", "050313", "8b92f0777c98d72c1701bd080531f5b4", 275456),
	ENTRY0("stc05_telling", "050330", "e50190b0a236084d01595dee39749478", 210944),
	ENTRY0("stc05_telling", "060518", "cfcbfc5d4a7faad9724f7bae961ee71b", 316014),

	// Spring Thing 2006
	ENTRY1("stc06_debaron", "060329", "32a2298f08a7274e1c1e8ed28d8b99fe", 322560, Common::DE_DEU),
	ENTRY0("stc06_thebaron", "060329", "cac25a2e2ce7aa87bccccb07dffad212", 310272),

	// Spring Thing 2007
	ENTRY0("stc07_fate", "070304", "03b02750dac39a92ef6dccf2bc1ced4f", 434568),
	ENTRY0("stc07_fate", "070329", "b030c4881ced350f51c1a64bd0f45584", 481792),
	ENTRY0("stc07_fate", "070910", "a659ea01a268760a28bd9a990c8bfb73", 503138),

	// Spring Thing 2008
	ENTRY0("stc08_pascal", "080217", "ccdf58a87467b4e822565d54b3ca2a20", 237568),

	// Spring Thing 2009
	ENTRY0("stc09_milkparadise", "090214", "37f34b58e9907dd1bc7549d451987a86", 239984),

	// Spring Thing 2012
	ENTRY0("stc12_rocketman", "120405", "a2aac06a0481334279c64ad069865006", 1977066),

	// Spring Thing 2014
	ENTRY0("stc14_mrp", "140406", "35da82199a79e37358de984a5db5d0d1", 174308),
	ENTRY0("stc14_mrp", "140620", "974627f05fc3e1c2260925a78171d73a", 175288),

	// Spring Thing 2015 - Back Garden
	ENTRY0("stc15_dirk", "150416", "b2d0a81eaf7008fa9a1c8654fdc5578d", 362462),

	// Spring Thing 2016
	ENTRY0("stc16_harmonic", "030321", "d3cc582cb71a3a911229b331490cac94", 380416),

	// Spring Thing 2017 - Back Garden
	ENTRY0("stc17_enlightened", "170402", "94d384fe0b5f3f6cead8653b4496d1d3", 218112),

	// Spring Thing 2018 - Main Festival
	ENTRY0("stc18_gopher", "180327", "1af73704d303624fcc88397bdd1cd4a4", 383488),
	ENTRY0("stc18_murderbig", "180403", "45b044f05a4f5b6fbd54a913d33e9dfe", 740794),

	// Spring Thing 2019 - Main Festival
	ENTRY0("stc19_pca", "181128", "8908ecd93be90264935432fc0986dfb2", 100864),

	// Spring Thing 2019 - Back Garden
	ENTRY0("stc19_69105keys", "190401", "4e8c195b1023319a0875ebec71d7a5f5", 318492),

	// Spring Thing 2021 - Main Festival
	ENTRY0("stc21_takethedogout", "210328", "3cc6fd155953f3a5321f6e762ff2cd19", 434112),
	ENTRY0("stc21_takethedogout", "210404", "48879bdbfdebc9e4cc57a8261fc4be96", 657462),

	// Spring Thing 2022 - Main Festival
	ENTRY0("stc22_hypercubic", "220320", "6895d310aa8d3bc7dfb67860ab1bc09e", 263680),
	ENTRY0("stc22_wry", "220407", "a7c52c575f68f1affe727cec8647c572", 460216),

	// Spring Thing 2023 - Main Festival
	ENTRY0("stc23_mariewaits", "230402", "8d6e47991a8eff1925acb14f918eb2dd", 105472),

	// Spring Thing 2024 - Main Festival
	ENTRY0("stc24_zomburbia", "240318", "e0937b22e9c393a5f00ddb8cb8115953", 327432),

	// Interactive Fiction Writing Month 2009
	ENTRY0("ifm_artifice", "090221", "9b10987623485aa736fca5d8d298010a", 106496),
	ENTRY0("ifm_assignone", "090221", "6bc2131c94c2e685f959cd13c6210dad", 138752),
	ENTRY0("ifm_blackberry", "090228", "372bff3cbfbccf49b732d77f299637e1", 141824),
	ENTRY0("ifm_blasphemy", "090223", "d66df23ff590c8a16842138199e11598", 83968),
	ENTRY0("ifm_breakenter", "090221", "a94a659187979018b481f72cc4db301d", 145920),
	ENTRY0("ifm_busstation", "090222", "2addd714375e67c9ef2b9c6ba99f80af", 82432),
	ENTRY0("ifm_busstation", "090302", "ab8e60a033ceb220e1dbc57d65d5d565", 86016),
	ENTRY0("ifm_busstation", "090307", "4156afe0aa8dc1fdb5ba90fa6929ff63", 89600),
	ENTRY0("ifm_busstation", "090316", "53e274376517d00f4c200c6ad218622d", 90624),
	ENTRY0("ifm_checkpoint", "090222", "2b53bdde933b2f8b4b8f2c7dcb8e7ef6", 177152),
	ENTRY0("ifm_colophon", "090223", "76d9b4140f3e404517046a22f5e02917", 83968),
	ENTRY0("ifm_colophon", "090315", "a3b3316ee01dbf17f1cbf2b9c15a934a", 167424),
	ENTRY0("ifm_fraterevan", "090220", "9c79ae105b9cfad69b4ae9f182b85975", 83456),
	ENTRY0("ifm_fraterevan", "090228", "1bedfd0d4a7225845629e2c998f333ab", 94208),
	ENTRY0("ifm_fraterevan", "090308", "fbc31dc689d1057a890a2136f985d105", 96256),
	ENTRY0("ifm_fraterevan", "090315", "e2baa09f68e69696ce086aea5ad382ba", 97792),
	ENTRY0("ifm_grocery", "090301", "fe01df3ad2b3e0f0e9cc5d38bbe3a008", 164864),
	ENTRY0("ifm_heavensent", "090221", "3cd4e4d7eb57ca8312ff6853a21cacea", 157696),
	ENTRY0("ifm_helloworld", "090222", "5f27d1bc38b0434e1375794272ecbd58", 183296),
	ENTRY0("ifm_ifmonth", "090222", "5de63a6c2816f38bac4b12a579965f9c", 143360),
	ENTRY0("ifm_inheritance", "090221", "f0ba0f9771894cdb05832da06bf9eb3a", 90624),
	ENTRY0("ifm_latetrain", "090306", "1a286fbf43377eaecc6821ee4f3ed7f4", 210432),
	ENTRY0("ifm_laura", "090228", "98a415373925c05bcaf776e5621dfee5", 94208),
	ENTRY0("ifm_laura", "090307", "c08952a31d691a03f4ef3b5e5eccd445", 100352),
	ENTRY0("ifm_metrohobo", "090221", "2351bbbd7a78eefaca4cec6ebe13b66a", 82432),
	ENTRY0("ifm_metrohobo", "090228", "6d24341d7b80899912c5770a3a75df2c", 83456),
	ENTRY0("ifm_nomuse", "090309", "237014a04f8af746dcd1a433e94b49e8", 150016),
	ENTRY0("ifm_octopus", "090228", "13f253ea85499e8d38f848a15a32e824", 207360),
	ENTRY0("ifm_office", "090222", "29730d2a73d4f4477ab68fbe761c9270", 144896),
	ENTRY0("ifm_ohmyeldergod", "090221", "e6f101a31324b925e6c959ba59f49976", 82944),
	ENTRY0("ifm_painterly", "090317", "dcff5796a5dc990a6f5e5e5005efafdd", 152576),
	ENTRY0("ifm_perilkeet", "090221", "2427a2b95dbca6d5f5e74f02ffe9eacd", 142336),
	ENTRY0("ifm_rolled", "090221", "b26fcfb0b2659a9b844eb1a239d57bb9", 86016),
	ENTRY0("ifm_seadreams", "090313", "7a46d05667d7eeac045c11d0da47441d", 216064),
	ENTRY0("ifm_searchanswer", "090301/z5", "16944a4b7d4f08cbfc2bb97b9798cc87", 169984),
	ENTRY0("ifm_searchanswer", "090301/zblorb", "fc2e893d078815240ec4baa7739c4bce", 171418),
	ENTRY0("ifm_smugglers", "090306", "0bd2b7d3164ed67edc6a2abb013947b3", 145920),
	ENTRY0("ifm_sparechange", "090307", "73d44c50673cc473dcfae3340923a69e", 198144),
	ENTRY0("ifm_tower", "090222", "409d5cf0ce32b8283ef1948cea70194a", 141824),
	ENTRY0("ifm_tower", "090228", "80fba25e34e5cb55961c5e705aac4928", 147968),
	ENTRY0("ifm_trails", "090221", "e78c8d21edf41ce74c192a450b2ec685", 54272),
	ENTRY0("ifm_upyoursleeve", "090302", "f97b59e9f4b6ba736b59bd6ba0e096c5", 150016),
	ENTRY0("ifm_upyoursleeve", "090307", "f4ce1c0e8f2e0d337f7753d9e4ae3a72", 172032),
	ENTRY0("ifm_winter", "090221", "bcef0fe05ba3e449d0110c5731157598", 143872),

	// Danish games
	ENTRY1("nissen", "171207/v1", "fb28905b7ff93a22d5fe43a18efa90c3", 142848, Common::DA_DNK),
	ENTRY1("nissen", "171207/v2", "c81784afb569b863098e8374dfdd4f32", 142848, Common::DA_DNK),

	// Dutch games
	ENTRY1("adv", "020822", "dff45a0d89ed78a204d49d18341d15ce", 153600, Common::NL_NLD),
	ENTRY1("avontuur", "020822", "998d3f46e9809625acd2cb80098766b3", 147456, Common::NL_NLD),
	ENTRY1("draak", "050819", "eb3f71c53025c70cdfea2d2f5c1fde2e", 88576, Common::NL_NLD),
	ENTRY1("wraakzucht", "000000", "519f81cf271b68ce034798cb2e589a5b", 84992, Common::NL_NLD),
	ENTRY1("zwijnsrug", "060511", "65592e5b731046a519be14fe66c4ac32", 139264, Common::NL_NLD),

	// French games
	ENTRY1("adv", "000531", "50027e3339900e1ca07f72e96396240e", 129536, Common::FR_FRA),
	ENTRY1("agentpaix", "080601", "6d34466f2d5ff45da468c34a6ec9970c", 175104, Common::FR_FRA),
	ENTRY1("ascenseur", "080306", "dadf12d4eb2d9232cdbaae79f8f26494", 96256, Common::FR_FRA),
	ENTRY1("aventure", "000531", "e70b316ec131bf753b89014e05438d47", 157696, Common::FR_FRA),
	ENTRY1("balcon", "070823", "6072d8d30e4e1546e72e851bd724bf78", 173440, Common::FR_FRA),
	ENTRY1("brume", "081230", "8f3633f5774cd4aed22b1b436e864723", 150528, Common::FR_FRA),
	ENTRY1("castelrous", "131119", "c34b46cf40f1954608955b7f2507d546", 390028, Common::FR_FRA),
	ENTRY1("caverne", "180901", "9cbf749eba2c1d33ce7bb125c5526a1a", 20992, Common::FR_FRA),
	ENTRY1("ciafr", "080316", "b75e95796dab8644cd8c32d503826211", 103424, Common::FR_FRA),
	ENTRY1("championbasketball", "180301", "cfebfe60cf4e99e4263a87bf61de4061", 63488, Common::FR_FRA),
	ENTRY1("championbasketball", "180331", "b0b23252575071fc7663564db9379e1c", 64512, Common::FR_FRA),
	ENTRY1("championbasketball", "180904", "92b21e4bc9163727c3dd586056f26fd8", 64000, Common::FR_FRA),
	ENTRY1("championbasketball", "210408", "f74ef19804e6094cf41f6a88458e290e", 64512, Common::FR_FRA),
	ENTRY1("chatiment", "070829", "606eba83ab500cf02d914cc634185ac8", 89600, Common::FR_FRA),
	ENTRY1("chatiment", "290807", "f852781d00d37112b7adfa7cc9462def", 89600, Common::FR_FRA),
	ENTRY1("citeeaux", "061026", "c0ee158b5fbff9440b2197a1769ac42c", 159744, Common::FR_FRA),
	ENTRY1("citronille", "090111", "17a97c92575acb22cb3a33797b5218c2", 131072, Common::FR_FRA),
	ENTRY1("enfant", "081228", "33b8b7c8c7c0c1cdc61e9c02fb34f683", 77312, Common::FR_FRA),
	ENTRY1("escaperoom", "110301", "c039c9749a754f312f4fb4fc45f9dc90", 75264, Common::FR_FRA),
	ENTRY1("espions", "070417/v1", "c592155da4dd1bb360c3c4129a1bd54b", 129536, Common::FR_FRA),
	ENTRY1("espions", "070417/v2", "7e59a5199b12376697deb4694cfac744", 124416, Common::FR_FRA),
	ENTRY1("etoiles", "080211", "19affb3442f83e3cd0d46c245bc81bcc", 69632, Common::FR_FRA),
	ENTRY1("etoiles", "190825", "313ea6b0a4f1978d6ad67180dd3c000b", 72704, Common::FR_FRA),
	ENTRY1("femme", "080103", "5edba9471b92073b15f8f15ebe2890d3", 69632, Common::FR_FRA),
	ENTRY1("fenetre", "200808", "c2d7715e2cec63ee01438818724462ba", 102912, Common::FR_FRA),
	ENTRY1("filaments", "030301", "0aa6d27086b546f6bd4dea84717da6ff", 337920, Common::FR_FRA),
	ENTRY1("grosscarabee", "070826", "d94a570da93e76ab4e74094fea36eb75", 173568, Common::FR_FRA),
	ENTRY1("histoire", "220313", "7b48ca5979ab2eccea797f05f86925e4", 105984, Common::FR_FRA),
	ENTRY1("homelandsecurity", "141204", "94d0f78d7274bb79d4550fecc1e52cd9", 241152, Common::FR_FRA),
	ENTRY1("iletait", "071011", "6c85bace51afec14dd30580785885695", 127488, Common::FR_FRA),
	ENTRY1("initiation", "070820", "3b50132dd7007bfe1b7cbc7147df37ee", 91136, Common::FR_FRA),
	ENTRY1("initiation", "070825", "284c3caa33ec305855f08843668377c2", 92672, Common::FR_FRA),
	ENTRY1("interra", "070726", "11a8ae7f072f4946a8be92630af36336", 98304, Common::FR_FRA),
	ENTRY1("jourdechance", "111206", "269c765ca2db6fbc847bb51d74b19270", 186368, Common::FR_FRA),
	ENTRY1("jourdechance", "130812", "b36c1a11aaf4b01128348fa4469df2cd", 668706, Common::FR_FRA),
	ENTRY1("katana", "070901", "949664ed3decc0349a0a7e36315c80cc", 232960, Common::FR_FRA),
	ENTRY1("katana", "070906", "133b2ba4e38fc5e4565dd8f22582b5a7", 562502, Common::FR_FRA),
	ENTRY1("katana", "090524", "2b8c48e6b81b7d8e84807da09d669329", 190976, Common::FR_FRA),
	ENTRY1("kheper", "070829", "3612cef78d31e80632d6b69b48652306", 94208, Common::FR_FRA),
	ENTRY1("lieuxcommuns_zcode", "070910", "d1032dc1a9635358e0aec29038fdb34e", 253952, Common::FR_FRA),
	ENTRY1("lieuxcommuns_zcode", "090606", "debbdf2966a3f9c8bf3adbde449d8efe", 257024, Common::FR_FRA),
	ENTRY1("lifeonmarsfr", "150925", "f7bf75ba1603e4764f0fc70382044352", 192000, Common::FR_FRA),
	ENTRY1("livraisonexpress", "081228", "d6d2a345af9f5a6d8ffb85d9f93d61a0", 94720, Common::FR_FRA),
	ENTRY1("lupercalia_zcode", "090524", "593326f44f87f77a1f1cd0308c317261", 182784, Common::FR_FRA),
	ENTRY1("lupercalia_zcode", "140825/z8", "66f553e0c7cd676a9273e92a7a63659a", 510976, Common::FR_FRA),
	ENTRY1("lmpsd", "040921", "bb90b4548c5160b10a7b6cbfdb5384a9", 215040, Common::FR_FRA),
	ENTRY1("lmpsd", "080323", "5693d0f10edbbc8b85d6eeed7e079a64", 252928, Common::FR_FRA),
	ENTRY1("magicien", "110328", "7177e5bd97685ad83e4189a65260e39a", 76800, Common::FR_FRA),
	ENTRY1("memelespommes", "070625", "ca70cd49b3f8d7e1d7ad7af5256c9e73", 86528, Common::FR_FRA),
	ENTRY1("mortblue", "140511", "48dcb800d06a763230fad45c3c733d69", 25088, Common::FR_FRA),
	ENTRY1("ombre", "150820", "a3df02e2ecda768542f326480709db02", 137216, Common::FR_FRA),
	ENTRY1("pantinelectrique", "080423", "29bc48d74b998480b99f950f3a419c26", 216064, Common::FR_FRA),
	ENTRY1("paranoiafr", "990528", "b6995a27770514b7940e1d744ebb5246", 92160, Common::FR_FRA),
	ENTRY1("petitgnome_zcode", "061101/z5", "fd58d4c7f9c2ed2ee0aec39c237ffe08", 144384, Common::FR_FRA),
	ENTRY1("petitgnome_zcode", "091225", "8c9c1918c32b90ad84d2f206d1ade717", 173056, Common::FR_FRA),
	ENTRY1("plicploc", "070819", "12fd25cc5a77fc408eb0661562bd5ab7", 75264, Common::FR_FRA),
	ENTRY1("princesse", "131028", "16cdf0b604c538e12de37b4c1e806e79", 124928, Common::FR_FRA),
	ENTRY1("princesse", "090216", "1c9e9dea744d7faf631ec54646b8f538", 133632, Common::FR_FRA),
	ENTRY1("quetedubaal", "080601", "1ed20eb58cc08f1593062fe880559373", 97280, Common::FR_FRA),
	ENTRY1("rats", "081216", "75959db3776f3323ee4ae153799fa321", 178688, Common::FR_FRA),
	ENTRY1("rats", "100808", "acfccabd9e28775c7b25bd6f1a0dd267", 164864, Common::FR_FRA),
	ENTRY1("recitsdegrandpere", "081228", "bd9b36e7dd22d203a44b17adca7c38b1", 114688, Common::FR_FRA),
	ENTRY1("routedesvins", "081227", "3517ee7d5ca9582c4a0e7aac4f948ddb", 196096, Common::FR_FRA),
	ENTRY1("sarvegne_zcode", "061022/z5", "d68b2a0384ef7fb93401325820fde394", 222720, Common::FR_FRA),
	ENTRY1("sarvegne_zcode", "061101/z5", "8c308818d08d924fdf69250fd5fdb210", 224768, Common::FR_FRA),
	ENTRY1("sdlc", "070917", "c80e6bf2a43340c87272f16bf3e64397", 67584, Common::FR_FRA),
	ENTRY1("sdlc", "190825", "082547e20ca3023379ac93872a9d2229", 71168, Common::FR_FRA),
	ENTRY1("secrets", "170408", "3bf44279ad08fe9b191e1dfc9c3966f8", 95744, Common::FR_FRA),
	ENTRY1("sorciereaubalcon", "070823", "eabc7e17bdfdc214b3e6fb7fe616c69c", 214528, Common::FR_FRA),
	ENTRY1("sorciereaubalcon", "080220", "e6f19e66d9981e35a87eaff228b4de62", 179712, Common::FR_FRA),
	ENTRY1("spoutnik", "140102", "b6f94c786d7041db3b86985f970d7492", 99840, Common::FR_FRA),
	ENTRY1("templedefeu", "070731", "05b8c729867644a2b6c417297dcabea3", 131072, Common::FR_FRA),
	ENTRY1("templenaga", "111218", "20470b0263caad21c9d938bd733c0bd9", 197632, Common::FR_FRA),
	ENTRY1("templenaga", "130812", "75d1826c8dcb7fd41645cb6f726de96e", 469026, Common::FR_FRA),
	ENTRY1("terres", "120116", "d554b81728b35803fb52363ac7dbd946", 175104, Common::FR_FRA),
	ENTRY1("tristam", "211220", "2dc9be4f723166273544c251beca3e5c", 57856, Common::FR_FRA),
	ENTRY1("valleemysterieuse", "140202", "3e6f3bb7faee707a62136fa5b55fb732", 908028, Common::FR_FRA),
	ENTRY1("verdeterre", "140228", "6665bda7c172285e281de2e120dac250", 1385176, Common::FR_FRA),
	ENTRY1("verdeterre", "150805", "a2bd16f0dbf5cae8cf90ce71c4c71fb4", 1402584, Common::FR_FRA),
	ENTRY1("vindaloofr", "021001", "95642b4ec28e36f1e250d343c1342bba", 98816, Common::FR_FRA),

	// French Comp 2005 (French)
	ENTRY1("frc_cercledesgros", "051008", "091a9ab30302eb20d421ccbd0c530439", 128512, Common::FR_FRA),
	ENTRY1("frc_dreamlands", "050908", "79cecc22e3f020a3ccc23912122785d4", 79872, Common::FR_FRA),
	ENTRY1("frc_templedefeu", "080317", "ce4d11a46341d6307f4cd2bb303d1c74", 134656, Common::FR_FRA),

	// French Comp 2006 (French)
	ENTRY1("frc_citeeaux", "061024", "aec85f8500c931d478d43bb25d75cf90", 159232, Common::FR_FRA),
	ENTRY1("frc_sarvegne", "081227", "5f6b489cd12c151e4bf67822a5d22b8a", 239104, Common::FR_FRA),

	// French Comp 2007 (French)
	ENTRY1("frc_heuresduvent", "080220", "5b9470352594bb79edee84ecc0134e4e", 337920, Common::FR_FRA),
	ENTRY1("frc_divinebonace", "070711", "a09271e95d3720d1e8016600b7871906", 103936, Common::FR_FRA),
	ENTRY1("frc_brrr", "070709", "8657c6e47832ce1a1a976fc1107e16fc", 73216, Common::FR_FRA),
	ENTRY1("frc_ilephare_zcode", "071220/z5", "b9a70f4bbebbcdca4c52baa6dbac143f", 102912, Common::FR_FRA),

	// French Comp 2008 (French)
	ENTRY1("frc_brume", "100424", "b661a3f9a2f9a3700c6cfee216063615", 152576, Common::FR_FRA),
	ENTRY0("frc_lettresvolees", "081227", "2590cafc6a7b22b40bd3765c16a0e3d0", 250880),
	ENTRY1("frc_louplachevre", "081121", "473a02087280f01a81e4ee4035249ed5", 98304, Common::FR_FRA),

	// French Comp 2009 (French)
	ENTRY1("frc_catapole", "100114", "e326f5ab2f236791b5b8f122d75bdb7d", 156672, Common::FR_FRA),
	ENTRY1("frc_chambresyrion", "100111", "7db1461b938e392ced1d36747525437d", 123392, Common::FR_FRA),
	ENTRY1("frc_mechants", "100117", "0d580033a9b180484ea6c8bbc2b6106b", 270848, Common::FR_FRA),

	// French Comp 2011 (French)
	ENTRY1("frc_aventureszeus", "120122", "bef1f77e9dfd65d7d39350e3c50b40ce", 91136, Common::FR_FRA),
	ENTRY1("frc_dardenfer", "120822", "fea4fe5607327d8f0c15c88b65ac5d9a", 275968, Common::FR_FRA),
	ENTRY1("frc_terreciel", "120122", "cf24a28bea8bb8f80e006acebbef8e33", 245248, Common::FR_FRA),
	ENTRY1("frc_astrologue", "120122", "4259e9bb2a97152e454ba8feda4f9a04", 198656, Common::FR_FRA),
	ENTRY1("frc_homelandsec", "120124", "ea70cf57b134c45c2234c976d06d1948", 155136, Common::FR_FRA),

	// French Comp 2013 (French)
	ENTRY1("frc_lifeonmarsfr", "140108", "8ef2f4e9dfe63bcce9ff7726cdcca7ea", 163840, Common::FR_FRA),
	ENTRY1("frc_sourcedezig", "140102", "675d4956a12d6a23cf2a7d491e13cbff", 88064, Common::FR_FRA),
	ENTRY1("frc_trac", "131213", "5db3d7270a0b2c7b8dab1672b2f24b8c", 334848, Common::FR_FRA),

	// French Comp 2015 (French)
	ENTRY1("frc_envol", "150201", "5aa75d7b333fdb69c71601d95d2def30", 157696, Common::FR_FRA),

	// French Comp 2016 (French)
	ENTRY1("frc_tipelau", "160104", "e4c8b0d99ceab57393db70b64dcd49c4", 173568, Common::FR_FRA),
	ENTRY1("frc_diamantblanc", "160103", "d349be5b008ee06ad93bb7342d5ffbea", 167936, Common::FR_FRA),

	// French Comp 2018 (French)
	ENTRY1("frc_exil", "180114", "ff9628e84c46771168b5be5a9e23acd9", 107520, Common::FR_FRA),

	// French Comp 2021 (French)
	ENTRY1("frc_donjon", "210110", "923d5ef805cb1ab6ecaef82e35750b7f", 201728, Common::FR_FRA),
	ENTRY1("frc_stationspatial", "210110", "6b9e23599bb921aaf4fc744d80e193f5", 130048, Common::FR_FRA),

	// German games
	ENTRY1("abenteuer", "990712", "f849c7a06ebfe21c50819e4222ffee4e", 135680, Common::DE_DEU),
	ENTRY1("adv", "980419", "6a04a6b303f09359155eb23baa201efc", 126464, Common::DE_DEU),
	ENTRY1("bearg", "981201", "59c94a5d3a4f62cfbb4a34d9ac99f747", 139264, Common::DE_DEU),
	ENTRY1("bearg", "990131", "b18427877fdfdaebac36fc3402244ae1", 134656, Common::DE_DEU),
	ENTRY1("bearg", "000326", "b85bc696a58e11f6a0cf72f4cf08ad7c", 134656, Common::DE_DEU),
	ENTRY1("dasspiel", "101229", "dae98c657d7cdd82757b0c841a211027", 505150, Common::DE_DEU),
	ENTRY1("debaron", "041104", "222874a5a0fdfd1f5a1d4df8df3c736f", 81408, Common::DE_DEU),
	ENTRY1("debaron", "050729", "e4e83cae1b826c901776fc6ba6c900da", 122368, Common::DE_DEU),
	ENTRY1("debaron", "051013", "0c0f7ce043b8b83abdefea52448309d9", 262144, Common::DE_DEU),
	ENTRY1("debaron", "060507", "34a4236301eaebe6c5f9897c36516ed7", 323072, Common::DE_DEU),
	ENTRY1("deklinator", "040630", "1ca69e16e8e3e1941eb5c4cd403b2022", 103424, Common::DE_DEU),
	ENTRY1("dermantel", "100913", "81155fc7363968d2a47612d13d12830d", 281564, Common::DE_DEU),
	ENTRY1("diegerechtenricht", "101024", "47f0308a684a569ed6df68dbdc92387e", 439064, Common::DE_DEU),
	ENTRY1("drakdiamanten_zcode", "230707", "8d1c7c9aeaf4ed6e55fb8eda0a7d4c61", 475998, Common::DE_DEU),
	ENTRY1("drakdiamanten_zcode", "240103", "484eead39ed34619a3af8a23e1e94b6e", 476510, Common::DE_DEU),
	ENTRY1("eskmausden", "130714", "71d37a2c11f9c0b085ff1ce06ba488f9", 199168, Common::DE_DEU),
	ENTRY1("halb2", "020523", "d424290a37b5efc67482e4fc9224bc0e", 30208, Common::DE_DEU),
	ENTRY1("herr", "140620", "a6493768373d50096344f4319e7f074a", 208028, Common::DE_DEU),
	ENTRY1("karisma", "130908", "48d6eb07a649a614b037722af9718bf2", 291328, Common::DE_DEU),
	ENTRY1("karisma", "151213", "7ede806c41268f8fc50e6a50b816d71c", 292352, Common::DE_DEU),
	ENTRY1("kleine", "181019", "2c2c6112c51bf47da4b5f3c7301c4729", 590784, Common::DE_DEU),
	ENTRY1("knack", "081215", "b8fa6a55c469cd92d01985933ac093a3", 64388, Common::DE_DEU),
	ENTRY1("mchatton", "120413", "1485309be7aed1ed0ada396fe3eeb1ec", 290768, Common::DE_DEU),
	ENTRY1("nurmengard", "171009", "45a45c3800da76386e74e2ea5ff2d4ec", 250368, Common::DE_DEU),
	ENTRY1("o", "120412", "850ed6092daa2dcc87862404a2e88338", 240424, Common::DE_DEU),
	ENTRY1("schiebefehl", "120412", "c9551891b01c10be5f1aa214be17a0ef", 236544, Common::DE_DEU),
	ENTRY1("seestrasse", "090315", "4739869217c7c79eaddafaf493039277", 192512, Common::DE_DEU),
	ENTRY1("starrider", "040110", "6c42a3e46e29419d9d0e3786117ab94b", 222208, Common::DE_DEU),
	ENTRY1("wasserhasser", "140105/z5", "b87ebdaca633e964a575abd70cb88c5a", 151040, Common::DE_DEU),
	ENTRY1("wasserhasser", "140105/zblorb", "1d332f660aec117a4460d0555a2b30f6", 165336, Common::DE_DEU),
	ENTRY1("whisperedworldta", "090401/Demo", "863627aea7e6095d75d175375072bdef", 146432, Common::DE_DEU),
	ENTRY1("wichtel", "021006", "f52166e02c6bd5e0311145683f415ef5", 88576, Common::DE_DEU),

	// Textfire Grand Prix 2002 (German)
	ENTRY1("tgp_eden", "020401/z5", "cac7c7f917cc93824f41efa96bf77e57", 120320, Common::DE_DEU),
	ENTRY1("tgp_eden", "020401/zblorb", "73a6cf485aa833ca895b0f860493cfce", 221618, Common::DE_DEU),
	ENTRY1("tgp_bewerbung", "020429", "5b2a90b66bfcf4564b37dab92afe846a", 114688, Common::DE_DEU),
	ENTRY1("tgp_seite", "020330", "7f1bcab47897d5bbed1351aea77e6f56", 169984, Common::DE_DEU),

	// Textfire Grand Prix 2003 (German)
	ENTRY1("tgp_linear", "030331", "427dd13d669e139f31011da42cdd6c6c", 116736, Common::DE_DEU),

	// Textfire Grand Prix 2004 (German)
	ENTRY1("tgp_jazteg", "040522", "6635a44223e0017418acdeb0c78a9c7a", 192000, Common::DE_DEU),
	ENTRY1("tgp_spater", "040330", "1dc330438f4f064b5bbc22e3f80d1c2c", 84208, Common::DE_DEU),
	ENTRY1("tgp_unterwelt", "040330", "606d95b5e66ccdeb54febbbf2de7c60c", 92160, Common::DE_DEU),
	ENTRY1("tgp_die5", "200304", "2a2139d806fc179c98cf8633f671559f", 121344, Common::DE_DEU),
	ENTRY1("tgp_dichter", "040222", "ea4afef4907aa5d232fe61168ceca08d", 13808, Common::DE_DEU),

	// Textfire Grand Prix 2005 (German)
	ENTRY1("tgp_kopialbuch", "050330", "3e4b21e39f57ad741fae18b4836e1d64", 151040, Common::DE_DEU),

	// Textfire Grand Prix 2006 (German)
	ENTRY1("tgp_felleisen", "060331", "51b62cb229fde3719ddc616f450ba1e2", 131072, Common::DE_DEU),
	ENTRY1("tgp_pmason", "060329", "7c7ec84f10d40a90f76685558abaf81c", 138240, Common::DE_DEU),

	// Textfire Grand Prix 2010 (German)
	ENTRY1("tgp_hausaufgabe", "100405", "74a0227fcee105fed02e7458ab8a4e51", 199168, Common::DE_DEU),
	ENTRY1("tgp_absturzmomente", "100405", "eac3e7f2a6eff119dd4141dc1af7608e", 283542, Common::DE_DEU),

	// Textfire Grand Prix 2011 (German)
	ENTRY1("tgp_gorgonir", "110330", "7b1f7c22b6fb7aceb630958e19c76e54", 542304, Common::DE_DEU),
	ENTRY1("tgp_ausgerechnet", "110403", "5c4cc7e4689850d0726423cb4d14f6d0", 158720, Common::DE_DEU),

	// IF Grand Prix 2015 (German)
	ENTRY1("tgp_dieakte", "141113", "a4e9bc0f32e66c521e21373ec67e7b8f", 264704, Common::DE_DEU),
	ENTRY1("tgp_dieakte", "150514", "f85adc48c759f62636cb435f2fc7dbee", 279040, Common::DE_DEU),
	ENTRY1("tgp_lilie", "150318", "645bb70a097de033452216d5ef85c8dd", 241256, Common::DE_DEU),

	// IF Grand Prix 2016 (German)
	ENTRY1("tgp_emilia", "160401", "bb5872728bbc705c3dbaee32e2629d7c", 161280, Common::DE_DEU),

	// IF Grand Prix 2022 (German)
	ENTRY1("tgp_schief", "220401", "8e74eeb93f60b63ec4eee8a7db80880b", 515794, Common::DE_DEU),
	ENTRY1("tgp_schief", "220404", "78aaa2553d490488b96bda6abe9a7aa7", 516306, Common::DE_DEU),
	ENTRY1("tgp_dieerstenacht", "220326", "c9a1532c6122c33ac40fd4cf19f41dbb", 104412, Common::DE_DEU),

	// IF Grand Prix 2023 (German)
	ENTRY1("tgp_fischstaebchen", "230401", "6c07ebfdcfe23bfcac98cc2a8fdf2c48", 869242, Common::DE_DEU),
	ENTRY1("tgp_gennorden", "230331", "dcc67e5b97f6aaf5f0c7e90a3c62f128", 208896, Common::DE_DEU),

	// Italian games
	ENTRY1("africanescape", "040110", "8775547da36268fb07c02f439876a682", 95232, Common::IT_ITA),
	ENTRY1("anello", "111537/z5", "24126591b2207f17c2ba532a599a0729", 104960, Common::IT_ITA),
	ENTRY1("anello", "111537/zblorb", "6b56c0ca4cf7610fccad13b2aadf2f87", 133998, Common::IT_ITA),
	ENTRY1("armando", "050429", "b74fb87510b6baa50307c2b63ba108de", 71168, Common::IT_ITA),
	ENTRY1("attentiombra", "020531", "457dbd8041c18716e9da6d0e8966c5bd", 98304, Common::IT_ITA),
	ENTRY1("ayon_zcode", "130730", "da1a2dab91d8ccec4b69d955487e9539", 232960, Common::IT_ITA),
	ENTRY1("ayon_zcode", "170423/z5", "05b083581fcf4ecc5fa3a7161261e1d8", 230400, Common::IT_ITA),
	ENTRY1("ayon_zcode", "200415", "aa86a043654bd0dd0fbb5b131ac1e43c", 198656, Common::IT_ITA),
	ENTRY1("beyondita_zcode", "100115/zblorb", "4b87d3038bdf6df986f44a6fc1d8772c", 349840, Common::IT_ITA),
	ENTRY1("beyondita_zcode", "210810/zblorb", "09912bc978bb88459fa1bb8405f5962a", 353192, Common::IT_ITA),
	ENTRY1("bustadimenticata", "081219", "71b28ea835d7a2bd590b5a72fec3f6a6", 68608, Common::IT_ITA),
	ENTRY1("campusinvad_zcode", "220421/z5", "b81b580a40995fabcb5ccfb76f8e7933", 92672, Common::IT_ITA),
	ENTRY1("casamisteriosa", "180814", "a0098dfe8e0f16ed67c40372daa88bb9", 410624, Common::IT_ITA ),
	ENTRY1("cosmic", "990619", "e33633d44993496892de89bb369e3f91", 146432, Common::IT_ITA),
	ENTRY1("darkiss1ita_zcode", "151120", "347db588b5663ab9be83048430d9f4a0", 156160, Common::IT_ITA),
	ENTRY1("darkiss1ita_zcode", "200425/z5", "a04bdff3ea97bc0fc28f9559fd35ada4", 156160, Common::IT_ITA),
	ENTRY1("darkiss2ita_zcode", "150616", "899316750483830be4f38caf37ba7d7f", 219136, Common::IT_ITA),
	ENTRY1("darkiss2ita_zcode", "180925/z5", "10dc81bfb3a8973e841293abf76fe1b8", 181248, Common::IT_ITA),
	ENTRY1("enigma_zcode", "111115", "6e9390f62c07038d44ab17167093c935", 111616, Common::IT_ITA),
	ENTRY1("enigma_zcode", "181116/z5", "18f8b0c236655a1d22e4f9677cb94958", 110592, Common::IT_ITA),
	ENTRY1("filamit", "030520", "41c7069568ea533a4fef227ca14850d3", 324608, Common::IT_ITA),
	ENTRY1("filamit", "030812", "edde5a37d24b112fb1cf3ff7fb133579", 333312, Common::IT_ITA),
	ENTRY1("flamel", "010503", "02bb0beba5cf37e7a9b6fe3558e20cab", 198656, Common::IT_ITA),
	ENTRY1("fugacropoli_zcode", "170417/z5", "69ab0e17ac70ec1020a031e596c1cc5c", 107520, Common::IT_ITA),
	ENTRY1("giardino_zcode", "150614", "f3244aa61ce6b3a4dd860c315bd24aa7", 124416, Common::IT_ITA),
	ENTRY1("giardino_zcode", "200415/z5", "2fb555596dbd4360a8ab8474330221ce", 123904, Common::IT_ITA),
	ENTRY1("hellosword_ita", "050929", "4ac3e9c0411d4ec87281d31ac7b3763d", 163840, Common::IT_ITA),
	ENTRY1("hellosword_ita", "060113", "a432a8286f9f76dae891699175332237", 166400, Common::IT_ITA),
	ENTRY1("intrappola", "120710", "b6b6127d3555f549376f5b84928a0e2c", 110592, Common::IT_ITA),
	ENTRY1("isolakiller", "070613", "e967c84cca2423e50c90120577886586", 94208, Common::IT_ITA),
	ENTRY1("jibbidu", "081122", "a0b1a308bc73847219e1f2fc3f6b067c", 99328, Common::IT_ITA),
	ENTRY1("kangourou", "130503", "55449579a0ecd73fb788120dd0707bc0", 95744, Common::IT_ITA),
	ENTRY1("kangourou", "130510", "249bd922d1247c0abcaad13234664e50", 95744, Common::IT_ITA),
	ENTRY1("katzrfun", "990605", "54b30a9d8f9ad46e7fc089a1222acb10", 90624, Common::IT_ITA),
	ENTRY1("killer", "160710", "037334568dd51618801bfdb5ff7838dc", 84992, Common::IT_ITA),
	ENTRY1("killer", "160711", "316a450d2a2306df6e769f63744427c6", 83968, Common::IT_ITA),
	ENTRY1("koohinoor", "060717", "867d2f12ae29cd97aadc1a3c6b2e2a2a", 88576, Common::IT_ITA),
	ENTRY1("lazyjones", "040383", "3a4393278ce83f98bbd66a33aa6c6b6c", 115200, Common::IT_ITA),
	ENTRY1("littlefalls_zcode", "050527/z5", "38ef6b7f83a39a0ccb117c7910bbfcf7", 155648, Common::IT_ITA),
	ENTRY1("lucifinanza_zcode", "200529/z5", "5919b2ef8d1956ac50ddb5b595af232d", 118784, Common::IT_ITA),
	ENTRY1("noalpitour", "990524", "6d664ec66b95a27cd0fa0c21cfd07d6a", 98816, Common::IT_ITA),
	ENTRY1("noavventura", "180418", "8c1d557f837896b5989a731a4b87df3e", 217600, Common::IT_ITA),
	ENTRY1("ordinedragone", "190905", "74e249190148fffd0a04b7411b7bad5d", 169984, Common::IT_ITA),
	ENTRY1("pietraluna_zcode", "110106/z8", "08c4d4e1e17d7a7c86a43e280c439294", 457728, Common::IT_ITA),
	ENTRY1("pietraluna_zcode", "110106/zblorb", "28f1d106a70a0f50aee1d2628bc42055", 522076, Common::IT_ITA),
	ENTRY1("pietraluna_zcode", "120206/zblorb", "53c42ccbb23ae4d987f3bcd6cce9875c", 523394, Common::IT_ITA),
	ENTRY1("pietraluna_zcode", "150824", "773550301bd1cdf4c086a47c93ec8e9a", 523394, Common::IT_ITA),
	ENTRY1("oldwest1", "020514", "0182ca4b8153fc90e7be355d1ef51c7e", 157184, Common::IT_ITA),
	ENTRY1("paosqura", "161029", "b18eec0c747b6731daabda7233d96898", 75776, Common::IT_ITA),
	ENTRY1("poesia_zcode", "398874/z5/v1", "c383d29e1237cfc4760b46a618f211d2", 196096, Common::IT_ITA),
	ENTRY1("poesia_zcode", "398874/z5/v2", "feabf26f662e70f4d15ac290b8884e3a", 197632, Common::IT_ITA),
	ENTRY1("roccatuono", "070508", "9d01134168e088864a58eab6757169ec", 147456, Common::IT_ITA),
	ENTRY1("rovo", "080424", "b2f570c667e32ea44c121b44cc6baeda", 113664, Common::IT_ITA),
	ENTRY1("scarafaggio", "070906", "6e980b7108fd4af148308214d6efc6bf", 131584, Common::IT_ITA),
	ENTRY1("sfida", "200425", "12465a5bc24ac29f5f11dbea510a11a1", 83968, Common::IT_ITA),
	ENTRY1("sognosangue_zcode", "150702/z5", "d00baeb8f4cca59ba88b24b6e8b71246", 151552, Common::IT_ITA),
	ENTRY1("sparviero", "071127", "65a760999058a1f8d20514cefca2003c", 139264, Common::IT_ITA),
	ENTRY1("sparviero2", "071213", "49bd0276270c0541b79a25c30d4caf15", 172544, Common::IT_ITA),
	ENTRY1("strega", "081007", "71f9cbdeb334cc1de6fce6e1a53cecf3", 153088, Common::IT_ITA),
	ENTRY1("stregatto_zcode", "200424/z5", "7a4f5ec05579fbd913cac1167e890f5c", 84992, Common::IT_ITA),
	ENTRY1("terkhen", "911906", "01670fb60050c159c7a0d542fbbdd7b4", 285184, Common::IT_ITA),
	ENTRY1("tesla_zcode", "160613/z5", "e2650593a2aadaef8fb9a5f3b8e10c27", 94720, Common::IT_ITA),
	ENTRY1("toyshop", "021014", "258f10e6082cd188440cfcb78e384c2e", 101888, Common::IT_ITA),
	ENTRY1("trincea", "081127", "d227b512ca0a5ce99e0b16ac097c006d", 25088, Common::IT_ITA),
	ENTRY1("villamorgana_zcode", "081014", "6be5dee7a5d1608362083850c9bfc679", 170256, Common::IT_ITA),
	ENTRY1("villamorgana_zcode", "210728", "cd43011f68340e1fa9dc437251276d73", 184880, Common::IT_ITA),
	ENTRY1("vita_zcode", "161803/z5/v1", "67cba2cbc9a46c97e3c641d5a7e5a3e7", 218112, Common::IT_ITA),
	ENTRY1("vita_zcode", "161803/z5/v2", "ae72de7a6dd8a38a6ee625c19bacaaf7", 216064, Common::IT_ITA),
	ENTRY1("volonta_zcode", "150624/z5", "28c90d35de32ddc055694cda2dcbee64", 204800, Common::IT_ITA ),
	ENTRY1("zazie", "990506", "74f0a21352b3f6f52e94309e5a02feaf", 114688, Common::IT_ITA),
	ENTRY1("zazie", "030113", "906b9a3e02b2080ce7f06595c8bdcbb3", 89600, Common::IT_ITA),
	ENTRY1("zenfactorspa", "100524", "22373bcd74d843ce647c4bd7b6a4404b", 288256, Common::IT_ITA),
	ENTRY1("zigamusita_zcode", "160227", "016f92fcf0125765da1ed8b3d8863fab", 98816, Common::IT_ITA),
	ENTRY1("zigamusita_zcode", "200426/z5", "72ef8669029dce42ec1e1f73ef66fa3c", 98816, Common::IT_ITA),
	ENTRY1("zombie", "180601", "aa5956094aa8e909f63679a67fb52b14", 134144, Common::IT_ITA),
	ENTRY1("zombie", "180820", "19a1369039b5226f86323ab19e7aea96", 135168, Common::IT_ITA),
	ENTRY1("zombie", "230620", "265679a116cca1867a8b05d6b7267318", 135680, Common::IT_ITA),
	ENTRY1("zorkita", "000031", "3d85a97ddfc1fb0f6bfbf1cb00b4df7b", 192512, Common::IT_ITA),
	ENTRY1("zorkita", "v6/000031", "be15759f2273cdaf124dbc40436244b0", 192512, Common::IT_ITA),

	// Marmellata d'Avventura 2018 (Italian)
	ENTRY1("ma_lastprigioniero", "180723", "e658aef675c3b44a5027f52b49d53abb", 199168, Common::IT_ITA),
	ENTRY1("ma_skepto", "989484", "b1642bac5df936d0d6e56e141a4fd120", 145408, Common::IT_ITA),
	ENTRY1("ma_dejavu_zcode", "180331/z5", "2fdfccb2539bf6d73ea5b86fe1cb7e81", 116224, Common::IT_ITA),
	ENTRY1("ma_lazystranocaso", "180331", "6d78774d7c8cc30f8bed2e33458e7fc5", 175616, Common::IT_ITA),
	ENTRY1("ma_pilastri", "180330", "ff72f757570e2a9d0675507c05a6bf69", 75776, Common::IT_ITA),

	// Marmellata d'Avventura 2019 (Italian)
	ENTRY1("ma_salagamescastle", "191222", "6285d40140396eb6b45a3900d4e79577", 239616, Common::IT_ITA),
	ENTRY1("ma_tesorosalagames", "191210", "bce5e94aa0667c83a869f9a71774086f", 137216, Common::IT_ITA),
	ENTRY1("ma_lazysalagiochi", "191215", "f8f5030501e6640dbfef11c850967e93", 114176, Common::IT_ITA),
	ENTRY1("ma_5feudi", "191213", "175cdef793bf9316f70714921e178ac3", 82944, Common::IT_ITA),

	// Russian games
	ENTRY1("allroads_rus", "070701", "427693ae57580bbc57f204c50437d3cc", 198656, Common::RU_RUS),
	ENTRY1("bluechairs_rus", "111222", "9d6af2460c034fa9078846180ec31f05", 338944, Common::RU_RUS),
	ENTRY1("metamorphoses_rus", "080518", "bc16740cc10500b0d1e1e8e768da6417", 203776, Common::RU_RUS),
	ENTRY1("photopia_rus", "130531", "8ec6305361537ecca28b40376b28b98f", 271360, Common::RU_RUS),
	ENTRY1("spiderandweb_rus", "091122", "3ea174936bc6d56c3f4d0bd5fa1fe720", 321536, Common::RU_RUS),

	// Slovenian games
	ENTRY1("zenin", "070628", "bda2d35eb0614374d02bae623d3a22ec", 90112, Common::SK_SVK),

	// Spanish games
	ENTRY1("abismo_zcode", "022001", "d99185503ef97dcad3a3bb10d6063b76", 94208, Common::ES_ESP),
	ENTRY1("acuario_zcode", "120122", "b0439b17a37760be2d12579e4fc5cb75", 116224, Common::ES_ESP),
	ENTRY1("adv", "971209", "2c38b40ffbc8c29fff29acbbefa317e8", 126976, Common::ES_ESP),
	ENTRY1("adso", "010806", "260d3c709d4efe5538a1f10e725172c6", 116224, Common::ES_ESP),
	ENTRY1("alicia", "980703", "5070504a35d51bdd7f09c67330170d8c", 65536, Common::ES_ESP),
	ENTRY1("anillo", "990610", "e071a84c1348e49ccd339be6329ea4e0", 75776, Common::ES_ESP),
	ENTRY1("anillo3", "15????", "046c9f3c20f190637ff9d1fd94abbcb4", 96256, Common::ES_ESP),
	ENTRY1("avent", "961111", "7d3f5a62df58d20631f2f38623c26810", 76288, Common::ES_ESP),
	ENTRY1("aventura", "971209", "5bee30fdf0d157186a3336ac2a977913", 128000, Common::ES_ESP),
	ENTRY1("bicho", "000402", "b82fba5bce71304bd2545b3c5a987b3b", 61952, Common::ES_ESP),
	ENTRY1("busqueda", "020219", "82cb634fb362358c5506b7c2d586f0f5", 119296, Common::ES_ESP),
	ENTRY1("byodspa", "150928", "b0ef443c46323adeb80471e0871bf74b", 383488, Common::ES_ESP),
	ENTRY1("casarisa", "140228", "16ff0aba9cf271c0b5432d323af59e61", 33792, Common::ES_ESP),
	ENTRY1("cavernad", "990610", "581b7ac02320a8c95d2c005f9fb89f78", 76800, Common::ES_ESP),
	ENTRY1("corto", "000826/Demo", "452c8fbcde064de1ec26ec18fb092f96", 58880, Common::ES_ESP),
	ENTRY1("corto", "001003/Demo", "b66026cd5d8ed70516fba5c551fcb4c8", 93696, Common::ES_ESP),
	ENTRY1("cronicasparaiso", "100705", "8ae9219d4afb912be81ce225b94c5112", 335024, Common::ES_ESP),
	ENTRY1("cruzado", "990610", "d872429765f5c378b35bbb4cd44d7fba", 65024, Common::ES_ESP),
	ENTRY1("cueva", "150301/z5", "e0a6f6e6949944b7793f2822af687f2f", 74752, Common::ES_ESP),
	ENTRY1("cueva", "150301/zblorb", "fb45c478ca1c30151186a7703a1040fc", 76172, Common::ES_ESP),
	ENTRY1("dagon_zcode", "09????", "69f01d02b13d7d873970848da5da0e6e", 190464, Common::ES_ESP),
	ENTRY1("despert", "980909", "f6c469e0931c9f18f149e1b6da484436", 129536, Common::ES_ESP),
	ENTRY1("despert", "990519", "2d2bb65c166c24f89dc30be8021309b7", 128000, Common::ES_ESP),
	ENTRY1("draculasp", "020709", "8466396cd5c66595fee8803f442e2e88", 100352, Common::ES_ESP),
	ENTRY1("draculasp", "071227", "53865e944daea77afeaf9cb909cfe85a", 101888, Common::ES_ESP),
	ENTRY1("ecss", "020320", "a24165f84beeb69d0b5c9e804f0ae8a1", 83456, Common::ES_ESP),
	ENTRY1("edaylobo", "020714", "5a7733503f33e5c63922e04086fef93b", 79360, Common::ES_ESP),
	ENTRY1("elcontinente", "080402", "6dd46c1e1f58734017f3775b31aac455", 161792, Common::ES_ESP),
	ENTRY1("ellibro", "030923", "33a3b35529ab33713913f8b92cc8ad8a", 109568, Common::ES_ESP),
	ENTRY1("elpuzzle", "00???\?/Corrupt", "4285418449df52fff8192db61969280e", 229976, Common::ES_ESP),
	ENTRY1("encierro", "010101", "083514ce5d9718020083ddd00d85e279", 247808, Common::ES_ESP),
	ENTRY1("ergotdelima_zcode", "170616/z8", "c0b8265d11c2bb2dd6dd62801fb7ed73", 382464, Common::ES_ESP),
	ENTRY1("ergotdelima_zcode", "170616/zblorb", "0bcc4c1c8cc24165cb8f02f17f00682d", 606834, Common::ES_ESP),
	ENTRY1("estacion_zcode", "080126", "2a8d4eb4c600ce76f05f03cf862a9328", 92672, Common::ES_ESP),
	ENTRY1("excessus", "990610", "5bb56f1cf94a376fb34bfaf9c07791f1", 58880, Common::ES_ESP),
	ENTRY1("forrajeo", "010101", "d86123253ae4b35570013dd87e48036a", 108032, Common::ES_ESP),
	ENTRY1("fotopia", "991213", "bb067cca7cd769c20e7bb5dc9ed09c65", 214016, Common::ES_ESP),
	ENTRY1("fotopia", "991220", "8f5264294438c6fa07a4774cae749b47", 214016, Common::ES_ESP),
	ENTRY1("geo", "129110", "b3e2f191f509cd0b3c627a988f578ef0", 130650, Common::ES_ESP),
	ENTRY1("gorron", "001127", "74a82670f409c93607e72860552ddda2", 104960, Common::ES_ESP),
	ENTRY1("gorron", "001205", "639f4ab6b26cc6b6e2023fa848f00d92", 74240, Common::ES_ESP),
	ENTRY1("gorron", "020726", "67a7a86523a72c85b9cc0a0cf730ee75", 80896, Common::ES_ESP),
	ENTRY1("hampa_zcode", "120621/z8", "7db989262feb3c36e5e6b46eeab7447a", 277504, Common::ES_ESP),
	ENTRY1("heredar", "980907", "d63cdbaf4f65a1fd6bca4f1c14317b38", 56832, Common::ES_ESP),
	ENTRY1("heresville", "990610", "6b8c930f5b874e0c5a629b6385b0fb30", 70144, Common::ES_ESP),
	ENTRY1("heresville", "001025", "7d4dfdf7911b926a44c66a355faddf8e", 71168, Common::ES_ESP),
	ENTRY1("i0", "000630", "d43fcdb06a748ea24f2329aba8c5761e", 218624, Common::ES_ESP),
	ENTRY1("i0", "000604", "8a1b35f2ad28a1d279f791417416b1ef", 218624, Common::ES_ESP),
	ENTRY1("imposibl", "000726", "4a42497c47157f11b87059fc41083358", 59904, Common::ES_ESP),
	ENTRY1("imposibl", "000903", "011f062695d82ca67afe6efe8001ab51", 59904, Common::ES_ESP),
	ENTRY1("islamisteriosa", "120113", "56ce52bdf2d9a51786a4b38cf5cc3725", 236544, Common::ES_ESP),
	ENTRY1("juguete", "010529", "28b197d2889a7f6c7c8a00c5d8f6c605", 107520, Common::ES_ESP),
	ENTRY1("laberinto", "081027", "968500b509f1da10153186dfb3d89a37", 164352, Common::ES_ESP),
	ENTRY1("latorre_zcode", "001031", "7b993efc82e58225d06f86eeb4a62812", 91648, Common::ES_ESP),
	ENTRY1("latorre_zcode", "001101", "0dda3de17af3eba86fb81fc1d5c926ea", 92672, Common::ES_ESP),
	ENTRY1("latorre_zcode", "010329", "d4ea29e86a8ca925951224c74a00be0f", 70144, Common::ES_ESP),
	ENTRY1("latorre_zcode", "010329/v1", "f3583ab9b0eaa47d5153133a628a0666", 70144, Common::ES_ESP),
	ENTRY1("latorre_zcode", "010329/v2", "39226e9f683f48e6b99895b9a230a577", 70656, Common::ES_ESP),
	ENTRY1("latorre_zcode", "010329/v3", "498ce02c038595577c56c4c57f6319f9", 70656, Common::ES_ESP),
	ENTRY1("latorre_zcode", "010329/v4", "dfba87a9e0e6f2701d79a37dda9fc36b", 71168, Common::ES_ESP),
	ENTRY1("latorre_zcode", "010329/v5", "5bf372c65c8abb3d2cded008147e5630", 71168, Common::ES_ESP),
	ENTRY1("latorre_zcode", "020530/v1", "b29230110159ad197c88e22e482865bf", 94720, Common::ES_ESP),
	ENTRY1("latorre_zcode", "020530/v2", "4e5fce18a37f427ae22a1804318febc3", 95232, Common::ES_ESP),
	ENTRY1("latorre_zcode", "020530/v3", "ffa4d3d6701b228ee95f626d101b9fcb", 98816, Common::ES_ESP),
	ENTRY1("latorre_zcode", "050702/z5", "e81e82829e35a4d6956ebfc9b883968f", 100352, Common::ES_ESP),
	ENTRY1("laverja", "070416", "283e88358e58bde90c5a00e593642fbb", 64000, Common::ES_ESP),
	ENTRY1("legion", "??????", "c7ee0051f293b520aad4c46e5530f783", 169004, Common::ES_ESP),
	ENTRY1("logicinv", "000809", "0c02dd96b334038619a7a7346ae34a47", 61952, Common::ES_ESP),
	ENTRY1("logicinv", "000909", "e7225635756c1e4a6a2c990bb8709345", 61952, Common::ES_ESP),
	ENTRY1("maquina", "980915", "3229392e8493a2ba08efd3ce53d27ee3", 59392, Common::ES_ESP),
	ENTRY1("marsmenace", "160109", "9d31cc5e05145d355fe7650a723eaa77", 422400, Common::ES_ESP),
	ENTRY1("marsmenace", "160210", "c2804882005d945917d765b32c6d39ec", 427008, Common::ES_ESP),
	ENTRY1("memorias_zcode", "000824", "e0af9b9bbef7ff5f9d1281e32e2db488", 65024, Common::ES_ESP),
	ENTRY1("memorias_zcode", "010325", "91ab4377ff9ced804949d3e854ab431e", 137216, Common::ES_ESP),
	ENTRY1("memorias_zcode", "071119", "c14d4b1b81b8b3c5db7b861d681c5a4f", 228454, Common::ES_ESP),
	ENTRY1("memorias_zcode", "140726", "25881186f8b84b888fcaae67724d1557", 365568, Common::ES_ESP),
	ENTRY1("memorias_zcode", "150220", "6796107900bad430d8531c44f306ddbc", 460312, Common::ES_ESP),
	ENTRY1("mpdroidone", "170820", "ea6bf4230bf6f267f76e191d84fb9804", 353792, Common::ES_ESP),
	ENTRY1("nada", "010924/Corrupt", "66e0ed01d50e7082e56cffda54fd2d6b", 79872, Common::ES_ESP),
	ENTRY1("pantalla", "060205", "c7b85db26b1bf03d9e0cc58bcc89758f", 64512, Common::ES_ESP),
	ENTRY1("paraiso", "000214", "28abb3f35a041ea425b9ec3e93086440", 68096, Common::ES_ESP),
	ENTRY1("perseo", "150428", "5242114f236e71848be5f6c92fdcb6ef", 26624, Common::ES_ESP),
	ENTRY1("peso", "981021", "db6fa2c6d6e9d385e0625f9b66c84f6b", 65536, Common::ES_ESP),
	ENTRY1("playera", "010101/v1", "9866408f548c8606e521075907b33ca7", 80896, Common::ES_ESP),
	ENTRY1("playera", "010101/v2", "314ac4dc83fd548049fe639eef8c782e", 114176, Common::ES_ESP),
	ENTRY1("playera", "021100", "a30d9013d71a6277e2b062fde304ff22", 80384, Common::ES_ESP),
	ENTRY1("premios", "000502", "085f093d0ab40c7a46ed2ca5a8382b86", 71168, Common::ES_ESP),
	ENTRY1("pueblofantasma", "140305", "37284904beae0eedbfb98b17585bcaae", 33792, Common::ES_ESP),
	ENTRY1("pyramid", "150427", "7b0f878f298d131024568d9364cf54e7", 29184, Common::ES_ESP),
	ENTRY1("quenoche", "031015", "0085e1d917110642a700551536770d8d", 101376, Common::ES_ESP),
	ENTRY1("quovadis", "031110/Demo", "fdebdc9b5ff49977ecec03c9d6800e9c", 63488, Common::ES_ESP),
	ENTRY1("raro", "000402", "dcbe2202d09a7f5b7dfd6ffb96438fd4", 58880, Common::ES_ESP),
	ENTRY1("redencionmomificad", "161212", "4e1370baee1a5713a792998f8ff5ce93", 117760, Common::ES_ESP),
	ENTRY1("reflejos", "010101", "7edc3b30022e97978ea93ef5c22edccd", 144384, Common::ES_ESP),
	ENTRY1("regalo", "100104", "8d7ea3a09f39d1d2de103e5117ad3224", 336064, Common::ES_ESP),
	ENTRY1("relojes_zcode", "040509/z5", "da0c9341ef8b15f726a18c22bf78877c", 66048, Common::ES_ESP),
	ENTRY1("roleando", "071221", "09f5aaad79cbb12084241d8d26199ea6", 124416, Common::ES_ESP),
	ENTRY1("saee", "000721", "d37e0681b1bc1ebbf001d1a37cbd355c", 29696, Common::ES_ESP),
	ENTRY1("saee", "010308", "0ad5c5a78ea37c53bb614f4bc6f7754f", 31232, Common::ES_ESP),
	ENTRY1("sgw_zcode", "070807/Demo/z5", "4c9b3dbfe549cfbad3aa193d777ad413", 94720, Common::ES_ESP),
	ENTRY1("sinclairmisterio", "030617/Demo", "ceb3d5b3086b5a1f4f3b6b239cb396a0", 115712, Common::ES_ESP),
	ENTRY1("sinrumbo", "000806", "e483112a43721be59b4519b037868adb", 58368, Common::ES_ESP),
	ENTRY1("toma", "070112", "24b40be7145095b291c60cb02b5310ad", 186368, Common::ES_ESP),
	ENTRY1("tiros", "070112", "731180f56273fc20895236b5e904a33b", 108544, Common::ES_ESP),
	ENTRY1("trapero", "000626", "58aa5c86ee0edd2c0e304b059e1a1ca3", 170496, Common::ES_ESP),
	ENTRY1("tribu", "060599", "65e2958400d47de5adfcf89428367c2d", 154624, Common::ES_ESP),
	ENTRY1("tribu", "000626", "9348691a35515442682a29daa3239f4b", 9728, Common::ES_ESP),
	ENTRY1("tuuli", "180501", "9382d5a2886dd7681203128a183ebae3", 671650, Common::ES_ESP),
	ENTRY1("varenna", "190399", "2b984b69649ff9de6c13438e4fb81172", 174592, Common::ES_ESP),
	ENTRY1("waxworksspa", "140301", "d0de29f4f375f6ce12539f9f26800ae3", 31232, Common::ES_ESP),
	ENTRY1("werewolf", "99????", "40010f97d191c074f55e045c0a780d0f", 126464, Common::ES_ESP),
	ENTRY1("zipi", "990707", "98067b8edc5edadf54c66c4becfa8a3c", 9728, Common::ES_ESP),
	ENTRY1("zna", "000803", "8e6da0f9124591a68d736e3d1036ec98", 59392, Common::ES_ESP),
	ENTRY1("zna", "001122", "d4652457908490465a0a4b17965cc695", 64000, Common::ES_ESP),

	// Ectocomp 2017 - Le Grand Guignol (Spanish)
	ENTRY1("ec17_decolor", "171106", "d6fb4727e8ee20d22dbbe17d5673c878", 104960, Common::ES_ESP),

	// Ectocomp 2022 - Le Grand Guignol (Spanish)
	ENTRY1("ec22_estadop_zcode", "230428", "7da49889125934b46036a89de3e6d3b3", 474158, Common::ES_ESP),

	// Premios Hispanos 2000 (Spanish)
	ENTRY1("ph00_abalanzate", "001116", "d235b2e983f74f6176aea5b1d1418a78", 79872, Common::ES_ESP),
	ENTRY1("ph00_alemanes", "000405", "859f5aaec930da12b42e5dbfe42723e9", 82432, Common::ES_ESP),
	ENTRY1("ph00_aparato", "000428", "46c40135d29e3fda669d974bb1b76b20", 63488, Common::ES_ESP),
	ENTRY1("ph00_aque1", "000206", "c660bbbba9e9d57cc26406a2b3a5ab8d", 85504, Common::ES_ESP),
	ENTRY1("ph00_aque1", "000428", "5d16ddd8030635e10065b7e36ba5f59e", 86528, Common::ES_ESP),
	ENTRY1("ph00_b1", "001126", "b3ae1f03ef51b51aec67ef4adc2f032c", 87552, Common::ES_ESP),
	ENTRY1("ph00_b1", "010411", "b390d3874a900f1bc952678106253395", 119808, Common::ES_ESP),
	ENTRY1("ph00_casi", "000305", "65f4d3c7c96413f1f2f59dae7f83597c", 98304, Common::ES_ESP),
	ENTRY1("ph00_casi", "000311", "d9351b2b336b96f481bc42e222adc3e4", 97792, Common::ES_ESP),
	ENTRY1("ph00_csa", "150201/v1", "28e903c8f759dd27d520a1b3b6c06448", 72192, Common::ES_ESP),
	ENTRY1("ph00_csa", "150201/v2", "157307e1f331491f1ed0defb05c2a42f", 72192, Common::ES_ESP),
	ENTRY1("ph00_csa", "150201/v3", "2efa23d92a10a64196ea6f01dea556c2", 72704, Common::ES_ESP),
	ENTRY1("ph00_cuentoch", "991224", "2b5deed86c0dc5229cd60c4b46788c2b", 77824, Common::ES_ESP),
	ENTRY1("ph00_faro00", "200599", "dac2578e464d19560297946919d9c8fa", 84480, Common::ES_ESP),
	ENTRY1("ph00_fotones", "000814", "1a52fbe4d726febddfff78f1a60a353f", 57856, Common::ES_ESP),
	ENTRY1("ph00_fotones", "001122", "195e147c36e6fe1536db425604424286", 62464, Common::ES_ESP),
	ENTRY1("ph00_gusano", "981010", "8b7400e0ff657ae0c12f86975eb81b17", 60416, Common::ES_ESP),
	ENTRY1("ph00_olvido_zcode", "001124", "392f381ccddd755b437a6ab0c85398a7", 91136, Common::ES_ESP),
	ENTRY1("ph00_olvido_zcode", "001230", "c7673107bf9909890480935b4d10ea24", 91648, Common::ES_ESP),
	ENTRY1("ph00_papi", "000627", "1b04e46fa60b6d53698e55b22cebcd64", 61952, Common::ES_ESP),
	ENTRY1("ph00_senten", "980623", "9a07adb394f9a6a135f26af4608405ee", 71168, Common::ES_ESP),
	ENTRY1("ph00_senten", "991225", "e578cb2626d969bba50d2ccd6d863ade", 90624, Common::ES_ESP),
	ENTRY1("ph00_torre", "000208/v1", "2a5bca50855883c01ce4e7e30c7bd444", 126464, Common::ES_ESP),
	ENTRY1("ph00_torre", "000208/v2", "a132c528864576ef5df1ee46c76a5c61", 126464, Common::ES_ESP),
	ENTRY1("ph00_tokland", "001130", "7f5c796474c250f418a47fa9285e3116", 139776, Common::ES_ESP),

	// Premios Hispanos 2001 (Spanish)
	ENTRY1("ph01_aciegas", "010927", "6825eaa8b9a2cc73293329bfacee1311", 78848, Common::ES_ESP),
	ENTRY1("ph01_celos", "010403", "6f4dc34a02fe5eb872ffe99faa06fb79", 69632, Common::ES_ESP),
	ENTRY1("ph01_ch3ch2oh", "010912", "1a4a689b28c4a17c69e7f8e662a63801", 66048, Common::ES_ESP),
	ENTRY1("ph01_churro", "010924", "fe54d90be488bcc60a122940d8f220fe", 225280, Common::ES_ESP),
	ENTRY1("ph01_churro", "020116", "885c3ffa9a9aeb8518746d69211bff4b", 225792, Common::ES_ESP),
	ENTRY1("ph01_conrumb_zcode", "010328", "f0d6bab3466ee98d08676aea0a2e6d76", 67072, Common::ES_ESP),
	ENTRY1("ph01_lldc", "666777/v1", "024e9465504e1ab7bda9399602102876", 164864, Common::ES_ESP),
	ENTRY1("ph01_lldc", "666777/v2", "80c0bd13d7017ba291d57d5982bc3384", 164864, Common::ES_ESP),
	ENTRY1("ph01_necedad", "010924", "4d29d5db290b39ccb636c2e58a546512", 101888, Common::ES_ESP),
	ENTRY1("ph01_necedad", "010925", "fc25f03b71e39ddca21e2c93607ac660", 101888, Common::ES_ESP),
	ENTRY1("ph01_necedad", "150515", "3f0daa76a913ec8244fc8ace590e24a1", 91648, Common::ES_ESP),
	ENTRY1("ph01_ocaso", "020215/v1", "017f1ee4813271b886347af89b4c917d", 164352, Common::ES_ESP),
	ENTRY1("ph01_ocaso", "020215/v2", "6b47826cde5cb4bb7777bcb735dd76cc", 164352, Common::ES_ESP),
	ENTRY1("ph01_segapark", "010506", "2d02fd8559b885868aac6a4adcc09198", 75776, Common::ES_ESP),

	// Premios Hispanos 2002 (Spanish)
	ENTRY1("ph02_asesinato", "020222", "988085754ca9b41f7ade080d9b9262ff", 79872, Common::ES_ESP),
	ENTRY1("ph02_aveces", "021024", "3e0c409d60bda50121c47094e30cd24c", 81920, Common::ES_ESP),
	ENTRY1("ph02_aveces", "030218", "b4232196474c2dc8ae992779e2f5deae", 80384, Common::ES_ESP),
	ENTRY1("ph02_aveces", "110119", "a4004dcb6db8682cc349282aeabb4309", 79872, Common::ES_ESP),
	ENTRY1("ph02_cv", "020804", "10aa5683d762eb22a513ea5079d9f57f", 76288, Common::ES_ESP),
	ENTRY1("ph02_ddddddcrj", "050616", "1b89e39bfdcf2ddd4675d8a8013746da", 66048, Common::ES_ESP),
	ENTRY1("ph02_declina", "020804", "bb122c716609e7c04e7d5e8812be7e64", 169472, Common::ES_ESP),
	ENTRY1("ph02_jugueteria", "090720", "b6cad6064dbbc4ef5f611db7c8037d6a", 96768, Common::ES_ESP),
	ENTRY1("ph02_ligue", "020923", "f1d3c7e09e76b29daa2ca82dce3d7bca", 91648, Common::ES_ESP),
	ENTRY1("ph02_meeva", "021130", "4d988f4963d14f43cf61c44417be22ae", 135168, Common::ES_ESP),
	ENTRY1("ph02_obituario", "000002", "f53ed22f096c68aa612d7a4994769c2a", 81920, Common::ES_ESP),
	ENTRY1("ph02_osito", "020918", "5a4cd96e65e9f3132ecd7ef2df8ee714", 128512, Common::ES_ESP),
	ENTRY1("ph02_osito", "021002", "7ab4fdbf2a13da9742350cf9f89121e6", 128512, Common::ES_ESP),
	ENTRY1("ph02_pruebafisica", "021115", "4a30167350b7b94a98463da12a798e8d", 77312, Common::ES_ESP),
	ENTRY1("ph02_regente_zcode", "090513", "7a84c3ce7a2df69b2da994aaa13ee788", 82432, Common::ES_ESP),
	ENTRY1("ph02_sombras", "021127", "72b83812567f8a4c9cd523b6a09a9c65", 121856, Common::ES_ESP),
	ENTRY1("ph02_uhogar_zcode", "021225/z5", "45edda9ec6eb400f409681d3f2b052d4", 122368, Common::ES_ESP),

	// Premios Hispanos 2003 (Spanish)
	ENTRY1("ph03_aluzine", "122103", "8d974b6777b04a288db409bfc777bd22", 78336, Common::ES_ESP),
	ENTRY1("ph03_array", "040107", "8a8e81e62d5762329bfa3d5c7a503d87", 74752, Common::ES_ESP),
	ENTRY1("ph03_castillo", "031122", "0c6b8c77106dba58e81223c5d5c1da5f", 95744, Common::ES_ESP),
	ENTRY1("ph03_cubo", "000001", "b403e06541c950598d72e504182eb3fb", 83456, Common::ES_ESP),
	ENTRY1("ph03_domador", "030921", "d7a862901c0e8a75688e813aeb9ccabe", 74752, Common::ES_ESP),
	ENTRY1("ph03_domador", "050712", "a2ee06887398686a1c78aaff78ca9208", 74752, Common::ES_ESP),
	ENTRY1("ph03_domador", "070905", "99babbd53e5cc5f2f3a312615c261b45", 74752, Common::ES_ESP),
	ENTRY1("ph03_dwight_zcode", "150211/z5", "35a5ea3853dd9a232d71e9ba853c49be", 196096, Common::ES_ESP),
	ENTRY1("ph03_libroaburria", "031202", "9267337f8592091ec23eb5b05aa8f17f", 110080, Common::ES_ESP),
	ENTRY1("ph03_libroaburria", "030730", "f3e8860af7d3751d467f085808f1922d", 106496, Common::ES_ESP),
	ENTRY1("ph03_lunasp", "030304", "c54874fa9ced274ba70f3ead2c8bfade", 114688, Common::ES_ESP),
	ENTRY1("ph03_mimoso", "000001", "d563e0682941e3f1518223c44e9eb3ce", 94720, Common::ES_ESP),
	ENTRY1("ph03_mimoso", "000002", "1df07c29b69fa6e1635a0f1af0ec68ba", 104960, Common::ES_ESP),
	ENTRY1("ph03_otrpalab", "010102", "1df1630200c6e9a631bb71af494c7d81", 67072, Common::ES_ESP),
	ENTRY1("ph03_rural", "031104", "ef441ece688cbeef11eef548d0c54aa1", 105472, Common::ES_ESP),

	// Premios Hispanos 2004 (Spanish)
	ENTRY1("ph04_islas", "050531", "3d7cee978d4f69e41e8af1a8ccda2b9d", 369152, Common::ES_ESP),
	ENTRY1("ph04_lamansion", "010103/v1", "332cf78fb9eb0a0a60895bf85b73a7f8", 138752, Common::ES_ESP),
	ENTRY1("ph04_lamansion", "010103/v2", "0acd4655161f834b562b5560353877bd", 138752, Common::ES_ESP),
	ENTRY1("ph04_navidad", "050110", "6fa3ccdcce7b0c45e608f2948a63646d", 102912, Common::ES_ESP),
	ENTRY1("ph04_oculta", "050214", "06340073888cd6850b9d176ed3b1a251", 190464, Common::ES_ESP),
	ENTRY1("ph04_remi", "040801", "8c724781c9356c5c94d2ccfe7dd38aba", 102400, Common::ES_ESP),
	ENTRY1("ph04_remi", "050107", "9c91d2d67a112caa5f51aae4dfc6d4f9", 102912, Common::ES_ESP),

	// Premios Hispanos 2005 (Spanish)
	ENTRY1("ph05_bardo", "050918", "7fbedef94068625973d94d672549f0ab", 156672, Common::ES_ESP),
	ENTRY1("ph05_castilsilenco", "031205", "09ef4bbe2455f592941c817cd17cd865", 77824, Common::ES_ESP),
	ENTRY1("ph05_elprotector", "010103/v1", "3189852634dcc62258c4e3af727bac46", 154624, Common::ES_ESP),
	ENTRY1("ph05_elprotector", "010103/v2", "a25d70bf5ae8c4af53085c4e57b304e4", 154624, Common::ES_ESP),
	ENTRY1("ph05_elprotector", "021225", "45edda9ec6eb400f409681d3f2b052d4", 122368, Common::ES_ESP),
	ENTRY1("ph05_leminscata", "050603", "5a62d55c9a0b2b2bd66deac693e40e35", 69120, Common::ES_ESP),
	ENTRY1("ph05_pozo", "030105", "2a6dfdb3ebde6ac58eb27bba9e037052", 73728, Common::ES_ESP),
	ENTRY1("ph05_romanfredo", "050602", "b29277fc75c8ab3580d7395b9b4a1a90", 73728, Common::ES_ESP),
	ENTRY1("ph05_trono", "051030", "ce8acc82e6fc3de5ba774b6dc4454e44", 180224, Common::ES_ESP),
	ENTRY1("ph05_trono", "060105/z5", "27576046399a460904d06942d534549c", 198656, Common::ES_ESP),
	ENTRY1("ph05_trono", "060105/zblorb", "87bfb28edc44caf30706f3643277ce51", 200052, Common::ES_ESP),

	// Premios Hispanos 2006 (Spanish)
	ENTRY1("ph06_draculasp2", "060331", "eec7853595240864f25c34da9083ca8b", 108032, Common::ES_ESP),
	ENTRY1("ph06_draculasp2", "080819", "4de47380bf5d802f295a487eaf1499c6", 123392, Common::ES_ESP),
	ENTRY1("ph06_elgatocheko", "061106", "3e668bdd0359ea7bdd6953f063ad0aaf", 110080, Common::ES_ESP),
	ENTRY1("ph06_goteras", "010101", "a1333ac5833be019f2c5f21f3b916fa8", 126976, Common::ES_ESP),
	ENTRY1("ph06_goteras", "010102/v1", "892f263e65c00fd92f6e384b2729acbe", 128000, Common::ES_ESP),
	ENTRY1("ph06_goteras", "010102/v2", "89ba901b7ce23ecfea5a433a29836d18", 160768, Common::ES_ESP),
	ENTRY0("ph06_paee_zcode", "000720", "8fb185a1f581a1cc9a2bb78f2f783345", 26112),
	ENTRY1("ph06_regreso", "060704", "21565444255a18e57988d983a283962b", 114176, Common::ES_ESP),
	ENTRY1("ph06_resaca", "060713", "868eb485714fbc34358bff2dfa9335d0", 152576, Common::ES_ESP),

	// Premios Hispanos 2007 (Spanish)
	ENTRY1("ph07_030307", "070303", "1483dca6052bb366ac32ef4901064b11", 143872, Common::ES_ESP),
	ENTRY1("ph07_afuera", "010101", "9ed80d0b530f38cce7a7b2c0f1b6ccd9", 116224, Common::ES_ESP),
	ENTRY1("ph07_aod", "071128", "6e486ac2bc483fab7bb713fa9e954e46", 178176, Common::ES_ESP),
	ENTRY1("ph07_boxman", "071223", "a1c807338a3a5db6a26d3a27004a54cb", 71680, Common::ES_ESP),
	ENTRY1("ph07_boxman", "080205", "384ea944adc4be83396c5368a3ba5ed9", 71680, Common::ES_ESP),
	ENTRY1("ph07_edc_zcode", "070831/z5", "3c4b4c9963fbcf69d858f6d16d9b584b", 126464, Common::ES_ESP),
	ENTRY1("ph07_faro07", "070803", "54fdad7789f03d4c7644835bd295ad1f", 94208, Common::ES_ESP),
	ENTRY1("ph07_htec_zcode", "071222", "64cf677261e13fb9fa6e071c3c864ae0", 222816, Common::ES_ESP),
	ENTRY1("ph07_htec_zcode", "111128", "1e4369aae046805529ca8b490c9e5774", 394904, Common::ES_ESP),
	ENTRY1("ph07_macetas", "010101", "7a20d35db70f537247ed2ef62e0a1f6c", 231424, Common::ES_ESP),
	ENTRY1("ph07_macetas", "010102", "3e987e21093af65c8fce2e458ee3dafb", 234496, Common::ES_ESP),
	ENTRY1("ph07_marcopolo", "120121/z8", "b65137ff4f3e7c8a88164f3729419708", 78848, Common::ES_ESP),
	ENTRY1("ph07_marcopolo", "120121/zblorb", "8bed81e9e1d234f4e0e5e715e9a7f0fb", 80038, Common::ES_ESP),
	ENTRY1("ph07_mausoleo", "071223", "a4356d2f89052328323a3dd2ce069658", 164864, Common::ES_ESP),
	ENTRY1("ph07_umami", "070807", "86a045a71db97910e78ba402b33c5c7d", 80384, Common::ES_ESP),

	// Premios Hispanos 2008 (Spanish)
	ENTRY1("ph08_diana_zcode", "080806", "2b117f98896856713a418bcd782be568", 107008, Common::ES_ESP),
	ENTRY1("ph08_emmy", "080915", "92890016648bbfe1290f71d9a903afc9", 79872, Common::ES_ESP),
	ENTRY1("ph08_gambito", "081202", "6ea6121f1891d46fe8cc957ef8939d68", 235990 , Common::ES_ESP),
	ENTRY1("ph08_mushahierba", "081021", "2d28f401d63f163e5d34115fec07df72", 207240, Common::ES_ESP),
	ENTRY1("ph08_pincoya", "010101", "a453de0f39d10cd9eb3ca028457b2b1a", 167424, Common::ES_ESP),
	ENTRY1("ph08_pincoya", "010102", "bafb201726dd331308cdf34ec1e478cb", 168960, Common::ES_ESP),
	ENTRY1("ph08_pronto", "080720", "a48b81b1c97d6074e3679659f9dce23a", 270826, Common::ES_ESP),
	ENTRY1("ph08_sm1pesadilla", "080430", "79f8cb30a31146e08465acd9038ba7e6", 604046, Common::ES_ESP),
	ENTRY1("ph08_sm2konix", "080505", "c9a4128b4874ec735f77295f590a3108", 74240, Common::ES_ESP),
	ENTRY1("ph08_sm3absolutos", "080516", "9505fa8fc4f4c2d06730ce6c33f0fd43", 270222, Common::ES_ESP),
	ENTRY1("ph08_sm4culpatuya", "080711", "c62a2c1fa6c4e6918f4c9ee7b7bb4b57", 196096, Common::ES_ESP),
	ENTRY1("ph08_sm4culpatuya", "080723/z5", "77cd0cbf40cd6badfa9edb9306108008", 198656, Common::ES_ESP),
	ENTRY1("ph08_sm4culpatuya", "080723/zblorb", "01ef491a2099414817f2a20d0ae78dd2", 536956, Common::ES_ESP),
	ENTRY1("ph08_sonrisas", "081202", "fce16ce85582c2b6157beaace24bf684", 146888, Common::ES_ESP),
	ENTRY1("ph08_venenaverbo", "160516", "0a89f0ad5b3df50ff6f32d961cce2c00", 192512, Common::ES_ESP),
	ENTRY1("ph08_venenaverbo", "160517", "1c7c3ce13621f78e7cf6752a2b0fc58b", 192512, Common::ES_ESP),

	// Premios Hispanos 2009 (Spanish)
	ENTRY1("ph09_amanda", "091110/z5", "11b63cb4c4ca11b86e835c1b00f9c5ae", 132096, Common::ES_ESP),
	ENTRY1("ph09_amanda", "091110/zblorb", "c373f508436b06081cd76039dc17582e", 342504, Common::ES_ESP),
	ENTRY1("ph09_gorbag", "100104", "581e67f731d6b1d0d40bfc38cb531bf9", 199168, Common::ES_ESP),
	ENTRY1("ph09_hhorcus_zcode", "100818", "092849be8d49f1ef509a88a1bc5bbea8", 118262, Common::ES_ESP),
	ENTRY1("ph09_hhorcus_zcode", "110512", "a5ad46532a527efdc80bc1e6638edbe5", 117248, Common::ES_ESP),
	ENTRY1("ph09_kerulen", "090720/z5", "b1b024e24c78e37d424163c5b2a6f1ad", 105984, Common::ES_ESP),
	ENTRY1("ph09_kerulen", "090720/zblorb", "0324283a404d8c6b425e37a05073c61d", 682958, Common::ES_ESP),
	ENTRY1("ph09_lae_zcode", "090721", "f4c316955bbda2ca557de27b1a3605b9", 152576, Common::ES_ESP),
	ENTRY1("ph09_megara", "091204/z5", "94b7019ed62d257344ef39727011c250", 214016, Common::ES_ESP),
	ENTRY1("ph09_megara", "091204/zblorb", "6798f8acb0c0c60d1026a1e9a6469b55", 472716, Common::ES_ESP),
	ENTRY1("ph09_panajo", "010102", "3969bf7b2dd00b342e6c0b4ec797919a", 124416, Common::ES_ESP),
	ENTRY1("ph09_panajo", "010102", "3969bf7b2dd00b342e6c0b4ec797919a", 124416, Common::ES_ESP),
	ENTRY1("ph09_sm6ascenso", "090409/z5", "108dd4e7623634e6ff7ca976118dfa29", 220672, Common::ES_ESP),
	ENTRY1("ph09_sm6ascenso", "090409/zblorb", "352b9bb39f2fff76b409025670169a98", 380434, Common::ES_ESP),
	ENTRY1("ph09_visit_zcode", "091130/z5", "d8e7c7376b11f022e109069e4ad3b22a", 87040, Common::ES_ESP),

	// Premios Hispanos 2010 (Spanish)
	ENTRY1("ph10_azul", "101223/z5", "a9b6daf1a3b3da110d1d17c1587feeef", 260096, Common::ES_ESP),
	ENTRY1("ph10_azul", "101223/zblorb", "f6b9cd8892425e7cb97465baf950b602", 362332, Common::ES_ESP),
	ENTRY1("ph10_lpc_zcode", "101213/z5", "b4e747ef59bf719b2edecf6b3796883f", 110592, Common::ES_ESP),
	ENTRY1("ph10_lpc_zcode", "101213/zblorb", "6bd57d7dd178cc4d3ef895c147e65087", 1154350, Common::ES_ESP),
	ENTRY1("ph10_modusvivendi", "010101", "20795fdfe47bc6f6c519f3fe11e34c17", 234496, Common::ES_ESP),
	ENTRY1("ph10_panico", "101224", "382cbfb32478ffca560884fc0af6cd83", 121856, Common::ES_ESP),
	ENTRY1("ph10_sobrevive", "021127", "f951b4a1540035a90c470fc9016ed39f", 111616, Common::ES_ESP),

	// Swedish games
	ENTRY1("aventyr", "071029", "fff0eb351b418ada4010d56c4298d6ac", 133632, Common::SE_SWE),
	ENTRY1("drakmagi_zcode", "080419", "12739044930fc939b0adf0efd5432713", 98304, Common::SE_SWE),
	ENTRY1("hotellet", "041212", "efb166d12edc19b19dd1d6e99d67800e", 109056, Common::SE_SWE),
	ENTRY1("pangnyheten", "040916", "2049135115dff02b2baf1b7bfb59606c", 185856, Common::SE_SWE),
	ENTRY1("storforsen", "041212", "5e3c06b6b0650f938d2831b2aac98153", 103936, Common::SE_SWE),
	ENTRY1("stuga", "090712/z5", "c9be7126299f47dfd395af5b951233ff", 122368, Common::SE_SWE),
	ENTRY1("stuga", "090712/zblorb", "58eef72aada351e7e059202ab00342c4", 588800, Common::SE_SWE),
	ENTRY1("vanyar_zcode", "030613", "abbc2b2fa0f5e922b45a3f8698ba9ad1", 110080, Common::SE_SWE),

	// Playfic games
	ENTRY0("1hour52min15sec", "190319", "9b6b06de44936164c6ebb2cc0e52bbb6", 179712),
	ENTRY0("4horsemenapocalyps", "190424", "fbed65ad9ee62bafab08a0ad12455e90", 158208),
	ENTRY0("5halfminutehallway", "120423", "b78bca213bf44efae1e9b09ac951aa51", 162816),
	ENTRY0("5nightsbarrypalace", "141008", "dc28c1bd00ce02a607bb88533fb07bd6", 156160),
	ENTRY0("8thgradehaleproj", "160209", "9b634799f1814f4d69563c9b52842df5", 159744),
	ENTRY0("221bbakerstreet", "161229", "514676fe1b40fc8e3d15abc334280ffc", 158720),
	ENTRY0("721acaroline", "120227", "c1173cfe7148ba30a496bfa7493e97db", 160768),
	ENTRY0("721acaroline", "120302", "23d5a14fa4efe607cf537e8bbbc9c56d", 163328),
	ENTRY0("1012staffordave", "150317", "d1f3bc06f30142640166b860389a0b08", 159744),
	ENTRY0("1156chapelstreet", "200909", "7fe83bfe8854e27a4308691f04004f37", 167424),
	ENTRY0("1948", "180824", "013228f886e7bb903f5cdca95771e234", 164352),
	ENTRY0("1984", "121027", "e30b3374275d2d4a256f1c06b388038f", 175616),
	ENTRY0("1997experiment", "240410", "a4ba6a4d380f959112c942afd26d6e18", 183808),
	ENTRY0("2084", "140609", "9d84e17d4a3b22dba42f96f7dbeaef7f", 164864),
	ENTRY0("2136", "240406", "0a902c0c9d232ac23a3c248db4138808", 250368),
	ENTRY0("a221", "130720", "2d830ad6bdd1fb8f600ddcc393cf0fc0", 160256),
	ENTRY0("abandonedhouse", "231025", "ec5061f9cfe11ba2a55de86705ae6f84", 158720),
	ENTRY0("abandonedlab", "231031", "cb7777878f4f2c6f84511d2c6967a995", 188928),
	ENTRY0("abandonedneigh", "231024", "f669560a3efd1b44570a1969ec872578", 158208),
	ENTRY0("abandonhospital_ms", "130325", "c9585ee3b9efe68548033d0add58d734", 160256),
	ENTRY0("abandonhospital_an", "231025", "2a49b7b647f9d86bbbed0c6582a81ace", 157696),
	ENTRY0("aboringroom", "151031", "a8bd6e8bbe73f5b3b38338ab1d287efa", 164352),
	ENTRY0("abramelin", "140214", "bfb951c88c4d2858981e5ef1d651e9fb", 157184),
	ENTRY0("acadecwars", "160307", "7416ead1a2cff4249a4d97f006263498", 161280),
	ENTRY0("accusations", "151217", "0b1ad637ec492ffc607ee7528a7512f9", 157696),
	ENTRY0("achangeofheart", "160309", "a41579b2d033928352cd1dd8334e84d9", 161280),
	ENTRY0("achangeofheart", "160310", "8c8dc4bc60866a68ba54380598e8496c", 161792),
	ENTRY0("achilles", "120615", "1964436de08743a8dea49318d4ca932e", 184832),
	ENTRY0("aclutteredroom", "120224", "128d341e58e448e4f4f2e596ab1b5842", 158208),
	ENTRY0("acoldfrostymorning", "180102", "85117f65d31aa37a64334a21e5e99438", 160768),
	ENTRY0("aconversation", "120505", "cd27a4cd8035441b01ae2f02ebd8c6de", 164352),
	ENTRY0("adamsplashes", "150602", "57341f04c2a354b5757a14ed513a3c35", 171008),
	ENTRY0("adayforspelling", "170201", "736c191e48e4ed484d77adb5bf602e35", 174080),
	ENTRY0("adayinthealex", "190901", "50b7dec633784cbe9a20a8410d5cc8c9", 156672),
	ENTRY0("adayinthelife", "170129", "89cdfb450aa6289a73afc079142b62a0", 240128),
	ENTRY0("adayintheoffice", "120907", "d4107c0a99c3fd16c593d217ae315b41", 160256),
	ENTRY0("adaylifejeymarlowe", "170614", "5060c7797e2c91f6573619def6960562", 167424),
	ENTRY0("advancedastronomy", "140629", "bc1312bc412a64c0f0a59568c9e054a7", 171520),
	ENTRY0("adventcrowther", "110819", "e5cc4af379f9dbc2eaa380e5d6c12d83", 254464),
	ENTRY0("adventcrowther", "130327", "3bcf081b848b3ae95632211f9aebed63", 293888),
	ENTRY0("adventurapf", "171031", "cabfc5a1cb39e77d420c0eef0c10112b", 169472),
	ENTRY0("adventurenews", "120218", "adfc3d003d0f8ff03504632e9e63867c", 156160),
	ENTRY0("adventurerback", "130829", "f8856ba158e59ca1d0795b05fb08fd27", 161792),
	ENTRY0("adventuresinspace", "170412", "6d4c68931d13b6b8d81f65c837969953", 162816),
	ENTRY0("adventuresww", "120302", "7c7e549abfe5eb149f9619142b391a29", 159744),
	ENTRY0("adwenturesprincess", "140702", "98b9424fa307d7ba80c3ee6afdd66fa1", 160256),
	ENTRY0("afairytaleoddity", "181019", "9be2f22ec0378414696f24f916a99fcb", 195072),
	ENTRY0("aforgottenmurder", "141009", "acef173c0bd52e28c8e1e7936321f54c", 156160),
	ENTRY0("aforgottenworld", "191029", "2422b635d11480637f8732a5995944b1", 161280),
	ENTRY0("againsthingsending", "120227", "c3ac321492aad96262fb2fbefdb28a69", 173056),
	ENTRY0("agame", "121024", "de123e78e92d7b673d02d0a14e2ae93c", 157696),
	ENTRY0("agameaboutyou", "150401", "7d67287ac3530374b10a3f2d9b1d1696", 165888),
	ENTRY0("agameoffiefs", "150906", "3e5ba1c3e195a4354a6e6cc89613da43", 158208),
	ENTRY0("agenerallybadday", "130430", "e1ba15c6d0a6ef4963f31123083416e9", 166400),
	ENTRY0("agescat1stadv", "180227", "43809e0a8cf91eff5515278aaff33fe6", 216064),
	ENTRY0("agrovegame", "130320", "f25a31a105c6a8a66f5abe8f93c6d830", 159744),
	ENTRY0("ahole", "140509", "640cc7046cf263c15371c5314e4806cf", 164352),
	ENTRY0("aholenewworld", "231107", "64f3d8bc3432e0773836a81933aea0bd", 170496),
	ENTRY0("ahorrificadvcomedy", "200803", "898c992e11c939546bb0e153d65814c4", 162304),
	ENTRY0("ahouse", "150112", "cd14b928d05a19cb59096ac46db9a20b", 174592),
	ENTRY0("ahousetour", "200930", "2047b572bd203b6ddcd2d1a7d569002f", 158208),
	ENTRY0("ajourney2paradise", "231029", "ef29992749a7a12cdf1735767a17052f", 158720),
	ENTRY0("aknighthisquest", "131127", "a4033b97e02581d7c765e0968ccbff04", 157184),
	ENTRY0("aknightsquest", "140619", "04ae60dc7a4a86971876dd426ab38d4b", 168960),
	ENTRY0("akonpliayiti", "181201", "d4938cfa164084c6618a59904c9514eb", 157184),
	ENTRY0("alexeivsevilpeps", "190610", "6928aaf0898abf1a46a5f1e6214bbdc5", 218112),
	ENTRY0("aliceintwistedland", "150510", "72e236a6b6c2d4c39e74f903a2c533ff", 163840),
	ENTRY0("alienabductescape", "140603", "f9569b673574fa6cfce124bc9ece6eea", 156672),
	ENTRY0("alienabductescape", "140607", "dbbb6d79336ec63e269651dac42077a1", 160256),
	ENTRY0("alilaboratoryroom", "231027", "46194b8bbee3e3855393fb70472c640b", 158720),
	ENTRY0("allaboard", "150317", "aa6215e7422d04a9587eac99fac5a6d7", 157184),
	ENTRY0("allerton", "130207", "a075f57f845f609291f51bd13de49907", 160256),
	ENTRY0("alleywaypossums", "231025", "ef138542dae64b9d8992eb77462bbca1", 159232),
	ENTRY0("alloneneedscarrot", "121109", "b5cb7a36aeb1bfcf152bab1cbd62e379", 161792),
	ENTRY0("almostadream", "151028", "51ef716ea2715d585f4712838cbd417b", 174080),
	ENTRY1("alone", "130926", "3f6d19c55818166c65049bfd8883bce5", 176128, Common::FR_FRA),
	ENTRY0("alonglayover", "161029", "241588926fb070509bcbf5053742198f", 156672),
	ENTRY0("alostmemory", "180327", "d628dac7863e240710c3f8dac7b2856e", 177152),
	ENTRY0("alternianforest", "170216", "4c6aab15b6c6bc0ad3c9760924218c83", 165376),
	ENTRY0("amatteroftiming", "120331", "064767b9a5852775a1c27ca430eb38ed", 156672),
	ENTRY0("amazetwistypass", "120215", "737279898630f7a081cb2098b8e77549", 156160),
	ENTRY0("ameadow", "150501", "a70c4dd92f1b9a9be3a0a8c431319864", 162816),
	ENTRY0("aminedorm", "240322", "2543747f29fd3d468447147887a4103f", 157696),
	ENTRY0("amnesiapf", "150501", "a0bacf26ed285681dd6a617f5b3656db", 222208),
	ENTRY1("amnesie", "130926", "e0f3c042c250c47479a5a0e0a90c1c2a", 162304, Common::FR_FRA),
	ENTRY0("amurder", "151208", "921252791f1281807f09610aedf23e56", 161280),
	ENTRY0("amodestproposal", "120403", "13db1dd26135f6c7f8b6a164c573d405", 183808),
	ENTRY0("amrnecklace", "120227", "778e87cb6e0aeba92f70aabc4d26f636", 159744),
	ENTRY0("amurdermystery", "160508", "07b0b7c2ae26989bb1159a63d8f5a2fe", 162816),
	ENTRY0("analchemicaladvent", "141121", "bb13f99cf335da4be6219106de7f721c", 176640),
	ENTRY0("anaponthebeach", "170316", "9eb79814304129fe8215226db4dfbd6c", 163328),
	ENTRY0("anartistneeds", "190429", "7a4ff0b66ae61fea9301050206e6c6ca", 158720),
	ENTRY0("angeldustartgal", "120308", "fbba2b1d6b7583a7579a4ac8742cafa3", 157184),
	ENTRY0("animalapocalypse", "231018", "815c9086dfa015566c6d29d6b12e719d", 156160),
	ENTRY0("animalfarm", "120229", "bb5494702a4dfc5d5397bc2509ffc6ad", 164864),
	ENTRY0("annoyathon", "140518", "3fd165c2e435f7a16b8d03a0306af20f", 175616),
	ENTRY0("anormalday", "210903", "cd5fc4eb9546424c464a68f75067edb0", 156160),
	ENTRY0("anotherartgallery", "120309", "61b106a4df9a55b0e5cdeea2352136b8", 157696),
	ENTRY0("anotherolddude", "190517", "31e6c16849aeea4dd91637e4beba9521", 163840),
	ENTRY0("anotsonormaltemple", "230406", "be1d1e97a1f84f2127930c845be82a6c", 168448),
	ENTRY0("ant129finalproject", "210214", "133b642a287bf94d47befa00a680b109", 160256),
	ENTRY0("ant129finalproject", "210307", "9aaba9715701bdd08f16863e12805422", 324588),
	ENTRY1("antidotes", "131023", "39fb3389916675aa2fdb55564f2ff1e3", 177664, Common::FR_FRA),
	ENTRY1("antidotes", "131026", "5f2151b7c0737e4554f32f9a7d5d7ab1", 179712, Common::FR_FRA),
	ENTRY0("aperfectdayforit", "120505", "902bda86104826dae9a05671d1f43341", 159232),
	ENTRY0("aperfectdayforit", "120507", "49c5746ca027bf5ea88aa8e334022139", 177152),
	ENTRY0("apocalypse", "120313", "d90f666879d4da1011f5f2c40da1ef57", 161792),
	ENTRY0("apocalypse", "130217", "6bae55110240838869625d6733012964", 157696),
	ENTRY0("apocalypsech1", "120314", "f1dc926afd68632807f0e4529bbbec94", 167936),
	ENTRY0("apocalypsech1", "120318", "f4079a9f5cf35c47211912388038ef4d", 174080),
	ENTRY0("apocalypsech1", "120324", "b854629d7da06ac3ba035db4dfe0b1bc", 176640),
	ENTRY0("apocalypsech12", "121205", "55f5106fbce03fbe6ab7d7e2638cb930", 196608),
	ENTRY0("apocalypserising", "130806", "64a8bc6c862d4aacdd8e591e5f01832f", 156160),
	ENTRY0("apocalypserising", "201106", "78b9a64db50138e09b60af38b23ee86a", 166400),
	ENTRY0("applemansion", "131127", "fa1c592bf283c1248e6a6a95d5b6dea5", 163840),
	ENTRY0("applestwoapples", "120720", "a1db38b3fe188661d05c82ad268e21cf", 160768),
	ENTRY0("apprenticeschg", "120907", "36e54df8d0424c29bbb95db8fc2705d7", 241664),
	ENTRY0("arainynight", "200329", "6510db05e30c4a7a307714e8c2d67119", 173568),
	ENTRY0("arcade", "230405", "6d9fb09eccbd3e857ac84f6fc0ad294d", 156672),
	ENTRY0("arcadeescape", "181005", "13db4f8811ce1e3251d99b2dfd4c3dc5", 156160),
	ENTRY0("arealbeach", "130426", "95dde5e9cc8f2606786b48201e104fa2", 158208),
	ENTRY0("areaone", "210424", "3362f396ffa9d0a0b8dff74464ac4d04", 157184),
	ENTRY0("arlocompendiumch1", "150311", "ad78d19a66ce9939a2f5dc82d4d7b6ef", 178176),
	ENTRY0("arlocompendiumch1", "201118", "ab70e3b02a23067f9b58821fbd1ad704", 178176),
	ENTRY0("arlocompendiumch2", "150405", "16338725f548cb06382859530f450315", 201216),
	ENTRY0("armorsandbuffs", "160220", "9213225f45f1ed4f4391c408b1f272ad", 159744),
	ENTRY0("aroundthehouse", "131217", "da224e7e5ce7ab121274e127394c183f", 157184),
	ENTRY0("artiststhenewdeal", "160310", "27b548c967d24de9b8ffbc01936ee260", 158208),
	ENTRY0("aseparatepeace", "130306", "232dfaa1058e92e02e61b1ecbfe84bb1", 158208),
	ENTRY0("ash", "160712", "a1fccf87551b30780f17a8371f1db19e", 157184),
	ENTRY0("ashorti7demo", "130912", "4358490dc234ca31a5285f3169568de8", 156160),
	ENTRY0("aslowdayatwork", "190809", "23e369cfdb674779a1995c3970e7c9a6", 162304),
	ENTRY0("assasin", "130119", "9528b8f323293fba075c9e2efa7635ad", 156672),
	ENTRY0("assessmentmatters", "131116", "cdb7a0857d47da1f6a980792de59b8c5", 157184),
	ENTRY0("astrangesdventure", "130320", "68ab1a03ce9764fc81e2e80fa4bcb059", 159744),
	ENTRY0("asurpriseparty", "231031", "d87abcd46298db215fc0a5097eb381f5", 165888),
	ENTRY0("asylumpf", "151217", "e936d97c841267ba7f9a2daad2c1d736", 159232),
	ENTRY0("ataleofbarizan", "161215", "a03de63492f48e844e468849bc11e292", 183808),
	ENTRY0("atestgame", "120328", "4999e9f0e010c0600774254d06b5a988", 156160),
	ENTRY0("atlantic", "200909", "0ed404d81b506f33d2f932e5e00e4ca4", 156160),
	ENTRY0("atob", "141214", "ec5fa2f5b718121beea96dc207c31320", 210944),
	ENTRY0("atomicmeat", "210305", "86980360e3ca8c6073145e55e5a2f006", 161280),
	ENTRY0("atourofthelouvre", "130223", "1a13e54ceef99afee23678909fe28b1d", 157696),
	ENTRY0("atrailhead", "230224", "a3e5c4afa46d15de11bca3176988b64a", 156672),
	ENTRY0("atraingame", "220104", "eb9b2a342df98cbd28d2d6218ff2c97e", 156672),
	ENTRY0("atriptoucla", "130302", "77fc31bd89b659c888bd5babb3d3c9ac", 162304),
	ENTRY0("attackkillerjarod", "121113", "d513f96bc80d24271f15b75f8c974b76", 157184),
	ENTRY0("atthelibrary", "180717", "a49c5f3b0f5049a69ea9d191a9220e46", 156160),
	ENTRY1("audeldesapparences", "131003", "daeacc7f9455724f7d6e1741d73836d3", 157696, Common::FR_FRA),
	ENTRY0("audioguide", "131003", "26e8b0a78fa4049c9261b56796589303", 176640),
	ENTRY1("augh", "120216", "1cb95db2252432116669c974f68bd46d", 157184, Common::FR_FRA),
	ENTRY0("austinhyattlivedie", "131003", "43650250f0134184131cc230278d887d", 157184),
	ENTRY0("averybadjoke", "160229", "75fb2f61a696f45cb819a91bc83275ca", 156672),
	ENTRY0("averyimpstory", "181111", "fb0396b85a1eed350228bbf379c22d05", 159232),
	ENTRY0("averyniceman", "161016", "01dde2022418b95ccfb62c9a6bbdc391", 165376),
	ENTRY0("averysmissingtrain", "180302", "ea42f8c9a05e4237eb54e28d5c043e85", 163328),
	ENTRY0("averystupidgame", "181002", "20bc637207d4dd65da8298f0a90a09cc", 158208),
	ENTRY0("awake", "160218", "1eed820040ecd7ca20970dc2d691bab1", 160768),
	ENTRY0("awakeningpf", "130520", "66e47f1c272b6a94d383ff5a838db231", 163328),
	ENTRY0("awarning", "160608", "de2c2bebc0ff7fbc242ba6dee56e552a", 192512),
	ENTRY0("awarning", "190726", "d956ed0f1239d2ab6af420fdd56a3943", 250880),
	ENTRY0("awarriorcatsmm", "231027", "1c39f5419a167befe3d2a9c6bd99e9dc", 157696),
	ENTRY0("awayout", "231026", "030fc22b028c4e4841f7828ecb27d0f9", 159744),
	ENTRY0("babysteps", "130817", "38a2308b92d7113d24a076b1fe1b6f05", 165376),
	ENTRY0("babytroubles", "191222", "4caabfc9954878501fe3ae8659d41154", 163328),
	ENTRY0("bakerbonanza", "121023", "9adf937f6a68aa8151ede3bd3d816189", 157184),
	ENTRY0("bananaquest", "150329", "2a9046700244c09c0dcc8571612513f6", 162304),
	ENTRY0("barroombrawl_zcode", "160314", "24cd8acebc3ffb08ff13fd08ba9c9e87", 176640),
	ENTRY0("bartholimules", "231025", "0a2e38c871fa3c7b64b35cb4969f8aa3", 157696),
	ENTRY0("base0", "191103", "ad75669f55edd48ae5641e30be88e74b", 224768),
	ENTRY0("bearsadventure", "160925", "f436b157aad302b316a1dba17e0c4382", 157184),
	ENTRY0("beastmode", "231026", "15d481f5180c09b15682f7ff1715c678", 157696),
	ENTRY0("beastmode", "231027", "f37ba14226d4ceb2e2c9c26ad00257ea", 159232),
	ENTRY0("beepboopboopbop", "210207", "25b493c7e60472482b167eac65f81300", 182784),
	ENTRY0("bellsloveforhermom", "210320", "6b4b7bbc443f22a2274d1f75bb667fe2", 164864),
	ENTRY0("bensappartment", "200527", "f7fe0db8c633ef1cb84dd733eb28d8dc", 159744),
	ENTRY0("birthdaycake", "130103", "620a580f9742c96f8eafa5ff53c01f40", 165376),
	ENTRY0("birthdayquestp1", "161120", "36a1fa7708d6672a0878c7dc6390a985", 290816),
	ENTRY0("birthdaysurprise", "230404", "72501d2ddcbabbf927fb613c748f588a", 171008),
	ENTRY0("blankgame", "230411", "ca279fe149df68cf1e858786a8edd595", 174080),
	ENTRY0("bloodlust", "140427", "2477062a53f74ccb22f9810f88363bc9", 225280),
	ENTRY0("breachofcode", "140220", "3f9346f0c2322fe508e5e8d0b12d549a", 207360),
	ENTRY0("breathdragon", "231016", "b5abcd1e20f18bd214b8c0e4788b3b97", 156160),
	ENTRY0("breakunderscorein", "150315", "aba9debf63eea4d75f9bddffbdc63826", 177664),
	ENTRY0("bruh", "221018", "f1d3679e308aacc4ff8810c9bb419823", 159744),
	ENTRY0("busstop", "131011", "3602e1f9014491d9bb0022e13e1605bd", 161792),
	ENTRY0("bymannualsession", "160816", "70283b4d760ab39ecba4e1bdb317df09", 162816),
	ENTRY0("cabbages", "200229", "adc19ce0c954aaf6a5b853cbde10d9db", 214016),
	ENTRY0("campywakegne", "191028", "2e42040a951047f7133de8d0459d52a9", 174592),
	ENTRY0("candyland", "170104", "7a2674e50f9f6a0e1f62dbe61dbc6e39", 158208),
	ENTRY0("cannibals", "140618", "ad115067caf1e2db6ebf83609f10d508", 159232),
	ENTRY0("carcinogenesis", "231024", "7983a4902cb108b79113a53bb7c150f1", 157184),
	ENTRY0("cardinal", "170525", "345ef241a7add6bb5abe79f7e075afbd", 261632),
	ENTRY0("castle", "130320", "a299d7121c77e4cb2c95990f7a51248d", 164864),
	ENTRY0("caveescape", "220522", "3144cfbed0304988cb207e083ecd50dc", 159232),
	ENTRY0("ceremony", "200114", "40506795283f4b9f07e078e8d65e122c", 228352),
	ENTRY0("chaldeabuffet", "230430", "984bcf4f7c943f6776d62efda7d543a2", 219136),
	ENTRY0("cheesequest", "230509", "2d5b22468be765a5b510b951961ed86f", 160256),
	ENTRY0("chipaguccicollar", "181020", "af9e47372373633cb647c80393639c13", 163840),
	ENTRY0("cipande", "140119", "808d36111342e9b9baf6561179a690c4", 218624),
	ENTRY0("courtyardmania", "231025", "9f27df0b30cafbabca169b6cad248a8a", 156672),
	ENTRY0("clarionsun", "150331", "cac1787eb2ab5c07673582b8e1c1170f", 178688),
	ENTRY0("clarionsun", "150430", "5488a2abae0378632f9b06a756a46487", 203264),
	ENTRY0("clarionsun2", "150421", "0e7e8d807dd4fc6539ccd1bf77bdc0a4", 181248),
	ENTRY0("classmurder", "231029", "ae95b728d40209cbd94a81b3706836f4", 158208),
	ENTRY0("clownsinthedark", "230407", "2e991d47fc79bc51a6985cb01944d6d5", 172544),
	ENTRY0("clownsinthedark", "230410", "b358a881d12adeb367bae638f56e3140", 190976),
	ENTRY0("coffequest", "161105", "989dc31f16fd9bac2e04b97a0448346c", 173056),
	ENTRY0("coffeetime", "170214", "fb0ef01b18771ec63a070cd6f58e599c", 161792),
	ENTRY0("coffinofchaos", "130917", "cf7fec8313f7e50d3677a7615a2ddc90", 161280),
	ENTRY0("colbysunusualday", "120228", "65a40fce9c5c740730d7b7386441e37a", 159232),
	ENTRY0("coleandmichael", "160310", "ea143276354a6415d93e64dc397f22d9", 159744),
	ENTRY0("colinexplore1984", "190303", "43b50829ff5da64463ab8828f27b7758", 172544),
	ENTRY0("colonialproject", "130425", "ab50d3c44e30d7d435ff75baea0998bf", 174080),
	ENTRY0("coma", "231216", "dd6a7674ebf19dda4116112d250524f9", 159744),
	ENTRY0("complimentarypean", "130213", "eeb7e01b8ff7dea3fa341863c838e11c", 158208),
	ENTRY0("comprehenszombie", "121007", "68e5a82ee7d268c3b199e76185fa15cc", 166400),
	ENTRY0("concealedpassages", "180112", "bf9074f9fe0990d5eb0c4fa5866ad068", 160256),
	ENTRY0("coverletter", "150814", "c0418c7d7398749bfdaed7163225e3d4", 159232),
	ENTRY0("crashcourse", "221104", "6aa4017c6245a9d194bd843ba1d20da9", 171520),
	ENTRY0("crashlanding", "151023", "b8284d74c316499e66b6669886476f22", 162304),
	ENTRY0("crater308", "171123", "5f9481bbf9aaf5f9490a1164fc889331", 161280),
	ENTRY0("crazytalk", "150330", "6a559707d5a54efe3833b6b1019b1a87", 168448),
	ENTRY0("creativeproject", "150428", "441af1c08f2c245843c1f186166ccdd0", 177664),
	ENTRY0("cruiseshipmystery", "120302", "0f0c4b0177fa4a7b84751ef57acf2aed", 165888),
	ENTRY0("cupcakeadventure", "131111", "7fb70f242f8404ed56fc848fc38de0f0", 158208),
	ENTRY0("damien", "191027", "e04e805b364c006e1a944238fb3d00ad", 246272),
	ENTRY0("dankception", "160414", "ce51694480c6d6a5da5f818ae60718e9", 163328),
	ENTRY0("darenotspeak", "130424", "13a2a4d517e08ba67e4d9ebf75d96953", 161280),
	ENTRY0("dark", "191031", "26ff01c3bc21bd1eecb7ea7dfe102dd7", 164352),
	ENTRY0("darkforest", "221116", "f1806f329d1a07e99946aeb7945aaf69", 157184),
	ENTRY0("datenightmare", "161212", "5ade0d809580fad7e3c36e120708f6ab", 159232),
	ENTRY0("david", "200211", "2ccea0e7a603d8908146ffb899e5e2ba", 157184),
	ENTRY0("dayalpha", "200211", "5df2b37f0c637c717cc7a7aa0bd1d425", 163840),
	ENTRY0("dayoff", "200819", "10091483fd499ce1c1e120630ff477bd", 316416),
	ENTRY0("daythyme", "131203", "17f03892dba7f05c125a17a13d3888e4", 163840),
	ENTRY0("dcblues", "190316", "67ed9ff56611f395a52f63ce6b229195", 157696),
	ENTRY0("deadication", "200815", "eca9763c1f2400a50e3278246ab3bb39", 314880),
	ENTRY0("deadmansgrave", "151101/z8", "456ba87f580ddd98ef1e1314bfc9247a", 213504),
	ENTRY0("deadmansgrave", "151101/zblorb", "85750a3498daff7750b65f298858f175", 547808),
	ENTRY0("deadsprint", "120425", "9a779b2f244b3b7cf29aa8833995bfab", 165888),
	ENTRY0("deadsprint", "120504", "696a91cd3d0b5134f13efe3b9ab9453f", 175616),
	ENTRY0("decisionsdecisions", "130311", "51d651745f08bfc307a75db186efea12", 157184),
	ENTRY1("demiddeleeuwen", "141203", "08fe83440581215f4e2bdc5b7b696bbb", 161792, Common::NL_NLD),
	ENTRY0("demonetization", "191030", "b1c2be9092d15291b6ca49cc4f489297", 158720),
	ENTRY0("demonhunterpf", "131003", "10b71b6db865acd5fb524b6f216b54ab", 160768),
	ENTRY0("deoxiousevent2015", "150918", "dfa22eb222b38ec7f0c3ac09e6f79a17", 158720),
	ENTRY0("depressed", "130607", "6714fccd6ef9239a5bea3c268cc485da", 165888),
	ENTRY0("detectiveconan", "200204", "b819788f4732b0e4615364af731adef6", 160256),
	ENTRY0("diamondcrime", "161208", "e5dd821794e034e20026a3edc7c9ad61", 163328),
	ENTRY1("dilemme", "140706", "97af0c336cda66fa3b82e656f465e3af", 182784, Common::FR_FRA),
	ENTRY0("dingdong", "191027", "b7f128438be940f6e8225b767baf6af2", 178688),
	ENTRY0("disenchantbaycanda", "150530", "5954cd70a4169983ffb8bd67ad864e89", 173056),
	ENTRY0("distractedwizard", "210315", "dc680bc515f5424e1537647a7d8b4318", 172544),
	ENTRY0("doctorwhoadventure", "12022", "a7c89f0250bcefa15c7103ac345f2408", 212992),
	ENTRY0("dogleashes", "150507", "0ab01df94101e1300ea7a94fa0d95c82", 161280),
	ENTRY0("dollhouse", "150618", "b1dc80f67b5dd66dfc460ce24cca0832", 161792),
	ENTRY0("dollhouse", "231018", "1ee508fcdc7dc4ef411b6ef19402bb12", 166400),
	ENTRY0("dontmakeshootback", "120119", "8277a46ca43cd732527ac7c72404a5cc", 181760),
	ENTRY0("dontmakeshootback", "150501", "fcc3af03b1e8395c23bae053e03594ec", 181760),
	ENTRY0("dontpeedarthvader", "151209", "9af079588db70e0fa8897fe8b4135cd7", 158208),
	ENTRY0("donttouchmyhat", "180227", "1c2c9e646051e5dde780eec396dec950", 173056),
	ENTRY0("donttrytoescape", "160416", "4360462fa43d8d8fb24f83284816ea25", 164864),
	ENTRY0("doors", "111104", "9bb1dea9e93a531e526c4f08bb5263c0", 157696),
	ENTRY0("doorstopinvasion", "180305", "855d77f3dd9f0460864c04eee54bc7aa", 221696),
	ENTRY0("downtownswingers", "240319", "bb6d3b1b85d9660b1d55fcd774f83dd1", 162816),
	ENTRY0("dragongems", "231008", "4e88f5e86b43edc3cef5bdf0420faf8c", 161280),
	ENTRY0("dragonquest", "231017", "9a64e31692b7339789c9452c047156d2", 159744),
	ENTRY0("dreampet", "231128", "89f7c86344716d9d4e34e67d11e01ea3", 161280),
	ENTRY0("dressingup", "170525", "fa86f009d4bef00b0b6110a1f7a3b29e", 179712),
	ENTRY0("drunkennight", "120425", "d4a8ba529013992770f823779c2c272f", 164352),
	ENTRY0("drunkennight", "120503", "af3843259f835d8b269c8d8a0df41c4a", 169472),
	ENTRY0("dryingup", "180315", "e51d06f5feb08417fce008875d538737", 172032),
	ENTRY0("duplicatebiopunkif", "140413", "874cccc9d447600e7382b7bebcf2ffd4", 173056),
	ENTRY0("eastereggs", "170424", "f931e395f44aec197968ee22f1c3d777", 167424),
	ENTRY0("eateverything", "170613", "a23f9ae7b0ee866fdd7a6cea99babfca", 166912),
	ENTRY0("echoesepoch", "231113", "342704ab550e36c3cdee5296566d15ab", 245760),
	ENTRY0("edu5cedtaskat2b", "150716", "ab5f5eeaa75c2016c007f1d27dd04651", 237056),
	ENTRY0("embassy", "130905", "d58651b994cf65f401de8249fa18602b", 226816),
	ENTRY0("emptypalace", "150505", "1b8ab31a8c2e7a22a8b27316b36b11dd", 214016),
	ENTRY0("endandwin", "231025", "687a359699efa21dfa77b3a6bb51af9e", 156672),
	ENTRY0("endinggamedeath", "231017", "772aa20164ac69c9bb35b411395ae93c", 156672),
	ENTRY1("entinieblas", "190122", "41381d7d84a0f111f7011e2290c76089", 178688, Common::ES_ESP),
	ENTRY0("endofdays", "170315", "ab43f07257b6ba4a46ba056823c283db", 173056),
	ENTRY0("endofdays", "170319", "5b2e4ef77cace8b8b1c675514e1b07bf", 179712),
	ENTRY0("environmentaladv", "170122", "47e08b74701c8db5bbacc1fd509637a3", 157696),
	ENTRY0("envstewardship", "180123", "99e40cc3cb4727ff08f5c6c684c84011", 162304),
	ENTRY0("epicdragonbattle3k", "190514", "f415f4b03431186e906b2893d4c9b10d", 214528),
	ENTRY0("epicfreakout", "120504", "da8fd3f4650fed7678bb50463137fe80", 165888),
	ENTRY0("escapechucky", "181018", "1f61675468a57245916243f8053346dd", 177152),
	ENTRY0("escapeemmalam", "220406", "be1f01557326695162eb99230046d4ed", 163328),
	ENTRY0("escapeforniteisle", "181002", "948a08c7a7ae700e3fdab76c728e5360", 166400),
	ENTRY0("escapegrimwoods", "221106", "c4092bde15e2893fc758fcc6740a2b90", 189440),
	ENTRY0("escapeloganli", "221018", "170b76ee8e61e0f7b023df41f2a4a0fd", 161280),
	ENTRY0("escapetheartmuseum", "151206", "bf25100f6c9f2f35099946444ad9c5e4", 160256),
	ENTRY0("escapethecolony", "221119", "45d49acdb2ca504534116980bb2772f2", 245760),
	ENTRY0("escapethecolosseum", "150511", "0ee8eb33e4c2b418a5817ffc64036dbb", 162304),
	ENTRY0("escapethehouse", "181027", "d0a2a55977a1e895404e6c0a8eedb496", 159232),
	ENTRY0("escapemcolorroom", "120406", "33e88a71f22eadb5af502770eadcba48", 159232),
	ENTRY0("escapethepyramid", "150505", "5013e387e93e581b1d73cc30e8e87731", 160256),
	ENTRY0("escapethepyramid", "230404", "28cffcc6cd718568b579689180a527b6", 173568),
	ENTRY0("escapethepyramid", "230406", "2f7b3aa68e37e5cbba8e2707fde724c1", 181248),
	ENTRY0("escapethepyramid", "230411", "b58af5a79ed0d3efd1a919caeb356da0", 178688),
	ENTRY0("escapethepyramid", "200420", "73dab14ee0f8544bb63e7920ba9cdeac", 158208),
	ENTRY0("escapetherooms", "141116", "41951d82082103d889b8e61a4df0760f", 169984),
	ENTRY0("escapethewizcastle", "121111", "16595d8dcdcb4df4d5d8a14aea674b65", 159744),
	ENTRY0("escapetsunami", "231026", "436a8cfe7e9379de29ef01b4fb5d2767", 162816),
	ENTRY0("escapedprisoner", "170505", "ed5c602d1b4ab2867e087e58bed8b10a", 181248),
	ENTRY0("escapingprison", "150622", "a1b5601fbf7f1e917568a50851cd4ffc", 161792),
	ENTRY0("experimentpf", "181019", "9d59860985a25f35ca0dfb24870a9314", 157184),
	ENTRY1("exploratriceperdue", "131013", "62cf8a29f9c0b5e77e5db901cf740338", 188416, Common::FR_FRA),
	ENTRY0("exploreexamine", "231210", "7481430b4e95c267fb17a1174c247848", 166400),
	ENTRY0("exterminsim2014", "131020", "718306165224f53ac8ee68fc774821eb", 167424),
	ENTRY0("exterminsim2014", "140310", "dbbd70a1d28f54efbeb0a7bdc50e5afa", 167424),
	ENTRY0("extremehs", "231028", "303a36ba6a4ae6e15a089257bf630304", 162304),
	ENTRY0("faction11", "120403", "145bd30f5373534fbda13aab436ab87f", 166912),
	ENTRY0("fairyring", "170127", "3ad93d293d28d4429a0fa98c11423c4c", 180224),
	ENTRY0("feverdream", "231107", "f5d2acc96375b7f0af48d1a2c2ce0d5c", 159744),
	ENTRY0("feverdream", "231108", "e0d2ce552cdc96ee3a8362a2a914c904", 176640),
	ENTRY0("findairplanekey", "120413", "559c55349f97f5f75143a3170585731c", 157184),
	ENTRY0("finallythefestival", "181109", "503b862cda59c608223032c032697125", 161280),
	ENTRY0("findjoker", "130916", "53dd23e5ec1e81021de82216cf9d1407", 158720),
	ENTRY0("findmealiger", "130415", "8e1b80c0ba2d358ebd7179c415defdfb", 158720),
	ENTRY0("findmystethoscope", "121107", "bf7ed3eea731242354c843c16d0afd56", 161792),
	ENTRY0("findpurplesteve", "220305", "7aefff2d227dfbfcdabb1b8273494b82", 157696),
	ENTRY0("findthatchinchilla", "180302", "4f9de301077ca1826e7573c98577784d", 214528),
	ENTRY0("findthecookie", "181102", "ac822d25e0c198e3444f6d4d20805ea1", 158208),
	ENTRY0("findyourjuprof", "130410", "ea2f83fc3b596bcb9bd8bea391dbbb26", 158720),
	ENTRY0("findingdora", "181220", "30d67a8de02344af04c413fe5837f7c5", 160256),
	ENTRY0("flosadventure", "211227", "b8f32c109ad47fe0c15ae7893f65d290", 165376),
	ENTRY0("fogwick", "151013", "60d5bb8a64bab45b6117c98c8841521d", 211456),
	ENTRY0("forourclass", "141116", "ebc2ae5321701278da55a5fa4f6dba74", 174080),
	ENTRY0("forgottennight", "150510", "71019bfc099a58aa56c118b76adea75d", 161792),
	ENTRY0("foulplay", "200406", "1e32334a07658319e1013e1eb6debd10", 199680),
	ENTRY0("foxgoosecorn", "130429", "765813a9de73c62e82bfb68239baefc9", 186880),
	ENTRY0("franknovelrecrch1", "191210", "00be041a34d2a781f29734b975ceda8c", 165376),
	ENTRY0("franknovelrecrch2", "191210", "89a01aec70cb07c6ecbe880a9142e77c", 220160),
	ENTRY0("franknovelrecrch3", "191210", "cb9bcefc40a71e0607bb83d01f4ca8a5", 214528),
	ENTRY0("freeparking", "120423", "aae58fc77cd1da8ac90c0fa84b28aec6", 160256),
	ENTRY0("freighterraphus", "191010", "71ab08049a6e941758e8618b3094bb18", 183808),
	ENTRY0("fridayafteroffice", "000831", "090f9ffa40b5ab307a0e733a3de359a1", 165376),
	ENTRY0("friendlydebat", "231116", "7e893c33bc306c9b1cfabf1c2a33cfac", 156672),
	ENTRY0("fromportlandxoxo", "141202", "2ce5bdd1fdb792cfaa0c766aaad54e5f", 180736),
	ENTRY0("fungus", "080220", "8d9bcf00404845bf31d35444db36f288", 174080),
	ENTRY0("futurebookshop", "120611", "d4615b0833770113074a912db4a51ce2", 160768),
	ENTRY0("galaxia", "240407", "75c2fb1f757f0f6c84dbc48e0165796a", 189440),
	ENTRY1("galisancommence", "131122", "38ed3d237f15630e30d390054220604e", 175616, Common::FR_FRA),
	ENTRY1("galisancommence", "140108", "3df8c049294f2ef358500bfd1991e220", 160768, Common::FR_FRA),
	ENTRY0("gamerstereotypes", "120501", "9381d0b68c587d6e1569370858c2f701", 159232),
	ENTRY0("garysgetaway", "231023", "fcf0ffb1a668f56477455aa34eab17ef", 158208),
	ENTRY0("generations", "170222", "59a625a1aba9e1d00365096d7118b571", 159744),
	ENTRY0("generations", "170307", "3b3158b06070f5c05e1f8f00998887c4", 159744),
	ENTRY0("generations", "180306", "8e5d4cb115bfd9c9dfc308332e00d844", 159744),
	ENTRY0("generations", "181002", "63d42a7b13184418e2c0a374c9914f17", 159744),
	ENTRY0("generations", "191009", "56ebbf23c152edd1caa882b9eb47a7d0", 159744),
	ENTRY0("generations", "220322", "c3fc659158b1516c346c206070ed9725", 160256),
	ENTRY0("generationsend", "240321", "e3c1db82daba00f5f7ca22026c69e94e", 160256),
	ENTRY0("genesis", "120219", "198ca37d943a60c59be718a9300c5961", 194560),
	ENTRY0("genesisevil", "231214", "2f6e25cfcbe4165d1a903e252819e07a", 158720),
	ENTRY0("getbackhomewoods", "160526", "d9cd0474e80bb568df4613cca81de15c", 176640),
	ENTRY0("getontrain", "231030", "30edaa28783907bb20ced0189b065128", 158208),
	ENTRY0("getoutalive", "171110", "408645bd678c75dc942eea0fc1b82176", 224768),
	ENTRY0("getoutofthehouse", "130911", "8d3b4554669efb5ee28b59ee11bb098a", 158208),
	ENTRY0("getthatcat", "161206", "db33729fbb5e22abb53f6b1380e30027", 166912),
	ENTRY0("getyourlifeback", "180207", "2173098460829d72f1fdb094a3f6f373", 156160),
	ENTRY0("getyourlifeback", "180301", "5885a9b69f2f4d050c65c6d6869f7d60", 159232),
	ENTRY0("ghosttownpf", "231027", "355c9a7ad5b52461a6ad89a839945e84", 160256),
	ENTRY0("gilberttheelephant", "180305", "00878539308259f3f2c98649f0ad2846", 159744),
	ENTRY0("giornogiovannas", "220419", "9253cc58ac335b1ff7b0044b152b09ad", 159744),
	ENTRY0("gotakeashower", "140408", "5daa21eec3725706a4725f699dd92d31", 159232),
	ENTRY0("goblinmarket", "161208", "6104aa37c6e4f4b139491b66e251a0be", 160256),
	ENTRY0("goddammitcthulhu", "161208", "33ecd01ef6d0ba76115d39c31adaf361", 170496),
	ENTRY0("goldrush", "190308", "32a42de9bf891d91f4ecf417864176d4", 169984),
	ENTRY0("gondoladvent", "230910", "06b29b42c0965937b565c47dd161ffe1", 160256),
	ENTRY0("goodmourning", "191108", "a590d35d82bfc23a1cf26cab56f759a4", 172544),
	ENTRY0("goofyahh", "240406", "49adc444b7fe96eea30d0f48b233bb8e", 237568),
	ENTRY0("gosandwich", "240322", "3a43cc2a8ec56d4315a61771b13db74b", 156672),
	ENTRY0("grandtheftfood", "130314", "c44006878d6b188893ea656faf401f34", 222208),
	ENTRY0("gravitationquand", "160415", "d1c451bb8dd1554f1f0889b0b2ba5c1f", 168448),
	ENTRY0("greatescape", "181031", "fcda838e33581a21534f370105d2c076", 157696),
	ENTRY0("griffynch1", "191025", "a68d300a14baa212e49a38b881c46059", 163328),
	ENTRY0("griffynch2", "191030", "202fa0b1eaa9c6d375ea031634bb6319", 157184),
	ENTRY0("grocerymadness", "130426", "146b3cc72840ef69f64dfd645dab5b96", 159744),
	ENTRY0("gustavosbusiness", "160310", "93b18bb486c39386fb82d300caf10550", 162304),
	ENTRY0("halfdead", "120311", "47e48c1f2cc1aef9d40e930fb5ffd5d0", 302080),
	ENTRY0("halloweenadv", "230912", "532905c5a28eb147b368f1d6b3e05fdb", 158720),
	ENTRY0("halloweenhangover", "171114", "38acccce87ec3ceaa499e17777815419", 161280),
	ENTRY0("hannahinwonderland", "180301", "cd72cd0ab27cb3bfbc3960696cfafa31", 160768),
	ENTRY1("hanze", "160621", "a4688fb12c647296ad80f8edc414862f", 178176, Common::NL_NLD),
	ENTRY0("happybirthdaygon", "150505", "3d186813528ab86c4e5e475865da4397", 165888),
	ENTRY0("happycycling", "200604", "321fe501e8539348afb4fc8a0a3afdcf", 171008),
	ENTRY0("harrowingdefiance", "150608", "e2c535f3eb77fb3c500c43d4d8a7c18f", 173568),
	ENTRY0("hauntedhallway", "231030", "636369addf4bcfe8ab458d954f93e51d", 159232),
	ENTRY0("hauntedmansion", "231019", "c6a0d36c0ce2393ed60f0ee3378df4e3", 157184),
	ENTRY0("hauntedmansion", "231011", "77496c5bc97735bd5efef3ba100e0032", 156160),
	ENTRY0("hauntedhousescape", "240412", "1db1cdf4e79593e920cac3224fe278af", 168960),
	ENTRY0("hellsisland", "120705", "653b7a339405d1782e73d1fa7b34a6bd", 217088),
	ENTRY0("hellsisland", "130704", "ac06011b480ea3b0af84fd687a9163c0", 232960),
	ENTRY0("hellsisland", "170213", "49093307ca6d5fd4267ccf5d57d23bca", 232960),
	ENTRY0("helpsherlock", "170129", "82541cbcc2c314db76fa12141441eb84", 199168),
	ENTRY0("henryhudson", "160523", "d610e9e5ee1bbd3c79e7191687b5422b", 160256),
	ENTRY0("herculesjourney", "120615", "b669d8f89ba804634a683a89c335167a", 197120),
	ENTRY0("highschoolsurvival", "170924", "58ab8e873cdcec4fe4b09c1a4f2399df", 167936),
	ENTRY0("historicalwln", "160601", "c54ac5e7bb207b8f3ffb067cb0bac460", 209408),
	ENTRY0("hogwartshome", "231027", "1c834759f7ab369393a47928522034cc", 157696),
	ENTRY0("honeymoondisaster", "161211", "a5f797bebe1b7ca183d5b907bfe713d6", 163328),
	ENTRY0("hospital", "071125", "4070dd29a806dee00d4c8381eaaa644e", 174080),
	ENTRY0("hotelgrindell", "240517", "2e05aeb08bea5d2699214ba6a8113c84", 175104),
	ENTRY0("housekeys", "231025", "c0dfa5d573d0a17fa729ec1c907c0331", 162304),
	ENTRY0("houseoflafayette", "210321", "e127a713a625c89d385898e6ae0e0387", 163328),
	ENTRY0("housetreasure", "231005", "380738eaabc73a4cb289ebc05cba9ad8", 164864),
	ENTRY0("howdidineverknow", "231012", "b814a97a55d9576149f73ee650aa6d0b", 156672),
	ENTRY0("howdidineverknow", "231017", "b1bc21e22340e8c1237b442a5fa65e7c", 157184),
	ENTRY0("howdidineverknow", "231025", "b67d075d57ff7f958e960ac610cbcb09", 157696),
	ENTRY0("howdidineverknow", "231029", "9a0480d1e858935bd0ed057953061d42", 157696),
	ENTRY0("howlsmovingcastle", "170328", "4bd96faf37812e62b342eb2b82ba10aa", 161280),
	ENTRY0("hungoverinbelfast", "120221", "fa766137a3744e87bed131bb68b44214", 164352),
	ENTRY0("idontevenknow", "190329", "ae7591fe067da922af5a862cb89ff782", 156160),
	ENTRY0("iliketrains", "170322", "57af94b4cce57b2edbda7e0238edcc64", 163328),
	ENTRY0("illusion", "140512", "ca50cc32839df2b4c18f498961f797f6", 175104),
	ENTRY0("imprieteneasca", "151223", "0dcfe52a9da55caf312ec2c695a2f0aa", 158720),
	ENTRY0("imprieteneasca", "160829", "2aa1f29cdf0c0eb371e0d93e268a90e7", 173056),
	ENTRY0("inthebasement", "121016", "a3b0a725c8d4bafd1d2636075cdaee2e", 159744),
	ENTRY0("inception", "150501", "92b244b4965225c23deef6117205606b", 179200),
	ENTRY0("incognizance", "240405", "3d1e8e0b64117b653d2c6d6c909156fb", 217600),
	ENTRY0("inescapableroom", "221116", "bf00d6f06a3f4c0fe0d2d1e020dd2035", 157184),
	ENTRY0("inferno", "171019", "9eaa0e8627d8413cb9796becd8039e77", 162816),
	ENTRY0("infiltrator", "231031", "6b11a22eeb83300a1b89eedb619ebedb", 187904),
	ENTRY0("infiltrator", "231101", "5058260ad0efd431eb5f83c5b4332da3", 191488),
	ENTRY1("infinitydestiny", "140120", "efa50487c5786e83bb24220e09f6cbef", 210432, Common::FR_FRA),
	ENTRY0("inheritancepf", "230504", "6c665834e2c73778bb565070133aca51", 174080),
	ENTRY0("insaneasylum", "160413", "0660fe63ba667f1902f9eff78cc74e33", 224256),
	ENTRY0("insidemindscape", "240412", "ac346fd7ed8463039851decd95eb6099", 175104),
	ENTRY0("isthisacowfantasy", "150331", "f393a76ac85085683057416bb977dd6b", 165888),
	ENTRY0("isiscrisis", "190531", "7abe45046fd54626f15203ba9653381d", 161280),
	ENTRY0("islandhopper", "231031", "ef46c155ef2191f74c0e42c8c0967710", 157696),
	ENTRY0("itsjustyou", "231031", "3d8ece1fea23346afa91f93876fb5457", 182784),
	ENTRY0("jaccuse", "161028", "ea9fcf094725bf4f5464f0f59575da05", 183296),
	ENTRY0("jaccuse", "161104", "c054479a44322d1dda9595361bf7d783", 186368),
	ENTRY0("jailbreak", "220419", "45dd886bf0a5e83d17e95f9a545ae8e3", 157184),
	ENTRY0("jailescape", "230410", "5b29277d8a2e099e25837fe1d1582d8b", 170496),
	ENTRY0("jakesgame", "150604", "611cb3dcd35fbe834754c5fcc5f472dd", 164864),
	ENTRY0("jellen", "090410", "d296cf7e7087b28c1402d3cf27910c33", 166400),
	ENTRY0("joeyskey", "191228", "63d2fa9717de78e4e385ff3f9ab81986", 163328),
	ENTRY0("johntory", "221024", "f246ed6445164332bb3e93b18c33dbca", 157696),
	ENTRY0("journeyofosiris", "200429", "a7b434a7971e7b92b6cc8efceb1c85ae", 159232),
	ENTRY0("journeyofosirisp2", "200429", "1a1eb9d4be3e7c36270d35f38b1c487b", 158208),
	ENTRY0("juliuscaesar", "130514", "1cffed64d60cf1c5af6357202df9203c", 167424),
	ENTRY0("junglesearch", "190306", "033f299e983d1d9d60432cac3c27243a", 167936),
	ENTRY0("jurassicescape", "231026", "79b49ad1bef9bbf0a52ca86895dd758f", 160256),
	ENTRY0("keepingdido", "120516", "56eb7cbeb898cbd892146d9795a91d4b", 332180),
	ENTRY0("keepingdido", "130111", "10f71f6f39eda761bab839ad6c3dfcec", 271872),
	ENTRY0("killerbear", "231025", "1a184325138c5b031de17a1edaabc08b", 157696),
	ENTRY0("killthedisease", "130911", "6941693d3fcba2b216f21ff158523d7b", 157696),
	ENTRY0("kindergarten", "231030", "1d522d1c09fbb34a8268d2bfcd8a3f96", 159744),
	ENTRY0("kingheartbuildraft", "160401", "de55253ef19b0378ba76e1502da2936a", 158720),
	ENTRY0("kingheartdangerous", "160401", "b00dbe2fd8c07167a38a9e57d96f3802", 160768),
	ENTRY0("kingdomofamphibia", "??????", "462ba394189cb5c806b9d7f0dffe3720", 402432),
	ENTRY1("lanote", "141006", "59f9e03b7228a1a8197b87bcee6dc594", 176640, Common::FR_FRA),
	ENTRY1("laboratoriodetalf", "190103", "41d834206975031543e92e2e6b5be108", 226816, Common::ES_ESP),
	ENTRY0("laboratory321", "120406", "99756a98e5002ebf5d3490eeebd8d433", 162304),
	ENTRY0("lacrimosa", "240420", "eabb8982007c2e634332359822c900e3", 200192),
	ENTRY0("lamia", "120504", "a893f08245a6438d71c3f2fefa2e23c7", 175104),
	ENTRY1("laska", "130224", "e0d7673b2142956fa984af87674a1db0", 163840, Common::CS_CZE),
	ENTRY0("lastnight", "130426", "ffa59db9de020687b3512693762339f2", 158208),
	ENTRY0("lepetiteprince", "160915", "1dde1b0a06b9af1586b0a8ae2dcdc2e0", 160768),
	ENTRY0("learnhowtodie", "160325", "4001d0769d90cc227a0e7269e604b268", 160768),
	ENTRY0("legacies", "130107", "4b0b9784f6fcabf4627363b953acb98d", 292864),
	ENTRY0("leosrage", "181221", "bde6bc7c1d89064c620c90539dc12484", 171520),
	ENTRY1("lesgaleriemaudites", "140123", "e09668ea2bc2c17d111b09e3f44d4f79", 204800, Common::FR_FRA),
	ENTRY0("lieblingshause", "190418", "c336a470e777c3714934027ea3a08f73", 175104),
	ENTRY0("lifeaftertesting", "121220", "e1b840667e7ed5cfc3138b10204ba8ed", 168448),
	ENTRY0("lifecat", "231019", "d0a479748fd63dacf16e5792b31db200", 161792),
	ENTRY0("lifegoals", "151217", "5a7fa5b756cf64f0f0e8a5d99e97efc4", 171520),
	ENTRY0("lifepf", "181011", "7188949d1ee7146d756099be2bc6940e", 156160),
	ENTRY0("lifesqpv", "231029", "a56cec8f8859b735328bb6d78bdb4ef7", 166400),
	ENTRY0("limbo", "190915", "1032e246551cf5bc5702efe763bbc7b5", 160768),
	ENTRY0("lines", "191128", "9562edb6f122bd20a8ae396a854a177c", 168448),
	ENTRY0("littleharmoniclab", "130831", "1334a82f500bb381a2f388ab153fc6b7", 289792),
	ENTRY0("littlered", "140413", "b6771652982f3306981c60f0274240b5", 161280),
	ENTRY0("littleredplumbboy", "150623", "46134a27b83ef84dad5fe0d002f14699", 163840),
	ENTRY0("livesoundadv", "231130", "b46c755efd130d8c50d0ce8c6c90bb4b", 161280),
	ENTRY0("ljsawesomeadvent", "120424", "9afc9d33a6867e4adecfe15863bfaeba", 171520),
	ENTRY0("locategoldendonut", "140628", "5535ecd439085f2814cda0667fc8cc0f", 167424),
	ENTRY0("locategoldendonut", "161004", "50fb1bcc5a601071de4b60aae7e8b9a9", 167424),
	ENTRY0("lookingatzombies", "190622", "0a27fa016a8542ef75c6d247d47522b6", 182784),
	ENTRY0("lookingforfood", "181231", "e68af3541354c0498005822f3ad196d7", 161280),
	ENTRY0("lookingforfulton", "210308", "dd6a66b127ed787a53f87e6e0deb34a5", 160256),
	ENTRY0("lostandafraid", "120221", "857a5bac0aebaea78fa9b0f79a2dcb76", 177152),
	ENTRY0("lostaxe", "190419", "8f4711053df1642e51eadac47f31490e", 160768),
	ENTRY0("lostchicken", "231102", "8a59e0facd82307ff8aabbb1b1fb8845", 156160),
	ENTRY0("lostduck", "190529", "bc807f61af3633ec443dc7e4c3dea6ee", 157184),
	ENTRY0("lostintheforest", "140610", "1ed934ecb82473007222221d2fc165bf", 160768),
	ENTRY0("lostinthejungle", "221101", "fdb73034cabab6f1e0df015fd7f56a3a", 164352),
	ENTRY0("lostinthewoods", "121016", "9c8622f85093acdcc0b07ea5c7e2f089", 159744),
	ENTRY0("lostkeys", "231207", "fd0f2e72bf08011921e79f316c8bfabe", 160768),
	ENTRY0("lostsoul", "151212", "dd5458a4fbbfa2cf0db38ba8afbd82da", 175616),
	ENTRY0("lozlanternshadows", "121022", "64529c5de9fccad50c4bd671bb68699d", 157184),
	ENTRY0("lukefeverdream", "231108", "9bad3f88e676efa204779c3ef0846dc8", 168448),
	ENTRY0("lunchbreak", "031010", "81d9b98e6e4cb380d2e263646ec50d89", 182784),
	ENTRY0("macandcheese", "191108", "fbbb25211458d0613d889279e08d135c", 157184),
	ENTRY0("machineprojectadv", "160311", "b5896c5de9aa82ba12be45f7d1690c7b", 161280),
	ENTRY0("magicway", "131122", "370921aa461530768d09b2f607acd15a", 181248),
	ENTRY0("makemeamandwich", "170316", "024c3151dba496772d3271f7edc9749f", 178688),
	ENTRY0("makingcoffee", "210310", "fdcfe1c9651f4cd0c9f46a988c56aeba", 158720),
	ENTRY0("makingmwavepopcorn", "200909", "ee5523481bfcfebcd9e2031f47f35f57", 158720),
	ENTRY0("manvspanda", "121105", "41b22563951f5939c9a538670cb8557a", 156672),
	ENTRY0("manicomio", "140115", "1bbc71d39aed711b6ec53441e5b05cf3", 187392),
	ENTRY0("manor", "120301", "65f31bee337b8d04eeea0665dddeec0f", 163328),
	ENTRY0("mansionmath", "130521", "acc63ca7c8cc1836c45e2bbbd7ca30c4", 160256),
	ENTRY0("maxcrazymom", "231026", "e191c9bd330b7813ca9ba5ba6e7d5c31", 158208),
	ENTRY0("mayantempleescape", "230413", "60856857c8bbd90c149f1dc43d8d3abe", 172544),
	ENTRY0("mcmansion", "230411", "83be8e34b4774189edddafeb780ddf82", 177664),
	ENTRY0("medievaljailbreak", "150915", "bccac2cba92d49388c5d8cc767cb79d8", 159232),
	ENTRY0("meetingtime", "161019", "1b67cf962a388fab8cb7b8bd68baa2d0", 161280),
	ENTRY0("memoriespf", "130531", "a2987bee5d153b6de23f6a7a0e4bcf29", 183296),
	ENTRY0("micasa", "960613", "b7dfec6fb5519b577ad6739f6aa3dd4b", 162304),
	ENTRY1("middeleeuwsdrama", "141209", "e6ed1daa45b50f24e2d711a98e03ab4d", 164864, Common::DE_DEU),
	ENTRY1("middeleeuwsdrama", "141210/v1", "48f09b6bf006f08f6824c2945b8869a4", 163840, Common::DE_DEU),
	ENTRY1("middeleeuwsdrama", "141210/v2", "f4dfb45ab6b081604d5811fbfa9d021a", 166400, Common::DE_DEU),
	ENTRY0("midnightlibrary", "150316", "f86e9d53ed22769d07d85721fc810475", 160768),
	ENTRY0("mirointeractgaming", "140402", "1c80daa50a6363450c46dfec0fbe5aff", 164352),
	ENTRY0("mirointeractgaming", "140403", "b0b3e8e42e2c82f9d593d522d02d8cb8", 157184),
	ENTRY0("misssticklebushs", "120816", "a490f4884139800a21245a4a7f9802ed", 160768),
	ENTRY0("missingpenatschool", "161215", "577f3c9e9492deb2d1f61e213a0185b2", 161792),
	ENTRY0("mistertulkinghorn", "150331", "1e72244dc3b66d3124a7f75bd2dc5b97", 163840),
	ENTRY0("module4computing", "151118", "2382054cb36578e97fbfe285d4e69fc1", 176128),
	ENTRY0("momentsnoticed", "200909", "54d4be17a9f0fc2b9dc617d369addf80", 158720),
	ENTRY1("monhistoiretroptop", "131003", "da33a4e727e69d02cf1d11e43d19719c", 159744, Common::FR_FRA),
	ENTRY1("monhistoiretroptop", "131013", "83943798b9ccba1f9f9e895c3dd3e30e", 178176, Common::FR_FRA),
	ENTRY1("monhistoiretroptop", "131028", "dbdd795facf8cf2587f535a4443ec5ad", 177664, Common::FR_FRA),
	ENTRY0("monalisa", "160420", "586111a22fa6edeb84895db102bd1f01", 164864),
	ENTRY0("mondaymorning", "170323", "ffe73976dab6fa21c9b42ecd44334680", 159744),
	ENTRY0("moneyandwater", "120331", "01addb98277afb9988086a702c416c27", 156160),
	ENTRY0("moonriver", "150411", "0b0a9cb95567d4164f3c7c879efbc96a", 174080),
	ENTRY0("moonriver", "150427", "edef266c05036c71c867a18d3b1ec105", 179712),
	ENTRY0("moonriver", "190425", "f0c66d0bc4e73075017dc23a2d1f6483", 174080),
	ENTRY0("mossybricks", "240412", "9f812c4cc56f52537a51b7857e5054ca", 331264),
	ENTRY0("motel6", "170823", "3ecd8b28b533761658b42eb0fa9277ec", 162816),
	ENTRY0("mrmoore", "231129", "1b5b375143bef8e71c3370cf4ab0f341", 158208),
	ENTRY0("multimodalproject", "150814", "2501e24dca0dfdc52a6a7efe2d85d1e8", 162304),
	ENTRY0("mummiesroom", "130301", "ba225b0b865c5cc60f4d1d1d7b49dee5", 157696),
	ENTRY0("murderinbigapple", "190321", "c2ae16adee4d149eb5ac159e8014fd21", 200192),
	ENTRY0("murderincourtyard", "120314", "0705dd7f52c08eb8c2efba7022d3effd", 159232),
	ENTRY0("murdermansion", "231208", "e388bc206d0f197ea7508e315c459984", 158720),
	ENTRY0("murdermystery", "150813", "18fb3cf3aa776772c1f0728185906b93", 177152),
	ENTRY0("murderspicemansion", "230411", "3ea2c835dca441097f22c1f0fced41b8", 186368),
	ENTRY0("mushrooms", "231029", "c397209e16419987dbcaa00ec2650c3f", 156160),
	ENTRY0("mushrooms", "231031", "a18cc0235f22e16ef78d3ecd61af5daa", 157696),
	ENTRY0("mushrooms", "231101", "65385f5e377d0a1e80c3e4cedc5bd2ce", 164352),
	ENTRY0("myjapanesemyth", "131203", "888127aa7c62b2ecc5cc05dd7cc24a31", 175104),
	ENTRY0("mysterymanorpf", "150223", "8c35e34e246eaf9c69c02a76008b7977", 162304),
	ENTRY0("mysteriousadv", "150622", "bd84375b82ccc0f7d4311a146fe917c9", 159744),
	ENTRY0("mysterysottomarin", "221105", "0b7fe4c9bbde26e3961cf9dad0f6d93e", 183808),
	ENTRY0("nautilisia_zcode", "120511", "a6993f07a35333bcf74396051e9a497f", 323072),
	ENTRY0("nautilisia_zcode", "130422", "aaa73314eca2a1731f5f212c7de11aef", 1796142),
	ENTRY0("needingawee", "071229", "7a6efd20402d9e138512dc9dfad8e1cf", 158720),
	ENTRY0("neithersense", "030412", "b51d0138b51430238a39ac436239aab3", 164864),
	ENTRY0("newdealhitman", "160311", "22c8c6bc8eebbfdc8a9f66faa5b64b1d", 161792),
	ENTRY0("newsheriff", "230927", "d92919097c16d35ded2ad23f7aadbeb8", 164352),
	ENTRY0("nickeldrophalley", "231127", "83d888f930833c2780796c2ce9dc0e69", 225792),
	ENTRY0("nigthbeforedusk", "240411", "1e4feecf258d639e086b9d28f3cd9bb7", 188416),
	ENTRY0("nightshiftz", "141107", "b950c5885139a4bf0b9cfc68b26c8a9e", 160256),
	ENTRY0("notalone", "120506", "dcca5b8db16197993528b77b1294c392", 169472),
	ENTRY0("nowlookwhatyoudone", "131213", "ce8ee36bfbb76cb126e83177f54c9bbf", 158720),
	ENTRY0("october17th", "181015", "deb227e33ba0dc77b2d73fc784a3539f", 217600),
	ENTRY0("odiasvirtreality", "150420", "8945baaa5acbd1a7a0ebe40afecd053a", 228352),
	ENTRY0("odiasvirtreality", "150425", "7288b80a05f8b35b3c3fb697354e18c7", 248832),
	ENTRY0("oneday", "231109", "7931d8d61351c53ee739cf38cebdb393", 157184),
	ENTRY0("onelostraena", "220406", "c828169b2d1321bfb8d8cc27a1cd4ef5", 175616),
	ENTRY0("oneroomandafish", "220518", "1f295e56f2669a7745fd2c4a93066507", 164352),
	ENTRY0("onewayhome", "231106", "7c5971fff05461675b6c421a9262d5d2", 163840),
	ENTRY0("operblackcobra", "160608", "70d15f0d71a6cacb0fd5b8a121e8425e", 171520),
	ENTRY0("opto", "120313", "72fe9c1b474a9c4df8ca7a8e046cb228", 157184),
	ENTRY0("overpoweringdark", "150515", "a60d87786da303f049bc728217a43939", 214016),
	ENTRY0("oxygenbreach", "190304", "685af42ec8aafc0ea8460a62afb9d96e", 168960),
	ENTRY0("palebluelight", "110708", "94ada352a565eeae35738b0ecdd3ecfd", 355538),
	ENTRY0("passiveaggrdragon", "200909", "1f55494a9e4b93093eca7237ee38344e", 163840),
	ENTRY0("peasantscreed", "151207", "89cf68dde5a7a3666ffc79293dbde230", 166400),
	ENTRY0("pedaltothemedal", "200325", "0f1a129e50d18487a153042c9f52923c", 178688),
	ENTRY0("persecution", "231026", "8c0f89da634df8027a1e39cfedcc63de", 162304),
	ENTRY0("piecethingtogether", "150314", "6fd44748805a28aa5d3b3ec8cd0614bf", 158720),
	ENTRY0("piratesofhighseas", "160425", "ca25ee09cf7c09a3db10f2b5b6559473", 196608),
	ENTRY0("pitchblack", "191031", "3db4c5d69f94c259d5e882c9ead0636b", 163840),
	ENTRY0("playwithrocco", "181227", "5b32fdc14dd89d4125da0508d80ff2ed", 157696),
	ENTRY0("playfictutorial", "120302", "70022542f6cdd3b3adc1dd7022b3d952", 163840),
	ENTRY0("ploc", "210513", "c9dc23eba538ee882bd8e00bfa486ab0", 195584),
	ENTRY0("poisoned", "170129", "cba75c344353342c9d338795fabf0cea", 186368),
	ENTRY0("policyhobo", "120227", "037ade0864d0ecda01b4c7c3b0fe6188", 160768),
	ENTRY0("powerkeys", "120624", "37d8409325b58cf384d88ab0c5a45669", 157184),
	ENTRY0("prankken", "000728", "9f83abe0c63ef3ae771aca0d64c05a48", 161792),
	ENTRY0("presidentialrace", "180918", "afa89a1165a0db506aec98c87789f207", 225280),
	ENTRY0("princtextfghturbo", "120916", "eeae786191d6f741f0605aa240856c4a", 299520),
	ENTRY0("prisonbreakpf", "190321", "eabd81867affbcb89cc36f181fbba538", 163328),
	ENTRY0("prisonescape", "150812", "58076ba2217fa52b1b8248c2d88c1e9e", 160256),
	ENTRY0("prisonerdrunkenera", "230411", "02a286d926945b4e212e82b33464b57a", 206848),
	ENTRY0("projectalfa", "231103", "149e806742e1393b16f489a19f6226fb", 172032),
	ENTRY1("projetinform", "140601", "7b3882ea3dd7757a39fd402559a28716", 165888, Common::FR_FRA),
	ENTRY0("puzzleescaperoom", "190116", "efc38b573496f59e489030a5a7a67f7f", 158208),
	ENTRY0("quarantine", "141026", "5689ba9cc75722bd645cd084b0637094", 159744),
	ENTRY0("quarantined", "200516", "14d1874ba423bc4ab968718d04cf377c", 161280),
	ENTRY0("quentaria", "120627", "8da1bca52421a3e4ee521fe1b01f1077", 243200),
	ENTRY0("questforadventure", "150406", "c5361bd224bff673c1b283dfcaba9eda", 229888),
	ENTRY1("questofsword", "1301017", "f7cdbf6c93adc80a0ce219f809b6fadf", 180224, Common::FR_FRA),
	ENTRY0("readingbooks", "130417", "acc958e4097696843adcadf359d95282", 157184),
	ENTRY0("readyforsummer", "170926", "770df18ae3dd406f82b8218983fd3dd5", 156672),
	ENTRY1("recherchedoudou", "131003", "1215031fd1440a3ffe35be9dce63f3f6", 179200, Common::FR_FRA),
	ENTRY1("recherchedoudou", "140119", "42060dd048b5cdb726551fcbcde9e771", 167936, Common::FR_FRA),
	ENTRY0("r2verbosity", "120214", "efd25b55fba16606a05647fa0fa49ebf", 156672),
	ENTRY0("r3slightlywrong", "120214", "7c38fefdc62090681e5058a8168e20f1", 156672),
	ENTRY0("r4portroyal", "120214", "5f7b6b6445c5ebf3ba5bf82474ea9186", 158720),
	ENTRY0("r5upandup", "120214", "b4927ddbe6c036495a1eb3702e8f94f1", 157184),
	ENTRY0("r6starryvoid", "120214", "5521e1b15eda03f4179490f1b5f2ff3f", 157696),
	ENTRY0("r7portroyal2", "120214", "e8a44c9b5db35fac98a02c4cc06a441b", 156672),
	ENTRY0("r8unbuttonelaffair", "120214", "8a74d175a33c6959c82fed97a47c6103", 156672),
	ENTRY0("r9portroyal3", "120214", "f36f3dee07e287cd52707c4e9e96ec78", 161280),
	ENTRY0("r10firstnamebasis", "120214", "0266d433afb90578dddb8ba549af8873", 157696),
	ENTRY0("r11midsummerday", "120214", "c6515f1cf9f4f9381a855d52060400fc", 156672),
	ENTRY0("r12tamed", "120214", "2291b30bd5e7362301afd362d89a0721", 157184),
	ENTRY0("r13disenchantbay", "120214", "0e8534f2e1d8500798af953711a8be9a", 157184),
	ENTRY0("r14replanting", "120214", "1f9053dc9a9993c69932676f32c36a4d", 156160),
	ENTRY0("r15disenchantbay2", "120214", "7a38975a399e37825e85cecfc9b2c263", 157696),
	ENTRY0("r16disenchantbay3", "120214", "77d33bae9d22ef57e0607028f6efc2bc", 157696),
	ENTRY0("r17disenchantbay4", "120214", "15bd655df5e824ffea0859194f285e23", 158208),
	ENTRY0("r18laura", "120214", "2ecd052f2a4f1e46bee605a689dd3a11", 157184),
	ENTRY0("r19disenchantbay5", "120214", "67f7e2ac3aec5c53db00d4087bc8bf2f", 158720),
	ENTRY0("r20escape", "120214", "4bea8a63aecee37686743a8ea8947506", 157184),
	ENTRY0("r21garibaldi", "120214", "ab7b0b14f9b38155068715a0e1eeffa2", 157696),
	ENTRY0("r22disenchantbay6", "120214", "90062fef6e39dbd5ca63db2bb8c226b7", 158720),
	ENTRY0("r23neighborwatch", "120215", "5557b0443863df3c2fac3718fbdd0609", 158208),
	ENTRY0("r24disenchantbay7", "120215", "d374af79701734cf4ea279bf93ac7ecc", 158720),
	ENTRY0("r25downbelow", "120215", "7d144d1dc879bdffe8d9b3a1b002fa90", 156672),
	ENTRY0("r26peugeot", "120215", "728516a74208fb1fc8e13641e18893db", 165888),
	ENTRY0("r27disenchantbay8", "120215", "ce70baad1751df502ca3091b96be3d39", 159232),
	ENTRY0("r28hover", "120215", "f4dc4f352806b4a5616efc5ab997cb1d", 158208),
	ENTRY0("r29disenchantbay9", "120215", "809bb7895b3b8efc1638179c039f2632", 159744),
	ENTRY0("r30belfry", "120215", "9d81ec33029c989affb8a6625c247889", 157184),
	ENTRY0("r31gopherwood", "120215", "bd19986cd640340bcb3dc93a4b61a5b3", 156672),
	ENTRY0("r32disenchantbay10", "120215", "b01edd7251e0af82cc6c28db8a1aa1fc", 160256),
	ENTRY0("r33disenchantbay11", "120215", "ccf8590de17242e14a815eaec24d0f83", 160256),
	ENTRY0("r34brown", "120216", "e208ef74076233766b7ad52f8908165e", 158208),
	ENTRY0("r35disenchantbay12", "120224", "07d4bfd1c2a19927a326b43d9f965527", 175616),
	ENTRY0("rabbitchase", "230502", "e346051fa378993110fd01b9d24ae70c", 157696),
	ENTRY0("reallylatest", "230809", "b804693c726d764a6dd2f4954fc374bf", 237568),
	ENTRY0("redsea", "240403", "29ab2fc64f479b0f732b0e1ec7989ae2", 162816),
	ENTRY0("refuge", "130911", "0d203646c88ca0b769ad2f06c61fde61", 158720),
	ENTRY0("reign", "191030", "551b3583ce7c48f76a40e8a9b47bc04f", 160768),
	ENTRY0("retribution", "180405", "f23829c0c26719f004a80e8b52b2ba23", 350720),
	ENTRY0("retrofatale", "110713", "b7816c11b5dc619c9b1e37ffb6575577", 222208),
	ENTRY0("retrofatale", "120216", "391ca06879628fd2dbd8a72622697aa8", 243200),
	ENTRY0("roadstoroam", "130502", "fd930e867b9875a823ea1e9936de7f6b", 172544),
	ENTRY0("romanadventure", "130214", "265bac7efefffd226d09dddabaedd8a6", 175616),
	ENTRY0("romancolosseum", "120305", "c388b54ecf9909b3af728cab56c0b4c6", 159232),
	ENTRY0("ronswoodshop", "231029", "a53662f7cff56c08895f39e964979252", 157184),
	ENTRY0("rooms", "221103", "69f3d4b871c7a4f0abb0919cd28be027", 170496),
	ENTRY0("safadventurev1", "130228", "165617f1aa1f4a27bc55c451b217a1c4", 158720),
	ENTRY0("safeskies", "190304", "14b16dbd5ede8bd0c2aa12c78bbc1eb1", 163840),
	ENTRY0("samsonrobots", "230405", "fbae6b823d659ce0dcecd0f27d1591c7", 157184),
	ENTRY0("scatterbrain", "170328", "bf7b6231a13181673bb604508c336989", 163328),
	ENTRY0("schooltingz", "191025", "15d03d5ff6c7cb77ebf388cf9cce6e06", 158208),
	ENTRY0("scifistorynewmedia", "190515", "42b6a624d3bcd13d5bbdc53ebe355c43", 179200),
	ENTRY0("scp173", "120517", "d5e5ada0d18345da6a0cbdbc1e80fa0d", 158720),
	ENTRY0("seadeep", "231029", "8f65a7dab4d27ea2ff89c92c0037c865", 161792),
	ENTRY0("searchmagicalbox", "150511", "1596bc9b3daeee73bd24ea451f7a62c7", 157184),
	ENTRY0("secretdoors", "180112", "0d498d4abf701a59eb4138673e697456", 160256),
	ENTRY0("secretswithin", "231103/Demo", "66ae27709841a85b9acd4b92fc15e9f2", 167424),
	ENTRY0("segradocorazon", "230428", "21a25d5bda7499dc9c8ea548196306ce", 158208),
	ENTRY0("selkiesnacks", "210723", "13f2c778f82bc1e753f4cdba15e5e6e0", 181760),
	ENTRY0("sepiadreams", "170129", "cae0c8724f505a98011b4792e27ffd0a", 174592),
	ENTRY0("sequencevents", "240421", "a1f34321821f9489e23247cea20b49c6", 205312),
	ENTRY0("shadowsflight", "190221", "b9da2c1d9853b8bee321b78867ddd94a", 190464),
	ENTRY0("sheepandshamans", "160410", "2dc5d09f91e47e03877312331b6b40eb", 177664),
	ENTRY0("sherlockic", "120218", "5303a2095cfdb68c7a1728f0293a23a9", 178176),
	ENTRY0("sherylcrowadvent", "140509", "8d5743c9558e87c333e99655574459ec", 157184),
	ENTRY0("shootingstarfinder", "180305", "dba33527b923b5e25ea3560f4e6bdec7", 160256),
	ENTRY0("simulation", "230131", "526b3d92b74b4c7d18149d38e929cc0d", 156672),
	ENTRY0("sleepingpearl", "231101", "24dc69d392fe180243edfb5104e88da9", 176128),
	ENTRY0("slenderforest", "130704", "ea9c7bc28a3df3446f9f10a863a2ce6f", 165376),
	ENTRY0("smallgaim", "200802", "af383adf85389c7696a22432278717bc", 163840),
	ENTRY0("snufflewhereareyou", "151208", "c5acf0ec396fc463ffd04f56a7e5d60c", 161792),
	ENTRY0("sorcerersforest", "151124", "7e0e81c88e6981da3338449eb5e2e48e", 173056),
	ENTRY0("sorrymum", "150314", "1b14e7a49d34f3733735d374b51b14d3", 158720),
	ENTRY0("soulwanderer", "141009", "d57684c9f6e8c503b48b8ebaa171692b", 166912),
	ENTRY0("sovietmarch", "220104", "c93738d39e5ec596cc05314314be2524", 156672),
	ENTRY0("spaceescape", "190503", "b932e7ba4de0bd7a9b7e8bda15e76525", 160256),
	ENTRY0("spatulasearch", "190731", "9c729e7db10b8aedd4f4049df61c792b", 173056),
	ENTRY0("splittinganatom", "161212", "812b91e06386a3dcf5acf6027df3401d", 164864),
	ENTRY0("spy", "190319", "6e1603a68778e1c6b0ae208523042029", 179200),
	ENTRY0("spyrothedragon", "140709", "1410248aa2c0344b5e5f026b7cf993a7", 166912),
	ENTRY0("starcitizen", "201113", "3b9f5e883745e5d7bcdf20a5c55eb4e7", 160256),
	ENTRY0("stardefense1", "220210", "9e4ba559158b81e3f3e0ebfd8d971e6f", 160768),
	ENTRY0("staringintoabyss", "141010", "97082994d3be82616c6016851a0657ae", 158208),
	ENTRY0("starsandbutts", "130623", "7efaf51e90ea6b7f2f3bdb00d93d77de", 229376),
	ENTRY0("startergame", "191209", "a4e9ace3b9b6a45b5b44aed9a2c1ccd2", 173568),
	ENTRY0("stealingthestolen", "120927", "6cbfbfa897ab53c52fff536245990c47", 186368),
	ENTRY0("stealingthestolen", "161222", "6daa42b7ae7f933fcf2fa441e702ea70", 424914),
	ENTRY0("steph", "231031", "3b42bb40df3325521d91e123bc3121c9", 158208),
	ENTRY0("stolengem", "231104", "0e5574b3da23d14b6aea63549ca0e603", 157184),
	ENTRY0("storything", "190821", "d3f239f244e24a8ae29b3abadeccbc65", 156672),
	ENTRY0("strandedspcemonkst", "180625", "ce807be7d8784bd039219bbf40d93cc6", 164864),
	ENTRY0("strangertheseparts", "120820", "c5a736a93771bccd55e4b7daa45a9f00", 195584),
	ENTRY0("strife", "120505", "b510f0f1a8c75f9ffbf9c3a285b5153a", 173056),
	ENTRY0("stuckinthesnow", "151207", "83cbb819a394ddcc6b29f6bc3301759d", 163328),
	ENTRY0("suddenlyseeksusan", "200112", "946bf6435708df67808bb30a0b78d6c0", 170496),
	ENTRY0("sugarrush", "160416", "fb53546ecaff1f6454e3501d550451ab", 173568),
	ENTRY0("suicideprevention", "190624", "62daf991c3b021b70967b2befe180154", 160256),
	ENTRY0("summertimeshy", "151006", "e1609a944183aa002dfba40f2f2bf04c", 170496),
	ENTRY0("sunrise", "121021", "c4300802aeac40bfda9c631f21884cdb", 160256),
	ENTRY0("survival", "120705", "ad9595a33a77e4611207a5f4a9ad56e7", 224768),
	ENTRY0("survivalrun", "120302", "d2aeb9718f3d52f665f277ff28f25443", 163840),
	ENTRY0("survivemathclass", "151025", "45cd470f106117ef98922935fccb5f18", 158720),
	ENTRY0("survseafoamisle4", "180706", "b5101bd5db657d03bfc98966f1126adc", 164864),
	ENTRY0("sushichef", "170608", "405a81e9c5af264b6b5bb7acd5bf7ad2", 209920),
	ENTRY0("sushichef", "190429", "919dbe89115ec17a24e4a356134ae9b5", 209920),
	ENTRY0("suspiciousinc4", "201024", "bbc94469835cfe0bcc993b9e409e9421", 252928),
	ENTRY0("swordsstory", "201130", "f89bbd11289a6c4170db043cbc72e85c", 218112),
	ENTRY0("takeyourmedicine", "150317", "669364549586c429f706d9d26ecdf6c5", 158720),
	ENTRY0("takingcareyourself", "191121", "f6661de1f80dc2dd4826e8962371ca87", 171520),
	ENTRY0("teamvoyager", "230316", "f9382be2707e0bb235996a2c4da6121f", 158720),
	ENTRY0("tearoom", "120508", "419307765ef0388c24b62e72b07c9f05", 189952),
	ENTRY0("templeescape", "230407", "afafbc39b458675e35e1ead187ad4e1a", 169984),
	ENTRY0("terrorblood", "160929", "0342e7d97bc78e0a727ce1e6c8d49852", 220672),
	ENTRY0("textadventinnyc", "190401", "c2f84e49ded3f03004219d73d5538eb5", 168448),
	ENTRY0("thanksgiving", "240410", "e9cadc401f9719c1dc03108c7326aa06", 173056),
	ENTRY0("thatdarnleprechaun", "181004", "cef4cf60d71f060c4c9082fddd2c2acd", 165888),
	ENTRY0("theabandonedmint", "200722", "05fcd7a1c5c275139e726a9151479438", 167424),
	ENTRY0("theanniversary", "120227", "78680dba1af856050b1a02b13389caf3", 165376),
	ENTRY0("theanniversary", "120302", "fcb54cd89ad5f47f4e6628c0de256208", 166912),
	ENTRY0("thebabysitter", "141031", "86aeafc387a3c465b10fc5ef97e72e77", 171008),
	ENTRY0("thebambooheist", "191028", "934ca59e21c8ac559ece127655a9b693", 230912),
	ENTRY0("thebasement", "200315", "97edd51d25faa6e63bf7667354ff1d0d", 207872),
	ENTRY0("thebieberfevprob", "120425", "92e4bccf43fdcf7ec50e4b9a5f4c4b12", 157184),
	ENTRY0("thebiter", "120925", "a99a2717d5546d6a4c6d99320241b08a", 158208),
	ENTRY0("theblue", "230106", "89e59284145fadefdedcb391b5d7e763", 158720),
	ENTRY0("thebumblebeeband", "180531", "10017c03974bb82ce3f8d73bec3672fd", 162816),
	ENTRY0("thecage", "160220", "a3eafd5967a972fe21db533d9943e30a", 177152),
	ENTRY0("thecellar", "120621", "eb7a7bb9ce6b7546882e6d786f85b992", 159744),
	ENTRY0("thechange", "120219", "775de38e27619212ec865a58081e4bba", 162816),
	ENTRY0("thechristgauntlet", "161223", "faf594cc3a7a24837febdd2690be7847", 163328),
	ENTRY0("thechristgauntlet2", "181225", "4bf6b96fe36ee0b3ec71321f7b76286e", 159232),
	ENTRY0("thecraving", "170201", "3dac6baba4f32742c0490e0de7b12966", 182272),
	ENTRY0("thecursedfieldtrip", "201006", "abebd4bc5bcda827ebb9c08da55cefa8", 177664),
	ENTRY0("thedeathquentcomp", "230522", "8e50c454a08f801d84b04e65066e133d", 194048),
	ENTRY0("thediabolocontrapt", "140607", "9e97ace8fadb4adc78d1f7db035adfc4", 168960),
	ENTRY0("thedream", "221116", "ae1eca05c35c5d831df07c501353c19e", 157184),
	ENTRY0("thedreamterminal", "161016", "e539e3eace13b6691aa8974423fd5f7e", 158720),
	ENTRY0("thedungdismember", "180323", "f1bb5e8c6bd2bd8a511b51e0e8d13b7f", 259584),
	ENTRY0("theelements", "200324", "c4b7287c1d594c5b37f738420d45b0e5", 267264),
	ENTRY0("theescape", "140618", "50a8d2d79291e391f6e09d4a1e101ceb", 161280),
	ENTRY0("thegameofsetanta", "200523", "4c4293d34722458c0045d1133f78bfc3", 282112),
	ENTRY0("thegreenhouse", "150317", "7f72cea783a2fc878c69353b27d5907c", 163840),
	ENTRY0("thegumshoes1case", "130923", "4c234fbd5e727fc60e65bdfe47627722", 158720),
	ENTRY0("thehardestdecision", "190703", "2fcbd4ee6f4a41332ca1c6fff923e98e", 159744),
	ENTRY0("theheist", "191028", "930c36631680725299453b77ca3f264e", 239104),
	ENTRY0("thehideworldmathew", "210902", "7f8b4d95a4a8ffbd99b6b54a857836cc", 161280),
	ENTRY0("thehospital", "231012", "3795347154bbf9be83e1bcb69a640a7c", 156160),
	ENTRY0("thehospital", "231030", "c1125f8e9850eb0a1a05816056614bd9", 182272),
	ENTRY0("theinnermostproj", "140517", "97df221f413f8e252251a3e9697c31ec", 157184),
	ENTRY0("theislandpf", "130225", "d18108dd62f934e665f0fc3b4d96ef28", 303104),
	ENTRY0("thejewelheist", "220406", "1d7dd786ac76983235dc8941c4a5e00c", 167936),
	ENTRY0("thejourneylifetime", "150619", "83931b11b8c82eedde117eb52659b82e", 156672),
	ENTRY0("thelabyrinthpf", "130513", "4400524c81a8590aa289058580c064e5", 169472),
	ENTRY0("thelibrarypf", "150317", "6675119f9a464d075fab555365505596", 161280),
	ENTRY0("thelightprince", "200731", "6f9ed5f14288217d88dada7314e597e3", 276480),
	ENTRY0("thelostbook", "231101", "9d15c79c8b808220a275c4088a14ac75", 158720),
	ENTRY0("themachine", "201016", "83604cd8e738bb3ba989f51688d2c481", 164352),
	ENTRY0("themafiagenerator", "130218", "ea7bc006b8ada46dd2cedf764110053e", 222720),
	ENTRY0("themafiagenerator", "190412", "dc3b48605a272b5234b644af04753994", 218624),
	ENTRY0("themagiccircle", "130501", "dd0454ec1585b807f8cd48a5273b8a56", 172032),
	ENTRY0("themountainfornost", "131103", "2d92110bc8ebf16775f857c486fa5120", 172032),
	ENTRY0("themystdisappear", "191229", "3b90a2fb87ab654e6ed9a30decb65ad7", 166912),
	ENTRY0("themythisisosiris", "120615", "66329f3a24fa52c0b04ce007e6b1331a", 171008),
	ENTRY0("themythicalforest", "130219", "b82191a36263969c1e0b284582aabc71", 164352),
	ENTRY0("themythicalforest", "130426", "36695151ff1870f2fffe89f6243fca18", 164864),
	ENTRY0("theonlywaytobesure", "120219", "49abcd44c3a6c23dc3a6a11ebce1d6c4", 158208),
	ENTRY0("thepackage", "161108", "9a48426f5e1409472b0b7563a0093d5d", 164352),
	ENTRY0("thepilot", "231102", "95668529f255b376ef7eb9c6e723cf08", 163840),
	ENTRY0("theprogressivexorn", "190301", "f6489c842c579911fde24f353f816c28", 225792),
	ENTRY0("thequestdragonlord", "230412", "3bcdadaeb413d7b41ed40f5ead94525e", 175616),
	ENTRY0("theredenglishbind", "150307", "80616df759a4f5fa9545570c50c668a6", 160768),
	ENTRY0("theschool", "231031", "989e4c571bf6d7a405ea4646f6207f62", 165376),
	ENTRY0("thesilence", "231026", "fa2889ad479f72d82fb63ec6da5b3eac", 162304),
	ENTRY0("thesixgatewayhell", "120428", "93522fc855141b48ee12c05369397829", 157184),
	ENTRY0("thesquawkingdead", "130107", "a2ba056d8e1530a37bc1a640466300db", 163840),
	ENTRY0("thesunwentdark", "120829", "7cb1f65e63dc807b2f931ed732f1d2f4", 158208),
	ENTRY0("thesuperbowl50", "160125", "d3169a735a5a3dade86969f9a14b3ddd", 158208),
	ENTRY0("thesustaintransp", "120920", "d7b282bdac1266575aa29c3451a5e738", 162304),
	ENTRY0("theterrosschism", "181221", "bfd7eb9e638c4642b585c166130c4fec", 165888),
	ENTRY0("thethingsivefelt", "150110", "0c86b59720e56367b38676089591c9f4", 202240),
	ENTRY0("thetwistedwood", "120425", "5e0d8057499e4c511d088219dbdfdb60", 168960),
	ENTRY0("theultimatum", "190429", "fb639da6d478a4fbf98e7d665898128d", 158720),
	ENTRY0("theuntoldstory", "200725", "d8be5eb87abdb0fb4f38a02fe51dc5bd", 158208),
	ENTRY0("thevirtgrandnation", "201103", "d213d8213d5816463fac57157e89c4e7", 175104),
	ENTRY0("thewallet", "210501", "671b69cc29a54377beaae50f9cd9cd7f", 328704),
	ENTRY0("thewallet", "210519", "375f70bb3b6451233751446035a18521", 165888),
	ENTRY0("thewizardcrystal", "150620", "b0a6cd566dce360e5b0ae33c601d9ce0", 164352),
	ENTRY0("thewizardslair", "150619", "a61b54e81e5b3b3898773546748aa874", 164352),
	ENTRY0("thewizardspotion", "170320", "bbc13040a768d0db88dfaa05ba86d38f", 178688),
	ENTRY0("thewizardstrials", "220406", "5fb10f5aa01d8af24e3af18dc22b4afe", 196608),
	ENTRY0("theworldgodyknows", "141202", "7ba8df985ffc75a43c71476bd9fd7428", 157696),
	ENTRY0("theyawninggrave", "181005", "3a604033e4987bce3f7c70277f5a0df4", 166400),
	ENTRY0("thezoobreak", "230411", "ab51e1646234caa4e2be615ca2dfd773", 204800),
	ENTRY0("theseus", "120615", "246908776bd8886531411b053c802b0e", 218624),
	ENTRY0("thiswarofmine", "140606", "11442185d0a59717a9941c775b4e40e0", 166912),
	ENTRY0("thosewhowander", "130609", "03e9318141fb59db9f03a361da6300d7", 167424),
	ENTRY0("threechoices3ways", "141515", "8aadaca67809b68293a0feaa1799e0bc", 225792),
	ENTRY0("thydungeonman", "120215", "ab0984441da50146bb0ec5580930ecca", 162816),
	ENTRY0("tiffanys", "120302", "d3b457d98cfdde50d55e30b691572468", 165888),
	ENTRY0("timetraveller", "240405", "40a07b59c656003e20d3f26dcf8c67fa", 231424),
	ENTRY0("timetraveller", "240412", "4078abeaf324d35325d9b25d389faced", 239104),
	ENTRY0("timmyland", "231026", "e95a0cf68a03c5b24a09ed502d3a70a5", 159744),
	ENTRY0("tinygardens", "240512", "1f63d03b46955b7d9c9a560f9f46ffe0", 161792),
	ENTRY0("toavengethestars", "120823", "361e7c9b412ae4c0abd96f3a95fb0940", 168960),
	ENTRY0("toyourowndevices", "120504", "af094f5f5bb61dfcd30db6a83251cceb", 166400),
	ENTRY0("todayjustasalways", "131218", "fa05a515c470f30ebbcda6ef0cd54f46", 252416),
	ENTRY0("tonyabottcrackcase", "130306", "024b5ce201a48d677625665e9b8a6031", 176128),
	ENTRY0("toxicschool", "151217", "8ab53ae3ca5e34570584674047bb6a62", 163840),
	ENTRY0("trainhopneworleans", "140927", "25b27df3e5e2c96e395e57540984c74f", 162304),
	ENTRY0("trainofthought", "120729", "3600a1c77c3a445f83144f7762aafc15", 220160),
	ENTRY0("trapped", "230412", "204f6ad68af1cb01ce692785d3882d9c", 166912),
	ENTRY0("trappedcastle", "231027", "718131d6c94f0d8238b0a911aefeeb80", 163328),
	ENTRY0("treasurehunt", "231101", "3600a1c77c3a445f83144f7762aafc15", 161280),
	ENTRY0("treasurehunting", "190304", "e632b687be4d8fd1251cfbdbd5034b58", 165888),
	ENTRY0("ttcrats", "231026", "19d52057f5067dbfbd48b11a58ef1f82", 165376),
	ENTRY0("tvdream", "190306", "a222e72f2f6bf12694c66abd0f28b3c4", 166912),
	ENTRY0("twilightmansion", "140702", "ffab730a2aaed3817a9fb3dceb0a7a31", 158720),
	ENTRY0("un1015lostsoul", "151119", "649faf05912c3339e31b363ec98a7d8f", 165888),
	ENTRY0("un1015tminus59min", "151205", "665065ef47c80135f01456d0959234ab", 169472),
	ENTRY0("unchartedisland", "231017", "52bed901470c57d26582e580a184af6c", 171008),
	ENTRY0("unchartedisland", "231107", "2e5aeb76854ed34f3f65ae0a10764def", 186368),
	ENTRY0("undercoop", "200303", "dbdc4c4b141f4b4e9c580cdc4d25a6e9", 158720),
	ENTRY1("unejournecomeautre", "131102", "a88b31c981b09028a9c7a2f7a2eac927", 178688, Common::FR_FRA),
	ENTRY1("unerebellion", "140120", "71e57e2efc20a28b4c3ab1937712bad7", 202752, Common::FR_FRA),
	ENTRY0("unlock", "150617", "06c82e297ed13b2e230ee4e0e95cb08b", 216576),
	ENTRY1("vacancesdeolie", "131122", "a608a90d7749a4269c6b30c1267fda8f", 161280, Common::FR_FRA),
	ENTRY0("vengefulalexdmon", "200325", "5c917c8e5baf09d404e57fc8e9868b19", 194048),
	ENTRY0("verge", "120523", "c7e60b875d240ee83dfac37596cce177", 267776),
	ENTRY0("vinylrecords", "160303", "409ae004fe9505ef4ac6d59cbfa04f47", 190464),
	ENTRY1("voyageinteractif", "130926", "9034dd27a46dd95833c176ebdacbffa7", 181760, Common::FR_FRA),
	ENTRY0("warrant", "120318", "d29223ca77f2a625fe4d8cd0e3100c4a", 169472),
	ENTRY0("weddinghunt", "230226", "a1b9c712b7148dff595597e43f947c69", 157696),
	ENTRY0("wednesday", "130724", "273f891a4c001b48908caef9eaf2e805", 248320),
	ENTRY0("welcomeafterlife", "231024", "cbbcf6d10ada8ea11010bd69d2945672", 158208),
	ENTRY0("whatswrong", "231024", "fc8edefeecf6765a9284321918006a03", 162816),
	ENTRY0("whereiseverybody", "140429", "f00d0e7de76c30260f0b9d0373c12f5a", 212480),
	ENTRY0("whereismrhuman", "221102", "c5d4e31c23e921f8ded68bc79699b5f6", 176640),
	ENTRY0("whittonavenue", "131228", "ca3358b79057cb31e0a2a2aba7925fde", 185856),
	ENTRY0("wingsofthepigeon", "140324", "3dfc014d949d5a7e9a7a3a39ae3a39ba", 264704),
	ENTRY0("witchcamebridge", "190427", "ce7a3fd88a98a83fcc500a0161803faa", 211456),
	ENTRY1("wohingehstdu", "130926", "5d5a5df290b6e8121e2fc68129ac5af4", 253440, Common::DE_DEU),
	ENTRY0("wrenchedanddivine", "141119", "a3e09817d7ca57f2b684b772f344ee7d", 167424),
	ENTRY0("wrenlaw_zcode", "130428", "1abcce7baecbe955f28f8a788bfde1e1", 291328),
	ENTRY0("wrenlaw_zcode", "130429", "24e41af3de0fd4052ef537d0e82b067c", 1835620),
	ENTRY0("yemagykqueste", "130805", "63a6c866aad16dccb74fa8b1cf684686", 170496),
	ENTRY0("youhavefreemanatee", "140506", "93e739304e8a4fb44f24a23dcaa32231", 161280),
	ENTRY0("yourang", "160607", "8b6145c6bb2ec63830277575a6f9854f", 210944),
	ENTRY0("yourgovneedsyou", "120504", "4d4557901e048ea8ba90b37762427adf", 185344),
	ENTRY0("zombiefacuacademy", "210215", "591a0e3bd3a3890406c9d7e4cbe887c1", 165376),
	ENTRY0("zoocapture", "231105", "7735b520c796a38435647cd58660d529", 159744),

	// ParserComp 2015
	ENTRY0("pc15_sixgrayrats", "150214", "822ebf1958e10ee5a9ba779a1edb0842", 427520),
	ENTRY0("pc15_sixgrayrats", "150331", "1a2b10729ebb83a08a38f0c80020a34d", 428544),
	ENTRY0("pc15_sunburn", "150214", "598cc42e8184a840bab2d38c6d6dece0", 471332),

	// ParserComp 2021
	ENTRY0("pc21_acidrain", "210701", "69376ad4a9d4d5d5a161b230911cb4af", 148480),
	ENTRY0("pc21_fivebyfivia", "210627", "655affb35059c2077dd74070db1d9572", 356386),
	ENTRY0("pc21_dannydipstick", "210701", "cfbd17cf1e4f16c61577c5a5b1df8a41", 114176),

	// ParserComp 2022
	ENTRY0("pc22_imposstairs", "220706", "ab5d27c9b35e52e34e2c5300680e7aa7", 243320),
	ENTRY0("pc22_alchemistgold", "220701/z3", "e3e298d416710e85778b2fe31a7afc63", 81408),
	ENTRY0("pc22_alchemistgold", "220701/z5", "271570957cf46d5c2f71904ec8f69b8f", 83968),
	ENTRY0("pc22_yougetback", "220731/v1", "ca81a9dd480cfa581fe897cf6f8140ea", 296080),
	ENTRY0("pc22_yougetback", "220731/v2", "a88c49f219bc9fae153a2fd123ed3eb8", 294544),

	// ParserComp 2023
	ENTRY0("pc23_searchlostark", "230629/z3", "4e08c5528461466698f81f0cd10a2e3f", 74240),
	ENTRY0("pc23_searchlostark", "230629/z5", "546ec46e41f9056a602ece827173e2c7", 77312),
	ENTRY0("pc23_xou", "230630/v1", "fe2156a887028e4715706162a47eccc3", 259054),
	ENTRY0("pc23_xou", "230630/v2", "0bb1331dc6c7cee296134bba21896e94", 260614),

	// PunyJam #1
	ENTRY0("pj1_captaincutters", "210503/z3", "02e6b574267034d10bb0cdeacb124cb4", 84992),
	ENTRY0("pj1_captaincutters", "210503/z5", "b7b85a17256237b07bb988a6df924873", 88064),
	ENTRY0("pj1_djinniotr", "210519/z5", "c99e6e3182a1d25c7deb0cf28361a0d8", 96256),
	ENTRY0("pj1_djinniotr", "210519/zblorb", "9f526cada9b557ee72b06984e475176f", 514430),
	ENTRY0("pj1_pubhubbub", "210503", "137af3205cb784e6f18764b726893796", 55296),
	ENTRY0("pj1_deathnumber4", "210502", "634b0a4a44859ad770f6754854d04d6b", 35840),
	ENTRY0("pj1_pubadventure", "210429", "1ff56a0f9039a07450c01d73b388742c", 31232),
	ENTRY0("pj1_arthurdayout", "210430", "9404cfa00a0e0dc3ea1dfefbcd8ded4d", 38400),
	ENTRY0("pj1_buccaneercache", "210502", "0fc979bfee360e4e966d361aa815ed54", 33792),
	ENTRY0("pj1_buccaneercache", "210522", "7ba7e32110ac3ca97db5e20e3f167bf3", 34304),
	ENTRY0("pj1_thejob", "210503", "db22f3109827bc180836f060147e7399", 37376),
	ENTRY0("pj1_thejob", "210608", "dc2167b524b00040e7540251b1eec08d", 44032),
	ENTRY0("pj1_thejob", "210930/z5", "4714cd741c5f5be68f1dece71361e11e", 44032),
	ENTRY0("pj1_thejob", "210930/z3", "1ef1bba228e38a1bd6fb2fde4a8b634c", 42496),

	// PunyJam #2
	ENTRY0("pj2_morris", "211121", "72c139095c2664c9162a1321d8694e19", 66048),
	ENTRY0("pj2_opportunity", "211120", "4dcc8c5041f3fd38c3172f38176904cf", 60928),
	ENTRY0("pj2_phobias", "211121", "95e7a65e33211b7d4699740e5d35a470", 48640),

	// PunyJam #3
	ENTRY0("pj3_submarinesabot", "230512/z3", "9f4843bb85c7c5dbfc06bbe09ad4bf7f", 65536),
	ENTRY0("pj3_submarinesabot", "230512/z5", "5cc0529faba276319dce0ccf0a3681fc", 68608),
	ENTRY0("pj3_a1rl0ck", "230225", "c41cdba49309a60aaafb79238e814726", 86528),
	ENTRY0("pj3_a1rl0ck", "230317", "11c071867ac8bb4f3a104069fb6c9c3d", 82432 ),
	ENTRY0("pj3_a1rl0ck", "230402/z3", "97c7a7a51201db8ed5943b6a5cf26a90", 81408),
	ENTRY0("pj3_a1rl0ck", "230402/z5/v1", "b0685da79e0c783b69a3944658ea34f6", 83456),
	ENTRY0("pj3_a1rl0ck", "230402/z5/v2", "83056c8cc43940d85e58e3fafd4199b3", 83456),
	ENTRY0("pj3_strikeforce", "230227", "721affab23af57a324a59046a32206bc", 76800),
	ENTRY0("pj3_lucidnight", "230226", "6544a782cfb772d929d6f8eb61e54db3", 64512),
	ENTRY0("pj3_lucidnight", "230320", "7e82fcf268692a811cffd60661f85798", 67072),
	ENTRY0("pj3_fallingpieces", "230225", "74dd61cb01e76b47b8580d7b234a1357", 48128),
	ENTRY0("pj3_blorp", "230419/z3", "24c6eb86d8710dc50675b9d84dc63f15", 56320),
	ENTRY0("pj3_blorp", "230419/z5", "f6172926a3de43e1dbd1d756dbcc91ab", 58368),
	ENTRY0("pj3_seacoral", "230225", "4f5db746459926ac273390c2424872d5", 69120),
	ENTRY0("pj3_cleangetaway", "230226", "1c9f4a67a48476426b1460aa49e0a70f", 53248),
	ENTRY0("pj3_cleangetaway", "230430/z3", "d10bd839564e6c737b674c35ab518baf", 53760),
	ENTRY0("pj3_cleangetaway", "230430/z5", "458678c12490137ffdd972b78b307562", 54784),
	ENTRY0("pj3_fantadimension", "230313", "2281e7a74333a7730ac44fd233d4f9b3", 60416),
	ENTRY0("pj3_fantadimension", "230508/z3", "25455e67bcc64a4343993e8acfbb0b48", 60928),
	ENTRY0("pj3_fantadimension", "230508/z5", "2eb8729675b9e461b7bbf3449f3538d8", 62464),

	// PunyJam #4
	ENTRY0("pj4_rop", "240109", "9ca3bd11f6e2c3b6efa60ad3267c3a2d", 102912 ),
	ENTRY0("pj4_cargobreach", "231217/z3", "b306ae2ae87676c1acff17855d965406", 67584 ),
	ENTRY0("pj4_cargobreach", "231217/z5", "cf5828552b5e93375b12ff4bb47efc8d", 70144 ),
	ENTRY0("pj4_shaka", "231217", "1ddfecc4bc43cc23bcce7a424045397b", 49664 ),
	ENTRY0("pj4_redux", "231217", "f74da4b780850c8b5025050701f0c107", 62976 ),
	ENTRY0("pj4_minimalgame", "231217", "f74da4b780850c8b5025050701f0c107", 62976 ),
	ENTRY0("pj4_pharaoh", "231212", "8c747ebe64aafb89b97be50cd8e2fe70", 46592 ),
	ENTRY0("pj4_pharaoh", "240123", "4893314695a7ce440169db126a77e290", 67072 ),

	// SeedComp! 2023
	ENTRY0("kingsball", "230301/z3", "d2f342061077926cc7be238a25af016e", 70144),
	ENTRY0("kingsball", "230301/z5", "546eea1e3d47cf5139a51188916f9c39", 72704 ),

	FROTZ_TABLE_END_MARKER
};

} // End of namespace ZCode
} // End of namespace Glk
