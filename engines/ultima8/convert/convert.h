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

#ifndef ULTIMA8_CONVERT_CONVERT_H
#define ULTIMA8_CONVERT_CONVERT_H

#include "ultima8/filesys/idata_source.h"
#include "ultima8/misc/common_types.h"
#include "ultima8/misc/console.h"
#include "ultima8/GenericNodes.h"
#include "ultima8/LoopScriptNodes.h"
#include "ultima8/std/containers.h"
#include "ultima8/std/string.h"

namespace Ultima8 {

class DebugSymbol {
public:
	uint32 number;
	uint32 unknown1;
	uint32 type;
	uint32 unknown2;
	uint32 unknown3;
	std::string name;		
};

class ConvertUsecode {
// just an 'empty' base class
public:
	ConvertUsecode() {};
	virtual ~ConvertUsecode() {};
		
	virtual const char* const *intrinsics()=0;
	virtual const char* const *event_names()=0;
	virtual void readheader(IDataSource *ucfile, UsecodeHeader &uch, uint32 &curOffset)=0;
	virtual void readevents(IDataSource *ucfile, const UsecodeHeader &uch)=0;
	virtual void readOp(TempOp &op, IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)=0;
	virtual Node *readOp(IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols, bool &done)=0;

	void readDbgSymbols(IDataSource *ucfile, std::vector<DebugSymbol> &debugSymbols);
	void readOpGeneric(TempOp &op, IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols,
		bool &done, const bool crusader);
	Node *readOpGeneric(IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols,
		bool &done, const bool crusader);
	void printDbgSymbols(std::vector<DebugSymbol> &debugSymbols);
		
	std::string UsecodeFunctionAddressToString(const sint32 uclass, const sint32 coffset, IDataSource *ucfile, const bool crusader);
	
private:
	static const uint32 MAX_UCFUNC_NAMELEN = 256; // max usecode function name length
};

/* This needs to go into Convert*Crusader only too */
void ConvertUsecode::readDbgSymbols(IDataSource *ucfile, std::vector<DebugSymbol> &debugSymbols) {
	uint32 count=read1(ucfile);

	for(uint32 i=0; i<count; ++i)
	{
		DebugSymbol ds;
		
		ds.number   = i;
		ds.unknown1 = read1(ucfile);
		ds.type     = read1(ucfile);
		ds.unknown2 = read1(ucfile);
		ds.unknown3 = read1(ucfile);
		uint32 tchar;
		while ((tchar = read1(ucfile)))
			ds.name += static_cast<char>(tchar);
		
		debugSymbols.push_back(ds);
	}
}

void ConvertUsecode::printDbgSymbols(std::vector<DebugSymbol> &debugSymbols)
{
	for(uint32 i=0; i<debugSymbols.size(); ++i)
	{
		DebugSymbol &ds = debugSymbols[i];
		
		con.Printf("%02X: %02X type=%02X (%c) %s (%02X) %02X %s\n",
			ds.number, ds.unknown1, ds.type, ds.type, print_bp(ds.unknown2), ds.unknown2, ds.unknown3, ds.name.c_str());
	}
};

void printbytes(IDataSource *f, uint32 num)
{
	//uint32 loff=0;
	while(num>0)
	{
		uint8 c = f->read1();
		con.Printf("%02X %c ", c, std::isprint(c) ? c : '.');
		--num;
	}
}

/* This needs to be shuffled into two different readOp() functions, one in Convert*Crusader, and
	the other in Convert*U8 */
void ConvertUsecode::readOpGeneric(TempOp &op, IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols,
	bool &done, const bool crusader)
{
		if(dbg_symbol_offset==curOffset)
		{
			readDbgSymbols(ucfile, debugSymbols);
			op.op(read1(ucfile));
			assert(op.op()==0x7a);
		}
		else
			op.op(read1(ucfile));

		op.offset = curOffset-1;

		/*
		  Guesses of opcodes. I'm reasonably sure about most of them,
		  but they're still guesses...
		  (Questionmarks generally indicate uncertainty)
		*/

		switch(op.op()) {

		// Poping to variables
		case 0x00:
			// 00 xx
			// pop 8 bit int into bp+xx
			op.i0 = read1(ucfile);
			break;
		case 0x01:
			// 01 xx
			// pop 16 bit int into bp+xx
			op.i0 = read1(ucfile);
			break;
		case 0x02:
			// 02 xx
			// pop 32 bit int into bp+xx
			op.i0 = read1(ucfile);
			break;
		case 0x03:
			// 03 xx yy
			// pop yy bytes into bp+xx
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;

		case 0x08:
			// 08
			// pop 32bits into result register
			break;
		case 0x09:
			// 09 xx yy zz
    			// pop yy bytes into an element of list bp+xx (or slist if zz is set).
			op.i0 = read1(ucfile); op.i1 = read1(ucfile); op.i2 = read1(ucfile);
			break;

		// Constant pushing
		case 0x0A:
			// 0A xx
			// push signed extended 8 bit xx onto the stack
			op.i0 = read1(ucfile);
			break;
		case 0x0B:
			// 0B xx xx
			// push 16 bit xxxx onto the stack
			op.i0 = read2(ucfile);
			break;
		case 0x0C:
			// 0C xx xx xx xx
			// push 32 bit xxxxxxxx onto the stack
			op.i0 = read4(ucfile);
			break;
		case 0x0D:
			// 0D xx xx yy ... yy 00
			// push string (yy ... yy) of length xx xx onto the stack
			op.i0 = read2(ucfile);
			op.str = "";
			while ((op.i1 = read1(ucfile))) op.str += static_cast<char>(op.i1);
			break;
		case 0x0E:
			// 0E xx yy
			// pop yy values of size xx from the stack and push the resulting list
			op.i0 = read1(ucfile);
			op.i1 = read1(ucfile);
			break;

		// Usecode function and intrinsic calls
		case 0x0F:
			// 0F xx yyyy
			// intrinsic call. xx is number of arguement bytes (includes this pointer)
			op.i0 = read1(ucfile);
			op.i1 = read2(ucfile);
			break;
		case 0x11:
			// 11 xx xx yy yy
			// call the function at offset yy yy of class xx xx
			op.i0 = read2(ucfile);
			op.i1 = read2(ucfile);
			break;
		case 0x12:
			// 12
			// pop 16bits into temp register
			break;

		case 0x14:
			// 14
			// pop two values from the stack and push the sum
			break;
		case 0x15:
			// 15
			// add two longs
			break;
		case 0x16:
			// 16
			// pop two strings from the stack and push the concatenation
			break;
		case 0x17:
			// 17
			// pop two lists from the stack and push the 'sum' of the lists
			break;
		case 0x19:
			// 19 02
			// add two stringlists
			op.i0 = read1(ucfile);
			break;
		case 0x1A:
			// 1A
			// pop two string lists from the stack and remove the 2nd from the 1st
			op.i0 = read1(ucfile);
			break;
		case 0x1B:
			// 1B
			// pop two lists from the stack and remove the 2nd from the 1st
			op.i0 = read1(ucfile);
			break;
		case 0x1C:
			// 1C
			// subtract two integers
			break;
		case 0x1D:
			// 1D
			// subtract two dwords
			break;
		case 0x1E:
			// 1E
			// multiply two integers
			break;
		case 0x1F:
			// 1F
			// multiply two dwords
			break;
		case 0x20:
			// 20
			// divide two integers
			break;
		case 0x21:
			// 21
			// divide two dwords
			break;
		case 0x22:
			// 22
			// mod
			break;
		case 0x23:
			// 23
			// mod long
			assert(false); // Guessed opcode
			break;
		case 0x24:
			// 24
			// compare two integers
			break;
		case 0x25:
			// 24
			// compare two dwords
			assert(false); // Guessed opcode
			break;
		case 0x26:
			// 26
			// compare two strings
			break;
		case 0x28:
			// 28
			// less than
			break;
		case 0x29:
			// 29
			// less than 32 bit
			break;
		case 0x2A:
			// 2A
			// less than or equal to
			break;
		case 0x2B:
			// 2B
			// less than or equal to 32 bit
			break;
		case 0x2C:
			// 2C
			// greater than
			break;
		case 0x2D:
			// 2D
			// greater than 32 bit
			break;
		case 0x2E:
			// 2E
			// 'greater than or equal to'
			break;
		case 0x2F:
			// 2F
			// 'greater than or equal to' (longs)
			break;
		case 0x30:
			// 30
			// pops a boolean from the stack and pushes the boolean not
			break;
		case 0x31:
			// 31
			// pops a boolean from the stack and pushes the boolean not
			break;
		case 0x32:
			// 32
			// pops two booleans from the stack and pushes the boolean and
			break;
		case 0x33:
			// 33
			// pops two booleans from the stack and pushes the boolean and
			break;
		case 0x34:
			// 34
			// boolean or
			break;
		case 0x35:
			// 35
			// boolean or
			break;
		case 0x36:
			// 36
			// are two integers not equal?
			break;
		case 0x37:
			// 37
			// are two dwords not equal?
			break;

		case 0x38:
			// 38 xx yy
			// pops a list (or slist if yy==true) from the stack, then pops
			// a value from the stack that it needs to test if it's in the
			// list, pushing 'true' if it is, 'false' if it isn't. 'xx' is
			// the 'size' of each list element, as is true for most list
			// opcodes.
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;

		case 0x39:
			// 39
			// bitwise and
			break;
		case 0x3A:
			// 3A
			// bitwise or
			break;
		case 0x3B:
			// 3B
			// bitwise not
			break;
		case 0x3C:
			// 3C
    		// left shift
			break;
		case 0x3D:
			// 3D
			// right shift
			break;

		case 0x3E:
			// 3E xx
			// push the value of the 8 bit local var xx ??
			op.i0 = read1(ucfile);
			break;
		case 0x3F:
			// 3F xx
			// push the value of the 16 bit local var xx
			op.i0 = read1(ucfile);
			break;
		case 0x40:
			// 40 xx
			// push the value of the 32 bit local var xx ??
			op.i0 = read1(ucfile);
			break;
		case 0x41:
			// 41 xx
			// push the string local var at BP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x42:
			// 42 xx yy
			// push the list (with yy size elements) at BP+xx
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;
		case 0x43:
			// 43 xx
			// push the stringlist local var at BP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x44:
			// 44 xx yy
			// push element from the second last var pushed onto the stack
			// (a list/slist), indexed by the last element pushed onto the list
			// (a byte/word). XX is the size of the types contained in the list
			// YY is true if it's a slist (for garbage collection)
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;
		case 0x45:
			// 45
			// push huge
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;
		case 0x4B:
			// 4B xx
			// push 32 pointer address of BP+XX
			op.i0 = read1(ucfile);
			break;
		case 0x4C:
			// 4C xx
			// indirect push,
			// pops a 32 bit pointer off the stack and pushes xx bytes
			// from the location referenced by the pointer
			op.i0 = read1(ucfile);
			break;
		case 0x4D:
			// 4D xx
			// indirect pop,
			// pops a 32 bit pointer off the stack and then pops xx bytes
			// into the location referenced by the pointer
			op.i0 = read1(ucfile);
			break;

		case 0x4E:
			// 4E xx xx yy
			// push global xx xx size yy
			op.i0 = read2(ucfile); op.i1 = read1(ucfile);
			break;
		case 0x4F:
			// 4F xx xx yy
			// pop value into global xx xx size yy
			op.i0 = read2(ucfile); op.i1 = read1(ucfile);
			break;

		case 0x50:
			// 50
			// return from function
			break;
		case 0x51:
			// 51 xx xx
			// relative jump to xxxx if false
			op.i0 = read2(ucfile);
			break;
		case 0x52:
			// 52 xx xx
			// relative jump to xxxx
			op.i0 = read2(ucfile);
			break;

		case 0x53:
			// 50
			// suspend function
			break;

		case 0x54:
			// 54 xx yy
			// implies
			// this seems to link two processes, 'implying' that if one terminates,
			// the other does also, but it's mostly a guess. *grin*
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;

		case 0x57:
			// 57 aa tt xx xx yy yy
			// spawn process function yyyy in class xxxx
			// aa = number of arg bytes pushed
			//      (not including this pointer which is 4 bytes)
			// tt = sizeof this pointer object
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			op.i2 = read2(ucfile); op.i3 = read2(ucfile);
			break;
		case 0x58:
			// 58 xx xx yy yy zz zz tt uu
			// spawn inline process function yyyy in class xxxx at offset zzzz
			// tt = size of this pointer
			// uu = unknown
			op.i0 = read2(ucfile); op.i1 = read2(ucfile);
			op.i2 = read2(ucfile);
			op.i3 = read1(ucfile); op.i4 = read1(ucfile);
			break;
		case 0x59:
			// 59
			// push process id of current process
			break;

		case 0x5A:
			// 5A xx
			// init function. xx = local var size
			// sets xx bytes on stack to 0, and moves sp by xx
			op.i0 = read1(ucfile);
			break;

		case 0x5B:
			// 5B xx xx
			// the current sourcecode line number
			op.i0 = read2(ucfile);
			break;
		case 0x5C:
			{
				// 5C xx xx yy yy yy yy yy yy yy yy 00
				// debugging symbol information
				// xxxx is the offset to one past the last 'ret' in the function, which
				// will be pointing to an 0x7a opcode if there is no debug info, else
				// to the first byte of the debug info.
				// yy .. yy is the class' name
				op.i0 = read2(ucfile);
				//assert(curOffset + (static_cast<short>(op.i0))==op.offset + 3 + (static_cast<short>(op.i0)));
				op.i0 = curOffset + (static_cast<short>(op.i0));
				op.str = "";
				for (uint32 i=0; i < 8; ++i)
				 op.str += static_cast<char>(read1(ucfile));
				if(read1(ucfile)!=0) assert(false); // trailing 0
				dbg_symbol_offset = op.i0; // the offset to the raw symbol data.
					// nothing between it and the 0x7a opcode is opcodes
				break;
			}

		case 0x5D:
			// 5D
			// push 8 bit value returned from function call
			break;
		case 0x5E:
			// 5E
			// push 16 bit value returned from function call
			break;
		case 0x5F:
			// 5F
			// push 32 bit value returned from function call?
			break;

		case 0x60:
			// 60
			// word to dword (sign extend)
			break;
		case 0x61:
			// 61
			// dword to word
			break;

		case 0x62:
			// 62 xx
			// free the string in var BP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x63:
			// 63 xx
			// free the list in var BP+xx
			// (This one seems to be similar to 0x64 but only used for lists
			//  of strings?)
			op.i0 = read1(ucfile);
			break;
		case 0x64:
			// 64 xx
			// free the list in var BP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x65:
			// 65 xx
			// free string at SP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x66:
			// 66 xx
			// free the list at SP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x67:
			// 66 xx
			// free the string list at SP+xx
			op.i0 = read1(ucfile);
			break;
		case 0x69:
			// 69 xx
			// push the string in var BP+xx as 32 bit pointer
			op.i0 = read1(ucfile);
			break;
		case 0x6B:
			// 6B
			// pop a string and push 32 bit pointer to string
			break;

// Just my ramblings on 0x6C. It's a bit of a mess since it's taken from an irc log
// but it's the best documentation we have on it at the moment. *grin*

/* Anyway, the number at the end (01 in the above example) means the type of
pointer the offset points to. 01==string ptr, 02==string list ptr, 03==list
ptr. 01 and 03 are verified, 02 is an educated guess, and I'm pretty sure it's
a BP+xxh value too from looking at the usecode.
 *lightbulb* It looks like it may have been used for pointer/reference
stuff. For example:
 When a 'string' type is created, the actual string is stored in a Yamm
class instantiation local to the current thread, only the reference to it is
stored on the stack.
 When a pointer to this string is passed to another newly created thread,
the data pointed to in the original thread's Yamm needs to be copied to the
newly constructed thread, incase the original thread terminates, (or the
function just returns destroying the data), before the spawned thread uses it.
 This is where Mr 0x6C comes in. He takes a pointer to the
oritinal variable, copies the data from the Parent Yamm to it's (the Child's)
Yamm.
 Eg: The call in Class 007C:
     371F: 59    push            pid
     3720: 42    push list       [BP-0Ah] (02)
     3723: 42    push list       [BP-0Ch] (02)
     3726: 42    push list       [BP-0Eh] (02)
     3729: 41    push string     [BP-02h]
     372B: 40    push dword      [BP+06h]
     372D: 57    spawn           08 02 007C:3757 (unknown)
     3734: 66    free list       [SP+06h]
     3736: 66    free list       [SP+04h]
     3738: 66    free list       [SP+02h]
     373A: 65    free string     [SP+00h]
     373C: 6E    add sp          -08h
     373E: 5E    push            retval
 Then the start of the newly spawned function:
 Func_3757:
     3757: 5A    init            0D
     3759: 6C    6C              [BP+0Ah] 01
     375C: 6C    6C              [BP+0Ch] 03
     375F: 6C    6C              [BP+0Eh] 03
     3762: 6C    6C              [BP+10h] 03
     3765: 0A    push byte       01h
 The type value (01/02/03) at the end tells the opcode what type of data
it's copying or reconstructing. And if you look closely, you'll note there's a
'tiny' flaw in this opcode. In all the other list (not string list) related
opcodes another value is passed, the size of the datatype stored in the array.
This opcode lacks that value, and thus assumes that all datatypes for 03
(list) are 2 bytes wide.
 Of course, this also tells us something about how the processes were
executed and/or queued. Either:
 1) When a new thread is created by a spawn, it is immediately executed
until it hits the first opcode that 'sleeps' it until the next cycle, and that
the 'init' and 0x6C opcodes are not 'sleep' inducing opcodes.
 Or 2) The 'spawn' is a 'sleep' inducing opcode, and the new process is
scheduled to be executed before the 'current' thread in the queue (whether it
just be inserted before, or first, who knows *grin*). That way upon the next
awakening of all the threads, the spawned thread gets the chance to copy all
the relevant data from it's parent thread, before it's parent has a chance to
terminate.
 Also in the second case, as in the first case, the 'init' and
'0x6C' opcodes aren't 'sleep' inducing.

The general consensus is that the new threads are executed instantly upon creation,
and that the child threads are indeed placed infront of the parent thread.
*/
		case 0x6C:
			// 6C xx yy
			// looks to be a BP+XX function... maybe not
			op.i0 = read1(ucfile); op.i1 = read1(ucfile);
			break;

		case 0x6D:
			// 6D
			// push result of process
			break;
		case 0x6E:
			// 6E xx
			// add xx to stack pointer
			op.i0 = read1(ucfile);
			break;
		case 0x6F:
			// 6F xx
			// push 32 pointer address of SP-xx
			op.i0 = read1(ucfile);
			break;

		// loop-related opcodes
		// Theory: put a 'container object' on the stack, and this will
		// loop over the objects in there. The 'loopscript' determines
		// which objects are selected. (By a simple 'shape == x, frame == y'
		// thing, it seems)
		// See the abacus code (function 375) for a simple example

		case 0x70:
			// 70 xx yy zz
			// loop something. Stores 'current object' in var xx
			// yy == num bytes in string
			// zz == type
			op.i0 = read1(ucfile); op.i1 = read1(ucfile); op.i2 = read1(ucfile);
			break;
		case 0x73:
			// 73
			// next loop object? pushes false if end reached
			break;
		case 0x74:
			// 74 xx
			// add xx to the current 'loopscript'
			op.i0 = read1(ucfile);
			break;

		// 75 appears to have something to do with lists, looks like an enum/next from u7...
		case 0x75:
			// 75 xx yy zz zz
			// xx appears to be the location to store 'current' value from the
			//   list (BP+xx)
			// yy is the 'datasize' of the list, identical to the second parameter
			//   of the create list/slist opcodes
			// zzzz appears to be the offset to jump to after it's finished the
			//   iteration, the opcode before is a 'jmp' to the original position
			//   of the opcode.
			// (all guesses from Remorse1.21 usecode, _may_ be different in u8,
			//   unlikely though)
			// the way it appears to operate is it pops a 'word' off the stack
			//   (maximum number of items to iterate through? No idea, I've only
			//   seen 0xFFFF pushed before it (in Remorse1.21)), then pops
			//   the 'list' off to iterate through
			op.i0 = read1(ucfile); op.i1 = read1(ucfile); op.i2 = read2(ucfile);
			break;

		/* 76 appears to be identical to 0x75, except it operates on slists */
		case 0x76:
			// 75 xx yy zz zz
			op.i0 = read1(ucfile); op.i1 = read1(ucfile); op.i2 = read2(ucfile);
			break;

		case 0x77:
			// 77
			// set info
			// assigns item number and ProcessType, both values popped from the stack
			break;

		case 0x78:
			// 78
			// process exclude
			// sets a flag on the object target of the 'current' function call that
			// said function call has exclusive use of the object until the call
			// returns.
			break;

/*
<Darke> Add global_address to the list of Things That Are Evil About U8Usecode.
<Colourless> why? what's *it's* problem?
<Darke> It makes no sense. *grin*
<Colourless> hehe
<Darke> None! None what so ever!
<Darke> AFAICT, it's only use it to push the address of the global variable to the stack.
	Said global variable appears to hold a 'this' pointer to something.
<Darke> There is, afaict, no difference between doing a 'global_address 003C' and doing a
	`push global [003C 04]; push addr [SP-00h];`
<Darke> Ok, fine, it _is_ used 667 times in remorse1.21, but I can't see how there could be
	an incredible speedup replacing two simple opcodes with a single opcode.
<Colourless> so that's what it does
<Darke> And it's always followed by either a calli or a spawn. So that means that it's
	'always' a 4 byte 'this' pointer stored in it.
<Darke> Same in no regret.
<Darke> So it's 'obviously' _never_ modified (which was the only possible problem with the
	push/push addr I could see, you'd need to have an appropriate 'un-addr' then pop after
	it)... I really can't see the point. *shrug*
*/
		case 0x79:
			// 79
			// end of function
			// 79 xx xx
			// global_address
			// push the address of the global xxxx onto the stack
			if (crusader) // it's not eof for crusader...
			{
				op.i0 = read2(ucfile);
			}
			else // but it is for U8
			{
				op.op(0x7A); // we'll translate it to the crusader's end to simplify things
				done = true;
			}
			break;

			case 0x7A:
			// end of function (79 = u8, 7a = crusader)
			printDbgSymbols(debugSymbols);
			done = true;
			break;

		// can't happen.
		default:
			con.Printf("db\t\t%02X", op.op());
			assert(false);
		}

		op.nextoffset=curOffset;
}

#include "FuncNodes.h"
#include "VarNodes.h"
#include "CallNodes.h"
#include "OperatorNodes.h"
#include "IfNode.h"

/* This needs to be shuffled into two different readOp() functions, one in Convert*Crusader, and
	the other in Convert*U8 */
Node *ConvertUsecode::readOpGeneric(IDataSource *ucfile, uint32 &dbg_symbol_offset, std::vector<DebugSymbol> &debugSymbols,
	bool &done, const bool crusader)
{
	Node *n=0;
	uint32 opcode=0;
	static std::string tstring;
	
	if(dbg_symbol_offset==curOffset)
	{
		readDbgSymbols(ucfile, debugSymbols);
		opcode = read1(ucfile);
		assert(opcode==0x7a);
	}
	else
		opcode = read1(ucfile);
	
	uint32 offset = curOffset-1;
	
	switch(opcode) {
		case 0x01: // pop a word into a local var
			n = new PopVarNode(opcode, offset, read1(ucfile));
			break;
		case 0x0A: // pushing a byte (1 byte)
			n = new PushVarNode(opcode, offset, read1(ucfile));
			break;
		case 0x0B: // pushing a word (2 bytes)
			n = new PushVarNode(opcode, offset, read2(ucfile));
			break;
		case 0x0C: // pushing a dword (4 bytes)
			n = new PushVarNode(opcode, offset, read4(ucfile));
			break;
		case 0x0D: // push string (xxxx bytes)
			{
				uint32 tint = read2(ucfile);
				tstring.clear();
				while (char c = static_cast<char>(read1(ucfile))) tstring += c;
				n = new PushVarNode(opcode, offset, tint, tstring);
			}
			break;
		case 0x0F: // calli
			{
				uint32 tint=read1(ucfile);
				n = new DCCallNode(opcode, offset, tint, read2(ucfile));
			}
			break;
		case 0x11: // call
			{
				uint32 tint=read2(ucfile);
				n = new DCCallNode(opcode, offset, tint, read2(ucfile));
			}
			break;
		case 0x12: // pop temp
			n = new PopVarNode(opcode, offset);
			break;
		case 0x14: // add
		case 0x1C: // sub
		case 0x1E: // mul
		case 0x24: // cmp
		case 0x28: // lt
		case 0x2A: // le
		case 0x2C: // gt
			n = new BinOperatorNode(opcode, offset);
			break;
		case 0x30: // not
			n = new UniOperatorNode(opcode, offset);
			break;
		case 0x32: // and
			n = new BinOperatorNode(opcode, offset);
			break;
		case 0x34: // or
			n = new BinOperatorNode(opcode, offset);
			break;
		case 0x36: // ne
			n = new BinOperatorNode(opcode, offset);
			break;
		case 0x3F: // pushing a word var (2 bytes)
			n = new PushVarNode(opcode, offset, read1(ucfile));
			break;
		case 0x40: // pushing a dword var (4 bytes)
			n = new PushVarNode(opcode, offset, read1(ucfile));
			break;
		case 0x4B: // pushing an address (4 bytes)
			n = new PushVarNode(opcode, offset, read1(ucfile));
			break;
		case 0x4C: // push indirect
			n = new DCCallMutatorNode(opcode, offset, read1(ucfile));
			break;
		case 0x4E: // push global
			{
				uint32 tint = read2(ucfile);
				n = new PushVarNode(opcode, offset, tint, read1(ucfile));
			}
			break;
		case 0x50: // ret
			n = new FuncMutatorNode(opcode, offset);
			break;
		case 0x51: // jne
			n = new IfNode(opcode, offset, offset + 3 + static_cast<uint16>(read2(ucfile)));
			break;
		case 0x52: // jmp
			n = new EndNode(opcode, offset, offset + 3 + static_cast<uint16>(read2(ucfile)));
			break;
		case 0x53: // suspend
			n = new FuncMutatorNode(opcode, offset);
			break;
		case 0x54: // implies
			{ // we'll read and ignore the two parameters to implies, since they're always 01 01
				uint32 tint1 = read1(ucfile);
				uint32 tint2 = read1(ucfile);
				assert(tint1==1 && tint2==1);
				n = new BinOperatorNode(opcode, offset);
			}
			break;
		case 0x57: // spawn
			{
				uint32 tint1 = read1(ucfile);
				uint32 tint2 = read1(ucfile);
				uint32 tint3 = read2(ucfile);
				n = new DCCallNode(opcode, offset, tint1, tint2, tint3, read2(ucfile));
			}
			break;
		case 0x59: // push pid
			n = new PushVarNode(opcode, offset);
			break;
		case 0x5A: // init
			n = new FuncMutatorNode(opcode, offset, read1(ucfile));
			break;
		case 0x5B: // line number
			n = new FuncMutatorNode(opcode, offset, read2(ucfile));
			break;
		case 0x5C: // symbol info
			{
				uint32 tint = offset + 3 + static_cast<sint16>(read2(ucfile));
				dbg_symbol_offset = tint;
				n = new FuncMutatorNode(opcode, offset, tint, readnstr(ucfile, 9));
			}
			break;
		case 0x5D: // push byte retval
		case 0x5E: // push retval
			n = new DCCallPostfixNode(opcode, offset);
			break;
		case 0x65: // free string
			n = new DCCallPostfixNode(opcode, offset, read1(ucfile));
			break;
		case 0x6B: // str to ptr
			n = new UniOperatorNode(opcode, offset);
			break;
		case 0x6E: // add sp
			n = new DCCallPostfixNode(opcode, offset, read1(ucfile));
			break;
		case 0x70: // loop
			{
				uint32 currobj = read1(ucfile);
				uint32 strsize = read1(ucfile);
				uint32 type = read1(ucfile);
				n = new LoopNode(opcode, offset, currobj, strsize, type);
			}
			break;
		//case 0x73: // loopnext
		//	n = new LoopNextNode(opcode, offset);
		//	break;
		case 0x74: // loopscr
			n = new LoopScriptNode(opcode, offset, read1(ucfile));
			break;
		case 0x77: // set info
			n = new DCCallMutatorNode(opcode, offset);
			break;
		case 0x78: // process exclude
			n = new DCCallMutatorNode(opcode, offset);
			break;
		case 0x79: // end of function / global_address
			if (crusader) // it's not eof for crusader...
			{
				assert(false); //TODO: Implement!: op.i0 = read2(ucfile);
			}
			else // but it is for U8
			{
				n = new FuncMutatorNode(0x7A, offset); // we'll translate it to the crusader's end to simplify things
				done = true;
			}
			break;
		case 0x7A: // end
			n = new FuncMutatorNode(opcode, offset);
			done = true;
			break;
		
		// can't happen.
		default:
			con.Printf("\n*** ERROR?:");
			con.Printf("\n*** potential opcode:\t%02X", opcode);
			con.Printf("\n*** next four bytes:\t");
			printbytes(ucfile, 4);
			con.Putchar('\n');
			return 0;
	}
	
	return n;
}
/* This looks _real_ dubious. Instead of loading all the offsets from the
   files and converting them to pair<uint32, uint32>, we're storing them in
   memory as strings, then having to convert the class:offset pair into a
   string, and strcmping against them. So instead of having a 2*O(N) operation
   at read, and a 2*O(1)*O(logN) at search. We've got a O(N) operation at read,
   and a O(N)*O(logN) for _each_ search. */
std::string ConvertUsecode::UsecodeFunctionAddressToString(const sint32 uclass, const sint32 coffset, IDataSource *ucfile, const bool crusader)
{
	char buf[MAX_UCFUNC_NAMELEN];
	
	#if 0 // FuncName stuff doesn't seem to be used anymore...
	std::map<string, string>::iterator funcoffset = FuncNames.find(buf);

	snprintf(buf, MAX_UCFUNC_NAMELEN, "%04X:%04X", uclass, coffset);
	funcoffset = FuncNames.find(buf);
	if (funcoffset != FuncNames.end())
		return funcoffset->second;
	#endif
	
	// Attempt to grab function name of the requested func

	// Save the original pos
	uint32 origpos = ucfile->getPos();

	// Seek to index table entry 1
	ucfile->seek(0x80 + 8);

	// Get details
	sint32 offset = ucfile->read4();
	/*sint32 length =*/ ucfile->read4();

	// Seek to name entry
	ucfile->seek(offset + uclass*13 + 4);

	// Read name
	buf[0]='\0';
	ucfile->read(buf, 9);

	// Return to the pos
	ucfile->seek(origpos);

	// Couldn't get name, just use number
	if (!buf[0]) snprintf(buf, MAX_UCFUNC_NAMELEN, "%04X", uclass);

	// String to return
	std::string str = buf;

	// This will only work in crusader
	if (crusader)
	{
		if (coffset < 0x20)
		{
			return str + "::" + event_names()[coffset];
		}
		else
		{
			snprintf(buf, MAX_UCFUNC_NAMELEN, "%02X", coffset);
			return str + "::ordinal" + buf;
		}
	}
	// For Ultima 8
	else
	{
		snprintf(buf, MAX_UCFUNC_NAMELEN, "%04X", coffset);
		return str + "::" + buf;
	}
	
	CANT_HAPPEN();
	// Shouldn't ever get here
	return "unknown";
}

} // End of namespace Ultima8

#endif
