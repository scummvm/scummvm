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

#ifndef TVDDATA_H
#define TVDDATA_H

#include "types.h"

const GameConstants _tvdConstants = {
	24,								// numItems
	120,							// numEventFlags
	{ 110, 111, 112, 113, 114 },	// genericEventFlags
	4,								// numCursorTypes
	167000,							// logoEndAfter
	59								// wonGameFlagID
};

const Common::Array<uint16> _tvdMapAccessSceneIDs = {
	0, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 125, 219, 220
};

const SoundChannelInfo _tvdToNancy2SoundChannelInfo = {
	32, 9,
	{ 7, 8, 30 },
	{ 0, 1, 2, 19, 27, 28, 29 },
	{ 3, 4, 5, 6, 17, 18, 20, 21, 22, 23, 24, 25, 26, 31 }
};

const Common::Array<GameLanguage> _tvdLanguagesOrder = {
	GameLanguage::kEnglish
};

const Common::Array<Common::Array<ConditionalDialogue>> _tvdConditionalDialogue = {
{	// Damon, empty
},
{	// Security guard, empty
},
{	// Mrs. Flowers, 20 responses
	{	19, 759, "FIC_01",
		{ { kEv, 0x4, kFalse }, { kEv, 0x49, kTrue }, { kIn, 0x7, kFalse } } },
	{	18, 758, "FIC_02",
		{ { kEv, 0x7, kFalse }, { kEv, 0x4F, kTrue }, { kIn, 0x3, kFalse } } },
	{	17, 757, "FIC_03",
		{ { kEv, 0x10, kFalse }, { kEv, 0x18, kTrue }, { kEv, 0x1E, kFalse }, { kEv, 0x22, kTrue } } },
	{	16, 756, "FIC_04",
		{ { kEv, 0x10, kFalse }, { kEv, 0x18, kTrue }, { kEv, 0x1E, kFalse }, { kEv, 0x22, kFalse } } },
	{	15, 755, "FIC_05",
		{ { kEv, 0xD, kFalse }, { kEv, 0x1C, kTrue }, { kIn, 0x14, kFalse } } },
	{	14, 754, "FIC_06",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x52, kFalse }, { kEv, 0x1, kFalse }, { kIn, 0x10, kFalse } } },
	{	13, 753, "FIC_07",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x52, kFalse }, { kEv, 0x9, kFalse }, { kIn, 0x7, kFalse } } },
	{	12, 753, "FIC_08",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x52, kFalse }, { kEv, 0x8, kFalse }, { kIn, 0x2, kFalse } } },
	{	11, 751, "FIC_09",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x52, kFalse }, { kEv, 0x11, kFalse }, { kIn, 0x3, kFalse } } },
	{	10, 750, "FIC_10",
		{ { kEv, 0x21, kTrue }, { kEv, 0xE, kFalse }, { kIn, 0x9, kTrue } } },
	{	9, 749, "FIC_11",
		{ { kEv, 0x21, kTrue }, { kEv, 0x5D, kFalse }, { kIn, 0xD, kTrue } } },
	{	8, 748, "FIC_12",
		{ { kEv, 0x21, kTrue }, { kEv, 0x5A, kFalse }, { kIn, 0x10, kTrue } } },
	{	7, 747, "FIC_13",
		{ { kEv, 0x21, kTrue }, { kEv, 0x5B, kFalse }, { kIn, 0x7, kTrue } } },
	{	6, 746, "FIC_14",
		{ { kEv, 0x21, kTrue }, { kEv, 0x5C, kFalse }, { kIn, 0x2, kTrue } } },
	{	5, 745, "FIC_15",
		{ { kEv, 0x21, kTrue }, { kEv, 0x5E, kFalse }, { kIn, 0x5, kTrue } } },
	{	4, 744, "FIC_16",
		{ { kEv, 0x21, kTrue }, { kEv, 0x60, kFalse }, { kIn, 0x8, kTrue } } },
	{	3, 743, "FIC_17",
		{ { kEv, 0x21, kTrue }, { kEv, 0x5F, kFalse }, { kIn, 0xB, kTrue } } },
	{	2, 742, "FIC_18",
		{ { kEv, 0x21, kTrue }, { kEv, 0x61, kFalse }, { kIn, 0x14, kTrue } } },
	{	1, 741, "FIC_19",
		{ { kEv, 0xC, kFalse }, { kEv, 0x46, kTrue }, { kEv, 0x1D, kFalse } } },
	{	0, 740, "FIC_20",
		{ { kEv, 0xF, kFalse }, { kEv, 0x1D, kTrue } } }
},
{	// Bonnie, 9 responses + 2 repeats
	{	29, 928, "BIC_01",
		{ { kEv, 0x7, kFalse }, { kEv, 0x4F, kTrue }, { kEv, 0x4A, kFalse } } },
	{	28, 927, "BIC_02",
		{ { kEv, 0x10, kFalse }, { kEv, 0x22, kTrue }, { kEv, 0x1E, kFalse } } },
	{	28, 927, "BIC_02",
		{ { kEv, 0x10, kFalse }, { kEv, 0x22, kFalse }, { kEv, 0x18, kTrue }, { kEv, 0x1E, kFalse } } },
	{	27, 926, "BIC_03",
		{ { kEv, 0xC, kFalse }, { kEv, 0x46, kTrue }, { kIn, 0xB, kFalse } } },
	{	27, 926, "BIC_03",
		{ { kEv, 0xC, kFalse }, { kEv, 0x46, kTrue }, { kIn, 0xB, kTrue }, { kIn, 0x8, kFalse } } },
	{	26, 925, "BIC_04",
		{ { kEv, 0xD, kFalse }, { kEv, 0x1C, kTrue } } },
	{	25, 924, "BIC_05",
		{ { kEv, 0xA, kFalse }, { kEv, 0x47, kTrue }, { kEv, 0x3C, kTrue }, { kIn, 0xB, kFalse } } },
	{	24, 923, "BIC_06",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x1, kFalse }, { kIn, 0x10, kFalse } } },
	{	23, 922, "BIC_07",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x4, kFalse }, { kIn, 0x7, kFalse } } },
	{	22, 921, "BIC_08",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x8, kFalse }, { kIn, 0x2, kFalse } } },
	{	21, 920, "BIC_09",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x11, kFalse }, { kIn, 0x3, kFalse } } },
	{	20, 919, "BIC_10",
		{ { kEv, 0x4A, kTrue }, { kEv, 0xF, kFalse }, { kEv, 0x1D, kTrue } } }
},
{	// Caroline, 4 responses
	{	33, 846, "CIC_01",
		{ { kEv, 0x4, kFalse }, { kEv, 0x49, kTrue }, { kIn, 0x7, kFalse } } },
	{	32, 845, "CIC_02",
		{ { kEv, 0x5, kFalse } } },
	{	31, 844, "CIC_03",
		{ { kEv, 0x6, kFalse }, { kEv, 0x56, kTrue }, { kIn, 0x16, kFalse } } },
	{	30, 843, "CIC_04",
		{ { kEv, 0x10, kFalse }, { kEv, 0x18, kTrue }, { kIn, 0x14, kFalse } } }
},
{	// Stefan, 10 responses
	{	43, 342, "SIC_10",
		{ { kEv, 0x52, kTrue } } },
	{	42, 351, "SIC_01",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4, kFalse }, { kEv, 0x49, kTrue }, { kIn, 0x7, kFalse } } },
	{	41, 350, "SIC_02",
		{ { kEv, 0x52, kFalse }, { kEv, 0x10, kFalse }, { kEv, 0x18, kTrue }, { kIn, 0x14, kFalse } } },
	{	40, 349, "SIC_03",
		{ { kEv, 0x52, kFalse }, { kEv, 0xD, kFalse }, { kEv, 0x1C, kTrue }, { kIn, 0xB, kFalse } } },
	{	39, 348, "SIC_04",
		{ { kEv, 0x52, kFalse }, { kEv, 0xA, kFalse }, { kEv, 0x47, kTrue }, { kEv, 0x3C, kTrue }, { kIn, 0xB, kFalse } } },
	{	38, 347, "SIC_05",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x1, kFalse }, { kIn, 0x10, kFalse } } },
	{	37, 346, "SIC_06",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x8, kFalse }, { kIn, 0x2, kFalse } } },
	{	36, 345, "SIC_07",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x11, kFalse }, { kIn, 0x3, kFalse } } },
	{	35, 344, "SIC_08",
		{ { kEv, 0x52, kFalse }, { kEv, 0xC, kFalse }, { kEv, 0x46, kTrue }, { kEv, 0x1D, kFalse } } },
	{	34, 343, "SIC_09",
		{ { kEv, 0x52, kFalse }, { kEv, 0xF, kFalse }, { kEv, 0x1D, kTrue }, { kIn, 0x12, kFalse } } }
},
{	// Mrs. Grimesby, 20 responses + 2 repeats
	{	63, 714, "EGS_02A",
		{ { kEv, 0x17, kTrue }, { kEv, 0x28, kFalse } } },
	{	63, 714, "EGS_02A",
		{ { kEv, 0x17, kFalse }, { kEv, 0x4F, kTrue }, { kEv, 0x28, kFalse } } },
	{	62, 713, "EGS_02B",
		{ { kEv, 0x17, kTrue }, { kEv, 0x28, kTrue } } },
	{	62, 713, "EGS_02B",
		{ { kEv, 0x17, kFalse }, { kEv, 0x4F, kTrue }, { kEv, 0x28, kTrue } } },
	{	61, 712, "EGS_02C",
		{ { kEv, 0x50, kTrue }, { kEv, 0x1D, kFalse }, { kEv, 0x2C, kFalse } } },
	{	60, 711, "EGS_02D",
		{ { kEv, 0x50, kTrue }, { kEv, 0x1D, kFalse }, { kEv, 0x2C, kTrue } } },
	{	59, 719, "EGS_02E",
		{ { kEv, 0x47, kTrue }, { kEv, 0x13, kTrue }, { kEv, 0x27, kFalse } } },
	{	58, 709, "EGS_02F",
		{ { kEv, 0x47, kTrue }, { kEv, 0x13, kTrue }, { kEv, 0x27, kTrue } } },
	{	57, 708, "EGS_02G",
		{ { kEv, 0x1E, kFalse }, { kEv, 0x18, kTrue }, { kEv, 0x2D, kFalse } } },
	{	56, 707, "EGS_02H",
		{ { kEv, 0x1E, kFalse }, { kEv, 0x18, kTrue }, { kEv, 0x2D, kTrue } } },
	{	55, 706, "EGS_02I",
		{ { kEv, 0x1E, kFalse }, { kEv, 0x1C, kTrue }, { kEv, 0x2A, kFalse }, { kIn, 0x14, kFalse } } },
	{	54, 705, "EGS_02J",
		{ { kEv, 0x1E, kFalse }, { kEv, 0x1C, kTrue }, { kEv, 0x2A, kTrue }, { kIn, 0x14, kFalse } } },
	{	53, 704, "EGS_02K",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x25, kFalse }, { kIn, 0x10, kFalse } } },
	{	52, 703, "EGS_02L",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x25, kTrue }, { kIn, 0x10, kFalse } } },
	{	51, 702, "EGS_02M",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x29, kFalse }, { kIn, 0x2, kFalse } } },
	{	50, 701, "EGS_02N",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x29, kTrue }, { kIn, 0x2, kFalse } } },
	{	49, 700, "EGS_02O",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x2E, kFalse }, { kIn, 0x3, kFalse } } },
	{	48, 699, "EGS_02P",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x2E, kTrue }, { kIn, 0x3, kFalse } } },
	{	47, 698, "EGS_02Q",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x26, kFalse }, { kIn, 0x7, kFalse } } },
	{	46, 697, "EGS_02R",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x26, kTrue }, { kIn, 0x7, kFalse } } },
	{	45, 696, "EGS_02S",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x1D, kTrue }, { kEv, 0x2B, kFalse } } },
	{	44, 695, "EGS_02T",
		{ { kEv, 0x52, kFalse }, { kEv, 0x4A, kTrue }, { kEv, 0x1D, kTrue }, { kEv, 0x2B, kTrue } } }
},
{	// Aunt Judith, 11 responses
	{	74, 493, "JIC_01",
		{ { kEv, 0x4, kFalse }, { kEv, 0x49, kTrue }, { kIn, 0x7, kFalse } } },
	{	73, 492, "JIC_02",
		{ { kEv, 0x10, kFalse }, { kEv, 0x18, kTrue }, { kIn, 0xD, kFalse } } },
	{	72, 491, "JIC_03",
		{ { kEv, 0xD, kFalse }, { kEv, 0x1C, kTrue }, { kIn, 0xD, kFalse } } },
	{	71, 490, "JIC_04",
		{ { kEv, 0xA, kFalse }, { kEv, 0x47, kTrue }, { kIn, 0xD, kFalse } } },
	{	70, 489, "JIC_05",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x1, kFalse }, { kIn, 0x10, kFalse } } },
	{	69, 488, "JIC_06",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x9, kFalse }, { kIn, 0x7, kFalse } } },
	{	68, 487, "JIC_07",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x8, kFalse }, { kIn, 0x2, kFalse } } },
	{	67, 486, "JIC_08",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x11, kFalse }, { kIn, 0x3, kFalse } } },
	{	66, 485, "JIC_09",
		{ { kEv, 0xC, kFalse }, { kEv, 0x46, kTrue }, { kEv, 0x1D, kFalse } } },
	{	65, 484, "JIC_10",
		{ { kEv, 0xF, kFalse }, { kEv, 0x1D, kTrue }, { kIn, 0x12, kFalse } } },
	{	64, 483, "JIC_11",
		{ { kEv, 0xB, kFalse }, { kEv, 0x48, kTrue }, { kIn, 0x5, kFalse } } }
},
{	// Mr. Richards, empty
},
{	// Mikhail, 6 responses
	{	80, 452, "MIC_01",
		{ { kEv, 0x10, kFalse }, { kIn, 0xD, kTrue }, { kIn, 0x14, kFalse } } },
	{	79, 451, "MIC_02",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x1, kFalse }, { kIn, 0x10, kFalse } } },
	{	78, 450, "MIC_03",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x4, kFalse }, { kIn, 0x7, kFalse } } },
	{	77, 449, "MIC_04",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x8, kFalse }, { kIn, 0x2, kFalse } } },
	{	76, 448, "MIC_05",
		{ { kEv, 0x4A, kTrue }, { kEv, 0x11, kFalse }, { kIn, 0x3, kFalse } } },
	{	75, 448, "MIC_06",
		{ { kEv, 0xF, kFalse }, { kEv, 0x1D, kTrue }, { kIn, 0x12, kFalse } } }
}
};

static Common::Array<Goodbye> _tvdGoodbyes = {
	// Damon
	{ "DAMBYE", {	{ { 809 }, {}, NOFLAG } } },
	// Security guard
	{ "EGBYE", {	 { { 1108 }, {}, NOFLAG } } },
	// Mrs. Flowers
	{ "FLOWBYE", {  { { 1112, 1113, 1114 }, { { kEv, 0x3E, kFalse } }, NOFLAG },
					{ { 1109, 1110, 1111 }, {}, NOFLAG } } },
	// Bonnie
	{ "BONBYE", {	{ { 1103, 1104, 1105 }, { { kEv, 0x3E, kTrue } }, NOFLAG },
					{ { 997 }, { { kEv, 0x3E, kFalse }, { kEv, 0x15, kTrue }, { kEv, 0x1B, kTrue }, { kEv, 0x20, kTrue }, { kEv, 0x3F, kTrue } }, { kEv, 0x41, kTrue } },
					{ { 1100, 1101, 1102 }, {}, NOFLAG } } },
	// Caroline; S1142.IFF appears to be missing
	{ "CAROLBYE", { { { 1140, 1141/*, 1142,*/ }, { { kEv, 0x3E, kFalse }, { kEv, 0x42, kTrue } }, NOFLAG },
					{ { 1115, 1116, 1117 }, { { kEv, 0x3E, kFalse }, { kEv, 0x42, kFalse } }, NOFLAG },
					{ { 1119 }, {}, NOFLAG } } },
	// Stefan
	{ "STEFBYE", {  { { 1121, 1122, 1123 }, { { kEv, 0x23, kTrue }, { kEv, 0x42, kTrue } }, NOFLAG },
					{ { 1150 }, {}, NOFLAG } } },
	// Mrs. Grimesby
	{ "GRMBYE", {	{ { 1124 }, {}, NOFLAG } } },
	// Aunt Judith
	{ "JUDYBYE", {  { { 1125 }, { { kEv, 0x3E, kFalse }, { kEv, 0x42, kFalse } }, NOFLAG },
					{ { 1126 }, { { kEv, 0x3E, kFalse }, { kEv, 0x42, kTrue } }, NOFLAG },
					{ { 1127 }, {}, NOFLAG } } },
	// Mr. Richards
	{ "RICHBYE", {  { { 1128 }, { { kEv, 0x59, kTrue } }, NOFLAG },
					{ { 1129 }, {}, NOFLAG } } },
	// Mikhail
	{ "EGBYE", {	 { { 1130 }, {}, NOFLAG } } }
};

const Common::Array<Common::Array<const char *>> _tvdConditionalDialogueTexts = { {
	// 00
	"<c1>D<c0>id you find a ring in the cemetery the other night? It was made of silver and Lapis.<h><n>",
	"<c1>I<c0>'m looking for silver and Lapis for a project I'm doing. Do you know where I could find some?<h><n>",
	"<c1>W<c0>ill you trade the vervain for this rune?<h><n>",
	"<c1>W<c0>ill you trade the vervain for some Lapis?<h><n>",
	"<c1>W<c0>ill you trade the vervain for my class ring?<h><n>",
	// 05
	"<c1>W<c0>ill you trade the vervain for this key?<h><n>",
	"<c1>W<c0>ill you trade the vervain for an owl feather?<h><n>",
	"<c1>W<c0>ill you trade the vervain for the dagger?<h><n>",
	"<c1>W<c0>ill you trade the vervain for this black candle?<h><n>",
	"<c1>I<c0>'ve got some gold to trade for the vervain, Mrs. Flowers. Would you take that?<h><n>",
	// 10
	"<c1>I<c0>'ve got something to trade for the vervain, Mrs. Flowers.It's a beautiful antique mortar and pestle I found in my attic. It belonged to my mother.<h><n>",
	"<c1>Y<c0>ou wouldn't happen to know what vervain is, would you, Mrs. Flowers?<h><n>",
	"<c1>I<c0>'m looking for an owls feather. Do you have any idea where I could find one?<h><n>",
	"<c1>T<c0>his sounds strange, I know, but I'm looking for a dagger.<h><n>",
	"<c1>Y<c0>ou wouldn't happen to have any black candles, would you, Mrs. Flowers?<h><n>",
	// 15
	"<c1>M<c0>rs. Flowers, how can someone my age get a loan? There's something at the gallery I have to have.<h><n>",
	"<c1>H<c0>ave you ever heard of runes, Mrs. Flowers? Bonnie swears they're for real, but I'm not so sure I should believe her.<h><n>",
	"<c1>W<c0>hat was that you told me about runes, Mrs. Flowers? I'm sorry, I forgot.<h><n>",
	"<c1>Y<c0>ou seem to know a lot of strange stuff, Mrs. Flowers. Do you know anything about Druids, by any chance?<h><n>",
	"<c1>H<c0>ave you seen the newspaper article on Mr. Richards' collection? Now he's got some weird dagger. Pretty creepy, huh?<h><n>",
	// 20
	"<c1>I<c0> went to the graveyard, at night, just like your grandmother said in her diary. And I did the Ring Ceremony. But I don't have the Ring and I can't find the silver or Lapis either. What do I do?<h><n>",
	"<c1>I<c0>n the spell book, it said I need vervain to power the binding spell. Do you have any vervain, Bonnie? What about Mr. Richards?<h><n>",
	"<c1>A<c0>ccording to the spell book, I need an owl feather -- of all things -- in the binding spell. If anybody has seen an owl around, I figured it would be you.<h><n>",
	"<c1>T<c0>he spell book said I need a dagger to complete the binding spell. There's only one dagger I can think of. Can you help me?<h><n>",
	"<c1>B<c0>onnie, the spell book said I need a black candle for the binding spell. But I don't know where to find one, do you?<h><n>",
	// 25
	"<c1>I<c0> read that ghost story about Adelaide Chambers, you remember it.  I think there really is gold buried in her grave. But I can't get past the ghost that's guarding it.  Do you have any ideas?<h><n>",
	"<c1>I<c0> think I found a rune to replace the one you're missing, Bonnie. It's in Mikhail's art gallery. But Caroline said it will cost loads of money? Do you have any idea what I can do?<h><n>",
	"<c1>I<c0> read your grandmother's diary, Bonnie. She says you need silver and Lapis to make a Ring of Power. Do you have any idea where I could find some?<h><n>",
	"<c1>I<c0> know you know something about Runes, Bonnie. What can you tell me about them?<h><n>",
	"<c1>H<c0>ave you ever heard of Druids, Bonnie? I heard they believed in magic, so I thought you might know something about them.<h><n>",
	// 30
	"<c1>T<c0>hat's a rune over there, isn't it? Bonnie would sure like that for her birthday. How much is it?<h><n>",
	"<c1>D<c0>o you know anything about the display Mr. Smith is doing for the high school?<h><n>",
	"<c1>M<c0>r. Smith gave you a ride home from the hospital, right? What do you know about him?<h><n>",
	"<c1>H<c0>ave you heard anything about that new dagger Gary Richards has added to his collection? Has Mikhail said mentioned it?<h><n>",
	"<c1>W<c0>hat am I going to do, Stefan? I did the ceremony to summon the Ring Maker, but nothing happened. And now I don't have the silver, or the Lapis, or the Ring of Power!<h><n>",
	// 35
	"<c1>A<c0>ccording to the diary of Bonnie's grandmother, I need silver and Lapis for a Ring of Power. But I don't have any silver or Lapis. And I need to summon the ring maker once I have the ingredients..<h><n>",
	"<c1>T<c0>here's one more thing that I need for the binding spell, Stefan. Some vervain. But I have no idea where to find any. Do you have any ideas?<h><n>",
	"<c1>O<c0>kay, I've got the binding spell, but I need an owl feather. You wouldn't happen to be an owl in your other form, would you?<h><n>",
	"<c1>N<c0>ow that I've found the binding spell, I need a black candle. But why does it have to be a black candle, and not a red or white candle?<h><n>",
	"<c1>D<c0>o you have any idea how I could get rid of Adelaide Chambers' ghost, Stefan?  I need to find out if there's gold buried there.  But every time I try to get near, she drives me off.<h><n>",
	// 40
	"<c1>I<c0> think I found a rune to replace the one that Bonnie is missing. The only snag is that it's for sale in Mikhail's art gallery. Which means that a need a whole lot of money to buy it. You wouldn'thappen to have any money you could lend me, do you?<h><n>",
	"<c1>B<c0>onnie told me that she's missing one of her grandmother's runes, Stefan. You wouldn't know where I could find it, do you?<h><n>",
	"<c1>I<c0> read about the Brasov Dagger in the paper, Stefan. They said it had something to do with immortality. But that doesn't sound right. What can you tell me about it?<h><n>",
	"<c1>I<c0>'ve got everything. Stefan. And -- it's the weirdest thing -- Bonnie and Mrs. Flowers just came to tell me it's time, that he's drawing in his power. Will you come with me, please?<h><n>",
	"<c1>H<c0>as anyone mentioned finding a silver and lapis ring yet, Mrs. Grimesby?<h><n>",
	// 45
	"<c1>I<c0>'m looking for a ring, made of silver and lapis. Have you seen it?<h><n>",
	"<c1>I<c0>'m still trying to find out about that weird dagger. Can you tell me anything more?<h><n>",
	"<c1>H<c0>ave you ever heard of a magical dagger that has something to do with ULTIMATE POWER?<h><n>",
	"<c1>W<c0>hat else can you tell me about vervain?<h><n>",
	"<c1>W<c0>hat's vervain?<h><n>",
	// 50
	"<c1>I<c0>'m still looking for an owl feather. What can I do?<h><n>",
	"<c1>D<c0>o you have any idea where I can find an owl feather?<h><n>",
	"<c1>I<c0>'m still looking for a black candle, Mrs. Grimesby. Can you help me?<h><n>",
	"<c1>I<c0>'m looking for a black candle. Do you know where I can find one?<h><n>",
	"<c1>I<c0>'m still trying to find enough money to buy something from the art gallery. Do you have any idea what I can do?<h><n>",
	// 55
	"<c1>M<c0>rs. Grimesby, do you have any idea where I can get enough money to buy something from the art gallery?<h><n>",
	"<c1>W<c0>here was that information about Runes again, please?<h><n>",
	"<c1>W<c0>here can I find something about Runes? You know, those Viking fortune-telling things.<h><n>",
	"<c1>W<c0>here did you say I could find something from that Civil War ghost story, Mrs. Grimesby?<h><n>",
	"<c1>I<c0> read a ghost story about the daughter of a Union general who was in love with a Confederate soldier. Do you know if it's true?<h><n>",
	// 60
	"<c1>W<c0>hat was it you said about a Ring of Power?<h><n>",
	"<c1>H<c0>ave you ever heard of something called a Ring of Power?<h><n>",
	"<c1>C<c0>an you tell about Druids again, please?<h><n>",
	"<c1>W<c0>hat can you tell me about Druids?<h><n>",
	"<c1>A<c0>unt Judith, do you know what Margaret did with the key to my jewelry box?<h><n>",
	// 65
	"<c1>I<c0> was looking for a silver ring. Have you seen one, Aunt Judith?<h><n>",
	"<c1>D<c0>o you have any idea where I could find any silver or Lapis jewelry, Aunt Judith?<h><n>",
	"<c1>A<c0>unt Judith, do you know where I could find some vervain?<h><n>",
	"<c1>W<c0>ould you know how I could get an owl feather?<h><n>",
	"<c1>H<c0>ow could I get Mr. Richards to lend me his new dagger?<h><n>",
	// 70
	"<c1>D<c0>o you know where I could find a black candle, Aunt Judith?<h><n>",
	"<c1>D<c0>o you remember the old ghost story about Adelaide Chambers, Aunt Judith? A book in the library says she was buried with gold. Do you think it's true?<h><n>",
	"<c1>I<c0> want to buy something from the art gallery, Aunt Judith. Can you help me?<h><n>",
	"<c1>D<c0>o you know anything about runes, Aunt Judith? Bonnie said one of her's is missing and I'd like to get her another one.<h><n>",
	"<c1>D<c0>o you know anything about that dagger Mr. Richards just added to his collection, Aunt Judith?<h><n>",
	// 75
	"<c1>I<c0> lost a ring, it was made of silver and Lapis. You haven't seen it, have you?<h><n>",
	"<c1>I<c0>'m trying to find some vervain. Do you know where I could find some?<h><n>",
	"<c1>H<c0>ave you by any chance seen any owl feathers anywhere around?<h><n>",
	"<c1>I<c0>'m looking for a special dagger, it's supposed to have something to do with Ultimate Power or something. Can you help me?<h><n>",
	"<c1>C<c0>ould you tell me where I could find a black candle, please?<h><n>",
	// 80
	"<c1>I<c0>'m interested in buying a rune you have for sale.<h><n>"
} };

const Common::Array<Common::Array<const char *>> _tvdGoodbyeTexts = { {
	"<c1>I<c0> think I'd better go.<h>", // DAMBYE
	"<c1>b<c0>ye.<h>", // EGBYE
	"<c1>I<c0> should go now, Mrs. Flowers. I'll see you later, okay?.<h>", // FLOWBYE
	"<c1>S<c0>ee you later, Bonnie..<h>", // BONBYE
	"<c1>F<c0>ine. I am out of here..<h>", // CAROLBYE
	"<c1>I<c0>'ll be back soon, Stefan. I promise..<h>", // STEFBYE
	"<c1>b<c0>ye.<h>", // GRMBYE
	"<c1>S<c0>ee you later, Aunt Judith..<h>", // JUDYBYE
	"<c1>I<c0> guess it's time for me to go..<h>", // RICHBYE
	"<c1>b<c0>ye.<h>", // EGBYE, again
} };

const Common::Array<const char *> _tvdEmptySaveStrings = {
	"-- Empty --",	// English
};

const Common::Array<const char *> _tvdEventFlagNames = {
	"Aristocrat Bonnie told about mikhails bgnd",
	"AskedCandle asked about a black candle",
	"AskedCaroline  asked Mikhail about caroline (in EG)",
	"AskedChildren ask Mik about children EG",
	"AskedDagger asked about the dagger",
	"AskedDamon asked about Damon",
	"AskedDisplay ask about display at school",
	"AskedDruids asked about druids",
	"AskedFeather asked about owl feathers",
	"AskedGetDagger ask about getting dagger",
	"AskedGold ask how to get gold from ghost",
	"AskedKey ask about key to her jewelry box",
	"AskedLapis asked about Lapis",
	"AskedMoney asked to borrow money",
	"AskedMortar asked to trade the Mortar",
	"AskedRing asked if whoever has seen the ring",
	"AskedRunes asked about runes",
	"AskedVervain asked about Vervain",
	"AskedWhy asked Mikhail why he is doing this",
	"BeenToGrave ",
	"BeenToHospitalOnce ",
	"BonieIntro met Bonnie",
	"BonnieMad  Bonnie mad during convo at party",
	"BonnieSaidDruid Bonnie mentioned her druid relatives",
	"BonnieToldRunes Bonnie said  missing rune",
	"BonnieToldJob Bonnie told about her job",
	"CarolineAngry made Caroline angry",
	"CarolineIntro met Caroline",
	"CarolineSaidMoney Caroline told rune costs",
	"DoneRingCeremony ",
	"DoneRuneReading",
	"DumbGuySaveGame Not really used",
	"FlowersIntro met Mrs Flowers",
	"FlowersSaidTrade Flowers told about trade",
	"FlowersToldRunes Flowers talked about runes",
	"GoneToLounge",
	"GrimesbyTalked talked to Grimesby",
	"GToldCandle Grimesby has mentioned candles",
	"GToldDagger Grimesby has mentioned dagger",
	"GToldDisplay Grimesby mentioned display",
	"GToldDruid",
	"GToldFeather Grimesby talked about feathers",
	"GToldGold Grimesby talked about gold",
	"GToldLapisRing mentioned ring to Grimesby",
	"GToldRingOfPower asked Grimesby ring of power",
	"GToldRunes Grimesby told where to find runes info",
	"GToldVervain Grimesby player about vervain ",
	"GuardCaught",
	"GuardMad player makes the guard mad",
	"GuardToldKey guard tells the player about the key",
	"School door chain is broken",
	"IntoThePit",
	"Elena Gone To Bed",
	"Damon At Desk",
	"Caroline Hospital Done",
	"Mikhail Working",
	"End Game Puzzle",
	"Judith Talked Once",
	"Stefan Talked Once",
	"Player Won Game",
	"Seen Ghost Attack",
	"Ghost Kiss Done",
	"IntroDone",
	"JudithIntro met Aunt Judy",
	"JudithToldCandle Judy told where the candle is",
	"LastToTalk ",
	"MargaretAttacked ",
	"MikhailMet player has met Mikhail",
	"MikhailTrapped ",
	"Dream Done",
	"ReadDiary ",
	"ReadGhostStory ",
	"ReadMargaretNote ",
	"ReadPaper ",
	"ReadSpellBook ",
	"RichardsIntro  met Gary Richards",
	"RingCeremonyStarted ",
	"StefanIntro player has met Stefan",
	"StefanSaidBrother Stefan has mentioned his brother",
	"StefanSaidDruid Stefan mentioned druids",
	"StefanSaidRingOfPower Stefan mentioned ring of power",
	"TalkedToMikhail  talked to Mikhail",
	"TimeForEndgame",
	"ToldAboutMary player mentioned Bonnies sister to Gary",
	"ToldBite ",
	"ToldDangerous ",
	"ToldDisplay player been told about school display",
	"ToldInnocent player been told about innocent blood?",
	"ToldVampire player been told Stefan is a vampire",
	"ToldVampireDagger player been told relation between dagger and vampires?",
	"TradeCandle player has offered the candle to Flowers",
	"TradeDagger player offered dagger to Flowers",
	"TradeFeather player offered the feather to flowers",
	"TradeGold player has offered gold to flowers",
	"TradeKey player has offered the key to flowers",
	"TradeLapis player has offered the lapis to flowers",
	"TradeClassRing offered class ring to flowers",
	"TradeRune player offered the rune to flowers",
	"TriedToOpenDoor tried door at Gary's house during day",
	"TriggerScream ",
	"WhereIsMikhail ",
	"Stop player scrolling ",
	"GaryRichardsMezzDragonSlider ",
	"GaryRichardsSpiralStaircase ",
	"SirenSoundEffect",
	"Stormy",
	"Dagger Puzzle",
	"Owl Puzzle",
	"Magic Book Movie",
	"Mask Used On Door",
	"Generic 0, single scene only - clear, set, clear",
	"Generic 1, single scene only - clear, set, clear",
	"Generic 2, single scene only - clear, set, clear",
	"Generic 3, single scene only - clear, set, clear",
	"Generic 4, single scene only - clear, set, clear",
	" ",
	" ",
	" ",
	" ",
	" "
};

#endif // TVDDATA_H
