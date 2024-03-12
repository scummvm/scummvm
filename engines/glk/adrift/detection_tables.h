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
namespace Adrift {

const PlainGameDescriptor ADRIFT_GAME_LIST[] = {
	{ "Adrift",             "Adrift IF Game" },

	// English Games
	{ "1sttime",            "1st Time" },
	{ "2playerex",          "2 Player Extreme" },
	{ "30seconds",          "30 Second" },
	{ "3monkeys",           "Three Monkeys, One Cage" },
	{ "abduction",          "Of Masters and Mistresses - Ep. 1: Abduction" },
	{ "adriftproject",      "The ADRIFT Project: Classified" },
	{ "adv350_adrift",      "Adventure - 350 Point Colossal Cave" },
	{ "adventurepostman",   "The Adventures of Postman Matt" },
	{ "adventurestrikes",   "Adventure Strikes When You Least Expect It" },
	{ "adventurethumper",   "Adventures of Thumper: Wonder Wombat" },
	{ "afp",                "Another Friday Party" },
	{ "akari",              "A Day in the Life 3: Akari's Story" },
	{ "akron",              "Akron" },
	{ "albertlost",         "Albert Is Lost!: An Adventure in Real Life" },
	{ "albridgemanor",      "Albridge Manor" },
	{ "alchemist",          "The Alchemist" },
	{ "alex",               "The Corruption of Alex" },
	{ "alexis",             "Alexis: Dalskee" },
	{ "alias",              "Alias: Undercover Agent" },
	{ "allroadsmars",       "All Roads Lead to Mars" },
	{ "alrhinitsys",        "ALR Hints System" },
	{ "amazgriswold",       "The Amazing Uncle Griswold" },
	{ "amaztvcaper",        "The Amazing TV Caper" },
	{ "americanmoney",      "USA Money Template" },
	{ "amnesiakid",         "Amnesia Kid" },
	{ "amy",                "Amy and the Raging Hormones" },
	{ "angel",              "The Angel, the Devil and the Human" },
	{ "aquarius1",          "The Doomed World of Aquarius - Part I" },
	{ "aquarius2",          "The Doomed World of Aquarius - Part II" },
	{ "aram",               "Alice's Restaurant Anti-Massacree Adventure" },
	{ "archiebdaych1",      "Archie's Birthday - Chapter 1: Reggie's Gift" },
	{ "armour",             "Armour" },
	{ "asylum_ms",          "Asylum (by Mel S)" },
	{ "athylon",            "The Tunnels of Athylon" },
	{ "attack",             "Attack" },
	{ "average",            "The Average Life" },
	{ "awalkatdusk",        "A Walk at Dusk" },
	{ "azra",               "The Town of Azra" },
	{ "backhome",           "Back Home" },
	{ "backtolife",         "Back to Life... Unfortunately" },
	{ "bananadvent",        "The Foggy Banana Adventure" },
	{ "barneysproblem",     "Barney's Problem" },
	{ "basement",           "Escape the Basement From Hell" },
	{ "bcl",                "Big City Laundry" },
	{ "bdw",                "Brain Dead Weekend" },
	{ "bedlam_mw",          "Bedlam (by Mark Whitmore)" },
	{ "beerisntenough",     "When Beer Isn't Enough" },
	{ "beethro",            "Beethro’s Text Adventure" },
	{ "bfhouse",            "Burtainian Fly's House" },
	{ "bigdayout",          "Big Day Out" },
	{ "bigspyfictionch1",   "The Big Spy Fiction: Comedy Mashup Game - Part I" },
	{ "bigspyfictionch2",   "The Big Spy Fiction: Comedy Mashup Game - Part II" },
	{ "bigspyfictionch3",   "The Big Spy Fiction: Comedy Mashup Game - Part III" },
	{ "bigstuff",           "Big Stuff" },
	{ "birthday",           "The Birthday" },
	{ "blacksheep",         "Black Sheep's Gold" },
	{ "blob",               "Overriding Get" },
	{ "bloodrelatives",     "Blood Relatives" },
	{ "bluesky_un",         "Blue Sky (by The Unknown Norwegian)" },
	{ "bobbobsly",          "The Adventures of Bob Bobsly" },
	{ "bombthreat",         "Bomb Threat" },
	{ "bountyhunter",       "Bounty Hunter" },
	{ "breakable",          "Colored Numbers" },
	{ "breakout",           "Breakout" },
	{ "britishfox",         "British Fox and the Celebrity Abduction" },
	{ "bsg22",              "BSG: Twenty Two" },
	{ "buffy",              "Buffy: Before the Date" },
	{ "bulkitem",           "Bulk Item Test" },
	{ "burbs",              "The Burbs" },
	{ "bz3americans",       "Battle Zone III: Americans" },
	{ "bz3soviets",         "Battle Zone III: Soviets" },
	{ "cabin",              "The Cabin" },
	{ "campwindylake",      "Camp Windy Lake" },
	{ "campwindylake2",     "Camp Windy Lake - Part 2" },
	{ "canidoit",           "Can I Do It?" },
	{ "captive",            "Captive Universe" },
	{ "car_an",             "Car (by Anonymous)" },
	{ "car_sm",             "Car (by Stewart McAbney)" },
	{ "cards",              "Playing Cards" },
	{ "casino",             "Casino" },
	{ "castlequest",        "Castle Quest" },
	{ "cat",                "The Cat" },
	{ "catburglar",         "Cat Burglar" },
	{ "caveofwonders",      "Cave of Wonders" },
	{ "cd",                 "Crimson Detritus" },
	{ "cellar",             "The Cellar" },
	{ "cellpart1",          "Cell: Part I" },
	{ "cellphone",          "Cell Phone" },
	{ "cell",               "The Cell" },
	{ "chairtest",          "Chair Test" },
	{ "changepicture",      "Locations Change Picture" },
	{ "charactercreator",   "Character Creator!" },
	{ "characterturn",      "Character Turns Back" },
	{ "chargenerator",      "Character Generator" },
	{ "chasingrussian",     "Chasing the Russian: Chapter 1" },
	{ "chicago",            "Chicago" },
	{ "choices",            "Choices" },
	{ "chooseyourown",      "Choose Your Own..." },
	{ "christpresent",      "Christmas Present" },
	{ "church",             "The Old Church" },
	{ "cif",                "Scene of the Crime 2: City in Fear" },
	{ "cldone",             "The Clairvoyant" },
	{ "clear",              "Description Display After" },
	{ "cleft",              "The Cleft in the Rock" },
	{ "cloak",              "Cloak of Darkness" },
	{ "clock",              "Clock" },
	{ "clodquest",          "Clod's Quest: The Dungeons of Zivulda" },
	{ "cluelessbob",        "In the Claws of Clueless Bob" },
	{ "cobl",               "Crazy Old Bag Lady" },
	{ "colony",             "Colony" },
	{ "combat",             "Combat" },
	{ "community",          "Community Policing" },
	{ "commuting",          "Commuting" },
	{ "compendiumendgame",  "The Woodfish Compendium: The Game to End All Games" },
	{ "compendiumforum1",   "The Woodfish Compendium: Forum" },
	{ "compendiumforum2",   "The Woodfish Compendium: Forum2" },
	{ "compendiumimagi",    "The Woodfish Compendium: ImagiDroids" },
	{ "compendiumsaffire",  "The Woodfish Compendium: Saffire" },
	{ "compendiumtopaz",    "The Woodfish Compendium: Topaz" },
	{ "com",                "Starship Cliche: A Communications" },
	{ "consequences",       "Consequences" },
	{ "cowboyblues",        "Cowboy Blues" },
	{ "crawlersdelight",    "Crawler's Delight" },
	{ "crimeadventure",     "Crime Adventure" },
	{ "crimelife",          "Crime Life" },
	{ "croftnon",           "Lara Croft: The Sun Obelisk" },
	{ "crookedest",         "The Crooked Estate" },
	{ "crossworldsp0",      "Crossworlds Part 0: The Girl Next Door" },
	{ "crossworldsp1",      "Crossworlds Part 1: Normville" },
	{ "crossworldsp2",      "Crossworlds Part 2: The Flower Opens" },
	{ "crossworldsp3",      "Crossworlds Part 3: The Finale Far Far Away Frontier" },
	{ "crossworldsp4",      "Crossworlds Part 4: Scream For Me" },
	{ "cumberbund",         "Cumberbund" },
	{ "cyber1",             "Cyber Warp 1" },
	{ "cyber2",             "Cyber Warp 2" },
	{ "cyo3",               "Choose Your Own..." },
	{ "darkfantasy",        "Dark Fantasy" },
	{ "darkness",           "Darkness" },
	{ "darktower",          "The Dark Tower" },
	{ "daveshouse",         "Melbourne Beach" },
	{ "daynightcycle",      "Day-Night Cycle" },
	{ "dayschool",          "A Day at School" },
	{ "daystemplate",       "Day Template" },
	{ "dead",               "A Dead Room" },
	{ "deadoralive",        "Dead or Alice Xtreme Beach" },
	{ "deadreckon_adrift",  "Dead Reckoning" },
	{ "deardiary",          "Dead Reckoning" },
	{ "deardiary2",         "Dear Diary 2: Prom Night" },
	{ "deathbedcash",       "Moving Items After an NPC Die" },
	{ "deathdoor",          "Death's Door" },
	{ "decisions",          "Decisions" },
	{ "delight",            "Crawler's Delight" },
	{ "delivery",           "The Delivery" },
	{ "demonhunter2",       "The Demon Hunter" },
	{ "demonhunter",        "Apprentice of the Demonhunter" },
	{ "devilchild",         "Devil Child" },
	{ "dickynoodle",        "Dicky Noodle 2002" },
	{ "digby",              "For Love of Digby" },
	{ "dinnerparty",        "The Dinner Party" },
	{ "disambtest",         "Disambiguation Test" },
	{ "display",            "LED Display" },
	{ "door2",              "Door and NPC" },
	{ "doortest",           "Wedge-A-Door" },
	{ "doortoutopia",       "The Door to Utopia" },
	{ "doorwithsensor",     "Door With Sensor" },
	{ "dragonsheart",       "Dragon's Heart" },
	{ "dragonsphere",       "Dragonsphere" },
	{ "dreamcometrue",      "A Dream Come True" },
	{ "dreamland",          "Dreamland" },
	{ "dreamquest",         "Dream Quest" },
	{ "dressbygender",      "Dress by Gender" },
	{ "driftingin",         "Adrift Drifting In" },
	{ "droneacademy",       "Drone Academy: Building Tomorrow's Soldier" },
	{ "druggylane",         "Druggy Lane" },
	{ "drwhovortexlust",    "Doctor Who and The Vortex of Lust" },
	{ "duchess",            "The Duchess of Desire" },
	{ "duck",               "Duck McCloud: The Fight Begins" },
	{ "easterpeeps",        "It's Easter, Peeps" },
	{ "eatfull",            "Task Counting" },
	{ "ebonysworld",        "Ebony's World" },
	{ "emotions",           "Basic Emotions Change" },
	{ "enc1",               "Encounter 1: Tim's Mom" },
	{ "enc2",               "Encounter 2: The Study Group" },
	{ "enigmacreature",     "Enigma Creature" },
	{ "escapecamelot",      "Escape From Camelot" },
	{ "escapefromsouth",    "Tom Ceader: Escape From the South" },
	{ "escapetofreedom",    "Escape to Freedom" },
	{ "eveningwithecod",    "An Evening With the Evil Chicken of Doom" },
	{ "event",              "Event" },
	{ "expsystem",          "Experience System" },
	{ "fairscarenightmare", "Fair Scare Nightmare" },
	{ "fairytales",         "Fairy Tales Remixed" },
	{ "fantasy",            "Fantasy" },
	{ "fantasyworld",       "The Quest (by Chlestron)" },
	{ "farfromhome",        "Far From Home" },
	{ "faucets",            "Running Water" },
	{ "filthybill",         "Filthy Bill Does Everyone but His Mother" },
	{ "findandy1",          "Find Andy: Part 1" },
	{ "firstarise",         "The First to Arise" },
	{ "firstday",           "First Day" },
	{ "first",              "The Book of Fistandantalus" },
	{ "foresthouse",        "Forest House" },
	{ "fox",                "Fox" },
	{ "freshman",           "Freshman Orientation" },
	{ "fugitive",           "The Fugitive" },
	{ "fullcircle",         "Full Circle" },
	{ "funhouse",           "FunHouse" },
	{ "funtown",            "Fun Town" },
	{ "g7056",              "G7056" },
	{ "gallows",            "Showtime at the Gallows" },
	{ "gamemaster",         "The Gamemaster: Resident Lust" },
	{ "gammagals",          "The Gamma Gals" },
	{ "gateway",            "Gateway: Guardian Child" },
	{ "gettingeven",        "Getting Even" },
	{ "ghoster",            "Ghoster" },
	{ "ghostjustice",       "Ghost Justice" },
	{ "ghoststory",         "Ghost Story" },
	{ "ghosttown_fi",       "Ghost Town (by Finn)" },
	{ "ginger",             "Ginger's Island" },
	{ "gmylm",              "Give Me Your Lunch Money" },
	{ "go",                 "Go!" },
	{ "goblin",             "A Goblin's Life" },
	{ "goldilocksbreakent", "Goldilocks: Breaking & Entering" },
	{ "golf"      ,         "Golf" },
	{ "golfball",           "Golfball" },
	{ "gosha",              "Gosha Adventure" },
	{ "gotcha",             "Gotcha" },
	{ "gps",                "GPS" },
	{ "graduation",         "Graduation Night at the Brothel" },
	{ "grandjourney",       "Grand Journey" },
	{ "grappling",          "Grappling Hook" },
	{ "great",              "The Great Escape" },
	{ "greek",              "Greek School Adventure" },
	{ "gross",              "Gross: The Game" },
	{ "guess3num",          "Guess 3 Numbers" },
	{ "gumball",            "Gumball Machine" },
	{ "guysguys",           "Guys, Guys" },
	{ "halloweenhijinks",   "Halloween Hijinks" },
	{ "hammurabi_rmo",      "Hammurabi (by Ron Moore)" },
	{ "handyman",           "Handyman" },
	{ "harem",              "The Tale of the Unlikely Harem Girl" },
	{ "hcw",                "How to Conquer the World" },
	{ "hellinahamper",      "To Hell in a Hamper" },
	{ "heretoday",          "Here Today, Tomorrow, Next Week!" },
	{ "heroes",             "Heroes Are Interesting Things to Be..." },
	{ "hhorror",            "House of Horror" },
	{ "hhouse",             "The Haunted House" },
	{ "hiddenassets",       "Hidden Assets" },
	{ "holdbreath",         "Hold Breath" },
	{ "home12",             "Home1.2" },
	{ "homealone",          "Home Alone" },
	{ "hornetsnest",        "The Star Trek Chain Reaction: The Hornets' Nest" },
	{ "hotelfuego",         "Hotel con Fuego" },
	{ "hotown",             "Hoedown in Ho-Town" },
	{ "house_jp",           "House (by Joe Powell)" },
	{ "house_mp",           "House (by Matt Wiltshire)" },
	{ "housedeath",         "House of Death" },
	{ "houseofthedamned",   "House of the Damned" },
	{ "howdidi",            "How Did I Get Into This?" },
	{ "howitstarted",       "How It All Started" },
	{ "humbug",             "Humbug" },
	{ "hungry",             "Hungry" },
	{ "huntung",            "Hunting Ground" },
	{ "hyperbs",            "HYPER Battle System" },
	{ "hypertag",           "Locked Door With Water Trap" },
	{ "iachini",            "A Day at the Iachini House" },
	{ "igor",               "Igor" },
	{ "igorone",            "One Room" },
	{ "illegal",            "Illegal Socks" },
	{ "imaginings",         "Imagings" },
	{ "infospace",          "Info Space" },
	{ "inmemory",           "In Memory" },
	{ "input_col",          "Question/Input: Colour" },
	{ "input_age",          "Question/Input: Age" },
	{ "insidejob",          "Inside Job" },
	{ "interrogate",        "Interrogate" },
	{ "invasionshirts",     "Invasion of the Second-Hand Shirts" },
	{ "inverness",          "Inverness Castle" },
	{ "invert",             "Invert's Road Trip" },
	{ "invisible",          "Invisible Visible" },
	{ "isle",               "The Isle" },
	{ "jacarandajim",       "Jacaranda Jim" },
	{ "janeysdiary",        "Janey's Diary - Gold Edition" },
	{ "jason2",             "Jason Evans 2: Misunderstood" },
	{ "jason3",             "Jason Evans 3: Jason's Return" },
	{ "jason4",             "Jason Evans 4" },
	{ "jasonevans",         "Jason Evans 1" },
	{ "jgrim",              "Jonathan Grimshaw: Space Tourist" },
	{ "jinx",               "JINXTRON" },
	{ "justanotherday_sr",  "Just Another Day (by Simply Ryan)" },
	{ "keys",               "Where Are My Keys?" },
	{ "kissing",            "Kissing Cousins" },
	{ "labrats",            "Laboratory R.A.T.S." },
	{ "lasthurrah",         "The Last Hurrah!" },
	{ "lastknight",         "Last Knight" },
	{ "lastweek",           "Last Week Before the Wedding" },
	{ "latework",           "Late Work" },
	{ "lauren",             "Lauren's Awakening" },
	{ "legacyofaprincess",  "Legacy of a Princess" },
	{ "legendakhbar",       "Legends of Ahkbar" },
	{ "legends",            "Legends RPG System" },
	{ "lessonsp1",          "The Extra Lessons - Episode 1" },
	{ "lessonsp2",          "The Extra Lessons - Episode 2" },
	{ "lessonsp3",          "The Extra Lessons - Episode 3" },
	{ "lessonsp4",          "The Extra Lessons - Episode 4" },
	{ "life",               "Life" },
	{ "lifesimulation",     "Life Simulation" },
	{ "lightsgoout",        "When the Lights Go Out" },
	{ "liqid",              "The Quest for More Hair" },
	{ "listenformonsters",  "Listen for Monsters" },
	{ "location",           "Location & Listen" },
	{ "lock2",              "Locked Box" },
	{ "lockedout",          "Locked Out" },
	{ "longjourneyhome",    "The Long Journey Home" },
	{ "lost_em",            "Lost (by Eric Mayer)" },
	{ "lostmines",          "The Lost Mines" },
	{ "losttomb",           "The Lost Tomb" },
	{ "loveforreal",        "Love for Real" },
	{ "lovingfamily",       "Loving Family" },
	{ "mages",              "The Mages of Costrador" },
	{ "magicshow",          "The Magic Show" },
	{ "makeshiftmagician",  "The Makeshift Magician" },
	{ "mammoth",            "Mammoth Vacuum Button of Death" },
	{ "marmalade",          "Marmalade Skies" },
	{ "mattshouse",         "Matt's House" },
	{ "mcl",                "Character Library #1" },
	{ "mhpquest_adrift",    "Quest for the Magic Healing Plant" },
	{ "microwaveman",       "Microwave Man!" },
	{ "midsomar",           "Midsomer Bottom Manor" },
	{ "mikes",              "The Life of Mike" },
	{ "missingpersona",     "Missing Person" },
	{ "mm2",                "Monster in the Mirror" },
	{ "mm3",                "Monster in the Mirror: Selma's Will" },
	{ "monsterisland",      "Monster Island" },
	{ "monstermirror",      "The Monster in the Mirror" },
	{ "monsters",           "Monsters" },
	{ "morning",            "A Morning With a Headache" },
	{ "morpheus",           "The Cave of Morpheus" },
	{ "mould",              "The Potter and the Mould" },
	{ "mount",              "Mount Voluptuous" },
	{ "movechars",          "Character Follows Character" },
	{ "movingobjects",      "Task Command Functions" },
	{ "mrsmith",            "The Search for Mr Smith" },
	{ "msl1",               "Mystery’s Static Library #1" },
	{ "mudergreatfalls",    "Murder in Great Falls" },
	{ "mudmonster",         "Mud Monster" },
	{ "murders",            "The Merry Murders" },
	{ "mws",                "The Magic Wishing Fountain" },
	{ "mynameis",           "Player Name as Input" },
	{ "mysteryhouse",       "Mystery House" },
	{ "nameme",             "Name Me" },
	{ "namiki",             "A Day in the Life 2: Namiki" },
	{ "nem",                "The Nem Rehsif" },
	{ "newbie",             "Newbie" },
	{ "newrooms",           "News Rooms" },
	{ "newton",             "Newton" },
	{ "nightmoon",          "The Night the Moon Shone Grey" },
	{ "nightthatdripped",   "The Night that Dripped Blood" },
	{ "nodrop",             "No Drop" },
	{ "nonsensemachine",    "The Nonsense Machine 6000" },
	{ "noordinarylove",     "No Ordinary Love" },
	{ "normville",          "Normville High" },
	{ "notebook",           "Notebook" },
	{ "noticeme",           "Notice Me" },
	{ "noximion",           "Noximion" },
	{ "npcgroupwalk",       "Room Group NPC Walk" },
	{ "oakwood",            "Oakwood School" },
	{ "obsession",          "Sad Obsession" },
	{ "officebreak",        "Office Breakout" },
	{ "oldmoney",           "Ye Olde Money" },
	{ "onnafa",             "Oh No, Not Another Fantasy Adventure" },
	{ "options",            "Options" },
	{ "orcho",              "The Search for Orcho's Pot of Gold" },
	{ "orient",             "Orient Express" },
	{ "outside",            "Outside" },
	{ "ovaloffice",         "The Oval Office" },
	{ "overtheedge",        "Over the Edge" },
	{ "panic",              "Panic" },
	{ "paperrack",          "Newspaper Rack" },
	{ "paradisehotel",      "Paradise Hotel" },
	{ "passages",           "The Passages" },
	{ "pathway_adrift",     "Pathway to Destruction" },
	{ "payback",            "Pay Back" },
	{ "phoenixdestiny",     "Phoenix Destiny" },
	{ "phonebooth",         "Pick Up the Phone Booth and Cry" },
	{ "picture",            "Conversation With a Picture" },
	{ "pizzaboy",           "The Pizza Boy" },
	{ "plainsfantasy",      "Plains of Fantasy" },
	{ "plan69",             "Plan 69" },
	{ "planescape",         "Plan 69" },
	{ "platinum",           "Generator Platinum" },
	{ "poker",              "Poker Game" },
	{ "powerplay",          "Power Play" },
	{ "practicepoker",      "Practice Poker" },
	{ "practiceprocedures", "Practice Procedures" },
	{ "praxis",             "from the Files of Sigmund Sigmund Praxis, Guerrilla Therapist" },
	{ "privateteacher",     "Private Teacher" },
	{ "professional",       "Professional" },
	{ "professorvonwitt",   "Professor von Witt's Fabulous Flying Machine" },
	{ "prostitute",         "The Prostitute" },
	{ "provenance",         "Provenance" },
	{ "ptadance",           "PTA Dance" },
	{ "questforfood",       "Quest for Food" },
	{ "rachelbadday",       "Rachel Has a Bad Day" },
	{ "raiders",            "Raiders of Sheppey" },
	{ "rainm1",             "Rain Sample #1" },
	{ "rainm2",             "Rain Sample #2" },
	{ "rainm3",             "Rain Sample #3" },
	{ "rainm4",             "Rain Sample #4" },
	{ "randomnumber",       "Random Number Generator" },
	{ "random",             "Random" },
	{ "randomscattering",   "Random Scattering" },
	{ "randpuzzle",         "Random Puzzle" },
	{ "razorsedge",         "Razor's Edge" },
	{ "realtime",           "Real Time!" },
	{ "rechargablelantern", "Rechargable Lantern w/ Turn Counter" },
	{ "relatives",          "Relatives" },
	{ "report",             "Report Espionage" },
	{ "respawningmonsters", "Respawning Monsters" },
	{ "restart",            "Restart" },
	{ "richard",            "Where is Richard?" },
	{ "ridinghome",         "Riding Home" },
	{ "rndperson",          "Random Person in Line" },
	{ "rockband",           "Rock Band: Revenge of Gigantor!" },
	{ "rockyraccoon",       "Rocky Raccoon" },
	{ "ronforest",          "Forest on the Norm" },
	{ "rontoxic",           "Toxically Earth" },
	{ "ronweasley",         "Ron Weasley and the Quest for Hermione" },
	{ "roomdesc",           "Changing Room Description" },
	{ "roomentrytask",      "Run Task on Room Entry" },
	{ "rtcountdown",        "Realtime Countdown" },
	{ "salvation",          "Salvation" },
	{ "sand",               "Sand" },
	{ "santababy",          "Santa Baby" },
	{ "sbft",               "Stupid Bloody Fairytale" },
	{ "scandal",            "Scandal on the Seven Seas" },
	{ "sceneofthecrime",    "Scene of the Crime" },
	{ "schoolplan",         "Making the Grade" },
	{ "score",              "To Score or Not to Score" },
	{ "screensavers",       "The Screen Savers on Planet X" },
	{ "seat",               "Seated Character" },
	{ "secondchance",       "Second Chance" },
	{ "secretoflostworld",  "The Secret of the Lost World" },
	{ "seekandenjoy",       "Seek and Enjoy" },
	{ "selmaswill",         "Selma's Will" },
	{ "sentor",             "Sentor" },
	{ "sere",               "SERE: Survive, Evade, Resist, Escape" },
	{ "seriespasswordsg1",  "Series Passwords: Game #1" },
	{ "seriespasswordsg2",  "Series Passwords: Game #2" },
	{ "severance",          "Severance Battle System" },
	{ "sexismental",        "Sex is Mental" },
	{ "sexawakening",       "Sexual Awakening" },
	{ "shadowpeak",         "Shadowpeak" },
	{ "shanilor",           "Shanilor's Tower" },
	{ "shetland",           "The Shetland Enigma" },
	{ "shuffling",          "The Shuffling Room" },
	{ "siblingseduction",   "Sibling Seduction" },
	{ "sierramadre",        "Terror of the Sierra Madre" },
	{ "sigurd",             "Sigurd Fafnesbane" },
	{ "silknoil",           "Silk Noil" },
	{ "silvermaiden",       "The Silver Maiden" },
	{ "silverwolf",         "SilverWolf" },
	{ "sk8sponsorz",        "SK8 Sponsorz" },
	{ "sleaze",             "Sleaze City" },
	{ "sleepover",          "Sleep Over" },
	{ "smercenary",         "Space Mercenary" },
	{ "smoke2",             "Smoke 2: The Time of Your Like" },
	{ "sniperscope",        "Sniper Scope" },
	{ "sommeril",           "Sommeril" },
	{ "sommerilse",         "Sommeril SE" },
	{ "sororityhouse",      "Sorority House" },
	{ "spaceboy",           "The Adventure of Space Boy!" },
	{ "spaceboy2",          "The Adventure of Space Boy! Volume II" },
	{ "spacerun",           "Space Run" },
	{ "spawnmonstershop",   "Respawning Monster and Shop" },
	{ "speaking",           "Speaking NPC" },
	{ "spirit",             "The Spirit's Flight" },
	{ "spooked2",           "Spooked!: The Haunted Horror House" },
	{ "spotbother",         "A Spot to Bother" },
	{ "ssteacher",          "Shamelessy Slutty: Teacher" },
	{ "sswhore",            "SS Whore" },
	{ "stalker",            "Stalker" },
	{ "standing",           "Stand on Stool to Examine" },
	{ "stardust",           "S Tar Dus T" },
	{ "station",            "Station XIII" },
	{ "stodw4",             "The Strange Tale of Dr Wilkins" },
	{ "stopfollowing",      "Stop Following" },
	{ "storetest",          "Simple Store" },
	{ "stowaway",           "Stowaway" },
	{ "strange",            "The Strange Adventure" },
	{ "studio",             "The Studio" },
	{ "superliam",          "Super Liam 1: A Hero Is Born" },
	{ "superstud",          "The New Superstud" },
	{ "swordincane",        "Sword Concealed in Cane" },
	{ "sword",              "Sword" },
	{ "talking",            "Conversation Tree Sample" },
	{ "target",             "Target" },
	{ "teleport",           "Teleport" },
	{ "tempest7",           "Storm Tossed" },
	{ "templeofthesun",     "Temple of the Sun" },
	{ "temporfell",         "Temporfell" },
	{ "testingstatus",      "Testing Status" },
	{ "texevil",            "Textident Evil" },
	{ "annihilofthink3",    "The Annihilation of Think.com 3" },
	{ "theclock_sm",        "The Clock (by Stewart J. McAbney)" },
	{ "thecrash",           "The Crash" },
	{ "thecrimescene",      "The Crime Scene" },
	{ "thedarkriver",       "The Dark River" },
	{ "thehole",            "The Hole" },
	{ "thelabexperiment",   "The Lab Experiment" },
	{ "thelabyrinth",       "The Labyrinth" },
	{ "thelabyrinthsystem", "The Labyrinth System" },
	{ "thelasthour",        "The Last Hour" },
	{ "therapy",            "Therapy" },
	{ "thesearch",          "The Search" },
	{ "thesorcerer",        "The Sorcerer" },
	{ "thesuccessor",       "The Successor" },
	{ "thetimemachine",     "The Time Machine (by Richard Anderson)" },
	{ "thevirtualhuman",    "The Virtual Human" },
	{ "thewill",            "The Will" },
	{ "tictactoe",          "Tic-Tac-Toe" },
	{ "time",               "Time" },
	{ "timeadventure",      "Time Adventure" },
	{ "timesystem",         "Time System" },
	{ "timething",          "Timething" },
	{ "tmm",                "The First Chapter" },
	{ "tmn",                "The Magician’s Niece" },
	{ "tobeking",           "To Be King" },
	{ "togetyou",           "We Are Coming to Get You!" },
	{ "tohell",             "To Hell and Beyond" },
	{ "tomdickharry",       "Tom Dick and Harry" },
	{ "toolkitdice",        "Toolkit - Dice Module" },
	{ "toolkitcalc",        "Toolkit - Calculator Module" },
	{ "toolkitgame",        "Toolkit - Game Module" },
	{ "tophat",             "Top Hat" },
	{ "toronto",            "A Day in Toronto" },
	{ "tq3",                "The Quest (by Chris Moody)" },
	{ "trackingdevice2",    "Tracking Device 2" },
	{ "tra",                "The Timmy Reid Adventure" },
	{ "tree",               "Conversation Tree" },
	{ "troll",              "Troll!" },
	{ "trouserpress",       "Trouserpress" },
	{ "truck",              "Truck" },
	{ "turnberry",          "Turnberry Manor" },
	{ "twentyone",          "Twenty-One!" },
	{ "twilight",           "The Twilight" },
	{ "txfbeg",             "The X-Files: A New Beginning" },
	{ "unexpected",         "Unexpected Proposal" },
	{ "untitled1",          "Untitled Title #1" },
	{ "untitled2",          "Untitled Title #2" },
	{ "upgirls",            "Uptown Girls" },
	{ "urbandragon",        "Urban Dragon" },
	{ "vague",              "Vague" },
	{ "vampireconscience",  "The Vampire With a Conscience" },
	{ "vandk",              "Villains and Kings" },
	{ "vengance",           "Vengance" },
	{ "veteranknowledge",   "Veteran Knowledge" },
	{ "viewhome",           "A View to a Home" },
	{ "viewisbetter",       "The View Is Better Here" },
	{ "villagell",          "Village of Love and Lust" },
	{ "virgin",             "Virgin" },
	{ "volant",             "Starship Volant: Stowaway" },
	{ "warlock",            "Warlock" },
	{ "warlord",            "The Warlord, The Princess & The Bulldog" },
	{ "waspconnection",     "The Wasp Connection" },
	{ "wasteland",          "Wasteland" },
	{ "weathertime",        "Time and Weather System" },
	{ "weirdstuff",         "And Then the Weird Stuff Started..." },
	{ "whatever",           "Whatever Happened to Uncle Grumble?" },
	{ "whereami",           "Where Am I?" },
	{ "whitesingularity",   "The White Singularity" },
	{ "without",            "Without a Clue" },
	{ "wizardplayground",   "Wizards Playground" },
	{ "welcwonderland",     "Welcome to Wonderland" },
	{ "trackingdevice",     "Tracking Device" },
	{ "wingman",            "Wingman" },
	{ "world",              "The World According to CBN" },
	{ "xclue",              "XClue" },
	{ "xme",                "X Me" },
	{ "xmen",               "X-Men Evolution: Jean's Quest for Rogue's Nookie" },
	{ "xmen1d",             "X-Men Evolution: First Day at the Institute" },
	{ "xycanthus",          "Doomed Xycanthus" },
	{ "yadfa",              "Yet Another Damn Fantasy Adventure" },
	{ "yeh",                "The Quest (by BoyBiz)" },
	{ "zac",                "Zombies Are Cool, But Not So Cool When They're Eating Your Head" },
	{ "zara",               "Zara's Mansion" },
	{ "ziva",               "Ziva" },
	{ "zombiewalk",         "Bringing Dead NPC Back to Life" },
	{ "zomelec",            "Zomelec" },

	// 1st One Hour Comp 2002
	{ "1h_endgame",         "The Game To End All Games" },                                              //    1st Place
	{ "1h_frog",            "The Green Princess" },                                                     //    2nd Place
	{ "1h_hauntedhouse",    "The Haunted House of Hideous Horror" },                                    //    3rd Place
	{ "1h_jasonvssalm",     "Jason vs. Salm" },                                                         //    4th Place
	{ "1h_1hrgame",         "A Masochist's Heaven" },                                                   //    5th Place
	{ "1h_microbewillie",   "Microbe Willie vs. The Rat" },                                             //    6th Place
	{ "1h_amonkeytoomany",  "A Monkey Too Many" },                                                      //    7th Place
	{ "1h_princess1",       "The Princess in the Tower" },                                              //    8th Place
	{ "1h_chicken",         "The Evil Chicken of Doom!" },                                              //    9th Place

	// 2nd One Hour Comp 2003
	{ "1h_forum",           "Forum" },                                                                  //    1st Place
	{ "1h_dfu",             "Dance Fever USA" },                                                        //    2nd Place
	{ "1h_percy",           "The Saga of Percy the Viking" },                                           //    3rd Place

	// 3rd One Hour Comp 2003
	{ "1h_demonhunter",     "Apprentice of the Demonhunter" },                                          //    1st Place
	{ "1h_shore",           "The Farthest Shore" },                                                     //    2nd Place
	{ "1h_imagination",     "Just My Imagination-Musings of a Child" },                                 //    3rd Place
	{ "1h_ticktick",        "Doom Cat!!!" },                                                            //    4th Place
	{ "1h_forum2",          "Forum 2" },                                                                //    5th Place
	{ "1h_saffire",         "Saffire" },                                                                //    6th Place
	{ "1h_cbn1",            "The Revenge of Clueless Bob Newbie!" },                                    //    7th Place
	{ "1h_cbn2",            "The Revenge of Clueless Bob Newbie Part 2: This Time It's Personal" },     //    8th Place
	{ "1h_ecod2",           "The Curse of the Ghost of the Evil Chicken of Doom... Returns!" },         //    9th Place
	{ "1h_asdfa",           "A Short Damn Fantasy Adventure" },                                         //   10th Place
	{ "1h_crm",             "That Crazy Radioactive Monkey!" },                                         //   11th Place
	{ "1h_pyramid",         "The Pyramid of Hamaratum" },                                               //   12th Place
	{ "1h_questi",          "Quest for Flesh" },                                                        //   13th Place

	// 4th One Hour Comp 2004
	{ "1h_vagabond",        "Vagabond" },                                                               //    1st Place
	{ "1h_arghgreatescape", "Argh's Great Escape" },                                                    //    2nd Place
	{ "1h_topaz",           "Topaz" },                                                                  //    3rd Place
	{ "1h_goblinhunt",      "Goblin Hunt" },                                                            //    4th Place
	{ "1h_wreckage",        "SE: Wreckage" },                                                           //    5th Place
	{ "1h_ecod3",           "An Evening with the Evil Chicken of Doom" },                               //    6th Place
	{ "1h_trabula",         "Get Treasure for Trabula" },                                               //    7th Place
	{ "1h_agent4fmars",     "Agent 4-F from Mars" },                                                    //    8th Place
	{ "1h_cah",             "Cruel and Hilarious Punishment!" },                                        //    9th Place
	{ "1h_thecatinthetree", "The Cat in the Tree" },                                                    //   10th Place
	{ "1h_adriftmaze",      "Adrift Maze" },                                                            //   11th Place
	{ "1h_undefined",       "Undefined" },                                                              //   12th Place
	{ "1h_woof",            "Woof" },                                                                   //   13th Place
	{ "1h_icecream",        "Ice Cream" },                                                              //   14th Place
	{ "1h_shredem",         "Shred 'em" },                                                              //   15th Place
	{ "1h_spam",            "SPAM" },                                                                   //   16th Place

	// 1st Three Hour Comp 2004
	{ "3h_briefcase",       "Briefcase" },                                                              //    1st Place
	{ "3h_jailbreakbob",    "Jailbreak Bob" },                                                          //    2nd Place
	{ "3h_zombiecow",       "Zombie Cow" },                                                             //    3rd Place
	{ "3h_lostsouls",       "Lost Souls" },                                                             //    4th Place
	{ "3h_theamulet",       "The Amulet" },                                                             //    5th Place
	{ "3h_shadrick1",       "Shadrick's Travels" },                                                     //    6th Place
	{ "3h_annihilofthink",  "The Annihilation of Think.com" },                                          //    7th Place

	// 2nd Three Hour Comp 2004
	{ "3h_buriedalive",     "Buried Alive" },                                                           //    1st Place
	{ "3h_veteran",         "Veteran Experience" },                                                     //    2nd Place
	{ "3h_togetyou",        "We Are Coming to Get You!" },                                              //    3rd Place
	{ "3h_morely",          "The Murder of Jack Morely" },                                              //    4th Place
	{ "3h_sandl",           "Snakes and Ladders" },                                                     //    5th Place
	{ "3h_zac",             "Zombies Are Cool, But Not So Cool When They're Eating Your Head" },        //    6th Place

	// ADRIFT End of the Year 2002 Comp
	{ "ey02_goldilocksfox", "Goldilocks Is a FOX!" },                                                   //    1st Place
	{ "ey02_woodsaredark",  "The Woods Are Dark" },                                                     //    2nd Place
	// { "ey02_unravelinggod", "Unraveling God" },                                                      //    3rd Place
	{ "ey02_adriftmasprt",  "ADRIFTMAS Party" },                                                        //    4th Place
	{ "ey02_lairvampire",   "Lair of the Vampire" },                                                    //    5th Place
	// { "ey02_partytomurder", "A Party to Murder" },                                                   //    6th Place

	// ADRIFT Spring Comp 2001
	{ "sc01_menagerie",     "Menagerie!" },                                                             //    1st Place
	{ "sc01_wheel",         "The Wheels Must Turn" },                                                   //    2nd Place
	{ "sc01_chosen",        "Chosen" },                                                                 //    3rd Place

	// ADRIFT Summer Minicomp 2003
	{ "sm03_sunempire",     "Sun Empire: Quest for the Founders" },                                     //    1st Place
	{ "sm03_jimpond",       "Jim Pond & the Agents of F.A.R.T." },                                      //    2nd Place
	{ "sm03_adriftorama",   "ADRIFT-O-RAMA" },                                                          //    3rd Place
	{ "sm03_worstgame",     "The Worst Game in the World... Ever!!!" },                                 //    4th Place
	{ "sm03_levilabyrinth", "The Leviathan Labyrinth" },                                                //    5th Place

	// Beginner's Game Comp
	{ "bg_trickortreat",    "Trick or Treat" },                                                         //    2nd Place

	// Ectocomp 2007
	{ "ec07_videotapedec",  "Video.Tape / Decay" },                                                     //    1st Place
	{ "ec07_witness",       "Witness: Demon vs Vampire" },                                              //    2nd Place
	{ "ec07_stowie",        "The Stowaway" },                                                           //    3rd Place
	{ "ec07_foresthouse",   "Forest House" },                                                           //    4th Place

	// Ectocomp 2008
	{ "ec08_drinks",        "Drinks with Lord Hansom" },
	{ "ec08_r2dc",          "Return to Dracula's Castle II: Revenge of Dracula's Castle" },             //    1st Place
	{ "ec08_foresthouse3",  "For3st House: Sacrifice" },                                                //    2nd Place
	{ "ec08_salutations",   "Salutations" },                                                            //    3rd Place
	{ "ec08_hiker",         "Conversation With a Hitchhiker" },                                         //    4th Place
	{ "ec08_blast",         "Blast" },                                                                  //    5th Place
	{ "ec08_delsol",        "DeL SoL MADNESS" },                                                        //    6th Place

	// Ectocomp 2009
	{ "ec09_drivingnight",  "The Dangers of Driving at Night" },                                        //    1st Place
	{ "ec09_patient7",      "Patient 7" },                                                              //    2nd Place
	{ "ec09_wayout",        "Way Out" },                                                                //    3rd Place

	// Ectocomp 2010
	{ "ec10_petespunkin",   "Pete's Punkin Junkinator" },                                               //    1st Place
	{ "ec10_renegadebw",    "Renegade Brainwave" },                                                     //    2nd Place
	{ "ec10_helsing",       "Steve Van Helsing: Process Server" },                                      //    3rd Place
	{ "ec10_iwasteenage",   "I Was a Teenage Headless Experiment" },                                    //    4th Place
	{ "ec10_thevault",      "The Vault" },                                                              //    5th Place
	{ "ec10_flyhuman",      "The Fly Human" },                                                          //    6th Place
	{ "ec10_allhallows",    "All Hallows Eve" },                                                        //    7th Place
	{ "ec10_tenebrasemper", "Tenebrae Semper" },                                                        //    8th Place

	// Ectocomp 2011
	{ "ec11_attacklobster", "Attack of Doc Lobster's Mutant Menagerie of Horror" },                     //    2nd Place

	// Even Comp 2009
	{ "ec_attmutaydid",     "Attack of the Mutaydid Meat Monsters" },                                   //    1st Place
	{ "ec_rain",            "Bringing the Rain" },                                                      //    2nd Place
	{ "ec_perspectives",    "Perspectives" },                                                           //    3rd Place

	// Finish the Game Comp 2005
	{ "ftg_pathway_adrift", "Pathway to Destruction" },                                                 //    1st Place
	{ "ftg_takeone",        "Take One" },                                                               //    2nd Place
	{ "ftg_demonhunter2",   "The Demon Hunter" },                                                       //    3rd Place
	{ "ftg_hunter",         "The Hunter" },                                                             //    4th Place
	{ "ftg_shadow",         "Shadow of the Past" },                                                     //    5th Place
	{ "ftg_shadowjack",     "Jack of Shadows" },                                                        //    6th Place

	// Hourglass Comp 2006
	{ "hc_longbarrow",      "The Long Barrow" },                                                        //    1st Place
	{ "hc_questforfood",    "Quest for Food" },                                                         //    2nd Place
	{ "hc_herrdoktor",      "Herr Doktor von Nördlingendinkelsbühlhündchen-am-Rhein" },                 //    3rd Place
	{ "hc_3minutes",        "3 Minutes to Live" },                                                      //    4th Place
	{ "hc_smote",           "Smote" },                                                                  //    5th Place
	{ "hc_dancing",         "Dancing Even Him?" },                                                      //    6th Place
	{ "hc_boiledeggs",      "Boiled Eggs" },                                                            //    7th Place
	{ "hc_chooseyourown",   "Choose Your Own..." },                                                     //    8th Place
	{ "hc_rollingdough",    "Rolling the Dough" },                                                      //    9th Place
	{ "hc_roadnowhere",     "The Road Leads to Nowhere" },                                              //   10th Place
	{ "hc_overtheedge",     "Over the Edge" },                                                          //   11th Place
	{ "hc_skydiver",        "The Skydiver" },                                                           //   12th Place
	{ "hc_pilfers",         "Pilfers" },                                                                //   13th Place

	// IFComp 2000
	{ "if00_wrecked",       "Wrecked" },                                                                //   39th Place
	{ "if00_marooned_bd",   "Marooned (by Bruce Davis)" },                                              //   45th Place

	// IFComp 2001
	{ "if01_tcom1",         "The Cave of Morpheus - Part 1" },                                          //   32th Place
	{ "if01_tcom2",         "The Cave of Morpheus - Part 2" },
	{ "if01_mysterymanor",  "Mystery Manor" },                                                          //   43th Place
	{ "if01_thetest",       "The Test" },                                                               //   45th Place

	// IFComp 2002
	{ "if02_pkgirl",        "The PK Girl" },                                                            //    6th Place
	{ "if02_unravelinggod", "Unraveling God" },                                                         //   12th Place
	{ "if02_partytomurder", "A Party to Murder" },                                                      //   28th Place

	// IFComp 2003
	{ "if03_sophie",        "Sophie's Adventure" },                                                     //   16th Place

	// IFComp 2004
	{ "if04_daylifesh",     "A Day in the Life of a Super Hero" },                                      //   23rd Place

	// IFComp 2005
	{ "if05_etny",          "Escape to New York" },                                                     //   11th Place
	{ "if05_mortality",     "Mortality" },                                                              //   12th Place
	{ "if05_vendetta",      "Vendetta" },                                                               //   14th Place
	{ "if05_plagueredux",   "The Plague Redux" },                                                       //   22th Place
	{ "if05_ptbad65",       "PTBAD6.5: The URL That Didn't Work" },                                     //   35th Place

	// IFComp 2006
	{ "if06_unauthtermin",  "Unauthorised Termination" },                                               //   10th Place
	{ "if06_thesisters",    "The Sisters" },                                                            //   12th Place
	{ "if06_requiem",       "Requiem" },                                                                //   19th Place
	{ "if06_wumpusrun",     "The Wumpus Run" },                                                         //   32nd Place
	{ "if06_ptgood",        "PTGOOD" },                                                                 //   43rd Place

	// IFComp 2007
	{ "if07_fineday4reap",  "A Fine Day for Reaping" },                                                 //    7th Place
	{ "if07_mymindmihmas",  "My Mind's Mishmash" },                                                     //   13th Place
	{ "if07_inthemind",     "In the Mind of the Master" },                                              //   14th Place

	// IFComp 2008
	{ "if08_datewithdeath", "A Date with Death" },                                                      //    9th Place
	{ "if08_cybercow",      "Lair of the Cybercrow" },                                                  //   27th Place

	// IFComp 2009
	{ "if09_yonastcastle",  "Yon Astounding Castle! of Some Sort" },                                    //   11th Place
	{ "if09_ascot",         "The Ascot" },                                                              //   15th Place
	{ "if09_hangover",      "Hangover" },                                                               //   24th Place

	// IFComp 2011
	{ "if11_cursed",        "Cursed" },                                                                 //   13th Place
	{ "if11_suzygotpwrs",   "How Suzy Got Her Powers" },                                                //   29th Place

	// IFComp 2012
	{ "if12_irvinequik",    "Irvine Quik & The Search for the Fish of Traglea" },                       //   18th Place

	// InsideADRIFT #41
	{ "i41_cut",            "Cut the Red Wire! No, the Blue Wire!" },

	// InsideADRIFT Game of the Year Comp 2004
	{ "goy_paint",          "Paint!!!" },                                                               //    1st Place
	{ "goy_shardsmemory",   "Shards of Memory" },                                                       //    2nd Place
	{ "goy_waxworx",        "Wax Worx" },                                                               //    3rd Place
	{ "goy_shadrick2",      "Shadrick's Underground Adventures" },                                      //    4th Place
	{ "goy_dragonshrine",   "The Curse of DragonShrine" },                                              //    5th Place
	{ "goy_darkness",       "Darkness" },                                                               //    6th Place

	// InsideADRIFT Game of the Year Comp 2007
	{ "goy_fineday4reap",   "A Fine Day for Reaping" },                                                 //    1st Place
	{ "goy_rking",          "The Reluctant Resurrectee" },                                              //    2nd Place
	{ "goy_marika",         "Marika the Offering" },                                                    //    3rd Place
	{ "goy_puzzlebox",      "The Puzzle Box" },                                                         //    4th Place
	{ "goy_videotapedec",   "Video.Tape / Decay" },                                                     //    5th Place
	{ "goy_terrified",      "Terrified" },                                                              //    6th Place

	// InsideADRIFT Spring Comp 2004
	{ "sp04_shardsmemory",  "Shards of Memory" },                                                       //    1st Place
	{ "sp04_dragonshrine",  "The Curse of DragonShrine" },                                              //    2nd Place
	{ "sp04_waxworx",       "Wax Worx" },                                                               //    3rd Place
	{ "sp04_sommeril",      "Sommeril" },                                                               //    4th Place

	// InsideADRIFT Summer Comp 2004
	{ "su04_chooseyourown", "Choose Your Own..." },                                                     //    1st Place
	{ "su04_shadrick2",     "Shadrick's Underground Adventures" },                                      //    2nd Place
	{ "su04_darkhavenmyst", "The Mystery of the Darkhaven Caves" },                                     //    3rd Place
	{ "su04_adriftproject", "The ADRIFT Project" },                                                     //    4th Place
	{ "su04_tick2nowhere",  "Ticket to No Where" },                                                     //    5th Place
	{ "su04_tearstoughman", "Tears of a Tough Man" },                                                   //    6th Place

	// InsideADRIFT Spring Comp 2005
	{ "sp05_blood",         "Fire in the Blood" },                                                      //    1st Place
	{ "sp05_frustrated",    "Frustrated Interviewee" },                                                 //    2nd Place
	{ "sp05_privateeye",    "Private Eye" },                                                            //    3rd Place
	{ "sp05_seaside",       "A Day at the Seaside" },                                                   //    4th Place
	{ "sp05_hub",           "The House Husband" },                                                      //    5th Place

	// InsideADRIFT Summer Comp 2005
	{ "su05_target",        "Target" },                                                                 //    1st Place
	{ "su05_lights",        "Lights, Camera, Action!" },                                                //    2nd Place
	{ "su05_cibass",        "Can It Be All So Simple?" },                                               //    3rd Place
	{ "su05_mustescape",    "Must Escape!" },                                                           //    4th Place
	{ "su05_regrets",       "Regrets" },                                                                //    5th Place

	// InsideADRIFT Summer Comp 2006
	{ "su06_reluctvampire", "The Reluctant Vampire" },                                                  //    1st Place
	{ "su06_pestilence",    "Pestilence" },                                                             //    2nd Place
	{ "su06_spooked",       "Spooked!: The Wonders of Science" },                                       //    3rd Place

	// InsideAdrift Summer Comp 2008
	{ "su08_happyvalley",   "Happy Valley" },                                                           //    1st Place
	{ "su08_marlin",        "The Marlin Affair: Prologue" },                                            //    2nd Place
	{ "su08_wolvesdoor",    "Wolves at the Door" },                                                     //    3rd Place
	{ "su08_huntung",       "Hunting Ground" },                                                         //    4th Place
	{ "su08_door",          "Door" },                                                                   //    5th Place

	// InsideADRIFT Summer Comp 2010
	{ "su10_aegis",         "Aegis" },                                                                  //    1st Place
	{ "su10_camelot",       "Camelot" },                                                                //    2nd Place
	{ "su10_plunder",       "Pirate's Plunder!" },                                                      //    3rd Place
	{ "su10_baroo",         "Ba'Roo!" },                                                                //    4th Place
	{ "su10_lightup",       "Light Up" },                                                               //    5th Place
	{ "su10_motion",        "Motion" },                                                                 //    6th Place

	// Intro Comp 2005
	{ "ic05_srs",           "Silk Road Secrets: Samarkand to Lop Nor" },                                //    1st Place
	{ "ic05_finalquestion", "The Final Question" },                                                     //    2nd Place
	{ "ic05_murdermansion", "Murder Mansion" },                                                         //    3rd Place
	{ "ic05_zacksmackfoot", "Zack Smackfoot" },                                                         //    4th Place
	{ "ic05_outline",       "Outline" },                                                                //    5th Place
	{ "ic05_mustescape",    "Must Escape!" },                                                           //    6th Place
	{ "ic05_rift",          "Rift" },                                                                   //    7th Place
	{ "ic05_p2p",           "Point 2 Point" },                                                          //    8th Place

	// Intro Comp 2009
	{ "ic09_apokalupsis",   "Apokalupsis" },                                                            //    1st Place
	{ "ic09_dbaa",          "Dung Beetles Are Aliens" },                                                //    2nd Place
	{ "ic09_throughtime",   "Through Time" },                                                           //    4th Place
	{ "ic09_existence",     "Existence" },                                                              //    5th Place
	{ "ic09_teaw",          "To End All Wars" },                                                        //    5th Place
	{ "ic09_donuts",        "Donuts" },                                                                 //    7th Place
	{ "ic09_yoncastle",     "Ye Intro for Yon Astounding Castle! of Some Sort" },                       //    8th Place
	{ "ic09_dishduty",      "Dish Duty" },                                                              //    9th Place
	{ "ic09_deadrace",      "Dead Race" },                                                              //   10th Place
	{ "ic09_merlinbrdprey", "The Merlin Bird of Prey" },                                                //   11th Place

	// Mini-Comp 2003
	{ "mc03_neighbours",    "Neighbours From Hell" },                                                   //    1st Place
	{ "mc03_thorn",         "The Thorn" },                                                              //    2nd Place
	{ "mc03_diarystrip",    "Diary of a Stripper" },                                                    //    3rd Place
	{ "mc03_monsters",      "Monsters" },                                                               //    4th Place

	// No Name Competition
	{ "non_beanstalk",      "Beanstalk the and Jack" },                                                 //       Winner
	{ "non_piecesofeden",   "Pieces of Eden" },                                                         //      Entrant
	{ "non_perfectspy",     "The Perfect Spy" },                                                        //      Entrant

	// One Room Comp 2003
	{ "1r_deadman",         "The Dead Man" },                                                           //    1st Place
	{ "1r_dayattheoffice",  "A Day at the Office" },                                                    //    2nd Place
	{ "1r_msmobius",        "Montahue Scott and the Mobius Belt" },                                     //    3rd Place
	{ "1r_insanity",        "Escape from Insanity" },                                                   //    4th Place
	{ "1r_everything",      "Everything Emanuelle" },                                                   //    5th Place
	{ "1r_i",               "I" },                                                                      //    6th Place
	{ "1r_trappedgirl",     "Trapped With a Girl" },                                                    //    7th Place
	{ "1r_dreamland",       "Dreamland" },                                                              //    8th Place

	// Spring Ting 2009
	{ "st09_reactor",       "ESS Chance: Reactor 1" },                                                  //    1st Place
	{ "st09_mrfluffy",      "Mr. Fluffykin’s Most Harrowing Misadventure" },                            //    2nd Place
	{ "st09_sandy",         "Sandy’s Lost Doll" },                                                      //    3rd Place
	{ "st09_homelesslarry", "Homeless Harry" },                                                         //    4th Place

	// Spring Ting 2010
	{ "st10_wghn",          "Wes Garden's Halting Nightmare" },                                         //    1st Place
	{ "st10_egghunt",       "Easter Egg Hunt" },                                                        //    2nd Place

	// The Challenge Comp
	{ "chg_mangiasour",     "Mangiasaur" },                                                             //    1st Place
	{ "chg_suburban",       "Surburban Prodigy" },                                                      //    2nd Place
	{ "chg_whitterscap",    "Whitterscap's Key" },                                                      //    3rd Place

	// The Odd Competition 2008
	{ "odd_witchtale",      "A Witch Tale" },                                                           //    1st Place
	{ "odd_iamlaw",         "I Am the Law" },                                                           //    2nd Place
	{ "odd_yakshaving",     "Yak Shaving for Kicks and Giggles!" },                                     //    3rd Place
	{ "odd_retforesthouse", "Return to the Forest House" },                                             //    4th Place
	{ "odd_maincourse",     "Main Course" },                                                            //    5th Place
	{ "odd_businessusual",  "Business as Usual" },                                                      //    6th Place
	{ "odd_gorxungula",     "Gorxungula’s Curse" },                                                     //    7th Place
	{ "odd_human",          "Oh, Human" },                                                              //    8th Place
	{ "odd_astaftermath",   "Asteroid Aftermath" },                                                     //    9th Place
	{ "odd_seance",         "Seance" },                                                                 //   10th Place

	// Twin Comp 2009
	{ "tc_crashland",       "Crashland" },                                                              //    1st Place
	{ "tc_melancholy",      "Melancholy Blood: Act 1" },                                                //    2nd Place

	// Writing Challenges Comp 2006
	{ "wri_jgrim",          "Jonathan Grimshaw: Space Tourist" },                                       //    1st Place
	{ "wri_toomuchexer",    "Too Much Exercise" },                                                      //    2nd Place
	{ "wri_glumfiddle",     "Glum Fiddle" },                                                            //    3rd Place
	{ "wri_manoverboard",   "Man Overboard!!!" },                                                       //    4th Place

	// Danish games
	{ "halloween",          "Halloween" },
	{ "illumina",           "Illumina" },
	{ "oct31d",             "October 31st" },

	// French games
	{ "bellesmeres",        "Belles-Mères" },
	{ "enquete",            "Enquête à Hauts Risques" },
	{ "largo",              "Largo Winch" },
	{ "lesfeux",            "Les Feux de l'Enfer" },
	{ "quiatuedana",        "Qui a tué Dana ?" },

	// German games
	{ "ronforestd",         "Forest on the Norm" },
	{ "rontoxicd",          "Toxically Earth" },

	// Italian games
	{ "ilgolem",            "Il Golem" },

	// Spanish games
	{ "bandera",            "Bandera" },
	{ "caidalibre",         "Caida Libre" },
	{ "elascensor",         "El Ascensor" },
	{ "impulso",            "Impulso" },
	{ "n3reloj",            "La Hija del Relojero" },
	{ "nano",               "Nano" },
	{ "renuntio",           "Renuntio" },
	{ "vardock",            "Vardock Bates" },

	// Russian games
	{ "akronr",             "Akron" },
	{ "bookexistence",      "Howard. Oblivion. Part 2: Book of Existence" },
	{ "dolg",               "Debt" },
	{ "golddragon",         "Howard. Oblivion. Part 1: Gold Dragon" },
	{ "nat01",              "Nathaniel Peck: Missing Necklace" },
	{ "nightelmstreet",     "A Nightmare on Elm Street" },
	{ "relife",             "ReLife" },
	{ "shablon",            "Shablon" },
	{ "zanoza",             "Zanoza Bill - Zolotaya likhoradka - Bilet v Vankuver" },

	{ nullptr, nullptr }
};

const PlainGameDescriptor ADRIFT5_GAME_LIST[] = {
	{ "4rooms",             "4rooms" },
	{ "4wallstoilet",       "Four Walls and a Toilet" },
	{ "achtung",            "Achtung Panzer!" },
	{ "advbackyard",        "An Adventurer's Backyard" },
	{ "aliendiver",         "Alien Diver" },
	{ "allthroughthenight", "All Through the Night" },
	{ "axeofkolt",          "The Axe of Kolt" },
	{ "badlands",           "Badlands" },
	{ "beginnerscave",      "The Beginner's Cave" },
	{ "birthofphoenix",     "Birth of the Phoenix" },
	{ "bookbuilding",       "Book Building" },
	{ "canyouguess",        "Can You Guess?" },
	{ "cccs2808",           "CCCS Virtual Campus" },
	{ "charloop",           "Character Info Input with Loop" },
	{ "combilib",           "Combined Library" },
	{ "darkhour",           "The Dark Hour" },
	{ "ddf",                "The Dwarf of Direwood Forest" },
	{ "dementophobia",      "Dementophobia" },
	{ "digitalroots",       "DigitalRoots" },
	{ "dreamspun",          "Dreamspun" },
	{ "edithscats",         "Edith's Cats" },
	{ "escapehouse2",       "Escape From the House 2" },
	{ "evilqueen",          "Evil Queen" },
	{ "expendableitems",    "Expendable Items" },
	{ "fortress",           "The Fortress of Fear" },
	{ "grandma",            "Grandma's Flying Saucer" },
	{ "grandpa",            "Grandpa's Ranch" },
	{ "hintsl",             "Hint System Library" },
	{ "inyourhome",         "In Your Home" },
	{ "isummon",            "I Summon Thee!" },
	{ "jabberwocky",        "Jabberwocky: The Untold Story" },
	{ "jacd",               "Just Another Christmas Day" },
	{ "layers",             "Clothing Layers" },
	{ "loopthrough",        "Loop Through Objects" },
	{ "ml256",              "Makers Local and the Transdimensional Margarita Blender" },
	{ "monsterage",         "Monster Age: Trials of Dustorn" },
	{ "museumheist",        "Museum Heist" },
	{ "nastyescape",        "The Nasty Escape" },
	{ "nycholiday",         "Holiday, Migrate or Bust" },
	{ "oct31",              "October 31st" },
	{ "os",                 "OS" },
	{ "pac",                "Pervert Action: Crisis" },
	{ "penrhyn",            "Penrhyn: Burning Skies" },
	{ "projectactually",    "A Place" },
	{ "rbr",                "Run, Bronwynn, Run!" },
	{ "royalpuzzle",        "The Royal Puzzle" },
	{ "salvage",            "The Salvage" },
	{ "savethekitten",      "Save the Kitten" },
	{ "schoolproject",      "My School Social Studies Project" },
	{ "seymourstuidquest",  "Seymour's Stoopid Quest" },
	{ "snowdrift",          "Snowdrift" },
	{ "soc",                "Son of Camelot" },
	{ "socc",               "The Spectre of Castle Coris" },
	{ "sophia",             "Sophia or Wisdom Defined" },
	{ "sorryforyourloss",   "Sorry for Your Loss" },
	{ "symphonica",         "Symphonica 64" },
	{ "tee",                "The Euripedes Enigma" },
	{ "television",         "Television Channel Changing" },
	{ "tempusfugit",        "Tempus Fugit" },
	{ "theawakeners",       "The Awakeners" },
	{ "tingalan",           "Tingalan" },
	{ "tributereturntocos", "Tribute: Return to CoS" },
	{ "wumpushunt",         "Wumpus Hunt" },
	{ "wwiiee",             "WWII Elevator Escape" },

	// ADRIFT 5 Intro Comp 2012
	{ "5ic_blankwall",      "The Blank Wall" },
	{ "5ic_organic",        "Organic" },
	{ "5ic_shatteredmem",   "Shattered Memory" },
	{ "5ic_trapped_adrift", "Trapped" },
	{ "5ic_headcase",       "Head Case" },
	{ "5ic_axeofkolt",      "The Axe of Kolt" },

	// ADRIFT End of the Year 2016 Comp
	{ "ey16_tlc",           "The Lost Children" },                                                      //    1st Place
	{ "ey16_thewayhome",    "Bash Saga 2: The Way Home" },                                              //    2nd Place
	{ "ey16_dff",           "Die Feuerfaust" },                                                         //    3rd Place
	{ "ey16_heritage",      "The Heritage" },                                                           //      Entrant
	{ "ey16_spacedetect1",  "Space Detective - Episode 1" },                                            //      Entrant
	{ "ey16_spacedetect2",  "Space Detective - Episode 2" },
	{ "ey16_spacedetect3",  "Space Detective - Episode 3" },
	{ "ey16_spacedetect4",  "Space Detective - Episode 4" },
	{ "ey16_spacedetect5",  "Space Detective - Episode 5" },
	{ "ey16_spacedetect6",  "Space Detective - Episode 6" },
	{ "ey16_spacedetect7",  "Space Detective - Episode 7" },
	{ "ey16_thedayprogram", "The Day Program" },                                                        //      Entrant
	{ "ey16_dragondiamond", "Bash Saga 1: The Dragon Diamond" },                                        //      Entrant
	{ "ey16_noblecrook1",   "Noble Crook - Episode 1" },                                                //      Entrant
	{ "ey16_noblecrook2",   "Noble Crook - Episode 2" },
	{ "ey16_noblecrook3",   "Noble Crook - Episode 3" },
	{ "ey16_noblecrook4",   "Noble Crook - Episode 4" },

	// ADRIFT Game of the Year 2019 Comp
	{ "ey19_skybreak",      "Skybreak!" },                                                              //    1st Place
	{ "ey19_tcots",         "The Call of the Shaman" },                                                 //    2nd Place
	// { "ey19_treasurehunt",  "Treasure Hunt in the Amazon" },                                         //    3rd Place
	{ "ey19_lazaitch",      "The Lost Labyrinth of Lazaitch" },                                         //    4th Place
	{ "ey19_starshipquest", "Starship Quest" },                                                         //    4th Place
	{ "ey19_magneticmoon",  "Magnetic Moon" },                                                          //    6th Place
	{ "ey19_rsp",           "Revenge of the Space Pirates" },                                           //    7th Place
	{ "ey19_deadwinter",    "The Dead of Winter" },                                                     //      Entrant
	{ "ey19_bethere",       "Be There!" },                                                              //      Entrant

	// Anyone in the mood for a comp?
	{ "moo_algernon",       "Algernon's Conundrum" },                                                   //    1st Place
	{ "moo_beagle",         "Beagle" },                                                                 //    2nd Place

	// Beginner's Game Comp
	{ "bg_ttp",             "The Tartarus Project" },                                                   //    1st Place
	{ "bg_lastexpedition",  "The Last Expedition" },                                                    //    3rd Place

	// Ectocomp 2011
	{ "ec11_deathshack",    "Death Shack" },                                                            //    3rd Place
	{ "ec11_ignisfatuus",   "Ignis Fatuus" },                                                           //    4th Place
	{ "ec11_thehouse",      "The House" },                                                              //    6th Place
	{ "ec11_stuckpiggy",    "Stuck Piggy" },                                                            //    7th Place

	// Ectocomp 2012
	{ "ec12_beythilda",     "Beythilda the Night Witch" },                                              //    5th Place
	{ "ec12_ecod3d",        "The Evil Chicken of Doom 3D" },                                            //    6th Place

	// IFComp 2011
	{ "if11_rtc",           "Return to Camelot" },                                                      //   30th Place

	// IFComp 2017
	{ "if17_lmk",           "Land of the Mountain King" },                                              //   58th Place
	{ "if17_temperamentum", "Temperamentum" },                                                          //   72th Place

	// IFComp 2018
	{ "if18_6silvbullets",  "Six Silver Bullets" },                                                     //   31th Place
	{ "if18_stoneofwisdom", "Stone of Wisdom" },                                                        //   44th Place
	{ "if18_anno1700",      "Anno 1700" },                                                              //   61th Place

	// IFComp 2019
	{ "if19_treasurehunt",  "Treasure Hunt in the Amazon" },                                            //    3rd Place

	// IFComp 2020
	{ "if20_jaft",          "Just Another Fairy Tale" },                                                //   84th Place
	{ "if20_rtcc",          "Return to Castle Coris" },                                                 //   86th Place

	// IFComp 2022
	{ "if22_lostcstlines",  "Lost Coastline" },                                                         //   28th Place

	// InsideADRIFT #41
	{ "i41_ambassador",     "Ambassador to Dupal" },
	{ "i41_bariscebik",     "Bariscebik" },
	{ "i41_canyoustandup",  "Can You Stand Up?" },
	{ "i41_colomc_adrift",  "Color of Milk Coffee" },
	{ "i41_dontgo_adrift",  "Don't Go" },
	{ "i41_murdererleft",   "What the Murderer Had Left" },
	{ "i41_readmay_adrift", "A Reading in May" },

	// P/o Prune's Mini Comp
	{ "mc_diffipuzzle",     "A Difficult Puzzle" },                                                     //    1st Place
	{ "mc_gardenparty",     "The Garden Party" },                                                       //    2nd Place
	{ "mc_illumina",        "Illumina" },                                                               //    3rd Place
	{ "mc_questgiver",      "Quest Giver" },                                                            //    4th Place

	{ nullptr, nullptr }
};


const GlkDetectionEntry ADRIFT_GAMES[] = {
	// English Games
	DT_ENTRY1("1sttime", "080531", "2ee60ee51dc1a2c672df78a3c91614bc", 29101),
	DT_ENTRY1("2playerex", "030219", "cc4a6e2019115daece9f94d1c66082dd", 1890),
	DT_ENTRY1("30seconds", "020716/Demo", "85f0c04222a017f1139cbbd965d7e026", 612),
	DT_ENTRY1("3monkeys", "0310??", "dec34c282511b1eadfe231dbbb49f625", 79286),
	DT_ENTRY1("abduction", "050130", "57ede4aec6ddfea72252b1e1dd594fd9", 55843),
	DT_ENTRY1("adriftproject", "040831", "fcce2118d99574eb724364d36dc71d0b", 231621),
	DT_ENTRY1("adv350_adrift", "06????", "1d50a4f82fda17e06a178fd634ced8b4", 45400),
	DT_ENTRY1("adventurepostman", "180131", "bbdd2a34e831964eafb5c06562429c74", 6668),
	DT_ENTRY1("adventurestrikes", "060913", "f5e18081fd6ec4955ed0665aee8580de", 3770),
	DT_ENTRY1("adventurethumper", "020221", "d050a635699242cbfd383f7e16fe3a63", 107200),
	DT_ENTRY1("afp", "110308", "218d6543f45a7cd921cb7c2187f9600e", 79881),
	DT_ENTRY1("akari", "040322", "71a1137269234ee0941e54965c951c73", 31094),
	DT_ENTRY1("akron", "000805", "2461ceeef3ef1eac15a2051a549b4402", 22258),
	DT_ENTRY1("albertlost", "100501", "3667644307d9e9683eb88b1fd4f90562", 279251),
	DT_ENTRY1("albridgemanor", "020713", "268003e454c4ade042d593af8397a490", 31353),
	DT_ENTRY1("alchemist", "03???\?/v1", "730b4f2e255024c501f28456324dc1b9", 165299),
	DT_ENTRY1("alchemist", "03???\?/v2", "c22b910b9c9703dd190c6bce71532bae", 165297),
	DT_ENTRY1("alex", "040401", "b84a1311dfe329f710f3b7760b08292b", 29468),
	DT_ENTRY1("alexis", "020330", "d1adc72ae5f19e18e386ec1a3cab7212", 87229),
	DT_ENTRY1("alias", "020807", "14176ef2af8207d54b6c4ae6a8938046", 9213),
	DT_ENTRY1("allroadsmars", "080209", "703cb672f13f7174a37c8d379abe466f", 2006),
	DT_ENTRY1("alrhinitsys", "??????", "799a0f2f0d9e44fd7542a13638c9f087", 3763),
	DT_ENTRY1("amazgriswold", "0505??", "a044a6c6117b98aaa81c4083a2144946", 18992),
	DT_ENTRY1("amaztvcaper", "060920", "f456b996a5dcb6037e75196406e3c86d", 6187),
	DT_ENTRY1("americanmoney", "011025", "ecbe30e0a0e1610591f11e4538abd837", 2356),
	DT_ENTRY1("amnesiakid", "040408", "fa5998636923d0319b7a62ac685a49b7", 10268),
	DT_ENTRY1("amy", "030326", "be11b8af8fb1885c684f34962a32a887", 23957),
	DT_ENTRY1("angel", "070702", "a4508d6fbbd89ead3e72e961fe288427", 4429),
	DT_ENTRY1("aquarius1", "010207", "46b916da57f20730af8efa872acf1dd5", 13043),
	DT_ENTRY1("aquarius2", "010209", "7b0d73d080a9a456591a8b0144788fbe", 18961),
	DT_ENTRY1("aram", "050220", "442cc382e180eabe878f69c1d8470374", 18765),
	DT_ENTRY1("archiebdaych1", "051101", "fe52abda2b95ef53b23fb0fbaa7cbae7", 284319),
	DT_ENTRY1("armour", "050730", "4e9f3b70ef54f736fe18d40087248ff6", 315),
	DT_ENTRY1("armour", "050731", "e42a5cde88dbb8dd07df855c785a9564", 279),
	DT_ENTRY1("asylum_ms", "030906", "5d03f29f530701db4dc096b9e7307ae4", 44503),
	DT_ENTRY1("athylon", "030524", "431cfbc8a19c86c40cb9e4e57a733926", 6456),
	DT_ENTRY1("attack", "040128/Demo", "c26f50555e90b95772402c6517ca8614", 1391),
	DT_ENTRY1("average", "011223", "bc4d0db4a6b95136bdb92aae623b7786", 11350),
	DT_ENTRY1("awalkatdusk", "050202", "7baf130db0c1f24b198a6cf55fe704f7", 20529),
	DT_ENTRY1("awalkatdusk", "050718", "f2e8aca65a17d41804709ee5a420ef36", 20725),
	DT_ENTRY1("azra", "010416", "f89eb8b96ff1810c5d8430a1b521ad51", 44145),
	DT_ENTRY1("azra", "021126", "26c4688cb6f66c9540fc4e603145e385", 13868),
	DT_ENTRY1("backhome", "110925", "bc13667aac95da199cf6ad32a51410dd", 31171),
	DT_ENTRY1("backhome", "120818", "5b086640b6099b686e8439d027449891", 36350),
	DT_ENTRY1("backtolife", "041227", "8493ffd877273ef95c88c8f433126473", 55438),
	DT_ENTRY1("backtolife", "050114", "e90cc6d87ce02fd0fa86ca1fdb4fbc54", 75316),
	DT_ENTRY1("bananadvent", "080622", "e5b4283f59f5d4feb322c653c7dd8cc0", 2745),
	DT_ENTRY1("barneysproblem", "040102", "22495d3bc10a568b8dfa2571d8c85751", 66056),
	DT_ENTRY1("barneysproblem", "040115", "3fe45352a6c0d7bb0bdbf4f9c08afba7", 66055),
	DT_ENTRY1("basement", "010703", "bd037c5c85b73880caa75a65f6efcfca", 3237),
	DT_ENTRY1("bedlam_mw", "011202", "a86a6d6f3c54e787aafd9f9a8c9bef86", 21344),
	DT_ENTRY1("bluesky_un", "020811", "32104bad81f6502654cf100bb5edd0e1", 30629),
	DT_ENTRY1("bcl", "100419", "14bfd0a8a53e95d23f8a5dcb3a79ae90", 8088),
	DT_ENTRY1("bdw", "110828", "85540c66a7a48a518572fb34bef5c1f8", 9723),
	DT_ENTRY1("beerisntenough", "030214", "98248a8e65a36f6d0fb98111255a85dc", 14070),
	DT_ENTRY1("beerisntenough", "030601", "8962dd84cee64d5239af7c6c50cc0c0d", 14186),
	DT_ENTRY1("beethro", "040307/Demo", "3d362adf355f2e0b0016703f9ca90eb0", 18602),
	DT_ENTRY1("bfhouse", "010817", "56f654ad401311cd827fb7a53354e0f4", 8148),
	DT_ENTRY1("bigdayout", "021104", "477e241b91f7154f4b82c4be54d5e364", 352208),
	DT_ENTRY1("bigspyfictionch1", "180421", "220f5d03bc089e1618ab358820b1eaca", 2051),
	DT_ENTRY1("bigspyfictionch2", "180422", "b27751a30a60d36a78a603560d9e41fd", 1989),
	DT_ENTRY1("bigspyfictionch3", "180422", "1e8392e3715bc5d553abcd1e0db50c0f", 2729),
	DT_ENTRY1("bigstuff", "030224", "b1774ed47e169020b33807deb0db46fd", 141374),
	DT_ENTRY1("birthday", "031219", "14d55b781eb392bb0f120dee857f6486", 4150329),
	DT_ENTRY1("blacksheep", "030209", "ac466de9e8f2e660bfe0c03b47f43d85", 29293),
	DT_ENTRY1("blob", "070219/Demo", "f6a852a1b4588be3848d4158fdd5325e", 463),
	DT_ENTRY1("bobbobsly", "010318", "de60977da0c1ddbcc467d2fedd9751bd", 10310),
	DT_ENTRY1("bombthreat", "010511", "150218c0377528a8353cb44a6806d5dd", 17955),
	DT_ENTRY1("bountyhunter", "030524", "6c23ce1ba2fd6aa9abc958442d14579b", 4057),
	DT_ENTRY1("breakable", "040402/Demo", "269f443d8d7102efc2c77eddba9d3672", 2135),
	DT_ENTRY1("breakout", "040313", "4d3df7d9ec87d33a6651d9d299ff9cd8", 24718),
	DT_ENTRY1("britishfox", "040713", "6695dacace136957c07584c0384ffbab", 220489),
	DT_ENTRY1("bsg22", "090527", "6f52a023828e24449e6181c50a10a50c", 35681),
	DT_ENTRY1("buffy", "110517", "0bd99db69a4e9654eb1bc02bd04679f1", 125581),
	DT_ENTRY1("bulkitem", "090408/Demo", "f0d0241ed7fd59ab40342ab414114a40", 622),
	DT_ENTRY1("burbs", "040130", "703a6c886a15db2391b8a929455c2719", 171239),
	DT_ENTRY1("bz3americans", "040605", "c78f5f1b1347b149499464534e358e49", 5402),
	DT_ENTRY1("bz3soviets", "040608", "199985dba2074122502af4c4422b8f1a", 4209),
	DT_ENTRY1("cabin", "050119", "bd0065f2199cd2edbfcb7cfc5d7978d3", 21823),
	DT_ENTRY1("campwindylake", "020619", "33ea1f1fea9998472e9c352ea6e1c1d8", 114698),
	DT_ENTRY1("campwindylake", "050123", "a5b82608d9c00605795ae9f0f75c01c3", 115119),
	DT_ENTRY1("campwindylake2", "030222", "d715d28f78eddebcbaea61987547bd1c", 191548),
	DT_ENTRY1("canidoit", "051027", "072f42e0a6d340e12885cae34e14bfd7", 5161),
	DT_ENTRY1("captive", "010321", "11409953b0f0042e9a36fabd8c46363f", 74568),
	DT_ENTRY1("car_an", "021025/Demo", "24ce042d41969aae4eff41b980174ffb", 525),
	DT_ENTRY1("car_sm", "021125/Demo", "970643c6a2eaeb20b42706759b334ba1", 1027),
	DT_ENTRY1("cards", "060821", "a4b8feeb2deea9f8c7abeb77a1af7586", 2422),
	DT_ENTRY1("casino", "040513", "e9bf9258d2219faed5f86b49e50a5433", 114649),
	DT_ENTRY1("castlequest", "000910", "485e9c1c15fb40b9da9f5bb865b36854", 14777),
	DT_ENTRY1("cat", "030610/Demo", "b45b97920aab576f37fdc9ef7e13825a", 588),
	DT_ENTRY1("catburglar", "040511", "02eb0b97435e8bc64ecd07ea20b88878", 23189),
	DT_ENTRY1("caveofwonders", "000626", "f55f4e63886eb09e1f89bea547574919", 85471),
	DT_ENTRY1("cd", "030822", "72d56f65dfdfded022f06c6d1dbc5294", 6312),
	DT_ENTRY1("cell", "010625", "3a6196e757d96ca07dab638325698c34", 20800),
	DT_ENTRY1("cellar", "070611", "acce6030ec753bffc6023f2d8cdc8994", 24032),
	DT_ENTRY1("cellpart1", "070520", "ad727adff523f2402a47901048c8e0e5", 18439),
	DT_ENTRY1("cellphone", "060702/Demo", "08395bf21cd1c0e2981def85864d7015", 4192),
	DT_ENTRY1("chairtest", "040415/Demo", "5dfb2744edc4e8521936f33ee266e598", 494),
	DT_ENTRY1("changepicture", "050821/Demo", "1f8f81330cf5f8b4259363c20ecdb1a7", 39732),
	DT_ENTRY1("charactercreator", "020524", "486cecd739b228f00d16141192b4cf54", 9363),
	DT_ENTRY1("charactercreator", "020601", "5c68e2721487e9f4ae8328da27270b8b", 12562),
	DT_ENTRY1("charactercreator", "020629", "21269fa2721a6d2397a8e570f7185498", 16329),
	DT_ENTRY1("charactercreator", "020630", "42914213ec25cf038e5a5d5e6935db79", 27074),
	DT_ENTRY1("characterturn", "040207/Demo", "6831efcef857abbf21e1f5113e7b79c7", 550),
	DT_ENTRY1("chargenerator", "020110", "fe37edc2577db84d20eba9659f198f1e", 18918),
	DT_ENTRY1("chasingrussian", "051210", "cb34a8cd2c852e34286b8913c61fb3e1", 35967),
	DT_ENTRY1("chicago", "011007", "c511775b25d9897b04df67ecd01e4af1", 19755),
	DT_ENTRY1("choices", "040214", "ac22dd465213347de1b97e03240a9bb7", 84761),
	DT_ENTRY1("chooseyourown", "040916", "67ac6321ad8be84bf6933c76c8e549bb", 54875),
	DT_ENTRY1("christpresent", "031219", "dfacdffa6910c548fa129c1270135060", 26882),
	DT_ENTRY1("church", "041128", "24e037d68d0597b43b5747c7d718f608", 40146),
	DT_ENTRY1("cif", "030306", "f146767064244176f8f40d03d790e6e7", 248182),
	DT_ENTRY1("cif", "030803", "a6f625f184fa848ee6746e9aa6c94915", 248088),
	DT_ENTRY1("cldone", "060401", "0ea3949348de043a914c18181230ef35", 97018),
	DT_ENTRY1("clear", "040430/Demo", "d0d8d3a1848668de28c9534a89c4abd3", 579),
	DT_ENTRY1("cleft", "010913", "59ebfd6a539bc98347ce5c95e1a861ec", 11820),
	DT_ENTRY1("cloak", "000910/Demo", "be07e768b470920fe16f6aa6831d312d", 4936),
	DT_ENTRY1("clock", "040129/Demo", "db977ba50ce044a599b334b8a2867623", 710),
	DT_ENTRY1("clodquest", "0903?\?/Demo", "ac5887ce48635c45475bcaba1e2926a5", 59848),
	DT_ENTRY1("cluelessbob", "050516", "b3015b3ef35ecb99909d1e91f1b7e4cd", 35109),
	DT_ENTRY1("cobl", "050717", "09b2348c28d18b31b14da4305be38a61", 22995),
	DT_ENTRY1("colony", "010128", "3cc46c5c44e1e853c6e8aeca4f3624db", 14781),
	DT_ENTRY1("com", "070924/Demo", "e376e159e41d8a46883d543b7f0acc90", 2221),
	DT_ENTRY1("combat", "021017/Demo", "69ce1c3cebc83b928ebc9d423cecab53", 1481),
	DT_ENTRY1("combat", "030220/Demo", "f4b54ddd2d9aa2db4c390ba772a5420d", 3422),
	DT_ENTRY1("community", "041013", "af4b289b443055ab16deab1eb71ff6dc", 73874),
	DT_ENTRY1("commuting", "050621", "a217febbdd2f2a34ebcf6e5ea6bf7897", 30230),
	DT_ENTRY1("compendiumendgame", "020101", "ad76afa2fcdb8c9e2272a115cce6bd4f", 4092),
	DT_ENTRY1("compendiumforum1", "020101", "273b73bf6262f603bee198238bc02196", 9128),
	DT_ENTRY1("compendiumforum2", "020101", "b4af4a085707f221dbabd2906e411d29", 11172),
	DT_ENTRY1("compendiumimagi", "020101", "3cfd6a4dd7ce552afdc52d0923e079cd", 9273),
	DT_ENTRY1("compendiumsaffire", "020101", "d8a0413cdb908ba2ac400fe6a880d954", 7486),
	DT_ENTRY1("compendiumtopaz", "020101", "5f91c9cd4391b6e44c2c052698d01118", 4866),
	DT_ENTRY1("consequences", "051201", "9e6b2d926f4d7beb485570bb598443aa", 542253),
	DT_ENTRY1("cowboyblues", "030418", "23eabfd5db63ded776dd0323d2abe7ea", 111835),
	DT_ENTRY1("crawlersdelight", "050721", "9da704541689f95b3636ad729cfda5f4", 18705),
	DT_ENTRY1("crimeadventure", "030524", "1bde8c066f6a38ac74ca67d5fa90e0c7", 3669),
	DT_ENTRY1("crimeadventure", "000813", "965eafa4579caa3bb8382a07b5e1771e", 15073),
	DT_ENTRY1("crimelife", "020527", "866b152bb8f718ad1c9437c4400d2e58", 25642),
	DT_ENTRY1("croftnon", "030222", "9dedbea3ca0ff1032bea290c27a5a7ad", 148447),
	DT_ENTRY1("croftnon", "030410", "0679075bcdb07776660a15e15d248270", 70722),
	DT_ENTRY1("crookedest", "110623", "f534e863b144b1e35c592e1740a00d68", 8745),
	DT_ENTRY1("crossworldsp0", "060925", "698820922f4e37937b5a947cd60e3e44", 184901),
	DT_ENTRY1("crossworldsp1", "040229", "8f3e45b03fe25a3b13a8c3a3aceb220c", 110897),
	DT_ENTRY1("crossworldsp2", "050105", "19de1f0516a954f3f224758779f1ee06", 165825),
	DT_ENTRY1("crossworldsp3", "051210", "f1bc090495051f41ca629c80372084f8", 220578),
	DT_ENTRY1("crossworldsp4", "070202", "7b04adb511c66fa0b22e256245aad78f", 376908),
	DT_ENTRY1("cumberbund", "090929", "f96d2e9840d7a1a1d71dfd742ce01803", 1865),
	DT_ENTRY1("cyber1", "050305", "b53f7a302edad4d9e2a3310dc1c66e72", 1786),
	DT_ENTRY1("cyber2", "050305", "535c161818ee238e112b1d00bd082b26", 3384),
	DT_ENTRY1("darkfantasy", "040224", "f70f6df807964755fad27ce57960cf2b", 29442),
	DT_ENTRY1("darkness", "051008", "cdf8d81b96e1778c810b28be4341918f", 25248),
	DT_ENTRY1("darkness", "061224", "f02a5be2f9a2fd59ef10f3d16157ee27", 25238),
	DT_ENTRY1("darktower", "020111", "6f544f097ce212ab8b33ecfa1cac0ccb", 39485),
	DT_ENTRY1("daveshouse", "010207", "2acccf1ea2b5a3391d7f33150cd4b81f", 70579),
	DT_ENTRY1("daynightcycle", "030130/Demo", "b6b2fd7e8adb3f87098f65cd656110fb", 417),
	DT_ENTRY1("dayschool", "080309", "14d2e091f8fa7c7a5877ebdc445af4fd", 119682),
	DT_ENTRY1("daystemplate", "011120", "92cba739c4f4566b83c33f4feac362d5", 9864),
	DT_ENTRY1("dead", "060620/Demo", "ff5ec87f8add5b4cfef50428e72a299d", 1012),
	DT_ENTRY1("deathbedcash", "021129/Demo", "6e48a69211c0c63513dba2a7f4b4b652", 580),
	DT_ENTRY1("decisions", "0506??", "b79e2a2e6584f9d6fbddd8fe611bcead", 148913),
	DT_ENTRY1("deadoralive", "050115", "f011e61ed867f93a404fe7e15d51558b", 286388),
	DT_ENTRY1("deadreckon_adrift", "031221", "c49f787df29d6a73e77a0a33386614d3", 82279),
	DT_ENTRY1("deardiary", "020609", "f956d7fefbe51dee0846bd864df0b775", 71076),
	DT_ENTRY1("deardiary2", "040822", "65d6189a1b2bdd11c85134398f6a69d5", 75243),
	DT_ENTRY1("deathdoor", "010217", "4df8f62198293a7ae569c7b4de77eb72", 25058),
	DT_ENTRY1("delight", "050714", "1ec450592e2eb47e5b6b91b331ca6a52", 18703),
	DT_ENTRY1("delivery", "171227", "3217f2a650d13c59f9da74a68e850c81", 7622),
	DT_ENTRY1("demonhunter", "051212", "cc9c55b0f67577f4e9bbaa388a61c494", 18643),
	DT_ENTRY1("demonhunter2", "061020", "aede840a7cfc1d82cc15d705a82d9410", 18725),
	DT_ENTRY1("devilchild", "030929", "66773381d60387364c66793a2a347876", 49377),
	DT_ENTRY1("dickynoodle", "010921", "1eff2af99b0b4d4fd674f01bce7fd6d0", 48044),
	DT_ENTRY1("digby", "060404/v1", "301eab10bb6f882ff38b40ef79c1d252", 48365),
	DT_ENTRY1("digby", "060404/v2", "770eebe936abff4de71fc6f15f7842bc", 48062),
	DT_ENTRY1("dinnerparty", "021107", "62683f3525d64684531a82984bc580c7", 37197),
	DT_ENTRY1("disambtest", "100531/v1", "69c6c3a16bc0b79a8f26bc95f7227336", 441),
	DT_ENTRY1("disambtest", "100531/v2", "0102b5e347eb57f3376df4b018d6cb71", 587),
	DT_ENTRY1("display", "020507", "beab90b4f3078113f981b4f7413f46b0", 1635),
	DT_ENTRY1("door2", "041201/Demo", "8b2e10db0ed489b59a74618c2e2aeaa0", 434),
	DT_ENTRY1("doortest", "020705/Demo", "f7842f2b8c90ce1217e8077cf09574ef", 805),
	DT_ENTRY1("doortoutopia", "021017", "9e727b8ad5b9c62d712d4a302083f6d8", 15432),
	DT_ENTRY1("doortoutopia", "021112", "a2a1b1fc91aa7e8557bfa29f241a8bb1", 18188),
	DT_ENTRY1("doorwithsensor", "040110/Demo", "020977ae43897fe72bdceb164c27c741", 803),
	DT_ENTRY1("dragonsheart", "011016", "a01f55139780cc0c3d48d6ad9265ad4a", 5912),
	DT_ENTRY1("dragonsphere", "001218", "e0fd0fef272cec3d5a9045738ab18530", 22738),
	DT_ENTRY1("dragonsphere", "081115", "63e9745b9b85cd2161e6248a64bb3496", 6421),
	DT_ENTRY1("dreamcometrue", "060402", "1cac5bc2ba392055bfc923628e385969", 191385),
	DT_ENTRY1("dreamquest", "030524", "894cc0faedf186fe0f54536872bfc0ad", 26175),
	DT_ENTRY1("dressbygender", "040729/Demo", "058f39d5f7395c402840d5cd5644882c", 529),
	DT_ENTRY1("driftingin", "011111", "7ee4b9d8cc9547b22a3fac8e34a70a03", 34637),
	DT_ENTRY1("droneacademy", "060104", "8f39111d7f8419a9100a42a0b9e2b796", 50090),
	DT_ENTRY1("druggylane", "011121", "ad740fa9cca9d13d1bd3d321a8f25019", 12028),
	DT_ENTRY1("drwhovortexlust", "020705", "e2b76d720096fb6427927456fc80c5a2", 166913),
	DT_ENTRY1("duchess", "010709", "7ff181282ecb89291599e4332b814017", 150682),
	DT_ENTRY1("duck", "050305", "d4b10dcad94ca6167139622421198380", 2498),
	DT_ENTRY1("easterpeeps", "0605??", "f9f7c5179f4dbda76be638ada1dc3b1b", 21295),
	DT_ENTRY1("eatfull", "021121/Demo", "18ba7253a21dab4c1655d8b0abbd3f36", 354),
	DT_ENTRY1("ebonysworld", "010720", "2ce6173633b5f49a08c0c2df018a13f8", 25668),
	DT_ENTRY1("emotions", "040719/Demo", "c634a5e84887ae043589060efd4aca43", 1044),
	DT_ENTRY1("enc1", "020605", "d297f4376e5ba5b05985ca12a60d8671", 101668),
	DT_ENTRY1("enc2", "040730", "4bd8ebd78a0a227510f62f2074e60ee5", 120335),
	DT_ENTRY1("enigmacreature", "020918", "8cac4426368204e34bf2316d86d35353", 10627),
	DT_ENTRY1("escapecamelot", "090805", "aa00c4cd06c40e303de81208e38723c4", 240667),
	DT_ENTRY1("escapefromsouth", "001125", "3a742c4bb03442749a4f89dc12e0eef1", 15407),
	DT_ENTRY1("escapefromsouth", "030524", "5b2608a2b7963ce2ba117dd361c91936", 4648),
	DT_ENTRY1("escapetofreedom", "050616", "a7edcb289eb8177a143a96912ec55393", 18171),
	DT_ENTRY1("escapetofreedom", "061112", "41b58982572f1275831ce2a9367135f2", 18156),
	DT_ENTRY1("eveningwithecod", "040301", "bd783057274a626868b052915bad2a2c", 10870),
	DT_ENTRY1("eveningwithecod", "040325", "736f0276c4028ccd972d391f0b30ab14", 3937),
	DT_ENTRY1("event", "061021/Demo", "7b9b2c949b584c838dc950454c3ee43c", 561),
	DT_ENTRY1("expsystem", "020808", "fc8043096e6594cf0ffdaa74be858552", 3615),
	DT_ENTRY1("fairscarenightmare", "050221", "975c72341afb6664df6cbecc28319596", 8121),
	DT_ENTRY1("fairytales", "030403", "8170fe61dfcdfd8129843166d1c4cfe5", 111015),
	DT_ENTRY1("fantasy", "040407", "82ecafb3fcce21e75f2d9cc6988f6904", 20187),
	DT_ENTRY1("fantasyworld", "010919", "6a2d5a6c8a775e9565853348277c61a4", 250039),
	DT_ENTRY1("farfromhome", "020723", "b84df75a6ff647006f1f2783bde114f7", 42118),
	DT_ENTRY1("farfromhome", "020809", "58d03e96c3500d6e804f8a4cafbe08a5", 14550),
	DT_ENTRY1("faucets", "040706/Demo", "3f2ae204587b9dd7ae97d33c00c72022", 763),
	DT_ENTRY1("filthybill", "010316", "e76a9f1939f7ccacbb6d7e2c4a55c846", 23849),
	DT_ENTRY1("findandy1", "080818", "5210c86752b788c9bc512cceed3bdab7", 28236489),
	DT_ENTRY1("findandy1", "100309", "ca1aeba91f2d17dff959f73da8b48f03", 27512645),
	DT_ENTRY1("first", "000915", "a44aac0441648f68a85e1cbaef4c5e53", 12213),
	DT_ENTRY1("firstarise", "040105", "d9d205cb6bcedbd159c826287b71a4e6", 13390),
	DT_ENTRY1("firstday", "010725", "00e1d28a1357484d3baf7a7e5de0bf9d", 13892),
	DT_ENTRY1("foresthouse", "070903", "e22e9f6dcc7870a82280dd4ca9ea87f2", 6269),
	DT_ENTRY1("foresthouse", "080630", "fc3b75ac5b5a6e13d415dd8049de99ad", 9476),
	DT_ENTRY1("fox", "010905", "1923d86c2614a35653c1a5234078f8fa", 28216),
	DT_ENTRY1("freshman", "040401", "6cb3921b3624620c09d5f14cfa0686a9", 21733),
	DT_ENTRY1("fugitive", "0106??", "4e81d29f22b743c3f57b5be433b592ed", 184974),
	DT_ENTRY1("fullcircle", "120821", "3de7dd74666f51ca89d37edc75ae85f3", 68233),
	DT_ENTRY1("funhouse", "030524", "ae8f15c95e8589392e4824acddaa64a7", 3585),
	DT_ENTRY1("funtown", "021013", "5635cee9cab080ae36a4d43af8425d69", 54419),
	DT_ENTRY1("g7056", "031217", "ad4cfe7411d00b57d432a4bdb1db3bc8", 9184),
	DT_ENTRY1("gallows", "05???\?/v1", "fce80062c50c4a44b3c8681c6fd101b6", 38073),
	DT_ENTRY1("gallows", "05???\?/v2", "a62d1e2f2f1a639602577b78752354c4", 38117),
	DT_ENTRY1("gamemaster", "060427", "cd443d0250e176a8add7ba69e6261191", 31268),
	DT_ENTRY1("gammagals", "020619", "db813e61ded3e7f642d437ef1e77d0b3", 277834),
	DT_ENTRY1("gateway", "010614", "b641bd132791bd847e9cc1aaa879e19f", 81139),
	DT_ENTRY1("gettingeven", "010614", "dfa8a7f2d58ea2ba9a167e41155b02b1", 123102),
	DT_ENTRY1("ghoster", "041027", "23ef182c934ba46bbda93c8290d6cdd8", 8031),
	DT_ENTRY1("ghostjustice", "040827", "3548fcc0efaa5170b6d1fa6007fbd158", 94632),
	DT_ENTRY1("ghoststory", "040604", "018c8441504e69e63e2aa9faaa7d063e", 16022),
	DT_ENTRY1("ghosttown_fi", "090421", "cfbe1e8099cdc9850aa006593bf46489", 30205598),
	DT_ENTRY1("ghosttown_fi", "090523", "60c11a24853dec5e6a292914c6b4f7f2", 30205645),
	DT_ENTRY1("ginger", "030422", "7e43ee9c3e0a685a6812ed99cadb816e", 18471),
	DT_ENTRY1("gmylm", "090929", "f757a3642284738d51b7540c1fe24409", 29488),
	DT_ENTRY1("gmylm", "100704", "67b61f16ca39d8abc9eb770385ef0e50", 15194757),
	DT_ENTRY1("go", "010408", "ea447aa7dea7164abad03517bc678367", 34724),
	DT_ENTRY1("goblin", "070511", "136fd82dd4313e617165198e8bd9f695", 186729),
	DT_ENTRY1("goldilocksbreakent", "080209", "0800e11d1a3c54bd56fe57f21cf2b2f5", 8398),
	DT_ENTRY1("golf", "040927/Demo", "bf75533058a1d9a2cab18eb9561ae92b", 752),
	DT_ENTRY1("golfball", "040926/Demo", "7b9b9e70f50d6b230d37f6595f1f0186", 230),
	DT_ENTRY1("gosha", "050320", "11abaedb5650bcc47cbb4daed44a36f8", 5625),
	DT_ENTRY1("gotcha", "040331", "3c95a09e952f11bddd5725a09431184a", 44619),
	DT_ENTRY1("gps", "060621/Demo", "5e9b07bdd5272ea6914b40d2adbca77d", 3259),
	DT_ENTRY1("graduation", "050119", "3bcd9dad30635d5b24e0e81360a250c1", 33789),
	DT_ENTRY1("grandjourney", "020301", "d38a8ba3df7e496c53238e2a0c6626a3", 85136),
	DT_ENTRY1("grappling", "011117Demo", "5d1749c0ec8e412c3bd3c753e543118a", 5828),
	DT_ENTRY1("great", "001107", "4cd33d4bbd04d8537f23bc5d278d2211", 108994),
	DT_ENTRY1("greek", "080115", "70a5e7abd3f76acf413bf5247f54f78f", 49664),
	DT_ENTRY1("gross", "050306", "8e05a9e3208a332fa9175a193e5262db", 7131),
	DT_ENTRY1("guess3num", "060515", "3f05bfc34575a8f4ed8b3daf1eb26cd5", 829),
	DT_ENTRY1("gumball", "050209/Demo", "e48b9db2269bee7b0b5da07af52256da", 952),
	DT_ENTRY1("guysguys", "100513", "baaca4f89c5107be250d5894bc440601", 11162),
	DT_ENTRY1("halloweenhijinks", "031026", "14694f7b9cef4baeb0f958500119d2ff", 68052),
	DT_ENTRY1("halloweenhijinks", "031031", "d82508a70cf2a46e81042fb433ea49a5", 68060),
	DT_ENTRY1("hammurabi_rmo", "040131", "1d84182e20c95959d1b9045f116cf548", 2960),
	DT_ENTRY1("handyman", "041015", "f8308a5e6392c20ec161ced78f3de26c", 143698),
	DT_ENTRY1("harem", "041011", "1656dbc8e3bc9c86ace8537141465557", 44993),
	DT_ENTRY1("hcw", "140523", "327dfec3d8f0b63ff5b05a552ee946ca", 82659),
	DT_ENTRY1("hellinahamper", "030428", "0af4c4b3b1bdd38db22a484c72885210", 80183),
	DT_ENTRY1("hellinahamper", "030802", "810247344fddc9a812bf6283b4826093", 80183),
	DT_ENTRY1("heretoday", "011121", "9d3f74bbe250156eafecf9d8b8f43cc2", 4856),
	DT_ENTRY1("heroes", "020920", "26a4eb30b9497ba6dbd57e21309e1901", 5483),
	DT_ENTRY1("hhorror", "020226", "68a77ba6e6fee7ed74cbe0b737446b12", 51820),
	DT_ENTRY1("hhouse", "000627", "c29c27f23dd1ba32e3df8e055bbe4b7c", 42874),
	DT_ENTRY1("hhouse", "030524", "8531a2ad7b8457bab1179c9cc652e9b1", 11132),
	DT_ENTRY1("hiddenassets", "020916", "6697094196c80fc7e8db3115d592778c", 52253),
	DT_ENTRY1("holdbreath", "060127/Demo", "07846768d69e77f17e85e5f44e5cb77a", 454),
	DT_ENTRY1("home12", "030208", "bc82d39d4168fdbf44a8a43737f3f38f", 11010),
	DT_ENTRY1("homealone", "021114", "b8cf48c14ecbca9140cfca8aaa94054c", 47360),
	DT_ENTRY1("hornetsnest", "031102", "4c598fd478a17a5a54228ddb1f2e44b0", 20080),
	DT_ENTRY1("hotelfuego", "060206", "4462d410e2f521cad1f971d95773b95f", 23790),
	DT_ENTRY1("hotown", "040926", "2ae410b1f07b81df507ee7eb7a27eaee", 38135),
	DT_ENTRY1("house_jp", "010506", "ab48826ab779b1d261e2afa0b5a9aebd", 14568),
	DT_ENTRY1("house_mp", "030428", "6b7feeb14682aceace2b7234900aab15", 144023),
	DT_ENTRY1("housedeath", "010210", "d610ddbdbadffb257940c6d34931a743", 18416),
	DT_ENTRY1("housedeath", "020918", "62f7c43f9cb942d10773cdce5fcb1ce6", 6074),
	DT_ENTRY1("houseofthedamned", "000826", "3b25e7d9e1fd6ff2206ab0555c19e899", 35974),
	DT_ENTRY1("houseofthedamned", "020912", "4f0a349840f5eaaf66922fd04a17cc52", 12089),
	DT_ENTRY1("howdidi", "120402", "f3c11c1983d8e75f389c9348da6c132b", 2619),
	DT_ENTRY1("howitstarted", "031218", "54032ee25c7119567833f937722ccf1c", 10528),
	DT_ENTRY1("humbug", "231207", "716cc338bfaab7574cc2618e3b104cda", 73308),
	DT_ENTRY1("hungry", "111017", "671f8460f67dfb722d349d6d3136596e", 5529),
	DT_ENTRY1("huntung", "081030", "1bebe58266fb3da3ec4edf5c895464c6", 49950),
	DT_ENTRY1("hyperbs", "020523", "f2f0e2a778efb96c7d73bb131d7147a7", 132534),
	DT_ENTRY1("hypertag", "010831", "87983ede01d5ecf02a8426dc652fe427", 37967),
	DT_ENTRY1("iachini", "010804", "bf6a3d149d11160aff8f1286e05f232c", 65823),
	DT_ENTRY1("iachini", "020510", "977daa9b7f7ae26234c2a50845b758ef", 19083),
	DT_ENTRY1("igor", "091117", "45693b3ddb21b12e3026cfd0022bc5be", 3228846),
	DT_ENTRY1("igorone", "100306", "de307f99a56fc109f1bafad5a33af524", 1106051),
	DT_ENTRY1("illegal", "0290918", "a4097e0b3c63bb12ffd7ac2f0ab43faa", 202752),
	DT_ENTRY1("imaginings", "040411/Demo", "23bedbba022acd0736ef92305432f617", 3335),
	DT_ENTRY1("infospace", "010906", "376a9471cb92c93db1ed2ff2737ce7b5", 7834),
	DT_ENTRY1("inmemory", "110507", "2d1fc2d0645ecb3bfdc6b2f1d66be5cd", 32093),
	DT_ENTRY1("input_col", "0402??", "7fddb4e9e902ea5b76c9426946e9eb92", 1250),
	DT_ENTRY1("input_age", "0402??", "318ec4281d26a6090b99c443ca28c56b", 1311),
	DT_ENTRY1("insidejob", "010710", "10ba64f64ddb6ded4bf7f2975c95ad51", 20502),
	DT_ENTRY1("interrogate", "020618/Demo", "4c72f4a6191284021188d0df95123c47", 1414),
	DT_ENTRY1("invasionshirts", "020522", "65ac612a92dbd6ea11cc026f9587aa4b", 5335),
	DT_ENTRY1("inverness", "010416", "e462cea35b007d91945dc443fdc9eee8", 45957),
	DT_ENTRY1("invert", "0209182", "bd15b1b68ca064a23c9986d0b5c9a354", 1941),
	DT_ENTRY1("invisible", "020829/Demo", "bb85a3e2d4cf8b70e197591bd5b8aa48", 340),
	DT_ENTRY1("isle", "070808", "cb3bc0c6ddaf702cb4fe288894563eec", 1436),
	DT_ENTRY1("jacarandajim", "11???\?/v1", "1755731d0bb0ce234d6160ec50b2d431", 109571),
	DT_ENTRY1("jacarandajim", "11???\?/v2", "78e0de68825eea88995d7490adb6c062", 79146),
	DT_ENTRY1("janeysdiary", "040213", "70c3a86f4c2143c71e9fc7d36b907430", 33448),
	DT_ENTRY1("jason2", "011021", "c8bcbb6eeb4dfc303bb19d85fb8cd5c4", 21091),
	DT_ENTRY1("jason3", "020207", "0e05e75dd85c60d1b675d175dfc45430", 12923),
	DT_ENTRY1("jason4", "020614", "9d4289031b6c0d86bc97867f2fc0c6b6", 267043),
	DT_ENTRY1("jasonevans", "011013", "fd842c27af8e599f1e8fd6f9c6ed6f24", 43165),
	DT_ENTRY1("jgrim", "Release 1", "f9c1b2064edeed5675a3466baeebdefd", 96713),
	DT_ENTRY1("jinx", "1003?\?/Demo", "2f10da81a1e1d352c3c1175c329bfd0f", 2179),
	DT_ENTRY1("justanotherday_sr", "100730", "2831d06f1faccd5b2b459eda5188dbd6", 2886),
	DT_ENTRY1("keys", "0410?\?/v1", "2faedd1bd484d0c51657bad81d058c75", 40864),
	DT_ENTRY1("keys", "0410?\?/v2", "c4509dd642e67960085c7f9354dc9abe", 40852),
	DT_ENTRY1("kissing", "030415", "f3eccddab5895a4e054fe26e0c2050f9", 78767),
	DT_ENTRY1("labrats", "050810", "8afaeee7d46387a02fc651820e2be44b", 7250),
	DT_ENTRY1("lasthurrah", "070913", "73787e245236731b4e8c9a59a39ce045", 176101),
	DT_ENTRY1("lastknight", "050121", "54ee8d64078e90969aa66992e1f0c685", 1405),
	DT_ENTRY1("lastweek", "091004", "b441d17690be14dcbc78f71f4f6c95dd", 543750),
	DT_ENTRY1("latework", "040303", "ea3daaefc75d05b82fbca9d3929f228f", 21281),
	DT_ENTRY1("lauren", "050620", "684868781e3b3ab6d7a6e4673dcea9e5", 56493),
	DT_ENTRY1("legacyofaprincess", "040124", "9db58796cc772b662743e9a65ae1a876", 37003),
	DT_ENTRY1("legendakhbar", "020101", "06dcbe93ed5878cc3fd40d16afbe2f02", 100600),
	DT_ENTRY1("legends", "020803", "70d80e6a8c5df484bf5e00bc1853c0a0", 66733),
	DT_ENTRY1("lessonsp1", "020923", "e0aa0635423f76bccc451c65ad19e325", 489426),
	DT_ENTRY1("lessonsp2", "020923", "a4df532a6d1f461def5c8cd354b3cb0e", 499993),
	DT_ENTRY1("lessonsp3", "020923", "96a29d48f95dc70451d6f9a47df450bb", 725795),
	DT_ENTRY1("lessonsp4", "020923", "84ddb87ecf69a7d460cd7e8dd4804dd1", 717132),
	DT_ENTRY1("life", "011014", "896b097fdb742cefdfa06c198e90338e", 45737),
	DT_ENTRY1("lifesimulation", "010320", "7d637eea155abec98f9a2cfd4020553f", 35886),
	DT_ENTRY1("lightsgoout", "011227", "497c59d46ed00e100e51df6a458ab6e1", 16919),
	DT_ENTRY1("liqid", "010731", "036a9b14c7fc3eb4b67ef67963ac6fb2", 49095),
	DT_ENTRY1("listenformonsters", "071202/Demo", "5a4e16620d65edca902294d79c07a801", 1376),
	DT_ENTRY1("location", "041010/Demo", "8684c35b213e4fc06e55debb17ab0b1c", 410),
	DT_ENTRY1("lock2", "021107/Demo", "5526da113101cd964a6d92853ba57610", 313),
	DT_ENTRY1("lockedout", "060930", "745360aa69f555882d3737b366b983b4", 22292),
	DT_ENTRY1("longjourneyhome", "010703", "c16c9e02c4a3248b25cb4f6c1ada0646", 59124),
	DT_ENTRY1("lost_em", "010330", "904b80ebf94df890ad7c26e454b6a612", 42596),
	DT_ENTRY1("lostmines", "020603", "08719eb487b62aa0072344ddabdb3ccd", 37088),
	DT_ENTRY1("losttomb", "010220", "3a6cb8ca1c9de79161a65ecb6a40fedd", 56336),
	DT_ENTRY1("loveforreal", "02????", "1d2629fe44ba4b363dd1c47a999c9dc3", 54635),
	DT_ENTRY1("lovingfamily", "040117", "d04532f7e84ce58dbd35b808fe4a1cce", 112064),
	DT_ENTRY1("mages", "010614", "5fd79d845d071cd4a83ff3029185862e", 56943),
	DT_ENTRY1("magicshow", "041211", "29fd03636eec8ffdaea26251a6f2444c", 104201),
	DT_ENTRY1("magicshow", "060923", "14c3abea49429b35f7c9a599e61fba2a", 104422),
	DT_ENTRY1("makeshiftmagician", "060211", "022e92887a488baefc0bd868ed76f22c", 10862),
	DT_ENTRY1("mammoth", "111222", "84037b03ff0dee65e800b64d4229e325", 4290),
	DT_ENTRY1("marmalade", "050607", "89669a0e1c82045b1a2fc0d8a2e5be5b", 15549),
	DT_ENTRY1("mattshouse", "010623", "39c67a455f68d1b01eadd9fe0563bfa0", 33429),
	DT_ENTRY1("mattshouse", "020529", "0116c0272509776b59767f4619af3759", 7127),
	DT_ENTRY1("mcl", "020921", "7cdfb44d05ae773c758fecdb88f4a1e5", 5952),
	DT_ENTRY1("mhpquest_adrift", "020303", "b3fe34b0d01f6486245eb15643417ce0", 17143),
	DT_ENTRY1("microwaveman", "050306", "45adefa528c0da90dea695ca5b4e5373", 1943),
	DT_ENTRY1("midsomar", "040910", "338a87edc27ed60f06f51dba6acb9990", 57277),
	DT_ENTRY1("mikes", "050306", "b19a80fb6e82b571b4efbf5099df63e1", 5552),
	DT_ENTRY1("missingpersona", "071031", "751cedd28b92d205f2f2aa1d2c5d483c", 63928),
	DT_ENTRY1("mm2", "011016", "7a5e1ab956e5526df6a25375e3ecded0", 34133),
	DT_ENTRY1("mm3", "011119", "d212909ac670ea371756f12a243b2df9", 55878),
	DT_ENTRY1("monsterisland", "110101", "8061045e6e083d940ea3c489ef4e39cf", 887821),
	DT_ENTRY1("monsters", "040222", "fde7be359d37c9229fec77244fc38a54", 17168),
	DT_ENTRY1("monstermirror", "010905", "c6dad4827bdeef9c3c7e3edea74e9cbd", 17950),
	DT_ENTRY1("morning", "010213", "4f72aa2af2d53114644694af372d1307", 26973),
	DT_ENTRY1("morpheus", "020918", "e31568ec16ba376229f117c7e9802aae", 13280),
	DT_ENTRY1("mould", "06????", "f9bb352e4b174b20fdfa6813da6eedc5", 124593),
	DT_ENTRY1("mount", "040905", "ba6eeed7e9051959bf84372737fdfa36", 453241),
	DT_ENTRY1("movechars", "050623/Demo", "987ac1bccd75ac2a46d8fed21b15e37d", 289),
	DT_ENTRY1("movingobjects", "040115/Demo", "3a64cce6a686f65d81b0d8ca7d7e9069", 415),
	DT_ENTRY1("mrsmith", "020215", "64de13adc2b22380ef4fcb5817fae229", 28863),
	DT_ENTRY1("msl1", "021115", "1e9e15973eba1413864c0313fac7513a", 4791),
	DT_ENTRY1("mudergreatfalls", "011124", "f8ad4ad63c7ebcaa7b479622c4acb867", 59896),
	DT_ENTRY1("mudmonster", "020418", "5c466401d629e0ffa243ef314a7d8d3f", 2176),
	DT_ENTRY1("murders", "030420", "bcc1499cc7fac4e9ef966a32caa18359", 69489),
	DT_ENTRY1("mws", "081222", "0f7bc694f8e6a97f6c6c27043b7acadf", 49765),
	DT_ENTRY1("mynameis", "050820/Demo", "33c9541d44a06ea25114ee67f1acc64f", 358),
	DT_ENTRY1("mysteryhouse", "070813", "9c7f8577c506737cc07d026d8318598e", 4916),
	DT_ENTRY1("nameme", "061004/Demo", "e0ab63271acb09c013529354ad7b6820", 413),
	DT_ENTRY1("namiki", "040315", "d70ef2d3fc256813c13604b37c2858ce", 22297),
	DT_ENTRY1("newbie", "020419", "5d3419855684f2fe65f9e1ed638ecda5", 3644),
	DT_ENTRY1("newrooms", "041206/Demo", "1fcdac97c2fa02a375f64104c128fbb7", 473),
	DT_ENTRY1("newton", "090513", "c847ab6a44719356f9a25c8519d397a8", 1291),
	DT_ENTRY1("nightmoon", "020301", "b7294103bcea0797d6af870a308100a8", 52330),
	DT_ENTRY1("nightthatdripped", "020925", "0a4fdf3c02c45977d83d334fdbacb8a2", 18637),
	DT_ENTRY1("nodrop", "050830/Demo", "3a98daa4b8301672983d58dadc240fea", 355),
	DT_ENTRY1("nonsensemachine", "010727", "8bd2414033024e1f926c9d017bed2e7a", 3590),
	DT_ENTRY1("noordinarylove", "040219", "847afe7eb12ebf3114f1928d69026245", 29692),
	DT_ENTRY1("normville", "040328", "7280e04fde5617c8b7bfa02c77ee7827", 57226),
	DT_ENTRY1("notebook", "060723/Demo", "055cab41692d04b959d15ef717d68cef", 1861),
	DT_ENTRY1("noticeme", "051128", "8d76571e44b4c44d37e6d2984e931e96", 26977),
	DT_ENTRY1("noximion", "071221", "6702a7a1b01162578d84494a0ce34a2b", 7321),
	DT_ENTRY1("npcgroupwalk", "040601/Demo", "e33a4e6b791390168ae3359745d13157", 334),
	DT_ENTRY1("oakwood", "050214", "4909878732cdfaece760cf84e9e3c323", 12558),
	DT_ENTRY1("obsession", "020925", "ac1a5d34888c978ad67800eee42c8eca", 6523),
	DT_ENTRY1("officebreak", "010923", "c075f4d19abdde30bdcaec6f25e34ebc", 7770),
	DT_ENTRY1("oldmoney", "011025", "fb8fa484a86b9cd7c2e2757b7816c53f", 2346),
	DT_ENTRY1("onnafa", "021108", "da4c0cc6fe34a08047de54abc66de038", 167339),
	DT_ENTRY1("onnafa", "021112", "df5a458010cad788691f362d7bbd77b3", 168018),
	DT_ENTRY1("options", "0508??", "472392dafa9ac6a52d8e0810f395530b", 302728),
	DT_ENTRY1("orcho", "011227", "82e8fd85e5ae7ba83c5614c13b84e226", 93928),
	DT_ENTRY1("orient", "020614", "c30c0319e44395c138bf548164dab197", 12651),
	DT_ENTRY1("outside", "010314", "926b3cd9a7ba18183711e7e61c3d8eb1", 53526),
	DT_ENTRY1("ovaloffice", "040329", "070b9aad43f654fd2f1794a3b78f9eb0", 21350),
	DT_ENTRY1("overtheedge", "060925/v1", "3dbca4d6fb60f6e9945e383f230e205f", 14760),
	DT_ENTRY1("overtheedge", "060925/v2", "4b2c0ff495bd9b2bb3ef46e3300fd11a", 14927),
	DT_ENTRY1("overtheedge", "060925/v3", "a6bb780b231c31a9e01455d0424acdca", 14937),
	DT_ENTRY1("overtheedge", "061022", "9c7bfb9252ac80597ef125c95d58a54b", 14771),
	DT_ENTRY1("overtheedge", "070113/v1", "80f8e3f00c0390b7b0e3e83de5e1fb1c", 14764),
	DT_ENTRY1("overtheedge", "070113/v2", "2e98a67cc74e551975f0953f4c979394", 14906),
	DT_ENTRY1("panic", "020416", "904ef60ba74852dcf17f347e2680a082", 198625),
	DT_ENTRY1("paperrack", "060402/Demo", "c29c3672b67da01bc0e8c949e2cdc42f", 261),
	DT_ENTRY1("paradisehotel", "060327", "48b43dc4b3a984cddaf9661245c6e22f", 106930),
	DT_ENTRY1("passages", "030226", "524ca347d623936586c68536a3b4d0d3", 85728),
	DT_ENTRY1("pathway_adrift", "060521", "1ac0787c4948e540a04ccdb1f8c156f7", 20935),
	DT_ENTRY1("pathway_adrift", "061224", "73cc920942af7d162538ead0d118f34c", 20926),
	DT_ENTRY1("payback", "030501", "5742ef7dffa3fdcb1172432b3b9ed0d8", 33884),
	DT_ENTRY1("phoenixdestiny", "010817", "6a271aae0ed4f9e99e5ea9dc14b402a5", 121059),
	DT_ENTRY1("phonebooth", "020802", "25f4977d7b8df28322df945809826f43", 1372),
	DT_ENTRY1("picture", "041128", "cffa045b7364b531473d2eb69fea2931", 2257),
	DT_ENTRY1("pizzaboy", "090113", "6fd47ddb95c9fa8acc05e2139413ea69", 23919),
	DT_ENTRY1("plainsfantasy", "070511", "212f392b63b48ebc82ccacbacdd9f0cc", 179884),
	DT_ENTRY1("plan69", "030705", "08ccbf41ed663e47511a98810db869ad", 3754342),
	DT_ENTRY1("planescape", "030323", "073f5f284acd9a72c093ff16a71b5fda", 53439),
	DT_ENTRY1("platinum", "020107", "bff449ef3d0f9bda6177903498164dbf", 149350),
	DT_ENTRY1("poker", "021114/Demo", "579224562b53a73a8ea828ac4e962fc0", 583),
	DT_ENTRY1("powerplay", "040519", "6f7531e6665596d691f67eee29aaabc2", 45308),
	DT_ENTRY1("practicepoker", "031110", "8bcd47304778e4eb24221ea59c233bb9", 62236),
	DT_ENTRY1("practiceprocedures", "040114", "3cb015e3a80447719e3e9cdeae2cf1d2", 27626),
	DT_ENTRY1("praxis", "020320", "da2b19b01df82d147f214a737a641a52", 9977),
	DT_ENTRY1("privateteacher", "040108", "720179325e5a1e71a0c2351e4fc3f3fd", 23769),
	DT_ENTRY1("professional", "060707", "0a79bee5792f05febdbac82260ee6313", 51657),
	DT_ENTRY1("professorvonwitt", "020815", "570e7d90fa42a199ee0cba44a3c95d6b", 31253),
	DT_ENTRY1("prostitute", "040318", "da27e6d645e9be23e057fd2fac33d953", 8555),
	DT_ENTRY1("provenance", "051216", "9b121d31282c687fddb705f789e80132", 532153),
	DT_ENTRY1("provenance", "060210", "49ebc8ad4f681eb727c8a2c90c3c6dd7", 532278),
	DT_ENTRY1("ptadance", "031203", "aa30379a53ccbebf49b4cc15f92fc4f1", 35815),
	DT_ENTRY1("rachelbadday", "050308", "8940ad383d78b2ba5d1fa9738bf1ea98", 98539),
	DT_ENTRY1("raiders", "030524", "3ca78ce9c91df25c811103a5cec679cc", 4729),
	DT_ENTRY1("rainm1", "040402", "544333041624fb4b37a11ee1ce142de0", 2703),
	DT_ENTRY1("rainm2", "040402", "283eece3157e4b2548cfcc3073add18d", 3109),
	DT_ENTRY1("rainm3", "040402", "e0230e42074c36b3589375c62ff504d9", 3127),
	DT_ENTRY1("rainm4", "040402", "b7211c1718e62dbb4f4d5e06a4280165", 2759),
	DT_ENTRY1("random", "060612/Demo", "e0f418cfc68a86d0a1746d00006385f2", 423),
	DT_ENTRY1("randomnumber", "070415", "b1dd0667df44632689411d22d0201dbb", 405),
	DT_ENTRY1("randomscattering", "041025/Demo", "2773de334367b223898d9d5f39a8d859", 562),
	DT_ENTRY1("randpuzzle", "040620/Demo", "27670c4c2e87a6c0982f3b65fbb6d8d1", 2766),
	DT_ENTRY1("razorsedge", "020529", "114c459be9cd7b642fed574f8e87840d", 3846),
	DT_ENTRY1("realtime", "011120", "c71d018130b1c5430aabb67519ca4320", 3857),
	DT_ENTRY1("rechargablelantern", "040223/Demo", "ee00b82275ffc2936eab3c800112feac", 3026),
	DT_ENTRY1("relatives", "040323", "7390071c7891f732a294cd441e6f4350", 46620),
	DT_ENTRY1("report", "010926", "a968fc7b37ebbb150ea927e5e5d02ab7", 41801),
	DT_ENTRY1("respawningmonsters", "070829/Demo", "ec64c70c61a6a34075fbf169aba6a866", 1742),
	DT_ENTRY1("restart", "031013/Demo", "6ac51e638473c57061d2506e8684b32d", 385),
	DT_ENTRY1("richard", "010424", "90dbf939d2548f8d61066ad77fe5af25", 55039),
	DT_ENTRY1("ridinghome", "080531", "c1361ae72e0613fa8623a9f2485d3c31", 55650),
	DT_ENTRY1("rndperson", "040811/Demo", "e7ee5e9ee632bf743598e47358d375e0", 839),
	DT_ENTRY1("rockband", "091206", "f01a0ffae6fb39338ca26075bb45644c", 43719),
	DT_ENTRY1("rockyraccoon", "080503", "8d393c6946cff07440aff14f4410c4a5", 5532),
	DT_ENTRY1("rockyraccoon", "080305", "147b6ff138571c8f700c66db572e1a77", 5530),
	DT_ENTRY1("ronforest", "020326", "9cc5999e462e0d53a3d0cf45c5acac70", 8292),
	DT_ENTRY1("rontoxic", "020413", "d12fd2d5fb22c16de740c7556ec5eadc", 51225),
	DT_ENTRY1("ronweasley", "061030", "4505cdc4a7defd9f6569cdc871f8df4a", 334421),
	DT_ENTRY1("roomdesc", "050107/Demo", "5ebaf1afec17a97eb42845f1af6c1aec", 592),
	DT_ENTRY1("roomentrytask", "060115", "49bb4299a979614b3bc185c4b0c5821b", 341),
	DT_ENTRY1("rtcountdown", "050402/Demo", "41b3bf0e121d113da727542a7f888500", 440),
	DT_ENTRY1("salvation", "030328", "a4acb5c5711182f211b3a49b1d8cd08d", 51557),
	DT_ENTRY1("sand", "030127/Demo", "aec1b0b5bf5611facca3bbc93ef2f03d", 579),
	DT_ENTRY1("santababy", "031207", "5ee0ab5df059d56be3090e37b408354d", 48656),
	DT_ENTRY1("sbft", "061229", "e5610b103dc25449d4232c745ac8dd81", 143564),
	DT_ENTRY1("scandal", "0905??", "a53fbcb8e4f1358088fac50b1516fb44", 127212),
	DT_ENTRY1("sceneofthecrime", "020105", "6ebd76c3586165c13707855bd337bf83", 228446),
	DT_ENTRY1("sceneofthecrime", "020817", "af077e5463b69778ac0d076eb50a574f", 67886),
	DT_ENTRY1("sceneofthecrime", "020915", "a8a094b145523c622e65ab6897727f2d", 68054),
	DT_ENTRY1("schoolplan", "040322", "ff30e3cb2052460eb662e232f1c1e188", 39189),
	DT_ENTRY1("score", "040320", "f97e463fe05c17e4fc59606ea7884f1a", 24186),
	DT_ENTRY1("screensavers", "010716", "cc11d1576475aa78366c4c8416f67e3e", 80870),
	DT_ENTRY1("seat", "060728/Demo", "f3aad262fdef15ca48ae80f80a1b79e1", 706),
	DT_ENTRY1("secondchance", "0505??", "3a3c30b7ab715a6dab7114a5cfb89cd2", 139093),
	DT_ENTRY1("secretoflostworld", "01????", "fcadd63ccfa96b38a20f2a850c6ed21b", 49842),
	DT_ENTRY1("seekandenjoy", "110522", "069596f2641e03f6bfcaf3bf8181eaf4", 9573160),
	DT_ENTRY1("selmaswill", "020401", "23b7fb4bf976135b8be2740e1fbfb83b", 19559),
	DT_ENTRY1("selmaswill", "050520", "28f22f3762597793a376d3cdcd6ba2d6", 19599),
	DT_ENTRY1("sentor", "050305", "ddca204b605c37fc1cb5e74c1cb49dbe", 9787),
	DT_ENTRY1("sere", "060610", "aa3360b0f6dc69daf56d3c00534ecb33", 11086),
	DT_ENTRY1("sere", "060626", "e60363aad3c8dc5890ca85b95955b2d4", 2724371),
	DT_ENTRY1("sere", "060627", "ddbdaccee5a532ea9a8e25753a8e14e8", 25956),
	DT_ENTRY1("seriespasswordsg1", "050206", "bc2073a2cfcfd368b86ce387b6e3f531", 1508),
	DT_ENTRY1("seriespasswordsg2", "050206", "5584f5db8593ff2b6573a49fbd7748ac", 1527),
	DT_ENTRY1("severance", "021123", "7674507e99bda35cd9df46c5fd2de82f", 1864),
	DT_ENTRY1("sexismental", "090520", "bc96eca6bf93af14f10f527b5fff7c93", 8373),
	DT_ENTRY1("sexawakening", "980101", "b89cbc47017d2f223502dc3d8a9b1c2a", 30966),
	DT_ENTRY1("shadowpeak", "090724", "79a637f5b7de7e4cbdbf853d5834ab41", 92285),
	DT_ENTRY1("shadowpeak", "090811", "39cdda2d8b303d0b4d292f4983af4591", 92859),
	DT_ENTRY1("shadowpeak", "101204", "63cb42d7928336dacd0025a6bced32dc", 96190),
	DT_ENTRY1("shanilor", "010806", "57516c5b8ebb6d9217c71ffc4491bf8a", 6133),
	DT_ENTRY1("shetland", "020609", "9b194a340f1f1123b1c4b8eb0f1c38e2", 9485),
	DT_ENTRY1("shuffling", "020918", "ec258e7584d43d4a43d8d03fcacccf11", 4419),
	DT_ENTRY1("siblingseduction", "100610", "2bf5b8b66501b254d7fc6a65ae0c42ad", 53313),
	DT_ENTRY1("sierramadre", "080201", "2e2fd88d193aa2d2b6a46933c4f766e4", 4094),
	DT_ENTRY1("sigurd", "041128", "c7f6cf278cfff76afd3f8a0c99623522", 53838),
	DT_ENTRY1("silknoil", "010901", "f3bf16d9004a2b0ada39918ebd07aff5", 71345),
	DT_ENTRY1("silvermaiden", "031008", "b8131e0947e08f864537a60eae2703c7", 11834),
	DT_ENTRY1("silverwolf", "020511", "26696842858a1ddedcdecea21d339821", 96342),
	DT_ENTRY1("sk8sponsorz", "010726", "ed33436d13bf7897636ab4f69ead2441", 8047),
	DT_ENTRY1("sleaze", "020425", "6674fe9167b4be0db99ba0c746342bcc", 29811),
	DT_ENTRY1("sleepover", "040213", "d9a03ef73e65399742ae1e3c34ca380b", 59471),
	DT_ENTRY1("smercenary", "130416", "5978a46dcf84e25897832813650d09e8", 51458),
	DT_ENTRY1("smoke2", "020730", "2961b79f144dba1795474e8cfe6470ca", 16601),
	DT_ENTRY1("sniperscope", "060622/Demo", "680cfcb04a541890fe029a2b34007d78", 1726),
	DT_ENTRY1("sommeril", "040418", "cf74b9be4772079380c0589309e399ba", 29092),
	DT_ENTRY1("sommerilse", "040218", "1870db33af741482bb5d93ab54200e7c", 22839),
	DT_ENTRY1("sororityhouse", "091026", "c548a8afb30b55b898cd18fd9ad33989", 20661),
	DT_ENTRY1("spaceboy", "060201", "b9c350309385706eb7ae30e2e52e1096", 22605),
	DT_ENTRY1("spaceboy2", "071117", "62f9b0e0840ee70cc311108d24cd7c83", 12712),
	DT_ENTRY1("spacerun", "020308", "c00956d8a2ce1bb46c14c8d2f718793c", 19501),
	DT_ENTRY1("spawnmonstershop", "081223", "cc2477d8ee67b6d850bb0586d89b743e", 1827),
	DT_ENTRY1("speaking", "030309/Demo", "216a55fdb8ae20c05382f84bbb6e487a", 1011),
	DT_ENTRY1("spirit", "010710", "1ea2a26f248cd3f1235167c952e55a4d", 37670),
	DT_ENTRY1("spooked2", "06????", "8255dca87623d0641828b3aaa9be2a08", 17320),
	DT_ENTRY1("spotbother", "0512??", "ffc3b255381b0315ee2210a309706508", 113076),
	DT_ENTRY1("ssteacher", "060403", "30d835f8b8c591aa499aa48fd6344ab0", 48080),
	DT_ENTRY1("sswhore", "110516", "311096d988532d1e80050e6c3dc061c6", 332720),
	DT_ENTRY1("stalker", "010819", "8130b5b37d050d6fcbdf1af5c56c39d0", 7934),
	DT_ENTRY1("standing", "050727/Demo", "87918bb50bc4dea047b443fd8d90a94a", 564),
	DT_ENTRY1("stardust", "061002", "caa5cdc48e4f23bad3a6893b5fd2b2a4", 35343),
	DT_ENTRY1("stardust", "061012", "0d0a2aa60004be9b02b1a212b3cd582e", 42463),
	DT_ENTRY1("station", "020510", "efdac6169c91e2b70fa1b63f0abdb237", 10823),
	DT_ENTRY1("stodw4", "060419", "bb6e78687fae941c7755e9d00c5d91b9", 26532),
	DT_ENTRY1("stopfollowing", "060716/Demo", "cb2721d383c930a8cc298772d65bf7f2", 499),
	DT_ENTRY1("storetest", "071006/Demo", "1615e89ef2e5b4edd90ddc2aabff74cb", 1010),
	DT_ENTRY1("stowaway", "040723", "6877c3079978ecf74e72b792005fca32", 14231),
	DT_ENTRY1("strange", "010331", "e298be9c1b55c9866c7355ecbac74534", 8912),
	DT_ENTRY1("studio", "030601", "fbf60f04bd2d9c485545d3478a8a3d0b", 118676),
	DT_ENTRY1("superliam", "010107", "835df44f633a5526656c6cfbbbabd72b", 28799),
	DT_ENTRY1("superstud", "060616", "69e6e1b85a2cbd6ac96a3f9aee7e3588", 327258),
	DT_ENTRY1("sword", "030210/Demo", "8ee854268fe8d0f1db821b6a33d1f4e3", 640),
	DT_ENTRY1("swordincane", "040715/Demo", "5fae53e155140692405dce9833450d20", 447),
	DT_ENTRY1("talking", "101113", "20cb9b81381a297c19e25a98d41921fb", 2804),
	DT_ENTRY1("target", "050828", "c6bbbae5ea3f6407a55435cb4e0c20c3", 43024),
	DT_ENTRY1("target", "061224", "47202ea8385188b3045c91bb55a72561", 43014),
	DT_ENTRY1("teleport", "050811/Demo", "86152d768f524777d0d091f97bd4a04e", 519),
	DT_ENTRY1("tempest7", "020715", "04a7d73ca191374cb112833ff5d705bf", 31369),
	DT_ENTRY1("templeofthesun", "031221", "8a18a97d0f46659d0436d891e7e0c8af", 18973),
	DT_ENTRY1("temporfell", "031105/Demo", "927bfa7b6c75489d39be1c40bfd256a7", 70045),
	DT_ENTRY1("testingstatus", "050919/Demo", "202dbce394a3cfacf309a47d536e67ac", 221),
	DT_ENTRY1("texevil", "011227", "1cc1157f409a435847074342adbef27f", 20652),
	DT_ENTRY1("annihilofthink3", "050510", "d277ea444f279f6f6fe25c4686c14bfd", 11401),
	DT_ENTRY1("thecrimescene", "010424", "dbf0dbcb8fb2392c899aba46b690b32f", 10468),
	DT_ENTRY1("thedarkriver", "081112", "bde0f0f135bcf33212c9d19d7ea2f645", 3547),
	DT_ENTRY1("thedarkriver", "100928", "63788f315bc11fe476eb2f53b5c57258", 4158),
	DT_ENTRY1("theclock_sm", "021122", "90ba998e679618644ae747d798f6a832", 881),
	DT_ENTRY1("thecrash", "050221", "6fcf1713c0b5bb8472f842f1805daf7a", 4509),
	DT_ENTRY1("thehole", "060908", "9d7449170ff3d6d8139a641d47e307ca", 5198),
	DT_ENTRY1("thelabexperiment", "020918", "6218d1a6ba705799bb863e9136a139c2", 3009),
	DT_ENTRY1("thelabyrinth", "030824", "a984fbc7d9a6d592315f17406981b04d", 19468),
	DT_ENTRY1("thelabyrinthsystem", "020118", "e6ceb574b2499d0629da789d53ab5d60", 38100),
	DT_ENTRY1("thelasthour", "040220", "786d39eb223a64fe836f87b032b60ae1", 10658),
	DT_ENTRY1("therapy", "060119", "1075d80c1420b169e1ca5d1a5bae1953", 58597),
	DT_ENTRY1("thesearch", "021118", "f7027e0d01c78325cf98951474ee44f5", 31356),
	DT_ENTRY1("thesorcerer", "010726", "189e4135879d163cb2f4c55890d5bc08", 22316),
	DT_ENTRY1("thesorcerer", "021018", "aea6313f54cd9a56ac90f8a425db6245", 6456),
	DT_ENTRY1("thesuccessor", "070225", "4c588168479f101a574610609ce192d6", 32966),
	DT_ENTRY1("thetimemachine", "041016", "c564fdef87a7cdc7a56bd123b9faacde", 10540),
	DT_ENTRY1("thevirtualhuman", "0807??", "b894e19638a65e03d28b696a761d23a3", 6168),
	DT_ENTRY1("thewill", "020117", "289f8ee71bfefe6469913cf8058bde0f", 132602),
	DT_ENTRY1("tictactoe", "091102", "cc48b5fa6ccc64dd0fbe164a85bd44fc", 1790391),
	DT_ENTRY1("time", "030322", "1810c3d8848f5eb628260a5110410a06", 16585),
	DT_ENTRY1("timeadventure", "020416", "34c978820d4bdd7d612be1c2257898dc", 12353),
	DT_ENTRY1("timesystem", "010919", "7f878438548731d90591aaee6154a59c", 3436),
	DT_ENTRY1("timething", "020609", "9241e0716aad5aed40e739abc1925639", 338),
	DT_ENTRY1("tmm", "060920", "b4730e7fc98802ade0ae85b104588016", 2573),
	DT_ENTRY1("tmn", "090519/Intro", "70f502d2f8a605909afa855e28440f05", 10801),
	DT_ENTRY1("tobeking", "030123", "010463626e39533ef7a93a2a3b634551", 103424),
	DT_ENTRY1("togetyou", "04???\?/v1", "cb0203f31bde1acf05312015363105d6", 4527),
	DT_ENTRY1("tohell", "020714", "43eb5899b3301dfc60fdd1c84a9e46a7", 18080),
	DT_ENTRY1("tomdickharry", "101021", "4be1f57ca11511034f32a0ea2c54fa86", 1318),
	DT_ENTRY1("toolkitdice", "010905", "e76c34e0ed932b944a5545addf20041b", 819),
	DT_ENTRY1("toolkitcalc", "010909", "17db8c8cf86332ba19badeceb8ce622f", 1818),
	DT_ENTRY1("toolkitgame", "010905", "bf74230b1f7839e0a93954e5b7529644", 3365),
	DT_ENTRY1("tophat", "090923", "4303ad8ce8d338ac3598d64940a2d546", 12959),
	DT_ENTRY1("toronto", "010505", "9371f62ed34f3eb382b0b97b5f9a676f", 9420),
	DT_ENTRY1("tq3", "010107", "8dff23d59f9dc9560ae3c82b53d2adb6", 22573),
	DT_ENTRY1("trackingdevice2", "041011/Demo", "a330535c4b5024a3ad49b48a8011c533", 441),
	DT_ENTRY1("tree", "020617/Demo", "d9310c133da963b1162579b70cd7d49d", 3778),
	DT_ENTRY1("troll", "020215", "5a9d7d4fea2f12bc010acadeff9dee01", 68742),
	DT_ENTRY1("trouserpress", "080203/Demo", "a28dfdee50b5af4b9c26a1d0f3c4ad8c", 508),
	DT_ENTRY1("truck", "040322", "48f1420a2e05ea20d0d5ad02a156a8e5", 9872),
	DT_ENTRY1("turnberry", "041015", "696883cf2873bc42e04500268f2e19a6", 438888),
	DT_ENTRY1("twentyone", "091027", "a08597706f81b58c331940c1aa7be126", 92778),
	DT_ENTRY1("twilight", "001104", "ab5ddd461c1fb2065d26fcfdf4baa5aa", 71377),
	DT_ENTRY1("twilight", "001220", "9824de022e7959fb9f974f01964b772c", 71621),
	DT_ENTRY1("txfbeg", "021218", "e874a0b37474d84d9349a08c285afc6a", 101727),
	DT_ENTRY1("txfbeg", "030525", "5e7a485ffa177482669986350680285e", 35739),
	DT_ENTRY1("unexpected", "030312", "7a46406c963d4309c775ecf8e0e0466c", 31902),
	DT_ENTRY1("untitled1", "021030/Demo", "0b3eadc3f32d95e335ed9d409b36b089", 1086),
	DT_ENTRY1("untitled2", "040110/Demo", "12bf1cfe67acce801a3ff1e6ce4e3b54", 1620),
	DT_ENTRY1("upgirls", "080128", "cbb2a224b019220bf4a22415a2acf5a9", 3538),
	DT_ENTRY1("urbandragon", "011224", "26a6abb965fe12ee2837b9f1a0d2fa47", 30158),
	DT_ENTRY1("vague", "090328", "0f5ef8616c668b9c3de078a77b26a512", 122681),
	DT_ENTRY1("vampireconscience", "010323", "1da43163193776d43f4e2a2918b1551b", 63183),
	DT_ENTRY1("vandk", "010801", "ec6de9a7062f0445bb09a2c9caf63d6d", 23938),
	DT_ENTRY1("vengance", "060218", "e26f76db7b64cf371c5583bed1ff3cce", 27974),
	DT_ENTRY1("veteranknowledge", "050212", "c2b2b3d07b399f7ef1daf7453de8d704", 197649),
	DT_ENTRY1("veteranknowledge", "050506", "409d3d59cb767dc8b812e16db78b2e47", 52248),
	DT_ENTRY1("veteranknowledge", "051211", "1a1c58aef367f40c8539e74bd3c990ff", 52290),
	DT_ENTRY1("viewhome", "090420", "ac6d123e7133c8fb2dd9f60474bd9d61", 9237),
	DT_ENTRY1("viewisbetter", "060928", "9334adc4ccd03ceb1684ee56fdb07ab9", 7720),
	DT_ENTRY1("villagell", "051222", "40804d966a80a662bbf4d9c0a9baa3eb", 200650),
	DT_ENTRY1("virgin", "030912", "34c22836728c7227cc4d2e86f08a6a02", 66260),
	DT_ENTRY1("volant", "070401", "ed8e93c25a77e4f331799dc4647e1e54", 64718),
	DT_ENTRY1("warlock", "060328", "96c1c5a7fad80e344d70b6a03646a363", 43722),
	DT_ENTRY1("warlord", "060316", "78e822290745dd8b42590da7436c8b04", 199403),
	DT_ENTRY1("warlord", "060427", "5694fe2acc420bf276e22c48e9ef7003", 217096),
	DT_ENTRY1("waspconnection", "021004", "df289810b7dded4aa3f0a6df7f0414fa", 2310),
	DT_ENTRY1("wasteland", "050826", "2587f2f2a72cc0d4bb43b09f66ada361", 5383),
	DT_ENTRY1("weathertime", "040730/Demo", "e812b3135627bc2af4ff0b28a85ef939", 2144),
	DT_ENTRY1("weirdstuff", "030917", "bbc31a2675b31a800277e1e6f4a5797d", 27589),
	DT_ENTRY1("whatever", "020510/v1","3e0f5ca9f0acff5ac8c4f60f0c60e712", 110051),
	DT_ENTRY1("whatever", "020510/v2","c7cf9bf6eba661ee4d089b648ac5d307", 110122),
	DT_ENTRY1("whereami", "090910", "6d8c799069d790b38b20f029ba21c119", 3470),
	DT_ENTRY1("whitesingularity", "050725", "9fe8537aecaf47059e6d2e4073fed06c", 788034),
	DT_ENTRY1("without", "0803?\?/v1", "2310f8d0ae20b048e28ea703a4819097", 117307),
	DT_ENTRY1("without", "0803?\?/v2", "e660b79c7fde567ff7515e70654fa9f7", 117288),
	DT_ENTRY1("wizardplayground", "050209", "c75d4ccce821284710dd757abe86b78a", 18585),
	DT_ENTRY1("welcwonderland", "030524", "845497c0a1617f724382e4f85f14dedd", 7381),
	DT_ENTRY1("trackingdevice", "040620/Demo", "22866fbcae738e66d212ccd91069089a", 895),
	DT_ENTRY1("wingman", "110517", "1206d8452c3c3f48f027c631c86405d8", 33734),
	DT_ENTRY1("world", "0708??", "155da95d704d262db3de6afd95d90023", 23152),
	DT_ENTRY1("xclue", "020412", "e075af06e9fc7e07a9a9739bf9c22503", 88758),
	DT_ENTRY1("xme", "060202/Demo", "8b68328f70fdeb13d3d66e92915e6407", 1499),
	DT_ENTRY1("xmen", "030522", "ab25e8523f0ef54c31ce23cb9769eb93", 102540),
	DT_ENTRY1("xmen1d", "050731", "1b102bb8c939195ffba2093bc3091d1f", 2766567),
	DT_ENTRY1("xycanthus", "011114", "5637cb8221087c5cca094b13138655c3", 116322),
	DT_ENTRY1("xycanthus", "021001", "5637cb8221087c5cca094b13138655c3", 116769),
	DT_ENTRY1("yadfa", "020426", "d64d8bde594d6b5d306d2dbcc2e0d7d6", 77658),
	DT_ENTRY1("yadfa", "021011", "5ee5906fc4b889d126cdfd83bd883a43", 77880),
	DT_ENTRY1("yeh", "010728", "ca9ec86147f95d5498e9a111f15f6865", 22322),
	DT_ENTRY1("zara", "040929", "723dddb75ed482cd497f9cfc85431052", 84268),
	DT_ENTRY1("ziva", "100207", "34d94d4ddebd72dfe0beb532171a2dbe", 5329),
	DT_ENTRY1("zombiewalk", "040812/Demo", "634b892a9ae2ccb944278a40903cd7fd", 583),
	DT_ENTRY1("zomelec", "010731", "a1aad86dbdaf90688e36a5b0abdbbf35", 15320),

#ifndef RELEASE_BUILD
	// Unsupported English ADRIFT 4 games
	DT_ENTRY1("bloodrelatives", "01????", "d9384fe44f98113e5df8fdb4aba1d2a2", 81356),
	DT_ENTRY1("nem", "05????", "01040757a6078cf1b9fa2ab80cb06a10", 21424),
	DT_ENTRY1("tra", "00????", "675a2d9b7428e2c19a50d2fd03393b76", 275724),

	// Unsupported English ADRIFT 5 games
	DT_ENTRY1("4rooms", "1402??", "db1616914cd81115ab9eed737384b7b1", 27928),
	DT_ENTRY1("4wallstoilet", "171124", "38259fa302905d70ec345d6ed6495747", 15178),
	DT_ENTRY1("achtung", "2202??", "f035e68e00477056402f69fd4049471c", 120732),
	DT_ENTRY1("advbackyard", "1502??", "73b1171283ffa2b3cf6fb72a0ec31a35", 48990),
	DT_ENTRY1("aliendiver", "2005??", "7a746245878dc3a5edafcc851562aa3a", 77266),
	DT_ENTRY1("aliendiver", "2010??", "6fdb9210b4c42d2d496a63bd04ee14e2", 87904),
	DT_ENTRY1("allthroughthenight", "1401??", "89dcb84d74f70945780ed9e719401301", 36345976),
	DT_ENTRY1("axeofkolt", "12???\?/v1", "e2cb7f32478a69b647bc90e0de158ac9", 901650),
	DT_ENTRY1("axeofkolt", "12???\?/v2", "c9555483f6706d60bd6b8ea18f7b92ae", 906106),
	DT_ENTRY1("badlands", "160418/Demo", "5d654bb182e0337937e792af8caf8d5d", 20248),
	DT_ENTRY1("beginnerscave", "18????", "44e32fc05411bfa1468e93e1e6e325ee", 30358),
	DT_ENTRY1("birthofphoenix", "1112??", "de798149fc307dbcfb5ccf173f95f24a", 23532),
	DT_ENTRY1("bookbuilding", "130922/Demo", "84386cfe7bee273b289d6e3b69aae114", 15628),
	DT_ENTRY1("canyouguess", "189292", "0b305b9014dc44066ec95a816b4fdf4b", 16254),
	DT_ENTRY1("cccs2808", "140828", "d779608ca4dd3aab27bac53e7f671b23", 20086),
	DT_ENTRY1("charloop", "171114/Demo", "e2523c216117855c3ea5b68eb4ad0829", 16824),
	DT_ENTRY1("combilib", "211026", "3ecc566574e9c1f551e6885c6ef94bbd", 46524),
	DT_ENTRY1("darkhour", "17????", "75bc2ac4d561ad2a2143bd90a19d1b46", 35184),
	DT_ENTRY1("ddf", "2109?\?/v1", "58e46de405a7d6b7cc9c924416c8318c", 240970),
	DT_ENTRY1("ddf", "2109?\?/v2", "4c41ebaa7578c72ca38f50fd756c6660", 242106),
	DT_ENTRY1("dementophobia", "1602?\?/Demo", "3f283f808ac6b9baa92e95e9edf83529", 6696738),
	DT_ENTRY1("digitalroots", "2302??", "b30ec06838f688fc98b899c28f0b7025", 727156),
	DT_ENTRY1("dreamspun", "200421", "490ff0065ff10268fd53ecd50082b94d", 29140),
	DT_ENTRY1("edithscats", "161029", "0c6bbf3d292a427182458bd5332f2348", 18316),
	DT_ENTRY1("escapehouse2", "140904", "d62bd657046d87cc15eb285dc87d3046", 18244),
	DT_ENTRY1("evilqueen", "17????", "9b74d1a1e66b841069619382a256d34e", 7909064),
	DT_ENTRY1("expendableitems", "131002/Demo", "02ef3452a42b66ae6e86b402bc8837c4", 16429),
	DT_ENTRY1("fortress", "0011?1?/v1", "d540bc6077de03e6d98aa99a26fca4e4", 1409426),
	DT_ENTRY1("fortress", "0011?1?/v2", "45a24ff9ce0f22fb606b1ad0d56250b2", 1332618),
	DT_ENTRY1("grandma", "22???\?/v1", "cb06f302e135cc907df429660cf11e22", 132016),
	DT_ENTRY1("grandma", "22???\?/v2", "cf1cf12c32b8adef27bfab78354304d8", 133460),
	DT_ENTRY1("grandma", "22???\?/v3", "a348dce214ebd982b290c9acbf7eb86c", 132210),
	DT_ENTRY1("grandma", "22???\?/v4", "62a10746a7d547422e8034b6d6e8e006", 106461),
	DT_ENTRY1("grandpa", "21???\?/v1", "999e88229f0cfb59cce87de0e8af6f59", 9372808),
	DT_ENTRY1("grandpa", "21???\?/v2", "a213dabff7f75307ffad0a053c119746", 9372916),
	DT_ENTRY1("grandpa", "21???\?/v3", "ce027321ed2d8ecd28a17e7dfea89ac8", 9374112),
	DT_ENTRY1("grandpa", "21???\?/v4", "1ab4f92e67d6374e691bef31338188ea", 9360542),
	DT_ENTRY1("grandpa", "21???\?/v5", "816f17ff20bbd973641f17c22162c94e", 64633),
	DT_ENTRY1("hintsl", "220717/Demo", "1084481cfdb745b72497def3f42d4e7b", 58005),
	DT_ENTRY1("inyourhome", "160327/Demo", "0040d880903b8a7fe6795e8680a7bbbf", 18280),
	DT_ENTRY1("isummon", "200420", "9bdc97bb90db1bcd6cfdbfcb26bb59ce", 58467),
	DT_ENTRY1("jabberwocky", "1101?\?/v1", "f2b9a4ed9f19bffb735bbe7c4c25751a", 48674),
	DT_ENTRY1("jabberwocky", "1101?\?/v2", "aaa98f426d20c156d427faf95424becb", 48574),
	DT_ENTRY1("jacd", "1110??", "28ae6ee45f1d2980765d3e9645f0b269", 54256),
	DT_ENTRY1("layers", "140713", "aea348f66c8f2e2fba08d7a15451f33f", 17787),
	DT_ENTRY1("loopthrough", "200415/Demo", "44d20d05134108ab5143aab2a13f8fd6", 24056),
	DT_ENTRY1("ml256", "120531", "68862c0031f1497e32ce26e0654bb07f", 32397),
	DT_ENTRY1("monsterage", "171219", "00b8e89e5e7421a0ea017707466efe17", 16486),
	DT_ENTRY1("museumheist", "2005?\?/v1", "3c10249b9d1580ec63ac63ddddb5342d", 2709918),
	DT_ENTRY1("museumheist", "2005?\?/v2", "c3ea7b30f4ad6736d1f344a95b3343c2", 2711656),
	DT_ENTRY1("nastyescape", "12022", "24b95325d0602ad7178926a0e0125892", 18008),
	DT_ENTRY1("nycholiday", "??????", "616722d93cff001d234a6e2c2c284b73", 19514),
	DT_ENTRY1("oct31", "22???\?/v1", "7ca7980f7adf3da370464610b702e94b", 356192),
	DT_ENTRY1("oct31", "22???\?/v2", "a1da95b23480fe69e7a2975a570326b4", 356130),
	DT_ENTRY1("oct31", "22???\?/v3", "9d74f378405b783d8ca041be70dc25b7", 356434),
	DT_ENTRY1("os", "140529", "dbdf2e91125a057eab5fa044023c28e3", 22244),
	DT_ENTRY1("pac", "140529", "ae7b444af20c48d01ca29175640d1065", 50819886),
	DT_ENTRY1("penrhyn", "2004??", "114335d0be1efac51fba6030fd00ebf2", 2765872),
	DT_ENTRY1("projectactually", "??????", "178324d32fb2bf27e2c14bf84190e9a5", 25559),
	DT_ENTRY1("rbr", "17???\?/v1", "88d7babfbc6122dc70e2673dabe93a47", 335894),
	DT_ENTRY1("rbr", "17???\?/v2", "c095893f61c9f1d8e1d047e148eb4894", 319278),
	DT_ENTRY1("royalpuzzle", "1703?\?/v1", "5ecced3471bbc0cff671238707d1e980", 41780),
	DT_ENTRY1("royalpuzzle", "1703?\?/v2", "fe9be5e874d51dd8dd7bb272f8d99ed7", 41708),
	DT_ENTRY1("salvage", "19????", "2493cfdd477d64dcd6fef8b9e9a603cb", 1302300),
	DT_ENTRY1("savethekitten", "??????", "3ce730972fc402d46c46b38546599c5f", 19957),
	DT_ENTRY1("schoolproject", "130607", "01aca4dc431d0661adcdef1f1b87bf2b", 19822),
	DT_ENTRY1("seymourstuidquest", "11????", "61f4e155b958c1c652f12ed4216ee10d", 12623),
	DT_ENTRY1("snowdrift", "1707??", "27409a216a03756bb13ca0984922e219", 25254),
	DT_ENTRY1("soc", "1210?\?/v1", "16b43395e647605562c04e5e8954d907", 474850),
	DT_ENTRY1("soc", "1210?\?/v2", "7b3e9e45edc75202b3f586d0ca8dddde", 474380),
	DT_ENTRY1("socc", "1511?\?/v1", "7fd4574ae646b8e052eb6513a1e7ece5", 513520),
	DT_ENTRY1("socc", "1511?\?/v2", "3cbcffb47f0a81ac15201276d9f83d2a", 576250),
	DT_ENTRY1("sophia", "21????", "53ff34729255845adad73eb8d133ef7c", 1242046),
	DT_ENTRY1("sorryforyourloss", "2208??", "3b061665f0d67150681c539229b56f2d", 41566),
	DT_ENTRY1("symphonica", "1510??", "62e3685e0e28a8bd1030fc66adb12041", 13685722),
	DT_ENTRY1("tee", "2207?\?/v1", "4f94cf6cfbbd0e141a14ad3c47341226", 359362),
	DT_ENTRY1("tee", "2207?\?/v2", "402bf552411f5da434c97e6ccf06e3de", 359550),
	DT_ENTRY1("television", "110411/Demo", "ad3db06d606092c734eb4f74953b9bb7", 9704),
	DT_ENTRY1("tempusfugit", "171119", "20e6c61ef3a98f4be78faebd15bb434e", 52386),
	DT_ENTRY1("theawakeners", "150225", "9323fdd3734fff99fa49469b3e04d933", 17117),
	DT_ENTRY1("tingalan", "1709??", "08c9ecd86664b62335d3852a2bb3d201", 259969026),
	DT_ENTRY1("tributereturntocos", "1910?\?/v1", "6132158b3cbc58cf38f2290c11b6768e", 99728),
	DT_ENTRY1("tributereturntocos", "1910?\?/v2", "2a5327a3ef791c7b9c8ac3d8c03e1925", 99780),
	DT_ENTRY1("wumpushunt", "230320", "48cfecc5c37bcb766c2657a8b0c98f71", 20608),
	DT_ENTRY1("wwiiee", "2201??", "f16636dee08e361339c724ac6d9b2611", 102152),
	DT_ENTRY1("wwiiee", "2201?\?/Demo", "16c2c030091cd02f1dc4babd85e49933", 78306),

	// ADRIFT 5 Intro Comp 2012
	DT_ENTRY1("5ic_blankwall", "1209?\?/Intro", "7b2490ab9015632eb546185c936b53d0", 44799),
	DT_ENTRY1("5ic_organic", "1209?\?/Intro", "7d3cce3b62f3d7773c8a567e45d5fada", 22738),
	DT_ENTRY1("5ic_shatteredmem", "1209?\?/Intro", "f37a962adfe504a4e1ff3badd4e26745", 33266),
	DT_ENTRY1("5ic_trapped_adrift", "1209?\?/Intro", "d80b609f3d25caa6e66fbb887bb387a1", 14495),
	DT_ENTRY1("5ic_headcase", "1209?\?/Intro", "81cdf1d83cd6f88aafb8ac4d77585fb7", 45980),
	DT_ENTRY1("5ic_axeofkolt", "1209?\?/Intro", "85bfd8637e8758c36ffd0659448f8087", 71534),

	// ADRIFT End of the Year 2016 Comp
	DT_ENTRY1("ey16_tlc", "1611??", "38480cff2f832d413812cb3d30310542", 299704),
	DT_ENTRY1("ey16_thewayhome", "1606?\?/v1", "98293a7c5fcd4754b90cf53f1c6242ae", 69660),
	DT_ENTRY1("ey16_thewayhome", "1606?\?/v2", "7331c940d269a8186789c6f4216674ba", 83478),
	DT_ENTRY1("ey16_dff", "160329/v1", "42991208d92645d9ddd07dd8be82d1dc", 565482),
	DT_ENTRY1("ey16_dff", "160329/v2", "8e107f2a3de2686ebac22e2c9b3752cc", 521536),
	DT_ENTRY1("ey16_heritage", "1605??", "01f8f81bf56bfdc81acd0ecc83671599", 27906),
	DT_ENTRY1("ey16_spacedetect1", "1610??", "170517dae3a195f37d876f5645637953", 34044),
	DT_ENTRY1("ey16_spacedetect2", "1610??", "1e12b19d8777173cbf9866b3dd2fc2f0", 32632),
	DT_ENTRY1("ey16_spacedetect3", "1610??", "fc9fb75830d7f59bf932896152b1e759", 30560),
	DT_ENTRY1("ey16_spacedetect4", "1610??", "86fbcf204c6b0982ef2394a60bb8df3e", 32184),
	DT_ENTRY1("ey16_spacedetect5", "1610??", "f1bf863cce436a5ef17e64237803571a", 32676),
	DT_ENTRY1("ey16_spacedetect6", "1610??", "6035c865f7a4831e4539b7a2ff2ab711", 31922),
	DT_ENTRY1("ey16_spacedetect7", "1610??", "2044086edd9b2288ff2279b3824dfcef", 30066),
	DT_ENTRY1("ey16_thedayprogram", "160731", "a2bceb91de7de4751056522d3cfd43a6", 18120),
	DT_ENTRY1("ey16_thedayprogram", "160805", "c07e41d0cb219089af4bf846d8bc2eab", 24664),
	DT_ENTRY1("ey16_dragondiamond", "160329", "5ced50484c4bc3806702084c589e3ea1", 41670),
	DT_ENTRY1("ey16_dragondiamond", "220315", "4c92c4f0c578c9d6d309d3169ae8e5b6", 54783),
	DT_ENTRY1("ey16_noblecrook1", "1610??", "334c706261ab79b783403ad3315980fd", 23966),
	DT_ENTRY1("ey16_noblecrook2", "1610??", "9d460ed691ad1b45d5d7c19f1ebf61d9", 22446),
	DT_ENTRY1("ey16_noblecrook3", "1610??", "c9d4a1ba7d5c1625f2d4cad0f067275a", 22088),
	DT_ENTRY1("ey16_noblecrook4", "1610??", "cb26241f8dba982bb0cd357ebd8fac45", 20606),

	// ADRIFT Game of the Year 2019 Comp
	DT_ENTRY1("ey19_skybreak", "1910?\?/v1", "5d2f34b91ff26546e7b1da18914d649c", 1132198),
	DT_ENTRY1("ey19_skybreak", "1910?\?/v2", "a5322754939d41aaf46c69c0a1100df8", 1191014),
	DT_ENTRY1("ey19_skybreak", "1910?\?/v3", "321ef94722e2ce0a5caebd55aee19c3b", 1390449),
	DT_ENTRY1("ey19_skybreak", "1910?\?/v4", "4e652cfbd76f164d5a90840d39d48b85", 1655947),
	DT_ENTRY1("ey19_tcots", "1910?\?/v1", "04f8330b06466a5eb96cd3ed6a914224", 231358),
	DT_ENTRY1("ey19_tcots", "1910?\?/v2", "06a4402f1757b17e4c04b923c177d449", 312596),
	// DT_ENTRY1("ey19_treasurehunt", "1910??", "f3c35c9b3f4051897d7c891e8e33e779", 25371192),
	DT_ENTRY1("ey19_lazaitch", "1912??", "055df9090ae88991a66a8be045534e65", 298502),
	DT_ENTRY1("ey19_starshipquest", "1910?\?/v1", "424984a2c89d4b404d180cf274d539a3", 454822),
	DT_ENTRY1("ey19_starshipquest", "1910?\?/v2", "02fb2460c61c8d82c31fa1d763e63a7e", 449736),
	DT_ENTRY1("ey19_magneticmoon", "19???\?/v1", "b6305fd896beeb933045e121b3261c4b", 492690),
	DT_ENTRY1("ey19_magneticmoon", "19???\?/v2", "b435f18684dce693fd64b10ae0df0021", 492630),
	DT_ENTRY1("ey19_rsp", "1911?\?/v1", "ce9d950582fc63a90599e158498a9691", 244356),
	DT_ENTRY1("ey19_rsp", "1911?\?/v2", "4c7a5b57bff1cce91d2bfc63a6e0f18e", 244810),
	DT_ENTRY1("ey19_deadwinter", "1903??", "331789f8590a39c811ea4b5f1b1f5a96", 86702),
	DT_ENTRY1("ey19_bethere", "191027", "2fd4326a88602c00e6215e167f9b9311", 36384),

	// Anyone in the mood for a comp?
	DT_ENTRY1("moo_algernon", "1502??", "7dcf06245873bb65bf56bfca08086ff0", 31280),
	DT_ENTRY1("moo_beagle", "1502?\?/v1", "6d5235a68aba891442a9da5ffb4f96a3", 25368),
	DT_ENTRY1("moo_beagle", "1502?\?/v2", "004db9d8bd9aca27d72387913b79eb5c", 25352),

	// Beginner's Game Comp
	DT_ENTRY1("bg_ttp", "1708??", "059940072c256539fb4ae20340ce6cc2", 72902),
	DT_ENTRY1("bg_lastexpedition", "170806", "4c65d3cf06d24134268e1d8015d722b9", 68762),

	// Ectocomp 2011
	DT_ENTRY1("ec11_deathshack", "1110??", "b7073278da4b3980b7ccc01a4f865f82", 17858),
	DT_ENTRY1("ec11_ignisfatuus", "1110??", "0414d55fc3984c158804575044809c48", 20575),
	DT_ENTRY1("ec11_thehouse", "1110??", "338d24e1cabc2c398927599dc89dd986", 21925),
	DT_ENTRY1("ec11_stuckpiggy","1110??", "bf6d5cd1b5a3865ff610110459d3104f", 18506),

	// Ectocomp 2012
	DT_ENTRY1("ec12_beythilda", "1210??", "527c5afe73b34f297d3d6dcfe7039aab", 18349),
	DT_ENTRY1("ec12_ecod3d", "121031", "b727a3df5a8cc75ce98899d01c479b9e", 19904),

	// IFComp 2011
	DT_ENTRY1("if11_rtc", "1110??", "58729d1f54ca2311d7ff584f223c6396", 149000),

	// IFComp 2017
	DT_ENTRY1("if17_lmk", "1710?\?/v1", "6b5ade214129491afcebc0e280eb22da", 10600962),
	DT_ENTRY1("if17_lmk", "1710?\?/v2", "c46e80c58bf0e343c1d0b8fb6be90f40", 10601042),
	DT_ENTRY1("if17_lmk", "1710?\?/v3", "a2907b5e4c64937222282fc564d9f7d3", 10600484),
	DT_ENTRY1("if17_temperamentum", "1710??", "c23c52ca0c2652edf94f3c6ea24c48f5", 17986940),

	// IFComp 2018
	DT_ENTRY1("if18_6silvbullets", "1810??", "e0f6170b5cbde7005609e68d8a25ddd3", 376934),
	DT_ENTRY1("if18_stoneofwisdom", "1810?\?/v1", "f414f8b8aa9bfaf3256a1d2b6c887921", 80444),
	DT_ENTRY1("if18_stoneofwisdom", "1810?\?/v2", "c9014e44b756c2e795179eacddb83805", 93416),
	DT_ENTRY1("if18_stoneofwisdom", "1810?\?/v3", "3213343c8468514cd644e3b7f843b8ec", 92122),
	DT_ENTRY1("if18_stoneofwisdom", "1810?\?/v4", "32fa7e8331ae01ed2d9a7092d05a0e00", 92044),
	DT_ENTRY1("if18_anno1700", "1810?\?/v1", "ec665dafae59aced3bade7c5b64e1e2e", 865004),
	DT_ENTRY1("if18_anno1700", "1810?\?/v2", "b45b7f7bdb90745c53d5df06ddcfeea2", 880810),

	// IFComp 2019
	DT_ENTRY1("if19_treasurehunt", "1910??", "f3c35c9b3f4051897d7c891e8e33e779", 25371192),

	// IFComp 2020
	DT_ENTRY1("if20_jaft", "2010?\?/v1", "0c754a07c00d9701b36f68703f1a58bd", 2018252),
	DT_ENTRY1("if20_jaft", "2010?\?/v2", "ca12aa24867b9eb43aab1ad972dec202", 2018188),
	DT_ENTRY1("if20_rtcc", "2010?\?/v1", "c79ed07084f08890c2ec4912c353ed34", 300308),
	DT_ENTRY1("if20_rtcc", "2010?\?/v2", "0e4ed541ed65b7ea207b31a50c471c28", 333916),

	// IFComp 2022
	DT_ENTRY1("if22_lostcstlines", "221001", "55ca67bed8541e0343f7462605570129", 990831),
	DT_ENTRY1("if22_lostcstlines", "22????", "6d4b421bc366bb2b76cd31277ae461d9", 2201112),

	// InsideADRIFT #41
	DT_ENTRY1("i41_ambassador", "120202", "d19a12c2c8c2eff7b3e8da5f3594163a", 29708),
	DT_ENTRY1("i41_bariscebik", "120128", "d63d310e2b2fe4ab7ea6bc74c136b8e0", 14634),
	DT_ENTRY1("i41_canyoustandup", "120202", "b02aa7d160a6d1e3e86ce4f3027dd89e", 13425),
	DT_ENTRY1("i41_colomc_adrift", "120202", "6b8d8f115a89c99f785156a9b5802bc2", 12681),
	DT_ENTRY1("i41_dontgo_adrift", "120202", "f192abcc4e1f04787a7f1fde2ad05385", 13789),
	DT_ENTRY1("i41_murdererleft", "120117", "f95c63f90f28061a15dbe3bdf0cd4048", 13425),
	DT_ENTRY1("i41_readmay_adrift", "120108", "ba8a12e79520234e31622f141c99cafd", 13434),

	// P/o Prune's Mini Comp
	DT_ENTRY1("mc_diffipuzzle", "2105?\?/v1", "315033c146b17ea767ef6d76529065d6", 54700),
	DT_ENTRY1("mc_diffipuzzle", "2105?\?/v2", "5d94dd2adfccce9b8bcee9581f7bdfff", 59466),
	DT_ENTRY1("mc_gardenparty", "2105?\?/v1", "7176bb8c7b174166ae1c22c6411c6a93", 51770),
	DT_ENTRY1("mc_gardenparty", "2105?\?/v2", "e5149c4915f17ac97174f6afa46110b4", 64572),
	DT_ENTRY1("mc_illumina", "2105?\?/v1", "3db679c32f240f7dcda0cefe1bd2e7a0", 44262),
	DT_ENTRY1("mc_illumina", "2105?\?/v2", "185fff329dec70801e4b7957b4513808", 198446),
	DT_ENTRY1("mc_illumina", "2105?\?/v3", "c96b0810cc672f053d2ff3f42b4ebce0", 195958),
	DT_ENTRY1("mc_questgiver", "2105??", "98758b4ba735cf1503a32ad5fad95cce", 1555684),
#endif

	// 1st One Hour Comp 2002
	DT_ENTRY1("1h_endgame", "021021", "6036e5654786f841ca59f8f7e95276be", 4063),
	DT_ENTRY1("1h_frog", "021020", "3a8f3a0a6b4efd4787e6d338e4b31cea", 2089),
	DT_ENTRY1("1h_hauntedhouse", "021020", "cff22c2f2af619ac56e075e95385e600", 6541),
	DT_ENTRY1("1h_jasonvssalm", "021020", "70d60774dc777918645d7f3359f534cf", 2947),
	DT_ENTRY1("1h_1hrgame", "021020", "63e9594bf2b4e7cf5bf75dd9cc1353dc", 2978),
	DT_ENTRY1("1h_microbewillie", "021021", "14fe6ac0db4943fc9951610f5d0e0428", 3538),
	DT_ENTRY1("1h_amonkeytoomany", "021022", "c9d719261d0f760f95685899aa80bbf5", 5016),
	DT_ENTRY1("1h_princess1", "021021", "d745a3f2c4dc40fb10f25d1539a2a1e7", 7181),
	DT_ENTRY1("1h_chicken", "0210??", "40f0f52f2eeb029516bca3e01e6ceac0", 5249),

	// 2nd One Hour Comp 2003
	DT_ENTRY1("1h_forum", "030624", "4df8a13d8823aca26207fb459e98dd8b", 9005),
	DT_ENTRY1("1h_dfu", "030618", "e70e0bdfd9ffa0f20c2bc682eeb2f1ab", 16443),
	DT_ENTRY1("1h_percy", "030621", "e995d3a23005914eb92836f141ebe1c4", 5965),

	// 3rd One Hour Comp 2003
	DT_ENTRY1("1h_demonhunter", "031011", "ca37aaf35fb15a40a7f5f8caa1475112", 4169),
	DT_ENTRY1("1h_shore", "031013", "ef0b6074e47f55b7dff868e5dbd0c0cf", 4354),
	DT_ENTRY1("1h_imagination", "031014", "91923aeec3b7a8c55a15bc6dd240f7fb", 3257),
	DT_ENTRY1("1h_ticktick", "031013", "5b952d75e3e46a71334419c78dc6ff51", 2055),
	DT_ENTRY1("1h_forum2", "031013", "5a534ac4e39a319022d145094c46930a", 11185),
	DT_ENTRY1("1h_saffire", "031017", "d3eb89cf10d11a42df2df8f7bdb1505a", 7489),
	DT_ENTRY1("1h_cbn1", "031012", "9e27ab68a1f37f5f7591b362c4888526", 7577),
	DT_ENTRY1("1h_cbn2", "031014", "983eacedabebf7bbd8fed72ed399bba4", 6137),
	DT_ENTRY1("1h_ecod2", "031010", "ff55438090db248964fd8944f3a2da9f", 17584),
	DT_ENTRY1("1h_asdfa", "030930", "06173b47b4e88c0a494d2263666ad375", 27733),
	DT_ENTRY1("1h_asdfa", "031013", "fccb2fb890d554263d5f55bc02220ab8", 6440),
	DT_ENTRY1("1h_crm", "031012", "d97d1ff8f01a61fb477b76df65c77795", 15432),
	DT_ENTRY1("1h_crm", "031116", "9b63538d88e6d61b6091fae55598e32d", 5155),
	DT_ENTRY1("1h_pyramid", "031115", "a198f4ca146fed3094bf5fd70d2eefdd", 1953),
	DT_ENTRY1("1h_questi", "031016", "c7d6058e8172ff5d4a974fe159aa9cb7", 2544),

	// 4th One Hour Comp 2004
	DT_ENTRY1("1h_vagabond", "040229", "ae8545f6506fe3a49e27c96721196dce", 4762),
	DT_ENTRY1("1h_vagabond", "040329", "954858806b66da9cf80bd701c01ec168", 4762),
	DT_ENTRY1("1h_arghgreatescape", "040218", "0de27ef52495de415637d78e393c612f", 3608),
	DT_ENTRY1("1h_topaz", "020101", "7d4beb159bf3876f761bbac911395d05", 4839),
	DT_ENTRY1("1h_topaz", "020103", "0777a97e473b41ae04ab825556748e8d", 5980),
	DT_ENTRY1("1h_goblinhunt", "040220", "5e36c63feebf77cfba30757042a9fafa", 10891),
	DT_ENTRY1("1h_wreckage", "040221", "822b988245ff40c6f490edc046921fcf", 4487),
	DT_ENTRY1("1h_wreckage", "040302", "6d02b43c6f14ff851c5b006746ff89d3", 4486),
	DT_ENTRY1("1h_ecod3", "021018", "bd783057274a626868b052915bad2a2c", 10870),
	DT_ENTRY1("1h_trabula", "040228", "54fd39f8d86b4e1726970073342057da", 2224),
	DT_ENTRY1("1h_trabula", "040605", "b091d28e0c4fed988ae053415e8190aa", 2224),
	DT_ENTRY1("1h_agent4fmars", "040219", "407cc79b128aefdae4fdabb4d7a661e4", 3703),
	DT_ENTRY1("1h_agent4fmars", "040531", "2935a990ade72f4c51a22f5843306b98", 3686),
	DT_ENTRY1("1h_cah", "040221", "1945c5584b775b22c9b329915629eb44", 9808),
	DT_ENTRY1("1h_thecatinthetree", "040229", "f67640c2ed3b621272ebbea06a5fd467", 6174),
	DT_ENTRY1("1h_thecatinthetree", "040302", "fa9477ce0960b5819291479a78687315", 6176),
	DT_ENTRY1("1h_adriftmaze", "040301", "017d681d45402c2d75ae70c28d2b467f", 2224),
	DT_ENTRY1("1h_adriftmaze", "040302", "b91ff6d0d0038e9c9968ac62638a00d9", 2226),
	DT_ENTRY1("1h_undefined", "040301", "cf5172b53c8e3ef3a8739a31b78cccc2", 2920),
	DT_ENTRY1("1h_woof", "040219", "f66cbdcf9f7c6a3b30b0c7ca95bdea46", 1712),
	DT_ENTRY1("1h_woof", "040302", "b27d426413d982ed0325765a87916bb8", 1712),
	DT_ENTRY1("1h_icecream", "040228", "f68b7ec0959459cd1ee3c741660010c6", 5039),
	DT_ENTRY1("1h_icecream", "040302", "c16451fbd61f5dc2dd6bc29eff9e5759", 5039),
	DT_ENTRY1("1h_shredem", "040301", "426885b0a8a9aade134307fad1693512", 1209),
	DT_ENTRY1("1h_shredem", "040329", "d2daab124408fb67deeada21dc356840", 1212),
	DT_ENTRY1("1h_spam", "040229", "2ffc74d57936add0cdc06d83924f0f15", 2807),

	// 1st Three Hour Comp 2004
	DT_ENTRY1("3h_briefcase", "040705", "f1c899773faf027d9cbb50078a6667c3", 10872),
	DT_ENTRY1("3h_jailbreakbob", "040627", "c2e909cf2ddda863d2eb613c8273ee37", 31064),
	DT_ENTRY1("3h_zombiecow", "040612", "f0fe890f9679b158f9a22ed9f78179e6", 5193),
	DT_ENTRY1("3h_lostsouls", "040602", "3083e21a3bd6766ea5fdf3166e8bd4d8", 16695),
	DT_ENTRY1("3h_theamulet", "040619", "7c08e1f03763ad757dc39f5df37a28b7", 5585),
	DT_ENTRY1("3h_shadrick1", "040628", "33036e2b4c11a36d28e6532051e9f05b", 7456),
	DT_ENTRY1("3h_annihilofthink", "040704", "22743eceafe94bf60ab5a3e43241883e", 6303),

	// 2nd Three Hour Comp 2004
	DT_ENTRY1("3h_buriedalive", "040918", "9e9a71babf3012461205dc7aa2cd2a3f", 11808),
	DT_ENTRY1("3h_veteran", "040907", "65d5fc95b59f24e0e4997f62bf592ba3", 12043),
	DT_ENTRY1("3h_togetyou", "04???\?/v2", "388b5596945067ed3b92e6282e670825", 4496),
	DT_ENTRY1("3h_morely", "041116", "718faa6f6e72fcd4b95ebcb1601be7bd", 203757),
	DT_ENTRY1("3h_morely", "041124", "9386162f8c53584a693a865bbb3ed683", 6355),
	DT_ENTRY1("3h_sandl", "041118", "73a1e0b4451690e76d306136ec30754b", 27995),
	DT_ENTRY1("3h_zac", "041121", "85d95b8a6a962222d15724d42740f378", 13692),
	DT_ENTRY1("3h_zac", "041124", "5282bb5e768658946d37ebc5b380f461", 5098),

	// ADRIFT End of the Year 2002 Comp
	DT_ENTRY1("ey02_goldilocksfox", "020916", "12e596065ae94d3f86a67acb4de2aae7", 56539),
	DT_ENTRY1("ey02_goldilocksfox", "021219", "ed0d92c4d4e2c0936bf7686404f52204", 56572),
	DT_ENTRY1("ey02_woodsaredark", "030914", "4acae86746292076b90fa6fa73ea76d0", 71216),
	// DT_ENTRY1("ey02_unravelinggod", "020927", "e93e057e5f6790ebdd90bb940efa0300", 44382),
	// DT_ENTRY1("ey02_unravelinggod", "030618", "2d43577a756ddc6b561f447e67270ac4", 45000),
	DT_ENTRY1("ey02_adriftmasprt", "021202", "d06febddd1f136bb7ef59713c31f2066", 3034048),
	DT_ENTRY1("ey02_lairvampire", "020827", "6b9cecc5a79579ebe427578ed634147a", 133685),
	DT_ENTRY1("ey02_lairvampire", "021024", "33dce0c1c3f9ed2beded0bab931e8d78", 133689),
	// DT_ENTRY1("ey02_partytomurder", "020928", "bd2d6d3202ff772173220b9acdf613cf", 39180),

	// ADRIFT Spring Comp 2001
	DT_ENTRY1("sc01_menagerie", "010414", "3814400e3abb3e6340f055d5ec883c46", 117017),
	DT_ENTRY1("sc01_menagerie", "020511", "b5e10d082fc95bb62fc688cdb732c8f5", 1002489),
	DT_ENTRY1("sc01_wheel", "010418", "6c7993a9339530081da521235f715957", 44666),
	DT_ENTRY1("sc01_chosen", "010226", "63250f92da88b4909a65a76cdbd70776", 23866),

	// ADRIFT Summer Minicomp 2003
	DT_ENTRY1("sm03_sunempire", "030823", "667cb96684f2cf3367c86aa70638cd77", 24355),
	DT_ENTRY1("sm03_jimpond", "030713", "eef1aed7efbd36283b7d39b1514b3933", 50551),
	DT_ENTRY1("sm03_adriftorama", "040605", "db66d96e2cbd397fc668abcd32d59aeb", 46432),
	DT_ENTRY1("sm03_worstgame", "030824", "8789e209419529ac22d2be4774620c78", 9858),
	DT_ENTRY1("sm03_levilabyrinth", "03????", "be233d835a3f821e757b122539544bbf", 298592),

	// Beginner's Game Comp
	DT_ENTRY1("bg_trickortreat", "1708??", "060156357075cedaca65904ebacdad70", 66556),

	// Ectocomp 2007
	DT_ENTRY1("ec07_videotapedec", "071031", "658e9dbe38364219b6a530332f2ed183", 10173),
	DT_ENTRY1("ec07_videotapedec", "071211", "469463ccacba1b7db3a0f95f3c895b1e", 9423),
	DT_ENTRY1("ec07_witness", "071028", "fce8f16deac176ee958cdcf0c5964df6", 3849),
	DT_ENTRY1("ec07_stowie", "071030", "73322467bf8e29c9ea7745a095c9091e", 3785),
	DT_ENTRY1("ec07_foresthouse", "071208", "423fede762b0bfc1cb1547edce5d4aed", 8965),

	// Ectocomp 2008
	DT_ENTRY1("ec08_drinks", "081002", "855589bf11df411d6943bcdd0718ef4b", 8458),
	DT_ENTRY1("ec08_r2dc", "081029", "647af322abede732259866d91dd5ad4c", 8861),
	DT_ENTRY1("ec08_foresthouse3", "081009", "a4992f8debcd083c3d5489e2243d6929", 13492),
	DT_ENTRY1("ec08_salutations", "081030", "c7d16f4fa626346256e08a0ac6617b4d", 5591),
	DT_ENTRY1("ec08_hiker", "081006", "55b1ebfd0d37dc855fe4180cf45de0c4", 4401),
	DT_ENTRY1("ec08_blast", "081004", "ff101cb5b9a29701e1c9b0e8e42dfb85", 3447),
	DT_ENTRY1("ec08_delsol", "081031","ab2a1d114782f8bc2b4da5d1b56f8de7", 6552),

	// Ectocomp 2009
	DT_ENTRY1("ec09_drivingnight", "091031/v1", "a66f03afba88b6ece716201319bda621", 7505),
	DT_ENTRY1("ec09_drivingnight", "091031/v2", "d7f914130efffd907d82914a7c1c6ee2", 7479),
	DT_ENTRY1("ec09_patient7", "091031", "a9417623fdeb4370f25f7849968bdeb3", 15502),
	DT_ENTRY1("ec09_wayout", "091030", "75c96f7fdf26104992db0a5b754637f1", 4598),

	// Ectocomp 2010
	DT_ENTRY1("ec10_petespunkin", "101031", "98dbc751d01eb51b0a3fda3562c6960d", 8512),
	DT_ENTRY1("ec10_renegadebw", "101031", "6ba674cb8f8c531d66c9b22c446bea2d", 8346),
	DT_ENTRY1("ec10_helsing", "101031", "9c06145372e7128bc8ce9ca6f2ef97da", 9776),
	DT_ENTRY1("ec10_iwasteenage", "101025", "d5890b294eb82890e3fdfa979cd8aa7b", 6521),
	DT_ENTRY1("ec10_thevault", "101028", "8cd76c2b9278f37b98666317a823c44d", 4258),
	DT_ENTRY1("ec10_flyhuman", "101010", "06e4371bf939531f0bb91dcaaa08c88a", 4743),
	DT_ENTRY1("ec10_allhallows", "101015", "941fb55e949f3272a089ea69a6ddba8d", 6707),
	DT_ENTRY1("ec10_tenebrasemper", "101031", "f0db9c56ba0c50fad1b1baaedf156771", 9757),

	// Ectocomp 2011
	DT_ENTRY1("ec11_attacklobster", "111031", "e4fdf252cfa05e672ffaeb806e9d6c0c", 7186),

	// Even Comp 2009
	DT_ENTRY1("ec_attmutaydid", "091213", "23c47fd2ff2a06e05e7ae7ba028ce090", 513587),
	DT_ENTRY1("ec_rain", "091214", "9f19f6c2637867be0bfce484b2daee8f", 10512),
	DT_ENTRY1("ec_perspectives", "091209", "c1fdcd8c811a892186c7d7ca6e070f44", 8043),

	// Finish the Game Comp 2005
	DT_ENTRY1("ftg_pathway_adrift", "050922", "bcf1f61393c1b8123a98ee4879ffd8a6", 20896),
	DT_ENTRY1("ftg_takeone", "050917", "114caad9308b7adf9e15267a11f12632", 9547),
	DT_ENTRY1("ftg_demonhunter2", "050920", "2af5b32810e6960881122732f48d3ccf", 18644),
	DT_ENTRY1("ftg_hunter", "050924", "d7af67a0753910bf32ec233e538ad659", 23531),
	DT_ENTRY1("ftg_shadow", "050924", "f4d0a427ebec3a4c3b7029adaa3e401f", 272030),
	DT_ENTRY1("ftg_shadowjack", "050813", "25f33d23e126ea0a86ed55833dbba8ef", 11606),

	// Hourglass Comp 2006
	DT_ENTRY1("hc_longbarrow", "060724", "b74d914a61421dbc322f3dbc7656eb97", 6211),
	DT_ENTRY1("hc_questforfood", "060805", "0618e06900fd5f15b3b18594d98a8090", 4534),
	DT_ENTRY1("hc_herrdoktor", "060802", "7c71715d076ce00cf39043b1c60d3bd4", 4422),
	DT_ENTRY1("hc_3minutes", "060729", "0313370bb74005525ec96580f146d286", 13232),
	DT_ENTRY1("hc_smote", "060805", "ef386edce4af4e55152596759cbc8e8c", 1987),
	DT_ENTRY1("hc_dancing", "060726", "eee9f9d60bc1f642fae83f59117a8780", 4578),
	DT_ENTRY1("hc_boiledeggs", "060807", "b4cb135ba16c5177c58c7a3f146437e8", 6423),
	DT_ENTRY1("hc_chooseyourown", "060728", "fb9c940b6997e9fe961db910cd7e882c", 10332),
	DT_ENTRY1("hc_rollingdough", "060801", "f36ab00d8f64b55569ddd2227f20c6e3", 3919),
	DT_ENTRY1("hc_roadnowhere", "060807", "4bdc52566f67189ea1239e73c8734435", 7903),
	DT_ENTRY1("hc_overtheedge", "060806", "d3fcc9f5fd301731610e0b3cd8c9ddd1", 8128),
	DT_ENTRY1("hc_skydiver", "060729", "57f0c54ab6e8c87904ebfde9d255d6f9", 7631),
	DT_ENTRY1("hc_pilfers", "060727", "fad35bf79453844ef82f0a9b11d5d6e2", 3727),

	// IFComp 2000
	DT_ENTRY1("if00_wrecked", "010110", "b1c6c4ef73025fbbe99b612a72e3186a", 88550),
	DT_ENTRY1("if00_marooned_bd", "000930", "5e3e0435c98a0a915a98d5b1b080522c", 50516),
	DT_ENTRY1("if00_marooned_bd", "020916", "6856af3da4f2821124115d16a610d20e", 12287),

	// IFComp 2001
	DT_ENTRY1("if01_tcom1", "01????", "61364f96ffdfd01e878f1a33557aff93", 37422),
	DT_ENTRY1("if01_tcom2", "01????", "b22abbff746998051e06bfa9a8083432", 67783),
	DT_ENTRY1("if01_mysterymanor", "010914", "c3ca694117fbf7b655f3b7d729d94048", 74498),
	DT_ENTRY1("if01_mysterymanor", "020919", "4262cee0ad7ff3062aa08ac69b980625", 24879),
	DT_ENTRY1("if01_thetest", "010923", "61a6f178f646c6158c0557ae4257b20d", 22465),

	// IFComp 2002
	DT_ENTRY1("if02_pkgirl", "0209?\?/v1", "038fb1c7d80bc03a48f71ab5231ab08f", 1627482),
	DT_ENTRY1("if02_pkgirl", "0209?\?/v2", "196773493b04cf42c92f6d1e85039469", 1645174),
	DT_ENTRY1("if02_pkgirl", "0608??", "c9282cf8f91ebfe5a93b136f56d7a1aa", 1645515),
	DT_ENTRY1("if02_unravelinggod", "020927", "e93e057e5f6790ebdd90bb940efa0300", 44382),
	DT_ENTRY1("if02_unravelinggod", "030618", "2d43577a756ddc6b561f447e67270ac4", 45000),
	DT_ENTRY1("if02_partytomurder", "020928", "bd2d6d3202ff772173220b9acdf613cf", 39180),

	// IFComp 2003
	DT_ENTRY1("if03_sophie", "03????", "85e8f7306ca02dabe0a083828981c31a", 531015),
	DT_ENTRY1("if03_sophie", "031116", "60d7c8034c96c935ab4c6595655da3d1", 536662),
	DT_ENTRY1("if03_sophie", "040104", "7436328561d2ab8686649adfbe1edb17", 536710),

	// IFComp 2004
	DT_ENTRY1("if04_daylifesh", "04???\?", "e3d14c0caf21f2739d1f461333fed885", 165073),
	DT_ENTRY1("if04_daylifesh", "041001", "c69534cf0c1e373a91978438ede6ba17", 164543),

	// IFComp 2005
	DT_ENTRY1("if05_etny", "050930", "8a376d5bdf833fa79614271d05dd36a8", 59531),
	DT_ENTRY1("if05_etny", "051117", "990c0e2390d197ebcae1109e5534c433", 59583),
	DT_ENTRY1("if05_mortality", "0508?\?/v1", "f940c211a945c2c81b1f72941a2d661c", 94578),
	DT_ENTRY1("if05_mortality", "0508?\?/v2", "1422ba317791af9084d9646c2e50ee30", 99502),
	DT_ENTRY1("if05_vendetta", "050930", "1d84bb4a60e7c559774133f6aff48bbb", 104909),
	DT_ENTRY1("if05_plagueredux", "05????", "ed6ea0a04b135e4531636dc4e5005195", 92327),
	DT_ENTRY1("if05_ptbad65", "050930", "2a90b684c6c086b6e4d7143d70da3d4f", 3349),

	// IFComp 2006
	DT_ENTRY1("if06_unauthtermin", "060928", "c28bf76f5c9873c0944db39b0c3faadd", 107316),
	DT_ENTRY1("if06_unauthtermin", "061224", "a7bba6890fe2945bb192c67e395516ac", 107335),
	DT_ENTRY1("if06_thesisters", "060824", "cd5aec78ac5cd64b7444768faecd10fc", 46788),
	DT_ENTRY1("if06_thesisters", "061204", "3f4d015651477767377390e237ee7621", 46836),
	DT_ENTRY1("if06_requiem", "0605??", "3cbe26d9153edaa74d000ca86f15920e", 114759),
	DT_ENTRY1("if06_wumpusrun", "060922", "464c1b782ef3dd4fe0623cfe819ac04e", 858951),
	DT_ENTRY1("if06_ptgood", "060929", "aca9cde82850a28b2da871c4d9e34edb", 1971),

	// IFComp 2007
	DT_ENTRY1("if07_fineday4reap", "070918", "9733d9cd4f000b409c171ecb507f1c6b", 96741),
	DT_ENTRY1("if07_mymindmihmas", "070922", "593f536f38418fdd334a6fa50fd79d97", 68353),
	DT_ENTRY1("if07_inthemind", "070910", "111ff1ff5c451c0a31b6849dc0277915", 101898),

	// IFComp 2008
	DT_ENTRY1("if08_datewithdeath", "0809??", "e52151688837e9c0be232b841097688a", 180314),
	DT_ENTRY1("if08_cybercow", "080929", "1ee59ffd44378be75b4b099c06e386a9", 108209),

	// IFComp 2009
	DT_ENTRY1("if09_yonastcastle", "090929", "f157eaf4c75d6f8499fa188ae7059cb0", 2326484),
	DT_ENTRY1("if09_yonastcastle", "100513", "baf2c0423903a3104f67d3a19bde43df", 2320881),
	DT_ENTRY1("if09_ascot", "090730", "01ed200d7aec907aa84ad4007c5b296b", 24494),
	DT_ENTRY1("if09_ascot", "090731", "878ebf802cab7ad9e0d2ff54be096266", 24332),
	DT_ENTRY1("if09_hangover", "090623", "610c83cbe9d9e1353ebc444a769c85a9", 33933),

	// IFComp 2011
	DT_ENTRY1("if11_cursed", "11???\?/v1", "ebfd66b37cc4f288e94825768c905e8e", 464552),
	DT_ENTRY1("if11_cursed", "11???\?/v1/Hints", "56b1b8a2ade186686004d3517fbedbc9", 32536),
	DT_ENTRY1("if11_cursed", "11???\?/v2", "e01017d4bf6462a2aed5129e05e25e1f", 487499),
	DT_ENTRY1("if11_cursed", "12???\?/v2", "62d2e05e62f1137b25a61bbb46154729", 487990),
	DT_ENTRY1("if11_cursed", "11???\?/v2/Hints", "4ad13bf274af97ebbfe47c4a852b3a46", 36793),
	DT_ENTRY1("if11_suzygotpwrs", "110624", "f2d47df24e1d2a71c60a58831bc9b9fa", 40676),

	// IFComp 2012
	DT_ENTRY1("if12_irvinequik", "120929", "213092ee3387a2b9eecf0ca93cd7a0be", 195043),
	DT_ENTRY1("if12_irvinequik", "121103", "a05651907f191f5f4ab9fb9ff51fd80d", 195059),

	// InsideADRIFT #41
	DT_ENTRY1("i41_cut", "120202", "0369af54c60f3d25ea2ebfe80579057b", 6102),

	// InsideADRIFT Game of the Year Comp 2004
	DT_ENTRY1("goy_paint", "040219", "d2bdca626f51b216f400355318c40000", 54899),
	DT_ENTRY1("goy_paint", "040829", "1183921d034d3614ec277e18d9112b69", 76493),
	DT_ENTRY1("goy_shardsmemory", "041024", "7e9cb5002fa22a25000b3478c826413d", 118610),
	DT_ENTRY1("goy_waxworx", "041216", "94aaf6625541238867b079bfe475f146", 38471),
	DT_ENTRY1("goy_shadrick2", "040805", "a9e1e59b37976c6c0fcc6aee69978afe", 113385),
	DT_ENTRY1("goy_shadrick2", "040831", "6c39ed75d19b60ee7b806edb97725d43", 113424),
	DT_ENTRY1("goy_dragonshrine", "040412", "604250027cfd8cfd1fb0d231085e40e5", 636932),
	DT_ENTRY1("goy_darkness", "041214", "6047363499d5eb666c341139d0eb532a", 24847),
	DT_ENTRY1("goy_darkness", "050107", "af7b3452fdb40f121f2a6be2fa81e4f8", 25203),

	// InsideADRIFT Game of the Year Comp 2007
	DT_ENTRY1("goy_fineday4reap", "071119", "ff47d5fa749d52e18c1d488b83d1d45e", 96834),
	DT_ENTRY1("goy_fineday4reap", "071123", "3eb40409db8034e4d2d4ff77e406caba", 96819),
	DT_ENTRY1("goy_rking", "0703?\?/v1", "198d2ac6056ea8f6b1acec7ce6fcdb03", 98452),
	DT_ENTRY1("goy_rking", "0703?\?/v2", "ef460bce1b6b692c714ff720f7884165", 183341),
	DT_ENTRY1("goy_rking", "0703?\?/v3", "2c9ede2cb9a50f7274bed28c9d9bc5f7", 186477),
	DT_ENTRY1("goy_marika", "071028", "9825adb22d66d16eb463f1c2674e1162", 29174),
	DT_ENTRY1("goy_marika", "071123", "d7694ac50de852126fadc7cc73dffd76", 29625),
	DT_ENTRY1("goy_marika", "071231", "af69d85f6766aad3d7c3983f0d77a3c8", 29687),
	DT_ENTRY1("goy_puzzlebox", "071116", "f0c4e68a0fe03947621eb35506bc509f", 19262),
	DT_ENTRY1("goy_puzzlebox", "071208", "0adbb7f9a7c37819e1a695468001acaa", 19540),
	DT_ENTRY1("goy_videotapedec", "071031", "658e9dbe38364219b6a530332f2ed183", 10173),
	DT_ENTRY1("goy_terrified", "070205", "38697e087d267c4ea0d6c607ce463847", 10488),

	// InsideADRIFT Spring Comp 2004
	DT_ENTRY1("sp04_shardsmemory", "040417", "9b75834354aed4b148d4ec2b8cdd66ae", 118604),
	DT_ENTRY1("sp04_dragonshrine", "040412", "604250027cfd8cfd1fb0d231085e40e5", 636932),
	DT_ENTRY1("sp04_waxworx", "040417", "4fa14c7944506f4e0142cb865738d0b0", 37542),
	DT_ENTRY1("sp04_waxworx", "040528", "97e5e0c7206a062f8892ba91fa721f59", 37541),
	DT_ENTRY1("sp04_waxworx", "050118", "a10a1e404ae77fd968b4427a13bd7034", 38414),
	DT_ENTRY1("sp04_sommeril", "040303", "9e8ee4cbd4ecab3ab51c702f4e0277e1", 400940),

	// InsideADRIFT Summer Comp 2004
	DT_ENTRY1("su04_chooseyourown", "040822", "774ae5ba4fea61da0898069df4ed1c81", 54869),
	DT_ENTRY1("su04_shadrikundgrd", "040805", "a9e1e59b37976c6c0fcc6aee69978afe", 113385),
	DT_ENTRY1("su04_darkhavenmyst", "040814", "1027e9e3dba08e0fd2796e0e934be855", 27826),
	DT_ENTRY1("su04_adriftproject", "0407??", "18a4db8af27fd98751504ee3e45089e3", 231594),
	DT_ENTRY1("su04_tick2nowhere", "04???\?/v1", "a69b0fa1b33d8e0ab6504d37615e6a3c", 65083),
	DT_ENTRY1("su04_tick2nowhere", "04???\?/v2", "8c549a6bdc6a5a4895412516be8dce25", 65502),
	DT_ENTRY1("su04_tick2nowhere", "04???\?/v3", "123aa5225a1060bd3b12bca07ac95e18", 65725),
	DT_ENTRY1("su04_tick2nowhere", "04???\?/v4", "b969b13cd0fe0d499f9c4d414e4abd79", 65526),
	DT_ENTRY1("su04_tick2nowhere", "04???\?/v5", "b440d0b4b18a93537803750f31e838b7", 9436),
	DT_ENTRY1("su04_tearstoughman", "040822/v2", "da92145aebc1d511cb12ff7c433b0438", 9437),

	// InsideADRIFT Spring Comp 2005
	DT_ENTRY1("sp05_blood", "05???\?/v1", "775404faa0d7973c1df537bd423a45d3", 100201),
	DT_ENTRY1("sp05_blood", "05???\?/v2", "7c56c7ecaf14c440eefa64afadac8103", 100210),
	DT_ENTRY1("sp05_blood", "05???\?/v3", "7b7a00f5d26e352a1650f13f9dd54869", 100202),
	DT_ENTRY1("sp05_frustrated", "050416", "1c218d4fcbbfa11271fb4d0003df7a6a", 210267),
	DT_ENTRY1("sp05_frustrated", "050507/v1", "ed79b5b0e3fd477441ee8db54c292380", 50625),
	DT_ENTRY1("sp05_frustrated", "050507/v2", "86dcc06a9edcd967183d5448165fd92e", 50578),
	DT_ENTRY1("sp05_privateeye", "050414", "3076b9f11acf2b1e377a31cd1a11c323", 236323),
	DT_ENTRY1("sp05_seaside", "050416", "271c64455f4abb3dee35c3902e5cdfc1", 279480),
	DT_ENTRY1("sp05_hub", "0504??", "c3224b3d6c55997056ea195ba4ebb60a", 24610),

	// InsideADRIFT Summer Comp 2005
	DT_ENTRY1("su05_target", "050820", "ea12656119b5522156539dd1da23866b", 42999),
	DT_ENTRY1("su05_lights", "0508?\?/v1", "cebe4af96402a18915f15205e52c7415", 124774),
	DT_ENTRY1("su05_lights", "0508?\?/v2","41470ec04235f0b6fa33ac1eb1a98140", 125378),
	DT_ENTRY1("su05_cibass", "05????", "7c2e95feb2e2debbbb0c343c1a920f6f", 21705),
	DT_ENTRY1("su05_mustescape", "050817/v1", "3de7c345c6f1dc26712b4ce6fa365830", 17472),
	DT_ENTRY1("su05_mustescape", "050817/v2", "e58fd08918fa3d5a38f424f79595fb4e", 17497),
	DT_ENTRY1("su05_regrets", "05????", "5b1a5763226b0f489e1d0760be59ccf4", 10448),

	// InsideADRIFT Summer Comp 2006
	DT_ENTRY1("su06_reluctvampire", "0606??", "619d031556cdff0805e599d6a82caa1d", 199575),
	DT_ENTRY1("su06_pestilence", "060629", "2d5ff3fe2d866150689749087d5cee3d", 29021),
	DT_ENTRY1("su06_spooked", "060623", "642ff47378f398ca97e9238dddea3914", 9908),
	DT_ENTRY1("su06_spooked", "060721", "ea200097ef6603bc8ee54fd4b7128edb", 9909),

	// InsideAdrift Summer Comp 2008
	DT_ENTRY1("su08_happyvalley", "080702", "375b217ae661e4ce192b0ea3756a01b6", 24938),
	DT_ENTRY1("su08_marlin", "089795", "15627cddf3b2a30393397d4faf2f6df8", 13192),
	DT_ENTRY1("su08_wolvesdoor", "080617", "2341592c42f93f12033f8a309fc11386", 19126),
	DT_ENTRY1("su08_huntung", "080701", "e3fe91ea00f762fbc78f5297e240f59f", 42578),
	DT_ENTRY1("su08_door", "080706", "696b2c0579a2c66afc5be808e54e4847", 6363),

	// InsideADRIFT Summer Comp 2010
	DT_ENTRY1("su10_aegis", "100802", "8726e90b50bdc92967f944a582daff2c", 45755),
	DT_ENTRY1("su10_camelot", "100731", "ad0856d169448c6a43d2f2441aa13780", 53528),
	DT_ENTRY1("su10_camelot", "100810", "f4c632787aebebbad220e5e4edb944f5", 54148),
	DT_ENTRY1("su10_plunder", "10???\?/v1", "e4f28c23cb4faf4387a8859eb8221f59", 16696),
	DT_ENTRY1("su10_plunder", "10???\?/v2", "837f53cd1d467a04e92f048719631a56", 17989),
	DT_ENTRY1("su10_plunder", "10???\?/v3", "f95124ff1253a03d71d3a8718ece299f", 17947),
	DT_ENTRY1("su10_baroo", "100802", "d6919d2e21beafb46c5b4712703bea73", 46299),
	DT_ENTRY1("su10_lightup", "100802", "2805a4f680c1b8343b940e3c89fc867b", 28347),
	DT_ENTRY1("su10_motion", "1008??", "25382d94db738588da78078cc34f83cc", 11579),

	// Intro Comp 2005
	DT_ENTRY1("ic05_srs", "050429", "f1243c19976246c2c319fba5be6f9d13", 157833),
	DT_ENTRY1("ic05_finalquestion", "050421", "1f1ede210336b6b8bd8e28eda2d00e0c", 15555),
	DT_ENTRY1("ic05_murdermansion", "050507", "07367e1f546f80e64b44923f474f5906", 6793),
	DT_ENTRY1("ic05_zacksmackfoot", "050507", "a8d2d303c6e5f698d40714c3d5e241e2", 6374),
	DT_ENTRY1("ic05_outline", "050509", "b86de4feca73e0ea49c324a2181b8b8f", 5724),
	DT_ENTRY1("ic05_mustescape", "050515", "c5a13fa9059ebc2904eea95e1c6b790a", 3964),
	DT_ENTRY1("ic05_rift", "050515", "b4dc3233be12767837bd596789379473", 2606),
	DT_ENTRY1("ic05_p2p", "050427", "3b3386f5f0f844c3a9eb47ba2596135a", 6491),

	// Intro Comp 2009
	DT_ENTRY1("ic09_apokalupsis", "090430/Intro", "c3e90bfb8cea957cebbf306add17cd3e", 19822),
	DT_ENTRY1("ic09_dbaa", "090505/Intro", "e827961c66e95bbc2a0d4645f63f6ec4", 37521),
	DT_ENTRY1("ic09_throughtime", "0905?\?/Intro", "181706abbfc03745d1ddc9b788638b9a", 42109),
	DT_ENTRY1("ic09_existence", "090531/Intro", "041e134a39146830a389d8dc62206563", 6370),
	DT_ENTRY1("ic09_teaw", "090531/Intro", "132d0803e31d70729194a292be93be64", 1235834),
	DT_ENTRY1("ic09_donuts", "090530/Intro", "55d52c6cd6a42dcb424ff3888e7aa88b", 5201),
	DT_ENTRY1("ic09_yoncastle", "090505/Intro", "6dd95cdc736ffab10a08e39cd9069356", 9814),
	DT_ENTRY1("ic09_dishduty", "090524/Intro", "263db72fe0be3fb494aa2566fd7e38b5", 5753),
	DT_ENTRY1("ic09_deadrace", "090531/Intro", "b62ec01f511e761d65ab637de7dda0f2", 5311),
	DT_ENTRY1("ic09_merlinbrdprey", "090516/Intro", "2e64695a67e1b4ffc0fc379d3f5a4e4d", 3842),

	// Mini-Comp 2003
	DT_ENTRY1("mc03_neighbours", "030626", "afdf85651706a4568762565b2c24aa8d", 13333),
	DT_ENTRY1("mc03_neighbours", "030906", "686d8c0995aa085fb056c6a5bd402911", 13520),
	DT_ENTRY1("mc03_thorn", "03????", "0d02030e9ac6563cda759a20ae1fc06b", 13887),
	DT_ENTRY1("mc03_diarystrip", "030626", "0e04238a2b49220c284ce9c8723abd77", 43334),
	DT_ENTRY1("mc03_monsters", "030624", "02a042f7277dfc9789bd6ffee1e8db08", 9628),

	// No Name Competition
	DT_ENTRY1("non_piecesofeden", "080530", "378dce8e7d36433a188cb0f2d69f2570", 5741),
	DT_ENTRY1("non_beanstalk", "080531", "fc9c17c355894948dc2e72c66e8d9963", 9837),
	DT_ENTRY1("non_perfectspy", "080521", "4248dee44c099804ee2932b7b8cbea9d", 7988),

	// One Room Comp 2003
	DT_ENTRY1("1r_deadman", "030228", "480da7306b482ac2df44527931523ccc", 14092),
	DT_ENTRY1("1r_deadman", "030311", "fa965f82a338db5c8ab0e101d092bcec", 16148),
	DT_ENTRY1("1r_dayattheoffice", "030311", "2d908b5448c6fd2dbc98a7f9d2dda9df", 13824),
	DT_ENTRY1("1r_dayattheoffice", "031213", "4771123f35431714ad489a2abc4448a9", 13802),
	DT_ENTRY1("1r_msmobius", "030304", "81cfc7e042294e71c60a264a2fbb3f2d", 23225),
	DT_ENTRY1("1r_insanity", "030213", "c57c140cc9331a60695a3f90fba8fb9a", 12660),
	DT_ENTRY1("1r_everything", "030227", "44a5b102675bcbc48c04c269301d0cd3", 20065),
	DT_ENTRY1("1r_i", "030213", "4e7e5beab23931f6f574c13cbde8c535", 5164),
	DT_ENTRY1("1r_trappedgirl", "030227", "61e1c14dddef8d3b6804c47b2072d9c4", 17235),
	DT_ENTRY1("1r_dreamland", "030211", "4eb8715f7a8f30c7e9baabae999b1148", 6508),
	DT_ENTRY1("1r_dreamland", "030301", "684cd1932e909105b7898953792c4065", 2422),

	// P/o Prune's Spring Ting 2009
	DT_ENTRY1("st09_reactor", "090223", "a5a0005b4c6668dfe27e1658f85d0713", 11380),
	DT_ENTRY1("st09_mrfluffy", "090213", "c04ffec6475a2ac3d79290613edeecd6", 14541),
	DT_ENTRY1("st09_sandy", "090324", "a6953f57c4632dc9847ef5dfe494283a", 1286),
	DT_ENTRY1("st09_homelesslarry", "090215", "1508b4cb0cd4be5d7d8365abe4660c36", 21880),

	// P⁄o Prune's Spring Ting 2010
	DT_ENTRY1("st10_wghn", "100531", "ae6a590cf043c14b10e9567d6f6661f3", 3559268),
	DT_ENTRY1("st10_egghunt", "100430", "ade16e3ac57d32a6be3c95438a651683", 7575026),

	// The Challenge Comp
	DT_ENTRY1("chg_mangiasour", "110423", "699e454caf6b2134acdfa8284b84721c", 28676),
	DT_ENTRY1("chg_suburban", "110423", "2bb9e69e373d35e6426185473decb5d0", 2427725),
	DT_ENTRY1("chg_whitterscap", "110422", "0456c1308fe97af89ccb097cc97b0f10", 7604),

	// The Odd Competition
	DT_ENTRY1("odd_witchtale", "080926", "2c69ada0c304413364739e98f3e8e8c1", 15252),
	DT_ENTRY1("odd_iamlaw", "080927", "9835c286df225e0357ae99a47775a78b", 30185),
	DT_ENTRY1("odd_yakshaving", "080925", "3cfab2021cb4711229bdc380b6a19058", 9716),
	DT_ENTRY1("odd_retforesthouse", "080924", "47a4a95da7babb06066825d048d20056", 8019),
	DT_ENTRY1("odd_maincourse", "080926", "f9c5a5041de7b2fc74f007339deb93de", 7389),
	DT_ENTRY1("odd_businessusual", "080926", "d71d95db428047e0963042ad09181176", 7235),
	DT_ENTRY1("odd_businessusual", "080927", "cbde699c47f946ef3936b8866affc5a7", 7234),
	DT_ENTRY1("odd_gorxungula", "080923", "b04e511c4a47adbf7b2d6392ed6d5f70", 6777),
	DT_ENTRY1("odd_human", "080927", "eff595bd11066585b41275e92b461f11", 7427),
	DT_ENTRY1("odd_human", "081126", "f5a6697f301011934bd36587c28a136f", 7392),
	DT_ENTRY1("odd_astaftermath", "080916", "d555b913fd85319b745a83efeb9a0601", 5792),
	DT_ENTRY1("odd_seance", "080922", "619599e36090917ea6ef4d02cd62c484", 10934),

	// Twin Comp 2009
	DT_ENTRY1("tc_crashland", "090717", "3626601b910e0beb7be717eb5381e6bf", 8106),
	DT_ENTRY1("tc_melancholy", "090927", "355aa8cbe66388627edb4cf2742cb22c", 43845),

	// Writing Challenges Comp 2006
	DT_ENTRY1("wri_jgrim", "060318", "0df0ded3147cd8bfd59787ab57ba00d7", 42994),
	DT_ENTRY1("wri_jgrim", "061224", "68b8e430bcc8515d7cccbc1dc136aabe", 96720),
	DT_ENTRY1("wri_toomuchexer", "060316", "aebb58d94f632c4232bee72b9a1b5e25", 6248),
	DT_ENTRY1("wri_glumfiddle", "060314", "f4ea370e93b2fad4d873f82332b113cb", 23074),
	DT_ENTRY1("wri_manoverboard", "060304", "c14ac9285a265451aec669d76ca5d4d1", 14141),

	// Danish games
	DT_ENTRYL1("halloween", Common::DA_DNK, "2106??", "ed3155a2581428dc72506297dc4cc49e", 67476),
	DT_ENTRYL1("illumina", Common::DA_DNK, "2106??", "2a79f187f6c67b2160ae8baf8d6e4742", 202576),
	DT_ENTRYL1("oct31d", Common::DA_DNK, "22????", "7f0637944bc69e2bff843e76689b645b", 194758),

	// French games
	DT_ENTRYL1("bellesmeres", Common::FR_FRA, "130318", "3d556ba5448b4bce8e7b0ee818bd1e79", 73972744),
	DT_ENTRYL1("enquete", Common::FR_FRA, "160906", "56157297a33db4855bf2a6eb2b06ef38", 156919),
	DT_ENTRYL1("largo", Common::FR_FRA, "061022", "2544ee9502a97511b27fee722508bd2c", 282396),
	DT_ENTRYL1("lesfeux", Common::FR_FRA, "050928", "b3534d72ce3d2a5bd40d1b0d922419a4", 54162),
	DT_ENTRYL1("quiatuedana", Common::FR_FRA, "050928", "d22079f4b173d33272bb5f3e97f71aae", 20034),

	// German games
	DT_ENTRYL1("ronforestd", Common::DE_DEU, "020326", "d445d58bc4737003f70edc4b34cc04a6", 7972),
	DT_ENTRYL1("rontoxicd", Common::DE_DEU, "020412", "453daa2ec43a6dcc795a02debcd562d6", 60110),

	// Italian games
	DT_ENTRYL1("ilgolem", Common::IT_ITA, "100130", "7ab97545be9c30457d0c9e748ab7885f", 6372213),

	// Spanish games
	DT_ENTRYL1("bandera", Common::ES_ESP, "080218", "e5257c2b5c66f2519da56023f5c84cc6", 36653),
	DT_ENTRYL1("bandera", Common::ES_ESP, "080319", "dd472902495610b4db02267d4fa2eff3", 8260541),
	DT_ENTRYL1("caidalibre", Common::ES_ESP, "071109", "acdea550b0323f2e0832f66a6fbf5eaa", 18926),
	DT_ENTRYL1("elascensor", Common::ES_ESP, "081208", "30494599eb5c012b677fec49fa0766a8", 3219428),
	DT_ENTRYL1("impulso", Common::ES_ESP, "050531", "c53f5ac48c9a831ce18a611301b5cb1b", 22214),
	DT_ENTRYL1("n3reloj", Common::ES_ESP, "021115", "0a65561670b4df68c1a47decd012cd7c", 17966),
	DT_ENTRYL1("nano", Common::ES_ESP, "080331", "f8fc86237a302999112b451b97992797", 21775),
	DT_ENTRYL1("renuntio", Common::ES_ESP, "040107", "feb81a137ac9641008dced9ff706449f", 48764),
	DT_ENTRYL1("vardock", Common::ES_ESP, "080902", "a20d1dcd26e8d1c1a981f97691c5e424", 2928980),

	// Russian games
	DT_ENTRYL1("akronr", Common::RU_RUS, "121119", "9ee9af6f08091f7e3c7c0043f99e378f", 31744),
	DT_ENTRYL1("bookexistence", Common::RU_RUS, "050214", "6f2bca3a945a221c575fa1b05d99e7a3", 183448),
	DT_ENTRYL1("dolg", Common::RU_RUS, "040312", "7f00a84a881593e1e61943520641dfb4", 377103),
	DT_ENTRYL1("golddragon", Common::RU_RUS, "040131", "6b36ffed69e8389ebfb42fe1d6a367a7", 238623),
	DT_ENTRYL1("nat01", Common::RU_RUS, "070426", "7d9f722d6b3cafbab8b6e962598243e6", 47603),
	DT_ENTRYL1("nightelmstreet", Common::RU_RUS, "110123", "5b6ea1810d1320d3262a61c9f7e6de22", 51770),
	DT_ENTRYL1("relife", Common::RU_RUS, "030607", "d936a059df8362d7d45d10f38d1d4332", 62524),
	DT_ENTRYL1("shablon", Common::RU_RUS, "030527", "bcb2a259e96ed2717ba7e6ce56d51750", 21271),
	DT_ENTRYL1("zanoza", Common::RU_RUS, "070426", "773eb8198e9f138215f82421f6ba0bf5", 91722),

	DT_END_MARKER
};

} // End of namespace Adrift
} // End of namespace Glk
