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

#ifndef KYRA_SCRIPT_H
#define KYRA_SCRIPT_H

#include "common/stream.h"
#include "common/array.h"
#include "common/func.h"

namespace Kyra {

struct EMCState;
typedef Common::Functor1<EMCState*, int> Opcode;

struct EMCData {
	byte *text;
	uint16 *data;
	uint16 *ordr;
	uint16 dataSize;

	const Common::Array<const Opcode*> *opcodes;
};

struct EMCState {
	uint16 *ip;
	const EMCData *dataPtr;
	int16 retValue;
	uint16 bp;
	uint16 sp;
	int16 regs[30];		// VM registers
	int16 stack[61];	// VM stack
};

#define stackPos(x) (script->stack[script->sp+x])
#define stackPosString(x) ((const char*)&script->dataPtr->text[READ_BE_UINT16(&script->dataPtr->text[stackPos(x)<<1])])

#define FORM_CHUNK 0x4D524F46
#define TEXT_CHUNK 0x54584554
#define DATA_CHUNK 0x41544144
#define ORDR_CHUNK 0x5244524F
#define AVTL_CHUNK 0x4C545641

class Resource;
class KyraEngine_v1;

class ScriptFileParser {
public:
	ScriptFileParser() : _stream(0), _startOffset(0), _endOffset(0) {}
	ScriptFileParser(const char *filename, Resource *res) : _stream(0), _startOffset(0), _endOffset(0) { setFile(filename, res); }
	~ScriptFileParser() { destroy(); }

	// 'script' must be allocated with new!
	void setFile(const char *filename, Resource *res);

	operator bool() const { return (_startOffset != _endOffset) && _stream; }

	uint32 getFORMBlockSize();
	uint32 getIFFBlockSize(const uint32 chunk);
	bool loadIFFBlock(const uint32 chunk, void *loadTo, uint32 ptrSize);
private:
	void destroy();

	Common::SeekableReadStream *_stream;
	uint32 _startOffset;
	uint32 _endOffset;
};

class EMCInterpreter {
public:
	EMCInterpreter(KyraEngine_v1 *vm);

	bool load(const char *filename, EMCData *data, const Common::Array<const Opcode*> *opcodes);
	void unload(EMCData *data);

	void init(EMCState *scriptState, const EMCData *data);
	bool start(EMCState *script, int function);

	bool isValid(EMCState *script);

	bool run(EMCState *script);
protected:
	KyraEngine_v1 *_vm;
	int16 _parameter;

	typedef void (EMCInterpreter::*CommandProc)(EMCState*);
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
	};

	const CommandEntry *_commands;
private:
	void cmd_jmpTo(EMCState*);
	void cmd_setRetValue(EMCState*);
	void cmd_pushRetOrPos(EMCState*);
	void cmd_push(EMCState*);
	void cmd_pushReg(EMCState*);
	void cmd_pushBPNeg(EMCState*);
	void cmd_pushBPAdd(EMCState*);
	void cmd_popRetOrPos(EMCState*);
	void cmd_popReg(EMCState*);
	void cmd_popBPNeg(EMCState*);
	void cmd_popBPAdd(EMCState*);
	void cmd_addSP(EMCState*);
	void cmd_subSP(EMCState*);
	void cmd_execOpcode(EMCState*);
	void cmd_ifNotJmp(EMCState*);
	void cmd_negate(EMCState*);
	void cmd_eval(EMCState*);
	void cmd_setRetAndJmp(EMCState*);
};
} // end of namespace Kyra

#endif

