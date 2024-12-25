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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/debug.h"

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/script.h"
#include "saga2/code.h"
#include "saga2/tile.h"
#include "saga2/mission.h"
#include "saga2/hresmgr.h"
#include "saga2/saveload.h"
#include "saga2/actor.h"

namespace Saga2 {

#define IMMED_WORD(w)   ((w = *pc++),(w |= (*pc++)<<8)); \
	debugC(3, kDebugScripts, "IMMED_WORD(%d 0x%04x)", w, w)
#define BRANCH(w)       pc = _codeSeg + (w); \
	debugC(3, kDebugScripts, "BRANCH(%ld 0x%04lx)", long(pc - _codeSeg), long(pc - _codeSeg))

const uint32        sagaID      = MKTAG('S', 'A', 'G', 'A'),
                    dataSegID   = MKTAG('_', '_', 'D', 'A'),
                    exportSegID = MKTAG('_', 'E', 'X', 'P');

const int           initialStackFrameSize = 10;

static bool lookupExport(uint16 entry, uint16 &segNum, uint16 &segOff);
uint8 *segmentAddress(uint16 segment, uint16 offset);

Thread                  *thisThread;

struct ModuleEntry      *moduleList;            // loaded from resource
int16                    moduleBaseResource,
                         moduleCount;

uint16                  dataSegIndex;           // saved index of data seg
byte                   *dataSegment,            // loaded in data
                       *exportSegment;          // export table from SAGA

int32                   dataSegSize;            // bytes in data segment

long                    exportCount;            // number of exported syms

//  An extended script is running -- suspend all background processing.
int16                   extendedThreadLevel;

int16                   lastExport;

extern hResource    *scriptResFile;         // script resources
hResContext         *scriptRes;             // script resource handle

void script_error(const char *msg) {
	thisThread->_flags |= Thread::kTFAborted;
	WriteStatusF(0, msg);
}

static Common::String seg2str(int16 segment) {
	switch (segment) {
	case kBuiltinTypeObject:
		return "GameObject";

	case kBuiltinTypeTAG:
		return "TAG";

	case kBuiltinAbstract:
		return Common::String::format("Abstract%d", segment);

	case kBuiltinTypeMission:
		return "Mission";

	default:
		return Common::String::format("%d", segment);
	}
}

//-----------------------------------------------------------------------
//	Return the address of a builtin object, such as an Actor or a TAG,
//	given a segment number and an index
uint8 *builtinObjectAddress(int16 segment, uint16 index) {
	uint16          segNum, segOff;

	switch (segment) {
	case kBuiltinTypeObject:
		return (uint8 *)(&GameObject::objectAddress(index)->_data);

	case kBuiltinTypeTAG:
		return (uint8 *)(&ActiveItem::activeItemAddress(index)->_data);

	case kBuiltinAbstract:
		assert(index > 0);
		if (lookupExport(index, segNum, segOff) == false)
			error("SAGA: Cannot take address of abtract class");

		return segmentAddress(segNum, segOff);

	case kBuiltinTypeMission:
		return (uint8 *)(&ActiveMission::missionAddress(index)->_data);

	default:
		error("Invalid builtin object segment number: %d\n", segment);
	}
}

//-----------------------------------------------------------------------
//	Given the builtin object type (SAGA segment number), and the address
//	from builtinObjectAddress(), return the address of the virtual
//	function table for the class associated with this object. Also
//	return the address of the C function call table for this builtin
//	class.
uint16 *builtinVTableAddress(int16 btype, uint8 *addr, CallTable **callTab) {
	GameObject      *obj;
	ActiveItem      *aItem;
	ActiveMission   *aMission;
	uint16          script,
	                vtSeg,
	                vtOffset;

	switch (btype) {
	case kBuiltinTypeObject:

		//  Get the address of a game object using the ID
		obj = ((ObjectData *)addr)->obj;
		script = obj->scriptClass();
		*callTab = &actorCFuncs;

		if (script <= 0)
			error("SAGA failure: GameObject %d (%s) has no script.\n", obj->thisID(), obj->proto() ? obj->objName() : "Unknown");

		break;

	case kBuiltinTypeTAG:
		aItem = ((ActiveItemData *)addr)->aItem;
		script = aItem->_data.scriptClassID;
		*callTab = &tagCFuncs;

		if (script <= 0)
			error("SAGA failure: TAG has no script.\n");

		break;

	case kBuiltinTypeMission:
		aMission = ((ActiveMissionData *)addr)->aMission;
		script = aMission->getScript();
		*callTab = &missionCFuncs;

		if (script <= 0)
			error("SAGA failure: Mission Object has no script.\n");

		break;

	case kBuiltinAbstract:
		*callTab = nullptr;

		return (uint16 *)addr;

	default:
		error("SAGA Failure: Attempt to call member function of invalid builtin type.\n");
	}

	//  Look up the vtable in the export table.
	if (script != 0 && lookupExport(script, vtSeg, vtOffset)) {
		return (uint16 *)segmentAddress(vtSeg, vtOffset);
	} else
		return nullptr;
}

uint8 *segmentAddress(uint16 segment, uint16 offset) {
	byte  *segHandle = nullptr;

	//  A segment number of less than zero means that this is
	//  a "builtin" object, in other words the game engine itself
	if ((int16)segment < 0)
		return builtinObjectAddress(segment, offset);

	segHandle = scriptRes->loadIndexResource(segment, "object segment");
	if (segHandle == nullptr)
		return nullptr;

	return segHandle + offset;
}

uint8 *segmentArrayAddress(uint16 segment, uint16 index) {
	byte  *segHandle = nullptr;

	if ((int16)segment < 0)
		return builtinObjectAddress(segment, index);

	segHandle = scriptRes->loadIndexResource(segment, "object array segment");
	if (segHandle == nullptr)
		return nullptr;

	return segHandle + sizeof(uint16) + (uint16)(index * READ_LE_INT16(segHandle));
}

//  Returns the address of a byte given an addressing mode

uint8 *byteAddress(Thread *th, uint8 **pcPtr) {
	uint8           *pc = *pcPtr,
	                 *addr;
	uint16          seg,
	                offset, offset2,
	                index,
	                *arg;

	switch (*pc++) {
	case skAddrData:
		IMMED_WORD(offset);
		debugC(3, kDebugScripts, "byteAddress: data[%d] = %d", offset, dataSegment[offset]);
		*pcPtr = pc;
		return &dataSegment[offset];

	case skAddrNear:
		IMMED_WORD(offset);
		debugC(3, kDebugScripts, "byteAddress: near[%d] = %d", offset, th->_codeSeg[offset]);
		*pcPtr = pc;
		return th->_codeSeg + offset;

	case skAddrFar:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		debugC(3, kDebugScripts, "byteAddress: far[%s:%d] = %d", seg2str(seg).c_str(), offset, *segmentAddress(seg, offset));
		*pcPtr = pc;

		// FIXME: WORKAROUND: Fixes Captain Navis (5299, 17715, 80) in Maldavith not allowing passage to the Tamnath Ruins through sail even if Muybridge is dead.
		if (seg == 130 && offset == 2862) {
			warning("WORKAROUND: byteAddress: far");
			Actor *boss = (Actor *)GameObject::objectAddress(32880);
			if (boss->isDead())
				return segmentAddress(130, 0);
		}

		return segmentAddress(seg, offset);

	case skAddrArray:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		addr = segmentArrayAddress(seg, offset);
		IMMED_WORD(offset2);
		debugC(3, kDebugScripts, "byteAddress: array[%s:%d:%d] = %d", seg2str(seg).c_str(), offset, offset2, addr[offset2]);
		*pcPtr = pc;
		return addr + offset2;

	case skAddrStack:
		IMMED_WORD(offset);
		debugC(3, kDebugScripts, "byteAddress: stack[%d] = %d", offset, *(th->_stackBase + th->_framePtr + (int16)offset));
		*pcPtr = pc;
		return th->_stackBase + th->_framePtr + (int16)offset;

	case skAddrThread:
		IMMED_WORD(offset);
		debugC(3, kDebugScripts, "byteAddress: thread[%d] = %d", offset, *((uint8 *)&th->_threadArgs + offset));
		*pcPtr = pc;
		return (uint8 *)&th->_threadArgs + offset;

	case skAddrThis:
		IMMED_WORD(offset);
		arg = (uint16 *)(th->_stackBase + th->_framePtr + 8);
		*pcPtr = pc;
		if (arg[0] == dataSegIndex) {
			debugC(3, kDebugScripts, "byteAddress: thisD[%d:%d] = %d", arg[1], offset, dataSegment[arg[1] + offset]);
			return &dataSegment[arg[1] + offset];
		}
		debugC(3, kDebugScripts, "byteAddress: thisS[%s:%d:%d] = %d", seg2str(arg[0]).c_str(), arg[1], offset, *(segmentArrayAddress(arg[0], arg[1]) + offset));
		return segmentArrayAddress(arg[0], arg[1]) + offset;

	case skAddrDeref:

		//  First, get the address of the reference.
		*pcPtr = pc;
		addr = byteAddress(th, pcPtr);
		pc = *pcPtr;

		//  Get the offset from the reference variable.
		index = *(uint16 *)addr;

		//  Get the segment to dereference from, and the offset
		//  within the object.
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		debugC(3, kDebugScripts, "byteAddress: deref[%s:%d:%d] = %d", seg2str(seg).c_str(), index, offset, *(segmentAddress(seg, index) + offset));
		*pcPtr = pc;

		//  Compute address of object
		return segmentAddress(seg, index) + offset;
	}

	error("byteAddress: Invalid addressing mode: %d.\n", **pcPtr);
}

//  Returns the address of an object given an addressing mode

uint8 *objectAddress(
    Thread          *th,
    uint8           **pcPtr,
    uint16          &segNum,                // segment of start of object
    uint16          &offs) {                // offset of start of object
	uint8           *pc = *pcPtr,
	                 *addr;
	uint16          seg,
	                offset = 0,
	                index,
	                *arg;

	switch (*pc++) {
	case skAddrData:
		IMMED_WORD(index);
		seg = dataSegIndex;
		addr = &dataSegment[index];
		debugC(3, kDebugScripts, "objectAddress: data[%s:%d] = %d", seg2str(seg).c_str(), index, *addr);
		break;

	case skAddrFar:
		IMMED_WORD(seg);
		IMMED_WORD(index);
		addr = segmentAddress(seg, index);
		debugC(3, kDebugScripts, "objectAddress: far[%s:%d] = %d", seg2str(seg).c_str(), index, *addr);
		break;

	case skAddrArray:
		IMMED_WORD(seg);
		IMMED_WORD(index);
		IMMED_WORD(offset);
		addr = segmentArrayAddress(seg, index) + offset;
		debugC(3, kDebugScripts, "objectAddress: array[%s:%d:%d] = %d", seg2str(seg).c_str(), index, offset, *addr);
		break;

	case skAddrThis:
		IMMED_WORD(offset);
		arg = (uint16 *)(th->_stackBase + th->_framePtr + 8);
		seg = arg[0];
		index = arg[1];
		if (seg == dataSegIndex) {
			debugC(3, kDebugScripts, "objectAddress: thisD[%d:%d] = %d", index, offset, dataSegment[index + offset]);
			return &dataSegment[index + offset];
		}
		addr = segmentArrayAddress(seg, index) + offset;
			debugC(3, kDebugScripts, "objectAddress: thisS[%s:%d:%d] = %d", seg2str(seg).c_str(), index, offset, *addr);
		break;

	case skAddrDeref:

		//  First, get the address of the reference.
		*pcPtr = pc;
		addr = byteAddress(th, pcPtr);
		pc = *pcPtr;

		//  Get the offset from the reference variable.
		index = *(uint16 *)addr;

		//  Get the segment to dereference from, and the offset
		//  within the object.
		IMMED_WORD(seg);
		IMMED_WORD(offset);

		//  Compute address of object
		addr = segmentAddress(seg, index) + offset;
		debugC(3, kDebugScripts, "objectAddress: deref[%s:%d:%d] = %d", seg2str(seg).c_str(), index, offset, *addr);
		break;

	default:
		error("objectAddress: Invalid addressing mode: %d.\n", **pcPtr);
	}

	offs = index;
	segNum = seg;
	*pcPtr = pc;
	return addr;
}

//  Returns the address and access mask of a bit, given addressing mode
uint8 *bitAddress(Thread *th, uint8 **pcPtr, int16 *mask) {
	uint8           *pc = *pcPtr,
	                *addr;
	uint16          seg,
	                offset;

	switch (*pc++) {
	case skAddrData:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		debugC(3, kDebugScripts, "bitAddress: data[%d] = %d", offset, (dataSegment[offset >> 3] & *mask) != 0);
		return &dataSegment[(offset >> 3)];

	case skAddrNear:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		debugC(3, kDebugScripts, "bitAddress: near[%d] = %d", offset, (*(th->_codeSeg + (offset >> 3)) & *mask) != 0);
		return th->_codeSeg + (offset >> 3);

	case skAddrFar:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		debugC(3, kDebugScripts, "bitAddress: far[%s:%d] = %d", seg2str(seg).c_str(), offset, (*segmentAddress(seg, offset >> 3) & *mask) != 0);
		return segmentAddress(seg, (offset >> 3));

	case skAddrArray:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		addr = segmentArrayAddress(seg, offset);
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		debugC(3, kDebugScripts, "bitAddress: array[%s:%d:%d] = %d", seg2str(seg).c_str(), offset, offset, (addr[offset >> 3] & *mask) != 0);
		return addr + (offset >> 3);

	case skAddrStack:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		debugC(3, kDebugScripts, "bitAddress: stack[%d] = %d", offset, (*(th->_stackBase + th->_framePtr + (offset >>3)) & *mask) != 0);
		return th->_stackBase + th->_framePtr + (offset >> 3);

	case skAddrThread:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		debugC(3, kDebugScripts, "bitAddress: thread[%d] = %d", offset, (*((uint8 *)&th->_threadArgs + (offset >> 3)) & *mask) != 0);
		return (uint8 *)&th->_threadArgs + (offset >> 3);

	case skAddrThis:
		error("Addressing relative to 'this' not supported just yet.\n");

	}
	error("bitAddress: Invalid addressing mode: %d.\n", **pcPtr);
}

//  Returns the address of a string

uint8 *Thread::strAddress(int strNum) {
	uint16 seg    = READ_LE_INT16(_codeSeg + 2);
	uint16 offset = READ_LE_INT16(_codeSeg + 4);
	uint8 *strSeg = segmentAddress(seg, offset);

	assert(strNum >= 0);
	assert(_codeSeg);
	assert(strSeg);

	return strSeg + (uint16)READ_LE_INT16(strSeg + 2 * strNum);
}

//-----------------------------------------------------------------------
//	RandomGenerator class - a random number generator class for function
//	objects which each maintain a local seed.
class RandomGenerator {
	uint32  _a;                      //  seed
	static const uint32 _b;          //  arbitrary constant

public:
	RandomGenerator() : _a(1) {
	}
	RandomGenerator(uint16 seed) {
		_a = (uint32)seed << 16;
	}

	void seed(uint16 seed) {
		_a = (uint32)seed << 16;
	}

	uint16 operator()() {
		_a = (_a * _b) + 1;
		return _a >> 16;
	}
};

const uint32 RandomGenerator::_b = 31415821;

//-----------------------------------------------------------------------
//	A restricted random function
int16 RRandom(int16 c, int16 s, int16 id) {
	//  Create a local random number generator with a seed calculated
	//  with a non-deterministic portion generated by the standard
	//  library rand() function and a deterministic potion based upon
	//  the "id" argument
	RandomGenerator rnd(g_vm->_rnd->getRandomNumber(s - 1) + (id * s));
	return rnd() % c;
}

/* ============================================================================ *
                                Main interpreter
 * ============================================================================ */
void print_script_name(uint8 *codePtr, const char *descr = nullptr) {
	char    scriptName[32];
	uint8   *sym = codePtr - 1;
	uint8   length = MIN<uint>(*sym, sizeof scriptName - 1);

	memcpy(scriptName, sym - *sym, length);
	scriptName[length] = '\0';

	if (descr)
		debugC(1, kDebugScripts, "Scripts: %d kOpEnter: [%s].%s ", lastExport, descr, scriptName);
	else
		debugC(1, kDebugScripts, "Scripts: %d kOpEnter: ::%s ", lastExport, scriptName);
}

const char *objectName(int16 segNum, uint16 segOff) {
	//static        nameBuf[64];

	if (segNum >= 0)
		return "SagaObject";

	switch (segNum) {
	case kBuiltinTypeObject:
		return GameObject::objectAddress(segOff)->objName();

	case kBuiltinTypeTAG:
		return "Tag";

	case kBuiltinAbstract:
		return "@";

	case kBuiltinTypeMission:
		return "Mission";
	}
	return "???";
}

#define STACK_PRINT_DEPTH 30

static void print_stack(int16 *_stackBase, int16 *stack) {
	int16 *end = (int16 *)((byte *)_stackBase + kStackSize - initialStackFrameSize);
	int size = end - stack;

	if (size > STACK_PRINT_DEPTH)
		end = stack + STACK_PRINT_DEPTH;

	debugCN(3, kDebugScripts, "stack size: %d: [", size);
	for (int16 *i = stack; i < end; i++)
		debugCN(3, kDebugScripts, "%d ", *i);
	if (size > STACK_PRINT_DEPTH)
		debugCN(3, kDebugScripts, "... ");

	debugC(3, kDebugScripts, "]");
}

#define D_OP(x) debugC(1, kDebugScripts, "[%04ld 0x%04lx]: %s", long(pc - _codeSeg - 1), long(pc - _codeSeg - 1), #x)
#define D_OP1(x) debugC(1, kDebugScripts, "[%04ld 0x%04lx]: %s = %d", long(pc - _codeSeg - 1), long(pc - _codeSeg - 1), #x, *stack)
#define D_OP2(x) debugC(1, kDebugScripts, "[%04ld 0x%04lx]: %s [%p] = %d", long(pc - _codeSeg - 1), long(pc - _codeSeg - 1), #x, (void *)addr, *stack)
#define D_OP3(x) debugC(1, kDebugScripts, "[%04ld 0x%04lx]: %s [%p] %d", long(pc - _codeSeg - 1), long(pc - _codeSeg - 1), #x, (void *)addr, *addr)

bool Thread::interpret() {
	uint8               *pc,
	                    *addr;
	int16               *stack = (int16 *)_stackPtr;
	int16               instruction_count;
	uint8               op;
	int16               w,
	                    n;
	C_Call              *cfunc;

	pc = (_codeSeg) + _programCounter.offset;

	thisThread = this;                          // set current thread address

	for (instruction_count = 0; instruction_count < maxTimeSlice; instruction_count++) {
		print_stack((int16 *)_stackBase, stack);

		switch (op = *pc++) {
		case kOpDup:
			--stack;
			*stack = stack[1];              // duplicate value on stack
			D_OP1(kOpDup);
			break;

		case kOpDrop:                           // drop word on stack
			D_OP(kOpDrop);
			stack++;
			break;

		case kOpZero:                           // constant integer of zero
			D_OP(kOpZero);
			*--stack = 0;                       // push integer on stack
			break;

		case kOpOne:                            // constant integer of one
			D_OP(kOpOne);
			*--stack = 1;                       // push integer on stack
			break;

		case kOpStrlit:                         // string literal (also pushes word)
		case kOpConstint:                       // constant integer
			IMMED_WORD(w);                      // pick up word after opcode
			*--stack = w;                       // push integer on stack

			if (op == kOpStrlit)
				D_OP1(kOpStrlit);
			else
				D_OP1(kOpConstint);
			break;

		case kOpGetflag:                        // get a flag
			addr = bitAddress(this, &pc, &w);    // get address of bit
			*--stack = ((*addr) & w) ? 1 : 0;     // true or false if bit set
			D_OP2(kOpGetflag);
			break;

		case kOpGetint:                         // read from integer field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*--stack = *(uint16 *)addr;         // get integer from address
			D_OP2(kOpGetint);
			break;

		case kOpGetbyte:                        // read from integer field (mode)
			addr = byteAddress(this, &pc);       // get address of integer
			*--stack = *addr;                   // get byte from address
			D_OP2(kOpGetbyte);
			break;

		//  Note that in the current implementation, "put" ops leave
		//  the value that was stored on the stack. We mat also do a
		//  'vput' which consumes the variable.

		case kOpPutflag:                    // put to flag bit (mode)
			addr = bitAddress(this, &pc, &w);  // get address of bit
			if (*stack) *addr |= w;         // set bit if stack non-zero
			else *addr &= ~w;               // else clear it
			D_OP3(kOpPutflag);
			break;

		case kOpPutflagV:                  // put to flag bit (mode)
			addr = bitAddress(this, &pc, &w);  // get address of bit
			if (*stack++) *addr |= w;       // set bit if stack non-zero
			else *addr &= ~w;               // else clear it
			D_OP3(kOpPutflagV);
			break;

		case kOpPutint:                     // put to integer field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr = *stack;       // put integer to address
			D_OP3(kOpPutint);
			break;

		case kOpPutintV:                   // put to integer field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr = *stack++;     // put integer to address
			D_OP3(kOpPutintV);
			break;

		case kOpPutbyte:                    // put to byte field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*addr = *stack;                 // put integer to address
			D_OP3(kOpPutbyte);
			break;

		case kOpPutbyteV:                  // put to byte field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*addr = *stack++;               // put integer to address
			D_OP3(kOpPutbyteV);
			break;

		case kOpEnter:
			D_OP(kOpEnter);
			print_script_name(pc - 1);
			*--stack = _framePtr;            // save old frame ptr on stack
			_framePtr = (uint8 *)stack - _stackBase;  // new frame pointer
			IMMED_WORD(w);                  // pick up word after address
			stack -= w / 2;                 // make room for the locals!
			break;

		//  function calls

		case kOpReturn:                     // return with value
			D_OP(kOpReturn);
			_returnVal = *stack++;
			// fall through

		case kOpReturn_v:                   // return with void
			D_OP(kOpReturn_v);
			stack = (int16 *)(_stackBase + _framePtr);    // pop autos
			_framePtr = *stack++;        // restore frame pointer

			if (stack >= (int16 *)(_stackBase + _stackSize - initialStackFrameSize)) {
				//  Halt the thread here, wait for death
				_programCounter.offset = (pc - (_codeSeg));
				_stackPtr = (uint8 *)stack;
				_flags |= kTFFinished;
				return true;
			} else {
				_programCounter.segment = *stack++;
				_programCounter.offset = *stack++;

				//RUnlockHandle((RHANDLE)_codeSeg);
				_codeSeg = scriptRes->loadIndexResource(_programCounter.segment, "saga code segment");
				pc = (_codeSeg) + _programCounter.offset;

				n = *stack++;               // get argument count from call
				stack += n;                 // pop that many args

				if (op == kOpReturn)        // if not void
					*--stack = _returnVal;// push return value
			}
			break;

		case kOpCallNear:                  // call function in same seg
			D_OP(kOpCallNear);

			n = *pc++;                      // get argument count

			_programCounter.offset = (pc + 2 - _codeSeg);

			*--stack = n;                   // push number of args (16 bits)
			// push the program counter
			*--stack = _programCounter.offset;
			*--stack = _programCounter.segment;

			IMMED_WORD(w);               // pick up segment offset
			_programCounter.offset = w;      // store into pc

			pc = _codeSeg + w;              // calculate PC address

			print_script_name(pc);
			break;

		case kOpCallFar:                   // call function in other seg
			D_OP(kOpCallFar);

			n = *pc++;                      // get argument count

			_programCounter.offset = (pc + 4 - _codeSeg);

			*--stack = n;                   // push number of args (16 bits)
			// push the program counter
			*--stack = _programCounter.offset;
			*--stack = _programCounter.segment;

			IMMED_WORD(w);               // pick up segment number
			_programCounter.segment = w;     // set current segment
			//RUnlockHandle((RHANDLE)_codeSeg);
			_codeSeg = scriptRes->loadIndexResource(w, "saga code segment");
			IMMED_WORD(w);               // pick up segment offset
			_programCounter.offset = w;      // store into pc

			pc = _codeSeg + w;              // calculate PC address
			print_script_name(pc);
			break;

		case kOpCcall:                      // call C function
		case kOpCcallV:                    // call C function
			if (op == kOpCcall)
				D_OP(kOpCcall);
			else
				D_OP(op_call_v);

			n = *pc++;                      // get argument count
			IMMED_WORD(w);                  // get function number
			if (w < 0 || w >= globalCFuncs.numEntries)
				error("Invalid function number");

			cfunc = globalCFuncs.table[w];
			_argCount = n;
			_returnVal = cfunc(stack);        // call the function

			stack += n;                     // pop args of of the stack

			if (op == kOpCcall) {           // push the return value
				*--stack = _returnVal;       // onto the stack
				_flags |= kTFExpectResult;      // script expecting result
			} else _flags &= ~kTFExpectResult;  // script not expecting result

			//  if the thread is asleep, then no more instructions
			if (_flags & kTFAsleep)
				instruction_count = maxTimeSlice;   // break out of loop!

			break;

		case kOpCallMember:                // call member function
		case kOpCallMemberV:              // call member function ()
			if (op == kOpCallMember)
				D_OP(kOpCallMember);
			else
				D_OP(kOpCallMemberV);

			n = *pc++;                      // get argument count
			w = *pc++;                      // index of member function

			{
				uint16  *vtable,
				        *vtableEntry,
				        seg,
				        offset;

				//  REM: We need a more deterministic way to
				//  set up the c function tables.

				CallTable   *callTab = &globalCFuncs;

				//  Get the address of the object
				addr = objectAddress(this, &pc, seg, offset);

				//  Handle the case of a builtin object which computes the
				//  vtable address in a different way.

				if ((int16)seg < 0) {
					vtable = builtinVTableAddress((int16)seg, addr, &callTab);
				} else {
					vtable = (uint16 *)segmentAddress(((int16 *)addr)[0],
					                                  ((int16 *)addr)[1]);
				}

				vtableEntry = vtable + (w * 2);

				if (vtable == nullptr) {
					//  Do nothing...
				} else if (vtableEntry[0] != 0xffff) { // It's a SAGA func
					_programCounter.offset = (pc - _codeSeg);

					//  Push the address of the object
					*--stack = offset;
					*--stack = seg;
					//  Push number of args. including 'this'
					*--stack = n + 2;

					// push the program counter
					*--stack = _programCounter.offset;
					*--stack = _programCounter.segment;

					//  Get the segment of the member function, and
					//  determine it's real address (save segment number
					//  into thread).
					w = vtableEntry[0];
					_programCounter.segment = w;
					//RUnlockHandle((RHANDLE)_codeSeg);
					_codeSeg = scriptRes->loadIndexResource(w, "saga code segment");

					// store pc-offset into pc
					_programCounter.offset = vtableEntry[1];

					// calculate PC address
					pc = (_codeSeg) + _programCounter.offset;
					print_script_name(pc, objectName(seg, offset));

					break;
				} else if (vtableEntry[1] != 0xffff) { // It's a C func
					//  Save the ID of the invoked object
					ObjectID    saveID = _threadArgs.invokedObject;

					//  Get the function number
					w = vtableEntry[1];
					if (w < 0 || w >= callTab->numEntries)
						error("Invalid member function number");

					//  Set up thread-specific vars
					_thisObject = addr;
					_argCount = n;
					_threadArgs.invokedObject = offset;

					//  Get address of function and call it.
					cfunc = callTab->table[w];
					_returnVal = cfunc(stack);        // call the function

					//  Restore object ID from thread args
					_threadArgs.invokedObject = saveID;

					//  Pop args off of the stack
					stack += n;

					//  Push the return value onto the stack if it's
					//  not a 'void' call.
					if (op == kOpCallMember) {
						*--stack = _returnVal;   // onto the stack
						_flags |= kTFExpectResult;  // script expecting result
					} else _flags &= ~kTFExpectResult; // script not expecting result

					//  if the thread is asleep, then break interpret loop
					if (_flags & kTFAsleep) instruction_count = maxTimeSlice;
					break;
				}
				// else it's a NULL function (i.e. pure virtual)
			}

			//  REM: Call the member function

			if (op == kOpCallMember)       // push the return value
				*--stack = 0;               // onto the stack

			break;

		case kOpJmpTrueV:
			D_OP(kOpJmpTrueV);
			IMMED_WORD(w);               // pick up word after address
			if (*stack++ != 0) {
				BRANCH(w);    // if stack is non-zero, jump
			}
			break;

		case kOpJmpFalseV:
			D_OP(kOpJmpFalseV);
			IMMED_WORD(w);               // pick up word after address
			if (*stack++ == 0) {
				BRANCH(w);    // if stack is zero, jump
			}
			break;

		case kOpJmpTrue:
			D_OP(op_true);
			IMMED_WORD(w);               // pick up word after address
			if (*stack != 0) {
				BRANCH(w);      // if stack is non-zero. jump
			}
			break;

		case kOpJmpDalse:
			D_OP(op_false);
			IMMED_WORD(w);               // pick up word after address
			if (*stack == 0) {
				BRANCH(w);      // if stack is zero, jump
			}
			break;

		case kOpJmp:
			D_OP(kOpJmp);
			IMMED_WORD(w);               // pick up word after address
			BRANCH(w);                   // jump relative to module
			break;

		case kOpJmpSwitch:
			D_OP(kOpJmpSwitch);
			IMMED_WORD(n);                  // n = number of cases
			w = *stack++;                   // w = value on stack
			{
				uint16      val,
				            jmp;

				while (n--) {
					IMMED_WORD(val);         // val = case value
					IMMED_WORD(jmp);         // jmp = address to jump to
					debugC(3, kDebugScripts, "Case %d: jmp %d", val, jmp);

					if (w == val) {         // if case values match
						BRANCH(jmp);         // jump to case
						break;
					}
				}
				if (n < 0) {
					IMMED_WORD(jmp);         // def = jump offset for default
					BRANCH(jmp);             // take default jump
				}
			}
			break;

		case kOpJmp_seedrandom:             // seeded random jump
		case kOpJmpRandom:                 // random jump
			if (op == kOpJmp_seedrandom)
				D_OP(kOpJmp_seedrandom);
			else
				D_OP(op_random);

			if (op == kOpJmpRandom) {
				IMMED_WORD(n);              // n = number of cases
				IMMED_WORD(n);              // total probability
				n = (uint16)(g_vm->_rnd->getRandomNumber(n - 1));     // random number between 0 and n-1
			} else {
				int16   seed,
				        r;

				seed = *stack++;            // the seed value
				IMMED_WORD(r);              // n = restriction
				IMMED_WORD(n);              // n = number of cases
				IMMED_WORD(n);              // total probability

				n = RRandom(n, r, seed);
			}

			for (;;) {
				uint16  val,
				        jmp;

				IMMED_WORD(val);             // val = probability of this case
				IMMED_WORD(jmp);             // jmp = address to jump to

				n -= val;                   // subtract prob from overall prob
				if (n < 0) {                // if number within range
					BRANCH(jmp);             // jump to prob
					break;
				}
			}
			break;

		case kOpNegate:
			D_OP(kOpNegate);
			*stack = - *stack;
			break;   // negate TOS
		case kOpNot:
			D_OP(kOpNot);
			*stack = ! *stack;
			break;   // not TOS
		case kOpCompl:
			D_OP(kOpCompl);
			*stack = ~ *stack;
			break;   // complement TOS

		case kOpIncV:
			D_OP(kOpIncV);
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr += 1;           // bump value by one
			break;

		case kOpDecV:
			D_OP(kOpDecV);
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr -= 1;           // bump value by one
			break;

		case kOpPostinc:
			D_OP(kOpPostinc);
			addr = byteAddress(this, &pc);   // get address of integer
			*--stack = *(uint16 *)addr;     // get integer from address
			*(uint16 *)addr += 1;           // bump value by one
			break;

		case kOpPostdec:
			D_OP(kOpPostdec);
			addr = byteAddress(this, &pc);   // get address of integer
			*--stack = *(uint16 *)addr;     // get integer from address
			*(uint16 *)addr -= 1;           // bump value by one
			break;

		//  Binary ops. Since I don't know the order of evaluation of
		//  These C operations, I use a temp variable. Note that
		//  stack is incremented before storing to skip over the
		//  dropped variable.

		case kOpAdd:
			D_OP(kOpAdd);
			w = (stack[1] +  stack[0]);
			*++stack = w;
			break;
		case kOpSub:
			D_OP(kOpSub);
			w = (stack[1] -  stack[0]);
			*++stack = w;
			break;
		case kOpMul:
			D_OP(kOpMul);
			w = (stack[1] *  stack[0]);
			*++stack = w;
			break;
		case kOpDiv:
			D_OP(kOpDiv);
			w = (stack[1] /  stack[0]);
			*++stack = w;
			break;
		case kOpMod:
			D_OP(kOpMod);
			w = (stack[1] %  stack[0]);
			*++stack = w;
			break;
		case kOpEq:
			D_OP(kOpEq);
			w = (stack[1] == stack[0]);
			*++stack = w;
			break;
		case kOpNe:
			D_OP(kOpNe);
			w = (stack[1] != stack[0]);
			*++stack = w;
			break;
		case kOpGt:
			D_OP(kOpGt);
			w = (stack[1] >  stack[0]);
			*++stack = w;
			break;
		case kOpLt:
			D_OP(kOpLt);
			w = (stack[1] <  stack[0]);
			*++stack = w;
			break;
		case kOpGe:
			D_OP(kOpGe);
			w = (stack[1] >= stack[0]);
			*++stack = w;
			break;
		case kOpLe:
			D_OP(kOpLe);
			w = (stack[1] <= stack[0]);
			*++stack = w;
			break;
		case kOpRsh:
			D_OP(kOpRsh);
			w = (stack[1] >> stack[0]);
			*++stack = w;
			break;
		case kOpLsh:
			D_OP(kOpLsh);
			w = (stack[1] << stack[0]);
			*++stack = w;
			break;
		case kOpAnd:
			D_OP(kOpAnd);
			w = (stack[1] &  stack[0]);
			*++stack = w;
			break;
		case kOpOr:
			D_OP(kOpOr);
			w = (stack[1] |  stack[0]);
			*++stack = w;
			break;
		case kOpXor:
			D_OP(kOpXor);
			w = (stack[1] ^  stack[0]);
			*++stack = w;
			break;
		case kOpLand:
			D_OP(kOpLand);
			w = (stack[1] && stack[0]);
			*++stack = w;
			break;
		case kOpLor:
			D_OP(kOpLor);
			w = (stack[1] || stack[0]);
			*++stack = w;
			break;
		case kOpLxor:
			D_OP(kOpLxor);
			w = (stack[1] && !stack[0]) || (!stack[1] && stack[0]);
			*++stack = w;
			break;

		case kOpSpeak:
		case kOpDialogBegin:
		case kOpDialogEnd:
		case kOpReply:
		case kOpAnimate:
			script_error("Feature not implemented.\n");
			break;

		default:
			script_error("fatal error: undefined opcode");
			break;
		}
	}

	_programCounter.offset = (pc - (_codeSeg));
	_stackPtr = (uint8 *)stack;

	return false;
}

/* ============================================================================ *
                            ThreadList class
 * ============================================================================ */

class ThreadList {
	enum {
		kNumThreads = 25
	};

	Thread *_list[kNumThreads];

public:
	//  Constructor
	ThreadList() {
		for (uint i = 0; i < kNumThreads; i++)
			_list[i] = nullptr;
	}

	void read(Common::InSaveFile *in);

	//  Return the number of bytes needed to archive this thread list
	//  in an archive buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out);

	//  Cleanup the active threads
	void cleanup();

	//  Place a thread back into the inactive list
	void deleteThread(Thread *p);

	void newThread(Thread *p, ThreadID id);

	void newThread(Thread *p);

	//  Return the specified thread's ID
	ThreadID getThreadID(Thread *thread) {
		for (uint i = 0; i < kNumThreads; i++) {
			if (_list[i] == thread)
				return i;
		}

		error("Unknown thread address: %p", (void *)thread);
	}

	//  Return a pointer to a thread, given an ID
	Thread *getThreadAddress(ThreadID id) {
		return _list[id];
	}

	//  Return a pointer to the first active thread
	Thread *first();

	Thread *next(Thread *thread);
};

void ThreadList::read(Common::InSaveFile *in) {
	int16 threadCount;

	//  Get the count of threads and increment the buffer pointer
	threadCount = in->readSint16LE();
	debugC(3, kDebugSaveload, "... threadCount = %d", threadCount);

	//  Iterate through the archive data, reconstructing the Threads
	for (int i = 0; i < threadCount; i++) {
		debugC(3, kDebugSaveload, "Saving Thread %d", i);
		ThreadID id;

		//  Retrieve the Thread's id number
		id = in->readSint16LE();
		debugC(4, kDebugSaveload, "...... id = %d", id);

		new Thread(in, id);
	}
}

int32 ThreadList::archiveSize() {
	int32 size = sizeof(int16);

	for (uint i = 0; i < kNumThreads; i++) {
		if (_list[i])
			size += sizeof(ThreadID) + _list[i]->archiveSize();
	}

	return size;
}

void ThreadList::write(Common::MemoryWriteStreamDynamic *out) {
	int16 threadCount = 0;
	Thread *th;

	//  Count the active threads
	for (th = first(); th; th = next(th))
		threadCount++;

	//  Store the thread count in the archive buffer
	out->writeSint16LE(threadCount);
	debugC(3, kDebugSaveload, "... threadCount = %d", threadCount);

	//  Iterate through the threads, archiving each
	for (th = first(); th; th = next(th)) {
		debugC(3, kDebugSaveload, "Loading ThreadID %d", getThreadID(th));
		//  Store the Thread's id number
		out->writeSint16LE(getThreadID(th));

		th->write(out);
	}
}

//-------------------------------------------------------------------
//	Cleanup the active threads

void ThreadList::cleanup() {
	for (uint i = 0; i < kNumThreads; i++) {
		delete _list[i];
		_list[i] = nullptr;
	}
}

//-------------------------------------------------------------------
//	Place a thread back into the inactive list

void ThreadList::deleteThread(Thread *p) {
	for (uint i = 0; i < kNumThreads; i++) {
		if (_list[i] == p) {
			_list[i] = nullptr;
		}
	}
}

void ThreadList::newThread(Thread *p, ThreadID id) {
	if (_list[id])
		error("Thread %d already exists", id);

	_list[id] = p;
}

void ThreadList::newThread(Thread *p) {
	for (uint i = 0; i < kNumThreads; i++) {
		if (!_list[i]) {
			_list[i] = p;
			return;
		}
	}

	error("ThreadList::newThread(): Too many threads");
}

//-------------------------------------------------------------------
//	Return a pointer to the first active thread

Thread *ThreadList::first() {
	for (uint i = 0; i < kNumThreads; i++)
		if (_list[i])
			return _list[i];

	return nullptr;
}

Thread *ThreadList::next(Thread *thread) {
	uint i;
	for (i = 0; i < kNumThreads; i++)
		if (_list[i] == thread)
			break;

	i++;
	if (i >= kNumThreads)
		return nullptr;

	for (; i < kNumThreads; i++)
		if (_list[i])
			return _list[i];

	return nullptr;
}


/* ===================================================================== *
   Global thread list instantiation
 * ===================================================================== */

//	The thread list is instantiated like this in order to keep the
//	constructor from being called until it is explicitly called with
//	the overloaded new operator.

static uint8 threadListBuffer[sizeof(ThreadList)];
static ThreadList &threadList = *((ThreadList *)threadListBuffer);

/* ============================================================================ *
                     ThreadList management functions
 * ============================================================================ */

//-------------------------------------------------------------------
//	Initialize the SAGA thread list

void initSAGAThreads() {
	//  Simply call the Thread List default constructor
}

void saveSAGAThreads(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving SAGA Threads");

	outS->write("SAGA", 4);
	CHUNK_BEGIN;
	threadList.write(out);
	CHUNK_END;
}

void loadSAGAThreads(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading SAGA Threads");

	if (chunkSize == 0) {
		return;
	}

	//  Reconstruct stackList from archived data
	threadList.read(in);
}

//-------------------------------------------------------------------
//	Dispose of the active SAGA threads

void cleanupSAGAThreads() {
	//  Simply call the ThreadList cleanup() function
	threadList.cleanup();
}

//-------------------------------------------------------------------
//	Dispose of an active SAGA thread

void deleteThread(Thread *thread) {
	threadList.deleteThread(thread);
}

void newThread(Thread *p, ThreadID id) {
	threadList.newThread(p, id);
}

void newThread(Thread *thread) {
	threadList.newThread(thread);
}

//-------------------------------------------------------------------
//	Return the ID of the specified SAGA thread

ThreadID getThreadID(Thread *thread) {
	return threadList.getThreadID(thread);
}

//-------------------------------------------------------------------
//	Return a pointer to a SAGA thread, given a thread ID

Thread *getThreadAddress(ThreadID id) {
	return threadList.getThreadAddress(id);
}

/* ============================================================================ *
                           Thread member functions
 * ============================================================================ */

//-----------------------------------------------------------------------
//	Thread constructor

Thread::Thread(uint16 segNum, uint16 segOff, scriptCallFrame &args) {
	_codeSeg = scriptRes->loadIndexResource(segNum, "saga code segment");

	//  initialize the thread
	_stackSize = kStackSize;
	_flags = 0;
	_returnVal = 0;
	_programCounter.segment = segNum;
	_programCounter.offset = segOff;
	_threadArgs = args;
	_stackBase = (byte *)malloc(_stackSize);
	_stackPtr = _stackBase + _stackSize - initialStackFrameSize;
	((uint16 *)_stackPtr)[0] = 0;          // 0 args
	((uint16 *)_stackPtr)[1] = 0;          // dummy return address
	((uint16 *)_stackPtr)[2] = 0;          // dummy return address
	_framePtr = _stackSize;
	_valid = true;

	if ((_codeSeg)[_programCounter.offset] != kOpEnter) {
		//warning("SAGA failure: Invalid script entry point (export=%d) [segment=%d:%d]\n", lastExport, segNum, segOff);
		_valid = false;
	}

	newThread(this);
}

Thread::Thread(Common::SeekableReadStream *stream, ThreadID id) {
	int16   stackOffset;

	_programCounter.segment = stream->readUint16LE();
	_programCounter.offset = stream->readUint16LE();

	_stackSize = stream->readSint16LE();
	_flags = stream->readSint16LE();
	_framePtr = stream->readSint16LE();
	_returnVal = stream->readSint16LE();

	_waitAlarm.read(stream);

	stackOffset = stream->readSint16LE();

	debugC(4, kDebugSaveload, "...... _stackSize = %d", _stackSize);
	debugC(4, kDebugSaveload, "...... flags = %d", _flags);
	debugC(4, kDebugSaveload, "...... _framePtr = %d", _framePtr);
	debugC(4, kDebugSaveload, "...... _returnVal = %d", _returnVal);
	debugC(4, kDebugSaveload, "...... stackOffset = %d", stackOffset);

	_codeSeg = scriptRes->loadIndexResource(_programCounter.segment, "saga code segment");

	_stackBase = (byte *)malloc(_stackSize);
	_stackPtr = _stackBase + _stackSize - stackOffset;

	stream->read(_stackPtr, stackOffset);

	newThread(this, id);
}

//-----------------------------------------------------------------------
//	Thread destructor

Thread::~Thread() {
	//  Clear extended bit if it was set
	clearExtended();

	//  Free the thread's code segment
	//RUnlockHandle((RHANDLE)_codeSeg);

	//  Deallocate the thread stack
	free(_stackBase);

	deleteThread(this);
}

//-----------------------------------------------------------------------
//	Return the number of bytes need to archive this thread in an arhive
//	buffer

int32 Thread::archiveSize() {
	return      sizeof(_programCounter)
	            +   sizeof(_stackSize)
	            +   sizeof(_flags)
	            +   sizeof(_framePtr)
	            +   sizeof(_returnVal)
	            +   sizeof(_waitAlarm)
	            +   sizeof(int16)                //  stack offset
	            + (_stackBase + _stackSize) - _stackPtr;
}

void Thread::write(Common::MemoryWriteStreamDynamic *out) {
	int16   stackOffset;

	out->writeUint16LE(_programCounter.segment);
	out->writeUint16LE(_programCounter.offset);

	out->writeSint16LE(_stackSize);
	out->writeSint16LE(_flags);
	out->writeSint16LE(_framePtr);
	out->writeSint16LE(_returnVal);

	_waitAlarm.write(out);

	warning("STUB: Thread::write: Pointer arithmetic");
	stackOffset = (_stackBase + _stackSize) - _stackPtr;
	out->writeSint16LE(stackOffset);

	out->write(_stackPtr, stackOffset);

	debugC(4, kDebugSaveload, "...... _stackSize = %d", _stackSize);
	debugC(4, kDebugSaveload, "...... flags = %d", _flags);
	debugC(4, kDebugSaveload, "...... _framePtr = %d", _framePtr);
	debugC(4, kDebugSaveload, "...... _returnVal = %d", _returnVal);
	debugC(4, kDebugSaveload, "...... stackOffset = %d", stackOffset);
}

//-----------------------------------------------------------------------
//	Thread dispatcher

void Thread::dispatch() {
	Thread              *th,
	                    *nextThread;

	int                 numThreads = 0,
	                    numExecute = 0,
	                    numWaitDelay = 0,
	                    numWaitFrames = 0,
	                    numWaitSemi = 0,
	                    numWaitOther = 0;

	for (th = threadList.first(); th; th = threadList.next(th)) {
		if (th->_flags & kTFWaiting) {
			switch (th->_waitType) {

			case kWaitDelay:
				numWaitDelay++;
				break;
			case kWaitFrameDelay:
				numWaitFrames++;
				break;
			case kWaitTagSemaphore:
				numWaitSemi++;
				break;
			default:
				numWaitOther++;
				break;
			}
		} else numExecute++;
		numThreads++;
	}

	debugC(9, kDebugScripts, "Threads:%d X:%d D:%d F:%d T:%d O:%d", numThreads, numExecute, numWaitDelay, numWaitFrames, numWaitSemi, numWaitOther);

	for (th = threadList.first(); th; th = nextThread) {
		nextThread = threadList.next(th);

		if (th->_flags & (kTFFinished | kTFAborted)) {
			delete th;
			continue;
		}

		if (th->_flags & kTFWaiting) {
			switch (th->_waitType) {

			case kWaitDelay:

				//  Wake up the thread!

				if (th->_waitAlarm.check())
					th->_flags &= ~kTFWaiting;
				break;

			case kWaitFrameDelay:

				if (th->_waitFrameAlarm.check())
					th->_flags &= ~kTFWaiting;
				break;

			case kWaitTagSemaphore:
				if (th->_waitParam->isExclusive() == false) {
					th->_flags &= ~kTFWaiting;
					th->_waitParam->setExclusive(true);
				}
				break;
			default:
				break;
			}
		}

		do {
			if (th->_flags & (kTFWaiting | kTFFinished | kTFAborted))
				break;

			if (th->interpret())
				goto break_thread_loop;
		} while (th->_flags & kTFSynchronous);
	}
break_thread_loop:
	;
}

//-----------------------------------------------------------------------
//	Run scripts which are on the queue

void dispatchScripts() {
	Thread::dispatch();
}

//-----------------------------------------------------------------------
//	Run a script until finished

scriptResult Thread::run() {
	int             i = 4000;

	while (i--) {
		//  If script stopped, then return
		if (_flags & (kTFWaiting | kTFFinished | kTFAborted)) {
			if (_flags & kTFFinished)   return kScriptResultFinished;
			if (_flags & kTFWaiting)    return kScriptResultAsync;
			return kScriptResultAborted;
			// can't ever fall thru here...
		}

		//  run the script some more...
		interpret();
	}
	error("Thread timed out!\n");
}

//-----------------------------------------------------------------------
//	Convert to extended thread

void Thread::setExtended() {
	if (!(_flags & kTFExtended)) {
		_flags |= kTFExtended;
		extendedThreadLevel++;
	}
}

//-----------------------------------------------------------------------
//	Convert back to regular thread

void Thread::clearExtended() {
	if (_flags & kTFExtended) {
		_flags &= ~kTFExtended;
		extendedThreadLevel--;
	}
}

/* ============================================================================ *
                        Script Management functions
 * ============================================================================ */
void initScripts() {
	//  Open the script resource group
	scriptRes = scriptResFile->newContext(sagaID,  "script resources");
	if (scriptRes == nullptr)
		error("Unable to open script resource file!\n");

	//  Load the data segment
	dataSegment = scriptRes->loadResource(dataSegID, "saga data segment");

	if (dataSegment == nullptr)
		error("Unable to load the SAGA data segment");

	dataSegSize = scriptRes->getSize(dataSegID, "saga data segment");
	debugC(2, kDebugScripts, "dataSegment loaded at %p: size: %d", (void*)dataSegment, dataSegSize);

//	Common::hexdump(dataSegment, dataSegSize);

	exportSegment = scriptRes->loadResource(exportSegID, "saga export segment");
	assert(exportSegment != nullptr);

//	Common::hexdump(exportSegment, scriptRes->getSize(exportSegID, "saga export segment"));

	exportCount = (scriptRes->getSize(exportSegID, "saga export segment") / sizeof(uint32)) + 1;
	debugC(2, kDebugScripts, "exportSegment loaded at %p: size: %d, exportCount: %ld",
	       (void*)exportSegment, scriptRes->getSize(exportSegID, "saga export segment"), exportCount);
}

void cleanupScripts() {
	if (exportSegment)
		free(exportSegment);

	if (dataSegment)
		free(dataSegment);

	if (scriptRes)
		scriptResFile->disposeContext(scriptRes);
	scriptRes = nullptr;
}

//-----------------------------------------------------------------------
//	Load the SAGA data segment from the resource file

void initSAGADataSeg() {
	//  Load the data segment
	scriptRes->seek(dataSegID);
	scriptRes->read(dataSegment, dataSegSize);
}

void saveSAGADataSeg(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Data Segment");

	outS->write("SDTA", 4);
	CHUNK_BEGIN;
	out->write(dataSegment, dataSegSize);
	CHUNK_END;
}

void loadSAGADataSeg(Common::InSaveFile *in) {
	in->read(dataSegment, dataSegSize);
}

//-----------------------------------------------------------------------
//	Look up an entry in the SAGA export table

static bool lookupExport(
    uint16          entry,
    uint16          &segNum,
    uint16          &segOff) {
	uint32          segRef;

	assert(entry > 0);
	assert(entry <= exportCount);

	segRef = READ_LE_INT32(exportSegment + 4 * entry - 2);
	segOff = segRef >> 16,
	segNum = segRef & 0x0000ffff;

	lastExport = entry;
	if (segNum > 1000)
		error("SAGA failure: Bad data in export table entry #%d (see scripts.r)", entry);

	return true;
}

//-----------------------------------------------------------------------
//	Run a script to completion (or until it forks)

scriptResult runScript(uint16 exportEntryNum, scriptCallFrame &args) {
	uint16          segNum,
	                segOff;
	Thread          *th;
	scriptResult    result;
	Thread          *saveThread = thisThread;

	assert(exportEntryNum > 0);
	lookupExport(exportEntryNum, segNum, segOff);

	//  Create a new thread
	th = new Thread(segNum, segOff, args);
	thisThread = th;
	// FIXME: We should probably just use an error(), but this will work for mass debugging
	if (th == nullptr) {
		debugC(4, kDebugScripts, "Couldn't allocate memory for Thread(%d, %d)", segNum, segOff);
		return kScriptResultNoScript;
	} else if (!th->_valid) {
		debugC(4, kDebugScripts, "Scripts: %d is not valid", lastExport);
		return kScriptResultNoScript;
	}
	print_script_name((th->_codeSeg) + th->_programCounter.offset, objectName(segNum, segOff));

	//  Run the thread to completion
	result = th->run();
	args.returnVal = th->_returnVal;

	//  If the thread is not still running, then delete it
	if (result != kScriptResultAsync) delete th;

	// restore "thisThread" ptr.
	thisThread = saveThread;
	return result;
}

//-----------------------------------------------------------------------
//	Run a class member function to completion (or until it forks)

scriptResult runMethod(
    uint16          scriptClassID,          // which script class
    int16           bType,                  // builtin type
    uint16          index,                  // object index
    uint16          methodNum,
    scriptCallFrame &args) {
	uint16          segNum,
	                segOff;
	uint16          *vTable;
	Thread          *th;
	scriptResult    result = kScriptResultNoScript;
	Thread          *saveThread = thisThread;

	//  For abstract classes, the object index is also the class
	//  index.
	if (bType == kBuiltinAbstract)
		index = scriptClassID;

	lookupExport(scriptClassID, segNum, segOff);

	//  Get address of class function table
	vTable = (uint16 *)
	         segmentAddress(segNum, segOff + methodNum * sizeof(uint32));

	segNum = vTable[0];
	segOff = vTable[1];

	if (segNum == 0xffff) {                 // it's a CFUNC or NULL func
		if (segOff == 0xffff) {             // it's a NULL function
			return kScriptResultNoScript;
		} else {                            //  It's a C function
			int16   funcNum = segOff;       // function number
			int16   stack[1];             // dummy stack argument
			C_Call  *cfunc;

			//  Make sure the C function number is OK
			assert(funcNum >= 0);
			assert(funcNum < globalCFuncs.numEntries);
			cfunc = globalCFuncs.table[funcNum];

			//  Build a temporary dummy thread
			th = new Thread(0, 0, args);
			thisThread = th;
			if (th == nullptr)
				return kScriptResultNoScript;
			else if (!th->_valid)
				return kScriptResultNoScript;

			result = (scriptResult)cfunc(stack);   // call the function
			delete th;
		}
	} else {
		//  Create a new thread
		th = new Thread(segNum, segOff, args);
		thisThread = th;
		if (th == nullptr) {
			debugC(3, kDebugScripts, "Couldn't allocate memory for Thread(%d, %d)", segNum, segOff);
			return kScriptResultNoScript;
		} else if (!th->_valid) {
			debugC(3, kDebugScripts, "Scripts: %d is not valid", lastExport);
			return kScriptResultNoScript;
		}
		print_script_name((th->_codeSeg) + th->_programCounter.offset, objectName(bType, index));

		//  Put the object segment and ID onto the dummy stack frame
		((uint16 *)th->_stackPtr)[3] = bType;
		((uint16 *)th->_stackPtr)[4] = index;

		//  Run the thread to completion
		result = th->run();
		args.returnVal = th->_returnVal;
		debugC(3, kDebugScripts, "return: %d", th->_returnVal);

		if (result != kScriptResultAsync) delete th;
	}

	thisThread = saveThread;        // restore "thisThread" ptr.
	return result;
}

//-----------------------------------------------------------------------
//	Run a class member function to completion (or until it forks)

scriptResult runObjectMethod(
    ObjectID        id,
    uint16          methodNum,
    scriptCallFrame &args) {
	GameObject      *obj;

	obj = GameObject::objectAddress(id);

	return runMethod(obj->scriptClass(),
	                 kBuiltinTypeObject,
	                 id,
	                 methodNum,
	                 args);
}

//-----------------------------------------------------------------------
//	Run a class member function to completion (or until it forks)

scriptResult runTagMethod(
    uint16          index,                      // tag number
    uint16          methodNum,
    scriptCallFrame &args) {
	ActiveItemPtr   aItem;

	aItem = ActiveItem::activeItemAddress(index);
	if (!aItem->_data.scriptClassID)
		return kScriptResultNoScript;

	return runMethod(aItem->_data.scriptClassID,
	                 kBuiltinTypeTAG,
	                 index,
	                 methodNum,
	                 args);
}

//-----------------------------------------------------------------------
//	Wake up a thread unconditionally

void wakeUpThread(ThreadID id) {
	if (id != NoThread) {
		Thread  *thread = getThreadAddress(id);

		thread->_flags &= ~Thread::kTFWaiting;
	}
}

void wakeUpThread(ThreadID id, int16 _returnVal) {
	if (id != NoThread) {
		Thread  *thread = getThreadAddress(id);

		if (thread->_flags & Thread::kTFExpectResult) {
			WriteStatusF(8, "Result %d", _returnVal);
			thread->_returnVal = _returnVal;
			*(int16 *)thread->_stackPtr = _returnVal;
		} else WriteStatusF(8, "Thread not expecting result!");

		thread->_flags &= ~(Thread::kTFWaiting | Thread::kTFExpectResult);
	}
}

} // end of namespace Saga2
