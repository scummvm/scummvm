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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_card.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/myst_sound.h"
#include "mohawk/video.h"

#include "common/debug-channels.h"
#include "common/system.h"
#include "common/memstream.h"
#include "common/textconsole.h"

namespace Mohawk {

MystScriptEntry::MystScriptEntry() {
	type = kMystScriptNone;
	var = 0;
	resourceId = 0;
	u1 = 0;
	opcode = 0;
}

const MystStack MystScriptParser::_stackMap[11] = {
	kSeleniticStack,
	kStoneshipStack,
	kMystStack,
	kMechanicalStack,
	kChannelwoodStack,
	kIntroStack,
	kDniStack,
	kMystStack,
	kCreditsStack,
	kMystStack,
	kMystStack
};

const uint16 MystScriptParser::_startCard[11] = {
	1282,
	2029,
	4396,
	6122,
	3137,
	1,
	5038,
	4134,
	10000,
	4739,
	4741
};

// NOTE: Credits Start Card is 10000

MystScriptParser::MystScriptParser(MohawkEngine_Myst *vm, MystStack stackId) :
		_vm(vm),
		_stackId(stackId),
		_globals(vm->_gameState->_globals) {
	setupCommonOpcodes();
	_invokingResource = nullptr;
	_savedCardId = 0;
	_savedCursorId = 0;
	_savedMapCardId = 0;
	_tempVar = 0;
	_scriptNestingLevel = 0;
	_startTime = 0;
}

MystScriptParser::~MystScriptParser() {
}

void MystScriptParser::setupCommonOpcodes() {
	// These opcodes are common to each stack

	// "Standard" Opcodes
	REGISTER_OPCODE(0, MystScriptParser, o_toggleVar);
	REGISTER_OPCODE(1, MystScriptParser, o_setVar);
	REGISTER_OPCODE(2, MystScriptParser, o_changeCardSwitch4);
	REGISTER_OPCODE(3, MystScriptParser, o_takePage);
	REGISTER_OPCODE(4, MystScriptParser, o_redrawCard);
	// Opcode 5 Not Present
	REGISTER_OPCODE(6, MystScriptParser, o_goToDestForward);
	REGISTER_OPCODE(7, MystScriptParser, o_goToDestRight);
	REGISTER_OPCODE(8, MystScriptParser, o_goToDestLeft);
	REGISTER_OPCODE(9, MystScriptParser, o_triggerMovie);
	REGISTER_OPCODE(10, MystScriptParser, o_toggleVarNoRedraw);
	// Opcode 11 Not Present
	REGISTER_OPCODE(12, MystScriptParser, o_changeCardSwitchLtR);
	REGISTER_OPCODE(13, MystScriptParser, o_changeCardSwitchRtL);
	REGISTER_OPCODE(14, MystScriptParser, o_drawAreaState);
	REGISTER_OPCODE(15, MystScriptParser, o_redrawAreaForVar);
	REGISTER_OPCODE(16, MystScriptParser, o_changeCardDirectional);
	REGISTER_OPCODE(17, MystScriptParser, o_changeCardPush);
	REGISTER_OPCODE(18, MystScriptParser, o_changeCardPop);
	REGISTER_OPCODE(19, MystScriptParser, o_enableAreas);
	REGISTER_OPCODE(20, MystScriptParser, o_disableAreas);
	REGISTER_OPCODE(21, MystScriptParser, o_directionalUpdate);
	REGISTER_OPCODE(22, MystScriptParser, o_goToDestUp);
	REGISTER_OPCODE(23, MystScriptParser, o_toggleAreasActivation);
	REGISTER_OPCODE(24, MystScriptParser, o_playSound);
	// Opcode 25 is unused; original calls replaceSoundMyst
	REGISTER_OPCODE(26, MystScriptParser, o_stopSoundBackground);
	REGISTER_OPCODE(27, MystScriptParser, o_playSoundBlocking);
	REGISTER_OPCODE(28, MystScriptParser, o_copyBackBufferToScreen);
	REGISTER_OPCODE(29, MystScriptParser, o_copyImageToBackBuffer);
	REGISTER_OPCODE(30, MystScriptParser, o_changeBackgroundSound);
	REGISTER_OPCODE(31, MystScriptParser, o_soundPlaySwitch);
	REGISTER_OPCODE(32, MystScriptParser, o_soundResumeBackground);
	REGISTER_OPCODE(33, MystScriptParser, o_copyImageToScreen);
	REGISTER_OPCODE(34, MystScriptParser, o_changeCard);
	REGISTER_OPCODE(35, MystScriptParser, o_drawImageChangeCard);
	REGISTER_OPCODE(36, MystScriptParser, o_changeMainCursor);
	REGISTER_OPCODE(37, MystScriptParser, o_hideCursor);
	REGISTER_OPCODE(38, MystScriptParser, o_showCursor);
	REGISTER_OPCODE(39, MystScriptParser, o_delay);
	REGISTER_OPCODE(40, MystScriptParser, o_changeStack);
	REGISTER_OPCODE(41, MystScriptParser, o_changeCardPlaySoundDirectional);
	REGISTER_OPCODE(42, MystScriptParser, o_directionalUpdatePlaySound);
	REGISTER_OPCODE(43, MystScriptParser, o_saveMainCursor);
	REGISTER_OPCODE(44, MystScriptParser, o_restoreMainCursor);
	// Opcode 45 Not Present
	REGISTER_OPCODE(46, MystScriptParser, o_soundWaitStop);
	REGISTER_OPCODE(48, MystScriptParser, o_goToDest);
	REGISTER_OPCODE(51, MystScriptParser, o_exitMap);
	// Opcodes 47 to 99 Not Present

	REGISTER_OPCODE(0xFFFF, MystScriptParser, NOP);
}

void MystScriptParser::registerOpcode(uint16 op, const char *name, OpcodeProcMyst *command) {
	_opcodes.push_back(MystOpcode(op, command, name));
}

void MystScriptParser::overrideOpcode(uint16 op, const char *name, MystScriptParser::OpcodeProcMyst *command) {
	for (uint i = 0; i < _opcodes.size(); i++) {
		if (_opcodes[i].op == op) {
			_opcodes[i].desc = name;
			_opcodes[i].proc = Common::SharedPtr<OpcodeProcMyst>(command);
			return;
		}
	}

	warning("Unable to find opcode %d to override with '%s'", op, name);
}

void MystScriptParser::runScript(const MystScript &script, MystArea *invokingResource) {
	_scriptNestingLevel++;

	for (uint16 i = 0; i < script.size(); i++) {
		const MystScriptEntry &entry = script[i];

		if (entry.type == kMystScriptNormal)
			_invokingResource = invokingResource;
		else
			_invokingResource = _vm->getCard()->getResource<MystArea>(entry.resourceId);

		runOpcode(entry.opcode, entry.var, entry.args);
	}

	_scriptNestingLevel--;
}

void MystScriptParser::runOpcode(uint16 op, uint16 var, const ArgumentsArray &args) {
	_scriptNestingLevel++;

	bool ranOpcode = false;
	for (uint16 i = 0; i < _opcodes.size(); i++)
		if (_opcodes[i].op == op) {
			if (DebugMan.isDebugChannelEnabled(kDebugScript)) {
				debugC(kDebugScript, "Running command: %s", describeCommand(_opcodes[i], var, args).c_str());
			}

			(*_opcodes[i].proc)(var, args);
			ranOpcode = true;
			break;
		}

	if (!ranOpcode)
		warning("Trying to run invalid opcode %d", op);

	_scriptNestingLevel--;
}

bool MystScriptParser::isScriptRunning() const {
	return _scriptNestingLevel > 0;
}

const Common::String MystScriptParser::getOpcodeDesc(uint16 op) {
	for (uint16 i = 0; i < _opcodes.size(); i++)
		if (_opcodes[i].op == op)
			return _opcodes[i].desc;

	return Common::String::format("%d", op);
}

Common::String MystScriptParser::describeCommand(const MystOpcode &command, uint16 var, const ArgumentsArray &args) {
	Common::String desc = Common::String::format("%s(", command.desc);

	if (var != 0) {
		desc += Common::String::format("var = %d%s", var, args.size() != 0 ? ", " : "");
	}

	for (uint16 j = 0; j < args.size(); j++) {
		desc += Common::String::format("%d", args[j]);
		if (j != args.size() - 1)
			desc += ", ";
	}
	desc += ")";
	return desc;
}

MystScript MystScriptParser::readScript(Common::SeekableReadStream *stream, MystScriptType type) {
	assert(stream);
	assert(type != kMystScriptNone);

	uint16 opcodeCount = stream->readUint16LE();

	MystScript script(opcodeCount);

	for (uint16 i = 0; i < opcodeCount; i++) {
		MystScriptEntry &entry = script[i];
		entry.type = type;

		// Resource ID only exists in INIT and EXIT scripts
		if (type != kMystScriptNormal)
			entry.resourceId = stream->readUint16LE();

		entry.opcode = stream->readUint16LE();
		entry.var = stream->readUint16LE();
		uint16 argumentCount = stream->readUint16LE();

		entry.args.resize(argumentCount);
		for (uint16 j = 0; j < entry.args.size(); j++)
			entry.args[j] = stream->readUint16LE();

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
	case 106:
		return _globals.ending;
	default:
		warning("Unimplemented var getter 0x%02x (%d)", var, var);
		return 0;
	}
}

void MystScriptParser::toggleVar(uint16 var) {
	warning("Unimplemented var toggle 0x%02x (%d)", var, var);
}

bool MystScriptParser::setVarValue(uint16 var, uint16 value) {
	if (var == 105) {
		if (_tempVar != value)
			_tempVar = value;
	} else {
		warning("Unimplemented var setter 0x%02x (%d)", var, var);
	}

	return false;
}

void MystScriptParser::animatedUpdate(const ArgumentsArray &args, uint16 delay) {
	uint16 argsRead = 0;

	while (argsRead < args.size()) {
		Common::Rect rect = Common::Rect(args[argsRead], args[argsRead + 1], args[argsRead + 2], args[argsRead + 3]);
		TransitionType kind = static_cast<TransitionType>(args[argsRead + 4]);
		uint16 steps = args[argsRead + 5];

		debugC(kDebugScript, "\trect.left: %d", rect.left);
		debugC(kDebugScript, "\trect.top: %d", rect.top);
		debugC(kDebugScript, "\trect.right: %d", rect.right);
		debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

		debugC(kDebugScript, "\tkind / direction: %d", kind);
		debugC(kDebugScript, "\tsteps: %d", steps);

		_vm->_gfx->runTransition(kind, rect, steps, delay);

		argsRead += 6;
	}
}

void MystScriptParser::NOP(uint16 var, const ArgumentsArray &args) {
}

void MystScriptParser::o_toggleVar(uint16 var, const ArgumentsArray &args) {
	toggleVar(var);
	_vm->getCard()->redrawArea(var);
}

void MystScriptParser::o_setVar(uint16 var, const ArgumentsArray &args) {
	if (setVarValue(var, args[0]))
		_vm->getCard()->redrawArea(var);
}

void MystScriptParser::o_changeCardSwitch4(uint16 var, const ArgumentsArray &args) {
	uint16 value = getVar(var);

	if (value)
		_vm->changeToCard(args[value -1 ], kTransitionDissolve);
	else if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionDissolve);
	else
		warning("Missing invokingResource in altDest call");
}

void MystScriptParser::o_changeCardSwitchLtR(uint16 var, const ArgumentsArray &args) {
	uint16 value = getVar(var);

	if (value)
		_vm->changeToCard(args[value -1 ], kTransitionLeftToRight);
	else if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionLeftToRight);
	else
		warning("Missing invokingResource in altDest call");
}

void MystScriptParser::o_changeCardSwitchRtL(uint16 var, const ArgumentsArray &args) {
	uint16 value = getVar(var);

	if (value)
		_vm->changeToCard(args[value -1 ], kTransitionRightToLeft);
	else if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionRightToLeft);
	else
		warning("Missing invokingResource in altDest call");
}

void MystScriptParser::o_takePage(uint16 var, const ArgumentsArray &args) {
	// In most game releases, the first opcode argument is the new mouse cursor.
	// However, in the original v1.0 English release this opcode takes no argument.
	uint16 cursorId; // = args[0];
	switch (var) {
		case 41: // Vault white page
			cursorId = kWhitePageCursor;
			break;
		case 25:  // Fireplace red page
		case 102: // Red page
			cursorId = kRedPageCursor;
			break;
		case 24:  // Fireplace blue page
		case 103: // Blue page
			cursorId = kBluePageCursor;
			break;
		default:
			warning("Unexpected take page variable '%d'", var);
			cursorId = kDefaultMystCursor;
	}

	HeldPage oldPage = _globals.heldPage;

	// Take / drop page
	toggleVar(var);

	if (oldPage != _globals.heldPage) {
		_vm->_cursor->hideCursor();
		_vm->getCard()->redrawArea(var);

		// Set new cursor
		if (_globals.heldPage != kNoPage)
			_vm->setMainCursor(cursorId);
		else
			_vm->setMainCursor(kDefaultMystCursor);

		_vm->_cursor->showCursor();
	}
}

void MystScriptParser::o_redrawCard(uint16 var, const ArgumentsArray &args) {
	_vm->getCard()->drawBackground();
	_vm->getCard()->drawResourceImages();
	_vm->_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
}

void MystScriptParser::o_goToDest(uint16 var, const ArgumentsArray &args) {
	if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionCopy);
	else
		warning("Opcode o_goToDest: Missing invokingResource");
}

void MystScriptParser::o_goToDestForward(uint16 var, const ArgumentsArray &args) {
	if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionDissolve);
	else
		warning("Opcode o_goToDestForward: Missing invokingResource");
}

void MystScriptParser::o_goToDestRight(uint16 var, const ArgumentsArray &args) {
	if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionPartToRight);
	else
		warning("Opcode o_goToDestRight: Missing invokingResource");
}

void MystScriptParser::o_goToDestLeft(uint16 var, const ArgumentsArray &args) {
	if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionPartToLeft);
	else
		warning("Opcode o_goToDestLeft: Missing invokingResource");
}

void MystScriptParser::o_goToDestUp(uint16 var, const ArgumentsArray &args) {
	if (_invokingResource != nullptr)
		_vm->changeToCard(_invokingResource->getDest(), kTransitionTopToBottom);
	else
		warning("Opcode o_goToDestUp: Missing invokingResource");
}

void MystScriptParser::o_triggerMovie(uint16 var, const ArgumentsArray &args) {
	// The original has code to pause the background music before playing the movie,
	// if the movie has a sound track, as well as code to resume it afterwards. But since
	// the movie has not yet been loaded at this point, it is impossible to know
	// if the movie actually has a sound track. The code is never executed.

	int16 direction = 1;
	if (args.size() == 1)
		direction = args[0];

	// Trigger resource 6 movie overriding play direction
	MystAreaVideo *resource = getInvokingResource<MystAreaVideo>();
	resource->setDirection(direction);
	resource->playMovie();
}

void MystScriptParser::o_toggleVarNoRedraw(uint16 var, const ArgumentsArray &args) {
	toggleVar(var);
}

void MystScriptParser::o_drawAreaState(uint16 var, const ArgumentsArray &args) {
	MystAreaImageSwitch *parent = static_cast<MystAreaImageSwitch *>(getInvokingResource<MystArea>()->_parent);
	parent->drawConditionalDataToScreen(args[0]);
}

void MystScriptParser::o_redrawAreaForVar(uint16 var, const ArgumentsArray &args) {
	_vm->getCard()->redrawArea(var);
}

void MystScriptParser::o_changeCardDirectional(uint16 var, const ArgumentsArray &args) {

	// Used by Channelwood Card 3262 (In Elevator)
	uint16 cardId = args[0];
	uint16 directionalUpdateDataSize = args[1];

	_vm->changeToCard(cardId, kNoTransition);

	animatedUpdate(ArgumentsArray(args.begin() + 2, directionalUpdateDataSize), 0);
}

// NOTE: Opcode 17 and 18 form a pair, where Opcode 17 jumps to a card,
// but with the current cardId stored.
// Opcode 18 then "pops" this stored CardId and returns to that card.

void MystScriptParser::o_changeCardPush(uint16 var, const ArgumentsArray &args) {
	_savedCardId = _vm->getCard()->getId();

	uint16 cardId = args[0];
	TransitionType transition = static_cast<TransitionType>(args[1]);

	_vm->changeToCard(cardId, transition);
}

void MystScriptParser::o_changeCardPop(uint16 var, const ArgumentsArray &args) {
	if (_savedCardId == 0) {
		warning("No pushed card to go back to");
		return;
	}

	TransitionType transition = static_cast<TransitionType>(args[0]);

	_vm->changeToCard(_savedCardId, transition);
}

void MystScriptParser::o_enableAreas(uint16 var, const ArgumentsArray &args) {
	uint16 count = args[0];

	for (uint16 i = 0; i < count; i++) {
		MystArea *resource = nullptr;
		if (args[i + 1] == 0xFFFF)
			resource = _invokingResource;
		else
			resource = _vm->getCard()->getResource<MystArea>(args[i + 1]);

		if (resource)
			resource->setEnabled(true);
		else
			warning("Unknown Resource in enableAreas script Opcode");
	}
}

void MystScriptParser::o_disableAreas(uint16 var, const ArgumentsArray &args) {
	uint16 count = args[0];

	for (uint16 i = 0; i < count; i++) {
		MystArea *resource = nullptr;
		if (args[i + 1] == 0xFFFF)
			resource = _invokingResource;
		else
			resource = _vm->getCard()->getResource<MystArea>(args[i + 1]);

		if (resource)
			resource->setEnabled(false);
		else
			warning("Unknown Resource in disableAreas script Opcode");
	}
}

void MystScriptParser::o_directionalUpdate(uint16 var, const ArgumentsArray &args) {
	animatedUpdate(args, 0);
}

void MystScriptParser::o_toggleAreasActivation(uint16 var, const ArgumentsArray &args) {
	uint16 count = args[0];

	for (uint16 i = 0; i < count; i++) {
		MystArea *resource = nullptr;
		if (args[i + 1] == 0xFFFF)
			resource = _invokingResource;
		else
			resource = _vm->getCard()->getResource<MystArea>(args[i + 1]);

		if (resource)
			resource->setEnabled(!resource->isEnabled());
		else
			warning("Unknown Resource in toggleAreasActivation script Opcode");
	}
}

void MystScriptParser::o_playSound(uint16 var, const ArgumentsArray &args) {
	uint16 soundId = args[0];

	// WORKAROUND: In the Myst age, when in front of the cabin coming from the left
	// with the door open, when trying to go left, a script tries to play a sound
	// with id 4197. That sound does not exist in the game archives. However, when
	// going right another script plays a door closing sound with id 4191.
	// Here, we replace the incorrect sound id with a proper one.
	if (soundId == 4197) {
		soundId = 4191;
	}

	_vm->_sound->playEffect(soundId);
}

void MystScriptParser::o_stopSoundBackground(uint16 var, const ArgumentsArray &args) {
	_vm->_sound->stopBackground();
}

void MystScriptParser::o_playSoundBlocking(uint16 var, const ArgumentsArray &args) {
	uint16 soundId = args[0];

	_vm->_sound->stopEffect();
	_vm->playSoundBlocking(soundId);
}

void MystScriptParser::o_copyBackBufferToScreen(uint16 var, const ArgumentsArray &args) {
	Common::Rect rect;
	if (args[0] == 0xFFFF) {
		// Used in Stoneship Card 2111 (Compass Rose)
		// Used in Mechanical Card 6267 (Code Lock)
		rect = _invokingResource->getRect();
	} else {
		rect = Common::Rect(args[0], args[1], args[2], args[3]);
	}

	debugC(kDebugScript, "\trect.left: %d", rect.left);
	debugC(kDebugScript, "\trect.top: %d", rect.top);
	debugC(kDebugScript, "\trect.right: %d", rect.right);
	debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

	_vm->_gfx->copyBackBufferToScreen(rect);

	// WORKAROUND: On Channelwood, wait for the sound to complete when
	// closing the gate on the third level near the blue page.
	// Fixes the gate not changing visual state despite the closing
	// sound playing.
	// There is one card id per side of the gate.
	if (_vm->getCard()->getId() == 3481 || _vm->getCard()->getId() == 3522) {
		soundWaitStop();
	}
}

void MystScriptParser::o_copyImageToBackBuffer(uint16 var, const ArgumentsArray &args) {
	uint16 imageId = args[0];

	// WORKAROUND wrong image id in mechanical staircase
	if (imageId == 7158)
		imageId = 7178;

	Common::Rect srcRect = Common::Rect(args[1], args[2], args[3], args[4]);

	Common::Rect dstRect = Common::Rect(args[5], args[6], 544, 333);

	if (dstRect.left == -1) {
		dstRect.left = 0;
	}

	if (dstRect.top == -1) {
		dstRect.top = 0;
	}

	dstRect.right = dstRect.left + srcRect.width();
	dstRect.bottom = dstRect.top + srcRect.height();

	debugC(kDebugScript, "\tsrcRect.left: %d", srcRect.left);
	debugC(kDebugScript, "\tsrcRect.top: %d", srcRect.top);
	debugC(kDebugScript, "\tsrcRect.right: %d", srcRect.right);
	debugC(kDebugScript, "\tsrcRect.bottom: %d", srcRect.bottom);
	debugC(kDebugScript, "\tdstRect.left: %d", dstRect.left);
	debugC(kDebugScript, "\tdstRect.top: %d", dstRect.top);
	debugC(kDebugScript, "\tdstRect.right: %d", dstRect.right);
	debugC(kDebugScript, "\tdstRect.bottom: %d", dstRect.bottom);

	_vm->_gfx->copyImageSectionToBackBuffer(imageId, srcRect, dstRect);

	// WORKAROUND: When hitting the switch of the torture chamber in Achenar's
	// hidden room on the Mechanical Age, the game calls this opcode multiple
	// times in a row with different images without waiting in between.
	// As a result the images are not shown since the screen is only
	// updated once per frame. The original engine misbehaves as well.
	// Here we artificially introduce a delay after each image to allow
	// them to be visible for a few frames.
	if (_vm->getCard()->getId() == 6009) {
		_vm->wait(100);
	}
}

void MystScriptParser::o_changeBackgroundSound(uint16 var, const ArgumentsArray &args) {
	soundWaitStop();

	// Used on Stoneship Card 2080
	// Used on Channelwood Card 3225 with argc = 8 i.e. Conditional Sound List
	Common::MemoryWriteStreamDynamic writeStream = Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	for (uint i = 0; i < args.size(); i++) {
		writeStream.writeUint16LE(args[i]);
	}

	Common::MemoryReadStream readStream = Common::MemoryReadStream(writeStream.getData(), writeStream.size());

	MystSoundBlock soundBlock = _vm->readSoundBlock(&readStream);
	_vm->applySoundBlock(soundBlock);
}

void MystScriptParser::o_soundPlaySwitch(uint16 var, const ArgumentsArray &args) {
	uint16 value = getVar(var);

	if (value < args.size()) {
		uint16 soundId = args[value];
		if (soundId)
			_vm->_sound->playEffect(soundId);
	}
}

void MystScriptParser::o_soundResumeBackground(uint16 var, const ArgumentsArray &args) {
	_vm->_sound->resumeBackground();
}

void MystScriptParser::o_copyImageToScreen(uint16 var, const ArgumentsArray &args) {
	uint16 imageId = args[0];

	Common::Rect srcRect = Common::Rect(args[1], args[2], args[3], args[4]);

	Common::Rect dstRect = Common::Rect(args[5], args[6], 544, 333);

	if (dstRect.left == -1 || dstRect.top == -1) {
		// Interpreted as full screen
		dstRect.left = 0;
		dstRect.top = 0;
	}

	dstRect.right = dstRect.left + srcRect.width();
	dstRect.bottom = dstRect.top + srcRect.height();

	debugC(kDebugScript, "\tsrcRect.left: %d", srcRect.left);
	debugC(kDebugScript, "\tsrcRect.top: %d", srcRect.top);
	debugC(kDebugScript, "\tsrcRect.right: %d", srcRect.right);
	debugC(kDebugScript, "\tsrcRect.bottom: %d", srcRect.bottom);
	debugC(kDebugScript, "\tdstRect.left: %d", dstRect.left);
	debugC(kDebugScript, "\tdstRect.top: %d", dstRect.top);
	debugC(kDebugScript, "\tdstRect.right: %d", dstRect.right);
	debugC(kDebugScript, "\tdstRect.bottom: %d", dstRect.bottom);

	_vm->_gfx->copyImageSectionToScreen(imageId, srcRect, dstRect);
}

void MystScriptParser::o_changeCard(uint16 var, const ArgumentsArray &args) {
	uint16 cardId = args[0];
	TransitionType transition = static_cast<TransitionType>(args[1]);

	_vm->changeToCard(cardId, transition);
}

void MystScriptParser::o_drawImageChangeCard(uint16 var, const ArgumentsArray &args) {
		uint16 imageId = args[0];
		uint16 cardId = args[1];
		TransitionType transition = static_cast<TransitionType>(args[2]);

		_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
		_vm->wait(200);

		_vm->changeToCard(cardId, transition);
}

void MystScriptParser::o_changeMainCursor(uint16 var, const ArgumentsArray &args) {
	uint16 cursorId = args[0];

	_vm->setMainCursor(cursorId);
	_vm->_cursor->setCursor(cursorId);
}

void MystScriptParser::o_hideCursor(uint16 var, const ArgumentsArray &args) {
	_vm->_cursor->hideCursor();
}

void MystScriptParser::o_showCursor(uint16 var, const ArgumentsArray &args) {
	_vm->_cursor->showCursor();
}

void MystScriptParser::o_delay(uint16 var, const ArgumentsArray &args) {
	// Used on Mechanical Card 6327 (Elevator)
	uint16 time = args[0];

	_vm->wait(time);
}

void MystScriptParser::o_changeStack(uint16 var, const ArgumentsArray &args) {
	uint16 targetStack = args[0];
	uint16 soundIdLinkSrc = args[1];
	uint16 soundIdLinkDst = args[2];

	debugC(kDebugScript, "\tSource Stack Link Sound: %d", soundIdLinkSrc);
	debugC(kDebugScript, "\tDestination Stack Link Sound: %d", soundIdLinkDst);

	_vm->_sound->stopEffect();

	if (_vm->isGameVariant(GF_DEMO)) {
		// No need to have a table for just this data...
		if (targetStack == 1)
			_vm->changeToStack(kDemoSlidesStack, 1000, soundIdLinkSrc, soundIdLinkDst);
		else if (targetStack == 2)
			_vm->changeToStack(kDemoPreviewStack, 3000, soundIdLinkSrc, soundIdLinkDst);
	} else {
		_vm->changeToStack(_stackMap[targetStack], _startCard[targetStack], soundIdLinkSrc, soundIdLinkDst);
	}
}

void MystScriptParser::o_changeCardPlaySoundDirectional(uint16 var, const ArgumentsArray &args) {
	uint16 cardId = args[0];
	uint16 soundId = args[1];
	uint16 delayBetweenSteps = args[2];
	uint16 dataSize = args[3];

	debugC(kDebugScript, "\tcard: %d", cardId);
	debugC(kDebugScript, "\tsound: %d", soundId);
	debugC(kDebugScript, "\tdelay between steps: %d", delayBetweenSteps);
	debugC(kDebugScript, "\tanimated update data size: %d", dataSize);

	_vm->changeToCard(cardId, kNoTransition);

	if (soundId)
		_vm->_sound->playEffect(soundId);

	animatedUpdate(ArgumentsArray(args.begin() + 4, dataSize), delayBetweenSteps);
}

void MystScriptParser::o_directionalUpdatePlaySound(uint16 var, const ArgumentsArray &args) {
	uint16 soundId = args[0];
	uint16 delayBetweenSteps = args[1];
	uint16 dataSize = args[2];

	debugC(kDebugScript, "\tsound: %d", soundId);
	debugC(kDebugScript, "\tdelay between steps: %d", delayBetweenSteps);
	debugC(kDebugScript, "\tanimated update data size: %d", dataSize);

	if (soundId)
		_vm->_sound->playEffect(soundId);

	animatedUpdate(ArgumentsArray(args.begin() + 3, dataSize), delayBetweenSteps);
}

void MystScriptParser::o_saveMainCursor(uint16 var, const ArgumentsArray &args) {
	_savedCursorId = _vm->getMainCursor();
}

void MystScriptParser::o_restoreMainCursor(uint16 var, const ArgumentsArray &args) {
	_vm->setMainCursor(_savedCursorId);
}

void MystScriptParser::o_soundWaitStop(uint16 var, const ArgumentsArray &args) {
	// Used on Selenitic Card 1191 (Maze Runner)
	// Used on Mechanical Card 6267 (Code Lock)
	// Used when Button is pushed...
	soundWaitStop();
}

void MystScriptParser::soundWaitStop() const {
	while (_vm->_sound->isEffectPlaying() && !Engine::shouldQuit())
		_vm->doFrame();
}

void MystScriptParser::o_quit(uint16 var, const ArgumentsArray &args) {
	Engine::quitGame();
}

void MystScriptParser::showMap() {
	if (_vm->getCard()->getId() != getMap()) {
		_savedMapCardId = _vm->getCard()->getId();
		_vm->changeToCard(getMap(), kTransitionCopy);
	}
}

void MystScriptParser::o_exitMap(uint16 var, const ArgumentsArray &args) {
	assert(_savedMapCardId);

	_vm->changeToCard(_savedMapCardId, kTransitionCopy);
}

} // End of namespace Mohawk
