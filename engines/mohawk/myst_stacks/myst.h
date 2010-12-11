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

#ifndef MYST_SCRIPTS_MYST_H
#define MYST_SCRIPTS_MYST_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Myst : public MystScriptParser {
public:
	MystScriptParser_Myst(MohawkEngine_Myst *vm);
	~MystScriptParser_Myst();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	void opcode_201_run();
	void opcode_201_disable();
	void towerRotationMap_run();
	void opcode_205_run();
	void opcode_205_disable();
	void libraryBookcaseTransform_run();
	void generatorControlRoom_run();
	void opcode_212_run();
	void opcode_212_disable();
	void libraryCombinationBook_run();
	void clockWheel_run();


	DECLARE_OPCODE(o_libraryBookPageTurnLeft);
	DECLARE_OPCODE(o_libraryBookPageTurnRight);
	DECLARE_OPCODE(o_fireplaceToggleButton);
	DECLARE_OPCODE(o_fireplaceRotation);
	DECLARE_OPCODE(opcode_105);
	DECLARE_OPCODE(o_towerRotationStart);
	DECLARE_OPCODE(o_towerRotationEnd);
	DECLARE_OPCODE(opcode_109);
	DECLARE_OPCODE(o_dockVaultOpen);
	DECLARE_OPCODE(o_dockVaultClose);
	DECLARE_OPCODE(o_bookGivePage);
	DECLARE_OPCODE(o_clockWheelsExecute);
	DECLARE_OPCODE(opcode_117);
	DECLARE_OPCODE(opcode_118);
	DECLARE_OPCODE(opcode_119);
	DECLARE_OPCODE(o_generatorButtonPressed);
	DECLARE_OPCODE(opcode_121);
	DECLARE_OPCODE(opcode_122);
	DECLARE_OPCODE(opcode_123);
	DECLARE_OPCODE(opcode_129);
	DECLARE_OPCODE(opcode_130);
	DECLARE_OPCODE(opcode_131);
	DECLARE_OPCODE(opcode_132);
	DECLARE_OPCODE(opcode_133);
	DECLARE_OPCODE(opcode_134);
	DECLARE_OPCODE(opcode_135);
	DECLARE_OPCODE(opcode_136);
	DECLARE_OPCODE(opcode_137);
	DECLARE_OPCODE(o_circuitBreakerStartMove);
	DECLARE_OPCODE(o_circuitBreakerMove);
	DECLARE_OPCODE(o_circuitBreakerEndMove);
	DECLARE_OPCODE(opcode_146);
	DECLARE_OPCODE(opcode_147);
	DECLARE_OPCODE(opcode_149);
	DECLARE_OPCODE(opcode_150);
	DECLARE_OPCODE(opcode_151);
	DECLARE_OPCODE(o_rocketSoundSliderStartMove);
	DECLARE_OPCODE(o_rocketSoundSliderMove);
	DECLARE_OPCODE(o_rocketSoundSliderEndMove);
	DECLARE_OPCODE(o_rocketLeverStartMove);
	DECLARE_OPCODE(o_rocketOpenBook);
	DECLARE_OPCODE(o_rocketLeverMove);
	DECLARE_OPCODE(o_rocketLeverEndMove);
	DECLARE_OPCODE(opcode_169);
	DECLARE_OPCODE(opcode_170);
	DECLARE_OPCODE(opcode_171);
	DECLARE_OPCODE(opcode_172);
	DECLARE_OPCODE(opcode_173);
	DECLARE_OPCODE(opcode_174);
	DECLARE_OPCODE(opcode_175);
	DECLARE_OPCODE(opcode_176);
	DECLARE_OPCODE(opcode_177);
	DECLARE_OPCODE(o_libraryCombinationBookStop);
	DECLARE_OPCODE(opcode_181);
	DECLARE_OPCODE(opcode_182);
	DECLARE_OPCODE(opcode_183);
	DECLARE_OPCODE(opcode_184);
	DECLARE_OPCODE(opcode_185);
	DECLARE_OPCODE(o_clockMinuteWheelStartTurn);
	DECLARE_OPCODE(o_clockWheelEndTurn);
	DECLARE_OPCODE(o_clockHourWheelStartTurn);
	DECLARE_OPCODE(o_libraryCombinationBookStartRight);
	DECLARE_OPCODE(o_libraryCombinationBookStartLeft);
	DECLARE_OPCODE(opcode_192);
	DECLARE_OPCODE(opcode_194);
	DECLARE_OPCODE(opcode_195);
	DECLARE_OPCODE(opcode_196);
	DECLARE_OPCODE(opcode_197);
	DECLARE_OPCODE(o_dockVaultForceClose);
	DECLARE_OPCODE(opcode_199);

	DECLARE_OPCODE(o_libraryBook_init);
	DECLARE_OPCODE(opcode_201);
	DECLARE_OPCODE(o_towerRotationMap_init);
	DECLARE_OPCODE(o_forechamberDoor_init);
	DECLARE_OPCODE(opcode_204);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);
	DECLARE_OPCODE(opcode_208);
	DECLARE_OPCODE(o_libraryBookcaseTransform_init);
	DECLARE_OPCODE(o_generatorControlRoom_init);
	DECLARE_OPCODE(o_fireplace_init);
	DECLARE_OPCODE(opcode_212);
	DECLARE_OPCODE(opcode_213);
	DECLARE_OPCODE(opcode_214);
	DECLARE_OPCODE(opcode_215);
	DECLARE_OPCODE(opcode_216);
	DECLARE_OPCODE(opcode_217);
	DECLARE_OPCODE(opcode_218);
	DECLARE_OPCODE(o_rocketSliders_init);
	DECLARE_OPCODE(o_rocketLinkVideo_init);
	DECLARE_OPCODE(o_greenBook_init);
	DECLARE_OPCODE(opcode_222);

	DECLARE_OPCODE(opcode_300);
	DECLARE_OPCODE(opcode_301);
	DECLARE_OPCODE(opcode_302);
	DECLARE_OPCODE(opcode_303);
	DECLARE_OPCODE(opcode_304);
	DECLARE_OPCODE(opcode_305);
	DECLARE_OPCODE(opcode_306);
	DECLARE_OPCODE(opcode_307);
	DECLARE_OPCODE(opcode_308);
	DECLARE_OPCODE(opcode_309);
	DECLARE_OPCODE(opcode_312);

	bool _generatorControlRoomRunning;
	uint16 _generatorVoltage; // 58

	MystResourceType10 *_rocketSlider1; // 248
	MystResourceType10 *_rocketSlider2; // 252
	MystResourceType10 *_rocketSlider3; // 256
	MystResourceType10 *_rocketSlider4; // 260
	MystResourceType10 *_rocketSlider5; // 264
	uint16 _rocketSliderSound; // 294
	uint16 _rocketLeverPosition; // 296

	bool _libraryCombinationBookPagesTurning;
	int16 _libraryBookPage; // 86
	uint16 _libraryBookNumPages; // 88
	uint16 _libraryBookBaseImage; // 90

	bool _libraryBookcaseMoving;
	MystResourceType6 *_libraryBookcaseMovie; // 104
	uint16 _libraryBookcaseSoundId; // 284
	bool _libraryBookcaseChanged; // 288
	uint16 _libraryBookSound1; // 298
	uint16 _libraryBookSound2; // 300

	uint16 _fireplaceLines[6]; // 74 to 84

	uint16 _clockTurningWheel;

	uint16 _dockVaultState; // 92

	bool _towerRotationMapRunning;
	uint16 _towerRotationMapInitialized; // 292
	MystResourceType11 *_towerRotationMapTower; // 108
	MystResourceType8 *_towerRotationMapLabel; // 112
	bool _towerRotationMapClicked; // 132

	void generatorRedrawRocket();
	void generatorButtonValue(MystResource *button, uint16 &offset, uint16 &value);

	void rocketSliderMove();
	uint16 rocketSliderGetSound(uint16 pos);
	void rocketCheckSolution();

	void libraryCombinationBookTurnRight();
	void libraryCombinationBookTurnLeft();

	uint16 bookCountPages(uint16 var);

	void clockWheelStartTurn(uint16 wheel);
	void clockWheelTurn(uint16 var);
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
