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

#include "bladerunner/script/init.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

void ScriptInit::SCRIPT_Initialize_Game() {
	Assign_Player_Gun_Hit_Sounds(0, 517, 518, 519);
	Assign_Player_Gun_Hit_Sounds(1, 520, 521, 522);
	Assign_Player_Gun_Hit_Sounds(2, 523, 524, 525);

	Assign_Player_Gun_Miss_Sounds(0, 526, 527, 528);
	Assign_Player_Gun_Miss_Sounds(1, 529, 530, 531);
	Assign_Player_Gun_Miss_Sounds(2, 532, 533, 534);

	Init_Globals();
	Init_Game_Flags();
	Init_Clues();
	Init_Clues2();
	Init_World_Waypoints();
	Init_SDB();
	Init_CDB();
	Init_Spinner();
	Init_Actor_Friendliness();
	Init_Actor_Combat_Aggressiveness();
	Init_Actor_Honesty();
	Init_Actor_Intelligence();
	Init_Actor_Stability();
	Init_Actor_Health();
	Init_Combat_Cover_Waypoints();
	Init_Combat_Flee_Waypoints();
	Init_Shadows();
}

void ScriptInit::Init_Globals() {
	for (int i = 0; i != 55; ++i)
		Global_Variable_Set(i, 0);

	Global_Variable_Set(35, 2);
	Global_Variable_Set(1, 1);
	Global_Variable_Set(2, 100);

	Set_Score(0, 0);
	Set_Score(1, 64);
	Set_Score(2, 10);
	Set_Score(3, 47);
	Set_Score(4, 35);
	Set_Score(5, 23);
	Set_Score(6, 28);
}

void ScriptInit::Init_Game_Flags() {
	for (int i = 0; i != 730; ++i)
		Game_Flag_Reset(i);

	if (Random_Query(1, 2) == 1)
		Game_Flag_Set(44);
	if (Random_Query(1, 2) == 1)
		Game_Flag_Set(45);
	if (Random_Query(1, 2) == 1)
		Game_Flag_Set(46);
	if (Random_Query(1, 2) == 1)
		Game_Flag_Set(47);
	if (Random_Query(1, 2) == 1)
		Game_Flag_Set(48);
	if (Random_Query(1, 2) == 1)
		Game_Flag_Set(560);

	if (!Game_Flag_Query(45) && !Game_Flag_Query(46) && !Game_Flag_Query(47))
		Game_Flag_Set(47);

	if (Game_Flag_Query(47)) {
		Global_Variable_Set(40, 1);
	} else if (!Game_Flag_Query(45) || Game_Flag_Query(46)) {
		if (Game_Flag_Query(45) || !Game_Flag_Query(46)) {
			if (Random_Query(1, 2) == 1)
				Global_Variable_Set(40, 2);
			else
				Global_Variable_Set(40, 3);
		} else {
			Global_Variable_Set(40, 3);
		}
	} else {
		Global_Variable_Set(40, 2);
	}

	Game_Flag_Set(182);
	Game_Flag_Set(249);
}

void ScriptInit::Init_Clues() {
	for(int i = 0; i != 288; ++i)
		Actor_Clue_Add_To_Database(0, i, 0, 0, 0, -1);
}

struct clue_weigth {
	int clue;
	int weight;
};

static clue_weigth clues_actor_1[44] = {
	{ 222, 100 }, { 227, 100 }, { 223, 100 }, { 224, 100 }, { 226, 100 }, { 228, 100 }, { 231, 100 }, { 162, 100 },
	{ 164, 100 }, { 166, 100 }, { 168, 100 }, { 170, 100 }, { 172, 100 }, { 174, 100 }, { 176, 100 }, { 239,  90 },
	{ 241,  90 }, { 242,  90 }, { 179,  90 }, { 180,  90 }, { 181,  90 }, {   8,  85 }, { 240,  85 }, { 216,  85 },
	{ 217,  85 }, { 178,  80 }, {   5,  65 }, {   9,  65 }, { 215,  65 }, { 218,  65 }, { 219,  65 }, { 220,  65 },
	{ 229,  65 }, { 211,  65 }, {  80,  65 }, { 108,  65 }, { 134,  65 }, { 135,  65 }, { 212,  55 }, { 221,  55 },
	{ 230,  55 }, {   6,  30 }, {   7,  30 }, {  65,  30 }
};

static clue_weigth clues_actor_2[28] = {
	{ 227,  70 }, { 240,  65 }, { 241,  70 }, { 242,  95 }, { 212,  70 }, { 213,  70 }, { 214,  70 }, { 215,  70 },
	{ 216,  95 }, { 217,  70 }, { 218,  70 }, { 219,  70 }, { 220,  70 }, { 221,  65 }, { 222,  70 }, { 223,  70 },
	{ 224,  70 }, { 226,  70 }, { 228,  70 }, { 229,  70 }, { 230,  70 }, { 231,  70 }, { 232,  70 }, { 116,  65 },
	{ 117,  65 }, { 145,  70 }, { 207,  55 }, { 211,  65 }
};

static clue_weigth clues_actor_3[46] = {
	{ 227,  70 }, { 240,  45 }, { 241,  70 }, { 242,  65 }, { 212,  70 }, { 213,  70 }, { 214,  70 }, { 215,  70 },
	{ 216,  65 }, { 217,  70 }, { 220,  70 }, { 219,  70 }, { 218,  70 }, { 221,  45 }, { 222,  70 }, { 223,  70 },
	{ 224,  70 }, { 225,  70 }, { 226,  70 }, { 228,  70 }, { 229,  70 }, { 230,  70 }, { 231,  70 }, {  95,  70 },
	{ 232,  70 }, { 239,  65 }, {  19,  65 }, {  25,  55 }, {  60,  60 }, {  69,  60 }, {  70,  60 }, {  92,  70 },
	{ 103,  65 }, { 121,  65 }, { 130,  70 }, { 147,  70 }, { 148,  65 }, { 149,  65 }, { 150,  65 }, { 151,  65 },
	{ 152,  65 }, { 116,  65 }, { 117,  65 }, { 145,  70 }, { 207,  55 }, { 211,  65 }
};

static clue_weigth clues_actor_4[23] = {
	{ 241,  90 }, { 242,  90 }, { 240,  70 }, { 214,  75 }, { 216,  75 }, { 218,  75 }, { 219,  75 }, { 220,  75 },
	{ 215,  70 }, { 217,  70 }, { 222,  70 }, { 223,  70 }, { 224,  70 }, { 226,  70 }, { 228,  70 }, { 230,  70 },
	{  73,  65 }, { 211,  65 }, {  80,  65 }, { 108,  65 }, { 134,  65 }, { 135,  65 }, { 212,  55 }
};

static clue_weigth clues_actor_5[46] = {
	{ 227,  70 }, { 241,  70 }, { 212,  70 }, { 213,  70 }, { 214,  70 }, { 215,  70 }, { 217,  70 }, { 220,  70 },
	{ 219,  70 }, { 218,  70 }, { 222,  70 }, { 223,  70 }, { 224,  70 }, { 226,  70 }, { 228,  70 }, { 229,  70 },
	{ 230,  70 }, { 232,  70 }, { 130,  70 }, { 147,  70 }, { 145,  70 }, { 242,  65 }, { 216,  65 }, { 239,  65 },
	{  19,  65 }, {  95,  65 }, { 103,  65 }, { 107,  65 }, { 121,  65 }, { 148,  65 }, { 149,  65 }, { 150,  65 },
	{ 151,  65 }, { 152,  65 }, { 116,  65 }, { 117,  65 }, { 211,  65 }, {  60,  60 }, {  69,  60 }, {  70,  60 },
	{  92,  60 }, {  25,  55 }, { 133,  55 }, { 207,  55 }, { 240,  45 }, { 221,  45 }
};

static clue_weigth clues_actor_6[47] = {
	{ 227,  70 }, { 240,  45 }, { 241,  70 }, { 242,  65 }, { 212,  70 }, { 213,  70 }, { 214,  70 }, { 215,  70 },
	{ 216,  65 }, { 217,  70 }, { 220,  70 }, { 219,  70 }, { 218,  70 }, { 221,  45 }, { 222,  70 }, { 223,  70 },
	{ 224,  70 }, { 226,  70 }, { 228,  70 }, { 229,  70 }, { 230,  70 }, { 231,  70 }, { 232,  70 }, { 239,  65 },
	{  19,  65 }, {  25,  55 }, {  60,  60 }, {  69,  60 }, {  70,  60 }, {  92,  60 }, {  95,  65 }, { 103,  65 },
	{ 107,  65 }, { 121,  55 }, { 130,  70 }, { 133,  70 }, { 147,  70 }, { 148,  65 }, { 149,  65 }, { 150,  65 },
	{ 151,  65 }, { 152,  65 }, { 116,  65 }, { 117,  65 }, { 145,  70 }, { 207,  55 }, { 211,  65 }
};

static clue_weigth clues_actor_7_and_8[47] = {
	{ 227,  70 }, { 240,  45 }, { 241,  70 }, { 242,  65 }, { 212,  70 }, { 213,  70 }, { 214,  70 }, { 215,  70 },
	{ 216,  65 }, { 217,  70 }, { 220,  70 }, { 219,  70 }, { 218,  70 }, { 221,  45 }, { 222,  70 }, { 223,  70 },
	{ 224,  70 }, { 226,  70 }, { 228,  70 }, { 229,  70 }, { 230,  70 }, { 231,  70 }, { 232,  70 }, { 239,  65 },
	{  19,  45 }, {  25,  45 }, {  60,  45 }, {  69,  45 }, {  70,  45 }, {  92,  45 }, {  95,  45 }, { 103,  45 },
	{ 107,  45 }, { 121,  45 }, { 130,  45 }, { 133,  45 }, { 147,  70 }, { 148,  70 }, { 149,  70 }, { 150,  70 },
	{ 151,  70 }, { 152,  70 }, { 116,  65 }, { 117,  65 }, { 145,  70 }, { 207,  55 }, { 211,  65 }
};

static clue_weigth clues_actor_9[49] = {
	{ 241,  70 }, { 212,  70 }, { 214,  70 }, { 217,  70 }, { 220,  70 }, { 219,  70 }, { 218,  70 }, { 222,  70 },
	{ 223,  70 }, { 224,  70 }, { 226,  70 }, { 228,  70 }, { 229,  70 }, { 230,  70 }, { 231,  70 }, { 130,  70 },
	{ 133,  70 }, { 147,  70 }, { 148,  70 }, { 149,  70 }, { 150,  70 }, { 151,  70 }, { 152,  70 }, { 145,  70 },
	{ 227,  65 }, { 240,  65 }, { 242,  65 }, { 213,  65 }, { 215,  65 }, { 216,  65 }, { 221,  65 }, { 239,  65 },
	{  95,  65 }, { 103,  65 }, { 107,  65 }, { 121,  65 }, { 116,  65 }, { 117,  65 }, { 211,  65 }, {  99,  65 },
	{ 236,  65 }, {  60,  60 }, {  69,  60 }, {  70,  60 }, { 232,  55 }, {  92,  55 }, { 207,  55 }, {  19,  50 },
	{  25,  40 }
};

static clue_weigth clues_actor_10[44] = {
	{ 241,  70 }, { 130,  70 }, { 147,  70 }, { 145,  70 }, { 240,  65 }, { 216,  65 }, { 217,  65 }, { 219,  65 },
	{ 218,  65 }, { 221,  65 }, { 223,  65 }, { 224,  65 }, { 226,  65 }, { 229,  65 }, { 239,  65 }, {  95,  65 },
	{ 121,  65 }, { 148,  65 }, { 149,  65 }, { 150,  65 }, { 152,  65 }, { 116,  65 }, { 117,  65 }, { 214,  60 },
	{ 215,  60 }, {  69,  60 }, {  70,  60 }, { 211,  60 }, { 242,  55 }, { 213,  55 }, { 220,  55 }, { 222,  55 },
	{  60,  55 }, { 107,  55 }, { 133,  55 }, { 103,  50 }, {  92,  45 }, { 207,  45 }, { 227,  35 }, { 212,  35 },
	{ 230,  35 }, { 232,  35 }, {  19,  30 }, {  25,  30 }
};

void ScriptInit::Init_Clues2() {
	for(int i = 0; i != 288; ++i)
		Actor_Clue_Add_To_Database(99, i, 100, 0, 0, -1);

#define IMPORT_CLUE_TABLE(a, arr) \
	for (int i = 0; i != ARRAYSIZE(arr); ++i) \
		Actor_Clue_Add_To_Database( a, arr[i].clue, arr[i].weight, 0, 0, -1);

	IMPORT_CLUE_TABLE( 1, clues_actor_1);
	IMPORT_CLUE_TABLE( 2, clues_actor_2);
	IMPORT_CLUE_TABLE( 3, clues_actor_3);
	IMPORT_CLUE_TABLE( 4, clues_actor_4);
	IMPORT_CLUE_TABLE( 5, clues_actor_5);
	IMPORT_CLUE_TABLE( 6, clues_actor_6);
	IMPORT_CLUE_TABLE( 7, clues_actor_7_and_8);
	IMPORT_CLUE_TABLE( 9, clues_actor_9);
	IMPORT_CLUE_TABLE(10, clues_actor_10);

#undef IMPORT_CLUE_TABLE

	Actor_Clue_Add_To_Database(11, 201,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 213,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 212,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 218,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 219,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 221,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 222,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 225,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 229,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 230,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 232,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 116,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 117,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 207,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(11, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 213,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 219,  75, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 222,  75, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 223,  75, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 228,  75, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 124,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(12, 131, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 227,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 214,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 215,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 216,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 217,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 220,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 221,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 222,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 224,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 226,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 232,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13,  25,  30, 0, 0, -1);
	Actor_Clue_Add_To_Database(13,  60,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13,  69,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(13,  70,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(13,  92,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(13,  95,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 19,   55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 103,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 107,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 121,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 130,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 133,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 147,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 148,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 149,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 150,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 152,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 116,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 117,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 207,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(13, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(14,   5,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 239,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 240,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 241,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 242,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 222,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 212,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 215,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 216,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 230,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14,   0,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(14,  73,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 211,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 108,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 134,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 135,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(14,  66,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 109,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 110,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 111,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(14, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 221,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 222,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(15, 232,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 240,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 213,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 219,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(16,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(16, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 241,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 222,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 218,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 219,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 220,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 221,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 223,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 224,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 226,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 228,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 230,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 231,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(17,  73,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(17, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 239,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 240,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 242,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 222, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 227, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 216,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 217,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 223, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 224, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 228, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 231, 100, 0, 0, -1);
	Actor_Clue_Add_To_Database(18,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 207,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(18, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 218,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 219,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 213,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 229,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 232,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 116,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 117,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 207,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(19, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 240,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 213,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 219,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(20,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(20, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 214,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 219,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 223,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(21,  25,  30, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 147,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 148,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 150,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 152,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 117,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(21, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 240,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 213,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 219,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(22,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(22, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(23,  27,  20, 0, 0, -1);
	Actor_Clue_Add_To_Database(23,  16,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23,  17,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23,  26,  25, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 227,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 218,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 219,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 239,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database( 1,  73 , 65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(23, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 227,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 218,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 219,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 239,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(24,  17,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24,  16,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24,  27,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24,  26,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(24, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(25, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(25, 213,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(25, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26,   0,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26,   5,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26,   8,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(26,   9,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 239,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 240,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 242,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 222,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 216,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 217,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(26,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(26, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 227,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 241,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 215,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 216,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 217,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 220,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 219,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 218,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 221,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 222,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 223,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 224,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 226,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 228,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 230,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 232,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  25,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  60,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  69,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  70,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  92,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  95,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(27,  19,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 103,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 107,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 121,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 130,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 133,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 147,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 148,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 149,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 150,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 151,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 152,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 116,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 117,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 145,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 207,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(27, 211,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(28,  25,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(28,  64,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(28,  69,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 111,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 124,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 219,  75, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 241,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 217,  25, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 221,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 223,  75, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 225,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 222,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(28, 232,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 240,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 213,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 219,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(29,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(29, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 126,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 195,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 197,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 198,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 202,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 218,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 225,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 222,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 239,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(30,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(30, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 240,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 213,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 219,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(31,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(31, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 240,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 241,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 212,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 213,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 214,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 219,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 221,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 223,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 229,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 230,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 231,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 232,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 239,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  25,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  60,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  69,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  70,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  92,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32,  19,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 103,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 121,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 207,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(32, 211,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(33,   5,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33,   8,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(33,   9,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 239,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 240,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 242,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 222,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 216,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 217,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(33,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(33, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 126,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 195,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 197,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 198,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 202,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 219,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 218,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 225,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 222,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 239,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(34,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34,  80,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(34, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 214,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 219,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 223,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(35,  25,  30, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 147,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 148,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 150,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 152,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 117,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(35, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(37,   0,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37,   5,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37,   8,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(37,   9,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 239,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 240,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 242,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 222,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 216,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 217,  85, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(37,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(37, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 241,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 227,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 212,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 219,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 221,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 224,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 228,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 231,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 207,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(42, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 241,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 212,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(44, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 241,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 227,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 216,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 217,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 221,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 224,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 226,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 214,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 229,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 232,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 145,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(51, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 214,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 219,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 223,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(52,  25,  30, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 147,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 148,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 150,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 152,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 117,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(52, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 126,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 162,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 164,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 166,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 168,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 170,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 172,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 176,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 195,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 197,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 198,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 202,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 111,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 219,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 241,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 212,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 230,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 215,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 216,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 218,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 220,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 221,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 225,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 222,  90, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 242,  95, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(53, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 241,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 227,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 212,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 219,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 221,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 224,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 228,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 229,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 230,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 231,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55,  80,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 108,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 134,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 135,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 214,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 207,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(55, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 222,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 227,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 240,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 212,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 213,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 216,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 223,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 224,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 229,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 230,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 231,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 232,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 239,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  25,  30, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  60,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  69,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  70,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  92,  25, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  95,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(56,  19,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 103,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 107,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 121,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 130,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 133,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 147,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 149,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 150,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 151,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 152,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 116,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 117,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 145,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 207,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(56, 211,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 241,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 227,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 216,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 217,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 221,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 224,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 226,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 228,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 214,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 229,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 232,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 145,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(57, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(58, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(58, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(58, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(58, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(58, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 227,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 240,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 242,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 212,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 213,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 215,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 216,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 217,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 220,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 219,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 218,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 221,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 222,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 224,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 226,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 228,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 230,  35, 0, 0, -1);
	Actor_Clue_Add_To_Database(59,  95,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 232,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 130,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 147,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 148,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 149,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 150,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 151,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 152,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 116,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 117,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 145,  50, 0, 0, -1);
	Actor_Clue_Add_To_Database(59, 211,  60, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 239,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 241,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 222,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 227,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 212,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 215,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 216,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 217,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 218,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 219,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 220,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 221,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 223,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 224,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 226,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 228,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 229,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 230,  45, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 231,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 162,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 164,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 166,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 168,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 170,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 172,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 174,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 176,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62,   0,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(62,  73,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 211,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(62,  80,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 108,  55, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 134,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(62, 135,  40, 0, 0, -1);
	Actor_Clue_Add_To_Database(66, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(66, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(66, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(66, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(66, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(67, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(67, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(67, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(67, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(67, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(68, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(68, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(68, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(68, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(68, 214,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(69, 240,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(69, 241,  70, 0, 0, -1);
	Actor_Clue_Add_To_Database(69, 242,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(69, 239,  65, 0, 0, -1);
	Actor_Clue_Add_To_Database(69, 214,  65, 0, 0, -1);
}

void ScriptInit::Init_World_Waypoints() {

}

void ScriptInit::Init_SDB() {

}

void ScriptInit::Init_CDB() {

}

void ScriptInit::Init_Spinner() {

}

void ScriptInit::Init_Actor_Friendliness() {

}

void ScriptInit::Init_Actor_Combat_Aggressiveness() {

}

void ScriptInit::Init_Actor_Honesty() {

}

void ScriptInit::Init_Actor_Intelligence() {

}

void ScriptInit::Init_Actor_Stability() {

}

void ScriptInit::Init_Actor_Health() {

}

void ScriptInit::Init_Combat_Cover_Waypoints() {

}

void ScriptInit::Init_Combat_Flee_Waypoints() {

}

void ScriptInit::Init_Shadows() {

}



} // End of namespace BladeRunner
