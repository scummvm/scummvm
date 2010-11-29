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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/mohawk/myst_scripts.cpp $
 * $Id: myst_scripts.cpp 54107 2010-11-07 01:03:29Z fingolfin $
 *
 */

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_scripts_selenitic.h"
#include "mohawk/myst_saveload.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "gui/message.h"

namespace Mohawk {

#define OPCODE(op, x) { op, &MystScriptParser::x, #x }
#define SPECIFIC_OPCODE(op, x) { op, (OpcodeProcMyst) &MystScriptParser_Selenitic::x, #x }

MystScriptParser_Selenitic::MystScriptParser_Selenitic(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
	_invokingResource = NULL;
}

MystScriptParser_Selenitic::~MystScriptParser_Selenitic() {
}

void MystScriptParser_Selenitic::setupOpcodes() {
	// "invalid" opcodes do not exist or have not been observed
	// "unknown" opcodes exist, but their meaning is unknown

	static const MystOpcode myst_opcodes[] = {
		// "Standard" Opcodes
		OPCODE(0, o_0_toggleVar),
		OPCODE(1, o_1_setVar),
		OPCODE(2, o_2_changeCardSwitch),
		OPCODE(3, o_3_takePage),
		OPCODE(4, o_4_redrawCard),
		// TODO: Opcode 5 Not Present
		OPCODE(6, o_6_goToDest),
		OPCODE(7, o_6_goToDest),
		OPCODE(8, o_6_goToDest),
		OPCODE(9, o_9_triggerMovie),
		OPCODE(10, o_10_toggleVarNoRedraw),
		// TODO: Opcode 10 to 11 Not Present
		OPCODE(12, o_2_changeCardSwitch),
		OPCODE(13, o_2_changeCardSwitch),
		OPCODE(14, o_14_drawAreaState),
		OPCODE(15, o_15_redrawAreaForVar),
		OPCODE(16, o_16_changeCardDirectional),
		OPCODE(17, o_17_changeCardPush),
		OPCODE(18, o_18_changeCardPop),
		OPCODE(19, o_19_enableAreas),
		OPCODE(20, o_20_disableAreas),
		OPCODE(21, o_21_directionalUpdate),
		OPCODE(22, o_6_goToDest),
		OPCODE(23, o_23_toggleAreasActivation),
		OPCODE(24, o_24_playSound),
		// TODO: Opcode 25 Not Present
		OPCODE(26, o_26_stopSoundBackground),
		OPCODE(27, o_27_playSoundBlocking),
		OPCODE(28, o_28_restoreDefaultRect),
		OPCODE(29, o_29_33_blitRect),
		OPCODE(30, o_30_changeSound),
		OPCODE(31, o_31_soundPlaySwitch),
		OPCODE(32, o_32_soundResumeBackground),
		OPCODE(33, o_29_33_blitRect),
		OPCODE(34, o_34_changeCard),
		OPCODE(35, o_35_drawImageChangeCard),
		OPCODE(36, o_36_changeMainCursor),
		OPCODE(37, o_37_hideCursor),
		OPCODE(38, o_38_showCursor),
		OPCODE(39, o_39_delay),
		OPCODE(40, o_40_changeStack),
		OPCODE(41, o_41_changeCardPlaySoundDirectional),
		OPCODE(42, o_42_directionalUpdatePlaySound),
		OPCODE(43, o_43_saveMainCursor),
		OPCODE(44, o_44_restoreMainCursor),
		// TODO: Opcode 45 Not Present
		OPCODE(46, o_46_soundWaitStop),
		// TODO: Opcodes 47 to 99 Not Present

		// "Stack-Specific" Opcodes
		SPECIFIC_OPCODE(100, opcode_100),
		SPECIFIC_OPCODE(101, opcode_101),
		SPECIFIC_OPCODE(102, o_102_soundReceiverSigma),
		SPECIFIC_OPCODE(103, o_103_soundReceiverRight),
		SPECIFIC_OPCODE(104, o_104_soundReceiverLeft),
		SPECIFIC_OPCODE(105, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(106, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(107, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(108, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(109, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(110, opcode_110),
		SPECIFIC_OPCODE(111, o_111_soundReceiverUpdateSound),
		SPECIFIC_OPCODE(112, o_112_soundLockMove),
		SPECIFIC_OPCODE(113, o_113_soundLockStartMove),
		SPECIFIC_OPCODE(114, o_114_soundLockEndMove),
		SPECIFIC_OPCODE(115, o_115_soundLockButton),
		SPECIFIC_OPCODE(116, NOP),
		SPECIFIC_OPCODE(117, o_117_soundReceiverEndMove),

		// "Init" Opcodes
		SPECIFIC_OPCODE(200, opcode_200),
		SPECIFIC_OPCODE(201, opcode_201),
		SPECIFIC_OPCODE(202, opcode_202),
		SPECIFIC_OPCODE(203, o_203_soundReceiver_init),
		SPECIFIC_OPCODE(204, o_204_soundLock_init),
		SPECIFIC_OPCODE(205, opcode_205),
		SPECIFIC_OPCODE(206, opcode_206),

		OPCODE(0xFFFF, NOP)
	};

	_opcodes = myst_opcodes;
	_opcodeCount = ARRAYSIZE(myst_opcodes);
}

void MystScriptParser_Selenitic::disablePersistentScripts() {
	_sound_receiver_running = false;
}

void MystScriptParser_Selenitic::runPersistentScripts() {
	if (_sound_receiver_running)
		o_203_soundReceiver_run();
}

uint16 MystScriptParser_Selenitic::getVar(uint16 var) {
	uint16 *game_globals = _vm->_saveLoad->_v->game_globals;
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	switch(var) {
    case 0: // Sound receiver emitters enabled
    	return selenitic_vars[4];
	case 1:
		return selenitic_vars[1];
    case 2:
    	return selenitic_vars[2];
	case 3:
		return selenitic_vars[0];
	case 4:
		return selenitic_vars[3];
	case 5: // Sound receiver opened
		return selenitic_vars[5];
	case 6: // Tunnel lights
		return selenitic_vars[6];
    case 8: // Viewer
    	return 0;
    case 9: // Sound receiver selected source
    	return selenitic_vars[7] == 0;
	case 10:
		return selenitic_vars[7] == 1;
    case 11:
    	return selenitic_vars[7] == 2;
    case 12:
    	return selenitic_vars[7] == 3;
    case 13:
    	return selenitic_vars[7] == 4;
	case 14: // Sound receiver position
		return (*_sound_receiver_position) / 1000;
	case 15:
		return ((*_sound_receiver_position) / 100) % 10;
	case 16:
		return ((*_sound_receiver_position) / 10) % 10;
	case 17:
		return (*_sound_receiver_position) % 10;
    case 20: // Sound lock sliders state
    case 21:
    case 22:
    case 23:
    case 24:
    	return 1;
	case 26:
		return _sound_receiver_sigma_pressed;
	case 27:
	case 29: // Maze runner door
		return 0;
	case 30:
		return _maze_runner_door_opened;
	case 102: // Red page
		return !(game_globals[6] & 2) && (game_globals[2] != 8);
	case 103: // Blue page
		return !(game_globals[7] & 2) && (game_globals[2] != 2);
	default:
		return MystScriptParser::getVar(var);
	}
}

void MystScriptParser_Selenitic::toggleVar(uint16 var) {
	uint16 *game_globals = _vm->_saveLoad->_v->game_globals;
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	switch(var) {
    case 0: // Sound receiver emitters enabled
    	selenitic_vars[4] = (selenitic_vars[4] + 1) % 2;
    	break;
	case 1:
		selenitic_vars[1] = (selenitic_vars[1] + 1) % 2;
    	break;
    case 2:
    	selenitic_vars[2] = (selenitic_vars[2] + 1) % 2;
    	break;
	case 3:
		selenitic_vars[0] = (selenitic_vars[0] + 1) % 2;
    	break;
	case 4:
		selenitic_vars[3] = (selenitic_vars[3] + 1) % 2;
    	break;
	case 5: // Sound receiver opened
		selenitic_vars[5] = (selenitic_vars[5] + 1) % 2;
    	break;
	case 6: // Tunnel lights
		selenitic_vars[6] = (selenitic_vars[6] + 1) % 2;
    	break;
	case 102: // Red page
        if (!(game_globals[6] & 2)) {
			if (game_globals[2] == 8)
				game_globals[2] = 0;
			else {
				//TODO: Cursor animation
				game_globals[2] = 8;
			}
		}
		break;
	case 103: // Blue page
        if (!(game_globals[7] & 2)) {
			if (game_globals[2] == 2)
				game_globals[2] = 0;
			else {
				//TODO: Cursor animation
				game_globals[2] = 2;
			}
		}
		break;
	default:
		MystScriptParser::toggleVar(var);
		break;
	}
}

bool MystScriptParser_Selenitic::setVarValue(uint16 var, uint16 value) {
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;
	bool refresh = false;

	switch (var) {
    case 0: // Sound receiver emitters enabled
    	if (selenitic_vars[4] != value) {
    		selenitic_vars[4] = value;
    		refresh = true;
    	}
    	break;
	case 1:
    	if (selenitic_vars[1] != value) {
    		selenitic_vars[1] = value;
    		refresh = true;
    	}
    	break;
    case 2:
    	if (selenitic_vars[2] != value) {
    		selenitic_vars[2] = value;
    		refresh = true;
    	}
    	break;
	case 3:
    	if (selenitic_vars[0] != value) {
    		selenitic_vars[0] = value;
    		refresh = true;
    	}
    	break;
	case 4:
    	if (selenitic_vars[3] != value) {
    		selenitic_vars[3] = value;
    		refresh = true;
    	}
    	break;
	case 5: // Sound receiver opened
    	if (selenitic_vars[5] != value) {
    		selenitic_vars[5] = value;
    		refresh = true;
    	}
    	break;
	case 6: // Tunnel lights
    	if (selenitic_vars[6] != value) {
    		selenitic_vars[6] = value;
    		refresh = true;
    	}
    	break;
	case 20: // Sound lock sliders
		selenitic_vars[13] = value;
		break;
	case 21:
		selenitic_vars[14] = value;
		break;
	case 22:
		selenitic_vars[15] = value;
		break;
	case 23:
		selenitic_vars[16] = value;
		break;
	case 24:
		selenitic_vars[17] = value;
		break;
	case 30:
		_maze_runner_door_opened = value;
		break;
	default:
		refresh = MystScriptParser::setVarValue(var, value);
		break;
	}

	return refresh;
}

void MystScriptParser_Selenitic::opcode_100(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 1191 (Maze Runner)
		// Called when Red Warning Button is pushed

		debugC(kDebugScript, "Opcode %d: Repeat Buzzer Sound?", op);

		// TODO: Fill in logic...
		// Repeat buzzer sound
	} else
		unknown(op, var, argc, argv);
}

/**
 * Sound receiver sigma button
 */
void MystScriptParser_Selenitic::o_102_soundReceiverSigma(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	debugC(kDebugScript, "Opcode %d: Sound receiver sigma button", op);

	_vm->_cursor->hideCursor();

	_sound_receiver_current_source->drawConditionalDataToScreen(0);

	uint16 *oldPosition = _sound_receiver_position;
	uint16 source = 0;

	for (uint i = 0; i < 5; i++) {
	    switch (i) {
		case 0:
			source = 3;
			break;
		case 1:
			source = 0;
			break;
		case 2:
			source = 4;
			break;
		case 3:
			source = 1;
			break;
		case 4:
			source = 2;
			break;
		}

		_sound_receiver_position = &selenitic_vars[8 + source];
		_vm->_sound->stopSound();
		_vm->_sound->playSound(2287);
		sound_receiver_draw_view();
		uint16 soundId = sound_receiver_current_sound(source, *_sound_receiver_position);
		_vm->_sound->replaceSound(soundId);
		_vm->_system->delayMillis(1000);
	}

	_sound_receiver_position = oldPosition;
	_sound_receiver_sigma_pressed = true;
	_vm->_sound->stopSound();

	_sound_receiver_sources[selenitic_vars[7]]->drawConditionalDataToScreen(1);

	sound_receiver_draw_view();

	_vm->_cursor->showCursor();
}

/**
 * Sound receiver right button
 */
void MystScriptParser_Selenitic::o_103_soundReceiverRight(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver right", op);

	sound_receiver_left_right(1);
}

/**
 * Sound receiver left button
 */
void MystScriptParser_Selenitic::o_104_soundReceiverLeft(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver left", op);

	sound_receiver_left_right(2);
}

void MystScriptParser_Selenitic::sound_receiver_left_right(uint direction) {

	if (_sound_receiver_sigma_pressed) {
		_sound_receiver_sigma_button->drawConditionalDataToScreen(0);
		_sound_receiver_sigma_pressed = false;
	}

	if (direction == 1) {
		_sound_receiver_right_button->drawConditionalDataToScreen(1);
	} else {
		_sound_receiver_left_button->drawConditionalDataToScreen(1);
	}

	_vm->_sound->stopSound();

	_sound_receiver_direction = direction;
	_sound_receiver_speed = 1;
	_sound_receiver_start_time = _vm->_system->getMillis();

	sound_receiver_update();
}

void MystScriptParser_Selenitic::sound_receiver_update() {
	if (_sound_receiver_direction == 1) {
		*_sound_receiver_position = ((*_sound_receiver_position) + _sound_receiver_speed) % 3600;
	} else if (_sound_receiver_direction == 2) {
		*_sound_receiver_position = ((*_sound_receiver_position) + 3600 - _sound_receiver_speed) % 3600;
	}

	sound_receiver_draw_view();
}

void MystScriptParser_Selenitic::sound_receiver_draw_view() {
	uint32 left = ((*_sound_receiver_position) * 1800) / 3600;

	_sound_receiver_viewer->_subImages->rect.left = left;
	_sound_receiver_viewer->_subImages->rect.right = left + 136;

	_sound_receiver_viewer->drawConditionalDataToScreen(0);

	sound_receiver_draw_angle();
}

void MystScriptParser_Selenitic::sound_receiver_draw_angle() {
	_vm->redrawResource(_sound_receiver_angle_1);
	_vm->redrawResource(_sound_receiver_angle_2);
	_vm->redrawResource(_sound_receiver_angle_3);
	_vm->redrawResource(_sound_receiver_angle_4);
}

/**
 * Sound receiver source selection buttons
 */
void MystScriptParser_Selenitic::o_105_109_soundReceiverSource(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
    uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	debugC(kDebugScript, "Opcode %d: Sound receiver source", op);

	if (_sound_receiver_sigma_pressed) {
		_sound_receiver_sigma_button->drawConditionalDataToScreen(0);
		_sound_receiver_sigma_pressed = false;
	}

	_vm->_cursor->hideCursor();

	uint pressed_button = var - 9;

	if (selenitic_vars[7] != pressed_button) {
		selenitic_vars[7] = pressed_button;

		_sound_receiver_current_source->drawConditionalDataToScreen(0);

		_sound_receiver_position = &selenitic_vars[8 + pressed_button];
		_sound_receiver_current_source = _sound_receiver_sources[pressed_button];

		_vm->_sound->stopSound();

		uint16 sound_id = argv[0];
		_vm->_sound->playSound(sound_id);

		_sound_receiver_current_source->drawConditionalDataToScreen(1);

		sound_receiver_draw_view();
	}

	_vm->_cursor->showCursor();
}

void MystScriptParser_Selenitic::opcode_110(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 15) {
		// Used for Selenitic Maze Runner Exit Logic
		uint16 CardIdEntry = argv[0];
		uint16 CardIdExit = argv[1];
		uint16 u0 = argv[2];
		Common::Rect rect1 = Common::Rect(argv[3], argv[4], argv[5], argv[6]);
		uint16 rect1UpdateDirection = argv[7];
		uint16 u1 = argv[8];
		Common::Rect rect2 = Common::Rect(argv[9], argv[10], argv[11], argv[12]);
		uint16 rect2UpdateDirection = argv[13];
		uint16 u2 = argv[14];

		debugC(kDebugScript, "Opcode %d: Maze Runner Exit Logic and Door Open Animation", op);
		debugC(kDebugScript, "\tExit Card: %d", CardIdEntry);
		debugC(kDebugScript, "\tEntry Card: %d", CardIdExit);
		debugC(kDebugScript, "\tu0 (Exit Var?): %d", u0);

		debugC(kDebugScript, "\trect1.left: %d", rect1.left);
		debugC(kDebugScript, "\trect1.top: %d", rect1.top);
		debugC(kDebugScript, "\trect1.right: %d", rect1.right);
		debugC(kDebugScript, "\trect1.bottom: %d", rect1.bottom);
		debugC(kDebugScript, "\trect1 updateDirection: %d", rect1UpdateDirection);
		debugC(kDebugScript, "\tu1: %d", u1);

		debugC(kDebugScript, "\trect2.left: %d", rect2.left);
		debugC(kDebugScript, "\trect2.top: %d", rect2.top);
		debugC(kDebugScript, "\trect2.right: %d", rect2.right);
		debugC(kDebugScript, "\trect2.bottom: %d", rect2.bottom);
		debugC(kDebugScript, "\trect2 updateDirection: %d", rect2UpdateDirection);
		debugC(kDebugScript, "\tu2: %d", u2);

		// TODO: Finish Implementing Logic...
		// HACK: Bypass Higher Logic for now...
		_vm->changeToCard(argv[1], true);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::o_111_soundReceiverUpdateSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver update sound", op);

	sound_receiver_update_sound();
}

uint16 MystScriptParser_Selenitic::soundLockCurrentSound(uint16 position, bool pixels) {
	if ((pixels && position < 96) || (!pixels && position == 0)) {
		return 289;
	} else if ((pixels && position < 108) || (!pixels && position == 1)) {
		return 1289;
	} else if ((pixels && position < 120) || (!pixels && position == 2)) {
		return 2289;
	} else if ((pixels && position < 132) || (!pixels && position == 3)) {
		return 3289;
	} else if ((pixels && position < 144) || (!pixels && position == 4)) {
		return 4289;
	} else if ((pixels && position < 156) || (!pixels && position == 5)) {
		return 5289;
	} else if ((pixels && position < 168) || (!pixels && position == 6)) {
		return 6289;
	} else if ((pixels && position < 180) || (!pixels && position == 7)) {
		return 7289;
	} else if ((pixels && position < 192) || (!pixels && position == 8)) {
		return 8289;
	} else if (pixels || (!pixels && position == 9)) {
		return 9289;
	} else {
		return 0;
	}
}

MystResourceType10 *MystScriptParser_Selenitic::soundLockSliderFromVar(uint16 var) {
	switch (var) {
	case 20:
		return _sound_lock_slider_1;
	case 21:
		return _sound_lock_slider_2;
	case 22:
		return _sound_lock_slider_3;
	case 23:
		return _sound_lock_slider_4;
	case 24:
		return _sound_lock_slider_5;
	}

	return 0;
}

void MystScriptParser_Selenitic::o_112_soundLockMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock move", op);

	MystResourceType10 *slider = soundLockSliderFromVar(var);

	uint16 soundId = soundLockCurrentSound(slider->_pos.y, true);
	if (_sound_lock_sound_id != soundId) {
		_sound_lock_sound_id = soundId;
		_vm->_sound->replaceSound(soundId, Audio::Mixer::kMaxChannelVolume, true);
	}
}

void MystScriptParser_Selenitic::o_113_soundLockStartMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock start move", op);

	MystResourceType10 *slider = soundLockSliderFromVar(var);

	_vm->_cursor->setCursor(700);
	_vm->_sound->pauseBackground();

	_sound_lock_sound_id = soundLockCurrentSound(slider->_pos.y, true);
	_vm->_sound->replaceSound(_sound_lock_sound_id, Audio::Mixer::kMaxChannelVolume, true);
}

void MystScriptParser_Selenitic::o_114_soundLockEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock end move", op);

	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;
	MystResourceType10 *slider = soundLockSliderFromVar(var);
	uint16 *value = 0;

	switch (var) {
	case 20: // Sound lock sliders
		value = &selenitic_vars[13];
		break;
	case 21:
		value = &selenitic_vars[14];
		break;
	case 22:
		value = &selenitic_vars[15];
		break;
	case 23:
		value = &selenitic_vars[16];
		break;
	case 24:
		value = &selenitic_vars[17];
		break;
	}

	uint16 stepped = 12 * (*value / 12) + 6;

	if ( stepped == 6 )
		stepped = 0;
	if ( stepped == 114 )
		stepped = 119;

	*value = stepped;

	slider->setStep(stepped);
	slider->drawDataToScreen();
	_vm->_gfx->updateScreen();

	uint16 soundId = slider->getList3(0);
	if (soundId)
		_vm->_sound->playSoundBlocking(soundId);

	_vm->_sound->stopSound();
	_vm->_sound->resumeBackground();
}

void MystScriptParser_Selenitic::soundLockCheckSolution(MystResourceType10 *slider, uint16 value, uint16 solution, bool &solved) {
	slider->drawConditionalDataToScreen(2);
	_vm->_sound->playSound(soundLockCurrentSound(value / 12, false));
	_vm->_system->delayMillis(1500);

	if (value / 12 != solution) {
		solved = false;
	}

	slider->drawConditionalDataToScreen(1);
	_vm->_sound->stopSound();
}

void MystScriptParser_Selenitic::o_115_soundLockButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound lock button", op);

	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;
	bool solved = true;

	_vm->_sound->pauseBackground();
	_vm->_sound->playSound(1147);
	_sound_lock_button->drawConditionalDataToScreen(1);
	_vm->_cursor->hideCursor();

	soundLockCheckSolution(_sound_lock_slider_1, selenitic_vars[13], 5, solved);
	soundLockCheckSolution(_sound_lock_slider_2, selenitic_vars[14], 9, solved);
	soundLockCheckSolution(_sound_lock_slider_3, selenitic_vars[15], 0, solved);
	soundLockCheckSolution(_sound_lock_slider_4, selenitic_vars[16], 6, solved);
	soundLockCheckSolution(_sound_lock_slider_5, selenitic_vars[17], 7, solved);

	_vm->_sound->playSound(1148);
	_vm->_sound->resumeBackground();

	if (solved) {
		_sound_lock_button->drawConditionalDataToScreen(2);

		uint16 cardIdClosed = argv[0];
		uint16 cardIdOpen = argv[1];

		_vm->changeToCard(cardIdClosed, true);

		_vm->changeToCard(cardIdOpen, false);
		_vm->_sound->playSound(argv[2]);

		animatedUpdate(argv[4], &argv[5], argv[3]);
	} else {
		_sound_lock_button->drawConditionalDataToScreen(0);
	}

	_vm->_cursor->showCursor();
}

void MystScriptParser_Selenitic::o_117_soundReceiverEndMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Sound receiver end move", op);

	uint16 old_direction = _sound_receiver_direction;

	if (_sound_receiver_direction) {
		_sound_receiver_direction = 0;

		sound_receiver_update_sound();

		if (old_direction == 1) {
			_sound_receiver_right_button->drawConditionalDataToScreen(0);
		} else {
			_sound_receiver_left_button->drawConditionalDataToScreen(0);
		}

	}
}

// Selenitic Stack Movies For Maze Runner (Card 1191)
//static const char* kHCMovPathSelenitic[36] = {
//	"backa1",
//	"backe1",
//	"backf0",
//	"backf1",
//	"backl0",
//	"backl1",
//	"backo0",
//	"backo1",
//	"backp0",
//	"backp1",
//	"backr0",
//	"backr1",
//	"backs0",
//	"backs1",
//	"forwa1",
//	"forwe0",
//	"forwf0",
//	"forwf1",
//	"forwl0",
//	"forwl1",
//	"forwo0",
//	"forwo1",
//	"forwp0",
//	"forwp1",
//	"forwr0",
//	"forwr1",
//	"forws0",
//	"forws1",
//	"left00",
//	"left01",
//	"left10",
//	"left11",
//	"right00",
//	"right01",
//	"right10",
//	"right11"
//};

void MystScriptParser_Selenitic::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {

	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)

	if (argc == 0) {

	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {

	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::o_203_soundReceiver_run(void) {
	if (_sound_receiver_start_time) {
		if (_sound_receiver_direction) {
			uint32 current_time = _vm->_system->getMillis();

			if (_sound_receiver_speed == 50) {
				if (current_time > _sound_receiver_start_time + 500) {
					sound_receiver_increase_speed();
				}
			} else {
				if (current_time > _sound_receiver_start_time + 1000) {
					sound_receiver_increase_speed();
				}
			}

			if (current_time > _sound_receiver_start_time + 100) {
				sound_receiver_update();
			}

		} else if (!_sound_receiver_sigma_pressed) {
			sound_receiver_update_sound();
		}
	}
}

void MystScriptParser_Selenitic::sound_receiver_increase_speed() {
	switch (_sound_receiver_speed) {
	case 1:
		_sound_receiver_speed = 10;
		break;
	case 10:
		_sound_receiver_speed = 50;
		break;
	case 50:
		_sound_receiver_speed = 100;
		break;
	}
}

void MystScriptParser_Selenitic::sound_receiver_update_sound() {
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	uint16 soundId = sound_receiver_current_sound(selenitic_vars[7], *_sound_receiver_position);
	_vm->_sound->replaceSound(soundId);
}

uint16 MystScriptParser_Selenitic::sound_receiver_current_sound(uint16 source, uint16 position) {
	uint16 solution;
	bool sourceEnabled;
	sound_receiver_solution(source, solution, sourceEnabled);

	uint16 soundIdGood;
	uint16 soundIdNear;
	uint16 soundId = 1245;

	switch (source) {
	case 0:
		soundIdNear = 3245;
		soundIdGood = 3093;
		break;
	case 1:
		soundIdNear = 5245;
		soundIdGood = 5093;
		break;
	case 2:
		soundIdNear = 6245;
		soundIdGood = 6093;
		break;
	case 3:
		soundIdNear = 2245;
		soundIdGood = 2093;
		break;
	case 4:
		soundIdNear = 4245;
		soundIdGood = 4093;
		break;
	}

	if (sourceEnabled) {
		if (position == solution) {
			soundId = soundIdGood;
		} else if (position > solution && position <= solution + 50) {
			_sound_receiver_left_button->drawConditionalDataToScreen(2);
			_sound_receiver_left_button->drawConditionalDataToScreen(0);
			soundId = soundIdNear;
		} else if (position < solution && position >= solution - 50) {
			_sound_receiver_right_button->drawConditionalDataToScreen(2);
			_sound_receiver_right_button->drawConditionalDataToScreen(0);
			soundId = soundIdNear;
		}
	}

	return soundId;
}

void MystScriptParser_Selenitic::sound_receiver_solution(uint16 source, uint16 &solution, bool &enabled) {
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	switch (source) {
	case 0:
		enabled = selenitic_vars[0];
		solution = 1534;
		break;
	case 1:
		enabled = selenitic_vars[1];
		solution = 1303;
		break;
	case 2:
		enabled = selenitic_vars[2];
		solution = 556;
		break;
	case 3:
		enabled = selenitic_vars[3];
		solution = 150;
		break;
	case 4:
		enabled = selenitic_vars[4];
		solution = 2122;
		break;
	}
}

void MystScriptParser_Selenitic::o_203_soundReceiver_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	debugC(kDebugScript, "Opcode %d: Sound receiver init", op);

	// Used for Card 1245 (Sound Receiver)
	_sound_receiver_running = true;

	_sound_receiver_right_button = static_cast<MystResourceType8 *>(_vm->_resources[0]);
	_sound_receiver_left_button = static_cast<MystResourceType8 *>(_vm->_resources[1]);
	_sound_receiver_sigma_button = static_cast<MystResourceType8 *>(_vm->_resources[2]);
	_sound_receiver_sources[4] = static_cast<MystResourceType8 *>(_vm->_resources[3]);
	_sound_receiver_sources[3] = static_cast<MystResourceType8 *>(_vm->_resources[4]);
	_sound_receiver_sources[2] = static_cast<MystResourceType8 *>(_vm->_resources[5]);
	_sound_receiver_sources[1] = static_cast<MystResourceType8 *>(_vm->_resources[6]);
	_sound_receiver_sources[0] = static_cast<MystResourceType8 *>(_vm->_resources[7]);
	_sound_receiver_viewer = static_cast<MystResourceType8 *>(_vm->_resources[8]);
	_sound_receiver_angle_1 = static_cast<MystResourceType8 *>(_vm->_resources[10]);
	_sound_receiver_angle_2 = static_cast<MystResourceType8 *>(_vm->_resources[11]);
	_sound_receiver_angle_3 = static_cast<MystResourceType8 *>(_vm->_resources[12]);
	_sound_receiver_angle_4 = static_cast<MystResourceType8 *>(_vm->_resources[13]);

    uint16 current_source = selenitic_vars[7];
    _sound_receiver_position = &selenitic_vars[8 + current_source];
	_sound_receiver_current_source = _sound_receiver_sources[current_source];

	_sound_receiver_sigma_pressed = false;
}

void MystScriptParser_Selenitic::o_204_soundLock_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 *selenitic_vars = _vm->_saveLoad->_v->selenitic_vars;

	debugC(kDebugScript, "Opcode %d: Sound lock init", op);

	for (uint i = 0; i < _vm->_resources.size(); i++) {
		if (_vm->_resources[i]->type == kMystSlider) {
			switch (_vm->_resources[i]->getType8Var()) {
			case 20:
				_sound_lock_slider_1 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_sound_lock_slider_1->setStep(selenitic_vars[13]);
				break;
			case 21:
				_sound_lock_slider_2 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_sound_lock_slider_2->setStep(selenitic_vars[14]);
				break;
			case 22:
				_sound_lock_slider_3 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_sound_lock_slider_3->setStep(selenitic_vars[15]);
				break;
			case 23:
				_sound_lock_slider_4 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_sound_lock_slider_4->setStep(selenitic_vars[16]);
				break;
			case 24:
				_sound_lock_slider_5 = static_cast<MystResourceType10 *>(_vm->_resources[i]);
				_sound_lock_slider_5->setStep(selenitic_vars[17]);
				break;
			}
		} else if (_vm->_resources[i]->type == kMystConditionalImage) {
			if (_vm->_resources[i]->getType8Var() == 28) {
				_sound_lock_button = static_cast<MystResourceType8 *>(_vm->_resources[i]);
			}
		}
	}

	_sound_lock_sound_id = 0;
}

void MystScriptParser_Selenitic::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {

	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Selenitic::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 1191 (Maze Runner)
	if (argc == 0) {

	} else
		unknown(op, var, argc, argv);
}

} // End of namespace Mohawk
