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
struct MystScriptEntry;

class MystScriptParser_Selenitic : public MystScriptParser {
public:
	MystScriptParser_Selenitic(MohawkEngine_Myst *vm);
	~MystScriptParser_Selenitic();

	void disableInitOpcodes();
	void runPersistentOpcodes();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	void opcode_203_run();
	void opcode_203_disable();

	DECLARE_OPCODE(opcode_100);
	DECLARE_OPCODE(opcode_101);
	DECLARE_OPCODE(opcode_102);
	DECLARE_OPCODE(opcode_103);
	DECLARE_OPCODE(opcode_104);
	DECLARE_OPCODE(opcode_105_109);
	DECLARE_OPCODE(opcode_110);
	DECLARE_OPCODE(opcode_111);
	DECLARE_OPCODE(opcode_115);
	DECLARE_OPCODE(opcode_117);

	DECLARE_OPCODE(opcode_200);
	DECLARE_OPCODE(opcode_201);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(opcode_203);
	DECLARE_OPCODE(opcode_204);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);

	bool _sound_receiver_sigma_pressed; // 6
	MystResource *_sound_receiver_sources[5]; // 92 -> 108
	MystResource *_sound_receiver_current_source; // 112
	uint16 *_sound_receiver_position; // 116
	uint16 _sound_receiver_direction; // 120
	uint16 _sound_receiver_speed; // 122
	uint32 _sound_receiver_start_time; //124
	MystResourceType8 *_sound_receiver_viewer; // 128
	MystResource *_sound_receiver_right_button; // 132
	MystResource *_sound_receiver_left_button; // 136
	MystResource *_sound_receiver_angle_1; // 140
	MystResource *_sound_receiver_angle_2; // 144
	MystResource *_sound_receiver_angle_3; // 148
	MystResource *_sound_receiver_angle_4; // 152
	MystResource *_sound_receiver_sigma_button; // 156

	void sound_receiver_left_right(uint direction);
	void sound_receiver_update();
	void sound_receiver_draw_view();
	void sound_receiver_draw_angle();
	void sound_receiver_increase_speed();
	void sound_receiver_update_sound();
	uint16 sound_receiver_current_sound(uint16 source, uint16 position);
	void sound_receiver_solution(uint16 source, uint16 &solution, bool &enabled);

	void draw_digit(MystResource *_resource);
};

}

#undef DECLARE_OPCODE

#endif
