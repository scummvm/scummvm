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

#include "mohawk/riven_stack.h"

#include "common/str-array.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#define DECLARE_OPCODE(x) void x(uint16 op, const ArgumentArray &args)

namespace Common {
class ReadStream;
}

namespace Mohawk {

// Script Types
enum {
	kMouseDownScript   = 0,
	kMouseDragScript   = 1,
	kMouseUpScript     = 2,
	kMouseEnterScript  = 3,
	kMouseInsideScript = 4,
	kMouseLeaveScript  = 5,

	kCardLoadScript    = 6,
	kCardLeaveScript   = 7,
	kCardFrameScript   = 8,
	kCardEnterScript   = 9,
	kCardUpdateScript  = 10
};

enum RivenCommandType {
	kRivenCommandDrawBitmap          = 1,
	kRivenCommandChangeCard          = 2,
	kRivenCommandPlayScriptSLST      = 3,
	kRivenCommandPlaySound           = 4,
	kRivenCommandSetVariable         = 7,
	kRivenCommandSwitch              = 8,
	kRivenCommandEnableHotspot       = 9,
	kRivenCommandDisableHotspot      = 10,
	kRivenCommandStopSound           = 12,
	kRivenCommandChangeCursor        = 13,
	kRivenCommandDelay               = 14,
	kRivenCommandRunExternal         = 17,
	kRivenCommandTransition          = 18,
	kRivenCommandRefreshCard         = 19,
	kRivenCommandBeginScreenUpdate   = 20,
	kRivenCommandApplyScreenUpdate   = 21,
	kRivenCommandIncrementVariable   = 24,
	kRivenCommandChangeStack         = 27,
	kRivenCommandDisableMovie        = 28,
	kRivenCommandDisableAllMovies    = 29,
	kRivenCommandEnableMovie         = 31,
	kRivenCommandPlayMovieBlocking   = 32,
	kRivenCommandPlayMovie           = 33,
	kRivenCommandStopMovie           = 34,
	kRivenCommandUnk36               = 36,
	kRivenCommandFadeAmbientSounds   = 37,
	kRivenCommandStoreMovieOpcode    = 38,
	kRivenCommandActivatePLST        = 39,
	kRivenCommandActivateSLST        = 40,
	kRivenCommandActivateMLSTAndPlay = 41,
	kRivenCommandActivateBLST        = 43,
	kRivenCommandActivateFLST        = 44,
	kRivenCommandZipMode             = 45,
	kRivenCommandActivateMLST        = 46,
	kRivenCommandTimer               = 1001
};

class MohawkEngine_Riven;
class RivenCommand;
class RivenScript;
class RivenScriptManager;
struct MLSTRecord;

typedef Common::SharedPtr<RivenScript> RivenScriptPtr;
typedef Common::SharedPtr<RivenCommand> RivenCommandPtr;

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
	void addCommand(RivenCommandPtr command);

	/** True if the script does not contain any command */
	bool empty() const;

	/**
	 * Run the script
	 *
	 * Script execution must go through the ScriptManager,
	 * this method should not be called directly.
	 */
	void run(RivenScriptManager *scriptManager);

	/** Print script details to the standard output */
	void dumpScript(byte tabs);

	/** Apply patches to card script to fix bugs in the original game scripts */
	void applyCardPatches(MohawkEngine_Riven *vm, uint32 cardGlobalId, uint16 scriptType, uint16 hotspotId);

	/** Append the commands of the other script to this script */
	RivenScript &operator+=(const RivenScript &other);

	/** Get a caption for a script type */
	static const char *getTypeName(uint16 type);

private:
	Common::Array<RivenCommandPtr> _commands;
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
 * Can run scripts immediately, or store them for future execution.
 */
class RivenScriptManager {
public:
	RivenScriptManager(MohawkEngine_Riven *vm);
	~RivenScriptManager();

	/** Read a single script from a stream */
	RivenScriptPtr readScript(Common::ReadStream *stream);

	/**
	 * Read a script from an array of uint16
	 * @param data Script data array. Will be modified.
	 * @param size Number of uint16 in data
	 * @return
	 */
	RivenScriptPtr readScriptFromData(uint16 *data, uint16 size);

	/** Create a script from the caller provided arguments containing raw data */
	RivenScriptPtr createScriptFromData(uint commandCount, ...);

	/**
	 * Create a script with a single user provided command
	 *
	 * The script takes ownership of the command.
	 */
	RivenScriptPtr createScriptWithCommand(RivenCommand *command);

	/** Read a list of typed scripts from a stream */
	RivenScriptList readScripts(Common::ReadStream *stream);

	/** Run a script */
	void runScript(const RivenScriptPtr &script, bool queue);

	/** Are scripts running in the background */
	bool hasQueuedScripts() const;

	/** Run queued scripts */
	void runQueuedScripts();

	/**
	 * Are queued scripts currently running?
	 *
	 * The game is mostly non-interactive while scripts are running.
	 * This method is used to check if user interaction should be permitted.
	 */
	bool runningQueuedScripts() const;

	/**
	 * Stop running all the scripts
	 *
	 * This is effective immediately after the current command completes.
	 * The next command in the script is not executed. The next scripts
	 * in the queue are skipped until the queue is empty.
	 * Scripts execution then resumes normally.
	 */
	void stopAllScripts();

	/** Should all the scripts stop immediately? */
	bool stoppingAllScripts() const;

	struct StoredMovieOpcode {
		RivenScriptPtr script;
		uint32 time;
		uint16 slot;
	};

	uint16 getStoredMovieOpcodeSlot() { return _storedMovieOpcode.slot; }
	uint32 getStoredMovieOpcodeTime() { return _storedMovieOpcode.time; }
	void setStoredMovieOpcode(const StoredMovieOpcode &op);
	void runStoredMovieOpcode();
	void clearStoredMovieOpcode();

private:
	MohawkEngine_Riven *_vm;

	Common::Array<RivenScriptPtr> _queue;
	bool _runningQueuedScripts;
	bool _stoppingAllScripts;

	StoredMovieOpcode _storedMovieOpcode;

	RivenCommandPtr readCommand(Common::ReadStream *stream);
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

	/** Get the command's type */
	virtual RivenCommandType getType() const = 0;

	/** Apply card patches for the command's sub-scripts */
	virtual void applyCardPatches(uint32 globalId, int scriptType, uint16 hotspotId) {}

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
	static RivenSimpleCommand *createFromStream(MohawkEngine_Riven *vm, RivenCommandType type, Common::ReadStream *stream);

	typedef Common::Array<uint16> ArgumentArray;

	RivenSimpleCommand(MohawkEngine_Riven *vm, RivenCommandType type, const ArgumentArray &arguments);
	~RivenSimpleCommand() override;

	// RivenCommand API
	void dump(byte tabs) override;
	void execute() override;
	RivenCommandType getType() const override;

private:
	typedef void (RivenSimpleCommand::*OpcodeProcRiven)(uint16 op, const ArgumentArray &args);
	struct RivenOpcode {
		OpcodeProcRiven proc;
		const char *desc;
	};


	void setupOpcodes();
	Common::String describe() const;

	void activateMLST(const MLSTRecord &mlst) const;

	DECLARE_OPCODE(empty) { warning ("Unknown Opcode %04x", op); }

	// Opcodes
	DECLARE_OPCODE(drawBitmap);
	DECLARE_OPCODE(switchCard);
	DECLARE_OPCODE(playScriptSLST);
	DECLARE_OPCODE(playSound);
	DECLARE_OPCODE(setVariable);
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

	RivenCommandType _type;
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
	static RivenSwitchCommand *createFromStream(MohawkEngine_Riven *vm, Common::ReadStream *stream);
	~RivenSwitchCommand() override;

	// RivenCommand API
	void dump(byte tabs) override;
	void execute() override;
	RivenCommandType getType() const override;
	void applyCardPatches(uint32 globalId, int scriptType, uint16 hotspotId) override;

private:
	RivenSwitchCommand(MohawkEngine_Riven *vm);

	struct Branch {
		uint16 value;
		RivenScriptPtr script;
	};

	uint16 _variableId;
	Common::Array<Branch> _branches;
};

/**
 * A command to go to a different stack
 *
 * Changes the active stack and sets the initial card.
 * The stack can be specified by global id or name id in the initial stack.
 * The destination card must be specified by global id.
 */
class RivenStackChangeCommand : public RivenCommand {
public:
	RivenStackChangeCommand(MohawkEngine_Riven *vm, uint16 stackId, uint32 globalCardId,
		                        bool byStackId, bool byStackCardId);

	static RivenStackChangeCommand *createFromStream(MohawkEngine_Riven *vm, Common::ReadStream *stream);
	~RivenStackChangeCommand() override;

	// RivenCommand API
	void dump(byte tabs) override;
	void execute() override;
	RivenCommandType getType() const override;

private:
	uint16 _stackId;
	uint32 _cardId;
	bool _byStackId; // Otherwise by stack name id
	bool _byStackCardId; // Otherwise by global card id
};

/**
 * A command to delay execution of card specific hardcoded scripts
 *
 * Timers are queued as script commands so that they don't run when the doFrame method
 * is called from an inner game loop.
 */
class RivenTimerCommand : public RivenCommand {
public:
	RivenTimerCommand(MohawkEngine_Riven *vm, const Common::SharedPtr<RivenStack::TimerProc> &timerProc);

	// RivenCommand API
	void dump(byte tabs) override;
	void execute() override;
	RivenCommandType getType() const override;

private:
	Common::SharedPtr<RivenStack::TimerProc> _timerProc;
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
