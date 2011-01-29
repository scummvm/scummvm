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

#ifndef MYST_SCRIPTS_STONESHIP_H
#define MYST_SCRIPTS_STONESHIP_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Stoneship : public MystScriptParser {
public:
	MystScriptParser_Stoneship(MohawkEngine_Myst *vm);
	~MystScriptParser_Stoneship();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	DECLARE_OPCODE(o_pumpTurnOff);
	DECLARE_OPCODE(o_brotherDoorOpen);
	DECLARE_OPCODE(o_cabinBookMovie);
	DECLARE_OPCODE(o_drawerOpenSirius);
	DECLARE_OPCODE(o_drawerClose);
	DECLARE_OPCODE(o_generatorStart);
	DECLARE_OPCODE(o_generatorStop);
	DECLARE_OPCODE(o_drawerOpenAchenar);
	DECLARE_OPCODE(o_hologramPlayback);
	DECLARE_OPCODE(o_hologramSelectionStart);
	DECLARE_OPCODE(o_hologramSelectionMove);
	DECLARE_OPCODE(o_hologramSelectionStop);
	DECLARE_OPCODE(o_compassButton);
	DECLARE_OPCODE(o_chestValveVideos);
	DECLARE_OPCODE(o_chestDropKey);
	DECLARE_OPCODE(o_trapLockOpen);
	DECLARE_OPCODE(o_sideDoorsMovies);
	DECLARE_OPCODE(o_drawerCloseOpened);

	DECLARE_OPCODE(o_hologramDisplay_init);
	DECLARE_OPCODE(o_hologramSelection_init);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(opcode_203);
	DECLARE_OPCODE(opcode_204);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);
	DECLARE_OPCODE(o_chest_init);
	DECLARE_OPCODE(opcode_208);
	DECLARE_OPCODE(o_achenarDrawers_init);
	DECLARE_OPCODE(opcode_210);

	DECLARE_OPCODE(opcode_300);

	void chargeBattery_run();

	MystGameState::Stoneship &_state;

	bool _batteryCharging;
	bool _batteryDepleting;
	uint32 _batteryNextTime;

	uint16 _cabinMystBookPresent; // 64

	uint16 _siriusDrawerDrugsOpen; // 72

	uint16 _brotherDoorOpen; // 76

	uint16 _chestDrawersOpen; // 78
	uint16 _chestAchenarBottomDrawerClosed; // 144
	void drawerClose(uint16 drawer);

	uint16 _hologramTurnedOn; // 80
	MystResourceType6 *_hologramDisplay; // 84
	MystResourceType6 *_hologramSelection; // 88
	uint16 _hologramDisplayPos;

	uint16 batteryRemainingCharge();
};

}

#undef DECLARE_OPCODE

#endif
