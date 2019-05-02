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

#ifndef MYST_SCRIPTS_H
#define MYST_SCRIPTS_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/util.h"

#include "mohawk/myst_state.h"

namespace Mohawk {

typedef Common::Array<uint16> ArgumentsArray;

#define DECLARE_OPCODE(x) void x(uint16 var, const ArgumentsArray &args)

class MohawkEngine_Myst;
class MystArea;

enum MystScriptType {
	kMystScriptNone,
	kMystScriptNormal,
	kMystScriptInit,
	kMystScriptExit
};

struct MystScriptEntry {
	MystScriptEntry();

	MystScriptType type;
	uint16 resourceId;
	uint16 opcode;
	uint16 var;
	ArgumentsArray args;
	uint16 u1;
};

typedef Common::Array<MystScriptEntry> MystScript;

class MystScriptParser {
public:
	MystScriptParser(MohawkEngine_Myst *vm, MystStack stackId);
	virtual ~MystScriptParser();

	void runScript(const MystScript &script, MystArea *invokingResource = nullptr);
	void runOpcode(uint16 op, uint16 var = 0, const ArgumentsArray &args = ArgumentsArray());
	const Common::String getOpcodeDesc(uint16 op);
	MystScript readScript(Common::SeekableReadStream *stream, MystScriptType type);
	void setInvokingResource(MystArea *resource) { _invokingResource = resource; }

	/**
	 * Is a script is running?
	 *
	 * Allows to detect if some inner loop is running instead of the main loop.
	 */
	bool isScriptRunning() const;

	virtual void disablePersistentScripts() = 0;
	virtual void runPersistentScripts() = 0;

	virtual uint16 getVar(uint16 var);
	virtual void toggleVar(uint16 var);
	virtual bool setVarValue(uint16 var, uint16 value);

	MystStack getStackId() const { return _stackId; }
	virtual uint16 getMap() { return 0; }
	void showMap();

	void animatedUpdate(const ArgumentsArray &args, uint16 delay);
	void soundWaitStop() const;

	// Common opcodes
	DECLARE_OPCODE(o_toggleVar);
	DECLARE_OPCODE(o_setVar);
	DECLARE_OPCODE(o_changeCardSwitch4);
	DECLARE_OPCODE(o_changeCardSwitchLtR);
	DECLARE_OPCODE(o_changeCardSwitchRtL);
	DECLARE_OPCODE(o_takePage);
	DECLARE_OPCODE(o_redrawCard);
	DECLARE_OPCODE(o_goToDest);
	DECLARE_OPCODE(o_goToDestForward);
	DECLARE_OPCODE(o_goToDestRight);
	DECLARE_OPCODE(o_goToDestLeft);
	DECLARE_OPCODE(o_goToDestUp);
	DECLARE_OPCODE(o_triggerMovie);
	DECLARE_OPCODE(o_toggleVarNoRedraw);
	DECLARE_OPCODE(o_drawAreaState);
	DECLARE_OPCODE(o_redrawAreaForVar);
	DECLARE_OPCODE(o_changeCardDirectional);
	DECLARE_OPCODE(o_changeCardPush);
	DECLARE_OPCODE(o_changeCardPop);
	DECLARE_OPCODE(o_enableAreas);
	DECLARE_OPCODE(o_disableAreas);
	DECLARE_OPCODE(o_directionalUpdate);
	DECLARE_OPCODE(o_toggleAreasActivation);
	DECLARE_OPCODE(o_playSound);
	DECLARE_OPCODE(o_stopSoundBackground);
	DECLARE_OPCODE(o_playSoundBlocking);
	DECLARE_OPCODE(o_copyBackBufferToScreen);
	DECLARE_OPCODE(o_copyImageToBackBuffer);
	DECLARE_OPCODE(o_changeBackgroundSound);
	DECLARE_OPCODE(o_soundPlaySwitch);
	DECLARE_OPCODE(o_copyImageToScreen);
	DECLARE_OPCODE(o_soundResumeBackground);
	DECLARE_OPCODE(o_changeCard);
	DECLARE_OPCODE(o_drawImageChangeCard);
	DECLARE_OPCODE(o_changeMainCursor);
	DECLARE_OPCODE(o_hideCursor);
	DECLARE_OPCODE(o_showCursor);
	DECLARE_OPCODE(o_delay);
	DECLARE_OPCODE(o_changeStack);
	DECLARE_OPCODE(o_changeCardPlaySoundDirectional);
	DECLARE_OPCODE(o_directionalUpdatePlaySound);
	DECLARE_OPCODE(o_saveMainCursor);
	DECLARE_OPCODE(o_restoreMainCursor);
	DECLARE_OPCODE(o_soundWaitStop);
	DECLARE_OPCODE(o_exitMap);

	// Used in multiple stacks
	DECLARE_OPCODE(o_quit);

	DECLARE_OPCODE(NOP);

protected:
	MohawkEngine_Myst *_vm;
	MystGameState::Globals &_globals;

	typedef Common::Functor2<uint16, const ArgumentsArray &, void> OpcodeProcMyst;

#define REGISTER_OPCODE(op, cls, method) \
		registerOpcode( \
			op, #method, new Common::Functor2Mem<uint16, const ArgumentsArray &, void, cls>(this, &cls::method) \
		)

#define OVERRIDE_OPCODE(op, cls, method) \
		overrideOpcode( \
			op, #method, new Common::Functor2Mem<uint16, const ArgumentsArray &, void, cls>(this, &cls::method) \
		)

	void registerOpcode(uint16 op, const char *name, OpcodeProcMyst *command);
	void overrideOpcode(uint16 op, const char *name, OpcodeProcMyst *command);

	uint16 _savedCardId;
	uint16 _savedMapCardId;
	uint16 _savedCursorId;
	int16 _tempVar; // Generic temp var used by the scripts
	uint32 _startTime; // Generic start time used by the scripts

	static const MystStack _stackMap[];
	static const uint16 _startCard[];

	void setupCommonOpcodes();

	template<class T>
	T *getInvokingResource() const;

private:
	struct MystOpcode {
		MystOpcode(uint16 o, OpcodeProcMyst *p, const char *d) : op(o), proc(p), desc(d) {}

		uint16 op;
		Common::SharedPtr<OpcodeProcMyst> proc;
		const char *desc;
	};

	Common::Array<MystOpcode> _opcodes;

	MystArea *_invokingResource;
	int32 _scriptNestingLevel;

	const MystStack _stackId;

	Common::String describeCommand(const MystOpcode &command, uint16 var, const ArgumentsArray &args);
};

template<class T>
T *MystScriptParser::getInvokingResource() const {
	T *resource = dynamic_cast<T *>(_invokingResource);

	if (!resource) {
		error("Invoking resource has unexpected type");
	}

	return resource;
}

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
