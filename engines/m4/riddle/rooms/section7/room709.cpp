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

Room709::Room709() : Room() {
	resetMaze();
}

void Room709::resetMaze() {
	for (int i = 0; i < 99; ++i) {
		_maze709Arr[i]._leftActive = 0;
		_maze709Arr[i]._rightActive = 0;
		_maze709Arr[i]._straightActive = 0;
		_maze709Arr[i]._backActive = 0;
		_maze709Arr[i]._leftIndex = 0;
		_maze709Arr[i]._rightIndex = 0;
		_maze709Arr[i]._straightIndex = 0;
		_maze709Arr[i]._backIndex = 0;
	}
}

void Room709::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room709::syncGame(Common::Serializer &s) {
	s.syncAsSint32LE(_mazeCurrentIndex);
}

void Room709::init() {
	AddSystemHotkey(KEY_CLEAR, clearPressed);
	AddSystemHotkey(5, clearPressed);
	digi_preload("950_s42");

	_pullLeftFl = false;
	_pullRightFl = false;
	_pullCenterFl = false;
	_pullNearFl = false;

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

	resetMaze();

	_chiselActiveFl = false;
	_incenseBurnerActiveFl = false;

	hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);

	_maze709Arr[1]._rightActive = 1;
	_maze709Arr[1]._rightIndex = 2;
	_maze709Arr[1]._backActive = 1;
	_maze709Arr[1]._backIndex = 12;
	_maze709Arr[2]._leftActive = 1;
	_maze709Arr[2]._leftIndex = 1;
	_maze709Arr[2]._backActive = 1;
	_maze709Arr[2]._backIndex = 13;
	_maze709Arr[3]._rightActive = 1;
	_maze709Arr[3]._rightIndex = 4;
	_maze709Arr[3]._backActive = 1;
	_maze709Arr[3]._backIndex = 14;
	_maze709Arr[4]._rightActive = 1;
	_maze709Arr[4]._rightIndex = 5;
	_maze709Arr[4]._leftActive = 1;
	_maze709Arr[4]._leftIndex = 3;
	_maze709Arr[4]._backActive = 1;
	_maze709Arr[4]._backIndex = 15;
	_maze709Arr[5]._rightActive = 1;
	_maze709Arr[5]._rightIndex = 6;
	_maze709Arr[5]._leftActive = 1;
	_maze709Arr[5]._leftIndex = 4;
	_maze709Arr[6]._leftActive = 1;
	_maze709Arr[6]._leftIndex = 5;
	_maze709Arr[6]._rightActive = 1;
	_maze709Arr[6]._rightIndex = 7;
	_maze709Arr[6]._backActive = 1;
	_maze709Arr[6]._backIndex = 17;
	_maze709Arr[7]._rightActive = 1;
	_maze709Arr[7]._rightIndex = 8;
	_maze709Arr[7]._leftActive = 1;
	_maze709Arr[7]._leftIndex = 6;
	_maze709Arr[8]._rightActive = 1;
	_maze709Arr[8]._rightIndex = 9;
	_maze709Arr[8]._leftActive = 1;
	_maze709Arr[8]._leftIndex = 7;
	_maze709Arr[9]._leftActive = 1;
	_maze709Arr[9]._leftIndex = 8;
	_maze709Arr[9]._backActive = 1;
	_maze709Arr[9]._backIndex = 20;
	_maze709Arr[10]._rightActive = 1;
	_maze709Arr[10]._rightIndex = 11;
	_maze709Arr[10]._backActive = 1;
	_maze709Arr[10]._backIndex = 21;
	_maze709Arr[11]._leftActive = 1;
	_maze709Arr[11]._leftIndex = 10;
	_maze709Arr[11]._backActive = 1;
	_maze709Arr[11]._backIndex = 22;
	_maze709Arr[12]._straightActive = 1;
	_maze709Arr[12]._straightIndex = 1;
	_maze709Arr[13]._straightActive = 1;
	_maze709Arr[13]._straightIndex = 2;
	_maze709Arr[13]._backActive = 1;
	_maze709Arr[13]._backIndex = 24;
	_maze709Arr[14]._straightActive = 1;
	_maze709Arr[14]._straightIndex = 3;
	_maze709Arr[14]._backActive = 1;
	_maze709Arr[14]._backIndex = 25;
	_maze709Arr[15]._straightActive = 1;
	_maze709Arr[15]._straightIndex = 4;
	_maze709Arr[15]._rightActive = 1;
	_maze709Arr[15]._rightIndex = 16;
	_maze709Arr[16]._leftActive = 1;
	_maze709Arr[16]._leftIndex = 15;
	_maze709Arr[17]._straightActive = 1;
	_maze709Arr[17]._straightIndex = 6;
	_maze709Arr[18]._rightActive = 1;
	_maze709Arr[18]._rightIndex = 19;
	_maze709Arr[18]._backActive = 1;
	_maze709Arr[18]._backIndex = 29;
	_maze709Arr[19]._rightActive = 1;
	_maze709Arr[19]._rightIndex = 20;
	_maze709Arr[19]._leftActive = 1;
	_maze709Arr[19]._leftIndex = 18;
	_maze709Arr[20]._straightActive = 1;
	_maze709Arr[20]._straightIndex = 9;
	_maze709Arr[20]._rightActive = 1;
	_maze709Arr[20]._rightIndex = 21;
	_maze709Arr[20]._leftActive = 1;
	_maze709Arr[20]._leftIndex = 19;
	_maze709Arr[21]._straightActive = 1;
	_maze709Arr[21]._straightIndex = 10;
	_maze709Arr[21]._leftActive = 1;
	_maze709Arr[21]._leftIndex = 20;
	_maze709Arr[22]._straightActive = 1;
	_maze709Arr[22]._straightIndex = 11;
	_maze709Arr[23]._rightActive = 1;
	_maze709Arr[23]._rightIndex = 24;
	_maze709Arr[23]._backActive = 1;
	_maze709Arr[23]._backIndex = 34;
	_maze709Arr[24]._straightActive = 1;
	_maze709Arr[24]._straightIndex = 13;
	_maze709Arr[24]._leftActive = 1;
	_maze709Arr[24]._leftIndex = 23;
	_maze709Arr[25]._straightActive = 1;
	_maze709Arr[25]._straightIndex = 14;
	_maze709Arr[25]._backActive = 1;
	_maze709Arr[25]._backIndex = 36;
	_maze709Arr[26]._leftActive = 1;
	_maze709Arr[26]._leftIndex = 25;
	_maze709Arr[26]._backActive = 1;
	_maze709Arr[26]._backIndex = 37;
	_maze709Arr[27]._rightActive = 1;
	_maze709Arr[27]._rightIndex = 28;
	_maze709Arr[27]._backActive = 1;
	_maze709Arr[27]._backIndex = 38;
	_maze709Arr[28]._leftActive = 1;
	_maze709Arr[28]._leftIndex = 27;
	_maze709Arr[29]._straightActive = 1;
	_maze709Arr[29]._straightIndex = 18;
	_maze709Arr[29]._backActive = 1;
	_maze709Arr[29]._backIndex = 40;
	_maze709Arr[30]._rightActive = 1;
	_maze709Arr[30]._rightIndex = 31;
	_maze709Arr[30]._backActive = 1;
	_maze709Arr[30]._backIndex = 41;
	_maze709Arr[31]._rightActive = 1;
	_maze709Arr[31]._rightIndex = 32;
	_maze709Arr[31]._leftActive = 1;
	_maze709Arr[31]._leftIndex = 30;
	_maze709Arr[31]._backActive = 1;
	_maze709Arr[31]._backIndex = 42;
	_maze709Arr[32]._rightActive = 1;
	_maze709Arr[32]._rightIndex = 33;
	_maze709Arr[32]._leftActive = 1;
	_maze709Arr[32]._leftIndex = 31;
	_maze709Arr[33]._leftActive = 1;
	_maze709Arr[33]._leftIndex = 32;
	_maze709Arr[33]._backActive = 1;
	_maze709Arr[33]._backIndex = 44;
	_maze709Arr[34]._straightActive = 1;
	_maze709Arr[34]._straightIndex = 23;
	_maze709Arr[34]._backActive = 1;
	_maze709Arr[34]._backIndex = 45;
	_maze709Arr[35]._rightActive = 1;
	_maze709Arr[35]._rightIndex = 36;
	_maze709Arr[36]._straightActive = 1;
	_maze709Arr[36]._straightIndex = 25;
	_maze709Arr[36]._leftActive = 1;
	_maze709Arr[36]._leftIndex = 35;
	_maze709Arr[36]._backActive = 1;
	_maze709Arr[36]._backIndex = 47;
	_maze709Arr[37]._straightActive = 1;
	_maze709Arr[37]._straightIndex = 26;
	_maze709Arr[37]._rightActive = 1;
	_maze709Arr[37]._rightIndex = 38;
	_maze709Arr[38]._straightActive = 1;
	_maze709Arr[38]._straightIndex = 27;
	_maze709Arr[38]._leftActive = 1;
	_maze709Arr[38]._leftIndex = 37;
	_maze709Arr[39]._rightActive = 1;
	_maze709Arr[39]._rightIndex = 40;
	_maze709Arr[39]._backActive = 1;
	_maze709Arr[39]._backIndex = 50;
	_maze709Arr[40]._straightActive = 1;
	_maze709Arr[40]._straightIndex = 29;
	_maze709Arr[40]._leftActive = 1;
	_maze709Arr[40]._leftIndex = 39;
	_maze709Arr[41]._straightActive = 1;
	_maze709Arr[41]._straightIndex = 30;
	_maze709Arr[41]._backActive = 1;
	_maze709Arr[41]._backIndex = 52;
	_maze709Arr[42]._straightActive = 1;
	_maze709Arr[42]._straightIndex = 31;
	_maze709Arr[42]._backActive = 1;
	_maze709Arr[42]._backIndex = 53;
	_maze709Arr[42]._rightActive = 1;
	_maze709Arr[42]._rightIndex = 43;
	_maze709Arr[43]._leftActive = 1;
	_maze709Arr[43]._leftIndex = 42;
	_maze709Arr[43]._rightActive = 1;
	_maze709Arr[43]._rightIndex = 44;
	_maze709Arr[44]._straightActive = 1;
	_maze709Arr[44]._straightIndex = 33;
	_maze709Arr[44]._leftActive = 1;
	_maze709Arr[44]._leftIndex = 43;
	_maze709Arr[45]._straightActive = 1;
	_maze709Arr[45]._straightIndex = 34;
	_maze709Arr[45]._rightActive = 1;
	_maze709Arr[45]._rightIndex = 46;
	_maze709Arr[45]._backActive = 1;
	_maze709Arr[45]._backIndex = 56;
	_maze709Arr[46]._leftActive = 1;
	_maze709Arr[46]._leftIndex = 45;
	_maze709Arr[46]._rightActive = 1;
	_maze709Arr[46]._rightIndex = 47;
	_maze709Arr[47]._straightActive = 1;
	_maze709Arr[47]._straightIndex = 36;
	_maze709Arr[47]._leftActive = 1;
	_maze709Arr[47]._leftIndex = 46;
	_maze709Arr[48]._rightActive = 1;
	_maze709Arr[48]._rightIndex = 49;
	_maze709Arr[48]._backActive = 1;
	_maze709Arr[48]._backIndex = 59;
	_maze709Arr[49]._leftActive = 1;
	_maze709Arr[49]._leftIndex = 48;
	_maze709Arr[50]._straightActive = 1;
	_maze709Arr[50]._straightIndex = 39;
	_maze709Arr[51]._rightActive = 1;
	_maze709Arr[51]._rightIndex = 52;
	_maze709Arr[51]._backActive = 1;
	_maze709Arr[51]._backIndex = 62;
	_maze709Arr[52]._straightActive = 1;
	_maze709Arr[52]._straightIndex = 41;
	_maze709Arr[52]._leftActive = 1;
	_maze709Arr[52]._leftIndex = 51;
	_maze709Arr[53]._straightActive = 1;
	_maze709Arr[53]._straightIndex = 42;
	_maze709Arr[53]._rightActive = 1;
	_maze709Arr[53]._rightIndex = 54;
	_maze709Arr[54]._rightActive = 1;
	_maze709Arr[54]._rightIndex = 55;
	_maze709Arr[54]._leftActive = 1;
	_maze709Arr[54]._leftIndex = 53;
	_maze709Arr[55]._leftActive = 1;
	_maze709Arr[55]._leftIndex = 54;
	_maze709Arr[55]._backActive = 1;
	_maze709Arr[55]._backIndex = 66;
	_maze709Arr[56]._straightActive = 1;
	_maze709Arr[56]._straightIndex = 45;
	_maze709Arr[56]._backActive = 1;
	_maze709Arr[56]._backIndex = 67;
	_maze709Arr[57]._leftActive = 1;
	_maze709Arr[57]._leftIndex = 56;
	_maze709Arr[58]._rightActive = 1;
	_maze709Arr[58]._rightIndex = 59;
	_maze709Arr[58]._backActive = 1;
	_maze709Arr[58]._backIndex = 69;
	_maze709Arr[59]._straightActive = 1;
	_maze709Arr[59]._straightIndex = 48;
	_maze709Arr[59]._leftActive = 1;
	_maze709Arr[59]._leftIndex = 58;
	_maze709Arr[59]._backActive = 1;
	_maze709Arr[59]._backIndex = 70;
	_maze709Arr[60]._rightActive = 1;
	_maze709Arr[60]._rightIndex = 61;
	_maze709Arr[60]._backActive = 1;
	_maze709Arr[60]._backIndex = 71;
	_maze709Arr[61]._rightActive = 1;
	_maze709Arr[61]._rightIndex = 62;
	_maze709Arr[61]._leftActive = 1;
	_maze709Arr[61]._leftIndex = 60;
	_maze709Arr[62]._straightActive = 1;
	_maze709Arr[62]._straightIndex = 51;
	_maze709Arr[62]._leftActive = 1;
	_maze709Arr[62]._leftIndex = 61;
	_maze709Arr[63]._rightActive = 1;
	_maze709Arr[63]._rightIndex = 64;
	_maze709Arr[64]._rightActive = 1;
	_maze709Arr[64]._rightIndex = 65;
	_maze709Arr[64]._leftActive = 1;
	_maze709Arr[64]._leftIndex = 63;
	_maze709Arr[64]._backActive = 1;
	_maze709Arr[64]._backIndex = 75;
	_maze709Arr[65]._rightActive = 1;
	_maze709Arr[65]._rightIndex = 66;
	_maze709Arr[65]._leftActive = 1;
	_maze709Arr[65]._leftIndex = 64;
	_maze709Arr[66]._straightActive = 1;
	_maze709Arr[66]._straightIndex = 55;
	_maze709Arr[66]._leftActive = 1;
	_maze709Arr[66]._leftIndex = 65;
	_maze709Arr[67]._straightActive = 1;
	_maze709Arr[67]._straightIndex = 56;
	_maze709Arr[67]._backActive = 1;
	_maze709Arr[67]._backIndex = 78;
	_maze709Arr[68]._rightActive = 1;
	_maze709Arr[68]._rightIndex = 69;
	_maze709Arr[68]._backActive = 1;
	_maze709Arr[68]._backIndex = 79;
	_maze709Arr[69]._leftActive = 1;
	_maze709Arr[69]._leftIndex = 68;
	_maze709Arr[69]._straightActive = 1;
	_maze709Arr[69]._straightIndex = 58;
	_maze709Arr[70]._straightActive = 1;
	_maze709Arr[70]._straightIndex = 59;
	_maze709Arr[70]._rightActive = 1;
	_maze709Arr[70]._rightIndex = 71;
	_maze709Arr[71]._straightActive = 1;
	_maze709Arr[71]._straightIndex = 60;
	_maze709Arr[71]._leftActive = 1;
	_maze709Arr[71]._leftIndex = 70;
	_maze709Arr[72]._rightActive = 1;
	_maze709Arr[72]._rightIndex = 73;
	_maze709Arr[72]._backActive = 1;
	_maze709Arr[72]._backIndex = 83;
	_maze709Arr[73]._leftActive = 1;
	_maze709Arr[73]._leftIndex = 72;
	_maze709Arr[73]._rightActive = 1;
	_maze709Arr[73]._rightIndex = 74;
	_maze709Arr[74]._leftActive = 1;
	_maze709Arr[74]._leftIndex = 73;
	_maze709Arr[74]._backActive = 1;
	_maze709Arr[74]._backIndex = 85;
	_maze709Arr[75]._straightActive = 1;
	_maze709Arr[75]._straightIndex = 64;
	_maze709Arr[75]._rightActive = 1;
	_maze709Arr[75]._rightIndex = 76;
	_maze709Arr[76]._leftActive = 1;
	_maze709Arr[76]._leftIndex = 75;
	_maze709Arr[76]._rightActive = 1;
	_maze709Arr[76]._rightIndex = 77;
	_maze709Arr[77]._leftActive = 1;
	_maze709Arr[77]._leftIndex = 76;
	_maze709Arr[77]._backActive = 1;
	_maze709Arr[77]._backIndex = 88;
	_maze709Arr[78]._straightActive = 1;
	_maze709Arr[78]._straightIndex = 67;
	_maze709Arr[78]._backActive = 1;
	_maze709Arr[78]._backIndex = 89;
	_maze709Arr[79]._straightActive = 1;
	_maze709Arr[79]._straightIndex = 68;
	_maze709Arr[79]._rightActive = 1;
	_maze709Arr[79]._rightIndex = 80;
	_maze709Arr[80]._rightActive = 1;
	_maze709Arr[80]._rightIndex = 81;
	_maze709Arr[80]._leftActive = 1;
	_maze709Arr[80]._leftIndex = 79;
	_maze709Arr[80]._backActive = 1;
	_maze709Arr[80]._backIndex = 91;
	_maze709Arr[81]._leftActive = 1;
	_maze709Arr[81]._leftIndex = 80;
	_maze709Arr[81]._backActive = 1;
	_maze709Arr[81]._backIndex = 92;
	_maze709Arr[82]._rightActive = 1;
	_maze709Arr[82]._rightIndex = 83;
	_maze709Arr[83]._straightActive = 1;
	_maze709Arr[83]._straightIndex = 72;
	_maze709Arr[83]._leftActive = 1;
	_maze709Arr[83]._leftIndex = 82;
	_maze709Arr[83]._backActive = 1;
	_maze709Arr[83]._backIndex = 94;
	_maze709Arr[84]._rightActive = 1;
	_maze709Arr[84]._rightIndex = 85;
	_maze709Arr[84]._backActive = 1;
	_maze709Arr[84]._backIndex = 95;
	_maze709Arr[85]._straightActive = 1;
	_maze709Arr[85]._straightIndex = 74;
	_maze709Arr[85]._leftActive = 1;
	_maze709Arr[85]._leftIndex = 84;
	_maze709Arr[85]._rightActive = 1;
	_maze709Arr[85]._rightIndex = 86;
	_maze709Arr[86]._leftActive = 1;
	_maze709Arr[86]._leftIndex = 85;
	_maze709Arr[86]._rightActive = 1;
	_maze709Arr[86]._rightIndex = 87;
	_maze709Arr[87]._leftActive = 1;
	_maze709Arr[87]._leftIndex = 86;
	_maze709Arr[87]._backActive = 1;
	_maze709Arr[87]._backIndex = 98;
	_maze709Arr[88]._straightActive = 1;
	_maze709Arr[88]._straightIndex = 77;
	_maze709Arr[88]._backActive = 1;
	_maze709Arr[88]._backIndex = 99;
	_maze709Arr[89]._straightActive = 1;
	_maze709Arr[89]._straightIndex = 78;
	_maze709Arr[89]._rightActive = 1;
	_maze709Arr[89]._rightIndex = 90;
	_maze709Arr[90]._rightActive = 1;
	_maze709Arr[90]._rightIndex = 91;
	_maze709Arr[90]._leftActive = 1;
	_maze709Arr[90]._leftIndex = 89;
	_maze709Arr[91]._straightActive = 1;
	_maze709Arr[91]._straightIndex = 80;
	_maze709Arr[91]._leftActive = 1;
	_maze709Arr[91]._leftIndex = 90;
	_maze709Arr[92]._straightActive = 1;
	_maze709Arr[92]._straightIndex = 81;
	_maze709Arr[92]._rightActive = 1;
	_maze709Arr[92]._rightIndex = 93;
	_maze709Arr[93]._leftActive = 1;
	_maze709Arr[93]._leftIndex = 92;
	_maze709Arr[94]._straightActive = 1;
	_maze709Arr[94]._straightIndex = 83;
	_maze709Arr[95]._straightActive = 1;
	_maze709Arr[95]._straightIndex = 84;
	_maze709Arr[95]._rightActive = 1;
	_maze709Arr[95]._rightIndex = 96;
	_maze709Arr[96]._leftActive = 1;
	_maze709Arr[96]._leftIndex = 95;
	_maze709Arr[96]._rightActive = 1;
	_maze709Arr[96]._rightIndex = 97;
	_maze709Arr[97]._leftActive = 1;
	_maze709Arr[97]._leftIndex = 96;
	_maze709Arr[98]._straightActive = 1;
	_maze709Arr[98]._straightIndex = 87;
	_maze709Arr[98]._rightActive = 1;
	_maze709Arr[98]._rightIndex = 99;
	
	_G(room303_btnFlag) = true;
	_G(global301) = 88;
	_G(flag1) = 1;
	_G(204_dword1A1898) = 98;
	
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s41", -1);
		if (_mazeCurrentIndex == 97 && inv_object_is_here("CHISEL")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_mazeCurrentIndex == 82) {
			_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
			sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 10, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
			_pullLeftFl = true;
		}

		break;

	case 706:
		digi_preload("950_s41", -1);
		_mazeCurrentIndex = 82;
		_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
		sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 10, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
		_pullLeftFl = true;
		ws_demand_location(_G(my_walker), 186, 290);
		ws_demand_facing(_G(my_walker), 2);
		ws_walk(_G(my_walker), 276, 300, nullptr, -1, 0, true);

		break;

	case 710:
		_mazeCurrentIndex = 39;
		ws_demand_location(_G(my_walker), 320, 400);
		ws_demand_facing(_G(my_walker), 1);

		break;

	default:
		_mazeCurrentIndex = 82;
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
		if (_maze709Arr[_mazeCurrentIndex]._rightActive == 0 || !_pullRightFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Left")) {
		if (_maze709Arr[_mazeCurrentIndex]._leftActive == 0 || !_pullLeftFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}

		if (_mazeCurrentIndex == 82) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Straight")) {
		if (_maze709Arr[_mazeCurrentIndex]._straightActive == 0 || !_pullCenterFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Back")) {
		if (_maze709Arr[_mazeCurrentIndex]._backActive == 0 || !_pullNearFl || lookFl || talkFl || gearFl) {
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
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to", "take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close", "RopeS", "RopeL", "RopeR", "RopeB");

	if (gearFl && player_said("rope   ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro8Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 2048, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro4Series, 1, 16, 1, _709rpro4Series, 16, 16, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro4Series, 17, 38, 2, _709rpro4Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_maze709Arr[_mazeCurrentIndex]._rightActive != 0 && !_pullRightFl) {
				_ripPullMach03 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach03, _mazeRightDoorLiteSeries, 1, 10, 3, _mazeRightDoorLiteSeries, 10, 10, 0);
				_pullRightFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro8Mach = series_place_sprite("709RPRO8", 0, 0, -53, 100, 2048);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope   ")

	else if (gearFl && player_said("rope     ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro6Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1792, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro2Series, 1, 15, 1, _709rpro2Series, 15, 15, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro2Series, 15, 38, 2, _709rpro2Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_maze709Arr[_mazeCurrentIndex]._leftActive != 0 && !_pullLeftFl) {
				_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 1, 10, 3, _mazeLeftDoorLiteSeries, 10, 10, 0);
				_pullLeftFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro6Mach = series_place_sprite("709RPRO6", 0, 0, -53, 100, 1792);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope     ")

	else if (gearFl && player_said("rope  ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro7Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 2304, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro3Series, 1, 16, 1, _709rpro3Series, 16, 16, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro3Series, 16, 38, 2, _709rpro3Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_maze709Arr[_mazeCurrentIndex]._straightActive != 0 && !_pullCenterFl) {
				_ripPullMach04 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach04, _mazeCenterDoorLiteSeries, 1, 10, 3, _mazeCenterDoorLiteSeries, 10, 10, 0);
				_pullCenterFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro7Mach = series_place_sprite("709RPRO7", 0, 0, -53, 100, 2304);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;

		}
	} // esi && player_said("rope  ")

	else if (gearFl && player_said("rope    ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro5Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 512, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro1Series, 1, 17, 1, _709rpro1Series, 17, 17, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro1Series, 17, 38, 2, _709rpro1Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_maze709Arr[_mazeCurrentIndex]._backActive != 0 && !_pullNearFl) {
				_ripPullMach05 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach05, _709NearDoorLiteSeries, 1, 10, 3, _709NearDoorLiteSeries, 10, 10, 0);
				_pullNearFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro5Mach = series_place_sprite("709RPRO5", 0, 0, -53, 100, 512);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope    ")

	else if (player_said("Right")) {
		if (_maze709Arr[_mazeCurrentIndex]._rightActive == 0 || !_pullRightFl) {
			digi_play("708R01", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);
				kernel_timing_trigger(30, 2, nullptr);

				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(30));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}
	} // player_said("Right")

	else if (player_said("Left")) {
		if (_maze709Arr[_mazeCurrentIndex]._leftActive == 0 || !_pullLeftFl) {
			if (_mazeCurrentIndex == 82) {
				switch (_G(kernel).trigger) {
				case -1:
					ws_walk(_G(my_walker), 182, 292, nullptr, 2, 9, true);
					break;

				case 2:
					disable_player_commands_and_fade_init(3);
					break;

				case 3:
					RemoveSystemHotkey(KEY_CLEAR);
					RemoveSystemHotkey(5);
					_G(game).new_room = 706;
					break;

				default:
					break;
				}
			} else {
				digi_play("708R01", 1, 255, -1, -1);
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);
				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(20));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}

	} // player_said("Left")

	else if (player_said("Straight")) {
		if (_maze709Arr[_mazeCurrentIndex]._straightActive == 0 || !_pullCenterFl) {
			digi_play("708R01", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);

				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(40));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}
	} // player_said("Straight")

	else if (player_said("Back")) {
		if (_maze709Arr[_mazeCurrentIndex]._backActive == 0 || !_pullNearFl) {
			digi_play("708R01", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);

				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(50));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}
	} // player_said("Back")

	else if (lookFl && player_said("Incense Burner") && inv_object_is_here("Incense Burner")) {
		digi_play("709R12", 1, 255, -1, -1);
	} else if (lookFl && player_said("Chisel") && inv_object_is_here("Chisel")) {
		digi_play("709R02", 1, 255, -1, -1);
	} else if (lookFl && player_said_any("rope  ", "rope   ", "rope    ", "rope     ")) {
		digi_play("com110", 1, 255, -1, 997);
	} else if (talkFl && player_said("Incense Burner") && inv_object_is_here("Incense Burner")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(_G(my_walker), 373, 279, nullptr, 2, 2, true);
			break;

		case 2:
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(10));
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} else if (talkFl && player_said("Chisel") && inv_object_is_here("chisel")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(_G(my_walker), 267, 278, nullptr, 2, 10, true);
			break;

		case 2:
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(10));
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} else if (!lookFl && player_said("journal") && !inv_player_has(_G(player).noun)) {
		digi_play("709R11", 1, 255, -1, -1);
	} else if (!lookFl && !talkFl && player_said_any("LIGHTER", "LIT LIGHTER")) {
		digi_play("com141", 1, 255, -1, 997);
	} else if (lookFl && player_said(" ")) {
		digi_play("709R01", 1, 255, -1, -1);
	} else if (lookFl && player_said("  ")) {
		digi_play("708R02", 1, 255, -1, 708);
	} else if (gearFl || talkFl) {
		switch (imath_ranged_rand(1, 5)) {
		case 1:
			digi_play("com006", 1, 255, -1, 997);
			break;

		case 2:
			digi_play("com008", 1, 255, -1, 997);
			break;

		case 3:
			digi_play("com013", 1, 255, -1, 997);
			break;

		case 4:
			digi_play("com010", 1, 255, -1, 997);
			break;

		case 5:
			digi_play("com011", 1, 255, -1, 997);
			break;

		default:
			break;
		}
	} else
		return;

	_G(player).command_ready = false;
}

void Room709::daemon() {
	int32 delay;

	switch (_G(kernel).trigger) {
	case 10:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekLowReachPos2Series, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 11);

		break;

	case 11:
		if (_mazeCurrentIndex == 97) {
			inv_give_to_player("CHISEL");
			terminateMachine(_709ChiselMach);
			_chiselActiveFl = false;
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			kernel_examine_inventory_object("PING CHISEL", _G(master_palette), 5, 1, 163, 203, -1, nullptr, -1);
			digi_play("709R03", 1, 255, 12, -1);
		}

		if (_mazeCurrentIndex == 22) {
			inv_give_to_player("INCENSE BURNER");
			terminateMachine(_709IncenseHolderMach);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			_incenseBurnerActiveFl = false;
			kernel_examine_inventory_object("PING INCENSE BURNER", _G(master_palette), 5, 1, 329, 189, -1, nullptr, -1);
			digi_play("709R13", 1, 255, 12, -1);
		}

		break;

	case 12:
		sendWSMessage_120000(_G(my_walker), 13);
		break;

	case 13:
		kernel_timing_trigger(5, 14, nullptr);
		break;

	case 14:
		sendWSMessage_150000(_G(my_walker), 15);
		break;

	case 15:
		player_set_commands_allowed(true);
		break;

	case 20: // related to player_said("left")
		ws_demand_location(_G(my_walker), 410, 300);
		ws_demand_facing(_G(my_walker), 9);
		_mazeCurrentIndex = _maze709Arr[_mazeCurrentIndex]._leftIndex;
		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("CHISEL")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		if (_mazeCurrentIndex == 82) {
			_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
			sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 1, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
			_pullLeftFl = true;
			pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
			player_set_commands_allowed(true);
			delay = imath_ranged_rand(1, 10);
			if (delay == 10) {
				kernel_timing_trigger(delay, 1000, nullptr);
			}
		}

		break;

	case 30:
		ws_demand_location(_G(my_walker), 230, 300);
		ws_demand_facing(_G(my_walker), 3);
		_mazeCurrentIndex = _maze709Arr[_mazeCurrentIndex]._rightIndex;
		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("Chisel")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0,0,0,100,3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}

		break;

	case 40:
		ws_demand_location(_G(my_walker), 320, 400);
		ws_demand_facing(_G(my_walker), 1);
		_mazeCurrentIndex = _maze709Arr[_mazeCurrentIndex]._straightIndex;
		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("Chisel")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}

		break;

	case 50:
		ws_demand_location(_G(my_walker), 320, 280);
		ws_demand_facing(_G(my_walker), 5);
		_mazeCurrentIndex = _maze709Arr[_mazeCurrentIndex]._backIndex;

		if (_mazeCurrentIndex == 50) {
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s41", 3, 255, -1, -1);
			RemoveSystemHotkey(KEY_CLEAR);
			RemoveSystemHotkey(5);
			_G(game).new_room = 710;
		}

		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("Chisel")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}

		break;

	case 100:
		digi_play("950_s41", 3, 255, 100, -1);
		break;

	case 500:
		digi_stop(1);
		digi_play_loop("304_s05", 2, 255, -1, -1);
		kernel_timing_trigger(20, 502, nullptr);

		break;

	case 502:
		digi_play("709_s99", 3, 255, 504, -1);
		break;

	case 504:
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s41", 3, 255, -1, -1);
		RemoveSystemHotkey(KEY_CLEAR);
		RemoveSystemHotkey(5);
		_G(game).new_room = 710;

		break;

	case 1000:
		player_set_commands_allowed(false);
		kernel_timing_trigger(10, 1001, nullptr);

		break;

	case 1001:
		player_update_info(_G(my_walker), &_G(player_info));

		switch (_G(player_info).facing) {
		case 1:
			kernel_timing_trigger(10, 1002);
			break;

		case 3:
		case 9:
			kernel_timing_trigger(10, 1047);
			break;

		case 5:
			kernel_timing_trigger(10, 1022);
			break;

		default:
			break;
		}
		break;

	case 1002:
		kernel_timing_trigger(10, imath_ranged_rand(1, 2) == 1 ? 1007 : 1012, nullptr);
		break;

	case 1007:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HEAD TURN POS1", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 3, 3, 3, 0, 3, 1, 1, 1, 0, 4, 8, 8, 8, 0, 8, 4, 4, 4, 0);
		sendWSMessage_110000(_G(my_walker), 1008);

		break;

	case 1008:
		sendWSMessage_190000(_G(my_walker), 5);
		sendWSMessage_120000(_G(my_walker), 1009);

		break;

	case 1009:
		sendWSMessage_130000(_G(my_walker), 1010);
		break;

	case 1010:
		sendWSMessage_180000(_G(my_walker), 1011);
		break;

	case 1011:
		sendWSMessage_150000(_G(my_walker), 1073);
		break;

	case 1012:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HANDS HIPS POS1", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 1, 1, 1, 0, 1, 12, 12, 12, 0, 12, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1013);

		break;

	case 1013:
		sendWSMessage_190000(_G(my_walker), 5);
		sendWSMessage_120000(_G(my_walker), 1014);
		break;

	case 1014:
		sendWSMessage_130000(_G(my_walker), 1015);
		break;

	case 1015:
	case 1031:
	case 1034:
	case 1041:
	case 1061:
	case 1070:
		sendWSMessage_150000(_G(my_walker), 1073);
		break;

	case 1022:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			kernel_timing_trigger(10, 1027, nullptr);
			break;

		case 2:
			kernel_timing_trigger(10, 1032, nullptr);
			break;

		case 3:
			kernel_timing_trigger(10, 1037, nullptr);
			break;


		default:
			break;
		}
		break;

	case 1027:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HEAD TURN POS5", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 7, 7, 7, 0, 7, 1, 1, 1, 0, 8, 12, 12, 12, 0, 12, 8, 8, 8, 0);
		sendWSMessage_110000(_G(my_walker), 1028);

		break;

	case 1028:
		sendWSMessage_120000(_G(my_walker), 1029);
		break;

	case 1029:
		sendWSMessage_130000(_G(my_walker), 1030);
		break;

	case 1030:
		sendWSMessage_180000(_G(my_walker), 1031);
		break;

	case 1032:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HANDS HIP POS5", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1033);

		break;

	case 1033:
		sendWSMessage_120000(_G(my_walker), 1034);
		break;

	case 1037:
		_ripTrekHeadTurnPos1Series = series_load("RIP LOOKS L R DOWN", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 4, 4, 4, 0, 4, 1, 1, 1, 0, 5, 9, 9, 9, 0, 9, 5, 5, 5, 0);
		sendWSMessage_110000(_G(my_walker), 1038);

		break;

	case 1038:
		sendWSMessage_120000(_G(my_walker), 1039);
		break;

	case 1039:
		sendWSMessage_130000(_G(my_walker), 1040);
		break;

	case 1040:
		sendWSMessage_180000(_G(my_walker), 1041);
		break;

	case 1047:
		kernel_timing_trigger(10, imath_ranged_rand(1, 2) == 1 ? 1057 : 1067, nullptr);
		break;

	case 1057:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HEAD TURN POS3", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 5, 5, 5, 0, 5, 1, 1, 1, 0, 6, 9, 9, 9, 0, 9, 6, 6, 6, 0);
		sendWSMessage_110000(_G(my_walker), 1058);

		break;

	case 1058:
		sendWSMessage_120000(_G(my_walker), 1059);
		break;

	case 1059:
		sendWSMessage_130000(_G(my_walker), 1060);
		break;

	case 1060:
		sendWSMessage_180000(_G(my_walker), 1061);
		break;

	case 1067:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HAND CHIN POS3", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 14, 14, 14, 0, 14, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1068);

		break;

	case 1068:
		kernel_timing_trigger(60, 1069, nullptr);
		break;

	case 1069:
		sendWSMessage_120000(_G(my_walker), 1070);
		break;

	case 1073:
		series_unload(_ripTrekHeadTurnPos1Series);
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));

		break;

	default:
		break;
	}
}

void Room709::clearPressed(void *, void *) {
	static int32 _field84 = 0;
	static int32 _field88 = 0;
	static uint32 _field8C = 0;

	_field8C = timer_read_60();

	if (_field84 == 0) {
		_field88 = timer_read_60();
		++_field84;
	} else if (_field8C - _field88 < 60) {
		_field88 = _field8C;
		++_field84;
	} else {
		_field88 = _field8C;
		_field84 = 1;
	}

	if (_field84 == 3) {
		digi_preload("304_s05", -1);
		digi_preload("709_s99", -1);
		_G(kernel).trigger_mode = KT_DAEMON;
		disable_player_commands_and_fade_init(500);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
