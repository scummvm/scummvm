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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "common/debug.h"

#include "saga2/std.h"
#include "saga2/fta.h"
#include "saga2/script.h"
#include "saga2/code.h"
#include "saga2/objects.h"
#include "saga2/tile.h"
#include "saga2/mission.h"

namespace Saga2 {

/* ============================================================================ *
                                   Constants
 * ============================================================================ */

#define IMMED_WORD(w)   ((w = *pc++),(w |= (*pc++)<<8))
#define BRANCH(w)       pc = *codeSeg + (w)

const uint32        sagaID      = MKTAG('S', 'A', 'G', 'A'),
                    dataSegID   = MKTAG('_', '_', 'D', 'A'),
                    exportSegID = MKTAG('_', 'E', 'X', 'P');

const int           initialStackFrameSize = 10;

/* ============================================================================ *
   Protos
 * ============================================================================ */

static bool lookupExport(uint16 entry, uint16 &segNum, uint16 &segOff);

uint8 *segmentAddress(uint16 segment, uint16 offset);

/* ============================================================================ *
                                    Globals
 * ============================================================================ */

Thread                  *thisThread;

//  xRefTable points to a list of segment / offset pairs which
//  indicate where to find every external symbol that lies within
//  a script.

//struct SegmentRef     *xRefTable;

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

/* ============================================================================ *
                                   Externals
 * ============================================================================ */

//extern struct SpeechObject activeSpeech;

extern hResource    *scriptResFile;         // script resources
hResContext         *scriptRes;             // script resource handle

/* ============================================================================ *
                                   Functions
 * ============================================================================ */

void script_error(char *msg) {
	thisThread->flags |= Thread::aborted;
	WriteStatusF(0, msg);
}

//-----------------------------------------------------------------------
//	Return the address of a builtin object, such as an Actor or a TAG,
//	given a segment number and an index

uint8 *builtinObjectAddress(int16 segment, uint16 index) {
	uint16          segNum, segOff;

	switch (segment) {
	case builtinTypeObject:
		return (uint8 *)GameObject::objectAddress(index);

	case builtinTypeTAG:
		return (uint8 *)ActiveItem::activeItemAddress(index);

	case builtinAbstract:
		assert(index > 0);
		if (lookupExport(index, segNum, segOff) == FALSE)
			error("SAGA: Cannot take address of abtract class");

		return segmentAddress(segNum, segOff);

	case builtinTypeMission:
		return (uint8 *)ActiveMission::missionAddress(index);

	default:
		error("Invalid builtin object segment number: %d\n", segment);
	}
#if _WIN32
	return NULL;
#endif
}

//-----------------------------------------------------------------------
//	Given the builtin object type (SAGA segment number), and the adress
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
	case builtinTypeObject:

		//  Get the address of a game object using the ID
		obj = (GameObject *)addr;
		script = obj->scriptClass();
		*callTab = &actorCFuncs;

		if (script <= 0)
			error("SAGA failure: GameObject %d (%s) has no script.\n", obj->thisID(), obj->proto() ? obj->objName() : "Unknown");

		break;

	case builtinTypeTAG:
		aItem = (ActiveItem *)addr;
		script = aItem->scriptClassID;
		*callTab = &tagCFuncs;

		if (script <= 0)
			error("SAGA failure: TAG has no script.\n");

		break;

	case builtinTypeMission:
		aMission = (ActiveMission *)addr;
		script = aMission->getScript();
		*callTab = &missionCFuncs;

		if (script <= 0)
			error("SAGA failure: Mission Object has no script.\n");

		break;

	case builtinAbstract:
		*callTab = NULL;

		return (uint16 *)addr;

	default:
		error("SAGA Failure: Attempt to call member function of invalid builtin type.\n");
	}

//	assert( script > 0 );

	//  Look up the vtable in the export table.
	if (script != 0 && lookupExport(script, vtSeg, vtOffset)) {
//		gError::warn( "vtable was %d %d %d\n", script, vtSeg, vtOffset );
		return (uint16 *)segmentAddress(vtSeg, vtOffset);
	} else return NULL;
}

uint8 *segmentAddress(uint16 segment, uint16 offset) {
	RHANDLE     segHandle;

	//  A segment number of less than zero means that this is
	//  a "builtin" object, in other words the game engine itself
	if ((int16)segment < 0) return builtinObjectAddress(segment, offset);

	segHandle = scriptRes->dataHandle(segment);
	if (segHandle == NULL || *segHandle == NULL) {
		segHandle = scriptRes->loadIndex(segment, "object segment");
		RUnlockHandle(segHandle);
	}
	return ((uint8 *)(*segHandle) + offset);
}

uint8 *segmentArrayAddress(uint16 segment, uint16 index) {
	RHANDLE     segHandle;

	if ((int16)segment < 0) return builtinObjectAddress(segment, index);

	segHandle = scriptRes->dataHandle(segment);
	if (segHandle == NULL || *segHandle == NULL) {
		segHandle = scriptRes->loadIndex(segment, "object array segment");
		RUnlockHandle(segHandle);
	}

	return (uint8 *)(*segHandle) + sizeof(uint16)
	       + (index * *(uint16 *)(*segHandle));
}

//  Returns the address of a byte given an addressing mode

uint8 *byteAddress(Thread *th, uint8 **pcPtr) {
	uint8           *pc = *pcPtr,
	                 *addr;
	uint16          seg,
	                offset,
	                index,
	                *arg;

	switch (*pc++) {
	case addr_data:
		IMMED_WORD(offset);
		*pcPtr = pc;
		return &dataSegment[offset];

	case addr_near:
		IMMED_WORD(offset);
		*pcPtr = pc;
		return *th->codeSeg + offset;

	case addr_far:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		*pcPtr = pc;
		return segmentAddress(seg, offset);

	case addr_array:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		addr = segmentArrayAddress(seg, offset);
		IMMED_WORD(offset);
		*pcPtr = pc;
		return addr + offset;

	case addr_stack:
		IMMED_WORD(offset);
		*pcPtr = pc;
		return th->stackBase + th->framePtr + (int16)offset;

	case addr_thread:
		IMMED_WORD(offset);
		*pcPtr = pc;
		return (uint8 *)&th->threadArgs + offset;

	case addr_this:
		IMMED_WORD(offset);
		arg = (uint16 *)(th->stackBase + th->framePtr + 8);
		*pcPtr = pc;
		if (arg[ 0 ] == dataSegIndex)
			return &dataSegment[arg[ 1 ] + offset];
		return segmentArrayAddress(arg[ 0 ],
		                           arg[ 1 ]) + offset;

	case addr_deref:

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
		*pcPtr = pc;

		//  Compute address of object
		return segmentAddress(seg, index) + offset;
	}
	error("byteAddress: Invalid addressing mode: %d.\n", *pcPtr);
//	return NULL;
#if _WIN32
	return NULL;
#endif
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
	case addr_data:
		IMMED_WORD(index);
		seg = dataSegIndex;
		addr = &dataSegment[index];
		break;

	case addr_far:
		IMMED_WORD(seg);
		IMMED_WORD(index);
		addr = segmentAddress(seg, index);
		break;

	case addr_array:
		IMMED_WORD(seg);
		IMMED_WORD(index);
		IMMED_WORD(offset);
		addr = segmentArrayAddress(seg, index) + offset;
		break;

	case addr_this:
		IMMED_WORD(offset);
		arg = (uint16 *)(th->stackBase + th->framePtr + 8);
		seg = arg[ 0 ];
		index = arg[ 1 ];
		if (seg == dataSegIndex)
			return &dataSegment[index + offset];
		addr = segmentArrayAddress(seg, index) + offset;
		break;

	case addr_deref:

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
		break;

	default:
		error("objectAddress: Invalid addressing mode: %d.\n", *pcPtr);
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
	case addr_data:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		return &dataSegment[(offset >> 3)];

	case addr_near:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		return *th->codeSeg + (offset >> 3);

	case addr_far:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		return segmentAddress(seg, (offset >> 3));

	case addr_array:
		IMMED_WORD(seg);
		IMMED_WORD(offset);
		addr = segmentArrayAddress(seg, offset);
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		return addr + (offset >> 3);

	case addr_stack:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		return th->stackBase + th->framePtr + (offset >> 3);

	case addr_thread:
		IMMED_WORD(offset);
		*pcPtr = pc;
		*mask = (1 << (offset & 7));
		return (uint8 *)&th->threadArgs + (offset >> 3);

	case addr_this:
		error("Addressing relative to 'this' not supported just yet.\n");

//	addr_indirect,                           // use SEG:offset on stack
//	addr_indirect_index,                 // use SEG:index:offset on stack

	}
	error("bitAddress: Invalid addressing mode: %d.\n", *pcPtr);
//	fatal( "Invalid addressing mode.\n" );
//	return NULL;
#if _WIN32
	return NULL;
#endif
}

//  Returns the address of a string

uint8 *Thread::strAddress(int strNum) {
	uint16          *codeBase = (uint16 *)*codeSeg;
	uint8           *strSeg = segmentAddress(codeBase[ 1 ], codeBase[ 2 ]);

	assert(strNum >= 0);
	assert(codeBase);
	assert(strSeg);

	return strSeg + ((uint16 *)strSeg)[ strNum ];
}

//-----------------------------------------------------------------------
//	RandomGenerator class - a random number generator class for function
//	objects which each maintain a local seed.

class RandomGenerator {
	uint32  a;                      //  seed
	static const uint32 b;          //  arbitrary constant

public:
	RandomGenerator(void) {
		a = 1;
	}
	RandomGenerator(uint16 seed) {
		a = (uint32)seed << 16;
	}

	void seed(uint16 seed) {
		a = (uint32)seed << 16;
	}

	uint16 operator()(void) {
		a = (a * b) + 1;
		return a >> 16;
	}
};

const uint32 RandomGenerator::b = 31415821;

//-----------------------------------------------------------------------
//	A restricted random function

int16 RRandom(int16 c, int16 s, int16 id) {
	//  Create a local random number generator with a seed calculated
	//  with a non-deterministic portion generated by the standard
	//  library rand() function and a deterministic potion based upon
	//  the "id" argument
	RandomGenerator random((rand() % s) + (id * s));
	return random() % c;
}

/* ============================================================================ *
                                Main interpreter
 * ============================================================================ */

void print_script_name(uint8 *codePtr, char *descr = NULL) {
	char    scriptName[ 32 ];
	uint8   *sym = codePtr - 1;
	uint8   length = MIN<uint>(*sym, sizeof scriptName - 1);

	memcpy(scriptName, sym - *sym, length);
	scriptName[ length ] = '\0';

	if (descr)
		debugC(1, kDebugScripts, "Scripts: op_enter: [%s].%s ", descr, scriptName);
	else
		debugC(1, kDebugScripts, "Scripts: op_enter: ::%s ", scriptName);
}

char *objectName(int16 segNum, uint16 segOff) {
	//static        nameBuf[ 64 ];

	uint8       *objAddr;
	if (segNum >= 0)
		return "SagaObject";

	objAddr = builtinObjectAddress(segNum, segOff);

	switch (segNum) {
	case builtinTypeObject:
		return ((GameObject *)objAddr)->objName();

	case builtinTypeTAG:
		return "Tag";

	case builtinAbstract:
		return "@";

	case builtinTypeMission:
		return "Mission";
	}
	return "???";
}

bool Thread::interpret(void) {
	uint8               *pc,
	                    *addr;
	int16               *stack = (int16 *)stackPtr;
	int16               instruction_count;
	uint8               op;
	int16               w,
	                    n;
	C_Call              *cfunc;

	pc = (*codeSeg) + programCounter.offset;

	thisThread = this;                          // set current thread address

	for (instruction_count = 0;
	        instruction_count < maxTimeSlice;
	        instruction_count++) {
		switch (op = *pc++) {

//		case op_nextblock:
//			n = (pc-1-(*codeSeg)) / 1024;      // calculate address of this block
//			BRANCH((n + 1) * 1024);                // jump to next block
//			break;

		case op_dup:
			*--stack = stack[ 0 ];              // duplicate value on stack
			break;

		case op_drop:                           // drop word on stack
			stack++;
			break;

		case op_zero:                           // constant integer of zero
			*--stack = 0;                       // push integer on stack
			break;

		case op_one:                            // constant integer of one
			*--stack = 1;                       // push integer on stack
			break;

		case op_strlit:                         // string literal (also pushes word)
		case op_constint:                       // constant integer
			IMMED_WORD(w);                      // pick up word after opcode
			*--stack = w;                       // push integer on stack
			break;

		case op_getflag:                        // get a flag
			addr = bitAddress(this, &pc, &w);    // get address of bit
			*--stack = (*addr) & w ? 1 : 0;     // true or false if bit set
			break;

		case op_getint:                         // read from integer field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*--stack = *(uint16 *)addr;         // get integer from address
			break;

		case op_getbyte:                        // read from integer field (mode)
			addr = byteAddress(this, &pc);       // get address of integer
			*--stack = *addr;                   // get byte from address
			break;

#if 0
			op_getstr,                                  // read from string field (mode)
#endif

		//  Note that in the current implementation, "put" ops leave
		//  the value that was stored on the stack. We mat also do a
		//  'vput' which consumes the variable.

		case op_putflag:                    // put to flag bit (mode)
			addr = bitAddress(this, &pc, &w);  // get address of bit
			if (*stack) *addr |= w;         // set bit if stack non-zero
			else *addr &= ~w;               // else clear it
			break;

		case op_putflag_v:                  // put to flag bit (mode)
			addr = bitAddress(this, &pc, &w);  // get address of bit
			if (*stack++) *addr |= w;       // set bit if stack non-zero
			else *addr &= ~w;               // else clear it
			break;

		case op_putint:                     // put to integer field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr = *stack;       // put integer to address
			break;

		case op_putint_v:                   // put to integer field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr = *stack++;     // put integer to address
			break;

		case op_putbyte:                    // put to byte field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*addr = *stack;                 // put integer to address
			break;

		case op_putbyte_v:                  // put to byte field (mode)
			addr = byteAddress(this, &pc);   // get address of integer
			*addr = *stack++;               // put integer to address
			break;

#if 0
			op_putstr,                              // put to string field (mode)
#endif

		case op_enter:

//#if DEBUG
//			print_script_name( pc - 1 );
//#endif
			*--stack = framePtr;            // save old frame ptr on stack
			framePtr = (uint8 *)stack - stackBase;  // new frame pointer
			IMMED_WORD(w);                  // pick up word after address
			stack -= w / 2;                 // make room for the locals!
			break;

		//  function calls

		case op_return:                     // return with value
			returnVal = *stack++;
		case op_return_v:                   // return with void

		debugC(1, kDebugScripts, "Scripts: op_return");

// REM: When we implement dynamic strings we'll want to clean up first.

			stack = (int16 *)(stackBase + framePtr);    // pop autos
			framePtr = *stack++;        // restore frame pointer

			if (stack >= (int16 *)(stackBase + stackSize - initialStackFrameSize)) {
				//  Halt the thread here, wait for death
				programCounter.offset = (pc - (*codeSeg));
				stackPtr = (uint8 *)stack;
				flags |= finished;
				return TRUE;
			} else {
				programCounter.segment = *stack++;
				programCounter.offset = *stack++;

				RUnlockHandle((RHANDLE)codeSeg);
				codeSeg = (UByteHandle)
				          scriptRes->loadIndex(programCounter.segment, "saga code segment");
				pc = (*codeSeg) + programCounter.offset;

				n = *stack++;               // get argument count from call
				stack += n;                 // pop that many args

				if (op == op_return)        // if not void
					*--stack = returnVal;// push return value
			}
			break;

		case op_call_near:                  // call function in same seg

			n = *pc++;                      // get argument count

			programCounter.offset = (pc + 2 - *codeSeg);

			*--stack = n;                   // push number of args (16 bits)
			// push the program counter
			*--stack = programCounter.offset;
			*--stack = programCounter.segment;

			IMMED_WORD(w);               // pick up segment offset
			programCounter.offset = w;      // store into pc

			pc = *codeSeg + w;              // calculate PC address

			print_script_name(pc);
			break;

		case op_call_far:                   // call function in other seg

			n = *pc++;                      // get argument count

			programCounter.offset = (pc + 4 - *codeSeg);

			*--stack = n;                   // push number of args (16 bits)
			// push the program counter
			*--stack = programCounter.offset;
			*--stack = programCounter.segment;

			IMMED_WORD(w);               // pick up segment number
			programCounter.segment = w;     // set current segment
			RUnlockHandle((RHANDLE)codeSeg);
			codeSeg = (UByteHandle)scriptRes->loadIndex(w, "saga code segment");
			IMMED_WORD(w);               // pick up segment offset
			programCounter.offset = w;      // store into pc

			pc = *codeSeg + w;              // calculate PC address
			print_script_name(pc);
			break;

		case op_ccall:                      // call C function
		case op_ccall_v:                    // call C function

			n = *pc++;                      // get argument count
			IMMED_WORD(w);                  // get function number
			if (w < 0 || w >= globalCFuncs.numEntries)
				error("Invalid function number");

			cfunc = globalCFuncs.table[ w ];
			argCount = n;
			returnVal = cfunc(stack);        // call the function

			stack += n;                     // pop args of of the stack

			if (op == op_ccall) {           // push the return value
				*--stack = returnVal;       // onto the stack
				flags |= expectResult;      // script expecting result
			} else flags &= ~expectResult;  // script not expecting result

			//  if the thread is asleep, then no more instructions

			if (flags & asleep)
				instruction_count = maxTimeSlice;   // break out of loop!

			break;

		case op_call_member:                // call member function
		case op_call_member_v:              // call member function (void)
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
					vtable = (uint16 *)segmentAddress(((int16 *)addr)[ 0 ],
					                                  ((int16 *)addr)[ 1 ]);
				}

				vtableEntry = vtable + (w * 2);

				if (vtable == NULL) {
					//  Do nothing...
				} else if (vtableEntry[ 0 ] != 0xffff) { // It's a SAGA func
					programCounter.offset = (pc - *codeSeg);

					//  Push the address of the object
					*--stack = offset;
					*--stack = seg;
					//  Push number of args. including 'this'
					*--stack = n + 2;

					// push the program counter
					*--stack = programCounter.offset;
					*--stack = programCounter.segment;

					//  Get the segment of the member function, and
					//  determine it's real address (save segment number
					//  into thread).
					w = vtableEntry[ 0 ];
					programCounter.segment = w;
					RUnlockHandle((RHANDLE)codeSeg);
					codeSeg = (UByteHandle)scriptRes->loadIndex(w, "saga code segment");

					// store pc-offset into pc
					programCounter.offset = vtableEntry[ 1 ];

					// calculate PC address
					pc = (*codeSeg) + programCounter.offset;
					print_script_name(pc, objectName(seg, offset));
					break;
				} else if (vtableEntry[ 1 ] != 0xffff) { // It's a C func
					//  Save the ID of the invoked object
					ObjectID    saveID = threadArgs.invokedObject;

					//  Get the function number
					w = vtableEntry[ 1 ];
					if (w < 0 || w >= callTab->numEntries)
						error("Invalid member function number");

					//  Set up thread-specific vars
					thisObject = addr;
					argCount = n;
					threadArgs.invokedObject = offset;

					//  Get address of function and call it.
					cfunc = callTab->table[ w ];
					returnVal = cfunc(stack);        // call the function

					//  Restore object ID from thread args
					threadArgs.invokedObject = saveID;

					//  Pop args off of the stack
					stack += n;

					//  Push the return value onto the stack if it's
					//  not a 'void' call.
					if (op == op_call_member) {
						*--stack = returnVal;   // onto the stack
						flags |= expectResult;  // script expecting result
					} else flags &= ~expectResult; // script not expecting result

					//  if the thread is asleep, then break interpret loop
					if (flags & asleep) instruction_count = maxTimeSlice;
					break;
				}
				// else it's a NULL function (i.e. pure virtual)
			}

			//  REM: Call the member function

			if (op == op_call_member)       // push the return value
				*--stack = 0;               // onto the stack

			break;

		case op_jmp_true_v:

			IMMED_WORD(w);               // pick up word after address
			if (*stack++ != 0) BRANCH(w);    // if stack is non-zero, jump
			break;

		case op_jmp_false_v:

			IMMED_WORD(w);               // pick up word after address
			if (*stack++ == 0) BRANCH(w);    // if stack is zero, jump
			break;

		case op_jmp_true:

			IMMED_WORD(w);               // pick up word after address
			if (*stack != 0) BRANCH(w);      // if stack is non-zero. jump
			break;

		case op_jmp_false:

			IMMED_WORD(w);               // pick up word after address
			if (*stack == 0) BRANCH(w);      // if stack is zero, jump
			break;

		case op_jmp:

			IMMED_WORD(w);               // pick up word after address
			BRANCH(w);                   // jump relative to module
			break;

		case op_jmp_switch:
			IMMED_WORD(n);                  // n = number of cases
			w = *stack++;                   // w = value on stack
			{
				uint16      val,
				            jmp;

				while (n--) {
					IMMED_WORD(val);         // val = case value
					IMMED_WORD(jmp);         // jmp = address to jump to

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

//		case op_jmp_sswitch:                // string-based case/switch

		case op_jmp_seedrandom:             // seeded random jump
		case op_jmp_random:                 // random jump

			if (op == op_jmp_random) {
				IMMED_WORD(n);              // n = number of cases
				IMMED_WORD(n);              // total probability
				n = (uint16)rand() % n;     // random number between 0 and n-1
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

		case op_negate:
			*stack = - *stack;
			break;   // negate TOS
		case op_not:
			*stack = ! *stack;
			break;   // not TOS
		case op_compl:
			*stack = ~ *stack;
			break;   // complement TOS

		case op_inc_v:
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr += 1;           // bump value by one
			break;

		case op_dec_v:
			addr = byteAddress(this, &pc);   // get address of integer
			*(uint16 *)addr -= 1;           // bump value by one
			break;

		case op_postinc:
			addr = byteAddress(this, &pc);   // get address of integer
			*--stack = *(uint16 *)addr;     // get integer from address
			*(uint16 *)addr += 1;           // bump value by one
			break;

		case op_postdec:
			addr = byteAddress(this, &pc);   // get address of integer
			*--stack = *(uint16 *)addr;     // get integer from address
			*(uint16 *)addr -= 1;           // bump value by one
			break;

		//  Binary ops. Since I don't know the order of evaluation of
		//  These C operations, I use a temp variable. Note that
		//  stack is incremented before storing to skip over the
		//  dropped variable.

		case op_add:
			w = (stack[ 1 ] +  stack [ 0 ]);
			*++stack = w;
			break;
		case op_sub:
			w = (stack[ 1 ] -  stack [ 0 ]);
			*++stack = w;
			break;
		case op_mul:
			w = (stack[ 1 ] *  stack [ 0 ]);
			*++stack = w;
			break;
		case op_div:
			w = (stack[ 1 ] /  stack [ 0 ]);
			*++stack = w;
			break;
		case op_mod:
			w = (stack[ 1 ] %  stack [ 0 ]);
			*++stack = w;
			break;
		case op_eq:
			w = (stack[ 1 ] == stack [ 0 ]);
			*++stack = w;
			break;
		case op_ne:
			w = (stack[ 1 ] != stack [ 0 ]);
			*++stack = w;
			break;
		case op_gt:
			w = (stack[ 1 ] >  stack [ 0 ]);
			*++stack = w;
			break;
		case op_lt:
			w = (stack[ 1 ] <  stack [ 0 ]);
			*++stack = w;
			break;
		case op_ge:
			w = (stack[ 1 ] >= stack [ 0 ]);
			*++stack = w;
			break;
		case op_le:
			w = (stack[ 1 ] <= stack [ 0 ]);
			*++stack = w;
			break;
		case op_rsh:
			w = (stack[ 1 ] >> stack [ 0 ]);
			*++stack = w;
			break;
		case op_lsh:
			w = (stack[ 1 ] << stack [ 0 ]);
			*++stack = w;
			break;
		case op_and:
			w = (stack[ 1 ] &  stack [ 0 ]);
			*++stack = w;
			break;
		case op_or:
			w = (stack[ 1 ] |  stack [ 0 ]);
			*++stack = w;
			break;
		case op_xor:
			w = (stack[ 1 ] ^  stack [ 0 ]);
			*++stack = w;
			break;
		case op_land:
			w = (stack[ 1 ] && stack [ 0 ]);
			*++stack = w;
			break;
		case op_lor:
			w = (stack[ 1 ] || stack [ 0 ]);
			*++stack = w;
			break;
		case op_lxor:
			w = (stack[ 1 ] && !stack [ 0 ]) || (!stack[ 1 ] && stack[ 0 ]);
			*++stack = w;
			break;

#if 0
			//  String functions. First figure out how strings are going to be
			//  stored!!

			op_str_eq,
			op_str_ne,
			op_str_gt,
			op_str_lt,
			op_str_ge,
			op_str_le,
			op_strcat,                  // string concatenation
			op_strformat,               // string formatting
#endif
		case op_speak:
		case op_dialog_begin:
		case op_dialog_end:
		case op_reply:
		case op_animate:
			script_error("Feature not implemented.\n");

		default:
			script_error("fatal error: undefined opcode");
			break;
		}
	}

	programCounter.offset = (pc - (*codeSeg));
	stackPtr = (uint8 *)stack;

	return FALSE;
}

/* ============================================================================ *
                            ThreadList class
 * ============================================================================ */

class ThreadList {

	struct ThreadPlaceHolder : public DNode {
		uint8       buf[ sizeof(Thread) ];

		Thread *getThread(void) {
			return (Thread *)&buf;
		}
	};

	DList               list,           //  List of active Threads
	                    free;           //  List of available Threads

	ThreadPlaceHolder   array[ 32 ];    //  Memory buffer for thread
	//  instantiation

public:
	//  Constructor
	ThreadList(void);

	//  Reconstruct from archive buffer
	void *restore(void *buf);

	//  Return the number of bytes needed to archive this thread list
	//  in an archive buffer
	int32 archiveSize(void);

	//  Create an archive of this thread list in an archive buffer
	void *archive(void *buf);

	//  Cleanup the active threads
	void cleanup(void);

	//  Place a new thread into the active list and return its pointer
	void *newThread(void);
	void *newThread(ThreadID id);

	//  Place a thread back into the inactive list
	void deleteThread(void *p);

	//  Return the specified thread's ID
	ThreadID getThreadID(Thread *thread) {
		ThreadPlaceHolder   *tp;

		tp = ((ThreadPlaceHolder *)(
		          (uint8 *)thread
		          -   offsetof(ThreadPlaceHolder, buf)));
		return tp - array;
	}

	//  Return a pointer to a thread, given an ID
	Thread *getThreadAddress(ThreadID id) {
		assert(id >= 0 && id < elementsof(array));
		return array[ id ].getThread();
	}

	//  Return a pointer to the first active thread
	Thread *first(void);

	//  Return a pointer to the next active thread
	Thread *next(Thread *thread);
};

//-------------------------------------------------------------------
//	Constructor

ThreadList::ThreadList(void) {
	int i;

	for (i = 0; i < elementsof(array); i++)
		free.addTail(array[ i ]);
}

//-------------------------------------------------------------------
//	Reconstruct from archive buffer

void *ThreadList::restore(void *buf) {
	int16   i,
	        threadCount;

	//  Get the count of threads and increment the buffer pointer
	threadCount = *((int16 *)buf);
	buf = (int16 *)buf + 1;

	//  Iterate through the archive data, reconstructing the Threads
	for (i = 0; i < threadCount; i++) {
		ThreadID        id;

		//  Retreive the Thread's id number
		id = *((ThreadID *)buf);
		buf = (ThreadID *)buf + 1;

		new (id) Thread(&buf);
	}

	return buf;
}

//-------------------------------------------------------------------
//	Return the number of bytes needed to archive this thead list
//	in an archive buffer

int32 ThreadList::archiveSize(void) {
	int32               size = sizeof(int16);
	ThreadPlaceHolder   *tp;

	for (tp = (ThreadPlaceHolder *)list.first();
	        tp != NULL;
	        tp = (ThreadPlaceHolder *)tp->next())
		size += sizeof(ThreadID) + tp->getThread()->archiveSize();

	return size;
}

//-------------------------------------------------------------------
//	Create an archive of this thread list in an archive buffer

void *ThreadList::archive(void *buf) {
	int16               threadCount = 0;
	ThreadPlaceHolder   *tp;

	//  Count the active threads
	for (tp = (ThreadPlaceHolder *)list.first();
	        tp != NULL;
	        tp = (ThreadPlaceHolder *)tp->next())
		threadCount++;

	//  Store the thread count in the archive buffer
	*((int16 *)buf) = threadCount;
	buf = (int16 *)buf + 1;

	//  Iterate through the threads, archiving each
	for (tp = (ThreadPlaceHolder *)list.first();
	        tp != NULL;
	        tp = (ThreadPlaceHolder *)tp->next()) {
		Thread  *thread = tp->getThread();

		//  Store the Thread's id number
		*((ThreadID *)buf) = tp - array;
		buf = (ThreadID *)buf + 1;

		buf = thread->archive(buf);
	}

	return buf;
}

//-------------------------------------------------------------------
//	Cleanup the active threads

void ThreadList::cleanup(void) {
	ThreadPlaceHolder       *tp;
	ThreadPlaceHolder       *nextTP;

	for (tp = (ThreadPlaceHolder *)list.first();
	        tp != NULL;
	        tp = nextTP) {
		//  Save the address of the next in the list
		nextTP = (ThreadPlaceHolder *)tp->next();

		delete tp->getThread();
	}
}

//-------------------------------------------------------------------
//	Place a new thread into the active list and return its pointer

void *ThreadList::newThread(void) {
	ThreadPlaceHolder   *tp;

	//  Grab a thread place holder from the inactive list
	tp = (ThreadPlaceHolder *)free.remHead();

	if (tp != NULL) {
		//  Place the place holder into the active list
		list.addTail(*tp);

		return tp->buf;
	}

	return NULL;
}

//-------------------------------------------------------------------
//	Place a new thread into the active list and return its pointer

void *ThreadList::newThread(ThreadID id) {
	assert(id >= 0 && id < elementsof(array));

	ThreadPlaceHolder   *tp;

	//  Grab the thread place holder from the inactive list
	tp = (ThreadPlaceHolder *)&array[ id ];
	tp->remove();

	//  Place the place holder into the active list
	list.addTail(*tp);

	return tp->buf;
}

//-------------------------------------------------------------------
//	Place a thread back into the inactive list

void ThreadList::deleteThread(void *p) {
	ThreadPlaceHolder       *tp;

	//  Convert the pointer to the Thread to a pointer to the
	//  ThreadPlaceHolder
	tp = (ThreadPlaceHolder *)(
	         (uint8 *)p
	         -   offsetof(ThreadPlaceHolder, buf));

	//  Remove the thread place holder from the active list
	tp->remove();

	//  Place it into the inactive list
	free.addTail(*tp);
}

//-------------------------------------------------------------------
//	Return a pointer to the first active thread

Thread *ThreadList::first(void) {
	ThreadPlaceHolder   *tp = (ThreadPlaceHolder *)list.first();

	return tp != NULL ? tp->getThread() : NULL;
}

//-------------------------------------------------------------------
//	Return a pointer to the next active thread

Thread *ThreadList::next(Thread *thread) {
	ThreadPlaceHolder   *tp;

	//  Convert the pointer to the Thread to a pointer to the
	//  ThreadPlaceHolder
	tp = (ThreadPlaceHolder *)(
	         (uint8 *)thread
	         -   offsetof(ThreadPlaceHolder, buf));

	tp = (ThreadPlaceHolder *)tp->next();

	return tp != NULL ? tp->getThread() : NULL;
}

/* ===================================================================== *
   Global thread list instantiation
 * ===================================================================== */

//	The thread list is instantiated like this in order to keep the
//	constructor from being called until it is explicitly called with
//	the overloaded new operator.

static uint8 threadListBuffer[ sizeof(ThreadList) ];
static ThreadList &threadList = *((ThreadList *)threadListBuffer);

/* ============================================================================ *
                     ThreadList management functions
 * ============================================================================ */

//-------------------------------------------------------------------
//	Initialize the SAGA thread list

void initSAGAThreads(void) {
	//  Simply call the Thread List default constructor
	new (&threadList) ThreadList;
}

//-------------------------------------------------------------------
//	Save the active SAGA threads to a save file

void saveSAGAThreads(SaveFileConstructor &saveGame) {
	int32   archiveBufSize;
	void    *archiveBuffer;

	archiveBufSize = threadList.archiveSize();

	archiveBuffer = RNewPtr(archiveBufSize, NULL, "archive buffer");
	if (archiveBuffer == NULL)
		error("Unable to allocate SAGA thread archive buffer");

	threadList.archive(archiveBuffer);

	saveGame.writeChunk(
	    MakeID('S', 'A', 'G', 'A'),
	    archiveBuffer,
	    archiveBufSize);

	RDisposePtr(archiveBuffer);
}

//-------------------------------------------------------------------
//	Load the active SAGA threads from a save file

void loadSAGAThreads(SaveFileReader &saveGame) {
	//  If there is no saved data, simply call the default constructor
	if (saveGame.getChunkSize() == 0) {
		new (&threadList) ThreadList;
		return;
	}

	void    *archiveBuffer;
	void    *bufferPtr;

	archiveBuffer = RNewPtr(saveGame.getChunkSize(), NULL, "archive buffer");
	if (archiveBuffer == NULL)
		error("Unable to allocate SAGA thread archive buffer");

	//  Read the archived thread data
	saveGame.read(archiveBuffer, saveGame.getChunkSize());

	bufferPtr = archiveBuffer;

	//  Reconstruct stackList from archived data
	new (&threadList) ThreadList;
	bufferPtr = threadList.restore(bufferPtr);

	assert((char *)bufferPtr == (char *)archiveBuffer
	       +   saveGame.getChunkSize());

	RDisposePtr(archiveBuffer);
}

//-------------------------------------------------------------------
//	Dispose of the active SAGA threads

void cleanupSAGAThreads(void) {
	//  Simply call the ThreadList cleanup() function
	threadList.cleanup();
}

//-------------------------------------------------------------------
//	Get a new SAGA thread from the global thread list

void *newThread(void) {
	return threadList.newThread();
}

//-------------------------------------------------------------------
//	Get a specific SAGA thread from the global thread list

void *newThread(ThreadID id) {
	return threadList.newThread(id);
}

//-------------------------------------------------------------------
//	Dispose of an active SAGA thread

void deleteThread(void *thread) {
	threadList.deleteThread(thread);
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
	codeSeg = (UByteHandle)scriptRes->loadIndex(segNum, "saga code segment");

	//  initialize the thread
	stackSize = kStackSize;
	flags = 0;
	returnVal = 0;
	programCounter.segment = segNum;
	programCounter.offset = segOff;
	threadArgs = args;
	stackBase = (UBytePtr)RNewPtr(stackSize, NULL, "saga stack");
	stackPtr = stackBase + stackSize - initialStackFrameSize;
	((uint16 *)stackPtr)[ 0 ] = 0;          // 0 args
	((uint16 *)stackPtr)[ 1 ] = 0;          // dummy return address
	((uint16 *)stackPtr)[ 2 ] = 0;          // dummy return address
	framePtr = stackSize;

	if ((*codeSeg)[ programCounter.offset ] != op_enter)
		error("SAGA failure: Invalid script entry point (export=%d) [segment=%d:%d]\n", lastExport, segNum, segOff);
//	assert ((*codeSeg)[ programCounter.offset ] == op_enter);
}

//-----------------------------------------------------------------------
//	Constructor -- reconstruct from archive buffer

Thread::Thread(void **buf) {
	void    *bufferPtr = *buf;

	int16   stackOffset;

	programCounter = *((SegmentRef *)bufferPtr);
	bufferPtr = (SegmentRef *)bufferPtr + 1;

	stackSize   = *((int16 *)bufferPtr);
	flags       = *((int16 *)bufferPtr + 1);
	framePtr    = *((int16 *)bufferPtr + 2);
	returnVal   = *((int16 *)bufferPtr + 3);
	bufferPtr = (int16 *)bufferPtr + 4;

	waitAlarm = *((Alarm *)bufferPtr);
	bufferPtr = (Alarm *)bufferPtr + 1;

	stackOffset = *((int16 *)bufferPtr);
	bufferPtr = (int16 *)bufferPtr + 1;

	codeSeg = (UByteHandle)scriptRes->loadIndex(
	              programCounter.segment, "saga code segment");

	stackBase = (UBytePtr)RNewPtr(stackSize, NULL, "saga stack");
	stackPtr = stackBase + stackSize - stackOffset;

	memcpy(stackPtr, bufferPtr, stackOffset);
	bufferPtr = (uint8 *)bufferPtr + stackOffset;

	*buf = bufferPtr;
}

//-----------------------------------------------------------------------
//	Thread destructor

Thread::~Thread() {
	//  Clear extended bit if it was set
	clearExtended();

	//  Free the thread's code segment
	RUnlockHandle((RHANDLE)codeSeg);

	//  Deallocate the thread stack
	RDisposePtr(stackBase);
}

//-----------------------------------------------------------------------
//	Return the number of bytes need to archive this thread in an arhive
//	buffer

int32 Thread::archiveSize(void) {
	return      sizeof(programCounter)
	            +   sizeof(stackSize)
	            +   sizeof(flags)
	            +   sizeof(framePtr)
	            +   sizeof(returnVal)
	            +   sizeof(waitAlarm)
	            +   sizeof(int16)                //  stack offset
	            + (stackBase + stackSize) - stackPtr;
}

//-----------------------------------------------------------------------
//	Create an archive of this thread in an archive buffer

void *Thread::archive(void *buf) {
	int16   stackOffset;

	*((SegmentRef *)buf) = programCounter;
	buf = (SegmentRef *)buf + 1;

	*((int16 *)buf)        = stackSize;
	*((int16 *)buf + 1)    = flags;
	*((int16 *)buf + 2)    = framePtr;
	*((int16 *)buf + 3)    = returnVal;
	buf = (int16 *)buf + 4;

	*((Alarm *)buf) = waitAlarm;
	buf = (Alarm *)buf + 1;

	stackOffset = (stackBase + stackSize) - stackPtr;
	*((int16 *)buf) = stackOffset;
	buf = (int16 *)buf + 1;

	memcpy(buf, stackPtr, stackOffset);
	buf = (uint8 *)buf + stackOffset;

	return buf;
}

//-----------------------------------------------------------------------
//	Thread dispatcher

void Thread::dispatch(void) {
	Thread              *th,
	                    *nextThread;

	int                 numThreads = 0,
	                    numExecute = 0,
	                    numWaitDelay = 0,
	                    numWaitFrames = 0,
	                    numWaitSemi = 0,
	                    numWaitOther = 0;

#if DEBUG
	for (th = threadList.first(); th; th = threadList.next(th)) {
		if (th->flags & waiting) {
			switch (th->waitType) {

			case waitDelay:
				numWaitDelay++;
				break;
			case waitFrameDelay:
				numWaitFrames++;
				break;
			case waitTagSemaphore:
				numWaitSemi++;
				break;
			default:
				numWaitOther++;
				break;
			}
		} else numExecute++;
		numThreads++;
	}

	WriteStatusF(17, "Threads:%d X:%d D:%d F:%d T:%d O:%d", numThreads, numExecute, numWaitDelay, numWaitFrames, numWaitSemi, numWaitOther);
#endif

	for (th = threadList.first(); th; th = nextThread) {
		nextThread = threadList.next(th);

		if (th->flags & (finished | aborted)) {
			delete th;
			continue;
		}

		if (th->flags & waiting) {
			switch (th->waitType) {

			case waitDelay:

				//  Wake up the thread!

				if (th->waitAlarm.check())
					th->flags &= ~waiting;
				break;

			case waitFrameDelay:

				if (th->waitFrameAlarm.check())
					th->flags &= ~waiting;
				break;

			case waitTagSemaphore:
				if (((ActiveItem *)th->waitParam)->isExclusive() == FALSE) {
					th->flags &= ~waiting;
					((ActiveItem *)th->waitParam)->setExclusive(TRUE);
				}
				break;
			}
		}

		do {
			if (th->flags & (waiting | finished | aborted))
				break;

			if (th->interpret())
				goto break_thread_loop;
		} while (th->flags & synchronous);
	}
break_thread_loop:
	;
}

//-----------------------------------------------------------------------
//	Run scripts which are on the queue

void dispatchScripts(void) {
	Thread::dispatch();
}

//-----------------------------------------------------------------------
//	Run a script until finished

scriptResult Thread::run(void) {
	int             i = 4000;

	while (i--) {
		//  If script stopped, then return
		if (flags & (waiting | finished | aborted)) {
			if (flags & finished)   return scriptResultFinished;
			if (flags & waiting)    return scriptResultAsync;
			return scriptResultAborted;
			// can't ever fall thru here...
		}

		//  run the script some more...
		interpret();
	}
	error("Thread timed out!\n");
#ifdef _WIN32
	return scriptResultFinished;
#endif
}

//-----------------------------------------------------------------------
//	Convert to extended thread

void Thread::setExtended(void) {
	if (!(flags & extended)) {
		flags |= extended;
		extendedThreadLevel++;
	}
}

//-----------------------------------------------------------------------
//	Convert back to regular thread

void Thread::clearExtended(void) {
	if (flags & extended) {
		flags &= ~extended;
		extendedThreadLevel--;
	}
}

/* ============================================================================ *
                        Script Management functions
 * ============================================================================ */

void initScripts(void) {
	//  Open the script resource group
	scriptRes = scriptResFile->newContext(sagaID,  "script resources");
	if (scriptRes == NULL)
		error("Unable to open script resource file!\n");

	//  Load the data segment
	dataSegment = scriptRes->loadResource(dataSegID, scriptResFile->_filename, "saga data segment");

	if (dataSegment == NULL)
		error("Unable to load the SAGA data segment");

	dataSegSize = scriptRes->getSize(dataSegID, "saga data segment");

//	Common::hexdump(dataSegment, dataSegSize);

	exportSegment = scriptRes->loadResource(exportSegID, scriptResFile->_filename, "saga export segment");
	assert(exportSegment != NULL);

//	Common::hexdump(exportSegment, scriptRes->getSize(exportSegID, "saga export segment"));

	exportCount = (scriptRes->getSize(exportSegID, "saga export segment") / sizeof(uint32)) + 1;
}

void cleanupScripts(void) {
	if (exportSegment)
		free(exportSegment);

	if (dataSegment)
		free(dataSegment);

	if (scriptRes)
		scriptResFile->disposeContext(scriptRes);
	scriptRes = NULL;
}

//-----------------------------------------------------------------------
//	Load the SAGA data segment from the resource file

void initSAGADataSeg(void) {
	//  Load the data segment
	scriptRes->seek(dataSegID);
	scriptRes->read(dataSegment, dataSegSize);
}

//-----------------------------------------------------------------------
//	Save the SAGA data segment to a save file

void saveSAGADataSeg(SaveFileConstructor &saveGame) {
	saveGame.writeChunk(
	    MakeID('S', 'D', 'T', 'A'),
	    dataSegment,
	    dataSegSize);
}

//-----------------------------------------------------------------------
//	Load the SAGA data segment from a save file

void loadSAGADataSeg(SaveFileReader &saveGame) {
	saveGame.read(dataSegment, dataSegSize);
}

//-----------------------------------------------------------------------
//	Look up an entry in the SAGA export table

static bool lookupExport(
    uint16          entry,
    uint16          &segNum,
    uint16          &segOff) {
	uint32          segRef,
	                *exportBase = (uint32 *)(*exportSegment - 2);

	assert(entry > 0);
	assert(entry <= exportCount);

	segRef = exportBase[ entry ];
	segOff = segRef >> 16,
	segNum = segRef & 0x0000ffff;

	lastExport = entry;
	if (segNum > 1000)
		error("SAGA failure: Bad data in export table entry #%d (see scripts.r)", entry);

	return TRUE;
}

//-----------------------------------------------------------------------
//	Run a script to completion (or until it forks)

scriptResult runScript(uint16 exportEntryNum, scriptCallFrame &args) {
	uint16          segNum,
	                segOff;
	Thread          *th;
	scriptResult    result;
	Thread          *saveThread = thisThread;

	//  Lookup function entry point in export table
	if (exportEntryNum < 0)
		error("SAGA failure: Attempt to run script with invalid export ID %d.", exportEntryNum);

	assert(exportEntryNum > 0);
	lookupExport(exportEntryNum, segNum, segOff);

	//  Create a new thread
	th = new Thread(segNum, segOff, args);
	thisThread = th;
	print_script_name((*th->codeSeg) + th->programCounter.offset, objectName(segNum, segOff));

	//  Run the thread to completion
	result = th->run();
	args.returnVal = th->returnVal;

	//  If the thread is not still running, then delete it
	if (result != scriptResultAsync) delete th;

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
	scriptResult    result = scriptResultNoScript;
	Thread          *saveThread = thisThread;

	//  For abstract classes, the object index is also the class
	//  index.
	if (bType == builtinAbstract) index = scriptClassID;

	//  Lookup class function table in export table
	if (scriptClassID < 0)
		error("SAGA failure: Attempt to run object script with invalid export ID %d.\n", scriptClassID);

	lookupExport(scriptClassID, segNum, segOff);

	//  Get address of class function table
	vTable = (uint16 *)
	         segmentAddress(segNum, segOff + methodNum * sizeof(uint32));

	segNum = vTable[ 0 ];
	segOff = vTable[ 1 ];

	if (segNum == 0xffff) {                 // it's a CFUNC or NULL func
		if (segOff == 0xffff) {             // it's a NULL function
			return scriptResultNoScript;
		} else {                            //  It's a C function
			int16   funcNum = segOff;       // function number
			int16   stack[ 1 ];             // dummy stack argument
			C_Call  *cfunc;

			//  Make sure the C function number is OK
			assert(funcNum >= 0);
			assert(funcNum < globalCFuncs.numEntries);
			cfunc = globalCFuncs.table[ funcNum ];

			//  Build a temporary dummy thread
			th = new Thread(0, 0, args);
			thisThread = th;

			result = (scriptResult)cfunc(stack);   // call the function
			delete th;
		}
	} else {
		//  Create a new thread
		th = new Thread(segNum, segOff, args);
		thisThread = th;
		print_script_name((*th->codeSeg) + th->programCounter.offset, objectName(bType, index));

		//  Put the object segment and ID onto the dummy stack frame
		((uint16 *)th->stackPtr)[ 3 ] = bType;
		((uint16 *)th->stackPtr)[ 4 ] = index;

		//  Run the thread to completion
		result = th->run();
		args.returnVal = th->returnVal;

		if (result != scriptResultAsync) delete th;
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

	assert(obj = GameObject::objectAddress(id));

	return runMethod(obj->scriptClass(),
	                 builtinTypeObject,
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

	assert(aItem = ActiveItem::activeItemAddress(index));
	if (!aItem->scriptClassID)
		return scriptResultNoScript;

	return runMethod(aItem->scriptClassID,
	                 builtinTypeTAG,
	                 index,
	                 methodNum,
	                 args);
}

//-----------------------------------------------------------------------
//	Wake up a thread unconditionally

void wakeUpThread(ThreadID id) {
	if (id != NoThread) {
		Thread  *thread = getThreadAddress(id);

		thread->flags &= ~Thread::waiting;
	}
}

void wakeUpThread(ThreadID id, int16 returnVal) {
	if (id != NoThread) {
		Thread  *thread = getThreadAddress(id);

		if (thread->flags & Thread::expectResult) {
			WriteStatusF(8, "Result %d", returnVal);
			thread->returnVal = returnVal;
			*(int16 *)thread->stackPtr = returnVal;
		} else WriteStatusF(8, "Thread not expecting result!");

		thread->flags &= ~(Thread::waiting | Thread::expectResult);
	}
}

//-----------------------------------------------------------------------
//	Old routines

#if 0
void wakeUpThreadsDelayed(enum WaitTypes wakeupType, int newdelay) {
	Thread              *th;

	for (th = threadList.first(); th; th = th->next) {
		if ((th->flags & THREADF_WAITING) && th->waitType == wakeupType) {
			//  Set thread to delayed mode.

			SetAlarm(&th->waitAlarm, newdelay);      // set the alarm
			th->waitType = TWAIT_DELAY;
		}
	}
}

void abortObjectThreads(Thread *keep, uint16 objID) {
	Thread              *th;

	for (th = threadList.first(); th; th = th->next) {
		if (th != keep && objID == th->threadArgs.theActor) {
			th->flags &= ~THREADF_WAITING;
			th->flags |= THREADF_ABORTED;
		}
	}
}

bool abortAllThreads(void) {
	bool                result = TRUE;
	Thread              *th;

	for (th = threadList.first(); th; th = th->next) {
#if 0
		if (th->flags & THREADF_WAITING) {
			switch (th->waitType) {

			case TWAIT_DELAY:
			case TWAIT_SPEECH:
				break;

			case TWAIT_DIALOG_BEGIN:
			case TWAIT_DIALOG_END:
				break;
			}
		}
#endif
		th->flags |= THREADF_ABORTED;
	}
	dispatchThreads();
	return result;
}
#endif

} // end of namespace Saga2
