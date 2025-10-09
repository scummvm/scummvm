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
#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#include "gamos/gamos.h"

namespace Gamos {

VM::CallDispatcher VM::_callFuncs = nullptr;
void *VM::_callingObject = nullptr;

Common::HashMap<uint32, VM::MemoryBlock> VM::_memMap;
bool VM::_interrupt = false;

VM VM::_threads[THREADS_COUNT];

VM::MemAccess VM::_memAccess;





uint8 VM::MemAccess::getU8(uint32 address) {
    if (!_currentBlock || address < _currentBlock->address || address >= (_currentBlock->address + 0x100))
        _currentBlock = findMemoryBlock(address);

    if (!_currentBlock)
        return 0; // ERROR!

    return _currentBlock->data[ address - _currentBlock->address ];
}

uint32 VM::MemAccess::getU32(uint32 address) {
    if (!_currentBlock || address < _currentBlock->address || address >= (_currentBlock->address + 0x100))
        _currentBlock = findMemoryBlock(address);

    if (!_currentBlock)
        return 0; // ERROR!

    uint32 pos = address - _currentBlock->address;
    if ((int32)0x100 - (int32)pos >= 4)
        return VM::getU32(_currentBlock->data + pos); //easy

    MemoryBlock *block = _currentBlock;
    uint32 val = block->data[ pos ];
    for (int i = 1; i < 4; i++) {
        pos++;
        if (pos >= 0x100) {
            block = findMemoryBlock(address + i);
            if (!block)
                break;
            pos = 0;
        }
        val |= block->data[ pos ] << (i * 8);
    }

    return val;
}

void VM::MemAccess::setU8(uint32 address, uint8 val) {
    if (!_currentBlock || address < _currentBlock->address || address >= (_currentBlock->address + 0x100))
        _currentBlock = findMemoryBlock(address);

    if (!_currentBlock)
        _currentBlock = createBlock(address);

    _currentBlock->data[ address - _currentBlock->address ] = val;
}

void VM::MemAccess::setU32(uint32 address, uint32 val) {
    if (!_currentBlock || address < _currentBlock->address || address >= (_currentBlock->address + 0x100))
        _currentBlock = findMemoryBlock(address);

    if (!_currentBlock)
        _currentBlock = createBlock(address);

    uint32 pos = address - _currentBlock->address;

    if (address + 4 <= _currentBlock->address + 0x100) {
        VM::setU32(_currentBlock->data + pos, val);
        return;
    }

    MemoryBlock *block = _currentBlock;

    _currentBlock->data[ pos ] = val & 0xff;

    for (int i = 1; i < 4; i++) {
        pos++;
        if (pos >= 0x100) {
            block = createBlock(address + i);
            if (!block)
                break;
            pos = 0;
        }
        block->data[ pos ] = (val >> (i * 8)) & 0xff;
    }
}





uint32 VM::execute(uint32 scriptAddress, byte *storage) {
    //Common::String disasm = disassembly(scriptAddress);

    ESI = scriptAddress;
    EBX = storage;

    _readAccess.reset();
    _writeAccess.reset();

    SP = STACK_POS;

    Common::Array<OpLog> cmdlog;

    bool loop = true;
    while (loop) {
        if (_interrupt)
            return 0;

        byte op = _readAccess.getU8(ESI);
        cmdlog.push_back({ESI, (OP)op, SP});
        ESI++;

        switch (op) {
        default:
        case OP_EXIT:
            loop = false;
            break;

        case OP_CMP_EQ:
            if (EDX.val == EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_NE:
            if (EDX.val != EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_LE:
            if ((int32)EDX.val < (int32)EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_LEQ:
            if ((int32)EDX.val <= (int32)EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_GR:
            if ((int32)EDX.val > (int32)EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_GREQ:
            if ((int32)EDX.val >= (int32)EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_NAE:
            if (EDX.val < EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_NA:
            if (EDX.val <= EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_A:
            if (EDX.val > EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_CMP_AE:
            if (EDX.val >= EAX.val)
                EAX.val = 1;
            else
                EAX.val = 0;
            break;

        case OP_BRANCH:
            if (EAX.val != 0)
                ESI += 4;
            else
                ESI += (int32)_readAccess.getU32(ESI);
            break;

        case OP_JMP:
            ESI += (int32)_readAccess.getU32(ESI);
            break;

        case OP_SP_ADD:
            SP += (int32)_readAccess.getU32(ESI);
            ESI += 4;
            break;

        case OP_MOV_EDI_ECX_AL:
            ECX.val = _readAccess.getU32(ESI);
            setMem8(REF_EDI, ECX.val, EAX.val & 0xff);
            ESI += 4;
            break;

        case OP_MOV_EBX_ECX_AL:
            ECX.val = _readAccess.getU32(ESI);
            setMem8(REF_EBX, ECX.val, EAX.val & 0xff);
            ESI += 4;
            break;

        case OP_MOV_EDI_ECX_EAX:
            ECX.val = _readAccess.getU32(ESI);
            setMem32(REF_EDI, ECX.val, EAX.val);
            ESI += 4;
            break;

        case OP_MOV_EBX_ECX_EAX:
            ECX.val = _readAccess.getU32(ESI);
            setMem32(REF_EBX, ECX.val, EAX.val);
            ESI += 4;
            break;

        case OP_RET:
            ESI = pop32();
            ESI += 4;
            break;

        case OP_RETX:
            ECX = popReg();
            SP += _readAccess.getU32(ESI);
            ESI = ECX.val;
            ESI += 4;
            break;

        case OP_MOV_EDX_EAX:
            EDX = EAX;
            break;

        case OP_ADD_EAX_EDX:
            EAX.val += EDX.val;
            if (EAX.ref == REF_UNK && EDX.ref != REF_UNK)
                EAX.ref = EDX.ref;
            break;

        case OP_MUL:
            EAX.val *= EDX.val;
            break;

        case OP_OR:
            EAX.val |= EDX.val;
            break;

        case OP_XOR:
            EAX.val ^= EDX.val;
            break;

        case OP_AND:
            EAX.val &= EDX.val;
            break;

        case OP_NEG:
            EAX.val = (uint32)(-((int32)EAX.val));
            break;

        case OP_SAR:
            EAX.val = (uint32)(((int32)EDX.val) >> (EAX.val & 0xff)); /* must be arythmetic shift! */
            break;

        case OP_SHL:
            EAX.val = EDX.val << (EAX.val & 0xff);
            break;

        case OP_LOAD:
            EAX.val = _readAccess.getU32(ESI);
            EAX.ref = REF_UNK;
            ESI += 4;
            break;

        case OP_INC:
            EAX.val += 1;
            break;

        case OP_DEC:
            EAX.val -= 1;
            break;

        case OP_XCHG:
            ECX = EAX;
            EAX = EDX;
            EDX = ECX;
            break;

        case OP_PUSH_EAX:
            pushReg(EAX);
            break;

        case OP_POP_EDX:
            EDX = popReg();
            break;

        case OP_LOAD_OFFSET_EDI:
        case OP_LOAD_OFFSET_EDI2:
            EAX.val = _readAccess.getU32(ESI);
            EAX.ref = REF_EDI;
            ESI += 4;
            break;

        case OP_LOAD_OFFSET_EBX:
            EAX.val = _readAccess.getU32(ESI);
            EAX.ref = REF_EBX;
            ESI += 4;
            break;

        case OP_LOAD_OFFSET_ESP:
            EAX.val = _readAccess.getU32(ESI) + SP;
            EAX.ref = REF_STACK;
            ESI += 4;
            break;

        case OP_MOV_PTR_EDX_AL:
            setMem8(EDX.ref, EDX.val, EAX.val & 0xff);
            break;

        case OP_MOV_PTR_EDX_EAX:
            setMem32(EDX.ref, EDX.val, EAX.val);
            break;

        case OP_SHL_2:
            EAX.val <<= 2;
            break;

        case OP_ADD_4:
            EAX.val += 4;
            break;

        case OP_SUB_4:
            EAX.val -= 4;
            break;

        case OP_XCHG_ESP:
            ECX = popReg();
            pushReg(EAX);
            EAX = ECX;
            break;

        case OP_NEG_ADD:
            EAX.val = (uint32)(-((int32)EAX.val)) + EDX.val;
            break;

        case OP_DIV:
            ECX = EAX;
            EAX.val = (int32)EDX.val / (int32)ECX.val;
            EDX.val = (int32)EDX.val % (int32)ECX.val;
            break;

        case OP_MOV_EAX_BPTR_EDI:
            ECX.val = _readAccess.getU32(ESI);
            EAX.val = (int8)getMem8(REF_EDI, ECX.val);
            ESI += 4;
            break;

        case OP_MOV_EAX_BPTR_EBX:
            ECX.val = _readAccess.getU32(ESI);
            EAX.val = (int8)getMem8(REF_EBX, ECX.val);
            ESI += 4;
            break;

        case OP_MOV_EAX_DPTR_EDI:
            ECX.val = _readAccess.getU32(ESI);
            EAX.val = getMem32(REF_EDI, ECX.val);
            ESI += 4;
            break;

        case OP_MOV_EAX_DPTR_EBX:
            ECX.val = _readAccess.getU32(ESI);
            EAX.val = getMem32(REF_EBX, ECX.val);
            ESI += 4;
            break;

        case OP_MOV_EAX_BPTR_EAX:
            EAX.val = (int8)getMem8(EAX.ref, EAX.val);
            EAX.ref = REF_UNK;
            break;

        case OP_MOV_EAX_DPTR_EAX:
            EAX.val = getMem32(EAX.ref, EAX.val);
            EAX.ref = REF_UNK;
            break;

        case OP_PUSH_ESI_ADD_EDI:
            push32(ESI);
            ESI = _readAccess.getU32(ESI);
            break;

        case OP_CALL_FUNC:
            EAX.val = _readAccess.getU32(ESI);
            ESI += 4;
            if (_callFuncs)
                _callFuncs(_callingObject, this, EAX.val);
            break;

        case OP_PUSH_ESI_SET_EDX_EDI:
            push32(ESI);
            ESI = EDX.val;
            break;
        }
    }

    return EAX.val;
}


uint32 VM::doScript(uint32 scriptAddress, byte *storage) {
    if (_interrupt)
        return 0;

    for (int i = 0; i < THREADS_COUNT; i++) {
        if (!_threads[i]._inUse) {
            _threads[i]._inUse = true;
            uint32 res = _threads[i].execute(scriptAddress, storage);
            _threads[i]._inUse = false;
            return res;
        }
    }

    VM *tmpcontext = new VM();
    uint32 res = tmpcontext->execute(scriptAddress, storage);
    delete tmpcontext;
    return res;
}



int32 VM::getS32(const void *mem) {
    const uint8 *mem8 = (const uint8 *)mem;
    return (int32)(mem8[0] | (mem8[1] << 8) | (mem8[2] << 16) | (mem8[3] << 24));
}

uint32 VM::getU32(const void *mem) {
    const uint8 *mem8 = (const uint8 *)mem;
    return (uint32)(mem8[0] | (mem8[1] << 8) | (mem8[2] << 16) | (mem8[3] << 24));
}

void VM::setU32(void *mem, uint32 val) {
    uint8 *mem8 = (uint8 *)mem;
    mem8[0] = val & 0xff;
    mem8[1] = (val >> 8) & 0xff;
    mem8[2] = (val >> 16) & 0xff;
    mem8[3] = (val >> 24) & 0xff;
}

void VM::push32(uint32 val) {
    SP -= 4;
    setU32(_stack + SP, val);
}

uint32 VM::pop32() {
    uint32 val = getU32(_stack + SP);
    SP += 4;
    return val;
}

void VM::pushReg(Reg reg) {
    SP -= 4;
    setU32(_stack + SP, reg.val);
    _stackT[SP] = reg.ref;
}

VM::Reg VM::popReg() {
    Reg tmp;
    tmp.val = getU32(_stack + SP);
    tmp.ref = _stackT[SP];
    SP += 4;
    return tmp;
}

uint32 VM::getMem32(int memtype, uint32 offset) {
    switch (memtype) {
    default:
    case REF_UNK:
        return 0;

    case REF_STACK:
        return getU32(_stack + offset);

    case REF_EBX:
        return getU32(EBX + offset);

    case REF_EDI:
        return _readAccess.getU32(offset);
    }
}

uint8 VM::getMem8(int memtype, uint32 offset) {
    switch (memtype) {
    default:
    case REF_UNK:
        return 0;

    case REF_STACK:
        return _stack[offset];

    case REF_EBX:
        return EBX[offset];

    case REF_EDI:
        return _readAccess.getU8(offset);
    }
}

void VM::setMem32(int memtype, uint32 offset, uint32 val) {
    switch (memtype) {
    default:
    case REF_UNK:
        break;
    case REF_STACK:
        setU32(_stack + offset, val);
        break;
    case REF_EBX:
        setU32(EBX + offset, val);
        break;

    case REF_EDI:
        _writeAccess.setU32(offset, val);
        break;
    }
}

void VM::setMem8(int memtype, uint32 offset, uint8 val) {
    switch (memtype) {
    default:
    case REF_UNK:
        break;
    case REF_STACK:
        _stack[offset] = val;
        break;
    case REF_EBX:
        EBX[offset] = val;
        break;

    case REF_EDI:
        _writeAccess.setU8(offset, val);
        break;
    }
}

void VM::clearMemory() {
    _memMap.clear();
    _memAccess.reset();
}

void VM::writeMemory(uint32 address, const byte* data, uint32 dataSize) {

    uint32 blockAddr = address & (~0xff);
    uint32 pos = 0;
    uint32 remain = dataSize;

    printf("Write memory at %x sz %x\n", address, dataSize);

    for (uint32 addr = blockAddr; addr < address + dataSize; addr += 0x100) {
        MemoryBlock &block = _memMap.getOrCreateVal(addr);

        block.address = addr; // update it

        uint32 copyCnt = (addr + 0x100) - (address + pos);
        if (copyCnt > remain)
            copyCnt = remain;

        uint32 blockPos = (address + pos) - addr;
        memcpy(block.data + blockPos, data + pos, copyCnt);

        pos += copyCnt;
        remain -= copyCnt;
    }
}

VM::MemoryBlock *VM::findMemoryBlock(uint32 address) {
    Common::HashMap<uint32, MemoryBlock>::iterator it = _memMap.find(address & (~0xff));
    if (it == _memMap.end())
        return nullptr;

    return &it->_value;
}

VM::MemoryBlock *VM::createBlock(uint32 address) {
    MemoryBlock &blk = _memMap.getOrCreateVal(address & (~0xff));
    blk.address = address & (~0xff);
    return &blk;
}



Common::Array<byte> VM::readMemBlocks(uint32 address, uint32 count) {
    Common::Array<byte> data;
    data.resize(count);

    MemoryBlock *blk = findMemoryBlock(address);

    uint32 pos = 0;
    uint32 blockAddr = address & (~0xff);
    uint32 remain = count;
    while(remain > 0) {
        uint32 dataCpyCount = (blockAddr + 0x100) - (address + pos);
        if (dataCpyCount < remain)
            dataCpyCount = remain;

        if (!blk) {
            memset(data.data() + pos, 0, dataCpyCount);
        } else {
            memcpy(data.data() + pos, blk->data + (address + pos - blk->address), dataCpyCount);
        }

        pos += dataCpyCount;
        remain -= dataCpyCount;
        blockAddr += 0x100;
        blk = findMemoryBlock(blockAddr);
    }
    return data;
}

Common::String VM::readMemString(uint32 address, uint32 maxLen) {
    Common::String s;

    MemoryBlock *blk = findMemoryBlock(address);

    if (!blk)
        return s;

    uint32 pos = address - blk->address;
    char c = blk->data[pos];

    while(c != 0) {
        s += c;

        pos++;
        if (pos >= 0x100) {
            blk = findMemoryBlock(blk->address + 0x100);
            pos = 0;
        }

        if (!blk)
            break;

        c = blk->data[pos];
    }
    return s;
}

Common::String VM::getString(int memtype, uint32 offset, uint32 maxLen) {
    switch (memtype) {
        default:
        case REF_UNK:
            return Common::String();

        case REF_STACK: {
            Common::String s = Common::String((const char *)_stack + offset);
            if (s.size() > maxLen)
                s.erase(maxLen);
            return s;
        }
        case REF_EBX: {
            Common::String s = Common::String((const char *)EBX + offset);
            if (s.size() > maxLen)
                s.erase(maxLen);
            return s;
        }

        case REF_EDI:
            return readMemString(offset, maxLen);
    }
}


Common::String VM::decodeOp(uint32 address, int *size) {
    Common::String tmp;

    MemAccess readmem;

    int sz = 1;
    byte op = readmem.getU8(address);

    address++;

    switch (op) {
    default:
    case OP_EXIT:
        tmp = Common::String("EXIT");
        break;

    case OP_CMP_EQ:
        tmp = Common::String("EAX =  EDX == EAX (CMP_EQ)");
        break;

    case OP_CMP_NE:
        tmp = Common::String("EAX =  EDX != EAX (CMP_NE)");
        break;

    case OP_CMP_LE:
        tmp = Common::String("EAX =  EDX < EAX (CMP_LE) //signed");
        break;

    case OP_CMP_LEQ:
        tmp = Common::String("EAX =  EDX <= EAX (CMP_LEQ) //signed");
        break;

    case OP_CMP_GR:
        tmp = Common::String("EAX =  EDX > EAX (CMP_GR) //signed");
        break;

    case OP_CMP_GREQ:
        tmp = Common::String("EAX =  EDX >= EAX (CMP_GREQ) //signed");
        break;

    case OP_CMP_NAE:
        tmp = Common::String("EAX =  EDX < EAX (CMP_NAE) //unsigned");
        break;

    case OP_CMP_NA:
        tmp = Common::String("EAX =  EDX <= EAX (CMP_NA) //unsigned");
        break;

    case OP_CMP_A:
        tmp = Common::String("EAX =  EDX > EAX (CMP_A) //unsigned");
        break;

    case OP_CMP_AE:
        tmp = Common::String("EAX =  EDX >= EAX (CMP_AE) //unsigned");
        break;

    case OP_BRANCH:
        tmp = Common::String::format("BR %x", address + (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_JMP:
        tmp = Common::String::format("JMP %x", address + (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_SP_ADD:
        tmp = Common::String::format("ADD SP, %x", (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EDI_ECX_AL:
        tmp = Common::String::format("MOV byte ptr[EDI + %x], AL", (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EBX_ECX_AL:
        tmp = Common::String::format("MOV byte ptr[EBX + %x], AL", (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EDI_ECX_EAX:
        tmp = Common::String::format("MOV dword ptr[EDI + %x], EAX", (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EBX_ECX_EAX:
        tmp = Common::String::format("MOV dword ptr[EBX + %x], EAX", (int32)readmem.getU32(address));
        sz += 4;
        break;

    case OP_RET:
        tmp = Common::String("RET");
        break;

    case OP_RETX:
        tmp = Common::String::format("RET%x", readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EDX_EAX:
        tmp = Common::String("MOV EDX, EAX");
        break;

    case OP_ADD_EAX_EDX:
        tmp = Common::String("ADD EAX, EDX");
        break;

    case OP_MUL:
        tmp = Common::String("MUL EDX");
        break;

    case OP_OR:
        tmp = Common::String("OR EDX");
        break;

    case OP_XOR:
        tmp = Common::String("XOR EDX");
        break;

    case OP_AND:
        tmp = Common::String("AND EDX");
        break;

    case OP_NEG:
        tmp = Common::String("NEG EAX");
        break;

    case OP_SAR:
        tmp = Common::String("SAR EAX, EDX,EAX // edx>>eax");
        break;

    case OP_SHL:
        tmp = Common::String("SHL EAX, EDX,EAX // edx<<eax");
        break;

    case OP_LOAD:
        tmp = Common::String::format("MOV EAX, %x", readmem.getU32(address));
        sz += 4;
        break;

    case OP_INC:
        tmp = Common::String("INC EAX");
        break;

    case OP_DEC:
        tmp = Common::String("DEC EAX");
        break;

    case OP_XCHG:
        tmp = Common::String("XCHG EAX,EDX");
        break;

    case OP_PUSH_EAX:
        tmp = Common::String("PUSH EAX");
        break;

    case OP_POP_EDX:
        tmp = Common::String("POP EDX");
        break;

    case OP_LOAD_OFFSET_EDI:
    case OP_LOAD_OFFSET_EDI2:
        tmp = Common::String::format("LEA EAX, [EDI + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_LOAD_OFFSET_EBX:
        tmp = Common::String::format("LEA EAX, [EBX + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_LOAD_OFFSET_ESP:
        tmp = Common::String::format("LEA EAX, [SP + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_PTR_EDX_AL:
        tmp = Common::String("MOV byte ptr [EDX], AL");
        break;

    case OP_MOV_PTR_EDX_EAX:
        tmp = Common::String("MOV dword ptr [EDX], EAX");
        break;

    case OP_SHL_2:
        tmp = Common::String("SHL EAX, 2");
        break;

    case OP_ADD_4:
        tmp = Common::String("ADD EAX, 4");
        break;

    case OP_SUB_4:
        tmp = Common::String("SUB EAX, 4");
        break;

    case OP_XCHG_ESP:
        tmp = Common::String("XCHG EAX, [SP]");
        break;

    case OP_NEG_ADD:
        tmp = Common::String("EAX = EDX - EAX (OP_NEG_ADD)");
        break;

    case OP_DIV:
        tmp = Common::String("EAX = EDX / EAX  |   EDX = EDX %% EAX (DIV)");
        break;

    case OP_MOV_EAX_BPTR_EDI:
        tmp = Common::String::format("MOV EAX, byte ptr [EDI + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EAX_BPTR_EBX:
        tmp = Common::String::format("MOV EAX, byte ptr [EBX + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EAX_DPTR_EDI:
        tmp = Common::String::format("MOV EAX, dword ptr [EDI + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EAX_DPTR_EBX:
        tmp = Common::String::format("MOV EAX, dword ptr [EBX + %x]", readmem.getU32(address));
        sz += 4;
        break;

    case OP_MOV_EAX_BPTR_EAX:
        tmp = Common::String("MOV EAX, byte ptr [EAX]");
        break;

    case OP_MOV_EAX_DPTR_EAX:
        tmp = Common::String("MOV EAX, dword ptr [EAX]");
        break;

    case OP_PUSH_ESI_ADD_EDI:
        tmp = Common::String::format("CALL %x", readmem.getU32(address));
        sz += 4;
        break;

    case OP_CALL_FUNC:
        tmp = Common::String::format("CALL FUNC %d", readmem.getU32(address));
        sz += 4;
        break;

    case OP_PUSH_ESI_SET_EDX_EDI:
        tmp = Common::String("CALL EDX");
        break;
    }

    if (size)
        *size = sz;

    return tmp;
}


Common::String VM::disassembly(uint32 address) {
    Common::String tmp;

    uint32 addr = address;
    MemAccess readmem;

    while (true) {
        tmp += Common::String::format("%08x: ", addr);

        byte op = readmem.getU8(addr);

        int sz = 1;
        tmp += decodeOp(addr, &sz);
        tmp += "\n";

        addr += sz;

        if (op == OP_EXIT)
            break;
    }

    return tmp;
}


Common::String VM::opLog(const Common::Array<OpLog> &log) {
    Common::String tmp;

    for (const OpLog &l : log) {
        tmp += Common::String::format("%08x: SP:%04x OP:[%02d] ", l.addr, l.sp, l.op) + decodeOp(l.addr) + "\n";
    }

    FILE *f = fopen("oplog", "wb");
    fwrite(tmp.c_str(), tmp.size(), 1, f);
    fclose(f);

    return tmp;
}

void VM::printDisassembly(uint32 address) {
    Common::String tmp = disassembly(address);
    warning(tmp.c_str());
}

}
