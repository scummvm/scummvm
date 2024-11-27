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

#ifndef NANCY1DATA_H
#define NANCY1DATA_H

#include "types.h"

const GameConstants _nancy1Constants = {
	11,													// numItems
	168,												// numEventFlags
	{	44, 45, 46, 47, 48, 49, 50, 51, 52, 53,			// genericEventFlags
		63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
		75, 76, 77, 78, 79, 80, 81, 82, 83, 84 },
	4,													// numCursorTypes
	7000,												// logoEndAfter
	42													// wonGameFlagID
};

const Common::Array<uint16> _nancy1MapAccessSceneIDs = {
	9, 10, 11, 666, 888, 1200, 1250, 1666
};

const Common::Array<GameLanguage> _nancy1LanguagesOrder = {
	GameLanguage::kEnglish,
	GameLanguage::kRussian
};

const Common::Array<Common::Array<ConditionalDialogue>> _nancy1ConditionalDialogue = {
{	// Daryl, 18 responses
	{	17, 124, "DIC1",
		{ { kEv, 0x1D, kTrue }, { kEv, 0x39, kFalse } } },
	{	16, 127, "DIC2",
		{ { kEv, 0x13, kTrue }, { kEv, 0x37, kFalse } } },
	{	15, 129, "DIC3",
		{ { kEv, 0xB, kTrue }, { kEv, 0x38, kFalse } } },
	{	14, 131, "DIC4",
		{ { kEv, 0x0, kTrue }, { kEv, 0x1, kFalse }, { kEv, 0x6B, kFalse } } },
	{	13, 132, "DIC5",
		{ { kEv, 0x64, kTrue }, { kEv, 0x1E, kFalse }, { kEv, 0x14, kFalse }, { kEv, 0xC, kFalse }, { kEv, 0x6C, kFalse } } },
	{	12, 134, "DIC6",
		{ { kEv, 0x6D, kFalse }, { kEv, 0x6, kTrue }, { kEv, 0x8, kTrue }, { kEv, 0x5E, kTrue }, { kEv, 0x17, kTrue }, { kEv, 0x24, kTrue }, { kEv, 0x9, kTrue } } },
	{	11, 139, "DIC7",
		{ { kEv, 0x6E, kFalse }, { kEv, 0x24, kTrue }, { kEv, 0x9, kTrue }, { kEv, 0x5E, kFalse }, { kEv, 0x8, kFalse } } },
	{	10, 141, "DIC8",
		{ { kEv, 0x6F, kFalse }, { kEv, 0x5E, kTrue }, { kEv, 0x24, kTrue }, { kEv, 0x9, kTrue }, { kEv, 0x8, kFalse } } },
	{	9, 143, "DIC9",
		{ { kEv, 0x70, kFalse }, { kEv, 0x24, kTrue }, { kEv, 0x9, kTrue }, { kEv, 0x6, kTrue }, { kEv, 0x8, kTrue }, { kEv, 0x5E, kFalse } } },
	{	8, 144, "DIC10",
		{ { kEv, 0x71, kFalse }, { kEv, 0x5E, kTrue }, { kEv, 0x24, kFalse }, { kEv, 0x8, kFalse } } },
	{	7, 145, "DIC10",
		{ { kEv, 0x72, kFalse }, { kEv, 0x5E, kTrue }, { kEv, 0x8, kTrue }, { kEv, 0x6, kTrue }, { kEv, 0x24, kFalse } } },
	{	6, 146, "DIC12",
		{ { kEv, 0x73, kFalse }, { kEv, 0x8, kTrue }, { kEv, 0x6, kTrue }, { kEv, 0x5E, kFalse }, { kEv, 0x24, kFalse } } },
	{	5, 150, "DIC13",
		{ { kEv, 0x74, kFalse }, { kEv, 0x1D, kTrue }, { kEv, 0x13, kTrue }, { kEv, 0xB, kTrue }, { kEv, 0x5E, kFalse }, { kEv, 0x24, kFalse }, { kEv, 0x8, kFalse } } },
	{	4, 151, "DIC14",
		{ { kEv, 0x27, kFalse }, { kEv, 0x5, kTrue } } },
	{	3, 156, "DIC15",
		{ { kEv, 0x28, kTrue }, { kEv, 0x75, kFalse } } },
	{	2, 147, "DIC16",
		{ { kEv, 0xC, kFalse }, { kEv, 0x6, kTrue }, { kEv, 0x76, kFalse } } },
	{	1, 148, "DIC17",
		{ { kEv, 0x14, kFalse }, { kEv, 0x4, kTrue }, { kEv, 0x77, kFalse } } },
	{	0, 149, "DIC18",
		{ { kEv, 0x1E, kFalse }, { kEv, 0x63, kTrue }, { kEv, 0x78, kFalse } } }
},
{	// Connie, 10 responses
	{	26, 233, "CIC1",
		{ { kEv, 0x1D, kTrue }, { kEv, 0x18, kFalse } } },
	{	25, 234, "CIC2",
		{ { kEv, 0x1F, kTrue }, { kEv, 0x19, kFalse } } },
	{	24, 235, "CIC3",
		{ { kEv, 0xB, kTrue }, { kEv, 0x1A, kFalse } } },
	{	23, 236, "CIC4",
		{ { kEv, 0x26, kTrue }, { kEv, 0x1C, kFalse } } },
	{	22, 237, "CIC5",
		{ { kEv, 0, kTrue }, { kEv, 1, kFalse }, { kEv, 0x79, kFalse } } },
	{	21, 238, "CIC6",
		{ { kEv, 2, kTrue }, { kEv, 3, kTrue }, { kEv, 0x17, kFalse } } },
	{	13, 239, "DIC5",
		{ { kEv, 0x64, kTrue }, { kEv, 0x16, kFalse } } },
	{	20, 240, "CIC8",
		{ { kEv, 0x5, kTrue }, { kEv, 0x14, kFalse } } },
	{	19, 245, "CIC9",
		{ { kEv, 0x28, kTrue } } },
	{	18, 231, "CIC10",
		{ { kEv, 0xD, kTrue }, { kEv, 0x5E, kFalse } } }
},
{	// Hal, 9 responses
	{	33, 435, "hic1",
		{ { kEv, 0x1D, kTrue }, { kEv, 0x11, kFalse } } },
	{	16, 437, "DIC2",
		{ { kEv, 0x13, kTrue }, { kEv, 0xE, kFalse } } },
	{	32, 438, "hic3",
		{ { kEv, 0x1B, kTrue }, { kEv, 0xF, kFalse } } },
	{	31, 439, "hic4",
		{ { kEv, 0x26, kTrue }, { kEv, 0x10, kFalse } } },
	{	30, 441, "hic5",
		{ { kEv, 0, kTrue }, { kEv, 1, kFalse }, { kEv, 0x68, kFalse } } },
	{	29, 442, "hic6",
		{ { kEv, 0, kTrue }, { kEv, 1, kFalse }, { kEv, 0x20, kTrue }, { kEv, 0x69, kFalse } } },
	{	13, 443, "DIC5",
		{ { kEv, 0x6A, kFalse }, { kEv, 0x64, kTrue }, { kEv, 0x5, kFalse } } },
	{	28, 444, "hic8",
		{ { kEv, 0x8, kTrue }, { kEv, 0x6, kTrue }, { kEv, 0xC, kFalse } } },
	{	27, 446, "hic9",
		{ { kEv, 0x28, kTrue } } },
},
{	// Hulk, 9 responses
	{	39, 333, "hdic1",
		{ { kEv, 0x13, kTrue }, { kEv, 0x3A, kFalse } } },
	{	24, 336, "CIC3",
		{ { kEv, 0xB, kTrue }, { kEv, 0x25, kFalse } } },
	{	38, 339, "hdic3",
		{ { kEv, 0x12, kTrue }, { kEv, 0x21, kFalse } } },
	{	31, 340, "hic4",
		{ { kEv, 0x26, kTrue }, { kEv, 0x22, kFalse } } },
	{	37, 341, "hdic5",
		{ { kEv, 0, kTrue }, { kEv, 1, kFalse }, { kEv, 0x66, kFalse } } },
	{	13, 342, "DIC5",
		{ { kEv, 0x67, kFalse }, { kEv, 0x64, kTrue } } },
	{	36, 343, "hdic7",
		{ { kEv, 0x63, kTrue }, { kEv, 0x24, kFalse } } },
	{	35, 344, "hdic8",
		{ { kEv, 0x5, kTrue }, { kEv, 0x1E, kFalse } } },
	{	34, 345, "hdic9",
		{ { kEv, 0x28, kTrue } } },
}
};

const Common::Array<Goodbye> _nancy1Goodbyes = {
	{ "nd0d", { { { 3220, 3221, 3222, 3223 }, {}, NOFLAG } } }, // Daryl
	{ "nd0c", { { { 252, 2520, 2521, 2523 }, {}, NOFLAG } } },  // Connie
	{ "nd0hl", { { { 451, 452, 453, 454 }, {}, NOFLAG } } },	 // Hal
	{ "nd0h", { { { 3298, 3296 }, {}, NOFLAG } } }				  // Hulk, only two answers
};

const Common::Array<Common::Array<Hint>> _nancy1Hints = {
{	// Ned, 8 hints
	{	1, -1,
		{ "hn01", "hn02", "hn03" },
		{ { kEv, 0, kFalse } } },
	{	2, -1,
		{ "hn04", "hn05", "hn06" },
		{ { kEv, 0, kTrue }, { kEv, 1, kFalse } } },
	{	3, -1,
		{ "hn07", "hn08", "hn09" },
		{ { kEv, 1, kFalse }, { kIn, 3, kFalse } } },
	{	4, -1,
		{ "hn10", "hn11", "hn09" },
		{ { kEv, 0x55, kFalse }, { kIn, 3, kTrue } } },
	{	5, -1,
		{ "hn13", "hn14", "hn15" },
		{ { kEv, 0x55, kTrue }, { kEv, 0x56, kFalse } } },
	{	6, -1,
		{ "hn16", "hn17", "hn18" },
		{ { kEv, 0x57, kFalse }, { kEv, 0x56, kTrue } } },
	{	7, -1,
		{ "hn21", "hn21", "hn20" },
		{ { kEv, 0xA, kTrue }, { kEv, 0x3B, kTrue }, { kIn, 7, kFalse } } },
	{	0, 0, // Out of hints
		{ "hn19", "hn19", "hn19" },
		{ } }
},
{	// Bess, 9 hints
	{	9, -1,
		{ "hb01", "hb02", "hb03" },
		{ { kEv, 0x57, kFalse } } },
	{	10, -1,
		{ "hb04", "hb05", "hb06" },
		{ { kEv, 0x57, kTrue }, { kEv, 0x3C, kFalse } } },
	{	11, -1,
		{ "hb07", "hb08", "hb09" },
		{ { kEv, 0x5A, kFalse }, { kEv, 0x3C, kTrue }, { kEv, 0x56, kFalse } } },
	{	12, -1,
		{ "hb11", "hb10", "hb12" },
		{ { kEv, 0x5A, kTrue }, { kEv, 0x56, kFalse } } },
	{	13, -1,
		{ "hb14", "hb15", "hb16" },
		{ { kEv, 0x5A, kFalse }, { kEv, 0x3C, kTrue }, { kEv, 0x56, kTrue } } },
	{	14, -1,
		{ "hb17", "hb18", "hb19" },
		{ { kEv, 0x59, kTrue }, { kEv, 0xA, kFalse }, { kEv, 0x56, kTrue }, { kIn, 0, kFalse } } },
	{	15, -1,
		{ "hb20", "hb21", "hb22" },
		{ { kEv, 0xA, kTrue }, { kEv, 0x3B, kTrue }, { kIn, 0, kTrue }, { kIn, 7, kFalse } } },
	{	16, -1,
		{ "hb24", "hb23", "hb25" },
		{ { kEv, 0x59, kFalse }, { kEv, 0xA, kTrue }, { kEv, 0x3B, kTrue }, { kIn, 7, kFalse } } },
	{	8, 0, // Out of hints
		{ "hb26", "hb26", "hb26" },
		{ } }
},
{	// George, 9 hints
	{	25, -1, // Easter egg
		{ "GeorBark", "GeorBark", "GeorBark" },
		{ { kEv, 0x4A, kTrue } } },
	{	18, -1,
		{ "hg01", "hg02", "hg03" },
		{ { kEv, 0x5B, kFalse } } },
	{	19, -1,
		{ "hg16", "hg15", "hg17" },
		{ { kEv, 0x5B, kTrue }, { kIn, 9, kFalse } } },
	{	20, -1,
		{ "hg18", "hg19", "hg20" },
		{ { kEv, 0x5B, kTrue }, { kEv, 0x5C, kFalse }, { kEv, 0x5D, kFalse }, { kIn, 9, kTrue } } },
	{	21, -1,
		{ "hg08", "hg09", "hg10" },
		{ { kEv, 0x5B, kTrue }, { kEv, 0x5C, kTrue }, { kEv, 0x5D, kFalse }, { kIn, 9, kFalse } } },
	{	22, -1,
		{ "hg04", "hg05", "hg06" },
		{ { kEv, 0x5B, kTrue }, { kEv, 0x5C, kTrue }, { kEv, 0x5D, kTrue }, { kEv, 0x3B, kFalse }, { kIn, 9, kTrue } } },
	{	23, -1,
		{ "hg22", "hg21", "hg13" },
		{ { kEv, 0xA, kFalse }, { kEv, 0x3B, kTrue }, { kIn, 9, kTrue } } },
	{	24, -1,
		{ "hg11", "hg12", "hg13" },
		{ { kEv, 0x3B, kTrue }, { kEv, 0xA, kTrue }, { kIn, 7, kFalse } } },
	{	17, 0, // Out of hints
		{ "hg14", "hg14", "hg14" },
		{ }, }
}
};

const SceneChangeDescription _nancy1HintSceneChange = { 501, 0, 0, true };

const Common::Array<Common::Array<const char *>> _nancy1ConditionalDialogueTexts = {
{	// English
	// 00
	"<c1>W<c0>hat do you know about the break-in at the pharmacy?<h><n>",
	"<c1>W<c0>as Jake interested in Judo?<h><n>",
	"<c1>W<c0>hy would Jake have an old English book in his locker?<h><n>",
	"<c1>D<c0>aryl, we're going to find the person who killed Jake.  If you help out now, the case will move a lot quicker.<h><n>",
	"<c1>I<c0> saw Jake's tape, Daryl.  I know he was blackmailing you.<h><n>",
	// 05
	"<c1>H<c0>al, Connie, and Hulk didn't seem to like Jake very much.  I think they all know something about Jake's death, I just don't know what.<h><n>",
	"<c1>H<c0>al had a reason to hate Jake, but it's hard to picture him as a murderer.<h><n>",
	"<c1>J<c0>ake had some kind of hold on Connie and Hal.  Is it possible that one of them could have resorted to murder?<h><n>",
	"<c1>C<c0>onnie lied about her dating Jake.  Could something have happened between them that would push her to murder?<h><n>",
	"<c1>J<c0>ake was pressuring both Hal and Hulk.  It could have been either of them.  This is really complicated.<h><n>",
	// 10
	"<c1>L<c0>ooks like Jake had a hold on both Hulk and Connie.  What now?<h><n>",
	"<c1>I<c0> think Jake had some sensitive information on Hulk Sanchez.  Do you think Hulk could have killed Jake?<h><n>",
	"<c1>H<c0>al, Hulk and Connie were all involved with Jake.  He had information that could jeopardize Hal's career.  Connie once dated Jake and Hulk seems awfully touchy about that break-in at the Drug Depot.<h><n>",
	"<c1>D<c0>o you know why Jake had a video camera in his locker?<h><n>",
	"<c1>D<c0>aryl, do you know where I could get Jake Roger's locker combination?<h><n>",
	// 15
	"<c1>W<c0>hat can you tell me about Hal Tanaka?<h><n>",
	"<c1>D<c0>o you know Connie Watson?<h><n>",
	"<c1>H<c0>ow well do you know Hulk Sanchez?<h><n>",
	"<c1>D<c0>idn't I hear you were dating Jake?<h><n>",
	"<c1>C<c0>onnie, we're going to find the person who killed Jake.  If you help out now, the case will move a lot quicker.<h><n>",
	// 20
	"<c1>I<c0> know you're the unknown winner of that judo competition.  Jake Rogers had it all on videotape.<h><n>",
	"<c1>Y<c0>ou're wearing a Japanese medallion with a symbol that means crane, and Crane is the name of the judo school on the poster in the gym.<h><n>",
	"<c1>D<c0>o you know the combination to Jake Roger's locker?<h><n>", // Misspelled in the original game
	"<c1>H<c0>ow well do you know Daryl Gray?<h><n>",
	"<c1>D<c0>o you know Hal Tanaka?<h><n>",
	// 25
	"<c1>H<c0>ulk told me money's been tight for you these days.<h><n>",
	"<c1>W<c0>hat do you know about Hulk Sanchez?<h><n>",
	"<c1>H<c0>al, we're going to find the person who killed Jake.  If you help out now, this case will move a lot quicker.<h><n>",
	"<c1>J<c0>ake knew you copied your essay from that book of English essays, didn't he?<h><n>",
	"<c1>H<c0>ulk said your locker was right next to Jake's.  Are you sure you don't know the combination?<h><n>",
	// 30
	"<c1>D<c0>o you know the combination to Jake Rogers' locker?<h><n>",
	"<c1>W<c0>hat can you tell me about Daryl Gray?<h><n>",
	"<c1>C<c0>onnie told me you study too hard.  Is that true?<h><n>",
	"<c1>H<c0>ave you heard of Hulk Sanchez?<h><n>",
	"<c1>H<c0>ulk, we're going to find the person who killed Jake.  If you help out now, this case will move a lot quicker.<h><n>",
	// 35
	"<c1>J<c0>ake knew you broke into the Drug Depot.  He was blackmailing you, wasn't he?<h><n>",
	"<c1>T<c0>ell me about the robbery at the Drug Depot pharmacy.<h><n>",
	"<c1>H<c0>ow could I get into Jake's locker?<h><n>",
	"<c1>I<c0>'m really sorry you got injured.  Does that affect your chances of playing college ball?<h><n>",
	"<c1>W<c0>hat can you tell me about Connie Watson?<h><n>",
},
{	// Russian
	// 00
	"<c1>Wto t= znaew% o krage v apteke?<c0><h><n>",
	"<c1>Dgek zanimals* dzydo?<c0><h><n>",
	"<c1>Poqemu Dgek xranil v svoem wkafqike staruy knihu?<c0><h><n>",
	"<c1>D&ril, m= sobiraems* pojmat% ubijcu. I nam oqen% nugna tvo* pomoQ%.<c0><h><n>",
	"<c1>D&ril, * videla videokassetu Dgeka. Y znay, qto on teb* wantagiroval.<c0><h><n>",
	// 05
	"<c1>Xolu, Konni i Xalku Dgek ne osobo nravils*. Dumay, im qto-to izvestno o eho smerti, no oni ne xot*t hovorit%.<c0><h><n>",
	"<c1>U Xola b=li priqin= nenavidet% Dgeka. No predstavit% eho ubijcej slogno.<c0><h><n>",
	"<c1>Dgek wantagiroval Konni i Xola. Moh kto-to iz nix pojti na ubijstvo?<c0><h><n>",
	"<c1>Konni ne skazala, qto <n>vstreqalas% s Dgekom. <n>Ona mohla pojti na <n>ubijstvo, potomu qto <n>oni possorilis%?<c0><h><n>",
	"<c1>Dgek wantagiroval i Xola, i Xalka. U oboix b=l motiv.<c0><h><n>",
	// 10
	"<c1>Poxoge, Dgek wantagiroval i Xola, i Konni. Wto teper%?<c0><h><n>",
	"<c1>Dgeku b=lo qto-to izvestno o Xalke Sanqese. Dumaew%, Xalk moh ubit% eho?<c0><h><n>",
	"<c1>Xol, Xalk i Konni b=li kak-to sv*zan= s Dgekom. U neho b=li svedeni*, kotor=e mohli navredit% kar%ere Xola. <n>Konni odin raz xodila s Dgekom na svidanie, a Xalka zadel moj vopros o krage v apteke.<c0><h><n>",
	"<c1>T= znaew%, zaqem Dgek xranil v svoem wkafqike videokameru?<c0><h><n>",
	"<c1>D&ril, kak mne uznat% kod k wkafqiku Dgeka?<c0><h><n>",
	// 15
	"<c1>Wto t= znaew% o Xole Tanake?<c0><h><n>",
	"<c1>T= znaew% Konni Vatson?<c0><h><n>",
	"<c1>T= xorowo znaew% Xalka Sanqesa?<c0><h><n>",
	"<c1>Y sl=wala, qto t= vstreqalas% s Dgekom.<c0><h><n>",
	"<c1>Konni, m= sobiraems* najti ubijcu Dgeka. Nam oqen% nugna tvo* pomoQ%.<c0><h><n>",
	// 20
	"<c1>Y znay, qto t= pobedila v sorevnovani*x po dzydo. Dgek Rodgers zapisal vse na video.<c0><h><n>",
	"<c1>U teb* na medal%one narisovan ierohlif 'guravl%'. Toqno tak ge naz=vaets* wkola dzydo. <n>Y videla plakat vozle sportzala.<c0><h><n>",
	"<c1>T= znaew% kod k wkafqiku Dgeka Rodgersa?<c0><h><n>",
	"<c1>T= xorowo znaew% D&rila Hre*?<c0><h><n>",
	"<c1>T= znaew% Xola Tanaku?<c0><h><n>",
	// 25
	"<c1>Xalk skazal, qto u teb* sejqas trudn=e vremena. Cto pravda?<c0><h><n>",
	"<c1>Wto t= znaew% o Xalke Sanqese?<c0><h><n>",
	"<c1>Xol, m= sobiraems* pojmat% <n>ubijcu Dgeka. Nam oqen% nugna <n>tvo* pomoQ%.<c0><h><n>",
	"<c1>Dgek znal, qto t= spisal <n>soqinenie iz knihi. Y prava?<c0><h><n>",
	"<c1>Xalk skazal, qto tvoj wkafqik <n>naxodits* r*dom so wkafqikom <n>Dgeka. T= toqno ne znaew% <n>kod ot eho zamka?<c0><h><n>",
	// 30
	"<c1>T= znaew% kod k wkafqiku <n>Dgeka Rodgersa?<c0><h><n>",
	"<c1>Wto t= znaew% o <n>D&rile Hree?<c0><h><n>",
	"<c1>Konni skazala, qto t= <n>sliwkom mnoho zanimaew%s*.<c0><h><n>",
	"<c1>Wto t= znaew% o <n>Xalke Sanqese?<c0><h><n>",
	"<c1>Xalk, m= sobiraems* pojmat% ubijcu Dgeka. Nam oqen% nugna tvo* pomoQ%.<c0><h><n>",
	// 35
	"<c1>Dgek znal, qto t= soverwil kragu v apteke. On wantagiroval teb*?<c0><h><n>",
	"<c1>Rasskagi o krage v apteke.<c0><h><n>",
	"<c1>T= znaew%, kak otkr=t% wkafqik Dgeka?<c0><h><n>",
	"<c1>Mne gal%, qto t= poluqil travmu. Teper% u teb* budet men%we wansov postupit% v prestign=j kolledg?<c0><h><n>",
	"<c1>Wto t= mogew% skazat% o <n>Konni Vatson?<c0><h><n>"
}
};

const Common::Array<Common::Array<const char *>> _nancy1GoodbyeTexts = {
{	// English
	"<c1>S<c0>ee ya' later.<h>",		  // Daryl
	"<c1>G<c0>ood Bye.<h>",				 // Connie
	"<c1>T<c0>alk to ya' later.<h>",	 // Hal
	"<c1>B<c0>ye.<h>"						 // Hulk
},
{	// Russian
	"<c1>Udaqi.<c0><h>",					 // Daryl
	"<c1>Poka.<c0><h>",					  // Connie
	"<c1>Pohovorim pozge.<c0><h>",		// Hal
	"<c1>Poka.<c0><h>"						// Hulk
}
};

const Common::Array<Common::Array<const char *>> _nancy1HintTexts = {
{	// English
	// 00
	"Nancy, I don't know how else to help.<n>Just be careful, OK?<n><e>",
	"Nancy, I don't know how else to help.<n>Just be careful, OK?<n><e>",
	"Nancy, I don't know how else to help.<n>Just be careful, OK?<n><e>",
	// 01
	"Try to find the victim's locker,<n>it may hold some clues.<n><e>",
	"I'd definitely search the crime<n>scene for clues.<n><e>",
	"Nancy, put on your detective's cap<n>and begin at the beginning!<n><e>",
	// 02
	"Nancy, to open the victim's<n>locker, I'd think of a way to make<n>the owner's name into<n>numbers, like maybe on a phone.<n><e>",
	"On my locker, my combination<n>is related to my name.<n><e>",
	"To open the victim's locker,<n>I'd think of how letters and<n>numbers could be related.<n><e>",
	// 03
	"Are you sure nothing fell out<n>of Jake's locker and onto the floor?<n><e>",
	"Search that locker, there's<n>bound to be something useful<n>there.<n><e>",
	"Nancy, be careful. I was<n>afraid that Jake's locker was<n>booby trapped.<n><e>",
	// 04
	"Use the glasscutter you found<n>by the locker to open up<n>a window, like outside<n>the school.<n><e>",
	"That Jake Rogers was sneaky,<n>he probably used that glasscutter<n>in his locker to break into<n>the school.<n><e>",
	"Nancy, be careful. I was<n>afraid that Jake's locker was<n>booby trapped.<n><e>",
	// 05
	"There must be a computer in<n>the teacher's lounge.<n>Can you access it?<n><e>",
	"There must be clues in the<n>teacher's lounge somewhere.<n>Search around the desk!<n><e>",
	"I can't believe you broke into<n>the teacher's lounge!<n>I'm sure you'll find some very<n>important information there!<n><e>",
	// 06
	"Nancy perhaps you can use your<n>Aunt Eloise's login and<n>password to access<n>the school computer.  Doesn't<n>she keep the password in her safe<n>at home?<n><e>",
	"Nancy perhaps you can use your<n>Aunt Eloise's login and<n>password to access<n>the school computer.  She's<n>probably hidden it somewhere at<n>home in a safe place.<n><e>",
	"Try your Aunt Eloise's name to<n>log onto the school computer.<n>She must keep her password<n>and login at home somewhere.<n><e>",
	// 07
	"I bet the first boiler lever<n>controls whether the second<n>and third lever can move.<n><e>",
	"I bet the first boiler lever<n>controls whether the second<n>and third lever can move.<n><e>",
	"I think the level of each<n>lever may have something to do<n>with reducing the boiler<n>pressure and temperature.<n><e>",
	// 08
	"Nancy, I've run out of ideas.<n>Please be careful, okay?<n><e>",
	"Nancy, I've run out of ideas.<n>Please be careful, okay?<n><e>",
	"Nancy, I've run out of ideas.<n>Please be careful, okay?<n><e>",
	// 09
	"Oh, I'm sure your Aunt Eloise<n>will be helpful.  She<n>just LOVES to hide things!<n><e>",
	"Oh, I'm sure your Aunt Eloise<n>will be helpful.<n><e>",
	"How is your Aunt Eloise?<n>Her house is so beautiful!<n><e>",
	// 10
	"Those letters on your Aunt's safe<n>are definitely Greek letters.<n>I bet the combination is related<n>to the Greek letters on that note<n>from her college sorority!<n><e>",
	"Those letters on your Aunt's safe<n>are definitely Greek letters.<n>I bet there's a note somewhere<n>in the house that also has<n>Greek letters.<n><e>",
	"Those letters on your Aunt's safe<n>are definitely Greek letters.<n><e>",
	// 11
	"That box in Aunt Eloise's safe<n>must hold an important clue!<n><e>",
	"Aunt Eloise must have some important<n>items in the safe.<n><e>",
	"This is such a puzzling situation!<n><e>",
	// 12
	"Can you use Aunt Eloise's login<n>and password to access the <n>school administration computers?<n>I bet they're in the teacher's<n>lounge.<n><e>",
	"Can you use Aunt Eloise's login<n>and password to access the <n>school administration computer?<n><e>",
	"Oh Nancy, are you sure your Aunt<n>wouldn't mind you poking around<n>with her school stuff?<n><e>",
	// 13
	"I bet that box in Aunt Eloise's<n>safe hides her login ID<n>and password.<n><e>",
	"Aunt Eloise probably hid her<n>login ID and password in<n>the safe at home.<n><e>",
	"Aunt Eloise just loves to hide things!<n>Why not go to her house and take<n>a look around?<n><e>",
	// 14
	"This sounds dangerous, Nancy!<n>You'll need to find something in the<n>kitchen that will prop up the<n>gas line so you can take<n>the bolt cutters!<n><e>",
	"Could something take the place<n>of the bolt cutters and hold<n>up that gas line?<n><e>",
	"Nancy, this sounds too dangerous!<n>There must be another way to prop<n>up that gas line instead<n>of the bolt cutter!<n><e>",
	// 15
	"Well, the bolt cutters cut the chain.<n>Can you move the levers on the<n>boiler to change the<n>dials so they match<n>the poster with the gauges?<n><e>",
	"There must be a poster down there<n>that shows the right position<n>of the levers so the pressure<n>goes down!<n><e>",
	"Can you find the right level<n>for the levers so the pressure<n>will lower?<n><e>",
	// 16
	"Nancy!  Could that correct combination<n>be in the boiler room?<n>Maybe written on a wall<n>or something.<n><e>",
	"Nancy!  Could that correct combination<n>be in the boiler room?<n><e>",
	"Nancy!  Have you searched the boiler<n>room for a clue?<n><e>",
	// 17
	"I'm stumped, Nancy.<n>I'm afraid I'm not very much help.<n><e>",
	"I'm stumped, Nancy.<n>I'm afraid I'm not very much help.<n><e>",
	"I'm stumped, Nancy.<n>I'm afraid I'm not very much help.<n><e>",
	// 18
	"Why not go to the school library<n>and look around.  There's<n>always so much to learn there.<n><e>",
	"Why don't you head for the library?<n>You might find some interesting<n>information there.<n><e>",
	"There must be some place to go where<n>you can find out lots of information.<n><e>",
	// 19
	"I bet an extra key to the library<n>is hidden in Aunt Eloise's house.<n>Go check the walls.  Maybe there's<n>a secret compartment!<n><e>",
	"I bet an extra key to the library<n>is hidden in Aunt Eloise's house.<n><e>",
	"Your Aunt Eloise is head librarian.<n>Have her open up the school library.<n><e>",
	// 20
	"Have you checked the school basement?<n>You should find the maintenance door<n>and take a look down there!<n><e>",
	"There must be other places to<n>investigate around the school.<n>Have you checked the basement?<n><e>",
	"There must be other places<n>around the school to investigate<n>Have you checked all over?<n><e>",
	// 21
	"That maintenance door lock is definitely<n>in Braille.<n>I'd look in the school computer<n>for a password.<n><e>",
	"The maintenance door lock uses Braille;<n>you can probably find a password<n>on the school computer.<n><e>",
	"That maintenance door lock is definitely<n>in Braille, but I bet the password isn't!<n><e>",
	// 22
	"You found the password!<n>The encyclopedias in the school<n>library can help you to translate<n>the maintenance door's password<n>into Braille.<n><e>",
	"You found the password!<n>Now you have to translate it into Braille<n>at the library.<n><e>",
	"You found the password!<n>Now you'll have to translate it<n>into Braille.<n><e>",
	// 23
	"I bet the school basement is full<n>of clues.  Be careful, Nancy,<n>I have a bad feeling about<n>that old boiler.<n><e>",
	"I bet the school basement is full of clues.<n><e>",
	"If I were the boiler room room supervisor,<n>I would keep the combination on<n>something close by.<n><e>",
	// 24
	"I would check out the numbers<n>on that stone that tells us<n>when the school was built.<n><e>",
	"I would look around the boiler room<n>for the combination.<n>It's a smart place to start.<n><e>",
	"If I were the boiler room room supervisor,<n>I would keep the combination on<n>something close by.<n><e>",
	// 25
	"Get down on your knees and bark<n>like a dog!<n><e>",
	"Get down on your knees and bark<n>like a dog!<n><e>",
	"Get down on your knees and bark<n>like a dog!<n><e>"
},
{	// Russian
	// 00
	"N&nsi, * ne znay, qem pomoq%. <n>Bud% ostorogna.<n><e>",
	"N&nsi, * ne znay, qem pomoq%. <n>Bud% ostorogna.<n><e>",
	"N&nsi, * ne znay, qem pomoq%. <n>Bud% ostorogna.<n><e>",
	// 01
	"Najdi wkafqik ubitoho. <n>Tam mohut b=t% uliki.<n><e>",
	"Y b= na tvoem meste ob=skal <n>mesto prestupleni*.<n><e>",
	"N&nsi, poprobuj naqat% s <n>samoho naqala!<n><e>",
	// 02
	"N&nsi, poprobuj otkr=t% <n>wkafqik Dgeka. Navernoe, u neho <n>ne oqen% slogn=j kod. <n>Moget b=t%, u teb* vse <n>poluqits*, esli zamenit% <n>bukv= v eho imeni na <n>cifr=. Kak na telefone.<n><e>",
	"Kod k moemu wkafqiku sv*zan <n>s moim imenem.<n><e>",
	"Wtob= uznat% kod k wkafqiku <n>Dgeka, nugno zamenit% bukv= v <n>eho imeni na cifr=.<n><e>",
	// 03
	"T= xorowo osmotrela wkafqik <n>Dgeka? Moget, iz neho qto-to <n>v=palo?<n><e>",
	"Osmotri wkafqik. Tam dolgno <n>b=t% qto-to interesnoe.<n><e>",
	"N&nsi, bud% ostorogna. <n>)kafqik Dgeka opeqatan <n>policiej.<n><e>",
	// 04
	"Ispol%zuj steklorez, qtob= <n>otkr=t% okno s bokovoj <n>storon= wkol=.<n><e>",
	"Navernoe, Dgek Rodgers <n>ispol%zoval steklorez ili <n>nog, qtob= popast% v <n>wkolu.<n><e>",
	"N&nsi, bud% ostorogna. <n>)kafqik Dgeka opeqatan <n>policiej.<n><e>",
	// 05
	"V uqitel%skoj dolgen b=t% <n>komp%yter. T= mogew% tuda <n>popast%?<n><e>",
	"V uqitel%skoj dolgn= b=t% <n>kakie-to uliki. Osmotri stol <n>s komp%yterom.<n><e>",
	"Ne mohu poverit%, qto t= <n>popala v uqitel%skuy! <n>Dumay, t= najdew% tam <n>qto-to vagnoe.<n><e>",
	// 06
	"Tebe nugno vvesti v komp%yter <n>parol% i lohin teti Cloiz=. <n>Po-moemu, ona xranit &ti dann=e <n>v sejfe u seb* doma.<n><e>",
	"Tebe nugno vvesti v komp%yter <n>parol% i lohin teti Cloiz=. <n>Navernoe, ona xranit &ti dann=e <n>u seb* doma.<n><e>",
	"Wtob= vojti v komp%yter, tebe <n>nugno znat% parol% i lohin <n>teti Cloiz=. Vozmogno, ona <n>xranit &ti dann=e u seb* doma.<n><e>",
	// 07
	"Skoree vseho, perv=j <n>r=qah kotla upravl*et vtor=m <n>i tret%im.<n><e>",
	"Skoree vseho, perv=j <n>r=qah kotla upravl*et vtor=m <n>i tret%im.<n><e>",
	"Dumay, s pomoQ%y r=qahov <n>mogno ponigat% v kotle <n>davlenie i temperaturu vod=.<n><e>",
	// 08
	"N&nsi, u men* net nikakix idej. <n>Bud% ostorogna, ladno?<n><e>",
	"N&nsi, u men* net nikakix idej. <n>Bud% ostorogna, ladno?<n><e>",
	"N&nsi, u men* net nikakix idej. <n>Bud% ostorogna, ladno?<n><e>",
	// 09
	"Dumay, tebe pomoget tet* <n>Cloiza. Ona OBOGAET <n>pr*tat% veQi!<n><e>",
	"Dumay, tebe pomoget tet* Cloiza.<n><e>",
	"Kak pogivaet tet* Cloiza? <n>U nee takoj krasiv=j dom!<n><e>",
	// 10
	"Na zamke sejfa bukv= hreqeskoho <n>alfavita, a v sekretere teti <n>Cloiz= est% zapiska s hreqeskimi <n>bukvami. <n>Navern*ka, oni kak-to sv*zan=!<n><e>",
	"Na zamke sejfa bukv= hreqeskoho <n>alfavita. Navern*ka, hde-to v dome teti <n>Cloiz= est% zapiska s hreqeskimi <n>bukvami.<n><e>",
	"Na zamke sejfa bukv= hreqeskoho <n>alfavita.<n><e>",
	// 11
	"Kagets*, v wkatulke sejfa est% <n>qto-to vagnoe!<n><e>",
	"Dumay, tet* Cloiza xranit v <n>sejfe qto-to vagnoe.<n><e>",
	"Vot tak zadaqka!<n><e>",
	// 12
	"T= mogew% ispol%zovat% lohin i <n>parol% teti Cloiz=, qtob= vojti <n>v hlavn=j wkol%n=j komp%yter. <n>Navern*ka, on naxodits* v <n>uqitel%skoj.<n><e>",
	"T= mogew% ispol%zovat% lohin i <n>parol% teti Cloiz=, qtob= vojti <n>v hlavn=j komp%yter wkol=.<n><e>",
	"N&nsi, a t= uverena, qto tet* <n>Cloiza razrewila tebe vse <n>osmotret%?<n><e>",
	// 13
	"Tet* Cloiza xranit svoj lohin <n>i parol% v wkatulke, kotora* <n>stoit v sejfe.<n><e>",
	"Skoree vseho, tet* Cloiza <n>pr*qet svoj lohin i parol% <n>v sejfe.<n><e>",
	"Tet* Cloiza obogaet pr*tat% <n>veQi! Tebe nugno kak sleduet <n>osmotret% ee dom.<n><e>",
	// 14
	"N&nsi, &to oqen% opasno! <n>Tebe nugno najti to, qto budet <n>dergat% hazovuy trubu. <n>Tohda t= smogew% vz*t% <n>boltorez!<n><e>",
	"PoiQi to, qem mogno zamenit% <n>boltorez pod hazovoj truboj.<n><e>",
	"N&nsi, &to oqen% opasno!<n>PoiQi to, qto budet dergat%<n>hazovuy trubu vmesto<n>boltoreza.<n><e>",
	// 15
	"(ep% pererezana. Teper% tebe <n>nugno dvihat% r=qahi kotla, <n>qtob= pokazani* datqikov <n>sootvetstvovali sxeme.<n><e>",
	"V kotel%noj dolgna b=t% <n>sxema s pravil%n=mi <n>pokazani*mi priborov.<n><e>",
	"Tebe nugno dvihat% r=qahi, <n>qtob= snizit% davlenie.<n><e>",
	// 16
	"N&nsi! Moget b=t%, kod hde-to <n>v kotel%noj? Vnimatel%no <n>osmotri sten=.<n><e>",
	"N&nsi! Moget b=t%, kod hde-to <n>v kotel%noj?<n><e>",
	"T= xorowo osmotrela kotel%nuy?<n><e>",
	// 17
	"Izvini, N&nsi. U men* net <n>nikakix idej.<n><e>",
	"Izvini, N&nsi. U men* net <n>nikakix idej.<n><e>",
	"Izvini, N&nsi. U men* net <n>nikakix idej.<n><e>",
	// 18
	"Pojdi v biblioteku i vse tam <n>osmotri. Dumay, t= uznaew% <n>mnoho interesnoho.<n><e>",
	"Poqemu b= tebe ne sxodit% v <n>biblioteku? T= najdew% tam <n>mnoho interesnoho.<n><e>",
	"Tebe nugno pojti tuda, hde <n>mogno poqitat% interesn=e <n>knihi.<n><e>",
	// 19
	"Y dumay, zapasnoj klyq ot <n>biblioteki spr*tan v dome <n>teti Cloiz=. Prover% sten=. <n>Tam dolgen b=t% sejf.<n><e>",
	"PoiQi zapasnoj klyq <n>ot biblioteki v <n>dome teti Cloiz=.<n><e>",
	"Tet* Cloiza - bibliotekar%. U nee <n>navern*ka est% klyq ot <n>biblioteki.<n><e>",
	// 20
	"Vnimatel%no osmotri <n>kotel%nuy.<n><e>",
	"V wkole mnoho razn=x <n>pomeQenij. T= uge <n>osmotrela podval?<n><e>",
	"V wkole est% mnoho razn=x <n>pomeQenij. T= uge vse <n>osmotrela?<n><e>",
	// 21
	"Na zamke kotel%noj bukv= <n>napisan= wriftom Brajl*. <n>PoiQi parol% v <n>komp%ytere.<n><e>",
	"Na zamke kotel%noj bukv= <n>napisan= wriftom Brajl*. <n>PoiQi parol% v <n>komp%ytere.<n><e>",
	"Bukv= na zamke kotel%noj - <n>wrift Brajl*. Takim ge nugno <n>sdelat% i parol%.<n><e>",
	// 22
	"T= znaew% parol% v kotel%nuy! <n>V biblioteke est% &nciklopedi*, <n>kotora* pomoget perevesti eho <n>v wrift Brajl*.<n><e>",
	"T= znaew% parol%! Teper% <n>nugno perevesti eho v wrift <n>Brajl*.<n><e>",
	"T= znaew% parol%! Teper% nugno <n>perevesti eho v wrift Brajl*.<n><e>",
	// 23
	"Dumay, v podvale est% mnoho <n>ulik. N&nsi, bud% ostorogna <n>so star=m kotlom.<n><e>",
	"Dumay, v podvale est% mnoho <n>ulik.<n><e>",
	"Esli b= * rabotala v kotel%noj, <n>* zapisala b= kod na samom <n>vidnom meste.<n><e>",
	// 24
	"Vnimatel%no posmotri na <n>datu osnovani* wkol=.<n><e>",
	"Vnimatel%no osmotri kotel%nuy. <n>Kod dolgen b=t% hde-to tam.<n><e>",
	"Esli b= * rabotala v kotel%noj, <n>* zapisala b= kod na samom <n>vidnom meste.<n><e>",
	// 25
	"Opustis% na koleni i laj, kak sobaka!<n><e>",
	"Opustis% na koleni i laj, kak sobaka!<n><e>",
	"Opustis% na koleni i laj, kak sobaka!<n><e>",
}
};

const Common::Array<const char *> _nancy1TelephoneRinging = {
	"ringing...<n><e>", // English
	"Hudki...  <n><e>"  // Russian
};

const Common::Array<const char *> _nancy1EmptySaveStrings = {
	"-- Empty --",	// English
	"- - - - -  "	// Russian
};

const Common::Array<const char *> _nancy1EventFlagNames = {
	"Tried the locker",
	"Locker open",
	"Read Kanji",
	"Seen the poster",
	"Has magazine",
	"Viewed the tape",
	"Read Literature book",
	"Gone To Teachers Lounge (Found Backpack)",
	"Seen Paper",
	"Researched drug",
	"Has Letter",
	"Met Hal",
	"Hal confessed",
	"Hal said date",
	"Hal told Connie",
	"Hal said lie",
	"Hal told Daryl",
	"Hal told Hulk",
	"Hal said injury",
	"Met Connie",
	"Connie confessed",
	"Connie suspicious",
	"Connie worried",
	"Connie scared",
	"Connie told Hulk",
	"Connie said lie",
	"Connie told Hal",
	"Connie said load",
	"Connie told Daryl",
	"Met Hulk",
	"Hulk confessed",
	"Hulk said money",
	"Hulk said locker",
	"Hulk said lie",
	"Hulk told Daryl",
	"Hulk worried",
	"Hulk angry",
	"Hulk told Hal",
	"Met Daryl",
	"Daryl confessed",
	"Set up sting",
	"Time for end game",
	"Player won game",
	"Stop player scrolling",
	"Generic 0",
	"Generic 1",
	"Generic 2",
	"Generic 3",
	"Generic 4",
	"Generic 5",
	"Generic 6",
	"Generic 7",
	"Generic 8",
	"Generic 9",
	"Jukebox Is Playing",
	"Daryl talked about Connie",
	"Daryl talked about Hal",
	"Daryl talked about Hulk",
	"Hulk told Connie",
	"Solved Boiler Door Puzzle",
	"Solved Aunt Safe Puzzle",
	"Boiler Has Chains on it",
	"Solved Boiler Lever Puzzle",
	"Generic 10",
	"Generic 11",
	"Generic 12",
	"Generic 13",
	"Generic 14",
	"Generic 15",
	"Generic 16",
	"Generic 17",
	"Generic 18",
	"Generic 19",
	"Generic 20",
	"Generic 21",
	"Generic 22",
	"Generic 23",
	"Generic 24",
	"Generic 25",
	"Generic 26",
	"Generic 27",
	"Generic 28",
	"Generic 29",
	"Generic 30",
	"Lounge Window Open",
	"Tried Computer",
	"Seen Aunt Safe",
	"Boiler Die",
	"Kitchen Die",
	"Solved Slider Puzzle",
	"Tried Library Door",
	"Seen Boiler Door",
	"Player Has Boiler Pwd",
	"Connie Said Date",
	"Connie Chickens",
	"Hal Chickens",
	"Hulk Chickens",
	"Solved Boiler Lock",
	"Seen the drug depot robbery article",
	"Seen video camera in Jake's locker",
	"Has soup ladle",
	"HDIC 5 Loop",
	"HDIC 6 Loop",
	"HIC 5 Loop",
	"HIC 6 Loop",
	"HIC 7 Loop",
	"DIC 4 Loop",
	"DIC 5 Loop",
	"DIC 6 Loop",
	"DIC 7 Loop",
	"DIC 8 Loop",
	"DIC 9 Loop",
	"DIC 10 Loop",
	"DIC 11 Loop",
	"DIC 12 Loop",
	"DIC 13 Loop",
	"DIC 15 Loop",
	"DIC 16 Loop",
	"DIC 17 Loop",
	"DIC 18 Loop",
	"CIC 5 Loop",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty",
	"empty"
};

#endif // NANCY1DATA_H
