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
 * $URL$
 * $Id$
 *
 */

#ifndef MYST_SCRIPTS_MECHANICAL_H
#define MYST_SCRIPTS_MECHANICAL_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Mechanical : public MystScriptParser {
public:
	Mechanical(MohawkEngine_Myst *vm);
	~Mechanical();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	void opcode_202_run();
	void opcode_202_disable();
	void elevatorRotation_run();
	void elevatorGoMiddle_run();
	void opcode_205_run();
	void opcode_205_disable();
	void opcode_206_run();
	void opcode_206_disable();
	void opcode_209_run();
	void opcode_209_disable();

	DECLARE_OPCODE(o_throneEnablePassage);
	DECLARE_OPCODE(o_snakeBoxTrigger);
	DECLARE_OPCODE(o_fortressStaircaseMovie);
	DECLARE_OPCODE(o_elevatorRotationStart);
	DECLARE_OPCODE(o_elevatorRotationMove);
	DECLARE_OPCODE(o_elevatorRotationStop);
	DECLARE_OPCODE(o_elevatorWindowMovie);
	DECLARE_OPCODE(o_elevatorGoMiddle);
	DECLARE_OPCODE(o_elevatorTopMovie);
	DECLARE_OPCODE(opcode_124);
	DECLARE_OPCODE(o_mystStaircaseMovie);
	DECLARE_OPCODE(opcode_126);
	DECLARE_OPCODE(o_crystalEnterYellow);
	DECLARE_OPCODE(o_crystalEnterGreen);
	DECLARE_OPCODE(o_crystalEnterRed);
	DECLARE_OPCODE(o_crystalLeaveYellow);
	DECLARE_OPCODE(o_crystalLeaveGreen);
	DECLARE_OPCODE(o_crystalLeaveRed);

	DECLARE_OPCODE(o_throne_init);
	DECLARE_OPCODE(o_fortressStaircase_init);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(o_snakeBox_init);
	DECLARE_OPCODE(o_elevatorRotation_init);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);
	DECLARE_OPCODE(opcode_209);

	MystGameState::Mechanical &_state;

	bool _mystStaircaseState; // 76

	uint16 _fortressPosition; // 82

	uint16 _elevatorGoingDown; // 112

	float _elevatorRotationSpeed; // 120
	float _elevatorRotationGearPosition; // 124
	uint16 _elevatorRotationSoundId; // 128
	bool _elevatorRotationLeverMoving; // 184

	bool _elevatorGoingMiddle; // 148
	bool _elevatorTooLate;
	uint16 _elevatorPosition; // 104
	bool _elevatorInCabin; // 108
	uint16 _elevatorTopCounter;
	uint32 _elevatorNextTime;

	uint16 _crystalLit; // 130

	MystResourceType6 *_snakeBox; // 156
};

} // End of namespace MystStacks
} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
