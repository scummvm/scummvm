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

#include "m4/riddle/rooms/section2/room203.h"
#include "m4/riddle/rooms/section2/section2.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/m4.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const int16 OFFICIAL_NORMAL_DIRS[] = { 200, 201, 202, -1 };
const char *OFFICIAL_NORMAL_NAMES[] = {
	"official walk pos11", "official walk pos9", "official walk pos7"
};
const int16 OFFICIAL_SHADOW_DIRS[] = { 210, 211, 212, -1 };
const char *OFFICIAL_SHADOW_NAMES[] = {
	"203of09s", "203of04s", "203of12s"
};

const int16 SNORMAL1_DIRS[] = { 200, -1 };
const char *SNORMAL1_NAMES[] = { "203s01" };
const int16 SSHADOW1_DIRS[] = { 210, -1 };
const char *SSHADOW1_NAMES[] = { "203ssh01" };

const int16 SNORMAL2_DIRS[] = { 240, -1 };
const char *SNORMAL2_NAMES[] = { "203s02" };
const int16 SSHADOW2_DIRS[] = { 250, -1 };
const char *SSHADOW2_NAMES[] = { "203ssh02" };

const int16 PEASANT_NORMAL_DIRS[] = { 240, -1 };
const char *PEASANT_NORMAL_NAMES[] = { "pesky walk pos3" };
const int16 PEASANT_SHADOW_DIRS[] = { 250, -1 };
const char *PEASANT_SHADOW_NAMES[] = { "shadow pesky walk pos3" };


void Room203::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val1 = 0;

	hotspot_set_active("SOLDIER'S HELMET", false);
	setupHelmetHotspot();

	_val2 = _val4 = _val5 = 0;
	_val6 = _val7 = _val8 = 0;
	_ripley80000 = 0;
	_digiName1 = nullptr;
	_showWalker = false;
	_trigger1 = _trigger2 = _trigger3 = -1;
	_trigger4 = _trigger5 = -1;

	digi_preload("203_s02");
	_shadow3 = series_load("safari shadow 3");

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		setupGk();
		setupOldLady();
		setupPigeons();
		setupOfficial();
		kernel_timing_trigger(1, 130);

		if (_val1) {
			digi_preload("203_s09");
			series_load("one frame helmet");
			series_place_sprite("one frame helmet", 0, 960, -53, 100, 0x700);
			_peasantSeries = series_load("pesky peasant reach for helmet");
			_peasantSeriesShadow = series_load("shadow pp reach for helmet");

			_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1240, 332, 75, 0x800, 0,
				triggerMachineByHashCallback, "pesky peasant");
			_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1240, 332, 75, 0x800, 0,
				triggerMachineByHashCallback, "pesky peasant SHADOW");
			sendWSMessage_10000(1, _peasant, _peasantSeries, 23, 36, 121,
				_peasantSeries, 36, 23, 2);
			sendWSMessage_10000(1, _peasantShadow, _peasantSeriesShadow, 23, 36, -1,
				_peasantSeriesShadow, 36, 23, 2);

			hotspot_set_active("SOLDIER'S HELMET", false);
			hotspot_set_active("SOLDIER'S HELMET ", true);

		} else if (_G(flags)[V061]) {
			hotspot_set_active("PEASANT", false);
			hotspot_set_active("SOLDIER'S HELMET", false);

		} else {
			switch (_peasantMode) {
			case 4050:
			case 4051:
				_peasantX = 745;
				_peasantY = 378;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			case 4052:
				_peasantX = 418;
				_peasantY = 385;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			case 4053:
				_peasantX = 950;
				_peasantY = 378;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			case 4054:
				_peasantX = 1100;
				_peasantY = 375;
				_peasantLayer = 0x800;
				_peasantScale = 75;
				break;
			default:
				break;
			}

			setupPeasant();
		}
		break;

	case 202:
		player_set_commands_allowed(false);
		MoveScreenDelta(_G(game_buff_ptr), -300, 0);
		ws_demand_location(547, 316, 1);

		setupGk();
		setupOldLady();
		setupPigeons();
		setupOfficial();

		if (_G(flags)[V061]) {
			hotspot_set_active("PEASANT", false);
			hotspot_set_active("SOLDIER'S HELMET", false);

		} else {
			if (_G(flags)[V072] == 4050) {
				_peasantMode = 4050;
				_peasantShould = 4091;
			} else {
				_peasantMode = 4051;
				_peasantShould = 4110;
			}

			_peasantX = 745;
			_peasantY = 378;
			_peasantLayer = 0x800;
			_peasantScale = 75;

			setupPeasant();
		}

		kernel_timing_trigger(1, 130);
		player_set_commands_allowed(true);
		break;

	case 204:
		player_set_commands_allowed(false);
		setupOfficial();
		setupGk();
		setupOldLady();
		setupPigeons();

		hotspot_set_active("PEASANT", false);
		hotspot_set_active("SOLDIER'S HELMET", false);
		kernel_timing_trigger(1, 130);
		ws_demand_location(348, 273, 1);
		ws_walk(231, 331, nullptr, 303, 4);
		break;

	case 207:
		MoveScreenDelta(_G(game_buff_ptr), -780, 0);
		player_set_commands_allowed(false);
		setupGk();
		setupOldLady();
		setupPigeons();

		if (_G(flags)[V067]) {
			ws_demand_location(1100, 290, 9);
			_G(player).disable_hyperwalk = true;

			_peskyYellsThief = series_load("pesky yells thief");
			_peasantRocksShadow = series_load("shadow pesky rock loop");
			_peasantX = 1100;
			_peasantY = 375;
			_peasantLayer = 0x800;
			_peasantScale = 75;

			_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, 0,
				triggerMachineByHashCallback, "pp");
			_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_peasantX, _peasantY - 53, _peasantScale, _peasantLayer, 0,
				triggerMachineByHashCallback, "pp");

			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 17, 354,
				_peskyYellsThief, 17, 17, 0);
			sendWSMessage_10000(1, _peasantShadow, _peasantRocksShadow, 1, 1, -1,
				_peasantRocksShadow, 1, 1, 0);

			digi_preload("203p02");
			_flag1 = true;
			_officialMode = 2001;
			_officialShould = 2010;

			_officialStander = series_load("official stander");
			_official = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1436, 362, 85, 0xf00, 0,
				triggerMachineByHashCallback, "of");
			sendWSMessage_10000(1, _official, _officialStander, 1, 1, 356,
				_officialStander, 1, 1, 0);

			_G(camera_reacts_to_player) = false;
			ws_walk_load_shadow_series(OFFICIAL_SHADOW_DIRS, OFFICIAL_SHADOW_NAMES);
			ws_walk_load_walker_series(OFFICIAL_NORMAL_DIRS, OFFICIAL_NORMAL_NAMES);
			_officialTurn9_11 = series_load("official turn 9-11");
			_officialTurn11_3 = series_load("official turn 11-3");
			_officialTurn3_7 = series_load("official turn 3-7");

			kernel_timing_trigger(10, 352);
			kernel_timing_trigger(1, 350);

		} else {
			ws_demand_location(1055, 295, 7);
			ws_walk(1045, 345, nullptr, 303, 7);
			kernel_timing_trigger(1, 300);
		}
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(-40, 352, 3);

		if (keyCheck()) {
			_G(kernel).call_daemon_every_loop = true;
			_gkShould = 0;

			series_unload(3);
			series_unload(4);
			series_unload(13);
			series_unload(14);

			setupPigeons();
			_ripHeadTurn = series_load("rip trek head turn pos3");
			_203sg01 = series_load("203sg01");

			series_load("mei chen trek walk pos1", 220);
			series_load("mei chen trek walk pos2", 221);
			series_load("mei chen trek walk pos3", 222);
			series_load("mctsh1", 230);
			series_load("mctsh2", 231);
			series_load("mctsh3", 232);

			ws_walk_load_walker_series(SNORMAL1_DIRS, SNORMAL1_NAMES);
			ws_walk_load_shadow_series(SSHADOW1_DIRS, SSHADOW1_NAMES);
			ws_walk_load_walker_series(SNORMAL2_DIRS, SNORMAL2_NAMES);
			ws_walk_load_shadow_series(SSHADOW2_DIRS, SSHADOW2_NAMES);

			_mei = triggerMachineByHash_3000(8, 4, *S2_MEI_NORMAL_DIRS, *S2_MEI_SHADOW_DIRS,
				-95, 352, 3, triggerMachineByHashCallback3000, "mc");
			ws_walk(196, 335, nullptr, 9000, 3);
			sendWSMessage_10000(_mei, 173, 344, 3, -1, 1);
			_G(player).disable_hyperwalk = true;

		} else {
			setupGk();

			if (player_been_here(203)) {
				_peasantMode = 4052;
				_peasantShould = 4170;
				_peasantX = 418;
				_peasantY = 385;
				_peasantLayer = 0x800;
				_peasantScale = 75;

				if (_G(flags)[V061]) {
					hotspot_set_active("PEASANT", false);
					hotspot_set_active("SOLDIER'S HELMET", false);
				} else {
					setupPeasant();
				}

				setupOldLady();
				setupPigeons();
				setupOfficial();
				kernel_timing_trigger(1, 130);
				ws_walk(115, 353, nullptr, 371, 3);

			} else {
				_ripLooksAtHeads = series_load("rip looks at heads pos2");
				ws_walk_load_shadow_series(S2_MEI_SHADOW_DIRS, S2_MEI_SHADOW_NAMES);
				ws_walk_load_walker_series(S2_MEI_NORMAL_DIRS, S2_MEI_NORMAL_NAMES);
				_mei = triggerMachineByHash_3000(8, 4, *S2_MEI_NORMAL_DIRS, *S2_MEI_SHADOW_DIRS,
					-40, 352, 3, triggerMachineByHashCallback3000, "mc");

				if (_G(kittyScreaming))
					kernel_timing_trigger(1, 31);
				else
					kernel_timing_trigger(60, 1);
			}
		}

		break;
	}

	digi_play_loop("203_s02", 3, 40);
}

void Room203::daemon() {
	if (keyCheck() && _gkShould == 0 && _G(game_buff_ptr)->x1 >= 380) {
		_gkShould = 1;
		_G(kernel).call_daemon_every_loop = false;
		digi_play("950_s13", 1, 255, -1, 950);
	}

	switch (_G(kernel).trigger) {
	case 1:
		kernel_trigger_dispatchx(kernel_trigger_create(3));
		kernel_timing_trigger(140, 4);
		break;

	case 3:
		sendWSMessage_10000(_mei, 170, 352, 4, -1, 1);
		break;

	case 4:
		ws_walk(115, 353, nullptr, 5, 3);
		break;

	case 5:
		_meiHallOfClassics = series_load("mc hall of classics line");
		setGlobals1(_meiHallOfClassics, 1, 8, 8, 8, 2, 35, 41, 41, 41);
		sendWSMessage_110000(_mei, 6);
		digi_play("203m01", 1);
		break;

	case 6:
		sendWSMessage_120000(_mei, 7);
		break;

	case 7:
		_ctr1 = 0;
		_ripLookAtHeadsTalkMei = series_load("rip look at heads talk mei");
		setGlobals1(_ripLookAtHeadsTalkMei, 1, 5, 5, 8, 1, 8, 8, 8, 8, 0, 9,
			16, 17, 20, 1, 16, 4, 4, 4, 0);
		sendWSMessage_110000(8);
		digi_play("203r01", 1, 255, 8);
		break;

	case 8:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_120000(9);
		} else {
			++_ctr1;
		}
		break;

	case 9:
		sendWSMessage_150000(_mei, -1);
		_meiCheekLine = series_load("mc cheek line");
		setGlobals1(_meiCheekLine, 1, 30, 30, 35, 1, 38, 38, 38, 38, 1, 21, 45, 45, 45, 1);
		sendWSMessage_110000(_mei, 10);
		digi_play("203m02", 1, 255, 10);
		break;

	case 10:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_120000(_mei, -1);
			sendWSMessage_110000(11);
			digi_play("203r02", 1, 255, 11);
		} else {
			++_ctr1;
		}
		break;

	case 11:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_130000(12);
		} else {
			++_ctr1;
		}
		break;

	case 12:
		digi_play("203r03", 1, 255, 13);
		break;

	case 13:
		sendWSMessage_180000(-1);
		sendWSMessage_130000(_mei, 14);
		digi_play("203m03", 1);
		break;

	case 14:
		sendWSMessage_150000(_mei, -1);
		sendWSMessage_10000(_mei, 271, 337, 2, -1, 1);
		kernel_timing_trigger(40, 15);
		break;

	case 15:
		sendWSMessage_150000(-1);
		ws_walk(315, 353, nullptr, 16);
		break;

	case 16:
		ws_walk(350, 328, nullptr, 17, 10);
		break;

	case 17:
		series_unload(_meiCheekLine);
		series_unload(_ripLookAtHeadsTalkMei);
		series_unload(_meiHallOfClassics);
		_meiRightHandOut = series_load("mei trek rt hand out pos2");
		setGlobals1(_meiRightHandOut, 1, 10, 10, 10, 1, 10, 1, 1, 1, 1);
		sendWSMessage_110000(_mei, 18);
		digi_play("203m04", 1, 255, 19);
		break;

	case 18:
	case 25:
		sendWSMessage_120000(_mei, -1);
		break;

	case 19:
		sendWSMessage_120000(_mei, -1);
		_gkMode = 3001;
		_digiName1 = "203g01";
		_digiTrigger1 = 20;
		kernel_trigger_dispatchx(kernel_trigger_create(125));
		break;

	case 20:
		sendWSMessage_150000(_mei, -1);
		series_unload(_meiRightHandOut);
		_ripYouSeeToIt = series_load("rip says you see to it");
		_meiTurnAndTalk = series_load("mc turn and talk");
		setGlobals1(_meiTurnAndTalk, 1, 55, 55, 55, 1, 1, 20, 20, 20, 0, 20, 1, 1, 1, 0, 55, 55, 55, 55, 0);
		sendWSMessage_110000(_mei, -1);
		digi_play("203m05", 1, 255, 21);
		break;

	case 21:
		sendWSMessage_150000(-1);
		setGlobals1(_ripYouSeeToIt, 1, 38, 38, 38, 1, 38, 26, 26, 26, 1);
		sendWSMessage_110000(23);
		digi_play("203r04", 1, 255, 24);
		break;

	case 23:
		sendWSMessage_120000(-1);
		break;

	case 24:
		sendWSMessage_150000(_mei, -1);
		_meiTalkToRip = series_load("mc talk to rip");
		setGlobals1(_meiTalkToRip, 1, 13, 13, 13, 1, 13, 1, 1, 1);
		sendWSMessage_110000(_mei, 25);
		digi_play("203m06", 1, 255, 26);
		break;

	case 26:
		setGlobals1(_ripLooksAtHeads, 1, 21, 21, 21, 1, 21, 1, 1, 1, 1);
		sendWSMessage_110000(-1);
		digi_play("203r05", 1, 255, 27);
		break;

	case 27:
		sendWSMessage_120000(28);
		sendWSMessage_150000(_mei, -1);
		sendWSMessage_10000(_mei, -40, 352, 2, 29, 1);
		break;

	case 28:
		digi_play("203r05a", 1);
		break;

	case 29:
		sendWSMessage_60000(_mei);
		kernel_timing_trigger(1, 4567);
		break;

	case 30:
		sendWSMessage_150000(1968);
		player_set_commands_allowed(true);
		_G(player).disable_hyperwalk = false;
		break;

	case 31:
		_ripYouSeeToIt = series_load("rip says you see to it");
		_meiTalkToRip = series_load("mc talk to rip");
		_meiTurnAndTalk = series_load("mc turn and talk");
		ws_demand_location(100, 360);
		kernel_timing_trigger(1, 29);
		break;

	case 40:
		_oldLadyMode = 40;
		_oldLady1 = series_load("old lady");
		_oldLadyProtectsHelmet = series_load("old lady protect helmet");
		_ripKneeling = series_load("rip kneels down to old lady");
		_ripKneelingTalk = series_load("rip kneeling talks to old lady");
		_ripPointsAtHelmet = series_load("rip points at helmet");
		ws_hide_walker();

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip kneel");
		sendWSMessage_10000(1, _ripley, _ripKneeling, 1, 31, 42, _ripKneeling, 31, 31, 0);
		break;

	case 42:
		sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 1, 16, 43,
			_ripKneelingTalk, 16, 16, 0);
		digi_play("203r20", 1, 255, 45);
		break;

	case 43:
		sendWSMessage_10000(1, _ripley, _ripPointsAtHelmet, 1, 5, 44,
			_ripPointsAtHelmet, 5, 5, 0);
		break;

	case 44:
		sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 5, 1, -1,
			_ripKneelingTalk, 5, 5, 0);
		break;

	case 45:
		sendWSMessage_10000(1, _oldLady, _oldLady1, 1, 7, 46, _oldLady1, 7, 7, 0);
		break;

	case 46:
		sendWSMessage_10000(1, _oldLady, _oldLady1, 7, 1, 47, _oldLadyFrame, 1, 1, 0);
		break;

	case 47:
		sendWSMessage_10000(1, _oldLady, _oldLadyProtectsHelmet, 1, 16, 48,
			_oldLadyProtectsHelmet, 16, 16, 0);
		break;

	case 48:
		sendWSMessage_10000(1, _oldLady, _oldLadyProtectsHelmet, 16, 1, 49, _oldLadyFrame, 1, 1, 0);
		break;

	case 49:
		sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 5, 1, 50,
			_ripKneelingTalk, 1, 1, 0);
		break;

	case 50:
		sendWSMessage_10000(1, _ripley, _ripKneeling, 31, 1, 51, _ripKneeling, 1, 1, 0);
		break;

	case 51:
		terminateMachineAndNull(_ripley);
		series_unload(_ripPointsAtHelmet);
		series_unload(_ripKneelingTalk);
		series_unload(_ripKneeling);
		series_unload(_oldLadyProtectsHelmet);
		series_unload(_oldLady1);
		_oldLadyMode = 5666;
		_oldLadyShould = 5100;
		kernel_timing_trigger(1, 130);
		break;

	case 60:
		_oldLadyMode = 60;
		_oldLadyNoHelmet = series_load("old lady feeding no helmet");
		ws_hide_walker();
		_ripKneeling = series_load("rip kneels down to old lady");
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip kneel");
		sendWSMessage_10000(1, _ripley, _ripKneeling, 1, 31, 62, _ripKneeling, 31, 31, 0);
		break;

	case 62:
		series_unload(_ripKneeling);
		_ripKneelingTalk = series_load("rip kneeling talks to old lady");
		sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 1, 5, 65, _ripKneelingTalk, 5, 5, 0);
		break;

	case 65:
		_ripGivesPhoto = series_load("rip gives photo to old lady");
		sendWSMessage_10000(1, _ripley, _ripGivesPhoto, 1, 9, 67, _ripGivesPhoto, 9, 9, 0);
		digi_play("203r18", 1, 255, 66);
		break;

	case 66:
		midi_play("HELMET", 255, 1, -1, 949);
		break;

	case 67:
		kernel_timing_trigger(60, 68);
		break;

	case 68:
		_oldLadyShowsPhoto = series_load("old lady shows photo");
		sendWSMessage_10000(1, _oldLady, _oldLadyShowsPhoto, 1, 23, 69,
			_oldLadyShowsPhoto, 23, 23, 0);
		break;

	case 69:
		_stream1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "POPUP OL & lover");
		_oldLadyPhotoPopup = series_load("old woman photo pop-up");
		sendWSMessage_10000(1, _stream1, _oldLadyPhotoPopup, 1, 5, -1,
			_oldLadyPhotoPopup, 5, 5, 0);
		kernel_timing_trigger(150, 70);
		break;

	case 70:
		sendWSMessage_10000(1, _stream1, _oldLadyPhotoPopup, 5, 1, 71,
			_oldLadyPhotoPopup, 1, 1, 0);
		break;

	case 71:
		terminateMachineAndNull(_stream1);
		series_unload(_oldLadyPhotoPopup);
		series_unload(_oldLadyShowsPhoto);
		_oldLadyPointsToPhoto = series_load("old lady points to photo");
		sendWSMessage_10000(1, _oldLady, _oldLadyPointsToPhoto, 1, 12, 72,
			_oldLadyPointsToPhoto, 12, 12, 0);
		break;

	case 72:
		kernel_timing_trigger(15, 73);
		break;

	case 73:
		sendWSMessage_10000(1, _oldLady, _oldLadyPointsToPhoto, 12, 1, 74,
			_oldLadyPointsToPhoto, 1, 1, 0);
		break;

	case 74:
		sendWSMessage_10000(1, _ripley, _ripGivesPhoto, 9, 1, 75, _ripGivesPhoto, 1, 1, 0);
		break;

	case 75:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_oldLady);
		series_unload(_oldLadyPointsToPhoto);
		series_unload(_ripGivesPhoto);

		digi_preload("203r21");
		_stream1 = series_stream("old lady takes photo", 5, 1, 76);
		inv_move_object("RIPLEY PHOTO", 203);
		digi_play("203r21", 1);
		break;

	case 76:
		digi_preload("203r21");
		_stream1 = series_stream("old lady gives helmet", 5, 1, 79);
		series_stream_break_on_frame(_stream1, 77, 62);
		break;

	case 77:
		series_set_frame_rate(_stream1, 3000);
		series_stream("old woman smiles", 5, 0, 78);
		break;

	case 78:
		series_set_frame_rate(_stream1, 5);
		ws_OverrideCrunchTime(_stream1);
		series_stream_break_on_frame(_stream1, 87, 79);
		break;

	case 79:
		_oldLady = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "ol");
		sendWSMessage_10000(1, _oldLady, _oldLadyNoHelmet, 1, 1, -1,
			_oldLadyNoHelmet, 1, 1, 0);
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip kneel");
		sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 1, 1, -1,
			_ripKneelingTalk, 1, 1, 0);

		digi_preload("203r22");
		_stream1 = series_stream("rip smiles close-up", 5, 0, 82);
		series_stream_break_on_frame(_stream1, 14, 6250);
		digi_play("203r22", 1, 255, 80);
		break;

	case 81:
		series_set_frame_rate(_stream1, 5);
		ws_OverrideCrunchTime(_stream1);
		break;

	case 82:
		terminateMachineAndNull(_ripley);
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip kneel");
		_ripKneeling = series_load("rip kneels down to old lady");
		sendWSMessage_10000(1, _ripley, _ripKneeling, 31, 1, 84, _ripKneeling, 1, 1, 0);
		break;

	case 84:
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 749);
		terminateMachineAndNull(_ripley);
		series_unload(_ripKneeling);
		sendWSMessage_10000(1, _oldLady, _oldLadyFrame, 2, 2, 0, _oldLadyFrame, 2, 2, 0);
		_oldLadyMode = 5669;
		_oldLadyShould = 5200;
		_G(flags)[V060] = 1;
		inv_give_to_player("SOLDIER'S HELMET");
		setupHelmetHotspot();
		kernel_timing_trigger(1, 130);
		ws_unhide_walker();
		_val2 = 0;
		player_set_commands_allowed(true);
		break;

	case 90:
		other_save_game_for_resurrection();
		player_set_commands_allowed(false);
		ws_hide_walker();
		_peskyYellsThief = series_load("pesky yells thief");
		_peasantRocksShadow = series_load("shadow pesky rock loop");
		_ripClimbsAndBacksDown = series_load("rip climbs and backs down");

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip climbs");
		sendWSMessage_10000(1, _ripley, _ripClimbsAndBacksDown, 1, 75, 96,
			_ripClimbsAndBacksDown, 75, 75, 0);
		kernel_timing_trigger(400, 91);
		break;

	case 91:
		terminateMachineAndNull(_peasant);
		terminateMachineAndNull(_peasantShadow);
		_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, 0,
			triggerMachineByHashCallback, "pesky peasant yells thief");
		_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, 0,
			triggerMachineByHashCallback, "SHADOW pp yells thief");
		sendWSMessage_10000(1, _peasant, _peskyYellsThief, 1, 19, 94,
			_peskyYellsThief, 19, 19, 0);
		sendWSMessage_10000(1, _peasantShadow, _peasantRocksShadow, 1, 1, -1,
			_peasantRocksShadow, 1, 1, 0);
		_flag1 = true;
		kernel_timing_trigger(100, 92);
		digi_preload("203p02");
		break;

	case 92:
		if (_flag1)
			digi_play("203p02", 1, 255, 93);
		else
			digi_unload("203p02");
		break;

	case 93:
		kernel_timing_trigger(120, 92);
		break;

	case 94:
		if (_flag1) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 19, 21, 95,
				_peskyYellsThief, 21, 21, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 19, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		}
		break;

	case 95:
		if (_flag1) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 19, 94,
				_peskyYellsThief, 19, 19, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		}
		break;

	case 96:
		sendWSMessage_10000(1, _ripley, _ripClimbsAndBacksDown, 75, 1, 97,
			_ripClimbsAndBacksDown, 1, 1, 0);
		break;

	case 97:
		terminateMachineAndNull(_ripley);
		series_unload(_ripClimbsAndBacksDown);
		player_update_info();
		ws_unhide_walker();
		kernel_timing_trigger(3, 98);
		break;

	case 98:
		ws_walk(_G(player_info).x, _G(player_info).y, nullptr, -1, 4);
		terminateMachineAndNull(_official);
		terminateMachineAndNull(_officialShadow);
		ws_walk_load_shadow_series(OFFICIAL_SHADOW_DIRS, OFFICIAL_SHADOW_NAMES);
		ws_walk_load_walker_series(OFFICIAL_NORMAL_DIRS, OFFICIAL_NORMAL_NAMES);
		_official = triggerMachineByHash_3000(8, 5, *OFFICIAL_NORMAL_DIRS, *OFFICIAL_SHADOW_DIRS,
			1300, 360, 9, triggerMachineByHashCallback3000, "official arresting");
		sendWSMessage_10000(_official, 720, 345, 9, 99, 1);
		break;

	case 99:
		_flag1 = false;
		_officialTurn9_11_pointGun = series_load("official turn 9-11 point gun");
		setGlobals1(_officialTurn9_11_pointGun, 1, 31, 31, 31, 0, 31, 13, 13, 13, 0, 13, 1, 1, 1);
		sendWSMessage_110000(_official, 100);
		break;

	case 100:
		sendWSMessage_120000(_official, -1);
		ws_walk(656, 335, nullptr, 101, 9);
		break;

	case 101:
		sendWSMessage_130000(_official, 102);
		ws_walk(280, 345, nullptr, 103, 9);
		break;

	case 102:
		sendWSMessage_150000(_official, -1);
		g_engine->camera_shift_xy(0, 0);
		sendWSMessage_10000(_official, 345, 355, 9, -1, 1);
		break;

	case 103:
		_ripLookAtHeadsTalkMei = series_load("rip look at heads talk mei");
		setGlobals1(_ripLookAtHeadsTalkMei, 11, 16, 16, 16, 1, 16, 11, 11, 11);
		sendWSMessage_110000(-1);
		digi_play("203r28", 1, 255, 104);
		break;

	case 104:
		_officialMoveAlong = series_load("official move along");
		setGlobals1(_officialMoveAlong, 1, 12, 12, 12, 0, 12, 1, 1, 1);
		sendWSMessage_110000(_official, 105);
		break;

	case 105:
		sendWSMessage_120000(_official, 106);
		break;

	case 106:
		sendWSMessage_150000(_official, 107);
		break;

	case 107:
		sendWSMessage_120000(108);
		break;

	case 108:
		sendWSMessage_150000(-1);
		ws_walk(-30, 355, nullptr, -1);
		kernel_timing_trigger(40, 109);
		break;

	case 109:
		sendWSMessage_10000(_official, -30, 355, 9, -1, 1);
		kernel_timing_trigger(60, 110);
		break;

	case 110:
		disable_player_commands_and_fade_init(111);
		break;

	case 111:
		_G(game).setRoom(413);
		player_set_commands_allowed(true);
		break;

	case 120:
		switch (_peasantMode) {
		case 4050:
			switch (_peasantShould) {
			case 4091:
			case 4094:
			case 4095:
				if (_trigger2 != -1) {
					kernel_trigger_dispatchx(kernel_trigger_create(_trigger2));
					_trigger2 = -1;
				} else {
					kernel_trigger_dispatchx(kernel_trigger_create(121));
				}
				break;

			default:
				break;
			}
			break;

		case 4051:
			if (_peasantShould == 4110) {
				if (_trigger2 != -1) {
					kernel_trigger_dispatchx(kernel_trigger_create(_trigger2));
					_trigger2 = -1;
				} else {
					player_update_info();

					if (_G(player_info).x >= 450 && _G(player_info).x <= 8000) {
						kernel_trigger_dispatchx(kernel_trigger_create(121));
					} else {
						player_set_commands_allowed(false);
						_peasantShould = (_G(player_info).x < 450) ? 4117 : 4113;
						peasantWalk();
						kernel_timing_trigger(1, 121);
					}
				}
			}
			break;

		case 4052:
			switch (_peasantShould) {
			case 4170:
				if (_trigger2 != -1) {
					kernel_trigger_dispatchx(kernel_trigger_create(_trigger2));
					_trigger2 = -1;
				} else {
					player_update_info();

					if (_G(player_info).x <= 450) {
						kernel_trigger_dispatchx(kernel_trigger_create(121));
					} else {
						player_set_commands_allowed(false);
						_peasantShould = 4175;
						peasantWalk();
						kernel_timing_trigger(1, 121);
					}
				}
				break;

			case 4174:
			case 4175:
				kernel_timing_trigger(1, 121);
				break;

			default:
				break;
			}
			break;

		case 4053:
			switch (_peasantShould) {
			case 4160:
				if (_trigger2 != -1) {
					kernel_trigger_dispatchx(kernel_trigger_create(_trigger2));
					_trigger2 = -1;
				} else {
					player_update_info();

					if (_G(player_info).x >= 450 && _G(player_info).x <= 1040) {
						kernel_trigger_dispatchx(kernel_trigger_create(121));
					} else {
						player_set_commands_allowed(false);
						_peasantShould = (_G(player_info).x < 800) ? 4162 : 4166;
						peasantWalk();
						kernel_timing_trigger(1, 121);
					}
				}
				break;

			default:
				break;
			}
			break;

		case 4054:
			switch (_peasantShould) {
			case 4140:
				if (_trigger2 != -1) {
					kernel_trigger_dispatchx(kernel_trigger_create(_trigger2));
					_trigger2 = -1;
				} else {
					player_update_info();

					if (_G(player_info).x < 1040) {
						player_set_commands_allowed(false);
						_peasantShould = 4142;
						peasantWalk();
						kernel_timing_trigger(1, 121);
					} else {
						if (_val5) {
							_peasantShould = _val5;
							_val5 = 0;
						}

						kernel_trigger_dispatchx(kernel_trigger_create(121));
					}
				}
				break;

			case 4146:
			case 4147:
			case 4148:
			case 4149:
				kernel_timing_trigger(1, 121);
				break;

			case 4150:
				kernel_trigger_dispatchx(kernel_trigger_create(121));
				break;

			default:
				break;
			}
			break;

		case 4055:
		case 4056:
		case 4058:
		case 4059:
			kernel_trigger_dispatchx(kernel_trigger_create(121));
			break;

		case 4057:
			if (_trigger2 != -1) {
				kernel_trigger_dispatchx(kernel_trigger_create(_trigger2));
				_trigger2 = -1;
			} else {
				kernel_trigger_dispatchx(kernel_trigger_create(121));
			}
			break;

		default:
			break;
		}
		break;

	case 125:
		kernel_trigger_dispatchx(kernel_trigger_create(126));
		break;

	case 126:
		switch (_gkMode) {
		case 3000:
			if (_gkShould == 3000 && _gkMode == 3000 && _trigger1 != -1) {
				kernel_trigger_dispatchx(_trigger1);
				_trigger1 = -1;
			}

			sendWSMessage_10000(1, _gk, _gkFrame, 1, 1, -1, _gkFrame, 1, 1, 0);
			break;

		case 3001:
			_gkManyDoNeedPass = series_load("gk many do but need a pass");

			if (_digiName1) {
				digi_play(_digiName1, 1, 255, _digiTrigger1);
				_digiName1 = nullptr;
			}

			sendWSMessage_10000(1, _gk, _gkManyDoNeedPass, 1, 15, 125,
				_gkManyDoNeedPass, 15, 15, 0);
			_gkShould = 3001;
			_gkMode = 3002;
			break;

		case 3002:
			sendWSMessage_10000(1, _gk, _gkManyDoNeedPass, 15, 1, 125, _gkFrame, 1, 1, 0);
			_gkShould = 3001;
			_gkMode = 3003;
			break;

		case 3003:
			series_unload(_gkManyDoNeedPass);
			_gkShould = _gkMode = 3000;
			break;

		case 3004:
			if (_digiName1) {
				digi_play(_digiName1, 1, 255, _digiTrigger1);
				_digiName1 = nullptr;
			}

			sendWSMessage_10000(1, _gk, _gkMayNotPass, 1, 36, 125,
				_gkManyDoNeedPass, 1, 1, 0);
			_gkShould = _gkMode = 3000;
			break;

		default:
			break;
		}
		break;

	case 150:
		if (_unkMode == 1001 && _oldLadyMode2 == 1030 && _trigger5 != -1) {
			kernel_trigger_dispatchx(_trigger5);
			_trigger5 = -1;

			if (_showWalker) {
				ws_unhide_walker();
				_showWalker = false;
			}
			if (_ripley80000) {
				sendWSMessage_80000(_ripley);
				_ripley80000 = false;
			}
		}

		kernel_timing_trigger(3, 151);
		break;

	case 152:
		_val2 = 0;

		switch (_unkMode) {
		case 1001:
			_oldLadyMode2 = 1040;
			break;
		case 1002:
			_oldLadyMode2 = (_unkShould == 1121) ? 1121 : 5303;
			break;
		case 1003:
			_oldLadyMode2 = 1240;
			break;
		default:
			break;
		}
		break;

	case 153:
		kernel_trigger_dispatchx(kernel_trigger_create(150));
		break;

	case 300:
		if (_G(flags)[V061]) {
			hotspot_set_active("PEASANT", false);
			hotspot_set_active("SOLDIER'S HELMET", false);
		} else {
			ws_walk_load_walker_series(PEASANT_NORMAL_DIRS, PEASANT_NORMAL_NAMES);
			ws_walk_load_shadow_series(PEASANT_SHADOW_DIRS, PEASANT_SHADOW_NAMES);
			_peasantRocks = series_load("peasant rocks 5frames");
			_peasantRocksShadow = series_load("shadow pesky rock loop");
			_peasantSquat3 = series_load("peasant 3 to squat");
			_peasantSquat9 = series_load("shadow pesky from 9 to squat");

			_peasantX = 1100;
			_peasantY = 375;
			_peasantLayer = 0x800;
			_peasantScale = 75;

			_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_peasantX, _peasantY - 53, _peasantScale, _peasantLayer, true,
				triggerMachineByHashCallback, "pesky peasant");
			_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, true,
				triggerMachineByHashCallback, "pesky peasant shadow");
			sendWSMessage_10000(1, _peasant, _peasantSquat3, 2, 18, -1,
				_peasantRocks, 1, 1, 0);
			sendWSMessage_10000(1, _peasantShadow, _peasantSquat9, 2, 18, 302,
				_peasantRocksShadow, 1, 1, 0);
		}

		setupOfficial();
		kernel_trigger_dispatchx(kernel_trigger_create(125));
		break;


	case 302:
		_peasantMode = 4054;
		_peasantShould = 4140;
		setupPeasantHotspot(_peasantMode);
		kernel_timing_trigger(1, 120);
		series_unload(_peasantSquat9);
		series_unload(_peasantSquat3);
		player_set_commands_allowed(true);
		break;

	case 303:
	case 371:
		player_set_commands_allowed(true);
		break;

	case 350:
		kernel_trigger_dispatchx(kernel_trigger_create(125));
		break;

	case 352:
		if (_flag1)
			digi_play("203p02", 1, 255, 93);
		else
			digi_unload("203p02");
		break;

	case 353:
		kernel_timing_trigger(120, 352);
		break;

	case 354:
		if (_flag1) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 17, 21, 355,
				_peskyYellsThief, 21, 21, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 17, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		}
		break;

	case 355:
		if (_flag1) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 17, 354,
				_peskyYellsThief, 17, 17, 0);
		}
		break;

	case 356:
		terminateMachineAndNull(_official);
		_official = triggerMachineByHash_3000(8, 5, *OFFICIAL_NORMAL_DIRS,
			*OFFICIAL_SHADOW_DIRS, 1436, 362, 9, triggerMachineByHashCallback3000,
			"official arresting");
		break;

	case 357:
		_flag1 = false;
		setGlobals1(_officialTurn9_11, 1, 19, 19, 19);
		sendWSMessage_110000(_official, 358);
		break;

	case 358:
		sendWSMessage_150000(_official, -1);
		ws_demand_facing(11);
		sendWSMessage_10000(_official, 1025, 290, 11, 359, 1);
		break;

	case 359:
		setGlobals1(_officialTurn11_3, 1, 39, 39, 39, 0, 39, 23, 23, 23, 0,
			23, 39, 39, 39, 0, 39, 39, 39, 39, 0);
		sendWSMessage_110000(_official, 360);
		break;

	case 360:
		sendWSMessage_120000(_official, 363);
		break;

	case 363:
		ws_demand_location(1100, 290, 9);
		ws_walk(1060, 290, nullptr, 36, 7);
		break;

	case 364:
		sendWSMessage_150000(_official, -1);
		ws_demand_facing(_official, 9);
		setGlobals1(_officialTurn3_7, 1, 27, 27, 27);
		sendWSMessage_110000(_official, 365);
		ws_walk(1062, 313, nullptr, 8888, 7);
		break;

	case 365:
		sendWSMessage_150000(_official, -1);
		ws_demand_facing(7);
		sendWSMessage_10000(_official, 1010, 345, 9, 366, 1);
		break;

	case 366:
		ws_walk(280, 345, nullptr, 103, 9);
		kernel_timing_trigger(50, 367);
		break;

	case 367:
		g_engine->camera_shift_xy(0, 0);
		sendWSMessage_10000(_official, 345, 355, 9, -1, 1);
		break;

	case 390:
		sendWSMessage_10000(1, _pigeons1, _pigeonsSeries1, 1, 88, -1,
			_pigeonsSeries1, 1, 88, 0);
		break;

	case 392:
		sendWSMessage_10000(1, _pigeons3, _pigeonsSeries3, 1, 93, -1,
			_pigeonsSeries3, 1, 93, 0);
		break;

	case 666:
		digi_play("203r56", 1, 255, 667);
		break;

	case 667:
		digi_unload("203r56");
		break;

	case 749:
		midi_stop();
		break;

	case 1968:
		series_unload(_ripLooksAtHeads);
		break;

	case 4567:
		series_unload(_meiTalkToRip);
		series_unload(_ripYouSeeToIt);
		series_unload(_meiTurnAndTalk);
		series_unload(S2_MEI_NORMAL_DIRS[4]);
		series_unload(S2_MEI_NORMAL_DIRS[3]);
		series_unload(S2_MEI_NORMAL_DIRS[2]);
		series_unload(S2_MEI_NORMAL_DIRS[1]);
		series_unload(S2_MEI_NORMAL_DIRS[0]);
		series_unload(S2_MEI_SHADOW_DIRS[4]);
		series_unload(S2_MEI_SHADOW_DIRS[3]);
		series_unload(S2_MEI_SHADOW_DIRS[2]);
		series_unload(S2_MEI_SHADOW_DIRS[1]);
		series_unload(S2_MEI_SHADOW_DIRS[0]);

		_peasantRocks = series_load("peasant rocks 5frames");
		_peasantRocksShadow = series_load("shadow pesky rock loop");
		ws_walk_load_walker_series(PEASANT_NORMAL_DIRS, PEASANT_NORMAL_NAMES);
		ws_walk_load_shadow_series(PEASANT_SHADOW_DIRS, PEASANT_SHADOW_NAMES);

		_peasantMode = 4050;
		_peasantShould = 4091;
		setupPeasantHotspot(4050);
		_peasantX = 745;
		_peasantY = 378;
		_peasantLayer = 0x800;
		_peasantScale = 75;

		_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, 0,
			triggerMachineByHashCallback, "pesky peasant");
		_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_peasantX, _peasantY - 53, _peasantScale, _peasantLayer, 0,
			triggerMachineByHashCallback, "pesky peasant shadow");
		sendWSMessage_10000(1, _peasantShadow, _peasantRocksShadow, 1, 1, -1,
			_peasantRocksShadow, 1, 1, 0);

		setupOfficial();
		setupOldLady();
		setupPigeons();
		kernel_timing_trigger(1, 130);
		kernel_timing_trigger(1, 120);
		kernel_timing_trigger(2, 30);
		break;

	case 6250:
		series_set_frame_rate(_stream1, 3000);
		kernel_timing_trigger(60, 81);
		break;

	case 8888:
		ws_walk(940, 345, nullptr, -1, 7);
		break;

	case 9000:
		setGlobals1(_ripHeadTurn, 1, 4, 4, 4);
		sendWSMessage_110000(9002);
		break;

	case 9002:
		digi_play("203r51", 1, 255, 9004);
		break;

	case 9004:
		_g1 = triggerMachineByHash_3000(8, 19, *SNORMAL1_DIRS, *SSHADOW1_DIRS, 660, 360, 9,
			triggerMachineByHashCallback3000, "g1");
		_g2 = triggerMachineByHash_3000(8, 20, *SNORMAL2_DIRS, *SSHADOW2_DIRS, 715, 370, 9,
			triggerMachineByHashCallback3000, "g2");
		sendWSMessage_10000(_g1, 340, 350, 9, 9040, 0);
		sendWSMessage_10000(_g2, -30, 360, 9, -1, 1);
		sendWSMessage_140000(-1);
		break;

	case 9005:
		sendWSMessage_10000(_g1, -30, 350, 9, -1, 1);
		_sg = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x500, 0,
			triggerMachineByHashCallback, "sg");
		sendWSMessage_10000(1, _sg, _203sg01, 1, 1, -1, _203sg01, 1, 1, 0);
		setGlobals1(_ripHeadTurn, 6, 9, 9, 9);
		sendWSMessage_110000(9006);
		break;

	case 9006:
		kernel_timing_trigger(60, 9007);
		break;

	case 9007:
		sendWSMessage_140000(9008);
		digi_play("com125", 1, 255, 9009);
		break;

	case 9008:
		ws_walk(417, 240, nullptr, -1, 1);
		kernel_timing_trigger(60, 9010);
		break;

	case 9009:
		digi_play("203r52", 1);
		break;

	case 9010:
		sendWSMessage_10000(_mei, 216, 332, 3, 9011, 0);
		kernel_timing_trigger(120, 9030);
		break;

	case 9011:
		sendWSMessage_10000(_mei, 417, 240, 2, -1, 1);
		break;

	case 9014:
		digi_play("203_s09", 2);
		sendWSMessage_10000(1, _sg, _203sg01, 85, 130, 9020, _203sg01, 130, 130, 0);
		break;

	case 9020:
		sendWSMessage_10000(1, _sg, _203sg01, 131, 155, -1, _203sg01, 155, 155, 0);
		disable_player_commands_and_fade_init(9025);
		break;

	case 9025:
		_G(game).setRoom(204);
		break;

	case 9030:
		_G(camera_reacts_to_player) = false;
		g_engine->camera_shift_xy(640, 0);
		kernel_timing_trigger(30, 9013);
		break;

	case 9040:
		series_stream("203pu99", 5, 0, -1);
		sendWSMessage_10000(_g1, 200, 350, 9, 9005, 0);
		break;

	default:
		error("Unhandled trigger");
		break;
	}
}

void Room203::setupHelmetHotspot() {
	for (HotSpotRec *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, "SOLDIER'S HELMET")) {
			hs->active = _G(flags)[V060] ?
				!inv_player_has("SOLDIER'S HELMET") && hs->lr_x > 1200 :
				hs->lr_x < 1200;
			break;
		}
	}
}

void Room203::setupPeasantHotspot(int mode) {
	for (HotSpotRec *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, "PEASANT")) {
			switch (mode) {
			case 4050:
			case 4051:
				hs->active = hs->lr_x > 450 && hs->lr_x < 800;
				break;
			case 4052:
				hs->active = hs->lr_x < 450;
				break;
			case 4053:
				hs->active = hs->lr_x > 800 && hs->lr_x < 1040;
				break;
			case 4054:
				hs->active = _peasantShould != 4150 && hs->lr_x > 1040;
				break;
			default:
				break;
			}
			break;
		}
	}
}

void Room203::setupGk() {
	_gkFrame = series_load("gk single frame");
	_gk = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x800, 0,
		triggerMachineByHashCallback, "gk single frame");
	_gkMode = _gkShould = 3000;
	sendWSMessage_10000(1, _gk, _gkFrame, 1, 1, -1, _gkFrame, 1, 1, 0);
	kernel_trigger_dispatchx(kernel_trigger_create(125));
}

void Room203::setupOldLady() {
	_oldLadyFrame = series_load("old lady - two single frames");

	if (_G(flags)[V060]) {
		hotspot_set_active("SOLDIER'S HELMET", false);
		_oldLadyNoHelmet = series_load("old lady feeding no helmet");
		_oldLadyMode = 5569;
		_oldLadyShould = 5200;
		_oldLady = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "ol");
		sendWSMessage_10000(1, _oldLady, _oldLadyNoHelmet, 1, 1, -1,
			_oldLadyNoHelmet, 1, 1, 0);

	} else {
		_oldLadyFeedingBirds = series_load("old lady feeding birds");
		_oldLady = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "ol");
		_oldLadyMode = 5666;
		_oldLadyShould = 5100;
		sendWSMessage_10000(1, _oldLady, _oldLadyFrame, 1, 1, -1,
			_oldLadyFrame, 1, 1, 0);
	}
}

void Room203::setupPigeons() {
	_pigeonsSeries1 = series_load("pigeon series of flcs 1");
	_pigeonsSeries3 = series_load("pigeon series of flcs 3");
	_pigeons1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1045, 373, 85, 0, 0,
		triggerMachineByHashCallback, "pig1");
	_pigeons3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1065, 370, 85, 1, 0,
		triggerMachineByHashCallback, "pig3");

	kernel_trigger_dispatchx(kernel_trigger_create(392));
}

void Room203::setupOfficial() {
	_officialMode = 2001;
	_officialShould = 2010;
	_officialStander = series_load("official stander");
	_official = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1436, 362, 85, 0xf00, 0,
		triggerMachineByHashCallback, "of");
	_officialShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1436, 362, 85, 0xf00, 0,
		triggerMachineByHashCallback, "ofSH");
	sendWSMessage_10000(1, _official, _officialStander, 1, 1, -1,
		_officialStander, 1, 1, 0);
	sendWSMessage_10000(1, _officialShadow, _shadow3, 1, 1, -1,
		_shadow3, 1, 1, 0);
}

void Room203::setupPeasant() {
	_peasantRocks = series_load("peasant rocks 5frames");
	_peasantRocksShadow = series_load("shadow pesky rock loop");

	ws_walk_load_walker_series(S2_PEASANT_NORMAL_DIRS, S2_PEASANT_NORMAL_NAMES);
	ws_walk_load_shadow_series(S2_PEASANT_SHADOW_DIRS, S2_PEASANT_SHADOW_NAMES);

	setupPeasantHotspot(_peasantMode);

	_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _peasantX, _peasantY - 53,
		_peasantScale, _peasantLayer, false, triggerMachineByHashCallback, "pp");
	sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120, _peasantRocks, 5, 5, 0);

	_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _peasantX, _peasantY - 53,
		_peasantScale, _peasantLayer, false, triggerMachineByHashCallback, "pp sh");
	sendWSMessage_10000(1, _peasant, _peasantRocksShadow, 1, 1, -1,
		_peasantRocksShadow, 1, 1, 0);
}

void Room203::peasantWalk() {
	player_update_info();

	switch (_peasantShould) {
	case 4113:
	case 4142:
	case 4162:
	case 4166:
		if (_G(player_info).y > 310 && _G(player_info).y < 330 &&
				_G(player_info).x > 735 && _G(player_info).x < 1140) {
			if (_G(player_info).facing == 1 || _G(player_info).facing == 2 ||
				_G(player_info).facing == 10 || _G(player_info).facing == 11)
				ws_walk(_G(player_info).x, 294, nullptr, -1, 0);
			else
				ws_walk(_G(player_info).x, 350, nullptr, -1, 0);
		}
		break;

	case 4117:
	case 4175:
		if (_G(player_info).y > 315 && _G(player_info).y < 335 &&
				_G(player_info).x > 373 && _G(player_info).x < 763) {
			ws_walk(_G(player_info).x, 350, nullptr, -1, 0);
		}
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
