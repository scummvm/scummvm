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
namespace TADS {

const PlainGameDescriptor TADS2_GAME_LIST[] = {
	{ "tads2",              "TADS 2 Game" },

	// English games
	{ "2044",               "2044: Space Troopers" },
	{ "3",                  "+= 3: A Logical Adventure" },
	{ "academy",            "Academy of Spies" },
	{ "allalone",           "All Alone" },
	{ "amissville",         "Amissville" },
	{ "asimpletheft",       "A Simple Theft" },
	{ "asugaredpill",       "A Sugared Pill" },
	{ "bmiss",              "Bmissfille" },
	{ "brokenstring",       "The Broken String" },
	{ "campingtrip",        "The Camping Trip" },
	{ "castleofdoom",       "Castle of Doom" },
	{ "dampcamp",           "The Damp Camp" },
	{ "deanna",             "Deanna" },
	{ "deepspace",          "Deep Space Drifter" },
	{ "depravitybites",     "Depravity Bites" },
	{ "diemasse",           "Die Vollkommene Masse" },
	{ "drool",              "Drool" },
	{ "e14s",               "Exactly 14 syllables... er, gulps!" },
	{ "ecdysis",            "Ecdysis" },
	{ "edgarhuntly",        "Edgar Huntly, Or, Memoirs of a Sleepwalker" },
	{ "enhanced",           "Enhanced" },
	{ "escapecrazyplace",   "Escape from the Crazy Place" },
	{ "eterna",             "Eterna Corp" },
	{ "excuseme",           "Excuse Me, Do You Have The Time?" },
	{ "firebird",           "Firebird" },
	{ "firewitch",          "John's Fire Witch" },
	{ "foom",               "FooM - the shoot-'em up text adventure" },
	{ "frustration",        "Frustration" },
	{ "galacticpeace",      "Galactic Peace" },
	{ "gc",                 "GC: A Thrashing Parity Bit of the Mind" },
	{ "gift2",              "Gifts of Phallius 2: The Key to Eternity" },
	{ "gift3",              "Gifts of Phallius 3: The Valkin War" },
	{ "gna",                "The Generic New York Apartment Building" },
	{ "goldenfleece",       "The Golden Fleece" },
	{ "goldenskull",        "The Golden Skull" },
	{ "helpfulman",         "The Adventures of Helpfulman" },
	{ "heroinc1",           "Hero, Inc Part One: Calling All Heroes" },
	{ "holygrail",          "The Holy Grail" },
	{ "idealneps",          "Ideal New England Prep School" },
	{ "indarkness",         "In Darkness" },
	{ "inheritance",        "Inheritance" },
	{ "insanitycubed",      "Insanity Cubed" },
	{ "islandinfinity",     "The Island of Infinity" },
	{ "islecult",           "Isle Cult" },
	{ "journeyking",        "The Journey of the King" },
	{ "katana_tads",        "Katana" },
	{ "kitchenencounters",  "Kitchen Encounters" },
	{ "letheflowphoenix",   "Lethe Flow Phoenix: A Flight of Fantasy" },
	{ "lost_jh",            "Lost (by Jeffrey Hersh)" },
	{ "meanstreets",        "Mean Streets" },
	{ "meine",              "Meine Dalix" },
	{ "modernism",          "Modernism" },
	{ "moist",              "Moist" },
	{ "mrsk",               "Magic Realms: The Sword of Kasza" },
	{ "murmellius2",        "Murmellius 2" },
	{ "museumoftads",       "Museum of TADS" },
	{ "myth_bv",            "Myth (by Barry Volain)" },
	{ "nbdemo",             "The Singular, and Historically Inaccurate, Misadventures of Nobeard" },
	{ "ninjaburger",        "Ninja Burger" },
	{ "onefootdown",        "One Foot Down" },
	{ "pasttense",          "Past Tense" },
	{ "pcu",                "PC University: An Everyday Nightmare" },
	{ "perditionsflame",    "Perdition's Flames" },
	{ "perilskies",         "Peril in the Skies" },
	{ "pesach",             "The Pesach Adventure" },
	{ "polyadv",            "PolyAdv - Colossal Cave" },
	{ "ptbad4",             "PTBAD4: How the Record Got its Groove Back" },
	{ "ptbad5",             "PTBAD5: Jesus Attack of Jesus Christ" },
	{ "reeferisland",       "Reefer Island" },
	{ "saveprince_tads",    "Save Princeton" },
	{ "sd2",                "School Dreams 2: Forfeit Fantasy" },
	{ "sd3",                "School Dreams 3: School Dreams Forever" },
	{ "sda",                "Special Detective Agent" },
	{ "seaofnight",         "The Sea of Night" },
	{ "secondpit",          "The Second Pit" },
	{ "sexartist",          "The Sex Artist" },
	{ "shadowland",         "Shadowland I: The Tower of Iron" },
	{ "silenceofthelambs",  "Silence of the Lambs" },
	{ "son0",               "Prelude to Night" },
	{ "spyderandjeb",       "Spyder and Jeb" },
	{ "stranded_jb",        "Stranded (by Jim Bayers)" },
	{ "suprematismblack",   "Suprematism - Black" },
	{ "suprematismwhite",   "Suprematism - White" },
	{ "tadsdungeon",        "Dungeon" },
	{ "textmetroid",        "Metroid" },
	{ "theapprentice",      "The Apprentice" },
	{ "thegate",            "The Gate" },
	{ "thehouset2",         "The House" },
	{ "themansion",         "The Mansion or The Tale of the Adventurous Thief" },
	{ "themission",         "The Mission" },
	{ "theoracle",          "The Oracle" },
	{ "tide",               "The Ebb and Flow of the Tide" },
	{ "tnc",                "Tomorrow Never Comes" },
	{ "tortoise",           "The Lesson of the Tortoise" },
	{ "tylerscube",         "Tyler's Great Cube Game" },
	{ "unease",             "Unease" },
	{ "urbancleanup",       "Urban Cleanup" },
	{ "veritas",            "Veritas" },
	{ "videobar",           "I'm Gonna Take You To The Video Bar!" },
	{ "waystation",         "Waystation" },
	{ "wormhole",           "Wormhole: The Beginning" },
	{ "xarbo",              "Escape from the Arboretum" },
	{ "yourchoice",         "Your Choice" },

	// Adventions games (TADS2)
	{ "ccr",                "Colossal Cave Revisited" },
	{ "legendlives",        "The Legend Lives!" },
	{ "rylvania",           "The Horror of Rylvania" },
	{ "u0",                 "Unnkulia Zero: The Search for Amanda" },
	{ "uhalf",              "Unnkulia One-Half: The Salesman Triumphant" },
	{ "uu1",                "Unnkulian Underworld: The Unknown Unventure" },
	{ "uu2",                "Unnkulian Unventure II: The Secret of Acme" },

	// IFComp 1995 (TADS2)
	{ "if95_zebulon",       "Uncle Zebulon's Will" },                                                   //    1th Place
	{ "if95_toonesia",      "Toonesia" },                                                               //    2nd Place
	{ "if95_onegotaway",    "The One That Got Away" },                                                  //    3rd Place
	{ "if95_museumforever", "A Night at the Museum Forever" },                                          //    4th Place
	{ "if95_undertow",      "Undertow" },                                                               //    4th Place
	{ "if95_undo",          "Undo" },                                                                   //    6th Place

	// IFComp 1996 (TADS2)
	{ "if96_smallworld",    "Small World" },                                                            //    4th Place
	{ "if96_kissingbuddha", "Kissing the Buddha's Feet" },                                              //    5th Place
	{ "if96_mamoonlight",   "Maiden of the Moonlight" },                                                //    7th Place
	{ "if96_alienabduct",   "Alien Abduction" },                                                        //    9th Place
	{ "if96_aayela",        "Aayela" },                                                                 //   10th Place

	// IFComp 1997 (TADS2)
	{ "if97_babel",         "Babel" },                                                                  //    2nd Place
	{ "if97_glowgrass",     "Glowgrass" },                                                              //    3rd Place
	{ "if97_sunsavannah",   "Sunset over Savannah" },                                                   //    6th Place
	{ "if97_zefronalmanac", "Poor Zefron's Almanac" },                                                  //    7th Place
	{ "if97_zerosumgame",   "Zero Sum Game" },                                                          //   11th Place
	{ "if97_zombietads2",   "Zombie!" },                                                                //   12th Place
	{ "if97_fren51",        "The Frenetic Five vs. Sturm und Drang" },                                  //   13th Place
	{ "if97_virtuatech",    "Virtua Tech" },                                                            //   21st Place
	{ "if97_aardvark",      "The Obscene Quest of Dr. Aardvarkbarf" },                                  //   22nd Place
	{ "if97_templeorcmage", "Temple of the Orc Mage" },                                                 //   26th Place

	// IFComp 1998 (TADS2)
	{ "if98_theplant",      "The Plant" },                                                              //    3rd Place
	{ "if98_arrival",       "Arrival, or Attack of the B-Movie Cliches" },                              //    4th Place
	{ "if98_fourin1",       "Four in One, an Interactive Marx Brothers Comedy" },                       //   16th Place
	{ "if98_lightiania",    "Lightiania" },                                                             //   22nd Place

	// IFComp 1999 (TADS2)
	{ "if99_6stories",      "Six Stories" },                                                            //    3rd Place
	{ "if99_exhibition",    "Exhibition" },                                                             //    5th Place
	{ "if99_onthefarm",     "On the Farm" },                                                            //    7th Place
	{ "if99_erehwon",       "Erehwon" },                                                                //   11th Place
	{ "if99_bliss",         "Bliss: An Interactive Harrowing" },                                        //   13th Place
	{ "if99_stonecell",     "Stone Cell" },                                                             //   14th Place
	{ "if99_4seconds",      "Four Seconds" },                                                           //   15th Place
	{ "if99_amoment",       "A Moment of Hope" },                                                       //   18th Place
	{ "if99_chaostads2",    "Chaos" },                                                                  //   19th Place
	{ "if99_strangernight", "Strangers in the Night" },                                                 //   20th Place
	{ "if99_bealstreet",    "Life on Beal Street" },                                                    //   26th Place
	{ "if99_thewaterbird",  "The Water Bird" },                                                         //   29th Place

	// IFComp 2000 (TADS2)
	{ "if00_kaged",         "Kaged" },                                                                  //    1st Place
	{ "if00_atwitsend",     "At Wit's End" },                                                           //   17th Place
	{ "if00_planmind",      "Planet of the Infinite Minds" },                                           //   19th Place
	{ "if00_endmeans",      "The End means Escape" },                                                   //   21st Place
	{ "if00_futzmutz",      "Futz Mutz" },                                                              //   25th Place
	{ "if00_unnkuliax",     "Unnkulia X" },                                                             //   27th Place
	{ "if00_desertheat",    "Desert Heat" },                                                            //   28th Place
	{ "if00_thetrip",       "The Trip" },                                                               //   33rd Place
	{ "if00_aftermath",     "Aftermath" },                                                              //   37th Place
	{ "if00_theclock_ck",   "The Clock (by Cleopatra Kozlowski)" },                                     //   38th Place
	{ "if00_threadlaby",    "Threading the Labyrinth" },                                                //   40th Place
	{ "if00_jarodsjourney", "Jarod's Journey" },                                                        //   47th Place

	// IFComp 2001 (TADS2)
	{ "if01_ntts",          "No Time to Squeal" },                                                      //    4th Place
	{ "if01_beetmonger",    "The Beetmonger's Journal" },                                               //    5th Place
	{ "if01_triune",        "Triune" },                                                                 //    9th Place
	{ "if01_journeyislet",  "Journey from an Islet" },                                                  //   12th Place
	{ "if01_grayscale",     "Grayscale" },                                                              //   13th Place
	{ "if01_coasthouse",    "The Coast House" },                                                        //   15th Place
	{ "if01_nightguest",    "A Night Guest" },                                                          //   16th Place
	{ "if01_fusillade",     "Fusillade" },                                                              //   18th Place
	{ "if01_thecruise",     "The Cruise" },                                                             //   27th Place
	{ "if01_kallisti",      "Kallisti" },                                                               //   31st Place
	{ "if01_stranded_rc",   "Stranded (by Rich Cummings)" },                                            //   37th Place
	{ "if01_volcanoisle",   "Volcano Isle" },                                                           //   42th Place

	// IFComp 2002 (TADS2)
	{ "if02_tilldeath",     "Till Death Makes a Monk-Fish Out of Me" },                                 //    2nd Place
	{ "if02_augustine",     "Augustine" },                                                              //   13th Place
	{ "if02_granite",       "The Granite Book" },                                                       //   16th Place
	{ "if02_evacuate",      "Evacuate" },                                                               //   19th Place
	{ "if02_notmuchtime",   "Not Much Time" },                                                          //   22nd Place
	{ "if02_color",         "Color and Number" },                                                       //   24th Place
	{ "if02_concrete",      "Concrete Paradise" },                                                      //   30th Place
	{ "if02_coffeequest",   "Coffee Quest II" },                                                        //   32nd Place
	{ "if02_moonbase",      "Moonbase" },                                                               //   34th Place
	{ "if02_terriblelzrds", "Terrible Lizards" },                                                       //   36th Place

	// IFComp 2003 (TADS2)
	{ "if03_scavenger",     "Scavenger" },                                                              //    3rd Place
	{ "if03_erudite",       "The Erudition Chamber" },                                                  //    4th Place
	{ "if03_recruit",       "The Recruit" },                                                            //    7th Place
	{ "if03_cerulstowaway", "Cerulean Stowaway" },                                                      //    9th Place
	{ "if03_episodeartist", "Episode in the Life of an Artist" },                                       //   11th Place
	{ "if03_templekaos",    "Temple of Kaos" },                                                         //   15th Place
	{ "if03_delvyn",        "Delvyn" },                                                                 //   23rd Place
	{ "if03_bio",           "Bio" },                                                                    //   25th Place
	{ "if03_amnesia",       "Amnesia" },                                                                //   27th Place

	// IFComp 2004 (TADS2)
	{ "if04_magocracy",     "Magocracy" },                                                              //   18th Place
	{ "if04_kurusu",        "Kurusu City" },                                                            //   20th Place
	{ "if04_whocreated",    "Who Created That Monster?" },                                              //   25th Place
	{ "if04_realm",         "The Realm" },                                                              //   27th Place
	{ "if04_redeye",        "Redeye" },                                                                 //   28th Place
	{ "if04_zero",          "Zero" },                                                                   //   30th Place
	{ "if04_lighttale",     "A Light's Tale" },                                                         //   32nd Place
	{ "if04_ruinedrobots",  "Ruined Robots" },                                                          //   34th Place
	{ "if04_ptbad3",        "PTBAD3" },                                                                 //   35th Place

	// IFComp 2005 (TADS2)
	{ "if05_chancellor",    "Chancellor" },                                                             //    9th Place
	{ "if05_gilded",        "Gilded" },                                                                 //   16th Place
	{ "if05_xen",           "Xen: The Contest" },                                                       //   16th Place
	{ "if05_offthetrolley", "Off the Trolley" },                                                        //   20th Place
	{ "if05_sabotagecldr",  "Sabotage on the Century Cauldron" },                                       //   23rd Place
	{ "if05_amissville2",   "Amissville II" },                                                          //   32nd Place

	// IFComp 2006 (TADS2)
	{ "if06_ballymunadv",   "Ballymun Adventure" },                                                     //   33rd Place
	{ "if06_lawnoflove",    "Lawn of Love" },                                                           //   35th Place

	// IFComp 2007 (TADS2)
	{ "if07_amoi",          "A Matter of Importance" },                                                 //   10th Place
	{ "if07_slapthatfish",  "Slap That Fish" },                                                         //   19th Place

	// IFComp 2008 (TADS2)
	{ "if08_berrost",       "Berrost's Challenge" },                                                    //   10th Place

	// IFComp 2011 (TADS2)
	{ "if11_blind",         "Blind" },                                                                  //   23rd Place
	{ "if11_fogconvict",    "Fog Convict" },                                                            //   10th Place

	// IFComp 2013 (TADS2)
	{ "if13_wizardapprent", "The Wizard's Apprentice" },                                                //   36th Place

	// IFComp 2017 (TADS2)
	{ "if17_antiquest",     "Antiquest" },                                                              //   62th Place

	// IF Art Show Spring 1999 (TADS2)
	{ "ifas_pillow",        "Pillow" },

	// IF Art Show 2001 (TADS2)
	{ "ifas_memories",      "Memories" },

	// IF Art Show 2003 (TADS2)
	{ "ifas_friendlyfoe",   "Friendly Foe" },
	{ "ifas_tarotreading",  "The Tarot Reading" },

	// IF Art Show 2004 (TADS2)
	{ "ifas_lastride",      "Last Ride of the Night" },

	// Arcade Collection (TADS2)
	{ "ifa_digdug",         "Dig Dug" },
	{ "ifa_mc",             "MC" },
	{ "ifa_pong",           "Pong" },
	{ "ifa_spaceinvaders",  "Space Invaders!" },
	{ "ifa_tapper",         "Tapper" },
	{ "ifa_tilt",           "Tilt!" },

	// Chicken Competition (TADS2)
	{ "cc_landing",         "The Landing" },
	{ "cc_learningtocross", "Learning to Cross" },
	{ "cc_sisychickenphus", "Sisychickenphus" },

	// Dinosaur Mini-Competition (TADS2)
	{ "dino_deepbrowlift",  "Deep Brow Lifter" },
	{ "dino_dinolove",      "Dinosaur Love" },

	// Dragon Mini-Competition (TADS2)
	{ "dr_dragrscstories",  "Dragon Resources Stories" },

	// IntroComp 2002 (TADS2)
	{ "ic02_awea",          "At Wit's End Again" },
	{ "ic02_maintenman",    "The Maintenance Man" },
	{ "ic02_privcyborg",    "Private Cyborg" },
	{ "ic02_virus",         "Virus" },

	// IntroComp 2004 (TADS2)
	{ "ic04_betsabet",      "A Bet's A Bet" },

	// IntroComp 2010 (TADS2)
	{ "ic10_flooby",        "For the Love of Ornery Blue Yaks" },

	// IF Library Competition 2002 (TADS2)
	{ "lc_dwenodon",        "Dwenodon" },
	{ "lc_passingfamiliar", "Passing Familiarity" },

	// LOTECH Competition 2001 (TADS2)
	{ "lo01_escplanmonk",   "Escape from a Planet Filled with Monkeys!" },
	{ "lo01_oneweek",       "One Week" },

	// Manos Mini-Competition (TADS2)
	{ "ma_foggywood",       "Foggywood Hijinx" },

	// Segment Mini-Comp (TADS2)
	{ "snowman",            "Snowman Sextet Part I: But for a Single Flake" },

	// SmoochieComp 2001 (TADS2)
	{ "sc_secondhoneymoon", "Second Honeymoon" },

	// SpeedIF 2 Competition (TADS2)
	{ "sif_kidsdonteat",    "Kids, Don’t Eat Your Halloween Candy" },

	// SpeedIF 3 Competition (TADS2)
	{ "sif_moralmethis",    "Moral Me This" },

	// SpeedIF 4 Competition (TADS2)
	{ "sif_onion",          "I Don't Remember Why This Game is Called 'Onion'" },
	{ "sif_onion2",         "Nostradamus's Onion Sandwich" },
	{ "sif_untitled",       "(Untitled)" },

	// SpeedIF 5 Competition (TADS2)
	{ "sif_sludge",         "Sludge" },
	{ "sif_speed5",         "Speed5" },
	{ "sif_spysnow",        "Spy in the Snow" },

	// SpeedIF 5.5 Competition (TADS2)
	{ "sif_bhy",            "Blue Head Yurt" },
	{ "sif_dontfire",       "Don't Fire Until You See the Yellows of Their Niblets" },
	{ "sif_rock",           "A Rock and a Hard Place" },
	{ "sif_rockdsg",        "The Yellow Stone" },
	{ "sif_rockie",         "A TADS Adventure" },
	{ "sif_yourmind",       "Your Mind Is Gone, It Must Be Brought, You Have Some Rock Candy" },

	// SpeedIF 6 Competition (TADS2)
	{ "sif_trudge",         "Trudge" },

	// SpeedIF 7 Competition (TADS2)
	{ "sif_bloodymess",     "The Bloody Mess" },

	// SpeedIF 8 Competition (TADS2)
	{ "sif_pants",          "I Went to the WTO Ministerial Conference" },

	// SpeedIF 9 Competition (TADS2)
	{ "sif_legbreak",       "LegBreaker 3k!" },

	// SpeedIF 10 Competition (TADS2)
	{ "sif_freedom",        "Freedom (by Peter Berman)" },

	// SpeedIF 10^-9 Competition (TADS2)
	{ "sif_undergrcomp",    "Underground Compound" },

	// SpeedIF 11 Competition (TADS2)
	{ "sif_funandgames",    "Fun and Games" },
	{ "sif_livingroom",     "Living Room" },
	{ "sif_stairs",         "Stairs" },

	// SpeedIF 12 Competition (TADS2)
	{ "sif_hose",           "The Hose" },
	{ "sif_scurvygift",     "A Scurvy Gift for Bosn Chuck" },

	// SpeedIF 14 Competition (TADS2)
	{ "sif_deathwaif",      "Death Waif" },

	// SpeedIF 15 Competition (TADS2)
	{ "sif_dangerschool",   "Danger School" },
	{ "sif_fivetutors",     "The Five Tutors" },
	{ "sif_ralph_dj",       "Ralph (by Doug Jones)" },
	{ "sif_weavingbasket",  "Weaving a Basket (or Something)" },

	// SpeedIF 17 Competition (TADS2)
	{ "sif_stupidgame",     "Stupid Game" },
	{ "sif_whydoesalways",  "Why Does My New Year's Eve Always Crash and Burn?" },

	// SpeedIF 18 Competition (TADS2)
	{ "sif_manna",          "Manna" },
	{ "sif_suffersuppl",    "The Suffering Supplicant" },
	{ "sif_zymurgy",        "Zymurgy" },

	// SpeedIF 20 Competition (TADS2)
	{ "sif_21points",       "21 Points" },

	// SpeedIF -1 Competition (TADS2)
	{ "sif_screw",          "Screw the Boston Tea Party" },

	// SpeedIF 2000 Competition (TADS2)
	{ "sif_diorama",        "Diorama" },
	{ "sif_lionwinter",     "The Lion in Winter" },

	// SpeedIF 2001 Competition (TADS2)
	{ "sif_talltales",      "Tall Tales in the Big Easy" },

	// SpeedIF 3rd Anniversary Competition (TADS2)
	{ "sif_leatherworld",   "Leatherworld" },
	{ "sif_chartage",       "The Carthage Corn Maze" },

	// SpeedIF 5th Anniversary Competition (TADS2)
	{ "sif_brainrain",      "A Brain in the Rain" },
	{ "sif_casinoviridoso", "Casino Viridoso V" },
	{ "sif_happen",         "It Could Only Happen to You" },

	// SpeedIF Apocalypse Competition (TADS2)
	{ "sif_speedapocalyp",  "SpeedApocalypse" },

	// SpeedIF Argonaut Competition (TADS2)
	{ "sif_ascii",          "ASCII and the Argonauts: Astral Plane" },

	// SpeedIF Autocratic Competition (TADS2)
	{ "sif_atthetop",       "At the Top of Your Game" },
	{ "sif_digging",        "Digging thru Doctrine of the Dead" },

	// SpeedIF Crinkle Cut Competition (TADS2)
	{ "sif_dithyramb",      "Dithyrambic Bastards" },
	{ "sif_glacialrift",    "Glacial Rift of the Dick Cavett" },

	// SpeedIF Douglas Adams Tribute Competition (TADS2)
	{ "sif_goodnight",      "Into That Good Night" },
	{ "sif_jeenin",         "Jeenin Tonn-nx" },

	// SpeedIF EXTREME Competition (TADS2)
	{ "sif_weirdsister",    "The Weird Sister" },

	// SpeedIF >= 20 Competition (TADS2)
	{ "sif_cranial",        "Cranial Pounding" },
	{ "sif_thing",          "Things" },
	{ "sif_tppwwe",         "Time Place People Water Way" },

	// SpeedIF Halloween 2002 Competition (TADS2)
	{ "sif_gummibear",      "A Gummi Bear's Night Out" },
	{ "sif_rivkin",         "Rivkin" },
	{ "sif_yellowdog",      "Yellow Dog Running" },

	// SpeedIF Jacket Competition (TADS2)
	{ "sif_spifftime",      "A Spliff in Time" },

	// SpeedIF Jacket 2 Competition (TADS2)
	{ "sif_beyondbluevent", "Beyond the Blue Event Wall" },
	{ "sif_realityshow",    "Reality Show" },
	{ "sif_tenyakamemor",   "The Tenyaka Memorial of Vegreville" },
	{ "sif_uglychapter",    "Ugly Chapter" },

	// SpeedIF Orange Competition (TADS2)
	{ "sif_kisschase",      "Kiss Chase" },

	// SpeedIF Spring Training Competition (TADS2)
	{ "sif_hippoflute",     "The Hippo and the Flute" },

	// SpeedIF That Dare Not Speak Its Number Competition (TADS2)
	{ "sif_solid",          "Solid Leather" },

	// SpeedIF Without Number Competition (TADS2)
	{ "sif_zumpffumping",   "Zumpf and Fumping" },

	// SpeedIF y=1/x Competition (TADS2)
	{ "sif_finalassault",   "Final Assault of the Big Green Cliches" },

	// Spring Thing 2005 (TADS2)
	{ "stc05_threnody",    "Threnody" },

	// Swash Comp (TADS2)
	{ "swc_legladymagaidh", "The Legend of Lady Magaidh" },

	// Toaster Competition (TADS2)
	{ "toa_masterstasting", "Masters of Toasting" },
	{ "toa_mrremoteman",    "Mr. Remote Mom" },
	{ "toa_thistoaster",    "This is The Toaster" },
	{ "toa_toasterama",     "Toasterama" },

	// TextFire, Inc. Demos (TADS2)
	{ "tf_coma",            "Coma!" },
	{ "tf_djinn",           "Djinn!" },
	{ "tf_evolution",       "Evolution" },
	{ "tf_george",          "George" },
	{ "tf_inanimator",      "The Inanimator" },
	{ "tf_insomnia",        "Insomnia" },
	{ "tf_jack",            "Jack's Adventures" },
	{ "tf_mastamind",       "Masta'mind" },
	{ "tf_operate",         "Operate!" },
	{ "tf_pumping",         "Pumping!" },
	{ "tf_tenuous",         "A Tenuous Hold" },
	{ "tf_ushockey",        "The U.S. Men's Hockey Team Olympic Challenge" },
	{ "tf_verb",            "Verb" },

	// Walkthrough Competition (TADS2)
	{ "wc_constraint_sg",   "Constraints (by Stephen Granade)" },

	// XYZZY Awards 1996 (TADS2)
	{ "xyz_fms",            "Frobozz Magic Support" },
	{ "xyz_lostny",         "Lost New York" },
	{ "xyz_shelby",         "The Light: Shelby's Addendum" },
	{ "xyz_underoos",       "The Underoos that Ate New York!" },

	// XYZZY Awards 1997 (TADS2)
	{ "xyz_lovesparade",    "Everybody Loves a Parade" },

	// XYZZY Awards 1998 (TADS2)
	{ "xyz_badmachine",     "Bad Machine" },
	{ "xyz_losingyourgrip", "Losing Your Grip, a Journey in Five Fits" },
	{ "xyz_onceandfuture",  "Once and Future" },

	// XYZZY Awards 1999 (TADS2)
	{ "xyz_commonground",   "Common Ground" },
	{ "xyz_fren52",         "The Frenetic Five vs. Mr. Redundancy Man" },
	{ "xyz_worldsapart",    "Worlds Apart" },

	// XYZZY Awards 2000 (TADS2)
	{ "xyz_aboveandbeyond", "Above and Beyond!" },
	{ "xyz_rematch",        "Rematch" },
	{ "xyz_youarechef",     "You are a Chef!" },

	// XYZZY Awards 2001 (TADS2)
	{ "xyz_ftf",            "First Things First" },

	// XYZZY Awards 2002 (TADS2)
	{ "xyz_1893",           "1893: A World's Fair Mystery" },
	{ "xyz_fren53",         "The Frenetic Five vs. The Seven Deadly Dwarves" },

	// XYZZY Awards 2003(TADS2)
	{ "xyz_hamper",         "To Hell in a Hamper" },

	// XYZZY Awards 2005 (TADS2)
	{ "xyz_bbn",            "Bolivia by Night" },
	{ "xyz_findingmartin",  "Finding Martin" },

	// Russian games
	{ "ditchday",           "Ditch Day Drifter" },

	{ nullptr, nullptr }
};

const PlainGameDescriptor TADS3_GAME_LIST[] = {
	{ "tads3",              "TADS 3 Game" },

	// English games
	{ "atopoesis",          "Atopoesis" },
	{ "blendings",          "Blendings" },
	{ "catchthatcat",       "Catch That Cat" },
	{ "combatdemo",         "TADS 3 Combat Demos" },
	{ "comp02tr",           "Comp02ter Game" },
	{ "cookoff",            "Cook Off" },
	{ "darkangel",          "Dark Angel" },
	{ "dearbrian",          "Dear Brian" },
	{ "diabolical",         "Diabolical" },
	{ "entrancingsin",      "Entrancing Sin" },
	{ "goldenbanana",       "The Quest of the Golden Banana" },
	{ "homeopen",           "Home Open" },
	{ "korenvliet",         "Korenvliet" },
	{ "iamprey",            "I Am Prey" },
	{ "liasad",             "Love Is as Powerful as Death, Jealousy Is as Cruel as the Grave" },
	{ "littlestgriffin",    "The Littlest Griffin" },
	{ "lowell",             "Escape from Lowell Prison" },
	{ "lydiasheart",        "Lydia's Heart" },
	{ "macrocosm",          "Macrocosm" },
	{ "marthasbigdate",     "Martha's Big Date" },
	{ "mazes",              "Nothing But Mazes" },
	{ "mylastrodeo",        "My Last Rodeo" },
	{ "natdewey",           "Nat Dewey" },
	{ "officeharassment",   "Office Harassment" },
	{ "pastpresent",        "Past Present" },
	{ "pekostory",          "Peko's Story" },
	{ "ratincontrol",       "Rat In Control" },
	{ "recluse",            "Recluse" },
	{ "revenge",            "The Fiendish Revenge of Baron von Glockenspiel" },
	{ "ribosoperation",     "Dr Who and the Ribos Operation" },
	{ "saturnschild",       "Saturn's Child" },
	{ "sawesome",           "Hide and Seek - Cindy's Something Awesome Project" },
	{ "schattenweg",        "Schattenweg" },
	{ "snowswinter",        "The First Snows of Winter" },
	{ "sod",                "SpeedIF of Destiny or Nothing, Punk" },
	{ "spaceshooter",       "Space Shooter: A TADS 3 abuse" },
	{ "spiegel",            "Durch den Spiegel" },
	{ "stormshelter",       "Shelter from the Storm" },
	{ "thereveal",          "The Reveal" },
	{ "threedaysofnight",   "Three Days of Night" },
	{ "trenchline",         "Trench Line" },
	{ "undertaleepilogue",  "Undertale: Epilogue" },
	{ "venice",             "A Lady in Waiting" },
	{ "vividity",           "Vividity" },
	{ "wardz",              "Ward Z" },
	{ "wilderness",         "In the Wilderness" },
	{ "wolfenschtein",      "Wolfenschtein" },
	{ "writingonthewall",   "The Writing on the Wall" },
	{ "wutheringheights",   "Wuthering Heights" },
	{ "yetanotherexp",      "Yet Another Experiment" },
	{ "youmatched",         "It's a Match!" },

	// IFComp 2002 (TADS3)
	{ "if02_ericgift",      "Eric's Gift" },                                                            //   34th Place

	// IFComp 2003 (TADS3)
	{ "if03_shadowsmirror", "Shadows on the Mirror" },                                                  //    6th Place

	// IFComp 2004 (TADS3)
	{ "if04_squarecircle",  "Square Circle" },                                                          //    5th Place
	{ "if04_imustplay",     "I Must Play" },                                                            //   14th Place

	// IFComp 2005 (TADS3)
	{ "if05_mixtape",       "Mix Tape" },                                                               //   18th Place

	// IFComp 2006 (TADS3)
	{ "if06_elysiumenigma", "The Elysium Enigma" },                                                     //    3rd Place
	{ "if06_xthunt",        "Xen: The Hunt" },                                                          //   17th Place
	{ "if06_brokenman",     "A Broken Man" },                                                           //   26th Place

	// IFComp 2007 (TADS3)
	{ "if07_reconcilmoth",  "Reconciling Mother" },                                                     //   13th Place

	// IFComp 2008 (TADS3)
	{ "if08_aprilparis",    "April in Paris" },                                                         //    8th Place
	{ "if08_magic",         "Magic" },                                                                  //   14th Place

	// IFComp 2010 (TADS3)
	{ "if10_multiversrgue", "Rogue of the Multiverse" },                                                //    2nd Place

	// IFComp 2011 (TADS3)
	{ "if11_it",            "It" },                                                                     //   16th Place

	// IFComp 2012 (TADS3)
	{ "if12_theisland",     "The Island" },                                                             //   25th Place

	// IFComp 2015 (TADS3)
	{ "if15_contentment",   "Koustrea's Contentment" },                                                 //   15th Place

	// IFComp 2016 (TADS3)
	{ "if16_hillridge",     "Hill Ridge Lost & Found" },                                                //    9th Place

	// IFComp 2017 (TADS3)
	{ "if17_fakenews",      "Fake News" },                                                              //   31th Place

	// IFComp 2020 (TADS3)
	{ "if20_captivity",     "Captivity" },                                                              //   16th Place
	{ "if20_edgeofchaos",   "Jay Schilling's Edge of Chaos" },                                          //   25th Place
	{ "if20_deezlebub",     "Deezlebub" },                                                              //   53th Place

	// IFComp 2021 (TADS3)
	{ "if21_ghostwithin",   "Ghosts Within" },                                                          //   28th Place

	// IFComp 2022 (TADS3)
	{ "if22_cain",          "According to Cain" },                                                      //    6th Place
	{ "if22_promdress",     "The Only Possible Prom Dress" },                                           //   28th Place

	// IF Art Show 2004 (TADS3)
	{ "ifas_battle",        "The Battle of Walcot Keep" },

	// Cover Stories Game-Jam (TADS3)
	{ "cs_feathergrange",   "The Feather Grange Job" },

	// IFBeginnersComp 2008 (TADS3)
	{ "ifb_germania",       "Germania" },
	{ "ifb_mrspepper",      "Mrs. Pepper's Nasty Secret" },
	{ "ifb_sleepprincess",  "The Sleeping Princess" },

	// IntroComp 2005 (TADS3)
	{ "ic05_negotis",       "Negotis: Book I" },

	// IntroComp 2007 (TADS3)
	{ "ic07_folkarstation", "Folkar Station" },
	{ "ic07_tin",           "Tin" },

	// IntroComp 2010 (TADS3)
	{ "ic10_closedcircles", "Closed Circles" },

	// IntroComp 2014 (TADS3)
	{ "ic14_devildetails",  "The Devil in the Details" },

	// IntroComp 2015 (TADS3)
	{ "ic15_deprivation",   "Deprivation" },

	// Shufflecomp Competition 2014 (TADS3)
	{ "sh14_deadmansparty", "Dead Mans Party" },
	{ "sh14_lookacorner",   "Look Around the Corner" },

	// SpeedIF 20 Competition (TADS3)
	{ "sif_greatzimbawe",   "Great Zimbawe" },
	{ "sif_miranda",        "Oh Miranda!" },

	// SpeedIF 5th Anniversary Competition (TADS3)
	{ "sif_evilbrain5",     "Evil Brain Five: Dead on the Set" },

	// SpeedIF Crinkle Cut Competition (TADS3)
	{ "sif_dining",         "Dining With the Alien" },

	// SpeedIF of Destiny Competition (TADS3)
	{ "sif_badlandmachine", "Badland Machine" },
	{ "sif_rainbow",        "Painting the Rainbow" },

	// SpeedIF Jacket Competition (TADS3)
	{ "sif_midsummer",      "The Midsummer Banquet" },

	// SpeedIF Jacket 4 Competition (TADS3)
	{ "sif_stomach",        "Light of My Stomach" },

	// SpeedIF Late! Competition (TADS3)
	{ "sif_fishbot",        "Fish-Bot!" },

	// SpeedIF Let's Make a Nightmare Competition (TADS3)
	{ "sif_threedoors_sl",  "Three Doors (by Soren Lovborg)" },
	{ "sif_threedoors_pb",  "Three Doors (by Peter Berman)" },

	// SpeedIF Thanksgiving Competition (TADS3)
	{ "sif_catchfragonfly", "To Catch a Dragonfly" },

	// SpeedIF U Competition (TADS3)
	{ "sif_deathscavenger", "Death's Scavenger Hunt" },

	// SpeedIF Ultimate not Numbered New Year's Competition  (TADS3)
	{ "sif_emma2",          "Emma II" },

	// Spring Thing 2003 (TADS3)
	{ "stc03_parrots",      "Max Blaster and Doris de Lightning" },

	// Spring Thing 2007 (TADS3)
	{ "stc07_camelgirl",    "The Epic Origins of CamelGirl!" },

	// Spring Thing 2012 (TADS3)
	{ "stc12_whitebull",    "The White Bull" },

	// Spring Thing 2018 (TADS3)
	{ "stc18_thehouset3",   "The House" },

	// XYZZY Awards 2002 (TADS3)
	{ "xyz_foreveralways",  "Elizabeth Hawk's Forever Always" },

	// XYZZY Awards 2004 (TADS3)
	{ "xyz_returnditchday", "Return to Ditch Day" },

	// XYZZY Awards 2005 (TADS3)
	{ "xyz_allhope",        "All Hope Abandon" },

	// XYZZY Awards 2007 (TADS3)
	{ "xyz_blightedisle",   "Blighted Isle" },

	// XYZZY Awards 2008 (TADS3)
	{ "xyz_gunmute",        "Gun Mute" },

	// XYZZY Awards 2009 (TADS3)
	{ "xyz_deadlikeants",   "Dead Like Ants" },
	{ "xyz_walkersilho",    "Walker and Silhouette" },

	// XYZZY Awards 2011 (TADS3)
	{ "xyz_indigo",         "Indigo" },

	// Czech games (TADS3)
	{ "exoter",             "Exoter" },

	// German games (TADS3)
	{ "pionierin",          "Die Pionierin und die Tragödie des telematischen Typewriters" },

	// Italian games (TADS3)
	{ "remerook",           "La missione di Kyle Remerook" },

	// Spanish games (TADS3)
	{ "memorias_tads",      "Memorias de reXXe" },
	{ "verla",              "No Quiero Verla" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry TADS_GAMES[] = {
	// ------------------------------------------------- TADS2 Section ------------------------------------------------
	// English games (TADS2)
	DT_ENTRY1("2044", "1.0", "ec9e00db314709ba9616ec1a7f9dd5ea", 48263),
	DT_ENTRY1("3", "1.2", "db9086179d99638372803e5500c3154a", 107073),
	DT_ENTRY1("academy", "1.3", "b9674d0cb50c45ab5631bea1dcdc7999", 336970),
	DT_ENTRY1("allalone", "1.0", "2137e080d79510d0655106e6d9f0d3ae", 154087),
	DT_ENTRY1("amissville", "1.0", "e6ffe067bdb05576e4ac1f4ea96075fa", 1021385),
	DT_ENTRY1("asimpletheft", "1.3", "4e5432f0bed40ec0b04aab92bda10cd6", 74599),
	DT_ENTRY1("asugaredpill", "1.2", "c1cc5d1d481d7dda8184f12c276e2de8", 654151),
	DT_ENTRY1("bmiss", "1.0", "35ca3147c38a022a82048e1aa61562a8", 121787),
	DT_ENTRY1("brokenstring", "1.1", "bb49b4d32b1cd760501ef843ebe43f16", 194718),
	DT_ENTRY1("campingtrip", "1.0", "4989bc609fd770666fc3fd161c36761e", 849755),
	DT_ENTRY1("castleofdoom", "1.0", "5a09aa5ebb8a62f94d9f33e081ef0ef4", 51848),
	DT_ENTRY1("dampcamp", "1.2", "17d330ba999713b65ebafb1527bbc84b", 87845),
	DT_ENTRY1("deanna", "1.1", "34570038fd9661c0c5235e52517644d5", 236165),
	DT_ENTRY1("deepspace", "1.0", "636e58c425e066335c6f92e7bfb0d5f7", 203942),
	DT_ENTRY1("depravitybites", "1.4", "b918fef79d70a12b048d413689d03495", 170029),
	DT_ENTRY1("diemasse", "1.0", "f1c42af81468d8d69f2726e3b56e8449", 211069),
	DT_ENTRY1("ditchday", "1.0", "c8f23c03638639c31f7e9602d09fe9ea", 127622),
	DT_ENTRY1("drool", "1.0", "d2b78f16da2a2f8c51cbbef0e00001a1", 291119),
	DT_ENTRY1("drool", "1.0/Bundled", "d2b78f16da2a2f8c51cbbef0e00001a1", 1955771),
	DT_ENTRY1("e14s", "1.1", "23c3aa566d41d325c1aa474ff6a9360a", 163826),
	DT_ENTRY1("ecdysis", "1.6", "ea91694e025c748af40079c4bbcbf995", 114423),
	DT_ENTRY1("ecdysis", "1.6/Bundled", "ea91694e025c748af40079c4bbcbf995", 141344),
	DT_ENTRY1("edgarhuntly", "1.0", "fb61663dda2a6d520cfc6de106075ca5", 158401),
	DT_ENTRY1("enhanced", "1.3", "fe4e227aaac4f4ab27a77944851603cc", 186737),
	DT_ENTRY1("escapecrazyplace", "1.0", "8540e96ba6475ef8232295b5b82c3ab8", 2299708),
	DT_ENTRY1("eterna", "1.0", "5d837d71e2b19236f9bb8fdb8f57169c", 146228),
	DT_ENTRY1("excuseme", "1.0", "897cfc0406e955f3b87ce95ca4d6df15", 332648),
	DT_ENTRY1("firebird", "1.0.1", "487f019e24cdfbeef733e11d60b18c08", 349270),
	DT_ENTRY1("firewitch", "1.0.1", "cd40d433dfb513d45d9dae58a098d3cd", 154517),
	DT_ENTRY1("foom", "1.01d", "d951126d5e59ee117e35240c24856278", 106100),
	DT_ENTRY1("foom", "1.01d/Debug", "37f19fd1d15ff19c35ffafde36513ff0", 169116),
	DT_ENTRY1("frustration", "1.0.2", "2c7f953e8448d9c0b5989a8233099fb7", 258786),
	DT_ENTRY1("galacticpeace", "1.0", "b7d4c41b0a2225207c3fd39c1bb608a6", 111044),
	DT_ENTRY1("galacticpeace", "1.2", "52fe5f0066910f29b2efd20838b5ed9e", 118852),
	DT_ENTRY1("gc", "0.1.1.2.3", "cfc900f65aaab118ad3621344ea0f45e", 305128),
	DT_ENTRY1("gift2", "1.1", "f703d906afd74d86fc0d823281999be5", 2372163),
	DT_ENTRY1("gift3", "1.0", "3da61fbf2ae0309cfe68633bad57a7aa", 32864239),
	DT_ENTRY1("gna", "1.0", "a556ddb456c3f230b4ac9554a1bcae13", 922981),
	DT_ENTRY1("gna", "1.0/Bundled", "a556ddb456c3f230b4ac9554a1bcae13", 1968152),
	DT_ENTRY1("goldenfleece", "1.0", "0297cc60d93409c33458cc17e26d97cd", 199983),
	DT_ENTRY1("goldenskull", "0.7/Demo", "551fb6d8bac8c169343a33a9211bc055", 1173687),
	DT_ENTRY1("helpfulman", "1.0", "2b73d801672bb49ba75e2eac680ee469", 351592),
	DT_ENTRY1("helpfulman", "1.0/Bundled", "2b73d801672bb49ba75e2eac680ee469", 979505),
	DT_ENTRY1("heroinc1", "1.3", "1d80dd1ffda1270926deb7a1745f9ccf", 148427),
	DT_ENTRY1("holygrail", "1.0.1", "a837f2c5a2ba17361c52f4e567211d7a", 216242),
	DT_ENTRY1("idealneps", "1.1", "516f3cece115c56ae70bc7c34fb17112", 546856),
	DT_ENTRY1("indarkness", "1.0", "365696166db6e16bc4dfaec94ab73534", 489743),
	DT_ENTRY1("inheritance", "1.0", "18e17f76b038b95e1ad99d47a1c5ef55", 106396),
	DT_ENTRY1("insanitycubed", "0.98", "5ed65e5000a2c394eec872ab71ed2ae7", 117209),
	DT_ENTRY1("islandinfinity", "1.1", "c60957d69aaf2b2438fa6ab9185217e5", 248815),
	DT_ENTRY1("islecult", "1.2", "c49319d7a1c1ea485d2cab86b077c647", 411081),
	DT_ENTRY1("journeyking", "1.0", "f68126f75c8134d26ea348b560f7cae2", 310554),
	DT_ENTRY1("katana_tads", "2.0", "bcf79886ddd9869f1e8a19b5cf186075", 828033),
	DT_ENTRY1("kitchenencounters", "0.2", "a73484097e4e24c2226a8361a2dc0009", 70272),
	DT_ENTRY1("letheflowphoenix", "2.0", "e3760cc303082aaa06b54e0b4a6358e8", 272075),
	DT_ENTRY1("letheflowphoenix", "2.0/noIO", "a5e60a545bcad6bbb4ebaf532ae7e745", 272140),
	DT_ENTRY1("lost_jh", "2.0.2", "a30d7caa49f58ed305fc669b4c5b9ecf", 297358),
	DT_ENTRY1("meanstreets", "1.0", "79a6207bbb92faac485e4c4677fa3ab3", 127027),
	DT_ENTRY1("meine", "1.0", "bf118303cd6c599f281070f9177bf15f", 229180),
	DT_ENTRY1("modernism", "1.0", "1a80f4b557d6c7dc94a906baab57edee", 46549),
	DT_ENTRY1("moist", "1.6", "2cb2b56e00df7148b7c40c4753c801d5", 373100),
	DT_ENTRY1("mrsk", "1.0", "c21cedc30ce3d6eadefc2f75f5b1a168", 224542),
	DT_ENTRY1("murmellius2", "1.0", "97df7e025e1a9f1ba1523664ad4ead9c", 105661),
	DT_ENTRY1("museumoftads", "1.2", "829d1af57ff47169ad4d7e6551a22721", 253171),
	DT_ENTRY1("myth_bv", "1.0", "63b1eee145cd8983cb645328c9e78cdf", 161000),
	DT_ENTRY1("nbdemo", "0.99/Demo", "eac25992e40de6aee5e36bc3195f21a9", 112331),
	DT_ENTRY1("ninjaburger", "1.0", "f4edb87ecf3e0e28ff0442a8b03e1dbb", 94376),
	DT_ENTRY1("onefootdown", "1.2", "f8e9be6afe3eb6b5296ebd098f638421", 210220),
	DT_ENTRY1("pasttense", "1.0", "8422454168ee6bc2d45385ce629c5c22", 195897),
	DT_ENTRY1("pcu", "1.0", "4e82b5774dab9948a0331a80075ed7c8", 183145),
	DT_ENTRY1("perditionsflame", "1.0.1", "4b2307fb8ff4095de94c9dcc0b68504e", 334065),
	DT_ENTRY1("perilskies", "1.2", "ad58dc3534cf0c0bc4d16d6cd4205968", 344020),
	DT_ENTRY1("pesach", "1.0", "205aa10d378fc0bd72fad6f4d565ac07", 100556),
	DT_ENTRY1("polyadv", "3.21a", "52430065c2953733074474674a65c411", 884370),
	DT_ENTRY1("ptbad4", "1.0", "81bccf14bd80c405bed38b8935b212dd", 58290),
	DT_ENTRY1("ptbad5", "1.0", "57e824ca2b61bcb7ad89d6410a66ec77", 59858),
	DT_ENTRY1("reeferisland", "1.0", "e1c33e8e06fd335870c1cecec782e02b", 364587),
	DT_ENTRY1("saveprince_tads", "2.0", "2159c4ff547d493f90c0bec0dfa1dcf6", 273825),
	DT_ENTRY1("sd2", "1.1", "eca426e7170cdf3e706b8d310b51ee34", 935510),
	DT_ENTRY1("sd3", "1.0", "4040e1febed0be63e42d9a35bacaef2b", 53083589),
	DT_ENTRY1("sda", "1.0","6fb2b989c85226cc422c511ad404ce48", 166208),
	DT_ENTRY1("sda", "1.0/Bundled", "6fb2b989c85226cc422c511ad404ce48", 622454),
	DT_ENTRY1("seaofnight", "1.2", "c3f6293fa675d74c5ab179f024354720", 95798),
	DT_ENTRY1("secondpit", "1.0", "0aee7dca101b9f20e533ce32a8178535", 370175),
	DT_ENTRY1("sexartist", "1.1", "8256e0b96819b15a2f083d6011c02573", 822794),
	DT_ENTRY1("shadowland", "1.0", "d1a187f4b8acacb5fde1ab9510c5949c", 157551),
	DT_ENTRY1("silenceofthelambs", "1.0", "ca9483a874cf2cf126b6043b222f1ee1", 45157),
	DT_ENTRY1("son0", "1.0", "cebd3ee5cf23fdc5737585c39de8fb3d", 87976),
	DT_ENTRY1("spyderandjeb", "1.0", "4003cfc9734df7ab20377f9fe30365ba", 90655),
	DT_ENTRY1("stranded_jb", "1.0", "74a0aa9b64d910eab42ae038e8477b9c", 376112),
	DT_ENTRY1("suprematismblack", "0.0", "c45e05bd86bc6ae6573810bfaff26a79", 54614),
	DT_ENTRY1("suprematismwhite", "0.0", "caf1756fad48f2cc008986b20e29d82e", 56991),
	DT_ENTRY1("tadsdungeon", "3.1", "5e78e4f4655c42b7bbf32e4088595eb2", 290310),
	DT_ENTRY1("textmetroid", "0.5", "4399099ee07f186ec05972cfdd7b10ea", 56436),
	DT_ENTRY1("theapprentice", "1.0", "dca00dd78cbf7ca1df6916547672f098", 178406),
	DT_ENTRY1("thegate", "1.1", "e88364be7f027da095eaab6a5150575a", 68294),
	DT_ENTRY1("thehouset2", "1.0/gam", "9b45d0ac097e99f6d47a105e52edda16", 83981),
	DT_ENTRY1("themansion", "1.4", "e47b961ffc30828f15551ac738192362", 95515),
	DT_ENTRY1("themission", "1.0.2", "6614dea95ebb3b54e2ee0d3c66a819da", 254145),
	DT_ENTRY1("theoracle", "1.5", "4565348d87a51856558000a018812e25", 172090),
	DT_ENTRY1("tide", "1.1", "ae0026feaf23a2d9987e77901f107622", 89757),
	DT_ENTRY1("tnc", "1.0", "fe79042b10d727c04cd23fb54f185e0a", 898785),
	DT_ENTRY1("tnc", "1.0/Bundled", "fe79042b10d727c04cd23fb54f185e0a", 1034403),
	DT_ENTRY1("tortoise", "1.2", "a6a2d324fabf5594c5950c17c52c7d44", 91936),
	DT_ENTRY1("tylerscube", "1.1", "9be988b677ed05ff12ccc66ab3eda535", 39002),
	DT_ENTRY1("unease", "2.0", "7c1709cb482498e00b9e70da7a33e605", 412858),
	DT_ENTRY1("urbancleanup", "1.0", "b632f5f31e70ebfaf2ad2bd09fda47b0", 149135),
	DT_ENTRY1("veritas", "2.2", "8ae2688ffa2cd25c4eac70c16688105b", 322359),
	DT_ENTRY1("videobar", "1.1", "1f335e8faeddb2736d3d17517a737f25", 1553252),
	DT_ENTRY1("waystation", "1.1", "2e9171818b2fc9bbb7c0ffbe8d97bcbb", 290376),
	DT_ENTRY1("wormhole", "1.0", "3e15f6f2c84eb3a0837f433aa8d0d8a1", 173579),
	DT_ENTRY1("xarbo", "1.0", "24c991db36a721192b5f974f55de1719", 285594),
	DT_ENTRY1("yourchoice", "1.0", "72f5727f770c6d355b6e5de32f05109e", 83855),

	// Adventions games (TADS2)
	DT_ENTRY1("ccr", "2.0g", "2da7dba524075aed0167ae02e6484a32", 291305),
	DT_ENTRY1("ccr", "1.0/v1", "3b500c9e8f09dc4283f33f17cf408961", 165236),
	DT_ENTRY1("ccr", "1.0/v2", "c044ace00b0604169457efa3b616b2b1", 165225),
	DT_ENTRY1("legendlives", "1.1g", "03a651ef3d904dbef6a80cd2c041423c", 622989),
	DT_ENTRY1("legendlives", "1.1", "33d0c4bb222c0267eef1f750526bc57f", 623095),
	DT_ENTRY1("rylvania", "1.0/Demo", "46e36010c320a503e4ca741bc2dbe840", 135174),
	DT_ENTRY1("rylvania", "2.5g", "98027d23f0da0e38c02f1326a2357713", 302903),
	DT_ENTRY1("u0", "1.0/Demo/v1", "41e935188b8a15bd61baaab1650bb015", 119999),
	DT_ENTRY1("u0", "1.0/Demo/v2", "737ec18187c67b3512beb5a5bc98d433", 119989),
	DT_ENTRY1("u0", "1.0g/Demo", "d9615e08336cc0d50984cdc0879f2006", 500949),
	DT_ENTRY1("uhalf", "1.0/v1", "3c2b1ad95dcb1ac885ef80dc6dca4e8e", 143654),
	DT_ENTRY1("uhalf", "1.0/v2", "b86c1887bf5f2fe91c1be49612626f02", 143644),
	DT_ENTRY1("uhalf", "1.0g", "80c18f27f656d7e83d1f3f8075115720", 242269),
	DT_ENTRY1("uu1", "3.0.1g", "705b91f9e54c591e361d6c2e15bff8ef", 367506),
	DT_ENTRY1("uu1", "3.0.1/v1", "2b760b87ce3a268dfb7dbbf8029d7494", 235198),
	DT_ENTRY1("uu1", "3.0.1/v2", "8f408d7a02ff94aab97f7b7c7746f920", 235188),
	DT_ENTRY1("uu2", "3.0g", "69a4f7cd914bd32bd7a3bef5c68f9f3a", 400113),
	DT_ENTRY1("uu2", "3.0/v1", "66405b7ea76d11b2b7b0aae2dc4874ee", 265619),
	DT_ENTRY1("uu2", "3.0/v2", "be6f956a3caf7644246a52bcdb06d41f", 265609),

	// IFComp 1995 (TADS2)
	DT_ENTRY1("if95_zebulon", "1.0", "551fe789d4a0e8c6c78d0b01f7d44a1f", 133098),
	DT_ENTRY1("if95_zebulon", "3.0", "6be895d5655514b5a5b5c97b9b245120", 147473),
	DT_ENTRY1("if95_toonesia", "1.0", "1fb332095f4bae1abb4a5dbd5d3030c1", 125323),
	DT_ENTRY1("if95_toonesia", "1.1", "54f9a3efea8faf6acf99629fdf787ad2", 126627),
	DT_ENTRY1("if95_onegotaway", "1.0", "64de6dda81e4a17bfadd5a9f72edf885", 102144),
	DT_ENTRY1("if95_onegotaway", "1.2", "df697076c90cb474a6ef5cf6fe15b06d", 108570),
	DT_ENTRY1("if95_museumforever", "1.0", "a27ee66cedfcc2e0e83740ebe6e586f9", 124634),
	DT_ENTRY1("if95_undertow", "1.0", "408fae009e457b57fbf56f8770b28b46", 158903),
	DT_ENTRY1("if95_undertow", "1.1", "1a178cde22a95108f5f176c58afa07df", 159487),
	DT_ENTRY1("if95_undo", "1.0.1", "d8be101bce162538bc1713530936e720", 58707),

	// IFComp 1996 (TADS2)
	DT_ENTRY1("if96_smallworld", "1.0", "9fbdb70c1971c73f91bb96a3dfee87aa", 210909),
	DT_ENTRY1("if96_smallworld", "3.0", "74b95dde89a25a61d28d380ab4ea1004", 262067),
	DT_ENTRY1("if96_kissingbuddha", "1.0", "7129c3bed5100c102062d114382e0850", 206403),
	DT_ENTRY1("if96_kissingbuddha", "2.1", "715e9bd3042346f2a179e903ea83763b", 207755),
	DT_ENTRY1("if96_mamoonlight", "1.1b", "724f419a31a2f5e5a2feffc0808e84fe", 211532),
	DT_ENTRY1("if96_mamoonlight", "1.4", "c3f7bb755c0945af26d0579ebd8a087f", 216202),
	DT_ENTRY1("if96_alienabduct", "1.0", "2e6d2e92d3ac37238ae2f5ccc746c479", 145350),
	DT_ENTRY1("if96_alienabduct", "2.0", "1ac6cb8ffd802663e77669c4493347d9", 192983),
	DT_ENTRY1("if96_aayela", "1.1", "bb6263b8cd000959e10a7014d3e53055", 101395),
	DT_ENTRY1("if96_aayela", "1.2", "b12057d8531dd36b899c70f339a01fe1", 110755),

	// IFComp 1997 (TADS2)
	DT_ENTRY1("if97_babel", "1.0", "cb6a76ac9844c654b1f1caeedd33b835", 445757),
	DT_ENTRY1("if97_babel", "3.1", "d0eb2c258829f97a988d168fad6aa891", 465717),
	DT_ENTRY1("if97_glowgrass", "1.0", "64cf5bba500945d68f6ea307636e136e", 148395),
	DT_ENTRY1("if97_sunsavannah", "1.0", "f647c8ba3099b31ba028fb5b49eb73fd", 542323),
	DT_ENTRY1("if97_sunsavannah", "1.0.3", "b73eccfcfab113f27b3a477f894ae882", 547813),
	DT_ENTRY1("if97_zefronalmanac", "1.0", "06d525b222d7835f320f0f68c2738de1", 207599),
	DT_ENTRY1("if97_zerosumgame", "1.0", "6c03e2b27c4b9b1fb60379c609fe6b5b", 260795),
	DT_ENTRY1("if97_zerosumgame", "2.0", "286ede6ac6deaa588d98252ad9207013", 296769),
	DT_ENTRY1("if97_zombietads2", "1.0", "35173cb72e7a334b91adc4014db07e1c", 218278),
	DT_ENTRY1("if97_fren51", "1.0", "6123989ad2b7ad09c476b01373b4a64c", 217954),
	DT_ENTRY1("if97_fren51", "1.1", "584ed64193d4ccfaf4b5bf70f5aa8c4f", 250729),
	DT_ENTRY1("if97_virtuatech", "1.0", "0aa0faeb31db4cef21ed9742b7ef81f9", 180033),
	DT_ENTRY1("if97_aardvark", "1.0", "aafb4bf7e8466989289909d5378473a8", 134750),
	DT_ENTRY1("if97_templeorcmage", "1.0", "337863385f55d10ccfde7d8973496c27", 106134),

	// IFComp 1998 (TADS2)
	DT_ENTRY1("if98_theplant", "1.0", "83fdda1882f9ecba67e5cf175812be8a", 344992),
	DT_ENTRY1("if98_theplant", "2.0", "312894d51640eb9012ec5130059f0b58", 384092),
	DT_ENTRY1("if98_arrival", "1.0", "0f9d5475fdab724814aaace75a44ca84", 236494),
	DT_ENTRY1("if98_arrival", "2.0", "41b1af240774f3ce8217d4acb5deb2dc", 404608),
	DT_ENTRY1("if98_arrival", "2.0/Bundled", "41b1af240774f3ce8217d4acb5deb2dc", 1313912),
	DT_ENTRY1("if98_fourin1", "1.0", "d43ef35c14d6f13d7690a146aea7aabb", 294187),
	DT_ENTRY1("if98_fourin1", "1.1.7", "3ff69b30fdce7042e8d01303302161f3", 304429),
	DT_ENTRY1("if98_lightiania", "1.14", "18f56adbac7c6138f7ee021e3f435562", 128148),

	// IFComp 1999 (TADS2)
	DT_ENTRY1("if99_6stories", "1.0", "f735c4f14a73ea3137311c6956e885af", 738878),
	DT_ENTRY1("if99_6stories", "1.0/Bundled", "f735c4f14a73ea3137311c6956e885af", 8848904),
	DT_ENTRY1("if99_exhibition", "1.0", "a0e68de3e083e8243b720b7111048f5c", 685560),
	DT_ENTRY1("if99_exhibition", "2.1", "53e06a49a8902bb5b7502bbeb5fcce08", 4380469),
	DT_ENTRY1("if99_onthefarm", "1.0", "90dfd97cc2bcdf8e03188a41eee3367a", 415513),
	DT_ENTRY1("if99_erehwon", "1.0", "c8750c6b35b66dd2b023e76d9406b834", 331155),
	DT_ENTRY1("if99_bliss", "1.1", "fc7df7d4c37791809e2c5aea17f11a1b", 176984),
	DT_ENTRY1("if99_stonecell", "1.0", "11cc1f1856f1decea2c81385cd6210b5", 293917),
	DT_ENTRY1("if99_4seconds", "1.0", "ac7e459341638fed6820e3da7575e747", 216337),
	DT_ENTRY1("if99_amoment", "0.258", "275361e20951b1574f2f72bdb62917c7", 144910),
	DT_ENTRY1("if99_chaostads2", "1.0", "1d6d02724efccddff92c6b65ab274d39", 230267),
	DT_ENTRY1("if99_strangernight", "1.0", "25fa5289d2e36b1b4dc470cf5631aace", 162611),
	DT_ENTRY1("if99_bealstreet", "1.0", "45da11585f6394a05eb7a36899da605d", 52172),
	DT_ENTRY1("if99_thewaterbird", "1.0", "4d27e2329e3c61f144642f8923392aff", 279278),
	DT_ENTRY1("if99_thewaterbird", "1.1", "49aa9dd604d21d6cc218ba72fb18a214", 280832),

	// IFComp 2000 (TADS2)
	DT_ENTRY1("if00_kaged", "1.0", "3ed582efc72202b6ac8301cc8cfcc901", 663472),
	DT_ENTRY1("if00_kaged", "1.0/noSound", "94cfb4696ecb029b4a3ad354d0e5e842", 1031879),
	DT_ENTRY1("if00_kaged", "1.0/Bundled", "3ed582efc72202b6ac8301cc8cfcc901", 12780406),
	DT_ENTRY1("if00_atwitsend", "1.0", "fe49c71432851840f588d1dd23715b14", 447784),
	DT_ENTRY1("if00_atwitsend", "1.2", "538e51543e14ea8fd69847ae070af1ab", 448382),
	DT_ENTRY1("if00_planmind", "1.0", "c151c997806f470a2427a0db65798afb", 336080),
	DT_ENTRY1("if00_endmeans", "1.0", "a975506eff55062629507ebf19b4c8a8", 277978),
	DT_ENTRY1("if00_futzmutz", "1.0", "31c19a82353dd28c3521cb2c9cced1b9", 1234778),
	DT_ENTRY1("if00_unnkuliax", "1.0", "7599879366ab32cdefd5992963665cfd", 854310),
	DT_ENTRY1("if00_unnkuliax", "1.1", "20b3b72ec1143f8ddb91e19dd209e9a0", 885548),
	DT_ENTRY1("if00_desertheat", "1.0", "9ea2d5f04ca85ccdd96fff69293e1532", 115680),
	DT_ENTRY1("if00_thetrip", "1.31", "161affd0867ddd22609f5f4bb5b07ace", 171738),
	DT_ENTRY1("if00_aftermath", "1.2", "f9f36bee9ecd75ff46fcb8bc8c969bfc", 118957),
	DT_ENTRY1("if00_theclock_ck", "1.0", "4b93335eb0f47fde1a5041c278147bba", 147383),
	DT_ENTRY1("if00_threadlaby", "1.0", "272ef80c30dd6c8f36bd6713791c04bc", 66873),
	DT_ENTRY1("if00_jarodsjourney", "1.0", "c87e592ba6123cbc2cfeddf43acf9bfd", 259778),
	DT_ENTRY1("if00_jarodsjourney", "1.0/Bundled", "c87e592ba6123cbc2cfeddf43acf9bfd", 771675),

	// IFComp 2001 (TADS2)
	DT_ENTRY1("if01_ntts", "1.0", "c7940d02a1564ea11b33e61096f8d64f", 581866),
	DT_ENTRY1("if01_ntts", "1.3", "32b0b4ccc1d208e7d163790074e434ce", 601033),
	DT_ENTRY1("if01_beetmonger", "1.0", "a8d5e2317dcaaca68903ae86732c4dd6", 458384),
	DT_ENTRY1("if01_beetmonger", "1.1", "e24f782fba8db13fac45669b7fb31945", 463248),
	DT_ENTRY1("if01_triune", "1.0", "ac041944a4a495470c02c55c12737380", 230133),
	DT_ENTRY1("if01_journeyislet", "1.0", "b96a6b4dca2da4735afef839bb4d2cf0", 186001),
	DT_ENTRY1("if01_grayscale", "1.0", "2da9e3535359b38a9b3dd00edf222d54", 274357),
	DT_ENTRY1("if01_coasthouse", "1.0", "b0bc168dd25f86e136365d7649a40848", 147069),
	DT_ENTRY1("if01_coasthouse", "1.1", "ba255c85d9565aef0c84e209cbcc7115", 151718),
	DT_ENTRY1("if01_nightguest", "1.0", "a6c7243f2fe0d8120f29b7f1d3cae4e1", 252078),
	DT_ENTRY1("if01_nightguest", "1.1", "b4d64bfece91b350475c93b27e58efd5", 261550),
	DT_ENTRY1("if01_fusillade", "1.01", "1a1acf187c38a6f086e727cd310458dd", 261608),
	DT_ENTRY1("if01_fusillade", "1.11", "a2489852bcf124e2c65a0588c7c8e86f", 268065),
	DT_ENTRY1("if01_fusillade", "1.11/Bundled", "a2489852bcf124e2c65a0588c7c8e86f", 356756),
	DT_ENTRY1("if01_thecruise", "1.0", "7709883e32e676cd11877e41946f4093", 188093),
	DT_ENTRY1("if01_thecruise", "1.1", "fd7cae80b84d0229f51f0f647ba674f7", 192995),
	DT_ENTRY1("if01_kallisti", "1.0", "d8a769816ccf34521245a223fa05285b", 230560),
	DT_ENTRY1("if01_stranded_rc", "1.0", "49d4e44e5d12befd50c73ba92b27b0d7", 3190761),
	DT_ENTRY1("if01_volcanoisle", "1.0", "712569455e0fddbb8124c11859b8c430", 106034),

	// IFComp 2002 (TADS2)
	DT_ENTRY1("if02_tilldeath", "1.0", "4cab236618031442869d404e4537be83", 444985),
	DT_ENTRY1("if02_tilldeath", "1.1", "346c068e777e6bddfd85ad851bbe8809", 566548),
	DT_ENTRY1("if02_augustine", "1.0", "864180d2bb590998e4c242bf67c7487c", 308700),
	DT_ENTRY1("if02_granite", "1.0", "89bdac8d148f0a9553bd01288abb645f", 132722),
	DT_ENTRY1("if02_evacuate", "1.0", "7ba97bb350f7eb3ff1f47a49f14b58da", 358701),
	DT_ENTRY1("if02_notmuchtime", "1.48", "51e5c128f02e7b191dad308ed830f521", 130305),
	DT_ENTRY1("if02_color", "2.0", "910fcde7b18c2d977ecbed192010dab2", 157316),
	DT_ENTRY1("if02_concrete", "1.0", "ff9f0fa9dc507eb2de7c77589f33eb0d", 182507),
	DT_ENTRY1("if02_coffeequest", "1.0", "026d356733e30c38f276e0ee012e4b3a", 189784),
	DT_ENTRY1("if02_coffeequest", "1.1", "9caa2da66fe0d510adc94c775edc00c5", 190195),
	DT_ENTRY1("if02_moonbase", "1.0", "7e4184dfcc90e004ec8e6ec131c2970e", 167456),
	DT_ENTRY1("if02_terriblelzrds", "0.1", "fd183bd09a42c443eaca522134986963", 119709),

	// IFComp 2003 (TADS2)
	DT_ENTRY1("if03_scavenger", "1.0", "2553c8ebd7f082a40e40ef26b03cefd4", 455592),
	DT_ENTRY1("if03_scavenger", "1.2", "17b62ca221ad5f046285594a57d6c0c0", 473137),
	DT_ENTRY1("if03_erudite", "1.0", "50fcd4afcd5428a3d5b8f3efd286ecef", 171627),
	DT_ENTRY1("if03_recruit", "1.0", "31014da30034cc9944883147ecbd16d9", 354325),
	DT_ENTRY1("if03_cerulstowaway", "1.0", "8f1a19a3dda4375f5a6124e8b5b668d8", 217023),
	DT_ENTRY1("if03_cerulstowaway", "1.1", "46e9b2205a5cf092b22ff68fae254c0f", 244366),
	DT_ENTRY1("if03_episodeartist", "1.0", "d8300f961aa7da8a4da08a6f6f625f55", 148778),
	DT_ENTRY1("if03_templekaos", "3.3.1", "eb33ebf4b168e1d8bfd4f9db4230d139", 146903),
	DT_ENTRY1("if03_delvyn", "1.0", "9b554afba03ed55f1d35f9aaf8cf22bd", 6189357),
	DT_ENTRY1("if03_bio", "1.0", "9289d17fc6f7bf747d65d0b08c37730a", 85243),
	DT_ENTRY1("if03_amnesia", "1.0", "55edf56ee543c5effe0c59e41522fa7f", 73764),

	// IFComp 2004 (TADS2)
	DT_ENTRY1("if04_magocracy", "0258", "20f9e89974fe00869ceec9e98723913c", 402483),
	DT_ENTRY1("if04_kurusu", "1.0", "b14756bfd4b51a4a20104a677b47cd71", 412310),
	DT_ENTRY1("if04_whocreated", "1.0", "7c0b5a10b28f6ded14643ab84eb6c457", 147729),
	DT_ENTRY1("if04_realm", "1.0", "4bbcf78457d08b8b6b2a9af00d6aede7", 97047),
	DT_ENTRY1("if04_redeye", "1.0", "ffb7bb53d27236353a14b1bc27aac3b0", 124488),
	DT_ENTRY1("if04_zero", "1.0", "0a42c68db0b109be6451684a8f4dea33", 591050),
	DT_ENTRY1("if04_lighttale", "1.0", "387a9f94032bdc8c8d964b936dfa2c73", 190412),
	DT_ENTRY1("if04_ruinedrobots", "1.0", "3de3c8e1c9b2c35f7da898cc4d8c0ebc", 2603047),
	DT_ENTRY1("if04_ptbad3", "1.0", "f5bb7f9a08ab56b00ff998e14763499c", 63804),

	// IFComp 2005 (TADS2)
	DT_ENTRY1("if05_chancellor", "1.15", "c757d3110ba6e63ad48b8bc896a47f23", 534819),
	DT_ENTRY1("if05_gilded", "0.9", "8a75e93945a591092d4cc6dc3005ffee", 1749468),
	DT_ENTRY1("if05_xen", "1.0", "a1d05850cf2a40a5ac7ed07db4930a77", 314751),
	DT_ENTRY1("if05_offthetrolley", "1.0", "03a0b14075aae7f7fe01632f1f89c137", 121435),
	DT_ENTRY1("if05_sabotagecldr", "1.0", "c6f031da845d02cfd23fc5d17936d35e", 322692),
	DT_ENTRY1("if05_sabotagecldr", "1.1", "181c7bf3a7928443b185f8e7025b7c22", 324350),
	DT_ENTRY1("if05_amissville2", "1.0", "3c7e430e4eec386f7a98a5bab3eeabb4", 1397498),

	// IFComp 2006 (TADS2)
	DT_ENTRY1("if06_ballymunadv", "1.0", "dffae8952dcbd99fe6fa16ef0552a5bf", 339491),
	DT_ENTRY1("if06_ballymunadv", "1.1", "9ec0690e89491e1407b3170ad31d814a", 344557),
	DT_ENTRY1("if06_lawnoflove", "1.0", "943860882bc02d355c79fb1524fb8528", 392467),

	// IFComp 2007 (TADS2)
	DT_ENTRY1("if07_amoi", "1.0", "3690d1f9cd3b4e18c796cf6efddb9e90", 465679),
	DT_ENTRY1("if07_amoi", "1.1", "f47a0612771c0e84589ffde45aa864f7", 469943),
	DT_ENTRY1("if07_slapthatfish", "1.0", "aa691393c9ae2c6cd3bcea8786c7aa6f", 130276),
	DT_ENTRY1("if07_slapthatfish", "1.1", "aa8a6c502f1f1582654c0942ad2231be", 144235),

	// IFComp 2008 (TADS2)
	DT_ENTRY1("if08_berrost", "1.0", "a01dcea10a6fb4adbc2de02c5ec136d1", 413213),

	// IFComp 2011 (TADS2)
	DT_ENTRY1("if11_blind", "1.2", "3384167b73a9fd1d04c5c66e687f81be", 160021),
	DT_ENTRY1("if11_blind", "1.6", "c9cabab152fd5201b5537c0d1094e3b8", 163549),
	DT_ENTRY1("if11_fogconvict", "1.0/v1", "aeb620908d62a9dc0b7998fc5dc1679c", 181571),
	DT_ENTRY1("if11_fogconvict", "1.0/v2", "fd9a83749665f2c41c558c84a5c81445", 181621),

	// IFComp 2013 (TADS2)
	DT_ENTRY1("if13_wizardapprent", "1.0", "038e198096f35104c2fa079c7e649e31", 174079),

	// IFComp 2017 (TADS2)
	DT_ENTRY1("if17_antiquest", "2.0", "6afcdace2f779a8bae906221429868f0", 15179291),

	// IF Art Show Spring 1999
	DT_ENTRY1("ifas_pillow", "1.0", "fc2939b8afb6faca09f2a9872dbea4de", 53292),
	DT_ENTRY1("ifas_pillow", "1.1", "fba7a5ec0ef13fc72851cbbf17a4e014", 53312),

	// IF Art Show 2001
	DT_ENTRY1("ifas_memories", "1.0", "9cc30865affb4ea5bd94389f6affa4f5", 113712),

	// IF Art Show 2003
	DT_ENTRY1("ifas_friendlyfoe", "1.0", "194719a7f6453e4a94636cce6819aea4", 238026),
	DT_ENTRY1("ifas_friendlyfoe", "1.1", "4db42d8e06d3e3ad1bbba4b57f322555", 237119),
	DT_ENTRY1("ifas_tarotreading", "1.0", "2abc9ca99977445eb402acccbe79cfdb", 118613),

	// IF Art Show 2004
	DT_ENTRY1("ifas_lastride", "1.0", "0886dab323caeff5d9b8f1f6d1bf08da", 100290),

	// Arcade Collection (TADS2)
	DT_ENTRY1("ifa_digdug", "1.0", "ce9cf2db855889eebc61a4c2811eb5d0", 79006),
	DT_ENTRY1("ifa_mc", "1.0", "4b4bd659c73ca676439c8ea94812bceb", 63644),
	DT_ENTRY1("ifa_pong", "1.0", "be32299d93db3c89f6ef37e8b2c2f478", 106572),
	DT_ENTRY1("ifa_spaceinvaders", "1.0", "de8d6eb1334fd45cbfd6c0f7b2d7ad63", 62274),
	DT_ENTRY1("ifa_tapper", "1.0", "affeb36fb4526366278161e2dc541958", 68007),
	DT_ENTRY1("ifa_tilt", "1.0", "3f8face457fd587a832e35d17281e4c7", 72756),

	// Chicken Competition (TADS2)
	DT_ENTRY1("cc_landing", "1.0", "66b27874e17d83011209a62d393e0973", 106998),
	DT_ENTRY1("cc_landing", "1.2", "88ecef018ef64b8456c13e894c9d60c2", 875620),
	DT_ENTRY1("cc_learningtocross", "1.0", "43a2d3db419b910636456dc6e0aa30d9", 96996),
	DT_ENTRY1("cc_learningtocross", "1.2", "7e17454f20f5f97f06d34ce77e6d3d8c", 110804),
	DT_ENTRY1("cc_sisychickenphus", "1.0", "5814a42c913f6fd082c3ed066af76bc1", 57361),

	// Dinosaur Mini-Competition (TADS2)
	DT_ENTRY1("dino_deepbrowlift", "1.0", "7cfe9edfab551d7f34acdf188adb4b09", 80154),
	DT_ENTRY1("dino_dinolove", "1.0", "5fa9e9284d6eb73706989adec3864701", 109257),

	// Dragon Mini-Competition (TADS2)
	DT_ENTRY1("dr_dragrscstories", "1.2", "afdc8bf191e17b6754e8e366735a6f17", 133845),

	// IntroComp 2002 (TADS2)
	DT_ENTRY1("ic02_awea", "1.0", "62411a7bcfa8589e0a13b7e58a958266", 177711),
	DT_ENTRY1("ic02_maintenman", "1.0", "fb843aec6bc44834d27a88895b086dca", 100845),
	DT_ENTRY1("ic02_privcyborg", "1.0", "ccf08638438879cdd41244707208f06c", 82552),
	DT_ENTRY1("ic02_virus", "1.0", "75570350ec294d8a8f7000d378020956", 100770),

	// IntroComp 2004 (TADS2)
	DT_ENTRY1("ic04_betsabet", "1.0", "6f83d92e97af64ab0685b7c24bc279bd", 132067),

	// IntroComp 2010 (TADS2)
	DT_ENTRY1("ic10_flooby", "1.0", "1b23e1230dce5a891447952ee0f42e0f", 103287),

	// IF Library Competition 2002 (TADS2)
	DT_ENTRY1("lc_dwenodon", "1.0", "eefe52977d95b395228e7b9e47e3fb86", 201301),
	DT_ENTRY1("lc_passingfamiliar", "1.0", "206265ad6aa7b91d32d0bc458465d6f6", 169143),

	// LOTECH Competition 2001 (TADS2)
	DT_ENTRY1("lo01_escplanmonk", "1.0", "a4ebb7d388ba6c6c9a24fd187a5fc845", 71162),
	DT_ENTRY1("lo01_escplanmonk", "1.0/Bundled", "a4ebb7d388ba6c6c9a24fd187a5fc845", 342810),
	DT_ENTRY1("lo01_oneweek", "1.0", "6e16b02d7d1043bae575622104f4bef4", 60679),

	// Manos Mini-Competition (TADS2)
	DT_ENTRY1("ma_foggywood", "1.1", "6c24d1720edd40efe1da8261cc82d3b4", 130507),

	// Segment Mini-Comp (TADS2)
	DT_ENTRY1("snowman", "1.0", "29d4bf8c9918437074f6a976c78191e1", 70780),

	// SmoochieComp 2001 (TADS2)
	DT_ENTRY1("sc_secondhoneymoon", "1.0", "5b2725191941761e75ffdfb55867176b", 84651),

	// SpeedIF 2 Competition (TADS2)
	DT_ENTRY1("sif_kidsdonteat", "1.0","3d563f89fbb0abd107dc25a66a7d743e", 53374),

	// SpeedIF 3 Competition (TADS2)
	DT_ENTRY1("sif_moralmethis", "1.0", "cc65c4c01aa86570043a50251985710f", 48328),

	// SpeedIF 4 Competition (TADS2)
	DT_ENTRY1("sif_onion", "1.0", "1629e305aef4eeff0737071fef4c5915", 52748),
	DT_ENTRY1("sif_onion2", "1.0", "a35dd6a99c3ae8986272234f2b0981da", 100407),
	DT_ENTRY1("sif_untitled", "1.0", "dfc37dc2b6ab12cf175b4d87e24095bd", 59430),

	// SpeedIF 5 Competition (TADS2)
	DT_ENTRY1("sif_sludge", "1.0", "59707130a98c9c191d8698058b469465", 140224),
	DT_ENTRY1("sif_speed5", "1.0", "04d796dd1f9782ab61b4ace31491c632", 55871),
	DT_ENTRY1("sif_spysnow", "1.0", "380719602d357e250c4f87ee15062cda", 49981),

	// SpeedIF 5.5 Competition (TADS2)
	DT_ENTRY1("sif_bhy", "1.3", "88fb8848d4680c5961e9c1328f5be17f", 101829),
	DT_ENTRY1("sif_dontfire", "1.0", "e7e21c485270af37702094899b24dfed", 57707),
	DT_ENTRY1("sif_rock", "1.0", "5d69967e1af5095a0a806ca9e3f2b62c", 80344),
	DT_ENTRY1("sif_rockdsg", "1.0", "063aa686e2cccb950c6c26b910079a5f", 50266),
	DT_ENTRY1("sif_rockie", "1.0", "a211279f43d06d2ddaf36a3948738d0d", 105470),
	DT_ENTRY1("sif_yourmind", "1.0", "708befb078f552920f63c8deabf70389", 54441),

	// SpeedIF 6 Competition (TADS2)
	DT_ENTRY1("sif_trudge", "1.0", "117af783b614ca139c5b5ce5c95b4ebd", 128546),

	// SpeedIF 7 Competition (TADS2)
	DT_ENTRY1("sif_bloodymess", "3.1415a", "f6f27a3ec0e42fdcc4531e42b1f5f2ae", 108213),

	// SpeedIF 8 Competition (TADS2)
	DT_ENTRY1("sif_pants", "1.0", "e2b69c8a76e4f86a2e6f6a589014788b", 48319),

	// SpeedIF 9 Competition (TADS2)
	DT_ENTRY1("sif_legbreak", "1.0", "177431a31e54d6f5e2beebc520eafc26", 56475),

	// SpeedIF 10 Competition (TADS2)
	DT_ENTRY1("sif_freedom", "1.0", "7e406dcda3d0cd0e344e073205320a0d", 108931),

	// SpeedIF 10^-9 Competition (TADS2)
	DT_ENTRY1("sif_undergrcomp", "1.0", "850c8ed4b3265834dcc8c1b9452a4a40", 60325),

	// SpeedIF 11 Competition (TADS2)
	DT_ENTRY1("sif_funandgames", "1.0", "cdebfbc72aa8cc113b671ee0ed7ba7de", 62850),
	DT_ENTRY1("sif_livingroom", "1.0", "737e0043fcab59c6671be5a40d00fe5f", 107038),
	DT_ENTRY1("sif_stairs", "1.0", "991780c049782f3fc2da3d84ad878d9f", 106309),

	// SpeedIF 12 Competition (TADS2)
	DT_ENTRY1("sif_hose", "1.0", "750482733ff51dff456d7866e88ab7af", 108723),
	DT_ENTRY1("sif_scurvygift", "1.0", "7b99f1dbd29d43a227acf0a2c731ae58", 92893),

	// SpeedIF 14 Competition (TADS2)
	DT_ENTRY1("sif_deathwaif", "1.0", "671c3a3fbdb608c42a21911eb15e7d09", 109614),

	// SpeedIF 15 Competition (TADS2)
	DT_ENTRY1("sif_dangerschool", "1.0", "e8b07fd03deb01fd8894e31ce1204329", 65679),
	DT_ENTRY1("sif_fivetutors", "1.0", "0fb3bf04b3f2bb32f52c841227bc3909", 63151),
	DT_ENTRY1("sif_ralph_dj", "1.0", "c4a72615ff6558bb356a49706f0da3ea", 54045),
	DT_ENTRY1("sif_weavingbasket", "1.0", "8c97f5d18c2efb123fcd61a69eff832f", 59385),

	// SpeedIF 17 Competition (TADS2)
	DT_ENTRY1("sif_stupidgame", "1.0", "2d5bada55db9d3d27d8a9127c7bb6c36", 60369),
	DT_ENTRY1("sif_whydoesalways", "1.0", "f086af359cfda5bad30b1bebbc08d8fc", 65964),

	// SpeedIF 18 Competition (TADS2)
	DT_ENTRY1("sif_manna", "1.0", "3395b240ddf498c1e73267f4636a4bac", 72077),
	DT_ENTRY1("sif_suffersuppl", "1.0", "363a628726480d6b8a2fc8f1c56fba92", 61376),
	DT_ENTRY1("sif_zymurgy", "0.1", "463ef2c005a72795ca435f0a66482a08", 65907),

	// SpeedIF 20 Competition (TADS2)
	DT_ENTRY1("sif_21points", "1.0", "7d8b4aa74133b0d68d541cfdb7902969", 59713),

	// SpeedIF -1 Competition (TADS2)
	DT_ENTRY1("sif_screw", "1.0", "49081a05bcd719f20f53bde81a9cdb29", 58221),

	// SpeedIF 2000 Competition (TADS2)
	DT_ENTRY1("sif_diorama", "1.0", "c506057bba2d67ecc3c524079c56a3b8", 102771),
	DT_ENTRY1("sif_lionwinter", "1.0", "7933e408b4a74a098646711c1f79a598", 75920),

	// SpeedIF 2001 Competition (TADS2)
	DT_ENTRY1("sif_talltales", "1.0", "8dd0cc1ed3292b73765599e17ffd7e75", 59985),

	// SpeedIF 3rd Anniversary Competition (TADS2)
	DT_ENTRY1("sif_chartage", "1.0", "0eb90637111bfc8d1facbf1ebd010b4f", 66303),
	DT_ENTRY1("sif_leatherworld", "0.1", "7590eefaa43cc946192fc121e7bf06fb", 63613),

	// SpeedIF 5th Anniversary Competition (TADS2)
	DT_ENTRY1("sif_brainrain", "1.0", "2f1817baf1de163b0dad23f05da2cec4", 61823),
	DT_ENTRY1("sif_casinoviridoso", "1.0", "d1df4ffb43958a72eada2d2961a90150", 59147),
	DT_ENTRY1("sif_happen", "1.0", "8bd4dd1073a6b072b8b5ca1be05b85a3", 129147),

	// SpeedIF Apocalypse Competition (TADS2)
	DT_ENTRY1("sif_speedapocalyp", "1.0", "510b1d6146a5332ac6b85449fa1adccd", 103603),

	// SpeedIF Argonaut Competition (TADS2)
	DT_ENTRY1("sif_ascii", "1.0", "501ee556bd4bfc3280337a9b4380a688", 57160),

	// SpeedIF Autocratic Competition (TADS2)
	DT_ENTRY1("sif_atthetop", "1.0", "daec6219ab92bc7cfbfe3ea7ba188737", 84926),
	DT_ENTRY1("sif_digging", "1.0", "f1288bec43f362147e083a0678b4259d", 58353),

	// SpeedIF Crinkle Cut Competition (TADS2)
	DT_ENTRY1("sif_dithyramb", "1.0", "f7244418f4b3fc51e5a964905fa9bd0c", 69290),
	DT_ENTRY1("sif_glacialrift", "1.0", "1ac7fb782cae5fccb16af21653a6e9ed", 116400),

	// SpeedIF Douglas Adams Tribute Competition (TADS2)
	DT_ENTRY1("sif_goodnight", "1.0", "52e8ee20cb2b77826724aea7585e5cb3", 80525),
	DT_ENTRY1("sif_jeenin", "1.0", "4f36687f53a00916cf9943a7a9fc7791", 63972),

	// SpeedIF EXTREME Competition (TADS2)
	DT_ENTRY1("sif_weirdsister", "0.1", "163c6d608f1e072560215cb1600de4b5", 64477),

	// SpeedIF >= 20 Competition (TADS2)
	DT_ENTRY1("sif_cranial", "1.0", "cab17d1cf54d669c579a5239d590c71e", 56319),
	DT_ENTRY1("sif_thing", "1.0", "cad56abd1aed0c43e032b08feac2ebb1", 120964),
	DT_ENTRY1("sif_tppwwe", "1.0", "5ceb6dc202ad1d7f7c73b95650bfeb57", 62401),

	// SpeedIF Halloween 2002 Competition (TADS2)
	DT_ENTRY1("sif_gummibear", "1.0", "2f53060f705838dc1349fcd43cf03c8f", 61057),
	DT_ENTRY1("sif_rivkin", "1.0", "c860a9342222e3dfaeece67428c7d823", 64425),
	DT_ENTRY1("sif_yellowdog", "1.0", "7aaca5f604b18869b6e1313dab5a7724", 82144),

	// SpeedIF Jacket Competition (TADS2)
	DT_ENTRY1("sif_spifftime", "1.0", "2def409972bb06e7729e9ac2ac20155d", 67274),

	// SpeedIF Jacket 2 Competition (TADS2)
	DT_ENTRY1("sif_beyondbluevent", "1.0", "e9f8b4346fc9009bfc24e4117693da7a", 58569),
	DT_ENTRY1("sif_realityshow", "1.0", "aa5e69b378aaae8ccfe797c9778dffd9", 72801),
	DT_ENTRY1("sif_tenyakamemor", "1.0", "25d78c39727d7a0e1bbc8f1fdd12b3b9", 206759),
	DT_ENTRY1("sif_uglychapter", "1.0", "c9a16188b427f731293dc4f9759eccaa", 73931),

	// SpeedIF Orange Competition (TADS2)
	DT_ENTRY1("sif_kisschase", "1.0", "62462239930cd8c6ca7f8fb7c788121d", 69410),

	// SpeedIF Spring Training Competition (TADS2)
	DT_ENTRY1("sif_hippoflute", "1.0", "f56d902febfa1a65437f7057cb2d62f8", 62077),

	// SpeedIF That Dare Not Speak Its Number Competition (TADS2)
	DT_ENTRY1("sif_solid", "1.0", "78ab35226b8da2e6fe5f0ca8fcfb9faa", 57456),

	// SpeedIF Without Number Competition (TADS2)
	DT_ENTRY1("sif_zumpffumping", "1.0", "7c047a9d690ac4af10e8dcda3de2ac25", 60616),

	// SpeedIF y=1/x Competition (TADS2)
	DT_ENTRY1("sif_finalassault", "1.0", "80205156dacdd0911fbd547659f48d1a", 60487),

	// Spring Thing 2005 (TADS2)
	DT_ENTRY1("stc05_threnody", "1.0b", "16861bc8f7c1f1b50cbdbd1c68199a9f", 1014120),
	DT_ENTRY1("stc05_threnody", "1.0b/Bundled", "16861bc8f7c1f1b50cbdbd1c68199a9f", 2965924),

	// Swash Comp (TADS2)
	DT_ENTRY1("swc_legladymagaidh", "1.0", "40f9b39e723c6034f2b99fd476f25677", 99175),

	// Toaster Competition (TADS2)
	DT_ENTRY1("toa_masterstasting", "1.0", "3c8d9e05c13451d856841f34875aa802", 84250),
	DT_ENTRY1("toa_mrremoteman", "1.0", "b1dd514ad5890a4e99e284e58182d69d", 78448),
	DT_ENTRY1("toa_thistoaster", "1.0", "c7289ed18efc3422469cd51a369c8b37", 146448),
	DT_ENTRY1("toa_toasterama", "1.0", "4424146b195458f64fb79fecfaacc6ab", 70489),

	// TextFire, Inc. Demos (TADS2)
	DT_ENTRY1("tf_coma", "1.0/Demo", "c1aee6421d0e3e7d218d74edb5e3b2c8", 68931),
	DT_ENTRY1("tf_djinn", "1.0", "ec1150b9ee767943c195cbf0e5d8cb45", 59792),
	DT_ENTRY1("tf_evolution", "1.0", "a2a8533b18df30dbcfc1fdd37b2cadfc", 51917),
	DT_ENTRY1("tf_george", "1.0", "078d3a9e3b41629d099ddc5024009235", 76300),
	DT_ENTRY1("tf_inanimator", "1.0/Demo", "c25880c63c468edc2082cce813a732ea", 55324),
	DT_ENTRY1("tf_insomnia", "1.0/Demo", "c21b05cd3d9b24e2f9550e8555b5c8d6", 81450),
	DT_ENTRY1("tf_jack", "1.0/Demo", "afb257aa5453b3066260f6315ca5fb6b", 57747),
	DT_ENTRY1("tf_mastamind", "1.0/Demo", "1b673cd9add2a57862f19a5fad59db0f", 125003),
	DT_ENTRY1("tf_operate", "1.0/Demo", "011ffe7cf876b02ea5cc07fe98c85145", 75645),
	DT_ENTRY1("tf_pumping", "1.0/Demo", "eda03fe5a7fde843006d7ad064c23060", 58275),
	DT_ENTRY1("tf_tenuous", "1.0/Demo", "a4c69a58b1b86321ed5c651d30f634de", 84537),
	DT_ENTRY1("tf_ushockey", "1.0/Demo", "0fb4b1c3694e89d2d09bbb9a2fdd3da7", 71044),
	DT_ENTRY1("tf_verb", "1.0/Demo", "8a9f07254c9b973f7cede3b8998d0974", 67217),

	// Walkthrough Competition (TADS2)
	DT_ENTRY1("wc_constraint_sg", "1.0", "d45255fa150e4506b4f84c05cab486aa", 176478),

	// XYZZY Awards 1996 (TADS2)
	DT_ENTRY1("xyz_fms", "1.4", "b15de56d4c675b4397bb73a71e66a6e9", 165365),
	DT_ENTRY1("xyz_lostny", "1.04", "d886ceec1b31eb5c0696db83dfaffd40", 417240),
	DT_ENTRY1("xyz_lostny", "1.4", "93212a4f1feac069eb13d97f53c41444", 434674),
	DT_ENTRY1("xyz_shelby", "2.0", "ee68b303594b964eddec7cdc0ae4dc10", 378069),
	DT_ENTRY1("xyz_shelby", "2.1", "0d39d9271cced70cba53426d17d197b8", 398947),
	DT_ENTRY1("xyz_underoos", "1.0", "cb1ce9f0228021292d5fd370eb54dee8", 59183),
	DT_ENTRY1("xyz_underoos", "2.0", "877f6a899c08b312afbbe31158c0689a", 68526),

	// XYZZY Awards 1997 (TADS2)
	DT_ENTRY1("xyz_lovesparade", "2.3", "66f7f4cfe536e1a80c87c302b5fdc490", 270763),

	// XYZZY Awards 1998 (TADS2)
	DT_ENTRY1("xyz_badmachine", "1.2", "274c4c2bf0008134b924851d19511ab0", 331577),
	DT_ENTRY1("xyz_losingyourgrip", "5.0", "4c6373e6db29f6b1671cd61690722e53", 713150),
	DT_ENTRY1("xyz_onceandfuture", "2.1", "b2d13457625706ba4c4015d293f83a63", 918200),

	// XYZZY Awards 1999 (TADS2)
	DT_ENTRY1("xyz_commonground", "2.0", "1e074e89c7a5aeb917661d3fd21d276c", 262320),
	DT_ENTRY1("xyz_fren52", "1.2", "dfffed6aee17d45b79a6046376add419", 201302),
	DT_ENTRY1("xyz_worldsapart", "2.2", "52b46186628e9a86d860aba91539287c", 1182959),
	DT_ENTRY1("xyz_worldsapart", "3.0", "c48fccde16b882d2037d1ee592c99657", 1172497),

	// XYZZY Awards 2000 (TADS2)
	DT_ENTRY1("xyz_aboveandbeyond", "1.22", "d3d84a29c0a8c2b2ab1007a42f38f7e4", 548383),
	DT_ENTRY1("xyz_rematch", "1.0.4", "90d3b611eee24387da7c911637d05757", 244009),
	DT_ENTRY1("xyz_youarechef", "1.0", "784c8eec40cbcf10803b48de188177c3", 57758),

	// XYZZY Awards 2001 (TADS2)
	DT_ENTRY1("xyz_ftf", "3.0", "b80c3f3309d2c0ae2ef44d0838a9881b", 1220695),

	// XYZZY Awards 2002 (TADS2)
	DT_ENTRY1("xyz_1893", "2.0.5", "f4a361e9af20b3c8617188e5d9a42134", 70848900),
	DT_ENTRY1("xyz_fren53", "1.0", "6069167beb127d0a281932f9b047ddc9", 380739),

	// XYZZY Awards 2003 (TADS2)
	DT_ENTRY1("xyz_hamper", "1.0", "9159eb24bc9311f4ca51916ca8200e9d", 251739),

	// XYZZY Awards 2005 (TADS2)
	DT_ENTRY1("xyz_bbn", "1.0", "d59d036f1806b5110b8ce69dcb9a2867", 2156274),
	DT_ENTRY1("xyz_findingmartin", "1.12", "1c92590cd85cb707e4e2763a825515de", 1547636),

	// Russian games (TADS2)
	DT_ENTRYL1("ditchday", Common::RU_RUS, "1.0", "30743db95826c38a4e683f44dc5fe2d2", 206234),

	// ------------------------------------------------- TADS3 Section ------------------------------------------------
	// English games (TADS3)
	DT_ENTRY1("atopoesis", "1.0", "e43854fcaa551d587bac020ffd141aa1", 448325),
	DT_ENTRY1("blendings", "1.0", "0eed7f70dcd7e7e4e532ec82972f9a9a", 469831),
	DT_ENTRY1("catchthatcat", "1.0", "2fe0399552c16b0fb5c7912565bfdd7f", 528804),
	DT_ENTRY1("combatdemo", "1.0.6", "23a59262f9d8534562379b123e1272b7", 515392),
	DT_ENTRY1("combatdemo", "1.0.6/Bundled", "23a59262f9d8534562379b123e1272b7", 620151),
	DT_ENTRY1("comp02tr", "1.0", "23327e39fa44259fcabcb99917302c22", 960653),
	DT_ENTRY1("cookoff", "1.0", "581d1996db3a442064dae237e2e962f7", 1147531),
	DT_ENTRY1("darkangel", "2.0", "04f418d9e6ad8fb5a8d8291e11a788e6", 2873281),
	DT_ENTRY1("darkangel", "2.0/WebUI", "f7b2b242d4029623171f35214ef213c8", 2873281),
	DT_ENTRY1("dearbrian", "1.10", "8488c4f5c18a76ab65dd9e8eb72393ac", 999423),
	DT_ENTRY1("diabolical", "2.0", "161bc401e1d61ea7f3e14bddebd1f35a", 1614329),
	DT_ENTRY1("entrancingsin", "1.0", "df6ccd84bf305f35231b090217a6285e", 1173354),
	DT_ENTRY1("goldenbanana", "1.7", "c5df512d642503556cd9c0d487a3c95f", 726650),
	DT_ENTRY1("homeopen", "1.0", "ae46177333238267362f9465f65fac80", 1869401),
	DT_ENTRY1("homeopen", "1.0/WebUI", "7db13d7785329886ad5f24533115a749", 2303325),
	DT_ENTRY1("iamprey", "0.9.6", "06133295cd3c5fc339f2004b1114ae9d", 6583055),
	DT_ENTRY1("korenvliet", "1.0", "9c91716c269e268938c5051dbfc07f33", 1311919),
	DT_ENTRY1("korenvliet", "1.2", "4ab26eb23354a000f66b77bcbb820be4", 719957),
	DT_ENTRY1("liasad", "1.0", "af7bedb1529d5de959426eb1dc1a65d2", 606851),
	DT_ENTRY1("littlestgriffin", "1.0.0", "cd7cc456b29ba875c95b57127e4ef46f", 685469),
	DT_ENTRY1("littlestgriffin", "1.0.0/WebUI", "4491867db64e3bb6272d50efbae2c0f4", 1100275),
	DT_ENTRY1("littlestgriffin", "1.1.0", "2d1476e1f06a781877f2a6df4e1d9765", 690053),
	DT_ENTRY1("littlestgriffin", "1.1.1", "bdab0e0329c174745b202f1419384d83", 690195),
	DT_ENTRY1("littlestgriffin", "1.1.2", "9b52be11b67f639839aba77a80d8c653", 694837),
	DT_ENTRY1("littlestgriffin", "1.1.3", "b5202047de737befc0d04dabf0300fc5", 696812),
	DT_ENTRY1("littlestgriffin", "1.1.4", "a6b5b8e7146a70d648ab883fa6c86378", 699180),
	DT_ENTRY1("lowell", "1.0", "43cc0c4f12348cf5be2a5585e4ac2adb", 490426),
	DT_ENTRY1("lydiasheart", "2.0.6", "aa92fa47c4e04e90d358e9cfd50de671", 2620069),
	DT_ENTRY1("macrocosm", "1.0", "44492ef14565d51cbfe8ae6ddf07af1f", 30678531),
	DT_ENTRY1("marthasbigdate", "3.0", "8b65ae35ad2165d8bdc48f33e7b2bbb2", 646813),
	DT_ENTRY1("mazes", "2.00","c77ea32fc4867f8ff79626e6498cdcc6", 8475954),
	DT_ENTRY1("mazes", "2.01", "664f1f83281d4093f9a48c98248a9af5", 8481749),
	DT_ENTRY1("mazes", "2.02", "2fccc4148714aeca2ee640fcd3916cd6", 8481841),
	DT_ENTRY1("mylastrodeo", "1.0", "82f4a03bfdd5b8da41c6e3dbba0d9666", 1614970),
	DT_ENTRY1("mylastrodeo", "1.0/WebUI", "d7f480254686e114037de11f9c82782b", 2042237),
	DT_ENTRY1("natdewey", "1.0", "a768d1669eb7b3f80e52ba544d5ca2a7", 2145219),
	DT_ENTRY1("officeharassment", "2.0", "e3b365c7c6840b05c2b0ef8981603574", 1209725),
	DT_ENTRY1("pastpresent", "1.0", "c2a72f503aaaca57ad3dabe7d76be0db", 1988734),
	DT_ENTRY1("pekostory", "1.0", "0d703060d485aa4aeefe0d267a819ab3", 1100463),
	DT_ENTRY1("ratincontrol", "1.0", "607502409e7143cb20896d09201744ac", 401192),
	DT_ENTRY1("recluse", "1.0", "c41b5ab77e458f38c487dc1a1e0778f2", 1073030),
	DT_ENTRY1("revenge", "1.0", "74203a6ae5510ac19ae2d8798847a534", 641270),
	DT_ENTRY1("ribosoperation", "1.0", "19b7227f381f4e9491f3e2b7d9bccb59", 714946),
	DT_ENTRY1("ribosoperation", "1.1", "7ce5c5c74b5adecc8520a99e720a6a0d", 718057),
	DT_ENTRY1("saturnschild", "1.0", "2644265c684d153777dc8477e9e51ce9", 2321035),
	DT_ENTRY1("saturnschild", "1.0/WebUI", "67b0926a6b96a37a88e4564e3481f386", 2795440),
	DT_ENTRY1("sawesome", "1.0", "558a930adf7995b566d901719cb90041", 1227313),
	DT_ENTRY1("schattenweg", "4.0", "d409b10878c512cf4b6b2c6749945b3c", 1439354),
	DT_ENTRY1("snowswinter", "1.0", "fee15dca3dc311d80973c31a8e8114e8", 824711),
	DT_ENTRY1("sod", "1.0", "f97cff3a3db44cc2a7d7f06027076eac", 376613),
	DT_ENTRY1("spaceshooter", "1.0", "cb7275fb06421696153aef808bc28d53", 79989),
	DT_ENTRY1("stormshelter", "1.0", "72d6180f16066cdb20725176d03d7ded", 1702075),
	DT_ENTRY1("stormshelter", "1.0/WebUI", "db84c1f25835763494fc8124fcade37f", 2293053),
	DT_ENTRY1("stormshelter", "1.1", "c44fb06b549a3e7e3caf2778b37fdd78", 1708137),
	DT_ENTRY1("thereveal", "1.0", "ff5485af8cbc5c0b4211bf87db23ceeb", 1590214),
	DT_ENTRY1("thereveal", "1.0/WebUI", "d4f196d2e6bd3dd49f70e52bb504e793", 2018665),
	DT_ENTRY1("threedaysofnight", "1.0", "8271c716c6c11b4aec8b0b95aacb276d", 1711576),
	DT_ENTRY1("threedaysofnight", "1.0/WebUI", "1de414e58b667eab103361dbcd95353a", 2139562),
	DT_ENTRY1("trenchline", "0.9", "88358f4007fff93a6420b5f73a331766", 813623),
	DT_ENTRY1("undertaleepilogue", "1.0", "e76c15c3446865916bb1647ed4f83b3e", 1189729),
	DT_ENTRY1("venice", "1.1", "2db783be567e7761b4501ec12bc2eb35", 880111),
	DT_ENTRY1("venice", "1.1/WebUI", "e0e9efc4c6a680ac60ddc380c13cc099", 1306352),
	DT_ENTRY1("vividity", "0.1", "7da90c0a140c73249b83b5c3e459ca83", 692049),
	DT_ENTRY1("vividity", "0.1/WebUI", "f535eb35d8f4776b2262dd8aca3281b3", 1106715),
	DT_ENTRY1("wardz", "1.0", "f17c720b4a80693534fea474a54f15b4", 1170346),
	DT_ENTRY1("wilderness", "1.0", "5ea2cc9ab1193d416c907b9c1f0934e3", 663353),
	DT_ENTRY1("wilderness", "1.0/WebUI", "b2a76f20abf3f255dfe23d53cbaedcf0", 1089562),
	DT_ENTRY1("wolfenschtein", "1.0/WebUI", "31ccf0d040271950ddf3ca91f8ff22e2", 1200645),
	DT_ENTRY1("writingonthewall", "1.0", "85555df67c45ed04cfd18effb7bbd670", 1138559),
	DT_ENTRY1("wutheringheights", "1.0", "0d2042689bdf63e719b77485d591f1fe", 1083215),
	DT_ENTRY1("yetanotherexp", "200327", "9978b9e1ed9e0aae9f33e815cf775734", 685671),
	DT_ENTRY1("yetanotherexp", "200329", "e6e6d00e6c99547929a311785f12a520", 685607),
	DT_ENTRY1("youmatched", "1.0", "63007bd45363f5ddbcc2e4fe4a1a52de", 1621574),
	DT_ENTRY1("youmatched", "1.0/WebUI", "055efcc37f945071ea2486a207703951", 2050047),

	// IFComp 2002 (TADS3)
	DT_ENTRY1("if02_ericgift", "1.0", "94b65257f63b3395eabceac6dcdc4484", 403829),

	// IFComp 2003 (TADS3)
	DT_ENTRY1("if03_shadowsmirror", "1.0", "8934fe2f3fdfbf20da8eb5a668f62d98", 665576),

	// IFComp 2004 (TADS3)
	DT_ENTRY1("if04_squarecircle", "1.0", "3813eb298213bdc00b7d02cd30f0e198", 991978),
	DT_ENTRY1("if04_squarecircle", "1.1", "935775a75e141df9a6f9d94b41ae71ee", 1024972),
	DT_ENTRY1("if04_squarecircle", "1.1/WebUI", "fb2eeb70d3b751bec1a0aa728e60f290", 1654749),
	DT_ENTRY1("if04_imustplay", "42.00.009", "070a5888a00ba13d433884964f6aedd5", 651023),

	// IFComp 2005 (TADS3)
	DT_ENTRY1("if05_mixtape", "1.0", "d684a75f38a1b42ee90d181444b8c8bc", 893688),

	// IFComp 2006 (TADS3)
	DT_ENTRY1("if06_elysiumenigma", "1.3", "cd0a03ce5c3efb3268bc5f0863c853f9", 1543240),
	DT_ENTRY1("if06_elysiumenigma", "2.3", "f98f0e4de8c7cf5d6f471b56c6a96d81", 2095654),
	DT_ENTRY1("if06_elysiumenigma", "2.3/WebUI", "7ca73a03b433b28a831808066a843d79", 2149533),
	DT_ENTRY1("if06_xthunt", "1.3", "b9bcae307b3586e60c18685084a003c9", 851365),
	DT_ENTRY1("if06_brokenman", "1.0", "4aab871cb1cdfd1ee3638c493a6ced98", 573441),

	// IFComp 2007 (TADS3)
	DT_ENTRY1("if07_reconcilmoth", "1.0", "5a37b230d073cb10fe79a87c42c72e67", 811696),

	// IFComp 2008 (TADS3)
	DT_ENTRY1("if08_aprilparis", "1.0", "003d517e0112ba1184d5f66e66552d33", 763952),
	DT_ENTRY1("if08_magic", "1.0.1", "2147b68f33c11b83ac7dee9fb6c5fa83", 661392),

	// IFComp 2010 (TADS3)
	DT_ENTRY1("if10_multiversrgue", "1.0", "d30dfee5c7a2f9c4b444e1810b01c128", 809848),
	DT_ENTRY1("if10_multiversrgue", "2.0", "551b85aaba2d958f2bfd1080c7787844", 811469),

	// IFComp 2011 (TADS3)
	DT_ENTRY1("if11_it", "1.0", "aae7a1a7eb812ef216406875df1035bc", 1161138),
	DT_ENTRY1("if11_it", "2.2", "bd6e1a94089538b9b1aa58616cfbba45", 2439207),
	DT_ENTRY1("if11_it", "2.2/WebUI", "e69e00d227df2ea9179bb3bc20d779c9", 1589779),

	// IFComp 2012 (TADS3)
	DT_ENTRY1("if12_theisland", "1.0", "db1223c6d7ca43dcb6e0cc7f77f8f72b", 713956),

	// IFComp 2015 (TADS3)
	DT_ENTRY1("if15_contentment", "1.03", "02a2f64a45501e5419ced0d733fb6397", 997294),
	DT_ENTRY1("if15_contentment", "1.05", "3ffc224bf30d8cd0f9c876ca83b703fe", 1000411),

	// IFComp 2016 (TADS3)
	DT_ENTRY1("if16_hillridge", "1.09", "29f0ab8b35c149ee7c011f956b427630", 855220),

	// IFComp 2017 (TADS3)
	DT_ENTRY1("if17_fakenews", "1.0", "df5c476ee2c06ab586b691f5d91761bd", 1891393),
	DT_ENTRY1("if17_fakenews", "1.0/WebUI", "436a715d168d84e2bb0a911e10a7124a", 2389400),
	DT_ENTRY1("if17_fakenews", "1.1", "1bed7ef5033b4a6168cac39ee4f3733e", 1892928),

	// IFComp 2020 (TADS3)
	DT_ENTRY1("if20_captivity", "1.0", "aeeb0bcd430448867d45a4e6bea96c23", 1908803),
	DT_ENTRY1("if20_edgeofchaos", "1.0", "1e0b04d1cf78192aa173d7647fa73d16", 7979255),
	DT_ENTRY1("if20_edgeofchaos", "1.0/WebUI", "6e20b8013ff2e8cc1a9e1b0ce570adbb", 11780522),
	DT_ENTRY1("if20_deezlebub", "1.0", "57d532aa89fc21abaa52745689326a5f", 1142633),

	// IFComp 2021 (TADS3)
	DT_ENTRY1("if21_ghostwithin", "1.0", "25faca7427be437e0d00b7f1656ff30e", 2001751),
	DT_ENTRY1("if21_ghostwithin", "1.2", "e589ef28d2276d82339fef3bff2945e0", 1745032),

	// IFComp 2022 (TADS3)
	DT_ENTRY1("if22_cain", "1.0", "794e2840955272c4c6847fa1d3869efb", 16243566),
	DT_ENTRY1("if22_promdress", "1.0", "b345c70532a3f73dc4ef2abe82e82ca7", 3732288),

	// IF Art Show 2004 (TADS3)
	DT_ENTRY1("ifas_battle", "1.0", "e8d7dd64768efb9da7d9d365ca8d45e3", 943915),

	// Cover Stories Game-Jam (TADS3)
	DT_ENTRY1("cs_feathergrange", "3.0", "ce30f438545469a078cc28e4676079fc", 684191),

	// IFBeginnersComp 2008 (TADS3)
	DT_ENTRY1("ifb_germania", "1.9", "1ad65401989aaeaae1a8bfa5b340d0b8", 555992),
	DT_ENTRY1("ifb_mrspepper", "1.03", "d9b3c2f7b954fb066561febf195b27d4", 1118901),
	DT_ENTRY1("ifb_mrspepper", "1.02", "fd96b4428eb75f7feb7c0719ac318b0d", 1118827),
	DT_ENTRY1("ifb_mrspepper", "1.02/WebUI", "9bcfcdc5a37e9c5beb9b961a98929b67", 1721042),
	DT_ENTRY1("ifb_sleepprincess", "1.1", "8dfc3e925caa99891cb1fef4b1157baa", 709058),
	DT_ENTRY1("ifb_sleepprincess", "1.1/WebUI", "711cf6098b0670821af907dc94793e90", 1337974),
	DT_ENTRY1("ifb_sleepprincess", "2.0","63255d1bde649f907725a6c328047809", 736075),

	// IntroComp 2005 (TADS3)
	DT_ENTRY1("ic05_negotis", "1.0", "9598735eaaa54fceeef68c9f3a5d8750", 723973),

	// IntroComp 2007 (TADS3)
	DT_ENTRY1("ic07_folkarstation", "1.0", "07abdfc44d4e0e60c4b986957c028ce3", 505463),
	DT_ENTRY1("ic07_tin", "0.5", "0756b7837f706a29cc24871ebbe72d55", 547524),

	// IntroComp 2010 (TADS3)
	DT_ENTRY1("ic10_closedcircles", "0.5", "3f5d083a5407c7e4f105856ea799ce9d", 612103),

	// IntroComp 2014 (TADS3)
	DT_ENTRY1("ic14_devildetails", "1.0/Alpha", "9cd086d4a3e284e368b9aa6228086bb7", 2203123),
	DT_ENTRY1("ic14_devildetails", "1.0", "0cca588557a79231daa78ef948f62397", 2892947),

	// IntroComp 2015 (TADS3)
	DT_ENTRY1("ic15_deprivation", "0.2", "4daf3729590d29dd918dae0a4ec5b107", 1640675),

	// Shufflecomp Competition 2014 (TADS3)
	DT_ENTRY1("sh14_deadmansparty", "1.0/v1", "bc78f6e390df143a13aeabcdb9de1fed", 1638558),
	DT_ENTRY1("sh14_deadmansparty", "1.0/v2", "eb52018f79eba192b7448c16d45f7226", 1630209),
	DT_ENTRY1("sh14_deadmansparty", "1.0/WebUI", "831ba48e61a40e442a7459aa798da3ea", 2056080),
	DT_ENTRY1("sh14_lookacorner", "3.0", "7827b2f97dc429aebb570ce1b3d8eecd", 663334),
	DT_ENTRY1("sh14_lookacorner", "3.0/WebUI", "5e7d1e0c7f93662fb455972797795f15", 1079205),
	DT_ENTRY1("sh14_lookacorner", "5.0", "c9ac24c941c39ef57b23a55ed36f844d", 664271),

	// SpeedIF 20 Competition (TADS3)
	DT_ENTRY1("sif_greatzimbawe", "1.1", "bbdcf7afdeb77cd14a72eba4c1615f75", 570209),
	DT_ENTRY1("sif_miranda", "1.0", "6aee961565c541725c536319a7a4da4a", 378691),

	// SpeedIF 5th Anniversary Competition (TADS3)
	DT_ENTRY1("sif_evilbrain5", "1.0", "c40acfa1f3b79988957c7908459fbf1b", 398901),

	// SpeedIF Crinkle Cut Competition (TADS3)
	DT_ENTRY1("sif_dining", "2.0", "b34d05d2243399343ce5366317627ff8", 421666),

	// SpeedIF of Destiny Competition (TADS3)
	DT_ENTRY1("sif_badlandmachine", "1.0", "acac58abdd63fba9dcdef8f12769b144", 132786),
	DT_ENTRY1("sif_rainbow", "1.0", "7e1316a28660cb9853322d2e18267afb", 514600),

	// SpeedIF Jacket Competition (TADS3)
	DT_ENTRY1("sif_midsummer", "1.0", "954f2608b04e5148df55f2f11b1af261", 290120),

	// SpeedIF Jacket 4 Competition (TADS3)
	DT_ENTRY1("sif_stomach", "1.0", "9c07b078efbe21612fade9b851c3838e", 523172),

	// SpeedIF Late! Competition (TADS3)
	DT_ENTRY1("sif_fishbot", "1.0", "e868effa94933f71b50b9e018d950940", 363180),

	// SpeedIF Let's Make a Nightmare Competition (TADS3)
	DT_ENTRY1("sif_threedoors_pb", "1.0", "24e02d46cd9e647593a2fed4802c5ea3", 66946),
	DT_ENTRY1("sif_threedoors_sl", "1.0", "54455dd8df6ca20a9c85452d734ad044", 399804),

	// SpeedIF Thanksgiving Competition (TADS3)
	DT_ENTRY1("sif_catchfragonfly", "1.0", "c15712045323480c273b13e313c58ce8", 540029),

	// SpeedIF U Competition (TADS3)
	DT_ENTRY1("sif_deathscavenger", "1.0", "6dd1c61b75a016085b7f2b985d37eec3", 362826),

	// SpeedIF Ultimate not Numbered New Year's Competition  (TADS3)
	DT_ENTRY1("sif_emma2", "1.0", "e3cb9450cbd09487447f9e1b4feff9dd", 699363),

	// Spring Thing 2003 (TADS3)
	DT_ENTRY1("stc03_parrots", "1.0", "b68b8dbc1ebcbb49322f8ea792182324", 1532127),
	DT_ENTRY1("stc03_parrots", "1.0/Bundled", "b68b8dbc1ebcbb49322f8ea792182324", 2058062),

	// Spring Thing 2007 (TADS3)
	DT_ENTRY1("stc07_camelgirl", "1.0", "24908c4fb1cc2faee2042348d6a0afc5", 1018598),

	// Spring Thing 2012 (TADS3)
	DT_ENTRY1("stc12_whitebull", "1.0", "dc01235c8fa2ca07920171e124447967", 14511731),

	// Spring Thing 2018 (TADS3)
	DT_ENTRY1("stc18_thehouset3", "1.0", "8b92f7f00ef0ca5713f7ffffa13b8a01", 1688640),

	// XYZZY Awards 2002 (TADS3)
	DT_ENTRY1("xyz_foreveralways", "1.0", "c4c7de117843a9756ad06e9e9aa23ff1", 347383),
	DT_ENTRY1("xyz_foreveralways", "3.0", "1e901bfebf416a910ba55e301101034c", 656194),

	// XYZZY Awards 2004 (TADS3)
	DT_ENTRY1("xyz_returnditchday", "2.0", "47577d2001035ac30332bdb5ad9bcf3c", 2141661),
	DT_ENTRY1("xyz_returnditchday", "2.0/WebUI", "7de1082c5c7e37435880576c1cc26d96", 2565423),

	// XYZZY Awards 2005 (TADS3)
	DT_ENTRY1("xyz_allhope", "2.0", "1e842218ad6eeacb2d284ad34adbeaaa", 1816249),
	DT_ENTRY1("xyz_allhope", "2.0/WebUI", "c65cff5c63cfecad3d8c922f0086816f", 2434146),

	// XYZZY Awards 2007 (TADS3)
	DT_ENTRY1("xyz_blightedisle", "2.0", "a400f71d73c0efdf301635e73ff00c79", 3326591),
	DT_ENTRY1("xyz_blightedisle", "2.0/WebUI", "4ee80cdf2e39f41e721440db7ddbc05e", 3999970),
	DT_ENTRY1("xyz_blightedisle", "2.2", "8e2ef477586d441e3a356da8dad594a3", 3331088),

	// XYZZY Awards 2008 (TADS3)
	DT_ENTRY1("xyz_gunmute", "6.0", "bf196b6b53fca37c8fec25d27bdb4f23", 713636),

	// XYZZY Awards 2009 (TADS3)
	DT_ENTRY1("xyz_deadlikeants", "6.0", "58dd749b5416d1341fa007a527be7ce5", 682402),
	DT_ENTRY1("xyz_walkersilho", "9.0", "05bd96f0782fefa3d09d0adad67249c9", 687377),

	// XYZZY Awards 2011 (TADS3)
	DT_ENTRY1("xyz_indigo", "1.0", "ca322b887ba56ef503b96ee462a2df30", 535570),

	// Czech games (TADS3)
	DT_ENTRYL1("exoter", Common::CS_CZE, "1.0", "02b93382a19cd69f3cb67a12073f4795", 1487144),
	DT_ENTRYL1("exoter", Common::CS_CZE, "1.0/WebUI", "865db2452b38f96035841f04e314c2c6", 1489600),
	DT_ENTRYL1("exoter", Common::CS_CZE, "2.0", "956f93c8c8b5270d75501039ef825429", 1074752),

	// German games (TADS3)
	DT_ENTRYL1("pionierin", Common::DE_DEU, "3.0", "11b10b38003dda7e7c0c43bac3ce67d0", 928810),
	DT_ENTRYL1("pionierin", Common::DE_DEU, "3.0/WebUI", "9f899c9826204184c09f7088acfa8cce", 1293016),
	DT_ENTRYL1("spiegel", Common::DE_DEU, "1.4", "75c48fd809da11db3a4b2d250c8b01e1", 816048),

	// Italian games (TADS3)
	DT_ENTRYL1("remerook", Common::IT_ITA, "1.0", "218f1ad01f58f93a610ba91f626b5fb5", 1166951),

	// Spanish games (TADS3)
	DT_ENTRYL1("memorias_tads", Common::ES_ESP, "1.0", "8b62037af64b77d08caf43794a0ccdd6", 616193),
	DT_ENTRYL1("verla", Common::ES_ESP, "0.2", "751eaee6249e4acec9b7ace03bb80fb4", 1118875),
	DT_ENTRYL1("verla", Common::ES_ESP, "1.1", "f901ee66728d813c76ce5ad95aa7199d", 8411036),
	DT_ENTRYL1("verla", Common::ES_ESP, "1.1/WebUI/v1", "a5fdc0388600bcfa2b424a0edd62844e", 1545007),
	DT_ENTRYL1("verla", Common::ES_ESP, "1.1/WebUI/v2", "beb148c15ff993f4714d8e908efc3ea1", 1695763),
	DT_ENTRYL1("verla", Common::ES_ESP, "1.1/WebUI/v3", "1ed6250a40e8e16d6e14976fb74bb5bd", 2616415),

	DT_END_MARKER
};

} // End of namespace TADS
} // End of namespace Glk
