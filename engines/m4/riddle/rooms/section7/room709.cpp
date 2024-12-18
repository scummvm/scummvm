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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section7/room709.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/riddle/vars.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room709::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room709::init() {
	_field88 = 0;
	_field84 = 0;

	AddSystemHotkey(KEY_CLEAR, clearPressed);
	AddSystemHotkey(5, clearPressed);
	digi_preload("950_s42");

	_field70 = 0;
	_field74 = 0;
	_field78 = 0;
	_field7C = 0;

	_ripTrekLowReachPos2Series = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
	_709rpro1Series = series_load("709RPRO1", -1, nullptr);
	_709rpro2Series = series_load("709RPRO2", -1, nullptr);
	_709rpro3Series = series_load("709RPRO3", -1, nullptr);
	_709rpro4Series = series_load("709RPRO4", -1, nullptr);
	_mazeLeftDoorLiteSeries = series_load("MAZE LEFT DOOR LITE", -1, nullptr);
	_mazeCenterDoorLiteSeries = series_load("MAZE CENTRE DOOR LITE", -1, nullptr);
	_mazeRightDoorLiteSeries = series_load("MAZE RIGHT DOOR LITE", -1, nullptr);
	_709NearDoorLiteSeries = series_load("709 NEAR DOOR LIGHT", -1, nullptr);

	_709rpro8Mach = series_place_sprite("709RPRO8", 0, 0, -53, 100, 2048);
	_709rpro7Mach = series_place_sprite("709RPRO7", 0, 0, -53, 100, 2304);
	_709rpro6Mach = series_place_sprite("709RPRO6", 0, 0, -53, 100, 1792);
	_709rpro5Mach = series_place_sprite("709RPRO5", 0, 0, -53, 100, 512);

	for (int i = 0; i < 99; ++i) {
		_maze709Arr[i]._field_0 = 0;
		_maze709Arr[i]._field_4 = 0;
		_maze709Arr[i]._field_8 = 0;
		_maze709Arr[i]._field_C = 0;
		_maze709Arr[i]._field_10 = 0;
		_maze709Arr[i]._field_14 = 0;
		_maze709Arr[i]._field_18 = 0;
		_maze709Arr[i]._field_1C = 0;
	}

	_field44 = 0;
	_field48 = 0;

	hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);

	_maze709Arr[1]._field_4 = 1;
	_maze709Arr[1]._field_14 = 2;
	_maze709Arr[1]._field_C = 1;
	_maze709Arr[1]._field_1C = 12;
	_maze709Arr[2]._field_0 = 1;
	_maze709Arr[2]._field_10 = 1;
	_maze709Arr[2]._field_C = 1;
	_maze709Arr[2]._field_1C = 13;
	_maze709Arr[3]._field_4 = 1;
	_maze709Arr[3]._field_14 = 4;
	_maze709Arr[3]._field_C = 1;
	_maze709Arr[3]._field_1C = 14;
	_maze709Arr[4]._field_4 = 1;
	_maze709Arr[4]._field_14 = 5;
	_maze709Arr[4]._field_0 = 1;
	_maze709Arr[4]._field_10 = 3;
	_maze709Arr[4]._field_C = 1;
	_maze709Arr[4]._field_1C = 15;
	_maze709Arr[5]._field_4 = 1;
	_maze709Arr[5]._field_14 = 6;
	_maze709Arr[5]._field_0 = 1;
	_maze709Arr[5]._field_10 = 4;
	_maze709Arr[6]._field_0 = 1;
	_maze709Arr[6]._field_10 = 5;
	_maze709Arr[6]._field_4 = 1;
	_maze709Arr[6]._field_14 = 7;
	_maze709Arr[6]._field_C = 1;
	_maze709Arr[6]._field_1C = 17;
	_maze709Arr[7]._field_4 = 1;
	_maze709Arr[7]._field_14 = 8;
	_maze709Arr[7]._field_0 = 1;
	_maze709Arr[7]._field_10 = 6;
	_maze709Arr[8]._field_4 = 1;
	_maze709Arr[8]._field_14 = 9;
	_maze709Arr[8]._field_0 = 1;
	_maze709Arr[8]._field_10 = 7;
	_maze709Arr[9]._field_0 = 1;
	_maze709Arr[9]._field_10 = 8;
	_maze709Arr[9]._field_C = 1;
	_maze709Arr[9]._field_1C = 20;
	_maze709Arr[10]._field_4 = 1;
	_maze709Arr[10]._field_14 = 11;
	_maze709Arr[10]._field_C = 1;
	_maze709Arr[10]._field_1C = 21;
	_maze709Arr[11]._field_0 = 1;
	_maze709Arr[11]._field_10 = 10;
	_maze709Arr[11]._field_C = 1;
	_maze709Arr[11]._field_1C = 22;
	_maze709Arr[12]._field_8 = 1;
	_maze709Arr[12]._field_18 = 1;
	_maze709Arr[13]._field_8 = 1;
	_maze709Arr[13]._field_18 = 2;
	_maze709Arr[13]._field_C = 1;
	_maze709Arr[13]._field_1C = 24;
	_maze709Arr[14]._field_8 = 1;
	_maze709Arr[14]._field_18 = 3;
	_maze709Arr[14]._field_C = 1;
	_maze709Arr[14]._field_1C = 25;
	_maze709Arr[15]._field_8 = 1;
	_maze709Arr[15]._field_18 = 4;
	_maze709Arr[15]._field_4 = 1;
	_maze709Arr[15]._field_14 = 16;
	_maze709Arr[16]._field_0 = 1;
	_maze709Arr[16]._field_10 = 15;
	_maze709Arr[17]._field_8 = 1;
	_maze709Arr[17]._field_18 = 6;
	_maze709Arr[18]._field_4 = 1;
	_maze709Arr[18]._field_14 = 19;
	_maze709Arr[18]._field_C = 1;
	_maze709Arr[18]._field_1C = 29;
	_maze709Arr[19]._field_4 = 1;
	_maze709Arr[19]._field_14 = 20;
	_maze709Arr[19]._field_0 = 1;
	_maze709Arr[19]._field_10 = 18;
	_maze709Arr[20]._field_8 = 1;
	_maze709Arr[20]._field_18 = 9;
	_maze709Arr[20]._field_4 = 1;
	_maze709Arr[20]._field_14 = 21;
	_maze709Arr[20]._field_0 = 1;
	_maze709Arr[20]._field_10 = 19;
	_maze709Arr[21]._field_8 = 1;
	_maze709Arr[21]._field_18 = 10;
	_maze709Arr[21]._field_0 = 1;
	_maze709Arr[21]._field_10 = 20;
	_maze709Arr[22]._field_8 = 1;
	_maze709Arr[22]._field_18 = 11;
	_maze709Arr[23]._field_4 = 1;
	_maze709Arr[23]._field_14 = 24;
	_maze709Arr[23]._field_C = 1;
	_maze709Arr[23]._field_1C = 34;
	_maze709Arr[24]._field_8 = 1;
	_maze709Arr[24]._field_18 = 13;
	_maze709Arr[24]._field_0 = 1;
	_maze709Arr[24]._field_10 = 23;
	_maze709Arr[25]._field_8 = 1;
	_maze709Arr[25]._field_18 = 14;
	_maze709Arr[25]._field_C = 1;
	_maze709Arr[25]._field_1C = 36;
	_maze709Arr[26]._field_0 = 1;
	_maze709Arr[26]._field_10 = 25;
	_maze709Arr[26]._field_C = 1;
	_maze709Arr[26]._field_1C = 37;
	_maze709Arr[27]._field_4 = 1;
	_maze709Arr[27]._field_14 = 28;
	_maze709Arr[27]._field_C = 1;
	_maze709Arr[27]._field_1C = 38;
	_maze709Arr[28]._field_0 = 1;
	_maze709Arr[28]._field_10 = 27;
	_maze709Arr[29]._field_8 = 1;
	_maze709Arr[29]._field_18 = 18;
	_maze709Arr[29]._field_C = 1;
	_maze709Arr[29]._field_1C = 40;
	_maze709Arr[30]._field_4 = 1;
	_maze709Arr[30]._field_14 = 31;
	_maze709Arr[30]._field_C = 1;
	_maze709Arr[30]._field_1C = 41;
	_maze709Arr[31]._field_4 = 1;
	_maze709Arr[31]._field_14 = 32;
	_maze709Arr[31]._field_0 = 1;
	_maze709Arr[31]._field_10 = 30;
	_maze709Arr[31]._field_C = 1;
	_maze709Arr[31]._field_1C = 42;
	_maze709Arr[32]._field_4 = 1;
	_maze709Arr[32]._field_14 = 33;
	_maze709Arr[32]._field_0 = 1;
	_maze709Arr[32]._field_10 = 31;
	_maze709Arr[33]._field_0 = 1;
	_maze709Arr[33]._field_10 = 32;
	_maze709Arr[33]._field_C = 1;
	_maze709Arr[33]._field_1C = 44;
	_maze709Arr[34]._field_8 = 1;
	_maze709Arr[34]._field_18 = 23;
	_maze709Arr[34]._field_C = 1;
	_maze709Arr[34]._field_1C = 45;
	_maze709Arr[35]._field_4 = 1;
	_maze709Arr[35]._field_14 = 36;
	_maze709Arr[36]._field_8 = 1;
	_maze709Arr[36]._field_18 = 25;
	_maze709Arr[36]._field_0 = 1;
	_maze709Arr[36]._field_10 = 35;
	_maze709Arr[36]._field_C = 1;
	_maze709Arr[36]._field_1C = 47;
	_maze709Arr[37]._field_8 = 1;
	_maze709Arr[37]._field_18 = 26;
	_maze709Arr[37]._field_4 = 1;
	_maze709Arr[37]._field_14 = 38;
	_maze709Arr[38]._field_8 = 1;
	_maze709Arr[38]._field_18 = 27;
	_maze709Arr[38]._field_0 = 1;
	_maze709Arr[38]._field_10 = 37;
	_maze709Arr[39]._field_4 = 1;
	_maze709Arr[39]._field_14 = 40;
	_maze709Arr[39]._field_C = 1;
	_maze709Arr[39]._field_1C = 50;
	_maze709Arr[40]._field_8 = 1;
	_maze709Arr[40]._field_18 = 29;
	_maze709Arr[40]._field_0 = 1;
	_maze709Arr[40]._field_10 = 39;
	_maze709Arr[41]._field_8 = 1;
	_maze709Arr[41]._field_18 = 30;
	_maze709Arr[41]._field_C = 1;
	_maze709Arr[41]._field_1C = 52;
	_maze709Arr[42]._field_8 = 1;
	_maze709Arr[42]._field_18 = 31;
	_maze709Arr[42]._field_C = 1;
	_maze709Arr[42]._field_1C = 53;
	_maze709Arr[42]._field_4 = 1;
	_maze709Arr[42]._field_14 = 43;
	_maze709Arr[43]._field_0 = 1;
	_maze709Arr[43]._field_10 = 42;
	_maze709Arr[43]._field_4 = 1;
	_maze709Arr[43]._field_14 = 44;
	_maze709Arr[44]._field_8 = 1;
	_maze709Arr[44]._field_18 = 33;
	_maze709Arr[44]._field_0 = 1;
	_maze709Arr[44]._field_10 = 43;
	_maze709Arr[45]._field_8 = 1;
	_maze709Arr[45]._field_18 = 34;
	_maze709Arr[45]._field_4 = 1;
	_maze709Arr[45]._field_14 = 46;
	_maze709Arr[45]._field_C = 1;
	_maze709Arr[45]._field_1C = 56;
	_maze709Arr[46]._field_0 = 1;
	_maze709Arr[46]._field_10 = 45;
	_maze709Arr[46]._field_4 = 1;
	_maze709Arr[46]._field_14 = 47;
	_maze709Arr[47]._field_8 = 1;
	_maze709Arr[47]._field_18 = 36;
	_maze709Arr[47]._field_0 = 1;
	_maze709Arr[47]._field_10 = 46;
	_maze709Arr[48]._field_4 = 1;
	_maze709Arr[48]._field_14 = 49;
	_maze709Arr[48]._field_C = 1;
	_maze709Arr[48]._field_1C = 59;
	_maze709Arr[49]._field_0 = 1;
	_maze709Arr[49]._field_10 = 48;
	_maze709Arr[50]._field_8 = 1;
	_maze709Arr[50]._field_18 = 39;
	_maze709Arr[51]._field_4 = 1;
	_maze709Arr[51]._field_14 = 52;
	_maze709Arr[51]._field_C = 1;
	_maze709Arr[51]._field_1C = 62;
	_maze709Arr[52]._field_8 = 1;
	_maze709Arr[52]._field_18 = 41;
	_maze709Arr[52]._field_0 = 1;
	_maze709Arr[52]._field_10 = 51;
	_maze709Arr[53]._field_8 = 1;
	_maze709Arr[53]._field_18 = 42;
	_maze709Arr[53]._field_4 = 1;
	_maze709Arr[53]._field_14 = 54;
	_maze709Arr[54]._field_4 = 1;
	_maze709Arr[54]._field_14 = 55;
	_maze709Arr[54]._field_0 = 1;
	_maze709Arr[54]._field_10 = 53;
	_maze709Arr[55]._field_0 = 1;
	_maze709Arr[55]._field_10 = 54;
	_maze709Arr[55]._field_C = 1;
	_maze709Arr[55]._field_1C = 66;
	_maze709Arr[56]._field_8 = 1;
	_maze709Arr[56]._field_18 = 45;
	_maze709Arr[56]._field_C = 1;
	_maze709Arr[56]._field_1C = 67;
	_maze709Arr[57]._field_0 = 1;
	_maze709Arr[57]._field_10 = 56;
	_maze709Arr[58]._field_4 = 1;
	_maze709Arr[58]._field_14 = 59;
	_maze709Arr[58]._field_C = 1;
	_maze709Arr[58]._field_1C = 69;
	_maze709Arr[59]._field_8 = 1;
	_maze709Arr[59]._field_18 = 48;
	_maze709Arr[59]._field_0 = 1;
	_maze709Arr[59]._field_10 = 58;
	_maze709Arr[59]._field_C = 1;
	_maze709Arr[59]._field_1C = 70;
	_maze709Arr[60]._field_4 = 1;
	_maze709Arr[60]._field_14 = 61;
	_maze709Arr[60]._field_C = 1;
	_maze709Arr[60]._field_1C = 71;
	_maze709Arr[61]._field_4 = 1;
	_maze709Arr[61]._field_14 = 62;
	_maze709Arr[61]._field_0 = 1;
	_maze709Arr[61]._field_10 = 60;
	_maze709Arr[62]._field_8 = 1;
	_maze709Arr[62]._field_18 = 51;
	_maze709Arr[62]._field_0 = 1;
	_maze709Arr[62]._field_10 = 61;
	_maze709Arr[63]._field_4 = 1;
	_maze709Arr[63]._field_14 = 64;
	_maze709Arr[64]._field_4 = 1;
	_maze709Arr[64]._field_14 = 65;
	_maze709Arr[64]._field_0 = 1;
	_maze709Arr[64]._field_10 = 63;
	_maze709Arr[64]._field_C = 1;
	_maze709Arr[64]._field_1C = 75;
	_maze709Arr[65]._field_4 = 1;
	_maze709Arr[65]._field_14 = 66;
	_maze709Arr[65]._field_0 = 1;
	_maze709Arr[65]._field_10 = 64;
	_maze709Arr[66]._field_8 = 1;
	_maze709Arr[66]._field_18 = 55;
	_maze709Arr[66]._field_0 = 1;
	_maze709Arr[66]._field_10 = 65;
	_maze709Arr[67]._field_8 = 1;
	_maze709Arr[67]._field_18 = 56;
	_maze709Arr[67]._field_C = 1;
	_maze709Arr[67]._field_1C = 78;
	_maze709Arr[68]._field_4 = 1;
	_maze709Arr[68]._field_14 = 69;
	_maze709Arr[68]._field_C = 1;
	_maze709Arr[68]._field_1C = 79;
	_maze709Arr[69]._field_0 = 1;
	_maze709Arr[69]._field_10 = 68;
	_maze709Arr[69]._field_8 = 1;
	_maze709Arr[69]._field_18 = 58;
	_maze709Arr[70]._field_8 = 1;
	_maze709Arr[70]._field_18 = 59;
	_maze709Arr[70]._field_4 = 1;
	_maze709Arr[70]._field_14 = 71;
	_maze709Arr[71]._field_8 = 1;
	_maze709Arr[71]._field_18 = 60;
	_maze709Arr[71]._field_0 = 1;
	_maze709Arr[71]._field_10 = 70;
	_maze709Arr[72]._field_4 = 1;
	_maze709Arr[72]._field_14 = 73;
	_maze709Arr[72]._field_C = 1;
	_maze709Arr[72]._field_1C = 83;
	_maze709Arr[73]._field_0 = 1;
	_maze709Arr[73]._field_10 = 72;
	_maze709Arr[73]._field_4 = 1;
	_maze709Arr[73]._field_14 = 74;
	_maze709Arr[74]._field_0 = 1;
	_maze709Arr[74]._field_10 = 73;
	_maze709Arr[74]._field_C = 1;
	_maze709Arr[74]._field_1C = 85;
	_maze709Arr[75]._field_8 = 1;
	_maze709Arr[75]._field_18 = 64;
	_maze709Arr[75]._field_4 = 1;
	_maze709Arr[75]._field_14 = 76;
	_maze709Arr[76]._field_0 = 1;
	_maze709Arr[76]._field_10 = 75;
	_maze709Arr[76]._field_4 = 1;
	_maze709Arr[76]._field_14 = 77;
	_maze709Arr[77]._field_0 = 1;
	_maze709Arr[77]._field_10 = 76;
	_maze709Arr[77]._field_C = 1;
	_maze709Arr[77]._field_1C = 88;
	_maze709Arr[78]._field_8 = 1;
	_maze709Arr[78]._field_18 = 67;
	_maze709Arr[78]._field_C = 1;
	_maze709Arr[78]._field_1C = 89;
	_maze709Arr[79]._field_8 = 1;
	_maze709Arr[79]._field_18 = 68;
	_maze709Arr[79]._field_4 = 1;
	_maze709Arr[79]._field_14 = 80;
	_maze709Arr[80]._field_4 = 1;
	_maze709Arr[80]._field_14 = 81;
	_maze709Arr[80]._field_0 = 1;
	_maze709Arr[80]._field_10 = 79;
	_maze709Arr[80]._field_C = 1;
	_maze709Arr[80]._field_1C = 91;
	_maze709Arr[81]._field_0 = 1;
	_maze709Arr[81]._field_10 = 80;
	_maze709Arr[81]._field_C = 1;
	_maze709Arr[81]._field_1C = 92;
	_maze709Arr[82]._field_4 = 1;
	_maze709Arr[82]._field_14 = 83;
	_maze709Arr[83]._field_8 = 1;
	_maze709Arr[83]._field_18 = 72;
	_maze709Arr[83]._field_0 = 1;
	_maze709Arr[83]._field_10 = 82;
	_maze709Arr[83]._field_C = 1;
	_maze709Arr[83]._field_1C = 94;
	_maze709Arr[84]._field_4 = 1;
	_maze709Arr[84]._field_14 = 85;
	_maze709Arr[84]._field_C = 1;
	_maze709Arr[84]._field_1C = 95;
	_maze709Arr[85]._field_8 = 1;
	_maze709Arr[85]._field_18 = 74;
	_maze709Arr[85]._field_0 = 1;
	_maze709Arr[85]._field_10 = 84;
	_maze709Arr[85]._field_4 = 1;
	_maze709Arr[85]._field_14 = 86;
	_maze709Arr[86]._field_0 = 1;
	_maze709Arr[86]._field_10 = 85;
	_maze709Arr[86]._field_4 = 1;
	_maze709Arr[86]._field_14 = 87;
	_maze709Arr[87]._field_0 = 1;
	_maze709Arr[87]._field_10 = 86;
	_maze709Arr[87]._field_C = 1;
	_maze709Arr[87]._field_1C = 98;
	_maze709Arr[88]._field_8 = 1;
	_maze709Arr[88]._field_18 = 77;
	_maze709Arr[88]._field_C = 1;
	_maze709Arr[88]._field_1C = 99;
	_maze709Arr[89]._field_8 = 1;
	_maze709Arr[89]._field_18 = 78;
	_maze709Arr[89]._field_4 = 1;
	_maze709Arr[89]._field_14 = 90;
	_maze709Arr[90]._field_4 = 1;
	_maze709Arr[90]._field_14 = 91;
	_maze709Arr[90]._field_0 = 1;
	_maze709Arr[90]._field_10 = 89;
	_maze709Arr[91]._field_8 = 1;
	_maze709Arr[91]._field_18 = 80;
	_maze709Arr[91]._field_0 = 1;
	_maze709Arr[91]._field_10 = 90;
	_maze709Arr[92]._field_8 = 1;
	_maze709Arr[92]._field_18 = 81;
	_maze709Arr[92]._field_4 = 1;
	_maze709Arr[92]._field_14 = 93;
	_maze709Arr[93]._field_0 = 1;
	_maze709Arr[93]._field_10 = 92;
	_maze709Arr[94]._field_8 = 1;
	_maze709Arr[94]._field_18 = 83;
	_maze709Arr[95]._field_8 = 1;
	_maze709Arr[95]._field_18 = 84;
	_maze709Arr[95]._field_4 = 1;
	_maze709Arr[95]._field_14 = 96;
	_maze709Arr[96]._field_0 = 1;
	_maze709Arr[96]._field_10 = 95;
	_maze709Arr[96]._field_4 = 1;
	_maze709Arr[96]._field_14 = 97;
	_maze709Arr[97]._field_0 = 1;
	_maze709Arr[97]._field_10 = 96;
	_maze709Arr[98]._field_8 = 1;
	_maze709Arr[98]._field_18 = 87;
	_maze709Arr[98]._field_4 = 1;
	_maze709Arr[98]._field_14 = 99;
	_btnFlag = 1;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s41", -1);
		if (_field80_save == 97 && inv_object_is_here("CHISEL")) {
			_field44 = 1;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_field80_save == 22 && inv_object_is_here("INCENSE BURNER")) {
			_field48 = 1;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_field80_save == 82) {
			_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
			sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 10, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
			_field70 = 1;
		}

		break;

	case 706:
		digi_preload("950_s41", -1);
		_field80_save = 82;
		_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
		sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 10, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
		_field70 = 1;
		ws_demand_location(_G(my_walker), 186, 290);
		ws_demand_facing(_G(my_walker), 2);
		ws_walk(_G(my_walker), 276, 300, nullptr, -1, 0, true);

		break;

	case 710:
		_field80_save = 39;
		ws_demand_location(_G(my_walker), 320, 400);
		ws_demand_facing(_G(my_walker), 1);

		break;

	default:
		_field80_save = 82;
		digi_preload("950_s41", -1);

		break;
	}

	digi_play_loop("950_s41", 3, 255, -1, -1);
}

void Room709::pre_parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to", "take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close", "RopeS", "RopeL", "RopeR", "RopeB");

	if (player_said("Right")) {
		if (_maze709Arr[_field80_save]._field_4 == 0 || _field74 == 0 || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Left")) {
		if (_maze709Arr[_field80_save]._field_0 == 0 || _field70 == 0 || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}

		if (_field80_save == 82) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Straight")) {
		if (_maze709Arr[_field80_save]._field_8 == 0 || _field78 == 0 || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Back")) {
		if (_maze709Arr[_field80_save]._field_C == 0 || _field7C == 0 || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if ((lookFl || talkFl || gearFl) && player_said_any(" ", "  ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (talkFl && player_said("Incense Burner") && inv_object_is_here("Incense Burner")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (talkFl && player_said("Chisel") && inv_object_is_here("Chisel")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (!lookFl && !talkFl && player_said_any("LIGHTER", "LIT LIGHTER")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room709::parser() {
	// TODO Not implemented yet
}

void Room709::daemon() {
	// TODO Not implemented yet
}

void Room709::clearPressed(void *, void *) {
	// TODO Not implemented yet
}


} // namespace Rooms
} // namespace Riddle
} // namespace M4
