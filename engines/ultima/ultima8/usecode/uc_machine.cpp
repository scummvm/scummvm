/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/uc_process.h"
#include "ultima/ultima8/usecode/usecode.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/usecode/bit_set.h"
#include "ultima/ultima8/usecode/byte_set.h"
#include "ultima/ultima8/usecode/uc_list.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/convert/u8/convert_usecode_u8.h"
#include "ultima/ultima8/convert/crusader/convert_usecode_regret.h"
#include "ultima/ultima8/world/actors/main_actor.h"

namespace Ultima {
namespace Ultima8 {

#ifdef DEBUG
#define LOGPF(X) do { if (trace_show(trace_pid, trace_objid, trace_classid)) pout.Print X; } while (0)
#else
#define LOGPF(X)
#endif

#ifdef DEBUG
static const char *print_bp(const int16 offset) {
	static char str[32];
	snprintf(str, 32, "[BP%c%02Xh]", offset < 0 ? '-' : '+',
	         offset < 0 ? -offset : offset);
	return str;
}

static const char *print_sp(const int16 offset) {
	static char str[32];
	snprintf(str, 32, "[SP%c%02Xh]", offset < 0 ? '-' : '+',
	         offset < 0 ? -offset : offset);
	return str;
}
#endif


//#define DUMPHEAP

enum UCSegments {
	SEG_STACK      = 0x0000,
	SEG_STACK_FIRST = 0x0001,
	SEG_STACK_LAST = 0x7FFE,
	SEG_STRING     = 0x8000,
	SEG_LIST       = 0x8001, // I don't think this is used
	SEG_OBJ        = 0x8002,
	SEG_GLOBAL     = 0x8003
};

UCMachine *UCMachine::_ucMachine = nullptr;

UCMachine::UCMachine(Intrinsic *iset, unsigned int icount) {
	debugN(MM_INFO, "Creating UCMachine...\n");

	_ucMachine = this;

	// zero _globals

	if (GAME_IS_U8) {
		_globals = new BitSet(0x1000);
		_convUse = new ConvertUsecodeU8();
	} else if (GAME_IS_REMORSE) {
		_globals = new ByteSet(0x1000);
		// slight hack: set global 003C to start as avatar number.
		_globals->setEntries(0x3C, 2, 1);
		_convUse = new ConvertUsecodeCrusader();
	} else {
		_globals = new ByteSet(0x1000);
		// slight hack: set global 001E to start as avatar number.
		_globals->setEntries(0x1E, 2, 1);
		_convUse = new ConvertUsecodeRegret();
	}

	loadIntrinsics(iset, icount); //!...

	_listIDs = new idMan(1, 65534, 128);
	_stringIDs = new idMan(1, 65534, 256);

#ifdef DEBUG
	_tracingEnabled = false;
	_traceAll = false;
#endif
}


UCMachine::~UCMachine() {
	debugN(MM_INFO, "Destroying UCMachine...\n");
	_ucMachine = nullptr;

	delete _globals;
	delete _convUse;
	delete _listIDs;
	delete _stringIDs;
}

void UCMachine::reset() {
	debugN(MM_INFO, "Resetting UCMachine\n");

	// clear _globals
	_globals->setSize(0x1000);

	// slight HACK: set global 003C (remorse) / 001E (regret)
	// to start as avatar number.
	if (GAME_IS_REMORSE) {
		_globals->setEntries(0x3C, 2, 1);
	} else if (GAME_IS_REGRET) {
		_globals->setEntries(0x1E, 2, 1);
	}

	// clear strings, lists
	Common::HashMap<uint16, UCList *>::iterator iter;
	for (iter = _listHeap.begin(); iter != _listHeap.end(); ++iter)
		delete(iter->_value);
	_listHeap.clear();
	_stringHeap.clear();
}

void UCMachine::loadIntrinsics(Intrinsic *i, unsigned int icount) {
	_intrinsics = i;
	_intrinsicCount = icount;
}

void UCMachine::execProcess(UCProcess *p) {
	assert(p);

	uint32 base = p->_usecode->get_class_base_offset(p->_classId);
	Common::SeekableReadStream *cs = new Common::MemoryReadStream(p->_usecode->get_class(p->_classId) + base,
																  p->_usecode->get_class_size(p->_classId) - base);
	cs->seek(p->_ip);

#ifdef DEBUG
	if (trace_show(p->_pid, p->_itemNum, p->_classId)) {
		pout << "tick " << Kernel::get_instance()->getTickNum()
			 << ConsoleStream::hex << " running process " << p->_pid
		     << ", item " << p->_itemNum << ", type " << p->_type
		     << ", class " << p->_classId << ", offset " << p->_ip
		     << ConsoleStream::dec << Std::endl;
	}
#endif

	bool cede = false;
	bool error = false;
	bool go_until_cede = false;

	while (!cede && !error && !p->is_terminated()) {
		//! guard against reading past end of class
		//! guard against other error conditions

		uint8 opcode = cs->readByte();

#ifdef DEBUG
		uint16 trace_classid = p->_classId;
		ObjId trace_objid = p->_itemNum;
		ProcId trace_pid = p->_pid;
#endif

		LOGPF(("sp = %02X; %04X:%04X: %02X\t",
		       p->_stack.stacksize(), p->_classId, p->_ip, opcode));

		int8 si8a, si8b;
		uint8 ui8a;
		uint16 ui16a, ui16b;
		uint32 ui32a, ui32b;
		int16 si16a, si16b;
		int32 si32a, si32b;

		switch (opcode) {

		// POP opcodes
		case 0x00:
			// 00 xx
			// pop 16 bit int, and assign LS 8 bit int into bp+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.pop2();
			p->_stack.assign1(p->_bp + si8a, static_cast<uint8>(ui16a));
			LOGPF(("pop byte\t%s = %02Xh\n", print_bp(si8a), ui16a));
			break;

		case 0x01:
			// 01 xx
			// pop 16 bit int into bp+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.pop2();
			p->_stack.assign2(p->_bp + si8a, ui16a);
			LOGPF(("pop\t\t%s = %04Xh\n", print_bp(si8a), ui16a));
			break;

		case 0x02:
			// 02 xx
			// pop 32 bit int into bp+xx
			si8a = cs->readSByte();
			ui32a = p->_stack.pop4();
			p->_stack.assign4(p->_bp + si8a, ui32a);
			LOGPF(("pop dword\t%s = %08Xh\n", print_bp(si8a), ui32a));
			break;

		case 0x03: {
			// 03 xx yy
			// pop yy bytes into bp+xx
			si8a = cs->readSByte();
			uint8 size = cs->readByte();
			uint8 buf[256];
			p->_stack.pop(buf, size);
			p->_stack.assign(p->_bp + si8a, buf, size);
			LOGPF(("pop huge\t%s %i\n", print_bp(si8a), size));
			break;
		}

		// 0x04 ASSIGN_MEMBER_CHAR (Unused)
		// 0x05 ASSIGN_MEMBER_INT (Unused)
		// 0x06 ASSIGN_MEMBER_LONG (Unused)
		// 0x07 ASSIGN_MEMBER_HUGE (Unused)

		case 0x08:
			// 08
			// pop 32bits into process result register
			LOGPF(("pop dword\tprocess result\n"));
			p->_result = p->_stack.pop4();
			break;

		case 0x09: {
			// 09 xx yy zz
			// pop yy bytes into an element of list bp+xx (or slist if zz set)
			si8a = cs->readSByte();
			ui32a = cs->readByte();
			si8b = cs->readSByte();
			LOGPF(("assign element\t%s (%02X) (slist==%02X)\n",
			       print_bp(si8a), ui32a, si8b));
			ui16a = p->_stack.pop2() - 1; // index
			ui16b = p->_stack.access2(p->_bp + si8a);
			UCList *l = getList(ui16b);
			if (!l) {
				perr << "assign element to an invalid list (" << ui16b << ")"
				     << Std::endl;
				error = true;
				break;
			}
			if (si8b) { // slist?
				// what special behaviour do we need here?
				// probably just that the overwritten element has to be freed?
				if (ui32a != 2) {
					perr << "Unhandled operand " << ui32a << " to pop slist"
					     << Std::endl;
					error = true; // um?
				}
				l->assign(ui16a, p->_stack.access());
				p->_stack.pop2(); // advance SP
			} else {
				l->assign(ui16a, p->_stack.access());
				p->_stack.addSP(ui32a);
			}
			break;
		}

		// PUSH opcodes

		case 0x0A:
			// 0A xx
			// push sign-extended 8 bit xx onto the stack as 16 bit
			ui16a = cs->readSByte();
			p->_stack.push2(ui16a);
			LOGPF(("push byte\t%04Xh\n", ui16a));
			break;

		case 0x0B:
			// 0B xx xx
			// push 16 bit xxxx onto the stack
			ui16a = cs->readUint16LE();
			p->_stack.push2(ui16a);
			LOGPF(("push\t\t%04Xh\n", ui16a));
			break;

		case 0x0C:
			// 0C xx xx xx xx
			// push 32 bit xxxxxxxx onto the stack
			ui32a = cs->readUint32LE();
			p->_stack.push4(ui32a);
			LOGPF(("push dword\t%08Xh\n", ui32a));
			break;

		case 0x0D: {
			// 0D xx xx yy ... yy 00
			// push string (yy ... yy) of length xx xx onto the stack
			ui16a = cs->readUint16LE();
			char *str = new char[ui16a + 1];
			cs->read(str, ui16a);
			str[ui16a] = 0;

			// WORKAROUND: German U8: When the candles are not in the right positions
			// for a sorcery spell, the string does not match, causing a crash.
			// Original bug: https://sourceforge.net/p/pentagram/bugs/196/
			if (GAME_IS_U8 && p->_classId == 0x7C) {
				if (!strcmp(str, " Irgendetwas stimmt nicht!")) {
					str[25] = '.'; // ! to .
				}
			}

			LOGPF(("push string\t\"%s\"\n", str));
			ui16b = cs->readByte();
			if (ui16b != 0) {
				perr << "Zero terminator missing in push string"
				     << Std::endl;
				error = true;
			}
			p->_stack.push2(assignString(str));
			delete[] str;
			break;
		}

		case 0x0E: {
			// 0E xx yy
			// pop yy values of size xx and push the resulting list
			// (list is created in reverse order)
			ui16a = cs->readByte();
			ui16b = cs->readByte();
			UCList *l = new UCList(ui16a, ui16b);
			p->_stack.addSP(ui16a * (ui16b - 1));
			for (unsigned int i = 0; i < ui16b; i++) {
				l->append(p->_stack.access());
				p->_stack.addSP(-ui16a);
			}
			p->_stack.addSP(ui16a * (ui16b + 1));
			p->_stack.push2(assignList(l));
			LOGPF(("create list\t%02X (%02X)\n", ui16b, ui16a));
			break;
		}

		// Usecode function and intrinsic calls

		case 0x0F: {
			// 0F xx yyyy
			// intrinsic call. xx is number of argument bytes
			// (includes this pointer, if present)
			// NB: do not actually pop these argument bytes
			uint16 arg_bytes = cs->readByte();
			uint16 func = cs->readUint16LE();
			LOGPF(("calli\t\t%04Xh (%02Xh arg bytes) %s\n", func, arg_bytes, _convUse->intrinsics()[func]));

			// !constants
			if (func >= _intrinsicCount || _intrinsics[func] == 0) {
				Item *testItem = nullptr;
				p->_temp32 = 0;

				if (arg_bytes >= 4) {
					// HACKHACKHACK to check what the argument is.
					uint8 *argmem = new uint8[arg_bytes];
					uint8 *args = argmem;
					p->_stack.pop(args, 4);
					p->_stack.addSP(-4); // don't really pop the args
					ARG_UC_PTR(iptr);
					uint16 testItemId = ptrToObject(iptr);
					testItem = getItem(testItemId);
					delete [] argmem;
				}
				perr << "Unhandled intrinsic << " << func << " \'" << _convUse->intrinsics()[func] << "\'? (";
				if (testItem) {
					perr << "item " << testItem->getObjId();
					if (arg_bytes > 4)
						perr << " + " << arg_bytes - 4 << " bytes";
				} else {
					perr << arg_bytes << " bytes";
				}
				perr << ") called" << Std::endl;
				if (testItem) {
					testItem->dumpInfo();
				}
			} else {
				//!! hackish
				if (_intrinsics[func] == UCMachine::I_dummyProcess ||
				        _intrinsics[func] == UCMachine::I_true) {
//						perr << "Unhandled intrinsic \'" << _convUse->_intrinsics()[func] << "\' (" << ConsoleStream::hex << func << ConsoleStream::dec << ") called" << Std::endl;
				}
				uint8 *argbuf = new uint8[arg_bytes];
				p->_stack.pop(argbuf, arg_bytes);
				p->_stack.addSP(-arg_bytes); // don't really pop the args

				p->_temp32 = _intrinsics[func](argbuf, arg_bytes);

				delete[] argbuf;
			}

			// WORKAROUND: In U8, the flag 'startedConvo' [0000 01] which acts
			// as a mutex is set too late in the script, allowing two copies of
			// of the Ancient Ones script (each spawned by a different egg) to
			// run simultaneously. Set the flag when the avatar is put in stasis
			// to avoid this.
			// Original bug: https://sourceforge.net/p/pentagram/feature-requests/6/
			if (GAME_IS_U8 && p->_classId == 0x48B && func == 0xD0) {
				// 0xD0 = setAvatarInStasis
				_globals->setEntries(0, 1, 1);
			}
			break;
		}

		// 0x10 NEAR_ROUTINE_CALL (Unused in U8 and Crusader)

		case 0x11: {
			// 11 xx xx yy yy
			// Ultima 8:
			// call the function at offset yy yy of class xx xx
			// Crusader:
			// call function number yy yy of class xx xx
			uint16 new_classid = cs->readUint16LE();
			uint16 new_offset = cs->readUint16LE();
			LOGPF(("call\t\t%04X:%04X\n", new_classid, new_offset));
			if (GAME_IS_CRUSADER) {
				new_offset = p->_usecode->get_class_event(new_classid,
				             new_offset);
			}

			p->_ip = static_cast<uint16>(cs->pos());   // Truncates!!
			p->call(new_classid, new_offset);

			// Update the code segment
			uint32 base_ = p->_usecode->get_class_base_offset(p->_classId);
			delete cs;
			cs = new Common::MemoryReadStream(p->_usecode->get_class(p->_classId) + base_,
											  p->_usecode->get_class_size(p->_classId) - base_);
			cs->seek(p->_ip);

			// Resume execution
			break;
		}

		case 0x12:
			// 12
			// pop 16bits into temp register
			p->_temp32 = p->_stack.pop2();
			LOGPF(("pop\t\ttemp = %04X\n", (p->_temp32 & 0xFFFF)));
			break;

		case 0x13:
			// 13
			// pop 32bits into temp register. (Not actually used in U8 or Crusader)
			p->_temp32 = p->_stack.pop4();
			LOGPF(("pop long\t\ttemp = %08X\n", p->_temp32));
			break;

		// Arithmetic

		case 0x14:
			// 14
			// 16 bit add
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			p->_stack.push2(static_cast<uint16>(si16a + si16b));
			LOGPF(("add\n"));
			break;

		case 0x15:
			// 15
			// 32 bit add
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			p->_stack.push4(static_cast<uint32>(si32a + si32b));
			LOGPF(("add long\n"));
			break;

		case 0x16:
			// 16
			// pop two strings from the stack and push the concatenation
			// (free the originals? order?)
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			if (ui16b == 0) {
				perr << "Trying to append to string 0." << Std::endl;
				error = true;
				break;
			}
			_stringHeap[ui16b] += getString(ui16a);
			freeString(ui16a);
			p->_stack.push2(ui16b);
			LOGPF(("concat\t\t= %s\n", _stringHeap[ui16b].c_str()));
			break;

		case 0x17: {
			// 17
			// pop two lists from the stack and push the 'sum' of the lists
			// (freeing the originals)
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			UCList *listA = getList(ui16a);
			UCList *listB = getList(ui16b);

			if (listB && listA) {
				if (listA->getElementSize() != listB->getElementSize()) {
					perr << "Trying to append lists with different element "
					     << "sizes (" << listB->getElementSize() << " != "
					     << listA->getElementSize() << ")" << Std::endl;
					error = true;
				} else {
					listB->appendList(*listA);
				}
				// CHECKME: do we allow appending a list to itself?
				assert(ui16a != ui16b);
				freeList(ui16a);
				p->_stack.push2(ui16b);
			} else {
				// at least one of the lists didn't exist. Error or not?
				// for now: if one exists, push that one.
				// if neither exists, push 0.

				if (listA) {
					p->_stack.push2(ui16a);
				} else if (listB) {
					p->_stack.push2(ui16b);
				} else {
					p->_stack.push2(0);
				}
			}
			LOGPF(("append\n"));
			break;
		}
		// 0x18 EXCLUSIVE_ADD_LIST (Unused in U8 and Crusader)

		case 0x19: {
			// 19 02
			// add two stringlists, removing duplicates
			ui32a = cs->readByte();
			if (ui32a != 2) {
				perr << "Unhandled operand " << ui32a << " to union slist"
				     << Std::endl;
				error = true;
			}
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			UCList *srclist = getList(ui16a);
			UCList *dstlist = getList(ui16b);
			if (!srclist || !dstlist) {
				perr << "Invalid list param to union slist" << Std::endl;
				error = true;
			} else {
				dstlist->unionStringList(*srclist);
				freeStringList(ui16a); // contents are actually freed in unionSL
			}
			p->_stack.push2(ui16b);
			LOGPF(("union slist\t(%02X)\n", ui32a));
			break;
		}
		case 0x1A: {
			// 1A 02
			// subtract string list
			ui32a = cs->readByte(); // elementsize (always 02)
			ui32a = 2;
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			UCList *srclist = getList(ui16a);
			UCList *dstlist = getList(ui16b);
			if (!srclist || !dstlist) {
				perr << "Invalid list param to subtract slist" << Std::endl;
				error = true;
			} else {
				dstlist->subtractStringList(*srclist);
				freeStringList(ui16a);
			}
			p->_stack.push2(ui16b);
			LOGPF(("remove slist\t(%02X)\n", ui32a));
			break;
		}
		case 0x1B: {
			// 1B xx
			// pop two lists from the stack of element size xx and
			// remove the 2nd from the 1st
			// (free the originals? order?)
			ui32a = cs->readByte(); // elementsize
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			UCList *srclist = getList(ui16a);
			UCList *dstlist = getList(ui16b);
			if (!srclist || !dstlist) {
				perr << "Invalid list param to remove from slist" << Std::endl;
				error = true;
			} else {
				dstlist->subtractList(*srclist);
				freeList(ui16a);
			}
			p->_stack.push2(ui16b);
			LOGPF(("remove list\t(%02X)\n", ui32a));
			break;
		}
		case 0x1C:
			// 1C
			// subtract two 16 bit integers
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			p->_stack.push2(static_cast<uint16>(si16b - si16a));
			LOGPF(("sub\n"));
			break;

		case 0x1D:
			// 1D
			// subtract two 32 bit integers
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			p->_stack.push4(static_cast<uint32>(si32b - si32a));
			LOGPF(("sub long\n"));
			break;

		case 0x1E:
			// 1E
			// multiply two 16 bit integers
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			p->_stack.push2(static_cast<uint16>(si16a * si16b));
			LOGPF(("mul\n"));
			break;

		case 0x1F:
			// 1F
			// multiply two 32 bit integers
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			p->_stack.push4(static_cast<uint32>(si32a * si32b));
			LOGPF(("mul long\n"));
			break;

		case 0x20:
			// 20
			// divide two 16 bit integers
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16a != 0) {
				p->_stack.push2(static_cast<uint16>(si16b / si16a));
			} else {
				perr.Print("0x20 division by zero.\n");
				p->_stack.push2(0);
			}
			LOGPF(("div\n"));
			break;

		case 0x21:
			// 21
			// divide two 32 bit integers
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32a != 0) {
				p->_stack.push4(static_cast<uint32>(si32b / si32a));
			} else {
				perr.Print("0x21 division by zero.\n");
				p->_stack.push4(0);
			}
			LOGPF(("div\n"));
			break;

		case 0x22:
			// 22
			// 16 bit mod, b % a
			// Appears to be C-style %
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16a != 0) {
				p->_stack.push2(static_cast<uint16>(si16b % si16a));
			} else {
				perr.Print("0x22 division by zero.\n");
				p->_stack.push2(0);
			}
			LOGPF(("mod\n"));
			break;

		case 0x23:
			// 23
			// 32 bit mod
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32a != 0) {
				p->_stack.push4(static_cast<uint32>(si32b % si32a));
			} else {
				perr.Print("0x23 division by zero.\n");
				p->_stack.push4(0);
			}
			LOGPF(("mod long\n"));
			break;

		case 0x24:
			// 24
			// 16 bit cmp
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16a == si16b) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("cmp\n"));
			break;

		case 0x25:
			// 25
			// 32 bit cmp
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32a == si32b) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("cmp long\n"));
			break;

		case 0x26:
			// 26
			// compare two strings
			// (delete strings)
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			if (getString(ui16b) == getString(ui16a))
				p->_stack.push2(1);
			else
				p->_stack.push2(0);
			freeString(ui16a);
			freeString(ui16b);
			LOGPF(("strcmp\n"));
			break;

		// 0x27 EQUALS_HUGE (Unused in U8 and Crusader)

		case 0x28:
			// 28
			// 16 bit less-than
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16b < si16a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("lt\n"));
			break;

		case 0x29:
			// 29
			// 32 bit less-than
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32b < si32a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("lt long\n"));
			break;

		case 0x2A:
			// 2A
			// 16 bit less-or-equal
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16b <= si16a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("le\n"));
			break;

		case 0x2B:
			// 2B
			// 32 bit less-or-equal
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32b <= si32a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("le long\n"));
			break;

		case 0x2C:
			// 2C
			// 16 bit greater-than
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16b > si16a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("gt\n"));
			break;

		case 0x2D:
			// 2D
			// 32 bit greater-than
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32b > si32a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("gt long\n"));
			break;

		case 0x2E:
			// 2E
			// 16 bit greater-or-equal
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16b >= si16a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("ge\n"));
			break;

		case 0x2F:
			// 2F
			// 32 bit greater-or-equal
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32b >= si32a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("ge long\n"));
			break;

		case 0x30:
			// 30
			// 16 bit boolean not
			ui16a = p->_stack.pop2();
			if (!ui16a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("not\n"));
			break;

		case 0x31:
			// 31
			// 32 bit boolean not (not used in U8 or Crusader)
			ui32a = p->_stack.pop4();
			if (!ui32a) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("not long\n"));
			break;

		case 0x32:
			// 32
			// 16 bit logical and
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			if (ui16a && ui16b) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("and\n"));
			break;

		case 0x33:
			// 33
			// 32 bit logical and (not used in U8 or Crusader)
			ui32a = p->_stack.pop4();
			ui32b = p->_stack.pop4();
			if (ui32a && ui32b) {
				p->_stack.push4(1);
			} else {
				p->_stack.push4(0);
			}
			LOGPF(("and long\n"));
			break;

		case 0x34:
			// 34
			// 16 bit logical or
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			if (ui16a || ui16b) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("or\n"));
			break;

		case 0x35:
			// 35
			// 32 bit logical or (not used in U8 or Crusader)
			ui32a = p->_stack.pop4();
			ui32b = p->_stack.pop4();
			if (ui32a || ui32b) {
				p->_stack.push4(1);
			} else {
				p->_stack.push4(0);
			}
			LOGPF(("or long\n"));
			break;

		case 0x36:
			// 36
			// 16 bit not-equal
			si16a = static_cast<int16>(p->_stack.pop2());
			si16b = static_cast<int16>(p->_stack.pop2());
			if (si16a != si16b) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("ne\n"));
			break;

		case 0x37:
			// 37
			// 32 bit not-equal (only used in Crusader)
			si32a = static_cast<int32>(p->_stack.pop4());
			si32b = static_cast<int32>(p->_stack.pop4());
			if (si32a != si32b) {
				p->_stack.push2(1);
			} else {
				p->_stack.push2(0);
			}
			LOGPF(("ne long\n"));
			break;

		case 0x38: {
			// 38 xx yy
			// is element (size xx) in list? (or slist if yy is true)
			// free list/slist afterwards

			ui16a = cs->readByte();
			ui32a = cs->readByte();
			ui16b = p->_stack.pop2();
			UCList *l = getList(ui16b);
			if (!l) {
				perr << "Invalid list id " << ui16b << Std::endl;
				error = true;
			} else if (ui32a) { // stringlist
				if (ui16a != 2) {
					perr << "Unhandled operand " << ui16a << " to in slist"
					     << Std::endl;
					error = true;
				}
				if (l->stringInList(p->_stack.pop2()))
					p->_stack.push2(1);
				else
					p->_stack.push2(0);
				freeStringList(ui16b);
			} else {
				bool found = l->inList(p->_stack.access());
				p->_stack.addSP(ui16a);
				if (found)
					p->_stack.push2(1);
				else
					p->_stack.push2(0);

				freeList(ui16b);
			}
			LOGPF(("in list\t\t%s slist==%02X\n", print_bp(ui16a), ui32a));
			break;
		}
		case 0x39:
			// 39
			// 16 bit bitwise and
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			p->_stack.push2(ui16a & ui16b);
			LOGPF(("bit_and\n"));
			break;

		case 0x3A:
			// 3A
			// 16 bit bitwise or
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			p->_stack.push2(ui16a | ui16b);
			LOGPF(("bit_or\n"));
			break;

		case 0x3B:
			// 3B
			// 16 bit bitwise not
			ui16a = p->_stack.pop2();
			p->_stack.push2(~ui16a);
			LOGPF(("bit_not\n"));
			break;

		case 0x3C:
			// 3C
			// 16 bit left shift
			// operand order is different between U8 and crusader!
			if (GAME_IS_U8) {
				si16a = static_cast<int16>(p->_stack.pop2());
				ui16b = static_cast<int16>(p->_stack.pop2());
			} else {
				ui16b = static_cast<int16>(p->_stack.pop2());
				si16a = static_cast<int16>(p->_stack.pop2());
			}
			p->_stack.push2(static_cast<uint16>(si16a << ui16b));
			LOGPF(("lsh\t%04Xh >> %xh = %xh\n", si16a, ui16b, si16a << ui16b));
			break;

		case 0x3D:
			// 3D
			// 16 bit right shift (sign-extended - game uses SAR opcode)
			// operand order is different between U8 and crusader!
			if (GAME_IS_U8) {
				si16a = static_cast<int16>(p->_stack.pop2());
				ui16b = static_cast<int16>(p->_stack.pop2());
			} else {
				ui16b = static_cast<int16>(p->_stack.pop2());
				si16a = static_cast<int16>(p->_stack.pop2());
			}
			p->_stack.push2(static_cast<uint16>(si16a >> ui16b));
			LOGPF(("rsh\t%04Xh >> %xh = %xh\n", si16a, ui16b, si16a >> ui16b));
			break;

		case 0x3E:
			// 3E xx
			// push the value of the sign-extended 8 bit local var xx as 16 bit int
			si8a = cs->readSByte();
			ui16a = static_cast<uint16>(static_cast<int8>(p->_stack.access1(p->_bp + si8a)));
			p->_stack.push2(ui16a);
			LOGPF(("push byte\t%s = %02Xh\n", print_bp(si8a), ui16a));
			break;

		case 0x3F:
			// 3F xx
			// push the value of the 16 bit local var xx
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_bp + si8a);
			p->_stack.push2(ui16a);
			LOGPF(("push\t\t%s = %04Xh\n", print_bp(si8a), ui16a));
			break;

		case 0x40:
			// 40 xx
			// push the value of the 32 bit local var xx
			si8a = cs->readSByte();
			ui32a = p->_stack.access4(p->_bp + si8a);
			p->_stack.push4(ui32a);
			LOGPF(("push dword\t%s = %08Xh\n", print_bp(si8a), ui32a));
			break;

		case 0x41: {
			// 41 xx
			// push the string local var xx
			// duplicating the string?
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_bp + si8a);
			p->_stack.push2(duplicateString(ui16a));
			LOGPF(("push string\t%s\n", print_bp(si8a)));
			break;
		}
		case 0x42: {
			// 42 xx yy
			// push the list (with yy size elements) at BP+xx
			// duplicating the list?
			si8a = cs->readSByte();
			ui16a = cs->readByte();
			ui16b = p->_stack.access2(p->_bp + si8a);
			UCList *l = new UCList(ui16a);
			if (getList(ui16b)) {
				l->copyList(*getList(ui16b));
			} else {
				// trying to push non-existent list. Error or not?
				// Not: for example, function 01E3::0080, offset 0112
				// perr << "Pushing non-existent list" << Std::endl;
				// error = true;
			}
			uint16 newlistid = assignList(l);
			p->_stack.push2(newlistid);
			LOGPF(("push list\t%s (%04X, copy %04X, %d elements)\n",
			       print_bp(si8a), ui16b, newlistid, l->getSize()));
			break;
		}
		case 0x43: {
			// 43 xx
			// push the stringlist local var xx
			// duplicating the list, duplicating the strings in the list
			si8a = cs->readSByte();
			ui16a = 2;
			ui16b = p->_stack.access2(p->_bp + si8a);
			UCList *l = new UCList(ui16a);
			if (getList(ui16b)) {
				l->copyStringList(*getList(ui16b));
			} else {
				// trying to push non-existent list. Error or not?
				// (Devon's talk code seems to use it; so no error for now)
				// perr << "Pushing non-existent slist" << Std::endl;
				// error = true;
			}
			p->_stack.push2(assignList(l));
			LOGPF(("push slist\t%s\n", print_bp(si8a)));
			break;
		}
		case 0x44: {
			// 44 xx yy
			// push element from the second last var pushed onto the stack
			// (a list/slist), indexed by the last element pushed onto the list
			// (a byte/word). XX is the size of the types contained in the list
			// YY is true if it's a slist (for garbage collection)

			// duplicate string if YY? yy = 1 only occurs
			// in two places in U8: once it pops into temp afterwards,
			// once it is indeed freed. So, guessing we should duplicate.
			ui32a = cs->readByte();
			ui32b = cs->readByte();
			ui16a = p->_stack.pop2() - 1; // index
			ui16b = p->_stack.pop2(); // list
			UCList *l = getList(ui16b);
			if (!l) {
//				perr << "push element from invalid list (" << ui16b << ")"
//					 << Std::endl;
				// This is necessary for closing the backpack to work
				p->_stack.push0(ui32a);
//				error = true;
			} else {
				if (ui32b) {
					uint16 s = l->getStringIndex(ui16a);
					p->_stack.push2(duplicateString(s));
				} else {
					if (ui16a < l->getSize()) {
						p->_stack.push((*l)[ui16a], ui32a);
					} else {
						// WORKAROUND
						warning("ignore 0x44 request to push %d from list len %d", ui16a, l->getSize());
					}
				}
			}
			LOGPF(("push element\t%02X slist==%02X\n", ui32a, ui32b));
			break;
		}
		case 0x45:
			// 45 xx yy
			// push huge of size yy from BP+xx
			si8a = cs->readSByte();
			ui16b = cs->readByte();
			p->_stack.push(p->_stack.access(p->_bp + si8a), ui16b);
			LOGPF(("push huge\t%s %02X\n", print_bp(si8a), ui16b));
			break;

		// 0x46 BYTE_MEMBER_REFERENCE (Unused)
		// 0x47 INT_MEMBER_REFERENCE (Unused)
		// 0x48 LONG_MEMBER_REFERENCE (Unused)
		// 0x49 HUGE_MEMBER_REFERENCE (Unused)
		// 0x4a THIS_REFERENCE (Unused)

		case 0x4B:
			// 4B xx
			// push 32 bit pointer address of BP+XX
			si8a = cs->readSByte();
			p->_stack.push4(stackToPtr(p->_pid, p->_bp + si8a));
			LOGPF(("push addr\t%s\n", print_bp(si8a)));
			break;

		case 0x4C: {
			// 4C xx
			// indirect push,
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			ui16a = cs->readByte();
			ui32a = p->_stack.pop4();

			p->_stack.addSP(-ui16a);
			if (!dereferencePointer(ui32a,
			                        p->_stack.access(),
			                        ui16a))
				error = true;

			LOGPF(("push indirect\t%02Xh bytes", ui16a));
			if (!error && ui16a == 2) {
				LOGPF((" = %04Xh\n", p->_stack.access2(p->_stack.getSP())));
			} else {
				LOGPF(("\n"));
			}
			break;
		}

		case 0x4D: {
			// 4D xx
			// indirect pop
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			ui16a = cs->readByte();
			ui32a = p->_stack.pop4();

			if (assignPointer(ui32a, p->_stack.access(), ui16a)) {
				p->_stack.addSP(ui16a);
			} else {
				error = true;
			}

			LOGPF(("pop indirect\t%02Xh bytes\n", ui16a));
			break;
		}

		case 0x4E:
			// 4E xx xx yy
			// push global xxxx size yy bits
			ui16a = cs->readUint16LE();
			ui16b = cs->readByte();
			ui32a = _globals->getEntries(ui16a, ui16b);
			p->_stack.push2(static_cast<uint16>(ui32a));
			LOGPF(("push\t\tglobal [%04X %02X] = %02X\n", ui16a, ui16b, ui32a));
			break;

		case 0x4F:
			// 4F xx xx yy
			// pop value into global xxxx size yy bits
			ui16a = cs->readUint16LE();	// pos
			ui16b = cs->readByte();		// len
			ui32a = p->_stack.pop2();	// val
			_globals->setEntries(ui16a, ui16b, ui32a);

			if ((GAME_IS_U8 && (ui32a & ~(((1 << ui16b) - 1)))) || (GAME_IS_CRUSADER && (ui16b > 2))) {
				perr << "Warning: value popped into a flag it doesn't fit in (" << ConsoleStream::hex
					 << ui16a << " " << ui16b << " " << ui32a << ")" << Std::endl;
			}

			// paranoid :-)
			if (GAME_IS_U8) {
				assert(_globals->getEntries(ui16a, ui16b) == (ui32a & ((1 << ui16b) - 1)));
			} else {
				assert(_globals->getEntries(ui16a, ui16b) == ui32a);
		    }

			LOGPF(("pop\t\tglobal [%04X %02X] = %02X\n", ui16a, ui16b, ui32a));
			break;

		case 0x50:
			// 50
			// return from function
			if (p->ret()) { // returning from process
				LOGPF(("ret\t\tfrom process\n"));
				p->terminateDeferred();

				// return value is going to be stored somewhere,
				// and some other process is probably waiting for it.
				// So, we can't delete ourselves just yet.
			} else {
				LOGPF(("ret\t\tto %04X:%04X\n", p->_classId, p->_ip));

				// return value is stored in _temp32 register

				// Update the code segment
				uint32 base_ = p->_usecode->get_class_base_offset(p->_classId);

				delete cs;
				cs = new Common::MemoryReadStream(p->_usecode->get_class(p->_classId) + base_,
												  p->_usecode->get_class_size(p->_classId) - base_);
				cs->seek(p->_ip);
			}

			// Resume execution
			break;

		case 0x51:
			// 51 xx xx
			// relative jump to xxxx if false
			si16a = static_cast<int16>(cs->readUint16LE());
			ui16b = p->_stack.pop2();
			if (!ui16b) {
				ui16a = cs->pos() + si16a;
				cs->seek(ui16a);
				LOGPF(("jne\t\t%04hXh\t(to %04X) (taken)\n", si16a,
				       cs->pos()));
			} else {
				LOGPF(("jne\t\t%04hXh\t(to %04X) (not taken)\n", si16a,
				       cs->pos()));
			}
			break;

		case 0x52:
			// 52 xx xx
			// relative jump to xxxx
			si16a = static_cast<int16>(cs->readUint16LE());
			ui16a = cs->pos() + si16a;
			cs->seek(ui16a);
			LOGPF(("jmp\t\t%04hXh\t(to %04X)\n", si16a, cs->pos()));
			break;

		case 0x53:
			// 53
			// suspend
			LOGPF(("suspend\n"));
			go_until_cede = false;
			cede = true;
			break;

		case 0x54: {
			// 54 01 01
			// implies
			// Links two processes (two pids are popped) a and b, meaning
			// b->waitfor(a)
			//
			// In the disassembly, '01 01' is the number of processes to
			// pop, but in practice only ever appears as 01 01.
			//
			// pid a is often the current pid in U8

			// 'implies' seems to push a value too, although it is very
			// often ignored. It looks like it's a pid, but which one?

			// additionally, it is possible that 'implies' puts the result
			// of a process in the 'process result' variable,
			// or more likely, when a process finishes, it sets the result
			// value of the processes that were waiting for it.
			// 0x6D (push process result) only seems to occur soon after
			// an 'implies'

			cs->readUint16LE(); // skip the 01 01
			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();
			p->_stack.push2(ui16a); //!! which pid do we need to push!?
			LOGPF(("implies\n"));

			Process *proc = Kernel::get_instance()->getProcess(ui16b);
			Process *proc2 = Kernel::get_instance()->getProcess(ui16a);
			if (proc && proc2) {
				proc->waitFor(ui16a);
				// The proc is now marked suspended, but finish this execution
				// until we hit a suspend or return.
				go_until_cede = true;
			} else {
				perr << "Non-existent process PID (";
				if (!proc && !proc2) {
					perr << ui16a << "," << ui16b;
				} else if (!proc) {
					perr << ui16b;
				} else {
					perr << ui16a;
				}
				perr << ") in implies." << Std::endl;
				// This condition triggers in 057C:1090 when talking
				// to a child (class 02C4), directly after the conversation
				// Specifically, it occurs because there is no
				// leaveFastArea usecode for class 02C4.
				// So currently we only regard this as an error when the
				// missing process wasn't PID 0.
				if ((ui16a && !proc2) || (ui16b && !proc))
					error = true;
			}
			break;
		}

		// 0x55: AND_IMPLIES (only does push 0x402 in disasm, unused in U8 and Crusader)
		// 0x56: OR_IMPLIES (only does push 0x404 in disasm, unused in U8 and Crusader)

		case 0x57: {
			// 57 aa tt xx xx yy yy
			// spawn process function yyyy in class xxxx
			// aa = number of arg bytes pushed (not including this pointer which is 4 bytes)
			// tt = sizeof this pointer object
			// only remove the this pointer from stack (4 bytes)
			// put PID of spawned process in temp
			int arg_bytes = cs->readByte();
			int this_size = cs->readByte();
			uint16 classid = cs->readUint16LE();
			uint16 offset = cs->readUint16LE();

			uint32 thisptr = p->_stack.pop4();

			LOGPF(("spawn\t\t%02X %02X %04X:%04X\n",
			       arg_bytes, this_size, classid, offset));

			if (GAME_IS_CRUSADER) {
				offset = p->_usecode->get_class_event(classid, offset);
			}

			UCProcess *newproc = new UCProcess(classid, offset,
			                                   thisptr,
			                                   this_size,
			                                   p->_stack.access(),
			                                   arg_bytes);
			// Note: order of execution of this process and the new one is
			// relevant. Currently, the spawned processes is executed once
			// immediately, after which the current process resumes
			p->_temp32 = Kernel::get_instance()->addProcessExec(newproc);

#ifdef DEBUG
			if (trace_show(p->_pid, p->_itemNum, p->_classId)) {
				pout << "tick " << Kernel::get_instance()->getTickNum()
				     << " (still) running process " << ConsoleStream::hex << p->_pid
				     << ", item " << p->_itemNum << ", type " << p->_type
				     << ", class " << p->_classId << ", offset " << p->_ip
				     << ConsoleStream::dec << Std::endl;
			}
#endif
			break;
		}

		case 0x58: {
			// 58 xx xx yy yy zz zz tt uu
			// spawn inline process function yyyy in class xxxx at offset zzzz
			// tt = size of this pointer
			// uu = unknown (occurring values: 00, 02, 05) - seems unused in original
			uint16 classid = cs->readUint16LE();
			uint16 offset = cs->readUint16LE();
			uint16 delta = cs->readUint16LE();
			int this_size = cs->readByte();
			int unknown = cs->readByte(); // ??

			// This only gets used in U8.  If it were used in Crusader it would
			// need the offset translation done in 0x57.
			assert(GAME_IS_U8);

			debug(MM_INFO, "spawn inline\t%04X:%04X+%04X=%04X %02X %02X\n",
				classid, offset, delta, offset + delta, this_size, unknown);

			uint32 thisptr = 0;
			if (this_size > 0)
				thisptr = p->_stack.access4(p->_bp + 6);
			UCProcess *newproc = new UCProcess(classid, offset + delta,
			                                   thisptr, this_size);

			// as with 'spawn', run the spawned process once immediately
			uint16 newpid = Kernel::get_instance()->addProcessExec(newproc);

#ifdef DEBUG
			if (trace_show(p->_pid, p->_itemNum, p->_classId)) {
				pout << "tick " << Kernel::get_instance()->getTickNum()
					 << ConsoleStream::hex << " (still) running process " << p->_pid
				     << ", item " << p->_itemNum << ", class " << p->_classId
				     << ", offset " << p->_ip << ConsoleStream::dec << Std::endl;
			}
#endif
			p->_stack.push2(newpid); //! push pid of new proc
			break;
		}

		case 0x59:
			// 59
			// push current process id
			p->_stack.push2(p->_pid);
			LOGPF(("push\t\tpid = %04Xh\n", p->_pid));
			break;

		case 0x5A:
			// 5A xx
			// init function. xx = local var size
			// sets xx bytes on stack to 0, moving sp
			ui16a = cs->readByte();
			LOGPF(("init\t\t%02X\n", ui16a));

			if (ui16a & 1) ui16a++; // 16-bit align
			if (ui16a > 0) {
				p->_stack.push0(ui16a);
			}
			break;

		case 0x5B:
			// 5B xx xx
			// debug line no xx xx
			ui16a = cs->readUint16LE(); // source line number
			debug(10, "ignore debug opcode %02X: line offset %d", opcode, ui16a);
			LOGPF(("line number %d\n", ui16a));
			break;

		case 0x5C: {
			// 5C xx xx char[9]
			// debug line no xx xx in class str
			ui16a = cs->readUint16LE(); // source line number
			char name[10] = {0};
			for (int x = 0; x < 9; x++) {
				// skip over class name and null terminator
				name[x] = cs->readByte();
			}
			LOGPF(("line number %s %d\n", name, ui16a));
			debug(10, "ignore debug opcode %02X: %s line offset %d", opcode, name, ui16a);
			break;
		}

		case 0x5D:
			// 5D
			// push 8 bit value returned from function call
			// (push temp8 as 16 bit value)
			p->_stack.push2(static_cast<uint8>(p->_temp32 & 0xFF));
			LOGPF(("push byte\tretval = %02Xh\n", (p->_temp32 & 0xFF)));
			break;

		case 0x5E:
			// 5E
			// push 16 bit value returned from function call
			// (push temp16)
			p->_stack.push2(static_cast<uint16>(p->_temp32 & 0xFFFF));
			LOGPF(("push\t\tretval = %04Xh\n", (p->_temp32 & 0xFFFF)));
			break;

		case 0x5F:
			// 5F
			// push 32 bit value returned from function call
			// (push _temp32)
			p->_stack.push4(p->_temp32);
			LOGPF(("push long\t\tretval = %08Xh\n", p->_temp32));
			break;

		case 0x60:
			// 60
			// convert 16-bit to 32-bit int (sign extend)
			si32a = static_cast<int16>(p->_stack.pop2());
			p->_stack.push4(si32a);
			LOGPF(("int to long\n"));
			break;

		case 0x61:
			// 61
			// convert 32-bit to 16-bit int
			si16a = static_cast<int16>(p->_stack.pop4());
			p->_stack.push2(si16a);
			LOGPF(("long to int\n"));
			break;

		case 0x62:
			// 62 xx
			// free the string in var BP+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_bp + si8a);
			freeString(ui16a);
			LOGPF(("free string\t%s = %04X\n", print_bp(si8a), ui16a));
			break;

		case 0x63:
			// 63 xx
			// free the stringlist in var BP+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_bp + si8a);
			freeStringList(ui16a);
			LOGPF(("free slist\t%s = %04X\n", print_bp(si8a), ui16a));
			break;

		case 0x64:
			// 64 xx
			// free the list in var BP+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_bp + si8a);
			freeList(ui16a);
			LOGPF(("free list\t%s = %04X\n", print_bp(si8a), ui16a));
			break;

		case 0x65:
			// 65 xx
			// free the string at SP+xx
			// NB: sometimes there's a 32-bit string pointer at SP+xx
			//     However, the low word of this is exactly the 16bit ref
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_stack.getSP() + si8a);
			freeString(ui16a);
			LOGPF(("free string\t%s = %04X\n", print_sp(si8a), ui16a));
			break;

		case 0x66:
			// 66 xx
			// free the list at SP+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_stack.getSP() + si8a);
			freeList(ui16a);
			LOGPF(("free list\t%s = %04X\n", print_sp(si8a), ui16a));
			break;

		case 0x67:
			// 67 xx
			// free the string list at SP+xx
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_stack.getSP() + si8a);
			freeStringList(ui16a);
			LOGPF(("free slist\t%s = %04x\n", print_sp(si8a), ui16a));
			break;

		// 0x68 COPY_STRING (unused in U8 and Crusader)

		case 0x69:
			// 69 xx
			// push the string in var BP+xx as 32 bit pointer
			si8a = cs->readSByte();
			ui16a = p->_stack.access2(p->_bp + si8a);
			p->_stack.push4(stringToPtr(ui16a));
			LOGPF(("str to ptr\t%s\n", print_bp(si8a)));
			break;

		// 0x6A Convert pointer to string (unused in U8 and Crusader)

		case 0x6B:
			// 6B
			// pop a string and push 32 bit pointer to string
			ui16a = p->_stack.pop2();
			p->_stack.push4(stringToPtr(ui16a));
			LOGPF(("str to ptr\n"));
			break;

		case 0x6C:
			// 6C xx yy
			// yy = type (01 = string, 02 = slist, 03 = list)
			// copy the (string/slist/list) in BP+xx to the current process,
			// and add it to the "Free Me" list of the process
			si8a = cs->readByte(); // index
			ui8a = cs->readByte(); // type
			LOGPF(("param _pid chg\t%s, type=%u\n", print_bp(si8a), ui8a));

			ui16a = p->_stack.access2(p->_bp + si8a);
			switch (ui8a) {
			case 1: // string
				// copy string
				ui16b = duplicateString(ui16a);
				break;
			case 2: { // slist
				UCList *l = new UCList(2);
				const UCList *srclist = getList(ui16a);
				if (!srclist) {
					perr << "Warning: invalid src list passed to slist copy"
						 << Std::endl;
					ui16b = 0;
					delete l;
					break;
				}
				l->copyStringList(*srclist);
				ui16b = assignList(l);
			}
			break;
			case 3: { // list
				const UCList *l = getList(ui16a);
				if (!l) {
					perr << "Warning: invalid src list passed to list copy"
						 << Std::endl;
					ui16b = 0;
					break;
				}
				int elementsize = l->getElementSize();
				UCList *l2 = new UCList(elementsize);
				l2->copyList(*l);
				ui16b = assignList(l2);
			}
			break;
			default:
				ui16b = 0;
				perr << "Error: invalid param _pid change type (" << ui8a
				     << ")" << Std::endl;
				error = true;
			}
			p->_stack.assign2(p->_bp + si8a, ui16b); // assign new index
			p->freeOnTerminate(ui16b, ui8a); // free new var when terminating
			break;

		case 0x6D:
			// 6D
			// push 32bit result of current process
			LOGPF(("push dword\tprocess result\n"));
			p->_stack.push4(p->_result);
			break;

		case 0x6E:
			// 6E xx
			// subtract xx from stack pointer
			// (effect on SP is the same as popping xx bytes)
			si8a = cs->readSByte();
			p->_stack.addSP(-si8a);
			LOGPF(("move sp\t\t%s%02Xh\n", si8a < 0 ? "-" : "", si8a < 0 ? -si8a : si8a));
			break;

		case 0x6F:
			// 6F xx
			// push 32 pointer address of SP-xx
			si8a = cs->readSByte();
			p->_stack.push4(stackToPtr(p->_pid, static_cast<uint16>(p->_stack.getSP() - si8a)));
			LOGPF(("push addr\t%s\n", print_sp(-si8a)));
			break;

		// loop-related opcodes
		// 0x70 has different types:
		//    02: search the area around an object
		//    03: search the area around an object, recursing into containers
		//    04: search a container
		//    05: search a container, recursing into containers
		//    06: something about looking for items on top of another (??)
		// each of these types allocate a rather large area on the stack
		// we expect SP to be at the end of that area when 0x73 is executed
		// a 'loop script' (created by 0x74) is used to select items

		case 0x70: {
			// 70 xx yy zz
			// loop something. Stores 'current object' in var xx
			// yy == num bytes in string
			// zz == type
			si16a = cs->readSByte();
			uint32 scriptsize = cs->readByte();
			uint32 searchtype = cs->readByte();

			ui16a = p->_stack.pop2();
			ui16b = p->_stack.pop2();

			//!! This may not be the way the original did things...

			// We'll first create a list of all matching items.
			// Store the id of this list in the last two bytes
			// of our stack area.
			// Behind that we'll store an index into this list.
			// This is followed by the variable in which to store the item
			// After that we store the loopscript length followed by
			// the loopscript itself.
			//   (Note that this puts a limit on the max. size of the
			//    loopscript of 0x20 bytes)

			if (scriptsize > 0x20) {
				perr << "Loopscript too long" << Std::endl;
				error = true;
				break;
			}

			uint8 *script = new uint8[scriptsize];
			p->_stack.pop(script, scriptsize);

			uint32 stacksize = 0;
			bool recurse = false;
			// we'll put everything on the stack after stacksize is set

			UCList *itemlist = new UCList(2);

			World *world = World::get_instance();

			switch (searchtype) {
			case 2:
			case 3: {
				// area search (3 = recursive)
				stacksize = GAME_IS_U8 ? 0x34 : 0x3A;
				if (searchtype == 3) recurse = true;

				// ui16a = item, ui16b = range
				const Item *item = getItem(ui16a);
				const uint16 range = GAME_IS_CRUSADER ? ui16b * 2 : ui16b;

				if (item) {
					int32 ix, iy, iz;
					item->getLocationAbsolute(ix, iy, iz);
					world->getCurrentMap()->areaSearch(itemlist, script,
					                                   scriptsize, nullptr,
					                                   range, recurse, ix, iy);
				} else {
					// return error or return empty list?
					perr << "Warning: invalid item " << ui16a << " passed to area search"
					     << Std::endl;
				}
				break;
			}
			case 4:
			case 5: {
				// container search (5 = recursive)
				stacksize = GAME_IS_U8 ? 0x28 : 0x2A;
				if (searchtype == 5) {
					stacksize += 2;
					recurse = true;
				}

				// ui16a = 0xFFFF (?), ui16b = container
				Container *container = getContainer(ui16b);

				if (ui16a != 0xFFFF) {
					perr << "Warning: non-FFFF value passed to "
					     << "container search" << Std::endl;
				}

				if (container) {
					container->containerSearch(itemlist, script,
					                           scriptsize, recurse);
				} else {
					// return error or return empty list?
					perr << "Warning: invalid container "<< ui16b << " passed to "
					     << "container search" << Std::endl;
				}
				break;
			}
			case 6: {
				// Surface search
				stacksize = GAME_IS_U8 ? 0x3D : 0x43;

				bool above = ui16a != 0xFFFF;
				bool below = ui16b != 0xFFFF;
				Item *item = getItem(below ? ui16b : ui16a);

				if (item) {
					world->getCurrentMap()->surfaceSearch(itemlist, script,
					                                      scriptsize, item,
					                                      above, below);
				} else {
					// return error or return empty list?
					perr << "Warning: invalid item passed to surface search"
					     << Std::endl;
				}
				break;
			}
			default:
				perr << "Unhandled search type " << searchtype << Std::endl;
				error = true;
				delete[] script;
				script = nullptr;
				break;
			}

			if (script != nullptr) {
				p->_stack.push0(stacksize - scriptsize - 8); // filler
				p->_stack.push(script, scriptsize);
				p->_stack.push2(scriptsize);
				p->_stack.push2(static_cast<uint16>(si16a));
				p->_stack.push2(0);
				uint16 itemlistID = assignList(itemlist);
				p->_stack.push2(itemlistID);

				delete[] script;

				LOGPF(("loop\t\t%s %02X %02X\n", print_bp(si16a),
					   scriptsize, searchtype));
			}
		}
		// Intentional fall-through

		// 0x71 SEARCH_RECURSIVE (Unused)
		// 0x72 SEARCH_SURFACE (Unused)

		case 0x73: {
			// 73
			// next loop object. pushes false if end reached
			unsigned int sp = p->_stack.getSP();
			uint16 itemlistID = p->_stack.access2(sp);
			UCList *itemlist = getList(itemlistID);
			uint16 index = p->_stack.access2(sp + 2);
			si16a = static_cast<int16>(p->_stack.access2(sp + 4));

			if (!itemlist) {
				perr << "Invalid item list in loopnext!" << Std::endl;
				error = true;
				break;
			}

			// see if there are still valid items left
			bool valid = false;
			do {
				if (index >= itemlist->getSize()) {
					break;
				}

				p->_stack.assign(p->_bp + si16a, (*itemlist)[index], 2);
				uint16 objid = p->_stack.access2(p->_bp + si16a);
				Item *item = getItem(objid);
				if (item) {
					valid = true;
				}

				if (!valid) index++;

			} while (!valid);

			if (!valid) {
				p->_stack.push2(0); // end of loop
				freeList(itemlistID);
			} else {
				p->_stack.push2(1);
				// increment index
				p->_stack.assign2(sp + 2, index + 1);
			}

			if (opcode == 0x73) { // because of the fall-through
				LOGPF(("loopnext\n"));
			}
			break;
		}

		case 0x74:
			// 74 xx
			// add xx to the current 'loopscript'
			ui8a = cs->readByte();
			p->_stack.push1(ui8a);
			LOGPF(("loopscr\t\t%02X \"%c\"\n", ui8a, static_cast<char>(ui8a)));
			break;

		case 0x75:
		case 0x76:
			// 75 xx yy zz zz  (foreach list)
			// 76 xx yy zz zz  (foreach string list)
			// xx is the stack offset to store 'current' value from the list
			//   (BP+xx)
			// yy is the 'datasize' of the list, identical to the second parameter
			//   of the create list/slist opcodes
			// zzzz is the offset to jump to after it's finished iteration
			//	 (the opcode before is always a 'jmp' to the start of the loop)
			// 2 16 bit values are on the stack and left there during each
			//   iteration:
			//   - loop index (always starts at 0xffff), updated each iteration
			//   - list id

			// 75 is for lists, 76 for slists
			// The only difference should be in the freeing afterwards.
			// Strings are _not_ duplicated when putting them in the loopvar
			// Lists _are_ freed afterwards

			si8a = cs->readByte();  // loop variable
			ui32a = cs->readByte(); // list size
			si16a = cs->readUint16LE(); // jump offset

			ui16a = p->_stack.access2(p->_stack.getSP());     // Loop index
			ui16b = p->_stack.access2(p->_stack.getSP() + 2); // Loop list

			if (opcode == 0x76 && ui32a != 2) {
				error = true;
			}

			if (opcode == 0x75) {
				LOGPF(("for each\t%s (%02X) %04hX\n",
				       print_bp(si8a), ui32a, si16a));
			} else {
				LOGPF(("for each str\t%s (%02X) %04hX\n",
				       print_bp(si8a), ui32a, si16a));
			}

			// Increment the counter
			if (ui16a == 0xFFFF) ui16a = 0;
			else ui16a++;

			if (ui16a >= getList(ui16b)->getSize()) {
				// loop done

				// free loop list
				if (opcode == 0x75) {
					freeList(ui16b);
				} else {
					freeStringList(ui16b);
				}

				p->_stack.addSP(4);  // Pop list and counter

				// jump out
				ui16a = cs->pos() + si16a;
				cs->seek(ui16a);
			} else {
				// loop iteration
				// (not duplicating any strings)

				// updated loop index
				p->_stack.assign2(p->_stack.getSP(), ui16a);

				// place next element from list in [bp+si8a]
				p->_stack.assign(p->_bp + si8a, (*getList(ui16b))[ui16a], ui32a);
			}
			break;

		case 0x77:
			// 77
			// set info
			// assigns item number and ProcessType
			p->setItemNum(p->_stack.pop2());
			p->setType(p->_stack.pop2());
			LOGPF(("set info itemno: %d type: %d\n", p->getItemNum(), p->getType()));
			break;

		case 0x78:
			// 78
			// process exclude
			// process gets 'exclusive access' to this (object,type)

			// Educated guess:
			// Check if any other processes have the same (object,type) info
			// set. If so, return from process.
			LOGPF(("process exclude"));

			if (Kernel::get_instance()->
			        getNumProcesses(p->_itemNum, p->_type) > 1) {
				// another process with this (object,type) is already running
				p->terminateDeferred();
				LOGPF(("\t(terminating)\n"));
			} else {
				LOGPF(("\n"));
			}

			break;

		case 0x79:
			// 79
			// push address of global (Crusader only)
			ui16a = cs->readUint16LE(); // global address
			ui32a = globalToPtr(ui16a);
			p->_stack.push4(ui32a);
			LOGPF(("push global 0x%x (value: %x)\n", ui16a, ui32a));
			break;

		case 0x7A:
			// 7A
			// end of function
			// shouldn't happen
			LOGPF(("end\n"));
			perr.Print("end of function opcode %02X reached!\n", opcode);
			error = true;
			break;

		// 0x7B REGRESS (Unused)

		default:
			perr.Print("unhandled opcode %02X\n", opcode);

		} // switch(opcode)

		// write back IP (but preserve IP if there was an error)
		if (!error)
			p->_ip = static_cast<uint16>(cs->pos());   // TRUNCATES!

		// check if we suspended ourselves
		if ((p->_flags & Process::PROC_SUSPENDED) != 0 && !go_until_cede)
			cede = true;
	} // while(!cede && !error && !p->terminated && !p->terminate_deferred)

	delete cs;

	if (error) {
		perr.Print("Process %d caused an error at %04X:%04X (item %d). Killing process.\n",
		            p->_pid, p->_classId, p->_ip, p->_itemNum);
		p->terminateDeferred();
	}
}


const Std::string &UCMachine::getString(uint16 str) const {
	static const Std::string emptystring("");

	Common::HashMap<uint16, Std::string>::const_iterator iter =
			_stringHeap.find(str);

	if (iter != _stringHeap.end())
		return iter->_value;

	return emptystring;
}

UCList *UCMachine::getList(uint16 l) {
	Common::HashMap<uint16, UCList *>::iterator iter = _listHeap.find(l);

	if (iter != _listHeap.end())
		return iter->_value;

	return nullptr;
}



uint16 UCMachine::assignString(const char *str) {
	uint16 id = _stringIDs->getNewID();
	if (id == 0) return 0;

	_stringHeap[id] = str;

	return id;
}

uint16 UCMachine::duplicateString(uint16 str) {
	return assignString(_stringHeap[str].c_str());
}


uint16 UCMachine::assignList(UCList *l) {
	uint16 id = _listIDs->getNewID();
	if (id == 0) return 0;
	assert(_listHeap.find(id) == _listHeap.end());

	_listHeap[id] = l;

	return id;
}

void UCMachine::freeString(uint16 s) {
	//! There's still a semi-bug in some places that string 0 can be assigned
	//! (when something accesses _stringHeap[0])
	//! This may not be desirable, but OTOH the created string will be
	//! empty, so not too much of a problem.
	Common::HashMap<uint16, Std::string>::iterator iter = _stringHeap.find(s);
	if (iter != _stringHeap.end()) {
		_stringHeap.erase(iter);
		_stringIDs->clearID(s);
	}
}

void UCMachine::freeList(uint16 l) {
	Common::HashMap<uint16, UCList *>::iterator iter = _listHeap.find(l);
	if (iter != _listHeap.end() && iter->_value) {
		iter->_value->free();
		delete iter->_value;
		_listHeap.erase(iter);
		_listIDs->clearID(l);
	}
}

void UCMachine::freeStringList(uint16 l) {
	Common::HashMap<uint16, UCList *>::iterator iter = _listHeap.find(l);
	if (iter != _listHeap.end() && iter->_value) {
		iter->_value->freeStrings();
		delete iter->_value;
		_listHeap.erase(iter);
		_listIDs->clearID(l);
	}
}

//static
uint32 UCMachine::listToPtr(uint16 l) {
	uint32 ptr = SEG_LIST;
	ptr <<= 16;
	ptr += l;
	return ptr;
}

//static
uint32 UCMachine::stringToPtr(uint16 s) {
	uint32 ptr = SEG_STRING;
	ptr <<= 16;
	ptr += s;
	return ptr;
}

//static
uint32 UCMachine::stackToPtr(uint16 _pid, uint16 offset) {
	uint32 ptr = SEG_STACK + _pid;
	ptr <<= 16;
	ptr += offset;
	return ptr;
}

//static
uint32 UCMachine::globalToPtr(uint16 offset) {
	uint32 ptr = SEG_GLOBAL;
	ptr <<= 16;
	ptr += offset;
	return ptr;
}

//static
uint32 UCMachine::objectToPtr(uint16 objID) {
	uint32 ptr = SEG_OBJ;
	ptr <<= 16;
	ptr += objID;
	return ptr;
}

bool UCMachine::assignPointer(uint32 ptr, const uint8 *data, uint32 size) {
	// Only implemented the following:
	// * stack pointers
	// * global pointers


	//! range checking...

	uint16 segment = static_cast<uint16>(ptr >> 16);
	uint16 offset = static_cast<uint16>(ptr & 0xFFFF);

	if (segment >= SEG_STACK_FIRST && segment <= SEG_STACK_LAST) {
		UCProcess *proc = dynamic_cast<UCProcess *>
		                  (Kernel::get_instance()->getProcess(segment));

		// reference to the stack of _pid 'segment'
		if (!proc) {
			// segfault :-)
			perr << "Trying to access stack of non-existent "
			     << "process (pid: " << segment << ")" << Std::endl;
			return false;
		} else {
			proc->_stack.assign(offset, data, size);
		}
	} else if (segment == SEG_GLOBAL) {
		if (!GAME_IS_CRUSADER)
			CANT_HAPPEN_MSG("Global pointers not supported in U8");

		if (size == 1) {
			_globals->setEntries(offset, 1, data[0]);
		} else if (size == 2) {
			uint16 val = ((data[1] << 8) | data[0]);
			_globals->setEntries(offset, 2, val);
		} else {
			CANT_HAPPEN_MSG("Global pointers must be size 1 or 2");
		}
	} else {
		perr << "Trying to access segment " << ConsoleStream::hex
		     << segment << ConsoleStream::dec << Std::endl;
		return false;
	}

	return true;
}

bool UCMachine::dereferencePointer(uint32 ptr, uint8 *data, uint32 size) {
	// this one is a bit tricky. There's no way we can support
	// all possible pointers, so we're just going to do a few:
	// * stack pointers
	// * object pointers, as long as xx == 02. (i.e., get objref)
	// * global pointers


	//! range checking...

	uint16 segment = static_cast<uint16>(ptr >> 16);
	uint16 offset = static_cast<uint16>(ptr & 0xFFFF);

	if (segment >= SEG_STACK_FIRST && segment <= SEG_STACK_LAST) {
		UCProcess *proc = dynamic_cast<UCProcess *>
		                  (Kernel::get_instance()->getProcess(segment));

		// reference to the stack of _pid 'segment'
		if (!proc) {
			// segfault :-)
			perr << "Trying to access stack of non-existent "
			     << "process (pid: " << segment << ")" << Std::endl;
			return false;
		} else {
			memcpy(data, proc->_stack.access(offset), size);
		}
	} else if (segment == SEG_OBJ) {
		if (size != 2) {
			perr << "Trying to read other than 2 bytes from objptr"
			     << Std::endl;
			return false;
		} else {
			// push objref
			data[0] = static_cast<uint8>(offset);
			data[1] = static_cast<uint8>(offset >> 8);
		}
	} else if (segment == SEG_GLOBAL) {
		if (!GAME_IS_CRUSADER)
			CANT_HAPPEN_MSG("Global pointers not supported in U8");

		if (size == 1) {
			data[0] = static_cast<uint8>(_globals->getEntries(offset, 1));
		} else if (size == 2) {
			uint16 val = _globals->getEntries(offset, 2);
			data[0] = static_cast<uint8>(val);
			data[1] = static_cast<uint8>(val >> 8);
		} else {
			CANT_HAPPEN_MSG("Global pointers must be size 1 or 2");
		}
	} else {
		perr << "Trying to access segment " << ConsoleStream::hex
		     << segment << ConsoleStream::dec << Std::endl;
		return false;
	}
	return true;
}

//static
uint16 UCMachine::ptrToObject(uint32 ptr) {
	//! This function is a bit of a misnomer, since it's more general than this

	uint16 segment = static_cast<uint16>(ptr >> 16);
	uint16 offset = static_cast<uint16>(ptr);
	if (segment >= SEG_STACK_FIRST && segment <= SEG_STACK_LAST) {
		UCProcess *proc = dynamic_cast<UCProcess *>
		                  (Kernel::get_instance()->getProcess(segment));

		// reference to the stack of _pid 'segment'
		if (!proc) {
			// segfault :-)
			perr << "Trying to access stack of non-existent "
			     << "process (pid: " << segment << ")" << Std::endl;
			return 0;
		} else if (proc->_stack.getSize() < (uint32)offset + 2) {
			perr << "Trying to access past end of stack offset " << offset
			     << " (size: " << proc->_stack.getSize()
				 << ") process (pid: " << segment << ")" << Std::endl;
			return 0;
		} else {
			return proc->_stack.access2(offset);
		}
	} else if (segment == SEG_OBJ || segment == SEG_STRING) {
		return offset;
	} else if (segment == SEG_GLOBAL) {
		return get_instance()->_globals->getEntries(offset, 2);
	} else {
		perr << "Trying to access segment " << ConsoleStream::hex
		     << segment << ConsoleStream::dec << Std::endl;
		return 0;
	}
}

void UCMachine::usecodeStats() const {
	g_debugger->debugPrintf("Usecode Machine memory stats:\n");
	g_debugger->debugPrintf("Strings    : %u/65534\n", _stringHeap.size());
#ifdef DUMPHEAP
	Common::HashMap<uint16, Std::string>::const_iterator iter;
	for (iter = _stringHeap.begin(); iter != _stringHeap.end(); ++iter)
		g_debugger->debugPrintf("%d:%s\n", iter->_key << ":" << iter->_value.c_str());
#endif
	g_debugger->debugPrintf("Lists      : %u/65534\n", _listHeap.size());
#ifdef DUMPHEAP
	Common::HashMap<uint16, UCList *>::const_iterator iterl;
	for (iterl = _listHeap.begin(); iterl != _listHeap.end(); ++iterl) {
		if (!iterl->_value) {
			g_debugger->debugPrintf("%d: <null>\n", iterl->_key);
			continue;
		}
		if (iterl->_value->getElementSize() == 2) {
			g_debugger->debugPrintf("%d:", iterl->_key);

			for (unsigned int i = 0; i < iterl->_value->getSize(); ++i) {
				if (i > 0) g_debugger->debugPrintf(",");
				g_debugger->debugPrintf("%d", iterl->_value->getuint16(i));
			}
			g_debugger->debugPrintf("\n");
		} else {
			g_debugger->debugPrintf("%d: %u elements of size %u\n",
				iterl->_key, iterl->_value->getSize(), iterl->_value->getElementSize());
		}
	}
#endif
}

void UCMachine::saveGlobals(Common::WriteStream *ws) const {
	_globals->save(ws);
}

void UCMachine::saveStrings(Common::WriteStream *ws) const {
	_stringIDs->save(ws);
	ws->writeUint32LE(static_cast<uint32>(_stringHeap.size()));

	Common::HashMap<uint16, Std::string>::const_iterator iter;
	for (iter = _stringHeap.begin(); iter != _stringHeap.end(); ++iter) {
		ws->writeUint16LE((*iter)._key);
		ws->writeUint32LE((*iter)._value.size());
		ws->write((*iter)._value.c_str(), (*iter)._value.size());
	}
}

void UCMachine::saveLists(Common::WriteStream *ws) const {
	_listIDs->save(ws);
	ws->writeUint32LE(_listHeap.size());

	Common::HashMap<uint16, UCList *>::const_iterator iter;
	for (iter = _listHeap.begin(); iter != _listHeap.end(); ++iter) {
		ws->writeUint16LE((*iter)._key);
		(*iter)._value->save(ws);
	}
}

bool UCMachine::loadGlobals(Common::ReadStream *rs, uint32 version) {
	return _globals->load(rs, version);
}

bool UCMachine::loadStrings(Common::ReadStream *rs, uint32 version) {
	if (!_stringIDs->load(rs, version)) return false;

	uint32 stringcount = rs->readUint32LE();
	for (unsigned int i = 0; i < stringcount; ++i) {
		uint16 sid = rs->readUint16LE();
		uint32 len = rs->readUint32LE();
		if (len) {
			char *buf = new char[len + 1];
			rs->read(buf, len);
			buf[len] = 0;
			_stringHeap[sid] = buf;
			delete[] buf;
		} else {
			_stringHeap[sid] = "";
		}
	}

	return true;
}

bool UCMachine::loadLists(Common::ReadStream *rs, uint32 version) {
	if (!_listIDs->load(rs, version)) return false;

	uint32 listcount = rs->readUint32LE();

	if (listcount > 65536) {
		warning("Improbable number of UC lists %d in save, corrupt save?", listcount);
		return false;
	}

	for (unsigned int i = 0; i < listcount; ++i) {
		uint16 lid = rs->readUint16LE();
		UCList *l = new UCList(2); // the "2" will be ignored by load()
		bool ret = l->load(rs, version);
		if (!ret) {
			delete l;
			return false;
		}

		_listHeap[lid] = l;
	}

	return true;
}


uint32 UCMachine::I_true(const uint8 * /*args*/, unsigned int /*argsize*/) {
	return 1;
}

uint32 UCMachine::I_false(const uint8 * /*args*/, unsigned int /*argsize*/) {
	return 1;
}

uint32 UCMachine::I_dummyProcess(const uint8 * /*args*/, unsigned int /*argsize*/) {
	return Kernel::get_instance()->addProcess(new DelayProcess(4));
}

uint32 UCMachine::I_getName(const uint8 * /*args*/, unsigned int /*argsize*/) {
	UCMachine *uc = UCMachine::get_instance();
	MainActor *av = getMainActor();
	// Note: assignString takes a copy
	return uc->assignString(av->getName().c_str());
}

uint32 UCMachine::I_numToStr(const uint8 *args, unsigned int /*argsize*/) {
	ARG_SINT16(num);

	char buf[16]; // a 16 bit int should easily fit
	snprintf(buf, 16, "%d", num);

	return UCMachine::get_instance()->assignString(buf);
}

uint32 UCMachine::I_urandom(const uint8 *args, unsigned int /*argsize*/) {
	ARG_UINT16(num);
	if (num <= 1) return 0;

	// return random integer between 0 (incl.) to num (excl.)

	return (getRandom() % num);
}

uint32 UCMachine::I_rndRange(const uint8 *args, unsigned int /*argsize*/) {
	ARG_SINT16(lo);
	ARG_SINT16(hi);

	// return random integer between lo (incl.) to hi (incl.)
	if (hi <= lo)
		return lo;

	return (lo + (getRandom() % (hi - lo + 1)));
}

} // End of namespace Ultima8
} // End of namespace Ultima
