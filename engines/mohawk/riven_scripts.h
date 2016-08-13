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

#ifndef RIVEN_SCRIPTS_H
#define RIVEN_SCRIPTS_H

#include "common/str-array.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 argc, uint16 *argv)

namespace Common {
class ReadStream;
}

namespace Mohawk {

// Script Types
enum {
	kMouseDownScript = 0,
	kMouseDragScript = 1,
	kMouseUpScript = 2,
	kMouseEnterScript = 3,
	kMouseInsideScript = 4,
	kMouseLeaveScript = 5,

	kCardLoadScript = 6,
	kCardLeaveScript = 7,
	kCardOpenScript = 9,
	kCardUpdateScript = 10
};

class MohawkEngine_Riven;
class RivenCommand;
class RivenScript;

typedef Common::SharedPtr<RivenScript> RivenScriptPtr;

/**
 * Scripts in Riven are a list of Commands
 *
 * This class should only be used through the RivenScriptPtr
 * type to ensure the underlying memory is not freed when changing card.
 */
class RivenScript {
public:
	RivenScript();
	~RivenScript();

	/** Append a command to the script */
	void addCommand(RivenCommand *command);

	/** True if the script does not contain any command */
	bool empty() const;

	/**
	 * Run the script
	 *
	 * Script execution must go through the ScriptManager,
	 * this method should not be called directly.
	 */
	void run();

	/** Print script details to the standard output */
	void dumpScript(byte tabs);

	/** Stop the script after the current command */
	void stopRunning() { _continueRunning = false; }

	/** Append the commands of the other script to this script */
	RivenScript &operator+=(const RivenScript &other);

	/** Get a caption for a script type */
	static const char *getTypeName(uint16 type);

private:
	Common::Array<RivenCommand *> _commands;
	bool _continueRunning;
};

/** Append the commands of the rhs Script to those of the lhs Script */
RivenScriptPtr &operator+=(RivenScriptPtr &lhs, const RivenScriptPtr &rhs);

/**
 * A script and its type
 *
 * The type defines when the script should be run
 */
struct RivenTypedScript {
	uint16 type;
	RivenScriptPtr script;
};

typedef Common::Array<RivenTypedScript> RivenScriptList;

/**
 * Script manager
 *
 * Reads scripts from raw data.
 * Can run scripts immediatly, or store them for future execution.
 */
class RivenScriptManager {
public:
	RivenScriptManager(MohawkEngine_Riven *vm);
	~RivenScriptManager();

	/** Read a single script from a stream */
	RivenScriptPtr readScript(Common::ReadStream *stream);

	/** Create a script from the caller provided arguments containing raw data */
	RivenScriptPtr createScriptFromData(uint16 commandCount, ...);

	/** Read a list of typed scripts from a stream */
	RivenScriptList readScripts(Common::ReadStream *stream);

	/** Run a script */
	void runScript(const RivenScriptPtr &script, bool queue);

	void stopAllScripts();

	struct StoredMovieOpcode {
		RivenScriptPtr script;
		uint32 time;
		uint16 id;
	};

	uint16 getStoredMovieOpcodeID() { return _storedMovieOpcode.id; }
	uint32 getStoredMovieOpcodeTime() { return _storedMovieOpcode.time; }
	void setStoredMovieOpcode(const StoredMovieOpcode &op);
	void runStoredMovieOpcode();
	void clearStoredMovieOpcode();

private:
	MohawkEngine_Riven *_vm;

	Common::Array<RivenScriptPtr> _queue;
	StoredMovieOpcode _storedMovieOpcode;

	RivenCommand *readCommand(Common::ReadStream *stream);
};

/**
 * An abstract command
 *
 * Commands are unit operations part of a script
 */
class RivenCommand {
public:
	RivenCommand(MohawkEngine_Riven *vm);
	virtual ~RivenCommand();

	/** Print details about the command to standard output */
	virtual void dump(byte tabs) = 0;

	/** Execute the command */
	virtual void execute() = 0;

protected:
	MohawkEngine_Riven *_vm;
};

/**
 * A simple Command
 *
 * Simple commands have a type and a list of arguments.
 * The operation to be executed when running the command
 * depends on the type.
 */
class RivenSimpleCommand : public RivenCommand {
public:
	static RivenSimpleCommand *createFromStream(MohawkEngine_Riven *vm, int type, Common::ReadStream *stream);
	virtual ~RivenSimpleCommand();

	// RivenCommand API
	virtual void dump(byte tabs) override;
	virtual void execute() override;

private:
	typedef Common::Array<uint16> ArgumentArray;
	typedef void (RivenSimpleCommand::*OpcodeProcRiven)(uint16 op, uint16 argc, uint16 *argv);
	struct RivenOpcode {
		OpcodeProcRiven proc;
		const char *desc;
	};

	RivenSimpleCommand(MohawkEngine_Riven *vm, int type, const ArgumentArray &arguments);

	void setupOpcodes();

	DECLARE_OPCODE(empty) { warning ("Unknown Opcode %04x", op); }

	// Opcodes
	DECLARE_OPCODE(drawBitmap);
	DECLARE_OPCODE(switchCard);
	DECLARE_OPCODE(playScriptSLST);
	DECLARE_OPCODE(playSound);
	DECLARE_OPCODE(setVariable);
	DECLARE_OPCODE(mohawkSwitch);
	DECLARE_OPCODE(enableHotspot);
	DECLARE_OPCODE(disableHotspot);
	DECLARE_OPCODE(stopSound);
	DECLARE_OPCODE(changeCursor);
	DECLARE_OPCODE(delay);
	DECLARE_OPCODE(runExternalCommand);
	DECLARE_OPCODE(transition);
	DECLARE_OPCODE(refreshCard);
	DECLARE_OPCODE(beginScreenUpdate);
	DECLARE_OPCODE(applyScreenUpdate);
	DECLARE_OPCODE(incrementVariable);
	DECLARE_OPCODE(changeStack);
	DECLARE_OPCODE(disableMovie);
	DECLARE_OPCODE(disableAllMovies);
	DECLARE_OPCODE(enableMovie);
	DECLARE_OPCODE(playMovieBlocking);
	DECLARE_OPCODE(playMovie);
	DECLARE_OPCODE(stopMovie);
	DECLARE_OPCODE(unk_36);
	DECLARE_OPCODE(fadeAmbientSounds);
	DECLARE_OPCODE(storeMovieOpcode);
	DECLARE_OPCODE(activatePLST);
	DECLARE_OPCODE(activateSLST);
	DECLARE_OPCODE(activateMLSTAndPlay);
	DECLARE_OPCODE(activateBLST);
	DECLARE_OPCODE(activateFLST);
	DECLARE_OPCODE(zipMode);
	DECLARE_OPCODE(activateMLST);

	const RivenOpcode *_opcodes;

	int _type;
	ArgumentArray _arguments;
};

/**
 * A switch branch command
 *
 * Switch commands have a variable id and a list of branches.
 * Each branch associates a value to a script.
 * The branch matching the variable's value is executed,
 * if not found an optional default branch can be executed.
 */
class RivenSwitchCommand : public RivenCommand {
public:
	static RivenSwitchCommand *createFromStream(MohawkEngine_Riven *vm, int type, Common::ReadStream *stream);
	virtual ~RivenSwitchCommand();

	// RivenCommand API
	virtual void dump(byte tabs) override;
	virtual void execute() override;

private:
	RivenSwitchCommand(MohawkEngine_Riven *vm);

	struct Branch {
		uint16 value;
		RivenScriptPtr script;
	};

	uint16 _variableId;
	Common::Array<Branch> _branches;
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
