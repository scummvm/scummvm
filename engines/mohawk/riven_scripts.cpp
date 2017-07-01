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
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_scripts.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_stack.h"
#include "mohawk/riven_video.h"
#include "common/memstream.h"

#include "common/debug-channels.h"
#include "common/stream.h"
#include "common/system.h"

namespace Mohawk {

static void printTabs(byte tabs) {
	for (byte i = 0; i < tabs; i++)
		debugN("\t");
}

RivenScriptManager::RivenScriptManager(MohawkEngine_Riven *vm) :
		_vm(vm),
		_runningQueuedScripts(false),
		_stoppingAllScripts(false) {

	_storedMovieOpcode.time = 0;
	_storedMovieOpcode.id = 0;
}

RivenScriptManager::~RivenScriptManager() {
	clearStoredMovieOpcode();
}

RivenScriptPtr RivenScriptManager::readScript(Common::ReadStream *stream) {
	RivenScriptPtr script = RivenScriptPtr(new RivenScript());

	uint16 commandCount = stream->readUint16BE();

	for (uint16 i = 0; i < commandCount; i++) {
		RivenCommandPtr command = readCommand(stream);
		script->addCommand(command);
	}

	return script;
}

RivenCommandPtr RivenScriptManager::readCommand(Common::ReadStream *stream) {
	RivenCommandType type = (RivenCommandType) stream->readUint16BE();

	switch (type) {
		case kRivenCommandSwitch:
			return RivenCommandPtr(RivenSwitchCommand::createFromStream(_vm, stream));
		case kRivenCommandChangeStack:
			return RivenCommandPtr(RivenStackChangeCommand::createFromStream(_vm, stream));
		default:
			return RivenCommandPtr(RivenSimpleCommand::createFromStream(_vm, type, stream));
	}
}

RivenScriptList RivenScriptManager::readScripts(Common::ReadStream *stream) {
	RivenScriptList scriptList;

	uint16 scriptCount = stream->readUint16BE();
	for (uint16 i = 0; i < scriptCount; i++) {
		RivenTypedScript script;
		script.type = stream->readUint16BE();
		script.script = readScript(stream);
		scriptList.push_back(script);
	}

	return scriptList;
}

void RivenScriptManager::stopAllScripts() {
	_stoppingAllScripts = true;
}

void RivenScriptManager::setStoredMovieOpcode(const StoredMovieOpcode &op) {
	clearStoredMovieOpcode();
	_storedMovieOpcode.script = op.script;
	_storedMovieOpcode.id = op.id;
	_storedMovieOpcode.time = op.time;
}

void RivenScriptManager::runStoredMovieOpcode() {
	if (_storedMovieOpcode.script) {
		runScript(_storedMovieOpcode.script, false);
		clearStoredMovieOpcode();
	}
}

void RivenScriptManager::clearStoredMovieOpcode() {
	_storedMovieOpcode.script = RivenScriptPtr();
	_storedMovieOpcode.time = 0;
	_storedMovieOpcode.id = 0;
}

void RivenScriptManager::runScript(const RivenScriptPtr &script, bool queue) {
	if (!script || script->empty()) {
		return;
	}

	if (!queue) {
		script->run(this);
	} else {
		_queue.push_back(script);
	}
}

bool RivenScriptManager::hasQueuedScripts() const {
	return !_queue.empty();
}

void RivenScriptManager::runQueuedScripts() {
	_runningQueuedScripts = true;

	for (uint i = 0; i < _queue.size(); i++) {
		_queue[i]->run(this);
	}

	_queue.clear();

	_stoppingAllScripts = false; // Once the queue is empty, all scripts have been stopped
	_runningQueuedScripts = false;
}

RivenScriptPtr RivenScriptManager::createScriptFromData(uint16 commandCount, ...) {
	va_list args;
	va_start(args, commandCount);

	// Build a script from the variadic arguments
	Common::MemoryWriteStreamDynamic writeStream = Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	writeStream.writeUint16BE(commandCount);

	for (uint i = 0; i < commandCount; i++) {
		uint16 command = va_arg(args, int);
		writeStream.writeUint16BE(command);

		if (command == kRivenCommandSwitch) {
			// The switch command has a different format that is not implemented
			error("Cannot create a Switch command from data");
		}

		uint16 argumentCount = va_arg(args, int);
		writeStream.writeUint16BE(argumentCount);

		for (uint j = 0; j < commandCount; j++) {
			uint16 argument = va_arg(args, int);
			writeStream.writeUint16BE(argument);
		}
	}

	va_end(args);

	Common::MemoryReadStream readStream = Common::MemoryReadStream(writeStream.getData(), writeStream.size());
	return readScript(&readStream);
}

RivenScriptPtr RivenScriptManager::createScriptWithCommand(RivenCommand *command) {
	assert(command);

	RivenScriptPtr script = RivenScriptPtr(new RivenScript());
	script->addCommand(RivenCommandPtr(command));
	return script;
}

bool RivenScriptManager::runningQueuedScripts() const {
	return _runningQueuedScripts;
}

bool RivenScriptManager::stoppingAllScripts() const {
	return _stoppingAllScripts;
}

RivenScript::RivenScript() {
}

RivenScript::~RivenScript() {
}

void RivenScript::dumpScript(byte tabs) {
	for (uint16 i = 0; i < _commands.size(); i++) {
		_commands[i]->dump(tabs);
	}
}

void RivenScript::run(RivenScriptManager *scriptManager) {
	for (uint i = 0; i < _commands.size(); i++) {
		if (scriptManager->stoppingAllScripts()) {
			return;
		}

		_commands[i]->execute();
	}
}

void RivenScript::addCommand(RivenCommandPtr command) {
	_commands.push_back(command);
}

bool RivenScript::empty() const {
	return _commands.empty();
}

RivenScript &RivenScript::operator+=(const RivenScript &other) {
	_commands.push_back(other._commands);
	return *this;
}

const char *RivenScript::getTypeName(uint16 type) {
	static const char *names[] = {
		"MouseDown",
		"MouseDrag",
		"MouseUp",
		"MouseEnter",
		"MouseInside",
		"MouseLeave",
		"CardLoad",
		"CardLeave",
		"CardUnknown",
		"CardEnter",
		"CardUpdate"
	};

	assert(type < ARRAYSIZE(names));
	return names[type];
}

RivenScriptPtr &operator+=(RivenScriptPtr &lhs, const RivenScriptPtr &rhs) {
	if (rhs) {
		*lhs += *rhs;
	}
	return lhs;
}

RivenCommand::RivenCommand(MohawkEngine_Riven *vm) :
		_vm(vm) {

}

RivenCommand::~RivenCommand() {

}

RivenSimpleCommand::RivenSimpleCommand(MohawkEngine_Riven *vm, RivenCommandType type, const ArgumentArray &arguments) :
		RivenCommand(vm),
		_type(type),
		_arguments(arguments) {
	setupOpcodes();
}

RivenSimpleCommand::~RivenSimpleCommand() {
}

RivenSimpleCommand *RivenSimpleCommand::createFromStream(MohawkEngine_Riven *vm, RivenCommandType type, Common::ReadStream *stream) {
	uint16 argCount = stream->readUint16BE();

	Common::Array<uint16> arguments;
	arguments.resize(argCount);

	for (uint16 i = 0; i < argCount; i++) {
		arguments[i] = stream->readUint16BE();
	}

	return new RivenSimpleCommand(vm, type, arguments);
}

#define OPCODE(x) { &RivenSimpleCommand::x, #x }

void RivenSimpleCommand::setupOpcodes() {
	static const RivenOpcode riven_opcodes[] = {
		// 0x00 (0 decimal)
		OPCODE(empty),
		OPCODE(drawBitmap),
		OPCODE(switchCard),
		OPCODE(playScriptSLST),
		// 0x04 (4 decimal)
		OPCODE(playSound),
		OPCODE(empty),						// Empty
		OPCODE(empty),						// Complex animation (not used)
		OPCODE(setVariable),
		// 0x08 (8 decimal)
		OPCODE(empty),                      // Not a SimpleCommand
		OPCODE(enableHotspot),
		OPCODE(disableHotspot),
		OPCODE(empty),						// Empty
		// 0x0C (12 decimal)
		OPCODE(stopSound),
		OPCODE(changeCursor),
		OPCODE(delay),
		OPCODE(empty),						// Empty
		// 0x10 (16 decimal)
		OPCODE(empty),						// Empty
		OPCODE(runExternalCommand),
		OPCODE(transition),
		OPCODE(refreshCard),
		// 0x14 (20 decimal)
		OPCODE(beginScreenUpdate),
		OPCODE(applyScreenUpdate),
		OPCODE(empty),						// Empty
		OPCODE(empty),						// Empty
		// 0x18 (24 decimal)
		OPCODE(incrementVariable),
		OPCODE(empty),						// Empty
		OPCODE(empty),						// Empty
		OPCODE(empty),                      // Not a SimpleCommand
		// 0x1C (28 decimal)
		OPCODE(disableMovie),
		OPCODE(disableAllMovies),
		OPCODE(empty),						// Set movie rate (not used)
		OPCODE(enableMovie),
		// 0x20 (32 decimal)
		OPCODE(playMovieBlocking),
		OPCODE(playMovie),
		OPCODE(stopMovie),
		OPCODE(empty),						// Start a water effect (not used)
		// 0x24 (36 decimal)
		OPCODE(unk_36),						// Unknown
		OPCODE(fadeAmbientSounds),
		OPCODE(storeMovieOpcode),
		OPCODE(activatePLST),
		// 0x28 (40 decimal)
		OPCODE(activateSLST),
		OPCODE(activateMLSTAndPlay),
		OPCODE(empty),						// Empty
		OPCODE(activateBLST),
		// 0x2C (44 decimal)
		OPCODE(activateFLST),
		OPCODE(zipMode),
		OPCODE(activateMLST),
		OPCODE(empty)                       // Activate an SLST with a volume parameter (not used)
	};

	_opcodes = riven_opcodes;
}

////////////////////////////////
// Opcodes
////////////////////////////////

// Command 1: draw tBMP resource (tbmp_id, left, top, right, bottom, u0, u1, u2, u3)
void RivenSimpleCommand::drawBitmap(uint16 op, uint16 argc, uint16 *argv) {
	if (argc < 5) // Copy the image to the whole screen, ignoring the rest of the parameters
		_vm->_gfx->copyImageToScreen(argv[0], 0, 0, 608, 392);
	else          // Copy the image to a certain part of the screen
		_vm->_gfx->copyImageToScreen(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

// Command 2: go to card (card id)
void RivenSimpleCommand::switchCard(uint16 op, uint16 argc, uint16 *argv) {
	_vm->changeToCard(argv[0]);
}

// Command 3: play an SLST from the script
void RivenSimpleCommand::playScriptSLST(uint16 op, uint16 argc, uint16 *argv) {
	int offset = 0, j = 0;
	uint16 soundCount = argv[offset++];

	SLSTRecord slstRecord;
	slstRecord.index = 0;		// not set by the scripts, so we set it to 0
	slstRecord.soundIds.resize(soundCount);

	for (j = 0; j < soundCount; j++)
		slstRecord.soundIds[j] = argv[offset++];
	slstRecord.fadeFlags = argv[offset++];
	slstRecord.loop = argv[offset++];
	slstRecord.globalVolume = argv[offset++];
	slstRecord.u0 = argv[offset++];
	slstRecord.suspend = argv[offset++];

	slstRecord.volumes.resize(soundCount);
	slstRecord.balances.resize(soundCount);
	slstRecord.u2.resize(soundCount);

	for (j = 0; j < soundCount; j++)
		slstRecord.volumes[j] = argv[offset++];

	for (j = 0; j < soundCount; j++)
		slstRecord.balances[j] = argv[offset++];	// negative = left, 0 = center, positive = right

	for (j = 0; j < soundCount; j++)
		slstRecord.u2[j] = argv[offset++];			// Unknown

	// Play the requested sound list
	_vm->_sound->playSLST(slstRecord);
}

// Command 4: play local tWAV resource (twav_id, volume, block)
void RivenSimpleCommand::playSound(uint16 op, uint16 argc, uint16 *argv) {
	uint16 volume = argv[1];
	bool playOnDraw = argv[2] == 1;

	_vm->_sound->playSound(argv[0], volume, playOnDraw);
}

// Command 7: set variable value (variable, value)
void RivenSimpleCommand::setVariable(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getStackVar(argv[0]) = argv[1];
}

// Command 9: enable hotspot (blst_id)
void RivenSimpleCommand::enableHotspot(uint16 op, uint16 argc, uint16 *argv) {
	RivenHotspot *hotspot = _vm->getCard()->getHotspotByBlstId(argv[0]);
	if (hotspot) {
		hotspot->enable(true);
	}
}

// Command 10: disable hotspot (blst_id)
void RivenSimpleCommand::disableHotspot(uint16 op, uint16 argc, uint16 *argv) {
	RivenHotspot *hotspot = _vm->getCard()->getHotspotByBlstId(argv[0]);
	if (hotspot) {
		hotspot->enable(false);
	}
}

// Command 12: stop sounds (flags)
void RivenSimpleCommand::stopSound(uint16 op, uint16 argc, uint16 *argv) {
	// WORKAROUND: The Play Riven/Visit Riven/Start New Game buttons
	// in the main menu call this function to stop ambient sounds
	// after the change stack call to Temple Island. However, this
	// would cause all ambient sounds not to play. An alternative
	// fix would be to stop all scripts on a stack change, but this
	// does fine for now.
	if (_vm->getStack()->getId() == kStackTspit && (_vm->getStack()->getCurrentCardGlobalId() == 0x6e9a ||
			_vm->getStack()->getCurrentCardGlobalId() == 0xfeeb))
		return;

	// The argument is a bitflag for the setting.
	// bit 0 is normal sound stopping
	// bit 1 is ambient sound stopping
	// Having no flags set means clear all
	if (argv[0] & 2 || argv[0] == 0)
		_vm->_sound->stopAllSLST();

	if (argv[0] & 1 || argv[0] == 0)
		_vm->_sound->stopSound();
}

// Command 13: set mouse cursor (cursor_id)
void RivenSimpleCommand::changeCursor(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_cursor->setCursor(argv[0]);
}

// Command 14: pause script execution (delay in ms, u1)
void RivenSimpleCommand::delay(uint16 op, uint16 argc, uint16 *argv) {
	if (argv[0] > 0)
		_vm->delay(argv[0]);
}

// Command 17: call external command
void RivenSimpleCommand::runExternalCommand(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getStack()->runCommand(argc, argv);
}

// Command 18: transition
// Note that this opcode has 1 or 5 parameters, depending on argc
// Parameter 0: transition type
// Parameters 1-4: transition rectangle
void RivenSimpleCommand::transition(uint16 op, uint16 argc, uint16 *argv) {
	if (argc == 1)
		_vm->_gfx->scheduleTransition((RivenTransition) argv[0]);
	else
		_vm->_gfx->scheduleTransition((RivenTransition) argv[0], Common::Rect(argv[1], argv[2], argv[3], argv[4]));
}

// Command 19: reload card
void RivenSimpleCommand::refreshCard(uint16 op, uint16 argc, uint16 *argv) {
	_vm->refreshCard();
}

// Command 20: begin screen update
void RivenSimpleCommand::beginScreenUpdate(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_gfx->beginScreenUpdate();
}

// Command 21: apply screen update
void RivenSimpleCommand::applyScreenUpdate(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_gfx->applyScreenUpdate();
}

// Command 24: increment variable (variable, value)
void RivenSimpleCommand::incrementVariable(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getStackVar(argv[0]) += argv[1];
}

// Command 28: disable a movie
void RivenSimpleCommand::disableMovie(uint16 op, uint16 argc, uint16 *argv) {
	RivenVideo *video = _vm->_video->openSlot(argv[0]);
	if (video)
		video->disable();
}

// Command 29: disable all movies
void RivenSimpleCommand::disableAllMovies(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->disableAllMovies();
}

// Command 31: enable a movie
void RivenSimpleCommand::enableMovie(uint16 op, uint16 argc, uint16 *argv) {
	RivenVideo *video = _vm->_video->openSlot(argv[0]);
	video->enable();
}

// Command 32: play foreground movie - blocking (movie_id)
void RivenSimpleCommand::playMovieBlocking(uint16 op, uint16 argc, uint16 *argv) {
	RivenVideo *video = _vm->_video->openSlot(argv[0]);
	video->setLooping(false);
	video->enable();
	video->playBlocking();
}

// Command 33: play background movie - nonblocking (movie_id)
void RivenSimpleCommand::playMovie(uint16 op, uint16 argc, uint16 *argv) {
	RivenVideo *video = _vm->_video->openSlot(argv[0]);
	video->enable();
	video->play();
}

// Command 34: stop a movie
void RivenSimpleCommand::stopMovie(uint16 op, uint16 argc, uint16 *argv) {
	RivenVideo *video = _vm->_video->openSlot(argv[0]);
	video->stop();
}

// Command 36: unknown
void RivenSimpleCommand::unk_36(uint16 op, uint16 argc, uint16 *argv) {
}

// Command 37: fade ambient sounds
void RivenSimpleCommand::fadeAmbientSounds(uint16 op, uint16 argc, uint16 *argv) {
	// Similar to stopSound(), but does fading
	_vm->_sound->stopAllSLST(true);
}

// Command 38: Store an opcode for use when playing a movie (movie id, time high, time low, opcode, arguments...)
void RivenSimpleCommand::storeMovieOpcode(uint16 op, uint16 argc, uint16 *argv) {
	// This opcode is used to delay an opcode's usage based on the elapsed
	// time of a specified movie. However, every use in the game is for
	// delaying an activateSLST opcode.

	uint32 delayTime = (argv[1] << 16) + argv[2];

	// Store the script
	RivenScriptManager::StoredMovieOpcode storedOp;
	storedOp.script = _vm->_scriptMan->createScriptFromData(1, argv[3], 1, argv[4]);
	storedOp.time = delayTime;
	storedOp.id = argv[0];

	// Store the opcode for later
	_vm->_scriptMan->setStoredMovieOpcode(storedOp);
}

// Command 39: activate PLST record (card picture lists)
void RivenSimpleCommand::activatePLST(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_activatedPLST = true;

	RivenCard::Picture picture = _vm->getCard()->getPicture(argv[0]);
	_vm->_gfx->copyImageToScreen(picture.id, picture.rect.left, picture.rect.top, picture.rect.right, picture.rect.bottom);
}

// Command 40: activate SLST record (card ambient sound lists)
void RivenSimpleCommand::activateSLST(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_activatedSLST = true;

	SLSTRecord slstRecord = _vm->getCard()->getSound(argv[0]);
	_vm->_sound->playSLST(slstRecord);
}

// Command 41: activate MLST record and play
void RivenSimpleCommand::activateMLSTAndPlay(uint16 op, uint16 argc, uint16 *argv) {
	MLSTRecord mlstRecord = _vm->getCard()->getMovie(argv[0]);
	activateMLST(mlstRecord);

	RivenVideo *video = _vm->_video->openSlot(mlstRecord.playbackSlot);
	video->enable();
	video->play();
}

// Command 43: activate BLST record (card hotspot enabling lists)
void RivenSimpleCommand::activateBLST(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getCard()->activateHotspotEnableRecord(argv[0]);
}

// Command 44: activate FLST record (information on which SFXE resource this card should use)
void RivenSimpleCommand::activateFLST(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getCard()->activateWaterEffect(argv[0]);
}

// Command 45: do zip mode
void RivenSimpleCommand::zipMode(uint16 op, uint16 argc, uint16 *argv) {
	assert(_vm->getCard() && _vm->getCard()->getCurHotspot());

	// Check the ZIPS records to see if we have a match to the hotspot name
	Common::String hotspotName = _vm->getCard()->getCurHotspot()->getName();

	for (uint16 i = 0; i < _vm->_zipModeData.size(); i++)
		if (_vm->_zipModeData[i].name == hotspotName) {
			_vm->changeToCard(_vm->_zipModeData[i].id);
			return;
		}
}

// Command 46: activate MLST record (movie lists)
void RivenSimpleCommand::activateMLST(uint16 op, uint16 argc, uint16 *argv) {
	MLSTRecord mlstRecord = _vm->getCard()->getMovie(argv[0]);
	activateMLST(mlstRecord);
}

void RivenSimpleCommand::activateMLST(const MLSTRecord &mlstRecord) const {
	RivenVideo *ptr = _vm->_video->openSlot(mlstRecord.playbackSlot);
	ptr->load(mlstRecord.movieID);
	ptr->moveTo(mlstRecord.left, mlstRecord.top);
	ptr->setLooping(mlstRecord.loop != 0);
	ptr->setVolume(mlstRecord.volume);
}

Common::String RivenSimpleCommand::describe() const {
	Common::String desc;

	if (_type == kRivenCommandSwitch) { // Use the variable name
		Common::String varName = _vm->getStack()->getName(kVariableNames, _arguments[0]);
		desc = Common::String::format("%s = %d", varName.c_str(), _arguments[1]);
	} else if (_type == kRivenCommandRunExternal) { // Use the external command name
		Common::String externalCommandName = _vm->getStack()->getName(kExternalCommandNames, _arguments[0]);
		desc = Common::String::format("%s(", externalCommandName.c_str());
		uint16 varCount = _arguments[1];
		for (uint16 j = 0; j < varCount; j++) {
			desc += Common::String::format("%d", _arguments[2 + j]);
			if (j != varCount - 1)
				desc += ", ";
		}
		desc += ")";
	} else if (_type == kRivenCommandIncrementVariable) { // Use the variable name
		Common::String varName = _vm->getStack()->getName(kVariableNames, _arguments[0]);
		desc = Common::String::format("%s += %d", varName.c_str(), _arguments[1]);
	} else if (_type == kRivenCommandSetVariable) { // Use the variable name
		Common::String varName = _vm->getStack()->getName(kVariableNames, _arguments[0]);
		desc = Common::String::format("%s = %d", varName.c_str(), _arguments[1]);
	} else {
		desc = Common::String::format("%s(", _opcodes[_type].desc);
		for (uint16 j = 0; j < _arguments.size(); j++) {
			desc += Common::String::format("%d", _arguments[j]);
			if (j != _arguments.size() - 1)
				desc += ", ";
		}
		desc += ")";
	}

	return desc;
}

void RivenSimpleCommand::dump(byte tabs) {
	printTabs(tabs);
	debugN("%s;\n", describe().c_str());
}

void RivenSimpleCommand::execute() {
	if (DebugMan.isDebugChannelEnabled(kRivenDebugScript)) {
		debugC(kRivenDebugScript, "Running opcode: %s", describe().c_str());
	}

	uint16 *argValues = new uint16[_arguments.size()];

	for (uint16 k = 0; k < _arguments.size(); k++)
		argValues[k] = _arguments[k];

	(this->*(_opcodes[_type].proc)) (_type, _arguments.size(), argValues);

	delete[] argValues;
}

RivenSwitchCommand::RivenSwitchCommand(MohawkEngine_Riven *vm) :
		RivenCommand(vm),
		_variableId(0) {

}

RivenSwitchCommand::~RivenSwitchCommand() {

}

RivenSwitchCommand *RivenSwitchCommand::createFromStream(MohawkEngine_Riven *vm, Common::ReadStream *stream) {
	RivenSwitchCommand *command = new RivenSwitchCommand(vm);

	if (stream->readUint16BE() != 2) {
		// This value is not used in the original engine
		warning("if-then-else unknown value is not 2");
	}

	// variable to check against
	command->_variableId = stream->readUint16BE();

	// number of logic blocks
	uint16 logicBlockCount = stream->readUint16BE();
	command->_branches.resize(logicBlockCount);

	for (uint16 i = 0; i < logicBlockCount; i++) {
		Branch &branch = command->_branches[i];

		// Value for this logic block
		branch.value = stream->readUint16BE();
		branch.script = vm->_scriptMan->readScript(stream);
	}

	return command;
}

void RivenSwitchCommand::dump(byte tabs) {
	Common::String varName = _vm->getStack()->getName(kVariableNames, _variableId);
	printTabs(tabs); debugN("switch (%s) {\n", varName.c_str());
	for (uint16 j = 0; j < _branches.size(); j++) {
		printTabs(tabs + 1);
		if (_branches[j].value == 0xFFFF)
			debugN("default:\n");
		else
			debugN("case %d:\n", _branches[j].value);
		_branches[j].script->dumpScript(tabs + 2);
		printTabs(tabs + 2); debugN("break;\n");
	}
	printTabs(tabs); debugN("}\n");
}

void RivenSwitchCommand::execute() {
	if (DebugMan.isDebugChannelEnabled(kRivenDebugScript)) {
		Common::String varName = _vm->getStack()->getName(kVariableNames, _variableId);
		debugC(kRivenDebugScript, "Running opcode: switch(%s)", varName.c_str());
	}

	// Get the switch variable value
	uint32 value = _vm->getStackVar(_variableId);

	// Look for a case matching the value
	for (uint i = 0; i < _branches.size(); i++) {
		if  (_branches[i].value == value) {
			_vm->_scriptMan->runScript(_branches[i].script, false);
			return;
		}
	}

	// Look for the default case if any
	for (uint i = 0; i < _branches.size(); i++) {
		if  (_branches[i].value == 0Xffff) {
			_vm->_scriptMan->runScript(_branches[i].script, false);
			return;
		}
	}
}

RivenStackChangeCommand::RivenStackChangeCommand(MohawkEngine_Riven *vm, uint16 stackId, uint32 globalCardId, bool byStackId) :
		RivenCommand(vm),
		_stackId(stackId),
		_cardId(globalCardId),
		_byStackId(byStackId) {

}

RivenStackChangeCommand::~RivenStackChangeCommand() {

}

RivenStackChangeCommand *RivenStackChangeCommand::createFromStream(MohawkEngine_Riven *vm, Common::ReadStream *stream) {
	/* argumentsSize = */ stream->readUint16BE();
	uint16 stackId = stream->readUint16BE();
	uint32 globalCardId = stream->readUint32BE();

	return new RivenStackChangeCommand(vm, stackId, globalCardId, false);
}

void RivenStackChangeCommand::execute() {
	debugC(kRivenDebugScript, "Running opcode: changeStack(%d, %d)", _stackId, _cardId);

	uint16 stackID;
	if (_byStackId) {
		stackID = _stackId;
	} else {
		Common::String stackName = _vm->getStack()->getName(kStackNames, _stackId);

		stackID = RivenStacks::getId(stackName.c_str());
		if (stackID == kStackUnknown) {
			error ("'%s' is not a stack name!", stackName.c_str());
		}
	}

	_vm->changeToStack(stackID);
	uint16 cardID = _vm->getStack()->getCardStackId(_cardId);
	_vm->changeToCard(cardID);
}

void RivenStackChangeCommand::dump(byte tabs) {
	printTabs(tabs);
	debugN("changeStack(%d, %d);\n", _stackId, _cardId);
}

RivenTimerCommand::RivenTimerCommand(MohawkEngine_Riven *vm, const Common::SharedPtr<RivenStack::TimerProc> &timerProc) :
	RivenCommand(vm),
	_timerProc(timerProc) {

}

void RivenTimerCommand::execute() {
	(*_timerProc)();
}

void RivenTimerCommand::dump(byte tabs) {
	printTabs(tabs);
	debugN("doTimer();\n");
}

} // End of namespace Mohawk
