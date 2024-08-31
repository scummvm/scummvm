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

static const char *const SAID[][2] = {
	{ "WATER",       "407R99M" },
	{ "WATER ",      "407R99M" },
	{ "WATER  ",     "407R99M" },
	{ "BUTTONS",     "407R99G" },
	{ "BUTTON",      "407R99G" },
	{ "BUTTON ",     "407R99G" },
	{ "BUTTON  ",    "407R99G" },
	{ "BUTTON   ",   "407R99G" },
	{ "HIDDEN DOOR", "407R02" },
	{ "WORK TABLE",  "407R03" },
	{ "DRAWER",      "407R04" },
	{ "SINK",        "407R05" },
	{ "PISTON ON TABLE", "407R11" },
	{ "PUMP",        "407R12a" },
	{ "COMPRESSED AIR TANK", "407R14" },
	{ "LARGE GEAR",  "407R20" },
	{ "ARMORED CABINET", "407R23" },
	{ "METAL DOOR",  "407R27" },
	{ "SHELVES",     "407R29" },
	{ "SURGICAL TUBE ", "407R86" },
	{ "SURGICAL TUBE  ", "407R90" },
	{ "SURGICAL TUBE   ", "407R31" },
	{ "GARDEN HOSE ",  "407R85" },
	{ "GARDEN HOSE  ", "407R91" },
	{ "GARDEN HOSE   ", "407R87" },
	{ "GLASS JAR ",    "407R15A" },
	{ "FAUCET PIPE  ", "407R92" },
	{ "FAUCET PIPE   ", "407R07" },
	{ "FAUCET STEM",    "407R07" },
	{ "JAR/RUBBER PLUG ", "407R77" },
	{ "JAR/CORK ",        "407R78" },
	{ "JAR/CORK/PLUG ",   "407R79" },
	{ "JAR/GRIPS ",       "407R80" },
	{ "JAR/PLUG/GRIPS ",  "407R81" },
	{ "JAR/PLUG/CORK/GRIPS ", "407R83" },
	{ "FAUCET PIPE/HOSE ", "407R94" },
	{ "FAUCET PIPE/HOSE/TUBE ", "407R95" },
	{ "MICROSCOPE",        "407R60" },
	{ "PERIODIC TABLE ",   "407R99C" },
	{ " ",                 "407R01" },
	{ nullptr, nullptr }
};

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
			_407pu08a = series_load("407 COMBINATION LOCK NUMERALS");
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

			sendWSMessage_10000(1, _lockButton1, _407pu08a,
				_buttonFrame1 * 2 + 1, _buttonFrame1 * 2 + 1, -1,
				_407pu08a, _buttonFrame1 * 2 + 1,
				_buttonFrame1 * 2 + 1, 0);
			sendWSMessage_10000(1, _lockButton2, _407pu08a,
				_buttonFrame1 * 2 + 1, _buttonFrame1 * 2 + 1, -1,
				_407pu08a, _buttonFrame1 * 2 + 1,
				_buttonFrame1 * 2 + 1, 0);
			sendWSMessage_10000(1, _lockButton3, _407pu08a,
				_buttonFrame1 * 2 + 1, _buttonFrame1 * 2 + 1, -1,
				_407pu08a, _buttonFrame1 * 2 + 1,
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
		_codeCorrect = false;

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

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		_drawer = series_place_sprite("407DRAWR", 0, 0, -53, 100, 0xf00);
		_escape = series_place_sprite("407ESCAP", 0, 0, -53, 100, 0xd00);
		_star2 = series_place_sprite("407STAR2", 0, 0, -53, 100, 0xf00);

		if (_val4 == 1010) {
			if (_val8 == 1100) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 410);
				_G(kernel).trigger_mode = KT_PARSE;
			} else if (_xyzzy3 == 1130) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 410);
				_G(kernel).trigger_mode = KT_PARSE;
			}
		}

		if (_val6 == 1010)
			_gears = series_place_sprite("407GEARS", 0, 0, -53, 100, 0xf00);

		if (_frotz2 != 0) {
			_407h = series_load("407H");
			_niche = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xb00, 0,
				triggerMachineByHashCallbackNegative, "Jar opens cabinet door");
			sendWSMessage_10000(1, _niche, _407h, 29, 29, -1, _407h, 29, 29, 0);

			if (_frotz4 == 0)
				_letter = series_place_sprite("407 letter", 0, 0, 0, 100, 0xc00);

		} else {
			_cpist = series_place_sprite("407CPIST", 0, 0, -53, 100, 0xf00);
			_niche = series_place_sprite("407NICH", 0, 0, -53, 100, 0xf00);

			if (_xyzzy9 == 1116 || _xyzzy9 == 1130) {
				_bits = series_place_sprite("407BITSR", 3, 0, -53, 100, 0xa00);
			} else if (_xyzzy7 == 1112) {
				_bits = series_place_sprite("407bbits", 0, 0, -53, 100, 0xa00);
			}

			switch (_xyzzy3) {
			case 1100:
				_tubeInDrawer = series_place_sprite("407 TUBING BY ITSELF", 1, 0, 0, 100, 0xa00);
				break;
			case 1116:
				_tubeInDrawer = series_place_sprite("407 HOSE HANG FROM JAR", 1, 0, 0, 100, 0xa00);
				break;
			case 1117:
				_tubeInDrawer = series_place_sprite("407surgn", 0, 0, -53, 100, 0xa00);
				break;
			case 1130:
				_tubeInDrawer = series_place_sprite("407 TUBING BY ITSELF", 0, 0, 0, 100, 0xa00);
				break;
			default:
				break;
			}

			switch (_xyzzy5) {
			case 1112:
				_stopperInDrawer = series_place_sprite("407bbits", 2, 0, -53, 100, 0xa00);
				break;
			case 1116:
			case 1130:
				_stopperInDrawer = series_place_sprite("407BITSR", 0, 0, -53, 100, 0xa00);
				break;
			default:
				break;
			}

			switch (_xyzzy1) {
			case 1114:
				_pump = series_place_sprite("407PMROD", 0, 0, -53, 100, 0xa00);
				break;
			case 1115:
				_pump = series_place_sprite("407pump", 0, 0, -53, 100, 0xf00);
				break;
			default:
				break;
			}

			switch (_xyzzy4) {
			case 1115:
				_handleInDrawer = series_place_sprite("407pump", 1, 0, -53, 100, 0xa00);
				break;
			case 1116:
			case 1130:
				if (_xyzzy7 == 1114 || _xyzzy7 == 1140)
					_handleInDrawer = series_place_sprite("407BITSR", 2, 0, -53, 100, 0xa00);
				break;
			default:
				break;
			}

			switch (_val10) {
			case 1112:
				_lever = series_place_sprite("407LEVRW", 0, 0, -53, 100, 0xa00);
				break;
			case 1113:
				_lever = series_place_sprite("407LEVRW", 0, 0, -53, 100, 0xb00);
				break;
			case 1114:
				if (!_frotz2)
					_lever = series_place_sprite("407LEVRW", 1, 0, -53, 100, 0xa00);
				break;
			default:
				break;
			}

			switch (_xyzzy7) {
			case 1112:
				if (!inv_object_is_here("EMERALD/CORK"))
					_bottle = series_place_sprite("sprite ofempty bottle", 0, 0, -53, 100, 0xf00);
				else
					_bottle = series_place_sprite((_val6 == 1010) ? "407BOTL2" : "407BOTLE",
						0, 0, -53, 100, 0xf00);
				break;
			case 1114:
			case 1140:
				_bottle = series_place_sprite("407BOTLR", 0, 0, -53, 100, 0xa00);
				break;
			default:
				break;
			}

			switch (_xyzzy2) {
			case 1100:
				_drawerPopupHose = series_place_sprite(
					"407 TUBE AND HOSE INTO SINK", 1, 0, 0, 100, 0xb00);
				break;
			case 1116:
				_drawerPopupHose = series_place_sprite(
					"407 HOSE HANG FROM JAR", 0, 0, 0, 100, 0xb00);
				break;
			case 1130:
				_drawerPopupHose = series_place_sprite(
					"407 TUBE AND HOSE INTO SINK", 0, 0, 0, 100, 0xb00);
				break;
			case 1140:
				_drawerPopupHose = series_place_sprite(
					"407 TUBE AND HOSE TO JAR", 0, 0, 0, 100, 0xb00);
				break;
			default:
				break;
			}

			switch (_val8) {
			case 1100:
				_faucet1 = series_place_sprite("407FAUC", 2, 0, 0, 100, 0xb00);
				break;
			case 1116:
			case 1140:
				_faucet1 = series_place_sprite("407BITSR", 1, 0, -53, 100, 0xb00);
				break;
			case 1130:
				_faucet1 = series_place_sprite("407 FAUCET IN SINK", 0, 0, 0, 100, 0xb00);
				break;
			default:
				break;
			}
		}

		switch (_xyzzy6) {
		case 1112:
			_chart = series_place_sprite("407CHART", 0, 0, -53, 100, 0xf00);
			break;
		case 1116:
			_chart = series_place_sprite("407bbits", 0, 0, -53, 100, 0xa00);
			break;
		case 1120:
			_chart = series_place_sprite("407chrt2", 0, 0, -53, 100, 0xa00);
			break;
		default:
			break;
		}

		switch (_xyzzy9) {
		case 1011:
			_faucet2 = series_place_sprite("407FAUC",
				(_val3 == 1011) ? 1 : 0, 0, 0, 100, 0xe00);
			break;
		case 1110:
			_faucet2 = series_place_sprite("407 AIR VALVE HANDLE",
				(_val3 == 1011) ? 1 : 0, 0, 0, 100, 0xe00);
			break;
		default:
			break;
		}

		player_set_commands_allowed(true);

	} else if (!_G(kittyScreaming)) {
		midi_play("DRAMA1", 255, 0, -1, 949);
		_ripEnters = series_load("407 RIP ENTERS");
		_stair = series_load("407STAIR");
		ws_demand_location(250, 331, 3);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "RIP enters");
		sendWSMessage_10000(1, _ripley, _ripEnters, 1, 27, 400,
			_ripEnters, 27, 27, 0);

	} else {
		terminateMachineAndNull(_faucet1);
		terminateMachineAndNull(_tubeInDrawer);
		terminateMachineAndNull(_drawerPopupHose);
		terminateMachineAndNull(_bottle);
		terminateMachineAndNull(_stopperInDrawer);
		terminateMachineAndNull(_bits);
		terminateMachineAndNull(_handleInDrawer);
		terminateMachineAndNull(_niche);
		terminateMachineAndNull(_lever);

		_407h = series_load("407H");
		_niche = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xb00, 0,
			triggerMachineByHashCallbackNegative, "Jar opens cabinet door");
		sendWSMessage_10000(1, _niche, _407h, 29, 29, -1,
			_407h, 29, 29, 0);
		inv_give_to_player("EMERALD/CORK");

		_xyzzy8 = 1000;
		_frotz2 = 1;
		_val8 = 1140;
		_xyzzy2 = 1140;
		_xyzzy3 = 1140;
		_xyzzy7 = 1140;
		_xyzzy5 = 1130;
		_xyzzy9 = 1130;
		_xyzzy4 = 1130;

		setHotspots();
		ws_demand_location(260, 335, 3);
	}
}

void Room407::daemon() {
	switch (_G(kernel).trigger) {
	case 10:
		player_set_commands_allowed(false);
		digi_play("407_s02", 2);
		terminateMachineAndNull(_drawerPopup);

		if (_xyzzy2 == 1101)
			terminateMachineAndNull(_drawerPopupHose);
		if (_xyzzy5 == 1101)
			terminateMachineAndNull(_stopperInDrawer);
		if (_xyzzy3 == 1101)
			terminateMachineAndNull(_tubeInDrawer);
		if (_xyzzy4 == 1101)
			terminateMachineAndNull(_handleInDrawer);

		sendWSMessage_120000(12);
		break;

	case 12:
		setHotspots();
		sendWSMessage_150000(13);
		break;

	case 13:
		series_unload(_ripMedReach);
		player_set_commands_allowed(true);
		break;

	case 20:
		if (_val10 != 1113) {
			_407tpis2 = series_load("407tpis2");
			terminateMachineAndNull(_tpist);
			_tpist = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
				triggerMachineByHashCallbackNegative, "PISTON ONLY");
			sendWSMessage_10000(1, _tpist, _407tpis2, 1, 33, 40,
				_407tpis2, 33, 33, 0);
			digi_play("407_s18", 2);
		} else if (_xyzzy7 != 1112) {
			kernel_timing_trigger(1, 90);
		} else if (!inv_object_is_here("EMERALD/CORK")) {
			kernel_timing_trigger(1, 50);
		} else if (_xyzzy6 != 1116) {
			kernel_timing_trigger(1, (_xyzzy5 == 1116) ? 70 : 50);
		} else {
			kernel_timing_trigger(1, (_xyzzy5 == 1116) ? 80 : 60);
		}
		break;

	case 40:
		terminateMachineAndNull(_tpist);
		series_unload(_407tpis2);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);

		if (_xyzzy7 != 1112)
			kernel_timing_trigger(1, 180);
		else if (!inv_object_is_here("EMERALD/CORK"))
			kernel_timing_trigger(1, 185);
		else if (_xyzzy6 != 1116)
			kernel_timing_trigger(1, (_xyzzy5 == 1116) ? 170 : 150);
		else
			kernel_timing_trigger(1, (_xyzzy5 == 1116) ? 175 : 160);
		break;

	case 50:
		_407j = series_load("407j");
		terminateMachineAndNull(_lever);
		terminateMachineAndNull(_tpist);

		_tpist = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "PISTON & LEVER");
		sendWSMessage_10000(1, _tpist, _407j, 1, 33, 52,
			_407j, 33, 33, 0);
		digi_play("407_s18", 2);
		break;

	case 52:
		terminateMachineAndNull(_tpist);
		series_unload(_407j);
		_lever = series_place_sprite("407LEVRW", 2, 0, -53, 100, 0xb00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);

		kernel_timing_trigger(1, inv_object_is_here("EMERALD/CORK") ? 150 : 185);
		break;

	case 60:
		_407o = series_load("407o");
		_407k = series_load("407k");
		terminateMachineAndNull(_lever);
		terminateMachineAndNull(_tpist);

		_tpist = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "PISTON & LEVER");
		sendWSMessage_10000(1, _tpist, _407k, 1, 33, 61, _407k, 33, 33, 0);
		digi_play("407_s18", 2);
		break;

	case 61:
		digi_stop(2);
		terminateMachineAndNull(_tpist);
		series_unload(_407k);

		_lever = series_place_sprite("407LEVRW", 2, 0, -53, 100, 0xb00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		terminateMachineAndNull(_bottle);
		terminateMachineAndNull(_chart);

		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR SHAKES");
		sendWSMessage_10000(1, _bottle, _407o, 1, 51, 62,
			_407o, 51, 51, 0);
		digi_play("407_s16", 3);
		break;

	case 62:
		digi_play("407_s19", 1);
		sendWSMessage_10000(1, _bottle, _407o, 52, 66, 63, _407o, 66, 66, 0);
		break;

	case 63:
		digi_play("407_s28", 1);
		sendWSMessage_10000(1, _bottle, _407o, 67, 91, 64, _407o, 91, 91, 0);
		break;

	case 64:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("sprite ofempty bottle", 0, 0, -53, 100, 0xf00);
		_chart = series_place_sprite("407chrt2", 0, 0, -53, 100, 0xf00);

		series_unload(_407o);
		kernel_timing_trigger(1, 190);
		break;

	case 70:
		_407l = series_load("407l");
		terminateMachineAndNull(_lever);
		terminateMachineAndNull(_tpist);

		_tpist = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "PISTON & LEVER");
		sendWSMessage_10000(1, _tpist, _407l, 1, 33, 72, _407l, 33, 33, 0);
		digi_play("407_s18", 2);
		break;

	case 72:
		terminateMachineAndNull(_tpist);
		series_unload(_407l);

		_lever = series_place_sprite("407LEVRW", 2, 0, -53, 100, 0xb00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		kernel_timing_trigger(1, 170);
		break;

	case 80:
		_407p = series_load("407p");
		_407m = series_load("407m");

		terminateMachineAndNull(_lever);
		terminateMachineAndNull(_tpist);
		_tpist = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "PISTON & LEVER");
		sendWSMessage_10000(1, _tpist, _407m, 1, 33, 81, _407m, 33, 33, 0);
		digi_play("407_s18", 2);
		break;

	case 81:
		digi_stop(2);
		terminateMachineAndNull(_tpist);

		_lever = series_place_sprite("407LEVRW", 2, 0, -53, 100, 0xb00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		terminateMachineAndNull(_stopperInDrawer);
		terminateMachineAndNull(_bottle);
		terminateMachineAndNull(_chart);

		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR TURNS");
		sendWSMessage_10000(1, _bottle, _407p, 1, 51, 82, _407p, 51, 51, 0);
		break;

	case 82:
		digi_play("407_s19", 1);
		sendWSMessage_10000(1, _bottle, _407p, 52, 66, 83, _407p, 66, 66, 0);
		break;

	case 83:
		sendWSMessage_10000(1, _bottle, _407p, 67, 91, 84, _407p, 91, 91, 0);
		break;

	case 84:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("sprite ofempty bottle", 0, 0, -53, 100, 0xf00);
		_stopperInDrawer = series_place_sprite("407bbits", 2, 0, -53, 100, 0xe00);
		_chart = series_place_sprite("407chrt2", 0, 0, -53, 100, 0xf00);

		series_unload(_407p);
		kernel_timing_trigger(1, 190);
		break;

	case 90:
		_407q = series_load("407q");
		terminateMachineAndNull(_lever);
		terminateMachineAndNull(_tpist);

		_tpist = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "PISTON & LEVER");
		sendWSMessage_10000(1, _tpist, _407q, 1, 33, 92, _407q, 33, 33, 0);
		digi_play("407_s18", 2);
		break;

	case 92:
		terminateMachineAndNull(_tpist);
		series_unload(_407q);
		_lever = series_place_sprite("407LEVRW", 2, 0, -53, 100, 0xb00);
		_tpist = series_place_sprite("407TPIST", 0, 0, -53, 100, 0xf00);
		kernel_timing_trigger(1, 180);
		break;

	case 150:
		digi_stop(2);
		terminateMachineAndNull(_bottle);

		_407a = series_load("407a");
		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR TURNS");
		sendWSMessage_10000(1, _bottle, _407a, 1, 30, 151, _407a, 30, 30, 0);
		digi_play("407_s16", 3);

		if (inv_object_is_here("EMERALD/CORK"))
			kernel_timing_trigger(240, 450);
		break;

	case 151:
		sendWSMessage_10000(1, _bottle, _407a, 31, 61, 152, _407a, 61, 61, 0);
		digi_play("407_s17", 3);
		break;

	case 152:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("407BOTLE", 0, 0, -53, 100, 0xf00);
		series_unload(_407a);
		player_set_commands_allowed(true);
		break;

	case 160:
		digi_stop(2);
		terminateMachineAndNull(_bottle);
		terminateMachineAndNull(_chart);

		_407b = series_load("407b");
		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR TURNS");
		sendWSMessage_10000(1, _bottle, _407b, 1, 30, 161, _407b, 30, 30, 0);
		digi_play("407_s16", 3);
		kernel_timing_trigger(240, 450);
		break;

	case 161:
		sendWSMessage_10000(1, _bottle, _407b, 31, 61, 162, _407b, 61, 61, 0);
		digi_play("407_s17", 3);
		break;

	case 162:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("407BOTLE", 0, 0, -53, 100, 0xf00);
		_chart = series_place_sprite("407bbits", 1, 0, -53, 100, 0xe00);
		series_unload(_407b);
		player_set_commands_allowed(true);
		break;

	case 170:
		digi_stop(2);
		terminateMachineAndNull(_bottle);
		terminateMachineAndNull(_stopperInDrawer);

		_407c = series_load("407c");
		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR TURNS");
		sendWSMessage_10000(1, _bottle, _407c, 1, 30, 171, _407c, 30, 30, 0);
		digi_play("407_s16", 3);
		kernel_timing_trigger(240, 450);
		break;

	case 171:
		sendWSMessage_10000(1, _bottle, _407c, 31, 61, 172, _407c, 61, 61, 0);
		digi_play("407_s17", 3);
		break;

	case 172:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("407BOTLE", 0, 0, -53, 100, 0xf00);
		_stopperInDrawer = series_place_sprite("407bbits", 2, 0, -53, 100, 0xe00);
		series_unload(_407c);
		player_set_commands_allowed(true);
		break;

	case 175:
		digi_stop(2);
		terminateMachineAndNull(_stopperInDrawer);
		terminateMachineAndNull(_bottle);
		terminateMachineAndNull(_chart);

		_407d = series_load("407d");
		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR TURNS");
		sendWSMessage_10000(1, _bottle, _407d, 1, 30, 176, _407d, 30, 30, 0);
		digi_play("407_s16", 3);
		kernel_timing_trigger(240, 450);
		break;

	case 176:
		sendWSMessage_10000(1, _bottle, _407d, 31, 61, 177, _407d, 61, 61, 0);
		digi_play("407_s17", 3);
		break;

	case 177:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("407BOTLE", 0, 0, -53, 100, 0xf00);
		_stopperInDrawer = series_place_sprite("407bbits", 2, 0, -53, 100, 0xe00);
		_chart = series_place_sprite("407bbits", 1, 0, -53, 100, 0xe00);
		series_unload(_407d);
		player_set_commands_allowed(true);
		break;

	case 180:
		digi_stop(2);
		terminateMachineAndNull(_gears);

		_407s = series_load("407s");
		_gears = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GEARS TURN");
		sendWSMessage_10000(1, _bottle, _407s, 1, 30, 181, _407s, 30, 30, 0);
		digi_play("407_s16", 3);
		break;

	case 181:
		sendWSMessage_10000(1, _gears, _407s, 31, 60, 182, _407s, 60, 60, 0);
		digi_play("407_s17", 3);
		break;

	case 182:
		terminateMachineAndNull(_gears);
		_gears = series_place_sprite("407GEARS", 0, 0, -53, 100, 0xf00);
		series_unload(_407s);
		player_set_commands_allowed(true);
		break;

	case 185:
		digi_stop(2);
		terminateMachineAndNull(_bottle);

		_407r = series_load("407r");
		_bottle = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "GLASS JAR TURNS");
		sendWSMessage_10000(1, _bottle, _407r, 1, 30, 186, _407r, 30, 30, 0);
		digi_play("407_s16", 3);
		break;

	case 186:
		sendWSMessage_10000(1, _bottle, _407r, 31, 61, 187, _407r, 61, 61, 0);
		digi_play("407_s17", 3);
		break;

	case 187:
		terminateMachineAndNull(_bottle);
		_bottle = series_place_sprite("sprite ofempty bottle", 0, 0, -53, 100, 0xf00);
		series_unload(_407r);
		player_set_commands_allowed(true);
		break;

	case 190:
		ws_walk(155, 335, nullptr, 192, 11);
		break;

	case 192:
		_ripMedHand1 = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_ripMedHand1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(193);
		break;

	case 193:
		inv_give_to_player("EMERALD/CORK");
		hotspot_set_active("EMERALD/CORK", false);
		hotspot_set_active("PERIODIC TABLE/JAR", false);
		hotspot_set_active("PERIODIC TABLE ", true);
		_xyzzy6 = 1120;

		midi_play("EMERALD", 255, 0, 194, 949);
		kernel_examine_inventory_object("PING EMERALD/CORK",
			_G(master_palette), 5, 1, 50, 200, 195, nullptr, -1);
		break;

	case 194:
		digi_play("407r32", 1);
		break;

	case 195:
		sendWSMessage_120000(196);
		break;

	case 196:
		sendWSMessage_150000(197);
		break;

	case 197:
		series_unload(_ripMedHand1);
		_xyzzy8 = 1000;
		player_set_commands_allowed(true);
		break;

	case 200:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_tabletopPopup);

		if (_xyzzy3 == 1117)
			terminateMachineAndNull(_tabletopPopupWithItems1);
		if (_val9 == 1110)
			terminateMachineAndNull(_tabletopPopupWithItems2);
		if (_val10 == 1113)
			terminateMachineAndNull(_tabletopPopupWithItems3);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 202:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_pivotPopup);

		if (_val10 == 1113)
			terminateMachineAndNull(_tabletopPopupWithItems3);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 204:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_glassTopPopup);

		if (_xyzzy6 == 1116)
			terminateMachineAndNull(_tabletopPopupWithItems2);
		if (_val6 == 1010)
			terminateMachineAndNull(_tabletopPopupWithItems1);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 206:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_glassTopPopup);

		if (_xyzzy6 == 1116)
			terminateMachineAndNull(_tabletopPopupWithItems2);
		if (_xyzzy8 == 1116)
			terminateMachineAndNull(_glassBottomWithItems1);
		if (_xyzzy5 == 1116)
			terminateMachineAndNull(_glassBottomWithItems2);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 208:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_glassTopPopup);

		if (_xyzzy6 == 1116)
			terminateMachineAndNull(_glassTopPopupWithItems2);
		if (_xyzzy8 == 1116)
			terminateMachineAndNull(_glassBottomWithItems1);
		if (_xyzzy5 == 1116)
			terminateMachineAndNull(_glassBottomWithItems2);
		if (_val6 == 1010)
			terminateMachineAndNull(_glassTopPopupWithItems1);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 210:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_roofPiston);

		if (_val10 == 1114)
			terminateMachineAndNull(_tabletopPopupWithItems3);
		if (_xyzzy1 == 1114)
			terminateMachineAndNull(_roofPistonWithItems);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 212:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_glassGone);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 214:
		player_set_commands_allowed(false);
		terminateMachineAndNull(_comboLockPopup);
		terminateMachineAndNull(_lockButton1);
		terminateMachineAndNull(_lockButton2);
		terminateMachineAndNull(_lockButton3);
		terminateMachineAndNull(_lockButton4);
		series_unload(_407pu08d);
		series_unload(_407pu08c);
		series_unload(_407pu08b);
		series_unload(_407pu08a);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 216:
		terminateMachineAndNull(_glassTopPopupWithItems2);
		setHotspots();
		interface_show();
		player_set_commands_allowed(true);
		break;

	case 218:
		player_set_commands_allowed(false);
		digi_stop(1);
		terminateMachineAndNull(_microscopeCloseup);

		setHotspots();
		player_set_commands_allowed(true);
		break;

	case 300:
		ws_walk(441, 331, nullptr, 302, 1);
		break;

	case 302:
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(1, _ripHiHand1, 12, 12, 12, 0, 12, 1, 1, 1);
		sendWSMessage_110000(303);
		break;

	case 303:
		terminateMachineAndNull(_tubeInDrawer);
		terminateMachineAndNull(_drawerPopupHose);
		terminateMachineAndNull(_faucet1);

		hotspot_set_active("GARDEN HOSE ", false);
		hotspot_set_active("GARDEN HOSE  ", false);
		hotspot_set_active("FAUCET PIPE  ", false);
		hotspot_set_active("FAUCET PIPE ", true);
		hotspot_set_active("SURGICAL TUBE  ", true);
		hotspot_set_active("SURGICAL TUBE ", false);
		hotspot_set_active("GARDEN HOSE   ", true);

		_val8 = 1140;
		_xyzzy2 = 1140;
		_xyzzy3 = 1140;
		_xyzzy7 = 1140;
		_drawerPopupHose = series_place_sprite("407 TUBE AND HOSE TO JAR",
			0, 0, 0, 100, 0xb00);
		_faucet1 = series_place_sprite("407BITSR", 1, 0, -53, 100, 0xb00);
		sendWSMessage_120000(304);
		break;

	case 304:
		sendWSMessage_150000(305);
		break;

	case 305:
		series_unload(_ripHiHand1);
		_frotz1 = 1;
		player_set_commands_allowed(true);
		break;

	case 310:
		ws_walk(441, 331, nullptr, 312, 1);
		break;

	case 312:
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 12, 12, 12, 0, 12, 1, 1, 1);
		sendWSMessage_110000(313);
		break;

	case 313:
		terminateMachineAndNull(_tubeInDrawer);
		terminateMachineAndNull(_drawerPopupHose);
		terminateMachineAndNull(_faucet1);

		hotspot_set_active("GARDEN HOSE ", false);
		hotspot_set_active("GARDEN HOSE  ", false);
		hotspot_set_active("FAUCET PIPE  ", false);
		hotspot_set_active("FAUCET PIPE ", true);
		hotspot_set_active("SURGICAL TUBE  ", true);
		hotspot_set_active("SURGICAL TUBE ", false);
		hotspot_set_active("GARDEN HOSE   ", true);

		_val8 = 1140;
		_xyzzy2 = 1140;
		_xyzzy3 = 1140;
		_xyzzy7 = 1140;

		sendWSMessage_120000(314);
		break;

	case 314:
		sendWSMessage_150000(315);
		break;

	case 315:
		series_unload(_ripHiHand1);
		ws_walk(155, 335, nullptr, 316, 11);
		break;

	case 316:
		_ripMedHand1 = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_ripMedHand1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(317);
		break;

	case 317:
		_drawerPopupHose = series_place_sprite("407 TUBE AND HOSE TO JAR",
			0, 0, 0, 100, 0xb00);
		_faucet1 = series_place_sprite("407BITSR", 1, 0, -53, 100, 0xb00);
		sendWSMessage_120000(318);
		break;

	case 318:
		sendWSMessage_150000(319);
		break;

	case 319:
		series_unload(_ripMedHand1);
		_frotz1 = 1;
		player_set_commands_allowed(true);
		break;

	case 320:
		_ripHeadTurn = series_load("RIP TREK HEAD TURN POS1");
		setGlobals1(_ripHeadTurn, 1, 3, 3, 3, 0, 3, 1, 1, 1);
		sendWSMessage_110000(322);
		break;

	case 322:
		if (_xyzzy5 == 1116 && _xyzzy9 == 1116 && _xyzzy4 == 1116) {
			terminateMachineAndNull(_faucet1);
			terminateMachineAndNull(_tubeInDrawer);
			terminateMachineAndNull(_drawerPopupHose);
			terminateMachineAndNull(_bottle);
			terminateMachineAndNull(_stopperInDrawer);
			terminateMachineAndNull(_bits);
			terminateMachineAndNull(_handleInDrawer);
			terminateMachineAndNull(_niche);
			terminateMachineAndNull(_lever);

			_letter = series_place_sprite("407 letter", 0, 0, 0, 100, 0xc00);
			_407h = series_load("407H");
			_niche = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xb00, 0,
				triggerMachineByHashCallbackNegative, "Jar opens cabinet door");
			sendWSMessage_10000(1, _niche, _407h, 1, 29, 330, _407h, 29, 29, 0);
			digi_play("407_s21", 2);
		} else {
			if (_xyzzy5 == 1116) {
				_jarWaterFallingLeft = series_load("407 JAR WATER FALLING LEFT");
				_jarLeft = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
					triggerMachineByHashCallbackNegative, "407 JAR WATER FALLING LEFT");
				sendWSMessage_10000(1, _jarLeft, _jarWaterFallingLeft, 1, 13, 323,
					_jarWaterFallingLeft, 14, 19, 0);
			}

			if (_xyzzy9 != 1116) {
				_jarWaterFallingCentre = series_load("407 JAR WATER FALLING CENTRE");
				_jarCentre = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
					triggerMachineByHashCallbackNegative, "Center jar hole leaks");
				sendWSMessage_10000(1, _jarCentre, _jarWaterFallingCentre, 1, 11, 323,
					_jarWaterFallingCentre, 12, 15, 0);
			}

			if (_xyzzy4 != 1116) {
				_jarWaterFallingRight = series_load("407 JAR WATER FALLING RIGHT");
				_jarRight = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
					triggerMachineByHashCallbackNegative, "Right jar hole leaks");
				sendWSMessage_10000(1, _jarRight, _jarWaterFallingRight, 1, 13, 323,
					_jarWaterFallingRight, 14, 20, 0);
			}

			kernel_timing_trigger(200, 340);
		}
		break;

	case 323:
		digi_play_loop("407_s10", 2);
		break;

	case 330:
		digi_preload("407r37");
		digi_play("407r37", 1, 255, 666);
		_frotz2 = 1;
		_xyzzy5 = 1130;
		_xyzzy9 = 1130;
		_xyzzy4 = 1130;

		hotspot_set_active("MICROSCOPE", true);
		hotspot_set_active("LETTER", true);
		hotspot_set_active("ARMORED CABINET", false);
		kernel_timing_trigger(1, 340);
		break;

	case 340:
		sendWSMessage_120000(342);
		break;

	case 342:
		sendWSMessage_150000(343);
		break;

	case 343:
		series_unload(_ripHeadTurn);
		_ripMedHand1 = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_ripMedHand1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(344);
		break;

	case 344:
		if (!_frotz2) {
			if (_xyzzy5 != 1115)
				sendWSMessage_10000(1, _jarLeft, _jarWaterFallingLeft, 20, 35, 350,
					_jarWaterFallingLeft, 35, 35, 0);
			if (_xyzzy9 != 1116)
				sendWSMessage_10000(1, _jarCentre, _jarWaterFallingCentre, 16, 28, 351,
					_jarWaterFallingCentre, 28, 28, 0);
			if (_xyzzy4 != 1116)
				sendWSMessage_10000(1, _jarRight, _jarWaterFallingRight, 21, 35, 352,
					_jarWaterFallingRight, 35, 35, 0);
		}

		terminateMachineAndNull(_faucet2);
		_val4 = 1011;
		_faucet2 = series_place_sprite("407FAUC", 1, 0, 0, 100, 0xe00);
		sendWSMessage_120000(345);
		break;

	case 345:
		sendWSMessage_150000(346);
		break;

	case 346:
		series_unload(_ripMedHand1);
		player_set_commands_allowed(true);
		break;

	case 350:
		digi_stop(2);
		terminateMachineAndNull(_jarLeft);
		series_unload(_jarWaterFallingLeft);
		break;

	case 351:
		digi_stop(2);
		terminateMachineAndNull(_jarCentre);
		series_unload(_jarWaterFallingCentre);
		break;

	case 352:
		digi_stop(2);
		terminateMachineAndNull(_jarRight);
		series_unload(_jarWaterFallingRight);
		break;

	case 360:
		_int7 = 1031;
		digi_preload("407_s24");
		digi_preload("407_s27");
		digi_preload("407_s27a");
		digi_preload("407_s27b");
		digi_preload("407_s27c");

		terminateMachineAndNull(_comboLockPopup);
		terminateMachineAndNull(_lockButton1);
		terminateMachineAndNull(_lockButton2);
		terminateMachineAndNull(_lockButton3);
		terminateMachineAndNull(_lockButton4);
		series_unload(_407pu08d);
		series_unload(_407pu08c);
		series_unload(_407pu08b);
		series_unload(_407pu08a);

		setHotspots();
		_407e = series_load("407E");
		terminateMachineAndNull(_escape);
		_escape = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
			triggerMachineByHashCallbackNegative, "METAL DOOR");
		sendWSMessage_10000(1, _escape, _407e, 1, 30, 364, _407e, 30, 30, 0);
		digi_play("407_s24", 2);
		kernel_timing_trigger(30, 362);
		break;

	case 362:
		player_update_info();
		ws_walk(_G(player_info).x, _G(player_info).y, nullptr, 363, 1);
		break;

	case 363:
		_exit = series_load("407 EXIT");

		if (_frotz5)
			digi_play("407r43", 1);
		break;

	case 364:
		ws_walk(409, 320, nullptr, 365, 1);
		break;

	case 365:
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "RIP exits");
		sendWSMessage_10000(1, _ripley, _exit, 1, 22, 366, _exit, 22, 22, 0);
		break;

	case 366:
		digi_play("407_s03", 3);
		digi_play("407r44", 1);
		sendWSMessage_10000(1, _ripley, _exit, 22, 65, 367, _exit, 65, 65, 0);
		break;

	case 367:
		sendWSMessage_10000(1, _ripley, _exit, 66, 81, -1, _exit, 81, 81, 0);
		sendWSMessage_10000(1, _escape, _407e, 30, 1, 368, _407e, 1, 1, 0);
		digi_play("407_s24", 2);
		break;

	case 368:
		digi_play("407_s25", 2, 255, 369);
		break;

	case 369:
		kernel_timing_trigger(60, 370);
		break;

	case 370:
		digi_play("407r45", 1);
		kernel_timing_trigger(60, 371);
		break;

	case 371:
		disable_player_commands_and_fade_init(380);
		break;

	case 380:
		kernel_timing_trigger(60, 381);
		break;

	case 381:
		digi_play("407_s27a", 3, 255, 382);
		break;

	case 382:
		digi_play("407_s27", 3, 255, 383);
		break;

	case 383:
		kernel_timing_trigger(60, 384);
		break;

	case 384:
		digi_play("407r46", 1, 255, 385);
		break;

	case 385:
		digi_play("407_s27b", 3, 255, 386);
		break;

	case 386:
		digi_play("407_s27", 3, 255, 390);
		break;

	case 390:
		digi_play("407_s27a", 3);
		digi_play("407r44", 1, 255, 391);
		break;

	case 391:
		digi_play("407r47", 1, 255, 392);
		break;

	case 392:
		digi_play("407_s7b", 3, 255, 393);
		break;

	case 393:
		digi_play("407_s30", 3, 255, 395);
		break;

	case 395:
		_G(flags)[V030] = 1;
		midi_stop();
		digi_stop(3);
		_G(game).setRoom(408);
		break;

	case 400:
		ws_unhide_walker();
		sendWSMessage_10000(1, _ripley, _stair, 1, 30, 402, _stair, 30, 30, 0);
		kernel_timing_trigger(60, 401);
		ws_walk(347, 331, nullptr, -1, 9);
		break;

	case 401:
		digi_play("407_s12", 2);
		break;

	case 402:
		terminateMachineAndNull(_ripley);
		_star2 = series_place_sprite("407STAR2", 0, 0, -53, 100, 0xf00);
		series_unload(_stair);
		series_unload(_ripEnters);
		player_set_commands_allowed(true);
		break;

	case 410:
		if (_xyzzy2 == 1100) {
			digi_play("407_s32", 3);
		} else if (_xyzzy3 == 1100) {
			digi_play_loop("407_s31", 3);
			_sink = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
				triggerMachineByHashCallbackNegative, "water into sink");
			sendWSMessage_10000(1, _sink, _waterFlowTube, 1, 4, -1,
				_waterFlowTube, 1, 4, 0);
			hotspot_set_active("WATER ", true);
		} else {
			_sink = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
				triggerMachineByHashCallbackNegative, "water into sink");
			sendWSMessage_10000(1, _sink, _waterFlowFaucet, 1, 4, -1,
				_waterFlowFaucet, 1, 4, 0);
			hotspot_set_active("WATER", true);
			digi_play("407_s31", 3);
		}

		player_set_commands_allowed(true);
		break;

	case 420:
		if (_xyzzy2 == 1130) {
			digi_play_loop("407_s32", 3);
		} else {
			digi_play_loop("407_s31", 3);
			_sink = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xe00, 0,
				triggerMachineByHashCallbackNegative, "water into sink");
			sendWSMessage_10000(1, _sink, _waterFlowStem, 1, 4, -1,
				_waterFlowStem, 1, 4, 0);
			hotspot_set_active("WATER  ", true);
		}

		player_set_commands_allowed(true);
		break;

	case 430:
		digi_stop(3);

		if (_xyzzy2 != 1100) {
			hotspot_set_active("WATER ", false);
			hotspot_set_active("WATER", false);
			terminateMachineAndNull(_sink);
		}
		break;

	case 440:
		digi_stop(3);

		if (_xyzzy2 != 1130) {
			hotspot_set_active("WATER  ", false);
			terminateMachineAndNull(_sink);
		}
		break;

	case 450:
		digi_play("407_s16a", 1);
		break;

	case 666:
		digi_unload("407r37");
		break;

	case 1200:
		if (_val4 == 1010) {
			digi_play("407_s31", 3);
			kernel_timing_trigger(_val1, 1200);
		}
		break;

	case 1202:
		if (_val4 == 1010) {
			digi_play("407_s32", 3);
			kernel_timing_trigger(_val2, 1202);
		}
		break;

	default:
		break;
	}
}

void Room407::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if ((player_said("SURGICAL TUBE", "FAUCET PIPE") || player_said("TUBE/HOSE", "FAUCET PIPE")) &&
			_val8 == 1100) {
		_G(player).resetWalk();
		kernel_timing_trigger(1, 777, KT_PARSE, KT_PREPARSE);
	}

	if (player_said("LEVER KEY  ", "GLASS JAR") ||
			player_said("LEVER KEY  ", "JAR/RUBBER PLUG") ||
			player_said("LEVER KEY  ", "JAR/CORK") ||
			player_said("LEVER KEY  ", "JAR/GRIPS") ||
			player_said("LEVER KEY  ", "JAR/CORK/PLUG") ||
			player_said("LEVER KEY  ", "JAR/PLUG/GRIPS") ||
			player_said("LEVER KEY  ", "JAR/CORK/GRIPS") ||
			player_said("LEVER KEY  ", "JAR/PLUG/CORK/GRIPS") ||
			player_said("FAUCET PIPE ", "TUBE/HOSE") ||
			player_said("FAUCET PIPE ", "GARDEN HOSE") ||
			(takeFlag && player_said("GARDEN HOSE ")) ||
			player_said("GLASS JAR ", "FAUCET PIPE") ||
			player_said("GLASS JAR ", "FAUCET PIPE/HOSE") ||
			player_said("GLASS JAR ", "FAUCET PIPE/HOSE/TUBE")) {
		_G(player).resetWalk();
		kernel_timing_trigger(1, 777, KT_PARSE, KT_PREPARSE);
	}

	if (useFlag && player_said("LEVER KEY  ")) {
		_G(player).resetWalk();
		kernel_timing_trigger(1, 777, KT_PARSE, KT_PREPARSE);
	}

	if (player_said("SURGICAL TUBE  ", "GARDEN HOSE ")) {
		player_set_commands_allowed(false);
		_G(player).resetWalk();
		intr_cancel_sentence();
		kernel_timing_trigger(1, 310, KT_DAEMON, KT_PARSE);
	}

	if (player_said("GARDEN HOSE  ", "FAUCET PIPE ")) {
		player_set_commands_allowed(false);
		_G(player).resetWalk();
		kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
	}

	if (player_said("FAUCET PIPE  ", "GLASS JAR ") ||
			player_said("SURGICAL TUBE ", "FAUCET STEM")) {
		player_set_commands_allowed(false);
		_G(player).resetWalk();
		intr_cancel_sentence();
		kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
	}

	if (useFlag && player_said("SURGICAL TUBE  ") && _xyzzy2 != 1130) {
		mouse_set_sprite(43);
		intr_freshen_sentence();
	}

	if (useFlag && player_said("SURGICAL TUBE ")) {
		mouse_set_sprite(43);
		intr_freshen_sentence();
	}

	if (useFlag && player_said("GARDEN HOSE  ") && _val8 != 1130) {
		if (_val4 != 1101) {
			digi_play("407R99E", 1);
			intr_cancel_sentence();
			return;
		}

		mouse_set_sprite(36);
		intr_freshen_sentence();
	}

	if (useFlag && player_said("GARDEN HOSE ") && _xyzzy3 != 1116) {
		mouse_set_sprite(36);
		intr_freshen_sentence();
	}

	if (useFlag && player_said("FAUCET PIPE  ")) {
		mouse_set_sprite(44);
		intr_freshen_sentence();
	}

	if (useFlag && player_said("FAUCET PIPE ") && _xyzzy2 != 1116) {
		mouse_set_sprite(44);
		intr_freshen_sentence();
	}

	if (_val15 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_val15 = 1031;
			kernel_timing_trigger(1, 216, KT_DAEMON, KT_PARSE);
		}
	} else if (_val5 == 1010) {
		_G(player).resetWalk();

		if (player_said(" ") || (lookFlag && player_said("MESSAGE LOG"))) {
			intr_cancel_sentence();
			_val5 = 1011;
			kernel_timing_trigger(1, 10, KT_DAEMON, KT_PARSE);
		}
	} else if (_frotz10 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_frotz10 = 1031;
			kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
		}
	} else if (_int1 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int1 = 1031;
			kernel_timing_trigger(1, 202, KT_DAEMON, KT_PARSE);
		}
	} else if (_int3 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int3 = 1031;
			kernel_timing_trigger(1, 204, KT_DAEMON, KT_PARSE);
		}
	} else if (_int4 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int4 = 1031;
			kernel_timing_trigger(1, 206, KT_DAEMON, KT_PARSE);
		}
	} else if (_int2 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int2 = 1031;
			kernel_timing_trigger(1, 208, KT_DAEMON, KT_PARSE);
		}
	} else if (_int5 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int5 = 1031;
			kernel_timing_trigger(1, 210, KT_DAEMON, KT_PARSE);
		}
	} else if (_int6 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int6 = 1031;
			kernel_timing_trigger(1, 212, KT_DAEMON, KT_PARSE);
		}
	} else if (_int7 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_int7 = 1031;
			kernel_timing_trigger(1, 214, KT_DAEMON, KT_PARSE);
		}
	} else if (_val16 == 1030) {
		_G(player).resetWalk();

		if (player_said(" ")) {
			intr_cancel_sentence();
			_val16 = 1031;
			kernel_timing_trigger(1, 218, KT_DAEMON, KT_PARSE);
		}
	}
}

void Room407::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (lookFlag && player_said("ROMANOV EMERALD")) {
		lookItem("PING ROMANOV EMERALD", "407R34");
	} else if (lookFlag && player_said("JAR/RUBBER PLUG")) {
		lookItem("PING JAR/RUBBER PLUG", "407R77");
	} else if (lookFlag && player_said("JAR/CORK")) {
		lookItem("PING JAR/CORK", "407R78");
	} else if (lookFlag && player_said("JAR/CORK/PLUG")) {
		lookItem("PING JAR/CORK/PLUG", "407R79");
	} else if (lookFlag && player_said("JAR/GRIPS")) {
		lookItem("PING JAR/GRIPS", "407R80");
	} else if (lookFlag && player_said("JAR/PLUG/GRIPS")) {
		lookItem("PING JAR/PLUG/GRIPS", "407R81");
	} else if (lookFlag && player_said("JAR/CORK/GRIPS")) {
		lookItem("PING JAR/CORK/GRIPS", "407R82");
	} else if (lookFlag && player_said("JAR/PLUG/CORK/GRIPS")) {
		lookItem("PING JAR/PLUG/CORK/GRIPS", "407R83");
	} else if (lookFlag && player_said("TUBE/HOSE")) {
		lookItem("PING TUBE/HOSE", "407R93");
	} else if (lookFlag && player_said("FAUCET PIPE/HOSE")) {
		lookItem("PING FAUCET PIPE/HOSE", "407R94");
	} else if (lookFlag && player_said("FAUCET PIPE ")) {
		digi_play((_xyzzy3 == 1116) ? "407R86" : "407R07", 1);
	} else if (lookFlag && player_said("FAUCET PIPE/HOSE/TUBE")) {
		lookItem("PING FAUCET PIPE/HOSE/TUBE", "407R95");
	} else if ((useFlag && player_said("MICROSCOPE")) ||
			player_said("ROMANOV EMERALD", "MICROSCOPE")) {
		useMicroscope();
	} else if (lookFlag && player_said("LETTER") &&
			inv_object_is_here("MENENDEZ'S LETTER")) {
		digi_play("407r38", 1);
	} else if (lookFlag && player_said("SURGICAL TUBE") &&
			inv_object_is_here("SURGICAL TUBE")) {
		digi_play("407r31", 1);
	} else if (lookFlag && player_said("WALL PISTON")) {
		digi_play("407r99r", 1);
	} else if (lookFlag && player_said("PUMP ROD ")) {
		digi_play("407r13", 1);
	} else if (lookFlag && player_said("LEVER KEY  ")) {
		roofPistonPopup();
		digi_play("407r26a", 1);
	} else if (lookFlag && player_said("RUBBER PLUG") &&
			inv_object_is_here("RUBBER PLUG")) {
		digi_play("407r06", 1);
	} else if (lookFlag && player_said("JAR/RUBBER PLUG") &&
			inv_object_is_here("JAR/RUBBER PLUG")) {
		if (_xyzzy7 == 1112)
			glassBottomPopup();
		digi_play("407r77", 1);
	} else if (lookFlag && player_said("FAUCET PIPE") &&
			inv_object_is_here("FAUCET PIPE")) {
		digi_play("407r07", 1);
	} else if (lookFlag && player_said("SURGICAL TUBE   ") &&
			inv_object_is_here("SURGICAL TUBE")) {
		digi_play("407r31", 1);
	} else if (lookFlag && player_said("FAUCET HANDLE") &&
			inv_object_is_here("FAUCET HANDLE")) {
		digi_play("407r08", 1);
	} else if (lookFlag && player_said("TABLE PIVOT")) {
		pivotPopup();
		digi_play("407r09", 1);
	} else if (lookFlag && player_said("NOZZLES")) {
		tabletopPopup();
		digi_play("407r10", 1);
	} else if (lookFlag && player_said("PUMP ROD") &&
			inv_object_is_here("PUMP ROD")) {
		digi_play("407r13", 1);
	} else if (lookFlag && player_said("GLASS JAR") &&
			inv_object_is_here("GLASS JAR") && lookGlassJar()) {
		// No implementation
	} else if (lookFlag && player_said("EMERALD/CORK") &&
			inv_object_is_here("EMERALD/CORK") && lookEmeraldCork()) {
		// No implementation
	} else if (lookFlag && player_said("IRON SUPPORT")) {
		if (_xyzzy7 == 1112) {
			glassTopPopup();
		} else {
			glassGonePopup();
		}
	} else if (lookFlag && player_said("SMALL GEAR WHEEL")) {
		if (_xyzzy7 == 1112) {
			glassTopPopup();
		} else {
			glassGonePopup();
		}

		digi_play("407r21", 1);
	} else if (lookFlag && player_said("PERIODIC TABLE/JAR") && _xyzzy7 == 1112) {
		glassTopPopup();
	} else if (lookFlag && player_said("PERIODIC TABLE")) {
		if (inv_player_has("PERIODIC TABLE")) {
			periodicTablePopup();
		} else {
			digi_play(_frotz8 ? "407r99c" : "407r22", 1);
		}
	} else if (lookFlag && player_said("BRACKET")) {
		roofPistonPopup();
		digi_play("407r24", 1);
	} else if (lookFlag && player_said("CEILING PISTON")) {
		roofPistonPopup();
		digi_play("407r25", 1);
	} else if (lookFlag && player_said("LEVER KEY") &&
			inv_object_is_here("LEVER KEY") && lookLeverKey()) {
		// No implementation
	} else if (lookFlag && player_said("LEVER KEY ") &&
			inv_object_is_here("LEVER KEY")) {
		pivotPopup();
		digi_play("407r64", 1);
	} else if (lookFlag && player_said("AIR VALVE/HANDLE")) {
		tabletopPopup();
		digi_play("407r63", 1);
	} else if (lookFlag && player_said("AIR VALVE")) {
		tabletopPopup();
		digi_play("407r62", 1);
	} else if (lookFlag && player_said("NOZZLES/TUBE")) {
		tabletopPopup();
		digi_play("407r31", 1);
	} else if (lookFlag && player_said("GARDEN HOSE") &&
			inv_object_is_here("GARDEN HOSE")) {
		digi_play("407r28", 1);
	} else if (lookFlag && player_said("PUMP GRIPS ") &&
			inv_object_is_here("PUMP GRIPS")) {
		digi_play("407r99", 1);
	} else if (lookFlag && player_said("PUMP GRIPS") &&
			inv_object_is_here("PUMP GRIPS")) {
		digi_play("407r30", 1);
	} else if (player_said("GARDEN HOSE", "SURGICAL TUBE   ") && _xyzzy3 == 1100) {
		gardenHoseSurgicalTube();
	} else if ((player_said("SURGICAL TUBE", "FAUCET PIPE") ||
			player_said("TUBE/HOSE", "FAUCET PIPE")) && _val8 == 1100) {
		surgicalTubeFaucetPipe();
	} else if ((player_said("SURGICAL TUBE", "FAUCET STEM") ||
			player_said("TUBE/HOSE", "FAUCET STEM") ||
			player_said("FAUCET PIPE/HOSE/TUBE", "FAUCET STEM")) &&
			_val8 != 1100) {
		surgicalTubeStem();
	} else if (player_said("GARDEN HOSE", "SURGICAL TUBE  ") ||
			player_said("FAUCET PIPE/HOSE", "SURGICAL TUBE  ")) {
		gardenHoseSurgicalTube2();
	} else if (player_said("FAUCET PIPE", "GARDEN HOSE  ")) {
		faucetPipeGardenHose();
	} else if (player_said("FAUCET PIPE", "GLASS JAR ") ||
			player_said("FAUCET PIPE/HOSE", "GLASS JAR ") ||
			player_said("FAUCET PIPE/HOSE/TUBE", "GLASS JAR ")) {
		faucetPipeGlassJar();
	} else if (player_said("GARDEN HOSE", "FAUCET PIPE ") ||
			player_said("TUBE/HOSE", "FAUCET PIPE ")) {
		gardenHoseFaucetPipe();
	} else if (player_said("SURGICAL TUBE", "GARDEN HOSE ")) {
		surgicalTubeGardenHose();
	} else if (player_said("ROMANOV EMERALD", "IRON SUPPORT") && _val6 == 1011) {
		emeraldIronSupport();
	} else if (player_said("GLASS JAR", "LEVER KEY  ") ||
			player_said("JAR/RUBBER PLUG", "LEVER KEY  ") ||
			player_said("JAR/CORK", "LEVER KEY  ") ||
			player_said("JAR/GRIPS", "LEVER KEY  ") ||
			player_said("JAR/CORK/PLUG", "LEVER KEY  ") ||
			player_said("JAR/PLUG/GRIPS", "LEVER KEY  ") ||
			player_said("JAR/CORK/GRIPS", "LEVER KEY  ") ||
			player_said("JAR/PLUG/CORK/GRIPS", "LEVER KEY  ")) {
		reachLeverKey();
	} else if (player_said("LEVER KEY/PUMP ROD", "BRACKET")) {
		placeLeverKey();
	} else if (player_said("RUBBER PLUG", "GLASS JAR") && _xyzzy7 == 1112) {
		rubberPlugGlassJar();
	} else if (player_said("CORK", "GLASS JAR") && _xyzzy7 == 1112) {
		corkGlassJar();
	} else if (player_said("pump grips", "glass jar") &&
			(_xyzzy7 == 1114 || _xyzzy7 == 1140)) {
		pumpGripsGlassJar();
	} else if (player_said("rubber plug", "glass jar ") &&
			(_xyzzy7 == 1114 || _xyzzy7 == 1140)) {
		rubberPlugGlassJar2();
	} else if (player_said("CORK", "GLASS JAR ") &&
			(_xyzzy7 == 1114 || _xyzzy7 == 1140)) {
		corkGlassJar2();
	} else if (player_said("PERIODIC TABLE", "GLASS JAR") && _xyzzy6 == 1000) {
		periodicTableGlassJar();
	} else if (player_said("SURGICAL TUBE", "NOZZLES") && _xyzzy3 == 1000) {
		surgicalTubeNozzles();
	} else if (player_said("LEVER KEY", "TABLE PIVOT") && _val10 == 1000) {
		leverKeyTablePivot();
	} else if (player_said("PUMP ROD", "BRACKET") && _xyzzy1 == 1000) {
		pumpRodBracket();
	} else if ((player_said("LEVER KEY", "BRACKET") ||
			player_said("LEVER KEY", "PUMP ROD ")) ||
			_val10 == 1000) {
		if (_xyzzy1 == 1114)
			leverKey1();
		else
			leverKey2();
	} if (player_said("FAUCET HANDLE", "AIR VALVE") && _val9 == 1000) {
		faucetHandleAirValve();
	} else if (player_said("FAUCET PIPE", "FAUCET HANDLE") &&
			_val9 == 1000 && _val8 == 1100) {
		faucetPipeFaucetHandle1();
	} else if (player_said("FAUCET PIPE", "FAUCET HANDLE") &&
			_val8 == 1000 && _val9 == 1100 && _xyzzy3 != 1130) {
		faucetPipeFaucetHandle2();
	} else if ((player_said("FAUCET PIPE", "FAUCET STEM") ||
			player_said("FAUCET PIPE/TUBE", "FAUCET STEM") ||
			player_said("FAUCET PIPE/TUBE", "FAUCET HANDLE") ||
			player_said("FAUCET PIPE/TUBE", "SINK") ||
			player_said("FAUCET PIPE", "SINK")) ||
			_xyzzy3 != 1130) {
		faucetPipeFaucetHandle2();
	} else if (player_said("FAUCET HANDLE", "FAUCET STEM") && _val9 == 1000) {
		faucetPipeFaucetHandle1();
	} else if (player_said("PUMP ROD", "PUMP") && _xyzzy1 == 1000) {
		pumpRodPump();
	} else if ((player_said("PUMP GRIPS", "PUMP") ||
			player_said("PUMP GRIPS", "PUMP ROD"))
			&& _xyzzy1 == 1115) {
		pumpGripsPump();
	} else if (player_said("RUBBER PLUG", "SINK")) {
		digi_play("407R35", 1);
	} else if (player_said("FAUCET PIPE", "GLASS JAR") ||
		player_said("FAUCET PIPE", "IRON SUPPORT")) {
		digi_play("407R67", 1);

	} else if (useFlag && player_said(" ")) {
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("com006", 1);
			break;
		case 2:
			digi_play("com011", 1);
			break;
		case 3:
			digi_play("com018", 1);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("BUTTON")) {
		useButton1();
	} else if (useFlag && player_said("BUTTON ")) {
		useButton2();
	} else if (useFlag && player_said("BUTTON  ")) {
		useButton3();
	} else if (useFlag && player_said("BUTTON   ")) {
		useButton4();
	} else if (useFlag && player_said("BUTTONS")) {
		useButtons();
	} else if (useFlag && player_said("LEVER KEY  ") &&
			_val10 == 1114 && _xyzzy7 != 1114 && _xyzzy7 != 1140) {
		useLeverKey();
	} else if (useFlag && player_said_any("FAUCET HANDLE", "FAUCET STEM") &&
			_val10 == 1100 && inv_object_is_here("FAUCET HANDLE")) {
		if (_frotz2) {
			digi_play("407r99e", 1);
		} else if (_val8 == 1100) {
			if (_xyzzy6 == 1120)
				digi_play("407r99o", 1);
			else
				useFaucet();
		} else if (_xyzzy3 == 1130 && _val8 != 1130) {
			if (_xyzzy6 == 1120)
				digi_play("407r99o", 1);
			else
				useFaucet();
		} else if (_frotz1) {
			useFaucet();
		} else if (_val8 == 1100 || _xyzzy3 == 1130 || _frotz1) {
			digi_play("407r99e", 1);
		} else {
			digi_play("407r99n", 1);
		}
	} else if (useFlag && player_said("AIR VALVE/HANDLE")
			&& _val9 == 1110 && inv_object_is_here("FAUCET HANDLE")) {
		if (inv_object_is_here("EMERALD/CORK") || _val6 != 1110 ||
				_xyzzy7 != 1112 || _val7 != 1021) {
			useValveHandle();
		} else {
			digi_play("com090a", 1);
		}
	} else if (useFlag && player_said_any("PUMP GRIPS ", "PUMP", "PUMP ROD")
			&& usePump()) {
		// No implementation
	} else if (useFlag && player_said("DRAWER")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripMedReach = series_load("RIP TREK MED REACH POS3");
			setGlobals1(_ripMedReach, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;

		case 1:
			_val5 = 1010;
			_drawerPopup = series_place_sprite("407 DRAWER POPUP", 0, 0, 0, 100, 0x200);
			hotspot_set_active(" ", true);

			if (_xyzzy2 == 1101) {
				_drawerPopupHose = series_place_sprite("407 DRAWER POPUP HOSE", 0, 0, 0, 100, 0x100);
				hotspot_set_active("GARDEN HOSE", true);
			}

			if (_xyzzy5 == 1101) {
				_stopperInDrawer = series_place_sprite("407 STOPPER IN DRAWER",
					0, 0, 0, 100, 0x100);
				hotspot_set_active("RUBBER PLUG", true);
			}

			if (_xyzzy3 == 1101) {
				_tubeInDrawer = series_place_sprite("407 TUBE IN DRAWER",
					0, 0, 0, 100, 0x100);
				hotspot_set_active("SURGICAL TUBE", true);
			}

			if (_xyzzy4 == 1101) {
				_handleInDrawer = series_place_sprite(
					"407 PUMP HANDLE IN DRAWER", 0, 0, 0, 100, 0x100);
				hotspot_set_active("PUMP GRIPS", true);
			}

			digi_play("407_s01", 2);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}

	} else if (takeFlag && player_said("LETTER")) {
		takeLetter();
	} else if (takeFlag && player_said("FAUCET PIPE  ") && _val8 == 1130) {
		takeFaucetPipe1();
	} else if (takeFlag && player_said("SURGICAL TUBE   ") && _xyzzy3 == 1100) {
		takeSurgicalTube1();
	} else if (takeFlag && player_said("GARDEN HOSE    ") && _xyzzy2 == 1100) {
		takeGardenHose1();
	} else if (takeFlag && player_said("GARDEN HOSE  ") && _xyzzy2 == 1130) {
		takeGardenHose2();
	} else if (takeFlag && player_said("SURGICAL TUBE  ") && _xyzzy3 == 1130) {
		takeSurgicalTube2();
	} else if (takeFlag && player_said("FAUCET PIPE ") && _val8 == 1116) {
		takeFaucetPipe2();
	} else if (takeFlag && player_said("GARDEN HOSE ") && _xyzzy2 == 1116) {
		takeGardenHose3();
	} else if (takeFlag && player_said("SURGICAL TUBE ") && _xyzzy3 == 1116) {
		takeSurgicalTube3();
	} else if (takeFlag && player_said("GLASS JAR ")) {
		if (_val8 == 1116 || _xyzzy7 != 1114)
			digi_play("com096", 1);
		else
			takeGlassJar1();
	} else if (takeFlag && player_said("GLASS JAR") &&
			_val6 == 1010 && _xyzzy7 == 1112) {
		takeGlassJar2();
	} else if (takeFlag && player_said("PERIODIC TABLE") && _xyzzy6 == 1112) {
		takePeriodicTable1();
	} else if (takeFlag && player_said("PERIODIC TABLE ")) {
		takePeriodicTable2();
	} else if (takeFlag && player_said("JAR/GRIPS ") &&
			(_xyzzy7 == 1114 || _xyzzy7 == 1140) &&
			_xyzzy4 == 1116) {
		takeJarGrips();
	} else if (takeFlag && player_said("JAR/CORK") && _xyzzy7 == 1112) {
		takeJarCork1();
	} else if (takeFlag && player_said("JAR/CORK ") &&
			(_xyzzy7 == 1114 || _xyzzy7 == 1140) &&
			_xyzzy9 == 1116) {
		takeJarCork2();
	} else if (takeFlag && player_said("JAR/RUBBER PLUG") && _xyzzy7 == 1112) {
		takeJarRubberPlug1();
	} else if (takeFlag && player_said("JAR/RUBBER PLUG ") &&
			(_xyzzy7 == 1114 || _xyzzy7 == 1140) &&
			_xyzzy5 == 1116) {
		takeJarRubberPlug2();
	} else if (takeFlag && player_said("NOZZLES/TUBE")) {
		takeNozzlesTube();
	} else if (takeFlag && player_said("LEVER KEY") && _val10 == 1112) {
		takeLeverKey1();
	} else if (takeFlag && player_said("LEVER KEY ")) {
		takeLeverKey2();
	} else if (takeFlag && player_said("AIR VALVE/HANDLE") && _val9 == 1110) {
		takeAirValveHandle();
	} else if (takeFlag && player_said("FAUCET HANDLE") && _val9 == 1100) {
		takeFaucetHandle();
	} else if (takeFlag && player_said("FAUCET PIPE") && _val8 == 1100) {
		takeFaucetPipe3();
	} else if (takeFlag && player_said("PUMP ROD") && _xyzzy1 == 1115) {
		takePumpRod1();
	} else if (takeFlag && player_said("PUMP ROD ") &&
		_xyzzy7 != 1114 && _xyzzy7 != 1140) {
		if (_val10 == 1114)
			takePumpRod2();
		else
			takePumpRod3();
	} else if (takeFlag && player_said("LEVER KEY  ")) {
		if (_val10 != 1114 || _xyzzy7 == 1114 || _xyzzy7 == 1140)
			digi_play("com129", 1);
		else
			takeLeverKey3();
	} else if (takeFlag && player_said("PUMP GRIPS")) {
		takePumpGrips1();
		return;
	} else if (takeFlag && player_said("PUMP GRIPS ")) {
		takePumpGrips2();
	} else if (takeFlag && player_said("GARDEN HOSE")) {
		takeGardenHose4();
		return;
	} else if (takeFlag && player_said("RUBBER PLUG")) {
		takeRubberPlug();
		return;
	} else if (takeFlag && player_said("SURGICAL TUBE")) {
		takeSurgicalTube();
		return;
	} else if (takeFlag && player_said("MICROSCOPE")) {
		digi_play("407r61", 1);
	} else if (takeFlag && (
		player_said("DRAIN") ||
		player_said("FAUCET STEM") ||
		player_said("IRON SUPPORT") ||
		player_said("AIR VALVE") ||
		player_said("NOZZLES") ||
		player_said("TALBE PIVOT") ||
		player_said("HIDDEN DOOR") ||
		player_said("CEILING PISTON") ||
		player_said("DRAWER") ||
		player_said("PISTON ON TABLE") ||
		player_said("BUTTONS") ||
		player_said("COMPRESSED AIR TANK") ||
		player_said("WALL PISTON") ||
		player_said("BRACKET") ||
		player_said("LARGE GEAR") ||
		player_said("SMALL GEAR WHEEL") ||
		player_said("METAL DOOR") ||
		player_said("ARMORED CABINET")
		)) {
		digi_play("com066", 1);
	} else if ((player_said("FAUCET PIPE/HOSE", "SURGICAL TUBE") ||
			player_said("TUBE/HOSE", "FAUCET PIPE") ||
			player_said("FAUCET PIPE/TUBE", "GARDEN HOSE")) &&
			(_xyzzy2 == 1060 || _xyzzy2 == 1061 || _xyzzy3 == 1061)) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE/TUBE",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy2 = 1062;
			_val8 = 1111;
			_xyzzy3 = 1111;
			inv_move_object("SURGICAL TUBE", 407);
			inv_move_object("FAUCET PIPE", 407);
			inv_move_object("GARDEN HOSE", 407);
			inv_move_object("FAUCET PIPE/HOSE", 407);
			inv_move_object("FAUCET PIPE/TUBE", 407);
			inv_move_object("TUBE/HOSE", 407);
			inv_give_to_player("FAUCET PIPE/HOSE/TUBE");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("FAUCET PIPE", "GARDEN HOSE") &&
		_xyzzy2 == 1000 && _val8 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy2 = 1061;
			_val8 = 1111;
			inv_move_object("GARDEN HOSE", 407);
			inv_move_object("FAUCET PIPE", 407);
			inv_give_to_player("FAUCET PIPE/HOSE");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("GARDEN HOSE", "SURGICAL TUBE") &&
			_xyzzy2 == 1000 && _xyzzy3 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING TUBE/HOSE",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy2 = 1060;
			_xyzzy3 = 1111;
			inv_move_object("GARDEN HOSE", 407);
			inv_move_object("SURGICAL TUBE", 407);
			inv_give_to_player("TUBE/HOSE");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("FAUCET PIPE", "SURGICAL TUBE") &&
			_val8 == 1000 && _xyzzy3 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING FAUCET PIPE/TUBE",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_val8 = 1060;
			_xyzzy3 = 1061;
			inv_move_object("FAUCET PIPE", 407);
			inv_move_object("SURGICAL TUBE", 407);
			inv_give_to_player("FAUCET PIPE/TUBE");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if ((player_said("JAR/PLUG/GRIPS", "CORK") ||
			player_said("JAR/CORK/PLUG", "PUMP GRIPS") ||
			player_said("JAR/CORK/GRIPS", "RUBBER PLUG")) &&
			(_xyzzy7 == 1054 || _xyzzy7 == 1053 || _xyzzy7 == 1055)) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/PLUG/CORK/GRIPS",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1056;
			_xyzzy9 = 1116;
			_xyzzy5 = 1116;
			_xyzzy4 = 1116;
			inv_move_object("JAR/PLUG/GRIPS", 407);
			inv_move_object("JAR/CORK/PLUG", 407);
			inv_move_object("JAR/CORK/GRIPS", 407);
			inv_move_object("RUBBER PLUG", 407);
			inv_move_object("CORK", 407);
			inv_move_object("PUMP GRIPS", 407);
			inv_give_to_player("JAR/PLUG/CORK/GRIPS");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if ((player_said("JAR/RUBBER PLUG", "CORK") ||
			player_said("JAR/CORK", "RUBBER PLUG")) &&
			(_xyzzy7 == 1050 || _xyzzy7 == 1051)) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/CORK/PLUG",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1053;
			_xyzzy9 = 1116;
			_xyzzy5 = 1116;
			inv_move_object("JAR/RUBBER PLUG", 407);
			inv_move_object("JAR/CORK", 407);
			inv_move_object("RUBBER PLUG", 407);
			inv_move_object("CORK", 407);
			inv_give_to_player("JAR/CORK/PLUG");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if ((player_said("JAR/GRIPS", "CORK") ||
			player_said("JAR/CORK", "PUMP GRIPS")) &&
			(_xyzzy7 == 1052 || _xyzzy7 == 1051)) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/CORK/GRIPS",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1055;
			_xyzzy9 = 1116;
			_xyzzy4 = 1116;
			inv_move_object("JAR/GRIPS", 407);
			inv_move_object("JAR/CORK", 407);
			inv_move_object("PUMP GRIPS", 407);
			inv_move_object("CORK", 407);
			inv_give_to_player("JAR/CORK/GRIPS");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if ((player_said("JAR/GRIPS", "RUBBER PLUG") ||
			player_said("JAR/RUBBER PLUG", "PUMP GRIPS")) &&
			(_xyzzy7 == 1052 || _xyzzy7 == 1050)) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/PLUG/GRIPS",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1055;
			_xyzzy5 = 1116;
			_xyzzy4 = 1116;
			inv_move_object("JAR/GRIPS", 407);
			inv_move_object("JAR/RUBBER PLUG", 407);
			inv_move_object("PUMP GRIPS", 407);
			inv_move_object("RUBBER PLUG", 407);
			inv_give_to_player("JAR/PLUG/GRIPS");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("GLASS JAR", "CORK") &&
			_xyzzy7 == 1000 && _xyzzy9 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/CORK",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1051;
			_xyzzy9 = 1116;
			inv_move_object("GLASS JAR", 407);
			inv_move_object("CORK", 407);
			inv_give_to_player("JAR/CORK");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("GLASS JAR", "PUMP GRIPS") &&
			_xyzzy7 == 1000 && _xyzzy4 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/GRIPS",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1052;
			_xyzzy4 = 1116;
			inv_move_object("GLASS JAR", 407);
			inv_move_object("PUMP GRIPS", 407);
			inv_give_to_player("JAR/GRIPS");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("GLASS JAR", "RUBBER PLUG") &&
			_xyzzy7 == 1000 && _xyzzy5 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING JAR/RUBBER PLUG",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_xyzzy7 = 1050;
			_xyzzy5 = 1116;
			inv_move_object("GLASS JAR", 407);
			inv_move_object("RUBBER PLUG", 407);
			inv_give_to_player("JAR/RUBBER PLUG");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("LEVER KEY", "PUMP ROD") &&
			_val10 == 1000 && _xyzzy1 == 1000) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_examine_inventory_object("PING LEVER KEY/PUMP ROD",
				_G(master_palette), 5, 1, 270, 150, 2, nullptr, -1);
			break;
		case 2:
			_val10 = 1041;
			_xyzzy1 = 1041;
			_xyzzy10 = 1041;
			inv_move_object("LEVER KEY", 407);
			inv_move_object("PUMP ROD", 407);
			inv_give_to_player("LEVER KEY/PUMP ROD");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}

	} else if (useFlag && player_said("FAUCET PIPE/HOSE/TUBE") &&
			_xyzzy2 == 1062) {
		_xyzzy2 = 1000;
		_xyzzy3 = 1000;
		_val8 = 1000;
		inv_move_object("FAUCET PIPE/HOSE/TUBE", 407);
		inv_give_to_player("GARDEN HOSE");
		inv_give_to_player("SURGICAL TUBE");
		inv_give_to_player("FAUCET PIPE");
	} else if (useFlag && player_said("TUBE/HOSE") && _xyzzy2 == 1060) {
		_xyzzy2 = 1000;
		_xyzzy3 = 1000;
		inv_move_object("TUBE/HOSE", 407);
		inv_give_to_player("GARDEN HOSE");
		inv_give_to_player("SURGICAL TUBE");
	} else if (useFlag && player_said("FAUCET PIPE/HOSE") && _xyzzy2 == 1061) {
		_xyzzy2 = 1000;
		_val8 = 1000;
		inv_move_object("FAUCET PIPE/HOSE", 407);
		inv_give_to_player("GARDEN HOSE");
		inv_give_to_player("FAUCET PIPE");
	} else if (useFlag && player_said("FAUCET PIPE/TUBE") && _xyzzy3 == 1061) {
		_xyzzy3 = 1000;
		_val8 = 1000;
		inv_move_object("FAUCET PIPE/TUBE", 407);
		inv_give_to_player("SURGICAL TUBE");
		inv_give_to_player("FAUCET PIPE");
	} else if (useFlag && player_said("JAR/PLUG/CORK/GRIPS") && _xyzzy7 == 1056) {
		_xyzzy7 = 1000;
		_xyzzy9 = 1000;
		_xyzzy4 = 1000;
		_xyzzy5 = 1000;
		inv_move_object("JAR/PLUG/CORK/GRIPS", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("PUMP GRIPS");
		inv_give_to_player("CORK");
		inv_give_to_player("RUBBER PLUG");
	} else if (useFlag && player_said("JAR/CORK/PLUG") && _xyzzy7 == 1053) {
		_xyzzy7 = 1000;
		_xyzzy9 = 1000;
		_xyzzy5 = 1000;
		inv_move_object("JAR/CORK/PLUG", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("RUBBER PLUG");
		inv_give_to_player("CORK");
	} else if (useFlag && player_said("JAR/CORK/GRIPS") && _xyzzy7 == 1055) {
		_xyzzy7 = 1000;
		_xyzzy9 = 1000;
		_xyzzy4 = 1000;
		inv_move_object("JAR/CORK/GRIPS", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("PUMP GRIPS");
		inv_give_to_player("CORK");
	} else if (useFlag && player_said("JAR/PLUG/GRIPS") && _xyzzy7 == 1054) {
		_xyzzy7 = 1000;
		_xyzzy5 = 1000;
		_xyzzy4 = 1000;
		inv_move_object("JAR/PLUG/GRIPS", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("PUMP GRIPS");
		inv_give_to_player("RUBBER PLUG");
	} else if (useFlag && player_said("JAR/GRIPS") && _xyzzy7 == 1052) {
		_xyzzy7 = 1000;
		_xyzzy4 = 1000;
		inv_move_object("JAR/GRIPS", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("PUMP GRIPS");
	} else if (useFlag && player_said("JAR/CORK") && _xyzzy7 == 1051) {
		_xyzzy7 = 1000;
		_xyzzy9 = 1000;
		inv_move_object("JAR/CORK", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("CORK");
	} else if (useFlag && player_said("JAR/RUBBER PLUG") && _xyzzy7 == 1050) {
		_xyzzy7 = 1000;
		_xyzzy5 = 1000;
		inv_move_object("JAR/RUBBER PLUG", 407);
		inv_give_to_player("GLASS JAR");
		inv_give_to_player("RUBBER PLUG");
	} else if (useFlag && player_said("LEVER KEY/PUMP ROD") && _xyzzy10 == 1041) {
		_val10 = 1000;
		_xyzzy1 = 1000;
		_xyzzy10 = 1040;
		inv_move_object("LEVER KEY/PUMP ROD", 407);
		inv_give_to_player("LEVER KEY");
		inv_give_to_player("PUMP ROD");
	} else if (useFlag && player_said("EMERALD/CORK") &&
			inv_player_has("EMERALD/CORK")) {
		_xyzzy8 = 1000;
		_xyzzy9 = 1000;
		inv_move_object("EMERALD/CORK", NOWHERE);
		inv_give_to_player("ROMANOV EMERALD");
		inv_give_to_player("CORK");
	} else if (player_said("journal") &&
			!takeFlag && !lookFlag && !inv_player_has(_G(player).noun)) {
		digi_play("407r59", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
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

void Room407::lookItem(const char *item, const char *digi) {
	switch (_G(kernel).trigger) {
	case -1:
		kernel_examine_inventory_object(item, _G(master_palette),
			5, 1, 175, 150, 1, digi, -1);
		break;
	case 1:
		player_set_commands_allowed(true);
		break;
	default:
		break;
	}
}

void Room407::useMicroscope() {
	if (_G(kernel).trigger == -1) {
		_val16 = 1030;

		if (player_said("ROMANOV EMERALD", "MICROSCOPE")) {
			_frotz5 = 1;
			_microscopeCloseup = series_place_sprite("407 MICROSCOPE CLOSEUP",
				0, 200, 150, 100, 0x200);
			digi_play("407r42", 1);
		} else {
			_microscopeCloseup = series_place_sprite("407pu07A", 0, 0, 0, 100, 0x200);
		}

		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

void Room407::roofPistonPopup() {
	if (_G(kernel).trigger == -1) {
		_int5 = 1030;
		_roofPiston = series_place_sprite("407 ROOF PISTON/BRACE",
			0, 0, 0, 100, 0x200);

		if (_val10 == 1114) {
			_tabletopPopupWithItems3 = series_place_sprite(
				"407 ROOF PISTON WITH ITEMS", 1, 0, 0, 100, 0x100);
		}

		if (_xyzzy1 == 1114) {
			_roofPistonWithItems = series_place_sprite(
				"407 ROOF PISTON WITH ITEMS", 0, 0, 0, 100, 0);
		}

		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

void Room407::glassBottomPopup() {
	if (_G(kernel).trigger == -1) {
		_int4 = 1030;
		_glassTopPopup = series_place_sprite(
			"407 GLASS BOTTOM POPUP", 0, 0, 0, 100, 0x200);

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
				"407 GLS BOTTOM PU WITH ITEMS", 1, 0, 0, 100, 0x100);

		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

void Room407::glassTopPopup() {
	if (_G(kernel).trigger == -1) {
		_int3 = 1030;
		_glassTopPopup = series_place_sprite("407 GLASS TOP POPUP",
			0, 0, 0, 100, 0x200);

		if (_val6 == 1010) {
			_glassTopPopupWithItems1 = series_place_sprite(
				"407 GLASS TOP POPUP WITH ITEMS", 1, 0, 0, 100, 0x100);

			if (!player_said("SMALL GEAR WHEEL"))
				digi_play("407r66a", 1);

		} else if (_xyzzy6 == 1116) {
			_glassTopPopupWithItems2 = series_place_sprite(
				"407 GLASS TOP POPUP WITH ITEMS", 0, 0, 0, 100, 0x100);

			if (!player_said("SMALL GEAR WHEEL"))
				digi_play("407r99a", 1);

		} else {
			if (!player_said("SMALL GEAR WHEEL"))
				digi_play("407r19", 1);
		}
	}
}

void Room407::glassGonePopup() {
	if (_G(kernel).trigger == -1) {
		_int6 = 1030;
		_glassGone = series_place_sprite("407 GLASS GONE CU PU",
			0, 0, 0, 100, 0x200);
		disableHotspots();
		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

void Room407::pivotPopup() {
	if (_G(kernel).trigger == -1) {
		_int1 = 1030;
		_pivotPopup = series_place_sprite("407 PIVOT POPUP",
			0, 0, 0, 100, 0x200);

		if (_val10 == 1113)
			_tabletopPopupWithItems3 = series_place_sprite(
				"407 TABLE PIVOT WITH LEVER", 0, 0, 0, 100, 0x200);

		disableHotspots();
		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

void Room407::tabletopPopup() {
	if (_G(kernel).trigger == -1) {
		_frotz10 = 1030;
		_tabletopPopup = series_place_sprite("407 TABLETOP POPUP",
			0, 0, 0, 100, 0x200);
		disableHotspots();
		hotspot_set_active(" ", true);

		if (_xyzzy3 == 1117)
			_tabletopPopupWithItems1 = series_place_sprite(
				"407 TABLETOP POPUP WITH ITEMS", 0, 0, 0, 100, 0);

		if (_val9 == 1110)
			_tabletopPopupWithItems2 = series_place_sprite(
				"407 TABLETOP POPUP WITH ITEMS",
				(_val3 == 1010) ? 1 : 2, 0, 0, 100, 0x100);

		if (_val10 == 1113)
			_tabletopPopupWithItems3 = series_place_sprite(
				"407 TABLETOP POPUP WITH ITEMS", 0, 0, 0, 100, 0x100);

		player_set_commands_allowed(true);
	}
}

bool Room407::lookGlassJar() {
	if (_xyzzy7 == 1112) {
		switch (_G(kernel).trigger) {
		case -1:
			lookGlassJar();

			if (!inv_object_is_here("EMERALD/CORK")) {
				digi_play("407r99a", 1);
			} else if (!_frotz6 && !_frotz7) {
				_frotz7 = 1;
				digi_play("407r15", 1);
			} else {
				_frotz7 = 1;
				digi_play("407r15", 1, 255, _frotz6 ? 2 : -1);
			}
			return true;
		case 1:
			digi_play("407r16", 1);
			return true;
		case 2:
			digi_play("407r16a", 1);
			return true;
		default:
			break;
		}
	}

	return false;
}

void Room407::fullglassPopup() {
	if (_G(kernel).trigger == -1) {
		_int2 = 1030;
		_glassTopPopup = series_place_sprite("407 FULL GLASS POPUP",
			0, 0, 0, 100, 0x200);

		if (_xyzzy6 == 1116)
			_glassTopPopupWithItems2 = series_place_sprite(
				"407 FULL GLASS POPUP WITH ITEMS", 0, 0, 0, 100, 0x100);

		if (_xyzzy8 == 1116)
			_glassBottomWithItems1 = series_place_sprite(
				"407 FULL GLASS POPUP WITH ITEMS", 1, 0, 0, 100, 0x100);

		if (_xyzzy5 == 1116)
			_glassBottomWithItems2 = series_place_sprite(
				"407 FULL GLASS POPUP WITH ITEMS", 4, 0, 0, 100, 0x100);

		if (_val6 == 1010)
			_glassTopPopupWithItems1 = series_place_sprite(
				"407 FULL GLASS POPUP WITH ITEMS", 6, 0, 0, 100, 0x100);

		disableHotspots();
		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

bool Room407::lookEmeraldCork() {
	if (_xyzzy7 == 1112) {
		switch (_G(kernel).trigger) {
		case -1:
			glassBottomPopup();

			if (_frotz6) {
				digi_play("407r17", 1);
			} else {
				_frotz6 = 1;
				digi_play("407r17", 1, 255, 1);
			}
			return true;

		case 1:
			digi_play("407r17a", 1);
			return true;

		default:
			break;
		}
	}

	return false;
}

void Room407::periodicTablePopup() {
	if (_G(kernel).trigger == -1) {
		_val15 = 1030;
		_glassTopPopupWithItems2 = series_place_sprite(
			"Large periodic table", 0, 0, 0, 100, 0);
		disableHotspots();
		hotspot_set_active(" ", true);
		player_set_commands_allowed(true);
	}
}

bool Room407::lookLeverKey() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_frotz9) {
			digi_play("407r26a", 1);
		} else {
			digi_play("407r26", 1, 255, 2);
			_frotz9 = 1;
		}

		return true;

	case 2:
		digi_play("407r26a", 1);
		return true;

	default:
		break;
	}

	return false;
}

void Room407::gardenHoseSurgicalTube() {
	switch (_G(kernel).trigger) {
	case -1:
		ws_walk(170, 335, nullptr, 0, 11, 1);
		break;
	case 0:
		if (_val4 == 1010) {
			digi_play("407r99e", 1);
		} else {
			player_set_commands_allowed(false);
			_ripHiHand1 = series_load("rip trek hi 1 hand");
			setGlobals1(_ripHiHand1, 1, 5, 5, 5, 0, 5, 1, 1, 1);
			sendWSMessage_110000(1);
		}
		break;

	case 1:
		_drawerPopupHose = series_place_sprite("407 TUBE AND HOSE INTO SINK",
			1, 0, 0, 100, 0xe00);
		hotspot_set_active("GARDEN HOSE    ", true);
		inv_move_object("GARDEN HOSE", 407);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy2 = 1100;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::gardenHoseSurgicalTube2() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_val4 == 1010)
			digi_play("407r99e", 1);
		else
			reachHand(10);
		break;

	case 1:
		_drawerPopupHose = series_place_sprite(
			"407 TUBE AND HOSE INTO SINK", 0, 0, 0, 100, 0xe00);
		hotspot_set_active("GARDEN HOSE  ", true);

		if (_xyzzy2 == 1061) {
			inv_move_object("GARDEN HOSE", 407);
		} else {
			_faucet1 = series_place_sprite("407 FAUCET IN SINK",
				0, 0, 0, 100, 0xe00);
			hotspot_set_active("FAUCET PIPE  ", true);
			inv_move_object("FAUCET PIPE/HOSE", 407);
		}

		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		if (_xyzzy2 == 1061)
			_val8 = 1130;

		_xyzzy2 = 1130;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::surgicalTubeFaucetPipe() {
	switch (_G(kernel).trigger) {
	case -1:
	case 0:
		if (_val4 == 1010) {
			digi_play("407r99e", 1);
		} else {
			player_set_commands_allowed(false);
			_ripHiHand1 = series_load("rip trek hi 1 hand");
			setGlobals1(_ripHiHand1, 1, 6, 6, 6, 0, 6, 1, 1, 1);
			sendWSMessage_110000(1);
		}
		break;

	case 1:
		inv_move_object("SURGICAL TUBE", 407);
		_tubeInDrawer = series_place_sprite("407 TUBING BY ITSELF",
			1, 0, 0, 100, 0xe00);
		hotspot_set_active("SURGICAL TUBE   ", true);

		if (_xyzzy3 == 1111) {
			_drawerPopupHose = series_place_sprite("407 TUBE AND HOSE INTO SINK",
				1, 0, 0, 100, 0xe00);
			hotspot_set_active("GARDEN HOSE    ", true);
			inv_move_object("TUBE/HOSE", 407);
			_xyzzy2 = 1100;
		}

		digi_play("407_s06", 2, 255, 2);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy3 = 1100;
		player_set_commands_allowed(true);
		break;

	case 777:
		ws_walk(170, 335, nullptr, 0, 11);
		break;

	default:
		break;
	}
}

void Room407::surgicalTubeStem() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		if (_xyzzy3 != 1111) {
			inv_move_object("SURGICAL TUBE", 407);
		} else if (_xyzzy2 == 1060) {
			_drawerPopupHose = series_place_sprite(
				"407 TUBE AND HOSE INTO SINK", 0, 0, 0, 100, 0xe00);
			hotspot_set_active("GARDEN HOSE  ", true);
			inv_move_object("TUBE/HOSE", 407);
			_xyzzy2 = 1130;
		} else if (_xyzzy2 == 1062) {
			_drawerPopupHose = series_place_sprite(
				"407 TUBE AND HOSE INTO SINK", 0, 0, 0, 100, 0xe00);
			hotspot_set_active("GARDEN HOSE  ", true);
			_faucet1 = series_place_sprite("407 FAUCET IN SINK", 0, 0, 0, 100, 0xe00);
			hotspot_set_active("FAUCET PIPE  ", true);
			inv_move_object("FAUCET PIPE/HOSE/TUBE", 407);
			_val8 = 1130;
			_xyzzy2 = 1130;
		}

		_tubeInDrawer = series_place_sprite("407 TUBING BY ITSELF",
			0, 0, 0, 100, 0xe00);
		hotspot_set_active("SURGICAL TUBE  ", true);
		digi_play("407_s06", 2, 255, 2);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::reachHand(int frame) {
	player_set_commands_allowed(false);
	_ripMedHand1 = series_load("RIP TREK MED REACH HAND POS1");
	setGlobals1(_ripMedHand1, 1, frame, frame, frame, 0, frame, 1, 1, 1);
	sendWSMessage_110000(1);
}

void Room407::reachLeverKey() {
	switch (_G(kernel).trigger) {
	case -1:
	case 0:
		player_set_commands_allowed(false);
		_ripHiHand2 = series_load("rip trek hi reach 2hnd");
		setGlobals1(_ripHiHand2, 1, 13, 13, 13, 0, 13, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		inv_move_object("GLASS JAR", 407);
		hotspot_set_active("GLASS JAR ", true);
		_bottle = series_place_sprite("407BOTLR", 0, 0, -53, 100, 0xe00);

		switch (_xyzzy7) {
		case 1050:
			_xyzzy5 = 1116;
			hotspot_set_active("JAR/RUBBER PLUG ", true);
			inv_move_object("JAR/RUBBER PLUG", 407);
			_stopperInDrawer = series_place_sprite(
				"407BITSR", 0, 0, -53, 100, 0xb00);
			break;

		case 1051:
			_xyzzy9 = 1116;
			hotspot_set_active("JAR/CORK ", true);
			inv_move_object("JAR/CORK", 407);
			_bits = series_place_sprite("407BITSR", 3, 0, -53, 100, 0xb00);
			break;

		case 1052:
			_xyzzy4 = 1116;
			hotspot_set_active("JAR/GRIPS ", true);
			inv_move_object("JAR/GRIPS", 407);
			_bits = series_place_sprite("407BITSR", 2, 0, -53, 100, 0xb00);
			break;

		case 1053:
			_xyzzy9 = 1116;
			_xyzzy5 = 1116;
			hotspot_set_active("JAR/RUBBER PLUG ", true);
			hotspot_set_active("JAR/CORK ", true);
			inv_move_object("JAR/CORK/PLUG", 407);
			_stopperInDrawer = series_place_sprite("407BITSR", 3, 0, -53, 100, 0xb00);
			_bits = series_place_sprite("407BITSR", 3, 0, -53, 100, 0xb00);
			break;

		case 1054:
			_xyzzy4 = 1116;
			_xyzzy5 = 1116;
			hotspot_set_active("JAR/RUBBER PLUG ", true);
			hotspot_set_active("JAR/GRIPS ", true);
			inv_move_object("JAR/PLUG/GRIPS", 407);
			_stopperInDrawer = series_place_sprite("407BITSR", 0, 0, -53, 100, 0xb00);
			_bits = series_place_sprite("407BITSR", 2, 0, -53, 100, 0xb00);
			break;

		case 1055:
			_xyzzy9 = 1116;
			_xyzzy4 = 1116;
			hotspot_set_active("JAR/CORK ", true);
			hotspot_set_active("JAR/GRIPS ", true);
			inv_move_object("JAR/CORK/GRIPS", 407);
			_bits = series_place_sprite("407BITSR", 3, 0, -53, 100, 0xe00);
			_handleInDrawer = series_place_sprite("407BITSR", 2, 0, -53, 100, 0xb00);
			break;

		case 1056:
			_xyzzy9 = 1116;
			_xyzzy4 = 1116;
			_xyzzy5 = 1116;
			hotspot_set_active("JAR/RUBBER PLUG ", true);
			hotspot_set_active("JAR/CORK ", true);
			hotspot_set_active("JAR/GRIPS ", true);
			inv_move_object("JAR/PLUG/CORK/GRIPS", 407);
			_stopperInDrawer = series_place_sprite("407BITSR", 0, 0, -53, 100, 0xb00);
			_bits = series_place_sprite("407BITSR", 2, 0, -53, 100, 0xb00);
			break;

		default:
			break;
		}

		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand2);
		_xyzzy7 = 1114;
		player_set_commands_allowed(true);
		break;

	case 777:
		player_set_commands_allowed(false);
		ws_walk(463, 323, nullptr, 0, 1);
		break;

	default:
		break;
	}
}

void Room407::faucetPipeGardenHose() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_val4 == 1010)
			digi_play("407r99e", 1);
		else
			reachHand(10);
		break;

	case 1:
		_faucet1 = series_place_sprite("407 FAUCET IN SINK",
			0, 0, 0, 100, 0xe00);
		hotspot_set_active("FAUCET PIPE  ", true);
		inv_move_object("FAUCET PIPE", 407);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_val8 = 1130;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::faucetPipeGlassJar() {
	switch (_G(kernel).trigger) {
	case 1:
		if (_val8 != 1111) {
			inv_move_object("FAUCET PIPE", 407);
		} else if (_xyzzy2 == 1061) {
			_drawerPopupHose = series_place_sprite(
				"407 HOSE HANG FROM JAR", 0, 0, 0, 100, 0xb00);
			hotspot_set_active("GARDEN HOSE ", true);
			inv_move_object("FAUCET PIPE/HOSE", 407);
			_xyzzy2 = 1116;
		} else if (_xyzzy2 == 1062) {
			_drawerPopupHose = series_place_sprite(
				"407 HOSE HANG FROM JAR", 0, 0, 0, 100, 0xb00);
			hotspot_set_active("GARDEN HOSE ", true);
			_tubeInDrawer = series_place_sprite("407 HOSE HANG FROM JAR",
				1, 0, 0, 100, 0xb00);
			hotspot_set_active("SURGICAL TUBE ", true);
			inv_move_object("FAUCET PIPE/HOSE/TUBE", 407);
			_xyzzy3 = 1116;
			_xyzzy2 = 1116;
		}

		_faucet1 = series_place_sprite("407BITSR", 1, 0, -53, 100, 0xb00);
		hotspot_set_active("FAUCET PIPE ", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_val8 = 1116;
		player_set_commands_allowed(true);
		break;

	case 70:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 12, 12, 12, 0, 12, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 777:
		ws_walk(436, 331, nullptr, 70, 1);
		break;

	default:
		break;
	}
}

void Room407::gardenHoseFaucetPipe() {
	switch (_G(kernel).trigger) {
	case 1:
		_drawerPopupHose = series_place_sprite(
			"407 HOSE HANG FROM JAR", 0, 0, 0, 100, 0xb00);
		hotspot_set_active("GARDEN HOSE ", true);

		if (_xyzzy2 == 1060) {
			_tubeInDrawer = series_place_sprite(
				"407 HOSE HANG FROM JAR", 1, 0, 0, 100, 0xb00);
			inv_move_object("TUBE/HOSE", 407);
			hotspot_set_active("SURGICAL TUBE ", true);
		} else {
			inv_move_object("GARDEN HOSE", 407);
		}

		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		if (_xyzzy2 == 1060)
			_xyzzy3 = 1116;

		_xyzzy2 = 1116;
		player_set_commands_allowed(true);
		break;

	case 70:
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 12, 12, 12, 0, 12, 1, 1, 1);
		break;

	case 777:
		ws_walk(436, 331, nullptr, 70, 1);
		break;

	default:
		break;
	}
}

void Room407::surgicalTubeGardenHose() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripLowHand1 = series_load("RIP LOW REACH POS1");
		setGlobals1(_ripLowHand1, 1, 11, 11, 11, 0, 11, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		_tubeInDrawer = series_place_sprite("407 HOSE HANG FROM JAR", 1, 0, 0, 100, 0xb00);
		inv_move_object("SURGICAL TUBE", 407);
		hotspot_set_active("SURGICAL TUBE ", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripLowHand1);
		_xyzzy3 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::emeraldIronSupport() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand2 = series_load("rip trek hi reach 2hnd");
		setGlobals1(_ripHiHand2, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_bottle);
		_gears = series_place_sprite("407GEARS", 0, 0, -53, 100, 0xf00);
		_bottle = series_place_sprite("407BOTL2", 0, 0, -53, 100, 0xe00);
		digi_play("407_s29", 2);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand2);
		_val6 = 1010;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::placeLeverKey() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_placeLeverKey = series_load("407 PLACE LEVER KEY");
		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP levers ceiling");
		sendWSMessage_10000(1, _ripley, _placeLeverKey, 1, 42, 2,
			_placeLeverKey, 42, 42, 0);
		break;

	case 2:
		digi_play("407_s07a", 1);
		sendWSMessage_10000(1, _ripley, _placeLeverKey, 42, 70, 3,
			_placeLeverKey, 70, 70, 0);
		break;

	case 3:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		_pump = series_place_sprite("407PMROD", 0, 0, -53, 100, 0xf00);
		_lever = series_place_sprite("407LEVRW", 1, 0, -53, 100, 0xf00);
		hotspot_set_active("LEVER KEY  ", true);
		hotspot_set_active("PUMP ROD ", true);
		inv_move_object("LEVER KEY/PUMP ROD", 407);
		_val10 = 1114;
		_xyzzy1 = 1114;
		_xyzzy10 = 1040;
		series_unload(_placeLeverKey);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::rubberPlugGlassJar() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		_stopperInDrawer = series_place_sprite("407bbits", 2, 0, -53, 100, 0xe00);
		inv_move_object("RUBBER PLUG", 407);
		hotspot_set_active("JAR/RUBBER PLUG", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_xyzzy5 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::rubberPlugGlassJar2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 6, 6, 6, 0, 6, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		_stopperInDrawer = series_place_sprite("407BITSR", 0, 0, -53, 100, 0xe00);
		inv_move_object("RUBBER PLUG", 407);
		hotspot_set_active("JAR/RUBBER PLUG ", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy5 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::corkGlassJar() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		_stopperInDrawer = series_place_sprite("407bbits", 0, 0, -53, 100, 0xe00);
		inv_move_object("CORK", 407);
		hotspot_set_active("JAR/CORK", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_xyzzy5 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::corkGlassJar2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		_stopperInDrawer = series_place_sprite("407BITSR", 3, 0, -53, 100, 0xe00);
		inv_move_object("CORK", 407);
		hotspot_set_active("JAR/CORK ", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy5 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::pumpGripsGlassJar() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 7, 7, 7, 0, 7, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		_handleInDrawer = series_place_sprite("407BITSR", 2, 0, -53, 100, 0xe00);
		inv_move_object("PUMP GRIPS", 407);
		hotspot_set_active("JAR/GRIPS ", true);
		sendWSMessage_110000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy4 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::periodicTableGlassJar() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("EMERALD/CORK")) {
			player_set_commands_allowed(false);
			_rollStuff = series_load("407 ROLL STUFF");
			player_update_info();
			_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
				_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
			ws_hide_walker();

			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "RIP folds table into jar");
			sendWSMessage_10000(1, _ripley, _rollStuff, 1, 90, 2,
				_rollStuff, 90, 90, 0);
			kernel_timing_trigger(130, 1);
		} else {
			digi_play("com118", 1);
		}
		break;

	case 1:
		digi_play("407_s14", 2);
		break;

	case 2:
		sendWSMessage_10000(1, _ripley, _rollStuff, 91, 109, 3,
			_rollStuff, 109, 109, 0);
		break;

	case 3:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		_chart = series_place_sprite("407bbits", 1, 0, -53, 100, 0xe00);
		series_unload(_rollStuff);
		inv_move_object("PERIODIC TABLE", 407);
		hotspot_set_active("PERIODIC TABLE/JAR", true);
		_xyzzy6 = 1116;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::surgicalTubeNozzles() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		_tubeInDrawer = series_place_sprite("407surgn", 0, 0, -53, 100, 0xa00);
		inv_move_object("SURGICAL TUBE", 407);
		hotspot_set_active("NOZZLES/TUBE", true);
		hotspot_set_active("NOZZLES", false);
		digi_play("407_s06", 2, 255, 2);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		digi_play("407r75", 1);
		_xyzzy3 = 1117;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::leverKeyTablePivot() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		_lever = series_place_sprite("407LEVRW", 2, 0, -53, 100, 0xb00);
		inv_move_object("LEVER KEY", 407);
		hotspot_set_active("LEVER KEY ", true);
		digi_play("407_s07a", 2);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_val10 = 1113;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::pumpRodBracket() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_407rp99 = series_load("407RP99");
		player_update_info();
		_safariShadow = series_place_sprite(
			"SAFARI SHADOW 1", 0, _G(player_info).x,
			_G(player_info).y, _G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP levers ceiling");
		sendWSMessage_10000(1, _ripley, _407rp99, 1, 35, 2,
			_407rp99, 35, 35, 0);
		break;

	case 2:
		digi_play("407_s07a", 1);
		sendWSMessage_10000(1, _ripley, _407rp99, 36, 61, 3,
			_407rp99, 61, 61, 0);
		break;

	case 3:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		_pump = series_place_sprite("407PMROD", 0, 0, -53, 100, 0xf00);

		hotspot_set_active("PUMP ROD ", true);
		inv_move_object("PUMP ROD", 407);
		_xyzzy1 = 1114;

		series_unload(_407rp99);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::leverKey1() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_407rp98 = series_load("407RP98");
		terminateMachineAndNull(_pump);
		player_update_info();
		_safariShadow = series_place_sprite(
			"SAFARI SHADOW 1", 0, _G(player_info).x,
			_G(player_info).y, _G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP levers ceiling");
		sendWSMessage_10000(1, _ripley, _407rp98, 1, 53, 2,
			_407rp98, 53, 53, 0);
		break;

	case 2:
		digi_play("407_s07a", 1);
		sendWSMessage_10000(1, _ripley, _407rp98, 54, 81, 3,
			_407rp98, 81, 81, 0);
		break;

	case 3:
		digi_play("407_s07a", 1);
		sendWSMessage_10000(1, _ripley, _407rp98, 81, 101, 4,
			_407rp98, 101, 101, 0);
		break;

	case 4:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		_pump = series_place_sprite("407PMROD", 0, 0, -53, 100, 0xf00);
		_lever = series_place_sprite("407LEVRW", 1, 0, -53, 100, 0xf00);
		hotspot_set_active("LEVER KEY  ", true);
		inv_move_object("LEVER KEY", 407);
		_val10 = 1114;

		series_unload(_407rp98);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::leverKey2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_placeLeverKey = series_load("407 PLACE LEVER KEY");
		player_update_info();
		_safariShadow = series_place_sprite(
			"SAFARI SHADOW 1", 0, _G(player_info).x,
			_G(player_info).y, _G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP levers ceiling");
		sendWSMessage_10000(1, _ripley, _placeLeverKey, 1, 30, 1,
			_placeLeverKey, 30, 30, 0);
		break;

	case 1:
		digi_play("com105", 1, 255, 2, 997);
		break;

	case 2:
		sendWSMessage_10000(1, _ripley, _placeLeverKey, 32, 1, 3,
			_placeLeverKey, 1, 1, 0);
		break;

	case 3:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		series_unload(_placeLeverKey);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::faucetHandleAirValve() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(4);
		break;

	case 1:
		digi_play("407_s13", 2);
		_faucet2 = series_place_sprite("407 AIR VALVE HANDLE",
			(_val3 == 1011) ? 1 : 0, 0, 0, 100, 0xe00);

		inv_move_object("FAUCET HANDLE", 407);
		hotspot_set_active("AIR VALVE/HANDLE", true);
		hotspot_set_active("AIR VALVE", false);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		digi_play("407r73", 1);
		_val9 = 1110;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::faucetPipeFaucetHandle1() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		digi_play("407_s04", 2);
		_faucet2 = series_place_sprite("407FAUC",
			(_val3 == 1011) ? 1 : 0, 0, 0, 100, 0xe00);

		inv_move_object("FAUCET HANDLE", 407);
		hotspot_set_active("FAUCET HANDLE", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_val9 = 1100;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::faucetPipeFaucetHandle2() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		_faucet1 = series_place_sprite("407FAUC", 2, 0, 0, 100, 0xe00);
		inv_move_object("FAUCET PIPE", 407);
		hotspot_set_active("FAUCET PIPE", true);

		if (_xyzzy3 == 1061) {
			_tubeInDrawer = series_place_sprite("407 TUBING BY ITSELF",
				1, 0, 0, 100, 0xb00);
			hotspot_set_active("SURGICAL TUBE   ", true);
			_xyzzy3 = 1100;
			inv_move_object("FAUCET PIPE/TUBE", 407);
		}

		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_val8 = 1100;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::pumpRodPump() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_pump407 = series_load("407 PUMP");
		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y,
			_G(player_info).scale, 0xf00);

		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP pumps");
		sendWSMessage_10000(1, _ripley, _pump407, 1, 40, 1,
			_pump407, 40, 40, 0);
		kernel_timing_trigger(150, 69);
		break;

	case 1:
		_pump = series_place_sprite("407pump", 0, 0, -53, 100, 0xf00);
		inv_move_object("PUMP ROD", 407);
		hotspot_set_active("PUMP ROD", true);
		sendWSMessage_10000(1, _ripley, _pump407, 40, 1, 4,
			_pump407, 1, 1, 0);
		break;

	case 4:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		series_unload(_pump407);
		_xyzzy1 = 1115;
		player_set_commands_allowed(true);
		break;

	case 69:
		digi_play("407_s07", 2);
		break;

	default:
		break;
	}
}

void Room407::pumpGripsPump() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_pump407 = series_load("407 PUMP");
		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y,
			_G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP pumps");
		sendWSMessage_10000(1, _ripley, _pump407, 1, 40, 1,
			_pump407, 40, 40, 0);
		kernel_timing_trigger(180, 69);
		break;

	case 1:
		_handleInDrawer = series_place_sprite("407pump", 1, 0, -53, 100, 0xe00);
		inv_move_object("PUMP GRIPS", 407);
		hotspot_set_active("PUMP GRIPS ", true);
		sendWSMessage_10000(1, _ripley, _pump407, 40, 1, 4,
			_pump407, 1, 1, 0);
		break;

	case 4:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		series_unload(_pump407);
		_xyzzy4 = 1115;
		player_set_commands_allowed(true);
		break;

	case 69:
		digi_play("407_s07", 2);
		break;

	default:
		break;
	}
}

void Room407::useButton1() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		sendWSMessage_10000(1, _lockButton1,
			_407pu08a, _buttonFrame1 * 2 + 2,
			_buttonFrame1 * 2 + 2, 1,
			_407pu08a, _buttonFrame1 * 2 + 2,
			_buttonFrame1 * 2 + 2, 0);
		digi_play("407_s26", 2);
		break;

	case 1:
		if (++_buttonFrame1 == 10)
			_buttonFrame1 = 0;

		sendWSMessage_10000(1, _lockButton1,
			_407pu08a, _buttonFrame1 * 2 + 1,
			_buttonFrame1 * 2 + 1, 2,
			_407pu08a, _buttonFrame1 * 2 + 1,
			_buttonFrame1 * 2 + 1, 0);
		break;

	case 2:
		if (_buttonFrame1 == 3)
			checkCode();

		if (_codeCorrect) {
			kernel_timing_trigger(1, 360, KT_DAEMON, KT_PARSE);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	default:
		break;
	}
}

void Room407::useButton2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		sendWSMessage_10000(1, _lockButton2,
			_407pu08b, _buttonFrame2 * 2 + 2,
			_buttonFrame2 * 2 + 2, 1,
			_407pu08b, _buttonFrame2 * 2 + 2,
			_buttonFrame2 * 2 + 2, 0);
		digi_play("407_s26", 2);
		break;

	case 1:
		if (++_buttonFrame2 == 10)
			_buttonFrame2 = 0;

		sendWSMessage_10000(1, _lockButton2,
			_407pu08b, _buttonFrame2 * 2 + 1,
			_buttonFrame2 * 2 + 1, 2,
			_407pu08b, _buttonFrame2 * 2 + 1,
			_buttonFrame2 * 2 + 1, 0);
		break;

	case 2:
		if (_buttonFrame2 == 4)
			checkCode();

		if (_codeCorrect) {
			kernel_timing_trigger(1, 360, KT_DAEMON, KT_PARSE);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	default:
		break;
	}
}

void Room407::useButton3() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		sendWSMessage_10000(1, _lockButton3,
			_407pu08c, _buttonFrame3 * 2 + 2,
			_buttonFrame3 * 2 + 2, 1,
			_407pu08c, _buttonFrame3 * 2 + 2,
			_buttonFrame3 * 2 + 2, 0);
		digi_play("407_s26", 2);
		break;

	case 1:
		if (++_buttonFrame3 == 1)
			_buttonFrame3 = 0;

		sendWSMessage_10000(1, _lockButton3,
			_407pu08c, _buttonFrame3 * 2 + 1,
			_buttonFrame3 * 2 + 1, 2,
			_407pu08c, _buttonFrame3 * 2 + 1,
			_buttonFrame3 * 2 + 1, 0);
		break;

	case 2:
		if (_buttonFrame3 == 4)
			checkCode();

		if (_codeCorrect) {
			kernel_timing_trigger(1, 360, KT_DAEMON, KT_PARSE);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	default:
		break;
	}
}

void Room407::useButton4() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		sendWSMessage_10000(1, _lockButton4,
			_407pu08d, _buttonFrame4 * 2 + 2,
			_buttonFrame4 * 2 + 2, 1,
			_407pu08d, _buttonFrame4 * 2 + 2,
			_buttonFrame4 * 2 + 2, 0);
		digi_play("407_s26", 2);
		break;

	case 1:
		if (++_buttonFrame4 == 10)
			_buttonFrame4 = 0;

		sendWSMessage_10000(1, _lockButton4,
			_407pu08d, _buttonFrame4 * 2 + 1,
			_buttonFrame4 * 2 + 1, 2,
			_407pu08d, _buttonFrame4 * 2 + 1,
			_buttonFrame4 * 2 + 1, 0);
		break;

	case 2:
		if (_buttonFrame4 == 1)
			checkCode();

		if (_codeCorrect) {
			kernel_timing_trigger(1, 360, KT_DAEMON, KT_PARSE);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	default:
		break;
	}
}

void Room407::checkCode() {
	if (_buttonFrame1 == 3 && _buttonFrame2 == 4 &&
		_buttonFrame3 == 1 && _buttonFrame4 == 1)
		_codeCorrect = true;
}

void Room407::useButtons() {
	if (_G(kernel).trigger == -1) {
		player_set_commands_allowed(false);
		_int7 = 1030;
		_comboLockPopup = series_place_sprite("407 COMBINATION LOCK POPUP",
			0, 0, 0, 100, 0x200);
		disableHotspots();
		hotspot_set_active(" ", true);

		_407pu08a = series_load("407 COMBINATION LOCK NUMERALS");
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

		int frame1 = _buttonFrame1 * 2 + 1;
		int frame2 = _buttonFrame2 * 2 + 1;
		int frame3 = _buttonFrame3 * 2 + 1;
		int frame4 = _buttonFrame4 * 2 + 1;
		sendWSMessage_10000(1, _lockButton1, _407pu08a, frame1, frame1, -1,
			_407pu08a, frame1, frame1, 0);
		sendWSMessage_10000(1, _lockButton2, _407pu08b, frame2, frame2, -1,
			_407pu08b, frame2, frame2, 0);
		sendWSMessage_10000(1, _lockButton1, _407pu08c, frame3, frame3, -1,
			_407pu08c, frame3, frame3, 0);
		sendWSMessage_10000(1, _lockButton1, _407pu08d, frame4, frame4, -1,
			_407pu08d, frame4, frame4, 0);

		hotspot_set_active("BUTTON", true);
		hotspot_set_active("BUTTON ", true);
		hotspot_set_active("BUTTON  ", true);
		hotspot_set_active("BUTTON   ", true);
		player_set_commands_allowed(true);
	}
}

void Room407::useLeverKey() {
	switch (_G(kernel).trigger) {
	case 1:
		_hangRip = series_load("407 HANG RIP");
		terminateMachineAndNull(_niche);
		terminateMachineAndNull(_cpist);
		terminateMachineAndNull(_lever);

		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP hangs from lever");
		sendWSMessage_10000(1, _ripley, _hangRip, 1, 25, 2,
			_hangRip, 25, 25, 0);
		break;

	case 2:
		sendWSMessage_10000(1, _ripley, _hangRip, 25, 63, 3,
			_hangRip, 63, 63, 0);
		digi_play("407_s23", 2);
		break;

	case 3:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();

		_niche = series_place_sprite("407NICH", 0, 0, -53, 100, 0xf00);
		_cpist = series_place_sprite("407CPIST", 0, 0, -53, 100, 0xf00);
		_lever = series_place_sprite("407LEVRW", 1, 0, -53, 100, 0xf00);
		series_unload(_hangRip);
		player_set_commands_allowed(true);
		break;

	case 777:
		player_set_commands_allowed(false);
		ws_walk(466, 330, nullptr, 1, 1);
		break;

	default:
		break;
	}
}

void Room407::useFaucet() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		digi_play("407_s04", 2);
		terminateMachineAndNull(_faucet2);

		if (_val4 != 1010) {
			_val4 = 1010;
			_faucet2 = series_place_sprite("407FAUC", 0, 0, 0, 100, 0xe00);
		} else {
			_val4 = 1011;
			_faucet2 = series_place_sprite("407FAUC", 1, 0, 0, 100, 0xe00);

			if (_val8 == 1100) {
				kernel_timing_trigger(1, 430, KT_DAEMON, KT_PARSE);
			} else if (_xyzzy3 == 1130) {
				kernel_timing_trigger(1, 440, KT_DAEMON, KT_PARSE);
			}
		}

		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);

		if (_frotz1 && _val4 == 1010)
			kernel_timing_trigger(1, 320, KT_DAEMON, KT_PARSE);
		else if (_val4 == 1010 && _val8 == 1100)
			kernel_timing_trigger(1, 410, KT_DAEMON, KT_PARSE);
		else if (_xyzzy3 == 1130)
			kernel_timing_trigger(1, 420, KT_DAEMON, KT_PARSE);
		else
			player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::useValveHandle() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(4);
		break;

	case 1:
		digi_play("407_s13", 2);
		terminateMachineAndNull(_faucet2);

		if (_val3 != 1011) {
			_val3 = 1011;
			_faucet2 = series_place_sprite("407 AIR VALVE HANDLE", 1, 0, 0, 100, 0xe00);
			digi_play("407r74a", 1);
		} else {
			_val3 = 1010;
			_faucet2 = series_place_sprite("407 AIR VALVE HANDLE", 0, 0, 0, 100, 0xe00);

			if (_val7 == 1021) {
				_val7 = 1020;

				if (_xyzzy3 == 1117) {
					_frotz3 = 1;
					kernel_timing_trigger(1, 20, KT_DAEMON, KT_PARSE);
				} else {
					digi_play("407_s09", 2);
					digi_play("407r76", 1);
				}
			} else {
				digi_play("407r74", 1);
			}
		}

		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);

		if (_frotz3)
			_frotz3 = 0;
		else
			player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

bool Room407::usePump() {
	if (_val7 != 1020) {
		digi_play("407r70", 1);
	} else if (_xyzzy1 == 1115 && _xyzzy4 == 1115) {
		if (inv_object_is_here("EMERALD/CORK") || _xyzzy7 != 1112 ||
				_val6 != 1010) {
			usePump2();
		} else {
			digi_play("com090a", 1);
		}
	} else if (_xyzzy1 == 1115) {
		digi_play("407r72", 1);
	} else {
		return false;
	}

	return true;
}

void Room407::usePump2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_pump407 = series_load("407 PUMP");

		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y,
			_G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP pumps");
		sendWSMessage_10000(1, _ripley, _pump407, 81, 62, 1,
			_pump407, 62, 62, 0);
		_val11 = 1;
		break;

	case 1:
		digi_play("407_s11", 2);
		sendWSMessage_10000(1, _ripley, _pump407, 62, 46, 2,
			_pump407, 46, 46, 0);
		break;

	case 2:
		sendWSMessage_10000(1, _ripley, _pump407, 46, 66, 3,
			_pump407, 66, 66, 0);
		break;

	case 3:
		sendWSMessage_10000(1, _ripley, _pump407, 67, 81, 4,
			_pump407, 81, 81, 0);
		break;

	case 4:
		_handleInDrawer = series_place_sprite("407pump", 1, 0, -53, 100, 0xf00);
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		series_unload(_pump407);

		if (_val3 == 1011) {
			_val7 = 1021;
			digi_play("407r69", 1);
		} else {
			digi_play("407r71", 1);
		}

		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeLetter() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("MENENDEZ'S LETTER")) {
			player_set_commands_allowed(false);
			_ripMedHand1 = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripMedHand1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(11);
		}
		break;

	case 8:
		series_unload(_ripMedHand1);
		break;

	case 11:
		terminateMachineAndNull(_letter);
		hotspot_set_active("LETTER", false);
		hotspot_set_active("BUTTONS", true);
		digi_play("407r39", 1, 255, 13);
		break;

	case 13:
		kernel_examine_inventory_object("PING MENENDEZ'S LETTER",
			_G(master_palette), 5, 1, 270, 150, 14, "407r41", -1);
		_G(flags)[GLB_TEMP_11] = 1;
		_G(flags)[V280] = 1;
		break;

	case 14:
		digi_stop(1);
		inv_give_to_player("MENENDEZ'S LETTER");
		_frotz4 = 1;
		_G(flags)[V370] = 1;
		sendWSMessage_120000(15);
		break;

	case 15:
		sendWSMessage_150000(8);
		break;

	default:
		break;
	}
}

void Room407::takeFaucetPipe1() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_faucet1);
		inv_give_to_player("FAUCET PIPE");
		hotspot_set_active("FAUCET PIPE  ", false);
		kernel_examine_inventory_object("PING FAUCET PIPE",
			_G(master_palette), 5, 1, 100, 200, 3, nullptr, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_val8 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeFaucetPipe2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 12, 12, 12, 0, 12, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_faucet1);
		hotspot_set_active("FAUCET PIPE ", false);
		inv_give_to_player("FAUCET PIPE");

		if (_xyzzy3 == 1116) {
			terminateMachineAndNull(_tubeInDrawer);
			hotspot_set_active("SURGICAL TUBE ", false);
			inv_give_to_player("SURGICAL TUBE");
			_xyzzy3 = 1000;

			terminateMachineAndNull(_drawerPopupHose);
			hotspot_set_active("GARDEN HOSE ", false);
			inv_give_to_player("GARDEN HOSE");
			_xyzzy2 = 1000;

			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE/TUBE",
				_G(master_palette), 5, 1, 400, 150, 3, 0, -1);
		} else if (_xyzzy2 == 1116) {
			terminateMachineAndNull(_drawerPopupHose);
			inv_give_to_player("GARDEN HOSE");
			hotspot_set_active("GARDEN HOSE ", false);
			_xyzzy2 = 1000;

			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE",
				_G(master_palette), 5, 1, 400, 150, 3, 0, -1);
		} else {
			kernel_examine_inventory_object("PING FAUCET PIPE",
				_G(master_palette), 5, 1, 400, 150, 3, 0, -1);
		}
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand1);
		_val8 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeFaucetPipe3() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_val4 == 1010)
			digi_play("407r99e", 1);
		else
			reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_faucet1);
		inv_give_to_player("FAUCET PIPE");
		hotspot_set_active("FAUCET PIPE", false);

		if (_xyzzy2 == 1100) {
			terminateMachineAndNull(_tubeInDrawer);
			inv_give_to_player("GARDEN HOSE");
			hotspot_set_active("GARDEN HOSE    ", false);
			_xyzzy2 = 1000;

			terminateMachineAndNull(_tubeInDrawer);
			hotspot_set_active("SURGICAL TUBE   ", false);
			inv_give_to_player("SURGICAL TUBE");
			_xyzzy3 = 1000;

			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE/TUBE",
				_G(master_palette), 5, 1, 75, 200, 2, 0, -1);
		} else if (_xyzzy3 == 1100) {
			terminateMachineAndNull(_tubeInDrawer);
			hotspot_set_active("SURGICAL TUBE   ", false);
			inv_give_to_player("SURGICAL TUBE");
			_xyzzy3 = 1000;

			kernel_examine_inventory_object("PING FAUCET PIPE/TUBE",
				_G(master_palette), 5, 1, 75, 200, 2, 0, -1);
		} else {
			kernel_examine_inventory_object("PING FAUCET PIPE",
				_G(master_palette), 5, 1, 75, 200, 2, 0, -1);
		}
		break;

	case 2:
		sendWSMessage_120000(4);
		break;

	case 3:
		sendWSMessage_150000(5);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_val8 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeSurgicalTube1() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_val4 == 1010) {
			digi_play("407r99e", 1);
		} else {
			player_set_commands_allowed(false);
			_ripHiHand1 = series_load("rip trek hi 1 hand");
			setGlobals1(_ripHiHand1, 1, 6, 6, 6, 0, 6, 1, 1, 1);
			sendWSMessage_110000(1);
		}
		break;

	case 1:
		terminateMachineAndNull(_tubeInDrawer);
		hotspot_set_active("SURGICAL TUBE   ", false);
		inv_give_to_player("SURGICAL TUBE");

		if (_xyzzy2 == 1100) {
			terminateMachineAndNull(_drawerPopupHose);
			inv_give_to_player("GARDEN HOSE");
			hotspot_set_active("GARDEN HOSE    ", false);
			_xyzzy2 = 1000;
			kernel_examine_inventory_object("PING TUBE/HOSE",
				_G(master_palette), 5, 1, 75, 200, 2, "407_s06", -1);
		} else {
			kernel_examine_inventory_object("PING SURGICAL TUBE",
				_G(master_palette), 5, 1, 75, 200, 2, "407_s06", -1);
		}
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy3 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeSurgicalTube2() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_val4 == 1010) {
			digi_play("407r99e", 1);
		} else {
			reachHand(10);
		}
		break;

	case 1:
		terminateMachineAndNull(_tubeInDrawer);
		hotspot_set_active("SURGICAL TUBE  ", false);
		inv_give_to_player("SURGICAL TUBE");

		if (_val8 == 1130) {
			terminateMachineAndNull(_faucet1);
			hotspot_set_active("GARDEN HOSE  ", false);
			inv_give_to_player("FAUCET PIPE");
			_val8 = 1000;

			terminateMachineAndNull(_drawerPopupHose);
			hotspot_set_active("GARDEN HOSE  ", false);
			inv_give_to_player("GARDEN HOSE");
			_xyzzy2 = 1000;

			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE/TUBE",
				_G(master_palette), 5, 1, 175, 150, 3, "407_s06", -1);
		} else if (_xyzzy2 == 1130) {
			terminateMachineAndNull(_drawerPopupHose);
			inv_give_to_player("GARDEN HOSE");
			hotspot_set_active("GARDEN HOSE  ", false);
			_xyzzy2 = 1000;

			kernel_examine_inventory_object("PING TUBE/HOSE",
				_G(master_palette), 5, 1, 175, 150, 3, "407_s06", -1);
		} else {
			kernel_examine_inventory_object("PING SURGICAL TUBE",
				_G(master_palette), 5, 1, 175, 150, 3, "407_s06", -1);
		}
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_xyzzy3 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeSurgicalTube3() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripLowHand1 = series_load("RIP LOW REACH POS1");
		setGlobals1(_ripLowHand1, 1, 11, 11, 11, 0, 11, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_tubeInDrawer);
		inv_give_to_player("SURGICAL TUBE");
		hotspot_set_active("SURGICAL TUBE ", false);
		kernel_examine_inventory_object("PING SURGICAL TUBE",
			_G(master_palette), 5, 1, 380, 250, 3, nullptr, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripLowHand1);
		_xyzzy3 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeGardenHose1() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_val4 == 1010) {
			digi_play("407r99e", 1);
		} else {
			player_set_commands_allowed(false);
			_ripHiHand1 = series_load("rip trek hi 1 hand");
			setGlobals1(_ripHiHand1, 1, 5, 5, 5, 0, 5, 1, 1, 1);
			sendWSMessage_110000(1);
		}
		break;

	case 1:
		terminateMachineAndNull(_drawerPopupHose);
		inv_give_to_player("GARDEN HOSE");
		hotspot_set_active("GARDEN HOSE    ", false);
		kernel_examine_inventory_object("PING GARDEN HOSE",
			_G(master_palette), 5, 1, 75, 200, 2, nullptr, -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand1);
		_xyzzy2 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeGardenHose2() {
	switch (_G(kernel).trigger) {
	case 1:
		terminateMachineAndNull(_drawerPopupHose);
		inv_give_to_player("GARDEN HOSE");
		hotspot_set_active("GARDEN HOSE  ", false);

		if (_val8 == 1130) {
			terminateMachineAndNull(_faucet1);
			hotspot_set_active("FAUCET PIPE  ", false);
			inv_give_to_player("FAUCET PIPE");
			_val8 = 1000;
			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE",
				_G(master_palette), 5, 1, 175, 150, 3, 0, -1);
		} else {
			kernel_examine_inventory_object("PING GARDEN HOSE",
				_G(master_palette), 5, 1, 175, 150, 3, 0, -1);
		}
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_xyzzy2 = 1000;
		player_set_commands_allowed(true);
		break;

	case 70:
		if (_val4 == 1010) {
			digi_play("407r99e", 1);
		} else {
			reachHand(10);
		}
		break;

	case 777:
		ws_walk(170, 335, nullptr, 70, 11);
		break;

	default:
		break;
	}
}

void Room407::takeGardenHose3() {
	switch (_G(kernel).trigger) {
	case 1:
		terminateMachineAndNull(_drawerPopupHose);
		inv_give_to_player("GARDEN HOSE");
		hotspot_set_active("GARDEN HOSE ", false);

		if (_xyzzy3 == 1116) {
			terminateMachineAndNull(_tubeInDrawer);
			hotspot_set_active("SURGICAL TUBE ", false);
			inv_give_to_player("SURGICAL TUBE");
			_xyzzy3 = 1000;
			kernel_examine_inventory_object("PING FAUCET PIPE/HOSE",
				_G(master_palette), 5, 1, 400, 150, 3, 0, -1);
		} else {
			kernel_examine_inventory_object("PING GARDEN HOSE",
				_G(master_palette), 5, 1, 400, 150, 3, 0, -1);
		}
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand1);
		_xyzzy2 = 1000;
		player_set_commands_allowed(true);
		break;

	case 70:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 12, 12, 12, 0, 12, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 777:
		ws_walk(436, 331, nullptr, 70, 1);
		break;

	default:
		break;
	}
}

void Room407::takeGardenHose4() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("GARDEN HOSE")) {
			inv_give_to_player("GARDEN HOSE");
			hotspot_set_active("GARDEN HOSE", false);
			terminateMachineAndNull(_drawerPopupHose);
			kernel_examine_inventory_object("PING GARDEN HOSE",
				_G(master_palette), 5, 1, 25, 190, 2, nullptr, -1);

			_G(player).command_ready = false;
		}
		break;

	case 2:
		player_set_commands_allowed(true);
		_G(player).command_ready = false;
		break;

	default:
		break;
	}
}

void Room407::takeGlassJar1() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand2 = series_load("rip trek hi reach 2hnd");
		setGlobals1(_ripHiHand2, 1, 13, 13, 13, 0, 13, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_bottle);
		inv_give_to_player("GLASS JAR");
		hotspot_set_active("GLASS JAR ", false);

		if (_xyzzy5 == 1116) {
			terminateMachineAndNull(_stopperInDrawer);
			inv_give_to_player("RUBBER PLUG");
			hotspot_set_active("JAR/RUBBER PLUG ", false);
			_xyzzy5 = 1000;
		}

		if (_xyzzy4 == 1116) {
			terminateMachineAndNull(_handleInDrawer);
			inv_give_to_player("PUMP GRIPS");
			hotspot_set_active("JAR/GRIPS ", false);
			_xyzzy4 = 1000;
		}

		if (_xyzzy9 == 1116) {
			terminateMachineAndNull(_bits);
			inv_give_to_player("CORK");
			hotspot_set_active("JAR/CORK ", false);
			_xyzzy9 = 1000;
		}

		kernel_examine_inventory_object("PING GLASS JAR",
			_G(master_palette), 5, 1, 375, 150, 3, 0, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand2);
		_xyzzy7 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeGlassJar2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand2 = series_load("rip trek hi reach 2hnd");
		setGlobals1(_ripHiHand2, 1, 13, 13, 13, 0, 13, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_bottle);
		inv_give_to_player("GLASS JAR");
		hotspot_set_active("GLASS JAR", false);

		if (_xyzzy5 == 1116) {
			terminateMachineAndNull(_stopperInDrawer);
			inv_give_to_player("RUBBER PLUG");
			hotspot_set_active("JAR/RUBBER PLUG", false);
			_xyzzy5 = 1000;
		}

		if (_xyzzy9 == 1116) {
			terminateMachineAndNull(_bits);
			inv_give_to_player("CORK");
			hotspot_set_active("JAR/CORK", false);
			_xyzzy9 = 1000;
		}

		kernel_examine_inventory_object("PING GLASS JAR",
			_G(master_palette), 5, 1, 100, 200, 3, 0, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand2);
		_xyzzy7 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takePeriodicTable1() {
	switch (_G(kernel).trigger) {	
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand2 = series_load("RPTMHR11");
		setGlobals1(_ripHiHand2, 1, 11, 11, 11, 0, 11, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_chart);
		inv_give_to_player("PERIODIC TABLE");
		hotspot_set_active("PERIODIC TABLE", false);
		kernel_examine_inventory_object("PING PERIODIC TABLE",
			_G(master_palette), 5, 1, 450, 200, 3, 0, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_rptmhr11);
		_xyzzy6 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takePeriodicTable2() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_chart);
		inv_give_to_player("PERIODIC TABLE");
		hotspot_set_active("PERIODIC TABLE ", false);
		kernel_examine_inventory_object("PING PERIODIC TABLE",
			_G(master_palette), 5, 1, 100, 200, 3, 0, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_xyzzy6 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeJarGrips() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 7, 7, 7, 0, 7, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_handleInDrawer);
		inv_give_to_player("PUMP GRIPS");
		hotspot_set_active("PUMP GRIPS ", false);
		kernel_examine_inventory_object("PING PUMP GRIPS",
			_G(master_palette), 5, 1, 425, 150, 3, "407R30", -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand1);
		_xyzzy4 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeJarCork1() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_bits);
		inv_give_to_player("CORK");
		hotspot_set_active("JAR/CORK", false);
		kernel_examine_inventory_object("PING CORK",
			_G(master_palette), 5, 1, 150, 200, 3, nullptr, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_xyzzy9 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeJarCork2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_bits);
		inv_give_to_player("CORK");
		hotspot_set_active("JAR/CORK ", false);
		kernel_examine_inventory_object("PING CORK",
			_G(master_palette), 5, 1, 425, 150, 3, nullptr, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand1);
		_xyzzy9 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeJarRubberPlug1() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_stopperInDrawer);
		inv_give_to_player("RUBBER PLUG");
		hotspot_set_active("JAR/RUBBER PLUG", false);
		kernel_examine_inventory_object("PING RUBBER PLUG",
			_G(master_palette), 5, 1, 150, 200, 3, nullptr, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_xyzzy5 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeJarRubberPlug2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand1 = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand1, 1, 6, 6, 6, 0, 6, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_stopperInDrawer);
		inv_give_to_player("RUBBER PLUG");
		hotspot_set_active("JAR/RUBBER PLUG ", false);
		kernel_examine_inventory_object("PING RUBBER PLUG",
			_G(master_palette), 5, 1, 425, 150, 3, nullptr, -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripHiHand1);
		_xyzzy5 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeNozzlesTube() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_tubeInDrawer);
		inv_give_to_player("SURGICAL TUBE");
		hotspot_set_active("NOZZLES/TUBE", false);
		kernel_examine_inventory_object("PING SURGICAL TUBE",
			_G(master_palette), 5, 1, 225, 200, 3, "407_s06", -1);
		break;

	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_ripMedHand1);
		_xyzzy3 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeLeverKey1() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripMedReach = series_load("RIP TREK MED REACH POS3");
		setGlobals1(_ripMedReach, 1, 11, 11, 11, 0, 11, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_lever);
		inv_give_to_player("LEVER KEY");
		hotspot_set_active("LEVER KEY", false);
		kernel_examine_inventory_object("PING LEVER KEY",
			_G(master_palette), 5, 1, 25, 200, 2, nullptr, -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedReach);
		_val10 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeLeverKey2() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		terminateMachineAndNull(_lever);
		inv_give_to_player("LEVER KEY");
		hotspot_set_active("LEVER KEY ", false);
		kernel_examine_inventory_object("PING LEVER KEY",
			_G(master_palette), 5, 1, 175, 200, 2, "407_s07a", -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedReach);
		_val10 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeLeverKey3() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_407rp98 = series_load("407RP98");
		terminateMachineAndNull(_lever);
		terminateMachineAndNull(_pump);

		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);

		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP levers ceiling");
		sendWSMessage_10000(1, _ripley, _407rp98, 101, 81, 2,
			_407rp98, 81, 81, 0);
		break;

	case 2:
		digi_play("407_s07a", 1);
		sendWSMessage_10000(1, _ripley, _407rp98, 80, 53, 3,
			_407rp98, 53, 53, 0);
		break;

	case 3:
		kernel_examine_inventory_object("PING LEVER KEY",
			_G(master_palette), 5, 1, 350, 150, 4, "407_s07a", -1);
		break;

	case 4:
		sendWSMessage_10000(1, _ripley, _407rp98, 52, 1, 5,
			_407rp98, 1, 1, 0);
		break;

	case 5:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();

		_pump = series_place_sprite("407PMROD", 0, 0, -53, 100, 0xf00);
		hotspot_set_active("LEVER KEY  ", false);
		inv_give_to_player("LEVER KEY");
		_val10 = 1000;

		series_unload(_407rp98);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeAirValveHandle() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(4);
		break;

	case 1:
		terminateMachineAndNull(_faucet2);
		inv_give_to_player("FAUCET HANDLE");
		hotspot_set_active("AIR VALVE/HANDLE", false);
		hotspot_set_active("AIR VALVE", true);
		kernel_examine_inventory_object("PING FAUCET HANDLE",
			_G(master_palette), 5, 1, 175, 200, 2, "407_s13", -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_val9 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takeFaucetHandle() {
	switch (_G(kernel).trigger) {
	case -1:
		reachHand(10);
		break;

	case 1:
		if (_val4 == 1010) {
			_val4 = 1011;

			if (_val8 == 1100)
				kernel_timing_trigger(1, 430, KT_DAEMON, KT_PARSE);
			else if (_xyzzy3 == 1130)
				kernel_timing_trigger(1, 440, KT_DAEMON, KT_PARSE);
		}

		terminateMachineAndNull(_faucet2);
		inv_give_to_player("FAUCET HANDLE");
		hotspot_set_active("FAUCET HANDLE", false);
		kernel_examine_inventory_object("PING FAUCET HANDLE",
			_G(master_palette), 5, 1, 75, 200, 2, "407_s04", -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripMedHand1);
		_val9 = 1000;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takePumpRod1() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_pump407 = series_load("407 PUMP");
		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);

		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP pumps");
		sendWSMessage_10000(1, _ripley, _pump407, 1, 40, 1,
			_pump407, 40, 40, 0);
		kernel_timing_trigger(150, 69);
		break;

	case 1:
		if (_xyzzy4 == 1115) {
			terminateMachineAndNull(_handleInDrawer);
			inv_give_to_player("PUMP GRIPS");
			hotspot_set_active("PUMP GRIPS ", false);
			_xyzzy4 = 1000;
			kernel_examine_inventory_object("PING PUMP GRIPS",
				_G(master_palette), 5, 1, 125, 200, 2, nullptr, -1);
		} else {
			kernel_timing_trigger(1, 2);
		}
		break;

	case 3:
		kernel_examine_inventory_object("PING PUMP ROD",
			_G(master_palette), 5, 1, 125, 200, 4, nullptr, -1);
		break;

	case 5:
		terminateMachineAndNull(_pump);
		hotspot_set_active("PUMP ROD", false);
		inv_give_to_player("PUMP ROD");
		sendWSMessage_10000(1, _ripley, _pump407, 14, 1, 6,
			_pump407, 1, 1, 0);
		break;

	case 6:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_hide_walker();
		series_unload(_pump407);
		_xyzzy1 = 1000;
		player_set_commands_allowed(true);
		break;

	case 69:
		digi_play("407_s07", 2);
		break;

	default:
		break;
	}
}

void Room407::takePumpRod2() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("PUMP ROD")) {
			player_set_commands_allowed(false);
			_placeLeverKey = series_load("407 PLACE LEVER KEY");
			terminateMachineAndNull(_lever);
			terminateMachineAndNull(_pump);
			player_update_info();

			_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
				_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);

			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "RIP levers ceiling");
			sendWSMessage_10000(1, _ripley, _placeLeverKey, 70, 49, 2,
				_placeLeverKey, 49, 49, 0);
		}
		break;

	case 2:
		kernel_examine_inventory_object("PING LEVER KEY/PUMP ROD",
			_G(master_palette), 5, 1, 350, 150, 3, "407_s07a", -1);
		break;

	case 3:
		sendWSMessage_10000(1, _ripley, _placeLeverKey, 48, 1, 4,
			_placeLeverKey, 1, 1, 0);
		break;

	case 4:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		hotspot_set_active("LEVER KEY  ", false);
		hotspot_set_active("PUMP ROD ", false);
		inv_give_to_player("PUMP ROD");
		_val10 = 1000;
		_xyzzy1 = 1000;
		series_unload(_placeLeverKey);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takePumpRod3() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_407rp99 = series_load("407rp99");
		ws_hide_walker();
		terminateMachineAndNull(_pump);
		player_update_info();

		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP takes pump rod from ceiling");
		sendWSMessage_10000(1, _ripley, _407rp99, 61, 42, 2,
			_407rp99, 42, 42, 0);
		break;

	case 2:
		digi_play("407_s07a", 1);
		kernel_examine_inventory_object("PING PUMP ROD",
			_G(master_palette), 5, 1, 350, 150, 3, "407_s07a", -1);
		break;

	case 3:
		sendWSMessage_10000(1, _ripley, _407rp99, 41, 1, 4,
			_407rp99, 1, 1, 0);
		break;

	case 4:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		hotspot_set_active("PUMP ROD ", false);
		inv_give_to_player("PUMP ROD");
		_xyzzy1 = 1000;
		series_unload(_407rp99);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room407::takePumpGrips1() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("PUMP GRIPS")) {
			inv_give_to_player("PUMP GRIPS");
			_xyzzy4 = 1000;

			hotspot_set_active("PUMP GRIPS", false);
			terminateMachineAndNull(_handleInDrawer);
			kernel_examine_inventory_object("PING PUMP GRIPS",
				_G(master_palette), 5, 1, 85, 190, 2, nullptr, -1);
			_G(player).command_ready = false;
		}
		break;

	case 2:
		player_set_commands_allowed(true);
		_G(player).command_ready = false;
		break;

	default:
		break;
	}
}

void Room407::takePumpGrips2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_pump407 = series_load("407 PUMP");

		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 1", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackNegative, "RIP pumps");
		sendWSMessage_10000(1, _ripley, _pump407, 1, 40, 1,
			_pump407, 40, 40, 0);
		kernel_timing_trigger(150, 69);
		break;

	case 1:
		terminateMachineAndNull(_handleInDrawer);
		inv_give_to_player("PUMP GRIPS");
		hotspot_set_active("PUMP GRIPS ", false);
		kernel_examine_inventory_object("PING PUMP GRIPS",
			_G(master_palette), 5, 1, 125, 200, 2, nullptr, -1);
		break;

	case 2:
		sendWSMessage_10000(1, _ripley, _pump407, 14, 1, 4,
			_pump407, 1, 1, 0);
		break;

	case 4:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_safariShadow);
		series_unload(_pump407);
		player_set_commands_allowed(true);
		break;

	case 69:
		digi_play("407_s07", 2);
		break;

	default:
		break;
	}
}

void Room407::takeRubberPlug() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("RUBBER PLUG")) {
			inv_give_to_player("RUBBER PLUG");
			hotspot_set_active("RUBBER PLUG", false);
			terminateMachineAndNull(_stopperInDrawer);
			kernel_examine_inventory_object("PING RUBBER PLUG",
				_G(master_palette), 5, 1, 75, 210, 2, nullptr, -1);

			_G(player).command_ready = false;
		}
		break;

	case 2:
		player_set_commands_allowed(true);
		_G(player).command_ready = false;
		break;

	default:
		break;
	}
}

void Room407::takeSurgicalTube() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("SURGICAL TUBE")) {
			inv_give_to_player("SURGICAL TUBE");
			_xyzzy3 = 1000;
			hotspot_set_active("SURGICAL TUBE", false);
			terminateMachineAndNull(_tubeInDrawer);
			kernel_examine_inventory_object("PING SURGICAL TUBE",
				_G(master_palette), 5, 1, 85, 210, 2, nullptr, -1);

			_G(player).command_ready = false;
		}
		break;

	case 2:
		player_set_commands_allowed(true);
		_G(player).command_ready = false;
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
