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

#ifndef DIRECTOR_DETECTION_PATHS_H
#define DIRECTOR_DETECTION_PATHS_H

namespace Director {

const char *directoryGlobs[] = {
	"install",
	"win_data",						// L-ZONE
	"data",
	"gadget",						// Gadget
	"vnc",
	"vnc2",							// Virtual Nightclub
	"program",						// Arc Media products
	"demodata",						// Edmark demos
	"media",						// Gundam
	"mvm",							// Master of the Elements
	"module_1",
	"module_2",						// Takeru
	"simpdata",
	"simpsons",						// Simpsons Cartoon Studio
	"dane",							// Polish Kontyngent '99 catalog
	"datas",						// O!KAY! subscription from Domino Verlag
	"daten",						// Bibi Blocksberg games (German)
	"source",						// Grand Prix 2006 directory
	"data_k",
	"data_1",
	"data_2",						// JumpStart World
	"numbers",						// JumpStart Learning Games
	"xtras",
	"files",						// Babar
	"swanlake",						// Barbie Swan Lake
	"assets",						// Barbie Nail Designer
	"fjpc",							// Forestia Jr
	"movies",						// Mulle Meck (Gary Gadget) series
	"winroot",
	"setup",
	"program files",
	"tohd",
	"terzio",
	"flugzeuge bauen",
	"willyhau",
	"willyraumschiff",				// Willy Werkel (Mulle Meck) series
	"demos",						// Headbone samplers
	"blender",						// Blender CD-ROM magazines
	"bilder",
	"elk1",							// Blinky Bill Kindergarten
	"endsoung",
	"gerhelp",
	"gervideo",						// GGMD German
	"music",
	"solos",
	"duthelp",
	"dutvideo",
	"endsound",						// GGMD Dutch
	"macos9",
	"macosx",						// Sethi 3
	"kripos",
	"strisser",
	"polis",
	"poliisi",
	"cd_data",						// Polis
	"fusion macintosh folder",		// Fusion
	"oslo",							// Oslo's Tools & Gadgets
	"ojo2000",
	"stuff",						// Abrapalabra
	"tooned",						// Stay Tooned
	"p2data",
	"p2media",
	"p3media",
	"annat",						// Pettson & Findus
	"bb",							// Blinky Bill
	"fctdata",
	"fct",					   		// Felix the Cat
	"dayfiles",
	"xmasfun",						// Fisher-Price Xmas Fun
	"95instal",						// Fisher-Price Little People
	"01",					   		// Doing it in C++
	"freading",
	"fsetup",						// My First Reading Words (Eureka)
	"english",
	"deutsch",
	"francais",						// Rename from Français
	"italiano",						// Virtual Tourism Paris
	"popup",						// Pop Up Computer
	"alpha",						// Interactive Alphabet
	"technik",						// Glasklar Technology Interactive
	"badday",
	"copy to hdd",					// Rename from HDDにコピーしてください
	"bdm",
	"dkyasp",						// Bad Day on the Midway Japanese
	"win_demo",
	"file",
	"mfile",
	"movie",						// Teichiku demos
	"install_mac",
	"install_win",
	"copyto_hd",					// Incarnatia
	"alcohol 101",
	"audio",
	"print",
	"vids",							// Alcohol 101
	"dir_data",						// Garage
	"cd extra",
	"catalog",						// Sarah McLachlan ECD (NettMedia)
	"hd",							// Grammar Playtime
	"scr",
	"scr_mov",						// Cardcaptor Sakura screen savers
	"dicfiles",						// Time Blazers series
	"docs",							// Enhanced CDs
	"anim",							// 1 vs 100 (Mindscape)
	"lingo expo",					// D4 sample movies
	"main",							// Meet MediaBand
	"toinstall",					// Jönssonligan series
	"pingucd",
	"instdata",						// Pingu
	"bellinin bikinit",				// Bellini 3 (Finnish)
	"put me on your desktop",		// Betty Page
	"abc",							// Alphabet Train
	"123",							// Ktar Alarkam
	"vlaams",
	"ned",							// I Spy School Days (Dutch)
	"fl1",
	"flipper",						// Flipper games (Yoram Gross)
	"cipdemo",
	"cipher",						// Jewels of the Oracle
	"gems",							// Gems of Darkness / Jewels II
	"corel cd home",
	"the cassandra galleries",		// Cassandra Galleries
	"knights",						// Nikolai's Knights
	"nemcina",						// Německá gramatika
	"hdfiles",
	"creator",						// Get a Head series
	"32bit",
	"sd",
	"mac install",
	"slam dunk typing",
	"typing",						// Slam Dunk Typing
	"ceremony",
	"copytohd",						// Ceremony of Innocence
	"xn--t8jo8k",					// Perfect Blue screen savers
	"maccurio",
	"wincurio",						// Curio City
	"a.d.a.m. the inside story 1.1", // ADAM
	"nine month miracle 1.0",		// Nine Month Miracle
	"_A.D.A.M. Data",				// ADAM Software Products
	"_A.D.A.M.  Data",				// ADAM Software Products with more space
	"_setup",
	"bin",
	"adam resources",				// ADAM Software Products
	"material",						// NY Yankees Yearbook
	"NAV",
	"InsMilo",						// Milo and the Magical Stones
	"dir",
	"CHRISTMAS PRESENT",			// Christmas Present
	"Subotnick Hummingbirds",		// All My Hummingbirds Have Alibis
	"Silly Noisy House Folder",		// A Silly Noisy House
	"Treasures of the AMNH",		// Treasures of the American Museum of Natural History (Mac)
	"TAMNH",						// Treasures of the American Museum of Natural History (Win)
	"xn--Sacred and Secular -ere",	// Sacred and Secular (Mac) ("Sacred and Secular ƒ")
	"SACRED",						// Sacred and Secular (Win)
	"Dracula - Truth and Terror",	// Dracula: Truth and Terror (Mac)
	"DRACULA",						// Dracula: Truth and Terror (Win)
	"The Louvre for Kids",			// The Louvre Museum (Mac)
	"LOUVRE",						// The Louvre Museum (Win)
	"Trinity",						// The Day After Trinity
	"Puppet Motel Folder",			// Puppet Motel (Mac)
	"PUPPET",						// Puppet Motel (Win)
	"Daedalus Max",					// The Daedalus Encounter
	0
};

} // End of namespace Director

#endif
