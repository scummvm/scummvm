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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/advancedDetector.h"
#include "base/plugins.h"

#include "sci/sci.h"
#include "sci/exereader.h"
#include "sci/include/versions.h"

namespace Sci {

// Titles of the games
static const PlainGameDescriptor SciGameTitles[] = {
	{"sci",             "Sierra SCI Game"},
	{"sci-fanmade",     "Fanmade SCI Game"},
	{"astrochicken",    "Astro Chicken"},
	{"christmas1988",   "Christmas Card 1988"},
	{"christmas1990",   "Christmas Card 1990: The Seasoned Professional"},
	{"christmas1992",   "Christmas Card 1992"},
	{"castlebrain",     "Castle of Dr. Brain"},
	{"iceman",          "Codename: Iceman"},
	{"camelot",         "Conquests of Camelot: King Arthur, Quest for the Grail"},
	{"longbow",         "Conquests of the Longbow: The Adventures of Robin Hood"},
	{"ecoquest",        "EcoQuest: The Search for Cetus"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},
	{"gk2",             "The Beast Within: A Gabriel Knight Mystery"},
	{"hoyle1",          "Hoyle's Official Book of Games: Volume 1"},
	{"hoyle2",          "Hoyle's Official Book of Games: Volume 2"},
	{"hoyle3",          "Hoyle's Official Book of Games: Volume 3"},
	{"jones",           "Jones in the Fast Lane"},
	{"kq1sci",          "King's Quest I: Quest for the Crown"},
	{"kq4sci",          "King's Quest IV: The Perils of Rosella"},
	{"kq5",             "King's Quest V: Absence Makes the Heart Go Yonder"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"kq7",             "King's Quest VII: The Princeless Bride"},
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"lsl1sci",         "Leisure Suit Larry in the Land of the Lounge Lizards"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"lsl7",            "Leisure Suit Larry 7: Love for Sail!"},
	{"lighthouse",      "Lighthouse: The Dark Being"},
	//{"fairytales",      "Mixed-up Fairy Tales"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	//{"pepper",          "Pepper's Adventure in Time"},
	{"phantasmagoria",  "Phantasmagoria"},
	{"phantasmagoria2", "Phantasmagoria II: A Puzzle of Flesh"},
	{"pq1sci",          "Police Quest: In Pursuit of the Death Angel"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"pq3",             "Police Quest III: The Kindred"},
	{"pq4",             "Police Quest IV: Open Season"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},
	{"rama",            "RAMA"},
	{"shivers",         "Shivers"},
	//{"shivers2",        "Shivers II: Harvest of Souls"},
	{"sq1sci",          "Space Quest I: The Sarien Encounter"},
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"sq6",             "Space Quest 6: The Spinal Frontier"},
	{"islandbrain",     "The Island of Dr. Brain"},
	{"torin",           "Torin's Passage"},
	{0, 0}
};

#define FANMADE_LV(name, resMapMd5, resMapSize, resMd5, resSize, lang, ver) \
	{{"sci-fanmade", name, { \
		{"resource.map", 0, resMapMd5, resMapSize}, \
		{"resource.001", 0, resMd5, resSize}, \
		{NULL, 0, NULL, 0}}, lang, Common::kPlatformPC, 0}, \
		{}, \
		SCI_VERSION_AUTODETECT, \
		ver \
	}
	
#define FANMADE_L(name, resMapMd5, resMapSize, resMd5, resSize, lang) FANMADE_LV(name, resMapMd5, resMapSize, resMd5, resSize, lang, SCI_VERSION(0, 0, 629))
#define FANMADE_V(name, resMapMd5, resMapSize, resMd5, resSize, ver) FANMADE_LV(name, resMapMd5, resMapSize, resMd5, resSize, Common::EN_ANY, ver)
#define FANMADE(name, resMapMd5, resMapSize, resMd5, resSize) FANMADE_LV(name, resMapMd5, resMapSize, resMd5, resSize, Common::EN_ANY, SCI_VERSION(0, 0, 629))


/*
	// Missing - from FreeSCI
	{ 0xE4A3234D, SCI_VERSION(0, 000, 506), "Fun Seekers Guide v1.02"},
	{ 0x1EACB959, SCI_VERSION(0, 000, 566), "HQ v1.000-5.25"},
	{ 0x2BEAF5E7, SCI_VERSION(0, 000, 566), "HQ v1.001-5.25"},
	{ 0x4447B28D, SCI_VERSION(1, 000, 72),  "Trial v1.105"},
*/

// Game descriptions
static const struct SciGameDescription SciGameDescriptions[] = {
	// Astro Chicken DOS
	{{"astrochicken", "", {
		{"resource.map", 0, "f3d1be7752d30ba60614533d531e2e98", 474},
		{"resource.001", 0, "6fd05926c2199af0af6f72f90d0d7260", 126895},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 453)
	},

	// Castle of Dr. Brain - English Amiga (from www.back2roots.org)
	{{"castlebrain", "", {
		{"resource.map", 0, "9f9fb826aa7e944b95eadbf568244a68", 2766},
		{"resource.000", 0, "0efa8409c43d42b32642f96652d3230d", 314773},
		{"resource.001", 0, "3fb02ce493f6eacdcc3713851024f80e", 559540},
		{"resource.002", 0, "d226d7d3b4f77c4a566913fc310487fc", 792380},
		{"resource.003", 0, "d226d7d3b4f77c4a566913fc310487fc", 464348},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},

	// Castle of Dr. Brain - German Amiga (from www.back2roots.org)
	{{"castlebrain", "", {
		{"resource.map", 0, "8e60424682db52a982bcc3535a7e86f3", 2796},
		{"resource.000", 0, "0efa8409c43d42b32642f96652d3230d", 332468},
		{"resource.001", 0, "4e0836fadc324316c1a418125709ba45", 569057},
		{"resource.002", 0, "85e51acb5f9c539d66e3c8fe40e17da5", 826309},
		{"resource.003", 0, "85e51acb5f9c539d66e3c8fe40e17da5", 493638},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},

	// Castle of Dr. Brain - English DOS Demo
	{{"castlebrain", "Demo", {
		{"resource.map", 0, "467bb5e3224bb54640c3280032aebff5", 633},
		{"resource.000", 0, "9780f040d58182994e22d2e34fab85b0", 67367},
		{"resource.001", 0, "2af49dbd8f2e1db4ab09f9310dc91259", 570553},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 0, 5)
	},

#if 0
	// Castle of Dr. Brain - Spanish DOS
	{{"castlebrain", "", {
		{"resource.map", 0, "5738c163e014bbe046474de009020b82", 2727},
		{"resource.000", 0, "27ec5fa09cd12a7fd16e86d96a2ed245", 1197694},
		{"resource.001", 0, "735be4e58957180cfc807d5e18fdffcd", 1433302},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

	// Christmas Card 1988 - English DOS
	{{"christmas1988", "", {
		{"resource.map", 0, "39485580d34a72997f3d5b3aba4d24f1", 426},
		{"resource.001", 0, "11391434f41c834090d7a1e9488ce936", 129739},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 0, 294)
	},

	// Christmas Card 1990: The Seasoned Professional - English DOS (16 Colors)
	{{"christmas1990", "16 Colors", {
		{"resource.map", 0, "8f656714a05b94423ac6eb10ee8797d0", 600},
		{"resource.001", 0, "acde93e58fca4f7a2a5a220558a94aa8", 272629},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 0, 172)
	},

	// Christmas Card 1990: The Seasoned Professional - English DOS (256 Colors)
	{{"christmas1990", "256 Colors", {
		{"resource.map", 0, "44b8f45b841b9b5e17e939a35e443988", 600},
		{"resource.001", 0, "acde93e58fca4f7a2a5a220558a94aa8", 335362},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 0, 174)
	},

	// Christmas Card 1992 - English DOS
	{{"christmas1992", "", {
		{"resource.map", 0, "f1f8c8a8443f523422af70b4ec85b71c", 318},
		{"resource.000", 0, "62fb9256f8e7e6e65a6875efdb7939ac", 203396},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 55)
	},

	// Codename: Iceman - English Amiga (from www.back2roots.org)
	{{"iceman", "", {
		{"resource.map", 0, "035829b391709a4e542d7c7b224625f6", 6000},
		{"resource.000", 0, "b1bccd827453d4cb834bfd5b45bef63c", 73682},
		{"resource.001", 0, "ede5a0e1e2a80fb629dae72c72f33d37", 293145},
		{"resource.002", 0, "36670a917550757d57df84c96cf9e6d9", 469387},
		{"resource.003", 0, "d97a96f1ab91b41cf46a02cc89b0a04e", 619219},
		{"resource.004", 0, "8613c45fc771d658e5a505b9a4a54f31", 713382},
		{"resource.005", 0, "605b67a9ef199a9bb015745e7c004cf4", 478384},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},

	// Codename: Iceman - English DOS
	{{"iceman", "", {
		{"resource.map", 0, "a18f3cef4481a81d3415fb87a754343e", 5700},
		{"resource.000", 0, "b1bccd827453d4cb834bfd5b45bef63c", 26989},
		{"resource.001", 0, "32b351072fccf76fc82234d73d28c08b", 438872},
		{"resource.002", 0, "36670a917550757d57df84c96cf9e6d9", 566549},
		{"resource.003", 0, "d97a96f1ab91b41cf46a02cc89b0a04e", 624303},
		{"resource.004", 0, "8613c45fc771d658e5a505b9a4a54f31", 670883},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},

	// Conquests of Camelot - English Amiga (from www.back2roots.org)
	{{"camelot", "", {
		{"resource.map", 0, "51aba42f8e63b219755d4372ea424509", 6654},
		{"resource.000", 0, "dfadf0b4c9fb44ce55570149856c302d", 128100},
		{"resource.001", 0, "67391de361b9347f123ac0899b4b91f7", 300376},
		{"resource.002", 0, "8c7f12b2c38d225d4c7121b30ea1b4d2", 605334},
		{"resource.003", 0, "82a73e7572e7ee627429bb5111ff82ca", 672392},
		{"resource.004", 0, "6821dc97cf643ba521a4e840dda3c58b", 647410},
		{"resource.005", 0, "c6e551bdc24f0acc193159038d4ca767", 605882},
		{"resource.006", 0, "8f880a536908ab496bbc552f7f5c3738", 585255},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},

	// Conquests of Camelot - English DOS
	{{"camelot", "", {
		{"resource.map", 0, "95eca3991906dfd7ed26d193df07596f", 7278},
		{"resource.001", 0, "8e1a3a8c588007404b532b8dfacc1460", 596774},
		{"resource.002", 0, "8e1a3a8c588007404b532b8dfacc1460", 722250},
		{"resource.003", 0, "8e1a3a8c588007404b532b8dfacc1460", 723712},
		{"resource.004", 0, "8e1a3a8c588007404b532b8dfacc1460", 729143},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},

	// Conquests of the Longbow - English Amiga (from www.back2roots.org)
	{{"longbow", "", {
		{"resource.map", 0, "6204f3d00c0f6c0f5f95a29a4190f2f9", 6048},
		{"resource.000", 0, "8d11c744b4a51e7a8ceac687a46f08ca", 830333},
		{"resource.001", 0, "76caf8593e065a98c8ab4a6e2c7dbafc", 839008},
		{"resource.002", 0, "eb312373045906b54a3181bebaf6651a", 733145},
		{"resource.003", 0, "7fe3b3372d7fdda60045807e9c8e4867", 824554},
		{"resource.004", 0, "d1038c75d85a6650d48e07d174a6a913", 838175},
		{"resource.005", 0, "1c3804e56b114028c5873a35c2f06d13", 653002},
		{"resource.006", 0, "f9487732289a4f4966b4e34eea413325", 842817},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},

#if 0
	// Conquests of the Longbow - English DOS
	{{"longbow", "", {
		{"resource.map", 0, "247f955865572569342751de47e861ab", 6027},
		{"resource.000", 0, "36e8fda5d0b8c49e587c8a9617959f72", 1297120},
		{"resource.001", 0, "1e6084a19f7a6c50af88d3a9b32c411e", 1366155},
		{"resource.002", 0, "7f6ce331219d58d5087731e4475ab4f1", 1234743},
		{"resource.003", 0, "1867136d01ece57b531032d466910522", 823686},
		{"resource.004", 0, "9cfce07e204a329e94fda8b5657621da", 1261462},
		{"resource.005", 0, "21ebe6b39b57a73fc449f67f013765aa", 1284720},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

	// Conquests of the Longbow - English DOS
	{{"longbow", "", {
		{"resource.map", 0, "737c6f83a1ee601727ff026898f19fa1", 6045},
		{"resource.000", 0, "36e8fda5d0b8c49e587c8a9617959f72", 1296607},
		{"resource.001", 0, "1e6084a19f7a6c50af88d3a9b32c411e", 1379267},
		{"resource.002", 0, "7f6ce331219d58d5087731e4475ab4f1", 1234140},
		{"resource.003", 0, "1867136d01ece57b531032d466910522", 823610},
		{"resource.004", 0, "9cfce07e204a329e94fda8b5657621da", 1260237},
		{"resource.005", 0, "21ebe6b39b57a73fc449f67f013765aa", 1284609},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 168)
	},

	// Eco Quest - English DOS CD 1.1
	{{"ecoquest", "CD", {
		{"resource.map", 0, "a4b73d5d2b55bdb6e44345e99c8fbdd0", 4804},
		{"resource.000", 0, "d908dbef56816ac6c60dd145fdeafb2b", 3536046},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 64)
	},

#if 0
	// Eco Quest - Spanish DOS Floppy
	{{"ecoquest", "Floppy", {
		{"resource.map", 0, "82e6b1e3bdb2f064b18380009df7b345", 4395},
		{"resource.000", 0, "0b12a91c935e385308af8d17811deded", 1004085},
		{"resource.001", 0, "2fed7451bca81b0c891eed1a956f2263", 1212060},
		{"resource.002", 0, "2d21a1d2dcbffa551552e3e0725d2284", 1186033},
		{"resource.003", 0, "84dd11b6825255671c703aee5ceff620", 1174993},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Eco Quest 2 - English DOS Demo
	{{"ecoquest2", "Demo", {
		{"resource.map", 0, "607cfa0d8a03b7d348c06ee727e3d939", 1321},
		{"resource.000", 0, "dd6f614c43c029f063e93cd243af90a4", 525992},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 55)
	},

#if 0
	// Freddy Pharkas - German DOS (from Tobis87)
	{{"freddypharkas", "", {
		{"resource.map", 0, "a32674e7fbf7b213b4a066c8037f16b6", 5816},
		{"resource.000", 0, "96b07e9b914dba1c8dc6c78a176326df", 5233230},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Freddy Pharkas - Spanish DOS CD
	{{"freddypharkas", "CD", {
		{"resource.map", 0, "a32674e7fbf7b213b4a066c8037f16b6", 5816},
		{"resource.000", 0, "fed4808fdb72486908ac7ad0044b14d8", 1456640},
		{"resource.001", 0, "15298fac241b5360763bfb68add1db07", 1456640},
		{"resource.002", 0, "419dbd5366f702b4123dedbbb0cffaae", 1456640},
		{"resource.003", 0, "05acdc256c742e79c50b9fe7ec2cc898", 863310},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Freddy Pharkas - English DOS CD Demo
	{{"freddypharkas", "CD Demo", {
		{"resource.map", 0, "a62a7eae85dd1e6b07f39662b278437e", 1918},
		{"resource.000", 0, "4962a3c4dd44e36e78ea4a7a374c2220", 957382},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 95)
	},

	// Gabriel Knight - English DOS Floppy
	{{"gk1", "", {
		{"resource.map", 0, "372d059f75856afa6d73dd84cbb8913d", 10783},
		{"resource.000", 0, "69b7516962510f780d38519cc15fcc7c", 13022630},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 000, 000)
	},

#if 0
	// Gabriel Knight - English DOS CD
	{{"gk1", "CD", {
		{"resource.map", 0, "372d059f75856afa6d73dd84cbb8913d", 10996},
		{"resource.000", 0, "69b7516962510f780d38519cc15fcc7c", 12581736},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Gabriel Knight - German DOS CD (from Tobis87)
	{{"gk1", "CD", {
		{"resource.map", 0, "a7d3e55114c65647310373cb390815ba", 11392},
		{"resource.000", 0, "091cf08910780feabc56f8551b09cb36", 13400497},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Gabriel Knight - Spanish DOS CD
	{{"gk1", "CD", {
		{"resource.map", 0, "7cb6e9bba15b544ec7a635c45bde9953", 11404},
		{"resource.000", 0, "091cf08910780feabc56f8551b09cb36", 13381599},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Gabriel Knight - English DOS CD Demo
	{{"gk1", "CD Demo", {
		{"resource.map", 0, "39645952ae0ed8072c7e838f31b75464", 2490},
		{"resource.000", 0, "eb3ed7477ca4110813fe1fcf35928561", 1718450},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 92)
	},

#if 0
	// Gabriel Knight 2 - English DOS
	{{"gk2", "", {
		{"resmap.001", 0, "1b8bf6a23b37ed67358eb825fc687260", 2776},
		{"ressci.001", 0, "24463ae235b1afbbc4ff5e2ed1b8e3b2", 50496082},
		{"resmap.002", 0, "2028230674bb54cd24370e0745e7a9f4", 1975},
		{"ressci.002", 0, "f0edc1dcd704bd99e598c5a742dc7150", 42015676},
		{"resmap.003", 0, "51f3372a2133c406719dafad86369be3", 1687},
		{"ressci.003", 0, "86cb3f3d176994e7f8a9ad663a4b907e", 35313750},
		{"resmap.004", 0, "0f6e48f3e84e867f7d4a5215fcff8d5c", 2719},
		{"ressci.004", 0, "4f30aa6e6f895132402c8652f9e1d741", 58317316},
		{"resmap.005", 0, "2dac0e232262b4a51271fd28559b3e70", 2065},
		{"ressci.005", 0, "14b62d4a3bddee57a03cb1495a798a0f", 38075705},
		{"resmap.006", 0, "ce9359037277b7d7976da185c2fa0aad", 2977},
		{"ressci.006", 0, "8e44e03890205a7be12f45aaba9644b4", 60659424},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Hoyle 1 - English Amiga (from www.back2roots.org)
	{{"hoyle1", "", {
		{"resource.map", 0, "2a72b1aba65fa6e339370eb86d8601d1", 5166},
		{"resource.001", 0, "e0dd44069a62a463fd124974b915f10d", 218755},
		{"resource.002", 0, "e0dd44069a62a463fd124974b915f10d", 439502},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 519)	// FIXME: some versions are v. 0.000.530
	},
#endif

#if 0
	// Hoyle 2 - English Amiga (from www.back2roots.org)
	{{"hoyle2", "", {
		{"resource.map", 0, "62ed48d20c580e5a98f102f7cd93706a", 1356},
		{"resource.001", 0, "8f2dd70abe01112eca464cda818b5eb6", 222704},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 572)
	},
#endif

#if 0
	// Hoyle 3 - English Amiga (from www.back2roots.org)
	{{"hoyle3", "", {
		{"resource.map", 0, "f1f158e428398cb87fc41fb4aa8c2119", 2088},
		{"resource.000", 0, "595b6039ea1356e7f96a52c58eedcf22", 355791},
		{"resource.001", 0, "143df8aef214a2db34c2d48190742012", 632273},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Hoyle 3 - English DOS
	{{"hoyle3", "", {
		{"resource.map", 0, "7216a2972f9c595c45ab314941628e43", 2247},
		{"resource.000", 0, "6ef28cac094dcd97fdb461662ead6f92", 541845},
		{"resource.001", 0, "0a98a268ee99b92c233a0d7187c1f0fa", 845795},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Jones in the Fast Lane - English DOS
	{{"jones", "", {
		{"resource.map", 0, "65cbe19b36fffc71c8e7b2686bd49ad7", 1800},
		{"resource.001", 0, "bac3ec6cb3e3920984ab0f32becf5163", 313476},
		{"resource.002", 0, "b86daa3ba2784d1502da881eedb80d9b", 719747},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 172)
	},
#endif

#if 0
	// King's Quest 1 SCI Remake - English Amiga (from www.back2roots.org)
	{{"kq1sci", "SCI Remake", {
		{"resource.map", 0, "37ed1a05eb719629eba15059c2eb6cbe", 6798},
		{"resource.001", 0, "9ae2a13708d691cd42f9129173c4b39d", 266621},
		{"resource.002", 0, "9ae2a13708d691cd42f9129173c4b39d", 795123},
		{"resource.003", 0, "9ae2a13708d691cd42f9129173c4b39d", 763224},
		{"resource.004", 0, "9ae2a13708d691cd42f9129173c4b39d", 820443},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 000)
	},
#endif

	// King's Quest 1 SCI Remake - English DOS
	{{"kq1sci", "SCI Remake", {
		{"resource.map", 0, "7fe9399a0bec84ca5727309778d27f07", 5790},
		{"resource.001", 0, "fed9e0072ffd511d248674e60dee2099", 555439},
		{"resource.002", 0, "fed9e0072ffd511d248674e60dee2099", 714062},
		{"resource.003", 0, "fed9e0072ffd511d248674e60dee2099", 717478},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 999)
	},

	// King's Quest 4 - English Amiga (from www.back2roots.org)
	{{"kq4sci", "", {
		{"resource.map", 0, "f88dd267fb9504d40a04d599c048d42b", 6354},
		{"resource.000", 0, "77615c595388acf3d1df8e107bfb6b52", 138523},
		{"resource.001", 0, "52c2231765eced34faa7f7bcff69df83", 44751},
		{"resource.002", 0, "fb351106ec865fad9af5d78bd6b8e3cb", 663629},
		{"resource.003", 0, "fd16c9c223f7dc5b65f06447615224ff", 683016},
		{"resource.004", 0, "3fac034c7d130e055d05bc43a1f8d5f8", 549993},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 502)
	},

	// King's Quest 4 - English DOS
	{{"kq4sci", "", {
		{"resource.map", 0, "3164a39790b599c954ecf716d0b32be8", 7476},
		{"resource.001", 0, "77615c595388acf3d1df8e107bfb6b52", 452523},
		{"resource.002", 0, "77615c595388acf3d1df8e107bfb6b52", 536573},
		{"resource.003", 0, "77615c595388acf3d1df8e107bfb6b52", 707591},
		{"resource.004", 0, "77615c595388acf3d1df8e107bfb6b52", 479562},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 502)
	},

	// King's Quest 4 - English DOS
	{{"kq4sci", "", {
		{"resource.map", 0, "adbe267662a5915d3c89c9075ec8cf3e", 9474},
		{"resource.001", 0, "851a62d00972dc4002f472cc0d84e71d", 188239},
		{"resource.002", 0, "851a62d00972dc4002f472cc0d84e71d", 329895},
		{"resource.003", 0, "851a62d00972dc4002f472cc0d84e71d", 355385},
		{"resource.004", 0, "851a62d00972dc4002f472cc0d84e71d", 322951},
		{"resource.005", 0, "851a62d00972dc4002f472cc0d84e71d", 321593},
		{"resource.006", 0, "851a62d00972dc4002f472cc0d84e71d", 333777},
		{"resource.007", 0, "851a62d00972dc4002f472cc0d84e71d", 341038},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 274)
	},

	// King's Quest 5 - English Amiga (from www.back2roots.org)
	{{"kq5", "", {
		{"resource.map", 0, "fcbcca058e1157221ffc27251cd59bc3", 8040},
		{"resource.000", 0, "c595ca99e7fa9b2cabcf69cfab0caf67", 344909},
		{"resource.001", 0, "964a3be90d810a99baf72ea70c09f935", 836477},
		{"resource.002", 0, "d10f3e8ff2cd95a798b21cd08797b694", 814730},
		{"resource.003", 0, "f72fdd994d9ba03a8360d639f256344e", 804882},
		{"resource.004", 0, "a5b80f95c66b3a032348989408eec287", 747914},
		{"resource.005", 0, "31a5487f4d942e6354d5be49d59707c9", 834146},
		{"resource.006", 0, "26c0c25399b6715fec03fc3e12544fe3", 823048},
		{"resource.007", 0, "b914b5901e786327213e779725d30dd1", 778772},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 784)
	},

	// King's Quest 5 - English DOS
	{{"kq5", "", {
		{"resource.map", 0, "f68ba690e5920725dcf9328001b90e33", 13122},
		{"resource.000", 0, "449471bfd77be52f18a3773c7f7d843d", 571368},
		{"resource.001", 0, "b45a581ff8751e052c7e364f58d3617f", 16800210},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 60)
	},

	// King's Quest 5 - English DOS
	{{"kq5", "", {
		{"resource.map", 0, "d6172c27b453350e158815fbae23f41e", 8004},
		{"resource.000", 0, "a591bd4b879fc832b8095c0b3befe9e2", 276351},
		{"resource.001", 0, "3f28c72dc7531aaccf8e972c7ee50d14", 1022087},
		{"resource.002", 0, "3e56ba5bf5e8637c619b57f6b6cacbb4", 1307211},
		{"resource.003", 0, "5d5d498f33ca7cde0d5b058630b36ad3", 1347875},
		{"resource.004", 0, "944a996f9cc90dabde9f51ed7dd52366", 1239689},
		{"resource.005", 0, "b6c43441cb78a9b484efc8e614aac092", 1287999},
		{"resource.006", 0, "672ede1136e9e401658538e51bd5dc22", 1172619},
		{"resource.007", 0, "2f48faf27666b58c276dda20f91f4a93", 1240456},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 60)
	},

#if 0
	// King's Quest 5 - Italian DOS Floppy (from glorifindel)
	{{"kq5", "", {
		{"resource.map", 0, "d55c9e83894a0885e37cd79bacf86384", 8283},
		{"resource.000", 0, "c99bbb11ace4aaacdc98b588a2ecea06", 332246},
		{"resource.001", 0, "42b98457b1a7282daa27afd89eef53f4", 1136389},
		{"resource.002", 0, "8cdc160f9dfc84aed7caa6c66fa31000", 1340730},
		{"resource.003", 0, "d0cb52dc41488c018359aa79a6527f51", 1323676},
		{"resource.004", 0, "e5c57060adf2b5c6fc24142acba023da", 1331097},
		{"resource.005", 0, "f4e441f284560eaa8022102315656a7d", 1267757},
		{"resource.006", 0, "8eeabd92af71e766e323db2100879102", 1209325},
		{"resource.007", 0, "dc10c107e0923b902326a040b9c166b9", 1337859},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 60)
	},
#endif

	// King's Quest 6 - English DOS
	{{"kq6", "", {
		{"resource.map", 0, "7a550ebfeae2575ca00d47703a6a774c", 9215},
		{"resource.000", 0, "233394a5f33b475ae5975e7e9a420865", 8376352},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 69)
	},

	// King's Quest 6 - English DOS
	{{"kq6", "", {
		{"resource.map", 0, "a362063318eebe7d6423b1d9dc6213e1", 8703},
		{"resource.000", 0, "f2b7f753992c56a0c7a08d6a5077c895", 7863324},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 54)
	},

	// King's Quest 7 - English DOS
	{{"kq7", "", {
		{"resource.map", 0, "2be9ab94429c721af8e05c507e048a15", 18697},
		{"resource.000", 0, "eb63ea3a2c2469dc2d777d351c626404", 203882535},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

#if 0
	// King's Quest 7 - Spanish DOS
	{{"kq7", "", {
		{"resource.map", 0, "0b62693cbe87e3aaca3e8655a437f27f", 18709},
		{"resource.000", 0, "51c1ead1163e19a2de8f121c39df7a76", 200764100},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// King's Quest 7 - English DOS Demo
	{{"kq7", "Demo", {
		{"resource.map", 0, "b44f774108d63faa1d021101221c5a54", 1690},
		{"resource.000", 0, "d9659d2cf0c269c6a9dc776707f5bea0", 2433827},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

#if 0
	// Laura Bow - English Amiga (from www.back2roots.org)
	{{"laurabow", "", {
		{"resource.map", 0, "731ab85e138f8cef1a7f4d1f36dfd375", 7422},
		{"resource.000", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 126317},
		{"resource.001", 0, "42fe895e9eb60e103025fd9ca737a849", 264763},
		{"resource.002", 0, "6f1ebd3692ce76644e0e06a38b7b56b5", 677436},
		{"resource.003", 0, "2ab23f64306b18c28302c8ec2964c5d6", 605134},
		{"resource.004", 0, "aa553977f7e5804081de293800d3bcce", 695067},
		{"resource.005", 0, "bfd870d51dc97729f0914095f58e6957", 676881},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 631)	// FIXME: some versions are v. 0.000.685
	},
#endif

#if 0
	// Laura Bow - English DOS (from "The Roberta Williams Anthology"/1996)
	{{"laurabow", "", {
		{"resource.map", 0, "4e511f47d9893fa529d6621a93fa0030", 8478},
		{"resource.001", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 515788},
		{"resource.002", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 721381},
		{"resource.003", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 667468},
		{"resource.004", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 683807},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 631)
	},
#endif

#if 0
	// Laura Bow - German DOS (from Tobis87)
	{{"laurabow", "", {
		{"resource.map", 0, "b1905f6aa68ff65a057b080b1eae954c", 12030},
		{"resource.001", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 108032},
		{"resource.002", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 354680},
		{"resource.003", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 361815},
		{"resource.004", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 339714},
		{"resource.005", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 327465},
		{"resource.006", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 328390},
		{"resource.007", 0, "e45c888d9c7c04aec0a20e9f820b79ff", 317687},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 631)	// FIXME: some versions are v. 0.000.685
	},
#endif

	// Laura Bow 2 - English DOS Floppy
	{{"laurabow2", "", {
		{"resource.map", 0, "610bfd9a852004222f0faaf5fc9e630a", 6489},
		{"resource.000", 0, "57084910bc923bff5d6d9bc1b56e9604", 5035964},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 000, 274)
	},

#if 0
	// Laura Bow 2 - English DOS CD (from "The Roberta Williams Antology"/1996)
	{{"laurabow2", "CD", {
		{"resource.map", 0, "a70945e61ba7ac7bfea6b7bd72c6aec5", 7274},
		{"resource.000", 0, "82578b8d5a7e09c4c58891ca49fae35b", 5598672},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 001, 69)		// FIXME: some version are v. 1.001.072
	},
#endif

#if 0
	// Laura Bow 2 - German DOS CD (from Tobis87)
	{{"laurabow2", "CD", {
		{"resource.map", 0, "3b6dfbcda210bbc3f23fd1927113bf98", 6483},
		{"resource.000", 0, "57084910bc923bff5d6d9bc1b56e9604", 5028766},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Laura Bow 2 - Spanish DOS CD
	{{"laurabow2", "CD", {
		{"resource.map", 0, "3b6dfbcda210bbc3f23fd1927113bf98", 6483},
		{"resource.000", 0, "57084910bc923bff5d6d9bc1b56e9604", 5028766},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Larry 1 EGA Remake - English DOS (from spookypeanut)
	{{"lsl1sci", "EGA Remake", {
		{"resource.map", 0, "abc0dc50c55de5b9723bb6de193f8756", 3282},
		{"resource.000", 0, "d3bceaebef3f7be941c2038b3565161e", 451366},
		{"resource.001", 0, "38936d3c68b6f79d3ffb13955713fed7", 591352},
		{"resource.002", 0, "24c958bc922b07f91e25e8c93aa01fcf", 491230},
		{"resource.003", 0, "685cd6c1e05a695ab1e0db826337ee2a", 553279},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 510)	// FIXME: some versions are v. 0.000.577
	},
#endif

#if 0
	// Larry 1 VGA Remake - English Amiga (from www.back2roots.org)
	{{"lsl1sci", "VGA Remake", {
		{"resource.map", 0, "7d115a9e27dc8ac71e8d5ef33d589bd5", 3366},
		{"resource.000", 0, "e67fd129d5810fc7ad8ea509d891cc00", 363073},
		{"resource.001", 0, "24ed6dc01b1e7fbc66c3d63a5994549a", 750465},
		{"resource.002", 0, "5790ac0505f7ca98d4567132b875eb1e", 681041},
		{"resource.003", 0, "4a34c3367c2fe7eb380d741374da1989", 572251},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 510)	// FIXME: some versions are v. 0.000.577
	},
#endif

	// Larry 1 VGA Remake - English DOS (from spookypeanut)
	{{"lsl1sci", "VGA Remake", {
		{"resource.map", 0, "6d04d26466337a1a64b8c6c0eb65c9a9", 3222},
		{"resource.000", 0, "d3bceaebef3f7be941c2038b3565161e", 922406},
		{"resource.001", 0, "ec20246209d7b19f38989261e5c8f5b8", 1111226},
		{"resource.002", 0, "85d6935ef77e6b0e16bc307640a0d913", 1088312},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 577)
	},

#if 0
	// Larry 1 VGA Remake - Spanish DOS
	{{"lsl1sci", "VGA Remake", {
		{"resource.map", 0, "4fbe5c25878d51d7b2a68b710de4491b", 3327},
		{"resource.000", 0, "5e501a9bf8c753bf4c96158042422f00", 839172},
		{"resource.001", 0, "112648995dbc194037f1e4ed2e195910", 1063341},
		{"resource.002", 0, "3fe2a3aec0ed53c7d6db1845a67e3aa2", 1095908},
		{"resource.003", 0, "ac175df0ea9a2cba57f0248651856d27", 376556},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 510)	// FIXME: some versions are v. 0.000.577
	},
#endif

	// Larry 2 - English Amiga (from www.back2roots.org)
	{{"lsl2", "", {
		{"resource.map", 0, "e36ce0fc94d1678d15acbf12d84ec47d", 6612},
		{"resource.001", 0, "a0d4a625311d307257da7fc43d00459d", 409124},
		{"resource.002", 0, "a0d4a625311d307257da7fc43d00459d", 630106},
		{"resource.003", 0, "a0d4a625311d307257da7fc43d00459d", 570356},
		{"resource.004", 0, "a0d4a625311d307257da7fc43d00459d", 717844},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 572)
	},

	// Larry 2 - English DOS
	{{"lsl2", "", {
		{"resource.map", 0, "42258cf767a8ebaa9e66b6151a80e601", 5628},
		{"resource.001", 0, "4a24443a25e2b1492462a52809605dc2", 143847},
		{"resource.002", 0, "4a24443a25e2b1492462a52809605dc2", 348331},
		{"resource.003", 0, "4a24443a25e2b1492462a52809605dc2", 236550},
		{"resource.004", 0, "4a24443a25e2b1492462a52809605dc2", 204861},
		{"resource.005", 0, "4a24443a25e2b1492462a52809605dc2", 277732},
		{"resource.006", 0, "4a24443a25e2b1492462a52809605dc2", 345683},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 409)
	},

#if 0
	// Larry 3 - English Amiga (from www.back2roots.org)
	{{"lsl3", "", {
		{"resource.map", 0, "4a6da6322ce189431b5ffbac992bad3a", 5328},
		{"resource.000", 0, "cdc2e21e297b10fe8fed6377af8c5698", 66523},
		{"resource.001", 0, "6abbaf8c7e3b36dd868868ed187e8995", 71761},
		{"resource.002", 0, "a883424fe6d594fec0cd5a79e7ad54c8", 476490},
		{"resource.003", 0, "5c10e462c8cf589610773e4fe8bfd996", 527238},
		{"resource.004", 0, "f408e59cbee1457f042e5773b8c53951", 651634},
		{"resource.005", 0, "433911eb764089d493aed1f958a5615a", 524259},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 572)
	},
#endif

	// Larry 3 - English DOS
	{{"lsl3", "", {
		{"resource.map", 0, "0b6bd3e039682830a51c5755c06591db", 5916},
		{"resource.001", 0, "f18441027154292836b973c655fa3175", 456722},
		{"resource.002", 0, "f18441027154292836b973c655fa3175", 578024},
		{"resource.003", 0, "f18441027154292836b973c655fa3175", 506807},
		{"resource.004", 0, "f18441027154292836b973c655fa3175", 513651},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 572)
	},

#if 0
	// Larry 3 - German DOS (from Tobis87)
	{{"lsl3", "", {
		{"resource.map", 0, "4a77c8382e48a90c4168d3c144fc1b8f", 6480},
		{"resource.001", 0, "3827a9b17b926e12dcc336860f50612a", 460488},
		{"resource.002", 0, "3827a9b17b926e12dcc336860f50612a", 672403},
		{"resource.003", 0, "3827a9b17b926e12dcc336860f50612a", 587036},
		{"resource.004", 0, "3827a9b17b926e12dcc336860f50612a", 691932},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 572)
	},
#endif

#if 0
	// Larry 5 - English Amiga (from www.back2roots.org)
	{{"lsl5", "", {
		{"resource.map", 0, "e36052ae0c8b14d6f074bcb0aee50a38", 6096},
		{"resource.000", 0, "d8b58ce10de52aa16f8b2006838c4fcc", 310510},
		{"resource.001", 0, "8caa8fbb50ea43f3efdfb66f1e68998b", 800646},
		{"resource.002", 0, "abdaa299e00c908052d33cd82eb60e9b", 784576},
		{"resource.003", 0, "810ad1d61638c27a780576cb09f18ed7", 805941},
		{"resource.004", 0, "3ce5901f1bc171ac0274d99a4eeb9e57", 623022},
		{"resource.005", 0, "f8b2d1137bb767e5d232056b99dd69eb", 623621},
		{"resource.006", 0, "bafc64e3144f115dc58c6aee02de98fb", 715598},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

	// Larry 5 - English DOS (from spookypeanut)
	{{"lsl5", "", {
		{"resource.map", 0, "be00ef895197754ae4eab021ca44cbcd", 6417},
		{"resource.000", 0, "f671ab479df0c661b19cd16237692846", 726823},
		{"resource.001", 0, "db4a1381d88028876a99303bfaaba893", 751296},
		{"resource.002", 0, "d39d8db1a1e7806e7ccbfea3ef22df44", 1137646},
		{"resource.003", 0, "13fd4942bb818f9acd2970d66fca6509", 768599},
		{"resource.004", 0, "999f407c9f38f937d4b8c4230ff5bb38", 1024516},
		{"resource.005", 0, "0cc8d35a744031c772ca7cd21ae95273", 1011944},
		{"resource.006", 0, "dda27ce00682aa76198dac124bbbe334", 1024810},
		{"resource.007", 0, "ac443fae1285fb359bf2b2bc6a7301ae", 1030656},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},

#if 0
	// Larry 5 - German DOS (from Tobis87)
	{{"lsl5", "", {
		{"resource.map", 0, "c97297aa76d4dd2ed144c7b7769e2caf", 6867},
		{"resource.000", 0, "4c00c14b8181ad47076a51d86097d97e", 759095},
		{"resource.001", 0, "245c44f8ccd796732e61857e67b30079", 918742},
		{"resource.002", 0, "e86aeb27711f4a673e06ec32cfc84125", 947382},
		{"resource.003", 0, "74edc89d8c1cb346ca346081b927e4c6", 1006884},
		{"resource.004", 0, "999f407c9f38f937d4b8c4230ff5bb38", 1023776},
		{"resource.005", 0, "0cc8d35a744031c772ca7cd21ae95273", 959342},
		{"resource.006", 0, "dda27ce00682aa76198dac124bbbe334", 1021774},
		{"resource.007", 0, "ac443fae1285fb359bf2b2bc6a7301ae", 993408},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

#if 0
	// Larry 5 - Italian DOS Floppy (from glorifindel)
	{{"lsl5", "", {
		{"resource.map", 0, "a99776df795127f387cb35dae872d4e4", 5919},
		{"resource.000", 0, "a8989a5a89e7d4f702b26b378c7a357a", 7001981},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

#if 0
	// Larry 5 - Spanish DOS
	{{"lsl5", "", {
		{"resource.map", 0, "b6f7da7bf24e5a6b2946032cec3ea59c", 6861},
		{"resource.000", 0, "4c00c14b8181ad47076a51d86097d97e", 765418},
		{"resource.001", 0, "245c44f8ccd796732e61857e67b30079", 916028},
		{"resource.002", 0, "e86aeb27711f4a673e06ec32cfc84125", 929645},
		{"resource.003", 0, "74edc89d8c1cb346ca346081b927e4c6", 1005496},
		{"resource.004", 0, "999f407c9f38f937d4b8c4230ff5bb38", 1021996},
		{"resource.005", 0, "0cc8d35a744031c772ca7cd21ae95273", 958079},
		{"resource.006", 0, "dda27ce00682aa76198dac124bbbe334", 1015136},
		{"resource.007", 0, "ac443fae1285fb359bf2b2bc6a7301ae", 987222},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

	// Larry 6 - English DOS (from spookypeanut)
	{{"lsl6", "", {
		{"resource.map", 0, "bb8a39d9e2a77ba449a1e591109ad9a8", 6973},
		{"resource.000", 0, "4462fe48c7452d98fddcec327a3e738d", 5789138},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 001, 113)
	},

	// Larry 6 - English/German DOS CD - LORES
	{{"lsl6", "", {
		{"resource.map", 0, "0b91234b7112782962cb480b7791b6e2", 7263},
		{"resource.000", 0, "57d5fe8bb9e044158514476ea7678eb0", 5754790},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 001, 115)
	},

	// Larry 6 - English/German DOS CD - HIRES
	{{"lsl6", "", {
		{"resource.map", 0, "0c0804434ea62278dd15032b1947426c", 8872},
		{"resource.000", 0, "9a9f4870504444cda863dd14d077a680", 18520872},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 002)
	},

#if 0
	// Larry 6 - German DOS (from Tobis87)
	{{"lsl6", "", {
		{"resource.map", 0, "badfdf446ffed569a310d2c63a249421", 8896},
		{"resource.000", 0, "bd944d2b06614a5b39f1586906f0ee88", 18534274},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Larry 6 - Spanish DOS
	{{"lsl6", "", {
		{"resource.map", 0, "633bf8f42170b6271019917c8009989b", 6943},
		{"resource.000", 0, "7884a8db9253e29e6b37a2651fd90ba3", 5733116},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Larry 7 - English DOS CD (from spookypeanut)
	{{"lsl7", "", {
		{"resmap.000", 0, "eae93e1b1d1ccc58b4691c371281c95d", 8188},
		{"ressci.000", 0, "89353723488219e25589165d73ed663e", 66965678},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(3, 000, 000)
	},

#if 0
	// Larry 7 - German DOS (from Tobis87)
	{{"lsl7", "", {
		{"resmap.000", 0, "c11e6bfcfc2f2d05da47e5a7df3e9b1a", 8188},
		{"ressci.000", 0, "a8c6817bb94f332ff498a71c8b47f893", 66971724},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Larry 7 - Italian DOS CD (from glorifindel)
	{{"lsl7", "", {
		{"resmap.000", 0, "9852a97141f789413f29bf956052acdb", 8212},
		{"ressci.000", 0, "440b9fed89590abb4e4386ed6f948ee2", 67140181},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Larry 7 - Spanish DOS
	{{"lsl7", "", {
		{"resmap.000", 0, "8f3d603e1acc834a5d598b30cdfc93f3", 8188},
		{"ressci.000", 0, "32792f9bc1bf3633a88b382bb3f6e40d", 67071418},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Lighthouse - English DOS
	{{"lighthouse", "", {
		{"resmap.001", 0, "47abc502c0b541b582db28f38dbc6a56", 7801},
		{"ressci.001", 0, "14e922c47b92156377cb49e241691792", 99591924},
		{"resmap.002", 0, "c68db5333f152fea6ca2dfc75cad8b34", 7573},
		{"ressci.002", 0, "175468431a979b9f317c294ce3bc1430", 94628315},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Lighthouse - Spanish DOS
	{{"lighthouse", "", {
		{"resmap.001", 0, "c5d49b2a8a4eafc92fd041a3a0f2da68", 7846},
		{"ressci.001", 0, "18553177dbf83fb2cb6c8edcbb174183", 99543093},
		{"resmap.002", 0, "e7dc85884a2417e2eff9de0c63dd65fa", 7630},
		{"ressci.002", 0, "3c8d627c555b0e3e4f1d9955bc0f0df4", 94631127},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Mixed-Up Mother Goose - English Amiga (from www.back2roots.org)
	{{"mothergoose", "", {
		{"resource.map", 0, "4aa28ac93fae03cf854594da13d9229c", 2700},
		{"resource.001", 0, "fb552ae550ca1dac19ed8f6a3767612d", 262885},
		{"resource.002", 0, "fb552ae550ca1dac19ed8f6a3767612d", 817191},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},
#endif

#if 0
	// Mixed-Up Mother Goose - English DOS CD
	{{"mothergoose", "CD", {
		{"resource.map", 0, "1c7f311b0a2c927b2fbe81ae341fb2f6", 5790},
		{"resource.001", 0, "5a0ed1d745855148364de1b3be099bac", 4369438},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},
#endif

#if 0
	// Phantasmagoria - English DOS
	{{"phantasmagoria", "", {
		{"resmap.001", 0, "416138651ea828219ca454cae18341a3", 11518},
		{"ressci.001", 0, "3aae6559aa1df273bc542d5ac6330d75", 65844612},
		{"resmap.002", 0, "de521d0c7ab32897e7fe58e421c816b7", 12058},
		{"ressci.002", 0, "3aae6559aa1df273bc542d5ac6330d75", 71588691},
		{"resmap.003", 0, "25df95bd7da3686f71a0af8784a2b8ca", 12334},
		{"ressci.003", 0, "3aae6559aa1df273bc542d5ac6330d75", 73651084},
		{"resmap.004", 0, "e108a3d35794f1721aeea3e62a3f8b3b", 12556},
		{"ressci.004", 0, "3aae6559aa1df273bc542d5ac6330d75", 75811935},
		{"resmap.005", 0, "390d81f9e14a3f3ee2ea477135f0288e", 12604},
		{"ressci.005", 0, "3aae6559aa1df273bc542d5ac6330d75", 78814934},
		{"resmap.006", 0, "8ea3c954606e80604680f9fe707f15d8", 12532},
		{"ressci.006", 0, "3aae6559aa1df273bc542d5ac6330d75", 77901360},
		{"resmap.007", 0, "afbd16ea77869a720afa1c5371de107d", 7972},
		//{"ressci.007", 0, "3aae6559aa1df273bc542d5ac6330d75", 25859038},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Phantasmagoria 2 - English Windows
	{{"phantasmagoria2", "", {
		{"resmap.001", 0, "0a961e135f4f7effb195158325856633", 1108},
		{"ressci.001", 0, "53f457cddb0dffc056593905c4cbb989", 24379964},
		{"resmap.002", 0, "5d3189fe3d4f286f83c5c8031fa3e9f7", 1126},
		{"ressci.002", 0, "53f457cddb0dffc056593905c4cbb989", 34465805},
		{"resmap.003", 0, "c92e3c840b827c236ab6671c03760c56", 1162},
		{"ressci.003", 0, "53f457cddb0dffc056593905c4cbb989", 38606375},
		{"resmap.004", 0, "8d5cfe19365f71370b87063686f39171", 1288},
		{"ressci.004", 0, "53f457cddb0dffc056593905c4cbb989", 42447131},
		{"resmap.005", 0, "8bd5ceeedcbe16dfe55d1b90dcd4be84", 1942},
		{"ressci.005", 0, "05f9fe2bee749659acb3cd2c90252fc5", 67905112},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Police Quest 1 VGA Remake - English DOS
	{{"pq1sci", "VGA Remake", {
		{"resource.map", 0, "35efa814fb994b1cbdac9611e401da67", 5013},
		{"resource.000", 0, "e0d5ddf34eda903a38f0837e2aa7145b", 6401433},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 001, 29)
	},

#if 0
	// Police Quest 2 - English Amiga (from www.back2roots.org)
	{{"pq2", "", {
		{"resource.map", 0, "499de78ae72b7ce219f944c5e7ef0c5b", 3426},
		{"resource.000", 0, "77f02def3094af804fd2371db25b7100", 250232},
		{"resource.001", 0, "523db0c07f1da2a822c2c39ee0482544", 179334},
		{"resource.002", 0, "499737c21a28ac026e11ab817100d610", 511099},
		{"resource.003", 0, "e008f5d6e2a7c4d4a0da0173e4fa8f8b", 553970},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 395)	// FIXME: some versions are v. 0.000.409/0.000.490
	},
#endif

	// Police Quest 2 - English DOS
	{{"pq2", "", {
		{"resource.map", 0, "28a6f471c7900c2c92da40eecb615d9d", 4584},
		{"resource.001", 0, "77f02def3094af804fd2371db25b7100", 509525},
		{"resource.002", 0, "77f02def3094af804fd2371db25b7100", 546000},
		{"resource.003", 0, "77f02def3094af804fd2371db25b7100", 591851},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 490)
	},

#if 0
	// Police Quest 3 - English Amiga (from www.back2roots.org)
	{{"pq3", "", {
		{"resource.map", 0, "29923fe1ef1f0909b57255d61c558e68", 5742},
		{"resource.000", 0, "4908e4f4977e8e19c90c29b36a741ffe", 298541},
		{"resource.001", 0, "0eb943ca807e2f69578821d490770d2c", 836567},
		{"resource.002", 0, "f7044bb08a1fcbe5077791ed8d4996f0", 691207},
		{"resource.003", 0, "630bfa65beb05f743552704ac2899dae", 759891},
		{"resource.004", 0, "7b229fbdf30d670d0728cede3e984a7e", 838663},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

	// Police Quest 3 - English DOS
	{{"pq3", "", {
		{"resource.map", 0, "6457bf0c8ca865a42d9ff5827ab49b89", 5559},
		{"resource.000", 0, "7659713720d61d9465a59091b7ee63ea", 737253},
		{"resource.001", 0, "61c7c187d25a8346be0a092d5f037278", 1196787},
		{"resource.002", 0, "c18e0d408e4f4f40365d42aa15931f67", 1153561},
		{"resource.003", 0, "8791b9eef53edf77c2dac950142221d3", 1159791},
		{"resource.004", 0, "1b91e891a3c60a941dac0eecdf83375b", 1143606},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},

	// Police Quest 4 - English DOS
	{{"pq4", "", {
		{"resource.map", 0, "379dfe80ed6bd16c47e4b950c4722eac", 11374},
		{"resource.000", 0, "fd316a09b628b7032248139003369022", 18841068},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 002)
	},

	// Police Quest 4 - English DOS
	{{"pq4", "", {
		{"resource.map", 0, "aed9643158ccf01b71f359db33137f82", 9895},
		{"resource.000", 0, "da383857b3be1e4514daeba2524359e0", 15141432},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 000, 000)
	},

#if 0
	// Quest for Glory 1 - English Amiga (from www.back2roots.org)
	{{"qfg1", "", {
		{"resource.map", 0, "e65034832f0c9df1dc22128227b782d0", 6066},
		{"resource.000", 0, "1c0255dea2d3cd71eee9f2db201eee3f", 111987},
		{"resource.001", 0, "a270012fa74445d74c044d1b65a9ff8c", 143570},
		{"resource.002", 0, "e64004e020fdf1813be52b639b08be89", 553201},
		{"resource.003", 0, "7ab2bf8e224b57f75e0cd6e4ba790761", 642203},
		{"resource.004", 0, "7ab2bf8e224b57f75e0cd6e4ba790761", 641688},
		{"resource.005", 0, "5f3386ef2f2b1254e4a066f5d9027324", 609529},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 629)
	},
#endif

#if 0
	// Quest for Glory 1 - English DOS
	{{"qfg1", "", {
		{"resource.map", 0, "74a108a7fb345bfc84f4113b6e5241bb", 6432},
		{"resource.000", 0, "40332d3ebfc70a4b6a6a0443c2763287", 79181},
		{"resource.001", 0, "917fcef303e9489597154727baaa9e07", 461422},
		{"resource.002", 0, "05ddce5f437a516b89ede2438fac09d8", 635734},
		{"resource.003", 0, "951299a82a8134ed12c5c18118d45c2f", 640483},
		{"resource.004", 0, "951299a82a8134ed12c5c18118d45c2f", 644443},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 629)
	},
#endif

	// Quest for Glory 1 - English DOS
	{{"qfg1", "", {
		{"resource.map", 0, "a731fb6c9c0b282443f7027bc8694d4c", 8469},
		{"resource.000", 0, "ecace1a2771846b1a8aa1afdd44111a0", 6570147},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 000, 411)
	},

#if 0
	// Quest for Glory 2 - English Amiga (from www.back2roots.org)
	{{"qfg2", "", {
		{"resource.map", 0, "365ea1033ba26d227ec4007be88c59cc", 7596},
		{"resource.000", 0, "810245be50fde5a67e3ea95e876e3e64", 233341},
		{"resource.001", 0, "7a5fde9875211ed67a896fc0d91940c8", 127294},
		{"resource.002", 0, "dcf6bc2c18660d7ad532fb61861eb721", 543644},
		{"resource.003", 0, "dcf6bc2c18660d7ad532fb61861eb721", 565044},
		{"resource.004", 0, "dcf6bc2c18660d7ad532fb61861eb721", 466630},
		{"resource.005", 0, "a77d2576c842b2b06da57d4ac8fc51c0", 579975},
		{"resource.006", 0, "ccf5dba33e5cab6d5872838c0f8db44c", 500039},
		{"resource.007", 0, "4c9fc1587545879295cb9627f56a2cb8", 575056},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Quest for Glory 3 - English DOS
	{{"qfg3", "", {
		{"resource.map", 0, "19e2bf9b693932b5e2bb59b9f9ab86c9", 5958},
		{"resource.000", 0, "6178ad2e83e58e4671ca03315f7a6498", 5868000},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 50)
	},

#if 0
	// Quest for Glory 3 - Spanish DOS
	{{"qfg3", "", {
		{"resource.map", 0, "10809197c33a5e62819311d8a2f73f85", 5978},
		{"resource.000", 0, "ba7ac86155e4c531e46cd73c86daa80a", 5884098},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Quest for Glory 4 - English DOS
	{{"qfg4", "", {
		{"resource.map", 0, "aba367f2102e81782d961b14fbe3d630", 10246},
		{"resource.000", 0, "263dce4aa34c49d3ad29bec889007b1c", 11571394},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Quest for Glory 4 - English DOS/Windows
	{{"qfg4", "", {
		{"resource.map", 0, "aba367f2102e81782d961b14fbe3d630", 10246},
		{"resource.000", 0, "263dce4aa34c49d3ad29bec889007b1c", 11571394},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

#if 0
	// NOTE: This version looks to be exactly the same as the English one
	// Perhaps it's the English one?

	// Quest for Glory 4 - German DOS/Windows (from PCJoker 2/98)
	{{"qfg4", "", {
		{"resource.map", 0, "aba367f2102e81782d961b14fbe3d630", 10246},
		{"resource.000", 0, "263dce4aa34c49d3ad29bec889007b1c", 11571394},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},
#endif

#if 0
	// Quest for Glory 4 - German DOS/Windows Disk V1.1 (from PCJoker 2/89)
	{{"qfg4", "", {
		{"resource.map", 0, "9e0abba8746f40565bc7eb5720522ecd", 9301 },
		{"resource.000", 0, "57f22cdc54eeb35fce1f26b31b5c3ee1", 11076197},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 000, 000)
	},
#endif

#if 0
	// RAMA - English Windows
	{{"rama", "", {
		{"resmap.001", 0, "3bac72a1910a563f8f92cf5b77c8b7f2", 8338},
		{"ressci.001", 0, "2a68edd064e5e4937b5e9c74b38f2082", 70588050},
		{"resmap.002", 0, "83c2aa4653a985ab4b49ff60532ed08f", 12082},
		{"ressci.002", 0, "2a68edd064e5e4937b5e9c74b38f2082", 128562138},
		{"resmap.003", 0, "31ef4c0621711585d031f0ae81707251", 1636},
		{"ressci.003", 0, "2a68edd064e5e4937b5e9c74b38f2082", 6860492},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// RAMA - Italian Windows CD (from glorifindel)
	{{"rama", "", {
		{"ressci.001", 0, "2a68edd064e5e4937b5e9c74b38f2082", 70611091},
		{"resmap.001", 0, "70ba2ff04a2b7fb2c52420ba7fbd47c2", 8338},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Shivers - English Windows
	{{"shivers", "", {
		{"resmap.000", 0, "f2ead37749ed8f6535a2445a7d05a0cc", 46525},
		{"ressci.000", 0, "4294c6d7510935f2e0a52e302073c951", 262654836},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Shivers - English Windows Demo
	{{"shivers", "Demo", {
		{"resmap.000", 0, "d9e0bc5eddefcbe47f528760085d8927", 1186},
		{"ressci.000", 0, "3a93c6340b54e07e65d0e5583354d186", 10505469},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformWindows, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Shivers - German Windows (from Tobis87)
	{{"shivers", "", {
		{"resmap.000", 0, "f483d0a1f78334c18052e92785c3086e", 46537},
		{"ressci.000", 0, "6751b144671e2deed919eb9d284b07eb", 262390692},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Space Quest 1 VGA Remake - English Amiga (from www.back2roots.org)
	{{"sq1sci", "VGA Remake", {
		{"resource.map", 0, "106484b372af1d4cbf866472cc2813dc", 6396},
		{"resource.000", 0, "cc9d6ace343661ae51ec8bd6e6b00a8c", 340944},
		{"resource.001", 0, "59efcfa2268d2f8608f544e2674d8151", 761721},
		{"resource.002", 0, "f00ef883128bf5fc2fbb888cdd7adf25", 814461},
		{"resource.003", 0, "2588c1c2ca8b9bed0e3411948c0856a9", 839302},
		{"resource.004", 0, "b25a1539c71701f7715f738c5037e9a6", 775515},
		{"resource.005", 0, "640ffe1a9acde392cc33cc1b1a528328", 806324},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

#if 0
	// Space Quest 1 VGA Remake - English DOS
	{{"sq1sci", "VGA Remake", {
		{"resource.map", 0, "38a74d8f555a2da9ca4f21d14e3c1d33", 5913},
		{"resource.000", 0, "e9d866534f8c84de82e25f2631ff258c", 1016436},
		{"resource.001", 0, "a89b7b52064c75b1985b289edc2f5c69", 1038757},
		{"resource.002", 0, "a9e847c687529481f3a22b9bf01f45f7", 1169831},
		{"resource.003", 0, "c47600e50c6fc591957ae0c5020ee7b8", 1213262},
		{"resource.004", 0, "e19ea4ad131472f9238590f2e1d40289", 1203051},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

#if 0
	// Space Quest 1 VGA Remake - Spanish DOS
	{{"sq1sci", "VGA Remake", {
		{"resource.map", 0, "cee2a67fa7f8f1f520f398110ca1c37e", 6111},
		{"resource.000", 0, "945081a73211e0c40e62f709edcd8d1d", 970657},
		{"resource.001", 0, "94692dc84c85c93bb8850f58aebf3cfc", 1085687},
		{"resource.002", 0, "fc9ad3357e4cedec1611ad2b67b193a9", 1175465},
		{"resource.003", 0, "8c22700a02991b763f512f837636b3ca", 1211307},
		{"resource.004", 0, "9b78228ad4f9f335fedf74f1812dcfca", 513325},
		{"resource.005", 0, "7d4ebcb745c0bf8fc42e4013f52ecd49", 1101812},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 510)
	},
#endif

#if 0
	// Space Quest 3 - English Amiga (from www.back2roots.org)
	{{"sq3", "", {
		{"resource.map", 0, "bad41385acde6d677a8d55a7b20437e3", 5868},
		{"resource.001", 0, "ceeda7202b96e5c85ecaa88a40a540fc", 171636},
		{"resource.002", 0, "ceeda7202b96e5c85ecaa88a40a540fc", 754432},
		{"resource.003", 0, "ceeda7202b96e5c85ecaa88a40a540fc", 746496},
		{"resource.004", 0, "ceeda7202b96e5c85ecaa88a40a540fc", 761984},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 453)	// FIXME: some versions are v. 0.000.685/0.000.999
	},
#endif

	// Space Quest 3 - English DOS
	{{"sq3", "", {
		{"resource.map", 0, "55e91aeef1705bce2a9b79172682f36d", 5730},
		{"resource.001", 0, "8b55c4875298f45ea5696a5ee8f6a7fe", 490247},
		{"resource.002", 0, "8b55c4875298f45ea5696a5ee8f6a7fe", 715777},
		{"resource.003", 0, "8b55c4875298f45ea5696a5ee8f6a7fe", 703370},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 685)
	},

#if 0
	// Space Quest 3 - German DOS (from Tobis87)
	{{"sq3", "", {
		{"resource.map", 0, "4965c78b5eff50d5e4148ce114594ba8", 7584},
		{"resource.001", 0, "9107c2aa5398e28b5c5406df13491f85", 117869},
		{"resource.002", 0, "9107c2aa5398e28b5c5406df13491f85", 336101},
		{"resource.003", 0, "9107c2aa5398e28b5c5406df13491f85", 350391},
		{"resource.004", 0, "9107c2aa5398e28b5c5406df13491f85", 349750},
		{"resource.005", 0, "9107c2aa5398e28b5c5406df13491f85", 322107},
		{"resource.006", 0, "9107c2aa5398e28b5c5406df13491f85", 320643},
		{"resource.007", 0, "9107c2aa5398e28b5c5406df13491f85", 344287},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 453)	// FIXME: some versions are v. 0.000.685/0.000.999
	},
#endif

	// Space Quest 4 - English Amiga
	{{"sq4", "", {
		{"resource.map", 0, "d87ae90031e7fd04f32a27db054f5c9c", 6174},
		{"resource.000", 0, "19671ac620a0a4720a1937c20c2e24a1", 323309},
		{"resource.001", 0, "abca51a4c896df550f095a2db71dce46", 805915},
		{"resource.002", 0, "5667852471ba5b7f5b9a770eabd07df2", 796615},
		{"resource.003", 0, "6ec43464f6a17e612636e2928fd9471c", 803868},
		{"resource.004", 0, "1887ed88bb34ae7238650e8f77f26315", 798226},
		{"resource.005", 0, "3540d1cc84d674cf4b2c898b88a3b563", 790296},
		{"resource.006", 0, "ade814bc4d56244c156d9e9bcfebbc11", 664085},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 744)
	},

	// Space Quest 4 - German Amiga (from www.back2roots.org)
	{{"sq4", "", {
		{"resource.map", 0, "79641c0d43408e33c251a1d494d2575e", 6252},
		{"resource.000", 0, "feff51c52146b3a31d4793c718279e13", 345170},
		{"resource.001", 0, "ab33060bfebe32450c0b8d9a3a066efc", 822470},
		{"resource.002", 0, "f79fd6a62da082addb205ed6cef99629", 810458},
		{"resource.003", 0, "f4c21da916f450d4b893b4cba6120866", 815854},
		{"resource.004", 0, "99c6a017da5e769a3b427ca52c8a564f", 824601},
		{"resource.005", 0, "10ee1709e6559c724676d058199b75b5", 818745},
		{"resource.006", 0, "67fb188b191d88efe8414af6ea297b93", 672675},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformAmiga, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 744)
	},

	// Space Quest 4 - English DOS CD
	{{"sq4", "", {
		{"resource.map", 0, "ed90a8e3ccc53af6633ff6ab58392bae", 7054},
		{"resource.000", 0, "63247e3901ab8963d4eece73747832e0", 5157378},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 001, 64)
	},

	// Space Quest 4 - English DOS
	{{"sq4", "", {
		{"resource.map", 0, "a18088c8aceb06025dbc945f29e02935", 5124},
		{"resource.000", 0, "e1f46832cd2458796028e054a0466031", 5502009},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 200)	// FIXME: the executable is version 1.000.753, which doesn't work
	},

#if 0
	// Space Quest 4 - German DOS (from Tobis87)
	{{"sq4", "", {
		{"resource.map", 0, "71715e775e3791178d606cfe6c7e1fb9", 6339},
		{"resource.000", 0, "5f6a1fff40584ee807efd547899b1ba5", 206032},
		{"resource.001", 0, "e924cf86a72ada7736043f045cce345f", 1065442},
		{"resource.002", 0, "e18d731c3fba51333a7f402e454714a5", 858402},
		{"resource.003", 0, "7c2e7508af1a6af877d921e476f70b5e", 1172738},
		{"resource.004", 0, "b8d6efbd3235329bfe844c794097b2c9", 1064761},
		{"resource.005", 0, "47ee647b5b12232d27e63cc627c25899", 1156765},
		{"resource.006", 0, "dfb023e4e2a1e7a00fa18f9ede72a91b", 924059},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 200)
	},
#endif

#if 0
	// Space Quest 4 - Italian DOS Floppy (from glorifindel)
	{{"sq4", "", {
		{"resource.map", 0, "e753dfa96d68dd95f84f6cd80479a35e", 6135},
		{"resource.000", 0, "2ac39ff61e369b79f3d7a4ad514f8e29", 203170},
		{"resource.001", 0, "99a6df6d366b3f061271ff3450ac0d32", 1286269},
		{"resource.002", 0, "a6a8d7a24dbb7a266a26b084e7275e89", 1241124},
		{"resource.003", 0, "5289000399d503b59da9e23129256f1a", 1325546},
		{"resource.004", 0, "4277c61bed40a50dadc4b5a344520af2", 1251000},
		{"resource.005", 0, "5f885abd335978e2fd4e5f886d7676c8", 1102880},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 200)
	},
#endif

#if 0
	// Space Quest 4 - Spanish DOS
	{{"sq4", "", {
		{"resource.map", 0, "51bcb305568ec19713f8b79727f10071", 6159},
		{"resource.000", 0, "8000a55aebc50a68b7cce07a8c33758c", 204315},
		{"resource.001", 0, "99a6df6d366b3f061271ff3450ac0d32", 1269094},
		{"resource.002", 0, "a6a8d7a24dbb7a266a26b084e7275e89", 1240998},
		{"resource.003", 0, "42a307941edeb1a3be31daeb2e4be90b", 1319306},
		{"resource.004", 0, "776fba81c110d1908776232cbe190e20", 1253752},
		{"resource.005", 0, "55fae26c2a92f16ef72c1e216e827c0f", 1098328},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 200)
	},
#endif

#if 0
	// Space Quest 4 - Spanish DOS
	{{"sq4", "", {
		{"resource.map", 0, "41543ae71036046fef69df29a838ee05", 5589},
		{"resource.000", 0, "2ac39ff61e369b79f3d7a4ad514f8e29", 242470},
		{"resource.001", 0, "567608beb69d9dffdb42a8f39cb11a5e", 994323},
		{"resource.002", 0, "74c62fa2146ff3b3b2ea2b3fb95b9af9", 1140801},
		{"resource.003", 0, "42a307941edeb1a3be31daeb2e4be90b", 1088408},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 000, 200)
	},
#endif

#if 0
	// Space Quest 5 - English DOS
	{{"sq5", "", {
		{"resource.map", 0, "66317c12ac6e818d1f7c17e83c1d9819", 6143},
		{"resource.000", 0, "4147edc5045e6d62998018b5614c58ec", 5496486},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Space Quest 5 - English DOS
	{{"sq5", "", {
		{"resource.map", 0, "8bde0a9adb9a3e9aaa861826874c983", 6473},
		{"resource.000", 0, "f4a48705764544d7cc64a7bb22a610df", 6025184},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 67)
	},

#if 0
	// Space Quest 5 - German DOS (from Tobis87)
	{{"sq5", "", {
		{"resource.map", 0, "66317c12ac6e818d1f7c17e83c1d9819", 6143},
		{"resource.000", 0, "4147edc5045e6d62998018b5614c58ec", 5496486},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

#if 0
	// Space Quest 5 - Italian DOS Floppy (from glorifindel)
	{{"sq5", "", {
		{"resource.000", 0, "5040026519f37199f3616fb1d4704dff", 6047170},
		{"resource.map", 0, "5b09168baa2f6e2e22787429b2d72f54", 6492},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// Space Quest 6 - English DOS/Win3.11 CD
	{{"sq6", "", {
		{"resource.map", 0, "6dddfa3a8f3a3a513ec9dfdfae955005", 10528},
		{"resource.000", 0, "c4259ab7355aead07773397b1052827d", 41150806},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 002)
	},

#if 0
	// Space Quest 6 - German DOS (from Tobis87)
	{{"sq6", "", {
		{"resource.map", 0, "664d797415484f85c90b1b45aedc7686", 10534},
		{"resource.000", 0, "ba87ba91e5bdabb4169dd0df75777722", 40933685},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// The Island of Dr. Brain - English 1.1 2.3.93 (from Quietust)
	{{"islandbrain", "", {
		{"resource.map", 0, "3c07da06bdd1689f9d07af78fb94d0ec", 3101},
		{"resource.000", 0, "ecc686e0034fb4d41de077ac7167b3cf", 1947866},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 53)
	},
#if 0
	// The Island of Dr. Brain - English DOS
	{{"islandbrain", "", {
		{"resource.map", 0, "2388efef8430b041b0f3b00b9050e4a2", 3281},
		{"resource.000", 0, "b3acd9b9dd7fe53c4ee133ac9a1acfab", 2103560},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(0, 000, 000)	// FIXME: add version here
	},
#endif

	// The Island of Dr. Brain - English DOS Demo
	{{"islandbrain", "Demo", {
		{"resource.map", 0, "a8e5ca8ed1996974afa59f4c45e06195", 986},
		{"resource.000", 0, "b3acd9b9dd7fe53c4ee133ac9a1acfab", 586560},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(1, 1, 53)
	},

	// Torin's Passage - English Windows
	{{"torin", "", {
		{"resmap.000", 0, "bb3b0b22ff08df54fbe2d06263409be6", 9799},
		{"ressci.000", 0, "693a259d346c9360f4a0c11fdaae430a", 55973887},
		{NULL, 0, NULL, 0}}, Common::EN_ANY, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

	// Torin's Passage - Spanish Windows
	{{"torin", "", {
		{"resmap.000", 0, "bb3b0b22ff08df54fbe2d06263409be6", 9799},
		{"ressci.000", 0, "693a259d346c9360f4a0c11fdaae430a", 55973887},
		{NULL, 0, NULL, 0}}, Common::ES_ESP, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

	// Torin's Passage - French Windows
	{{"torin", "", {
		{"resmap.000", 0, "bb3b0b22ff08df54fbe2d06263409be6", 9799},
		{"ressci.000", 0, "693a259d346c9360f4a0c11fdaae430a", 55973887},
		{NULL, 0, NULL, 0}}, Common::FR_FRA, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

	// Torin's Passage - German Windows
	{{"torin", "", {
		{"resmap.000", 0, "bb3b0b22ff08df54fbe2d06263409be6", 9799},
		{"ressci.000", 0, "693a259d346c9360f4a0c11fdaae430a", 55973887},
		{NULL, 0, NULL, 0}}, Common::DE_DEU, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},

	// Torin's Passage - Italian Windows CD (from glorifindel)
	{{"torin", "", {
		{"resmap.000", 0, "bb3b0b22ff08df54fbe2d06263409be6", 9799},
		{"ressci.000", 0, "693a259d346c9360f4a0c11fdaae430a", 55973887},
		{NULL, 0, NULL, 0}}, Common::IT_ITA, Common::kPlatformWindows, 0},
		{},
		SCI_VERSION_AUTODETECT,
		SCI_VERSION(2, 100, 2)
	},
	
	// SCI Fanmade Games
	FANMADE("Al Pond 2: Island Quest", "9625372e710d1a95d2027b48f9e325af", 1506, "a0f9aa65b9bf3d8703adff5a621f243c", 889843),
	FANMADE("Al Pond: Island Quest 2", "4cba6a5a4c8f66f21935ed78b0511a92", 870, "876587dc9a5ec569287a3dc4b29139d8", 613769),
	FANMADE("Another DG Game: I Want My C64 Back", "4a8ca7ca2abd18899ef856f47665e2e9", 588, "12ff558d20c72e42cc6adb408f34d6d8", 150513),
	FANMADE_L("Another DG Game: I Want My C64 Back", "13dc1d9ebc57daf8895412eee5e39fea", 576, "e2ad60b3a280171429db5c85f158f84a", 141697, Common::FR_FRA),
	FANMADE("Bluntman and Chronic (Politically Correct Version)", "c3ef9fa6c7c5fb840078bf28d87c7f8b", 1362, "441636a9f6f86710844868fded868ee7", 596688),
	FANMADE_V("Cascade Quest", "c94efc10d18c040b6e22a1dc6d3adfe1", 3468, "8ada33dfa945f81531e5508240b573de", 1432195, SCI_VERSION(0, 0, 685)),
	FANMADE_V("Curt Quest 1.0", "b0e555370380d218968a40a68eaaaffc", 1146, "c851182cdf6fc6a81b840f4d4875f1a0", 307165, SCI_VERSION(0, 0, 685)),
	FANMADE_V("Curt Quest 1.1", "54084c29346683296e45ef32d7ae74f3", 1128, "c851182cdf6fc6a81b840f4d4875f1a0", 302000, SCI_VERSION(0, 0, 685)),
	FANMADE("Demo Quest", "c89a0c9e0a4e4af0ecedb300a3b31dbf", 384, "a32f3495ba24764cba091119cc3f1e13", 160098),
	FANMADE("Dr. Jummybummy's Space Adventure 2", "6ae6cb7de423f51736d9487b4ca0c6da", 810, "26e5b563f578e104d79689f36568b7cf", 394670),
	FANMADE_L("Grostesteing: Plus Mechant que Jamais", "ec9a97ccb134f69249f6ea8b16c13d8e", 1500, "b869f5f11bfe2ab5f67f4f0c618f2ce1", 464657, Common::FR_FRA), // FIXME: Accent
	FANMADE("Jim Quest", "0af50be1d3f0cb77a09137709a76ef4f", 960, "9c042c136548b20d9183495668e03526", 496446),
	FANMADE("Knight's Quest Demo 1.0", "5e816edf993956752ed06fccfeeae6d9", 1260, "959321f88a22905fa1f8c6d897874744", 703836),
	FANMADE("LockerGnome Quest", "3eeff9130206cad0c4e1551e2b9dd2c5", 420, "ae05ca90806fd90cc43f147c82d3547c", 158906),
	FANMADE("New Year's Mystery", "efd1beb5120293725065c95959144f81", 714, "b3bd3c2372ed6efa28adb12403c4c31a", 305027),
	FANMADE_V("Osama", "db8f1710453cfbecf4214b2946970043", 390, "7afd75d4620dedf97a84ae8f0a7523cf", 123827, SCI_VERSION(0, 0, 685)),
	FANMADE("Quest for the Cheat", "a359d4cf27f98264b42b55c017671214", 882, "8a943029f73c4bc85d454b7f473455ba", 455209),
	FANMADE_V("SCI Companion Template", "ad54d4f504086cd597aa2348d0aa3b09", 354, "6798b7b601ce8154c1d1bc0f0edcdd18", 113061, SCI_VERSION(0, 0, 685)),
	FANMADE("SCI Studio Template 3.0", "ca0dc8d586e0a8670b7621cde090b532", 354, "58a48ee692a86c0575e6bd0b00a92b9a", 113097),
	FANMADE("SCI Quest", "9067e1f1e54436d2dbfce855524bc84a", 552, "ffa7d355cd9223f245289108a696bcd2", 149634),
	FANMADE("The Legend of the Lost Jewel", "ba1bca315e3818c5626eda51bcfbcccf", 636, "9b0736d69924af0cff32a0f78db96855", 300398),
	
	// FIXME: The vga demo does not have a resource.000/001 file.
	//FANMADE_V("SCI VGA Demo", "00b1abd87bad356b90fcdfcb6132c26f", 8, "", 0, SCI_VERSION(1, 0, 577)),

	{AD_TABLE_END_MARKER, {}, SCI_VERSION_AUTODETECT, SCI_VERSION(0, 000, 000)}
};

/**
 * The fallback game descriptor used by the SCI engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static SciGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		ADGF_NO_FLAGS
	},
	{},
	SCI_VERSION_AUTODETECT,
	SCI_VERSION(0, 000, 000)
};


static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)SciGameDescriptions,
	// Size of that superset structure
	sizeof(SciGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	SciGameTitles,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"sci",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

class SciMetaEngine : public AdvancedMetaEngine {
public:
	SciMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "SCI Engine";
	}

	virtual const char *getCopyright() const {
		return "Sierra's Creative Interpreter (C) Sierra Online";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const;
};


const ADGameDescription *SciMetaEngine::fallbackDetect(const Common::FSList &fslist) const {
	bool foundResMap = false;
	bool foundRes000 = false;
	Common::Platform exePlatform = Common::kPlatformUnknown;
	Common::String exeVersionString;

	// First grab all filenames
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.map") || filename.contains("resmap.000"))
			foundResMap = true;

		if (filename.contains("resource.000") || filename.contains("resource.001")
			|| filename.contains("ressci.000") || filename.contains("ressci.001"))
			foundRes000 = true;

		// Check if it's a known executable name
		if (filename.contains("scidhuv") || filename.contains("sciv") ||
			filename.contains("sierra") || filename.contains("sciw") ||
			filename.contains("prog")) {

			// We already found a valid exe, no need to check this one.
			if (!exeVersionString.empty())
				continue;

			// Is it really an executable file?
			Common::SeekableReadStream *fileStream = file->createReadStream();
			exePlatform = getGameExePlatform(fileStream);

			// It's a valid exe, read the interpreter version string
			if (exePlatform != Common::kPlatformUnknown)
				exeVersionString = readSciVersionFromExe(fileStream, exePlatform);

			delete fileStream;
		}
	}

	if (exePlatform == Common::kPlatformUnknown)
		return 0;

	// If these files aren't found, it can't be SCI
	if (!foundResMap && !foundRes000)
		return 0;

	// Set some defaults
	g_fallbackDesc.desc.gameid = "sci";
	g_fallbackDesc.desc.extra = "";
	g_fallbackDesc.desc.language = Common::UNK_LANG;
	g_fallbackDesc.desc.platform = exePlatform;
	g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;
	g_fallbackDesc.version = SCI_VERSION(0, 0, 0);

	printf("If this is *NOT* a fan-modified version (in particular, not a fan-made\n");
	printf("translation), please, report the data above, including the following\n");
	printf("version number, from the game's executable:\n");

	// Try to parse the executable version
	if (getSciVersionFromString(exeVersionString, &g_fallbackDesc.version, g_fallbackDesc.desc.platform)) {
		printf("Interpreter version: %d.%03d.%03d (got %s by executable scan)\n",
			SCI_VERSION_MAJOR(g_fallbackDesc.version),
			SCI_VERSION_MINOR(g_fallbackDesc.version),
			SCI_VERSION_PATCHLEVEL(g_fallbackDesc.version),
			exeVersionString.c_str());

		return (const ADGameDescription *)&g_fallbackDesc;
	} else {
		printf("Couldn't parse the interpreter version: %s (by executable scan)\n",
			exeVersionString.c_str());
		return NULL;
	}
}

bool SciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	const SciGameDescription *desc = (const SciGameDescription *)gd;

	*engine = new SciEngine(syst, desc);

	return true;
}

} // End of namespace Sci

#if PLUGIN_ENABLED_DYNAMIC(SCI)
	REGISTER_PLUGIN_DYNAMIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#endif
