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
	// TODO
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
		lookLeverKey();
		digi_play("407r26a", 1);
	} else if (lookFlag && player_said("RUBBER PLUG") &&
			inv_object_is_here("RUBBER PLUG")) {
		digi_play("407r06", 1);
	} else if (lookFlag && player_said("JAR/RUBBER PLUG") &&
			inv_object_is_here("JAR/RUBBER PLUG")) {
		if (_xyzzy7 == 1112)
			lookGlassBottom();
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
		lookPivot();
		digi_play("407r09", 1);
	} else if (lookFlag && player_said("NOZZLES")) {
		lookNozzles();
		digi_play("407r10", 1);
	}
	// TODO
	else {
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

void Room407::lookLeverKey() {
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

void Room407::lookGlassBottom() {
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

void Room407::lookPivot() {
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

void Room407::lookNozzles() {
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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
