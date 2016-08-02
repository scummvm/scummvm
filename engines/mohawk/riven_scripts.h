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

namespace Mohawk {

// Script Types
enum {
	kMouseDownScript = 0,
	kMouseDownScriptAlt = 1,
	kMouseUpScript = 2,
	kMouseMovedPressedReleasedScript = 3,
	kMouseInsideScript = 4,
	kMouseLeaveScript = 5, // This is unconfirmed

	kCardLoadScript = 6,
	kCardLeaveScript = 7,
	kCardOpenScript = 9,
	kCardUpdateScript = 10
};

class MohawkEngine_Riven;
class RivenCommand;

class RivenScript {
public:
	RivenScript(MohawkEngine_Riven *vm);
	~RivenScript();

	void addCommand(RivenCommand *command);

	void runScript();
	void dumpScript(const Common::StringArray &varNames, const Common::StringArray &xNames, byte tabs);
	void stopRunning() { _continueRunning = false; }

private:
	MohawkEngine_Riven *_vm;

	Common::Array<RivenCommand *> _commands;
	bool _continueRunning;
};

typedef Common::SharedPtr<RivenScript> RivenScriptPtr;

struct RivenTypedScript {
	uint16 type;
	RivenScriptPtr script;
};

typedef Common::Array<RivenTypedScript> RivenScriptList;

class RivenScriptManager {
public:
	RivenScriptManager(MohawkEngine_Riven *vm);
	~RivenScriptManager();

	RivenScriptPtr readScript(Common::ReadStream *stream);
	RivenScriptList readScripts(Common::ReadStream *stream);
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

	StoredMovieOpcode _storedMovieOpcode;

	RivenCommand *readCommand(Common::ReadStream *stream);
};

class RivenCommand {
public:
	RivenCommand(MohawkEngine_Riven *vm);
	virtual ~RivenCommand();

	virtual void dump(const Common::StringArray &varNames, const Common::StringArray &xNames, byte tabs) = 0;

	virtual void execute() = 0;

protected:
	MohawkEngine_Riven *_vm;
};

class RivenSimpleCommand : public RivenCommand {
public:
	static RivenSimpleCommand *createFromStream(MohawkEngine_Riven *vm, int type, Common::ReadStream *stream);
	virtual ~RivenSimpleCommand();

	// RivenCommand API
	virtual void dump(const Common::StringArray &varNames, const Common::StringArray &xNames, byte tabs) override;
	virtual void execute() override;

private:
	typedef Common::Array<uint16> ArgumentArray;

	RivenSimpleCommand(MohawkEngine_Riven *vm, int type, const ArgumentArray &arguments);

	int _type;
	ArgumentArray _arguments;

	typedef void (RivenSimpleCommand::*OpcodeProcRiven)(uint16 op, uint16 argc, uint16 *argv);
	struct RivenOpcode {
		OpcodeProcRiven proc;
		const char *desc;
	};
	const RivenOpcode *_opcodes;
	void setupOpcodes();

	DECLARE_OPCODE(empty) { warning ("Unknown Opcode %04x", op); }

	//Opcodes
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
	DECLARE_OPCODE(disableScreenUpdate);
	DECLARE_OPCODE(enableScreenUpdate);
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
};

class RivenSwitchCommand : public RivenCommand {
public:
	static RivenSwitchCommand *createFromStream(MohawkEngine_Riven *vm, int type, Common::ReadStream *stream);
	virtual ~RivenSwitchCommand();

	// RivenCommand API
	virtual void dump(const Common::StringArray &varNames, const Common::StringArray &xNames, byte tabs) override;
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
