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

#include "m4/riddle/rooms/section5/room504.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng.h"
#include "m4/adv_r/other.h"
#include "m4/m4.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS[] = { 200, -1 };
static const char *NORMAL_NAMES[] = { "menendez walker 3" };
static const int16 SHADOW_DIRS[] = { 210, -1 };
static const char *SHADOW_NAMES[] = { "menendez walker shadow 3" };


void Room504::init() {
	_trigger1 = -1;

	_vines1 = nullptr;
	_vines2 = nullptr;
	_vines3 = nullptr;
	_vines4 = nullptr;
	_rope = nullptr;

	_ladder = nullptr;
	_toy = nullptr;
	_shovel = nullptr;
	_driftwood = nullptr;
	_pole = 0;

	_waterfall = series_plain_play("peruvian waterfall", -1, 0, 100, 0xf00, 9, -1, 0);
	digi_preload("504_S01");
	_volume = 1;
	kernel_timing_trigger(1, 501);
	digi_play("504_501", 3, _volume);
	kernel_timing_trigger(828, 754);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		break;

	case 506:
		player_set_commands_allowed(false);
		ws_demand_location(1384, 205, 9);
		ws_hide_walker();
		MoveScreenDelta(_G(game_buff_ptr), -1280, 0);
		_flag1 = true;

		_downSteps = series_load("504 down steps");
		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp");
		sendWSMessage_10000(1, _ripley, _downSteps, 1, 27, 647,
			_downSteps, 27, 27, 0);
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(50, 226, 3);
		midi_play("MOCAMO", 200, 0, -1, 949);

		if (player_been_here(504)) {
			inv_give_to_player("ROPE");
			_G(flags)[V154] = 3;
			ws_walk(183, 207, nullptr, 762, 3);
		} else {
			_G(flags)[V152] = 6;
			_G(flags)[V153] = 6;
			_G(flags)[V154] = 2;
			_G(flags)[V171] = 3;
			ws_walk(183, 207, nullptr, 655, 3);
		}

		_flag1 = false;
		break;
	}

	setVines();
	setVinesRope();
	setMiscItems();

	if (!_G(flags)[V141])
		kernel_timing_trigger(1, 502);
}

void Room504::daemon() {
	switch (_G(kernel).trigger) {
	case 501:
		player_update_info();

		if (_G(player_info).x <= 550) {
			_volume = 127;
		} else if (_G(player_info).x <= 978) {
			_volume = (float)_G(player_info).x * -0.119158878 + 193.0373832;
		} else if (_G(player_info).x > 978 && _G(player_info).x <= 1919) {
			_volume = (float)_G(player_info).x * -0.040648246 + 116.2539851;
		}

		digi_change_panning(3, _volume);
		kernel_timing_trigger(60, 501);
		break;

	case 502:
		_val2 = 1;
		_convState1 = 1;
		_trigger2 = -1;
		_trigger3 = -1;
		_mzDigs = series_load("504 MZ DIGS");
		_mzStandsTalks = series_load("504 MZ STANDS TALKS");
		_mzMachine = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xb00, 0,
			triggerMachineByHashCallbackNegative, "Emilio Menendez Machine");
		kernel_timing_trigger(1, 503);
		break;

	case 503:
		if (_trigger2 != -1 && _convState1 == 2 && _val2 == 2) {
			kernel_trigger_dispatchx(_trigger2);
			_trigger2 = -1;
		}

		if (_trigger3 != -1 && _convState1 == 1 && _val2 == 1) {
			kernel_trigger_dispatchx(_trigger3);
			_trigger3 = -1;
		}

		if (_trigger3 != -1 && _convState1 == 5 && _val2 == 5) {
			kernel_trigger_dispatchx(_trigger3);
			_trigger3 = -1;
		}

		if (_trigger3 != -1 && _convState1 == 9 && _val2 == 9) {
			kernel_trigger_dispatchx(_trigger3);
			_trigger3 = -1;
		}

		if (_trigger3 != -1 && _convState1 == 15 && _val2 == 15) {
			kernel_trigger_dispatchx(_trigger3);
			_trigger3 = -1;
		}

		kernel_timing_trigger(1, 504);
		break;

	case 504:
		// TODO
		switch (_val2) {
		case 1:
			break;
		}
		break;

	case 508:
		_G(kernel).trigger_mode = KT_PARSE;
		conv_load("con504a", 10, 10, 747);
		_val3 = inv_player_has("PERUVIAN INTI") ? 1 : 0;
		conv_export_pointer_curr(&_val3, 0);
		conv_play();
		break;

	case 509:
		_convState1 = 2;
		_trigger2 = kernel_trigger_create(510);
		break;

	case 510:
		_convState1 = 1;
		_convState2 = 9;

		if (!_G(flags)[V040]) {
			_G(flags)[V040] = 1;
			setMiscItems();
		}
		break;

	case 513:
		_convState2 = 1;
		_convState1 = 2;
		_trigger2 = kernel_trigger_create(514);
		break;

	case 514:
		_convState2 = 5;
		_trigger4 = kernel_trigger_create(515);
		break;

	case 516:
		_convState2 = 5;
		_convState1 = 6;
		break;

	case 520:
		_convState2 = 1;
		sendWSMessage_10000(1, _mzMachine, _mzSeries1, 21, 29, 503,
			_mzStandsTalks, 11, 11, 4);
		_convState1 = 2;
		_val2 = 2;
		_trigger2 = kernel_trigger_create(521);
		break;

	case 523:
		_convState2 = 1;
		_convState1 = 16;
		break;

	case 524:
		_trigger1 = kernel_trigger_create(525);
		break;

	case 529:
		_convState2 = 3;
		digi_play("504R16", 1, 255, 530);
		break;

	case 530:
		_convState2 = 1;
		sendWSMessage_10000(1, _mzMachine, _mzSeries2, 60, 61, -1,
			_mzSeries2, 60, 62, 4);
		digi_play("504Z17", 1, 255, 531);
		break;

	case 533:
		_shovel = series_place_sprite("504shov", 0, 0, 0, 100, 0xf00);
		_trigger4 = kernel_trigger_create(534);
		break;

	case 534:
		digi_unload("504_S07");
		ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
		terminateMachineAndNull(_mzMachine);
		series_unload(_mzSeries2);
		digi_unload("504_S02D");

		_menendez = triggerMachineByHash_3000(8, 16, NORMAL_DIRS, SHADOW_DIRS,
			1005, 155, 9, triggerMachineByHashCallback3000, "Emilio Menendez Walker");
		sendWSMessage_10000(_menendez, -_G(game_buff_ptr)->x1 - 30,
			155, 9, 536, 1);
		kernel_timing_trigger(150, 535);
		break;

	case 535:
	case 539:
		_convState2 = 13;
		break;

	case 536:
		_convState2 = 10;
		digi_play("504R17", 1, 255, 537);
		break;

	case 537:
		_convState2 = 13;
		digi_play("504Z18", 1, 255, 538);
		break;

	case 540:
		_convState2 = 10;
		digi_play("504R18A", 1, 255, 541);
		break;

	case 541:
		kernel_timing_trigger(5, 542);
		break;

	case 542:
		_convState2 = 11;
		_trigger4 = kernel_trigger_create(543);
		break;

	case 543:
		kernel_timing_trigger(5, 544);
		break;

	case 544:
		_convState2 = 7;
		_trigger4 = kernel_trigger_create(545);
		break;

	case 545:
		sendWSMessage_60000(_menendez);
		kernel_timing_trigger(5, 745);
		_G(flags)[V141] = 1;
		setMiscItems();
		_convState2 = 9;
		break;

	case 549:
		_convState2 = 3;
		digi_play("504R51", 1, 255, 550);
		break;

	case 550:
		_convState2 = 1;
		_convState1 = 2;
		_trigger2 = kernel_trigger_create(551);
		break;

	case 551:
		_convState2 = 5;
		_trigger4 = kernel_trigger_create(552);
		break;

	case 552:
		_convState2 = 6;
		digi_play("504R13", 1, 255, 553);
		break;

	case 554:
		_convState1 = 2;
		_trigger2 = kernel_trigger_create(555);
		break;

	case 555:
		_convState2 = 1;
		digi_play("504Z12", 1, 255, 556);
		break;

	case 556:
		_convState1 = 1;
		_trigger1 = kernel_trigger_create(557);
		break;

	case 557:
		_convState2 = 9;
		break;

	case 558:
		player_set_commands_allowed(false);
		digi_preload("504_S05");
		_flag1 = false;
		_ripStep = series_stream("RIP STEP UP TO R PLATFORM", 5, 0, 560);
		series_stream_break_on_frame(_ripStep, 1, 559);
		break;

	case 559:
	case 571:
	case 715:
	case 735:
		ws_hide_walker();
		break;

	case 560:
		series_stream("504 SHUFFLE TO ROPE", 5, 0, 561);
		break;

	case 561:
		terminateMachineAndNull(_rope);
		_ripStep = series_stream("504 RIP CROSS R TO L", 5, 0, 562);
		series_stream_break_on_frame(_ripStep, 103, 563);
		break;

	case 562:
		series_stream("504 RIP R TO L FINISH CROSS", 5, 0, 564);
		break;

	case 563:
		digi_play("504_S05", 1);
		break;

	case 564:
		_rope = series_place_sprite("ROPE SPRITE", 0, 0, 0, 100, 0xe00);
		series_stream("RIP STEP DOWN ON LEFT", 5, 0, 565);
		break;

	case 565:
		ws_unhide_walker();
		ws_demand_location(170, 145, 1);
		_flag1 = false;
		kernel_timing_trigger(5, 566);
		break;

	case 567:
		player_update_info();
		digi_preload("COM125");
		_ripWipe = series_load("504WIPE");
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
			_G(player_info).depth, 0, triggerMachineByHashCallbackNegative,
			"Rip Delta Machine State");
		sendWSMessage_10000(1, _ripley, _ripWipe, 1, 6, 568,
			_ripWipe, 6, 6, 0);
		break;

	case 570:
		player_set_commands_allowed(false);
		digi_preload("504_S05");
		_ripStep = series_stream("RIP STEP UP ON LEFT", 5, 0, 572);
		series_stream_break_on_frame(_ripStep, 1, 571);
		break;

	case 572:
		_ripStep = series_stream("504 RIP L TO R CROSS", 5, 0, 574);
		series_stream_break_on_frame(_ripStep, 45, 573);
		terminateMachineAndNull(_rope);
		break;

	case 573:
		digi_play("504_S05", 1);
		break;

	case 574:
		series_stream("504 RIP L TO R FINISH CROSS", 5, 0, 575);
		break;

	case 575:
		_rope = series_place_sprite("ROPE SPRITE", 0, 0, 0, 100, 0xe00);
		series_stream("504 SHUFFLE FROM ROPE", 5, 0, 576);
		break;

	case 576:
		_ripStep = series_stream("RIP STEP OFF R PLATFORM", 5, 0, 578);
		series_stream_break_on_frame(_ripStep, 27, 577);
		break;

	case 577:
		digi_play("COM125", 1);
		break;

	case 581:
		_vineMachine2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "Rope Falling to the Ground");
		sendWSMessage_10000(1, _vineMachine2, _vineTie, 1, 6, 582, _vineTie, 6, 6, 0);
		break;

	case 582:
		switch (_val1) {
		case 1:
			inv_move_object("ROPE", 504);
			break;
		case 2:
			inv_move_object("GREEN VINE", 504);
			break;
		case 3:
			inv_move_object("BROWN VINE", 504);
			break;
		case 4:
			inv_move_object("VINES", 4);
			break;
		default:
			break;
		}

		setVines();
		sendWSMessage_120000(583);
		break;

	case 583:
		sendWSMessage_150000(584);
		terminateMachineAndNull(_vineMachine2);
		break;

	case 588:
		sendWSMessage_120000(589);
		break;

	case 592:
		sendWSMessage_120000(593);
		break;

	case 606:
		terminateMachineAndNull(_vineMachine2);
		sendWSMessage_120000(607);
		break;

	case 607:
		switch (_val1) {
		case 1:
			inv_give_to_player("ROPE");
			kernel_examine_inventory_object("PING ROPE",
				5, 1, 449, 90, 608);
			break;

		case 2:
			inv_give_to_player("GREEN VINE");
			kernel_examine_inventory_object("PING GREEN VINE",
				5, 1, 449, 90, 608);
			break;

		case 3:
			inv_give_to_player("BROWN VINE");
			kernel_examine_inventory_object("PING BROWN VINE",
				5, 1, 449, 90, 608);
			break;

		case 4:
			inv_give_to_player("VINES");
			kernel_examine_inventory_object("PING VINES",
				5, 1, 449, 90, 608);
			break;

		default:
			break;
		}
		break;

	case 633:
		switch (_val1) {
		case 1:
			_G(flags)[V154] = 0;
			inv_move_object("ROPE", 504);
			break;
		case 2:
			_G(flags)[V152] = 0;
			inv_move_object("GREEN VINE", 504);
			break;
		case 3:
			_G(flags)[V153] = 0;
			inv_move_object("BROWN VINE", 504);
			break;
		case 4:
			_G(flags)[V171] = 0;
			inv_move_object("VINES", 504);
			break;
		default:
			break;
		}

		setVines();
		terminateMachineAndNull(_vineMachine2);
		sendWSMessage_120000(634);
		break;

	case 634:
		sendWSMessage_150000(635);
		terminateMachineAndNull(_vineMachine2);
		break;

	case 641:
		_vineTie = series_load("504 R VINE TIE BEFORE THROW");
		_vine = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xe00, 0,
			triggerMachineByHashCallbackNegative, "Vine Unrolling as Tied");
		sendWSMessage_10000(1, _vine, _vineTie, 1, 6, 642, _vineTie, 6, 6, 0);
		break;


	case 657:
		_convState2 = 5;
		_trigger4 = kernel_trigger_create(658);
		break;

	case 658:
		_convState2 = 6;
		digi_play(conv_sound_to_play(), 1, 255, 549);
		break;

	case 659:
		_convState2 = 5;
		_convState1 = 5;
		_trigger3 = kernel_trigger_create(659);
		break;

	case 660:
		_convState2 = 1;
		_convState1 = 2;
		_trigger2 = kernel_trigger_create(661);
		break;

	case 661:
		_convState1 = 9;
		_trigger3 = kernel_trigger_create(662);
		break;

	case 662:
		_trigger1 = kernel_trigger_create(663);
		break;

	case 663:
		inv_give_to_player("SHRUNKEN HEAD");
		_convState2 = 4;
		_trigger4 = kernel_trigger_create(664);
		break;

	case 664:
		kernel_examine_inventory_object("PING SHRUNKEN HEAD",
			5, 1, 136, 120, 665);
		break;

	case 665:
		_convState1 = 2;
		_convState2 = 1;
		_trigger2 = kernel_trigger_create(666);
		break;

	case 666:
		_trigger1 = kernel_trigger_create(667);
		break;

	case 667:
	case 674:
		conv_resume();
		break;

	case 669:
		_convState1 = 15;
		_trigger3 = kernel_trigger_create(670);
		break;

	case 670:
		_convState2 = 8;
		_trigger4 = kernel_trigger_create(671);
		break;

	case 671:
		digi_play(conv_sound_to_play(), 1, 255, 672);
		break;

	case 672:
		_convState1 = 2;
		_convState2 = 1;
		_trigger1 = kernel_trigger_create(673);
		break;

	case 673:
		_trigger2 = kernel_trigger_create(674);
		break;

	case 696:
		switch (_val1) {
		case 1:
			inv_give_to_player("ROPE");
			kernel_examine_inventory_object("PING ROPE",
				5, 1, 105, 60, 697);
			break;

		case 2:
			inv_give_to_player("GREEN VINE");
			kernel_examine_inventory_object("PING GREEN VINE",
				5, 1, 105, 60, 697);
			break;

		case 3:
			inv_give_to_player("BROWN VINE");
			kernel_examine_inventory_object("PING BROWN VINE",
				5, 1, 105, 60, 697);
			break;

		case 4:
			inv_give_to_player("VINES");
			kernel_examine_inventory_object("PING VINES",
				5, 1, 105, 60, 697);
			break;

		default:
			break;
		}
		break;

	case 703:
		switch (_val1) {
		case 1:
			inv_give_to_player("ROPE");
			kernel_examine_inventory_object("PING ROPE",
				5, 1, 105, 60, 704);
			break;

		case 2:
			inv_give_to_player("GREEN VINE");
			kernel_examine_inventory_object("PING GREEN VINE",
				5, 1, 105, 60, 704);
			break;

		case 3:
			inv_give_to_player("BROWN VINE");
			kernel_examine_inventory_object("PING BROWN VINE",
				5, 1, 105, 60, 704);
			break;

		case 4:
			inv_give_to_player("VINES");
			kernel_examine_inventory_object("PING VINES",
				5, 1, 105, 60, 704);
			break;

		default:
			break;
		}
		break;

	case 707:
		player_set_commands_allowed(false);
		digi_play("504R45", 1, 255, 708);
		break;

	case 714:
		player_set_commands_allowed(false);
		digi_preload("504_S09");
		digi_preload("504_S03");
		digi_preload("504_S05");
		_ripStep = series_stream("RIP STEP UP ON LEFT", 5, 0, 716);
		series_stream_break_on_frame(_ripStep, 1, 715);
		ws_hide_walker();
		break;

	case 716:
		_ripStep = series_stream("504 RIP L TO R CROSS", 5, 0, 718);
		series_stream_break_on_frame(_ripStep, 45, 717);
		terminateMachineAndNull(_rope);
		break;

	case 717:
		digi_play("504_S05", 1);
		break;

	case 718:
		digi_play("504_S09", 1, 255, 719);
		_ripStep = series_stream("504 RIP L TO R FALLS", 5, 0, -1);
		series_stream_break_on_frame(_ripStep, 27, 721);
		break;

	case 720:
		digi_play("504_S03", 1);
		break;

	case 722:
		pal_fade_init(21, 255, 0, 30, 723);
		interface_hide();
		break;

	case 723:
	case 744:
		other_save_game_for_resurrection();
		_G(game).setRoom(413);
		break;

	case 725:
		player_set_commands_allowed(false);
		digi_play("504R45", 1, 255, 726);
		break;

	case 734:
		player_set_commands_allowed(false);
		digi_preload("504_s09");
		digi_preload("504_s03");
		digi_preload("504_s05");
		_flag1 = false;
		_ripStep = series_stream("RIP STEP UP TO R PLATFORM", 5, 0, 736);
		series_stream_break_on_frame(_ripStep, 1, 735);
		break;

	case 736:
		series_stream("504 SHUFFLE TO ROPE", 5, 0, 737);
		break;

	case 737:
		terminateMachineAndNull(_rope);
		_ripStep = series_stream("504 RIP CROSS R TO L", 5, 0, 739);
		series_stream_break_on_frame(_ripStep, 103, 738);
		break;

	case 738:
		digi_play("504_S05", 1);
		break;

	case 739:
		digi_play("504_S09", 1, 255, 740);
		_ripStep = series_stream("504 RIP R TO L FALL", 5, 0, -1);
		series_stream_break_on_frame(_ripStep, 25, 742);
		break;

	case 741:
		digi_play("504_S03", 1);
		break;

	case 743:
		pal_fade_init(21, 255, 0, 30, 744);
		interface_hide();
		break;

	case 748:
		_G(game).setRoom(501);
		inv_move_object("ROPE", 504);
		break;

	case 751:
		digi_play("504R44", 1, 255, 752);
		break;

	case 755:
		player_set_commands_allowed(false);
		digi_play("504R45", 1, 255, 756);
		break;

	case 756:
		player_set_commands_allowed(false);
		ws_walk(546, 143, nullptr, 757, 11);
		break;

	// TODO: Various switch cases

	default:
		break;
	}
}

void Room504::pre_parser() {
	bool useFlag = player_said("gear");
	bool useFlag1 = useFlag && _flag1;
	bool useFlag0 = useFlag && !_flag1;

	if (useFlag1 && player_said("ROPE COIL "))
		intr_freshen_sentence(62);
	else if (useFlag1 && player_said("GREEN VINE COIL "))
		intr_freshen_sentence(63);
	else if (useFlag1 && player_said("BROWN VINE COIL "))
		intr_freshen_sentence(64);
	else if (useFlag1 && player_said("COIL OF VINES "))
		intr_freshen_sentence(107);

	else if (useFlag1 && player_said("ROPE COIL    "))
		intr_freshen_sentence(62);
	else if (useFlag1 && player_said("GREEN VINE COIL    "))
		intr_freshen_sentence(63);
	else if (useFlag1 && player_said("BROWN VINE COIL    "))
		intr_freshen_sentence(64);
	else if (useFlag1 && player_said("COIL OF VINES    "))
		intr_freshen_sentence(107);

	else if (useFlag0 && player_said("ROPE COIL  "))
		intr_freshen_sentence(62);
	else if (useFlag0 && player_said("GREEN VINE COIL  "))
		intr_freshen_sentence(63);
	else if (useFlag0 && player_said("BROWN VINE COIL  "))
		intr_freshen_sentence(64);
	else if (useFlag0 && player_said("COIL OF VINES  "))
		intr_freshen_sentence(107);

	else if (useFlag0 && player_said("ROPE COIL   "))
		intr_freshen_sentence(62);
	else if (useFlag0 && player_said("GREEN VINE COIL   "))
		intr_freshen_sentence(63);
	else if (useFlag0 && player_said("BROWN VINE COIL   "))
		intr_freshen_sentence(64);
	else if (useFlag0 && player_said("COIL OF VINES   "))
		intr_freshen_sentence(107);
}

#define ITEM(NAME) player_said(NAME) && inv_object_is_here(NAME)

void Room504::parser() {
	bool ropeCoilFlag = player_said_any("ROPE COIL ", "ROPE COIL  ",
		"ROPE COIL   ", "ROPE COIL    ");
	bool greenVineFlag = player_said_any("GREEN VINE COIL ",
		"GREEN VINE COIL  ", "GREEN VINE COIL   ", "GREEN VINE COIL    ");
	bool brownVineFlag = player_said_any("BROWN VINE COIL ",
		"BROWN VINE COIL  ", "BROWN VINE COIL   ", "BROWN VINE COIL    ");
	bool vineCoilFlag = player_said_any("COIL OF VINES ", "COIL OF VINES  ",
		"COIL OF VINES   ", "COIL OF VINES    ");
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool talkFlag = player_said_any("talk", "talk to");
	bool useFlag = player_said("gear");
	bool vineStatueFlag = player_said_any("rope ",
		"green vine ", "brown vine ", "vines ", "spider statue");
	bool menendezFlag = player_said_any("PERSON IN HOLE", "MENENDEZ");

	player_update_info();
	_flag1 = _G(player_info).x > 300;

	if (player_said("GREEN VINE", "BROWN VINE") && inv_player_has("GREEN VINE") &&
			inv_player_has("BROWN VINE")) {
		inv_move_object("GREEN VINE", NOWHERE);
		inv_move_object("BROWN VINE", NOWHERE);
		inv_give_to_player("VINES");
		_G(player).command_ready = false;
		return;
	} else if (((!_flag1 && _G(player).click_x > 300) ||
			(_flag1 && _G(player).click_x <= 300)) &&
			!lookFlag && !takeFlag && !useFlag &&
			checkVinesDistance()) {
		_G(player).command_ready = false;
		return;
	}

	_flag2 = _G(flags)[V154] == 2 ||
		(_G(flags)[V152] == 2 && _G(flags)[V153] == 2);

	if (!lookFlag && !takeFlag && !useFlag && !ropeCoilFlag &&
			!greenVineFlag && !brownVineFlag && !vineCoilFlag &&
			(_G(flags)[V154] == 2 || _G(flags)[V152] == 2 || _G(flags)[V153] == 2) &&
			parser1()) {
		// No implementation
	} else if (player_said("conv504a")) {
		conv504a();
	} else if (_G(kernel).trigger == 747) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 509);
	} else if (talkFlag && menendezFlag) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			g_engine->camera_shift_xy(862, 0);
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			if (g_engine->game_camera_panning()) {
				kernel_timing_trigger(5, 1);
			} else {
				_trigger1 = kernel_trigger_create(2);
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(5, 505);
			}
			break;
		case 2:
			_convState2 = 3;
			digi_play("504R51", 1, 255, 3);
			break;
		case 3:
			_convState2 = 1;
			_convState1 = 2;
			_trigger2 = kernel_trigger_create(4);
			break;
		case 4:
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 508);
			break;
		default:
			break;
		}
	} else if (menendezFlag && player_said("ROMANOV EMERALD")) {
		player_set_commands_allowed(false);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 511);
	} else if ((menendezFlag && player_said("PERUVIAN INTI")) ||
			(menendezFlag && !lookFlag && !takeFlag && !useFlag)) {
		player_set_commands_allowed(false);
		_flag4 = true;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 548);
	} else if (player_said("WALK UP")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			_upSteps = series_load("504 UP STEPS");
			player_update_info();
			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100,
				_G(player_info).depth, 0, triggerMachineByHashCallbackNegative, "Rp");
			sendWSMessage_10000(1, _ripley, _upSteps, 1, 25, -1, _upSteps, 25, 25, 0);
			kernel_timing_trigger(60, 2);
			break;
		case 2:
			disable_player_commands_and_fade_init(2);
			break;
		case 3:
			_G(game).setRoom(506);
			break;	
		default:
			break;
		}
	} else if (player_said("EXIT") && !_flag1) {
		if (inv_player_has("ROPE")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info();
				ws_walk(_G(player_info).x, _G(player_info).y, nullptr, -1, 9);
				disable_player_commands_and_fade_init(2);
				break;
			case 2:
				_G(game).setRoom(501);
				inv_move_object("ROPE", 504);
				break;
			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_walk(183, 207, nullptr, 2, 3);
				break;
			case 2:
				digi_play("504R44", 1, 255, 3);
				break;
			case 3:
				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("ROPE ") &&
			(_G(flags)[V154] == 1 || _G(flags)[V154] == 4)) {
		digi_play("504R39", 1);
	} else if (lookFlag && player_said("GREEN VINE ") &&
			(_G(flags)[V152] == 1 || _G(flags)[V152] == 4)) {
		digi_play("504R39", 1);
	} else if (lookFlag && player_said("BROWN VINE ") &&
			(_G(flags)[V153] == 1 || _G(flags)[V153] == 4)) {
		digi_play("504R39", 1);
	} else if (lookFlag && player_said("ROPE  ") &&
			(_G(flags)[V154] == 0 || _G(flags)[V154] == 5)) {
		digi_play("504R40", 1);
	} else if (lookFlag && player_said("GREEN VINE  ") &&
			(_G(flags)[V152] == 0 || _G(flags)[V152] == 5)) {
		digi_play("504R40", 1);
	} else if (lookFlag && player_said("BROWN VINE  ") &&
			(_G(flags)[V153] == 0 || _G(flags)[V153] == 5)) {
		digi_play("504R40", 1);
	} else if (lookFlag && ropeCoilFlag) {
		digi_play("504R36", 1);
	} else if (lookFlag && greenVineFlag) {
		digi_play("504R34", 1);
	} else if (lookFlag && brownVineFlag) {
		digi_play("504R34", 1);
	} else if (lookFlag && vineCoilFlag) {
		digi_play("504R35", 1);
	} else if (lookFlag && (
			player_said_any("GREEN VINE ", "GREEN VINE  ") ||
			player_said_any("BROWN VINE ", "BROWN VINE  ") ||
			player_said_any("ROPE ", "ROPE  ", "ROPE   ") ||
			player_said_any("GREEN VINE   ", "BROWN VINE   ")) &&
			lookVines()) {
		// No implementation
	} else if (lookFlag && player_said(" ")) {
		if (_G(flags)[V150]) {
			digi_play("504R02", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				digi_play("504R02", 1, 255, 2);
				break;
			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				digi_play("504R02A", 1, 255, 762);
				_G(flags)[V150] = 1;
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("WATERFALL")) {
		digi_play("504R03", 1);
	} else if (lookFlag && player_said("TREE")) {
		digi_play("504R04", 1);
	} else if (lookFlag && player_said("SPIDER STATUE")) {
		if (_G(flags)[V151] == 0 && _G(flags)[V154] == 2) {
			player_set_commands_allowed(false);
			_G(kernel).trigger_mode = KT_DAEMON;
			digi_play("504R06", 1, 255, 762);
			_G(flags)[V151] = 1;
		} else if (_G(flags)[V154] == 2 || _G(flags)[V154] == 1 ||
				_G(flags)[V154] == 4) {
			digi_play("504R06A", 1);
		} else {
			digi_play("504R06B", 1);
		}
	} else if (lookFlag && player_said("BROWN VINES")) {
		digi_play("504R07", 1);
	} else if (lookFlag && player_said("CHASM")) {
		digi_play("504R08", 1);
	} else if (lookFlag && player_said("STONE WALL")) {
		digi_play("504R09", 1);
	} else if (lookFlag && menendezFlag) {
		digi_play("504R12", 1);
	} else if (lookFlag && player_said("HOLE ")) {
		digi_play("504R33", 1);
	} else if (lookFlag && player_said("WHEELED TOY") &&
			_G(flags)[V141] == 1 && inv_player_has("WHEELED TOY")) {
		digi_play("504R20", 1);
	} else if (lookFlag && ITEM("WHEELED TOY")) {
		digi_play("504R50", 1);
	} else if (lookFlag && player_said("STAIRS")) {
		digi_play("504R25", 1);
	} else if (lookFlag && ITEM("DRIFTWOOD STUMP")) {
		digi_play("504R43", 1);
	} else if (lookFlag && ITEM("POLE")) {
		digi_play("504R43", 1);
	} else if (lookFlag && player_said("WOODEN LADDER ")) {
		digi_play("504R43", 1);
	} else if (lookFlag && ITEM("WOODEN LADDER")) {
		digi_play("COM107", 1, 255, -1, 504);
	} else if (lookFlag && ITEM("SHOVEL")) {
		digi_play("COM106", 1);
	} else if (lookFlag && player_said("SHOVEL ")) {
		digi_play("504R43", 1);
	}

	else if (useFlag && ITEM("SHOVEL")) {
		digi_play("504R19", 1);
	} else if (useFlag && player_said("HOLE ")) {
		digi_play("504R21", 1);
	} else if (vineStatueFlag && player_said("ROPE") &&
			_flag1 && inv_player_has("ROPE")) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 1;
		hotspot_set_active("ROPE", true);
		hotspot_set_active("ROPE COIL", true);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	} else if (vineStatueFlag && player_said("GREEN VINE") &&
			_flag1 && inv_player_has("GREEN VINE")) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	} else if (vineStatueFlag && player_said("BROWN VINE") &&
		_flag1 && inv_player_has("BROWN VINE")) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	} else if (vineStatueFlag && player_said("VINES") &&
		_flag1 && inv_player_has("VINES")) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 580);
	}

	else if (takeFlag && player_said_any("ROPE ", "ROPE COIL ") &&
			_G(flags)[V154] == 1 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said_any("ROPE ") &&
		_G(flags)[V154] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 3;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && player_said_any("GREEN VINE ", "GREEN VINE COIL ") &&
			_G(flags)[V152] == 1 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said("GREEN VINE ") &&
			_G(flags)[V152] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && player_said_any(
			"BROWN VINE ", "BROWN VINE COIL ") && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V153] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said("BROWN VINE ") &&
			_G(flags)[V153] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && player_said("VINES ", "COIL OF VINES ") &&
			_G(flags)[V171] == 1 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(491, 166, nullptr, 586, 11);
	} else if (takeFlag && player_said("VINES ") &&
			_G(flags)[V171] == 4 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 603);
	} else if (takeFlag && !_flag1 && player_said_any("ROPE ", "ROPE COIL ") &&
			_G(flags)[V154] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && !_flag1 && player_said_any("GREEN VINE ", "GREEN VINE COIL ") &&
			_G(flags)[V154] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && !_flag1 && player_said_any("BROWN VINE ", "BROWN VINE COIL ") &&
			_G(flags)[V153] == 4) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && !_flag1 && player_said_any("VINES ", "COIL OF VINES  ") &&
			_G(flags)[V171] == 4) {
		digi_play("COM126", 1, 255, -1, 997);

	} else if (_flag1 && _G(player).click_x <= 300 &&
			player_said("ROPE COIL ") && _G(flags)[V154] == 1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(528, 168, nullptr, 595, 8);
	} else if (_flag1 && _G(player).click_x <= 300 &&
			player_said("GREEN VINE COIL ") && _G(flags)[V152] == 1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(528, 168, nullptr, 595, 8);
	} else if (_flag1 && _G(player).click_x <= 300 &&
			player_said("BROWN VINE COIL ") && _G(flags)[V153] == 1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(528, 168, nullptr, 595, 8);
	} else if (_flag1 && _G(player).click_x <= 300 &&
			player_said("COIL OF VINES ") && _G(flags)[V171] == 1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(528, 168, nullptr, 595, 8);

	} else if (!_flag1 && player_said("TREE", "ROPE COIL  ")) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 611, 11);
	} else if (!_flag1 && player_said("TREE", "GREEN VINE COIL  ")) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 611, 11);
	} else if (!_flag1 && player_said("TREE", "COIL OF VINES  ")) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 611, 11);
	} else if (!_flag1 && player_said("TREE", "BROWN VINE COIL  ")) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 611, 11);

	} else if (!_flag1 && _G(player).click_x > 300 && player_said("ROPE COIL   ")) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(200, 153, nullptr, 620, 3);
	} else if (!_flag1 && _G(player).click_x > 300 && player_said("GREEN VINE COIL   ")) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(200, 153, nullptr, 620, 3);
	} else if (!_flag1 && _G(player).click_x > 300 && player_said("COIL OF VINES   ")) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(200, 153, nullptr, 620, 3);
	} else if (!_flag1 && _G(player).click_x > 300 && player_said("BROWN VINE COIL   ")) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(200, 153, nullptr, 620, 3);

	} else if (player_said("TREE") && player_said("ROPE") &&
			!_flag1 && inv_player_has("ROPE")) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 0;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 630, 11);
	} else if (player_said("TREE") && player_said("GREEN VINE") &&
			!_flag1 && inv_player_has("GREEN VINE")) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 0;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 630, 11);
	} else if (player_said("TREE") && player_said("VINES") &&
			!_flag1 && inv_player_has("VINES")) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 0;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 630, 11);
	} else if (player_said("TREE") && player_said("BROWN VINE") &&
			!_flag1 && inv_player_has("BROWN VINE")) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 0;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 630, 11);

	} else if (_flag1 && vineStatueFlag && player_said("ROPE COIL    ")) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 638);
	} else if (_flag1 && vineStatueFlag && player_said("GREEN VINE COIL    ")) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 638);
	} else if (_flag1 && vineStatueFlag && player_said("BROWN VINE COIL    ")) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 638);
	} else if (_flag1 && vineStatueFlag && player_said("COIL OF VINES    ")) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 2;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 638);

	} else if (useFlag && player_said_any("ROPE ", "ROPE   ") &&
			_G(flags)[V154] == 2 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 5;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(485, 166, nullptr, 675, 11);
	} else if (useFlag && player_said_any("GREEN VINE ", "GREEN VINE   ") &&
			_G(flags)[V152] == 2 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 5;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(485, 166, nullptr, 675, 11);
	} else if (useFlag && player_said_any("BROWN VINE ", "BROWN VINE   ") &&
			_G(flags)[V153] == 2 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 5;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(485, 166, nullptr, 675, 11);
	} else if (useFlag && player_said("VINES ") &&
			_G(flags)[V171] == 2 && _flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 5;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(485, 166, nullptr, 675, 11);

	} else if (useFlag && player_said_any("ROPE  ", "ROPE   ") &&
			_G(flags)[V154] == 2 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 684, 11);
	} else if (useFlag && player_said_any("GREEN VINE  ", "GREEN VINE   ") &&
			_G(flags)[V152] == 2 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 684, 11);
	} else if (useFlag && player_said_any("BROWN VINE  ", "BROWN VINE   ") &&
			_G(flags)[V154] == 2 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 684, 11);
	} else if (useFlag && player_said("VINES  ") &&
			_G(flags)[V171] == 2 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V171] = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 684, 11);

	} else if (takeFlag && player_said("ROPE  ") &&
			_G(flags)[V154] == 0 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 693, 11);
	} else if (takeFlag && player_said_any("GREEN VINE  ", "GREEN VINE COIL   ") &&
			_G(flags)[V152] == 0 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 693, 11);
	} else if (takeFlag && player_said_any("BROWN VINE  ", "BROWN VINE COIL   ") &&
			_G(flags)[V153] == 0 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 693, 11);
	} else if (takeFlag && player_said_any("VINES  ", "COIL OF VINES   ") &&
			_G(flags)[V171] == 0 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 693, 11);

	} else if (takeFlag && player_said("ROPE  ") &&
			_G(flags)[V154] == 5 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 1;
		_G(flags)[V154] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 700, 11);
	} else if (takeFlag && player_said_any("GREEN VINE  ", "GREEN VINE COIL   ") &&
			_G(flags)[V153] == 5 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 3;
		_G(flags)[V153] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 700, 11);
	} else if (takeFlag && player_said_any("BROWN VINE  ", "BROWN VINE COIL   ") &&
			_G(flags)[V152] == 5 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 2;
		_G(flags)[V152] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 700, 11);
	} else if (takeFlag && player_said_any("VINES  ", "COIL OF VINES   ") &&
			_G(flags)[V171] == 5 && !_flag1) {
		player_set_commands_allowed(false);
		_val1 = 4;
		_G(flags)[V171] = 3;
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(135, 146, nullptr, 700, 11);

	} else if (takeFlag && _flag1 && player_said_any("ROPE  ", "ROPE COIL    ") &&
			_G(flags)[V154] == 5) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && _flag1 && player_said_any("GREEN VINE  ", "GREEN VINE COIL    ") &&
			_G(flags)[V152] == 5) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && _flag1 && player_said_any("BROWN VINE  ", "BROWN VINE COIL    ") &&
			_G(flags)[V153] == 5) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && _flag1 && player_said_any("VINES  ", "COIL OF VINES    ") &&
			_G(flags)[V171] == 5) {
		digi_play("COM126", 1, 255, -1, 997);

	} else if (lookFlag && player_said("STELE") && _G(flags)[V149] == 1) {
		digi_play("504R23", 1);
	} else if (_G(flags)[V149] == 0 && lookFlag && (
			player_said("STELE") || player_said_any("JOURNAL", "STELE"))) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			digi_play("504R22", 1, 255, 2);
			break;
		case 2:
			digi_preload("950_s34");
			_ripSketching = series_load("RIP SKETCHING IN NOTEBOOK POS 3");
			setGlobals1(_ripSketching, 1, 22, 22, 22, 0, 23, 36, 36, 36);
			sendWSMessage_110000(3);
			break;
		case 3:
			sendWSMessage_190000(9);
			sendWSMessage_120000(4);
			digi_play("950_s34", 1, 255, 7);
			break;
		case 4:
			sendWSMessage_110000(5);
			break;
		case 5:
			sendWSMessage_140000(6);
			break;
		case 6:
			digi_unload("950_s34");
			series_unload(_ripSketching);
			_G(flags)[V149] = 1;
			_G(flags)[V283] = 1;
			player_set_commands_allowed(true);
			break;
		case 7:
			switch (_G(player_info).facing) {
			case 3:
			case 9:
				sendWSMessage_190000(5);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	} else if (_G(flags)[V149] == 1 && player_said("JOURNAL", "STELE")) {
		Common::strcpy_s(_G(player).noun, " ");
		return;
	} else if (takeFlag && ITEM("WHEELED TOY") && _flag1) {
		if (_G(flags)[V141] == 1) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				g_engine->camera_shift_xy(862, 0);
				kernel_timing_trigger(1, 1);
				break;
			case 1:
				if (g_engine->game_camera_panning()) {
					kernel_timing_trigger(5, 1);
				} else {
					_ripLowReach = series_load("RIP LOW REACH POS1");
					setGlobals1(_ripLowReach, 1, 10, 10, 10);
					sendWSMessage_110000(2);
				}
				break;
			case 2:
				terminateMachineAndNull(_toy);
				kernel_examine_inventory_object("PING WHEELED TOY",
					_G(master_palette), 5, 1, 113, 107, 3, nullptr, -1);
				break;
			case 3:
				sendWSMessage_140000(4);
				break;
			case 4:
				series_unload(_ripLowReach);
				inv_give_to_player("WHELED TOY");
				setMiscItems();
				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		} else {
			digi_play("504R48", 1);
		}
	} else if (takeFlag && player_said("WOODEN LADDER ") &&
			inv_object_is_here("WOODEN LADDER") &&
			_G(flags)[V155] == 1 && _flag1) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			g_engine->camera_shift_xy(0, 0);
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			if (g_engine->game_camera_panning()) {
				kernel_timing_trigger(5, 1);
			} else {
				_ripMedReach = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(_ripMedReach, 1, 10, 10, 10);
				sendWSMessage_110000(2);
			}
			break;
		case 2:
			terminateMachineAndNull(_ladder);
			kernel_examine_inventory_object("PING WOODEN LADDER",
				_G(master_palette), 5, 1, 480, 75, 3, nullptr, -1);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			inv_give_to_player("WOODEN LADDER");
			setMiscItems();
			series_unload(_ripMedReach);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("WOODEN LADDER") &&
			_G(flags)[V141] == 0) {
		digi_play("504R48", 1);
	} else if (takeFlag && ITEM("WOODEN LADDER") &&
			_G(flags)[V155] == 0 && _flag1 && _G(flags)[V141] == 1) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			g_engine->camera_shift_xy(862, 0);
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			if (g_engine->game_camera_panning()) {
				kernel_timing_trigger(5, 1);
			} else {
				_ripLowReach = series_load("RIP LOW REACH POS1");
				setGlobals1(_ripLowReach, 1, 10, 10, 10);
				sendWSMessage_110000(2);
			}
			break;
		case 2:
			terminateMachineAndNull(_ladder);
			kernel_examine_inventory_object("PING WOODEN LADDER",
				_G(master_palette), 5, 1, 136, 120, 3, nullptr, -1);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			inv_give_to_player("WOODEN LADDER");
			setMiscItems();
			series_unload(_ripLowReach);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && ITEM("DRIFTWOOD STUMP") && !_flag1) {
		switch(_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripLowReach = series_load("RIP LOW REACH POS1");
			setGlobals1(_ripLowReach, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			break;
		case 2:
			terminateMachineAndNull(_driftwood);
			kernel_examine_inventory_object("PING DRIFTWOOD STUMP",
				_G(master_palette), 5, 1, 139, 102, 3, nullptr, -1);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			inv_give_to_player("DRIFTWOOD STUMP");
			setMiscItems();
			series_unload(_ripLowReach);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said_any("GREEN VINE ",
			"GREEN VINE  ", "GREEN VINE   ") && _G(flags)[V152] == 2) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && player_said_any("BROWN VINE ",
			"BROWN VINE  ", "BROWN VINE   ") && _G(flags)[V153] == 2) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && player_said_any("ROPE ", "ROPE  ", "ROPE   ") &&
			_G(flags)[V154] == 2) {
		digi_play("COM126", 1, 255, -1, 997);
	} else if (takeFlag && ITEM("POLE") && !_flag1) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripLowReach = series_load("RIP LOW REACH POS1");
			setGlobals1(_ripLowReach, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			break;
		case 2:
			terminateMachineAndNull(_pole);
			kernel_examine_inventory_object("PING POLE",
				_G(master_palette), 5, 1, 139, 102, 3, nullptr, -1);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			inv_give_to_player("POLE");
			setMiscItems();
			series_unload(_ripLowReach);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && ITEM("SHOVEL") && _flag1) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			g_engine->camera_shift_xy(862, 0);
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			if (g_engine->game_camera_panning()) {
				kernel_timing_trigger(5, 1);
			} else {
				_ripLowReach = series_load("RIP LOW REACH POS1");
				setGlobals1(_ripLowReach, 1, 10, 10, 10);
				sendWSMessage_110000(2);
			}
			break;
		case 2:
			terminateMachineAndNull(_shovel);
			kernel_examine_inventory_object("PING SHOVEL",
				_G(master_palette), 5, 1, 154, 114, 3, nullptr, -1);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			inv_give_to_player("SHOVEL");
			setMiscItems();
			series_unload(_ripLowReach);
			_G(flags)[V044] = 1;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("SHOVEL ") &&
			inv_object_is_here("SHOVEL") && _flag1) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			g_engine->camera_shift_xy(0, 0);
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			if (g_engine->game_camera_panning()) {
				kernel_timing_trigger(5, 1);
			} else {
				_ripLowReach = series_load("RIP LOW REACH POS1");
				setGlobals1(_ripLowReach, 1, 10, 10, 10);
				sendWSMessage_110000(2);
			}
			break;
		case 2:
			terminateMachineAndNull(_shovel);
			kernel_examine_inventory_object("PING SHOVEL",
				_G(master_palette), 5, 1, 440, 109, 3, nullptr, -1);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			inv_give_to_player("SHOVEL");
			setMiscItems();
			series_unload(_ripLowReach);
			_G(flags)[V044] = 1;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("STELE")) {
		digi_play("504R24", 1);
	} else if (player_said("GREEN VINE", "BROWN VINE")) {
		inv_move_object("GREEN VINE", NOWHERE);
		inv_move_object("BROWN VINE", NOWHERE);
		inv_give_to_player("VINES");
	} else if (useFlag && player_said("VINES")) {
		inv_move_object("VINES", NOWHERE);
		inv_give_to_player("GREEN VINE");
		inv_give_to_player("BROWN VINE");
	} else if (!lookFlag && !takeFlag && !useFlag &&
			!ropeCoilFlag && !greenVineFlag && !brownVineFlag &&
			!vineCoilFlag && _G(flags)[V154] != 2 &&
			_G(flags)[V152] != 2 && _G(flags)[V153] != 2) {
		if (_flag1) {
			if (_G(player).click_x <= 300)
				digi_play("504R05A", 1);
		} else {
			if (_G(player).click_x > 300)
				digi_play("504R05A", 1);
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room504::setVines() {
	freeVines();

	bool ropeFlag = _G(flags)[V152] == 2 || _G(flags)[V153] == 2 ||
		_G(flags)[V154] == 2;
	bool tiedFlag = _G(flags)[V152] == 0 || _G(flags)[V153] == 0 ||
		_G(flags)[V154] == 0 || _G(flags)[V171] == 0;
	bool rightVine = _G(flags)[V152] == 1 || _G(flags)[V153] == 1 ||
		_G(flags)[V154] == 1 || _G(flags)[V171] == 1;
	bool acrossFlag = _G(flags)[V152] == 5 || _G(flags)[V153] == 5 ||
		_G(flags)[V154] == 5 || _G(flags)[V171] == 5;
	bool hangingFlag = _G(flags)[V152] == 4 || _G(flags)[V153] == 4 ||
		_G(flags)[V154] == 4 || _G(flags)[V171] == 4;

	if (hangingFlag)
		_vines1 = series_place_sprite("504 R VINE HANGING ",
			0, 0, 0, 100, 0xe00);
	if (acrossFlag)
		_vines2 = series_place_sprite("VINE SPRITE ACROSS CHASM",
			0, 0, 0, 100, 0xe00);
	if (tiedFlag)
		_vines3 = series_place_sprite("VINE SPRITE AFTER FIRST TIED",
			0, 0, 0, 100, 0xe00);
	if (rightVine)
		_vines4 = series_place_sprite("504 R VINE SPRITE", 0, 0, 0, 100, 0xe00);
	if (ropeFlag)
		_rope = series_place_sprite("ROPE SPRITE", 0, 0, 0, 100, 0xe00);
}

void Room504::freeVines() {
	terminateMachineAndNull(_vines1);
	terminateMachineAndNull(_vines2);
	terminateMachineAndNull(_vines3);
	terminateMachineAndNull(_vines4);
	terminateMachineAndNull(_rope);
}

void Room504::setVinesRope() {
	disableVinesRope();

	if (_G(flags)[V154] == 2) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE  ", true);
		addRope();
	}

	if (_G(flags)[V152] == 2) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE  ", true);
		addGreenVine();
	}

	if (_G(flags)[V153] == 2) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE  ", true);
		addBrownVine();
	}

	if (_G(flags)[154] == 1) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE COIL ", true);
	}

	if (_G(flags)[V152] == 1) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE COIL ", true);
	}

	if (_G(flags)[V153] == 1) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE COIL ", true);
	}

	if (_G(flags)[V171] == 1) {
		hotspot_set_active("VINES ", true);
		hotspot_set_active("COIL OF VINES ", true);
	}

	if (_G(flags)[V154] == 0) {
		hotspot_set_active("ROPE  ", true);
		hotspot_set_active("ROPE COIL   ", true);
	}

	if (_G(flags)[V152] == 0) {
		hotspot_set_active("GREEN VINE  ", true);
		hotspot_set_active("GREEN VINE COIL   ", true);
	}

	if (_G(flags)[V153] == 0) {
		hotspot_set_active("BROWN VINE  ", true);
		hotspot_set_active("BROWN VINE COIL   ", true);
	}

	if (_G(flags)[V171] == 0) {
		hotspot_set_active("VINES  ", true);
		hotspot_set_active("COIL OF VINES   ", true);
	}

	if (_G(flags)[V154] == 4) {
		hotspot_set_active("ROPE ", true);
		hotspot_set_active("ROPE COIL  ", true);
	}

	if (_G(flags)[V152] == 4) {
		hotspot_set_active("GREEN VINE ", true);
		hotspot_set_active("GREEN VINE  ", true);
	}

	if (_G(flags)[V153] == 4) {
		hotspot_set_active("BROWN VINE ", true);
		hotspot_set_active("BROWN VINE COIL  ", true);
	}

	if (_G(flags)[V171] == 4) {
		hotspot_set_active("VINES ", true);
		hotspot_set_active("COIL OF VINES  ", true);
	}

	if (_G(flags)[V154] == 5) {
		hotspot_set_active("ROPE  ", true);
		hotspot_set_active("ROPE COIL    ", true);
	}

	if (_G(flags)[V152] == 5) {
		hotspot_set_active("GREEN VINE  ", true);
		hotspot_set_active("GREEN VINE COIL    ", true);
	}

	if (_G(flags)[V153] == 5) {
		hotspot_set_active("BROWN VINE  ", true);
		hotspot_set_active("BROWN VINE COIL    ", true);
	}

	if (_G(flags)[V171] == 5) {
		hotspot_set_active("VINES  ", true);
		hotspot_set_active("COIL OF VINES    ", true);
	}
}

void Room504::setMiscItems() {
	hotspot_set_active("WHEELED TOY", false);

	hotspot_set_active("SHOVEL", false);
	deleteHotspot("SHOVEL ");

	hotspot_set_active("WOODEN LADDER", false);
	hotspot_set_active("WOODEN LADDER ", false);
	hotspot_set_active("DRIFTWOOD STUMP", false);
	hotspot_set_active("POLE", false);
	hotspot_set_active("HOLE", false);
	hotspot_set_active("HOLE ", false);
	hotspot_set_active("PERSON IN HOLE", false);
	hotspot_set_active("MENENDEZ", false);

	terminateMachineAndNull(_ladder);
	terminateMachineAndNull(_toy);
	terminateMachineAndNull(_shovel);
	terminateMachineAndNull(_driftwood);
	terminateMachineAndNull(_pole);

	if (_G(flags)[V155] == 0) {
		_ladder = series_place_sprite("MENENDEZ LADDER", 0, 640, 0, 100, 0xf00);
		hotspot_set_active("WOODEN LADDER", true);
	}

	if (_G(flags)[V155] == 1 && inv_object_is_here("WOODEN LADDER")) {
		_ladder = series_place_sprite("504 DROPPED LADDER", 0, 0, 0, 100, 0xe00);
		hotspot_set_active("WOODEN LADDER ", true);
	}

	if (inv_object_is_here("WHEELED TOY")) {
		_toy = series_place_sprite("504TOY", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("WHEELED TOY", true);
	}

	if (_G(flags)[V044] == 0 && inv_object_is_here("SHOVEL") &&
			_G(flags)[V141] == 1) {
		_shovel = series_place_sprite("504shov", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("SHOVEL", true);
	}

	if (_G(flags)[V044] == 1 && inv_object_is_here("SHOVEL") &&
			_G(flags)[V141] == 1) {
		_shovel = series_place_sprite("504 DROPPED SHOVEL", 0, 0, 0, 100, 0xff0);
		addShovel();
	}

	if (inv_object_is_here("DRIFTWOOD STUMP")) {
		_shovel = series_place_sprite("504 DROPPED DRIFTWOOD", 0, 0, 0, 100, 0x750);
		hotspot_set_active("DRIFTWOOD STUMP", true);
	}

	if (inv_object_is_here("POLE")) {
		_shovel = series_place_sprite("504 DROPPED POLE", 0, 0, 0, 100, 0x750);
		hotspot_set_active("POLE", true);
	}

	if (_G(flags)[V141] != 0)
		hotspot_set_active("HOLE ", true);
	else if (_G(flags)[V040] != 1)
		hotspot_set_active("PERSON IN HOLE", true);
	else
		hotspot_set_active("MENENDEZ", true);
}

void Room504::disableVinesRope() {
	static const char *HOTSPOTS[] = {
		"ROPE ", "GREEN VINE ", "BROWN VINE ", "VINES ",
		"ROPE  ", "GREEN VINE  ", "BROWN VINE  "
		"VINES  ", "ROPE COIL ", "GREEN VINE COIL ",
		"BROWN VINE COIL ", "COIL OF VINES ",
		"ROPE COIL  ", "GREEN VINE COIL  ",
		"BROWN VINE COIL  ", "COIL OF VINES  ",
		"ROPE COIL   ", "GREEN VINE COIL   ",
		"BROWN VINE COIL   ", "COIL OF VINES   ",
		"ROPE COIL    ", "GREEN VINE COIL    ",
		"BROWN VINE COIL    ", "COIL OF VINES    ",
		nullptr
	};
	for (const char **hs = HOTSPOTS; *hs; ++hs)
		hotspot_set_active(*hs, false);

	deleteHotspot("ROPE   ");
	deleteHotspot("GREEN VINE   ");
	deleteHotspot("BROWN VINE   ");
}

void Room504::deleteHotspot(const char *hotspotName) {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next) {
		if (!strcmp(hs->vocab, hotspotName)) {
			_G(currentSceneDef).hotspots = hotspot_delete_record(
				_G(currentSceneDef).hotspots, hs);
			break;
		}
	}
}

void Room504::addRope() {
	if (_G(flags)[V152] != 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "ROPE   ");
	if (_G(flags)[V152] == 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 95, "LOOK AT", "ROPE   ");
	if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2)
		addHotspot(145, 73, 460, 88, "LOOK AT", "ROPE   ");
}

void Room504::addGreenVine() {
	if (_G(flags)[V154] != 2 && _G(flags)[V153] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V153] != 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] != 2 && _G(flags)[V153] == 2)
		addHotspot(145, 73, 460, 95, "LOOK AT", "GREEN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V153] == 2)
		addHotspot(145, 89, 460, 104, "LOOK AT", "GREEN VINE   ");
}

void Room504::addBrownVine() {
	if (_G(flags)[V154] != 2 && _G(flags)[V152] != 2)
		addHotspot(145, 73, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V152] != 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] != 2 && _G(flags)[V152] == 2)
		addHotspot(145, 96, 460, 118, "LOOK AT", "BROWN VINE   ");
	if (_G(flags)[V154] == 2 && _G(flags)[V152] == 2)
		addHotspot(145, 104, 460, 118, "LOOK AT", "BROWN VINE   ");
}

void Room504::addHotspot(int x1, int y1, int x2, int y2,
		const char *verb, const char *vocab) {
	HotSpotRec *hs = hotspot_new(x1, y1, x2, y2);
	hotspot_newVerb(hs, verb);
	hotspot_newVocab(hs, vocab);
	hs->cursor_number = 6;
	hotspot_add(_G(currentSceneDef).hotspots, hs, true);
}

void Room504::addShovel() {
	HotSpotRec *hs = hotspot_new(529, 109, 546, 143);
	hotspot_newVerb(hs, "LOOK AT");
	hotspot_newVocab(hs, "SHOVEL ");
	hs->cursor_number = 6;
	hs->feet_x = 543;
	hs->feet_y = 142;
	hs->facing = 11;

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots, hs, true);
}

bool Room504::checkVinesDistance() {
	if (player_said("ROPE") && inv_player_has("ROPE"))
		digi_play("504R49", 1);
	else if (player_said("GREEN VINE") && inv_player_has("GREEN VINE"))
		digi_play("504R49", 1);
	else if (player_said("BROWN VINE") && inv_player_has("BROWN VINE"))
		digi_play("504R49", 1);
	else if (player_said("VINES") && inv_player_has("VINES"))
		digi_play("504R49", 1);
	else
		return false;

	return true;
}

bool Room504::parser1() {
	if (!_flag1 && _G(player).click_x > 300) {
		_G(kernel).trigger_mode = KT_DAEMON;
		if (inv_player_has("DRIFTWOOD STUMP") || inv_player_has("POLE")) {
			kernel_timing_trigger(1, 707);
		} else if (!_flag2) {
			ws_walk(200, 153, nullptr, 714, 3);
		} else {
			ws_walk(200, 153, nullptr, 570, 3);
		}
	} else if (_flag1 && _G(player).click_x <= 300) {
		_flag3 = player_said("EXIT") && _flag2;
		_G(kernel).trigger_mode = KT_DAEMON;

		if (inv_player_has("WOODEN LADDER") ||
				inv_player_has("LADDER/ROPE") ||
				inv_player_has("LADDER/GREEN VINE") ||
				inv_player_has("LADDER/BROWN VINE") ||
				inv_player_has("LADDER/VINES")) {
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 725);
		} else if (inv_player_has("SHOVEL")) {
			kernel_timing_trigger(1, 755);
		} else if (_flag2) {
			ws_walk(528, 168, nullptr, 558, 8);
		} else {
			ws_walk(528, 168, nullptr, 734, 8);
		}
	} else {
		return false;
	}

	return true;
}

void Room504::conv504a() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		if (who <= 0)
			_convState1 = 2;
		else if (who == 1)
			_convState2 = 1;

		conv_resume();
	} else {
		if (who <= 0) {
			if (node == 8 && entry == 3) {
				_convState1 = 10;
			} else if (node == 19 && entry == 2) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 669);
			} else {
				_convState1 = 4;

				if (sound)
					digi_play(sound, 1, 255, 1);
				else
					conv_resume();
			}
		} else if (who == 1) {
			if ((node == 15 && entry == 0) || (node == 18 && entry == 0)) {
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 657);
			} else {
				_convState2 = 3;

				if (sound)
					digi_play(sound, 1, 255, 1);
				else
					conv_resume();
			}
		}
	}
}

bool Room504::lookVines() {
	if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2 &&
		_G(flags)[V154] == 2) {
		digi_play("504R38", 1);
	} else if (_G(flags)[V154] == 2 && _G(flags)[V152] == 2) {
		digi_play("504R37", 1);
	} else if (_G(flags)[V154] == 2 && _G(flags)[V153] == 2) {
		digi_play("504R37", 1);
	} else if (_G(flags)[V154] == 2) {
		if (_G(flags)[V167]) {
			digi_play("504R05A", 1);
		} else {
			digi_play("504R05", 1);
			_G(flags)[V167] = 1;
		}
	} else if (_G(flags)[V152] == 2 && _G(flags)[V153] == 2) {
		digi_play("504R42", 1);
	} else if (_G(flags)[V152] == 2 || _G(flags)[V153] == 2) {
		digi_play("504R41", 1);
	} else {
		return false;
	}

	return true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
