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

#ifndef SCI_ENGINE_VM_H
#define SCI_ENGINE_VM_H

/* VM and kernel declarations */

#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/resource.h"	// for SciVersion

#include "common/util.h"

namespace Sci {

class SegManager;
struct EngineState;
class Object;
class ResourceManager;

/** Number of bytes to be allocated for the stack */
#define VM_STACK_SIZE 0x1000

/** Magical object identifier */
#define SCRIPT_OBJECT_MAGIC_NUMBER 0x1234

/** Offset of this identifier */
#define SCRIPT_OBJECT_MAGIC_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? -8 : 0)

/** Script-relative offset of the species ID */
#define SCRIPT_SPECIES_OFFSET 8 -8

#define SCRIPT_SUPERCLASS_OFFSET (getSciVersion() < SCI_VERSION_1_1 ? 10 -8 : 12)

/** Stack pointer value: Use predecessor's value */
#define CALL_SP_CARRY NULL

/** Types of selectors as returned by lookupSelector() below. */
enum SelectorType {
	kSelectorNone = 0,
	kSelectorVariable,
	kSelectorMethod
};

struct Class {
	int script; ///< number of the script the class is in, -1 for non-existing
	reg_t reg; ///< offset; script-relative offset, segment: 0 if not instantiated
};

#define RAW_IS_OBJECT(datablock) (READ_SCI11ENDIAN_UINT16(((byte *) datablock) + SCRIPT_OBJECT_MAGIC_OFFSET) == SCRIPT_OBJECT_MAGIC_NUMBER)

// A reference to an object's variable.
// The object is stored as a reg_t, the variable as an index into _variables
struct ObjVarRef {
	reg_t obj;
	int varindex;

	reg_t* getPointer(SegManager *segMan) const;
};

enum ExecStackType {
	EXEC_STACK_TYPE_CALL = 0,
	EXEC_STACK_TYPE_KERNEL = 1,
	EXEC_STACK_TYPE_VARSELECTOR = 2
};

struct ExecStack {
	reg_t objp;  ///< Pointer to the beginning of the current object
	reg_t sendp; ///< Pointer to the object containing the invoked method

	union {
		ObjVarRef varp; // Variable pointer for r/w access
		reg_t pc;       // Pointer to the initial program counter. Not accurate for the TOS element
	} addr;

	StackPtr fp; // Frame pointer
	StackPtr sp; // Stack pointer
	int argc;

	StackPtr variables_argp; // Argument pointer
	SegmentId local_segment; // local variables etc

	Selector selector;      // The selector which was used to call or -1 if not applicable
	int exportId;           // The exportId which was called or -1 if not applicable
	int localCallOffset;    // Local call offset or -1 if not applicable
	int origin;             // The stack frame position the call was made from, or -1 if it was the initial call
	ExecStackType type;

	reg_t* getVarPointer(SegManager *segMan) const;
};

enum {
	VAR_GLOBAL = 0,
	VAR_LOCAL = 1,
	VAR_TEMP = 2,
	VAR_PARAM = 3
};

/** Number of kernel calls in between gcs; should be < 50000 */
enum {
	GC_INTERVAL = 32768
};


/**
 * Executes function pubfunct of the specified script.
 * @param[in] s				The state which is to be executed with
 * @param[in] script		The script which is called
 * @param[in] pubfunct		The exported script function which is to
 * 							be called
 * @param[in] sp			Stack pointer position
 * @param[in] calling_obj	The heap address of the object that
 * 							executed the call
 * @param[in] argc			Number of arguments supplied
 * @param[in] argp			Pointer to the first supplied argument
 * @return					A pointer to the new exec stack TOS entry
 */
ExecStack *execute_method(EngineState *s, uint16 script, uint16 pubfunct,
		StackPtr sp, reg_t calling_obj, uint16 argc, StackPtr argp);


/**
 * Executes a "send" or related operation to a selector.
 * @param[in] s			The EngineState to operate on
 * @param[in] send_obj	Heap address of the object to send to
 * @param[in] work_obj	Heap address of the object initiating the send
 * @param[in] sp		Stack pointer position
 * @param[in] framesize	Size of the send as determined by the "send"
 * 						operation
 * @param[in] argp		Pointer to the beginning of the heap block
 * 						containing the data to be sent. This area is a
 * 						succession of one or more sequences of
 * 						[selector_number][argument_counter] and then
 * 						"argument_counter" word entries with the
 * 						parameter values.
 * @return				A pointer to the new execution stack TOS entry
 */
ExecStack *send_selector(EngineState *s, reg_t send_obj, reg_t work_obj,
	StackPtr sp, int framesize, StackPtr argp);


/**
 * This function executes SCI bytecode
 * It executes the code on s->heap[pc] until it hits a 'ret' operation
 * while (stack_base == stack_pos). Requires s to be set up correctly.
 * @param[in] s			The state to use
 * @param[in] restoring	true if s has just been restored, false otherwise
 */
void run_vm(EngineState *s, bool restoring);

/**
 * Debugger functionality
 * @param[in] s					The state at which debugging should take place
 */
void script_debug(EngineState *s);

/**
 * Looks up a selector and returns its type and value
 * varindex is written to iff it is non-NULL and the selector indicates a property of the object.
 * @param[in] segMan		The Segment Manager
 * @param[in] obj			Address of the object to look the selector up in
 * @param[in] selectorid	The selector to look up
 * @param[out] varp			A reference to the selector, if it is a
 * 							variable.
 * @param[out] fptr			A reference to the function described by that
 * 							selector, if it is a valid function selector.
 * 							fptr is written to iff it is non-NULL and the
 * 							selector indicates a member function of that
 * 							object.
 * @return					kSelectorNone if the selector was not found in
 * 							the object or its superclasses.
 * 							kSelectorVariable if the selector represents an
 * 							object-relative variable.
 * 							kSelectorMethod if the selector represents a
 * 							method
 */
SelectorType lookupSelector(SegManager *segMan, reg_t obj, Selector selectorid,
		ObjVarRef *varp, reg_t *fptr);

/**
 * Makes sure that a script and its superclasses get loaded to the heap.
 * If the script already has been loaded, only the number of lockers is
 * increased. All scripts containing superclasses of this script are loaded
 * recursively as well, unless 'recursive' is set to zero. The
 * complementary function is "script_uninstantiate()" below.
 * @param[in] resMan		The resource manager
 * @param[in] segMan	The segment manager
 * @param[in] script_nr		The script number to load
 * @return					The script's segment ID or 0 if out of heap
 */
int script_instantiate(ResourceManager *resMan, SegManager *segMan, int script_nr);

/**
 * Decreases the numer of lockers of a script and unloads it if that number
 * reaches zero.
 * This function will recursively unload scripts containing its
 * superclasses, if those aren't locked by other scripts as well.
 * @param[in] segMan	The segment manager
 * @param[in] version		The SCI version to use
 * @param[in] script_nr	The script number that is requestet to be unloaded
 */
void script_uninstantiate(SegManager *segMan, int script_nr);

/**
 * Read a PMachine instruction from a memory buffer and return its length.
 *
 * @param[in] src		address from which to start parsing
 * @param[out] extOpcode	"extended" opcode of the parsed instruction
 * @param[out] opparams	parameter for the parsed instruction
 * @return the length in bytes of the instruction
 *
 * @todo How about changing opparams from int16 to int / int32 to preserve
 *       unsigned 16bit words as read for Script_Word? In the past, this
 *       was irrelevant as only a debug opcode used Script_Word. But with
 *       SCI32 we are now using Script_Word for more opcodes. Maybe this is
 *       just a mistake and those opcodes should used Script_SWord -- but if
 *       not then we definitely should change this to int, else we might run
 *       into trouble if we encounter high value words. *If* those exist at all.
 */
int readPMachineInstruction(const byte *src, byte &extOpcode, int16 opparams[4]);

} // End of namespace Sci

#endif // SCI_ENGINE_VM_H
