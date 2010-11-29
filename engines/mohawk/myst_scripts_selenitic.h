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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/mohawk/myst_scripts.h $
 * $Id: myst_scripts.h 47541 2010-01-25 01:39:44Z lordhoto $
 *
 */

#ifndef MYST_SCRIPTS_SELENITIC_H
#define MYST_SCRIPTS_SELENITIC_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
class MystResourceType8;
struct MystScriptEntry;

class MystScriptParser_Selenitic : public MystScriptParser {
public:
	MystScriptParser_Selenitic(MohawkEngine_Myst *vm);
	~MystScriptParser_Selenitic();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	DECLARE_OPCODE(opcode_100);
	DECLARE_OPCODE(opcode_101);
	DECLARE_OPCODE(o_102_soundReceiverSigma);
	DECLARE_OPCODE(o_103_soundReceiverRight);
	DECLARE_OPCODE(o_104_soundReceiverLeft);
	DECLARE_OPCODE(o_105_109_soundReceiverSource);
	DECLARE_OPCODE(o_110_mazeRunnerDoorButton);
	DECLARE_OPCODE(o_111_soundReceiverUpdateSound);
	DECLARE_OPCODE(o_112_soundLockMove);
	DECLARE_OPCODE(o_113_soundLockStartMove);
	DECLARE_OPCODE(o_114_soundLockEndMove);
	DECLARE_OPCODE(o_115_soundLockButton);
	DECLARE_OPCODE(o_117_soundReceiverEndMove);

	DECLARE_OPCODE(o_200_mazeRunnerCompass_init);
	DECLARE_OPCODE(o_201_mazeRunnerWindow_init);
	DECLARE_OPCODE(o_202_mazeRunnerLight_init);
	DECLARE_OPCODE(o_203_soundReceiver_init);
	DECLARE_OPCODE(o_204_soundLock_init);
	DECLARE_OPCODE(o_205_mazeRunnerRight_init);
	DECLARE_OPCODE(o_206_mazeRunnerLeft_init);

	void o_203_soundReceiver_run();

	bool _sound_receiver_running;
	bool _sound_receiver_sigma_pressed; // 6
	MystResourceType8 *_sound_receiver_sources[5]; // 92 -> 108
	MystResourceType8 *_sound_receiver_current_source; // 112
	uint16 *_sound_receiver_position; // 116
	uint16 _sound_receiver_direction; // 120
	uint16 _sound_receiver_speed; // 122
	uint32 _sound_receiver_start_time; //124
	MystResourceType8 *_sound_receiver_viewer; // 128
	MystResourceType8 *_sound_receiver_right_button; // 132
	MystResourceType8 *_sound_receiver_left_button; // 136
	MystResourceType8 *_sound_receiver_angle_1; // 140
	MystResourceType8 *_sound_receiver_angle_2; // 144
	MystResourceType8 *_sound_receiver_angle_3; // 148
	MystResourceType8 *_sound_receiver_angle_4; // 152
	MystResourceType8 *_sound_receiver_sigma_button; // 156

	uint16 _maze_runner_position; // 56
	MystResourceType8 *_maze_runner_window; // 68
	MystResourceType8 *_maze_runner_compass; // 72
	MystResourceType8 *_maze_runner_light; // 76
	MystResourceType8 *_maze_runner_right_button; // 80
	MystResourceType8 *_maze_runner_left_button; // 84
	bool _maze_runner_door_opened; // 160

	uint16 _sound_lock_sound_id;
	MystResourceType10 *_sound_lock_slider_1; // 164
	MystResourceType10 *_sound_lock_slider_2; // 168
	MystResourceType10 *_sound_lock_slider_3; // 172
	MystResourceType10 *_sound_lock_slider_4; // 176
	MystResourceType10 *_sound_lock_slider_5; // 180
	MystResourceType8 *_sound_lock_button; // 184

	void sound_receiver_left_right(uint direction);
	void sound_receiver_update();
	void sound_receiver_draw_view();
	void sound_receiver_draw_angle();
	void sound_receiver_increase_speed();
	void sound_receiver_update_sound();
	uint16 sound_receiver_current_sound(uint16 source, uint16 position);
	void sound_receiver_solution(uint16 source, uint16 &solution, bool &enabled);

	uint16 soundLockCurrentSound(uint16 position, bool pixels);
	MystResourceType10 *soundLockSliderFromVar(uint16 var);
	void soundLockCheckSolution(MystResourceType10 *slider, uint16 value, uint16 solution, bool &solved);

	void redrawResource(MystResource *_resource);
};

}

#undef DECLARE_OPCODE

#endif
