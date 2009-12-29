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

#ifndef RIVEN_SCRIPTS_H
#define RIVEN_SCRIPTS_H

class MohawkEngine_Riven;

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

class RivenScript;
typedef Common::Array<Common::SharedPtr<RivenScript> > RivenScriptList;

class RivenScript {
public:
	RivenScript(MohawkEngine_Riven *vm, Common::SeekableReadStream *stream, uint16 scriptType);
	~RivenScript();
	
	void runScript();
	void dumpScript(Common::StringList varNames, Common::StringList xNames, byte tabs);
	uint16 getScriptType() { return _scriptType; }
	
	// Read in an array of script objects from a stream
	static RivenScriptList readScripts(MohawkEngine_Riven *vm, Common::SeekableReadStream *stream);
	
private:
	typedef void (RivenScript::*OpcodeProcRiven)(uint16 op, uint16 argc, uint16 *argv);
	struct RivenOpcode {
		OpcodeProcRiven proc;
		const char *desc;
	};
	const RivenOpcode* _opcodes;
	void setupOpcodes();
	
	MohawkEngine_Riven *_vm;
	Common::SeekableReadStream *_stream;
	uint16 _scriptType;
	
	void dumpCommands(Common::StringList varNames, Common::StringList xNames, byte tabs);
	void processCommands(bool runCommands);
	
	static uint32 calculateCommandSize(Common::SeekableReadStream* script);
	static uint32 calculateScriptSize(Common::SeekableReadStream* script);
	
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
	DECLARE_OPCODE(clearSLST);
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
	DECLARE_OPCODE(playMovie);
	DECLARE_OPCODE(playMovieBg);
	DECLARE_OPCODE(stopMovie);
	DECLARE_OPCODE(unk_36);
	DECLARE_OPCODE(fadeAmbientSounds);
	DECLARE_OPCODE(complexPlayMovie);
	DECLARE_OPCODE(activatePLST);
	DECLARE_OPCODE(activateSLST);
	DECLARE_OPCODE(activateMLSTAndPlay);
	DECLARE_OPCODE(activateBLST);
	DECLARE_OPCODE(activateFLST);
	DECLARE_OPCODE(zipMode);
	DECLARE_OPCODE(activateMLST);
	DECLARE_OPCODE(activateSLSTWithVolume);
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
