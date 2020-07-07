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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Adrift {

const PlainGameDescriptor ADRIFT_GAME_LIST[] = {
	{ "Adrift", "Adrift IF Game" },

	// English Games
	{ "3monkeys", "Three Monkeys, One Cage" },
	{ "akron", "Akron" },
	{ "albridgemanor", "Albridge Manor" },
	{ "asdfa", "A Short Damn Fantasy Adventure" },
	{ "awalkatdusk", "A Walk At Dusk" },
	{ "bariscebik", "Bariscebik" },
	{ "barneysproblem", "Barney's Problem" },
	{ "beanstalk", "Beanstalk the and Jack" },
	{ "beerisntenough", "WHen Beer Isn't Enough" },
	{ "caveofwonders", "Cave of Wondors" },
	{ "circusmenagerie", "Menagerie!" },
	{ "cityInfear", "City In Fear" },
	{ "coloromcadrift", "Color of Milk COffee" },
	{ "compendiumendgame", "The Woodfish Compendium: The Game to End All Games" },
	{ "compendiumforum", "The Woodfish Compendium: Forum" },
	{ "compendiumforum2", "The Woodfish Compendium: Forum2" },
	{ "compendiumimagi", "The Woodfish Compendium: ImagiDroids" },
	{ "compendiumsaffire", "The Woodfish Compendium: Saffire " },
	{ "compendiumtopaz", "The Woodfish Compendium: Topaz" },
	{ "cowboyblues", "Cowboy Blues" },
	{ "crawlersdelight", "Crawler's Delight" },
	{ "crimeadventure", "Crime Adventure" },
	{ "cursed", "Cursed" },
	{ "cyber1", "Cyber Warp 1" },
	{ "cyber2", "Cyber Warp 2" },
	{ "darkness", "Darkness" },
	{ "dayattheoffice", "A Day at the Office" },
	{ "deadreckoningadrift", "Dead Reckoning" },
	{ "dontgoadrift", "Don't Go" },
	{ "doortoutopia", "The Door to Utopia" },
	{ "drwhovortexlust", "Doctor Who and The Vortex of Lust" },
	{ "edithscats", "Edith's Cats" },
	{ "enc1", "Encounter 1 : Tim's Mom" },
	{ "enc2", "Encounter 2 : The Study Group" },
	{ "escapetofreedom", "Escape to Freedom" },
	{ "etnyadrift", "Escape to New York" },
	{ "farfromhome", "Far From Home" },
	{ "finedayforreaping", "A Fine Day for Reaping" },
	{ "frustratedinterviewee", "Frustrated Interviewee" },
	{ "gammagals", "The Gamma Gals" },
	{ "ghosttownadrift", "Ghost Town" },
	{ "gmylm", "Give Me Your Lunch Money" },
	{ "goldilocksadrift", "Goldilocks is a FOX!" },
	{ "halloweenhijinks", "Halloween Hijinks" },
	{ "houseofthedamned", "House of the Damned" },
	{ "hcw", "How to Conquer the World" },
	{ "jgrim", "Jonathan Grimshaw: Space Tourist" },
	{ "jimpond", "Jim Pond 1" },
	{ "lairofthevampire", "Lair of the Vampire" },
	{ "legacyofaprincess", "Legacy of a Princess" },
	{ "longjourneyhome", "The Long Journey Home" },
	{ "lostadrift", "Lost" },
	{ "magicshow", "The Magic Show" },
	{ "maroonedadrift", "Marooned" },
	{ "ml256", "Makers Local and the Transdimensional Margarita Blender" },
	{ "monsterage", "Monster Age: Trials of Dustorn" },
	{ "monsters", "Monsters" },
	{ "murdererhadleft", "What The Murderer Had Left" },
	{ "mustescape", "Must Escape!" },
	{ "neighboursfromhell", "Neighbours From Hell" },
	{ "onnafa", "Oh No, Not Another Fantasy Adventure" },
	{ "overtheedge", "Over the Edge" },
	{ "pathwayadrift", "Pathway to Destruction" },
	{ "pestilence", "Pestilence" },
	{ "phonebooth", "Pick Up the Phone Booth and Cry" },
	{ "professorvonwitt", "Professor von Witt's Fabulous Flying Machine" },
	{ "provenance", "Provenance" },
	{ "rachelbadday", "Rachel has a bad day" },
	{ "readinginmayadrift", "A Reading in May" },
	{ "sceneofthecrime", "Scene of the Crime" },
	{ "selmaswill", "Selma's Will" },
	{ "seymoursstoopidquest", "Seymour's Stoopid Quest" },
	{ "skybreak", "Skybreak!" },
	{ "sommeril", "Sommeril" },
	{ "stowaway", "Stowaway" },
	{ "takeone", "Take One" },
	{ "target", "Target" },
	{ "thepkgirl", "The PK Girl" },
	{ "viewisbetter", "The View Is Better Here" },
	{ "thelasthour", "The Last Hour" },
	{ "thevirtualhuman", "The Virtual Human" },
	{ "hellinahamper", "To Hell in a Hamper" },
	{ "toomuchexercise", "Too Much Exercise" },
	{ "topaz", "Topaz" },
	{ "twilight", "The Twilight" },
	{ "unauthorizedtermination", "Unauthorised Termination" },
	{ "unfortunately", "Back To Life... Unfortunately" },
	{ "unravelinggod", "Unraveling God" },
	{ "vague", "Vague" },
	{ "veteranknowledge" , "Veteran Knowledge" },
	{ "waxworx", "Wax Worx" },
	{ "campwindylake", "Camp Windy Lake" },
	{ "woodsaredark", "The Woods Are Dark" },
	{ "worstgame", "The Worst Game In The World... Ever!!!" },
	{ "wrecked", "Wrecked" },
	{ "xycanthus", "Doomed Xycanthus" },
	{ "yadfa", "Yet Another Damn Fantasy Adventure" },
	{ "yonastoundingcastle", "Yon Astounding Castle! of some sort" },

	// English Blorb Games
	{ "advbackyard", "An Adventurer's Backyard" },
	{ "allthroughthenight", "All Through the Night" },
	{ "darkhour", "The Dark Hour" },
	{ "jacarandajim", "Jacaranda Jim" },
	{ "jacd", "Just Another Christmas Day" },
	{ "noblecrook1", "Noble Crook, episode 1" },
	{ "noblecrook2", "Noble Crook, episode 2" },
	{ "noblecrook3", "Noble Crook, episode 3" },
	{ "noblecrook4", "Noble Crook, episode 4" },
	{ "sonofcamelot", "Son of Camelot" },

	// Italian Games
	{ "ilgolem", "Il Golem" },

	// Three Hour Games Competition: July 2004
	{ "3hgjailbreakbob", "Jailbreak Bob" },
	{ "3hgbriefcase", "Briefcase" },
	{ "3hgtheannihilationofthink", " Annihilation of Think.com" },
	{ "3hgshadricktravels", "Shadrick's Travels" },
	{ "3hglostsouls", "Lost Souls" },
	{ "3hgtheamulet", "Amulet" },
	{ "3hgzombiecow", "Zombie Cow" },

	// Three Hour Games Competition: November 2004
	{ "3hgsandl", " Snakes and Ladders" },
	{ "3hgconfession", "The Murder of Jack Morely" },
	{ "3hgveteran", " Veteran Experience" },
	{ "3hgburiedalive", " Buried Alive" },
	{ "3hgzac", "Zombies are Cool" },
	{ "3hgtogetyou", "We are coming to get you!" },

	// Game of the Year Competition: December 2004
	{ "goyshardsofmemory", " Shards of Memory" },
	{ "goypaint", "Paint!!!" },
	{ "goydragonshrine", "The Curse of DragonShrine" },
	{ "goydarkness", "Darkness" },

	// ADRIFT Intro Comp 2005
	{ "aicmurdermansion", "Murder Mansion" },
	{ "aicoutline", "Outline" },
	{ "aicsrs", "Silk Road Secrets: Samarkand to Lop Nor" },
	{ "aicescape", "Must Escape!" },
	{ "aicfinalquestion", "The Final Question" },
	{ "aicrift", "Rift" },
	{ "aicp2p", " Point 2 Point" },
	{ "aiczacksmackfoot", "Zack Smackfoot" },

	// ADRIFT Mini-Competition Summer 2003
	{ "amcthorn", "Thorn" },
	{ "amcneighbours", "Neighbours From Hell" },
	{ "amcmonsters", "Monsters" },
	{ "amcdiarystrip", "Diary of a Stripper" },

	// ADRIFT One-Hour Game Competition 1
	{ "1hgamonkeytoomany", "" },
	{ "1hgprincess1", "The Green Princess" },
	{ "1hg1hrgame", "TODO" },
	{ "1hgendgame", "The Game To End All Games" },
	{ "1hgchicken", "The Evil Chicken of Doom!" },
	{ "1hghauntedhouse", "The Haunted House of Hideous Horror" },
	{ "1hgfrog", "TODO" },
	{ "1hgmicrobewillie", "Microbe Willie vs. The Rat" },
	{ "1hgjasonvssalm", "Jason vs. Salm" },

	// ADRIFT One-Hour Game Competition 2
	{ "1hgpercy", "The Saga of Percy the Viking" },
	{ "1hgforum", "Forum" },
	{ "1hgdfu", "Dance Fever USA" },

	// ADRIFT One-Hour Game Competition 3
	{ "1hgforum2", "Forum 2" },
	{ "1hgcrm", "That Crazy Radioactive Monkey!" },
	{ "1hgasdfa", "A Short Damn Fantasy Adventure" },
	{ "1hgdemonhunter", "Apprentice of the Demonhunter" },
	{ "1hgcbn1", "The Revenge of Clueless Bob Newbie!" },
	{ "1hgcbn2", "The Revenge of Clueless Bob Newbie Part 2: This Time it's Personal" },
	{ "1hgticktick", "TODO" },
	{ "1hgpyramid", " The Pyramid of Hamaratum" },
	{ "1hgquesti", "Quest for Flesh" },
	{ "1hgshore", "The Farthest Shore" },
	{ "1hgsaffire", "Saffire" },
	{ "1hgecod2", " The Curse of the Revenge of the Ghost of the Evil Chicken of Doom... Returns!" },
	{ "1hgimagination", " Just My Imagination-Musings of a Child" },

	// ADRIFT One-Hour Game Competition 4
	{ "1hgcah", "Cruel and Hilarious Punishment!" },
	{ "1hgarghsgreatescape", "Argh's Great Escape" },
	{ "1hgshreddem", "Shred 'em" },
	{ "1hgagent4f", "Agent 4-F From Mars" },
	{ "1hgecod3", "An Evening with the Evil Chicken of Doom" },
	{ "1hgtrabula", " Get Treasure for Trabula" },
	{ "1hgwoof", "Woof" },
	{ "1hgundefined", "Undefined" },
	{ "1hgadriftmaze", "Adrift Maze" },
	{ "1hgicecream", "Ice Cream" },
	{ "1hgwreckage", "SE: Wreckage" },
	{ "1hgspam", "SPAM" },
	{ "1hgvagabond", "Vagabond" },
	{ "1hgthecatinthetree", "The Cat in the Tree" },
	{ "1hgtopaz", "Topaz" },
	{ "1hggoblinhunt", "Goblin Hunt" },

	// InsideADRIFT Summer Competition 2004
	{ "chooseyourown", "Choose Your Own..." },
	{ "darkhavenmystery", "The Mystery Of The Darkhaven Caves" },
	{ "shadricksunderground", "Shadrack's Underground Adventure" },
	{ "tickettonowhere", "Ticket to No Where" },
	{ "tearsofatoughman", "Tears of a tough man" },
	{ "theadriftproject", "The ADRIFT Project" },

	// InsideADRIFT Spring Competition 2005
	{ "iadcseaside", "A Day At The Seaside" },
	{ "iadcfrustrated", "Frustrated Interviewee" },
	{ "iadchub", "The House Husband" },
	{ "iadcprivateeye", "Private Eye" },
	{ "iadcblood", "Fire in the Blood" },

	{ nullptr, nullptr }
};

const PlainGameDescriptor ADRIFT5_GAME_LIST[] = {
	{ "axeofkolt", "The Axe of Kolt" },
	{ "castlecoris", "The Spectre of Castle Coris" },
	{ "fistoffire", "Die Feuerfaust - The Fist of Fire" },
	{ "magneticmoon", "Magnetic Moon" },
	{ "starshipquest", "Starship Quest" },
	{ "sixsilverbullets", "Six Silver Bullets" },
	{ "tingalan", "Tingalan" },

	{ nullptr, nullptr }
};


const GlkDetectionEntry ADRIFT_GAMES[] = {
	// English Games
	DT_ENTRY0("3monkeys", "dec34c282511b1eadfe231dbbb49f625", 79286),
	DT_ENTRY0("akron", "2461ceeef3ef1eac15a2051a549b4402", 22258),
	DT_ENTRY0("albridgemanor", "268003e454c4ade042d593af8397a490", 31353),
	DT_ENTRY0("asdfa", "06173b47b4e88c0a494d2263666ad375", 27733),
	DT_ENTRY0("awalkatdusk", "f2e8aca65a17d41804709ee5a420ef36", 20725),
	DT_ENTRY0("bariscebik", "d63d310e2b2fe4ab7ea6bc74c136b8e0", 14634),
	DT_ENTRY0("barneysproblem", "3fe45352a6c0d7bb0bdbf4f9c08afba7", 66055),
	DT_ENTRY0("beanstalk", "fc9c17c355894948dc2e72c66e8d9963", 9837),
	DT_ENTRY0("beerisntenough", "98248a8e65a36f6d0fb98111255a85dc", 14070),
	DT_ENTRY0("caveofwonders", "f55f4e63886eb09e1f89bea547574919", 85471),
	DT_ENTRY0("circusmenagerie", "3814400e3abb3e6340f055d5ec883c46", 117017),
	DT_ENTRY0("cityInfear", "a6f625f184fa848ee6746e9aa6c94915", 248088),
	DT_ENTRY0("coloromcadrift", "6b8d8f115a89c99f785156a9b5802bc2", 12681),
	DT_ENTRY0("compendiumendgame", "ad76afa2fcdb8c9e2272a115cce6bd4f", 4092),
	DT_ENTRY0("compendiumforum1", "273b73bf6262f603bee198238bc02196", 9128),
	DT_ENTRY0("compendiumforum2", "b4af4a085707f221dbabd2906e411d29", 11172),
	DT_ENTRY0("compendiumimagi", "3cfd6a4dd7ce552afdc52d0923e079cd", 9273),
	DT_ENTRY0("compendiumsaffire", "d8a0413cdb908ba2ac400fe6a880d954", 7486),
	DT_ENTRY0("compendiumtopaz", "5f91c9cd4391b6e44c2c052698d01118", 4866),
	DT_ENTRY0("cowboyblues", "23eabfd5db63ded776dd0323d2abe7ea", 111835),
	DT_ENTRY0("crawlersdelight", "9da704541689f95b3636ad729cfda5f4", 18705),
	DT_ENTRY0("crimeadventure", "965eafa4579caa3bb8382a07b5e1771e", 15073),
	DT_ENTRY0("cursed", "62d2e05e62f1137b25a61bbb46154729", 487990),
	DT_ENTRY1("cursed", "Hints", "4ad13bf274af97ebbfe47c4a852b3a46", 36793),
	DT_ENTRY0("cyber1", "b53f7a302edad4d9e2a3310dc1c66e72", 1786),
	DT_ENTRY0("cyber2", "535c161818ee238e112b1d00bd082b26", 3384),
	DT_ENTRY0("darkness", "af7b3452fdb40f121f2a6be2fa81e4f8", 25203),
	DT_ENTRY0("darkness", "f02a5be2f9a2fd59ef10f3d16157ee27", 25238),
	DT_ENTRY0("dayattheoffice", "2d908b5448c6fd2dbc98a7f9d2dda9df", 13824),
	DT_ENTRY0("deadreckoningadrift", "c49f787df29d6a73e77a0a33386614d3", 82279),
	DT_ENTRY0("dontgoadrift", "f192abcc4e1f04787a7f1fde2ad05385", 13789),
	DT_ENTRY0("doortoutopia", "9e727b8ad5b9c62d712d4a302083f6d8", 15432),
	DT_ENTRY0("drwhovortexlust", "e2b76d720096fb6427927456fc80c5a2", 166913),
	DT_ENTRY0("edithscats", "0c6bbf3d292a427182458bd5332f2348", 18316),
	DT_ENTRY0("enc1", "d297f4376e5ba5b05985ca12a60d8671", 101668),
	DT_ENTRY0("enc2", "4bd8ebd78a0a227510f62f2074e60ee5", 120335),
	DT_ENTRY0("escapetofreedom", "a7edcb289eb8177a143a96912ec55393", 18171),
	DT_ENTRY0("etnyadrift", "990c0e2390d197ebcae1109e5534c433", 59583),
	DT_ENTRY0("farfromhome", "b84df75a6ff647006f1f2783bde114f7", 42118),
	DT_ENTRY0("finedayforreaping", "3eb40409db8034e4d2d4ff77e406caba", 96819),
	DT_ENTRY0("frustratedinterviewee", "86dcc06a9edcd967183d5448165fd92e", 50578),
	DT_ENTRY0("gammagals", "db813e61ded3e7f642d437ef1e77d0b3", 277834),
	DT_ENTRY0("ghosttownadrift", "60c11a24853dec5e6a292914c6b4f7f2", 30205645),
	DT_ENTRY0("gmylm", "67b61f16ca39d8abc9eb770385ef0e50", 15194757),
	DT_ENTRY0("goldilocksadrift", "12e596065ae94d3f86a67acb4de2aae7", 56539),
	DT_ENTRY0("halloweenhijinks", "14694f7b9cef4baeb0f958500119d2ff", 68052),
	DT_ENTRY0("houseofthedamned", "3b25e7d9e1fd6ff2206ab0555c19e899", 35974),
	DT_ENTRY0("hcw", "327dfec3d8f0b63ff5b05a552ee946ca", 82659),
	DT_ENTRY0("jgrim", "f9c1b2064edeed5675a3466baeebdefd", 96713),
	DT_ENTRY0("jimpond", "eef1aed7efbd36283b7d39b1514b3933", 50551),
	DT_ENTRY0("lairofthevampire", "33dce0c1c3f9ed2beded0bab931e8d78", 133689),
	DT_ENTRY0("legacyofaprincess", "9db58796cc772b662743e9a65ae1a876", 37003),
	DT_ENTRY0("longjourneyhome", "c16c9e02c4a3248b25cb4f6c1ada0646", 59124),
	DT_ENTRY0("lostadrift", "904b80ebf94df890ad7c26e454b6a612", 42596),
	DT_ENTRY0("magicshow", "29fd03636eec8ffdaea26251a6f2444c", 104201),
	DT_ENTRY0("maroonedadrift", "5e3e0435c98a0a915a98d5b1b080522c", 50516),
	DT_ENTRY0("ml256", "68862c0031f1497e32ce26e0654bb07f", 32397),
	DT_ENTRY0("monsterage", "00b8e89e5e7421a0ea017707466efe17", 16486),
	DT_ENTRY0("monsters", "fde7be359d37c9229fec77244fc38a54", 17168),
	DT_ENTRY0("murdererhadleft", "f95c63f90f28061a15dbe3bdf0cd4048", 13425),
	DT_ENTRY0("mustescape", "e58fd08918fa3d5a38f424f79595fb4e", 17497),
	DT_ENTRY0("neighboursfromhell", "686d8c0995aa085fb056c6a5bd402911", 13520),
	DT_ENTRY0("onnafa", "da4c0cc6fe34a08047de54abc66de038", 167339),
	DT_ENTRY0("overtheedge", "9c7bfb9252ac80597ef125c95d58a54b", 14771),
	DT_ENTRY1("overtheedge", "Gargoyle", "a6bb780b231c31a9e01455d0424acdca", 14937),
	DT_ENTRY0("pathwayadrift", "bcf1f61393c1b8123a98ee4879ffd8a6", 20896),
	DT_ENTRY0("pestilence", "2d5ff3fe2d866150689749087d5cee3d", 29021),
	DT_ENTRY0("phonebooth", "25f4977d7b8df28322df945809826f43", 1372),
	DT_ENTRY0("professorvonwitt", "570e7d90fa42a199ee0cba44a3c95d6b", 31253),
	DT_ENTRY0("provenance", "49ebc8ad4f681eb727c8a2c90c3c6dd7", 532278),
	DT_ENTRY0("rachelbadday", "8940ad383d78b2ba5d1fa9738bf1ea98", 98539),
	DT_ENTRY0("readinginmayadrift", "ba8a12e79520234e31622f141c99cafd", 13434),
	DT_ENTRY0("sceneofthecrime", "a8a094b145523c622e65ab6897727f2d", 68054),
	DT_ENTRY0("selmaswill", "23b7fb4bf976135b8be2740e1fbfb83b", 19559),
	DT_ENTRY0("seymoursstoopidquest", "61f4e155b958c1c652f12ed4216ee10d", 12623),
	DT_ENTRY0("skybreak", "a5322754939d41aaf46c69c0a1100df8", 1191014),
	DT_ENTRY1("sommeril", "Special Edition", "1870db33af741482bb5d93ab54200e7c", 22839),
	DT_ENTRY0("stowaway", "6877c3079978ecf74e72b792005fca32", 14231),
	DT_ENTRY0("takeone", "114caad9308b7adf9e15267a11f12632", 9547),
	DT_ENTRY0("target", "c6bbbae5ea3f6407a55435cb4e0c20c3", 43024),
	DT_ENTRY0("thepkgirl", "c9282cf8f91ebfe5a93b136f56d7a1aa", 1645515),
	DT_ENTRY1("thepkgirl", "R3", "196773493b04cf42c92f6d1e85039469", 1645174),
	DT_ENTRY0("viewisbetter", "9334adc4ccd03ceb1684ee56fdb07ab9", 7720),
	DT_ENTRY0("thelasthour", "786d39eb223a64fe836f87b032b60ae1", 10658),
	DT_ENTRY0("thevirtualhuman", "b894e19638a65e03d28b696a761d23a3", 6168),
	DT_ENTRY0("tickettonowhere", "8c549a6bdc6a5a4895412516be8dce25", 65502),
	DT_ENTRY0("hellinahamper", "810247344fddc9a812bf6283b4826093", 80183),
	DT_ENTRY0("toomuchexercise", "aebb58d94f632c4232bee72b9a1b5e25", 6248),
	DT_ENTRY0("topaz", "0777a97e473b41ae04ab825556748e8d", 5980),
	DT_ENTRY0("twilight", "ab5ddd461c1fb2065d26fcfdf4baa5aa", 71377),
	DT_ENTRY0("unauthorizedtermination", "a7bba6890fe2945bb192c67e395516ac", 107335),
	DT_ENTRY0("unfortunately", "8493ffd877273ef95c88c8f433126473", 55438),
	DT_ENTRY0("unravelinggod", "2d43577a756ddc6b561f447e67270ac4", 45000),
	DT_ENTRY0("vague", "0f5ef8616c668b9c3de078a77b26a512", 122681),
	DT_ENTRY0("veteranknowledge", "409d3d59cb767dc8b812e16db78b2e47", 52248),
	DT_ENTRY0("waxworx", "a10a1e404ae77fd968b4427a13bd7034", 38414),
	DT_ENTRY0("campwindylake", "33ea1f1fea9998472e9c352ea6e1c1d8", 114698),
	DT_ENTRY0("woodsaredark", "4acae86746292076b90fa6fa73ea76d0", 71216),
	DT_ENTRY0("worstgame", "8789e209419529ac22d2be4774620c78", 9858),
	DT_ENTRY0("wrecked", "b1c6c4ef73025fbbe99b612a72e3186a", 88550),
	DT_ENTRY0("xycanthus", "5637cb8221087c5cca094b13138655c3", 116322),
	DT_ENTRY0("yadfa", "5ee5906fc4b889d126cdfd83bd883a43", 77880),
	DT_ENTRY0("yonastoundingcastle", "baf2c0423903a3104f67d3a19bde43df", 2320881),

	// English Blorb Games
	DT_ENTRY0("advbackyard", "73b1171283ffa2b3cf6fb72a0ec31a35", 48990),
	DT_ENTRY0("allthroughthenight", "89dcb84d74f70945780ed9e719401301", 36345976),
	DT_ENTRY0("darkhour", "75bc2ac4d561ad2a2143bd90a19d1b46", 35184),
	DT_ENTRY0("jacarandajim", "78e0de68825eea88995d7490adb6c062", 79146),
	DT_ENTRY0("jacd", "28ae6ee45f1d2980765d3e9645f0b269", 54256),
	DT_ENTRY0("noblecrook1", "334c706261ab79b783403ad3315980fd", 23966),
	DT_ENTRY0("noblecrook2", "9d460ed691ad1b45d5d7c19f1ebf61d9", 22446),
	DT_ENTRY0("noblecrook3", "c9d4a1ba7d5c1625f2d4cad0f067275a", 22088),
	DT_ENTRY0("noblecrook4", "cb26241f8dba982bb0cd357ebd8fac45", 20606),
	DT_ENTRY0("sonofcamelot", "7b3e9e45edc75202b3f586d0ca8dddde", 474380),
	DT_ENTRY0("spacedetective1", "170517dae3a195f37d876f5645637953", 34044),
	DT_ENTRY0("spacedetective2", "1e12b19d8777173cbf9866b3dd2fc2f0", 32632),
	DT_ENTRY0("spacedetective3", "fc9fb75830d7f59bf932896152b1e759", 30560),
	DT_ENTRY0("spacedetective4", "86fbcf204c6b0982ef2394a60bb8df3e", 32184),
	DT_ENTRY0("spacedetective5", "f1bf863cce436a5ef17e64237803571a", 32676),
	DT_ENTRY0("spacedetective6", "6035c865f7a4831e4539b7a2ff2ab711", 31922),
	DT_ENTRY0("spacedetective7", "2044086edd9b2288ff2279b3824dfcef", 30066),

#ifndef RELEASE_BUILD
	// Unsupported English ADRIFT 5 games
	DT_ENTRY0("axeofkolt", "353a29250cc5e463876350e3d232fd22", 931390),
	DT_ENTRY0("castlecoris", "15f21891cf889e24d21f644b07d249ce", 573856),
	DT_ENTRY0("fistoffire", "3b76af72ce4876dfea4da23ab0b7f641", 567450),
	DT_ENTRY0("magneticmoon", "26a11345dd49413dea2c1b4f7fd2b3fd", 488582),
	DT_ENTRY0("starshipquest", "02fb2460c61c8d82c31fa1d763e63a7e", 449736),
	DT_ENTRY0("sixsilverbullets", "e0f6170b5cbde7005609e68d8a25ddd3", 376934),
	DT_ENTRY0("tingalan", "08c9ecd86664b62335d3852a2bb3d201", 259969026),
#endif

	// Italian Games
	DT_ENTRYL0("ilgolem", Common::IT_ITA, "7ab97545be9c30457d0c9e748ab7885f", 6372213),

	// Three Hour Games Competition: July 2004
	DT_ENTRY0("3hgjailbreakbob", "c2e909cf2ddda863d2eb613c8273ee37", 31064),
	DT_ENTRY0("3hgbriefcase", "f1c899773faf027d9cbb50078a6667c3", 10872),
	DT_ENTRY0("3hgtheannihilationofthink", "22743eceafe94bf60ab5a3e43241883e", 6303),
	DT_ENTRY0("3hgshadricktravels", "33036e2b4c11a36d28e6532051e9f05b", 7456),
	DT_ENTRY0("3hglostsouls", "3083e21a3bd6766ea5fdf3166e8bd4d8", 16695),
	DT_ENTRY0("3hgtheamulet", "7c08e1f03763ad757dc39f5df37a28b7", 5585),
	DT_ENTRY0("3hgzombiecow", "f0fe890f9679b158f9a22ed9f78179e6", 5193),

	// Three Hour Games Competition: November 2004
	DT_ENTRY0("3hgsandl", "73a1e0b4451690e76d306136ec30754b", 27995),
	DT_ENTRY0("3hgconfession", "718faa6f6e72fcd4b95ebcb1601be7bd", 203757),
	DT_ENTRY0("3hgveteran", "65d5fc95b59f24e0e4997f62bf592ba3", 12043),
	DT_ENTRY0("3hgburiedalive", "9e9a71babf3012461205dc7aa2cd2a3f", 11808),
	DT_ENTRY0("3hgzac", "85d95b8a6a962222d15724d42740f378", 13692),
	DT_ENTRY0("3hgtogetyou", "388b5596945067ed3b92e6282e670825", 4496),

	// Game of the Year Competition: December 2004
	DT_ENTRY0("goyshardsofmemory", "7e9cb5002fa22a25000b3478c826413d", 118610),
	DT_ENTRY0("goyshardsofmemory", "9b75834354aed4b148d4ec2b8cdd66ae", 118604),
	DT_ENTRY0("waxworx", "94aaf6625541238867b079bfe475f146", 38471),
	DT_ENTRY0("goypaint", "1183921d034d3614ec277e18d9112b69", 76493),
	DT_ENTRY0("goydragonshrine", "604250027cfd8cfd1fb0d231085e40e5", 636932),
	DT_ENTRY0("goydarkness", "6047363499d5eb666c341139d0eb532a", 24847),

	// ADRIFT Intro Comp 2005
	DT_ENTRY0("aicmurdermansion", "07367e1f546f80e64b44923f474f5906", 6793),
	DT_ENTRY0("aicoutline", "b86de4feca73e0ea49c324a2181b8b8f", 5724),
	DT_ENTRY0("aicsrs", "f1243c19976246c2c319fba5be6f9d13", 157833),
	DT_ENTRY0("aicescape", "c5a13fa9059ebc2904eea95e1c6b790a", 3964),
	DT_ENTRY0("aicfinalquestion", "1f1ede210336b6b8bd8e28eda2d00e0c", 15555),
	DT_ENTRY0("aicrift", "b4dc3233be12767837bd596789379473", 2606),
	DT_ENTRY0("aicp2p", "3b3386f5f0f844c3a9eb47ba2596135a", 6491),
	DT_ENTRY0("aiczacksmackfoot", "a8d2d303c6e5f698d40714c3d5e241e2", 6374),

	// ADRIFT Mini-Competition Summer 2003
	DT_ENTRY0("amcthorn", "0d02030e9ac6563cda759a20ae1fc06b", 13887),
	DT_ENTRY0("amcneighbours", "afdf85651706a4568762565b2c24aa8d", 13333),
	DT_ENTRY0("amcmonsters", "02a042f7277dfc9789bd6ffee1e8db08", 9628),
	DT_ENTRY0("amcdiarystrip", "0e04238a2b49220c284ce9c8723abd77", 43334),

	// ADRIFT One-Hour Game Competition 1
	DT_ENTRY0("1hgamonkeytoomany", "c9d719261d0f760f95685899aa80bbf5", 5016),
	DT_ENTRY0("1hgprincess1", "d745a3f2c4dc40fb10f25d1539a2a1e7", 7181),
	DT_ENTRY0("1hg1hrgame", "63e9594bf2b4e7cf5bf75dd9cc1353dc", 2978),
	DT_ENTRY0("1hgendgame", "6036e5654786f841ca59f8f7e95276be", 4063),
	DT_ENTRY0("1hgchicken", "40f0f52f2eeb029516bca3e01e6ceac0", 5249),
	DT_ENTRY0("1hghauntedhouse", "cff22c2f2af619ac56e075e95385e600", 6541),
	DT_ENTRY0("1hgfrog", "3a8f3a0a6b4efd4787e6d338e4b31cea", 2089),
	DT_ENTRY0("1hgmicrobewillie", "14fe6ac0db4943fc9951610f5d0e0428", 3538),
	DT_ENTRY0("1hgjasonvssalm", "70d60774dc777918645d7f3359f534cf", 2947),

	// ADRIFT One-Hour Game Competition 2
	DT_ENTRY0("1hgpercy", "e995d3a23005914eb92836f141ebe1c4", 5965),
	DT_ENTRY0("1hgforum", "4df8a13d8823aca26207fb459e98dd8b", 9005),
	DT_ENTRY0("1hgdfu", "e70e0bdfd9ffa0f20c2bc682eeb2f1ab", 16443),

	// ADRIFT One-Hour Game Competition 3
	DT_ENTRY0("1hgforum2", "5a534ac4e39a319022d145094c46930a", 11185),
	DT_ENTRY0("1hgcrm", "d97d1ff8f01a61fb477b76df65c77795", 15432),
	DT_ENTRY0("1hgasdfa", "fccb2fb890d554263d5f55bc02220ab8", 6440),
	DT_ENTRY0("1hgdemonhunter", "ca37aaf35fb15a40a7f5f8caa1475112", 4169),
	DT_ENTRY0("1hgcbn1", "9e27ab68a1f37f5f7591b362c4888526", 7577),
	DT_ENTRY0("1hgcbn2", "983eacedabebf7bbd8fed72ed399bba4", 6137),
	DT_ENTRY0("1hgticktick", "5b952d75e3e46a71334419c78dc6ff51", 2055),
	DT_ENTRY0("1hgpyramid", "a198f4ca146fed3094bf5fd70d2eefdd", 1953),
	DT_ENTRY0("1hgquesti", "c7d6058e8172ff5d4a974fe159aa9cb7", 2544),
	DT_ENTRY0("1hgshore", "ef0b6074e47f55b7dff868e5dbd0c0cf", 4354),
	DT_ENTRY0("1hgsaffire", "d3eb89cf10d11a42df2df8f7bdb1505a", 7489),
	DT_ENTRY0("1hgecod2", "ff55438090db248964fd8944f3a2da9f", 17584),
	DT_ENTRY0("1hgimagination", "91923aeec3b7a8c55a15bc6dd240f7fb", 3257),

	// ADRIFT One-Hour Game Competition 4
	DT_ENTRY0("1hgcah", "1945c5584b775b22c9b329915629eb44", 9808),
	DT_ENTRY0("1hgarghsgreatescape", "0de27ef52495de415637d78e393c612f", 3608),
	DT_ENTRY0("1hgshreddem", "426885b0a8a9aade134307fad1693512", 1209),
	DT_ENTRY0("1hgagent4f", "407cc79b128aefdae4fdabb4d7a661e4", 3703),
	DT_ENTRY0("1hgecod3", "bd783057274a626868b052915bad2a2c", 10870),
	DT_ENTRY0("1hgtrabula", "54fd39f8d86b4e1726970073342057da", 2224),
	DT_ENTRY0("1hgwoof", "f66cbdcf9f7c6a3b30b0c7ca95bdea46", 1712),
	DT_ENTRY0("1hgundefined", "cf5172b53c8e3ef3a8739a31b78cccc2", 2920),
	DT_ENTRY0("1hgadriftmaze", "017d681d45402c2d75ae70c28d2b467f", 2224),
	DT_ENTRY0("1hgicecream", "f68b7ec0959459cd1ee3c741660010c6", 5039),
	DT_ENTRY0("1hgwreckage", "822b988245ff40c6f490edc046921fcf", 4487),
	DT_ENTRY0("1hgspam", "2ffc74d57936add0cdc06d83924f0f15", 2807),
	DT_ENTRY0("1hgvagabond", "ae8545f6506fe3a49e27c96721196dce", 4762),
	DT_ENTRY0("1hgthecatinthetree", "f67640c2ed3b621272ebbea06a5fd467", 6174),
	DT_ENTRY0("1hgtopaz", "7d4beb159bf3876f761bbac911395d05", 4839),
	DT_ENTRY0("1hggoblinhunt", "5e36c63feebf77cfba30757042a9fafa", 10891),

	// InsideADRIFT Spring Competition 2004
	DT_ENTRY0("waxworx", "4fa14c7944506f4e0142cb865738d0b0", 37542),
	DT_ENTRY0("sommeril", "9e8ee4cbd4ecab3ab51c702f4e0277e1", 400940),

	// InsideADRIFT Summer Competition 2004
	DT_ENTRY0("chooseyourown", "774ae5ba4fea61da0898069df4ed1c81", 54869),
	DT_ENTRY0("darkhavenmystery", "1027e9e3dba08e0fd2796e0e934be855", 27826),
	DT_ENTRY0("shadricksunderground", "a9e1e59b37976c6c0fcc6aee69978afe", 113385),
	DT_ENTRY0("tickettonowhere", "a69b0fa1b33d8e0ab6504d37615e6a3c", 65083),
	DT_ENTRY0("tearsofatoughman", "b440d0b4b18a93537803750f31e838b7", 9436),
	DT_ENTRY0("theadriftproject", "18a4db8af27fd98751504ee3e45089e3", 231594),

	// InsideADRIFT Spring Competition 2005
	DT_ENTRY0("iadcseaside", "271c64455f4abb3dee35c3902e5cdfc1", 279480),
	DT_ENTRY0("iadcfrustrated", "1c218d4fcbbfa11271fb4d0003df7a6a", 210267),
	DT_ENTRY0("iadchub", "c3224b3d6c55997056ea195ba4ebb60a", 24610),
	DT_ENTRY0("iadcprivateeye", "3076b9f11acf2b1e377a31cd1a11c323", 236323),
	DT_ENTRY0("iadcblood", "775404faa0d7973c1df537bd423a45d3", 100201),

	DT_END_MARKER
};

} // End of namespace Adrift
} // End of namespace Glk
