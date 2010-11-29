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
	_maze_runner_position = 288;
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
		SPECIFIC_OPCODE(100, o_100_mazeRunnerMove),
		SPECIFIC_OPCODE(101, o_101_mazeRunnerSoundRepeat),
		SPECIFIC_OPCODE(102, o_102_soundReceiverSigma),
		SPECIFIC_OPCODE(103, o_103_soundReceiverRight),
		SPECIFIC_OPCODE(104, o_104_soundReceiverLeft),
		SPECIFIC_OPCODE(105, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(106, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(107, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(108, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(109, o_105_109_soundReceiverSource),
		SPECIFIC_OPCODE(110, o_110_mazeRunnerDoorButton),
		SPECIFIC_OPCODE(111, o_111_soundReceiverUpdateSound),
		SPECIFIC_OPCODE(112, o_112_soundLockMove),
		SPECIFIC_OPCODE(113, o_113_soundLockStartMove),
		SPECIFIC_OPCODE(114, o_114_soundLockEndMove),
		SPECIFIC_OPCODE(115, o_115_soundLockButton),
		SPECIFIC_OPCODE(116, NOP),
		SPECIFIC_OPCODE(117, o_117_soundReceiverEndMove),

		// "Init" Opcodes
		SPECIFIC_OPCODE(200, o_200_mazeRunnerCompass_init),
		SPECIFIC_OPCODE(201, o_201_mazeRunnerWindow_init),
		SPECIFIC_OPCODE(202, o_202_mazeRunnerLight_init),
		SPECIFIC_OPCODE(203, o_203_soundReceiver_init),
		SPECIFIC_OPCODE(204, o_204_soundLock_init),
		SPECIFIC_OPCODE(205, o_205_mazeRunnerRight_init),
		SPECIFIC_OPCODE(206, o_206_mazeRunnerLeft_init),

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
    case 7:// Maze runner display
    	if (_maze_runner_position == 288) {
    		return 0;
    	} else if (_maze_runner_position == 289) {
    		return 1;
    	} else if (!mazeRunnerForwardAllowed(_maze_runner_position)) {
    		return 2;
    	} else {
    		return 3;
    	}
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
    case 25: // Maze runner direction
    	return _maze_runner_direction;
	case 26:
		return _sound_receiver_sigma_pressed;
	case 27:
	case 29: // Maze runner door
		return 0;
	case 30:
		return _maze_runner_door_opened;
	case 31: // Maze runner forward allowed
		return mazeRunnerForwardAllowed(_maze_runner_position) || _maze_runner_position == 288;
	case 32: // Maze runner exit allowed
		return _maze_runner_position != 288 && _maze_runner_position != 289;
	case 33: // Maze runner at entry
		return _maze_runner_position != 288;
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

void MystScriptParser_Selenitic::o_100_mazeRunnerMove(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 oldPosition = _maze_runner_position;
	uint16 move = var;

	uint16 videoToNext = _mazeRunnerVideos[_maze_runner_position][move];
	_maze_runner_position = _mazeRunnerMap[_maze_runner_position][move];

	if (videoToNext) {
		_maze_runner_compass->drawConditionalDataToScreen(8);

		mazeRunnerPlayVideo(videoToNext, oldPosition);
		mazeRunnerUpdateCompass();

		if (move == 0 || move == 3)
			mazeRunnerPlaySoundHelp();
	}
}

void MystScriptParser_Selenitic::mazeRunnerPlayVideo(uint16 video, uint16 pos) {
	Common::String file;

	switch (video) {
	case 1:
		file = _vm->wrapMovieFilename("forwa1", kSeleniticStack);
		break;
	case 2:
		file = _vm->wrapMovieFilename("forwe0", kSeleniticStack);
		break;
	case 3:
		if (mazeRunnerForwardAllowed(_maze_runner_position)) {
			file = _vm->wrapMovieFilename("forwf1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("forwf0", kSeleniticStack);
		}
		break;
	case 4:
		file = _vm->wrapMovieFilename("left00", kSeleniticStack);
		break;
	case 5:
		file = _vm->wrapMovieFilename("left01", kSeleniticStack);
		break;
	case 6:
		file = _vm->wrapMovieFilename("left10", kSeleniticStack);
		break;
	case 7:
		file = _vm->wrapMovieFilename("left11", kSeleniticStack);
		break;
	case 8:
		file = _vm->wrapMovieFilename("right00", kSeleniticStack);
		break;
	case 9:
		file = _vm->wrapMovieFilename("right01", kSeleniticStack);
		break;
	case 10:
		file = _vm->wrapMovieFilename("right10", kSeleniticStack);
		break;
	case 11:
		file = _vm->wrapMovieFilename("right11", kSeleniticStack);
		break;
	case 12:
		if (mazeRunnerForwardAllowed(_maze_runner_position)) {
			file = _vm->wrapMovieFilename("forwo1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("forwo0", kSeleniticStack);
		}
		break;
	case 13:
		if (mazeRunnerForwardAllowed(_maze_runner_position)) {
			file = _vm->wrapMovieFilename("forwp1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("forwp0", kSeleniticStack);
		}
		break;
	case 14:
		if (mazeRunnerForwardAllowed(_maze_runner_position)) {
			file = _vm->wrapMovieFilename("forws1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("forws0", kSeleniticStack);
		}
		break;
	case 15:
		if (mazeRunnerForwardAllowed(_maze_runner_position)) {
			file = _vm->wrapMovieFilename("forwr1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("forwr0", kSeleniticStack);
		}
		break;
	case 16:
		if (mazeRunnerForwardAllowed(_maze_runner_position)) {
			file = _vm->wrapMovieFilename("forwl1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("forwl0", kSeleniticStack);
		}
		break;
	case 17:
		file = _vm->wrapMovieFilename("backa1", kSeleniticStack);
		break;
	case 18:
		file = _vm->wrapMovieFilename("backe1", kSeleniticStack);
		break;
	case 19:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backf1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backf0", kSeleniticStack);
		}
		break;
	case 20:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backo1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backo0", kSeleniticStack);
		}
		break;
	case 21:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backp1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backp0", kSeleniticStack);
		}
		break;
	case 22:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backs1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backs0", kSeleniticStack);
		}
		break;
	case 23:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backr1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backr0", kSeleniticStack);
		}
		break;
	case 24:
		if (mazeRunnerForwardAllowed(pos)) {
			file = _vm->wrapMovieFilename("backl1", kSeleniticStack);
		} else {
			file = _vm->wrapMovieFilename("backl0", kSeleniticStack);
		}
		break;
	}

	if (file != "") {
		const Common::Rect &dest = _maze_runner_window->getRect();
		_vm->_video->playMovie(file, dest.left, dest.top, false);
	}
}

void MystScriptParser_Selenitic::mazeRunnerUpdateCompass() {
	if (_maze_runner_position == 288 || _maze_runner_position == 289) {
		_maze_runner_direction = 8;
	} else {
		_maze_runner_direction = _maze_runner_position % 8;
	}
	_maze_runner_compass->drawConditionalDataToScreen(_maze_runner_direction);
}

bool MystScriptParser_Selenitic::mazeRunnerForwardAllowed(uint16 position) {
	uint16 move = _mazeRunnerVideos[position][1];
	return move == 6 || move == 7;
}

void MystScriptParser_Selenitic::mazeRunnerPlaySoundHelp() {
	uint16 soundId = 0;

	_maze_runner_light->drawConditionalDataToScreen(1);

	if (_maze_runner_position >= 272)
		soundId = 0;
	else if (_maze_runner_position >= 264)
		soundId =  8191;
	else if (_maze_runner_position >= 256)
		soundId = 0;
	else if (_maze_runner_position >= 248)
		soundId =  5191;
	else if (_maze_runner_position >= 232)
		soundId = 0;
	else if (_maze_runner_position >= 224)
		soundId =  5191;
	else if (_maze_runner_position >= 216)
		soundId = 0;
	else if (_maze_runner_position >= 208)
		soundId =  5191;
	else if (_maze_runner_position >= 176)
		soundId = 0;
	else if (_maze_runner_position >= 168)
		soundId =  7191;
	else if (_maze_runner_position >= 152)
		soundId = 0;
	else if (_maze_runner_position >= 144)
		soundId =  7191;
	else if (_maze_runner_position >= 136)
		soundId =  2191;
	else if (_maze_runner_position >= 112)
		soundId = 0;
	else if (_maze_runner_position >= 104)
		soundId =  6191;
	else if (_maze_runner_position >= 96)
		soundId =  2191;
	else if (_maze_runner_position >= 88)
		soundId =  3191;
	else if (_maze_runner_position >= 80)
		soundId =  4191;
	else if (_maze_runner_position >= 72)
		soundId =  8191;
	else if (_maze_runner_position >= 64)
		soundId =  7191;
	else if (_maze_runner_position >= 56)
		soundId =  8191;
	else if (_maze_runner_position >= 40)
		soundId =  5191;
	else if (_maze_runner_position >= 24)
		soundId =  1191;
	else if (_maze_runner_position >= 16)
		soundId =  2191;
	else if (_maze_runner_position >= 8)
		soundId =  8191;
	else
		soundId =  2191;

	if (soundId)
		_vm->_sound->playSound(soundId);

	_maze_runner_light->drawConditionalDataToScreen(0);
}

void MystScriptParser_Selenitic::o_101_mazeRunnerSoundRepeat(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	mazeRunnerPlaySoundHelp();
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

void MystScriptParser_Selenitic::o_110_mazeRunnerDoorButton(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Selenitic Maze Runner Exit Logic
	uint16 cardIdExit = argv[0];
	uint16 cardIdEntry = argv[1];

	if (_maze_runner_position == 288) {
		_vm->changeToCard(cardIdEntry, false);
		_vm->_sound->playSound(cardIdEntry);
		animatedUpdate(argv[2], &argv[3], 10);
	} else if (_maze_runner_position == 289) {
		_vm->changeToCard(cardIdExit, false);
		_vm->_sound->playSound(cardIdExit);
		animatedUpdate(argv[2], &argv[3], 10);
	}
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

void MystScriptParser_Selenitic::o_200_mazeRunnerCompass_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_maze_runner_compass = static_cast<MystResourceType8 *>(_invokingResource);
}

void MystScriptParser_Selenitic::o_201_mazeRunnerWindow_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_maze_runner_window = static_cast<MystResourceType8 *>(_invokingResource);
}

void MystScriptParser_Selenitic::o_202_mazeRunnerLight_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_maze_runner_light = static_cast<MystResourceType8 *>(_invokingResource);
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

void MystScriptParser_Selenitic::o_205_mazeRunnerRight_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_maze_runner_right_button = static_cast<MystResourceType8 *>(_invokingResource);
}

void MystScriptParser_Selenitic::o_206_mazeRunnerLeft_init(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_maze_runner_left_button = static_cast<MystResourceType8 *>(_invokingResource);
}

const uint16 MystScriptParser_Selenitic::_mazeRunnerMap[300][4] = {
		 {8, 7, 1, 288},
		 {1, 0, 2, 288},
		 {2, 1, 3, 288},
		 {3, 2, 4, 288},
		 {4, 3, 5, 288},
		 {5, 4, 6, 288},
		 {6, 5, 7, 288},
		 {7, 6, 0, 288},
		 {8, 15, 9, 0},
		 {9, 8, 10, 0},
		 {10, 9, 11, 0},
		 {11, 10, 12, 0},
		 {4, 11, 13, 0},
		 {13, 12, 14, 0},
		 {22, 13, 15, 0},
		 {15, 14, 8, 0},
		 {24, 23, 17, 14},
		 {17, 16, 18, 14},
		 {10, 17, 19, 14},
		 {19, 18, 20, 14},
		 {20, 19, 21, 14},
		 {21, 20, 22, 14},
		 {22, 21, 23, 14},
		 {23, 22, 16, 14},
		 {112, 31, 25, 16},
		 {25, 24, 26, 16},
		 {34, 25, 27, 16},
		 {27, 26, 28, 16},
		 {20, 27, 29, 16},
		 {29, 28, 30, 16},
		 {30, 29, 31, 16},
		 {31, 30, 24, 16},
		 {120, 39, 33, 26},
		 {33, 32, 34, 26},
		 {42, 33, 35, 26},
		 {35, 34, 36, 26},
		 {36, 35, 37, 26},
		 {37, 36, 38, 26},
		 {30, 37, 39, 26},
		 {39, 38, 32, 26},
		 {40, 47, 41, 34},
		 {143, 40, 42, 34},
		 {42, 41, 43, 34},
		 {43, 42, 44, 34},
		 {52, 43, 45, 34},
		 {45, 44, 46, 34},
		 {38, 45, 47, 34},
		 {47, 46, 40, 34},
		 {40, 55, 49, 44},
		 {49, 48, 50, 44},
		 {146, 49, 51, 44},
		 {51, 50, 52, 44},
		 {60, 51, 53, 44},
		 {53, 52, 54, 44},
		 {54, 53, 55, 44},
		 {55, 54, 48, 44},
		 {48, 63, 57, 52},
		 {57, 56, 58, 52},
		 {58, 57, 59, 52},
		 {171, 58, 60, 52},
		 {60, 59, 61, 52},
		 {61, 60, 62, 52},
		 {68, 61, 63, 52},
		 {63, 62, 56, 52},
		 {58, 71, 65, 62},
		 {65, 64, 66, 62},
		 {66, 65, 67, 62},
		 {67, 66, 68, 62},
		 {68, 67, 69, 62},
		 {77, 68, 70, 62},
		 {70, 69, 71, 62},
		 {71, 70, 64, 62},
		 {72, 79, 73, 69},
		 {65, 72, 74, 69},
		 {186, 73, 75, 69},
		 {75, 74, 76, 69},
		 {196, 75, 77, 69},
		 {77, 76, 78, 69},
		 {86, 77, 79, 69},
		 {79, 78, 72, 69},
		 {80, 87, 81, 78},
		 {81, 80, 82, 78},
		 {74, 81, 83, 78},
		 {83, 82, 84, 78},
		 {212, 83, 85, 78},
		 {229, 84, 86, 78},
		 {86, 85, 87, 78},
		 {95, 86, 80, 78},
		 {88, 95, 89, 87},
		 {97, 88, 90, 87},
		 {90, 89, 91, 87},
		 {83, 90, 92, 87},
		 {92, 91, 93, 87},
		 {93, 92, 94, 87},
		 {254, 93, 95, 87},
		 {95, 94, 88, 87},
		 {106, 103, 97, 89},
		 {97, 96, 98, 89},
		 {98, 97, 99, 89},
		 {99, 98, 100, 89},
		 {100, 99, 101, 89},
		 {93, 100, 102, 89},
		 {102, 101, 103, 89},
		 {271, 102, 96, 89},
		 {104, 111, 105, 96},
		 {105, 104, 106, 96},
		 {106, 105, 107, 96},
		 {289, 106, 108, 96},
		 {108, 107, 109, 96},
		 {109, 108, 110, 96},
		 {100, 109, 111, 96},
		 {111, 110, 104, 96},
		 {112, 119, 113, 24},
		 {113, 112, 114, 24},
		 {114, 113, 115, 24},
		 {115, 114, 116, 24},
		 {28, 115, 117, 24},
		 {117, 116, 118, 24},
		 {118, 117, 119, 24},
		 {119, 118, 112, 24},
		 {120, 127, 121, 32},
		 {121, 120, 122, 32},
		 {122, 121, 123, 32},
		 {123, 122, 124, 32},
		 {36, 123, 125, 32},
		 {125, 124, 126, 32},
		 {126, 125, 127, 32},
		 {127, 126, 120, 32},
		 {128, 135, 129, 136},
		 {129, 128, 130, 136},
		 {130, 129, 131, 136},
		 {131, 130, 132, 136},
		 {140, 131, 133, 136},
		 {133, 132, 134, 136},
		 {134, 133, 135, 136},
		 {135, 134, 128, 136},
		 {128, 143, 137, 41},
		 {137, 136, 138, 41},
		 {138, 137, 139, 41},
		 {45, 138, 140, 41},
		 {140, 139, 141, 41},
		 {141, 140, 142, 41},
		 {142, 141, 143, 41},
		 {143, 142, 136, 41},
		 {144, 151, 145, 50},
		 {145, 144, 146, 50},
		 {146, 145, 147, 50},
		 {147, 146, 148, 50},
		 {148, 147, 149, 50},
		 {157, 148, 150, 50},
		 {54, 149, 151, 50},
		 {151, 150, 144, 50},
		 {152, 159, 153, 149},
		 {145, 152, 154, 149},
		 {154, 153, 155, 149},
		 {155, 154, 156, 149},
		 {156, 155, 157, 149},
		 {157, 156, 158, 149},
		 {158, 157, 159, 149},
		 {159, 158, 152, 149},
		 {160, 167, 161, 168},
		 {161, 160, 162, 168},
		 {162, 161, 163, 168},
		 {163, 162, 164, 168},
		 {172, 163, 165, 168},
		 {165, 164, 166, 168},
		 {166, 165, 167, 168},
		 {167, 166, 160, 168},
		 {160, 175, 169, 59},
		 {169, 168, 170, 59},
		 {170, 169, 171, 59},
		 {171, 170, 172, 59},
		 {172, 171, 173, 59},
		 {181, 172, 174, 59},
		 {174, 173, 175, 59},
		 {63, 174, 168, 59},
		 {176, 183, 177, 173},
		 {169, 176, 178, 173},
		 {178, 177, 179, 173},
		 {179, 178, 180, 173},
		 {180, 179, 181, 173},
		 {181, 180, 182, 173},
		 {182, 181, 183, 173},
		 {183, 182, 176, 173},
		 {184, 191, 185, 74},
		 {185, 184, 186, 74},
		 {186, 185, 187, 74},
		 {187, 186, 188, 74},
		 {188, 187, 189, 74},
		 {189, 188, 190, 74},
		 {78, 189, 191, 74},
		 {191, 190, 184, 74},
		 {72, 199, 193, 76},
		 {193, 192, 194, 76},
		 {194, 193, 195, 76},
		 {195, 194, 196, 76},
		 {196, 195, 197, 76},
		 {197, 196, 198, 76},
		 {198, 197, 199, 76},
		 {199, 198, 192, 76},
		 {200, 207, 201, 212},
		 {201, 200, 202, 212},
		 {202, 201, 203, 212},
		 {203, 202, 204, 212},
		 {204, 203, 205, 212},
		 {205, 204, 206, 212},
		 {208, 205, 207, 212},
		 {207, 206, 200, 212},
		 {80, 215, 209, 84},
		 {209, 208, 210, 84},
		 {210, 209, 211, 84},
		 {211, 210, 212, 84},
		 {202, 211, 213, 84},
		 {213, 212, 214, 84},
		 {208, 213, 215, 84},
		 {215, 214, 208, 84},
		 {224, 223, 217, 228},
		 {217, 216, 218, 228},
		 {218, 217, 219, 228},
		 {219, 218, 220, 228},
		 {220, 219, 221, 228},
		 {221, 220, 222, 228},
		 {216, 221, 223, 228},
		 {223, 222, 216, 228},
		 {224, 231, 225, 85},
		 {81, 224, 226, 85},
		 {226, 225, 227, 85},
		 {227, 226, 228, 85},
		 {220, 227, 229, 85},
		 {229, 228, 230, 85},
		 {238, 229, 231, 85},
		 {231, 230, 224, 85},
		 {232, 239, 233, 230},
		 {233, 232, 234, 230},
		 {226, 233, 235, 230},
		 {235, 234, 236, 230},
		 {236, 235, 237, 230},
		 {237, 236, 238, 230},
		 {232, 237, 239, 230},
		 {239, 238, 232, 230},
		 {240, 247, 241, 252},
		 {241, 240, 242, 252},
		 {248, 241, 243, 252},
		 {243, 242, 244, 252},
		 {244, 243, 245, 252},
		 {245, 244, 246, 252},
		 {240, 245, 247, 252},
		 {247, 246, 240, 252},
		 {256, 255, 249, 94},
		 {249, 248, 250, 94},
		 {90, 249, 251, 94},
		 {251, 250, 252, 94},
		 {246, 251, 253, 94},
		 {253, 252, 254, 94},
		 {248, 253, 255, 94},
		 {255, 254, 248, 94},
		 {256, 263, 257, 248},
		 {257, 256, 258, 248},
		 {258, 257, 259, 248},
		 {259, 258, 260, 248},
		 {252, 259, 261, 248},
		 {261, 260, 262, 248},
		 {256, 261, 263, 248},
		 {263, 262, 256, 248},
		 {280, 271, 265, 103},
		 {265, 264, 266, 103},
		 {266, 265, 267, 103},
		 {99, 266, 268, 103},
		 {268, 267, 269, 103},
		 {269, 268, 270, 103},
		 {278, 269, 271, 103},
		 {271, 270, 264, 103},
		 {272, 279, 273, 270},
		 {273, 272, 274, 270},
		 {266, 273, 275, 270},
		 {275, 274, 276, 270},
		 {276, 275, 277, 270},
		 {277, 276, 278, 270},
		 {272, 277, 279, 270},
		 {279, 278, 272, 270},
		 {280, 287, 281, 264},
		 {281, 280, 282, 264},
		 {282, 281, 283, 264},
		 {283, 282, 284, 264},
		 {268, 283, 285, 264},
		 {285, 284, 286, 264},
		 {280, 285, 287, 264},
		 {287, 286, 280, 264},
		 {0, 288, 288, 288},
		 {289, 289, 289, 107},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0},
		 { 0, 0, 0, 0}
	};

const uint8 MystScriptParser_Selenitic::_mazeRunnerVideos[300][4] = {
		{3, 6, 10, 17},
		{0, 5, 8, 17},
		{0, 4, 8, 17},
		{0, 4, 8, 17},
		{0, 4, 8, 17},
		{0, 4, 8, 17},
		{0, 4, 8, 17},
		{0, 4, 9, 17},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{12, 6, 10, 19},
		{0, 5, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{14, 6, 10, 20},
		{0, 5, 9, 20},
		{13, 6, 10, 20},
		{0, 5, 9, 20},
		{12, 6, 10, 20},
		{0, 5, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 9, 20},
		{12, 6, 10, 21},
		{0, 5, 9, 21},
		{3, 6, 10, 21},
		{0, 5, 8, 21},
		{12, 4, 8, 21},
		{0, 4, 9, 21},
		{13, 6, 10, 21},
		{0, 5, 9, 21},
		{0, 4, 9, 19},
		{16, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 9, 19},
		{12, 6, 10, 19},
		{0, 5, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{3, 6, 10, 20},
		{0, 5, 9, 20},
		{13, 6, 10, 20},
		{0, 5, 9, 20},
		{14, 6, 10, 20},
		{0, 5, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 9, 20},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 9, 22},
		{3, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 9, 22},
		{16, 6, 10, 22},
		{0, 5, 9, 22},
		{15, 6, 10, 24},
		{0, 5, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 9, 24},
		{14, 6, 10, 24},
		{0, 5, 8, 24},
		{0, 4, 9, 24},
		{0, 4, 9, 22},
		{14, 6, 11, 22},
		{12, 7, 10, 22},
		{0, 5, 9, 22},
		{13, 6, 10, 22},
		{0, 5, 9, 22},
		{3, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 5, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 9, 19},
		{14, 6, 11, 19},
		{3, 7, 10, 19},
		{0, 5, 9, 19},
		{14, 6, 10, 19},
		{0, 4, 9, 22},
		{3, 6, 10, 22},
		{0, 5, 9, 22},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 9, 22},
		{3, 6, 10, 22},
		{0, 5, 8, 22},
		{15, 7, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 9, 19},
		{14, 6, 11, 19},
		{0, 4, 8, 23},
		{0, 4, 8, 23},
		{0, 4, 9, 23},
		{2, 6, 10, 23},
		{0, 5, 8, 23},
		{0, 4, 9, 23},
		{16, 6, 10, 23},
		{0, 5, 8, 23},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 9, 22},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 9, 20},
		{12, 6, 10, 20},
		{0, 5, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{3, 6, 10, 24},
		{0, 5, 8, 24},
		{0, 4, 9, 24},
		{15, 6, 10, 24},
		{0, 5, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 9, 24},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 9, 21},
		{3, 6, 11, 21},
		{13, 7, 10, 21},
		{0, 5, 8, 21},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 9, 22},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{14, 7, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 9, 19},
		{3, 6, 11, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 9, 20},
		{12, 6, 10, 20},
		{0, 5, 8, 20},
		{13, 6, 10, 21},
		{0, 5, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 9, 21},
		{0, 4, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 8, 24},
		{0, 4, 9, 24},
		{15, 6, 10, 24},
		{0, 5, 8, 24},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 9, 22},
		{16, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 9, 22},
		{12, 6, 10, 20},
		{0, 5, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 8, 20},
		{0, 4, 9, 20},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 9, 19},
		{12, 6, 10, 19},
		{0, 5, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 23},
		{0, 4, 9, 23},
		{16, 6, 10, 23},
		{0, 5, 8, 23},
		{0, 4, 8, 23},
		{0, 4, 8, 23},
		{0, 4, 8, 23},
		{0, 4, 8, 23},
		{14, 6, 10, 19},
		{0, 5, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 9, 19},
		{15, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 9, 22},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 8, 22},
		{0, 4, 8, 22},
		{3, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 9, 22},
		{14, 6, 10, 22},
		{0, 5, 8, 22},
		{0, 4, 9, 22},
		{13, 6, 10, 22},
		{0, 5, 9, 22},
		{0, 4, 8, 21},
		{0, 4, 9, 21},
		{12, 6, 10, 21},
		{0, 5, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 21},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 9, 19},
		{3, 6, 10, 19},
		{0, 5, 8, 19},
		{0, 4, 8, 19},
		{0, 4, 8, 19},
		{1, 0, 0, 0},
		{0, 0, 0, 18},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};
} // End of namespace Mohawk
