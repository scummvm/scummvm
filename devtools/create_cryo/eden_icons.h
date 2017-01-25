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

#pragma once
#include "eden.h"

// Note: the following data can be found in the original game's executable

// NB! this enum must match kActionCursors[] array
enum kCursors {		// offset in the executable
	cuNone = 0,	// 0x51F
	cu1 = 1,	// 0x563
	cu2 = 2,	// 0x556
	cu3 = 3,	// 0x549
	cu4 = 4,	// 0x570
	cu5 = 5,	// 0x57D
	cuHand = 6,	// 0x502
	cu7 = 7,	// 0x52C
	cu8 = 8,	// 0x58A
	cu9 = 9,	// 0x539
	cuFa = 0xF,	// 0x50F
	cuFinger = 53,	// 0x541
	ICON_HIDDEN = 0x8000
};

// NB! this enum must match EdenGame::*mouse_actions[] array
enum kActions {			// offset in the executable
	ac_ret = 27,		// 0xD651
	ac_clicplanval = 139,	// 0xE068
	ac_endFrescoes = 140,	// 0xB12A
	ac_choisir = 141,	// 0xDD68
	ac_parle_moi = 246,	// 0xBFE
	ac_adam = 247,		// 0x9E4
	ac_takeobject = 248,	// 0xE66B
	ac_putobject = 249,	// 0xE681
	ac_clictimbre = 250,	// 0xE03F
	ac_dinaparle = 251,	// 0xDF32
	ac_close_perso = 252,	// 0x13EC
	ac_generique = 260,	// 0xAF51
	ac_choixsubtitle = 261,	// 0xACBF
	ac_EdenQuit = 262,	// 0xAF6D
	ac_restart = 263,	// 0xAEE7
	ac_cancel2 = 264,	// 0xACE8
	ac_testvoice = 265,	// 0xACF8
	ac_reglervol = 266,	// 0xAB9E
	ac_load = 267,		// 0xAD76
	ac_save = 268,		// 0xAD40
	ac_cliccurstape = 269,	// 0xB004
	ac_playtape = 270,	// 0x19DB
	ac_stoptape = 271,	// 0xB095
	ac_rewindtape = 272,	// 0xB0C9
	ac_forwardtape = 273,	// 0xB0E3
	ac_confirmyes = 274,	// 0xADAE
	ac_confirmno = 275,	// 0xADC1
	ac_gotocarte = 276	// 0xE07E
};

// Indicies in to gotos[] array for World map areas
enum kTravel {			// offset in the executable
	goMo = 24,		// 0x324D
	goChamaar = 40,		// 0x3287
	goUluru = 51,		// 0x32AF
	goKoto = 65,		// 0x32E3
	goNarim = 70,		// 0x32F5
	goTamara = 75,		// 0x3307
	goCantura = 84,		// 0x3329
	goShandovra = 93,	// 0x334B
	goEmbalmers = 102,	// 0x336D
	goWhiteArch = 111,	// 0x338F
	goMoorkusLair = 120	// 0x33B1
};

const int kNumIcons = 136;
const icon_t gameIcons[kNumIcons] = {
	{90, 50, 220, 150, cu8, ac_parle_moi, 0},
	{0, 0, 319, 178, cuNone, ac_close_perso, 0},
	END_ICONS,
	{220, 16, 310, 176, cu5, ac_adam, 0},
	{0, 0, 320, 200, cu8, ac_parle_moi, 0},
	END_ICONS,
	{215, 140, 245, 176, cuHand, ac_choisir, 0},
	{245, 140, 275, 176, cuHand, ac_choisir, 1},
	{275, 140, 305, 176, cuHand, ac_choisir, 2},
	END_ICONS,
	{245, 140, 275, 176, cuHand, ac_choisir, 0},
	{275, 140, 305, 176, cuHand, ac_choisir, 1},
	END_ICONS,
	{0, 0, 320, 165, cuFa, ac_dinaparle, 0},
	{0, 165, 320, 200, cu2, ac_endFrescoes, 0},
	END_ICONS,
	{0, 176, 319, 200, ICON_HIDDEN|cu9, ac_putobject, 0},
	{120, 0, 200, 16, cuFinger, ac_clictimbre, 0},
	{266, 0, 320, 16, ICON_HIDDEN|cuFinger, ac_clicplanval, 0},
	// Inventory bar items
	// Mac version displays only 9 items, with extra margins
	{0, 178, 28, 200, cuHand, ac_takeobject, 0},	// Not on Mac
	{30, 178, 57, 200, cuHand, ac_takeobject, 0},
	{59, 178, 86, 200, cuHand, ac_takeobject, 0},
	{88, 178, 115, 200, cuHand, ac_takeobject, 0},
	{117, 178, 144, 200, cuHand, ac_takeobject, 0},
	{146, 178, 173, 200, cuHand, ac_takeobject, 0},
	{175, 178, 202, 200, cuHand, ac_takeobject, 0},
	{204, 178, 231, 200, cuHand, ac_takeobject, 0},
	{233, 178, 260, 200, cuHand, ac_takeobject, 0},
	{262, 178, 289, 200, cuHand, ac_takeobject, 0},
	{290, 178, 317, 200, cuHand, ac_takeobject, 0},	// Not on Mac
	// reserve for room's icons
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	END_ICONS,
	// Menu icons
	{0, 0, 319, 15, cuFinger, ac_generique, 32},
	{8, 42, 86, 51, cuFinger, ac_choixsubtitle, 16},
	{8, 51, 86, 60, cuFinger, ac_choixsubtitle, 17},
	{8, 60, 86, 69, cuFinger, ac_choixsubtitle, 18},
	{8, 69, 86, 78, cuFinger, ac_choixsubtitle, 19},
	{8, 78, 86, 87, cuFinger, ac_choixsubtitle, 20},
	{8, 87, 86, 96, cuFinger, ac_choixsubtitle, 21},
	{16, 137, 79, 148, cuFinger, ac_EdenQuit, 34},
	{129, 137, 192, 148, cuFinger, ac_restart, 35},
	{239, 137, 302, 148, cuFinger, ac_cancel2, 36},
	{130, 112, 193, 123, cuFinger, ac_testvoice, 37},
	{114, 40, 121, 110, cuFinger, ac_reglervol, 48},
	{121, 40, 128, 110, cuFinger, ac_reglervol, 56},
	{128, 40, 136, 110, cuFinger, ac_reglervol, 49},
	{147, 40, 154, 110, cuFinger, ac_reglervol, 50},
	{154, 40, 161, 110, cuFinger, ac_reglervol, 58},
	{161, 40, 169, 110, cuFinger, ac_reglervol, 51},
	{179, 40, 186, 110, cuFinger, ac_reglervol, 52},
	{186, 40, 193, 110, cuFinger, ac_reglervol, 60},
	{193, 40, 201, 110, cuFinger, ac_reglervol, 53},
	{249, 42, 307, 51, cuFinger, ac_load, 65},
	{249, 51, 307, 60, cuFinger, ac_load, 66},
	{249, 60, 307, 69, cuFinger, ac_load, 67},
	{231, 69, 307, 78, cuFinger, ac_load, 68},
	{230, 104, 307, 112, cuFinger, ac_save, 81},
	{230, 113, 307, 121, cuFinger, ac_save, 82},
	{230, 122, 307, 130, cuFinger, ac_save, 83},
	{0, 176, 0, 185, cuFinger, ac_cliccurstape, 100},
	{149, 185, 166, 200, cuFinger, ac_playtape, 96},
	{254, 185, 269, 200, cuFinger, ac_stoptape, 97},
	{85, 185, 111, 200, cuFinger, ac_rewindtape, 98},
	{204, 185, 229, 200, cuFinger, ac_forwardtape, 99},
	{0, 0, 320, 200, cuFinger, ac_ret, 0},
	END_ICONS,
	// Yes/No dialog icons
	{129, 84, 157, 98, cuFinger, ac_confirmyes, 0},
	{165, 84, 188, 98, cuFinger, ac_confirmno, 113},
	{0, 0, 320, 200, cuFinger, ac_ret, 0},
	END_ICONS,
	// World map hotspots
	{136, 100, 160, 124, cu5, ac_gotocarte, goMo},
	{150, 55, 174, 79, cu5, ac_gotocarte, goChamaar},
	{186, 29, 210, 53, ICON_HIDDEN|cu5, ac_gotocarte, goUluru},
	{217, 20, 241, 44, ICON_HIDDEN|cu5, ac_gotocarte, goKoto},
	{248, 45, 272, 69, ICON_HIDDEN|cu5, ac_gotocarte, goNarim},
	{233, 68, 257, 92, ICON_HIDDEN|cu5, ac_gotocarte, goTamara},
	{235, 109, 259, 133, ICON_HIDDEN|cu5, ac_gotocarte, goCantura},
	{163, 137, 187, 161, ICON_HIDDEN|cu5, ac_gotocarte, goEmbalmers},
	{93, 145, 117, 169, ICON_HIDDEN|cu5, ac_gotocarte, goWhiteArch},
	{70, 39, 94, 63, ICON_HIDDEN|cu5, ac_gotocarte, goShandovra},
	{99, 8, 123, 32, ICON_HIDDEN|cu5, ac_gotocarte, goMoorkusLair},
	{0, 0, 319, 199, cuNone, ac_close_perso, 0},
	END_ICONS,
};
