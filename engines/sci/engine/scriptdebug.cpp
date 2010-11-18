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

// Script debugger functionality. Absolutely not threadsafe.

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"

namespace Sci {

const char *opcodeNames[] = {
	   "bnot",       "add",      "sub",      "mul",      "div",
		"mod",       "shr",      "shl",      "xor",      "and",
		 "or",       "neg",      "not",      "eq?",      "ne?",
		"gt?",       "ge?",      "lt?",      "le?",     "ugt?",
	   "uge?",      "ult?",     "ule?",       "bt",      "bnt",
		"jmp",       "ldi",     "push",    "pushi",     "toss",
		"dup",      "link",     "call",    "callk",    "callb",
	  "calle",       "ret",     "send",    "dummy",    "dummy",
	  "class",     "dummy",     "self",    "super",    "&rest",
		"lea",    "selfID",    "dummy",    "pprev",     "pToa",
	   "aTop",      "pTos",     "sTop",    "ipToa",    "dpToa",
	  "ipTos",     "dpTos",    "lofsa",    "lofss",    "push0",
	  "push1",     "push2", "pushSelf",    "dummy",      "lag",
		"lal",       "lat",      "lap",      "lsg",      "lsl",
		"lst",       "lsp",     "lagi",     "lali",     "lati",
	   "lapi",      "lsgi",     "lsli",     "lsti",     "lspi",
		"sag",       "sal",      "sat",      "sap",      "ssg",
		"ssl",       "sst",      "ssp",     "sagi",     "sali",
	   "sati",      "sapi",     "ssgi",     "ssli",     "ssti",
	   "sspi",       "+ag",      "+al",      "+at",      "+ap",
		"+sg",       "+sl",      "+st",      "+sp",     "+agi",
	   "+ali",      "+ati",     "+api",     "+sgi",     "+sli",
	   "+sti",      "+spi",      "-ag",      "-al",      "-at",
		"-ap",       "-sg",      "-sl",      "-st",      "-sp",
	   "-agi",      "-ali",     "-ati",     "-api",     "-sgi",
	   "-sli",      "-sti",     "-spi"
};

// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
reg_t disassemble(EngineState *s, reg_t pos, bool printBWTag, bool printBytecode) {
	SegmentObj *mobj = s->_segMan->getSegment(pos.segment, SEG_TYPE_SCRIPT);
	Script *script_entity = NULL;
	const byte *scr;
	int scr_size;
	reg_t retval = make_reg(pos.segment, pos.offset + 1);
	uint16 param_value;
	int i = 0;
	Kernel *kernel = g_sci->getKernel();

	if (!mobj) {
		warning("Disassembly failed: Segment %04x non-existant or not a script", pos.segment);
		return retval;
	} else
		script_entity = (Script *)mobj;

	scr = script_entity->getBuf();
	scr_size = script_entity->getBufSize();

	if (pos.offset >= scr_size) {
		warning("Trying to disassemble beyond end of script");
		return pos;
	}

	int16 opparams[4];
	byte opsize;
	int bytecount = readPMachineInstruction(scr + pos.offset, opsize, opparams);
	const byte opcode = opsize >> 1;

	opsize &= 1; // byte if true, word if false

	debugN("%04x:%04x: ", PRINT_REG(pos));

	if (printBytecode) {
		if (pos.offset + bytecount > scr_size) {
			warning("Operation arguments extend beyond end of script");
			return retval;
		}

		for (i = 0; i < bytecount; i++)
			debugN("%02x ", scr[pos.offset + i]);

		for (i = bytecount; i < 5; i++)
			debugN("   ");
	}

	if (printBWTag)
		debugN("[%c] ", opsize ? 'B' : 'W');

	debugN("%s", opcodeNames[opcode]);

	i = 0;
	while (g_opcode_formats[opcode][i]) {
		switch (g_opcode_formats[opcode][i++]) {
		case Script_Invalid:
			warning("-Invalid operation-");
			break;

		case Script_SByte:
		case Script_Byte:
			param_value = scr[retval.offset++];
			debugN(" %02x", scr[retval.offset++]);
			break;

		case Script_Word:
		case Script_SWord:
			param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.offset]);
			debugN(" %04x", READ_SCI11ENDIAN_UINT16(&scr[retval.offset]));
			retval.offset += 2;
			break;

		case Script_SVariable:
		case Script_Variable:
		case Script_Property:
		case Script_Global:
		case Script_Local:
		case Script_Temp:
		case Script_Param:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.offset]);
				retval.offset += 2;
			}

			if (opcode == op_callk)
				debugN(" %s[%x]", (param_value < kernel->_kernelFuncs.size()) ?
							((param_value < kernel->getKernelNamesSize()) ? kernel->getKernelName(param_value).c_str() : "[Unknown(postulated)]")
							: "<invalid>", param_value);
			else
				debugN(opsize ? " %02x" : " %04x", param_value);

			break;

		case Script_Offset:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.offset]);
				retval.offset += 2;
			}
			debugN(opsize ? " %02x" : " %04x", param_value);
			break;

		case Script_SRelative:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.offset]);
				retval.offset += 2;
			}
			debugN(opsize ? " %02x  [%04x]" : " %04x  [%04x]", param_value, (0xffff) & (retval.offset + param_value));
			break;

		case Script_End:
			retval = NULL_REG;
			break;

		default:
			error("Internal assertion failed in disassemble()");

		}
	}

	if (pos == s->xs->addr.pc) { // Extra information if debugging the current opcode
		if ((opcode == op_pTos) || (opcode == op_sTop) || (opcode == op_pToa) || (opcode == op_aTop) ||
		        (opcode == op_dpToa) || (opcode == op_ipToa) || (opcode == op_dpTos) || (opcode == op_ipTos)) {
			const Object *obj = s->_segMan->getObject(s->xs->objp);
			if (!obj) {
				warning("Attempted to reference on non-object at %04x:%04x", PRINT_REG(s->xs->objp));
			} else {
				if (getSciVersion() == SCI_VERSION_3)
					debugN("\t(%s)", g_sci->getKernel()->getSelectorName(param_value).c_str());
				else
					debugN("\t(%s)", g_sci->getKernel()->getSelectorName(obj->propertyOffsetToId(s->_segMan, param_value)).c_str());
			}
		}
	}

	debugN("\n");

	if (pos == s->xs->addr.pc) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.offset + 2] >> 1) + (s->restAdjust);
			int argc = ((s->xs->sp)[- stackframe - 1]).offset;
			bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

			if (!oldScriptHeader)
				argc += (s->restAdjust);

			debugN(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				debugN("%04x:%04x", PRINT_REG((s->xs->sp)[j - stackframe]));
				if (j + 1 < argc)
					debugN(", ");
			}
			debugN(")\n");
		} else if ((opcode == op_send) || (opcode == op_self)) {
			int restmod = s->restAdjust;
			int stackframe = (scr[pos.offset + 1] >> 1) + restmod;
			reg_t *sb = s->xs->sp;
			uint16 selector;
			reg_t fun_ref;

			while (stackframe > 0) {
				int argc = sb[- stackframe + 1].offset;
				const char *name = NULL;
				reg_t called_obj_addr = s->xs->objp;

				if (opcode == op_send)
					called_obj_addr = s->r_acc;
				else if (opcode == op_self)
					called_obj_addr = s->xs->objp;

				selector = sb[- stackframe].offset;

				name = s->_segMan->getObjectName(called_obj_addr);

				if (!name)
					name = "<invalid>";

				debugN("  %s::%s[", name, g_sci->getKernel()->getSelectorName(selector).c_str());

				switch (lookupSelector(s->_segMan, called_obj_addr, selector, 0, &fun_ref)) {
				case kSelectorMethod:
					debugN("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case kSelectorVariable:
					debugN("VAR");
					break;
				case kSelectorNone:
					debugN("INVALID");
					break;
				}

				debugN("](");

				while (argc--) {
					debugN("%04x:%04x", PRINT_REG(sb[- stackframe + 2]));
					if (argc)
						debugN(", ");
					stackframe--;
				}

				debugN(")\n");
				stackframe -= 2;
			} // while (stackframe > 0)
		} // Send-like opcodes
	} // (heappos == *p_pc)

	return retval;
}


void SciEngine::scriptDebug() {
	EngineState *s = _gamestate;
	if (_debugState.seeking && !_debugState.breakpointWasHit) { // Are we looking for something special?
		if (_debugState.seeking == kDebugSeekStepOver) {
			// are we above seek-level? resume then
			if (_debugState.seekLevel < (int)s->_executionStack.size())
				return;
			_debugState.seeking = kDebugSeekNothing;
		}

		if (_debugState.seeking != kDebugSeekNothing) {
			const reg_t pc = s->xs->addr.pc;
			SegmentObj *mobj = s->_segMan->getSegment(pc.segment, SEG_TYPE_SCRIPT);

			if (mobj) {
				Script *scr = (Script *)mobj;
				const byte *code_buf = scr->getBuf();
				int code_buf_size = scr->getBufSize();
				int opcode = pc.offset >= code_buf_size ? 0 : code_buf[pc.offset];
				int op = opcode >> 1;
				int paramb1 = pc.offset + 1 >= code_buf_size ? 0 : code_buf[pc.offset + 1];
				int paramf1 = (opcode & 1) ? paramb1 : (pc.offset + 2 >= code_buf_size ? 0 : (int16)READ_SCI11ENDIAN_UINT16(code_buf + pc.offset + 1));

				switch (_debugState.seeking) {
				case kDebugSeekSpecialCallk:
					if (paramb1 != _debugState.seekSpecial)
						return;

				case kDebugSeekCallk:
					if (op != op_callk)
						return;
					break;

				case kDebugSeekLevelRet:
					if ((op != op_ret) || (_debugState.seekLevel < (int)s->_executionStack.size()-1))
						return;
					break;

				case kDebugSeekGlobal:
					if (op < op_sag)
						return;
					if ((op & 0x3) > 1)
						return; // param or temp
					if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
						return; // locals and not running in script.000
					if (paramf1 != _debugState.seekSpecial)
						return; // CORRECT global?
					break;

				default:
					break;
				}

				_debugState.seeking = kDebugSeekNothing;
			}
		}
		// OK, found whatever we were looking for
	}

	debugN("Step #%d\n", s->scriptStepCounter);
	disassemble(s, s->xs->addr.pc, false, true);

	if (_debugState.runningStep) {
		_debugState.runningStep--;
		return;
	}

	_debugState.debugging = false;

	_console->attach();
}

void Kernel::dumpScriptObject(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 14 + seeker);
	int i = 0;

	debugN("Object\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);
	//-4 because the size includes the two-word header

	debugN("Name: %s\n", namepos ? ((char *)(data + namepos)) : "<unknown>");
	debugN("Superclass: %x\n", superclass);
	debugN("Species: %x\n", species);
	debugN("-info-:%x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 12 + seeker) & 0xffff);

	debugN("Function area offset: %x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + seeker + 4));
	debugN("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + seeker + 6)));

	seeker += 8;

	while (selectors--) {
		debugN("  [#%03x] = 0x%x\n", i++, (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker) & 0xffff);
		seeker += 2;
	}

	debugN("Overridden functions: %x\n", selectors = overloads = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	if (overloads < 100)
		while (overloads--) {
			int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + (seeker));

			debugN("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>");
			debugN("%04x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + selectors*2 + 2) & 0xffff);

			seeker += 2;
		}
}

void Kernel::dumpScriptClass(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 14 + seeker);

	debugN("Class\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);

	debugN("Name: %s\n", namepos ? ((char *)data + namepos) : "<unknown>");
	debugN("Superclass: %x\n", superclass);
	debugN("Species: %x\n", species);
	debugN("-info-:%x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + 12 + seeker) & 0xffff);

	debugN("Function area offset: %x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + 4));
	debugN("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + 6)));

	seeker += 8;
	selectorsize <<= 1;

	while (selectors--) {
		int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + (seeker) + selectorsize);

		debugN("  [%03x] %s = 0x%x\n", 0xffff & selector, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>",
		          (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker) & 0xffff);

		seeker += 2;
	}

	seeker += selectorsize;

	debugN("Overloaded functions: %x\n", selectors = overloads = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	while (overloads--) {
		int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + (seeker));
		debugN("selector=%d; selectorNames.size() =%d\n", selector, _selectorNames.size());
		debugN("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ?
		          _selectorNames[selector].c_str() : "<?>");
		debugN("%04x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + selectors * 2 + 2) & 0xffff);

		seeker += 2;
	}
}

void Kernel::dissectScript(int scriptNumber, Vocabulary *vocab) {
	int objectctr[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int _seeker = 0;
	Resource *script = _resMan->findResource(ResourceId(kResourceTypeScript, scriptNumber), 0);

	if (!script) {
		warning("dissectScript(): Script not found!\n");
		return;
	}

	while (_seeker < script->size) {
		int objType = (int16)READ_SCI11ENDIAN_UINT16(script->data + _seeker);
		int objsize;
		unsigned int seeker = _seeker + 4;

		if (!objType) {
			debugN("End of script object (#0) encountered.\n");
			debugN("Classes: %i, Objects: %i, Export: %i,\n Var: %i (all base 10)",
			          objectctr[6], objectctr[1], objectctr[7], objectctr[10]);
			return;
		}

		debugN("\n");

		objsize = (int16)READ_SCI11ENDIAN_UINT16(script->data + _seeker + 2);

		debugN("Obj type #%x, size 0x%x: ", objType, objsize);

		_seeker += objsize;

		objectctr[objType]++;

		switch (objType) {
		case SCI_OBJ_OBJECT:
			dumpScriptObject((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_CODE:
			debugN("Code\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_SYNONYMS:
			debugN("Synonyms\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_SAID:
			debugN("Said\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);

			debugN("%04x: ", seeker);
			vocab->debugDecipherSaidBlock(script->data + seeker);
			debugN("\n");
			break;

		case SCI_OBJ_STRINGS:
			debugN("Strings\n");
			while (script->data [seeker]) {
				debugN("%04x: %s\n", seeker, script->data + seeker);
				seeker += strlen((char *)script->data + seeker) + 1;
			}
			seeker++; // the ending zero byte
			break;

		case SCI_OBJ_CLASS:
			dumpScriptClass((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_EXPORTS:
			debugN("Exports\n");
			Common::hexdump((unsigned char *)script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_POINTERS:
			debugN("Pointers\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case 9:
			debugN("<unknown>\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_LOCALVARS:
			debugN("Local vars\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		default:
			debugN("Unsupported!\n");
			return;
		}

	}

	debugN("Script ends without terminator\n");
}

} // End of namespace Sci
