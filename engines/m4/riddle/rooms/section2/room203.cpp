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
#include "m4/riddle/riddle.h"

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


void Room203::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_val1 = 0;

	hotspot_set_active("SOLDIER'S HELMET", false);
	setupHelmetHotspot();

	_val4 = _val5 = 0;
	_val6 = _val8 = 0;
	_ripley80000 = 0;
	_digiName1 = _digiName3 = nullptr;
	_showWalker = false;
	_flag2 = false;
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
			_yellThiefFlag = true;
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
	int frame;

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
		sendWSMessage_150000(-1);
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

	case 80:
		digi_unload("203r22");
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
		_flag2 = false;
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
		_yellThiefFlag = true;
		kernel_timing_trigger(100, 92);
		digi_preload("203p02");
		break;

	case 92:
		if (_yellThiefFlag)
			digi_play("203p02", 1, 255, 93);
		else
			digi_unload("203p02");
		break;

	case 93:
		kernel_timing_trigger(120, 92);
		break;

	case 94:
		if (_yellThiefFlag) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 19, 21, 95,
				_peskyYellsThief, 21, 21, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 19, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		}
		break;

	case 95:
		if (_yellThiefFlag) {
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
		_yellThiefFlag = false;
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

	case 121:
		switch (_peasantMode) {
		case 4050:
			switch (_peasantShould) {
			case 4091:
				sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120,
					_peasantRocks, 5, 5, 0);
				break;

			case 4094:
				_peasantSeries = series_load("pesky peasant reach for helmet");
				_peasantSeriesShadow = series_load("shadow pp reach for helmet");
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);

				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4095;
				break;

			case 4095:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS,
					763, 325, 3, triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 1200, 332, 2, 120, 1);
				_peasantMode = 4054;
				_peasantShould = 4148;
				break;

			default:
				break;
			}
			break;

		case 4051:
			switch (_peasantShould) {
			case 4110:
				sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120,
					_peasantRocks, 5, 5, 0);
				break;

			case 4113:
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");
				_peasantSquat3 = series_load("peasant 3 to squat");
				_peasantSquat9 = series_load("shadow pesky from 9 to squat");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4114;
				break;

			case 4114:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS,
					763, 325, 3, triggerMachineByHashCallback3000, "pp_walk");
				sendWSMessage_10000(_peasant, 932, 325, 3, 121, 1);
				_peasantShould = 4115;
				break;

			case 4115:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim2();
				_peasantShould = 4116;
				break;

			case 4116:
				_peasantMode = 4053;
				_peasantShould = 4160;
				setupPeasantHotspot(_peasantMode);
				series_unload(_peasantSquat9);
				series_unload(_peasantSquat3);
				series_unload(_peasantFromSquat3);
				series_unload(_peasantSquatTo9);

				if (!_flag2)
					player_set_commands_allowed(true);

				kernel_timing_trigger(1, 120);
				break;

			case 4117:
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow peskey from squat to pos3");
				_peasantSquat3 = series_load("peasant 3 to squat");
				_peasantSquat9 = series_load("shadow pesky from 9 to squat");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");
				_peasantShould = 4118;
				break;

			case 4118:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS,
					727, 325, 9, triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 436, 332, 9, 121, 1);
				_peasantShould = 4119;
				break;

			case 4119:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 418, 332, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 418, 332, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");

				peasantAnim2();
				_peasantShould = 4120;
				break;

			case 4120:
				_peasantMode = 4052;
				_peasantShould = 4170;
				setupPeasantHotspot(_peasantMode);
				series_unload(_peasantSquat9);
				series_unload(_peasantSquat3);
				series_unload(_peasantFromSquat3);
				series_unload(_peasantSquatTo9);
				player_set_commands_allowed(true);
				kernel_timing_trigger(1, 120);
				break;

			default:
				break;
			}
			break;

		case 4052:
			switch (_peasantShould) {
			case 4170:
				sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120,
					_peasantRocks, 5, 5, 0);
				break;

			case 4175:
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");
				_peasantSquat3 = series_load("peasant 3 to squat");
				_peasantSquat9 = series_load("shadow pesky from 9 to squat");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 418, 332, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 418, 332, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4176;
				break;

			case 4176:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS, 436, 332, 3,
					triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 727, 325, 3, 121, 1);
				_peasantShould = 4177;
				break;

			case 4177:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim2();
				_peasantShould = 4178;
				break;

			case 4178:
				series_unload(_peasantSquat9);
				series_unload(_peasantSquat3);
				series_unload(_peasantFromSquat3);
				series_unload(_peasantSquatTo9);
				_peasantMode = 4051;
				_peasantShould = 4110;
				setupPeasantHotspot(_peasantMode);
				player_set_commands_allowed(true);
				kernel_timing_trigger(1, 120);
				break;

			default:
				break;
			}
			break;

		case 4053:
			switch (_peasantShould) {
			case 4160:
				sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120,
					_peasantRocks, 5, 5, 0);
				break;

			case 4162:
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");
				_peasantSquat3 = series_load("peasant 3 to squat");
				_peasantSquat9 = series_load("shadow pesky from 9 to squat");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4163;
				break;

			case 4163:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS, 932, 325, 9,
					triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 763, 325, 9, 121, 1);
				_peasantShould = 4164;
				break;

			case 4164:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 745, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim2();
				_peasantShould = 4165;
				break;

			case 4165:
				_peasantMode = 4051;
				_peasantShould = 4110;
				setupPeasantHotspot(_peasantMode);
				series_unload(_peasantSquat9);
				series_unload(_peasantSquat3);
				series_unload(_peasantFromSquat3);
				series_unload(_peasantSquatTo9);
				player_set_commands_allowed(true);
				kernel_timing_trigger(1, 120);
				break;

			case 4166:
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");
				_peasantSquat3 = series_load("peasant 3 to squat");
				_peasantSquat9 = series_load("shadow pesky from 9 to squat");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4167;
				break;

			case 4167:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS, 968, 325, 3,
					triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 1082, 322, 3, 121, 1);
				_peasantShould = 4168;
				break;

			case 4168:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim2();
				_peasantShould = 4169;
				break;

			case 4169:
				_peasantMode = 4054;
				_peasantShould = 4140;
				setupPeasantHotspot(_peasantMode);
				series_unload(_peasantSquat9);
				series_unload(_peasantSquat3);
				series_unload(_peasantFromSquat3);
				series_unload(_peasantSquatTo9);

				if (!_G(flags)[V061] && !_flag2)
					player_set_commands_allowed(true);

				kernel_timing_trigger(1, 120);
				break;

			default:
				break;
			}
			break;

		case 4054:
			switch (_peasantShould) {
			case 4140:
				sendWSMessage_10000(1, _peasant, _peasantRocks, 1, 5, 120,
					_peasantRocks, 5, 5, 0);
				break;

			case 4142:
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");
				_peasantSquat3 = series_load("peasant 3 to squat");
				_peasantSquat9 = series_load("shadow pesky from 9 to squat");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4143;
				break;

			case 4143:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS, 1082, 322, 9,
					triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 968, 325, 9, 121, 1);
				_peasantShould = 4144;
				break;

			case 4144:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 950, 325, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim2();
				_peasantShould = 4145;
				break;

			case 4145:
				_peasantMode = 4053;
				_peasantShould = 4160;
				setupPeasantHotspot(_peasantMode);

				series_unload(_peasantSquat9);
				series_unload(_peasantSquat3);
				series_unload(_peasantFromSquat3);
				series_unload(_peasantSquatTo9);

				if (!_flag2)
					player_set_commands_allowed(true);
				kernel_timing_trigger(1, 120);
				break;

			case 4146:
				_peasantSeries = series_load("pesky peasant reach for helmet");
				_peasantSeriesShadow = series_load("shadow pp reach for helmet");
				_peasantSquatTo9 = series_load("peasant squat to 9");
				_peasantFromSquat3 = series_load("shadow pesky from squat to pos3");

				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1100, 322, 75, 0x800, true,
					triggerMachineByHashCallback, "pesky peasant shadow");
				peasantAnim1();
				_peasantShould = 4147;
				break;

			case 4147:
				terminateMachineAndNull(_peasant);
				terminateMachineAndNull(_peasantShadow);
				_peasant = triggerMachineByHash_3000(8, 6,
					*S2_PEASANT_NORMAL_DIRS, *S2_PEASANT_SHADOW_DIRS, 1118, 322, 3,
					triggerMachineByHashCallback3000, "pp_walking");
				sendWSMessage_10000(_peasant, 1200, 332, 2, 121, 1);
				_peasantShould = 4148;
				break;

			case 4148:
				sendWSMessage_60000(_peasant);
				_peasant = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1240, 332, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant");
				_peasantShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 1240, 332, 75, 0x800, false,
					triggerMachineByHashCallback, "pesky peasant shadow");

				sendWSMessage_10000(1, _peasant, _peasantSeries, 1, 36, 121,
					_peasantSeries, 36, 23, 2);
				sendWSMessage_10000(1, _peasantShadow, _peasantSeriesShadow, 1, 36, -1,
					_peasantSeriesShadow, 36, 23, 2);
				_peasantShould = 4149;
				break;

			case 4149:
				series_unload(_peasantSquatTo9);
				series_unload(_peasantFromSquat3);
				player_set_commands_allowed(true);
				_peasantMode = 4054;
				_peasantShould = 4150;
				setupPeasantHotspot(_peasantMode);
				kernel_timing_trigger(1, 120);
				break;

			case 4150:
				sendWSMessage_10000(1, _peasant, _peasantSeries, 23, 36, 121,
					_peasantSeries, 36, 36, 0);
				sendWSMessage_10000(1, _peasantShadow, _peasantSeriesShadow, 23, 36, -1,
					_peasantSeriesShadow, 36, 36, 0);
				_peasantShould = 4151;
				break;

			case 4151:
				sendWSMessage_10000(1, _peasant, _peasantSeries, 36, 23, 120,
					_peasantSeries, 23, 23, 0);
				sendWSMessage_10000(1, _peasantShadow, _peasantSeriesShadow, 36, 23, -1,
					_peasantSeriesShadow, 23, 23, 0);
				_peasantShould = 4150;
				break;

			default:
				break;
			}
			break;

		case 4055:
			sendWSMessage_10000(1, _peasant, _peskyBegLoop, 1, 16, 120,
				_peskyBegLoop, 16, 16, 0);
			_peasantMode = 4057;
			break;

		case 4056:
			frame = imath_ranged_rand(17, 19);
			sendWSMessage_10000(1, _peasant, _peskyBegLoop, frame, frame, 120,
				_peskyBegLoop, frame, frame, 0);
			break;

		case 4057:
			sendWSMessage_10000(1, _peasant, _peskyBegLoop, 16, 16, 120,
				_peskyBegLoop, 16, 16, 0);
			break;

		case 4058:
			sendWSMessage_10000(1, _peasant, _peskyBegLoop, 16, 1, 121,
				_peasantRocks, 1, 1, 0);
			_peasantMode = 4059;
			break;

		case 4059:
			series_unload(_peskyBegLoop);

			switch (_peasantMode2) {
			case 4052:
				_peasantMode = 4052;
				_peasantShould = 4170;
				break;
			case 4053:
				_peasantMode = 4053;
				_peasantShould = 4160;
				break;
			case 4054:
				_peasantMode = 4054;
				_peasantShould = 4140;
				break;
			default:
				_peasantMode = 4051;
				_peasantShould = 4110;
				break;
			}

			kernel_timing_trigger(1, 120);
			player_set_commands_allowed(true);
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

	case 130:
		switch (_oldLadyMode) {
		case 30:
			break;

		case 5666:
			if (_trigger3 != -1) {
				kernel_trigger_dispatchx(kernel_trigger_create(_trigger3));
				_trigger3 = -1;
			} else if (_oldLadyShould == 5100) {
				if (imath_ranged_rand(1, 80) < 20)
					_oldLadyShould = 5101;

				kernel_timing_trigger(30, 131);
			}
			break;

		case 5668:
			if (_trigger3 != -1) {
				kernel_trigger_dispatchx(kernel_trigger_create(_trigger3));
				_trigger3 = -1;
			} else {
				// Original has a whole switch statement, but all cases lead
				// to the same trigger
				kernel_trigger_dispatchx(kernel_trigger_create(131));
			}
			break;

		case 5669:
			if (_trigger3 != -1) {
				kernel_trigger_dispatchx(kernel_trigger_create(_trigger3));
				_trigger3 = -1;
			} else {
				switch (_oldLadyShould) {
				case 5200:
					if (imath_ranged_rand(1, 80) < 20)
						_oldLadyShould = 5201;
					break;

				case 5202:
					_oldLadyShould = 5200;
					break;

				default:
					break;
				}

				kernel_timing_trigger(30, 131);
			}
			break;

		default:
			kernel_trigger_dispatchx(kernel_trigger_create(131));
			break;
		}
		break;

	case 131:
		switch (_oldLadyMode) {
		case 5666:
			switch (_oldLadyShould) {
			case 5100:
				sendWSMessage_10000(1, _oldLady, _oldLadyFrame, 1, 1, 130, _oldLadyFrame, 1, 1, 0);
				break;

			case 5101:
				sendWSMessage_10000(1, _oldLady, _oldLadyFeedingBirds, 1, 26, 131,
					_oldLadyFeedingBirds, 26, 26, 0);
				_oldLadyShould = 5102;
				break;

			case 5102:
				sendWSMessage_10000(1, _oldLady, _oldLadyFeedingBirds, 26, 1, 131,
					_oldLadyFrame, 1, 1, 0);
				_oldLadyShould = 5100;
				break;

			default:
				break;
			}
			break;

		case 5668:
			switch (_oldLadyShould) {
			case 5301:
				sendWSMessage_10000(1, _oldLady, _oldLady1, 1, 6, 130, _oldLady1, 6, 6, 0);
				_oldLadyShould = 5302;
				break;

			case 5302:
				sendWSMessage_10000(1, _oldLady, _oldLady1, 6, 6, 130, _oldLady1, 6, 6, 0);
				break;

			case 5303:
				sendWSMessage_10000(1, _oldLady, _oldLady1, 6, 6, 131, _oldLady1, 6, 6, 0);
				_oldLadyShould = 5304;
				break;

			case 5304:
				sendWSMessage_10000(1, _oldLady, _oldLady1, 6, 6, 131, _oldLady1, 6, 6, 0);
				_oldLadyShould = 5305;
				break;

			case 5305:
				_ripleyShould = 1140;
				_oldLadyShould = 5302;
				kernel_trigger_dispatchx(kernel_trigger_create(130));
				break;

			case 5306:
				sendWSMessage_10000(1, _oldLady, _oldLady1, 6, 1, 131, _oldLadyFrame, 1, 1, 0);
				_oldLadyShould = 5307;
				break;

			case 5307:
				series_unload(_oldLady1);
				_oldLadyMode = 5666;
				_oldLadyShould = 5100;
				kernel_timing_trigger(120, 130);
				break;

			case 5308:
				sendWSMessage_10000(1, _oldLady, _oldLady1, 6, 6, -1, _oldLady1, 6, 6, 0);
				break;

			default:
				break;
			}
			break;

		case 5669:
			switch (_oldLadyShould) {
			case 5200:
				sendWSMessage_10000(1, _oldLady, _oldLadyNoHelmet, 1, 1, 130,
					_oldLadyNoHelmet, 1, 1, 0);
				break;

			case 5201:
				sendWSMessage_10000(1, _oldLady, _oldLadyNoHelmet, 1, 36, 131,
					_oldLadyNoHelmet, 36, 36, 0);
				_oldLadyShould = 5202;
				break;

			case 5202:
				sendWSMessage_10000(1, _oldLady, _oldLadyNoHelmet, 36, 1, 130,
					_oldLadyFrame, 2, 2, 0);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 140:
		switch (_officialMode) {
		case 2001:
			switch (_officialShould) {
			case 2010:
				if (_trigger4 != -1) {
					kernel_trigger_dispatchx(_trigger4);
					_trigger4 = -1;
				} else {
					kernel_timing_trigger(2, 141);
				}
				break;

			case 2020:
			case 2040:
				kernel_timing_trigger(2, 141);
				break;

			default:
				break;
			}
			break;

		case 2002:
			switch (_officialShould) {
			case 2011:
			case 2013:
			case 2014:
			case 2015:
			case 2016:
			case 2017:
				kernel_trigger_dispatchx(kernel_trigger_create(141));
				break;

			case 2012:
				kernel_timing_trigger(1, 141);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 141:
		switch (_officialMode) {
		case 2001:
			switch (_officialShould) {
			case 2010:
				sendWSMessage_10000(1, _official, _officialStander, 1, 1, -1,
					_officialStander, 1, 1, 0);
				break;

			case 2020:
				ws_hide_walker();
				_ripHandTalk = series_load("rip trek hand talk pos3");

				player_update_info();
				_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale,
					0x100, 0, triggerMachineByHashCallback, "rip talks official");
				_ripsh1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale,
					0x100, 0, triggerMachineByHashCallback, "rip talks official SHADOW");
				sendWSMessage_10000(1, _ripley, _ripHandTalk, 1, 16, -1, _ripHandTalk, 1, 1, 0);
				sendWSMessage_10000(1, _ripsh1, _shadow3, 1, 1, -1, _shadow3, 1, 1, 0);

				_officialShould = 2023;
				digi_play("03_02p01", 1, 255, 141);
				break;

			case 2023:
				_officialHalt = series_load("official halt");
				sendWSMessage_10000(1, _official, _officialHalt, 1, 15, 141,
					_officialHalt, 15, 15, 0);
				digi_play("03_02n01", 1);
				break;

			case 2024:
				sendWSMessage_10000(1, _official, _officialHalt, 15, 1, 141,
					_officialStander, 1, 1, 0);
				_officialShould = 2025;
				break;

			case 2025:
				series_unload(_officialHalt);
				series_unload(_ripHandTalk);
				_ripArmsX = series_load("rip trek arms x pos3");
				_officialThroughThere = series_load("official through there");
				sendWSMessage_10000(1, _ripley, _ripArmsX, 1, 15, -1, _ripArmsX, 15, 15, 0);

				_officialShould = 2026;
				digi_play("03_06p02", 1, 255, 141);
				break;

			case 2026:
				sendWSMessage_10000(1, _official, _officialThroughThere, 1, 14, 141,
					_officialThroughThere, 14, 14, 0);
				_officialShould = 2027;
				digi_play("03_06n02", 1);
				break;

			case 2027:
				sendWSMessage_10000(1, _official, _officialThroughThere, 14, 1, 141,
					_officialStander, 1, 1, 0);
				_officialShould = 2028;
				break;

			case 2028:
				series_unload(_officialThroughThere);
				_ripHeadDownTalkOff = series_load("rip head down talk off td33");
				sendWSMessage_10000(1, _ripley, _ripHeadDownTalkOff, 1, 5, -1,
					_ripHeadDownTalkOff, 3, 5, 1);
				_officialShould = 2029;
				digi_play("03_07p03", 1, 255, 141);
				break;

			case 2029:
				sendWSMessage_10000(1, _ripley, _ripHeadDownTalkOff, 5, 1, 141,
					_ripArmsX, 15, 15, 0);
				_officialShould = 2030;
				break;

			case 2030:
				sendWSMessage_10000(1, _ripley, _ripArmsX, 15, 1, 141, _ripArmsX, 1, 1, 0);
				_officialShould = 2031;
				break;

			case 2031:
				series_unload(_ripArmsX);
				terminateMachineAndNull(_ripley);
				terminateMachineAndNull(_ripsh1);
				ws_unhide_walker();
				_officialShould = 2010;
				kernel_trigger_dispatchx(kernel_trigger_create(140));
				player_set_commands_allowed(true);
				break;

			case 2040:
				_officialHalt = series_load("official halt");

				if (_digiName3) {
					digi_play(_digiName3, 1, 255, _digiTrigger3);
					_digiName3 = nullptr;
				}

				sendWSMessage_10000(1, _official, _officialHalt, 1, 15, 141,
					_officialHalt, 15, 15, 0);
				_officialShould = 2042;
				break;

			case 2042:
				sendWSMessage_10000(1, _official, _officialHalt, 15, 1, 141,
					_officialStander, 1, 1, 0);
				_officialShould = 2043;
				break;

			case 2043:
				series_unload(_officialHalt);
				_officialShould = 2010;
				kernel_trigger_dispatchx(kernel_trigger_create(140));
				break;

			default:
				break;
			}
			break;

		case 2002:
			switch (_officialShould) {
			case 2011:
			case 2012:
				sendWSMessage_10000(1, _official, _officialStander, 1, 1, 140,
					_officialStander, 1, 1, 0);
				_officialShould = 2012;
				break;

			case 2013:
				frame = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _official, _officialStander, 1, frame, 140,
					_officialStander, frame, frame, 0);
				break;

			case 2014:
				sendWSMessage_10000(1, _official, _officialThroughThere, 1, 14, 141,
					_officialThroughThere, 14, 14, 0);
				_officialShould = 2015;
				break;

			case 2015:
				sendWSMessage_10000(1, _official, _officialThroughThere, 14, 1, 140,
					_officialStander, 1, 1, 0);
				_officialShould = 2016;
				break;

			case 2016:
				_officialShould = 2012;
				kernel_trigger_dispatchx(kernel_trigger_create(140));
				conv_resume();
				break;

			case 2017:
				sendWSMessage_10000(1, _official, _officialStander, 1, 1, -1,
					_officialStander, 1, 1, 0);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 150:
		if (_ripleyMode == 1001 && _ripleyShould == 1030 && _trigger5 != -1) {
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

	case 151:
		switch (_ripleyMode) {
		case 1001:
			switch (_ripleyShould) {
			case 1010:
				conv_load("conv203d", 10, 10, 152);
				conv_export_value_curr(_G(flags)[V054], 0);
				conv_export_value_curr(0, 1);
				conv_export_value_curr(0, 2);
				conv_export_value_curr(_G(flags)[V073], 3);
				conv_export_value_curr(_G(flags)[V043], 5);
				conv_play();
				_ripleyShould = 1030;
				kernel_trigger_dispatchx(kernel_trigger_create(150));
				break;

			case 1020:
			case 1030:
				sendWSMessage_10000(1, _ripley, _ripHandsBehBack, 11, 11, 150,
					_ripHandsBehBack, 11, 11, 0);
				break;

			case 1040:
				_ripleyShould = 1060;
				sendWSMessage_10000(1, _ripley, _ripHandsBehBack, 11, 1, 151,
					_ripHandsBehBack, 1, 1, 0);
				break;

			case 1060:
				terminateMachineAndNull(_ripley);
				terminateMachineAndNull(_ripsh1);
				series_unload(_ripHandsBehBack);
				_peasantMode = 4058;
				ws_unhide_walker();
				break;

			default:
				break;
			}
			break;

		case 1002:
			switch (_ripleyShould) {
			case 1110:
				ws_hide_walker();
				player_set_commands_allowed(false);
				_oldLady1 = series_load("old lady");
				_ripKneeling = series_load("rip kneels down to old lady");
				_ripKneelingTalk = series_load("rip kneeling talks to old lady");

				_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0x400, 0,
					triggerMachineByHashCallback, "rip in conv");
				sendWSMessage_10000(1, _ripley, _ripKneeling, 1, 31, 150, _ripKneeling, 31, 31, 0);
				_ripleyShould = 1112;
				_oldLadyMode = 5568;
				_oldLadyShould = 5301;
				kernel_trigger_dispatchx(kernel_trigger_create(130));
				break;

			case 1112:
				sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 1, 4, 150,
					_ripKneelingTalk, 4, 4, 0);
				_ripleyShould = 1113;
				break;

			case 1113:
				conv_load("conv203e", 10, 10, 152);
				conv_export_value_curr(_G(flags)[V051], 0);
				conv_play();
				_ripleyShould = 1130;
				kernel_trigger_dispatchx(kernel_trigger_create(150));
				break;

			case 1120:
				frame = imath_ranged_rand(5, 9);
				sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 1, frame, 150,
					_ripKneelingTalk, frame, frame, 0);
				break;

			case 1121:
				_oldLadyProtectsHelmet = series_load("old lady protect helmet");
				_ripPointsAtHelmet = series_load("rip points at helmet");
				_ripleyShould = 1122;

				sendWSMessage_10000(1, _ripley, _ripPointsAtHelmet, 1, 5, 151,
					_ripPointsAtHelmet, 5, 5, 0);
				_oldLadyShould = 5308;
				break;

			case 1122:
				_ripleyShould = 1123;
				digi_play(_digiName2.c_str(), 1, 255, 151);
				break;

			case 1123:
				_ripleyShould = 1124;
				sendWSMessage_10000(1, _ripley, _ripPointsAtHelmet, 5, 1, 151,
					_ripKneelingTalk, 4, 4, 0);
				break;

			case 1124:
				_ripleyShould = 1125;
				sendWSMessage_10000(1, _oldLady, _oldLadyProtectsHelmet, 1, 16, 151,
					_oldLadyProtectsHelmet, 16, 16, 0);
				break;

			case 1125:
				_ripleyShould = 1126;
				sendWSMessage_10000(1, _oldLady, _oldLadyProtectsHelmet, 16, 1, 151,
					_oldLady1, 6, 6, 0);
				break;

			case 1126:
				_ripleyShould = 1127;
				series_stream("old woman cries for mommie", 5, 0, 151);
				break;

			case 1127:
				_ripleyShould = 1128;
				digi_preload("203r56");
				kernel_timing_trigger(30, 666);
				series_stream("rip blinks", 10, 0, 151);
				break;

			case 1128:
				series_unload(_ripPointsAtHelmet);
				series_unload(_oldLadyProtectsHelmet);
				_oldLadyShould = 5302;
				kernel_trigger_dispatchx(kernel_trigger_create(130));
				_ripleyShould = 1130;
				kernel_timing_trigger(1, 150);
				conv_resume();
				break;

			case 1130:
				sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 4, 4, 150,
					_ripKneelingTalk, 4, 4, 0);
				break;

			case 1140:
				_ripleyShould = 1142;
				sendWSMessage_10000(1, _ripley, _ripKneelingTalk, 4, 1, 151,
					_ripKneelingTalk, 1, 1, 0);
				break;

			case 1142:
				_ripleyShould = 1143;
				sendWSMessage_10000(1, _ripley, _ripKneeling, 31, 1, 151, _ripKneeling, 1, 1, 0);
				break;

			case 1143:
				terminateMachineAndNull(_ripley);
				series_unload(_ripKneelingTalk);
				series_unload(_ripKneeling);
				_oldLadyShould = 5306;
				ws_unhide_walker();
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
			break;

		case 1003:
			switch (_ripleyShould) {
			case 1210:
				ws_hide_walker();
				player_set_commands_allowed(false);
				_ripHandTalk = series_load("rip trek hand talk pos3");
				_ripTalker = series_load("rip trek talker pos3");
				_officialThroughThere = series_load("official through there");

				player_update_info();
				_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale,
					0x100, 0, triggerMachineByHashCallback, "rip talks official");
				_ripsh1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale,
					0x100, 0, triggerMachineByHashCallback, "rip talks official SHADOW");
				sendWSMessage_10000(1, _ripley, _ripTalker, 1, 1, -1, _ripTalker, 1, 1, 0);
				sendWSMessage_10000(1, _ripsh1, _shadow3, 1, 1, 150, _shadow3, 1, 1, 0);
				_ripleyShould = 1211;
				_officialMode = 2002;
				_officialShould = 2011;
				kernel_trigger_dispatchx(kernel_trigger_create(140));
				break;

			case 1211:
				conv_load("conv203c", 10, 10, 152);
				conv_export_value_curr(_G(flags)[V070], 0);
				conv_export_pointer_curr(&_G(flags)[V071], 1);
				conv_play();
				_ripleyShould = 1230;
				kernel_trigger_dispatchx(kernel_trigger_create(150));
				break;

			case 1220:
				frame = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _ripley, _ripTalker, 1, frame, 150,
					_ripTalker, frame, frame, 0);
				break;

			case 1221:
				sendWSMessage_10000(1, _ripley, _ripHandTalk, 1, 16, 150,
					_ripHandTalk, 1, 1, 0);
				_ripleyShould = 1230;
				break;

			case 1230:
				sendWSMessage_10000(1, _ripley, _ripTalker, 1, 1, 150, _ripTalker, 1, 1, 0);
				break;

			case 1240:
				terminateMachineAndNull(_ripley);
				terminateMachineAndNull(_ripsh1);
				series_unload(_officialThroughThere);
				series_unload(_ripTalker);
				series_unload(_ripHandTalk);
				_officialShould = 2017;
				ws_unhide_walker();
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 152:
		_flag2 = false;

		switch (_ripleyMode) {
		case 1001:
			_ripleyShould = 1040;
			break;
		case 1002:
			_ripleyShould = (_unkShould == 1121) ? 1121 : 5303;
			break;
		case 1003:
			_ripleyShould = 1240;
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
			ws_walk_load_walker_series(S2_PEASANT_NORMAL_DIRS, S2_PEASANT_NORMAL_NAMES);
			ws_walk_load_shadow_series(S2_PEASANT_SHADOW_DIRS, S2_PEASANT_SHADOW_NAMES);
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
		if (_yellThiefFlag)
			digi_play("203p02", 1, 255, 93);
		else
			digi_unload("203p02");
		break;

	case 353:
		kernel_timing_trigger(120, 352);
		break;

	case 354:
		if (_yellThiefFlag) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 17, 21, 355,
				_peskyYellsThief, 21, 21, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 17, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		}
		break;

	case 355:
		if (_yellThiefFlag) {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 17, 354,
				_peskyYellsThief, 17, 17, 0);
		} else {
			sendWSMessage_10000(1, _peasant, _peskyYellsThief, 21, 1, -1,
				_peskyYellsThief, 1, 1, 0);
		}
		break;

	case 356:
		terminateMachineAndNull(_official);
		_official = triggerMachineByHash_3000(8, 5, *OFFICIAL_NORMAL_DIRS,
			*OFFICIAL_SHADOW_DIRS, 1436, 362, 9, triggerMachineByHashCallback3000,
			"official arresting");
		sendWSMessage_10000(_official, 1065, 355, 9, 357, 1);
		break;

	case 357:
		_yellThiefFlag = false;
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
		ws_walk(1060, 290, nullptr, 364, 7);
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
		ws_walk_load_walker_series(S2_PEASANT_NORMAL_DIRS, S2_PEASANT_NORMAL_NAMES);
		ws_walk_load_shadow_series(S2_PEASANT_SHADOW_DIRS, S2_PEASANT_SHADOW_NAMES);

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

	case 9013:
		if (g_engine->game_camera_panning()) {
			kernel_timing_trigger(30, 9013);
		} else {
			sendWSMessage_10000(1, _sg, _203sg01, 1, 84, 9014, _203sg01, 84, 84, 0);
		}
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
		if (_G(kernel).trigger < 9999)
			error("Unhandled trigger");
		break;
	}
}

void Room203::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");

	if (lookFlag && player_said(" "))
		_G(player).resetWalk();

	if (lookFlag && player_said("ALLEY WALL") && _G(kernel).trigger == -1) {
		_G(player).resetWalk();
		_G(kernel).trigger_mode = KT_PARSE;
		ws_walk(710, 300, nullptr, 230, 11);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}
}

void Room203::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");

	if (player_said("conv203c")) {
		if (_G(kernel).trigger == 1) {
			_ripleyShould = 1230;
			_officialShould = 2012;
			conv_resume();
		} else {
			conv203c();
		}
	} else if (player_said("conv203d")) {
		conv203d();
	} else if (player_said("conv203e")) {
		conv203e();
	} else if (player_said("enter bsa")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			_G(game).setRoom(207);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("ARCHWAY")) {
		_G(flags)[V073] = 1;
		digi_play("203R15", 1);
	} else if (player_said("archway")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_G(flags)[V073] = 1;
			_officialMode = 2001;
			_officialShould = 2040;
			_digiName3 = "203O01";
			_digiTrigger3 = -1;
			_trigger4 = kernel_trigger_create(2);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(140));
			_G(kernel).trigger_mode = KT_PARSE;
			break;
		case 1:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("leave")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			_G(game).setRoom(201);
			break;
		default:
			break;
		}
	} else if (player_said("pass", "gatekeeper")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripHeadTurn = series_load("203pass");
			setGlobals1(_ripHeadTurn, 1, 6, 6, 6);
			sendWSMessage_110000(2);
			break;
		case 1:
			kernel_timing_trigger(60, 7);
			break;
		case 3:
			ws_walk(400, 252, nullptr, 5, 1);
			break;
		case 5:
			disable_player_commands_and_fade_init(5);
			break;
		case 6:
			_G(game).setRoom(204);
			break;
		default:
			break;
		}
	} else if (player_said("walk through")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_gkMayNotPass = series_load("gk may not pass");
			_gkMode = 3004;
			_digiName1 = "203g03";
			_digiTrigger1 = -1;
			_trigger1 = kernel_trigger_create(2);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(125));
			_G(kernel).trigger_mode = KT_PARSE;
			break;
		case 2:
			series_unload(_gkMayNotPass);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("HALL OF CLASSICS")) {
		digi_play("203r12", 1);
	} else if (lookFlag && player_said("GATEKEEPER")) {
		digi_play("203r06", 1);
	} else if (lookFlag && player_said("OLD LADY")) {
		player_set_commands_allowed(false);
		playSound("203r07", _G(kernel).trigger);
	} else if (lookFlag && player_said("SOLDIER'S HELMET ")) {
		digi_play("203r09a", 1);
	} else if (lookFlag && player_said("SOLDIER'S HELMET") &&
			!inv_player_has("SOLDIER'S HELMET") && !_G(flags)[V060]) {
		_G(flags)[V051] = 1;
		player_set_commands_allowed(false);
		playSound("203r09", _G(kernel).trigger);
	} else if (lookFlag && player_said("PEASANT")) {
		digi_play("203r08", 1);
	} else if (lookFlag && player_said("tree")) {
		digi_play("203r63", 1);
	} else if (player_said("SHRUNKEN HEAD", "BASKET")) {
		digi_play("203r65", 1);
	} else if (player_said("SOLDIER'S HELMET", "BASKET") && inv_player_has("SOLDIER'S HELMET")) {
		digi_play("203r59", 1);
	} else if (lookFlag && player_said("BASKET")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripTugsAtCollar = series_load("rip tugs at collar");
			setGlobals1(_ripTugsAtCollar, 1, 9, 9, 9, 0, 9, 1, 1, 1);
			sendWSMessage_110000(211);
			digi_play("203CLRT1", 1);
			break;
		case 211:
			sendWSMessage_120000(212);
			break;
		case 212:
			sendWSMessage_150000(213);
			break;
		case 213:
			series_unload(_ripTugsAtCollar);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("BLACKSMITH'S WINDOW")) {
		_G(flags)[V054] = 1;
		_G(flags)[V036] = 1;
		digi_play("203R34", 1);
	} else if (lookFlag && player_said("sign")) {
		digi_play("203R55", 1);
	} else if (lookFlag && player_said("pigeons")) {
		digi_play("203R57", 1);
	} else if (lookFlag && player_said("HERBAL SHOP")) {
		digi_play("203R11", 1);
	} else if (lookFlag && player_said("OFFICIAL")) {
		digi_play("203R13", 1);
	} else if (lookFlag && player_said("ALLEY WALL")) {
		if (_G(kernel).trigger != -1)
			lookThroughHole("203r14", _G(kernel).trigger);
	} else if (lookFlag && player_said("AWNING")) {
		digi_play("203R16", 1);
	} else if (lookFlag && player_said("WOODPILE")) {
		digi_play("203R35", 1);
	} else if (lookFlag && player_said("WINDOW")) {
		switch (_G(kernel).trigger) {
		case -1:
			_G(flags)[V072] = _peasantMode;
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			_G(game).setRoom(202);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("DOOR")) {
		digi_play("203R37", 1);

	} else if (talkFlag && player_said("OLD LADY")) {
		g_engine->camera_shift_xy(760, 0);
		player_set_commands_allowed(false);
		_flag2 = true;

		if (_G(flags)[V060]) {
			playSound("203r23", _G(kernel).trigger);
		} else {
			_ripleyMode = 1002;
			_ripleyShould = 1110;
			_trigger3 = 153;
		}
	} else if (talkFlag && player_said("PEASANT")) {
		ws_hide_walker();
		player_set_commands_allowed(false);
		player_update_info();
		_ripsh1 = series_place_sprite("ripsh1", 0, _G(player_info).x, _G(player_info).y,
			_G(player_info).scale, 0x500);
		_peskyBegLoop = series_load("pesky beg loop");
		_ripHandsBehBack = series_load("rip trek hands beh back pos1");

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x400, 0,
			triggerMachineByHashCallback, "rip in conv");
		_ripleyMode = 1001;
		_ripleyShould = 1010;
		sendWSMessage_10000(1, _ripley, _ripHandsBehBack, 1, 11, 150,
			_ripHandsBehBack, 11, 11, 0);

		_G(kernel).trigger_mode = KT_PARSE;
		_peasantMode2 = _peasantMode;
		_peasantMode = 4055;
	} else if (talkFlag && player_said("OFFICIAL")) {
		player_set_commands_allowed(false);
		_G(flags)[V073] = 1;
		_trigger4 = -1;
		_G(kernel).trigger_mode = KT_DAEMON;

		if (_G(flags)[V071]) {
			_ripleyMode = 1003;
			_ripleyShould = 1210;
			kernel_trigger_dispatchx(kernel_trigger_create(150));
			_G(kernel).trigger_mode = KT_PARSE;
		} else {
			_officialMode = 2001;
			_officialShould = 2020;
		}
	} else if (talkFlag && player_said("GATEKEEPER")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);

			if (_G(flags)[V069]) {
				kernel_trigger_dispatchx(kernel_trigger_create(1));
			} else {
				_G(flags)[V069] = 1;
				digi_play("203r17", 1, 255, 1);
			}
			break;
		case 1:
			_gkMayNotPass = series_load("gk may not pass");
			_gkMode = 3004;
			_digiName1 = "203g02";
			_digiTrigger1 = -1;
			_trigger1 = kernel_trigger_create(2);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(125));
			_G(kernel).trigger_mode = KT_PARSE;
			break;
		case 2:
			series_unload(_gkMayNotPass);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}

	} else if (takeFlag && player_said("SOLDIER'S HELMET ")) {
		digi_play("203R39", 1);
	} else if (takeFlag && HERE("SOLDIER'S HELMET")) {
		if (!_G(flags)[V061]) {
			player_set_commands_allowed(false);
			playSound("203r58", _G(kernel).trigger);
		}
	} else if (takeFlag && player_said("pigeons")) {
		digi_play("203R62", 1);
	} else if (player_said("SOLDIER'S HELMET", "tree") && inv_player_has("SOLDIER'S HELMET")) {
		digi_play("203r60", 1);
	} else if (player_said("BUCKET", "BASKET") && inv_player_has("BUCKET")) {
		digi_play("203r61", 1);
	} else if (takeFlag && player_said("WOODPILE")) {
		digi_play("207R40", 1);
	} else if (takeFlag && player_said("BASKET")) {
		lookAtHeads("203r38", _G(kernel).trigger);
	} else if (player_said("US DOLLARS", "PEASANT") && inv_player_has("US DOLLARS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_peskyAction = series_load("rip trek med reach hand pos1");
			setGlobals1(_peskyAction, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			if (_G(flags)[V075]) {
				digi_play("203r40b", 1, 255, 2);
			} else {
				digi_play("203r40a", 1, 255, 2);
				_G(flags)[V075] = 1;
			}
			break;
		case 2:
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(4);
			break;
		case 4:
			series_unload(_peskyAction);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("CHINESE YUAN", "PEASANT") && inv_player_has("CHINESE YUAN")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_peskyBegLoop = series_load("pesky beg loop");
			_peasantMode2 = _peasantMode;
			_peasantMode = 4055;
			_peskyAction = series_load("rip trek med reach hand pos1");
			setGlobals1(_peskyAction, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			_peasantMode = 4056;
			digi_play("203p01", 1, 255, 2);
			break;
		case 2:
			_peasantMode = 4058;
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(4);
			break;
		case 4:
			series_unload(_peskyAction);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("OFFICIAL") && (HAS("US DOLLARS") || HAS("CHINESE YUAN"))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripHandTalk = series_load("rip trek hand talk pos3");
			setGlobals1(_ripHandTalk, 1, 4, 4, 4, 0, 4, 1, 1, 1);
			sendWSMessage_110000(4);
			break;
		case 4:
			_officialMode = 2001;
			_officialShould = 2040;
			_digiName3 = "203O02";
			_digiTrigger3 = -1;
			_trigger4 = kernel_trigger_create(5);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(140));
			_G(kernel).trigger_mode = KT_PARSE;
			break;
		case 5:
			sendWSMessage_120000(6);
			break;
		case 6:
			sendWSMessage_150000(7);
			break;
		case 7:
			series_unload(_ripHandTalk);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("SEVEN SPOKES", "ALLEY WALL") && inv_player_has("SEVEN SPOKES")) {
		if (_G(flags)[V061]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker();
				_ripley = series_stream("climbs wall", 5, 3584, -1);
				series_stream_break_on_frame(_ripley, 21, 203);
				break;
			case 202:
				inv_move_object("SEVEN SPOKES", 999);
				_G(game).setRoom(204);
				break;
			case 203:
				digi_play("203_s06", 1, 100, -1);
				series_stream_break_on_frame(_ripley, 49, 204);
				break;
			case 204:
				series_stream_break_on_frame(_ripley, 63, 205);
				break;
			case 205:
				series_stream_break_on_frame(_ripley, 74, 206);
				digi_play("203_s06", 1, 100, -1);
				break;
			case 206:
				series_stream_break_on_frame(_ripley, 121, 207);
				digi_play("203_s06", 1, 100, -1);
				break;
			case 207:
				series_stream_break_on_frame(_ripley, 123, 8833);
				digi_play("203_s06", 1, 100, -1);
				break;
			case 208:
				series_stream_break_on_frame(_ripley, 202, 209);
				digi_play("203_s06", 1, 100, -1);
				break;
			case 209:
				digi_play("203_s06", 1, 100, -1);
				series_stream_break_on_frame(_ripley, 315, 3333);
				break;
			case 3333:
				disable_player_commands_and_fade_init(202);
				break;
			case 8833:
				digi_play("203_s07", 1, 100, -1);
				series_stream_break_on_frame(_ripley, 151, 208);
				break;
			default:
				break;
			}
		} else {
			player_set_commands_allowed(false);
			_G(player).disable_hyperwalk = true;
			_trigger2 = 90;
		}
	} else if (player_said("OLD LADY") && (HAS("US DOLLARS") || HAS("CHINESE YUAN"))) {
		if (_G(flags)[V051] != 0 && _G(flags)[V060] == 0) {
			_trigger3 = 40;
			player_set_commands_allowed(false);
		} else {
			playSound("203r19", _G(kernel).trigger);
		}
	} else if (player_said("RIPLEY PHOTO", "OLD LADY") && inv_player_has("RIPLEY PHOTO")) {
		player_set_commands_allowed(false);
		_flag2 = true;
		g_engine->camera_shift_xy(760, 0);
		_trigger3 = 60;
	} else if (player_said("SOLDIER'S HELMET", "AWNING") && inv_player_has("SOLDIER'S HELMET")) {
		switch (_G(kernel).trigger) {
		case -1:
			_G(flags)[V061] = 1;
			ws_hide_walker();
			player_set_commands_allowed(false);
			hotspot_set_active("SOLDIER'S HELMET ", true);
			series_load("one frame helmet");
			_ripTossesHelmet = series_load("rip tosses helmet");
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 960, -53, 100, 0x100, 0,
				triggerMachineByHashCallback, "rip throws helmet");
			sendWSMessage_10000(1, _ripley, _ripTossesHelmet, 1, 33, 2, _ripTossesHelmet, 33, 33, 0);
			break;
		case 2:
			digi_play("203_s03", 1);
			sendWSMessage_10000(1, _ripley, _ripTossesHelmet, 33, 42, 3, _ripTossesHelmet, 42, 42, 0);
			break;
		case 3:
			setupHelmetHotspot();
			terminateMachineAndNull(_ripley);
			series_unload(_ripTossesHelmet);
			series_place_sprite("one frame helmet", 0, 960, -53, 100, 0x700);
			ws_unhide_walker();
			kernel_timing_trigger(3, 4);
			break;
		case 4:
			inv_move_object("SOLDIER'S HELMET", 203);
			player_update_info();
			ws_walk(_G(player_info).x + 65, _G(player_info).y + 10, nullptr, -1, 10);
			_val1 = 1;

			if (_peasantMode == 4050) {
				_peasantMode = 4050;
				_peasantShould = 4094;
			} else {
				_val5 = 4146;
			}
			break;
		default:
			break;
		}
	} else if (player_said("BUCKET", "AWNING") && inv_player_has("BUCKET")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_hide_walker();
			player_set_commands_allowed(false);
			_ripTossesBucket = series_load("rip tosses bucket");
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 960, -53, 100, 0x100, false,
				triggerMachineByHashCallback, "rip throws bucket");
			sendWSMessage_10000(1, _ripley, _ripTossesBucket, 1, 47, 1,
				_ripTossesBucket, 47, 47, 0);
			break;
		case 1:
			terminateMachineAndNull(_ripley);
			ws_unhide_walker();
			series_unload(_ripTossesBucket);
			_oneFrameBucket = series_load("one frame bucket");
			series_place_sprite("one frame bucket", 0, 960, -53, 100, 0x700);

			if (_peasantShould == 4150 || _peasantShould == 4151)
				player_set_commands_allowed(true);
			else
				_peasantShould = 4146;
			break;
		default:
			break;
		}
	} else if (player_said("GATEKEEPER") && (HAS("US DOLLARS") || HAS("CHINESE YUAN"))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_gkMayNotPass = series_load("gk may not pass");
			_peskyAction = series_load("rip trek med reach hand pos1");
			setGlobals1(_peskyAction, 1, 10, 10, 10, 1, 10, 1, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			_gkMode = 3004;
			_digiName1 = nullptr;
			_digiTrigger1 = -1;
			_trigger1 = kernel_trigger_create(2);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(125));
			_G(kernel).trigger_mode = KT_PARSE;
			break;
		case 2:
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(4);
			break;
		case 4:
			series_unload(_peskyAction);
			series_unload(_gkMayNotPass);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("SOLDIER'S HELMET", "PEASANT")) {
		digi_play("203r64", 1);
	} else if (player_said("journal") && !takeFlag && !lookFlag && !inv_player_has(_G(player).noun)) {
		if (_G(flags)[kChinshiCartoon] == 1) {
			digi_play("203r54", 1);
		} else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[V089] = 1;
				_G(flags)[kChinshiCartoon] = 1;
			}

			sketchInJournal("203r53");
		}
	} else if (lookFlag && !inv_player_has(_G(player).noun)) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags)[V050]) {
				digi_play("203R36A", 1);
			} else {
				_G(flags)[V050] = 1;
				player_set_commands_allowed(false);
				_flag2 = true;
				digi_play("203R36", 1, 255, 1);
			}
			break;
		case 1:
			_flag2 = false;
			player_set_commands_allowed(true);
			digi_play("203R36A", 1);
			break;

		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room203::syncGame(Common::Serializer &s) {
	s.syncAsSint32LE(_ripleyMode);
	s.syncAsSint32LE(_ripleyShould);
	s.syncAsSint32LE(_oldLadyMode);
	s.syncAsSint32LE(_oldLadyShould);
	s.syncAsSint32LE(_peasantMode);
	s.syncAsSint32LE(_peasantMode2);
	s.syncAsSint32LE(_peasantShould);
	s.syncAsSint32LE(_officialMode);
	s.syncAsSint32LE(_officialShould);
	s.syncAsSint32LE(_gkMode);
	s.syncAsSint32LE(_gkShould);
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

void Room203::peasantAnim1() {
	sendWSMessage_10000(1, _peasant, _peasantSquatTo9, 1, 11, 121,
		_peasantSquatTo9, 11, 11, 0);
	sendWSMessage_10000(1, _peasantShadow, _peasantFromSquat3, 1, 11, -1,
		_peasantFromSquat3, 11, 11, 0);
}

void Room203::peasantAnim2() {
	sendWSMessage_10000(1, _peasant, _peasantSquat3, 6, 18, -1,
		_peasantRocks, 1, 1, 0);
	sendWSMessage_10000(1, _peasantShadow, _peasantSquat9, 6, 18, 121,
		_peasantRocksShadow, 1, 1, 0);
}

void Room203::conv203c() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		digi_play(sound, 1, 255, (node == 3 && entry == 0 && who == 0) ? -1 : 1);

		if (who <= 0) {
			_officialShould = (node == 3 && entry == 0) ? 2014 : 2013;
		} else if (who == 1) {
			_ripleyShould = (node == 1 && entry == 0) ? 1221 : 1220;
		}
	} else {
		conv_resume();
	}
}

void Room203::conv203d() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();

	if (_G(kernel).trigger == 1) {
		if (who <= 0)
			_peasantMode = 4057;
		else if (who == 1)
			_ripleyShould = 1030;
		conv_resume();

	} else {
		if (who <= 0)
			_peasantMode = 4056;
		else if (who == 1)
			_ripleyShould = 1020;

		if (sound)
			digi_play(sound, 1, 255, 1);
		else
			conv_resume();
	}
}

void Room203::conv203e() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 1) {
		if (who == 1)
			_ripleyShould = 1130;

		conv_resume();
	} else {
		if (who <= 0) {
			_oldLadyShould = 5302;
		} else if (who == 1) {
			if (node == 1 && entry == 3) {
				_digiName2 = sound;
				_ripleyShould = 1121;
			} else {
				_unkShould = 1120;
				_ripleyShould = 1120;
			}
		}

		if (!sound) {
			conv_resume();
		} else if (node != 1 || entry != 3) {
			digi_play(sound, 1, 255, 1);
		}
	}
}

void Room203::playSound(const char *digiName, int trigger) {
	switch (trigger) {
	case -1:
		_flag2 = true;
		player_set_commands_allowed(false);
		_ripLookDown = series_load("rip trek look down pos3");
		setGlobals1(_ripLookDown, 1, 3, 3, 3, 0, 3, 1, 1, 1);
		sendWSMessage_110000(210);
		break;

	case 210:
		digi_play(digiName, 1, 255, 211);
		break;

	case 211:
		sendWSMessage_120000(212);
		break;

	case 212:
		sendWSMessage_150000(213);
		break;

	case 213:
		series_unload(_ripLookDown);
		_flag2 = false;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room203::lookThroughHole(const char *digiName, int trigger) {
	switch (trigger) {
	case 230:
		player_set_commands_allowed(false);
		_ripLooksThroughHole = series_load("rip looks through hole pos1");
		setGlobals1(_ripLooksThroughHole, 1, 12, 12, 12, 1, 12, 1, 1, 1);
		sendWSMessage_110000(231);
		break;

	case 231:
		if (digiName)
			digi_play(digiName, 1, 255, 232);
		else
			kernel_timing_trigger(40, 232);
		break;

	case 232:
		sendWSMessage_120000(233);
		break;

	case 233:
		sendWSMessage_150000(234);
		break;

	case 234:
		series_unload(_ripLooksThroughHole);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room203::lookAtHeads(const char *digiName, int trigger) {
	switch (trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripLookAtHeadsTalkMei = series_load("rip look at heads talk mei");
		setGlobals1(_ripLookAtHeadsTalkMei, 11, 16, 16, 16, 1, 16, 11, 11, 11);
		sendWSMessage_110000(220);
		break;

	case 220:
		if (digiName)
			digi_play(digiName, 1, 255, 221);
		else
			kernel_timing_trigger(40, 221);
		break;

	case 221:
		sendWSMessage_120000(222);
		break;

	case 222:
		sendWSMessage_150000(223);
		break;

	case 223:
		series_unload(_ripLookAtHeadsTalkMei);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
