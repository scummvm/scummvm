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

#ifndef SAGA2_CODE_H
#define SAGA2_CODE_H

namespace Saga2 {

// types of operations for code generation
// note that the macros listed below depend on the ordering of this
// table to determine things like unary-op, binary-op, etc.

enum op_types {
	kOpUndefined = 0,

	//  internal operations

	kOpNextblock,                           // continue execution at next block
	kOpDup,                                 // duplicate 16-bit value on stack
	kOpDrop,                                // drop 16-bit value on stack

	//  primary values

	kOpZero,                                // push a zero on the stack
	kOpOne,                                 // push a one on the stack
	kOpConstint,                            // constant integer
	kOpConstid,                             // constant id reference
	kOpStrlit,                              // string literal
	kOpSym,                                 // symbol address
	kOpSymref,                              // symbol contents
	kOpClassref,                            // reference to "this"
	kOpDeref,                               // dereference of an ID

	//  references within this module

	kOpGetflag,                             // read from flag bit (mode)
	kOpGetbyte,                             // read from byte field (mode)
	kOpGetint,                              // read from integer field (mode)
	kOpGetstr,                              // read from string field (mode)
	kOpGetid,                               // read from id field (mode)

	kOpPutflag,                             // put to flag bit (mode)
	kOpPutbyte,                             // put to byte field (mode)
	kOpPutint,                              // put to integer field (mode)
	kOpPutstr,                              // put to string field (mode)
	kOpPutid,                               // put to id field (mode)

	kOpPea,                                 // push effective address onto stack

	//  'void' versions consume their arguments

	kOpPutflagV,                           // put to flag bit (mode)
	kOpPutbyteV,                           // put to byte field (mode)
	kOpPutintV,                            // put to integer field (mode)
	kOpPutstrV,                            // put to string field (mode)
	kOpPutidV,                             // put to id field (mode)

	//  function call

	kOpCallNear,                           // call function in same segment
	kOpCallFar,                            // call function in other segment
	kOpCcall,                               // call C function
	kOpCcallV,                             // call C function ()
	kOpCallMember,                         // call member function
	kOpCallMemberV,                       // call member function ()

	kOpEnter,                               // enter a function
	kOpReturn,                              // return from function
	kOpReturn_v,                            // return nothing from function

	//  branches

	kOpJmp,
	kOpJmpTrueV,                          // test arg and consume
	kOpJmpFalseV,                         // test arg and consume
	kOpJmpTrue,                            // test arg and don't consume
	kOpJmpDalse,                           // test arg and don't consume
	kOpJmpSwitch,                          // switch statement (integer)
	kOpJmpStrswitch,                       // switch statement (string)
	kOpJmpRandom,                          // random jump

	//  unary operators

	kOpNegate,
	kOpNot,
	kOpCompl,

	kOpIncV,                               // increment, don't push
	kOpDecV,                               // decrement, don't push
	kOpPostinc,
	kOpPostdec,

	// arithmetic

	kOpAdd,
	kOpSub,
	kOpMul,
	kOpDiv,
	kOpMod,

	//  conditional

	kOpConditional,
	kOpComma,

	//  comparison

	kOpEq,
	kOpNe,
	kOpGt,
	kOpLt,
	kOpGe,
	kOpLe,

	//  string comparison

	kOpStrEq,
	kOpStrNe,
	kOpStrGt,
	kOpStrLt,
	kOpStrGe,
	kOpStrLe,

	//  shift

	kOpRsh,
	kOpLsh,

	//  bitwise

	kOpAnd,
	kOpOr,
	kOpXor,

	//  logical

	kOpLand,
	kOpLor,
	kOpLxor,

	//  string functions

	kOpStrcat,                  // string concatenation
	kOpStrformat,               // string formatting

	// assignment operators -- none of these are actually compiled into
	// code (they become get/put type operations)

	kOpAssign,
	// none of these are even used currently...
	kOpAsplus,
	kOpAsminus,
	kOpAstimes,
	kOpAsdiv,
	kOpAsmod,
	kOpAsrshift,
	kOpAslshift,
	kOpAsand,
	kOpAsor,

	//  Special ops

	kOpSpeak,
	kOpDialogBegin,
	kOpDialogEnd,
	kOpReply,
	kOpAnimate,

// New opcodes

	kOpJmp_seedrandom,                      // seeded random jump
	kOpSymrefX,                            // get the export number of the symbol

	kOpLast         /* about 90 so far */
};

// addressing modes for get and put

enum addr_types {

	//  Offset reference to the thread structure

	skAddrThread = 0,

	//  Offset reference to the stack

	skAddrStack,

	//  Implicit reference to the currently executing segment

	skAddrNear,

	//  Implicit reference to data segment

	skAddrData,

	//  This addressing mode references any external segment
	//  using a 16-bit segment number and a 16-bit segment offset
	//  which immediately follow the instruction.

	skAddrFar,

	//  This addressing mode is used for segment-array addressing
	//  it's a 16-bit segment followed by a 16-bit object number,
	//  followed by a (byte or bit) offset within the object

	skAddrArray,

	//  This addressing mode uses a 16-bit segment number
	//  and a 16-bit offset which have been put on the stack.

//	skAddrIndirect,                      // use SEG:offset on stack
//	skAddrIndirectIndex,                 // use SEG:index:offset on stack

	//  This addressing mode is used for dereferencing objects.
	//  It consists of an _embedded_ address for retrieving the
	//  object number, followed by a 16-bit segment number for
	//  the dereferenced objects, followed by a 16-bit index
	//  into the dereferenced object.

	//  REM: We also need a "far deref" for computing the
	//  dereferenced object's segment number.

	skAddrDeref,

	//  Addressing mode used for class member functions. It
	//  specified that the address is relative to whatever
	//  object the 1st argument is referrring to.

	skAddrThis                              // relative to arg 1
};

#define IS_CONST(x)     ((x) >= kOpConstint && (x) <= kOpConststr)
#define IS_ADDRESS(x)   ((x) == kOpSym)
#define IS_UNOP(x)      ((x) >= kOpNegate && (x) <= kOpPostdec)

#define IS_BINOP(x)     ((x) >= kOpAdd && (x) <= kOpStrcat)
#define IS_ASOP(x)      ((x) >= kOpAssign && (x) <= kOpAsor)

// #define IS_UNOP2(x)      ((x) == kOpGetarray || (x) == kOpPutarray)
// #define CONST(op)        ((op) >= kOpConstflag && (op) <= kOpConststr)

//  Flags for special statements

#define SPEAKF_NOANIMATE (1<<0)             // speaker should animate
#define SPEAKF_ASYNC    (1<<1)              // async speech.

#define REPLYF_ONCE     (1<<0)              // 'once' flag
#define REPLYF_SUMMARY  (1<<1)              // response is only a summary
#define REPLYF_CONDITION (1<<2)             // response has a condition

#define ANIMATEF_REPEAT (1<<0)

//  BasicBlock describes a block of generated code

#ifdef COMPILE_H

typedef struct _BasicBlock {
	struct _BasicBlock  *next;              // pointer to next block

	short               label;              // label for this block

	struct _BasicBlock  *from[2],           // where we could have come from
			       *jumpto,            // where to go if jump
			       *fallto;            // where to go if fall through

		struct _Statement   *first_st,          // statement list for this bblock
			       *last_st;           // it's a circular list

		ENode               *test_expr;         // test expression for jump
		char                in_flags,           // flags for entering this block
		                    jump_type;          // where to go after this block

		uint16              start_offset,       // offset in module of this block
		                    jump_offset;        // offset of module of jump at end

		int16               source_file,        // source line of statement
		                    source_line;        // source file of statement
	} BasicBlock;

//  Various flags for the block

#define BBLOCK_CLABEL   (1<<0)              /* bblock begins with a 'C' label */
#define BBLOCK_INTLABEL (1<<1)              /* bblock begins with internal label */
#define BBLOCK_FIRST    (1<<2)              /* first bblock in the function */
#define BBLOCK_MANY     (1<<3)              /* many guys jump to here       */
#define BBLOCK_CASE     (1<<4)              /* case stmts might not have 'from' ptr */

//  Jump types

enum jump_types {
	kJumpNever = 0,                         /* never jump                   */
	kJumpFalse,                             /* jump on expression if false  */
	kJumpTrue,                              /* jump on expression if true   */
	kJumpAlways,                            /* jump always                  */
	kJumpCjump,                             /* 'c' goto stm, jumpto is a label # */
	kJumpReturn,                            /* block ends with a 'return' statement */
	kJumpSwitch,                            /* jumps to lots of places      */
};
#endif
/*
    //  Module describes all the code associated with a particular
    //  compilation unit.

struct Module {
    uint16              exportCount,        // number of symbols exported
                        importCount;        // number of symbols imported
    uint16              exportOffset,       // start of export table
                        importOffset;       // start of import table

        // Q: Should ModuleName be somewhere else, like in the main program?

    uint16              staticSize;         // size of static data
    uint16              stringOffset;       // unused in this version
//  int16               moduleName;         // offset to name of module

        //  Followed by list of exports
        //  Followed by list of imports
        //  Followed by symbol names, each NULL-terminated
        //  Followed by actual code blocks

};

    //  Exports: Each Export contains the offset to find the name of
    //  the object. After that is the offset to find the object itself.

    //  The first Export is always the module entry point

struct ModuleExport {                       // an exported symbol
    uint16              symbolOffset,       // offset in module of symbol name
                        objectOffset;       // where to find object
};

    //  Imports: Each Import contains the offset to find the name of
    //  the object. After that is a blank pointer, which will be filled
    //  in with the real address of the object. When addressing the
    //  imported object, the code in this module will refer to this
    //  ModuleImport entry, and indirectly locate the referenced object.

struct ModuleImport {                       // a module xref
    uint16              symbolOffset;       // where, from here, to find name
    void                *objectPointer;     // NULL pointer, filled in on load
};
*/

} // end of namespace Saga2

#endif
