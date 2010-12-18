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

#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/stoneship.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptParser_Stoneship::MystScriptParser_Stoneship(MohawkEngine_Myst *vm) : MystScriptParser(vm) {
	setupOpcodes();
}

MystScriptParser_Stoneship::~MystScriptParser_Stoneship() {
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, (OpcodeProcMyst) &MystScriptParser_Stoneship::x, #x))

void MystScriptParser_Stoneship::setupOpcodes() {
	// "Stack-Specific" Opcodes
	OPCODE(100, opcode_100);
	OPCODE(101, opcode_101);
	OPCODE(102, opcode_102);
	OPCODE(103, opcode_103);
	OPCODE(104, opcode_104);
	OPCODE(111, opcode_111);
	OPCODE(112, opcode_112);
	OPCODE(116, opcode_116);
	OPCODE(117, opcode_117);
	OPCODE(118, opcode_118);
	OPCODE(119, opcode_119);
	OPCODE(120, opcode_120);
	OPCODE(125, opcode_125);

	// "Init" Opcodes
	OPCODE(200, opcode_200);
	OPCODE(201, opcode_201);
	OPCODE(202, opcode_202);
	OPCODE(203, opcode_203);
	OPCODE(204, opcode_204);
	OPCODE(205, opcode_205);
	OPCODE(206, opcode_206);
	OPCODE(207, opcode_207);
	OPCODE(208, opcode_208);
	OPCODE(209, opcode_209);
	OPCODE(210, opcode_210);

	// "Exit" Opcodes
	OPCODE(300, opcode_300);
}

#undef OPCODE

void MystScriptParser_Stoneship::disablePersistentScripts() {
	opcode_200_disable();
	opcode_201_disable();
	opcode_209_disable();
}

void MystScriptParser_Stoneship::runPersistentScripts() {
	opcode_200_run();
	opcode_201_run();
	opcode_209_run();
}

void MystScriptParser_Stoneship::opcode_100(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Cards 2185 (Water Pump)
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		// TODO: Called when Water Pump Button is pressed? Animation?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_101(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 6) {
		// Used by Door Buttons to Brothers' Rooms
		// Cards 2294, 2255
		Common::Rect u0_rect = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
		uint16 u1 = argv[3];
		uint16 u2 = argv[2];

		debugC(kDebugScript, "Opcode %d: Unknown", op);
		debugC(kDebugScript, "u0_rect.left: %d", u0_rect.left);
		debugC(kDebugScript, "u0_rect.top: %d", u0_rect.top);
		debugC(kDebugScript, "u0_rect.right: %d", u0_rect.right);
		debugC(kDebugScript, "u0_rect.bottom: %d", u0_rect.bottom);
		debugC(kDebugScript, "u1: %d", u1);
		debugC(kDebugScript, "u2: %d", u2);

		// TODO: Fill in logic...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_102(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Play Book Room Movie", op);

		uint16 startTime = argv[0];
		uint16 endTime = argv[1];

		debugC(kDebugScript, "\tstartTime: %d", startTime);
		debugC(kDebugScript, "\tendTime: %d", endTime);

		warning("TODO: Opcode %d Movie Time Index %d to %d", op, startTime, endTime);
		// TODO: Need version of playMovie blocking which allows selection
		//       of start and finish points.
		_vm->_video->playMovie(_vm->wrapMovieFilename("bkroom", kStoneshipStack), 159, 99);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_103(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Card 2197 (Sirrus' Room Drawers)
		debugC(kDebugScript, "Opcode %d: Unknown", op);

		uint16 u0 = argv[0];

		debugC(kDebugScript, "\tu0: %d", u0);
		// TODO: Fill in Logic...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_104(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 2004 (Achenar's Room Drawers)
	// Used for Closeup of Torn Note?
	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Fill in Function...
		// Does u0 correspond to a resource Id? Enable? Disable?
		// Similar to Opcode 111 (Stoneship Version).. But does this also
		// draw closeup image of note / change to closeup card?
	} else
		unknown(op, var, argc, argv);
}


void MystScriptParser_Stoneship::opcode_111(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used for Card 2004 (Achenar's Room Drawers)
		// Used by Drawers Hotspots...

		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Fill in Function...
		// Does u0 correspond to a resource Id? Enable? Disable?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_112(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: Rose-Skull Hologram Playback", op);

		uint16 varValue = _vm->_varStore->getVar(var);

		debugC(kDebugScript, "\tVar: %d = %d", var, varValue);

		uint16 startPoint = argv[0];
		uint16 endPoint = argv[1];
		uint16 u0 = argv[2];

		debugC(kDebugScript, "\tstartPoint: %d", startPoint);
		debugC(kDebugScript, "\tendPoint: %d", endPoint);
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Fill in Function...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_116(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Card 2111 (Compass Rose)
		// Called when Button Clicked.
		uint16 correctButton = argv[0];

		if (correctButton) {
			// Correct Button -> Light On Logic
			// TODO: Deal with if main power on?
			_vm->_varStore->setVar(16, 1);
			_vm->_varStore->setVar(30, 0);
		} else {
			// Wrong Button -> Power Failure Logic
			// TODO: Fill in Alarm
			_vm->_varStore->setVar(16, 0);
			_vm->_varStore->setVar(30, 2);
			_vm->_varStore->setVar(33, 0);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_117(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 2132 (Chest at Bottom of Lighthouse)
		// Called when Valve Hotspot Clicked.
		// TODO: Fill in Function to play right section of movie
		//       based on valve state and water in chest..
		_vm->_video->playMovie(_vm->wrapMovieFilename("ligspig", kStoneshipStack), 97, 267);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_118(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 0) {
		// Used on Card 2126 (Lighthouse Looking Along Plank)
		// Called when Exit Resource is clicked

		// TODO: Implement Function...
		// If holding Key to Lamp Room Trapdoor, drop to bottom of
		// Lighthouse...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_119(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Card 2143 (Lighthouse Trapdoor)
		// Called when Lock Hotspot Clicked while holding key.
		_vm->_video->playMovie(_vm->wrapMovieFilename("openloc", kStoneshipStack), 187, 72);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_120(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used for Cards 2285, 2289, 2247, 2251 (Side Doors in Tunnels Down To Brothers Rooms)
		uint16 movieId = argv[0];

		debugC(kDebugScript, "Opcode %d: Play Side Door Movies", op);
		debugC(kDebugScript, "\tmovieId: %d", movieId);

		switch (movieId) {
		case 0:
			// Card 2251
			_vm->_video->playMovie(_vm->wrapMovieFilename("tunaup", kStoneshipStack), 149, 161);
			break;
		case 1:
			// Card 2247
			_vm->_video->playMovie(_vm->wrapMovieFilename("tunadown", kStoneshipStack), 218, 150);
			break;
		case 2:
			// Card 2289
			_vm->_video->playMovie(_vm->wrapMovieFilename("tuncup", kStoneshipStack), 259, 161);
			break;
		case 3:
			// Card 2285
			_vm->_video->playMovie(_vm->wrapMovieFilename("tuncdown", kStoneshipStack), 166, 150);
			break;
		default:
			warning("Opcode 120 MovieId Out Of Range");
			break;
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_125(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		// Used on Card 2197 (Sirrus' Room Drawers)
		debugC(kDebugScript, "Opcode %d: Unknown uses Var %d", op, var);

		uint16 u0 = argv[0];

		debugC(kDebugScript, "\tu0: %d", u0);
		// TODO: Fill in Logic...
	} else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode200Parameters;

void MystScriptParser_Stoneship::opcode_200_run() {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)

	// TODO: Implement Function...
}

void MystScriptParser_Stoneship::opcode_200_disable() {
	g_opcode200Parameters.enabled = false;
}

void MystScriptParser_Stoneship::opcode_200(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	if (argc == 0)
		g_opcode200Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

static struct {
	bool enabled;
} g_opcode201Parameters;

void MystScriptParser_Stoneship::opcode_201_run() {
	// Used for Card 2013 (Achenar's Rose-Skull Hologram)

	// TODO: Fill in Function...
}

void MystScriptParser_Stoneship::opcode_201_disable() {
	g_opcode201Parameters.enabled = false;
}

void MystScriptParser_Stoneship::opcode_201(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2013 (Achenar's Rose-Skull Hologram)
	if (argc == 0)
		g_opcode201Parameters.enabled = true;
	else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_202(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2160 (Lighthouse Battery Pack Closeup)
	// TODO: Implement Code...
	// Not Sure of Purpose - Update of Light / Discharge?
	unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_203(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for all/most Cards in Tunnels Down To Brothers Rooms

	// TODO: Duplicate or similar function to Opcode 203?
	if (argc == 2 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: %d Arguments", op, argc);

		uint16 imageIdDarkDoorOpen = 0;
		uint16 imageIdDarkDoorClosed = 0;
		uint16 u0 = argv[0];
		uint16 soundIdAlarm = argv[argc - 1];

		if (argc == 4) {
			imageIdDarkDoorOpen = argv[1];
			imageIdDarkDoorClosed = argv[2];
		}

		debugC(kDebugScript, "\tu0: %d", u0);

		if (argc == 4) {
			debugC(kDebugScript, "\timageIdDarkDoorOpen: %d", imageIdDarkDoorOpen);
			debugC(kDebugScript, "\tsoundIdDarkDoorClosed: %d", imageIdDarkDoorClosed);
		}

		debugC(kDebugScript, "\tsoundIdAlarm: %d", soundIdAlarm);

		// TODO: Fill in Correct Function for Lights
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_204(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2160 (Lighthouse Battery Pack Closeup)
	if (argc == 0) {
		// TODO: Implement Code For Battery Meter Level
		// Overwrite _vm->_resources[1]->_subImages[0].rect.bottom 1 to 80
		// Add accessor functions for this...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_205(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Cards 2322, 2285 (Tunnels Down To Brothers Rooms)

	// TODO: Duplicate or similar function to Opcode 203?
	if (argc == 2 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: %d Arguments", op, argc);

		uint16 imageIdDoorOpen = 0;
		uint16 imageIdDoorClosed = 0;

		uint16 u0 = argv[0];
		if (argc == 4) {
			imageIdDoorOpen = argv[1];
			imageIdDoorClosed = argv[2];
		}

		uint16 soundIdAlarm = argv[argc - 1];

		debugC(kDebugScript, "\tu0: %d", u0);

		if (argc == 4) {
			debugC(kDebugScript, "\timageIdDoorOpen: %d", imageIdDoorOpen);
			debugC(kDebugScript, "\tsoundIdDoorClosed: %d", imageIdDoorClosed);
		}

		debugC(kDebugScript, "\tsoundIdAlarm: %d", soundIdAlarm);

		// TODO: Fill in Correct Function for Lights
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_206(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Cards 2272 and 2234 (Facing Out of Door)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown, %d Arguments", op, argc);
		// TODO: Function Unknown...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_207(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2138 (Lighthouse Key/Chest Animation Logic)
	// TODO: Fill in function
	warning("TODO: Opcode 207 Lighthouse Key/Chest Animation Logic");
}

void MystScriptParser_Stoneship::opcode_208(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used in Card 2218 (Telescope view)
	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: Telescope View", op);
		uint16 imagePanorama = argv[0];
		uint16 imageLighthouseOff = argv[1];
		uint16 imageLighthouseOn = argv[2];

		debugC(kDebugScript, "Image (Panorama): %d", imagePanorama);
		debugC(kDebugScript, "Image (Lighthouse Off): %d", imageLighthouseOff);
		debugC(kDebugScript, "Image (Lighthouse On): %d", imageLighthouseOn);

		// TODO: Fill in Logic.
	} else
		unknown(op, var, argc, argv);
}

static struct {
	uint16 u0[5];
	uint16 u1[5];
	uint16 stateVar;

	bool enabled;
} g_opcode209Parameters;

void MystScriptParser_Stoneship::opcode_209_run(void) {
	// Used for Card 2004 (Achenar's Room Drawers)

	// TODO: Implement Function...
	// Swap Open Drawers?
}

void MystScriptParser_Stoneship::opcode_209_disable(void) {
	g_opcode209Parameters.enabled = false;
}

void MystScriptParser_Stoneship::opcode_209(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used for Card 2004 (Achenar's Room Drawers)
	if (argc == 11) {
		g_opcode209Parameters.u0[0] = argv[0];
		g_opcode209Parameters.u0[1] = argv[1];
		g_opcode209Parameters.u0[2] = argv[2];
		g_opcode209Parameters.u0[3] = argv[3];
		g_opcode209Parameters.u0[4] = argv[4];

		g_opcode209Parameters.u1[0] = argv[5];
		g_opcode209Parameters.u1[1] = argv[6];
		g_opcode209Parameters.u1[2] = argv[7];
		g_opcode209Parameters.u1[3] = argv[8];
		g_opcode209Parameters.u1[4] = argv[9];

		g_opcode209Parameters.stateVar = argv[10];

		g_opcode209Parameters.enabled = true;
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_210(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used in Cards 2205 and 2207 (Cloud Orbs in Sirrus' Room)
	if (argc == 2) {
		uint16 soundId = argv[0];
		uint16 soundIdStopping = argv[1];

		// TODO: Work Out Function i.e. control Var etc.
		if (false) {
			_vm->_sound->replaceSound(soundId);
			_vm->_sound->replaceSound(soundIdStopping);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser_Stoneship::opcode_300(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used in Card 2218 (Telescope view)
	varUnusedCheck(op, var);
	// TODO: Fill in Logic. Clearing Variable for View?
}

} // End of namespace Mohawk
