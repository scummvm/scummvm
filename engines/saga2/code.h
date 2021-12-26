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
	op_undefined = 0,

	//  internal operations

	op_nextblock,                           // continue execution at next block
	op_dup,                                 // duplicate 16-bit value on stack
	op_drop,                                // drop 16-bit value on stack

	//  primary values

	op_zero,                                // push a zero on the stack
	op_one,                                 // push a one on the stack
	op_constint,                            // constant integer
	op_constid,                             // constant id reference
	op_strlit,                              // string literal
	op_sym,                                 // symbol address
	op_symref,                              // symbol contents
	op_classref,                            // reference to "this"
	op_deref,                               // dereference of an ID

	//  references within this module

	op_getflag,                             // read from flag bit (mode)
	op_getbyte,                             // read from byte field (mode)
	op_getint,                              // read from integer field (mode)
	op_getstr,                              // read from string field (mode)
	op_getid,                               // read from id field (mode)

	op_putflag,                             // put to flag bit (mode)
	op_putbyte,                             // put to byte field (mode)
	op_putint,                              // put to integer field (mode)
	op_putstr,                              // put to string field (mode)
	op_putid,                               // put to id field (mode)

	op_pea,                                 // push effective address onto stack

	//  'void' versions consume their arguments

	op_putflag_v,                           // put to flag bit (mode)
	op_putbyte_v,                           // put to byte field (mode)
	op_putint_v,                            // put to integer field (mode)
	op_putstr_v,                            // put to string field (mode)
	op_putid_v,                             // put to id field (mode)

	//  function call

	op_call_near,                           // call function in same segment
	op_call_far,                            // call function in other segment
	op_ccall,                               // call C function
	op_ccall_v,                             // call C function ()
	op_call_member,                         // call member function
	op_call_member_v,                       // call member function ()

	op_enter,                               // enter a function
	op_return,                              // return from function
	op_return_v,                            // return nothing from function

	//  branches

	op_jmp,
	op_jmp_true_v,                          // test arg and consume
	op_jmp_false_v,                         // test arg and consume
	op_jmp_true,                            // test arg and don't consume
	op_jmp_false,                           // test arg and don't consume
	op_jmp_switch,                          // switch statement (integer)
	op_jmp_strswitch,                       // switch statement (string)
	op_jmp_random,                          // random jump

	//  unary operators

	op_negate,
	op_not,
	op_compl,

	op_inc_v,                               // increment, don't push
	op_dec_v,                               // decrement, don't push
	op_postinc,
	op_postdec,

	// arithmetic

	op_add,
	op_sub,
	op_mul,
	op_div,
	op_mod,

	//  conditional

	op_conditional,
	op_comma,

	//  comparison

	op_eq,
	op_ne,
	op_gt,
	op_lt,
	op_ge,
	op_le,

	//  string comparison

	op_str_eq,
	op_str_ne,
	op_str_gt,
	op_str_lt,
	op_str_ge,
	op_str_le,

	//  shift

	op_rsh,
	op_lsh,

	//  bitwise

	op_and,
	op_or,
	op_xor,

	//  logical

	op_land,
	op_lor,
	op_lxor,

	//  string functions

	op_strcat,                  // string concatenation
	op_strformat,               // string formatting

	// assignment operators -- none of these are actually compiled into
	// code (they become get/put type operations)

	op_assign,
	// none of these are even used currently...
	op_asplus,
	op_asminus,
	op_astimes,
	op_asdiv,
	op_asmod,
	op_asrshift,
	op_aslshift,
	op_asand,
	op_asor,

	//  Special ops

	op_speak,
	op_dialog_begin,
	op_dialog_end,
	op_reply,
	op_animate,

// New opcodes

	op_jmp_seedrandom,                      // seeded random jump
	op_symref_x,                            // get the export number of the symbol

	op_last         /* about 90 so far */
};

// addressing modes for get and put

enum addr_types {

	//  Offset reference to the thread structure

	addr_thread = 0,

	//  Offset reference to the stack

	addr_stack,

	//  Implicit reference to the currently executing segment

	addr_near,

	//  Implicit reference to data segment

	addr_data,

	//  This addressing mode references any external segment
	//  using a 16-bit segment number and a 16-bit segment offset
	//  which immediately follow the instruction.

	addr_far,

	//  This addressing mode is used for segment-array addressing
	//  it's a 16-bit segment followed by a 16-bit object number,
	//  followed by a (byte or bit) offset within the object

	addr_array,

	//  This addressing mode uses a 16-bit segment number
	//  and a 16-bit offset which have been put on the stack.

//	addr_indirect,                           // use SEG:offset on stack
//	addr_indirect_index,                 // use SEG:index:offset on stack

	//  This addressing mode is used for dereferencing objects.
	//  It consists of an _embedded_ address for retrieving the
	//  object number, followed by a 16-bit segment number for
	//  the dereferenced objects, followed by a 16-bit index
	//  into the dereferenced object.

	//  REM: We also need a "far deref" for computing the
	//  dereferenced object's segment number.

	addr_deref,

	//  Addressing mode used for class member functions. It
	//  specified that the address is relative to whatever
	//  object the 1st argument is referrring to.

	addr_this                              // relative to arg 1
};

#define IS_CONST(x)     ((x) >= op_constint && (x) <= op_conststr)
#define IS_ADDRESS(x)   ((x) == op_sym)
#define IS_UNOP(x)      ((x) >= op_negate && (x) <= op_postdec)

#define IS_BINOP(x)     ((x) >= op_add && (x) <= op_strcat)
#define IS_ASOP(x)      ((x) >= op_assign && (x) <= op_asor)

// #define IS_UNOP2(x)      ((x) == op_getarray || (x) == op_putarray)
// #define CONST(op)        ((op) >= op_constflag && (op) <= op_conststr)

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
	jump_never = 0,                         /* never jump                   */
	jump_false,                             /* jump on expression if false  */
	jump_true,                              /* jump on expression if true   */
	jump_always,                            /* jump always                  */
	jump_cjump,                             /* 'c' goto stm, jumpto is a label # */
	jump_return,                            /* block ends with a 'return' statement */
	jump_switch,                            /* jumps to lots of places      */
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
