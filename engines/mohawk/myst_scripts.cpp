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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/graphics.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "gui/message.h"

namespace Mohawk {

MystScriptEntry::MystScriptEntry() {
	type = kMystScriptNone;
	var = 0;
	argc = 0;
	argv = 0;
	u0 = 0;
	u1 = 0;
}

MystScriptEntry::~MystScriptEntry() {
	delete[] argv;
}

const uint8 MystScriptParser::stack_map[8] = {
	kSeleniticStack,
	kStoneshipStack,
	kMystStack,
	kMechanicalStack,
	kChannelwoodStack,
	0x0f,
	kDniStack,
	kMystStack
};

const uint16 MystScriptParser::start_card[8] = {
	1282,
	2029,
	4396,
	6122,
	3137,
	0,
	5038,
	4134
};

// NOTE: Credits Start Card is 10000

#define OPCODE(op, x) { op, &MystScriptParser::x, #x }

MystScriptParser::MystScriptParser(MohawkEngine_Myst *vm) : _vm(vm) {
	setupOpcodes();
	_invokingResource = NULL;
}

MystScriptParser::~MystScriptParser() {
}

void MystScriptParser::setupOpcodes() {
	// "invalid" opcodes do not exist or have not been observed
	// "unknown" opcodes exist, but their meaning is unknown

	static const MystOpcode myst_opcodes[] = {
		// "Standard" Opcodes
		OPCODE(0, o_0_toggleVar),
		OPCODE(1, o_1_setVar),
		OPCODE(2, o_2_changeCardSwitch),
		OPCODE(3, takePage),
		OPCODE(4, opcode_4),
		// Opcode 5 Not Present
		OPCODE(6, o_6_changeCard),
		OPCODE(7, o_6_changeCard),
		OPCODE(8, o_6_changeCard),
		OPCODE(9, o_9_triggerMovie),
		OPCODE(10, o_10_toggleVarNoRedraw),
		// Opcode 11 Not Present
		OPCODE(12, o_2_changeCardSwitch),
		OPCODE(13, o_2_changeCardSwitch),
		OPCODE(14, o_14_drawAreaState),
		OPCODE(15, o_15_redrawAreaForVar),
		OPCODE(16, opcode_16),
		OPCODE(17, o_17_changeCardPush),
		OPCODE(18, o_18_changeCardPop),
		OPCODE(19, o_19_enableAreas),
		OPCODE(20, o_20_disableAreas),
		OPCODE(21, opcode_21),
		OPCODE(22, o_6_changeCard),
		OPCODE(23, o_23_toggleAreasActivation),
		OPCODE(24, playSound),
		// Opcode 25 Not Present, original calls replaceSound
		OPCODE(26, opcode_26),
		OPCODE(27, playSoundBlocking),
		OPCODE(28, opcode_28),
		OPCODE(29, opcode_29_33),
		OPCODE(30, opcode_30),
		OPCODE(31, opcode_31),
		OPCODE(32, opcode_32),
		OPCODE(33, opcode_29_33),
		OPCODE(34, opcode_34),
		OPCODE(35, opcode_35),
		OPCODE(36, changeCursor),
		OPCODE(37, hideCursor),
		OPCODE(38, showCursor),
		OPCODE(39, opcode_39),
		OPCODE(40, changeStack),
		OPCODE(41, opcode_41),
		OPCODE(42, opcode_42),
		OPCODE(43, opcode_43),
		OPCODE(44, opcode_44),
		// Opcode 45 Not Present
		OPCODE(46, opcode_46),
		// Opcodes 47 to 99 Not Present

		OPCODE(0xFFFF, NOP)
	};

	_opcodes = myst_opcodes;
	_opcodeCount = ARRAYSIZE(myst_opcodes);
}

void MystScriptParser::runScript(MystScript script, MystResource *invokingResource) {
	_invokingResource = invokingResource;

	debugC(kDebugScript, "Script Size: %d", script->size());
	for (uint16 i = 0; i < script->size(); i++) {
		MystScriptEntry &entry = script->operator[](i);
		debugC(kDebugScript, "\tOpcode %d: %d", i, entry.opcode);
		runOpcode(entry.opcode, entry.var, entry.argc, entry.argv);
	}
}

void MystScriptParser::runOpcode(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	bool ranOpcode = false;

	for (uint16 i = 0; i < _opcodeCount; i++)
		if (_opcodes[i].op == op) {
			(this->*(_opcodes[i].proc)) (op, var, argc, argv);
			ranOpcode = true;
			break;
		}

	if (!ranOpcode)
		error ("Trying to run invalid opcode %d", op);
}

const char *MystScriptParser::getOpcodeDesc(uint16 op) {
	for (uint16 i = 0; i < _opcodeCount; i++)
		if (_opcodes[i].op == op)
			return _opcodes[i].desc;

	error("Unknown opcode %d", op);
	return "";
}

MystScript MystScriptParser::readScript(Common::SeekableReadStream *stream, MystScriptType type) {
	assert(stream);
	assert(type != kMystScriptNone);

	MystScript script = MystScript(new Common::Array<MystScriptEntry>());

	uint16 opcodeCount = stream->readUint16LE();
	script->resize(opcodeCount);

	for (uint16 i = 0; i < opcodeCount; i++) {
		MystScriptEntry &entry = script->operator[](i);
		entry.type = type;

		// u0 only exists in INIT and EXIT scripts
		if (type != kMystScriptNormal)
			entry.u0 = stream->readUint16LE();

		entry.opcode = stream->readUint16LE();
		entry.var = stream->readUint16LE();
		entry.argc = stream->readUint16LE();

		if (entry.argc > 0) {
			entry.argv = new uint16[entry.argc];
			for (uint16 j = 0; j < entry.argc; j++)
				entry.argv[j] = stream->readUint16LE();
		}

		// u1 exists only in EXIT scripts
		if (type == kMystScriptExit)
			entry.u1 = stream->readUint16LE();
	}

	return script;
}

uint16 MystScriptParser::getVar(uint16 var) {
	warning("Unimplemented var getter 0x%02x (%d)", var, var);
	return _vm->_varStore->getVar(var);
}

void MystScriptParser::toggleVar(uint16 var) {
	warning("Unimplemented var toggle 0x%02x (%d)", var, var);
	_vm->_varStore->setVar(var, (_vm->_varStore->getVar(var) + 1) % 2);
}

bool MystScriptParser::setVarValue(uint16 var, uint16 value) {
	warning("Unimplemented var setter 0x%02x (%d)", var, var);
	_vm->_varStore->setVar(var, value);
	return false;
}

// NOTE: Check to be used on Opcodes where var is thought
// not to be used. This emits a warning if var is nonzero.
// It is possible that the opcode does use var 0 in this case,
// but this will catch the majority of missed cases.
void MystScriptParser::varUnusedCheck(uint16 op, uint16 var) {
	if (var != 0)
		warning("Opcode %d: Unused Var %d", op, var);
}

void MystScriptParser::unknown(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	warning("Unimplemented opcode 0x%02x (%d)", op, op);
	warning("\tUses var %d", var);
	warning("\tArg count = %d", argc);
	if (argc) {
		Common::String str;
		str += Common::String::format("%d", argv[0]);
		for (uint16 i = 1; i < argc; i++) {
			str += Common::String::format(", %d", argv[i]);
		}
		warning("\tArgs: %s\n", str.c_str());
	}
}

void MystScriptParser::NOP(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// NOTE: Don't check argc/argv here as they vary depending on NOP erased opcode
	debugC(kDebugScript, "NOP");
}

void MystScriptParser::o_0_toggleVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	toggleVar(var);
	_vm->redrawArea(var);
}

void MystScriptParser::o_1_setVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (setVarValue(var, argv[0])) {
		_vm->redrawArea(var);
	}
}

void MystScriptParser::o_2_changeCardSwitch(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Opcodes 2, 12, and 13 are the same
	uint16 value = getVar(var);

	debugC(kDebugScript, "Opcode %d: changeCardSwitch var %d: %d", op, var, value);

	if (value)
		_vm->changeToCard(argv[value -1 ]);
	else if (_invokingResource != NULL)
		_vm->changeToCard(_invokingResource->getDest());
	else
		warning("Missing invokingResource in altDest call");
}

void MystScriptParser::takePage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc == 1) {
		uint16 cursorId = argv[0];

		debugC(kDebugScript, "Opcode %d: takePage Var %d CursorId %d", op, var, cursorId);

		if (getVar(var)) {
			_vm->setMainCursor(cursorId);

			setVarValue(var, 0);

			// Return pages that are already held
			if (var == 102)
				setVarValue(103, 1);

			if (var == 103)
				setVarValue(102, 1);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_4(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Exit Script of Mechanical Card 6044 (Fortress Rotation Simulator)

	if (argc == 0 && _vm->getCurStack() == kSeleniticStack && _vm->getCurCard() == 1275) {
		// TODO: Fixes Selenitic Card 1275, but not sure if this is correct for general case..
		// as it breaks Selenitic Card 1257, though this may be due to screen update. Also,
		// this may actually be a "Force Card Reload" or "VIEW conditional re-evaluation".. in
		// the general case, rather than this image blit...
		uint16 var_value = getVar(var);
		if (var_value < _vm->_view.scriptResCount) {
			if (_vm->_view.scriptResources[var_value].type == 1) { // TODO: Add Symbols for Types
				_vm->_gfx->copyImageToScreen(_vm->_view.scriptResources[var_value].id, Common::Rect(0, 0, 544, 333));
				_vm->_gfx->updateScreen();
			} else
				warning("Opcode %d: Script Resource %d Type Not Image", op, var_value);
		} else
			warning("Opcode %d: var %d value %d outside Script Resource Range %d", op, var, var_value, _vm->_view.scriptResCount);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_6_changeCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used for Selenitic Card 1286 Resource #0
		// Used for Myst Card 4143 Resource #0 & #5
		debugC(kDebugScript, "Opcode %d: Change To Dest of Invoking Resource", op);

		if (_invokingResource != NULL)
			_vm->changeToCard(_invokingResource->getDest());
		else
			warning("Opcode %d: Missing invokingResource", op);
	} else
		unknown(op, var, argc, argv);
}
void MystScriptParser::o_9_triggerMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Trigger Type 6 Resource Movie..", op);
	// If movie has sound, pause background music

	int16 direction = 1;
	if (argc == 1) {
		direction = argv[0];
	}
	debugC(kDebugScript, "\tDirection: %d", direction);

	// Trigger resource 6 movie overriding play direction

	// If movie has sound, resume background music
}

void MystScriptParser::o_10_toggleVarNoRedraw(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: toggleVarNoRedraw", op);

	toggleVar(var);
}

void MystScriptParser::o_14_drawAreaState(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: drawAreaState, state: %d", op, argv[0]);
	debugC(kDebugScript, "\tVar: %d", var);

	MystResourceType8 *parent = static_cast<MystResourceType8 *>(_invokingResource->_parent);
	parent->drawConditionalDataToScreen(argv[0]);
}

void MystScriptParser::o_15_redrawAreaForVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: dropPage", op);
	debugC(kDebugScript, "\tvar: %d", var);

	_vm->redrawArea(var);
}

void MystScriptParser::opcode_16(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used by Channelwood Card 3262 (In Elevator)
	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Change Card? Conditional?", op);

		uint16 cardId = argv[0];
		uint16 u0 = argv[1];

		debugC(kDebugScript, "\tcardId: %d", cardId);
		debugC(kDebugScript, "\tu0: %d", u0);

		// TODO: Finish Implementation...
		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

// NOTE: Opcode 17 and 18 form a pair, where Opcode 17 jumps to a card,
// but with the current cardId stored.
// Opcode 18 then "pops" this stored CardId and returns to that card.

// TODO: The purpose of the optional argv[1] on Opcode 17 and argv[0]
// on Opcode 18 which are always 4, 5 or 6 is unknown.

static uint16 opcode_17_18_cardId = 0;

void MystScriptParser::o_17_changeCardPush(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Jump to Card Id, Storing Current Card Id", op);

		uint16 cardId = argv[0];
		debugC(kDebugScript, "\tJump to CardId: %d", cardId);

		uint16 u0 = argv[1]; // TODO
		debugC(kDebugScript, "\tu0: %d", u0);

		opcode_17_18_cardId = _vm->getCurCard();

		debugC(kDebugScript, "\tCurrent CardId: %d", opcode_17_18_cardId);

		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_18_changeCardPop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Return To Stored Card Id", op);
		debugC(kDebugScript, "\tCardId: %d", opcode_17_18_cardId);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		_vm->changeToCard(opcode_17_18_cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_19_enableAreas(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc > 0) {
		debugC(kDebugScript, "Opcode %d: Enable Hotspots", op);

		uint16 count = argv[0];

		if (argc != count + 1)
			unknown(op, var, argc, argv);
		else {
			for (uint16 i = 0; i < count; i++) {
				debugC(kDebugScript, "Enable hotspot index %d", argv[i + 1]);
				_vm->setResourceEnabled(argv[i + 1], true);
			}
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_20_disableAreas(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc > 0) {
		debugC(kDebugScript, "Opcode %d: Disable Hotspots", op);

		uint16 count = argv[0];

		if (argc != count + 1)
			unknown(op, var, argc, argv);
		else {
			for (uint16 i = 0; i < count; i++) {
				debugC(kDebugScript, "Disable hotspot index %d", argv[i + 1]);
				if (argv[i + 1] == 0xFFFF) {
					if (_invokingResource != NULL)
						_invokingResource->setEnabled(false);
					else
						warning("Unknown Resource in disableHotspots script Opcode");
				} else
					_vm->setResourceEnabled(argv[i + 1], false);
			}
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_21(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 6) {
		// Used in Channelwood Card 3318 (Sirrus' Room Drawer)
		debugC(kDebugScript, "Opcode %d: Vertical Slide?", op);

		Common::Rect rect1 = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
		uint16 u0 = argv[4];
		uint16 u1 = argv[5];

		debugC(kDebugScript, "\trect1.left: %d", rect1.left);
		debugC(kDebugScript, "\trect1.top: %d", rect1.top);
		debugC(kDebugScript, "\trect1.right: %d", rect1.right);
		debugC(kDebugScript, "\trect1.bottom: %d", rect1.bottom);

		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);

		// TODO: Complete Implementation...
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_23_toggleAreasActivation(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	if (argc > 0) {
		debugC(kDebugScript, "Opcode %d: Toggle areas activation", op);

		uint16 count = argv[0];

		if (argc != count + 1)
			unknown(op, var, argc, argv);
		else {
			for (uint16 i = 0; i < count; i++) {
				debugC(kDebugScript, "Enable/Disable hotspot index %d", argv[i + 1]);

				MystResource *resource = 0;
				if (argv[i + 1] == 0xFFFF)
					resource = _invokingResource;
				else
					resource = _vm->_resources[argv[i + 1]];

				if (resource)
					resource->setEnabled(!resource->isEnabled());
			}
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::playSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: playSound", op);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_26(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);

		// TODO: Work out function...
		if (_vm->getCurStack() == kSeleniticStack && _vm->getCurCard() == 1245) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Exit Hotspot Resource");
		} else if (_vm->getCurStack() == kStoneshipStack && _vm->getCurCard() == 2226) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Ship Cabin Door");
		} else if (_vm->getCurStack() == kStoneshipStack && _vm->getCurCard() == 2294) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Sirrus' Room Door");
		} else if (_vm->getCurStack() == kMechanicalStack && _vm->getCurCard() == 6327) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Elevator");
		} else if (_vm->getCurStack() == kDniStack && _vm->getCurCard() == 5014) {
			debugC(kDebugScript, "TODO: Function Not Known... Used by Atrus");
		} else
			unknown(op, var, argc, argv);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::playSoundBlocking(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: playSoundBlocking", op);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		_vm->_sound->playSoundBlocking(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_28(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	Common::Rect rect;

	if (argc == 1 || argc == 4) {
		debugC(kDebugScript, "Opcode %d: Restore VIEW Default Image in Region", op);

		if (argc == 1) {
			// Used in Stoneship Card 2111 (Compass Rose)
			// Used in Mechanical Card 6267 (Code Lock)
			if (argv[0] == 0xFFFF) {
				rect = _invokingResource->getRect();
			} else
				unknown(op, var, argc, argv);
		} else if (argc == 4) {
			// Used in ... TODO: Fill in.
			rect = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
		} else
			warning("Opcode %d: argc Error", op);

		debugC(kDebugScript, "\trect.left: %d", rect.left);
		debugC(kDebugScript, "\trect.top: %d", rect.top);
		debugC(kDebugScript, "\trect.right: %d", rect.right);
		debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

		// TODO: Need to fix VIEW logic so this doesn't need
		//       calculation at this level.
		uint16 imageToDraw = 0;
		if (_vm->_view.conditionalImageCount == 0)
			imageToDraw = _vm->_view.mainImage;
		else {
			for (uint16 i = 0; i < _vm->_view.conditionalImageCount; i++)
				if (getVar(_vm->_view.conditionalImages[i].var) < _vm->_view.conditionalImages[i].numStates)
					imageToDraw = _vm->_view.conditionalImages[i].values[getVar(_vm->_view.conditionalImages[i].var)];
		}
		_vm->_gfx->copyImageSectionToScreen(imageToDraw, rect, rect);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_29_33(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Opcode 29 called on Mechanical Card 6178 causes a engine
	//       abort this is because imageId is 7158 (not valid), but the
	//       script resource gives this as 7178 (valid)...

	if (argc == 7) {
		uint16 imageId = argv[0];

		Common::Rect srcRect = Common::Rect(argv[1], argv[2], argv[3], argv[4]);

		Common::Rect dstRect = Common::Rect(argv[5], argv[6], 544, 333);

		if (dstRect.left == -1 || dstRect.top == -1) {
			// Interpreted as full screen
			dstRect.left = 0;
			dstRect.top = 0;
		}

		dstRect.right = dstRect.left + srcRect.width();
		dstRect.bottom = dstRect.top + srcRect.height();

		debugC(kDebugScript, "Opcode %d: Blit Image", op);
		debugC(kDebugScript, "\timageId: %d", imageId);
		debugC(kDebugScript, "\tsrcRect.left: %d", srcRect.left);
		debugC(kDebugScript, "\tsrcRect.top: %d", srcRect.top);
		debugC(kDebugScript, "\tsrcRect.right: %d", srcRect.right);
		debugC(kDebugScript, "\tsrcRect.bottom: %d", srcRect.bottom);
		debugC(kDebugScript, "\tdstRect.left: %d", dstRect.left);
		debugC(kDebugScript, "\tdstRect.top: %d", dstRect.top);
		debugC(kDebugScript, "\tdstRect.right: %d", dstRect.right);
		debugC(kDebugScript, "\tdstRect.bottom: %d", dstRect.bottom);

		_vm->_gfx->copyImageSectionToScreen(imageId, srcRect, dstRect);
	} else
		unknown(op, var, argc, argv);
}

// TODO: Implement common engine function for read and processing of sound blocks
//       for use by this opcode and VIEW sound block.
// TODO: Though the playSound and PlaySoundBlocking opcodes play sounds immediately,
//       this opcode changes the main background sound playing..
//       Current behaviour here and with VIEW sound block is not right as demonstrated
//       by Channelwood Card 3280 (Tank Valve) and water flow sound behaviour in pipe
//       on cards leading from shed...
void MystScriptParser::opcode_30(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	int16 *soundList = NULL;
	uint16 *soundListVolume = NULL;

	// Used on Stoneship Card 2080
	// Used on Channelwood Card 3225 with argc = 8 i.e. Conditional Sound List
	if (argc == 1 || argc == 2 || argc == 8) {
		debugC(kDebugScript, "Opcode %d: Process Sound Block", op);
		uint16 decodeIdx = 0;

		int16 soundAction = argv[decodeIdx++];
		uint16 soundVolume = 65535;
		if (soundAction == kMystSoundActionChangeVolume || soundAction > 0) {
			soundVolume = argv[decodeIdx++];
		} else if (soundAction == kMystSoundActionConditional) {
			debugC(kDebugScript, "Conditional sound list");
			uint16 condVar = argv[decodeIdx++];
			uint16 condVarValue = getVar(condVar);
			uint16 condCount = argv[decodeIdx++];

			debugC(kDebugScript, "\tcondVar: %d = %d", condVar, condVarValue);
			debugC(kDebugScript, "\tcondCount: %d", condCount);

			soundList = new int16[condCount];
			soundListVolume = new uint16[condCount];

			if (condVarValue >= condCount)
				warning("Opcode %d: Conditional sound variable outside range",  op);
			else {
				for (uint16 i = 0; i < condCount; i++) {
					soundList[i] = argv[decodeIdx++];
					debugC(kDebugScript, "\t\tCondition %d: Action %d", i, soundList[i]);
					if (soundAction == kMystSoundActionChangeVolume || soundAction > 0) {
						soundListVolume[i] = argv[decodeIdx++];
					} else
						soundListVolume[i] = 65535;
					debugC(kDebugScript, "\t\tCondition %d: Volume %d", i, soundListVolume[i]);
				}

				soundAction = soundList[condVarValue];
				soundVolume = soundListVolume[condVarValue];
			}
		}

		// NOTE: Mixer only has 8-bit channel volume granularity,
		// Myst uses 16-bit? Or is part of this balance?
		soundVolume = (byte)(soundVolume / 255);

		if (soundAction == kMystSoundActionContinue)
			debugC(kDebugScript, "Continue current sound");
		else if (soundAction == kMystSoundActionChangeVolume) {
			debugC(kDebugScript, "Continue current sound, change volume");
			debugC(kDebugScript, "\tVolume: %d", soundVolume);
			// TODO: Implement Volume Control..
		} else if (soundAction == kMystSoundActionStop) {
			debugC(kDebugScript, "Stop sound");
			_vm->_sound->stopSound();
		} else if (soundAction > 0) {
			debugC(kDebugScript, "Play new Sound, change volume");
			debugC(kDebugScript, "\tSound: %d", soundAction);
			debugC(kDebugScript, "\tVolume: %d", soundVolume);
			_vm->_sound->stopSound();
			// TODO: Need to keep sound handle and add function to change volume of
			// looped running sound for kMystSoundActionChangeVolume type
			_vm->_sound->playSound(soundAction, soundVolume);
		} else {
			debugC(kDebugScript, "Unknown");
			warning("Unknown sound control value in opcode %d", op);
		}
	} else
		unknown(op, var, argc, argv);

	delete[] soundList;
	soundList = NULL;
	delete[] soundListVolume;
	soundListVolume = NULL;
}

void MystScriptParser::opcode_31(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Channelwood Card 3505 (Walkway from Sirrus' Room)
	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Boolean Choice of Play Sound", op);

		uint16 soundId0 = argv[0];
		uint16 soundId1 = argv[1];

		debugC(kDebugScript, "\tvar: %d", var);
		debugC(kDebugScript, "\tsoundId0: %d", soundId0);
		debugC(kDebugScript, "\tsoundId1: %d", soundId1);

		if (getVar(var)) {
			if (soundId1)
				_vm->_sound->playSound(soundId1);
		} else {
			if (soundId0)
				_vm->_sound->playSound(soundId0);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_32(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// Used on Channelwood Card 3503 (Door to Sirrus' Room)
	// Used on Myst Card 4188 (Door to Cabin)
	// Used on Myst Card 4363 (Red Book Open)
	// Used on Myst Card 4371 (Blue Book Open)
	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown...", op);
		// TODO: Implement function...
		// Set Resource 0 Enabled?
		// or Trigger Movie?
		// Set resource flag to Enabled?
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_34(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Change Card (with Delay?)", op);

		uint16 cardId = argv[0];
		uint16 u0 = argv[1];

		debugC(kDebugScript, "\tTarget Card: %d", cardId);
		debugC(kDebugScript, "\tu0: %d", u0); // TODO: Delay?

		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_35(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: Draw Full Screen Image, Delay then Change Card", op);

		uint16 imageId = argv[0];
		uint16 cardId = argv[1];
		uint16 delay = argv[2]; // TODO: Not sure about argv[2] being delay..

		debugC(kDebugScript, "\timageId: %d", imageId);
		debugC(kDebugScript, "\tcardId: %d", cardId);
		debugC(kDebugScript, "\tdelay: %d", delay);

		_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
		_vm->_gfx->updateScreen();
		_vm->_system->delayMillis(delay * 100);
		_vm->changeToCard(cardId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::changeCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Change Cursor", op);
		debugC(kDebugScript, "Cursor: %d", argv[0]);

		// TODO: Not sure if this needs to change mainCursor or similar...
		_vm->_cursor->setCursor(argv[0]);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::hideCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Hide Cursor", op);
		_vm->_cursor->hideCursor();
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::showCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Show Cursor", op);
		_vm->_cursor->showCursor();
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_39(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		// Used on Mechanical Card 6327 (Elevator)
		debugC(kDebugScript, "Opcode %d: Delay?", op);

		uint16 time = argv[0];

		debugC(kDebugScript, "\tTime: %d", time);

		// TODO: Fill in Function...
		// May actually be related to movie control.. not sure.
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::changeStack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	Audio::SoundHandle *handle;
	varUnusedCheck(op, var);

	if (argc == 3) {
		debugC(kDebugScript, "Opcode %d: changeStack", op);

		uint16 targetStack = argv[0];
		uint16 soundIdLinkSrc = argv[1];
		uint16 soundIdLinkDst = argv[2];

		debugC(kDebugScript, "\tTarget Stack: %d", targetStack);
		debugC(kDebugScript, "\tSource Stack Link Sound: %d", soundIdLinkSrc);
		debugC(kDebugScript, "\tDestination Stack Link Sound: %d", soundIdLinkDst);

		_vm->_sound->stopSound();

		if (_vm->getFeatures() & GF_DEMO) {

			// The demo has linking sounds too for this, but it just sounds completely
			// wrong as you're not actually linking when using this opcode. The sounds are only
			// played for the full game linking.
			if (!_vm->_tweaksEnabled) {
				handle= _vm->_sound->playSound(soundIdLinkSrc);
				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);
			}

			// No need to have a table for just this data...
			if (targetStack == 1) {
				_vm->changeToStack(kDemoSlidesStack);
				_vm->changeToCard(1000);
			} else if (targetStack == 2) {
				_vm->changeToStack(kDemoPreviewStack);
				_vm->changeToCard(3000);
			}

			if (!_vm->_tweaksEnabled) {
				handle = _vm->_sound->playSound(soundIdLinkDst);
				while (_vm->_mixer->isSoundHandleActive(*handle))
					_vm->_system->delayMillis(10);
			}
		} else {
			handle = _vm->_sound->playSound(soundIdLinkSrc);
			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);

			// TODO: Play Flyby Entry Movie on Masterpiece Edition..? Only on Myst to Age Link?
			_vm->changeToStack(stack_map[targetStack]);
			_vm->changeToCard(start_card[targetStack]);

			handle = _vm->_sound->playSound(soundIdLinkDst);
			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_41(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Will need to stop immediate screen update on
	//       Opcode 29 etc. for this to work correctly..
	//       Also, script processing will have to block U/I
	//       events etc. for correct sequencing.

	if (argc == 10 || argc == 16) {
		uint16 cardId = argv[0];
		uint16 soundId = argv[1];
		uint16 u0 = argv[2];
		uint16 u1 = argv[3];
		Common::Rect region = Common::Rect(argv[4], argv[5], argv[6], argv[7]);
		uint16 updateDirection = argv[8];
		uint16 u2 = argv[9];

		Common::Rect region2;
		uint16 updateDirection2 = 0;
		uint16 u3 = 0;
		if (argc == 16) {
			region2 = Common::Rect(argv[10], argv[11], argv[12], argv[13]);
			updateDirection2 = argv[14];
			u3 = argv[15];
		}

		debugC(kDebugScript, "Opcode %d: Change Card, Play Sound and Directional Update Screen Region", op);
		debugC(kDebugScript, "\tCard Id: %d", cardId);
		debugC(kDebugScript, "\tSound Id: %d", soundId);
		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);
		debugC(kDebugScript, "\tregion.left: %d", region.left);
		debugC(kDebugScript, "\tregion.top: %d", region.top);
		debugC(kDebugScript, "\tregion.right: %d", region.right);
		debugC(kDebugScript, "\tregion.bottom: %d", region.bottom);
		debugCN(kDebugScript, "\tupdateDirection: %d = ", updateDirection);

		switch (updateDirection) {
		case 0:
			debugC(kDebugScript, "Left to Right");
			break;
		case 1:
			debugC(kDebugScript, "Right to Left");
			break;
		case 5:
			debugC(kDebugScript, "Top to Bottom");
			break;
		case 6:
			debugC(kDebugScript, "Bottom to Top");
			break;
		default:
			warning("Unknown Update Direction");
			break;
		}

		debugC(kDebugScript, "\tu2: %d", u2); // TODO: Speed / Delay of Update?

		// 10 Argument version Used in:
		// Selenitic Card 1243 (Sound Receiver Door)
		// Myst Card 4317 (Generator Room Door)

		if (argc == 16) {
			// 16 Argument version Used in:
			// Selenitic Card 1008 and 1010 (Mazerunner Door)

			debugC(kDebugScript, "\tregion2.left: %d", region2.left);
			debugC(kDebugScript, "\tregion2.top: %d", region2.top);
			debugC(kDebugScript, "\tregion2.right: %d", region2.right);
			debugC(kDebugScript, "\tregion2.bottom: %d", region2.bottom);
			debugCN(kDebugScript, "\tupdateDirection2: %d = ", updateDirection2);

			switch (updateDirection2) {
			case 0:
				debugC(kDebugScript, "Left to Right");
				break;
			case 1:
				debugC(kDebugScript, "Right to Left");
				break;
			case 5:
				debugC(kDebugScript, "Top to Bottom");
				break;
			case 6:
				debugC(kDebugScript, "Bottom to Top");
				break;
			default:
				warning("Unknown Update Direction");
				break;
			}

			debugC(kDebugScript, "\tu3: %d", u3); // TODO: Speed / Delay of Update?
		}

		_vm->changeToCard(cardId);
		_vm->_sound->playSound(soundId);
		// TODO: Complete Implementation
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_42(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Will need to stop immediate screen update on
	//       Opcode 29 etc. for this to work correctly..
	//       Also, script processing will have to block U/I
	//       events etc. for correct sequencing.

	if (argc == 9 || argc == 15) {
		uint16 soundId = argv[0];
		uint16 u0 = argv[1];
		uint16 u1 = argv[2];
		Common::Rect region = Common::Rect(argv[3], argv[4], argv[5], argv[6]);
		uint16 updateDirection = argv[7];
		uint16 u2 = argv[8];

		Common::Rect region2;
		uint16 updateDirection2 = 0;
		uint16 u3 = 0;
		if (argc == 15) {
			region2 = Common::Rect(argv[9], argv[10], argv[11], argv[12]);
			updateDirection2 = argv[13];
			u3 = argv[14];
		}

		debugC(kDebugScript, "Opcode %d: Play Sound and Directional Update Screen Region", op);
		debugC(kDebugScript, "\tsound: %d", soundId);
		debugC(kDebugScript, "\tu0: %d", u0);
		debugC(kDebugScript, "\tu1: %d", u1);
		debugC(kDebugScript, "\tregion.left: %d", region.left);
		debugC(kDebugScript, "\tregion.top: %d", region.top);
		debugC(kDebugScript, "\tregion.right: %d", region.right);
		debugC(kDebugScript, "\tregion.bottom: %d", region.bottom);
		debugCN(kDebugScript, "\tupdateDirection: %d = ", updateDirection);

		switch (updateDirection) {
		case 0:
			debugC(kDebugScript, "Left to Right");
			break;
		case 1:
			debugC(kDebugScript, "Right to Left");
			break;
		case 5:
			debugC(kDebugScript, "Top to Bottom");
			break;
		case 6:
			debugC(kDebugScript, "Bottom to Top");
			break;
		default:
			warning("Unknown Update Direction");
			break;
		}

		debugC(kDebugScript, "\tu2: %d", u2); // TODO: Speed / Delay of Update?

		// 9 Argument version Used in:
		// Myst Card 4730 (Stellar Observatory Door)
		// Myst Card 4184 (Cabin Door)

		if (argc == 15) {
			// 15 Argument version Used in:
			// Channelwood Card 3492 (Achenar's Room Door)

			debugC(kDebugScript, "\tregion2.left: %d", region2.left);
			debugC(kDebugScript, "\tregion2.top: %d", region2.top);
			debugC(kDebugScript, "\tregion2.right: %d", region2.right);
			debugC(kDebugScript, "\tregion2.bottom: %d", region2.bottom);
			debugCN(kDebugScript, "\tupdateDirection2: %d = ", updateDirection2);

			switch (updateDirection2) {
			case 0:
				debugC(kDebugScript, "Left to Right");
				break;
			case 1:
				debugC(kDebugScript, "Right to Left");
				break;
			case 5:
				debugC(kDebugScript, "Top to Bottom");
				break;
			case 6:
				debugC(kDebugScript, "Bottom to Top");
				break;
			default:
				warning("Unknown Update Direction");
				break;
			}

			debugC(kDebugScript, "\tu3: %d", u3); // TODO: Speed / Delay of Update?
		}

		_vm->_sound->playSound(soundId);
		// TODO: Complete Implementation
	} else
		unknown(op, var, argc, argv);
}

// TODO: Are Opcode 43 and 44 enable / disable paired commands?

void MystScriptParser::opcode_43(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		// TODO: Function Unknown
		// Used on Stoneship Card 2154 (Bottom of Lighthouse)
		// Used on Stoneship Card 2138 (Lighthouse Floating Chest Closeup)
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_44(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		debugC(kDebugScript, "Opcode %d: Unknown Function", op);

		// TODO: Function Unknown
		// Used on Stoneship Card 2154 (Bottom of Lighthouse)
		// Used on Stoneship Card 2138 (Lighthouse Floating Chest Closeup)
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::opcode_46(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 0) {
		// Used on Selenitic Card 1191 (Maze Runner)
		// Used on Mechanical Card 6267 (Code Lock)
		// Used when Button is pushed...
		debugC(kDebugScript, "Opcode %d: Conditional Code Jump?", op);
		// TODO: Function Unknown - Fill in...
		// Logic looks like this is some kind of Conditional Code
		// Jump Point.
		// The Logic for the Mechanical Code Lock Seems to be in this
		// opcode with it being present twice delimiting the start
		// of the incorrect code and correct code action blocks...
		// Not sure how a general case can be made for this..
	} else
		unknown(op, var, argc, argv);
}

} // End of namespace Mohawk
