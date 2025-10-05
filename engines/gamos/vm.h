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
#ifndef GAMOS_VM_H
#define GAMOS_VM_H

#include "common/array.h"
#include "common/hashmap.h"

namespace Gamos {

class VM {
public:
    enum OP{
        OP_EXIT = 0,
        OP_CMP_EQ = 1,
        OP_CMP_NE = 2,
        OP_CMP_LE = 3,
        OP_CMP_LEQ = 4,
        OP_CMP_GR = 5,
        OP_CMP_GREQ = 6,
        OP_CMP_NAE = 7,
        OP_CMP_NA = 8,
        OP_CMP_A = 9,
        OP_CMP_AE = 10,
        OP_BRANCH = 11,
        OP_JMP = 12,
        OP_SP_ADD = 13,
        OP_MOV_EDI_ECX_AL = 14,
        OP_MOV_EBX_ECX_AL = 15,
        OP_MOV_EDI_ECX_EAX = 16,
        OP_MOV_EBX_ECX_EAX = 17,
        OP_RET = 18,
        OP_RETX = 19,
        OP_MOV_EDX_EAX = 20,
        OP_ADD_EAX_EDX = 21,
        OP_MUL = 22,
        OP_OR = 23,
        OP_XOR = 24,
        OP_AND = 25,
        OP_NEG = 26,
        OP_SAR = 27,
        OP_SHL = 28,
        OP_LOAD = 29,
        OP_INC = 30,
        OP_DEC = 31,
        OP_XCHG = 32,
        OP_PUSH_EAX = 33,
        OP_POP_EDX = 34,
        OP_LOAD_OFFSET_EDI = 35,
        OP_LOAD_OFFSET_EDI2 = 36,
        OP_LOAD_OFFSET_EBX = 37,
        OP_LOAD_OFFSET_ESP = 38,
        OP_MOV_PTR_EDX_AL = 39,
        OP_MOV_PTR_EDX_EAX = 40,
        OP_SHL_2 = 41,
        OP_ADD_4 = 42,
        OP_SUB_4 = 43,
        OP_XCHG_ESP = 44,
        OP_NEG_ADD = 45,
        OP_DIV = 46,
        OP_MOV_EAX_BPTR_EDI = 47,
        OP_MOV_EAX_BPTR_EBX = 48,
        OP_MOV_EAX_DPTR_EDI = 49,
        OP_MOV_EAX_DPTR_EBX = 50,
        OP_MOV_EAX_BPTR_EAX = 51,
        OP_MOV_EAX_DPTR_EAX = 52,
        OP_PUSH_ESI_ADD_EDI = 53,
        OP_CALL_FUNC = 54,
        OP_PUSH_ESI_SET_EDX_EDI = 55,

        OP_MAX
    };

    enum MEMREF {
        REF_UNK = 0,
        REF_STACK = 1,
        REF_EBX = 2,
        REF_EDI = 3
    };

    struct Reg {
        uint32 val = 0;
        byte ref = REF_UNK;
    };

    typedef void (* CallDispatcher)(void *object, VM *state, uint32 funcID);

    struct MemoryBlock {
        uint32 address = 0;
        byte data[256];

        MemoryBlock() {
            address = 0;
            memset(data, 0, sizeof(data));
        }
    };

    struct OpLog {
        uint32 addr;
        OP op;
        uint32 sp;
    };

public:
    void clearMemory();
    void writeMemory(uint32 address, const byte* data, uint32 dataSize);
    MemoryBlock *findMemoryBlock(uint32 address);
    uint8 getMemBlockU8(uint32 address);
    uint32 getMemBlockU32(uint32 address);

    MemoryBlock *createBlock(uint32 address);

    void setMemBlockU8(uint32 address, uint8 val);
    void setMemBlockU32(uint32 address, uint32 val);

    Common::Array<byte> readMemBlocks(uint32 address, uint32 count);

    Common::String readMemString(uint32 address, uint32 maxLen = 256);

    Common::String getString(int memtype, uint32 offset, uint32 maxLen = 256);

    uint32 doScript(uint32 scriptAddress);

    int32 getS32(const void *);
    uint32 getU32(const void *);
    void setU32(void *, uint32 val);

    void push32(uint32 val);
    uint32 pop32();

    void pushReg(Reg reg);
    Reg popReg();

    uint32 getMem32(int memtype, uint32 offset);
    uint8 getMem8(int memtype, uint32 offset);

    void setMem32(int memtype, uint32 offset, uint32 val);
    void setMem8(int memtype, uint32 offset, uint8 val);

    Common::String decodeOp(uint32 address, int *size = nullptr);
    Common::String disassembly(uint32 address);

    Common::String opLog(const Common::Array<OpLog> &log);

    void printDisassembly(uint32 address);

public:
    uint32 ESI = 0;
    byte *EBX = nullptr;
    Reg EAX;
    Reg EDX;
    Reg ECX;
    uint32 SP = 0;
    Common::Array<byte> _stack;
    Common::Array<byte> _stackT;

    CallDispatcher _callFuncs = nullptr;
    void *_callingObject = nullptr;

    Common::HashMap<uint32, MemoryBlock> _memMap;

    bool _interrupt = false;
private:
    MemoryBlock *_currentReadMemBlock = nullptr;
    MemoryBlock *_currentWriteMemBlock = nullptr;
};


}

#endif
