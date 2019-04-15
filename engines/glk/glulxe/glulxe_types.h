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
 */

#ifndef GLK_GLULXE_TYPES
#define GLK_GLULXE_TYPES

#include "common/scummsys.h"

namespace Glk {
namespace Glulxe {


	/* Comment this definition to turn off memory-address checking. With
	   verification on, all reads and writes to main memory will be checked
	   to ensure they're in range. This is slower, but prevents malformed
	   game files from crashing the interpreter. */
#define VERIFY_MEMORY_ACCESS (1)

	   /* Uncomment this definition to permit an exception for memory-address
		  checking for @glk and @copy opcodes that try to write to memory address 0.
		  This was a bug in old Superglus-built game files. */
		  /* #define TOLERATE_SUPERGLUS_BUG (1) */

		  /* Uncomment this definition to turn on Glulx VM profiling. In this
			 mode, all function calls are timed, and the timing information is
			 written to a data file called "profile-raw".
			 (Build note: on Linux, glibc may require you to also define
			 _BSD_SOURCE or _DEFAULT_SOURCE or both for the timeradd() macro.) */
			 /* #define VM_PROFILING (1) */

			 /* Uncomment this definition to turn on the Glulx debugger. You should
				only do this when debugging facilities are desired; it slows down
				the interpreter. If you do, you will need to build with libxml2;
				see the Makefile. */
				/* #define VM_DEBUGGER (1) */

				/* Comment this definition to turn off floating-point support. You
				   might need to do this if you are building on a very limited platform
				   with no math library. */
#define FLOAT_SUPPORT (1)

				   /* Comment this definition to not cache the original state of RAM in
					  (real) memory. This saves some memory, but slows down save/restore/undo
					  operations, which will have to read the original state off disk
					  every time. */
#define SERIALIZE_CACHE_RAM (1)

/**
 * Some macros to read and write integers to memory, always in big-endian format.
 */
#define Read4(ptr) READ_BE_UINT32(ptr)
#define Read2(ptr) READ_BE_UINT16(ptr)
#define Read1(ptr) ((byte)(((byte *)(ptr))[0]))
#define Write4(ptr, vl) WRITE_BE_UINT32(ptr, vl)
#define Write2(ptr, vl) WRITE_BE_UINT16(ptr, vl)
#define Write1(ptr, vl) (((byte *)(ptr))[0] = (vl))

#if VERIFY_MEMORY_ACCESS
#define Verify(adr, ln) verify_address(adr, ln)
#define VerifyW(adr, ln) verify_address_write(adr, ln)
#else
#define Verify(adr, ln) (0)
#define VerifyW(adr, ln) (0)
#endif /* VERIFY_MEMORY_ACCESS */

#define Mem1(adr)  (Verify(adr, 1), Read1(memmap+(adr)))
#define Mem2(adr)  (Verify(adr, 2), Read2(memmap+(adr)))
#define Mem4(adr)  (Verify(adr, 4), Read4(memmap+(adr)))
#define MemW1(adr, vl)  (VerifyW(adr, 1), Write1(memmap+(adr), (vl)))
#define MemW2(adr, vl)  (VerifyW(adr, 2), Write2(memmap+(adr), (vl)))
#define MemW4(adr, vl)  (VerifyW(adr, 4), Write4(memmap+(adr), (vl)))

/**
 * Macros to access values on the stack. These *must* be used with proper alignment!
 * (That is, Stk4 and StkW4 must take addresses which are multiples of four, etc.)
 * If the alignment rules are not followed, the program will see performance
 * degradation or even crashes, depending on the machine CPU.
 */
#define Stk1(adr)   \
  (*((unsigned char *)(stack+(adr))))
#define Stk2(adr)   \
  (*((uint16 *)(stack+(adr))))
#define Stk4(adr)   \
  (*((uint32 *)(stack+(adr))))

#define StkW1(adr, vl)   \
  (*((byte *)(stack+(adr))) = (byte)(vl))
#define StkW2(adr, vl)   \
  (*((uint16 *)(stack+(adr))) = (uint16)(vl))
#define StkW4(adr, vl)   \
  (*((uint32 *)(stack+(adr))) = (uint32)(vl))


struct dispatch_splot_struct {
	int numwanted;
	int maxargs;
	gluniversal_t *garglist;
	uint *varglist;
	int numvargs;
	uint *retval;
};
typedef dispatch_splot_struct dispatch_splot_t;

/**
 * We maintain a linked list of arrays being used for Glk calls. It is only used for integer
 * (uint) arrays -- char arrays are handled in place. It's not worth bothering with a hash table,
 * since most arrays appear here only momentarily.
 */
struct arrayref_struct {
	void *array;
	uint addr;
	uint elemsize;
	uint len; /* elements */
	int retained;
	arrayref_struct *next;
};
typedef arrayref_struct arrayref_t;

/**
 * We maintain a hash table for each opaque Glk class. classref_t are the nodes of the table,
 * and classtable_t are the tables themselves.
 */
struct classref_struct {
	void *obj;
	uint id;
	int bucknum;
	classref_struct *next;
};
typedef classref_struct classref_t;

#define CLASSHASH_SIZE (31)
struct classtable_struct {
	uint lastid;
	classref_t *bucket[CLASSHASH_SIZE];
};
typedef classtable_struct classtable_t;

/**
 * Represents the operand structure of an opcode.
 */
struct operandlist_struct {
	int num_ops; /* Number of operands for this opcode */
	int arg_size; /* Usually 4, but can be 1 or 2 */
	int *formlist; /* Array of values, either modeform_Load or modeform_Store */
};
typedef operandlist_struct operandlist_t;

enum modeform {
	modeform_Load = 1,
	modeform_Store = 2
};

/**
 * Represents one operand value to an instruction being executed. The
 * code in exec.c assumes that no instruction has more than MAX_OPERANDS of these.
*/
struct oparg_struct {
	uint desttype;
	uint value;
};
typedef oparg_struct oparg_t;

#define MAX_OPERANDS (8)

} // End of namespace Glulxe
} // End of namespace Glk

#endif
