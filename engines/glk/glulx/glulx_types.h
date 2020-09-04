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
namespace Glulx {

class Glulx;

/**
 * Comment this definition to turn off memory-address checking. With verification on,
 * all reads and writes to main memory will be checked to ensure they're in range.
 * This is slower, but prevents malformed game files from crashing the interpreter.
 */
#define VERIFY_MEMORY_ACCESS (1)

/**
 * Uncomment this definition to permit an exception for memory-address checking for @glk and @copy
 * opcodes that try to write to memory address 0. This was a bug in old Superglus-built game files.
 */
/* #define TOLERATE_SUPERGLUS_BUG (1) */

/**
 * Uncomment this definition to turn on Glulx VM profiling. In this mode, all function calls are timed,
 * and the timing information is written to a data file called "profile-raw".
 * (Build note: on Linux, glibc may require you to also define _BSD_SOURCE or _DEFAULT_SOURCE or both
 * for the timeradd() macro.)
 */
/* #define VM_PROFILING (1) */

/**
 * Uncomment this definition to turn on the Glulx debugger. You should only do this when debugging
 * facilities are desired; it slows down the interpreter. If you do, you will need to build with libxml2;
 * see the Makefile.
 */
/* #define VM_DEBUGGER (1) */

/**
 * Comment this definition to turn off floating-point support. You might need to do this if you are building
 * on a very limited platform with no math library.
 */
#define FLOAT_SUPPORT (1)

/**
 * Comment this definition to not cache the original state of RAM in (real) memory. This saves some memory,
 * but slows down save/restore/undo operations, which will have to read the original state off disk
 * every time.
 */
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

#define Mem1(adr)  (Read1(memmap+(adr)))
#define Mem2(adr)  (Read2(memmap+(adr)))
#define Mem4(adr)  (Read4(memmap+(adr)))
#define MemW1(adr, vl)  (VerifyW(adr, 1), Write1(memmap+(adr), (vl)))
#define MemW2(adr, vl)  (VerifyW(adr, 2), Write2(memmap+(adr), (vl)))
#define MemW4(adr, vl)  (VerifyW(adr, 4), Write4(memmap+(adr), (vl)))

#ifndef _HUGE_ENUF
#define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif
#ifndef INFINITY
#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#endif
#ifndef NAN
#define NAN        ((float)(INFINITY * 0.0F))
#endif

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

enum Opcode {
	op_nop          = 0x00,

	op_add          = 0x10,
	op_sub          = 0x11,
	op_mul          = 0x12,
	op_div          = 0x13,
	op_mod          = 0x14,
	op_neg          = 0x15,
	op_bitand       = 0x18,
	op_bitor        = 0x19,
	op_bitxor       = 0x1A,
	op_bitnot       = 0x1B,
	op_shiftl       = 0x1C,
	op_sshiftr      = 0x1D,
	op_ushiftr      = 0x1E,

	op_jump         = 0x20,
	op_jz           = 0x22,
	op_jnz          = 0x23,
	op_jeq          = 0x24,
	op_jne          = 0x25,
	op_jlt          = 0x26,
	op_jge          = 0x27,
	op_jgt          = 0x28,
	op_jle          = 0x29,
	op_jltu         = 0x2A,
	op_jgeu         = 0x2B,
	op_jgtu         = 0x2C,
	op_jleu         = 0x2D,

	op_call         = 0x30,
	op_return       = 0x31,
	op_catch        = 0x32,
	op_throw        = 0x33,
	op_tailcall     = 0x34,

	op_copy         = 0x40,
	op_copys        = 0x41,
	op_copyb        = 0x42,
	op_sexs         = 0x44,
	op_sexb         = 0x45,
	op_aload        = 0x48,
	op_aloads       = 0x49,
	op_aloadb       = 0x4A,
	op_aloadbit     = 0x4B,
	op_astore       = 0x4C,
	op_astores      = 0x4D,
	op_astoreb      = 0x4E,
	op_astorebit    = 0x4F,

	op_stkcount     = 0x50,
	op_stkpeek      = 0x51,
	op_stkswap      = 0x52,
	op_stkroll      = 0x53,
	op_stkcopy      = 0x54,

	op_streamchar   = 0x70,
	op_streamnum    = 0x71,
	op_streamstr    = 0x72,
	op_streamunichar = 0x73,

	op_gestalt      = 0x100,
	op_debugtrap    = 0x101,
	op_getmemsize   = 0x102,
	op_setmemsize   = 0x103,
	op_jumpabs      = 0x104,

	op_random       = 0x110,
	op_setrandom    = 0x111,

	op_quit         = 0x120,
	op_verify       = 0x121,
	op_restart      = 0x122,
	op_save         = 0x123,
	op_restore      = 0x124,
	op_saveundo     = 0x125,
	op_restoreundo  = 0x126,
	op_protect      = 0x127,

	op_glk          = 0x130,

	op_getstringtbl = 0x140,
	op_setstringtbl = 0x141,
	op_getiosys     = 0x148,
	op_setiosys     = 0x149,

	op_linearsearch = 0x150,
	op_binarysearch = 0x151,
	op_linkedsearch = 0x152,

	op_callf        = 0x160,
	op_callfi       = 0x161,
	op_callfii      = 0x162,
	op_callfiii     = 0x163,

	op_mzero        = 0x170,
	op_mcopy        = 0x171,
	op_malloc       = 0x178,
	op_mfree        = 0x179,

	op_accelfunc    = 0x180,
	op_accelparam   = 0x181,

	op_numtof       = 0x190,
	op_ftonumz      = 0x191,
	op_ftonumn      = 0x192,
	op_ceil         = 0x198,
	op_floor        = 0x199,
	op_fadd         = 0x1A0,
	op_fsub         = 0x1A1,
	op_fmul         = 0x1A2,
	op_fdiv         = 0x1A3,
	op_fmod         = 0x1A4,
	op_sqrt         = 0x1A8,
	op_exp          = 0x1A9,
	op_log          = 0x1AA,
	op_pow          = 0x1AB,
	op_sin          = 0x1B0,
	op_cos          = 0x1B1,
	op_tan          = 0x1B2,
	op_asin         = 0x1B3,
	op_acos         = 0x1B4,
	op_atan         = 0x1B5,
	op_atan2        = 0x1B6,
	op_jfeq         = 0x1C0,
	op_jfne         = 0x1C1,
	op_jflt         = 0x1C2,
	op_jfle         = 0x1C3,
	op_jfgt         = 0x1C4,
	op_jfge         = 0x1C5,
	op_jisnan       = 0x1C8,
	op_jisinf       = 0x1C9
};

enum gestulx {
	gestulx_GlulxVersion = 0,
	gestulx_TerpVersion  = 1,
	gestulx_ResizeMem    = 2,
	gestulx_Undo         = 3,
	gestulx_IOSystem     = 4,
	gestulx_Unicode      = 5,
	gestulx_MemCopy      = 6,
	gestulx_MAlloc       = 7,
	gestulx_MAllocHeap   = 8,
	gestulx_Acceleration = 9,
	gestulx_AccelFunc    = 10,
	gestulx_Float        = 11
};

/**
 * You may have to edit the definition of gfloat32 to make sure it's really a 32-bit floating-point type.
 */
typedef float gfloat32;

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
	int num_ops;            ///< Number of operands for this opcode
	int arg_size;           ///< Usually 4, but can be 1 or 2
	const int *formlist;    ///< Array of values, either modeform_Load or modeform_Store
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

typedef uint(Glulx::*acceleration_func)(uint argc, uint *argv);

struct accelentry_struct {
	uint addr;
	uint index;
	acceleration_func func;
	accelentry_struct *next;
};
typedef accelentry_struct accelentry_t;

#define ACCEL_HASH_SIZE (511)

struct heapblock_struct {
	uint addr;
	uint len;
	int isfree;
	struct heapblock_struct *next;
	struct heapblock_struct *prev;
};
typedef heapblock_struct heapblock_t;

/**
 * This structure allows us to write either to a Glk stream or to a dynamically-allocated memory chunk.
 */
struct dest_struct {
	bool _isMem;

	/* If it's a raw stream */
	Common::SeekableReadStream *_src;
	Common::WriteStream *_dest;

	/* If it's a block of memory: */
	byte *_ptr;
	uint _pos;
	uint _size;

	dest_struct() : _isMem(false), _src(nullptr), _dest(nullptr),
		_ptr(nullptr), _pos(0), _size(0) {}
};
typedef dest_struct dest_t;

/**
 * These constants are defined in the Glulx spec.
 */
enum iosys {
	iosys_None   = 0,
	iosys_Filter = 1,
	iosys_Glk    = 2
};

#define CACHEBITS (4)
#define CACHESIZE (1 << CACHEBITS)
#define CACHEMASK (15)

struct cacheblock_struct {
	int depth; /* 1 to 4 */
	int type;
	union {
		struct cacheblock_struct *branches;
		unsigned char ch;
		uint uch;
		uint addr;
	} u;
};
typedef cacheblock_struct cacheblock_t;

} // End of namespace Glulx
} // End of namespace Glk

#endif
