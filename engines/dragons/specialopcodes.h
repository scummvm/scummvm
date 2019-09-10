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
 */

#ifndef DRAGONS_SPECIALOPCODES_H
#define DRAGONS_SPECIALOPCODES_H

#include "common/func.h"
#include "common/str.h"


namespace Dragons {

#define DRAGONS_NUM_SPECIAL_OPCODES 0x8c

class DragonsEngine;

typedef Common::Functor0<void> SpecialOpcode;

class SpecialOpcodes {
public:
	SpecialOpcodes(DragonsEngine *vm);
	~SpecialOpcodes();
	void run(int16 opcode);

	struct {
		void *tbl;
		uint16 counter;
		int16 iniID;
		int16 sequenceID;
		uint32 curSequence;
		uint32 curSequenceIndex;
		uint32 numTotalSequences;
		uint32 sequenceDuration;
		uint16 numSteps[8];
		uint16 iniIDTbl[8][5];
		uint16 sequenceIDTbl[8][5];
		uint32 textTbl[8][5];
	} sceneUpdater;

protected:
	DragonsEngine *_vm;
	SpecialOpcode *_opcodes[DRAGONS_NUM_SPECIAL_OPCODES];
	Common::String _opcodeNames[DRAGONS_NUM_SPECIAL_OPCODES];


	void initOpcodes();
	void freeOpcodes();

	// Opcodes
	void spcCatapultMiniGame(); // 1
	void spcClearEngineFlag10(); // 3
	void spcSetEngineFlag10();   // 4

	void spcCastleGardenLogic(); // 8
	void spcUnk9();
	void spcUnkA();

	void spcUnkC();
	void spcFadeScreen(); // 0xd

	void spc11ShakeScreen(); //0x11
	void spcHandleInventionBookTransition(); // 0x12
	void spcUnk13InventionBookCloseRelated(); //0x13
	void spcClearEngineFlag8(); // 0x14
	void spcSetEngineFlag8(); // 0x15

	void spcKnightPoolReflectionLogic(); //0x17

	void spcActivatePizzaMakerActor(); // 0x1a
	void spcDeactivatePizzaMakerActor(); // 0x1b
	void spcPizzaMakerActorStopWorking(); // 0x1c

	void spcSetEngineFlag0x20000(); // 0x21
	void spcClearEngineFlag0x20000(); // 0x22
	void spcSetEngineFlag0x200000(); // 0x23
	void spcClearEngineFlag0x200000(); // 0x24

	void spcMonksAtBarSceneLogic(); //0x28
	void spcStopMonksAtBarSceneLogic(); //0x29

	void spcFlameBedroomEscapeSceneLogic(); // 0x2b
	void spcStopFlameBedroomEscapeSceneLogic(); // 0x2b

	void spcUnk34(); //0x34 pitchfork mole.

	void spcFlickerClearFlag0x80(); //0x36

	void spcNoop1(); // 0x38
	void spcTownAngryVillagersSceneLogic(); //0x39

	void spcSetEngineFlag0x2000000(); // 0x3b
	void spcClearEngineFlag0x2000000(); // 0x3c

	void spcZigmondFraudSceneLogic(); // 0x3e

	void spcZigmondFraudSceneLogic1(); // 0x40

	void spcDodoUnderAttackSceneLogic(); //0x42

	void spcBlackDragonOnHillSceneLogic(); //0x46

	void spcLoadScene1(); // 0x49

	void spcKnightsSavedCastleCutScene(); //0x4b

	void spcUnk4e();
	void spcUnk4f();
	void spcCloseInventory(); // 0x50
	void spcOpenInventionBook(); // 0x51
	void spcCloseInventionBook(); // 0x52
	void spcClearEngineFlag0x4000000(); // 0x53
	void spcSetEngineFlag0x4000000(); // 0x54
	void spcSetCursorSequenceIdToZero(); // 0x55

	void spcFlickerSetFlag0x80(); // 0x5b

	void spcUnk5d();
	void spcUnk5e();
	void spcUnk5f();

	void spcSetInventorySequenceTo5(); // 0x63
	void spcResetInventorySequence(); // 0x64
	void spcUnk65ScenePaletteRelated(); // 0x65;
	void spcUnk66();

	void spcTransitionToMap(); // 0x6b
	void spcTransitionFromMap(); // 0x6c

	void spcJesterInLibrarySceneLogic(); // 0x77

	void spcBlackDragonDialogForCamelhot(); // 0x7a
	void spcSetCameraXToZero(); //0x7b
	void spcDiamondIntroSequenceLogic(); //0x7c

	void spc82CallResetDataMaybe(); // 0x82

	void spcSetUnkFlag2(); // 0x89
	void spcClearUnkFlag2(); //0x8a

	void setupTableBasedSceneUpdateFunction(uint16 initialCounter, uint16 numSequences, uint16 sequenceDuration);
private:
	void panCamera(int16 mode);
	void pizzaMakerStopWorking();

	void clearSceneUpdateFunction();

};

// update functions

void pizzaUpdateFunction();
void tableBasedSceneUpdateFunction();

} // End of namespace Dragons

#endif // DRAGONS_SPECIALOPCODES_H
