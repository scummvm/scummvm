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

#include "m4/riddle/rooms/section4/room407.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room407::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room407::init() {
	player_set_commands_allowed(false);
	digi_preload("407_s31");
	digi_preload("407_s32");
	_val1 = 357;
	_val2 = 293;

	_waterFlowTube = series_load("water flowing out surg tube");
	_waterFlowFaucet = series_load("WATER FLOWING OUT OF FAUCET");
	_waterFlowStem = series_load("water flws out fauct stem surg");

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (_val5 == 1010) {
			ws_demand_facing(9);
			_ripMedReach = series_load("RIP TREK MED REACH POS3");
			setGlobals1(_ripMedReach, 10, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(-1);
			_drawerPopup = series_place_sprite("407 DRAWER POPUP", 0, 0, 0, 100, 0x200);
			disableHotspots();
			hotspot_set_active(" ", true);

			if (_xyzzy2 == 1101) {
				_drawerPopupHose = series_place_sprite("407 DRAWER POPUP HOSE", 0, 0, 0, 100, 0x100);
				hotspot_set_active("GARDEN HOSE", true);
			}

			if (_xyzzy5 == 1101) {
				_stopperInDrawer = series_place_sprite("407 STOPPER IN DRAWER", 0, 0, 0, 100, 0x100);
				hotspot_set_active("RUBBER PLUG", true);
			}

			if (_xyzzy3 == 1101) {
				_tubeInDrawer = series_place_sprite("407 TUBE IN DRAWER", 0, 0, 0, 100, 0x100);
				hotspot_set_active("SURGICAL TUBE", true);
			}

			if (_xyzzy4 == 1101) {
				_handleInDrawer = series_place_sprite("407 PUMP HANDLE IN DRAWER", 0, 0, 0, 100, 0x100);
				hotspot_set_active("PUMP GRIPS", true);
			}
		} else if (_frotz10 == 1030) {
			_tabletopPopup = series_place_sprite("407 TABLETOP POPUP", 0, 0, 0, 100, 0x200);
			disableHotspots();
			hotspot_set_active(" ", true);

			if (_xyzzy3 == 1117)
				_tabletopPopupWithItems1 = series_place_sprite(
					"407 TABLETOP POPUP WITH ITEMS", 0, 0, 0, 100, 0);

			if (_val9 == 1110)
				_tabletopPopupWithItems1 = series_place_sprite(
					"407 TABLETOP POPUP WITH ITEMS",
					(_val3 == 1010) ? 1 : 2,
					0, 0, 100, 0x100);

			if (_val10 == 1113)
				_tabletopPopupWithItems1 = series_place_sprite(
					"407 TABLETOP POPUP WITH ITEMS", 3, 0, 0, 100, 0x100);

		} else if (_int1 == 1030) {
			_pivotPopup = series_place_sprite("407 PIVOT POPUP", 0, 0, 0, 100, 0x200);

			if (_val10 == 1113)
				_tabletopPopupWithItems3 = series_place_sprite(
					"407 TABLE PIVOT WITH LEVER", 0, 0, 0, 100, 0x200);

			disableHotspots();
			hotspot_set_active(" ", true);

		} else if (_int3 == 1030) {
			_glassTopPopup = series_place_sprite("407 GLASS TOP POPUP", 0, 0, 0, 100, 0x200);

			if (_val6 == 1010)
				_glassTopPopupWithItems1 = series_place_sprite(
					"407 GLASS TOP POPUP WITH ITEMS", 1, 0, 0, 100, 0x100);
			else if (_xyzzy6 == 1116)
				_glassTopPopupWithItems2 = series_place_sprite(
					"407 GLASS TOP POPUP WITH ITEMS", 0, 0, 0, 100, 0x100);

			disableHotspots();
			hotspot_set_active(" ", true);

		} else if (_int4 == 1030) {
			_glassTopPopup = series_place_sprite("407 GLASS BOTTOM POPUP", 0, 0, 0, 100, 0x200);

			if (_xyzzy6 == 1116)
				_glassTopPopupWithItems2 = series_place_sprite(
					"407 GLS BOTTOM PU WITH ITEMS", 0, 0, 0, 100, 0x100);

			if (_xyzzy8 == 1116)
				_glassBottomWithItems1 = series_place_sprite(
					"407 GLS BOTTOM PU WITH ITEMS", 1, 0, 0, 100, 0x100);

			if (_xyzzy5 == 1116)
				_glassBottomWithItems2 = series_place_sprite(
					"407 GLS BOTTOM PU WITH ITEMS", 4, 0, 0, 100, 0x100);

			if (_val10 == 1113)
				_tabletopPopupWithItems3 = series_place_sprite(
					"407 GLS BOTTOM PU WITH ITEMS", 5, 0, 0, 100, 0x100);

			disableHotspots();
			hotspot_set_active(" ", true);

		} else if (_int2 == 1030) {
			_glassTopPopup = series_place_sprite("407 FULL GLASS POPUP",
				0, 0, 0, 100, 0x200);

			if (_xyzzy6 == 1116)
				_glassTopPopupWithItems2 = series_place_sprite(
					"407 FULL GLASS POPUP", 0, 0, 0, 100, 0x100);

			if (_xyzzy8 == 1116)
				_glassBottomWithItems1 = series_place_sprite(
					"407 FULL GLASS POPUP WITH ITEMS", 1, 0, 0, 100, 0x100);

			if (_xyzzy5 == 1116)
				_glassBottomWithItems1 = series_place_sprite(
					"407 FULL GLASS POPUP WITH ITEMS", 4, 0, 0, 100, 0x100);

			disableHotspots();
			hotspot_set_active(" ", true);

		} else if (_int5 == 1030) {
			_roofPiston = series_place_sprite("407 ROOF PISTON/BRACE", 0, 0, 0, 100, 0x200);

			if (_val10 == 1114)
				_tabletopPopupWithItems3 = series_place_sprite(
					"407 ROOF PISTON WITH ITEMS", 1, 0, 0, 100, 0x100);

			if (_xyzzy1 == 1114)
				_roofPistonWithItems = series_place_sprite(
					"407 ROOF PISTON WITH ITEMS", 0, 0, 0, 100, 0);

			disableHotspots();
			hotspot_set_active(" ", true);

		} else if (_int6 == 1030) {
			_glassGone = series_place_sprite("407 GLASS GONE CU PU", 0, 0, 0, 100, 0x200);
			disableHotspots();
			hotspot_set_active(" ", true);

		} else if (_int7 == 1030) {
			_comboLockPopup = series_place_sprite(
				"407 COMBINATION LOCK POPUP", 0, 0, 0, 100, 0x200);
			disableHotspots();
			hotspot_set_active(" ", true);
			_comboLockNumerals = series_load("407 COMBINATION LOCK NUMERALS");
			_407pu08b = series_load("407PU08B");
			_407pu08c = series_load("407PU08C");
			_407pu08d = series_load("407PU08D");

			_lockButton1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "LOCK button 1");
			_lockButton2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "LOCK button 2");
			_lockButton3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "LOCK button 3");
			_lockButton4 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "LOCK button 4");

			sendWSMessage_10000(1, _lockButton1, _comboLockNumerals,
				_buttonFrame1 * 2 + 1, _buttonFrame1 * 2 + 1, -1,
				_comboLockNumerals, _buttonFrame1 * 2 + 1,
				_buttonFrame1 * 2 + 1, 0);
			sendWSMessage_10000(1, _lockButton2, _comboLockNumerals,
				_buttonFrame1 * 2 + 1, _buttonFrame1 * 2 + 1, -1,
				_comboLockNumerals, _buttonFrame1 * 2 + 1,
				_buttonFrame1 * 2 + 1, 0);
			sendWSMessage_10000(1, _lockButton3, _comboLockNumerals,
				_buttonFrame1 * 2 + 1, _buttonFrame1 * 2 + 1, -1,
				_comboLockNumerals, _buttonFrame1 * 2 + 1,
				_buttonFrame1 * 2 + 1, 0);

			hotspot_set_active("BUTTON", true);
			hotspot_set_active("BUTTON ", true);
			hotspot_set_active("BUTTON  ", true);
			hotspot_set_active("BUTTON   ", true);

		} else {
			setHotspots();
		}
	} else {
		static const char *HOTSPOTS[] = {
			"GARDEN HOSE", "RUBBER PLUG", "SURGICAL TUBE",
			"PUMP GRIPS", "PUMP GRIPS ", "JAR/CORK ",
			"JAR/GRIPS ", "JAR/RUBBER PLUG ", "PUMP ROD ",
			"GLASS JAR ", "LEVER KEY  ", "GARDEN HOSE ",
			"SURGICAL TUBE ", "FAUCET PIPE ", "GARDEN HOSE  ",
			"SURGICAL TUBE  ", "FAUCET PIPE  ", "SURGICAL TUBE   ",
			"GARDEN HOSE   ", "GARDEN HOSE    ", "AIR VALVE/HANDLE",
			"LEVER KEY ", "NOZZLES/TUBE", "PERIODIC TABLE/JAR",
			"PERIODIC TABLE ", "JAR/RUBBER PLUG", "JAR/CORK",
			"MICROSCOPE", "BUTTONS", "LETTER", "BUTTON",
			"BUTTON ", "BUTTON  ", "BUTTON   ", "WATER",
			"WATER ", "WATER  ", nullptr
		};
		for (auto *hs = HOTSPOTS; *hs; ++hs)
			hotspot_set_active(*hs, false);

		_val3 = 1010;
		_val4 = 1011;
		_val5 = 1011;
		_val6 = 1011;
		_val7 = 1020;
		_val8 = 1100;
		_val9 = 1100;
		_val10 = 1112;
		_xyzzy1 = 1115;
		_xyzzy2 = 1101;
		_xyzzy3 = 1101;
		_xyzzy4 = 1101;
		_xyzzy5 = 1101;
		_xyzzy6 = 1112;
		_xyzzy7 = 1112;
		_xyzzy8 = 1116;
		_xyzzy9 = 1118;
		_xyzzy10 = 1040;
		_frotz1 = 0;
		_frotz2 = 0;
		_frotz3 = 0;
		_frotz4 = 0;
		_frotz5 = 0;
		_frotz6 = 0;
		_frotz7 = 0;
		_frotz8 = 0;
		_frotz9 = 0;
		_frotz10 = 1031;
		_frotz10 = 1031;
		_int1 = 1031;
		_int2 = 1031;
		_int3 = 1031;
		_int4 = 1031;
		_int5 = 1031;
		_int6 = 1031;
		_int7 = 1031;
		_buttonFrame1 = 0;
		_buttonFrame2 = 0;
		_buttonFrame3 = 0;
		_buttonFrame4 = 0;
		_int14 = 0;

		_bottle = series_place_sprite("407BOTLE", 0, 0, -53, 100, 0xf00);
		_chart = series_place_sprite("407CHART", 0, 0, -53, 100, 0xf00);
		_cpist = series_place_sprite("407CPIST", 0, 0, -53, 100, 0xf00);
		_drawer = series_place_sprite("407DRAWR", 0, 0, -53, 100, 0xf00);
		_escape = series_place_sprite("407ESCAP", 0, 0, -53, 100, 0xd00);
		_faucet1 = series_place_sprite("407FAUC", 0, 0, -53, 100, 0xe00);
		_faucet2 = series_place_sprite("407FAUC", 0, 0, -53, 100, 0xe00);
		_lever = series_place_sprite("407LEVRW", 0, 0, -53, 100, 0xf00);
		_niche = series_place_sprite("407NICH", 0, 0, -53, 100, 0xf00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		_pump = series_place_sprite("407pump", 0, 0, -53, 100, 0xf00);
	}
	// TODO
}

void Room407::daemon() {
	// TODO
}

void Room407::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");
	// TODO
}

void Room407::parser() {
	// TODO
}

void Room407::setHotspots() {
	disableHotspots();

	hotspot_set_active(" ", true);

	if (_val5 == 1010) {
		if (_xyzzy2 == 1101)
			hotspot_set_active("GARDEN HOSE", true);
		if (_xyzzy5 == 1101)
			hotspot_set_active("RUBBER PLUG", true);
		if (_xyzzy3 == 1101)
			hotspot_set_active("SURGICAL TUBE", true);
		if (_xyzzy4 == 1101)
			hotspot_set_active("PUMP GRIPS", true);

	} else if (_frotz10 == 1030 || _int1 == 1030 ||
			_int3 == 1030 || _int4 == 1030 || _int2 == 1030 ||
			_int5 == 1030 || _int6 == 1030) {
		// No implementation
	} else if (_int7 == 1030) {
		hotspot_set_active("BUTTON", true);
		hotspot_set_active("BUTTON ", true);
		hotspot_set_active("BUTTON  ", true);
		hotspot_set_active("BUTTON   ", true);

	} else {
		enableHotspots();

		hotspot_set_active("GARDEN HOSE", false);
		hotspot_set_active("RUBBER PLUG", false);
		hotspot_set_active("SURGICAL TUBE", false);
		hotspot_set_active("PUMP GRIPS", false);
		hotspot_set_active("BUTTON", false);
		hotspot_set_active("BUTTON ", false);
		hotspot_set_active("BUTTON  ", false);
		hotspot_set_active("BUTTON   ", false);

		if (_xyzzy4 != 1115)
			hotspot_set_active("PUMP GRIPS ", false);
		if (_xyzzy1 != 1115)
			hotspot_set_active("PUMP ROD", false);
		if (_xyzzy1 != 1114)
			hotspot_set_active("PUMP ROD ", false);
		if (_val9 != 1100)
			hotspot_set_active("FAUCET HANDLE", false);
		if (_val8 != 1100)
			hotspot_set_active("FAUCET PIPE", false);
		if (_val9 != 1110)
			hotspot_set_active("AIR VALVE/HANDLE", false);
		if (_val9 != 1110)
			hotspot_set_active("AIR VALVE", false);
		if (_val10 != 1112)
			hotspot_set_active("LEVER KEY", false);
		if (_val10 != 1113)
			hotspot_set_active("LEVER KEY ", false);
		if (_val10 != 1114)
			hotspot_set_active("LEVER KEY  ", false);
		if (_val10 != 1117)
			hotspot_set_active("NOZZLES/TUBE", false);
		if (_xyzzy3 != 1117)
			hotspot_set_active("NOZZLES", false);
		if (_xyzzy6 != 1112)
			hotspot_set_active("PERIODIC TABLE", false);
		if (_xyzzy6 != 1116)
			hotspot_set_active("PERIODIC TABLE/JAR", false);
		if (_xyzzy6 != 1120)
			hotspot_set_active("PERIODIC TABLE ", false);

		if (inv_object_is_here("EMERALD/CORK"))
			hotspot_set_active("EMERALD/CORK", false);

		if (_xyzzy7 == 1112) {
			if (_xyzzy5 != 1116)
				hotspot_set_active("JAR/RUBBER PLUG", false);
			if (_xyzzy9 != 1116)
				hotspot_set_active("JAR/CORK", false);
			if (_xyzzy8 != 1116)
				hotspot_set_active("EMERALD PIN/CORK", false);
		} else {
			hotspot_set_active("GLASS JAR", false);
			hotspot_set_active("JAR/RUBBER PLUG", false);
			hotspot_set_active("JAR/CORK", false);
			hotspot_set_active("EMERALD PIN/CORK", false);
		}

		if (_xyzzy7 != 1114 && _xyzzy7 != 1140) {
			hotspot_set_active("JAR/CORK ", false);
			hotspot_set_active("JAR/GRIPS ", false);
			hotspot_set_active("GLASS JAR ", false);
			hotspot_set_active("JAR/RUBBER PLUG ", false);
		} else {
			if (_xyzzy5 != 1116 && _xyzzy5 != 1130)
				hotspot_set_active("JAR/RUBBER PLUG ", false);
			if (_xyzzy9 != 1116 && _xyzzy9 != 1130)
				hotspot_set_active("JAR/CORK ", false);
			if (_xyzzy4 != 1116 && _xyzzy4 != 1130)
				hotspot_set_active("JAR/GRIP ", false);
		}

		if (_val8 != 1116 && _val8 != 1140)
			hotspot_set_active("FAUCET PIPE ", false);
		if (_xyzzy2 != 1116)
			hotspot_set_active("GARDEN HOSE ", false);
		if (_xyzzy3 != 1116)
			hotspot_set_active("SURGICAL TUBE ", false);
		if (_xyzzy2 != 1130)
			hotspot_set_active("GARDEN HOSE  ", false);
		if (_val8 != 1130)
			hotspot_set_active("FAUCET PIPE  ", false);
		if (_xyzzy3 != 1130 && _xyzzy3 != 1140)
			hotspot_set_active("SURGICAL TUBE  ", false);
		if (_xyzzy3 != 1100)
			hotspot_set_active("SURGICAL TUBE   ", false);
		if (_xyzzy2 != 1140)
			hotspot_set_active("GARDEN HOSE   ", false);
		if (_xyzzy2 != 1100)
			hotspot_set_active("GARDEN HOSE    ", false);

		if (_frotz2) {
			hotspot_set_active("ARMORED CABINET", false);

			if (_frotz4)
				hotspot_set_active("LETTER", false);
			if (!_frotz4)
				hotspot_set_active("BUTTONS", false);

		} else {
			hotspot_set_active("MICROSCOPE", false);
			hotspot_set_active("BUTTONS", false);
			hotspot_set_active("LETTER", false);
		}

		hotspot_set_active("WATER", false);
		hotspot_set_active("WATER ", false);
		hotspot_set_active("WATER  ", false);

		if (!_val4) {
			if (_val8 == 1100) {
				if (_xyzzy3 != 1100)
					hotspot_set_active("WATER", true);
				else if (_xyzzy2 != 1100)
					hotspot_set_active("WATER ", true);
			} else {
				if (_xyzzy3 == 1130 && _xyzzy2 != 1130)
					hotspot_set_active("WATER  ", true);
			}
		}
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
