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
#include "mohawk/myst_saveload.h"
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
	_savedCardId = 0;
	_savedCursorId = 0;
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
		OPCODE(3, o_3_takePage),
		OPCODE(4, o_4_redrawCard),
		// Opcode 5 Not Present
		OPCODE(6, o_6_goToDest),
		OPCODE(7, o_6_goToDest),
		OPCODE(8, o_6_goToDest),
		OPCODE(9, o_9_triggerMovie),
		OPCODE(10, o_10_toggleVarNoRedraw),
		// Opcode 11 Not Present
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
		// Opcode 25 Not Present, original calls replaceSound
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
		// Opcode 45 Not Present
		OPCODE(46, o_46_soundWaitStop),
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
	switch(var) {
	case 105:
		return _tempVar;
	default:
		warning("Unimplemented var getter 0x%02x (%d)", var, var);
		return _vm->_varStore->getVar(var);
	}
}

void MystScriptParser::toggleVar(uint16 var) {
	warning("Unimplemented var toggle 0x%02x (%d)", var, var);
	_vm->_varStore->setVar(var, (_vm->_varStore->getVar(var) + 1) % 2);
}

bool MystScriptParser::setVarValue(uint16 var, uint16 value) {
	if (var == 105) {
		if (_tempVar != value)
			_tempVar = value;
	} else {
		warning("Unimplemented var setter 0x%02x (%d)", var, var);
		_vm->_varStore->setVar(var, value);
	}

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

void MystScriptParser::animatedUpdate(uint16 argc, uint16 *argv, uint16 delay) {
	uint16 argsRead = 0;

	while (argsRead < argc) {
		Common::Rect rect = Common::Rect(argv[argsRead], argv[argsRead + 1], argv[argsRead + 2], argv[argsRead + 3]);
		uint16 kind = argv[argsRead + 4];
		uint16 steps = argv[argsRead + 5];

		debugC(kDebugScript, "\trect.left: %d", rect.left);
		debugC(kDebugScript, "\trect.top: %d", rect.top);
		debugC(kDebugScript, "\trect.right: %d", rect.right);
		debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

		debugC(kDebugScript, "\tkind / direction: %d", kind);
		debugC(kDebugScript, "\tsteps: %d", steps);

		_vm->_gfx->animatedUpdate(kind, rect, steps, delay);

		argsRead += 6;
	}
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
		_vm->changeToCard(argv[value -1 ], true);
	else if (_invokingResource != NULL)
		_vm->changeToCard(_invokingResource->getDest(), true);
	else
		warning("Missing invokingResource in altDest call");
}

void MystScriptParser::o_3_takePage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 *game_globals = _vm->_saveLoad->_v->game_globals;

	uint16 cursorId = argv[0];
	uint16 oldPage = game_globals[2];

	debugC(kDebugScript, "Opcode %d: takePage Var %d CursorId %d", op, var, cursorId);

	// Take / drop page
	toggleVar(var);

	if (oldPage != game_globals[2]) {
		_vm->_gfx->hideCursor();
		_vm->redrawArea(var);

		// Set new cursor
		if (game_globals[2])
			_vm->setMainCursor(cursorId);
		else
			_vm->setMainCursor(kDefaultMystCursor);

		_vm->_gfx->showCursor();
	}
}

void MystScriptParser::o_4_redrawCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Redraw card", op);

	// TODO: Is redrawing the background correct ?
	_vm->drawCardBackground();
	_vm->drawResourceImages();
	_vm->_gfx->updateScreen();
}

void MystScriptParser::o_6_goToDest(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Change To Dest of Invoking Resource", op);

	if (_invokingResource != NULL)
		_vm->changeToCard(_invokingResource->getDest(), true);
	else
		warning("Opcode %d: Missing invokingResource", op);
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

void MystScriptParser::o_16_changeCardDirectional(uint16 op, uint16 var, uint16 argc, uint16 *argv) {

	// Used by Channelwood Card 3262 (In Elevator)
	debugC(kDebugScript, "Opcode %d: Change Card with optional directional update", op);

	uint16 cardId = argv[0];
	uint16 directionalUpdateDataSize = argv[1];

	debugC(kDebugScript, "\tcardId: %d", cardId);
	debugC(kDebugScript, "\tdirectonal update data size: %d", directionalUpdateDataSize);

	_vm->changeToCard(cardId, false);

	animatedUpdate(directionalUpdateDataSize, &argv[2], 0);
}

// NOTE: Opcode 17 and 18 form a pair, where Opcode 17 jumps to a card,
// but with the current cardId stored.
// Opcode 18 then "pops" this stored CardId and returns to that card.

// TODO: The purpose of the optional argv[1] on Opcode 17 and argv[0]
// on Opcode 18 which are always 4, 5 or 6 is unknown.

void MystScriptParser::o_17_changeCardPush(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 2) {
		debugC(kDebugScript, "Opcode %d: Jump to Card Id, Storing Current Card Id", op);

		uint16 cardId = argv[0];
		debugC(kDebugScript, "\tJump to CardId: %d", cardId);

		uint16 u0 = argv[1]; // TODO
		debugC(kDebugScript, "\tu0: %d", u0);

		_savedCardId = _vm->getCurCard();

		debugC(kDebugScript, "\tCurrent CardId: %d", _savedCardId);

		_vm->changeToCard(cardId, true);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_18_changeCardPop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		debugC(kDebugScript, "Opcode %d: Return To Stored Card Id", op);
		debugC(kDebugScript, "\tCardId: %d", _savedCardId);

		uint16 u0 = argv[0];
		debugC(kDebugScript, "\tu0: %d", u0);

		_vm->changeToCard(_savedCardId, true);
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

void MystScriptParser::o_21_directionalUpdate(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Transition / Directional update", op);

	animatedUpdate(argc, argv, 0);
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

void MystScriptParser::o_24_playSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	if (argc == 1) {
		uint16 soundId = argv[0];

		debugC(kDebugScript, "Opcode %d: playSound", op);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		_vm->_sound->playSound(soundId);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_26_stopSoundBackground(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: stopSoundBackground", op);
	//_vm->_sound->stopBackground();
}

void MystScriptParser::o_27_playSoundBlocking(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 soundId = argv[0];

	debugC(kDebugScript, "Opcode %d: playSoundBlocking", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	_vm->_sound->playSoundBlocking(soundId);
}

void MystScriptParser::o_28_restoreDefaultRect(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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

		_vm->_gfx->copyImageSectionToScreen(_vm->getCardBackgroundId(), rect, rect);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_29_33_blitRect(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	varUnusedCheck(op, var);

	// TODO: Opcode 29 called on Mechanical Card 6178 causes a engine
	//       abort this is because imageId is 7158 (not valid), but the
	//       script resource gives this as 7178 (valid)...
	// FIXME: opcode 33 also hides the cursor when drawing if it is in the way

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
void MystScriptParser::o_30_changeSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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

void MystScriptParser::o_31_soundPlaySwitch(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Switch Choice of Play Sound", op);

	uint16 value = getVar(var);
	if (value < argc) {
		uint16 soundId = argv[value];
		debugC(kDebugScript, "\tvar: %d", var);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		if (soundId)
			_vm->_sound->playSound(soundId);
	}
}

void MystScriptParser::o_32_soundResumeBackground(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: soundResumeBackground", op);
	//_vm->_sound->resumeBackground();
}

void MystScriptParser::o_34_changeCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Change Card", op);

	uint16 cardId = argv[0];

	// Argument 1 if present is not used
	// uint16 u0 = argv[1];

	debugC(kDebugScript, "\tTarget Card: %d", cardId);
	//debugC(kDebugScript, "\tu0: %d", u0); // Unused data

	_vm->changeToCard(cardId, true);
}

void MystScriptParser::o_35_drawImageChangeCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
		_vm->changeToCard(cardId, true);
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_36_changeMainCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Change main cursor", op);

	uint16 cursorId = argv[0];

	debugC(kDebugScript, "Cursor: %d", cursorId);

	_vm->setMainCursor(cursorId);
	_vm->_cursor->setCursor(cursorId);
}

void MystScriptParser::o_37_hideCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hide Cursor", op);
	_vm->_cursor->hideCursor();
}

void MystScriptParser::o_38_showCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Show Cursor", op);
	_vm->_cursor->showCursor();
}

void MystScriptParser::o_39_delay(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Mechanical Card 6327 (Elevator)
	debugC(kDebugScript, "Opcode %d: Delay", op);

	uint16 time = argv[0];

	debugC(kDebugScript, "\tTime: %d", time);

	_vm->_system->delayMillis(time);
}

void MystScriptParser::o_40_changeStack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
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
				_vm->changeToCard(1000, true);
			} else if (targetStack == 2) {
				_vm->changeToStack(kDemoPreviewStack);
				_vm->changeToCard(3000, true);
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
			_vm->changeToCard(start_card[targetStack], true);

			handle = _vm->_sound->playSound(soundIdLinkDst);
			while (_vm->_mixer->isSoundHandleActive(*handle))
				_vm->_system->delayMillis(10);
		}
	} else
		unknown(op, var, argc, argv);
}

void MystScriptParser::o_41_changeCardPlaySoundDirectional(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Sound, Change Card and Directional Update Screen Region", op);

	uint16 cardId = argv[0];
	uint16 soundId = argv[1];
	uint16 delayBetweenSteps = argv[2];
	uint16 dataSize = argv[3];

	debugC(kDebugScript, "\tcard: %d", cardId);
	debugC(kDebugScript, "\tsound: %d", soundId);
	debugC(kDebugScript, "\tdelay between steps: %d", delayBetweenSteps);
	debugC(kDebugScript, "\tanimated update data size: %d", dataSize);

	if (soundId)
		_vm->_sound->playSound(soundId);

	_vm->changeToCard(cardId, false);

	animatedUpdate(dataSize, &argv[4], delayBetweenSteps);
}

void MystScriptParser::o_42_directionalUpdatePlaySound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Sound and Directional Update Screen Region", op);

	uint16 soundId = argv[0];
	uint16 delayBetweenSteps = argv[1];
	uint16 dataSize = argv[2];

	debugC(kDebugScript, "\tsound: %d", soundId);
	debugC(kDebugScript, "\tdelay between steps: %d", delayBetweenSteps);
	debugC(kDebugScript, "\tanimated update data size: %d", dataSize);

	if (soundId)
		_vm->_sound->playSound(soundId);

	animatedUpdate(dataSize, &argv[3], delayBetweenSteps);
}

void MystScriptParser::o_43_saveMainCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Save main cursor", op);

	_savedCursorId = _vm->getMainCursor();
}

void MystScriptParser::o_44_restoreMainCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Restore main cursor", op);

	_vm->setMainCursor(_savedCursorId);
}

void MystScriptParser::o_46_soundWaitStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Selenitic Card 1191 (Maze Runner)
	// Used on Mechanical Card 6267 (Code Lock)
	// Used when Button is pushed...
	debugC(kDebugScript, "Opcode %d: Wait for foreground sound to finish", op);

	//TODO: Implement
}

} // End of namespace Mohawk
