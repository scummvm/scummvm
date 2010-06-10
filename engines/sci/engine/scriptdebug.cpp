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

extern const char *selector_name(EngineState *s, int selector);

DebugState g_debugState;

// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
reg_t disassemble(EngineState *s, reg_t pos, int print_bw_tag, int print_bytecode) {
	SegmentObj *mobj = s->_segMan->getSegment(pos.segment, SEG_TYPE_SCRIPT);
	Script *script_entity = NULL;
	byte *scr;
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

	scr = script_entity->_buf;
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

	printf("%04x:%04x: ", PRINT_REG(pos));

	if (print_bytecode) {
		if (pos.offset + bytecount > scr_size) {
			warning("Operation arguments extend beyond end of script");
			return retval;
		}

		for (i = 0; i < bytecount; i++)
			printf("%02x ", scr[pos.offset + i]);

		for (i = bytecount; i < 5; i++)
			printf("   ");
	}

	if (print_bw_tag)
		printf("[%c] ", opsize ? 'B' : 'W');

	printf("%s", opcodeNames[opcode]);

	i = 0;
	while (g_opcode_formats[opcode][i]) {
		switch (g_opcode_formats[opcode][i++]) {
		case Script_Invalid:
			warning("-Invalid operation-");
			break;

		case Script_SByte:
		case Script_Byte:
			printf(" %02x", scr[retval.offset++]);
			break;

		case Script_Word:
		case Script_SWord:
			printf(" %04x", READ_SCI11ENDIAN_UINT16(&scr[retval.offset]));
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
				printf(" %s[%x]", (param_value < kernel->_kernelFuncs.size()) ?
							((param_value < kernel->getKernelNamesSize()) ? kernel->getKernelName(param_value).c_str() : "[Unknown(postulated)]")
							: "<invalid>", param_value);
			else
				printf(opsize ? " %02x" : " %04x", param_value);

			break;

		case Script_Offset:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.offset]);
				retval.offset += 2;
			}
			printf(opsize ? " %02x" : " %04x", param_value);
			break;

		case Script_SRelative:
			if (opsize)
				param_value = scr[retval.offset++];
			else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.offset]);
				retval.offset += 2;
			}
			printf(opsize ? " %02x  [%04x]" : " %04x  [%04x]", param_value, (0xffff) & (retval.offset + param_value));
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
			if (!obj)
				warning("Attempted to reference on non-object at %04x:%04x", PRINT_REG(s->xs->objp));
			else
				printf("	(%s)", selector_name(s, obj->propertyOffsetToId(s->_segMan, scr[pos.offset + 1])));
		}
	}

	printf("\n");

	if (pos == s->xs->addr.pc) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.offset + 2] >> 1) + (s->restAdjust);
			int argc = ((s->xs->sp)[- stackframe - 1]).offset;
			bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

			if (!oldScriptHeader)
				argc += (s->restAdjust);

			printf(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				printf("%04x:%04x", PRINT_REG((s->xs->sp)[j - stackframe]));
				if (j + 1 < argc)
					printf(", ");
			}
			printf(")\n");
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

				printf("  %s::%s[", name, (selector > kernel->getSelectorNamesSize()) ? "<invalid>" : selector_name(s, selector));

				switch (lookupSelector(s->_segMan, called_obj_addr, selector, 0, &fun_ref)) {
				case kSelectorMethod:
					printf("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case kSelectorVariable:
					printf("VAR");
					break;
				case kSelectorNone:
					printf("INVALID");
					break;
				}

				printf("](");

				while (argc--) {
					printf("%04x:%04x", PRINT_REG(sb[- stackframe + 2]));
					if (argc)
						printf(", ");
					stackframe--;
				}

				printf(")\n");
				stackframe -= 2;
			} // while (stackframe > 0)
		} // Send-like opcodes
	} // (heappos == *p_pc)

	return retval;
}


void script_debug(EngineState *s) {
	// Do we support a separate console?

#if 0
	if (sci_debug_flags & _DEBUG_FLAG_LOGGING) {
		printf("%d: acc=%04x:%04x  ", scriptStepCounter, PRINT_REG(s->r_acc));
		disassemble(s, s->xs->addr.pc, 0, 1);
		if (s->seeking == kDebugSeekGlobal)
			printf("Global %d (0x%x) = %04x:%04x\n", s->seekSpecial,
			          s->seekSpecial, PRINT_REG(s->script_000->_localsBlock->_locals[s->seekSpecial]));
	}
#endif

#if 0
	if (!g_debugState.debugging)
		return;
#endif

	if (g_debugState.seeking && !g_debugState.breakpointWasHit) { // Are we looking for something special?
		SegmentObj *mobj = s->_segMan->getSegment(s->xs->addr.pc.segment, SEG_TYPE_SCRIPT);

		if (mobj) {
			Script *scr = (Script *)mobj;
			byte *code_buf = scr->_buf;
			int code_buf_size = scr->getBufSize();
			int opcode = s->xs->addr.pc.offset >= code_buf_size ? 0 : code_buf[s->xs->addr.pc.offset];
			int op = opcode >> 1;
			int paramb1 = s->xs->addr.pc.offset + 1 >= code_buf_size ? 0 : code_buf[s->xs->addr.pc.offset + 1];
			int paramf1 = (opcode & 1) ? paramb1 : (s->xs->addr.pc.offset + 2 >= code_buf_size ? 0 : (int16)READ_SCI11ENDIAN_UINT16(code_buf + s->xs->addr.pc.offset + 1));

			switch (g_debugState.seeking) {
			case kDebugSeekSpecialCallk:
				if (paramb1 != g_debugState.seekSpecial)
					return;

			case kDebugSeekCallk: {
				if (op != op_callk)
					return;
				break;
			}

			case kDebugSeekLevelRet: {
				if ((op != op_ret) || (g_debugState.seekLevel < (int)s->_executionStack.size()-1))
					return;
				break;
			}

			case kDebugSeekGlobal:
				if (op < op_sag)
					return;
				if ((op & 0x3) > 1)
					return; // param or temp
				if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
					return; // locals and not running in script.000
				if (paramf1 != g_debugState.seekSpecial)
					return; // CORRECT global?
				break;

			case kDebugSeekSO:
				// FIXME: Unhandled?
				break;

			case kDebugSeekNothing:
				// We seek nothing, so just continue
				break;
			}

			g_debugState.seeking = kDebugSeekNothing;
			// OK, found whatever we were looking for
		}
	}

	printf("Step #%d\n", s->scriptStepCounter);
	disassemble(s, s->xs->addr.pc, 0, 1);

	if (g_debugState.runningStep) {
		g_debugState.runningStep--;
		return;
	}

	g_debugState.debugging = false;

	Console *con = ((Sci::SciEngine*)g_engine)->getSciDebugger();
	con->attach();
}

void Kernel::dumpScriptObject(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 14 + seeker);
	int i = 0;

	printf("Object\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);
	//-4 because the size includes the two-word header

	printf("Name: %s\n", namepos ? ((char *)(data + namepos)) : "<unknown>");
	printf("Superclass: %x\n", superclass);
	printf("Species: %x\n", species);
	printf("-info-:%x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 12 + seeker) & 0xffff);

	printf("Function area offset: %x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + seeker + 4));
	printf("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + seeker + 6)));

	seeker += 8;

	while (selectors--) {
		printf("  [#%03x] = 0x%x\n", i++, (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker) & 0xffff);
		seeker += 2;
	}

	printf("Overridden functions: %x\n", selectors = overloads = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	if (overloads < 100)
		while (overloads--) {
			int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + (seeker));

			printf("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>");
			printf("%04x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + selectors*2 + 2) & 0xffff);

			seeker += 2;
		}
}

void Kernel::dumpScriptClass(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 14 + seeker);

	printf("Class\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);

	printf("Name: %s\n", namepos ? ((char *)data + namepos) : "<unknown>");
	printf("Superclass: %x\n", superclass);
	printf("Species: %x\n", species);
	printf("-info-:%x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + 12 + seeker) & 0xffff);

	printf("Function area offset: %x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + 4));
	printf("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + 6)));

	seeker += 8;
	selectorsize <<= 1;

	while (selectors--) {
		int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + (seeker) + selectorsize);

		printf("  [%03x] %s = 0x%x\n", 0xffff & selector, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>",
		          (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker) & 0xffff);

		seeker += 2;
	}

	seeker += selectorsize;

	printf("Overloaded functions: %x\n", selectors = overloads = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	while (overloads--) {
		int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + (seeker));
		fprintf(stderr, "selector=%d; selectorNames.size() =%d\n", selector, _selectorNames.size());
		printf("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ?
		          _selectorNames[selector].c_str() : "<?>");
		printf("%04x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + selectors * 2 + 2) & 0xffff);

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
			printf("End of script object (#0) encountered.\n");
			printf("Classes: %i, Objects: %i, Export: %i,\n Var: %i (all base 10)",
			          objectctr[6], objectctr[1], objectctr[7], objectctr[10]);
			return;
		}

		printf("\n");

		objsize = (int16)READ_SCI11ENDIAN_UINT16(script->data + _seeker + 2);

		printf("Obj type #%x, size 0x%x: ", objType, objsize);

		_seeker += objsize;

		objectctr[objType]++;

		switch (objType) {
		case SCI_OBJ_OBJECT:
			dumpScriptObject((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_CODE: {
			printf("Code\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case 3: {
			printf("<unknown>\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case SCI_OBJ_SAID: {
			printf("Said\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);

			printf("%04x: ", seeker);
			while (seeker < _seeker) {
				unsigned char nextitem = script->data [seeker++];
				if (nextitem == 0xFF)
					printf("\n%04x: ", seeker);
				else if (nextitem >= 0xF0) {
					switch (nextitem) {
					case 0xf0:
						printf(", ");
						break;
					case 0xf1:
						printf("& ");
						break;
					case 0xf2:
						printf("/ ");
						break;
					case 0xf3:
						printf("( ");
						break;
					case 0xf4:
						printf(") ");
						break;
					case 0xf5:
						printf("[ ");
						break;
					case 0xf6:
						printf("] ");
						break;
					case 0xf7:
						printf("# ");
						break;
					case 0xf8:
						printf("< ");
						break;
					case 0xf9:
						printf("> ");
						break;
					}
				} else {
					nextitem = nextitem << 8 | script->data [seeker++];
					printf("%s[%03x] ", vocab->getAnyWordFromGroup(nextitem), nextitem);
				}
			}
			printf("\n");
		}
		break;

		case SCI_OBJ_STRINGS: {
			printf("Strings\n");
			while (script->data [seeker]) {
				printf("%04x: %s\n", seeker, script->data + seeker);
				seeker += strlen((char *)script->data + seeker) + 1;
			}
			seeker++; // the ending zero byte
		};
		break;

		case SCI_OBJ_CLASS:
			dumpScriptClass((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_EXPORTS: {
			printf("Exports\n");
			Common::hexdump((unsigned char *)script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case SCI_OBJ_POINTERS: {
			printf("Pointers\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case 9: {
			printf("<unknown>\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		case SCI_OBJ_LOCALVARS: {
			printf("Local vars\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
		};
		break;

		default:
			printf("Unsupported!\n");
			return;
		}

	}

	printf("Script ends without terminator\n");
}

} // End of namespace Sci
